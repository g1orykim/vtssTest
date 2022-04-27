/*

 Vitesse Switch Software.

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

*/
/*****************************************************************************/
//
// vtss_fdma_common.c
//
//*****************************************************************************/

// Avoid "vtss_api.h not used in module vtss_fdma.c"
/*lint --e{766} */

#define VTSS_TRACE_LAYER VTSS_TRACE_LAYER_AIL

#include "vtss_api.h"
#include "vtss_state.h"
#include "vtss_common.h"
#include "vtss_fdma_common.h"

#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// vtss_fdma_cmn_xtr_split_frame_data()
// Split the frame data part into the three parts it consists of:
// 1) vtss_packet_rx_info_t, that is, the decoded extraction header,
// 2) worstcase injection/extraction header size,
// 3) real frame data.
//
// The vtss_packet_rx_info_t by itself must take a multiple of the cache-line
// size, because it's intermingled with frame data, which also must be a
// multiple of the cache-line size in order to be able to invalidate the
// cache used to hold the extracted frame without affecting the neigboring
// data.
/*****************************************************************************/
void vtss_fdma_cmn_xtr_split_frame_data(fdma_sw_dcb_t *sw_dcb)
{
    FDMA_USER_DCB(sw_dcb)->rx_info = (vtss_packet_rx_info_t *)VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE(FDMA_USER_DCB(sw_dcb)->alloc_ptr);
    sw_dcb->data                   = (u8 *)VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE((u8 *)FDMA_USER_DCB(sw_dcb)->rx_info + VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE(sizeof(*FDMA_USER_DCB(sw_dcb)->rx_info)));
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */


/*****************************************************************************/
// vtss_fdma_cmn_fcs_get()
/*****************************************************************************/
u32 vtss_fdma_cmn_fcs_get(const fdma_sw_dcb_t *dcb)
{
    fdma_sw_dcb_t    *sof = (fdma_sw_dcb_t *)dcb; // Get rid of const, which is only here due to JR's xtr_hdr_decode() function.
    vtss_fdma_list_t *user_dcb, *next_to_last_user_dcb;
    u32              fcs = 0, fcs_start_pos, fcs_bytes_in_dcb, i;

    // Get the FCS, since it is overwritten by the switch core, and thus contains additional info.
    user_dcb = FDMA_USER_DCB(sof);
    while (user_dcb->next) {
        user_dcb = user_dcb->next;
    }

    if (user_dcb->act_len < 4) {
        // The FCS is split into two DCBs (due to size constraints, it cannot be split across more than two DCBs).
        // First get the LSBytes from the last DCB.
        for (i = 0; i < (u32)user_dcb->act_len; i++) {
            fcs |= (u32)user_dcb->frm_ptr[i] << (8 * (user_dcb->act_len - 1 - i));
        }

        // Then find the next-to-last DCB to prepare for the loop outside this if statement.
        next_to_last_user_dcb = FDMA_USER_DCB(sof);
        while (next_to_last_user_dcb->next != user_dcb) {
            next_to_last_user_dcb = next_to_last_user_dcb->next;
        }

        // Prepare for the loop below where the MSBytes of the FCS is ORed on.
        fcs_bytes_in_dcb = 4 - user_dcb->act_len;
        user_dcb         = next_to_last_user_dcb;
    } else {
        fcs_bytes_in_dcb = 4;
    }

    fcs_start_pos = user_dcb->act_len - (user_dcb->frm_ptr - FDMA_SW_DCB(user_dcb)->ifh_ptr) - fcs_bytes_in_dcb;

    for (i = 0; i < fcs_bytes_in_dcb; i++) {
        fcs |= (u32)user_dcb->frm_ptr[fcs_start_pos + i] << (8 * (3 - i));
    }

    return fcs;
}

/****************************************************************************/
// vtss_fdma_cmn_insert_vlan_tag()
/****************************************************************************/
vtss_rc vtss_fdma_cmn_insert_vlan_tag(fdma_sw_dcb_t *sw_dcb, vtss_vlan_tag_t *tag, BOOL move_ifh, BOOL thread_safe)
{
    vtss_trace_group_t trc_grp  = thread_safe ? VTSS_TRACE_GROUP_FDMA_IRQ : VTSS_TRACE_GROUP_FDMA_NORMAL;
    int                i, cnt = SIZE_OF_TWO_MAC_ADDRS;
    u8                 *dst, *src = move_ifh ? sw_dcb->ifh_ptr : FDMA_USER_DCB(sw_dcb)->frm_ptr;
    u16                tci;

    if (src - sw_dcb->data < 4) {
        VTSS_EG(trc_grp, "Internal error. Not room for VLAN tag (move_ifh = %d)", move_ifh);
        return VTSS_RC_ERROR;
    }

    if (move_ifh) {
        cnt += (FDMA_USER_DCB(sw_dcb)->frm_ptr - sw_dcb->ifh_ptr);
    }

    dst = src - 4;

    // Move MAC addresses (and possibly IFH) 4 bytes ahead.
    for (i = 0; i < cnt; i++) {
        *(dst++) = *(src++);
    }

    tci = (tag->pcp << 13) | (tag->dei << 12) | tag->vid;

    *(dst++) = tag->tpid >> 8;
    *(dst++) = tag->tpid >> 0;
    *(dst++) = tci       >> 8;
    *(dst++) = tci       >> 0;

    FDMA_USER_DCB(sw_dcb)->act_len += 4;
    FDMA_USER_DCB(sw_dcb)->frm_ptr -= 4;
    sw_dcb->ifh_ptr                -= 4;

    return VTSS_RC_OK;
}

/*****************************************************************************/
// inj_pre_tx()
// Check to see if the user has installed a "just-before-handing-over-the-
// DCBs-to-the-FDMA-H/W-handler" callback, and if so, give him a chance to
// do a last-minute change of the data.
/*****************************************************************************/
#if VTSS_OPT_FDMA_VER <= 2
static void inj_pre_tx(fdma_sw_dcb_t *sw_dcb, const vtss_fdma_inj_props_t *const props)
#else
static void inj_pre_tx(fdma_sw_dcb_t *sw_dcb, const vtss_fdma_tx_info_t   *const props)
#endif /* VTSS_OPT_FDMA_VER */
{
    u8 *ptr;

    if (props->pre_cb != NULL) {
        props->pre_cb(props->pre_cb_ctxt1, props->pre_cb_ctxt2, FDMA_USER_DCB(sw_dcb));
        VTSS_OS_REORDER_BARRIER();

#if VTSS_OPT_FDMA_VER <= 2
        // In FDMA API v2-, act_len includes size of IFH, frame, and FCS.
        ptr = sw_dcb->ifh_ptr;
#else
        // In FDMA API v3+, act_len is the raw size of the frame.
        ptr = FDMA_USER_DCB(sw_dcb)->frm_ptr;
#endif /* VTSS_OPT_FDMA_VER */

        // Flush the dcache, so that the modified frame data gets written to main memory.
        VTSS_OS_DCACHE_FLUSH(ptr, FDMA_USER_DCB(sw_dcb)->act_len);
    }
}

/*****************************************************************************/
// is_inj_ch()
/*****************************************************************************/
static BOOL is_inj_ch(vtss_fdma_ch_usage_t usage)
{
    return (usage == VTSS_FDMA_CH_USAGE_INJ
#if defined(VTSS_FEATURE_AFI_FDMA)
            || usage == VTSS_FDMA_CH_USAGE_CCM
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
    );
}

/*****************************************************************************/
// vtss_fdma_cmn_inj_attach_frm()
/*****************************************************************************/
#if VTSS_OPT_FDMA_VER <= 2
vtss_rc vtss_fdma_cmn_inj_attach_frm(vtss_state_t *const vstate, fdma_sw_dcb_t *head, fdma_sw_dcb_t *tail, vtss_fdma_ch_t inj_ch, u32 dcb_cnt, u32 len, const vtss_fdma_inj_props_t *const props, BOOL thread_safe)
#else
vtss_rc vtss_fdma_cmn_inj_attach_frm(vtss_state_t *const vstate, fdma_sw_dcb_t *head, fdma_sw_dcb_t *tail, vtss_fdma_ch_t inj_ch, u32 dcb_cnt, u32 len, const vtss_fdma_tx_info_t   *const props, BOOL thread_safe)
#endif /* VTSS_OPT_FDMA_VER */
{
    fdma_state_t    *state    = &vstate->fdma_state;
    fdma_ch_state_t *ch_state = &state->fdma_ch_state[inj_ch];
    vtss_rc         result    = VTSS_RC_OK;
    BOOL            start_ch;
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS __attribute__((__unused__)) flags = 0;

    // It must be an injection channel
    FDMA_CHECK_RC(thread_safe, is_inj_ch(ch_state->usage));

    // Add the frame to the end of the current list.
    // Since both the interrupt handler and this function can alter the current
    // list, we need to safe-guard it by disabling interrupts while manipulating
    // the pointers.
    // We get one transfer done interrupt for every frame that has been injected,
    // and the channel stops. The interrupt handler is taking care of restarting
    // the channel if there are more frames to go. If - on the other hand - the
    // interrupt handler is not currently active, we should start it.
    /*lint --e{454, 456} */ // Two execution paths are being combined with different mutex lock states
    if (!thread_safe) {
        FDMA_INTERRUPT_DISABLE(flags);
    }

    // Safe to update now.
#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
    state->fdma_stats.tx_bufs_used[inj_ch] += dcb_cnt;
    state->fdma_stats.tx_bytes[inj_ch]     += len; // This should've been counted in the IRQ handler, but in that function we would have to re-compute the total length, which takes time!
#else
    // Safe to update now.
    state->fdma_stats.dcbs_used[inj_ch] += dcb_cnt;
    state->fdma_stats.bytes[inj_ch]     += len; // This should've been counted in the IRQ handler, but in that function we would have to re-compute the total length, which takes time!
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER */

    if (ch_state->cur_tail == NULL) {
        // The DMA channel is non-active, but we can't re-allow interrupts now, because
        // on Jaguar, received frames subject to learning (learn-all) may use the channel.
        // On all platforms that can't inject on a channel in the IRQ handler, we could
        // re-enable interrupts now, but for simplicity, we  wait until we're done.

        VTSS_DG(VTSS_TRACE_GROUP_FDMA_IRQ, "inj_attach_frm(ch=%d, l=%d). Start immediately\n", inj_ch, len);

        // The DMA channel is inactive.
        FDMA_CHECK(TRUE, ch_state->cur_head == NULL, result = VTSS_RC_ERROR; goto do_exit;);

        ch_state->cur_head = head;
        ch_state->cur_tail = tail;

        // Perform a possible pre-tx callback.
        // The pre-tx callback must be called as close as possible to the actual
        // enabling of the channel, but we cannot do it after the suspended check,
        // since that would require us to callback when the channel gets resumed,
        // which is prohibitive, since we don't have the callback information
        // at that time.
        if (props) {
            inj_pre_tx(head, props);
        }

        start_ch = TRUE;
    } else {
        VTSS_DG(VTSS_TRACE_GROUP_FDMA_IRQ, "inj_attach_frm(ch=%d, l=%d). Append to existing\n", inj_ch, len);

        // It doesn't matter whether the DMA channel is running right now or not,
        // since we can't get into the interrupt handler, and since the tail's
        // DCB->llp is NULL, which means that the DMA controller will
        // definitely stop before it starts consuming the frame we're trying to add.

        // When the tail is non-NULL, so must the head be.
        FDMA_CHECK(TRUE, ch_state->cur_head,                              result = VTSS_RC_INV_STATE; goto do_exit;);
        FDMA_CHECK(TRUE, FDMA_USER_DCB(ch_state->cur_tail)->next == NULL, result = VTSS_RC_INV_STATE; goto do_exit;);

        if (ch_state->cur_head == NULL || FDMA_USER_DCB(ch_state->cur_tail)->next != NULL) {
            result = VTSS_RC_INV_STATE;
            goto do_exit;
        }

        // Perform a possible pre-tx callback. This must be done before adding the packet to the active list,
        // since the FDMA may be running, and we don't want it to inject this frame until the user
        // has had a chance to modify it.
        if (props) {
            inj_pre_tx(head, props);
        }

        FDMA_USER_DCB(ch_state->cur_tail)->next = FDMA_USER_DCB(head);
        ch_state->cur_tail = tail;

        start_ch = FALSE;
    }

    result = vstate->fdma_state.fdma_func.fdma_inj_restart_ch(vstate, inj_ch, head, start_ch, TRUE);

do_exit:
    if (!thread_safe) {
        FDMA_INTERRUPT_RESTORE(flags);
    }

    return result;
}

#if !defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER >= 3
/****************************************************************************/
// vtss_fdma_cmn_strip_vlan_tag()
/****************************************************************************/
static vtss_rc vtss_fdma_cmn_strip_vlan_tag(fdma_sw_dcb_t *sw_dcb, BOOL move_ifh, vtss_vlan_tag_t *stripped_tag)
{
    int i, cnt = SIZE_OF_TWO_MAC_ADDRS;
    u8  *dst, *src = FDMA_USER_DCB(sw_dcb)->frm_ptr + SIZE_OF_TWO_MAC_ADDRS - 1;

    if (move_ifh) {
        cnt += (FDMA_USER_DCB(sw_dcb)->frm_ptr - sw_dcb->ifh_ptr);
    }

    // #src points to one byte ahead of the VLAN tag, hence the 1-based indexing.
    stripped_tag->tpid = ((u16)src[1] << 8) | ((u16)src[2]);
    stripped_tag->pcp  = (src[3] >> 5) & 0x7;
    stripped_tag->dei  = (src[3] >> 4) & 0x1;
    stripped_tag->vid  = (((u16)src[3] & 0xF) << 8) | ((u16)src[4]);

    dst = src + 4;

    // Move MAC addresses (and possibly IFH) 4 bytes.
    for (i = 0; i < cnt; i++) {
        *(dst--) = *(src--);
    }

    FDMA_USER_DCB(sw_dcb)->act_len -= 4;
    FDMA_USER_DCB(sw_dcb)->frm_ptr += 4;
    sw_dcb->ifh_ptr                += 4;

    return VTSS_RC_OK;
}
#endif /* !defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER >= 3 */

/****************************************************************************/
// vtss_fdma_cmn_dcb_mem_size()
// Given #dcb_cnt DCBs, how much memory is required, taking into account that
// a DCB must be cache-aligned?
/****************************************************************************/
u32 vtss_fdma_cmn_dcb_mem_size(u32 dcb_cnt)
{
    u32 size_of_one_dcb = VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE(sizeof(fdma_sw_dcb_t));
    return (size_of_one_dcb * dcb_cnt) + VTSS_OS_DCACHE_LINE_SIZE_BYTES;
}

/****************************************************************************/
// vtss_fdma_cmn_dcb_and_frm_alloc()
// Allocate #dcb_cnt DCBs from #mem_type type of memory.
// The function may also be used to allocate accompanying frame data, so
// that only one single chunk of memory is allocated for DCBs and frame data.
// It's up to the caller to then sticth the DCBs and frame data together.
//
//   dcb_cnt        [IN]  Number of DCBs to allocate
//   frm_bytes      [IN]  Additional bytes to allocate, e.g. for frame data.
//   mem_type       [IN]  Instructs the OS to allocate from this type of memory.
//                        Is a combination of the flags from the vtss_mem_flags_t type.
//   dcb_ptr        [OUT] Pointer to the first DCB, which is guaranteed to be cache aligned.
//   frm_ptr        [OUT] Pointer to additional memory. Only needs be non-NULL if frm_bytes is > 0.
//   end_of_mem_ptr [OUT] Pointer to one byte beyond last allocated (may be used for verification of sub-allocation by caller). May be NULL.
//   thread_safe    [IN]  Used to figure out where to output trace. If TRUE, the function
//                        is assumed to be called from within a DSR/ISR, and therefore
//                        can't wait for a mutex or semaphore.
//
// Returns a pointer to be used in call to VTSS_OS_FREE() or NULL if out-of-memory.
// All allocated memory will be zeroed out.
/****************************************************************************/
void *vtss_fdma_cmn_dcb_and_frm_alloc(u32 dcb_cnt, u32 frm_bytes, u32 mem_type, fdma_sw_dcb_t **sw_dcb_ptr, void **frm_ptr, void **end_of_mem_ptr, BOOL thread_safe)
{
    vtss_trace_group_t trc_grp  = thread_safe ? VTSS_TRACE_GROUP_FDMA_IRQ : VTSS_TRACE_GROUP_FDMA_NORMAL;
    size_t             dcb_sz   = vtss_fdma_cmn_dcb_mem_size(dcb_cnt);
    size_t             alloc_sz = dcb_sz + frm_bytes;
    void               *mem_ptr;

    VTSS_IG(trc_grp, "Allocating %zu bytes", alloc_sz);
    mem_ptr = VTSS_OS_MALLOC(alloc_sz, mem_type);
    if (mem_ptr == NULL) {
        VTSS_EG(trc_grp, "Failed to allocate %zu bytes", alloc_sz);
        return NULL;
    }
    memset(mem_ptr, 0, alloc_sz);
    *sw_dcb_ptr = (fdma_sw_dcb_t *)VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE((u32)mem_ptr);
    if (frm_ptr) {
        if (frm_bytes) {
            *frm_ptr = (u8 *)mem_ptr + dcb_sz;
        } else {
            *frm_ptr = NULL;
        }
    }
    if (end_of_mem_ptr) {
        *end_of_mem_ptr = (u8 *)mem_ptr + alloc_sz;
    }
    return mem_ptr;
}

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// vtss_fdma_cmn_xtr_hdr_decode()
/*****************************************************************************/
vtss_rc vtss_fdma_cmn_xtr_hdr_decode(vtss_state_t *vstate, fdma_sw_dcb_t *sof, vtss_chip_no_t chip_no, vtss_packet_rx_queue_t xtr_qu, VTSS_OS_TIMESTAMP_TYPE *sw_tstamp)
{
    vtss_packet_rx_meta_t meta;
    vtss_fdma_cfg_t       *cfg = &vstate->fdma_state.fdma_cfg;
    vtss_packet_rx_info_t *rx_info = FDMA_USER_DCB(sof)->rx_info;
    vtss_fdma_list_t      *user_dcb;
    const u8              *etype_ptr;

    if (cfg->rx_allow_multiple_dcbs == FALSE) {

        // Application doesn't want frames spanning multiple DCBs.
        if (FDMA_USER_DCB(sof)->next != NULL) {
            // But it is.
            vstate->fdma_state.fdma_stats.rx_multiple_dcb_drops++;
            return VTSS_RC_ERROR;
        }

        // We have a bit of slack in each buffer, so it's possible
        // that we do receive frames with an MTU greater than what
        // the application wants. The reason for this slack is
        // that frames may arrive with e.g. a VStaX header, which is
        // moved to the IFH and shouldn't count in the MTU as seen
        // from the application's point of view.
        //
        // The MTU check is disabled if the application really *can*
        // handle frames spanning multiple DCBs, because the #rx_mtu
        // parameter then changes its meaning slightly. Therefore,
        // we only check for this on single-DCB applications.
        if ((FDMA_USER_DCB(sof)->act_len - (FDMA_USER_DCB(sof)->frm_ptr - sof->ifh_ptr)) > cfg->rx_mtu) {
            vstate->fdma_state.fdma_stats.rx_jumbo_drops++;
            return VTSS_RC_ERROR;
        }
    }

    // Get info from IFH
    memset(&meta, 0, sizeof(meta));
    meta.no_wait = TRUE; // Trace goes to VTSS_TRACE_GROUP_FDMA_IRQ
    meta.sw_tstamp = *sw_tstamp;
    etype_ptr = sof->ifh_ptr + vstate->packet.rx_ifh_size + 2 * 6 /* 2 MAC addresses */;
    meta.etype = (etype_ptr[0] << 8) | etype_ptr[1];

#if defined(VTSS_ARCH_JAGUAR_1)
    meta.chip_no = chip_no;
    meta.xtr_qu  = xtr_qu;
    meta.fcs = vtss_fdma_cmn_fcs_get(sof);
#endif /* VTSS_ARCH_JAGUAR_1 */

    VTSS_RC(vstate->packet.rx_hdr_decode(vstate, &meta, sof->ifh_ptr, rx_info));

    // Check if the application wants packets with VLAN tag mismatch.
    if (cfg->rx_allow_vlan_tag_mismatch == FALSE && (rx_info->hints & VTSS_PACKET_RX_HINTS_VLAN_TAG_MISMATCH)) {
        vstate->fdma_state.fdma_stats.rx_vlan_tag_mismatch++;
        return VTSS_RC_ERROR;
    }

#if defined(VTSS_ARCH_LUTON28)
    // Luton28 strips both C- and S-tags on VLAN aware ports, so
    // to provide a uniform interface towards the application, we can
    // re-insert a tag upon vlan mismatch.
    if (cfg->rx_dont_reinsert_vlan_tag == FALSE && (rx_info->hints & VTSS_PACKET_RX_HINTS_VLAN_TAG_MISMATCH)) {
        VTSS_RC(vtss_fdma_cmn_insert_vlan_tag(sof, &rx_info->tag, TRUE, TRUE));
    }
#else
    // Other platforms don't remove any tags from the payload, so in order to provide
    // a uniform interface towards the application, we can do it for it, if it has configured
    // us to do so.
    if (rx_info->tag_type != VTSS_TAG_TYPE_UNTAGGED && cfg->rx_dont_strip_vlan_tag == FALSE) {
#if defined(VTSS_ARCH_JAGUAR_1) && VTSS_SWITCH_STACKABLE
        // In normal scenarios, Jaguar doesn't remove tags from the payload, but in case the frame
        // was received with a VStaX header (on a stack port), any tag that the frame origally
        // arrived with is now stripped (on egress of the first switch), so in that case, there's
        // no VLAN tag to remove.
        if (!rx_info->vstax.valid)
#endif /* defined(VTSS_ARCH_JAGUAR_1) && VTSS_SWITCH_STACKABLE */
        {
            VTSS_RC(vtss_fdma_cmn_strip_vlan_tag(sof, TRUE, &rx_info->stripped_tag));
        }
    }
#endif /* VTSS_ARCH_LUTON28 */

    // Find the length of the frame (this could have gone into the meta structure above, but
    // since we potentially have inserted or stripped a VLAN tag, we do it here.
    user_dcb = FDMA_USER_DCB(sof);
    while (user_dcb) {
        // += is OK, since we've memset() rx_info() within rx_hdr_decode()
        rx_info->length += (user_dcb->act_len - (user_dcb->frm_ptr - FDMA_SW_DCB(user_dcb)->ifh_ptr));
        user_dcb = user_dcb->next;
    }
    rx_info->length -= 4; /* FCS */

    if (rx_info->length < 40) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_IRQ, "Length < 40 (%u)", rx_info->length);
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

/*****************************************************************************/
// vtss_fdma_cmn_throttle_suspend_check()
/*****************************************************************************/
void vtss_fdma_cmn_throttle_suspend_check(vtss_state_t *vstate, vtss_packet_rx_queue_t xtr_qu, u32 bytes)
{
    fdma_throttle_state_t *throttle = &vstate->fdma_state.throttle;
    BOOL suspend_due_to_frm_cnt, suspend_due_to_byte_cnt;

    throttle->frm_cnt[xtr_qu]++;
    throttle->byte_cnt[xtr_qu] += bytes;

#if VTSS_OPT_FDMA_VER <= 2
    suspend_due_to_byte_cnt = FALSE;
    suspend_due_to_frm_cnt  = throttle->cfg.limit_per_tick[xtr_qu] > 0 && throttle->frm_cnt[xtr_qu] > throttle->cfg.limit_per_tick[xtr_qu];
#else
    suspend_due_to_byte_cnt = throttle->cfg.byte_limit_per_tick[xtr_qu] > 0 && throttle->byte_cnt[xtr_qu] > throttle->cfg.byte_limit_per_tick[xtr_qu];
    suspend_due_to_frm_cnt  = throttle->cfg.frm_limit_per_tick[xtr_qu]  > 0 && throttle->frm_cnt[xtr_qu]  > throttle->cfg.frm_limit_per_tick[xtr_qu];
#endif /* VTSS_OPT_FDMA_VER */

    if ((suspend_due_to_frm_cnt || suspend_due_to_byte_cnt) &&
        throttle->tick_cnt > 1                              && // Started?
        throttle->ticks_left[xtr_qu] == 0) {                // Not already suspended?
        // Suspend the queue
        vstate->fdma_state.fdma_func.fdma_xtr_qu_suspend_set(vstate, xtr_qu, TRUE);
        // And let vtss_fdma_throttle_tick() re-enable the queue once the suspension period elapses.
        // We add one to the requested suspend_tick_cnt because we are anywhere in between
        // two throttle ticks, and the suspension period starts upon the next throttle tick.
        throttle->ticks_left[xtr_qu] = throttle->cfg.suspend_tick_cnt[xtr_qu] + 1;
        // Count the event.
        throttle->suspend_cnt[xtr_qu]++;
    }
}

/*****************************************************************************/
// vtss_fdma_cmn_xtr_restart_ch()
// Must be called thread safe.
/*****************************************************************************/
void vtss_fdma_cmn_xtr_restart_ch(vtss_state_t *const vstate, vtss_fdma_ch_t xtr_ch)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[xtr_ch];

    // For the sake of Serval, we cannot test whether ch_state->cur_head is NULL
    // here, because it may be non-NULL at the same time as the H/W indicates
    // that the channel's DCBs have depleted. Instead, we just call the
    // CIL function and checks its return value, and connect the new head
    // to the end of cur_head.
    if (ch_state->free_head != NULL) {
        // Potentially restart the channel. If this function returns TRUE,
        // the H/W has indeed been restarted.
        if (vstate->fdma_state.fdma_func.fdma_xtr_restart_ch(vstate, xtr_ch)) {
            // Update S/W pointers.
            if (ch_state->cur_head) {
                // Should only occur on Serval.
                // Connect the free_head to the end of the cur_head (which may or may not be NULL).
                // We only need to connect the two lists S/W-wise - not H/W-wise.
                vtss_fdma_list_t *temp = FDMA_USER_DCB(ch_state->cur_head);
                while (temp->next) {
                    temp = temp->next;
                }
                temp->next = FDMA_USER_DCB(ch_state->free_head);
            } else {
                ch_state->cur_head = ch_state->free_head;
            }
            ch_state->free_head = ch_state->cur_tail = NULL;
        }
    }
}

