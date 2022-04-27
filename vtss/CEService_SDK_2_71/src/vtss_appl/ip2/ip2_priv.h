/*

 Vitesse API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _IP2_PRIV_H_
#define _IP2_PRIV_H_

#include "ip2_types.h"

/*
 * Route DB: Static routes + dynamic route suppliers (DHCP, ...)
 */
#define IP2_MAX_ROUTES_DB (IP2_MAX_ROUTES + 16)

typedef struct {
    vtss_if_param_t   param;    /**< MTU */
    vtss_ip_network_t network;  /**< Interface address/mask */
} ip2_interface_conf_t;

typedef struct ip2_route_entry {
    /* NB - leaving room for metrics */
    vtss_routing_entry_t route;
} ip2_route_entry_t;

/* Configuration structure (master only) */
typedef struct {
    /* Global config */
    vtss_ip2_global_param_t global;
    /* Per interface */
    struct ip2_iface_entry {
        vtss_vid_t               vlan_id;   /**< Key */
        vtss_interface_ip_conf_t conf;      /**< Data */
    } interfaces[IP2_MAX_INTERFACES];
    /* Routes */
    ip2_route_entry_t routes[IP2_MAX_ROUTES];
} ip2_stack_conf_t;

#define IP2_CONF_VERSION  1 /* IP2 flash configuration version */

/* Overall configuration as saved in flash */
typedef struct {
    u32              version;    /* Current version of the configuration in flash */
    ip2_stack_conf_t stack_conf; /* Configuration for the whole stack */
} ip2_flash_conf_t;

#define VTSS_TRACE_MODULE_ID   VTSS_MODULE_ID_IP2
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_RXPKT_DUMP   1
#define TRACE_GRP_TXPKT_DUMP   2
#define TRACE_GRP_CRIT         3
#define TRACE_GRP_CNT          4

#define IP2_MAX_STATUS_OBJS 1024

typedef enum {
    VIF_FWD_UNDEFINED,    /* Initial state */
    VIF_FWD_FORWARDING,   /* At least one port on VIF is forwarding */
    VIF_FWD_BLOCKING,     /* All ports on VIF are blocking  */
} ip2_vif_fwd_t;

void vtss_ip2_if_signal(vtss_if_id_vlan_t if_id);

void vtss_ip2_routing_monitor_enable(void);
void vtss_ip2_routing_monitor_disable(void);

#endif /* _IP2_PRIV_H_ */
