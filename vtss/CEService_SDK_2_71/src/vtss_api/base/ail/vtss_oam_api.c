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
#include "vtss_api.h"
#include "vtss_state.h"
#include "vtss_common.h"

#if defined(VTSS_FEATURE_OAM)

vtss_rc vtss_oam_vop_conf_get(const vtss_inst_t          inst,
                              vtss_oam_vop_conf_t *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam vop get");

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        *cfg = vtss_state->oam.vop_conf;
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_vop_conf_set(const vtss_inst_t                inst,
                              const vtss_oam_vop_conf_t *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam vop set");

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        vtss_state->oam.vop_conf = *cfg;
        rc = VTSS_FUNC_0(oam.vop_cfg_set);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_oam_event_poll(const vtss_inst_t            inst,
                            vtss_oam_event_mask_t *const mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam event poll");

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.event_poll, mask);
    }
    VTSS_EXIT();
    return rc;        

}

vtss_rc vtss_oam_voe_event_enable(const vtss_inst_t               inst,
                                  const vtss_oam_voe_idx_t        voe_idx,
                                  const vtss_oam_voe_event_mask_t mask,
                                  const BOOL                      enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d event enable: 0x%08x", voe_idx, mask);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.voe_event_enable, voe_idx, mask, enable);
    }
    VTSS_EXIT();
    return rc;        
}

vtss_rc vtss_oam_voe_event_poll(const vtss_inst_t         inst,
                                const vtss_oam_voe_idx_t  voe_idx,
                                vtss_oam_voe_event_mask_t *const mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d event poll", voe_idx);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.voe_event_poll, voe_idx, mask);
    }
    VTSS_EXIT();
    return rc;        
}

vtss_rc vtss_oam_voe_alloc(const vtss_inst_t              inst,
                           const vtss_oam_voe_type_t      type,
                           const vtss_oam_voe_alloc_cfg_t *data,
                           vtss_oam_voe_idx_t             *voe_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe alloc, type %d", type);

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.voe_alloc, type, data, voe_idx);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_oam_voe_free(const vtss_inst_t        inst,
                          const vtss_oam_voe_idx_t voe_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d free", voe_idx);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.voe_free, voe_idx);
    }
    VTSS_EXIT();
    return rc;

}

vtss_rc vtss_oam_voe_conf_get(const vtss_inst_t          inst,
                              const vtss_oam_voe_idx_t   voe_idx,
                              vtss_oam_voe_conf_t *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d get", voe_idx);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        *cfg = vtss_state->oam.voe_conf[voe_idx];
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_voe_conf_set(const vtss_inst_t                inst,
                              const vtss_oam_voe_idx_t         voe_idx,
                              const vtss_oam_voe_conf_t *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d set", voe_idx);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        vtss_state->oam.voe_conf[voe_idx] = *cfg;
        rc = VTSS_FUNC(oam.voe_cfg_set, voe_idx);
    }
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_voe_ccm_arm_hitme(const vtss_inst_t        inst,
                                   const vtss_oam_voe_idx_t voe_idx,
                                   BOOL                     enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d hitme arm: %d", voe_idx, enable);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.voe_ccm_arm_hitme, voe_idx, enable);
    }
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_voe_ccm_set_rdi_flag(const vtss_inst_t        inst,
                                      const vtss_oam_voe_idx_t voe_idx,
                                      BOOL                     flag)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d rdi set: %d", voe_idx, flag);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.voe_ccm_set_rdi_flag, voe_idx, flag);
    }
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_ccm_status_get(const vtss_inst_t        inst,
                                const vtss_oam_voe_idx_t voe_idx,
                                vtss_oam_ccm_status_t    *value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d status get", voe_idx);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.ccm_status_get, voe_idx, value);
    }
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_pdu_seen_status_get(const vtss_inst_t          inst,
                                     const vtss_oam_voe_idx_t   voe_idx,
                                     vtss_oam_pdu_seen_status_t *value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d status get", voe_idx);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.pdu_seen_status_get, voe_idx, value);
    }
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_proc_status_get(const vtss_inst_t        inst,
                                 const vtss_oam_voe_idx_t voe_idx,
                                 vtss_oam_proc_status_t   *value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d processing status get", voe_idx);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.proc_status_get, voe_idx, value);
    }
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_voe_counter_update(const vtss_inst_t        inst,
                                    const vtss_oam_voe_idx_t voe_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d counter update", voe_idx);

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.voe_counter_update, voe_idx);
    }
    VTSS_EXIT();
    return rc;

}

