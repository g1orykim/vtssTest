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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_EVC
#include "vtss_serval_cil.h"

#if defined(VTSS_ARCH_SERVAL)
#if defined(VTSS_FEATURE_EVC)
/* - CIL functions ------------------------------------------------- */

static vtss_rc srvl_isdx_table_idle(vtss_state_t *vtss_state)
{
    u32 value;
    
    do {
        SRVL_RD(VTSS_ANA_ANA_TABLES_ISDXACCESS, &value);
    } while (VTSS_X_ANA_ANA_TABLES_ISDXACCESS_ISDX_TBL_CMD(value) != ISDX_CMD_IDLE);

    return VTSS_RC_OK;
}

static vtss_rc srvl_isdx_update(vtss_state_t *vtss_state,
                                BOOL isdx_ena, u32 isdx, u32 isdx_mask, u32 pol_idx, u32 voe_idx)
{
    /* Write to ISDX table */
    SRVL_WR(VTSS_ANA_ANA_TABLES_ISDXTIDX,
            (isdx_ena ? VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_ES0_KEY_ENA : 0) |
            VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_SDLBI(pol_idx) |
            VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_INDEX(isdx) |
            (isdx_ena ? VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_FORCE_ENA : 0));
    SRVL_WR(VTSS_ANA_ANA_TABLES_ISDXACCESS,
            VTSS_F_ANA_ANA_TABLES_ISDXACCESS_ISDX_PORT_MASK(isdx_mask) |
            VTSS_F_ANA_ANA_TABLES_ISDXACCESS_ISDX_TBL_CMD(ISDX_CMD_WRITE));
    
    /* Write VOE mapping to IPT table */
    SRVL_WR(VTSS_ANA_IPT_OAM_MEP_CFG(isdx),
            VTSS_F_ANA_IPT_OAM_MEP_CFG_MEP_IDX(voe_idx) |
            VTSS_F_ANA_IPT_OAM_MEP_CFG_MEP_IDX_P(voe_idx) |
            (voe_idx == VTSS_EVC_VOE_IDX_NONE ? 0 : VTSS_F_ANA_IPT_OAM_MEP_CFG_MEP_IDX_ENA));
    
    return srvl_isdx_table_idle(vtss_state);
}

/* ================================================================= *
 *  MCE
 * ================================================================= */

static vtss_vcap_key_size_t srvl_port_key_get(vtss_state_t *vtss_state,
                                              vtss_port_no_t       port_no,
                                              u8                   lookup,
                                              vtss_vcap_key_type_t *type)
{
    vtss_vcap_key_type_t key_type;

    if (lookup) {
        /* Second lookup */
        key_type = vtss_state->vcap.port_conf[port_no].key_type_is1_1;
    } else {
        /* First lookup */
        key_type = vtss_state->evc.port_conf[port_no].key_type;
    }

    if (type != NULL)
        *type = key_type;

    return vtss_vcap_key_type_to_size(key_type);
}

static vtss_vcap_key_size_t srvl_evc_port_key_get(vtss_state_t *vtss_state,
                                                  vtss_port_no_t       port_no,
                                                  vtss_ece_entry_t     *ece,
                                                  vtss_vcap_key_type_t *type)
{
    return srvl_port_key_get(vtss_state, port_no, ece->key_flags & VTSS_ECE_KEY_LOOKUP ? 1 : 0, type);
}

vtss_rc vtss_srvl_isdx_update_es0(vtss_state_t *vtss_state,
                                  BOOL isdx_ena, u32 isdx, u32 isdx_mask)
{
    /* Write to ISDX table */
    SRVL_WRM(VTSS_ANA_ANA_TABLES_ISDXTIDX,
             (isdx_ena ? VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_ES0_KEY_ENA : 0) |
             VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_INDEX(isdx)                  |
             (isdx_ena ? VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_FORCE_ENA : 0),
             VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_ES0_KEY_ENA |
             VTSS_M_ANA_ANA_TABLES_ISDXTIDX_ISDX_INDEX       |
             VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_FORCE_ENA);
    SRVL_WR(VTSS_ANA_ANA_TABLES_ISDXACCESS,
            VTSS_F_ANA_ANA_TABLES_ISDXACCESS_ISDX_PORT_MASK(isdx_mask) |
            VTSS_F_ANA_ANA_TABLES_ISDXACCESS_ISDX_TBL_CMD(ISDX_CMD_WRITE));

    return srvl_isdx_table_idle(vtss_state);
}

static void srvl_mce_sdx_realloc(vtss_state_t *vtss_state,
                                 vtss_mce_entry_t *mce, vtss_port_no_t port_no, BOOL isdx)
{
    vtss_sdx_entry_t *sdx, **list = (isdx ? &mce->isdx_list : &mce->esdx_list);

    if ((sdx = *list) != NULL) {
        if (sdx->port_no == port_no) {
            /* Reallocate */
            port_no = VTSS_PORT_NO_NONE;
        } else {
            /* Free */
            vtss_cmn_sdx_free(vtss_state, sdx, isdx);
            *list = NULL;
        }
    }
    if (port_no != VTSS_PORT_NO_NONE) {
        /* Allocate */
        *list = vtss_cmn_sdx_alloc(vtss_state, port_no, isdx);
    }
}

static void srvl_mce2is1_tag(const vtss_mce_tag_t *mce_tag, vtss_is1_tag_t *is1_tag)
{
    is1_tag->tagged = mce_tag->tagged;
    is1_tag->s_tag = mce_tag->s_tagged;
    is1_tag->vid.type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
    is1_tag->vid.vr.v.value = mce_tag->vid.value;
    is1_tag->vid.vr.v.mask = mce_tag->vid.mask;
    is1_tag->pcp = mce_tag->pcp;
    is1_tag->dei = mce_tag->dei;
}

static vtss_vcap_key_size_t srvl_mce_port_key_get(vtss_state_t         *vtss_state,
                                                  vtss_port_no_t       port_no,
                                                  const vtss_mce_t     *mce,
                                                  vtss_vcap_key_type_t *type)
{
    return srvl_port_key_get(vtss_state, port_no, mce->key.lookup, type);
}

static vtss_vcap_id_t srvl_mce_vcap_id(vtss_mce_id_t mce_id, vtss_port_no_t port_no)
{
    vtss_vcap_id_t id = port_no;

    return ((id << 32) + mce_id);
}

vtss_rc vtss_srvl_mce_is1_add(vtss_state_t *vtss_state, vtss_mce_entry_t *mce)
{
    vtss_vcap_data_t  data;
    vtss_is1_entry_t  is1;
    vtss_is1_key_t    *key = &is1.key;
    vtss_is1_action_t *action = &is1.action;
    vtss_mce_t        *conf = &mce->conf;
    vtss_mce_entry_t  *cur;
    vtss_port_no_t    port_no;
    u8                value, mask;

    vtss_vcap_is1_init(&data, &is1);
    data.u.is1.lookup = (conf->key.lookup ? 1 : 0); /* First/second lookup */
    port_no = vtss_cmn_first_port_no_get(vtss_state, conf->key.port_list, 0);
    data.key_size = srvl_mce_port_key_get(vtss_state, port_no, conf, &key->key_type);
    key->type = !conf->key.service_detect ? VTSS_IS1_TYPE_ETYPE : VTSS_IS1_TYPE_ANY;
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        key->port_list[port_no] = conf->key.port_list[port_no];
    }
    srvl_mce2is1_tag(&conf->key.tag, &key->tag);
    srvl_mce2is1_tag(&conf->key.inner_tag, &key->inner_tag);

    if (!conf->key.service_detect) {    /* Only have OAM detection in key if not service detection required */
        /* ETYPE field encoding */
        key->frame.etype.etype.value[0] = 0x89;
        key->frame.etype.etype.mask[0] = 0xff;
        if (vtss_state->misc.chip_id.revision == 0) {
            /* Revision A */
            value = 0x02;
            mask = 0xff;
        } else {
            /* Revision B or later */
            key->isdx.value[0] = 0x02; /* ISDX bit 9 indicates OAM frame */
            key->isdx.mask[0] = 0x02;
            value = (conf->key.mel.value & 0x7f);
            mask = (conf->key.mel.mask & 0x7f);
            if (conf->key.injected != VTSS_VCAP_BIT_ANY) {
                if (conf->key.injected == VTSS_VCAP_BIT_1)
                    value |= (1 << 7);
                mask |= (1 << 7);
            }
        }
        key->frame.etype.etype.value[1] = value;
        key->frame.etype.etype.mask[1] = mask;
    }

    /* MAC header matching */
    key->mac.dmac = conf->key.dmac;
    key->mac.dmac_mc = conf->key.dmac_mc;

    if (conf->action.pop_cnt < VTSS_MCE_POP_NONE) {
        action->pop_enable = 1;
        action->pop = conf->action.pop_cnt;
    }
    if (conf->action.vid != VTSS_VID_NULL) {
        action->vid_enable = 1;
        action->vid = conf->action.vid;
    }
    action->isdx_enable = (conf->action.isdx == VTSS_MCE_ISDX_NONE ? 0 : 1);
    action->isdx = (mce->isdx_list == NULL ? 0 : mce->isdx_list->sdx);
    if (conf->action.prio_enable) {
        action->prio_enable = 1;
        action->prio = conf->action.prio;
        action->pcp_dei_enable = 1;
        action->pcp = conf->action.prio;
    }
    if (conf->action.policy_no != VTSS_ACL_POLICY_NO_NONE) {
        action->pag_enable = 1;
        action->pag = conf->action.policy_no;
    }
    action->oam_detect = conf->action.oam_detect;

    /* Find next ID */
    for (cur = mce->next; cur != NULL; cur = cur->next) {
        if (vtss_cmn_first_port_no_get(vtss_state, cur->conf.key.port_list, 0) < vtss_state->port_count)
            break;
    }
    return vtss_vcap_add(vtss_state, &vtss_state->vcap.is1.obj, VTSS_IS1_USER_MEP,
                         mce->conf.id, cur == NULL ? VTSS_VCAP_ID_LAST : cur->conf.id, &data, 0);
}

