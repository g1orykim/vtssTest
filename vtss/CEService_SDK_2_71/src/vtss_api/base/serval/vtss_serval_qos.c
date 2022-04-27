/*

 Vitesse API software.

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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_QOS
#include "vtss_serval_cil.h"

#if defined(VTSS_ARCH_SERVAL)

/* - CIL functions ------------------------------------------------- */

/* ================================================================= *
 *  QoS
 * ================================================================= */

vtss_rc vtss_srvl_policer_conf_set(vtss_state_t *vtss_state,
                                   u32 policer, vtss_srvl_policer_conf_t *conf)
{
    u32  cir = 0, cbs = 0, pir, pbs, mode;
    u32  cir_ena = 0, cf = 0, pbs_max, cbs_max = 0;
    BOOL pir_discard = 0, cir_discard = 0;

    pir = conf->eir;
    pbs = conf->ebs;
    if (conf->frame_rate) {
        /* Frame rate policing (single leaky bucket) */
        if (pir >= 100) {
            mode = POL_MODE_FRMRATE_100FPS;
            pir = SRVL_DIV_ROUND_UP(pir, 100); /* Resolution is in steps of 100 fps */
            pbs = (pbs * 10 / 328);            /* Burst unit is 32.8 frames */
            pbs_max = VTSS_BITMASK(6);         /* Limit burst to the maximum value */
        } else {
            mode = POL_MODE_FRMRATE_1FPS;
            pbs = (pbs * 10 / 3); /* Burst unit is 0.3 frames */
            pbs_max = 60;         /* See Bugzilla#4944, comment#2 */
            if (pir == 0 && pbs == 0) {
                /* Discard all frames */
                pir_discard = 1;
                cir_discard = 1;
            }
        }
        pbs++; /* Round up burst size */
    } else {
        /* Bit rate policing */
        mode = (conf->data_rate ? POL_MODE_DATARATE : POL_MODE_LINERATE);
        if (conf->dual) {
            /* Dual leaky bucket mode */
            cir = conf->cir;
            cbs = conf->cbs;
            if (cir == 0 && cbs == 0) {
                /* Discard CIR frames */
                cir_discard = 1;
            }
            cir = SRVL_DIV_ROUND_UP(cir, 100);  /* Rate unit is 100 kbps, round up */
            cbs = (cbs ? cbs : 1);              /* BZ 9813: Avoid using zero burst size */
            cbs = SRVL_DIV_ROUND_UP(cbs, 4096); /* Burst unit is 4kB, round up */
            cbs_max = 61;                       /* See Bugzilla#4944, comment#2  */
            cir_ena = 1;
            cf = conf->cf;
            if (cf)
                pir += conf->cir;
        }
        if (pir == 0 && pbs == 0) {
            /* Discard PIR frames */
            pir_discard = 1;
        }
        pir = SRVL_DIV_ROUND_UP(pir, 100);  /* Rate unit is 100 kbps, round up */
        pbs = (pbs ? pbs : 1);              /* BZ 9813: Avoid using zero burst size */
        pbs = SRVL_DIV_ROUND_UP(pbs, 4096); /* Burst unit is 4kB, round up */
        pbs_max = 61;                       /* See Bugzilla#4944, comment#2  */
    }

    /* Policer rate fix for Revision B or later. See Bugzilla#8648 */
    if (vtss_state->misc.chip_id.revision > 0) {
        pir *= 3;
        cir *= 3;
    }

    /* Limit to maximum values */
    pir = MIN(VTSS_BITMASK(15), pir);
    cir = MIN(VTSS_BITMASK(15), cir);
    pbs = MIN(pbs_max, pbs);
    cbs = MIN(cbs_max, cbs);

    SRVL_WR(VTSS_ANA_POL_POL_MODE_CFG(policer),
            VTSS_F_ANA_POL_POL_MODE_CFG_IPG_SIZE(20) |
            VTSS_F_ANA_POL_POL_MODE_CFG_FRM_MODE(mode) |
            (cf ? VTSS_F_ANA_POL_POL_MODE_CFG_DLB_COUPLED : 0) |
            (cir_ena ? VTSS_F_ANA_POL_POL_MODE_CFG_CIR_ENA : 0) |
            VTSS_F_ANA_POL_POL_MODE_CFG_OVERSHOOT_ENA);
    SRVL_WR(VTSS_ANA_POL_POL_PIR_CFG(policer), 
            VTSS_F_ANA_POL_POL_PIR_CFG_PIR_RATE(pir) |
            VTSS_F_ANA_POL_POL_PIR_CFG_PIR_BURST(pbs));
    SRVL_WR(VTSS_ANA_POL_POL_CIR_CFG(policer), 
            VTSS_F_ANA_POL_POL_CIR_CFG_CIR_RATE(cir) |
            VTSS_F_ANA_POL_POL_CIR_CFG_CIR_BURST(cbs));
    SRVL_WR(VTSS_ANA_POL_POL_PIR_STATE(policer), pir_discard ? VTSS_M_ANA_POL_POL_PIR_STATE_PIR_LVL : 0);
    SRVL_WR(VTSS_ANA_POL_POL_CIR_STATE(policer), cir_discard ? VTSS_M_ANA_POL_POL_CIR_STATE_CIR_LVL : 0);

    return VTSS_RC_OK;
}

#define SRVL_DEFAULT_POL_ORDER 0x1d3 /* Serval policer order: Serial (QoS -> Port -> VCAP) */

static vtss_rc srvl_port_policer_set(vtss_state_t *vtss_state,
                                     u32 port, BOOL enable, vtss_srvl_policer_conf_t *conf)
{
    u32  order      = SRVL_DEFAULT_POL_ORDER;

    VTSS_RC(vtss_srvl_policer_conf_set(vtss_state, SRVL_POLICER_PORT + port, conf));

    SRVL_WRM(VTSS_ANA_PORT_POL_CFG(port),
             (enable ? VTSS_F_ANA_PORT_POL_CFG_PORT_POL_ENA : 0) |
             VTSS_F_ANA_PORT_POL_CFG_POL_ORDER(order),
             VTSS_F_ANA_PORT_POL_CFG_PORT_POL_ENA |
             VTSS_M_ANA_PORT_POL_CFG_POL_ORDER);

    return VTSS_RC_OK;
}

