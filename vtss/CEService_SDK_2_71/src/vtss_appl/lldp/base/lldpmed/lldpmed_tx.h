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

#ifndef LLDPMED_TX_H
#define LLDPMED_TX_H
lldp_u16_t lldpmed_tlv_add(lldp_u8_t *buf, lldp_port_t port_idx);
lldp_u16_t lldpmed_get_capabilities_word(lldp_port_t port_idx);
lldp_u8_t lldpmed_coordinate_location_tlv_add(lldp_u8_t *buf);
lldp_u8_t lldpmed_ecs_location_tlv_add(lldp_u8_t *buf);
lldp_u8_t lldpmed_civic_location_tlv_add(lldp_u8_t *buf);
void lldpmed_cal_fraction (lldp_64_t  tude_val, lldp_u32_t *fraction , lldp_u32_t *int_part, lldp_bool_t negative_number, lldp_u32_t fraction_bits_cnt);
#endif



