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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_QOS
#include "vtss_api.h"
#include "vtss_state.h"
#include "vtss_common.h"
#include "vtss_util.h"

#if defined(VTSS_FEATURE_QOS)

/* - QoS configuration --------------------------------------------- */

#if defined(VTSS_ARCH_CARACAL)
vtss_rc vtss_mep_policer_conf_get(const vtss_inst_t       inst,
                                  const vtss_port_no_t    port_no,
                                  const vtss_prio_t       prio,
                                  vtss_dlb_policer_conf_t *const conf)
{
    VTSS_D("port: %u, prio: %u", port_no, prio);
    memset(conf, 0, sizeof(*conf));
    return VTSS_RC_OK;
}

vtss_rc vtss_mep_policer_conf_set(const vtss_inst_t             inst,
                                  const vtss_port_no_t          port_no,
                                  const vtss_prio_t             prio,
                                  const vtss_dlb_policer_conf_t *const conf)
{
    VTSS_D("port: %u, prio: %u", port_no, prio);
    return VTSS_RC_OK;
}
#endif /* defined(VTSS_ARCH_CARACAL) */

vtss_rc vtss_qos_conf_get(const vtss_inst_t  inst,
                          vtss_qos_conf_t    *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        *conf = vtss_state->qos.conf;
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_qos_conf_set(const vtss_inst_t      inst,
                          const vtss_qos_conf_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    vtss_prio_t  prios = conf->prios;
    BOOL         changed;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        if (prios == 1 || prios == 2 || prios == 4 || prios == vtss_state->qos.prio_count) {
            changed = (vtss_state->qos.conf.prios != prios);
            vtss_state->qos.conf = *conf;
            rc = VTSS_FUNC_COLD(qos.conf_set, changed);
        } else {
            VTSS_E("illegal prios: %u", prios);
            rc = VTSS_RC_ERROR;
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_qos_port_conf_get(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               vtss_qos_port_conf_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK)
        *conf = vtss_state->qos.port_conf[port_no];
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_qos_port_conf_set(const vtss_inst_t           inst,
                               const vtss_port_no_t        port_no,
                               const vtss_qos_port_conf_t  *const conf)
{
    vtss_state_t         *vtss_state;
    vtss_rc              rc;
    vtss_qos_port_conf_t *port_conf;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        port_conf = &vtss_state->qos.port_conf[port_no];
        vtss_state->qos.port_conf_old = *port_conf;
        *port_conf = *conf;
        rc = VTSS_FUNC_COLD(qos.port_conf_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}

#if defined(VTSS_ARCH_B2)
vtss_rc vtss_dscp_table_get(const vtss_inst_t           inst,
                            const vtss_dscp_table_no_t  table_no,
                            vtss_dscp_entry_t           dscp_table[VTSS_DSCP_TABLE_SIZE])
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        if (table_no < VTSS_DSCP_TABLE_NO_END)
            memcpy(dscp_table, vtss_state->qos.dscp_table[table_no], sizeof(vtss_dscp_entry_t)*VTSS_DSCP_TABLE_SIZE);
        else
            rc = VTSS_RC_ERROR;
    }
    VTSS_EXIT();
    return rc;

}

vtss_rc vtss_dscp_table_set(const vtss_inst_t           inst,
                            const vtss_dscp_table_no_t  table_no,
                            const vtss_dscp_entry_t     dscp_table[VTSS_DSCP_TABLE_SIZE])
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        memcpy(vtss_state->qos.dscp_table[table_no], dscp_table, sizeof(vtss_dscp_entry_t)*VTSS_DSCP_TABLE_SIZE);
        rc = VTSS_FUNC(qos.dscp_table_set, table_no);
    }
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_ARCH_B2 */

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC) || defined(VTSS_ARCH_B2)
/* Get QoS setup for logical port */
vtss_rc vtss_qos_lport_conf_get(const vtss_inst_t      inst,
                                const vtss_lport_no_t  lport_no,
                                vtss_qos_lport_conf_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    if (conf == NULL) {
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    if (lport_no >= VTSS_LPORTS)
        rc = VTSS_RC_ERROR;
    else if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        *conf = vtss_state->qos.lport[lport_no];
    VTSS_EXIT();
    return rc;
}

/* Set QoS setup for logical port */
vtss_rc vtss_qos_lport_conf_set(const vtss_inst_t            inst,
                                const vtss_lport_no_t        lport_no,
                                const vtss_qos_lport_conf_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    if (conf == NULL) {
        return VTSS_RC_ERROR;
    }

    VTSS_ENTER();
    if (lport_no >= VTSS_LPORTS)
        rc = VTSS_RC_ERROR;
    else if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        vtss_state->qos.lport[lport_no] = *conf;
        rc = VTSS_FUNC(qos.lport_conf_set, lport_no);
    }
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC || VTSS_ARCH_B2 */

/* - QCL configuration --------------------------------------------- */

#if defined(VTSS_FEATURE_QCL)
vtss_rc vtss_qce_init(const vtss_inst_t      inst,
                      const vtss_qce_type_t  type,
                      vtss_qce_t             *const qce)
{
    VTSS_D("type: %d", type);

    memset(qce, 0, sizeof(*qce));
#if defined(VTSS_FEATURE_QCL_V2)
    qce->key.type = type;
#endif /* VTSS_FEATURE_QCL_V2 */

    return VTSS_RC_OK;
}

vtss_rc vtss_qce_add(const vtss_inst_t    inst,
                     const vtss_qcl_id_t  qcl_id,
                     const vtss_qce_id_t  qce_id,
                     const vtss_qce_t     *const qce)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("qce_id: %u before %u %s",
           qce->id, qce_id, qce_id == VTSS_QCE_ID_LAST ? "(last)" : "");

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(qos.qce_add, qcl_id, qce_id, qce);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_qce_del(const vtss_inst_t    inst,
                     const vtss_qcl_id_t  qcl_id,
                     const vtss_qce_id_t  qce_id)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("qce_id: %u", qce_id);

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(qos.qce_del, qcl_id, qce_id);
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_FEATURE_QCL */

/* - DLB policers -------------------------------------------------- */

#if defined(VTSS_FEATURE_QOS_POLICER_DLB)
vtss_rc vtss_evc_policer_id_check(vtss_state_t *vtss_state,
                                  const vtss_evc_policer_id_t policer_id, BOOL resv)
{
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    /* Allow reserved policers */
    if (resv && 
        (policer_id == VTSS_EVC_POLICER_ID_DISCARD || policer_id == VTSS_EVC_POLICER_ID_NONE))
        return VTSS_RC_OK;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    
    if (policer_id >= vtss_state->qos.evc_policer_max) {
        VTSS_E("illegal policer_id: %u", policer_id);
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

vtss_rc vtss_evc_policer_conf_get(const vtss_inst_t           inst,
                                  const vtss_evc_policer_id_t policer_id,
                                  vtss_evc_policer_conf_t     *const conf)
{
    vtss_state_t *vtss_state;

    VTSS_D("policer_id: %u", policer_id);
    VTSS_RC(vtss_inst_check(inst, &vtss_state));
    VTSS_RC(vtss_evc_policer_id_check(vtss_state, policer_id, 0));
    VTSS_ENTER();
    *conf = vtss_state->qos.evc_policer_conf[policer_id];
    VTSS_EXIT();
    return VTSS_RC_OK;
}

vtss_rc vtss_evc_policer_conf_set(const vtss_inst_t             inst,
                                  const vtss_evc_policer_id_t   policer_id,
                                  const vtss_evc_policer_conf_t *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("policer_id: %u", policer_id);
    VTSS_RC(vtss_inst_check(inst, &vtss_state));
    VTSS_RC(vtss_evc_policer_id_check(vtss_state, policer_id, 0));
    VTSS_ENTER();
    vtss_state->qos.evc_policer_conf[policer_id] = *conf;
    rc = VTSS_FUNC_COLD(qos.evc_policer_conf_set, policer_id);
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_FEATURE_QOS_POLICER_DLB */

/* - Warm start synchronization ------------------------------------ */

#if defined(VTSS_FEATURE_WARM_START)
vtss_rc vtss_qos_restart_sync(vtss_state_t *vtss_state)
{
    vtss_port_no_t port_no;

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_state->qos.port_conf_old = vtss_state->qos.port_conf[port_no];
        VTSS_FUNC_RC(qos.port_conf_set, port_no);
    }

    VTSS_FUNC_RC(qos.conf_set, 1);

#if defined(VTSS_FEATURE_QOS_POLICER_DLB)
    {
        vtss_evc_policer_id_t policer_id;

        for (policer_id = 0; policer_id < vtss_state->qos.evc_policer_max; policer_id++) {
            VTSS_FUNC_RC(qos.evc_policer_conf_set, policer_id);
        }
    }
#endif /* VTSS_FEATURE_QOS_POLICER_DLB */

    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_WARM_START */

/* - Instance create and initialization ---------------------------- */

vtss_rc vtss_qos_inst_create(struct vtss_state_s *vtss_state)
{
    vtss_port_no_t     port_no;
    u32                i;
    vtss_burst_level_t level = 0;

    {
        vtss_qos_conf_t *qos = &vtss_state->qos.conf;

        qos->prios = vtss_state->qos.prio_count;

#if defined(VTSS_FEATURE_QOS_DSCP_REMARK)
        for (i = 0; i < 64; i++) {
            qos->dscp_remark[i] = FALSE;
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
            qos->dscp_translate_map[i] = i;
            qos->dscp_remap[i] = i;
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
            qos->dscp_remap_dp1[i] = i;
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
        }
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */

#if defined(VTSS_FEATURE_QOS_POLICER_CPU_SWITCH)
        qos->policer_mac = VTSS_PACKET_RATE_DISABLED;
        qos->policer_cat = VTSS_PACKET_RATE_DISABLED;
        qos->policer_learn = VTSS_PACKET_RATE_DISABLED;
#endif /* defined(VTSS_FEATURE_QOS_POLICER_CPU_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH)
        qos->policer_uc = VTSS_PACKET_RATE_DISABLED;
#endif /* defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH)
        qos->policer_mc = VTSS_PACKET_RATE_DISABLED;
#endif /* defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH)
        qos->policer_bc = VTSS_PACKET_RATE_DISABLED;
#endif /* defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) */

#if defined(VTSS_FEATURE_QOS_WRED_V2)
        {
            int queue, dpl;

            for (queue = VTSS_QUEUE_START; queue < VTSS_QUEUE_END; queue++) {
                for (dpl = 0; dpl < 2; dpl++) {
                    qos->red_v2[queue][dpl].enable   = FALSE;
                    qos->red_v2[queue][dpl].min_fl   = 0;
                    qos->red_v2[queue][dpl].max      = 50;
                    qos->red_v2[queue][dpl].max_unit = VTSS_WRED_V2_MAX_DP; /* Defaults to 50% drop probability at 100% fill level */
                }
            }
        }
#endif /* VTSS_FEATURE_QOS_WRED_V2 */
    } /* Global configuration */

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    level = 4096;
#endif /* defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL) */

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        vtss_qos_port_conf_t *qos = &vtss_state->qos.port_conf[port_no];

        for (i = 0; i < VTSS_PORT_POLICERS; i++) {
            qos->policer_port[i].level                    = level;
            qos->policer_port[i].rate                     = VTSS_BITRATE_DISABLED;

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT)
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS)
            qos->policer_ext_port[i].frame_rate           = FALSE;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS */
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL)
            qos->policer_ext_port[i].dp_bypass_level      = 0;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL */
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM)
            qos->policer_ext_port[i].unicast              = TRUE;
            qos->policer_ext_port[i].multicast            = TRUE;
            qos->policer_ext_port[i].broadcast            = TRUE;
            qos->policer_ext_port[i].uc_no_flood          = FALSE;
            qos->policer_ext_port[i].mc_no_flood          = FALSE;
            qos->policer_ext_port[i].flooded              = TRUE;
            qos->policer_ext_port[i].learning             = TRUE;
            qos->policer_ext_port[i].to_cpu               = TRUE;
            {
                int q;
                for (q = 0; q < VTSS_PORT_POLICER_CPU_QUEUES; q++) {
                    qos->policer_ext_port[i].cpu_queue[q]     = TRUE;
                }
            }
            qos->policer_ext_port[i].limit_noncpu_traffic = TRUE;
            qos->policer_ext_port[i].limit_cpu_traffic    = FALSE;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM */
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FC)
            qos->policer_ext_port[i].flow_control         = FALSE;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FC */
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT */
        }
        