static BOOL srvl_mce_isdx_reuse(vtss_isdx_t isdx)
{
    return (isdx == 0 || isdx == VTSS_MCE_ISDX_NONE || isdx == VTSS_MCE_ISDX_NEW ? 0 : 1);
}

static vtss_rc srvl_mce_add(vtss_state_t *vtss_state,
                            const vtss_mce_id_t mce_id, const vtss_mce_t *const mce)
{
    const vtss_mce_key_t    *key = &mce->key;
    const vtss_mce_action_t *action = &mce->action;
    vtss_mce_action_t       old_action;
    vtss_mce_info_t         *mce_info = &vtss_state->evc.mce_info;
    vtss_mce_entry_t        *cur, *prev = NULL;
    vtss_mce_entry_t        *old = NULL, *old_prev = NULL, *ins = NULL, *ins_prev = NULL;
    vtss_vcap_key_size_t    key_size;
    vtss_res_t              res;
    vtss_vcap_obj_t         *is1_obj = &vtss_state->vcap.is1.obj;
    vtss_vcap_obj_t         *es0_obj = &vtss_state->vcap.es0.obj;
    vtss_vcap_data_t        data;
    vtss_es0_entry_t        es0_entry;
    vtss_es0_key_t          *es0_key = &es0_entry.key;
    vtss_es0_action_t       *es0_action = &es0_entry.action;
    vtss_es0_tag_conf_t     *ot = &es0_action->outer_tag;
    vtss_sdx_entry_t        *isdx = NULL, *isdx_old = NULL, *esdx;
    vtss_port_no_t          port_no, port_rx, port_tx;
    u32                     isdx_mask, voe_idx;
    vtss_vcap_id_t          id, id_next;

    VTSS_D("mce_id: %u", mce_id);

    /* Check MCE ID */
    if (mce->id == VTSS_MCE_ID_LAST || mce->id == mce_id) {
        VTSS_E("illegal mce id: %u", mce->id);
        return VTSS_RC_ERROR;
    }

    /* Search for existing entry and place to add */
    for (cur = mce_info->used; cur != NULL; prev = cur, cur = cur->next) {
        if (cur->conf.id == mce->id) {
            /* Entry already exists */
            old_prev = prev;
            old = cur;
        }
        
        if (cur->conf.id == mce_id) {
            /* Found insertion point */
            ins_prev = prev;
            ins = cur;
        }

        if (cur->isdx_list != NULL && cur->isdx_list->sdx == action->isdx) {
            /* Found ISDX, which can be cloned */
            isdx = cur->isdx_list;
        }
    }
    if (mce_id == VTSS_MCE_ID_LAST)
        ins_prev = prev;

    /* Check if the place to insert was found */
    if (ins == NULL && mce_id != VTSS_MCE_ID_LAST) {
        VTSS_E("could not find mce ID: %u", mce_id);
        return VTSS_RC_ERROR;
    }
    
    /* Check if cloned ISDX was found */
    if (srvl_mce_isdx_reuse(action->isdx) && isdx == NULL) {
        VTSS_E("could not reuse ISDX %u", action->isdx);
        return VTSS_RC_ERROR;
    }

    /* Check that resources are available */
    vtss_cmn_res_init(&res);
    memset(&old_action, 0, sizeof(old_action));
    if (old != NULL) {
        old_action = old->conf.action;
        /* Old resources */
        if (old->isdx_list != NULL && old->conf.action.isdx == VTSS_MCE_ISDX_NEW) {
            res.isdx.del++;
        }
        if (old->esdx_list != NULL) {
            res.esdx.del++;
        }
        port_no = vtss_cmn_first_port_no_get(vtss_state, old->conf.key.port_list, 0);
        if (port_no < vtss_state->port_count) {
            key_size = srvl_mce_port_key_get(vtss_state, port_no, &old->conf, NULL);
            res.is1.del_key[key_size]++;
        }
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            if (old->conf.action.port_list[port_no])
                res.es0.del++;
        }
    }

    /* New/changed resources */
    port_rx = vtss_cmn_first_port_no_get(vtss_state, key->port_list, key->port_cpu);
    port_tx = vtss_cmn_first_port_no_get(vtss_state, action->port_list, 0);
    if (port_rx < vtss_state->port_count) {
        key_size = srvl_mce_port_key_get(vtss_state, port_rx, mce, NULL);
        res.is1.add_key[key_size]++;
    }
    if (action->isdx == VTSS_MCE_ISDX_NEW && (port_rx < vtss_state->port_count || port_rx == VTSS_PORT_NO_CPU)) {
        res.isdx.add++;
    }
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (action->port_list[port_no])
            res.es0.add++;
    }
    if (res.es0.add)
        res.esdx.add++;

    /* Check resource availability */
    VTSS_RC(vtss_cmn_res_check(vtss_state, &res));

    cur = old;
    if (cur == NULL) {
        /* Take entry from free list */
        if ((cur = mce_info->free) == NULL) {
            VTSS_E("no more MCEs");
            return VTSS_RC_ERROR;
        }
        mce_info->free = cur->next;
        mce_info->count++;
    } else {
        /* Take existing entry out of list */
        if (ins_prev == cur)
            ins_prev = old_prev;
        if (old_prev == NULL)
            mce_info->used = cur->next;
        else
            old_prev->next = cur->next;
        if (srvl_mce_isdx_reuse(cur->conf.action.isdx))
            cur->isdx_list = NULL;     /* Delete old cloned ISDX */
        else
            isdx_old = cur->isdx_list; /* Remember old allocated ISDX */
    }

    /* Insert new entry in list */
    if (ins_prev == NULL) {
        cur->next = mce_info->used;
        mce_info->used = cur;
    } else {
        cur->next = ins_prev->next;
        ins_prev->next = cur;
    }
    cur->conf = *mce;

    /* Free/allocate SDX resources */
    srvl_mce_sdx_realloc(vtss_state, cur, action->isdx == VTSS_MCE_ISDX_NEW ? port_rx :
                         VTSS_PORT_NO_NONE, 1);
    if (isdx != NULL)
        cur->isdx_list = isdx;
    isdx = cur->isdx_list;
    srvl_mce_sdx_realloc(vtss_state, cur, port_tx, 0);
    esdx = cur->esdx_list;

    /* Add/delete IS1 entry */
    voe_idx = (action->voe_idx == VTSS_OAM_VOE_IDX_NONE ? VTSS_EVC_VOE_IDX_NONE : action->voe_idx);
    if (port_rx < vtss_state->port_count) {
        VTSS_RC(vtss_srvl_mce_is1_add(vtss_state, cur));
        if (action->isdx == VTSS_MCE_ISDX_NEW && isdx != NULL) {
            /* ISDX owner, update ISDX table */
            isdx_mask = (action->tx_lookup == VTSS_MCE_TX_LOOKUP_VID ? 0xffffffff :
                         vtss_srvl_port_mask(vtss_state, action->port_list));
            VTSS_RC(srvl_isdx_update(vtss_state, action->tx_lookup == VTSS_MCE_TX_LOOKUP_VID ? 0 : 1,
                                     isdx->sdx, isdx_mask, 0, voe_idx));
        }
    } else {
        VTSS_RC(vtss_vcap_del(vtss_state, is1_obj, VTSS_IS1_USER_MEP, mce->id));
    }

    /* Find next ID for first ES0 entry */
    id_next = VTSS_VCAP_ID_LAST;
    for (cur = cur->next; cur != NULL; cur = cur->next) {
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            if (cur->conf.action.port_list[port_no] && cur->isdx_list != NULL && cur->esdx_list != NULL) {
                id_next = srvl_mce_vcap_id(cur->conf.id, port_no);
                break;
            }
        }
    }

    /* Add/delete ES0 entries, start from last port to avoid VCAP move operations */
    for (port_no = (vtss_state->port_count - 1); ;port_no--) {
        id = srvl_mce_vcap_id(mce->id, port_no);
        if (action->port_list[port_no] && (action->rule != VTSS_MCE_RULE_RX) && (esdx != NULL) && ((action->tx_lookup == VTSS_MCE_TX_LOOKUP_VID) || (isdx != NULL))) {
            vtss_vcap_es0_init(&data, &es0_entry);
            es0_key->port_no = port_no;
            if ((action->tx_lookup == VTSS_MCE_TX_LOOKUP_VID) && (isdx == NULL))
                es0_key->isdx_neq0 = VTSS_VCAP_BIT_0;   /* VID based TX lookup without ISDX -> ES0 hit on not service frame */
            else
                es0_key->isdx_neq0 = VTSS_VCAP_BIT_1;
            if (action->tx_lookup == VTSS_MCE_TX_LOOKUP_VID) {
                es0_key->type = VTSS_ES0_TYPE_VID;
                es0_key->data.vid.vid = action->vid;
            } else {    /* Lookup is NOT VID. According to the above 'if' statement 'isdx' must be != NULL. Lint is complaining anyway leading to extra check of 'isdx' */
                if (isdx != NULL) {
                    es0_key->type = VTSS_ES0_TYPE_ISDX;
                    es0_key->data.isdx.isdx = isdx->sdx;
                }
            }
            if (action->tx_lookup == VTSS_MCE_TX_LOOKUP_ISDX_PCP && action->prio_enable) {
                es0_key->data.isdx.pcp.value = action->prio;
                es0_key->data.isdx.pcp.mask = 0x7;
            }
            if (action->outer_tag.enable) {
                /* Add ES0 tag */
                data.u.es0.flags = VTSS_ES0_FLAG_TPID;
                data.u.es0.port_no = port_no;
                ot->tag = VTSS_ES0_TAG_ES0;
                ot->tpid = VTSS_ES0_TPID_PORT;
                ot->vid.sel = 1;
                ot->vid.val = action->outer_tag.vid;
                ot->pcp.sel = (action->outer_tag.pcp_mode == VTSS_MCE_PCP_MODE_FIXED ?
                               VTSS_ES0_PCP_ES0 : VTSS_ES0_PCP_MAPPED);
                ot->pcp.val = action->outer_tag.pcp;
                ot->dei.sel = (action->outer_tag.dei_mode == VTSS_MCE_DEI_MODE_FIXED ?
                               VTSS_ES0_DEI_ES0 : VTSS_ES0_DEI_DP);
                ot->dei.val = action->outer_tag.dei;
            } else {
                ot->tag = VTSS_ES0_TAG_PORT;
            }
            es0_action->esdx = esdx->sdx;
            if (voe_idx != VTSS_EVC_VOE_IDX_NONE) {
                es0_action->mep_idx_enable = 1;
                es0_action->mep_idx = voe_idx;
            }
            VTSS_RC(vtss_vcap_add(vtss_state, es0_obj, VTSS_ES0_USER_MEP, id, id_next, &data, 0));
            id_next = id;
        } else if (old_action.port_list[port_no]) {
            VTSS_RC(vtss_vcap_del(vtss_state, es0_obj, VTSS_ES0_USER_MEP, id));
        }
        if (port_no == VTSS_PORT_NO_START)
            break;
    }

    if (isdx == NULL && isdx_old != NULL) {
        /* Remove references to deleted ISDX */
        for (cur = mce_info->used; cur != NULL; cur = cur->next) {
            if (cur->isdx_list == isdx_old)
                cur->isdx_list = NULL;
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_mce_del(vtss_state_t *vtss_state, const vtss_mce_id_t mce_id)
{
    vtss_mce_info_t  *mce_info = &vtss_state->evc.mce_info;
    vtss_mce_entry_t *mce, *prev = NULL;
    vtss_vcap_id_t   id;
    vtss_port_no_t   port_no;
    
    VTSS_D("mce_id: %u", mce_id);
    
    /* Find MCE */
    for (mce = mce_info->used; mce != NULL; prev = mce, mce = mce->next) {
        if (mce->conf.id == mce_id) {
            break;
        }
    }

    /* Check if MCE was found */
    if (mce == NULL) {
        VTSS_E("could not find mce ID: %u", mce_id);
        return VTSS_RC_ERROR;
    }

    /* Free SDX resources */
    if (mce->conf.action.isdx == VTSS_MCE_ISDX_NEW) {
        /* Own ISDX */
        srvl_mce_sdx_realloc(vtss_state, mce, VTSS_PORT_NO_NONE, 1);
    }
    mce->isdx_list = NULL;
    srvl_mce_sdx_realloc(vtss_state, mce, VTSS_PORT_NO_NONE, 0);

    /* Free IS1 and ES0 entries */
    VTSS_RC(vtss_vcap_del(vtss_state, &vtss_state->vcap.is1.obj, VTSS_IS1_USER_MEP, mce_id));
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        if (mce->conf.action.port_list[port_no]) {
            id = srvl_mce_vcap_id(mce_id, port_no);
            VTSS_RC(vtss_vcap_del(vtss_state, &vtss_state->vcap.es0.obj, VTSS_ES0_USER_MEP, id));
        }
    }

    /* Move MCE from used to free list */
    mce_info->count--;
    if (prev == NULL)
        mce_info->used = mce->next;
    else
        prev->next = mce->next;
    mce->next = mce_info->free;
    mce_info->free = mce;

    return VTSS_RC_OK;
}

static vtss_rc srvl_mce_port_get(vtss_state_t *vtss_state,
                                 const vtss_mce_id_t mce_id, const vtss_port_no_t port_no,
                                 vtss_mce_port_info_t *const info)

{
    vtss_mce_entry_t *mce;
    vtss_sdx_entry_t *isdx;
    
    VTSS_D("mce_id: %u, port_no: %u", mce_id, port_no);

    /* Find MCE */
    for (mce = vtss_state->evc.mce_info.used; mce != NULL; mce = mce->next) {
        if (mce->conf.id == mce_id) {
            break;
        }
    }

    if (mce == NULL) {
        VTSS_E("could not find mce ID: %u", mce_id);
        return VTSS_RC_ERROR;
    }
    
    isdx = mce->isdx_list;
    if (isdx == NULL) {
        VTSS_E("could not find ISDX for mce ID: %u, port_no: %u",
                mce_id, port_no);
        return VTSS_RC_ERROR;
    }
    info->isdx = isdx->sdx;

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  EVC
 * ================================================================= */

static vtss_vcap_id_t srvl_evc_vcap_id(vtss_ece_id_t ece_id, vtss_port_no_t port_no)
{
    vtss_vcap_id_t id = port_no;

    return ((id << 32) + ece_id);
}

/* Determine if IS1 rule is needed for UNI/NNI port */
BOOL vtss_srvl_ece_is1_needed(BOOL nni, vtss_ece_dir_t dir, vtss_ece_rule_t rule)
{
    return (nni ? 
            /* NNI port */
            (dir == VTSS_ECE_DIR_UNI_TO_NNI ? 0 :
             dir == VTSS_ECE_DIR_NNI_TO_UNI ? (rule == VTSS_ECE_RULE_TX ? 0 : 1) :
             (rule == VTSS_ECE_RULE_RX ? 0 : 1)) :
            /* UNI port */
            (dir == VTSS_ECE_DIR_NNI_TO_UNI ? 0 :
             dir == VTSS_ECE_DIR_UNI_TO_NNI ? (rule == VTSS_ECE_RULE_TX ? 0 : 1) :
             (rule == VTSS_ECE_RULE_TX ? 0 : 1)));
}

/* Determine if ES0 rule is needed for UNI/NNI port */
static BOOL srvl_ece_es0_needed(BOOL nni, vtss_ece_dir_t dir, vtss_ece_rule_t rule) 
{
    return (nni ? 
            /* NNI port */
            (dir == VTSS_ECE_DIR_NNI_TO_UNI ? 0 :
             dir == VTSS_ECE_DIR_UNI_TO_NNI ? (rule == VTSS_ECE_RULE_RX ? 0 : 1) :
             (rule == VTSS_ECE_RULE_RX ? 0 : 1)) :
            /* UNI port */
            (dir == VTSS_ECE_DIR_UNI_TO_NNI ? 0 :
             dir == VTSS_ECE_DIR_NNI_TO_UNI ? (rule == VTSS_ECE_RULE_RX ? 0 : 1) :
             (rule == VTSS_ECE_RULE_TX ? 0 : 1)));
}

static vtss_rc srvl_evc_range_alloc(vtss_vcap_range_chk_table_t *range, vtss_ece_entry_t *ece,
                                    vtss_is1_data_t *is1)
{
    vtss_vcap_vr_t *dscp, *sport, *dport;

    VTSS_RC(vtss_vcap_vr_alloc(range, &is1->vid_range, VTSS_VCAP_RANGE_TYPE_VID, &ece->vid));
    if (ece->key_flags & (VTSS_ECE_KEY_PROT_IPV4 | VTSS_ECE_KEY_PROT_IPV6)) {
        if (ece->key_flags & VTSS_ECE_KEY_PROT_IPV4) {
            dscp = &ece->frame.ipv4.dscp;
            sport = &ece->frame.ipv4.sport; 
            dport = &ece->frame.ipv4.dport; 
        } else {
            dscp = &ece->frame.ipv6.dscp;
            sport = &ece->frame.ipv6.sport; 
            dport = &ece->frame.ipv6.dport; 
        }
        VTSS_RC(vtss_vcap_vr_alloc(range, &is1->dscp_range, VTSS_VCAP_RANGE_TYPE_DSCP, dscp));
        VTSS_RC(vtss_vcap_vr_alloc(range, &is1->sport_range, VTSS_VCAP_RANGE_TYPE_SPORT, sport));
        VTSS_RC(vtss_vcap_vr_alloc(range, &is1->dport_range, VTSS_VCAP_RANGE_TYPE_DPORT, dport));
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_evc_range_free(vtss_state_t *vtss_state,
                                   vtss_vcap_range_chk_table_t *range, vtss_vcap_id_t id)
{
    vtss_vcap_data_t data;
    vtss_is1_data_t  *is1 = &data.u.is1;
    
    if (vtss_vcap_lookup(vtss_state, &vtss_state->vcap.is1.obj, VTSS_IS1_USER_EVC, id,
                         &data, NULL) == VTSS_RC_OK) {
        VTSS_RC(vtss_vcap_range_free(range, is1->vid_range));
        VTSS_RC(vtss_vcap_range_free(range, is1->dscp_range));
        VTSS_RC(vtss_vcap_range_free(range, is1->sport_range));
        VTSS_RC(vtss_vcap_range_free(range, is1->dport_range));
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_ece_isdx_update(vtss_state_t *vtss_state,
                                    vtss_evc_entry_t *evc, vtss_ece_entry_t *ece, 
                                    vtss_sdx_entry_t *isdx)
{
    BOOL             isdx_ena = (ece->act_flags & VTSS_ECE_ACT_TX_LOOKUP_ISDX ? 1 : 0);
    BOOL             nni = VTSS_PORT_BF_GET(evc->ports, isdx->port_no);
    u32              isdx_mask = (isdx_ena ? 0 : 0xffffffff), pol_idx, voe_idx;
    vtss_sdx_entry_t *esdx;
    
    VTSS_D("ece_id: %u, isdx: %u, port_no: %u",
            ece->ece_id, isdx->sdx, isdx->port_no);

    /* Find ISDX port mask */
    for (esdx = ece->esdx_list; isdx_ena && esdx != NULL; esdx = esdx->next) {
        if (esdx->port_no != isdx->port_no) {
            isdx_mask |= VTSS_BIT(VTSS_CHIP_PORT(esdx->port_no));
        }
    }
    
    /* Map to policer */
    if (nni && vtss_cmn_ece_dir_get(ece) == VTSS_ECE_DIR_BOTH) {
        /* NNI port with DIR_BOTH, no policing */
        pol_idx = VTSS_EVC_POLICER_ID_NONE;
    } else if ((pol_idx = ece->policer_id) == VTSS_EVC_POLICER_ID_EVC)
        pol_idx = evc->policer_id;

    if (pol_idx == VTSS_EVC_POLICER_ID_DISCARD)
        pol_idx = (SRVL_POLICER_DISCARD - SRVL_POLICER_EVC + 1);
    else if (pol_idx < vtss_state->qos.evc_policer_max)
        pol_idx++;
    else
        pol_idx = 0;

    voe_idx = evc->voe_idx[isdx->port_no];
    return srvl_isdx_update(vtss_state, isdx_ena, isdx->sdx, isdx_mask, pol_idx, voe_idx);
}

static vtss_rc srvl_sdx_counters_update(vtss_state_t *vtss_state,
                                        vtss_sdx_entry_t *isdx, vtss_sdx_entry_t *esdx, 
                                        vtss_evc_counters_t *const counters, BOOL clear)
{
    u32                 base, *p = &base;
    vtss_sdx_counters_t *c;
    
    /* ISDX counters */
    if (isdx != NULL && isdx->port_no < vtss_state->port_count) {
        /* Setup counter view */
        SRVL_WR(VTSS_SYS_SYSTEM_STAT_CFG, VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(isdx->sdx));
        
        c = &vtss_state->evc.sdx_info.sdx_table[isdx->sdx];
        base = 0xc0;
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_green.bytes, clear));
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_green.frames, clear));
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_yellow.bytes, clear));
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_yellow.frames, clear));
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_red.bytes, clear));
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_red.frames, clear));
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_discard.bytes, clear));  /* drop_green */
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->rx_discard.frames, clear)); /* drop_green */
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_discard.bytes, clear));  /* drop_yellow */
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_discard.frames, clear)); /* drop_yellow */

        if (counters != NULL) {
            /* Add counters */
            counters->rx_green.bytes += c->rx_green.bytes.value;
            counters->rx_green.frames += c->rx_green.frames.value;
            counters->rx_yellow.bytes += c->rx_yellow.bytes.value;
            counters->rx_yellow.frames += c->rx_yellow.frames.value;
            counters->rx_red.bytes += c->rx_red.bytes.value;
            counters->rx_red.frames += c->rx_red.frames.value;
            counters->rx_discard.bytes += (c->rx_discard.bytes.value + c->tx_discard.bytes.value);
            counters->rx_discard.frames += (c->rx_discard.frames.value + c->tx_discard.frames.value);
        }
    }

    /* ESDX counters */
    if (esdx != NULL && esdx->port_no < vtss_state->port_count) {
        /* Setup counter view */
        SRVL_WR(VTSS_SYS_SYSTEM_STAT_CFG, VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(esdx->sdx));
        
        c = &vtss_state->evc.sdx_info.sdx_table[esdx->sdx];
        base = 0x100;
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_green.bytes, clear));
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_green.frames, clear));
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_yellow.bytes, clear));
        VTSS_RC(vtss_srvl_counter_update(vtss_state, p, &c->tx_yellow.frames, clear));
        
        if (counters != NULL) {
            /* Add counters */
            counters->tx_green.bytes += c->tx_green.bytes.value;
            counters->tx_green.frames += c->tx_green.frames.value;
            counters->tx_yellow.bytes += c->tx_yellow.bytes.value;
            counters->tx_yellow.frames += c->tx_yellow.frames.value;
        }
    }
    
    return VTSS_RC_OK;
}

