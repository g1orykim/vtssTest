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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_OAM
#include "vtss_serval_cil.h"

#if defined(VTSS_ARCH_SERVAL)

#if defined(VTSS_FEATURE_OAM)

/* - CIL functions ------------------------------------------------- */

/* ================================================================= *
 *  Y.1731/802.1ag OAM
 * ================================================================= */
static vtss_rc srvl_oam_voe_counter_update_internal(vtss_state_t *vtss_state,
                                                    const vtss_oam_voe_idx_t voe_idx,
                                                    const u32 clear_mask);

/* Boolean-to-bits */
#define B2B(boolean, bits)        ((boolean) ? (bits) : 0)

static vtss_rc srvl_oam_vop_int_enable(vtss_state_t *vtss_state, u32 enable)
{
    SRVL_WRM(VTSS_OAM_MEP_COMMON_MASTER_INTR_CTRL, B2B(enable, VTSS_F_OAM_MEP_COMMON_MASTER_INTR_CTRL_OAM_MEP_INTR_ENA), VTSS_F_OAM_MEP_COMMON_MASTER_INTR_CTRL_OAM_MEP_INTR_ENA);
    return VTSS_RC_OK;
}

/* Determine VOP interrupt flag -- clear if no enabled VOEs have
 * interrupts enabled */
static vtss_rc srvl_oam_vop_int_update(vtss_state_t *vtss_state)
{
    u32 i;
    u32 must_enable;
    for (i=0, must_enable=0; !must_enable && i<VTSS_OAM_VOE_CNT; ++i) {
        if (vtss_state->oam.voe_internal[i].allocated) {
            SRVL_RD(VTSS_OAM_MEP_VOE_INTR_ENA(i), &must_enable);
            must_enable &= 0xff;  /* Only bits 0-7 are valid on Serval */
        }
    }
    return srvl_oam_vop_int_enable(vtss_state, must_enable);
}

/* CCM-LM: the HW period "runs at double speed", so we need to dedicate two
 * slots int the CCM period config register for double values and use those
 * slots when CCM-LM is configured.
 *
 * However, from an API standpoint the user doesn't need to know this, so we
 * use the standard enums from #vtss_oam_period_t in the API and convert them
 * to the HW-near values here.
 *
 * This, of course, invalidates the semantics of the enum, so we return u32
 * from this func. Also note that only the 100ms and 1-sec values are supported
 * by the API.
 */
static u32 srvl_oam_ccm_lm_period_index(vtss_oam_period_t p)
{
    switch (p) {
    case VTSS_OAM_PERIOD_100_MS:
        return VTSS_OAM_PERIOD_1_SEC + 1;
    case VTSS_OAM_PERIOD_1_SEC:
        return VTSS_OAM_PERIOD_1_SEC + 2;
    default:
        return 0;
    }
}

static vtss_rc doing_calculate(vtss_state_t *vtss_state,
                               u32 voe_idx,  BOOL *doing_lb,  BOOL *doing_tst)
{
    u32 v;
    BOOL lb_hw_enabled, tst_hw_enabled;

    SRVL_RD(VTSS_OAM_MEP_VOE_OAM_HW_CTRL(voe_idx), &v);
    lb_hw_enabled = (v & (VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_LBM_ENA | VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_LBR_ENA)) != 0;
    SRVL_RD(VTSS_OAM_MEP_VOE_OAM_FWD_CTRL(voe_idx), &v);
    tst_hw_enabled = (VTSS_X_OAM_MEP_VOE_OAM_FWD_CTRL_GENERIC_FWD_MASK(v) & (1<<6)) != 0;

    *doing_lb  = lb_hw_enabled && !tst_hw_enabled;
    *doing_tst = lb_hw_enabled &&  tst_hw_enabled;

    return(VTSS_RC_OK);
}