#ifdef VTSS_FEATURE_QOS_QUEUE_POLICER
        for (i = VTSS_QUEUE_START; i < VTSS_QUEUE_END; i++) {
            qos->policer_queue[i].level = level;
            qos->policer_queue[i].rate  = VTSS_BITRATE_DISABLED;
        }
#endif /* VTSS_FEATURE_QOS_QUEUE_POLICER */
        
        qos->shaper_port.rate  = VTSS_BITRATE_DISABLED;
        qos->shaper_port.level = level;
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
        qos->shaper_port.eir   = VTSS_BITRATE_DISABLED;
        qos->shaper_port.ebs   = level;
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */

#if defined(VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS)
        for (i = VTSS_QUEUE_START; i < VTSS_QUEUE_END; i++) {
            qos->shaper_queue[i].rate  = VTSS_BITRATE_DISABLED;
            qos->shaper_queue[i].level = level;
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
            qos->shaper_queue[i].eir   = VTSS_BITRATE_DISABLED;
            qos->shaper_queue[i].ebs   = level;
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
            qos->excess_enable[i] = FALSE;
        }
#endif  /* VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS */

#if defined(VTSS_FEATURE_QOS_WRED)
#if defined(VTSS_ARCH_JAGUAR_1)
        for (i = VTSS_QUEUE_START; i < VTSS_QUEUE_END; i++) {
            qos->red[i].enable = FALSE;
            qos->red[i].max_th = 100;
            qos->red[i].min_th = 0;
            qos->red[i].max_prob_1 = 1;
            qos->red[i].max_prob_2 = 5;
            qos->red[i].max_prob_3 = 10;
        }
