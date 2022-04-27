#ifndef _VTSS_JAGUAR_REGS_ARB_H_
#define _VTSS_JAGUAR_REGS_ARB_H_

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
 * Target: \a ARB
 *
 * Arbiter
 *
 ***********************************************************************/

/**
 * Register Group: \a ARB:CFG_STATUS
 *
 * Various configuration and status registers
 */


/** 
 * \brief Configuration of settings affecting all ports
 *
 * \details
 * Register: \a ARB:CFG_STATUS:PORT_ARB_CFG
 */
#define VTSS_ARB_CFG_STATUS_PORT_ARB_CFG     VTSS_IOREG(VTSS_TO_ARB,0x0)

/** 
 * \brief
 * Configures the bandwidth for the High Speed Port (HSP) group. 
 * This can be used to ensure that the Low Speed Port (LSP) group cannot
 * get starved by HSP traffic.
 * 
 * If there is bandwidth left after serving the Low Speed Port (LSP) group,
 * then remaining bandwidth may be utilized by HSP group.
 * 
 * Note: The High Speed Port rate limiter does not take frame adjustment
 * defined by PORT_ARB_CFG.FRM_ADJ into account.
 *
 * \details 
 * 0: Disable rate control. High speed ports have highest priority and can
 * use the entire bandwidth
 * 1: Rate = 62500 Kbps
 * n: Rate = n x 62500 Kbps
 *
 * Field: ::VTSS_ARB_CFG_STATUS_PORT_ARB_CFG . HI_SPEED_PORTS_MIN_RATE
 */
#define  VTSS_F_ARB_CFG_STATUS_PORT_ARB_CFG_HI_SPEED_PORTS_MIN_RATE(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_ARB_CFG_STATUS_PORT_ARB_CFG_HI_SPEED_PORTS_MIN_RATE     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_ARB_CFG_STATUS_PORT_ARB_CFG_HI_SPEED_PORTS_MIN_RATE(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

/** 
 * \brief
 * Value to be added to Ingress Shaper LBs each time a frame is counted for
 * this inbound port
 * 
 * If set to 20, this will correspond to inclusion of min. Ethernet IFG and
 * preamble in shaping.
 * 
 * This feature can be enabled on a per ingress port basis, using
 * INGR_SHAPER_CFG.FRM_ADJ_ENA.
 *
 * \details 
 * 0-31: Bytes.
 *
 * Field: ::VTSS_ARB_CFG_STATUS_PORT_ARB_CFG . FRM_ADJ
 */
#define  VTSS_F_ARB_CFG_STATUS_PORT_ARB_CFG_FRM_ADJ(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_ARB_CFG_STATUS_PORT_ARB_CFG_FRM_ADJ     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_ARB_CFG_STATUS_PORT_ARB_CFG_FRM_ADJ(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Select which frames Ingress Shapers shall count when measuring the rate.
 * 
 * In normal mode both discarded- and forwarded traffic is counted.
 * In forward mode only forwarded traffic is counted. 

 *
 * \details 
 * 0: Normal mode. The Ingress shapers limit all traffic.
 * 1: Forward mode. The Ingress shapers only limit forwarded traffic.
 *
 * Field: ::VTSS_ARB_CFG_STATUS_PORT_ARB_CFG . INGR_SHAPER_CNT_CFG
 */
#define  VTSS_F_ARB_CFG_STATUS_PORT_ARB_CFG_INGR_SHAPER_CNT_CFG  VTSS_BIT(24)

/** 
 * \brief
 * Select which frames Priority Shapers for priority 0-5 shall count when
 * measuring the rate.
 * 
 * In normal mode both discarded- and forwarded traffic is counted.
 * In forward mode only forwarded traffic is counted. 
 *
 * \details 
 * 0: Normal mode. Limit all traffic.
 * 1: Forward mode. Only limit forwarded traffic.
 *
 * Field: ::VTSS_ARB_CFG_STATUS_PORT_ARB_CFG . LOW_PRIO_CNT_CFG
 */
#define  VTSS_F_ARB_CFG_STATUS_PORT_ARB_CFG_LOW_PRIO_CNT_CFG  VTSS_BIT(25)

/** 
 * \brief
 * Select which frames Priority Shapers for priority 0-5 shall count when
 * measuring the rate.
 * 
 * In normal mode both discarded- and forwarded traffic is counted.
 * In forward mode only forwarded traffic is counted. 
 *
 * \details 
 * 0: Normal mode. Limit all traffic.
 * 1: Forward mode. Only limit forwarded traffic.
 *
 * Field: ::VTSS_ARB_CFG_STATUS_PORT_ARB_CFG . HIGH_PRIO_CNT_CFG
 */
#define  VTSS_F_ARB_CFG_STATUS_PORT_ARB_CFG_HIGH_PRIO_CNT_CFG  VTSS_BIT(26)


/** 
 * \brief Configuration of Ingress Shaper bandwidth
 *
 * \details
 * Configuration of  Ingress Shaper.
 * 
 * Ingress Shaper can be used to limit the amount of traffic for each
 * inbound port.. 
 * 
 * Each inbound port is configured independently.
 *
 * Register: \a ARB:CFG_STATUS:INGR_SHAPER_CFG
 *
 * @param ri Replicator: x_NUM_PORTS_IQS (??), 0-34
 */
#define VTSS_ARB_CFG_STATUS_INGR_SHAPER_CFG(ri)  VTSS_IOREG(VTSS_TO_ARB,0x1 + (ri))

/** 
 * \brief
 * Controls the rate of Ingress Shaper leaky bucket.
 *
 * \details 
 * 0: Open until burst capacity is used, then closed.
 * 1: Rate = 100160 bps
 * n: Rate = n x 100160 bps
 *
 * Field: ::VTSS_ARB_CFG_STATUS_INGR_SHAPER_CFG . INGR_SHAPER_RATE
 */
#define  VTSS_F_ARB_CFG_STATUS_INGR_SHAPER_CFG_INGR_SHAPER_RATE(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ARB_CFG_STATUS_INGR_SHAPER_CFG_INGR_SHAPER_RATE     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ARB_CFG_STATUS_INGR_SHAPER_CFG_INGR_SHAPER_RATE(x)  VTSS_EXTRACT_BITFIELD(x,0,21)

/** 
 * \brief
 * Burst capacity of Ingress Shaper leaky bucket.
 * 
 * The unit is 4KB (1KB = 1024Bytes). The largest supported threshold is
 * 252KB.
 *
 * \details 
 * 0: Always closed
 * 1-15: Burst capacity = n x 4096 bytes.
 * Values 1-15 must not be used for rates larger than 12Gbps
 * ...
 * n: Burst capacity = n x 4096 bytes
 *
 * Field: ::VTSS_ARB_CFG_STATUS_INGR_SHAPER_CFG . INGR_SHAPER_THRES
 */
