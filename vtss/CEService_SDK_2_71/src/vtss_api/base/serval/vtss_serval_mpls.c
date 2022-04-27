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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_MPLS
#include "vtss_serval_cil.h"

#if defined(VTSS_ARCH_SERVAL)

/* NOTE: The following functions are part of the core MPLS code, but since they are used by
 *       the raw MPLS encapsulation code which is "independent" of MPLS, we need them here
 *       as well.
 */

#define VTSS_MPLS_CHECK(chk)    do { if (!(chk)) { VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Check failed: " #chk); return VTSS_RC_ERROR; } } while (0)
#define VTSS_MPLS_CHECK_RC(chk) do { if (VTSS_RC_OK != (chk)) { VTSS_DG(VTSS_TRACE_GROUP_MPLS, "RC check failed: " #chk); return VTSS_RC_ERROR; } } while (0)

static inline void srvl_mpls_out_encap_set_bit(u32 *bits, u32 offset, u8 val)
{
    u8 *data = (u8*)bits;
    u8 shift = offset % 8;
    data += (320-offset-1)/8;
    *data = (*data & ~VTSS_BIT(shift)) | (val ? VTSS_BIT(shift) : 0);
}

static void srvl_mpls_out_encap_set_bits(u32 *bits, u32 offset, u32 width, u32 value)
{
    while (width > 0) {
        srvl_mpls_out_encap_set_bit(bits, offset, value & 0x01);
        width--;
        offset++;
        value >>= 1;
    }
}

static vtss_es0_mpls_encap_len_t srvl_bytes_to_encap_len(u16 bytes)
{
    switch (bytes) {
    case 0:
        return VTSS_ES0_MPLS_ENCAP_LEN_NONE;
    case 14:
        return VTSS_ES0_MPLS_ENCAP_LEN_14;
    case 18:
        return VTSS_ES0_MPLS_ENCAP_LEN_18;
    case 22:
        return VTSS_ES0_MPLS_ENCAP_LEN_22;
    case 26:
        return VTSS_ES0_MPLS_ENCAP_LEN_26;
    case 30:
        return VTSS_ES0_MPLS_ENCAP_LEN_30;
    case 34:
        return VTSS_ES0_MPLS_ENCAP_LEN_34;
    default:
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Unmatched encap byte count: %u", bytes);
        return VTSS_ES0_MPLS_ENCAP_LEN_NONE;
    }
}

/* ----------------------------------------------------------------- *
 * Raw MPLS encapsulation.
 *
 * WARNING: Special use, not used by other MPLS functionality.
 *
 * WARNING: Serval-specific!
 * ----------------------------------------------------------------- */

vtss_rc vtss_srvl_mpls_out_encap_raw_set(vtss_state_t *vtss_state,
                                         const u32 idx,
                                         const vtss_srvl_mpls_out_encap_raw_t *const entry,
                                         vtss_es0_mpls_encap_len_t *length)
{
    const int word_cnt = (entry->length + 6 + 3) / 4;      // The first 6 bytes are for re-marking configuration; all-zero. 3 to round up if necessary.

    u32  bits[word_cnt];    // MSB is bit 31 in [0]
    int  i;
    u32  offset = 272 - entry->length * 8;

    *length = srvl_bytes_to_encap_len(entry->length);

    if (idx == 0  ||  idx > VTSS_MPLS_OUT_ENCAP_CNT) {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Invalid MPLS out-segment encap idx (%u)", idx);
        return VTSS_RC_ERROR;
    }

    memset(bits, 0, sizeof(bits));

    for (i = entry->length - 1; i >= 0; i--) {
        srvl_mpls_out_encap_set_bits(bits, offset, 8, entry->data[i]);
        offset += 8;
    }

    for (i = 0; i < word_cnt; i++) {
        u32 val = VTSS_OS_NTOHL(bits[i]);
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "%2d   0x%08x   0x%08x", i, val, bits[i]);
        SRVL_WR(VTSS_SYS_ENCAPSULATIONS_ENCAP_DATA(i), val);
    }

    SRVL_WR(VTSS_SYS_ENCAPSULATIONS_ENCAP_CTRL, VTSS_F_SYS_ENCAPSULATIONS_ENCAP_CTRL_ENCAP_ID(idx) | VTSS_F_SYS_ENCAPSULATIONS_ENCAP_CTRL_ENCAP_WR);

    return VTSS_RC_OK;
}

#if defined (VTSS_FEATURE_MPLS)

/* - CIL functions ------------------------------------------------- */

/*
 * Internal MPLS utilities
 */

#define VTSS_MPLS_CHECK(chk)    do { if (!(chk)) { VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Check failed: " #chk); return VTSS_RC_ERROR; } } while (0)
#define VTSS_MPLS_CHECK_RC(chk) do { if (VTSS_RC_OK != (chk)) { VTSS_DG(VTSS_TRACE_GROUP_MPLS, "RC check failed: " #chk); return VTSS_RC_ERROR; } } while (0)

#define VTSS_MPLS_TAKE_FROM_FREELIST(freelist, table, idx) \
    do { \
        (idx) = vtss_state->freelist; \
        if (VTSS_MPLS_IDX_IS_UNDEF((idx))) { \
            VTSS_DG(VTSS_TRACE_GROUP_MPLS, "No free " #table " entries"); \
            VTSS_MPLS_IDX_UNDEF((idx)); \
            return VTSS_RC_ERROR; \
        } \
        vtss_state->freelist = vtss_state->table[idx].next_free; \
        VTSS_MPLS_IDX_UNDEF(vtss_state->table[idx].next_free); \
    } while (0)

#define VTSS_MPLS_RETURN_TO_FREELIST(freelist, table, idx) \
    vtss_state->table[idx].next_free = vtss_state->freelist; \
    vtss_state->freelist = (idx);

#if 0
// Not used yet, so disabled due to lint
static BOOL srvl_mpls_hw_avail_mll(u32 cnt)
{
    return (vtss_state->mpls.is0_mll_cnt + cnt <= VTSS_MPLS_L2_CNT)  &&
           (vtss_state->mpls.is0_mll_cnt + cnt + vtss_state->mpls.is0_mlbs_cnt / 2 <= SRVL_IS0_CNT);
}

static BOOL srvl_mpls_hw_avail_mlbs(u32 cnt)
{
    return (vtss_state->mpls.is0_mlbs_cnt + cnt <= VTSS_MPLS_LSP_CNT)  &&
           (vtss_state->mpls.is0_mll_cnt + (vtss_state->mpls.is0_mlbs_cnt + cnt) / 2 <= SRVL_IS0_CNT);
}

static BOOL srvl_mpls_hw_avail_encap(u32 cnt)
{
    return (vtss_state->mpls.encap_cnt + cnt) <= VTSS_MPLS_OUT_ENCAP_CNT;
}

static BOOL srvl_mpls_hw_avail_vprofile(u32 cnt)
{
    return (vtss_state->mpls.vprofile_cnt + cnt) <= VTSS_MPLS_VPROFILE_CNT;
}
#endif

static vtss_rc srvl_mpls_sdx_alloc(vtss_state_t *vtss_state,
                                   BOOL isdx, vtss_port_no_t port_no, u32 cnt, vtss_sdx_entry_t **first)
{
    *first = vtss_cmn_sdx_alloc(vtss_state, port_no, isdx);
    if (*first) {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "SDX alloc success: %d entries starting at %d", cnt, (*first)->sdx);
        return VTSS_RC_OK;
    }
    else {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "SDX alloc FAILURE: %d entries", cnt);
        return VTSS_RC_ERROR;
    }
}

static vtss_rc srvl_mpls_sdx_free(vtss_state_t *vtss_state, BOOL isdx, u32 cnt, vtss_sdx_entry_t *first)
{
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "SDX free: %d entries starting at %d", cnt, first->sdx);
    vtss_cmn_sdx_free(vtss_state, first, isdx);
    return VTSS_RC_OK;
}

/* ----------------------------------------------------------------- *
 *  VProfile functions (with regards to MPLS processing)
 * ----------------------------------------------------------------- */

