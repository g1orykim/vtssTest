/*

 Vitesse Switch Software.

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

*/
/*****************************************************************************/
//
// vtss_fdma.c
//
//*****************************************************************************/

// Avoid "vtss_api.h not used in module vtss_fdma.c"
/*lint --e{766} */

#define VTSS_TRACE_LAYER VTSS_TRACE_LAYER_AIL

#include "vtss_api.h"

#if VTSS_OPT_FDMA

#include "vtss_fdma_common.h"
#include "vtss_common.h"
#include "vtss_util.h" /* For MAX() */

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// alloc_and_stitch_dcbs()
/*****************************************************************************/
static void *alloc_and_stitch_dcbs(fdma_sw_dcb_t **dcb_ptr, u32 dcb_cnt, u32 mem_flags, vtss_fdma_dcb_type_t dcb_type)
{
    void          *mem, *end_of_mem_ptr; // For verification
    fdma_sw_dcb_t *dcb_iter, *latest = NULL;
    u32           dcb, dcb_size_bytes;

    mem = vtss_fdma_cmn_dcb_and_frm_alloc(
        dcb_cnt,            // Number of DCBs to allocate.
        0,                  // We don't need frame data memory.
        mem_flags,          // Where to allocate from?
        dcb_ptr,            // Pointer to cache-aligned DCB.
        NULL,               // We don't need frame data memory.
        &end_of_mem_ptr,    // Pointer to one byte beyond the last allocated byte. For verification.
        FALSE);             // We're not in a thread safe context right now.

    if (mem == NULL) {
        // Error already printed
        return NULL;
    }

    dcb_iter = *dcb_ptr;

    // Size of one DCB, so that the next also becomes cache aligned.
    dcb_size_bytes = VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE(sizeof(fdma_sw_dcb_t));

    // Stitch the DCBs together.
    for (dcb = 0; dcb < dcb_cnt; dcb++) {
        dcb_iter->dcb_type            = dcb_type;
        FDMA_USER_DCB(dcb_iter)->next = FDMA_USER_DCB((fdma_sw_dcb_t *)((u8 *)dcb_iter + dcb_size_bytes));
        latest                        = dcb_iter;
        dcb_iter                      = FDMA_SW_DCB(FDMA_USER_DCB(dcb_iter)->next);
    }
    FDMA_USER_DCB(latest)->next = NULL;

    // Verify that we haven't exceeded the allocated memory.
    if ((void *)dcb_iter > end_of_mem_ptr) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Internal allocation error");
        VTSS_OS_FREE(mem, mem_flags);
        return NULL;
    }

    return mem;
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_init()
// OBSOLETE. It's functionality is handled through fdma_init_conf_set() during
// API initialization
/*****************************************************************************/
vtss_rc vtss_fdma_init(const vtss_inst_t inst)
{
    // Better not break old code.
    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/*****************************************************************************/
// vtss_fdma_uninit()
/*****************************************************************************/
vtss_rc vtss_fdma_uninit(const vtss_inst_t inst)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_uninit);
}

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_cfg()
/*****************************************************************************/
vtss_rc vtss_fdma_cfg(const vtss_inst_t inst, const vtss_fdma_cfg_t *const cfg)
{
    // Unused
    return VTSS_RC_OK;
}
#else
/*****************************************************************************/
// vtss_fdma_cfg()
// For use with FDMA API v3.
/*****************************************************************************/
vtss_rc vtss_fdma_cfg(const vtss_inst_t inst, const vtss_fdma_cfg_t *const cfg)
{
    vtss_state_t     *vstate;
    fdma_state_t     *state;
    u32              dcb_cnt_per_ch, dcb_cnt_total, dcb_size_bytes, dcb;
    u32              bytes_per_frm_fdma_allocated, total_frame_data_bytes;
    u32              alloc_len_due_to_burst_size;
    fdma_sw_dcb_t    *dcb_ptr, *latest = NULL;
    u8               *frm_ptr;
    void             *orig_frm_ptr, *end_of_mem_ptr;
    vtss_fdma_ch_t   ch, xtr_ch;

    FDMA_CHECK_INST_RC(inst, vstate);
    state = &vstate->fdma_state;

    if (!cfg->enable) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Disabling of FDMA is not (yet) supported");
        return VTSS_RC_ERROR;
    }

    if (state->enabled) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "FDMA is already enabled");
        return VTSS_RC_ERROR;
    }

    if (cfg->rx_mtu == 0 || cfg->rx_buf_cnt == 0) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Freeing of memory is not (yet) supported");
        return VTSS_RC_ERROR;
    }

    /*lint -e{506} Constant value Boolean */
    if (cfg->rx_mtu <= MAX(64, VTSS_FDMA_MIN_DATA_PER_XTR_SOF_DCB_BYTES) || cfg->rx_mtu > VTSS_FDMA_MAX_DATA_PER_DCB_BYTES) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Invalid MTU (%u)", cfg->rx_mtu);
        return VTSS_RC_ERROR;
    }

    if (cfg->rx_cb == NULL) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "No Rx callback specified");
        return VTSS_RC_ERROR;
    }

    if (cfg->tx_done_cb == NULL) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "No Tx done callback specified");
        return VTSS_RC_ERROR;
    }

    if (cfg->tx_buf_cnt == 0) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Freeing of memory is not (yet) supported");
        return VTSS_RC_ERROR;
    }

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    // It's OK not to use the AFI offload mechanism even if defined.
    if ((cfg->afi_done_cb == NULL && cfg->afi_buf_cnt != 0) ||
        (cfg->afi_done_cb != NULL && cfg->afi_buf_cnt == 0)) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Either both AFI buffer count and callback must be zero or both be non-zero");
        return VTSS_RC_ERROR;
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

    // Allocate injection DCBs.
    state->inj_mem = alloc_and_stitch_dcbs(&state->inj_dcbs, cfg->tx_buf_cnt, VTSS_MEM_FLAGS_DMA, VTSS_FDMA_DCB_TYPE_INJ);
    if (state->inj_mem == NULL) {
        goto exit_with_error;
    }

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    // Allocate injection AFI DCBs.
    // For FDMA-based injected frames (VTSS_FEATURE_AFI_FDMA), these DCBs don't
    // need to be from DMA-capable memory, because they are only used to hold
    // user frame data. The underlying FDMA driver will allocate its own DCBs
    // and frame memory from persistent and DMA-capabale memory.
    // For AFI-based injected frames (VTSS_FEATURE_AFI_SWC), these DCBs must be
    // allocated from DMA-capable memory, because they are being used as is
    // to inject to the switch core.
    //
    // The common denominator is to actually allocate from DMA-capable memory.
    if (cfg->afi_buf_cnt) {
        state->afi_mem = alloc_and_stitch_dcbs(&state->afi_dcbs, cfg->afi_buf_cnt, VTSS_MEM_FLAGS_DMA, VTSS_FDMA_DCB_TYPE_AFI);
        if (state->afi_mem == NULL) {
            goto exit_with_error;
        }
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

    // Allocate extraction DCBs and possibly also frame memory.

    // We allocate the same number of DCBs for all extraction channels, so find the next value
    // of rx_buf_cnt which is a multiple of the extraction channel count.
    dcb_cnt_per_ch = ((cfg->rx_buf_cnt + state->xtr_ch_cnt - 1) / state->xtr_ch_cnt);
    dcb_cnt_total  = dcb_cnt_per_ch * state->xtr_ch_cnt;

    // Size of one DCB, so that the next also becomes cache aligned.
    dcb_size_bytes = VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE(sizeof(fdma_sw_dcb_t));

    // Find the required length per frame.
    // We need to take into account the FDMA's burst size. The FDMA bursts
    // in chunks of vstate->fdma_state.xtr_burst_size_bytes, so the amount of data
    // the FDMA is fed with per DCB must be a multiple of the burst size.
    alloc_len_due_to_burst_size = VTSS_FDMA_CMN_ALIGNED_SIZE(state->xtr_hdr_size + cfg->rx_mtu, state->xtr_burst_size_bytes);
    // The additional bytes to accommodate for differences between extraction and injection
    // header size must be added now.
    alloc_len_due_to_burst_size += (state->hdr_size - state->xtr_hdr_size);

    // Size of one frame's data area must also be a multiple of a cache line size.
    // Otherwise, invalidating the cache lines might stretch into the next frame.
    // Also, due to the way that the rx_info is intermingled with frame data, this
    // structure must also fill a multiple of a cache line.
    bytes_per_frm_fdma_allocated = VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE(alloc_len_due_to_burst_size) +
                                   VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE(sizeof(vtss_packet_rx_info_t));

    // We need to request dcb_cnt_total * bytes_per_frm_fdma_allocated bytes frame memory PLUS
    // one cache line (less one byte), because it might be that the pointer we get
    // back is not cache aligned.
    // Note, that we only need one single additional cache line, no matter how many DCBs
    // we allocate. If the application allocates the frame memory, we need one additional
    // cache line PER DCB.
    total_frame_data_bytes = dcb_cnt_total * bytes_per_frm_fdma_allocated + VTSS_OS_DCACHE_LINE_SIZE_BYTES - 1;

    // Set state->bytes_per_frm to reflect the application-scenario mentioned in the
    // previous comment. In the API-allocation-scenario, this value is no longer used.
    state->bytes_per_frm = bytes_per_frm_fdma_allocated + (VTSS_OS_DCACHE_LINE_SIZE_BYTES - 1);

    // Figure out how much frame data is required per dcb.
    // Layout of frame data is as follows:
    //    1) Cache-aligned rx_info (sizeof(vtss_packet_rx_info_t)
    //    4) Cache-aligned worst-case-sized injection/extraction header + cfg->rx_mtu bytes.
    state->xtr_mem = vtss_fdma_cmn_dcb_and_frm_alloc(
        dcb_cnt_total,                                 // Number of DCBs to allocate.
        cfg->rx_alloc_cb ? 0 : total_frame_data_bytes, // Number of frame data bytes to allocate. 0 if application-allocated.
        VTSS_MEM_FLAGS_DMA,                            // We need memory from DMA-capable area.
        &dcb_ptr,                                      // Pointer to cache-aligned DCB.
        &orig_frm_ptr,                                 // Pointer to frame memory. Only set if we allocated any.
        &end_of_mem_ptr,                               // Pointer to one byte beyond the last allocated byte. For verification.
        FALSE);                                        // We're not in a thread safe context right now.

    if (state->xtr_mem == NULL) {
        // Oops. Out of memory. Error already printed.
        return VTSS_RC_ERROR;
    }

    frm_ptr = (u8 *)VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE(orig_frm_ptr);

    // Stitch the DCBs and frame data together.
    for (xtr_ch = 0; xtr_ch < VTSS_FDMA_CH_CNT; xtr_ch++) {
        fdma_ch_state_t *ch_state = &state->fdma_ch_state[xtr_ch];

        if (ch_state->usage != VTSS_FDMA_CH_USAGE_XTR) {
            continue;
        }

        // Found an extraction channel. Assign dcb_cnt_per_ch DCBs to it.
        if (ch_state->cur_head || ch_state->free_head || ch_state->cur_tail || ch_state->pend_head || ch_state->status != FDMA_CH_STATUS_DISABLED) {
            VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Channel (%d) state is not reset. Continuing anyway", xtr_ch);
        }
        ch_state->cur_head  = dcb_ptr;
        ch_state->free_head = NULL;
        ch_state->cur_tail  = NULL;
        ch_state->pend_head = NULL;
        ch_state->status    = FDMA_CH_STATUS_DISABLED;

        for (dcb = 0; dcb < dcb_cnt_per_ch; dcb++) {
            if (cfg->rx_alloc_cb) {
                FDMA_USER_DCB(dcb_ptr)->alloc_ptr = NULL;

                // Ask application to allocate frame data into dcb_ptr->alloc_ptr.
                cfg->rx_alloc_cb(state->bytes_per_frm, FDMA_USER_DCB(dcb_ptr), VTSS_MEM_FLAGS_DMA);

                // See if it happened.
                if (FDMA_USER_DCB(dcb_ptr)->alloc_ptr == NULL) {
                    // Oops. Application couldn't allocate memory for us. Stop.
                    VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Application failed to allocate %u bytes of frame data", state->bytes_per_frm);
                    goto exit_with_error;
                }
            } else {
                FDMA_USER_DCB(dcb_ptr)->alloc_ptr = frm_ptr;
                frm_ptr += bytes_per_frm_fdma_allocated;
            }

            vtss_fdma_cmn_xtr_split_frame_data(dcb_ptr);
            dcb_ptr->dcb_type               = VTSS_FDMA_DCB_TYPE_XTR;
            dcb_ptr->xtr_ch                 = xtr_ch;
            FDMA_USER_DCB(dcb_ptr)->next    = FDMA_USER_DCB((fdma_sw_dcb_t *)((u8 *)dcb_ptr + dcb_size_bytes));
            dcb_ptr->alloc_len              = alloc_len_due_to_burst_size;
            latest                          = dcb_ptr;
            dcb_ptr                         = FDMA_SW_DCB(FDMA_USER_DCB(dcb_ptr)->next);
        }
        FDMA_USER_DCB(latest)->next = NULL;

        // This will initialize the DCBs.
        VTSS_RC(vtss_fdma_cmn_dcb_release(vstate, xtr_ch, ch_state->cur_head, FALSE /* It's not thread safe, since we're called from thread context */, FALSE /* don't connect */, FALSE /* don't start channel just yet */));
    }

    if ((void *)dcb_ptr > (cfg->rx_alloc_cb ? end_of_mem_ptr : orig_frm_ptr)) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Internal allocation error");
        goto exit_with_error;
    }

    if (cfg->rx_alloc_cb == NULL && latest != NULL && (void *)(latest->data + latest->alloc_len) > end_of_mem_ptr) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Internal allocation error");
        goto exit_with_error;
    }

    // Now that everything seems to be in place, enable all channels.
    for (ch = 0; ch < VTSS_FDMA_CH_CNT; ch++) {
        fdma_ch_state_t *ch_state = &state->fdma_ch_state[ch];

#if defined(VTSS_FEATURE_AFI_FDMA)
        if (ch_state->usage == VTSS_FDMA_CH_USAGE_CCM) {
            if (cfg->afi_buf_cnt > 0) {
                ch_state->status = FDMA_CH_STATUS_ENABLED;
            }
        } else
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
        if (ch_state->usage != VTSS_FDMA_CH_USAGE_UNUSED) {
            // Started
            ch_state->status = FDMA_CH_STATUS_ENABLED;
        }

        if (ch_state->status == FDMA_CH_STATUS_ENABLED) {
            // Ask the CIL layer to start the channel.
            vstate->fdma_state.fdma_func.fdma_start_ch(vstate, ch);
        }
    }

    state->fdma_cfg = *cfg;
    state->enabled = TRUE;
    return VTSS_RC_OK;

