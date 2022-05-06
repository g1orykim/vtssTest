/*

 Vitesse Switch Software.

==========================================================================

      Vitesse Switch Software

      Register Definitions

==========================================================================
 #####ECOSGPLCOPYRIGHTBEGIN#####
 -------------------------------------------
 This file is part of eCos, the Embedded Configurable Operating System.
 Copyright (C) 1998-2012 Free Software Foundation, Inc.

 eCos is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 or (at your option) any later
 version.

 eCos is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License
 along with eCos; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 As a special exception, if other files instantiate templates or use
 macros or inline functions from this file, or you compile this file
 and link it with other works to produce a work based on this file,
 this file does not by itself cause the resulting work to be covered by
 the GNU General Public License. However the source code for this file
 must still be made available in accordance with section (3) of the GNU
 General Public License v2.

 This exception does not invalidate any other reasons why a work based
 on this file might be covered by the GNU General Public License.
 -------------------------------------------
 #####ECOSGPLCOPYRIGHTEND#####
 ==========================================================================
 #####DESCRIPTIONBEGIN####

 Author(s):    lpovlsen
 Contributors: 
 Date:         2011-10-18
 Purpose:      HAL register definitions
 Description:  This file contains register definitions used by the
               HAL.

 ####DESCRIPTIONEND####

*/
#ifndef _VTSS_JAGUAR_REGS_ASM_H_
#define _VTSS_JAGUAR_REGS_ASM_H_

#include "vtss_jaguar_regs_common.h"

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
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_IN_BYTES_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,0)


/** 
 * \brief Rx Symbol Carrier Error Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SYMBOL_ERR_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SYMBOL_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,1)


/** 
 * \brief Rx Pause Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_PAUSE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_PAUSE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,2)


/** 
 * \brief Rx Control Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_UNSUP_OPCODE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_UNSUP_OPCODE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,3)


/** 
 * \brief Rx OK Byte Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_OK_BYTES_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_OK_BYTES_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,4)


/** 
 * \brief Rx Bad Byte Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_BAD_BYTES_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_BAD_BYTES_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,5)


/** 
 * \brief Rx Unicast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_UC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_UC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,6)


/** 
 * \brief Rx Multicast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_MC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_MC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,7)


/** 
 * \brief Rx Broadcast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_BC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_BC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,8)


/** 
 * \brief Rx CRC Error Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_CRC_ERR_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_CRC_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,9)


/** 
 * \brief Rx Undersize Counter (valid frame format)
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_UNDERSIZE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_UNDERSIZE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,10)


/** 
 * \brief Rx Undersize Counter (CRC error)
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_FRAGMENTS_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_FRAGMENTS_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,11)


/** 
 * \brief Rx In-range Length Error Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_IN_RANGE_LEN_ERR_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_IN_RANGE_LEN_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,12)


/** 
 * \brief Rx Out-Of-Range Length Error Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_OUT_OF_RANGE_LEN_ERR_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_OUT_OF_RANGE_LEN_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,13)


/** 
 * \brief Rx Oversize Counter (valid frame format)
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_OVERSIZE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_OVERSIZE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,14)


/** 
 * \brief Rx Jabbers Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_JABBERS_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_JABBERS_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,15)


/** 
 * \brief Rx 64 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE64_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE64_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,16)


/** 
 * \brief Rx 65-127 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE65TO127_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE65TO127_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,17)


/** 
 * \brief Rx 128-255 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE128TO255_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE128TO255_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,18)


/** 
 * \brief Rx 256-511 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE256TO511_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE256TO511_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,19)


/** 
 * \brief Rx 512-1023 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE512TO1023_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE512TO1023_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,20)


/** 
 * \brief Rx 1024-1518 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE1024TO1518_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE1024TO1518_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,21)


/** 
 * \brief Rx 1519 To Max. Length Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_SIZE1519TOMAX_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_SIZE1519TOMAX_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,22)


/** 
 * \brief Rx Inter Packet Gap Shrink Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:RX_IPG_SHRINK_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_IPG_SHRINK_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,23)


/** 
 * \brief Tx Byte Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_OUT_BYTES_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_OUT_BYTES_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,24)


/** 
 * \brief Tx Pause Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_PAUSE_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_PAUSE_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,25)


/** 
 * \brief Tx OK Byte Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_OK_BYTES_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_OK_BYTES_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,26)


/** 
 * \brief Tx Unicast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_UC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_UC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,27)


/** 
 * \brief Tx Multicast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_MC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_MC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,28)


/** 
 * \brief Tx Broadcast Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_BC_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_BC_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,29)


/** 
 * \brief Tx 64 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE64_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE64_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,30)


/** 
 * \brief Tx 65-127 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE65TO127_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE65TO127_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,31)


/** 
 * \brief Tx 128-255 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE128TO255_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE128TO255_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,32)


/** 
 * \brief Tx 256-511 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE256TO511_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE256TO511_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,33)


/** 
 * \brief Tx 512-1023 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE512TO1023_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE512TO1023_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,34)


/** 
 * \brief Tx 1024-1518 Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE1024TO1518_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE1024TO1518_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,35)


/** 
 * \brief Tx 1519 To Max. Length Byte Frame Counter
 *
 * \details
 * Register: \a ASM:DEV_STATISTICS:TX_SIZE1519TOMAX_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_SIZE1519TOMAX_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,36)


/** 
 * \brief Tx Multi Collision Counter
 *
 * \details
 * Counter collecting the number of frames transmitted without errors after
 * multiple collisions.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_MULTI_COLL_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_MULTI_COLL_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,37)


/** 
 * \brief Tx Late Collision Counter
 *
 * \details
 * Counter collecting the number of late collisions.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_LATE_COLL_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_LATE_COLL_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,38)


/** 
 * \brief Tx Excessive Collision Counter
 *
 * \details
 * Counter collecting the number of frames due to excessive collisions.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_XCOLL_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_XCOLL_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,39)


/** 
 * \brief Tx First Defer Counter
 *
 * \details
 * Counter collecting the number of frames being deferred on first
 * transmission attempt.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_DEFER_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_DEFER_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,40)


/** 
 * \brief Tx Excessive Defer Counter
 *
 * \details
 * Counter collecting the number of frames sent with excessive deferral.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_XDEFER_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_XDEFER_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,41)


/** 
 * \brief Tx 1 Backoff Counter
 *
 * \details
 * Counter collecting the number of frames sent successfully after 1
 * backoff/collision.
 *
 * Register: \a ASM:DEV_STATISTICS:TX_BACKOFF1_CNT
 *
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_BACKOFF1_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,42)


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
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_IN_BYTES_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,44)

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
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_OK_BYTES_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,45)

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
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_RX_BAD_BYTES_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,46)

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
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_OUT_BYTES_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,47)

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
 * @param gi Replicator: x_NUM_DEVS_PER_STATS_BLOCK (??), 0-27
 */
