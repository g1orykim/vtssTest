/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _MVR_CONF_H_
#define _MVR_CONF_H_

#include "ipmc_lib_base.h"
#include "vlan_api.h"

#define MVR_VLAN_MAX                    4
#define MVR_VID_VOID                    VTSS_IPMC_VID_VOID
#define MVR_VID_MIN                     VLAN_ID_MIN
#define MVR_VID_MAX                     VLAN_ID_MAX

#define MVR_NAME_MAX_LEN                VTSS_IPMC_MVR_NAME_MAX_LEN

#define VTSS_MVR_IGMP_VERSION_DEF       VTSS_IPMC_VERSION_DEFAULT
#define VTSS_MVR_IGMP_VERSION1          VTSS_IPMC_VERSION1
#define VTSS_MVR_IGMP_VERSION2          VTSS_IPMC_VERSION2
#define VTSS_MVR_IGMP_VERSION3          VTSS_IPMC_VERSION3
#define VTSS_MVR_MLD_VERSION_DEF        VTSS_IPMC_VERSION_DEFAULT
#define VTSS_MVR_MLD_VERSION1           VTSS_IPMC_VERSION2
#define VTSS_MVR_MLD_VERSION2           VTSS_IPMC_VERSION3

#define MVR_CONF_DEF_GLOBAL_MODE        VTSS_IPMC_DISABLE
#define MVR_CONF_DEF_INTF_MODE          MVR_INTF_MODE_DYNA
#define MVR_CONF_DEF_INTF_VTAG          IPMC_INTF_TAGED
#define MVR_CONF_DEF_INTF_ADRS4         IPMC_PARAM_DEF_QUERIER_ADRS4
#define MVR_CONF_DEF_INTF_PRIO          IPMC_PARAM_DEF_PRIORITY
#define MVR_CONF_DEF_INTF_PROFILE       0x0
#define MVR_CONF_DEF_INTF_LLQI          0x5 /* tenths seconds */
#define MVR_CONF_DEF_INTF_GRP_CNT       0x1
#define MVR_CONF_DEF_PORT_ROLE          MVR_PORT_ROLE_INACT
#define MVR_CONF_DEF_FAST_LEAVE         VTSS_IPMC_DISABLE

/* MVR Configuration Version */
/* If MVR_CONF_VERSION is changed, modify mvr_conf_transition */
#define MVR_CONF_VERINIT                0
#define MVR_CONF_VERSION                5

typedef struct {
    BOOL                    valid;

    vtss_vid_t              vid;
    mvr_port_role_t         ports[VTSS_PORT_ARRAY_SIZE];
} mvr_conf_port_role_t;

typedef struct {
    mvr_conf_port_role_t    intf[MVR_VLAN_MAX];
} mvr_conf_intf_role_t;

typedef struct {
    BOOL                    ports[VTSS_PORT_BF_SIZE];
} mvr_conf_fast_leave_t;

typedef struct {
    BOOL                    valid;

    BOOL                    protocol_status;
    BOOL                    querier_status;
    u8                      priority;

    vtss_ipv4_t             querier4_address;
    mvr_intf_mode_t         mode;
    ipmc_intf_vtag_t        vtag;
    u32                     profile_index;

    u32                     compatibility;
    u32                     robustness_variable;
    u32                     query_interval;
    u32                     query_response_interval;
    u32                     last_listener_query_interval;
    u32                     unsolicited_report_interval;

    vtss_vid_t              vid;
    i8                      name[MVR_NAME_MAX_LEN];
    u8                      reserved[5];
} mvr_conf_intf_entry_t;

typedef struct {
    BOOL                    mvr_state;
} mvr_conf_global_t;

typedef struct {
    mvr_conf_global_t       mvr_conf_global;
    mvr_conf_intf_entry_t   mvr_conf_vlan[MVR_VLAN_MAX];

    mvr_conf_intf_role_t    mvr_conf_role[VTSS_ISID_END];
    mvr_conf_fast_leave_t   mvr_conf_fast_leave[VTSS_ISID_END];
} mvr_configuration_t;

typedef struct {
    u32                     blk_version;    /* Block version */
    mvr_configuration_t     mvr_conf;       /* Configuration for MVR */
} mvr_conf_blk_t;

/* For MVR Interface Management */
typedef struct {
    vtss_vid_t              vid;
    mvr_conf_intf_entry_t   intf;   /* Interface Parameters */
    mvr_conf_port_role_t    role;   /* Port Roles per Interface */
} mvr_mgmt_interface_t;

/* For Local MVR Interface Information */
typedef struct {
    vtss_vid_t              vid;
    ipmc_ip_version_t       version;

    mvr_conf_intf_entry_t   intf;
    mvr_port_role_t         role_ports[VTSS_PORT_ARRAY_SIZE];
    u8                      vlan_ports[VTSS_BF_SIZE(VTSS_PORT_ARRAY_SIZE)];
} mvr_local_interface_t;

#endif /* _MVR_CONF_H_ */
