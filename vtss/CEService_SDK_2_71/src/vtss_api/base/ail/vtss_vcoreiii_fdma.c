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
// vtss_vcoreiii_fdma.c
//
//*****************************************************************************/

// Do not use the VTSS_IOREG() macro defined in vtss_luton26_regs_common.h,
// because that will dereference the register address. And since we in
// at least one case (VTSS_ICPU_CFG_GPDMA_FDMA_XTR_STAT_LAST_DCB) need the
// address itself and not the value stored on that address, we need to
// override VTSS_IOREG() to return the address and not the value.
// #define VTSS_IOREG(t,o) (VTSS_IOADDR(t,o))

#define VTSS_TRACE_LAYER VTSS_TRACE_LAYER_CIL

// All register accesses within this file happen through the init_conf->reg_read/write() callout,
// which requires a relative 32-bit address, but in one occassion we need to present a physical
// address to the H/W, so here is the macro that does that.
#define FDMA_TO_PHYS(r) (((r) << 2) + VTSS_IO_ORIGIN1_OFFSET)

// Avoid "vtss_api.h not used in module vtss_vcoreiii_fdma.c"
/*lint --e{766} */
#include "vtss_api.h"
#if defined(VTSS_OPT_VCORE_III) && VTSS_OPT_FDMA && (defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_JAGUAR_1))
#include "vtss_state.h"
#include "vtss_fdma.h"
#include "vtss_fdma_common.h"

/* Use relative addresses for registers - this is a mess! */
#undef VTSS_IOADDR
#undef VTSS_IOREG
#undef VTSS_IO_OFFSET1
#undef VTSS_IO_OFFSET2
#define VTSS_IOADDR(t,o) ((((t) - VTSS_IO_ORIGIN1_OFFSET) >> 2) + (o))
#define VTSS_IOREG(t,o)  (VTSS_IOADDR(t,o))
#define VTSS_IO_OFFSET1(offset) (VTSS_IO_ORIGIN1_OFFSET + offset)
#define VTSS_IO_OFFSET2(offset) (VTSS_IO_ORIGIN2_OFFSET + offset)

#if defined(VTSS_OPSYS_ECOS)
#elif defined(VTSS_OPSYS_LINUX)
  #include <asm-mips/mach-vcoreiii/hardware.h>
  #if defined(VTSS_ARCH_LUTON26)
    #include "vtss_luton26_regs.h"
  #endif /* VTSS_ARCH_LUTON26 */
  #if defined(VTSS_ARCH_JAGUAR_1)
    #include "vtss_jaguar_regs.h"
  #endif /* VTSS_ARCH_JAGUAR_1 */
#else
  #error "Operating system not supported".
#endif /* VTSS_OPSYS_xxx */

#include "vtss_common.h" /* For vtss_cmn_debug_reg_check_set() */

#if defined(VTSS_ARCH_LUTON26)
  #define VTSS_FDMA_XTR_HDR_SIZE_BYTES  8 // Number of bytes in the extraction header.
  #define VTSS_FDMA_INJ_HDR_SIZE_BYTES 16 // Number of bytes in the injection header (always at least 8 for IFH, @ptp_action != 0 => +4 for timestamp. @switch_frm == TRUE => +4 for VLAN tag)
#elif defined(VTSS_ARCH_JAGUAR_1)
  #define VTSS_FDMA_XTR_HDR_SIZE_BYTES 28 // Number of bytes in the extraction header (incl. 4 bytes queue number).
  #define VTSS_FDMA_INJ_HDR_SIZE_BYTES 24 // Number of bytes in the injection header.
#else
  #error "Not compilable for this platform"
#endif /* VTSS_ARCH_xxx */

// We don't operate with assertions in the API, so we print an error and gracefully return.
// Note: This macro is sometimes used with code that MUST be executed - even on non-debug builds -,
// so if you don't like to have VTSS_EG() calls, then rewrite the macro to avoid that call.
#define FDMA_ASSERT(grp, expr, code) do {if (!(expr)) {VTSS_EG((VTSS_TRACE_GROUP_FDMA_ ## grp), "Assert failed: " # expr); code}} while (0);

#define DW_DMAC_VER 0x3231342a

// The FDMA overrides the usage of the DCB->dar field in the injection case
#define FDMA_DAR_CHUNK_SIZE(x)   VTSS_ENCODE_BITFIELD((x), 16, 16)
#define FDMA_DAR_INJ_GRP(x)      VTSS_ENCODE_BITFIELD((x),  8,  3)
#define FDMA_DAR_SAR_OFFSET(x)   VTSS_ENCODE_BITFIELD((x),  4,  2)
#define FDMA_DAR_EOF(x)          VTSS_ENCODE_BITFIELD((x),  3,  1)
#define FDMA_DAR_SOF(x)          VTSS_ENCODE_BITFIELD((x),  2,  1)
#define FDMA_X_DAR_CHUNK_SIZE(x) VTSS_EXTRACT_BITFIELD((x), 16, 16)

// The FDMA overrides the usage of the DCB->sar field in the extraction case
#define FDMA_SAR_CHUNK_SIZE(x) VTSS_ENCODE_BITFIELD((x), 16, 16)
#define FDMA_SAR_CH_ID(x)      VTSS_ENCODE_BITFIELD((x), FDMA_F_SAR_CH_ID_FPOS, FDMA_F_SAR_CH_ID_FLEN) /* This is actually a reserved field in the DCB, but we can safely use it to store the channel ID for later retrieval with vcoreiii_fdma_xtr_ch_from_list() */

// Source and destination burst transaction lengths for extraction and injection, respectively.
#define XTR_SRC_MSIZE 2
#define XTR_DST_MSIZE 2
#define XTR_FIFOMODE  0
#define INJ_SRC_MSIZE 2
#define INJ_DST_MSIZE 1
#define INJ_FIFOMODE  0

#define FDMA_F_SAR_CH_ID_FPOS 2
#define FDMA_F_SAR_CH_ID_FLEN 3

#define VTSS_FDMA_AFI_CH_MAX 4 /* AFI can only run on channels 0-3, since the GPDMA needs to support locking. */

// This define determines the length of an AFI chain in terms of milliseconds when frame counting is enabled.
// In other words: It determines how often do we want interrupts to update AFI frame counters.
#define VTSS_FDMA_AFI_MINIMUM_INTERRUPT_PERIOD_MS 50ULL

#define FDMA_INLINE inline

#if defined(VTSS_FEATURE_AFI_FDMA)

  // Initialize the special overloaded fields
  #define AFI_DCB_INIT_SPECIAL_FIELDS(dcb) {(dcb)->alloc_len = 0;}

  // We use an unused field in the DCBs to indicate that counting is
  // enabled for a given frame.
  #define AFI_COUNTING_ENABLED(dcb)      (((dcb)->alloc_len & 0x80000000) ? 1 : 0)
  #define AFI_COUNTING_ENA_SET(dcb, ena) {if (ena) {(dcb)->alloc_len |= 0x80000000;} else {(dcb)->alloc_len &= ~0x80000000UL;}}

  // We use an unused field in the DCBs to indicate that sequence
  // numbering is enabled for a given frame.
  #define AFI_SEQ_NUM_ENABLED(dcb)       (((dcb)->alloc_len & 0x40000000) ? 1 : 0)
  #define AFI_SEQ_NUM_ENA_SET(dcb, ena)  {if (ena) {(dcb)->alloc_len |= 0x40000000;} else {(dcb)->alloc_len &= ~0x40000000UL;}}

  // We use an unused field in the DCBs to indicate the number of frames per
  // second in the DCBs handed to the GPDMA (not in the user-frame-DCBs).
  #define AFI_FPS_GET(dcb)               ((dcb)->alloc_len & 0x3FFFFFFF)
  #define AFI_FPS_SET(dcb, fps)          {(dcb)->alloc_len &= ~0x3FFFFFFFUL; (dcb)->alloc_len |= (fps & 0x3FFFFFFF);}

  // If FPS == 0, it's a dummy DCB
  #define AFI_DCB_IS_DUMMY(dcb)          (AFI_FPS_GET(dcb) == 0 ? TRUE : FALSE)

  // Round a number up to being the next multiple of 4.
  #define AFI_CEIL4(len) ((((len) + 3) / 4) * 4)
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#define VTSS_FDMA_CH_INJ 4

// Try to get around several FDMA API versions, naming-wise.
#if defined(VTSS_FEATURE_AFI_FDMA)
#if VTSS_OPT_FDMA_VER <= 2
#define AFI_CH_AUTO                          VTSS_FDMA_CCM_CH_AUTO
#define AFI_SEQ_NUMBER(dcb)                  dcb->ccm_seq_number
#define AFI_FRM_CNT(dcb)                     dcb->ccm_frm_cnt
#define AFI_FPS(props)                       props->ccm_fps
#define AFI_ENABLE_COUNTING(props)           props->ccm_enable_counting
#define AFI_ENABLE_SEQUENCE_NUMBERING(props) props->ccm_enable_sequence_numbering
#define AFI_SEQUENCE_NUMBER_OFFSET(props)    props->ccm_sequence_number_offset
#else
#define AFI_CH_AUTO                          -1
#define AFI_SEQ_NUMBER(dcb)                  dcb->afi_seq_number
#define AFI_FRM_CNT(dcb)                     dcb->afi_frm_cnt
#define AFI_FPS(props)                       props->afi_fps
#define AFI_ENABLE_COUNTING(props)           props->afi_enable_counting
#define AFI_ENABLE_SEQUENCE_NUMBERING(props) props->afi_enable_sequence_numbering
#define AFI_SEQUENCE_NUMBER_OFFSET(props)    props->afi_sequence_number_offset
#endif /* VTSS_OPT_FDMA_VER */
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

// Avoid "unprotected access to vtss_trace_conf" Lint warnings
/*lint -sem(vcoreiii_fdma_ch_cfg,              thread_protected) */
/*lint -sem(vcoreiii_fdma_inj,                 thread_protected) */
/*lint -sem(vcoreiii_fdma_irq_handler,         thread_protected) */
/*lint -sem(vcoreiii_fdma_xtr_ch_from_list,    thread_protected) */
/*lint -sem(vcoreiii_fdma_xtr_hdr_decode,      thread_protected) */
/*lint -sem(jr_reinject_done,                  thread_protected) */

/*****************************************************************************/
// Read target register using current CPU interface
/*****************************************************************************/
static FDMA_INLINE u32 VCOREIII_RD(vtss_state_t *const vstate, u32 reg)
{
    u32 value;
    FDMA_ASSERT(IRQ, vstate->init_conf.reg_read(0, reg, &value) == VTSS_RC_OK, return 0;);
    return value;
}

/*****************************************************************************/
// Write target register using current CPU interface
/*****************************************************************************/
static FDMA_INLINE void VCOREIII_WR(vtss_state_t *const vstate, u32 reg, u32 value)
{
    FDMA_ASSERT(IRQ, vstate->init_conf.reg_write(0, reg, value) == VTSS_RC_OK, ;);
}

/*****************************************************************************/
// check_version()
// Returns: 0 on failure to support the current silicon, 1 on success.
/*****************************************************************************/
static FDMA_INLINE int check_version(vtss_state_t *const vstate)
{
    return VCOREIII_RD(vstate, VTSS_FDMA_MISC_DMA_COMP_VERSION) == DW_DMAC_VER;
}

/*****************************************************************************/
// enable_ch()
/*****************************************************************************/
static FDMA_INLINE void enable_ch(vtss_state_t *const vstate, vtss_fdma_ch_t ch)
{
    // Create a memory barrier here, so that the compiler doesn't enable the channel
    // before it's configured properly (needed because this function is inline).
    VTSS_OS_REORDER_BARRIER();
    VCOREIII_WR(vstate, VTSS_FDMA_MISC_CH_EN_REG, VTSS_F_FDMA_MISC_CH_EN_REG_CH_EN_WE(VTSS_BIT(ch)) | VTSS_F_FDMA_MISC_CH_EN_REG_CH_EN(VTSS_BIT(ch))); // CH_EN_WE = CH_EN = 1
}

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// ch_enabled()
/*****************************************************************************/
static FDMA_INLINE BOOL ch_enabled(vtss_state_t *const vstate, vtss_fdma_ch_t ch)
{
    return (VTSS_X_FDMA_MISC_CH_EN_REG_CH_EN(VCOREIII_RD(vstate, VTSS_FDMA_MISC_CH_EN_REG)) & VTSS_BIT(ch)) ? TRUE : FALSE;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA) || VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// disable_ch()
/*****************************************************************************/
static FDMA_INLINE void disable_ch(vtss_state_t *const vstate, vtss_fdma_ch_t ch)
{
    VCOREIII_WR(vstate, VTSS_FDMA_MISC_CH_EN_REG, VTSS_F_FDMA_MISC_CH_EN_REG_CH_EN_WE(VTSS_BIT(ch))); // CH_EN_WE = 1, CH_EN = 0
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) || VTSS_OPT_FDMA_VER <= 2 */

/*****************************************************************************/
// disable_chs()
/*****************************************************************************/
static FDMA_INLINE void disable_chs(vtss_state_t *const vstate, int ch_mask)
{
    VCOREIII_WR(vstate, VTSS_FDMA_MISC_CH_EN_REG, VTSS_F_FDMA_MISC_CH_EN_REG_CH_EN_WE(ch_mask)); // CH_EN_WE = mask, CH_EN = 0
}

/*****************************************************************************/
// enable_block_done_intr()
/*****************************************************************************/
static FDMA_INLINE void enable_block_done_intr(vtss_state_t *const vstate, vtss_fdma_ch_t ch)
{
    VTSS_OS_REORDER_BARRIER();
    VCOREIII_WR(vstate, VTSS_FDMA_INTR_MASK_BLOCK, VTSS_F_FDMA_INTR_MASK_BLOCK_INT_MASK_WE_BLOCK(VTSS_BIT(ch)) | VTSS_F_FDMA_INTR_MASK_BLOCK_INT_MASK_BLOCK(VTSS_BIT(ch)));
}