/*****************************************************************************/
// xtr_dcb_init()
/*****************************************************************************/
static vtss_rc xtr_dcb_init(vtss_state_t *const vstate, vtss_fdma_ch_t xtr_ch, fdma_sw_dcb_t *const head, BOOL thread_safe, BOOL connect, BOOL restart)
{
    fdma_ch_state_t  *ch_state = &vstate->fdma_state.fdma_ch_state[xtr_ch];
    fdma_sw_dcb_t    *sw_dcb = head, *new_tail = head;
    vtss_fdma_list_t *user_dcb;
    u32              offset_into_data;
    u32              cfg_alloc_len, burst_size_bytes = vstate->fdma_state.xtr_burst_size_bytes, dcb_cnt = 0;
    vtss_rc          rc = VTSS_RC_OK;

    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS __attribute__((__unused__)) flags = 0;

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER < 2
    offset_into_data = 0;
#else
    offset_into_data = vstate->fdma_state.hdr_size - vstate->fdma_state.xtr_hdr_size;
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER */

    while (sw_dcb) {
        user_dcb = FDMA_USER_DCB(sw_dcb);

        // Align the allocation length that we configure into the FDMA to be the closest lower multiple of the burst size.
        // Also, take into account that the allocated header size (VTSS_FDMA_HDR_SIZE_BYTES) might be greater than the actual
        // extraction header size that the GPDMA inserts (VTSS_FDMA_XTR_HDR_SIZE_BYTES).
        cfg_alloc_len = ((sw_dcb->alloc_len - offset_into_data) / burst_size_bytes) * burst_size_bytes;

        // The actual position into which the GPDMA must inject data must be adjusted for mismatch between
        // allocated header size (VTSS_FDMA_HDR_SIZE_BYTES) and the header size that the GPDMA inserts (VTSS_FDMA_XTR_HDR_SIZE_BYTES).
        sw_dcb->ifh_ptr = sw_dcb->data + offset_into_data;

        FDMA_CHECK_RC(thread_safe, ((u32)FDMA_HW_DCB(sw_dcb) & (VTSS_OS_DCACHE_LINE_SIZE_BYTES - 1)) == 0);
        FDMA_CHECK_RC(thread_safe, sw_dcb->data && ((u32)sw_dcb->ifh_ptr & 0x3) == 0);
        FDMA_CHECK_RC(thread_safe, cfg_alloc_len >= VTSS_FDMA_MIN_DATA_PER_XTR_SOF_DCB_BYTES && cfg_alloc_len <= VTSS_FDMA_MAX_DATA_PER_DCB_BYTES);

        // Furthermore, the allocated length (at least when reported to the GPDMA) must be a multiple of the source burst size.

        // If the user has modified the buffer, we need to invalidate the cache
        // before the FDMA has actually written data to the buffer because
        // part of the act of invalidating the cache is to drain the write buffer,
        // and if we waited with invalidating the cache until after the FDMA has
        // filled data into it, the FDMA-filled data would be overwritten with
        // the stale data in the write buffer.
        VTSS_OS_DCACHE_INVALIDATE(sw_dcb->ifh_ptr, cfg_alloc_len);

        // Ask H/W-specific driver to initialize its H/W DCB area.
        VTSS_RC(vstate->fdma_state.fdma_func.fdma_xtr_dcb_init(vstate, xtr_ch, sw_dcb, cfg_alloc_len));

        // Really not needed, but easier to figure out which DCBs are used and which are not during debugging.
        user_dcb->act_len = 0;

        dcb_cnt++;

        if (user_dcb->next) {
            new_tail = FDMA_SW_DCB(user_dcb->next);
        }
        sw_dcb = FDMA_SW_DCB(user_dcb->next);
    }

    // While manipulating the free list, we cannot allow interrupts to occur,
    // since they may also manipulate the list.
    /*lint --e{454, 456} */ // Two execution paths are being combined with different mutex lock states
    if (!thread_safe) {
        FDMA_INTERRUPT_DISABLE(flags);
    }

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
    vstate->fdma_state.fdma_stats.rx_bufs_added[ch_state->phys_qu] += dcb_cnt;
#else
    vstate->fdma_state.fdma_stats.dcbs_added[xtr_ch] += dcb_cnt;
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER */

    if (connect) {
        if (ch_state->chip_no == 1) {
            // Since we use manual extraction for secondary chip, we connect directly to cur_head, since
            // it's not going to be committed to H/W (the FDMA).
            // Put the new head in the front.
            FDMA_USER_DCB(new_tail)->next = FDMA_USER_DCB(ch_state->cur_head);
            ch_state->cur_head = head;
        } else {
            // Connect the new head to the end of the current free list whose
            // start and end are given by ch_state->free_head and ch_state->cur_tail.
            if (ch_state->free_head) {
                if (ch_state->cur_tail == NULL || FDMA_USER_DCB(ch_state->cur_tail)->next != NULL) {
                    VTSS_EG(VTSS_TRACE_GROUP_FDMA_IRQ, "Assert failed");
                    rc = VTSS_RC_ERROR;
                    goto do_exit;
                }
                // The free list already exists. Append to it, S/W-wise...
                FDMA_USER_DCB(ch_state->cur_tail)->next = FDMA_USER_DCB(head);
                // ...and H/W-wise:
                vstate->fdma_state.fdma_func.fdma_xtr_connect(vstate, FDMA_HW_DCB(ch_state->cur_tail), FDMA_HW_DCB(head));
            } else {
                // The free list doesn't exist. Create it.
                ch_state->free_head = head;
            }
            ch_state->cur_tail = new_tail;
        }
    }

    if (restart) {
        vtss_fdma_cmn_xtr_restart_ch(vstate, xtr_ch);
    }

do_exit:
    if (!thread_safe) {
        FDMA_INTERRUPT_RESTORE(flags);
    }
    return rc;
}

