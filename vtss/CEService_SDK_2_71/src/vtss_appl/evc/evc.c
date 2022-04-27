/*

 Vitesse Switch API software.

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

#include "main.h"
#include "critd_api.h"
#include "conf_api.h"
#include "vlan_api.h"
#include "port_api.h"
#include "acl_api.h"
#include "packet_api.h"
#include "evc_api.h"
#include "evc.h"
#include "misc_api.h"

#ifdef VTSS_SW_OPTION_VCLI
#include "evc_cli.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "evc_icfg.h"
#endif

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "evc",
    .descr     = "Ethernet Virtual Connections"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_L2CP] = {
        .name      = "l2cp",
        .descr     = "L2CP forwarding",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    }
};

#define EVC_CRIT_ENTER() critd_enter(&evc_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define EVC_CRIT_EXIT()  critd_exit( &evc_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define EVC_CRIT_ENTER() critd_enter(&evc_global.crit)
#define EVC_CRIT_EXIT()  critd_exit( &evc_global.crit)
#endif /* VTSS_TRACE_ENABLED */

static evc_global_t evc_global;

#if defined(VTSS_ARCH_SERVAL)
/* Well-known multicast addresses */
static const u8 evc_dmac_bpdu[]         = { 0x01, 0x80, 0xc2, 0x00, 0x00, 0x00 };
static const u8 evc_dmac_custom[]       = { 0x01, 0x01, 0xc1, 0x00, 0x01, 0x00 };
static const u8 evc_dmac_cisco_tunnel[] = { 0x01, 0x00, 0x0c, 0xcd, 0xcd, 0xd0 };
static const u8 evc_dmac_cisco_snap[]   = { 0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc };
#endif /* VTSS_ARCH_SERVAL */

/* ================================================================= *
 *  Control API
 * ================================================================= */

/* EVC change callback registration */
vtss_rc evc_change_register(evc_change_callback_t callback)
{
    vtss_rc rc = VTSS_RC_ERROR;
    int     i;

    /* Enter critical region */
    EVC_CRIT_ENTER();

    for (i = 0; i < EVC_CHANGE_REG_MAX; i++) {
        if (evc_global.change_callback[i] == NULL) {
            evc_global.change_callback[i] = callback;
            rc = VTSS_RC_OK;
            break;
        }
    }

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return rc;
}

/* EVC change event, called inside critical region */
static void evc_change_event(vtss_evc_id_t evc_id)
{
    int                   i;
    evc_change_callback_t callback;

    /*lint --e{454,455,456} ... We are called within a critical region */

    for (i = 0; i < EVC_CHANGE_REG_MAX; i++) {
        if ((callback = evc_global.change_callback[i]) != NULL) {
            /* Exit critical region to allow caller to use EVC API */
            EVC_CRIT_EXIT();

            /* Do callback */
            callback(evc_id);

            /* Enter critical region again */
            EVC_CRIT_ENTER();
        }
    }
}

/* Check EVC table and generate change events */
static void evc_change_event_check(void)
{
    vtss_evc_id_t evc_id;
    evc_conf_t    *evc;
    
    for (evc_id = 0; evc_id < EVC_ID_COUNT; evc_id++) {
        evc = &evc_global.evc_table[evc_id];
        if (evc->flags & EVC_FLAG_CHANGED) {
            evc->flags &= ~EVC_FLAG_CHANGED;
            evc_change_event(evc_id);
        }
    }
}

/* Set EVC change flag */
static void evc_change_flag_set(vtss_evc_id_t evc_id)
{
    if (evc_id < EVC_ID_COUNT) {
        evc_global.evc_table[evc_id].flags |= EVC_FLAG_CHANGED;
    }
}

/* ================================================================= *
 *  Internal data and VTSS API access
 * ================================================================= */

/* - VLAN interface ------------------------------------------------ */

#if defined(VTSS_ARCH_CARACAL)
/* For Caracal, a VLAN is always used */
#define EVC_VLAN_ENABLED(evc) (evc->flags & EVC_FLAG_ENABLED)
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_ARCH_JAGUAR_1)
/* For Jaguar-1, a VLAN is only used if learning is enabled */
#define EVC_VLAN_ENABLED(evc) ((evc->flags & EVC_FLAG_ENABLED) && (evc->flags & EVC_FLAG_LEARN))
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_SERVAL)
/* For Serval, a VLAN is always used */
#define EVC_VLAN_ENABLED(evc)     (evc->flags & EVC_FLAG_ENABLED)
#define EVC_ECE_VLAN_ENABLED(ece) (ece->conf.flags_act & (ECE_FLAG_ACT_TX_LOOKUP_VID | ECE_FLAG_ACT_TX_LOOKUP_VID_PCP))
#endif /* VTSS_ARCH_SERVAL */

/* Add/modify/delete VLAN */
static vtss_rc evc_vlan_update(vtss_vid_t vid)
{
    vtss_rc           rc = VTSS_RC_OK;
    evc_conf_t        *evc;
    evc_ece_t         *ece;
    vtss_evc_id_t     evc_id;
    vlan_mgmt_entry_t vlan_entry;
    vtss_port_no_t    port_no;
    port_iter_t       pit;

    /* Find all ECEs mapping to an enabled EVC with learning enabled for the VID */
    memset(&vlan_entry, 0, sizeof(vlan_entry));
    for (ece = evc_global.ece_used; ece != NULL; ece = ece->next) {
        if ((evc_id = ece->conf.evc_id) == VTSS_EVC_ID_NONE) {
            continue;
        }
        evc = &evc_global.evc_table[evc_id];
        if (evc->ivid == vid && 
#if defined(VTSS_ARCH_SERVAL)
            EVC_ECE_VLAN_ENABLED(ece) &&
#endif /* VTSS_ARCH_SERVAL */
            EVC_VLAN_ENABLED(evc)) {
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                port_no = pit.iport;
                if (VTSS_PORT_BF_GET(evc->ports, port_no) ||
                    VTSS_PORT_BF_GET(ece->conf.ports, port_no)) {
                    vlan_entry.vid = vid;
                    vlan_entry.ports[port_no] = 1;
                }
            }
        }
    }

    /* Add/delete VLAN */
    if (vlan_entry.vid == VTSS_VID_NULL) {
        T_D("deleting vid: %u", vid);
        if (VTSS_BF_GET(evc_global.vid_added, vid)) {
            rc = vlan_mgmt_vlan_del(VTSS_ISID_START, vid, VLAN_USER_EVC);
        }
        VTSS_BF_SET(evc_global.vid_added, vid, 0);
    } else {
        T_D("adding vid: %u", vid);
        rc = vlan_mgmt_vlan_add(VTSS_ISID_START, &vlan_entry, VLAN_USER_EVC);
        VTSS_BF_SET(evc_global.vid_added, vid, 1);
    }

    return rc;
}

/* Update VLAN for ECE based on EVC ID */
static vtss_rc evc_ece_vlan_update(vtss_evc_id_t evc_id)
{
    vtss_rc    rc = VTSS_RC_OK;
    evc_conf_t *evc;

    T_D("evc_id: %u", evc_id);

    if (evc_id != VTSS_EVC_ID_NONE) {
        evc = &evc_global.evc_table[evc_id];
        if (EVC_VLAN_ENABLED(evc)) {
            rc = evc_vlan_update(evc->ivid);
        }
    }
    return rc;
}

#if defined(VTSS_ARCH_SERVAL)
/* VLAN tag information */
typedef struct {
    BOOL       tagged;
    BOOL       c_tagged;
    BOOL       s_tagged;
    vtss_vid_t vid;
} evc_tag_info_t;

