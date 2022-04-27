/*

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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


#ifndef LLDP_REMOTE_H
#define LLDP_REMOTE_H

#include "main.h"
#include "lldp_os.h"
#include "vtss_lldp.h"
#include "lldp_tlv.h"

#ifdef VTSS_SW_OPTION_POE
#include "poe_api.h"
#endif

#define MAX_CHASSIS_ID_LENGTH 255  // Figure 9-4 in IEEE802.1AB-2005
#define MAX_PORT_ID_LENGTH 255     // Figure 9-5 in IEEE802.1AB-2005
#define MAX_PORT_DESCR_LENGTH 255  // Figure 9-7 in IEEE802.1AB-2005
#define MAX_SYSTEM_NAME_LENGTH 255 // Figure 9-8 in IEEE802.1AB-2005
#define MAX_SYSTEM_DESCR_LENGTH 255 // Figure 9-9 in IEEE802.1AB-2005

#define MAX_LLDPMED_LOCATION_LENGTH 256 // Max Location ID data length in table 14,TIA1057
#define MAX_LLDPMED_INVENTORY_LENGTH 32 // Max inventory string length. See all inventory TLVs in TIA1057, Section 10.2.6)

// Mulitple policies (applications) TLVs shall be supported ( Section 10.2.3.1, TIA1057) so we have to define a
// number of Policy fields in the entry. This can be limited to the number of applications defined (Section 10.2.3.8, TIA1057).
#define MAX_LLDPMED_POLICY_APPLICATIONS_CNT 9  // Number of defined policy applications - Table 12 in TIA1057 


#define MAX_LLDPMED_LOCATION_CNT 3  // Number of (valid) defined locations TLVs within one LLDPDU - Table 14 in TIA1057 

#define MAX_MGMT_LENGTH 255
#define MAX_OID_LENGTH 128 // Figure 9-11 in IEEE802.1AB-2005
#define MAX_ORG_LENGTH 507 // See figure 9-12 in IEEE802.1-AB-2005

#ifdef VTSS_SW_OPTION_EEE
typedef struct {
    lldp_bool_t valid;          // TRUE when the rest of the EEE fields contains valid information
    lldp_u16_t  RemTxTwSys;     // See EEE TLV, Figure 79-5a,IEEE802.3az
    lldp_u16_t  RemRxTwSys;     // See EEE TLV, Figure 79-5a,IEEE802.3az
    lldp_u16_t  RemFbTwSys;     // See EEE TLV, Figure 79-5a,IEEE802.3az
    lldp_u16_t  RemTxTwSysEcho; // See EEE TLV, Figure 79-5a,IEEE802.3az
    lldp_u16_t  RemRxTwSysEcho; // See EEE TLV, Figure 79-5a,IEEE802.3az
} lldp_eee_t;
#endif

// IEEE 802.1AB-2005 Section 9.5.9.9 Bullet a), says that multiple management addresses is supported but not how many.
// For now we say multiple = 2.
#define LLDP_MGMT_ADDR_CNT 2
typedef struct {
    lldp_u8_t      subtype;
    lldp_u8_t      length;
    lldp_8_t       mgmt_address[31];
    lldp_u8_t      if_number_subtype;
    lldp_8_t       if_number[4];
    lldp_u8_t      oid_length;
    lldp_8_t       oid[MAX_OID_LENGTH];
} lldp_mgmt_addr_tlv_t;



typedef struct {
    /* to begin with, we have a number of fields needed for
    ** SNMP MIB purposes or for other management related
    ** purposes */
    lldp_port_t    receive_port;

    lldp_8_t       smac[6]; // Not part of the LLDP standard, but needed for Voice Vlan
    lldp_u16_t     lldp_remote_index;
    lldp_u32_t     time_mark;

    lldp_u8_t      in_use;

    /* The following fields are "data fields" with received data */
    lldp_u8_t      chassis_id_subtype;
    lldp_u8_t      chassis_id_length;
    lldp_8_t       chassis_id[MAX_CHASSIS_ID_LENGTH];

    lldp_u8_t      port_id_subtype;
    lldp_u8_t      port_id_length;
    lldp_8_t       port_id[MAX_PORT_ID_LENGTH];

    lldp_u8_t      port_description_length;
    lldp_8_t       port_description[MAX_PORT_DESCR_LENGTH];

    lldp_u8_t      system_name_length;
    lldp_8_t       system_name[MAX_SYSTEM_NAME_LENGTH];

    lldp_u8_t      system_description_length;
    lldp_8_t       system_description[MAX_SYSTEM_DESCR_LENGTH];

    lldp_u8_t      system_capabilities[4];

    lldp_mgmt_addr_tlv_t mgmt_addr[LLDP_MGMT_ADDR_CNT];