#define VTSS_ASM_DEV_STATISTICS_TX_OK_BYTES_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_ASM,0x0,gi,64,0,48)

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
#define VTSS_ASM_CFG_STAT_CFG                VTSS_IOREG(VTSS_TO_ASM,0x700)

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
 * \brief Cell bus calendar configuration
 *
 * \details
 * The ASM cell bus calendar that is used to control the cell bus bandwidth
 * allocation for each device can be configured via indirect access to the
 * cell bus calendar RAM
 *
 * Register: \a ASM:CFG:CBC_CFG
 *
 * @param ri Replicator: x_ASM_CALENDAR_ENTRIES (??), 0-255
 */
#define VTSS_ASM_CFG_CBC_CFG(ri)             VTSS_IOREG(VTSS_TO_ASM,0x701 + (ri))

/** 
 * \brief
 * This field holds the number of the device group that must be allocated
 * the given cell bus slot. Group 13 is used for allocating the slot for
 * the virtual device.
 *
 * \details 
 * Field: ::VTSS_ASM_CFG_CBC_CFG . DEV_GRP_NUM
 */
#define  VTSS_F_ASM_CFG_CBC_CFG_DEV_GRP_NUM(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_ASM_CFG_CBC_CFG_DEV_GRP_NUM     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_ASM_CFG_CBC_CFG_DEV_GRP_NUM(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Cell bus calendar configuration
 *
 * \details
 * The ASM cell bus calendar that is used to control the cell bus bandwidth
 * allocation for each device can be configured via indirect access to the
 * cell bus calendar RAM
 *
 * Register: \a ASM:CFG:CBC_LEN_CFG
 */
#define VTSS_ASM_CFG_CBC_LEN_CFG             VTSS_IOREG(VTSS_TO_ASM,0x801)

