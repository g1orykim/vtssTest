#ifndef _VTSS_JAGUAR_REGS_IQS_H_
#define _VTSS_JAGUAR_REGS_IQS_H_

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

#include "vtss_jaguar_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a IQS
 *
 * Inbound Queue System
 *
 ***********************************************************************/

/**
 * Register Group: \a IQS:CONG_CTRL
 *
 * Congestion handling as flow-control vs. dropping
 */


/** 
 * \brief Congestion handling configuration
 *
 * \details
 * Congestion handling:
 * - Frame/fragment deletion in RX direction
 * - ATOP drop disable
 *
 * Register: \a IQS:CONG_CTRL:CH_CFG
 */
#define VTSS_IQS_CONG_CTRL_CH_CFG            VTSS_IOREG(VTSS_TO_IQS,0x5300)

/** 
 * \brief
 * Enable Discard Eligibility based on Drop Precedence Level.
 * One bit per DP level.
 *
 * \details 
 * '0': DE marking for the respective DP level is disabled
 * '1': DE marking for the respective DP level is enabled
 *
 * Field: ::VTSS_IQS_CONG_CTRL_CH_CFG . DE_ENA
 */
#define  VTSS_F_IQS_CONG_CTRL_CH_CFG_DE_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_IQS_CONG_CTRL_CH_CFG_DE_ENA     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_IQS_CONG_CTRL_CH_CFG_DE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Method of cell deletion:
 * Specify whether to delete only entire failed frames or whether to delete
 * all cells that are marked as invalid.
 * Related bitgroup is FAILED_FRAME_DELETE
 * 
 * Note that if a frame is partially deleted on RX and partially deleted on
 * TX then the statistic counters on RX will count all cells of that frame
 * even though some are transmitted to (and discarded on) the TX side.
 *
 * \details 
 * '0' only delete entire frames on RX side
 * '1' delete all cells that are marked as invalid on RX side
 *
 * Field: ::VTSS_IQS_CONG_CTRL_CH_CFG . FRAGMENT_CUT
 */
#define  VTSS_F_IQS_CONG_CTRL_CH_CFG_FRAGMENT_CUT  VTSS_BIT(2)

/** 
 * \brief
 * If FAILED_FRAME_DELETE is disabled no frame will be deleted; if
 * FAILED_FRAME_DELETE is enabled it belongs to the setting of
 * 'FRAGMENT_CUT' whether to delete all invalid cells or only entire
 * invalid frames on the RX side.
 *
 * \details 
 * '0' do not delete the failed frames/cells
 * '1' allow deletion of failed frames/cells
 *
 * Field: ::VTSS_IQS_CONG_CTRL_CH_CFG . FAILED_FRM_DEL
 */
#define  VTSS_F_IQS_CONG_CTRL_CH_CFG_FAILED_FRM_DEL  VTSS_BIT(1)

/** 
 * \brief
 * Disable dropping of frames when ATOP is reached.
 * If HWM = ATOP = maximum value then neither FC mode nor drop mode is
 * enabled.
 * This affects all levels of the flow control hierarchies.
 *
 * \details 
 * '0': enable dropping of frames when ATOP is reached
 * '1': disable dropping of frames when ATOP is reached
 *
 * Field: ::VTSS_IQS_CONG_CTRL_CH_CFG . ATOP_DROP_DISABLE
 */
#define  VTSS_F_IQS_CONG_CTRL_CH_CFG_ATOP_DROP_DISABLE  VTSS_BIT(0)

/**
 * Register Group: \a IQS:MTU
 *
 * Configuration and mapping of preallocated frame size
 */


/** 
 * \brief Pre-allocated frame size
 *
 * \details
 * Size that is pre-allocated for each frame per queue. A high value leads
 * to earlier reaching a watermark (threshold).
 * Note that size 0 is not allowed.
 *
 * Register: \a IQS:MTU:MTU_FRM_SIZE_CFG
 *
 * @param ri Replicator: x_QS_NUM_WORDS_FOR_PRE_ALLOC_SIZES (??), 0-3
 */
#define VTSS_IQS_MTU_MTU_FRM_SIZE_CFG(ri)    VTSS_IOREG(VTSS_TO_IQS,0x5301 + (ri))

/** 
 * \brief
 * Number of cells that is pre-allocated for each frame that will be
 * received. Each queue can be mapped to one value used as pre-allocation
 * size.
 * Note that size 0 is not allowed.
 *
 * \details 
 * Number of preallocated segments (cells)
 *
 * Field: ::VTSS_IQS_MTU_MTU_FRM_SIZE_CFG . PRE_ALLOC_FRM_SIZE
 */
#define  VTSS_F_IQS_MTU_MTU_FRM_SIZE_CFG_PRE_ALLOC_FRM_SIZE(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_IQS_MTU_MTU_FRM_SIZE_CFG_PRE_ALLOC_FRM_SIZE     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_IQS_MTU_MTU_FRM_SIZE_CFG_PRE_ALLOC_FRM_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a IQS:QU_RAM_CFG
 *
 * Combined configuration bitgroups located in one RAM
 */


/** 
 * \brief Resource control configuration on queue level
 *
 * \details
 * RC mode configuration (normal/minimum-guaranteed)
 * Cell counter enable on port level
 * Cell counter enable on buffer level
 * ATOP for resource control on queue level
 *
 * Register: \a IQS:QU_RAM_CFG:QU_RC_CFG
 *
 * @param ri Replicator: x_QS_NUM_QUEUES (??), 0-279
 */
#define VTSS_IQS_QU_RAM_CFG_QU_RC_CFG(ri)    VTSS_IOREG(VTSS_TO_IQS,0x6000 + (ri))

/** 
 * \brief
 * The forward pressure threshold defines how many cells may reside in the
 * resp. queue without being scheduled, e.g. a value of 0 disables this
 * feature while a value N allows to keep N cells permanently in that
 * queue.
 * Note that the decision whether to schedule cells or not is finally made
 * in the scheduling block.

 *
 * \details 
 * 0: feature is disabled
 * N: signal the scheduling block whether the queue contains more than N
 * cells or not.
 * 
 * If used as CM_QUEUE_WM:
 * Being above the WM activates congestion notification.
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_CFG . FWD_PRESS_THRES
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_CFG_FWD_PRESS_THRES(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_QU_RAM_CFG_QU_RC_CFG_FWD_PRESS_THRES     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_QU_RAM_CFG_QU_RC_CFG_FWD_PRESS_THRES(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * Each queue can be enabled to have its received cells counted
 * additionally on priority level.
 *
 * \details 
 * '0': disable queue for cell counting on priorty level
 * '1': enable queue for cell counting on priority level
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_CFG . QU_CNT_ON_PRIO_LVL_ENA
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_PRIO_LVL_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Each queue can be enabled to have its received cells counted
 * additionally on port level.
 *
 * \details 
 * '0': disable queue for cell counting on port level
 * '1': enable queue for cell counting on port level
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_CFG . QU_CNT_ON_PORT_LVL_ENA
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_PORT_LVL_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Each queue can be enabled to have its received cells counted
 * additionally on priority related buffer level.
 * Note that if a buffer level memory allocation counter is enabled for a
 * priority then it is disabled for the queues of that priority regardless
 * of this enable bit.
 *
 * \details 
 * '0': disable queue for cell counting on priority related buffer level
 * '1': enable queue for cell counting on priority related buffer level
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_CFG . QU_CNT_ON_BUF_PRIO_LVL_ENA
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_BUF_PRIO_LVL_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Each queue can be enabled to have its received cells counted
 * additionally on buffer level.
 * Note that if a buffer level memory allocation counter is enabled for a
 * port then it is disabled for the queues of that port regardless of this
 * enable bit.
 *
 * \details 
 * '0': disable queue for cell counting on buffer level
 * '1': enable queue for cell counting on buffer level
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_CFG . QU_CNT_ON_BUF_PORT_LVL_ENA
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_CFG_QU_CNT_ON_BUF_PORT_LVL_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Choose between resource control modes 'normal' and 'minimum guaranteed'
 * for all queues of the resp. buffer.
 *
 * \details 
 * '0' normal mode
 * '1' minimum guaranteed mode
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_CFG . QU_RC_MODE
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_CFG_QU_RC_MODE  VTSS_BIT(2)

/** 
 * \brief
 * Select between FC mode and drop mode for the resp. queue.
 *
 * \details 
 * '0': FC is enabled
 * '1': Drop mode is enabled
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_CFG . QU_FC_DROP_MODE
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_CFG_QU_FC_DROP_MODE  VTSS_BIT(1)

/** 
 * \brief
 * Queue enable/disable on RX side
 *
 * \details 
 * '0': disable RX queue
 * '1': enable RX queue
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_CFG . RX_QU_ENA
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_CFG_RX_QU_ENA  VTSS_BIT(0)


/** 
 * \brief High and low watermarks for queues
 *
 * \details
 * HWM and LWM for resource control on queue level
 *
 * Register: \a IQS:QU_RAM_CFG:QU_RC_HLWM_CFG
 *
 * @param ri Replicator: x_QS_NUM_QUEUES (??), 0-279
 */