#ifdef VTSS_SW_OPTION_POE
    // organizationally TLV for PoE - See figure 33-26 in IEEE802.3at/D3
    lldp_u8_t      ieee_draft_version; // Because there are already equipment that supports earlier versions of the ieee standard,
    //we have to support multiple versions #!&&%"

    lldp_u8_t      poe_info_valid ; // Set to 1 if the PoE fields below contains valid information.
    lldp_u8_t      requested_type_source_prio;
    lldp_u16_t     requested_power;
    lldp_u8_t      actual_type_source_prio;
    lldp_u16_t     actual_power;

    lldp_u8_t      poe_mdi_power_support; // Figure 79-2, IEEE801.3at/D4
    lldp_u8_t      poe_pse_power_pair;    // Figure 79-2, IEEE801.3at/D4
    lldp_u8_t      poe_power_class;       // Figure 79-2, IEEE801.3at/D4
#endif

#ifdef VTSS_SW_OPTION_LLDP_ORG
    lldp_bool_t lldporg_autoneg_vld; // Set to TRUE when the fields below contains valid data.
    lldp_u8_t   lldporg_autoneg; //Autonegotiation Support/Status - Figure G-1, IEEE802.1AB
    lldp_u16_t  lldporg_autoneg_advertised_capa; // Figure G-1, IEEE802.1AB
    lldp_u16_t  lldporg_operational_mau_type; // Figure G-1, IEEE802.1AB
#endif // VTSS_SW_OPTION_LLDP_ORG


#ifdef VTSS_SW_OPTION_LLDP_MED
    lldp_bool_t lldpmed_info_vld; // Set to TRUE if any LLDP-MED TLV is received.



    // LLDP-MED Capabilites TLV - Figure 6, TIA1057
    lldp_bool_t lldpmed_capabilities_vld; // Set to TRUE when the fields below contains valid data.
    lldp_u16_t lldpmed_capabilities;
    lldp_u16_t lldpmed_capabilities_current; // See TIA1057, MIBS LLDPXMEDREMCAPCURRENT
    lldp_u8_t  lldpmed_device_type;



    // LLDP-MED Policy TLV - Figure 7,TIA1057
    lldp_bool_t       lldpmed_policy_vld[MAX_LLDPMED_POLICY_APPLICATIONS_CNT]; // Valid bit signaling that the corrensponding lldpmed_policy contains valid information
    lldp_u32_t lldpmed_policy[MAX_LLDPMED_POLICY_APPLICATIONS_CNT]; // Contains Application type, Policy flag, tag flag, VLAN Id and DSCP

    // LLDP-MED location TLV - Figure 8,TIA1057
    lldp_u16_t   lldpmed_location_length[MAX_LLDPMED_LOCATION_CNT];
    lldp_u16_t   lldpmed_location_format[MAX_LLDPMED_LOCATION_CNT];
    lldp_bool_t  lldpmed_location_vld[MAX_LLDPMED_LOCATION_CNT]; // Valid bit signaling that the corrensponding lldpmed_location contains valid information
    lldp_u8_t     lldpmed_location[MAX_LLDPMED_LOCATION_CNT][MAX_LLDPMED_LOCATION_LENGTH];


    // hardware revision TLV - Figure 13,  TIA1057
    lldp_u8_t    lldpmed_hw_rev_length;
    lldp_8_t     lldpmed_hw_rev[MAX_LLDPMED_INVENTORY_LENGTH];

    // firmware revision TLV - Figure 14,  TIA1057
    lldp_u8_t    lldpmed_firm_rev_length;
    lldp_8_t     lldpmed_firm_rev[MAX_LLDPMED_INVENTORY_LENGTH];

    // Software revision TLV - Figure 15,  TIA1057
    lldp_u8_t    lldpmed_sw_rev_length;
    lldp_8_t     lldpmed_sw_rev[MAX_LLDPMED_INVENTORY_LENGTH];

    // Serial number  TLV - Figure 16,  TIA1057
    lldp_u8_t    lldpmed_serial_no_length;
    lldp_8_t     lldpmed_serial_no[MAX_LLDPMED_INVENTORY_LENGTH];

    // Manufacturer name TLV - Figure 17,  TIA1057
    lldp_u8_t    lldpmed_manufacturer_name_length;
    lldp_8_t     lldpmed_manufacturer_name[MAX_LLDPMED_INVENTORY_LENGTH];

    // Model Name TLV - Figure 18,  TIA1057
    lldp_u8_t    lldpmed_model_name_length;
    lldp_8_t     lldpmed_model_name[MAX_LLDPMED_INVENTORY_LENGTH];

    // Asset ID TLV - Figure 19,  TIA1057
    lldp_u8_t    lldpmed_assert_id_length;
    lldp_8_t     lldpmed_assert_id[MAX_LLDPMED_INVENTORY_LENGTH];


