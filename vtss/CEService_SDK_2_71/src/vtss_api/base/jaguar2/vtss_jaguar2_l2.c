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

 $Id$
 $Revision$

*/

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_L2
#include "vtss_jaguar2_cil.h"

#if defined(VTSS_ARCH_JAGUAR_2)

/* - CIL functions ------------------------------------------------- */

static vtss_rc jr2_pgid_table_write(vtss_state_t *vtss_state,
                                     u32 pgid, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_aggr_table_write(vtss_state_t *vtss_state,
                                     u32 ac, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_src_table_write(vtss_state_t *vtss_state,
                                    vtss_port_no_t port_no, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_aggr_mode_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_pmap_table_write(vtss_state_t *vtss_state,
                                     vtss_port_no_t port_no, vtss_port_no_t l_port_no)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_learn_state_set(vtss_state_t *vtss_state,
                                    const BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_mac_table_add(vtss_state_t *vtss_state,
                                  const vtss_mac_table_entry_t *const entry, u32 pgid)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_mac_table_del(vtss_state_t *vtss_state, const vtss_vid_mac_t *const vid_mac)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_mac_table_get(vtss_state_t *vtss_state,
                                  vtss_mac_table_entry_t *const entry, u32 *pgid)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_mac_table_get_next(vtss_state_t *vtss_state,
                                       vtss_mac_table_entry_t *const entry, u32 *pgid)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_mac_table_age_time_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_mac_table_age(vtss_state_t *vtss_state,
                                  BOOL             pgid_age, 
                                  u32              pgid,
                                  BOOL             vid_age,
                                  const vtss_vid_t vid)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_mac_table_status_get(vtss_state_t *vtss_state,
                                         vtss_mac_table_status_t *status) 
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_learn_port_mode_set(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

/* ================================================================= *
 *  Layer 2 - VLAN 
 * ================================================================= */

static vtss_rc jr2_vlan_conf_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_vlan_port_conf_update(vtss_state_t *vtss_state,
                                          vtss_port_no_t port_no, vtss_vlan_port_conf_t *conf)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_vlan_mask_update(vtss_state_t *vtss_state,
                                     vtss_vid_t vid, BOOL member[VTSS_PORT_ARRAY_SIZE])
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

/* ================================================================= *
 *  Layer 2 - PVLAN / Isolated ports
 * ================================================================= */

static vtss_rc jr2_isolated_port_members_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

/* ================================================================= *
 *  Layer 2 - IP Multicast
 * ================================================================= */

static vtss_rc jr2_flood_conf_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

/* ================================================================= *
 *  Layer 2 - Mirror
 * ================================================================= */

#ifdef VTSS_FEATURE_MIRROR_CPU
/* CPU Ingress ports subjects for mirroring */
static vtss_rc jr2_mirror_cpu_ingress_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

/* CPU Egress ports subjects for mirroring */
static vtss_rc jr2_mirror_cpu_egress_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}
#endif // VTSS_FEATURE_MIRROR_CPU

static vtss_rc jr2_mirror_port_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_mirror_ingress_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_mirror_egress_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

#ifdef VTSS_FEATURE_SFLOW
/* ================================================================= *
 *  SFLOW
 * ================================================================= */

static vtss_rc jr2_sflow_sampling_rate_convert(struct vtss_state_s *const state, const BOOL power2, const u32 rate_in, u32 *const rate_out)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_sflow_port_conf_set(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no,
                                        const vtss_sflow_port_conf_t *const new_conf)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}
#endif /* VTSS_FEATURE_SFLOW */

/* - Debug print --------------------------------------------------- */

vtss_rc vtss_jr2_l2_debug_print(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

/* - Initialization ------------------------------------------------ */

static vtss_rc jr2_l2_port_map_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_l2_init(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

vtss_rc vtss_jr2_l2_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_l2_state_t *state = &vtss_state->l2;
    
    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->mac_table_add = jr2_mac_table_add;
        state->mac_table_del = jr2_mac_table_del;
        state->mac_table_get = jr2_mac_table_get;
        state->mac_table_get_next = jr2_mac_table_get_next;
        state->mac_table_age_time_set = jr2_mac_table_age_time_set;
        state->mac_table_age = jr2_mac_table_age;
        state->mac_table_status_get = jr2_mac_table_status_get;
        state->learn_port_mode_set = jr2_learn_port_mode_set;
        state->learn_state_set = jr2_learn_state_set;
        state->mstp_state_set = vtss_cmn_mstp_state_set;
        state->mstp_vlan_msti_set = vtss_cmn_vlan_members_set;
        state->erps_vlan_member_set = vtss_cmn_erps_vlan_member_set;
        state->erps_port_state_set = vtss_cmn_erps_port_state_set;
        state->pgid_table_write = jr2_pgid_table_write;
        state->src_table_write = jr2_src_table_write;
        state->aggr_table_write = jr2_aggr_table_write;
        state->aggr_mode_set = jr2_aggr_mode_set;
        state->pmap_table_write = jr2_pmap_table_write;
        state->vlan_conf_set = jr2_vlan_conf_set;
        state->vlan_port_conf_set = vtss_cmn_vlan_port_conf_set;
        state->vlan_port_conf_update = jr2_vlan_port_conf_update;
        state->vlan_port_members_set = vtss_cmn_vlan_members_set;
        state->vlan_mask_update = jr2_vlan_mask_update;
#if defined(VTSS_FEATURE_VLAN_TX_TAG)
        state->vlan_tx_tag_set = vtss_cmn_vlan_tx_tag_set;
#endif /* VTSS_FEATURE_VLAN_TX_TAG */
        state->isolated_vlan_set = vtss_cmn_vlan_members_set;
        state->isolated_port_members_set = jr2_isolated_port_members_set;
        state->flood_conf_set = jr2_flood_conf_set;
#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_FEATURE_IPV6_MC_SIP)
        state->ipv4_mc_add = vtss_cmn_ipv4_mc_add;
        state->ipv4_mc_del = vtss_cmn_ipv4_mc_del;
        state->ipv6_mc_add = vtss_cmn_ipv6_mc_add;
        state->ipv6_mc_del = vtss_cmn_ipv6_mc_del;
        state->ip_mc_update = jr2_ip_mc_update;
#endif /* VTSS_FEATURE_IPV4_MC_SIP || VTSS_FEATURE_IPV6_MC_SIP */
        state->mirror_port_set = jr2_mirror_port_set;
        state->mirror_ingress_set = jr2_mirror_ingress_set;
        state->mirror_egress_set = jr2_mirror_egress_set;
#ifdef VTSS_FEATURE_MIRROR_CPU
        state->mirror_cpu_ingress_set = jr2_mirror_cpu_ingress_set;
        state->mirror_cpu_egress_set = jr2_mirror_cpu_egress_set;
#endif //VTSS_FEATURE_MIRROR_CPU
        state->eps_port_set = vtss_cmn_eps_port_set;
#if defined(VTSS_FEATURE_SFLOW)
        state->sflow_port_conf_set         = jr2_sflow_port_conf_set;
        state->sflow_sampling_rate_convert = jr2_sflow_sampling_rate_convert;
#endif /* VTSS_FEATURE_SFLOW */
#if defined(VTSS_FEATURE_VCL)
        state->vcl_port_conf_set = jr2_vcl_port_conf_set;
        state->vce_add = vtss_cmn_vce_add;
        state->vce_del = vtss_cmn_vce_del;
        state->vlan_trans_group_add = vtss_cmn_vlan_trans_group_add;
        state->vlan_trans_group_del = vtss_cmn_vlan_trans_group_del;
        state->vlan_trans_group_get = vtss_cmn_vlan_trans_group_get;
        state->vlan_trans_port_conf_set  = vtss_cmn_vlan_trans_port_conf_set;
        state->vlan_trans_port_conf_get  = vtss_cmn_vlan_trans_port_conf_get;
#endif /* VTSS_FEATURE_VCL */
        state->ac_count = JR2_ACS;
        break;
    case VTSS_INIT_CMD_INIT:
        VTSS_RC(jr2_l2_init(vtss_state));
        break;
    case VTSS_INIT_CMD_PORT_MAP:
        VTSS_RC(jr2_l2_port_map_set(vtss_state));
        break;
    default:
        break;
    }
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_JAGUAR_2 */
