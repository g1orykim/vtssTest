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

 $Id$
 $Revision$

*/

/**
 * \file
 * \brief ipmc snooping icli common functions
 * \details This header file describes icli common functions for IPMC Snooping
 */


#ifndef _IPMC_SNP_ICLI_H_
#define _IPMC_SNP_ICLI_H_

#include "icli_api.h"
#include "icli_porting_util.h"
#include "mgmt_api.h"
#include "msg_api.h"
#include "topo_api.h"
#include "misc_api.h"
#include "ipmc.h"
#include "ipmc_api.h"


BOOL icli_ipmc_snp_check_present(IN u32 session_id, IN icli_runtime_ask_t ask, OUT icli_runtime_t *runtime);

BOOL icli_ipmc_snp_show_statistics(ipmc_ip_version_t version, i32 session_id,
                                   BOOL by_vlan, icli_unsigned_range_t *vlist,
                                   BOOL mrouter, BOOL detail);
BOOL icli_ipmc_snp_clear_statistics(ipmc_ip_version_t version, i32 session_id,
                                    BOOL by_vlan, icli_unsigned_range_t *vlist);
BOOL icli_ipmc_snp_show_db(ipmc_ip_version_t version, i32 session_id,
                           BOOL by_vlan, icli_unsigned_range_t *vlist,
                           BOOL by_port, icli_stack_port_range_t *plist,
                           BOOL inc_sfm, BOOL detail);

BOOL icli_ipmc_snp_intf_config(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist);
BOOL icli_ipmc_snp_intf_delete(ipmc_ip_version_t version, i32 session_id, BOOL chk, icli_unsigned_range_t *vlist);
BOOL icli_ipmc_snp_intf_state_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, BOOL val);
BOOL icli_ipmc_snp_intf_querier_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, BOOL val);
BOOL icli_ipmc_snp_intf_querier_adrs_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, vtss_ipv4_t val);
#ifdef VTSS_SW_OPTION_SMB_IPMC
BOOL icli_ipmc_snp_intf_compat_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val);
BOOL icli_ipmc_snp_intf_pri_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val);
BOOL icli_ipmc_snp_intf_rv_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val);
BOOL icli_ipmc_snp_intf_qi_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val);
BOOL icli_ipmc_snp_intf_qri_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val);
BOOL icli_ipmc_snp_intf_lmqi_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val);
BOOL icli_ipmc_snp_intf_uri_set(ipmc_ip_version_t version, i32 session_id, icli_unsigned_range_t *vlist, i32 val);
#endif /* VTSS_SW_OPTION_SMB_IPMC */

BOOL icli_ipmc_snp_immediate_leave_set(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist, BOOL state);
BOOL icli_ipmc_snp_mrouter_set(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist, BOOL state);
#ifdef VTSS_SW_OPTION_SMB_IPMC
BOOL icli_ipmc_snp_port_throttle_clear(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist);
BOOL icli_ipmc_snp_port_throttle_set(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist, i32 maxg);
BOOL icli_ipmc_snp_port_filter_clear(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist);
BOOL icli_ipmc_snp_port_filter_set(ipmc_ip_version_t version, i32 session_id, icli_stack_port_range_t *plist, i8 *profile_name);
#endif /* VTSS_SW_OPTION_SMB_IPMC */

#endif /* _IPMC_SNP_ICLI_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