/*****************************************************************************/
// enable_tfr_done_intr()
/*****************************************************************************/
static FDMA_INLINE void enable_tfr_done_intr(vtss_state_t *const vstate, vtss_fdma_ch_t ch)
{
    VTSS_OS_REORDER_BARRIER();
    VCOREIII_WR(vstate, VTSS_FDMA_INTR_MASK_TFR, VTSS_F_FDMA_INTR_MASK_TFR_INT_MASK_WE_TFR(VTSS_BIT(ch)) | VTSS_F_FDMA_INTR_MASK_TFR_INT_MASK_TFR(VTSS_BIT(ch)));
}

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// disable_block_done_intr()
/*****************************************************************************/
static FDMA_INLINE void disable_block_done_intr(vtss_state_t *const vstate, vtss_fdma_ch_t ch)
{
    VTSS_OS_REORDER_BARRIER();
    VCOREIII_WR(vstate, VTSS_FDMA_INTR_MASK_BLOCK, VTSS_F_FDMA_INTR_MASK_BLOCK_INT_MASK_WE_BLOCK(VTSS_BIT(ch)));
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// disable_tfr_done_intr()
/*****************************************************************************/
static FDMA_INLINE void disable_tfr_done_intr(vtss_state_t *const vstate, vtss_fdma_ch_t ch)
{
    VTSS_OS_REORDER_BARRIER();
    VCOREIII_WR(vstate, VTSS_FDMA_INTR_MASK_TFR, VTSS_F_FDMA_INTR_MASK_TFR_INT_MASK_WE_TFR(VTSS_BIT(ch)));
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/*****************************************************************************/
// clear_pending_block_done_intr()
/*****************************************************************************/
static FDMA_INLINE void clear_pending_block_done_intr(vtss_state_t *const vstate, int mask)
{
    VCOREIII_WR(vstate, VTSS_FDMA_INTR_CLEAR_BLOCK, VTSS_F_FDMA_INTR_CLEAR_BLOCK_CLEAR_BLOCK(mask));
}

/*****************************************************************************/
// clear_pending_tfr_done_intr()
/*****************************************************************************/
static FDMA_INLINE void clear_pending_tfr_done_intr(vtss_state_t *const vstate, int mask)
{
    VCOREIII_WR(vstate, VTSS_FDMA_INTR_CLEAR_TFR, VTSS_F_FDMA_INTR_CLEAR_TFR_CLEAR_TFR(mask));
}

/*****************************************************************************/
// vcoreiii_fdma_xtr_dcb_init()
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_xtr_dcb_init(vtss_state_t *const vstate, vtss_fdma_ch_t xtr_ch, fdma_sw_dcb_t *list, u32 cfg_alloc_len)
{
    fdma_hw_dcb_t *hw_dcb = FDMA_HW_DCB(list);

    // Initialize the DCB area
    hw_dcb->sar  = CPU_TO_BUS(FDMA_SAR_CHUNK_SIZE(cfg_alloc_len) | FDMA_SAR_CH_ID(xtr_ch));
    hw_dcb->dar  = CPU_TO_BUS(VTSS_OS_VIRT_TO_PHYS(list->ifh_ptr));
    hw_dcb->llp  = CPU_TO_BUS(FDMA_USER_DCB(list)->next ? VTSS_OS_VIRT_TO_PHYS(FDMA_HW_DCB(FDMA_SW_DCB(FDMA_USER_DCB(list)->next))) : 0);
    hw_dcb->ctl0 = CPU_TO_BUS(VTSS_F_FDMA_CH_CTL0_LLP_SRC_EN | VTSS_F_FDMA_CH_CTL0_LLP_DST_EN               | VTSS_F_FDMA_CH_CTL0_SMS(1)                    |
                              VTSS_F_FDMA_CH_CTL0_TT_FC(4)   | VTSS_F_FDMA_CH_CTL0_SRC_MSIZE(XTR_SRC_MSIZE) | VTSS_F_FDMA_CH_CTL0_DEST_MSIZE(XTR_DST_MSIZE) |
                              VTSS_F_FDMA_CH_CTL0_SINC(2)    | VTSS_F_FDMA_CH_CTL0_SRC_TR_WIDTH(2)          | VTSS_F_FDMA_CH_CTL0_DST_TR_WIDTH(2)           | VTSS_F_FDMA_CH_CTL0_INT_EN);
    hw_dcb->ctl1 = hw_dcb->stat = 0;

    VTSS_OS_REORDER_BARRIER();

    // Flush the dcache, so that the DCB gets written to main memory.
    VTSS_OS_DCACHE_FLUSH(hw_dcb, VTSS_FDMA_DCB_SIZE_BYTES);

    return VTSS_RC_OK;
}

/*****************************************************************************/
// vcoreiii_fdma_xtr_connect()
// This function must be called with interrupts disabled, so that no context
// switches can occur, since it manipulates list pointers that are used in
// IRQ (DSR) context.
/*****************************************************************************/
static void vcoreiii_fdma_xtr_connect(vtss_state_t *const vstate, fdma_hw_dcb_t *tail, fdma_hw_dcb_t *more)
{
    tail->llp = CPU_TO_BUS(VTSS_OS_VIRT_TO_PHYS(more));
    VTSS_OS_REORDER_BARRIER();

    // Flush the dcache, so that the previous tail's DCB's LLP gets written to main memory.
    VTSS_OS_DCACHE_FLUSH(&tail->llp, 4);
}

/*****************************************************************************/
// vcoreiii_fdma_xtr_restart_ch()
// This function must be called with interrupts disabled, so that no context
// switches can occur, since it manipulates list pointers that are used in
// IRQ (DSR) context.
/*****************************************************************************/
static BOOL vcoreiii_fdma_xtr_restart_ch(vtss_state_t *vstate, vtss_fdma_ch_t xtr_ch)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[xtr_ch];

    if (ch_state->chip_no == 1) {
        // Manual extraction, so there's nothing to restart. Pretend that we have restarted.
        return TRUE;
    }

    if (ch_state->cur_head) {
        // As long as we haven't yet used all DCBs, don't restart.
        return FALSE;
    }

    if ((VCOREIII_RD(vstate, VTSS_FDMA_MISC_CH_EN_REG) & VTSS_F_FDMA_MISC_CH_EN_REG_CH_EN(VTSS_BIT(xtr_ch))) != 0) {
        // Channel not disabled yet.
        return FALSE;
    }

    // Re-enable the channel
    VCOREIII_WR(vstate, VTSS_FDMA_CH_LLP(xtr_ch), VTSS_OS_VIRT_TO_PHYS(FDMA_HW_DCB(ch_state->free_head)));
    enable_ch(vstate, xtr_ch);
    return TRUE;
}

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_insert_vlan_tag()
// #ptr points four bytes ahead of current DMAC.
// #tpid is ethertype.
// #tci is the Tag Control Identifier, i.e. the combined PCP/CFI/VID.
/*****************************************************************************/
static void inj_insert_vlan_tag(u8 *ptr, vtss_etype_t tpid, u16 tci)
{
    int i;

    // DMAC & SMAC four bytes ahead (memcpy potentially fails due to overlapping regions):
    for (i = 0; i < SIZE_OF_TWO_MAC_ADDRS; i++) {
      *ptr = *(ptr + 4);
      ptr++;
    }

    *(ptr++) = tpid >> 8;
    *(ptr++) = tpid;
    *(ptr++) = tci >> 8;
    *(ptr++) = tci;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON26) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_ifh_init()
// Frame layout under different conditions:
// On call:
//   [16 bytes of uninitialized free space][6 bytes DMAC][6 bytes SMAC][EtherType][Payload]
//
// Temporary variable. We insert a tag if we either send the frame switched or are requested to insert one with #tpid.
//   InsertTag = switch_frm != FALSE || tpid != 0
//
// On exit when InsertTag == FALSE && ptp_action == 0:
//   [8 bytes of uninitialized free space][8 bytes injection header][6 bytes DMAC][6 bytes SMAC][EtherType][Payload]
//   That is, ifh_offset is set to skip the first 8 bytes.
//
// On exit when InsertTag == FALSE && ptp_action != 0:
//   [4 bytes of uninitialized free space][8 bytes injection header][4 bytes timestamp][6 bytes DMAC][6 bytes SMAC][EtherType][Payload]
//   That is, ifh_offset is set to skip the first 4 bytes.
//
// On exit when InsertTag == TRUE && ptp_action == 0:
//   [4 bytes of uninitialized free space][8 bytes injection header][6 bytes DMAC][6 bytes SMAC][4 bytes VLAN Tag][EtherType][Payload]
//   That is, ifh_offset is set to skip the first 4 bytes and DMAC and SMAC are moved 4 bytes ahead while inserting a VLAN tag.
//
// On exit when InsertTag == TRUE && ptp_action != 0:
//   [0 bytes of uninitialized free space][8 bytes injection header][4 bytes timestamp][6 bytes DMAC][6 bytes SMAC][4 bytes VLAN Tag][EtherType][Payload]
//   That is, ifh_offset is set to skip 0 bytes, a PTP timestamp is inserted, and DMAC and SMAC are moved 4 bytes ahead while inserting a VLAN tag.
/*****************************************************************************/
static FDMA_INLINE u32 inj_ifh_init(u8 *const data_ptr, const vtss_fdma_inj_props_t *const props, u32 *ifh_size)
{
    u64 inj_hdr;
    u32 ifh_offset;
    u32 offset_into_data = VTSS_FDMA_HDR_SIZE_BYTES - VTSS_FDMA_INJ_HDR_SIZE_BYTES; // Take into account any asymmetry between INJ and XTR headers.
    u8  qos_class;

    // From now on, qos_class == 8 is not used for anything. Adjust it to 7 for further usage.
    if (props->qos_class == 8) {
        qos_class = 7;
    } else {
        qos_class = props->qos_class;
    }

    if (props->switch_frm || props->tpid) {
        if (props->ptp_action == 0) {
            ifh_offset = 4;
        } else {
            ifh_offset = 0;
        }
    } else {
        if (props->ptp_action == 0) {
            ifh_offset = 8;
        } else {
            ifh_offset = 4;
        }
    }

    ifh_offset += offset_into_data;
    *ifh_size = props->ptp_action == 0 ? 8 : 8 + 4;

    if (!props->dont_touch_ifh) {
        inj_hdr  = VTSS_ENCODE_BITFIELD64(!props->switch_frm, 63,  1); // BYPASS
        inj_hdr |= VTSS_ENCODE_BITFIELD64( props->ptp_action, 61,  2); // PTP_ACTION
        inj_hdr |= VTSS_ENCODE_BITFIELD64( props->ptp_id,     59,  2); // PTP_ID
        inj_hdr |= VTSS_ENCODE_BITFIELD64( props->port_mask,  32, 26); // Ignore the CPU port
        inj_hdr |= VTSS_ENCODE_BITFIELD64(        qos_class,  17,  3); // QOS Class.

        if (!props->switch_frm) {
            if (props->tpid) {
                inj_insert_vlan_tag(data_ptr + VTSS_FDMA_HDR_SIZE_BYTES - 4, props->tpid,  (qos_class << 13) | (props->vlan & 0xFFF));
            }
            // Inject frame as is (backwards compatibility with Luton28), i.e. disable rewriter.
            inj_hdr |= VTSS_ENCODE_BITFIELD64(0x3, 28, 2); // Disable rewriter (this is the POP_CNT field of the injection header).
        } else {
            // If sending the frame switched, we insert a VLAN tag in the packet by moving DMAC
            // and SMAC four bytes ahead. When doing that, the frame is classified to the
            // VLAN that the user wants, but on egress it still contains the tag, so we
            // have to remove it by setting POP_CNT to 1.
            inj_hdr |= VTSS_ENCODE_BITFIELD64(0x1, 28, 2);
            // PCP determines the QoS Class. A one-to-one mapping is set-up in vtss_luton26.c::l26_init_conf_set(). No matter value of CFI-value, the DP level will become 0.
            inj_insert_vlan_tag(data_ptr + VTSS_FDMA_HDR_SIZE_BYTES - 4, 0x8100,  (qos_class << 13) | (props->vlan & 0xFFF));
        }

        // Lu26 has variable-length IFH. If PTP_ACTION > 0, a 32-bit timestamp
        // is inserted in between the normal injection header and the first
        // byte of the DMAC.
        if (props->ptp_action > 0) {
            // Store the time stamp.
            data_ptr[ifh_offset + 8 + 0] = props->ptp_timestamp >> 24;
            data_ptr[ifh_offset + 8 + 1] = props->ptp_timestamp >> 16;
            data_ptr[ifh_offset + 8 + 2] = props->ptp_timestamp >>  8;
            data_ptr[ifh_offset + 8 + 3] = props->ptp_timestamp >>  0;
        }

        // Time to store it in DDR SDRAM.
        data_ptr[ifh_offset + 0] = inj_hdr >> 56;
        data_ptr[ifh_offset + 1] = inj_hdr >> 48;
        data_ptr[ifh_offset + 2] = inj_hdr >> 40;
        data_ptr[ifh_offset + 3] = inj_hdr >> 32;
        data_ptr[ifh_offset + 4] = inj_hdr >> 24;
        data_ptr[ifh_offset + 5] = inj_hdr >> 16;
        data_ptr[ifh_offset + 6] = inj_hdr >>  8;
        data_ptr[ifh_offset + 7] = inj_hdr >>  0;
    }
    return ifh_offset;
}
#endif /* VTSS_ARCH_LUTON26  && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_JAGUAR_1) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_ifh_init()
/*****************************************************************************/
static FDMA_INLINE u32 inj_ifh_init(u8 *const data_ptr, const vtss_fdma_inj_props_t *const props, u32 *ifh_size)
{
    u64 dst;
    u16 vstax_hi = 0;
    u64 vstax_lo = VTSS_ENCODE_BITFIELD64(1, 55, 1); // ingr_drop_mode = 1 => don't make head-of-line blocking
    u64 fwd;
    u32 ifh_offset = VTSS_FDMA_HDR_SIZE_BYTES - VTSS_FDMA_INJ_HDR_SIZE_BYTES; // Take into account any asymmetry between INJ and XTR headers.
    u8  qos_class;

    // From now on, qos_class == 8 is not used for anything. Adjust it to 7 for further usage.
    if (props->qos_class == 8) {
        qos_class = 7;
    } else {
        qos_class = props->qos_class;
    }

    if (!props->dont_touch_ifh) {
        dst = 0;
        fwd = VTSS_ENCODE_BITFIELD64(1, 44, 1); // Enforce update of FCS.

#if defined(VTSS_FEATURE_VSTAX)
        if (props->contains_stack_hdr && (props->inj_grp == 2 || props->inj_grp == 3)) {
            // Super-priority injection. We only need to set the VStaX header and the .VSTAX_AVAIL bit.
            fwd |= VTSS_ENCODE_BITFIELD64(1, 45, 1); // FWD.VSTAX_AVAIL = 1
        } else
#endif /* VTSS_FEATURE_VSTAX */
        {
            if (props->switch_frm) {
                // We gotta insert a VLAN tag into the frame. The alternative to this is to use the
                // VStaX header to hold the tag and set FWD.VSTAX_AVAIL, but this will not work in
                // a stacking environment (switched frames will not go across the stack ports then,
                // and TTL cannot be set-up, since we don't know which way the frame goes).
                // PCP determines the QoS Class. A one-to-one mapping is set-up in vtss_jaguar1.c::jr_init_packet(). No matter value of CFI-value, the DP level will become 0.
                inj_insert_vlan_tag(data_ptr + VTSS_FDMA_HDR_SIZE_BYTES - 4, 0x8100,  (qos_class << 13) | (props->vlan & 0xFFF));
                ifh_offset -= 4;
                vstax_hi = 0;
            } else {
                if (props->qos_class == 8) {
                    // User wants to inject on super-priority to a front-port.
                    // This mode requires the use of the "INJECT_UNICAST" mode. Find the first bit in the port_mask
                    // that is set (i.e. this mode can only inject on a single port at a time, as stated in the API).
                    u64 port;
                    u64 mask = 1;
                    for (port = 0; port < VTSS_PHYS_PORT_CNT; port++) {
                        if ((mask & props->port_mask) != 0) {
                            break;
                        }
                        mask <<= 1;
                    }
                    fwd |= VTSS_ENCODE_BITFIELD64(2,    16,  2); // INJECT_UNICAST
                    dst |= VTSS_ENCODE_BITFIELD64(port,  0,  8);
                } else {
                    // When not injecting through the analyzer, use INJECT_MULTICAST.
                    // This means that we cannot currently inject into a host queue.
                    // The classifier and analyzer are completely bypassed, and the frame is directed to the ports specified in IFH.DST.MC_INJECT_DST.
                    fwd |= VTSS_ENCODE_BITFIELD64(3,                16,  2); // INJECT_MULTICAST
                    dst |= VTSS_ENCODE_BITFIELD64(props->port_mask,  0, 32);
                    fwd |= VTSS_ENCODE_BITFIELD64(1,                45,  1); // FWD.VSTAX_AVAIL = 1
                }

                fwd |= VTSS_ENCODE_BITFIELD64(props->latch_timestamp, 46,  2); // FWD.CAPTURE_TSTAMP
                fwd |= VTSS_ENCODE_BITFIELD64(1,                      28,  1); // FWD.DO_NOT_REW

                if (props->tpid) {
                    inj_insert_vlan_tag(data_ptr + VTSS_FDMA_HDR_SIZE_BYTES - 4, props->tpid,  (qos_class << 13) | (props->vlan & 0xFFF));
                    ifh_offset -= 4;
                }

                // We can get here even with a stack header, in the case the user doesn't inject into
                // a super-priority group (i.e. inj_grp != 2 && inj_grp != 3).
#if defined(VTSS_FEATURE_VSTAX)
                if (!props->contains_stack_hdr)
#endif /* VTSS_FEATURE_VSTAX */
                {
                    // We don't have a VLAN tag to get the switch determine the QoS class from,
                    // so we must use the VStaX header (props->port_mask must not include stack ports)
                    // to convey the information (this is not possible when sending switched).
                    vstax_hi = VTSS_ENCODE_BITFIELD  (1,         15, 1); // MSBit must be 1
                    vstax_lo |= VTSS_ENCODE_BITFIELD64(qos_class, 56, 3); // qos_class/iprio (internal priority)
                }
            }
        }

        data_ptr[ifh_offset +  0] = dst >> 56;
        data_ptr[ifh_offset +  1] = dst >> 48;
        data_ptr[ifh_offset +  2] = dst >> 40;
        data_ptr[ifh_offset +  3] = dst >> 32;
        data_ptr[ifh_offset +  4] = dst >> 24;
        data_ptr[ifh_offset +  5] = dst >> 16;
        data_ptr[ifh_offset +  6] = dst >>  8;
        data_ptr[ifh_offset +  7] = dst >>  0;
#if defined(VTSS_FEATURE_VSTAX)
        if (props->contains_stack_hdr) {
            // Skip EtherType.
            memcpy(data_ptr + ifh_offset + 8, props->stack_hdr_bin + 2, VTSS_VSTAX_HDR_SIZE - 2);
        } else
#endif /* VTSS_FEATURE_VSTAX */
        {
            data_ptr[ifh_offset +  8] = vstax_hi >>  8;
            data_ptr[ifh_offset +  9] = vstax_hi >>  0;
            data_ptr[ifh_offset + 10] = vstax_lo >> 56;
            data_ptr[ifh_offset + 11] = vstax_lo >> 48;
            data_ptr[ifh_offset + 12] = vstax_lo >> 40;
            data_ptr[ifh_offset + 13] = vstax_lo >> 32;
            data_ptr[ifh_offset + 14] = vstax_lo >> 24;
            data_ptr[ifh_offset + 15] = vstax_lo >> 16;
            data_ptr[ifh_offset + 16] = vstax_lo >>  8;
            data_ptr[ifh_offset + 17] = vstax_lo >>  0;
        }
        data_ptr[ifh_offset + 18] = fwd >> 40;
        data_ptr[ifh_offset + 19] = fwd >> 32;
        data_ptr[ifh_offset + 20] = fwd >> 24;
        data_ptr[ifh_offset + 21] = fwd >> 16;
        data_ptr[ifh_offset + 22] = fwd >>  8;
        data_ptr[ifh_offset + 23] = fwd >>  0;

        if (
#if defined(VTSS_FEATURE_VSTAX)
            props->contains_stack_hdr ||
#endif /* VTSS_FEATURE_VSTAX */
            props->qos_class == 8) {
            // When injecting stack-frames through the super-priority queues,
            // or frames through the front port super-priority queue,
            // the IFH checksum must be correct, because it's checking in the
            // chip (DSM) cannot be disabled, as it can for non-SP-transmitted
            // frames (through ASM::CPU_CFG.CPU_CHK_IFH_CHKSUM_ENA).
            // This is computed over the entire IFH as BIP-8 (BIP8).
            u8 chksum = 0;
            u8 *p = &data_ptr[ifh_offset];
            int i;
            for (i = 0; i < VTSS_FDMA_INJ_HDR_SIZE_BYTES; i++) {
                chksum ^= *(p++);
            }
            data_ptr[ifh_offset + 23] = chksum;
        }
    }
    return ifh_offset;
}
#endif /* defined(VTSS_ARCH_JAGUAR_1) && VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// inj_ifh_init()
/*****************************************************************************/
static FDMA_INLINE vtss_rc inj_ifh_init(vtss_state_t *const vstate, fdma_sw_dcb_t *sw_dcb, vtss_packet_tx_info_t *const tx_info, vtss_packet_tx_grp_t *inj_grp, u32 *frm_size)
{
    vtss_fdma_list_t *user_dcb = FDMA_USER_DCB(sw_dcb), *user_dcb_iter;
    u32              bytes_stripped_on_egress = 0, actual_length_bytes = 0, bin_hdr_len, trace_lvl;
    vtss_rc          rc = VTSS_RC_OK;

#if defined(VTSS_ARCH_JAGUAR_1)
    bin_hdr_len = 24 /* VTSS_FDMA_INJ_HDR_SIZE_BYTES */;
#else
    // Luton26
    bin_hdr_len = tx_info->ptp_action == 0 ? 8 : 8 + 4;
#endif /* defined(VTSS_ARCH_xxx) */

    // Frame pointer must be equal to the IFH pointer by now.
    FDMA_ASSERT(NORMAL, sw_dcb->ifh_ptr == user_dcb->frm_ptr, return VTSS_RC_ERROR;);

    if (tx_info->switch_frm || tx_info->tag.tpid) {
        vtss_vlan_tag_t tag;
        if (tx_info->switch_frm) {
            // We gotta insert a VLAN tag into the frame when switching the frame.
            // The alternative to this is to use the VStaX header to hold the tag
            // and set FWD.VSTAX_AVAIL, but this will not work in a stacking environment
            // (switched frames will not go across the stack ports then, and TTL cannot
            // be set-up, since we don't know which way the frame goes).
            tag.tpid = 0x8100,
            tag.pcp  = tx_info->cos == 8 ? 7 : tx_info->cos, // Even though there is a PCP-to-CoS conversion.
            tag.dei  = 0,
            tag.vid  = tx_info->tag.vid;
        }

        // Use tag from tx_info when asked to insert a tag (tpid != 0).
        VTSS_RC(vtss_fdma_cmn_insert_vlan_tag(sw_dcb, tx_info->switch_frm ? &tag : &tx_info->tag, FALSE /* No need to move IFH data because we haven't yet constructed it */, FALSE /* Called with interrupts enabled */));
        bytes_stripped_on_egress += tx_info->switch_frm ? 4 : 0;
    }

    // We got to adjust the length of the EOF DCB to accommodate for a
    // minimum sized Ethernet frame.
    user_dcb_iter = user_dcb;
    while (user_dcb_iter) {
        actual_length_bytes += user_dcb_iter->act_len;

        if (user_dcb_iter->next == NULL) {
            // End-of-frame DCB. All lengths are excluding IFH/CMD and
            // excluding FCS, hence the 60 instead of 64.
            // We cannot pad the actual frame data with zeros because we
            // don't know how much data was allocated by the application.
            u32 bytes_on_wire = actual_length_bytes - bytes_stripped_on_egress;
            if (bytes_on_wire < 60) {
                u32 pad_bytes = 60 - bytes_on_wire;
                user_dcb_iter->act_len += pad_bytes;
                actual_length_bytes    += pad_bytes;
            }
        }

        user_dcb_iter = user_dcb_iter->next;
    }

    *frm_size  = actual_length_bytes + 4; /* Exclude IFH and CMD, but include FCS */

    // And finally time for the IFH.
    // Adjust the IFH pointer.
    sw_dcb->ifh_ptr -= bin_hdr_len;

#if defined(VTSS_ARCH_JAGUAR_1)
    // Prevent this frame from being transmitted multiple times.
    sw_dcb->dst_port_mask = 0;

    // There is one thing that needs to be handled by the FDMA driver on JR-48, and
    // that is multi-port injection on a 48-ported where ports from both devices
    // or multiple ports on secondary device are involved. This requires multiple
    // injections, so we have to investigate that case here.
    if (tx_info->switch_frm == FALSE && vstate->chip_count == 2) {
        // Gotta split the destination port mask into two physical port masks.
        u64            pri_mask = 0, sec_mask = 0, sec_first_mask = 0, mask = tx_info->dst_port_mask;
        BOOL           port_on_sec = FALSE;
        vtss_port_no_t p;

        if (mask == 0) {
            VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Called with an empty destination port mask");
            VTSS_EG_HEX(VTSS_TRACE_GROUP_FDMA_NORMAL, sw_dcb->ifh_ptr, 64);
            return VTSS_RC_ERROR;
        }

        // Remember that the physical ports on one device could come
        // in any order in the logical port mask, so we have to go through
        // all bits (can't just separate the logical port mask into two halves).
        while((p = VTSS_OS_CTZ64(mask)) < 64) {
            mask &= ~VTSS_BIT64(p);

            if (p >= VTSS_ARRSZ(vstate->port.map)) {
                // Should have been a check against vstate->port_count, but that won't satisfy Lint.
                VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Port %u not in port map", p);
                return VTSS_RC_ERROR;
            }

            if (vstate->port.map[p].chip_no == 0) {
                pri_mask |= VTSS_BIT64(p);
            } else {
                if (!port_on_sec) {
                    // First port on secondary.
                    sec_first_mask = VTSS_BIT64(p);
                    port_on_sec    = TRUE;
                } else {
                    sec_mask |= VTSS_BIT64(p);
                }
            }
        }

        if (pri_mask) {
            // Now, if pri_mask is non-zero, we first send to the primary
            // device's ports, but we must also add the #first_on_sec
            // to the secondary mask, because we ommitted that in the
            // loop above.
            sec_mask |= sec_first_mask;
            tx_info->dst_port_mask = pri_mask;
        } else {
            // No ports on the primary device. The very first port on the secondary
            // is given by #first_on_sec;
            tx_info->dst_port_mask = sec_first_mask;
        }

        // The remaining ports on the secondary must be saved off somewhere
        // so that the frame can get reinjected upon completion.
        sw_dcb->dst_port_mask = sec_mask;

        if (sec_mask) {
            u64 save_mask;

            // We have to create a signature IFH for use by reinjection.
            // "Luckily" we have reserved an additional 24 bytes in front of the
            // "real" IFH, into which we can store an IFH as it would look when
            // injecting to a port on the secondary device.
            FDMA_ASSERT(NORMAL, (u32)(sw_dcb->ifh_ptr - sw_dcb->data) >= bin_hdr_len, return VTSS_RC_ERROR;);

            // We have to provide a mask with a single port (located on the secondary device), so
            // let's save a copy of the current mask.
            save_mask = tx_info->dst_port_mask;
            tx_info->dst_port_mask = sec_first_mask;
            rc = vstate->fdma_state.tx_hdr_encode_internal(vstate, tx_info, sw_dcb->data, &bin_hdr_len, inj_grp);
            tx_info->dst_port_mask = save_mask;

            // When we reinject the frame, we assume that the injection group is #0,
            // which - with the given signature IFH - is supposed to hit the interconnect
            // port, hence the below check.
            FDMA_ASSERT(NORMAL, *inj_grp == 0, return VTSS_RC_ERROR;);
        }
    }
#endif

    if (rc == VTSS_RC_OK) {
#if defined(VTSS_ARCH_JAGUAR_1)
        // And call an internal function, which also returns the injection group to use.
        rc = vstate->fdma_state.tx_hdr_encode_internal(vstate, tx_info, sw_dcb->ifh_ptr, &bin_hdr_len, inj_grp);
#else
        rc = vstate->packet.tx_hdr_encode(vstate, tx_info, sw_dcb->ifh_ptr, &bin_hdr_len);
        *inj_grp = vstate->fdma_state.fdma_ch_state[VTSS_FDMA_CH_INJ].inj_grp;
#endif /* defined(VTSS_ARCH_xxx) */
    }

    if (rc == VTSS_RC_OK) {
        // Check that we haven't inserted too much data.
        FDMA_ASSERT(NORMAL, sw_dcb->ifh_ptr >= sw_dcb->data, return VTSS_RC_ERROR;);

        trace_lvl = VTSS_TRACE_LEVEL_INFO;
    } else {
        trace_lvl = VTSS_TRACE_LEVEL_ERROR;
    }

    VTSS_T(VTSS_TRACE_GROUP_FDMA_NORMAL, trace_lvl, "IFH (%u bytes) + a bit frame data (%u bytes incl. FCS):", FDMA_USER_DCB(sw_dcb)->frm_ptr - sw_dcb->ifh_ptr, *frm_size);
    VTSS_HEX(VTSS_TRACE_GROUP_FDMA_NORMAL, trace_lvl, sw_dcb->ifh_ptr, 64);

    return rc;
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_dcb_init()
// @sw_dcb : NULL-terminated List of DCBs
// @inj_ch: The channel that this one goes through.
// @whl_frm_sz_bytes is filled with the total frame size, including IFH and FCS,
// computed based on each fragment size.
// The function returns a pointer to the last list item used for this frame.
/*****************************************************************************/
static fdma_sw_dcb_t *inj_dcb_init(const vtss_packet_tx_grp_t inj_grp, fdma_sw_dcb_t *sw_dcb, u32 ifh_offset, u32 ifh_size, int *const whl_frm_sz_bytes, u32 *const dcb_cnt, BOOL want_interrupt_on_eof, vtss_fdma_ch_usage_t usage, BOOL need_not_be_cache_line_aligned)
{
    u32              *phys_data_ptr, phys_data_off;
    fdma_hw_dcb_t    *hw_dcb;
    u8               *data_ptr;
    int              frag_sz_bytes;
    int              sof = 1, eof;
    fdma_sw_dcb_t    *tail = sw_dcb;
    vtss_fdma_list_t *user_dcb;
    u32              tt_fc;

    *whl_frm_sz_bytes = 0;
    *dcb_cnt          = 0;

    while (sw_dcb) {
        user_dcb = FDMA_USER_DCB(sw_dcb);
        frag_sz_bytes = user_dcb->act_len;

        FDMA_ASSERT(NORMAL, sw_dcb->data, return NULL;);
        FDMA_ASSERT(NORMAL,
          ((frag_sz_bytes >= VTSS_FDMA_MIN_DATA_PER_INJ_SOF_DCB_BYTES && sof == 1)  ||
           (frag_sz_bytes >= VTSS_FDMA_MIN_DATA_PER_NON_SOF_DCB_BYTES && sof == 0)) &&
            frag_sz_bytes <= VTSS_FDMA_MAX_DATA_PER_DCB_BYTES, return NULL;);

        if (sof) {
            // Due to variable length IFH on Lu26, we need to add @ifh_offset to the sw_dcb->data before storing it in the DCB
            // and subtract @ifh_offset from the frag_sz_bytes before writing the length.
            frag_sz_bytes -= ifh_offset;
            data_ptr = sw_dcb->data + ifh_offset;
            sw_dcb->ifh_ptr = data_ptr;
            user_dcb->frm_ptr = sw_dcb->ifh_ptr + ifh_size;
        } else {
            // Non-SOF DCBs aren't changed.
            data_ptr = sw_dcb->data;
            sw_dcb->ifh_ptr = user_dcb->frm_ptr = sw_dcb->data;
        }

        *whl_frm_sz_bytes += frag_sz_bytes;

        eof = user_dcb->next == NULL;
        if (eof) {
            // whl_frm_sz_bytes includes INJ header, the frame itself, and the FCS.
            // We gotta adjust the length of the last fragment to accommodate for a minimum-sized Ethernet frame.
            if ((u32)*whl_frm_sz_bytes < 64 + ifh_size) {
                frag_sz_bytes += (64 + ifh_size - *whl_frm_sz_bytes);
            }
        }

        // Due to a bug in the FDMA, all non-EOF DCBs' associated data area cannot have a size that is not a multiple of 4.
        // This is documented in GNATS #5355.
        // For testing, it is possible to turn off this check.
#if !defined(VTSS_FDMA_IGNORE_GNATS_5355)
        FDMA_ASSERT(NORMAL, eof || (frag_sz_bytes & 0x3) == 0, return NULL;);
#endif /* VTSS_FDMA_IGNORE_GNATS_5355 */

        phys_data_ptr = (u32 *)VTSS_OS_VIRT_TO_PHYS(data_ptr);
        phys_data_off = (u32)phys_data_ptr & 0x3;
        hw_dcb = FDMA_HW_DCB(sw_dcb);

        // The H/W DCB should be cache-line-size aligned
        FDMA_ASSERT(NORMAL, need_not_be_cache_line_aligned || ((u32)hw_dcb & (VTSS_OS_DCACHE_LINE_SIZE_BYTES - 1)) == 0, return NULL;);

        hw_dcb->sar  = CPU_TO_BUS((u32)phys_data_ptr & ~0x3);
        hw_dcb->dar  = CPU_TO_BUS(FDMA_DAR_CHUNK_SIZE(frag_sz_bytes) | FDMA_DAR_INJ_GRP(inj_grp) | FDMA_DAR_SAR_OFFSET(phys_data_off) | FDMA_DAR_EOF(eof) | FDMA_DAR_SOF(sof));
        hw_dcb->llp  = CPU_TO_BUS(user_dcb->next ? VTSS_OS_VIRT_TO_PHYS(FDMA_HW_DCB(FDMA_SW_DCB(user_dcb->next))) : 0); // NULL
        tt_fc = usage == VTSS_FDMA_CH_USAGE_INJ ? 1 : 2; // For normal injection, we use Memory-to-Peripheral. For AFI we must use Peripheral-to-Memory for the channel not to read data from the AHB bus ASAP, which in turn causes the channel to lock until the next AFI tick.
        hw_dcb->ctl0 = CPU_TO_BUS(VTSS_F_FDMA_CH_CTL0_LLP_SRC_EN   | VTSS_F_FDMA_CH_CTL0_LLP_DST_EN               | VTSS_F_FDMA_CH_CTL0_DMS(1)                    |
                                              VTSS_F_FDMA_CH_CTL0_TT_FC(tt_fc) | VTSS_F_FDMA_CH_CTL0_SRC_MSIZE(INJ_SRC_MSIZE) | VTSS_F_FDMA_CH_CTL0_DEST_MSIZE(INJ_DST_MSIZE) |
                                              VTSS_F_FDMA_CH_CTL0_DINC(2)      | VTSS_F_FDMA_CH_CTL0_SRC_TR_WIDTH(2)          | VTSS_F_FDMA_CH_CTL0_DST_TR_WIDTH(2)           | (want_interrupt_on_eof && eof ? VTSS_F_FDMA_CH_CTL0_INT_EN : 0)); // We only want interrupts if this is the EOF.

        // If the data area is non-32-bit-aligned (as is the case for Linux), then we need to check if we're gonna inject one more 32-bit word.
        hw_dcb->ctl1 = CPU_TO_BUS((frag_sz_bytes + phys_data_off + 3) / 4); // BLOCK_TS
        hw_dcb->stat = CPU_TO_BUS(0);

        VTSS_OS_REORDER_BARRIER();

        // Flush the dcache, so that the DCB gets written to main memory.
        VTSS_OS_DCACHE_FLUSH(hw_dcb, VTSS_FDMA_DCB_SIZE_BYTES);

        VTSS_OS_REORDER_BARRIER();

        // Flush the dcache, so that the frame data gets written to main memory.
        VTSS_OS_DCACHE_FLUSH(data_ptr, frag_sz_bytes);

        (*dcb_cnt)++;

        // Prepare for next iteration
        sof = 0;
        if (user_dcb->next) {
            tail = FDMA_SW_DCB(user_dcb->next);
        }

        sw_dcb = FDMA_SW_DCB(user_dcb->next);
    }
    return tail;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// inj_dcb_init()
/*****************************************************************************/
static FDMA_INLINE vtss_rc inj_dcb_init(vtss_state_t *const vstate, fdma_sw_dcb_t *sw_dcb, vtss_packet_tx_grp_t inj_grp, fdma_sw_dcb_t **tail, u32 *dcb_cnt, vtss_fdma_dcb_type_t dcb_type, BOOL want_interrupt_on_eof, BOOL need_not_be_cache_line_aligned, BOOL thread_safe)
{
    u32              *phys_data_ptr, phys_data_off, tt_fc;
    fdma_hw_dcb_t    *hw_dcb;
    int              frag_sz_bytes;
    int              sof = 1, eof;
    vtss_fdma_list_t *user_dcb;

    *dcb_cnt = 0;
    *tail    = sw_dcb;

    while (sw_dcb) {
        user_dcb      = FDMA_USER_DCB(sw_dcb);
        eof           = user_dcb->next == NULL;
        // user_dcb->act_len must hold the size of the actual frame data excluding IFH and FCS.
        frag_sz_bytes = user_dcb->act_len + (user_dcb->frm_ptr - sw_dcb->ifh_ptr) + (eof ? 4 /* FCS */ : 0);

        FDMA_CHECK(thread_safe, sw_dcb->ifh_ptr, return VTSS_RC_ERROR;);
        FDMA_CHECK(thread_safe,
          ((frag_sz_bytes >= VTSS_FDMA_MIN_DATA_PER_INJ_SOF_DCB_BYTES && sof == 1)  ||
           (frag_sz_bytes >= VTSS_FDMA_MIN_DATA_PER_NON_SOF_DCB_BYTES && sof == 0)) &&
            frag_sz_bytes <= VTSS_FDMA_MAX_DATA_PER_DCB_BYTES, return VTSS_RC_ERROR;);

        // Due to a bug in the FDMA, all non-EOF DCBs' associated data area cannot have a size that is not a multiple of 4.
        // This is documented in GNATS #5355.
        // For testing, it is possible to turn off this check.
#if !defined(VTSS_FDMA_IGNORE_GNATS_5355)
        FDMA_CHECK(thread_safe, eof || (frag_sz_bytes & 0x3) == 0, return VTSS_RC_ERROR;);
#endif /* VTSS_FDMA_IGNORE_GNATS_5355 */

        phys_data_ptr = (u32 *)VTSS_OS_VIRT_TO_PHYS(sw_dcb->ifh_ptr);
        phys_data_off = (u32)phys_data_ptr & 0x3;
        hw_dcb = FDMA_HW_DCB(sw_dcb);

        // The H/W DCB should be cache-line-size aligned
        FDMA_CHECK(thread_safe, need_not_be_cache_line_aligned || ((u32)hw_dcb & (VTSS_OS_DCACHE_LINE_SIZE_BYTES - 1)) == 0, return VTSS_RC_ERROR;);

        hw_dcb->sar  = CPU_TO_BUS((u32)phys_data_ptr & ~0x3);
        hw_dcb->dar  = CPU_TO_BUS(FDMA_DAR_CHUNK_SIZE(frag_sz_bytes) | FDMA_DAR_INJ_GRP(inj_grp) | FDMA_DAR_SAR_OFFSET(phys_data_off) | FDMA_DAR_EOF(eof) | FDMA_DAR_SOF(sof));
        hw_dcb->llp  = CPU_TO_BUS(user_dcb->next ? VTSS_OS_VIRT_TO_PHYS(FDMA_HW_DCB(FDMA_SW_DCB(user_dcb->next))) : 0); // NULL
        tt_fc        = dcb_type == VTSS_FDMA_DCB_TYPE_INJ ? 1 : 2; // For normal injection, we use Memory-to-Peripheral. For AFI we must use Peripheral-to-Memory for the channel not to read data from the AHB bus ASAP, which in turn causes the channel to lock until the next AFI tick.
        hw_dcb->ctl0 = CPU_TO_BUS(VTSS_F_FDMA_CH_CTL0_LLP_SRC_EN   | VTSS_F_FDMA_CH_CTL0_LLP_DST_EN               | VTSS_F_FDMA_CH_CTL0_DMS(1)                    |
                                              VTSS_F_FDMA_CH_CTL0_TT_FC(tt_fc) | VTSS_F_FDMA_CH_CTL0_SRC_MSIZE(INJ_SRC_MSIZE) | VTSS_F_FDMA_CH_CTL0_DEST_MSIZE(INJ_DST_MSIZE) |
                                              VTSS_F_FDMA_CH_CTL0_DINC(2)      | VTSS_F_FDMA_CH_CTL0_SRC_TR_WIDTH(2)          | VTSS_F_FDMA_CH_CTL0_DST_TR_WIDTH(2)           | (want_interrupt_on_eof && eof ? VTSS_F_FDMA_CH_CTL0_INT_EN : 0)); // We only want interrupts if this is the EOF.

        // If the data area is non-32-bit-aligned (as is the case for Linux), then we need to check if we're gonna inject one more 32-bit word.
        hw_dcb->ctl1 = CPU_TO_BUS((frag_sz_bytes + phys_data_off + 3) / 4); // BLOCK_TS
        hw_dcb->stat = CPU_TO_BUS(0);

        VTSS_OS_REORDER_BARRIER();

        // Flush the dcache, so that the DCB gets written to main memory.
        VTSS_OS_DCACHE_FLUSH(hw_dcb, VTSS_FDMA_DCB_SIZE_BYTES);

        VTSS_OS_REORDER_BARRIER();

        // Flush the dcache, so that the frame data gets written to main memory.
        VTSS_OS_DCACHE_FLUSH(sw_dcb->ifh_ptr, frag_sz_bytes);

        (*dcb_cnt)++;

        // Prepare for next iteration
        sof = 0;
        if (user_dcb->next) {
            *tail = FDMA_SW_DCB(user_dcb->next);
        }

        sw_dcb = FDMA_SW_DCB(user_dcb->next);
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

/*****************************************************************************/
// inj_ch_init()
/*****************************************************************************/
static void inj_ch_init(vtss_state_t *const vstate, fdma_hw_dcb_t *hw_dcb, vtss_fdma_ch_t inj_ch)
{
    VCOREIII_WR(vstate, VTSS_FDMA_CH_LLP(inj_ch), VTSS_OS_VIRT_TO_PHYS(hw_dcb));
}

#if (defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)) || defined(VTSS_ARCH_LUTON26)
/*****************************************************************************/
// xtr_qu_from_mask()
// The IFH contains an extraction queue mask, where each bit in the mask
// implies that the frame was subject to CPU forwarding on the specific port.
// The most significant of these bits indicate the extraction queue that the
// frame was actually sent to.
// This function finds this queue number (0-based) from the mask.
/*****************************************************************************/
static FDMA_INLINE u8 xtr_qu_from_mask(u32 xtr_qu_mask)
{
    int qu; // Must be an integer to be able to terminate this loop if called with 0.
    for (qu = VTSS_PACKET_RX_QUEUE_CNT - 1; qu >= 0; qu--) {
        if (xtr_qu_mask & VTSS_BIT(qu)) {
            break;
        }
    }

    FDMA_ASSERT(IRQ, qu >= 0 && qu < VTSS_PACKET_RX_QUEUE_CNT, return 0;);
    return (u8)qu;
}
#endif /* /defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)) || defined(VTSS_ARCH_LUTON26) */

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
/*****************************************************************************/
// xtr_super_prio_ifh_synthesize()
// Jaguar does not provide an IFH for frames received with super priority, so
// to make the interface towards the application uniform, we insert one here.
// "Luckily" we've made room for such an artificial IFH.
// The function returns TRUE if a valid external stack port can be found
// from the current VStaX configuration. Otherwise, it returns FALSE.
/*****************************************************************************/
static FDMA_INLINE BOOL xtr_super_prio_ifh_synthesize(vtss_state_t *vstate, u32 xtr_qu, fdma_sw_dcb_t *sof)
{
    u8  vstax_hdr[VTSS_VSTAX_HDR_SIZE], *dst_ptr, *src_ptr, src_port;
    u16 etype;
    int i;
    u64 fwd;
    vtss_chip_no_t chip_no = 0;
    vtss_port_no_t port_0 = vstate->l2.vstax_conf.port_0, port_1 = vstate->l2.vstax_conf.port_1;

    if (port_0 == VTSS_PORT_NO_NONE || port_1 == VTSS_PORT_NO_NONE) {
        // Well, stacking doesn't seem to be enabled. Drop the frame.
        return FALSE;
    }

    if (vstate->chip_count == 2) {
        // 48-port solution.
        if (xtr_qu == 8) {
            // Frames received into xtr_qu 8 is received on the primary device's
            // interconnect, and therefore originally received on the secondary
            // device's external stack port (see also ASM:SP_CFG:SP_RX_CFG.SP_RX_SEL).
            chip_no = 1;
        }

        // Find the corresponding external stack port.
        if (VTSS_CHIP_NO_FROM_STATE(vstate, port_0) == chip_no) {
            src_port = VTSS_CHIP_PORT_FROM_STATE(vstate, port_0);
        } else if (VTSS_CHIP_NO_FROM_STATE(vstate, port_1) == chip_no) {
            src_port = VTSS_CHIP_PORT_FROM_STATE(vstate, port_1);
        } else {
            return FALSE; // Can't find a matching chip (configuration error). Drop frame.
        }
    } else {
        // 24-port solution.
        // Convention says that xtr_qu 8 corresponds to port_1
        // and xtr_qu 9 corresponds to port_0.
        if (xtr_qu == 8) {
            src_port = VTSS_CHIP_PORT_FROM_STATE(vstate, port_1);
        } else {
            src_port = VTSS_CHIP_PORT_FROM_STATE(vstate, port_0);
        }
    }

//     VTSS_IG(VTSS_TRACE_GROUP_FDMA_IRQ, "Received on xtr_qu = %u, chip_no = %d, src_port = %u (port_0 = %u, port_1 = %u)", xtr_qu, chip_no, src_port, port_0, port_1);

    // sof->ifh_ptr points to 4 bytes containing extraction queue number.
    // FDMA_USER_DCB(sof)->frm_ptr points to somewhere inside the frame, since we've assumed
    // that FDMA_USER_DCB(sof)->frm_ptr - sof->ifh_ptr == VTSS_FDMA_XTR_HDR_SIZE_BYTES, but which
    // is not true because the IFH doesn't exist (in fact, it's the IFH that
    // we are trying to construct here).
    // The DMAC is located at sof->ifh_ptr + 4.

    // Start by stripping the VStaX header from the payload.
    // The payload includes the Vitesse Ethertype. Give up if this is
    // not true.
    memcpy(vstax_hdr, sof->ifh_ptr + 4 /* For extraction queue number */ + 6 /* for DMAC */ + 6 /* for SMAC */, VTSS_VSTAX_HDR_SIZE);
    etype = (vstax_hdr[0] << 8) | (vstax_hdr[1] << 0);
    FDMA_ASSERT(IRQ, etype == VTSS_ETYPE_VTSS, return FALSE;);

    // Shrink the payload by copying DMAC and SMAC to old position of VStaX header.
    // But first update FDMA_USER_DCB(sof(->frm_ptr to point to the new location of the DMAC.
    FDMA_USER_DCB(sof)->frm_ptr = sof->ifh_ptr         + /* Take offset in the IFH pointer, which points to the 4-byte extraction queue number */
                                  4                    + /* skip across extraction queue number */
                                  VTSS_VSTAX_HDR_SIZE;   /* and across the number of bytes occupied by the VStaX header that we're currently attempting to remove */

    // Don't use memcpy(), since the two areas may be overlapping.
    src_ptr = sof->ifh_ptr + 4 + SIZE_OF_TWO_MAC_ADDRS - 1;
    dst_ptr = src_ptr + VTSS_VSTAX_HDR_SIZE;
    for (i = SIZE_OF_TWO_MAC_ADDRS - 1; i >= 0; i--) {
        *(dst_ptr--) = *(src_ptr--);
    }

    // Then it's time to create the artificial IFH. It may seem like crossing the river to get water, because it's
    // most likely that the application will call vtss_fdma_xtr_hdr_decode() in just a second to get this information
    // decoded, but what don't we do to provide a uniform interface to the application?

    // First update the IFH pointer.
    sof->ifh_ptr = FDMA_USER_DCB(sof)->frm_ptr - VTSS_FDMA_XTR_HDR_SIZE_BYTES;

    // We have an internal convention in this file, that the chip number is saved
    // in the MSByte of the IFH, which is not used for anything anyway.
    sof->ifh_ptr[0] = chip_no;
    sof->ifh_ptr[1] = (xtr_qu >> 16) & 0xFF;
    sof->ifh_ptr[2] = (xtr_qu >>  8) & 0xFF;
    sof->ifh_ptr[3] = (xtr_qu >>  0) & 0xFF;

    memset(&sof->ifh_ptr[4], 0, 8); // First 8 bytes are not used.
    // Then the VStaX header that we just stripped from the payload. Omit the Ethertype.
    memcpy(&sof->ifh_ptr[12], &vstax_hdr[2], VTSS_VSTAX_HDR_SIZE - 2);
    // Finally the fwd part.
    fwd  = VTSS_ENCODE_BITFIELD64(1, 45, 1); // FWD.VSTAX_AVAIL = 1
    fwd |= VTSS_ENCODE_BITFIELD64(src_port, 29, 6);
    fwd |= VTSS_ENCODE_BITFIELD64(1, 26, 3); // Simulate an extraction queue, or this frame will be classified as an ACL Hit. The queue number will not be correct (since we only have one bit to indicate two stack queues in the classification_result[])
    sof->ifh_ptr[22] = fwd >> 40;
    sof->ifh_ptr[23] = fwd >> 32;
    sof->ifh_ptr[24] = fwd >> 24;
    sof->ifh_ptr[25] = fwd >> 16;
    sof->ifh_ptr[26] = fwd >>  8;
    sof->ifh_ptr[27] = fwd >>  0;

    // And finally update the length. The current act_len includes:
    //   Four bytes of xtr_qu number
    //   VTSS_VSTAX_HDR_SIZE bytes of VStaX header
    //   Remainder of payload + FCS
    // The new act_len must include:
    //   VTSS_FDMA_XTR_HDR_SIZE_BYTES of extraction header (including four bytes of xtr_qu number)
    //   Remainder of payload + FCS
    FDMA_USER_DCB(sof)->act_len -= (VTSS_VSTAX_HDR_SIZE + 4 /* For xtr_qu, which is included in VTSS_FDMA_XTR_HDR_SIZE_BYTES below */);
    FDMA_USER_DCB(sof)->act_len += VTSS_FDMA_XTR_HDR_SIZE_BYTES;

    return TRUE; // Don't drop
}
#endif /* defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) */

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
/*****************************************************************************/
// jr_src_port()
/*****************************************************************************/
static FDMA_INLINE u16 jr_src_port(u8 *ifh_ptr)
{
    // The source port is located in the extraction IFH's FWD field, bits [34; 29].
    // The MSByte of IFH.FWD is 4 (for xtr qu #) + 8 (for IFH.FWD_HANDLING) + 10 (for IFH.VSTAX).
    u16 src_port = (((ifh_ptr[23] << 8) | (ifh_ptr[24] << 0)) >> 5) & 0x3F;
    return src_port;
}
#endif /* defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) */

#if defined(VTSS_ARCH_JAGUAR_1)
/*****************************************************************************/
// jr_xtr_qu()
/*****************************************************************************/
static FDMA_INLINE u32 jr_xtr_qu(u8 *ifh_ptr)
{
    u32 xtr_qu = (ifh_ptr[0] << 24) | (ifh_ptr[1] << 16) | (ifh_ptr[2] << 8) | (ifh_ptr[3] << 0);
    return xtr_qu;
}
#endif /* VTSS_ARCH_JAGUAR_1 */

/*****************************************************************************/
// stat_rx_update()
/*****************************************************************************/
static FDMA_INLINE void stat_rx_update(fdma_state_t *state, vtss_chip_no_t chip_no, u32 xtr_qu, BOOL drop)
{
    if (xtr_qu >= VTSS_PACKET_RX_QUEUE_CNT) {
        // What to do if the chip reports an invalid queue?
        xtr_qu = VTSS_PACKET_RX_QUEUE_CNT - 1;
    }
    if (drop) {
        state->fdma_stats.xtr_qu_drops[xtr_qu][chip_no]++;
    } else {
        state->fdma_stats.xtr_qu_packets[xtr_qu][chip_no]++;
    }
}

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
/*****************************************************************************/
// jr_drop_check()
/*****************************************************************************/
static FDMA_INLINE BOOL jr_drop_check(vtss_state_t *vstate, fdma_sw_dcb_t *dcb, u32 xtr_qu, vtss_chip_no_t chip_no)
{
    vtss_fdma_list_t *user_dcb = FDMA_USER_DCB(dcb);

    if ((user_dcb->frm_ptr[ 0] == 0xFF && user_dcb->frm_ptr[ 1] == 0xFF && user_dcb->frm_ptr[ 2] == 0xFF  &&
         user_dcb->frm_ptr[ 3] == 0xFF && user_dcb->frm_ptr[ 4] == 0xFF && user_dcb->frm_ptr[ 5] == 0xFF) &&
       ((user_dcb->frm_ptr[12] == 0x08 && user_dcb->frm_ptr[13] == 0x00)  ||
        (user_dcb->frm_ptr[12] == 0x86 && user_dcb->frm_ptr[13] == 0xDD)) &&
        ((dcb->ifh_ptr[16] >> 4) & 0x7) == 5) {
        // IPv4/IPv6 frames sent to the B/C MAC address come to the CPU multiple times on 48-ported
        // devices and stackable builds, because of a special "feature" in the L3 RTL (documented in Bugzilla#9547#c13).
        // We drop such frames if they were forwarded as <GCPU, UPS> (last check in above if()).
        return TRUE;
    }

    if (vstate->chip_count == 2 && vstate->port.mask_int_ports != 0) {
        // We need to check if this frame was received on an internal port. If so, discard it, because it
        // is also received on the partner chip.
        u16 src_port = jr_src_port(dcb->ifh_ptr);
        u32 src_port_mask = VTSS_BIT(src_port);

        if ((vstate->port.mask_int_ports & src_port_mask) != 0) {
            // There is a couple of exceptions here:
            if (xtr_qu == vstate->packet.rx_conf.map.sflow_queue) {
                // If frame is received by the CPU due to enabled sFlow, don't drop it.
                // Example: Enable Tx SFlow on a port on the secondary chip and then transmit a broadcast
                // frame to a port on the primary chip.
                // When it hits the port on the secondary chip, and is subject to SFlow sampling, it will
                // be copied to the CPU on the secondary chip, but it will look as if it arrived to the
                // secondary chip on one of the internal ports. Therefore, we must pass these frames through
                // to higher layers.
                // The below sflow_queue comparison works even for non-sflow-enabled builds, because sflow_queue defaults
                // to VTSS_PACKET_RX_QUEUE_NONE, which can't be the same as a physical queue.
                return FALSE; // Don't drop it.
            } else if (xtr_qu == vstate->packet.rx_conf.map.stack_queue || xtr_qu == vstate->packet.rx_conf.map.l3_uc_queue || xtr_qu == vstate->packet.rx_conf.map.l3_other_queue) {
                u16 new_src_port;
                u32 vstax_src, src_addr_mode, src_port_type;

                // If frame is received by the CPU in the stack queue, L3 queue or L3_other queues,
                // it's either a stack management frame or a frame sent to the management address (then one of the L3 queues).
                // Such frames go directly to the primary device's CPU, and not to the secondary device's CPU,
                // since they are forwarded using VTSS_VSTAX_FWD_MODE_CPU_UPSID, with an UPSID matching the
                // primary device.
                if (chip_no == 1) {
                    // Drop all frames received on the secondary device.
                    return TRUE;
                }

                vstax_src = ((u32)dcb->ifh_ptr[20] << 8) | ((u32)dcb->ifh_ptr[21] << 0);
                src_addr_mode = VTSS_EXTRACT_BITFIELD(vstax_src, 10, 1);
                if (src_addr_mode == 0) {
                    src_port_type = VTSS_EXTRACT_BITFIELD(vstax_src, 11, 1);
                    if (src_port_type == 0) {
                        // Originally received on <upsid, upspn>
                        u8 upsid = VTSS_EXTRACT_BITFIELD(vstax_src, 5, 5);

                        if (vstate->l2.vstax_info.upsid[1] == upsid) {
                            // Originally received on a port on the secondary unit, and not
                            // on a port connected to a switch connected to the secondary
                            // unit's external stack port.
                            new_src_port = VTSS_EXTRACT_BITFIELD(vstax_src, 0, 5);
                        } else {
                            // Received on a non-local switch (i.e. a switch connected to
                            // the secondary unit's external stack link.
                            // We have no other option than telling the upper layers that it
                            // is received on the secondary unit's external stack port.
                            u32 mask = vstate->l2.vstax_info.chip_info[1].mask_a;
                            if (mask == 0) {
                                return TRUE; // No such port. Drop it.
                            }
                            new_src_port = VTSS_OS_CTZ(mask);
                        }
                    } else {
                        // Originally sent from remote CPU. Pretend it was received
                        // on secondary unit's external stack linexternal stack link.
                        u32 mask = vstate->l2.vstax_info.chip_info[1].mask_a;
                        if (mask == 0) {
                            return TRUE; // No such port. Drop it.
                        }
                        new_src_port = VTSS_OS_CTZ(mask);
                    }
                } else {
                    // src_addr_mode == 1 => originally received on a GLAG.
                    vtss_glag_no_t glag_no = VTSS_EXTRACT_BITFIELD(vstax_src, 0, 5);
                    vtss_port_no_t port_no;
                    BOOL           found = FALSE;

                    new_src_port = 0; /* Satisfy lint */

                    // Find the first port that is up and is member of this GLAG on chip #1.
                    for (port_no = VTSS_PORT_NO_START; port_no < vstate->port_count; port_no++) {
                        if (vstate->l2.port_glag_no[port_no] == glag_no && VTSS_CHIP_NO_FROM_STATE(vstate, port_no) == 1 && vstate->l2.port_state[port_no]) {
                            new_src_port = VTSS_CHIP_PORT_FROM_STATE(vstate, port_no);
                            found = TRUE;
                            break;
                        }
                    }

                    if (!found) {
                       // No such GLAG on the secondary chip. It is probably received
                       // on a non-local switch (i.e. on a switch connected to the
                       // secondary unit's external stack link). We have no other option
                       // than telling the upper layers that is is received on the secondary
                       // unit's external stack port.
                       u32 mask = vstate->l2.vstax_info.chip_info[1].mask_a;
                       if (mask == 0) {
                           return TRUE; // Drop it. Port may have gone down just in the same second that we received the frame.
                       }
                       new_src_port = VTSS_OS_CTZ(mask);
                    }
                }

                // Encode it back into the frame. See jr_src_port() for bit position explanations.
                dcb->ifh_ptr[23] &= 0xf8;                        // Clear 3 lsbits.
                dcb->ifh_ptr[23] |= (new_src_port >> 3) & 0x07;  // Get the 3 msbits of new_src_port and place them at the 3 lsbits of [23]
                dcb->ifh_ptr[24] &= 0x1f;                        // Clear 3 msbits.
                dcb->ifh_ptr[24] |= (new_src_port << 5) & 0xe0;  // Get the 3 lsbits of new_src_port and place them at the 3 msbits of [24]

                // We have an internal convention in this file, that the chip number is saved
                // in the MSByte of the IFH, which is not used for anything anyway.
                // In our case, we must pretend that the frame is received on the external stack link on the secondary chip.
                dcb->ifh_ptr[0] = 1;

                return FALSE; // Don't drop it.
            } else {
                // Drop it.
                return TRUE;
            }
        }
    }

    return FALSE;
}
#endif /* VTSS_ARCH_JAGUAR_1 && VTSS_FEATURE_VSTAX */

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
/*****************************************************************************/
// jr_drop_reinject_check()
/*****************************************************************************/
static void jr_drop_reinject_check(vtss_state_t *vstate, fdma_sw_dcb_t *sw_dcb, u32 xtr_qu, vtss_chip_no_t chip_no, BOOL *drop_it, BOOL *reinject_it)
{
    *drop_it     = FALSE;
    *reinject_it = FALSE;

    if (xtr_qu == 8 || xtr_qu == 9) {
        if (chip_no == 0) {
            // Jaguar does not provide an IFH for frames received with super priority, so
            // to make the interface towards the application uniform, we insert one here.
            // "Luckily" we've made room for such an artificial IFH.
            if (!xtr_super_prio_ifh_synthesize(vstate, xtr_qu, sw_dcb)) {
                // VStaX configuration doesn't match, or is not yet stable.
                *drop_it = TRUE;
            }
        } else {
            *drop_it = TRUE;
        }
    } else if (xtr_qu == vstate->packet.rx_conf.map.lrn_all_queue) {
        // Learn-all frames should be reinjected if we're in stacking mode, otherwise dropped.
        vtss_vstax_chip_info_t *chip_info = &vstate->l2.vstax_info.chip_info[chip_no];
        BOOL stacking = chip_info->mask_a != 0 || chip_info->mask_b != 0;
        if (stacking) {
            *reinject_it = TRUE;
        } else {
            *drop_it = TRUE;
        }
    } else {
        // If not received as a super-priority frame (but a normal frame), we need to check
        // if this frame was received on an internal port. If so, discard it, because it
        // is also received in one of the secondary chip's extraction queues (which is not
        // the case for SP frames).
        // Also frames with B/C DMAC and EtherType == IPv4/IPv6 must be dropped under certain
        // circumstances.
        *drop_it = jr_drop_check(vstate, sw_dcb, xtr_qu, chip_no);
    }
}
#endif /* VTSS_ARCH_JAGUAR_1 && VTSS_FEATURE_VSTAX */

#if defined(VTSS_ARCH_JAGUAR_1)
/*****************************************************************************/
// vcoreiii_fdma_xtr_qu_suspend_set()
/*****************************************************************************/
static void vcoreiii_fdma_xtr_qu_suspend_set(vtss_state_t *const vstate, u32 xtr_qu, BOOL do_suspend)
{
    vtss_chip_no_t chip_no;

    for (chip_no = 0; chip_no < vstate->chip_count; chip_no++) {
        // On JR, we will stop Rx on a specific queue at the OQS level.
        // This means that frames already in the OQS will be extracted before
        // the queue will become empty.
        u32 val;
        (void)vstate->init_conf.reg_read(chip_no, VTSS_OQS_QU_RAM_CFG_QU_RC_CFG(456 + xtr_qu), &val);
        if (do_suspend) {
            val &= ~VTSS_F_OQS_QU_RAM_CFG_QU_RC_CFG_RX_QU_ENA;
        } else {
            val |= VTSS_F_OQS_QU_RAM_CFG_QU_RC_CFG_RX_QU_ENA;
        }
        (void)vstate->init_conf.reg_write(chip_no, VTSS_OQS_QU_RAM_CFG_QU_RC_CFG(456 + xtr_qu), val);
    }
}
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_LUTON26)
/*****************************************************************************/
// vcoreiii_fdma_xtr_qu_suspend_set()
/*****************************************************************************/
static void vcoreiii_fdma_xtr_qu_suspend_set(vtss_state_t *const vstate, vtss_packet_rx_queue_t xtr_qu, BOOL do_suspend)
{
    // On Lu26 there are two CPU ports (physical #26 and #27). When running only on
    // the on-chip, internal CPU, we only use #26. However, some applications may
    // want to divide the extraction queues among them and forward some queues to
    // #26 and others to #27, which can then be read by the external CPU.
    // If enabling throttling, the internal CPU will suspend a given queue by
    // moving that queue from port #26 to port #27, which might have some
    // unexpected side-effects on an external CPU reading port #27.
    u32 val;

    // First disable dequeing from port #27
    (void)vstate->init_conf.reg_read(0, VTSS_SYS_SYSTEM_PORT_MODE(27), &val);
    val |= VTSS_F_SYS_SYSTEM_PORT_MODE_DEQUEUE_DIS;
    (void)vstate->init_conf.reg_write(0, VTSS_SYS_SYSTEM_PORT_MODE(27), val);

    // Then either redirect the queue to port #26 (ourselves) or port #27 (suspend)
    (void)vstate->init_conf.reg_read(0, VTSS_SYS_SCH_SCH_CPU, &val);
    if (do_suspend) {
        // Move queue #xtr_qu to port #27
        val |= VTSS_F_SYS_SCH_SCH_CPU_SCH_CPU_MAP(VTSS_BIT(xtr_qu));
    } else {
        // Move queue #xtr_qu back to port #26.
        val &= ~VTSS_F_SYS_SCH_SCH_CPU_SCH_CPU_MAP(VTSS_BIT(xtr_qu));
    }
    (void)vstate->init_conf.reg_write(0, VTSS_SYS_SCH_SCH_CPU, val);
}
#endif /* VTSS_ARCH_LUTON26 */

/*****************************************************************************/
// vcoreiii_fdma_inj_restart_ch()
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_inj_restart_ch(vtss_state_t *const vstate, vtss_fdma_ch_t inj_ch, fdma_sw_dcb_t *head, BOOL do_start, BOOL thread_safe)
{
    if (do_start) {
        FDMA_CHECK_RC(thread_safe, (VCOREIII_RD(vstate, VTSS_FDMA_MISC_CH_EN_REG) & VTSS_BIT(inj_ch)) == 0);

        inj_ch_init(vstate, FDMA_HW_DCB(head), inj_ch);
        enable_ch(vstate, inj_ch);
    }

    return VTSS_RC_OK;
}

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// jr_reinject_done()
/*****************************************************************************/
static void jr_reinject_done(void *cntxt, vtss_fdma_list_t *head, vtss_fdma_ch_t inj_ch, BOOL dropped)
{
    // Gotta give the frame back to the extraction channel that it came from.
    // In jr_reinject_lrn_all() we saved the xtr_ch in the first part of head->data.
    vtss_fdma_ch_t   xtr_ch = *((vtss_fdma_ch_t *)FDMA_SW_DCB(head)->data);
    vtss_state_t     *vstate = cntxt;
    fdma_ch_state_t  *ch_state = &vstate->fdma_state.fdma_ch_state[xtr_ch];

    FDMA_ASSERT(IRQ, xtr_ch >= 0 && xtr_ch < VTSS_FDMA_CH_CNT, return;);
    FDMA_ASSERT(IRQ, ch_state->usage == VTSS_FDMA_CH_USAGE_XTR, return;);
    FDMA_ASSERT(IRQ, head->next == NULL, return;);

    // Initialize the new list's DCB areas
    (void)vtss_fdma_cmn_dcb_release(vstate, xtr_ch, FDMA_SW_DCB(head), TRUE /* thread safe */, TRUE /* connect */, TRUE /* restart channel */);
}
#endif /* defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
/*****************************************************************************/
// jr_reinject_lrn_all()
// Due to a chip-bug, we have to do S/W-assisted H/W learning.
// Learn-all frames are forwarded to a CPU queue that ain't used for anything
// else. Once this FDMA driver gets the frame, it forwards it in a special
// way to the stack ports in order to learn the SMAC on other switches in the
// stack - or in case of JR-48: On the opposite chip.
/*****************************************************************************/
static fdma_sw_dcb_t *jr_reinject_lrn_all(vtss_state_t *vstate, vtss_fdma_ch_t xtr_ch, fdma_sw_dcb_t *frm)
{
    fdma_sw_dcb_t        *new_tail;
    u32                  dcb_cnt;
    vtss_fdma_ch_t       ch;
    vtss_packet_tx_grp_t inj_grp = 0; // Must go through the analyzer, in order to get the frame to potentially two stack ports with REW updates. The frame will not be analyzed.
    u64                  dst;
    u16                  vstax_hi;
    u64                  vstax_lo;
    u64                  fwd;
    u32                  dst_port_mask;
    int                  full_sz_bytes;
    u8                   *ifh_ptr;

#if VTSS_OPT_FDMA_VER <= 2
    fdma_state_t         *state = &vstate->fdma_state;

    // Find a suitable channel to inject the frame on
    for (ch = 0; ch < VTSS_FDMA_CH_CNT; ch++) {
        if (state->fdma_ch_state[ch].usage == VTSS_FDMA_CH_USAGE_INJ && (state->fdma_ch_state[ch].inj_grp_mask & VTSS_BIT(inj_grp))) {
            break;
        }
    }

    FDMA_ASSERT(IRQ, ch < VTSS_FDMA_CH_CNT, return frm;);
#else
    ch = VTSS_FDMA_CH_INJ; // We only have one.
#endif /* VTSS_OPT_FDMA_VER */

    // We're always reinjecting frames on the primary unit in a 48-port solution, so get the stack
    // ports as seen from chip_no == 0.
    dst_port_mask = vstate->l2.vstax_info.chip_info[0].mask_a; // External stack port on primary unit.

    if (vstate->chip_count == 2) {
        // If chip_count is 2, mask_b has two bits set, but we should only send it once.
        dst_port_mask |= VTSS_BIT(vstate->port.port_int_0); // Interconnect port on primary unit.
    } else {
        // If chip_count is != 2, mask_b only has one bit set.
        dst_port_mask |= vstate->l2.vstax_info.chip_info[0].mask_b; // External stack port on primary unit in 24-port solutions.
    }

    FDMA_ASSERT(IRQ, dst_port_mask != 0, return frm;);
    dst = VTSS_ENCODE_BITFIELD64(dst_port_mask, 0, 32); // IFH.DST.MC_INJECT_DST

    fwd  = VTSS_ENCODE_BITFIELD64(1, 45, 1); // FWD.VSTAX_AVAIL = 1
    fwd |= VTSS_ENCODE_BITFIELD64(1, 44, 1); // Enforce update of FCS.
    // fwd.do_not_rew = 0: Force REW to update frame's TTL.
    fwd |= VTSS_ENCODE_BITFIELD64(3, 16, 2); // FWD.INJECT_MODE = INJECT_MULTICAST

    vstax_hi  = VTSS_ENCODE_BITFIELD  ( 1, 15, 1); // MSBit must be 1
    vstax_lo  = VTSS_ENCODE_BITFIELD64( 7, 56, 3); // VStaX.QoS.cl_qos = qos_class/iprio = 7.
    vstax_lo |= VTSS_ENCODE_BITFIELD64(31, 48, 5); // VStaX.TTL = 31 => ask REW to insert actual port TTLs.
    // VStaX.lrn_mode = 0 => Normal Learning. SMAC and VID of the frame are subject to learning.
    vstax_lo |= VTSS_ENCODE_BITFIELD64( 7, 44, 3); // VStaX.FWD_MODE = 7 = Reserved = Inject Stack (normally not needed by S/W, hence reserved).

    // Cater for differences in extraction and injection IFH sizes.
    frm->ifh_ptr                += (VTSS_FDMA_XTR_HDR_SIZE_BYTES - VTSS_FDMA_INJ_HDR_SIZE_BYTES);
    FDMA_USER_DCB(frm)->act_len -= (VTSS_FDMA_XTR_HDR_SIZE_BYTES - VTSS_FDMA_INJ_HDR_SIZE_BYTES);
    ifh_ptr = frm->ifh_ptr;

    ifh_ptr[ 0] = dst      >> 56;
    ifh_ptr[ 1] = dst      >> 48;
    ifh_ptr[ 2] = dst      >> 40;
    ifh_ptr[ 3] = dst      >> 32;
    ifh_ptr[ 4] = dst      >> 24;
    ifh_ptr[ 5] = dst      >> 16;
    ifh_ptr[ 6] = dst      >>  8;
    ifh_ptr[ 7] = dst      >>  0;
    ifh_ptr[ 8] = vstax_hi >>  8;
    ifh_ptr[ 9] = vstax_hi >>  0;
    ifh_ptr[10] = vstax_lo >> 56;
    ifh_ptr[11] = vstax_lo >> 48;
    ifh_ptr[12] = vstax_lo >> 40;
    ifh_ptr[13] = vstax_lo >> 32;
    // Don't overwrite the last four bytes of the VStaX header,
    // since this contains info about the frame's origin (i.e.
    // source port and VID).
    // ifh_ptr[14] = vstax_lo >> 24;
    // ifh_ptr[15] = vstax_lo >> 16;
    // ifh_ptr[16] = vstax_lo >>  8;
    // ifh_ptr[17] = vstax_lo >>  0;
    ifh_ptr[18] = fwd >> 40;
    ifh_ptr[19] = fwd >> 32;
    ifh_ptr[20] = fwd >> 24;
    ifh_ptr[21] = fwd >> 16;
    ifh_ptr[22] = fwd >>  8;
    ifh_ptr[23] = fwd >>  0;

    // Due to Bugzilla#9545, we need to insert the broadcast
    // MAC address as DMAC and a non-IP and non-length ethertype.
    memset(&ifh_ptr[24], 0xFF, 6);
    ifh_ptr[36] = 0x12;
    ifh_ptr[37] = 0x12;

#if VTSS_OPT_FDMA_VER <= 2
    frm->inj_post_cb = jr_reinject_done;

    // Gotta cheat inj_dcb_init(), since it requires the length including the unused gap before frm->ifh_ptr.
    FDMA_USER_DCB(frm)->act_len += (frm->ifh_ptr - frm->data);

    new_tail = inj_dcb_init(inj_grp, frm, frm->ifh_ptr - frm->data, VTSS_FDMA_INJ_HDR_SIZE_BYTES, &full_sz_bytes, &dcb_cnt, TRUE, state->fdma_ch_state[ch].usage, FALSE);

    // Save the xtr_ch so that jr_reinject_done() can find it when releasing the DCBs.
    // There's room in the beginning of frm->data.
    FDMA_ASSERT(IRQ, frm->ifh_ptr - frm->data >= (int)sizeof(vtss_fdma_ch_t), return frm;);
    *((vtss_fdma_ch_t *)frm->data) = xtr_ch;

#else
    // Gotta cheat inj_dcb_init(), since it requires the length excluding IFH and excluding FCS
    FDMA_USER_DCB(frm)->act_len -= (FDMA_USER_DCB(frm)->frm_ptr - frm->ifh_ptr + 4);

    FDMA_ASSERT(IRQ, inj_dcb_init(vstate, frm, inj_grp, &new_tail, &dcb_cnt, VTSS_FDMA_DCB_TYPE_INJ, TRUE, FALSE, TRUE) == VTSS_RC_OK, return frm;);
    FDMA_ASSERT(IRQ, new_tail != NULL, return frm;);
    full_sz_bytes = FDMA_USER_DCB(frm)->act_len + 4 /* for FCS */ + VTSS_FDMA_INJ_HDR_SIZE_BYTES; // Just used for statistics
#endif /* VTSS_OPT_FDMA_VER */

    FDMA_ASSERT(IRQ, new_tail != NULL, return frm;);

    VTSS_IG_HEX(VTSS_TRACE_GROUP_FDMA_IRQ, frm->ifh_ptr, 64);

    if (vtss_fdma_cmn_inj_attach_frm(vstate, frm, new_tail, ch, dcb_cnt, full_sz_bytes - VTSS_FDMA_INJ_HDR_SIZE_BYTES, NULL, TRUE) != VTSS_RC_OK) {
        return frm;
    }
    return NULL; // Success.
}
#endif /* defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) */

/*****************************************************************************/
// xtr_irq_block_done()
/*****************************************************************************/
static FDMA_INLINE BOOL xtr_irq_block_done(vtss_state_t *const vstate, void *const cntxt, vtss_fdma_ch_t xtr_ch)
{
    int                    xtr_cnt_dwords, total_frm_size_bytes_incl_ifh, frag_size_bytes;
    fdma_ch_state_t        *ch_state;
    fdma_hw_dcb_t          *hw_dcb;
    fdma_state_t           *state = &vstate->fdma_state;
    vtss_fdma_list_t       *new_list, *new_head, *temp_list;
    u32                    ctl1, dstat;
    u32                    xtr_qu;
    u32                    offset_into_data = VTSS_FDMA_HDR_SIZE_BYTES - VTSS_FDMA_XTR_HDR_SIZE_BYTES;
    BOOL                   sof, eof, drop_it, reinject_it, aborted;
    VTSS_OS_TIMESTAMP_TYPE sw_tstamp;
#if VTSS_OPT_FDMA_VER >= 3
    vtss_chip_no_t         chip_no = 0; // Received on chip_no 0, unless we pretend it's received on another below.
#endif /* VTSS_OPT_FDMA_VER >= 3 */
#if VTSS_OPT_FDMA_DEBUG
    int                    debug_sum;
#endif /* VTSS_OPT_FDMA_DEBUG */

    VTSS_NG(VTSS_TRACE_GROUP_FDMA_IRQ, "xtr_irq_block_done(ch = %d)\n", xtr_ch);
    FDMA_ASSERT(IRQ, xtr_ch >= 0 && xtr_ch < VTSS_FDMA_CH_CNT,    return FALSE;);
    ch_state = &state->fdma_ch_state[xtr_ch];
    FDMA_ASSERT(IRQ, ch_state->status != FDMA_CH_STATUS_DISABLED, return FALSE;);
    FDMA_ASSERT(IRQ, ch_state->usage  == VTSS_FDMA_CH_USAGE_XTR,  return FALSE;);

    // Iterate over all DCBs and stop when we reach one with the DONE bit cleared.
    while (ch_state->cur_head) {
        hw_dcb = FDMA_HW_DCB(ch_state->cur_head);

        // Before dereferencing the DCB, we must invalidate the cache line(s).
        VTSS_OS_DCACHE_INVALIDATE(hw_dcb, VTSS_FDMA_DCB_SIZE_BYTES);
        VTSS_OS_REORDER_BARRIER();

        // The order that the GPDMA writes back status fields is: First write back hw_dcb->ctl1, then write back hw_dcb->dstat.
        // This means that we have to check DSTAT.VLD field before the CTL1.DONE bit.

        // Get the status information for this DCB. This information was automatically
        // fetched by the General Purpose DMA from the VTSS_ICPU_CFG_GPDMA_FDMA_XTR_STAT_LAST_DCB[xtr_grp]
        // register after the transfer completed.
        // This means that we can use that register's bit definitions when referring to
        // bits within the dstat.
        dstat = BUS_TO_CPU(hw_dcb->stat);

        // If DSTAT.VLD is not set, we've consumed all the frames, and we're done.
        if ((dstat & VTSS_F_ICPU_CFG_GPDMA_FDMA_XTR_STAT_LAST_DCB_XTR_STAT_VLD) == 0) {
            break;
        }

        state->fdma_stats.dcbs_used[xtr_ch]++;

        ctl1 = BUS_TO_CPU(hw_dcb->ctl1);

        // When DSTAT.VLD == 1, then CTL1.DONE must be 1 as well.
        FDMA_ASSERT(IRQ, ctl1 & VTSS_F_FDMA_CH_CTL1_DONE, VTSS_EG(VTSS_TRACE_GROUP_FDMA_IRQ, "ch=%d, ctl1=0x%08x", xtr_ch, ctl1); return FALSE;);

        // Bits 11:0 of dcb->ctl1 indicate the number of 32-bit words that the GPDMA
        // has written to the data area (not necessarily the number of valid frame words,
        // because the GPDMA always attempts to burst from the FDMA logic).
        xtr_cnt_dwords = VTSS_X_FDMA_CH_CTL1_BLOCK_TS(ctl1);
        VTSS_OS_DCACHE_INVALIDATE(ch_state->cur_head->data + offset_into_data, 4 * xtr_cnt_dwords);
        VTSS_OS_REORDER_BARRIER();

        // We don't expect zero or more than alloc_len bytes to having been extracted to the data area.
        FDMA_ASSERT(IRQ, xtr_cnt_dwords > 0 && 4 * (u32)xtr_cnt_dwords <= ch_state->cur_head->alloc_len - offset_into_data, VTSS_EG(VTSS_TRACE_GROUP_FDMA_IRQ, "ch=%d, xtr_cnt_dwords=%d, alloc_len=%d", xtr_ch, xtr_cnt_dwords, ch_state->cur_head->alloc_len); return FALSE;);

        aborted = (dstat & VTSS_F_ICPU_CFG_GPDMA_FDMA_XTR_STAT_LAST_DCB_XTR_STAT_ABORT) != 0;

        // DSTAT.FRM_LEN is the accummulated frame length (including IFH).
        // This means that we must loop through all fragments of this frame to figure out how much data
        // is actually stored for this particular DCB (which need not be the SOF DCB).
        total_frm_size_bytes_incl_ifh = frag_size_bytes = VTSS_X_ICPU_CFG_GPDMA_FDMA_XTR_STAT_LAST_DCB_XTR_STAT_FRM_LEN(dstat);
        temp_list = FDMA_USER_DCB(ch_state->pend_head);
        while (temp_list) {
            FDMA_ASSERT(IRQ, (u32)frag_size_bytes > temp_list->act_len, return FALSE;);
            frag_size_bytes -= temp_list->act_len;
            temp_list = temp_list->next;
        }

        // Now, frag_size_bytes contains this fragment's length.
        FDMA_ASSERT(IRQ, frag_size_bytes > 0 && 4 * xtr_cnt_dwords >= frag_size_bytes, return FALSE;);
        FDMA_USER_DCB(ch_state->cur_head)->act_len = frag_size_bytes;

        sof = (dstat & VTSS_F_ICPU_CFG_GPDMA_FDMA_XTR_STAT_LAST_DCB_XTR_STAT_SOF) != 0;
        eof = (dstat & VTSS_F_ICPU_CFG_GPDMA_FDMA_XTR_STAT_LAST_DCB_XTR_STAT_EOF) != 0;

        if (sof) {
            ch_state->cur_head->ifh_ptr = ch_state->cur_head->data + offset_into_data;
            FDMA_USER_DCB(ch_state->cur_head)->frm_ptr = ch_state->cur_head->ifh_ptr + VTSS_FDMA_XTR_HDR_SIZE_BYTES;
        } else {
            FDMA_USER_DCB(ch_state->cur_head)->frm_ptr = ch_state->cur_head->ifh_ptr = ch_state->cur_head->data + offset_into_data;
        }

        // If we're receiving a jumbo frame (or a frame that is larger
        // than what we've reserved of space in the data area), we may
        // not have received it all. If the EOF flag in the DCB is not
        // set, this is the case, and we need to receive the remainder of
        // the frame before passing it on.
        if (eof) {
            // EOF reached. Pass the frame on to the callback function.

            // Pass the frame on to the application-level handler, but timestamp it first.
            VTSS_OS_TIMESTAMP(&sw_tstamp);

            // Terminate the current list item before passing it on, but save a reference to the new head.
            new_head = FDMA_USER_DCB(ch_state->cur_head)->next;
            FDMA_USER_DCB(ch_state->cur_head)->next = NULL;

            if (!ch_state->pend_head) {
                // The frame is not part of a jumbo frame, so we expect the SOF flag to be set in the DCB.
                FDMA_ASSERT(IRQ, sof, return FALSE;);

                // For further, unified processing, we move the current DCB to the pending list.
                ch_state->pend_head = ch_state->cur_head;
            } else {
                // The frame is part of a jumbo frame. In that case, the SOF flag can't be set, because the SOF is
                // already received and moved to the pending list.
                FDMA_ASSERT(IRQ, !sof, return FALSE;);

                // Attach the current list item to the end of the pending list.
                temp_list = FDMA_USER_DCB(ch_state->pend_head);
                while (temp_list->next) {
                    temp_list = temp_list->next;
                }
                temp_list->next = FDMA_USER_DCB(ch_state->cur_head);
            }

            state->fdma_stats.packets[xtr_ch]++;
            // Don't include IFH in packet byte counter
            state->fdma_stats.bytes[xtr_ch] += (total_frm_size_bytes_incl_ifh - VTSS_FDMA_XTR_HDR_SIZE_BYTES);

#if defined(VTSS_ARCH_LUTON26)
            {
                // Find the physical queue that this frame was forwarded onto. This is the most significant bit of
                // the IFH.cpu_queue_mask.
                u32 xtr_qu_mask = (((ch_state->pend_head->data[4 + offset_into_data] << 8) | (ch_state->pend_head->data[5 + offset_into_data])) >> 4) & 0xFF;
                xtr_qu = xtr_qu_from_mask(xtr_qu_mask);
            }
#endif /* VTSS_ARCH_LUTON26 */

            drop_it = FALSE;
            reinject_it = FALSE;

#if defined(VTSS_ARCH_JAGUAR_1)
            {
                xtr_qu = jr_xtr_qu(ch_state->pend_head->ifh_ptr);
#if defined(VTSS_FEATURE_VSTAX)
                jr_drop_reinject_check(vstate, ch_state->pend_head, xtr_qu, 0, &drop_it, &reinject_it);
#if VTSS_OPT_FDMA_VER >= 3
                // Internal convention is to save the chip number in ifh_ptr[0].
                chip_no = ch_state->pend_head->ifh_ptr[0];
#endif /* VTSS_OPT_FDMA_VER >= 3 */
#endif /* VTSS_FEATURE_VSTAX */
            }
#endif /* VTSS_ARCH_JAGUAR_1 */

            vtss_fdma_cmn_throttle_suspend_check(vstate, xtr_qu, total_frm_size_bytes_incl_ifh - VTSS_FDMA_XTR_HDR_SIZE_BYTES);
            stat_rx_update(state, 0, xtr_qu, drop_it || aborted);

            if (drop_it || aborted) {
                // Don't send to xtr_cb(), but re-cycle the DCBs.
                new_list = FDMA_USER_DCB(ch_state->pend_head);

                // Print an error if aborted
                FDMA_ASSERT(IRQ, aborted == FALSE, VTSS_EG(VTSS_TRACE_GROUP_FDMA_IRQ, "ch=%d, xtr_qu=%u, dstat=0x%08x", xtr_ch, xtr_qu, dstat););
            } else if (reinject_it) {
#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
                // Re-using current list to re-inject. Will be given back once the learn-all frame has been injected.
                new_list = FDMA_USER_DCB(jr_reinject_lrn_all(vstate, xtr_ch, ch_state->pend_head));
#endif /* defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) */
            } else {
#if VTSS_OPT_FDMA_VER >= 3
#if defined(VTSS_ARCH_JAGUAR_1)
                ch_state->pend_head->ifh_ptr                += 4; /* Skip across extraction queue number */
                FDMA_USER_DCB(ch_state->pend_head)->act_len -= 4;
#endif /* VTSS_ARCH_JAGUAR_1 */
                if (vtss_fdma_cmn_xtr_hdr_decode(vstate, ch_state->pend_head, chip_no, xtr_qu, &sw_tstamp) == VTSS_RC_OK) {
                    new_list = state->fdma_cfg.rx_cb(cntxt, FDMA_USER_DCB(ch_state->pend_head));
                } else {
                    // Re-cycle
                    new_list = FDMA_USER_DCB(ch_state->pend_head);
                }
#else
                FDMA_USER_DCB(ch_state->pend_head)->timestamp = sw_tstamp;
                new_list = ch_state->xtr_cb(cntxt, FDMA_USER_DCB(ch_state->pend_head), xtr_qu + VTSS_PACKET_RX_QUEUE_START);
#endif /* VTSS_OPT_FDMA_VER */
            }

            // NULLify the pending list, since there're no more pending fragments.
            ch_state->pend_head = NULL;

            // Return the DCBs (if any) to the extraction channel. Don't restart the channel (that's handled in xtr_irq_tfr_done()).
            (void)vtss_fdma_cmn_dcb_release(vstate, xtr_ch, FDMA_SW_DCB(new_list), TRUE /* thread safe */, TRUE /* connect */, FALSE /* don't restart channel */);
        } else {
            // The EOF is not reached yet, so there wasn't room for the whole frame
            // in the allocated data area of this DCB. Move the DCB to the pending list.

            // Save a reference to the new head.
            new_head = FDMA_USER_DCB(ch_state->cur_head)->next;

            // Clear the next entry, so that we can accommodate the case where the
            // FDMA's DCB list exhausts in the middle of a jumbo frame, causing
            // xtr_irq_tfr_done() to restart the channel with a new list of DCBs.
            // If we didn't always clear the pend_head->next we wouldn't know where
            // to attach the remainder of jumbo frames.
            FDMA_USER_DCB(ch_state->cur_head)->next = NULL;

            if (!ch_state->pend_head) {
                // This is the first part of the frame. The SOF flag must be set. Continue execution if it isn't.
                FDMA_ASSERT(IRQ, sof, VTSS_EG_HEX(VTSS_TRACE_GROUP_FDMA_IRQ, (u8 *)ch_state, sizeof(*ch_state)); VTSS_EG_HEX(VTSS_TRACE_GROUP_FDMA_IRQ, (u8 *)hw_dcb, sizeof(*hw_dcb)));
                ch_state->pend_head = ch_state->cur_head;
            } else {
                // This is not the first and not the last part of the frame. The SOF flag must be cleared. Continue execution if it isn't.
                FDMA_ASSERT(IRQ, !sof, VTSS_EG_HEX(VTSS_TRACE_GROUP_FDMA_IRQ, (u8 *)ch_state, sizeof(*ch_state)); VTSS_EG_HEX(VTSS_TRACE_GROUP_FDMA_IRQ, (u8 *)hw_dcb, sizeof(*hw_dcb)));

                // We need to attach it to the last item in the pend_head list.
                temp_list = FDMA_USER_DCB(ch_state->pend_head);
                while (temp_list->next) {
                    temp_list = temp_list->next;
                }
                temp_list->next = FDMA_USER_DCB(ch_state->cur_head);
            }
        }

        // Advance the head of the "in progress" list
        ch_state->cur_head = FDMA_SW_DCB(new_head);
    }

    // Return non-zero if the active head is now NULL.
    return ch_state->cur_head == NULL;
}

/*****************************************************************************/
// xtr_irq_tfr_done()
// We need to restart the channel.
/*****************************************************************************/
static FDMA_INLINE void xtr_irq_tfr_done(vtss_state_t *vstate, vtss_fdma_ch_t xtr_ch)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[xtr_ch];

    FDMA_ASSERT(IRQ, ch_state->status != FDMA_CH_STATUS_DISABLED, return;);
    FDMA_ASSERT(IRQ, ch_state->usage  == VTSS_FDMA_CH_USAGE_XTR,  return;);
    VTSS_NG(VTSS_TRACE_GROUP_FDMA_IRQ, "xtr_irq_tfr_done(ch = %d). New DCBs available: %s\n", xtr_ch, ch_state->free_head?"yes":"no");

    // The following function performs all the necessary checks for the restart to take effect.
    vtss_fdma_cmn_xtr_restart_ch(vstate, xtr_ch);
}

#if defined(VTSS_ARCH_JAGUAR_1)
/*****************************************************************************/
// jr_inj_masq_err_check()
// A bug in Jaguar might cause certain registers (ANA_CL:PORT[]) to be in-
// accessible after a frame with a certain payload pattern is injected.
// This patch checks - after an injection - whether that situation has
// occurred, and attempts to remedy it by injecting a frame long enough to
// unlock the ASM's claim of the ANA_CL:PORT[] RAM and then abort the frame.
// The bug is reported as Bugzilla#9592 and Bugzilla#9596.
/*****************************************************************************/
static FDMA_INLINE void jr_inj_masq_err_check(vtss_state_t *vstate)
{
    u32 old_err_cnt, new_err_cnt;

    // Before reading registers to determine if the problem is there, we need
    // to disable register access checking, or it could happen that we print
    // error messages to the console.
    (void)vtss_cmn_debug_reg_check_set(vstate, FALSE);

    // Avoid Lint Warning 572: Excessive shift value (precision 1 shifted right by 2),
    // which is due to use of the first target in the range (VTSS_DEVCPU_ORG).
    /*lint --e{572} */
    old_err_cnt = VCOREIII_RD(vstate, VTSS_DEVCPU_ORG_ORG_ERR_CNTS);

    (void)VCOREIII_RD(vstate, VTSS_ANA_CL_2_PORT_FILTER_CTRL(0));
    if ((new_err_cnt = VCOREIII_RD(vstate, VTSS_DEVCPU_ORG_ORG_ERR_CNTS)) == old_err_cnt) {
        // ANA_CL:PORT[] is accessible. Nothing more to do.
        goto do_exit;
    }

    VCOREIII_WR(vstate, VTSS_DEVCPU_QS_INJ_INJ_CTRL(0), VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_SOF);
    VCOREIII_WR(vstate, VTSS_DEVCPU_QS_INJ_INJ_WR(0), 0x00000000);
    VCOREIII_WR(vstate, VTSS_DEVCPU_QS_INJ_INJ_WR(0), 0x00000000);
    VCOREIII_WR(vstate, VTSS_DEVCPU_QS_INJ_INJ_WR(0), 0x00000080);
    VCOREIII_WR(vstate, VTSS_DEVCPU_QS_INJ_INJ_WR(0), 0x00000000);
    VCOREIII_WR(vstate, VTSS_DEVCPU_QS_INJ_INJ_WR(0), 0x00000000);
    VCOREIII_WR(vstate, VTSS_DEVCPU_QS_INJ_INJ_WR(0), 0x00000300);
    VCOREIII_WR(vstate, VTSS_DEVCPU_QS_INJ_INJ_CTRL(0), VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_EOF | VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_ABORT);
    VCOREIII_WR(vstate, VTSS_DEVCPU_QS_INJ_INJ_WR(0), 0x00000000);
    while (VCOREIII_RD(vstate, VTSS_DEVCPU_QS_INJ_INJ_CTRL(0)) & VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_ABORT);

    (void)VCOREIII_RD(vstate, VTSS_ANA_CL_2_PORT_FILTER_CTRL(0));

    if (VCOREIII_RD(vstate, VTSS_DEVCPU_ORG_ORG_ERR_CNTS) == new_err_cnt) {
        VTSS_IG(VTSS_TRACE_GROUP_FDMA_IRQ, "Unlocked ANA_CL:PORT[] successfully!");
    } else {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_IRQ, "Failed to unlock ANA_CL:PORT[]");
    }

do_exit:
    // Gotta re-enable register access checking. The enabling/disabling is reference counted
    // so that we don't get to enable checking when it was disabled prior to entering this function.
    (void)vtss_cmn_debug_reg_check_set(vstate, TRUE);
}
#endif /* VTSS_ARCH_JAGUAR_1 */

#if defined(VTSS_ARCH_JAGUAR_1) && VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// jr_multicast_reinject_check()
// On a 48-ported Jaguar, we might need to reinject the same frame multiple
// times, if the frame was requested transmitted on several ports of which
// at least one includes the secondary device.
// Returns TRUE if the frame was reinjected. FALSE otherwise.
/*****************************************************************************/
static FDMA_INLINE BOOL jr_multicast_reinject_check(vtss_state_t *vstate, fdma_sw_dcb_t *sw_dcb)
{
    vtss_port_no_t      port_no;
    vtss_phys_port_no_t chip_port;
    fdma_sw_dcb_t       *tail;
    u32                 dcb_cnt;

    if (vstate->chip_count != 2 || sw_dcb->dst_port_mask == 0) {
        // If this is not a 48-ported, or we're not requested to reinject,
        // return this fact.
        return FALSE;
    }

    // We already have generated a signature IFH, which is held in
    // the first 24 bytes at dcb->data, but we need to change the
    // port number to the next in dst_port_mask.

    // First copy the signature IFH to the frame's IFH.
    memcpy(sw_dcb->ifh_ptr, sw_dcb->data, FDMA_USER_DCB(sw_dcb)->frm_ptr - sw_dcb->ifh_ptr);

    // Then find the next port number.
    port_no = VTSS_OS_CTZ64(sw_dcb->dst_port_mask);

    FDMA_ASSERT(IRQ, port_no < VTSS_ARRSZ(vstate->port.map) && vstate->port.map[port_no].chip_no == 1, return FALSE;);

    chip_port = vstate->port.map[port_no].chip_port;

    // Clear the port we are going to transmit on in the "remaining ports" mask
    sw_dcb->dst_port_mask &= ~VTSS_BIT64(port_no);

    // Modify the new IFH. A bit dirty to do it here, but what can we do when
    // we don't have all the tx_info properties anymore?!?
    // It's located in the VStax header's 6th byte's 5 lsbits. Clear them and
    // mask the new port onto it.
    sw_dcb->ifh_ptr[13] &= 0xe0;
    sw_dcb->ifh_ptr[13] |= chip_port;

    if (inj_dcb_init(vstate, sw_dcb, 0 /* inj_grp */, &tail, &dcb_cnt, sw_dcb->dcb_type, TRUE, FALSE, TRUE)               != VTSS_RC_OK ||
        vtss_fdma_cmn_inj_attach_frm(vstate, sw_dcb, tail, VTSS_FDMA_CH_INJ, dcb_cnt, 0 /* for statistics */, NULL, TRUE) != VTSS_RC_OK) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_IRQ, "Multicast reinject failed");
        return FALSE;
    }

    return TRUE;
}
#endif /* defined(VTSS_ARCH_JAGUAR_1) && VTSS_OPT_FDMA_VER >= 3 */