exit_with_error:
    if (state->inj_mem) {
        VTSS_OS_FREE(state->inj_mem, VTSS_MEM_FLAGS_DMA);
        state->inj_mem = 0;
    }
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    if (state->afi_mem) {
        VTSS_OS_FREE(state->afi_mem, VTSS_MEM_FLAGS_DMA);
        state->afi_mem = 0;
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
    if (state->xtr_mem) {
        VTSS_OS_FREE(state->xtr_mem, VTSS_MEM_FLAGS_DMA);
        state->xtr_mem = 0;
    }
    return VTSS_RC_ERROR;
}
#endif /* VTSS_OPT_FDMA_VER */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_xtr_cfg()
// Deprecated. Use vtss_fdma_ch_cfg() instead.
/*****************************************************************************/
vtss_rc vtss_fdma_xtr_cfg(const vtss_inst_t            inst,
                          const BOOL                   ena,
                          const vtss_fdma_ch_t         ch,
                          const vtss_packet_rx_queue_t qu,
#if VTSS_OPT_FDMA_VER == 1
                          const int                    start_gap,
                          const int                    end_gap,
#endif /* VTSS_OPT_FDMA_VER == 1 */
                                vtss_fdma_list_t       *const list,
                                vtss_fdma_list_t       *(*xtr_cb)(void *cntxt, vtss_fdma_list_t *list, vtss_packet_rx_queue_t qu))
{
#if VTSS_OPT_FDMA_VER == 2
    FDMA_CHECK_CALL_RETURN(inst, fdma_xtr_cfg, ena, ch, qu,                     list, xtr_cb);
#else
    FDMA_CHECK_CALL_RETURN(inst, fdma_xtr_cfg, ena, ch, qu, start_gap, end_gap, list, xtr_cb);
#endif /* VTSS_OPT_FDMA_VER */
}
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_inj_cfg()
// Deprecated. Use vtss_fdma_ch_cfg() instead.
/*****************************************************************************/
vtss_rc vtss_fdma_inj_cfg(const vtss_inst_t inst, const BOOL ena, const vtss_fdma_ch_t ch)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_inj_cfg, ena, ch);
}
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_ch_cfg()
/*****************************************************************************/
vtss_rc vtss_fdma_ch_cfg(const vtss_inst_t inst, const vtss_fdma_ch_t ch, const vtss_fdma_ch_cfg_t *const cfg)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_ch_cfg, ch, cfg);
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_xtr_add_dcbs()
/*****************************************************************************/
vtss_rc vtss_fdma_xtr_add_dcbs(const vtss_inst_t inst, const vtss_fdma_ch_t xtr_ch, vtss_fdma_list_t *const list)
{
    vtss_state_t         *vstate;
    fdma_state_t         *state;
    fdma_ch_state_t      *ch_state;
    fdma_sw_dcb_t        *sw_dcb = FDMA_SW_DCB(list);

    // Valid list?
    if (list == NULL) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Illegal parameter, list");
        return VTSS_RC_ERROR;
    }

    FDMA_CHECK_INST_RC(inst, vstate);

    // Valid channel?
    if (xtr_ch < 0 || xtr_ch >= VTSS_FDMA_CH_CNT) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Illegal channel (%d)", xtr_ch);
        return VTSS_RC_ERROR;
    }

    state = &vstate->fdma_state;
    ch_state = &state->fdma_ch_state[xtr_ch];

    // Configured for extraction and enabled?
    if (ch_state->usage != VTSS_FDMA_CH_USAGE_XTR || ch_state->status != FDMA_CH_STATUS_ENABLED) {
         VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Not an extraction channel or not enabled");
         return VTSS_RC_INV_STATE;
    }

    return vtss_fdma_cmn_dcb_release(vstate, xtr_ch, sw_dcb, FALSE /* Not thread safe */, TRUE /* connect */, TRUE /* restart channel */);
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// vtss_fdma_dcb_release()
/*****************************************************************************/
vtss_rc vtss_fdma_dcb_release(const vtss_inst_t inst, vtss_fdma_list_t *const list)
{
    vtss_state_t *vstate;

    // Valid list?
    if (list == NULL) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Illegal parameter, list");
        return VTSS_RC_ERROR;
    }

    FDMA_CHECK_INST_RC(inst, vstate);
    return vtss_fdma_cmn_dcb_release(vstate, 0 /* not used */, FDMA_SW_DCB(list), FALSE /* not thread safe */, TRUE /* do connect if these are extraction DCBs */, TRUE /* do restart if these are extraction DCBs */);
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_inj()
/*****************************************************************************/
vtss_rc vtss_fdma_inj(const vtss_inst_t inst, vtss_fdma_list_t *list, const vtss_fdma_ch_t ch, const u32 len, const vtss_fdma_inj_props_t *const props)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_inj, list, ch, len, props);
}
#else
/*****************************************************************************/
// vtss_fdma_tx)
/*****************************************************************************/
vtss_rc vtss_fdma_tx(const vtss_inst_t inst, vtss_fdma_list_t *list, vtss_fdma_tx_info_t *const fdma_info, vtss_packet_tx_info_t *const tx_info)
{
    vtss_state_t     *vstate;
    fdma_state_t     *state;
    vtss_fdma_list_t *user_dcb;
    fdma_sw_dcb_t    *sw_dcb;
    vtss_rc          rc;
    BOOL             sof = TRUE;

    // Valid structures?
    if (list == NULL || fdma_info == NULL || tx_info == NULL) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Illegal parameter");
        return VTSS_RC_ERROR;
    }

    FDMA_CHECK_INST_RC(inst, vstate);
    state    = &vstate->fdma_state;
