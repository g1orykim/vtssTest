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
#ifndef _VTSS_FDMA_H_
#define _VTSS_FDMA_H_

// Some of the register definition stuff is buried in the public part.
#include "vtss_fdma_api.h"
#include "vtss_options.h"
#include <vtss_os.h> /* For VTSS_OS_BIG_ENDIAN */

#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA

#if defined(VTSS_FEATURE_AFI_SWC)
#include "../serval/vtss_serval.h" /* For VTSS_AFI_SLOT_CNT */
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA)
#if VTSS_OPT_FDMA_VER <= 2
#define AFI_FREQ_LIST_LEN VTSS_FDMA_CCM_FREQ_LIST_LEN
#else
#define AFI_FREQ_LIST_LEN 5
#endif /* VTSS_OPT_FDMA_VER */
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */


// The whole FDMA driver will only be compilable on one platform at a time,
// which is fine, because it must run on the internal CPU corresponding to
// the platform it is compiled for.
typedef struct {
#if defined(VTSS_ARCH_LUTON28)
    u32 sar;
    u32 dar;
    u32 llp;
    u32 ctl0;
    u32 ctl1;
    u32 sstat;
    u32 dstat;
#elif defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_LUTON26)
    u32 sar;
    u32 dar;
    u32 llp;
    u32 ctl0;
    u32 ctl1;
    /**
     * XTR: GPDMA writes back extraction status (DS_UPD_EN == TRUE).
     * INJ: Not used.
     * AFI: DCBs sent to H/W: Holds pointer to user-DCB (which is not sent to H/W).
     *      User-DCBs: Holds sequence number offset.
     *      Both are purely S/W controlled and are just put here in order to save
     *      the number of bytes that a vtss_fdma_list_t occupies.
     */
    u32 stat;
#elif defined(VTSS_ARCH_SERVAL)
    u32 llp;
    u32 datap;
    u32 datal;
    u32 stat;
#else
#error "Unsupported platform"
#endif /* VTSS_ARCH_xxx */
} volatile fdma_hw_dcb_t;