#define VTSS_IQS_QU_RAM_CFG_QU_RC_HLWM_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x6200 + (ri))

/** 
 * \brief
 * HWM threshold used in profiles for queues
 * Note that HWM < LWM is not allowed and leeds to an improper behaviour of
 * the QS.
 *
 * \details 
 * Number of cells per queue
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_HLWM_CFG . QU_RC_PROFILE_HWM
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_HWM(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_HWM     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_HWM(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * LWM threshold used in profiles for queues
 * Note that HWM < LWM is not allowed and leeds to an improper behaviour of
 * the QS.
 *
 * \details 
 * Number of cells per queue
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_HLWM_CFG . QU_RC_PROFILE_LWM
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_LWM(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_LWM     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_QU_RAM_CFG_QU_RC_HLWM_CFG_QU_RC_PROFILE_LWM(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief ATOP and SWM for queue configuration
 *
 * \details
 * This register allows to configure the over-all top threshold ATOP and
 * the spill-over watermark SWM on a per queue basis.
 *
 * Register: \a IQS:QU_RAM_CFG:QU_RC_ATOP_SWM_CFG
 *
 * @param ri Replicator: x_QS_NUM_QUEUES (??), 0-279
 */
#define VTSS_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x6400 + (ri))

/** 
 * \brief
 * ATOP threshold used in profiles for queues
 *
 * \details 
 * Number of cells per queue
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG . QU_RC_PROFILE_ATOP
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_ATOP(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_ATOP     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_ATOP(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * SWM (spill over watermark) threshold used in profiles for queues to
 * determine the number of cells guaranteed per queue.
 *
 * \details 
 * Number of cells per queue
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG . QU_RC_PROFILE_SWM
 */
#define  VTSS_F_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_SWM(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_SWM     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_QU_RAM_CFG_QU_RC_ATOP_SWM_CFG_QU_RC_PROFILE_SWM(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief Mapping of MTUs to queues
 *
 * \details
 * Mapping of each queue to one of all specified maximum transmission units
 * (MTU)
 *
 * Register: \a IQS:QU_RAM_CFG:MTU_QU_MAP_CFG
 *
 * @param ri Replicator: x_QS_NUM_QUEUES (??), 0-279
 */
#define VTSS_IQS_QU_RAM_CFG_MTU_QU_MAP_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x6600 + (ri))

/** 
 * \brief
 * Mapping to 1 of the maximum transmission unit values
 * "0...0" is used to map to the 1st MTU value
 * "1...1" is used to map to the last MTU value
 * Refer to bitgroup PRE_ALLOC_FRAME_SIZE
 *
 * \details 
 *   "...00" use PRE_ALLOC_FRAME_SIZE 0
 *   "...01" use PRE_ALLOC_FRAME_SIZE 1
 *   "...10" use PRE_ALLOC_FRAME_SIZE 2
 *   "...11" use PRE_ALLOC_FRAME_SIZE 3
 * ...
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_MTU_QU_MAP_CFG . MTU_PRE_ALLOC
 */
#define  VTSS_F_IQS_QU_RAM_CFG_MTU_QU_MAP_CFG_MTU_PRE_ALLOC(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_IQS_QU_RAM_CFG_MTU_QU_MAP_CFG_MTU_PRE_ALLOC     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_IQS_QU_RAM_CFG_MTU_QU_MAP_CFG_MTU_PRE_ALLOC(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Configuration bits for Minth and MaxTh for RED and ECN
 *
 * \details
 * Entire configuration for RED and ECN consists of:
 * - ECN enable
 * - WQ
 * - MaxP
 * - Multiplier for Minth and (MaxTh - MinTh)
 * - (MaxTh - MinTh)
 * - MinTh
 * - RED enable
 *
 * Register: \a IQS:QU_RAM_CFG:RED_ECN_MINTH_MAXTH_CFG
 *
 * @param ri Replicator: x_QS_NUM_QUEUES (??), 0-279
 */
#define VTSS_IQS_QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x6800 + (ri))

/** 
 * \brief
 * High threshold minus low threshold for RED drop probability (MaxTh -
 * MinTh). If the average fill level is below MaxTh (and above MinTh) then
 * the drop probability depends on MaxP. If high threshold (MaxTh) is
 * reached, then the drop probability is 1..
 *
 * \details 
 * Measured in number of cells
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG . RED_ECN_PROFILE_MINMAXTH
 */
#define  VTSS_F_IQS_QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG_RED_ECN_PROFILE_MINMAXTH(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_IQS_QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG_RED_ECN_PROFILE_MINMAXTH     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_IQS_QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG_RED_ECN_PROFILE_MINMAXTH(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Low threshold for RED drop probability (MinTh). If reached or exceeded,
 * the drop probability is > 0 depending on MaxP
 *
 * \details 
 * Measured in number of cells
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG . RED_ECN_PROFILE_MINTH
 */
#define  VTSS_F_IQS_QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG_RED_ECN_PROFILE_MINTH(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_IQS_QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG_RED_ECN_PROFILE_MINTH     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_IQS_QU_RAM_CFG_RED_ECN_MINTH_MAXTH_CFG_RED_ECN_PROFILE_MINTH(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Miscellaneous configuration bits for RED and ECN
 *
 * \details
 * Entire configuration for RED and ECN:
 * - ECN enable
 * - WQ
 * - MaxP
 * - Multiplier for Minth and (MaxTh - MinTh)
 * - (MaxTh - MinTh)
 * - MinTh
 * - RED enable
 *
 * Register: \a IQS:QU_RAM_CFG:RED_ECN_MISC_CFG
 *
 * @param ri Replicator: x_QS_NUM_QUEUES (??), 0-279
 */
#define VTSS_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x6a00 + (ri))

/** 
 * \brief
 * Threshold for drop probability in RED mode. 1 to n thresholds (chip
 * depending) are combined in this bitgroup, each is 8 bit wide.
 *
 * \details 
 * 0: No frame is dropped
 * 1: Drop probability is 1/256 (0.4%) when average queue size almost
 * reaches MaxTh
 * 2: Drop probability is 2/256 (0.8%) when average queue size almost
 * reaches MaxTh
 * (...)
 * 255: Drop probability is 255/256 (99.6%) when average queue size almost
 * reaches MaxTh
 * 
 * Note: There should never be a reason to set MaxP greater than 10% (26).
 * A reasonable value is in the range of 1% to 2% (2 to 5).
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG . RED_ECN_PROF_MAXP
 */
#define  VTSS_F_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG_RED_ECN_PROF_MAXP(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG_RED_ECN_PROF_MAXP     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG_RED_ECN_PROF_MAXP(x)  VTSS_EXTRACT_BITFIELD(x,8,24)

/** 
 * \brief
 * Shift left (multiply) the configured values for MinTh and MinMaxTh by
 * RED_THRESH_GAIN to increase their range.
 *
 * \details 
 * 0: multiply Minth and MinMaxTh by 1
 * 1: multiply Minth and MinMaxTh by 2
 * ...
 * n: multiply Minth and MinMaxTh by 2^n
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG . RED_THRES_GAIN
 */
#define  VTSS_F_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG_RED_THRES_GAIN(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG_RED_THRES_GAIN     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG_RED_THRES_GAIN(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * RED mode switch
 *
 * \details 
 * '0': disables RED mode
 * '1': enables RED mode
 *
 * Field: ::VTSS_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG . RED_ENA
 */
#define  VTSS_F_IQS_QU_RAM_CFG_RED_ECN_MISC_CFG_RED_ENA  VTSS_BIT(0)

/**
 * Register Group: \a IQS:PORT_RAM_CFG
 *
 * Combined configuration bitgroups located in one RAM
 */


/** 
 * \brief Resource control configuration on port level
 *
 * \details
 * RX enable
 * Drop / non-drop mode
 * RC mode configuration (normal/minimum-guaranteed)
 * Cell counter enable on buffer level
 * ATOP for resource control on buffer level
 *
 * Register: \a IQS:PORT_RAM_CFG:PORT_RC_CFG
 *
 * @param ri Replicator: x_QS_NUM_PORTS (??), 0-34
 */
#define VTSS_IQS_PORT_RAM_CFG_PORT_RC_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5200 + (ri))

/** 
 * \brief
 * Each port can be enabled to have its received cells counted additionally
 * on buffer level.Note that if a buffer level memory allocation counter is
 * enabled for a port then it is disabled for the queues of that port
 * regardless of the setting of the resp. queue enable bit.
 *
 * \details 
 * '0': disable port for cell counting on buffer level
 * '1': enable port for cell counting on buffer level
 *
 * Field: ::VTSS_IQS_PORT_RAM_CFG_PORT_RC_CFG . PORT_CNT_ON_BUF_LVL_ENA
 */