static vtss_rc srvl_mpls_vprofile_hw_update(vtss_state_t *vtss_state,
                                            vtss_mpls_vprofile_idx_t idx)
{
    u32                  v, m;
    vtss_mpls_vprofile_t *vp;

    if (idx >= VTSS_MPLS_VPROFILE_CNT) {
        return VTSS_RC_ERROR;
    }

    vp = &VP_P(idx);

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "VProfile update, entry %d (port %d)", idx, vp->port);

    SRVL_WRM(VTSS_ANA_PORT_VCAP_CFG(idx),    vp->s1_s2_enable ? VTSS_F_ANA_PORT_VCAP_CFG_S1_ENA    : 0, VTSS_F_ANA_PORT_VCAP_CFG_S1_ENA);
    SRVL_WRM(VTSS_ANA_PORT_VCAP_S2_CFG(idx), vp->s1_s2_enable ? VTSS_F_ANA_PORT_VCAP_S2_CFG_S2_ENA : 0, VTSS_F_ANA_PORT_VCAP_S2_CFG_S2_ENA);

    v = (vp->recv_enable       ? VTSS_F_ANA_PORT_PORT_CFG_RECV_ENA  : 0)      |
        (vp->learn_enable      ? VTSS_F_ANA_PORT_PORT_CFG_LEARN_ENA : 0)      |
        (vp->src_mirror_enable ? VTSS_F_ANA_PORT_PORT_CFG_SRC_MIRROR_ENA : 0) |
        VTSS_F_ANA_PORT_PORT_CFG_PORTID_VAL(VTSS_CHIP_PORT(vp->port));
    m = VTSS_F_ANA_PORT_PORT_CFG_RECV_ENA       |
        VTSS_F_ANA_PORT_PORT_CFG_LEARN_ENA      |
        VTSS_F_ANA_PORT_PORT_CFG_SRC_MIRROR_ENA |
        VTSS_M_ANA_PORT_PORT_CFG_PORTID_VAL;
    SRVL_WRM(VTSS_ANA_PORT_PORT_CFG(idx), v, m);

    SRVL_WRM(VTSS_ANA_PORT_VLAN_CFG(idx), vp->vlan_aware ? VTSS_F_ANA_PORT_VLAN_CFG_VLAN_AWARE_ENA : 0, VTSS_F_ANA_PORT_VLAN_CFG_VLAN_AWARE_ENA);

    v = (vp->map_dscp_cos_enable ? VTSS_F_ANA_PORT_QOS_CFG_QOS_DSCP_ENA : 0) |
        (vp->map_eth_cos_enable  ? VTSS_F_ANA_PORT_QOS_CFG_QOS_PCP_ENA  : 0);
    m = VTSS_F_ANA_PORT_QOS_CFG_QOS_DSCP_ENA |
        VTSS_F_ANA_PORT_QOS_CFG_QOS_PCP_ENA;
    SRVL_WRM(VTSS_ANA_PORT_QOS_CFG(idx), v, m);

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_vprofile_init(vtss_state_t *vtss_state)
{
    vtss_mpls_vprofile_idx_t i;
    vtss_mpls_vprofile_t     *vp;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Attempting static VProfile config for LSR (entry %u) and OAM (entry %u)", VTSS_MPLS_VPROFILE_LSR_IDX, VTSS_MPLS_VPROFILE_OAM_IDX);

    // TN1135/MPLS, sect. 3.3.3, 3.3.4: Static LSR, OAM entries:

    vp = &VP_P(VTSS_MPLS_VPROFILE_LSR_IDX);
    vp->s1_s2_enable        = FALSE;
    vp->recv_enable         = TRUE;
    vp->ptp_dly1_enable     = FALSE;
    vp->vlan_aware          = FALSE;
    vp->map_dscp_cos_enable = FALSE;
    vp->map_eth_cos_enable  = FALSE;
    (void) srvl_mpls_vprofile_hw_update(vtss_state, VTSS_MPLS_VPROFILE_LSR_IDX);

    vp = &VP_P(VTSS_MPLS_VPROFILE_OAM_IDX);
    vp->s1_s2_enable        = FALSE;
    vp->recv_enable         = FALSE;
    vp->ptp_dly1_enable     = FALSE;
    vp->vlan_aware          = FALSE;
    vp->map_dscp_cos_enable = FALSE;
    vp->map_eth_cos_enable  = FALSE;
    (void) srvl_mpls_vprofile_hw_update(vtss_state, VTSS_MPLS_VPROFILE_OAM_IDX);

    // Set up VProfile free-list. The reserved entries, of course, aren't in:
    for (i = VTSS_MPLS_VPROFILE_RESERVED_CNT; i < VTSS_MPLS_VPROFILE_CNT - 1; i++) {
        vtss_state->mpls.vprofile_conf[i].next_free = i + 1;
    }
    vtss_state->mpls.vprofile_conf[VTSS_MPLS_VPROFILE_CNT - 1].next_free = VTSS_MPLS_IDX_UNDEFINED;
    vtss_state->mpls.vprofile_free_list = VTSS_MPLS_VPROFILE_RESERVED_CNT;

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_vprofile_alloc(vtss_state_t *vtss_state,
                                        vtss_mpls_vprofile_idx_t * const idx)
{
    VTSS_MPLS_TAKE_FROM_FREELIST(mpls.vprofile_free_list, mpls.vprofile_conf, *idx);
    vtss_state->mpls.vprofile_cnt++;
    VTSS_MPLS_CHECK(vtss_state->mpls.vprofile_cnt <= (VTSS_MPLS_VPROFILE_CNT - VTSS_MPLS_VPROFILE_RESERVED_CNT));

    memset(&vtss_state->mpls.vprofile_conf[*idx].pub, 0, sizeof(vtss_state->mpls.vprofile_conf[*idx].pub));

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Allocated VProfile entry %d (%d allocated)", *idx, vtss_state->mpls.vprofile_cnt);
    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_vprofile_free(vtss_state_t *vtss_state,
                                       const vtss_mpls_vprofile_idx_t idx)
{
    if (idx < VTSS_MPLS_VPROFILE_RESERVED_CNT) {
        return VTSS_RC_ERROR;
    }
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Freeing VProfile entry %d (%d allocated afterwards)", idx, vtss_state->mpls.vprofile_cnt - 1);
    VTSS_MPLS_RETURN_TO_FREELIST(mpls.vprofile_free_list, mpls.vprofile_conf, idx);
    VTSS_MPLS_CHECK(vtss_state->mpls.vprofile_cnt > 0);
    vtss_state->mpls.vprofile_cnt--;
    return VTSS_RC_OK;
}

/* ----------------------------------------------------------------- *
 *  MPLS Out-segment (Egress) Encapsulation
 * ----------------------------------------------------------------- */

/* The MPLS egress encapsulation data consists of 320 bits; the first 48 bits
 * contain remarking information, and the subsequent bits consist of the
 * MLL + optional MLBS configuration prepended to the payload.
 */

static void srvl_mpls_out_encap_set_mac(u32 *bits, u32 offset, const vtss_mac_t *mac)
{
    int i;
    for (i = 5; i >= 0; i--) {
        srvl_mpls_out_encap_set_bits(bits, offset, 8, mac->addr[i]);
        offset += 8;
    }
}

typedef struct {
    struct {
        BOOL use_cls_vid;
        BOOL use_cls_dp;
        BOOL use_cls_pcp;
        BOOL use_cls_dei;
        struct {
            BOOL use_ttl;
            BOOL use_s_bit;
            BOOL use_qos_to_tc_map;
            u8   qos_to_tc_map_idx;
        } label[VTSS_MPLS_OUT_ENCAP_LABEL_CNT];
    } remark;
    vtss_mac_t                   dmac;
    vtss_mac_t                   smac;
    vtss_mll_tagtype_t           tag_type;
    vtss_vid_t                   vid;                   /**< C or B VID */
    vtss_tagprio_t               pcp;                   /**< PCP value */
    vtss_dei_t                   dei;                   /**< DEI value */
    vtss_mll_ethertype_t         ether_type;
    u8                           label_stack_depth;     /**< Range [0..VTSS_MPLS_ENCAP_LABEL_CNT] */
    vtss_mpls_label_t            label_stack[VTSS_MPLS_OUT_ENCAP_LABEL_CNT];
    BOOL                         use_cw;
    u32                          cw;
} srvl_mpls_out_encap_t;

static vtss_rc srvl_mpls_out_encap_set(vtss_state_t *vtss_state,
                                       u32 idx, const srvl_mpls_out_encap_t *const entry, u32 *length)
{
    const int word_cnt = 320/32;

    u32  bits[word_cnt];    // MSB is bit 31 in [0]
    int  i;
    u32  offset;
    u32  offset_base;

    if (idx == 0  ||  idx > VTSS_MPLS_OUT_ENCAP_CNT) {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Invalid MPLS out-segment encap idx (%u)", idx);
        return VTSS_RC_ERROR;
    }

    *length = 0;

    memset(bits, 0, sizeof(bits));

#define SET_BOOL(offset, boolean)      srvl_mpls_out_encap_set_bit(bits, (offset), (boolean) ? 1 : 0)
#define SET_INT(offset, width, value)  srvl_mpls_out_encap_set_bits(bits, (offset), (width), (value))
#define SET_INT_O(width, value)        offset -= (width); srvl_mpls_out_encap_set_bits(bits, offset, (width), (value))

    SET_BOOL(298, entry->remark.use_cls_vid);
    SET_BOOL(297, entry->remark.use_cls_dp);
    SET_BOOL(296, entry->remark.use_cls_pcp);
    SET_BOOL(295, entry->remark.use_cls_dei);
    SET_BOOL(294, entry->tag_type != VTSS_MPLS_TAGTYPE_UNTAGGED);
    for (i = 0; i < 3; i++) {
        SET_INT (272 + i*4, 3, entry->remark.label[i].qos_to_tc_map_idx);
        SET_BOOL(275 + i*4,    entry->remark.label[i].use_qos_to_tc_map);
        SET_BOOL(288 + i,      entry->remark.label[i].use_s_bit);
        SET_BOOL(291 + i,      entry->remark.label[i].use_ttl);
    }

    offset_base = offset = 272;

    offset -= 6*8;
    srvl_mpls_out_encap_set_mac(bits, offset, &entry->dmac);
    offset -= 6*8;
    srvl_mpls_out_encap_set_mac(bits, offset, &entry->smac);

    switch (entry->tag_type) {
    case VTSS_MPLS_TAGTYPE_UNTAGGED:
        break;
    case VTSS_MPLS_TAGTYPE_CTAGGED:
        SET_INT_O(16, 0x8100);
        SET_INT_O(3, entry->pcp);
        SET_INT_O(1, entry->dei);
        SET_INT_O(12, entry->vid);
        break;
    case VTSS_MPLS_TAGTYPE_STAGGED:
        SET_INT_O(16, 0x88A8);              // TBD: Use global value if a such is configured?
        SET_INT_O(3, entry->pcp);
        SET_INT_O(1, entry->dei);
        SET_INT_O(12, entry->vid);
        break;
    }

    if (entry->ether_type == VTSS_MLL_ETHERTYPE_DOWNSTREAM_ASSIGNED) {
        SET_INT_O(16, 0x8847);
    }
    else {
        SET_INT_O(16, 0x8848);
    }

    for (i = 0; i < (int)entry->label_stack_depth; i++) {
        SET_INT_O(20, entry->label_stack[i].value);
        SET_INT_O(3,  entry->label_stack[i].tc);
        SET_INT_O(1,  (i == (int)entry->label_stack_depth - 1) ? 1 : 0);
        SET_INT_O(8,  entry->label_stack[i].ttl);
    }
    if (entry->use_cw) {
        SET_INT_O(32, entry->cw);
    }

    *length = (offset_base - offset + 7) / 8;

#undef SET_BOOL
#undef SET_INT
#undef SET_INT_O

    for (i = 0; i < word_cnt; i++) {
        u32 val = VTSS_OS_NTOHL(bits[i]);
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "%2d   0x%08x   0x%08x", i, val, bits[i]);
        SRVL_WR(VTSS_SYS_ENCAPSULATIONS_ENCAP_DATA(i), val);
    }

    SRVL_WR(VTSS_SYS_ENCAPSULATIONS_ENCAP_CTRL, VTSS_F_SYS_ENCAPSULATIONS_ENCAP_CTRL_ENCAP_ID(idx) | VTSS_F_SYS_ENCAPSULATIONS_ENCAP_CTRL_ENCAP_WR);

    return VTSS_RC_OK;
}

/* ----------------------------------------------------------------- *
 *  Segment utilities
 * ----------------------------------------------------------------- */

static vtss_rc srvl_mpls_segment_state_get(vtss_state_t *vtss_state,
                                           const vtss_mpls_segment_idx_t     idx,
                                           vtss_mpls_segment_state_t * const state)
{
    /*lint -save -e506 Disable 'Constant value Boolean' warning */

#define SET(x) *state = VTSS_MPLS_SEGMENT_STATE_##x
#define SET_UP(tst) do { if (!seg->need_hw_alloc && (tst)) SET(UP); } while (0)

    vtss_mpls_segment_internal_t *seg;
    vtss_mpls_xc_internal_t      *xc;
    BOOL                         has_l2, has_xc, has_isdx, has_srv, has_hw;
    BOOL                         is_lsr, is_pw, srv_up, has_esdx;
    vtss_mpls_segment_state_t    srv_state;

    SET(UNCONF);

    VTSS_MPLS_IDX_CHECK_SEGMENT(idx);

    seg      = &SEG_I(idx);
    has_l2   = VTSS_MPLS_IDX_IS_DEF(seg->pub.l2_idx);
    has_xc   = VTSS_MPLS_IDX_IS_DEF(seg->pub.xc_idx);
    has_srv  = VTSS_MPLS_IDX_IS_DEF(seg->pub.server_idx);
    has_hw   = FALSE;
    srv_up   = FALSE;

    /* In all of the below, we cannot enter UP when seg->need_hw_alloc == TRUE.
     *
     * In-segment with L2:
     *
     *     UNCONF  = !XC || !LABEL
     *     CONF    = !UNCONF
     *     UP[LSR] = CONF && xc.ISDX && MLL && MLBS
     *     UP[LER] = if seg.PW
     *                   CONF && xc.ISDX && MLL && MLBS
     *               else
     *                   CONF
     *
     * In-segment without L2:
     *
     *     UNCONF  = !XC || !LABEL
     *     CONF    = !UNCONF  &&  has_server
     *     UP[LSR] = CONF && xc.ISDX && MLL && MLBS && server.UP
     *     UP[LER] = if seg.PW
     *                   CONF && xc.ISDX && MLL && MLBS && server.UP
     *               else
     *                   CONF
     *
     * Out-segment with L2:
     *
     *     UNCONF  = !XC || !LABEL || !TTL
     *     CONF    = !UNCONF
     *     UP[LSR] = CONF && xc.ISDX && ESDX && ENCAP && ES0
     *     UP[LER] = if seg.PW
     *                   CONF && ENCAP
     *               else
     *                   CONF
     *
     * Out-segment without L2:
     *
     *     UNCONF  = !XC || !LABEL || !TTL
     *     CONF    = !UNCONF  &&  has_server
     *     UP[LSR] = CONF && xc.ISDX && ESDX && ENCAP && ES0 && server.UP
     *     UP[LER] = if seg.PW
     *                   CONF && ENCAP && server.UP
     *               else
     *                   CONF && server.UP
     *
     */

    if (!has_xc  ||  (seg->pub.label.value == VTSS_MPLS_LABEL_VALUE_DONTCARE)  ||
        (!seg->pub.is_in  &&  (seg->pub.label.ttl == 0))) {
        *state = VTSS_MPLS_SEGMENT_STATE_UNCONF;
        return VTSS_RC_OK;
    }

    xc       = &XC_I(seg->pub.xc_idx);
    has_isdx = xc->isdx != NULL;
    is_lsr   = xc->pub.type == VTSS_MPLS_XC_TYPE_LSR;
    is_pw    = xc->pub.type == VTSS_MPLS_XC_TYPE_LER  &&  seg->pub.pw_conf.is_pw;

    if (seg->pub.is_in) {
        if (has_l2) {
            SET(CONF);
            if (is_lsr  ||  is_pw) {
                has_hw = has_isdx  &&  seg->u.in.has_mll  &&  seg->u.in.has_mlbs;
                SET_UP(has_hw);
            }
            else {
                SET_UP(TRUE);
            }
        }
        else {
            if (has_srv) {
                SET(CONF);
                (void) srvl_mpls_segment_state_get(vtss_state, seg->pub.server_idx, &srv_state);
                srv_up = srv_state == VTSS_MPLS_SEGMENT_STATE_UP;
                if (is_lsr  ||  is_pw) {
                    has_hw = has_isdx  &&  seg->u.in.has_mlbs;
                    SET_UP(has_hw && srv_up);
                }
                else {
                    SET_UP(TRUE);
                }
            }
        }
    }
    else {  // Out-segment
        if (has_l2) {
            SET(CONF);
            srv_up = TRUE;   // Or so we pretend.
        }
        else if (has_srv) {
            SET(CONF);
            (void) srvl_mpls_segment_state_get(vtss_state, seg->pub.server_idx, &srv_state);
            srv_up = srv_state == VTSS_MPLS_SEGMENT_STATE_UP;
        }
        if (is_lsr) {
            has_esdx = seg->u.out.esdx != NULL;
            has_hw = has_isdx  &&  has_esdx  &&  seg->u.out.has_encap  &&  seg->u.out.has_es0;
            SET_UP(has_hw  && srv_up);
        }
        else if (is_pw) {
            SET_UP(seg->u.out.has_encap && srv_up);
        }
        else {
            SET_UP(srv_up);
        }
    }

    return VTSS_RC_OK;
#undef SET
#undef SET_UP
    /*lint -restore */
}

static const char *srvl_mpls_segment_state_to_str(const vtss_mpls_segment_state_t s)
{
    switch (s) {
    case VTSS_MPLS_SEGMENT_STATE_UNCONF:
        return "UNCONF";
    case VTSS_MPLS_SEGMENT_STATE_CONF:
        return "CONF";
    case VTSS_MPLS_SEGMENT_STATE_UP:
        return "UP";
    default:
        return "(unknown)";
    }
}

static vtss_mpls_segment_idx_t srvl_mpls_find_ultimate_server(vtss_state_t *vtss_state,
                                                              vtss_mpls_segment_idx_t seg, u8 *depth)
{
    *depth = 1;
    if (VTSS_MPLS_IDX_IS_UNDEF(seg)) {
        VTSS_EG(VTSS_TRACE_GROUP_MPLS, "Segment idx is undefined");
    }
    else {
        while (VTSS_MPLS_IDX_IS_DEF(SEG_P(seg).server_idx)) {
            seg = SEG_P(seg).server_idx;
            (*depth)++;
        }
        if (*depth > VTSS_MPLS_IN_ENCAP_LABEL_CNT) {
            VTSS_EG(VTSS_TRACE_GROUP_MPLS, "Label stack is too deep");
        }
    }
    return seg;
}

static vtss_port_no_t srvl_mpls_port_no_get(vtss_state_t *vtss_state,
                                            vtss_mpls_segment_idx_t idx)
{
    u8                      depth;
    vtss_mpls_segment_idx_t ultimate_server;
    vtss_mpls_l2_idx_t      l2_idx;

    if (VTSS_MPLS_IDX_IS_DEF(idx)) {
        ultimate_server = srvl_mpls_find_ultimate_server(vtss_state, idx, &depth);
        l2_idx          = SEG_P(ultimate_server).l2_idx;
        return VTSS_MPLS_IDX_IS_DEF(l2_idx) ? L2_P(l2_idx).port : 0;
    }

    VTSS_EG(VTSS_TRACE_GROUP_MPLS, "Invariant breach; there's a bug somewhere");
    return 0;
}

/* ----------------------------------------------------------------- *
 *  IS0 MLL entries
 * ----------------------------------------------------------------- */

static vtss_rc srvl_mpls_is0_mll_update(vtss_state_t *vtss_state,
                                        vtss_mpls_l2_internal_t *l2, BOOL upstream);

static vtss_rc srvl_mpls_is0_mll_alloc(vtss_state_t *vtss_state,
                                       vtss_mpls_l2_internal_t *l2, BOOL upstream)
{
    i16                       *ll_idx = upstream ? &l2->ll_idx_upstream : &l2->ll_idx_downstream;
    vtss_mpls_idxchain_iter_t dummy;
    vtss_mpls_idxchain_user_t user;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Allocating new MLL %sstream entry id", upstream ? "up" : "down");
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(*ll_idx));

    VTSS_MPLS_CHECK(vtss_mpls_idxchain_get_first(vtss_state, &vtss_state->mpls.is0_mll_free_chain, &dummy, &user));
    VTSS_MPLS_CHECK_RC(vtss_mpls_idxchain_remove_first(vtss_state, &vtss_state->mpls.is0_mll_free_chain, user));
    *ll_idx = user;

    vtss_state->mpls.is0_mll_cnt++;
    VTSS_MPLS_CHECK(vtss_state->mpls.is0_mll_cnt <= VTSS_MPLS_L2_CNT  &&
                    vtss_state->mpls.is0_mll_cnt + vtss_state->mpls.is0_mlbs_cnt / 2 <= SRVL_IS0_CNT);
    
    return srvl_mpls_is0_mll_update(vtss_state, l2, upstream);
}