#if VTSS_OPT_FDMA_VER >= 3
// The application allocates DCBs prior to version 3 of the FDMA API,
// so we can't divide the vtss_fdma_list_t into a public and private
// part unless we're using versions >= 3.
typedef struct {
    // H/W DCB. Must come first and must be cache-aligned.
    fdma_hw_dcb_t hw_dcb;

    u8 unused[VTSS_OS_DCACHE_LINE_SIZE_BYTES - sizeof(fdma_hw_dcb_t)];

    /**
     * Determines what this DCB is used for, so that it can be returned to the
     * correct pool of DCBs once it needs to be freed.
     */
    vtss_fdma_dcb_type_t dcb_type;

    /**
     * <B>XTR:</B>\n
     *   Maintained by FDMA driver. Do not use. See also #alloc_ptr.
     * <B>INJ/AFI:</B>\n
     *   VIRTUAL ADDRESS. Must point to a pre-allocated area of act_len bytes. For SOF DCB,
     *   it must point to the IFH. Need only be byte-aligned. Set by user.
     */
    u8 *data;

    /**
     * <B>XTR/INJ/AFI:</B>\n
     *   VIRTUAL ADDRESS. This points to the first byte of the XTR/INJ IFH. Set by FDMA driver.
     *   This is useful for architectures with variable-length XTR/INJ headers
     *   or in cases where the reserved space in front of the packet is made
     *   such that the same data area can be used for both injection and
     *   extraction (VTSS_OPT_FDMA_VER >= 2).
     */
    u8 *ifh_ptr;

    /**
     * <B>XTR:</B>\n
     *   Set by FDMA driver.
     * <B>INJ:</B>\n
     *   Not used.
     * <B>AFI:</B>\n
     *   Internally used by the FDMA driver to hold various information.\n
     */
    u32 alloc_len;

    /**
     * <B>XTR:</B>\n
     *   The channel that this list belongs to. Filled in by FDMA
     *   driver and is supposed only to be used by FDMA driver.
     * <B>INJ/AFI:</B>\n
     *   Not used.
     */
    vtss_fdma_ch_t xtr_ch;

#if defined(VTSS_ARCH_LUTON28)
    /**
     * <B>XTR:</B>\n
     *   Not used.\n
     * <B>INJ:</B>\n
     *   The physical port number that this frame is sent to. Need only be set in SOF DCB.
     *
     * On architectures different from Luton28, the physical port(s) that this frame is
     * sent to are controlled through vtss_fdma_inj_props_t::port_mask and
     * vtss_fdma_inj_props_t::switch_frm members.
     */
    vtss_phys_port_no_t phys_port;
#endif /* defined(VTSS_ARCH_LUTON28) */

#if defined(VTSS_ARCH_JAGUAR_1)
    /**
     * <B>XTR:</B>\n
     *   Not used.\n
     * <B>INJ:</B>\n
     *   On 48-ported JR, we might have to inject a frame several
     *   times, if it is requested transmitted on several ports, of
     *   which at least one includes the secondary device, so we need
     *   a place to save off the remaining ports.
     */
    u64 dst_port_mask;
#endif

#if defined(VTSS_FEATURE_AFI_SWC)
    /**
     * <B>XTR/INJ:</B>\n
     *    Not used.\n
     * <B>AFI:</B>
     *   Indicates whether this is a switch-core-based periodically
     *   injected frame. If #afi_ids[0] == VTSS_AFI_ID_NONE, it's not
     *   AFI-based.
     *   The same frame may give rise to up to four AFI slots in order
     *   to get to the requested frame rate.
     */
    vtss_afi_id_t afi_ids[4];

    /**
     * <B>XTR/INJ:</B>\n
     *    Not used.\n
     * <B>AFI:</B>
     *   Holds the number of times this frame has been injected.
     *   It's an index into #afi_ids.
     */
    u32 afi_idx;
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_FDMA)
    /**
     * <B>XTR/INJ</B>\n
     *   Not used.
     * <B>AFI</B>\n
     *   Internal parameter. Holds the number of frames to increment #afi_frm_cnt by for
     *   each interrupt, when the AFI channel is counting.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u32 afi_frm_incr;

    /**
     * <B>XTR/INJ</B>\n
     *   Not used.
     * <B>AFI</B>\n
     *   Internal parameter. Holds the number of frames to increment #afi_frm_cnt by
     *   after an interrupt. The reason for this parameter is that the increment may
     *   change if the AFI channel in question will serve a different list of frames
     *   (because an existing is cancelled or a new frame is added) after the interrupt.
     *   The interrupt handler will then update #afi_frm_incr with the value of
     *   #afi_frm_incr_next.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u32 afi_frm_incr_next;

    /**
     * <B>XTR/INJ</B>\n
     *   Not used.
     * <B>AFI</B>\n
     *   Internal parameter. Temporary that holds the next number of repetitions
     *   of this frame. If all resources required are available, this number will
     *   be copied to #afi_frm_incr_next.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    u32 afi_frm_incr_temp;

    /**
     * <B>XTR/INJ:</B>\n
     *   Not used.
     * <B>AFI:</B>\n
     *   Internally used to link AFI frames while adding or deleting AFI frames from a
     *   given AFI-enabled channel. This is in order not to disturb the interrupt handler
     *   in this process, so that interrupts can be kept enabled while updating
     *   the new list of DCBs. Worst case, it takes around 150 ms to create a list with
     *   e.g. 300,000 frames per second that require frame counting and sequence numbering.
     *   It's unacceptable to have interrupts diabled (or the scheduler locked) for such
     *   a long time, since existing frames will not be counted and sequence-number updated then.
     *
     * Validity:
     *   Luton26: Y
     *   Luton28: N
     *   Jaguar1: Y
     *   Serval : Y (TBD)
     */
    vtss_fdma_list_t *afi_next;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    vtss_fdma_list_t user_dcb;
} fdma_sw_dcb_t;
#else
/* VTSS_OPT_FDMA_VER <= 2 */
typedef vtss_fdma_list_t fdma_sw_dcb_t;
#endif /* VTSS_OPT_FDMA_VER */