static vtss_rc srvl_ece_sdx_alloc(vtss_state_t *vtss_state,
                                  vtss_ece_entry_t *ece, vtss_port_no_t port_no, BOOL isdx)
{
    return (vtss_cmn_ece_sdx_alloc(vtss_state, ece, port_no, isdx) == NULL ? VTSS_RC_ERROR : VTSS_RC_OK);
}

static void srvl_ece_nni_is1_tag_set(vtss_ece_entry_t *ece, vtss_is1_tag_t *tag, vtss_vid_t vid, 
                                     u8 pcp_val, vtss_vcap_u8_t *pcp_key, 
                                     u32 mask_pcp_fixed, u32 mask_pcp_mapped,
                                     u32 mask_dei, u32 mask_dei_fixed, u32 mask_dei_dp)
{
    /* NNI->UNI: Match the same tag as pushed in the UNI->NNI direction */
    
    /* Match any tag if VID is zero */
    if (vid == VTSS_VID_NULL)
        return;

    /* Match VID */
    tag->tagged = VTSS_VCAP_BIT_1;
    tag->vid.vr.v.value = vid;
    tag->vid.vr.v.mask = 0xfff;

    if (ece->act_flags & mask_pcp_fixed) {
        /* Fixed PCP */
        tag->pcp.value = pcp_val;
        tag->pcp.mask = 0x7;
    } else if (ece->act_flags & mask_pcp_mapped) {
        /* Mapped (QOS, DP), match any PCP */
    } else {
        /* Preserved PCP */
        tag->pcp = *pcp_key;
    }

    if (ece->act_flags & mask_dei_fixed) {
        /* Fixed DEI */
        tag->dei = (ece->act_flags & mask_dei ? VTSS_VCAP_BIT_1 : VTSS_VCAP_BIT_0);
    } else if (ece->act_flags & mask_dei_dp) {
        /* DP based DEI, match any DEI */
        tag->dei = VTSS_VCAP_BIT_ANY;
    } else {
        /* Preserved DEI */
        tag->dei = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_DEI_VLD, VTSS_ECE_KEY_TAG_DEI_1);
    }
}