/*****************************************************************************/
// inj_irq_tfr_done()
/*****************************************************************************/
static FDMA_INLINE void inj_irq_tfr_done(vtss_state_t *vstate, void *const cntxt, vtss_fdma_ch_t inj_ch)
{
    fdma_state_t     *state = &vstate->fdma_state;
    fdma_ch_state_t  *ch_state = &state->fdma_ch_state[inj_ch];
    vtss_fdma_list_t *sof_l, *eof_l;
    fdma_hw_dcb_t    *hw_dcb;
#if VTSS_OPT_FDMA_VER >= 3
    BOOL             reinject = FALSE;
#endif /* VTSS_OPT_FDMA_VER >= 3 */

    VTSS_NG(VTSS_TRACE_GROUP_FDMA_IRQ, "inj_irq_tfr_done(ch = %d)\n", inj_ch);

    FDMA_ASSERT(IRQ, inj_ch >= 0 && inj_ch < VTSS_FDMA_CH_CNT,    return;);
    FDMA_ASSERT(IRQ, ch_state->status != FDMA_CH_STATUS_DISABLED, return;);
    FDMA_ASSERT(IRQ, ch_state->usage  == VTSS_FDMA_CH_USAGE_INJ,  return;);
    FDMA_ASSERT(IRQ, ch_state->cur_head,                          return;);
    sof_l = eof_l = FDMA_USER_DCB(ch_state->cur_head);

    // We need to traverse the DCBs until we find a DCB with the hw_dcb->llp
    // location being NULL, which is the latest injected frame's EOF. We get called
    // once per frame that has been injected, so it cannot happen that the DMA
    // controller goes beyond that point in the list.
    while (1) {
        FDMA_ASSERT(IRQ, eof_l, return;);

        // Check that the DMA controller indeed has injected the current block.
        // This is conveyed through the hw_dcb->ctl1.DONE bit.
        // Since the DMA controller writes this bit, we must invalidate the cache
        // for that position before reading it. It is the only part of the DCB
        // that the DMA controller writes back, so it's the only part that we need
        // to invalidate. Also, since we only use it in an assert, we need only
        // invalidate it in VTSS_OPT_FDMA_DEBUG mode.
        hw_dcb = FDMA_HW_DCB(FDMA_SW_DCB(eof_l));

#if VTSS_OPT_FDMA_DEBUG
        VTSS_OS_DCACHE_INVALIDATE(&hw_dcb->ctl1, 4);
        VTSS_OS_REORDER_BARRIER();
        FDMA_ASSERT(IRQ, BUS_TO_CPU(hw_dcb->ctl1) & VTSS_F_FDMA_CH_CTL1_DONE, return;);
#endif /* VTSS_OPT_FDMA_DEBUG */

        if (hw_dcb->llp) {
            eof_l = eof_l->next;
        } else {
            break;
        }
    }

#if VTSS_OPT_FDMA_VER <= 2
    VTSS_OS_TIMESTAMP(&sof_l->timestamp);
#else
    VTSS_OS_TIMESTAMP(&sof_l->sw_tstamp);
#endif /* VTSS_OPT_FDMA_VER */

    // eof_l points to the EOF list item.
    // Terminate it before passing it on to the callback function,
    // but keep a pointer to the next in the list, which is the one
    // we need to reactivate the channel for if non-NULL.
    ch_state->cur_head = FDMA_SW_DCB(eof_l->next);
    eof_l->next = NULL;

    if (ch_state->cur_head) {
        // More frames to go. Re-start the channel.
        VTSS_DG(VTSS_TRACE_GROUP_FDMA_IRQ, "inj_irq_tfr_done(ch = %d). Re-starting channel\n", inj_ch);

        inj_ch_init(vstate, FDMA_HW_DCB(ch_state->cur_head), inj_ch);
        enable_ch(vstate, inj_ch);
    } else {
        // No more frames. Also reset the tail.
        ch_state->cur_tail = NULL;

#if defined(VTSS_ARCH_JAGUAR_1)
      jr_inj_masq_err_check(vstate);
#endif /* VTSS_ARCH_JAGUAR_1 */
    }

    state->fdma_stats.packets[inj_ch]++;

#if VTSS_OPT_FDMA_VER <= 2
    // Callback the Tx handler unless this is a simple retransmit
    FDMA_ASSERT(IRQ, sof_l->inj_post_cb, return;);

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
    if (sof_l->inj_post_cb == jr_reinject_done) {
        // Dirty hack to get the context right.
        sof_l->inj_post_cb(vstate, sof_l, inj_ch, FALSE);
    } else
#endif /* defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) */
    {
        sof_l->inj_post_cb(cntxt, sof_l, inj_ch, FALSE); // Last argument is @dropped, which cannot happen on this architecture.
    }
#else

    // VTSS_OPT_FDMA_VER >= 3
#if defined (VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
    if (FDMA_SW_DCB(sof_l)->dcb_type == VTSS_FDMA_DCB_TYPE_XTR) {
        // This DCB is a result of re-injection of a learn-all frame.
        // Just return it to the list of extraction DCBs (taken care of
        // by vtss_fdma_cmn_dcb_release()), and do not call anyone back.
    } else
#endif /* defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) */
    {

#if defined(VTSS_ARCH_JAGUAR_1)
        reinject = jr_multicast_reinject_check(vstate, FDMA_SW_DCB(sof_l));
#endif
        if (!reinject) {
            vstate->fdma_state.fdma_cfg.tx_done_cb(cntxt, sof_l, VTSS_RC_OK);
        }
    }

    if (!reinject) {
        // Release the injection DCBs (or extraction in case of learn-all reinject) to the relevant list.
        (void)vtss_fdma_cmn_dcb_release(vstate, 0 /* doesn't matter in FDMA v3+ */, FDMA_SW_DCB(sof_l), TRUE /* thread safe */, TRUE /* do connect in case of extraction */, TRUE /* do restart in case of extraction */);
    }
#endif /* VTSS_OPT_FDMA_VER */
}

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
/*****************************************************************************/
// xtr_irq_secondary()
/*****************************************************************************/
static FDMA_INLINE void xtr_irq_secondary(vtss_state_t *vstate, void *const cntxt, vtss_fdma_ch_t xtr_ch)
{
    fdma_state_t           *state    = &vstate->fdma_state;
    fdma_ch_state_t        *ch_state = &state->fdma_ch_state[xtr_ch];
    vtss_fdma_list_t       *new_list;
    fdma_sw_dcb_t          *sw_dcb;
    u32                    frm_len, xtr_qu;
    BOOL                   drop_it, reinject_it;
    VTSS_OS_TIMESTAMP_TYPE sw_tstamp;
    vtss_rc                rc;

    FDMA_ASSERT(IRQ, state->rx_frame_get_internal     != NULL,    return;);
    FDMA_ASSERT(IRQ, state->rx_frame_discard_internal != NULL,    return;);
    FDMA_ASSERT(IRQ, ch_state->status != FDMA_CH_STATUS_DISABLED, return;);
    FDMA_ASSERT(IRQ, ch_state->status == VTSS_FDMA_CH_USAGE_XTR,  return;);

    if (ch_state->cur_head == NULL) {
        // We're out of DCBs. Discard the frame
        // Figure out which extraction queue the frame is likely to get discarded on.
        u32 xtr_qu_mask, discarded_bytes;
        (void)vstate->init_conf.reg_read(1, VTSS_DEVCPU_QS_XTR_XTR_DATA_PRESENT, &xtr_qu_mask);
        xtr_qu_mask = VTSS_X_DEVCPU_QS_XTR_XTR_DATA_PRESENT_DATA_PRESENT(xtr_qu_mask);
        (void)state->rx_frame_discard_internal(vstate, 1, ch_state->xtr_grp, &discarded_bytes);
        vtss_fdma_cmn_throttle_suspend_check(vstate, xtr_qu_from_mask(xtr_qu_mask), discarded_bytes);
        // Discard only one frame at a time to allow other processes to run.
        return;
    }

    while (1) {
        if (ch_state->cur_head == NULL) {
            // Out of DCBs. Simply return and get invoked again later.
            return;
        }

        FDMA_ASSERT(IRQ, ch_state->pend_head == NULL, return;);

        sw_dcb = ch_state->cur_head;
        FDMA_USER_DCB(sw_dcb)->frm_ptr = sw_dcb->data + VTSS_FDMA_HDR_SIZE_BYTES;

        rc = state->rx_frame_get_internal(vstate, 1, ch_state->xtr_grp, sw_dcb->ifh_ptr, FDMA_USER_DCB(sw_dcb)->frm_ptr, sw_dcb->alloc_len - VTSS_FDMA_HDR_SIZE_BYTES, &frm_len);
        if (rc == VTSS_RC_ERROR) {
            // No more frames.
            return;
        } else if (rc == VTSS_RC_INCOMPLETE) {
            // Jumbo frame. Update throttling.
            xtr_qu = jr_xtr_qu(sw_dcb->ifh_ptr);
            vtss_fdma_cmn_throttle_suspend_check(vstate, xtr_qu, frm_len);
            return;
        }

        VTSS_OS_TIMESTAMP(&sw_tstamp);

        state->fdma_stats.dcbs_used[xtr_ch]++;
        state->fdma_stats.packets[xtr_ch]++;
        // Don't include IFH in packet byte counter
        state->fdma_stats.bytes[xtr_ch] += frm_len;

        FDMA_USER_DCB(sw_dcb)->act_len = frm_len + (FDMA_USER_DCB(sw_dcb)->frm_ptr - sw_dcb->ifh_ptr); // frm_len is the full frame excluding IFH but including FCS, and act_len is the full frame including IFH and FCS.

        xtr_qu = jr_xtr_qu(sw_dcb->ifh_ptr);
        jr_drop_reinject_check(vstate, sw_dcb, xtr_qu, 1, &drop_it, &reinject_it);

        vtss_fdma_cmn_throttle_suspend_check(vstate, xtr_qu, frm_len);

        // We pass the chip number in the MSbyte of the xtr_qu, i.e. byte #0 of the IFH.
        sw_dcb->ifh_ptr[0] = 1;

        stat_rx_update(state, 1, xtr_qu, drop_it);

        if (drop_it) {
            // We drop the frame. Nothing else to do, since the DCB is not taken out of the current list.
            state->fdma_stats.dcbs_added[xtr_ch]++;
        } else {
            ch_state->cur_head = FDMA_USER_DCB(ch_state->cur_head)->next ? FDMA_SW_DCB(FDMA_USER_DCB(ch_state->cur_head)->next) : NULL;
            FDMA_USER_DCB(sw_dcb)->next = NULL;
            if (reinject_it) {
                // Returns non-NULL on error.
                fdma_sw_dcb_t *dcb = jr_reinject_lrn_all(vstate, xtr_ch, sw_dcb);
                if (dcb) {
                    new_list = FDMA_USER_DCB(dcb);
                } else {
                    new_list = NULL;
                }
            } else {
#if VTSS_OPT_FDMA_VER >= 3
#if defined(VTSS_ARCH_JAGUAR_1)
                sw_dcb->ifh_ptr += 4; /* Skip across extraction queue number */
                FDMA_USER_DCB(sw_dcb)->act_len -= 4;
#endif /* VTSS_ARCH_JAGUAR_1 */
                if (vtss_fdma_cmn_xtr_hdr_decode(vstate, sw_dcb, 1, xtr_qu, &sw_tstamp) == VTSS_RC_OK) {
                    new_list = state->fdma_cfg.rx_cb(cntxt, FDMA_USER_DCB(sw_dcb));
                } else {
                    // Re-cycle
                    new_list = FDMA_USER_DCB(sw_dcb);
                }
#else
                FDMA_USER_DCB(sw_dcb)->timestamp = sw_tstamp;
                new_list = ch_state->xtr_cb(cntxt, FDMA_USER_DCB(sw_dcb), xtr_qu + VTSS_PACKET_RX_QUEUE_START);
#endif /* VTSS_OPT_FDMA_VER */
            }

            (void)vtss_fdma_cmn_dcb_release(vstate, xtr_ch, FDMA_SW_DCB(new_list), TRUE /* thread safe */, TRUE /* connect */, TRUE /* restart channel */);
        }
    }
}
#endif /* defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_ch_init()
/*****************************************************************************/
static void afi_ch_init(vtss_state_t *const vstate, const vtss_fdma_ch_t afi_ch)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[afi_ch];
    FDMA_ASSERT(IRQ, ch_enabled(vstate, afi_ch) == FALSE,;);

    if (ch_state->afi_state.cur.timer_reload_value == 0) {
        // Disable the channel for frame spacing and disable the timer.
        VCOREIII_WR(vstate, VTSS_ICPU_CFG_INJ_FRM_SPC_INJ_FRM_SPC_CFG(afi_ch), 0);
        disable_ch(vstate, afi_ch);
    } else {
        // Enable the channel for frame spacing along with starting the frame spacing timer
        // First stop the timer.
        VCOREIII_WR(vstate, VTSS_ICPU_CFG_INJ_FRM_SPC_INJ_FRM_SPC_CFG(afi_ch), VTSS_F_ICPU_CFG_INJ_FRM_SPC_INJ_FRM_SPC_CFG_FRM_SPC_ENA);
        VCOREIII_WR(vstate, VTSS_ICPU_CFG_INJ_FRM_SPC_INJ_FRM_SPC_TMR_CFG(afi_ch), ch_state->afi_state.cur.timer_reload_value);
        VCOREIII_WR(vstate, VTSS_ICPU_CFG_INJ_FRM_SPC_INJ_FRM_SPC_LACK_CNTR(afi_ch), 0);

        // Clear pending interrupts
        clear_pending_block_done_intr(vstate, VTSS_BIT(afi_ch));
        inj_ch_init(vstate, FDMA_HW_DCB(ch_state->afi_state.cur.head), afi_ch);
        VCOREIII_WR(vstate, VTSS_ICPU_CFG_INJ_FRM_SPC_INJ_FRM_SPC_CFG(afi_ch), VTSS_F_ICPU_CFG_INJ_FRM_SPC_INJ_FRM_SPC_CFG_FRM_SPC_ENA | VTSS_F_ICPU_CFG_INJ_FRM_SPC_INJ_FRM_SPC_CFG_TMR_ENA);
        enable_ch(vstate, afi_ch);
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_seq_number_update()
// May only be called if sequence number updates are enabled.
/*****************************************************************************/
static void afi_seq_number_update(afi_dcb_list_t *afi_dcb_list, u32 circle_segment)
{
    vtss_fdma_list_t *afi_dcb; // The one committed to H/W
    u32              seq_number = 0;

    FDMA_ASSERT(IRQ, afi_dcb_list->head && afi_dcb_list->tail[0] && afi_dcb_list->tail[1], return;);
    afi_dcb = circle_segment == 0 ? FDMA_USER_DCB(afi_dcb_list->head) : FDMA_USER_DCB(afi_dcb_list->tail[0])->next;
    // Update this circle segment's half.

    VTSS_IG(VTSS_TRACE_GROUP_FDMA_IRQ, "AFI(seq_upd begin. Segment = %u)", circle_segment);

    while (afi_dcb != FDMA_USER_DCB(afi_dcb_list->tail[circle_segment])->next) {
        if (AFI_SEQ_NUM_ENABLED(FDMA_SW_DCB(afi_dcb))) {
            vtss_fdma_list_t *signature_user_dcb; // The one coming from the application
            u16              offset;
            u8               *frm_ptr;

            // Get the user frame. A pointer to its DCB is held in an unused field in the H/W DCB.
            signature_user_dcb = (vtss_fdma_list_t *)FDMA_HW_DCB(FDMA_SW_DCB(afi_dcb))->stat;
            // The next sequence number to used is held in the user-frame (signature_user_dcb).
            seq_number = AFI_SEQ_NUMBER(signature_user_dcb)++;
            // The offset to update is also held in the user-frame.
            // The - by H/W - unused H/W DCB's stat field is used for this purpose
            // in the signature frame.
            offset = (u16)FDMA_HW_DCB(FDMA_SW_DCB(signature_user_dcb))->stat;
            // Save it in network order.
            frm_ptr = &afi_dcb->frm_ptr[offset];
            *(frm_ptr++) = seq_number >> 24;
            *(frm_ptr++) = seq_number >> 16;
            *(frm_ptr++) = seq_number >>  8;
            *(frm_ptr++) = seq_number >>  0;
            // Flush it to main memory
            VTSS_OS_DCACHE_FLUSH(frm_ptr - 4, 4);
        }
        afi_dcb = afi_dcb->next;
    }
    VTSS_IG(VTSS_TRACE_GROUP_FDMA_IRQ, "AFI(seq_upd done (next seq=%u)", seq_number + 1);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_seq_number_rollback()
/*****************************************************************************/
static FDMA_INLINE void afi_seq_number_rollback(afi_dcb_list_t *afi_dcb_list)
{
    vtss_fdma_list_t *afi_dcb; // The one committed to H/W

    FDMA_ASSERT(IRQ, afi_dcb_list->head && afi_dcb_list->tail[0] && afi_dcb_list->tail[1], return;);

    // It doesn't matter which of the two halves we choose to use for rolling back the
    // sequence numbers, because they are identical and affect the same user frame DCBs.
    afi_dcb = FDMA_USER_DCB(afi_dcb_list->head);

    while (afi_dcb != FDMA_USER_DCB(afi_dcb_list->tail[0])->next) {
        if (AFI_SEQ_NUM_ENABLED(FDMA_SW_DCB(afi_dcb))) {
          // Get the user frame. A pointer to its DCB is held in an unused field in the H/W DCB.
          vtss_fdma_list_t *signature_user_dcb = (vtss_fdma_list_t *)FDMA_HW_DCB(FDMA_SW_DCB(afi_dcb))->stat;

          // The next sequence number to used is held in the user-frame (signature_dcb).
          AFI_SEQ_NUMBER(signature_user_dcb)--;
        }
        afi_dcb = afi_dcb->next;
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_irq_update_seq_num_and_counters()
/*****************************************************************************/
static FDMA_INLINE void afi_irq_update_seq_num_and_counters(fdma_state_t *state, vtss_fdma_ch_t afi_ch, BOOL afi_ch_enabled)
{
    fdma_ch_state_t  *ch_state = &state->fdma_ch_state[afi_ch];
    vtss_fdma_list_t *user_dcb;
    int              i, circle_segment;

    if (ch_state->afi_state.cur.enable_counting || ch_state->afi_state.cur.enable_sequence_numbering) {
        // Counting and/or sequence numbering is enabled on this channel.
        for (circle_segment = 0; circle_segment < 2; circle_segment++) {
            if (ch_state->afi_state.cur.tail[circle_segment] != NULL) {
                // If tail[circle_segment] is non-NULL, that segment of the circle does generate interrupts.
                fdma_hw_dcb_t *hw_dcb = FDMA_HW_DCB(ch_state->afi_state.cur.tail[circle_segment]);
                u32           ctl1;

                // Before dereferencing the DCB, we must invalidate the cache line(s).
                VTSS_OS_DCACHE_INVALIDATE(hw_dcb, VTSS_FDMA_DCB_SIZE_BYTES);
                VTSS_OS_REORDER_BARRIER();

                // Check to see if the GPDMA has handled this DCB.
                ctl1 = BUS_TO_CPU(hw_dcb->ctl1);
                if (!(ctl1 & VTSS_F_FDMA_CH_CTL1_DONE)) {
                    // Nope. Not yet. Nothing more to do for this circle-segment
                    continue;
                }

                VTSS_IG(VTSS_TRACE_GROUP_FDMA_IRQ, "AFI(UpdStart %d, ena = %d)", afi_ch, afi_ch_enabled);

                // Clear the DONE bit.
                ctl1 &= ~VTSS_F_FDMA_CH_CTL1_DONE;
                hw_dcb->ctl1 = CPU_TO_BUS(ctl1);

                VTSS_OS_REORDER_BARRIER();

                // Flush the DCB to main memory.
                VTSS_OS_DCACHE_FLUSH(hw_dcb, VTSS_FDMA_DCB_SIZE_BYTES);

                if (ch_state->afi_state.cur.enable_sequence_numbering && afi_ch_enabled) {
                    // Do not update sequence numbers if channel is now disabled, since
                    // it's a waste of time, and could cause problems with the next
                    // sequence numbers in the user frames.
                    // The (possible) upcoming frame list has based its new sequence numbers
                    // on the fact that we end now.
                    afi_seq_number_update(&ch_state->afi_state.cur, circle_segment);
                }

                // To save CPU cycles, we only do counting when the second half of the circle has been injected.
                // This is safe to do because we know that once a channel is requested to be stopped, it's
                // always after injection of the second half - not the first half.
                if (circle_segment == 1 && ch_state->afi_state.cur.enable_counting) {
                    // Time to update per-frame counters.
                    for (i = 0; i < AFI_FREQ_LIST_LEN; i++) {
                        user_dcb = FDMA_USER_DCB(ch_state->afi_state.user_frames.list[i].frm_list);
                        while (user_dcb) {
                            AFI_FRM_CNT(user_dcb) += FDMA_SW_DCB(user_dcb)->afi_frm_incr;
                            VTSS_IG(VTSS_TRACE_GROUP_FDMA_IRQ, "AFI[1][%d]: %" PRIu64, afi_ch, AFI_FRM_CNT(user_dcb));
                            user_dcb = user_dcb->next;
                        }
                    }

                    user_dcb = FDMA_USER_DCB(ch_state->afi_state.tx_done_head);
                    while (user_dcb) {
                        AFI_FRM_CNT(user_dcb) += FDMA_SW_DCB(user_dcb)->afi_frm_incr;
                        VTSS_IG(VTSS_TRACE_GROUP_FDMA_IRQ, "AFI[2][%d]: %" PRIu64, afi_ch, AFI_FRM_CNT(user_dcb));
                        user_dcb = user_dcb->next;
                    }
                }
                VTSS_IG(VTSS_TRACE_GROUP_FDMA_IRQ, "AFI(UpdEnd %d)", afi_ch);
            }
        }
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_dcb_list_reset()
/*****************************************************************************/
static FDMA_INLINE void afi_dcb_list_reset(afi_dcb_list_t *list)
{
    memset(list, 0, sizeof(*list));
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_dcb_list_free()
/*****************************************************************************/
static void afi_dcb_list_free(fdma_state_t *state, vtss_fdma_ch_t afi_ch, afi_dcb_list_t *list)
{
    state->fdma_stats.dcbs_used[afi_ch] -= list->dcb_cnt;

    // And free the internally managed DCB and frame memory for the old list.
    VTSS_OS_FREE(list->dcb_and_frm_mem, VTSS_MEM_FLAGS_DMA | VTSS_MEM_FLAGS_PERSIST);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_dcb_list_prepare()
/*****************************************************************************/
static void afi_dcb_list_prepare(fdma_ch_state_t *ch_state)
{
    vtss_fdma_list_t *user_dcb;
    int              i, circle_segment;

    // If counting is enabled for this channel, we need to move all
    // afi_frm_incr_next members to the afi_frm_incr member of all
    // user-frames.
    if (ch_state->afi_state.cur.enable_counting) {
        for (i = 0; i < AFI_FREQ_LIST_LEN; i++) {
            user_dcb = FDMA_USER_DCB(ch_state->afi_state.user_frames.list[i].frm_list);
            while (user_dcb) {
                fdma_sw_dcb_t *sw_dcb = FDMA_SW_DCB(user_dcb);
                // If the list of frames has changed, the new number
                // of frames that we send per round-trip is saved in
                // dcb->afi_frm_incr_next.
                sw_dcb->afi_frm_incr = sw_dcb->afi_frm_incr_next;
                user_dcb = user_dcb->next;
            }
        }
    }

    // Comment when called from IRQ:
    //   Before re-starting the channel, we must update the sequence numbers
    //   of the new list of frames. Unfortunately this can not be done
    //   from thread context, because the thread doesn't know exactly when
    //   the channel is actually disabling itself (due to a NULL pointer
    //   in the last DCB). So to avoid race conditions, we must do it here :-(
    //   We must update both halves of the circle.
    // Comment when called from thread (when the channel is not currently in use):
    //   If sequence number updates are enabled for this channel,
    //   we need to do the job that would normally be done in the interrupt handler,
    //   namely to update the sequence numbers of the new list frames.
    if (ch_state->afi_state.cur.enable_sequence_numbering) {
        for (circle_segment = 0; circle_segment < 2; circle_segment++) {
            afi_seq_number_update(&ch_state->afi_state.cur, circle_segment);
        }
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_irq_tfr_done()
/*****************************************************************************/
static FDMA_INLINE void afi_irq_tfr_done(vtss_state_t *vstate, void *const cntxt, vtss_fdma_ch_t afi_ch)
{
    fdma_state_t     *state = &vstate->fdma_state;
    fdma_ch_state_t  *ch_state;
    vtss_fdma_list_t *dcb_ptr, *dcb_next;
    afi_dcb_list_t   old;

    VTSS_IG(VTSS_TRACE_GROUP_FDMA_IRQ, "AFI[%d]: Tfr done", afi_ch);

    ch_state = &state->fdma_ch_state[afi_ch];
    FDMA_ASSERT(IRQ, ch_state->status != FDMA_CH_STATUS_DISABLED, return;);
    FDMA_ASSERT(IRQ, ch_state->usage  == VTSS_FDMA_CH_USAGE_CCM,  return;);
    FDMA_ASSERT(IRQ, ch_state->afi_state.cur.head != NULL,        return;);
    FDMA_ASSERT(IRQ, ch_enabled(vstate, afi_ch) == FALSE,         return;);

    // We were interrupted due to a changed frame list, not due to an update of counters.

    // Save a pointer to the old list of frames
    old                     = ch_state->afi_state.cur;
    ch_state->afi_state.cur = ch_state->afi_state.pend;

    // The user-frame's sequence number is not half a circle too far ahead,
    // because the first half segment was updated after the first half of
    // the last roundtrip of frames were sent. So before we insert sequence
    // numbers in the new list of frames, we have to roll the sequence
    // numbers of the old list back half a roundtrip.
    if (old.enable_sequence_numbering) {
        afi_seq_number_rollback(&old);
    }

    afi_dcb_list_prepare(ch_state);

    // Start the channel right away with the new list of frames to avoid too much lack.
    afi_ch_init(vstate, afi_ch);

    // Now that it's started, we can do the rest of the housekeeping.
    afi_dcb_list_reset(&ch_state->afi_state.pend);

    // Free the old list
    afi_dcb_list_free(state, afi_ch, &old);

    // Gotta callback the application with the list of pending tx done frames.
    dcb_ptr = FDMA_USER_DCB(ch_state->afi_state.tx_done_head);
    while (dcb_ptr) {
        // Clear the ->next before calling back to prevent the application from freeing more than one frame at a time.
        dcb_next = dcb_ptr->next;
        dcb_ptr->next = NULL;

#if VTSS_OPT_FDMA_VER <= 2
        dcb_ptr->inj_post_cb(cntxt, dcb_ptr, afi_ch, FALSE);
#else
        state->fdma_cfg.afi_done_cb(cntxt, dcb_ptr, VTSS_RC_OK);

        // Release the AFI DCB to the relevant list.
        (void)vtss_fdma_cmn_dcb_release(vstate, 0 /* doesn't matter in FDMA v3+ */, FDMA_SW_DCB(dcb_ptr), TRUE /* thread safe */, FALSE /* only used for XTR DCBs */, FALSE /* only used for XTR DCBs */);
#endif /* VTSS_OPT_FDMA_VER */
        dcb_ptr = dcb_next;
    }
    ch_state->afi_state.tx_done_head = NULL;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

/*****************************************************************************/
// xtr_start_ch()
/*****************************************************************************/
static void xtr_start_ch(vtss_state_t *const vstate, vtss_fdma_ch_t xtr_ch)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[xtr_ch];
    fdma_hw_dcb_t   *hw_dcb;
    u32             val;

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
    if (ch_state->chip_no == 1) {
        // Nothing else to do since the remaining is for GPDMA/FDMA usage, and in dual-chip solutions,
        // we perform manual extraction - unfortunately.
        // Interrupts and invokation of us are handled by the application.
        return;
    }
#endif /* defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) */

    hw_dcb = FDMA_HW_DCB(ch_state->cur_head);
    VCOREIII_WR(vstate, VTSS_FDMA_CH_LLP(xtr_ch), VTSS_OS_VIRT_TO_PHYS(hw_dcb));

    // By default, the API sets up the DEVCPU Queue System for byte swapping and status word just before
    // the last data to prepare for manual extraction.
    // The FDMA needs the status word *after* the last data, so we must change the register for that.
    // On Jaguar, the API also sets up the DEVCPU Queue System to provide four initial bytes containing the
    // queue number. This is also required by the FDMA code.
    VCOREIII_WR(vstate, VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(ch_state->xtr_grp), VCOREIII_RD(vstate, VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(ch_state->xtr_grp)) | VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_STATUS_WORD_POS);

    VCOREIII_WR(vstate, VTSS_ICPU_CFG_GPDMA_FDMA_CH_CFG(xtr_ch), VTSS_F_ICPU_CFG_GPDMA_FDMA_CH_CFG_CH_ENA); // USAGE = extraction
    VCOREIII_WR(vstate, VTSS_ICPU_CFG_GPDMA_FDMA_XTR_CFG(ch_state->xtr_grp), VTSS_F_ICPU_CFG_GPDMA_FDMA_XTR_CFG_XTR_BURST_SIZE(XTR_SRC_MSIZE)); // Burst is the same as set-up in extraction DCBs. Can be tweaked.
    VCOREIII_WR(vstate, VTSS_FDMA_CH_LLP(xtr_ch),  VTSS_OS_VIRT_TO_PHYS(hw_dcb));
    VCOREIII_WR(vstate, VTSS_FDMA_CH_CTL0(xtr_ch), VTSS_F_FDMA_CH_CTL0_LLP_SRC_EN  | VTSS_F_FDMA_CH_CTL0_LLP_DST_EN); // We just need to enable LLP_SRC_EN and LLP_DST_EN here, since a new descriptor is loaded right after.
    VCOREIII_WR(vstate, VTSS_FDMA_CH_CFG0(xtr_ch), /* VTSS_F_FDMA_CH_CFG0_LOCK_CH | VTSS_F_FDMA_CH_CFG0_LOCK_CH_L(1) | */ VTSS_F_FDMA_CH_CFG0_HS_SEL_DST  | VTSS_F_FDMA_CH_CFG0_CH_PRIOR(ch_state->prio)); // HS_SEL_SRC=Hardware Initiated, HS_SEL_DST=1=Software Initiated, use programmed priority
    val = VTSS_F_FDMA_CH_CFG1_SRC_PER(xtr_ch) | VTSS_F_FDMA_CH_CFG1_DS_UPD_EN | VTSS_BIT(2) | VTSS_F_FDMA_CH_CFG1_FCMODE; // SRC_PER=channel, DS_UPD_EN=1, VTSS_BIT(2) = Reserved = must be 1.
#if XTR_FIFOMODE
    val |= VTSS_F_FDMA_CH_CFG1_FIFOMODE;
#endif /* XTR_FIFO_MODE */
    VCOREIII_WR(vstate, VTSS_FDMA_CH_CFG1(xtr_ch), val);

    // With the DS_UPD_EN bit set in CFG1, we must also set the address from which the destination update status
    // is fetched by the DMA controller. The ICPU_CFG:GPDMA:FDMA_XTR_STAT_LAST_DCB[xtr_grp] provides the needed information
    // (VLD, SOF, EOF, PRUNED, ABORT, FRM_LEN).
    VCOREIII_WR(vstate, VTSS_FDMA_CH_DSTATAR(xtr_ch), FDMA_TO_PHYS(VTSS_ICPU_CFG_GPDMA_FDMA_XTR_STAT_LAST_DCB(ch_state->xtr_grp)));

    // We enable block interrupts for extraction channels, so that we get an interrupt every time a DCB has been filled with
    // data. If we used Transfer Done interrupts, we wouldn't get an interrupt until the list of DCBs was exhausted.
    // We also, on the other hand enable transfer done interrupts. This is needed in order to restart a channel if it happens
    // that the list of DCBs gets exhausted, which may occur if the FDMA transfers packets faster than the operating system
    // can take them off.
    enable_block_done_intr(vstate, xtr_ch);
    enable_tfr_done_intr(vstate, xtr_ch);

    // Clear pending interrupts
    clear_pending_block_done_intr(vstate, VTSS_BIT(xtr_ch));
    clear_pending_tfr_done_intr  (vstate, VTSS_BIT(xtr_ch));

    VTSS_OS_REORDER_BARRIER();

    // Enable the channel
    enable_ch(vstate, xtr_ch);
}

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// xtr_cfg()
// To avoid OS-dependent allocation problems on the various OS's we support,
// we require, that the list[] is pre-allocated and that the data areas that
// its members point to are as well.
// Call this function per channel and queue.
/*****************************************************************************/
static vtss_rc xtr_cfg(vtss_state_t *const vstate, const vtss_fdma_ch_t xtr_ch, const vtss_fdma_ch_cfg_t *const cfg)
{
    int              i;
    fdma_state_t     *state    = &vstate->fdma_state;
    fdma_ch_state_t  *ch_state = &state->fdma_ch_state[xtr_ch];

    // Check parameters and the current status (@xtr_ch already checked)
    // On this chip, the xtr_grp must be the same as the channel number.
    FDMA_ASSERT(NORMAL, cfg->chip_no < 2,                               return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, cfg->chip_no == 1 || cfg->xtr_grp == xtr_ch,    return VTSS_RC_ERROR;); // For chip_no == 0, there must be a one-to-one correspondence between extraction group and channel number.
    FDMA_ASSERT(NORMAL, cfg->xtr_grp < VTSS_PACKET_RX_GRP_CNT,          return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, cfg->prio >= 0 && cfg->prio < VTSS_FDMA_CH_CNT, return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, cfg->list,                                      return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, cfg->xtr_cb,                                    return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, ch_state->status == FDMA_CH_STATUS_DISABLED,    return VTSS_RC_INV_STATE;);

    // Check that no other non-inactive, extraction channel is managing the xtr_grp.
    for (i = 0; i < VTSS_FDMA_CH_CNT; i++) {
        fdma_ch_state_t *ch_state_i = &state->fdma_ch_state[i];
        FDMA_ASSERT(NORMAL, ch_state_i->status   == FDMA_CH_STATUS_DISABLED ||
                            ch_state_i->usage    != VTSS_FDMA_CH_USAGE_XTR  ||
                            ch_state_i->xtr_grp  != cfg->xtr_grp            ||
                            ch_state_i->chip_no  != cfg->chip_no,
                            return VTSS_RC_INV_STATE;);
    }

    ch_state->usage     = VTSS_FDMA_CH_USAGE_XTR;
    ch_state->status    = FDMA_CH_STATUS_ENABLED;
    ch_state->cur_head  = cfg->list;
    ch_state->free_head = NULL; // No un-committed items.
    ch_state->cur_tail  = NULL;
    ch_state->pend_head = NULL;
    ch_state->xtr_grp   = cfg->xtr_grp;
    ch_state->prio      = cfg->prio;
    ch_state->xtr_cb    = cfg->xtr_cb;
    ch_state->chip_no   = cfg->chip_no;

    // Initialize the list's DCB areas.
    VTSS_RC(vtss_fdma_cmn_dcb_release(vstate, xtr_ch, cfg->list, TRUE /* pretend it's thread safe, since we know it's not started yet */, FALSE /* don't connect */, FALSE /* don't start channel */));

    // Start the channel
    xtr_start_ch(vstate, xtr_ch);

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// is_inj_ch()
/*****************************************************************************/
static FDMA_INLINE BOOL is_inj_ch(vtss_fdma_ch_usage_t usage)
{
    return (usage == VTSS_FDMA_CH_USAGE_INJ
#if defined(VTSS_FEATURE_AFI_FDMA)
            || usage == VTSS_FDMA_CH_USAGE_CCM
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
    );
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/*****************************************************************************/
// inj_start_ch()
/*****************************************************************************/
static void inj_start_ch(vtss_state_t *const vstate, const vtss_fdma_ch_t inj_ch)
{
    fdma_ch_state_t      *ch_state = &vstate->fdma_state.fdma_ch_state[inj_ch];
    vtss_packet_tx_grp_t inj_grp;
    u32                  val;
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_FEATURE_AFI_FDMA)
    u32                  inj_grp_cnt = 0;
#endif /* defined(VTSS_ARCH_LUTON26) || defined(VTSS_FEATURE_AFI_FDMA) */

    // For each injection group that this channel serves, configure byte swapping and back pressure
    for (inj_grp = 0; inj_grp < VTSS_PACKET_TX_GRP_CNT; inj_grp++) {
        if (ch_state->inj_grp_mask & (1 << inj_grp)) {
#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_FEATURE_AFI_FDMA)
            inj_grp_cnt++;
#endif /* defined(VTSS_ARCH_LUTON26) || defined(VTSS_FEATURE_AFI_FDMA) */

            if (ch_state->usage == VTSS_FDMA_CH_USAGE_INJ) {
                // Setup back-pressure from the injection group in question to the DMA channel in question.
                VCOREIII_WR(vstate, VTSS_ICPU_CFG_GPDMA_FDMA_INJ_CFG(inj_grp), VTSS_F_ICPU_CFG_GPDMA_FDMA_INJ_CFG_INJ_GRP_BP_ENA | VTSS_F_ICPU_CFG_GPDMA_FDMA_INJ_CFG_INJ_GRP_BP_MAP(inj_ch));
#if defined(VTSS_ARCH_LUTON26)
                ch_state->inj_grp = inj_grp;
#endif /* defined(VTSS_ARCH_LUTON26) */
            } else {
                // For AFI channels, disable back pressure, since it doesn't work with multiple channels mapped to the same injection group.
                VCOREIII_WR(vstate, VTSS_ICPU_CFG_GPDMA_FDMA_INJ_CFG(inj_grp), 0);
                ch_state->inj_grp = inj_grp; // There can only be one.
            }
        }
    }

#if defined(VTSS_ARCH_LUTON26)
    // On Luton26, the number of injection groups that a channel can serve must be exactly 1.
    FDMA_ASSERT(NORMAL, inj_grp_cnt == 1,;); // Don't return
#endif /* defined(VTSS_ARCH_LUTON26) */

#if defined(VTSS_FEATURE_AFI_FDMA)
    // If the channel is configured for AFI usage, the number of injection groups that a channel can serve must be exactly 1.
    if (ch_state->usage == VTSS_FDMA_CH_USAGE_CCM) {
        FDMA_ASSERT(NORMAL, inj_grp_cnt == 1,;); // Don't return
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    // Enable linked list DCB operation.
    // We just need to enable LLP_SRC_EN and LLP_DST_EN here since the first descriptor overwrites the remaining fields anyway.
    VCOREIII_WR(vstate, VTSS_FDMA_CH_CTL0(inj_ch), VTSS_F_FDMA_CH_CTL0_LLP_SRC_EN | VTSS_F_FDMA_CH_CTL0_LLP_DST_EN);

    val = VTSS_F_FDMA_CH_CFG0_CH_PRIOR(ch_state->prio);

#if defined(VTSS_FEATURE_AFI_FDMA)
    if (ch_state->usage == VTSS_FDMA_CH_USAGE_CCM) {
        // Must use channel locking for AFI, since multiple channels may inject at the same time to the same group.
        val |= VTSS_F_FDMA_CH_CFG0_LOCK_CH | VTSS_F_FDMA_CH_CFG0_LOCK_CH_L(1);
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    VCOREIII_WR(vstate, VTSS_FDMA_CH_CFG0(inj_ch), val);

    // VTSS_BIT(2) = Reserved. Must be 1. DST_PER = handshake interface to obey, which is identical to the channel number (used for normal injection).
    // SRC_PER = handshake interface to obey, which is identical to the channel number (used for AFI injection).
    val = VTSS_F_FDMA_CH_CFG1_DST_PER(inj_ch) | VTSS_F_FDMA_CH_CFG1_SRC_PER(inj_ch) | VTSS_BIT(2) | VTSS_F_FDMA_CH_CFG1_FCMODE;
#if INJ_FIFOMODE
    val |= VTSS_F_FDMA_CH_CFG1_FIFOMODE;
#endif /* INJ_FIFOMODE */
    VCOREIII_WR(vstate, VTSS_FDMA_CH_CFG1(inj_ch), val);

    // Setup the usage of this DMA channel to be injection, and enable the channel.
    VCOREIII_WR(vstate, VTSS_ICPU_CFG_GPDMA_FDMA_CH_CFG(inj_ch), VTSS_F_ICPU_CFG_GPDMA_FDMA_CH_CFG_USAGE | VTSS_F_ICPU_CFG_GPDMA_FDMA_CH_CFG_CH_ENA); // USAGE == Injection and CH_ENA

    if (ch_state->usage == VTSS_FDMA_CH_USAGE_INJ) {
        // This function is also called for AFI features.
        // Since the same channel can be used to inject to different ports, and since we need to re-configure
        // the DMA controller if the port changes, we must get a new interrupt for every frame that has been
        // injected. Therefore we make sure that the DCB->llp is NULL for the last DCB of every
        // frame, and that the interrupt enable flag in DCB->ctl1 is 1 for the EOF DCB, only.
        // When the DMA controller has injected the EOF DCB, it will then invoke the transfer done interrupt
        // so that we can notify the callback function that the frame has been transmitted, and possibly
        // restart the controller if there're pending frames.
        enable_tfr_done_intr(vstate, inj_ch);
    } else {
        // AFI channel.
        // We enable block done interrupts permanently. The INTR bit in the DCBs work
        // as a guard against unintended interrupts.
        enable_block_done_intr(vstate, inj_ch);

        // We also enable transfer done interrupts permanently. Only when the NULL-DCB
        // is loaded, will the GPDMA generate such interrupts.
        // This is primarily required because we cannot rely on the block done interrupt
        // being asserted when we get to the interrupt handler, because of a bug
        // in the GPDMA.
        enable_tfr_done_intr(vstate, inj_ch);
    }
}

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// vcoreiii_fdma_start_ch()
/*****************************************************************************/
static void vcoreiii_fdma_start_ch(vtss_state_t *const vstate, const vtss_fdma_ch_t ch)
{
    if (vstate->fdma_state.fdma_ch_state[ch].usage == VTSS_FDMA_CH_USAGE_XTR) {
        xtr_start_ch(vstate, ch);
    } else {
        inj_start_ch(vstate, ch);
    }
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_cfg()
/*****************************************************************************/
static vtss_rc inj_cfg(vtss_state_t *const vstate, const vtss_fdma_ch_t inj_ch, const vtss_fdma_ch_cfg_t *const cfg)
{
    fdma_state_t    *state    = &vstate->fdma_state;
    fdma_ch_state_t *ch_state = &state->fdma_ch_state[inj_ch];

    // Check current status (@inj_ch already checked)
    FDMA_ASSERT(NORMAL, state->fdma_ch_state[inj_ch].status == FDMA_CH_STATUS_DISABLED,              return VTSS_RC_INV_STATE;);
    FDMA_ASSERT(NORMAL, cfg->prio >= 0 && cfg->prio < VTSS_FDMA_CH_CNT,                              return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, cfg->inj_grp_mask != 0 && cfg->inj_grp_mask < (1 << VTSS_PACKET_TX_GRP_CNT), return VTSS_RC_ERROR;);

    ch_state->usage        = cfg->usage;
    ch_state->status       = FDMA_CH_STATUS_ENABLED;
    ch_state->inj_grp_mask = cfg->inj_grp_mask;
    ch_state->prio         = cfg->prio;
    ch_state->cur_head     = NULL;
    ch_state->cur_tail     = NULL;
    ch_state->pend_head    = NULL;

    inj_start_ch(vstate, inj_ch);

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_JAGUAR_1) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_grp_get()
/*****************************************************************************/
static FDMA_INLINE vtss_rc inj_grp_get(vtss_state_t *const vstate, vtss_fdma_ch_t inj_ch, vtss_fdma_inj_props_t *props, vtss_packet_tx_grp_t *inj_grp)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[inj_ch];

    if (ch_state->usage == VTSS_FDMA_CH_USAGE_INJ && props->inj_grp_auto) {
#if defined(VTSS_FEATURE_VSTAX)
        if (props->contains_stack_hdr && props->qos_class == 8) {
            BOOL tx_to_inj_grp_3;
            if (vstate->chip_count == 2) {
                // In 48-port stackable solutions, transmit on injection group 3, which hits the
                // interconnect port, if the frame must go out on the external stack port on the
                // secondary chip (props->chip_no == 1).
                tx_to_inj_grp_3 = props->chip_no == 1;
            } else {
                // In 24-port stackable solutions, transmit on injection group 3, which hits
                // the external stack port B, whenever the port mask matches mask_b.
                tx_to_inj_grp_3 = (props->port_mask & vstate->vstax_info.chip_info[0].mask_b) != 0;
            }

            if (tx_to_inj_grp_3) {
                *inj_grp = 3; // Direct traffic to VStaX-right super-priority queue.
            } else {
                *inj_grp = 2; // Direct traffic to VStaX-left super-priority queue.
            }
        } else if (props->contains_stack_hdr) {
            // Non-super-priority with a stack header. These will be directed to injection group 0.
            *inj_grp = 0;

            // But we have to change the physical port mask in dual-chip solutions, so that
            // it will reflect the interconnect port if the external port on the secondary chip is specified.
            if (vstate->chip_count == 2 && props->chip_no == 1) {
                // Do a hard overwrite.
                props->chip_no = 0;
                props->port_mask = VTSS_BIT64(vstate->port_int_0);
            }
        } else
#endif /* VTSS_FEATURE_VSTAX */
        if (props->qos_class == 8) {
            // Super-priority injection.
            *inj_grp = 4; // Injection group 4 direct traffic to front-port super-priority queue.
        } else {
            *inj_grp = 0; // Injection groups 0 and 1 direct traffic through the analyzer, but it may be bypassed.
        }
#if defined(VTSS_FEATURE_AFI_FDMA)
    } else if (ch_state->usage == VTSS_FDMA_CH_USAGE_CCM && props->inj_grp_auto) {
        // Pick the channel's configured injection group. This must be a one-hot group.
        for (*inj_grp = 0; *inj_grp < VTSS_PACKET_TX_GRP_CNT; (*inj_grp)++) {
            if (ch_state->inj_grp_mask & (1 << *inj_grp)) {
                return VTSS_RC_OK;
            }
        }
        return VTSS_RC_ERROR;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
    } else {
        *inj_grp = props->inj_grp;
    }

    return VTSS_RC_OK;
}
#endif /* defined(VTSS_ARCH_JAGUAR_1) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON26) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_grp_get()
/*****************************************************************************/
static FDMA_INLINE vtss_rc inj_grp_get(vtss_state_t *const vstate, vtss_fdma_ch_t inj_ch, vtss_fdma_inj_props_t *props, vtss_packet_tx_grp_t *inj_grp)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[inj_ch];

    // On Luton26, just pick the first bit of the configured injection groups.
    // Exactly one bit is set in this mask.
    for (*inj_grp = 0; *inj_grp < VTSS_PACKET_TX_GRP_CNT; (*inj_grp)++) {
        if (ch_state->inj_grp_mask & (1 << *inj_grp)) {
            return VTSS_RC_OK;
        }
    }

    return VTSS_RC_ERROR;
}
#endif /* defined(VTSS_ARCH_LUTON26) && VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_init()
/*****************************************************************************/
static vtss_rc inj_init(vtss_state_t *const vstate, vtss_fdma_ch_t inj_ch, fdma_sw_dcb_t *sw_dcb, const u32 len, vtss_fdma_inj_props_t *props, fdma_sw_dcb_t **new_tail, u32 *dcb_cnt)
{
    fdma_ch_state_t      *ch_state = &vstate->fdma_state.fdma_ch_state[inj_ch];
    vtss_packet_tx_grp_t inj_grp;
    u32                  ifh_size = VTSS_FDMA_INJ_HDR_SIZE_BYTES, ifh_offset; // To compensate for variable-length IFHs.
    int                  computed_whl_frm_sz_bytes;

#if defined(VTSS_FEATURE_AFI_FDMA)
    // This function cannot be called with a AFI auto-ch, and it cannot be called on
    // an AFI channel if the user is attempting to cancel an AFI frame.
    FDMA_ASSERT(NORMAL, inj_ch != AFI_CH_AUTO && (ch_state->usage != VTSS_FDMA_CH_USAGE_CCM || props->ccm_cancel == FALSE), return VTSS_RC_ERROR;);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
    FDMA_ASSERT(NORMAL, len >= 18 && len <= VTSS_FDMA_MAX_FRAME_SIZE_BYTES,         return VTSS_RC_ERROR;); // The 18 bytes == 2 * MAC Addr + EtherType + FCS.
    FDMA_ASSERT(NORMAL, inj_grp_get(vstate, inj_ch, props, &inj_grp) == VTSS_RC_OK, return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, ch_state->inj_grp_mask & (1 << inj_grp),                    return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, is_inj_ch(ch_state->usage),                                 return VTSS_RC_INV_STATE;);
    FDMA_ASSERT(NORMAL, ch_state->status != FDMA_CH_STATUS_DISABLED,                return VTSS_RC_INV_STATE;);

    VTSS_NG(VTSS_TRACE_GROUP_FDMA_NORMAL, "vtss_fdma_inj(ch = %d, pmask = 0x%08llx, l = %d).", inj_ch, props->port_mask, len);

#if VTSS_OPT_FDMA_VER <= 2
    // Mirroring (currently it only works on Lu26), and will always be zero on other platforms.
    props->port_mask |= vstate->fdma_state.mirror_port_mask;
#else
    // Mirroring occurs through calls to vtss_packet_tx_hdr_encode().
#endif /* VTSS_OPT_FDMA_VER */

#if defined(VTSS_ARCH_JAGUAR_1)
    // Injection group back into props.
    props->inj_grp = inj_grp;
#endif /* defined(VTSS_ARCH_JAGUAR_1) */

    ifh_offset = inj_ifh_init(sw_dcb->data, props, &ifh_size);
    *new_tail  = inj_dcb_init(inj_grp, sw_dcb, ifh_offset, ifh_size, &computed_whl_frm_sz_bytes, dcb_cnt, ch_state->usage == VTSS_FDMA_CH_USAGE_INJ, ch_state->usage, FALSE);
#if VTSS_OPT_FDMA_DEBUG
    FDMA_ASSERT(NORMAL, computed_whl_frm_sz_bytes == len + ifh_size, return VTSS_RC_ERROR;);
#endif /* VTSS_OPT_FDMA_DEBUG */

    if (*new_tail == 0) {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_create_dummy_dcb()
/*****************************************************************************/
static FDMA_INLINE vtss_rc afi_create_dummy_dcb(vtss_state_t *const vstate, vtss_fdma_ch_t afi_ch)
{
    fdma_ch_state_t  *ch_state = &vstate->fdma_state.fdma_ch_state[afi_ch];
    u32              dummy_dcb_cnt;
    fdma_sw_dcb_t    *dcb_tail, *dummy_dcb = &ch_state->afi_state.dummy_dcb;
    fdma_hw_dcb_t    *hw_dcb;
    u32              dar;
#if VTSS_OPT_FDMA_VER <= 2
    int              dummy_sz_bytes;
#endif /* VTSS_OPT_FDMA_VER <= 2 */

    memset(dummy_dcb, 0, sizeof(*dummy_dcb));

    // Let @data point to any data in memory, but it must point to some virtual memory, so
    // that it can be converted to a legal physical address.
    dummy_dcb->data = dummy_dcb->ifh_ptr = FDMA_USER_DCB(dummy_dcb)->frm_ptr = (u8 *)&dummy_dcb;

    // Anything that will satisfy inj_dcb_init().
    FDMA_USER_DCB(dummy_dcb)->act_len = VTSS_FDMA_HDR_SIZE_BYTES + 64;

    // Only this single DCB.
    FDMA_USER_DCB(dummy_dcb)->next = NULL;

    // Initialize it. The dummy DCB doesn't need to be cache-line-size-aligned, because it's only used as a signature DCB and never passed to the GPDMA.
#if VTSS_OPT_FDMA_VER <= 2
    dcb_tail = inj_dcb_init(ch_state->inj_grp, dummy_dcb, 0, VTSS_FDMA_INJ_HDR_SIZE_BYTES, &dummy_sz_bytes, &dummy_dcb_cnt, FALSE, ch_state->usage, TRUE);
    FDMA_ASSERT(NORMAL, dcb_tail == dummy_dcb, return VTSS_RC_ERROR;);
#else
    dummy_dcb->dcb_type = VTSS_FDMA_DCB_TYPE_AFI;
    VTSS_RC(inj_dcb_init(vstate, dummy_dcb, ch_state->inj_grp, &dcb_tail, &dummy_dcb_cnt, dummy_dcb->dcb_type, FALSE, TRUE, FALSE));
#endif /* VTSS_OPT_FDMA_VER */
    // Time to overload the special fields that in combintaion make this a dummy DCB.
    hw_dcb = FDMA_HW_DCB(dummy_dcb);

    // First one is the chunksize located in hw_dcb->dar.
    // Setting this to 0 causes the FDMA-part to interpret the DCB as a status reading (not documented anywhere).
    dar = BUS_TO_CPU(hw_dcb->dar);    // Back to native form.
    dar &= ~FDMA_DAR_CHUNK_SIZE(~0);  // Clear the bits.
    hw_dcb->dar = CPU_TO_BUS(dar);    // Back to FDMA form.

    // The next one is the transfer size held in ctl1.
    // This is lowered to just one item (of 32 bits) to minimize the burden on the AHB bus.
    hw_dcb->ctl1 = CPU_TO_BUS(1);
    return VTSS_RC_OK;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// afi_cfg()
/*****************************************************************************/
static vtss_rc afi_cfg(vtss_state_t *const vstate, const vtss_fdma_ch_t afi_ch, const vtss_fdma_ch_cfg_t *const cfg)
{
    fdma_state_t    *state    = &vstate->fdma_state;
    fdma_ch_state_t *ch_state = &state->fdma_ch_state[afi_ch];

    FDMA_ASSERT(NORMAL, cfg->list == NULL,                                                                return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, cfg->ccm_quotient_max > 0 && cfg->ccm_quotient_max <= VTSS_FDMA_CCM_QUOTIENT_MAX, return VTSS_RC_ERROR;);

    // AFI only supported on channel-lockable channels (i.e. 0-3).
    FDMA_ASSERT(NORMAL, afi_ch < VTSS_FDMA_AFI_CH_MAX, return VTSS_RC_ERROR;);

    // Put the list of free DCBs in our channel state.
    ch_state->afi_state.tx_done_head = NULL;
    ch_state->afi_state.quotient_max = cfg->ccm_quotient_max;
    afi_dcb_list_reset(&ch_state->afi_state.cur);
    afi_dcb_list_reset(&ch_state->afi_state.pend);
    memset(&ch_state->afi_state.user_frames, 0, sizeof(ch_state->afi_state.user_frames));

    FDMA_USER_DCB(&ch_state->afi_state.dummy_dcb)->next = NULL;

    state->fdma_stats.dcbs_added[afi_ch] = 0; // N/A
    state->fdma_stats.dcbs_used[afi_ch]  = 0; // Counts the number of frames currently allocated by this channel.

    // Create a dummy DCB.
    // The dummy DCB itself is never handed over to the FDMA,
    // but is merely used as a template for the dummy DCBs that are really
    // handed over to the FDMA).
    FDMA_ASSERT(NORMAL, afi_create_dummy_dcb(vstate, afi_ch) == VTSS_RC_OK, return VTSS_RC_ERROR;);

    // Gotta call inj_cfg() to setup everything else related to injection.
    return inj_cfg(vstate, afi_ch, cfg);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_cp_user_frame_lists()
// When #use_afi_next is TRUE, this function is used to update the next
// fields of the frame lists from afi_next. This means that the function is
// called after afi_build() has created a new list of frames. In this case,
// interrupts or the scheduler must be disabled.
// When #use_afi_next is FALSE, this function is used to take a snapshot of
// the currently running frames on this channel and update the afi_next fields
// from the next fields. It is used prior to building a new set of DCBs for the
// GPDMA and interrupts or scheduler need not be disabled, but it's required
// that vtss_fdma_inj() is not re-entrant.
/*****************************************************************************/
static FDMA_INLINE void afi_cp_user_frame_lists(afi_user_frames_t *dst, afi_user_frames_t *src, BOOL use_afi_next)
{
    vtss_fdma_list_t *user_dcb;
    int              i;

    // First copy the entire structure.
    *dst = *src;

    for (i = 0; i < AFI_FREQ_LIST_LEN; i++) {
        // Then update the #next or #afi_next fields of the
        // individual frequency lists.
        user_dcb = FDMA_USER_DCB(dst->list[i].frm_list);
        while (user_dcb) {
            fdma_sw_dcb_t *sw_dcb = FDMA_SW_DCB(user_dcb);
            if (use_afi_next) {
                user_dcb->next = sw_dcb->afi_next;
                // Now that we're allowed to write to afi_frm_incr_next, copy
                // the amount of times this DCB is injected per roundtrip from
                // the temporary field.
                sw_dcb->afi_frm_incr_next = sw_dcb->afi_frm_incr_temp;
            } else {
                // Just taking a snapshot of the currently running list of user frames.
                // No need to update counters.
                sw_dcb->afi_next = user_dcb->next;
            }
            user_dcb = user_dcb->next;
        }
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_frame_set_change()
/*****************************************************************************/
static void afi_frame_set_change(vtss_state_t *const vstate, const vtss_fdma_ch_t afi_ch, afi_dcb_list_t *new, afi_user_frames_t *new_user_frames, fdma_sw_dcb_t *cancelled_user_frame)
{
    fdma_state_t    *state    = &vstate->fdma_state;
    fdma_ch_state_t *ch_state = &state->fdma_ch_state[afi_ch];
    u32             ctl0;
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS flags;
    FDMA_INTERRUPT_DISABLE(flags);

    // Update the currently running list of user frames and link the frames correctly.
    afi_cp_user_frame_lists(&ch_state->afi_state.user_frames, new_user_frames, TRUE);

    // If we've just removed a frame from the list of user frames (user cancelled a frame)
    // we need to attach that into the tx_done_head list, so that it can be signalled
    // back to the application in the IRQ handler that the frame can now be released.
    if (cancelled_user_frame != NULL) {
        FDMA_USER_DCB(cancelled_user_frame)->next = FDMA_USER_DCB(ch_state->afi_state.tx_done_head);
        ch_state->afi_state.tx_done_head = cancelled_user_frame;
    }

    // Three cases to consider:
    // 1) Channel is currently enabled, but in the future, don't inject anymore frames.
    // 2) Channel is currently enabled, but the frame set should be changed to another.
    // 3) Channel is currently disabled, and a new frame set is to be injected.

    // In the first two cases, we need to gracefully stop the channel on a frame boundary.
    // For this purpoose, we change the ch_state properties to the new frame list
    // and set the hw_dcb->llp to NULL of the last currently injected DCB list.

    // At this point in time, we need to check if we've already built a list
    // that is pending to be inserted in place of the currently running.
    // If so, we must free it. Worst case is that we need 3 times the maximum
    // expected number of DCBs: One list for currently running, one list that
    // is currently pending, and one list that we've just built. The currently
    // pending is in that case being freed now, but cannot be freed earlier
    // because we could lose frames on the link in that case.
    if (ch_state->afi_state.pend.head) {
        // A list of frames is already pending. Free it.
        afi_dcb_list_free(state, afi_ch, &ch_state->afi_state.pend);
    }

    // Clear the pending head/tail
    afi_dcb_list_reset(&ch_state->afi_state.pend);

    if (ch_state->afi_state.cur.head) {
        fdma_hw_dcb_t *hw_dcb;

        // Channel is currently enabled with a list of DCBs.
        // To gracefully stop it, emable Transfer Done interrupts
        // on the channel, enable interrupts in the last H/W DCB,
        // and insert a NULL H/W DCB pointer in the last H/W DCB.

        // Update the pending head and the new timer reload value
        // so that the vcoreiii_fdma_irq_handler() can update the channel
        // once it gets invoked (which is likely to happen when we
        // release the interrupt.
        ch_state->afi_state.pend = *new;

        hw_dcb = FDMA_HW_DCB(ch_state->afi_state.cur.tail[1]);

        // Enable interrupts in tail DCB (if not already enabled)
        ctl0  = BUS_TO_CPU(hw_dcb->ctl0) | VTSS_F_FDMA_CH_CTL0_INT_EN;
        hw_dcb->ctl0 = CPU_TO_BUS(ctl0);

        // Set LLP to NULL in tail DCB. This must and will be an atomic 32-bit operation.
        hw_dcb->llp = CPU_TO_BUS(0);

        VTSS_OS_REORDER_BARRIER();

        // Flush the DCB to main memory.
        VTSS_OS_DCACHE_FLUSH(hw_dcb, VTSS_FDMA_DCB_SIZE_BYTES);

        // That's it. When the GPDMA has injected this DCB, it will
        // generate an interrupt (which it might already have done),
        // causing the vtss_fdma_irq_handler() to be invoked.
        // That function will detect that this is an AFI channel and
        // free the current chain of frames, disable tfr done interrupts,
        // and inject the possible new chain of frames.
    } else {
        // Channel is currently not in use. Start it with the new list of frames.

        // Update channel state with new list
        ch_state->afi_state.cur = *new;

        // Update frame increments and sequence numbers for new DCB list.
        afi_dcb_list_prepare(ch_state);

        // Start the channel.
        afi_ch_init(vstate, afi_ch);
    }

    FDMA_INTERRUPT_RESTORE(flags);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_create_lookup_table()
/*****************************************************************************/
static FDMA_INLINE void afi_create_lookup_table(afi_user_frames_t *user_frames, u32 lu[AFI_FREQ_LIST_LEN], int *use_cnt)
{
    u32 f_prev;
    int i, j, idx = 0;

    *use_cnt = 0;

    // Create a look-up array that turns the arbitrarily ordered
    // user_frames[] into an ordered list from higher towards smaller
    // frame frequencies.
    f_prev = 0xFFFFFFFF;
    for (i = 0; i < AFI_FREQ_LIST_LEN; i++) {
        // Find the next, highest non-zero frequency.
        u32 f_highest = 0;
        for (j = 0; j < AFI_FREQ_LIST_LEN; j++) {
            u32 f_cur = user_frames->list[j].fps;
            if (f_cur < f_prev && f_cur > f_highest) {
                f_highest = f_cur;
                idx       = j;
            }
        }
        if (f_highest == 0) {
            break; // Done
        }
        lu[(*use_cnt)++] = idx;
        f_prev        = f_highest;
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_build()
// This function will build a list of DCBs and compute a new timer value for
// the AFI channel given a list of frames with various frequencies.
//
// In the following example, we want to inject:
//   X:  2 frames with a frequency of 300 fps (i.e. 3.33 ms),
//   Y:  1 frame  with a frequency of 100 fps (i.e. 10.0 ms), and
//   Z: 10 frames with a frequency of  10 fps (i.e. 100 ms).
//
// This can be achieved by setting the FDMA AFI to a period of 0.83 ms (3.33 / 4)
// and then setting up the list of DCBs to inject the frames like this:
//   X0 indicates the first of the two frames to be injected with a frequency of 300 fps,
//   X1 the second frame, etc.
//   DU stands for Dummy DCB, i.e. don't transmit anything in this time-slot.
//
//   X0 -> X1 -> Y0 -> Z0 ->
//   X0 -> X1 -> DU -> Z1 ->
//   X0 -> X1 -> DU -> Z2 ->
//   X0 -> X1 -> Y0 -> Z3 ->
//   X0 -> X1 -> DU -> Z4 ->
//   X0 -> X1 -> DU -> Z5 ->
//   X0 -> X1 -> Y0 -> Z6 ->
//   X0 -> X1 -> DU -> Z7 ->
//   X0 -> X1 -> DU -> Z8 ->
//   X0 -> X1 -> Y0 -> Z9 ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> Y0 -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> Y0 -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> Y0 -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> Y0 -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> Y0 -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> Y0 -> DU ->
//   X0 -> X1 -> DU -> DU ->
//   X0 -> X1 -> DU -> DU -> back to the first DCB
//
// In the implementation below, each line constitutes a "block", whose number
// of items defines the AFI timer. In the example above, the number of items
// in a block is 4, which multiplied with the fastest frame frequency currently
// in use (300 Hz) defines the timer frequency (which becomes 1200 Hz or 0.83 ms).
// The number of blocks is given by the frequency of the fastest frames
// divided by the frequency of the slowest frames, which in the above example is
// 300 / 10 = 30.
// The total number of required DCBs is then given by the number of blocks
// multiplied by the number of items in a block. In the example above this will
// become 30 * 4 = 120 DCBs.
//
// When frame counting or sequence number updating is enabled, the list will
// be repeated until it's 50 ms long.
//
// Furthermore, when sequence number updating is enabled, the list will be
// doubled and generate interrupts twice per round-trip. This allows the
// GPDMA to be free running and inject one half while S/W updates sequence
// numbers in the other half.
//
// Note: This function does not make FDMA_ASSERT() calls because the caller
// make trials to see if a frame fits into the channel. If not, it goes
// on to the next channel.
/*****************************************************************************/
static vtss_rc afi_build(vtss_state_t *const vstate, const vtss_fdma_ch_t afi_ch, afi_user_frames_t *new_user_frames, fdma_sw_dcb_t *cancelled_user_frame)
{
    fdma_state_t     *state = &vstate->fdma_state;
    fdma_ch_state_t  *ch_state = &state->fdma_ch_state[afi_ch];
    u32              lu[AFI_FREQ_LIST_LEN];
    int              i, j, idx, use_cnt, idx_fast, idx_slow;
    u32              dcbs_per_block, offset_within_block[AFI_FREQ_LIST_LEN + 1], block_cnt, dcb_cnt;
    u32              blocks_per_frm_repetition[AFI_FREQ_LIST_LEN];
    vtss_fdma_list_t *dma_dcb_next, *user_dcb;
    fdma_sw_dcb_t    *dma_dcb, *sw_dcb;
    afi_dcb_list_t   new;
    fdma_hw_dcb_t    *hw_dcb;
    u64              timer_reload_val_ps, timer_reload_val, min_allowed_frame_spacing_ps;
    u32              list_repetition_cnt, list_repetition;
    u32              circle_segment_cnt, circle_segment;
    u32              frm_mem_req_bytes = 0, dcb_mem_req_bytes, dcb_size_bytes;
    u8               *frm_mem_ptr, *init_frm_mem_ptr;
    fdma_sw_dcb_t    *dcb_ptr;
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS flags;

    afi_dcb_list_reset(&new);

    afi_create_lookup_table(new_user_frames, lu, &use_cnt);

    if (use_cnt == 0) {
        // No frames left. Stop AFI channel gracefully.
        afi_frame_set_change(vstate, afi_ch, &new, new_user_frames, cancelled_user_frame);
        return VTSS_RC_OK;
    }

    idx_fast = lu[0];           // The direct index into the new_user_frames[] with the highest frequency.
    idx_slow = lu[use_cnt - 1]; // The direct index into the new_user_frames[] with the smallest frequency.

    // All entries from [0; use_cnt[ (indirect indices) contain at least one frame and must be handled.

    // Compute the number of blocks we need. See description in header of this function for a
    // definition of the term 'block'.
    block_cnt = new_user_frames->list[idx_fast].fps / new_user_frames->list[idx_slow].fps;

    // Figure out the number of DCBs we need per block.
    // i = indirect index, j = direct index into new_user_frames->list[].
    dcbs_per_block = 0;
    for (i = 0; i < use_cnt; i++) {
        // distinct_frames_per_roundtrip is the maximum number of different frames
        // we can inject with this frequency before the list of DCBs loop back to itself.
        // In the example above, this will give 1 for X, 3 for Y and 30 for Z frames.
        //
        // required_dcbs_per_block is the number of DCBs that this frequency
        // requires in a single block.
        // In the example above, this will give 2 for X, 1 for Y and 1 for Z.
        //
        // offset_within_block[] (indirectly indexed) contains, for a given
        // frequency, the number of its first DCB within a block.
        // In the example above, this will give 0 for X, 2 for Y, and 3 for Z.
        //
        // dcbs_per_block will ultimately hold the number of DCBs in a block (surprise).
        // In the example above, this will become 4.
        //
        // blocks_per_frm_repetition[] (directly indexed) will hold the number of blocks
        // to inject before the same frame gets repeated (this is basically the same as
        // distinct_frames_per_roundtrip).
        // In the example above, this will become 1 for X, 3 for Y and 30 for Z.
        u32 distinct_frames_per_roundtrip, required_dcbs_per_block;
        j = lu[i]; // Get the direct index.
        distinct_frames_per_roundtrip = new_user_frames->list[idx_fast].fps / new_user_frames->list[j].fps;
        required_dcbs_per_block       = (new_user_frames->list[j].frm_cnt + distinct_frames_per_roundtrip - 1) / distinct_frames_per_roundtrip;
        offset_within_block[i]        = dcbs_per_block;                // Indirectly indexed, hence the @i.
        blocks_per_frm_repetition[j]  = distinct_frames_per_roundtrip; // Directly indexed, hence the @j.
        dcbs_per_block               += required_dcbs_per_block;
    }

    // Kind of termination of the offsets:
    offset_within_block[use_cnt] = dcbs_per_block;

    // Compute the total number of DCBs required for this.
    dcb_cnt = block_cnt * dcbs_per_block;

    // The period in picoseconds is now given by 1E12 / (frequency_of_fastest * dcbs_per_block)
    timer_reload_val_ps = VTSS_DIV64(1000000000000ULL, (u64)new_user_frames->list[idx_fast].fps * dcbs_per_block); // Now in picoseconds.
    // Check against maximum allowed FPS. If multiple frames are multiplexed into this channel, it could happen
    // that the frequency exceeds VTSS_AFI_FPS_MAX.
    min_allowed_frame_spacing_ps = VTSS_DIV64(1000000000000ULL, VTSS_AFI_FPS_MAX);
    if (timer_reload_val_ps < min_allowed_frame_spacing_ps) {
        // Just informational. The caller of this function will try another channel.
        VTSS_IG(VTSS_TRACE_GROUP_FDMA_NORMAL, "AFI[%d]: Frame spacing smaller than allowed (computed = %" PRIu64 " ps, min = %" PRIu64 " ps, ", afi_ch, timer_reload_val_ps, min_allowed_frame_spacing_ps);
        return VTSS_RC_ERROR;
    }
    // Convert it to number of AHB clocks
    timer_reload_val = VTSS_DIV64(timer_reload_val_ps, state->ahb_clk_period_ps); // Now in AHB clocks.
    // And subtract one (according to datasheet).
    timer_reload_val--;
    // This number must be a valid 32-bit number
    if (timer_reload_val == 0ULL || timer_reload_val > 0xFFFFFFFFULL) {
        // Just informational. The caller of this function will try another channel.
        VTSS_IG(VTSS_TRACE_GROUP_FDMA_NORMAL, "AFI[%d]: Invalid timer_reload_val (%" PRIu64 ")", afi_ch, timer_reload_val);
        return VTSS_RC_ERROR;
    }
    new.timer_reload_value = timer_reload_val;

    // Before being able to modify any ch_state->afi_state, we need to check that all resources are
    // available, and return with an error if not - before making permanent changes to the running state.

    // Figure out whether frame counting is enabled. If any frame requires frame counting, then all frames on this
    // channel will have frame counting enabled.
    // Likewise, figure out whether sequence numbering is enabled.
    for (i = 0; i < AFI_FREQ_LIST_LEN; i++) {
        user_dcb = FDMA_USER_DCB(new_user_frames->list[i].frm_list);
        while (user_dcb) {
            sw_dcb = FDMA_SW_DCB(user_dcb);
            // It's OK to modify this per-user-frame parameter, since it's not used
            // after exit of this function.
            sw_dcb->afi_frm_incr_temp = 0;
            if (AFI_COUNTING_ENABLED(sw_dcb)) {
                new.enable_counting = TRUE;
            }
            if (AFI_SEQ_NUM_ENABLED(sw_dcb)) {
                new.enable_sequence_numbering = TRUE;
            }
            user_dcb = sw_dcb->afi_next;
        }
    }

    // When frame counting or sequence numbering is enabled, the roundtrip time must be VTSS_FDMA_AFI_MINIMUM_INTERRUPT_PERIOD_MS milliseconds,
    // so that we don't get overwhelmed by interrupts (which in turn might cause us to count wrong).
    // This potentially puts a pretty heavy load on the memory (DCB) consumption. For instance, if this channel
    // is loaded with one frame that must be repeated every 10 microseconds, and VTSS_FDMA_AFI_MINIMUM_INTERRUPT_PERIOD_MS
    // is 50 ms, we require 50,000 / 10 = 5000 DCBs. Ouch!
    if (new.enable_counting || new.enable_sequence_numbering) {
        // Compute the number of picoseconds it takes to complete one trip around a single instance of the list.
        u64 roundtrip_time_single_list_ps = (u64)dcb_cnt * timer_reload_val_ps;
        list_repetition_cnt = VTSS_DIV64(VTSS_FDMA_AFI_MINIMUM_INTERRUPT_PERIOD_MS * 1000000000ULL, roundtrip_time_single_list_ps);
        if (list_repetition_cnt * roundtrip_time_single_list_ps < VTSS_FDMA_AFI_MINIMUM_INTERRUPT_PERIOD_MS * 1000000000ULL) {
            list_repetition_cnt++;
        }
    } else {
        list_repetition_cnt = 1;
    }

    // When sequence numbering is enabled, we need two 50 ms lists (i.e. two half-circles).
    circle_segment_cnt = new.enable_sequence_numbering ? 2 : 1;
    new.dcb_cnt = dcb_cnt * list_repetition_cnt * circle_segment_cnt;

    // Figure out how much memory is required for the DCBs.
    dcb_mem_req_bytes = vtss_fdma_cmn_dcb_mem_size(new.dcb_cnt);
    dcb_size_bytes    = VTSS_FDMA_CMN_CACHE_ALIGNED_SIZE(sizeof(fdma_sw_dcb_t));

    if (new.enable_counting || new.enable_sequence_numbering) {
        // Make a trial-run to figure out the number of times a given user-frame is
        // transmitted per roundtrip. This is also used to compute the total amount of
        // frame data we need in case sequence numbering is enabled.
        for (circle_segment = 0; circle_segment < circle_segment_cnt; circle_segment++) {
            for (list_repetition = 0; list_repetition < list_repetition_cnt; list_repetition++) {
                for (idx = 0; idx < (int)dcb_cnt; idx++) {
                    // block points out the block that this frame should go into.
                    // dcb_within_block is the 0-based DCB number that this frame should go into.
                    u32 block            = (u32)idx / dcbs_per_block;
                    u32 dcb_within_block = (u32)idx % dcbs_per_block;
                    u32 frm_number;

                    // Find the indirect index ([0; use_cnt[) that should populate this entry.
                    for (i = 0; i < use_cnt; i++) {
                        // offset_within_block[] is indirectly indexed.
                        if (dcb_within_block < offset_within_block[i + 1]) {
                            break;
                        }
                    }

                    // @i is now the indirect index into new_user_frames->list[]. Get the direct index (into @j).
                    j = lu[i];
                    frm_number = (block % blocks_per_frm_repetition[j]) + (dcb_within_block - offset_within_block[i]) * blocks_per_frm_repetition[j];
                    if (frm_number < new_user_frames->list[j].frm_cnt) {
                        // We need a frame DCB here.
                        // Find the DCB signature for frame number @frm_number.
                        int k;
                        user_dcb = FDMA_USER_DCB(new_user_frames->list[j].frm_list);
                        for (k = 0; k < (int)frm_number; k++) {
                            user_dcb = FDMA_SW_DCB(user_dcb)->afi_next;
                        }
                        // This user-frame is referenced once more.
                        // This is the major reason for this loop.
                        FDMA_SW_DCB(user_dcb)->afi_frm_incr_temp++;
                    }
                }
            }
        }

        // The amount of memory required can now be found by traversing the list of
        // user-frames and multiply the user-frame-length by the number of times it
        // must be repeated. We could call VTSS_OS_MALLOC() per copied frame, or per user-frame
        // or only once (covering all user frames in one go). Which method we actually
        // use only determines how hard it is to release it all again (either when
        // a frame is being cancelled or we detect an error later in this function).
        // The easiest is to maintain one single pointer to the allocated memory, so that
        // it can be freed in one go. The problem with this is that the heap must be
        // able to deliver several MBytes in one call. The heap might have had an
        // easier job if it should find e.g. 20000 100 byte chunks. The good thing
        // by allocating one chunk is that it's much faster and gives less overhead for
        // heap management (in terms of space).
        // Another advantage of only one pointer (well, three, actually, as we shall see),
        // for the whole channel is that we can save it in the channel's state, so that
        // we don't have to save it along with the user-DCBs, which would require even
        // more internally used fields in a vtss_fdma_list_t structure.
        if (new.enable_sequence_numbering) {
            for (i = 0; i < AFI_FREQ_LIST_LEN; i++) {
                user_dcb = FDMA_USER_DCB(new_user_frames->list[i].frm_list);
                while (user_dcb) {
                    sw_dcb = FDMA_SW_DCB(user_dcb);
                    if (AFI_SEQ_NUM_ENABLED(sw_dcb)) {
                        // Make sure that we can allocate all frames on a 32-bit boundary.
                        // Not strictly needed, but nice.
#if VTSS_OPT_FDMA_VER <= 2
                        // user_dcb->act_len includes IFH + FCS
                        frm_mem_req_bytes += AFI_CEIL4(user_dcb->act_len) * sw_dcb->afi_frm_incr_temp;
#else
                        // user_dcb->act_len encludes IFH + FCS
                        frm_mem_req_bytes += AFI_CEIL4(user_dcb->act_len + state->hdr_size + 4) * sw_dcb->afi_frm_incr_temp;
#endif /* VTSS_OPT_FDMA_VER */
                    }
                    user_dcb = sw_dcb->afi_next;
                }
            }
        }
    }

    // Allocate the required DCB and frame memory.
    new.dcb_and_frm_mem = vtss_fdma_cmn_dcb_and_frm_alloc(new.dcb_cnt, frm_mem_req_bytes, VTSS_MEM_FLAGS_DMA | VTSS_MEM_FLAGS_PERSIST, &new.head, (void **)&init_frm_mem_ptr, NULL, FALSE);
    if (new.dcb_and_frm_mem == NULL) {
        // Error already printed
        return VTSS_RC_ERROR;
    }

    // We use the first part of the newly allocated memory chunk for DCBs and the next for frame memory.
    dcb_ptr = new.head;
    frm_mem_ptr = init_frm_mem_ptr;

    // Chip the DCB memory up in cache-aligned DCBs and link the list.
    for (i = 0; i < (int)new.dcb_cnt - 1; i++) {
        FDMA_USER_DCB(dcb_ptr)->next = FDMA_USER_DCB((fdma_sw_dcb_t *)((u8 *)dcb_ptr + dcb_size_bytes));
#if VTSS_OPT_FDMA_VER >= 3
        dcb_ptr->dcb_type = VTSS_FDMA_DCB_TYPE_AFI;
#endif /* VTSS_OPT_FDMA_VER >= 3 */
        dcb_ptr = FDMA_SW_DCB(FDMA_USER_DCB(dcb_ptr)->next);
    }

    // Connect the last one back to head to form a circular list.
    FDMA_USER_DCB(dcb_ptr)->next = FDMA_USER_DCB(new.head);

    if (frm_mem_req_bytes) {
        // dcb_ptr points to the last DCB. The frame data memory is now one DCB away.
        // It's a coding error if this is not the case, so it's OK to pull an assertion here.
        if ((u8 *)dcb_ptr + dcb_size_bytes > init_frm_mem_ptr) {
            VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "dcb_ptr = %p, dcb_size_bytes = %u, init_frm_mem_ptr = %p, dcb_cnt = %u dcb_and_frm_mem = %p, frm_mem_req_bytes = %u",
                dcb_ptr, dcb_size_bytes, init_frm_mem_ptr, new.dcb_cnt, new.dcb_and_frm_mem, frm_mem_req_bytes);
            goto do_exit_with_error;
        }
    }

    // dcb_ptr points to the last DCB. Check that we haven't written outside the
    // last byte allocated for the DCBs. If so, there's a coding error somewhere, so it's
    // OK to pull an assertion hree.
    FDMA_ASSERT(NORMAL, ((u32)dcb_ptr + dcb_size_bytes - (u32)new.head) <= dcb_mem_req_bytes, goto do_exit_with_error;);

    if (circle_segment_cnt == 2) {
        // Sequence numbering is enabled.
        // Make new.tail[0] point to the last DCB in
        // the first circle segment.
        // We know that there is an even number of DCBs,
        // and we know that the DCBs are laid out in one chunk
        // of memory - evenly spaced.
        new.tail[0] = (fdma_sw_dcb_t *)((u8 *)new.head + dcb_size_bytes * ((new.dcb_cnt / 2) - 1));
    }

    // In the case where sequence numbering is disabled,
    // we leave new.tail[0] == NULL (already initialized).

    // new.tail[1] = must point to the last DCB, i.e. the one that
    // points back to the head (which could be the head itself).
    new.tail[1] = dcb_ptr;

    FDMA_INTERRUPT_DISABLE(flags);
    state->fdma_stats.dcbs_used[afi_ch] += new.dcb_cnt;
    FDMA_INTERRUPT_RESTORE(flags);

    // Now connect the frames to the DCBs in the order that we have
    // prescribed in the code above.
    // i = indirect index, j = direct index into new_user_frames->list[].
    dma_dcb = new.head;
    for (circle_segment = 0; circle_segment < circle_segment_cnt; circle_segment++) {
        for (list_repetition = 0; list_repetition < list_repetition_cnt; list_repetition++) {
            for (idx = 0; idx < (int)dcb_cnt; idx++) {
                // block points out the block that this frame should go into.
                // dcb_within_block is the 0-based DCB number that this frame should go into.
                u32 block            = (u32)idx / dcbs_per_block;
                u32 dcb_within_block = (u32)idx % dcbs_per_block;
                u32 frm_number;
                u32 fps;

                // Find the indirect index ([0; use_cnt[) that should populate this entry.
                for (i = 0; i < use_cnt; i++) {
                    // offset_within_block[] is indirectly indexed.
                    if (dcb_within_block < offset_within_block[i + 1]) {
                        break;
                    }
                }

                // @i is now the indirect index into new_user_frames->list[]. Get the direct index (into @j).
                j = lu[i];
                frm_number = (block % blocks_per_frm_repetition[j]) + (dcb_within_block - offset_within_block[i]) * blocks_per_frm_repetition[j];
                if (frm_number >= new_user_frames->list[j].frm_cnt) {
                    // We need a dummy DCB here.
                    sw_dcb = &ch_state->afi_state.dummy_dcb;
                    fps    = 0; /* Special value to indicate a dummy DCB */
                } else {
                    // We need a frame DCB here.
                    // Find the DCB signature for frame number @frm_number.
                    int k;
                    sw_dcb = new_user_frames->list[j].frm_list;
                    fps     = new_user_frames->list[j].fps;
                    for (k = 0; k < (int)frm_number; k++) {
                        sw_dcb = FDMA_SW_DCB(sw_dcb->afi_next);
                    }

                    // Here would've been a good place to update afi_frm_incr_next
                    // from afi_frm_incr_temp, had it not been because interrupts or
                    // scheduler are enabled. Deferring it to afi_frame_set_change()
                }

                // @sw_dcb now points to the DCB we need. Copy it's contents, but save dcb->next first.
                dma_dcb_next = FDMA_USER_DCB(dma_dcb)->next;
                memcpy(dma_dcb, sw_dcb, sizeof(*dma_dcb));
                FDMA_USER_DCB(dma_dcb)->next = dma_dcb_next;

                // We use one of the unused fields in the DCB for holding the frequency so that
                // we can identify it directly in the vcoreiii_debug_print() function.
                AFI_FPS_SET(dma_dcb, fps);

                // We also need to connect the DCBs H/W-wise.
                hw_dcb = FDMA_HW_DCB(dma_dcb);
                hw_dcb->llp  = CPU_TO_BUS(VTSS_OS_VIRT_TO_PHYS(FDMA_HW_DCB(FDMA_SW_DCB(FDMA_USER_DCB(dma_dcb)->next))));
                // And use the - by the GPDMA - unused stat field as a pointer back to
                // the signature DCB (i.e. the user frame's DCB).
                hw_dcb->stat = (u32)FDMA_USER_DCB(sw_dcb);

                if (AFI_SEQ_NUM_ENABLED(sw_dcb)) {
                    u32 *phys_ifh_ptr, phys_ifh_off, frag_sz_bytes, dar, full_data_size;

                    // Sequence numbering is enabled for this user-frame.
                    // This means that we must copy the user-frame into the
                    // next part of the new.dcb_and_frm_mem allocated far above in this
                    // function. frm_mem_ptr is a free-running pointer within
                    // the new.dcb_and_frm_mem.
                    // It's a coding error if this is not the case, so it's OK to
                    // pull an assertion.
                    FDMA_ASSERT(NORMAL, frm_mem_ptr, return VTSS_RC_ERROR;);
                    dma_dcb->data = frm_mem_ptr;

                    full_data_size = FDMA_USER_DCB(sw_dcb)->act_len;
#if VTSS_OPT_FDMA_VER >= 3
                    // act_len doesn't include IFH and FCS in FDMA API v3+.
                    full_data_size += state->hdr_size + 4 /* FCS */;
#endif /* VTSS_OPT_FDMA_VER >= 3 */

                    // Prepare for next iteration.
                    frm_mem_ptr += AFI_CEIL4(full_data_size);

                    // Get a copy of the user frame.
                    memcpy(dma_dcb->data, sw_dcb->data, full_data_size);

                    // Update the DCB handed to the GPDMA with the new frame properties.
                    dma_dcb->ifh_ptr = dma_dcb->data + (sw_dcb->ifh_ptr - sw_dcb->data);
                    FDMA_USER_DCB(dma_dcb)->frm_ptr = dma_dcb->data + (FDMA_USER_DCB(sw_dcb)->frm_ptr - sw_dcb->data);

                    // To save time, we flush the frame to main memory after we get out of this loop.

                    phys_ifh_ptr = (u32 *)VTSS_OS_VIRT_TO_PHYS(dma_dcb->ifh_ptr);
                    phys_ifh_off = (u32)phys_ifh_ptr & 0x3;
                    hw_dcb->sar = CPU_TO_BUS((u32)phys_ifh_ptr & ~0x3);
                    // Get dar field in native form
                    dar = BUS_TO_CPU(hw_dcb->dar);
                    // Clear the offset field
                    dar &= ~FDMA_DAR_SAR_OFFSET(~0);
                    // And apply the new.
                    dar |= FDMA_DAR_SAR_OFFSET(phys_ifh_off);
                    // And back to the GPDMA format.
                    hw_dcb->dar = CPU_TO_BUS(dar);
                    // Get the frame length.
                    frag_sz_bytes = FDMA_X_DAR_CHUNK_SIZE(dar);
                    // If the data area is non-32-bit-aligned (as is the case for Linux), then we need to check if we're gonna inject one more 32-bit word.
                    hw_dcb->ctl1 = CPU_TO_BUS((frag_sz_bytes + phys_ifh_off + 3) / 4); // BLOCK_TS
                }

                if (idx == dcb_cnt - 1 && list_repetition == list_repetition_cnt - 1 && (new.enable_counting || new.enable_sequence_numbering)) {
                    u32 ctl0;
                    // Enable interrupts in last DCB of this half-circle when we're counting or sequence numbering.
                    ctl0 = BUS_TO_CPU(hw_dcb->ctl0) | VTSS_F_FDMA_CH_CTL0_INT_EN;
                    hw_dcb->ctl0 = CPU_TO_BUS(ctl0);
                }

                // Since the DCBs are also dynamically allocated, we postpone the
                // cache flushing until after this function.

                // And go on with the next.
                dma_dcb = FDMA_SW_DCB(FDMA_USER_DCB(dma_dcb)->next);
            }
        }
    }

    if (init_frm_mem_ptr) {
        // A final check on whether we've written more frame data than we have allocated.
        // If so, it's an internal coding error, that we must fix.
        FDMA_ASSERT(NORMAL, init_frm_mem_ptr + frm_mem_req_bytes == frm_mem_ptr, goto do_exit_with_error;);
    }

    // Flush the allocated DCBs and frame data to RAM in one go instead of flushing it per frame.
    // RBNTBD: Optimize by flushing whole cache in one go if more than cache size bytes
    // is to be flushed here.
    // Note, that we must start with the first DCB, which isn't necessarily the same
    // as the pointer to the allocated memory, since the first DCB is cache aligned.
    // If we didn't do it like this, it might happen that we flush a cache line too
    // soon, which might be someone else's memory that is not supposed to be flushed.
    VTSS_OS_DCACHE_FLUSH(new.head, new.dcb_cnt * dcb_size_bytes + frm_mem_req_bytes);

    // Gracefully change the frame set on the AFI channel
    afi_frame_set_change(vstate, afi_ch, &new, new_user_frames, cancelled_user_frame);

    return VTSS_RC_OK;

do_exit_with_error:
    VTSS_OS_FREE(new.dcb_and_frm_mem, VTSS_MEM_FLAGS_DMA | VTSS_MEM_FLAGS_PERSIST);
    return VTSS_RC_ERROR;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_update_bw()
/*****************************************************************************/
static void afi_update_bw(afi_user_frames_t *user_frames)
{
    int i;
    u64 bw = 0;

    for (i = 0; i < AFI_FREQ_LIST_LEN; i++) {
        vtss_fdma_list_t *dcb_ptr = FDMA_USER_DCB(user_frames->list[i].frm_list);
        u32              fps = user_frames->list[i].fps;
        while (dcb_ptr) {
            u32 frm_sz;
#if VTSS_OPT_FDMA_VER <= 2
            // When using FDMA API v2-, act_len includes IFH and FCS
            frm_sz = dcb_ptr->act_len - VTSS_FDMA_HDR_SIZE_BYTES;
#else
            // When using FDMA API v3+, act_len excludes IFH and excludes FCS
            frm_sz = dcb_ptr->act_len + 4 /* FCS */;
#endif
            bw += (u64)fps * frm_sz;
            dcb_ptr = FDMA_SW_DCB(dcb_ptr)->afi_next;
        }
    }
    user_frames->bw_consumption_bytes_per_second = bw;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_link()
// Insert @item into @user_frames->frm_list, using the afi_next pointer.
/*****************************************************************************/
static FDMA_INLINE void afi_link(afi_user_frames_t *user_frames, fdma_sw_dcb_t *item, int idx, u32 fps)
{
    // In order not to change the timing of the existing frames too much, we need to
    // insert the new frame at the end of the list, which is a bit more cumbersome
    // compared to inserting it first.
    if (user_frames->list[idx].frm_list) {
        vtss_fdma_list_t *dcb = FDMA_USER_DCB(user_frames->list[idx].frm_list);
        // Use the afi_next field so that we don't disturb the interrupt handler.
        while (FDMA_SW_DCB(dcb)->afi_next) {
            dcb = FDMA_SW_DCB(dcb)->afi_next;
        }
        FDMA_SW_DCB(dcb)->afi_next = FDMA_USER_DCB(item);
    } else {
        user_frames->list[idx].frm_list = item;
    }

    user_frames->list[idx].fps = fps;
    user_frames->list[idx].frm_cnt++;
    user_frames->frm_cnt++;

    // Compute new bandwidth consumption.
    afi_update_bw(user_frames);
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_unlink()
// Remove @item from @user_frames->frm_list, using the afi_next pointer.
// Return VTSS_RC_OK if found and removed, otherwise VTSS_RC_ERROR.
/*****************************************************************************/
static FDMA_INLINE vtss_rc afi_unlink(afi_user_frames_t *user_frames, fdma_sw_dcb_t *item, int idx)
{
    fdma_sw_dcb_t *dcb = user_frames->list[idx].frm_list;
    if (dcb == item) {
        user_frames->list[idx].frm_list = FDMA_SW_DCB(item->afi_next);
    } else {
        while (dcb && FDMA_SW_DCB(dcb->afi_next) != item) {
            dcb = FDMA_SW_DCB(dcb->afi_next);
        }

        // If dcb == NULL, the item was not found.
        if (dcb == NULL) {
            return VTSS_RC_ERROR;
        }
        // @dcb now points to the DCB just before the one we want to remove.
        dcb->afi_next = FDMA_SW_DCB(dcb->afi_next)->afi_next;
    }
    item->afi_next = NULL;

    user_frames->frm_cnt--;
    user_frames->list[idx].frm_cnt--;
    if (user_frames->list[idx].frm_cnt == 0) {
        // Free the entry
        user_frames->list[idx].fps = 0;
    }

    // Compute new bandwidth consumption.
    afi_update_bw(user_frames);
    return VTSS_RC_OK;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_link_and_initialize()
/*****************************************************************************/
#if VTSS_OPT_FDMA_VER <= 2
static vtss_rc afi_link_and_initialize(vtss_state_t *const vstate, vtss_fdma_ch_t afi_ch, fdma_sw_dcb_t *sw_dcb, int idx, afi_user_frames_t *new_user_frames, vtss_fdma_inj_props_t *props)
#else
static vtss_rc afi_link_and_initialize(vtss_state_t *const vstate, vtss_fdma_ch_t afi_ch, fdma_sw_dcb_t *sw_dcb, int idx, afi_user_frames_t *new_user_frames, vtss_fdma_tx_info_t *const props, vtss_packet_tx_info_t *const tx_info)
#endif /* VTSS_OPT_FDMA_VER */
{
    fdma_sw_dcb_t        *new_tail;
    u32                  dcb_cnt;
#if VTSS_OPT_FDMA_VER >= 3
    u32                  dummy_frm_size;
    vtss_packet_tx_grp_t dummy_inj_grp;
#endif /* VTSS_OPT_FDMA_VER >= 3 */

    afi_link(new_user_frames, sw_dcb, idx, AFI_FPS(props));

#if VTSS_OPT_FDMA_VER <= 2
    // It's a fatal if we can't initialize IFH and H/W DCB.
    FDMA_ASSERT(NORMAL, inj_init(vstate, afi_ch, sw_dcb, FDMA_USER_DCB(sw_dcb)->act_len - VTSS_FDMA_HDR_SIZE_BYTES, props, &new_tail, &dcb_cnt)                    == VTSS_RC_OK, return VTSS_RC_ERROR;);
#else
    FDMA_ASSERT(NORMAL, inj_ifh_init(vstate, sw_dcb, tx_info, &dummy_inj_grp, &dummy_frm_size)                                                                     == VTSS_RC_OK, return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, inj_dcb_init(vstate, sw_dcb, vstate->fdma_state.fdma_ch_state[afi_ch].inj_grp, &new_tail, &dcb_cnt, sw_dcb->dcb_type, FALSE, FALSE, FALSE) == VTSS_RC_OK, return VTSS_RC_ERROR;);
#endif /* VTSS_OPT_FDMA_VER */
    // One single DCB (the tail must therefore be identical to the list itself).
    FDMA_ASSERT(NORMAL, new_tail == sw_dcb && dcb_cnt == 1,                                                                                                                       return VTSS_RC_ERROR;);

    // Use an unused bit in the DCB to indicate whether frame counting and seq_numbering is enabled for this frame.
    AFI_DCB_INIT_SPECIAL_FIELDS(sw_dcb);
    AFI_COUNTING_ENA_SET(sw_dcb, AFI_ENABLE_COUNTING(props));
    AFI_SEQ_NUM_ENA_SET( sw_dcb, AFI_ENABLE_SEQUENCE_NUMBERING(props));
    AFI_FRM_CNT(FDMA_USER_DCB(sw_dcb)) = 0;
    sw_dcb->afi_frm_incr               = 0;
    sw_dcb->afi_frm_incr_next          = 0;
    // Use hw_dcb->stat to hold the requested sequence numbering offset.
    FDMA_HW_DCB(sw_dcb)->stat = AFI_SEQUENCE_NUMBER_OFFSET(props);

    if (AFI_ENABLE_SEQUENCE_NUMBERING(props)) {
        // Initialize the next sequence number from the frame data.
        // It's four bytes long and in network order in the frame.
        AFI_SEQ_NUMBER(FDMA_USER_DCB(sw_dcb)) = (FDMA_USER_DCB(sw_dcb)->frm_ptr[AFI_SEQUENCE_NUMBER_OFFSET(props) + 0] << 24) |
                                                (FDMA_USER_DCB(sw_dcb)->frm_ptr[AFI_SEQUENCE_NUMBER_OFFSET(props) + 1] << 16) |
                                                (FDMA_USER_DCB(sw_dcb)->frm_ptr[AFI_SEQUENCE_NUMBER_OFFSET(props) + 2] <<  8) |
                                                (FDMA_USER_DCB(sw_dcb)->frm_ptr[AFI_SEQUENCE_NUMBER_OFFSET(props) + 3] <<  0);
    } else {
        AFI_SEQ_NUMBER(FDMA_USER_DCB(sw_dcb)) = 1;
    }

    return VTSS_RC_OK;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_find_frame()
/*****************************************************************************/
static fdma_sw_dcb_t *afi_find_frame(vtss_state_t *const vstate, vtss_fdma_ch_t *afi_ch, fdma_sw_dcb_t *item, int *idx)
{
    vtss_fdma_ch_t afi_ch_min, afi_ch_max;
    u8             *frm_ptr = FDMA_USER_DCB(item)->frm_ptr;

    // Gotta search through one or more channels to find the one that #item contains.
    if (*afi_ch == AFI_CH_AUTO) {
        afi_ch_min = 0;
        afi_ch_max = VTSS_FDMA_AFI_CH_MAX - 1;
    } else {
        afi_ch_min = afi_ch_max = *afi_ch;
    }

    for (*afi_ch = afi_ch_min; *afi_ch <= afi_ch_max; (*afi_ch)++) {
        fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[*afi_ch];
        if (ch_state->usage == VTSS_FDMA_CH_USAGE_CCM && ch_state->afi_state.user_frames.frm_cnt > 0) {
            for (*idx = 0; *idx < AFI_FREQ_LIST_LEN; (*idx)++) {
                vtss_fdma_list_t *user_dcb = FDMA_USER_DCB(ch_state->afi_state.user_frames.list[*idx].frm_list);
                while (user_dcb) {
                    if (user_dcb->frm_ptr == frm_ptr) {
                        return FDMA_SW_DCB(user_dcb);
                    }
                    user_dcb = user_dcb->next;
                }
            }
        }
    }
    return NULL;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_do_cancel()
/*****************************************************************************/
static FDMA_INLINE vtss_rc afi_do_cancel(vtss_state_t *const vstate, vtss_fdma_ch_t afi_ch, fdma_sw_dcb_t *item)
{
    afi_user_frames_t new_user_frames;
    vtss_rc           result;
    int               idx;
    fdma_sw_dcb_t     *sw_dcb;

    VTSS_IG(VTSS_TRACE_GROUP_FDMA_NORMAL, "AFI: Cancel start");

    sw_dcb = afi_find_frame(vstate, &afi_ch, item, &idx);

    FDMA_ASSERT(NORMAL, sw_dcb != NULL, return VTSS_RC_ERROR;); // Frame found?

    // Found the channel.
    // Take a snapshot of the current list of user frames and link them using the afi_next field in
    // order not to disturb the interrupt handler which is using the current set of user frames.
    afi_cp_user_frame_lists(&new_user_frames, &vstate->fdma_state.fdma_ch_state[afi_ch].afi_state.user_frames, FALSE);

    // Take the frame out of the list.
    FDMA_ASSERT(NORMAL, afi_unlink(&new_user_frames, sw_dcb, idx) == VTSS_RC_OK, return VTSS_RC_ERROR;);

    // And rebuild the DCB list for this channel. If afi_build() fails here, it's a coding error.
    result = afi_build(vstate, afi_ch, &new_user_frames, sw_dcb);

    VTSS_IG(VTSS_TRACE_GROUP_FDMA_NORMAL, "AFI: Cancel done (ch=%d)", afi_ch);

    return result;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// afi_inj()
// Start or cancel periodic injection of a frame.
/*****************************************************************************/
#if VTSS_OPT_FDMA_VER <= 2
static FDMA_INLINE vtss_rc afi_inj(vtss_state_t *const vstate, vtss_fdma_ch_t afi_ch, fdma_sw_dcb_t *sw_dcb, vtss_fdma_inj_props_t *props, BOOL afi_cancel)
#else
static FDMA_INLINE vtss_rc afi_inj(vtss_state_t *const vstate, vtss_fdma_ch_t afi_ch, fdma_sw_dcb_t *sw_dcb, vtss_fdma_tx_info_t *const props, vtss_packet_tx_info_t *const tx_info, BOOL afi_cancel)
#endif /* VTSS_OPT_FDMA_VER */
{
    fdma_state_t          *state = &vstate->fdma_state;
    int                   i, add_idx;
    afi_user_frames_t     new_user_frames;
    afi_user_frames_t     *user_frames;
    fdma_ch_state_t       *ch_state;
    fdma_afi_state_t      *afi_state;
    vtss_fdma_ch_t        afi_ch_min, afi_ch_max;
    BOOL                  ch_suitable;
#if VTSS_OPT_FDMA_VER <= 2
    // Make an alias for unified code below.
    vtss_fdma_inj_props_t *tx_info = props;
#endif /* VTSS_OPT_FDMA_VER <= 2 */

    if (afi_cancel == FALSE) {
        // pre_cb not supported for AFI flows.
        FDMA_ASSERT(NORMAL, props->pre_cb == NULL,                                         return VTSS_RC_ERROR;);
        // Cannot be switched.
        FDMA_ASSERT(NORMAL, tx_info->switch_frm == FALSE,                                  return VTSS_RC_ERROR;);
#if defined(VTSS_FEATURE_VSTAX)
#if VTSS_OPT_FDMA_VER <= 2
        // Cannot contain a stack header
        FDMA_ASSERT(NORMAL, tx_info->contains_stack_hdr == FALSE,                          return VTSS_RC_ERROR;);
#else
        // Cannot contain a stack header
        FDMA_ASSERT(NORMAL, tx_info->tx_vstax_hdr == VTSS_PACKET_TX_VSTAX_NONE,            return VTSS_RC_ERROR;);
#endif /* VTSS_OPT_FDMA_VER */
#endif /* VTSS_FEATURE_VSTAX */

#if VTSS_OPT_FDMA_VER <= 2
        // Super-prio inject not supported for AFI flows.
        FDMA_ASSERT(NORMAL, tx_info->qos_class < 8,                                        return VTSS_RC_ERROR;);

        // The afi_sequence_number_offset must be within the frame boundaries.
        // FDMA_USER_DCB(sw_dcb)->act_len includes IFH and FCS, so the raw frame data is VTSS_FDMA_HDR_SIZE_BYTES + 4 shorter.
        // Furthermore, the offset must be small enough to accommodate a 4-byte update of the frame, hence the
        // subtraction of four bytes more.
        FDMA_ASSERT(NORMAL, !AFI_ENABLE_SEQUENCE_NUMBERING(props) ||
                             AFI_SEQUENCE_NUMBER_OFFSET(props) <= (FDMA_USER_DCB(sw_dcb)->act_len - VTSS_FDMA_HDR_SIZE_BYTES - 4 /* FCS_SIZE_BYTES */ - 4), return VTSS_RC_ERROR;);
#else
        // Super-prio inject not supported for AFI flows.
        FDMA_ASSERT(NORMAL, tx_info->cos < 8,                                              return VTSS_RC_ERROR;);

        // The afi_sequence_number_offset must be within the frame boundaries.
        // FDMA_USER_DCB(sw_dcb)->act_len contains the raw frame data, excluding IFH and FCS.
        // Furthermore, the offset must be small enough to accommodate a 4-byte update of the frame, hence the
        // subtraction of four bytes more.
        FDMA_ASSERT(NORMAL, !AFI_ENABLE_SEQUENCE_NUMBERING(props) ||
                             AFI_SEQUENCE_NUMBER_OFFSET(props) <= (FDMA_USER_DCB(sw_dcb)->act_len - 4), return VTSS_RC_ERROR;);
#endif /* VTSS_OPT_FDMA_VER */
        // The frame frequency must be non-zero
        FDMA_ASSERT(NORMAL, AFI_FPS(props) > 0 && AFI_FPS(props) <= VTSS_AFI_FPS_MAX, return VTSS_RC_ERROR;);
    }

    if (afi_cancel) {
        return afi_do_cancel(vstate, afi_ch, sw_dcb);
    }

    // Gotta search through a number of channels to find a suitable for this new AFI frame
    if (afi_ch == AFI_CH_AUTO) {
        afi_ch_min = 0;
        afi_ch_max = VTSS_FDMA_AFI_CH_MAX - 1;
    } else {
        afi_ch_min = afi_ch_max = afi_ch;
    }

    VTSS_IG(VTSS_TRACE_GROUP_FDMA_NORMAL, "AFI: Add start");

    // If we get here, the user wants to add a new flow. Find a suitable channel.
    for (afi_ch = afi_ch_min; afi_ch <= afi_ch_max; afi_ch++) {
        // First go through all channels currently in use to see if we can find a suitable
        // channel. If we just picked the first available, we could end up using all channels
        // on flows that could have run on the same channel, thereby disallowing the user to
        // add an Nth flow that doesn't fit the running channels.
        ch_state = &state->fdma_ch_state[afi_ch];

        if (ch_state->usage == VTSS_FDMA_CH_USAGE_CCM && ch_state->afi_state.user_frames.frm_cnt > 0) {
            afi_state   = &ch_state->afi_state;
            user_frames = &afi_state->user_frames;

            // Channel configured for AFI and in use.
            ch_suitable = TRUE;
            add_idx     = -1;

            // Gotta loop through all the currently handled frequencies to see if the new frame matches.
            // Iterate in reverse order to get the lowest free index in case of no other frames
            // with this frequency (it's not really needed for the further processing, but nice
            // when debugging).
            for (i = AFI_FREQ_LIST_LEN - 1; i >= 0; i--) {
                u32 fps = user_frames->list[i].fps;

                if (fps == AFI_FPS(props)) {
                    // No need to search further, and no need to check whether it's a multiple of all others or vice versa.
                    add_idx = i;
                    break;
                } else if (fps == 0) {
                    // add_idx is just a candidate. To ease debugging, we should get the lowest index.
                    // Also, we must check against all used frequencies on this channel, so we cannot stop now.
                    add_idx = i;
                } else if (AFI_FPS(props) > fps) {
                    if ((AFI_FPS(props) % fps) != 0 || AFI_FPS(props) / fps > afi_state->quotient_max) {
                        ch_suitable = FALSE; // Doesn't fit. Next channel.
                        break;
                    }
                } else if ((fps % AFI_FPS(props)) != 0 || fps / AFI_FPS(props) > afi_state->quotient_max) {
                    ch_suitable = FALSE; // Doesn't fit. Next channel.
                    break;
                }
            }

            if (add_idx == -1 || ch_suitable == FALSE) {
                // Either this channel is not suitable for the new frame (frequency
                // not a multiple of the other frequencies on the channel or out of range)
                // or there's not room. Try the next channel.
                continue;
            }

            // Hey - we found a channel candidate. Our sorrows aren't over until we've
            // successfully built a new set of frames on this channel.

            // Take a snapshot of the current list of user frames and link them using the afi_next field in
            // order not to disturb the interrupt handler which is using the current set of user frames.
            afi_cp_user_frame_lists(&new_user_frames, &ch_state->afi_state.user_frames, FALSE);

            // Link the new user frame into the existing list (using afi_next) and initialize user DCB,
            // so that it can be used as a signature DCB. We can't do that before this point, because
            // we haven't known which channel to use.
#if VTSS_OPT_FDMA_VER <= 2
            if (afi_link_and_initialize(vstate, afi_ch, sw_dcb, add_idx, &new_user_frames, props)          != VTSS_RC_OK) {
#else
            if (afi_link_and_initialize(vstate, afi_ch, sw_dcb, add_idx, &new_user_frames, props, tx_info) != VTSS_RC_OK) {
#endif /* VTSS_OPT_FDMA_VER */
                // It's a fatal if that function fails (an error is already printed).
                return VTSS_RC_ERROR;
            }

            // Finally, try to build the list of DCBs to hand over to the GPDMA.
            // If this fails, we must go on with the next channel.
            if (afi_build(vstate, afi_ch, &new_user_frames, NULL) == VTSS_RC_OK) {
                VTSS_IG(VTSS_TRACE_GROUP_FDMA_NORMAL, "AFI: Add done (in-use ch=%d)", afi_ch);
                return VTSS_RC_OK;
            }

            // Frame didn't fit entirely into that channel. Try the next.
        }
    }

    // If we get here, it's because no currently used channels were suitable for this frame.
    // Find an unused and try to add it there.
    for (afi_ch = afi_ch_min; afi_ch <= afi_ch_max; afi_ch++) {
        ch_state = &state->fdma_ch_state[afi_ch];
        if (ch_state->usage == VTSS_FDMA_CH_USAGE_CCM && ch_state->afi_state.user_frames.frm_cnt == 0) {

            // Take a snapshot of the current list of user frames (well, it's empty, but use
            // that function to create a new list of user frames)
            afi_cp_user_frame_lists(&new_user_frames, &ch_state->afi_state.user_frames, FALSE);

            // Link the new user frame into the first item of the new list and initialize user DCB,
            // so that it can be used as a signature DCB. We can't do that before this point, because
            // we haven't known which channel to use.
#if VTSS_OPT_FDMA_VER <= 2
            if (afi_link_and_initialize(vstate, afi_ch, sw_dcb, 0, &new_user_frames, props)          != VTSS_RC_OK) {
#else
            if (afi_link_and_initialize(vstate, afi_ch, sw_dcb, 0, &new_user_frames, props, tx_info) != VTSS_RC_OK) {
#endif /* VTSS_OPT_FDMA_VER */
                // It's a fatal if that function fails (an error is already printed).
                return VTSS_RC_ERROR;
            }

            // Finally, try to build the list of DCBs to hand over to the GPDMA.
            // If this fails, it's a fatal, because the channel is not already in use.
            FDMA_ASSERT(NORMAL, afi_build(vstate, afi_ch, &new_user_frames, NULL) == VTSS_RC_OK, return VTSS_RC_ERROR;);
            VTSS_IG(VTSS_TRACE_GROUP_FDMA_NORMAL, "AFI: Add done (free ch=%d)", afi_ch);
            return VTSS_RC_OK;
        }
    }

    // If we get here, the frame didn't fit into running channels, and there are no free channels left.
    /*lint -e(506) */ // Avoid Lint Warning: "Constant value Boolean"
    FDMA_ASSERT(NORMAL, FALSE, return VTSS_RC_ERROR;);

    return VTSS_RC_ERROR; // Unreachable, but satisfies Lint.
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_FEATURE_AFI_FDMA)
/*****************************************************************************/
// print_bw_consumption()
/*****************************************************************************/
static FDMA_INLINE void print_bw_consumption(const vtss_debug_printf_t pr, u64 bytes_per_second)
{
    u64 bps = bytes_per_second * 8;
    // Scale the printing.
    if (bps < 1000000ULL) {
        // Less than 1Mbps. Print it in bps.
        pr("B/W Consumption: %" PRIu64 " bps\n", bps);
    } else if (bps < 100000000ULL) {
        // Less than 100 Mbps. Print it in kbps.
        pr("B/W Consumption: %" PRIu64 " kbps\n", VTSS_DIV64(bps, 1000ULL));
    } else {
        // 100 Mbps or above. Print it in Mbps.
        pr("B/W Consumption: %" PRIu64 " Mbps\n", VTSS_DIV64(bps, 1000000ULL));
    }
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

/*****************************************************************************/
// ch_state_defaults()
/*****************************************************************************/
static FDMA_INLINE void ch_state_defaults(vtss_state_t *const vstate)
{
#if VTSS_OPT_FDMA_VER >= 3
#if defined(VTSS_FEATURE_AFI_FDMA)
    vtss_fdma_ch_t  ch;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
    fdma_ch_state_t *ch_state;

    // The FDMA API v3+ makes its own channel setup. Enabling happens through a call to vtss_fdma_cfg()
    ch_state = &vstate->fdma_state.fdma_ch_state[0];
    ch_state->usage        = VTSS_FDMA_CH_USAGE_XTR;
    ch_state->prio         = 7;
    ch_state->xtr_grp      = 0;

#if defined(VTSS_FEATURE_AFI_FDMA)
    for (ch = 1; ch <= 3; ch++) {
        ch_state = &vstate->fdma_state.fdma_ch_state[ch];
        ch_state->usage                  = VTSS_FDMA_CH_USAGE_CCM;
        ch_state->prio                   = 0; // Lowest priority
        ch_state->inj_grp                = 1;
        ch_state->inj_grp_mask           = VTSS_BIT(ch_state->inj_grp);
        ch_state->afi_state.quotient_max = VTSS_FDMA_CCM_QUOTIENT_MAX;

        // Create a dummy DCB.
        // The dummy DCB itself is never handed over to the FDMA,
        // but is merely used as a template for the dummy DCBs that are really
        // handed over to the FDMA).
        FDMA_ASSERT(NORMAL, afi_create_dummy_dcb(vstate, ch) == VTSS_RC_OK, return;);

        FDMA_USER_DCB(&ch_state->afi_state.dummy_dcb)->next = NULL;
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    ch_state = &vstate->fdma_state.fdma_ch_state[VTSS_FDMA_CH_INJ];
    ch_state->usage        = VTSS_FDMA_CH_USAGE_INJ;
    ch_state->prio         = 7;
#if defined(VTSS_ARCH_LUTON26)
    ch_state->inj_grp_mask = 1 << 0; // Group 0
#else /* Jaguar */
    ch_state->inj_grp_mask = (1 << VTSS_PACKET_TX_GRP_CNT) - 1; // Take over all injection groups.
#endif /* Luton26/Jaguar */

#if defined(VTSS_ARCH_JAGUAR_1)
    if (vstate->chip_count == 2) {
        // This entry is an artificial entry used to extract frames from a secondary JR in a dual-chip solution.
        // The channel number is completely arbitrary, and the FDMA H/W will not be used to extract frames due
        // to a chip bug (extraction queue number not obeyed if auto-forwarding received CPU frames from secondary
        // to primary chip). Instead, manual extraction will be performed.
        ch_state = &vstate->fdma_state.fdma_ch_state[5];
        ch_state->usage   = VTSS_FDMA_CH_USAGE_XTR;
        ch_state->prio    = 7; // Doesn't matter, since we're extracting manually.
        ch_state->xtr_grp = 0; // Doesn't need to match channel number on chip_no == 1, since we're using manual extraction.
        ch_state->chip_no = 1;
    }
#endif /* VTSS_ARCH_JAGUAR_1 */
#endif /* VTSS_OPT_FDMA_VER >= 3 */
}

/*****************************************************************************/
//
// PUBLIC FUNCTIONS
//
//****************************************************************************/

/*****************************************************************************/
// vcoreiii_fdma_init_conf_set()
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_init_conf_set(vtss_state_t *const vstate)
{
    fdma_state_t *state = &vstate->fdma_state;

    if (!check_version(vstate)) {
        return VTSS_RC_ERROR; // Unsupported silicon
    }

    // Disable all channels.
    disable_chs(vstate, VTSS_BITMASK(VTSS_FDMA_CH_CNT));

    // Globally enable DMA controller
    VCOREIII_WR(vstate, VTSS_FDMA_MISC_DMA_CFG_REG, VTSS_F_FDMA_MISC_DMA_CFG_REG_DMA_EN);

    // Enable FDMA's access to the queue system.
    VCOREIII_WR(vstate, VTSS_ICPU_CFG_GPDMA_FDMA_CFG, VTSS_F_ICPU_CFG_GPDMA_FDMA_CFG_FDMA_ENA);

    // Create new channel state defaults
    ch_state_defaults(vstate);

    state->hdr_size     = VTSS_FDMA_HDR_SIZE_BYTES;
    state->xtr_hdr_size = VTSS_FDMA_XTR_HDR_SIZE_BYTES;
    state->inj_hdr_size = VTSS_FDMA_INJ_HDR_SIZE_BYTES;
    state->xtr_ch_cnt   = vstate->chip_count;

    // The allocation length we configure in hw_dcb->sar must be a multiple of the GPDMA burst size.
    // The MSIZE enumeration table looks like:
    //   MSIZE == 0 =>   1 item
    //   MSIZE == 1 =>   4 items
    //   MSIZE == 2 =>   8 items
    //   MSIZE == 3 =>  16 items
    //   MSIZE == 4 =>  32 items
    //   MSIZE == 5 =>  64 items
    //   MSIZE == 6 => 128 items
    //   MSIZE == 7 => 256 items
    // We multiple the number of items by 4 to get it in bytes, since we always transfer 32-bit items.
#if XTR_SRC_MSIZE == 0
    state->xtr_burst_size_bytes = 4 * 1;
#else
    state->xtr_burst_size_bytes = 4 * 2 * (1 << (XTR_SRC_MSIZE));
#endif /* XTR_SRC_MSIZE */

#if defined(VTSS_FEATURE_AFI_FDMA)
    {
        // We gotta do a one-time detection of the CPU frequency for use in the AFI timer setup.
        u32 cpu_clk_div = VTSS_X_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CPU_CLK_DIV(VCOREIII_RD(vstate, VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0));
        switch (cpu_clk_div) {
        case 2:
            state->ahb_clk_period_ps =  4000; // MIPS = 500.0 MHz => AHB = 250.00 MHz => AHB period =  4.0 ns =  4000 ps
            break;

        case 5:
            state->ahb_clk_period_ps =  8000; // MIPS = 250.0 MHz => AHB = 125.00 MHz => AHB period =  8.0 ns =  8000 ps
            break;

        case 6: /* Normal configuration */
            state->ahb_clk_period_ps =  4800; // MIPS = 416.6 MHz => AHB = 208.33 MHz => AHB period =  4.8 ns =  4800 ps
            break;

        case 14:
            state->ahb_clk_period_ps =  6400; // MIPS = 312.5 MHz => AHB = 156.25 MHz => AHB period =  6.4 ns =  6400 ps
            break;

        case 15:
            state->ahb_clk_period_ps = 12000; // MIPS = 166.6 MHz => AHB =  83.33 MHz => AHB period = 12.0 ns = 12000 ps
            break;

        default:
            // Avoid Lint Warning: "Constant value Boolean"
            /*lint -e(506) */
            FDMA_ASSERT(NORMAL, FALSE, return VTSS_RC_ERROR;);
        }
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    return VTSS_RC_OK;
}

/*****************************************************************************/
// vcoreiii_fdma_uninit()
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_uninit(vtss_state_t *const vstate)
{
    // Globally disable DMA controller
    VCOREIII_WR(vstate, VTSS_ICPU_CFG_GPDMA_FDMA_CFG, 0);

    // Disable all channels
    disable_chs(vstate, VTSS_BITMASK(VTSS_FDMA_CH_CNT));

    // Disable interrupts from DMA controller
    VCOREIII_WR(vstate, VTSS_FDMA_INTR_MASK_TFR,   VTSS_F_FDMA_INTR_MASK_TFR_INT_MASK_WE_TFR    (VTSS_BITMASK(VTSS_FDMA_CH_CNT)));
    VCOREIII_WR(vstate, VTSS_FDMA_INTR_MASK_BLOCK, VTSS_F_FDMA_INTR_MASK_BLOCK_INT_MASK_WE_BLOCK(VTSS_BITMASK(VTSS_FDMA_CH_CNT)));

    // Clear pending interrupts
    clear_pending_block_done_intr(vstate, VTSS_BITMASK(VTSS_FDMA_CH_CNT));
    clear_pending_tfr_done_intr  (vstate, VTSS_BITMASK(VTSS_FDMA_CH_CNT));
    return VTSS_RC_OK;
}

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vcoreiii_fdma_ch_cfg()
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_ch_cfg(      vtss_state_t       *const vstate,
                                    const vtss_fdma_ch_t     ch,
                                    const vtss_fdma_ch_cfg_t *const cfg)
{
    fdma_state_t *state = &vstate->fdma_state;

    FDMA_ASSERT(NORMAL, cfg != NULL, return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, ch >= 0 && ch < VTSS_FDMA_CH_CNT, return VTSS_RC_ERROR;);

    if (cfg->usage == VTSS_FDMA_CH_USAGE_UNUSED) {
        // User wants to disable this channel.
        disable_ch(vstate, ch);
        state->fdma_ch_state[ch].usage  = VTSS_FDMA_CH_USAGE_UNUSED;
        state->fdma_ch_state[ch].status = FDMA_CH_STATUS_DISABLED;
        // Disable block (enabled if the channel was used for extraction) and transfer done interrupts (enabled for both extraction and injection).
        // Clearing of the interrupts occur if the channel gets re-enabled.
        disable_block_done_intr(vstate, ch);
        disable_tfr_done_intr(vstate, ch);
    } else if (cfg->usage == VTSS_FDMA_CH_USAGE_XTR) {
        return xtr_cfg(vstate, ch, cfg);
    } else if (cfg->usage == VTSS_FDMA_CH_USAGE_INJ) {
        return inj_cfg(vstate, ch, cfg);
#if defined(VTSS_FEATURE_AFI_FDMA)
    } else if(cfg->usage == VTSS_FDMA_CH_USAGE_CCM) {
        return afi_cfg(vstate, ch, cfg);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
    } else {
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vcoreiii_fdma_inj()
// @len: Length of frame in bytes, excluding IFH but including FCS.
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_inj(      vtss_state_t          *const vstate,
                                       vtss_fdma_list_t      *user_dcb,
                                 const vtss_fdma_ch_t        the_inj_ch,
                                 const u32                   len,
                                 const vtss_fdma_inj_props_t *const inj_props)
{
    vtss_fdma_ch_t        inj_ch = the_inj_ch; // Allows for local modifications.
    fdma_sw_dcb_t         *new_tail = NULL;
    u32                   dcb_cnt = 0;
    vtss_fdma_inj_props_t the_props = *inj_props; // Get a local copy that we can modify
    vtss_fdma_inj_props_t *props    = &the_props; // This is quite new code, and we don't wanna change everything below.

    FDMA_ASSERT(NORMAL, user_dcb,                                                            return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, props->chip_no < vstate->chip_count,                                 return VTSS_RC_ERROR;);
#if defined(VTSS_FEATURE_AFI_FDMA)
    FDMA_ASSERT(NORMAL, props->switch_frm || props->port_mask != 0 || props->ccm_cancel,     return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, (inj_ch >= 0 && inj_ch < VTSS_FDMA_CH_CNT) || inj_ch == AFI_CH_AUTO, return VTSS_RC_ERROR;);
#else
    FDMA_ASSERT(NORMAL, props->switch_frm || props->port_mask != 0,                          return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, inj_ch >= 0 && inj_ch < VTSS_FDMA_CH_CNT,                            return VTSS_RC_ERROR;);
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
    if (props->switch_frm == FALSE && props->contains_stack_hdr == FALSE && props->chip_no == 1) {
        int                    p    = 0;
        u64                    mask = 0x1;
        vtss_vstax_tx_header_t vs_hdr;

        // This frame is supposed to be sent to a front port on the secondary chip.
        // In order to achieve that, we inject it on one of the internal ports (connected to the secondary chip)

        // We don't support multicasting on the secondary chip, so find the port that we're interested in.
        for (p = 0; p < 64; p++) {
            if (props->port_mask & mask) {
                break;
            }
            mask <<= 1;
        }

        props->port_mask &= ~mask;
        // No more ports left in port_mask (since we don't support multicasting on secondary chip)
        FDMA_ASSERT(NORMAL, props->port_mask == 0, return VTSS_RC_ERROR;);

        memset(&vs_hdr, 0, sizeof(vs_hdr));
        vs_hdr.fwd_mode  = VTSS_VSTAX_FWD_MODE_UPSID_PORT;
        vs_hdr.ttl       = 1;
        vs_hdr.prio      = props->qos_class == 8 ? 7 : props->qos_class;
        vs_hdr.upsid     = vstate->vstax_info.upsid[props->chip_no];
        vs_hdr.port_no   = VTSS_PORT_NO_NONE; // To force header2frame() to use already converted chip_port.
        vs_hdr.chip_port = p;
        vs_hdr.glag_no   = VTSS_GLAG_NO_NONE; // Disable GLAG source filtering.
        FDMA_ASSERT(NORMAL, vstate->cil_func.vstax_header2frame, return VTSS_RC_ERROR;);
        (void)vstate->cil_func.vstax_header2frame(vstate, VTSS_PORT_NO_NONE, &vs_hdr, props->stack_hdr_bin);
        props->contains_stack_hdr = TRUE;
        props->port_mask = 1LLU << vstate->port_int_0;
        props->inj_grp_auto = FALSE;
        // Send the frame through injection group 0, which is for normal injection. It will hit
        // the interconnect port only.
        props->inj_grp = 0;
    }
#endif /* VTSS_ARCH_JAGUAR_1 && VTSS_FEATURE_VSTAX */

#if defined(VTSS_FEATURE_AFI_FDMA)
    if (inj_ch == AFI_CH_AUTO || vstate->fdma_state.fdma_ch_state[inj_ch].usage == VTSS_FDMA_CH_USAGE_CCM) {
        // Only one single DCB for AFI flows.
        FDMA_ASSERT(NORMAL, props->ccm_cancel == TRUE || user_dcb->next == NULL, return VTSS_RC_ERROR;);

        // Handle the rest in a separate function.
        return afi_inj(vstate, inj_ch, FDMA_SW_DCB(user_dcb), props, props->ccm_cancel);
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    // The remainder takes care of normal injection.
    FDMA_ASSERT(NORMAL, inj_init(vstate, inj_ch, FDMA_SW_DCB(user_dcb), len, props, &new_tail, &dcb_cnt) == VTSS_RC_OK, return VTSS_RC_ERROR;);

    VTSS_IG_HEX(VTSS_TRACE_GROUP_FDMA_NORMAL, FDMA_SW_DCB(user_dcb)->ifh_ptr, 64);

    return vtss_fdma_cmn_inj_attach_frm(vstate, FDMA_SW_DCB(user_dcb), new_tail, inj_ch, dcb_cnt, len, props, FALSE);
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// vcoreiii_fdma_tx()
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_tx(vtss_state_t          *const vstate,
                                fdma_sw_dcb_t         *sw_dcb,
                                vtss_fdma_tx_info_t   *const fdma_info,
                                vtss_packet_tx_info_t *const tx_info,
                                BOOL                  afi_cancel)
{
    vtss_packet_tx_grp_t inj_grp;
    fdma_sw_dcb_t        *tail;
    u32                  dcb_cnt, frm_size;

#if defined(VTSS_FEATURE_AFI_FDMA)
    if (AFI_FPS(fdma_info) || afi_cancel) {
        // Only one single DCB for AFI flows.
        FDMA_ASSERT(NORMAL, afi_cancel == TRUE || FDMA_USER_DCB(sw_dcb)->next == NULL, return VTSS_RC_ERROR;);

        // Handle the rest in a separate function.
        return afi_inj(vstate, AFI_CH_AUTO, sw_dcb, fdma_info, tx_info, afi_cancel);
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    VTSS_RC(inj_ifh_init(vstate, sw_dcb, tx_info, &inj_grp, &frm_size));
    VTSS_RC(inj_dcb_init(vstate, sw_dcb, inj_grp, &tail, &dcb_cnt, sw_dcb->dcb_type, TRUE, FALSE, FALSE));
    return vtss_fdma_cmn_inj_attach_frm(vstate, sw_dcb, tail, VTSS_FDMA_CH_INJ, dcb_cnt, frm_size /* for statistics */, fdma_info, FALSE);
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if defined(VTSS_FEATURE_AFI_FDMA) && VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// vcoreiii_fdma_afi_frm_cnt()
/*****************************************************************************/
vtss_rc vcoreiii_fdma_afi_frm_cnt(vtss_state_t *const vstate, const u8 *const frm_ptr, u64 *const frm_cnt)
{
    fdma_sw_dcb_t  *sw_dcb, dummy_dcb;
    int            dummy_idx;
    vtss_fdma_ch_t dummy_afi_ch = AFI_CH_AUTO;

    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS flags;

    FDMA_USER_DCB(&dummy_dcb)->frm_ptr = (u8 *)frm_ptr;

    FDMA_INTERRUPT_DISABLE(flags);
    sw_dcb = afi_find_frame(vstate, &dummy_afi_ch, &dummy_dcb, &dummy_idx);
    FDMA_INTERRUPT_RESTORE(flags);

    if (sw_dcb == NULL) {
        return VTSS_RC_ERROR;
    }

    // We can't do frame counting when using AFI.
    *frm_cnt = AFI_FRM_CNT(FDMA_USER_DCB(sw_dcb));
    return VTSS_RC_OK;
}
#endif /* defined(VTSS_FEATURE_AFI_FDMA) && VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vcoreiii_fdma_xtr_ch_from_list()
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_xtr_ch_from_list(vtss_state_t *const vstate, const vtss_fdma_list_t *const list, vtss_fdma_ch_t *const xtr_ch)
{
    fdma_hw_dcb_t *hw_dcb;

    FDMA_ASSERT(NORMAL, list && xtr_ch, return VTSS_RC_ERROR;);
    hw_dcb = (fdma_hw_dcb_t *)list->hw_dcb;
    *xtr_ch = (hw_dcb->sar >> FDMA_F_SAR_CH_ID_FPOS) & VTSS_BITMASK(FDMA_F_SAR_CH_ID_FLEN);

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_LUTON26) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vcoreiii_fdma_xtr_hdr_decode()
// We gotta take into account that the first
// VTSS_FDMA_HDR_SIZE_BYTES - VTSS_FDMA_XTR_HDR_SIZE_BYTES bytes of list->data
// are not used.
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_xtr_hdr_decode(vtss_state_t *const vstate, const vtss_fdma_list_t *const list, vtss_fdma_xtr_props_t *const xtr_props)
{
    u64          ifh;
    u8           signature;
    u8           *xtr_hdr;
    u32          sflow_id;

    FDMA_ASSERT(NORMAL, list != NULL && xtr_props != NULL && list->data != NULL && (list->act_len - (list->frm_ptr - list->ifh_ptr)) > VTSS_FDMA_XTR_HDR_SIZE_BYTES, return VTSS_RC_ERROR;);
    xtr_hdr = list->ifh_ptr;
    ifh = ((u64)xtr_hdr[0] << 56) | ((u64)xtr_hdr[1] << 48) | ((u64)xtr_hdr[2] << 40) | ((u64)xtr_hdr[3] << 32) |
          ((u64)xtr_hdr[4] << 24) | ((u64)xtr_hdr[5] << 16) | ((u64)xtr_hdr[6] <<  8) | ((u64)xtr_hdr[7] <<  0);

    // Signature must be 0xFF, otherwise it's not a valid extraction header.
    signature                      = VTSS_EXTRACT_BITFIELD64(ifh, 56,  8);
    FDMA_ASSERT(NORMAL, signature == 0xFF, return VTSS_RC_ERROR;);

    memset(xtr_props, 0, sizeof(*xtr_props));

    xtr_props->src_port            = VTSS_EXTRACT_BITFIELD64(ifh, 51,  5);
    xtr_props->src_port_decoded    = TRUE;

    xtr_props->chip_no             = 0;
    xtr_props->chip_no_decoded     = TRUE;

    xtr_props->dscp                = VTSS_EXTRACT_BITFIELD64(ifh, 45,  6);
    xtr_props->dscp_decoded        = TRUE;

    xtr_props->acl_idx             = VTSS_EXTRACT_BITFIELD64(ifh, 37,  8);
    xtr_props->acl_idx_decoded     = TRUE;

    // sflow_id:
    // 0-26 : Frame has been SFlow sampled by a Tx sampler on port given by @sflow_id.
    // 27   : Frame has been SFlow sampled by an Rx sampler on port given by @src_port.
    // 28-30: Reserved.
    // 31   : Frame has not been SFlow sampled.
    sflow_id = VTSS_EXTRACT_BITFIELD64(ifh, 32,  5);
    xtr_props->sflow_type          = sflow_id == 27 ? 0 : 1; // 0 => Rx, 1 => Tx.
    xtr_props->sflow_type_decoded  = sflow_id <= 28;         // TRUE if Rx or Tx. False if not an SFlow sample frame.

    xtr_props->sflow_port          = sflow_id == 27 ? xtr_props->src_port : sflow_id;
    xtr_props->sflow_port_decoded  = xtr_props->sflow_type_decoded;

    xtr_props->acl_hit             = VTSS_EXTRACT_BITFIELD64(ifh, 31,  1);
    xtr_props->acl_hit_decoded     = TRUE;

    xtr_props->dp                  = VTSS_EXTRACT_BITFIELD64(ifh, 30,  1);
    xtr_props->dp_decoded          = TRUE;

    xtr_props->lrn_flags           = VTSS_EXTRACT_BITFIELD64(ifh, 28,  2);
    xtr_props->lrn_flags_decoded   = TRUE;

    xtr_props->xtr_qu_mask         = VTSS_EXTRACT_BITFIELD64(ifh, 20,  8);
    xtr_props->xtr_qu_mask_decoded = TRUE;

    xtr_props->xtr_qu              = xtr_qu_from_mask(xtr_props->xtr_qu_mask);
    xtr_props->xtr_qu_decoded      = TRUE;

    xtr_props->qos_class           = VTSS_EXTRACT_BITFIELD64(ifh, 17,  3);
    xtr_props->qos_class_decoded   = TRUE;

    xtr_props->tag_type            = VTSS_EXTRACT_BITFIELD64(ifh, 16,  1);
    xtr_props->tag_type_decoded    = TRUE;

    xtr_props->pcp                 = VTSS_EXTRACT_BITFIELD64(ifh, 13,  3);
    xtr_props->pcp_decoded         = TRUE;

    xtr_props->dei                 = VTSS_EXTRACT_BITFIELD64(ifh, 12,  1);
    xtr_props->dei_decoded         = TRUE;

    xtr_props->vid                 = VTSS_EXTRACT_BITFIELD64(ifh,  0, 12);
    xtr_props->vid_decoded         = TRUE;

    return VTSS_RC_OK;
}
#endif /* defined(VTSS_ARCH_LUTON26) && VTSS_OPT_FDMA_VER <= 2 */

#if defined(VTSS_ARCH_JAGUAR_1) && VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// vcoreiii_fdma_xtr_hdr_decode()
// We gotta take into account that the first
// VTSS_FDMA_HDR_SIZE_BYTES - VTSS_FDMA_XTR_HDR_SIZE_BYTES bytes of user_dcb->data
// are not used.
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_xtr_hdr_decode(vtss_state_t *const vstate, const vtss_fdma_list_t *const user_dcb, vtss_fdma_xtr_props_t *const xtr_props)
{
    u32                 xtr_qu, fcs, sflow_id, sflow_marked;
//  u64                 fwd_hndl;
    u16                 vstax_hi;
    u64                 vstax_lo;
    u64                 fwd;
    u8                  *xtr_hdr;
    const fdma_sw_dcb_t *sw_dcb = FDMA_SW_DCB(user_dcb);

    FDMA_ASSERT(NORMAL, user_dcb != NULL && xtr_props != NULL && sw_dcb->data != NULL && (user_dcb->act_len - (user_dcb->frm_ptr - sw_dcb->ifh_ptr)) > VTSS_FDMA_XTR_HDR_SIZE_BYTES, return VTSS_RC_ERROR;);
    xtr_hdr = sw_dcb->ifh_ptr;

    // Get the FCS, since it is overwritten by the switch core, and thus contains additional info.
    fcs = vtss_fdma_cmn_fcs_get(sw_dcb);

    xtr_qu   = ((u32)xtr_hdr[ 0] << 24) | ((u32)xtr_hdr[ 1] << 16) | ((u32)xtr_hdr[ 2] <<  8) | ((u32)xtr_hdr[ 3] <<  0);
//  fwd_hndl = ((u64)xtr_hdr[ 4] << 56) | ((u64)xtr_hdr[ 5] << 48) | ((u64)xtr_hdr[ 6] << 40) | ((u64)xtr_hdr[ 7] << 32) |
//             ((u64)xtr_hdr[ 8] << 24) | ((u64)xtr_hdr[ 9] << 16) | ((u64)xtr_hdr[10] <<  8) | ((u64)xtr_hdr[11] <<  0);
    vstax_hi =                                                       ((u16)xtr_hdr[12] <<  8) | ((u16)xtr_hdr[13] <<  0);
    vstax_lo = ((u64)xtr_hdr[14] << 56) | ((u64)xtr_hdr[15] << 48) | ((u64)xtr_hdr[16] << 40) | ((u64)xtr_hdr[17] << 32) |
               ((u64)xtr_hdr[18] << 24) | ((u64)xtr_hdr[19] << 16) | ((u64)xtr_hdr[20] <<  8) | ((u64)xtr_hdr[21] <<  0);
    fwd      =                                                       ((u64)xtr_hdr[22] << 40) | ((u64)xtr_hdr[23] << 32) |
               ((u64)xtr_hdr[24] << 24) | ((u64)xtr_hdr[25] << 16) | ((u64)xtr_hdr[26] <<  8) | ((u64)xtr_hdr[27] <<  0);

    // The VStaX header's MSbit must be 1.
    FDMA_ASSERT(NORMAL, VTSS_EXTRACT_BITFIELD(vstax_hi, 15, 1), return VTSS_RC_ERROR;);

    memset(xtr_props, 0, sizeof(*xtr_props));

#if defined(VTSS_FEATURE_VSTAX)
    xtr_props->contains_stack_hdr = VTSS_EXTRACT_BITFIELD64(fwd, 45, 1); // vstax_avail == 1 if received with stack header.
    // Copy the VStaX header as is into the extraction properties.
    // The first two bytes are for the Ether type
    memcpy(xtr_props->stack_hdr_bin + 2, &xtr_hdr[12], VTSS_VSTAX_HDR_SIZE - 2);

    // For compatibility, also set the EtherType to VTSS_ETYPE_VTSS
    xtr_props->stack_hdr_bin[0] = (VTSS_ETYPE_VTSS >> 8) & 0xFF;
    xtr_props->stack_hdr_bin[1] = (VTSS_ETYPE_VTSS >> 0) & 0xFF;
#endif /* VTSS_FEATURE_VSTAX */

    sflow_marked = VTSS_EXTRACT_BITFIELD64(fwd, 40,  1);
    if (!sflow_marked) {
        // Get the timestamp from the FCS.
        xtr_props->tstamp         = fcs;
        xtr_props->tstamp_decoded = TRUE;
    }

    xtr_props->src_port                 = VTSS_EXTRACT_BITFIELD64(fwd, 29,  6);
    xtr_props->src_port_decoded         = TRUE;

    xtr_props->dscp                     = VTSS_EXTRACT_BITFIELD64(fwd,  8,  6);
    xtr_props->dscp_decoded             = TRUE;

    sflow_id = (fcs >> 26) & 0x3F; // Indicates port number.
    xtr_props->sflow_type               = xtr_props->src_port == sflow_id ? 0 : 1; // 0 => Rx, 1 => Tx. So if src port is the same as sflow ID, then it's Rx.
    xtr_props->sflow_type_decoded       = sflow_marked;                            // TRUE indicates that this is sent to CPU due to SFlow-sampling.

    xtr_props->sflow_port               = sflow_id; // Both Rx and Tx SFlow if ANA_AC:PS_COMMON:PS_COMMON_CFG.SFLOW_SMPL_ID_IN_STAMP_ENA == 1.
    xtr_props->sflow_port_decoded       = xtr_props->sflow_type_decoded;

    xtr_props->sflow_fwd_cnt            = (fcs >> 8) & 0x3FFFF; // 18 LSbits of frame forward count. If ANA_AC:PS_COMMON:PS_COMMON_CFG.SFLOW_SMPL_ID_IN_STAMP_ENA == 0, you can get 6 bits more from the sflow_id.
    xtr_props->sflow_fwd_cnt_decoded    = xtr_props->sflow_type_decoded;

    xtr_props->sflow_sample_cnt         = (fcs >> 0) & 0xFF;
    xtr_props->sflow_sample_cnt_decoded = xtr_props->sflow_type_decoded;

    // In dual-chip solutions, we save the chip number in the MSByte of the xtr_qu.
    xtr_props->chip_no                  = xtr_qu >> 24;
    xtr_props->chip_no_decoded          = TRUE;
    xtr_qu &= 0x00FFFFFF;

    xtr_props->xtr_qu                   = xtr_qu;
    xtr_props->xtr_qu_decoded           = TRUE;

    if (VTSS_EXTRACT_BITFIELD64(fwd, 27, 1)) {
        // If classification_result[9] == 1, classification_result[7:0] == xtr_qu_mask for 8 non-super-priority queues.
        xtr_props->xtr_qu_mask = VTSS_EXTRACT_BITFIELD64(fwd, 18, 8);
    } else {
        // Either hit a non-default VCAP IS2 rule, or received on a super-priority queue.
        // Either way, construct the qu mask.
        xtr_props->xtr_qu_mask = 1 << xtr_qu;

        // We can also set the sflow queue in the queue mask.
        if (sflow_marked && vstate->rx_conf.map.sflow_queue != VTSS_PACKET_RX_QUEUE_NONE) {
            xtr_props->xtr_qu_mask |= (1 << vstate->rx_conf.map.sflow_queue);
        }

        // Hit a non-default VCAP IS2 rule.
        xtr_props->acl_hit         = TRUE;
        xtr_props->acl_idx         = VTSS_EXTRACT_BITFIELD64(fwd, 18, 9);
        xtr_props->acl_idx_decoded = TRUE;
    }
    xtr_props->acl_hit_decoded          = TRUE;
    xtr_props->xtr_qu_mask_decoded      = TRUE;

    xtr_props->dp                       = VTSS_EXTRACT_BITFIELD64(vstax_lo, 60,  2);
    xtr_props->dp_decoded               = TRUE;

    xtr_props->qos_class                = VTSS_EXTRACT_BITFIELD64(vstax_lo, 56,  3);
    xtr_props->qos_class_decoded        = TRUE;

    xtr_props->tag_type                 = VTSS_EXTRACT_BITFIELD64(vstax_lo, 14,  1);
    xtr_props->tag_type_decoded         = TRUE;

    xtr_props->pcp                      = VTSS_EXTRACT_BITFIELD64(vstax_lo, 29,  3);
    xtr_props->pcp_decoded              = TRUE;

    xtr_props->dei                      = VTSS_EXTRACT_BITFIELD64(vstax_lo, 28,  1);
    xtr_props->dei_decoded              = TRUE;

    xtr_props->vid                      = VTSS_EXTRACT_BITFIELD64(vstax_lo, 16, 12);
    xtr_props->vid_decoded              = TRUE;

    return VTSS_RC_OK;
}
#endif /* defined(VTSS_ARCH_JAGUAR_1) && VTSS_OPT_FDMA_VER <= 2 */

/*****************************************************************************/
// vcoreiii_fdma_stats_clr()
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_stats_clr(vtss_state_t *const vstate)
{
    fdma_state_t *state = &vstate->fdma_state;
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS flags;

    FDMA_INTERRUPT_DISABLE(flags);
    memset(&state->fdma_stats, 0, sizeof(state->fdma_stats));
    FDMA_INTERRUPT_RESTORE(flags);

    return VTSS_RC_OK;
}

/*****************************************************************************/
// vcoreiii_debug_print()
/*****************************************************************************/
static vtss_rc vcoreiii_debug_print(vtss_state_t *const vstate, const vtss_debug_printf_t pr, const vtss_debug_info_t *const info)
{
    fdma_state_t *state = &vstate->fdma_state;
    int ch, qu;
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS __attribute__((__unused__)) flags;

    if (vstate->chip_no == 0) {
        // Same statistics for chip #0 and #1. Only print once.
        pr("Interrupts: %u\n\n", state->fdma_stats.intr_cnt);
        pr("Ch Usage DCBs Used  DCBs Added Packets    Bytes          Interrupts\n");
        pr("-- ----- ---------- ---------- ---------- -------------- ----------\n");
        for (ch = 0; ch < VTSS_FDMA_CH_CNT; ch++) {
            vtss_fdma_ch_usage_t usage = state->fdma_ch_state[ch].usage;
            pr("%2d %-5s %10u %10u %10u %14llu %10u\n",
               ch,
               usage == VTSS_FDMA_CH_USAGE_UNUSED ? "Free" : usage == VTSS_FDMA_CH_USAGE_XTR ? "Xtr" : usage == VTSS_FDMA_CH_USAGE_INJ ? "Inj" :
#if defined(VTSS_FEATURE_AFI_FDMA)
               usage == VTSS_FDMA_CH_USAGE_CCM ? "AFI" :
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
               "UNKNOWN",
               state->fdma_stats.dcbs_used[ch],
               state->fdma_stats.dcbs_added[ch],
               state->fdma_stats.packets[ch],
               state->fdma_stats.bytes[ch],
               state->fdma_stats.ch_intr_cnt[ch]);
        }
    }

    pr("\n");
    pr("Xtr Qu Packets    Drops\n");
    pr("------ ---------- ----------\n");
    for (qu = 0; qu < VTSS_PACKET_RX_QUEUE_CNT; qu++) {
        pr("%6d %10u %10u\n", qu, state->fdma_stats.xtr_qu_packets[qu][vstate->chip_no], state->fdma_stats.xtr_qu_drops[qu][vstate->chip_no]);
    }


#if defined(VTSS_FEATURE_AFI_FDMA)
    if (vstate->chip_no == 0) {
        // Same statistics for chip #0 and #1. Only print once.
        // Print the DCB layout of the frames that are currently being injected.
        pr("\n");
        for (ch = 0; ch < VTSS_FDMA_CH_CNT; ch++) {
            fdma_sw_dcb_t *dcb;
            int              dcb_cnt = 0;
            if (state->fdma_ch_state[ch].usage == VTSS_FDMA_CH_USAGE_CCM) {
                fdma_ch_state_t *ch_state = &state->fdma_ch_state[ch];
                pr("AFI Channel #%d. Timer Period = (%u + 1) * %u ps\n", ch, ch_state->afi_state.cur.timer_reload_value, state->ahb_clk_period_ps);
                pr("Frames on this channel: %u\n", ch_state->afi_state.user_frames.frm_cnt);
                print_bw_consumption(pr, ch_state->afi_state.user_frames.bw_consumption_bytes_per_second);

                if (info->full) {
                    pr("Frames/sec Frame Ptr  Length DMAC              Dest. Port Counting Numbering\n");
                    pr("---------- ---------- ------ ----------------- ---------- -------- ---------\n");

                    if (ch_state->afi_state.cur.head == NULL) {
                        pr("<none>\n\n");
                        continue;
                    }

                    // We have to disable interrupts while looking into the current chain of DCBs,
                    // so this command should be used with care.
                    FDMA_INTERRUPT_DISABLE(flags);
                    #define MAX_OUTPUT 50
                    dcb = ch_state->afi_state.cur.head;
                    do {
                        dcb_cnt++;
                        if (AFI_DCB_IS_DUMMY(dcb) && dcb_cnt < MAX_OUTPUT) {
                            // Dummy DCB
                            pr("Dummy\n");
                        } else if (dcb_cnt < MAX_OUTPUT) {
                            u8 *frm = FDMA_USER_DCB(dcb)->frm_ptr;
                            u8 *inj_hdr = dcb->ifh_ptr;
                            u32 dst_ports, p, len;
#if defined(VTSS_ARCH_LUTON26)
                            u64 ifh = ((u64)inj_hdr[0] << 56) | ((u64)inj_hdr[1] << 48) | ((u64)inj_hdr[2] << 40) | ((u64)inj_hdr[3] << 32) |
                                      ((u64)inj_hdr[4] << 24) | ((u64)inj_hdr[5] << 16) | ((u64)inj_hdr[6] <<  8) | ((u64)inj_hdr[7] <<  0);
                            dst_ports = VTSS_EXTRACT_BITFIELD64(ifh, 32, 26);
#elif defined(VTSS_ARCH_JAGUAR_1)
                            u64 fwd = ((u64)inj_hdr[18] << 40) | ((u64)inj_hdr[19] << 32) |
                                      ((u64)inj_hdr[20] << 24) | ((u64)inj_hdr[21] << 16) |
                                      ((u64)inj_hdr[22] <<  8) | ((u64)inj_hdr[23] <<  0);
                            BOOL unicast = VTSS_EXTRACT_BITFIELD64(fwd, 16, 2) == 2; // INJECT_UNICAST == 2, INJECT_MULTICAST == 3
                            u64 dst = ((u64)inj_hdr[0] << 56) | ((u64)inj_hdr[1] << 48) | ((u64)inj_hdr[2] << 40) | ((u64)inj_hdr[3] << 32) |
                                      ((u64)inj_hdr[4] << 24) | ((u64)inj_hdr[5] << 16) | ((u64)inj_hdr[6] <<  8) | ((u64)inj_hdr[7] <<  0);
                            if (unicast) {
                                dst_ports = 1 << VTSS_EXTRACT_BITFIELD64(dst, 0, 8);
                            } else {
                                dst_ports = VTSS_EXTRACT_BITFIELD64(dst, 0, 32);
                            }
#else
#error "Unknown architecture"
#endif /* defined(VTSS_ARCH_xxx) */
                            for (p = 0; p < 32; p++) {
                                if (dst_ports & (1 << p)) {
                                    // We only support unicast AFI
                                    break;
                                }
                            }
                            len = FDMA_USER_DCB(dcb)->act_len;
#if VTSS_OPT_FDMA_VER <= 2
                            // act_len includes IFH and FCS. Exclude them before printing.
                            len -= (VTSS_FDMA_HDR_SIZE_BYTES + 4);
#endif
                            pr("%10u 0x%08x %6u %02x-%02x-%02x-%02x-%02x-%02x %10u %-8s %s\n", AFI_FPS_GET(dcb), (u32)frm, len, frm[0], frm[1], frm[2], frm[3], frm[4], frm[5], p, AFI_COUNTING_ENABLED(dcb) ? "Enabled" : "Disabled", AFI_SEQ_NUM_ENABLED(dcb) ? "Enabled" : "Disabled");
                        }
                        dcb = FDMA_SW_DCB(FDMA_USER_DCB(dcb)->next);
                    } while (dcb != ch_state->afi_state.cur.head);

                    FDMA_INTERRUPT_RESTORE(flags);
                    if (dcb_cnt >= MAX_OUTPUT) {
                        pr("...\n");
                    }
                    pr("\nDCB count = %d\n\n", dcb_cnt);
                } else {
                    pr("Use 'debug api fdma full' to see a list of DCBs. WARNING: Using 'full' may cause the AFI counting and sequence updating to fail.\n\n");
                }
            }
        }
    }
#else
    pr("\n");
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
    return VTSS_RC_OK;
}

/*****************************************************************************/
// vcoreiii_fdma_irq_handler()
/*****************************************************************************/
static vtss_rc vcoreiii_fdma_irq_handler(vtss_state_t *const vstate, void *const cntxt)
{
    fdma_state_t   *state           = &vstate->fdma_state;
    u32            tfr_done_cause   = VCOREIII_RD(vstate, VTSS_FDMA_INTR_STATUS_TFR);   // Both AFI, injection, and extraction channels use transfer done interrupts.
    u32            block_done_cause = VCOREIII_RD(vstate, VTSS_FDMA_INTR_STATUS_BLOCK); // The extraction and AFI channels use block done interrupts
    u32            combined_intr_cause;
    vtss_fdma_ch_t ch;
#if defined(VTSS_FEATURE_AFI_FDMA)
    BOOL           afi_ch_enabled[VTSS_FDMA_AFI_CH_MAX];
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    state->fdma_stats.intr_cnt++;

#if defined(VTSS_FEATURE_AFI_FDMA)
    // Gotta loop through *ALL* AFI-enabled channels to see if they are counting-enabled,
    // and if so, check whether the DCB(s) containing the interrupt bit has been processed
    // and if so, clear the bit and update the counters.
    // The reason for this is described in details in Bugzilla#6045.
    for (ch = 0; ch < VTSS_FDMA_AFI_CH_MAX; ch++) {
        // To avoid race conditions, sample whether the channel is enabled once.
        if (state->fdma_ch_state[ch].usage == VTSS_FDMA_CH_USAGE_CCM) {
            afi_ch_enabled[ch] = ch_enabled(vstate, ch);
            afi_irq_update_seq_num_and_counters(state, ch, afi_ch_enabled[ch]);
        } else {
            // Satisfy lint :-(
            afi_ch_enabled[ch] = FALSE;
        }
    }
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

    // We first handle all the block interrupts. These are:
    //  * The per-DCB extraction interrupts.
    //  * AFI has completed a roundtrip of the circular list of frames.
    // For the extraction interrupts, we need to clear the interrupts before iterating
    // through the DCBs. If we didn't do so, we might end up in a situation where a
    // frame arrives after we've looped through the DCBs, but before we clear the
    // interrupts, and the frame would be stuck in RAM until the next frame arrives.
    // About the same argumentation holds for injection interrupts.
    clear_pending_block_done_intr(vstate, block_done_cause);

    // The transfer done interrupts occur both when one frame has been injected and
    // in the (rare?) case where the OS cannot take off frames in the rate that the
    // FDMA extracts them to RAM, causing the DCB list to exhaust and the channel
    // to stop.
    clear_pending_tfr_done_intr(vstate, tfr_done_cause);

    VTSS_OS_REORDER_BARRIER();

    combined_intr_cause = block_done_cause | tfr_done_cause;

    for (ch = 0; ch < VTSS_FDMA_CH_CNT; ch++) {
        if (VTSS_BIT(ch) & combined_intr_cause) {
            state->fdma_stats.ch_intr_cnt[ch]++;
            switch (state->fdma_ch_state[ch].usage) {
            case VTSS_FDMA_CH_USAGE_XTR:
                // For extraction, we first check block done, the transfer done.
                if (block_done_cause & VTSS_BIT(ch)) {
                    if (xtr_irq_block_done(vstate, cntxt, ch)) {
                        // xtr_irq_block_done() returns non-zero if the last DCB was consumed by the FDMA.
                        // This should cause us to restart the channel, so we "fake" a transfer done
                        // interrupt causing us to enter xtr_irq_tfr_done() below.
                        tfr_done_cause |= VTSS_BIT(ch);
                    }
                    if (tfr_done_cause & VTSS_BIT(ch)) {
                        xtr_irq_tfr_done(vstate, ch);
                    }
                }
                break;

            case VTSS_FDMA_CH_USAGE_INJ:
                // Injection can only generate transfer done interrupts.
                FDMA_ASSERT(IRQ, (block_done_cause & VTSS_BIT(ch)) == 0, return VTSS_RC_ERROR;);
                inj_irq_tfr_done(vstate, cntxt, ch);
                break;

#if defined(VTSS_FEATURE_AFI_FDMA)
            case VTSS_FDMA_CH_USAGE_CCM:
                // In theory we could have lived of a block done interrupt, but due to the missing
                // stickyness of this status bit for AFI channels (bug in GPDMA), we also rely
                // on transfer done interrupts. Whichever comes first, we check for channel enabledness
                // in order to really determine end-of-transfer.
                FDMA_ASSERT(IRQ, ch < VTSS_FDMA_AFI_CH_MAX, return VTSS_RC_ERROR;); // Satisfy Lint.
                if (afi_ch_enabled[ch] == FALSE) {
                    afi_irq_tfr_done(vstate, cntxt, ch);
                }
                break;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */

            default:
                // Spurious interrupt.
                /*lint -e(506) */ // Avoid "Constant value Boolean"
                FDMA_ASSERT(IRQ, FALSE, return VTSS_RC_ERROR;);
            }
        }
    }

#if defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX)
    // Finally, in dual-chip solutions, we need to check for frames in the secondary chip.
    if (vstate->chip_count == 2) {
        u32 xtr_grp_data_present;
        (void)vstate->init_conf.reg_read(1, VTSS_DEVCPU_QS_XTR_XTR_DATA_PRESENT, &xtr_grp_data_present);
        xtr_grp_data_present = VTSS_X_DEVCPU_QS_XTR_XTR_DATA_PRESENT_DATA_PRESENT_GRP(xtr_grp_data_present);
        for (ch = 0; xtr_grp_data_present != 0 && ch < VTSS_FDMA_CH_CNT; ch++) {
            if (state->fdma_ch_state[ch].chip_no == 1 && state->fdma_ch_state[ch].usage == VTSS_FDMA_CH_USAGE_XTR && (VTSS_BIT(state->fdma_ch_state[ch].xtr_grp) & xtr_grp_data_present) != 0) {
                // Found a "channel" serving this extraction group.
                state->fdma_stats.ch_intr_cnt[ch]++;
                xtr_irq_secondary(vstate, cntxt, ch);
            }
        }
    }
#endif /* defined(VTSS_ARCH_JAGUAR_1) && defined(VTSS_FEATURE_VSTAX) */
    return VTSS_RC_OK;
}

/*****************************************************************************/
// vcoreiii_fdma_func_init()
// Initialize current global state's function pointers.
// This is the only real public function.
/*****************************************************************************/
void vcoreiii_fdma_func_init(vtss_state_t *vtss_state)
{
    fdma_func_t *func = &vtss_state->fdma_state.fdma_func;

    // Internal functions
    func->fdma_init_conf_set      = vcoreiii_fdma_init_conf_set;
    func->fdma_xtr_qu_suspend_set = vcoreiii_fdma_xtr_qu_suspend_set;
    func->fdma_xtr_dcb_init       = vcoreiii_fdma_xtr_dcb_init;
    func->fdma_xtr_connect        = vcoreiii_fdma_xtr_connect;
    func->fdma_xtr_restart_ch     = vcoreiii_fdma_xtr_restart_ch;
    func->fdma_inj_restart_ch     = vcoreiii_fdma_inj_restart_ch;
#if VTSS_OPT_FDMA_VER >= 3
    func->fdma_start_ch           = vcoreiii_fdma_start_ch;
#endif /* VTSS_OPT_FDMA_VER >= 3 */

    // External functions
    func->fdma_uninit             = vcoreiii_fdma_uninit;
    func->fdma_stats_clr          = vcoreiii_fdma_stats_clr;
    func->fdma_debug_print        = vcoreiii_debug_print;
    func->fdma_irq_handler        = vcoreiii_fdma_irq_handler;
#if VTSS_OPT_FDMA_VER >= 3
    func->fdma_tx                 = vcoreiii_fdma_tx;
#if defined(VTSS_FEATURE_AFI_FDMA)
    func->fdma_afi_frm_cnt        = vcoreiii_fdma_afi_frm_cnt;
#endif /* defined(VTSS_FEATURE_AFI_FDMA) */
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
    // Older functions
    func->fdma_inj                = vcoreiii_fdma_inj;
    func->fdma_xtr_ch_from_list   = vcoreiii_fdma_xtr_ch_from_list;
    func->fdma_xtr_hdr_decode     = vcoreiii_fdma_xtr_hdr_decode;
    func->fdma_ch_cfg             = vcoreiii_fdma_ch_cfg;
#endif /* VTSS_OPT_FDMA_VER <= 2 */
}

#endif /* VTSS_OPT_VCORE_III && VTSS_OPT_FDMA */

/*****************************************************************************/
//
// End of file
//
//****************************************************************************/
