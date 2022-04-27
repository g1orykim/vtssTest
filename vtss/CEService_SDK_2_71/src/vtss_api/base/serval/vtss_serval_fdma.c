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

#define VTSS_IOREG(t, o)  (VTSS_IOADDR(t, o))

#define VTSS_TRACE_LAYER VTSS_TRACE_LAYER_CIL

// Avoid "vtss_api.h not used in module vtss_serval_fdma.c"
/*lint --e{766} */
#include "vtss_api.h"
#if defined(VTSS_OPT_VCORE_III) && VTSS_OPT_FDMA && defined(VTSS_ARCH_SERVAL)
#include "../ail/vtss_state.h"
#include "../ail/vtss_fdma.h"
#include "../ail/vtss_fdma_common.h"
#include "../ail/vtss_common.h" /* For vtss_cmn_XXX() */
#if defined(VTSS_OPSYS_LINUX)
#include "vtss_serval_regs.h"
#endif  /* VTSS_OPSYS_LINUX */

/* Use relative addresses for registers - this is a mess! */
// Avoid Lint Warning 572: Excessive shift value (precision 1 shifted right by 2), which occurs
// in this file because (t) - VTSS_IO_ORIGIN1_OFFSET == 0 for t = VTSS_TO_CFG (i.e. ICPU_CFG), and 0 >> 2 gives a lint warning.
#undef VTSS_IOADDR
#undef VTSS_IOREG
#undef VTSS_IO_OFFSET1
#undef VTSS_IO_OFFSET2
#define VTSS_IOADDR(t, o) /*lint -save -e572 */((((t) - VTSS_IO_ORIGIN1_OFFSET) >> 2) + (o)) /*lint -restore */
#define VTSS_IOREG(t,o)  (VTSS_IOADDR(t,o))
#define VTSS_IO_OFFSET1(offset) (VTSS_IO_ORIGIN1_OFFSET + offset)
#define VTSS_IO_OFFSET2(offset) (VTSS_IO_ORIGIN2_OFFSET + offset)

#define VTSS_FDMA_XTR_HDR_SIZE_BYTES 16 // Number of bytes in the extraction header.
#define VTSS_FDMA_INJ_HDR_SIZE_BYTES 20 // Number of bytes in the injection header (16) + 4 for a VLAN tag (is @switch_frm == TRUE)

// We don't operate with assertions in the API, so we print an error and gracefully return.
// Note: This macro is sometimes used with code that MUST be executed - even on non-debug builds -,
// so if you don't like to have VTSS_EG() calls, then rewrite the macro to avoid that call.
#define FDMA_ASSERT(grp, expr, code) do {if (!(expr)) {VTSS_EG((VTSS_TRACE_GROUP_FDMA_ ## grp), "Assert failed: " # expr); code}} while (0);

#define FDMA_INLINE inline

// A field of hw_dcb->datal is free for S/W to use. This field is not part of the
// register list, and so we have to define it ourselves.
// For extraction we use it for the channel number.
#define VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_DATAL_SW(x)  VTSS_ENCODE_BITFIELD(x, 24, 8)
#define VTSS_M_ICPU_CFG_FDMA_FDMA_DCB_DATAL_SW     VTSS_ENCODE_BITMASK(24, 8)
#define VTSS_X_ICPU_CFG_FDMA_FDMA_DCB_DATAL_SW(x)  VTSS_EXTRACT_BITFIELD(x, 24, 8)

/*****************************************************************************/
//
// REGISTER ACCESS
//
//****************************************************************************/
#define SRVL_WRM_SET(vstate, p, mask)         SRVL_WRM(vstate, p, mask, mask)
#define SRVL_WRM_CLR(vstate, p, mask)         SRVL_WRM(vstate, p, 0,    mask)
#define SRVL_WRM_CTL(vstate, p, _cond_, mask) SRVL_WRM(vstate, p, (_cond_) ? mask : 0, mask)

// Decode register bit field
#define SRVL_BF(name, value) ((VTSS_F_##name & value) ? 1 : 0)

#define VTSS_FDMA_CH_INJ 2

/*****************************************************************************/
// Read target register using current CPU interface
/*****************************************************************************/
static FDMA_INLINE u32 SRVL_RD(vtss_state_t *const vstate, u32 reg)
{
  u32 value;
  FDMA_ASSERT(IRQ, vstate->init_conf.reg_read(0, reg, &value) == VTSS_RC_OK, return 0;);
  return value;
}

/*****************************************************************************/
// Write target register using current CPU interface
/*****************************************************************************/
static FDMA_INLINE void SRVL_WR(vtss_state_t *const vstate, u32 reg, u32 value)
{
  FDMA_ASSERT(IRQ, vstate->init_conf.reg_write(0, reg, value) == VTSS_RC_OK, ;);
}

/*****************************************************************************/
// Read-modify-write target register using current CPU interface
/*****************************************************************************/
static FDMA_INLINE void SRVL_WRM(vtss_state_t *const vstate, u32 reg, u32 value, u32 mask)
{
    u32 val = SRVL_RD(vstate, reg);
    val = ((val & ~mask) | (value & mask));
    SRVL_WR(vstate, reg, val);
}

/*****************************************************************************/
//
// PRIVATE FUNCTIONS
//
//****************************************************************************/

/*****************************************************************************/
// disable_chs()
/*****************************************************************************/
static FDMA_INLINE void disable_chs(vtss_state_t *const vstate, u32 ch_mask, BOOL graceful)
{
    if (graceful) {
      u32 val;
      SRVL_WR(vstate, VTSS_ICPU_CFG_FDMA_FDMA_CH_DISABLE, ch_mask);
      do {
        val = SRVL_RD(vstate, VTSS_ICPU_CFG_FDMA_FDMA_CH_DISABLE);
      } while (val != 0);
    } else {
      SRVL_WR(vstate, VTSS_ICPU_CFG_FDMA_FDMA_CH_FORCEDIS, ch_mask);
    }
}

/*****************************************************************************/
// enable_llp_frm_glbl_interrupts()
/*****************************************************************************/
static void enable_llp_frm_glbl_interrupts(vtss_state_t *const vstate, u32 ch_mask)
{
    // Clear and enable FRM interrupts
    SRVL_WR( vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_FRM, ch_mask);
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_FRM_ENA, ch_mask, ch_mask);

    // Clear and enable LLP interrupts
    SRVL_WR( vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_LLP, ch_mask);
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_LLP_ENA, ch_mask, ch_mask);

    // Enable global interrupts from DMA controller
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_ENA, ch_mask, ch_mask);
}

/*****************************************************************************/
// enable_ch()
/*****************************************************************************/
static void enable_ch(vtss_state_t *const vstate, u32 ch)
{
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_CH_ACTIVATE, VTSS_BIT(ch), VTSS_BIT(ch));
}

/*****************************************************************************/
// serval_fdma_xtr_dcb_init()
/*****************************************************************************/
static vtss_rc serval_fdma_xtr_dcb_init(vtss_state_t *const vstate, vtss_fdma_ch_t xtr_ch, fdma_sw_dcb_t *list, u32 cfg_alloc_len)
{
    fdma_hw_dcb_t *hw_dcb = FDMA_HW_DCB(list);

    // Initialize the DCB area
    hw_dcb->datap = CPU_TO_BUS(VTSS_OS_VIRT_TO_PHYS(list->ifh_ptr));
    hw_dcb->datal = CPU_TO_BUS((VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_DATAL_SW(xtr_ch)) | VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_DATAL_DATAL(cfg_alloc_len));
    hw_dcb->llp   = CPU_TO_BUS(FDMA_USER_DCB(list)->next ? VTSS_OS_VIRT_TO_PHYS(FDMA_HW_DCB(FDMA_SW_DCB(FDMA_USER_DCB(list)->next))) : 0);
    hw_dcb->stat  = 0;

    VTSS_OS_REORDER_BARRIER();

    // Flush the dcache, so that the DCB gets written to main memory.
    VTSS_OS_DCACHE_FLUSH(hw_dcb, VTSS_FDMA_DCB_SIZE_BYTES);

    return VTSS_RC_OK;
}

/*****************************************************************************/
// serval_fdma_xtr_connect()
// This function must be called with interrupts disabled, so that no context
// switches can occur, since it manipulates list pointers that are used in
// IRQ (DSR) context.
/*****************************************************************************/
static void serval_fdma_xtr_connect(vtss_state_t *const vstate, fdma_hw_dcb_t *tail, fdma_hw_dcb_t *more)
{
    tail->llp = CPU_TO_BUS(VTSS_OS_VIRT_TO_PHYS(more));
    VTSS_OS_REORDER_BARRIER();

    // Flush the dcache, so that the previous tail's DCB's LLP gets written to main memory.
    VTSS_OS_DCACHE_FLUSH(&tail->llp, 4);
}

/*****************************************************************************/
// serval_fdma_xtr_restart_ch()
// We need to feed the channel with a new list of DCBs (if any).
// This function must be called with interrupts disabled.
/*****************************************************************************/
static BOOL serval_fdma_xtr_restart_ch(vtss_state_t *const vstate, vtss_fdma_ch_t xtr_ch)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[xtr_ch];
    u32             val;

    if (SRVL_RD(vstate, VTSS_ICPU_CFG_FDMA_FDMA_DCB_LLP(xtr_ch)) != 0) {
        // The FDMA is not yet done.
        return FALSE;
    }

    // Pause the channel gracefully
    SRVL_WR(vstate, VTSS_ICPU_CFG_FDMA_FDMA_CH_DISABLE, VTSS_BIT(xtr_ch));
    do {
        val = SRVL_RD(vstate, VTSS_ICPU_CFG_FDMA_FDMA_CH_SAFE);
    } while ((val & VTSS_BIT(xtr_ch)) == 0);

    // Feed the free_head into the H/W.
    SRVL_WR(vstate, VTSS_ICPU_CFG_FDMA_FDMA_DCB_LLP(xtr_ch), VTSS_OS_VIRT_TO_PHYS(FDMA_HW_DCB(ch_state->free_head)));

    // Re-activate the channel.
    enable_ch(vstate, xtr_ch);

    return TRUE;
}