/*****************************************************************************/
// fdma_ch_status_t
/*****************************************************************************/
typedef enum {
    FDMA_CH_STATUS_DISABLED,
    FDMA_CH_STATUS_ENABLED,
    FDMA_CH_STATE_SUSPENDED,
} fdma_ch_status_t;

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_user_frame_list_t
// Structure for maintaining the AFI state for one AFI frequency.
// X such structures are put in an array and represents the frames that are
// currently requested to be injected on one GPDMA channel.
/*****************************************************************************/
typedef struct {
    // This AFI state's frequency, i.e. frames per second. 0 if unused.
    u32 fps;

    // Number of frames with this frequency.
    u32 frm_cnt;

    // NULL-terminated list of frames with this frequency.
    fdma_sw_dcb_t *frm_list;

} afi_user_frame_list_t;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_user_frames_t
// Structure for maintaining the AFI state for AFI_FREQ_LIST_LEN
// frequencies.
/*****************************************************************************/
typedef struct {
    // We can support up to AFI_FREQ_LIST_LEN different frequencies
    // on the same channel (all multiples of each other).
    afi_user_frame_list_t list[AFI_FREQ_LIST_LEN];

    // How much bandwidth are we currently using on this channel?
    u64 bw_consumption_bytes_per_second;

    // How many total frames in the list?
    u32 frm_cnt;

} afi_user_frames_t;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_dcb_list_t
// Structure for holding the properties of a (circular) list of DCBs.
/*****************************************************************************/
typedef struct {
    // A pointer to the head of the DCB list.
    fdma_sw_dcb_t *head;

    // Pointers to the two tails of the DCB list.
    // In case we're sequence numbering the frames, the circular
    // list is divided in two halves.
    //   tail[0] points to the end of the first half.
    //   tail[1] points to the end of the second half. The DCB itself points back to #head.
    // If the list is not divided into two halves,
    //   tail[0] is NULL.
    //   tail[1] points to the end of the circular list. The DCB itself points back to #head.
    fdma_sw_dcb_t *tail[2];

    // The number of DCBs in this list.
    u32 dcb_cnt;

    // Holds the value that this list fo DCBs require for the frame spacing timer.
    u32 timer_reload_value;

    // Pointer to a chunk of dynamically allocated memory that
    // contains the DCBs handed to the GPDMA and the frame data
    // of the currently running frames.
    // The amount of frame memory held in this pointer is
    // 0 if sequence numbering is not enabled for this set of
    // frames.
    void *dcb_and_frm_mem;

    // Indicates whether frame counting is enabled in this DCB list.
    BOOL enable_counting;

    // Indicates whether sequence numbering is enabled in this DCB list.
    BOOL enable_sequence_numbering;
} afi_dcb_list_t;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// fdma_afi_state_t
/*****************************************************************************/
typedef struct {

    // DCB list describing the set of frames that is currently being injected.
    // Used in DSR: Yes.
    afi_dcb_list_t cur;

    // DCB list describing the set of frames that will be changed to on the
    // next transfer-done interrupt.
    // Used in DSR: Yes.
    afi_dcb_list_t pend;

    // Holds a list of user frames that should be returned to the application once
    // the GPDMA has stopped injecting them (upon an afi_cancel).
    // Used in DSR: Yes.
    fdma_sw_dcb_t *tx_done_head;

    // Holds the maximum frequency quotient allowed on this channel.
    // Used in DSR: No.
    u32 quotient_max;

    // Maintains the user frames.
    // Used in DSR: Yes, it's read, but not written.
    //              The only (non-initialization) function that can write it is vtss_fdma_inj().
    //              The interrupt handler may modify the user frame DCBs held in the
    //              frm_list member, but not modify the frm_list itself.
    afi_user_frames_t user_frames;

    // Dummy DCB, initialized once during call to vtss_fdma_ch_cfg().
    // The dummy DCB is used as a signature for the real dummy DCBs required to
    // operate the channel.
    // Used in DSR: No.
    fdma_sw_dcb_t dummy_dcb;

} fdma_afi_state_t;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

/*****************************************************************************/
// fdma_throttle_state_t
/*****************************************************************************/
typedef struct {
    // The current throttle configuration.
    vtss_fdma_throttle_cfg_t cfg;

    // Everytime the FDMA driver has extracted a frame from a given
    // extraction queue (successfully as well as unsuccessfully), it
    // will increment this variable by one.
    // The counter is cleared upon every call to vtss_fdma_throttle_tick().
    u32 frm_cnt[VTSS_PACKET_RX_QUEUE_CNT];

    // Everytime the FDMA driver has extracted a frame from a given
    // extraction queue (successfully as well as unsuccessfully), it
    // will increment this variable by the number of bytes extracted.
    // The counter is cleared upon every call to vtss_fdma_throttle_tick().
    u32 byte_cnt[VTSS_PACKET_RX_QUEUE_CNT];

    // Counts the number of times vtss_fdma_throttle_tick() will have yet
    // to be invoked in order to re-open that queue for extraction.
    u32 ticks_left[VTSS_PACKET_RX_QUEUE_CNT];

    // Counts the number of times extraction queues have been turned off.
    u32 suspend_cnt[VTSS_PACKET_RX_QUEUE_CNT];

    // Holds the maximum number of frames seen in between two calls to vtss_fdma_throttle_tick().
    u32 statistics_max_frames_per_tick[VTSS_PACKET_RX_QUEUE_CNT];

    // Holds the maximum number of bytes seen in between two calls to vtss_fdma_throttle_tick().
    u32 statistics_max_bytes_per_tick[VTSS_PACKET_RX_QUEUE_CNT];

    // XTR: Number of times vtss_fdma_throttle_tick() has been invoked.
    //      The real thing won't start until it's greater than 1 (not
    //      just greater than 0).
    // INJ/AFI: Not used.
    u64 tick_cnt;
} fdma_throttle_state_t;