static vtss_ece_tx_lookup_t srvl_tx_lookup_get(vtss_ece_entry_t *ece)
{
    u32 flags = ece->act_flags;
    
    return ((flags & VTSS_ECE_ACT_TX_LOOKUP_VID_PCP) ? VTSS_ECE_TX_LOOKUP_VID_PCP :
            (flags & VTSS_ECE_ACT_TX_LOOKUP_ISDX) ? VTSS_ECE_TX_LOOKUP_ISDX : 
            VTSS_ECE_TX_LOOKUP_VID);
}

static vtss_rc srvl_ece_is1_add(vtss_state_t *vtss_state,
                                vtss_vcap_id_t id, vtss_vcap_id_t id_next,
                                vtss_evc_entry_t *evc, vtss_ece_entry_t *ece, 
                                vtss_sdx_entry_t *isdx)
{
    vtss_vcap_data_t            data;
    vtss_is1_entry_t            is1;
    vtss_is1_key_t              *key = &is1.key;
    vtss_is1_mac_t              *mac = &key->mac;
    vtss_is1_tag_t              *tag = &key->tag;
    vtss_is1_frame_ipv4_t       *ipv4 = &key->frame.ipv4;
    vtss_is1_frame_ipv6_t       *ipv6 = &key->frame.ipv6;
    vtss_is1_action_t           *action = &is1.action;
    vtss_vcap_range_chk_table_t range_new = vtss_state->vcap.range;
    vtss_port_no_t              port_no = isdx->port_no;
    BOOL                        prio_enable = (ece->act_flags & VTSS_ECE_ACT_PRIO_ENA ? 1 : 0);
    BOOL                        nni = VTSS_PORT_BF_GET(evc->ports, port_no);
    vtss_ece_dir_t              dir = vtss_cmn_ece_dir_get(ece);
    vtss_ece_tx_lookup_t        tx_lookup = srvl_tx_lookup_get(ece);

    VTSS_D("id: %s, id_next: %s",
            vtss_vcap_id_txt(vtss_state, id), vtss_vcap_id_txt(vtss_state, id_next));

    /* Free old range checkers */
    VTSS_RC(srvl_evc_range_free(vtss_state, &range_new, id));
    
    vtss_vcap_is1_init(&data, &is1);
    data.u.is1.lookup = (ece->key_flags & VTSS_ECE_KEY_LOOKUP ? 1 : 0);

    data.key_size = srvl_evc_port_key_get(vtss_state, port_no, ece, &key->key_type);
    key->type = VTSS_IS1_TYPE_ANY;
    key->port_list[port_no] = 1;
    
    action->pop_enable = 1;
    if (nni && dir == VTSS_ECE_DIR_BOTH) {
        /* NNI port with DIR_BOTH */

        /* Outer tag matching */
        srvl_ece_nni_is1_tag_set(ece, tag, evc->vid, ece->ot_pcp, &ece->pcp,
                                 VTSS_ECE_ACT_OT_PCP_MODE_FIXED, 
                                 VTSS_ECE_ACT_OT_PCP_MODE_MAPPED, 
                                 VTSS_ECE_ACT_OT_DEI, 
                                 VTSS_ECE_ACT_OT_DEI_MODE_FIXED,
                                 VTSS_ECE_ACT_OT_DEI_MODE_DP);
        
        /* Pop outer tag if it was pushed in the other direction */
        if (evc->vid != VTSS_VID_NULL) {
            action->pop = 1;
            tag = &key->inner_tag;
        }

        /* NNI->UNI: Pop the same number of tags as pushed in the UNI->NNI direction */
        if (tx_lookup == VTSS_ECE_TX_LOOKUP_ISDX) {
            /* ISDX-based ES0 lookup, inner tag popping/matching may be done */
            if (ece->act_flags & VTSS_ECE_ACT_IT_TYPE_USED) {
                /* Inner tag used */
                action->pop++;
                srvl_ece_nni_is1_tag_set(ece, tag, ece->it_vid, ece->it_pcp, &ece->pcp,
                                         VTSS_ECE_ACT_IT_PCP_MODE_FIXED,
                                         VTSS_ECE_ACT_IT_PCP_MODE_MAPPED,
                                         VTSS_ECE_ACT_IT_DEI, 
                                         VTSS_ECE_ACT_IT_DEI_MODE_FIXED,
                                         VTSS_ECE_ACT_IT_DEI_MODE_DP);
            } else {
                /* No inner tag pushed */
                if (!(ece->act_flags & VTSS_ECE_ACT_POP_1) && 
                    (ece->key_flags & VTSS_ECE_KEY_TAG_TAGGED_VLD) &&
                    (ece->key_flags & VTSS_ECE_KEY_TAG_TAGGED_1) &&
                    ece->vid.type == VTSS_VCAP_VR_TYPE_VALUE_MASK) {
                    /* Tag preserved in UNI->NNI direction, match tag in NNI->UNI direction */
                    tag->tagged = VTSS_VCAP_BIT_1;
                    tag->s_tag = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_S_TAGGED_VLD,
                                                      VTSS_ECE_KEY_TAG_S_TAGGED_1);
                    tag->vid = ece->vid;
                    tag->pcp = ece->pcp;
                    tag->dei = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_DEI_VLD,
                                                    VTSS_ECE_KEY_TAG_DEI_1);
                }
            }
        } else if (tx_lookup == VTSS_ECE_TX_LOOKUP_VID) {
            /* VID-based ES0 lookup, I-NNI assumed */
            prio_enable = 0; /* Use basic classification */
        }

    } else {
        /* UNI port or unidirectional NNI port */
        
        /* Allocate range checkers for UNI/NNIs */
        VTSS_RC(srvl_evc_range_alloc(&range_new, ece, &data.u.is1));

        /* MAC header matching */
        mac->dmac_mc = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_DMAC_MC_VLD, VTSS_ECE_KEY_DMAC_MC_1);
        mac->dmac_bc = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_DMAC_BC_VLD, VTSS_ECE_KEY_DMAC_BC_1);
        mac->smac = ece->smac;
        mac->dmac = ece->dmac;
        
        /* Outer tag matching */
        tag->vid = ece->vid;
        tag->pcp = ece->pcp;
        tag->dei = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_DEI_VLD, VTSS_ECE_KEY_TAG_DEI_1);
        tag->tagged = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_TAGGED_VLD,
                                           VTSS_ECE_KEY_TAG_TAGGED_1);
        tag->s_tag = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_S_TAGGED_VLD,
                                          VTSS_ECE_KEY_TAG_S_TAGGED_1);

        /* Inner tag matching */
        tag = &key->inner_tag;
        tag->vid = ece->in_vid;
        tag->pcp = ece->in_pcp;
        tag->dei = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_IN_TAG_DEI_VLD,
                                        VTSS_ECE_KEY_IN_TAG_DEI_1);
        tag->tagged = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_IN_TAG_TAGGED_VLD,
                                           VTSS_ECE_KEY_IN_TAG_TAGGED_1);
        tag->s_tag = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_IN_TAG_S_TAGGED_VLD,
                                          VTSS_ECE_KEY_IN_TAG_S_TAGGED_1);
        
        /* Frame type specific matching */
        if (ece->key_flags & VTSS_ECE_KEY_PROT_ETYPE) {
            key->type = VTSS_IS1_TYPE_ETYPE;
            key->frame.etype.etype = ece->frame.etype.etype;
            key->frame.etype.data = ece->frame.etype.data;
        } else if (ece->key_flags & VTSS_ECE_KEY_PROT_LLC) {
            key->type = VTSS_IS1_TYPE_LLC;
            key->frame.llc.data = ece->frame.llc.data;
        } else if (ece->key_flags & VTSS_ECE_KEY_PROT_SNAP) {
            key->type = VTSS_IS1_TYPE_SNAP;
            key->frame.snap.data = ece->frame.snap.data;
        } else if (ece->key_flags & VTSS_ECE_KEY_PROT_IPV4) {
            key->type = VTSS_IS1_TYPE_IPV4;
            ipv4->fragment = ece->frame.ipv4.fragment;
            ipv4->proto = ece->frame.ipv4.proto;
            ipv4->dscp = ece->frame.ipv4.dscp;
            ipv4->sip = ece->frame.ipv4.sip;
            ipv4->dip = ece->frame.ipv4.dip;
            ipv4->sport = ece->frame.ipv4.sport;
            ipv4->dport = ece->frame.ipv4.dport;
        } else if (ece->key_flags & VTSS_ECE_KEY_PROT_IPV6) {
            key->type = VTSS_IS1_TYPE_IPV6;
            ipv6->proto = ece->frame.ipv6.proto;
            ipv6->dscp = ece->frame.ipv6.dscp;
            ipv6->sip = ece->frame.ipv6.sip;
            ipv6->dip = ece->frame.ipv6.dip;
            ipv6->sport = ece->frame.ipv6.sport;
            ipv6->dport = ece->frame.ipv6.dport;
        }

        /* Tag pop count */
        if (ece->act_flags & VTSS_ECE_ACT_POP_1)
            action->pop = 1;
        else if (ece->act_flags & VTSS_ECE_ACT_POP_2)
            action->pop = 2;
    }