#define  VTSS_F_ARB_CFG_STATUS_INGR_SHAPER_CFG_INGR_SHAPER_THRES(x)  VTSS_ENCODE_BITFIELD(x,24,6)
#define  VTSS_M_ARB_CFG_STATUS_INGR_SHAPER_CFG_INGR_SHAPER_THRES     VTSS_ENCODE_BITMASK(24,6)
#define  VTSS_X_ARB_CFG_STATUS_INGR_SHAPER_CFG_INGR_SHAPER_THRES(x)  VTSS_EXTRACT_BITFIELD(x,24,6)

/** 
 * \brief
 * If enabled, the value configured in PORT_ARB_CFG.FRM_ADJ will be added
 * to the frame length for each frame when updating the Ingress Shaper LB.
 *
 * \details 
 * 0: Disable frame length adjustment.
 * 1: Enable frame length adjustment.
 *
 * Field: ::VTSS_ARB_CFG_STATUS_INGR_SHAPER_CFG . FRM_ADJ_ENA
 */
#define  VTSS_F_ARB_CFG_STATUS_INGR_SHAPER_CFG_FRM_ADJ_ENA  VTSS_BIT(21)

/** 
 * \brief
 * The mode of the Ingress Shaper  leaky bucket bandwidth limiter.
 * 
 * Related configuration:
 * ARB::PORT_ARB_CFG.INGR_SHAPER_CNT_CFG
 *
 * \details 
 * 0: Disabled. I.e. Ingress Shaper will not limit BW from inbound port.
 * 1: Enabled. Multicast frames are counted once.
 * 2: Enabled. All copies of multicast frames are counted.
 * 3: Reserved
 *
 * Field: ::VTSS_ARB_CFG_STATUS_INGR_SHAPER_CFG . INGR_SHAPER_MODE
 */
#define  VTSS_F_ARB_CFG_STATUS_INGR_SHAPER_CFG_INGR_SHAPER_MODE(x)  VTSS_ENCODE_BITFIELD(x,22,2)
#define  VTSS_M_ARB_CFG_STATUS_INGR_SHAPER_CFG_INGR_SHAPER_MODE     VTSS_ENCODE_BITMASK(22,2)
#define  VTSS_X_ARB_CFG_STATUS_INGR_SHAPER_CFG_INGR_SHAPER_MODE(x)  VTSS_EXTRACT_BITFIELD(x,22,2)


/** 
 * \brief Configuration of Priority Shaper
 *
 * \details
 * The total bandwidth of each priority can by limited individually using
 * leaky buckets for rate control.

 *
 * Register: \a ARB:CFG_STATUS:PRIO_ARB_CFG
 *
 * @param ri Replicator: x_NUM_IQS_PRIOS (??), 0-7
 */
#define VTSS_ARB_CFG_STATUS_PRIO_ARB_CFG(ri)  VTSS_IOREG(VTSS_TO_ARB,0x24 + (ri))

/** 
 * \brief
 * Controls the rate of the Priority Shaper leaky bucket.
 * Note: The Priority Shaper does not take frame adjustment defined by
 * PORT_ARB_CFG.FRM_ADJ into account.
 *
 * \details 
 * 0: Open until burst capacity is used, then closed.
 * 1: Rate = 62500 Kbps
 * n: Rate = n x 62500 Kbps
 *
 * Field: ::VTSS_ARB_CFG_STATUS_PRIO_ARB_CFG . PRIO_ARB_LB_RATE
 */
#define  VTSS_F_ARB_CFG_STATUS_PRIO_ARB_CFG_PRIO_ARB_LB_RATE(x)  VTSS_ENCODE_BITFIELD(x,16,12)
#define  VTSS_M_ARB_CFG_STATUS_PRIO_ARB_CFG_PRIO_ARB_LB_RATE     VTSS_ENCODE_BITMASK(16,12)
#define  VTSS_X_ARB_CFG_STATUS_PRIO_ARB_CFG_PRIO_ARB_LB_RATE(x)  VTSS_EXTRACT_BITFIELD(x,16,12)

/** 
 * \brief
 * Burst capacity of Priority Shaper leaky bucket.
 * 
 * The unit is 2KB (1KB = 1024Bytes). The largest supported threshold is
 * 254KB.
 *
 * \details 
 * 0: Always closed
 * 1: Burst capacity = 2048 bytes
 * ...
 * n: Burst capacity = n x 2048 bytes
 *
 * Field: ::VTSS_ARB_CFG_STATUS_PRIO_ARB_CFG . PRIO_ARB_LB_THRES
 */
#define  VTSS_F_ARB_CFG_STATUS_PRIO_ARB_CFG_PRIO_ARB_LB_THRES(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_ARB_CFG_STATUS_PRIO_ARB_CFG_PRIO_ARB_LB_THRES     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_ARB_CFG_STATUS_PRIO_ARB_CFG_PRIO_ARB_LB_THRES(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * The mode of the Priority Shaper leaky bucket.
 *
 * \details 
 * 0: Disabled. I.e. Priority Shaper will not limit BW.
 * 1: Max limit. The priority is given no more BW than configured rate.
 * 2: Min guarantee. The priority is guaranteed at least the configured
 * rate.
 * 3: Reserved.
 *
 * Field: ::VTSS_ARB_CFG_STATUS_PRIO_ARB_CFG . PRIO_ARB_MODE
 */
#define  VTSS_F_ARB_CFG_STATUS_PRIO_ARB_CFG_PRIO_ARB_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ARB_CFG_STATUS_PRIO_ARB_CFG_PRIO_ARB_MODE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ARB_CFG_STATUS_PRIO_ARB_CFG_PRIO_ARB_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Configuration of priority DWRR
 *
 * \details
 * Configuration of Deficit Weighted Round Robin (DWRR) for arbitrating
 * among prorities 0-5.
 * 
 * The two highest priorities (6 and 7) are always strict and not included
 * in the DWRR scheduling. 
 *
 * Register: \a ARB:CFG_STATUS:PRIO_COST_CFG
 *
 * @param ri Replicator: x_PRIO_DWRR_CFG_CNT (??), 0-5
 */
#define VTSS_ARB_CFG_STATUS_PRIO_COST_CFG(ri)  VTSS_IOREG(VTSS_TO_ARB,0x2c + (ri))

/** 
 * \brief
 * The cost associated with scheduling a byte from this priority. Used by
 * the Deficit Weighted Round Robin (DWRR) algorithm. The higher the cost,
 * the less bandwidth the priority will get.
 * 
 * The two highest priorities (6 and 7) are always strict and not included
 * in the DWRR scheduling. 
 *
 * \details 
 * Cost configuration for priority 0 to 5
 * 0: Cost is 1
 * 1: Cost is 2
 * ...
 * 127: Cost is 128
 *
 * Field: ::VTSS_ARB_CFG_STATUS_PRIO_COST_CFG . DWRR_COST
 */
#define  VTSS_F_ARB_CFG_STATUS_PRIO_COST_CFG_DWRR_COST(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_ARB_CFG_STATUS_PRIO_COST_CFG_DWRR_COST     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_ARB_CFG_STATUS_PRIO_COST_CFG_DWRR_COST(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief Enable DWRR control of priority 0 to 5
 *
 * \details
 * Register: \a ARB:CFG_STATUS:DWRR_CFG
 */
