#ifndef _VTSS_JAGUAR2_REGS_ASM_H_
#define _VTSS_JAGUAR2_REGS_ASM_H_

/*

 Vitesse Switch API software.

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

#include "vtss_jaguar2_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a ASM
 *
 * Assembler
 *
 ***********************************************************************/

/**
 * Register Group: \a ASM:DEV_STATISTICS
 *
 * Not documented
 */


/** 
 * \brief Rx Byte Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_IN_BYTES_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_IN_BYTES_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,0)


/** 
 * \brief Rx Symbol Carrier Error Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SYMBOL_ERR_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SYMBOL_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,1)


/** 
 * \brief Rx Pause Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_PAUSE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_PAUSE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,2)


/** 
 * \brief Rx Control Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_UNSUP_OPCODE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_UNSUP_OPCODE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,3)


/** 
 * \brief Rx OK Byte Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_OK_BYTES_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_OK_BYTES_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,4)


/** 
 * \brief Rx Bad Byte Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_BAD_BYTES_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_BAD_BYTES_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,5)


/** 
 * \brief Rx Unicast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_UC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_UC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,6)


/** 
 * \brief Rx Multicast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_MC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_MC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,7)


/** 
 * \brief Rx Broadcast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_BC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_BC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,8)


/** 
 * \brief Rx CRC Error Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_CRC_ERR_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_CRC_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,9)


/** 
 * \brief Rx Undersize Counter (valid frame format)
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_UNDERSIZE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_UNDERSIZE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,10)


/** 
 * \brief Rx Undersize Counter (CRC error)
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_FRAGMENTS_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_FRAGMENTS_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,11)


/** 
 * \brief Rx In-range Length Error Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_IN_RANGE_LEN_ERR_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_IN_RANGE_LEN_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,12)


/** 
 * \brief Rx Out-Of-Range Length Error Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_OUT_OF_RANGE_LEN_ERR_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_OUT_OF_RANGE_LEN_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,13)


/** 
 * \brief Rx Oversize Counter (valid frame format)
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_OVERSIZE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_OVERSIZE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,14)


/** 
 * \brief Rx Jabbers Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_JABBERS_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_JABBERS_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,15)


/** 
 * \brief Rx 64 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE64_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE64_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,16)


/** 
 * \brief Rx 65-127 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE65TO127_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE65TO127_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,17)


/** 
 * \brief Rx 128-255 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE128TO255_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE128TO255_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,18)


/** 
 * \brief Rx 256-511 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE256TO511_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE256TO511_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,19)


/** 
 * \brief Rx 512-1023 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE512TO1023_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE512TO1023_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,20)


/** 
 * \brief Rx 1024-1518 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE1024TO1518_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE1024TO1518_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,21)


/** 
 * \brief Rx 1519 To Max. Length Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE1519TOMAX_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE1519TOMAX_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,22)


/** 
 * \brief Rx Inter Packet Gap Shrink Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_IPG_SHRINK_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_IPG_SHRINK_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,23)


/** 
 * \brief Tx Byte Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_OUT_BYTES_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_OUT_BYTES_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,24)


/** 
 * \brief Tx Pause Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_PAUSE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_PAUSE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,25)


/** 
 * \brief Tx OK Byte Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_OK_BYTES_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_OK_BYTES_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,26)


/** 
 * \brief Tx Unicast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_UC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_UC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,27)


/** 
 * \brief Tx Multicast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_MC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_MC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,28)


/** 
 * \brief Tx Broadcast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_BC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_BC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,29)


/** 
 * \brief Tx 64 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE64_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE64_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,30)


/** 
 * \brief Tx 65-127 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE65TO127_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE65TO127_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,31)


/** 
 * \brief Tx 128-255 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE128TO255_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE128TO255_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,32)


/** 
 * \brief Tx 256-511 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE256TO511_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE256TO511_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,33)


/** 
 * \brief Tx 512-1023 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE512TO1023_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE512TO1023_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,34)


/** 
 * \brief Tx 1024-1518 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE1024TO1518_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE1024TO1518_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,35)


/** 
 * \brief Tx 1519 To Max. Length Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE1519TOMAX_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE1519TOMAX_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,36)


/** 
 * \brief Counter to track the dribble-nibble (extra nibble) errors in frames.
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_ALIGNMENT_LOST_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_ALIGNMENT_LOST_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,37)


/** 
 * \brief Counts frames that are tagged (C-Tagged or S-Tagged).
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_TAGGED_FRMS_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_TAGGED_FRMS_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,38)


/** 
 * \brief Counts frames that are Not tagged  (neither C-Tagged nor S-Tagged).
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_UNTAGGED_FRMS_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_UNTAGGED_FRMS_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,39)


/** 
 * \brief Counts frames that are tagged (C-Tagged or S-Tagged).
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_TAGGED_FRMS_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_TAGGED_FRMS_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,40)


/** 
 * \brief Counts frames that are Not tagged  (neither C-Tagged nor S-Tagged).
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_UNTAGGED_FRMS_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_UNTAGGED_FRMS_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,41)


/** 
 * \brief Tx Multi Collision Counter
 *
 * \details
 * Counter collecting the number of frames transmitted without errors after
 * multiple collisions.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_MULTI_COLL_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_MULTI_COLL_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,42)


/** 
 * \brief Tx Late Collision Counter
 *
 * \details
 * Counter collecting the number of late collisions.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_LATE_COLL_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_LATE_COLL_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,43)


/** 
 * \brief Tx Excessive Collision Counter
 *
 * \details
 * Counter collecting the number of frames due to excessive collisions.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_XCOLL_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_XCOLL_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,44)


/** 
 * \brief Tx First Defer Counter
 *
 * \details
 * Counter collecting the number of frames being deferred on first
 * transmission attempt.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_DEFER_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_DEFER_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,45)


/** 
 * \brief Tx Excessive Defer Counter
 *
 * \details
 * Counter collecting the number of frames sent with excessive deferral.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_XDEFER_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_XDEFER_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,46)


/** 
 * \brief Tx 1 Backoff Counter
 *
 * \details
 * Counter collecting the number of frames sent successfully after 1
 * backoff/collision.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_BACKOFF1_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_BACKOFF1_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,47)


/** 
 * \brief Tx Carrier Sense Error Counter
 *
 * \details
 * Counter collecting the number of times CarrierSenseError is true at the
 * end of a frame transmission.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_CSENSE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_CSENSE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,48)


/** 
 * \brief MSB of RX in byte Counter
 *
 * \details
 * Register allowing to access the upper 4 bits of RX_IN_BYTE counter.
 * Please note: When writing to RX_IN_BYTES counter RX_IN_BYTES_MSB_CNT has
 * to be written before RX_IN_BYTES_CNT is written. When reading
 * RX_IN_BYTES counter RX_IN_BYTES_CNT has to be read before
 * RX_IN_BYTES_MSB_CNT is read. Accessing both counters must not be
 * interfered by other register accesses.
 *
 * Register: \a ASM:DEV_STATISTICS:RX_IN_BYTES_MSB_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_IN_BYTES_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,49)

/** 
 * \brief
 * Upper 4 bits of RX_IN_BYTES_CNT.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_ASM_DEV_STATISTICS_RX_IN_BYTES_MSB_CNT . RX_IN_BYTES_MSB_CNT
 */