/*lint -sem(evc_packet_rx, thread_protected) */
static BOOL evc_packet_rx(void *contxt, const uchar *const frame, const vtss_packet_rx_info_t *const rx_info)
{
    evc_conf_t                *evc;
    evc_ece_t                 *ece;
    evc_ece_conf_t            *conf;
    vtss_port_no_t            port_no = rx_info->port_no;
    vtss_vid_t                ivid = rx_info->tag.vid;
    const vtss_vlan_tag_t     *tag = &rx_info->stripped_tag;
    u32                       i, flags_key, flags_act, len = rx_info->length;
    const u8                  *dmac = frame, *data = &frame[12];
    u8                        *tx_frame, mask, dmac_tunnel[6];
    vtss_etype_t              etype, tpid, s_custom_etype = evc_global.s_custom_etype;
    vtss_ece_type_t           ece_type;
    evc_tag_info_t            tag_table[2], *tag_info;
    int                       nni_rx, uni_tx, skip, popped, pop_cnt, push_cnt, pop_diff;
    port_iter_t               pit;
    packet_tx_props_t         tx_props;
    vtss_packet_port_info_t   packet_info;
    vtss_packet_port_filter_t filter[VTSS_PORT_ARRAY_SIZE];
    u64                       uni_mask = 0, nni_mask = 0;

    T_DG(TRACE_GRP_L2CP, "frame of length %d received on port %u, vid: %u", len, port_no, ivid);
    T_DG_HEX(TRACE_GRP_L2CP, frame, 48);

    /* Parse up to two VLAN tags */
    etype = ((data[0] << 8) + data[1]);
    memset(tag_table, 0, sizeof(tag_table));
    for (i = 0; i < 2; i++) {
        tag_info = &tag_table[i];
        popped = (i == 0 && tag->tpid != 0);
        tpid = (popped ? tag->tpid : etype);
        if (tpid == 0x8100) {
            tag_info->c_tagged = 1;
            tag_info->tagged = 1;
        } else if (tpid == 0x88a8 || tpid == s_custom_etype) {
            tag_info->s_tagged = 1;
            tag_info->tagged = 1;
        } else {
            break;
        }
        T_DG(TRACE_GRP_L2CP, "%s tag is %s", i ? "inner" : "outer", tag_info->c_tagged ? "C" : "S");
        if (popped) {
            tag_info->vid = tag->vid;
        } else {
            tag_info->vid = (((data[2] & 0xf) << 8) + data[3]);
            data += 4; /* Skip tag */
            etype = ((data[0] << 8) + data[1]);
        }
    }
    tag_info = &tag_table[0];

    /* Parse frame data */
    if (etype < 10) {
        /* Short LLC frame ignored, we expect at least 2 (len) + 3 (LLC) + 5 (SNAP data) bytes */
        T_NG(TRACE_GRP_L2CP, "short frame");
        return FALSE;
    } else if (etype < 0x0600) {
        data += 2; /* Skip length field */
        if (data[0] == 0xaa && data[1] == 0xaa && data[2] == 0x03) {
            /* SNAP frame */
            ece_type = VTSS_ECE_TYPE_SNAP;
            data += 3; /* Skip LLC header */
        } else {
            /* LLC frame */
            ece_type = VTSS_ECE_TYPE_LLC;
        }
    } else if (etype == 0x0800 || etype == 0x86dd) {
        /* IPv4/IPv6 frame, no further processing */
        T_NG(TRACE_GRP_L2CP, "IP frame");
        return FALSE;
    } else {
        /* ETYPE frame */
        ece_type = VTSS_ECE_TYPE_ETYPE;
    }

    /* Find matching ECE */
    for (ece = evc_global.ece_used; ece != NULL; ece = ece->next) {
        conf = &ece->conf;
        /* Skip disabled EVCs */
        if (conf->evc_id == VTSS_EVC_ID_NONE) {
            T_NG(TRACE_GRP_L2CP, "ece %u: evc none", conf->ece_id);
            continue;
        }
        evc = &evc_global.evc_table[conf->evc_id];
        if (!(evc->flags & EVC_FLAG_ENABLED)) {
            T_NG(TRACE_GRP_L2CP, "ece %u: evc disabled", conf->ece_id);
            continue;
        }

        /* Skip non-tunneling ECEs */
        flags_act = conf->flags_act;
        flags_key = conf->flags_key;
        if ((flags_act & ECE_FLAG_ACT_L2CP_TUNNEL) == 0) {
            T_NG(TRACE_GRP_L2CP, "ece %u: not tunnel", conf->ece_id);
            continue;
        }

        /* Match classified VID */
        if (evc->ivid != ivid) {
            T_NG(TRACE_GRP_L2CP, "ece %u: ivid mismatch", conf->ece_id);
            continue;
        }

        /* Calculate tunnel DMAC */
        if (flags_act & ECE_FLAG_ACT_L2CP_DMAC_CISCO) {
            for (i = 0; i < 6; i++) {
                dmac_tunnel[i] = evc_dmac_cisco_tunnel[i];
            }
        } else {
            for (i = 0; i < 6; i++) {
                dmac_tunnel[i] = (i < 5 ? evc_dmac_custom[i] :
                                  ece_type == VTSS_ECE_TYPE_SNAP ? 0xff : conf->frame_dmac.value[5]);
            }
        }

        /* Match UNI/NNI ports */
        skip = 0;
        if (VTSS_PORT_BF_GET(conf->ports, port_no)) {
            /* UNI port match */
            nni_rx = 0;

            /* Match direction */
            if (flags_act & ECE_FLAG_ACT_DIR_NNI_TO_UNI) {
                T_NG(TRACE_GRP_L2CP, "ece %u: nni dir mismatch", conf->ece_id);
                continue;
            }

            /* Match L2CP DMAC */
            for (i = 0; i < 6; i++) {
                mask = conf->frame_dmac.mask[i];
                if ((dmac[i] & mask) != (conf->frame_dmac.value[i] & mask))
                    skip = 1;
            }
        } else if (VTSS_PORT_BF_GET(evc->ports, port_no)) {
            /* NNI port match */
            nni_rx = 1;

            /* Match direction */
            if (flags_act & ECE_FLAG_ACT_DIR_UNI_TO_NNI) {
                T_NG(TRACE_GRP_L2CP, "ece %u: nni dir mismatch", conf->ece_id);
                continue;
            }

            /* Match tunnel DMAC */
            for (i = 0; i < 6; i++) {
                if (dmac[i] != dmac_tunnel[i])
                    skip = 1;
            }
        } else {
            T_NG(TRACE_GRP_L2CP, "ece %u: port mismatch", conf->ece_id);
            continue;
        }

        if (skip) {
            T_NG(TRACE_GRP_L2CP, "ece %u: dmac mismatch", conf->ece_id);
            continue;
        }

        /* Match frame type */
        if (ece->conf.frame_type != ece_type) {
            T_NG(TRACE_GRP_L2CP, "ece %u: frame type mismatch", conf->ece_id);
            continue;
        }

        /* Match tags */
        if (nni_rx && (flags_act & ECE_FLAG_ACT_DIR_NNI_TO_UNI) == 0) {
            /* NNI and bidirectional ECE */
            pop_cnt = 4;

            /* Outer tag matching */
            if (tag_info->tagged == 0 || tag_info->vid != evc->vid)
                skip = 1;
        } else {
            /* UNI or unidirectional ECE */
            pop_cnt = 0;
            if (tag_info->tagged) {
                if (flags_act & ECE_FLAG_ACT_POP_1) {
                    pop_cnt = 4;
                } else if (flags_act & ECE_FLAG_ACT_POP_2) {
                    pop_cnt = (tag_table[1].tagged ? 8 : 4);
                }
            }

            /* Outer tag matching */
            if (flags_key & ECE_FLAG_KEY_TAGGED_VLD) {
                if (flags_key & ECE_FLAG_KEY_TAGGED_1) {
                    /* Tag type */
                    if (flags_key & ECE_FLAG_KEY_S_TAGGED_VLD) {
                        if (flags_key & ECE_FLAG_KEY_S_TAGGED_1) {
                            if (!tag_info->s_tagged)
                                skip = 1;
                        } else if (!tag_info->c_tagged)
                            skip = 1;
                    }

                    /* VID matching */
                    if (flags_key & ECE_FLAG_KEY_VID_RANGE) {
                        if (tag_info->vid < conf->vid.low || tag_info->vid > conf->vid.high)
                            skip = 1;
                    } else if ((tag_info->vid & conf->vid.high) != (conf->vid.low & conf->vid.high))
                        skip = 1;
                } else if (tag_info->tagged)
                    skip = 1;
            }
        }

        if (skip) {
            T_NG(TRACE_GRP_L2CP, "ece %u: tag mismatch, flags_key: 0x%08x", conf->ece_id, flags_key);
            continue;
        }

        /* Match frame data */
        for (i = 0; i < 5; i++) {
            mask = conf->frame_data.mask[i];
            if ((data[i] & mask) != (conf->frame_data.value[i] & mask))
                skip = 1;
        }
        if (skip) {
            T_NG(TRACE_GRP_L2CP, "ece %u: frame data mismatch", conf->ece_id);
            continue;
        }

        /* ECE match */
        T_DG(TRACE_GRP_L2CP, "ece ID match: %u", conf->ece_id);

        /* Use packet filter for correct forwarding to potential UNI/NNI ports*/
        if (vtss_packet_port_info_init(&packet_info) != VTSS_RC_OK) {
            T_WG(TRACE_GRP_L2CP, "vtss_packet_port_info_init failed");
            break;
        }
        packet_info.port_no = port_no;
        packet_info.vid = ivid;
        if (vtss_packet_port_filter_get(NULL, &packet_info, filter) != VTSS_RC_OK) {
            T_WG(TRACE_GRP_L2CP, "vtss_packet_port_filter_get failed");
            break;
        }
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            if (filter[pit.iport].filter == VTSS_PACKET_FILTER_DISCARD)
                continue;

            if (VTSS_PORT_BF_GET(conf->ports, pit.iport)) {
                /* UNI port */
                T_DG(TRACE_GRP_L2CP, "forward to UNI port %u", pit.iport);
                uni_mask |= VTSS_BIT64(pit.iport);
            }
            if (VTSS_PORT_BF_GET(evc->ports, pit.iport)) {
                /* NNI port */
                T_DG(TRACE_GRP_L2CP, "forward to NNI port %u", pit.iport);
                nni_mask |= VTSS_BIT64(pit.iport);
            }
        }

        packet_tx_props_init(&tx_props);
        tx_props.packet_info.modid = VTSS_MODULE_ID_EVC;
        tx_props.tx_info.tag = rx_info->tag;      /* Classified VID, PCP and DEI */
        tx_props.tx_info.tag.tpid = 0;            /* Enable rewriter */
        tx_props.tx_info.cos = rx_info->cos;      /* Preserve CoS */
        tx_props.tx_info.isdx = VTSS_ISDX_CPU_TX; /* Enable egress statistics */
        tx_props.tx_info.isdx_dont_use = 1;

        /* Forward to UNI/NNI ports */
        for (uni_tx = 0; uni_tx < 2; uni_tx++) {
            if ((tx_props.tx_info.dst_port_mask = (uni_tx ? uni_mask : nni_mask)) == 0) {
                /* Skip forwarding if no egress ports */
                continue;
            }

            /* Allocate Tx buffer, making room for one pushed tag */
            if ((tx_frame = packet_tx_alloc(len + 4)) == NULL) {
                T_WG(TRACE_GRP_L2CP, "packet_tx_alloc failed");
                continue;
            }

            /* Copy MAC header */
            memcpy(tx_frame, frame, 12);
            if (nni_rx) {
                if (uni_tx) {
                    /* Forwarding from NNI to UNI, insert L2CP DMAC */
                    for (i = 0; i < 6; i++) {
                        tx_frame[i] = conf->frame_dmac.value[i];
                    }
                }
            } else if (!uni_tx) {
                /* Forwarding from UNI to NNI, insert tunnel DMAC */
                for (i = 0; i < 6; i++) {
                    tx_frame[i] = dmac_tunnel[i];
                }
            }

            /* Tag pushing/popping */
            pop_diff = (pop_cnt - (tag->tpid ? 4 : 0));
            push_cnt = 0;
            if (pop_diff < 0) {
                /* The popped tag is pushed into the frame again */
                tx_frame[12] = ((tag->tpid >> 8) & 0xff);
                tx_frame[13] = (tag->tpid & 0xff);
                tx_frame[14] = (((tag->pcp << 5) & 0xe0) + ((tag->dei ? 1 : 0) << 4) + ((tag->vid >> 8) & 0x0f));
                tx_frame[15] = (tag->vid & 0xff);
                push_cnt = 4;
            }

            /* Copy data */
            memcpy(tx_frame + 12 + push_cnt, frame + 12 + pop_diff + push_cnt , len - 12 - pop_diff);

            tx_props.packet_info.frm[0] = tx_frame;
            tx_props.packet_info.len[0] = (len - pop_diff);
            if (packet_tx(&tx_props) != VTSS_RC_OK) {
                T_WG(TRACE_GRP_L2CP, "packet_tx failed");
            }
        }

        /* ECE matched and forwarding done */
        break;
    }

    if (ece == NULL) {
        T_DG(TRACE_GRP_L2CP, "no ECE match");
    }

    return FALSE;
}

static vtss_ace_id_t evc_ace_id_get(vtss_acl_policy_no_t policy_no, vtss_ace_type_t type, BOOL cisco)
{
    /* Return unique ACE ID (0-511) */
    return ((policy_no << 3) + (type << 1) + (cisco ? 1 : 0));
}

static void evc_ace_info_get(vtss_ace_id_t id, vtss_acl_policy_no_t *policy_no, vtss_ace_type_t *type, BOOL *cisco)
{
    /* Map back from ACE ID to policy/type/tunnel */
    *policy_no = (id >> 3);
    *type = ((id >> 1) & 0x3);
    *cisco = (id & 0x1);
}

static void evc_acl_update(void)
{
    port_iter_t          pit;
    evc_conf_t           *evc;
    evc_ece_t            *ece;
    acl_user_t           user = ACL_USER_EVC;
    vtss_ace_id_t        id, ace_id, id_next = VTSS_ACE_ID_LAST;
    acl_entry_conf_t     ace;
    vtss_acl_policy_no_t policy_no;
    evc_acl_info_t       *info_old = &evc_global.acl_info_old;
    evc_acl_info_t       *info = &evc_global.acl_info;
    evc_ace_entry_t      *entry, *entry_discard, *entry_cpu_redir, *entry_bpdu, *old_entry;
    evc_packet_info_t    *packet_info = &evc_global.packet_info;
    evc_packet_info_t    packet_info_old;
    vtss_ace_type_t      ace_type;
    int                  i;
    BOOL                 changed, cisco;
    u32                  flags_act;
    vtss_ace_u48_t       dmac;
    packet_rx_filter_t   filter;

    /* Copy old information and clear new */
    *info_old = *info;
    memset(info, 0, sizeof(*info));
    packet_info_old = *packet_info;
    memset(packet_info, 0, sizeof(*packet_info));

    /* Initialize pointers for discard, CPU redirect and BPDU ACEs */
    id = evc_ace_id_get(ACL_POLICY_DISCARD, VTSS_ACE_TYPE_ANY, 0);
    entry_discard = &info->entry[id];
    id = evc_ace_id_get(ACL_POLICY_CPU_REDIR, VTSS_ACE_TYPE_ANY, 0);
    entry_cpu_redir = &info->entry[id];
    id = evc_ace_id_get(ACL_POLICY_CPU_REDIR, VTSS_ACE_TYPE_ETYPE, 0);
    entry_bpdu = &info->entry[id];

    /* Traverse ECEs to build ACE information */
    for (ece = evc_global.ece_used; ece != NULL; ece = ece->next) {
        /* Skip disabled EVCs */
        if (ece->conf.evc_id == VTSS_EVC_ID_NONE)
            continue;
        evc = &evc_global.evc_table[ece->conf.evc_id];
        if (!(evc->flags & EVC_FLAG_ENABLED))
            continue;

        /* Enable discard ACE if any ECE requires this */
        flags_act = ece->conf.flags_act;
        if (flags_act & ECE_FLAG_ACT_L2CP_DISCARD)
            entry_discard->add = 1;

        /* Enable CPU redirect ACE if any ECE requires this */
        if (flags_act & (ECE_FLAG_ACT_L2CP_TUNNEL | ECE_FLAG_ACT_L2CP_PEER))
            entry_cpu_redir->add = 1;

        /* Include NNI ports in BPDU ACE */
        for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
            entry_bpdu->ports[i] |= evc->ports[i];
        }

        /* Find ACE type for tunnel ACEs */
        switch (ece->conf.frame_type) {
        case VTSS_ECE_TYPE_ETYPE:
            ace_type = VTSS_ACE_TYPE_ETYPE;
            break;
        case VTSS_ECE_TYPE_LLC:
            ace_type = VTSS_ACE_TYPE_LLC;
            break;
        case VTSS_ECE_TYPE_SNAP:
            ace_type = VTSS_ACE_TYPE_SNAP;
            break;
        default:
            ace_type = VTSS_ACE_TYPE_ANY;
            break;
        }

        /* Tunnel processing below */
        if (ace_type == VTSS_ACE_TYPE_ANY || (flags_act & ECE_FLAG_ACT_L2CP_TUNNEL) == 0)
            continue;

        if ((flags_act & ECE_FLAG_ACT_DIR_UNI_TO_NNI) == 0) {
            /* Include NNI ports for tunnel ACEs and packet registration*/
            id = evc_ace_id_get(ece->conf.policy_no, ace_type, flags_act & ECE_FLAG_ACT_L2CP_DMAC_CISCO ? 1 : 0);
            entry = &info->entry[id];
            entry->add = 1;
            packet_info->add = 1;
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                entry->ports[i] |= evc->ports[i];
                packet_info->ports[i] |= evc->ports[i];
            }
        }

        if ((flags_act & ECE_FLAG_ACT_DIR_NNI_TO_UNI) == 0) {
            /* Include UNI ports for packet registration */
            packet_info->add = 1;
            for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                packet_info->ports[i] |= ece->conf.ports[i];
            }
        }
    }

    /* Include all ports for discard and CPU redir ACEs */
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        if (entry_discard->add) {
            VTSS_PORT_BF_SET(entry_discard->ports, pit.iport, 1);
        }
        if (entry_cpu_redir->add) {
            VTSS_PORT_BF_SET(entry_cpu_redir->ports, pit.iport, 1);
        }
        if (VTSS_PORT_BF_GET(entry_bpdu->ports, pit.iport)) {
            entry_bpdu->add = 1;
        }
    }

    /* Save LLC BPDU entry */
    id = evc_ace_id_get(ACL_POLICY_CPU_REDIR, VTSS_ACE_TYPE_LLC, 0);
    info->entry[id] = *entry_bpdu;

    /* Update ACL rules */
    for (id = 0; id < ACE_MAX; id++) {
        ace_id = (id + 1); /* ACL module requires ID in range 1-512 */
        entry = &info->entry[id];
        old_entry = &info_old->entry[id];

        /* Determine if ACE is has changed */
        changed = (entry->add == old_entry->add ? 0 : 1);
        for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
            if (entry->ports[i] != old_entry->ports[i]) {
                changed = 1;
                break;
            }
        }
        if (!changed) {
            if (entry->add) {
                id_next = ace_id;
            }
            continue;
        }

        evc_ace_info_get(id, &policy_no, &ace_type, &cisco);
        T_D("%s id: 0x%x, id_next: 0x%x, policy_no: %u, type: %u, cisco: %u",
            entry->add ? "add" : "del", id, id_next, policy_no, ace_type, cisco);

        if (!entry->add) {
            /* Delete ACE */
            if (acl_mgmt_ace_del(user, ace_id) != VTSS_RC_OK) {
                T_W("delete ace_id: 0x%x failed", ace_id);
            }
            continue;
        }

        /* Add/update ACE */
        if (acl_mgmt_ace_init(ace_type, &ace) != VTSS_RC_OK) {
            T_W("init ace_id: 0x%x failed", ace_id);
            continue;
        }

        ace.id = ace_id;
        ace.isid = VTSS_ISID_LOCAL;
        ace.action.port_action = VTSS_ACL_PORT_ACTION_REDIR; /* Redirect is used to make EVC policer work */
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            ace.port_list[pit.iport] = VTSS_PORT_BF_GET(entry->ports, pit.iport);
            ace.action.port_list[pit.iport] = 0;
        }

        /* Match policy by default */
        ace.policy.value = policy_no;
        ace.policy.mask = 0x3f;
 
        /* Use L2CP queue by default */
        ace.action.cpu_queue = PACKET_XTR_QU_L2CP;

        memset(&dmac, 0, sizeof(dmac));
        switch (policy_no) {
        case ACL_POLICY_CPU_REDIR:
            if (ace_type != VTSS_ACE_TYPE_ANY) {
                /* Untagged ETYPE/LLC BPDU matching for NNI ports */
                ace.policy.mask = 0x00;
                ace.tagged = VTSS_ACE_BIT_0;
                for (i = 0; i < 6; i++) {
                    dmac.value[i] = evc_dmac_bpdu[i];
                    dmac.mask[i] = (i == 5 ? 0xf0 : 0xff);
                }
                ace.action.cpu_queue = PACKET_XTR_QU_BPDU;
            }
            ace.action.force_cpu = 1;
            break;
        case ACL_POLICY_DISCARD:
            /* Match any frame based on policy */
            break;
        default:
            /* Match tunnel DMAC */
            for (i = 0; i < 6; i++) {
                dmac.value[i] = (cisco ? evc_dmac_cisco_tunnel[i] : evc_dmac_custom[i]);
                dmac.mask[i] = (cisco || i != 5 ? 0xff : 0x00);
            }
            ace.action.force_cpu = 1;
            break;
        }

        /* DMAC matching for ETYPE/LLC/SNAP */
        switch (ace_type) {
        case VTSS_ACE_TYPE_ETYPE:
            ace.frame.etype.dmac = dmac;
            break;
        case VTSS_ACE_TYPE_LLC:
            ace.frame.llc.dmac = dmac;
            break;
        case VTSS_ACE_TYPE_SNAP:
            ace.frame.snap.dmac = dmac;
            break;
        default:
            break;
        }

        if (acl_mgmt_ace_add(user, id_next, &ace) == VTSS_RC_OK) {
            id_next = ace_id;
        } else {
            T_W("add ace_id: 0x%x failed", ace_id);
        }
    }

    /* Update packet registration, if changed */
    changed = (packet_info->add == packet_info_old.add ? 0 : 1);
    memset(&filter, 0, sizeof(filter));
    for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
        filter.src_port_mask[i] = packet_info->ports[i];
        if (packet_info->ports[i] != packet_info_old.ports[i]) {
            changed = 1;
        }
    }
    if (changed) {
        /* Remove old registration */
        if (packet_info_old.filter_id != NULL) {
            T_D("remove packet registration");
            if (packet_rx_filter_unregister(packet_info_old.filter_id) != VTSS_OK) {
                T_W("packet deregistration failed");
            }
        }

        /* Add new registration */
        if (packet_info->add) {
            T_D("add packet registration");
            filter.prio  = PACKET_RX_FILTER_PRIO_NORMAL;
            filter.modid = VTSS_MODULE_ID_EVC;
            filter.match = (PACKET_RX_FILTER_MATCH_ACL | PACKET_RX_FILTER_MATCH_SRC_PORT);
            filter.cb = evc_packet_rx;
            if (packet_rx_filter_register(&filter, &packet_info->filter_id) != VTSS_OK) {
                T_W("packet registration failed");
            }
        }
    } else {
        /* No change, preserve old packet registration */
        packet_info->filter_id = packet_info_old.filter_id;
    }
}
#endif /* VTSS_ARCH_SERVAL */