#if defined(VTSS_FEATURE_AFI_FDMA) && defined(VTSS_FEATURE_AFI_SWC)
    // Both FDMA- and Switch-core-based AFI is supported on this platform.
    if (fdma_info->afi_fps > 0 && fdma_info->afi_type == VTSS_FDMA_AFI_TYPE_AUTO) {
        // And the application wants to use the AFI, and wants us to auto-select.
        // We do that based on whether frame counting or sequence numbering is
        // enabled. If enabled, we can only use the FDMA-based.
        if (fdma_info->afi_enable_counting || fdma_info->afi_enable_sequence_numbering) ®{
            fdma_info->afi_type = VTSS_FDMA_AFI_TYPE_FDMA;
        } else {
            fdma_info->afi_type = VTSS_FDMA_AFI_TYPE_SWC;
        }
    } else if (fdma_info->afi_fps > 0 && fdma_info->afi_type == VTSS_FDMA_AFI_TYPE_SWC) {
        if (fdma_info->afi_enable_counting || fdma_info->afi_enable_sequence_numbering) {
            VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Counting or Sequence numbering is not supported on switch-core-AFI");
            return VTSS_RC_ERROR;
        }
    }
#elif defined(VTSS_FEATURE_AFI_FDMA)
    // Only FDMA-based is supported on this platform.
    if (fdma_info->afi_fps > 0) {
        if (fdma_info->afi_type != VTSS_FDMA_AFI_TYPE_AUTO  && fdma_info->afi_type != VTSS_FDMA_AFI_TYPE_FDMA) {
            VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Invalid AFI type chosen (%d). Only FDMA-based supported", fdma_info->afi_type);
            return VTSS_RC_ERROR;
        }
        fdma_info->afi_type = VTSS_FDMA_AFI_TYPE_FDMA;
    }
