/*

 Vitesse Switch Software.

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
#ifndef __XXRP_TYPES_H__
#define __XXRP_TYPES_H__

#include <vtss_icli_type.h>

extern void gvrp_global_enable(int enable, int mac_vlans);
extern void gvrp_protocol_state(icli_stack_port_range_t *v_port_type_list, icli_unsigned_range_t *v_vlan_list);

extern void gvrp_join_request(icli_stack_port_range_t *plist, icli_unsigned_range_t *v_vlan_list);
extern void gvrp_leave_request(icli_stack_port_range_t *plist, icli_unsigned_range_t *v_vlan_list);

extern void gvrp_port_enable(icli_stack_port_range_t *plist, int enable);

#endif
