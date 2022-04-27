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

#ifndef _IP2_ICLI_PRIV_H_
#define _IP2_ICLI_PRIV_H_

#include "icli_api.h"

vtss_rc get_vlan_if(vtss_vid_t vid);

#ifdef VTSS_SW_OPTION_ICFG

#define IP2_ICFG_DEF_ROUTING4   TRUE
#define IP2_ICFG_DEF_ROUTING6   TRUE

vtss_rc vtss_ip2_ipv4_icfg_init(void);

#endif /* VTSS_SW_OPTION_ICFG */

void icli_ip2_intf_status_display(i32 session_id,
                                  BOOL brief,
                                  void *intf_status);

void icli_ip2_intf_neighbor_display(i32 session_id,
                                    vtss_ip_type_t version,
                                    BOOL by_vlan,
                                    vtss_vid_t vid);

void icli_ip2_stat_ip_syst_display(i32 session_id,
                                   vtss_ips_ip_stat_t *entry);

void icli_ip2_stat_ip_intf_display(i32 session_id,
                                   BOOL *first_pr,
                                   vtss_if_status_ip_stat_t *entry);

void icli_ip2_stat_icmp_syst_display(i32 session_id,
                                     vtss_ips_icmp_stat_t *entry);

void icli_ip2_stat_icmp_type_display(i32 session_id,
                                     BOOL force_pr,
                                     BOOL *first_pr,
                                     vtss_ips_icmp_stat_t *entry);

#endif /* _IP2_ICLI_PRIV_H_ */