#endif /* VTSS_ARCH_JAGUAR_1 */
#endif /* VTSS_FEATURE_QOS_WRED */

#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
        qos->default_dpl = 0;
        qos->default_dei = 0;
        qos->tag_class_enable = FALSE;
        for (i = VTSS_PCP_START; i < VTSS_PCP_ARRAY_SIZE; i++) {
            int dei;
            for (dei = VTSS_DEI_START; dei < VTSS_DEI_ARRAY_SIZE; dei++) {
                qos->qos_class_map[i][dei] = vtss_cmn_pcp2qos(i);
                qos->dp_level_map[i][dei] = dei; // Defaults to same value as DEI
            }
        }
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */
        
#if defined(VTSS_FEATURE_QOS_SCHEDULER_V2)
        qos->dwrr_enable = FALSE;
        for (i = VTSS_QUEUE_START; i < (VTSS_QUEUE_END - 2); i++) {
            qos->queue_pct[i] = 17; // ~17% to each queue (17 * 6 ~ 100%)
        }
#endif /* VTSS_FEATURE_QOS_SCHEDULER_V2 */
        
#if defined(VTSS_FEATURE_QOS_TAG_REMARK_V2)
        qos->tag_remark_mode = VTSS_TAG_REMARK_MODE_CLASSIFIED;
        qos->tag_default_pcp = 0;
        qos->tag_default_dei = 0;
        qos->tag_dp_map[0] = 0;
        qos->tag_dp_map[1] = 1;
        qos->tag_dp_map[2] = 1;
        qos->tag_dp_map[3] = 1;
        for (i = VTSS_PRIO_START; i < VTSS_PRIO_END; i++) {
            int dpl;
            for (dpl = 0; dpl < 2; dpl++) {
                qos->tag_pcp_map[i][dpl] = vtss_cmn_pcp2qos(i);
                qos->tag_dei_map[i][dpl] = dpl; // Defaults to same value as DP level
            }
        }
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */

#if defined(VTSS_ARCH_B2)
        qos->vlan_aware = 1; /* Enable VLAN awareness for QoS and filtering */
        qos->custom_filter.local.forward = 1;
        for (i = VTSS_QUEUE_START; i < VTSS_QUEUE_END; i++)
            qos->policer_queue[i].rate = VTSS_BITRATE_DISABLED;
        qos->shaper_port.rate = VTSS_BITRATE_DISABLED;
        qos->shaper_host.rate = VTSS_BITRATE_DISABLED;
        qos->scheduler.rate = VTSS_BITRATE_DISABLED;
        qos->scheduler.queue_pct[0] = 0;  /* Q0 */
        qos->scheduler.queue_pct[1] = 10; /* Q1 */
        qos->scheduler.queue_pct[2] = 15; /* Q2 */
        qos->scheduler.queue_pct[3] = 20; /* Q3 */
        qos->scheduler.queue_pct[4] = 25; /* Q4 */
        qos->scheduler.queue_pct[5] = 30; /* Q5 */
        
        qos->default_qos.prio = VTSS_PRIO_START;
        qos->l2_control_qos.prio = VTSS_PRIO_START;
        qos->l3_control_qos.prio = VTSS_PRIO_START;
        qos->cfi_qos.prio = VTSS_PRIO_START;
        qos->ipv4_arp_qos.prio = VTSS_PRIO_START;
        qos->ipv6_qos.prio = VTSS_PRIO_START;
        for (i = 0; i < 8; i++) {
            qos->mpls_exp_qos[i].prio = VTSS_PRIO_START;
            qos->vlan_tag_qos[i].prio = VTSS_PRIO_START;
        }
        qos->llc_qos.prio = VTSS_PRIO_START;
        qos->dscp_table_no = VTSS_DSCP_TABLE_NO_START;
        for (i = 0; i < 2; i++)
            qos->udp_tcp.local.pair[i].qos.prio = VTSS_PRIO_START;
        qos->ip_proto.qos.prio = VTSS_PRIO_START;
        qos->etype.qos.prio = VTSS_PRIO_START;
        for (i = 0; i < 2; i++)
            qos->vlan[i].qos.prio = VTSS_PRIO_START;
        qos->custom_filter.local.forward = 1;
        qos->custom_filter.local.qos.prio = VTSS_PRIO_START;
#endif /* VTSS_ARCH_B2 */
        
#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
        qos->tx_rate_limiter.preamble_in_payload = TRUE;
        qos->tx_rate_limiter.payload_rate        = 100;
        qos->tx_rate_limiter.frame_overhead      = 12;
        qos->tx_rate_limiter.frame_rate          = 0x40;
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */
    } /* Port loop */

#if defined(VTSS_ARCH_B2)
    {
        u32 dscp;

        for (i = 0; i < VTSS_DSCP_TABLE_COUNT; i++)
            for (dscp = 0; dscp < VTSS_DSCP_TABLE_SIZE; dscp++)
                vtss_state->qos.dscp_table[i][dscp].qos.prio = VTSS_PRIO_START;
    }
#endif /* VTSS_ARCH_B2 */

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    {
        vtss_lport_no_t lport_no;

        /* Lport and Lport Queue QoS defaults */
        for (lport_no = VTSS_PORT_NO_START; lport_no < VTSS_LPORTS; lport_no++) {
            vtss_qos_lport_conf_t *qos = &vtss_state->qos.lport[lport_no];
            qos->shaper.rate = VTSS_BITRATE_DISABLED;
            qos->shaper.level = level;
            qos->lport_pct = 17;
            for (i = 0; i < VTSS_PRIOS; i++) {
                qos->red[i].enable = FALSE;
                qos->red[i].max_th = 100;
                qos->red[i].min_th = 0;
                qos->red[i].max_prob_1 = 1;
                qos->red[i].max_prob_2 = 5;
                qos->red[i].max_prob_3 = 10;
            }
            for (i = 0; i < 2; i++) {
                qos->scheduler.queue[i].rate  = VTSS_BITRATE_DISABLED;
                qos->scheduler.queue[i].level = level;
            }
        }
    }
#endif /* (VTSS_ARCH_JAGUAR_1_CE_MAC) */