#define  VTSS_F_ASM_DEV_STATISTICS_RX_IN_BYTES_MSB_CNT_RX_IN_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_ASM_DEV_STATISTICS_RX_IN_BYTES_MSB_CNT_RX_IN_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_ASM_DEV_STATISTICS_RX_IN_BYTES_MSB_CNT_RX_IN_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief MSB of RX ok byte Counter
 *
 * \details
 * Register allowing to access the upper 4 bits of RX_IN_BYTE counter.
 * Please note: When writing to RX_OK_BYTES counter RX_OK_BYTES_MSB_CNT has
 * to be written before RX_OK_BYTES_CNT is written. When reading
 * RX_OK_BYTES counter RX_OK_BYTES_CNT has to be read before
 * RX_OK_BYTES_MSB_CNT is read. Accessing both counters must not be
 * interfered by other register accesses.
 *
 * Register: \a ASM:DEV_STATISTICS:RX_OK_BYTES_MSB_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_OK_BYTES_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,50)

/** 
 * \brief
 * Upper 4 bits of RX_OK_BYTES_CNT.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_ASM_DEV_STATISTICS_RX_OK_BYTES_MSB_CNT . RX_OK_BYTES_MSB_CNT
 */
#define  VTSS_F_ASM_DEV_STATISTICS_RX_OK_BYTES_MSB_CNT_RX_OK_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_ASM_DEV_STATISTICS_RX_OK_BYTES_MSB_CNT_RX_OK_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_ASM_DEV_STATISTICS_RX_OK_BYTES_MSB_CNT_RX_OK_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief MSB of RX bad byte Counter
 *
 * \details
 * Register allowing to access the upper 4 bits of RX_IN_BYTE counter.
 * Please note: When writing to RX_BAD_BYTES counter RX_BAD_BYTES_MSB_CNT
 * has to be written before RX_BAD_BYTES_CNT is written. When reading
 * RX_BAD_BYTES counter RX_BAD_BYTES_CNT has to be read before
 * RX_BAD_BYTES_MSB_CNT is read. Accessing both counters must not be
 * interfered by other register accesses.
 *
 * Register: \a ASM:DEV_STATISTICS:RX_BAD_BYTES_MSB_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_BAD_BYTES_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,51)

/** 
 * \brief
 * Upper 4 bits of RX_BAD_BYTES_CNT.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_ASM_DEV_STATISTICS_RX_BAD_BYTES_MSB_CNT . RX_BAD_BYTES_MSB_CNT
 */
#define  VTSS_F_ASM_DEV_STATISTICS_RX_BAD_BYTES_MSB_CNT_RX_BAD_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_ASM_DEV_STATISTICS_RX_BAD_BYTES_MSB_CNT_RX_BAD_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_ASM_DEV_STATISTICS_RX_BAD_BYTES_MSB_CNT_RX_BAD_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief MSB of TX out byte Counter
 *
 * \details
 * Register allowing to access the upper 4 bits of RX_IN_BYTE counter.
 * Please note: When writing to TX_OUT_BYTES counter TX_OUT_BYTES_MSB_CNT
 * has to be written before TX_OUT_BYTES_CNT is written. When reading
 * TX_OUT_BYTES counter TX_OUT_BYTES_CNT has to be read before
 * TX_OUT_BYTES_MSB_CNT is read. Accessing both counters must not be
 * interfered by other register accesses.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_OUT_BYTES_MSB_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_OUT_BYTES_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,52)

/** 
 * \brief
 * Upper 4 bits of TX_OUT_BYTES_CNT.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_ASM_DEV_STATISTICS_TX_OUT_BYTES_MSB_CNT . TX_OUT_BYTES_MSB_CNT
 */
#define  VTSS_F_ASM_DEV_STATISTICS_TX_OUT_BYTES_MSB_CNT_TX_OUT_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_ASM_DEV_STATISTICS_TX_OUT_BYTES_MSB_CNT_TX_OUT_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_ASM_DEV_STATISTICS_TX_OUT_BYTES_MSB_CNT_TX_OUT_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief MSB of TX ok byte Counter
 *
 * \details
 * Register allowing to access the upper 4 bits of RX_IN_BYTE counter.
 * Please note: When writing to TX_OK_BYTES counter TX_OK_BYTES_MSB_CNT has
 * to be written before TX_OK_BYTES_CNT is written. When reading
 * TX_OK_BYTES counter TX_OK_BYTES_CNT has to be read before
 * TX_OK_BYTES_MSB_CNT is read. Accessing both counters must not be
 * interfered by other register accesses.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_OK_BYTES_MSB_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_TX_OK_BYTES_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,53)

/** 
 * \brief
 * Upper 4 bits of TX_OK_BYTES_CNT.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_ASM_DEV_STATISTICS_TX_OK_BYTES_MSB_CNT . TX_OK_BYTES_MSB_CNT
 */
