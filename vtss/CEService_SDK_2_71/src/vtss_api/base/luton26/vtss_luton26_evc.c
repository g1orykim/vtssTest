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
#include "vtss_luton26_cil.h"

#if defined(VTSS_ARCH_LUTON26)
#if defined(VTSS_ARCH_CARACAL)

/* - CIL functions ------------------------------------------------- */

/* VCAP ID for EVC entries in ES0 */
static vtss_vcap_id_t l26_evc_es0_id(vtss_evc_id_t evc_id, vtss_port_no_t port_no)
{
    vtss_vcap_id_t id = port_no;

    return ((id << 32) + evc_id);
}

static vtss_rc l26_evc_es0_add(vtss_state_t *vtss_state,
                               vtss_evc_id_t evc_id, vtss_ece_entry_t *ece, 
                               vtss_port_no_t port_no)
{
    vtss_vcap_id_t       id = l26_evc_es0_id(evc_id, port_no);
    vtss_evc_entry_t     *evc = &vtss_state->evc.info.table[evc_id];
    vtss_evc_inner_tag_t *it = &evc->inner_tag;
    vtss_vcap_data_t     data;
    vtss_es0_data_t      *es0 = &data.u.es0;
    vtss_es0_entry_t     entry;
    vtss_es0_key_t       *key = &entry.key;
    vtss_es0_action_t    *action = &entry.action;
    BOOL                 normal;
    
    vtss_vcap_es0_init(&data, &entry);
    key->port_no = port_no;
    key->data.vid.vid = evc->ivid;
    if (ece == NULL) {
        /* NNI port */
        if (it->type == VTSS_EVC_INNER_TAG_NONE) {
            /* Push one tag ("port tag") */
            action->vid_a = evc->vid;
            action->tag = VTSS_ES0_TAG_NONE;
        } else {
            /* Push two tags ("port tag" is outer and "ES0 tag" is inner) */
            action->tag = VTSS_ES0_TAG_PORT;
            normal = VTSS_BOOL(it->vid_mode == VTSS_EVC_VID_MODE_NORMAL);
            action->vid_a = (normal ? evc->vid : it->vid);
            action->vid_b = (normal ? it->vid : evc->vid);
            action->qos = (it->pcp_dei_preserve ? VTSS_ES0_QOS_CLASS : VTSS_ES0_QOS_ES0);
            action->pcp = it->pcp;
            action->dei = it->dei;
            action->tpid = (it->type == VTSS_EVC_INNER_TAG_C ? VTSS_ES0_TPID_C :
                            it->type == VTSS_EVC_INNER_TAG_S ? VTSS_ES0_TPID_S :
                            VTSS_ES0_TPID_PORT);
        }
    } else {
        /* UNI port: Push one tag ("ES0 tag") */
        es0->port_no = port_no;
        es0->flags = VTSS_ES0_FLAG_TPID;
        action->tag = VTSS_ES0_TAG_ES0;
        action->vid_b = ((ece->act_flags & VTSS_ECE_ACT_OT_ENA) ? evc->uvid: ece->vid.vr.v.value);
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
    }
    return vtss_vcap_add(vtss_state, &vtss_state->vcap.es0.obj, VTSS_ES0_USER_EVC, 
                         id, VTSS_VCAP_ID_LAST, &data, 0);
}

/* VCAP ID for EVC entries in IS1 */
static vtss_vcap_id_t l26_evc_is1_id(vtss_ece_entry_t *ece, BOOL nni)
{
    vtss_vcap_id_t id = (nni ? 1 : 0);

    return ((id << 32) + ece->ece_id);
}