#endif

    // TIA 1057
#ifdef VTSS_SW_OPTION_POE
    lldp_u8_t      tia_info_valid ;       // Set to 1 if the PoE fields below contains valid information.
    lldp_u8_t      tia_type_source_prio;  // For supporting TIA 1057
    lldp_u16_t     tia_power;             // For supporting TIA 1057
#endif

#ifdef VTSS_SW_OPTION_EEE
    lldp_eee_t eee;
#endif

    // General
    lldp_u16_t     rx_info_ttl;
    lldp_u8_t      something_changed_remote;
} lldp_remote_entry_t;

typedef struct {
    lldp_u8_t      subtype;
    lldp_u8_t      length;
    lldp_8_t   *mgmt_address;
    lldp_u8_t      if_number_subtype;
    lldp_8_t   *if_number;
    lldp_u8_t      oid_length;
    lldp_8_t   *oid;
} lldp_mgmt_addr_tlv_p_t;


/* like a remote entry, but with pointers into the received frame instead
** of arrays */
typedef struct {
    lldp_port_t    receive_port;

    lldp_8_t   *smac; // Not part of the LLDP standard, but needed for Voice Vlan


    /* The following fields are "data fields" with received data */
    lldp_u8_t      chassis_id_subtype;
    lldp_u8_t      chassis_id_length;
    lldp_8_t   *chassis_id;

    lldp_u8_t      port_id_subtype;
    lldp_u8_t      port_id_length;
    lldp_8_t   *port_id;

    lldp_u8_t      port_description_length;
    lldp_8_t   *port_description;

    lldp_u8_t      system_name_length;
    lldp_8_t   *system_name;

    lldp_u8_t      system_description_length;
    lldp_8_t   *system_description;

    lldp_8_t     system_capabilities[4];

    lldp_mgmt_addr_tlv_p_t mgmt_addr[LLDP_MGMT_ADDR_CNT];

    lldp_u16_t     ttl;
#ifdef VTSS_SW_OPTION_POE
    // organizationally TLV for PoE - See figure 33-26 in IEEE802.3at/D3
    lldp_u8_t      ieee_draft_version; // Because there are already equipment that supports earlier versions of the ieee standard,
    //we have to support multiple versions #!&&%"

    lldp_u8_t      poe_info_valid ; // Set to 1 if the PoE fields below contains valid information.
    lldp_u8_t      requested_type_source_prio;
    lldp_u16_t     requested_power;
    lldp_u8_t      actual_type_source_prio;
    lldp_u16_t     actual_power;

    lldp_u8_t      poe_mdi_power_support; // Figure 79-2, IEEE801.3at/D4
    lldp_u8_t      poe_pse_power_pair;    // Figure 79-2, IEEE801.3at/D4
    lldp_u8_t      poe_power_class;       // Figure 79-2, IEEE801.3at/D4
#endif

#ifdef VTSS_SW_OPTION_LLDP_ORG
    // Organizationally Specific TLVs
    lldp_bool_t lldporg_autoneg_vld; // Set to TRUE when the fields below contains valid data.
    lldp_u8_t   lldporg_autoneg; //Autonegotiation Support/Status - Figure G-1, IEEE802.1AB
    lldp_u16_t  lldporg_autoneg_advertised_capa; // Figure G-1, IEEE802.1AB
    lldp_u16_t  lldporg_operational_mau_type; // Figure G-1, IEEE802.1AB
#endif // VTSS_SW_OPTION_LLDP_ORG

#ifdef VTSS_SW_OPTION_LLDP_MED
    //
    // TIA 1057 - LLDP MED
    //
    lldp_bool_t lldpmed_info_vld; // Set to TRUE if any LLDP-MED TLV is received.

    // LLDP-MED Capabilites TLV - Figure 6, TIA1057
    lldp_bool_t lldpmed_capabilities_vld; // Set to TRUE when the fields below contains valid data.
    lldp_u16_t lldpmed_capabilities;
    lldp_u16_t lldpmed_capabilities_current; // See TIA1057, MIBS LLDPXMEDREMCAPCURRENT
    lldp_u8_t  lldpmed_device_type;


    // LLDP-MED Policy TLV - Figure 7,TIA1057
    lldp_bool_t lldpmed_policy_vld[MAX_LLDPMED_POLICY_APPLICATIONS_CNT]; // Valid bit signaling that the corrensponding lldpmed_policy contains valid information
    lldp_u32_t  lldpmed_policy[MAX_LLDPMED_POLICY_APPLICATIONS_CNT]; // Contains Application type, Policy flag, tag flag, VLAN Id and DSCP

    // LLDP-MED location TLV - Figure 8,TIA1057
    lldp_bool_t lldpmed_location_vld[MAX_LLDPMED_LOCATION_CNT]; // Valid bit signaling that the corrensponding lldpmed_location contains valid information
    lldp_u16_t  lldpmed_location_format[MAX_LLDPMED_LOCATION_CNT];
    lldp_u16_t  lldpmed_location_length[MAX_LLDPMED_LOCATION_CNT];
    lldp_8_t   *lldpmed_location[MAX_LLDPMED_LOCATION_CNT];

    // LLDP-MED hardware revision - Figure 13, TIA1057
    lldp_u8_t   lldpmed_hw_rev_length;
    lldp_8_t   *lldpmed_hw_rev;

    // firmware revision TLV - Figure 14,  TIA1057
    lldp_u8_t   lldpmed_firm_rev_length;
    lldp_8_t   *lldpmed_firm_rev;

    // Software revision TLV - Figure 15,  TIA1057
    lldp_u8_t   lldpmed_sw_rev_length;
    lldp_8_t   *lldpmed_sw_rev;

    // Serial number  TLV - Figure 16,  TIA1057
    lldp_u8_t   lldpmed_serial_no_length;
    lldp_8_t   *lldpmed_serial_no;

    // Manufacturer name TLV - Figure 17,  TIA1057
    lldp_u8_t   lldpmed_manufacturer_name_length;
    lldp_8_t   *lldpmed_manufacturer_name;

    // Model Name TLV - Figure 18,  TIA1057
    lldp_u8_t   lldpmed_model_name_length;
    lldp_8_t   *lldpmed_model_name;

    // Asset ID TLV - Figure 19,  TIA1057
    lldp_u8_t   lldpmed_assert_id_length;
    lldp_8_t   *lldpmed_assert_id;
#endif

#ifdef VTSS_SW_OPTION_POE
    // POE
    lldp_u8_t   tia_info_valid ;       // Set to 1 if the PoE fields below contains valid information.
    lldp_u8_t   tia_type_source_prio;  // For supporting TIA 1057
    lldp_u16_t  tia_power;             // For supporting TIA 1057
#endif

#ifdef VTSS_SW_OPTION_EEE
    lldp_eee_t eee;
#endif
} lldp_rx_remote_entry_t;