#define  VTSS_F_ASM_DEV_STATISTICS_TX_OK_BYTES_MSB_CNT_TX_OK_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_ASM_DEV_STATISTICS_TX_OK_BYTES_MSB_CNT_TX_OK_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_ASM_DEV_STATISTICS_TX_OK_BYTES_MSB_CNT_TX_OK_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Counter to track the PCS's Sync-lost error
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SYNC_LOST_ERR_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-52
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SYNC_LOST_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,54)

/**
 * Register Group: \a ASM:CFG
 *
 * Assembler Configuration Registers
 */


/** 
 * \brief Statistics counter configuration
 *
 * \details
 * Register that contains the bitgroups to configure/control the statistics
 * counters.
 *
 * Register: \a ASM:CFG:STAT_CFG
 */
#define VTSS_ASM_CFG_STAT_CFG                VTSS_IOREG(VTSS_TO_ASM,0xd42)

/** 
 * \brief
 * Setting of this bit initiates the clearing of all statistics counter.
 *
 * \details 
 * '0': No action
 * '1': Stat cnt clr (Bit is automatically cleared)
 *
 * Field: ::VTSS_ASM_CFG_STAT_CFG . STAT_CNT_CLR_SHOT
 */
#define  VTSS_F_ASM_CFG_STAT_CFG_STAT_CNT_CLR_SHOT  VTSS_BIT(0)


/** 
 * \brief MAC Address Configuration Register (MSB)
 *
 * \details
 * Register: \a ASM:CFG:MAC_ADDR_HIGH_CFG
 *
 * @param ri Replicator: x_NUM_ASM_PORTS (??), 0-54
 */
#define VTSS_ASM_CFG_MAC_ADDR_HIGH_CFG(ri)   VTSS_IOREG(VTSS_TO_ASM,0xd43 + (ri))

/** 
 * \brief
 * Upper 24 bits of MAC address.
 * The MAC address is used when filtering incoming Pause Control Frames -
 * i.e. when the ASM detemines whether or not a pause value must be passed
 * to the DSM.
 *
 * \details 
 * The resulting MAC address of a device is determined as:
 * MAC_ADDR_HIGH  & MAC_ADDR_LOW.
 *
 * Field: ::VTSS_ASM_CFG_MAC_ADDR_HIGH_CFG . MAC_ADDR_HIGH
 */
#define  VTSS_F_ASM_CFG_MAC_ADDR_HIGH_CFG_MAC_ADDR_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_ASM_CFG_MAC_ADDR_HIGH_CFG_MAC_ADDR_HIGH     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_ASM_CFG_MAC_ADDR_HIGH_CFG_MAC_ADDR_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief MAC Address Configuration Register (LSB)
 *
 * \details
 * Register: \a ASM:CFG:MAC_ADDR_LOW_CFG
 *
 * @param ri Replicator: x_NUM_ASM_PORTS (??), 0-54
 */
#define VTSS_ASM_CFG_MAC_ADDR_LOW_CFG(ri)    VTSS_IOREG(VTSS_TO_ASM,0xd7a + (ri))

/** 
 * \brief
 * Lower 24 bits of MAC address.
 * The MAC address is used when filtering incoming Pause Control Frames -
 * i.e. when the ASM detemines whether or not a pause value must be passed
 * to the DSM.
 *
 * \details 
 * The resulting MAC address of a device is determined as:
 * MAC_ADDR_HIGH  & MAC_ADDR_LOW.

 *
 * Field: ::VTSS_ASM_CFG_MAC_ADDR_LOW_CFG . MAC_ADDR_LOW
 */
#define  VTSS_F_ASM_CFG_MAC_ADDR_LOW_CFG_MAC_ADDR_LOW(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_ASM_CFG_MAC_ADDR_LOW_CFG_MAC_ADDR_LOW     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_ASM_CFG_MAC_ADDR_LOW_CFG_MAC_ADDR_LOW(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Port configuration
 *
 * \details
 * This register holds port configuration bit groups
 *
 * Register: \a ASM:CFG:PORT_CFG
 *
 * @param ri Replicator: x_NUM_ASM_PORTS (??), 0-54
 */
#define VTSS_ASM_CFG_PORT_CFG(ri)            VTSS_IOREG(VTSS_TO_ASM,0xdb1 + (ri))

/** 
 * \brief
 * Disables the CSC statistics counters in the ASM for the port. Set this
 * when the port utilizes a DEV10G device as this handles the statistics
 * locally in the device.
 *
 * \details 
 * Field: ::VTSS_ASM_CFG_PORT_CFG . CSC_STAT_DIS
 */
#define  VTSS_F_ASM_CFG_PORT_CFG_CSC_STAT_DIS  VTSS_BIT(11)

/** 
 * \brief
 * If this field is set the ASM remove the first 4 bytes of the payload and
 * insert it into the HIH field of the IFH.
 *
 * \details 
 * Field: ::VTSS_ASM_CFG_PORT_CFG . HIH_AFTER_PREAMBLE_ENA
 */
#define  VTSS_F_ASM_CFG_PORT_CFG_HIH_AFTER_PREAMBLE_ENA  VTSS_BIT(10)

/** 
 * \brief
 * If this field is set the ASM will ignore any abort indication received
 * on the TAXI interface for the port.
 *
 * \details 
 * Field: ::VTSS_ASM_CFG_PORT_CFG . IGN_TAXI_ABORT_ENA
 */
#define  VTSS_F_ASM_CFG_PORT_CFG_IGN_TAXI_ABORT_ENA  VTSS_BIT(9)

/** 
 * \brief
 * If this field is set the ASM does not expect the incoming frame data to
 * have a preamble prepended.
 *
 * \details 
 * Field: ::VTSS_ASM_CFG_PORT_CFG . NO_PREAMBLE_ENA
 */
#define  VTSS_F_ASM_CFG_PORT_CFG_NO_PREAMBLE_ENA  VTSS_BIT(8)

