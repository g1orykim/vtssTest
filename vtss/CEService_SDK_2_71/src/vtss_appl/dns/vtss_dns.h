/*

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

/*
 * This file contains the API definitions used between the DNS protocol
 * module and the operating environment.
 */
#ifndef _VTSS_DNS_H_
#define _VTSS_DNS_H_

#include "packet_api.h"
#include "vlan_api.h"

#define DNS_MAX_SRV_CNT         1
#define DNS_DEF_SRV_IDX         0

typedef enum {
    VTSS_DNS_SRV_TYPE_DHCP_ANY  = 0x0,
    VTSS_DNS_SRV_TYPE_NONE      = 0x1,
    VTSS_DNS_SRV_TYPE_STATIC    = 0x2,
    VTSS_DNS_SRV_TYPE_DHCP_VLAN = 0x4
} vtss_dns_srv_conf_type_t;

typedef struct {
    vtss_dns_srv_conf_type_t    dns_type;

    union {
        vtss_ip_addr_t          static_conf_addr;
        vtss_vid_t              dynamic_conf_vlan;
    } u;
} vtss_dns_srv_conf_t;

#define VTSS_DNS_INFO_CONF_TYPE(x)  ((x) ? (x)->dns_type : VTSS_DNS_SRV_TYPE_NONE)
#define VTSS_DNS_INFO_CONF_ADDR4(x) ((VTSS_DNS_INFO_CONF_TYPE((x)) == VTSS_DNS_SRV_TYPE_STATIC) ? (((x)->u.static_conf_addr.type == VTSS_IP_TYPE_IPV4) ? (x)->u.static_conf_addr.addr.ipv4 : 0) : 0)
#define VTSS_DNS_INFO_CONF_ADDR6(x) ((VTSS_DNS_INFO_CONF_TYPE((x)) == VTSS_DNS_SRV_TYPE_STATIC) ? (((x)->u.static_conf_addr.type == VTSS_IP_TYPE_IPV6) ? &((x)->u.static_conf_addr.addr.ipv6) : NULL) : NULL)
#define VTSS_DNS_INFO_CONF_VLAN(x)  (((VTSS_DNS_INFO_CONF_TYPE((x)) == VTSS_DNS_SRV_TYPE_DHCP_ANY) || (VTSS_DNS_INFO_CONF_TYPE((x)) == VTSS_DNS_SRV_TYPE_DHCP_VLAN)) ? (x)->u.dynamic_conf_vlan : VTSS_VID_NULL)

#define VTSS_DNS_ADDR_VALID(x)      (vtss_dns_ipa_valid(&((x)->u.static_conf_addr)))
#define VTSS_DNS_VLAN_VALID(x)      (((x)->u.dynamic_conf_vlan >= VLAN_ID_MIN) && ((x)->u.dynamic_conf_vlan < MIN(VLAN_ID_MAX, 4094)))

/* DNS General configuration */
typedef struct {
    vtss_dns_srv_conf_t         dns_conf[DNS_MAX_SRV_CNT];

    BOOL                        dns_proxy_status;   /* status of DNS Proxy (1:enable 0:disable) */
} dns_conf_t;

void vtss_dns_init(void);

void vtss_dns_client_update_timer(void);

BOOL vtss_dns_ipa_valid(vtss_ip_addr_t *ipa);

BOOL vtss_rx_dns(void  *contxt, const uchar *const frame, const vtss_packet_rx_info_t *const rx_info);

#endif