#define  VTSS_F_IQS_PORT_RAM_CFG_PORT_RC_CFG_PORT_CNT_ON_BUF_LVL_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Choose between resource control modes 'normal' and 'minimum guaranteed'
 * for all ports of the resp. buffer.
 *
 * \details 
 * '0': normal mode
 * '1': minimum guaranteed mode
 *
 * Field: ::VTSS_IQS_PORT_RAM_CFG_PORT_RC_CFG . PORT_RC_MODE
 */
#define  VTSS_F_IQS_PORT_RAM_CFG_PORT_RC_CFG_PORT_RC_MODE  VTSS_BIT(2)

/** 
 * \brief
 * Select between FC mode and drop mode for the resp. port.
 *
 * \details 
 * '0': FC is enabled
 * '1': Drop mode is enabled
 *
 * Field: ::VTSS_IQS_PORT_RAM_CFG_PORT_RC_CFG . PORT_FC_DROP_MODE
 */
#define  VTSS_F_IQS_PORT_RAM_CFG_PORT_RC_CFG_PORT_FC_DROP_MODE  VTSS_BIT(1)

/** 
 * \brief
 * Each port is configured to allow an individual enable/disable of the RX
 * data flow.
 *
 * \details 
 * '0': disable RX port
 * '1': enable RX port
 *
 * Field: ::VTSS_IQS_PORT_RAM_CFG_PORT_RC_CFG . RX_PORT_ENA
 */
#define  VTSS_F_IQS_PORT_RAM_CFG_PORT_RC_CFG_RX_PORT_ENA  VTSS_BIT(0)


/** 
 * \brief High and low watermark for ports
 *
 * \details
 * HWM and LWM for resource control on port level
 *
 * Register: \a IQS:PORT_RAM_CFG:PORT_RC_HLWM_CFG
 *
 * @param ri Replicator: x_QS_NUM_PORTS (??), 0-34
 */
#define VTSS_IQS_PORT_RAM_CFG_PORT_RC_HLWM_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5240 + (ri))

/** 
 * \brief
 * HWM threshold used in profiles for ports
 * Note that HWM < LWM is not allowed and leeds to an improper behaviour of
 * the QS.
 *
 * \details 
 * Number of cells per port
 *
 * Field: ::VTSS_IQS_PORT_RAM_CFG_PORT_RC_HLWM_CFG . PORT_RC_PROFILE_HWM
 */
#define  VTSS_F_IQS_PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_HWM(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_HWM     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_HWM(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * LWM threshold used in profiles for ports
 * Note that HWM < LWM is not allowed and leeds to an improper behaviour of
 * the QS.
 *
 * \details 
 * Number of cells per port
 *
 * Field: ::VTSS_IQS_PORT_RAM_CFG_PORT_RC_HLWM_CFG . PORT_RC_PROFILE_LWM
 */
#define  VTSS_F_IQS_PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_LWM(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_LWM     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_PORT_RAM_CFG_PORT_RC_HLWM_CFG_PORT_RC_PROFILE_LWM(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief ATOP and SWM for port configuration
 *
 * \details
 * This register allows to configure the over-all top threshold ATOP and
 * the spill-over watermark SWM on a per port basis.
 *
 * Register: \a IQS:PORT_RAM_CFG:PORT_RC_ATOP_SWM_CFG
 *
 * @param ri Replicator: x_QS_NUM_PORTS (??), 0-34
 */
#define VTSS_IQS_PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5280 + (ri))

/** 
 * \brief
 * ATOP threshold used in profiles for ports
 *
 * \details 
 * Number of cells per port
 *
 * Field: ::VTSS_IQS_PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG . PORT_RC_PROFILE_ATOP
 */
#define  VTSS_F_IQS_PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_ATOP(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_ATOP     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_ATOP(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * SWM (spill over watermark) threshold used in profiles for ports to
 * determine the number of cells guaranteed per port.
 *
 * \details 
 * Number of cells per port
 *
 * Field: ::VTSS_IQS_PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG . PORT_RC_PROFILE_SWM
 */
#define  VTSS_F_IQS_PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_SWM(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_SWM     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_PORT_RAM_CFG_PORT_RC_ATOP_SWM_CFG_PORT_RC_PROFILE_SWM(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief GWM for port configuration
 *
 * \details
 * This register allows to configure the green watermark GWM on a per port
 * basis.
 *
 * Register: \a IQS:PORT_RAM_CFG:PORT_RC_GWM_CFG
 *
 * @param ri Replicator: x_QS_NUM_PORTS (??), 0-34
 */
#define VTSS_IQS_PORT_RAM_CFG_PORT_RC_GWM_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x52c0 + (ri))

/** 
 * \brief
 * GWM (green watermark) threshold used in profiles for ports to determine
 * the number of guaranteed green cells.
 *
 * \details 
 * Number of green cells per port.
 * Feature disabled by setting to 0
 *
 * Field: ::VTSS_IQS_PORT_RAM_CFG_PORT_RC_GWM_CFG . PORT_RC_PROFILE_GWM
 */
#define  VTSS_F_IQS_PORT_RAM_CFG_PORT_RC_GWM_CFG_PORT_RC_PROFILE_GWM(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_PORT_RAM_CFG_PORT_RC_GWM_CFG_PORT_RC_PROFILE_GWM     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_PORT_RAM_CFG_PORT_RC_GWM_CFG_PORT_RC_PROFILE_GWM(x)  VTSS_EXTRACT_BITFIELD(x,0,15)

/**
 * Register Group: \a IQS:RESOURCE_CTRL_CFG
 *
 * Resource control configuration
 */


/** 
 * \brief Resource control configuration on buffer level
 *
 * \details
 * RX buffer enable
 * Drop / non-drop mode
 * ATOP threshold for resource control on buffer level
 *
 * Register: \a IQS:RESOURCE_CTRL_CFG:BUF_RC_CFG
 */
#define VTSS_IQS_RESOURCE_CTRL_CFG_BUF_RC_CFG  VTSS_IOREG(VTSS_TO_IQS,0x5305)

/** 
 * \brief
 * ATOP threshold used on buffer level compared to port related counters
 * (i.e. the buffer filllevel results from the sum of desired port
 * filllevels or directly from the sum of desired queue filllevels)
 *
 * \details 
 * Number of cells per buffer
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_BUF_RC_CFG . BUF_PORT_RC_PROFILE_ATOP
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_BUF_RC_CFG_BUF_PORT_RC_PROFILE_ATOP(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_RESOURCE_CTRL_CFG_BUF_RC_CFG_BUF_PORT_RC_PROFILE_ATOP     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_RESOURCE_CTRL_CFG_BUF_RC_CFG_BUF_PORT_RC_PROFILE_ATOP(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * Select between FC mode and drop mode. The respective WMs are compared to
 * the port related buffer filllevel .
 *
 * \details 
 * '0': FC is enabled
 * '1': Drop mode is enabled
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_BUF_RC_CFG . BUF_PORT_FC_DROP_MODE
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_BUF_RC_CFG_BUF_PORT_FC_DROP_MODE  VTSS_BIT(1)

/** 
 * \brief
 * Each buffer is configured to allow an individual enable/disable of the
 * RX data flow.
 * Note that the default value is 0 (disable) to allow configuration before
 * data throughput is started.
 *
 * \details 
 * '0': disable RX buffer
 * '1': enable RX buffer
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_BUF_RC_CFG . RX_BUF_ENA
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_BUF_RC_CFG_RX_BUF_ENA  VTSS_BIT(0)


/** 
 * \brief High and low watermarks for buffers
 *
 * \details
 * HWM and LWM for resource control on buffer level
 *
 * Register: \a IQS:RESOURCE_CTRL_CFG:BUF_PORT_RC_HLWM_CFG
 */
#define VTSS_IQS_RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG  VTSS_IOREG(VTSS_TO_IQS,0x5306)

/** 
 * \brief
 * HWM threshold used on buffer level compared to port related counters
 * (i.e. the buffer filllevel results from the sum of desired port
 * filllevels or directly from the sum of desired queue filllevels)
 * Note that HWM < LWM is not allowed and leeds to an improper behaviour of
 * the QS.
 *
 * \details 
 * Number of cells per buffer
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG . BUF_PORT_RC_PROFILE_HWM
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_HWM(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_HWM     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_HWM(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * LWM threshold used on buffer level compared to port related counters
 * (i.e. the buffer filllevel results from the sum of desired port
 * filllevels or directly from the sum of desired queue filllevels)
 * Note that HWM < LWM is not allowed and leeds to an improper behaviour of
 * the QS.
 *
 * \details 
 * Number of cells per buffer
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG . BUF_PORT_RC_PROFILE_LWM
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_LWM(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_LWM     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_RESOURCE_CTRL_CFG_BUF_PORT_RC_HLWM_CFG_BUF_PORT_RC_PROFILE_LWM(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief Resource control configuration on priority related buffer level
 *
 * \details
 * RX buffer enable
 * Drop / non-drop mode
 * ATOP threshold for resource control on buffer level
 *
 * Register: \a IQS:RESOURCE_CTRL_CFG:BUF_PRIO_RC_CFG
 */
