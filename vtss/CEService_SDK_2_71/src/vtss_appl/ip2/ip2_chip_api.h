/*

 Vitesse API software.

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

#ifndef _IP2_CHIP_API_H_
#define _IP2_CHIP_API_H_

#include "ip2_types.h"
#include "vtss_types.h"

vtss_rc vtss_ip2_chip_init(void);
vtss_rc vtss_ip2_chip_start(void);
vtss_rc vtss_ip2_chip_master_up(const vtss_mac_t *const mac);
vtss_rc vtss_ip2_chip_master_down(void);
vtss_rc vtss_ip2_chip_switch_add(vtss_isid_t isid);
vtss_rc vtss_ip2_chip_switch_del(vtss_isid_t isid);

vtss_rc vtss_ip2_chip_routing_enable(BOOL enable);

vtss_rc vtss_ip2_chip_rleg_add(const vtss_vid_t vlan);
vtss_rc vtss_ip2_chip_rleg_del(const vtss_vid_t vlan);

vtss_rc vtss_ip2_chip_mac_subscribe(const vtss_vid_t vlan, const vtss_mac_t *const mac);
vtss_rc vtss_ip2_chip_mac_unsubscribe(const vtss_vid_t vlan, const vtss_mac_t *const mac);

vtss_rc vtss_ip2_chip_route_add(const vtss_routing_entry_t *const rt);
vtss_rc vtss_ip2_chip_route_del(const vtss_routing_entry_t *const rt);

vtss_rc vtss_ip2_chip_neighbour_add(const vtss_neighbour_t *const nb);
vtss_rc vtss_ip2_chip_neighbour_del(const vtss_neighbour_t *const nb);

/* Statistics Section: RFC-4293 */
vtss_rc vtss_ip2_chip_counters_vlan_get(const vtss_vid_t vlan,
                                        vtss_l3_counters_t *counters);
vtss_rc vtss_ip2_chip_counters_vlan_clear(const vtss_vid_t vlan);

#endif /* _IP2_CHIP_API_H_ */