static vtss_rc srvl_oam_vop_cfg_set(vtss_state_t *vtss_state)
{
    u32 i, mask, external_cpu_portmask;
    u32 v;

    const vtss_oam_vop_conf_t *cfg = &vtss_state->oam.vop_conf;

    SRVL_WR(VTSS_OAM_MEP_COMMON_CCM_PERIOD_CFG(VTSS_OAM_PERIOD_3_3_MS-1),   16650);
    SRVL_WR(VTSS_OAM_MEP_COMMON_CCM_PERIOD_CFG(VTSS_OAM_PERIOD_10_MS-1),    50454);
    SRVL_WR(VTSS_OAM_MEP_COMMON_CCM_PERIOD_CFG(VTSS_OAM_PERIOD_100_MS-1),  504541);
    SRVL_WR(VTSS_OAM_MEP_COMMON_CCM_PERIOD_CFG(VTSS_OAM_PERIOD_1_SEC-1),  5045409);
    SRVL_WR(VTSS_OAM_MEP_COMMON_CCM_PERIOD_CFG(srvl_oam_ccm_lm_period_index(VTSS_OAM_PERIOD_100_MS)-1),  2*504541);
    SRVL_WR(VTSS_OAM_MEP_COMMON_CCM_PERIOD_CFG(srvl_oam_ccm_lm_period_index(VTSS_OAM_PERIOD_1_SEC)-1),  2*5045409);

    v = B2B(cfg->pdu_type.other.to_front,  VTSS_F_OAM_MEP_COMMON_CPU_CFG_DEF_EXT_PORT_ENA)     | VTSS_F_OAM_MEP_COMMON_CPU_CFG_DEF_COPY_QU(cfg->pdu_type.other.rx_queue) |
        B2B(cfg->pdu_type.err.to_front,    VTSS_F_OAM_MEP_COMMON_CPU_CFG_CPU_ERR_EXT_PORT_ENA) | VTSS_F_OAM_MEP_COMMON_CPU_CFG_CPU_ERR_QU(cfg->pdu_type.err.rx_queue)    |
        B2B(cfg->pdu_type.dmr.to_front,    VTSS_F_OAM_MEP_COMMON_CPU_CFG_DMR_EXT_PORT_ENA)     | VTSS_F_OAM_MEP_COMMON_CPU_CFG_DMR_CPU_QU(cfg->pdu_type.dmr.rx_queue)    |
        B2B(cfg->pdu_type.lmr.to_front,    VTSS_F_OAM_MEP_COMMON_CPU_CFG_LMR_EXT_PORT_ENA)     | VTSS_F_OAM_MEP_COMMON_CPU_CFG_LMR_CPU_QU(cfg->pdu_type.lmr.rx_queue)    |
        B2B(cfg->pdu_type.lbr.to_front,    VTSS_F_OAM_MEP_COMMON_CPU_CFG_LBR_EXT_PORT_ENA)     | VTSS_F_OAM_MEP_COMMON_CPU_CFG_LBR_CPU_QU(cfg->pdu_type.lbr.rx_queue)    |
        B2B(cfg->pdu_type.ccm_lm.to_front, VTSS_F_OAM_MEP_COMMON_CPU_CFG_CCM_LM_EXT_PORT_ENA)  | VTSS_F_OAM_MEP_COMMON_CPU_CFG_CCM_LM_CPU_QU(cfg->pdu_type.ccm_lm.rx_queue);
    SRVL_WR(VTSS_OAM_MEP_COMMON_CPU_CFG, v);

    v = B2B(cfg->pdu_type.lt.to_front,     VTSS_F_OAM_MEP_COMMON_CPU_CFG_1_LT_EXT_PORT_ENA)    | VTSS_F_OAM_MEP_COMMON_CPU_CFG_1_LT_CPU_QU(cfg->pdu_type.lt.rx_queue)    |
        B2B(cfg->pdu_type.dmm.to_front,    VTSS_F_OAM_MEP_COMMON_CPU_CFG_1_DMM_EXT_PORT_ENA)   | VTSS_F_OAM_MEP_COMMON_CPU_CFG_1_DMM_CPU_QU(cfg->pdu_type.dmm.rx_queue)  |
        B2B(cfg->pdu_type.lmm.to_front,    VTSS_F_OAM_MEP_COMMON_CPU_CFG_1_LMM_EXT_PORT_ENA)   | VTSS_F_OAM_MEP_COMMON_CPU_CFG_1_LMM_CPU_QU(cfg->pdu_type.lmm.rx_queue)  |
        B2B(cfg->pdu_type.lbm.to_front,    VTSS_F_OAM_MEP_COMMON_CPU_CFG_1_LBM_EXT_PORT_ENA)   | VTSS_F_OAM_MEP_COMMON_CPU_CFG_1_LBM_CPU_QU(cfg->pdu_type.lbm.rx_queue)  |
        B2B(cfg->pdu_type.ccm.to_front,    VTSS_F_OAM_MEP_COMMON_CPU_CFG_1_CCM_EXT_PORT_ENA)   | VTSS_F_OAM_MEP_COMMON_CPU_CFG_1_CCM_CPU_QU(cfg->pdu_type.ccm.rx_queue);
    SRVL_WR(VTSS_OAM_MEP_COMMON_CPU_CFG_1, v);

    for (i=0; i<VTSS_OAM_GENERIC_OPCODE_CFG_CNT; ++i) {
        const vtss_oam_vop_generic_opcode_conf_t *g = &(cfg->pdu_type.generic[i]);
        v = B2B(g->extract.to_front, VTSS_F_OAM_MEP_COMMON_OAM_GENERIC_CFG_GENERIC_OPCODE_EXT_PORT_ENA) |
            VTSS_F_OAM_MEP_COMMON_OAM_GENERIC_CFG_GENERIC_OPCODE_CPU_QU(g->extract.rx_queue) |
            VTSS_F_OAM_MEP_COMMON_OAM_GENERIC_CFG_GENERIC_OPCODE_VAL(g->opcode) |
            B2B(!g->check_dmac, VTSS_F_OAM_MEP_COMMON_OAM_GENERIC_CFG_GENERIC_DMAC_CHK_DIS);
        SRVL_WR(VTSS_OAM_MEP_COMMON_OAM_GENERIC_CFG(i), v);
    }

    v = (cfg->common_multicast_dmac.addr[0] << 8) | cfg->common_multicast_dmac.addr[1];
    SRVL_WR(VTSS_OAM_MEP_COMMON_COMMON_MEP_MC_MAC_MSB, VTSS_F_OAM_MEP_COMMON_COMMON_MEP_MC_MAC_MSB_MEP_MC_MAC_MSB(v));

    v = (cfg->common_multicast_dmac.addr[2] << 24) |
        (cfg->common_multicast_dmac.addr[3] << 16) |
        (cfg->common_multicast_dmac.addr[4] <<  8) |
        (cfg->common_multicast_dmac.addr[5]);
    v >>= 4;   /* Value in reg. doesn't include the least significant nibble */
    SRVL_WR(VTSS_OAM_MEP_COMMON_COMMON_MEP_MC_MAC_LSB, VTSS_F_OAM_MEP_COMMON_COMMON_MEP_MC_MAC_LSB_MEP_MC_MAC_LSB(v));

    for (i=0, external_cpu_portmask=0, mask=0x01; i<VTSS_PORT_ARRAY_SIZE; ++i, mask<<=1) { /* Convert the logical port mask to a chip port mask */
        if ((cfg->external_cpu_portmask & mask) != 0)
            external_cpu_portmask |= 1<<VTSS_CHIP_PORT(i);
    }

    v = B2B(cfg->ccm_lm_enable_rx_fcf_in_reserved_field, VTSS_F_OAM_MEP_COMMON_MEP_CTRL_CCM_LM_UPD_RSV_ENA) |
        B2B(cfg->down_mep_lmr_proprietary_fcf_use,       VTSS_F_OAM_MEP_COMMON_MEP_CTRL_LMR_UPD_RxFcL_ENA) |
        B2B(cfg->enable_all_voe,                         VTSS_F_OAM_MEP_COMMON_MEP_CTRL_MEP_ENA) |
        VTSS_F_OAM_MEP_COMMON_MEP_CTRL_CCM_SCAN_ENA |
        VTSS_F_OAM_MEP_COMMON_MEP_CTRL_EXT_CPU_PORTMASK(external_cpu_portmask);
    SRVL_WR(VTSS_OAM_MEP_COMMON_MEP_CTRL, v);

    SRVL_WR(VTSS_OAM_MEP_COMMON_OAM_SDLB_CPU_COPY, cfg->sdlb_cpy_copy_idx);

    /* We don't touch the interrupt enable bit here; it defaults to disabled,
     * so we leave it to VOE interrupt configuration to modify it.
     */

    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_voe_alloc(vtss_state_t *vtss_state,
                                  const vtss_oam_voe_type_t      type,
                                  const vtss_oam_voe_alloc_cfg_t *data,
                                  vtss_oam_voe_idx_t             *voe_idx)
{
    u32 i;
    if (type == VTSS_OAM_VOE_SERVICE) {
        for (i=0; i<VTSS_OAM_PATH_SERVICE_VOE_CNT  &&  vtss_state->oam.voe_internal[i].allocated; ++i)
            /* search for free VOE instance */ ;
        if (i == VTSS_OAM_PATH_SERVICE_VOE_CNT)
            return VTSS_RC_ERROR;
    }
    else {
        i = VTSS_CHIP_PORT(data->phys_port) + VTSS_OAM_PORT_VOE_BASE_IDX;
        if (i >= VTSS_OAM_VOE_CNT  ||  vtss_state->oam.voe_internal[i].allocated)
            return VTSS_RC_ERROR;
    }

    *voe_idx = i;
    vtss_state->oam.voe_internal[i].allocated = TRUE;
    SRVL_WR(VTSS_OAM_MEP_VOE_BASIC_CTRL(i), 0);   /* Make sure the VOE isn't physically enabled yet */

    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_voe_free(vtss_state_t *vtss_state, const vtss_oam_voe_idx_t voe_idx)
{
    if (vtss_state->oam.voe_internal[voe_idx].allocated) {
        /* Disable VOE, and we're done */
        // FIXME: Correct? HenrikB's has an idea about copy-to-CPU?
        vtss_state->oam.voe_internal[voe_idx].allocated = FALSE;
        SRVL_WRM(VTSS_OAM_MEP_VOE_BASIC_CTRL(voe_idx), 0, VTSS_F_OAM_MEP_VOE_BASIC_CTRL_VOE_ENA);
        return srvl_oam_vop_int_update(vtss_state);
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_event_poll(vtss_state_t *vtss_state, vtss_oam_event_mask_t *const mask)
{
    u32 i;
    for (i=0; i<VTSS_OAM_EVENT_MASK_ARRAY_SIZE; ++i)
        SRVL_RD(VTSS_OAM_MEP_COMMON_INTR(i), &(mask->voe_mask[i]));
    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_voe_event_enable(vtss_state_t *vtss_state,
                                         const vtss_oam_voe_idx_t        voe_idx,
                                         const vtss_oam_voe_event_mask_t mask,
                                         const BOOL                      enable)
{
    u32 m;
    SRVL_RD(VTSS_OAM_MEP_VOE_INTR_ENA(voe_idx), &m);
    m = enable ? (m | mask) : (m & ~mask);
    SRVL_WR(VTSS_OAM_MEP_VOE_INTR_ENA(voe_idx), m);
    return m ? srvl_oam_vop_int_enable(vtss_state, TRUE) : srvl_oam_vop_int_update(vtss_state);
}

static vtss_rc srvl_oam_voe_event_poll(vtss_state_t *vtss_state,
                                       const vtss_oam_voe_idx_t  voe_idx,
                                       vtss_oam_voe_event_mask_t *const mask)
{
    u32 intmask;
    SRVL_RD(VTSS_OAM_MEP_VOE_INTR_ENA(voe_idx), &intmask);
    SRVL_RD(VTSS_OAM_MEP_VOE_STICKY(voe_idx), mask);
    *mask &= intmask;
    SRVL_WR(VTSS_OAM_MEP_VOE_STICKY(voe_idx), *mask);
    return VTSS_RC_OK;
}

/* Forward decl; the implementation really doesn't belong here. */
static vtss_rc srvl_oam_voe_counter_clear(vtss_state_t *vtss_state,
                                          const vtss_oam_voe_idx_t voe_idx, const u32 mask);

static vtss_rc srvl_oam_voe_cfg_set(vtss_state_t *vtss_state, const vtss_oam_voe_idx_t voe_idx)
{
    u32  i, v;
    u32  mask;
    BOOL doing_lb;
    BOOL doing_tst;
    BOOL doing_anything;
    const vtss_oam_voe_conf_t *cfg = &(vtss_state->oam.voe_conf[voe_idx]);
    const u8 *p;
    vtss_oam_voe_internal_counters_t *chipcnt = &vtss_state->oam.voe_internal[voe_idx].counters;

    if (cfg->lb.enable && cfg->tst.enable) {
        VTSS_D("VOE %d: Cannot enable simultaneous LB and TST on Serval", voe_idx);
        return VTSS_RC_ERROR;
    }

    /* If the VOE isn't enabled yet, i.e. it's only been allocated and this
     * is the first call to this  function, then we clear the logical
     * counters.
     */
    SRVL_RD(VTSS_OAM_MEP_VOE_BASIC_CTRL(voe_idx), &v);
    if ((v & VTSS_F_OAM_MEP_VOE_BASIC_CTRL_VOE_ENA) == 0)
        VTSS_RC(srvl_oam_voe_counter_clear(vtss_state, voe_idx, VTSS_OAM_CNT_ALL));

    /* Then, if we're enabling LB (and it wasn't enabled previously), we discard
     * any changes to the LB HW counters. Same thing for TST.
     *
     * This is necessary since the two functions share registers on Serval-1.
     */
    VTSS_RC(doing_calculate(vtss_state, voe_idx, &doing_lb, &doing_tst));

    doing_anything = doing_lb || doing_tst;

    if ((!cfg->tst.enable && doing_tst) || (!cfg->lb.enable && doing_lb))  /* TST or LB is being disabled - update the counters */
        (void)srvl_oam_voe_counter_update_internal(vtss_state, voe_idx, 0);
    if (cfg->tst.enable && (doing_lb || !doing_anything)) {        /* Idle-to-TST or change from LB to TST - rebase TST counters */
        SRVL_RD(VTSS_OAM_MEP_VOE_LBR_RX_FRM_CNT(voe_idx), &v);
        vtss_cmn_counter_32_rebase(v, &chipcnt->tst.rx_tst);
        SRVL_RD(VTSS_OAM_MEP_VOE_LBM_TX_TRANSID_CFG(voe_idx), &v);
        vtss_cmn_counter_32_rebase(v, &chipcnt->tst.tx_tst);
    }
    if (cfg->lb.enable && (doing_tst || !doing_anything)) {        /* Idle-to-LB or change from TST to LB - rebase LB counters */
        SRVL_RD(VTSS_OAM_MEP_VOE_LBR_RX_FRM_CNT(voe_idx), &v);
        vtss_cmn_counter_32_rebase(v, &chipcnt->lb.rx_lbr);
        SRVL_RD(VTSS_OAM_MEP_VOE_LBM_TX_TRANSID_CFG(voe_idx), &v);
        vtss_cmn_counter_32_rebase(v, &chipcnt->lb.tx_lbm);
    }

    /* MEL_CTRL */
    v = 0;
    if (!cfg->proc.copy_next_only)
        v |= B2B(cfg->proc.copy_on_mel_too_low_err, VTSS_F_OAM_MEP_VOE_MEL_CTRL_CPU_COPY_MEL_TOO_LOW | VTSS_F_OAM_MEP_VOE_MEL_CTRL_CPU_COPY_CCM_MEL_TOO_LOW);
    v |= VTSS_F_OAM_MEP_VOE_MEL_CTRL_MEL_VAL(cfg->proc.meg_level);
    if (cfg->mep_type == VTSS_OAM_UPMEP) {
        v |= B2B(cfg->upmep.loopback, VTSS_F_OAM_MEP_VOE_MEL_CTRL_UPMEP_LB);
        v |= VTSS_F_OAM_MEP_VOE_MEL_CTRL_MEP_PORTMASK(1 << VTSS_CHIP_PORT(cfg->upmep.port));
    }
    SRVL_WR(VTSS_OAM_MEP_VOE_MEL_CTRL(voe_idx), v);  /* Debug bit 4 is always set to zero; don't expect it to be used */

    /* OAM_CPU_COPY_CTRL */
    v = 0;
    for (i=0, mask=0x01; i<VTSS_OAM_GENERIC_OPCODE_CFG_CNT; ++i, mask<<=1)
        v |= B2B(cfg->generic[i].enable  && cfg->generic[i].copy_to_cpu,          VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_GENERIC_COPY_MASK(mask));
    v |= B2B(cfg->unknown.enable         && cfg->unknown.copy_to_cpu,             VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_UNK_OPCODE_CPU_COPY_ENA);
    v |= B2B(cfg->single_ended_lm.enable && cfg->single_ended_lm.copy_lmm_to_cpu, VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_LMM_CPU_COPY_ENA);
    v |= B2B(cfg->single_ended_lm.enable && cfg->single_ended_lm.copy_lmr_to_cpu, VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_LMR_CPU_COPY_ENA);
    v |= B2B(cfg->tst.enable             && cfg->tst.copy_to_cpu,                 VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_LBR_CPU_COPY_ENA);  /* TST RX overloads LBR on Serval 1 */
    v |= B2B(cfg->lb.enable              && cfg->lb.copy_lbm_to_cpu,              VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_LBM_CPU_COPY_ENA);
    v |= B2B(cfg->lb.enable              && cfg->lb.copy_lbr_to_cpu,              VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_LBR_CPU_COPY_ENA);
    v |= B2B(cfg->lt.enable              && cfg->lt.copy_ltm_to_cpu,              VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_LTM_CPU_COPY_ENA);
    v |= B2B(cfg->lt.enable              && cfg->lt.copy_ltr_to_cpu,              VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_LTR_CPU_COPY_ENA);
    v |= B2B(cfg->dm.enable_dmm          && cfg->dm.copy_dmm_to_cpu,              VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_DMM_CPU_COPY_ENA);
    v |= B2B(cfg->dm.enable_dmm          && cfg->dm.copy_dmr_to_cpu,              VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_DMR_CPU_COPY_ENA);
    v |= B2B(cfg->dm.enable_1dm          && cfg->dm.copy_1dm_to_cpu,              VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_SM_CPU_COPY_ENA);
    v |= B2B(cfg->ccm_lm.enable          && cfg->ccm_lm.copy_to_cpu,              VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_CCM_LM_CPU_COPY_ENA);
    if (!cfg->proc.copy_next_only) {
        v |= B2B(                           cfg->ccm.copy_to_cpu,                 VTSS_F_OAM_MEP_VOE_OAM_CPU_COPY_CTRL_CCM_CPU_COPY_ENA);
    }
    SRVL_WR(VTSS_OAM_MEP_VOE_OAM_CPU_COPY_CTRL(voe_idx), v);

    /* OAM_FWD_CTRL */
    v = 0;
    for (i=0, mask=0x01; i<VTSS_OAM_GENERIC_OPCODE_CFG_CNT; ++i, mask<<=1)
        v |= B2B(cfg->generic[i].enable  && cfg->generic[i].forward,          VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_GENERIC_FWD_MASK(mask));
    v |= B2B(                               cfg->ccm.forward,                 VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_CCM_FWD_ENA);
    v |= B2B(cfg->ccm_lm.enable          && cfg->ccm_lm.forward,              VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_CCM_LM_FWD_ENA);
    v |= B2B(cfg->single_ended_lm.enable && cfg->single_ended_lm.forward_lmm, VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_LMM_FWD_ENA);
    v |= B2B(cfg->single_ended_lm.enable && cfg->single_ended_lm.forward_lmr, VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_LMR_FWD_ENA);
    v |= B2B(cfg->lb.enable              && cfg->lb.forward_lbm,              VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_LBM_FWD_ENA);
    v |= B2B(cfg->lb.enable              && cfg->lb.forward_lbr,              VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_LBR_FWD_ENA);
    v |= B2B(cfg->lt.enable              && cfg->lt.forward_ltm,              VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_LTM_FWD_ENA);
    v |= B2B(cfg->lt.enable              && cfg->lt.forward_ltr,              VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_LTR_FWD_ENA);
    v |= B2B(cfg->dm.enable_dmm          && cfg->dm.forward_dmm,              VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_DMM_FWD_ENA);
    v |= B2B(cfg->dm.enable_dmm          && cfg->dm.forward_dmr,              VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_DMR_FWD_ENA);
    v |= B2B(cfg->dm.enable_1dm          && cfg->dm.forward_1dm,              VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_SM_FWD_ENA);
    v |= B2B(                               cfg->tst.enable,                  VTSS_F_OAM_MEP_VOE_OAM_FWD_CTRL_GENERIC_FWD_MASK(1<<6));
    SRVL_WR(VTSS_OAM_MEP_VOE_OAM_FWD_CTRL(voe_idx), v);

    /* TST setup. Overloaded LB register semantics on Serval 1. */
    if (cfg->tst.enable) {
        if (doing_tst) {
            /* Don't change anything if we're already running TST frames; it would mess up sequence numbering. */
        }
        else {
            SRVL_WRM(VTSS_OAM_MEP_VOE_LBM_TX_TRANSID_UPDATE(voe_idx),
                    B2B(cfg->tst.tx_seq_no_auto_update, VTSS_F_OAM_MEP_VOE_LBM_TX_TRANSID_UPDATE_LBM_TRANSID_UPDATE_ENA),
                    VTSS_F_OAM_MEP_VOE_LBM_TX_TRANSID_UPDATE_LBM_TRANSID_UPDATE_ENA);
            SRVL_WR(VTSS_OAM_MEP_VOE_LBM_TX_TRANSID_CFG(voe_idx), cfg->tst.tx_seq_no);
            SRVL_WR(VTSS_OAM_MEP_VOE_LBR_RX_TRANSID_CFG(voe_idx), cfg->tst.rx_seq_no);
            vtss_cmn_counter_32_rebase(cfg->tst.tx_seq_no, &chipcnt->tst.tx_tst);    /* Always rebase when transaction id is changed */
        }
    }
    else {
        if (cfg->lb.enable) {
            if (doing_lb) {
                /* Don't change anything if we're already running LB frames; it would mess up sequence numbering. */
            }
            else {
                SRVL_WRM(VTSS_OAM_MEP_VOE_LBM_TX_TRANSID_UPDATE(voe_idx), B2B(cfg->lb.tx_update_transaction_id, VTSS_F_OAM_MEP_VOE_LBM_TX_TRANSID_UPDATE_LBM_TRANSID_UPDATE_ENA), VTSS_F_OAM_MEP_VOE_LBM_TX_TRANSID_UPDATE_LBM_TRANSID_UPDATE_ENA);
                SRVL_WR(VTSS_OAM_MEP_VOE_LBM_TX_TRANSID_CFG(voe_idx), cfg->lb.tx_transaction_id);
                SRVL_WR(VTSS_OAM_MEP_VOE_LBR_RX_TRANSID_CFG(voe_idx), cfg->lb.rx_transaction_id);
                vtss_cmn_counter_32_rebase(cfg->lb.tx_transaction_id, &chipcnt->lb.tx_lbm);    /* Always rebase when transaction id is changed */
            }
        }
        else
            SRVL_WRM(VTSS_OAM_MEP_VOE_LBM_TX_TRANSID_UPDATE(voe_idx), 0, VTSS_F_OAM_MEP_VOE_LBM_TX_TRANSID_UPDATE_LBM_TRANSID_UPDATE_ENA);
    }

    /* OAM_SDLB_CTRL */
    SRVL_WR(VTSS_OAM_MEP_VOE_OAM_SDLB_CTRL(voe_idx),
        B2B(cfg->sdlb_enable, VTSS_F_OAM_MEP_VOE_OAM_SDLB_CTRL_OAM_SDLB_ENA) |
        VTSS_F_OAM_MEP_VOE_OAM_SDLB_CTRL_OAM_SDLB_IDX(cfg->sdlb_idx));

    /* OAM_CNT_OAM_CTRL */
    v = 0;
    for (i=0, mask=0x01; i<VTSS_OAM_GENERIC_OPCODE_CFG_CNT; ++i, mask<<=1)
        v |= B2B(cfg->generic[i].enable  && cfg->generic[i].count_as_selected,            VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_GENERIC_OAM_CNT_MASK(mask));
    v |= B2B(cfg->unknown.enable         && cfg->unknown.count_as_selected,               VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_UNK_OPCODE_OAM_CNT_ENA);
    v |= B2B(                               cfg->ccm.count_as_selected,                   VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_CCM_OAM_CNT_ENA);
    v |= B2B(cfg->ccm_lm.enable          && cfg->ccm_lm.count_as_selected,                VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_CCM_LM_OAM_CNT_ENA);
    v |= B2B(cfg->single_ended_lm.enable && cfg->single_ended_lm.count_as_selected,       VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_LMM_OAM_CNT_ENA | VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_LMR_OAM_CNT_ENA);
    v |= B2B(cfg->lb.enable              && cfg->lb.count_as_selected,                    VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_LBM_OAM_CNT_ENA | VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_LBR_OAM_CNT_ENA);
    v |= B2B(cfg->tst.enable             && cfg->tst.count_as_selected,                   VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_LBM_OAM_CNT_ENA | VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_LBR_OAM_CNT_ENA);
    v |= B2B(cfg->lt.enable              && cfg->lt.count_as_selected,                    VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_LTM_OAM_CNT_ENA | VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_LTR_OAM_CNT_ENA);
    v |= B2B(cfg->dm.enable_dmm          && cfg->dm.count_as_selected,                    VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_DMM_OAM_CNT_ENA | VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_DMR_OAM_CNT_ENA);
    v |= B2B(cfg->dm.enable_1dm          && cfg->dm.count_as_selected,                    VTSS_F_OAM_MEP_VOE_OAM_CNT_OAM_CTRL_SM_OAM_CNT_ENA);
    SRVL_WR(VTSS_OAM_MEP_VOE_OAM_CNT_OAM_CTRL(voe_idx), v);

    /* OAM_CNT_DATA_CTRL */
    v = 0;
    for (i=0, mask=0x01; i<VTSS_OAM_GENERIC_OPCODE_CFG_CNT; ++i, mask<<=1)
        v |= B2B(cfg->generic[i].enable  && cfg->generic[i].count_as_data,            VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_GENERIC_DATA_CNT_MASK(mask));
    v |= B2B(cfg->unknown.enable         && cfg->unknown.count_as_data,               VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_UNK_OPCODE_DATA_CNT_ENA);
    v |= B2B(                               cfg->ccm.count_as_data,                   VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_CCM_DATA_CNT_ENA);
    v |= B2B(cfg->single_ended_lm.enable && cfg->single_ended_lm.count_as_data,       VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_LMM_DATA_CNT_ENA | VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_LMR_DATA_CNT_ENA);
    v |= B2B(cfg->lb.enable              && cfg->lb.count_as_data,                    VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_LBM_DATA_CNT_ENA | VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_LBR_DATA_CNT_ENA);
    v |= B2B(cfg->tst.enable             && cfg->tst.count_as_data,                   VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_LBM_DATA_CNT_ENA | VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_LBR_DATA_CNT_ENA);
    v |= B2B(cfg->lt.enable              && cfg->lt.count_as_data,                    VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_LTM_DATA_CNT_ENA | VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_LTR_DATA_CNT_ENA);
    v |= B2B(cfg->dm.enable_dmm          && cfg->dm.count_as_data,                    VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_DMM_DATA_CNT_ENA | VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_DMR_DATA_CNT_ENA);
    v |= B2B(cfg->dm.enable_1dm          && cfg->dm.count_as_data,                    VTSS_F_OAM_MEP_VOE_OAM_CNT_DATA_CTRL_SM_DATA_CNT_ENA);
    SRVL_WR(VTSS_OAM_MEP_VOE_OAM_CNT_DATA_CTRL(voe_idx), v);

    /* MEP_UC_MAC_LSB / MEP_UC_MAC_MSB */
    v = (cfg->unicast_mac.addr[0] << 8) | cfg->unicast_mac.addr[1];
    SRVL_WR(VTSS_OAM_MEP_VOE_MEP_UC_MAC_MSB(voe_idx), v);

    v = (cfg->unicast_mac.addr[2] << 24) |
        (cfg->unicast_mac.addr[3] << 16) |
        (cfg->unicast_mac.addr[4] <<  8) |
        (cfg->unicast_mac.addr[5]);
    SRVL_WR(VTSS_OAM_MEP_VOE_MEP_UC_MAC_LSB(voe_idx), v);

    /* OAM_HW_CTRL */
    v = 0;
    v |= B2B(cfg->ccm.enable,                   VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_CCM_ENA);
    v |= B2B(cfg->ccm_lm.enable,                VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_CCM_LM_ENA);
    v |= B2B(cfg->single_ended_lm.enable,       VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_LMM_ENA | VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_LMR_ENA);
    v |= B2B(cfg->lb.enable || cfg->tst.enable, VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_LBM_ENA | VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_LBR_ENA);
    v |= B2B(cfg->dm.enable_dmm,                VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_DMM_ENA | VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_DMR_ENA);
    v |= B2B(cfg->dm.enable_1dm,                VTSS_F_OAM_MEP_VOE_OAM_HW_CTRL_SDM_ENA);
    SRVL_WR(VTSS_OAM_MEP_VOE_OAM_HW_CTRL(voe_idx), v);

    /* LOOPBACK_CTRL */
    v = 0;
    switch (cfg->mep_type) {
    case VTSS_OAM_UPMEP:
        v = 0;
        break;
    case VTSS_OAM_DOWNMEP:
        v |= VTSS_F_OAM_MEP_VOE_LOOPBACK_CTRL_LB_LMM_ENA | VTSS_F_OAM_MEP_VOE_LOOPBACK_CTRL_LB_DMM_ENA;
        v |= VTSS_F_OAM_MEP_VOE_LOOPBACK_CTRL_LB_ISDX(cfg->loop_isdx_w);
        v |= VTSS_F_OAM_MEP_VOE_LOOPBACK_CTRL_LB_ISDX_P(cfg->loop_isdx_p);
        v |= VTSS_F_OAM_MEP_VOE_LOOPBACK_CTRL_LB_PORTNUM_P(cfg->loop_portidx_p);
        break;
    case VTSS_OAM_MIP:
        break;
    }
    SRVL_WR(VTSS_OAM_MEP_VOE_LOOPBACK_CTRL(voe_idx), v);

    /* CCM_CFG */
    v = 0;
    v |= B2B(cfg->mep_type == VTSS_OAM_DOWNMEP, VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_LM_PERIOD(srvl_oam_ccm_lm_period_index(cfg->ccm_lm.period)));
    switch (cfg->ccm.tx_seq_no_auto_upd_op) {
    case VTSS_OAM_AUTOSEQ_DONT:
        break;
    case VTSS_OAM_AUTOSEQ_INCREMENT_AND_UPDATE:
        if (cfg->mep_type == VTSS_OAM_DOWNMEP)
            v |= VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_SEQ_INCR_ENA | VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_SEQ_UPD_ENA;
        break;
    case VTSS_OAM_AUTOSEQ_UPDATE:
        if (cfg->mep_type == VTSS_OAM_DOWNMEP)
            v |= VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_SEQ_UPD_ENA;
        break;
    }
    v |= B2B(cfg->ccm.rx_seq_no_check, VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_RX_SEQ_CHK_ENA);
    v |= VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_PRIO(cfg->ccm.rx_priority);
    v |= VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_PERIOD(cfg->ccm.rx_period);
    v |= VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_MEGID_CHK_ENA | VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_MEPID_CHK_ENA;
    mask = VTSS_M_OAM_MEP_VOE_CCM_CFG_CCM_LM_PERIOD |
           VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_SEQ_INCR_ENA |
           VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_SEQ_UPD_ENA |
           VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_SEQ_INCR_ENA |
           VTSS_M_OAM_MEP_VOE_CCM_CFG_CCM_PRIO |
           VTSS_M_OAM_MEP_VOE_CCM_CFG_CCM_PERIOD |
           VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_MEGID_CHK_ENA |
           VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_MEPID_CHK_ENA;
    SRVL_WRM(VTSS_OAM_MEP_VOE_CCM_CFG(voe_idx), v, mask);

    /* CCM_TX_SEQ_CFG */
    SRVL_WR(VTSS_OAM_MEP_VOE_CCM_TX_SEQ_CFG(voe_idx), cfg->ccm.tx_seq_no);

    /* CCM_RX_SEQ_CFG */
    SRVL_WR(VTSS_OAM_MEP_VOE_CCM_RX_SEQ_CFG(voe_idx), cfg->ccm.rx_seq_no);

    /* CCM_MEPID_CFG */
    SRVL_WR(VTSS_OAM_MEP_VOE_CCM_MEPID_CFG(voe_idx), VTSS_F_OAM_MEP_VOE_CCM_MEPID_CFG_CCM_MEPID(cfg->ccm.mepid));

    /* CCM_MEGID_CFG */
    p = &cfg->ccm.megid.data[47];    // MSB
    for (i=0; i<12; ++i, p-=4) {
        v = (*(p-3) << 24) |
            (*(p-2) << 16) |
            (*(p-1) <<  8) |
            (*(p-0));
        SRVL_WR(VTSS_OAM_MEP_VOE_CCM_MEGID_CFG(voe_idx, i), v);
    }

    if (cfg->voe_type == VTSS_OAM_VOE_SERVICE)
        SRVL_WR(VTSS_OAM_MEP_COMMON_VOE_CFG(voe_idx), B2B(cfg->mep_type == VTSS_OAM_UPMEP, VTSS_F_OAM_MEP_COMMON_VOE_CFG_UPMEP_VOE));

    /* VOE_CNT_CTRL */
    v = 0;
    if (cfg->single_ended_lm.enable) {
        v |= B2B(cfg->single_ended_lm.count.yellow, VTSS_F_OAM_MEP_COMMON_VOE_CNT_CTRL_CNT_YELLOW_ENA);
        v |= VTSS_F_OAM_MEP_COMMON_VOE_CNT_CTRL_PRIO_CNT_MASK(cfg->single_ended_lm.count.priority_mask);
    } else if (cfg->ccm_lm.enable) {
        v |= B2B(cfg->ccm_lm.count.yellow, VTSS_F_OAM_MEP_COMMON_VOE_CNT_CTRL_CNT_YELLOW_ENA);
        v |= VTSS_F_OAM_MEP_COMMON_VOE_CNT_CTRL_PRIO_CNT_MASK(cfg->ccm_lm.count.priority_mask);
    }
    SRVL_WR(VTSS_OAM_MEP_COMMON_VOE_CNT_CTRL(voe_idx), v);

    /* VOE_MAP_CTRL */
    if (cfg->voe_type == VTSS_OAM_VOE_SERVICE) {
        v = 0;
        if (cfg->svc_to_path) {
            v |= VTSS_F_OAM_MEP_COMMON_VOE_MAP_CTRL_PATH_VOEID_ENA;
            v |= VTSS_F_OAM_MEP_COMMON_VOE_MAP_CTRL_PATH_VOEID(cfg->svc_to_path_idx_w);
            v |= VTSS_F_OAM_MEP_COMMON_VOE_MAP_CTRL_PATH_VOEID_P(cfg->svc_to_path_idx_p);
        }
        SRVL_WR(VTSS_OAM_MEP_COMMON_VOE_MAP_CTRL(voe_idx), v);
    }

    /* BASIC_CTRL. Comes last, as this is where we enable the VOE */
    switch (cfg->proc.dmac_check_type) {
    case VTSS_OAM_DMAC_CHECK_NONE:
        v = 0;
        break;
    case VTSS_OAM_DMAC_CHECK_UNICAST:
        v = VTSS_F_OAM_MEP_VOE_BASIC_CTRL_MAC_ADDR_ERR_REDIR_ENA | VTSS_F_OAM_MEP_VOE_BASIC_CTRL_RX_DMAC_CHK_SEL(0x01);
        break;
    case VTSS_OAM_DMAC_CHECK_MULTICAST:
        v = VTSS_F_OAM_MEP_VOE_BASIC_CTRL_MAC_ADDR_ERR_REDIR_ENA | VTSS_F_OAM_MEP_VOE_BASIC_CTRL_RX_DMAC_CHK_SEL(0x02);
        break;
    case VTSS_OAM_DMAC_CHECK_BOTH:
        v = VTSS_F_OAM_MEP_VOE_BASIC_CTRL_MAC_ADDR_ERR_REDIR_ENA | VTSS_F_OAM_MEP_VOE_BASIC_CTRL_RX_DMAC_CHK_SEL(0x03);
        break;
    }

    /* Hit Me Once is configured by calling @vtss_oam_voe_ccm_arm_hitme */

    if (!cfg->proc.copy_next_only) {
        v |= B2B(cfg->proc.copy_on_ccm_err,  VTSS_F_OAM_MEP_VOE_BASIC_CTRL_CCM_ERR_CPU_COPY_ENA);
        v |= B2B(cfg->proc.copy_on_dmac_err, VTSS_F_OAM_MEP_VOE_BASIC_CTRL_MAC_ADDR_ERR_REDIR_ENA);
    }
    v |= B2B(cfg->mep_type == VTSS_OAM_UPMEP  &&  cfg->upmep.discard_rx, VTSS_F_OAM_MEP_VOE_BASIC_CTRL_UPMEP_KILL_ALL_RX);
    v |= B2B(cfg->mep_type == VTSS_OAM_MIP, VTSS_F_OAM_MEP_VOE_BASIC_CTRL_MIP_UNICAST_ONLY_COPY_ENA | VTSS_F_OAM_MEP_VOE_BASIC_CTRL_VOE_AS_MIP_ENA);
    v |= VTSS_F_OAM_MEP_VOE_BASIC_CTRL_VOE_ENA;
    SRVL_WR(VTSS_OAM_MEP_VOE_BASIC_CTRL(voe_idx), v);

    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_voe_ccm_arm_hitme(vtss_state_t *vtss_state,
                                          const vtss_oam_voe_idx_t voe_idx,
                                          const BOOL               enable)
{
    u32 bc = 0, mc = 0;
    const vtss_oam_voe_conf_t *cfg = &(vtss_state->oam.voe_conf[voe_idx]);

    if (cfg->proc.copy_next_only) {
        /* BASIC_CTRL */
        bc = B2B(cfg->proc.copy_on_ccm_more_than_one_tlv, VTSS_F_OAM_MEP_VOE_BASIC_CTRL_CPU_HITME_ONCE_CCM_TLV) |
             B2B(cfg->proc.copy_on_ccm_err, VTSS_F_OAM_MEP_VOE_BASIC_CTRL_CCM_ERR_CPU_HITME_ONCE) |
             B2B(cfg->ccm.copy_to_cpu, VTSS_F_OAM_MEP_VOE_BASIC_CTRL_CCM_NXT_CPU_HITME_ONCE);

        /* MEL_CTRL */
        mc = B2B(cfg->proc.copy_on_mel_too_low_err, VTSS_F_OAM_MEP_VOE_MEL_CTRL_CPU_HITME_ONCE_MEL_TOO_LOW | VTSS_F_OAM_MEP_VOE_MEL_CTRL_CPU_HITME_ONCE_CCM_MEL_TOO_LOW);
    }

    SRVL_WRM(VTSS_OAM_MEP_VOE_BASIC_CTRL(voe_idx), bc, bc);
    SRVL_WRM(VTSS_OAM_MEP_VOE_MEL_CTRL(voe_idx),   mc, mc);

    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_voe_ccm_set_rdi_flag(vtss_state_t *vtss_state,
                                             const vtss_oam_voe_idx_t voe_idx,
                                             const BOOL               flag)
{
    SRVL_WRM(VTSS_OAM_MEP_VOE_CCM_CFG(voe_idx), (flag) ? VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_TX_RDI : 0, VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_TX_RDI);
    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_ccm_status_get(vtss_state_t *vtss_state,
                                       const vtss_oam_voe_idx_t voe_idx,
                                       vtss_oam_ccm_status_t    *value)
{
    u32 v;

    SRVL_RD(VTSS_OAM_MEP_VOE_CCM_CFG(voe_idx), &v);
    value->period_err         = (v & VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_PERIOD_ERR)      != 0;
    value->priority_err       = (v & VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_PRIO_ERR)        != 0;
    value->zero_period_err    = (v & VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_ZERO_PERIOD_ERR) != 0;
    value->rx_rdi             = (v & VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_RX_RDI)          != 0;
    value->mep_id_err         = (v & VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_MEPID_ERR)       != 0;
    value->meg_id_err         = (v & VTSS_F_OAM_MEP_VOE_CCM_CFG_CCM_MEGID_ERR)       != 0;
    value->loc                = (VTSS_X_OAM_MEP_VOE_CCM_CFG_CCM_MISS_CNT(v) == 0x07) != 0;   // The CCM_LOC_DEFECT bit isn't wired properly in Serval 1

    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_pdu_seen_status_get(vtss_state_t *vtss_state,
                                            const vtss_oam_voe_idx_t   voe_idx,
                                            vtss_oam_pdu_seen_status_t *value)
{
    u32 i, v, mask;

    SRVL_RD(VTSS_OAM_MEP_VOE_OAM_RX_STICKY(voe_idx), &v);
    for (i=0; i<VTSS_OAM_GENERIC_OPCODE_CFG_CNT; ++i)
        value->generic_seen[i] = (VTSS_X_OAM_MEP_VOE_OAM_RX_STICKY_GENERIC_RX_STICKY_MASK(v) & (1<<i)) != 0;

    value->unknown_seen = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_UNK_OPCODE_RX_STICKY) != 0;
    value->ltm_seen     = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LTM_RX_STICKY)        != 0;
    value->ltr_seen     = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LTR_RX_STICKY)        != 0;
    value->lmm_seen     = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LMM_RX_STICKY)        != 0;
    value->lmr_seen     = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LMR_RX_STICKY)        != 0;
    value->lbm_seen     = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LBM_RX_STICKY)        != 0;
    value->lbr_seen     = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LBR_RX_STICKY)        != 0;
    value->dmm_seen     = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_DMM_RX_STICKY)        != 0;
    value->dmr_seen     = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_DMR_RX_STICKY)        != 0;
    value->one_dm_seen  = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_SM_RX_STICKY)         != 0;
    value->ccm_seen     = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_CCM_RX_STICKY)        != 0;
    value->ccm_lm_seen  = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_CCM_LM_RX_STICKY)     != 0;

    mask = VTSS_M_OAM_MEP_VOE_OAM_RX_STICKY_GENERIC_RX_STICKY_MASK |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_UNK_OPCODE_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LTM_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LTR_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LMM_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LMR_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LBM_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LBR_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_DMM_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_DMR_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_SM_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_CCM_RX_STICKY |
           VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_CCM_LM_RX_STICKY;

    SRVL_WR(VTSS_OAM_MEP_VOE_OAM_RX_STICKY(voe_idx), mask);

    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_proc_status_get(vtss_state_t *vtss_state,
                                        const vtss_oam_voe_idx_t voe_idx,
                                        vtss_oam_proc_status_t   *value)
{
    u32 v;

    SRVL_RD(VTSS_OAM_MEP_VOE_CCM_RX_SEQ_CFG(voe_idx), &v);
    value->rx_ccm_seq_no = (vtss_state->oam.voe_conf[voe_idx].mep_type == VTSS_OAM_UPMEP) ? v & 0xffff : v;

    SRVL_RD(VTSS_OAM_MEP_VOE_CCM_TX_SEQ_CFG(voe_idx), &value->tx_next_ccm_seq_no);
    SRVL_RD(VTSS_OAM_MEP_VOE_LBM_TX_TRANSID_CFG(voe_idx), &value->tx_next_lbm_transaction_id);
    SRVL_RD(VTSS_OAM_MEP_VOE_LBR_RX_TRANSID_CFG(voe_idx), &value->rx_lbr_transaction_id);

    value->rx_tst_seq_no = value->rx_lbr_transaction_id;  /* Same reg. on Serval-1 */

    SRVL_RD(VTSS_OAM_MEP_VOE_OAM_RX_STICKY(voe_idx), &v);
    value->rx_dmac_err_seen         = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_MAC_ADDR_ERR_STICKY)  != 0;
    value->tx_meg_level_err_seen    = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_MEP_EGR_BLOCK_STICKY) != 0;
    SRVL_WR(VTSS_OAM_MEP_VOE_OAM_RX_STICKY(voe_idx), VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_MAC_ADDR_ERR_STICKY | VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_MEP_EGR_BLOCK_STICKY);

    SRVL_RD(VTSS_OAM_MEP_VOE_STICKY(voe_idx), &v);
    value->rx_meg_level_err_seen    = (v & VTSS_F_OAM_MEP_VOE_STICKY_OAM_MEL_STICKY) != 0;
    SRVL_WR(VTSS_OAM_MEP_VOE_STICKY(voe_idx), VTSS_F_OAM_MEP_VOE_STICKY_OAM_MEL_STICKY);

    SRVL_RD(VTSS_OAM_MEP_VOE_CCM_CFG(voe_idx), &v);
    value->rx_meg_level_err         = (v & VTSS_F_OAM_MEP_VOE_CCM_CFG_OAM_MEL_ERR) != 0;

    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_voe_counter_update_serval_internal(vtss_state_t *vtss_state,
                                                           const vtss_oam_voe_idx_t voe_idx,
                                                           const u32 clear_mask)
{
    const u64 MAX_32U = 0xffffffffUL;
    u32 v;
    vtss_rc rc = VTSS_RC_OK;
    vtss_oam_voe_internal_counters_t *chipcnt = &vtss_state->oam.voe_internal[voe_idx].counters;

    /* CCM: For Serval we need to read a sticky bit and internally accumulate the value. */
    SRVL_RD(VTSS_OAM_MEP_VOE_OAM_RX_STICKY(voe_idx), &v);
    SRVL_WR(VTSS_OAM_MEP_VOE_OAM_RX_STICKY(voe_idx), VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_CCM_RX_SEQ_ERR_STICKY);
    v = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_CCM_RX_SEQ_ERR_STICKY) ? 1 : 0;
    vtss_cmn_counter_32_update(v + (chipcnt->ccm.rx_invalid_seq_no.prev & MAX_32U), &chipcnt->ccm.rx_invalid_seq_no, (clear_mask & VTSS_OAM_CNT_CCM) != 0);

    /* LB + TST: Again, a sticky-accumulator: */
    SRVL_RD(VTSS_OAM_MEP_VOE_OAM_RX_STICKY(voe_idx), &v);
    SRVL_WR(VTSS_OAM_MEP_VOE_OAM_RX_STICKY(voe_idx), VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LBR_TRANSID_ERR_STICKY);
    v = (v & VTSS_F_OAM_MEP_VOE_OAM_RX_STICKY_LBR_TRANSID_ERR_STICKY) ? 1 : 0;
    if (vtss_state->oam.voe_conf[voe_idx].lb.enable  ||  (clear_mask & VTSS_OAM_CNT_LB) != 0) {
        vtss_cmn_counter_32_update(v + (chipcnt->lb.rx_lbr_trans_id_err.prev  & MAX_32U), &chipcnt->lb.rx_lbr_trans_id_err, (clear_mask & VTSS_OAM_CNT_LB) != 0);
    }
    if (vtss_state->oam.voe_conf[voe_idx].tst.enable  ||  (clear_mask & VTSS_OAM_CNT_TST) != 0) {
        vtss_cmn_counter_32_update(v + (chipcnt->tst.rx_tst_trans_id_err.prev & MAX_32U), &chipcnt->tst.rx_tst_trans_id_err, (clear_mask & VTSS_OAM_CNT_TST) != 0);
    }

    return rc;
}