#define VTSS_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_CFG  VTSS_IOREG(VTSS_TO_IQS,0x5307)

/** 
 * \brief
 * ATOP threshold used on buffer level compared to priority related
 * counters (i.e. the buffer filllevel results from the sum of desired
 * priority filllevels or directly from the sum of desired queue
 * filllevels)
 *
 * \details 
 * Number of cells per buffer
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_CFG . BUF_PRIO_RC_PROFILE_ATOP
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_CFG_BUF_PRIO_RC_PROFILE_ATOP(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_CFG_BUF_PRIO_RC_PROFILE_ATOP     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_CFG_BUF_PRIO_RC_PROFILE_ATOP(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * Select between FC mode and drop mode. The respective WMs are compared to
 * the priority related buffer filllevel .
 *
 * \details 
 * '0': FC is enabled
 * '1': Drop mode is enabled
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_CFG . BUF_PRIO_FC_DROP_MODE
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_CFG_BUF_PRIO_FC_DROP_MODE  VTSS_BIT(1)


/** 
 * \brief High and low watermarks for buffers
 *
 * \details
 * HWM and LWM for resource control on buffer level
 *
 * Register: \a IQS:RESOURCE_CTRL_CFG:BUF_PRIO_RC_HLWM_CFG
 */
#define VTSS_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG  VTSS_IOREG(VTSS_TO_IQS,0x5308)

/** 
 * \brief
 * HWM threshold used on buffer level compared to priority related counters
 * (i.e. the buffer filllevel results from the sum of desired priority
 * filllevels or directly from the sum of desired queue filllevels)
 * Note that HWM < LWM is not allowed and leeds to an improper behaviour of
 * the QS.
 *
 * \details 
 * Number of cells per buffer
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG . BUF_PRIO_RC_PROFILE_HWM
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG_BUF_PRIO_RC_PROFILE_HWM(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG_BUF_PRIO_RC_PROFILE_HWM     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG_BUF_PRIO_RC_PROFILE_HWM(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * LWM threshold used on buffer level compared to priority related counters
 * (i.e. the buffer filllevel results from the sum of desired priority
 * filllevels or directly from the sum of desired queue filllevels)
 * Note that HWM < LWM is not allowed and leeds to an improper behaviour of
 * the QS.
 *
 * \details 
 * Number of cells per buffer
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG . BUF_PRIO_RC_PROFILE_LWM
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG_BUF_PRIO_RC_PROFILE_LWM(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG_BUF_PRIO_RC_PROFILE_LWM     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_RESOURCE_CTRL_CFG_BUF_PRIO_RC_HLWM_CFG_BUF_PRIO_RC_PROFILE_LWM(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief Resource control configuration on priority level
 *
 * \details
 * RX enable per priority per buffer
 * Drop / non-drop mode
 * RC mode configuration (normal/minimum-guaranteed)
 * Cell counter enable on buffer level
 * ATOP for resource control on buffer level
 *
 * Register: \a IQS:RESOURCE_CTRL_CFG:PRIO_RC_CFG
 *
 * @param ri Replicator: x_QS_NUM_PRIO_TIMES_NUM_BUF (??), 0-7
 */
#define VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5309 + (ri))

/** 
 * \brief
 * Each priority can be enabled to have its received cells counted
 * additionally on buffer level.
 * Note that the configuration can be made per priority per buffer.
 *
 * \details 
 * '0': disable priority for cell counting on buffer level
 * '1': enable priority for cell counting on buffer level
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_CFG . PRIO_CNT_ON_BUF_LVL_ENA
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_PRIO_RC_CFG_PRIO_CNT_ON_BUF_LVL_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Choose between resource control modes 'normal' and 'minimum guaranteed'
 * for all priorities of the resp. buffer.
 * Note that the configuration can be made per priority per buffer.
 *
 * \details 
 * '0': normal mode
 * '1': minimum guaranteed mode
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_CFG . PRIO_RC_MODE
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_PRIO_RC_CFG_PRIO_RC_MODE  VTSS_BIT(2)

/** 
 * \brief
 * Select between FC mode and drop mode for the resp. priority.
 * Note that the configuration can be made per priority per buffer.
 *
 * \details 
 * '0': FC is enabled
 * '1': Drop mode is enabled
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_CFG . PRIO_FC_DROP_MODE
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_PRIO_RC_CFG_PRIO_FC_DROP_MODE  VTSS_BIT(1)

/** 
 * \brief
 * Each priority is configured to allow an individual enable/disable of the
 * RX data flow.
 * Note that the configuration can be made per priority per buffer.
 *
 * \details 
 * '0': disable RX priority
 * '1': enable RX priority
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_CFG . RX_PRIO_ENA
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_PRIO_RC_CFG_RX_PRIO_ENA  VTSS_BIT(0)


/** 
 * \brief High and low watermarks for priorities
 *
 * \details
 * HWM and LWM for resource control on priority level
 *
 * Register: \a IQS:RESOURCE_CTRL_CFG:PRIO_RC_HLWM_CFG
 *
 * @param ri Replicator: x_QS_NUM_PRIO_TIMES_NUM_BUF (??), 0-7
 */
#define VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5311 + (ri))

/** 
 * \brief
 * HWM threshold used for the resp. priority
 * Note that the configuration can be made per priority per buffer.
 * Note that HWM < LWM is not allowed and leeds to an improper behaviour of
 * the QS.
 *
 * \details 
 * Number of cells per priority
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG . PRIO_RC_PROFILE_HWM
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG_PRIO_RC_PROFILE_HWM(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG_PRIO_RC_PROFILE_HWM     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG_PRIO_RC_PROFILE_HWM(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * LWM threshold used for the resp. priority
 * Note that the configuration can be made per priority per buffer.
 * Note that HWM < LWM is not allowed and leeds to an improper behaviour of
 * the QS.
 *
 * \details 
 * Number of cells per priority
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG . PRIO_RC_PROFILE_LWM
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG_PRIO_RC_PROFILE_LWM(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG_PRIO_RC_PROFILE_LWM     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_RESOURCE_CTRL_CFG_PRIO_RC_HLWM_CFG_PRIO_RC_PROFILE_LWM(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief ATOP and SWM for priority configuration
 *
 * \details
 * This register allows to configure the over-all top threshold ATOP and
 * the spill-over watermark SWM on a per priority basis.
 *
 * Register: \a IQS:RESOURCE_CTRL_CFG:PRIO_RC_ATOP_SWM_CFG
 *
 * @param ri Replicator: x_QS_NUM_PRIO_TIMES_NUM_BUF (??), 0-7
 */
#define VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5319 + (ri))

/** 
 * \brief
 * ATOP threshold used for the resp. priority
 * Note that the configuration can be made per priority per buffer.
 *
 * \details 
 * Number of cells per priority
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG . PRIO_RC_PROFILE_ATOP
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG_PRIO_RC_PROFILE_ATOP(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_IQS_RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG_PRIO_RC_PROFILE_ATOP     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_IQS_RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG_PRIO_RC_PROFILE_ATOP(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * SWM (spill over watermark) threshold used in profiles for priorities to
 * determine the number of cells guaranteed per priority.
 *
 * \details 
 * Number of cells per priority
 *
 * Field: ::VTSS_IQS_RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG . PRIO_RC_PROFILE_SWM
 */
#define  VTSS_F_IQS_RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG_PRIO_RC_PROFILE_SWM(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG_PRIO_RC_PROFILE_SWM     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_RESOURCE_CTRL_CFG_PRIO_RC_ATOP_SWM_CFG_PRIO_RC_PROFILE_SWM(x)  VTSS_EXTRACT_BITFIELD(x,0,15)

/**
 * Register Group: \a IQS:QU_MEM_RAM_STATUS
 *
 * Combined configuration bitgroups located in one RAM
 */

/**
 * Register Group: \a IQS:RED_RAM
 *
 * Combined configuration bitgroups located in one RAM
 */


/** 
 * \brief Average fill level for a queue fractional part
 *
 * \details
 * Average number of cells accumulated over one queue - fractional part
 *
 * Register: \a IQS:RED_RAM:QU_MEM_AVG_ALLOC_FRAC_STATUS
 *
 * @param ri Replicator: x_QS_NUM_QUEUES (??), 0-279
 */
#define VTSS_IQS_RED_RAM_QU_MEM_AVG_ALLOC_FRAC_STATUS(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5800 + (ri))

/** 
 * \brief
 * Fractional part of average fill level of the resp. queue
 *
 * \details 
 * Fractional 18 bits of  number of cells. (Fractional bit due to low-pass
 * filtering.)
 *
 * Field: ::VTSS_IQS_RED_RAM_QU_MEM_AVG_ALLOC_FRAC_STATUS . QU_MEM_AVG_ALLOC_FRAC
 */