static vtss_rc srvl_queue_policer_set(vtss_state_t *vtss_state,
                                      u32 port, u32 queue, BOOL enable, vtss_srvl_policer_conf_t *conf)
{
    VTSS_RC(vtss_srvl_policer_conf_set(vtss_state, SRVL_POLICER_QUEUE + port * 8 + queue, conf));

    SRVL_WRM(VTSS_ANA_PORT_POL_CFG(port),
             (enable       ? VTSS_F_ANA_PORT_POL_CFG_QUEUE_POL_ENA(VTSS_BIT(queue)): 0),
             VTSS_F_ANA_PORT_POL_CFG_QUEUE_POL_ENA(VTSS_BIT(queue)));

    return VTSS_RC_OK;
}

static u32 srvl_packet_rate(vtss_packet_rate_t rate, u32 *unit)
{
    u32 value, max;

    /* If the rate is greater than 1000 pps, the unit is kpps */
    max = (rate >= 1000 ? (rate/1000) : rate);
    *unit = (rate >= 1000 ? 0 : 1);
    for (value = 15; value != 0; value--) {
        if (max >= (u32)(1<<value))
            break;
    }
    return value;
}

static u32 srvl_chip_prio(vtss_state_t *vtss_state, const vtss_prio_t prio)
{
    if (prio >= SRVL_PRIOS) {
        VTSS_E("illegal prio: %u", prio);
    }

    switch (vtss_state->qos.conf.prios) {
    case 1:
        return 0;
    case 2:
        return (prio < 4 ? 0 : 1);
    case 4:
        return (prio < 2 ? 0 : prio < 4 ? 1 : prio < 6 ? 2 : 3);
    case 8:
        return prio;
    default:
        break;
    }
    VTSS_E("illegal prios: %u", vtss_state->qos.conf.prios);
    return 0;
}