#elif defined(VTSS_FEATURE_AFI_SWC)
    // Only SwC-based is supported on this platform.
    if (fdma_info->afi_fps > 0) {
        if (fdma_info->afi_type != VTSS_FDMA_AFI_TYPE_AUTO  && fdma_info->afi_type != VTSS_FDMA_AFI_TYPE_SWC) {
            VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Invalid AFI type chosen (%d). Only SwC-based supported", fdma_info->afi_type);
            return VTSS_RC_ERROR;
        }
        fdma_info->afi_type = VTSS_FDMA_AFI_TYPE_SWC;
    }
#endif /* defined(SOME_KIND_OF_AFI_SUPPORT) */

    user_dcb = list;

    // Prepare the list's data and ifh_ptr members.
    while (user_dcb) {
        sw_dcb = FDMA_SW_DCB(user_dcb);

        // The application has prepended state->hdr_size free space to the frame for the start-of-frame DCB.
        if (sof) {
            sw_dcb->data = user_dcb->frm_ptr - state->hdr_size;
            sof = FALSE;
        } else {
            sw_dcb->data = user_dcb->frm_ptr;
        }

        // Initially, let the IFH pointer point to the frame pointer, which is consistent
        // with the way that we think of "frm_ptr - ifh_ptr", which gives the size of the IFH
        // in bytes, which is 0 by now.
        sw_dcb->ifh_ptr = user_dcb->frm_ptr;
        user_dcb        = user_dcb->next;
    }

    // Call the CIL function
    sw_dcb = FDMA_SW_DCB(list);
    if ((rc = vstate->fdma_state.fdma_func.fdma_tx(vstate, sw_dcb, fdma_info, tx_info, FALSE)) != VTSS_RC_OK) {
        // Release the DCBs
        (void)vtss_fdma_cmn_dcb_release(vstate, 0 /* doesn't matter */, sw_dcb, FALSE /* called from thread */, FALSE /* doesn't matter */, FALSE /* doesn't matter */);
    }

    return rc;
}
#endif /* VTSS_OPT_FDMA_VER */

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// vtss_fdma_tx_info_init()
/*****************************************************************************/
vtss_rc vtss_fdma_tx_info_init(const vtss_inst_t inst, vtss_fdma_tx_info_t *const fdma_info)
{
    if (fdma_info == NULL) {
        return VTSS_RC_ERROR;
    }

    memset(fdma_info, 0, sizeof(*fdma_info));
    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if (defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)) && VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// vtss_fdma_afi_cancel()
