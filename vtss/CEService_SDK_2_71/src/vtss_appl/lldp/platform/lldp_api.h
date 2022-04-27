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

#ifndef _LLDP_API_H_
#define _LLDP_API_H_

#include "msg_api.h"
#include "vtss_lldp.h"
#include "lldp_remote.h"


//
// Callback. Used to get called back when an entry is updated.
//
typedef void (*lldp_callback_t)(vtss_port_no_t port_no, lldp_remote_entry_t *entry);

//
// Defintion of rc errors - See lldp_error_txt in lldp.c
//
enum {
    LLDP_ERROR_ISID = MODULE_ERROR_START(VTSS_MODULE_ID_LLDP),
    LLDP_ERROR_FLASH,
    LLDP_ERROR_SLAVE,
    LLDP_ERROR_NOT_MASTER,
    LLDP_ERROR_ISID_LOCAL,
    LLDP_ERROR_VOICE_VLAN_CONFIGURATION_MISMATCH,
    LLDP_ERROR_TX_DELAY,
    LLDP_ERROR_NOTIFICATION_VALUE,
    LLDP_ERROR_NOTIFICATION_INTERVAL_VALUE,
    LLDP_ERROR_NULL_POINTER,
};
char *lldp_error_txt(vtss_rc rc);

/* Master's configuration for a 2.80 release - Used for silent upgrade*/
typedef struct {
    lldp_struc_0_280_t         conf;                                         // Configuration that shall be known by all switches
    unsigned long              version;                                      // Block version
    admin_state_t              admin_states[VTSS_ISID_END];                  /* lldp_struc_0_t does also contain this information for a single switch, but because it is re-used code,
                                                                                we will live with that we "waste" some RAM. Needed because we wants to store configuration in flash. */
#ifdef VTSS_SW_OPTION_CDP
    cdp_aware_t                cdp_aware[VTSS_ISID_END];
#endif
    optional_tlv_t             optional_tlvs[VTSS_ISID_END];                 // Enable/disable of optional for all swithes in the stack.
    notification_ena_t         snmp_notification_ena[VTSS_ISID_END];         // Enable/disable of SNMP Trap notification (per port)
    int                        snmp_notification_interval;                   // Max one snmp trap with this interval. See the LLDP MIB for further information.
#ifdef VTSS_SW_OPTION_LLDP_MED
    lldpmed_notification_ena_t lldpmed_snmp_notification_ena[VTSS_ISID_END]; // Enable/Disable of SNMP Trap notification (per port)
    lldpmed_ports_policies_t   lldpmed_port_policies[VTSS_ISID_END];
#endif
} lldp_master_conf_280_t; // Configuration used in 2.80 release (for silent upgrade)

// Define the size of the entry table.
#define LLDP_ENTRIES_TABLE_SIZE sizeof(lldp_remote_entry_t) *  LLDP_REMOTE_ENTRIES

/************************/
/* Configuration         */
/************************/
// Port configuration that is stored in flash
typedef struct {
    admin_state_t              admin_states;                  /* lldp_struc_0_t does also contain this information for a single switch, but because it is re-used code,
                                                                                we will live with that we "waste" some RAM. Needed because we wants to store configuration in flash. */
#ifdef VTSS_SW_OPTION_CDP
    cdp_aware_t                cdp_aware;                     // TRUE if we shall convert received CDP frame to LLDP information
#endif
    optional_tlv_t             optional_tlvs;                 // Enable/disable of optional for all swithes in the stack.
    notification_ena_t         snmp_notification_ena;         // Enable/disable of SNMP Trap notification (per port)

#ifdef VTSS_SW_OPTION_LLDP_MED
    lldpmed_notification_ena_t lldpmed_snmp_notification_ena; // Enable/Disable of SNMP Trap notification (per port)
#endif
} lldp_port_conf_t;


// Common configuration that is stored in flash
typedef struct {
    lldp_u16_t       reInitDelay;   //IEEE 802.1AB-2005, Section 10.5.3.3, bullet c.
    lldp_u16_t       msgTxHold;     //IEEE 802.1AB-2005, Section 10.5.3.3, bullet a.
    lldp_u16_t       msgTxInterval; //IEEE 802.1AB-2005, Section 10.5.3.3, bullet b.
    lldp_u16_t       txDelay;       //IEEE 802.1AB-2005, Section 10.5.3.3, bullet d.
#ifdef VTSS_SW_OPTION_CDP
    cdp_aware_t      cdp_aware;
#endif
#ifdef VTSS_SW_OPTION_LLDP_MED
    lldpmed_policies_table_t policies_table;  // Table with all policies
    lldpmed_ports_policies_t ports_policies; // The policies enabled for each port
    lldpmed_location_info_t  location_info; // Location information
    lldpmed_optional_tlv_t   lldpmed_optional_tlv; // Enable/disable of the LLDP-MED optional TLVs ( See TIA-1057, MIB LLDPXMEDPORTCONFIGTLVSTXENABLE).
    lldp_u8_t medFastStartRepeatCount; // Fast Start Repeat Count, See TIA1057
#endif
    int  snmp_notification_interval;        // Max one snmp trap with this interval. See the LLDP MIB for further information.
} lldp_common_conf_t;