static vtss_rc srvl_mpls_is0_mll_free(vtss_state_t *vtss_state,
                                      vtss_mpls_l2_internal_t *l2, BOOL upstream)
{
    vtss_rc rc;
    i16 *ll_idx = upstream ? &l2->ll_idx_upstream : &l2->ll_idx_downstream;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Removing IS0 MLL entry %d", *ll_idx);
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(*ll_idx));

    // Remove VCAP entry and put its index back on the free chain
    rc = vtss_vcap_del(vtss_state, &vtss_state->vcap.is0.obj, VTSS_IS0_USER_MPLS_LL, *ll_idx);
    VTSS_MPLS_CHECK_RC(vtss_mpls_idxchain_insert_at_head(vtss_state, &vtss_state->mpls.is0_mll_free_chain, *ll_idx));
    VTSS_MPLS_IDX_UNDEF(*ll_idx);

    VTSS_MPLS_CHECK(vtss_state->mpls.is0_mll_cnt > 0);
    vtss_state->mpls.is0_mll_cnt--;

    return rc;
}

static vtss_rc srvl_mpls_is0_mll_update(vtss_state_t *vtss_state,
                                        vtss_mpls_l2_internal_t *l2, BOOL upstream)
{
    vtss_rc                   rc;
    vtss_is0_entry_t          entry;
    vtss_vcap_data_t          data;
    i16                       ll_idx = upstream ? l2->ll_idx_upstream : l2->ll_idx_downstream;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Updating MLL entry id %d", ll_idx);
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(ll_idx));

    memset(&entry, 0, sizeof(entry));

    entry.type = VTSS_IS0_TYPE_MLL;

    entry.key.mll.ingress_port = l2->pub.port;
    entry.key.mll.tag_type     = (vtss_is0_tagtype_t) l2->pub.tag_type;
    entry.key.mll.b_vid        = l2->pub.vid;
    entry.key.mll.ether_type   = upstream ? VTSS_MLL_ETHERTYPE_UPSTREAM_ASSIGNED : VTSS_MLL_ETHERTYPE_DOWNSTREAM_ASSIGNED;
    memcpy(&entry.key.mll.smac, &l2->pub.peer_mac, sizeof(entry.key.mll.smac));
    memcpy(&entry.key.mll.dmac, &l2->pub.self_mac, sizeof(entry.key.mll.dmac));

    entry.action.mll.linklayer_index = ll_idx;
    entry.action.mll.mpls_forwarding = TRUE;

    data.key_size    = VTSS_VCAP_KEY_SIZE_FULL;
    data.u.is0.entry = &entry;

    rc = vtss_vcap_add(vtss_state, &vtss_state->vcap.is0.obj, VTSS_IS0_USER_MPLS_LL, ll_idx,
                       VTSS_VCAP_ID_LAST, &data, FALSE);
    
    if (rc != VTSS_RC_OK) {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "MLL entry update FAILED, rc = %d. Attempting to delete entry.", rc);
        VTSS_MPLS_CHECK_RC(srvl_mpls_is0_mll_free(vtss_state, l2, upstream));
    }

    return rc;
 }



/* ----------------------------------------------------------------- *
 *  IS0 MLBS entries
 * ----------------------------------------------------------------- */

static vtss_vcap_user_t srvl_mpls_depth_to_is0_user(u8 depth)
{
#if (VTSS_MPLS_IN_ENCAP_LABEL_CNT != 3)
#error "This code assumes VTSS_MPLS_IN_ENCAP_LABEL_CNT == 3"
#endif
    switch (depth) {
    case 1:  return VTSS_IS0_USER_MPLS_MLBS_1;
    case 2:  return VTSS_IS0_USER_MPLS_MLBS_2;
    case 3:  return VTSS_IS0_USER_MPLS_MLBS_3;
    default:
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Label stack too deep: %d", depth);
        return VTSS_IS0_USER_MPLS_MLBS_1;
    }
}

static vtss_rc srvl_mpls_is0_mlbs_free(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    vtss_mpls_segment_internal_t *seg = &SEG_I(idx);
    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_CHECK(seg->pub.is_in);

    if (seg->u.in.has_mlbs) {
        u8 depth;

        (void) srvl_mpls_find_ultimate_server(vtss_state, idx, &depth);

        // Remove VCAP entry
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Removing IS0 MLBS entry %d", idx);
        VTSS_MPLS_CHECK_RC(vtss_vcap_del(vtss_state, &vtss_state->vcap.is0.obj, srvl_mpls_depth_to_is0_user(depth), idx));
        seg->u.in.has_mlbs = FALSE;

        VTSS_MPLS_CHECK(vtss_state->mpls.is0_mlbs_cnt > 0);
        vtss_state->mpls.is0_mlbs_cnt--;
    }
    return VTSS_RC_OK;
}

/* Update IS0 MLBS entry for in-segment. If this segment is itself a server then
 * the clients must be updated as well, but that's outside the scope of this
 * function.
 */
static vtss_rc srvl_mpls_is0_mlbs_update(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
#if (VTSS_MPLS_IN_ENCAP_LABEL_CNT != 3)
#error "This code assumes VTSS_MPLS_IN_ENCAP_LABEL_CNT == 3"
#endif
    vtss_mpls_segment_internal_t *seg        = &SEG_I(idx);
    vtss_mpls_segment_internal_t *srv1       = VTSS_MPLS_IDX_IS_DEF(seg->pub.server_idx) ? &SEG_I(seg->pub.server_idx) : NULL;
    vtss_mpls_segment_internal_t *srv2       = srv1 && VTSS_MPLS_IDX_IS_DEF(srv1->pub.server_idx) ? &SEG_I(srv1->pub.server_idx) : NULL;
    vtss_mpls_l2_internal_t      *l2         = NULL;
    vtss_mpls_segment_idx_t      seg_with_l2 = idx;      // Just an assumption as this point
    u8                           depth       = 0;
    vtss_mpls_xc_internal_t      *xc;
    i16                          i;
    vtss_rc                      rc;
    vtss_is0_entry_t             entry;
    vtss_vcap_data_t             data;
    BOOL                         already_has_mlbs = seg->u.in.has_mlbs;

    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "entry, segment %d", idx);

    VTSS_MPLS_IDX_CHECK_XC(seg->pub.xc_idx);
    xc = &XC_I(seg->pub.xc_idx);

    VTSS_MPLS_CHECK(xc->isdx);

    memset(&entry, 0, sizeof(entry));
    entry.type = VTSS_IS0_TYPE_MLBS;

    // Configure label stack for MLBS key

    entry.action.mlbs.pop_count = VTSS_IS0_MLBS_POPCOUNT_18;  // DMAC, SMAC, eth-type, 1 label

    if (srv1) {
        seg_with_l2 = seg->pub.server_idx;
        entry.action.mlbs.pop_count++;           // One more label
        if (srv2) {
            seg_with_l2 = srv1->pub.server_idx;
            entry.action.mlbs.pop_count++;       // One more label
            entry.key.mlbs.label_stack[depth].value      = srv2->pub.label.value;
            entry.key.mlbs.label_stack[depth].tc         = srv2->pub.label.tc;
            entry.key.mlbs.label_stack[depth].value_mask = VTSS_BITMASK(srv2->pub.label.value == VTSS_MPLS_LABEL_VALUE_DONTCARE ? 0 : 20);
            entry.key.mlbs.label_stack[depth].tc_mask    = VTSS_BITMASK(srv2->pub.label.tc    == VTSS_MPLS_TC_VALUE_DONTCARE    ? 0 :  8);
            depth++;
        }
        entry.key.mlbs.label_stack[depth].value      = srv1->pub.label.value;
        entry.key.mlbs.label_stack[depth].tc         = srv1->pub.label.tc;
        entry.key.mlbs.label_stack[depth].value_mask = VTSS_BITMASK(srv1->pub.label.value == VTSS_MPLS_LABEL_VALUE_DONTCARE ? 0 : 20);
        entry.key.mlbs.label_stack[depth].tc_mask    = VTSS_BITMASK(srv1->pub.label.tc    == VTSS_MPLS_TC_VALUE_DONTCARE    ? 0 :  8);
        depth++;
    }
    entry.key.mlbs.label_stack[depth].value      = seg->pub.label.value;
    entry.key.mlbs.label_stack[depth].tc         = seg->pub.label.tc;
    entry.key.mlbs.label_stack[depth].value_mask = VTSS_BITMASK(seg->pub.label.value == VTSS_MPLS_LABEL_VALUE_DONTCARE ? 0 : 20);
    entry.key.mlbs.label_stack[depth].tc_mask    = VTSS_BITMASK(seg->pub.label.tc    == VTSS_MPLS_TC_VALUE_DONTCARE    ? 0 :  8);
    depth++;

    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(SEG_P(seg_with_l2).l2_idx));
    l2 = &L2_I(SEG_P(seg_with_l2).l2_idx);

    // Pop VID if L2 has it

    entry.action.mlbs.pop_count += (l2->pub.vid != 0) ? 1 : 0;

    // Get MLL:LL_IDX

    i = seg->pub.upstream ? l2->ll_idx_upstream : l2->ll_idx_downstream;
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(i));
    entry.key.mlbs.linklayer_index = i;

    // Choose label for swap, tc, ttl.
    // FIXME: More to come
    entry.action.mlbs.tc_label_index          = depth - 1;
    entry.action.mlbs.ttl_label_index         = depth - 1;
    entry.action.mlbs.swap_label_index        = depth - 1;

    // VProfile, classification output setup
    entry.action.mlbs.vprofile_index     = seg->u.in.vprofile_idx;
    entry.action.mlbs.use_service_config = TRUE;
    entry.action.mlbs.classified_vid     = 0;
    entry.action.mlbs.s_tag              = 0;
    entry.action.mlbs.pcp                = 0;
    entry.action.mlbs.dei                = 0;

    switch (xc->pub.type) {
    case VTSS_MPLS_XC_TYPE_LER:
        entry.action.mlbs.cw_enable    = seg->pub.pw_conf.process_cw;
        entry.action.mlbs.terminate_pw = TRUE;
        entry.action.mlbs.pop_count    += seg->pub.pw_conf.process_cw ? 1 : 0;
        break;
    case VTSS_MPLS_XC_TYPE_LSR:
        break;
    }

    entry.action.mlbs.isdx              = xc->isdx->sdx;
    entry.action.mlbs.e_lsp             = seg->pub.e_lsp;
    entry.action.mlbs.add_tc_to_isdx    = FALSE;            // Depends on statistics: Per service or per COS per service
    entry.action.mlbs.tc_maptable_index = VTSS_MPLS_IDX_IS_DEF(seg->pub.tc_qos_map_idx) ? seg->pub.tc_qos_map_idx : 0;
    entry.action.mlbs.l_lsp_qos_class   = seg->pub.l_lsp_cos;

#if 0
    /* Fields with default zero. Just kept here for reference */
    entry.action.mlbs.b_portlist
    entry.action.mlbs.cpu_queue = ;
    entry.action.mlbs.oam = ;
    entry.action.mlbs.oam_buried_mip = ;
    entry.action.mlbs.oam_reserved_label_value = ;
    entry.action.mlbs.oam_reserved_label_bottom_of_stack = ;
    entry.action.mlbs.oam_isdx = ;
    entry.action.mlbs.oam_isdx_add_replace = ;
    entry.action.mlbs.classified_vid = ;
    entry.action.mlbs.s_tag = ;
    entry.action.mlbs.pcp = ;
    entry.action.mlbs.dei = ;
