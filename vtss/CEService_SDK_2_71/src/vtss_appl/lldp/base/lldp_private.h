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


#ifndef LLDP_PRIVATE_H
#define LLDP_PRIVATE_H

#include "lldp_os.h"

lldp_sm_t   *lldp_get_port_sm (lldp_port_t port);
void lldp_tx_initialize_lldp (lldp_sm_t   *sm);
void lldp_construct_info_lldpdu(lldp_port_t port_no, lldp_port_t sid);
void lldp_construct_shutdown_lldpdu(lldp_port_t port_no);
void lldp_tx_frame(lldp_port_t port_no);
void lldp_rx_initialize_lldp (lldp_port_t port);

#endif