/*****************************************************************************/
// fdma_ch_state_t
/*****************************************************************************/
typedef struct {
    // Current use of the channel. Can be either of FDMA_CH_USAGE_xxx.
    vtss_fdma_ch_usage_t usage;

    // Current status of the channel. Can be either of FDMA_CH_STATE_xxx.
    fdma_ch_status_t status;

    // XTR: List of DCBs and frame data to which data can currently get extracted to, i.e. these are committed to the FDMA.
    // INJ: List of frames that are currently being injected.
    // AFI: Not used.
    fdma_sw_dcb_t *cur_head;

    // XTR: List of DCBs and frame data that are not yet committed to the FDMA. When the transfer done interrupt occurs, they do get committed. Unused for chip_no == 1.
    // INJ/AFI: Not used.
    fdma_sw_dcb_t *free_head;

    // XTR: Holds a pointer to the last item in the free_head list, i.e. the one whose next pointer is NULL. Unused for chip_no == 1.
    // INJ: Holds a pointer to the last item in the cur_head list, i.e. the one whose next pointer is NULL.
    // AFI: Not used.
    fdma_sw_dcb_t *cur_tail;

    // XTR: In case of jumbo frames (or frames whose size exceed the allocated), the pend_head holds the up-until-now-received fragments of the frame. Unused for chip_no == 1.
    // INJ/AFI: Not used.
    fdma_sw_dcb_t *pend_head;

#if defined(VTSS_ARCH_LUTON28)
    // XTR: Unused
    // INJ/AFI: The physical front port number that the channel is currently set-up to inject to.
    u32 phys_port;

    // XTR: Physical CPU Port Module queue number that this channel serves.
    // INJ/AFI: Not used
    u32 phys_qu;

    // XTR: Holds the configured start gap in words.
    // INJ/AFI: Not used
    int start_gap;

    // XTR: Holds the configured end gap in words.
    // INJ/AFI: Not used
    int end_gap;
#endif /* VTSS_ARCH_LUTON28 */

#if defined(VTSS_FEATURE_PACKET_GROUPING)
    // XTR: The extraction group that this channel serves.
    // INJ/AFI: Unused
    vtss_packet_rx_grp_t xtr_grp;

    // XTR: Unused
    // INJ/AFI: A mask indicating the injection groups that this channel can inject to.
    u32 inj_grp_mask;

    // XTR/INJ: Unused
    // AFI: Once an injection group is chosen, this will hold the chosen one.
    vtss_packet_tx_grp_t inj_grp;
#endif /* defined(VTSS_FEATURE_PACKET_GROUPING) */

    // XTR/INJ/AFI: Holds the channel priority.
    int prio;

#if VTSS_OPT_FDMA_VER <= 2
    // XTR: The per-channel callback function invoked when a new frame is ready.
    // INJ/AFI: Not used.
    vtss_fdma_list_t *(*xtr_cb)(void *cntxt, vtss_fdma_list_t *list, vtss_packet_rx_queue_t qu);
#endif /* VTSS_OPT_FDMA_VER <= 2 */

    // XTR: In dual-chip solutions: The chip (0 or 1) that this channel servers.
    // INJ/AFI: Not used.
    vtss_chip_no_t chip_no;

#if defined(VTSS_FEATURE_AFI_FDMA)
    fdma_afi_state_t afi_state;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
} fdma_ch_state_t;