#if defined(VTSS_FEATURE_QCL)
    {
        vtss_qcl_id_t    qcl_id;
        vtss_qcl_t       *qcl;
        vtss_qcl_entry_t *qcl_entry;

        /* Initialize QCL free/used lists */
        for (qcl_id = VTSS_QCL_ID_START; qcl_id < VTSS_QCL_ID_END; qcl_id++) {
            qcl = &vtss_state->qos.qcl[qcl_id];
            for (i = 0; i < VTSS_QCL_LIST_SIZE; i++) {
                qcl_entry = &qcl->qcl_list[i];
                /* Insert in free list */
                qcl_entry->next = qcl->qcl_list_free;
                qcl->qcl_list_free = qcl_entry;
            }
        }
    }
#endif /* VTSS_FEATURE_QCL */
    
#if defined(VTSS_FEATURE_QOS_POLICER_DLB)
    vtss_state->qos.evc_policer_max = VTSS_EVC_POLICERS;
#endif /* VTSS_FEATURE_QOS_POLICER_DLB */

    return VTSS_RC_OK;
}

/* - QoS utilities ------------------------------------------------- */

vtss_rc vtss_cmn_qos_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_rc                  rc = vtss_state->qos.port_conf_update(vtss_state, port_no);
#if defined(VTSS_FEATURE_ES0)
    vtss_qos_port_conf_t     *old = &vtss_state->qos.port_conf_old;
    vtss_qos_port_conf_t     *new = &vtss_state->qos.port_conf[port_no];
    u8                       pcp;
    u16                      flags = 0;

    if (old->tag_remark_mode != new->tag_remark_mode ||
        old->tag_default_pcp != new->tag_default_pcp ||
        old->tag_default_dei != new->tag_default_dei) {
        /* PCP/DEI remark mode changed, update QOS */
        flags |= VTSS_ES0_FLAG_QOS;
    }
    for (pcp = 0; pcp < 8; pcp++) {
        if (old->qos_class_map[pcp][0] != new->qos_class_map[pcp][0]) {
            flags |= VTSS_ES0_FLAG_PCP_MAP;
            break;
        }
    }
    if (flags) {
        /* Update ES0 rules */
        VTSS_RC(vtss_vcap_es0_update(vtss_state, port_no, flags));
    }
#endif /* VTSS_FEATURE_ES0 */
    return rc;
}

/**
 * \brief Convert QoS scheduler weight to cost.
 *
 * \param weight [IN]    Array of weights. Range is 1..100.
 * \param cost [OUT]     Array of costs. Range is 0..(2^bit_width - 1).
 * \param num [IN]       Number of entries in weight and cost.
 * \param bit_width [IN] Bit-width of resulting cost. Range is 4..8. E.g. 5 corresponds to a cost between 0 and 31.
 *
 * \return Return code.
 **/
vtss_rc vtss_cmn_qos_weight2cost(const vtss_pct_t * weight, u8 * cost, size_t num, u8 bit_width)
{
    u32 i, c_max;
    vtss_pct_t w_min = 100;
    if ((bit_width < 4) || (bit_width > 8)) {
        VTSS_E("illegal bit_width: %u", bit_width);
        return VTSS_RC_ERROR;
    }
    c_max = 1 << bit_width;
    for (i = 0; i < num; i++) {
        if ((weight[i] < 1) || (weight[i] > 100)) {
            VTSS_E("illegal weight: %u", weight[i]);
            return VTSS_RC_ERROR;
        }
        w_min = MIN(w_min, weight[i]);
    }
    for (i = 0; i < num; i++) {
        // Round half up: Multiply with 16 before division, add 8 and divide result with 16 again
        u32 c = (((c_max << 4) * w_min / weight[i]) + 8) >> 4;
        cost[i] = MAX(1, c) - 1; // Force range to be 0..(c_max - 1)
    }
    return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_QCL_V2)
/* Add QCE */
vtss_rc vtss_cmn_qce_add(vtss_state_t *vtss_state,
                         const vtss_qcl_id_t  qcl_id,
                         const vtss_qce_id_t  qce_id,
                         const vtss_qce_t     *const qce)
{
    vtss_vcap_obj_t             *is1_obj = &vtss_state->vcap.is1.obj;
    vtss_vcap_user_t            is1_user = VTSS_IS1_USER_QOS;
    vtss_vcap_data_t            data;
    vtss_is1_data_t             *is1 = &data.u.is1;
    vtss_is1_entry_t            entry;
    vtss_is1_action_t           *action = &entry.action;
    vtss_is1_key_t              *key = &entry.key;
    vtss_res_chg_t              res_chg;
    vtss_vcap_key_size_t        key_size = VTSS_VCAP_KEY_SIZE_FULL;
    vtss_vcap_range_chk_table_t range_new = vtss_state->vcap.range; /* Make a temporary working copy of the range table */

    /* Check QCE ID */
    if (qce->id == VTSS_QCE_ID_LAST || qce->id == qce_id) {
        VTSS_E("illegal qce id: %u", qce->id);
        return VTSS_RC_ERROR;
    }

    /* Initialize entry data */
    vtss_vcap_is1_init(&data, &entry);
    
    is1->lookup = 1; /* Second lookup */

#if defined(VTSS_ARCH_SERVAL)
    /* For Serval, the configured key of the first port is used */
    if (vtss_state->arch == VTSS_ARCH_SRVL) {
        vtss_port_no_t port_no;
        is1->lookup = 2; /* Third lookup */

        key->key_type = VTSS_VCAP_KEY_TYPE_DOUBLE_TAG; /* Default if no ports assigned */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            if (qce->key.port_list[port_no]) {
                key->key_type = vtss_state->qos.port_conf[port_no].key_type;
                VTSS_D("Using key_type %u from port %u", key->key_type, port_no);
                break; /* Stop after first port */
            }
        }
        key_size = vtss_vcap_key_type_to_size(key->key_type);
    }
#endif /* VTSS_ARCH_SERVAL */
    
    /* Check if main entry exists */
    memset(&res_chg, 0, sizeof(res_chg));
    if (vtss_vcap_lookup(vtss_state, is1_obj, is1_user, qce->id, &data, NULL) == VTSS_RC_OK) {
        is1->entry = &entry; /* NOTE: Restore entry pointer which was overwritten by vtss_vcap_lookup() */
        res_chg.del_key[key_size] = 1;
        /* Free eventually old range checkers */
        VTSS_RC(vtss_vcap_range_free(&range_new, is1->vid_range));
        VTSS_RC(vtss_vcap_range_free(&range_new, is1->dscp_range));
        VTSS_RC(vtss_vcap_range_free(&range_new, is1->sport_range));
        VTSS_RC(vtss_vcap_range_free(&range_new, is1->dport_range));
        is1->vid_range = VTSS_VCAP_RANGE_CHK_NONE;
        is1->dscp_range = VTSS_VCAP_RANGE_CHK_NONE;
        is1->sport_range = VTSS_VCAP_RANGE_CHK_NONE;
        is1->dport_range = VTSS_VCAP_RANGE_CHK_NONE;
    }

    /* Check that the entry can be added */
    res_chg.add_key[key_size] = 1;
    VTSS_RC(vtss_cmn_vcap_res_check(is1_obj, &res_chg));
    data.key_size = key_size;

    /* Copy action data */
    action->dscp_enable    = qce->action.dscp_enable;
    action->dscp           = qce->action.dscp;
    action->dp_enable      = qce->action.dp_enable;
    action->dp             = qce->action.dp;
    action->prio_enable    = qce->action.prio_enable;
    action->prio           = qce->action.prio;