static vtss_rc l26_evc_range_alloc(vtss_state_t *vtss_state,
                                   vtss_vcap_range_chk_table_t *range, vtss_ece_entry_t *ece,
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

static vtss_rc l26_evc_range_free(vtss_state_t *vtss_state,
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

static BOOL l26_evc_id_valid(vtss_state_t *vtss_state, vtss_evc_id_t evc_id)
{
    return (evc_id != VTSS_EVC_ID_NONE && vtss_state->evc.info.table[evc_id].enable ? 1 : 0);
}

static vtss_rc l26_evc_is1_add(vtss_state_t *vtss_state, vtss_ece_entry_t *ece, BOOL nni)
{
    u32                         port_count = vtss_state->port_count;
    vtss_vcap_id_t              id_next, id = l26_evc_is1_id(ece, nni);
    vtss_is1_entry_t            entry;
    vtss_is1_key_t              *key = &entry.key;
    vtss_is1_action_t           *action = &entry.action;
    vtss_vcap_data_t            data;
    vtss_is1_data_t             *is1 = &data.u.is1;
    vtss_evc_entry_t            *evc = &vtss_state->evc.info.table[ece->evc_id];
    vtss_evc_inner_tag_t        *it = &evc->inner_tag;
    vtss_port_no_t              port_no;
    BOOL                        ece_pcp, nni_bidir = 0;
    BOOL                        inner_tag = 0;
    vtss_vcap_range_chk_table_t range_new = vtss_state->vcap.range;

    /* Free old range checkers */
    VTSS_RC(l26_evc_range_free(vtss_state, &range_new, id));

    vtss_vcap_is1_init(&data, &entry);
    is1->lookup = 0; /* First lookup */

    if (nni) {
        /* NNI rule, not added for DIR_UNI_TO_NNI */
        if (ece->act_flags & VTSS_ECE_ACT_DIR_UNI_TO_NNI) 
            return VTSS_RC_OK;

        /* For DIR_BOTH, bidirectional rules are used for NNI ports */
        if ((ece->act_flags & VTSS_ECE_ACT_DIR_NNI_TO_UNI) == 0) 
            nni_bidir = 1;
        
        /* NNI ports */
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            if (VTSS_PORT_BF_GET(evc->ports, port_no)) {
                key->port_list[port_no] = 1;
                if (vtss_state->evc.port_conf[port_no].inner_tag)
                    inner_tag = 1;
            }
        }
    } else {
        /* UNI rule, not added for DIR_NNI_TO_UNI */
        if (ece->act_flags & VTSS_ECE_ACT_DIR_NNI_TO_UNI) 
            return VTSS_RC_OK;

        /* UNI ports */
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            if (VTSS_PORT_BF_GET(ece->ports, port_no)) {
                key->port_list[port_no] = 1;
                if (vtss_state->evc.port_conf[port_no].inner_tag)
                    inner_tag = 1;
            }
        }
    }

    /* Match only double tagged frames if inner tag is enabled on one of the ports */
    if (inner_tag)
        key->inner_tag.tagged = VTSS_VCAP_BIT_1;

    if (nni_bidir) {
        /* NNI bidirectional rule */

        /* Match EVC VID by default */
        key->tag.vid.vr.v.value = evc->vid;
        key->tag.vid.vr.v.mask = 0xfff;
        key->tag.tagged = VTSS_VCAP_BIT_1;

        /* VID and PCP matching */
        ece_pcp = 0;
        if (inner_tag) {
            /* Matching on inner tag is enabled */
            if (it->type != VTSS_EVC_INNER_TAG_NONE) {
                /* Inner tag added */

                if (it->vid_mode == VTSS_EVC_VID_MODE_NORMAL)
                    key->tag.vid.vr.v.value = it->vid; /* Match inner tag in Normal mode */

                if (it->pcp_dei_preserve) {
                    /* Preserved PCP/DEI */
                    ece_pcp = 1;
                } else {
                    /* Fixed PCP/DEI */
                    key->tag.pcp.value = it->pcp;
                    key->tag.pcp.mask = 0x7;
                }
            }
        } else {
            /* Matching on outer tag is enabled */
            if (it->type != VTSS_EVC_INNER_TAG_NONE && it->vid_mode == VTSS_EVC_VID_MODE_TUNNEL)
                key->tag.vid.vr.v.value = it->vid; /* Match inner tag in Tunnel mode */
            ece_pcp = 1;
        }

        if (ece_pcp) {
            /* The ECE determines the PCP matching */ 
            if (ece->act_flags & VTSS_ECE_ACT_OT_PCP_MODE_FIXED) {
                /* Fixed PCP/DEI */
                key->tag.pcp.value = ece->ot_pcp;
                key->tag.pcp.mask = 0x7;
            } else {
                /* Preserved PCP/DEI */
                key->tag.pcp = ece->pcp;
            }
        }

        action->pop = (it->type == VTSS_EVC_INNER_TAG_NONE ? 1 : 2);
    } else {
        /* UNI rule or unidirectional NNI rule */

        /* Allocate range checkers for UNI/NNIs */
        VTSS_RC(l26_evc_range_alloc(vtss_state, &range_new, ece, is1));

        /* MAC header matching */
        key->mac.dmac_mc = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_DMAC_MC_VLD,
                                                VTSS_ECE_KEY_DMAC_MC_1);
        key->mac.dmac_bc = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_DMAC_BC_VLD,
                                                VTSS_ECE_KEY_DMAC_BC_1);
        key->mac.smac = ece->smac;
        
        /* VLAN tag matching */
        key->tag.vid = ece->vid;
        key->tag.pcp = ece->pcp;
        key->tag.dei = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_DEI_VLD, VTSS_ECE_KEY_TAG_DEI_1);
        key->tag.tagged = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_TAGGED_VLD,
                                               VTSS_ECE_KEY_TAG_TAGGED_1);
        key->tag.s_tag = vtss_cmn_ece_bit_get(ece, VTSS_ECE_KEY_TAG_S_TAGGED_VLD,
                                              VTSS_ECE_KEY_TAG_S_TAGGED_1);

        /* IP header matching */
        if (ece->key_flags & VTSS_ECE_KEY_PROT_IPV4) {
            key->type = VTSS_IS1_TYPE_IPV4;
            key->frame.ipv4.fragment = ece->frame.ipv4.fragment;
            key->frame.ipv4.proto = ece->frame.ipv4.proto;
            key->frame.ipv4.dscp = ece->frame.ipv4.dscp;
            key->frame.ipv4.sip = ece->frame.ipv4.sip;
            key->frame.ipv4.sport = ece->frame.ipv4.sport;
            key->frame.ipv4.dport = ece->frame.ipv4.dport;
        } else if (ece->key_flags & VTSS_ECE_KEY_PROT_IPV6) {
            key->type = VTSS_IS1_TYPE_IPV6;
            key->frame.ipv6.proto = ece->frame.ipv6.proto;
            key->frame.ipv6.dscp = ece->frame.ipv6.dscp;
            key->frame.ipv6.sip = ece->frame.ipv6.sip;
            key->frame.ipv6.sport = ece->frame.ipv6.sport;
            key->frame.ipv6.dport = ece->frame.ipv6.dport;
        }

        if (ece->act_flags & VTSS_ECE_ACT_POP_1)
            action->pop = 1;
        else if (ece->act_flags & VTSS_ECE_ACT_POP_2)
            action->pop = 2;
        if (ece->act_flags & VTSS_ECE_ACT_OT_PCP_MODE_FIXED) {
            /* Fixed PCP/DEI is used in outer tag ("port tag"), change classification */
            action->pcp_dei_enable = 1;
            action->pcp = ece->ot_pcp;
            action->dei = VTSS_BOOL(ece->act_flags & VTSS_ECE_ACT_OT_DEI);
        }
    }

    /* Action fields common for both directions */
    action->vid = evc->ivid;
    action->pop_enable = 1;
    if (!(ece->act_flags & VTSS_ECE_ACT_POLICY_NONE)) {
        action->pag_enable = 1;
        action->pag = ece->policy_no;
    }
    if (ece->act_flags & VTSS_ECE_ACT_PRIO_ENA) {
        action->prio_enable = 1;
        action->prio = ece->prio;
    }

    /* Commit range checkers */
    VTSS_RC(vtss_vcap_range_commit(vtss_state, &range_new));

    /* Find next ID with valid EVC */
    id_next = VTSS_VCAP_ID_LAST;
    for (ece = ece->next ; ece != NULL; ece = ece->next) {
        if (l26_evc_id_valid(vtss_state, ece->evc_id)) {
            id_next = l26_evc_is1_id(ece, (ece->act_flags & VTSS_ECE_ACT_DIR_NNI_TO_UNI) ? 1 : 0);
            break;
        }
    } 
    return vtss_vcap_add(vtss_state, &vtss_state->vcap.is1.obj, VTSS_IS1_USER_EVC, id, id_next, &data, 0);
}