#if defined(VTSS_ARCH_LUTON28)
/*****************************************************************************/
// fdma_port_state_t
/*****************************************************************************/
typedef struct {
    // On Luton28, frames may be Tx'd directly to the front ports rather than through
    // a shared queue system. Due to limited CPU bandwidth into these ports, software
    // has to check for overflow after the FDMA has injected the frame into the port,
    // but before the port is told to actually transmit it.
    // This way of handling frame Tx is not exhibited on other chips.

    // XTR: Not used
    // INJ: Configuration. This member is initialized by vtss_fdma_inj_flow_ctrl_cfg_set().
    //      It defaults to drop mode. When a port is in Tx flow control
    //      mode (DEV::FCCONF.FLOW_CONTROL_OBEY==1), frame injections
    //      to the port is a bit slower, because S/W has to check for
    //      overflow before writing the DEV::MISCFIFO.CPU_TX bit.
    //      In drop mode, the FDMA performs the writing to this register
    //      automatically. If - in flow control mode - the
    //      DEV::MISCSTAT.CPU_TX_DATA_OVERFLOW bit is set, the latest
    //      frame is rewound, and re-transmitted if configured to (see
    //      cfg_retransmit_cnt member).
    BOOL cfg_tx_flow_ctrl;

    // XTR: Not used
    // INJ: Configuration. Only used in the case where cfg_tx_flow_ctrl is 1.
    //      Controls the number of times a frame is attempted re-injected
    //      before giving up. Configured with vtss_fdma_inj_flow_ctrl_cfg_set().
    u8 cfg_retransmit_cnt;

    // XTR: Not used
    // INJ: State. Holds the number of times to yet retransmit the same frame
    //      in case of
    u8 state_retransmit_cnt;

    // XTR: Not used
    // INJ: Configuration. Set and cleared through a call to vtss_fdma_inj_suspend_port().
    //      Causes the FDMA driver code not to start anymore injections to this port
    //      as long as this bit is set. The user can call vtss_fdma_inj_port_active() to
    //      figure out whether an FDMA transfer to a port is ongoing.
    BOOL suspended;

    // XTR: Not used
    // INJ: State. Counts the number of channels that are currently attempting to
    //      inject to a given port. The counter cannot exceed the number of channels.
    u8 active_cnt;

} fdma_port_state_t;
#endif /* VTSS_ARCH_LUTON28 */

/*****************************************************************************/
// Needs forwards declaration, because it includes fdma_func_t, which needs
// a pointer to the state.
/*****************************************************************************/
struct vtss_state_s;

/*****************************************************************************/
// fdma_func_t
// Functions that must be implemented by an FDMA implementation.
/*****************************************************************************/
typedef struct {
    // Internal functions (called by AIL layer)
    vtss_rc (*fdma_init_conf_set)        (struct vtss_state_s *const vstate);
    void    (*fdma_xtr_qu_suspend_set)   (struct vtss_state_s *const vstate, vtss_packet_rx_queue_t xtr_qu, BOOL do_suspend);
    vtss_rc (*fdma_xtr_dcb_init)         (struct vtss_state_s *const vstate, vtss_fdma_ch_t xtr_ch, fdma_sw_dcb_t *list, u32 cfg_alloc_len);
    void    (*fdma_xtr_connect)          (struct vtss_state_s *const vstate, fdma_hw_dcb_t *tail, fdma_hw_dcb_t *more);
    BOOL    (*fdma_xtr_restart_ch)       (struct vtss_state_s *const vstate, vtss_fdma_ch_t xtr_ch); // Returns TRUE if channel has been restarted.
    vtss_rc (*fdma_inj_restart_ch)       (struct vtss_state_s *const vstate, vtss_fdma_ch_t inj_ch, fdma_sw_dcb_t *head, BOOL do_start, BOOL thread_safe);
#if VTSS_OPT_FDMA_VER >= 3
    void    (*fdma_start_ch)             (struct vtss_state_s *const vstate, vtss_fdma_ch_t ch);
#endif /* VTSS_OPT_FDMA_VER >= 3 */

    // External functions (called by Application (directly through AIL layer)
    vtss_rc (*fdma_uninit)               (struct vtss_state_s *const vstate);
    vtss_rc (*fdma_stats_clr)            (struct vtss_state_s *const vstate);
    vtss_rc (*fdma_debug_print)          (struct vtss_state_s *const vstate, const vtss_debug_printf_t pr, const vtss_debug_info_t *const info);
    vtss_rc (*fdma_irq_handler)          (struct vtss_state_s *const vstate, void *const cntxt);
#if defined(VTSS_FEATURE_AFI_FDMA) && VTSS_OPT_FDMA_VER >= 3
    vtss_rc (*fdma_afi_frm_cnt)          (struct vtss_state_s *const vstate, const u8 *const frm_ptr, u64 *const frm_cnt);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) && VTSS_OPT_FDMA_VER >= 3 */