#define  VTSS_F_IQS_RED_RAM_QU_MEM_AVG_ALLOC_FRAC_STATUS_QU_MEM_AVG_ALLOC_FRAC(x)  VTSS_ENCODE_BITFIELD(x,0,18)
#define  VTSS_M_IQS_RED_RAM_QU_MEM_AVG_ALLOC_FRAC_STATUS_QU_MEM_AVG_ALLOC_FRAC     VTSS_ENCODE_BITMASK(0,18)
#define  VTSS_X_IQS_RED_RAM_QU_MEM_AVG_ALLOC_FRAC_STATUS_QU_MEM_AVG_ALLOC_FRAC(x)  VTSS_EXTRACT_BITFIELD(x,0,18)


/** 
 * \brief Average fill level for a queue
 *
 * \details
 * Average number of cells accumulated over one queue
 *
 * Register: \a IQS:RED_RAM:QU_MEM_AVG_ALLOC_STATUS
 *
 * @param ri Replicator: x_QS_NUM_QUEUES (??), 0-279
 */
#define VTSS_IQS_RED_RAM_QU_MEM_AVG_ALLOC_STATUS(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5a00 + (ri))

/** 
 * \brief
 * Average fill level of the resp. queue
 *
 * \details 
 * Number of cells (Integer part)
 *
 * Field: ::VTSS_IQS_RED_RAM_QU_MEM_AVG_ALLOC_STATUS . QU_MEM_AVG_ALLOC
 */
#define  VTSS_F_IQS_RED_RAM_QU_MEM_AVG_ALLOC_STATUS_QU_MEM_AVG_ALLOC(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_RED_RAM_QU_MEM_AVG_ALLOC_STATUS_QU_MEM_AVG_ALLOC     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_RED_RAM_QU_MEM_AVG_ALLOC_STATUS_QU_MEM_AVG_ALLOC(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief Configuration register for Wq
 *
 * \details
 * The Wq value can be written to RAM serialized for all queues. This
 * register holds the Wq value as well as the queue number specified as
 * port number, priority number. The register value is written to RAM by HW
 * in an appropriate timeslot.
 *
 * Register: \a IQS:RED_RAM:RED_ECN_WQ_CFG
 *
 * @param ri Replicator: x_QS_NUM_QUEUES (??), 0-279
 */
#define VTSS_IQS_RED_RAM_RED_ECN_WQ_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5c00 + (ri))

/** 
 * \brief
 * al(n) = (1-2^-wq) * al(n-1) + (2^-wq) * fl  
 * al = average fill level
 * fl = current fill level
 * n, n-1 = current, previous timeslot
 * 1 timeslot =  4 * #QS_QUEUES * system-clock cycle
 *
 * \details 
 * Field: ::VTSS_IQS_RED_RAM_RED_ECN_WQ_CFG . RED_ECN_PROFILE_WQ
 */
#define  VTSS_F_IQS_RED_RAM_RED_ECN_WQ_CFG_RED_ECN_PROFILE_WQ(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_IQS_RED_RAM_RED_ECN_WQ_CFG_RED_ECN_PROFILE_WQ     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_IQS_RED_RAM_RED_ECN_WQ_CFG_RED_ECN_PROFILE_WQ(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

/**
 * Register Group: \a IQS:PORT_MEM_RAM_STATUS
 *
 * Combined configuration bitgroups located in one RAM
 */

/**
 * Register Group: \a IQS:RESOURCE_CTRL_STATUS
 *
 * Status of different data memories
 */

/**
 * Register Group: \a IQS:MAIN
 *
 * Combined QS configuration fields
 */


/** 
 * \brief Reinitialization of ABMs, Q-Descriptors and counters
 *
 * \details
 * Reinitialization of all allocation bitmaps, queue descriptors and memory
 * allocation counters.
 *
 * Register: \a IQS:MAIN:MAIN_REINIT_1SHOT
 */
#define VTSS_IQS_MAIN_MAIN_REINIT_1SHOT      VTSS_IOREG(VTSS_TO_IQS,0x532d)

/** 
 * \brief
 * This bit allows to reinitialize the allocation bitmaps, all queue
 * descriptors, the memory allocation counters and the resource control
 * configurations (stored in RAMs), I.e. to have a kind of internal reset.
 * All allocation bitmap words are overwritten entirely with '1's.
 * To reinitialize all queue descriptors the empty bits are set 'valid' and
 * the pointers are set to 0.
 * The queue-size, port-size, priority-size and buffer-size counters are
 * set to zero.
 * The queue and port configuration (RAMs) is set to the default values.
 * 
 * Note that this feature should typically be triggered after HW reset was
 * used to have all registers (HW reset) as well as all RAMs (main reinit)
 * set to their default values. If not using HW reset before triggering the
 * main_reinit bit the status of the QS may become improper.
 *
 * \details 
 * '0': No action
 * '1': Reinitialization (Bit is automatically cleared)
 *
 * Field: ::VTSS_IQS_MAIN_MAIN_REINIT_1SHOT . MAIN_REINIT
 */
#define  VTSS_F_IQS_MAIN_MAIN_REINIT_1SHOT_MAIN_REINIT  VTSS_BIT(0)


/** 
 * \brief Flag for errors during RAM integrity check
 *
 * \details
 * Register holding one bit per RAM for integrity check errors.
 *
 * Register: \a IQS:MAIN:RAM_INTEGRITY_ERR_STICKY
 */
#define VTSS_IQS_MAIN_RAM_INTEGRITY_ERR_STICKY  VTSS_IOREG(VTSS_TO_IQS,0x532f)

/** 
 * \brief
 * Whenever the integrity check of a RAM fails then the resp. bit is set.
 * 
 * bit	 0: RAM for Queue Descriptor - 1st part
 * bit	 1: RAM for Queue Descriptor - 2nd part
 * bit	 2: RAM for Linked List
 * bit	 3: RAM for Frame Descriptor Part A (SOF + Dst. Info)
 * bit	 4: RAM for Frame Descriptor Part B (Cell cnt, EOF_rxed, Abort,
 * Unused_bytes)
 * bit	 5: RAM for Segment Pointer
 * bit	 6: RAM for the main bitmap for segments in the allocation bitmap
 * (Not SOQS)
 * bit	 7: RAM for the main bitmap for chunks in the allocation bitmap
 * bit	 8: RAM for queue size, queue status and cell_cnt of the current
 * frame RAM
 * bit	 9: RAM for port size
 * bit 10: RAM for resource control per queue configurations
 * bit 11: RAM for resource control per port configurations
 * bit 12: RAM for RX statistic counting - CR RAM 
 * bit 13: RAM for RX statistic counting - IR RAM 
 * bit 14: RAM for TX statistic counting - CR RAM 
 * bit 15: RAM for TX statistic counting - IR RAM 
 * bit 16: RAM for ISDX statistic counting - CR RAM 
 * bit 17: RAM for ISDX statistic counting - IR RAM
 * bit 18: RAM for RED - Average queue size
 * bit 19: RAM for RED - Real queue size
 * bit 20: RAM for RED - Count of non-dropped frames
 * bit 21: RAM for traffic data to wait on resource control decision (Not
 * SOQS)
 * bit 21: RAM for cell use count (Only SOQS)
 * bit 22: RAM for traffic data to be stored (Not SOQS)
 * bit 22: Use count RAM: unexpected first copy seen (Only SOQS)
 * bit 23: IFH Bip8 checksum err, when data is read from traffic RAM (Only
 * SOQS)
 * bit 24: Use count RAM: underrun (Only SOQS)
 * bit 25: Use count RAM: overflow (Only SOQS)
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_IQS_MAIN_RAM_INTEGRITY_ERR_STICKY . RAM_INTEG_ERR_STICKY
 */
#define  VTSS_F_IQS_MAIN_RAM_INTEGRITY_ERR_STICKY_RAM_INTEG_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,26)
#define  VTSS_M_IQS_MAIN_RAM_INTEGRITY_ERR_STICKY_RAM_INTEG_ERR_STICKY     VTSS_ENCODE_BITMASK(0,26)
#define  VTSS_X_IQS_MAIN_RAM_INTEGRITY_ERR_STICKY_RAM_INTEG_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,26)

/**
 * Register Group: \a IQS:MEM_ALLOC_CNT_SYNC
 *
 * Synchronization of the memory allocation counters
 */

/**
 * Register Group: \a IQS:ABM_DBG
 *
 * Reinitialization and status of allocation bitmaps
 */


/** 
 * \brief Status of allocation bitmap
 *
 * \details
 * Shows whether (chunk, segment) allocation bitmap was empty since last
 * access.
 *
 * Register: \a IQS:ABM_DBG:ABM_EMPTY_STICKY
 */
#define VTSS_IQS_ABM_DBG_ABM_EMPTY_STICKY    VTSS_IOREG(VTSS_TO_IQS,0x533b)