/** 
 * \brief
 * This field indicates the number of valid ASM cell bus calendar entries.
 *
 * \details 
 * 0: The cell bus calendar is ignored. I.e. no data is transmitted at cell
 * bus.
 * 1: Only the first entry (0) is read and all other entries are ignored.
 * 2: Only the first two entries (0->1) are read and all other entries are
 * ignored.
 * X: Only the first X entries (0->X-1) are read and all other entries are
 * ignored.
 *
 * Field: ::VTSS_ASM_CFG_CBC_LEN_CFG . CBC_LEN
 */
#define  VTSS_F_ASM_CFG_CBC_LEN_CFG_CBC_LEN(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ASM_CFG_CBC_LEN_CFG_CBC_LEN     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ASM_CFG_CBC_LEN_CFG_CBC_LEN(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief MAC Address Configuration Register (MSB)
 *
 * \details
 * Register: \a ASM:CFG:MAC_ADDR_HIGH_CFG
 *
 * @param ri Replicator: x_NUM_ETHERNET_PORTS (??), 0-31
 */
#define VTSS_ASM_CFG_MAC_ADDR_HIGH_CFG(ri)   VTSS_IOREG(VTSS_TO_ASM,0x802 + (ri))

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
 * @param ri Replicator: x_NUM_ETHERNET_PORTS (??), 0-31
 */
#define VTSS_ASM_CFG_MAC_ADDR_LOW_CFG(ri)    VTSS_IOREG(VTSS_TO_ASM,0x822 + (ri))

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
 * \brief Ethernet port configuration
 *
 * \details
 * This register holds Ethernet port configuration bit groups
 *
 * Register: \a ASM:CFG:ETH_CFG
 *
 * @param ri Replicator: x_NUM_ETHERNET_PORTS (??), 0-31
 */
#define VTSS_ASM_CFG_ETH_CFG(ri)             VTSS_IOREG(VTSS_TO_ASM,0x842 + (ri))

/** 
 * \brief
 * This field determines if the ASM must abort mark frames that become
 * older than 16-24 ms before and EOF is received.
 *
 * \details 
 * '0': Aging enabled.
 * '1': Aging disabled.
 *
 * Field: ::VTSS_ASM_CFG_ETH_CFG . ETH_FRM_AGING_DIS
 */
#define  VTSS_F_ASM_CFG_ETH_CFG_ETH_FRM_AGING_DIS  VTSS_BIT(8)

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
 * Field: ::VTSS_ASM_CFG_ETH_CFG . ETH_PAD_ENA
 */
#define  VTSS_F_ASM_CFG_ETH_CFG_ETH_PAD_ENA   VTSS_BIT(4)

/** 
 * \brief
 * This field determines if the ASM must insert a dummy tag after the SMAC.
 *
 * \details 
 * Field: ::VTSS_ASM_CFG_ETH_CFG . ETH_DTAG_ENA
 */
#define  VTSS_F_ASM_CFG_ETH_CFG_ETH_DTAG_ENA  VTSS_BIT(2)

/** 
 * \brief
 * This field determines if frames are assumed to be received with a
 * prepended HIH to be relocated to the internal frame header (IFH).
 *
 * \details 
 * Field: ::VTSS_ASM_CFG_ETH_CFG . ETH_PRE_MODE
 */
#define  VTSS_F_ASM_CFG_ETH_CFG_ETH_PRE_MODE  VTSS_BIT(1)

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
 *
 * \details 
 * 0: Vstax2 awareness is disabled. 
 * 1: Vstax2 awareness is enabled.
 *
 * Field: ::VTSS_ASM_CFG_ETH_CFG . ETH_VSTAX2_AWR_ENA
 */
#define  VTSS_F_ASM_CFG_ETH_CFG_ETH_VSTAX2_AWR_ENA  VTSS_BIT(0)


/** 
 * \brief This register holds CPU specific configuration
 *
 * \details
 * Register: \a ASM:CFG:CPU_CFG
 *
 * @param ri Replicator: x_ASM_NUM_OF_CPU_CH (??), 0-1
 */
#define VTSS_ASM_CFG_CPU_CFG(ri)             VTSS_IOREG(VTSS_TO_ASM,0x862 + (ri))

/** 
 * \brief
 * This field determines if the ASM must check that the IFH checksum is
 * correct before changing the existing IFH and update the checksum.
 *
 * \details 
 * '0': The checksum of the received IFH is not checked.
 * '1': The checksum of the received IFH is checked.
 *
 * Field: ::VTSS_ASM_CFG_CPU_CFG . CPU_CHK_IFH_CHKSUM_ENA
 */
