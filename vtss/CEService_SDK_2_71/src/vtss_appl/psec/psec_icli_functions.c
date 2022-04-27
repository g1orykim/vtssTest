/*

 Vitesse Switch API software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.

*/
#include "icli_api.h"
#include "icli_porting_util.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif
#include "psec_api.h"
#include "psec.h"
#include "psec_trace.h"
#include "msg_api.h"       /* For msg_abstime_get()                                   */
/***************************************************************************/
/*  Internal types                                                         */
/****************************************************************************/
// Enum for selecting which status or configuration to access
typedef enum {
    PORT,      // Displaying ports security port
    SWITCH    // Displaying ports security switch;
} psec_cmd_t;

/***************************************************************************/
/*  Internal functions                                                     */
/****************************************************************************/

// Function printing the port security status
// In : Session_Id - For ICLI printing
//      sit        - Switch information
//      pit        - Port information
static vtss_rc psec_icli_print_switch(const i32 session_id, const switch_iter_t *sit, const port_iter_t *pit)
{
    i8 ena_str[PSEC_USER_CNT + 1]; // +1 for terminating NULL
    i8 buf[100];
    psec_users_t user;
    psec_port_state_t port_state;

    // Print headers
    if (pit->first) {
        ICLI_PRINTF("Users:\n");
        for (user = (psec_users_t)0; user < PSEC_USER_CNT; user++) {
            ICLI_PRINTF("%c = %s\n", psec_user_abbr(user), psec_user_name(user));
        }


        sprintf(buf, "%-23s  %-5s  %-13s  %-7s", "Interface", "Users", "State", "MAC Cnt");
        icli_table_header(session_id, buf);
    }

    // The FALSE argument tells the psec_mgmt_port_state_get() function
    // that we haven't obtained the critical section, so that the function
    // should do it itself.
    VTSS_RC(psec_mgmt_port_state_get(sit->isid, pit->iport, &port_state, FALSE));

    strcpy(ena_str, "");
    for (user = (psec_users_t)0; user < PSEC_USER_CNT; user++) {
        sprintf(ena_str, "%s%c", ena_str, PSEC_USER_ENA_GET(&port_state, user) ? psec_user_abbr(user) : '-');
    }

    ICLI_PRINTF("%-23s  %-5s  ", icli_port_info_txt(sit->usid, pit->uport, buf), ena_str);
    ICLI_PRINTF("%-13s  ", port_state.ena_mask == 0 ? "No users" : (port_state.flags & PSEC_PORT_STATE_FLAGS_SHUT_DOWN) ? "Shutdown" : (port_state.flags & PSEC_PORT_STATE_FLAGS_LIMIT_REACHED) ? "Limit Reached" : "Ready");
    ICLI_PRINTF("%7u\n", port_state.mac_cnt);

    return VTSS_RC_OK;
}

// Function printing MAC Addresses learned by Port Security.
// In : Session_Id - For ICLI printing
//      sit        - Switch information
//      pit        - Port information
static vtss_rc psec_icli_print_port(const i32 session_id, const switch_iter_t *sit, const port_iter_t *pit)
{
    i8                buf[100];
    psec_port_state_t port_state;
    psec_mac_state_t  *mac_state;
    i8                age_or_hold_time_secs_str[15];

    // Print Interface
    icli_table_header(session_id, icli_port_info_txt(sit->usid, pit->uport, buf));

    sprintf(buf, "%-17s  %-4s  %-10s  %-25s  %-13s", "MAC Address", "VID", "State" , "Added", "Age/Hold Time");
    icli_table_header(session_id, buf);

    // The TRUE in the call to psec_mgmt_port_state_get() means that we also need
    // the MAC addresses. The function VTSS_MALLOC()s an array of such addresses, and
    // we must free it.
    VTSS_RC(psec_mgmt_port_state_get(sit->isid, pit->iport, &port_state, TRUE));

    // Loop through all MAC addresses attached to this port
    mac_state = port_state.macs;
    if (mac_state) {
        while (mac_state) {
            if (mac_state->flags & PSEC_MAC_STATE_FLAGS_IN_MAC_MODULE) {
                if (mac_state->age_or_hold_time_secs == 0) {
                    (void) snprintf(age_or_hold_time_secs_str, sizeof(age_or_hold_time_secs_str), "N/A");
                } else {
                    (void) snprintf(age_or_hold_time_secs_str, sizeof(age_or_hold_time_secs_str), "%13u", mac_state->age_or_hold_time_secs);
                }

                // Don't bother the user with entries not added to the MAC table for one or the other reason.
                (void)snprintf(buf, sizeof(buf), "%17s  %4u  %-10s  %25s  %13s\n", misc_mac2str(mac_state->vid_mac.mac.addr),
                               mac_state->vid_mac.vid,
                               (mac_state->flags & PSEC_MAC_STATE_FLAGS_BLOCKED) ? "Blocked" : "Forwarding",
                               misc_time2str(msg_abstime_get(VTSS_ISID_LOCAL, mac_state->creation_time_secs)), /* 25 chars */
                               age_or_hold_time_secs_str);

                ICLI_PRINTF(buf);
            }

            mac_state = mac_state->next;
        } /* while (mac_state) */
    } else {
        ICLI_PRINTF("<none>\n");
    } /* if (mac_state) */

    ICLI_PRINTF("\n");

    // Must free the MAC address "array".
    if (port_state.macs) {
        VTSS_FREE(port_state.macs);
    }
    return VTSS_RC_OK;
}

// Function for looping over all switches and all ports a the port list, and the calling a configuration or status/statistics function.
// In : session_id - For printing
//      plist      - Containing information about which switches and ports to "access"
//      cmd        - Containing information about which function to call.
static vtss_rc psec_icli_sit_port_loop(i32 session_id, icli_stack_port_range_t *plist, const psec_cmd_t *cmd)
{
    switch_iter_t         sit;
    VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID));
    while (icli_switch_iter_getnext(&sit, plist)) {
        psec_switch_status_t switch_status;

        VTSS_RC(psec_mgmt_switch_status_get(sit.isid, &switch_status))

        // Loop through all ports
        port_iter_t pit;
        VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI ));
        while (icli_port_iter_getnext(&pit, plist)) {
            switch (*cmd) {
            case PORT:
                VTSS_RC(psec_icli_print_port(session_id, &sit, &pit));
                break;

            case SWITCH:
                VTSS_RC(psec_icli_print_switch(session_id, &sit, &pit));
                break;
            }
        }
    }
    return VTSS_RC_OK;
}
/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/
//  see psec_icli_functions.h
vtss_rc psec_icli_show_port(i32 session_id, icli_stack_port_range_t *plist)
{
    psec_cmd_t  cmd;
    cmd = PORT;
    VTSS_ICLI_ERR_PRINT(psec_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

//  see psec_icli_functions.h
vtss_rc psec_icli_show_switch(i32 session_id, icli_stack_port_range_t *plist)
{
    psec_cmd_t  cmd;
    cmd = SWITCH;
    VTSS_ICLI_ERR_PRINT(psec_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