#define VTSS_ARB_CFG_STATUS_DWRR_CFG         VTSS_IOREG(VTSS_TO_ARB,0x32)

/** 
 * \brief
 * This bit controls how priorities 0 to 5 are scheduled. 
 * 
 * If this bit is cleared priorities 0 to 5 are scheduled using strict
 * priority.
 * 
 * If this bit is set then priorities 0 to 5 are scheduled using DWRR. This
 * will insure the each priority is assigned a minimum share of the
 * bandwidth. 
 * 
 * The Priority Shapers can by used to set a maximum rate limit for each
 * priority.

 *
 * \details 
 * 0: Arbitrate among priority 0 to 5 using strict priority.
 * 1: Arbitrate among priority 0 to 5 using DWRR.
 *
 * Field: ::VTSS_ARB_CFG_STATUS_DWRR_CFG . DWRR_ENA
 */
#define  VTSS_F_ARB_CFG_STATUS_DWRR_CFG_DWRR_ENA  VTSS_BIT(0)


/** 
 * \brief CMI operation control
 *
 * \details
 * Register: \a ARB:CFG_STATUS:CM_CFG_VEC0
 *
 * @param ri Replicator: x_NUM_IQS_PRIOS (??), 0-7
 */
#define VTSS_ARB_CFG_STATUS_CM_CFG_VEC0(ri)  VTSS_IOREG(VTSS_TO_ARB,0x33 + (ri))


/** 
 * \brief CMI operation control
 *
 * \details
 * Register: \a ARB:CFG_STATUS:CM_CFG_VEC1
 *
 * @param ri Replicator: x_NUM_IQS_PRIOS (??), 0-7
 */
#define VTSS_ARB_CFG_STATUS_CM_CFG_VEC1(ri)  VTSS_IOREG(VTSS_TO_ARB,0x3b + (ri))

/** 
 * \brief
 * Enable/disable use of Congestion Management (CM) information per queue. 
 * 
 * CM is used for two purposes:
 * 1) Signalling of congestion state for ports on other chips in multi-chip
 * configuration.
 * 2) Signalling of congestion state for Host Mode queues from OQS to ARB.
 * 
 * When CM is used in multi-chip configurations, CM must be enabled for all
 * front ports, but disabled for ports, which interconnect chips. 
 * 
 * When enabled, the arbitration of frames from inbound ports will avoid
 * forwarding to congested destination queues.
 * 
 * When disabled, the arbitration of frames from inbound ports will ignore
 * any CM information, and may decide to forward to congested destination
 * queues.
 *
 * \details 
 * 0b000: CM disabled for priority N
 * 0bxx1: CM enabled for priority N, port 32
 * 0bx1x: CM enabled for priority N, port 33
 * 0b1xx: CM enabled for priority N, port 34
 *
 * Field: ::VTSS_ARB_CFG_STATUS_CM_CFG_VEC1 . CM_VEC1_ENA
 */
#define  VTSS_F_ARB_CFG_STATUS_CM_CFG_VEC1_CM_VEC1_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ARB_CFG_STATUS_CM_CFG_VEC1_CM_VEC1_ENA     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ARB_CFG_STATUS_CM_CFG_VEC1_CM_VEC1_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief CMI access control
 *
 * \details
 * Read/write access to current value of Arbiter's CM information.
 * 
 * This is intended mainly for debug purposes.
 *
 * Register: \a ARB:CFG_STATUS:CM_ACCESS
 */
#define VTSS_ARB_CFG_STATUS_CM_ACCESS        VTSS_IOREG(VTSS_TO_ARB,0x43)

/** 
 * \brief
 * Selects UPSID  for the indirect CMI data access
 *
 * \details 
 * 0: UPSID 0
 * ...
 * 31: UPSID 31
 *
 * Field: ::VTSS_ARB_CFG_STATUS_CM_ACCESS . CM_UPSID
 */
#define  VTSS_F_ARB_CFG_STATUS_CM_ACCESS_CM_UPSID(x)  VTSS_ENCODE_BITFIELD(x,2,5)
#define  VTSS_M_ARB_CFG_STATUS_CM_ACCESS_CM_UPSID     VTSS_ENCODE_BITMASK(2,5)
#define  VTSS_X_ARB_CFG_STATUS_CM_ACCESS_CM_UPSID(x)  VTSS_EXTRACT_BITFIELD(x,2,5)

/** 
 * \brief
 * Controls the type of operation when accessing the congestion management
 * information.
 *
 * \details 
 * 0: Read operation
 * 1: Write operation
 *
 * Field: ::VTSS_ARB_CFG_STATUS_CM_ACCESS . CM_WR_ENA
 */
#define  VTSS_F_ARB_CFG_STATUS_CM_ACCESS_CM_WR_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Control of the CMI access operation. Initiates an access when written to
 * and indicates status of the access operation when read.
 * 
 * When this bit is set, the CMI storage is accessed with the operation and
 * UPSID configured in the 'CM_WR_ENA' and 'CM_UPSID' bit groups. 
 * 
 * The SW can only set this bit. When the operation is complete the HW will
 * clear it.
 *
 * \details 
 * Read
 * 0: No access operation in progress
 * 1: Access operation in progress
 * 
 * Write
 * 0: Reserved
 * 1: Initiate access operation on the specified CMI UPSID
 *
 * Field: ::VTSS_ARB_CFG_STATUS_CM_ACCESS . CM_START_ACCESS
 */
#define  VTSS_F_ARB_CFG_STATUS_CM_ACCESS_CM_START_ACCESS  VTSS_BIT(0)


/** 
 * \brief CMI data
 *
 * \details
 * Register: \a ARB:CFG_STATUS:CM_DATA
 */
#define VTSS_ARB_CFG_STATUS_CM_DATA          VTSS_IOREG(VTSS_TO_ARB,0x44)


/** 
 * \brief CMI disable updates
 *
 * \details
 * Register: \a ARB:CFG_STATUS:CM_IGNORE_UPDATE
 */
#define VTSS_ARB_CFG_STATUS_CM_IGNORE_UPDATE  VTSS_IOREG(VTSS_TO_ARB,0x45)


/** 
 * \brief Multi-chip control
 *
 * \details
 * Register: \a ARB:CFG_STATUS:STACK_CFG
 */
#define VTSS_ARB_CFG_STATUS_STACK_CFG        VTSS_IOREG(VTSS_TO_ARB,0x46)

/** 
 * \brief
 * Select which local CPU ports, that shall be forwarded the GCPU port,
 * instead of to the local CPU.
 * 
 * All selected CPU ports are forwarded to the same GCPU port, as
 * configured in 
 * REW:COMMON:VSTAX_GCPU_CFG.GCPU_UPSID
 * REW:COMMON:VSTAX_GCPU_CFG.GCPU_UPSPN
 * 
 * To redirect frames to different ports on the GCPU, PGID entries can be
 * used and pointed to from MAC table or VCAP_IS2.
 * 
 * When copying frames to GCPU, CPU_TO_GCPU_MASK must be used.
 *
 * \details 
 * 0b00000000: No CPU ports enabled
 * 0bxxxxxxx1: CPU port 0
 * 0bxxxxxx1x: CPU port 1
 * ..
 * 0b1xxxxxxx: CPU port 7
 * 
 * Multiple local CPU ports can be enabled at the same time
 *
 * Field: ::VTSS_ARB_CFG_STATUS_STACK_CFG . CPU_TO_GCPU_MASK
 */