#define  VTSS_F_ASM_CFG_CPU_CFG_CPU_CHK_IFH_CHKSUM_ENA  VTSS_BIT(8)

/** 
 * \brief
 * This field determines if the ASM must abort mark frames that become
 * older than 16-24 ms before and EOF is received.
 *
 * \details 
 * '0': Aging enabled.
 * '1': Aging disabled.
 *
 * Field: ::VTSS_ASM_CFG_CPU_CFG . CPU_FRM_AGING_DIS
 */
#define  VTSS_F_ASM_CFG_CPU_CFG_CPU_FRM_AGING_DIS  VTSS_BIT(4)

/** 
 * \brief
 * This field determines if the ASM must zero-pad Ethernet frames that are
 * less than 64 bytes (after a FCS field optionally has been added).
 *
 * \details 
 * '0': Padding is disabled. Frames that are less than 64 bytes and have
 * not been abort marked are passed to the ANA block 'as is'. Frames that
 * are less than 64 bytes and have been abort marked are normally discarded
 * silently by the ASM.
 * '1': Padding is enabled. If the resulting frame size (after an
 * optionally FCS field has been added) will be less than 64 bytes, the
 * frame is zero-padded, so that the resulting frame size is 64 bytes.
 *
 * Field: ::VTSS_ASM_CFG_CPU_CFG . CPU_PAD_ENA
 */
#define  VTSS_F_ASM_CFG_CPU_CFG_CPU_PAD_ENA   VTSS_BIT(0)


/** 
 * \brief Holds configuration related to Pause frame detection.
 *
 * \details
 * This register control whether pause and control frames should be
 * forwarded or terminated at ingress.
 *
 * Register: \a ASM:CFG:PAUSE_CFG
 *
 * @param ri Replicator: x_NUM_ETHERNET_PORTS (??), 0-31
 */
#define VTSS_ASM_CFG_PAUSE_CFG(ri)           VTSS_IOREG(VTSS_TO_ASM,0x865 + (ri))

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
 * Register Group: \a ASM:CPU_STATUS
 *
 * Status for CPU ingress ports
 */


/** 
 * \brief DEVCPU port/channel sticky bits
 *
 * \details
 * This register holds all the DEVCPU sticky bits that exists for each
 * port/channel.
 *
 * Register: \a ASM:CPU_STATUS:CPU_CH_STICKY
 *
 * @param ri Replicator: x_ASM_NUM_OF_CPU_CH (??), 0-1
 */
#define VTSS_ASM_CPU_STATUS_CPU_CH_STICKY(ri)  VTSS_IOREG(VTSS_TO_ASM,0x885 + (ri))

/** 
 * \brief
 * This field indicates if one or more CPU frames have been discarded due
 * to aging.
 *
 * \details 
 * '0': No CPU frames have been discarded due to aging.
 * '1': One or more CPU frames have been discarded due to aging.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_CPU_STATUS_CPU_CH_STICKY . CPU_FRM_AGING_STICKY
 */
#define  VTSS_F_ASM_CPU_STATUS_CPU_CH_STICKY_CPU_FRM_AGING_STICKY  VTSS_BIT(0)

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
#define VTSS_ASM_DBG_DBG_CFG                 VTSS_IOREG(VTSS_TO_ASM,0x887)


/** 
 * \brief Holds a number of sticky bits that are set if internal errors are detected.
 *
 * \details
 * Writing a '1' to a bit group clears that bit.
 *
 * Register: \a ASM:DBG:ERR_STICKY
 *
 * @param ri Replicator: x_NUM_OF_TAXI (??), 0-6
 */
#define VTSS_ASM_DBG_ERR_STICKY(ri)          VTSS_IOREG(VTSS_TO_ASM,0x888 + (ri))

/** 
 * \brief
 * A sticky bit is asserted, if the ASM receives a frame together with an
 * IFH and the IFH checksum is incorrect - given that IFH checksum check is
 * enabled. This only applies for CPU injected frames.
 *
 * \details 
 * '0': No IFH checksum errors detected - or IFH checksum check is
 * disabled.
 * '1': One or more IFH checksum errors have been detected and the IFH
 * checksum check is enabled.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_DBG_ERR_STICKY . IFH_CHKSUM_ERR_STICKY
 */
