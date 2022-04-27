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

#ifndef _MVR_API_H_
#define _MVR_API_H_

#include "mvr_conf.h"

/* Initialize MVR module */
vtss_rc mvr_init(vtss_init_data_t *data);

/* Set MVR Global Mode */
vtss_rc mvr_mgmt_set_mode(BOOL *mode);

/* Get MVR Global Mode */
vtss_rc mvr_mgmt_get_mode(BOOL *mode);

/* Set MVR VLAN Interface */
vtss_rc mvr_mgmt_set_intf_entry(vtss_isid_t isid_in, ipmc_operation_action_t action, mvr_mgmt_interface_t *entry);

/* Validate MVR VLAN Channel */
vtss_rc mvr_mgmt_validate_intf_channel(void);

/* Get MVR VLAN Interface By Name(String) */
vtss_rc mvr_mgmt_get_intf_entry_by_name(vtss_isid_t isid, mvr_mgmt_interface_t *entry);

/* Get-Next MVR VLAN Interface By Name(String) */
vtss_rc mvr_mgmt_get_next_intf_entry_by_name(vtss_isid_t isid, mvr_mgmt_interface_t *entry);

/* Get MVR VLAN Interface */
vtss_rc mvr_mgmt_get_intf_entry(vtss_isid_t isid, mvr_mgmt_interface_t *entry);

/* Get-Next MVR VLAN Interface */
vtss_rc mvr_mgmt_get_next_intf_entry(vtss_isid_t isid, mvr_mgmt_interface_t *entry);

/* Get Local MVR VLAN Interface */
vtss_rc mvr_mgmt_local_interface_get(mvr_local_interface_t *entry);

/* Get-Next Local MVR VLAN Interface */
vtss_rc mvr_mgmt_local_interface_get_next(mvr_local_interface_t *entry);

/* Get MVR VLAN Operational Status */
vtss_rc mvr_mgmt_get_intf_info(vtss_isid_t isid, ipmc_ip_version_t version, ipmc_prot_intf_entry_param_t *entry);

/* Get-Next MVR VLAN Operational Status */
vtss_rc mvr_mgmt_get_next_intf_info(vtss_isid_t isid, ipmc_ip_version_t *version, ipmc_prot_intf_entry_param_t *entry);

/* GetNext IPMC Interface(VLAN) Group Info */
vtss_rc mvr_mgmt_get_next_intf_group(vtss_isid_t isid,
                                     ipmc_ip_version_t *ipmc_version,
                                     u16 *vid,
                                     ipmc_prot_intf_group_entry_t *intf_group_entry);

/* GetNext SourceList Entry per Group */
vtss_rc mvr_mgmt_get_next_group_srclist(vtss_isid_t isid,
                                        ipmc_ip_version_t ipmc_version,
                                        vtss_vid_t vid,
                                        vtss_ipv6_t *addr,
                                        ipmc_prot_group_srclist_t *group_srclist_entry);

/* Set MVR Fast leave Ports */
vtss_rc mvr_mgmt_set_fast_leave_port(vtss_isid_t isid, mvr_conf_fast_leave_t *fast_leave_port);

/* Get MVR Fast leave Ports */
vtss_rc mvr_mgmt_get_fast_leave_port(vtss_isid_t isid, mvr_conf_fast_leave_t *fast_leave_port);

/* Clear MVR Statistics Counters */
vtss_rc mvr_mgmt_clear_stat_counter(vtss_isid_t isid, vtss_vid_t vid);

/* Obsoleted API for MVID */
vtss_rc mvr_mgmt_get_mvid(u16 *obs_mvid);

#endif /* _MVR_API_H_ */