#define  VTSS_F_ARB_CFG_STATUS_STACK_CFG_CPU_TO_GCPU_MASK(x)  VTSS_ENCODE_BITFIELD(x,20,8)
#define  VTSS_M_ARB_CFG_STATUS_STACK_CFG_CPU_TO_GCPU_MASK     VTSS_ENCODE_BITMASK(20,8)
#define  VTSS_X_ARB_CFG_STATUS_STACK_CFG_CPU_TO_GCPU_MASK(x)  VTSS_EXTRACT_BITFIELD(x,20,8)

/** 
 * \brief
 * Enable/disable remapping of priority for frames forwarded to GCPU.

 *
 * \details 
 * 0: Do not change priority of GCPU frames.
 * 1: Change priority of GCPU frames using GCPU_PRIO_CFG:GCPU_PRIO_MAP.
 *
 * Field: ::VTSS_ARB_CFG_STATUS_STACK_CFG . GCPU_PRIO_MODE
 */
#define  VTSS_F_ARB_CFG_STATUS_STACK_CFG_GCPU_PRIO_MODE  VTSS_BIT(16)

/** 
 * \brief
 * Controls how to forward frames, which by STACK_CFG.CPU_TO_GCPU_MASK have
 * been redirected to GCPU. I.e. which port is used to reach the GCPU.
 *
 * \details 
 * 0: Send GCPU frames to port 0
 * 1: Send GCPU frames to port 1
 * ..
 * 31: Send GCPU frames to port 31 (RGMII)
 * 32: Send GCPU frames to port 32 (VD)
 * 33-40: Send GCPU frames to ports 33-40 (CPU Queue 0-7)
 * 41-62: Not Allowed
 * 63: Disable redirection
 *
 * Field: ::VTSS_ARB_CFG_STATUS_STACK_CFG . GCPU_PORT_CFG
 */
#define  VTSS_F_ARB_CFG_STATUS_STACK_CFG_GCPU_PORT_CFG(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_ARB_CFG_STATUS_STACK_CFG_GCPU_PORT_CFG     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_ARB_CFG_STATUS_STACK_CFG_GCPU_PORT_CFG(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Controls which stack ports will receive Learn All frames.
 * 
 * Learn All frames are used to exchange MAC table information between
 * chips in a multi-chip configuration.

 *
 * \details 
 * 0b0000000: No ports enabled
 * 0b0000001: VStaX2 port 0 (Port 24)
 * 0b0000010: VStaX2 port 1 (Port 25)
 * ..
 * 0b1000000: VStaX2 port 6 (Port 30)
 * 
 * Multiple ports can be enabled at the same time
 *
 * Field: ::VTSS_ARB_CFG_STATUS_STACK_CFG . LRN_ALL_CFG
 */
#define  VTSS_F_ARB_CFG_STATUS_STACK_CFG_LRN_ALL_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_ARB_CFG_STATUS_STACK_CFG_LRN_ALL_CFG     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_ARB_CFG_STATUS_STACK_CFG_LRN_ALL_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief OQS priority queue of GCPU frames. One value per local, outbound CPU port
 *
 * \details
 * If the frame is to be forwarded to a GCPU port (instead of a local CPU
 * port) and STACK_CFG.GCPU_PRIO_MODE is set, then GCPU_PRIO_MAP is used to
 * choose a different priority queue in OQS depending on the local,
 * outbound CPU port, which the frame was originally destined to.
 * 
 * The intended use of this is to distribute frames to different OQS
 * priority queues of a local port, which connects to an external CPU.
 *
 * Register: \a ARB:CFG_STATUS:GCPU_PRIO_CFG
 *
 * @param ri Replicator: x_NUM_OUTBOUND_CPU_PORTS (??), 0-7
 */
#define VTSS_ARB_CFG_STATUS_GCPU_PRIO_CFG(ri)  VTSS_IOREG(VTSS_TO_ARB,0x47 + (ri))

/** 
 * \brief
 * OQS priority queue for CPU port.
 *
 * \details 
 * 0: Assign priority 0
 * ...
 * 7: Assign priority 7
 *
 * Field: ::VTSS_ARB_CFG_STATUS_GCPU_PRIO_CFG . GCPU_PRIO_MAP
 */
#define  VTSS_F_ARB_CFG_STATUS_GCPU_PRIO_CFG_GCPU_PRIO_MAP(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ARB_CFG_STATUS_GCPU_PRIO_CFG_GCPU_PRIO_MAP     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ARB_CFG_STATUS_GCPU_PRIO_CFG_GCPU_PRIO_MAP(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Mirror probe control
 *
 * \details
 * Configures the physical mirror port and mirror probe type for each
 * mirror probe.
 *
 * Register: \a ARB:CFG_STATUS:MIRROR_CFG
 *
 * @param ri Replicator: x_NUM_MIRROR_PROBES (??), 0-2
 */
#define VTSS_ARB_CFG_STATUS_MIRROR_CFG(ri)   VTSS_IOREG(VTSS_TO_ARB,0x4f + (ri))

/** 
 * \brief
 * The physical mirror port for the mirror probe.
 *
 * \details 
 * 0: Mirror port is 0
 * N: Mirror port is N
 *
 * Field: ::VTSS_ARB_CFG_STATUS_MIRROR_CFG . MIRROR_PROBE_CFG
 */
#define  VTSS_F_ARB_CFG_STATUS_MIRROR_CFG_MIRROR_PROBE_CFG(x)  VTSS_ENCODE_BITFIELD(x,1,6)
#define  VTSS_M_ARB_CFG_STATUS_MIRROR_CFG_MIRROR_PROBE_CFG     VTSS_ENCODE_BITMASK(1,6)
#define  VTSS_X_ARB_CFG_STATUS_MIRROR_CFG_MIRROR_PROBE_CFG(x)  VTSS_EXTRACT_BITFIELD(x,1,6)

/** 
 * \brief
 * The mirror type for the mirror probe.
 * 
 * The mirror type determines how the mirror copies are generated:
 *   - TX-Mirror: The mirror copy is only generated if the original frame
 * is transmitted.
 *   - RX-Mirror: The mirror copy is always generated but it can be lost 
 * if the mirror port is congested
 *
 * \details 
 * 0: Rx-Mirror probe.
 * 1: Tx-Mirror probe.
 *
 * Field: ::VTSS_ARB_CFG_STATUS_MIRROR_CFG . MIRROR_PROBE_TYPE
 */
#define  VTSS_F_ARB_CFG_STATUS_MIRROR_CFG_MIRROR_PROBE_TYPE  VTSS_BIT(0)


/** 
 * \brief Host Mode control
 *
 * \details
 * Register: \a ARB:CFG_STATUS:HM_CFG
 */