#if (defined VTSS_FEATURE_QCL_PCP_DEI_ACTION)
    action->pcp_dei_enable = qce->action.pcp_dei_enable;
    action->pcp            = qce->action.pcp;
    action->dei            = qce->action.dei;
#endif /* (defined VTSS_FEATURE_QCL_PCP_DEI_ACTION) */
#if (defined VTSS_FEATURE_QCL_POLICY_ACTION)
    action->pag_enable     = qce->action.policy_no_enable;
    action->pag            = qce->action.policy_no;
#endif /* VTSS_FEATURE_QCL_POLICY_ACTION */

    /* Copy key data */
    memcpy(key->port_list, qce->key.port_list, sizeof(key->port_list));

    key->mac.dmac_mc = qce->key.mac.dmac_mc;
    key->mac.dmac_bc = qce->key.mac.dmac_bc;
    if (key->mac.dmac_bc == VTSS_VCAP_BIT_1) {
        key->mac.dmac_mc = VTSS_VCAP_BIT_1; /* mc must be 1 or don't care in order to match on a bc frame */
    }
#if defined(VTSS_ARCH_SERVAL)
    key->mac.dmac = qce->key.mac.dmac;
#endif /* VTSS_ARCH_SERVAL */
    key->mac.smac = qce->key.mac.smac; /* Only the 24 most significant bits (OUI) are supported on Jaguar1, rest are wildcards */

    key->tag.vid    = qce->key.tag.vid;
    VTSS_RC(vtss_vcap_vr_alloc(&range_new, &is1->vid_range, VTSS_VCAP_RANGE_TYPE_VID, &key->tag.vid));
    key->tag.pcp    = qce->key.tag.pcp;
    key->tag.dei    = qce->key.tag.dei;
    key->tag.tagged = qce->key.tag.tagged;
#if defined(VTSS_ARCH_SERVAL)
    key->tag.s_tag  = qce->key.tag.s_tag;

    key->inner_tag.vid    = qce->key.inner_tag.vid;
    key->inner_tag.pcp    = qce->key.inner_tag.pcp;
    key->inner_tag.dei    = qce->key.inner_tag.dei;
    key->inner_tag.tagged = qce->key.inner_tag.tagged;
    key->inner_tag.s_tag  = qce->key.inner_tag.s_tag;
#endif /* VTSS_ARCH_SERVAL */

    switch (qce->key.type) {
    case VTSS_QCE_TYPE_ANY:
        key->type = VTSS_IS1_TYPE_ANY;
        break;
    case VTSS_QCE_TYPE_ETYPE:
        key->type              = VTSS_IS1_TYPE_ETYPE;
        key->frame.etype.etype = qce->key.frame.etype.etype;
        key->frame.etype.data  = qce->key.frame.etype.data;
#if defined(VTSS_ARCH_JAGUAR_1)
        /* Jaguar classifies MPLS frames to EtherType 0x0000. If we want to match
         * specific on MPLS frames we will have to match on EtherType 0x0000 instead.
         *
         * Check if user wants to match on specific MPLS frame (EtherType = 0x8847 or 0x8848) */
        if ((vtss_state->arch == VTSS_ARCH_JR1) &&
            (qce->key.frame.etype.etype.mask[0] == 0xff) &&
            (qce->key.frame.etype.etype.mask[1] == 0xff) &&
            (qce->key.frame.etype.etype.value[0] == 0x88) &&
            ((qce->key.frame.etype.etype.value[1] == 0x47) || (qce->key.frame.etype.etype.value[1] == 0x48))) {
            key->frame.etype.etype.value[0] = 0; /* Change EtherType to 0x0000 */
            key->frame.etype.etype.value[1] = 0;
        }
#endif /* VTSS_ARCH_JAGUAR_1 */
        break;
    case VTSS_QCE_TYPE_LLC:
        key->type           = VTSS_IS1_TYPE_LLC;
        key->frame.llc.data = qce->key.frame.llc.data;
        break;
    case VTSS_QCE_TYPE_SNAP:
        key->type            = VTSS_IS1_TYPE_SNAP;
        key->frame.snap.data = qce->key.frame.snap.data;
        break;
    case VTSS_QCE_TYPE_IPV4:
    case VTSS_QCE_TYPE_IPV6:
        if (qce->key.type == VTSS_QCE_TYPE_IPV4) {
            key->type                = VTSS_IS1_TYPE_IPV4;
            key->frame.ipv4.fragment = qce->key.frame.ipv4.fragment;
            key->frame.ipv4.dscp     = qce->key.frame.ipv4.dscp;
            VTSS_RC(vtss_vcap_vr_alloc(&range_new, &is1->dscp_range, VTSS_VCAP_RANGE_TYPE_DSCP, &key->frame.ipv4.dscp));
            key->frame.ipv4.proto    = qce->key.frame.ipv4.proto;
            key->frame.ipv4.sip      = qce->key.frame.ipv4.sip;
#if defined(VTSS_ARCH_SERVAL)
            key->frame.ipv4.dip      = qce->key.frame.ipv4.dip;
#endif /* VTSS_ARCH_SERVAL */
            key->frame.ipv4.sport    = qce->key.frame.ipv4.sport;
            VTSS_RC(vtss_vcap_vr_alloc(&range_new, &is1->sport_range, VTSS_VCAP_RANGE_TYPE_SPORT, &key->frame.ipv4.sport));
            key->frame.ipv4.dport    = qce->key.frame.ipv4.dport;
            VTSS_RC(vtss_vcap_vr_alloc(&range_new, &is1->dport_range, VTSS_VCAP_RANGE_TYPE_DPORT, &key->frame.ipv4.dport));
        } else {
            key->type                = VTSS_IS1_TYPE_IPV6;
            key->frame.ipv6.dscp     = qce->key.frame.ipv6.dscp;
            VTSS_RC(vtss_vcap_vr_alloc(&range_new, &is1->dscp_range, VTSS_VCAP_RANGE_TYPE_DSCP, &key->frame.ipv6.dscp));
            key->frame.ipv6.proto    = qce->key.frame.ipv6.proto;
            key->frame.ipv6.sip      = qce->key.frame.ipv6.sip;
#if defined(VTSS_ARCH_SERVAL)
            key->frame.ipv6.dip      = qce->key.frame.ipv6.dip;
#endif /* VTSS_ARCH_SERVAL */
            key->frame.ipv6.sport    = qce->key.frame.ipv6.sport;
            VTSS_RC(vtss_vcap_vr_alloc(&range_new, &is1->sport_range, VTSS_VCAP_RANGE_TYPE_SPORT, &key->frame.ipv6.sport));
            key->frame.ipv6.dport    = qce->key.frame.ipv6.dport;
            VTSS_RC(vtss_vcap_vr_alloc(&range_new, &is1->dport_range, VTSS_VCAP_RANGE_TYPE_DPORT, &key->frame.ipv6.dport));
        }
        break;
    default:
        VTSS_E("illegal type: %d", qce->key.type);
        return VTSS_RC_ERROR;
    }

    /* Commit range checkers */
    VTSS_RC(vtss_vcap_range_commit(vtss_state, &range_new));

    /* Add main entry */
    VTSS_RC(vtss_vcap_add(vtss_state, is1_obj, is1_user, qce->id, qce_id, &data, 0));
    return VTSS_RC_OK;
}

