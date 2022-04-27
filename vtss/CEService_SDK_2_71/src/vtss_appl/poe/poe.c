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
/*lint -esym(459,poe_msg_id_get_stat_req_flags) */
/*lint -esym(459,poe_msg_id_get_port_status_req_flags) */
/*lint -esym(459,poe_msg_id_get_pd_classes_req_flags) */

/*lint -esym(457,lldp_remote_set_requested_power) */ // Ok - LLDP is writing and PoE is reading. It doesn't matter if the read is missed for one cycle.
/****************************************************************************
PoE ( Power Over Ethernet ) is used to control external PoE chips. For more
information see the Design Spec (DS 0153)
*****************************************************************************/
#include "critd_api.h"
#include "conf_api.h"
#include "msg_api.h"
#include "poe.h"
#ifdef VTSS_SW_OPTION_LLDP
#include "lldp_sm.h"
#include "lldp_remote.h"
#include "lldp_api.h"
#include "lldp_basic_types.h"
#endif //  VTSS_SW_OPTION_LLDP
#include "poe_custom_api.h"
#include "misc_api.h"
#include "interrupt_api.h"

#ifdef VTSS_SW_OPTION_VCLI
#include "poe_cli.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "poe_icli_functions.h" // For poe_icfg_init
#endif

/****************************************************************************/
/*  TRACE system                                                            */
/****************************************************************************/

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "poe",
    .descr     = "Power Over Ethernet"
};

static vtss_trace_grp_t trace_grps[VTSS_TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_MGMT] = {
        .name      = "pow_mgmt",
        .descr     = "Power Management",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_CONF] = {
        .name      = "conf",
        .descr     = "Configuration",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_MSG] = {
        .name      = "msg",
        .descr     = "Messages",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_CUSTOM] = {
        .name      = "custom",
        .descr     = "PoE Chip set",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_STATUS] = {
        .name      = "status",
        .descr     = "Status",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_ICLI] = {
        .name      = "iCLI",
        .descr     = "iCLI",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
};
#define POE_CRIT_ENTER()         critd_enter(        &crit,        VTSS_TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define POE_CRIT_EXIT()          critd_exit(         &crit,        VTSS_TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define POE_CRIT_CUSTOM_ENTER()  T_R("Custom Enter "); critd_enter(        &crit_custom, VTSS_TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define POE_CRIT_CUSTOM_EXIT()   T_R("Custom Exit"); critd_exit(         &crit_custom, VTSS_TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)

#define POE_CRIT_STATUS_ENTER()  critd_enter(&crit_status, VTSS_TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define POE_CRIT_STATUS_EXIT()   critd_exit(&crit_status,  VTSS_TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)

#else
#define POE_CRIT_ENTER()         critd_enter(        &crit)
#define POE_CRIT_EXIT()          critd_exit(         &crit)
#define POE_CRIT_CUSTOM_ENTER()  critd_enter(        &crit_custom)
#define POE_CRIT_CUSTOM_EXIT()   critd_exit(         &crit_custom)
#define POE_CRIT_STATUS_ENTER()  critd_enter(        &crit_status)
#define POE_CRIT_STATUS_EXIT()   critd_exit(         &crit_status)

#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*  Pre-defined functions */
/****************************************************************************/
static void update_registers(void);
/****************************************************************************/
/*  Global variables */
/****************************************************************************/

// Variable to signaling the PoE chipset has been initialized
static BOOL poe_init_done = FALSE;

// Message
static cyg_flag_t poe_msg_id_get_stat_req_flags;        // Flag for synch. of request and reply of status.
static cyg_flag_t poe_msg_id_get_pd_classes_req_flags;  // Flag for synch. of request and reply of pd classes.
#define POE_TIMEOUT cyg_current_time() + VTSS_OS_MSEC2TICK(20 * 1000)         // Wait for timeout (20 seconds) or synch. flag to be set ( I2C might be blocked by other modules/threads ).

static poe_status_t poe_status_global; // The Status from a slave switch

// Configuration
static poe_stack_conf_t poe_stack_cfg; // Current stack configuration (only valid for the master).
static poe_local_conf_t poe_local_conf;   // Current configuration for the local switch.

static BOOL power_budget_exceeded[VTSS_PORTS]; // If we do manual power management ( which is
// needed in some modes) this indicates if the port is forced off,
// due to that the power budget has been exceeded
static BOOL pd_overload[VTSS_PORTS]; //

/* Critical region protection */
static critd_t crit;  // Critical region for global variables
static critd_t crit_custom;  // Critial region for the shared custom poe chipset.
static critd_t crit_status;  // Critial region for the poe_status.

// When a port is turned off due to that the port uses too much power, we have to turn it on again
// sometime to see if it still uses too much power.
// This is done with this counter. When the the counter hs counted down to zero, the port is turn
// on again.
static char retry_cnt[VTSS_PORTS];
const  char retry_cnt_max = 5; // Specifies the maximum number of seconds (Depending upon the sleep in poe_thread function) to wait before tuning on the port


// PoE request message buffer pool
static void *poe_request;

// PoE reply message buffer pool
static void *poe_reply;

/*************************************************************************
** Misc Functions
*************************************************************************/


/*****************************************************************/
// Description: Converting a integer to a string with one digit. E.g. 102 becomes 10.2. *
// Output: Returns pointer to string
/*****************************************************************/

char *one_digi_float2str(int val, char *max_power_val_str)
{
    char digi[2] = "";

    // Convert the integer to a string
    sprintf(max_power_val_str, "%d", val);

    int str_len = strlen(max_power_val_str);

    // get the last charater in the string
    digi[0] =  max_power_val_str[str_len - 1];
    digi[1] =  '\0';

    // Remove the last digi in the string
    max_power_val_str[str_len - 1] = '\0';


    if (str_len == 1) {
        // If the integer only were one digi then add "0." in front. E.g. 4 becomes 0.4
        strcat(max_power_val_str, "0.");
    } else {
        // Add the "dot" to the string
        strcat(max_power_val_str, ".");
    }

    // Add the digi to the string
    strcat(max_power_val_str, &digi[0]);


    // return the string
    return max_power_val_str;
}



// Function that converts from class into power in deci watts
static int class2power (char class, vtss_port_no_t iport)
{
    // Classes is defined in IEEE 802.3at table 33-6.
    switch (class) {
    case 0 :
    case 3 :
        return 154; // 15.4 watts
    case 1 :
        return 40; // 4.0 watts
    case 2 :
        return 70; // 7.0 watts
    case 4 :
        return 300; // Maximum for PoE+
    default :
        return 300; // Maximum for PoE+
    }
}

