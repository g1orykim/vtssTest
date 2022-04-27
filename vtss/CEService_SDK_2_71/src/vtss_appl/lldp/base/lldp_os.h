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


/******************************************************************************
 * This header file defines types and macros that are OS and application      *
 * specific.                                                                  *
 ******************************************************************************/

#ifndef LLDP_OS_H
#define LLDP_OS_H


//
// Types and macros for Vitesse Semiconductors ECOS platform
//

#include "lldp_basic_types.h"
#include "vtss_common_os.h"
#include "port_api.h"
#include "main.h"
#include "lldp_tlv.h"

#define LLDP_FALSE 0
#define LLDP_TRUE  1


#ifdef VTSS_SW_OPTION_LLDP_MED
#include "lldpmed_shared.h"
#endif

// Macro for returning an unique port id for all ports in a stacking solution.
#define LLDP_GET_UNIQUE_PORT_ID(iport) lldp_mgmt_get_unique_port_id(iport)


typedef enum {
    LLDP_DISABLED,
    LLDP_ENABLED_RX_TX,
    LLDP_ENABLED_TX_ONLY,
    LLDP_ENABLED_RX_ONLY,
} lldp_admin_state_t;

lldp_timer_t lldp_os_get_msg_tx_interval (void);
lldp_timer_t lldp_os_get_msg_tx_hold (void);
lldp_timer_t lldp_os_get_reinit_delay (void);
lldp_timer_t lldp_os_get_tx_delay (void);

void lldp_os_set_msg_tx_interval (lldp_timer_t val);
void lldp_os_set_msg_tx_hold (lldp_timer_t val);
void lldp_os_set_reinit_delay (lldp_timer_t val);
void lldp_os_set_tx_delay (lldp_timer_t val);

lldp_u8_t *lldp_os_get_frame_storage (lldp_bool_t init);
lldp_admin_state_t lldp_os_get_admin_status (lldp_port_t port);
void lldp_os_set_admin_status (lldp_port_t port, lldp_admin_state_t admin_state);
void lldp_os_tx_frame (lldp_port_t port_no, lldp_u8_t *frame, lldp_u16_t len);
void lldp_os_get_if_descr (lldp_port_t sid, lldp_port_t port, lldp_8_t *dest);
void lldp_os_get_system_name (lldp_8_t *dest);
void lldp_os_get_system_descr (lldp_8_t *dest);
void lldp_os_get_ip_address (lldp_u8_t *dest, lldp_u8_t port);

lldp_u8_t lldp_os_get_ip_enabled (void);
int  lldp_os_get_port_id(lldp_u8_t sid, lldp_u8_t port);
lldp_u8_t lldp_os_get_optional_tlv_enabled (lldp_tlv_t tlv, lldp_u8_t port, vtss_isid_t isid);
void lldp_os_print_remotemib (void);
void lldp_os_set_tx_in_progress (lldp_bool_t tx_in_progress);
int lldp_os_get_usid (void);
vtss_common_macaddr_t lldp_os_get_masters_mac (void);
lldp_u32_t lldp_os_get_sys_up_time (void);
void lldp_os_ip_ulong2char_array(lldp_u8_t *ip_char_array, ulong ip_ulong);

typedef uchar notification_ena_t[LLDP_PORTS]; // Enable/Disable of SNMP trap notification - See MIBs
typedef lldp_admin_state_t admin_state_t[LLDP_PORTS]; // TX/RX enable disable array for each port
typedef uchar optional_tlv_t[LLDP_PORTS]; // Enable/disable of the optional TLVs
typedef lldp_bool_t cdp_aware_t[LLDP_PORTS]; // Enable/disable of CDP awareness
typedef vtss_ipv4_t lldp_vtss_ipv4_t[LLDP_PORTS];

typedef struct {
    /* tlv_optionals_enabled uses bits 0-5 of the octet:
    ** Port Description:    bit 0
    ** System Name:         bit 1
    ** System Description:  bit 2
    ** System Capabilities: bit 3
    ** Management Address:  bit 4
    */

    lldp_u16_t       reInitDelay;
    lldp_u16_t       msgTxHold;
    lldp_u16_t       msgTxInterval;
    lldp_u16_t       txDelay;
    lldp_8_t         timingChanged; // set to one when reInitDelay,msgTxHold,msgTxInterval or txDelay have changed.

    /* interpretation of admin_state is as follows:
    ** (must match lldp_admin_state_t)
    ** 0 = disabled
    ** 1 = rx_tx
    ** 2 = tx
    ** 3 = rx only
    */
    admin_state_t    admin_state;
    optional_tlv_t   optional_tlv;
#ifdef VTSS_SW_OPTION_CDP
    cdp_aware_t      cdp_aware;
#endif
    vtss_common_macaddr_t mac_addr ; /* Master's mac address*/

#ifdef VTSS_SW_OPTION_LLDP_MED
    lldpmed_policies_table_t policies_table;  // Table with all policies
    lldpmed_ports_policies_t ports_policies; // The policies enabled for each port
    lldpmed_location_info_t location_info; // Location information
    lldp_u8_t medFastStartRepeatCount; // Fast Start Repeat Count, See TIA1057
    lldpmed_optional_tlv_t   lldpmed_optional_tlv; // Enable/disable of the LLDP-MED optional TLVs ( See TIA-1057, MIB LLDPXMEDPORTCONFIGTLVSTXENABLE).
#endif

} lldp_struc_0_t;