#define  VTSS_F_ASM_DBG_ERR_STICKY_IFH_CHKSUM_ERR_STICKY  VTSS_BIT(13)

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
 * \brief Register containing sticky bits for pre counter overflow
 *
 * \details
 * Register containing sticky bits for pre counter overflow
 *
 * Register: \a ASM:DBG:PRE_CNT_OFLW_STICKY
 */
#define VTSS_ASM_DBG_PRE_CNT_OFLW_STICKY     VTSS_IOREG(VTSS_TO_ASM,0x88f)


/** 
 * \brief Ram integrity errors 
 *
 * \details
 * Register: \a ASM:DBG:RAM_INTEGRITY_ERR_STICKY
 */
#define VTSS_ASM_DBG_RAM_INTEGRITY_ERR_STICKY  VTSS_IOREG(VTSS_TO_ASM,0x890)

/** 
 * \brief
 * Integrity error indication for RAMs used in ASM.
 * Bit is cleared by writing a '1' to the respective position.
 * '0': No RAM integrity check error occurred
 * '1': RAM integrity check error occurred
 * Bit 0: Cell Bus Calendar RAM
 * Bit 1: Device statistics RAM
 * Remaining bits: Reserved
 *
 * \details 
 * Field: ::VTSS_ASM_DBG_RAM_INTEGRITY_ERR_STICKY . RAM_INTEGRITY_ERROR_STICKY
 */
#define  VTSS_F_ASM_DBG_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERROR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ASM_DBG_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERROR_STICKY     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ASM_DBG_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERROR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a ASM:ETH_STATUS
 *
 * Status for ETH ingress ports
 */


/** 
 * \brief Ethernet port sticky bits
 *
 * \details
 * This register holds all the Ethernet port sticky bits that exists for
 * each port.
 *
 * Register: \a ASM:ETH_STATUS:ETH_CH_STICKY
 *
 * @param ri Replicator: x_NUM_ETHERNET_PORTS (??), 0-31
 */
#define VTSS_ASM_ETH_STATUS_ETH_CH_STICKY(ri)  VTSS_IOREG(VTSS_TO_ASM,0x891 + (ri))

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
 * Field: ::VTSS_ASM_ETH_STATUS_ETH_CH_STICKY . ETH_FRM_AGING_STICKY
 */
#define  VTSS_F_ASM_ETH_STATUS_ETH_CH_STICKY_ETH_FRM_AGING_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ASM:CM_CFG
 *
 * VStax2 Congestion Management configuration registers
 */


/** 
 * \brief Enable reception of CMEFs
 *
 * \details
 * Enable reception of CMEFs
 * The address of the configuration is based on the following layout:
 *  0: Configuration for physical port number 24
 *  1: Configuration for physical port number 25
 *  2: Configuration for physical port number 26
 *  3: Configuration for physical port number 27
 *  4: Configuration for physical port number 28
 *  5: Configuration for physical port number 29
 *  6: Configuration for physical port number 30
 *
 * Register: \a ASM:CM_CFG:CMEF_RX_CFG
 *
 * @param ri Replicator: x_NUM_POT_STACK_PORTS (??), 0-6
 */
#define VTSS_ASM_CM_CFG_CMEF_RX_CFG(ri)      VTSS_IOREG(VTSS_TO_ASM,0x8b1 + (ri))

/** 
 * \brief
 * Enable forwarding of CMEFs on cell bus.
 *
 * \details 
 * '0': CMEFs are not forwarded on cell bus.
 * '1': CMEFs are forwarded on cell bus.
 *
 * Field: ::VTSS_ASM_CM_CFG_CMEF_RX_CFG . CMEF_FWD_CELL_BUS_ENA
 */
#define  VTSS_F_ASM_CM_CFG_CMEF_RX_CFG_CMEF_FWD_CELL_BUS_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable reception of CMEFs to be forwarded to DSM.
 * 
 * Note: If CMEFs should be relayed, please enable the respective port in
 * the DSM: CMEF_GEN_CFG::CMEF_RELAY_TX_ENA.
 * 
 * Note: If the device supports Super-Priority, SP_RX_CFG::SP_RX_SEL is
 * responsible to select the relay buffer, CMEFs are forwarded to.
 * 
 * Note: If the device does not support Super-Priority,
 * CMEF_RX_CFG::CMEF_FWD_NUF_SEL is responsible to select the relay buffer,
 * CMEFs are forwarded to.
 *
 * \details 
 * '0': CMEFs are discarded
 * '1': CMEFs are forwarded to DSM
 *
 * Field: ::VTSS_ASM_CM_CFG_CMEF_RX_CFG . CMEF_RX_ENA
 */