/** 
 * \brief
 * If this field is set the ASM will not store the first 8 bytes from the
 * packet. This must be enabled when injecting with IFH without prefix from
 * an extenal CPU (INJ_FORMAT_CFG=1).
 *
 * \details 
 * Field: ::VTSS_ASM_CFG_PORT_CFG . SKIP_PREAMBLE_ENA
 */
#define  VTSS_F_ASM_CFG_PORT_CFG_SKIP_PREAMBLE_ENA  VTSS_BIT(7)

/** 
 * \brief
 * This field determines if the ASM must abort mark frames that become
 * older than 16-24 ms before and EOF is received.
 *
 * \details 
 * '0': Aging enabled.
 * '1': Aging disabled.
 *
 * Field: ::VTSS_ASM_CFG_PORT_CFG . FRM_AGING_DIS
 */
#define  VTSS_F_ASM_CFG_PORT_CFG_FRM_AGING_DIS  VTSS_BIT(6)

/** 
 * \brief
 * This field determines if the ASM mustzero-pad Ethernet frames that are
 * less than 64 bytes.
 *
 * \details 
 * '0': Padding is disabled. Frames that are less than 64 bytes and have
 * not been abort marked are passed to the ANA block 'as is'. Frames that
 * are less than 64 bytes and have been abort marked are normally discarded
 * silently by the ASM.
 * '1': Padding is enabled. If the resulting frame size will be less than
 * 64 bytes, the frame is zero-padded, so that the resulting frame size is
 * 64 bytes.
 *
 * Field: ::VTSS_ASM_CFG_PORT_CFG . PAD_ENA
 */
#define  VTSS_F_ASM_CFG_PORT_CFG_PAD_ENA      VTSS_BIT(5)

/** 
 * \brief
 * Configure discard behaviour depending on matching result of the selected
 * injection format.
 * This setting is only valid for injection formats with short or long
 * prefix.
 *
 * \details 
 * 0: Discard none
 * 1: Discard frames with wrong injection format
 * 2: Discard frames with correct injection format
 *
 * Field: ::VTSS_ASM_CFG_PORT_CFG . INJ_DISCARD_CFG
 */
#define  VTSS_F_ASM_CFG_PORT_CFG_INJ_DISCARD_CFG(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_ASM_CFG_PORT_CFG_INJ_DISCARD_CFG     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_ASM_CFG_PORT_CFG_INJ_DISCARD_CFG(x)  VTSS_EXTRACT_BITFIELD(x,3,2)

/** 
 * \brief
 * Set the mode for the formatting of incoming frames. Four different modes
 * can be selected:
 *  - Normal mode (No IFH)
 *  - IFH without prefix
 *  - IFH with short prefix
 *  - IFH with long prefix
 * 
 * If one of the IFH modes are selected incoming frames are expected to
 * contain the selected prefix followed by an IFH as the first part of the
 * frame. Frames are forwarded based on the contents in the IFH instead of
 * normal forwarding.
 * 
 * Three different prefix modes are supported:
 * - No prefix.
 * - IFH short prefix:. any DMAC, any SMAC, EtherType=0x8880,
 * payload=0x0007
 * - IFH long prefix: any DMAC, any SMAC, VLAN Tag, EtherType=0x8880,
 * payload=0x0007.
 * 
 * In the IFH modes, if the incoming frame's format does not comply with
 * the prefix, then IFH_PREFIX_ERR_STICKY is set.
 *
 * \details 
 * 0: Normal mode (No IFH)
 * 1: IFH without prefix
 * 2: IFH with short prefix
 * 3: IFH with long prefix
 *
 * Field: ::VTSS_ASM_CFG_PORT_CFG . INJ_FORMAT_CFG
 */
#define  VTSS_F_ASM_CFG_PORT_CFG_INJ_FORMAT_CFG(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_ASM_CFG_PORT_CFG_INJ_FORMAT_CFG     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_ASM_CFG_PORT_CFG_INJ_FORMAT_CFG(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * This bit defines if the ASM must be Vstax2 aware or not. If Vstax2
 * awareness is enabled and a frame holds a Vstax2 header following the
 * SMAC address, this Vstax2 header is removed from the frame and placed in
 * the IFH and the vstax_avail and update_fcs fields in the IFH will be
 * set, so that the frame FCS is recalculated in the egress direction.
 * If Vstax2 awareness is disabled or a frame does not hold a Vstax2
 * header, no bytes will be removed from the frame and the vstax_hdr,
 * vstax_avail and fcs_update fields in the IFH will be cleared.
 * When Vstax2 awareness is enabled INJ_FORMAT_CFG must be set to 0
 *
 * \details 
 * 0: Vstax2 awareness is disabled. 
 * 1: Vstax2 awareness is enabled.
 *
 * Field: ::VTSS_ASM_CFG_PORT_CFG . VSTAX2_AWR_ENA
 */
#define  VTSS_F_ASM_CFG_PORT_CFG_VSTAX2_AWR_ENA  VTSS_BIT(0)


/** 
 * \brief Holds DEVCPU specific Flow Control configuration signals
 *
 * \details
 * Register: \a ASM:CFG:CPU_FC_CFG
 */
#define VTSS_ASM_CFG_CPU_FC_CFG              VTSS_IOREG(VTSS_TO_ASM,0xde8)

/** 
 * \brief
 * This field determines the ASM FIFO fill level required for the ASM to
 * activate FC. The fill level is given by the number of complete cells in
 * the FIFO that are ready to be passed to the ANA block.
 *
 * \details 
 * 0: Flow control is activated if the ASM FIFO of the DEVCPU holds 1 or
 * more complete cells.
 * 1: Flow control is activated if the ASM FIFO of the DEVCPU holds 2 or
 * more complete cells.
 * ---
 * X: Flow control is activated if the ASM FIFO of the DEVCPU holds X+1 or
 * more complete cells.
 *
 * Field: ::VTSS_ASM_CFG_CPU_FC_CFG . CPU_FC_WM
 */
