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

#ifndef _VTSS_PACKET_STATE_H_
#define _VTSS_PACKET_STATE_H_

#if defined(VTSS_FEATURE_PACKET)
#if defined(VTSS_ARCH_LUTON28)
typedef struct {
    BOOL                    used;        /* Packet used flag */
    u32                     words_read;  /* Number of words read */
    vtss_packet_rx_header_t header;      /* Rx frame header */
} vtss_packet_rx_t;
#endif  /* VTSS_ARCH_LUTON28 */

typedef struct {
    /* CIL function pointers */
    vtss_rc (*rx_conf_set)(struct vtss_state_s *vtss_state);
    vtss_rc (*rx_frame_get)(struct vtss_state_s *vtss_state,
                            const vtss_packet_rx_queue_t  queue_no,
                            vtss_packet_rx_header_t       *const header,
                            u8                            *const frame,
                            const u32                     length);
    vtss_rc (*rx_frame_get_raw)(struct vtss_state_s *vtss_state,
                                u8                  *const data,
                                const u32           buflen,
                                u32                 *const ifhlen,
                                u32                 *const frmlen);
    vtss_rc (*rx_frame_discard)(struct vtss_state_s *vtss_state,
                                const vtss_packet_rx_queue_t  queue_no);
    vtss_rc (*tx_frame_port)(struct vtss_state_s *vtss_state,
                             const vtss_port_no_t  port_no,
                             const u8              *const frame,
                             const u32             length,
                             const vtss_vid_t      vid);
    vtss_rc (*tx_frame_ifh)(struct vtss_state_s *vtss_state,
                            const vtss_packet_tx_ifh_t *const ifh,
                            const u8              *const frame,
                            const u32             length);
    vtss_rc (*rx_hdr_decode)(const struct vtss_state_s   *const state,
                             const vtss_packet_rx_meta_t *const meta,
                             const u8                     hdr[VTSS_PACKET_HDR_SIZE_BYTES],
                                   vtss_packet_rx_info_t *const info);
    vtss_rc (*tx_hdr_encode)(struct vtss_state_s   *const state,
                             const vtss_packet_tx_info_t *const info,
                                   u8                    *const bin_hdr,
                                   u32                   *const bin_hdr_len);
#if defined(VTSS_FEATURE_VSTAX)
    vtss_rc (*vstax_header2frame)(const struct vtss_state_s     *const state,
                                  const vtss_port_no_t          port_no,
                                  const vtss_vstax_tx_header_t  *const header,
                                  u8                            *const frame);
    vtss_rc (*vstax_frame2header)(const struct vtss_state_s     *const state,
                                  const u8                  *const frame,
                                  vtss_vstax_rx_header_t    *const header);
#endif /* VTSS_FEATURE_VSTAX */

#if defined(VTSS_FEATURE_NPI)
    vtss_rc (*npi_conf_set)(struct vtss_state_s *vtss_state, const vtss_npi_conf_t *const conf);
#endif /* VTSS_FEATURE_NPI */
    
    vtss_rc (*dma_conf_set)(struct vtss_state_s *vtss_state,
                             const vtss_packet_dma_conf_t *const conf);
    vtss_rc (*dma_offset)(struct vtss_state_s *vtss_state,
                          BOOL extraction, u32 *offset);

#if defined(VTSS_FEATURE_AFI_SWC)
    vtss_rc (*afi_alloc)(struct vtss_state_s *vtss_state, vtss_afi_frm_dscr_t *const dscr, vtss_afi_id_t *const id);
    vtss_rc (*afi_free)(struct vtss_state_s *vtss_state, vtss_afi_id_t id);
#endif /* defined(VTSS_FEATURE_AFI_SWC) */

    /* Configuration/state */
    u32                        rx_queue_count;
    vtss_packet_rx_conf_t      rx_conf;
#if defined(VTSS_ARCH_LUTON28)
    vtss_packet_rx_t           rx_packet[VTSS_PACKET_RX_QUEUE_CNT];
#endif  /* VTSS_ARCH_LUTON28 */
#if defined(VTSS_FEATURE_PACKET_PORT_REG)
    vtss_packet_rx_port_conf_t rx_port_conf[VTSS_PORT_ARRAY_SIZE];
#endif /* VTSS_FEATURE_PACKET_PORT_REG */
#if defined(VTSS_FEATURE_NPI)
    vtss_npi_conf_t            npi_conf;
#endif /* VTSS_FEATURE_NPI */
    vtss_packet_dma_conf_t     dma_conf;

    /* RX IFH Size */
    unsigned int               rx_ifh_size;

} vtss_packet_state_t;

vtss_rc vtss_packet_inst_create(struct vtss_state_s *vtss_state);
vtss_rc vtss_cmn_logical_to_chip_port_mask(const struct vtss_state_s *const state,
                                                 u64                 logical_port_mask,
                                                 u64                 *chip_port_mask,
                                                 vtss_chip_no_t      *chip_no,
                                                 vtss_port_no_t      *stack_port_no,
                                                 u32                 *port_cnt,
                                                 vtss_port_no_t      *port_no);

vtss_port_no_t vtss_cmn_chip_to_logical_port(const struct vtss_state_s       *const state,
                                             const vtss_chip_no_t      chip_no,
                                             const vtss_phys_port_no_t chip_port);

vtss_rc vtss_cmn_packet_hints_update(const struct vtss_state_s          *const state,
                                     const vtss_trace_group_t           trc_grp,
                                     const vtss_etype_t                 etype,
                                           vtss_packet_rx_info_t *const info);

void vtss_packet_debug_print(struct vtss_state_s *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info);

#endif /* VTSS_FEATURE_PACKET */

#endif /* _VTSS_PACKET_STATE_H_ */
