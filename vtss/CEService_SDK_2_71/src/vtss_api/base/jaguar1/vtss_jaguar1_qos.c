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
#include "vtss_jaguar1_cil.h"

#if defined(VTSS_ARCH_JAGUAR_1)

/* - CIL functions ------------------------------------------------- */

static u32 jr_chip_prio(vtss_state_t *vtss_state, const vtss_prio_t prio)
{
    if (prio >= JR_PRIOS) {
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

/* Calculate policer rate

   If input rate is kbps (frame_rate = FALSE) then output rate is calculated like this:
   output rate = ((input rate * 1000) + 100159) / 100160
   This will round up the rate to the nearest possible value:
     0 -> 0 (Open until burst capacity is used, then closed)
     1..100 -> 1 (100160 bps)
   101..200 -> 2 (200320 bps)
   201..300 -> 3 (300480 bps)
   ...

   If input rate is frames per second (frame_rate = TRUE) then output rate is also frames per second.

   In both cases the maximum rate returned is limited to 0x1FFF.
*/
static u32 jr_calc_policer_rate(u32 rate, BOOL frame_rate)
{
    if (!frame_rate) { /* input rate is measured in kbps */
        rate = VTSS_DIV64(((u64)rate * 1000) + 100159, 100160);
    }
    return MIN(0x1ffff, rate);
}

/* Calculate policer burst level
*/
static u32 jr_calc_policer_level(u32 level, u32 rate, BOOL frame_rate)
{
    if (rate == 0) {
        return 0;                               /* Always closed */
    } else if ((rate == VTSS_BITRATE_DISABLED) || frame_rate) {
        return 0x3f;                            /* Maximum burst level */
    } else {
        return MIN(0x3f, MAX(1, level / 2048)); /* Calculated value 1..0x3f ~ 2048..129024 bytes */
    }
}

vtss_rc vtss_jr1_port_policer_fc_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no, u32 chipport)
{
    u32 i, fc_ena = 0;
    vtss_port_conf_t *port_conf = &vtss_state->port.conf[port_no];
    vtss_qos_port_conf_t *qos_conf = &vtss_state->qos.port_conf[port_no];

    if (port_conf->flow_control.generate) {
        for (i = 0; i < 4; i++) {
            if ((qos_conf->policer_port[i].rate != VTSS_BITRATE_DISABLED) && qos_conf->policer_ext_port[i].flow_control) {
                fc_ena |= 1 << i;
            }
        }
    }
    JR_WRX(ANA_AC, POL_ALL_CFG_POL_PORT_FC_CFG, chipport, fc_ena);
    JR_WRXB(DSM, CFG_ETH_FC_GEN, chipport, ETH_POLICED_PORT_FC_GEN, fc_ena ? 1 : 0); 

    return VTSS_RC_OK;
}

static vtss_rc jr_port_policer_set(vtss_state_t *vtss_state,
                                   u32 port, u32 idx, vtss_policer_t *conf, vtss_policer_ext_t *conf_ext)
{
    u32 pol_idx = (port * 4 + idx);
    u32 mask = 0, q, type = 0;
    
    /* Burst size and rate */
    JR_WRX(ANA_AC, POL_PORT_CFG_POL_PORT_THRES_CFG_0, pol_idx, jr_calc_policer_level(conf->level, conf->rate, conf_ext->frame_rate));
    JR_WRX(ANA_AC, POL_PORT_CFG_POL_PORT_THRES_CFG_1, pol_idx, 1); /* 2048 bytes flow control deassert threshold */
    JR_WRX(ANA_AC, POL_PORT_CFG_POL_PORT_RATE_CFG, pol_idx, jr_calc_policer_rate(conf->rate, conf_ext->frame_rate));

    /* GAP */
    JR_WRX(ANA_AC, POL_PORT_CTRL_POL_PORT_GAP, port, 0);

    /* CPU queue mask */
    for (q = 0; q < 8; q++) {
        if (conf_ext->cpu_queue[q])
            mask |= VTSS_BIT(q);
    }
    
    /* Traffic type mask */
    if (conf->rate != VTSS_BITRATE_DISABLED) {
        if (conf_ext->multicast)
            type |= VTSS_BIT(0);
        if (conf_ext->broadcast)
            type |= VTSS_BIT(1);
        if (conf_ext->unicast)
            type |= VTSS_BIT(2);
        if (conf_ext->flooded)
            type |= VTSS_BIT(3);
        if (conf_ext->to_cpu)
            type |= VTSS_BIT(4);
        if (conf_ext->learning)
            type |= VTSS_BIT(5);
    }
    
    JR_WRXY(ANA_AC, POL_PORT_CTRL_POL_PORT_CFG, port, idx,
            JR_PUT_FLD(ANA_AC, POL_PORT_CTRL_POL_PORT_CFG, CPU_QU_MASK, mask) |
            JR_PUT_FLD(ANA_AC, POL_PORT_CTRL_POL_PORT_CFG, 
                       DP_BYPASS_LVL, conf_ext->dp_bypass_level) |
            JR_PUT_BIT(ANA_AC, POL_PORT_CTRL_POL_PORT_CFG, FRAME_RATE_ENA, conf_ext->frame_rate) |
            JR_PUT_BIT(ANA_AC, POL_PORT_CTRL_POL_PORT_CFG, 
                       LIMIT_NONCPU_TRAFFIC_ENA, conf_ext->limit_noncpu_traffic) | 
            JR_PUT_BIT(ANA_AC, POL_PORT_CTRL_POL_PORT_CFG, 
                       LIMIT_CPU_TRAFFIC_ENA, conf_ext->limit_cpu_traffic) | 
            JR_PUT_BIT(ANA_AC, POL_PORT_CTRL_POL_PORT_CFG, 
                       MC_WITHOUT_FLOOD_ENA, conf_ext->mc_no_flood) |
            JR_PUT_BIT(ANA_AC, POL_PORT_CTRL_POL_PORT_CFG, 
                       UC_WITHOUT_FLOOD_ENA, conf_ext->uc_no_flood) |
            JR_PUT_FLD(ANA_AC, POL_PORT_CTRL_POL_PORT_CFG, TRAFFIC_TYPE_MASK, type));
    
    return VTSS_RC_OK;
}

static vtss_rc jr_queue_policer_set(vtss_state_t *vtss_state,
                                    u32 port, u32 queue, vtss_policer_t *conf)
{
    u32 pol_idx = (port * 8 + queue);

    JR_WRX(ANA_AC, POL_PRIO_CFG_POL_PRIO_RATE_CFG, pol_idx, jr_calc_policer_rate(conf->rate, FALSE));
    JR_WRX(ANA_AC, POL_PRIO_CFG_POL_PRIO_THRES_CFG, pol_idx, jr_calc_policer_level(conf->level, conf->rate, FALSE));
    JR_WRX(ANA_AC, POL_PRIO_CTRL_POL_PRIO_GAP, pol_idx, 0);
    JR_WRX(ANA_AC, POL_PRIO_CTRL_POL_PRIO_CFG, pol_idx, 0);

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_qos_wred_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_qos_port_conf_t *conf = &vtss_state->qos.port_conf[port_no];
    u32                  port = VTSS_CHIP_PORT(port_no);
    int                  m, queue;
    u32                  iqs_mtu[4], oqs_mtu[4];

    for (m = 0; m < 4; m++) {
        JR_RDX(IQS, MTU_MTU_FRM_SIZE_CFG, m, &iqs_mtu[m]);                
        JR_RDX(OQS, MTU_MTU_FRM_SIZE_CFG, m, &oqs_mtu[m]);                
    }

    for (queue = 0; queue < 8; queue++) {
        u32  instance = (port * 8) + queue;  
        u32  rc_cfg, hlwm, atop_swm, map_cfg, hwm, atop, mtu, l_th, h_th, min_th, max_th, minmax_th, gain;
        vtss_red_t *red = &conf->red[queue];

        /* Sanity check */
        if (red->max_th > 100) {
            VTSS_E("illegal max_th (%u) on port %u, queue %d", red->max_th, port_no, queue);
            return VTSS_RC_ERROR;
        }
        if (red->min_th > 100) {
            VTSS_E("illegal min_th (%u) on port %u, queue %d", red->min_th, port_no, queue);
            return VTSS_RC_ERROR;
        }
        if (red->max_prob_1 > 100) {
            VTSS_E("illegal max_prob_1 (%u) on port %u, queue %d", red->max_prob_1, port_no, queue);
            return VTSS_RC_ERROR;
        }
        if (red->max_prob_2 > 100) {
            VTSS_E("illegal max_prob_2 (%u) on port %u, queue %d", red->max_prob_2, port_no, queue);
            return VTSS_RC_ERROR;
        }
        if (red->max_prob_3 > 100) {
            VTSS_E("illegal max_prob_3 (%u) on port %u, queue %d", red->max_prob_3, port_no, queue);
            return VTSS_RC_ERROR;
        }
        if (red->min_th > red->max_th) {
            VTSS_E("min_th (%u) > max_th (%u) on port %u, queue %d", red->min_th, red->max_th, port_no, queue);
            return VTSS_RC_ERROR;
        }

        /* IQS */
        JR_RDX(IQS, QU_RAM_CFG_QU_RC_HLWM_CFG, instance, &hlwm);
        JR_RDX(IQS, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG, instance, &atop_swm);
        JR_RDX(IQS, QU_RAM_CFG_MTU_QU_MAP_CFG, instance, &map_cfg);
        JR_RDX(IQS, QU_RAM_CFG_QU_RC_CFG, instance, &rc_cfg);

        hwm  = VTSS_X_IQS_QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_HWM(hlwm);
        atop = VTSS_X_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_ATOP(atop_swm);
        mtu  = iqs_mtu[VTSS_X_IQS_QU_RAM_CFG_MTU_QU_MAP_CFG_MTU_PRE_ALLOC(map_cfg)];
        l_th = VTSS_X_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_SWM(atop_swm);                    /* l_th always == swn on IQS */
        h_th = (VTSS_F_IQS_QU_RAM_CFG_QU_RC_CFG_QU_FC_DROP_MODE & rc_cfg) ? (hwm - mtu) : (atop - mtu); /* h_th depends on drop_mode on IQS */
        if (l_th >= h_th) {
            l_th = h_th / 4; /* l_th must be less than h_th */
        } 
        min_th = (((h_th - l_th) * red->min_th) / 100) + l_th; /* convert min_th from pct to absolute value where 0% = l_th and 100% = h_th */
        max_th = (((h_th - l_th) * red->max_th) / 100) + l_th; /* convert max_th from pct to absolute value where 0% = l_th and 100% = h_th */
        minmax_th = max_th - min_th;
        gain = 0;
        while ((min_th > 255) || (minmax_th > 255)) {
            /* scale min_th and minmax_th down to be within 0..255 and adjust gain accordingly */ 
            min_th /= 2;
            minmax_th /= 2;
            gain++;
        }

        JR_WRXF(IQS, QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG, instance,  RED_ECN_PROFILE_MINTH, min_th);
        JR_WRXF(IQS, QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG, instance,  RED_ECN_PROFILE_MINMAXTH, minmax_th);

        JR_WRXF(IQS, QU_RAM_CFG_RED_ECN_MISC_CFG, instance, RED_ECN_PROF_MAXP,
                VTSS_ENCODE_BITFIELD((red->max_prob_1 * 255) / 100,  0, 8) |
                VTSS_ENCODE_BITFIELD((red->max_prob_2 * 255) / 100,  8, 8) |
                VTSS_ENCODE_BITFIELD((red->max_prob_3 * 255) / 100, 16, 8));
        JR_WRXF(IQS, QU_RAM_CFG_RED_ECN_MISC_CFG, instance, RED_THRES_GAIN, gain);
        JR_WRXB(IQS, QU_RAM_CFG_RED_ECN_MISC_CFG, instance, RED_ENA, 0); /* Never enable WRED on IQS (Bugzilla#9523) */

        JR_WRX(IQS, RED_RAM_RED_ECN_WQ_CFG, instance, VTSS_PORT_IS_10G(port) ? 4 : 6); /* Recommended values for IQS from VSC7460 Datasheet */

        /* OQS */
        JR_RDX(OQS, QU_RAM_CFG_QU_RC_HLWM_CFG, instance, &hlwm);
        JR_RDX(OQS, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG, instance, &atop_swm);
        JR_RDX(OQS, QU_RAM_CFG_MTU_QU_MAP_CFG, instance, &map_cfg);

        hwm  = VTSS_X_OQS_QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_HWM(hlwm);
        mtu  = iqs_mtu[VTSS_X_OQS_QU_RAM_CFG_MTU_QU_MAP_CFG_MTU_PRE_ALLOC(map_cfg)];
        l_th = VTSS_X_OQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_SWM(atop_swm); /* l_th always == swn on OQS */
        h_th = hwm - mtu;                                                            /* h_th always == hwm - mtu on OQS */
        if (l_th >= h_th) {
            l_th = h_th / 4; /* l_th must be less than h_th */
        } 
        min_th = (((h_th - l_th) * red->min_th) / 100) + l_th; /* convert min_th from pct to absolute value where 0% = l_th and 100% = h_th */
        max_th = (((h_th - l_th) * red->max_th) / 100) + l_th; /* convert max_th from pct to absolute value where 0% = l_th and 100% = h_th */
        minmax_th = max_th - min_th;
        gain = 0;
        while ((min_th > 255) || (minmax_th > 255)) {
            /* scale min_th and minmax_th down to be within 0..255 and adjust gain accordingly */ 
            min_th /= 2;
            minmax_th /= 2;
            gain++;
        }

        JR_WRXF(OQS, QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG, instance,  RED_ECN_PROFILE_MINTH, min_th);
        JR_WRXF(OQS, QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG, instance,  RED_ECN_PROFILE_MINMAXTH, minmax_th);

        JR_WRXF(OQS, QU_RAM_CFG_RED_ECN_MISC_CFG, instance, RED_ECN_PROF_MAXP,
                VTSS_ENCODE_BITFIELD((red->max_prob_1 * 255) / 100,  0, 8) |
                VTSS_ENCODE_BITFIELD((red->max_prob_2 * 255) / 100,  8, 8) |
                VTSS_ENCODE_BITFIELD((red->max_prob_3 * 255) / 100, 16, 8));
        JR_WRXF(OQS, QU_RAM_CFG_RED_ECN_MISC_CFG, instance, RED_THRES_GAIN, gain);
        JR_WRXB(OQS, QU_RAM_CFG_RED_ECN_MISC_CFG, instance, RED_ENA, red->enable);

        JR_WRX(OQS, RED_RAM_RED_ECN_WQ_CFG, instance, 0); /* Recommended values for OQS from Bugzilla#9523 */
    }

    return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
static vtss_rc jr_qos_port_dot3ar_rate_limiter_conf_set(vtss_state_t *vtss_state,
                                                        const vtss_port_no_t port_no)
{
    vtss_qos_port_conf_t                *conf = &vtss_state->qos.port_conf[port_no];
    vtss_qos_port_dot3ar_rate_limiter_t *tx_rate_limiter = &conf->tx_rate_limiter;
    u32                                 rate_limit_val;
    u32                                 rate_ctrl_val;
    u32                                 tmp_gap_val;
    u32                                 tmp_ipg_rate = 0, tmp_scale = 0;
    u32                                 frame_overhead_scale = 0;
    u8                                  mask = 0;
    u32                                 port = VTSS_CHIP_PORT(port_no);


    if(vtss_jr1_cemax_port_is_host(vtss_state, port_no) == TRUE) {
        /* 802.3ar is not supported on host interfaces according to the Bug# 4108 */
        if (tx_rate_limiter->frame_rate_enable == TRUE ||
            tx_rate_limiter->payload_rate_enable == TRUE ||
            tx_rate_limiter->frame_overhead_enable == TRUE) {
            return VTSS_RC_ERROR;
        } else {
            return VTSS_RC_OK; /* No need to go further */
        }
    }

    if (tx_rate_limiter->accumulate_mode_enable == TRUE && tx_rate_limiter->frame_rate_enable == TRUE) {
        /* if accumulate mode is enabled, frame rate limiting should be disabled */
        return VTSS_RC_ERROR;
    }

    tmp_gap_val = 0;
    JR_RDX(DSM, RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE, port_no, &rate_limit_val);

    /* == Calculate the Frame overhead configuration == */
    if (VTSS_PORT_IS_10G(port_no)) {
        tmp_gap_val = 12 + 8; /* Include preamble also as said in data sheet for other ports than Dev1G and Dev2.5G*/
    } else {
        tmp_gap_val = 12;
    }
    if (tx_rate_limiter->frame_overhead_enable) { /* Frame overhead is enabled */
        rate_limit_val |= VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_FRAME_OVERHEAD_ENA;
        if (tx_rate_limiter->frame_overhead > 12) {
            frame_overhead_scale = tx_rate_limiter->frame_overhead / 255;
            if (frame_overhead_scale > 1024) {
                return VTSS_RC_ERROR;
            } else if (frame_overhead_scale){
                if (frame_overhead_scale) {
                    while (frame_overhead_scale >= (1LU << mask))  {
                        mask++;
                    }
                    frame_overhead_scale = mask;
                }
                /* CEIL the gap value */
                tmp_gap_val = (tx_rate_limiter->frame_overhead / (1LU << frame_overhead_scale)) +
                    ((tx_rate_limiter->frame_overhead % (1LU << frame_overhead_scale)) ? 1 : 0);
            } else {
                tmp_gap_val = tx_rate_limiter->frame_overhead;
            }
        }
    } else {
        rate_limit_val &= (~VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_FRAME_OVERHEAD_ENA);
    }

    /* == Calculate Payload rate limiter configuration == */
    if (tx_rate_limiter->payload_rate_enable == TRUE) {
        rate_limit_val |= VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_PAYLOAD_RATE_ENA;
        /* Calculate the IPG_STRETCH_RATION based on bandwidth utilization */
        do {
            if (tx_rate_limiter->payload_rate == 100) {
                rate_limit_val &= (~VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_PAYLOAD_RATE_ENA);
                break;
            } else {
                tmp_ipg_rate = ( (1<<tmp_scale) * 256 * (tx_rate_limiter->payload_rate) ) /
                    (100-tx_rate_limiter->payload_rate);
                tmp_ipg_rate += ( (1<<tmp_scale) * 256 * (tx_rate_limiter->payload_rate) ) %
                    (100-tx_rate_limiter->payload_rate);

            }
            if (tmp_ipg_rate > 1152 && tmp_ipg_rate < 518143) { /* Range as specified in DS */
                break;
            }
            tmp_scale++;
        } while (tmp_scale <= 10);
        if (tmp_scale > 10) {
            return VTSS_RC_ERROR;
        }

    } else {
        rate_limit_val &= (~VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_PAYLOAD_RATE_ENA);
    }

    tmp_scale = (tmp_scale > frame_overhead_scale) ? tmp_scale : frame_overhead_scale;
    rate_limit_val &= (~VTSS_M_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_IPG_SCALE_VAL);
    rate_limit_val |= (VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_IPG_SCALE_VAL(tmp_scale));

    /* == Calculate Frame rate limiter configuration == */
    if (tx_rate_limiter->frame_rate_enable == TRUE) {
        rate_limit_val |= (VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_FRAME_RATE_ENA);
    } else {
        rate_limit_val &= (~VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_FRAME_RATE_ENA);
    }

    /* == Include/Exclude preamble from payload calculation == */
    if (tx_rate_limiter->preamble_in_payload == TRUE) {
        rate_limit_val |= VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_PAYLOAD_PREAM_CFG;
    } else {
        rate_limit_val &= (~VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_PAYLOAD_PREAM_CFG);
    }

    /* == Include/Exclude header size from payload calculation == */
    if (tx_rate_limiter->header_in_payload == TRUE) {
        rate_limit_val |= VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_PAYLOAD_CFG;
    } else {
        rate_limit_val &= (~VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_PAYLOAD_CFG);
    }

    /* == Enable/Disable accumulate mode == */
    if (tx_rate_limiter->accumulate_mode_enable == TRUE) {
        rate_limit_val |= VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_ACCUM_MODE_ENA;
    } else {
        rate_limit_val &= (~VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_ACCUM_MODE_ENA);
    }

    /* == Configure Rate CTRL value  == */
    JR_RDX(DSM, CFG_RATE_CTRL, port, &rate_ctrl_val);
    rate_ctrl_val &= (~VTSS_M_DSM_CFG_RATE_CTRL_FRM_GAP_COMP);
    rate_ctrl_val |= (VTSS_F_DSM_CFG_RATE_CTRL_FRM_GAP_COMP(tmp_gap_val));
    JR_WRX(DSM, CFG_RATE_CTRL, port, rate_ctrl_val);

    /* == Configure the IPG stretch ratio == */
    JR_WRX(DSM, RATE_LIMIT_CFG_TX_IPG_STRETCH_RATIO_CFG, port, tmp_ipg_rate);

    /* == Configure the Frame rate == */
    if (tx_rate_limiter->frame_rate < 64) {
        JR_WRX(DSM, RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG, port, 64);
    } else {
        JR_WRX(DSM, RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG, port, tx_rate_limiter->frame_rate);
    }

    /* == Configure the Rate limiters == */
    JR_WRX(DSM, RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE, port, rate_limit_val);

    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_DOT3AR_RATE_LIMITER */

vtss_rc vtss_jr1_qos_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_qos_port_conf_t *conf = &vtss_state->qos.port_conf[port_no];
    u32 port = VTSS_CHIP_PORT(port_no);
    int pcp, dei, queue, class, policer;
    u8 cost[6];
    u32  tag_remark_mode;
    BOOL tag_default_dei;

    /* Select device based on port map */
    VTSS_SELECT_CHIP_PORT_NO(port_no);

    /* Port default PCP and DEI configuration */
    JR_WRXF(ANA_CL_2, PORT_VLAN_CTRL, port, PORT_UPRIO, conf->usr_prio);
    JR_WRXB(ANA_CL_2, PORT_VLAN_CTRL, port, PORT_CFI,   conf->default_dei);
    
    /* Port default QoS class, DP level, tagged frames mode and DSCP mode configuration */
    JR_WRXF(ANA_CL_2, PORT_QOS_CFG, port, DEFAULT_QOS_VAL,    jr_chip_prio(vtss_state, conf->default_prio));
    JR_WRXF(ANA_CL_2, PORT_QOS_CFG, port, DEFAULT_DP_VAL,     conf->default_dpl);
    /* If translation is enabled, we must enable QoS classification based on the DSCP value also. */
    /* All trusted DSCP values will now be used for QoS classification as a side effect of this. */
    JR_WRXB(ANA_CL_2, PORT_QOS_CFG, port, DSCP_QOS_ENA,       conf->dscp_class_enable || conf->dscp_translate);
    JR_WRXB(ANA_CL_2, PORT_QOS_CFG, port, DSCP_DP_ENA,        conf->dscp_class_enable);


    /* Map for (PCP and DEI) to (QoS class and DP level) */
    for (pcp = VTSS_PCP_START; pcp < VTSS_PCP_END; pcp++) {
        for (dei = VTSS_DEI_START; dei < VTSS_DEI_END; dei++) {
            JR_WRXYF(ANA_CL_2, PORT_UPRIO_MAP_CFG, port, (8*dei + pcp), UPRIO_CFI_TRANSLATE_QOS_VAL, jr_chip_prio(vtss_state, conf->qos_class_map[pcp][dei]));
            JR_WRXYF(ANA_CL_2, PORT_DP_CONFIG,     port, (8*dei + pcp), UPRIO_CFI_DP_VAL,            conf->dp_level_map[pcp][dei]);
        }
    }

    /* Update vlan port configuration in order to enable/disable tag classification */
    VTSS_RC(vtss_cmn_vlan_port_conf_set(vtss_state, port_no));

    /* Enable gap value adjustment */
    JR_WRXB(SCH, ETH_ETH_LB_DWRR_CFG, port, FRM_ADJ_ENA, 1);

    /* DWRR configuration */
    VTSS_RC(vtss_cmn_qos_weight2cost(conf->queue_pct, cost, 6, 5));
    JR_WRXB(SCH, ETH_ETH_DWRR_CFG, port, DWRR_MODE, conf->dwrr_enable);
    JR_WRXF(SCH, ETH_ETH_DWRR_CFG, port, COST_CFG,
            VTSS_ENCODE_BITFIELD(cost[0],  0, 5) |
            VTSS_ENCODE_BITFIELD(cost[1],  5, 5) |
            VTSS_ENCODE_BITFIELD(cost[2], 10, 5) |
            VTSS_ENCODE_BITFIELD(cost[3], 15, 5) |
            VTSS_ENCODE_BITFIELD(cost[4], 20, 5) |
            VTSS_ENCODE_BITFIELD(cost[5], 25, 5));

    /* Egress port shaper burst level configuration
     * The value is rounded up to the next possible value:
     *           0 -> 0 (Always closed)
     *    1.. 4096 -> 1 ( 4 KB)
     * 4097.. 8192 -> 2 ( 8 KB)
     * 8193..12288 -> 3 (12 KB)
     */
    if (conf->shaper_port.rate == VTSS_BITRATE_DISABLED) {
        JR_WRXF(SCH, ETH_LB_ETH_LB_THRES, ((9 * port) + 8), LB_THRES,  0x3f); // Set burst level to maximum if shaper is disabled
    }
    else {
        JR_WRXF(SCH, ETH_LB_ETH_LB_THRES, ((9 * port) + 8), LB_THRES,  MIN(0x3f, ((conf->shaper_port.level + 4095) / 4096)));
    }

    /* Egress port shaper rate configuration
     * The value (in kbps) is rounded up to the next possible value:
     *        0 -> 0 (Open until burst capacity is used, then closed)
     *   1..100 -> 1 (100160 bps)
     * 101..200 -> 2 (200320 bps)
     * 201..300 -> 3 (300480 bps)
     */
    JR_WRXF(SCH, ETH_LB_ETH_LB_RATE, ((9 * port) + 8), LB_RATE, MIN(0x1ffff, VTSS_DIV64(((u64)conf->shaper_port.rate * 1000) + 100159, 100160)));

    /* Egress queue shaper rate and burst level configuration. See documentation above */
    for (queue = 0; queue < 8; queue++) {
        if (conf->shaper_queue[queue].rate == VTSS_BITRATE_DISABLED) {
            JR_WRXF(SCH, ETH_LB_ETH_LB_THRES, ((9 * port) + queue), LB_THRES,  0x3f); // Set burst level to maximum if shaper is disabled
        }
        else {
            JR_WRXF(SCH, ETH_LB_ETH_LB_THRES, ((9 * port) + queue), LB_THRES,  MIN(0x3f, ((conf->shaper_queue[queue].level + 4095) / 4096)));
        }
        JR_WRXF(SCH, ETH_LB_ETH_LB_RATE, ((9 * port) + queue), LB_RATE, MIN(0x1ffff, VTSS_DIV64(((u64)conf->shaper_queue[queue].rate * 1000) + 100159, 100160)));
    }

    /* Egress port and queue shaper enable/disable configuration
     * In Jaguar we cannot enable/disable shapers individually
     * so we always enable shaping.
     * If an individual shaper (port or queue) is disabled, we
     * must set rate and burst level to maximum.
     * The rate is automatically configured to maximum if the
     * value for disabled is applied (0xffffffff).
     * The burst level is explicit set to maximum is the shaper
     * is disabled
     */
    JR_WRXB(SCH, ETH_ETH_SHAPING_CTRL, port, SHAPING_ENA, 1);
    JR_WRXF(SCH, ETH_ETH_SHAPING_CTRL, port, PRIO_LB_EXS_ENA,
            (conf->excess_enable[0] ? VTSS_BIT(0) : 0) |
            (conf->excess_enable[1] ? VTSS_BIT(1) : 0) |
            (conf->excess_enable[2] ? VTSS_BIT(2) : 0) |
            (conf->excess_enable[3] ? VTSS_BIT(3) : 0) |
            (conf->excess_enable[4] ? VTSS_BIT(4) : 0) |
            (conf->excess_enable[5] ? VTSS_BIT(5) : 0) |
            (conf->excess_enable[6] ? VTSS_BIT(6) : 0) |
            (conf->excess_enable[7] ? VTSS_BIT(7) : 0));

    /* Tag remarking configuration
     */
    tag_remark_mode = conf->tag_remark_mode;
    tag_default_dei = (tag_remark_mode == VTSS_TAG_REMARK_MODE_DEFAULT ? 
                       conf->tag_default_dei : 0);
#if defined(VTSS_FEATURE_EVC)
    /* Enable DEI colouring for NNI ports */
    if (vtss_state->evc.port_info[port_no].nni_count) {
        /* Allow only CLASSIFIED and MAPPED modes */
        if (tag_remark_mode == VTSS_TAG_REMARK_MODE_DEFAULT)
            tag_remark_mode = VTSS_TAG_REMARK_MODE_CLASSIFIED;
        
        /* DEI colouring is enabled using default DEI field (requires revision B) */
        tag_default_dei = vtss_state->evc.port_conf[port_no].dei_colouring;
    }
#endif /* VTSS_FEATURE_EVC */    

    JR_WRXF(REW, PORT_TAG_CTRL,         port, QOS_SEL,       tag_remark_mode);
    JR_WRXF(REW, PORT_PORT_TAG_DEFAULT, port, PORT_TCI_PRIO, conf->tag_default_pcp);
    JR_WRXB(REW, PORT_PORT_TAG_DEFAULT, port, PORT_TCI_CFI,  tag_default_dei);
    JR_WRXF(REW, PORT_PORT_DP_MAP,      port, DP,
            (conf->tag_dp_map[0] ? VTSS_BIT(0) : 0) |
            (conf->tag_dp_map[1] ? VTSS_BIT(1) : 0) |
            (conf->tag_dp_map[2] ? VTSS_BIT(2) : 0) |
            (conf->tag_dp_map[3] ? VTSS_BIT(3) : 0));

    /* Map for (QoS class and DP level) to (PCP and DEI) */
    for (class = VTSS_QUEUE_START; class < VTSS_QUEUE_END; class++) {
        JR_WRXYF(REW, PORT_PCP_MAP_DE0, port, class, PCP_DE0, conf->tag_pcp_map[class][0]);
        JR_WRXYF(REW, PORT_PCP_MAP_DE1, port, class, PCP_DE1, conf->tag_pcp_map[class][1]);
        JR_WRXYB(REW, PORT_DEI_MAP_DE0, port, class, DEI_DE0, conf->tag_dei_map[class][0]);
        JR_WRXYB(REW, PORT_DEI_MAP_DE1, port, class, DEI_DE1, conf->tag_dei_map[class][1]);
    }

    /* DSCP remarking configuration
     */
    JR_WRXF(ANA_CL_2, PORT_QOS_CFG,  port, DSCP_REWR_MODE_SEL, conf->dscp_mode);
    JR_WRXB(ANA_CL_2, PORT_QOS_CFG,  port, DSCP_TRANSLATE_ENA, conf->dscp_translate);
    JR_WRXB(REW,      PORT_DSCP_MAP, port, DSCP_UPDATE_ENA,   (conf->dscp_emode > VTSS_DSCP_EMODE_DISABLE));
    JR_WRXB(REW,      PORT_DSCP_MAP, port, DSCP_REMAP_ENA,    (conf->dscp_emode > VTSS_DSCP_EMODE_REMARK));

    /* WRED configuration
     */
    VTSS_RC(vtss_jr1_qos_wred_conf_set(vtss_state, port_no));

    /* Port policer configuration
     */
    for (policer = 0; policer < 4; policer++) {
        VTSS_RC(jr_port_policer_set(vtss_state, port, policer, &conf->policer_port[policer], &conf->policer_ext_port[policer]));
    }

    /* Queue policer configuration */
    for (queue = 0; queue < 8; queue++) {
        VTSS_RC(jr_queue_policer_set(vtss_state, port, queue, &conf->policer_queue[queue]));
    }

    /* Update policer flow control configuration */
    VTSS_RC(vtss_jr1_port_policer_fc_set(vtss_state, port_no, port));

#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
    VTSS_RC(jr_qos_port_dot3ar_rate_limiter_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */

    return VTSS_RC_OK;
}

static vtss_rc jr_qos_conf_set_chip(vtss_state_t *vtss_state) 
{
    vtss_qos_conf_t *conf = &vtss_state->qos.conf;
    int             i;

    /* Frame adjustment (gap value - number of bytes added in leaky buckets and DWRR calculations) */
    JR_WRF(SCH, ETH_ETH_LB_DWRR_FRM_ADJ, FRM_ADJ, 20); /* 20 bytes added */

    /* DSCP classification and remarking configuration
     */
    for (i = 0; i < 64; i++) {
        JR_WRXF(ANA_CL_2, COMMON_DSCP_CFG,   i, DSCP_TRANSLATE_VAL, conf->dscp_translate_map[i]);
        JR_WRXF(ANA_CL_2, COMMON_DSCP_CFG,   i, DSCP_QOS_VAL,       jr_chip_prio(vtss_state, conf->dscp_qos_class_map[i]));
        JR_WRXF(ANA_CL_2, COMMON_DSCP_CFG,   i, DSCP_DP_VAL,        conf->dscp_dp_level_map[i]);
        JR_WRXB(ANA_CL_2, COMMON_DSCP_CFG,   i, DSCP_REWR_ENA,      conf->dscp_remark[i]);
        JR_WRXB(ANA_CL_2, COMMON_DSCP_CFG,   i, DSCP_TRUST_ENA,     conf->dscp_trust[i]);
        JR_WRXF(REW,      COMMON_DSCP_REMAP, i, DSCP_REMAP,         conf->dscp_remap[i]);
    }

    /* DSCP classification from QoS configuration
     */
    for (i = 0; i < 8; i++) {
        JR_WRXF(ANA_CL_2, COMMON_QOS_MAP_CFG, i, DSCP_REWR_VAL, conf->dscp_qos_map[i]);
    }

#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
    JR_WR(DSM, RATE_LIMIT_CFG_TX_RATE_LIMIT_HDR_CFG, conf->header_size);
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */

    return VTSS_RC_OK;
}

static vtss_rc jr_qos_conf_set(vtss_state_t *vtss_state, BOOL changed)
{
    vtss_port_no_t port_no;
        
    if (changed) {
        /* Number of priorities changed, update QoS setup for all ports */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            VTSS_RC(vtss_jr1_qos_port_conf_set(vtss_state, port_no));   
        }
    }

    /* Setup config for all devices */
    return vtss_jr1_conf_chips(vtss_state, jr_qos_conf_set_chip);
}

#ifdef VTSS_FEATURE_VSTAX_V2
vtss_rc vtss_jr1_cpu_pol_setup_chip(vtss_state_t *vtss_state)
{
    vtss_policer_t             pol_conf;
    vtss_policer_ext_t         pol_ext_conf;
    vtss_packet_rx_queue_map_t *map;
    u32                        q, port;

    /* Only setup internal ports if dual device target */
    if (vtss_state->chip_count != 2) {
        return VTSS_RC_OK;
    }

    map = &vtss_state->packet.rx_conf.map;

    /* Use policers to avoid CPU copy on internal ports.
     * If we didn't do this, frames that for some reason come
     * to both chips (e.g. broadcast frames) would be copied
     * twice to the CPU - once on the primary and once on the
     * secondary chip.
     */
    memset(&pol_conf,     0, sizeof(pol_conf));
    memset(&pol_ext_conf, 0, sizeof(pol_ext_conf));

    pol_ext_conf.to_cpu = 1;
    pol_ext_conf.limit_cpu_traffic = 1;
    for (q = 0; q < 8; q++) {
        if (q != map->sflow_queue && q != map->stack_queue && q != map->l3_other_queue && q != map->l3_uc_queue) {
            /* We need multiple CPU copies for sFlow frames */
            /* In reality, we only need to allow l3_other_queue and l3_uc_queue on
               the primary chip of a JR-48 and only if it is master. */
            pol_ext_conf.cpu_queue[q] = 1;
        }
    }

    for (port = vtss_state->port.port_int_0; port <= vtss_state->port.port_int_1; port++) {
        /* Use policers to avoid CPU copy */
        VTSS_RC(jr_port_policer_set(vtss_state, port, 0, &pol_conf, &pol_ext_conf));
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_VSTAX_V2 */

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
static vtss_rc jr_qos_lport_wred_conf_set(vtss_state_t *vtss_state, const vtss_lport_no_t lport_no)
{
    vtss_qos_lport_conf_t *conf = &vtss_state->qos.lport[lport_no];
    int                   queue;
    u32                   oqs_mtu[4];
    u8                    m, total_queues = 0;
    vtss_rc               rc = VTSS_RC_OK;

    for (m = 0; m < 4; m++) {
        JR_RDX(OQS, MTU_MTU_FRM_SIZE_CFG, m, &oqs_mtu[m]);
    }

    switch(vtss_state->init_conf.host_mode) {
#if defined(VTSS_CHIP_CE_MAX_24)
        case VTSS_HOST_MODE_0:
            total_queues = 8;
            break;
        case VTSS_HOST_MODE_1:
            total_queues = 4;
            break;
#endif
        case VTSS_HOST_MODE_2:
            total_queues = 8;
            break;
        case VTSS_HOST_MODE_3:
            total_queues = 4;
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
    }
    if (rc != VTSS_RC_OK) {
        return rc;
    }
    for (queue = 0; queue < total_queues; queue++) {
        u32        instance = 256 + (lport_no * total_queues) + queue;
        u32        hlwm, atop_swm, map_cfg, hwm, mtu, l_th, h_th, min_th, max_th, minmax_th, gain;
        vtss_red_t *red = &conf->red[queue];

        /* Sanity check */
        if (red->max_th > 100) {
            VTSS_E("illegal max_th (%u) on lport %u, queue %d", red->max_th, lport_no, queue);
            return VTSS_RC_ERROR;
        }
        if (red->min_th > 100) {
            VTSS_E("illegal min_th (%u) on lport %u, queue %d", red->min_th, lport_no, queue);
            return VTSS_RC_ERROR;
        }
        if (red->max_prob_1 > 100) {
            VTSS_E("illegal max_prob_1 (%u) on lport %u, queue %d", red->max_prob_1, lport_no, queue);
            return VTSS_RC_ERROR;
        }
        if (red->max_prob_2 > 100) {
            VTSS_E("illegal max_prob_2 (%u) on lport %u, queue %d", red->max_prob_2, lport_no, queue);
            return VTSS_RC_ERROR;
        }
        if (red->max_prob_3 > 100) {
            VTSS_E("illegal max_prob_3 (%u) on lport %u, queue %d", red->max_prob_3, lport_no, queue);
            return VTSS_RC_ERROR;
        }
        if (red->min_th > red->max_th) {
            VTSS_E("min_th (%u) > max_th (%u) on lport %u, queue %d", red->min_th, red->max_th,
                    lport_no, queue);
            return VTSS_RC_ERROR;
        }

        /* OQS */
        JR_RDX(OQS, QU_RAM_CFG_QU_RC_HLWM_CFG, instance, &hlwm);
        JR_RDX(OQS, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG, instance, &atop_swm);
        JR_RDX(OQS, QU_RAM_CFG_MTU_QU_MAP_CFG, instance, &map_cfg);

        hwm  = VTSS_X_OQS_QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_HWM(hlwm);
        mtu  = oqs_mtu[VTSS_X_OQS_QU_RAM_CFG_MTU_QU_MAP_CFG_MTU_PRE_ALLOC(map_cfg)];
        l_th = VTSS_X_OQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_SWM(atop_swm); /* l_th always == swn on OQS */
        h_th = hwm - mtu;                                                            /* h_th always == hwm - mtu on OQS */
        if (l_th >= h_th) {
            l_th = h_th / 4; /* l_th must be less than h_th */
        }
        min_th = (((h_th - l_th) * red->min_th) / 100) + l_th; /* convert min_th from pct to absolute value where 0% = l_th and 100% = h_th */
        max_th = (((h_th - l_th) * red->max_th) / 100) + l_th; /* convert max_th from pct to absolute value where 0% = l_th and 100% = h_th */
        minmax_th = max_th - min_th;
        gain = 0;
        while ((min_th > 255) || (minmax_th > 255)) {
            /* scale min_th and minmax_th down to be within 0..255 and adjust gain accordingly */
            min_th /= 2;
            minmax_th /= 2;
            gain++;
        }
        JR_WRXF(OQS, QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG, instance,  RED_ECN_PROFILE_MINTH, min_th);
        JR_WRXF(OQS, QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG, instance,  RED_ECN_PROFILE_MINMAXTH, minmax_th);

        JR_WRXF(OQS, QU_RAM_CFG_RED_ECN_MISC_CFG, instance, RED_ECN_PROF_MAXP,
                VTSS_ENCODE_BITFIELD((red->max_prob_1 * 255) / 100,  0, 8) |
                VTSS_ENCODE_BITFIELD((red->max_prob_2 * 255) / 100,  8, 8) |
                VTSS_ENCODE_BITFIELD((red->max_prob_3 * 255) / 100, 16, 8));
        JR_WRXF(OQS, QU_RAM_CFG_RED_ECN_MISC_CFG, instance, RED_THRES_GAIN, gain);
        JR_WRXB(OQS, QU_RAM_CFG_RED_ECN_MISC_CFG, instance, RED_ENA, red->enable);

        JR_WRX(OQS, RED_RAM_RED_ECN_WQ_CFG, instance, 3); /* Recommended values for OQS from VSC7460 Datasheet */
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_qos_lport_dwrr_conf_set(vtss_state_t *vtss_state, const vtss_lport_no_t lport_no)
{
    vtss_qos_lport_conf_t *tmp_lport_conf;
    u8                    lport_pct[VTSS_LPORTS];
    u8                    lport_cost[VTSS_LPORTS];
    u8                    tmp_instance = 0;
    vtss_lport_no_t       tmp_lport = 0, total_lports = 0;
    vtss_rc               rc = VTSS_RC_OK;
    u8                    tmp;
    vtss_host_mode_t      host_mode = vtss_state->init_conf.host_mode;

    /* Enable the DWRR configurations */
    switch (host_mode) {
#if defined(VTSS_CHIP_CE_MAX_24)
        case VTSS_HOST_MODE_0:
            tmp_instance = (lport_no > 11) ? 1 : 0;
            tmp_lport = (tmp_instance) * 12;
            total_lports = (tmp_instance + 1) * 12;
            break;
        case VTSS_HOST_MODE_1:
            tmp_instance = (lport_no > 23) ? 1 : 0;
            tmp_lport = (tmp_instance) * 24;
            total_lports = (tmp_instance + 1) * 24;
            break;
#endif
        case VTSS_HOST_MODE_2:
            tmp_instance = 0;
            tmp_lport = (tmp_instance) * 24;
            total_lports = (tmp_instance + 1) * 24;
            break;
        case VTSS_HOST_MODE_3:
            tmp_instance = 0;
            tmp_lport = (tmp_instance) * 48;
            total_lports = (tmp_instance + 1) * 48;
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
    }
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    /* Build lport specific DWRR costs */
    for (tmp = 0; tmp_lport < total_lports; tmp_lport++, tmp++) {
        tmp_lport_conf = &vtss_state->qos.lport[tmp_lport];
        lport_pct[tmp] = tmp_lport_conf->lport_pct;
    }

    VTSS_RC(vtss_cmn_qos_weight2cost(lport_pct, lport_cost, tmp, 7));

    switch (host_mode) {
#if defined(VTSS_CHIP_CE_MAX_24)
        case VTSS_HOST_MODE_0:
            tmp_lport = (tmp_instance) * 12;
            total_lports = (tmp_instance + 1) * 12;
            break;
        case VTSS_HOST_MODE_1:
            tmp_lport = (tmp_instance) * 24;
            total_lports = (tmp_instance + 1) * 24;
            break;
#endif
        case VTSS_HOST_MODE_2:
            tmp_lport = (tmp_instance) * 24;
            total_lports = (tmp_instance + 1) * 24;
            break;
        case VTSS_HOST_MODE_3:
            tmp_lport = (tmp_instance) * 48;
            total_lports = (tmp_instance + 1) * 48;
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
    }
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    /* Configure the HW with LPORT costs */
    for (tmp = 0; tmp_lport < total_lports; tmp_lport++, tmp++) {
        JR_WRXF(SCH, HM_HM_DWRR_COST, tmp_lport, DWRR_COST,lport_cost[tmp]);
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_qos_lport_conf_set(vtss_state_t *vtss_state, const vtss_lport_no_t lport_no)
{
    vtss_qos_lport_conf_t *conf;
    vtss_host_mode_t      host_mode = vtss_state->init_conf.host_mode;
    vtss_rc               rc = VTSS_RC_OK;
    u8                    queue_cnt = 0;
    u8                    queue_cost[6];

    switch(host_mode) {
#if defined(VTSS_CHIP_CE_MAX_24)
        case VTSS_HOST_MODE_0:
            if (lport_no > 11) {
                rc = VTSS_RC_ERROR;
            }
            break;
        case VTSS_HOST_MODE_1:
            if (lport_no > 23) {
                rc = VTSS_RC_ERROR;
            }
            break;
#endif
        case VTSS_HOST_MODE_2:
            if (lport_no > 23) {
                rc = VTSS_RC_ERROR;
            }
            break;
        case VTSS_HOST_MODE_3:
            if (lport_no > 47) {
                rc = VTSS_RC_ERROR;
            }
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
    }
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    if (lport_no >= VTSS_LPORTS) { /* To make Lint happy, added this */
        return VTSS_RC_ERROR;
    }

    conf = &vtss_state->qos.lport[lport_no];
    JR_WRXB(SCH, HM_HM_DWRR_AND_LB, lport_no, FRM_ADJ_ENA, 1);

    if (conf->shaper.rate == VTSS_BITRATE_DISABLED) {
        // Set burst level to maximum if shaper is disabled
        JR_WRXF(SCH, HM_LB_HM_LB_THRES, ((3 * lport_no) + 2), LB_THRES,  0x3f);
    }
    else {
        JR_WRXF(SCH, HM_LB_HM_LB_THRES, ((3 * lport_no) + 2), LB_THRES,  MIN(0x3f, ((conf->shaper.level + 4095) / 4096)));
    }

    /* Enable Logical port LB Shaping */
    if (conf->shaper.rate == VTSS_BITRATE_DISABLED) {
        JR_WRXB(SCH, HM_HM_LB_CFG, (3*lport_no)+2, LB_SHAPING_ENA, 0);
    } else {
        JR_WRXB(SCH, HM_HM_LB_CFG, (3*lport_no)+2, LB_SHAPING_ENA, 1);
    }
    /* Enable Logical port Shaper */
    JR_WRXF(SCH, HM_LB_HM_LB_RATE, (3*lport_no)+2, LB_RATE, MIN(0x1ffff,
                VTSS_DIV64(((u64)conf->shaper.rate * 1000) + 100159, 100160)));

    for (queue_cnt = 0; queue_cnt < 2; queue_cnt++) {

        if (conf->scheduler.queue[queue_cnt].rate == VTSS_BITRATE_DISABLED) {
            // Set burst level to maximum if shaper is disabled
            JR_WRXF(SCH, HM_LB_HM_LB_THRES, ((3 * lport_no) + queue_cnt), LB_THRES,  0x3f);
        }
        else {
            JR_WRXF(SCH, HM_LB_HM_LB_THRES, ((3 * lport_no) + queue_cnt), LB_THRES,
                    MIN(0x3f, ((conf->scheduler.queue[queue_cnt].level + 4095) / 4096)));
        }

        /* Enable Q7, Q6 LB Shaping */
        if (conf->scheduler.queue[queue_cnt].rate == VTSS_BITRATE_DISABLED) {
            JR_WRXB(SCH, HM_HM_LB_CFG, (3*lport_no)+queue_cnt, LB_SHAPING_ENA, 0);
        } else {
            JR_WRXB(SCH, HM_HM_LB_CFG, (3*lport_no)+queue_cnt, LB_SHAPING_ENA, 1);
        }
        /* Enable Logical queue Shaper */
        JR_WRXF(SCH, HM_LB_HM_LB_RATE, (3*lport_no)+queue_cnt, LB_RATE, MIN(0x1ffff,
                    VTSS_DIV64(((u64)conf->scheduler.queue[queue_cnt].rate * 1000) + 100159, 100160)));
    }

    /* Enable the DWRR configurations */
    switch (host_mode) {
#if defined(VTSS_CHIP_CE_MAX_24)
        case VTSS_HOST_MODE_0:
            VTSS_RC(vtss_cmn_qos_weight2cost(conf->scheduler.queue_pct, queue_cost, 6, 7));
            for (queue_cnt = 0; queue_cnt < 6; queue_cnt++) {
                JR_WRXF(SCH, HM_HM_DWRR_COST, (6*lport_no)+24+queue_cnt, DWRR_COST,queue_cost[queue_cnt]);
            }
            break;
        case VTSS_HOST_MODE_1:
            VTSS_RC(vtss_cmn_qos_weight2cost(conf->scheduler.queue_pct, queue_cost, 2, 7));
            for (queue_cnt = 0; queue_cnt < 2; queue_cnt++) {
                JR_WRXF(SCH, HM_HM_DWRR_COST, (2*lport_no)+48+queue_cnt, DWRR_COST,queue_cost[queue_cnt]);
            }
            break;
#endif
        case VTSS_HOST_MODE_2:
            VTSS_RC(vtss_cmn_qos_weight2cost(conf->scheduler.queue_pct, queue_cost, 6, 7));
            for (queue_cnt = 0; queue_cnt < 6; queue_cnt++) {
                JR_WRXF(SCH, HM_HM_DWRR_COST, (6*lport_no)+24+queue_cnt, DWRR_COST,queue_cost[queue_cnt]);
            }
            break;
        case VTSS_HOST_MODE_3:
            VTSS_RC(vtss_cmn_qos_weight2cost(conf->scheduler.queue_pct, queue_cost, 2, 7));
            for (queue_cnt = 0; queue_cnt < 2; queue_cnt++) {
                JR_WRXF(SCH, HM_HM_DWRR_COST, (2*lport_no)+48+queue_cnt, DWRR_COST,queue_cost[queue_cnt]);
            }
            break;
        default:
            break;
    }

    /* Configure the LPORT DWRR cost */
    rc = jr_qos_lport_dwrr_conf_set(vtss_state, lport_no);
    if (rc != VTSS_RC_OK) {
        return rc;
    }
    /* WRED configuration */
    rc = jr_qos_lport_wred_conf_set(vtss_state, lport_no);

    return rc;
}
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

/* - Debug print --------------------------------------------------- */

static vtss_rc jr_debug_qos(vtss_state_t *vtss_state,
                            const vtss_debug_printf_t pr,
                            const vtss_debug_info_t   *const info)
{
    u32            value, port;
    vtss_port_no_t port_no;
    int            i;
    u32            iqs_mtu[4], oqs_mtu[4];
    char           buf[32];

    vtss_debug_print_header(pr, "QoS Port Classification Config");

    pr("Port PCP CLS DEI DPL D_EN Q_EN DD_EN DQ_EN\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 vlan, qos;
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        JR_RDX(ANA_CL_2, PORT_VLAN_CTRL, port, &vlan);
        JR_RDX(ANA_CL_2, PORT_QOS_CFG, port, &qos);
        pr("%4u %3u %3u %3u %3u %4u %4u %5u %5u\n",
           port_no,
           VTSS_X_ANA_CL_2_PORT_VLAN_CTRL_PORT_UPRIO(vlan),
           VTSS_X_ANA_CL_2_PORT_QOS_CFG_DEFAULT_QOS_VAL(qos),
           VTSS_BOOL(vlan & VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_PORT_CFI),
           VTSS_X_ANA_CL_2_PORT_QOS_CFG_DEFAULT_DP_VAL(qos),
           VTSS_BOOL(qos & VTSS_F_ANA_CL_2_PORT_QOS_CFG_UPRIO_DP_ENA),
           VTSS_BOOL(qos & VTSS_F_ANA_CL_2_PORT_QOS_CFG_UPRIO_QOS_ENA),
           VTSS_BOOL(qos & VTSS_F_ANA_CL_2_PORT_QOS_CFG_DSCP_DP_ENA),
           VTSS_BOOL(qos & VTSS_F_ANA_CL_2_PORT_QOS_CFG_DSCP_QOS_ENA));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port Classification PCP, DEI to QoS class, DP level Mapping");

    pr("Port QoS class (8*DEI+PCP)           DP level (8*DEI+PCP)\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        int pcp, dei, class_ct = 0, dpl_ct = 0;
        u32 class, dpl;
        char class_buf[40], dpl_buf[40];
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        for (dei = VTSS_DEI_START; dei < VTSS_DEI_END; dei++) {
            for (pcp = VTSS_PCP_START; pcp < VTSS_PCP_END; pcp++) {
                const char *delim = ((pcp == VTSS_PCP_START) && (dei == VTSS_DEI_START)) ? "" : ",";
                JR_RDXY(ANA_CL_2, PORT_UPRIO_MAP_CFG, port, (8*dei + pcp), &class);
                JR_RDXY(ANA_CL_2, PORT_DP_CONFIG,     port, (8*dei + pcp), &dpl);
                class_ct += snprintf(class_buf + class_ct, sizeof(class_buf) - class_ct, "%s%u", delim,
                                     VTSS_X_ANA_CL_2_PORT_UPRIO_MAP_CFG_UPRIO_CFI_TRANSLATE_QOS_VAL(class));
                dpl_ct   += snprintf(dpl_buf   + dpl_ct,   sizeof(dpl_buf)   - dpl_ct,   "%s%u",  delim,
                                     VTSS_X_ANA_CL_2_PORT_DP_CONFIG_UPRIO_CFI_DP_VAL(dpl));
            }
        }
        pr("%4u %s %s\n", port_no, class_buf, dpl_buf);
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port Leaky Bucket and Scheduler Config");

    JR_RD(SCH, ETH_ETH_LB_DWRR_FRM_ADJ, &value);
    pr("Frame Adjustment (gap value): %u bytes\n", VTSS_X_SCH_ETH_ETH_LB_DWRR_FRM_ADJ_FRM_ADJ(value));
    pr("Port F_EN Mode C0 C1 C2 C3 C4 C5\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 frm_adj_ena, dwrr_cfg, cost;
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        JR_RDX(SCH, ETH_ETH_LB_DWRR_CFG, port, &frm_adj_ena);
        JR_RDX(SCH, ETH_ETH_DWRR_CFG, port, &dwrr_cfg);
        cost = VTSS_X_SCH_ETH_ETH_DWRR_CFG_COST_CFG(dwrr_cfg);
        pr("%4u %4u %4u %2u %2u %2u %2u %2u %2u\n",
           port_no,
           VTSS_BOOL(frm_adj_ena & VTSS_F_SCH_ETH_ETH_LB_DWRR_CFG_FRM_ADJ_ENA),
           VTSS_BOOL(dwrr_cfg & VTSS_F_SCH_ETH_ETH_DWRR_CFG_DWRR_MODE),
           VTSS_EXTRACT_BITFIELD(cost,  0, 5),
           VTSS_EXTRACT_BITFIELD(cost,  5, 5),
           VTSS_EXTRACT_BITFIELD(cost, 10, 5),
           VTSS_EXTRACT_BITFIELD(cost, 15, 5),
           VTSS_EXTRACT_BITFIELD(cost, 20, 5),
           VTSS_EXTRACT_BITFIELD(cost, 25, 5));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port and Queue Shaper enable/disable Config");

    pr("Port S Ex Q0-Q7\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 queue_excess;
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        JR_RDX(SCH, ETH_ETH_SHAPING_CTRL, port, &value);
        queue_excess = VTSS_X_SCH_ETH_ETH_SHAPING_CTRL_PRIO_LB_EXS_ENA(value);
        pr("%4u %u %u%u%u%u%u%u%u%u\n",
           port_no,
           VTSS_BOOL(value & VTSS_F_SCH_ETH_ETH_SHAPING_CTRL_SHAPING_ENA),
           VTSS_BOOL(queue_excess & VTSS_BIT(0)),
           VTSS_BOOL(queue_excess & VTSS_BIT(1)),
           VTSS_BOOL(queue_excess & VTSS_BIT(2)),
           VTSS_BOOL(queue_excess & VTSS_BIT(3)),
           VTSS_BOOL(queue_excess & VTSS_BIT(4)),
           VTSS_BOOL(queue_excess & VTSS_BIT(5)),
           VTSS_BOOL(queue_excess & VTSS_BIT(6)),
           VTSS_BOOL(queue_excess & VTSS_BIT(7)));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port and Queue Shaper Burst and Rate Config");

    pr("Port Queue Burst Rate\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 burst, rate;
        int queue;
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        JR_RDX(SCH, ETH_LB_ETH_LB_THRES, ((9 * port) + 8), &burst);
        JR_RDX(SCH, ETH_LB_ETH_LB_RATE,  ((9 * port) + 8), &rate);
        
        pr("%4u     - 0x%02x  0x%04x\n     ",
           port_no,
           VTSS_X_SCH_ETH_LB_ETH_LB_THRES_LB_THRES(burst),
           VTSS_X_SCH_ETH_LB_ETH_LB_RATE_LB_RATE(rate));
        for (queue = 0; queue < 8; queue++) {
            JR_RDX(SCH, ETH_LB_ETH_LB_THRES, ((9 * port) + queue), &burst);
            JR_RDX(SCH, ETH_LB_ETH_LB_RATE,  ((9 * port) + queue), &rate);
            pr("%5d 0x%02x  0x%04x\n     ",
               queue,
               VTSS_X_SCH_ETH_LB_ETH_LB_THRES_LB_THRES(burst),
               VTSS_X_SCH_ETH_LB_ETH_LB_RATE_LB_RATE(rate));
        }
        pr("\r");
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port Tag Remarking Config");

    pr("Port Mode PCP DEI DPL mapping\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 tag_default, dp_map, tag_ctrl;
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        JR_RDX(REW, PORT_PORT_TAG_DEFAULT, port, &tag_default);
        JR_RDX(REW, PORT_PORT_DP_MAP,      port, &dp_map);
        JR_RDX(REW, PORT_TAG_CTRL,         port, &tag_ctrl);
        pr("%4u %4x %3d %3d   %d %d %d %d\n",
           port_no,
           VTSS_X_REW_PORT_TAG_CTRL_QOS_SEL(tag_ctrl),
           VTSS_X_REW_PORT_PORT_TAG_DEFAULT_PORT_TCI_PRIO(tag_default),
           VTSS_BOOL(tag_default & VTSS_F_REW_PORT_PORT_TAG_DEFAULT_PORT_TCI_CFI),
           VTSS_BOOL(dp_map & VTSS_BIT(0)),
           VTSS_BOOL(dp_map & VTSS_BIT(1)),
           VTSS_BOOL(dp_map & VTSS_BIT(2)),
           VTSS_BOOL(dp_map & VTSS_BIT(3)));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port Tag Remarking Map");

    pr("Port PCP (2*QoS class+DPL)           DEI (2*QoS class+DPL)\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        int class, pcp_ct = 0, dei_ct = 0;
        u32 pcp_dp0, pcp_dp1, dei_dp0, dei_dp1;
        char pcp_buf[40], dei_buf[40];
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        for (class = VTSS_QUEUE_START; class < VTSS_QUEUE_END; class++) {
            const char *delim = (class == VTSS_QUEUE_START) ? "" : ",";
            JR_RDXY(REW, PORT_PCP_MAP_DE0, port, class, &pcp_dp0);
            JR_RDXY(REW, PORT_PCP_MAP_DE1, port, class, &pcp_dp1);
            JR_RDXY(REW, PORT_DEI_MAP_DE0, port, class, &dei_dp0);
            JR_RDXY(REW, PORT_DEI_MAP_DE1, port, class, &dei_dp1);
            pcp_ct += snprintf(pcp_buf + pcp_ct, sizeof(pcp_buf) - pcp_ct, "%s%u,%u",
                               delim,
                               VTSS_X_REW_PORT_PCP_MAP_DE0_PCP_DE0(pcp_dp0),
                               VTSS_X_REW_PORT_PCP_MAP_DE1_PCP_DE1(pcp_dp1));
            dei_ct += snprintf(dei_buf + dei_ct, sizeof(dei_buf) - dei_ct, "%s%u,%u",
                               delim,
                               VTSS_BOOL(dei_dp0 & VTSS_F_REW_PORT_DEI_MAP_DE0_DEI_DE0),
                               VTSS_BOOL(dei_dp1 & VTSS_F_REW_PORT_DEI_MAP_DE1_DEI_DE1));
        }
        pr("%4u %s %s\n", port_no, pcp_buf, dei_buf);
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Port DSCP Remarking Config");

    pr("Port I_Mode Keep Trans Update Remap\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        u32 qos_cfg, dscp_map;
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        JR_RDX(ANA_CL_2, PORT_QOS_CFG, port, &qos_cfg);
        JR_RDX(REW, PORT_DSCP_MAP, port, &dscp_map);
        pr("%4u %6u %4u %5u %6u %5u\n",
           port_no,
           VTSS_X_ANA_CL_2_PORT_QOS_CFG_DSCP_REWR_MODE_SEL(qos_cfg),
           VTSS_BOOL(qos_cfg & VTSS_F_ANA_CL_2_PORT_QOS_CFG_DSCP_KEEP_ENA),
           VTSS_BOOL(qos_cfg & VTSS_F_ANA_CL_2_PORT_QOS_CFG_DSCP_TRANSLATE_ENA),
           VTSS_BOOL(dscp_map & VTSS_F_REW_PORT_DSCP_MAP_DSCP_UPDATE_ENA),
           VTSS_BOOL(dscp_map & VTSS_F_REW_PORT_DSCP_MAP_DSCP_REMAP_ENA));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS DSCP Config");

    pr("DSCP Trans CLS DPL Rewr Trust Remap\n");
    for (i = 0; i < 64; i++) {
        u32 dscp_cfg, dscp_remap;
        JR_RDX(ANA_CL_2, COMMON_DSCP_CFG, i, &dscp_cfg);
        JR_RDX(REW, COMMON_DSCP_REMAP, i, &dscp_remap);
        pr("%4u %5u %3u %3u %4u %5u %5u\n",
           i,
           VTSS_X_ANA_CL_2_COMMON_DSCP_CFG_DSCP_TRANSLATE_VAL(dscp_cfg),
           VTSS_X_ANA_CL_2_COMMON_DSCP_CFG_DSCP_QOS_VAL(dscp_cfg),
           VTSS_X_ANA_CL_2_COMMON_DSCP_CFG_DSCP_DP_VAL(dscp_cfg),
           VTSS_BOOL(dscp_cfg & VTSS_F_ANA_CL_2_COMMON_DSCP_CFG_DSCP_REWR_ENA),
           VTSS_BOOL(dscp_cfg & VTSS_F_ANA_CL_2_COMMON_DSCP_CFG_DSCP_TRUST_ENA),
           VTSS_X_REW_COMMON_DSCP_REMAP_DSCP_REMAP(dscp_remap));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS DSCP Classification from QoS Config");

    pr("QoS DSCP\n");
    for (i = 0; i < 8; i++) {
        u32 qos;
        JR_RDX(ANA_CL_2, COMMON_QOS_MAP_CFG, i, &qos);
        pr("%3u %4u\n",
           i,
           VTSS_X_ANA_CL_2_COMMON_QOS_MAP_CFG_DSCP_REWR_VAL(qos));
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS WRED Config");

    for (i = 0; i < 4; i++) {
        RDX_SQS(0, MTU_MTU_FRM_SIZE_CFG, i, &iqs_mtu[i]);                
        RDX_SQS(1, MTU_MTU_FRM_SIZE_CFG, i, &oqs_mtu[i]);                
    }

    pr("Port Q I/O E G Min MMx Dp1 Dp2 Dp3 Wq D   SWM   HWM  ATOP  MTU\n");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        int queue, output;
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        pr("%4u ", port_no);
        for (queue = 0; queue < 8; queue++) {
            u32 instance = (port * 8) + queue;  
            for (output = 0; output < 2; output++) {
                u32 red_min_max, red_misc, red_wq, maxp, rc_cfg, hlwm, atop_swm, map_cfg, mtu_ix;
                RDX_SQS(output, QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG, instance, &red_min_max);
                RDX_SQS(output, QU_RAM_CFG_RED_ECN_MISC_CFG, instance, &red_misc);
                maxp = PST_SQS(VTSS_X, output, QU_RAM_CFG_RED_ECN_MISC_CFG_RED_ECN_PROF_MAXP(red_misc));
                RDX_SQS(output, RED_RAM_RED_ECN_WQ_CFG, instance, &red_wq);

                RDX_SQS(output, QU_RAM_CFG_QU_RC_CFG, instance, &rc_cfg);
                RDX_SQS(output, QU_RAM_CFG_QU_RC_HLWM_CFG, instance, &hlwm);
                RDX_SQS(output, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG, instance, &atop_swm);
                RDX_SQS(output, QU_RAM_CFG_MTU_QU_MAP_CFG, instance, &map_cfg);
                mtu_ix = PST_SQS(VTSS_X, output, QU_RAM_CFG_MTU_QU_MAP_CFG_MTU_PRE_ALLOC(map_cfg));

                pr("%1d  %c  %1d %1d %3d %3d %3d %3d %3d %2d %1d %5d %5d %5d %1d:%2d\n     ",
                   queue,
                   output ? 'O' : 'I',
                   PST_SQS(VTSS_F, output, QU_RAM_CFG_RED_ECN_MISC_CFG_RED_ENA) & red_misc ? 1 : 0,
                   PST_SQS(VTSS_X, output, QU_RAM_CFG_RED_ECN_MISC_CFG_RED_THRES_GAIN(red_misc)),
                   PST_SQS(VTSS_X, output, QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG_RED_ECN_PROFILE_MINTH(red_min_max)),
                   PST_SQS(VTSS_X, output, QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG_RED_ECN_PROFILE_MINMAXTH(red_min_max)),
                   VTSS_EXTRACT_BITFIELD(maxp,  0, 8),
                   VTSS_EXTRACT_BITFIELD(maxp,  8, 8),
                   VTSS_EXTRACT_BITFIELD(maxp,  16, 8),
                   PST_SQS(VTSS_X, output, RED_RAM_RED_ECN_WQ_CFG_RED_ECN_PROFILE_WQ(red_wq)),
                   PST_SQS(VTSS_F, output, QU_RAM_CFG_QU_RC_CFG_QU_FC_DROP_MODE) & rc_cfg ? 1 : 0,
                   PST_SQS(VTSS_X, output, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_SWM(atop_swm)),
                   PST_SQS(VTSS_X, output, QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_HWM(hlwm)),
                   PST_SQS(VTSS_X, output, QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_ATOP(atop_swm)),
                   mtu_ix,
                   output ? oqs_mtu[mtu_ix] : iqs_mtu[mtu_ix]);
            }
        }
        pr("\r");
    }
    pr("\n");

    VTSS_RC(vtss_jr1_debug_range_checkers(vtss_state, pr, info));
    VTSS_RC(vtss_jr1_debug_vcap_is1(vtss_state, pr, info));

    vtss_debug_print_header(pr, "QoS Port Policers");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        int policer;
        u32 pol_idx;
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        sprintf(buf, "Port %u (%u)", port, port_no);
        vtss_jr1_debug_reg_header(pr, buf);
        for (policer = 0; policer < 4; policer++) {
            pol_idx = (port * 4 + policer);
            JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_PORT_CFG_POL_PORT_THRES_CFG_0, pol_idx, "THRES_CFG_0");
            JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_PORT_CFG_POL_PORT_THRES_CFG_1, pol_idx, "THRES_CFG_1");
            JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_PORT_CFG_POL_PORT_RATE_CFG,    pol_idx, "RATE_CFG");
            JR_DEBUG_REGXY_NAME(pr, ANA_AC, POL_PORT_CTRL_POL_PORT_CFG, port, policer, "POL_PORT_CFG");
        }
        JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_PORT_CTRL_POL_PORT_GAP,       port, "GAP");
        JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_ALL_CFG_POL_PORT_FC_CFG,      port, "POL_PORT_FC_CFG");
    }
    pr("\n");

    vtss_debug_print_header(pr, "QoS Queue Policers");
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        int queue;
        u32 pol_idx;
        if (vtss_jr1_debug_port_skip(vtss_state, port_no, info))
            continue;
        port = VTSS_CHIP_PORT(port_no);
        sprintf(buf, "Port %u (%u)", port, port_no);
        vtss_jr1_debug_reg_header(pr, buf);
        for (queue = 0; queue < 8; queue++) {
            pol_idx = (port * 8 + queue);
            JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_PRIO_CFG_POL_PRIO_THRES_CFG, pol_idx, "THRES_CFG");
            JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_PRIO_CFG_POL_PRIO_RATE_CFG,  pol_idx, "RATE_CFG");
            JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_PRIO_CTRL_POL_PRIO_GAP,      pol_idx, "PRIO_GAP");
            JR_DEBUG_REGX_NAME(pr, ANA_AC, POL_PRIO_CTRL_POL_PRIO_CFG,      pol_idx, "PRIO_CFG");
        }
    }
    pr("\n");

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_qos_debug_print(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_QOS, jr_debug_qos, vtss_state, pr, info);
}

/* - Initialization ------------------------------------------------ */

vtss_rc vtss_jr1_qos_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_qos_state_t   *state = &vtss_state->qos;
    vtss_policer_t     pol_conf;
    vtss_policer_ext_t pol_ext_conf;
    u32                port, i;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->conf_set = jr_qos_conf_set;
        state->port_conf_set = vtss_cmn_qos_port_conf_set;
        state->port_conf_update = vtss_jr1_qos_port_conf_set;
        state->qce_add = vtss_cmn_qce_add;
        state->qce_del = vtss_cmn_qce_del;
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
        state->lport_conf_set = jr_qos_lport_conf_set;
#endif
        state->prio_count = JR_PRIOS;
        break;
    case VTSS_INIT_CMD_INIT:
        /* Disable policers */
        memset(&pol_conf, 0, sizeof(pol_conf));
        memset(&pol_ext_conf, 0, sizeof(pol_ext_conf));
        pol_conf.rate = VTSS_BITRATE_DISABLED;
        for (port = 0; port <= VTSS_CHIP_PORTS; port++) {
            for (i = 0; i < 4; i++) {
                VTSS_RC(jr_port_policer_set(vtss_state, port, i, &pol_conf, &pol_ext_conf));
            }
            for (i = 0; i < 8; i++) {
                VTSS_RC(jr_queue_policer_set(vtss_state, port, i, &pol_conf));
            }
        }
        break;
    default:
        break;
    }
    
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_JAGUAR_1 */