#if defined(VTSS_FEATURE_AFI_SWC)
    // Internal function for pausing/resuming frames sent through the switch core's AFI.
    vtss_rc (*fdma_afi_pause_resume)     (struct vtss_state_s *const vstate, vtss_afi_id_t id, BOOL resume);
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

#if VTSS_OPT_FDMA_VER <= 2
    vtss_rc (*fdma_inj)                  (struct vtss_state_s *const vstate, vtss_fdma_list_t *list, const vtss_fdma_ch_t ch, const u32 len, const vtss_fdma_inj_props_t *const props);
#else
    vtss_rc (*fdma_tx)                   (struct vtss_state_s *const vstate, fdma_sw_dcb_t *list, vtss_fdma_tx_info_t *const fdma_info, vtss_packet_tx_info_t *const tx_info, BOOL afi_cancel);
#endif /* VTSS_OPT_FDMA_VER */

    // Older external functions
#if VTSS_OPT_FDMA_VER <= 2
    vtss_rc (*fdma_xtr_ch_from_list)     (struct vtss_state_s *const vstate, const vtss_fdma_list_t *const list, vtss_fdma_ch_t *const ch);
    vtss_rc (*fdma_xtr_hdr_decode)       (struct vtss_state_s *const vstate, const vtss_fdma_list_t *const list, vtss_fdma_xtr_props_t *const xtr_props);
    vtss_rc (*fdma_ch_cfg)               (struct vtss_state_s *const vstate, const vtss_fdma_ch_t ch, const vtss_fdma_ch_cfg_t *const cfg);
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON28)
#if VTSS_OPT_FDMA_VER == 1
    vtss_rc (*fdma_xtr_cfg)              (struct vtss_state_s *const vstate, const BOOL ena, const vtss_fdma_ch_t ch, const vtss_packet_rx_queue_t qu, const int start_gap, const int end_gap, vtss_fdma_list_t *const list, vtss_fdma_list_t *(*xtr_cb)(void *cntxt, vtss_fdma_list_t *list, vtss_packet_rx_queue_t qu));
#elif VTSS_OPT_FDMA_VER == 2
    vtss_rc (*fdma_xtr_cfg)              (struct vtss_state_s *const vstate, const BOOL ena, const vtss_fdma_ch_t ch, const vtss_packet_rx_queue_t qu, vtss_fdma_list_t *const list, vtss_fdma_list_t *(*xtr_cb)(void *cntxt, vtss_fdma_list_t *list, vtss_packet_rx_queue_t qu));
#endif /* VTSS_OPT_FDMA_VER */

#if VTSS_OPT_FDMA_VER <= 2
    vtss_rc (*fdma_stats_get)            (struct vtss_state_s *const vstate, vtss_fdma_stats_t *const stats);
    vtss_rc (*fdma_inj_flow_ctrl_cfg_get)(struct vtss_state_s *const vstate, const vtss_phys_port_no_t phys_port, BOOL *const in_tx_flow_ctrl, u8 *const retransmit_cnt);
    vtss_rc (*fdma_inj_flow_ctrl_cfg_set)(struct vtss_state_s *const vstate, const vtss_phys_port_no_t phys_port, const BOOL in_tx_flow_ctrl, const u8 retransmit_cnt);
    vtss_rc (*fdma_inj_cfg)              (struct vtss_state_s *const vstate, const BOOL ena, const vtss_fdma_ch_t ch);
#endif /* VTSS_OPT_FDMA_VER <= 2 */

    vtss_rc (*fdma_inj_suspend_port)     (struct vtss_state_s *const vstate, const vtss_phys_port_no_t phys_port, const BOOL suspend);
    vtss_rc (*fdma_inj_port_active)      (struct vtss_state_s *const vstate, const vtss_phys_port_no_t phys_port, BOOL *const active);
#endif /* VTSS_ARCH_LUTON28 */
} fdma_func_t;