#if defined(VTSS_FEATURE_MPLS)
    vtss_mpls_ece_is1_update(vtss_state, evc, ece, isdx, key);
#endif /* VTSS_FEATURE_MPLS */

    /* Commit range checkers */
    VTSS_RC(vtss_vcap_range_commit(vtss_state, &range_new));

    action->vid_enable = 1;
    action->vid = evc->ivid;
    action->isdx_enable = 1;
    action->isdx = isdx->sdx;
    if (!(ece->act_flags & VTSS_ECE_ACT_POLICY_NONE)) {
        action->pag_enable = 1;
        action->pag = ece->policy_no;
    }

    if (nni && dir == VTSS_ECE_DIR_NNI_TO_UNI && 
        tx_lookup == VTSS_ECE_TX_LOOKUP_VID_PCP && ece->pcp.mask == 0) {
        /* Unidirectional rule for NNI port with (VID, PCP) egress lookup and matching any PCP.
           It is assumed that this is an I-NNI port using basic classification */
        prio_enable = 0;
    }

    if (prio_enable) {
        action->prio_enable = 1;
        action->prio = ece->prio;
        action->pcp_dei_enable = 1; /* Use COS = IFH.PCP = IFH.QOS */
        action->pcp = ece->prio;
    }
    if (ece->act_flags & VTSS_ECE_ACT_DP_ENA) {
        action->dp_enable = 1;
        action->dp = ece->dp;
    }
    return vtss_vcap_add(vtss_state, &vtss_state->vcap.is1.obj, VTSS_IS1_USER_EVC,
                         id, id_next, &data, 0);
}