#define  VTSS_F_ASM_CM_CFG_CMEF_RX_CFG_CMEF_RX_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ASM:CM_STATUS
 *
 * VStax2 Congestion Management status and sticky bit registers
 */


/** 
 * \brief CMEF counter
 *
 * \details
 * CMEF counter
 *
 * Register: \a ASM:CM_STATUS:CMEF_CNT
 *
 * @param ri Replicator: x_NUM_128_BIT_SP_FWDS_TO_DSM (??), 0-1
 */
#define VTSS_ASM_CM_STATUS_CMEF_CNT(ri)      VTSS_IOREG(VTSS_TO_ASM,0x8b8 + (ri))

/** 
 * \brief
 * Count the number of all CMEFs received.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_ASM_CM_STATUS_CMEF_CNT . CMEF_CNT
 */
#define  VTSS_F_ASM_CM_STATUS_CMEF_CNT_CMEF_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ASM_CM_STATUS_CMEF_CNT_CMEF_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ASM_CM_STATUS_CMEF_CNT_CMEF_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief CMEF discard sticky bits
 *
 * \details
 * CMEF discard sticky bits
 *
 * Register: \a ASM:CM_STATUS:CMEF_DISCARD_STICKY
 */
#define VTSS_ASM_CM_STATUS_CMEF_DISCARD_STICKY  VTSS_IOREG(VTSS_TO_ASM,0x8ba)

/** 
 * \brief
 * Detect discarded CMEFs
 *
 * \details 
 * '0': No CMEF has been discarded.
 * '1': One or more CMEFs have been dicarded.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_CM_STATUS_CMEF_DISCARD_STICKY . CMEF_DISCARD_STICKY
 */
#define  VTSS_F_ASM_CM_STATUS_CMEF_DISCARD_STICKY_CMEF_DISCARD_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ASM_CM_STATUS_CMEF_DISCARD_STICKY_CMEF_DISCARD_STICKY     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ASM_CM_STATUS_CMEF_DISCARD_STICKY_CMEF_DISCARD_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a ASM:SP_CFG
 *
 * Configuration registers for Super-Priority
 */


/** 
 * \brief Super-Priority reception configuration
 *
 * \details
 * Super-Priority reception configuration registers.
 * 
 * These registers are replicated once for each of the potential stack
 * ports.
 * The address of the configuration is based on the following layout:
 *  0: Configuration for physical port number 24
 *  1: Configuration for physical port number 25
 *  2: Configuration for physical port number 26
 *  3: Configuration for physical port number 27
 *  4: Configuration for physical port number 28
 *  5: Configuration for physical port number 29
 *  6: Configuration for physical port number 30
 *
 * Register: \a ASM:SP_CFG:SP_RX_CFG
 *
 * @param ri Replicator: x_NUM_POT_STACK_PORTS (??), 0-6
 */
#define VTSS_ASM_SP_CFG_SP_RX_CFG(ri)        VTSS_IOREG(VTSS_TO_ASM,0x8bb + (ri))

/** 
 * \brief
 * Enable copying of super-priority frames to the cell bus (even if
 * SP_RX_ENA=1).

 *
 * \details 
 * Field: ::VTSS_ASM_SP_CFG_SP_RX_CFG . SP_FWD_CELL_BUS_ENA
 */
#define  VTSS_F_ASM_SP_CFG_SP_RX_CFG_SP_FWD_CELL_BUS_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Selects which DSM relay and extraction buffer SP frames from the stack
 * port shall be received in.
 * 
 * DSM has two relay buffers and two extraction buffers. 
 * 
 * For a configuration with two stack ports, one stack port must receive SP
 * frames in DSM relay/extraction buffer 0 and the other stack port must
 * receive SP frames in DSM relay/extraction buffer 1.
 * 
 * In DSM the stack ports must be configured to transmit from the relay
 * buffer the other stack port uses for reception.
 * 
 * Note: This bit is also responsible to select the CMEF relay buffer CMEFs
 * are received in.
 * 
 * Related registers:
 * DSM::SP_TX_CFG.
 *
 * \details 
 * '0': Forward SP/CM traffic to DSM relay/extraction buffer 0
 * '1': Forward SP/CM traffic to DSM relay/extraction buffer 1

 *
 * Field: ::VTSS_ASM_SP_CFG_SP_RX_CFG . SP_RX_SEL
 */