/* - Port Configuration -------------------------------------------- */

/* Convert internal port configuration to management API port configuration */
static void evc_port2mgmt_conf(evc_mgmt_port_conf_t *api, evc_port_conf_t *conf)
{
    u32 i;

    memset(api, 0, sizeof(*api));
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_CARACAL)
    api->conf.dei_colouring = (conf->flags & EVC_PORT_FLAG_DEI_COLOURING ? 1 : 0);
#endif /* VTSS_ARCH_JAGUAR_1/CARACAL */
#if defined(VTSS_ARCH_SERVAL)
    api->conf.key_type = conf->key_type;
    api->vcap_conf.key_type_is1_1 = conf->key_type_adv;
    api->vcap_conf.dmac_dip_1 = (conf->flags & EVC_PORT_FLAG_DMAC_DIP_ADV ? 1 : 0);
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_ARCH_CARACAL)
    api->conf.inner_tag = (conf->flags & EVC_PORT_FLAG_INNER_TAG ? 1 : 0);
#endif /* VTSS_ARCH_CARACAL */
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    api->conf.dmac_dip = (conf->flags & EVC_PORT_FLAG_DMAC_DIP ? 1 : 0);
#endif /* VTSS_ARCH_CARACAL/SERVAL */
    for (i = 0; i < 16; i++) {
        /* BPDU, convert NORMAL to CPU_ONLY */
        api->reg.bpdu_reg[i] = conf->bpdu[i];
        if (conf->bpdu[i] == VTSS_PACKET_REG_NORMAL)
            conf->bpdu[i] = VTSS_PACKET_REG_CPU_ONLY;

        /* GARP, convert NORMAL to FORWARD */
        api->reg.garp_reg[i] = conf->garp[i];
        if (conf->garp[i] == VTSS_PACKET_REG_NORMAL)
            conf->garp[i] = VTSS_PACKET_REG_FORWARD;
    }
}

/* Convert management API port configuration to internal port configuration */
static void evc_mgmt2port_conf(evc_mgmt_port_conf_t *api, evc_port_conf_t *conf)
{
    u32 i;

    memset(conf, 0, sizeof(*conf));
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_CARACAL)
    if (api->conf.dei_colouring)
        conf->flags |=  EVC_PORT_FLAG_DEI_COLOURING;
#endif /* VTSS_ARCH_JAGUAR_1/CARACAL */
#if defined(VTSS_ARCH_SERVAL)
    conf->key_type = api->conf.key_type;
    conf->key_type_adv = api->vcap_conf.key_type_is1_1;
    if (api->vcap_conf.dmac_dip_1)
        conf->flags |= EVC_PORT_FLAG_DMAC_DIP_ADV;
#endif /* VTSS_ARCH_SERVAL */
#if defined(VTSS_ARCH_CARACAL)
    if (api->conf.inner_tag)
        conf->flags |= EVC_PORT_FLAG_INNER_TAG;
#endif /* VTSS_ARCH_CARACAL */
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    if (api->conf.dmac_dip)
        conf->flags |= EVC_PORT_FLAG_DMAC_DIP;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
    for (i = 0; i < 16; i++) {
        conf->bpdu[i] = api->reg.bpdu_reg[i];
        conf->garp[i] = api->reg.garp_reg[i];
    }
}

/* Set port configuration */
static vtss_rc evc_port_conf_set(vtss_port_no_t port_no, evc_port_conf_t *new)
{
    vtss_rc              rc, rc2;
    evc_mgmt_port_conf_t conf;

    evc_port2mgmt_conf(&conf, new);
    evc_global.port_conf[port_no] = *new;
    rc = vtss_evc_port_conf_set(NULL, port_no, &conf.conf);
#if defined(VTSS_ARCH_SERVAL)
    if ((rc2 = vtss_vcap_port_conf_set(NULL, port_no, &conf.vcap_conf)) != VTSS_RC_OK) {
        rc = rc2;
    }
#endif /* VTSS_ARCH_SERVAL */
    if ((rc2 = vtss_packet_rx_port_conf_set(NULL, port_no, &conf.reg)) != VTSS_RC_OK) {
        rc = rc2;
    }

    return rc;
}

/* - Policer Configuration ----------------------------------------- */

/* Set policer configuration */
static vtss_rc evc_policer_conf_set(vtss_evc_policer_id_t policer_id, evc_pol_conf_t *new)
{
    evc_global.pol_conf[policer_id] = *new;

    return vtss_evc_policer_conf_set(NULL, policer_id, &new->conf);
}

/* - EVC Configuration --------------------------------------------- */

/* Convert internal EVC configuration to API EVC configuration */
static void evc_evc2api_conf(vtss_evc_conf_t *api, evc_conf_t *conf)
{
    port_iter_t             pit;
    vtss_port_no_t          port_no;
    vtss_evc_pb_conf_t      *pb   = &api->network.pb;
#if defined(VTSS_FEATURE_MPLS)
    vtss_evc_mpls_tp_conf_t *mpls = &api->network.mpls_tp;
#endif

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    api->policer_id = conf->policer_id;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    api->learning = (conf->flags & EVC_FLAG_LEARN ? 1 : 0);
    port_iter_init_local_all(&pit);
    while (port_iter_getnext(&pit)) {
        port_no = pit.iport;
        pb->nni[port_no] = VTSS_PORT_BF_GET(conf->ports, port_no);
    }
    pb->ivid = conf->ivid;
    pb->vid = conf->vid;
#if defined(VTSS_ARCH_CARACAL)
    {
        vtss_evc_inner_tag_t *it = &pb->inner_tag;

        pb->uvid = conf->uvid;
        it->type = ((conf->flags & EVC_FLAG_IT_C) ? VTSS_EVC_INNER_TAG_C :
                    (conf->flags & EVC_FLAG_IT_S) ? VTSS_EVC_INNER_TAG_S :
                    (conf->flags & EVC_FLAG_IT_S_CUSTOM) ? VTSS_EVC_INNER_TAG_S_CUSTOM : 
                    VTSS_EVC_INNER_TAG_NONE);
        it->vid_mode = (conf->flags & EVC_FLAG_VID_TUNNEL ? VTSS_EVC_VID_MODE_TUNNEL :
                        VTSS_EVC_VID_MODE_NORMAL);
        it->vid = conf->it_vid;
        it->pcp_dei_preserve = (conf->flags & EVC_FLAG_IT_PRES ? 1 : 0);
        it->pcp = conf->it_pcp;
        it->dei = (conf->flags & EVC_FLAG_IT_DEI ? 1 : 0);
    }
#endif /* VTSS_ARCH_CARACAL */
#if defined(VTSS_FEATURE_MPLS)
    mpls->pw_egress_xc  = conf->pw_egress_xc;
    mpls->pw_ingress_xc = conf->pw_ingress_xc;
#endif /* VTSS_FEATURE_MPLS */
}

/* Convert API EVC configuration to internal EVC configuration */
static void evc_api2evc_conf(vtss_evc_conf_t *api, evc_conf_t *conf)
{
    port_iter_t             pit;
    vtss_port_no_t          port_no;
    vtss_evc_pb_conf_t      *pb   = &api->network.pb;
#if defined(VTSS_FEATURE_MPLS)
    vtss_evc_mpls_tp_conf_t *mpls = &api->network.mpls_tp;
#endif

    memset(conf, 0, sizeof(*conf));

    conf->flags |= EVC_FLAG_ENABLED;
    conf->ivid = pb->ivid;
    conf->vid = pb->vid;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    conf->policer_id = api->policer_id;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    if (api->learning)
        conf->flags |= EVC_FLAG_LEARN;
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        port_no = pit.iport;
        if (pb->nni[port_no]) {
            VTSS_PORT_BF_SET(conf->ports, port_no, 1);
        }
    }
#if defined(VTSS_ARCH_CARACAL)
    {
        vtss_evc_inner_tag_t *it = &pb->inner_tag;

        conf->uvid = pb->uvid;
        if (it->type == VTSS_EVC_INNER_TAG_C)
            conf->flags |= EVC_FLAG_IT_C;
        else if (it->type == VTSS_EVC_INNER_TAG_S)
            conf->flags |= EVC_FLAG_IT_S;
        else if (it->type == VTSS_EVC_INNER_TAG_S_CUSTOM)
            conf->flags |= EVC_FLAG_IT_S_CUSTOM;
        if (it->vid_mode == VTSS_EVC_VID_MODE_TUNNEL)
            conf->flags |= EVC_FLAG_VID_TUNNEL;
        conf->it_vid = it->vid;
        if (it->pcp_dei_preserve)
            conf->flags |= EVC_FLAG_IT_PRES;
        conf->it_pcp = it->pcp;
        if (it->dei)
            conf->flags |= EVC_FLAG_IT_DEI;
    }
#endif /* VTSS_ARCH_CARACAL */

#if defined(VTSS_FEATURE_MPLS)
    conf->pw_egress_xc  = mpls->pw_egress_xc;
    conf->pw_ingress_xc = mpls->pw_ingress_xc;
#endif /* VTSS_FEATURE_MPLS */
}

