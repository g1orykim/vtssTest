/*

 Vitesse Switch API software.

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

#ifndef _IPMC_CONF_H_
#define _IPMC_CONF_H_

#include "ipmc_lib_base.h"


#define IPMC_VLAN_MAX                   32
#define IPMC_VID_INIT                   VTSS_IPMC_VID_VOID

#define IPMC_VLAN_RESERVED_ID           0x1

#define VTSS_IPMC_IGMP_VERSION_DEF      VTSS_IPMC_VERSION_DEFAULT
#define VTSS_IPMC_IGMP_VERSION1         VTSS_IPMC_VERSION1
#define VTSS_IPMC_IGMP_VERSION2         VTSS_IPMC_VERSION2
#define VTSS_IPMC_IGMP_VERSION3         VTSS_IPMC_VERSION3
#define VTSS_IPMC_MLD_VERSION_DEF       VTSS_IPMC_VERSION_DEFAULT
#define VTSS_IPMC_MLD_VERSION1          VTSS_IPMC_VERSION2
#define VTSS_IPMC_MLD_VERSION2          VTSS_IPMC_VERSION3


/* IPMC Configuration Version */
/* If IPMC_CONF_VERSION is changed, modify ipmc_conf_transition */
#define IPMC_CONF_VERINIT               0
#define IPMC_CONF_VERSION               5


typedef struct {
    BOOL        ports[VTSS_PORT_ARRAY_SIZE];
} ipmc_conf_router_port_t;

typedef struct {
    BOOL        ports[VTSS_PORT_ARRAY_SIZE];
} ipmc_conf_fast_leave_port_t;

typedef struct {
    int         ports[VTSS_PORT_ARRAY_SIZE];
} ipmc_conf_throttling_max_no_t;

typedef struct {
    int         port_no;

    /* when used for getnext func and "group_address" is set to "0", it means getting the first entry */
    union {
        struct {
            vtss_ipv6_t group_address;
        } v6;

        struct {
            vtss_ipv4_t reserved[3];
            vtss_ipv4_t group_address;
        } v4;

        struct {
            i8          name[VTSS_IPMC_NAME_MAX_LEN];
        } profile;
    } addr;
} ipmc_conf_port_group_filtering_t;

typedef struct {
    BOOL                    valid;
    BOOL                    protocol_status;
    BOOL                    querier_status;
    BOOL                    proxy_status;

    vtss_ipv4_t             querier4_address;
    u32                     compatibility;

    u32                     robustness_variable;
    u32                     query_interval;
    u32                     query_response_interval;
    u32                     last_listener_query_interval;
    u32                     unsolicited_report_interval;

    vtss_vid_t              vid;
    u8                      priority;
    u8                      reserved[5];
} ipmc_conf_intf_entry_t;

typedef struct {
    BOOL                    ipmc_mode_enabled;
    BOOL                    ipmc_unreg_flood_enabled;
    BOOL                    ipmc_leave_proxy_enabled;
    BOOL                    ipmc_proxy_enabled;

    ipmc_prefix_t           ssm_range;
} ipmc_conf_global_t;

typedef struct {
    ipmc_ip_version_t       version;

    ipmc_conf_global_t      global;

    ipmc_conf_intf_entry_t  ipmc_conf_intf_entries[IPMC_VLAN_MAX];

    BOOL                    ipmc_router_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];         /* Dest. ports */
    BOOL                    ipmc_fast_leave_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];     /* Dest. ports */
    int                     ipmc_throttling_max_no[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];    /* "0" means disabled */
    u32                     ipmc_port_group_filtering[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];
} ipmc_configuration_t;

typedef struct {
    u32                     blk_version;    /* Block version */

    ipmc_configuration_t    ipv4_conf;      /* Configuration for IGMP */
    ipmc_configuration_t    ipv6_conf;      /* Configuration for MLD */
} ipmc_conf_blk_t;

#endif /* _IPMC_CONF_H_ */