#define  VTSS_F_ASM_CFG_CPU_FC_CFG_CPU_FC_WM(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_ASM_CFG_CPU_FC_CFG_CPU_FC_WM     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_ASM_CFG_CPU_FC_CFG_CPU_FC_WM(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * This field determines if the ASM must assert the flow control signal to
 * the CPU device when the ASM FIFO fill level exceeds the watermark given
 * in	   CPU_FC_WM.
 *
 * \details 
 * '0': Flow control is disabled.
 * '1': Flow control is enabled.
 *
 * Field: ::VTSS_ASM_CFG_CPU_FC_CFG . CPU_FC_ENA
 */
#define  VTSS_F_ASM_CFG_CPU_FC_CFG_CPU_FC_ENA  VTSS_BIT(0)


/** 
 * \brief Holds configuration related to Pause frame detection.
 *
 * \details
 * This register control whether pause and control frames should be
 * forwarded or terminated at ingress.
 *
 * Register: \a ASM:CFG:PAUSE_CFG
 *
 * @param ri Replicator: x_NUM_ASM_PORTS (??), 0-54
 */
#define VTSS_ASM_CFG_PAUSE_CFG(ri)           VTSS_IOREG(VTSS_TO_ASM,0xde9 + (ri))

/** 
 * \brief
 * This field indicates whether or not the ASM must discard a valid Pause
 * frame to the IQS by asserting the abort signal.
 * One configuration bit is defined for each port.
 *
 * \details 
 * '0': The ASM must not discard valid Pause frames.
 * '1': The ASM must discard valid Pause frames to the IQS by asserting the
 * abort signal, but the Pause value must still be used to stall the egress
 * data flow.
 *
 * Field: ::VTSS_ASM_CFG_PAUSE_CFG . ABORT_PAUSE_ENA
 */
#define  VTSS_F_ASM_CFG_PAUSE_CFG_ABORT_PAUSE_ENA  VTSS_BIT(0)

/** 
 * \brief
 * This field indicates whether or not the ASM must discard Control frames
 * with type 0x8808 not being Pause frames, to the IQS by asserting the
 * abort signal.
 * One configuration bit is defined for each port.
 *
 * \details 
 * '0': The ASM must not discard Control frames.
 * '1': The ASM must discard Control frames to the IQS by asserting the
 * abort signal.
 *
 * Field: ::VTSS_ASM_CFG_PAUSE_CFG . ABORT_CTRL_ENA
 */
#define  VTSS_F_ASM_CFG_PAUSE_CFG_ABORT_CTRL_ENA  VTSS_BIT(1)


/** 
 * \brief Configure custom VLAN tag for injection
 *
 * \details
 * Register: \a ASM:CFG:INJ_VLAN_CFG
 */
#define VTSS_ASM_CFG_INJ_VLAN_CFG            VTSS_IOREG(VTSS_TO_ASM,0xe20)

/** 
 * \brief
 * The VID used for VLAN tag matching when injection with long IFH prefix
 * is selected in INJ_FORMAT_CFG.
 *
 * \details 
 * Field: ::VTSS_ASM_CFG_INJ_VLAN_CFG . INJ_VID_CFG
 */
#define  VTSS_F_ASM_CFG_INJ_VLAN_CFG_INJ_VID_CFG(x)  VTSS_ENCODE_BITFIELD(x,16,12)
#define  VTSS_M_ASM_CFG_INJ_VLAN_CFG_INJ_VID_CFG     VTSS_ENCODE_BITMASK(16,12)
#define  VTSS_X_ASM_CFG_INJ_VLAN_CFG_INJ_VID_CFG(x)  VTSS_EXTRACT_BITFIELD(x,16,12)

/** 
 * \brief
 * The TPID used for VLAN tag matching when injection with long IFH prefix
 * is selected in INJ_FORMAT_CFG.
 *
 * \details 
 * Field: ::VTSS_ASM_CFG_INJ_VLAN_CFG . INJ_TPID_CFG
 */
#define  VTSS_F_ASM_CFG_INJ_VLAN_CFG_INJ_TPID_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ASM_CFG_INJ_VLAN_CFG_INJ_TPID_CFG     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ASM_CFG_INJ_VLAN_CFG_INJ_TPID_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a ASM:DBG
 *
 * Assembler Debug Registers
 */


/** 
 * \brief Miscellaneous debug configuration
 *
 * \details
 * This register holds miscellaneous configuration bit groups used for
 * debug
 *
 * Register: \a ASM:DBG:DBG_CFG
 */
#define VTSS_ASM_DBG_DBG_CFG                 VTSS_IOREG(VTSS_TO_ASM,0xe21)

/** 
 * \brief
 * If this fields is set to 1, cell cycles allocated for front ports, but
 * with no data available, will be given to the virtual device. Otherwise
 * the VD is only given the cycles set by the cell bus calendar.
 *
 * \details 
 * Field: ::VTSS_ASM_DBG_DBG_CFG . IDLE_TO_VD
 */
#define  VTSS_F_ASM_DBG_DBG_CFG_IDLE_TO_VD    VTSS_BIT(2)

/** 
 * \brief
 * This field indicates if a given ASM FIFO must be reset or not. Resetting
 * a FIFO does not affect the device status interface to the DSM.
 *
 * \details 
 * '0': The FIFO is NOT reset.
 * '1': The FIFO is reset and it will remain reset until FIFO_RST is
 * de-asserted.
 *
 * Field: ::VTSS_ASM_DBG_DBG_CFG . FIFO_RST
 */
#define  VTSS_F_ASM_DBG_DBG_CFG_FIFO_RST(x)   VTSS_ENCODE_BITFIELD(x,3,9)
#define  VTSS_M_ASM_DBG_DBG_CFG_FIFO_RST      VTSS_ENCODE_BITMASK(3,9)
#define  VTSS_X_ASM_DBG_DBG_CFG_FIFO_RST(x)   VTSS_EXTRACT_BITFIELD(x,3,9)