/** 
 * \brief
 * Status of ABM for chunks
 * Note: An empty bitmap is equivalent to a full QS buffer.
 *
 * \details 
 * '0'	No request appeared while the ABM was empty or the ABM was not
 * empty
 * '1' A request appeared while the ABM was empty
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_IQS_ABM_DBG_ABM_EMPTY_STICKY . CHUNK_ABM_REQ_WHILE_EMPTY_STICKY
 */
#define  VTSS_F_IQS_ABM_DBG_ABM_EMPTY_STICKY_CHUNK_ABM_REQ_WHILE_EMPTY_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Status of ABM for segments
 * Note: An empty bitmap is equivalent to a full QS buffer.
 *
 * \details 
 * '0'	No request appeared while the ABM was empty or the ABM was not
 * empty
 * '1' A request appeared while the ABM was empty
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_IQS_ABM_DBG_ABM_EMPTY_STICKY . SEG_ABM_REQ_WHILE_EMPTY_STICKY
 */
#define  VTSS_F_IQS_ABM_DBG_ABM_EMPTY_STICKY_SEG_ABM_REQ_WHILE_EMPTY_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a IQS:RESERVED
 *
 * Register group holding a reserved register
 */


/** 
 * \brief Reserved register
 *
 * \details
 * Register: \a IQS:RESERVED:RESERVED
 */
#define VTSS_IQS_RESERVED_RESERVED           VTSS_IOREG(VTSS_TO_IQS,0x533c)

/** 
 * \brief
 * Must be set to 0 during initialization
 *
 * \details 
 * Field: ::VTSS_IQS_RESERVED_RESERVED . RESERVED_1
 */
#define  VTSS_F_IQS_RESERVED_RESERVED_RESERVED_1(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_IQS_RESERVED_RESERVED_RESERVED_1     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_IQS_RESERVED_RESERVED_RESERVED_1(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Must be set to 0 during initialization
 *
 * \details 
 * Field: ::VTSS_IQS_RESERVED_RESERVED . RESERVED_2
 */
#define  VTSS_F_IQS_RESERVED_RESERVED_RESERVED_2(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_IQS_RESERVED_RESERVED_RESERVED_2     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_IQS_RESERVED_RESERVED_RESERVED_2(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

/**
 * Register Group: \a IQS:REPLAY
 *
 * Chunk and segment address as well as deallocation disable used for replay function
 */

/**
 * Register Group: \a IQS:DEBUG
 *
 * Reinitialization or reset of counters and queue descriptors
 */

/**
 * Register Group: \a IQS:FRM_LEN_MODE
 *
 * Frame length mode
 */

/**
 * Register Group: \a IQS:STAT_GLOBAL_CFG_RX
 *
 * "RX counter global configuration registers"
 */


/** 
 * \brief Event mask for counter.
 *
 * \details
 * Event mask for counter of all flows.
 *
 * Register: \a IQS:STAT_GLOBAL_CFG_RX:STAT_GLOBAL_EVENT_MASK
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-3
 */
#define VTSS_IQS_STAT_GLOBAL_CFG_RX_STAT_GLOBAL_EVENT_MASK(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5354 + (ri))

/** 
 * \brief
 * "This value stores the event mask which indicates the counter of all
 * flows to count certain events.
 * If set to '1' the respective event is not filtered and can trigger the
 * counter.
 * If set to '0' the respective event is filtered and the counter will
 * treat the frame as if no event has occurred.
 * Which type of frame is counted is defined in: STAT_CFG,
 * CFG_CNT_FRM_TYPE.
 *	 The following events apply to QS_STAT_RX:
 * The following events apply to QS_STAT_RX and STAT_ISDX:
 * 0. Delete a failed frame (IQS, OQS)
 * 1. Delete a frame due to unavailability of buffer resources (IQS, OQS)
 * 2. Delete a frame due to unavailability of priority resources (IQS, 
 * OQS)
 * 3. Delete a frame due to unavailability of port resources (IQS, OQS)
 * 4. Delete a frame due to unavailability of queue resources (IQS, OQS)
 * 5. Delete a frame due to the RED mechanism (IQS, OQS)
 * 6. Delete a frame due to a disabled queue (IQS, OQS)
 * 7. Delete a frame due to more than 62 cells long (IQS, OQS)
 * 8: Green frame (IQS, OQS)
 * 9: Yellow frame (IQS, OQS)
 * 10: Dropped green frame (IQS, OQS)
 * 11: Dropped yellow frame (IQS, OQS)
 * 12a. Delete a frame due to ANA - ACL policer (IQS)
 * 12b. Delete a frame due to IQS head drop (OQS)
 * 13. Delete a frame due to ANA - Queue policer (IQS)
 * 14. Delete a frame due to ANA - Port policer (IQS)
 * 
 * 
 * 
 * There is a priority within the RX discard events:
 * IF
 *   event[6] == true (disabled queue/port/prio/buffer)
 * THEN
 *   all other events are masked out
 * ELSIF
 *   event[0] == true (failed frame)
 * THEN 
 *   all remaining events are masked out
 * ENDIF
 * 
 * A too long frame (event[7]) is masked by RED (event[5]) or the
 * unavailability of at least one of queue, port priority or buffer
 * resources.
 * 
 * E.g. If all good frames that where dropped, because the QS run out of
 * resources on any of the levels, should be counted, than the
 * GLOBAL_EVENT_MASK has to be set to 0x1E and the CFG_CNT_FRM_TYPE
 * register must be set to 0x1.
 *    "
 *
 * \details 
 * If an asserted event is masked out in the GLOBAL_EVENT_MASK, the
 * incoming frame should not be counted for all flows.
 *
 * Field: ::VTSS_IQS_STAT_GLOBAL_CFG_RX_STAT_GLOBAL_EVENT_MASK . GLOBAL_EVENT_MASK
 */
#define  VTSS_F_IQS_STAT_GLOBAL_CFG_RX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_STAT_GLOBAL_CFG_RX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_STAT_GLOBAL_CFG_RX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief Reset all statistic counter.
 *
 * \details
 * Writing '1' into this register will lead to the reset of STAT.
 *
 * Register: \a IQS:STAT_GLOBAL_CFG_RX:STAT_RESET
 */
#define VTSS_IQS_STAT_GLOBAL_CFG_RX_STAT_RESET  VTSS_IOREG(VTSS_TO_IQS,0x5358)

/** 
 * \brief
 * Writing '1' into this bit will lead to the reset of STAT (counter and
 * configuration). This bit is auto-cleared after the reset is done.
 *
 * \details 
 * '0': No action
 * '1': to reset STAT (Bit is automatically cleared)
 *
 * Field: ::VTSS_IQS_STAT_GLOBAL_CFG_RX_STAT_RESET . RESET
 */
#define  VTSS_F_IQS_STAT_GLOBAL_CFG_RX_STAT_RESET_RESET  VTSS_BIT(0)

/**
 * Register Group: \a IQS:STAT_CNT_CFG_RX
 *
 * "RX counter per queue configuration and status registers"
 */


/** 
 * \brief Sticky bits for events.
 *
 * \details
 * It is the sticky bits of events. If one event is triggered, the
 * corresponding bit is set to '1' before it is cleared. To write '1' into
 * this bit will clear the sticky bit.
 *
 * Register: \a IQS:STAT_CNT_CFG_RX:STAT_EVENTS_STICKY
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-279
 */
#define VTSS_IQS_STAT_CNT_CFG_RX_STAT_EVENTS_STICKY(gi)  VTSS_IOREG_IX(VTSS_TO_IQS,0x4000,gi,16,0,0)

/** 
 * \brief
 * These are the sticky bits of events. There is a sticky bit for an event
 * for each flow.
 *
 * \details 
 * '1': The corresponding event is triggered since it is cleared last time.
 * '0': No such event is triggered since it is cleared last time.
 *
 * Field: ::VTSS_IQS_STAT_CNT_CFG_RX_STAT_EVENTS_STICKY . STICKY_BITS
 */
#define  VTSS_F_IQS_STAT_CNT_CFG_RX_STAT_EVENTS_STICKY_STICKY_BITS(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_STAT_CNT_CFG_RX_STAT_EVENTS_STICKY_STICKY_BITS     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_STAT_CNT_CFG_RX_STAT_EVENTS_STICKY_STICKY_BITS(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief Event handling configuration.
 *
 * \details
 * This register group defines how to handle the incoming events.
 *
 * Register: \a IQS:STAT_CNT_CFG_RX:STAT_CFG
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-279
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-3
 */
#define VTSS_IQS_STAT_CNT_CFG_RX_STAT_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_IQS,0x4000,gi,16,ri,1)

