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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_EVC
#include "vtss_jaguar1_cil.h"

#if defined(VTSS_ARCH_JAGUAR_1)

#if defined(VTSS_FEATURE_EVC)

/* - CIL functions ------------------------------------------------- */

/* Read and update ANA_AC/REW counters */
#define JR_SDX_CNT_GET(tgt, sdx, idx, id, cnt, clr)                     \
{                                                                       \
    u32 lsb, msb;                                                       \
    JR_RDXY(tgt, STAT_CNT_CFG_##sdx##_STAT_LSB_CNT, idx, id, &lsb);     \
    JR_RDXY(tgt, STAT_CNT_CFG_##sdx##_STAT_MSB_CNT, idx, id/2, &msb);   \
    vtss_cmn_counter_40_update(lsb, msb, cnt.bytes, clr);               \
    JR_RDXY(tgt, STAT_CNT_CFG_##sdx##_STAT_LSB_CNT, idx, id + 1, &lsb); \
    vtss_cmn_counter_32_update(lsb, cnt.frames, clr);                   \
}

/* Read and update IQS/OQS counters */
#define JR_SQS_CNT_GET(tgt, idx, cnt, clr)                      \
{                                                               \
    u32 lsb, msb;                                               \
    JR_RDXY(tgt, STAT_CNT_CFG_ISDX_STAT_LSB_CNT, idx, 0, &lsb); \
    JR_RDX(tgt, STAT_CNT_CFG_ISDX_STAT_MSB_CNT, idx, &msb);     \
    vtss_cmn_counter_40_update(lsb, msb, cnt.bytes, clr);       \
    JR_RDXY(tgt, STAT_CNT_CFG_ISDX_STAT_LSB_CNT, idx, 1, &lsb); \
    vtss_cmn_counter_32_update(lsb, cnt.frames, clr);           \
}

#define JR_SDX_CNT_ADD(c, counters, name)          \
{                                                  \
    counters->name.frames += c->name.frames.value; \
    counters->name.bytes += c->name.bytes.value;   \
}

static vtss_rc jr_sdx_counters_update(vtss_state_t *vtss_state,
                                      vtss_sdx_entry_t *isdx, vtss_sdx_entry_t *esdx, 
                                      vtss_evc_counters_t *const counters, BOOL clear)
{
    u32                 idx;
    vtss_sdx_counters_t *c;
    
    /* ISDX counters */
    if (isdx != NULL && isdx->port_no != VTSS_PORT_NO_NONE) {
        idx = isdx->sdx;
        c = &vtss_state->evc.sdx_info.sdx_table[idx];
        JR_SDX_CNT_GET(ANA_AC, ISDX, idx, 0, &c->rx_green, clear);
        JR_SDX_CNT_GET(ANA_AC, ISDX, idx, 2, &c->rx_yellow, clear);
        JR_SDX_CNT_GET(ANA_AC, ISDX, idx, 4, &c->rx_red, clear);
        JR_SQS_CNT_GET(IQS, idx, &c->rx_discard, clear);

        if (counters != NULL) {
            JR_SDX_CNT_ADD(c, counters, rx_green);
            JR_SDX_CNT_ADD(c, counters, rx_yellow);
            JR_SDX_CNT_ADD(c, counters, rx_red);
            JR_SDX_CNT_ADD(c, counters, rx_discard);
        }
    }
    

    /* ESDX counters */
    if (esdx != NULL && esdx->port_no != VTSS_PORT_NO_NONE) {
        idx = esdx->sdx;
        c = &vtss_state->evc.sdx_info.sdx_table[idx];
        JR_SQS_CNT_GET(OQS, idx, &c->tx_discard, clear);
        JR_SDX_CNT_GET(REW, ESDX, idx, 0, &c->tx_green, clear);
        JR_SDX_CNT_GET(REW, ESDX, idx, 2, &c->tx_yellow, clear);
        
        if (counters != NULL) {
            /* Add counters */
            JR_SDX_CNT_ADD(c, counters, tx_discard);
            JR_SDX_CNT_ADD(c, counters, tx_green);
            JR_SDX_CNT_ADD(c, counters, tx_yellow);
        }
    }
    return VTSS_RC_OK;
}

/* Allocate SDX, insert in list and return pointer to new entry */
static vtss_rc jr_ece_sdx_alloc(vtss_state_t *vtss_state,
                                vtss_ece_entry_t *ece, vtss_port_no_t port_no, BOOL isdx)
{
    return (vtss_cmn_ece_sdx_alloc(vtss_state, ece, port_no, isdx) == NULL ? VTSS_RC_ERROR :
            VTSS_RC_OK);
}

static vtss_rc jr_evc_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    /* Override tag remark mode for NNI ports */
    return vtss_jr1_qos_port_conf_set(vtss_state, port_no);
}

/* Map policer ID to Jaguar policer index */
static vtss_rc jr_evc_policer_idx(vtss_state_t *vtss_state,
                                  const vtss_evc_policer_id_t policer_id, u32 *idx, BOOL *tdlb)
{
    vtss_rc rc = VTSS_RC_OK;
    
    *tdlb = 0;
    switch (policer_id) {
    case VTSS_EVC_POLICER_ID_NONE:
        *idx = JR_EVC_POLICER_NONE; 
        break;
    case VTSS_EVC_POLICER_ID_DISCARD:
        *idx = JR_EVC_POLICER_DISCARD; 
        break;
    default:
        if (policer_id < VTSS_EVC_POLICERS) {
            *idx = vtss_state->evc.policer_info[policer_id].tdlb_id;
            if (*idx == VTSS_TDLB_ID_NONE) {
                /* SDLB policer */
                *idx = (policer_id + JR_EVC_POLICER_RESV_CNT); /* Skip the reserved policers */
            } else {
                /* TDLB policer */
                *tdlb = 1;
            }
        } else {
            VTSS_E("illegal policer_id: %u", policer_id);
            rc = VTSS_RC_ERROR;
        }
        break;
    }
    return rc;
}

static vtss_vcap_id_t jr_evc_vcap_id(vtss_ece_id_t ece_id, vtss_port_no_t port_no)
{
    vtss_vcap_id_t id = port_no;

    return ((id << 32) + ece_id);
}

/* Update ISDX entry for UNI/NNI */
static vtss_rc jr_ece_isdx_update(vtss_state_t *vtss_state,
                                  vtss_evc_entry_t *evc, vtss_ece_entry_t *ece, 
                                  vtss_sdx_entry_t *isdx)
{
    BOOL                  isdx_ena, tdlb;
    u32                   mask, pol_idx;
    vtss_sdx_entry_t      *esdx;
    vtss_evc_policer_id_t policer_id;

    if (evc->learning) {
        /* VLAN based forwarding */
        isdx_ena = 0;
        mask = 0xffffffff;
    } else {
        /* ISDX based forwarding */
        isdx_ena = 1;
        mask = 0;
        for (esdx = ece->esdx_list; esdx != NULL; esdx = esdx->next) {
            if (esdx->port_no != isdx->port_no) {
                mask |= VTSS_BIT(VTSS_CHIP_PORT(esdx->port_no));
            }
        }
    }
    
    /* Map to policer index */
    if (vtss_state->evc.port_info[isdx->port_no].nni_count &&
        (ece->act_flags & VTSS_ECE_ACT_DIR_NNI_TO_UNI) == 0)
        policer_id = VTSS_EVC_POLICER_ID_NONE;
    else if ((policer_id = ece->policer_id) == VTSS_EVC_POLICER_ID_EVC)
        policer_id = evc->policer_id;
    VTSS_RC(jr_evc_policer_idx(vtss_state, policer_id, &pol_idx, &tdlb));

    JR_WRX(ANA_L2, ISDX_DLB_CFG, isdx->sdx,
           JR_PUT_FLD(ANA_L2, ISDX_DLB_CFG, DLB_IDX, tdlb ? JR_EVC_POLICER_NONE : pol_idx));
    JR_WRX(ANA_L2, ISDX_SERVICE_CTRL, isdx->sdx, 
           JR_PUT_FLD(ANA_L2, ISDX_SERVICE_CTRL, FWD_ADDR, tdlb ? pol_idx : 0) |
           JR_PUT_BIT(ANA_L2, ISDX_SERVICE_CTRL, ES0_ISDX_KEY_ENA, isdx_ena) |
           JR_PUT_BIT(ANA_L2, ISDX_SERVICE_CTRL, CDA_FWD_ENA, 1) |
           JR_PUT_BIT(ANA_L2, ISDX_SERVICE_CTRL, ISDX_BASED_FWD_ENA, 0) |
           JR_PUT_FLD(ANA_L2, ISDX_SERVICE_CTRL, FWD_TYPE, tdlb ? 7 : 0));
    JR_WRX(ANA_L2, ISDX_PORT_MASK_CFG, isdx->sdx, mask);
    
    return VTSS_RC_OK;
}

/* Add IS0 entry for UNI/NNI */
static vtss_rc jr_ece_is0_add(vtss_state_t *vtss_state,
                              vtss_vcap_id_t id, vtss_vcap_id_t id_next,
                              vtss_evc_entry_t *evc, vtss_ece_entry_t *ece, 
                              vtss_sdx_entry_t *isdx)
{
    vtss_is0_entry_t  is0;
    vtss_is0_key_t    *key = &is0.key;
    vtss_is0_action_t *action = &is0.action;
    vtss_vcap_data_t  data;
    vtss_vcap_tag_t   *tag;
    
    memset(&data, 0, sizeof(data));
    memset(&is0, 0, sizeof(is0));
    data.u.is0.entry = &is0; 
    key->port_no = isdx->port_no;
    key->type = VTSS_IS0_TYPE_DBL_VID;

    if (vtss_state->evc.port_info[isdx->port_no].nni_count && 
        (ece->act_flags & VTSS_ECE_ACT_DIR_ONE) == 0) {
        /* NNI port with DIR_BOTH */
        tag = &key->data.dbl_vid.outer_tag;
        tag->tagged = VTSS_VCAP_BIT_1;
        tag->vid.value = evc->vid;
        tag->vid.mask = 0xfff;
        /* NNI->UNI: Match the same PCP as pushed in the UNI->NNI direction.
           Any DEI is matched, because the DEI may be changed by policers */
        if (ece->act_flags & VTSS_ECE_ACT_OT_PCP_MODE_FIXED) {
            /* Fixed PCP/DEI */
            tag->pcp.value = ece->ot_pcp;
            tag->pcp.mask = 0x7;
        } else {
            /* Preserved PCP/DEI */
            tag->pcp = ece->pcp;
        }
        
        /* NNI->UNI: Pop the same number of tags as pushed on the UNI->NNI direction */
        tag = &key->data.dbl_vid.inner_tag;
        if (ece->act_flags & VTSS_ECE_ACT_IT_TYPE_USED) {
            /* Inner tag used */
            action->vlan_pop_cnt = 2;
            tag->tagged = VTSS_VCAP_BIT_1;
            tag->vid.value = ece->it_vid;
            tag->vid.mask = 0xfff;
            if (ece->act_flags & VTSS_ECE_ACT_IT_PCP_MODE_FIXED) {
                /* Fixed PCP/DEI */
                tag->pcp.value = ece->it_pcp;
                tag->pcp.mask = 0x7;
                tag->dei = JR_VCAP_BIT(ece->act_flags & VTSS_ECE_ACT_IT_DEI);
            } else {
                /* Preserved PCP/DEI */
                tag->pcp = ece->pcp;
                tag->dei = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_DEI_VLD,
                                                VTSS_ECE_KEY_TAG_DEI_1);
            }
        } else {
            /* No inner tag pushed */
            action->vlan_pop_cnt = 1;
            
            if (!(ece->act_flags & VTSS_ECE_ACT_POP_1) && 
                (ece->key_flags & VTSS_ECE_KEY_TAG_TAGGED_VLD) &&
                (ece->key_flags & VTSS_ECE_KEY_TAG_TAGGED_1)) {
                /* Tag preserved in UNI->NNI direction, match VID in NNI->UNI direction */
                tag->tagged = VTSS_VCAP_BIT_1;
                tag->s_tag = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_S_TAGGED_VLD,
                                                  VTSS_ECE_KEY_TAG_S_TAGGED_1);
                tag->vid.value = ece->vid.vr.v.value;
                tag->vid.mask = ece->vid.vr.v.mask;
                tag->pcp = ece->pcp;
                tag->dei = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_DEI_VLD,
                                                VTSS_ECE_KEY_TAG_DEI_1);
            }
        }
    } else {
        /* UNI port or unidirectional NNI port */
        
        /* Outer tag */
        tag = &key->data.dbl_vid.outer_tag;
        tag->tagged = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_TAGGED_VLD,
                                           VTSS_ECE_KEY_TAG_TAGGED_1);
        tag->s_tag = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_S_TAGGED_VLD,
                                          VTSS_ECE_KEY_TAG_S_TAGGED_1);
        tag->dei = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_DEI_VLD, VTSS_ECE_KEY_TAG_DEI_1);
        tag->vid.value = ece->vid.vr.v.value;
        tag->vid.mask = ece->vid.vr.v.mask;
        tag->pcp = ece->pcp;

        /* Inner tag */
        tag = &key->data.dbl_vid.inner_tag;
        tag->tagged = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_IN_TAG_TAGGED_VLD,
                                           VTSS_ECE_KEY_IN_TAG_TAGGED_1);
        tag->s_tag = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_IN_TAG_S_TAGGED_VLD,
                                          VTSS_ECE_KEY_IN_TAG_S_TAGGED_1);
        tag->dei = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_IN_TAG_DEI_VLD,
                                        VTSS_ECE_KEY_IN_TAG_DEI_1);
        tag->vid.value = ece->in_vid.vr.v.value;
        tag->vid.mask = ece->in_vid.vr.v.mask;
        tag->pcp = ece->in_pcp;
        
        if (ece->key_flags & VTSS_ECE_KEY_PROT_IPV4) {
            key->data.dbl_vid.proto = VTSS_IS0_PROTO_IPV4;
            key->data.dbl_vid.dscp.value = ece->frame.ipv4.dscp.vr.v.value;
            key->data.dbl_vid.dscp.mask = ece->frame.ipv4.dscp.vr.v.mask;
        } else if (ece->key_flags & VTSS_ECE_KEY_PROT_IPV6) {
            key->data.dbl_vid.proto = VTSS_IS0_PROTO_IPV6;
            key->data.dbl_vid.dscp.value = ece->frame.ipv6.dscp.vr.v.value;
            key->data.dbl_vid.dscp.mask = ece->frame.ipv6.dscp.vr.v.mask;
        }
        if (ece->act_flags & VTSS_ECE_ACT_POP_1)
            action->vlan_pop_cnt = 1;
        else if (ece->act_flags & VTSS_ECE_ACT_POP_2)
            action->vlan_pop_cnt = 2;
        if (ece->act_flags & VTSS_ECE_ACT_OT_PCP_MODE_FIXED) {
            /* Fixed PCP/DEI is used in outer tag ("port tag"), change classification */
            action->pcp_dei_ena = 1;
            action->pcp = ece->ot_pcp;
            action->dei = VTSS_BOOL(ece->act_flags & VTSS_ECE_ACT_OT_DEI);
        }
    }
    action->vid_ena = 1;
    action->vid = (evc->learning ? evc->ivid : 0);
    action->isdx = isdx->sdx;
    action->pag = ece->policy_no;
    return vtss_vcap_add(vtss_state, &vtss_state->vcap.is0.obj, VTSS_IS0_USER_EVC, id, id_next, &data, 0);
}