#define VTSS_ARB_CFG_STATUS_HM_CFG           VTSS_IOREG(VTSS_TO_ARB,0x52)

/** 
 * \brief
 * Enable host mode operation in the ARB
 *
 * \details 
 * 0: HM operation disabled
 * 1: Enable HM operation. 
 * A special <UPSID, UPSPN> range is reserved to encode the destination HM
 * queues (in OQS).
 *
 * Field: ::VTSS_ARB_CFG_STATUS_HM_CFG . HM_ENA
 */
#define  VTSS_F_ARB_CFG_STATUS_HM_CFG_HM_ENA  VTSS_BIT(0)

/** 
 * \brief
 * This register configures the UPSID range that is used to encode Host
 * Mode destination queues.
 * 
 * The reserved UPSID range consists of 8 UPSIDs. These 8 UPSIDs are used
 * as follows:
 * 0. UPSID: Used to signal CM for local ports from DSM to ARB.
 * 
 * 1. - 6. UPSID: Used to encode HM destinations queue and to signal CM for
 * these from DSM to ARB .
 * 
 * 7. UPSID: Not used.
 * 
 * When Host Mode is enabled, then the fill-level signalled directly from
 * OQS to ARB is ignored and instead this information is retrieved from the
 * CM information for the first UPSID in the reserved range. This way the
 * Arbiters in different chips in a multi-chip configuration gets to make
 * their arbitration decision on almost identical congestion information.
 *
 * \details 
 * 0: UPSID 0-7
 * 1: Reserved
 * 2: UPSID 16-23
 * 3: Reserved
 *
 * Field: ::VTSS_ARB_CFG_STATUS_HM_CFG . HM_UPSID_RANGE
 */
#define  VTSS_F_ARB_CFG_STATUS_HM_CFG_HM_UPSID_RANGE(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_ARB_CFG_STATUS_HM_CFG_HM_UPSID_RANGE     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_ARB_CFG_STATUS_HM_CFG_HM_UPSID_RANGE(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * Ethernet device, which is used as Host Mode Device A (HMDA). Must be
 * configured when HM_ENA is set.
 * 
 * This information must be configured consistently among the following
 * registers:
 * ANA_CL::HM_CFG.HMD_PORT
 * ANA_AC::HM_CFG.HMD_PORT
 * ARB::HM_CFG.ETH_DEV_HMDA / ARB::HM_CFG.ETH_DEV_HMDB
 * SCH::QSIF_HM_CTRL.ETH_DEV_HMDA / SCH::QSIF_HM_CTRL.ETH_DEV_HMDB
 * REW::HM_CTRL.ETH_DEV_HMDA / REW::HM_CTRL.ETH_DEV_HMDB
 * 
 * Normally this is one of the 10G ports, i.e. port 27-30.
 *
 * \details 
 * Field: ::VTSS_ARB_CFG_STATUS_HM_CFG . ETH_DEV_HMDA
 */
#define  VTSS_F_ARB_CFG_STATUS_HM_CFG_ETH_DEV_HMDA(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_ARB_CFG_STATUS_HM_CFG_ETH_DEV_HMDA     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_ARB_CFG_STATUS_HM_CFG_ETH_DEV_HMDA(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Ethernet device, which is used as Host Mode Device B (HMDB). For modes
 * with only one HM device active ETH_DEV_HMDB must be equal to
 * ETH_DEV_HMDA.
 * 
 * This information must be configured consistently among the following
 * registers:
 * ANA_CL::HM_CFG.HMD_PORT
 * ANA_AC::HM_CFG.HMD_PORT
 * ARB::HM_CFG.ETH_DEV_HMDA / ARB::HM_CFG.ETH_DEV_HMDB
 * SCH::QSIF_HM_CTRL.ETH_DEV_HMDA / SCH::QSIF_HM_CTRL.ETH_DEV_HMDB
 * REW::HM_CTRL.ETH_DEV_HMDA / REW::HM_CTRL.ETH_DEV_HMDB
 * 
 * Normally this is one of the 10G ports, i.e. port 27-30.
 *
 * \details 
 * Field: ::VTSS_ARB_CFG_STATUS_HM_CFG . ETH_DEV_HMDB
 */
#define  VTSS_F_ARB_CFG_STATUS_HM_CFG_ETH_DEV_HMDB(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_ARB_CFG_STATUS_HM_CFG_ETH_DEV_HMDB     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_ARB_CFG_STATUS_HM_CFG_ETH_DEV_HMDB(x)  VTSS_EXTRACT_BITFIELD(x,16,5)


/** 
 * \brief Congestion handling configuration
 *
 * \details
 * Register: \a ARB:CFG_STATUS:CH_CFG
 */
#define VTSS_ARB_CFG_STATUS_CH_CFG           VTSS_IOREG(VTSS_TO_ARB,0x54)

/** 
 * \brief
 * Enable Discard Eligibility (DE) based on Drop Precedence Level.
 * One bit per DP level.
 * 
 * When a DE is set, frames with the corresponding drop precedence level
 * are eligible to be discarded. I.e. the frames are regarded as yellow
 * traffic.
 * 
 * When a DE bit is not set, frames for this DP level will always be
 * forwarded to the OQS regardless of congestion status and drop or FC
 * mode. I.e. the frames are regarded as green traffic.
 *
 * \details 
 * 0: DE marking for the respective DP level is disabled
 * 1: DE marking for the respective DP level is enabled
 *
 * Field: ::VTSS_ARB_CFG_STATUS_CH_CFG . DE_ENA
 */
#define  VTSS_F_ARB_CFG_STATUS_CH_CFG_DE_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_ARB_CFG_STATUS_CH_CFG_DE_ENA     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_ARB_CFG_STATUS_CH_CFG_DE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Flush control
 *
 * \details
 * Using FLUSH_CTRL all frames in an IQS queue can be discarded.
 *
 * Register: \a ARB:CFG_STATUS:FLUSH_CTRL
 */
#define VTSS_ARB_CFG_STATUS_FLUSH_CTRL       VTSS_IOREG(VTSS_TO_ARB,0x55)

/** 
 * \brief
 * The priority number of the queue to flush.
 *
 * \details 
 * 0-7: Priority
 *
 * Field: ::VTSS_ARB_CFG_STATUS_FLUSH_CTRL . FLUSH_PRIO
 */
#define  VTSS_F_ARB_CFG_STATUS_FLUSH_CTRL_FLUSH_PRIO(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_ARB_CFG_STATUS_FLUSH_CTRL_FLUSH_PRIO     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_ARB_CFG_STATUS_FLUSH_CTRL_FLUSH_PRIO(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * The port number of the queue to flush.
 *
 * \details 
 * 0-N: Port number
 *
 * Field: ::VTSS_ARB_CFG_STATUS_FLUSH_CTRL . FLUSH_PORT
 */