/** 
 * \brief
 * This field is to configure the counters of a flow to count frames with
 * different types.
 *
 * \details 
 * "000": The frames without any event signal or frame error signal
 * asserted are counted.
 * "001": The frames with unmasked (enabled) events asserted but with no
 * error indications are counted.
 * "010": The frames with both event signal and the error signal asserted
 * are counted.
 * "011": The frames with event signal asserted are counted in spite of the
 * error indications.
 * "100": The frames with the error signal asserted, but with no event
 * signal are counted.
 * "101": The frames with error signal asserted are counted in spite of the
 * accompied event indications.
 * 
 *	| Error | Event
 * -----+-------+------
 *  000 |   N	|   N
 * -----+-------+-----
 *  001 |   N	|   Y
 * -----+-------+------
 *  010 |   Y	|   Y
 * -----+-------+------
 *  011 |   -	|   Y
 * -----+-------+------
 *  100 |   Y	|   N
 * -----+-------+------
 *  101 |   Y	|   -
 * -----+-------+------
 *
 * Field: ::VTSS_IQS_STAT_CNT_CFG_RX_STAT_CFG . CFG_CNT_FRM_TYPE
 */
#define  VTSS_F_IQS_STAT_CNT_CFG_RX_STAT_CFG_CFG_CNT_FRM_TYPE(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_IQS_STAT_CNT_CFG_RX_STAT_CFG_CFG_CNT_FRM_TYPE     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_IQS_STAT_CNT_CFG_RX_STAT_CFG_CFG_CNT_FRM_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * This field is to configure the counters of a flow to count the byte
 * number or the frame number.
 *
 * \details 
 * '0': The frame number is counted.
 * '1': The byte number is counted.
 *
 * Field: ::VTSS_IQS_STAT_CNT_CFG_RX_STAT_CFG . CFG_CNT_BYTE
 */
#define  VTSS_F_IQS_STAT_CNT_CFG_RX_STAT_CFG_CFG_CNT_BYTE  VTSS_BIT(0)


/** 
 * \brief Least significant 32 bits.
 *
 * \details
 * This register contains the least significant 32 bits of a counter.
 *
 * Register: \a IQS:STAT_CNT_CFG_RX:STAT_LSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-279
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-3
 */
#define VTSS_IQS_STAT_CNT_CFG_RX_STAT_LSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_IQS,0x4000,gi,16,ri,5)


/** 
 * \brief Most significant 8 bits.
 *
 * \details
 * This register contains the most significant 8 bits of a counter.
 *
 * Register: \a IQS:STAT_CNT_CFG_RX:STAT_MSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-279
 * @param ri Replicator: x_STAT_CNT_MSB_PER_FLW_ID (??), 0-3
 */
#define VTSS_IQS_STAT_CNT_CFG_RX_STAT_MSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_IQS,0x4000,gi,16,ri,9)

/** 
 * \brief
 * The field stores the value in the counters of a flow from bit 32 to the
 * most significant bit.
 *
 * \details 
 * Field: ::VTSS_IQS_STAT_CNT_CFG_RX_STAT_MSB_CNT . MSB_CNT
 */
#define  VTSS_F_IQS_STAT_CNT_CFG_RX_STAT_MSB_CNT_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_IQS_STAT_CNT_CFG_RX_STAT_MSB_CNT_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_IQS_STAT_CNT_CFG_RX_STAT_MSB_CNT_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a IQS:STAT_GLOBAL_CFG_TX
 *
 * "TX counter global configuration registers"
 */


/** 
 * \brief Event mask for counter.
 *
 * \details
 * Event mask for counter of all flows.
 *
 * Register: \a IQS:STAT_GLOBAL_CFG_TX:STAT_GLOBAL_EVENT_MASK
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-1
 */
#define VTSS_IQS_STAT_GLOBAL_CFG_TX_STAT_GLOBAL_EVENT_MASK(ri)  VTSS_IOREG(VTSS_TO_IQS,0x5359 + (ri))

/** 
 * \brief
 * "This value stores the event mask which indicates the counter of all
 * flows to count certain events.
 * If set to '1' the respective event is not filtered and can trigger the
 * counter.
 * The following events apply to QS_STAT_TX:
 * 0. Delete a failed frame (IQS, OQS)
 * 1. Delete a frame due to aging (IQS, OQS)
 * 2. Delete a frame due to queue flush (IQS, OQS)
 * 3. Delete a frame due to full OQS (IQS only)
 * 4: Green frame (IQS, OQS)
 * 5: Yellow frame (IQS, OQS)
 * 6: Dropped green frame (IQS, OQS)
 * 7: Dropped yellow frame (IQS, OQS)
 * 
 * Note that the scheduling block reinforms the OQS about the discard
 * reason whenever a cell of the respective frame is scheduled and that
 * there can only be one reason valid at the same time. The priority of the
 * discard reasons is therefore the purpose of the scheduling block.
 * Note: For queue flush the scheduling block does not provide correct
 * frame boundary information. Thus frame counter will count cells instead
 * and for a byte counter the value is invalid.
 * 
 * Which type of frame is counted is defined in: STAT_CFG,
 * CFG_CNT_FRM_TYPE.
 * 
 *    "
 *
 * \details 
 * If an asserted event is masked out in the GLOBAL_EVENT_MASK, the
 * incoming frame should not be counted for all flows.
 *
 * Field: ::VTSS_IQS_STAT_GLOBAL_CFG_TX_STAT_GLOBAL_EVENT_MASK . GLOBAL_EVENT_MASK
 */
#define  VTSS_F_IQS_STAT_GLOBAL_CFG_TX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_IQS_STAT_GLOBAL_CFG_TX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_IQS_STAT_GLOBAL_CFG_TX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Reset all statistic counter.
 *
 * \details
 * Writing '1' into this register will lead to the reset of STAT.
 *
 * Register: \a IQS:STAT_GLOBAL_CFG_TX:STAT_RESET
 */
#define VTSS_IQS_STAT_GLOBAL_CFG_TX_STAT_RESET  VTSS_IOREG(VTSS_TO_IQS,0x535b)

/** 
 * \brief
 * Writing '1' into this bit will lead to the reset of STAT (counter and
 * configuration). This bit is auto-cleared after the reset is done.
 *
 * \details 
 * '0': No action
 * '1': to reset STAT (Bit is automatically cleared)
 *
 * Field: ::VTSS_IQS_STAT_GLOBAL_CFG_TX_STAT_RESET . RESET
 */
#define  VTSS_F_IQS_STAT_GLOBAL_CFG_TX_STAT_RESET_RESET  VTSS_BIT(0)

/**
 * Register Group: \a IQS:STAT_CNT_CFG_TX
 *
 * "TX counter per queue configuration and status registers"
 */


/** 
 * \brief Sticky bits for events.
 *
 * \details
 * It is the sticky bits of events. If one event is triggered, the
 * corresponding bit is set to '1' before it is cleared. To write '1' into
 * this bit will clear the sticky bit.
 *
 * Register: \a IQS:STAT_CNT_CFG_TX:STAT_EVENTS_STICKY
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-279
 */
#define VTSS_IQS_STAT_CNT_CFG_TX_STAT_EVENTS_STICKY(gi)  VTSS_IOREG_IX(VTSS_TO_IQS,0x7000,gi,8,0,0)

/** 
 * \brief
 * These are the sticky bits of events. There is a sticky bit for an event
 * for each flow.
 *
 * \details 
 * '1': The corresponding event is triggered since it is cleared last time.
 * '0': No such event is triggered since it is cleared last time.
 *
 * Field: ::VTSS_IQS_STAT_CNT_CFG_TX_STAT_EVENTS_STICKY . STICKY_BITS
 */
#define  VTSS_F_IQS_STAT_CNT_CFG_TX_STAT_EVENTS_STICKY_STICKY_BITS(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_IQS_STAT_CNT_CFG_TX_STAT_EVENTS_STICKY_STICKY_BITS     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_IQS_STAT_CNT_CFG_TX_STAT_EVENTS_STICKY_STICKY_BITS(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Event handling configuration.
 *
 * \details
 * This register group defines how to handle the incoming events.
 *
 * Register: \a IQS:STAT_CNT_CFG_TX:STAT_CFG
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-279
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-1
 */
#define VTSS_IQS_STAT_CNT_CFG_TX_STAT_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_IQS,0x7000,gi,8,ri,1)

/** 
 * \brief
 * This field is to configure the counters of a flow to count frames with
 * different types.
 *
 * \details 
 * "000": The frames without any event signal or frame error signal
 * asserted are counted.
 * "001": The frames with unmasked (enabled) events asserted but with no
 * error indications are counted.
 * "010": The frames with both event signal and the error signal asserted
 * are counted.
 * "011": The frames with event signal asserted are counted in spite of the
 * error indications.
 * "100": The frames with the error signal asserted, but with no event
 * signal are counted.
 * "101": The frames with error signal asserted are counted in spite of the
 * accompied event indications.
 * 
 *	| Error | Event
 * -----+-------+------
 *  000 |   N	|   N
 * -----+-------+-----
 *  001 |   N	|   Y
 * -----+-------+------
 *  010 |   Y	|   Y
 * -----+-------+------
 *  011 |   -	|   Y
 * -----+-------+------
 *  100 |   Y	|   N
 * -----+-------+------
 *  101 |   Y	|   -
 * -----+-------+------
 *
 * Field: ::VTSS_IQS_STAT_CNT_CFG_TX_STAT_CFG . CFG_CNT_FRM_TYPE
 */