static vtss_rc srvl_oam_voe_counter_update_serval(vtss_state_t *vtss_state,
                                                  const vtss_oam_voe_idx_t voe_idx)
{
    return srvl_oam_voe_counter_update_serval_internal(vtss_state, voe_idx, 0);
}

static vtss_rc srvl_oam_voe_counter_update_internal(vtss_state_t *vtss_state,
                                                    const vtss_oam_voe_idx_t voe_idx,
                                                    const u32 clear_mask)
{
#define CHIPREAD(reg, cnt, clear) { SRVL_RD(reg, &v); vtss_cmn_counter_32_update(v, cnt, (clear_mask & (clear)) != 0); }

    u32 v;
    u32 i, offset;
    BOOL doing_lb, doing_tst;
    vtss_rc rc = VTSS_RC_OK;
    vtss_oam_voe_internal_counters_t *chipcnt = &vtss_state->oam.voe_internal[voe_idx].counters;

    rc = srvl_oam_voe_counter_update_serval_internal(vtss_state, voe_idx, clear_mask);

    /* LM */

    switch (vtss_state->oam.voe_conf[voe_idx].voe_type) {
    case VTSS_OAM_VOE_SERVICE:
        offset = voe_idx * VTSS_PRIO_ARRAY_SIZE;
        for (i=0; i<VTSS_PRIO_ARRAY_SIZE; ++i) {
            CHIPREAD(VTSS_OAM_MEP_RX_VOE_PM_MEP_RX_FRM_CNT(offset+i), &chipcnt->lm.lm_count[i].rx, VTSS_OAM_CNT_LM);
            CHIPREAD(VTSS_OAM_MEP_TX_VOE_PM_MEP_TX_FRM_CNT(offset+i), &chipcnt->lm.lm_count[i].tx, VTSS_OAM_CNT_LM);
        }
        break;
    case VTSS_OAM_VOE_PORT:
        offset = (voe_idx - VTSS_OAM_PORT_VOE_BASE_IDX) * VTSS_PRIO_ARRAY_SIZE;
        for (i=0; i<VTSS_PRIO_ARRAY_SIZE; ++i) {
            CHIPREAD(VTSS_OAM_MEP_PORT_PM_PORT_RX_FRM_CNT(offset+i), &chipcnt->lm.lm_count[i].rx, VTSS_OAM_CNT_LM);
            CHIPREAD(VTSS_OAM_MEP_PORT_PM_PORT_TX_FRM_CNT(offset+i), &chipcnt->lm.lm_count[i].tx, VTSS_OAM_CNT_LM);
        }
        break;
    }

    /* CCM */

    CHIPREAD(VTSS_OAM_MEP_VOE_CCM_RX_VLD_FC_CNT(voe_idx),   &chipcnt->ccm.rx_valid_count,   VTSS_OAM_CNT_CCM);
    CHIPREAD(VTSS_OAM_MEP_VOE_CCM_RX_INVLD_FC_CNT(voe_idx), &chipcnt->ccm.rx_invalid_count, VTSS_OAM_CNT_CCM);

    VTSS_RC(doing_calculate(vtss_state, voe_idx, &doing_lb, &doing_tst));

    /* LB */

    if (doing_lb || (clear_mask & VTSS_OAM_CNT_LM)) {
        CHIPREAD(VTSS_OAM_MEP_VOE_LBR_RX_FRM_CNT(voe_idx),     &chipcnt->lb.rx_lbr, VTSS_OAM_CNT_LB);
        CHIPREAD(VTSS_OAM_MEP_VOE_LBM_TX_TRANSID_CFG(voe_idx), &chipcnt->lb.tx_lbm, VTSS_OAM_CNT_LB);
    }

    /* TST */

    if (doing_tst || (clear_mask & VTSS_OAM_CNT_TST)) {
        CHIPREAD(VTSS_OAM_MEP_VOE_LBR_RX_FRM_CNT(voe_idx),     &chipcnt->tst.rx_tst, VTSS_OAM_CNT_TST);
        CHIPREAD(VTSS_OAM_MEP_VOE_LBM_TX_TRANSID_CFG(voe_idx), &chipcnt->tst.tx_tst, VTSS_OAM_CNT_TST);
    }

    /* Selected */

    CHIPREAD(VTSS_OAM_MEP_VOE_RX_SEL_OAM_CNT(voe_idx), &chipcnt->sel.selected_frames.rx,    VTSS_OAM_CNT_SEL);
    CHIPREAD(VTSS_OAM_MEP_VOE_TX_SEL_OAM_CNT(voe_idx), &chipcnt->sel.selected_frames.tx,    VTSS_OAM_CNT_SEL);
    CHIPREAD(VTSS_OAM_MEP_VOE_RX_OAM_FRM_CNT(voe_idx), &chipcnt->sel.nonselected_frames.rx, VTSS_OAM_CNT_SEL);
    CHIPREAD(VTSS_OAM_MEP_VOE_TX_OAM_FRM_CNT(voe_idx), &chipcnt->sel.nonselected_frames.tx, VTSS_OAM_CNT_SEL);

    return rc;
#undef CHIPREAD
}

