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

#ifndef _IP2_OS_API_H_
#define _IP2_OS_API_H_

#include "ip2_api.h"
#include "ip2_priv.h"


void if_index_to_if_id(u32 _if_index, vtss_if_id_t  *ifid);
vtss_rc vtss_ip2_os_if_index_to_vlan_if(u32 _if_index, vtss_if_id_vlan_t *vlan);

vtss_rc vtss_ip2_os_init(void);

vtss_rc vtss_ip2_os_global_param_set(const vtss_ip2_global_param_t *const param);

vtss_rc vtss_ip2_os_if_add(vtss_vid_t               vid);

vtss_rc vtss_ip2_os_if_set(vtss_vid_t               vid,
                           const vtss_if_param_t   *const if_params);

vtss_rc vtss_ip2_os_if_ctl(vtss_vid_t               vid,
                           BOOL                     up);

vtss_rc vtss_ip2_os_if_del(vtss_vid_t vid);

vtss_rc vtss_ip2_os_if_status(vtss_if_status_type_t   type,
                              const u32               max,
                              u32                    *cnt,
                              vtss_if_id_vlan_t       id,
                              vtss_if_status_t       *status);

vtss_rc vtss_ip2_os_if_status_all(vtss_if_status_type_t  type,
                                  const u32              max,
                                  u32                   *cnt,
                                  vtss_if_status_t      *status);

vtss_rc vtss_ip2_os_inject(vtss_vid_t vid,
                           u32                     length,
                           const u8                *const data);

vtss_rc vtss_ip2_os_ip_add(vtss_vid_t  vid,
                           const vtss_ip_network_t *const network);

vtss_rc vtss_ip2_os_ip_del(vtss_vid_t vid,
                           const vtss_ip_network_t *const network);

vtss_rc vtss_ip2_os_route_add(const vtss_routing_entry_t *const rt);
vtss_rc vtss_ip2_os_route_del(const vtss_routing_entry_t *const rt);

vtss_rc vtss_ip2_os_route_get(vtss_routing_entry_type_t type,
                              u32                       max,
                              vtss_routing_status_t     *rt,
                              u32                       *const cnt);

vtss_rc vtss_ip2_os_nb_clear(      vtss_ip_type_t        type);

vtss_rc vtss_ip2_os_nb_status_get( vtss_ip_type_t        type,
                                   u32                   max,
                                   u32                  *cnt,
                                   vtss_neighbour_status_t *status);

/* Statistics Section: RFC-4293 */
vtss_rc vtss_ip2_os_stat_syst_cntr_clear(vtss_ip_type_t version);
vtss_rc vtss_ip2_os_stat_intf_cntr_clear(vtss_ip_type_t version, vtss_if_id_t *ifidx);
vtss_rc vtss_ip2_os_stat_icmp_cntr_clear(vtss_ip_type_t version);
vtss_rc vtss_ip2_os_stat_imsg_cntr_clear(vtss_ip_type_t version, u32 type);

vtss_rc vtss_ip2_os_stat_ipoutnoroute_get(      vtss_ip_type_t          version,
                                                u32                     *val);

vtss_rc vtss_ip2_os_stat_icmp_cntr_get(         vtss_ip_type_t          version,
                                                vtss_ips_icmp_stat_t    *entry);
vtss_rc vtss_ip2_os_stat_icmp_cntr_get_first(   vtss_ip_type_t          version,
                                                vtss_ips_icmp_stat_t    *entry);
vtss_rc vtss_ip2_os_stat_icmp_cntr_get_next(    vtss_ip_type_t          version,
                                                vtss_ips_icmp_stat_t    *entry);

vtss_rc vtss_ip2_os_stat_imsg_cntr_get(         vtss_ip_type_t          version,
                                                u32                     type,
                                                vtss_ips_icmp_stat_t    *entry);
vtss_rc vtss_ip2_os_stat_imsg_cntr_get_first(   vtss_ip_type_t          version,
                                                u32                     type,
                                                vtss_ips_icmp_stat_t    *entry);
vtss_rc vtss_ip2_os_stat_imsg_cntr_get_next(    vtss_ip_type_t          version,
                                                u32                     type,
                                                vtss_ips_icmp_stat_t    *entry);


#endif /* _IP2_OS_API_H_ */
