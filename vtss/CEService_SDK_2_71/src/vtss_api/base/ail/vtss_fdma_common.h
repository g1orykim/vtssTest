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
/*****************************************************************************/
#ifndef _VTSS_FDMA_COMMON_H_
#define _VTSS_FDMA_COMMON_H_

// Some of the register definition stuff is buried in the public part.
#include "vtss_options.h"

#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
#include "vtss_fdma_api.h"
#include "vtss_state.h"

/*****************************************************************************/
// FDMA_VTSS_STATE_FROM_INST()
// Returns pointer to fdma_state if _inst_ is valid, otherwise NULL.
/*****************************************************************************/
#define FDMA_VTSS_STATE_FROM_INST(_inst_) ((_inst_) ? (_inst_) : vtss_default_inst)

/*****************************************************************************/
// FDMA_CONTAINER_OF()
/*****************************************************************************/
#define FDMA_CONTAINER_OF(ptr, type, member)              \
    ({                                                    \
    const __typeof__(((type *)0)->member)*__mptr = (ptr); \
    (type *)((char *)__mptr - offsetof(type,member));     \
    })

#if VTSS_OPT_FDMA_VER <= 2
#define FDMA_USER_DCB(_sw_dcb_)              (_sw_dcb_)
#define FDMA_SW_DCB(_user_dcb_)              (_user_dcb_)
#define FDMA_HW_DCB(_sw_dcb_)                ((fdma_hw_dcb_t *)&(_sw_dcb_)->hw_dcb[0])
#else
static inline vtss_fdma_list_t *FDMA_USER_DCB(fdma_sw_dcb_t *sw_dcb)
{
    return sw_dcb ? &sw_dcb->user_dcb : NULL;
}
static inline fdma_sw_dcb_t *FDMA_SW_DCB(vtss_fdma_list_t *user_dcb_ptr)
{
    return user_dcb_ptr ? FDMA_CONTAINER_OF(user_dcb_ptr, fdma_sw_dcb_t, user_dcb) : NULL;
}
#define FDMA_HW_DCB(_sw_dcb_)                (&(_sw_dcb_)->hw_dcb)
#endif /* VTSS_OPT_FDMA_VER */

// We don't operate with assertions in the API, so we print an error and gracefully return.
// Note: This macro is sometimes used with code that MUST be executed - even on non-debug builds -,
// so if you don't like to have VTSS_EG() calls, then rewrite the macro to avoid that call.
#define FDMA_CHECK_RC(thread_safe, expr)    /*lint -save -e506 */ do {if (!(expr)) {VTSS_EG((thread_safe ? VTSS_TRACE_GROUP_FDMA_IRQ : VTSS_TRACE_GROUP_FDMA_NORMAL), "Assert failed: " # expr); return VTSS_RC_ERROR;}} while (0); /*lint -restore */
#define FDMA_CHECK(thread_safe, expr, code) /*lint -save -e506 */ do {if (!(expr)) {VTSS_EG((thread_safe ? VTSS_TRACE_GROUP_FDMA_IRQ : VTSS_TRACE_GROUP_FDMA_NORMAL), "Assert failed: " # expr); code}}                  while (0); /*lint -restore */

/*****************************************************************************
 * FDMA_CHECK_INST_RC()
 * Assigns _vstate_ from an _inst_, and returns VTSS_RC_ERROR if it's NULL.
 *****************************************************************************/
