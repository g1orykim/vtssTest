/*

 Copyright (c) 2002-2008 Vitesse Semiconductor Corporation "Vitesse". All
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
* Types / functions defintions for LLDP-MED receive part
******************************************************************************/

#ifndef LLDPMED_RX_H
#define LLDPMED_RX_H
void        lldpmed_update_entry(lldp_rx_remote_entry_t   *rx_entry, lldp_remote_entry_t   *entry);
lldp_bool_t lldpmed_validate_lldpdu(lldp_8_t   *tlv, lldp_rx_remote_entry_t   *rx_entry, lldp_u16_t len, lldp_bool_t first_lldpmed_tlv);
lldp_bool_t lldpmed_update_neccessary(lldp_rx_remote_entry_t   *rx_entry, lldp_remote_entry_t   *entry);
void        lldpmed_device_type2str (lldp_remote_entry_t   *entry, char *string_ptr );
void        lldpmed_capabilities2str (lldp_remote_entry_t   *entry, char *string_ptr );
void        lldpmed_policy_dscp2str (lldp_u32_t policy, char *string_ptr );
void        lldpmed_policy_prio2str (lldp_u32_t policy, char *string_ptr );
void        lldpmed_policy_vlan_id2str (lldp_u32_t policy, char *string_ptr );
void        lldpmed_policy_tag2str (lldp_u32_t policy, char *string_ptr );
void        lldpmed_policy_appl_type2str (lldp_u32_t policy, char *string_ptr );
void        lldpmed_policy_flag_type2str (lldp_u32_t policy, char *string_ptr );
lldp_u8_t   lldpmed_get_policies_cnt (lldp_remote_entry_t   *entry);
lldp_u8_t   lldpmed_get_locations_cnt (lldp_remote_entry_t   *entry);
void        lldpmed_cal_la (void);
void        lldpmed_location2str (lldp_remote_entry_t   *entry, char *string_ptr, lldp_u8_t index);
void        lldpmed_medTransmitEnabled_set(lldp_port_t p_index, lldp_bool_t tx_enable);
lldp_bool_t lldpmed_medTransmitEnabled_get(lldp_port_t p_index);
lldp_u8_t   lldpmed_medFastStart_timer_get(lldp_port_t p_index);
void        lldpmed_medFastStart_timer_action(lldp_port_t p_index, lldp_u8_t tx_enable, lldpmed_fast_start_repeat_count_t action);
BOOL        lldpmed_get_unknown_policy_flag (lldp_u32_t lldpmed_policy);
BOOL        lldpmed_get_tagged_flag(lldp_u32_t lldpmed_policy);
void        lldpmed_catype2str(lldpmed_catype_t ca_type, lldp_8_t *string_ptr);
lldp_bool_t lldpmed_tude2decimal_str (lldp_u8_t res,  lldp_u64_t tude, lldp_u8_t int_bits, lldp_u8_t frac_bits, lldp_8_t *string_ptr);
void        lldpmed_at2str(lldpmed_at_type_t, lldp_8_t *string_ptr);
void        lldpmed_update_civic_info(lldpmed_location_civic_info_t *civic_info, lldpmed_catype_t new_ca_type, lldp_8_t *new_ca_value);
lldpmed_catype_t lldpmed_index2catype(lldp_u8_t index);
lldp_u8_t   lldpmed_catype2index(lldpmed_catype_t ca_type);
void        lldpmed_policy_appl_type2str (lldp_u32_t lldpmed_policy, lldp_8_t *string_ptr);
void        lldpmed_appl_type2str (lldpmed_application_type_t appl_type_value, lldp_8_t *string_ptr);
void        lldpmed_datum2str(lldpmed_datum_t datum, lldp_8_t *string_ptr);

#endif //LLDPMED_RX_H