/* Delete QCE */
vtss_rc vtss_cmn_qce_del(vtss_state_t *vtss_state,
                         const vtss_qcl_id_t  qcl_id,
                         const vtss_qce_id_t  qce_id)
{
    vtss_vcap_obj_t  *obj = &vtss_state->vcap.is1.obj;
    vtss_vcap_data_t data;
    vtss_is1_data_t  *is1 = &data.u.is1;
    
    if (vtss_vcap_lookup(vtss_state, obj, VTSS_IS1_USER_QOS, qce_id, &data, NULL) != VTSS_RC_OK) {
        VTSS_E("qce_id: %u not found", qce_id);
        return VTSS_RC_ERROR;
    }
    
    /* Delete range checkers and main entry */
    VTSS_RC(vtss_vcap_range_free(&vtss_state->vcap.range, is1->vid_range));
    VTSS_RC(vtss_vcap_range_free(&vtss_state->vcap.range, is1->dscp_range));
    VTSS_RC(vtss_vcap_range_free(&vtss_state->vcap.range, is1->sport_range));
    VTSS_RC(vtss_vcap_range_free(&vtss_state->vcap.range, is1->dport_range));
    is1->vid_range = VTSS_VCAP_RANGE_CHK_NONE;
    is1->dscp_range = VTSS_VCAP_RANGE_CHK_NONE;
    is1->sport_range = VTSS_VCAP_RANGE_CHK_NONE;
    is1->dport_range = VTSS_VCAP_RANGE_CHK_NONE;
    VTSS_RC(vtss_vcap_del(vtss_state, obj, VTSS_IS1_USER_QOS, qce_id));
    
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_QCL_V2 */

/* - Debug print --------------------------------------------------- */

#if defined(VTSS_FEATURE_QOS_POLICER_DLB)
void vtss_qos_debug_print_dlb(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
    u32                     i;
    vtss_evc_policer_conf_t *pol_conf;
    BOOL                    cm = 1, header = 1;
    
    for (i = 0; i < VTSS_EVC_POLICERS; i++) {
#if defined(VTSS_ARCH_LUTON26)
        vtss_policer_alloc_t *pol_alloc = &vtss_state->qos.evc_policer_alloc[i];
#endif /* VTSS_ARCH_LUTON26 */
        pol_conf = &vtss_state->qos.evc_policer_conf[i];
        if (pol_conf->enable == 0
#if defined(VTSS_ARCH_LUTON26)
            && pol_alloc->count == 0
#endif /* VTSS_ARCH_LUTON26 */
            )
            continue;
        if (header) {
            header = 0;
            vtss_debug_print_header(pr, "Policers");
            pr("Policer  Type    CM  CF  Rate  CIR         CBS         EIR         EBS         ");
#if defined(VTSS_ARCH_LUTON26)
            pr("Count  L26 Policer");
#endif /* VTSS_ARCH_LUTON26 */
#if defined(VTSS_ARCH_JAGUAR_1)
            pr("TDLB ID");
#endif /* VTSS_ARCH_JAGUAR_1 */
            pr("\n");
        }
#if defined(VTSS_ARCH_JAGUAR_1)
        cm = pol_conf->cm;
#endif /* VTSS_ARCH_JAGUAR_1 */
        pr("%-9u%-8s%-4u%-4u%-6s%-12u%-12u%-12u%-12u",
           i,
           pol_conf->type == VTSS_POLICER_TYPE_MEF ? "MEF" :
           pol_conf->type == VTSS_POLICER_TYPE_SINGLE ? "SINGLE" : "?",
           cm, pol_conf->cf, pol_conf->line_rate ? "Line" : "Data",
           pol_conf->cir, pol_conf->cbs, pol_conf->eir, pol_conf->ebs);
#if defined(VTSS_ARCH_LUTON26)
        pr("%-7u%u", pol_alloc->count, pol_alloc->policer);
#endif /* VTSS_ARCH_LUTON26 */
#if defined(VTSS_ARCH_JAGUAR_1)
        pr("%u", vtss_state->evc.policer_info[i].tdlb_id);
#endif /* VTSS_ARCH_JAGUAR_1 */
        pr("\n");
    }
    if (!header)
        pr("\n");
}
#endif /* VTSS_FEATURE_QOS_POLICER_DLB */

#if defined(VTSS_FEATURE_QOS_POLICER_CPU_SWITCH) || defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) || \
    defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH)  || defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH)
static void vtss_debug_print_packet_rate(const vtss_debug_printf_t pr,
                                         const vtss_debug_info_t   *const info,
                                         const char                *name,
                                         vtss_packet_rate_t        rate)
{
    if (rate == VTSS_PACKET_RATE_DISABLED) {
        pr("%-32s: Disabled\n", name);
    }
    else if (rate >= 1000) {
        pr("%-32s: %u kfps\n", name, rate/1000);
    }
    else {
        pr("%-32s: %u fps\n", name, rate);
    }
}
#endif /* defined(VTSS_FEATURE_QOS_POLICER_CPU_SWITCH) || defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) ||
          defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH)  || defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) */

void vtss_qos_debug_print(vtss_state_t *vtss_state,
                          const vtss_debug_printf_t pr,
                          const vtss_debug_info_t   *const info)
{
    vtss_qos_conf_t *conf = &vtss_state->qos.conf;
    vtss_port_no_t  port_no;
    
    if (!vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_QOS))
        return;
    
    vtss_debug_print_value(pr, "Number of priorities", conf->prios);
#if defined(VTSS_FEATURE_QOS_POLICER_CPU_SWITCH)
    vtss_debug_print_packet_rate(pr, info, "Storm MAC-based",   conf->policer_mac);
    vtss_debug_print_packet_rate(pr, info, "Storm Categorizer", conf->policer_cat);
    vtss_debug_print_packet_rate(pr, info, "Storm Learn",       conf->policer_learn);
#endif /* defined(VTSS_FEATURE_QOS_POLICER_CPU_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH)
    vtss_debug_print_packet_rate(pr, info, "Storm Unicast",     conf->policer_uc);