#define  VTSS_F_IQS_STAT_CNT_CFG_TX_STAT_CFG_CFG_CNT_FRM_TYPE(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_IQS_STAT_CNT_CFG_TX_STAT_CFG_CFG_CNT_FRM_TYPE     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_IQS_STAT_CNT_CFG_TX_STAT_CFG_CFG_CNT_FRM_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * This field is to configure the counters of a flow to count the byte
 * number or the frame number.
 *
 * \details 
 * '0': The frame number is counted.
 * '1': The byte number is counted.
 *
 * Field: ::VTSS_IQS_STAT_CNT_CFG_TX_STAT_CFG . CFG_CNT_BYTE
 */
#define  VTSS_F_IQS_STAT_CNT_CFG_TX_STAT_CFG_CFG_CNT_BYTE  VTSS_BIT(0)


/** 
 * \brief Least significant 32 bits.
 *
 * \details
 * This register contains the least significant 32 bits of a counter.
 *
 * Register: \a IQS:STAT_CNT_CFG_TX:STAT_LSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-279
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-1
 */
#define VTSS_IQS_STAT_CNT_CFG_TX_STAT_LSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_IQS,0x7000,gi,8,ri,3)


/** 
 * \brief Most significant 8 bits.
 *
 * \details
 * This register contains the most significant 8 bits of a counter.
 *
 * Register: \a IQS:STAT_CNT_CFG_TX:STAT_MSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-279
 * @param ri Replicator: x_STAT_CNT_MSB_PER_FLW_ID (??), 0-1
 */
#define VTSS_IQS_STAT_CNT_CFG_TX_STAT_MSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_IQS,0x7000,gi,8,ri,5)

/** 
 * \brief
 * The field stores the value in the counters of a flow from bit 32 to the
 * most significant bit.
 *
 * \details 
 * Field: ::VTSS_IQS_STAT_CNT_CFG_TX_STAT_MSB_CNT . MSB_CNT
 */
#define  VTSS_F_IQS_STAT_CNT_CFG_TX_STAT_MSB_CNT_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_IQS_STAT_CNT_CFG_TX_STAT_MSB_CNT_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_IQS_STAT_CNT_CFG_TX_STAT_MSB_CNT_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a IQS:STAT_GLOBAL_CFG_ISDX
 *
 * "ISDX counter global configuration registers"
 */


/** 
 * \brief Global configuration register
 *
 * \details
 * Global configuration register
 *
 * Register: \a IQS:STAT_GLOBAL_CFG_ISDX:STAT_GLOBAL_CFG
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-1
 */
#define VTSS_IQS_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_CFG(ri)  VTSS_IOREG(VTSS_TO_IQS,0x535c + (ri))

/** 
 * \brief
 * This field is to configure the counters of all flows to count the byte
 * number or the frame number.
 *
 * \details 
 * '0': The frame number is counted.
 * '1': The byte number is counted.
 *
 * Field: ::VTSS_IQS_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_CFG . GLOBAL_CFG_CNT_BYTE
 */
#define  VTSS_F_IQS_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_CFG_GLOBAL_CFG_CNT_BYTE  VTSS_BIT(0)


/** 
 * \brief Event mask for counter.
 *
 * \details
 * Event mask for counter of all flows.
 *
 * Register: \a IQS:STAT_GLOBAL_CFG_ISDX:STAT_GLOBAL_EVENT_MASK
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-1
 */
#define VTSS_IQS_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_EVENT_MASK(ri)  VTSS_IOREG(VTSS_TO_IQS,0x535e + (ri))

/** 
 * \brief
 * "This value stores the event mask which indicates the counter of all
 * flows to count certain events.
 * If set to '1' the respective event is not filtered and can trigger the
 * counter.
 * If set to '0' the respective event is filtered and the counter will
 * treat the frame as if no event has occurred.
 * Which type of frame is counted is defined in: STAT_CFG,
 * CFG_CNT_FRM_TYPE.
 *	 The following events apply to QS_STAT_ISDX:
 * The following events apply to QS_STAT_RX and STAT_ISDX:
 * 0. Delete a failed frame (IQS, OQS)
 * 1. Delete a frame due to unavailability of buffer resources (IQS, OQS)
 * 2. Delete a frame due to unavailability of priority resources (IQS, 
 * OQS)
 * 3. Delete a frame due to unavailability of port resources (IQS, OQS)
 * 4. Delete a frame due to unavailability of queue resources (IQS, OQS)
 * 5. Delete a frame due to the RED mechanism (IQS, OQS)
 * 6. Delete a frame due to a disabled queue (IQS, OQS)
 * 7. Delete a frame due to more than 126 cells long (IQS, OQS)
 * 8: Green frame (IQS, OQS)
 * 9: Yellow frame (IQS, OQS)
 * 10: Dropped green frame (IQS, OQS)
 * 11: Dropped yellow frame (IQS, OQS)
 * 12a. Delete a frame due to ANA - ACL policer (IQS)
 * 12b. Delete a frame due to IQS head drop (OQS)
 * 13. Delete a frame due to ANA - Queue policer (IQS)
 * 14. Delete a frame due to ANA - Port policer (IQS)
 * 
 * 
 * 
 * There is a priority within the RX discard events:
 * IF
 *   event[6] == true (disabled queue/port/prio/buffer)
 * THEN
 *   all other events are masked out
 * ELSIF
 *   event[0] == true (failed frame)
 * THEN 
 *   all remaining events are masked out
 * ENDIF
 * 
 * A too long frame (event[7]) is masked by RED (event[5]) or the
 * unavailability of at least one of queue, port priority or buffer
 * resources.
 * 
 * E.g. If all good frames that where dropped, because the QS run out of
 * resources on any of the levels, should be counted, than the
 * GLOBAL_EVENT_MASK has to be set to 0x1E and the CFG_CNT_FRM_TYPE
 * register must be set to 0x1.
 *    "
 *
 * \details 
 * If an asserted event is masked out in the GLOBAL_EVENT_MASK, the
 * incoming frame should not be counted for all flows.
 *
 * Field: ::VTSS_IQS_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_EVENT_MASK . GLOBAL_EVENT_MASK
 */
#define  VTSS_F_IQS_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_IQS_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_IQS_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief Reset all statistic counter.
 *
 * \details
 * Writing '1' into this register will lead to the reset of STAT.
 *
 * Register: \a IQS:STAT_GLOBAL_CFG_ISDX:STAT_RESET
 */
#define VTSS_IQS_STAT_GLOBAL_CFG_ISDX_STAT_RESET  VTSS_IOREG(VTSS_TO_IQS,0x5360)

/** 
 * \brief
 * Writing '1' into this bit will lead to the reset of STAT (counter and
 * configuration). This bit is auto-cleared after the reset is done.
 *
 * \details 
 * '0': No action
 * '1': to reset STAT (Bit is automatically cleared)
 *
 * Field: ::VTSS_IQS_STAT_GLOBAL_CFG_ISDX_STAT_RESET . RESET
 */
#define  VTSS_F_IQS_STAT_GLOBAL_CFG_ISDX_STAT_RESET_RESET  VTSS_BIT(0)

/**
 * Register Group: \a IQS:STAT_CNT_CFG_ISDX
 *
 * "ISDX counter per service configuration and status registers"
 */


/** 
 * \brief Least significant 32 bits.
 *
 * \details
 * This register contains the least significant 32 bits of a counter.
 *
 * Register: \a IQS:STAT_CNT_CFG_ISDX:STAT_LSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-4095
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-1
 */
#define VTSS_IQS_STAT_CNT_CFG_ISDX_STAT_LSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_IQS,0x0,gi,4,ri,0)


/** 
 * \brief Most significant 8 bits.
 *
 * \details
 * This register contains the most significant 8 bits of a counter.
 *
 * Register: \a IQS:STAT_CNT_CFG_ISDX:STAT_MSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-4095
 */
#define VTSS_IQS_STAT_CNT_CFG_ISDX_STAT_MSB_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_IQS,0x0,gi,4,0,2)

/** 
 * \brief
 * The field stores the value in the counters of a flow from bit 32 to the
 * most significant bit.
 *
 * \details 
 * Field: ::VTSS_IQS_STAT_CNT_CFG_ISDX_STAT_MSB_CNT . MSB_CNT
 */
#define  VTSS_F_IQS_STAT_CNT_CFG_ISDX_STAT_MSB_CNT_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_IQS_STAT_CNT_CFG_ISDX_STAT_MSB_CNT_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_IQS_STAT_CNT_CFG_ISDX_STAT_MSB_CNT_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


#endif /* _VTSS_JAGUAR_REGS_IQS_H_ */