/* Set (add/delete) EVC configuration */
static vtss_rc evc_conf_set(vtss_evc_id_t evc_id, evc_conf_t *new)
{
    vtss_rc         rc = VTSS_RC_ERROR, rc2;
    evc_conf_t      *old = &evc_global.evc_table[evc_id];
    vtss_evc_conf_t conf;
    vtss_vid_t      vid_old, vid_new = VTSS_VID_NULL;

    vid_old = (EVC_VLAN_ENABLED(old) ? old->ivid : VTSS_VID_NULL);
    if (new != NULL && (new->flags & EVC_FLAG_ENABLED)) {
        /* Add/modify EVC */
        if (EVC_VLAN_ENABLED(new)) {
            vid_new = new->ivid;
        }
        evc_evc2api_conf(&conf, new);
        if ((rc = vtss_evc_add(NULL, evc_id, &conf)) == VTSS_RC_OK) {
            new->flags &= ~EVC_FLAG_CONFLICT;
        } else {
            new->flags |= EVC_FLAG_CONFLICT;
        }
        *old = *new;
        evc_change_flag_set(evc_id);
    } else if (old->flags & EVC_FLAG_ENABLED) {
        /* Delete EVC */
        if (old->flags & EVC_FLAG_LEARN) {
            vid_old = old->ivid;
        }
        rc = (old->flags & EVC_FLAG_CONFLICT ? VTSS_RC_OK : vtss_evc_del(NULL, evc_id));
        memset(old, 0, sizeof(*old));
        evc_change_flag_set(evc_id);
    }

    /* Update old and new VLAN */
    if (vid_old != VTSS_VID_NULL && (rc2 = evc_vlan_update(vid_old)) != VTSS_RC_OK) {
        rc = rc2;
    }
    if (vid_new != VTSS_VID_NULL && vid_new != vid_old &&
        (rc2 = evc_vlan_update(vid_new)) != VTSS_RC_OK) {
        rc = rc2;
    }

    return rc;
}

/* - ECE Configuration --------------------------------------------- */

static vtss_vcap_bit_t evc_key_bit_get(evc_ece_conf_t *ece, u32 mask_vld, u32 mask_1)
{
    return ((ece->flags_key & mask_1) ? VTSS_VCAP_BIT_1 : 
            (ece->flags_key & mask_vld) ? VTSS_VCAP_BIT_0 : VTSS_VCAP_BIT_ANY);
}

/* Convert internal ECE range to API range */
static void evc_ece2api_range(vtss_vcap_vr_t *api, evc_u16_range_t *range, 
                              evc_ece_conf_t *conf, u32 mask)
{
    if (conf->flags_key & mask) {
        api->type = VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE;
        api->vr.r.low = range->low; 
        api->vr.r.high = range->high;
    } else {
        api->type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
        api->vr.v.value = range->low;
        api->vr.v.mask = range->high;
    }
}

/* Convert API range to internal ECE range */
static void evc_api2ece_range(vtss_vcap_vr_t *api, evc_u16_range_t *range, 
                              evc_ece_conf_t *conf, u32 mask)
{
    switch (api->type) {
    case VTSS_VCAP_VR_TYPE_VALUE_MASK:
        range->low = api->vr.v.value;
        range->high = api->vr.v.mask;
        break;
    case VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE:
        range->low = api->vr.r.low;
        range->high = api->vr.r.high;
        conf->flags_key |= mask;
        break;
    default:
        T_E("illegal type, mask: 0x%08x", mask);
        break;
    }
}

/* Convert internal ECE configuration to management API ECE configuration */
static void evc_ece2mgmt_conf(evc_mgmt_ece_conf_t *ece, evc_ece_conf_t *conf, BOOL mgmt)
{
#if defined(VTSS_ARCH_SERVAL)
    evc_l2cp_data_t       *l2cp = &ece->data.l2cp;
#endif /* VTSS_ARCH_SERVAL */
    vtss_ece_t            *api = &ece->conf;
    vtss_ece_key_t        *key = &api->key;
    vtss_ece_tag_t        *tag = &key->tag;
    vtss_ece_frame_ipv4_t *ipv4 = &key->frame.ipv4;
    vtss_ece_frame_ipv6_t *ipv6 = &key->frame.ipv6;
    vtss_ece_action_t     *action = &api->action;
    vtss_ece_outer_tag_t  *ot = &action->outer_tag;
    port_iter_t           pit;
    vtss_port_no_t        port_no;
    evc_u16_range_t       range;
    u32                   flags;
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    int                   i, j;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
    
    /* Clear entire structure */
    memset(ece, 0, sizeof(*ece));

    /* ECE type */
    key->type = conf->type;
    api->id = conf->ece_id;

    /* Key */
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        port_no = pit.iport;
        if (VTSS_PORT_BF_GET(conf->ports, port_no)) {
            key->port_list[port_no] = VTSS_ECE_PORT_ROOT;
        }
    }

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    key->mac.dmac_mc = evc_key_bit_get(conf, ECE_FLAG_KEY_DMAC_MC_VLD, ECE_FLAG_KEY_DMAC_MC_1);
    key->mac.dmac_bc = evc_key_bit_get(conf, ECE_FLAG_KEY_DMAC_BC_VLD, ECE_FLAG_KEY_DMAC_BC_1);
    key->mac.smac = conf->smac;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    key->mac.dmac = conf->dmac;
    key->lookup = (conf->flags_key & ECE_FLAG_KEY_LOOKUP ? 1 : 0);
#endif /* VTSS_ARCH_SERVAL */

    evc_ece2api_range(&tag->vid, &conf->vid, conf, ECE_FLAG_KEY_VID_RANGE);
    tag->pcp = conf->pcp;
    tag->dei = evc_key_bit_get(conf, ECE_FLAG_KEY_DEI_VLD, ECE_FLAG_KEY_DEI_1);
    tag->tagged = evc_key_bit_get(conf, ECE_FLAG_KEY_TAGGED_VLD, ECE_FLAG_KEY_TAGGED_1);
    tag->s_tagged = evc_key_bit_get(conf, ECE_FLAG_KEY_S_TAGGED_VLD, ECE_FLAG_KEY_S_TAGGED_1);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    tag = &key->inner_tag;
    evc_ece2api_range(&tag->vid, &conf->in_vid, conf, ECE_FLAG_KEY_IN_VID_RANGE);
    tag->pcp = conf->in_pcp;
    tag->dei = evc_key_bit_get(conf, ECE_FLAG_KEY_IN_DEI_VLD, ECE_FLAG_KEY_IN_DEI_1);
    tag->tagged = evc_key_bit_get(conf, ECE_FLAG_KEY_IN_TAGGED_VLD, ECE_FLAG_KEY_IN_TAGGED_1);
    tag->s_tagged = evc_key_bit_get(conf, ECE_FLAG_KEY_IN_S_TAGGED_VLD,
                                    ECE_FLAG_KEY_IN_S_TAGGED_1);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    
    switch (conf->type) {
#if defined(VTSS_ARCH_SERVAL)
    case VTSS_ECE_TYPE_ETYPE:
        key->frame.etype = conf->data.etype;
        break;
    case VTSS_ECE_TYPE_LLC:
        key->frame.llc = conf->data.llc;
        break;
    case VTSS_ECE_TYPE_SNAP:
        key->frame.snap = conf->data.snap;
        break;
#endif /* VTSS_ARCH_SERVAL */
    case VTSS_ECE_TYPE_IPV4:
        range.low = conf->data.ipv4.dscp.low;
        range.high = conf->data.ipv4.dscp.high;
        evc_ece2api_range(&ipv4->dscp, &range, conf, ECE_FLAG_KEY_DSCP_RANGE);
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        ipv4->proto = conf->data.ipv4.proto;
        ipv4->fragment = evc_key_bit_get(conf, ECE_FLAG_KEY_IPV4_FRAG_VLD, 
                                         ECE_FLAG_KEY_IPV4_FRAG_1);
        ipv4->sip = conf->data.ipv4.sip;
#if defined(VTSS_ARCH_SERVAL)
        ipv4->dip = conf->data.ipv4.dip;
#endif /* VTSS_ARCH_SERVAL */
        evc_ece2api_range(&ipv4->sport, &conf->data.ipv4.sport, conf, ECE_FLAG_KEY_SPORT_RANGE);
        evc_ece2api_range(&ipv4->dport, &conf->data.ipv4.dport, conf, ECE_FLAG_KEY_DPORT_RANGE);
#endif /* VTSS_ARCH_CARACAL/SERVAL */
        break;
    case VTSS_ECE_TYPE_IPV6:
        range.low = conf->data.ipv6.dscp.low;
        range.high = conf->data.ipv6.dscp.high;
        evc_ece2api_range(&ipv6->dscp, &range, conf, ECE_FLAG_KEY_DSCP_RANGE);
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        ipv6->proto = conf->data.ipv6.proto;
        for (i = 0; i < 4; i++) {
            j = (i + 12);
            ipv6->sip.value[j] = conf->data.ipv6.sip.value[i];
            ipv6->sip.mask[j] = conf->data.ipv6.sip.mask[i];
#if defined(VTSS_ARCH_SERVAL)
            ipv6->dip.value[j] = conf->data.ipv6.dip.value[i];
            ipv6->dip.mask[j] = conf->data.ipv6.dip.mask[i];
#endif /* VTSS_ARCH_SERVAL */
        }
        evc_ece2api_range(&ipv6->sport, &conf->data.ipv6.sport, conf, ECE_FLAG_KEY_SPORT_RANGE);
        evc_ece2api_range(&ipv6->dport, &conf->data.ipv6.dport, conf, ECE_FLAG_KEY_DPORT_RANGE);
#endif /* VTSS_ARCH_CARACAL/SERVAL */
        break;
    default:
        break;
    }
#if defined(VTSS_ARCH_SERVAL)
    if (conf->flags_key & ECE_FLAG_KEY_TYPE_L2CP) {
        l2cp->proto = conf->data.l2cp;
    }
#endif /* VTSS_ARCH_SERVAL */
    
    /* Action */
    flags = conf->flags_act;
    action->dir = ((flags & ECE_FLAG_ACT_DIR_UNI_TO_NNI) ? VTSS_ECE_DIR_UNI_TO_NNI :
                   (flags & ECE_FLAG_ACT_DIR_NNI_TO_UNI) ? VTSS_ECE_DIR_NNI_TO_UNI :
                   VTSS_ECE_DIR_BOTH);
    action->pop_tag = ((flags & ECE_FLAG_ACT_POP_2) ? VTSS_ECE_POP_TAG_2 : 
                       (flags & ECE_FLAG_ACT_POP_1) ? VTSS_ECE_POP_TAG_1 : 
                       VTSS_ECE_POP_TAG_0);
    ot->enable = (flags & ECE_FLAG_ACT_OT_ENA ? 1 : 0);
#if defined(VTSS_ARCH_SERVAL)
    action->rule = (flags & ECE_FLAG_ACT_RULE_RX ? VTSS_ECE_RULE_RX :
                    flags & ECE_FLAG_ACT_RULE_TX ? VTSS_ECE_RULE_TX : VTSS_ECE_RULE_BOTH);
    action->tx_lookup = (flags & ECE_FLAG_ACT_TX_LOOKUP_VID ? VTSS_ECE_TX_LOOKUP_VID :
                         flags & ECE_FLAG_ACT_TX_LOOKUP_VID_PCP ? VTSS_ECE_TX_LOOKUP_VID_PCP :
                         VTSS_ECE_TX_LOOKUP_ISDX);
    ot->pcp_mode = (flags & ECE_FLAG_ACT_OT_PCP_MODE_CLASS ? VTSS_ECE_PCP_MODE_CLASSIFIED :
                    flags & ECE_FLAG_ACT_OT_PCP_MODE_MAP ? VTSS_ECE_PCP_MODE_MAPPED : 
                    VTSS_ECE_PCP_MODE_FIXED);
#else
    ot->pcp_dei_preserve = (flags & ECE_FLAG_ACT_OT_PCP_MODE_CLASS ? 1 : 0);
#endif /* VTSS_ARCH_SERVAL */
    ot->pcp = conf->ot_pcp;
#if defined(VTSS_ARCH_SERVAL)
    ot->dei_mode = ((flags & ECE_FLAG_ACT_OT_DEI_MODE_CLASS) ? VTSS_ECE_DEI_MODE_CLASSIFIED :
                    (flags & ECE_FLAG_ACT_OT_DEI_MODE_FIXED) ? VTSS_ECE_DEI_MODE_FIXED :
                    VTSS_ECE_DEI_MODE_DP);