#define  VTSS_F_ARB_CFG_STATUS_FLUSH_CTRL_FLUSH_PORT(x)  VTSS_ENCODE_BITFIELD(x,16,6)
#define  VTSS_M_ARB_CFG_STATUS_FLUSH_CTRL_FLUSH_PORT     VTSS_ENCODE_BITMASK(16,6)
#define  VTSS_X_ARB_CFG_STATUS_FLUSH_CTRL_FLUSH_PORT(x)  VTSS_EXTRACT_BITFIELD(x,16,6)

/** 
 * \brief
 * Control of the flush operation.
 * Initiates a flush when written to and indicates status of the flush
 * operation when read.
 * 
 * When initiating a flush operation, the queue to flush must be:
 * 1. Identified in FLUSH_PORT and FLUSH_PRIORITY, and 
 * 2. Disabled in IQS, ref. IQS::QU_RC_CFG.RX_QU_ENA.
 * 
 * These settings must remain unchanged while the flush operation is in
 * progress. It is recommended to disable all shapers in ARB for the port
 * that is flushed, because this will speed up the flush operation.
 *
 * \details 
 * Read
 * 0: No flush operation in progress
 * 1: Flush operation in progress
 * 
 * Write
 * 0: Reserved
 * 1: Initiate flush operation on the specified queue
 *
 * Field: ::VTSS_ARB_CFG_STATUS_FLUSH_CTRL . FLUSH_ENA
 */
#define  VTSS_F_ARB_CFG_STATUS_FLUSH_CTRL_FLUSH_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration of beating pattern protection mechanism
 *
 * \details
 * Register: \a ARB:CFG_STATUS:RND_GEN_CFG
 */
#define VTSS_ARB_CFG_STATUS_RND_GEN_CFG      VTSS_IOREG(VTSS_TO_ARB,0x56)

/**
 * Register Group: \a ARB:DROP_MODE_CFG
 *
 * Registers controlling the Drop modes in ARB
 */


/** 
 * \brief Configuration of drop mode per outbound Ethernet port
 *
 * \details
 * Register: \a ARB:DROP_MODE_CFG:OUTB_ETH_DROP_MODE_CFG
 *
 * @param ri Replicator: x_QS_PRIO (??), 0-7
 */
#define VTSS_ARB_DROP_MODE_CFG_OUTB_ETH_DROP_MODE_CFG(ri)  VTSS_IOREG(VTSS_TO_ARB,0x57 + (ri))


/** 
 * \brief Configuration of drop mode per outbound CPU port
 *
 * \details
 * Register: \a ARB:DROP_MODE_CFG:OUTB_CPU_DROP_MODE_CFG
 *
 * @param ri Replicator: x_QS_PRIO (??), 0-7
 */
#define VTSS_ARB_DROP_MODE_CFG_OUTB_CPU_DROP_MODE_CFG(ri)  VTSS_IOREG(VTSS_TO_ARB,0x5f + (ri))

/** 
 * \brief
 * Enable dropping of frames to outbound CPU port, regardless of ingress
 * drop mode (ref. ANA_CL::STACKING_CTRL.IGR_DROP_ENA).
 * 
 * This may be useful to avoid head of line blocking for ingress ports with
 * frames to an oversubscribed outbound port, e.g. a port interconnecting
 * chips in a multi-chip configuration.
 *
 * \details 
 * 0: Disabled. 
 * 1: Enabled.	The Arbiter is allowed to drop frames to the corresponding
 * outbound port if this is congested regardless of ingress drop mode.
 *
 * Field: ::VTSS_ARB_DROP_MODE_CFG_OUTB_CPU_DROP_MODE_CFG . OUTB_CPU_DROP_MODE_ENA
 */
#define  VTSS_F_ARB_DROP_MODE_CFG_OUTB_CPU_DROP_MODE_CFG_OUTB_CPU_DROP_MODE_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ARB_DROP_MODE_CFG_OUTB_CPU_DROP_MODE_CFG_OUTB_CPU_DROP_MODE_ENA     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ARB_DROP_MODE_CFG_OUTB_CPU_DROP_MODE_CFG_OUTB_CPU_DROP_MODE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Configuration of drop mode per outbound virtual device port
 *
 * \details
 * Register: \a ARB:DROP_MODE_CFG:OUTB_VD_DROP_MODE_CFG
 *
 * @param ri Replicator: x_QS_PRIO (??), 0-7
 */
#define VTSS_ARB_DROP_MODE_CFG_OUTB_VD_DROP_MODE_CFG(ri)  VTSS_IOREG(VTSS_TO_ARB,0x67 + (ri))

/** 
 * \brief
 * Enable force drop mode for frames send to the outbound virtual device
 * port
 *
 * \details 
 * 0: Disabled. 
 * 1: Enabled.	The Arbiter is allowed to drop frames to the corresponding
 * outbound port if this is congested independent of ingress drop- or FC
 * mode.
 *
 * Field: ::VTSS_ARB_DROP_MODE_CFG_OUTB_VD_DROP_MODE_CFG . OUTB_VD_DROP_MODE_ENA
 */
#define  VTSS_F_ARB_DROP_MODE_CFG_OUTB_VD_DROP_MODE_CFG_OUTB_VD_DROP_MODE_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration of multicast drop mode
 *
 * \details
 * Register: \a ARB:DROP_MODE_CFG:MC_DROP_MODE_CFG
 */
#define VTSS_ARB_DROP_MODE_CFG_MC_DROP_MODE_CFG  VTSS_IOREG(VTSS_TO_ARB,0x6f)

/** 
 * \brief
 * Enable dropping of all copies of multicast frames in a single operation.
 * 
 * If enabled, the dropped copies will not be counted by counters in OQS.
 * 
 * If disabled, each dropped copy of the frame will be arbitrated to the
 * intended OQS queue so it can be counted before being dropped.
 *
 * \details 
 * '0': Dropped copies of multicast frames are forwarded to OQS and to be
 * counted.
 * '1': Dropped copies of multicast frames are dropped immediately and not
 * counted.
 *
 * Field: ::VTSS_ARB_DROP_MODE_CFG_MC_DROP_MODE_CFG . MC_DROP_MODE
 */
#define  VTSS_F_ARB_DROP_MODE_CFG_MC_DROP_MODE_CFG_MC_DROP_MODE  VTSS_BIT(0)

/**
 * Register Group: \a ARB:AGING_CFG
 *
 * Configuration of aging
 */


/** 
 * \brief Configuration of era generation
 *
 * \details
 * This register holds information related to configuration of the era
 * generator.
 * 
 * The era generator outputs the current "era", i.e. 0 -> 1 -> 2 -> 3 -> 0
 * ...
 * 
 * The era is used for frame aging:
 * When the queuing system receives frames, it "stamps" them with the
 * current era. 
 * Whenever a frame gets two eras older than the current era, the frame
 * will be discarded.
 * 
 * Related registers:
 * SCH::ERA_CTRL
 * SCH::QSIF_ETH_PORT_CTRL.ETH_AGING_DIS
 * ARB::PORT_AGING_ENA_VEC0
 * ARB::PORT_AGING_ENA_VEC1
 * ARB::ERA_PRESCALER
 * SCH::ERA_PRESCALER

 *
 * Register: \a ARB:AGING_CFG:ERA_CTRL
 */
