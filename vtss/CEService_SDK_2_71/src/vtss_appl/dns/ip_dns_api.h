/*

 Vitesse Switch API software.

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
/* debug msg can be enabled by cmd "debug trace module level dns default debug" */

#ifndef _VTSS_IP_DNS_API_H_
#define _VTSS_IP_DNS_API_H_

#include "vtss_dns.h"

/* IP DNS error codes (vtss_rc) */
typedef enum {
    IP_DNS_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_IP_DNS),  /* Generic error code */
} ip_dns_error_t;

#define VTSS_DNS_SERVER_DEF_TYPE    VTSS_DNS_SRV_TYPE_NONE
#define VTSS_DNS_TYPE_DEFAULT(x)    ((x)->dns_type == VTSS_DNS_SERVER_DEF_TYPE)
#define VTSS_DNS_PROXY_DISABLE      0
#define VTSS_DNS_PROXY_ENABLE       1
#define VTSS_DNS_PROXY_DEF_STATE    VTSS_DNS_PROXY_DISABLE

vtss_rc ip_dns_init(vtss_init_data_t *data);

vtss_rc ip_dns_mgmt_get_proxy_status(BOOL *status);
vtss_rc ip_dns_mgmt_set_proxy_status(BOOL *status);

#define VTSS_DNS_TYPE_PTR(x)        (&((x)->dns_type))
#define VTSS_DNS_ADDR_TYPE_PTR(x)   (&((x)->u.static_conf_addr.type))
#define VTSS_DNS_ADDR_IPA4_PTR(x)   (&((x)->u.static_conf_addr.addr.ipv4))
#define VTSS_DNS_ADDR_IPA6_PTR(x)   (&((x)->u.static_conf_addr.addr.ipv6))
#define VTSS_DNS_ADDR_PTR(x)        (&((x)->u.static_conf_addr))
#define VTSS_DNS_VLAN_PTR(x)        (&((x)->u.dynamic_conf_vlan))
#define VTSS_DNS_TYPE_GET(x)        ((x)->dns_type)
#define VTSS_DNS_ADDR_TYPE_GET(x)   ((x)->u.static_conf_addr.type)
#define VTSS_DNS_ADDR_IPA4_GET(x)   ((x)->u.static_conf_addr.addr.ipv4)
#define VTSS_DNS_ADDR_IPA6_GET(x)   ((x)->u.static_conf_addr.addr.ipv6)
#define VTSS_DNS_ADDR_GET(x)        ((x)->u.static_conf_addr)
#define VTSS_DNS_VLAN_GET(x)        ((x)->u.dynamic_conf_vlan)

#define VTSS_DNS_TYPE_SET(x, y)     do {VTSS_DNS_TYPE_GET((x)) = (y);} while (0)
#define VTSS_DNS_ADDR4_SET(x, y)    do {VTSS_DNS_ADDR_TYPE_GET((x)) = VTSS_IP_TYPE_IPV4; VTSS_DNS_ADDR_IPA4_GET((x)) = (y);} while (0)
#define VTSS_DNS_ADDR6_SET(x, y)    do {VTSS_DNS_ADDR_TYPE_GET((x)) = VTSS_IP_TYPE_IPV6; memcpy(VTSS_DNS_ADDR_IPA6_PTR((x)), (y), sizeof(vtss_ipv6_t));} while (0)
#define VTSS_DNS_ADDR_SET(x, y)     do {memcpy(VTSS_DNS_ADDR_PTR((x)), (y), sizeof(vtss_ip_addr_t));} while (0)
#define VTSS_DNS_VLAN_SET(x, y)     do {VTSS_DNS_VLAN_GET((x)) = (y);} while (0)

void vtss_dns_signal(void);

vtss_rc vtss_dns_mgmt_get_server(u8 idx, vtss_dns_srv_conf_t *dns_srv);
vtss_rc vtss_dns_mgmt_set_server(u8 idx, vtss_dns_srv_conf_t *dns_srv);

vtss_rc vtss_dns_mgmt_get_server4(vtss_ipv4_t *dns_srv);
#ifdef VTSS_SW_OPTION_IPV6
vtss_rc vtss_dns_mgmt_get_server6(vtss_ipv6_t *dns_srv);
#endif /* VTSS_SW_OPTION_IPV6 */

#endif /* _VTSS_IP_DNS_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