#endif

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Updating MLBS %d: ISDX %d, label stack  %d/%d  %d/%d  %d/%d",
             idx, xc->isdx->sdx,
             entry.key.mlbs.label_stack[0].value,
             entry.key.mlbs.label_stack[0].tc,
             entry.key.mlbs.label_stack[1].value,
             entry.key.mlbs.label_stack[1].tc,
             entry.key.mlbs.label_stack[2].value,
             entry.key.mlbs.label_stack[2].tc);

    data.key_size    = VTSS_VCAP_KEY_SIZE_HALF;
    data.u.is0.entry = &entry;

    rc = vtss_vcap_add(vtss_state, &vtss_state->vcap.is0.obj, srvl_mpls_depth_to_is0_user(depth),
                       idx, VTSS_VCAP_ID_LAST, &data, FALSE);

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "MLBS Done, rc = %d", rc);

    seg->u.in.has_mlbs = (rc == VTSS_RC_OK);

    if (!already_has_mlbs  &&  seg->u.in.has_mlbs) {
        vtss_state->mpls.is0_mlbs_cnt++;
        VTSS_MPLS_CHECK(vtss_state->mpls.is0_mlbs_cnt <= VTSS_MPLS_LSP_CNT &&
                        vtss_state->mpls.is0_mll_cnt + vtss_state->mpls.is0_mlbs_cnt / 2 <= SRVL_IS0_CNT);
    }

    return rc;
}

static vtss_rc srvl_mpls_is0_mlbs_tear_all_recursive(vtss_state_t *vtss_state,
                                                     vtss_mpls_segment_idx_t idx, u8 depth)
{
    vtss_mpls_idxchain_iter_t    iter;
    vtss_mpls_idxchain_user_t    user;
    BOOL                         more;
    vtss_mpls_segment_internal_t *seg;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry, segment %d, depth = %u", idx, depth);
    VTSS_MPLS_IDX_CHECK_SEGMENT(idx);
    VTSS_MPLS_CHECK(depth <= VTSS_MPLS_IN_ENCAP_LABEL_CNT);

    seg = &SEG_I(idx);
    VTSS_MPLS_CHECK_RC(srvl_mpls_is0_mlbs_free(vtss_state, idx));

    more = vtss_mpls_idxchain_get_first(vtss_state, &seg->clients, &iter, &user);
    while (more) {
        VTSS_MPLS_CHECK_RC(srvl_mpls_is0_mlbs_tear_all_recursive(vtss_state, user, depth + 1));
        more = vtss_mpls_idxchain_get_next(vtss_state, &iter, &user);
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_is0_mlbs_tear_all(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    u8 depth;

    (void) srvl_mpls_find_ultimate_server(vtss_state, idx, &depth);
    return srvl_mpls_is0_mlbs_tear_all_recursive(vtss_state, idx, depth);
}



/* ----------------------------------------------------------------- *
 * MPLS Egress encapsulation entries
 * ----------------------------------------------------------------- */

static vtss_rc srvl_mpls_encap_update(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx);

static vtss_rc srvl_mpls_encap_alloc(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    vtss_mpls_segment_internal_t *seg = &SEG_I(idx);
    vtss_mpls_idxchain_iter_t    dummy;
    vtss_mpls_idxchain_user_t    user;
    
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->u.out.encap_idx));

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Allocating new MPLS encap entry id for out-seg %d", idx);
    VTSS_MPLS_CHECK(vtss_mpls_idxchain_get_first(vtss_state, &vtss_state->mpls.encap_free_chain, &dummy, &user));
    VTSS_MPLS_CHECK_RC(vtss_mpls_idxchain_remove_first(vtss_state, &vtss_state->mpls.encap_free_chain, user));
    vtss_state->mpls.encap_cnt++;
    VTSS_MPLS_CHECK(vtss_state->mpls.encap_cnt <= VTSS_MPLS_OUT_ENCAP_CNT);
    VTSS_MPLS_CHECK(user != 0);   // Index 0 doesn't work for ES0; it won't push the encap then.
    seg->u.out.encap_idx = user;

    return srvl_mpls_encap_update(vtss_state, idx);
}

static vtss_rc srvl_mpls_encap_free(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    vtss_mpls_segment_internal_t *seg = &SEG_I(idx);
    
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Removing encap entry %d", seg->u.out.encap_idx);
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(seg->u.out.encap_idx));
    VTSS_MPLS_CHECK(seg->u.out.has_encap);

    // Put index back on the free chain
    VTSS_MPLS_CHECK_RC(vtss_mpls_idxchain_insert_at_head(vtss_state, &vtss_state->mpls.encap_free_chain, seg->u.out.encap_idx));
    VTSS_MPLS_IDX_UNDEF(seg->u.out.encap_idx);
    seg->u.out.has_encap = FALSE;

    VTSS_MPLS_CHECK(vtss_state->mpls.encap_cnt > 0);
    vtss_state->mpls.encap_cnt--;

    /* There may still be an ES0 entry that uses this encap; the caller must
     * clean it up
     */

    return VTSS_RC_OK;
}

/* Fill in segment pointer array. First item must be outer-most label, so we
 * use recursion down the server list to get to the ultimate server before
 * beginning to fill in the array.
 */
static void srvl_mpls_encap_make_seg_array(vtss_state_t *vtss_state,
                                           vtss_mpls_segment_idx_t idx,
                                           vtss_mpls_segment_internal_t *ppseg[],
                                           i8 *depth,
                                           i8 *i)
{
    vtss_mpls_segment_internal_t *s = &SEG_I(idx);
    if (*depth > VTSS_MPLS_IN_ENCAP_LABEL_CNT) {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Recursion level too deep");
        return;
    }
    if (VTSS_MPLS_IDX_IS_DEF(s->pub.server_idx)) {
        (*depth)++;
        srvl_mpls_encap_make_seg_array(vtss_state, s->pub.server_idx, ppseg, depth, i);
    }
    ppseg[(*i)++] = s;
}

static vtss_rc srvl_mpls_encap_update(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    vtss_mpls_segment_internal_t *seg[VTSS_MPLS_IN_ENCAP_LABEL_CNT] = { 0 };
    vtss_mpls_xc_internal_t      *xc [VTSS_MPLS_IN_ENCAP_LABEL_CNT] = { 0 };
    vtss_mpls_l2_internal_t      *l2;
    i8                           i;
    i8                           depth;
    vtss_rc                      rc;
    srvl_mpls_out_encap_t        encap;
    u32                          length;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "MPLS encap table setup of idx %d for segment %d", SEG_I(idx).u.out.encap_idx, idx);
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(SEG_I(idx).u.out.encap_idx));

    depth = 1;
    i = 0;
    srvl_mpls_encap_make_seg_array(vtss_state, idx, seg, &depth, &i);
    
    memset(&encap, 0, sizeof(encap));
    seg[depth-1]->u.out.encap_bytes = 0;

    l2 = &L2_I(seg[0]->pub.l2_idx);

    for (i = 0; i < depth; i++) {
        xc[i]                = &XC_I(seg[i]->pub.xc_idx);
        encap.label_stack[i] = seg[i]->pub.label;
    }
    encap.label_stack_depth = depth;

    // For LSR, set innermost label's S-bit, TC and TTL from incoming frame's
    // classified values.
    //
    // For MPLS LER, S-bit is set for the innermost label; TC and TTL are
    // taken from the tunneled label in a way depending on the DiffServ
    // tunneling mode:
    //   Pipe + Short Pipe = push new TC / TTL
    //   Uniform           = use inner (tunneled) TC / TTL in tunnel label

    switch (xc[depth-1]->pub.type) {
    case VTSS_MPLS_XC_TYPE_LSR:
        encap.remark.label[depth-1].use_qos_to_tc_map = TRUE;
        encap.remark.label[depth-1].qos_to_tc_map_idx = VTSS_MPLS_IDX_IS_DEF(seg[depth-1]->pub.tc_qos_map_idx) ? seg[depth-1]->pub.tc_qos_map_idx : 0;  // 0 == Reserved entry; 1:1 QoS => TC
        encap.remark.label[depth-1].use_ttl           = TRUE;
        encap.remark.label[depth-1].use_s_bit         = TRUE;

        // Handle TC/TTL usage for Uniform mode for the (LER) tunnel hierarchy (if any)
        for (i = depth - 2; i >= 0; i--) {
            if(xc[i]->pub.tc_tunnel_mode == VTSS_MPLS_TUNNEL_MODE_UNIFORM) {
                encap.remark.label[i].use_qos_to_tc_map = encap.remark.label[i + 1].use_qos_to_tc_map;
                encap.remark.label[i].qos_to_tc_map_idx = encap.remark.label[i + 1].qos_to_tc_map_idx & 0x07;
                encap.label_stack[i].tc                 = encap.label_stack[i + 1].tc;
            }
            else {
                encap.remark.label[i].use_qos_to_tc_map = FALSE;
            }
            if(xc[i]->pub.ttl_tunnel_mode == VTSS_MPLS_TUNNEL_MODE_UNIFORM) {
                encap.remark.label[i].use_ttl = encap.remark.label[i + 1].use_ttl;
                encap.label_stack[i].ttl      = encap.label_stack[i + 1].ttl;
            }
        }

        for (i = depth-1; i >= 0; i--) {
            VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Level %d, Lbl %d/%d/%d, qos2tc %s/%d, use TTL %s, use S %s", i, encap.label_stack[i].value, encap.label_stack[i].tc, encap.label_stack[i].ttl,
            encap.remark.label[i].use_qos_to_tc_map ? "Y":"N", encap.remark.label[i].qos_to_tc_map_idx, encap.remark.label[i].use_ttl?"Y":"N", encap.remark.label[i].use_s_bit?"Y":"N");
        }
        break;
    case VTSS_MPLS_XC_TYPE_LER:
        break;
    }

    memcpy(&encap.dmac, &l2->pub.peer_mac, sizeof(encap.dmac));
    memcpy(&encap.smac, &l2->pub.self_mac, sizeof(encap.smac));

    encap.tag_type   = l2->pub.tag_type;
    encap.vid        = l2->pub.vid;
    encap.pcp        = l2->pub.pcp;
    encap.dei        = l2->pub.dei;
    encap.ether_type = seg[depth-1]->pub.upstream ? VTSS_MLL_ETHERTYPE_UPSTREAM_ASSIGNED : VTSS_MLL_ETHERTYPE_DOWNSTREAM_ASSIGNED;
    encap.use_cw     = seg[depth-1]->pub.pw_conf.process_cw;
    encap.cw         = seg[depth-1]->pub.pw_conf.cw;

    rc = srvl_mpls_out_encap_set(vtss_state, seg[depth-1]->u.out.encap_idx, &encap, &length);
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Encap entry done, %u bytes, rc=%d", length, rc);

    seg[depth-1]->u.out.encap_bytes = length & 0xffff;
    seg[depth-1]->u.out.has_encap = TRUE;

    if (rc != VTSS_RC_OK) {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Encap entry update FAILED, rc = %d. Attempting to delete entry.", rc);
        VTSS_MPLS_CHECK_RC(srvl_mpls_encap_free(vtss_state, idx));
    }
    
    return rc;
}



/* ----------------------------------------------------------------- *
 * ES0 entries
 * ----------------------------------------------------------------- */

static vtss_rc srvl_mpls_es0_free(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    vtss_rc                      rc;
    vtss_mpls_segment_internal_t *seg = &SEG_I(idx);
    vtss_mpls_xc_internal_t      *xc  = &XC_I(seg->pub.xc_idx);

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Removing ES0 entry for segment %d", idx);
    VTSS_MPLS_CHECK(seg->u.out.has_es0);
    VTSS_MPLS_CHECK(xc->isdx != NULL);
    rc = vtss_vcap_del(vtss_state, &vtss_state->vcap.es0.obj, VTSS_ES0_USER_MPLS, idx);
    VTSS_MPLS_CHECK_RC(vtss_srvl_isdx_update_es0(vtss_state, FALSE, xc->isdx->sdx, 0));
    seg->u.out.has_es0 = FALSE;
    return rc;
}

static vtss_rc srvl_mpls_es0_update(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    vtss_mpls_segment_internal_t *seg = &SEG_I(idx);
    vtss_mpls_xc_internal_t      *xc  = &XC_I(seg->pub.xc_idx);
    vtss_rc                      rc;
    vtss_es0_entry_t             entry;
    vtss_vcap_data_t             data;
    u32                          isdx_mask;
    vtss_port_no_t               port;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Updating ES0 entry/segment %d", idx);
    VTSS_MPLS_CHECK(!seg->pub.is_in);
    VTSS_MPLS_CHECK(xc->isdx != NULL);
    VTSS_MPLS_CHECK(seg->u.out.esdx != NULL);
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(seg->u.out.encap_idx));
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(seg->pub.xc_idx));

    port = srvl_mpls_port_no_get(vtss_state, idx);

    vtss_vcap_es0_init(&data, &entry);

    entry.key.port_no          = port;

    entry.key.type             = VTSS_ES0_TYPE_ISDX;
    entry.key.isdx_neq0        = VTSS_VCAP_BIT_1;
    entry.key.data.isdx.pcp.value = 0;  // FIXME
    entry.key.data.isdx.pcp.mask  = 0;  // FIXME -- wildcard

    entry.key.data.isdx.isdx = xc->isdx->sdx;

    entry.action.mpls_encap_idx = seg->u.out.encap_idx;
    entry.action.mpls_encap_len = srvl_bytes_to_encap_len(seg->u.out.encap_bytes);
    entry.action.esdx           = seg->u.out.esdx->sdx;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Attempting ES0 config, entry/segment %d, ISDX %d, ESDX %d, encap %d",
            idx, xc->isdx->sdx, seg->u.out.esdx->sdx, seg->u.out.encap_idx);

    data.key_size    = VTSS_VCAP_KEY_SIZE_FULL;
    data.u.es0.flags = 0;
    data.u.es0.entry = &entry;

    rc = vtss_vcap_add(vtss_state, &vtss_state->vcap.es0.obj, VTSS_ES0_USER_MPLS,
                       idx, VTSS_VCAP_ID_LAST, &data, FALSE);

    seg->u.out.has_es0 = TRUE;

    isdx_mask = VTSS_BIT(VTSS_CHIP_PORT(port));
    VTSS_MPLS_CHECK_RC(vtss_srvl_isdx_update_es0(vtss_state, TRUE, xc->isdx->sdx, isdx_mask));
    
    if (rc != VTSS_RC_OK) {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "ES0 update FAILED, rc = %d. Attempting to delete entry.", rc);
        VTSS_MPLS_CHECK_RC(srvl_mpls_es0_free(vtss_state, idx));           // Clears seg->u.out.has_es0
    }

    return rc;
}