#endif /* defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH)
    vtss_debug_print_packet_rate(pr, info, "Storm Multicast",   conf->policer_mc);
#endif /* defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH)
    vtss_debug_print_packet_rate(pr, info, "Storm Broadcast",   conf->policer_bc);
#endif /* defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) */
    pr("\n");

#if defined(VTSS_FEATURE_QOS_WRED_V2)
    {
        int queue, dpl;
        pr("QoS WRED Config:\n\n");
        pr("Queue Dpl Ena MinFl Max Dp or Fl\n");
        for (queue = VTSS_QUEUE_START; queue < VTSS_QUEUE_END; queue++) {
            for (dpl = 0; dpl < 2; dpl++) {
                pr("%5d %3d %3d %3d %% %3d %% %-s\n",
                   queue,
                   dpl,
                   VTSS_BOOL(conf->red_v2[queue][dpl].enable),
                   conf->red_v2[queue][dpl].min_fl,
                   conf->red_v2[queue][dpl].max,
                   (conf->red_v2[queue][dpl].max_unit == VTSS_WRED_V2_MAX_DP) ? "Drop Probability" : "Fill Level");
            }
        }
        pr("\n");
    }
#endif  /* VTSS_FEATURE_QOS_WRED_V2 */

#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
    {
        int i;
        pr("QoS DSCP Classification Config:\n\n");
        pr("DSCP Trust QoS DPL\n");
        for (i = 0; i < 64; i++) {
            pr("%4d %5d %3u %3d\n", i, conf->dscp_trust[i], conf->dscp_qos_class_map[i], conf->dscp_dp_level_map[i]);
        }
        pr("\n");
    }
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */

#if defined(VTSS_FEATURE_QOS_DSCP_REMARK)
    {
        int i;
        pr("QoS DSCP Remarking Config:\n\n");
        pr("DSCP I_Remark ");
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
        pr("Translate QoS_Remap");
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
        pr("_DP0 QoS_Remap_DP1");
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
        pr("\n");

        for (i = 0; i < 64; i++) {
            pr("%4d %8d ", i, conf->dscp_remark[i]);
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
            pr("%9d %9d", conf->dscp_translate_map[i], conf->dscp_remap[i]);
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
            pr("     %9d", conf->dscp_remap_dp1[i]);
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
            pr("\n");
        }
        pr("\n");

#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
        pr("QoS DSCP Classification from QoS Config:\n\n");
        pr("QoS DSCP");
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
        pr("_DP0 DSCP_DP1");
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE */
        pr("\n");

        for (i = 0; i < VTSS_PRIO_ARRAY_SIZE; i++) {
            pr("%3d %4d", i, conf->dscp_qos_map[i]);
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
            pr("     %4d", conf->dscp_qos_map_dp1[i]);
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE */
            pr("\n");
        }
        pr("\n");
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
    }
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */

#if defined(VTSS_FEATURE_LAYER2) || defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
    pr("QoS Port Classification Config:\n\n");
    pr("Port PCP CLS ");
#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
    pr("DEI DPL T_EN D_EN");
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */
    pr("\n");

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        if (info->port_list[port_no] == 0)
            continue;
        pr("%4u ", port_no);
#if defined(VTSS_FEATURE_LAYER2)
        pr("%3u %3u ", port_conf->usr_prio, port_conf->default_prio);
#endif /* VTSS_FEATURE_LAYER2 */
#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
        pr("%3u %3u %4u %4u", port_conf->default_dei, port_conf->default_dpl, port_conf->tag_class_enable, port_conf->dscp_class_enable);
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */
        pr("\n");
    }
    pr("\n");
#endif /* defined(VTSS_FEATURE_LAYER2) || defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2) */

#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
    pr("QoS Port Classification PCP, DEI to QoS class, DP level Mapping:\n\n");
    pr("Port QoS class (2*PCP+DEI)           DP level (2*PCP+DEI)\n");

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        int pcp, dei, class_ct = 0, dpl_ct = 0;
        char class_buf[40], dpl_buf[40];
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        if (info->port_list[port_no] == 0)
            continue;
        for (pcp = VTSS_PCP_START; pcp < VTSS_PCP_END; pcp++) {
            for (dei = VTSS_DEI_START; dei < VTSS_DEI_END; dei++) {
                const char *delim = ((pcp == VTSS_PCP_START) && (dei == VTSS_DEI_START)) ? "" : ",";
                class_ct += snprintf(class_buf + class_ct, sizeof(class_buf) - class_ct, "%s%u", delim, port_conf->qos_class_map[pcp][dei]);
                dpl_ct   += snprintf(dpl_buf   + dpl_ct,   sizeof(dpl_buf)   - dpl_ct,   "%s%u",  delim, port_conf->dp_level_map[pcp][dei]);
            }
        }
        pr("%4u %s %s\n", port_no, class_buf, dpl_buf);
    }
    pr("\n");
#endif /* defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2) */

    pr("QoS Port Ingress Policer Config:\n\n");
    pr("Port Policer Burst      Rate");
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FC)
    pr("       FC");
#endif
    pr("\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        int policer;
        if (info->port_list[port_no] == 0)
            continue;
        pr("%4u ", port_no);
        for (policer = 0; policer < VTSS_PORT_POLICERS; policer++) {
            pr("%7d 0x%08x 0x%08x",
               policer,
               port_conf->policer_port[policer].level,
               port_conf->policer_port[policer].rate);
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FC)
            pr(" %d", port_conf->policer_ext_port[policer].flow_control);
#endif
            pr("\n     ");
        }
        pr("\r");
    }
    pr("\n");

#if defined(VTSS_FEATURE_QOS_QUEUE_POLICER)
    pr("QoS Queue Ingress Policer Config:\n\n");
    pr("Port Queue Burst      Rate\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        int queue;
        if (info->port_list[port_no] == 0)
            continue;
        pr("%4u ", port_no);
        for (queue = 0; queue < VTSS_QUEUE_ARRAY_SIZE; queue++) {
            pr("%5d 0x%08x 0x%08x\n     ",
               queue,
               port_conf->policer_queue[queue].level,
               port_conf->policer_queue[queue].rate);
        }
        pr("\r");
    }
    pr("\n");
#endif  /* VTSS_FEATURE_QOS_QUEUE_POLICER */

#if defined(VTSS_FEATURE_QOS_SCHEDULER_V2)
    pr("QoS Port Scheduler Config:\n\n");
    pr("Port Sch Mode  Q0  Q1  Q2  Q3  Q4  Q5\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        int i;
        if (info->port_list[port_no] == 0)
            continue;
        pr("%4u %8s ", port_no, port_conf->dwrr_enable ? "Weighted" : "Strict");
        for (i=0; i<6; i++) {
            pr("%3u ", port_conf->queue_pct[i]);
        } 
        pr("\n");
    }
    pr("\n");