/*****************************************************************************/
vtss_rc vtss_fdma_afi_cancel(const vtss_inst_t inst, const u8 *const frm_ptr)
{
    vtss_state_t          *vstate;
    fdma_sw_dcb_t         sw_dcb;
    vtss_fdma_tx_info_t   fdma_info;
    vtss_packet_tx_info_t tx_info;

    if (frm_ptr == NULL) {
        return VTSS_RC_ERROR;
    }

    FDMA_CHECK_INST_RC(inst, vstate);

    // All we need to set is a pointer to the frame.
    FDMA_USER_DCB(&sw_dcb)->frm_ptr = (u8 *)frm_ptr;

    return vstate->fdma_state.fdma_func.fdma_tx(vstate, &sw_dcb, &fdma_info, &tx_info, TRUE);
}
#endif /* (defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)) && VTSS_OPT_FDMA_VER >= 3 */

#if defined(VTSS_FEATURE_AFI_FDMA) && VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// vtss_fdma_afi_frm_cnt()
/*****************************************************************************/
vtss_rc vtss_fdma_afi_frm_cnt(const vtss_inst_t inst, const u8 *const frm_ptr, u64 *const frm_cnt)
{
    if (frm_ptr == NULL || frm_cnt == NULL) {
        return VTSS_RC_ERROR;
    }

    FDMA_CHECK_CALL_RETURN(inst, fdma_afi_frm_cnt, frm_ptr, frm_cnt);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) && VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// vtss_fdma_dcb_get()