// Function that returns the maximum power allowed for a port (depending upon the mode)
// In : iport - Port starting from 0. The port at which to get the maximum power
//      poe_mode - The current mode for iport.
// return  : The maximum allowed power for the port.
u16 poe_max_power_mode_dependent(vtss_port_no_t iport, poe_mode_t poe_mode)
{
    u16 max_power = 0;

    POE_CRIT_ENTER();
    switch (poe_mode) {
    case POE_MODE_POE:
        // 15.4 W is Maximum for PoE
        max_power = 154;
        break;
    case POE_MODE_POE_PLUS:
        // 30W is Maximum for PoE+
        max_power = 300;
        break;
    case POE_MODE_POE_DISABLED:
        max_power = 300;
        break;
    default:
        max_power = 300;
        break;
    }

    T_N("max_power = %d, %d, mode = %d",
        max_power,
        poe_custom_get_port_power_max(iport),
        poe_local_conf.poe_mode[iport]);

    POE_CRIT_EXIT();

    // Make sure that we doesn't configure more power than the PoE chip set can deliver.
    if (max_power < poe_custom_get_port_power_max(iport)) {
        return max_power;
    } else {
        return poe_custom_get_port_power_max(iport);
    }
}

static int poe_find_allocated_power(int requested_power, vtss_port_no_t iport)
{
    int max_power;

    // Set max power according to class.
    max_power = poe_max_power_mode_dependent(iport, poe_local_conf.poe_mode[iport]);


    POE_CRIT_ENTER(); // Protect poe_local_conf
    if (poe_local_conf.poe_mode[iport] == POE_MODE_POE_DISABLED) {
        // If the port is disabled then allocate  0W
        max_power = 0;
    }
    POE_CRIT_EXIT();

    // If the request power is less than the class allow then only allocate the requested power.
    if (requested_power < max_power) {
        max_power = requested_power;
    }

    return max_power;
}



// Getting Port status
// In/Out : port_status - Pointer to where the port classes are put.
static void poe_get_all_pd_classes(char *classes)
{
    poe_custom_get_all_ports_classes(classes);
}


// Function for updating a local copy of the PoE status.
// IN :  local_poe_status - Pointer to the new status, or Pointer to where to put the current status. Depends upon the get parameter-.
//       get _ True to get the last status, FALSE to update status.
static void poe_status_set_get(poe_status_t *local_poe_status, BOOL get)
{
    // The status from PoE is updated every sec in order to be able to respond upon changes. To get fast access to the status from management
    // we keep a local copy of the status which we give back fast.
    static poe_status_t poe_status;
    T_R("Enter poe_status_get get:%d", get);
    POE_CRIT_STATUS_ENTER();
    if (get) {
        memcpy(local_poe_status, &poe_status, sizeof(poe_status_t));
    } else {
        memcpy(&poe_status, local_poe_status, sizeof(poe_status_t));
    }
    POE_CRIT_STATUS_EXIT();
    T_R("Exit poe_status_get get:%d", get);
}


// Getting and updating poe status
static void poe_status_update(void)
{
    poe_status_t poe_status;
    vtss_port_no_t port_index;
    char classes[VTSS_PORTS];
    T_NG(VTSS_TRACE_GRP_STATUS, "Getting PoE poe_status" );

    // Update Power and current used fields
    poe_custom_get_status(&poe_status);

    // Update the port_status fields
    poe_custom_get_all_ports_status(&poe_status.port_status[0]);

    // Get the class for all PDs connected.
    poe_get_all_pd_classes(&classes[0]);

    for (port_index = 0 ; port_index < VTSS_PORTS ; port_index++) {
        poe_status.poe_chip_found[port_index] = poe_custom_is_chip_found(port_index);

        // Make sure that power used and current used have valid value even when no PoE chip is not found.
        if (poe_status.port_status[port_index] == POE_NOT_SUPPORTED ||
            poe_status.port_status[port_index] == NO_PD_DETECTED ||
            poe_local_conf.poe_mode[port_index] == POE_MODE_POE_DISABLED ||
            poe_status.port_status[port_index] == UNKNOWN_STATE) {
            T_DG_PORT(VTSS_TRACE_GRP_STATUS, port_index, "Resetting status for port with not PoE chipset");
            poe_status.power_used[port_index] = 0;
            poe_status.current_used[port_index] = 0;
            poe_status.pd_class[port_index] = 0;
            poe_status.power_requested[port_index] = 0;
            poe_status.power_allocated[port_index] =  0;

            // We have seen that some PoE Chipsets don't give PoE disable back when the port is powered down,
            // so we force the status in order to make sure that status is shown correct.
            if (poe_status.port_status[port_index] != POE_NOT_SUPPORTED) {
                if (poe_local_conf.poe_mode[port_index] == POE_MODE_POE_DISABLED) {
                    poe_status.port_status[port_index] = POE_DISABLED;
                }
            }
            continue; // continue to next port
        }

        // Set reqeusted power either by LLDP, class or reserved.
        if (poe_local_conf.power_mgmt_mode == LLDPMED_RESERVED ||
            poe_local_conf.power_mgmt_mode == LLDPMED_CONSUMP) {
#ifdef VTSS_SW_OPTION_LLDP
            if (lldp_remote_lldp_is_info_valid(port_index)) {
                poe_status.power_requested[port_index] = lldp_remote_get_requested_power(port_index, poe_local_conf.poe_mode[port_index]);
                T_DG_PORT(VTSS_TRACE_GRP_STATUS, port_index, "lldp_remote_lldp_is_info_valid");
            } else {
                T_DG_PORT(VTSS_TRACE_GRP_STATUS, port_index, "NOT lldp_remote_lldp_is_info_valid");
                poe_status.power_requested[port_index] = class2power(classes[port_index], port_index);
            }
#endif // VTSS_SW_OPTION_LLDP
        } else if (poe_local_conf.power_mgmt_mode == CLASS_RESERVED ||
                   poe_local_conf.power_mgmt_mode == CLASS_CONSUMP) {
            poe_status.power_requested[port_index] =  class2power(classes[port_index], port_index);
            T_NG_PORT(VTSS_TRACE_GRP_MGMT, port_index, "power_requested = %d ( via class)", poe_status.power_requested[port_index]);
        } else {
            poe_status.power_requested[port_index] = poe_local_conf.max_port_power[port_index];
            T_NG_PORT(VTSS_TRACE_GRP_MGMT, port_index, "power_requested = %d", poe_local_conf.max_port_power[port_index]);
        }

        // Work around because the pd63000 PoE card sets some ports error state when no PD is connected.
        // This is still being investigated.
        if (poe_status.port_status[port_index] == UNKNOWN_STATE) {
            poe_status.power_requested[port_index] = 0;
        }

        // If PoE is disabled for a port then it shall of course request 0 W.
        if (poe_local_conf.poe_mode[port_index] == POE_MODE_POE_DISABLED ||
            poe_custom_is_chip_found(port_index) == NO_POE_CHIPSET_FOUND  ||
            poe_status.port_status[port_index] == NO_PD_DETECTED) {
            poe_status.power_requested[port_index] = 0;
        }

        T_DG_PORT(VTSS_TRACE_GRP_STATUS, port_index, "Port status = %d, Requested Power = %d,",
                  poe_status. port_status[port_index], poe_status.power_requested[port_index]);


        // Change port status if the port is forced off if the power budget is exceeded.
        if (power_budget_exceeded[port_index]) {
            T_DG_PORT(VTSS_TRACE_GRP_STATUS, port_index, "Forcing status to power budget exceeded");
            poe_status.port_status[port_index] = POWER_BUDGET_EXCEEDED;
        }

        if (pd_overload[port_index]) {
            T_DG_PORT(VTSS_TRACE_GRP_STATUS, port_index, "Forcing status to pd overload");
            poe_status.port_status[port_index] = PD_OVERLOAD;
        }


        // Update power allocated fields.
        if (poe_status.port_status[port_index] == PD_ON) {
            poe_status.power_allocated[port_index] =  poe_find_allocated_power(poe_status.power_requested[port_index], port_index);
        } else {
            poe_status.power_allocated[port_index] =  0;
        }
        poe_status.pd_class[port_index] = classes[port_index]; // Set the class field (used by Web).

        T_DG_PORT(VTSS_TRACE_GRP_STATUS, port_index, "Class = %d, power allocated = %d, power requested = %d",
                  classes[port_index], poe_status.power_allocated[port_index], poe_status.power_requested[port_index]);
    }

    T_RG_PORT(VTSS_TRACE_GRP_STATUS, port_index, "Exiting poe_get_status" );
    poe_status_set_get(&poe_status, FALSE); // Update the status
}

