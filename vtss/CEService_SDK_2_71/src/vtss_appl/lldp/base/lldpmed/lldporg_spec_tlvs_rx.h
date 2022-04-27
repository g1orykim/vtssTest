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

#ifndef LLDPORG_RX_H
#define LLDPORG_RX_H
#include "lldp_remote.h"
lldp_bool_t lldporg_update_neccessary(lldp_rx_remote_entry_t   *rx_entry, lldp_remote_entry_t   *entry);
void lldporg_update_entry(lldp_rx_remote_entry_t   *rx_entry, lldp_remote_entry_t   *entry) ;
lldp_bool_t lldporg_validate_lldpdu(lldp_8_t   *tlv, lldp_rx_remote_entry_t   *rx_entry, lldp_u16_t len);
void lldporg_autoneg_support2str (lldp_remote_entry_t   *entry, char *string_ptr);
void lldporg_autoneg_status2str (lldp_remote_entry_t   *entry, char *string_ptr);
void lldporg_operational_mau_type2str (lldp_remote_entry_t   *entry, char *string_ptr);
void lldporg_autoneg_capa2str (lldp_remote_entry_t   *entry, char *string_ptr);
#endif // LLDPORG_RX_H