/* ----------------------------------------------------------------- *
 * Teardown of both ES0 and MPLS encapsulation entries
 * ----------------------------------------------------------------- */

static vtss_rc srvl_mpls_es0_encap_tear_all_recursive(vtss_state_t *vtss_state,
                                                      vtss_mpls_segment_idx_t idx, u8 depth)
{
    vtss_mpls_idxchain_iter_t    iter;
    vtss_mpls_idxchain_user_t    user;
    BOOL                         more;
    vtss_mpls_segment_internal_t *seg;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry, segment %d, depth = %u", idx, depth);
    VTSS_MPLS_IDX_CHECK_SEGMENT(idx);
    VTSS_MPLS_CHECK(depth <= VTSS_MPLS_OUT_ENCAP_LABEL_CNT);

    seg = &SEG_I(idx);
    if (seg->u.out.has_es0) {
        VTSS_MPLS_CHECK_RC(srvl_mpls_es0_free(vtss_state, idx));
    }
    if (seg->u.out.has_encap) {
        VTSS_MPLS_CHECK_RC(srvl_mpls_encap_free(vtss_state, idx));
    }
    if (seg->u.out.esdx) {
        VTSS_MPLS_CHECK_RC(srvl_mpls_sdx_free(vtss_state, FALSE, 1, seg->u.out.esdx));
        seg->u.out.esdx = NULL;
    }

    more = vtss_mpls_idxchain_get_first(vtss_state, &seg->clients, &iter, &user);
    while (more) {
        VTSS_MPLS_CHECK_RC(srvl_mpls_es0_encap_tear_all_recursive(vtss_state, user, depth + 1));
        more = vtss_mpls_idxchain_get_next(vtss_state, &iter, &user);
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_es0_encap_tear_all(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    u8 depth;

    (void)srvl_mpls_find_ultimate_server(vtss_state, idx, &depth);
    return srvl_mpls_es0_encap_tear_all_recursive(vtss_state, idx, depth);
}



/* ----------------------------------------------------------------- *
 *  Global TC config
 * ----------------------------------------------------------------- */

static vtss_rc srvl_mpls_tc_conf_set(vtss_state_t *vtss_state,
                                     const vtss_mpls_tc_conf_t * const new_map)
{
    u8  map, qos, tc;
    u32 dp0, dp1;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry");

    // QOS => TC: We never update entry 0; it's a static 1-1 mapping necessary
    // to carry ingress TC to egress on Serval-1.
    for (map = 1; map < VTSS_MPLS_QOS_TO_TC_MAP_CNT; map++) {
        for (qos = 0; qos < VTSS_MPLS_QOS_TO_TC_ENTRY_CNT; qos++) {
            vtss_state->mpls.tc_conf.qos_to_tc_map[map][qos].dp0_tc = new_map->qos_to_tc_map[map][qos].dp0_tc & 0x07;
            vtss_state->mpls.tc_conf.qos_to_tc_map[map][qos].dp1_tc = new_map->qos_to_tc_map[map][qos].dp1_tc & 0x07;
        }
    }

    // Commit each map to chip (including map 0)
    for (map = 0; map < VTSS_MPLS_QOS_TO_TC_MAP_CNT; map++) {
        dp0 = dp1 = 0;
        for (qos = VTSS_MPLS_QOS_TO_TC_MAP_CNT; qos > 0; qos--) {
            dp0 = (dp0 << 3) | vtss_state->mpls.tc_conf.qos_to_tc_map[map][qos - 1].dp0_tc;
            dp1 = (dp1 << 3) | vtss_state->mpls.tc_conf.qos_to_tc_map[map][qos - 1].dp1_tc;
        }
        SRVL_WR(VTSS_SYS_SYSTEM_MPLS_QOS_MAP_CFG(map),     dp0);
        SRVL_WR(VTSS_SYS_SYSTEM_MPLS_QOS_MAP_CFG(map + 8), dp1);
    }

    // TC => QOS: Here we start at zero
    for (map = 0; map < VTSS_MPLS_TC_TO_QOS_MAP_CNT; map++) {
        for (tc = 0; tc < VTSS_MPLS_TC_TO_QOS_ENTRY_CNT; tc++) {
            vtss_state->mpls.tc_conf.tc_to_qos_map[map][tc].qos = new_map->tc_to_qos_map[map][tc].qos & 0x07;
            vtss_state->mpls.tc_conf.tc_to_qos_map[map][tc].dp  = new_map->tc_to_qos_map[map][tc].dp  & 0x01;
            SRVL_WR(VTSS_ANA_MPLS_TC_MAP_TC_MAP_TBL(map, tc),
                    vtss_state->mpls.tc_conf.tc_to_qos_map[map][tc].qos << 1 |
                    vtss_state->mpls.tc_conf.tc_to_qos_map[map][tc].dp);
        }
    }

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "exit");

    return VTSS_RC_OK;
}


/* ----------------------------------------------------------------- *
 *  L2 entries
 * ----------------------------------------------------------------- */

static void srvl_mpls_segment_hw_update(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx);

static BOOL vtss_mpls_l2_equal(const vtss_mpls_l2_t * const a,
                               const vtss_mpls_l2_t * const b)
{
    return
        (a->port == b->port) &&
        (a->ring_port == b->ring_port) &&
        (a->tag_type == b->tag_type) &&
        (a->vid == b->vid) &&
        (memcmp(a->peer_mac.addr, b->peer_mac.addr, sizeof(a->peer_mac.addr)) == 0) &&
        (memcmp(a->self_mac.addr, b->self_mac.addr, sizeof(a->self_mac.addr)) == 0);
}

static vtss_rc srvl_mpls_l2_alloc(vtss_state_t *vtss_state, vtss_mpls_l2_idx_t * const idx)
{
    vtss_mpls_l2_internal_t *l2;
    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_TAKE_FROM_FREELIST(mpls.l2_free_list, mpls.l2_conf, *idx);
    l2 = &L2_I(*idx);
    memset(&l2->pub, 0, sizeof(l2->pub));
    VTSS_MPLS_IDXCHAIN_UNDEF(l2->users);
    VTSS_MPLS_IDX_UNDEF(l2->ll_idx_upstream);
    VTSS_MPLS_IDX_UNDEF(l2->ll_idx_downstream);
    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_l2_free(vtss_state_t *vtss_state, vtss_mpls_l2_idx_t idx)
{
    vtss_mpls_l2_internal_t *l2;

    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_IDX_CHECK_L2(idx);

    l2 = &L2_I(idx);

    // No segments must be using this index
    VTSS_MPLS_CHECK(VTSS_MPLS_IDXCHAIN_END(l2->users));

    // Reset port so AIL debug code can see this entry is free,
    // see vtss_common.c:vtss_debug_print_mpls().
    l2->pub.port = VTSS_PORT_NO_NONE;

    VTSS_MPLS_RETURN_TO_FREELIST(mpls.l2_free_list, mpls.l2_conf, idx);
    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_l2_set(vtss_state_t *vtss_state,
                                const vtss_mpls_l2_idx_t     idx,
                                const vtss_mpls_l2_t * const l2)
{
    vtss_mpls_l2_internal_t   *l2_i;
    vtss_mpls_idxchain_iter_t iter;
    vtss_mpls_idxchain_user_t user;
    BOOL                      more;

    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_IDX_CHECK_L2(idx);
    l2_i = &L2_I(idx);

    if (!vtss_mpls_l2_equal(l2, &l2_i->pub)) {
        memcpy(&l2_i->pub, l2, sizeof(l2_i->pub));

        if (VTSS_MPLS_IDX_IS_DEF(l2_i->ll_idx_upstream)) {
            VTSS_MPLS_CHECK_RC(srvl_mpls_is0_mll_update(vtss_state, l2_i, TRUE));
        }
        if (VTSS_MPLS_IDX_IS_DEF(l2_i->ll_idx_downstream)) {
            VTSS_MPLS_CHECK_RC(srvl_mpls_is0_mll_update(vtss_state, l2_i, FALSE));
        }
    }

    // In-segment users are easy; they don't need to know about the change;
    // out-segment users need to rebuild their encapsulations and ES0
    // entries

    more = vtss_mpls_idxchain_get_first(vtss_state, &l2_i->users, &iter, &user);
    while (more) {
        srvl_mpls_segment_hw_update(vtss_state, user);
        more = vtss_mpls_idxchain_get_next(vtss_state, &iter, &user);
    }

    return VTSS_RC_OK;
}

// Return: VTSS_RC_OK if L2 entry's HW allocation is OK
static vtss_rc srvl_mpls_l2_hw_alloc(vtss_state_t *vtss_state,
                                     const vtss_mpls_l2_idx_t      l2_idx,
                                     const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_rc                      rc   = VTSS_RC_OK;
    vtss_mpls_l2_internal_t      *l2  = &L2_I(l2_idx);
    vtss_mpls_segment_internal_t *seg = &SEG_I(seg_idx);

    if (seg->pub.is_in) {
        if ( ( seg->pub.upstream  &&  VTSS_MPLS_IDX_IS_UNDEF(l2->ll_idx_upstream))  ||
             (!seg->pub.upstream  &&  VTSS_MPLS_IDX_IS_UNDEF(l2->ll_idx_downstream)) ) {
            rc = srvl_mpls_is0_mll_alloc(vtss_state, l2, seg->pub.upstream);
        }
        seg->u.in.has_mll = (rc == VTSS_RC_OK);
    }
    
    return rc;
}


/* As soon as an in-segment is attached, an MLL will be allocated (if HW so
 * permits).
 */
static vtss_rc srvl_mpls_l2_segment_attach(vtss_state_t *vtss_state,
                                           const vtss_mpls_l2_idx_t      l2_idx,
                                           const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_mpls_l2_internal_t      *l2;
    vtss_mpls_segment_internal_t *seg;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry");

    // Rules:
    //   - Segment must not already be attached to an L2 entry
    //   - Segment must not be a client

    VTSS_MPLS_IDX_CHECK_L2(l2_idx);
    VTSS_MPLS_IDX_CHECK_SEGMENT(seg_idx);

    l2  = &L2_I(l2_idx);
    seg = &SEG_I(seg_idx);
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->pub.l2_idx));
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->pub.server_idx));
    VTSS_MPLS_CHECK(!seg->u.in.has_mll);    // Consistency check

    if (vtss_mpls_idxchain_insert_at_head(vtss_state, &l2->users, seg_idx) != VTSS_RC_OK) {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Couldn't insert segment %u in L2 idx %u user chain", seg_idx, l2_idx);
        return VTSS_RC_ERROR;
    }
    seg->pub.l2_idx = l2_idx;

    VTSS_MPLS_CHECK_RC(srvl_mpls_l2_hw_alloc(vtss_state, l2_idx, seg_idx));

    if (++vtss_state->mpls.port_state[l2->pub.port].l2_refcnt == 1) {
        // First segment attached -- make the port an LSR port and enable MPLS
        SRVL_WRM(VTSS_ANA_PORT_PORT_CFG(VTSS_CHIP_PORT(l2->pub.port)),
                 VTSS_F_ANA_PORT_PORT_CFG_LSR_MODE, VTSS_F_ANA_PORT_PORT_CFG_LSR_MODE);
        SRVL_WRM(VTSS_SYS_SYSTEM_PORT_MODE(VTSS_CHIP_PORT(l2->pub.port)),
                 VTSS_F_SYS_SYSTEM_PORT_MODE_MPLS_ENA, VTSS_F_SYS_SYSTEM_PORT_MODE_MPLS_ENA);
    }

    srvl_mpls_segment_hw_update(vtss_state, seg_idx);

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_l2_segment_detach(vtss_state_t *vtss_state,
                                           const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_mpls_l2_idx_t           l2_idx;
    vtss_mpls_l2_internal_t      *l2;
    vtss_mpls_segment_internal_t *seg;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry");

    // Rules:
    //   - Segment must be attached to an L2 entry

    VTSS_MPLS_IDX_CHECK_SEGMENT(seg_idx);
    seg    = &SEG_I(seg_idx);
    l2_idx = seg->pub.l2_idx;
    VTSS_MPLS_IDX_CHECK_L2(l2_idx);
    l2     = &L2_I(l2_idx);

    if (vtss_mpls_idxchain_remove_first(vtss_state, &l2->users, seg_idx) != VTSS_RC_OK) {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Couldn't remove segment %u from L2 idx %u user chain", seg_idx, l2_idx);
        return VTSS_RC_ERROR;
    }
    VTSS_MPLS_IDX_UNDEF(seg->pub.l2_idx);
    VTSS_MPLS_CHECK(vtss_state->mpls.port_state[l2->pub.port].l2_refcnt > 0);
    if (--vtss_state->mpls.port_state[l2->pub.port].l2_refcnt == 0) {
        // Last segment detached --  remove LSR and MPLS state from port
        SRVL_WRM(VTSS_ANA_PORT_PORT_CFG(VTSS_CHIP_PORT(l2->pub.port)),
                 0, VTSS_F_ANA_PORT_PORT_CFG_LSR_MODE);
        SRVL_WRM(VTSS_SYS_SYSTEM_PORT_MODE(VTSS_CHIP_PORT(l2->pub.port)),
                 0, VTSS_F_SYS_SYSTEM_PORT_MODE_MPLS_ENA);
    }


    if (seg->pub.is_in) {
        seg->u.in.has_mll = FALSE;
    }
    
    srvl_mpls_segment_hw_update(vtss_state, seg_idx);
    
    if (seg->pub.is_in) {
        // If no other in-segment users exist with the same upstream/downstream
        // setting as the just-removed segment, free the HW resource

        vtss_mpls_idxchain_iter_t    iter;
        vtss_mpls_idxchain_user_t    user;
        BOOL                         more;
        BOOL                         found_one = FALSE;
        
        more = vtss_mpls_idxchain_get_first(vtss_state, &l2->users, &iter, &user);
        while (more  &&  !found_one) {
            found_one = SEG_P(user).is_in  &&  SEG_P(user).upstream == seg->pub.upstream;
            more      = vtss_mpls_idxchain_get_next(vtss_state, &iter, &user);
        }
        if (! found_one) {
            VTSS_MPLS_CHECK_RC(srvl_mpls_is0_mll_free(vtss_state, l2, seg->pub.upstream));
        }
    }

    return VTSS_RC_OK;
}