static vtss_rc srvl_qos_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_qos_port_conf_t     *conf = &vtss_state->qos.port_conf[port_no];
    u32                      port = VTSS_CHIP_PORT(port_no);
    int                      pcp, dei, queue, class, dpl;
    u8                       cost[6];
    vtss_srvl_policer_conf_t pol_cfg;
    u32                      tag_remark_mode, value;
    BOOL                     tag_default_dei;

    /* Port default PCP and DEI configuration */
    SRVL_WRM(VTSS_ANA_PORT_VLAN_CFG(port),
             (conf->default_dei ? VTSS_F_ANA_PORT_VLAN_CFG_VLAN_DEI : 0) |
             VTSS_F_ANA_PORT_VLAN_CFG_VLAN_PCP(conf->usr_prio),
             VTSS_F_ANA_PORT_VLAN_CFG_VLAN_DEI                           |
             VTSS_M_ANA_PORT_VLAN_CFG_VLAN_PCP);

    /* Port default QoS class, DP level, tagged frames mode, DSCP mode and DSCP remarking configuration */
    SRVL_WR(VTSS_ANA_PORT_QOS_CFG(port),
            (conf->default_dpl ? VTSS_F_ANA_PORT_QOS_CFG_DP_DEFAULT_VAL : 0)            |
            VTSS_F_ANA_PORT_QOS_CFG_QOS_DEFAULT_VAL(srvl_chip_prio(vtss_state, conf->default_prio)) |
            (conf->tag_class_enable ? VTSS_F_ANA_PORT_QOS_CFG_QOS_PCP_ENA : 0)          |
            (conf->dscp_class_enable ? VTSS_F_ANA_PORT_QOS_CFG_QOS_DSCP_ENA : 0)        |
            (conf->dscp_translate ? VTSS_F_ANA_PORT_QOS_CFG_DSCP_TRANSLATE_ENA : 0)     |
            VTSS_F_ANA_PORT_QOS_CFG_DSCP_REWR_CFG(conf->dscp_mode));

    /* Egress DSCP remarking configuration */
    SRVL_WR(VTSS_REW_PORT_DSCP_CFG(port), VTSS_F_REW_PORT_DSCP_CFG_DSCP_REWR_CFG(conf->dscp_emode));

    /* Map for (PCP and DEI) to (QoS class and DP level */
    for (pcp = VTSS_PCP_START; pcp < VTSS_PCP_END; pcp++) {
        for (dei = VTSS_DEI_START; dei < VTSS_DEI_END; dei++) {
            SRVL_WR(VTSS_ANA_PORT_QOS_PCP_DEI_MAP_CFG(port, (8*dei + pcp)),
                    (conf->dp_level_map[pcp][dei] ? VTSS_F_ANA_PORT_QOS_PCP_DEI_MAP_CFG_DP_PCP_DEI_VAL : 0) |
                    VTSS_F_ANA_PORT_QOS_PCP_DEI_MAP_CFG_QOS_PCP_DEI_VAL(srvl_chip_prio(vtss_state, conf->qos_class_map[pcp][dei])));
        }
    }

    /*
     * Verify that the default scheduler configuration is used.
     * If this is not the case then this code must be modified
     * to use actual scheduler configuration.
     * See VSC7418 Target Specification, section 3.11.2, Queue Mapping, Figure 42 for details.
     *
     * P is port number (0..11)
     * Q is QoS class (0..7)
     * Level 1: SE# = 218 + P
     * Level 2: SE# = Q + (P * 8)
     */

    SRVL_RD(VTSS_QSYS_QMAP_QMAP(port), &value);
    if (value) {
        VTSS_E("Unsupported scheduler configuration (0x%x)", value);
    } else {
        u32 level_1_se = 218 + port;
        u32 cir, cbs, eir, ebs;

        /* DWRR configuration */
        SRVL_WRM(VTSS_QSYS_HSCH_SE_CFG(level_1_se),
                 VTSS_F_QSYS_HSCH_SE_CFG_SE_DWRR_CNT((conf->dwrr_enable ? 6 : 0)), /* 6 DWRR inputs if enabled, otherwise 0 inputs (all strict) */
                 VTSS_M_QSYS_HSCH_SE_CFG_SE_DWRR_CNT);

        VTSS_RC(vtss_cmn_qos_weight2cost(conf->queue_pct, cost, 6, 5));
        for (queue = 0; queue < 6; queue++) {
            SRVL_WR(VTSS_QSYS_HSCH_SE_DWRR_CFG(level_1_se, queue), cost[queue]);
        }

        /* Egress port/queue shaper rate configuration
         * The value (in kbps) is rounded up to the next possible value:
         *        0 -> 0 (Open until burst capacity is used, then closed)
         *   1..100 -> 1 (100 kbps)
         * 101..200 -> 2 (200 kbps)
         * 201..300 -> 3 (300 kbps)
         */

        /* Egress port/queue shaper burst level configuration
         * The value is rounded up to the next possible value:
         *           0 -> 0 (Shaper disabled)
         *    1.. 4096 -> 1 ( 4 KB)
         * 4097.. 8192 -> 2 ( 8 KB)
         * 8193..12288 -> 3 (12 KB)
         */

        /* Egress port shaper configuration */
        if (conf->shaper_port.rate != VTSS_BITRATE_DISABLED) {
            cir = MIN(VTSS_BITMASK(15), SRVL_DIV_ROUND_UP(conf->shaper_port.rate,   100));
            cbs = MIN(VTSS_BITMASK(6),  SRVL_DIV_ROUND_UP(conf->shaper_port.level, 4096));

            SRVL_WR(VTSS_QSYS_HSCH_CIR_CFG(level_1_se),
                    VTSS_F_QSYS_HSCH_CIR_CFG_CIR_RATE(cir) |
                    VTSS_F_QSYS_HSCH_CIR_CFG_CIR_BURST(cbs));

            /* DLB configuration */
            if (conf->shaper_port.eir != VTSS_BITRATE_DISABLED) {
                eir = MIN(VTSS_BITMASK(15), SRVL_DIV_ROUND_UP(conf->shaper_port.eir,  100));
                ebs = MIN(VTSS_BITMASK(6),  SRVL_DIV_ROUND_UP(conf->shaper_port.ebs, 4096));

                SRVL_WR(VTSS_QSYS_HSCH_EIR_CFG(level_1_se),
                        VTSS_F_QSYS_HSCH_EIR_CFG_EIR_RATE(eir) |
                        VTSS_F_QSYS_HSCH_EIR_CFG_EIR_BURST(ebs));

                SRVL_WR(VTSS_QSYS_HSCH_SE_DLB_SENSE(level_1_se),
                        VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO(0)     | /* Sense on QoS class 0 */
                        VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT(port) | /* and egress port */
                        VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO_ENA    |
                        VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT_ENA);
            } else {
                SRVL_WR(VTSS_QSYS_HSCH_EIR_CFG(level_1_se), 0);      /* Disable EIR */
                SRVL_WR(VTSS_QSYS_HSCH_SE_DLB_SENSE(level_1_se), 0); /* Disable DLB */
            }
        } else {
            SRVL_WR(VTSS_QSYS_HSCH_CIR_CFG(level_1_se), 0);      /* Disable shaper */
            SRVL_WR(VTSS_QSYS_HSCH_EIR_CFG(level_1_se), 0);      /* Disable EIR */
            SRVL_WR(VTSS_QSYS_HSCH_SE_DLB_SENSE(level_1_se), 0); /* Disable DLB */
        }

        /* Egress queue shaper configuration */
        for (queue = 0; queue < 8; queue++) {
            u32 level_2_se = queue + (port * 8);
            if (conf->shaper_queue[queue].rate != VTSS_BITRATE_DISABLED) {
                cir = MIN(VTSS_BITMASK(15), SRVL_DIV_ROUND_UP(conf->shaper_queue[queue].rate,   100));
                cbs = MIN(VTSS_BITMASK(6),  SRVL_DIV_ROUND_UP(conf->shaper_queue[queue].level, 4096));

                SRVL_WR(VTSS_QSYS_HSCH_CIR_CFG(level_2_se),
                        VTSS_F_QSYS_HSCH_CIR_CFG_CIR_RATE(cir) |
                        VTSS_F_QSYS_HSCH_CIR_CFG_CIR_BURST(cbs));

                /* DLB configuration */
                if (conf->shaper_queue[queue].eir != VTSS_BITRATE_DISABLED) {
                    eir = MIN(VTSS_BITMASK(15), SRVL_DIV_ROUND_UP(conf->shaper_queue[queue].eir,  100));
                    ebs = MIN(VTSS_BITMASK(6),  SRVL_DIV_ROUND_UP(conf->shaper_queue[queue].ebs, 4096));

                    SRVL_WR(VTSS_QSYS_HSCH_EIR_CFG(level_2_se),
                            VTSS_F_QSYS_HSCH_EIR_CFG_EIR_RATE(eir) |
                            VTSS_F_QSYS_HSCH_EIR_CFG_EIR_BURST(ebs));

                    SRVL_WR(VTSS_QSYS_HSCH_SE_DLB_SENSE(level_2_se),
                            VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO(queue) | /* Sense on actual QoS class */
                            VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT(port) | /* and egress port */
                            VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO_ENA    |
                            VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT_ENA);
                } else {
                    SRVL_WR(VTSS_QSYS_HSCH_EIR_CFG(level_2_se), 0);      /* Disable EIR */
                    SRVL_WR(VTSS_QSYS_HSCH_SE_DLB_SENSE(level_2_se), 0); /* Disable DLB */
                }
            } else {
                SRVL_WR(VTSS_QSYS_HSCH_CIR_CFG(level_2_se), 0);      /* Disable shaper */
                SRVL_WR(VTSS_QSYS_HSCH_EIR_CFG(level_2_se), 0);      /* Disable EIR */
                SRVL_WR(VTSS_QSYS_HSCH_SE_DLB_SENSE(level_2_se), 0); /* Disable DLB */
            }

            /* Excess configuration */
            SRVL_WRM(VTSS_QSYS_HSCH_SE_CFG(level_2_se),
                     conf->excess_enable[queue] ? VTSS_F_QSYS_HSCH_SE_CFG_SE_EXC_ENA : 0,
                     VTSS_F_QSYS_HSCH_SE_CFG_SE_EXC_ENA);
        }
    }

    tag_remark_mode = conf->tag_remark_mode;
    tag_default_dei = (tag_remark_mode == VTSS_TAG_REMARK_MODE_DEFAULT ? conf->tag_default_dei : 0);

    /* Tag remarking configuration */
    SRVL_WRM(VTSS_REW_PORT_PORT_VLAN_CFG(port),
             (tag_default_dei ? VTSS_F_REW_PORT_PORT_VLAN_CFG_PORT_DEI : 0) |
             VTSS_F_REW_PORT_PORT_VLAN_CFG_PORT_PCP(conf->tag_default_pcp),
             VTSS_F_REW_PORT_PORT_VLAN_CFG_PORT_DEI                               |
             VTSS_M_REW_PORT_PORT_VLAN_CFG_PORT_PCP);

    /* Map remark mode */
    switch (tag_remark_mode) {
    case VTSS_TAG_REMARK_MODE_DEFAULT:
        tag_remark_mode = 1; /* PORT_PCP/PORT_DEI */
        break;
    case VTSS_TAG_REMARK_MODE_MAPPED:
        tag_remark_mode = 2; /* MAPPED */
        break;
    default:
        tag_remark_mode = 0; /* Classified PCP/DEI */
        break;
    }

    SRVL_WRM(VTSS_REW_PORT_TAG_CFG(port),
             VTSS_F_REW_PORT_TAG_CFG_TAG_PCP_CFG(tag_remark_mode) |
             VTSS_F_REW_PORT_TAG_CFG_TAG_DEI_CFG(tag_remark_mode),
             VTSS_M_REW_PORT_TAG_CFG_TAG_PCP_CFG |
             VTSS_M_REW_PORT_TAG_CFG_TAG_DEI_CFG);

    /* Map for (QoS class and DP level) to (PCP and DEI) */
    for (class = VTSS_QUEUE_START; class < VTSS_QUEUE_END; class++) {
        for (dpl = 0; dpl < 2; dpl++) {
            SRVL_WRM(VTSS_REW_PORT_PCP_DEI_QOS_MAP_CFG(port, (8 * dpl + class)),
                     (conf->tag_dei_map[class][dpl] ? VTSS_F_REW_PORT_PCP_DEI_QOS_MAP_CFG_DEI_QOS_VAL : 0) |
                     VTSS_F_REW_PORT_PCP_DEI_QOS_MAP_CFG_PCP_QOS_VAL(conf->tag_pcp_map[class][dpl]),
                     VTSS_F_REW_PORT_PCP_DEI_QOS_MAP_CFG_DEI_QOS_VAL                                       |
                     VTSS_M_REW_PORT_PCP_DEI_QOS_MAP_CFG_PCP_QOS_VAL);
        }
    }

    /* Port policer configuration */
    memset(&pol_cfg, 0, sizeof(pol_cfg));
    if (conf->policer_port[0].rate != VTSS_BITRATE_DISABLED) {
        pol_cfg.frame_rate = conf->policer_ext_port[0].frame_rate;
        pol_cfg.eir = conf->policer_port[0].rate;
        pol_cfg.ebs = pol_cfg.frame_rate ? 64 : conf->policer_port[0].level; /* If frame_rate we always use 64 frames as burst value */
    }
    VTSS_RC(srvl_port_policer_set(vtss_state, port, conf->policer_port[0].rate != VTSS_BITRATE_DISABLED, &pol_cfg));

    /* Queue policer configuration */
    for (queue = 0; queue < 8; queue++) {
        memset(&pol_cfg, 0, sizeof(pol_cfg));
        if (conf->policer_queue[queue].rate != VTSS_BITRATE_DISABLED) {
            pol_cfg.eir = conf->policer_queue[queue].rate;
            pol_cfg.ebs = conf->policer_queue[queue].level;
        }
        VTSS_RC(srvl_queue_policer_set(vtss_state, port, queue, conf->policer_queue[queue].rate != VTSS_BITRATE_DISABLED, &pol_cfg));
    }

    /* Update policer flow control configuration */
    VTSS_RC(vtss_srvl_port_policer_fc_set(vtss_state, port_no));

    /* Update QCL port configuration */
    return vtss_srvl_vcap_port_key_addr_set(vtss_state,
                                            port_no,
                                            2, /* Third IS1 lookup */
                                            vtss_state->qos.port_conf[port_no].key_type,
                                            vtss_state->qos.port_conf_old.key_type,
                                            vtss_state->qos.port_conf[port_no].dmac_dip);
}