void lldp_os_set_optional_tlv(lldp_tlv_t tlv, lldp_u8_t enabled, lldp_struc_0_t *current_conf, lldp_u8_t port);

#ifndef LOW_BYTE
#define LOW_BYTE(v) ((lldp_u8_t) (v))
#endif
#ifndef HIGH_BYTE
#define HIGH_BYTE(v) ((lldp_u8_t) (((lldp_u16_t) (v)) >> 8))
#endif


//
// Define limits and default (only non-zero) for configuration settings
//

// Fast Start Repeat Count
#define FAST_START_REPEAT_COUNT_MIN 1      // TIA1057, medFastStartRepeatCount MIB 
#define FAST_START_REPEAT_COUNT_MAX 10     // TIA1057, medFastStartRepeatCount MIB 
#define FAST_START_REPEAT_COUNT_DEFAULT 4  // TIA1057, Section 12.1, bullet c)


// TX Delay
#define LLDP_TX_DELAY_MIN 1      // IEEE 802.1AB-2005, lldpTxDelay MIB
#define LLDP_TX_DELAY_MAX 8192   // IEEE 802.1AB-2005, lldpTxDelay MIB
#define LLDP_TX_DELAY_DEFAULT 2  // IEEE 802.1AB-2005, Section 10.5.3.3, bullet d)

// TX Hold
#define LLDP_TX_HOLD_MIN 2      // IEEE 802.1AB-2005, lldpMessageTxHoldMutiplier MIB (page 65)
#define LLDP_TX_HOLD_MAX 10     // IEEE 802.1AB-2005, lldpMessageTxHoldMutiplier MIB (page 65)
#define LLDP_TX_HOLD_DEFAULT 4  // IEEE 802.1AB-2005, Section 10.5.3.3, bullet a)

// Re-init
#define LLDP_REINIT_MIN 1       // IEEE 802.1AB-2005, lldpTxHold MIB (page 65)
#define LLDP_REINIT_MAX 10      // IEEE 802.1AB-2005, lldpReinitDelay MIB (page 65)
#define LLDP_REINIT_DEFAULT 2   // IEEE 802.1AB-2005, Section 10.5.3.3, bullet c)

// Tx interval
#define LLDP_TX_INTERVAL_MIN 5       // IEEE 802.1AB-2005, lldpMessageTxInterval MIB (page 65)
#define LLDP_TX_INTERVAL_MAX 32768   // IEEE 802.1AB-2005, lldpMessageTxInterval MIB (page 65)
#define LLDP_TX_INTERVAL_DEFAULT 30  // IEEE 802.1AB-2005, Section 10.5.3.3, bullet b)


#ifdef VTSS_SW_OPTION_LLDP_MED
#define LLDP_ADMIN_STATE_DEFAULT LLDP_ENABLED_RX_TX // Section 8, TIA1057, Bullet b.1
#else
// For LLDP default enabled is only recommended (for LLDP-MED is "shall") (Section 10.5.1 Bullet 1. in 802.1AB-20005)
// but this were discussed when implemented and decided to have it disabled because we like to have all protocols disabled as default.
#define LLDP_ADMIN_STATE_DEFAULT LLDP_DISABLED
#endif // VTSS_SW_OPTION_LLDP_MED

// Use for silent upgrade from release 2.80
typedef struct {
    /* tlv_optionals_enabled uses bits 0-5 of the octet:
    ** Port Description:    bit 0
    ** System Name:         bit 1
    ** System Description:  bit 2
    ** System Capabilities: bit 3
    ** Management Address:  bit 4
    */
    lldp_u16_t       reInitDelay;
    lldp_u16_t       msgTxHold;
    lldp_u16_t       msgTxInterval;
    lldp_u16_t       txDelay;
    lldp_8_t         timingChanged; // set to one when reInitDelay,msgTxHold,msgTxInterval or txDelay have changed.

    /* interpretation of admin_state is as follows:
    ** (must match lldp_admin_state_t)
    ** 0 = disabled
    ** 1 = rx_tx
    ** 2 = tx
    ** 3 = rx only
    */
    admin_state_t    admin_state;
    optional_tlv_t   optional_tlv;
#ifdef VTSS_SW_OPTION_CDP
    cdp_aware_t      cdp_aware;
#endif
    vtss_ipv4_t      ipv4_addr;
    int              usid; // User switch id, used for stacking solutions
    int              isid; // Internal switch id, used for stacking solutions
    vtss_common_macaddr_t mac_addr ; /* Master's mac address*/

#ifdef VTSS_SW_OPTION_LLDP_MED
    lldpmed_policies_table_t policies_table;  // Table with all policies
    lldpmed_ports_policies_t ports_policies; // The policies enabled for each port
    lldpmed_location_info_t location_info; // Location information
    lldp_u8_t medFastStartRepeatCount; // Fast Start Repeat Count, See TIA1057
    lldpmed_optional_tlv_t   lldpmed_optional_tlv; // Enable/disable of the LLDP-MED optional TLVs ( See TIA-1057, MIB LLDPXMEDPORTCONFIGTLVSTXENABLE).
#endif
} lldp_struc_0_280_t;
#endif