/*****************************************************************************/
// xtr_start_ch()
/*****************************************************************************/
static void xtr_start_ch(vtss_state_t *const vstate, vtss_fdma_ch_t xtr_ch)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[xtr_ch];

    // Configure the extraction group for FDMA-based extraction (default by API-setup is register-based if running on internal CPU, VRAP if running on external).
    SRVL_WRM(vstate, VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(ch_state->xtr_grp), VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE(2), VTSS_M_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE);

    // Hand the DCB list to the channel
    SRVL_WR(vstate, VTSS_ICPU_CFG_FDMA_FDMA_DCB_LLP(xtr_ch), VTSS_OS_VIRT_TO_PHYS(ch_state->cur_head));

    // Set the channel priority
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_CH_CFG(xtr_ch), VTSS_F_ICPU_CFG_FDMA_FDMA_CH_CFG_CH_PRIO(ch_state->prio), VTSS_M_ICPU_CFG_FDMA_FDMA_CH_CFG_CH_PRIO);

    // Clear and enable LLP, FRM and global interrupts.
    enable_llp_frm_glbl_interrupts(vstate, VTSS_BIT(xtr_ch));

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
// Call this function per channel.
/*****************************************************************************/
static vtss_rc xtr_cfg(vtss_state_t *const vstate, const vtss_fdma_ch_t xtr_ch, const vtss_fdma_ch_cfg_t *const cfg)
{
    int              i;
    fdma_state_t     *state    = &vstate->fdma_state;
    fdma_ch_state_t  *ch_state = &state->fdma_ch_state[xtr_ch];

    // Check parameters and the current status
    FDMA_ASSERT(NORMAL, xtr_ch < VTSS_PACKET_RX_GRP_CNT,                return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, cfg->chip_no == 0,                              return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, cfg->prio >= 0 && cfg->prio < VTSS_FDMA_CH_CNT, return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, cfg->list,                                      return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, cfg->xtr_cb,                                    return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, ch_state->status == FDMA_CH_STATUS_DISABLED,    return VTSS_RC_INV_STATE;);

    // Check that no other non-inactive, extraction channel is managing the xtr_grp.
    for (i = 0; i < VTSS_FDMA_CH_CNT; i++) {
        fdma_ch_state_t *ch_state_i = &state->fdma_ch_state[i];
        FDMA_ASSERT(NORMAL, ch_state_i->status   == FDMA_CH_STATUS_DISABLED ||
                            ch_state_i->usage    != VTSS_FDMA_CH_USAGE_XTR  ||
                            ch_state_i->xtr_grp  != xtr_ch                  ||
                            ch_state_i->chip_no  != cfg->chip_no,
                            return VTSS_RC_INV_STATE;);
    }

    ch_state->usage     = VTSS_FDMA_CH_USAGE_XTR;
    ch_state->status    = FDMA_CH_STATUS_ENABLED;
    ch_state->cur_head  = cfg->list;
    ch_state->free_head = NULL; // No un-committed items.
    ch_state->cur_tail  = NULL;
    ch_state->pend_head = NULL;
    ch_state->xtr_grp   = xtr_ch; // One-to-one correspondence between exxtraction group and channnel number.
    ch_state->prio      = cfg->prio;
    ch_state->xtr_cb    = cfg->xtr_cb;

    // Initialize the list's DCB areas.
    VTSS_RC(vtss_fdma_cmn_dcb_release(vstate, xtr_ch, cfg->list, TRUE /* pretend it's thread safe, since we know it's not started yet */, FALSE /* don't connect */, FALSE /* don't start channel */));

    // Start the channel
    xtr_start_ch(vstate, xtr_ch);

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/*****************************************************************************/
// inj_start_ch()
/*****************************************************************************/
static void inj_start_ch(vtss_state_t *const vstate, vtss_fdma_ch_t inj_ch)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[inj_ch];

    // According to datasheet, injection buffer watermark must be set to 13.
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_GCFG, VTSS_F_ICPU_CFG_FDMA_FDMA_GCFG_INJ_RF_WM(0xD), VTSS_M_ICPU_CFG_FDMA_FDMA_GCFG_INJ_RF_WM);

    // Configure the injection group for FDMA-based injection (default by API-setup is register-based if running on internal CPU, VRAP if running on external).
    SRVL_WRM(vstate, VTSS_DEVCPU_QS_INJ_INJ_GRP_CFG(ch_state->inj_grp), VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE(2), VTSS_M_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE);

    // Set the channel priority
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_CH_CFG(inj_ch), VTSS_F_ICPU_CFG_FDMA_FDMA_CH_CFG_CH_PRIO(ch_state->prio), VTSS_M_ICPU_CFG_FDMA_FDMA_CH_CFG_CH_PRIO);

    // Clear and enable LLP, FRM and global interrupts.
    enable_llp_frm_glbl_interrupts(vstate, VTSS_BIT(inj_ch));
}

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_cfg()
/*****************************************************************************/
static vtss_rc inj_cfg(vtss_state_t *const vstate, const vtss_fdma_ch_t inj_ch, const vtss_fdma_ch_cfg_t *const cfg)
{
    fdma_state_t         *state    = &vstate->fdma_state;
    fdma_ch_state_t      *ch_state = &state->fdma_ch_state[inj_ch];

    // Check current status
    FDMA_ASSERT(NORMAL, state->fdma_ch_state[inj_ch].status == FDMA_CH_STATUS_DISABLED,                   return VTSS_RC_INV_STATE;);
    FDMA_ASSERT(NORMAL, cfg->prio >= 0 && cfg->prio < VTSS_FDMA_CH_CNT,                                   return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, inj_ch == 2 || inj_ch == 3,                                                       return VTSS_RC_ERROR;);

    ch_state->usage        = cfg->usage;
    ch_state->status       = FDMA_CH_STATUS_ENABLED;
    ch_state->prio         = cfg->prio;
    ch_state->cur_head     = NULL;
    ch_state->cur_tail     = NULL;
    ch_state->pend_head    = NULL;

    ch_state->inj_grp = inj_ch - 2; // On Serval there is a one-to-one mapping between injection channel and injection group.

    inj_start_ch(vstate, inj_ch);

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// serval_fdma_start_ch()
/*****************************************************************************/
static void serval_fdma_start_ch(vtss_state_t *const vstate, const vtss_fdma_ch_t ch)
{
    if (vstate->fdma_state.fdma_ch_state[ch].usage == VTSS_FDMA_CH_USAGE_XTR) {
        xtr_start_ch(vstate, ch);
    } else {
        inj_start_ch(vstate, ch);
    }
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

/*****************************************************************************/
// disable_and_clear_chs()
/*****************************************************************************/
static void disable_and_clear_chs(vtss_state_t *const vstate, u32 ch_mask, BOOL graceful)
{
    // Disable all channels
    disable_chs(vstate, ch_mask, graceful);

    // Disable global interrupts from DMA controller
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_ENA, ~ch_mask, ch_mask);

    // Disable and clear SIG interrupts
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_SIG_ENA, ~ch_mask, ch_mask);
    SRVL_WR( vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_SIG, ch_mask);

    // Disable and clear FRM interrupts
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_FRM_ENA, ~ch_mask, ch_mask);
    SRVL_WR( vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_FRM, ch_mask);

    // Disable and clear LLP interrupts
    SRVL_WRM(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_LLP_ENA, ~ch_mask, ch_mask);
    SRVL_WR( vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_LLP, ch_mask);
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

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_oam_type_to_ifh()
/*****************************************************************************/
static FDMA_INLINE vtss_rc inj_oam_type_to_ifh(vtss_packet_oam_type_t oam_type, u32 *result)
{
    vtss_rc rc = VTSS_RC_OK;

    switch (oam_type) {
    case VTSS_PACKET_OAM_TYPE_NONE:
        *result = 0;
        break;
    case VTSS_PACKET_OAM_TYPE_CCM:
        *result = 2;
        break;
    case VTSS_PACKET_OAM_TYPE_CCM_LM:
        *result = 3;
        break;
    case VTSS_PACKET_OAM_TYPE_LBM:
        *result = 4;
        break;
    case VTSS_PACKET_OAM_TYPE_LBR:
        *result = 5;
        break;
    case VTSS_PACKET_OAM_TYPE_LMM:
        *result = 6;
        break;
    case VTSS_PACKET_OAM_TYPE_LMR:
        *result = 7;
        break;
    case VTSS_PACKET_OAM_TYPE_DMM:
        *result = 8;
        break;
    case VTSS_PACKET_OAM_TYPE_DMR:
        *result = 9;
        break;
    case VTSS_PACKET_OAM_TYPE_1DM:
        *result = 10;
        break;
    case VTSS_PACKET_OAM_TYPE_LTM:
        *result = 11;
        break;
    case VTSS_PACKET_OAM_TYPE_LTR:
        *result = 12;
        break;
    case VTSS_PACKET_OAM_TYPE_GENERIC:
        *result = 13;
        break;
    default:
        /*lint -e(506) */ // Avoid Lint Warning: "Constant value Boolean"
        FDMA_ASSERT(NORMAL, FALSE, ;);
        rc = VTSS_RC_ERROR;
        break;
    }
    *result = (*result << 3) | 4;
    return rc;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_ptp_action_to_ifh()
/*****************************************************************************/
static FDMA_INLINE vtss_rc inj_ptp_action_to_ifh(vtss_packet_ptp_action_t ptp_action, u8 ptp_id, u32 *result)
{
    vtss_rc rc = VTSS_RC_OK;

    switch (ptp_action) {
    case VTSS_PACKET_PTP_ACTION_NONE:
        *result = 0;
        break;
    case VTSS_PACKET_PTP_ACTION_ONE_STEP:
        *result = 2; // Residence compensation must be done in S/W, so bit 5 is not set.
        break;
    case VTSS_PACKET_PTP_ACTION_TWO_STEP:
        *result = (3 | (ptp_id << 3)); // Select a PTP timestamp identifier for two-step PTP.
        break;
    case VTSS_PACKET_PTP_ACTION_ORIGIN_TIMESTAMP:
        *result = 5;
        break;
    default:
        /*lint -e(506) */ // Avoid Lint Warning: "Constant value Boolean"
        FDMA_ASSERT(NORMAL, FALSE, ;);
        rc = VTSS_RC_ERROR;
        break;
    }
    return rc;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_ifh_init()
// Frame layout under different conditions:
// On call:
//   [20 bytes of uninitialized free space][6 bytes DMAC][6 bytes SMAC][EtherType][Payload]
//
// Temporary variable. We insert a tag if requested to with #tpid while not switching frame, or if switching frame unless (masquerading && VID == VTSS_VID_NULL).
//   InsertTag = (!switch_frm && tpid != 0) || (switch_frm && !masquerading) || (switch_frm && masquerading && vlan != 0)
//
// On exit when InsertTag == FALSE:
//   [4 bytes of uninitialized free space][16 bytes injection header][6 bytes DMAC][6 bytes SMAC][EtherType][Payload]
//   That is, ifh_offset is set to skip the first 4 bytes.
//
// On exit when InsertTag == TRUE:
//   [0 bytes of uninitialized free space][16 bytes injection header][6 bytes DMAC][6 bytes SMAC][4 bytes VLAN Tag][EtherType][Payload]
//   That is, ifh_offset is set to skip 0 bytes and DMAC and SMAC are moved 4 bytes ahead while inserting a VLAN tag.
/*****************************************************************************/
static FDMA_INLINE vtss_rc inj_ifh_init(vtss_state_t *const vstate, u8 *const data_ptr, const vtss_fdma_inj_props_t *const props, u64 port_mask, u32 *ifh_size, u32 *ifh_offset, u32 afi_id)
{
    u64        inj_hdr[2];
    u32        offset_into_data = VTSS_FDMA_HDR_SIZE_BYTES - VTSS_FDMA_INJ_HDR_SIZE_BYTES; // Take into account any asymmetry between INJ and XTR headers.
    vtss_vid_t vid = props->vlan & 0xFFF;

    *ifh_offset = offset_into_data;
    *ifh_size = 16;

    if (!props->dont_touch_ifh) {
        u8   qos_class = props->qos_class >= 8 ? 7 : props->qos_class;
        BOOL insert_tag = (
            (!props->switch_frm && props->tpid)                                 ||
            ( props->switch_frm && (props->masquerade_port == VTSS_PORT_NO_NONE ||
                                   (props->masquerade_port != VTSS_PORT_NO_NONE && vid != 0))));
        if (!insert_tag) {
            *ifh_offset += 4;
        }

        inj_hdr[0] = VTSS_ENCODE_BITFIELD64(!props->switch_frm, 127 - 64, 1); // BYPASS
        inj_hdr[1] = 0;

        if (props->switch_frm) {
            if (props->masquerade_port != VTSS_PORT_NO_NONE) {
                u32 chip_port;
                FDMA_ASSERT(NORMAL, props->masquerade_port < vstate->port_count, return VTSS_RC_ERROR;);
                chip_port = VTSS_CHIP_PORT_FROM_STATE(vstate, props->masquerade_port);
                inj_hdr[0] |= VTSS_ENCODE_BITFIELD64(1,         126 - 64,  1); // Use masquerade port.
                inj_hdr[0] |= VTSS_ENCODE_BITFIELD64(chip_port, 122 - 64,  4); // Port number.
            }

            if (props->masquerade_port == VTSS_PORT_NO_NONE || vid != VTSS_VID_NULL) {
                // If sending the frame switched (unless masquerading and not requested to insert a VLAN tag),
                // we insert a VLAN tag in the packet by moving DMAC and SMAC four bytes ahead.
                // When doing that, the frame gets classified to the VLAN that the user wants, but on egress it
                // still contains the tag, so we have to remove it by setting POP_CNT to 1.
                inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(1, 28 - 0, 2);

                // PCP determines the QoS Class. A one-to-one mapping is set-up in vtss_serval.c::srvl_init_conf_set(). No matter value of CFI-value, the DP level will become 0.
                inj_insert_vlan_tag(data_ptr + VTSS_FDMA_HDR_SIZE_BYTES - 4, 0x8100,  (qos_class << 13) | vid);
            }
        } else {
            // Not sending through analyzer.
            u32 rew_op = 0, rew_val = 0, pop_cnt = 3 /* Disable rewriter */;

            if (props->tpid) {
                inj_insert_vlan_tag(data_ptr + VTSS_FDMA_HDR_SIZE_BYTES - 4, props->tpid,  (qos_class << 13) | vid);
            }

            if (props->ptp_action != VTSS_PACKET_PTP_ACTION_NONE) {
                if (inj_ptp_action_to_ifh(props->ptp_action, props->ptp_id, &rew_op) != VTSS_RC_OK) {
                    return VTSS_RC_ERROR;
                }
                rew_val = props->ptp_timestamp;
                pop_cnt = 0; // Don't disable rewriter.
            } else if (props->oam_type != VTSS_PACKET_OAM_TYPE_NONE) {
                if (inj_oam_type_to_ifh(props->oam_type, &rew_op) != VTSS_RC_OK) {
                    return VTSS_RC_ERROR;
                }
                pop_cnt = 0; // Don't disable rewriter.
            }

#if defined(VTSS_FEATURE_AFI_SWC)
            if (props->ptp_action == VTSS_PACKET_PTP_ACTION_NONE && afi_id != VTSS_AFI_ID_NONE) {
                vtss_phys_port_no_t  chip_port = VTSS_OS_CTZ((u32)port_mask);
                vtss_port_no_t       port_no;
                vtss_afi_slot_conf_t *slot_conf = &vstate->afi_slots[afi_id];
                u64 val;

                // On FDMA v2, we have to do the housekeeping of vstate->afi_slots ourselves,
                // since we don't call vstate->packet.tx_hdr_encode(), which will otherwise to it.

                FDMA_ASSERT(NORMAL, chip_port < vstate->port_count, return VTSS_RC_ERROR;);
                // Convert chip port to a logical port.
                port_no = vtss_cmn_chip_to_logical_port(vstate, 0, chip_port);
                FDMA_ASSERT(NORMAL, port_no != VTSS_PORT_NO_NONE, return VTSS_RC_ERROR;);

                slot_conf->state   = VTSS_AFI_SLOT_STATE_ENABLED;
                slot_conf->port_no = port_no;

                val = slot_conf->timer_idx + 1;
                inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(val, 37 - 0, 4);
                rew_val = afi_id;
            }
#endif

            inj_hdr[0] |= VTSS_ENCODE_BITFIELD64(rew_op,         118 - 64,  9); // REW_OP
            inj_hdr[0] |= VTSS_ENCODE_BITFIELD64(rew_val,         86 - 64, 32); // REW_VAL
            inj_hdr[0] |= VTSS_ENCODE_BITFIELD64(port_mask >> 7,  64 - 64,  4); // Don't send to the CPU port (hence length == 4 and not 6)
            inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(port_mask,       57 -  0,  7);
            if (props->isdx != VTSS_ISDX_NONE) {
                inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(props->isdx,   47 -  0, 10); // ISDX
                inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(1,             32 -  0,  1); // Use ISDX
            }
            inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(props->dp,       30 -  0,  1); // Drop Precedence.
            inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(pop_cnt,         28 -  0,  2); // POP_CNT
            inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(qos_class,       17 -  0,  3); // QOS Class.
            inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(props->pcp,      13 -  0,  3); // PCP.
            inj_hdr[1] |= VTSS_ENCODE_BITFIELD64(props->vlan,      0 -  0, 13); // Encompasses DEI, and VID. Ethertype can be specified in ifh[16], but we don't support that currently, so it'll always become 0x8100.
        }

        // Time to store it in DDR SDRAM.
        data_ptr[*ifh_offset +  0] = inj_hdr[0] >> 56;
        data_ptr[*ifh_offset +  1] = inj_hdr[0] >> 48;
        data_ptr[*ifh_offset +  2] = inj_hdr[0] >> 40;
        data_ptr[*ifh_offset +  3] = inj_hdr[0] >> 32;
        data_ptr[*ifh_offset +  4] = inj_hdr[0] >> 24;
        data_ptr[*ifh_offset +  5] = inj_hdr[0] >> 16;
        data_ptr[*ifh_offset +  6] = inj_hdr[0] >>  8;
        data_ptr[*ifh_offset +  7] = inj_hdr[0] >>  0;
        data_ptr[*ifh_offset +  8] = inj_hdr[1] >> 56;
        data_ptr[*ifh_offset +  9] = inj_hdr[1] >> 48;
        data_ptr[*ifh_offset + 10] = inj_hdr[1] >> 40;
        data_ptr[*ifh_offset + 11] = inj_hdr[1] >> 32;
        data_ptr[*ifh_offset + 12] = inj_hdr[1] >> 24;
        data_ptr[*ifh_offset + 13] = inj_hdr[1] >> 16;
        data_ptr[*ifh_offset + 14] = inj_hdr[1] >>  8;
        data_ptr[*ifh_offset + 15] = inj_hdr[1] >>  0;

        VTSS_IG(VTSS_TRACE_GROUP_FDMA_NORMAL, "IFH:");
        VTSS_IG_HEX(VTSS_TRACE_GROUP_FDMA_NORMAL, &data_ptr[*ifh_offset], 16);
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// inj_ifh_init()
/*****************************************************************************/
static FDMA_INLINE vtss_rc inj_ifh_init(vtss_state_t *const vstate, fdma_sw_dcb_t *sw_dcb, vtss_packet_tx_info_t *const tx_info, u32 *frm_size)
{
    vtss_fdma_list_t *user_dcb = FDMA_USER_DCB(sw_dcb), *user_dcb_iter;
    u32              bytes_stripped_on_egress = 0, actual_length_bytes = 0;
    u32              bin_hdr_len = 16 /* VTSS_FDMA_INJ_HDR_SIZE_BYTES */;
    BOOL             insert_tpid_tag, insert_masq_tag, insert_clas_tag;

    // Frame pointer must be equal to the IFH pointer by now.
    FDMA_ASSERT(NORMAL, sw_dcb->ifh_ptr == user_dcb->frm_ptr, return VTSS_RC_ERROR;);

    insert_tpid_tag = (!tx_info->switch_frm && tx_info->tag.tpid != 0);
    insert_masq_tag = ( tx_info->switch_frm && tx_info->masquerade_port != VTSS_PORT_NO_NONE && tx_info->tag.vid != 0);
    insert_clas_tag = ( tx_info->switch_frm && tx_info->masquerade_port == VTSS_PORT_NO_NONE);

    if (insert_tpid_tag || insert_masq_tag || insert_clas_tag) {
        vtss_vlan_tag_t tag;
        if (insert_masq_tag || insert_clas_tag) {
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
        VTSS_RC(vtss_fdma_cmn_insert_vlan_tag(sw_dcb, insert_masq_tag || insert_clas_tag ? &tag : &tx_info->tag, FALSE /* No need to move IFH data because we haven't yet constructed it */, FALSE /* Called with interrupts enabled */));
        bytes_stripped_on_egress += insert_clas_tag ? 4 : 0; // TPID and tags on masquerade ports are not stripped on egress. Only tags that get a switched frame classified to the correct VLAN are.
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

    VTSS_RC(vstate->packet.tx_hdr_encode(vstate, tx_info, sw_dcb->ifh_ptr, &bin_hdr_len));

    // Check that we haven't inserted too much data.
    FDMA_ASSERT(NORMAL, sw_dcb->ifh_ptr >= sw_dcb->data, return VTSS_RC_ERROR;);

    VTSS_IG(VTSS_TRACE_GROUP_FDMA_NORMAL, "IFH (%u bytes) + a bit frame data (%u bytes incl. FCS):", FDMA_USER_DCB(sw_dcb)->frm_ptr - sw_dcb->ifh_ptr, *frm_size);
    VTSS_IG_HEX(VTSS_TRACE_GROUP_FDMA_NORMAL, sw_dcb->ifh_ptr, 64);

    return VTSS_RC_OK;
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
static fdma_sw_dcb_t *inj_dcb_init(fdma_sw_dcb_t *sw_dcb, u32 ifh_offset, u32 ifh_size, int *const whl_frm_sz_bytes, u32 *const dcb_cnt, vtss_fdma_ch_usage_t usage, BOOL need_not_be_cache_line_aligned)
{
    u32               *phys_data_ptr, phys_data_off;
    fdma_hw_dcb_t     *hw_dcb;
    u8                *data_ptr;
    int               frag_sz_bytes;
    int               sof = 1, eof;
    fdma_sw_dcb_t    *tail = sw_dcb;
    vtss_fdma_list_t *user_dcb;

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
            // Due to variable length IFH, we need to add @ifh_offset to the sw_dcb->data before storing it in the DCB
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

        phys_data_ptr = (u32 *)VTSS_OS_VIRT_TO_PHYS(data_ptr);
        phys_data_off = (u32)phys_data_ptr & 0x3;
        hw_dcb = FDMA_HW_DCB(sw_dcb);
        // The H/W DCB should be cache-line-size aligned
        FDMA_ASSERT(NORMAL, need_not_be_cache_line_aligned || ((u32)hw_dcb & (VTSS_OS_DCACHE_LINE_SIZE_BYTES - 1)) == 0, return NULL;);

        hw_dcb->llp   = CPU_TO_BUS(user_dcb->next ? VTSS_OS_VIRT_TO_PHYS(FDMA_HW_DCB(FDMA_SW_DCB(user_dcb->next))) : 0); // NULL
        hw_dcb->datap = CPU_TO_BUS((u32)phys_data_ptr & ~0x3);
        hw_dcb->datal = CPU_TO_BUS(VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_DATAL_DATAL(0xFFFF)); // RBNTBD: Can this be zero?
        hw_dcb->stat  = CPU_TO_BUS(
            VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_BLOCKO(phys_data_off) |
            (sof ? VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_SOF : 0)       |
            (eof ? VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_EOF : 0)       |
            VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_BLOCKL(frag_sz_bytes));

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
static vtss_rc inj_dcb_init(vtss_state_t *const vstate, fdma_sw_dcb_t *sw_dcb, fdma_sw_dcb_t **tail, u32 *dcb_cnt, BOOL need_not_be_cache_line_aligned)
{
    u32               *phys_data_ptr, phys_data_off;
    fdma_hw_dcb_t     *hw_dcb;
    int               frag_sz_bytes;
    int               sof = 1, eof;
    vtss_fdma_list_t *user_dcb;

    *dcb_cnt = 0;
    *tail    = sw_dcb;

    while (sw_dcb) {
        user_dcb = FDMA_USER_DCB(sw_dcb);
        eof      = user_dcb->next == NULL;
        // user_dcb->act_len must hold the size of the actual frame data excluding IFH and FCS.
        frag_sz_bytes = user_dcb->act_len + (user_dcb->frm_ptr - sw_dcb->ifh_ptr) + (eof ? 4 /* FCS */ : 0);

        FDMA_ASSERT(NORMAL, sw_dcb->ifh_ptr, return VTSS_RC_ERROR;);
        FDMA_ASSERT(NORMAL,
          ((frag_sz_bytes >= VTSS_FDMA_MIN_DATA_PER_INJ_SOF_DCB_BYTES && sof == 1)  ||
           (frag_sz_bytes >= VTSS_FDMA_MIN_DATA_PER_NON_SOF_DCB_BYTES && sof == 0)) &&
            frag_sz_bytes <= VTSS_FDMA_MAX_DATA_PER_DCB_BYTES, return VTSS_RC_ERROR;);

        phys_data_ptr = (u32 *)VTSS_OS_VIRT_TO_PHYS(sw_dcb->ifh_ptr);
        phys_data_off = (u32)phys_data_ptr & 0x3;
        hw_dcb = FDMA_HW_DCB(sw_dcb);

        // The H/W DCB should be cache-line-size aligned
        FDMA_ASSERT(NORMAL, need_not_be_cache_line_aligned || ((u32)hw_dcb & (VTSS_OS_DCACHE_LINE_SIZE_BYTES - 1)) == 0, return VTSS_RC_ERROR;);

        hw_dcb->llp   = CPU_TO_BUS(user_dcb->next ? VTSS_OS_VIRT_TO_PHYS(FDMA_HW_DCB(FDMA_SW_DCB(user_dcb->next))) : 0); // NULL
        hw_dcb->datap = CPU_TO_BUS((u32)phys_data_ptr & ~0x3);
        hw_dcb->datal = CPU_TO_BUS(VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_DATAL_DATAL(0xFFFF)); // RBNTBD: Can this be zero?
        hw_dcb->stat  = CPU_TO_BUS(
            VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_BLOCKO(phys_data_off) |
            (sof ? VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_SOF : 0)       |
            (eof ? VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_EOF : 0)       |
            VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_BLOCKL(frag_sz_bytes));

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

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// inj_init()
/*****************************************************************************/
static vtss_rc inj_init(vtss_state_t *const vstate, vtss_fdma_ch_t inj_ch, fdma_sw_dcb_t *sw_dcb, const u32 len, const vtss_fdma_inj_props_t *const props, fdma_sw_dcb_t **new_tail, u32 *dcb_cnt, u32 afi_id)
{
    fdma_ch_state_t *ch_state = &vstate->fdma_state.fdma_ch_state[inj_ch];
    u32             ifh_size = VTSS_FDMA_INJ_HDR_SIZE_BYTES, ifh_offset; // To compensate for variable-length IFHs.
    int             computed_whl_frm_sz_bytes;

    FDMA_ASSERT(NORMAL, len >= 18 && len <= VTSS_FDMA_MAX_FRAME_SIZE_BYTES, return VTSS_RC_ERROR;); // The 18 bytes == 2 * MAC Addr + EtherType + FCS.
    FDMA_ASSERT(NORMAL, ch_state->usage == VTSS_FDMA_CH_USAGE_INJ,          return VTSS_RC_INV_STATE;);
    FDMA_ASSERT(NORMAL, ch_state->status != FDMA_CH_STATUS_DISABLED,        return VTSS_RC_INV_STATE;);

    if (inj_ifh_init(vstate, sw_dcb->data, props, props->port_mask | vstate->fdma_state.mirror_port_mask, &ifh_size, &ifh_offset, afi_id) != VTSS_RC_OK) {
        return VTSS_RC_ERROR;
    }
    *new_tail  = inj_dcb_init(sw_dcb, ifh_offset, ifh_size, &computed_whl_frm_sz_bytes, dcb_cnt, ch_state->usage, FALSE);
#if VTSS_OPT_FDMA_DEBUG
    FDMA_ASSERT(NORMAL, computed_whl_frm_sz_bytes == len + ifh_size, return VTSS_RC_ERROR;);
#endif /* VTSS_OPT_FDMA_DEBUG */

    if (*new_tail == 0) {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/*****************************************************************************/
// inj_ch_init()
/*****************************************************************************/
static FDMA_INLINE void inj_ch_init(vtss_state_t *const vstate, fdma_hw_dcb_t *hw_dcb, vtss_fdma_ch_t inj_ch)
{
    // Hand the DCB list to the channel
    SRVL_WR(vstate, VTSS_ICPU_CFG_FDMA_FDMA_DCB_LLP(inj_ch), VTSS_OS_VIRT_TO_PHYS(hw_dcb));
}

/*****************************************************************************/
// stat_rx_update()
/*****************************************************************************/
static FDMA_INLINE void stat_rx_update(fdma_state_t *state, u32 xtr_qu)
{
    state->fdma_stats.xtr_qu_packets[xtr_qu >= VTSS_PACKET_RX_QUEUE_CNT ? VTSS_PACKET_RX_QUEUE_CNT - 1 : xtr_qu][0]++;
}

/*****************************************************************************/
// xtr_qu_suspend_set()
/*****************************************************************************/
static void serval_fdma_xtr_qu_suspend_set(vtss_state_t *const vstate, vtss_packet_rx_queue_t xtr_qu, BOOL do_suspend)
{
    // On Serval there are two CPU ports (physical #11 and #12). When running only on
    // the internal CPU, we only use #11. However, some applications may
    // want to divide the extraction queues among them and forward some queues to
    // #11 and others to #12, which can then be read by the external CPU.
    // If enabling throttling, the internal CPU will suspend a given queue by
    // moving that queue from port #11 to port #12, which might have some
    // unexpected side-effects on an external CPU reading port #12.

    // First make sure that port #12 is flushing all data it receives.
    SRVL_WRM_SET(vstate, VTSS_DEVCPU_QS_XTR_XTR_FLUSH, VTSS_F_DEVCPU_QS_XTR_XTR_FLUSH_FLUSH(VTSS_BIT(1)));

    // Then either redirect the queue to port #11 (ourselves) or port #12 (suspend)
    SRVL_WRM_CTL(vstate, VTSS_QSYS_SYSTEM_CPU_GROUP_MAP, do_suspend, VTSS_F_QSYS_SYSTEM_CPU_GROUP_MAP_CPU_GROUP_MAP(VTSS_BIT(xtr_qu)));
}

/*****************************************************************************/
// serval_fdma_inj_restart_ch()
/*****************************************************************************/
static vtss_rc serval_fdma_inj_restart_ch(vtss_state_t *const vstate, vtss_fdma_ch_t inj_ch, fdma_sw_dcb_t *head, BOOL do_start, BOOL thread_safe)
{
    if (do_start) {
        // Channel must be inactive
        FDMA_CHECK_RC(thread_safe, (SRVL_RD(vstate, VTSS_ICPU_CFG_FDMA_FDMA_CH_SAFE) & VTSS_BIT(inj_ch)) != 0);

        inj_ch_init(vstate, FDMA_HW_DCB(head), inj_ch);
        enable_ch(vstate, inj_ch);
    }

    return VTSS_RC_OK;
}

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

/*****************************************************************************/
// xtr_irq_frm_done()
/*****************************************************************************/
static FDMA_INLINE void xtr_irq_frm_done(vtss_state_t *const vstate, void *const cntxt, vtss_fdma_ch_t xtr_ch)
{
    int                    total_frm_size_bytes_incl_ifh, frag_size_bytes;
    fdma_ch_state_t        *ch_state;
    fdma_hw_dcb_t          *hw_dcb;
    fdma_state_t           *state = &vstate->fdma_state;
    vtss_fdma_list_t       *new_list, *new_head, *temp_list;
    u32                    stat, xtr_qu, xtr_qu_mask;
    u32                    offset_into_data = VTSS_FDMA_HDR_SIZE_BYTES - VTSS_FDMA_XTR_HDR_SIZE_BYTES;
    BOOL                   sof, eof;
    VTSS_OS_TIMESTAMP_TYPE sw_tstamp;

    VTSS_NG(VTSS_TRACE_GROUP_FDMA_IRQ, "xtr_irq_frm_done(ch = %d)\n", xtr_ch);
    ch_state = &state->fdma_ch_state[xtr_ch];
    FDMA_ASSERT(IRQ, ch_state->status != FDMA_CH_STATUS_DISABLED, return;);

    // Iterate over all DCBs and stop when we reach one with the DONE bit cleared.
    while (ch_state->cur_head) {
        hw_dcb = FDMA_HW_DCB(ch_state->cur_head);

        // Before dereferencing the DCB, we must invalidate the cache line(s).
        VTSS_OS_DCACHE_INVALIDATE(hw_dcb, VTSS_FDMA_DCB_SIZE_BYTES);

        stat = BUS_TO_CPU(hw_dcb->stat);

        // If stat.blockl is 0, we've consumed all the frames, and we're done.
        total_frm_size_bytes_incl_ifh = frag_size_bytes = VTSS_X_ICPU_CFG_FDMA_FDMA_DCB_STAT_BLOCKL(stat);
        if (frag_size_bytes == 0) {
            break;
        }

        state->fdma_stats.dcbs_used[xtr_ch]++;

        VTSS_OS_DCACHE_INVALIDATE(ch_state->cur_head->data + offset_into_data, frag_size_bytes);
        VTSS_OS_REORDER_BARRIER();

        // Currently we cannot handle aborted or pruned frames.
        FDMA_ASSERT(IRQ, (stat & (VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_PD | VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_ABORT)) == 0, VTSS_EG(VTSS_TRACE_GROUP_FDMA_IRQ, "ch=%d, stat=0x%08x", xtr_ch, stat); return;);

        FDMA_USER_DCB(ch_state->cur_head)->act_len = frag_size_bytes;

        sof = (stat & VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_SOF) != 0;
        eof = (stat & VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_EOF) != 0;

        if (sof) {
            ch_state->cur_head->ifh_ptr = ch_state->cur_head->data    + offset_into_data;
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
                FDMA_ASSERT(IRQ, sof, return;);

                // For further, unified processing, we move the current DCB to the pending list.
                ch_state->pend_head = ch_state->cur_head;
            } else {
                // The frame is part of a jumbo frame. In that case, the SOF flag can't be set, because the SOF is
                // already received and moved to the pending list.
                FDMA_ASSERT(IRQ, !sof, return;);

                // Attach the current list item to the end of the pending list while computing the total length.
                temp_list = FDMA_USER_DCB(ch_state->pend_head);
                total_frm_size_bytes_incl_ifh += temp_list->act_len;
                while (temp_list->next) {
                    temp_list = temp_list->next;
                    total_frm_size_bytes_incl_ifh += temp_list->act_len;
                }
                temp_list->next = FDMA_USER_DCB(ch_state->cur_head);
            }

            state->fdma_stats.packets[xtr_ch]++;
            // Don't include IFH in packet byte counter
            state->fdma_stats.bytes[xtr_ch] += (total_frm_size_bytes_incl_ifh - VTSS_FDMA_XTR_HDR_SIZE_BYTES);

            // Find the physical queue that this frame was forwarded onto. This is the most significant bit of
            // the IFH.CPUQ.
            xtr_qu_mask = (((ch_state->pend_head->data[12 + offset_into_data] << 8) | (ch_state->pend_head->data[13 + offset_into_data])) >> 4) & 0xFF;
            xtr_qu = xtr_qu_from_mask(xtr_qu_mask);

            vtss_fdma_cmn_throttle_suspend_check(vstate, xtr_qu, total_frm_size_bytes_incl_ifh - VTSS_FDMA_XTR_HDR_SIZE_BYTES);
            stat_rx_update(state, xtr_qu);
#if VTSS_OPT_FDMA_VER >= 3
            if (vtss_fdma_cmn_xtr_hdr_decode(vstate, ch_state->pend_head, 0, xtr_qu, &sw_tstamp) == VTSS_RC_OK) {
                new_list = state->fdma_cfg.rx_cb(cntxt, FDMA_USER_DCB(ch_state->pend_head));
            } else {
                // Re-cycle
                new_list = FDMA_USER_DCB(ch_state->pend_head);
            }
#else
            FDMA_USER_DCB(ch_state->pend_head)->timestamp = sw_tstamp;
            new_list = ch_state->xtr_cb(cntxt, ch_state->pend_head, xtr_qu + VTSS_PACKET_RX_QUEUE_START);
#endif /* VTSS_OPT_FDMA_VER */

            // NULLify the pending list, since there're no more pending fragments.
            ch_state->pend_head = NULL;

            // Return the DCBs (if any) to the extraction channel.
            (void)vtss_fdma_cmn_dcb_release(vstate, xtr_ch, FDMA_SW_DCB(new_list), TRUE /* thread safe */, TRUE /* connect */, FALSE /* don't restart channel */);
        } else {
            // The EOF is not reached yet, so there wasn't room for the whole frame
            // in the allocated data area of this DCB. Move the DCB to the pending list.

            // Save a reference to the new head.
            new_head = FDMA_USER_DCB(ch_state->cur_head)->next;

            // Clear the next entry, so that we can accommodate the case where the
            // FDMA's DCB list exhausts in the middle of a jumbo frame, causing
            // vtss_fdma_cmn_xtr_restart_ch() to restart the channel with a new list of DCBs.
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
}

/*****************************************************************************/
// inj_irq_tfr_done()
/*****************************************************************************/
static FDMA_INLINE void inj_irq_tfr_done(vtss_state_t *vstate, void *const cntxt, vtss_fdma_ch_t inj_ch)
{
    fdma_state_t     *state = &vstate->fdma_state;
    fdma_ch_state_t  *ch_state = &state->fdma_ch_state[inj_ch];
    vtss_fdma_list_t *sof_l, *eof_l;
    fdma_hw_dcb_t    *hw_dcb;
    fdma_sw_dcb_t    *sw_dcb;

    VTSS_NG(VTSS_TRACE_GROUP_FDMA_IRQ, "inj_irq_tfr_done(ch = %d)\n", inj_ch);

    FDMA_ASSERT(IRQ, inj_ch >= 0 && inj_ch < VTSS_FDMA_CH_CNT,    return;);
    FDMA_ASSERT(IRQ, ch_state->status != FDMA_CH_STATUS_DISABLED, return;);
    FDMA_ASSERT(IRQ, ch_state->usage  == VTSS_FDMA_CH_USAGE_INJ,  return;);
    FDMA_ASSERT(IRQ, ch_state->cur_head,                          return;);
    sof_l = eof_l = FDMA_USER_DCB(ch_state->cur_head);

    // We need to traverse the DCBs until we find a DCB with the dcb_ptr->llp
    // location being NULL, which is the latest injected frame's EOF. We get called
    // once per frame that has been injected, so it cannot happen that the DMA
    // controller goes beyond that point in the list.
    while (1) {
        FDMA_ASSERT(IRQ, eof_l, return;);

        // Check that the DMA controller indeed has injected the current block.
        // This is conveyed through the dcb_ptr->stat.pruned/done bit.
        // Since the DMA controller writes this bit, we must invalidate the cache
        // for that position before reading it. It is the only part of the DCB
        // that the DMA controller writes back, so it's the only part that we need
        // to invalidate.
        hw_dcb = FDMA_HW_DCB(FDMA_SW_DCB(eof_l));

#if VTSS_OPT_FDMA_DEBUG
        VTSS_OS_DCACHE_INVALIDATE(&hw_dcb->stat, 4);
        VTSS_OS_REORDER_BARRIER();
        FDMA_ASSERT(IRQ, BUS_TO_CPU(hw_dcb->stat) & VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_PD, return;);
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
    }

    state->fdma_stats.packets[inj_ch]++;

    sw_dcb = FDMA_SW_DCB(sof_l);

#if defined(VTSS_FEATURE_AFI_SWC)
#if VTSS_OPT_FDMA_VER <= 2
    if (sw_dcb->afi_id != VTSS_AFI_ID_NONE) {
        // Do not report Tx done, since we've just transmitted it into
        // the switch core, and won't release the buffers.
    } else
#else
    if (sw_dcb->afi_ids[0] != VTSS_AFI_ID_NONE) {
        // Do not report Tx done, since we've just transmitted it into
        // the switch core, and won't release the buffers.
        // Instead check, whether we should re-transmit the frame, which is
        // the case if this AFI frame is spread across several AFI slots.
        if (++sw_dcb->afi_idx < VTSS_ARRSZ(sw_dcb->afi_ids) && sw_dcb->afi_ids[sw_dcb->afi_idx] != VTSS_AFI_ID_NONE) {
            fdma_sw_dcb_t *tail;
            u32           timer_idx, dcb_cnt;
            u64           ifh;

            // Need to transmit the frame once more with another AFI ID and timer index.
            vtss_afi_slot_conf_t *slot_conf = &vstate->afi_slots[sw_dcb->afi_ids[sw_dcb->afi_idx]];

            // Unfortunately, we cannot call vstate->packet.tx_hdr_encode(), because we don't have the
            // original tx_info here, and because tx_hdr_encode() takes the VTSS mutex
            // which we can't do from interrupt context.
            FDMA_ASSERT(IRQ, slot_conf->state == VTSS_AFI_SLOT_STATE_RESERVED, return;);

            slot_conf->state = VTSS_AFI_SLOT_STATE_ENABLED;
            // Copy the port number from the very first afi_id, which we know is initialized by now.
            slot_conf->port_no = vstate->afi_slots[sw_dcb->afi_ids[0]].port_no;

            // Time to modify the IFH. This is a dirty hack.

            // First REW_VAL (slot number), which is located at bits 117:86 of the IFH.
            ifh  = (u64)sw_dcb->ifh_ptr[0] << 56;
            ifh |= (u64)sw_dcb->ifh_ptr[1] << 48;
            ifh |= (u64)sw_dcb->ifh_ptr[2] << 40;
            ifh |= (u64)sw_dcb->ifh_ptr[3] << 32;
            ifh |= (u64)sw_dcb->ifh_ptr[4] << 24;
            ifh |= (u64)sw_dcb->ifh_ptr[5] << 16;
            ifh |= (u64)sw_dcb->ifh_ptr[6] <<  8;
            ifh |= (u64)sw_dcb->ifh_ptr[7] <<  0;
            ifh &= ~(((1LLU << 32) - 1) << (86 - 64)); // Clear REW_VAL bits (117:86).
            ifh |= VTSS_ENCODE_BITFIELD64(sw_dcb->afi_ids[sw_dcb->afi_idx], 86 - 64, 32); // REW_VAL
            sw_dcb->ifh_ptr[0] = ifh >> 56;
            sw_dcb->ifh_ptr[1] = ifh >> 48;
            sw_dcb->ifh_ptr[2] = ifh >> 40;
            sw_dcb->ifh_ptr[3] = ifh >> 32;
            sw_dcb->ifh_ptr[4] = ifh >> 24;
            sw_dcb->ifh_ptr[5] = ifh >> 16;
            sw_dcb->ifh_ptr[6] = ifh >>  8;
            sw_dcb->ifh_ptr[7] = ifh >>  0;

            // Then timer index, which is located at bits 40:37 of the IFH.
            // We take a different approach here, since it only spans two bytes.
            sw_dcb->ifh_ptr[10] &= 0xfe; // Clear bit 40.
            sw_dcb->ifh_ptr[11] &= 0x1f; // Clear bits 39:37.
            timer_idx = slot_conf->timer_idx + 1;
            sw_dcb->ifh_ptr[10] |= (timer_idx & 0x8) >> 3; // Set bit 40
            sw_dcb->ifh_ptr[11] |= (timer_idx & 0x7) << 5; // Set bits 39:37

            FDMA_ASSERT(IRQ, inj_dcb_init(vstate, sw_dcb, &tail, &dcb_cnt, FALSE) == VTSS_RC_OK, ;);
            FDMA_ASSERT(IRQ, vtss_fdma_cmn_inj_attach_frm(vstate, sw_dcb, tail, VTSS_FDMA_CH_INJ, 1, sof_l->act_len + 4, NULL, TRUE) == VTSS_RC_OK, ;);
        }
    } else
#endif /* VTSS_OPT_FDMA_VER */
#endif /* defined(VTSS_FEATURE_AFI_SWC) */
    {
#if VTSS_OPT_FDMA_VER <= 2
        FDMA_ASSERT(IRQ, sof_l->inj_post_cb, return;);
        sof_l->inj_post_cb(cntxt, sof_l, inj_ch, FALSE); // Last argument is @dropped, which cannot happen on this architecture.
#else
        vstate->fdma_state.fdma_cfg.tx_done_cb(cntxt, sof_l, VTSS_RC_OK);

        // Release the injection DCBs.
        (void)vtss_fdma_cmn_dcb_release(vstate, 0 /* doesn't matter in FDMA v3+ */, sw_dcb, TRUE /* thread safe */, FALSE /* only used for XTR DCBs */, FALSE /* only used for XTR DCBs */);
#endif /* VTSS_OPT_FDMA_VER */
    }
}

#if defined(VTSS_FEATURE_AFI_SWC)
/*****************************************************************************/
// afi_find_frame()
/*****************************************************************************/
static vtss_afi_id_t afi_find_frame(vtss_state_t *const vstate, const u8 *const frm_ptr)
{
    vtss_afi_id_t slot;

    for (slot = 0; slot < VTSS_ARRSZ(vstate->fdma_state.afi_frames); slot++) {
        fdma_sw_dcb_t *sw_dcb = vstate->fdma_state.afi_frames[slot];
        if (sw_dcb && FDMA_USER_DCB(sw_dcb)->frm_ptr == frm_ptr) {
            return slot;
        }
    }
    return VTSS_AFI_ID_NONE;
}
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_SWC)
/*****************************************************************************/
// afi_unlink()
// Return VTSS_RC_OK if found and removed, otherwise VTSS_RC_ERROR.
/****************************************************************************/
static void afi_unlink(vtss_state_t *const vstate, vtss_afi_id_t afi_id, BOOL check, BOOL callback)
{
    fdma_sw_dcb_t *sw_dcb;
#if VTSS_OPT_FDMA_VER >= 3
    u32           i, afi_ids_left = 0;
    BOOL          found = FALSE;
#endif

    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS flags;

    FDMA_INTERRUPT_DISABLE(flags);

    if ((sw_dcb = vstate->fdma_state.afi_frames[afi_id]) != NULL) {
        vtss_fdma_list_t *user_dcb = FDMA_USER_DCB(sw_dcb);
#if VTSS_OPT_FDMA_VER <= 2
        user_dcb->inj_post_cb(NULL, user_dcb, VTSS_FDMA_CCM_CH_AUTO, FALSE);
#else
        // One single sanity check before starting:
        // It could be that the frame gets cancelled before it is actually re-injected
        // the required number of times. We can check that by looking at the
        // afi_idx and compare it to the afi_ids[] array. We only do it if #check
        // is TRUE, indicating that this is the first time this function is invoked
        // for the user frame.
        if (check) {
            if (sw_dcb->afi_idx < VTSS_ARRSZ(sw_dcb->afi_ids) && sw_dcb->afi_ids[sw_dcb->afi_idx] != VTSS_AFI_ID_NONE) {
                VTSS_EG(VTSS_TRACE_GROUP_FDMA_IRQ, "Unlinking AFI flow that haven't yet been fully started");
                // A possible work-around is to re-enable interrupts and do some busy-wait, but let's see if this
                // is ever going to be a problem.
            }
        }

        // Gotta go through all the AFI IDs we have allocated and remove this one.
        // Only when we get to the very last, we will call-back the application
        // and release the DCBs.
        for (i = 0; i < VTSS_ARRSZ(sw_dcb->afi_ids); i++) {
            if (sw_dcb->afi_ids[i] == afi_id) {
                sw_dcb->afi_ids[i] = VTSS_AFI_ID_NONE;
                found = TRUE;
            } else if (sw_dcb->afi_ids[i] != VTSS_AFI_ID_NONE) {
                afi_ids_left++;
            }
        }

        FDMA_ASSERT(IRQ, found, ;);

        if (callback && !afi_ids_left) {
            vstate->fdma_state.fdma_cfg.afi_done_cb(NULL, user_dcb, VTSS_RC_OK);

            // Release the AFI DCB to the relevant list.
            (void)vtss_fdma_cmn_dcb_release(vstate, 0 /* doesn't matter in FDMA v3+ */, sw_dcb, TRUE /* thread safe */, FALSE /* only used for XTR DCBs */, FALSE /* only used for XTR DCBs */);
        }
#endif /* VTSS_OPT_FDMA_VER */

        vstate->fdma_state.afi_frames[afi_id] = NULL;
    } else {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_IRQ, "No such AFI frame found (afi_id = %u)", afi_id);
    }

    FDMA_INTERRUPT_RESTORE(flags);
}
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_SWC)
/*****************************************************************************/
// afi_alloc()
// Prepare for an AFI frame.
// #fps is an INOUT parameter: On entry, it must hold the requested
// number of frames per second.
// On exit, it holds the actual, achievable number of frames per second,
// which is always <= the requested.
/*****************************************************************************/
static FDMA_INLINE vtss_rc afi_alloc(vtss_state_t *const vstate, fdma_sw_dcb_t *sw_dcb, u32 *fps, vtss_afi_id_t *afi_id)
{
    vtss_afi_frm_dscr_t dscr;

    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS flags;

    // The frame frequency must be non-zero
    FDMA_ASSERT(NORMAL, *fps > 0 && *fps <= VTSS_AFI_FPS_MAX, return VTSS_RC_ERROR;);

    // Ask the API to allocate an AFI slot for this frame. We must go through the official
    // API and not go through cil_func, because we need to take the VTSS mutex.
    memset(&dscr, 0, sizeof(dscr));
    dscr.fps    = *fps;
    VTSS_RC(vtss_afi_alloc(vstate, &dscr, afi_id));
    *fps = dscr.actual_fps; // How many frames per second did we actually get?

    VTSS_IG(VTSS_TRACE_GROUP_FDMA_NORMAL, "AFI: Requested fps = %u, achieved fps = %u", dscr.fps, dscr.actual_fps);

    // Update the S/W state
    FDMA_INTERRUPT_DISABLE(flags);
    vstate->fdma_state.afi_frames[*afi_id] = sw_dcb;
    FDMA_INTERRUPT_RESTORE(flags);

    return VTSS_RC_OK;
}
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

/*****************************************************************************/
// ch_state_defaults()
/*****************************************************************************/
static FDMA_INLINE void ch_state_defaults(vtss_state_t *const vstate)
{
#if VTSS_OPT_FDMA_VER >= 3
    fdma_ch_state_t *ch_state;

    // The FDMA API makes its own channel setup in later FDMA driver versions.
    ch_state = &vstate->fdma_state.fdma_ch_state[0];
    ch_state->usage = VTSS_FDMA_CH_USAGE_XTR;
    ch_state->prio  = 3;

    ch_state = &vstate->fdma_state.fdma_ch_state[VTSS_FDMA_CH_INJ];
    ch_state->usage   = VTSS_FDMA_CH_USAGE_INJ;
    ch_state->prio    = 3;
    ch_state->inj_grp = VTSS_FDMA_CH_INJ - 2;
#endif /* VTSS_OPT_FDMA_VER >= 3 */
}

/*****************************************************************************/
//
// PUBLIC FUNCTIONS
//
//****************************************************************************/

#if defined(VTSS_FEATURE_AFI_SWC)
/*****************************************************************************/
// serval_fdma_afi_pause_resume()
// This is called from the API whenever a port links up or down.
/*****************************************************************************/
static vtss_rc serval_fdma_afi_pause_resume(vtss_state_t *const vstate, vtss_afi_id_t afi_id, BOOL resume)
{
    fdma_sw_dcb_t        *sw_dcb;
    vtss_rc              rc = VTSS_RC_ERROR;
    /*lint --e{529} */ // Avoid "Symbol 'flags' not subsequently referenced" Lint warning
    FDMA_INTERRUPT_FLAGS flags;

    if (!resume) {
        // If pausing, we have nothing to do. The API has already cancelled the frame.
        return VTSS_RC_OK;
    }

    FDMA_ASSERT(NORMAL, afi_id < VTSS_ARRSZ(vstate->fdma_state.afi_frames), return VTSS_RC_ERROR;);

    FDMA_INTERRUPT_DISABLE(flags);

    // If we're going to resume, we gotta reinject the frame.
    if ((sw_dcb = vstate->fdma_state.afi_frames[afi_id]) != NULL) {
        // Gotta clear the DONE bit in the DCB, or the frame will be treated as ABORT by the FDMA.
        fdma_hw_dcb_t *hw_dcb = FDMA_HW_DCB(sw_dcb);
        u32 val = BUS_TO_CPU(hw_dcb->stat);
        val &= ~VTSS_F_ICPU_CFG_FDMA_FDMA_DCB_STAT_PD;
        hw_dcb->stat = CPU_TO_BUS(val);
        VTSS_OS_DCACHE_FLUSH(&hw_dcb->stat, sizeof(hw_dcb->stat));
        rc = vtss_fdma_cmn_inj_attach_frm(vstate, sw_dcb, sw_dcb, VTSS_FDMA_CH_INJ, 0, 0, NULL, TRUE);
    }

    FDMA_INTERRUPT_RESTORE(flags);

    if (rc != VTSS_RC_OK) {
        VTSS_EG(VTSS_TRACE_GROUP_FDMA_NORMAL, "Restart of AFI frame failed");
    }

    return rc;
}
#endif /* VTSS_FEATURE_AFI_SWC */

/*****************************************************************************/
// serval_fdma_init_conf_set()
/*****************************************************************************/
static vtss_rc serval_fdma_init_conf_set(vtss_state_t *const vstate)
{
    fdma_state_t *state = &vstate->fdma_state;

    disable_and_clear_chs(vstate, VTSS_BITMASK(VTSS_FDMA_CH_CNT), FALSE);

    // Create new channel state defaults
    ch_state_defaults(vstate);

    state->hdr_size     = VTSS_FDMA_HDR_SIZE_BYTES;
    state->xtr_hdr_size = VTSS_FDMA_XTR_HDR_SIZE_BYTES;
    state->inj_hdr_size = VTSS_FDMA_INJ_HDR_SIZE_BYTES;
    state->xtr_ch_cnt   = 1;

    return VTSS_RC_OK;
}

/*****************************************************************************/
// serval_fdma_uninit()
/*****************************************************************************/
static vtss_rc serval_fdma_uninit(vtss_state_t *const vstate)
{
    disable_and_clear_chs(vstate, VTSS_BITMASK(VTSS_FDMA_CH_CNT), TRUE);
    return VTSS_RC_OK;
}

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// serval_fdma_ch_cfg()
/*****************************************************************************/
static vtss_rc serval_fdma_ch_cfg(      vtss_state_t       *const vstate,
                                  const vtss_fdma_ch_t     ch,
                                  const vtss_fdma_ch_cfg_t *const cfg)
{
    fdma_state_t *state = &vstate->fdma_state;

    FDMA_ASSERT(NORMAL, cfg != NULL, return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, ch >= 0 && ch < VTSS_FDMA_CH_CNT, return VTSS_RC_ERROR;);

    if (cfg->usage == VTSS_FDMA_CH_USAGE_UNUSED) {
        // User wants to disable this channel.
        disable_and_clear_chs(vstate, VTSS_BIT(ch), TRUE);
        state->fdma_ch_state[ch].usage  = VTSS_FDMA_CH_USAGE_UNUSED;
        state->fdma_ch_state[ch].status = FDMA_CH_STATUS_DISABLED;
    } else if (cfg->usage == VTSS_FDMA_CH_USAGE_XTR) {
        return xtr_cfg(vstate, ch, cfg);
    } else if (cfg->usage == VTSS_FDMA_CH_USAGE_INJ) {
        return inj_cfg(vstate, ch, cfg);
    } else {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// serval_fdma_inj()
// @len: Length of frame in bytes, excluding IFH but including FCS.
/*****************************************************************************/
static vtss_rc serval_fdma_inj(      vtss_state_t          *const vstate,
                                     vtss_fdma_list_t      *user_dcb,
                               const vtss_fdma_ch_t        inj_ch,
                               const u32                   len,
                               const vtss_fdma_inj_props_t *const props)
{
#if defined(VTSS_FEATURE_AFI_SWC)
    vtss_afi_id_t    afi_id = VTSS_AFI_ID_NONE;
#else
    u32              afi_id = 0; /* Anything */
#endif /* defined(VTSS_FEATURE_AFI_SWC) */
    u32              dcb_cnt = 0;
    vtss_fdma_ch_t   the_ch = inj_ch;
    fdma_sw_dcb_t    *new_tail, *sw_dcb = FDMA_SW_DCB(user_dcb);

    FDMA_ASSERT(NORMAL, user_dcb,                                                                      return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, props->chip_no < vstate->chip_count,                                           return VTSS_RC_ERROR;);
#if defined(VTSS_FEATURE_AFI_SWC)
    FDMA_ASSERT(NORMAL, props->switch_frm || props->port_mask != 0 || props->ccm_cancel,               return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, (the_ch >= 0 && the_ch < VTSS_FDMA_CH_CNT) || the_ch == VTSS_FDMA_CCM_CH_AUTO, return VTSS_RC_ERROR;);
#else
    FDMA_ASSERT(NORMAL, props->switch_frm || props->port_mask != 0,                                    return VTSS_RC_ERROR;);
    FDMA_ASSERT(NORMAL, the_ch >= 0 && the_ch < VTSS_FDMA_CH_CNT,                                      return VTSS_RC_ERROR;);
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

#if defined(VTSS_FEATURE_AFI_SWC)
    if (props->ccm_cancel) {
        // Find the AFI frame
        afi_id = afi_find_frame(vstate, FDMA_USER_DCB(sw_dcb)->frm_ptr);
        FDMA_ASSERT(NORMAL, afi_id != VTSS_AFI_ID_NONE, return VTSS_RC_ERROR;);

        // Ask the API to cancel the frame. We have to go through the official API
        // and not go through cil_func, because we need to take the VTSS mutex.
        VTSS_RC(vtss_afi_free(vstate, afi_id));

        // We also need to notify the application and update our own state.
        afi_unlink(vstate, afi_id, TRUE, TRUE); // TRUE => doesn't matter on FDMA v2, TRUE => Always callback user

        return VTSS_RC_OK;
    } else if (props->ccm_fps > 0) {
        u32 ccm_fps = props->ccm_fps;

        // Only one single DCB for AFI flows.
        FDMA_ASSERT(NORMAL, FDMA_USER_DCB(sw_dcb)->next == NULL, return VTSS_RC_ERROR;);

        FDMA_CHECK_RC(FALSE, afi_alloc(vstate, sw_dcb, &ccm_fps, &afi_id) == VTSS_RC_OK);

        the_ch = VTSS_FDMA_CH_INJ;

        // Indicate that this is an AFI frame.
        sw_dcb->afi_id = afi_id;
        // Fall through to the injection stuff.
    } else {
        // Indicate that this is not an AFI frame.
        sw_dcb->afi_id = VTSS_AFI_ID_NONE;
    }
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

    // The remainder takes care of normal injection.
    FDMA_ASSERT(NORMAL, inj_init(vstate, the_ch, sw_dcb, len, props, &new_tail, &dcb_cnt, afi_id) == VTSS_RC_OK, return VTSS_RC_ERROR;);

    return vtss_fdma_cmn_inj_attach_frm(vstate, sw_dcb, new_tail, the_ch, dcb_cnt, len, props, FALSE);
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER >= 3
/*****************************************************************************/
// serval_fdma_tx()
/*****************************************************************************/
static vtss_rc serval_fdma_tx(vtss_state_t          *const vstate,
                              fdma_sw_dcb_t         *sw_dcb,
                              vtss_fdma_tx_info_t   *const fdma_info,
                              vtss_packet_tx_info_t *const tx_info,
                              BOOL                  afi_cancel)
{
    fdma_sw_dcb_t  *tail;
    u32            dcb_cnt, frm_size;

#if defined(VTSS_FEATURE_AFI_SWC)
    if (afi_cancel) {
        // Find the AFI frame
        vtss_afi_id_t afi_id;
        u32           cnt = 0;
        BOOL          first = TRUE;

        while ((afi_id = afi_find_frame(vstate, FDMA_USER_DCB(sw_dcb)->frm_ptr)) != VTSS_AFI_ID_NONE) {
            // Ask the API to cancel the frame. We have to go through the official API
            // and not go through cil_func, because we need to take the VTSS mutex.
            VTSS_RC(vtss_afi_free(vstate, afi_id));

            // We also need to notify the application and update our own state.
            afi_unlink(vstate, afi_id, first, TRUE);
            first = FALSE;

            cnt++;
        }

        FDMA_ASSERT(NORMAL, cnt > 0, return VTSS_RC_ERROR;);

        return VTSS_RC_OK;
    } else if (fdma_info->afi_fps > 0) {
        u32     cnt, fps_desired = fdma_info->afi_fps, fps_actual = 0;
        vtss_rc rc;

        // Only one single DCB for AFI flows.
        FDMA_ASSERT(NORMAL, FDMA_USER_DCB(sw_dcb)->next == NULL, return VTSS_RC_ERROR;);

        for (cnt = 0; cnt < VTSS_ARRSZ(sw_dcb->afi_ids); cnt++) {
            sw_dcb->afi_ids[cnt] = VTSS_AFI_ID_NONE;
        }

        cnt = 0;

        // As long as we have room in our array of AFI IDs and
        // we aren't spot on w.r.t. the number of frames to transmit per
        // second, allocate new AFI IDs.
        while (fps_actual < fps_desired && cnt < VTSS_ARRSZ(sw_dcb->afi_ids)) {
            u32 fps_missing = fps_desired - fps_actual;

            if ((rc = afi_alloc(vstate, sw_dcb, &fps_missing, &sw_dcb->afi_ids[cnt])) != VTSS_RC_OK) {
                // Something went wrong. Clean-up.
                for (cnt = 0; cnt < VTSS_ARRSZ(sw_dcb->afi_ids); cnt++) {
                    if (sw_dcb->afi_ids[cnt] == VTSS_AFI_ID_NONE) {
                        break;
                    }

                    afi_unlink(vstate, sw_dcb->afi_ids[cnt], FALSE, FALSE); // FALSE = don't do additional checks, FALSE => Never callback the user.
                }
                return rc;
            }

            // On exit from afi_alloc(), fps_missing holds the actual, achievable number of frames per second.
            fps_actual += fps_missing;
            cnt++;
        }

        // Tells the FDMA Tx done function how far we are in (re-)transmitting the AFI frames.
        sw_dcb->afi_idx = 0;

        // Tell inj_ifh_init() which afi_id to use for the first instance of transmitting this frame.
        tx_info->afi_id = sw_dcb->afi_ids[0];

        // Fall through to the injection stuff.
    } else {
        // Indicate that this is not an AFI frame.
        sw_dcb->afi_ids[0] = tx_info->afi_id = VTSS_AFI_ID_NONE;
        sw_dcb->afi_idx = 0;
    }
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

    VTSS_RC(inj_ifh_init(vstate, sw_dcb, tx_info, &frm_size));
    VTSS_RC(inj_dcb_init(vstate, sw_dcb, &tail, &dcb_cnt, FALSE));
    return vtss_fdma_cmn_inj_attach_frm(vstate, sw_dcb, tail, VTSS_FDMA_CH_INJ, dcb_cnt, frm_size /* for statistics */, fdma_info, FALSE);
}
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// serval_fdma_xtr_ch_from_list()
/*****************************************************************************/
static vtss_rc serval_fdma_xtr_ch_from_list(vtss_state_t *const vstate, const vtss_fdma_list_t *const list, vtss_fdma_ch_t *const xtr_ch)
{
    fdma_hw_dcb_t *hw_dcb;

    FDMA_ASSERT(NORMAL, list && xtr_ch, return VTSS_RC_ERROR;);
    hw_dcb = (fdma_hw_dcb_t *)list->hw_dcb;
    // We've saved the extraction channel number in a for-s/w-only field in hw_dcb->datal.
    *xtr_ch = VTSS_X_ICPU_CFG_FDMA_FDMA_DCB_DATAL_SW(hw_dcb->datal);
    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

#if VTSS_OPT_FDMA_VER <= 2
/*****************************************************************************/
// serval_fdma_xtr_hdr_decode()
// We gotta take into account that the first
// VTSS_FDMA_HDR_SIZE_BYTES - VTSS_FDMA_XTR_HDR_SIZE_BYTES bytes of list->data
// are not used.
/*****************************************************************************/
static vtss_rc serval_fdma_xtr_hdr_decode(vtss_state_t *const vstate, const vtss_fdma_list_t *const list, vtss_fdma_xtr_props_t *const xtr_props)
{
    u64 ifh[2];
    u8  *xtr_hdr;
    u32 sflow_id;
    u32 rew_op, rew_op_lsb, rew_val;
    int i;

    FDMA_ASSERT(NORMAL, list != NULL && xtr_props != NULL && list->data != NULL && (list->act_len - (list->frm_ptr - list->ifh_ptr)) > VTSS_FDMA_XTR_HDR_SIZE_BYTES, return VTSS_RC_ERROR;);
    xtr_hdr = list->ifh_ptr;
    for (i = 0; i < 2; i++) {
        int offset = 8 * i;
        ifh[i] = ((u64)xtr_hdr[offset + 0] << 56) | ((u64)xtr_hdr[offset + 1] << 48) | ((u64)xtr_hdr[offset + 2] << 40) | ((u64)xtr_hdr[offset + 3] << 32) |
                 ((u64)xtr_hdr[offset + 4] << 24) | ((u64)xtr_hdr[offset + 5] << 16) | ((u64)xtr_hdr[offset + 6] <<  8) | ((u64)xtr_hdr[offset + 7] <<  0);
    }

    memset(xtr_props, 0, sizeof(*xtr_props));

    xtr_props->isdx                = VTSS_EXTRACT_BITFIELD64(ifh[1], 47, 10);
    xtr_props->isdx_decoded        = TRUE;

    xtr_props->src_port            = VTSS_EXTRACT_BITFIELD64(ifh[1], 43, 4);
    xtr_props->src_port_decoded    = TRUE;

    xtr_props->chip_no             = 0;
    xtr_props->chip_no_decoded     = TRUE;

    xtr_props->acl_idx             = VTSS_EXTRACT_BITFIELD64(ifh[1], 37, 6);
    xtr_props->acl_idx_decoded     = TRUE;

    rew_op  = VTSS_EXTRACT_BITFIELD64(ifh[0], 118 - 64,  9);
    rew_val = VTSS_EXTRACT_BITFIELD64(ifh[0],  86 - 64, 32);
    rew_op_lsb = rew_op & 0x7;

    if (rew_op_lsb != 4) {
      // Rx timestamp in rew_val except when REW_OP[2:0] == 4
      xtr_props->tstamp         = rew_val;
      xtr_props->tstamp_decoded = TRUE;
    }

    if (rew_op_lsb == 3) {
        // Two-step PTP Tx timestamp in MSbits of the rew_op field
        xtr_props->tstamp_id         = rew_op >> 3;
        xtr_props->tstamp_id_decoded = TRUE;
    } else if (rew_op_lsb == 4) {
        // REW_VAL contains OAM info that we cannot decode without frame knowledge.
        // As a way out, we just save it in oam_info and let the application decode it itself :-(
        xtr_props->oam_info         = rew_val;
        xtr_props->oam_info_decoded = TRUE;
    }

    // sflow_id:
    // 0-11 : Frame has been sFlow sampled by a Tx sampler on port given by @sflow_id.
    // 12   : Frame has been sFlow sampled by an Rx sampler on port given by @src_port.
    // 13-14: Reserved.
    // 15   : Frame has not been sFlow sampled.
    sflow_id = VTSS_EXTRACT_BITFIELD64(ifh[1], 32, 4);
    xtr_props->sflow_type          = sflow_id == 12 ? 0 : 1; // 0 => Rx, 1 => Tx.
    xtr_props->sflow_type_decoded  = sflow_id < 13;          // TRUE if Rx or Tx. False if not an sFlow sample frame.

    xtr_props->sflow_port          = sflow_id == 12 ? xtr_props->src_port : sflow_id;
    xtr_props->sflow_port_decoded  = xtr_props->sflow_type_decoded;

    xtr_props->acl_hit             = VTSS_EXTRACT_BITFIELD64(ifh[1], 31,  1);
    xtr_props->acl_hit_decoded     = TRUE;

    xtr_props->dp                  = VTSS_EXTRACT_BITFIELD64(ifh[1], 30,  1);
    xtr_props->dp_decoded          = TRUE;

    xtr_props->lrn_flags           = VTSS_EXTRACT_BITFIELD64(ifh[1], 28,  2);
    xtr_props->lrn_flags_decoded   = TRUE;

    xtr_props->xtr_qu_mask         = VTSS_EXTRACT_BITFIELD64(ifh[1], 20,  8);
    xtr_props->xtr_qu_mask_decoded = TRUE;

    xtr_props->xtr_qu              = xtr_qu_from_mask(xtr_props->xtr_qu_mask);
    xtr_props->xtr_qu_decoded      = TRUE;

    xtr_props->qos_class           = VTSS_EXTRACT_BITFIELD64(ifh[1], 17,  3);
    xtr_props->qos_class_decoded   = TRUE;

    xtr_props->tag_type            = VTSS_EXTRACT_BITFIELD64(ifh[1], 16,  1);
    xtr_props->tag_type_decoded    = TRUE;

    xtr_props->pcp                 = VTSS_EXTRACT_BITFIELD64(ifh[1], 13,  3);
    xtr_props->pcp_decoded         = TRUE;

    xtr_props->dei                 = VTSS_EXTRACT_BITFIELD64(ifh[1], 12,  1);
    xtr_props->dei_decoded         = TRUE;

    xtr_props->vid                 = VTSS_EXTRACT_BITFIELD64(ifh[1],  0, 12);
    xtr_props->vid_decoded         = TRUE;

    return VTSS_RC_OK;
}
#endif /* VTSS_OPT_FDMA_VER <= 2 */

/*****************************************************************************/
// serval_fdma_stats_clr()
/*****************************************************************************/
static vtss_rc serval_fdma_stats_clr(vtss_state_t *const vstate)
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
// serval_debug_print()
/*****************************************************************************/
static vtss_rc serval_debug_print(vtss_state_t *const vstate, const vtss_debug_printf_t pr, const vtss_debug_info_t *const info)
{
    fdma_state_t *state = &vstate->fdma_state;
    int          ch, qu;

    pr("Interrupts: %u\n\n", state->fdma_stats.intr_cnt);
    pr("Ch Usage DCBs Used  DCBs Added Packets    Bytes          Interrupts\n");
    pr("-- ----- ---------- ---------- ---------- -------------- ----------\n");
    for (ch = 0; ch < VTSS_FDMA_CH_CNT; ch++) {
        vtss_fdma_ch_usage_t usage = state->fdma_ch_state[ch].usage;
        pr("%2d %-5s %10u %10u %10u %14llu %10u\n",
           ch,
           usage == VTSS_FDMA_CH_USAGE_UNUSED ? "Free" : usage == VTSS_FDMA_CH_USAGE_XTR ? "Xtr" : usage == VTSS_FDMA_CH_USAGE_INJ ? "Inj" : "UNKNOWN",
           state->fdma_stats.dcbs_used[ch],
           state->fdma_stats.dcbs_added[ch],
           state->fdma_stats.packets[ch],
           state->fdma_stats.bytes[ch],
           state->fdma_stats.ch_intr_cnt[ch]);
    }

    pr("\n");
    pr("Xtr Qu Packets\n");
    pr("------ ----------\n");
    for (qu = 0; qu < VTSS_PACKET_RX_QUEUE_CNT; qu++) {
        pr("%6d %10u\n", qu, state->fdma_stats.xtr_qu_packets[qu][0]);
    }

    pr("\n");
    return VTSS_RC_OK;
}

/*****************************************************************************/
// serval_fdma_irq_handler()
/*****************************************************************************/
static vtss_rc serval_fdma_irq_handler(vtss_state_t *const vstate, void *const cntxt)
{
    fdma_state_t   *state           = &vstate->fdma_state;
    u32            intr_ident       = SRVL_RD(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_IDENT); // Which enabled channel is interrupting?
    u32            intr_err         = SRVL_RD(vstate, VTSS_ICPU_CFG_FDMA_FDMA_EVT_ERR);    // Who is signaling error events?
    u32            intr_llp         = SRVL_RD(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_LLP);   // Who is signaling LLP interrupts?
    u32            intr_frm         = SRVL_RD(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_FRM);   // Who is signaling FRM interrupts?
    u32            intr_sig         = SRVL_RD(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_SIG);   // Who is signaling SIG interrupts?
    u32            combined_intr_cause;
    vtss_fdma_ch_t ch;

    state->fdma_stats.intr_cnt++;

    // We don't expect any errors. Write a message and clear them if they occur.
    FDMA_ASSERT(IRQ, (intr_err & intr_ident) == 0, SRVL_WR(vstate, VTSS_ICPU_CFG_FDMA_FDMA_EVT_ERR, intr_err & intr_ident););

    // We also don't expect any SIG interrupts (for now). Write a message and clear them if they occur.
    FDMA_ASSERT(IRQ, (intr_sig & intr_ident) == 0, SRVL_WR(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_SIG, intr_sig & intr_ident););

    // Clear interrupts.
    SRVL_WR(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_LLP, intr_llp & intr_ident);
    SRVL_WR(vstate, VTSS_ICPU_CFG_FDMA_FDMA_INTR_FRM, intr_frm & intr_ident);

    VTSS_OS_REORDER_BARRIER();

    combined_intr_cause = (intr_llp | intr_frm) & intr_ident;

    for (ch = 0; ch < VTSS_FDMA_CH_CNT; ch++) {
        if (VTSS_BIT(ch) & combined_intr_cause) {
            state->fdma_stats.ch_intr_cnt[ch]++;

            switch (state->fdma_ch_state[ch].usage) {
            case VTSS_FDMA_CH_USAGE_XTR:
                // Unfortunately, we don't get FRM interrupts when the very
                // last DCB has been filled with frame data. This means that we
                // can't depend on that interrupt for loading a new list of
                // DCBs to the FDMA.
                // Instead, we have to rely on the LLP interrupt, which fires
                // when the very last DCB of the current chain gets *loaded*.
                // When this event happens, we must feed a new list of DCBs
                // to the channel (if any in ch_state->free_head).
                // The very last DCB of the previous list is already loaded
                // into H/W, and if it receives a frame before we get to
                // load a new LLP pointer into H/W, we will not get a FRM
                // interrupt from the previous list's last DCB.
                // If however, we get to load a new LLP pointer into H/W
                // before the last DCB of the previous list is filled with
                // frame data, we will get a FRM interrupt.
                // To overcome the first situation, we must execute
                // xtr_irq_frm_done() even on LLP interrupts only.
                if (intr_llp & VTSS_BIT(ch)) {
                    vtss_fdma_cmn_xtr_restart_ch(vstate, ch);
                }

                // Whether or not the FRM interrupt is active, we must
                // check for done frames, due to missing FRM interrupt
                // in certain situations (see above).
                xtr_irq_frm_done(vstate, cntxt, ch);
                break;

            case VTSS_FDMA_CH_USAGE_INJ:
                inj_irq_tfr_done(vstate, cntxt, ch);
                break;

            default:
                // Spurious interrupt.
                /*lint -e(506) */ // Avoid "Constant value Boolean"
                FDMA_ASSERT(IRQ, FALSE, return VTSS_RC_ERROR;);
            }
        }
    }

    return VTSS_RC_OK;
}

/*****************************************************************************/
// serval_fdma_func_init()
// Initialize current global state's function pointers.
// This is the only real public function.
/*****************************************************************************/
void serval_fdma_func_init(vtss_state_t *vtss_state)
{
    fdma_func_t *func = &vtss_state->fdma_state.fdma_func;

    // Internal functions
    func->fdma_init_conf_set      = serval_fdma_init_conf_set;
    func->fdma_xtr_qu_suspend_set = serval_fdma_xtr_qu_suspend_set;
    func->fdma_xtr_dcb_init       = serval_fdma_xtr_dcb_init;
    func->fdma_xtr_connect        = serval_fdma_xtr_connect;
    func->fdma_xtr_restart_ch     = serval_fdma_xtr_restart_ch;
    func->fdma_inj_restart_ch     = serval_fdma_inj_restart_ch;
#if VTSS_OPT_FDMA_VER >= 3
    func->fdma_start_ch           = serval_fdma_start_ch;
#endif /* VTSS_OPT_FDMA_VER >= 3 */

    // External functions
    func->fdma_uninit             = serval_fdma_uninit;
    func->fdma_stats_clr          = serval_fdma_stats_clr;
    func->fdma_debug_print        = serval_debug_print;
    func->fdma_irq_handler        = serval_fdma_irq_handler;
#if VTSS_OPT_FDMA_VER >= 3
    func->fdma_tx                 = serval_fdma_tx;
#endif /* VTSS_OPT_FDMA_VER >= 3 */

#if defined(VTSS_FEATURE_AFI_SWC)
    // Function called from API only, to be used when port changes link state. Temporary implementation.
    func->fdma_afi_pause_resume   = serval_fdma_afi_pause_resume;
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

#if VTSS_OPT_FDMA_VER <= 2
    // Older functions
    func->fdma_xtr_ch_from_list   = serval_fdma_xtr_ch_from_list;
    func->fdma_xtr_hdr_decode     = serval_fdma_xtr_hdr_decode;
    func->fdma_ch_cfg             = serval_fdma_ch_cfg;
    func->fdma_inj                = serval_fdma_inj;
#endif /* VTSS_OPT_FDMA_VER <= 2 */
}

#endif /* defined(VTSS_OPT_VCORE_III) && VTSS_OPT_FDMA && defined(VTSS_ARCH_SERVAL) */

/*****************************************************************************/
//
// End of file
//
//****************************************************************************/