static void srvl_ece_es0_tag_set(vtss_ece_entry_t *ece, vtss_es0_tag_conf_t *tag,
                                 vtss_vid_t vid, u8 pcp, u32 mask_pcp_fixed, u32 mask_pcp_mapped, 
                                 u32 mask_dei_fixed, u32 mask_dei_dp, u32 mask_dei)
{
    /* Do not add tag if VID is zero */
    if (vid == VTSS_VID_NULL)
        return;

    tag->tag = VTSS_ES0_TAG_ES0;
    tag->tpid = VTSS_ES0_TPID_PORT;
    tag->vid.sel = 1;
    tag->vid.val = vid;
    tag->pcp.sel = ((ece->act_flags & mask_pcp_fixed) ? VTSS_ES0_PCP_ES0 : 
                    (ece->act_flags & mask_pcp_mapped) ? VTSS_ES0_PCP_MAPPED : VTSS_ES0_PCP_CLASS);
    tag->pcp.val = pcp;
    tag->dei.sel = ((ece->act_flags & mask_dei_fixed) ? VTSS_ES0_DEI_ES0 :
                    (ece->act_flags & mask_dei_dp) ? VTSS_ES0_DEI_DP : VTSS_ES0_DEI_CLASS);
    tag->dei.val = (ece->act_flags & mask_dei ? 1 : 0);
}

static vtss_rc srvl_ece_es0_add(vtss_state_t *vtss_state,
                                vtss_vcap_id_t id, vtss_vcap_id_t id_next,
                                vtss_evc_entry_t *evc, vtss_ece_entry_t *ece, 
                                vtss_sdx_entry_t *esdx)
{
    vtss_vcap_data_t     data;
    vtss_es0_data_t      *es0 = &data.u.es0;
    vtss_es0_entry_t     entry;
    vtss_es0_key_t       *key = &entry.key;
    vtss_es0_action_t    *action = &entry.action;
    vtss_es0_tag_conf_t  *tag = &action->outer_tag;
    vtss_port_no_t       port_no = esdx->port_no;
    vtss_sdx_entry_t     *isdx;
    u8                   pcp, mep_idx;
    BOOL                 pcp_map, nni = VTSS_PORT_BF_GET(evc->ports, port_no);
    vtss_ece_dir_t       dir = vtss_cmn_ece_dir_get(ece);
    vtss_ece_tx_lookup_t tx_lookup = srvl_tx_lookup_get(ece);

    VTSS_D("id: %s, id_next: %s",
            vtss_vcap_id_txt(vtss_state, id), vtss_vcap_id_txt(vtss_state, id_next));

    vtss_vcap_es0_init(&data, &entry);
    es0->port_no = port_no;
    es0->flags = VTSS_ES0_FLAG_TPID;
    key->port_no = port_no;
    action->esdx = esdx->sdx;
    if (tx_lookup == VTSS_ECE_TX_LOOKUP_ISDX) {
        /* ISDX based forwarding */
        key->isdx_neq0 = VTSS_VCAP_BIT_1;
        key->type = VTSS_ES0_TYPE_ISDX;
        for (isdx = ece->isdx_list; isdx != NULL; isdx = isdx->next) {
            if (isdx->port_no != port_no) {
                key->data.isdx.isdx = isdx->sdx;
                break;
            }
        }
    } else {
        /* VLAN based forwarding */
        key->type = VTSS_ES0_TYPE_VID;
        key->data.vid.vid = evc->ivid;
        if (ece->act_flags & VTSS_ECE_ACT_PRIO_ENA) {
            pcp_map = 0;
            if (tx_lookup == VTSS_ECE_TX_LOOKUP_VID_PCP) {
                if (!nni && dir == VTSS_ECE_DIR_NNI_TO_UNI && ece->pcp.mask == 0) {
                    /* UNI port for unidirectional rule matching any PCP: I-NNI assumed */
                    pcp_map = 1;
                } else {
                    /* Match (VID, PCP), where PCP = COS */
                    key->data.vid.pcp.value = ece->prio;
                    key->data.vid.pcp.mask = 0x7;
                }
            } else if (!nni && dir == VTSS_ECE_DIR_BOTH &&
                       (ece->act_flags & VTSS_ECE_ACT_OT_PCP_MODE_MAPPED)) {
                /* UNI port for bidirectional rule with VID lookup and mapped PCP: I-NNI assumed */
                pcp_map = 1;
            }
            if (pcp_map) {
                /* On the I-NNI, the PCP value mapping to the QOS may be used as key */
                for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
                    if (VTSS_PORT_BF_GET(evc->ports, port_no)) {
                        /* NNI port found, indicate that PCP mapping must be used */
                        es0->flags |= VTSS_ES0_FLAG_PCP_MAP;
                        es0->prio = ece->prio;
                        es0->nni = port_no;
                        break;
                    }
                }
            }
        }
    } 
    mep_idx = evc->voe_idx[esdx->port_no];
    if (mep_idx != VTSS_EVC_VOE_IDX_NONE) {
        action->mep_idx_enable = 1;
        action->mep_idx = mep_idx;
    }

    if (nni) {
        /* NNI port */

        /* Outer tag */
        srvl_ece_es0_tag_set(ece, tag, evc->vid, ece->ot_pcp, 
                             VTSS_ECE_ACT_OT_PCP_MODE_FIXED, VTSS_ECE_ACT_OT_PCP_MODE_MAPPED,
                             VTSS_ECE_ACT_OT_DEI_MODE_FIXED, VTSS_ECE_ACT_OT_DEI_MODE_DP,
                             VTSS_ECE_ACT_OT_DEI);

        /* Inner tag */
        if (ece->act_flags & VTSS_ECE_ACT_IT_TYPE_USED) {
            tag = &action->inner_tag;
            srvl_ece_es0_tag_set(ece, tag, ece->it_vid, ece->it_pcp, 
                                 VTSS_ECE_ACT_IT_PCP_MODE_FIXED, VTSS_ECE_ACT_IT_PCP_MODE_MAPPED,
                                 VTSS_ECE_ACT_IT_DEI_MODE_FIXED, VTSS_ECE_ACT_IT_DEI_MODE_DP,
                                 VTSS_ECE_ACT_IT_DEI);
            if (ece->act_flags & VTSS_ECE_ACT_IT_TYPE_C)
                tag->tpid = VTSS_ES0_TPID_C;
            else if (ece->act_flags & VTSS_ECE_ACT_IT_TYPE_S)
                tag->tpid = VTSS_ES0_TPID_S;
        }
    } else if (ece->act_flags & VTSS_ECE_ACT_OT_ENA) {
        /* UNI port, direction NNI_TO_UNI, push one tag */
        srvl_ece_es0_tag_set(ece, tag, ece->ot_vid, ece->ot_pcp,
                             VTSS_ECE_ACT_OT_PCP_MODE_FIXED, VTSS_ECE_ACT_OT_PCP_MODE_MAPPED,
                             VTSS_ECE_ACT_OT_DEI_MODE_FIXED, VTSS_ECE_ACT_OT_DEI_MODE_DP,
                             VTSS_ECE_ACT_OT_DEI);
    } else if (dir == VTSS_ECE_DIR_BOTH &&
               (ece->act_flags & VTSS_ECE_ACT_POP_1) &&
               (ece->key_flags & VTSS_ECE_KEY_TAG_TAGGED_1) &&
               ece->vid.type == VTSS_VCAP_VR_TYPE_VALUE_MASK &&
               (ece->vid.vr.v.mask & 0xfff) == 0xfff && ece->vid.vr.v.value != VTSS_VID_NULL) {
        /* UNI port, direction BOTH, push one tag */
        srvl_ece_es0_tag_set(ece, tag, ece->vid.vr.v.value, 0, 
                             VTSS_ECE_ACT_OT_PCP_MODE_FIXED, VTSS_ECE_ACT_OT_PCP_MODE_MAPPED,
                             VTSS_ECE_ACT_OT_DEI_MODE_FIXED, VTSS_ECE_ACT_OT_DEI_MODE_DP,
                             VTSS_ECE_ACT_OT_DEI);

        if (ece->act_flags & VTSS_ECE_ACT_OT_PCP_MODE_FIXED) {
            /* If PCP is fixed, the smallest matching PCP is used */
            for (pcp = 0; pcp < 8; pcp++) {
                if ((pcp & ece->pcp.mask) == (ece->pcp.value & ece->pcp.mask)) {
                        tag->pcp.val = pcp;
                        break;
                }
            }
        }
        
        if (ece->act_flags & (VTSS_ECE_ACT_OT_DEI_MODE_FIXED | VTSS_ECE_ACT_OT_DEI_MODE_DP)) {
            /* If DEI is fixed or DP based, the smallest matching DEI is used */
            tag->dei.sel = VTSS_ES0_DEI_ES0;
            tag->dei.val = (ece->key_flags & VTSS_ECE_KEY_TAG_DEI_1 ? 1 : 0);
        }
    }

#if defined(VTSS_FEATURE_MPLS)
    vtss_mpls_ece_es0_update(vtss_state, evc, ece, esdx, action);
#endif /* VTSS_FEATURE_MPLS */
    
    return vtss_vcap_add(vtss_state, &vtss_state->vcap.es0.obj, VTSS_ES0_USER_EVC,
                         id, id_next, &data, 0);
}