/* Add ES0 entry for UNI/NNI */
static vtss_rc jr_ece_es0_add(vtss_state_t *vtss_state,
                              vtss_vcap_id_t id, vtss_vcap_id_t id_next,
                              vtss_evc_entry_t *evc, vtss_ece_entry_t *ece, 
                              vtss_sdx_entry_t *esdx)
{
    vtss_vcap_data_t  data;
    vtss_es0_data_t   *es0 = &data.u.es0;
    vtss_es0_entry_t  entry;
    vtss_es0_key_t    *key = &entry.key;
    vtss_es0_action_t *action = &entry.action;
    vtss_port_no_t    port_no = esdx->port_no;
    vtss_sdx_entry_t  *isdx;

    vtss_vcap_es0_init(&data, &entry);
    key->port_no = port_no;
    key->isdx_neq0 = VTSS_VCAP_BIT_1;
    action->esdx = esdx->sdx;
    if (evc->learning) {
        /* VLAN based forwarding */
        key->type = VTSS_ES0_TYPE_VID;
        key->data.vid.vid = evc->ivid;
    } else {
        /* ISDX based forwarding */
        key->type = VTSS_ES0_TYPE_ISDX;
        for (isdx = ece->isdx_list; isdx != NULL; isdx = isdx->next) {
            if (isdx->port_no != port_no) {
                key->data.isdx.isdx = isdx->sdx;
                break;
            }
        }
    } 
    if (vtss_state->evc.port_info[port_no].nni_count) {
        /* NNI port */
        action->vid_a = evc->vid;
        if (ece->act_flags & VTSS_ECE_ACT_IT_TYPE_USED) {
            /* Push two tags ("port tag" is outer and "ES0 tag" is inner) */
            action->tag = VTSS_ES0_TAG_PORT;
            action->vid_b = ece->it_vid;
            action->qos = (ece->act_flags & VTSS_ECE_ACT_IT_PCP_MODE_FIXED ?
                           VTSS_ES0_QOS_ES0 : VTSS_ES0_QOS_CLASS);
            action->pcp = ece->it_pcp;
            action->dei = VTSS_BOOL(ece->act_flags & VTSS_ECE_ACT_IT_DEI);
            action->tpid = (ece->act_flags & VTSS_ECE_ACT_IT_TYPE_C ? VTSS_ES0_TPID_C :
                            ece->act_flags & VTSS_ECE_ACT_IT_TYPE_S ? VTSS_ES0_TPID_S :
                            VTSS_ES0_TPID_PORT);
        } else {
            /* Push one tag ("port tag") */
            action->tag = VTSS_ES0_TAG_NONE;
        }
    } else {
        /* UNI port */
        if ((ece->act_flags & VTSS_ECE_ACT_OT_ENA) ||
            ((ece->act_flags & VTSS_ECE_ACT_POP_1) && 
             (ece->key_flags & VTSS_ECE_KEY_TAG_TAGGED_1) &&
             (ece->vid.vr.v.mask & 0xfff) == 0xfff && ece->vid.vr.v.value != VTSS_VID_NULL)) {
            /* NNI->UNI: Push one tag ("ES0 tag") */
            es0->port_no = port_no;
            es0->flags = VTSS_ES0_FLAG_TPID;
            action->tag = VTSS_ES0_TAG_ES0;
            action->vid_b = (ece->act_flags & VTSS_ECE_ACT_OT_ENA ? ece->ot_vid : 
                             ece->vid.vr.v.value);
            if (ece->act_flags & VTSS_ECE_ACT_OT_PCP_MODE_FIXED) {
                /* Fixed PCP/DEI */
                action->qos = VTSS_ES0_QOS_ES0;
                if (ece->act_flags & VTSS_ECE_ACT_OT_ENA) {
                    /* Direction NNI_TO_UNI */
                    action->pcp = ece->ot_pcp;
                    if (ece->act_flags & VTSS_ECE_ACT_OT_DEI)
                        action->dei = 1;
                } else {
                    /* Direction BOTH, use lowest PCP matching the ingress PCP value/mask */
                    for (action->pcp = 0; action->pcp < 8; action->pcp++)
                        if ((action->pcp & ece->pcp.mask) == (ece->pcp.value & ece->pcp.mask))
                            break;
                    if (ece->key_flags & VTSS_ECE_KEY_TAG_DEI_1)
                        action->dei = 1;
                }
            } else {
                /* Preserved PCP/DEI, use classified values */
                action->qos = VTSS_ES0_QOS_CLASS;
            }
        } else {
            /* NNI->UNI: Do not push any tag */
            action->tag = VTSS_ES0_TAG_NONE;
        }
    }
    return vtss_vcap_add(vtss_state, &vtss_state->vcap.es0.obj, VTSS_ES0_USER_EVC, id, id_next, &data, 0);
}