#define VTSS_ARB_AGING_CFG_ERA_CTRL          VTSS_IOREG(VTSS_TO_ARB,0x70)

/** 
 * \brief
 * Configure the aging interval. See also register ERA_PRESCALER
 * 
 * ERA_CTRL.ERA_ENA must be set to 0 when changing ERA_MODE.
 *
 * \details 
 * Tick interval with default PRESCALER_LOAD_VAL:
 * 
 * 0: tick set to 0.03125s
 * 1: tick set to 0.0625s
 * 2: tick set to 0.125s
 * 3: tick set to 0.25s
 * 4: tick set to 0.5s
 * 5: tick set to 1s
 * 6: tick set to 2s
 * 7: tick set to 4s
 *
 * Field: ::VTSS_ARB_AGING_CFG_ERA_CTRL . ERA_MODE
 */
#define  VTSS_F_ARB_AGING_CFG_ERA_CTRL_ERA_MODE(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ARB_AGING_CFG_ERA_CTRL_ERA_MODE     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ARB_AGING_CFG_ERA_CTRL_ERA_MODE(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Enable era generator.
 * 
 * If era generator is disabled, it will output a constant era value, i.e.
 * no frame aging will occur.
 * 
 * Before changing any of the following configuration parameters, the era
 * generator must be disabled:
 * ERA_CTRL.ERA_MODE
 * ERA_CTRL.ERA
 * ERA_PRESCALER.PRESCALER_LOAD_VALUE
 * 
 * Once configuration has been changed era generator may be re-enabled.
 *
 * \details 
 * 0: Disable era generation
 * 1: Enable era generation
 *
 * Field: ::VTSS_ARB_AGING_CFG_ERA_CTRL . ERA_ENA
 */
#define  VTSS_F_ARB_AGING_CFG_ERA_CTRL_ERA_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration of era prescaling
 *
 * \details
 * This register holds configuration parameters for the "prescaler" of the
 * era generator.
 * 
 * The era generator consists of a 28 bit  "prescaler" and a 7 bit "tick
 * counter".
 * 
 * Each time the prescaler reaches all-ones, the tick counter is
 * incremented by one and the prescaler is set to
 * ERA_PRESCALER.PRESCALER_LOAD_VALUE.
 * 
 * Each of the 7 bits in the tick counter corresponds to one of the era
 * lengths, specified in ERA_CTRL.ERA_MODE. I.e. when the tick counter bit
 * (chosen by ERA_MODE) changes from 0 to one, then the current era is
 * incremented.

 *
 * Register: \a ARB:AGING_CFG:ERA_PRESCALER
 */
#define VTSS_ARB_AGING_CFG_ERA_PRESCALER     VTSS_IOREG(VTSS_TO_ARB,0x71)

/** 
 * \brief
 * The value, which the prescaler is set to, when having reached all-ones. 
 * Should only be changed for debug purposes.
 * 
 * Using a different value than the default, will cause the era lengths
 * configurable in ERA_CTRL.ERA_MODE to change.
 * 
 * Due to limitation in the QSIF subblock of the scheduler, the
 * PRESCALER_LOAD_VALUE must never be set lower than ~2000 clock cycles.
 * 
 * ERA_CTRL.ERA_ENA must be set to 0 when changing PRESCALER_LOAD_VALUE.
 * 
 * The resulting era interval is:
 * 
 *   T = (2^28 - PRESCALE_LOAD_VAL) * 2 ^ ERA_MODE / 250000000
 *
 * \details 
 * N: The load value of the tick counter.
 *
 * Field: ::VTSS_ARB_AGING_CFG_ERA_PRESCALER . PRESCALER_LOAD_VAL
 */
#define  VTSS_F_ARB_AGING_CFG_ERA_PRESCALER_PRESCALER_LOAD_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,28)
#define  VTSS_M_ARB_AGING_CFG_ERA_PRESCALER_PRESCALER_LOAD_VAL     VTSS_ENCODE_BITMASK(0,28)
#define  VTSS_X_ARB_AGING_CFG_ERA_PRESCALER_PRESCALER_LOAD_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,28)


/** 
 * \brief Configuration of aging per port.
 *
 * \details
 * Configuration of aging per port.
 *
 * Register: \a ARB:AGING_CFG:PORT_AGING_ENA_VEC0
 */
#define VTSS_ARB_AGING_CFG_PORT_AGING_ENA_VEC0  VTSS_IOREG(VTSS_TO_ARB,0x72)


/** 
 * \brief Configuration of aging per port.
 *
 * \details
 * Configuration of aging per port.
 *
 * Register: \a ARB:AGING_CFG:PORT_AGING_ENA_VEC1
 */
#define VTSS_ARB_AGING_CFG_PORT_AGING_ENA_VEC1  VTSS_IOREG(VTSS_TO_ARB,0x73)

/** 
 * \brief
 * Controls if aging is enabled for each port.
 *
 * \details 
 * 0b000: Aging disabled
 * 0box1: Aging enabled for port 32
 * 0bx1x: Aging enabled for port 33
 * 0b1xx: Aging enabled for port 34
 * 
 * Multiple ports can be enables at the same time
 *
 * Field: ::VTSS_ARB_AGING_CFG_PORT_AGING_ENA_VEC1 . PORT_AGING_VEC1_ENA
 */
#define  VTSS_F_ARB_AGING_CFG_PORT_AGING_ENA_VEC1_PORT_AGING_VEC1_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ARB_AGING_CFG_PORT_AGING_ENA_VEC1_PORT_AGING_VEC1_ENA     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ARB_AGING_CFG_PORT_AGING_ENA_VEC1_PORT_AGING_VEC1_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a ARB:STICKY
 *
 * Sticky bit status
 */


/** 
 * \brief Queue overflow sticky 
 *
 * \details
 * Register: \a ARB:STICKY:REQ_QU_OFLW_VEC0
 *
 * @param ri Replicator: x_NUM_IQS_PRIOS (??), 0-7
 */
#define VTSS_ARB_STICKY_REQ_QU_OFLW_VEC0(ri)  VTSS_IOREG(VTSS_TO_ARB,0x74 + (ri))


/** 
 * \brief Queue overflow sticky 
 *
 * \details
 * Register: \a ARB:STICKY:REQ_QU_OFLW_VEC1
 *
 * @param ri Replicator: x_NUM_IQS_PRIOS (??), 0-7
 */
#define VTSS_ARB_STICKY_REQ_QU_OFLW_VEC1(ri)  VTSS_IOREG(VTSS_TO_ARB,0x7c + (ri))


/** 
 * \brief Queue empty sticky 
 *
 * \details
 * Register: \a ARB:STICKY:REQ_QU_EMPTY
 */
#define VTSS_ARB_STICKY_REQ_QU_EMPTY         VTSS_IOREG(VTSS_TO_ARB,0x84)


/** 
 * \brief Queue not empty sticky 
 *
 * \details
 * Register: \a ARB:STICKY:REQ_QU_NON_EMPTY
 */