#if defined(VTSS_ARCH_SERVAL)
vtss_rc vtss_oam_voe_counter_update_serval(const vtss_inst_t        inst,
                                           const vtss_oam_voe_idx_t voe_idx)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d counter update (serval)", voe_idx);

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.voe_counter_update_serval, voe_idx);
    }
    VTSS_EXIT();
    return rc;

}
#endif

vtss_rc vtss_oam_voe_counter_get(const vtss_inst_t        inst,
                                 const vtss_oam_voe_idx_t voe_idx,
                                 vtss_oam_voe_counter_t   *value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d counter get", voe_idx);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.voe_counter_get, voe_idx, value);
    }
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_voe_counter_clear(const vtss_inst_t        inst,
                                   const vtss_oam_voe_idx_t voe_idx,
                                   const u32                mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam voe %d status get", voe_idx);

    if (voe_idx >= VTSS_OAM_VOE_CNT)
        return VTSS_RC_ERROR;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.voe_counter_clear, voe_idx, mask);
    }
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_ipt_conf_get(const vtss_inst_t   inst,
                              const u32           isdx,
                              vtss_oam_ipt_conf_t *cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam ipt %d conf get", isdx);

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.ipt_conf_get, isdx, cfg);
    }
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_oam_ipt_conf_set(const vtss_inst_t                inst,
                              const u32                        isdx,
                              const vtss_oam_ipt_conf_t *const cfg)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("oam ipt %d conf set", isdx);

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(oam.ipt_conf_set, isdx, cfg);
    }
    VTSS_EXIT();
    return rc;    
}

/* - Instance create and initialization ---------------------------- */