/*************************************************************************
** Message functions
*************************************************************************/

/* Allocate request buffer */
static poe_msg_req_t *poe_msg_req_alloc(poe_msg_id_t msg_id)
{
    poe_msg_req_t *msg = msg_buf_pool_get(poe_request);
    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

/* Allocate reply buffer */
static poe_msg_rep_t *poe_msg_rep_alloc(poe_msg_id_t msg_id)
{
    poe_msg_rep_t *msg = msg_buf_pool_get(poe_reply);
    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

/* Release message buffer */
static void poe_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    (void)msg_buf_pool_put(msg);
}

/* Send message */
static void poe_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
    // Avoid "Warning -- Constant value Boolean" Lint warning, due to the use of MSG_TX_DATA_HDR_LEN_MAX() below
    /*lint -e{506} */
    msg_tx_adv(NULL, poe_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_POE, isid, msg, len + MSG_TX_DATA_HDR_LEN_MAX(poe_msg_req_t, req, poe_msg_rep_t, rep));
}

/* Send configuration to a slave  */
static void poe_update_slave_conf(vtss_isid_t isid)
{
    poe_msg_req_t *msg;
    switch_iter_t  sit;

    T_RG(VTSS_TRACE_GRP_CONF, "isid:%d", isid);
    // Send the request for setting PoE configuration to selected switch(es).
    (void)switch_iter_init(&sit, isid, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        T_NG(VTSS_TRACE_GRP_CONF, "isid:%d", sit.isid);
        msg = poe_msg_req_alloc(POE_MSG_ID_CONF_SET_REQ);
        POE_CRIT_ENTER();
        msg->req.local_conf = poe_stack_cfg.local_conf[sit.isid];
        msg->req.local_conf.power_mgmt_mode = poe_stack_cfg.master_conf.power_mgmt_mode;
        POE_CRIT_EXIT();
        poe_msg_tx(msg, sit.isid, sizeof(msg->req.local_conf));
    }
}

// Function that enables/disables power for a PD depending upon power budget and pd_overload.
// In : None (power_budget_exceeded and pd_overload are global variables)
// Return : None.
static void poe_update_pd_state(const poe_status_t *poe_status)
{
    vtss_port_no_t port_index;
    poe_local_conf_t configuration;

    // Only update the PD state when PoE initialization is done.
    if (poe_init_done) {
        for (port_index = 0 ; port_index < VTSS_PORTS; port_index++ ) {
            configuration = poe_local_conf;
            T_IG_PORT(VTSS_TRACE_GRP_MGMT, port_index, "power_budget_exceeded:%d, pd_overload:%d, status:%d",
                      power_budget_exceeded[port_index], pd_overload[port_index], poe_status->port_status[port_index]);

            // Turn of in case power is exceeded. We keep the PD off until it is detected in order to make sure that
            // power budget calculation is done before powering on a PD.
            if ((poe_status->port_status[port_index] == NO_PD_DETECTED && poe_custom_is_chip_found(port_index) != PD690xx) || // PD690xx is special, because the PD is not detected correctly, if the port is disabled. When the port isn't kept disabled for the PD690xx, it means that in "reserved mode" the PD is turned on for a short moment in the case where there is not enough power left for it, and it should therefore never have been turned on. Still looking for a work around for this (Bugzilla#9061).
                power_budget_exceeded[port_index] ||
                pd_overload[port_index]) {


                poe_custom_port_enable(port_index, FALSE, configuration.poe_mode[port_index]); // Turn Off the PD
                T_IG_PORT(VTSS_TRACE_GRP_MGMT, port_index, "poe_mode:%d", configuration.poe_mode[port_index]);

            } else {
                poe_custom_port_enable(port_index,
                                       configuration.poe_mode[port_index] != POE_MODE_POE_DISABLED,
                                       configuration.poe_mode[port_index]); // Turn On the PD (unless PoE is disabled for this port)
                T_IG_PORT(VTSS_TRACE_GRP_MGMT, port_index, "poe_mode:%d", configuration.poe_mode[port_index]);
            }
        }
    }
}