/* Determine if IS0 rule is needed for UNI/NNI port */
static BOOL jr_ece_is0_needed(BOOL nni, vtss_ece_dir_t dir) 
{
    return (nni ? 
            (dir == VTSS_ECE_DIR_UNI_TO_NNI ? 0 : 1) :
            (dir == VTSS_ECE_DIR_NNI_TO_UNI ? 0 : 1));
}

/* Determine if ES0 rule is needed for UNI/NNI port */
static BOOL jr_ece_es0_needed(BOOL nni, vtss_ece_dir_t dir) 
{
    return (nni ? 
            (dir == VTSS_ECE_DIR_NNI_TO_UNI ? 0 : 1) :
            (dir == VTSS_ECE_DIR_UNI_TO_NNI ? 0 : 1));
}

static vtss_rc jr_ece_update(vtss_state_t *vtss_state,
                             vtss_ece_entry_t *ece, vtss_res_t *res, vtss_res_cmd_t cmd)
{
    vtss_evc_entry_t *evc;
    vtss_ece_id_t    ece_id;
    vtss_ece_entry_t *ece_next;
    vtss_port_no_t   port_no;
    vtss_vcap_id_t   id, id_next_is0, id_next_es0;
    vtss_sdx_entry_t *sdx, *sdx_next;
    BOOL             nni;
    vtss_ece_dir_t   dir;

    if (cmd == VTSS_RES_CMD_CALC) {
        /* Calculate resource usage */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            nni = res->port_nni[port_no];
            if (res->port_add[port_no] || res->port_chg[port_no]) {
                /* Add/change port */
                if (jr_ece_is0_needed(nni, res->dir_new)) {
                    res->is0.add++;
                    res->isdx.add++;
                }
                if (jr_ece_es0_needed(nni, res->dir_new)) {
                    res->es0.add++;
                    res->esdx.add++;
                }
            }
            
            if (res->port_del[port_no] || res->port_chg[port_no]) {
                /* Delete/change port */
                if (jr_ece_is0_needed(nni, res->dir_old)) {
                    res->is0.del++;
                    res->isdx.del++;
                }
                if (jr_ece_es0_needed(nni, res->dir_old)) {
                    res->es0.del++;
                    res->esdx.del++;
                }
            }
        }
        return VTSS_RC_OK;
    }

    evc = &vtss_state->evc.info.table[ece->evc_id];
    ece_id = ece->ece_id;
    
    if (cmd == VTSS_RES_CMD_DEL) {
        /* Delete resources */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            if (res->port_del[port_no] || res->port_chg[port_no]) {
                /* Port deleted or changed (e.g. direction) */
                nni = res->port_nni[port_no];
                id = jr_evc_vcap_id(ece_id, port_no);
                
                /* Delete IS0 entry and ISDX */
                if (jr_ece_is0_needed(nni, res->dir_old)) {
                    VTSS_RC(vtss_vcap_del(vtss_state, &vtss_state->vcap.is0.obj, VTSS_IS0_USER_EVC, id));
                    VTSS_RC(vtss_cmn_ece_sdx_free(vtss_state, ece, port_no, 1));
                }
                
                /* Delete ES0 entry and ESDX */
                if (jr_ece_es0_needed(nni, res->dir_old)) {
                    VTSS_RC(vtss_vcap_del(vtss_state, &vtss_state->vcap.es0.obj, VTSS_ES0_USER_EVC, id));
                    VTSS_RC(vtss_cmn_ece_sdx_free(vtss_state, ece, port_no, 0));
                }
            }
        }
    } else if (cmd == VTSS_RES_CMD_ADD) {
        /* Add resources */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            if (res->port_add[port_no] || res->port_chg[port_no]) {
                /* Allocate ISDX */
                nni = res->port_nni[port_no];
                if (jr_ece_is0_needed(nni, res->dir_new))
                    VTSS_RC(jr_ece_sdx_alloc(vtss_state, ece, port_no, 1));

                /* Allocate ESDX */
                if (jr_ece_es0_needed(nni, res->dir_new))
                    VTSS_RC(jr_ece_sdx_alloc(vtss_state, ece, port_no, 0));
            }
        }
        
        /* Find next VCAP IDs */
        id_next_is0 = VTSS_VCAP_ID_LAST;
        id_next_es0 = VTSS_VCAP_ID_LAST;
        for (ece_next = ece->next; ece_next != NULL; ece_next = ece_next->next) {
            dir = vtss_cmn_ece_dir_get(ece_next);
            for (sdx = ece_next->isdx_list; sdx != NULL && id_next_is0 == VTSS_VCAP_ID_LAST; 
                 sdx = sdx->next) {
                nni = (vtss_state->evc.port_info[sdx->port_no].nni_count ? 1 : 0);
                if (jr_ece_is0_needed(nni, dir))
                    id_next_is0 = jr_evc_vcap_id(ece_next->ece_id, sdx->port_no);
                
            }
            for (sdx = ece_next->esdx_list; sdx != NULL && id_next_es0 == VTSS_VCAP_ID_LAST; 
                 sdx = sdx->next) {
                nni = (vtss_state->evc.port_info[sdx->port_no].nni_count ? 1 : 0);
                if (jr_ece_es0_needed(nni, dir))
                    id_next_es0 = jr_evc_vcap_id(ece_next->ece_id, sdx->port_no);
            }
            if (id_next_is0 != VTSS_VCAP_ID_LAST && id_next_es0 != VTSS_VCAP_ID_LAST)
                break;
        }
        
        /* Add/update IS0, ES0 and ISDX entries */
        sdx_next = NULL;
        while (sdx_next != ece->isdx_list) {
            /* Start with the last ISDX */
            for (sdx = ece->isdx_list; sdx != NULL; sdx = sdx->next) {
                if (sdx->next == sdx_next) {
                    /* Update ISDX entry */
                    VTSS_RC(jr_ece_isdx_update(vtss_state, evc, ece, sdx));

                    /* Add IS0 entry */
                    id = jr_evc_vcap_id(ece_id, sdx->port_no);
                    VTSS_RC(jr_ece_is0_add(vtss_state, id, id_next_is0, evc, ece, sdx));
                    id_next_is0 = id;
                    sdx_next = sdx;
                    break;
                }
            }
        }

        sdx_next = NULL;
        while (sdx_next != ece->esdx_list) {
            /* Start with the last ESDX */
            for (sdx = ece->esdx_list; sdx != NULL; sdx = sdx->next) {
                if (sdx->next == sdx_next) {
                    /* Add ES0 entry */
                    id = jr_evc_vcap_id(ece_id, sdx->port_no);
                    VTSS_RC(jr_ece_es0_add(vtss_state, id, id_next_es0, evc, ece, sdx));
                    id_next_es0 = id;
                    sdx_next = sdx;
                    break;
                }
            }
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_evc_update(vtss_state_t *vtss_state,
                             vtss_evc_id_t evc_id, vtss_res_t *res, vtss_res_cmd_t cmd)
{
    vtss_ece_entry_t *ece;
    
    for (ece = vtss_state->evc.ece_info.used; ece != NULL; ece = ece->next) {
        if (ece->evc_id == evc_id) {
            res->dir_old = vtss_cmn_ece_dir_get(ece);
            res->dir_new = res->dir_old;
            VTSS_RC(jr_ece_update(vtss_state, ece, res, cmd));
        }
    }
    return VTSS_RC_OK;

}

static vtss_rc jr_evc_policer_conf_apply(vtss_state_t *vtss_state,
                                         const vtss_evc_policer_id_t policer_id,
                                         vtss_evc_policer_conf_t *conf)
{
    u32                     i, pol_idx, scale, level, cf, eir;
    u32                     dp_val = 1, gap_val, aware_lvl;
    u64                     rate, interval;
    BOOL                    tdlb = 0, isdx_update = 0;
    u16                     *tdlb_id;
    vtss_evc_entry_t        *evc;
    vtss_ece_entry_t        *ece;
    vtss_sdx_entry_t        *isdx;
    vtss_evc_policer_conf_t cfg = *conf;

    switch (conf->type) {
    case VTSS_POLICER_TYPE_MEF:
        break;
    case VTSS_POLICER_TYPE_SINGLE:
    default:
        /* Single bucket, disable EIR */
        cfg.eir = 0;
        cfg.ebs = 0;
        cfg.cm = 0;
        cfg.cf = 0;
        break;
    }

    /* GAP and color mode */
    gap_val = (cfg.line_rate ? 20 : 0);
    aware_lvl = (cfg.cm ? 0 : 3);
    cf = (cfg.cm && cfg.cf ? 1 : 0);

    /* Calculate scale based on maximum rate */
    eir = cfg.eir;
    if (cf)
        eir += cfg.cir;
    rate = MAX(eir, cfg.cir);

    /* CIR/EIR are in kbps */
    if (rate > 1000000) {
        /* More than 1 Gbps, use TDLB policer */
        tdlb = 1;
        scale = 0;
        interval = 8196721;
    } else if (rate > 90000) {
        /* More than 90 Mbps */
        scale = 0;
        interval = 512032;
    } else if (rate >= 3000) {
        /* More than 3 Mbps */
        scale = 1;
        interval = 64004;
    } else {
        /* Less than 3 Mbps */
        scale = 2;
        interval = (rate == 1000 ? 8130 : 8000); /* Adjust for 1 Mbps */
    }
    
    if (policer_id < VTSS_EVC_POLICERS) {
        tdlb_id = &vtss_state->evc.policer_info[policer_id].tdlb_id;
        if (tdlb) {
            /* Allocate tunnel policer */
            for (i = 0; i < VTSS_JR1_TDLB_CNT && *tdlb_id == VTSS_TDLB_ID_NONE; i++) {
                if (vtss_state->evc.tdlb_used[i] == 0) {
                    vtss_state->evc.tdlb_used[i] = 1;
                    *tdlb_id = i;
                    isdx_update = 1;
                }
            }
            if (*tdlb_id == VTSS_TDLB_ID_NONE) {
                VTSS_E("no more TDLB policers for policer_id %u", policer_id);
                return VTSS_RC_ERROR;
            }
        } else if (*tdlb_id != VTSS_TDLB_ID_NONE) {
            /* Free tunnel policer */
            vtss_state->evc.tdlb_used[*tdlb_id] = 0;
            *tdlb_id = VTSS_TDLB_ID_NONE;
            isdx_update = 1;
        }

        /* Update ISDX table */
        for (ece = vtss_state->evc.ece_info.used; isdx_update && ece != NULL; ece = ece->next) {
            evc = &vtss_state->evc.info.table[ece->evc_id];
            if (ece->policer_id == policer_id || evc->policer_id == policer_id) {
                for (isdx = ece->isdx_list; isdx != NULL; isdx = isdx->next) {
                    VTSS_RC(jr_ece_isdx_update(vtss_state, evc, ece, isdx));
                }
            }
        }
    }

    /* Map to policer index */
    VTSS_RC(jr_evc_policer_idx(vtss_state, policer_id, &pol_idx, &tdlb));

    if (tdlb) {
        JR_WRX(ANA_AC, TDLB_DLB_CFG, pol_idx,
               JR_PUT_BIT(ANA_AC, TDLB_DLB_CFG, COUPLING_MODE, cfg.cf) |
               JR_PUT_FLD(ANA_AC, TDLB_DLB_CFG, COLOR_AWARE_LVL, aware_lvl) |
               JR_PUT_FLD(ANA_AC, TDLB_DLB_CFG, CIR_INC_DP_VAL, dp_val) |
               JR_PUT_FLD(ANA_AC, TDLB_DLB_CFG, GAP_VAL, gap_val) |
               JR_PUT_FLD(ANA_AC, TDLB_DLB_CFG, TIMESCALE_VAL, scale));
    } else {
        JR_WRX(ANA_AC, SDLB_DLB_CFG, pol_idx,
               JR_PUT_BIT(ANA_AC, SDLB_DLB_CFG, COUPLING_MODE, cfg.cf) |
               JR_PUT_FLD(ANA_AC, SDLB_DLB_CFG, COLOR_AWARE_LVL, aware_lvl) |
               JR_PUT_FLD(ANA_AC, SDLB_DLB_CFG, CIR_INC_DP_VAL, dp_val) |
               JR_PUT_FLD(ANA_AC, SDLB_DLB_CFG, GAP_VAL, gap_val) |
               JR_PUT_FLD(ANA_AC, SDLB_DLB_CFG, TIMESCALE_VAL, scale));
    }

    /* Setup CIR/PIR policers */
    for (i = 0; i < 2; i++) {
        if (i == 0) {
            rate = cfg.cir;
            level = cfg.cbs;
        } else{
            rate = eir;
            level = cfg.ebs;
        }
        rate = VTSS_DIV64((rate * 1000), interval);
        if (rate > 0x7ff || cfg.enable == 0)
            rate = 0x7ff;
        level /= 2048;
        if (level > 0x7f)
            level = 0x7f;
        else if (level == 0)
            level = 1;
        if (tdlb) {
            JR_WRXY(ANA_AC, TDLB_LB_CFG, pol_idx, i, 
                    JR_PUT_FLD(ANA_AC, TDLB_LB_CFG, THRES_VAL, level) |
                    JR_PUT_FLD(ANA_AC, TDLB_LB_CFG, RATE_VAL, rate));
        } else {
            JR_WRXY(ANA_AC, SDLB_LB_CFG, pol_idx, i, 
                    JR_PUT_FLD(ANA_AC, SDLB_LB_CFG, THRES_VAL, level) |
                    JR_PUT_FLD(ANA_AC, SDLB_LB_CFG, RATE_VAL, rate));
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_evc_policer_conf_set(vtss_state_t *vtss_state,
                                       const vtss_evc_policer_id_t policer_id)
{
    return jr_evc_policer_conf_apply(vtss_state, policer_id, &vtss_state->qos.evc_policer_conf[policer_id]);
}

/* - Debug print --------------------------------------------------- */

static void jr_debug_sdx_cnt(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr, const char *name,  
                             vtss_chip_counter_pair_t *c1, vtss_chip_counter_pair_t *c2)
{
    char buf[80];
    int  bytes;

    for (bytes = 0; bytes < 2; bytes++) {
        sprintf(buf, "%s_%s", name, bytes ? "bytes" : "frames");
        vtss_jr1_debug_cnt(vtss_state, pr, buf, c2 == NULL ? NULL : "", 
                           bytes ? &c1->bytes : &c1->frames, 
                           c2 == NULL ? 0 : (bytes ? &c2->bytes : &c2->frames));
    }
}

#define JR_DEBUG_SDX_NAME(pr, tgt, sdx, idx, name)                                        \
{                                                                                         \
    JR_DEBUG_REGX_NAME(pr, tgt, STAT_GLOBAL_CFG_##sdx##_STAT_GLOBAL_CFG, idx, name":CFG"); \
    JR_DEBUG_REGX_NAME(pr, tgt, STAT_GLOBAL_CFG_##sdx##_STAT_GLOBAL_EVENT_MASK, idx, name":EVENT_MASK"); \
}

static vtss_rc jr_debug_evc(vtss_state_t *vtss_state,
                            const vtss_debug_printf_t pr,
                            const vtss_debug_info_t   *const info)
{
    /*lint --e{454, 455} */ // Due to VTSS_EXIT_ENTER
    vtss_ece_entry_t    *ece;
    vtss_sdx_entry_t    *isdx, *esdx;
    vtss_sdx_counters_t *icnt, *ecnt, zero_cnt;
    vtss_port_no_t      port_no;
    u32                 value, i, j, tdlb_id;
    BOOL                header = 1;
    char                buf[80], *p;

    /* VCAPs */
    VTSS_RC(vtss_jr1_debug_vcap_is0(vtss_state, pr, info));
    VTSS_RC(vtss_jr1_debug_vcap_es0(vtss_state, pr, info));

    vtss_jr1_debug_reg_header(pr, "REW");
    vtss_jr1_debug_reg(vtss_state, pr, VTSS_REW_COMMON_SERVICE_CTRL, "SERVICE_CTRL");
    pr("\n");

    /* ISDX table */
    for (ece = vtss_state->evc.ece_info.used; ece != NULL; ece = ece->next) {
        for (isdx = ece->isdx_list; isdx != NULL; isdx = isdx->next) {
            if (header)
                pr("ISDX  ECE ID  Port  FWD_TYPE  ISDX_FWD  CDA_FWD  ES0_ISDX  FWD_ADDR  POL_IDX\n");
            header = 0;
            JR_RDX(ANA_L2, ISDX_SERVICE_CTRL, isdx->sdx, &value);
            pr("%-6u%-8u%-6u%-10u%-10u%-9u%-10u%-10u",
               isdx->sdx,
               ece->ece_id, 
               VTSS_CHIP_PORT(isdx->port_no), 
               JR_GET_FLD(ANA_L2, ISDX_SERVICE_CTRL, FWD_TYPE, value),
               JR_GET_BIT(ANA_L2, ISDX_SERVICE_CTRL, ISDX_BASED_FWD_ENA, value),
               JR_GET_BIT(ANA_L2, ISDX_SERVICE_CTRL, CDA_FWD_ENA, value),
               JR_GET_BIT(ANA_L2, ISDX_SERVICE_CTRL, ES0_ISDX_KEY_ENA, value),
               JR_GET_FLD(ANA_L2, ISDX_SERVICE_CTRL, FWD_ADDR, value));
            JR_RDX(ANA_L2, ISDX_DLB_CFG, isdx->sdx, &value);
            pr("%u\n", value);
            VTSS_EXIT_ENTER();
        }
    }
    if (!header)
        pr("\n");

    /* SDX counter setup */
    vtss_jr1_debug_reg_header(pr, "GLOBAL_CFG");
    for (i = 0; i < 6; i++) {
        JR_DEBUG_SDX_NAME(pr, ANA_AC, ISDX, i, "ANA");
    }
    for (i = 0; i < 2; i++) {
        JR_DEBUG_SDX_NAME(pr, IQS, ISDX, i, "IQS");
    }
    for (i = 0; i < 2; i++) {
        JR_DEBUG_SDX_NAME(pr, OQS, ISDX, i, "OQS");
    }
    for (i = 0; i < 4; i++) {
        JR_DEBUG_SDX_NAME(pr, REW, ESDX, i, "REW");
    }
    pr("\n");
    
    memset(&zero_cnt, 0, sizeof(zero_cnt));
    for (ece = vtss_state->evc.ece_info.used; ece != NULL; ece = ece->next) {
        icnt = &zero_cnt;
        ecnt = &zero_cnt;
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            for (isdx = ece->isdx_list; isdx != NULL; isdx = isdx->next) {
                if (isdx->port_no == port_no) {
                    icnt = &vtss_state->evc.sdx_info.sdx_table[isdx->sdx];
                    break;
                }
            }
            for (esdx = ece->esdx_list; esdx != NULL; esdx = esdx->next) {
                if (esdx->port_no == port_no) {
                    ecnt = &vtss_state->evc.sdx_info.sdx_table[esdx->sdx];
                    break;
                }
            }
            if (isdx == NULL && esdx == NULL)
                continue;
            VTSS_RC(jr_sdx_counters_update(vtss_state, isdx, esdx, NULL, 0));
            pr("Counters for ECE ID %u, port %u (chip_port: %u), ISDX: %u, ESDX: %u\n", 
               ece->ece_id, port_no, VTSS_CHIP_PORT(port_no), 
               isdx == NULL ? 0 : isdx->sdx, esdx == NULL ? 0 : esdx->sdx);
            jr_debug_sdx_cnt(vtss_state, pr, "green", &icnt->rx_green, &ecnt->tx_green);
            jr_debug_sdx_cnt(vtss_state, pr, "yellow", &icnt->rx_yellow, &ecnt->tx_yellow);
            jr_debug_sdx_cnt(vtss_state, pr, "red", &icnt->rx_red, NULL);
            jr_debug_sdx_cnt(vtss_state, pr, "discard", &icnt->rx_discard, &ecnt->tx_discard);
            pr("\n");
            VTSS_EXIT_ENTER();
        }
    }

    /* Policer table */
    p = (buf + sprintf(buf, "Policer "));
    for (i = 0; i < VTSS_EVC_POLICERS + JR_EVC_POLICER_RESV_CNT; i++) {
        tdlb_id = VTSS_TDLB_ID_NONE;
        if (i == JR_EVC_POLICER_NONE)
            strcpy(p, "None");
        else if (i == JR_EVC_POLICER_DISCARD)
            strcpy(p, "Discard");
        else {
            j = (i - JR_EVC_POLICER_RESV_CNT);
            if (vtss_state->qos.evc_policer_conf[j].enable)
                sprintf(p, "%u" , j);
            else
                continue;
            tdlb_id = vtss_state->evc.policer_info[j].tdlb_id;
        }
        vtss_jr1_debug_reg_header(pr, buf);
        if (tdlb_id == VTSS_TDLB_ID_NONE) {
            JR_DEBUG_REGX_NAME(pr, ANA_AC, SDLB_DLB_CFG, i, "SDLB_CFG");
            JR_DEBUG_REGXY_NAME(pr, ANA_AC, SDLB_LB_CFG, i, 0, "LB_CFG_0");
            JR_DEBUG_REGXY_NAME(pr, ANA_AC, SDLB_LB_CFG, i, 1, "LB_CFG_1");
        } else {
            JR_DEBUG_REGX_NAME(pr, ANA_AC, TDLB_DLB_CFG, tdlb_id, "TDLB_CFG");
            JR_DEBUG_REGXY_NAME(pr, ANA_AC, TDLB_LB_CFG, tdlb_id, 0, "LB_CFG_0");
            JR_DEBUG_REGXY_NAME(pr, ANA_AC, TDLB_LB_CFG, tdlb_id, 1, "LB_CFG_1");
        }
        pr("\n");
        VTSS_EXIT_ENTER();
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_evc_debug_print(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_EVC, jr_debug_evc, vtss_state, pr, info);
}

/* - Initialization ------------------------------------------------ */

vtss_rc vtss_jr1_evc_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_evc_state_t        *state = &vtss_state->evc;
    vtss_evc_policer_conf_t pol_conf;
    vtss_sdx_info_t         *sdx_info = &state->sdx_info;
    vtss_sdx_entry_t        *isdx, *esdx;
    u32                     i, idx;
    
    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->port_conf_set = jr_evc_port_conf_set;
        vtss_state->qos.evc_policer_conf_set = jr_evc_policer_conf_set;
        state->add = vtss_cmn_evc_add;
        state->del = vtss_cmn_evc_del;
        state->ece_add = vtss_cmn_ece_add;
        state->ece_del= vtss_cmn_ece_del;
        state->ece_counters_get = vtss_cmn_ece_counters_get;
        state->ece_counters_clear = vtss_cmn_ece_counters_clear;
        state->counters_get = vtss_cmn_evc_counters_get;
        state->counters_clear = vtss_cmn_evc_counters_clear;
        state->sdx_counters_update = jr_sdx_counters_update;
        state->ece_update = jr_ece_update;
        state->update = jr_evc_update;
        state->info.max_count = VTSS_EVCS;
        state->ece_info.max_count = VTSS_ECES;
        state->sdx_info.max_count = VTSS_JR1_SDX_CNT;
        break;
    case VTSS_INIT_CMD_INIT:
        /* Setup EVC open and discard policer */
        memset(&pol_conf, 0, sizeof(pol_conf));
        pol_conf.cm = 1; /* Colour aware to make statistics show incoming colour */
        VTSS_RC(jr_evc_policer_conf_apply(vtss_state, VTSS_EVC_POLICER_ID_NONE, &pol_conf));
        pol_conf.enable = 1;
        VTSS_RC(jr_evc_policer_conf_apply(vtss_state, VTSS_EVC_POLICER_ID_DISCARD, &pol_conf));

        /* Change SDLB granularity */
        JR_WRF(ANA_AC, COMMON_SDLB_DLB_CTRL, BASE_TICK_CNT, 3906);
        break;
    case VTSS_INIT_CMD_PORT_MAP:
    {
#if defined(VTSS_OPT_EVC_OAM_SDX_ZERO)
        /* Ensure that all OAM frames get SDX zero */
        vtss_vcap_data_t  data;
        vtss_is1_data_t   *is1 = &data.is1;
        vtss_is1_entry_t  entry;
        vtss_is1_action_t *action = &entry.action;
        vtss_is1_key_t    *key = &entry.key;

        memset(&data, 0, sizeof(data));
        memset(&entry, 0, sizeof(entry));
        is1->vid_range = VTSS_VCAP_RANGE_CHK_NONE;
        is1->dscp_range = VTSS_VCAP_RANGE_CHK_NONE;
        is1->sport_range = VTSS_VCAP_RANGE_CHK_NONE;
        is1->dport_range = VTSS_VCAP_RANGE_CHK_NONE;
        is1->entry = &entry;
        is1->first = 1;
        action->isdx_enable = 1;
        key->type = VTSS_IS1_TYPE_ETYPE;
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++)
            key->port_list[port_no] = 1;
        key->frame.etype.etype.value[0] = 0x89;
        key->frame.etype.etype.value[1] = 0x02;
        key->frame.etype.etype.mask[0] = 0xff;
        key->frame.etype.etype.mask[1] = 0xff;
        VTSS_RC(vtss_vcap_add(vtss_state, &vtss_state->is1.obj, VTSS_IS1_USER_EVC, 
                              1, VTSS_VCAP_ID_LAST, &data, 0));
#endif /* VTSS_OPT_EVC_OAM_SDX_ZERO */
        break;
    }
    case VTSS_INIT_CMD_POLL:
        /* Poll counters for 20 SDX entries, giving 4095/20 = 205 seconds between each poll.
           This ensures that any counter can wrap only once between each poll.
           The worst case is a 32-bit frame counter on a 10Gbps port, which takes about
           0xffffffff/14.880.000.000 = 288 seconds to wrap. */
        for (i = 0; i < 20; i++) {
            idx = sdx_info->poll_idx;
            isdx = &sdx_info->isdx.table[idx];
            esdx = &sdx_info->esdx.table[idx];
            idx++;
            sdx_info->poll_idx = (idx < sdx_info->max_count ? idx : 0);
            VTSS_RC(jr_sdx_counters_update(vtss_state, isdx, esdx, NULL, 0));
        }
        break;
    default:
        break;
    }
    
    return VTSS_RC_OK;
}

#endif /* VTSS_FEATURE_EVC */

#endif /* VTSS_ARCH_JAGUAR_1 */
