/*

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


#include "lldp_os.h"
#include "main.h"
#include "vtss_common_os.h"
#include "lldp_sm.h"
#include "lldp.h"
#include "lldp_private.h"
#include "misc_api.h"
#include "msg_api.h"

/* ************************************************************************ **
 *
 *
 * Defines
 *
 *
 *
 * ************************************************************************ */



/* ************************************************************************ **
 *
 *
 * Typedefs and enums
 *
 *
 *
 * ************************************************************************ */

/* ************************************************************************ **
 *
 *
 * Prototypes for local functions
 *
 *
 *
 * ************************************************************************ */

/* ************************************************************************ **
 *
 *
 * Public data
 *
 *
 *
 * ************************************************************************ */

/* ************************************************************************ **
 *
 *
 * Local data
 *
 *
 *
 * ************************************************************************ */
static lldp_bool_t use_tx_shutdown_buffer = LLDP_FALSE;
static lldp_u8_t tx_shutdown_buffer[64];
#define SHARED_BUFFER_SIZE 1518
lldp_u8_t shared_buffer [SHARED_BUFFER_SIZE];


/* from SNMP */
extern lldp_u32_t time_since_boot;


lldp_timer_t lldp_os_get_msg_tx_interval (void)
{
    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL);

    // According to the standard the msgmsgTxInterval shall not be below 5 sec. ( This should never happen since the cli code also should do this check. )
    if (conf.msgTxInterval < 5) {
        conf.msgTxInterval = 5;
    }
    return conf.msgTxInterval;

}

lldp_timer_t lldp_os_get_msg_tx_hold (void)
{
    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL);
    return conf.msgTxHold;
}

lldp_timer_t lldp_os_get_reinit_delay (void)
{
    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL);
    return conf.reInitDelay;
}

lldp_timer_t lldp_os_get_tx_delay (void)
{
    lldp_timer_t tx_interval = lldp_os_get_msg_tx_interval();

    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL);
    lldp_timer_t tx_delay = conf.txDelay;

    /* must not be larger that 0.25 * msgTxInterval*/
    if (tx_delay > (tx_interval / 4)) {
        return (tx_interval / 4);
    }
    return tx_delay;
}


lldp_admin_state_t lldp_os_get_admin_status (lldp_port_t port)
{
    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL);
    return conf.admin_state[port];
}


/* return some memory we can use to build up frame in */
lldp_u8_t *lldp_os_get_frame_storage (lldp_bool_t init)
{
    if (use_tx_shutdown_buffer) {
        T_R("Returning tx_shutdown_buffer");
        return tx_shutdown_buffer;
    }
    if (init) {
        memset(&shared_buffer[0], 0, SHARED_BUFFER_SIZE);
    }

    T_RG(TRACE_GRP_TX, "Returning shared_buffer");
    return shared_buffer;
}

void lldp_os_set_tx_in_progress (lldp_bool_t tx_in_progress)
{
    use_tx_shutdown_buffer = tx_in_progress;
}

void lldp_os_get_if_descr (lldp_port_t sid, lldp_port_t port, lldp_8_t   *dest)
{
    T_RG(TRACE_GRP_TX, "Entering lldp_os_get_if_descr");
    char if_descr[MAX_PORT_DESCR_LENGTH] = "" ;


#if VTSS_SWITCH_STACKABLE
    sprintf(if_descr, "Sid #%u, Port #%u", sid, port);
#else
    sprintf(if_descr, "Port #%u", port);
#endif
    memcpy(&dest[0], if_descr, strlen(if_descr) + 1 );// + 1 for getting the NULL pointer
}

void lldp_os_get_system_name (lldp_8_t   *dest)
{
    lldp_system_name(dest, 1); // lldp_os_get_system_name is the "old" function, so we pass it on to the "new" function lldp_get_system_name.
}