#endif /* VTSS_FEATURE_QOS_SCHEDULER_V2 */

    pr("QoS Port Egress Shaper Config:\n\n");
    pr("Port CBS        CIR        ");
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
    pr("EBS        EIR");
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
    pr("\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        if (info->port_list[port_no] == 0)
            continue;
        pr("%4u 0x%08x 0x%08x ",
           port_no,
           port_conf->shaper_port.level,
           port_conf->shaper_port.rate);
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
        pr("0x%08x 0x%08x ",
           port_conf->shaper_port.ebs,
           port_conf->shaper_port.eir);
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
        pr("\n");
    }
    pr("\n");

#if defined(VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS)
    pr("QoS Queue Egress Shaper Config:\n\n");
    pr("Port Queue CBS        CIR        ");
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
    pr("EBS        EIR        ");
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
    pr("Excess\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        int queue;
        if (info->port_list[port_no] == 0)
            continue;
        pr("%4u ", port_no);
        for (queue = 0; queue < VTSS_QUEUE_ARRAY_SIZE; queue++) {
            pr("%5d 0x%08x 0x%08x ",
               queue,
               port_conf->shaper_queue[queue].level,
               port_conf->shaper_queue[queue].rate);
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
            pr("0x%08x 0x%08x ",
               port_conf->shaper_queue[queue].ebs,
               port_conf->shaper_queue[queue].eir);
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
            pr("%d\n     ", VTSS_BOOL(port_conf->excess_enable[queue]));
        }
        pr("\r");
    }
    pr("\n");
#endif  /* VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS */

#if defined(VTSS_FEATURE_QOS_TAG_REMARK_V2)
    pr("QoS Port Tag Remarking Config:\n\n");
    pr("Port Mode       PCP DEI DPL mapping\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        const char           *mode;
        if (info->port_list[port_no] == 0)
            continue;

        switch (port_conf->tag_remark_mode) {
        case VTSS_TAG_REMARK_MODE_CLASSIFIED:
            mode = "Classified";
            break;
        case VTSS_TAG_REMARK_MODE_DEFAULT:
            mode = "Default";
            break;
        case VTSS_TAG_REMARK_MODE_MAPPED:
            mode = "Mapped";
            break;
        default:
            mode = "?";
            break;
        }
        pr("%4u %-10s %3d %3d   %d %d %d %d\n",
           port_no,
           mode,
           port_conf->tag_default_pcp,
           port_conf->tag_default_dei,
           port_conf->tag_dp_map[0],
           port_conf->tag_dp_map[1],
           port_conf->tag_dp_map[2],
           port_conf->tag_dp_map[3]);
    }
    pr("\n");

    pr("QoS Port Tag Remarking Map:\n\n");
    pr("Port PCP (2*QoS class+DPL)           DEI (2*QoS class+DPL)\n");

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        int class, dpl, pcp_ct = 0, dei_ct = 0;
        char pcp_buf[40], dei_buf[40];
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        if (info->port_list[port_no] == 0)
            continue;
        for (class = VTSS_QUEUE_START; class < VTSS_QUEUE_END; class++) {
            for (dpl = 0; dpl < 2; dpl++) {
                const char *delim = ((class == VTSS_QUEUE_START) && (dpl == 0)) ? "" : ",";
                pcp_ct += snprintf(pcp_buf + pcp_ct, sizeof(pcp_buf) - pcp_ct, "%s%u", delim, port_conf->tag_pcp_map[class][dpl]);
                dei_ct += snprintf(dei_buf + dei_ct, sizeof(dei_buf) - dei_ct, "%s%u",  delim, port_conf->tag_dei_map[class][dpl]);
            }
        }
        pr("%4u %s %s\n", port_no, pcp_buf, dei_buf);
    }
    pr("\n");
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */

#if defined(VTSS_FEATURE_QOS_DSCP_REMARK)
    pr("QoS Port DSCP Remarking Config:\n\n");
    pr("Port I_Mode E_Mode ");
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
    pr("T_EN");
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
    pr("\n");

    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        if (info->port_list[port_no] == 0)
            continue;
        pr("%4u %6d ", port_no, port_conf->dscp_mode);
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
        pr("%6d %4d", port_conf->dscp_emode, port_conf->dscp_translate);
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
        pr("\n");
    }
    pr("\n");
#endif  /* VTSS_FEATURE_QOS_DSCP_REMARK */

#if defined(VTSS_FEATURE_QOS_WRED)
#if defined(VTSS_ARCH_JAGUAR_1)
    pr("QoS WRED Config:\n\n");
    pr("Port Queue Ena MinTh MaxTh MaxP1 MaxP2 MaxP3\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        int queue;
        if (info->port_list[port_no] == 0)
            continue;
        pr("%4u ", port_no);
        for (queue = 0; queue < VTSS_QUEUE_ARRAY_SIZE; queue++) {
            pr("%5d %3d %5d %5d %5d %5d %5d\n     ",
               queue,
               VTSS_BOOL(port_conf->red[queue].enable),
               port_conf->red[queue].min_th,
               port_conf->red[queue].max_th,
               port_conf->red[queue].max_prob_1,
               port_conf->red[queue].max_prob_2,
               port_conf->red[queue].max_prob_3);
        }
        pr("\r");
    }
    pr("\n");
#endif /* VTSS_ARCH_JAGUAR_1 */
#endif  /* VTSS_FEATURE_QOS_WRED */

#if defined(VTSS_FEATURE_QCL_DMAC_DIP) || defined(VTSS_ARCH_SERVAL)
    pr("QoS QCL Port Config:\n\n");
    pr("Port ");
#if defined(VTSS_FEATURE_QCL_DMAC_DIP)
    pr("Address  ");
#endif
#if defined(VTSS_ARCH_SERVAL)
    pr("KeyType ");
#endif
    pr("\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        vtss_qos_port_conf_t *port_conf = &vtss_state->qos.port_conf[port_no];
        if (info->port_list[port_no] == 0) {
            continue;
        }
        pr("%4u ", port_no);
#if defined(VTSS_FEATURE_QCL_DMAC_DIP)
        pr("%s ", port_conf->dmac_dip ? "DMAC_DIP" : "SMAC_SIP");
#endif
#if defined(VTSS_ARCH_SERVAL)
        pr("%s ", vtss_vcap_key_type_txt(port_conf->key_type));
#endif
        pr("\n");
    }
    pr("\n");
#endif /* defined(VTSS_FEATURE_QCL_DMAC_DIP) || defined(VTSS_ARCH_SERVAL) */

#if defined(VTSS_FEATURE_VCAP)
    vtss_vcap_debug_print_range_checkers(vtss_state, pr, info);
#endif /* VTSS_FEATURE_VCAP */

#if defined(VTSS_FEATURE_IS1)
    vtss_vcap_debug_print_is1(vtss_state, pr, info);
#endif /* VTSS_FEATURE_IS1 */    
}

#endif /* VTSS_FEATURE_QOS */