// Getting message from the message module.
static BOOL poe_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    vtss_port_no_t  port_index;
    poe_msg_id_t msg_id = *(poe_msg_id_t *)rx_msg;

    T_NG(VTSS_TRACE_GRP_MSG, "Entering poe_msg_rx");

    switch (msg_id) {
    case POE_MSG_ID_CONF_SET_REQ: {
        poe_msg_req_t *msg = (poe_msg_req_t *)rx_msg;
        int conf_changed;

        T_DG(VTSS_TRACE_GRP_MSG, "msg_id = POE_MSG_ID_CONF_SET_REQ");
        POE_CRIT_ENTER();
        conf_changed = memcmp(&poe_local_conf, &msg->req.local_conf, sizeof(poe_local_conf));
        poe_local_conf = msg->req.local_conf; // Update configuration for this switch.
        POE_CRIT_EXIT();

        if (conf_changed) {
            T_IG(VTSS_TRACE_GRP_MGMT, "Conf changed");
            // Tell each port to retry to turn on the PD at once, else the user is wondering why it takes some time from
            // he has given the command to it is executed.
            POE_CRIT_ENTER();
            for (port_index = 0 ; port_index < VTSS_PORTS; port_index++ ) {
                retry_cnt[port_index] = 0;
            }
            POE_CRIT_EXIT();
            update_registers(); // Update the Chipset with the new configuration.
        }
        break;
    }

    case POE_MSG_ID_GET_STAT_REQ: {
        // Send a message back with the status
        poe_msg_rep_t *msg;

        // Master has requested a slave's status
        T_DG(VTSS_TRACE_GRP_MSG, "Transmitting status request, isid = %u", isid);

        msg = poe_msg_rep_alloc(POE_MSG_ID_GET_STAT_REP);
        T_DG_HEX(VTSS_TRACE_GRP_MSG, (uchar *)msg, 10);
        T_IG(VTSS_TRACE_GRP_MSG, "msg:%p, status:%p", msg, &msg->rep.status);
        poe_status_set_get(&msg->rep.status, TRUE);
        T_DG_HEX(VTSS_TRACE_GRP_MSG, (uchar *)msg, 10);
        poe_msg_tx(msg, isid, sizeof(msg->rep.status));
        T_DG(VTSS_TRACE_GRP_MSG, "Transmitted status request, isid = %u", isid);
        break;
    }

    case POE_MSG_ID_GET_STAT_REP: {
        poe_msg_rep_t *msg = (poe_msg_rep_t *)rx_msg;

        T_DG(VTSS_TRACE_GRP_MSG, "Got STATUS reply from isid:%u", isid);
        POE_CRIT_ENTER();
        // FJTBD: Only one single status? What if you have more than one management interface active at a time, requesting status from two different switches?
        poe_status_global = msg->rep.status;
        POE_CRIT_EXIT();
        cyg_flag_setbits(&poe_msg_id_get_stat_req_flags, 1 << isid); // Signal that the message has been received
        break;
    }

    case POE_MSG_ID_GET_PD_CLASSES_REQ: {
        // Master has requested a slave's PD classes
        // Send a message back with the status
        poe_msg_rep_t *msg;

        T_DG(VTSS_TRACE_GRP_MSG, "Transmitting status request, isid = %u", isid);

        msg = poe_msg_rep_alloc(POE_MSG_ID_GET_STAT_REP);

        POE_CRIT_CUSTOM_ENTER();
        poe_get_all_pd_classes(&msg->rep.classes[0]) ;
        POE_CRIT_CUSTOM_EXIT();

        poe_msg_tx(msg, isid, sizeof(msg->rep.classes));
        break;
    }

    case POE_MSG_ID_GET_PD_CLASSES_REP: {
        poe_msg_rep_t *msg = (poe_msg_rep_t *)rx_msg;
        int i;

        POE_CRIT_ENTER();
        // FJTBD: Only one single status? What if you have more than one management interface active at a time, requesting status from two different switches?
        for (i = 0; i < (int)ARRSZ(poe_status_global.pd_class); i++) {
            poe_status_global.pd_class[i] = msg->rep.classes[i];
        }
        POE_CRIT_EXIT();
        cyg_flag_setbits(&poe_msg_id_get_pd_classes_req_flags, 1 << isid); // Signal that the message has been received
        break;
    }

    default:
        T_E("unknown message ID: %d", msg_id);
        break;
    }

    return TRUE;
}

// Function called when booting
static void poe_msg_init(void)
{
    /* Register for stack messages */
    msg_rx_filter_t filter;

    T_RG(VTSS_TRACE_GRP_MSG, "entering poe_msg_init");

    /* Initialize message buffers */
    poe_request = msg_buf_pool_create(VTSS_MODULE_ID_POE, "Request", 1, sizeof(poe_msg_req_t));
    poe_reply   = msg_buf_pool_create(VTSS_MODULE_ID_POE, "Reply",   1, sizeof(poe_msg_rep_t));

    memset(&filter, 0, sizeof(filter));
    filter.cb = poe_msg_rx;
    filter.modid = VTSS_TRACE_MODULE_ID;
    T_DG(VTSS_TRACE_GRP_MSG, "Exiting poe_msg_init");
    if (msg_rx_filter_register(&filter) != VTSS_OK) {
        T_D("Problem registering msg_rx_filter_register");
    }
}

/*********************************************************************
** Power management
*********************************************************************/

// Function that sorts a port list in order according to the ports priority
// Port with highest priority is first in the list.
static void poe_sort_after_prio(int *port_list)
{
    int port_index;
    POE_CRIT_ENTER();
    for (port_index = 0 ; port_index < VTSS_PORTS; port_index++ ) {
        if (poe_local_conf.priority[port_index] == CRITICAL) {
            *port_list = port_index;
            port_list ++;
        }
    }

    for (port_index = 0 ; port_index < VTSS_PORTS; port_index++ ) {
        if (poe_local_conf.priority[port_index] == HIGH) {
            *port_list = port_index;
            port_list ++;
        }
    }

    for (port_index = 0 ; port_index < VTSS_PORTS; port_index++ ) {
        if (poe_local_conf.priority[port_index] == LOW) {
            *port_list = port_index;
            port_list ++;
        }
    }

    POE_CRIT_EXIT();
}


// Function for doing power management of the individual ports.
static void poe_port_power_mgmt (const poe_status_t *poe_status)
{
    vtss_port_no_t port_index;

    POE_CRIT_ENTER();
    for (port_index = 0; port_index < VTSS_PORTS; port_index ++ ) {
        if (retry_cnt[port_index] == 0) {
            int power_used = poe_status->power_used[port_index];
            T_DG_PORT(VTSS_TRACE_GRP_MGMT, port_index, "Power used= %u, Power allocated= %d",
                      power_used, poe_status->power_allocated[port_index]);

            // If the port doesn't have a PD connected there is no need to check it.
            if (poe_status->port_status[port_index] != NO_PD_DETECTED && (poe_local_conf.poe_mode[port_index] != POE_MODE_POE_DISABLED)) {
                // Turn port off in case that the allocated power exceeds the  max power set by user.
                if (power_used > poe_status->power_allocated[port_index]) {
                    if (poe_local_conf.poe_mode[port_index] != POE_MODE_POE_DISABLED) {
                        pd_overload[port_index]  = TRUE;
                        retry_cnt[port_index] = rand() % retry_cnt_max; // Set random time before "reenabling" poe
                    }
                } else {
                    pd_overload[port_index]  = FALSE;
                }
            } else {
                pd_overload[port_index]  = FALSE;
            }
        }
    }
    POE_CRIT_EXIT();
}