/*****************************************************************************/
// vtss_fdma_cmn_dcb_release()
/*****************************************************************************/
vtss_rc vtss_fdma_cmn_dcb_release(vtss_state_t *const vstate, vtss_fdma_ch_t ch, fdma_sw_dcb_t *const list, BOOL thread_safe, BOOL connect, BOOL restart)
{
    if (list == NULL) {
        // Simplifies CIL code to support empty lists.
        return VTSS_RC_OK;
    }

#if VTSS_OPT_FDMA_VER <= 2
    return xtr_dcb_init(vstate, ch, list, thread_safe, connect, restart);
#else
    fdma_state_t         *state;
    vtss_fdma_dcb_type_t sof_dcb_type;
    vtss_fdma_ch_t       sof_xtr_ch;
    void                 *sof_alloc_ptr;
    fdma_sw_dcb_t        *sw_dcb   = list, **state_dcb_list;
    vtss_fdma_list_t     *user_dcb = FDMA_USER_DCB(list), *tail_dcb;
    vtss_trace_group_t   trc_grp   = thread_safe ? VTSS_TRACE_GROUP_FDMA_IRQ : VTSS_TRACE_GROUP_FDMA_NORMAL;
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    /*lint --e{454, 456} */ // Two execution paths are being combined with different mutex lock states
    FDMA_INTERRUPT_FLAGS flags;

    state = &vstate->fdma_state;

    sof_dcb_type  = sw_dcb->dcb_type;
    sof_xtr_ch    = sw_dcb->xtr_ch;
    sof_alloc_ptr = user_dcb->alloc_ptr;

    // Check that all DCBs are of the same type and - if extraction DCBs - go to the same channel and have the same allocation method.
    sw_dcb = FDMA_SW_DCB(user_dcb->next);
    tail_dcb = user_dcb->next ? user_dcb->next : user_dcb;
    while (sw_dcb) {
        user_dcb = FDMA_USER_DCB(sw_dcb);
        if (sw_dcb->dcb_type != sof_dcb_type) {
            VTSS_EG(trc_grp, "Mixed list of DCB type not supported");
            return VTSS_RC_ERROR;
        }
        if (sof_dcb_type == VTSS_FDMA_DCB_TYPE_XTR) {
            if (sw_dcb->xtr_ch != sof_xtr_ch) {
                VTSS_EG(trc_grp, "Mixed list of extraction channels not supported");
                return VTSS_RC_ERROR;
            }
            if ((sof_alloc_ptr == NULL && user_dcb->alloc_ptr != NULL) ||
                (sof_alloc_ptr != NULL && user_dcb->alloc_ptr == NULL)) {
                VTSS_EG(trc_grp, "Mixed allocation schemes not supported");
                return VTSS_RC_ERROR;
            }
        }
        if (user_dcb->next) {
            tail_dcb = user_dcb->next;
        }
        sw_dcb = FDMA_SW_DCB(user_dcb->next);
    }

    sw_dcb = list;

    switch (sof_dcb_type) {
    case VTSS_FDMA_DCB_TYPE_XTR:
        // Check to see if we have to call back the application to get new frame data.
        if (sof_alloc_ptr == NULL) {

            // Gotta allocate new frame data.
            if (state->fdma_cfg.rx_alloc_cb == NULL) {
                VTSS_EG(trc_grp, "Got DCBs with NULL alloc_ptr, but no rx_alloc_cb() specified");
                return VTSS_RC_ERROR;
            }

            while (sw_dcb) {
                user_dcb = FDMA_USER_DCB(sw_dcb);
                state->fdma_cfg.rx_alloc_cb(state->bytes_per_frm, user_dcb, VTSS_MEM_FLAGS_DMA);
                if (user_dcb->alloc_ptr == NULL) {
                    VTSS_EG(trc_grp, "Application failed to allocate %u bytes of frame data", state->bytes_per_frm);
                    return VTSS_RC_ERROR;
                }
                vtss_fdma_cmn_xtr_split_frame_data(sw_dcb);
                sw_dcb = FDMA_SW_DCB(user_dcb->next);
            }
        }

        return xtr_dcb_init(vstate, sof_xtr_ch, list, thread_safe, connect, restart);

    case VTSS_FDMA_DCB_TYPE_INJ:
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    case VTSS_FDMA_DCB_TYPE_AFI:
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

        if (!thread_safe) {
            FDMA_INTERRUPT_DISABLE(flags);
        }

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
        if (sof_dcb_type == VTSS_FDMA_DCB_TYPE_AFI) {
            state_dcb_list = &vstate->fdma_state.afi_dcbs;
        } else
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
        {
            state_dcb_list = &vstate->fdma_state.inj_dcbs;
        }

        // Prepend the DCBs.
        tail_dcb->next  = FDMA_USER_DCB(*state_dcb_list);
        *state_dcb_list = list;

        if (!thread_safe) {
            FDMA_INTERRUPT_RESTORE(flags);
        }
        break;

    default:
        VTSS_EG(trc_grp, "Unknown DCB type (%d)", sw_dcb->dcb_type);
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
#endif /* VTSS_OPT_FDMA_VER */
}

/*****************************************************************************/
// vtss_fdma_cmn_debug_print_fdma()
/*****************************************************************************/
vtss_rc vtss_fdma_cmn_debug_print_fdma(vtss_state_t *const vstate, const vtss_debug_printf_t pr, const vtss_debug_info_t *const info)
{
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    fdma_throttle_state_t  *throttle = &vstate->fdma_state.throttle;
    vtss_packet_rx_queue_t xtr_qu;
    vtss_fdma_cfg_t        *cfg = &vstate->fdma_state.fdma_cfg;
    FDMA_INTERRUPT_FLAGS flags;

    FDMA_INTERRUPT_DISABLE(flags);
    // Print throttle statistics.
    pr("Throttle ticks: %" PRIu64 "\n", throttle->tick_cnt);
    pr("Xtr Qu Frame Limit/Tick Byte Limit/Tick Max Frames/Tick Max Bytes/Tick # Suspends Suspend Ticks Left\n");
    pr("------ ---------------- --------------- --------------- -------------- ---------- ------------------\n");
    for (xtr_qu = 0; xtr_qu < VTSS_PACKET_RX_QUEUE_CNT; xtr_qu++) {
#if VTSS_OPT_FDMA_VER <= 2
        u32 frm_limit  = throttle->cfg.limit_per_tick[xtr_qu];
        u32 byte_limit = 0;
#else
        u32 frm_limit  = throttle->cfg.frm_limit_per_tick[xtr_qu];
        u32 byte_limit = throttle->cfg.byte_limit_per_tick[xtr_qu];
#endif /* VTSS_OPT_FDMA_VER */
        pr("%6u %16u %15u %15u %14u %10u %18u\n", xtr_qu, frm_limit, byte_limit, throttle->statistics_max_frames_per_tick[xtr_qu], throttle->statistics_max_bytes_per_tick[xtr_qu], throttle->suspend_cnt[xtr_qu], throttle->ticks_left[xtr_qu]);
    }

#if VTSS_OPT_FDMA_VER >= 3
    pr("\n");
    pr("Rx MTU                    : %10u\n",  cfg->rx_mtu);
    pr("Rx Buffer Count           : %10u\n",  cfg->rx_buf_cnt);
    pr("Rx Strip VLAN Tag         : %10s\n", !cfg->rx_dont_strip_vlan_tag     ? "Yes" : "No");
    pr("Rx Allow VLAN Tag Mismatch: %10s\n",  cfg->rx_allow_vlan_tag_mismatch ? "Yes" : "No");
    pr("Rx Allow Multiple DCBs    : %10s\n",  cfg->rx_allow_multiple_dcbs     ? "Yes" : "No");
    pr("Rx Multi-DCB Drops        : %10u\n",  vstate->fdma_state.fdma_stats.rx_multiple_dcb_drops);
    pr("Rx Jumbo Drops            : %10u\n",  vstate->fdma_state.fdma_stats.rx_jumbo_drops);
    pr("Tx Buffer Count           : %10u\n",  cfg->tx_buf_cnt);
#endif

    FDMA_INTERRUPT_RESTORE(flags);

    pr("\n");

    return VTSS_RC_OK;
}

/*****************************************************************************/
// vtss_fdma_cmn_init_conf_set()
/*****************************************************************************/
vtss_rc vtss_fdma_cmn_init_conf_set(vtss_state_t *vtss_state)
{
#if VTSS_OPT_FDMA_VER <= 2
    vtss_state->fdma_state.mirror_port_no   = VTSS_PORT_NO_NONE;
    vtss_state->fdma_state.mirror_port_mask = 0;
#endif /* VTSS_OPT_FDMA_VER <= 2 */

    vtss_state->fdma_state.xtr_burst_size_bytes = 4; /* Default. May be overridden by chip-specific function */

    FDMA_CHECK_CALL_RETURN(vtss_state, fdma_init_conf_set);
}

#endif /* defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA */

/*****************************************************************************/
//
// End of file
//
//****************************************************************************/