#define FDMA_CHECK_INST_RC(_inst_, _vstate_)                                   \
    do {                                                                       \
        _vstate_ = FDMA_VTSS_STATE_FROM_INST(_inst_);                          \
        if (_vstate_ == NULL) {                                                \
            VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Illegal inst: %p", _inst_); \
            return VTSS_RC_ERROR;                                              \
        }                                                                      \
    } while (0);

/*****************************************************************************/
// The FDMA code is self-mutually excluding, so that the calls to the functions
// don't need be protected by VTSS_ENTER()/VTSS_EXIT() calls.
// In fact, protecting it would be prohibitive if the IRQ handler should be
// called in interrupt context, since calls to wait() are unallowed in that case.
// Internally, the FDMA code ensures mutual exlusion on the FDMA state between
// function calls by the use of the VTSS_OS_INTERRUPT_DISABLE()/RESTORE() or
// VTSS_OS_SCHEDULER_LOCK()/UNLOCK() macros.
//
// Note on the implementation of the macro:
//   1) The __VA_ARGS__ replaces the ... in the argument list.
//   2) Since the ... may be empty, ## is needed in the call. This will ensure
//      that the dangling comma after "state" is removed.
/*****************************************************************************/
#define FDMA_CHECK_CALL_RETURN(_inst_, _func_, ...)                                 \
    do {                                                                            \
        vtss_state_t *_vstate_;                                                     \
        FDMA_CHECK_INST_RC(_inst_, _vstate_);                                       \
        if (_vstate_->fdma_state.fdma_func._func_) {                                \
            return _vstate_->fdma_state.fdma_func._func_(_vstate_, ## __VA_ARGS__); \
        } else {                                                                    \
            VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Function not supported");        \
            return VTSS_RC_ERROR;                                                   \
        }                                                                           \
    } while (0)


/****************************************************************************/
// VTSS_FDMA_CMN_ALIGNED_SIZE()
// Use this macro to align a block of memory to a given alignment.
// Only use powers of two for _align_.
/****************************************************************************/
#define VTSS_FDMA_CMN_ALIGNED_SIZE(_addr_, _align_) (((u32)(_addr_) + ((u32)(_align_) - 1)) & ~((u32)(_align_) - 1))

/****************************************************************************/
// VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE()
// Get the size of a DCB, given it must be cache-aigned.
// The macro may also be used to cache-align a block of memory.
/****************************************************************************/
#define VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE(_size_) VTSS_FDMA_CMN_ALIGNED_SIZE(_size_, VTSS_OS_DCACHE_LINE_SIZE_BYTES)

void    vtss_fdma_cmn_throttle_suspend_check(vtss_state_t *vstate, vtss_packet_rx_queue_t xtr_qu, u32 bytes);
vtss_rc vtss_fdma_cmn_debug_print_fdma(vtss_state_t *const vstate, const vtss_debug_printf_t pr, const vtss_debug_info_t *const info); // AIL Layer
vtss_rc vtss_fdma_cmn_init_conf_set(vtss_state_t *vtss_state);
vtss_rc vtss_fdma_cmn_dcb_release(vtss_state_t *const vstate, vtss_fdma_ch_t ch, fdma_sw_dcb_t *const list, BOOL thread_safe, BOOL connect, BOOL restart);
void    vtss_fdma_cmn_xtr_restart_ch(vtss_state_t *const vstate, vtss_fdma_ch_t xtr_ch);
#if VTSS_OPT_FDMA_VER <= 2
vtss_rc vtss_fdma_cmn_inj_attach_frm(vtss_state_t *const vstate, fdma_sw_dcb_t *head, fdma_sw_dcb_t *tail, vtss_fdma_ch_t inj_ch, u32 dcb_cnt, u32 len, const vtss_fdma_inj_props_t *const props, BOOL thread_safe);
#else
vtss_rc vtss_fdma_cmn_inj_attach_frm(vtss_state_t *const vstate, fdma_sw_dcb_t *head, fdma_sw_dcb_t *tail, vtss_fdma_ch_t inj_ch, u32 dcb_cnt, u32 len, const vtss_fdma_tx_info_t   *const props, BOOL thread_safe);
#endif /* VTSS_OPT_FDMA_VER */
u32     vtss_fdma_cmn_dcb_mem_size(u32 dcb_cnt);
void   *vtss_fdma_cmn_dcb_and_frm_alloc(u32 dcb_cnt, u32 frm_bytes, u32 mem_type, fdma_sw_dcb_t **dcb_ptr, void **frm_ptr, void **end_of_mem_ptr, BOOL thread_safe);
u32     vtss_fdma_cmn_fcs_get(const fdma_sw_dcb_t *sof);
void     vtss_fdma_cmn_xtr_split_frame_data(fdma_sw_dcb_t *sw_dcb);
#if VTSS_OPT_FDMA_VER >= 3
vtss_rc vtss_fdma_cmn_xtr_hdr_decode(vtss_state_t *vstate, fdma_sw_dcb_t *sof, vtss_chip_no_t chip_no, vtss_packet_rx_queue_t xtr_qu, VTSS_OS_TIMESTAMP_TYPE *sw_tstamp);
vtss_rc vtss_fdma_cmn_insert_vlan_tag(fdma_sw_dcb_t *sw_dcb, vtss_vlan_tag_t *tag, BOOL move_ifh, BOOL thread_safe);
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#endif /* defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA */
#endif /* _VTSS_FDMA_COMMON_H_ */
/*****************************************************************************/
//
// End of file
//
//****************************************************************************/
