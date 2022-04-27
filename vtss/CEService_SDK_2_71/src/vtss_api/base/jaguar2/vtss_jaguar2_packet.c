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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_PACKET
#include "vtss_jaguar2_cil.h"

// Avoid Lint Warning 572: Excessive shift value (precision 1 shifted right by 2), which occurs
// in this file because (t) - VTSS_IO_ORIGIN1_OFFSET == 0 for t = VTSS_TO_CFG (i.e. ICPU_CFG), and 0 >> 2 gives a lint warning.
/*lint --e{572} */
#if defined(VTSS_ARCH_JAGUAR_2)

/* - CIL functions ------------------------------------------------- */

#if defined(VTSS_FEATURE_AFI_SWC)
/* ================================================================= *
 *  AFI - Automatic Frame Injector
 * ================================================================= */

/*
 * jr2_afi_alloc()
 * Attempts to find a suitable AFI timer and AFI slot for a given frame to be
 * periodically injected.
 */
static vtss_rc jr2_afi_alloc(vtss_state_t *vtss_state, vtss_afi_frm_dscr_t *const dscr, vtss_afi_id_t *const id)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

/*
 * jr2_afi_free()
 * Cancels a periodically injected frame and frees up the
 * resources allocated for it.
 */
static vtss_rc jr2_afi_free(vtss_state_t *vtss_state, vtss_afi_id_t id)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

/* ================================================================= *
 *  NPI
 * ================================================================= */

static vtss_rc jr2_npi_conf_set(vtss_state_t *vtss_state, const vtss_npi_conf_t *const new)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_dma_conf_set(vtss_state_t *vtss_state, const vtss_packet_dma_conf_t *const new)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_dma_offset(struct vtss_state_s *vtss_state, BOOL extract, u32 *const offset)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_rx_conf_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_rx_frame_discard_grp(vtss_state_t *vtss_state, const vtss_packet_rx_grp_t xtr_grp)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_rx_frame_discard(vtss_state_t *vtss_state, const vtss_packet_rx_queue_t queue_no)
{
    vtss_packet_rx_grp_t xtr_grp = vtss_state->packet.rx_conf.grp_map[queue_no];
    return jr2_rx_frame_discard_grp(vtss_state, xtr_grp);
}

static vtss_rc jr2_rx_frame_get(vtss_state_t                 *vtss_state,
                                 const vtss_packet_rx_queue_t queue_no,
                                 vtss_packet_rx_header_t      *const header,
                                 u8                           *const frame,
                                 const u32                    length)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_rx_frame_get_raw(struct vtss_state_s *vtss_state,
                                     u8                  *const data,
                                     const u32           buflen,
                                     u32                 *const ifhlen,
                                     u32                 *const frmlen)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_tx_frame_ifh(vtss_state_t *vtss_state,
                                 const vtss_packet_tx_ifh_t *const ifh,
                                 const u8 *const frame,
                                 const u32 length)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}


static vtss_rc jr2_rx_hdr_decode(const vtss_state_t          *const state,
                                  const vtss_packet_rx_meta_t *const meta,
                                  const u8                           xtr_hdr[VTSS_PACKET_HDR_SIZE_BYTES],
                                        vtss_packet_rx_info_t *const info)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

/*****************************************************************************/
// jr2_tx_hdr_encode()
/*****************************************************************************/
static vtss_rc jr2_tx_hdr_encode(      vtss_state_t          *const state,
                                  const vtss_packet_tx_info_t *const info,
                                        u8                    *const bin_hdr,
                                        u32                   *const bin_hdr_len)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_tx_frame_port(vtss_state_t *vtss_state,
                                  const vtss_port_no_t  port_no,
                                  const u8              *const frame,
                                  const u32             length,
                                  const vtss_vid_t      vid)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

/* - Debug print --------------------------------------------------- */

static vtss_rc jr2_debug_pkt(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

vtss_rc vtss_jr2_packet_debug_print(vtss_state_t *vtss_state,
                                     const vtss_debug_printf_t pr,
                                     const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_PACKET, jr2_debug_pkt, vtss_state, pr, info);
}

#if defined(VTSS_FEATURE_AFI_SWC)
static vtss_rc jr2_debug_afi(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr,
                              const vtss_debug_info_t   *const info)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

vtss_rc vtss_jr2_afi_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_AFI, jr2_debug_afi, vtss_state, pr, info);
}
#endif /* VTSS_FEATURE_AFI_SWC */

/* - Initialization ------------------------------------------------ */

static vtss_rc jr2_packet_init(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

vtss_rc vtss_jr2_packet_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_packet_state_t *state = &vtss_state->packet;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->rx_conf_set      = jr2_rx_conf_set;
        state->rx_frame_get     = jr2_rx_frame_get;
        state->rx_frame_get_raw = jr2_rx_frame_get_raw;
        state->rx_frame_discard = jr2_rx_frame_discard;
        state->tx_frame_port    = jr2_tx_frame_port;
        state->tx_frame_ifh     = jr2_tx_frame_ifh;
        state->rx_hdr_decode    = jr2_rx_hdr_decode;
        state->rx_ifh_size      = VTSS_JR2_RX_IFH_SIZE;
        state->tx_hdr_encode    = jr2_tx_hdr_encode;
        state->npi_conf_set     = jr2_npi_conf_set;
        state->dma_conf_set     = jr2_dma_conf_set;
        state->dma_offset       = jr2_dma_offset;
        state->rx_queue_count   = VTSS_PACKET_RX_QUEUE_CNT;
#if defined(VTSS_FEATURE_AFI_SWC)
        state->afi_alloc = jr2_afi_alloc;
        state->afi_free  = jr2_afi_free;
#endif /* VTSS_FEATURE_AFI_SWC */
#if defined(VTSS_FEATURE_FDMA) && VTSS_OPT_FDMA
        jaguar_2_fdma_func_init(vtss_state);
#endif
        break;
    case VTSS_INIT_CMD_INIT:
        VTSS_RC(jr2_packet_init(vtss_state));
        break;
    case VTSS_INIT_CMD_PORT_MAP:
        if (!vtss_state->warm_start_cur) {
            VTSS_RC(jr2_rx_conf_set(vtss_state));
        }
        break;
    default:
        break;
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_ARCH_JAGUAR_2 */