typedef struct {
    lldp_counter_t table_inserts;
    lldp_counter_t table_deletes;
    lldp_counter_t table_drops;
    lldp_counter_t table_ageouts;
} lldp_mib_stats_t;

void lldp_remote_delete_entries_for_local_port (lldp_port_t port);
lldp_bool_t lldp_remote_handle_msap (lldp_rx_remote_entry_t   *rx_entry);
lldp_u8_t lldp_remote_get_max_entries (void);
lldp_remote_entry_t   *lldp_get_remote_entry (lldp_u8_t idx);
void lldp_remote_1sec_timer (void);
void lldp_remote_tlv_to_string (lldp_remote_entry_t   *entry, lldp_tlv_t field, lldp_8_t *output_string, lldp_u8_t mgmt_addr_index);
void lldp_chassis_type_to_string (lldp_remote_entry_t   *entry, lldp_printf_t lldp_printf);
void lldp_port_type_to_string (lldp_remote_entry_t   *entry, lldp_printf_t lldp_printf);
void lldp_get_mib_stats (lldp_mib_stats_t   *stat);
lldp_remote_entry_t   *lldp_remote_get_next(lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx);
lldp_remote_entry_t   *lldp_remote_get_next_non_zero_addr(lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx, u8 mgmt_addr_index);
lldp_remote_entry_t   *lldp_remote_get(lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx);
void lldp_mgmt_address_to_string (lldp_remote_entry_t   *entry, lldp_u8_t   *dest);
void lldp_remote_chassis_id_to_string (lldp_remote_entry_t   *entry, char *string_ptr);
void lldp_remote_system_capa_to_string (lldp_remote_entry_t   *entry, char *string_ptr);
void lldp_remote_mgmt_addr_to_string (lldp_remote_entry_t   *entry, char *string_ptr, lldp_bool_t mgmt_addr_only,  u8 mgmt_addr_index);
void lldp_remote_system_name_to_string (lldp_remote_entry_t   *entry, char *string_ptr);
void lldp_remote_port_descr_to_string (lldp_remote_entry_t   *entry, char *string_ptr);
void lldp_remote_system_descr_to_string (lldp_remote_entry_t   *entry, char *string_ptr);
unsigned char lldp_remote_receive_port_to_string (vtss_port_no_t port_number, char *string_ptr, vtss_isid_t  isid );
lldp_remote_entry_t   *lldp_remote_get_entries(void);
lldp_u8_t lldp_remote_get_ieee_draft_version(lldp_u8_t port_index);
void lldp_remote_set_ieee_draft_version(lldp_u8_t port_index, lldp_u8_t value);
void lldp_remote_port_id_to_string (lldp_remote_entry_t   *entry, char *string_ptr);
void lldp_mib_stats_clear(void);

#ifdef VTSS_SW_OPTION_POE
lldp_u16_t lldp_remote_get_requested_power(lldp_u8_t port_index, poe_mode_t poe_mode);
void lldp_remote_set_requested_power(lldp_u8_t port_index, lldp_u16_t value);
lldp_bool_t lldp_remote_get_poe_power_info(const lldp_remote_entry_t *entry, int *power_type, int *power_source, int *power_priority, int *power_value);
void  lldp_remote_poeinfo2string (lldp_remote_entry_t   *entry, char *string_ptr);
void lldp_remote_set_lldp_info_valid(lldp_port_t port_index, lldp_u8_t value);
lldp_u8_t lldp_remote_lldp_is_info_valid(lldp_port_t port_index);

#endif

#define LLDP_REMOTE_ENTRIES LLDP_PORTS * 4
#endif