vtss_rc vtss_srvl_ece_update(vtss_state_t *vtss_state,
                             vtss_ece_entry_t *ece, vtss_res_t *res, vtss_res_cmd_t cmd)
{
    vtss_evc_entry_t            *evc;
    vtss_ece_id_t               ece_id = ece->ece_id;
    vtss_ece_entry_t            *ece_next;
    vtss_vcap_id_t              id, id_next_is1, id_next_es0;
    vtss_port_no_t              port_no;
    BOOL                        nni;
    vtss_vcap_data_t            data;
    vtss_sdx_entry_t            *sdx, *sdx_next;
    vtss_ece_dir_t              dir;
    vtss_ece_rule_t             rule;
    vtss_vcap_key_size_t        key_size;
    vtss_vcap_range_chk_table_t range_new = vtss_state->vcap.range;
    
    VTSS_D("ece_id: %u, cmd: %d", ece->ece_id, cmd);

    if (cmd == VTSS_RES_CMD_CALC) {
        /* Calculate resource usage */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            nni = res->port_nni[port_no];
            key_size = srvl_evc_port_key_get(vtss_state, port_no, ece, NULL);
            if (res->port_add[port_no] || res->port_chg[port_no]) {
                /* Add/change port */
                if (vtss_srvl_ece_is1_needed(nni, res->dir_new, res->rule_new)) {
                    res->is1.add_key[key_size]++;
                    res->isdx.add++;
                }
                if (srvl_ece_es0_needed(nni, res->dir_new, res->rule_new)) {
                    res->es0.add++;
                    res->esdx.add++;
                }
            }
            
            if (res->port_del[port_no] || res->port_chg[port_no]) {
                /* Delete/change port */
                if (vtss_srvl_ece_is1_needed(nni, res->dir_old, res->rule_old)) {
                    /* Free range checkers for each deleted port */
                    id = srvl_evc_vcap_id(ece_id, port_no);
                    VTSS_RC(srvl_evc_range_free(vtss_state, &range_new, id));
                    res->is1.del_key[key_size]++;
                    res->isdx.del++;
                }
                if (srvl_ece_es0_needed(nni, res->dir_old, res->rule_old)) {
                    res->es0.del++;
                    res->esdx.del++;
                }
            }
        }
        
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            if ((res->port_add[port_no] || res->port_chg[port_no]) && 
                vtss_srvl_ece_is1_needed(res->port_nni[port_no], res->dir_new, res->rule_new)) {
                /* Allocate range checkers for each added port */
                VTSS_RC(srvl_evc_range_alloc(&range_new, ece, &data.u.is1));
            }
        }
    } else if (cmd == VTSS_RES_CMD_DEL) {
        /* Delete resources */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            if (res->port_del[port_no] || res->port_chg[port_no]) {
                /* Port deleted or changed (e.g. direction) */
                nni = res->port_nni[port_no];
                id = srvl_evc_vcap_id(ece_id, port_no);

                /* Delete IS1 entry, ISDX and range checkers */
                if (vtss_srvl_ece_is1_needed(nni, res->dir_old, res->rule_old)) {
                    VTSS_RC(srvl_evc_range_free(vtss_state, &vtss_state->vcap.range, id));
                    VTSS_RC(vtss_vcap_del(vtss_state, &vtss_state->vcap.is1.obj, VTSS_IS1_USER_EVC, id));
                    VTSS_RC(vtss_cmn_ece_sdx_free(vtss_state, ece, port_no, 1));
                }

                /* Delete ES0 entry and ESDX */
                if (srvl_ece_es0_needed(nni, res->dir_old, res->rule_old)) {
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
                if (vtss_srvl_ece_is1_needed(nni, res->dir_new, res->rule_new))
                    VTSS_RC(srvl_ece_sdx_alloc(vtss_state, ece, port_no, 1));

                /* Allocate ESDX */
                if (srvl_ece_es0_needed(nni, res->dir_new, res->rule_new))
                    VTSS_RC(srvl_ece_sdx_alloc(vtss_state, ece, port_no, 0));
            }
        }
        
        /* Find next VCAP IDs */
        id_next_is1 = VTSS_VCAP_ID_LAST;
        id_next_es0 = VTSS_VCAP_ID_LAST;
        for (ece_next = ece->next; ece_next != NULL; ece_next = ece_next->next) {
            dir = vtss_cmn_ece_dir_get(ece_next);
            rule = vtss_cmn_ece_rule_get(ece_next);
            for (sdx = ece_next->isdx_list; sdx != NULL && id_next_is1 == VTSS_VCAP_ID_LAST; 
                 sdx = sdx->next) {
                nni = (VTSS_PORT_BF_GET(ece_next->ports, sdx->port_no) ? 0 : 1);
                if (vtss_srvl_ece_is1_needed(nni, dir, rule))
                    id_next_is1 = srvl_evc_vcap_id(ece_next->ece_id, sdx->port_no);
            }
            for (sdx = ece_next->esdx_list; sdx != NULL && id_next_es0 == VTSS_VCAP_ID_LAST; 
                 sdx = sdx->next) {
                nni = (VTSS_PORT_BF_GET(ece_next->ports, sdx->port_no) ? 0 : 1);
                if (srvl_ece_es0_needed(nni, dir, rule))
                    id_next_es0 = srvl_evc_vcap_id(ece_next->ece_id, sdx->port_no);
            }
            if (id_next_is1 != VTSS_VCAP_ID_LAST && id_next_es0 != VTSS_VCAP_ID_LAST)
                break;
        }
        
        /* Add/update IS1, ES0 and ISDX entries */
        evc = &vtss_state->evc.info.table[ece->evc_id];
        sdx_next = NULL;
        while (sdx_next != ece->isdx_list) {
            /* Start with the last ISDX */
            for (sdx = ece->isdx_list; sdx != NULL; sdx = sdx->next) {
                if (sdx->next == sdx_next) {
                    /* Update ISDX entry */
                    VTSS_RC(srvl_ece_isdx_update(vtss_state, evc, ece, sdx));

                    /* Add IS1 entry */
                    id = srvl_evc_vcap_id(ece_id, sdx->port_no);
                    VTSS_RC(srvl_ece_is1_add(vtss_state, id, id_next_is1, evc, ece, sdx));
                    id_next_is1 = id;
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
                    id = srvl_evc_vcap_id(ece_id, sdx->port_no);
                    VTSS_RC(srvl_ece_es0_add(vtss_state, id, id_next_es0, evc, ece, sdx));
                    id_next_es0 = id;
                    sdx_next = sdx;
                    break;
                }
            }
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_evc_update(vtss_state_t *vtss_state,
                               vtss_evc_id_t evc_id, vtss_res_t *res, vtss_res_cmd_t cmd)
{
    vtss_ece_entry_t *ece;
    vtss_port_no_t   port_no;

    VTSS_D("evc_id: %u, cmd: %d", evc_id, cmd);

#if defined(VTSS_FEATURE_MPLS)
    VTSS_RC(vtss_srvl_evc_mpls_update(evc_id, res, cmd));
#endif /* VTSS_FEATURE_MPLS */

    for (ece = vtss_state->evc.ece_info.used; ece != NULL; ece = ece->next) {
        if (ece->evc_id != evc_id)
            continue;

        res->dir_old = vtss_cmn_ece_dir_get(ece);
        res->dir_new = res->dir_old;
        res->rule_old = vtss_cmn_ece_rule_get(ece);
        res->rule_new = res->rule_old;
        for (port_no = VTSS_PORT_NO_START; res->evc_add && port_no < vtss_state->port_count; port_no++) {
            /* Set port add flag for UNI ports if is a new EVC */
            if (VTSS_PORT_BF_GET(ece->ports, port_no)) {
                res->port_add[port_no] = 1;
            }
        }
        VTSS_RC(vtss_srvl_ece_update(vtss_state, ece, res, cmd));
        for (port_no = VTSS_PORT_NO_START; res->evc_add && port_no < vtss_state->port_count; port_no++) {
            /* Clear port add flag for UNI ports again */
            if (VTSS_PORT_BF_GET(ece->ports, port_no)) {
                res->port_add[port_no] = 0;
            }
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_evc_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    VTSS_D("port_no: %u", port_no);
    return vtss_srvl_vcap_port_key_addr_set(vtss_state,
                                            port_no,
                                            0, /* First IS1 lookup */
                                            vtss_state->evc.port_conf[port_no].key_type,
                                            vtss_state->evc.port_conf_old.key_type,
                                            vtss_state->evc.port_conf[port_no].dmac_dip);
}

/* - Debug print --------------------------------------------------- */

static void srvl_debug_sdx_cnt(const vtss_debug_printf_t pr, const char *name,  
                               vtss_chip_counter_pair_t *c1, vtss_chip_counter_pair_t *c2)
{
    char buf[80];
    int  bytes;

    for (bytes = 0; bytes < 2; bytes++) {
        sprintf(buf, "%s_%s", name, bytes ? "bytes" : "frames");
        vtss_srvl_debug_cnt(pr, buf, c2 == NULL ? NULL : "", 
                              bytes ? &c1->bytes : &c1->frames, 
                              c2 == NULL ? NULL : (bytes ? &c2->bytes : &c2->frames));
    }
}

static vtss_rc srvl_debug_isdx_list(vtss_state_t *vtss_state,
                                    const vtss_debug_printf_t pr, vtss_sdx_entry_t *isdx_list, 
                                    u32 id, BOOL *header, BOOL ece)
{
    vtss_sdx_entry_t *isdx;
    u32              value;

    for (isdx = isdx_list; isdx != NULL; isdx = isdx->next) {
        if (*header) {
            pr("ISDX  %sCE ID  Port  FORCE_ENA  ES0_ENA  SDLBI  VOE  ", ece ? "E" : "M");
            vtss_srvl_debug_print_port_header(vtss_state, pr, "");
            *header = 0;
        }
        SRVL_WR(VTSS_ANA_ANA_TABLES_ISDXTIDX, 
                VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_INDEX(isdx->sdx));
        SRVL_WR(VTSS_ANA_ANA_TABLES_ISDXACCESS, 
                VTSS_F_ANA_ANA_TABLES_ISDXACCESS_ISDX_TBL_CMD(ISDX_CMD_READ));
        VTSS_RC(srvl_isdx_table_idle(vtss_state));
        SRVL_RD(VTSS_ANA_ANA_TABLES_ISDXTIDX, &value);
        pr("%-6u%-8u", isdx->sdx, id);
        if (isdx->port_no < vtss_state->port_count)
            pr("%-6u", VTSS_CHIP_PORT(isdx->port_no));
        else
            pr("%-6s", isdx->port_no == VTSS_PORT_NO_CPU ? "CPU" : "?");
        pr("%-11u%-9u%-7u",
           value & VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_FORCE_ENA ? 1 : 0,
           value & VTSS_F_ANA_ANA_TABLES_ISDXTIDX_ISDX_ES0_KEY_ENA ? 1 : 0,
           VTSS_X_ANA_ANA_TABLES_ISDXTIDX_ISDX_SDLBI(value));
        
        SRVL_RD(VTSS_ANA_IPT_OAM_MEP_CFG(isdx->sdx), &value);
        if (value & VTSS_F_ANA_IPT_OAM_MEP_CFG_MEP_IDX_ENA)
            pr("%-5u", VTSS_X_ANA_IPT_OAM_MEP_CFG_MEP_IDX(value));
        else
            pr("None ");
        
        SRVL_RD(VTSS_ANA_ANA_TABLES_ISDXACCESS, &value);
        vtss_srvl_debug_print_mask(pr, VTSS_X_ANA_ANA_TABLES_ISDXACCESS_ISDX_PORT_MASK(value));
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_debug_sdx_counters(vtss_state_t *vtss_state,
                                       const vtss_debug_printf_t pr, 
                                       vtss_sdx_entry_t *isdx_list, vtss_sdx_entry_t *esdx_list,
                                       u32 id, BOOL ece)
{
    /*lint --e{454, 455} */ // Due to VTSS_EXIT_ENTER
    vtss_sdx_entry_t    *isdx, *esdx;
    vtss_sdx_counters_t *icnt, *ecnt, zero_cnt;
    vtss_port_no_t      port_no;

    memset(&zero_cnt, 0, sizeof(zero_cnt));
    for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
        icnt = &zero_cnt;
        ecnt = &zero_cnt;
        for (isdx = isdx_list; isdx != NULL; isdx = isdx->next) {
            if (isdx->port_no == port_no) {
                icnt = &vtss_state->evc.sdx_info.sdx_table[isdx->sdx];
                break;
            }
        }
        for (esdx = esdx_list; esdx != NULL; esdx = esdx->next) {
            if (esdx->port_no == port_no) {
                ecnt = &vtss_state->evc.sdx_info.sdx_table[esdx->sdx];
                break;
            }
        }
        if (isdx == NULL && esdx == NULL)
            continue;
        VTSS_RC(srvl_sdx_counters_update(vtss_state, isdx, esdx, NULL, 0));
        pr("Counters for %sCE ID %u, port %u (chip_port: %u), ISDX: %u, ESDX: %u\n", 
           ece ? "E" : "M", id, port_no, VTSS_CHIP_PORT(port_no), 
           isdx == NULL ? 0: isdx->sdx, esdx == NULL ? 0: esdx->sdx);
        srvl_debug_sdx_cnt(pr, "green", &icnt->rx_green, &ecnt->tx_green);
        srvl_debug_sdx_cnt(pr, "yellow", &icnt->rx_yellow, &ecnt->tx_yellow);
        srvl_debug_sdx_cnt(pr, "red", &icnt->rx_red, NULL);
        srvl_debug_sdx_cnt(pr, "drop_green", &icnt->rx_discard, NULL);
        srvl_debug_sdx_cnt(pr, "drop_yellow", &icnt->tx_discard, NULL);
        pr("\n");
        VTSS_EXIT_ENTER();
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_debug_evc(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
    /*lint --e{454, 455} */ // Due to VTSS_EXIT_ENTER
    vtss_ece_entry_t *ece;
    vtss_mce_entry_t *mce;
    BOOL             header = 1;

    VTSS_RC(vtss_srvl_debug_range_checkers(vtss_state, pr, info));
    VTSS_RC(vtss_srvl_debug_is0_all(vtss_state, pr, info));
    VTSS_RC(vtss_srvl_debug_is1_all(vtss_state, pr, info));
    VTSS_RC(vtss_srvl_debug_es0_all(vtss_state, pr, info));

    /* ISDX table - ECEs */
    for (ece = vtss_state->evc.ece_info.used; ece != NULL; ece = ece->next) {
        VTSS_RC(srvl_debug_isdx_list(vtss_state, pr, ece->isdx_list, ece->ece_id, &header, 1));
    }
    if (!header)
        pr("\n");
    header = 1;

    /* ISDX table - MCEs */
    for (mce = vtss_state->evc.mce_info.used; mce != NULL; mce = mce->next) {
        VTSS_RC(srvl_debug_isdx_list(vtss_state, pr, mce->isdx_list, mce->conf.id, &header, 0));
    }
    if (!header)
        pr("\n");

    /* SDX counters - ECEs */
    for (ece = vtss_state->evc.ece_info.used; ece != NULL; ece = ece->next) {
        VTSS_RC(srvl_debug_sdx_counters(vtss_state, pr, ece->isdx_list, ece->esdx_list, ece->ece_id, 1));
    }

    /* SDX counters - MCEs */
    for (mce = vtss_state->evc.mce_info.used; mce != NULL; mce = mce->next) {
        VTSS_RC(srvl_debug_sdx_counters(vtss_state, pr, mce->isdx_list, mce->esdx_list, mce->conf.id, 0));
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_evc_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_EVC, srvl_debug_evc, vtss_state, pr, info);
}

/* - Initialization ------------------------------------------------ */

vtss_rc vtss_srvl_evc_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_evc_state_t *state = &vtss_state->evc;
    vtss_sdx_info_t  *sdx_info = &state->sdx_info;
    vtss_sdx_entry_t *isdx, *esdx;
    u32              idx;
    u32              i;
    vtss_port_no_t   port_no;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->port_conf_set = srvl_evc_port_conf_set;
        state->add = vtss_cmn_evc_add;
        state->del = vtss_cmn_evc_del;
        state->ece_add = vtss_cmn_ece_add;
        state->ece_del = vtss_cmn_ece_del;
        state->ece_update = vtss_srvl_ece_update;
        state->update = srvl_evc_update;
        state->ece_counters_get = vtss_cmn_ece_counters_get;
        state->ece_counters_clear = vtss_cmn_ece_counters_clear;
        state->counters_get = vtss_cmn_evc_counters_get;
        state->counters_clear = vtss_cmn_evc_counters_clear;
        state->sdx_counters_update = srvl_sdx_counters_update;
        state->mce_add = srvl_mce_add;
        state->mce_del = srvl_mce_del;
        state->mce_port_get = srvl_mce_port_get;
        state->info.max_count = SRVL_EVC_CNT;
        state->ece_info.max_count = SRVL_EVC_CNT;
        state->mce_info.max_count = VTSS_MCES;
        state->sdx_info.max_count = SRVL_EVC_CNT;
        if (state->sdx_info.max_count > VTSS_SRVL_SDX_CNT)
            state->sdx_info.max_count = VTSS_SRVL_SDX_CNT;
        break;
    case VTSS_INIT_CMD_INIT:
        /* Clear SDX counters */
        for (i = 0; i <= vtss_state->evc.sdx_info.max_count; i++) {
            SRVL_WR(VTSS_SYS_SYSTEM_STAT_CFG,
                    VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_CLEAR_SHOT(0x18) |
                    VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(i));
        }
        
        /* Only update ESDX statistics if ES0 hit */
        SRVL_WR(VTSS_REW_COMMON_STAT_CFG, VTSS_F_REW_COMMON_STAT_CFG_STAT_MODE(1));
        break;
    case VTSS_INIT_CMD_POLL:
        /* Poll counters for 30 SDX entries, giving 1023/30 = 34 seconds between each poll.
           This ensures that any counter can wrap only once between each poll.
           The worst case is considered 32-bit byte counter on a 1Gbps port, which takes about
           0xffffffff/(1.000.000.000/8) = 34.3 seconds to wrap. */
        for (i = 0; i < 30; i++) {
            idx = sdx_info->poll_idx;
            isdx = &sdx_info->isdx.table[idx];
            esdx = &sdx_info->esdx.table[idx];
            idx++;
            sdx_info->poll_idx = (idx < sdx_info->max_count ? idx : 0);
            VTSS_RC(srvl_sdx_counters_update(vtss_state, isdx, esdx, NULL, 0));
        }
        break;
    case VTSS_INIT_CMD_PORT_MAP:
    {
        /* Use quarter key in IS1 first lookup by default */
        vtss_vcap_key_type_t key_type = VTSS_VCAP_KEY_TYPE_DOUBLE_TAG;

        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            vtss_state->evc.port_conf[port_no].key_type = key_type;
            VTSS_RC(vtss_srvl_vcap_port_key_addr_set(vtss_state, port_no, 0, key_type, key_type, FALSE));
        }
        break;
    }
    default:
        break;
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_EVC */
#endif /* VTSS_ARCH_SERVAL */