// Internal debug statistics.
typedef struct
{
    u32 dcbs_used[VTSS_FDMA_CH_CNT];                 /**< Number of software DCBs used per FDMA channel.                                */
    u32 dcbs_added[VTSS_FDMA_CH_CNT];                /**< Number of software DCBs added per FDMA channel                                */
    u32 packets[VTSS_FDMA_CH_CNT];                   /**< Number of packets Rx'd or Tx'd per FDMA channel.                              */
    u64 bytes[VTSS_FDMA_CH_CNT];                     /**< Number of bytes Rx'd or Tx'd per FDMA channel.                                */
    u32 ch_intr_cnt[VTSS_FDMA_CH_CNT];               /**< Number of interrupts per FDMA channel.                                        */
    u32 xtr_qu_packets[VTSS_PACKET_RX_QUEUE_CNT][2]; /**< Number of packets received on a given extraction queue, per chip.             */
    u32 xtr_qu_drops[VTSS_PACKET_RX_QUEUE_CNT][2];   /**< Number of packets dropped by the FDMA driver, per chip.                       */
    u32 intr_cnt;                                    /**< Number of interrupts (calls to vtss_fdma_irq_handler()).                      */
#if VTSS_OPT_FDMA_VER >= 3
    u32 rx_multiple_dcb_drops;                       /**< Number of rx-dropped packets because they span multiple DCBs.                 */
    u32 rx_jumbo_drops;                              /**< Number of rx-dropped packets because they are longer than the configured MTU. */
    u32 rx_vlan_tag_mismatch;                        /**< Number of rx-dropped packets because the tag didn't match the port's setup.   */
#endif /* VTSS_OPT_FDMA_VER */
} fdma_internal_statistics_t;

/*****************************************************************************/
// fdma_state_t
// The overall FDMA state held in one single structure, so that all functions
// can get this one passed. This allows for not protecting the state from
// the outside via an API semaphore, which is nice, since we also define
// an interrupt handler, which possibly can't wait for the semaphore.
// Internally, we assure mutual exclusion by using the
// VTSS_OS_INTERRUPT_DISABLE()/RESTORE() or VTSS_OS_SCHEDULER_LOCK()/UNLOCK()
// macros.
/*****************************************************************************/
typedef struct {
    fdma_func_t       fdma_func; // Pointers to chip-specific functions
    fdma_ch_state_t   fdma_ch_state[VTSS_FDMA_CH_CNT];
    vtss_fdma_cfg_t   fdma_cfg;  // General FDMA configuration

    // These are chip-specific and set during XXX_fdma_init_conf_set().
    u32 hdr_size, xtr_hdr_size, inj_hdr_size, xtr_burst_size_bytes, xtr_ch_cnt;

#if defined(VTSS_FEATURE_AFI_SWC)
    // Frames sent through AFI.
    fdma_sw_dcb_t *afi_frames[VTSS_AFI_SLOT_CNT];
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

#if VTSS_OPT_FDMA_VER >= 3
    BOOL enabled;

    // This is what came out of VTSS_OS_MALLOC() when allocating extraction DCBs
    // and possible frame data. It's the pointer that must be used directly
    // in the call to VTSS_OS_FREE().
    void *xtr_mem;

    // This is what came out of VTSS_OS_MALLOC() when allocating injection DCBs.
    // It's the pointer that must be used directly in the call to VTSS_OS_FREE().
    void *inj_mem;

    // List of free injection DCBs
    fdma_sw_dcb_t *inj_dcbs;

#if defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC)
    // This is what came out of VTSS_OS_MALLOC() when allocating AFI DCBs.
    // It's the pointer that must be used directly in the call to VTSS_OS_FREE().
    void *afi_mem;

    // List of free AFI DCBs.
    fdma_sw_dcb_t *afi_dcbs;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || defined(VTSS_FEATURE_AFI_SWC) */

    // The number of bytes required for frame and meta data - per frame.
    u32 bytes_per_frm;
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if defined(VTSS_FEATURE_VSTAX)
    // For dual-chip (48-port) solutions, we need to be able to manually extract
    // frames from the secondary chip, because we cannot forward them automatically
    // to the primary chip without losing the CPU queue number. Therefore, the
    // JR API file must provide us with a function that can be called in order to
    // do this read.
    vtss_rc (*rx_frame_get_internal)(const struct vtss_state_s *const state,
                                     vtss_chip_no_t            chip_no,
                                     vtss_packet_rx_grp_t      grp,
                                     u8                        *const ifh,
                                     u8                        *const frame,
                                     const u32                 buf_length,
                                     u32                       *frm_length);

    // For dual-chip (48-port) solutions, we need to be able to discard frames
    // from the secondary chip if we're out of buffers.
    vtss_rc (*rx_frame_discard_internal)(const struct vtss_state_s *const state,
                                         vtss_chip_no_t            chip_no,
                                         vtss_packet_rx_grp_t      grp,
                                         u32                       *discarded_bytes);
#endif /* defined(VTSS_FEATURE_VSTAX) */

#if defined(VTSS_ARCH_JAGUAR_1) && VTSS_OPT_FDMA_VER >= 3
    // We need a special version of cil_func->tx_hdr_encode() which can
    // also provide us with the injection group.
    vtss_rc (*tx_hdr_encode_internal)(const struct vtss_state_s   *const state,
                                      const vtss_packet_tx_info_t *const info,
                                            u8                    *const bin_hdr,
                                            u32                   *const bin_hdr_len,
                                            vtss_packet_tx_grp_t  *const inj_grp);
#endif /* defined(VTSS_ARCH_JAGUAR_1) && VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
    // Mirroring supported through vtss_packet_tx_hdr_encode() if VTSS_OPT_FDMA_VER >= 3
    u64            mirror_port_mask;
    vtss_port_no_t mirror_port_no;
#endif /* VTSS_OPT_FDMA_VER <= 2 */

    // XTR: Used to do S/W policing of extraction queues.
    // INJ/AFI: Not used.
    fdma_throttle_state_t throttle;

#if defined(VTSS_FEATURE_AFI_FDMA)
    // Gotta specify the AHB clock period in picoseconds to avoid decimals.
    u32 ahb_clk_period_ps;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_ARCH_LUTON28)
    // We only per-port state on Luton28.
    fdma_port_state_t fdma_port_state[VTSS_FDMA_TOTAL_PORT_CNT];