#define  VTSS_F_ASM_SP_CFG_SP_RX_CFG_SP_RX_SEL  VTSS_BIT(1)

/** 
 * \brief
 * Enable recognition of super-priority frames.
 * 
 * If this bit is not set, super-priority frames will be forwarded to the
 * cell bus, similarly to all other frames.
 * 
 * If this bit is set, super-priority frames will be forwarded to DSM and
 * from there forwarded onto the other stack port and/or to the CPU.
 * 
 * Related registers:
 * DSM::SP_TX_CFG.
 *
 * \details 
 * Field: ::VTSS_ASM_SP_CFG_SP_RX_CFG . SP_RX_ENA
 */
#define  VTSS_F_ASM_SP_CFG_SP_RX_CFG_SP_RX_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration register for own UPSID
 *
 * \details
 * Configuration register for own UPSID
 *
 * Register: \a ASM:SP_CFG:SP_UPSID_CFG
 */
#define VTSS_ASM_SP_CFG_SP_UPSID_CFG         VTSS_IOREG(VTSS_TO_ASM,0x8c2)

/** 
 * \brief
 * Configuration of own UPSID.
 * 
 * This must be configured consistently across the following registers:
 * ANA_AC::COMMON_VSTAX_CFG.OWN_UPSID
 * ANA_L2::VSTAX_CTRL.OWN_UPSID
 * ASM::SP_UPSID_CFG.OWN_UPSID
 *
 * \details 
 * value represents own upsid
 *
 * Field: ::VTSS_ASM_SP_CFG_SP_UPSID_CFG . OWN_UPSID
 */
#define  VTSS_F_ASM_SP_CFG_SP_UPSID_CFG_OWN_UPSID(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_ASM_SP_CFG_SP_UPSID_CFG_OWN_UPSID     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_ASM_SP_CFG_SP_UPSID_CFG_OWN_UPSID(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Configuration register for SP TTL check
 *
 * \details
 * Configuration register for SP TTL check
 *
 * Register: \a ASM:SP_CFG:SP_KEEP_TTL_CFG
 */
#define VTSS_ASM_SP_CFG_SP_KEEP_TTL_CFG      VTSS_IOREG(VTSS_TO_ASM,0x8c3)

/** 
 * \brief
 * If this bit is set, Super-Priority frames with TTL==1 are forwarded to
 * the relay buffer as well.
 * 
 * This must be configured consistently across the following registers:
 * DSM::SP_KEEP_TTL_CFG.SP_KEEP_TTL_ENA
 * ASM::SP_KEEP_TTL_CFG.SP_KEEP_TTL_ENA

 *
 * \details 
 * '0': Super-Priority frames with TTL==1 are filtered for relay
 * '1': Super-Priority frames with TTL==1 are forwarded to relay buffer
 *
 * Field: ::VTSS_ASM_SP_CFG_SP_KEEP_TTL_CFG . SP_KEEP_TTL_ENA
 */
#define  VTSS_F_ASM_SP_CFG_SP_KEEP_TTL_CFG_SP_KEEP_TTL_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ASM:SP_STATUS
 *
 * Super-Priority status and sticky bit registers
 */


/** 
 * \brief Super-Priority sticky registers
 *
 * \details
 * Super-Priority sticky registers
 *
 * Register: \a ASM:SP_STATUS:SP_STICKY
 */
#define VTSS_ASM_SP_STATUS_SP_STICKY         VTSS_IOREG(VTSS_TO_ASM,0x8c4)

/** 
 * \brief
 * A Super-Priority frame has been dropped due to:
 *  ttl=1 and fwd_mode=fwd_gcpu_ups and dst_upsid/=own_upsid
 *
 * \details 
 * '0': A Super-Priority frame has not been dropped
 * '1': A Super-Priority frame has been dropped
 * One bit per relay/extract fwd bus.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ASM_SP_STATUS_SP_STICKY . SP_DROP_STICKY
 */
#define  VTSS_F_ASM_SP_STATUS_SP_STICKY_SP_DROP_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ASM_SP_STATUS_SP_STICKY_SP_DROP_STICKY     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ASM_SP_STATUS_SP_STICKY_SP_DROP_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


#endif /* _VTSS_JAGUAR_REGS_ASM_H_ */