/** 
 * \brief
 * This field can be used to disable the cell bus output of the ASM - i.e.
 * to configure the ASM to replace any data cells with IDLE cells. Data is
 * flushed from the ASM FIFO when the output is disabled.
 *
 * \details 
 * '0': Any data cells read from the ASM FIFO are passed to the ANA block.
 * '1': No data cells are passed to the ANA block. Only IDLE and REFRESH
 * cells will be transmitted. Data is still read from the ASM FIFO, even
 * though the cell bus interface has been disabled.
 *
 * Field: ::VTSS_ASM_DBG_DBG_CFG . CELL_BUS_DIS
 */
#define  VTSS_F_ASM_DBG_DBG_CFG_CELL_BUS_DIS  VTSS_BIT(1)

/** 
 * \brief
 * This field can be used to configure the ASM not to silently discard
 * frames that are aborted by a device within the first cell of a frame.
 * Note that enabling this feature may cause overflow in the ASM when small
 * fragments are received at the Taxi interface.
 *
 * \details 
 * '0': Frames are silently discarded by the ASM if it is aborted within
 * the first cell of the frame.
 * '1': No frames are silently discarded by the ASM
 *
 * Field: ::VTSS_ASM_DBG_DBG_CFG . ABORT_DIS
 */
#define  VTSS_F_ASM_DBG_DBG_CFG_ABORT_DIS     VTSS_BIT(0)


/** 
 * \brief Holds a number of sticky bits that are set if internal errors are detected.
 *
 * \details
 * Writing a '1' to a bit group clears that bit.
 *
 * Register: \a ASM:DBG:ERR_STICKY
 *
 * @param ri Replicator: x_NUM_OF_TAXI (??), 0-8
 */
#define VTSS_ASM_DBG_ERR_STICKY(ri)          VTSS_IOREG(VTSS_TO_ASM,0xe22 + (ri))

/** 
 * \brief
 * The ASM must assert a sticky bit if an underflow occurs in the 'free
 * cell' FIFO.
 *
 * \details 
 * '0': No underflow has been detected in the 'free cell' FIFO.
 * '1': An underflow has been detected in the 'free cell' FIFO.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . FC_UFLW_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_FC_UFLW_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * The ASM must assert a sticky bit if an overflow occurs in the 'free
 * cell' FIFO.
 *
 * \details 
 * '0': No overflow has been detected in the 'free cell' FIFO.
 * '1': An overflow has been detected in the 'free cell' FIFO.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . FC_OFLW_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_FC_OFLW_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * The ASM must assert a sticky bit if an internal error occurs in the
 * 'complete cell' FIFO.
 *
 * \details 
 * '0': No internal error has been detected in the 'complete cell' FIFO.
 * '1': An internal error has been detected in the 'complete cell' FIFO.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . CC_INTRN_ERR_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_CC_INTRN_ERR_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * The ASM must assert a sticky bit if an underflow occurs in the 'complete
 * cell' FIFO.
 *
 * \details 
 * '0': No underflow has been detected in the 'complete cell' FIFO.
 * '1': An underflow has been detected in the 'complete cell' FIFO.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . CC_UFLW_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_CC_UFLW_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * The ASM must assert a sticky bit if an overflow occurs in the 'complete
 * cell' FIFO.
 *
 * \details 
 * '0': No overflow has been detected in the 'complete cell' FIFO.
 * '1': An overflow has been detected in the 'complete cell' FIFO.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . CC_OFLW_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_CC_OFLW_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Cell words must only be granted a given Taxi bus every 3rd cell cycle or
 * more. I.e. for Taxi A there must always be two or more cell slots given
 * to another Taxi other than A or idle, before Taxi A is allowed to get
 * the next grant. If the cell bus calendar causes 2 cell slots to be
 * allocated the same Taxi bus within 3 cell cycles, the last cell slot is
 * ignored and a sticky bit is asserted.
 *
 * \details 
 * '0': No cell slot calendar error detected.
 * '1': One or more cell slots have been ignored by ASM.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . CALENDAR_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_CALENDAR_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * This sticky bit is set if a partial Taxi word (unused_bytes <> 0) is
 * received while EOF = 0.
 *
 * \details 
 * '0': No error detected in UNUSED_BYTES field ofTaxi word.
 * '1': One or more Taxi words have been received where the UNUSED_BYTES
 * field was different from 0 and EOP = 0.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . UNUSED_BYTES_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_UNUSED_BYTES_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * This sticky bit is set if the ASM passes a frame to the cell bus, which
 * is less than 64 bytes (before any padding is done, if enabled) - and has
 * not been aborted or abortion has been disabled.
 * The padding configuration does not affect this sticky bit.
 *
 * \details 
 * '0': No error detected.
 * '1': One or more frames have been passed to the cell bus, where the
 * frame size was less than 64 bytes.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . FRAGMENT_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_FRAGMENT_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * This sticky bit is set if the ASM receives a Taxi word where SOF is
 * asserted and the previous valid Taxi word from that port did not hold an
 * EOF.
 *
 * \details 
 * '0': No missing EOF detected
 * '1': Missing EOF detected
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . MISSING_EOF_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_MISSING_EOF_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * This sticky bit is set if the ASM receives a Taxi word where ABORT is
 * asserted but EOF is not asserted.
 *
 * \details 
 * '0': No misaligned ABORT/EOF indications detected.
 * '1': Misaligned ABORT/EOF indications detected.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . INVLD_ABORT_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_INVLD_ABORT_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * This sticky bit is set if the ASM receives a Taxi word with SOF=0 and
 * the previous valid Taxi word from that port hold an EOF.
 *
 * \details 
 * '0': No missing EOF detected
 * '1': Missing EOF detected
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . MISSING_SOF_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_MISSING_SOF_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * The ASM must assert a sticky bit if an overflow occurs in the main
 * statemachine.
 *
 * \details 
 * '0': No overflow has been detected in the main statemachine.
 * '1': An overflow has been detected in the main statemachine.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . MAIN_SM_INTRN_ERR_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_MAIN_SM_INTRN_ERR_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * The ASM must assert a sticky bit if an overflow occurs in the main
 * statemachine.
 *
 * \details 
 * '0': No overflow has been detected in the main statemachine.
 * '1': An overflow has been detected in the main statemachine.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . MAIN_SM_OFLW_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_MAIN_SM_OFLW_STICKY  VTSS_BIT(0)


/** 
 * \brief Register containing sticky bits for pre counter overflow
 *
 * \details
 * Register containing sticky bits for pre counter overflow
 *
 * Register: \a ASM:DBG:PRE_CNT_OFLW_STICKY
 */