// Function for doing power management
static void poe_power_mgmt (poe_mgmt_mode_t mode)
{
    poe_status_t poe_status;
    poe_status_set_get(&poe_status, TRUE);
    T_DG(VTSS_TRACE_GRP_MGMT, "Enter poe_power_mgmt, mode :%d", mode);
    int port_list[VTSS_PORTS];
    BOOL port_on[VTSS_PORTS];

    POE_CRIT_ENTER();
    if (!poe_init_done) {
        POE_CRIT_EXIT();
        return;
    }
    POE_CRIT_EXIT();

    int total_power = 0;
    int power       = 0;
    int  port_list_index, port_index; // Must be signed because -1 is used to exit a for loop.

    poe_sort_after_prio(&port_list[0]); // Sort the port list with highest priority first.

    ushort power_source_value;
    if (poe_custom_get_power_source() == PRIMARY) {
        power_source_value = poe_local_conf.primary_power_supply;
    } else {
        power_source_value = poe_local_conf.backup_power_supply;
    }


    // Turn on ports starting with the highest priority, until the total power exceeds the power suppply
    for (port_list_index = 0; port_list_index < VTSS_PORTS; port_list_index ++ ) {
        port_index = port_list[port_list_index]; // Get port with highest priority.

        // Make sure that we don't get out of bounds (Should never happen).
        if (port_index >= VTSS_PORTS) {
            T_E("port index out of range - %d",  port_index);
            continue;
        }

        // No need to go on if the port doesn't support PoE
        if (poe_status.port_status[port_index] == POE_NOT_SUPPORTED) {
            continue;
        }

        T_DG_PORT(VTSS_TRACE_GRP_MGMT, (vtss_port_no_t)port_index, "poe_status.port_status =%d, power_source_value:%d",
                  poe_status.port_status[port_index], power_source_value);

        if ((mode == REQUESTED && poe_status.port_status[port_index] != NO_PD_DETECTED ) ||
            (mode == ACTUAL && poe_status.port_status[port_index] == PD_ON)) {

            if (mode == ACTUAL) {
                power = poe_status.power_used[port_index];
            } else {
                power = poe_status.power_requested[port_index];
            }

            total_power += power; // Add the request power to the power budget.

            // Determine is more power is requested than the power supply can deliever.
            // Multiply with 10 because PD's power is in deciwatts while power supply is in watts
            if (total_power > (power_source_value * 10)) {
                port_on[port_index] = FALSE; // Turn port off to lower the power requested
                total_power -= power;    // Ok - Port turn off. Don't use the reuqested power for this port.
                T_DG_PORT(VTSS_TRACE_GRP_MGMT, (vtss_port_no_t)port_index, "Total_power = %u,power = %u, port off", total_power, power);
            } else {
                port_on[port_index] = TRUE;
                T_DG_PORT(VTSS_TRACE_GRP_MGMT, (vtss_port_no_t)port_index, "Total_power = %u, port on", total_power);
            }

        } else {
            port_on[port_index] = TRUE;
        }

        // Count down retry counter for determine when to try to turn on the PoE for the port agian.
        if (retry_cnt[port_index] > 0 && retry_cnt[port_index] <= retry_cnt_max) {
            retry_cnt[port_index] --;
            T_DG_PORT(VTSS_TRACE_GRP_MGMT, (vtss_port_no_t)port_index, "retry_cnt = %d", retry_cnt[port_index]);
        } else {
            retry_cnt[port_index] = 0;
        }

        if (port_on[port_index]) {
            // The port was turned off because it used to much power last time it was turned on.
            // Wait until retry counter has counted down before turning on the port agian.
            if (retry_cnt[port_index] == 0) {
                T_DG_PORT(VTSS_TRACE_GRP_MGMT, (vtss_port_no_t)port_index, "Turned on, power allocated:", poe_status.power_allocated[port_index]);
                if (poe_status.power_allocated[port_index] > 0 ) {
                    poe_custom_set_port_max_power(port_index, poe_status.power_allocated[port_index]);
                }
                POE_CRIT_ENTER();
                power_budget_exceeded[port_index]  = FALSE;
                POE_CRIT_EXIT();
            }
        } else {
            // OK - Port turned off.
            retry_cnt[port_index] = rand() % retry_cnt_max;
            T_DG_PORT(VTSS_TRACE_GRP_MGMT, (vtss_port_no_t)port_index, "Forced off");
            POE_CRIT_ENTER();
            if (poe_local_conf.poe_mode[port_index] != POE_MODE_POE_DISABLED) {
                power_budget_exceeded[port_index]  = TRUE;
            }
            POE_CRIT_EXIT();
        }
    }
    poe_update_pd_state(&poe_status); // Update PD enable/disable state
    poe_port_power_mgmt(&poe_status);
}




// Function for updating the hardware with the current configuration.
static void update_registers(void)
{
    if (!poe_init_done) {
        return;
    }

    vtss_port_no_t port_index;
    char classes[VTSS_PORTS];
    memset(classes, 0, VTSS_PORTS);

    POE_CRIT_ENTER();
    // Take a local copy for this function, because it in case that the I2C communication fails can take quite a
    // while before we returns from the configuration functions used below. If this case we can run into a thread deadlock
    // if we are still in a critical section.
    poe_local_conf_t configuration = poe_local_conf;
    POE_CRIT_EXIT();


    POE_CRIT_CUSTOM_ENTER();
    poe_custom_set_power_supply(configuration.primary_power_supply, configuration.backup_power_supply);


    int pd63000_chip;
    if (poe_custom_is_chip_found(0) == PD63000) {
        pd63000_chip = 1;
    } else {
        pd63000_chip = 0;
    }



    if (!pd63000_chip) {
        //We do the power management
        if (configuration.power_mgmt_mode == LLDPMED_RESERVED ||
            configuration.power_mgmt_mode == CLASS_RESERVED ||
            configuration.power_mgmt_mode == ALLOCATED_RESERVED ) {
            poe_power_mgmt(REQUESTED);
        } else {
            poe_power_mgmt(ACTUAL);
        }
    } else {
        poe_custom_pm(&poe_stack_cfg); // Update the power management registers
        // Enable/Disable PoE
        for (port_index = 0 ; port_index < VTSS_PORTS; port_index++ ) {
            poe_custom_port_enable(port_index,
                                   configuration.poe_mode[port_index] != POE_MODE_POE_DISABLED,
                                   configuration.poe_mode[port_index]); // Turn On the PD (unless PoE is disabled for this port)
        }
    }

    if (configuration.power_mgmt_mode == LLDPMED_CONSUMP ||
        configuration.power_mgmt_mode == LLDPMED_RESERVED ||
        configuration.power_mgmt_mode == CLASS_CONSUMP ||
        configuration.power_mgmt_mode == CLASS_RESERVED) {
        poe_custom_get_all_ports_classes(&classes[0]);
    }


    // Setup power management for the chip set
    for (port_index = 0 ; port_index < VTSS_PORTS; port_index++ ) {
        // port priority +  port power + Port enable/disable
        poe_custom_set_port_priority(port_index, configuration.priority[port_index]);

        if (configuration.power_mgmt_mode == LLDPMED_CONSUMP || configuration.power_mgmt_mode == LLDPMED_RESERVED ) {
#ifdef VTSS_SW_OPTION_LLDP
            if (lldp_remote_lldp_is_info_valid(port_index)) {
                lldp_u16_t requested_power = lldp_remote_get_requested_power(port_index, poe_local_conf.poe_mode[port_index]);
                T_RG_PORT(VTSS_TRACE_GRP_MGMT, port_index, "Max power set via lldp, power = %d",
                          requested_power);

                poe_custom_set_port_max_power(port_index, requested_power); //
            } else {
                poe_custom_set_port_max_power(port_index, class2power(classes[port_index], port_index)); //
            }
#endif // VTSS_SW_OPTION_LLDP
        } else if (configuration.power_mgmt_mode == CLASS_CONSUMP || configuration.power_mgmt_mode == CLASS_RESERVED ) {
            T_NG_PORT(VTSS_TRACE_GRP_MGMT, port_index, "power:%d, class:%d", class2power(classes[port_index], port_index), classes[port_index]);
            poe_custom_set_port_max_power(port_index, class2power(classes[port_index], port_index)); //
        } else {
            T_RG_PORT(VTSS_TRACE_GRP_MGMT, port_index, "Max power set, power = %d", configuration.max_port_power[port_index]);
            poe_custom_set_port_max_power(port_index, configuration.max_port_power[port_index]);
        }
    }

    POE_CRIT_CUSTOM_EXIT();
}

/**********************************************************************
** Flash Configuration
**********************************************************************/