#endif /* VTSS_ARCH_SERVAL */
    ot->dei = (flags & ECE_FLAG_ACT_OT_DEI ? 1 : 0);
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    {
        vtss_ece_inner_tag_t *it = &action->inner_tag;
        
        ot->vid = conf->ot_vid;
        it->type = ((flags & ECE_FLAG_ACT_IT_TYPE_C) ? VTSS_ECE_INNER_TAG_C :
                    (flags & ECE_FLAG_ACT_IT_TYPE_S) ? VTSS_ECE_INNER_TAG_S :
                    (flags & ECE_FLAG_ACT_IT_TYPE_S_CUSTOM) ? 
                    VTSS_ECE_INNER_TAG_S_CUSTOM : VTSS_ECE_INNER_TAG_NONE);
        it->vid = conf->it_vid;
#if defined(VTSS_ARCH_SERVAL)
        it->pcp_mode = (flags & ECE_FLAG_ACT_IT_PCP_MODE_CLASS ? VTSS_ECE_PCP_MODE_CLASSIFIED : 
                        flags & ECE_FLAG_ACT_IT_PCP_MODE_MAP ? VTSS_ECE_PCP_MODE_MAPPED : 
                        VTSS_ECE_PCP_MODE_FIXED);
#else
        it->pcp_dei_preserve = (flags & ECE_FLAG_ACT_IT_PCP_MODE_CLASS ? 1 : 0);
#endif /* VTSS_ARCH_SERVAL */
        it->pcp = conf->it_pcp;
#if defined(VTSS_ARCH_SERVAL)
        it->dei_mode = ((flags & ECE_FLAG_ACT_IT_DEI_MODE_CLASS) ? VTSS_ECE_DEI_MODE_CLASSIFIED :
                        (flags & ECE_FLAG_ACT_IT_DEI_MODE_FIXED) ? VTSS_ECE_DEI_MODE_FIXED :
                        VTSS_ECE_DEI_MODE_DP);
#endif /* VTSS_ARCH_SERVAL */
        it->dei = (flags & ECE_FLAG_ACT_IT_DEI ? 1 : 0);
        action->policer_id = conf->policer_id;
    }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    action->evc_id = conf->evc_id;
    action->policy_no = conf->policy_no;
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    action->prio_enable = (flags & ECE_FLAG_ACT_PRIO_ENA ? 1 : 0);
    action->prio = conf->prio;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    action->dp_enable = (flags & ECE_FLAG_ACT_DP_ENA ? 1 : 0);
    action->dp = conf->dp;
    l2cp->mode = (flags & ECE_FLAG_ACT_L2CP_TUNNEL ? EVC_L2CP_MODE_TUNNEL :
                  flags & ECE_FLAG_ACT_L2CP_DISCARD ? EVC_L2CP_MODE_DISCARD :
                  flags & ECE_FLAG_ACT_L2CP_PEER ? EVC_L2CP_MODE_PEER : EVC_L2CP_MODE_FORWARD);
    l2cp->dmac = (flags & ECE_FLAG_ACT_L2CP_DMAC_CISCO ? EVC_L2CP_DMAC_CISCO : EVC_L2CP_DMAC_CUSTOM);

    if (mgmt) {
        /* No further processing for normal management API conversion */
        return;
    }

    /* Further ECE structure updates done before calling VTSS API */
    if (l2cp->proto != EVC_L2CP_NONE) {
        evc_l2cp_info_t *info = &evc_global.l2cp_info[l2cp->proto];

        /* Match L2CP DMAC */
        for (i = 0; i < 6; i++) {
            key->mac.dmac.value[i] = info->dmac.addr[i];
            key->mac.dmac.mask[i] = 0xff;
        }

        /* Match L2CP frame type and data */
        key->type = info->type;
        switch (key->type) {
        case VTSS_ECE_TYPE_ETYPE:
            for (i = 0; i < 2; i++) {
                j = (i == 0 ? 8 : 0);
                key->frame.etype.etype.value[i] = ((info->etype >> j) & 0xff);
                key->frame.etype.etype.mask[i] = 0xff;
                key->frame.etype.data.value[i] = info->pid.value[i];
                key->frame.etype.data.mask[i] = info->pid.mask[i];
            }
            break;
        case VTSS_ECE_TYPE_LLC:
            for (i = 0; i < 5; i++) {
                j = (i - 3);
                key->frame.llc.data.value[i] = (i < 2 ? 0x42 : i == 2 ? 0x03 : info->pid.value[j]);
                key->frame.llc.data.mask[i] = (i < 3 ? 0xff : info->pid.mask[j]);
            }
            break;
        case VTSS_ECE_TYPE_SNAP:
            for (i = 0; i < 5; i++) {
                j = (i - 3);
                key->frame.snap.data.value[i] = (i < 2 ? 0x00 : i == 2 ? 0x0c : info->pid.value[j]);
                key->frame.snap.data.mask[i] = (i < 3 ? 0xff : info->pid.mask[j]);
            }
            break;
        default:
            break;
        }
    }

    /* Adjust ACL policy depending on L2CP mode */
    switch (l2cp->mode) {
    case EVC_L2CP_MODE_TUNNEL:
        action->policy_no = ACL_POLICY_CPU_REDIR;
        if (action->dir == VTSS_ECE_DIR_NNI_TO_UNI) {
            /* NNI-to-UNI, avoid ECE match, only ACL matching used */
            key->mac.dmac_mc = VTSS_VCAP_BIT_0;
            key->mac.dmac_bc = VTSS_VCAP_BIT_1;
        }
        break;
    case EVC_L2CP_MODE_PEER:
        action->policy_no = ACL_POLICY_CPU_REDIR;
        break;
    case EVC_L2CP_MODE_DISCARD:
        action->policy_no = ACL_POLICY_DISCARD;
        break;
    default:
        break;
    }

    /* Adjust direction depending on L2CP mode */
    if (l2cp->mode != EVC_L2CP_MODE_FORWARD) {
        if (action->dir == VTSS_ECE_DIR_BOTH) {
            /* Only UNI-to-NNI ECE is used */
            action->dir = VTSS_ECE_DIR_UNI_TO_NNI;
        }
        action->rule = VTSS_ECE_RULE_RX;
    }

    /* Store frame type, DMAC and data given to API */
    conf->frame_type = key->type;
    conf->frame_dmac = key->mac.dmac;
    switch (key->type) {
    case VTSS_ECE_TYPE_ETYPE:
        for (i = 0; i < 4; i++) {
            conf->frame_data.value[i] = (i < 2 ? key->frame.etype.etype.value[i] : key->frame.etype.data.value[i - 2]);
            conf->frame_data.mask[i] = (i < 2 ? key->frame.etype.etype.mask[i] : key->frame.etype.data.mask[i - 2]);
        }
        conf->frame_data.mask[4] = 0x00;
        break;
    case VTSS_ECE_TYPE_LLC:
        for (i = 0; i < 5; i++) {
            conf->frame_data.value[i] = key->frame.llc.data.value[i];
            conf->frame_data.mask[i] = key->frame.llc.data.mask[i];
        }
        break;
    case VTSS_ECE_TYPE_SNAP:
        for (i = 0; i < 5; i++) {
            conf->frame_data.value[i] = key->frame.snap.data.value[i];
            conf->frame_data.mask[i] = key->frame.snap.data.mask[i];
        }
        break;
    default:
        break;
    }
#endif /* VTSS_ARCH_SERVAL */
}

static void evc_key_bit_set(vtss_vcap_bit_t fld, evc_ece_conf_t *ece, u32 mask_vld, u32 mask_1)
{
    if (fld == VTSS_VCAP_BIT_0)
        ece->flags_key |= mask_vld;
    else if (fld == VTSS_VCAP_BIT_1)
        ece->flags_key |= (mask_vld | mask_1);
}

/* Convert management API ECE configuration to internal ECE configuration */
static void evc_mgmt2ece_conf(evc_mgmt_ece_conf_t *ece, evc_ece_conf_t *conf)
{
#if defined(VTSS_ARCH_SERVAL)
    evc_l2cp_data_t       *l2cp = &ece->data.l2cp;
#endif /* VTSS_ARCH_SERVAL */
    vtss_ece_t            *api = &ece->conf;
    vtss_ece_key_t        *key = &api->key;
    vtss_ece_tag_t        *tag = &key->tag;
    vtss_ece_frame_ipv4_t *ipv4 = &key->frame.ipv4;
    vtss_ece_frame_ipv6_t *ipv6 = &key->frame.ipv6;
    vtss_ece_action_t     *action = &api->action;
    vtss_ece_outer_tag_t  *ot = &action->outer_tag;
    port_iter_t           pit;
    vtss_port_no_t        port_no;
    evc_u16_range_t       range;

    memset(conf, 0, sizeof(*conf));
    conf->ece_id = api->id;

    /* Key */
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        port_no = pit.iport;
        if (key->port_list[port_no] != VTSS_ECE_PORT_NONE) {
            VTSS_PORT_BF_SET(conf->ports, port_no, 1);
        }
    }

#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    evc_key_bit_set(key->mac.dmac_mc, conf, ECE_FLAG_KEY_DMAC_MC_VLD, ECE_FLAG_KEY_DMAC_MC_1);
    evc_key_bit_set(key->mac.dmac_bc, conf, ECE_FLAG_KEY_DMAC_BC_VLD, ECE_FLAG_KEY_DMAC_BC_1);
    conf->smac = key->mac.smac;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    conf->dmac = key->mac.dmac;
    if (key->lookup)
        conf->flags_key |= ECE_FLAG_KEY_LOOKUP;
#endif /* VTSS_ARCH_SERVAL */

    evc_api2ece_range(&tag->vid, &conf->vid, conf, ECE_FLAG_KEY_VID_RANGE);
    conf->pcp = tag->pcp;
    evc_key_bit_set(tag->dei, conf, ECE_FLAG_KEY_DEI_VLD, ECE_FLAG_KEY_DEI_1);
    evc_key_bit_set(tag->tagged, conf, ECE_FLAG_KEY_TAGGED_VLD, ECE_FLAG_KEY_TAGGED_1);
    evc_key_bit_set(tag->s_tagged, conf, ECE_FLAG_KEY_S_TAGGED_VLD, ECE_FLAG_KEY_S_TAGGED_1);

#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    tag = &key->inner_tag;
    evc_api2ece_range(&tag->vid, &conf->in_vid, conf, ECE_FLAG_KEY_IN_VID_RANGE);
    conf->in_pcp = tag->pcp;
    evc_key_bit_set(tag->dei, conf, ECE_FLAG_KEY_IN_DEI_VLD, ECE_FLAG_KEY_IN_DEI_1);
    evc_key_bit_set(tag->tagged, conf, ECE_FLAG_KEY_IN_TAGGED_VLD, ECE_FLAG_KEY_IN_TAGGED_1);
    evc_key_bit_set(tag->s_tagged, conf, ECE_FLAG_KEY_IN_S_TAGGED_VLD, ECE_FLAG_KEY_IN_S_TAGGED_1);
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */

    conf->type = key->type;
#if defined(VTSS_ARCH_SERVAL)
    if (l2cp->proto != EVC_L2CP_NONE) {
        conf->type = VTSS_ECE_TYPE_ANY;
        conf->flags_key |= ECE_FLAG_KEY_TYPE_L2CP;
        conf->data.l2cp = l2cp->proto;
    }
#endif /* VTSS_ARCH_SERVAL */

    switch (conf->type) {
#if defined(VTSS_ARCH_SERVAL)
    case VTSS_ECE_TYPE_ETYPE:
        conf->data.etype = key->frame.etype;
        break;
    case VTSS_ECE_TYPE_LLC:
        conf->data.llc = key->frame.llc;
        break;
    case VTSS_ECE_TYPE_SNAP:
        conf->data.snap = key->frame.snap;
        break;
#endif /* VTSS_ARCH_SERVAL */
    case VTSS_ECE_TYPE_IPV4:
        evc_api2ece_range(&ipv4->dscp, &range, conf, ECE_FLAG_KEY_DSCP_RANGE);
        conf->data.ipv4.dscp.low = range.low;
        conf->data.ipv4.dscp.high = range.high;
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        conf->data.ipv4.proto = ipv4->proto;
        evc_key_bit_set(ipv4->fragment, conf, 
                        ECE_FLAG_KEY_IPV4_FRAG_VLD, ECE_FLAG_KEY_IPV4_FRAG_1);
        conf->data.ipv4.sip = ipv4->sip;
#if defined(VTSS_ARCH_SERVAL)
        conf->data.ipv4.dip = ipv4->dip;
#endif /* VTSS_ARCH_SERVAL */
        evc_api2ece_range(&ipv4->sport, &conf->data.ipv4.sport, conf, ECE_FLAG_KEY_SPORT_RANGE);
        evc_api2ece_range(&ipv4->dport, &conf->data.ipv4.dport, conf, ECE_FLAG_KEY_DPORT_RANGE);
#endif /* VTSS_ARCH_CARACAL/SERVAL */
        break;
    case VTSS_ECE_TYPE_IPV6:
        evc_api2ece_range(&ipv6->dscp, &range, conf, ECE_FLAG_KEY_DSCP_RANGE);
        conf->data.ipv6.dscp.low = range.low;
        conf->data.ipv6.dscp.high = range.high;
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
        conf->data.ipv6.proto = ipv6->proto;
        {
            int i, j;
            
            for (i = 0; i < 4; i++) {
                j = (i + 12);
                conf->data.ipv6.sip.value[i] = ipv6->sip.value[j];
                conf->data.ipv6.sip.mask[i] = ipv6->sip.mask[j];
#if defined(VTSS_ARCH_SERVAL)
                conf->data.ipv6.dip.value[i] = ipv6->dip.value[j];
                conf->data.ipv6.dip.mask[i] = ipv6->dip.mask[j];
#endif /* VTSS_ARCH_SERVAL */
            }
        }
        evc_api2ece_range(&ipv6->sport, &conf->data.ipv6.sport, conf, ECE_FLAG_KEY_SPORT_RANGE);
        evc_api2ece_range(&ipv6->dport, &conf->data.ipv6.dport, conf, ECE_FLAG_KEY_DPORT_RANGE);
#endif /* VTSS_ARCH_CARACAL/SERVAL */
        break;
    default:
        break;
    }

    /* Action */
    if (action->dir == VTSS_ECE_DIR_UNI_TO_NNI)
        conf->flags_act |= ECE_FLAG_ACT_DIR_UNI_TO_NNI;
    else if (action->dir == VTSS_ECE_DIR_NNI_TO_UNI)
        conf->flags_act |= ECE_FLAG_ACT_DIR_NNI_TO_UNI;
    if (action->pop_tag == VTSS_ECE_POP_TAG_2)
        conf->flags_act |= ECE_FLAG_ACT_POP_2;
    else if (action->pop_tag == VTSS_ECE_POP_TAG_1)
        conf->flags_act |= ECE_FLAG_ACT_POP_1;
    if (ot->enable)
        conf->flags_act |= ECE_FLAG_ACT_OT_ENA;