// Get injection or AFI DCBs (extraction DCBs cannot get gotten).
/*****************************************************************************/
vtss_rc vtss_fdma_dcb_get(const vtss_inst_t inst, u32 dcb_cnt, vtss_fdma_dcb_type_t dcb_type, vtss_fdma_list_t **list)
{
    vtss_state_t     *vstate;
    vtss_fdma_list_t *dcb;
    fdma_sw_dcb_t    **state_dcb_list;
    vtss_rc          rc;
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS flags;

    if (dcb_cnt == 0 || list == NULL || (dcb_type != VTSS_FDMA_DCB_TYPE_INJ
#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
        && dcb_type != VTSS_FDMA_DCB_TYPE_AFI
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
    )) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Illegal parameter");
        return VTSS_RC_ERROR;
    }

    FDMA_CHECK_INST_RC(inst, vstate);

    // Don't get disturbed while messing with the list.
    FDMA_INTERRUPT_DISABLE(flags);

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    if (dcb_type == VTSS_FDMA_DCB_TYPE_AFI) {
        state_dcb_list = &vstate->fdma_state.afi_dcbs;
        *list = FDMA_USER_DCB(*state_dcb_list);
    } else
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */
    {
        state_dcb_list = &vstate->fdma_state.inj_dcbs;
        *list = FDMA_USER_DCB(*state_dcb_list);
    }

    dcb = *list;
    while (dcb != NULL && --dcb_cnt > 0) {
        dcb = dcb->next;
    }

    // dcb now points to the last DCB the user wants.
    if (dcb == NULL) {
        *list = NULL;
        VTSS_IG(VTSS_TRACE_GROUP_FDMA_IRQ, "Out of %s DCBs", dcb_type == VTSS_FDMA_DCB_TYPE_INJ ? "injection" : "AFI");
        rc = VTSS_RC_INCOMPLETE; // Not enough DCBs to fulfill request.
    } else {
        // Update the appropriate DCB list pointer in fdma_state.
        *state_dcb_list = FDMA_SW_DCB(dcb->next);
        dcb->next = NULL;
        rc = VTSS_RC_OK;
    }

    FDMA_INTERRUPT_RESTORE(flags);

    return rc;
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_inj_flow_ctrl_cfg_get()
/*****************************************************************************/
vtss_rc vtss_fdma_inj_flow_ctrl_cfg_get(const vtss_inst_t inst, const vtss_phys_port_no_t port, BOOL *const in_tx_flow_ctrl, u8 *const retransmit_cnt)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_inj_flow_ctrl_cfg_get, port, in_tx_flow_ctrl, retransmit_cnt);
}
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_inj_flow_ctrl_cfg_set()
/*****************************************************************************/
vtss_rc vtss_fdma_inj_flow_ctrl_cfg_set(const vtss_inst_t inst, const vtss_phys_port_no_t port, const BOOL in_tx_flow_ctrl, const u8 retransmit_cnt)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_inj_flow_ctrl_cfg_set, port, in_tx_flow_ctrl, retransmit_cnt);
}
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_xtr_ch_from_list()
/*****************************************************************************/
vtss_rc vtss_fdma_xtr_ch_from_list(const vtss_inst_t inst, const vtss_fdma_list_t *const list, vtss_fdma_ch_t *const ch)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_xtr_ch_from_list, list, ch);
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28)
/*****************************************************************************/
// vtss_fdma_inj_suspend_port(int port, BOOL suspend)
/*****************************************************************************/
vtss_rc vtss_fdma_inj_suspend_port(const vtss_inst_t inst, const vtss_phys_port_no_t port, const BOOL suspend)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_inj_suspend_port, port, suspend);
}
#endif /* VTSS_ARCH_LUTON28 */