// Store the current configuration in flash
static void poe_store_conf_in_flash(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    poe_flash_conf_t *flash_conf;
    ulong size;

    T_RG(VTSS_TRACE_GRP_CONF, "Storing configuration in flash");

    if ((flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_POE_CONF, &size)) != NULL) {
        if (size == sizeof(*flash_conf)) {
            POE_CRIT_ENTER();
            // Copy configuration stored in RAM to the flash variable.
            memcpy(flash_conf->stack_cfg.local_conf, poe_stack_cfg.local_conf, sizeof(poe_stack_cfg.local_conf));
            flash_conf->stack_cfg.master_conf.power_mgmt_mode = poe_local_conf.power_mgmt_mode;
            POE_CRIT_EXIT();
        } else {
            T_W("Could not store configuration - Size did not match");
        }
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_POE_CONF);
    } else {
        T_W("Could not store configuration");
    }
#else
    T_D("Not storing in conf; using ICFG");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

static void poe_flash_configuration_read(BOOL create)
{
    poe_flash_conf_t *flash_conf;
    ulong             size;
    BOOL              do_create;
    uint              port_index;

    T_RG(VTSS_TRACE_GRP_CONF, "Entering poe_flash_configuration_read");

    if (misc_conf_read_use()) {
        if ((flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_POE_CONF, &size)) == NULL || size != sizeof(*flash_conf)) {
            flash_conf = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_POE_CONF, sizeof(*flash_conf));
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            do_create = TRUE;
        } else if (flash_conf->version != POE_CONF_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        flash_conf = NULL;
        do_create  = 1;
    }

    POE_CRIT_ENTER();
    if (do_create) {
        int sid;

        // Set default configuration
        T_NG(VTSS_TRACE_GRP_CONF, "Restoring to default value");

        // First reset whole structure...
        memset(&poe_stack_cfg, 0, sizeof(poe_stack_cfg));

        // Set the individual fields that need to be something other than 0
        for (sid = 0; sid < VTSS_ISID_END; sid++) {
            poe_stack_cfg.local_conf[sid].primary_power_supply = POE_POWER_SUPPLY_MAX;

            // If backup power supported then set a default value, else it is let to 0 W.
            if (poe_custom_is_backup_power_supported()) {
                poe_stack_cfg.local_conf[sid].backup_power_supply = POE_POWER_SUPPLY_MAX;
            }

            // Set default PoE for all ports
            for (port_index = 0; port_index < VTSS_PORTS; port_index++) {
                poe_stack_cfg.local_conf[sid].max_port_power[port_index] = POE_MAX_POWER_DEFAULT;
                poe_stack_cfg.local_conf[sid].poe_mode[port_index] = POE_MODE_DEFAULT;
                poe_stack_cfg.local_conf[sid].priority[port_index] = POE_PRIORITY_DEFAULT;
            }
        }
        poe_stack_cfg.master_conf.power_mgmt_mode = POE_MGMT_MODE_DEFAULT;

        if (flash_conf) {
            // Prepare for closing/saving configuration
            flash_conf->version = POE_CONF_VERSION;
            flash_conf->stack_cfg = poe_stack_cfg;
        }
    } else {
        if (flash_conf != NULL) {
            poe_stack_cfg = flash_conf->stack_cfg;
        }
    }

    memcpy(&poe_local_conf, &poe_stack_cfg.local_conf[0], sizeof(poe_local_conf)); // For now, this is a fix for silent upgrade race conditional #Bugzilla#11666

    poe_local_conf.power_mgmt_mode = poe_stack_cfg.master_conf.power_mgmt_mode;
    T_DG(VTSS_TRACE_GRP_CONF, "mode b = %d", poe_local_conf.power_mgmt_mode);
    POE_CRIT_EXIT();

    // Transfer configuration to affected slaves, if we're called with
    // #create == TRUE, since that's a restore to defaults, whereas
    // #create == FALSE is a MASTER_UP event, where we don't send anything.
    if (create) {
        poe_update_slave_conf(VTSS_ISID_GLOBAL);
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Write our settings back to flash
    if (flash_conf) {
        T_NG(VTSS_TRACE_GRP_CONF, "Closing CONF_BLK_POE_CONF");
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_POE_CONF);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/**********************************************************************
** Thread
**********************************************************************/

#if VTSS_SWITCH_MANAGED
static cyg_handle_t poe_thread_handle;
static cyg_thread   poe_thread_block;
static char poe_thread_stack[2 * (THREAD_DEFAULT_STACK_SIZE)];
#endif


static void poe_thread(cyg_addrword_t data)
{

    T_N("Entering poe_thread");

    T_N("poe_custom_init");
    POE_CRIT_CUSTOM_ENTER();
    poe_custom_init(); // Detecting PoE chip set.
    POE_CRIT_CUSTOM_EXIT();

    interrupt_poe_init_done(); // Signal to the interrupt module that it can enable the interrupt used by poe.
#ifdef VTSS_SW_OPTION_VCLI
    poe_cli_txt_init(); // Update the cli text so the commands reflects the PoE chipset.
#endif
    for (;;) {
        VTSS_OS_MSLEEP(1000); // Allow other threads to get access

        // Do thread stuff
        update_registers();
        POE_CRIT_CUSTOM_ENTER();
        poe_status_update();
        POE_CRIT_CUSTOM_EXIT();
        POE_CRIT_ENTER();
        poe_init_done = TRUE; // Signal that PoE chipset has been initialized.
        POE_CRIT_EXIT();
    }
}

/**********************************************************************
** Management functions
**********************************************************************/
// Function for getting which PoE chipset is found from outside this file (In order to do semaphore protection)
poe_chipset_t poe_is_chip_found(vtss_port_no_t port_index)
{
    poe_chipset_t chipset;
    POE_CRIT_CUSTOM_ENTER();
    chipset = poe_custom_is_chip_found(port_index);
    POE_CRIT_CUSTOM_EXIT();
    return chipset;
}

// Debug function for getting the PoE chipset firmware
i8 *poe_mgmt_firmware_info_get(vtss_port_no_t port_index, i8 *firmware_string)
{
    POE_CRIT_CUSTOM_ENTER();
    firmware_string = poe_custom_firmware_info_get(port_index, firmware_string);
    POE_CRIT_CUSTOM_EXIT();
    return firmware_string;
}


// Debug function for updating the PoE chipset firmware
void poe_mgmt_firmware_update(u8 *firmware, u32 firmware_size)
{
    // Updating firmware can take long time - Set crit region time high.
    int crit_custom_max_lock_time_orig  = crit_custom.max_lock_time;
    POE_CRIT_CUSTOM_ENTER();
    crit_custom.max_lock_time = 60000;
    POE_CRIT_CUSTOM_EXIT();

    // Enter crit region with new max lock time.
    POE_CRIT_CUSTOM_ENTER();
    poe_download_firmware(firmware, firmware_size);

    // Set the crit region timeout back to the original value
    crit_custom.max_lock_time = crit_custom_max_lock_time_orig;
    POE_CRIT_CUSTOM_EXIT();
}

// Debug function for setting legacy capacitor detection (Critical region protected) (***NO stacking support)
// In : port_index : Port number starting from 0
//      enable : TRUE to enable legacy capacitor detection
void poe_mgmt_capacitor_detection_set(vtss_port_no_t port_index, BOOL enable)
{
    POE_CRIT_CUSTOM_ENTER();
    poe_custom_capacitor_detection_set(port_index, enable);
    POE_CRIT_CUSTOM_EXIT();
}

// Debug function for getting legacy capacitor detection (Critical region protected) (***NO stacking support)
// In : port_index : Port number starting from 0
BOOL poe_mgmt_capacitor_detection_get(vtss_port_no_t port_index)
{
    BOOL enable;
    POE_CRIT_CUSTOM_ENTER();
    enable = poe_custom_capacitor_detection_get(port_index);
    POE_CRIT_CUSTOM_EXIT();
    return enable;
}


// Reading PoE chipset register (Critical region protected) (***NO stacking support)
void poe_mgmt_reg_rd(vtss_port_no_t port_index, u16 addr, u16 *data)
{
    POE_CRIT_ENTER();
    poe_custom_rd(port_index, addr, data);
    POE_CRIT_EXIT();
}

// Write PoE chipset register (Critical region protected) (***NO stacking support)
void poe_mgmt_reg_wr(vtss_port_no_t port_index, u16 addr, u16 data)
{
    POE_CRIT_ENTER();
    poe_custom_wr(port_index, addr, data);
    POE_CRIT_EXIT();
}



BOOL poe_mgmt_is_backup_power_supported(void)
{
    BOOL result;
    POE_CRIT_ENTER();
    result = poe_custom_is_backup_power_supported();
    POE_CRIT_EXIT();
    return result;
}


// Function for getting if PoE chipset is found (Critical region protected)
void poe_mgmt_is_chip_found(vtss_isid_t isid, poe_chipset_t *chip_found)
{
    poe_status_t local_poe_status;
    poe_mgmt_get_status(isid, &local_poe_status);
    memcpy(chip_found, local_poe_status.poe_chip_found, sizeof(local_poe_status.poe_chip_found));
}


// Function that returns the current configuration for a local switch
void poe_mgmt_get_local_config(poe_local_conf_t *conf, vtss_isid_t isid)
{
    T_NG(VTSS_TRACE_GRP_CONF, "Getting local conf");
    POE_CRIT_ENTER();

    if (isid == VTSS_ISID_LOCAL) {
        T_DG(VTSS_TRACE_GRP_CONF, "Getting local conf");
        *conf = poe_local_conf; // Return this switch configuration
    } else if (!VTSS_ISID_LEGAL(isid)) {
        T_W("isid:%d not legal", isid);
        *conf = poe_local_conf; // Return this switch configuration
    } else {
        // Return another switch's configuration.
        *conf = poe_stack_cfg.local_conf[isid];
        T_DG(VTSS_TRACE_GRP_CONF, "Getting global conf isid = %d", isid);
    }

    POE_CRIT_EXIT();
}


// Function that can set the current configuration for a local switch
void poe_mgmt_set_local_config(poe_local_conf_t *new_conf, vtss_isid_t isid)
{
    vtss_port_no_t port_index;
    BOOL new_conf_valid = TRUE; // signaling if the new configuration is valid.
    T_DG(VTSS_TRACE_GRP_CONF, "Setting local conf - isid = %d", isid);

    if (msg_switch_is_master()) {
        for (port_index = 0 ; port_index < VTSS_PORTS; port_index++ ) {
            if (new_conf->poe_mode[port_index] == POE_MODE_POE &&  new_conf->max_port_power[port_index] > 154 ) {
                T_E("Maximum port power must not exceed 15.4W when in mode is set to PoE (For port %u)", iport2uport(port_index));
                new_conf_valid = FALSE;
            }
        }

        // Update new configuration, store new configuration in flash and update the switch in question
        if (new_conf_valid) {
            POE_CRIT_ENTER();
            poe_stack_cfg.local_conf[isid] = *new_conf;
            POE_CRIT_EXIT();
            poe_store_conf_in_flash();
            poe_update_slave_conf(isid);
        } else {
            T_W("Configuration NOT updated due to above error(s)");
        }
    } else {
        T_W("Configuration can only be changed from the master switch, configuration NOT updated");
    }
}


// Function that returns specific master configurations
void poe_mgmt_get_master_config(poe_master_conf_t *conf)
{
    POE_CRIT_ENTER();
    poe_stack_cfg.master_conf.power_mgmt_mode = poe_local_conf.power_mgmt_mode;
    *conf = poe_stack_cfg.master_conf; // Return the master configuration
    POE_CRIT_EXIT();
}


// Function that sets specific master configurations
void poe_mgmt_set_master_config(poe_master_conf_t *new_conf)
{
    T_DG(VTSS_TRACE_GRP_CONF, "Setting master conf");

    if (msg_switch_is_master()) {
        T_RG(VTSS_TRACE_GRP_CONF, "Setting master conf");
        // Update new configuration, store new conf in flash
        POE_CRIT_ENTER();
        poe_stack_cfg.master_conf = *new_conf;
        poe_local_conf.power_mgmt_mode = poe_stack_cfg.master_conf.power_mgmt_mode;
        POE_CRIT_EXIT();
        poe_store_conf_in_flash();
    } else {
        T_W("Configuration can only be changed from the master switch, configuration NOT updated");
    }

}

// Getting status for mangement
void poe_mgmt_get_status(vtss_isid_t isid, poe_status_t *local_poe_status)
{
    /* Send request and wait for response */
    poe_msg_req_t   *msg;
    cyg_flag_value_t flag;

    T_DG(VTSS_TRACE_GRP_STATUS, "Enter poe_mgmt_get_status isid:%d", isid);

    if (!poe_init_done) {
        memset(local_poe_status, 0, sizeof(poe_status_t));
        return;
    }

    if (isid == VTSS_ISID_LOCAL || !msg_switch_is_master()) {
        poe_status_set_get(local_poe_status, TRUE);
    } else if (!VTSS_ISID_LEGAL(isid)) {
        T_W("isid:%d not legal", isid);
        poe_status_set_get(local_poe_status, TRUE); // Return this switch configuration
    } else {
        if (msg_switch_is_master()) {
            // Send the request for getting PoE status
            if (msg_switch_exists(isid)) {
                // Setup sync flag.
                flag = (1 << isid);
                cyg_flag_maskbits(&poe_msg_id_get_stat_req_flags, ~flag);

                msg = poe_msg_req_alloc(POE_MSG_ID_GET_STAT_REQ);
                T_DG(VTSS_TRACE_GRP_MSG, "Transmitting message POE_MSG_ID_GET_STAT_REQ to isid %d", isid);
                poe_msg_tx(msg, isid, 0);

                T_DG(VTSS_TRACE_GRP_CONF, "Starting timeout counter");
                if (cyg_flag_timed_wait(&poe_msg_id_get_stat_req_flags, flag, CYG_FLAG_WAITMODE_OR, POE_TIMEOUT) & flag ? 0 : 1) {
                    T_W("PoE status timeout - Status values not valid");
                    memset(local_poe_status, 0, sizeof(poe_status_t));
                } else {
                    POE_CRIT_ENTER();
                    memcpy(local_poe_status, &poe_status_global, sizeof(poe_status_t));
                    POE_CRIT_EXIT();
                }
            } else {
                memset(local_poe_status, 0, sizeof(poe_status_t));
            }
        } else {
            T_W("Must be master");
        }
    }
}

// Management function for getting PoE port status.
// In     : isid       - ISID number
// In/Out : poe_status - Pointer to where to put the Port status.
void poe_mgmt_get_port_status(vtss_isid_t isid, poe_port_status_t *port_status)
{
    if (!poe_init_done) {
        memset(port_status, 0, sizeof(*port_status));
        return;
    }

    poe_status_t local_poe_status;
    poe_mgmt_get_status(isid, &local_poe_status);
    memcpy(port_status, local_poe_status.port_status, sizeof(local_poe_status.port_status));
}

// Management function for getting PoE PD classes.
// In     : isid       - ISID number
// In/Out : poe_status - Pointer to where to put the Port status.
void poe_mgmt_get_pd_classes(vtss_isid_t isid, char *classes)
{
    /* Send request and wait for response */
    poe_msg_req_t   *msg;
    cyg_flag_value_t flag;

    if (!poe_init_done) {
        return;
    }

    if (isid == VTSS_ISID_LOCAL) {
        POE_CRIT_CUSTOM_ENTER();
        poe_get_all_pd_classes(classes);
        POE_CRIT_CUSTOM_EXIT();
    } else if (!VTSS_ISID_LEGAL(isid)) {
        T_W("isid:%d not legal", isid);
        POE_CRIT_CUSTOM_ENTER();
        poe_get_all_pd_classes(classes); // Return this switch's configuration
        POE_CRIT_CUSTOM_EXIT();
    } else {
        if (msg_switch_is_master()) {
            // Send the request for getting PoE status
            if (msg_switch_exists(isid)) {
                // Setup sync flag.
                flag = (1 << isid);

                cyg_flag_maskbits(&poe_msg_id_get_pd_classes_req_flags, ~flag);

                msg = poe_msg_req_alloc(POE_MSG_ID_GET_PD_CLASSES_REQ);
                T_DG(VTSS_TRACE_GRP_MSG, "Transmitting message POE_MSG_ID_GET_PD_CLASSES_REQ to isid %d", isid);
                poe_msg_tx(msg, isid, 0);

                T_DG(VTSS_TRACE_GRP_CONF, "Starting timeout counter");
                if (cyg_flag_timed_wait(&poe_msg_id_get_pd_classes_req_flags, flag, CYG_FLAG_WAITMODE_OR, POE_TIMEOUT) & flag ? 0 : 1) {
                    T_W("PoE status timeout - Status values not valid");
                    memset(classes, 0, sizeof(*classes));
                } else {
                    POE_CRIT_ENTER();
                    memcpy(&classes[0], &poe_status_global.pd_class[0], sizeof(*classes));
                    POE_CRIT_EXIT();
                }

            } else {
                memset(classes, 0, sizeof(*classes));
            }
        }
    }
}

// Function converting the port status to a printable string.
char *poe_status2str(poe_port_status_t status, vtss_port_no_t port_index, poe_local_conf_t *conf)
{
    if (!poe_init_done) {
        return "Detecting PoE chipset";
    } else if (status ==  POE_NOT_SUPPORTED) {
        return "PoE not available - No PoE chip found";
    } else if (conf->poe_mode[port_index] == POE_MODE_POE_DISABLED ) {
        return "PoE turned OFF - PoE disabled";
    } else {

        switch (status) {
        case POE_DISABLED :
            return "PoE turned OFF - PoE disabled";

        case POWER_BUDGET_EXCEEDED:
            return "PoE turned OFF - Power budget exceeded";

        case NO_PD_DETECTED:
            return "No PD detected";

        case PD_ON:
            return "PoE turned ON";

        case PD_OVERLOAD:
            return "PoE turned OFF - PD overload";

        case PD_OFF:
            return "PoE turned OFF";

        case UNKNOWN_STATE:
            return "Invalid PD";

        default :
            // This should never happen.
            T_W("Unknown PoE status:%d", status);
            return "PoE state unknown";
        }
    }
}

// Function converting the class to a printable string.
char *poe_class2str(const poe_status_t *status, vtss_port_no_t port_index, char *class_str)
{
    T_IG_PORT(VTSS_TRACE_GRP_STATUS, port_index, "port_status:%d", status->port_status[port_index]);
    if (status->port_status[port_index] == NO_PD_DETECTED ||
        status->port_status[port_index] == POE_NOT_SUPPORTED ||
        status->port_status[port_index] == POE_DISABLED ||
        status->pd_class[port_index] > 4) { // Only classes 0-4 is defined in the standard.
        strcpy(&class_str[0], "-");
    } else {
        sprintf(class_str, "%d", status->pd_class[port_index]);
    }

    return class_str;
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Initialize module */
vtss_rc poe_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid; // Get switch id


    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, VTSS_TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_I("INIT_CMD_INIT");

        critd_init(&crit, "PoE crit", VTSS_MODULE_ID_POE, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        POE_CRIT_EXIT();
        critd_init(&crit_custom, "PoE crit custom", VTSS_MODULE_ID_POE, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        POE_CRIT_CUSTOM_EXIT();

        critd_init(&crit_status, "PoE crit status", VTSS_MODULE_ID_POE, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        POE_CRIT_STATUS_EXIT();

#ifdef VTSS_SW_OPTION_VCLI
        poe_cli_init();
#endif

#ifdef VTSS_SW_OPTION_ICFG
        if (poe_icfg_init() != VTSS_RC_OK) {
            T_E("ICFG not initialized correctly");
        }
#endif
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          poe_thread,
                          0,
                          "POE",
                          poe_thread_stack,
                          sizeof(poe_thread_stack),
                          &poe_thread_handle,
                          &poe_thread_block);
        T_I("INIT_CMD_INIT");
        break;
    case INIT_CMD_START:
        T_I("INIT_CMD_START");
        poe_msg_init(); // Init. message functions
        T_I("INIT_CMD_START LEAVE");
        break;
    case INIT_CMD_CONF_DEF:
        T_I("RESTORE TO DEFAULT");
        // We don't support per-switch restore of defaults.
        if (isid == VTSS_ISID_GLOBAL) {
            poe_flash_configuration_read(TRUE); // Restore to default.
        }
        T_I("INIT_CMD_CONF_DEF LEAVE");
        break;
    case INIT_CMD_MASTER_UP:
        T_I("MASTER UP");
        poe_flash_configuration_read(FALSE); // Read the configuration from flash.
        if ((vtss_board_features() & VTSS_BOARD_FEATURE_POE)) {
            /* Only start thread if we actually are POE-capable */
            cyg_thread_resume(poe_thread_handle);
        }
        T_I("MASTER UP LEAVE");
        break;
    case INIT_CMD_MASTER_DOWN:
        T_I("MASTER DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_I("INIT_CMD_SWITCH_ADD");
        poe_update_slave_conf(isid);
        T_I("INIT_CMD_SWITCH_ADD POE DONE");
        break;
    case INIT_CMD_SWITCH_DEL:
        T_I("INIT_CMD_SWITCH_DEL");
        break;
    default:
        break;
    }

    return 0;

}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
