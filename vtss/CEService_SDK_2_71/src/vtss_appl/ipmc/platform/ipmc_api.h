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

#ifndef _IPMC_API_H_
#define _IPMC_API_H_

#include "ipmc_conf.h"

/* Initialize module */
vtss_rc ipmc_init(vtss_init_data_t *data);

/* Set IPMC Global Mode */
vtss_rc ipmc_mgmt_set_mode(BOOL *mode, ipmc_ip_version_t ipmc_version);

/* Get IPMC Global Mode */
vtss_rc ipmc_mgmt_get_mode(BOOL *mode, ipmc_ip_version_t ipmc_version);

/* Set IPMC Unregisterred Flooding */
vtss_rc ipmc_mgmt_set_unreg_flood(BOOL *mode, ipmc_ip_version_t ipmc_version);

/* Get IPMC Unregisterred Flooding */
vtss_rc ipmc_mgmt_get_unreg_flood(BOOL *mode, ipmc_ip_version_t ipmc_version);

/* Set IPMC Proxy */
vtss_rc ipmc_mgmt_set_proxy(BOOL *mode, ipmc_ip_version_t ipmc_version);

/* Get IPMC Proxy */
vtss_rc ipmc_mgmt_get_proxy(BOOL *mode, ipmc_ip_version_t ipmc_version);

/* Set IPMC Leave Proxy */
vtss_rc ipmc_mgmt_set_leave_proxy(BOOL *mode, ipmc_ip_version_t ipmc_version);

/* Get IPMC Leave Proxy */
vtss_rc ipmc_mgmt_get_leave_proxy(BOOL *mode, ipmc_ip_version_t ipmc_version);

/* Set IPMC SSM Range */
vtss_rc ipmc_mgmt_set_ssm_range(ipmc_ip_version_t ipmc_version, ipmc_prefix_t *range);

/* Get IPMC SSM Range */
vtss_rc ipmc_mgmt_get_ssm_range(ipmc_ip_version_t ipmc_version, ipmc_prefix_t *range);

/* Check whether default IPMC SSM Range is changed for not  */
BOOL ipmc_mgmt_def_ssm_range_chg(ipmc_ip_version_t version, ipmc_prefix_t *range);

/* Check whether the ADD/DEL operation is allowed or not */
BOOL ipmc_mgmt_intf_op_allow(vtss_isid_t isid);

/* Set IPMC VLAN state and querier */
vtss_rc ipmc_mgmt_set_intf_state_querier(ipmc_operation_action_t action,
                                         vtss_vid_t vid,
                                         BOOL *state_enable,
                                         BOOL *querier_enable,
                                         ipmc_ip_version_t ipmc_version);

/* Get IPMC Interface(VLAN) State and Querier Status */
vtss_rc ipmc_mgmt_get_intf_state_querier(BOOL conf, vtss_vid_t *vid, u8 *state_enable, u8 *querier_enable, BOOL next, ipmc_ip_version_t ipmc_version);

/* Set IPMC Interface(VLAN) Info */
vtss_rc ipmc_mgmt_set_intf_info(vtss_isid_t isid, ipmc_prot_intf_entry_param_t *intf_param, ipmc_ip_version_t ipmc_version);

/* Get IPMC Interface(VLAN) Info */
vtss_rc ipmc_mgmt_get_intf_info(vtss_isid_t isid, vtss_vid_t vid, ipmc_prot_intf_entry_param_t *intf_param, ipmc_ip_version_t ipmc_version);

/* Set IPMC Throttling */
vtss_rc ipmc_mgmt_get_throttling_max_count(vtss_isid_t isid, ipmc_conf_throttling_max_no_t *ipmc_throttling_max_no, ipmc_ip_version_t ipmc_version);

/* Get IPMC Throttling */
vtss_rc ipmc_mgmt_set_throttling_max_count(vtss_isid_t isid, ipmc_conf_throttling_max_no_t *ipmc_throttling_max_no, ipmc_ip_version_t ipmc_version);

/* Add IPMC Group Filtering Entry */
vtss_rc ipmc_mgmt_set_port_group_filtering(vtss_isid_t isid, ipmc_conf_port_group_filtering_t *ipmc_port_group_filtering, ipmc_ip_version_t ipmc_version);

/* Del IPMC Group Filtering Entry */
vtss_rc ipmc_mgmt_del_port_group_filtering(vtss_isid_t isid, ipmc_conf_port_group_filtering_t *ipmc_port_group_filtering, ipmc_ip_version_t ipmc_version);

/* GetNext IPMC Group Filtering Entry */
vtss_rc ipmc_mgmt_get_port_group_filtering(vtss_isid_t isid, ipmc_conf_port_group_filtering_t *ipmc_port_group_filtering, ipmc_ip_version_t ipmc_version);

/* Set IPMC Router Ports */
vtss_rc ipmc_mgmt_set_router_port(vtss_isid_t isid, ipmc_conf_router_port_t *router_port, ipmc_ip_version_t ipmc_version);

/* Get IPMC Router Ports */
vtss_rc ipmc_mgmt_get_static_router_port(vtss_isid_t isid, ipmc_conf_router_port_t *router_port, ipmc_ip_version_t ipmc_version);

/* Set IPMC Fast leave Ports */
vtss_rc ipmc_mgmt_set_fast_leave_port(vtss_isid_t isid, ipmc_conf_fast_leave_port_t *fast_leave_port, ipmc_ip_version_t ipmc_version);

/* Get IPMC Fast leave Ports */
vtss_rc ipmc_mgmt_get_fast_leave_port(vtss_isid_t isid, ipmc_conf_fast_leave_port_t *fast_leave_port, ipmc_ip_version_t ipmc_version);

/* Clear IPMC Statistics Counters */
vtss_rc ipmc_mgmt_clear_stat_counter(vtss_isid_t isid, ipmc_ip_version_t ipmc_version, vtss_vid_t vid);

/* Get IPMC Dynamic Router Ports */
vtss_rc ipmc_mgmt_get_dynamic_router_ports(vtss_isid_t isid, ipmc_dynamic_router_port_t *router_port, ipmc_ip_version_t ipmc_version);

/* Get IPMC Working Versions */
vtss_rc ipmc_mgmt_get_intf_version(vtss_isid_t isid, ipmc_intf_query_host_version_t *vlan_version_entry, ipmc_ip_version_t ipmc_version);

/* GetNext IPMC Interface(VLAN) Group Info */
vtss_rc ipmc_mgmt_get_next_intf_group_info(vtss_isid_t isid, vtss_vid_t vid, ipmc_prot_intf_group_entry_t *intf_group_entry, ipmc_ip_version_t ipmc_version);

/* GetNext SourceList Entry per Group */
vtss_rc ipmc_mgmt_get_next_group_srclist_info(vtss_isid_t isid, ipmc_ip_version_t ipmc_version, vtss_vid_t vid, vtss_ipv6_t *addr, ipmc_prot_group_srclist_t *group_srclist_entry);

#endif /* _IPMC_API_H_ */