#if defined(VTSS_ARCH_SERVAL)
    if (action->rule == VTSS_ECE_RULE_RX)
        conf->flags_act |= ECE_FLAG_ACT_RULE_RX;
    else if (action->rule == VTSS_ECE_RULE_TX)
        conf->flags_act |= ECE_FLAG_ACT_RULE_TX;
    if (action->tx_lookup == VTSS_ECE_TX_LOOKUP_VID)
        conf->flags_act |= ECE_FLAG_ACT_TX_LOOKUP_VID;
    else if (action->tx_lookup == VTSS_ECE_TX_LOOKUP_VID_PCP)
        conf->flags_act |= ECE_FLAG_ACT_TX_LOOKUP_VID_PCP;
    if (ot->pcp_mode == VTSS_ECE_PCP_MODE_CLASSIFIED)
        conf->flags_act |= ECE_FLAG_ACT_OT_PCP_MODE_CLASS;
    else if (ot->pcp_mode == VTSS_ECE_PCP_MODE_MAPPED)
        conf->flags_act |= ECE_FLAG_ACT_OT_PCP_MODE_MAP;
#else
    if (ot->pcp_dei_preserve)
        conf->flags_act |= ECE_FLAG_ACT_OT_PCP_MODE_CLASS;
#endif /* VTSS_ARCH_SERVAL */
    conf->ot_pcp = ot->pcp;
#if defined(VTSS_ARCH_SERVAL)
    if (ot->dei_mode == VTSS_ECE_DEI_MODE_CLASSIFIED)
        conf->flags_act |= ECE_FLAG_ACT_OT_DEI_MODE_CLASS;
    else if (ot->dei_mode == VTSS_ECE_DEI_MODE_FIXED)
        conf->flags_act |= ECE_FLAG_ACT_OT_DEI_MODE_FIXED;
    else
        conf->flags_act |= ECE_FLAG_ACT_OT_DEI_MODE_DP;
#endif /* VTSS_ARCH_SERVAL */
    if (ot->dei) 
        conf->flags_act |= ECE_FLAG_ACT_OT_DEI;
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    {
        vtss_ece_inner_tag_t *it = &action->inner_tag;
        
        conf->ot_vid = ot->vid;
        if (it->type == VTSS_ECE_INNER_TAG_C)
            conf->flags_act |= ECE_FLAG_ACT_IT_TYPE_C;
        else if (it->type == VTSS_ECE_INNER_TAG_S)
            conf->flags_act |= ECE_FLAG_ACT_IT_TYPE_S;
        else if (it->type == VTSS_ECE_INNER_TAG_S_CUSTOM)
            conf->flags_act |= ECE_FLAG_ACT_IT_TYPE_S_CUSTOM;
        conf->it_vid = it->vid;
#if defined(VTSS_ARCH_SERVAL)
        if (it->pcp_mode == VTSS_ECE_PCP_MODE_CLASSIFIED)
            conf->flags_act |= ECE_FLAG_ACT_IT_PCP_MODE_CLASS;
        else if (it->pcp_mode == VTSS_ECE_PCP_MODE_MAPPED)
            conf->flags_act |= ECE_FLAG_ACT_IT_PCP_MODE_MAP;
#else
        if (it->pcp_dei_preserve)
            conf->flags_act |= ECE_FLAG_ACT_IT_PCP_MODE_CLASS;
#endif /* VTSS_ARCH_SERVAL */
        conf->it_pcp = it->pcp;
#if defined(VTSS_ARCH_SERVAL)
        if (it->dei_mode == VTSS_ECE_DEI_MODE_CLASSIFIED)
            conf->flags_act |= ECE_FLAG_ACT_IT_DEI_MODE_CLASS;
        else if (it->dei_mode == VTSS_ECE_DEI_MODE_FIXED)
            conf->flags_act |= ECE_FLAG_ACT_IT_DEI_MODE_FIXED;
        else
            conf->flags_act |= ECE_FLAG_ACT_IT_DEI_MODE_DP;
#endif /* VTSS_ARCH_SERVAL */
        if (it->dei)
            conf->flags_act |= ECE_FLAG_ACT_IT_DEI;
        conf->policer_id = action->policer_id;
    }
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
    conf->evc_id = action->evc_id;
    conf->policy_no = action->policy_no;
#if defined(VTSS_ARCH_CARACAL) || defined(VTSS_ARCH_SERVAL)
    if (action->prio_enable)
        conf->flags_act |= ECE_FLAG_ACT_PRIO_ENA;
    conf->prio = action->prio;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
#if defined(VTSS_ARCH_SERVAL)
    if (action->dp_enable)
        conf->flags_act |= ECE_FLAG_ACT_DP_ENA;
    conf->dp = action->dp;

    if (l2cp->mode == EVC_L2CP_MODE_TUNNEL)
        conf->flags_act |= ECE_FLAG_ACT_L2CP_TUNNEL;
    else if (l2cp->mode == EVC_L2CP_MODE_DISCARD)
        conf->flags_act |= ECE_FLAG_ACT_L2CP_DISCARD;
    else if (l2cp->mode == EVC_L2CP_MODE_PEER)
        conf->flags_act |= ECE_FLAG_ACT_L2CP_PEER;

    if (l2cp->dmac == EVC_L2CP_DMAC_CISCO)
        conf->flags_act |= ECE_FLAG_ACT_L2CP_DMAC_CISCO;
#endif /* VTSS_ARCH_CARACAL/SERVAL */
}

/* Add ECE configuration */
static vtss_rc evc_ece_add(vtss_ece_id_t next_id, evc_ece_conf_t *new)
{
    vtss_rc             rc, rc2;
    evc_mgmt_ece_conf_t conf;

    evc_ece2mgmt_conf(&conf, new, 0);
    if ((rc = vtss_ece_add(NULL, next_id, &conf.conf)) == VTSS_RC_OK) {
        new->flags_act &= ~ECE_FLAG_CONFLICT;
    } else {
        new->flags_act |= ECE_FLAG_CONFLICT;
    }
    if ((rc2 = evc_ece_vlan_update(new->evc_id)) != VTSS_RC_OK) {
        rc = rc2;
    }
    evc_change_flag_set(new->evc_id);
    return rc;
}

/* Delete ECE configuration */
static vtss_rc evc_ece_del(vtss_ece_id_t id, u32 flags)
{
    return (flags & ECE_FLAG_CONFLICT ? VTSS_RC_OK : vtss_ece_del(NULL, id));
}

/* ================================================================= *
 *  Configuration
 * ================================================================= */

static void evc_conf_read(BOOL create)
{
    conf_blk_id_t         blk_id;
    port_iter_t           pit;
    vtss_port_no_t        port_no;
    evc_global_blk_t      *global_blk;
    evc_port_blk_t        *port_blk;
    evc_mgmt_port_conf_t  port_conf;
    vtss_evc_policer_id_t policer_id;
    evc_pol_blk_t         *pol_blk;
    vtss_evc_id_t         evc_id;
    evc_blk_t             *evc_blk;
    evc_ece_blk_t         *ece_blk;
    evc_ece_t             *ece, *prev;
    BOOL                  do_create;
    ulong                 size, i;

    /* Enter critical region */
    EVC_CRIT_ENTER();

    if (misc_conf_read_use()) {
        /* Read/create global block */
        blk_id = CONF_BLK_EVC_GLOBAL_CONF;
        if ((global_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*global_blk)) {
            T_W("Global conf open failed or size mismatch, creating defaults");
            global_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*global_blk));
            do_create = 1;
        } else if (global_blk->version != EVC_GLOBAL_BLK_VERSION) {
            T_W("Global conf version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }

        if (global_blk == NULL) {
            T_E("global conf open failed");
        } else {
            if (do_create) {
                memset(global_blk, 0, sizeof(*global_blk));
                global_blk->conf.port_check = 1;
            }
            evc_global.conf.port_check = global_blk->conf.port_check;
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            global_blk->version = EVC_GLOBAL_BLK_VERSION;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
        }
    } else {
        evc_global.conf.port_check = 1;
        do_create = 1;
    }

    if (misc_conf_read_use()) {
        /* Read/create port block */
        blk_id = CONF_BLK_EVC_PORT_CONF;
        if ((port_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*port_blk)) {
            T_W("Port table open failed or size mismatch, creating defaults");
            port_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*port_blk));
            do_create = 1;
        } else if (port_blk->version != EVC_PORT_BLK_VERSION) {
            T_W("EVC table version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }

        if (port_blk == NULL) {
            T_E("Port table open failed");
        } else {
            evc_mgmt_port_conf_get_default(&port_conf);
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                port_no = pit.iport;
                if (do_create) {
                    /* Creating defaults: Port configuration is setup after EVCs/ECEs to avoid
                       API failures due to temporary resource limitations if key size changes */
                    evc_mgmt2port_conf(&port_conf, &port_blk->table[port_no]);
                } else {
                    /* Read configuration: Port configuration is setup before EVCs/ECEs to ensure
                       that resources are available if the key size has been reduced */
                    (void)evc_port_conf_set(port_no, &port_blk->table[port_no]);
                }
            }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            if (do_create) {
                port_blk->version = EVC_PORT_BLK_VERSION;
            } else {
                port_blk = NULL;
            }
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
        }
    } else {
        port_blk  = NULL;  // Quiet lint
        do_create = 1;
    }

    if (misc_conf_read_use()) {
        /* Read/create policer block */
        blk_id = CONF_BLK_EVC_POL_CONF;
        if ((pol_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*pol_blk)) {
            T_W("POL table open failed or size mismatch, creating defaults");
            pol_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*pol_blk));
            do_create = 1;
        } else if (pol_blk->version != EVC_POL_BLK_VERSION) {
            T_W("POL table version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }

        if (pol_blk == NULL) {
            T_E("POL table open failed");
        } else {
            if (do_create) {
                memset(pol_blk, 0, sizeof(*pol_blk));
                pol_blk->version = EVC_POL_BLK_VERSION;
            }
            for (policer_id = 0; policer_id < EVC_POL_COUNT; policer_id++) {
                (void)evc_policer_conf_set(policer_id, &pol_blk->table[policer_id]);
            }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
        }
    } else {
        evc_pol_conf_t pol_def_conf;
        memset(&pol_def_conf, 0, sizeof(pol_def_conf));
        for (policer_id = 0; policer_id < EVC_POL_COUNT; policer_id++) {
            (void)evc_policer_conf_set(policer_id, &pol_def_conf);
        }
    }

    if (misc_conf_read_use()) {
        /* Read/create EVC block */
        blk_id = CONF_BLK_EVC_CONF;
        if ((evc_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*evc_blk)) {
            T_W("EVC table open failed or size mismatch, creating defaults");
            evc_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*evc_blk));
            do_create = 1;
        } else if (evc_blk->version != EVC_BLK_VERSION) {
            T_W("EVC table version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }

        if (evc_blk == NULL) {
            T_E("EVC table open failed");
        } else {
            if (do_create) { /* Flush EVC configuration */
                memset(evc_blk, 0, sizeof(*evc_blk));
                evc_blk->version = EVC_BLK_VERSION;
            }
            for (evc_id = 0; evc_id < EVC_ID_COUNT; evc_id++) {
                (void)evc_conf_set(evc_id, &evc_blk->table[evc_id]);
            }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
        }
    } else {
        evc_conf_t evc_def_conf;
        memset(&evc_def_conf, 0, sizeof(evc_def_conf));
        for (evc_id = 0; evc_id < EVC_ID_COUNT; evc_id++) {
            (void)evc_conf_set(evc_id, &evc_def_conf);
        }
    }

    if (misc_conf_read_use()) {
        /* Read/create ECE block */
        blk_id = CONF_BLK_EVC_ECE_CONF;
        if ((ece_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*ece_blk) || do_create) {
            if (!create) {
                T_W("ECE table open failed or size mismatch, creating defaults");
            }
            ece_blk = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*ece_blk));
            do_create = 1;
        } else if (ece_blk->version != EVC_ECE_BLK_VERSION) {
            T_W("ECE table version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }

        if (ece_blk == NULL) {
            T_E("ECE table open failed");
        } else {
            if (do_create) {
                /* Flush ECE configuration */
                memset(ece_blk, 0, sizeof(*ece_blk));
                while (evc_global.ece_used != NULL) {
                    ece = evc_global.ece_used;
                    evc_change_flag_set(ece->conf.evc_id);
                    (void)evc_ece_del(ece->conf.ece_id, ece->conf.flags_act);
                    evc_global.ece_used = ece->next;
                    ece->next = evc_global.ece_free;
                    evc_global.ece_free = ece;
                }
                ece_blk->version = EVC_ECE_BLK_VERSION;
            } else {
                /* Add ECEs */
                prev = evc_global.ece_used;
                for (i = 0; i < ece_blk->count; i++) {
                    /* Take entry from free list */
                    if ((ece = evc_global.ece_free) == NULL) {
                        break;
                    }
                    evc_global.ece_free = ece->next;

                    /* Move entry to the end of used list */
                    if (prev == NULL) {
                        evc_global.ece_used = ece;
                    } else {
                        prev->next = ece;
                    }
                    prev = ece;
                    ece->next = NULL;
                    ece->conf = ece_blk->table[i];

                    /* Add last via API */
                    (void)evc_ece_add(VTSS_ECE_ID_LAST, &ece->conf);
                }
            }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif
        }
    } else {
        /* Flush ECE configuration */
        while (evc_global.ece_used != NULL) {
            ece = evc_global.ece_used;
            evc_change_flag_set(ece->conf.evc_id);
            (void)evc_ece_del(ece->conf.ece_id, ece->conf.flags_act);
            evc_global.ece_used = ece->next;
            ece->next = evc_global.ece_free;
            evc_global.ece_free = ece;
        }
    }

    if (misc_conf_read_use()) {
        if (port_blk != NULL) {
            /* Setup port configuration after restoring defaults */
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                port_no = pit.iport;
                (void)evc_port_conf_set(port_no, &port_blk->table[port_no]);
            }
        }
    } else {
        /* Creating defaults: Port configuration is setup after EVCs/ECEs to avoid
           API failures due to temporary resource limitations if key size changes */
        evc_port_conf_t def_conf;

        evc_mgmt_port_conf_get_default(&port_conf);
        evc_mgmt2port_conf(&port_conf, &def_conf);
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            port_no = pit.iport;
            (void)evc_port_conf_set(port_no, &def_conf);
        }
    }