static vtss_rc srvl_oam_voe_counter_update(vtss_state_t *vtss_state,
                                           const vtss_oam_voe_idx_t voe_idx)
{
    return srvl_oam_voe_counter_update_internal(vtss_state, voe_idx, 0);
}

#define SET(v) value->v = chipcnt->v.value
static vtss_rc srvl_oam_voe_counter_get(vtss_state_t *vtss_state,
                                        const vtss_oam_voe_idx_t voe_idx,
                                        vtss_oam_voe_counter_t   *value)
{
    u32 v, i;
    vtss_rc rc = VTSS_RC_OK;
    const vtss_oam_voe_internal_counters_t *chipcnt = &vtss_state->oam.voe_internal[voe_idx].counters;

    /* Poll so we get the most recent values */
    rc = srvl_oam_voe_counter_update_internal(vtss_state, voe_idx, 0);

    /* LM */

    switch (vtss_state->oam.voe_conf[voe_idx].mep_type) {
    case VTSS_OAM_DOWNMEP:
        SRVL_RD(VTSS_OAM_MEP_VOE_CCM_CAP_TX_FCf(voe_idx), &value->lm.down_mep.tx_FCf);
        SRVL_RD(VTSS_OAM_MEP_VOE_CCM_CAP_RX_FCl(voe_idx), &value->lm.down_mep.rx_FCl);
        break;
    case VTSS_OAM_UPMEP:
        SRVL_RD(VTSS_OAM_MEP_COMMON_UPMEP_LM_CNT_VLD(voe_idx), &v);
        value->lm.up_mep.lmm_valid          = (v & VTSS_F_OAM_MEP_COMMON_UPMEP_LM_CNT_VLD_LMM_RX_CNT_VLD)    != 0;
        value->lm.up_mep.lmr_valid          = (v & VTSS_F_OAM_MEP_COMMON_UPMEP_LM_CNT_VLD_LMR_RX_CNT_VLD)    != 0;
        value->lm.up_mep.ccm_lm_valid       = (v & VTSS_F_OAM_MEP_COMMON_UPMEP_LM_CNT_VLD_CCM_LM_RX_CNT_VLD) != 0;

        SRVL_RD(VTSS_OAM_MEP_VOE_CCM_CAP_TX_FCf(voe_idx), &value->lm.up_mep.lmm);
        SRVL_RD(VTSS_OAM_MEP_VOE_CCM_CAP_RX_FCl(voe_idx), &value->lm.up_mep.ccm_lm);
        SRVL_RD(VTSS_OAM_MEP_VOE_UPMEP_LMR_RX_LM_CNT(voe_idx), &value->lm.up_mep.lmr);

        SRVL_RD(VTSS_OAM_MEP_VOE_UPMEP_LM_CNT_STICKY(voe_idx), &v);
        value->lm.up_mep.lmm_sample_lost    = (v & VTSS_F_OAM_MEP_VOE_UPMEP_LM_CNT_STICKY_LMM_RX_CNT_STICKY)    != 0;
        value->lm.up_mep.lmr_sample_lost    = (v & VTSS_F_OAM_MEP_VOE_UPMEP_LM_CNT_STICKY_LMR_RX_CNT_STICKY)    != 0;
        value->lm.up_mep.ccm_lm_sample_lost = (v & VTSS_F_OAM_MEP_VOE_UPMEP_LM_CNT_STICKY_CCM_LM_RX_CNT_STICKY) != 0;
        SRVL_WR(VTSS_OAM_MEP_VOE_UPMEP_LM_CNT_STICKY(voe_idx),
            VTSS_F_OAM_MEP_VOE_UPMEP_LM_CNT_STICKY_LMM_RX_CNT_STICKY |
            VTSS_F_OAM_MEP_VOE_UPMEP_LM_CNT_STICKY_LMR_RX_CNT_STICKY |
            VTSS_F_OAM_MEP_VOE_UPMEP_LM_CNT_STICKY_CCM_LM_RX_CNT_STICKY);

        SRVL_RD(VTSS_OAM_MEP_VOE_CCM_RX_SEQ_CFG(voe_idx), &v);

        value->lm.up_mep.rx_ccm_lm_sample_seq_no = (v>>16) & 0x1f;
        value->lm.up_mep.rx_lmr_sample_seq_no    = (v>>21) & 0x1f;
        value->lm.up_mep.rx_lmm_sample_seq_no    = (v>>26) & 0x1f;

        break;
    case VTSS_OAM_MIP:
        rc = VTSS_RC_INV_STATE;
        break;
    }

    for (i=0; i<VTSS_PRIO_ARRAY_SIZE; ++i) {
        SET(lm.lm_count[i].rx);
        SET(lm.lm_count[i].tx);
    }

    /* CCM */

    SET(ccm.rx_valid_count);
    SET(ccm.rx_invalid_count);
    SET(ccm.rx_invalid_seq_no);

    /* LB */

    SET(lb.rx_lbr);
    SET(lb.tx_lbm);
    SET(lb.rx_lbr_trans_id_err);

    /* TST */

    SET(tst.rx_tst);
    SET(tst.tx_tst);
    SET(tst.rx_tst_trans_id_err);

    /* Selected */

    SET(sel.selected_frames.rx);
    SET(sel.selected_frames.tx);
    SET(sel.nonselected_frames.rx);
    SET(sel.nonselected_frames.tx);

    return rc;
}
#undef SET