/* ----------------------------------------------------------------- *
 *  MPLS Segments
 * ----------------------------------------------------------------- */

static void srvl_mpls_segment_internal_init(vtss_mpls_segment_internal_t *s, BOOL in)
{
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Segment init, type %s", in ? "in" : "out");

    memset(s, 0, sizeof(*s));

    s->pub.is_in       = in;
    s->pub.label.value = VTSS_MPLS_LABEL_VALUE_DONTCARE;
    s->pub.label.tc    = VTSS_MPLS_TC_VALUE_DONTCARE;
    s->pub.label.ttl   = 255;
    VTSS_MPLS_IDX_UNDEF(s->pub.l2_idx);
    VTSS_MPLS_IDX_UNDEF(s->pub.policer_idx);
    VTSS_MPLS_IDX_UNDEF(s->pub.server_idx);
    VTSS_MPLS_IDX_UNDEF(s->pub.xc_idx);
    VTSS_MPLS_IDXCHAIN_UNDEF(s->clients);

    VTSS_MPLS_IDX_UNDEF(s->next_free);
    s->need_hw_alloc = TRUE;

    if (in) {
        s->u.in.has_mll  = FALSE;
        s->u.in.has_mlbs = FALSE;
        VTSS_MPLS_IDX_UNDEF(s->u.in.vprofile_idx);
    }
    else {
        s->u.out.has_encap = FALSE;
        s->u.out.has_es0   = FALSE;
        s->u.out.esdx      = NULL;
        VTSS_MPLS_IDX_UNDEF(s->u.out.encap_idx);
        s->u.out.encap_bytes = 0;
    }
}

static vtss_rc srvl_mpls_segment_alloc(vtss_state_t *vtss_state,
                                       BOOL                            in_seg,
                                       vtss_mpls_segment_idx_t * const idx)
{
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_TAKE_FROM_FREELIST(mpls.segment_free_list, mpls.segment_conf, *idx);
    srvl_mpls_segment_internal_init(&vtss_state->mpls.segment_conf[*idx], in_seg);
    vtss_state->mpls.segment_conf[*idx].pub.is_in = in_seg;
    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_segment_free(vtss_state_t *vtss_state,
                                      vtss_mpls_segment_idx_t idx)
{
    vtss_mpls_segment_internal_t *seg;
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry");

    if (VTSS_MPLS_IDX_IS_UNDEF(idx)) {
        return VTSS_RC_OK;
    }
    VTSS_MPLS_IDX_CHECK_SEGMENT(idx);

    // Rules:
    //   - Segment must not be attached to an L2 entry
    //   - Segment must not be attached to an XC
    //   - Segment must not be a server
    //   - Segment must not be attached to a server

    seg = &SEG_I(idx);

    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->pub.l2_idx));
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->pub.xc_idx));
    VTSS_MPLS_CHECK(VTSS_MPLS_IDXCHAIN_END(seg->clients));
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->pub.server_idx));

    VTSS_MPLS_RETURN_TO_FREELIST(mpls.segment_free_list, mpls.segment_conf, idx);
    return VTSS_RC_OK;
}

/** \brief Add/update label stacks for segment and any clients it has.
 *
 * Does not affect IS0 VCAP ordering.
 */
static vtss_rc srvl_mpls_segment_hw_label_stack_refresh_recursive(vtss_state_t *vtss_state,
                                                                  vtss_mpls_segment_idx_t idx, u8 depth)
{
    vtss_mpls_segment_internal_t *seg = &SEG_I(idx);
    vtss_mpls_idxchain_iter_t    iter;
    vtss_mpls_idxchain_user_t    user;
    BOOL                         more;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Refreshing %s-segment %d, depth = %u", seg->pub.is_in ? "in" : "out", idx, depth);

    VTSS_MPLS_CHECK(depth <= (seg->pub.is_in ? VTSS_MPLS_IN_ENCAP_LABEL_CNT : VTSS_MPLS_OUT_ENCAP_LABEL_CNT));

    if (VTSS_MPLS_IDX_IS_UNDEF(seg->pub.xc_idx)) {
        VTSS_NG(VTSS_TRACE_GROUP_MPLS, "Segment %d has no XC, cannot update label stack", idx);
        return VTSS_RC_OK;
    }

    // Update labels for this segment
    
    if (seg->pub.is_in) {
        if (seg->u.in.has_mlbs) {
            VTSS_MPLS_CHECK_RC(srvl_mpls_is0_mlbs_update(vtss_state, idx));
        }
    }
    else {
        if (seg->u.out.has_encap) {
            VTSS_MPLS_CHECK_RC(srvl_mpls_encap_update(vtss_state, idx));
        }
        if (seg->u.out.has_es0) {
            VTSS_MPLS_CHECK_RC(srvl_mpls_es0_update(vtss_state, idx));
        }
    }

    // Update clients

    more = vtss_mpls_idxchain_get_first(vtss_state, &seg->clients, &iter, &user);
    while (more) {
        VTSS_MPLS_CHECK_RC(srvl_mpls_segment_hw_label_stack_refresh_recursive(vtss_state, user, depth + 1));
        more = vtss_mpls_idxchain_get_next(vtss_state, &iter, &user);
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_xc_hw_alloc(vtss_state_t *vtss_state, vtss_mpls_xc_idx_t idx)
{
    vtss_mpls_xc_internal_t      *xc = &XC_I(idx);
    vtss_mpls_segment_state_t    in_state, out_state;
    BOOL                         need_isdx = FALSE;
    vtss_port_no_t               port_no   = 0;       // Quiet lint

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "XC %d, HW alloc", idx);

    if (xc->isdx) {
        return VTSS_RC_OK;
    }

    switch (xc->pub.type) {
    case VTSS_MPLS_XC_TYPE_LER:
        if (VTSS_MPLS_IDX_IS_DEF(xc->pub.in_seg_idx)) {
            need_isdx = SEG_P(xc->pub.in_seg_idx).pw_conf.is_pw;
            port_no   = srvl_mpls_port_no_get(vtss_state, xc->pub.in_seg_idx);
        }
        break;
    case VTSS_MPLS_XC_TYPE_LSR:
        if (VTSS_MPLS_IDX_IS_DEF(xc->pub.in_seg_idx)  &&  VTSS_MPLS_IDX_IS_DEF(xc->pub.out_seg_idx)) {
            need_isdx = SEG_P(xc->pub.in_seg_idx).pw_conf.is_pw;
            port_no   = srvl_mpls_port_no_get(vtss_state, xc->pub.in_seg_idx);

            (void) srvl_mpls_segment_state_get(vtss_state, xc->pub.in_seg_idx,  &in_state);
            (void) srvl_mpls_segment_state_get(vtss_state, xc->pub.out_seg_idx, &out_state);
            need_isdx = in_state == VTSS_MPLS_SEGMENT_STATE_CONF  ||  out_state == VTSS_MPLS_SEGMENT_STATE_CONF;
        }
        break;
    }

    if (need_isdx) {
        VTSS_MPLS_CHECK_RC(srvl_mpls_sdx_alloc(vtss_state, TRUE, port_no, 1, &xc->isdx));
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_xc_hw_free(vtss_state_t *vtss_state, vtss_mpls_xc_idx_t idx)
{
    vtss_mpls_xc_internal_t   *xc = &XC_I(idx);
    vtss_mpls_segment_state_t in_state, out_state;
    BOOL                      free_isdx = TRUE;

    if (! xc->isdx) {
        return VTSS_RC_OK;
    }

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "XC %d, HW free. ISDX %d", idx, xc->isdx->sdx);

    switch (xc->pub.type) {
    case VTSS_MPLS_XC_TYPE_LER:
        break;
    case VTSS_MPLS_XC_TYPE_LSR:
        if (VTSS_MPLS_IDX_IS_DEF(xc->pub.in_seg_idx)  &&  VTSS_MPLS_IDX_IS_DEF(xc->pub.out_seg_idx)) {
            (void) srvl_mpls_segment_state_get(vtss_state, xc->pub.in_seg_idx,  &in_state);
            (void) srvl_mpls_segment_state_get(vtss_state, xc->pub.out_seg_idx, &out_state);
            free_isdx = in_state != VTSS_MPLS_SEGMENT_STATE_UP  &&  out_state != VTSS_MPLS_SEGMENT_STATE_UP;
        }
        break;
    }

    if (free_isdx) {
        VTSS_MPLS_CHECK_RC(srvl_mpls_sdx_free(vtss_state, TRUE, 1, xc->isdx));
        xc->isdx = NULL;
    } else {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Not freeing ISDX %d; still used", xc->isdx->sdx);
    }

    return VTSS_RC_OK;

}

// Helper function for @srvl_mpls_segment_hw_update() and friends, don't call directly
static vtss_rc srvl_mpls_segment_hw_free_recursive(vtss_state_t *vtss_state,
                                                   vtss_mpls_segment_idx_t idx)
{
    vtss_mpls_segment_internal_t *seg = &SEG_I(idx);
    vtss_mpls_idxchain_iter_t    iter;
    vtss_mpls_idxchain_user_t    user;
    BOOL                         more;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry, segment %d", idx);

    if (seg->pub.is_in) {
        (void) srvl_mpls_vprofile_free(vtss_state, seg->u.in.vprofile_idx);
        VTSS_MPLS_IDX_UNDEF(seg->u.in.vprofile_idx);
    }

    (void) srvl_mpls_xc_hw_free(vtss_state, seg->pub.xc_idx);

    seg->need_hw_alloc = TRUE;

    more = vtss_mpls_idxchain_get_first(vtss_state, &seg->clients, &iter, &user);
    while (more) {
        VTSS_MPLS_CHECK_RC(srvl_mpls_segment_hw_free_recursive(vtss_state, user));
        more = vtss_mpls_idxchain_get_next(vtss_state, &iter, &user);
    }

    return VTSS_RC_OK;
}

// Helper function for @srvl_mpls_segment_hw_update() and friends, don't call directly
static vtss_rc srvl_mpls_segment_hw_free(vtss_state_t *vtss_state,
                                         vtss_mpls_segment_idx_t idx)
{
    vtss_mpls_segment_internal_t *seg = &SEG_I(idx);

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry, segment %d", idx);

    /* We do actual tearing here, since this func is called with the
     * ultimate server as index *and* since the MLBS/ES0/encap tear
     * funcs are recursive.
     */

    if (seg->pub.is_in) {
        // We never free the MLL since it's owned by the L2 entry
        if (seg->u.in.has_mlbs) {
            (void) srvl_mpls_is0_mlbs_tear_all(vtss_state, idx);
        }
    }
    else {
        if (seg->u.out.has_encap  ||  seg->u.out.has_es0) {
            (void) srvl_mpls_es0_encap_tear_all(vtss_state, idx);
        }
    }

    return srvl_mpls_segment_hw_free_recursive(vtss_state, idx);
}

// Helper function for @srvl_mpls_segment_hw_update(), don't call directly
static vtss_rc srvl_mpls_segment_hw_alloc_recursive(vtss_state_t *vtss_state,
                                                    vtss_mpls_segment_idx_t idx, u8 depth)
{
    vtss_mpls_idxchain_iter_t    iter;
    vtss_mpls_idxchain_user_t    user;
    BOOL                         more;
    vtss_mpls_segment_state_t    state;
    vtss_mpls_segment_internal_t *seg    = &SEG_I(idx);
    vtss_mpls_xc_internal_t      *xc     = &XC_I(seg->pub.xc_idx);
    BOOL                         is_lsr  = FALSE;
    BOOL                         is_pw   = FALSE;
    vtss_port_no_t               l2_port = srvl_mpls_port_no_get(vtss_state, idx);

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry: container = %d, depth %d, alloc needed: %s", idx, depth, seg->need_hw_alloc ? "Yes" : "no");
    VTSS_MPLS_CHECK(depth <= VTSS_MPLS_IN_ENCAP_LABEL_CNT);

    (void) srvl_mpls_segment_state_get(vtss_state, idx, &state);
    if (state != VTSS_MPLS_SEGMENT_STATE_CONF) {
        VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Segment %d is not CONF but %s; cannot allocate HW", idx, srvl_mpls_segment_state_to_str(state));
        return VTSS_RC_ERROR;
    }

    if (VTSS_MPLS_IDX_IS_DEF(seg->pub.server_idx)) {
        (void) srvl_mpls_segment_state_get(vtss_state, seg->pub.server_idx, &state);
        if (state != VTSS_MPLS_SEGMENT_STATE_UP) {
            VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Server %d is not UP but %s; cannot allocate HW", seg->pub.server_idx, srvl_mpls_segment_state_to_str(state));
            return VTSS_RC_ERROR;
        }
    }

    seg->need_hw_alloc = FALSE;

    // Allocate XC HW resources or bail out
    if (srvl_mpls_xc_hw_alloc(vtss_state, seg->pub.xc_idx) != VTSS_RC_OK) {
        goto err_out;
    }

    is_lsr = xc->pub.type == VTSS_MPLS_XC_TYPE_LSR;
    is_pw  = xc->pub.type == VTSS_MPLS_XC_TYPE_LER  &&  seg->pub.pw_conf.is_pw;

    if (seg->pub.is_in) {
        // For LSR: use fixed VProfile;
        // For LER PW: allocate VProfile or bail out
        if (is_lsr) {
            seg->u.in.vprofile_idx = VTSS_MPLS_VPROFILE_LSR_IDX;
        }
        else if (is_pw) {
            vtss_mpls_vprofile_t *vp;

            (void) srvl_mpls_vprofile_alloc(vtss_state, &seg->u.in.vprofile_idx);
            if (VTSS_MPLS_IDX_IS_UNDEF(seg->u.in.vprofile_idx)) {
                goto err_out;
            }
            vp               = &VP_P(seg->u.in.vprofile_idx);
            vp->s1_s2_enable = TRUE;
            vp->port         = l2_port;
            vp->recv_enable  = TRUE;

            // FIXME: Raw/tagged mode setup
            vp->vlan_aware          = TRUE;
            vp->map_dscp_cos_enable = FALSE;
            vp->map_eth_cos_enable  = FALSE;

            (void) srvl_mpls_vprofile_hw_update(vtss_state, seg->u.in.vprofile_idx);
        }

        // LSR, LER PW: Build label stack for ourself or bail out
        if (is_lsr  ||  is_pw) {
            (void) srvl_mpls_is0_mlbs_update(vtss_state, idx);
            if (!seg->u.in.has_mlbs) {
                goto err_out;
            }
        }
    }
    else {
        // Allocate entries or bail out.
        // Note order: ES0 depends on encap and ESDX

        if ((is_lsr || is_pw)  &&  !seg->u.out.has_encap) {
            (void) srvl_mpls_encap_alloc(vtss_state, idx);
            if (!seg->u.out.has_encap) {
                goto err_out;
            }
        }

        if (is_lsr  &&  seg->u.out.esdx == NULL) {
            (void) srvl_mpls_sdx_alloc(vtss_state, FALSE, l2_port, 1, &seg->u.out.esdx);
            if (seg->u.out.esdx == NULL) {
                goto err_out;
            }
        }

        if (is_lsr  &&  !seg->u.out.has_es0) {
            (void) srvl_mpls_es0_update(vtss_state, idx);
            if (!seg->u.out.has_es0) {
                goto err_out;
            }
        }
    }

    // Go through all clients
    more = vtss_mpls_idxchain_get_first(vtss_state, &seg->clients, &iter, &user);
    while (more) {
        VTSS_MPLS_CHECK_RC(srvl_mpls_segment_hw_alloc_recursive(vtss_state, user, depth + 1));
        more = vtss_mpls_idxchain_get_next(vtss_state, &iter, &user);
    }

    return VTSS_RC_OK;

err_out:
    (void) srvl_mpls_segment_hw_free(vtss_state, idx);
    return VTSS_RC_ERROR;
}

// Helper function for @srvl_mpls_segment_hw_update(), don't call directly
static vtss_rc srvl_mpls_segment_hw_alloc(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    u8 depth;
    (void) srvl_mpls_find_ultimate_server(vtss_state, idx, &depth);
    return srvl_mpls_segment_hw_alloc_recursive(vtss_state, idx, depth);
}

// Helper function for @srvl_mpls_segment_hw_update(), don't call directly
static vtss_rc srvl_mpls_segment_hw_refresh(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    u8 depth;
    (void) srvl_mpls_find_ultimate_server(vtss_state, idx, &depth);
    return srvl_mpls_segment_hw_label_stack_refresh_recursive(vtss_state, idx, depth);
}

static void srvl_mpls_segment_hw_update(vtss_state_t *vtss_state, vtss_mpls_segment_idx_t idx)
{
    vtss_rc                   rc = VTSS_RC_OK;  // Kill silly compiler warning
    vtss_mpls_segment_state_t state;

    (void) srvl_mpls_segment_state_get(vtss_state, idx, &state);
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Update of segment %d, entry state %s", idx,
            srvl_mpls_segment_state_to_str(state));

    switch (state) {
    case VTSS_MPLS_SEGMENT_STATE_UNCONF:
        rc = srvl_mpls_segment_hw_free(vtss_state, idx);
        break;
    case VTSS_MPLS_SEGMENT_STATE_CONF:
        SEG_I(idx).need_hw_alloc = TRUE;
        rc = srvl_mpls_segment_hw_alloc(vtss_state, idx);
        break;
    case VTSS_MPLS_SEGMENT_STATE_UP:
        rc = srvl_mpls_segment_hw_refresh(vtss_state, idx);
        break;
    }

    (void) srvl_mpls_segment_state_get(vtss_state, idx, &state);
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "Update of segment %d done, exit state %s, rc = %d", idx,
            srvl_mpls_segment_state_to_str(state), rc);
}