#if defined(VTSS_ARCH_SERVAL)
    /* Update ACL rules */
    evc_acl_update();
#endif /* VTSS_ARCH_SERVAL */

    /* Generate EVC change events */
    evc_change_event_check();

    /* Exit critical region */
    EVC_CRIT_EXIT();
}

static vtss_rc evc_conf_save(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    vtss_rc               rc = VTSS_RC_OK;
    conf_blk_id_t         blk_id;
    port_iter_t           pit;
    vtss_port_no_t        port_no;
    evc_global_blk_t      *global_blk;
    evc_port_blk_t        *port_blk;
    vtss_evc_policer_id_t policer_id;
    evc_pol_blk_t         *pol_blk;
    vtss_evc_id_t         evc_id;
    evc_blk_t             *evc_blk;
    evc_ece_blk_t         *ece_blk;
    evc_ece_t             *ece;
    u32                   i;

    /* Save global configuration */
    blk_id = CONF_BLK_EVC_GLOBAL_CONF;
    if ((global_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_E("global conf open failed");
        rc = VTSS_RC_ERROR;
    } else {
        global_blk->conf.port_check = evc_global.conf.port_check;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    /* Save port configuration */
    blk_id = CONF_BLK_EVC_PORT_CONF;
    if ((port_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_E("Port table open failed");
        rc = VTSS_RC_ERROR;
    } else {
        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            port_no = pit.iport;
            port_blk->table[port_no] = evc_global.port_conf[port_no];
        }
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    /* Save policer configuration */
    blk_id = CONF_BLK_EVC_POL_CONF;
    if ((pol_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_E("Policer table open failed");
        rc = VTSS_RC_ERROR;
    } else {
        for (policer_id = 0; policer_id < EVC_POL_COUNT; policer_id++) {
            pol_blk->table[policer_id] = evc_global.pol_conf[policer_id];
        }
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    /* Save EVC configuration */
    blk_id = CONF_BLK_EVC_CONF;
    if ((evc_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_E("EVC table open failed");
        rc = VTSS_RC_ERROR;
    } else {
        for (evc_id = 0; evc_id < EVC_ID_COUNT; evc_id++) {
            evc_blk->table[evc_id] = evc_global.evc_table[evc_id];
        }
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    /* Save ECE configuration */
    blk_id = CONF_BLK_EVC_ECE_CONF;
    if ((ece_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_E("ECE table open failed");
        rc = VTSS_RC_ERROR;
    } else {
        for (i = 0, ece = evc_global.ece_used; ece != NULL; ece = ece->next, i++) {
            ece_blk->table[i] = ece->conf;
        }
        ece_blk->count = i;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
    return rc;
#else
    return VTSS_RC_OK;
#endif
}

/* ================================================================= *
 *  Management
 * ================================================================= */

vtss_rc evc_mgmt_conf_get(evc_mgmt_global_conf_t *conf)
{
    /* Enter critical region */
    EVC_CRIT_ENTER();

    *conf = evc_global.conf;
    
    /* Exit critical region */
    EVC_CRIT_EXIT();

    return VTSS_RC_OK;
}

vtss_rc evc_mgmt_conf_set(evc_mgmt_global_conf_t *conf)
{
    vtss_rc rc;

    /* Enter critical region */
    EVC_CRIT_ENTER();

    evc_global.conf = *conf;
    
    rc = evc_conf_save();
    
    /* Exit critical region */
    EVC_CRIT_EXIT();

    return rc;
}

static BOOL evc_port_invalid(vtss_port_no_t port_no)
{
    if (port_no > VTSS_PORTS) {
        T_W("illegal port_no: %u", port_no);
        return 1;
    }
    return 0;
}

vtss_rc evc_mgmt_port_conf_get(vtss_port_no_t port_no, evc_mgmt_port_conf_t *conf)
{
    T_D("port_no: %u", port_no);
    
    if (evc_port_invalid(port_no)) {
        return VTSS_RC_ERROR;
    }

    /* Enter critical region */
    EVC_CRIT_ENTER();

    /* Read port configuration */
    evc_port2mgmt_conf(conf, &evc_global.port_conf[port_no]);
    
    /* Exit critical region */
    EVC_CRIT_EXIT();

    return VTSS_RC_OK;
}

void evc_mgmt_port_conf_get_default(evc_mgmt_port_conf_t *conf)
{
    uint i;

    memset(conf, 0, sizeof(*conf));
#if defined(VTSS_ARCH_SERVAL)
    conf->conf.key_type = VTSS_VCAP_KEY_TYPE_DOUBLE_TAG;
    conf->conf.dmac_dip = 1;
    conf->vcap_conf.key_type_is1_1 = VTSS_VCAP_KEY_TYPE_DOUBLE_TAG;
    conf->vcap_conf.dmac_dip_1 = 1;
#endif /* VTSS_ARCH_SERVAL */
    for (i = 0; i < 16; i++) {
        conf->reg.bpdu_reg[i] = VTSS_PACKET_REG_CPU_ONLY;
        conf->reg.garp_reg[i] = VTSS_PACKET_REG_FORWARD;
    }
}

vtss_rc evc_mgmt_port_conf_set(vtss_port_no_t port_no, evc_mgmt_port_conf_t *conf)
{
    vtss_rc         rc, rc2;
    evc_port_conf_t new;

    T_D("port_no: %u", port_no);

    if (evc_port_invalid(port_no)) {
        return VTSS_RC_ERROR;
    }

    /* Enter critical region */
    EVC_CRIT_ENTER();

    /* Set and save port configuration */
    evc_mgmt2port_conf(conf, &new);
    rc = evc_port_conf_set(port_no, &new);
    if ((rc2 = evc_conf_save()) != VTSS_RC_OK) {
        rc = rc2;
    }

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return rc;
}

/* - Policer Configuration ----------------------------------------- */

static BOOL evc_policer_invalid(vtss_evc_policer_id_t policer_id)
{
    if (policer_id >= EVC_POL_COUNT) {
        T_W("illegal policer_id: %u", policer_id);
        return 1;
    }
    return 0;
}

/* Get policer configuration */
vtss_rc evc_mgmt_policer_conf_get(vtss_evc_policer_id_t policer_id,
                                  evc_mgmt_policer_conf_t *conf)
{
    T_D("policer_id: %u", policer_id);

    /* Check EVC ID */
    if (evc_policer_invalid(policer_id)) {
        return VTSS_RC_ERROR;
    }

    /* Enter critical region */
    EVC_CRIT_ENTER();

    /* Read policer configuration */
    conf->conf = evc_global.pol_conf[policer_id].conf;

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return VTSS_RC_OK;
}

void evc_mgmt_policer_conf_get_default(evc_mgmt_policer_conf_t *conf)
{
    memset(conf, 0, sizeof(*conf));
}

/* Set port configuration */
vtss_rc evc_mgmt_policer_conf_set(vtss_evc_policer_id_t policer_id,
                                  evc_mgmt_policer_conf_t *conf)
{
    vtss_rc        rc, rc2;
    evc_pol_conf_t new;

    T_D("policer_id: %u", policer_id);

    /* Check EVC ID */
    if (evc_policer_invalid(policer_id)) {
        return VTSS_RC_ERROR;
    }

    /* Enter critical region */
    EVC_CRIT_ENTER();

    /* Set and save policer configuration */
    new.conf = conf->conf;
    rc = evc_policer_conf_set(policer_id, &new);
    if ((rc2 = evc_conf_save()) != VTSS_RC_OK) {
        rc = rc2;
    }

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return rc;
}

/* - EVC Configuration --------------------------------------------- */

static BOOL evc_id_invalid(vtss_evc_id_t evc_id)
{
    if (evc_id >= EVC_ID_COUNT) {
        T_W("illegal evc_id: %u", evc_id);
        return 1;
    }
    return 0;
}

vtss_rc evc_mgmt_add(vtss_evc_id_t evc_id, evc_mgmt_conf_t *conf)
{
    vtss_rc    rc, rc2;
    evc_conf_t new;

    T_D("evc_id: %u", evc_id);

    /* Check EVC ID */
    if (evc_id_invalid(evc_id)) {
        return VTSS_RC_ERROR;
    }

    /* Enter critical region */
    EVC_CRIT_ENTER();

    /* Set and save EVC configuration */
    evc_api2evc_conf(&conf->conf, &new);
    rc = evc_conf_set(evc_id, &new);
    if ((rc2 = evc_conf_save()) != VTSS_RC_OK) {
        rc = rc2;
    }

#if defined(VTSS_ARCH_SERVAL)
    /* Update ACL rules */
    evc_acl_update();
#endif /* VTSS_ARCH_SERVAL */

    /* Generate EVC change events */
    evc_change_event_check();

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return rc;
}

vtss_rc evc_mgmt_del(vtss_evc_id_t evc_id)
{
    vtss_rc rc, rc2;

    T_D("evc_id: %u", evc_id);

    /* Check EVC ID */
    if (evc_id_invalid(evc_id)) {
        return VTSS_RC_ERROR;
    }

    /* Enter critical region */
    EVC_CRIT_ENTER();

    /* Set and save EVC configuration */
    rc = evc_conf_set(evc_id, NULL);
    if ((rc2 = evc_conf_save()) != VTSS_RC_OK) {
        rc = rc2;
    }

#if defined(VTSS_ARCH_SERVAL)
    /* Update ACL rules */
    evc_acl_update();
#endif /* VTSS_ARCH_SERVAL */

    /* Generate EVC change events */
    evc_change_event_check();

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return rc;
}

vtss_rc evc_mgmt_get(vtss_evc_id_t *evc_id, evc_mgmt_conf_t *conf, BOOL next)
{
    vtss_rc       rc = VTSS_RC_ERROR;
    vtss_evc_id_t id, id_first, id_last;
    evc_conf_t    *evc;

    T_D("evc_id: %u, next: %u", *evc_id, next);

    id = *evc_id;
    if (id == EVC_ID_FIRST) {
        /* Get first entry */
        id_first = 0;
        id_last = EVC_ID_COUNT;
    } else {
        if (evc_id_invalid(id)) {
            return VTSS_RC_ERROR;
        }
        if (next) {
            id_first = (id + 1);
            id_last = EVC_ID_COUNT;
        } else {
            id_first = id;
            id_last = (id + 1);
        }
    }

    /* Enter critical region */
    EVC_CRIT_ENTER();

    /* Get entry or get next valid entry */
    for (id = id_first; id < id_last; id++) {
        evc = &evc_global.evc_table[id];
        if (evc->flags & EVC_FLAG_ENABLED) {
            T_D("found evc_id: %u", id);
            *evc_id = id;
            evc_evc2api_conf(&conf->conf, evc);
            conf->conflict = (evc->flags & EVC_FLAG_CONFLICT ? 1 : 0);
            rc = VTSS_RC_OK;
            break;
        }
    }

    /* If EVC not found, return defaults */
    if (rc == VTSS_RC_ERROR) {
        evc_mgmt_get_default(conf);
    }

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return rc;
}

void evc_mgmt_get_default(evc_mgmt_conf_t *conf)
{
    memset(conf, 0, sizeof(*conf));
    conf->conf.network.pb.vid = 1;
    conf->conf.network.pb.ivid = 1;
#if defined(VTSS_FEATURE_MPLS)
    VTSS_MPLS_IDX_UNDEF(conf->conf.network.mpls_tp.pw_egress_xc);
    VTSS_MPLS_IDX_UNDEF(conf->conf.network.mpls_tp.pw_ingress_xc);
#endif
}

/* - ECE Configuration --------------------------------------------- */

vtss_rc evc_mgmt_ece_add(vtss_ece_id_t next_id, evc_mgmt_ece_conf_t *conf)
{
    vtss_rc       rc = VTSS_RC_OK, rc2;
    evc_ece_t     *ece, *prev = NULL;
    evc_ece_t     *ins = NULL, *ins_prev = NULL, *new = NULL, *new_prev = NULL;
    u8            id_used[VTSS_BF_SIZE(EVC_ECE_COUNT)];
    u32           i;
    vtss_ece_id_t id;
    vtss_evc_id_t evc_id_old = VTSS_EVC_ID_NONE, evc_id_next = VTSS_ECE_ID_LAST;
    BOOL          old_ok = 0;

    memset(id_used, 0, sizeof(id_used));

    /* Enter critical region */
    EVC_CRIT_ENTER();

    /* Search for existing entry, place to insert and next free ID */
    for (ece = evc_global.ece_used; ece != NULL; prev = ece, ece = ece->next) {
        id = ece->conf.ece_id;
        if (id == next_id) {
            /* Found insertion place */
            ins = ece;
            ins_prev = prev;
        }

        if (id == conf->conf.id) {
            /* Found entry */
            new = ece;
            new_prev = prev;
            evc_id_old = ece->conf.evc_id;
            if ((ece->conf.flags_act & ECE_FLAG_CONFLICT) == 0) {
                old_ok = 1;
            }
        } else if (evc_id_next == VTSS_ECE_ID_LAST && ins != NULL &&
                   (ece->conf.flags_act & ECE_FLAG_CONFLICT) == 0) {
            /* The first non-conflicting entry after the next entry is used towards the API */
            evc_id_next = id;
        }

        if (id) {
            /* Mark ID as used */
            VTSS_BF_SET(id_used, id - 1, 1);
        }
    }
    if (next_id == VTSS_ECE_ID_LAST) {
        ins_prev = prev;
    } else if (ins == NULL) {
        T_W("next_id: %u not found", next_id);
        rc = VTSS_RC_ERROR;
    }

    /* Allocate new entry */
    if (rc == VTSS_RC_OK) {
        if (new == NULL) {
            /* Take entry from free list */
            if ((new = evc_global.ece_free) == NULL) {
                T_W("no more ECEs");
                rc = VTSS_RC_ERROR;
            } else {
                evc_global.ece_free = new->next;
            }
        } else {
            /* Take existing entry out of list */
            if (ins_prev == new) {
                ins_prev = new_prev;
            }
            if (new_prev == NULL) {
                evc_global.ece_used = new->next;
            } else {
                new_prev->next = new->next;
            }
        }
    }

    if (rc == VTSS_RC_OK && new != NULL) { /* new != NULL is to please Lint */
        /* Insert new entry in list */
        if (ins_prev == NULL) {
            new->next = evc_global.ece_used;
            evc_global.ece_used = new;
        } else {
            new->next = ins_prev->next;
            ins_prev->next = new;
        }

        /* Find next available ID */
        if (conf->conf.id == 0) {
            for (i = 0; i < EVC_ECE_COUNT; i++) {
                if (VTSS_BF_GET(id_used, i) == 0) {
                    conf->conf.id = (i + 1);
                    break;
                }
            }
        }

        /* Add new entry */
        evc_mgmt2ece_conf(conf, &new->conf);
        rc = evc_ece_add(evc_id_next, &new->conf);

        /* If the API add failed and the entry had previously been added successfully,
           the entry is deleted using API. This is done to avoid inconsistency in the
           ordering of entries in the API and the application. This means that entries
           with the conflict flag set do not exist in the VTSS API. */
        if ((new->conf.flags_act & ECE_FLAG_CONFLICT) && old_ok &&
            (rc2 = evc_ece_del(conf->conf.id, 0)) != VTSS_RC_OK) {
            rc = rc2;
        }

        /* Save new configuration */
        if ((rc2 = evc_conf_save()) != VTSS_RC_OK) {
            rc = rc2;
        }

        /* If EVC has changed, update the VLAN of the old EVC */
        if (evc_id_old != new->conf.evc_id &&
            (rc2 = evc_ece_vlan_update(evc_id_old)) != VTSS_RC_OK) {
            rc = rc2;
        }
        
#if defined(VTSS_ARCH_SERVAL)
        /* Update ACL rules */
        evc_acl_update();
#endif /* VTSS_ARCH_SERVAL */

        /* Generate EVC change events */
        evc_change_flag_set(evc_id_old);
        evc_change_event_check();
    }

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return rc;
}

vtss_rc evc_mgmt_ece_del(vtss_ece_id_t id)
{
    vtss_rc   rc = VTSS_RC_OK, rc2;
    evc_ece_t *ece, *prev = NULL;

    /* Enter critical region */
    EVC_CRIT_ENTER();

    /* Look for entry */
    for (ece = evc_global.ece_used; ece != NULL; prev = ece, ece = ece->next) {
        if (ece->conf.ece_id == id) {
            break;
        }
    }

    if (ece == NULL) {
        T_W("id: %u not found", id);
        rc = VTSS_RC_ERROR;
    } else {
        /* Move from used to free list */
        if (prev == NULL) {
            evc_global.ece_used = ece->next;
        } else {
            prev->next = ece->next;
        }
        ece->next = evc_global.ece_free;
        evc_global.ece_free = ece;
        rc = evc_ece_del(id, ece->conf.flags_act);
        if ((rc2 = evc_conf_save()) != VTSS_RC_OK) {
            rc = rc2;
        }

        /* Update the VLAN of the EVC */
        if ((rc2 = evc_ece_vlan_update(ece->conf.evc_id)) != VTSS_RC_OK) {
            rc = rc2;
        }

#if defined(VTSS_ARCH_SERVAL)
        /* Update ACL rules */
        evc_acl_update();
#endif /* VTSS_ARCH_SERVAL */

        /* Generate EVC change events */
        evc_change_flag_set(ece->conf.evc_id);
        evc_change_event_check();
    }

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return rc;
}

vtss_rc evc_mgmt_ece_get(vtss_ece_id_t id, evc_mgmt_ece_conf_t *conf, BOOL next)
{
    vtss_rc   rc = VTSS_RC_ERROR;
    evc_ece_t *ece;
    BOOL      first = (id == EVC_ECE_ID_FIRST);

    /* Enter critical region */
    EVC_CRIT_ENTER();

    T_D("id: %u, next: %u", id, next);

    for (ece = evc_global.ece_used; ece != NULL; ece = ece->next) {
        if (first || ece->conf.ece_id == id) {
            if (first || next == 0 || (ece = ece->next) != NULL) {
                evc_ece2mgmt_conf(conf, &ece->conf, 1);
                conf->conflict = (ece->conf.flags_act & ECE_FLAG_CONFLICT ? 1 : 0);
                rc = VTSS_RC_OK;
            }
            break;
        }
    }

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return rc;
}

void evc_mgmt_ece_get_default(evc_mgmt_ece_conf_t *conf)
{
    memset(conf, 0, sizeof(*conf));
#if defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    conf->conf.action.outer_tag.vid = VLAN_ID_DEFAULT;
    conf->conf.action.inner_tag.vid = VLAN_ID_DEFAULT;
#endif /* VTSS_ARCH_JAGUAR_1/SERVAL */
}

/* - Port information ---------------------------------------------- */

vtss_rc evc_mgmt_port_info_get(evc_port_info_t info[VTSS_PORT_ARRAY_SIZE])
{
    port_iter_t    pit;
    vtss_port_no_t port_no;
    evc_conf_t     *evc;
    evc_ece_t      *ece;
    vtss_evc_id_t  evc_id;

    /* Enter critical region */
    EVC_CRIT_ENTER();

    /* Initialize all port information */
    port_iter_init_local(&pit);
    while (port_iter_getnext(&pit)) {
        port_no = pit.iport;
        info[port_no].uni_count = 0;
        info[port_no].nni_count = 0;
    }

    if (evc_global.conf.port_check) {
        /* Search for NNI ports */
        for (evc_id = 0; evc_id < EVC_ID_COUNT; evc_id++) {
            evc = &evc_global.evc_table[evc_id];
            if (evc->flags & EVC_FLAG_ENABLED) {
                port_iter_init_local(&pit);
                while (port_iter_getnext(&pit)) {
                    port_no = pit.iport;
                    if (VTSS_PORT_BF_GET(evc->ports, port_no)) {
                        info[port_no].nni_count++;
                    }
                }
            }
        }
        
        /* Search for UNI ports */
        for (ece = evc_global.ece_used; ece != NULL; ece = ece->next) {
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                port_no = pit.iport;
                if (VTSS_PORT_BF_GET(ece->conf.ports, port_no)) {
                    info[port_no].uni_count++;
                }
            }
        }
    }

    /* Exit critical region */
    EVC_CRIT_EXIT();

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Initialization
 * ================================================================= */

#if defined(VTSS_ARCH_SERVAL)
/* Initialize L2CP */
static void evc_l2cp_init(void)
{
    evc_l2cp_t      l2cp;
    evc_l2cp_info_t *info;
    u16             i, pid, pid_mask;

    for (l2cp = EVC_L2CP_STP; l2cp < EVC_L2CP_CNT; l2cp++) {
        info = &evc_global.l2cp_info[l2cp];

        /* Setup default values */
        if (l2cp > EVC_L2CP_GVRP) {
            /* Cisco protocol */
            info->type = VTSS_ECE_TYPE_SNAP;
            for (i = 0; i < 6; i++)
                info->dmac.addr[i] = evc_dmac_cisco_snap[i];
            pid_mask = 0xffff;
        } else {
            /* IEEE protocol */
            info->type = VTSS_ECE_TYPE_ETYPE;
            for (i = 0; i < 6; i++)
                info->dmac.addr[i] = evc_dmac_bpdu[i];
            pid_mask = 0x0000;
        }
        pid = 0x0000;

        /* Setup L2CP specific values */
        switch (l2cp) {
        case EVC_L2CP_STP:
            info->type = VTSS_ECE_TYPE_LLC;
            info->dmac.addr[5] = 0x00;
            break;
        case EVC_L2CP_PAUSE:
            info->dmac.addr[5] = 0x01;
            info->etype = 0x8808;
            break;
        case EVC_L2CP_LACP:
            info->dmac.addr[5] = 0x02;
            info->etype = 0x8809;
            pid = 0x0100;
            pid_mask = 0xff00;
            break;
        case EVC_L2CP_LAMP:
            info->dmac.addr[5] = 0x02;
            info->etype = 0x8809;
            pid = 0x0200;
            pid_mask = 0xff00;
            break;
        case EVC_L2CP_LOAM:
            info->dmac.addr[5] = 0x02;
            info->etype = 0x8809;
            pid = 0x0300;
            pid_mask = 0xff00;
            break;
        case EVC_L2CP_DOT1X:
            info->dmac.addr[5] = 0x03;
            info->etype = 0x888e;
            break;
        case EVC_L2CP_ELMI:
            info->dmac.addr[5] = 0x07;
            info->etype = 0x88ee;
            break;
        case EVC_L2CP_PB:
            info->type = VTSS_ECE_TYPE_LLC;
            info->dmac.addr[5] = 0x08;
            break;
        case EVC_L2CP_PB_GVRP:
            info->type = VTSS_ECE_TYPE_LLC;
            info->dmac.addr[5] = 0x0d;
            pid = 0x0001;
            break;
        case EVC_L2CP_LLDP:
            info->dmac.addr[5] = 0x0e;
            info->etype = 0x88cc;
            break;
        case EVC_L2CP_GMRP:
            info->type = VTSS_ECE_TYPE_LLC;
            info->dmac.addr[5] = 0x20;
            pid = 0x0001;
            break;
        case EVC_L2CP_GVRP:
            info->type = VTSS_ECE_TYPE_LLC;
            info->dmac.addr[5] = 0x21;
            pid = 0x0001;
            break;
        case EVC_L2CP_ULD:
            pid = 0x0111;
            break;
        case EVC_L2CP_PAGP:
            pid = 0x0104;
            break;
        case EVC_L2CP_PVST:
            info->dmac.addr[5] = 0xcd;
            pid = 0x010b;
            break;
        case EVC_L2CP_CISCO_VLAN:
            info->dmac.addr[3] = 0xcd;
            info->dmac.addr[4] = 0xcd;
            info->dmac.addr[5] = 0xce;
            pid = 0x010c;
            break;
        case EVC_L2CP_CDP:
            pid = 0x2000;
            break;
        case EVC_L2CP_VTP:
            pid = 0x2003;
            break;
        case EVC_L2CP_DTP:
            pid = 0x2004;
            break;
        case EVC_L2CP_CISCO_STP:
            info->dmac.addr[3] = 0xcd;
            info->dmac.addr[4] = 0xcd;
            info->dmac.addr[5] = 0xcd;
            pid = 0x200a;
            break;
        case EVC_L2CP_CISCO_CFM:
            info->dmac.addr[5] = 0xc3;
            pid = 0x0126;
            break;
        default:
            break;
        }

        /* PID is always used for LLC frames */
        if (info->type == VTSS_ECE_TYPE_LLC) {
            pid_mask = 0xffff;
        }

        info->pid.value[0] = ((pid >> 8) & 0xff);
        info->pid.value[1] = (pid & 0xff);
        info->pid.mask[0] = ((pid_mask >> 8) & 0xff);
        info->pid.mask[1] = (pid_mask & 0xff);
    }
}
#endif /* VTSS_ARCH_SERVAL */

static void evc_module_init(void)
{
    int       i;
    evc_ece_t *ece;

    /* Enable port check */
    evc_global.conf.port_check = 1;

    /* Place all ECEs in free list */
    for (i = 0; i < EVC_ECE_COUNT; i++) {
        ece = &evc_global.ece_table[i];
        ece->next = evc_global.ece_free;
        evc_global.ece_free = ece;
    }

    /* Create semaphore for critical regions */
    critd_init(&evc_global.crit, "evc_global.crit", VTSS_MODULE_ID_EVC, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);

#if defined(VTSS_ARCH_SERVAL)
    /* Initialize L2CP */
    evc_l2cp_init();
#endif /* VTSS_ARCH_SERVAL */

    EVC_CRIT_EXIT();
}


#if defined(VTSS_ARCH_SERVAL)
static void evc_s_custom_etype_change_callback(vtss_etype_t tpid)
{
    T_D("new tpid: 0x%04x", tpid);
    EVC_CRIT_ENTER();
    evc_global.s_custom_etype = tpid;
    EVC_CRIT_EXIT();
}

static void evc_module_start(void)
{
    vtss_etype_t tpid;

    vlan_s_custom_etype_change_register(VTSS_MODULE_ID_EVC, evc_s_custom_etype_change_callback);
    if (vlan_mgmt_s_custom_etype_get(&tpid) == VTSS_RC_OK) {
        evc_s_custom_etype_change_callback(tpid);
    }
}
#endif /* VTSS_ARCH_SERVAL */

vtss_rc evc_init(vtss_init_data_t *data)
{
    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        evc_module_init();
#ifdef VTSS_SW_OPTION_VCLI
        evc_cli_init();
#endif

#ifdef VTSS_SW_OPTION_ICFG
        evc_icfg_init();
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
#if defined(VTSS_ARCH_SERVAL)
        evc_module_start();
#endif /* VTSS_ARCH_SERVAL */
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF");
        if (data->isid == VTSS_ISID_GLOBAL) {
            evc_conf_read(1);
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");
        evc_conf_read(0);
        break;
    default:
        break;
    }

    return VTSS_RC_OK;
}