#define VTSS_ASM_DBG_PRE_CNT_OFLW_STICKY     VTSS_IOREG(VTSS_TO_ASM,0xe2b)

/** 
 * \brief
 * Will be set if one of the statistics pre counters for unsupported
 * control frames has an overflow.
 *
 * \details 
 * '0': An overflow in	pre-counter has not occured
 * '1': An overflow in	pre-counter has occured
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_PRE_CNT_OFLW_STICKY . UNSUP_OPCODE_PRE_CNT_OFLW_STICKY
 */
#define  VTSS_F_ASM_DBG_PRE_CNT_OFLW_STICKY_UNSUP_OPCODE_PRE_CNT_OFLW_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Will be set if one of the statistics pause frame pre counters has an
 * overflow.
 *
 * \details 
 * '0': An overflow in	pre-counter has not occured
 * '1': An overflow in	pre-counter has occured
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_PRE_CNT_OFLW_STICKY . PAUSE_FRM_PRE_CNT_OFLW_STICKY
 */
#define  VTSS_F_ASM_DBG_PRE_CNT_OFLW_STICKY_PAUSE_FRM_PRE_CNT_OFLW_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ASM:PORT_STATUS
 *
 * Status for ASM ingress ports
 */


/** 
 * \brief ASM port sticky bits
 *
 * \details
 * This register holds all the sticky bits that exists for each port.
 *
 * Register: \a ASM:PORT_STATUS:PORT_STICKY
 *
 * @param ri Replicator: x_NUM_ASM_PORTS (??), 0-54
 */
#define VTSS_ASM_PORT_STATUS_PORT_STICKY(ri)  VTSS_IOREG(VTSS_TO_ASM,0xe2c + (ri))

/** 
 * \brief
 *  This field is set if the PORT_CFG.INJ_FORMAT_CFG field is set to one of
 * the IFH modes and the incoming frame's format does not comply with the
 * configured prefix.
 *
 * \details 
 * Field: ::VTSS_ASM_PORT_STATUS_PORT_STICKY . IFH_PREFIX_ERR_STICKY
 */
#define  VTSS_F_ASM_PORT_STATUS_PORT_STICKY_IFH_PREFIX_ERR_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * This field indicates if one or more Ethernet frames have been discarded
 * due to aging.
 *
 * \details 
 * '0': No Ethernet frames have been discarded due to aging.
 * '1': One or more Ethernet frames have been discarded due to aging.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_PORT_STATUS_PORT_STICKY . FRM_AGING_STICKY
 */
#define  VTSS_F_ASM_PORT_STATUS_PORT_STICKY_FRM_AGING_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ASM:PFC
 *
 * Priority-based flow control configuration
 */


/** 
 * \brief Priority-based flow control configuration
 *
 * \details
 * Register: \a ASM:PFC:PFC_CFG
 *
 * @param gi Replicator: x_NUM_ASM_PORTS (??), 0-54
 */
#define VTSS_ASM_PFC_PFC_CFG(gi)             VTSS_IOREG_IX(VTSS_TO_ASM,0x1000,gi,16,0,0)

/** 
 * \brief
 * Enable PFC per priority. Bit n enables PFC on priority n.
 *
 * \details 
 * Field: ::VTSS_ASM_PFC_PFC_CFG . RX_PFC_ENA
 */
#define  VTSS_F_ASM_PFC_PFC_CFG_RX_PFC_ENA(x)  VTSS_ENCODE_BITFIELD(x,3,8)
#define  VTSS_M_ASM_PFC_PFC_CFG_RX_PFC_ENA     VTSS_ENCODE_BITMASK(3,8)
#define  VTSS_X_ASM_PFC_PFC_CFG_RX_PFC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,3,8)

/** 
 * \brief
 * Configures the link speed. This is used to evaluate the time
 * specifications in incoming pause frames. 
 *
 * \details 
 * 0: 12000 Mbps
 * 1: 10000 Mbps
 * 2: 2500 Mbps
 * 3: 1000 Mbps
 * 4: 100 Mbps
 * 5: 10 Mbps
 *
 * Field: ::VTSS_ASM_PFC_PFC_CFG . FC_LINK_SPEED
 */
#define  VTSS_F_ASM_PFC_PFC_CFG_FC_LINK_SPEED(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ASM_PFC_PFC_CFG_FC_LINK_SPEED     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ASM_PFC_PFC_CFG_FC_LINK_SPEED(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Current timer per priority
 *
 * \details
 * Register: \a ASM:PFC:PFC_TIMER
 *
 * @param gi Replicator: x_NUM_ASM_PORTS (??), 0-54
 * @param ri Replicator: x_PRIO_CNT (??), 0-7
 */
#define VTSS_ASM_PFC_PFC_TIMER(gi,ri)        VTSS_IOREG_IX(VTSS_TO_ASM,0x1000,gi,16,ri,1)

/** 
 * \brief
 * The current timer value per priority. Value >0 indicates that the
 * priority is paused.
 *
 * \details 
 * Unit is 1024 bit times.
 *
 * Field: ::VTSS_ASM_PFC_PFC_TIMER . PFC_TIMER_VAL
 */
#define  VTSS_F_ASM_PFC_PFC_TIMER_PFC_TIMER_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ASM_PFC_PFC_TIMER_PFC_TIMER_VAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ASM_PFC_PFC_TIMER_PFC_TIMER_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a ASM:LBK_WM_CFG
 *
 * Loopback Watermark Configuration
 */


/** 
 * \brief Loopback watermark configration for flowcontrol on virtual devices
 *
 * \details
 * Register: \a ASM:LBK_WM_CFG:VD_FC_WM
 *
 * @param ri Register: VD_FC_WM (??), 0-1
 */
