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
#ifndef _IP2_ECOS_DRIVER_H_
#define _IP2_ECOS_DRIVER_H_

#include "vtss_types.h"

void vtss_ip2_ecos_driver_init(void);
vtss_vid_t vtss_ip2_ecos_driver_if_index_to_vid(int idx);
int vtss_ip2_ecos_driver_vid_to_if_index(vtss_vid_t idx);
int vtss_ip2_ecos_driver_inject(vtss_vid_t vid, u32 length, const u8 *const data);
int vtss_ip2_ecos_driver_if_add(vtss_vid_t vlan, const vtss_mac_t *mac);
int vtss_ip2_ecos_driver_if_del(vtss_vid_t vid);

vtss_rc vtss_ip2_ecos_driver_if_status(    vtss_if_status_type_t      type,
                                           u32                        max,
                                           u32                       *cnt,
                                           vtss_vid_t                 vid,
                                           vtss_if_status_t          *status);

vtss_rc vtss_ip2_ecos_driver_if_status_all(vtss_if_status_type_t      type,
                                           u32                        max,
                                           u32                       *cnt,
                                           vtss_if_status_t          *status);

vtss_rc vtss_ip2_ecos_driver_route_get(    vtss_routing_entry_type_t  type,
                                           u32                        max,
                                           vtss_routing_status_t     *rt,
                                           u32                       *const cnt);

vtss_rc vtss_ip2_ecos_driver_nb_clear(     vtss_ip_type_t             type);

vtss_rc vtss_ip2_ecos_driver_nb_status_get(vtss_ip_type_t             type,
                                           u32                        max,
                                           u32                       *cnt,
                                           vtss_neighbour_status_t   *status);
/* Statistics Section: RFC-4293 */
vtss_rc vtss_ip2_ecos_driver_stat_syst_cntr_clear(vtss_ip_type_t version);
vtss_rc vtss_ip2_ecos_driver_stat_intf_cntr_clear(vtss_ip_type_t version, vtss_if_id_t *ifidx);
vtss_rc vtss_ip2_ecos_driver_stat_icmp_cntr_clear(vtss_ip_type_t version);
vtss_rc vtss_ip2_ecos_driver_stat_imsg_cntr_clear(vtss_ip_type_t version, u32 type);

vtss_rc vtss_ip2_ecos_driver_stat_ipoutnoroute_get(     vtss_ip_type_t          version,
                                                        u32                     *val);

vtss_rc vtss_ip2_ecos_driver_stat_icmp_cntr_get(        vtss_ip_type_t          version,
                                                        vtss_ips_icmp_stat_t    *entry);
vtss_rc vtss_ip2_ecos_driver_stat_icmp_cntr_get_first(  vtss_ip_type_t          version,
                                                        vtss_ips_icmp_stat_t    *entry);
vtss_rc vtss_ip2_ecos_driver_stat_icmp_cntr_get_next(   vtss_ip_type_t          version,
                                                        vtss_ips_icmp_stat_t    *entry);

vtss_rc vtss_ip2_ecos_driver_stat_imsg_cntr_get(        vtss_ip_type_t          version,
                                                        u32                     type,
                                                        vtss_ips_icmp_stat_t    *entry);
vtss_rc vtss_ip2_ecos_driver_stat_imsg_cntr_get_first(  vtss_ip_type_t          version,
                                                        u32                     type,
                                                        vtss_ips_icmp_stat_t    *entry);
vtss_rc vtss_ip2_ecos_driver_stat_imsg_cntr_get_next(   vtss_ip_type_t          version,
                                                        u32                     type,
                                                        vtss_ips_icmp_stat_t    *entry);

#endif /* _IP2_ECOS_DRIVER_H_ */