static vtss_rc srvl_mpls_segment_set(vtss_state_t *vtss_state,
                                     const vtss_mpls_segment_idx_t     idx,
                                     const vtss_mpls_segment_t * const seg)
{
    vtss_mpls_segment_internal_t *seg_i;
    BOOL                         update;
    vtss_mpls_segment_state_t    state;

    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_IDX_CHECK_SEGMENT(idx);
    seg_i = &SEG_I(idx);

    // Rules:
    //   - Can't change L2 index once set (but can detach with another func call)
    //   - Can't change server index once set (but can detach with another func call)
    //   - Can't change XC index once set (but can detach with another func call)
    //   - Can't change in/out type
    //   - Can't change upstream flag if attached to L2
    //   - Can't change between PW/LSP type if attached to XC

#define TST_IDX(x)     VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg_i->pub.x)   || (seg->x == seg_i->pub.x))
#define TST_DEP(idx,x) VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg_i->pub.idx) || (seg->x == seg_i->pub.x))

    TST_IDX(l2_idx);
    TST_IDX(server_idx);
    TST_IDX(xc_idx);
    VTSS_MPLS_CHECK(seg->is_in == seg_i->pub.is_in);
    TST_DEP(l2_idx, upstream);
    TST_DEP(xc_idx, pw_conf.is_pw);

#undef TST_IDX
#undef TST_DEP

    // Can change label, E-/L-LSP type, TC, policer

    update = (seg->label.value != seg_i->pub.label.value)  ||
             (seg->label.tc    != seg_i->pub.label.tc)     ||
             (seg->e_lsp       != seg_i->pub.e_lsp);

    if (seg->is_in) {
        update = update ||
            (seg->label.ttl      != seg_i->pub.label.ttl) ||
            (seg->l_lsp_cos      != seg_i->pub.l_lsp_cos)  ||
            (seg->tc_qos_map_idx != seg_i->pub.tc_qos_map_idx);
    }
    else {
        update = update ||
            (seg->tc_qos_map_idx != seg_i->pub.tc_qos_map_idx);
    }

    seg_i->pub = *seg;

    (void) srvl_mpls_segment_state_get(vtss_state, idx, &state);
    if (state != VTSS_MPLS_SEGMENT_STATE_UP  ||  update) {
        srvl_mpls_segment_hw_update(vtss_state, idx);
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_segment_server_attach(vtss_state_t *vtss_state,
                                               const vtss_mpls_segment_idx_t idx,
                                               const vtss_mpls_segment_idx_t srv_idx)
{
    vtss_mpls_segment_t *seg;
    vtss_mpls_segment_t *srv_seg;
    u8                  depth;

    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_IDX_CHECK_SEGMENT(idx);
    VTSS_MPLS_IDX_CHECK_SEGMENT(srv_idx);

    seg     = &SEG_P(idx);
    srv_seg = &SEG_P(srv_idx);

    // Rules:
    //   - Segment must not already be attached to a server
    //   - Both segments must be attached to XCs
    //   - The XCs can't be the same
    //   - The segments must be different
    //   - Our HW can handle a limited label stack depth; don't allow more
    //   - We can't use an LSR XC as server
    //  (- The segment can't already be in server's client list. Invariant
    //     check; shouldn't be possible since seg can't be attached to server in
    //     the first place)

    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->server_idx));
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(seg->xc_idx));
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(srv_seg->xc_idx));
    VTSS_MPLS_CHECK(XC_P(srv_seg->xc_idx).type != VTSS_MPLS_XC_TYPE_LSR);
    VTSS_MPLS_CHECK(seg->xc_idx != srv_seg->xc_idx);
    VTSS_MPLS_CHECK(idx != srv_idx);
    (void) srvl_mpls_find_ultimate_server(vtss_state, srv_idx, &depth);
    VTSS_MPLS_CHECK(depth <= (seg->is_in ? VTSS_MPLS_IN_ENCAP_LABEL_CNT : VTSS_MPLS_OUT_ENCAP_LABEL_CNT));
    VTSS_MPLS_CHECK(!vtss_mpls_idxchain_find(vtss_state, &SEG_I(srv_idx).clients, idx));

    VTSS_MPLS_CHECK_RC(vtss_mpls_idxchain_append_to_tail(vtss_state, &SEG_I(srv_idx).clients, idx));
    seg->server_idx = srv_idx;

    srvl_mpls_segment_hw_update(vtss_state, idx);

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_segment_server_detach(vtss_state_t *vtss_state,
                                               const vtss_mpls_segment_idx_t idx)
{
    vtss_mpls_segment_t *seg;

    // Rules:
    //   - Segment must be attached to a server
    //  (- Segment must be in server's client list -- consistency check)

    VTSS_MPLS_IDX_CHECK_SEGMENT(idx);
    seg = &SEG_P(idx);
    VTSS_MPLS_IDX_CHECK_SEGMENT(seg->server_idx);

    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "Detaching segment %d from server %d", idx, seg->server_idx);
    VTSS_MPLS_CHECK_RC(vtss_mpls_idxchain_remove_first(vtss_state, &SEG_I(seg->server_idx).clients, idx));
    VTSS_MPLS_IDX_UNDEF(seg->server_idx);

    srvl_mpls_segment_hw_update(vtss_state, idx);

    return VTSS_RC_OK;
}



/* ----------------------------------------------------------------- *
 *  XC entries
 * ----------------------------------------------------------------- */