static vtss_rc srvl_oam_voe_counter_clear(vtss_state_t *vtss_state,
                                          const vtss_oam_voe_idx_t voe_idx, const u32 mask)
{
    /* Poll & clear */
    return srvl_oam_voe_counter_update_internal(vtss_state, voe_idx, mask);
}

static vtss_rc srvl_oam_ipt_conf_get(vtss_state_t *vtss_state,
                                     u32 isdx, vtss_oam_ipt_conf_t *cfg)
{
    u32 v;
    SRVL_RD(VTSS_ANA_IPT_OAM_MEP_CFG(isdx), &v);
    cfg->enable          = (v & VTSS_F_ANA_IPT_OAM_MEP_CFG_MEP_IDX_ENA) != 0;
    cfg->service_voe_idx = VTSS_X_ANA_IPT_OAM_MEP_CFG_MEP_IDX(v);
    return VTSS_RC_OK;
}

static vtss_rc srvl_oam_ipt_conf_set(vtss_state_t *vtss_state,
                                     u32 isdx, const vtss_oam_ipt_conf_t *const cfg)
{
    SRVL_WR(VTSS_ANA_IPT_OAM_MEP_CFG(isdx),
        VTSS_F_ANA_IPT_OAM_MEP_CFG_MEP_IDX(cfg->service_voe_idx) |
        VTSS_F_ANA_IPT_OAM_MEP_CFG_MEP_IDX_P(cfg->service_voe_idx) |
        B2B(cfg->enable, VTSS_F_ANA_IPT_OAM_MEP_CFG_MEP_IDX_ENA));
    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

/*
 * OAM CIL debug. (AIL debug is in vtss_common.c)
 */

// D_COM: Debug COMmon; DR_COM: Debug Read COMmon. _I for Instance. Etc.
#define D_COM(pr, name)            vtss_srvl_debug_reg(vtss_state, pr, VTSS_OAM_MEP_COMMON_##name,                    "COMMON:"#name)
#define D_COM_I(pr, name, i)       vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_OAM_MEP_COMMON_##name(i),      (i),  "COMMON:"#name)
#define D_VOE_I(pr, name, i)       vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_OAM_MEP_VOE_##name(i),         (i),  "VOE:"#name)
#define D_VOE_II(pr, name, i1, i2) vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_OAM_MEP_VOE_##name((i1),(i2)), (i2), "VOE:"#name)
#define DR_COM(name, v)            { SRVL_RD(VTSS_OAM_MEP_COMMON_##name, &v); }
#define DR_COM_I(name, i, v)       { SRVL_RD(VTSS_OAM_MEP_COMMON_##name(i), &v); }
#define DR_VOE_I(name, i, v)       { SRVL_RD(VTSS_OAM_MEP_VOE_##name(i), &v); }
#define DR_VOE_II(name, i1, i2, v) { SRVL_RD(VTSS_OAM_MEP_VOE_##name((i1),(i2)), &v); }

static vtss_rc srvl_debug_oam(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
    u32 i, k, v;
    char buf[10];

    vtss_srvl_debug_reg_header(pr, "VOP");
    D_COM(pr, MEP_CTRL);
    D_COM(pr, CPU_CFG);
    D_COM(pr, CPU_CFG_1);
    D_COM(pr, OAM_SDLB_CPU_COPY);
    for (i=0; i<8; ++i)
        D_COM_I(pr, OAM_GENERIC_CFG, i);
    for (i=0; i<7; ++i)
        D_COM_I(pr, CCM_PERIOD_CFG, i);
    D_COM(pr, CCM_CTRL);
    D_COM(pr, CCM_SCAN_STICKY);
    D_COM(pr, MASTER_INTR_CTRL);
    for (i=0; i<3; ++i)
        D_COM_I(pr, INTR, i);
    D_COM(pr, COMMON_MEP_MC_MAC_MSB);
    D_COM(pr, COMMON_MEP_MC_MAC_LSB);
    pr("\n");

    for (i=0; i<VTSS_OAM_VOE_CNT; ++i) {
        DR_VOE_I(BASIC_CTRL, i, v);
        if (info->full  ||  (v & VTSS_F_OAM_MEP_VOE_BASIC_CTRL_VOE_ENA) != 0) {
            sprintf(buf, "VOE %u", i);
            vtss_srvl_debug_reg_header(pr, buf);
            D_COM_I(pr, VOE_CFG, i);
            D_COM_I(pr, UPMEP_LM_CNT_VLD, i);
            D_COM_I(pr, VOE_MAP_CTRL, i);
            D_COM_I(pr, VOE_CNT_CTRL, i);
            D_VOE_I(pr, BASIC_CTRL, i);
            D_VOE_I(pr, MEL_CTRL, i);
            D_VOE_I(pr, OAM_CPU_COPY_CTRL, i);
            D_VOE_I(pr, OAM_FWD_CTRL, i);
            D_VOE_I(pr, OAM_SDLB_CTRL, i);
            D_VOE_I(pr, OAM_CNT_OAM_CTRL, i);
            D_VOE_I(pr, OAM_CNT_DATA_CTRL, i);
            D_VOE_I(pr, MEP_UC_MAC_MSB, i);
            D_VOE_I(pr, MEP_UC_MAC_LSB, i);
            D_VOE_I(pr, OAM_HW_CTRL, i);
            D_VOE_I(pr, LOOPBACK_CTRL, i);
            D_VOE_I(pr, LBR_RX_FRM_CNT, i);
            D_VOE_I(pr, LBR_RX_TRANSID_CFG, i);
            D_VOE_I(pr, LBM_TX_TRANSID_UPDATE, i);
            D_VOE_I(pr, LBM_TX_TRANSID_CFG, i);
            D_VOE_I(pr, CCM_CFG, i);
            D_VOE_I(pr, CCM_RX_VLD_FC_CNT, i);
            D_VOE_I(pr, CCM_RX_INVLD_FC_CNT, i);
            D_VOE_I(pr, CCM_CAP_TX_FCf, i);
            D_VOE_I(pr, CCM_CAP_RX_FCl, i);
            D_VOE_I(pr, CCM_TX_SEQ_CFG, i);
            D_VOE_I(pr, CCM_RX_SEQ_CFG, i);
            D_VOE_I(pr, CCM_MEPID_CFG, i);
            for (k=0; k<12; ++k)
                D_VOE_II(pr, CCM_MEGID_CFG, i, 12-k);
            D_VOE_I(pr, OAM_RX_STICKY, i);
            D_VOE_I(pr, STICKY, i);
            D_VOE_I(pr, UPMEP_LM_CNT_STICKY, i);
            D_VOE_I(pr, UPMEP_LMR_RX_LM_CNT, i);
            D_VOE_I(pr, INTR_ENA, i);
            D_VOE_I(pr, RX_SEL_OAM_CNT, i);
            D_VOE_I(pr, TX_SEL_OAM_CNT, i);
            D_VOE_I(pr, RX_OAM_FRM_CNT, i);
            D_VOE_I(pr, TX_OAM_FRM_CNT, i);
            // Not included: PORT_OM, RX_VOE_OM, TX_VOE_PM
            //   -- too much output
        }
    }
    pr("\n");

    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_oam_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_OAM, srvl_debug_oam, vtss_state, pr, info);
}