vtss_rc vtss_oam_inst_create(vtss_state_t *vtss_state)
{
    vtss_oam_voe_conf_t *v;
    int                 i;

    for (i=0; i<VTSS_OAM_VOE_CNT; ++i) {
        v = &vtss_state->oam.voe_conf[i];
        v->voe_type = (i<VTSS_OAM_PATH_SERVICE_VOE_CNT) ? VTSS_OAM_VOE_SERVICE : VTSS_OAM_VOE_PORT;
    }
    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

#define YN(x) ((x) ? "Yes" : "No ")

static void vtss_debug_print_mac(const vtss_debug_printf_t pr,
                                        const vtss_mac_t   *m)
{
     pr("%02X-%02X-%02X-%02X-%02X-%02X",
        m->addr[0], m->addr[1], m->addr[2], m->addr[3], m->addr[4], m->addr[5]);
}

static void vtss_debug_print_oam_extract(const vtss_debug_printf_t         pr,
                                         const vtss_oam_vop_extract_conf_t c)
{
    pr("%-5s %4u", (c.to_front ? "Front" : "CPU"), c.rx_queue);
}

void vtss_oam_debug_print(vtss_state_t *vtss_state,
                          const vtss_debug_printf_t pr,
                          const vtss_debug_info_t   *const info)
{
    u32 i, n;
    const vtss_oam_vop_conf_t *vop = &vtss_state->oam.vop_conf;
    const vtss_oam_voe_conf_t *voe;
    static const char *mep_type[3] = { "Down", "Up  ", "MIP " };
    static const char *ccm_tx_seq_type[3] = { "No     ", "Inc+Upd", "Update " };
    static const char *period[5] = { "Inval", "3.3ms", "10ms ", "100ms", "1sec " };
    static const char *dmac_check[4] = { "Unicast   ", "Multicast ",
                                         "Uni+Multi ", "None      " };

    if (!vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_OAM))
        return;

    // VOP

    pr("VOP:\n\n");

    pr("Enable_VOEs:%s CCM-LM_propr:%s LMR_propr:%s MC-DMAC:",
        YN(vop->enable_all_voe),
        YN(vop->ccm_lm_enable_rx_fcf_in_reserved_field),
        YN(vop->down_mep_lmr_proprietary_fcf_use));
    vtss_debug_print_mac(pr, &vop->common_multicast_dmac);
    pr(" S-DLB_idx:%u Ext_CPU_portmask:0x%08x\n",
        vop->sdlb_cpy_copy_idx,
        vop->external_cpu_portmask);

    pr("PDU extract:\n");
    pr("Type      Dest. Idx. Opcode DMAC_check\n");
    for (i=0; i<VTSS_OAM_GENERIC_OPCODE_CFG_CNT; ++i) {
        pr("Generic_%u ", i);
        vtss_debug_print_oam_extract(pr, vop->pdu_type.generic[i].extract);
        pr(" %6u %s\n", vop->pdu_type.generic[i].opcode, YN(vop->pdu_type.generic[i].check_dmac));
    }
    pr("CCM       "); vtss_debug_print_oam_extract(pr, vop->pdu_type.ccm);    pr("\n");
    pr("CCM-LM    "); vtss_debug_print_oam_extract(pr, vop->pdu_type.ccm_lm); pr("\n");
    pr("LT        "); vtss_debug_print_oam_extract(pr, vop->pdu_type.lt);     pr("\n");
    pr("DMM       "); vtss_debug_print_oam_extract(pr, vop->pdu_type.dmm);    pr("\n");
    pr("DMR       "); vtss_debug_print_oam_extract(pr, vop->pdu_type.dmr);    pr("\n");
    pr("LMM       "); vtss_debug_print_oam_extract(pr, vop->pdu_type.lmm);    pr("\n");
    pr("LMR       "); vtss_debug_print_oam_extract(pr, vop->pdu_type.lmr);    pr("\n");
    pr("LBM       "); vtss_debug_print_oam_extract(pr, vop->pdu_type.lbm);    pr("\n");
    pr("LBR       "); vtss_debug_print_oam_extract(pr, vop->pdu_type.lbr);    pr("\n");
    pr("Errored   "); vtss_debug_print_oam_extract(pr, vop->pdu_type.err);    pr("\n");
    pr("Other     "); vtss_debug_print_oam_extract(pr, vop->pdu_type.other);  pr("\n");
    pr("\nVOEs:\n\n");

    for (i=0; i<VTSS_OAM_VOE_CNT; ++i) {
        voe = &vtss_state->oam.voe_conf[i];
        if (info->full  ||  vtss_state->oam.voe_internal[i].allocated) {
            pr("VOE:%4u Enabled:%s Type:%s MEP-type:%s UC-MAC:",
                i,
                YN(vtss_state->oam.voe_internal[i].allocated),
                (voe->voe_type == VTSS_OAM_VOE_SERVICE ? "Service" : "Port"),
                mep_type[voe->mep_type]);
            vtss_debug_print_mac(pr, &voe->unicast_mac);
            pr("\n");

            pr("         Cnt_in_path_MEP:%s W:%-4u P:%-4u "
                "Loop:W:%-4u P:%-4u PPort:%-2u S-DLB:%s S-DLB_Idx:%-4u\n",
                YN(voe->svc_to_path), voe->svc_to_path_idx_w,
                voe->svc_to_path_idx_p,
                voe->loop_isdx_w, voe->loop_isdx_p, voe->loop_portidx_p,
                YN(voe->sdlb_enable), voe->sdlb_idx);

            for (n=0; n<VTSS_OAM_GENERIC_OPCODE_CFG_CNT; ++n) {
                if (info->full  ||  voe->generic[n].enable) {
                    pr("         Generic:%u Opcode:%-3u Enable:%s "
                        "Copy_CPU:%s Fwd:%s CntSel:%s CntData:%s\n",
                        n, vop->pdu_type.generic[n].opcode,
                        YN(voe->generic[n].enable),
                        YN(voe->generic[n].copy_to_cpu),
                        YN(voe->generic[n].forward),
                        YN(voe->generic[n].count_as_selected),
                        YN(voe->generic[n].count_as_data));
                }
            }
            if (info->full  ||  voe->ccm.enable) {
                pr("         CCM:    Enable:%s Copy_CPU:%s Fwd:%s "
                    "CntSel:%s CntData:%s MEPid:%-5u\n"
                    "                 MEGid:",
                    YN(voe->ccm.enable),
                    YN(voe->ccm.copy_to_cpu),
                    YN(voe->ccm.forward),
                    YN(voe->ccm.count_as_selected),
                    YN(voe->ccm.count_as_data),
                    voe->ccm.mepid                  
                    );
                for (n=0; n<48; ++n) {
                    if (n && n % 12 == 0)
                        pr("\n                       ");
                    if (voe->ccm.megid.data[n] >= 32)
                        pr("%c", voe->ccm.megid.data[n]);
                    else
                        pr("\\x%02u", voe->ccm.megid.data[n]);
                }
                pr("\n");
                pr("                 AutoTXSeq:%s TxSeq:%-5u CheckRxSeq:%s "
                    "RxSeq:%-5u "
                    "RxPri:%1u RxPeriod:%s\n",
                    ccm_tx_seq_type[voe->ccm.tx_seq_no_auto_upd_op],
                    voe->ccm.tx_seq_no,
                    YN(voe->ccm.rx_seq_no_check),
                    voe->ccm.rx_seq_no,
                    voe->ccm.rx_priority,
                    period[voe->ccm.rx_period]);
            }
            if (info->full  ||  voe->ccm_lm.enable) {
                pr("         CCM-LM: Enable:%s Copy_CPU:%s Fwd:%s "
                   "CntSel:%s PriMask:0x%02x Yellow:%s Period:%s\n",
                   YN(voe->ccm_lm.enable),
                   YN(voe->ccm_lm.copy_to_cpu),
                   YN(voe->ccm_lm.forward),
                   YN(voe->ccm_lm.count_as_selected),
                   voe->ccm_lm.count.priority_mask,
                   YN(voe->ccm_lm.count.yellow),
                   period[voe->ccm_lm.period]);
            }
            if (info->full  ||  voe->single_ended_lm.enable) {
                pr("         SE-LM:  Enable:%s Copy_LMM_CPU:%s Copy_LMR_CPU:%s"
                    " Fwd_LMM:%s Fwd_LMR:%s CntSel:%s CntData:%s PriMask:0x%02x"
                    " Yellow:%s\n",
                    YN(voe->single_ended_lm.enable),
                    YN(voe->single_ended_lm.copy_lmm_to_cpu),
                    YN(voe->single_ended_lm.copy_lmr_to_cpu),
                    YN(voe->single_ended_lm.forward_lmm),
                    YN(voe->single_ended_lm.forward_lmr),
                    YN(voe->single_ended_lm.count_as_selected),
                    YN(voe->single_ended_lm.count_as_data),
                    voe->single_ended_lm.count.priority_mask,
                    YN(voe->single_ended_lm.count.yellow));
            }
            if (info->full  ||  voe->lt.enable) {
                pr("         LT:     Enable:%s Copy_LTM_CPU:%s Copy_LTR_CPU:%s"
                    " Fwd_LTM:%s Fwd_LTR:%s CntSel:%s CntData:%s\n",
                    YN(voe->lt.enable),
                    YN(voe->lt.copy_ltm_to_cpu),
                    YN(voe->lt.copy_ltr_to_cpu),
                    YN(voe->lt.forward_ltm),
                    YN(voe->lt.forward_ltr),
                    YN(voe->lt.count_as_selected),
                    YN(voe->lt.count_as_data));
            }
            if (info->full  ||  voe->lb.enable) {
                pr("         LB:     Enable:%s Copy_LBM_CPU:%s Copy_LBR_CPU:%s"
                    " Fwd_LBM:%s Fwd_LBR:%s CntSel:%s CntData:%s\n"
                    "                 Upd_TX_transid:%s TX_transid:0x%08x RX_transid:0x%08x\n",
                    YN(voe->lb.enable),
                    YN(voe->lb.copy_lbm_to_cpu),
                    YN(voe->lb.copy_lbr_to_cpu),
                    YN(voe->lb.forward_lbm),
                    YN(voe->lb.forward_lbr),
                    YN(voe->lb.count_as_selected),
                    YN(voe->lb.count_as_data),
                    YN(voe->lb.tx_update_transaction_id),
                    voe->lb.tx_transaction_id,
                    voe->lb.rx_transaction_id);
            }
            if (info->full  ||  voe->tst.enable) {
                pr("         TST:    Enable:%s Copy_CPU:%s"
                    " Fwd:%s CntSel:%s CntData:%s "
                    " Upd_TX_seq:%s TX_seq:0x%08x RX_seq:0x%08x\n",
                    YN(voe->tst.enable),
                    YN(voe->tst.copy_to_cpu),
                    YN(voe->tst.forward),
                    YN(voe->tst.count_as_selected),
                    YN(voe->tst.count_as_data),
                    YN(voe->tst.tx_seq_no_auto_update),
                    voe->tst.tx_seq_no,
                    voe->tst.rx_seq_no);
            }
            if (info->full  ||  voe->dm.enable_dmm  ||  voe->dm.enable_1dm) {
                pr("         DM:     CntSel:%s CntData:%s\n",
                    YN(voe->dm.count_as_selected),
                    YN(voe->dm.count_as_data));
                pr("         1DM:    Enable:%s Copy_CPU:%s Fwd:%s\n",
                    YN(voe->dm.enable_1dm),
                    YN(voe->dm.copy_1dm_to_cpu),
                    YN(voe->dm.forward_1dm));
                pr("         DMM:    Enable:%s Copy_DMM_CPU:%s Copy_DMR_CPU:%s "
                    "Fwd_DMM:%s Fwd_DMR:%s\n",
                    YN(voe->dm.enable_dmm),
                    YN(voe->dm.copy_dmm_to_cpu),
                    YN(voe->dm.copy_dmr_to_cpu),
                    YN(voe->dm.forward_dmm),
                    YN(voe->dm.forward_dmr));
            }
            pr("         Proc:   MEL:%u DMAC_chk:%s Chk_CCM_only:%s "
                "Cp_Nxt_Only:%s Cp_CCM_err:%s Cp_CCM_TLV:%s\n"
                "                 Cp_MEL_low:%s Cp_DMAC_err:%s\n",
                voe->proc.meg_level,
                dmac_check[voe->proc.dmac_check_type],
                YN(voe->proc.ccm_check_only),
                YN(voe->proc.copy_next_only),
                YN(voe->proc.copy_on_ccm_err),
                YN(voe->proc.copy_on_ccm_more_than_one_tlv),
                YN(voe->proc.copy_on_mel_too_low_err),
                YN(voe->proc.copy_on_dmac_err));
            if (info->full  ||  voe->mep_type == VTSS_OAM_UPMEP) {
                pr("         Up-MEP: Discard_RX:%s Loopback:%s Port:%-u\n",
                    YN(voe->upmep.discard_rx),
                    YN(voe->upmep.loopback),
                    voe->upmep.port);
            }
        }
    }
}
#undef YN

#endif  /* VTSS_FEATURE_OAM */