#define VTSS_ARB_STICKY_REQ_QU_NON_EMPTY     VTSS_IOREG(VTSS_TO_ARB,0x85)


/** 
 * \brief OQS Flow Control Status
 *
 * \details
 * Sticky bits for OQS Ethernet queue-level flow control signals
 *
 * Register: \a ARB:STICKY:OQS_ETH_QU_FC_STATUS
 *
 * @param ri Replicator: x_NUM_IQS_PRIOS (??), 0-7
 */
#define VTSS_ARB_STICKY_OQS_ETH_QU_FC_STATUS(ri)  VTSS_IOREG(VTSS_TO_ARB,0x86 + (ri))


/** 
 * \brief OQS Flow Control Status
 *
 * \details
 * Sticky bits for OQS MC queue-level flow control signals
 *
 * Register: \a ARB:STICKY:OQS_VD_QU_FC_STATUS
 *
 * @param ri Replicator: x_NUM_IQS_PRIOS (??), 0-7
 */
#define VTSS_ARB_STICKY_OQS_VD_QU_FC_STATUS(ri)  VTSS_IOREG(VTSS_TO_ARB,0x8e + (ri))

/** 
 * \brief
 * Sticky bit indicating if the queue in the OQS has signaled flow control
 * to the ARB.
 *
 * \details 
 * 0: The queue has not signaled flow control.
 * 1: The queue has signaled flow control.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ARB_STICKY_OQS_VD_QU_FC_STATUS . OQS_VD_QU_FC_STICKY
 */
#define  VTSS_F_ARB_STICKY_OQS_VD_QU_FC_STATUS_OQS_VD_QU_FC_STICKY  VTSS_BIT(0)


/** 
 * \brief OQS Flow Control Status
 *
 * \details
 * Sticky bits for OQS CPU queue-level flow control signals
 *
 * Register: \a ARB:STICKY:OQS_CPU_QU_FC_STATUS
 *
 * @param ri Replicator: x_NUM_IQS_PRIOS (??), 0-7
 */
#define VTSS_ARB_STICKY_OQS_CPU_QU_FC_STATUS(ri)  VTSS_IOREG(VTSS_TO_ARB,0x96 + (ri))

/** 
 * \brief
 * Sticky bit indicating if the queue in the OQS has signaled flow control
 * to the ARB.
 *
 * \details 
 * 0: The queue has not signaled flow control.
 * 1: The queue has signaled flow control.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ARB_STICKY_OQS_CPU_QU_FC_STATUS . OQS_CPU_QU_FC_STICKY
 */
#define  VTSS_F_ARB_STICKY_OQS_CPU_QU_FC_STATUS_OQS_CPU_QU_FC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ARB_STICKY_OQS_CPU_QU_FC_STATUS_OQS_CPU_QU_FC_STICKY     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ARB_STICKY_OQS_CPU_QU_FC_STATUS_OQS_CPU_QU_FC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief OQS Flow Control Status
 *
 * \details
 * Sticky bits for OQS port-level flow control signals
 *
 * Register: \a ARB:STICKY:OQS_ETH_PORT_FC_STATUS
 */
#define VTSS_ARB_STICKY_OQS_ETH_PORT_FC_STATUS  VTSS_IOREG(VTSS_TO_ARB,0x9e)


/** 
 * \brief OQS Flow Control Status
 *
 * \details
 * Sticky bits for OQS port-level flow control signals
 *
 * Register: \a ARB:STICKY:OQS_VD_PORT_FC_STATUS
 */
#define VTSS_ARB_STICKY_OQS_VD_PORT_FC_STATUS  VTSS_IOREG(VTSS_TO_ARB,0x9f)

/** 
 * \brief
 * Sticky bit indicating if the port in the OQS has signaled flow control
 * to the ARB.
 *
 * \details 
 * 0: The port has not signaled flow control.
 * 1: The port has signaled flow control.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ARB_STICKY_OQS_VD_PORT_FC_STATUS . OQS_VD_PORT_FC_STICKY
 */
#define  VTSS_F_ARB_STICKY_OQS_VD_PORT_FC_STATUS_OQS_VD_PORT_FC_STICKY  VTSS_BIT(0)


/** 
 * \brief OQS Flow Control Status
 *
 * \details
 * Sticky bits for OQS port-level flow control signals
 *
 * Register: \a ARB:STICKY:OQS_CPU_PORT_FC_STATUS
 */
#define VTSS_ARB_STICKY_OQS_CPU_PORT_FC_STATUS  VTSS_IOREG(VTSS_TO_ARB,0xa0)

/** 
 * \brief
 * Sticky bit indicating if the port in the OQS has signaled flow control
 * to the ARB.
 *
 * \details 
 * 0: The port has not signaled flow control.
 * 1: The port has signaled flow control.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ARB_STICKY_OQS_CPU_PORT_FC_STATUS . OQS_CPU_PORT_FC_STICKY
 */
#define  VTSS_F_ARB_STICKY_OQS_CPU_PORT_FC_STATUS_OQS_CPU_PORT_FC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ARB_STICKY_OQS_CPU_PORT_FC_STATUS_OQS_CPU_PORT_FC_STICKY     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ARB_STICKY_OQS_CPU_PORT_FC_STATUS_OQS_CPU_PORT_FC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief OQS Flow Control Status
 *
 * \details
 * Sticky bits for OQS priority-level flow control signals
 *
 * Register: \a ARB:STICKY:OQS_PRIO_FC_STATUS
 */
#define VTSS_ARB_STICKY_OQS_PRIO_FC_STATUS   VTSS_IOREG(VTSS_TO_ARB,0xa1)

/** 
 * \brief
 * Sticky bit indicating if the priority in the OQS has signaled flow
 * control to the ARB.
 *
 * \details 
 * 0: The priority has not signaled flow control.
 * 1: The priority has signaled flow control.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ARB_STICKY_OQS_PRIO_FC_STATUS . OQS_PRIO_FC_STICKY
 */
#define  VTSS_F_ARB_STICKY_OQS_PRIO_FC_STATUS_OQS_PRIO_FC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ARB_STICKY_OQS_PRIO_FC_STATUS_OQS_PRIO_FC_STICKY     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ARB_STICKY_OQS_PRIO_FC_STATUS_OQS_PRIO_FC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief OQS Flow Control Status
 *
 * \details
 * Sticky bits for OQS buffer-level flow control signals
 *
 * Register: \a ARB:STICKY:OQS_BUF_FC_STATUS
 */
#define VTSS_ARB_STICKY_OQS_BUF_FC_STATUS    VTSS_IOREG(VTSS_TO_ARB,0xa2)

/** 
 * \brief
 * Sticky bit indicating if the buffer in the OQS has signaled flow control
 * to the ARB.
 *
 * \details 
 * 0: The buffer has not signaled flow control.
 * 1: The buffer has signaled flow control.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ARB_STICKY_OQS_BUF_FC_STATUS . OQS_BUF_FC_STICKY
 */
#define  VTSS_F_ARB_STICKY_OQS_BUF_FC_STATUS_OQS_BUF_FC_STICKY  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_ARB_H_ */