void lldp_os_get_system_descr (lldp_8_t   *dest)
{
    char          buf[MAX_SYSTEM_DESCR_LENGTH];
    strncpy(buf, misc_software_version_txt(), MAX_SYSTEM_DESCR_LENGTH - 1);
    strncat(buf, " ", MAX_SYSTEM_DESCR_LENGTH - strlen(buf) - 1);
    strncat(buf, misc_software_date_txt(), MAX_SYSTEM_DESCR_LENGTH - strlen(buf) - 1);
    buf[MAX_SYSTEM_DESCR_LENGTH - 1] = '\0';
    T_NG(TRACE_GRP_TX, "buf = %s", buf);

    strcpy(&dest[0], buf);
}

// Converting IP as ulong to a 4 bytes char array.
//
// In : ip_ulong - IP address as ulong type
//
// In/Out : ip_char_array - IP as a 4 bytes char array
//
void lldp_os_ip_ulong2char_array (lldp_u8_t *ip_char_array, ulong ip_ulong)
{
    memcpy(ip_char_array, &ip_ulong, 4);

    // Swap bytes
    lldp_u8_t temp;
    temp = *ip_char_array;
    *ip_char_array = *(ip_char_array + 3);
    *(ip_char_array + 3) = temp;

    temp = *(ip_char_array + 1) ;
    *(ip_char_array + 1) = *(ip_char_array + 2);
    *(ip_char_array + 2) = temp;
}

// Geting the IP address
void lldp_os_get_ip_address (lldp_u8_t *dest, lldp_u8_t port)
{
    lldp_os_ip_ulong2char_array(dest, lldp_ip_addr_get(port));
}

// Geting the USID
int lldp_os_get_usid (void)
{
    return lldp_local_usid_set_get(FALSE, 0);
}

// Geting the master max addr
vtss_common_macaddr_t lldp_os_get_masters_mac (void)
{
    // Get the master's MAC address from configuration
    lldp_struc_0_t conf;
    lldp_get_config(&conf, VTSS_ISID_LOCAL);

    return conf.mac_addr;
}

void lldp_os_set_optional_tlv (lldp_tlv_t tlv_t, lldp_u8_t enabled, lldp_struc_0_t *current_conf, lldp_u8_t port)
{
    lldp_u8_t tlv_enabled = current_conf->optional_tlv[port];
    lldp_u8_t tlv_u8 = (lldp_u8_t) tlv_t;
    /* adjust from TLV value to bits */
    tlv_u8 -= 4;
    if (enabled) {
        tlv_enabled = tlv_enabled |  (1 << tlv_u8);
        T_D("Set TLV %u to %u, tlv_enabled= %d", tlv_u8, (unsigned)enabled, tlv_enabled);
    } else {
        tlv_enabled = tlv_enabled &  ~((lldp_u8_t)(1 << tlv_u8));
    }

    current_conf->optional_tlv[port] = tlv_enabled;
    T_DG(TRACE_GRP_TX, "Optional TLV set - port = %d, tlv_enabled = %d", port, tlv_enabled);
}

lldp_u8_t lldp_os_get_optional_tlv_enabled (lldp_tlv_t tlv_t, lldp_u8_t port, vtss_isid_t isid )
{
    lldp_struc_0_t conf;
    lldp_u8_t tlv_u8 = (lldp_u8_t) tlv_t;
    lldp_get_config(&conf, isid);

    int tlv_enable = conf.optional_tlv[port];

    /* adjust from TLV value to bits */
    tlv_u8 -= 4;

    // return 1 if enable else 0
    return tlv_enable & 1 << tlv_u8;
}


// Return unique number for each port in the stack.
int lldp_os_get_port_id (lldp_u8_t usid, lldp_u8_t port)
{
    return (usid - 1) * LLDP_PORTS  + port;
}




/* send a frame on external port number */
void lldp_os_tx_frame (lldp_port_t port_no, lldp_u8_t *frame, lldp_u16_t len)
{
    lldp_send_frame(port_no, frame, len);
}

lldp_u32_t lldp_os_get_sys_up_time (void)
{
    T_DG(TRACE_GRP_RX, "Getting entry time to %s", misc_time2str(msg_uptime_get(VTSS_ISID_LOCAL)));
    return msg_uptime_get(VTSS_ISID_LOCAL); // FJ return time_since_boot;
}