#endif /* VTSS_ARCH_LUTON28 */

#if defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2
    // Lu28 statistics is special in earlier FDMA versions */
    vtss_fdma_stats_t fdma_stats;
#endif /* defined(VTSS_ARCH_LUTON28) && VTSS_OPT_FDMA_VER <= 2 */

#if !defined(VTSS_ARCH_LUTON28) || VTSS_OPT_FDMA_VER >= 3
    // On later FDMA API versions, we use the same statistics
    // on all platforms.
    // We only have internal statistics, which are printable
    // only through a debug print function called
    // vtss_debug_info_print(print_func, info), where
    //   @info.layer is set to 'cil' and
    //   @info.group is set to VTSS_DEBUG_GROUP_FDMA
    fdma_internal_statistics_t fdma_stats;
#endif /* !defined(VTSS_ARCH_LUTON28) || VTSS_OPT_FDMA_VER >= 3 */
} fdma_state_t;

// When filling in or extracting info from DCBs,
// we need to do it to or from the native bus's endianness,
// which is little endian. Whether we have to swap
// the bytes or not depends on the CPU's endianness.
#if defined(VTSS_OS_BIG_ENDIAN)
static inline u32 CPU_TO_BUS(u32 v)
{
    register u32 v1 = v;
    v1 = ((v1 >> 24) & 0x000000FF) | ((v1 >> 8) & 0x0000FF00) | ((v1 << 8) & 0x00FF0000) | ((v1 << 24) & 0xFF000000);
    return v1;
}
#else
    #define CPU_TO_BUS(v) (v)
#endif /* defined(VTSS_OS_BIG_ENDIAN) */

#define BUS_TO_CPU(v) CPU_TO_BUS(v)

// Select between IRQ or DSR context
#if VTSS_OPT_FDMA_IRQ_CONTEXT
    /* We're called in deferred interrupt context */
    #define FDMA_INTERRUPT_FLAGS          VTSS_OS_INTERRUPT_FLAGS
    #define FDMA_INTERRUPT_DISABLE(flags) VTSS_OS_INTERRUPT_DISABLE(flags)
    #define FDMA_INTERRUPT_RESTORE(flags) VTSS_OS_INTERRUPT_RESTORE(flags)
#else
    /* We're called directly in interrupt context */
    #define FDMA_INTERRUPT_FLAGS          VTSS_OS_SCHEDULER_FLAGS
    #define FDMA_INTERRUPT_DISABLE(flags) VTSS_OS_SCHEDULER_LOCK(flags)
    #define FDMA_INTERRUPT_RESTORE(flags) VTSS_OS_SCHEDULER_UNLOCK(flags)
#endif /* VTSS_OPT_FDMA_IRQ_CONTEXT */

#define SIZE_OF_TWO_MAC_ADDRS (2 * 6)

#endif /* defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA */
#endif /* _VTSS_FDMA_H_ */
/*****************************************************************************/
//
// End of file
//
//****************************************************************************/