// Common configuration that is found at run-time.
typedef struct {
    lldp_8_t         timingChanged; // set to one when reInitDelay,msgTxHold,msgTxInterval or txDelay have changed.
    vtss_common_macaddr_t mac_addr ; /* Master's mac address*/
} lldp_conf_run_time_t;


/* Master's configuration that is stored in flash*/
typedef struct {
    unsigned long              version;     // Block version
    lldp_common_conf_t         common;      // Configuration that is common for all switches in a stack
    lldp_port_conf_t           port[VTSS_ISID_END]; // Configuration for each port.
} lldp_master_flash_conf_t;

/* Configuration that comes from flash for a single switch*/
typedef struct {
    lldp_common_conf_t         common;      // Configuration that is common for all switches in a stack
    lldp_port_conf_t           port;        // Configuration for each port.
} lldp_switch_flash_conf_t;

//
// Functions
//
// Function for getting an unique port id for each port in a stack.
// In : iport - Internal port number (starting from 0)
// Return : Unique port id
vtss_port_no_t       lldp_mgmt_get_unique_port_id(vtss_port_no_t iport);
vtss_rc              lldp_init(vtss_init_data_t *data);
void                 lldp_get_config(lldp_struc_0_t *conf, vtss_isid_t isid);
void                 lldp_mgmt_get_config(lldp_struc_0_t *conf, vtss_isid_t isid);
vtss_rc              lldp_mgmt_set_config(lldp_struc_0_t *conf, vtss_isid_t isid);
vtss_rc              lldp_mgmt_set_admin_state(const admin_state_t admin_state , vtss_isid_t isid);
vtss_rc              lldp_mgmt_set_optional_tlvs(optional_tlv_t optional_tlv , vtss_isid_t isid);
void                 lldp_send_frame(lldp_port_t port_no, lldp_u8_t *frame, lldp_u16_t len);
lldp_remote_entry_t *lldp_mgmt_get_entries(vtss_isid_t isid);
void                 lldp_mgmt_last_change_ago_to_str(time_t last_change_ago, char *last_change_str);
vtss_rc              lldp_mgmt_stat_get(vtss_isid_t isid, lldp_counters_rec_t *stat_cnt, lldp_mib_stats_t *global_stat_cnt, time_t *last_change_ago);
void                 lldp_mgmt_stat_clr(vtss_isid_t isid);
void                 lldp_something_has_changed(void);
void                 lldp_mgmt_get_unlock(void);
void                 lldp_mgmt_get_lock(void);
lldp_u8_t            lldp_mgmt_get_opt_tlv_enabled(lldp_tlv_t tlv_t, lldp_u8_t port, vtss_isid_t isid );

// Function that must be called when ever an entry is created, deleted or modified
void lldp_entry_changed(lldp_remote_entry_t *entry);

int lldp_mgmt_get_notification_interval(BOOL crit_region_not_set);
vtss_rc lldp_mgmt_set_notification_interval(int notification_interval);
int lldp_mgmt_get_notification_ena(lldp_port_t port, vtss_isid_t isid, BOOL crit_region_not_set);
vtss_rc lldp_mgmt_set_notification_ena(int notification_ena, lldp_port_t port, vtss_isid_t isid);
int lldpmed_mgmt_get_notification_ena(lldp_port_t port, vtss_isid_t isid);
vtss_rc lldpmed_mgmt_set_notification_ena(int notification_ena, lldp_port_t port, vtss_isid_t isid);
vtss_rc lldp_mgmt_set_cdp_aware(cdp_aware_t cdp_aware , vtss_isid_t isid); // Function for setting cdp awareness configuration.

void lldp_mgmt_entry_updated_callback_register(lldp_callback_t cb);
void lldp_mgmt_entry_updated_callback_unregister(lldp_callback_t cb);

vtss_ipv4_t lldp_ip_addr_get(vtss_port_no_t iport);

#ifdef VTSS_SW_OPTION_LLDP_MED
lldp_u8_t lldp_mgmt_lldpmed_coordinate_location_tlv_add(lldp_u8_t *buf);
lldp_u8_t lldp_mgmt_lldpmed_civic_location_tlv_add(lldp_u8_t *buf);
lldp_u8_t lldp_mgmt_lldpmed_ecs_location_tlv_add(lldp_u8_t *buf);
#endif
#ifdef VTSS_SW_OPTION_LLDP_MED_DEBUG
void lldp_mgmt_set_med_transmit_var(lldp_port_t iport, lldp_bool_t tx_enable);
lldp_bool_t lldp_mgmt_get_med_transmit_var(lldp_port_t iport, lldp_bool_t tx_enable);
#endif // VTSS_SW_OPTION_LLDP_MED_DEBUG

vtss_isid_t lldp_local_usid_set_get(BOOL set, vtss_isid_t  new_usid);

vtss_rc lldp_mgmt_set_common_config(lldp_common_conf_t *common_conf); // See lldp.c
void    lldp_mgmt_get_common_config(lldp_common_conf_t *common_conf); // See lldp.c

#endif // _LLDP_API_H_

// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