#define VTSS_ASM_LBK_WM_CFG_VD_FC_WM(ri)     VTSS_IOREG(VTSS_TO_ASM,0xe63 + (ri))

/** 
 * \brief
 * Flowcontrol to QS is set when the FIFO fill level reaches this
 * watermark.
 *
 * \details 
 * Field: ::VTSS_ASM_LBK_WM_CFG_VD_FC_WM . VD_FC_WM
 */
#define  VTSS_F_ASM_LBK_WM_CFG_VD_FC_WM_VD_FC_WM(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_ASM_LBK_WM_CFG_VD_FC_WM_VD_FC_WM     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_ASM_LBK_WM_CFG_VD_FC_WM_VD_FC_WM(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

/**
 * Register Group: \a ASM:LBK_MISC_CFG
 *
 * Not documented
 */


/** 
 * \brief Disable aging 
 *
 * \details
 * Register: \a ASM:LBK_MISC_CFG:LBK_AGING_DIS
 *
 * @param ri Register: LBK_AGING_DIS (??), 0-1
 */
#define VTSS_ASM_LBK_MISC_CFG_LBK_AGING_DIS(ri)  VTSS_IOREG(VTSS_TO_ASM,0xe65 + (ri))


/** 
 * \details
 * Register: \a ASM:LBK_MISC_CFG:LBK_FIFO_CFG
 *
 * @param ri Register: LBK_FIFO_CFG (??), 0-2
 */
#define VTSS_ASM_LBK_MISC_CFG_LBK_FIFO_CFG(ri)  VTSS_IOREG(VTSS_TO_ASM,0xe67 + (ri))

/** 
 * \brief
 * Flush all data in the FIFO
 *
 * \details 
 * Field: ::VTSS_ASM_LBK_MISC_CFG_LBK_FIFO_CFG . FIFO_FLUSH
 */
#define  VTSS_F_ASM_LBK_MISC_CFG_LBK_FIFO_CFG_FIFO_FLUSH  VTSS_BIT(0)

/**
 * Register Group: \a ASM:LBK_STAT
 *
 * Loopback Block Status registers
 */


/** 
 * \brief Stickybits
 *
 * \details
 * Register: \a ASM:LBK_STAT:LBK_OVFLW_STICKY
 *
 * @param ri Register: LBK_OVFLW_STICKY (??), 0-1
 */
#define VTSS_ASM_LBK_STAT_LBK_OVFLW_STICKY(ri)  VTSS_IOREG(VTSS_TO_ASM,0xe6a + (ri))


/** 
 * \details
 * Register: \a ASM:LBK_STAT:LBK_AGING_STICKY
 *
 * @param ri Register: LBK_AGING_STICKY (??), 0-1
 */
#define VTSS_ASM_LBK_STAT_LBK_AGING_STICKY(ri)  VTSS_IOREG(VTSS_TO_ASM,0xe6c + (ri))

/**
 * Register Group: \a ASM:RAM_CTRL
 *
 * Access core memory
 */


/** 
 * \brief Core reset control
 *
 * \details
 * Controls reset and initialization of the switching core. Proper startup
 * sequence is:
 * - Enable memories
 * - Initialize memories
 * - Enable core
 *
 * Register: \a ASM:RAM_CTRL:RAM_INIT
 */
#define VTSS_ASM_RAM_CTRL_RAM_INIT           VTSS_IOREG(VTSS_TO_ASM,0xe6e)

/** 
 * \brief
 * Initialize core memories. Field is automatically cleared when operation
 * is complete ( approx. 40 us).
 *
 * \details 
 * Field: ::VTSS_ASM_RAM_CTRL_RAM_INIT . RAM_INIT
 */
#define  VTSS_F_ASM_RAM_CTRL_RAM_INIT_RAM_INIT  VTSS_BIT(1)

/** 
 * \brief
 * Core memory controllers are enabled when this field is set.
 *
 * \details 
 * Field: ::VTSS_ASM_RAM_CTRL_RAM_INIT . RAM_ENA
 */
#define  VTSS_F_ASM_RAM_CTRL_RAM_INIT_RAM_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ASM:COREMEM
 *
 * Access core memory
 */


/** 
 * \brief Address selection
 *
 * \details
 * Register: \a ASM:COREMEM:CM_ADDR
 */
#define VTSS_ASM_COREMEM_CM_ADDR             VTSS_IOREG(VTSS_TO_ASM,0xd40)

/** 
 * \brief
 * Please refer to cmid.xls in the AS1000, misc_docs folder.
 *
 * \details 
 * Field: ::VTSS_ASM_COREMEM_CM_ADDR . CM_ID
 */
#define  VTSS_F_ASM_COREMEM_CM_ADDR_CM_ID(x)  VTSS_ENCODE_BITFIELD(x,22,8)
#define  VTSS_M_ASM_COREMEM_CM_ADDR_CM_ID     VTSS_ENCODE_BITMASK(22,8)
#define  VTSS_X_ASM_COREMEM_CM_ADDR_CM_ID(x)  VTSS_EXTRACT_BITFIELD(x,22,8)

/** 
 * \brief
 * Address selection within selected core memory (CMID register). Address
 * is automatically advanced at every data access.
 *
 * \details 
 * Field: ::VTSS_ASM_COREMEM_CM_ADDR . CM_ADDR
 */
#define  VTSS_F_ASM_COREMEM_CM_ADDR_CM_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,22)
#define  VTSS_M_ASM_COREMEM_CM_ADDR_CM_ADDR     VTSS_ENCODE_BITMASK(0,22)
#define  VTSS_X_ASM_COREMEM_CM_ADDR_CM_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,22)


/** 
 * \brief Data register for core memory access.
 *
 * \details
 * Register: \a ASM:COREMEM:CM_DATA
 */
#define VTSS_ASM_COREMEM_CM_DATA             VTSS_IOREG(VTSS_TO_ASM,0xd41)


#endif /* _VTSS_JAGUAR2_REGS_ASM_H_ */