#undef D_COM
#undef D_COM_I
#undef D_VOE_I
#undef D_VOE_II
#undef DR_COM
#undef DR_COM_I
#undef DR_VOE_I
#undef DR_VOE_II

/* - Initialization ------------------------------------------------ */

static vtss_rc srvl_oam_voe_poll_1sec(vtss_state_t *vtss_state)
{
    /* Since all VOE counters are PDU counters (not byte counters) and Serval-1
     * has 75 VOEs in total, we can relax quite a bit; 32-bit wrap-around is
     * several minutes down the line, worst-case.
     */
    const u32 N = 1;

    vtss_oam_voe_idx_t *idx = &(vtss_state->oam.voe_poll_idx);  /* For readability */
    u32 i;

    for (i=0; i<N; ++i) {
        VTSS_RC(srvl_oam_voe_counter_update(vtss_state, *idx));
        *idx = (*idx == VTSS_OAM_VOE_CNT-1) ? 0 : *idx + 1;
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_oam_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_oam_state_t *state = &vtss_state->oam;
    u32              i;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->vop_cfg_set               = srvl_oam_vop_cfg_set;
        state->voe_alloc                 = srvl_oam_voe_alloc;
        state->voe_free                  = srvl_oam_voe_free;
        state->voe_cfg_set               = srvl_oam_voe_cfg_set;
        state->event_poll                = srvl_oam_event_poll;
        state->voe_event_poll            = srvl_oam_voe_event_poll;
        state->voe_event_enable          = srvl_oam_voe_event_enable;
        state->voe_ccm_arm_hitme         = srvl_oam_voe_ccm_arm_hitme;
        state->voe_ccm_set_rdi_flag      = srvl_oam_voe_ccm_set_rdi_flag;
        state->ccm_status_get            = srvl_oam_ccm_status_get;
        state->pdu_seen_status_get       = srvl_oam_pdu_seen_status_get;
        state->proc_status_get           = srvl_oam_proc_status_get;
        state->voe_counter_update        = srvl_oam_voe_counter_update;
        state->voe_counter_update_serval = srvl_oam_voe_counter_update_serval;
        state->voe_counter_get           = srvl_oam_voe_counter_get;
        state->voe_counter_clear         = srvl_oam_voe_counter_clear;
        state->ipt_conf_get              = srvl_oam_ipt_conf_get;
        state->ipt_conf_set              = srvl_oam_ipt_conf_set;
        break;
    case VTSS_INIT_CMD_INIT:
        /* All VOEs are default enabled by hardware; we disable them here. This is
         * mainly to prevent the CLI command "debug api cil oam" from spewing out
         * loads of irrelevant information -- it only dumps enabled VOEs by
         * default. */
        for (i=0; i<VTSS_OAM_VOE_CNT; ++i)
            SRVL_WR(VTSS_OAM_MEP_VOE_BASIC_CTRL(i), 0);
        break;
    case VTSS_INIT_CMD_POLL:
        VTSS_RC(srvl_oam_voe_poll_1sec(vtss_state));
        break;
    default:
        break;
    }
    return VTSS_RC_OK;
}

#endif /* VTSS_FEATURE_OAM */

#endif /* VTSS_ARCH_SERVAL */