static vtss_rc l26_evc_is1_del(vtss_state_t *vtss_state, vtss_ece_entry_t *ece, BOOL nni)
{
    vtss_vcap_obj_t  *obj = &vtss_state->vcap.is1.obj;
    vtss_vcap_user_t user = VTSS_IS1_USER_EVC;
    vtss_vcap_id_t   id = l26_evc_is1_id(ece, nni);

    if (vtss_vcap_lookup(vtss_state, obj, user, id, NULL, NULL) == VTSS_RC_OK) {
        /* Free range checkers */
        VTSS_RC(l26_evc_range_free(vtss_state, &vtss_state->vcap.range, id));
        
        /* Delete IS1 entry */
        VTSS_RC(vtss_vcap_del(vtss_state, obj, user, id));
    }
    return VTSS_RC_OK;
}

static vtss_rc l26_ece_update(vtss_state_t *vtss_state,
                              vtss_ece_entry_t *ece, vtss_res_t *res, vtss_res_cmd_t cmd)
{
    u32                         port_count = vtss_state->port_count;
    vtss_port_no_t              port_no;
    vtss_vcap_id_t              id;
    vtss_vcap_data_t            data;
    vtss_vcap_range_chk_table_t range_new = vtss_state->vcap.range;

    if (cmd == VTSS_RES_CMD_CALC) {
        /* Calculate resource usage */

        /* One IS1 rule for UNIs and one IS1 entry for NNIs */
        if (res->ece_del) {
            res->is1.del += (res->dir_old == VTSS_ECE_DIR_BOTH ? 2 : 1);

            if (res->dir_old == VTSS_ECE_DIR_NNI_TO_UNI) {
                /* Free old range checkers for NNIs */
                id = l26_evc_is1_id(ece, 1);
                VTSS_RC(l26_evc_range_free(vtss_state, &range_new, id));
            } else {
                /* Free old range checkers for UNIs */
                id = l26_evc_is1_id(ece, 0);
                VTSS_RC(l26_evc_range_free(vtss_state, &range_new, id));
            }
        }
        
        if (res->ece_add) {
            res->is1.add += (res->dir_new == VTSS_ECE_DIR_BOTH ? 2 : 1);

            /* Allocate range checkers for UNI/NNIs */
            VTSS_RC(l26_evc_range_alloc(vtss_state, &range_new, ece, &data.u.is1));
        }
        
        /* ES0 rules per UNI */
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            if (res->es0_add[port_no])
                res->es0.add++;
            if (res->es0_del[port_no])
                res->es0.del++;
        }
    } else if (cmd == VTSS_RES_CMD_DEL) {
        /* Delete resources */
        if (res->ece_del) {
            /* IS1 rule for UNIs */
            VTSS_RC(l26_evc_is1_del(vtss_state, ece, 0));

            /* IS1 rule for NNIs */
            VTSS_RC(l26_evc_is1_del(vtss_state, ece, 1));
        }

        /* ES0 rules for UNIs */
        for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
            if (res->es0_del[port_no]) {
                id = l26_evc_es0_id(ece->evc_id, port_no);
                VTSS_RC(vtss_vcap_del(vtss_state, &vtss_state->vcap.es0.obj, VTSS_ES0_USER_EVC, id));
            }
        }
    } else if (cmd == VTSS_RES_CMD_ADD) {
        /* Add/update resources */
        if (res->ece_add) {
            /* IS1 rule for UNIs */
            VTSS_RC(l26_evc_is1_add(vtss_state, ece, 0));

            /* IS1 rule for NNIs */
            VTSS_RC(l26_evc_is1_add(vtss_state, ece, 1));

            /* ES0 rules for UNIs */
            if (vtss_cmn_ece_es0_needed(ece)) {
                for (port_no = VTSS_PORT_NO_START; port_no < port_count; port_no++) {
                    if (VTSS_PORT_BF_GET(ece->ports, port_no)) {
                        VTSS_RC(l26_evc_es0_add(vtss_state, ece->evc_id, ece, port_no));
                    }
                }
            }
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc l26_evc_update(vtss_state_t *vtss_state,
                              vtss_evc_id_t evc_id, vtss_res_t *res, vtss_res_cmd_t cmd)
{
    vtss_evc_entry_t *evc = &vtss_state->evc.info.table[evc_id];
    vtss_port_no_t   port_no;
    vtss_vcap_id_t   id;
    vtss_ece_entry_t *ece;

    if (cmd == VTSS_RES_CMD_CALC) {
        /* Calculate resource usage */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            /* One ES0 rule per (EVC, NNI) */
            if (res->port_add[port_no])
                res->es0.add++;
            if (res->port_del[port_no])
                res->es0.del++;
        }
    } else if (cmd == VTSS_RES_CMD_DEL) {
        /* Delete resources */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            if (res->port_del[port_no]) {
                id = l26_evc_es0_id(evc_id, port_no);
                VTSS_RC(vtss_vcap_del(vtss_state, &vtss_state->vcap.es0.obj, VTSS_ES0_USER_EVC, id));
            }
        }
    } else if (cmd == VTSS_RES_CMD_ADD) {
        /* Add/update resources */
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            if (VTSS_PORT_BF_GET(evc->ports, port_no)) {
                /* One ES0 rule per (EVC, NNI) */
                VTSS_RC(l26_evc_es0_add(vtss_state, evc_id, NULL, port_no));
            }
        }

        /* Add/update IS1 rules for UNI/NNIs */
        res->ece_add = 1;
        for (ece = vtss_state->evc.ece_info.used; ece != NULL; ece = ece->next) {
            if (ece->evc_id == evc_id) {
                VTSS_RC(l26_ece_update(vtss_state, ece, res, cmd));
            }
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc l26_evc_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    u32              mask, port = VTSS_CHIP_PORT(port_no);
    BOOL             inner_tag = vtss_state->evc.port_conf[port_no].inner_tag;
    vtss_ece_entry_t *ece;
    
    /* Enable/disable inner tag classification in first IS1 lookup */
    mask = VTSS_F_ANA_PORT_VCAP_CFG_S1_VLAN_INNER_TAG_ENA(1);
    L26_WRM(VTSS_ANA_PORT_VCAP_CFG(port), inner_tag ? mask : 0, mask);

    /* Enable/disable DMAC/DIP match in first IS1 lookup */
    VTSS_RC(vtss_l26_vcl_port_conf_set(vtss_state, port_no));
    
    /* Update IS1 rule for NNIs */
    for (ece = vtss_state->evc.ece_info.used; ece != NULL; ece = ece->next) {
        if (l26_evc_id_valid(vtss_state, ece->evc_id)) {
            VTSS_RC(l26_evc_is1_add(vtss_state, ece, 1));
        }
    }
    
    /* Override tag remark mode for NNI ports */
    return vtss_l26_qos_port_conf_set(vtss_state, port_no);
}

/* - Debug print --------------------------------------------------- */

static vtss_rc l26_debug_evc(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_l26_debug_range_checkers(vtss_state, pr, info));
    VTSS_RC(vtss_l26_debug_vcap_port(vtss_state, pr, info));
    VTSS_RC(vtss_l26_debug_vcap_is1(vtss_state, pr, info));
    VTSS_RC(vtss_l26_debug_vcap_es0(vtss_state, pr, info));
    
    return VTSS_RC_OK;
}

vtss_rc vtss_l26_evc_debug_print(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_EVC, l26_debug_evc, vtss_state, pr, info);
}

/* - Initialization ------------------------------------------------ */

vtss_rc vtss_l26_evc_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_evc_state_t *state = &vtss_state->evc;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->port_conf_set = l26_evc_port_conf_set;
        state->add = vtss_cmn_evc_add;
        state->del = vtss_cmn_evc_del;
        state->ece_add = vtss_cmn_ece_add;
        state->ece_del = vtss_cmn_ece_del;
        state->ece_update = l26_ece_update;
        state->update = l26_evc_update;
        state->mce_add = vtss_cmn_mce_add;
        state->mce_del = vtss_cmn_mce_del;
        state->info.max_count = 256;
        state->ece_info.max_count = 256;
        break;
    case VTSS_INIT_CMD_INIT:
        break;
    default:
        break;
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_ARCH_CARACAL */
#endif /* VTSS_ARCH_LUTON26 */
