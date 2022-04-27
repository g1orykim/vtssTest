/* Switch API software.

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

/**
 * \file
 * \brief vlan icli functions
 * \details This header file describes vlan icli functions
 */

#ifndef _VLAN_ICLI_FUNCTIONS_H_
#define _VLAN_ICLI_FUNCTIONS_H_

#include "icli_api.h"

BOOL VLAN_ICLI_runtime_dot1x(     u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);
BOOL VLAN_ICLI_runtime_mvr(       u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);
BOOL VLAN_ICLI_runtime_voice_vlan(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);
BOOL VLAN_ICLI_runtime_mstp(      u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);
BOOL VLAN_ICLI_runtime_erps(      u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);
BOOL VLAN_ICLI_runtime_vcl(       u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);
BOOL VLAN_ICLI_runtime_evc(       u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);
BOOL VLAN_ICLI_runtime_gvrp(      u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);

/**
 * \brief Function for showing vlan status
 * \param session_id      [IN] For ICLI_PRINTF
 * \param has_interface   [IN] TRUE if VLANs parameters for a specific interface should be shown.
 * \param plist           [IN] List of interface ports to show (if has_interface is TRUE)
 * \param has_combined    [IN] TRUE if only VLANs configured by a combination should be shown
 * \param has_static      [IN] TRUE if only VLANs configured by static should be shown
 * \param has_nas         [IN] TRUE if only VLANs configured by dot1x should be shown
 * \param has_mvr         [IN] TRUE if only VLANs configured by mvr should be shown
 * \param has_voice_vlan  [IN] TRUE if only VLANs configured by voice vlan should be shown
 * \param has_mstp        [IN] TRUE if only VLANs configured by mstp should be shown
 * \param has_vcl         [IN] TRUE if only VLANs configured by vcl should be shown
 * \param has_erps        [IN] TRUE if only VLANs configured by erps should be shown
 * \param has_evc         [IN] TRUE if only VLANs configured by evc should be shown
 * \param has_gvrp        [IN] TRUE if only VLANs configured by gvrp should be shown
 * \param has_all         [IN] TRUE to show all VLANs configured
 * \param has_conflicts   [IN] TRUE if only VLANs that has conflicts should be shown
 * \return "return code"
 **/
vtss_rc VLAN_ICLI_show_status(i32 session_id, BOOL has_interface, icli_stack_port_range_t *plist, BOOL has_combined, BOOL has_static, BOOL has_nas, BOOL has_mvr, BOOL has_voice_vlan, BOOL has_mstp, BOOL has_vcl, BOOL has_erps, BOOL has_all, BOOL has_conflicts, BOOL has_evc, BOOL has_gvrp);

/**
 * \brief Function for converting VLAN static tag to printable string
 * \param tx_tag_type [IN] Static tag type
 * \return string */
const char *VLAN_ICLI_tx_tag_type_to_txt(vlan_tx_tag_type_t tx_tag_type);

vtss_rc VLAN_ICLI_hybrid_port_conf(icli_stack_port_range_t *plist, vlan_port_composite_conf_t *new_conf, BOOL frametype, BOOL ingressfilter, BOOL porttype, BOOL tx_tag);
vtss_rc VLAN_ICLI_pvid_set(icli_stack_port_range_t *plist, vlan_port_mode_t port_mode, vtss_vid_t new_pvid);
vtss_rc VLAN_ICLI_mode_set(icli_stack_port_range_t *plist, vlan_port_mode_t new_mode);
vtss_rc VLAN_ICLI_tag_native_vlan_set(icli_stack_port_range_t *plist, BOOL tag_native_vlan);
char *VLAN_ICLI_port_mode_txt(vlan_port_mode_t mode);
vtss_rc VLAN_ICLI_show_vlan(u32 session_id, icli_unsigned_range_t *vlan_list, char *name, BOOL has_vid, BOOL has_name);
vtss_rc VLAN_ICLI_show_forbidden(u32 session_id, BOOL has_vid, vtss_vid_t vid, BOOL has_name, i8 *name);
vtss_rc VLAN_ICLI_add_remove_forbidden(icli_stack_port_range_t *plist, icli_unsigned_range_t *vlan_list, BOOL has_add);
vtss_rc VLAN_ICLI_allowed_vids_set(icli_stack_port_range_t *plist, vlan_port_mode_t port_mode, icli_unsigned_range_t *vlan_list, BOOL has_default, BOOL has_all, BOOL has_none, BOOL has_add, BOOL has_remove, BOOL has_except);
BOOL VLAN_ICLI_runtime_vlan_name(u32 session_id, icli_runtime_ask_t ask, icli_runtime_t *runtime);

#endif /* _VLAN_ICLI_FUNCTIONS_H_ */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