static vtss_rc srvl_qos_wred_conf_set(vtss_state_t *vtss_state)
{
    vtss_qos_conf_t *conf = &vtss_state->qos.conf;
    int              queue, dpl;

    for (queue = VTSS_QUEUE_START; queue < VTSS_QUEUE_END; queue++) {
        u32 wm_high, wm_red_low, wm_red_high;
        SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((queue + 216)), &wm_high); /* Shared ingress high watermark for queue - common for dpl 0 and 1 */
        wm_high = vtss_srvl_wm_dec(wm_high) * SRVL_BUFFER_CELL_SZ; /* Convert from 60 byte chunks to bytes */
        for (dpl = 0; dpl < 2; dpl++) {
            vtss_red_v2_t *red = &conf->red_v2[queue][dpl];
            vtss_pct_t     max_dp = 100;
            vtss_pct_t     max_fl = 100;

            /* Sanity check */
            if (red->min_fl > 100) {
                VTSS_E("illegal min_fl (%u) on queue %d, dpl %d", red->min_fl, queue, dpl);
                return VTSS_RC_ERROR;
            }
            if ((red->max < 1) || (red->max > 100)) {
                VTSS_E("illegal max (%u) on queue %d, dpl %d", red->max, queue, dpl);
                return VTSS_RC_ERROR;
            }
            if ((red->max_unit != VTSS_WRED_V2_MAX_DP) && (red->max_unit != VTSS_WRED_V2_MAX_FL)) {
                VTSS_E("illegal max_unit (%u) on queue %d, dpl %d", red->max_unit, queue, dpl);
                return VTSS_RC_ERROR;
            }
            if (red->max_unit == VTSS_WRED_V2_MAX_DP) {
                max_dp = red->max; /* Unit is drop probability - save specified value */
            } else {
                if (red->min_fl >= red->max) {
                    VTSS_E("min_fl (%u) >= max fl (%u) on queue %d, dpl %d", red->min_fl, red->max, queue, dpl);
                    return VTSS_RC_ERROR;
                } else {
                    max_fl = red->max; /* Unit is fill level - save specified value */
                }
            }

            if (red->enable) {
                wm_red_low  = wm_high * red->min_fl / 100;                              /* Convert from % to actual value in bytes */
                wm_red_high = wm_high * max_fl / 100;                                   /* Convert from % to actual value in bytes */
                wm_red_high = ((wm_red_high - wm_red_low) * 100 / max_dp) + wm_red_low; /* Adjust wm_red_high to represent 100% drop probability */
                wm_red_low  = MIN(wm_red_low / 960, VTSS_BITMASK(11));                  /* Convert from bytes to 960 byte chunks and prevent overflow */
                wm_red_high = MIN(wm_red_high / 960, VTSS_BITMASK(11));                 /* Convert from bytes to 960 byte chunks and prevent overflow */
            } else {
                wm_red_low = wm_red_high = VTSS_BITMASK(11);                            /* Disable red by setting both fields to max */
            }

            SRVL_WR(VTSS_QSYS_RES_QOS_ADV_RED_PROFILE((queue + (8 * dpl))), /* Red profile for queue, dpl */
                    VTSS_F_QSYS_RES_QOS_ADV_RED_PROFILE_WM_RED_LOW(wm_red_low) |
                    VTSS_F_QSYS_RES_QOS_ADV_RED_PROFILE_WM_RED_HIGH(wm_red_high));
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_qos_conf_set(vtss_state_t *vtss_state, BOOL changed)
{
    vtss_qos_conf_t    *conf = &vtss_state->qos.conf;
    vtss_port_no_t     port_no;
    u32                i, unit;
    vtss_packet_rate_t rate;

    if (changed) {
        /* Number of priorities changed, update QoS setup for all ports */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            VTSS_RC(srvl_qos_port_conf_set(vtss_state, port_no));
        }
    }
    /* Storm control */
    SRVL_WR(VTSS_ANA_ANA_STORMLIMIT_BURST, VTSS_F_ANA_ANA_STORMLIMIT_BURST_STORM_BURST(6)); /* Burst of 64 frames allowed */
    for (i = 0; i < 3; i++) {
        rate = (i == 0 ? conf->policer_uc : i == 1 ? conf->policer_bc : conf->policer_mc);
        SRVL_WR(VTSS_ANA_ANA_STORMLIMIT_CFG(i),
                VTSS_F_ANA_ANA_STORMLIMIT_CFG_STORM_RATE(srvl_packet_rate(rate, &unit)) |
                (unit ? VTSS_F_ANA_ANA_STORMLIMIT_CFG_STORM_UNIT : 0) |
                VTSS_F_ANA_ANA_STORMLIMIT_CFG_STORM_MODE(rate == VTSS_PACKET_RATE_DISABLED ? 0 : 3)); /* Disabled or police both CPU and front port destinations */
    }

    /* DSCP classification and remarking configuration
     */
    for (i = 0; i < 64; i++) {
        SRVL_WR(VTSS_ANA_COMMON_DSCP_CFG(i),
                (conf->dscp_dp_level_map[i] ? VTSS_F_ANA_COMMON_DSCP_CFG_DP_DSCP_VAL : 0)           |
                VTSS_F_ANA_COMMON_DSCP_CFG_QOS_DSCP_VAL(srvl_chip_prio(vtss_state, conf->dscp_qos_class_map[i])) |
                VTSS_F_ANA_COMMON_DSCP_CFG_DSCP_TRANSLATE_VAL(conf->dscp_translate_map[i])          |
                (conf->dscp_trust[i] ? VTSS_F_ANA_COMMON_DSCP_CFG_DSCP_TRUST_ENA : 0)               |
                (conf->dscp_remark[i] ? VTSS_F_ANA_COMMON_DSCP_CFG_DSCP_REWR_ENA : 0));

        SRVL_WR(VTSS_REW_COMMON_DSCP_REMAP_CFG(i),
                VTSS_F_REW_COMMON_DSCP_REMAP_CFG_DSCP_REMAP_VAL(conf->dscp_remap[i]));

        SRVL_WR(VTSS_REW_COMMON_DSCP_REMAP_DP1_CFG(i),
                VTSS_F_REW_COMMON_DSCP_REMAP_DP1_CFG_DSCP_REMAP_DP1_VAL(conf->dscp_remap_dp1[i]));
    }

    /* DSCP classification from QoS configuration
     */
    for (i = 0; i < 8; i++) {
        SRVL_WR(VTSS_ANA_COMMON_DSCP_REWR_CFG(i),
                VTSS_F_ANA_COMMON_DSCP_REWR_CFG_DSCP_QOS_REWR_VAL(conf->dscp_qos_map[i]));

        SRVL_WR(VTSS_ANA_COMMON_DSCP_REWR_CFG(i + 8),
                VTSS_F_ANA_COMMON_DSCP_REWR_CFG_DSCP_QOS_REWR_VAL(conf->dscp_qos_map_dp1[i]));
    }

    /* WRED configuration
     */
    VTSS_RC(srvl_qos_wred_conf_set(vtss_state));

    return VTSS_RC_OK;
}

static vtss_rc srvl_evc_policer_conf_set(vtss_state_t *vtss_state,
                                         const vtss_evc_policer_id_t policer_id)
{
    vtss_evc_policer_conf_t  *conf = &vtss_state->qos.evc_policer_conf[policer_id];
    vtss_srvl_policer_conf_t pol_conf;

    /* Convert to Serval policer configuration */
    memset(&pol_conf, 0, sizeof(pol_conf));
    pol_conf.dual = 1;
    pol_conf.data_rate = (conf->line_rate ? 0 : 1);
    if (conf->enable) {
        /* Use configured values if policer enabled */
        switch (conf->type) {
        case VTSS_POLICER_TYPE_MEF:
            pol_conf.cir = conf->cir;
            pol_conf.cbs = conf->cbs;
            pol_conf.eir = conf->eir;
            pol_conf.ebs = conf->ebs;
            pol_conf.cf = conf->cf;
            break;
        case VTSS_POLICER_TYPE_SINGLE:
        default:
            pol_conf.dual = 0;
            pol_conf.eir = conf->cir;
            pol_conf.ebs = conf->cbs;
            break;
        }
    } else {
        /* Use maximum rates if policer disabled */
        pol_conf.cir = 100000000; /* 100 Gbps, will be rounded down */
        pol_conf.eir = 100000000; /* 100 Gbps, will be rounded down */
    }

    return vtss_srvl_policer_conf_set(vtss_state, SRVL_POLICER_EVC + policer_id, &pol_conf);
}

/* - Debug print --------------------------------------------------- */

static vtss_rc srvl_debug_qos(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
    u32            i, port, pir, cir, value, mode, level_1_se;
    int            queue;
    BOOL           header = 1;
    vtss_port_no_t port_no;

    /* Global configuration starts here */

    vtss_debug_print_header(pr, "QoS Storm Control");

    SRVL_RD(VTSS_ANA_ANA_STORMLIMIT_BURST, &value);
    pr("Burst: %u\n", VTSS_X_ANA_ANA_STORMLIMIT_BURST_STORM_BURST(value));
    for (i = 0; i < 3; i++) {
        const char *name = (i == 0 ? "UC" : i == 1 ? "BC" : "MC");
        SRVL_RD(VTSS_ANA_ANA_STORMLIMIT_CFG(i), &value);
        pr("%s   : Rate %2u, Unit %u, Mode %u\n", name,
           VTSS_X_ANA_ANA_STORMLIMIT_CFG_STORM_RATE(value),
           VTSS_BOOL(value & VTSS_F_ANA_ANA_STORMLIMIT_CFG_STORM_UNIT),
           VTSS_X_ANA_ANA_STORMLIMIT_CFG_STORM_MODE(value));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS WRED Config");

    pr("Queue Dpl WM_HIGH  bytes RED_LOW  bytes RED_HIGH bytes\n");
//      xxxxx xxx 0xxxxx xxxxxxx 0xxxxx xxxxxxx 0xxxxx xxxxxxx
    for (queue = VTSS_QUEUE_START; queue < VTSS_QUEUE_END; queue++) {
        int dpl;
        u32 wm_high, red_profile, wm_red_low, wm_red_high;
        SRVL_RD(VTSS_QSYS_RES_CTRL_RES_CFG((queue + 216)), &wm_high); /* Shared ingress high watermark for queue */
        for (dpl = 0; dpl < 2; dpl++) {
            SRVL_RD(VTSS_QSYS_RES_QOS_ADV_RED_PROFILE((queue + (8 * dpl))), &red_profile); /* Red profile for queue, dpl */
            wm_red_low  = VTSS_X_QSYS_RES_QOS_ADV_RED_PROFILE_WM_RED_LOW(red_profile);
            wm_red_high = VTSS_X_QSYS_RES_QOS_ADV_RED_PROFILE_WM_RED_HIGH(red_profile);
            pr("%5u %3u 0x%04x %7u 0x%04x %7u 0x%04x %7u\n",
               queue,
               dpl,
               wm_high,
               vtss_srvl_wm_dec(wm_high) * SRVL_BUFFER_CELL_SZ,
               wm_red_low,
               wm_red_low * 960,
               wm_red_high,
               wm_red_high * 960);
        }
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS DSCP Config");

    pr("DSCP Trans CLS DPL Rewr Trust Remap_DP0 Remap_DP1\n");
    for (i = 0; i < 64; i++) {
        u32 dscp_cfg, dscp_remap, dscp_remap_dp1;
        SRVL_RD(VTSS_ANA_COMMON_DSCP_CFG(i), &dscp_cfg);
        SRVL_RD(VTSS_REW_COMMON_DSCP_REMAP_CFG(i), &dscp_remap);
        SRVL_RD(VTSS_REW_COMMON_DSCP_REMAP_DP1_CFG(i), &dscp_remap_dp1);

        pr("%4u %5u %3u %3u %4u %5u %5u     %5u\n",
           i,
           VTSS_X_ANA_COMMON_DSCP_CFG_DSCP_TRANSLATE_VAL(dscp_cfg),
           VTSS_X_ANA_COMMON_DSCP_CFG_QOS_DSCP_VAL(dscp_cfg),
           VTSS_BOOL(dscp_cfg & VTSS_F_ANA_COMMON_DSCP_CFG_DP_DSCP_VAL),
           VTSS_BOOL(dscp_cfg & VTSS_F_ANA_COMMON_DSCP_CFG_DSCP_REWR_ENA),
           VTSS_BOOL(dscp_cfg & VTSS_F_ANA_COMMON_DSCP_CFG_DSCP_TRUST_ENA),
           VTSS_X_REW_COMMON_DSCP_REMAP_CFG_DSCP_REMAP_VAL(dscp_remap),
           VTSS_X_REW_COMMON_DSCP_REMAP_DP1_CFG_DSCP_REMAP_DP1_VAL(dscp_remap_dp1));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS DSCP Classification from QoS Config");

    pr("QoS DSCP_DP0 DSCP_DP1\n");
    for (i = 0; i < 8; i++) {
        u32 qos_dp0, qos_dp1;
        SRVL_RD(VTSS_ANA_COMMON_DSCP_REWR_CFG(i), &qos_dp0);
        SRVL_RD(VTSS_ANA_COMMON_DSCP_REWR_CFG(i + 8), &qos_dp1);
        pr("%3u %4u     %4u\n",
           i,
           VTSS_X_ANA_COMMON_DSCP_REWR_CFG_DSCP_QOS_REWR_VAL(qos_dp0),
           VTSS_X_ANA_COMMON_DSCP_REWR_CFG_DSCP_QOS_REWR_VAL(qos_dp1));
    }
    pr("\n");

    /* Per port configuration starts here */

    vtss_debug_print_header(pr, "QoS Port Classification Config");

    pr("LP CP PCP CLS DEI DPL TC_EN DC_EN\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 vlan, qos;
        if (!info->port_list[port_no]) {
            continue;
        }
        port = VTSS_CHIP_PORT(port_no);
        SRVL_RD(VTSS_ANA_PORT_VLAN_CFG(port), &vlan);
        SRVL_RD(VTSS_ANA_PORT_QOS_CFG(port), &qos);
        pr("%2u %2u %3u %3u %3u %3u %5u %5u\n",
           port_no, // Logical port
           port,    // Chip port
           VTSS_X_ANA_PORT_VLAN_CFG_VLAN_PCP(vlan),
           VTSS_X_ANA_PORT_QOS_CFG_QOS_DEFAULT_VAL(qos),
           VTSS_BOOL(vlan & VTSS_F_ANA_PORT_VLAN_CFG_VLAN_DEI),
           VTSS_BOOL(qos & VTSS_F_ANA_PORT_QOS_CFG_DP_DEFAULT_VAL),
           VTSS_BOOL(qos & VTSS_F_ANA_PORT_QOS_CFG_QOS_PCP_ENA),
           VTSS_BOOL(qos & VTSS_F_ANA_PORT_QOS_CFG_QOS_DSCP_ENA));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port Classification PCP, DEI to QoS class, DP level Mapping");

    pr("LP CP QoS class (8*DEI+PCP)           DP level (8*DEI+PCP)\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        int pcp, dei, class_ct = 0, dpl_ct = 0;
        char class_buf[40], dpl_buf[40];
        if (!info->port_list[port_no]) {
            continue;
        }
        port = VTSS_CHIP_PORT(port_no);
        for (dei = VTSS_DEI_START; dei < VTSS_DEI_END; dei++) {
            for (pcp = VTSS_PCP_START; pcp < VTSS_PCP_END; pcp++) {
                const char *delim = ((pcp == VTSS_PCP_START) && (dei == VTSS_DEI_START)) ? "" : ",";
                SRVL_RD(VTSS_ANA_PORT_QOS_PCP_DEI_MAP_CFG(port, (8*dei + pcp)), &value);
                class_ct += snprintf(class_buf + class_ct, sizeof(class_buf) - class_ct, "%s%u", delim,
                                     VTSS_X_ANA_PORT_QOS_PCP_DEI_MAP_CFG_QOS_PCP_DEI_VAL(value));
                dpl_ct   += snprintf(dpl_buf   + dpl_ct,   sizeof(dpl_buf)   - dpl_ct,   "%s%u",  delim,
                                     VTSS_BOOL(value & VTSS_F_ANA_PORT_QOS_PCP_DEI_MAP_CFG_DP_PCP_DEI_VAL));
            }
        }
        pr("%2u %2u %s %s\n", port_no, port, class_buf, dpl_buf);
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Scheduler Config");

    pr("LP CP Base IdxSel InpSel DWRR C0 C1 C2 C3 C4 C5\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 qmap;
        if (!info->port_list[port_no]) {
            continue;
        }
        port = VTSS_CHIP_PORT(port_no);
        level_1_se = 218 + port;
        SRVL_RD(VTSS_QSYS_QMAP_QMAP(port), &qmap);
        SRVL_RD(VTSS_QSYS_HSCH_SE_CFG(level_1_se), &value);
        pr("%2u %2u %4u %6u %6u %4u",
           port_no, // Logical port
           port,    // Chip port
           VTSS_X_QSYS_QMAP_QMAP_SE_BASE(qmap),
           VTSS_X_QSYS_QMAP_QMAP_SE_IDX_SEL(qmap),
           VTSS_X_QSYS_QMAP_QMAP_SE_INP_SEL(qmap),
           VTSS_X_QSYS_HSCH_SE_CFG_SE_DWRR_CNT(value));
        for (queue = 0; queue < 6; queue++) {
            SRVL_RD(VTSS_QSYS_HSCH_SE_DWRR_CFG(level_1_se, queue), &value);
            pr(" %2u", value);
        }
        pr("\n");
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port and Queue Shaper Config");

    pr("LP CP Queue CBS  CIR    EBS  EIR    SE_PRIO SE_SPORT SE_DPORT Excess\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 eir, sense, excess;
        if (!info->port_list[port_no]) {
            continue;
        }
        port = VTSS_CHIP_PORT(port_no);
        level_1_se = 218 + port;
        SRVL_RD(VTSS_QSYS_HSCH_CIR_CFG(level_1_se), &value);
        SRVL_RD(VTSS_QSYS_HSCH_EIR_CFG(level_1_se), &eir);
        SRVL_RD(VTSS_QSYS_HSCH_SE_DLB_SENSE(level_1_se), &sense);
        pr("%2u %2u     - 0x%02x 0x%04x 0x%02x 0x%04x ",
           port_no,
           port,
           VTSS_X_QSYS_HSCH_CIR_CFG_CIR_BURST(value),
           VTSS_X_QSYS_HSCH_CIR_CFG_CIR_RATE(value),
           VTSS_X_QSYS_HSCH_EIR_CFG_EIR_BURST(eir),
           VTSS_X_QSYS_HSCH_EIR_CFG_EIR_RATE(eir));
        if (sense & VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO_ENA) {
            pr("%7u ", VTSS_X_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO(sense));
        } else {
            pr("      - ");
        }
        if (sense & VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_SPORT_ENA) {
            pr("%8u ", VTSS_X_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_SPORT(sense));
        } else {
            pr("       - ");
        }
        if (sense & VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT_ENA) {
            pr("%8u ", VTSS_X_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT(sense));
        } else {
            pr("       - ");
        }
        pr("     -\n      ");
        for (queue = 0; queue < 8; queue++) {
            u32 level_2_se = queue + (port * 8);
            SRVL_RD(VTSS_QSYS_HSCH_CIR_CFG(level_2_se), &value);
            SRVL_RD(VTSS_QSYS_HSCH_SE_CFG(level_2_se), &excess);
            SRVL_RD(VTSS_QSYS_HSCH_EIR_CFG(level_2_se), &eir);
            SRVL_RD(VTSS_QSYS_HSCH_SE_DLB_SENSE(level_2_se), &sense);
            pr("%5d 0x%02x 0x%04x 0x%02x 0x%04x ",
               queue,
               VTSS_X_QSYS_HSCH_CIR_CFG_CIR_BURST(value),
               VTSS_X_QSYS_HSCH_CIR_CFG_CIR_RATE(value),
               VTSS_X_QSYS_HSCH_EIR_CFG_EIR_BURST(eir),
               VTSS_X_QSYS_HSCH_EIR_CFG_EIR_RATE(eir));
            if (sense & VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO_ENA) {
                pr("%7u ", VTSS_X_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO(sense));
            } else {
                pr("      - ");
            }
            if (sense & VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_SPORT_ENA) {
                pr("%8u ", VTSS_X_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_SPORT(sense));
            } else {
                pr("       - ");
            }
            if (sense & VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT_ENA) {
                pr("%8u ", VTSS_X_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT(sense));
            } else {
                pr("       - ");
            }
            pr("%6d\n      ", VTSS_BOOL(excess & VTSS_F_QSYS_HSCH_SE_CFG_SE_EXC_ENA));
        }
        pr("\r");
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port Tag Remarking Config");

    pr("LP CP MPCP MDEI PCP DEI\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 tag_default, tag_ctrl;
        if (!info->port_list[port_no]) {
            continue;
        }
        port = VTSS_CHIP_PORT(port_no);
        SRVL_RD(VTSS_REW_PORT_PORT_VLAN_CFG(port), &tag_default);
        SRVL_RD(VTSS_REW_PORT_TAG_CFG(port), &tag_ctrl);
        pr("%2u %2u %4x %4x %3d %3d\n",
           port_no,
           port,
           VTSS_X_REW_PORT_TAG_CFG_TAG_PCP_CFG(tag_ctrl),
           VTSS_X_REW_PORT_TAG_CFG_TAG_DEI_CFG(tag_ctrl),
           VTSS_X_REW_PORT_PORT_VLAN_CFG_PORT_PCP(tag_default),
           VTSS_BOOL(tag_default & VTSS_F_REW_PORT_PORT_VLAN_CFG_PORT_DEI));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port Tag Remarking Map");

    pr("LP CP PCP (2*QoS class+DPL)           DEI (2*QoS class+DPL)\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        int class, dpl, pcp_ct = 0, dei_ct = 0;
        char pcp_buf[40], dei_buf[40];
        if (info->port_list[port_no] == 0)
            continue;
        port = VTSS_CHIP_PORT(port_no);
        for (class = VTSS_QUEUE_START; class < VTSS_QUEUE_END; class++) {
            for (dpl = 0; dpl < 2; dpl++) {
                const char *delim = ((class == VTSS_QUEUE_START) && (dpl == 0)) ? "" : ",";
                SRVL_RD(VTSS_REW_PORT_PCP_DEI_QOS_MAP_CFG(port, (8*dpl + class)), &value);
                pcp_ct += snprintf(pcp_buf + pcp_ct, sizeof(pcp_buf) - pcp_ct, "%s%u", delim,
                                   VTSS_X_REW_PORT_PCP_DEI_QOS_MAP_CFG_PCP_QOS_VAL(value));
                dei_ct += snprintf(dei_buf + dei_ct, sizeof(dei_buf) - dei_ct, "%s%u",  delim,
                                   VTSS_BOOL(value & VTSS_F_REW_PORT_PCP_DEI_QOS_MAP_CFG_DEI_QOS_VAL));
            }
        }
        pr("%2u %2u %s %s\n", port_no, port, pcp_buf, dei_buf);
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port DSCP Remarking Config");

    pr("LP CP I_Mode Trans E_Mode\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 qos_cfg, dscp_cfg;
        if (!info->port_list[port_no]) {
            continue;
        }
        port = VTSS_CHIP_PORT(port_no);
        SRVL_RD(VTSS_ANA_PORT_QOS_CFG(port), &qos_cfg);
        SRVL_RD(VTSS_REW_PORT_DSCP_CFG(port), &dscp_cfg);
        pr("%2u %2u %6u %5u %6u\n",
           port_no,
           port,
           VTSS_X_ANA_PORT_QOS_CFG_DSCP_REWR_CFG(qos_cfg),
           VTSS_BOOL(qos_cfg & VTSS_F_ANA_PORT_QOS_CFG_DSCP_TRANSLATE_ENA),
           VTSS_X_REW_PORT_DSCP_CFG_DSCP_REWR_CFG(dscp_cfg));
    }
    pr("\n");

    VTSS_RC(vtss_srvl_debug_range_checkers(vtss_state, pr, info));
    VTSS_RC(vtss_srvl_debug_is1_all(vtss_state, pr, info));

    vtss_debug_print_header(pr, "QoS Port and Queue Policer");

    vtss_srvl_debug_reg_header(pr, "Policer Config (chip ports)");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (!info->port_list[port_no]) {
            continue;
        }
        port = VTSS_CHIP_PORT(port_no);
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_ANA_PORT_POL_CFG(port), port, "POL_CFG");
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_ANA_POL_MISC_POL_FLOWC(port), port, "POL_FLOWC");
    }
    pr("\n");

    vtss_debug_print_header(pr, "Policers");

    header = 1;
    for (i = 0; i < SRVL_POLICER_CNT; i++) {
        SRVL_RD(VTSS_ANA_POL_POL_PIR_CFG(i), &pir);
        if (pir == 0) {
            continue;
        }

        if (header) {
            pr("Index  Mode  Dual  IFG  CF  OS  PIR    PBS  CIR    CBS\n");
            header = 0;
        }

        if (!info->full && i >= SRVL_POLICER_EVC) {
            pr("Use 'full' to see all of the %d policers!", SRVL_POLICER_CNT);
            break;
        }

        SRVL_RD(VTSS_ANA_POL_POL_MODE_CFG(i), &value);
        SRVL_RD(VTSS_ANA_POL_POL_CIR_CFG(i), &cir);
        mode = VTSS_X_ANA_POL_POL_MODE_CFG_FRM_MODE(value);
        pr("%-7u%-6s%-6u%-5u%-4u%-4u%-7u%-5u%-7u%-5u\n",
           i,
           mode == POL_MODE_LINERATE ? "Line" : mode == POL_MODE_DATARATE ? "Data" :
           mode == POL_MODE_FRMRATE_100FPS ? "F100" : "F1",
           SRVL_BF(ANA_POL_POL_MODE_CFG_CIR_ENA, value),
           VTSS_X_ANA_POL_POL_MODE_CFG_IPG_SIZE(value),
           SRVL_BF(ANA_POL_POL_MODE_CFG_DLB_COUPLED, value),
           SRVL_BF(ANA_POL_POL_MODE_CFG_OVERSHOOT_ENA, value),
           VTSS_X_ANA_POL_POL_PIR_CFG_PIR_RATE(pir),
           VTSS_X_ANA_POL_POL_PIR_CFG_PIR_BURST(pir),
           VTSS_X_ANA_POL_POL_CIR_CFG_CIR_RATE(cir),
           VTSS_X_ANA_POL_POL_CIR_CFG_CIR_BURST(cir));
    }
    if (!header) {
        pr("\n");
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_qos_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_QOS, srvl_debug_qos, vtss_state, pr, info);
}

/* - Initialization ------------------------------------------------ */

vtss_rc vtss_srvl_qos_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_qos_state_t         *state = &vtss_state->qos;
    vtss_srvl_policer_conf_t pol_conf;
    vtss_port_no_t           port_no;
    
    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->conf_set = srvl_qos_conf_set;
        state->port_conf_set = vtss_cmn_qos_port_conf_set;
        state->port_conf_update = srvl_qos_port_conf_set;
        state->qce_add = vtss_cmn_qce_add;
        state->qce_del = vtss_cmn_qce_del;
        state->evc_policer_conf_set = srvl_evc_policer_conf_set;
        state->evc_policer_max = 1022;
        state->prio_count = SRVL_PRIOS;
        break;
    case VTSS_INIT_CMD_INIT:
        /* Setup discard policer */
        memset(&pol_conf, 0, sizeof(pol_conf));
        pol_conf.frame_rate = 1;
        VTSS_RC(vtss_srvl_policer_conf_set(vtss_state, SRVL_POLICER_DISCARD, &pol_conf));
        break;
    case VTSS_INIT_CMD_PORT_MAP:
    {
        /* Use quarter key in IS1 third lookup by default */
        vtss_vcap_key_type_t key_type = VTSS_VCAP_KEY_TYPE_DOUBLE_TAG;

        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
#if defined(VTSS_FEATURE_QOS)
            vtss_state->qos.port_conf[port_no].key_type = key_type;
#endif /* VTSS_FEATURE_QOS */
            VTSS_RC(vtss_srvl_vcap_port_key_addr_set(vtss_state, port_no, 2, key_type, key_type, FALSE));
        }
        break;
    }
    default:
        break;
    }
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_SERVAL */