#if defined(VTSS_ARCH_LUTON28)
/*****************************************************************************/
// vtss_fdma_inj_port_active()
/*****************************************************************************/
vtss_rc vtss_fdma_inj_port_active(const vtss_inst_t inst, const vtss_phys_port_no_t port, BOOL *const active)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_inj_port_active, port, active);
}
#endif /* VTSS_ARCH_LUTON28 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_xtr_hdr_decode()
/*****************************************************************************/
vtss_rc vtss_fdma_xtr_hdr_decode(const vtss_inst_t inst, const vtss_fdma_list_t *const list, vtss_fdma_xtr_props_t *const xtr_props)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_xtr_hdr_decode, list, xtr_props);
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/*
 * Mirroring is supported directly in vtss_packet_tx_hdr_encode()
 * if using VTSS_OPT_FDMA_VER >= 3
 */
#if defined(VTSS_FEATURE_MIRROR_CPU) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_inj_mirror_port_get()
/*****************************************************************************/
vtss_rc vtss_fdma_inj_mirror_port_get(const vtss_inst_t inst, vtss_chip_no_t *const chip_no, vtss_phys_port_no_t *const phys_port)
{
    vtss_state_t *vstate;

    if (chip_no == NULL || phys_port == 0) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Illegal parameter");
        return VTSS_RC_ERROR;
    }

    FDMA_CHECK_INST_RC(inst, vstate);

    VTSS_ENTER();

    if (vstate->fdma_state.mirror_port_no == VTSS_PORT_NO_NONE) {
        *chip_no   = 0;
        *phys_port = VTSS_PORT_NO_NONE;
    } else {
        *chip_no   = vstate->port_map[vstate->fdma_state.mirror_port_no].chip_no;
        *phys_port = vstate->port_map[vstate->fdma_state.mirror_port_no].chip_port;
    }

    VTSS_EXIT();
    return VTSS_RC_OK;
}
#endif /* defined(VTSS_FEATURE_MIRROR_CPU) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_FEATURE_MIRROR_CPU) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_inj_mirror_port_set()
// In FDMA API v3+, mirroring is supported in the implementation of
// vtss_packet_tx_hdr_encode(), which is also invoked by the FDMA driver.
/*****************************************************************************/
vtss_rc vtss_fdma_inj_mirror_port_set(const vtss_inst_t inst, vtss_chip_no_t chip_no, vtss_phys_port_no_t phys_port)
{
    vtss_rc rc = VTSS_RC_OK;
    vtss_state_t *vstate;
    FDMA_CHECK_INST_RC(inst, vstate);

    VTSS_ENTER();
    if (phys_port == VTSS_PORT_NO_NONE) {
        vstate->fdma_state.mirror_port_no   = VTSS_PORT_NO_NONE;
        vstate->fdma_state.mirror_port_mask = 0;
    } else {
        vtss_port_no_t port_no;
        port_no = vtss_cmn_chip_to_logical_port(vstate, chip_no, phys_port);
        if (port_no == VTSS_PORT_NO_NONE) {
            VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Illegal chip_no (%u) or physical port no (%u)", chip_no, phys_port);
            rc = VTSS_RC_ERROR;
            goto do_exit;
        }

#if defined(VTSS_FEATURE_VSTAX)
        if (port_no == vstate->vstax_conf.port_0 || port_no == vstate->vstax_conf.port_1) {
            VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Stack ports cannot be configured as mirror ports");
            rc = VTSS_RC_ERROR;
            goto do_exit;
        }
#endif /* defined(VTSS_FEATURE_VSTAX) */
        vstate->fdma_state.mirror_port_no   = port_no;
        vstate->fdma_state.mirror_port_mask = 1ULL << phys_port;
    }

do_exit:
    VTSS_EXIT();
    return rc;
}
#endif /* defined(VTSS_FEATURE_MIRROR_CPU) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vtss_fdma_stats_get()
// Deprecated.  On newer platforms, statistics cannot be obtained
// directly. Instead, use vtss_debug_info_print(print_func, info) with
//   @info.layer set to 'cil' and
//   @info.group set to VTSS_DEBUG_GROUP_FDMA.
/*****************************************************************************/
vtss_rc vtss_fdma_stats_get(const vtss_inst_t inst, vtss_fdma_stats_t *const stats)
{
    FDMA_CHECK_CALL_RETURN(inst, fdma_stats_get, stats);
}
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

/*****************************************************************************/
// vtss_fdma_stats_clr()
/*****************************************************************************/
vtss_rc vtss_fdma_stats_clr(const vtss_inst_t inst)
{
  FDMA_CHECK_CALL_RETURN(inst, fdma_stats_clr);
}

/*****************************************************************************/
// vtss_fdma_irq_handler()
/*****************************************************************************/
vtss_rc vtss_fdma_irq_handler(const vtss_inst_t inst, void *const cntxt)
{
  FDMA_CHECK_CALL_RETURN(inst, fdma_irq_handler, cntxt);
}