static vtss_rc srvl_mpls_xc_alloc(vtss_state_t *vtss_state,
                                  vtss_mpls_xc_type_t type,
                                  vtss_mpls_xc_idx_t * const idx)
{
    vtss_mpls_xc_internal_t *xc;
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_TAKE_FROM_FREELIST(mpls.xc_free_list, mpls.xc_conf, *idx);

    xc = &vtss_state->mpls.xc_conf[*idx];
    xc->isdx = NULL;
    VTSS_MPLS_IDX_UNDEF(xc->next_free);
    VTSS_MPLS_IDXCHAIN_UNDEF(xc->mc_chain);

    xc->pub.type = type;
    VTSS_MPLS_IDX_UNDEF(xc->pub.in_seg_idx);
    VTSS_MPLS_IDX_UNDEF(xc->pub.out_seg_idx);
#if 0
    VTSS_MPLS_IDX_UNDEF(xc->pub.prot_linear_w_idx);
    VTSS_MPLS_IDX_UNDEF(xc->pub.prot_linear_p_idx);
    VTSS_MPLS_IDX_UNDEF(xc->pub.prot_fb_idx);
#endif

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_xc_free(vtss_state_t *vtss_state, vtss_mpls_xc_idx_t idx)
{
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry");

    if (VTSS_MPLS_IDX_IS_UNDEF(idx)) {
        return VTSS_RC_OK;
    }
    VTSS_MPLS_IDX_CHECK_XC(idx);
    VTSS_MPLS_RETURN_TO_FREELIST(mpls.xc_free_list, mpls.xc_conf, idx);
    return VTSS_RC_OK;
}


static vtss_rc srvl_mpls_xc_set(vtss_state_t *vtss_state,
                                const vtss_mpls_xc_idx_t     idx,
                                const vtss_mpls_xc_t * const xc)
{
    vtss_mpls_xc_t *old_xc;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry, XC=%d", idx);
    VTSS_MPLS_IDX_CHECK_XC(idx);
    old_xc = &XC_P(idx);

    // Rules:
    //   - Can't change indices once set
    //   - Can't change type

#define TST_IDX(x)     VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(old_xc->x)   || (xc->x == old_xc->x))
#define TST_DEP(idx,x) VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(old_xc->idx) || (xc->x == old_xc->x))

    VTSS_MPLS_CHECK(old_xc->type == xc->type);
    TST_IDX(in_seg_idx);
    TST_IDX(out_seg_idx);

    // FIXME: More tests once protection is integrated

#undef TST_IDX
#undef TST_DEP

    if (xc->tc_tunnel_mode != old_xc->tc_tunnel_mode  ||  xc->ttl_tunnel_mode != old_xc->ttl_tunnel_mode) {
        old_xc->tc_tunnel_mode  = xc->tc_tunnel_mode;
        old_xc->ttl_tunnel_mode = xc->ttl_tunnel_mode;
        if (VTSS_MPLS_IDX_IS_DEF(old_xc->out_seg_idx)) {
            srvl_mpls_segment_hw_update(vtss_state, old_xc->out_seg_idx);
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_xc_segment_attach(vtss_state_t *vtss_state,
                                           const vtss_mpls_xc_idx_t      xc_idx,
                                           const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_mpls_xc_t               *xc;
    vtss_mpls_segment_internal_t *seg;

    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_IDX_CHECK_XC(xc_idx);
    VTSS_MPLS_IDX_CHECK_SEGMENT(seg_idx);

    xc  = &XC_P(xc_idx);
    seg = &SEG_I(seg_idx);

    // Rules:
    //   - Segment must not already be attached to an XC
    //   - Can't overwrite an already attached segment of same kind
    //   - Segment must match XC type (e.g. "only one segment for LERs")
    //  (- Segment can't have clients yet. Invariant check)
    //  (- Segment can't be attached to a server segment yet. Invariant check)

    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->pub.xc_idx));
    VTSS_MPLS_CHECK(VTSS_MPLS_IDXCHAIN_END(seg->clients));
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->pub.server_idx));

    switch (xc->type) {
    case VTSS_MPLS_XC_TYPE_LSR:
        // No check necessary
        break;
    case VTSS_MPLS_XC_TYPE_LER:
        // Either an in- or an out-segment, but not both:
        VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->pub.is_in ? xc->out_seg_idx : xc->in_seg_idx));
        break;
    }

    if (seg->pub.is_in) {
        VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(xc->in_seg_idx));
        xc->in_seg_idx     = seg_idx;
        seg->pub.xc_idx    = xc_idx;
    }
    else {
        VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(xc->out_seg_idx));
        xc->out_seg_idx     = seg_idx;
        seg->pub.xc_idx     = xc_idx;
    }

    srvl_mpls_segment_hw_update(vtss_state, seg_idx);

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_xc_segment_detach(vtss_state_t *vtss_state,
                                           const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_mpls_xc_idx_t           xc_idx;
    vtss_mpls_xc_t               *xc;
    vtss_mpls_segment_internal_t *seg;

    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "entry: Detach seg %d", seg_idx);

    VTSS_MPLS_IDX_CHECK_SEGMENT(seg_idx);
    seg = &SEG_I(seg_idx);
    xc_idx = seg->pub.xc_idx;

    // Rules:
    //   - Segment must be attached to an XC (and XC to segment)
    //   - Segment can't be server to other segments  -- FIXME: Change?
    //   - Segment can't be attached to a server      -- FIXME: Change?

    VTSS_MPLS_IDX_CHECK_XC(xc_idx);
    VTSS_MPLS_CHECK(VTSS_MPLS_IDXCHAIN_END(seg->clients));
    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_UNDEF(seg->pub.server_idx));

    xc = &XC_P(xc_idx);

    VTSS_MPLS_CHECK((seg_idx == xc->in_seg_idx) || (seg_idx == xc->out_seg_idx));  // Consistency check

    if (seg->pub.is_in) {
        VTSS_MPLS_IDX_UNDEF(xc->in_seg_idx);
        VTSS_MPLS_IDX_UNDEF(seg->pub.xc_idx);
    }
    else {
        VTSS_MPLS_IDX_UNDEF(xc->out_seg_idx);
        VTSS_MPLS_IDX_UNDEF(seg->pub.xc_idx);
    }

    srvl_mpls_segment_hw_update(vtss_state, seg_idx);
    
    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_xc_mc_segment_attach(vtss_state_t *vtss_state,
                                              const vtss_mpls_xc_idx_t      xc_idx,
                                              const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_mpls_xc_internal_t *xc;
    vtss_mpls_segment_t     *seg;

    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_IDX_CHECK_XC(xc_idx);
    VTSS_MPLS_IDX_CHECK_SEGMENT(seg_idx);

    xc  = &XC_I(xc_idx);
    seg = &SEG_P(seg_idx);

    // Rules:
    //   - Segment must be attached to an XC (and not this one, @xc_idx)
    //   - Must be out-segment
    //   - Must not already be in chain

    VTSS_MPLS_CHECK(VTSS_MPLS_IDX_IS_DEF(seg->xc_idx));
    VTSS_MPLS_CHECK(seg->xc_idx != xc_idx);
    VTSS_MPLS_CHECK(!seg->is_in);
    VTSS_MPLS_CHECK(!vtss_mpls_idxchain_find(vtss_state, &xc->mc_chain, seg_idx));

    // Add to chain

    VTSS_MPLS_CHECK_RC(vtss_mpls_idxchain_insert_at_head(vtss_state, &xc->mc_chain, seg_idx));

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_xc_mc_segment_detach(vtss_state_t *vtss_state,
                                              const vtss_mpls_xc_idx_t      xc_idx,
                                              const vtss_mpls_segment_idx_t seg_idx)
{
    vtss_mpls_xc_internal_t *xc;
    vtss_mpls_segment_t     *seg;

    VTSS_NG(VTSS_TRACE_GROUP_MPLS, "entry");

    VTSS_MPLS_IDX_CHECK_XC(xc_idx);
    VTSS_MPLS_IDX_CHECK_SEGMENT(seg_idx);

    xc  = &XC_I(xc_idx);
    seg = &SEG_P(seg_idx);

    VTSS_MPLS_CHECK_RC(vtss_mpls_idxchain_remove_first(vtss_state, &xc->mc_chain, seg_idx));
    VTSS_MPLS_IDX_UNDEF(seg->xc_idx);

    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_xc_evc_attach_calc(vtss_evc_id_t evc_id, vtss_res_t *res)
{
    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_xc_evc_detach(vtss_evc_id_t evc_id)
{
    return VTSS_RC_OK;
}

static vtss_rc srvl_mpls_xc_evc_attach(vtss_evc_id_t evc_id)
{
    return VTSS_RC_OK;
}

void vtss_mpls_ece_is1_update(vtss_state_t *vtss_state,
                              vtss_evc_entry_t *evc, vtss_ece_entry_t *ece,
                              vtss_sdx_entry_t *isdx, vtss_is1_key_t *key)
{
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "MPLS ingress XC %d, PB VID %d, IVID %d",
            evc->pw_ingress_xc, evc->vid, evc->ivid);

    if (VTSS_MPLS_IDX_IS_DEF(evc->pw_ingress_xc)) {
        const vtss_mpls_xc_internal_t *xc     = &XC_I(evc->pw_ingress_xc);
        const vtss_mpls_segment_idx_t seg_idx = xc->pub.in_seg_idx;

        if (VTSS_MPLS_IDX_IS_DEF(seg_idx)) {
            vtss_mpls_segment_state_t state;
            vtss_port_no_t            l2_port = srvl_mpls_port_no_get(vtss_state, seg_idx);

            (void) srvl_mpls_segment_state_get(vtss_state, seg_idx, &state);

            if (state == VTSS_MPLS_SEGMENT_STATE_UP  &&  l2_port == isdx->port_no) {
                key->isdx.value[0] = xc->isdx->sdx >> 8;
                key->isdx.value[1] = xc->isdx->sdx & 0xff;
                key->isdx.mask[0]  = 0xff;
                key->isdx.mask[1]  = 0xff;
                VTSS_DG(VTSS_TRACE_GROUP_MPLS, "MPLS IS1 (PW) cfg for evc_id %u: XC %d, seg %d, state %s, PW ISDX %d -> SDX %d",
                        ece->evc_id, evc->pw_ingress_xc, seg_idx,
                        srvl_mpls_segment_state_to_str(state),
                        xc->isdx->sdx, isdx->sdx);
            }
        }
    }
}

void vtss_mpls_ece_es0_update(vtss_state_t *vtss_state,
                              vtss_evc_entry_t *evc, vtss_ece_entry_t *ece,
                              vtss_sdx_entry_t *esdx, vtss_es0_action_t *action)
{
    VTSS_DG(VTSS_TRACE_GROUP_MPLS, "MPLS egress XC %d, PB VID %d, IVID %d",
            evc->pw_egress_xc, evc->vid, evc->ivid);

    if (VTSS_MPLS_IDX_IS_DEF(evc->pw_egress_xc)) {
        const vtss_mpls_segment_idx_t seg_idx = XC_P(evc->pw_egress_xc).out_seg_idx;

        if (VTSS_MPLS_IDX_IS_DEF(seg_idx)) {
            vtss_mpls_segment_state_t    state;
            vtss_mpls_segment_internal_t *seg    = &SEG_I(seg_idx);
            vtss_port_no_t               l2_port = srvl_mpls_port_no_get(vtss_state, seg_idx);

            (void) srvl_mpls_segment_state_get(vtss_state, seg_idx, &state);

            if (state == VTSS_MPLS_SEGMENT_STATE_UP  &&  l2_port == esdx->port_no) {
                action->mpls_encap_idx = seg->u.out.encap_idx;
                action->mpls_encap_len = srvl_bytes_to_encap_len(seg->u.out.encap_bytes);
                VTSS_DG(VTSS_TRACE_GROUP_MPLS, "MPLS encap cfg for evc_id %u: XC %d, seg %d, state %s, encap %d/%d",
                        ece->evc_id, evc->pw_egress_xc, seg_idx,
                        srvl_mpls_segment_state_to_str(state),
                        action->mpls_encap_idx, action->mpls_encap_len);
            }
        }
    }
}

vtss_rc vtss_srvl_evc_mpls_update(vtss_evc_id_t evc_id, vtss_res_t *res, vtss_res_cmd_t cmd)
{
    if (cmd == VTSS_RES_CMD_CALC) {
        VTSS_RC(srvl_mpls_xc_evc_attach_calc(evc_id, res));
    } else if (cmd == VTSS_RES_CMD_DEL) {
        VTSS_RC(srvl_mpls_xc_evc_detach(evc_id));
    } else if (cmd == VTSS_RES_CMD_ADD) {
        VTSS_RC(srvl_mpls_xc_evc_attach(evc_id));
    }
    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

static vtss_rc srvl_debug_mpls(vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t   *const info)
{
    // vtss_debug_print_header(pr, "MPLS Core");
    VTSS_RC(vtss_srvl_debug_is0_all(vtss_state, pr, info));
    VTSS_RC(vtss_srvl_debug_es0_all(vtss_state, pr, info));
    return VTSS_RC_OK;
}

static vtss_rc srvl_debug_mpls_oam(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info)
{
    vtss_debug_print_header(pr, "MPLS OAM");
    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_mpls_debug_print(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_MPLS, srvl_debug_mpls, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_MPLS_OAM, srvl_debug_mpls_oam, vtss_state, pr, info));
    return VTSS_RC_OK;
}

/* - Initialization ------------------------------------------------ */

static vtss_rc srvl_mpls_init(vtss_state_t *vtss_state)
{
    u8 map, qos, tc;

    (void) srvl_mpls_vprofile_init(vtss_state);

    // TC mapping tables. All start out as identity mappings.
    
    for (map = 0; map < VTSS_MPLS_QOS_TO_TC_MAP_CNT; map++) {
        for (qos = 0; qos < VTSS_MPLS_QOS_TO_TC_ENTRY_CNT; qos++) {
            vtss_state->mpls.tc_conf.qos_to_tc_map[map][qos].dp0_tc = qos;
            vtss_state->mpls.tc_conf.qos_to_tc_map[map][qos].dp1_tc = qos;
        }
    }
    
    for (map = 0; map < VTSS_MPLS_TC_TO_QOS_MAP_CNT; map++) {
        for (tc = 0; tc < VTSS_MPLS_TC_TO_QOS_ENTRY_CNT; tc++) {
            vtss_state->mpls.tc_conf.tc_to_qos_map[map][tc].qos = tc;
            vtss_state->mpls.tc_conf.tc_to_qos_map[map][tc].dp = FALSE;
        }
    }
    
    (void) srvl_mpls_tc_conf_set(vtss_state, &vtss_state->mpls.tc_conf);
    
    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_mpls_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_mpls_state_t *state = &vtss_state->mpls;
    
    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->tc_conf_set           = srvl_mpls_tc_conf_set;
        state->l2_alloc              = srvl_mpls_l2_alloc;
        state->l2_free               = srvl_mpls_l2_free;
        state->l2_set                = srvl_mpls_l2_set;
        state->l2_segment_attach     = srvl_mpls_l2_segment_attach;
        state->l2_segment_detach     = srvl_mpls_l2_segment_detach;
        state->xc_alloc              = srvl_mpls_xc_alloc;
        state->xc_free               = srvl_mpls_xc_free;
        state->xc_set                = srvl_mpls_xc_set;
        state->xc_segment_attach     = srvl_mpls_xc_segment_attach;
        state->xc_segment_detach     = srvl_mpls_xc_segment_detach;
        state->xc_mc_segment_attach  = srvl_mpls_xc_mc_segment_attach;
        state->xc_mc_segment_detach  = srvl_mpls_xc_mc_segment_detach;
        state->segment_alloc         = srvl_mpls_segment_alloc;
        state->segment_free          = srvl_mpls_segment_free;
        state->segment_set           = srvl_mpls_segment_set;
        state->segment_state_get     = srvl_mpls_segment_state_get;
        state->segment_server_attach = srvl_mpls_segment_server_attach;
        state->segment_server_detach = srvl_mpls_segment_server_detach;
        break;
    case VTSS_INIT_CMD_INIT:
        VTSS_RC(srvl_mpls_init(vtss_state));
        break;
    default:
        break;
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_MPLS */

#endif /* VTSS_ARCH_SERVAL */