/*****************************************************************************/
// fdma_throttling_supported_check
/*****************************************************************************/
static vtss_rc fdma_throttling_supported_check(const vtss_state_t *const vstate)
{
    if (vstate->fdma_state.fdma_func.fdma_xtr_qu_suspend_set == NULL) {
        // Throttling is not supported.
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Throttling not supported on this platform");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

/*****************************************************************************/
// vtss_fdma_throttle_cfg_get()
/*****************************************************************************/
#if VTSS_OPT_FDMA_VER <= 2
vtss_rc vtss_fdma_throttle_cfg_get(const vtss_inst_t inst, const vtss_fdma_ch_t ch, vtss_fdma_throttle_cfg_t *const cfg)
#else
vtss_rc vtss_fdma_throttle_cfg_get(const vtss_inst_t inst, vtss_fdma_throttle_cfg_t *const cfg)
#endif /* VTSS_OPT_FDMA_VER */
{
    vtss_state_t *vstate;

    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS flags;

    FDMA_CHECK_INST_RC(inst, vstate);
    VTSS_RC(fdma_throttling_supported_check(vstate));

    if (cfg == NULL) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Illegal parameter");
        return VTSS_RC_ERROR;
    }

    FDMA_INTERRUPT_DISABLE(flags);
    *cfg = vstate->fdma_state.throttle.cfg;
    FDMA_INTERRUPT_RESTORE(flags);

    return VTSS_RC_OK;
}

/*****************************************************************************/
// vtss_fdma_throttle_cfg_set()
/*****************************************************************************/
#if VTSS_OPT_FDMA_VER <= 2
vtss_rc vtss_fdma_throttle_cfg_set(const vtss_inst_t inst, const vtss_fdma_ch_t ch, const vtss_fdma_throttle_cfg_t *const cfg)
#else
vtss_rc vtss_fdma_throttle_cfg_set(const vtss_inst_t inst, const vtss_fdma_throttle_cfg_t *const cfg)
#endif /* VTSS_OPT_FDMA_VER */
{
    vtss_state_t           *vstate;
    fdma_state_t           *state;
    vtss_packet_rx_queue_t xtr_qu;
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS   flags;

    FDMA_CHECK_INST_RC(inst, vstate);
    VTSS_RC(fdma_throttling_supported_check(vstate));

    state = &vstate->fdma_state;

    if (cfg == NULL) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Illegal parameter");
        return VTSS_RC_ERROR;
    }

    FDMA_INTERRUPT_DISABLE(flags);

    for (xtr_qu = 0; xtr_qu < VTSS_PACKET_RX_QUEUE_CNT; xtr_qu++) {
        if (state->throttle.ticks_left[xtr_qu]) {
            // Before changing the configuration, pull the extraction queue out of suspend state.
            state->fdma_func.fdma_xtr_qu_suspend_set(vstate, xtr_qu, FALSE);
        }
    }

    memset(&state->throttle, 0, sizeof(state->throttle));
    state->throttle.cfg = *cfg;
    FDMA_INTERRUPT_RESTORE(flags);
    return VTSS_RC_OK;
}

/*****************************************************************************/
// vtss_fdma_throttle_tick()
/*****************************************************************************/
vtss_rc vtss_fdma_throttle_tick(const vtss_inst_t inst)
{
    vtss_state_t           *vstate;
    fdma_state_t           *state;
    vtss_packet_rx_queue_t xtr_qu;
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS   flags;

    FDMA_CHECK_INST_RC(inst, vstate);
    VTSS_RC(fdma_throttling_supported_check(vstate));
    state = &vstate->fdma_state;

    FDMA_INTERRUPT_DISABLE(flags);

    state->throttle.tick_cnt++;

    // Loop over all queues to see if any of them is currently suspended and needs unsuspension.
    for (xtr_qu = 0; xtr_qu < VTSS_PACKET_RX_QUEUE_CNT; xtr_qu++) {
        if (state->throttle.ticks_left[xtr_qu] > 0) {
            // The queue is suspended. Check to see if suspension time has elapsed.
            if (--state->throttle.ticks_left[xtr_qu] == 0) {
                // The queue has been silent long enough. Re-enable it.
                state->fdma_func.fdma_xtr_qu_suspend_set(vstate, xtr_qu, FALSE);
            }
        }

        // Update max received and clear counter. We do it even if the queue is not enabled for throttling,
        // because it can give the user an idea of the maximum number of frames received between two ticks.
        if (state->throttle.tick_cnt > 2) {
            if (state->throttle.frm_cnt[xtr_qu] > state->throttle.statistics_max_frames_per_tick[xtr_qu]) {
                // Only update max when the application has started a steady throttle tick.
                state->throttle.statistics_max_frames_per_tick[xtr_qu] = state->throttle.frm_cnt[xtr_qu];
            }
            if (state->throttle.byte_cnt[xtr_qu] > state->throttle.statistics_max_bytes_per_tick[xtr_qu]) {
                // Only update max when the application has started a steady throttle tick.
                state->throttle.statistics_max_bytes_per_tick[xtr_qu] = state->throttle.byte_cnt[xtr_qu];
            }
        }

        state->throttle.frm_cnt[xtr_qu] = 0;
        state->throttle.byte_cnt[xtr_qu] = 0;
    }

    FDMA_INTERRUPT_RESTORE(flags);
    return VTSS_RC_OK;
}

#endif /* VTSS_OPT_FDMA */

/*****************************************************************************/
//
// End of file
//
//****************************************************************************/
