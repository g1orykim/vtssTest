#ifndef _VTSS_JAGUAR_REGS_SCH_H_
#define _VTSS_JAGUAR_REGS_SCH_H_

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
 * Target: \a SCH
 *
 * Scheduler
 *
 ***********************************************************************/

/**
 * Register Group: \a SCH:QSIF
 *
 * QSIF SCH registers
 */


/** 
 * \brief Flush Control
 *
 * \details
 * Register: \a SCH:QSIF:QSIF_FLUSH_CTRL
 */
#define VTSS_SCH_QSIF_QSIF_FLUSH_CTRL        VTSS_IOREG(VTSS_TO_SCH,0xa1c)

/** 
 * \brief
 * Enable flush of queue identified by FLUSH_PORT & FLUSH_PRIO.
 * 
 * Flush will schedule all frames on the queue for discard. 
 * 
 * When flush has been completed, the FLUSH_ENA bit will be cleared.
 * 
 * Before writing '1' to FLUSH_ENA, the following must be done:
 * #1:
 * FLUSH_PORT and FLUSH_PRIO must have been setup. FLUSH_PORT and
 * FLUSH_PRIO must not be changed until the chip has cleared FLUSH_ENA.
 * 
 * #2:
 * It must be ensured that no new cells/frames can enter the queue in the
 * Queuing System. This is achieved by disabling the queue (or the whole
 * port to which the queue belongs) using one of the following registers:
 *   PORT_RC_CFG.RX_PORT_ENA
 *   QU_RC_CFG.RX_QU_ENA
 * 
 * After flushing, the corresponding buffer in DSM should be flushed as
 * well (to avoid fragment when restarting transmission). Ref.
 * DSM::CLR_BUF.
 *
 * \details 
 * Field: ::VTSS_SCH_QSIF_QSIF_FLUSH_CTRL . FLUSH_ENA
 */
#define  VTSS_F_SCH_QSIF_QSIF_FLUSH_CTRL_FLUSH_ENA  VTSS_BIT(0)

/** 
 * \brief
 * See FLUSH_ENA.
 *
 * \details 
 * Field: ::VTSS_SCH_QSIF_QSIF_FLUSH_CTRL . FLUSH_PORT
 */
#define  VTSS_F_SCH_QSIF_QSIF_FLUSH_CTRL_FLUSH_PORT(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_SCH_QSIF_QSIF_FLUSH_CTRL_FLUSH_PORT     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_SCH_QSIF_QSIF_FLUSH_CTRL_FLUSH_PORT(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * See FLUSH_ENA.
 *
 * \details 
 * Field: ::VTSS_SCH_QSIF_QSIF_FLUSH_CTRL . FLUSH_PRIO
 */
#define  VTSS_F_SCH_QSIF_QSIF_FLUSH_CTRL_FLUSH_PRIO(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_SCH_QSIF_QSIF_FLUSH_CTRL_FLUSH_PRIO     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_SCH_QSIF_QSIF_FLUSH_CTRL_FLUSH_PRIO(x)  VTSS_EXTRACT_BITFIELD(x,16,3)


/** 
 * \brief Various QSIF control information.
 *
 * \details
 * Register: \a SCH:QSIF:QSIF_CTRL
 */
#define VTSS_SCH_QSIF_QSIF_CTRL              VTSS_IOREG(VTSS_TO_SCH,0xa1d)

/** 
 * \brief
 * Start initialization of the MCTX RAM (internal control RAM in the QSIF
 * block).
 * The initialization takes one clock cycle per queue.
 *
 * \details 
 * Field: ::VTSS_SCH_QSIF_QSIF_CTRL . RAM_INIT_MCTX
 */
#define  VTSS_F_SCH_QSIF_QSIF_CTRL_RAM_INIT_MCTX  VTSS_BIT(2)


/** 
 * \brief Various Host Mode control
 *
 * \details
 * Register: \a SCH:QSIF:QSIF_HM_CTRL
 */
#define VTSS_SCH_QSIF_QSIF_HM_CTRL           VTSS_IOREG(VTSS_TO_SCH,0xa1e)

/** 
 * \brief
 * Controls how the 192 host mode queues are assigned to the two Ethernet
 * devices/ports operating in host mode (HMDA and HMDB), and how these
 * queues are mapped into virtual ports (VPs) and priorities within a
 * virtual port.
 * 
 * The configuration of HM_MODE must be consistent with
 * HM_SCH_CTRL.HM_MODE.
 * 
 * The Ethernet device/port number, which HMDA and HMDB is mapped to, is
 * configured in 
 * QSIF_HM_CTRL.ETH_DEV_HMDA/ETH_DEV_HMDB
 *
 * \details 
 * 0: Mode 12x8 - 12 VPs each with 8 priorities on HMDA and HMDB
 * 1: Mode 24x4 - 24 VPs each with 4 priorities on HMDA and HMDB
 * 2: Mode 24x8 - 24 VPs each with 8 priorities on HMDA
 * 3: Mode 48x4 - 48 VPs each with 4 priorities on HMDA
 *
 * Field: ::VTSS_SCH_QSIF_QSIF_HM_CTRL . HM_MODE
 */
#define  VTSS_F_SCH_QSIF_QSIF_HM_CTRL_HM_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_SCH_QSIF_QSIF_HM_CTRL_HM_MODE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_SCH_QSIF_QSIF_HM_CTRL_HM_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/** 
 * \brief
 * Ethernet device/port, which is used as Host Mode Device A (HMDA).
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
 * Must be configured during startup, i.e. before the Ethernet port is up.
 * 
 * Note:
 * When HMDA is not used, then this field must be set to an unused port
 * number.
 *
 * \details 
 * Field: ::VTSS_SCH_QSIF_QSIF_HM_CTRL . ETH_DEV_HMDA
 */
#define  VTSS_F_SCH_QSIF_QSIF_HM_CTRL_ETH_DEV_HMDA(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_SCH_QSIF_QSIF_HM_CTRL_ETH_DEV_HMDA     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_SCH_QSIF_QSIF_HM_CTRL_ETH_DEV_HMDA(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Ethernet device/port, which is used as Host Mode Device B (HMDB).
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
 * Must be configured during startup, i.e. before the Ethernet port is up.
 * 
 * Note:
 * When HMDB is not used, then this field must be set to an unused port
 * number.
 *
 * \details 
 * Field: ::VTSS_SCH_QSIF_QSIF_HM_CTRL . ETH_DEV_HMDB
 */
#define  VTSS_F_SCH_QSIF_QSIF_HM_CTRL_ETH_DEV_HMDB(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_SCH_QSIF_QSIF_HM_CTRL_ETH_DEV_HMDB     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_SCH_QSIF_QSIF_HM_CTRL_ETH_DEV_HMDB(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Swap XAUI outbound status channels.
 *
 * \details 
 * 0:
 * HMDA uses XAUI status channel 0
 * HMDB uses XAUI status channel 1
 * 
 * 1:
 * HMDA uses XAUI status channel 1
 * HMDB uses XAUI status channel 0
 *
 * Field: ::VTSS_SCH_QSIF_QSIF_HM_CTRL . SWAP_XAUI_OBS
 */
#define  VTSS_F_SCH_QSIF_QSIF_HM_CTRL_SWAP_XAUI_OBS  VTSS_BIT(24)


/** 
 * \brief Enable queue-level flow control for Host Mode device A (HMDA)
 *
 * \details
 * Enable out-of-band queue-level flow control for host mode queues using
 * XAUI status channel.
 * 
 * If flow control is disabled, then transmission is allowed (unless
 * stopped by other types of flow control).
 * 
 * The first replication of this register holds bits 0-31, the second bits
 * 32-63, etc.
 * 
 * The following examples shows how the bits are used depending on the mode
 * configured in QSIF_HM_CTRL.HM_MODE.
 * 
 * Examples:
 * 
 * HM_MODE=12x8:
 * Bit 0: Priority 0 on virtual port 0
 * Bit 1: Priority 1 on virtual port 0
 * ...
 * Bit 8: Priority 0 on virtual port 1
 * ...
 * Bit 95: Priority 7 on virtual port 11
 * Bit 96-191: Unused
 * 
 * HM_MODE=24x4:
 * Bit 0: Priority 0 on virtual port 0
 * Bit 1: Priority 1 on virtual port 0
 * ...
 * Bit 95: Priority 3 on virtual port 23
 * Bit 96-191: Unused
 * 
 * HM_MODE=24x8:
 * Bit 0: Priority 0 on virtual port 0
 * Bit 1: Priority 1 on virtual port 0
 * ...
 * Bit 191: Priority 7 on virtual port 23
 * 
 * HM_MODE=48x4
 * Bit 0: Priority 0 on virtual port 0
 * Bit 1: Priority 1 on virtual port 0
 * ...
 * Bit 191: Priority 3 on virtual port 47

 *
 * Register: \a SCH:QSIF:QSIF_HMDA_QU_FC_ENA
 *
 * @param ri Replicator: x_SCH_DEVHM_QU_CNT_DIV32 (??), 0-5
 */
#define VTSS_SCH_QSIF_QSIF_HMDA_QU_FC_ENA(ri)  VTSS_IOREG(VTSS_TO_SCH,0xa1f + (ri))


/** 
 * \brief Enable queue-level flow control for Host Mode device B (HMDB)
 *
 * \details
 * Enable out-of-band queue-level flow control for host mode queues using
 * XAUI status channel.
 * 
 * If flow control is disabled, then transmission is allowed (unless
 * stopped by other types of flow control).
 * 
 * The first replication of this register holds bits 0-31, the second bits
 * 32-63, etc.
 * 
 * The following examples shows how the bits are used depending on the mode
 * configured in QSIF_HM_CTRL.HM_MODE.
 * 
 * Examples:
 * 
 * HM_MODE=12x8:
 * Bit 0: Priority 0 on virtual port 0
 * Bit 1: Priority 1 on virtual port 0
 * ...
 * Bit 8: Priority 0 on virtual port 1
 * ...
 * Bit 95: Priority 7 on virtual port 11
 * Bit 96-191: Unused
 * 
 * HM_MODE=24x4:
 * Bit 0: Priority 0 on virtual port 0
 * Bit 1: Priority 1 on virtual port 0
 * ...
 * Bit 95: Priority 3 on virtual port 23
 * Bit 96-191: Unused

 *
 * Register: \a SCH:QSIF:QSIF_HMDB_QU_FC_ENA
 *
 * @param ri Replicator: x_SCH_DEVHM_QU_CNT_DIV32 (??), 0-5
 */
#define VTSS_SCH_QSIF_QSIF_HMDB_QU_FC_ENA(ri)  VTSS_IOREG(VTSS_TO_SCH,0xa25 + (ri))


/** 
 * \brief Enable VP-level flow control for Host Mode device A (HMDA)
 *
 * \details
 * Register: \a SCH:QSIF:QSIF_HMDA_VP_FC_ENA
 *
 * @param ri Replicator: x_SCH_DEVHM_VP_CNT_PER_HMD_DIV32 (??), 0-1
 */
#define VTSS_SCH_QSIF_QSIF_HMDA_VP_FC_ENA(ri)  VTSS_IOREG(VTSS_TO_SCH,0xa2b + (ri))


/** 
 * \brief Enable VP-level flow control for Host Mode device B (HMDB)
 *
 * \details
 * Register: \a SCH:QSIF:QSIF_HMDB_VP_FC_ENA
 *
 * @param ri Replicator: x_SCH_DEVHM_VP_CNT_PER_HMD_DIV32 (??), 0-1
 */
#define VTSS_SCH_QSIF_QSIF_HMDB_VP_FC_ENA(ri)  VTSS_IOREG(VTSS_TO_SCH,0xa2d + (ri))


/** 
 * \brief QSIF Port Control for Ethernet ports
 *
 * \details
 * Register: \a SCH:QSIF:QSIF_ETH_PORT_CTRL
 *
 * @param ri Replicator: x_SCH_DEVETH_CNT (??), 0-31
 */
#define VTSS_SCH_QSIF_QSIF_ETH_PORT_CTRL(ri)  VTSS_IOREG(VTSS_TO_SCH,0xa53 + (ri))

/** 
 * \brief
 * Disable transmission for the queue at next frame boundary.
 *
 * \details 
 * Bit 0: Priority 0
 * Bit 1: Priority 1
 * ...

 *
 * Field: ::VTSS_SCH_QSIF_QSIF_ETH_PORT_CTRL . ETH_TX_DIS
 */
#define  VTSS_F_SCH_QSIF_QSIF_ETH_PORT_CTRL_ETH_TX_DIS(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_SCH_QSIF_QSIF_ETH_PORT_CTRL_ETH_TX_DIS     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_SCH_QSIF_QSIF_ETH_PORT_CTRL_ETH_TX_DIS(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/** 
 * \brief
 * Disable aging for port.
 *
 * \details 
 * Field: ::VTSS_SCH_QSIF_QSIF_ETH_PORT_CTRL . ETH_AGING_DIS
 */
#define  VTSS_F_SCH_QSIF_QSIF_ETH_PORT_CTRL_ETH_AGING_DIS  VTSS_BIT(8)


/** 
 * \brief QSIF Port Control for Virtual Device ports
 *
 * \details
 * Register: \a SCH:QSIF:QSIF_VD_PORT_CTRL
 */
#define VTSS_SCH_QSIF_QSIF_VD_PORT_CTRL      VTSS_IOREG(VTSS_TO_SCH,0xa73)

/** 
 * \brief
 * Disable transmission for the queue at next frame boundary.
 *
 * \details 
 * Bit 0: Priority 0
 * Bit 1: Priority 1
 * ...

 *
 * Field: ::VTSS_SCH_QSIF_QSIF_VD_PORT_CTRL . VD_TX_DIS
 */
#define  VTSS_F_SCH_QSIF_QSIF_VD_PORT_CTRL_VD_TX_DIS(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_SCH_QSIF_QSIF_VD_PORT_CTRL_VD_TX_DIS     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_SCH_QSIF_QSIF_VD_PORT_CTRL_VD_TX_DIS(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/** 
 * \brief
 * Disable aging for port.
 *
 * \details 
 * Field: ::VTSS_SCH_QSIF_QSIF_VD_PORT_CTRL . VD_AGING_DIS
 */
#define  VTSS_F_SCH_QSIF_QSIF_VD_PORT_CTRL_VD_AGING_DIS  VTSS_BIT(8)


/** 
 * \brief QSIF Port Control for HM ports
 *
 * \details
 * Port control for each HM queue.
 * 
 * Note that one QSIF_HM_PORT_CTRL register is present for each HM queue,
 * regardless of QSIF_HM_CTRL::HM_MODE.
 *
 * Register: \a SCH:QSIF:QSIF_HM_PORT_CTRL
 *
 * @param ri Replicator: x_SCH_HM_PORT_CNT (??), 0-191
 */
#define VTSS_SCH_QSIF_QSIF_HM_PORT_CTRL(ri)  VTSS_IOREG(VTSS_TO_SCH,0xa74 + (ri))

/** 
 * \brief
 * Disable transmission for the queue at next frame boundary.
 *
 * \details 
 * Field: ::VTSS_SCH_QSIF_QSIF_HM_PORT_CTRL . HM_TX_DIS
 */
#define  VTSS_F_SCH_QSIF_QSIF_HM_PORT_CTRL_HM_TX_DIS  VTSS_BIT(0)


/** 
 * \brief QSIF Port Control for CPU ports
 *
 * \details
 * Register: \a SCH:QSIF:QSIF_CPU_PORT_CTRL
 *
 * @param ri Replicator: x_SCH_CPU_PORT_CNT (??), 0-7
 */
#define VTSS_SCH_QSIF_QSIF_CPU_PORT_CTRL(ri)  VTSS_IOREG(VTSS_TO_SCH,0xb34 + (ri))

/** 
 * \brief
 * Disable transmission for the queue at next frame boundary.
 *
 * \details 
 * Field: ::VTSS_SCH_QSIF_QSIF_CPU_PORT_CTRL . CPU_TX_DIS
 */
#define  VTSS_F_SCH_QSIF_QSIF_CPU_PORT_CTRL_CPU_TX_DIS  VTSS_BIT(0)

/**
 * Register Group: \a SCH:HM
 *
 * Registers related to HM Scheduler
 */


/** 
 * \brief DWRR cost configuration
 *
 * \details
 * Mapping of the DWRR Cost address depends on the HM_SCH_CTRL.HM_MODE
 * register:
 * 
 * Mode 12x8:
 * Address 0-11: Cost values for VP 0-11 on HMDA
 * Address 12-23: Cost values for VP 0-11 on HMDB
 * Address 24-31: Cost values for queues 0-5 on VP 0 on HMDA
 * Address 32-37: Cost values for queues 0-5 on VP 1 on HMDA
 * ...
 * Address 90-95: Cost values for queues 0-5 on VP 11 on HMDA
 * Address 96-101: Cost values for queues 0-5 on VP 0 on HMDB
 * Address 102-107: Cost values for queues 0-5 on VP 1 on HMDB
 * ...
 * Address 162-167: Cost values for queues 0-5 on VP 11 on HMDB
 * 
 * Mode 24x4:
 * Address 0-23: Cost values for VP 0-23 on HMDA.
 * Address 24-47: Cost values for VP 0-23 on HMDB.
 * Address 48-49: Cost values for queue 0-1 on VP 0 on HMDA
 * Address 50-51: Cost values for queue 0-1 on VP 1 on HMDA
 * ...
 * Address 94-95: Cost values for queue 0-1 on VP 23 on HMDA
 * Address 96-97: Cost values for queue 0-1 on VP 0 on HMDB
 * Address 98-99: Cost values for queue 0-1 on VP 1 on HMDB
 * ...
 * Address 142-143: Cost values for queue 0-1 on VP 23 on HMDB
 * 
 * Mode 24x8:
 * Address 0-23: Cost values for VP 0-23 on HMDA.
 * Address 24-31: Cost values for queues 0-5 on VP 0 on HMDA
 * Address 32-37: Cost values for queues 0-5 on VP 1 on HMDA
 * ...
 * Address 162-167: Cost values for queues 0-5 on VP 23 on HMDA
 * 
 * Mode 48x4:
 * Address 0-47: Cost values for VP 0-47 on HMDA
 * Address 48-49: Cost values for queue 0-1 on VP 0 on HMDA
 * Address 50-51: Cost values for queue 0-1 on VP 1 on HMDA
 * ...
 * Address 142-143: Cost values for queue 0-1 on VP 47 on HMDA

 *
 * Register: \a SCH:HM:HM_DWRR_COST
 *
 * @param ri Replicator: x_SCH_DEVHM_DWRR_CFG_CNT (??), 0-191
 */
#define VTSS_SCH_HM_HM_DWRR_COST(ri)         VTSS_IOREG(VTSS_TO_SCH,0xb4b + (ri))

/** 
 * \brief
 * The cost associated with scheduling a byte from this each queue/virtual
 * port (VP). Used by the Deficit Weighted Round Robin (DWRR) algorithm.
 * The higher the cost, the less bandwidth will be reserved to the
 * queue/VP. 
 * 
 * The DWRR cost values are mapped to virtual ports (VP) and queues
 * depending on the mode configured in HM_SCH_CTRL.HM_MODE. The one or two
 * Ethernet devices/ports operating in host mode are termed HMDA and HMDB.
 * 
 * Regardless of mode the two highest priorities are always strict and not
 * included in the DWRR scheduling. 

 *
 * \details 
 * 0: Cost 1
 * 1: Cost 2
 * ...
 * 127: Cost 128
 *
 * Field: ::VTSS_SCH_HM_HM_DWRR_COST . DWRR_COST
 */
#define  VTSS_F_SCH_HM_HM_DWRR_COST_DWRR_COST(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_SCH_HM_HM_DWRR_COST_DWRR_COST     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_SCH_HM_HM_DWRR_COST_DWRR_COST(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief Additional configuration parameters for virtual ports
 *
 * \details
 * The configured values are mapped to virtual ports (VP) depending on the
 * mode configured in HM_SCH_CTRL.HM_MODE. The one or two Ethernet
 * devices/ports operating in host mode are termed HMDA and HMDB.
 * 
 * Mode 12x8:
 * Address 0: VP 0 on HMDA
 * Address 1: VP 1 on HMDA
 * ...
 * Address 12: VP 11 on HMDA
 * Address 13: VP 0 on HMDB
 * Address 14: VP 1 on HMDB
 * ...
 * Address 24: VP 11 on HMDB
 * 
 * Mode 24x4:
 * Address 0: VP 0 on HMDA
 * Address 1: VP 1 on HMDA
 * ...
 * Address 23: VP 23 on HMDA
 * Address 24: VP 0 on HMDB
 * Address 25: VP 1 on HMDB
 * ...
 * Address 47: VP 23 on HMDB
 * 
 * Mode 24x8:
 * Address 0: VP 0 on HMDA
 * Address 1: VP 1 on HMDA
 * ...
 * Address 23: VP 23 on HMDA
 * 
 * Mode 48x4:
 * Address 0: VP 0 on HMDA
 * Address 1: VP 1 on HMDA
 * ...
 * Address 47: VP 47 on HMDA
 *
 * Register: \a SCH:HM:HM_DWRR_AND_LB
 *
 * @param ri Replicator: x_SCH_DEVHM_LPORT_CNT (??), 0-47
 */
#define VTSS_SCH_HM_HM_DWRR_AND_LB(ri)       VTSS_IOREG(VTSS_TO_SCH,0xc0b + (ri))

/** 
 * \brief
 * If enabled, the value configured in HM_SCH_CTRL.FRM_ADJ will be added to
 * the frame length for each frame. 
 * 
 * The modified frame length will be used by both the leaky bucket and DWRR
 * algorithm. 
 *
 * \details 
 * 0: Disable frame length adjustment.
 * 1: Enable frame length adjustment.
 *
 * Field: ::VTSS_SCH_HM_HM_DWRR_AND_LB . FRM_ADJ_ENA
 */
#define  VTSS_F_SCH_HM_HM_DWRR_AND_LB_FRM_ADJ_ENA  VTSS_BIT(0)


/** 
 * \brief Additional configuration parameters for virtual ports
 *
 * \details
 * The configured values are mapped to virtual ports and queue	(VP, Q)
 * depending on the mode configured in HM_SCH_CTRL.HM_MODE. The one or two
 * Ethernet devices/ports operating in host mode are termed HMDA and HMDB.
 * 
 * Each VP is assigned to 3 leaky buckets. One LB is controlling the total
 * VP rate and two are controlling the rates of the two highest priorities
 * within the VP. Each of the leaky buckets can be enabled individually.
 * 
 * Mode 12x8:
 * Address 0: VP 0, Q6 on HMDA
 * Address 1: VP 0, Q7 on HMDA
 * Address 2: VP 0 on HMDA
 * ...
 * Address 34: VP 11, Q7 on HMDA
 * Address 35: VP 11 on HMDA
 * Address 36: VP 0, Q6 on HMDB
 * Address 37: VP 0, Q7 on HMDB
 * ...
 * Address 70: VP 11, Q7 on HMDB
 * Address 71: VP 11 on HMDB
 * 
 * Mode 24x4:
 * Address 0: VP 0, Q2 on HMDA
 * Address 1: VP 0, Q3 on HMDA
 * Address 2: VP 0 on HMDA
 * ...
 * Address 70: VP 23, Q3 on HMDA
 * Address 71: VP 23 on HMDA
 * Address 72: VP 0, Q2 on HMDB
 * Address 73: VP 0, Q3 on HMDB
 * Address 74: VP 0, on HMDB
 * ...
 * Address 142: VP 23, Q3 on HMDB
 * Address 143: VP 23 on HMDB
 * 
 * Mode 24x8:
 * Address 0: VP 0, Q6 on HMDA
 * Address 1: VP 0, Q7 on HMDA
 * Address 2: VP 0 on HMDA
 * ...
 * Address 70: VP 23, Q3 on HMDA
 * Address 71: VP 23 on HMDA
 * 
 * Mode 48x4:
 * Address 0: VP 0, Q2 on HMDA
 * Address 1: VP 0, Q3 on HMDA
 * Address 2: VP 0 on HMDA
 * ...
 * Address 142: VP 47, Q3 on HMDA
 * Address 143: VP 47 on HMDA

 *
 * Register: \a SCH:HM:HM_LB_CFG
 *
 * @param ri Replicator: x_SCH_DEVHM_LB_CNT (??), 0-143
 */
#define VTSS_SCH_HM_HM_LB_CFG(ri)            VTSS_IOREG(VTSS_TO_SCH,0xc3b + (ri))

/** 
 * \brief
 * Please refer to HM_LB for details.
 * 
 * Note that prior to enabling leaky bucket shaping, an initialization must
 * be performed, ref. HM_SCH_CTRL.DWRR_INIT.
 *
 * \details 
 * 0: Disable leaky bucket for VP, Q
 * 1: Enable leaky bucket for VP, Q.
 *
 * Field: ::VTSS_SCH_HM_HM_LB_CFG . LB_SHAPING_ENA
 */
#define  VTSS_F_SCH_HM_HM_LB_CFG_LB_SHAPING_ENA  VTSS_BIT(0)


/** 
 * \brief Overall HM scheduler control.
 *
 * \details
 * Register: \a SCH:HM:HM_SCH_CTRL
 */
#define VTSS_SCH_HM_HM_SCH_CTRL              VTSS_IOREG(VTSS_TO_SCH,0xccb)

/** 
 * \brief
 * Controls how the 192 host mode queues are assigned to the two Ethernet
 * devices/ports operating in host mode (HMDA and HMDB), and how these
 * queues are mapped into virtual ports (VPs) and priorities within a
 * virtual port.
 * 
 * The configuration of HM_MODE must be consistent with
 * QSIF_HM_CTRL.HM_MODE.
 * 
 * The Ethernet device/port number, which HMDA and HMDB are mapped to, is
 * configured in 
 * QSIF_HM_CTRL.ETH_DEV_HMDA/ETH_DEV_HMDB
 *
 * \details 
 * 0: Mode 12x8 - 12 VPs each with 8 priorities on HMDA and HMDB
 * 1: Mode 24x4 - 24 VPs each with 4 priorities on HMDA and HMDB
 * 2: Mode 24x8 - 24 VPs each with 8 priorities on HMDA
 * 3: Mode 48x4 - 48 VPs each with 4 priorities on HMDA
 *
 * Field: ::VTSS_SCH_HM_HM_SCH_CTRL . HM_MODE
 */
#define  VTSS_F_SCH_HM_HM_SCH_CTRL_HM_MODE(x)  VTSS_ENCODE_BITFIELD(x,9,2)
#define  VTSS_M_SCH_HM_HM_SCH_CTRL_HM_MODE     VTSS_ENCODE_BITMASK(9,2)
#define  VTSS_X_SCH_HM_HM_SCH_CTRL_HM_MODE(x)  VTSS_EXTRACT_BITFIELD(x,9,2)

/** 
 * \brief
 * Value to be added to frame length when updating DWRR and LBs state each
 * time a frame is scheduled.
 * If set to 20, this will correspond to inclusion of min. Ethernet IFG and
 * preamble in shaping.
 * 
 * This feature can be enabled on a per virtual port basis, using
 * HM_DWRR_AND_LB.FRM_ADJ_ENA.
 *
 * \details 
 * 0-31: Number of bytes added at start of frame
 *
 * Field: ::VTSS_SCH_HM_HM_SCH_CTRL . FRM_ADJ
 */
#define  VTSS_F_SCH_HM_HM_SCH_CTRL_FRM_ADJ(x)  VTSS_ENCODE_BITFIELD(x,3,5)
#define  VTSS_M_SCH_HM_HM_SCH_CTRL_FRM_ADJ     VTSS_ENCODE_BITMASK(3,5)
#define  VTSS_X_SCH_HM_HM_SCH_CTRL_FRM_ADJ(x)  VTSS_EXTRACT_BITFIELD(x,3,5)

/** 
 * \brief
 * Force a complete initialization of the leaky bucket state- and
 * configuration.
 * 
 * Initialization must be performed before rates are configured and leaky
 * buckets enabled. Normally this should be done during startup.
 *
 * \details 
 * 0: No Action
 * 1: Force initialization.
 *
 * Field: ::VTSS_SCH_HM_HM_SCH_CTRL . LB_INIT
 */
#define  VTSS_F_SCH_HM_HM_SCH_CTRL_LB_INIT    VTSS_BIT(2)

/**
 * Register Group: \a SCH:HM_LB
 *
 * Configuration of leaky buckets for shaping virtual ports.
 */


/** 
 * \brief HM Leaky bucket threshold
 *
 * \details
 * Register: \a SCH:HM_LB:HM_LB_THRES
 *
 * @param ri Replicator: x_SCH_DEVHM_LB_CNT (??), 0-143
 */
#define VTSS_SCH_HM_LB_HM_LB_THRES(ri)       VTSS_IOREG(VTSS_TO_SCH,0x800 + (ri))

/** 
 * \brief
 * Max burst size configuration for shaping of virtual ports.
 * 
 * The unit is 4kB (1kB = 1024Bytes). 
 * The largest supported threshold is 252kB when the register value is set
 * to all "1"s.
 * 
 * Only applicable if HM_LB_CFG.LB_SHAPING_ENA=1.
 * 
 * Prior to leaky bucket configuration, an initialization must be
 * performed, ref. HM_SCH_CTRL.LB_INIT.
 *
 * \details 
 * 0: Always closed
 * 1: Burst capacity = 4096 bytes
 * ...
 * n: Burst capacity = n x 4096 bytes
 *
 * Field: ::VTSS_SCH_HM_LB_HM_LB_THRES . LB_THRES
 */
#define  VTSS_F_SCH_HM_LB_HM_LB_THRES_LB_THRES(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_SCH_HM_LB_HM_LB_THRES_LB_THRES     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_SCH_HM_LB_HM_LB_THRES_LB_THRES(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HM Leaky bucket rate
 *
 * \details
 * Register: \a SCH:HM_LB:HM_LB_RATE
 *
 * @param ri Replicator: x_SCH_DEVHM_LB_CNT (??), 0-143
 */
#define VTSS_SCH_HM_LB_HM_LB_RATE(ri)        VTSS_IOREG(VTSS_TO_SCH,0x900 + (ri))

/** 
 * \brief
 * Leaky bucket rate in units of 100160 bps for shaping of virtual port.
 * 
 * Only applicable if HM_LB_CFG.LB_SHAPING_ENA=1.
 * 
 * Prior to leaky bucket configuration, an initialization must be
 * performed, ref. HM_SCH_CTRL.LB_INIT.
 *
 * \details 
 * 0: Open until burst capacity is used, then closed.
 * 1: Rate = 100160 bps
 * n: Rate = n x 100160 bps
 *
 * Field: ::VTSS_SCH_HM_LB_HM_LB_RATE . LB_RATE
 */
#define  VTSS_F_SCH_HM_LB_HM_LB_RATE_LB_RATE(x)  VTSS_ENCODE_BITFIELD(x,0,17)
#define  VTSS_M_SCH_HM_LB_HM_LB_RATE_LB_RATE     VTSS_ENCODE_BITMASK(0,17)
#define  VTSS_X_SCH_HM_LB_HM_LB_RATE_LB_RATE(x)  VTSS_EXTRACT_BITFIELD(x,0,17)

/**
 * Register Group: \a SCH:ETH
 *
 * ETH SCH registers
 */


/** 
 * \brief Ethernet device scheduler control register
 *
 * \details
 * Register: \a SCH:ETH:ETH_CTRL
 */
#define VTSS_SCH_ETH_ETH_CTRL                VTSS_IOREG(VTSS_TO_SCH,0xccc)

/** 
 * \brief
 * Setting this bit allows the virtual device to use excess bandwidth. 
 * 
 * I.e. if the port - to which a cell slot is assigned - has not data to
 * transmit, then the virtual device may use the cell slot.
 * 
 * This is the only case where a cell slot can be utilized by a different
 * port than the one it is assigned to in CBC.
 *
 * \details 
 * 0: Disable usage of excess bandwidth
 * 1: Enable usage of excess bandwidth for virtual device (VD)
 *
 * Field: ::VTSS_SCH_ETH_ETH_CTRL . VD_EXCESS_BW_ENA
 */
#define  VTSS_F_SCH_ETH_ETH_CTRL_VD_EXCESS_BW_ENA  VTSS_BIT(1)


/** 
 * \brief ETH port pause frame mode
 *
 * \details
 * Register: \a SCH:ETH:ETH_PORT_PAUSE_FRM_MODE
 *
 * @param ri Replicator: x_SCH_DEVETHVD_CNT (??), 0-32
 */
#define VTSS_SCH_ETH_ETH_PORT_PAUSE_FRM_MODE(ri)  VTSS_IOREG(VTSS_TO_SCH,0xcd6 + (ri))

/** 
 * \brief
 * Specify what to do, when receiving port pause frame from DSM.
 * 
 * Normally pause frames are handled only by DSM, but a non-standard mode
 * is supported, where DSM will just pass the flow-control on to SCH. SCH
 * can then be setup to only apply the flow-control to some queues.
 * 
 * Related registers:
 * SCH::ETH_PORT_PAUSE_FRM_PRIO_MASK
 * DSM::RX_PAUSE_CFG.FC_OBEY_LOCAL
 *
 * \details 
 * 0x0 : No action.
 * 0x1 : Stop transmission (on next frame boundary) for all priorities .
 * 0x2 : Stop transmission (on next frame boundary) on the priorities
 *	    specified in ETH_PORT_PAUSE_FRM_PRIO_MASK.
 * 0x3 : Reserved.
 *
 * Field: ::VTSS_SCH_ETH_ETH_PORT_PAUSE_FRM_MODE . PORT_PAUSE_FRM_MODE
 */
#define  VTSS_F_SCH_ETH_ETH_PORT_PAUSE_FRM_MODE_PORT_PAUSE_FRM_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_SCH_ETH_ETH_PORT_PAUSE_FRM_MODE_PORT_PAUSE_FRM_MODE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_SCH_ETH_ETH_PORT_PAUSE_FRM_MODE_PORT_PAUSE_FRM_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief ETH port pause frame priority mask
 *
 * \details
 * Register: \a SCH:ETH:ETH_PORT_PAUSE_FRM_PRIO_MASK
 */
#define VTSS_SCH_ETH_ETH_PORT_PAUSE_FRM_PRIO_MASK  VTSS_IOREG(VTSS_TO_SCH,0xcf7)

/** 
 * \brief
 * Bitmask w. one bit per priority, specifying which priorities shall
 * respect port-based Ethernet pause frames.
 * In normal operation Ethernet pause frames are handled by DSM and thus
 * ignored by SCH, but if only selected priorities shall respect pause
 * frames, then must be controlled by SCH.
 * 
 * Related registers:
 * SCH::ETH_PORT_PAUSE_FRM_MODE
 * DSM::RX_PAUSE_CFG.FC_OBEY_LOCAL
 *
 * \details 
 * Field: ::VTSS_SCH_ETH_ETH_PORT_PAUSE_FRM_PRIO_MASK . PORT_PAUSE_FRM_PRIO_MASK
 */
#define  VTSS_F_SCH_ETH_ETH_PORT_PAUSE_FRM_PRIO_MASK_PORT_PAUSE_FRM_PRIO_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_SCH_ETH_ETH_PORT_PAUSE_FRM_PRIO_MASK_PORT_PAUSE_FRM_PRIO_MASK     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_SCH_ETH_ETH_PORT_PAUSE_FRM_PRIO_MASK_PORT_PAUSE_FRM_PRIO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief ETH Leaky bucket frame adjustment
 *
 * \details
 * Register: \a SCH:ETH:ETH_LB_DWRR_FRM_ADJ
 */
#define VTSS_SCH_ETH_ETH_LB_DWRR_FRM_ADJ     VTSS_IOREG(VTSS_TO_SCH,0xcf8)

/** 
 * \brief
 * Value to be added to leaky buckets and DWRR each time a frame is
 * scheduled. If set to 20, this will correspond to inclusion of min.
 * Ethernet IFG and preamble in shaping.
 * 
 * This is enabled per port in
 * SCH::ETH_LB_DWRR_CFG.FRM_ADJ_ENA
 *
 * \details 
 * 0-31: Number of bytes added at start of frame
 *
 * Field: ::VTSS_SCH_ETH_ETH_LB_DWRR_FRM_ADJ . FRM_ADJ
 */
#define  VTSS_F_SCH_ETH_ETH_LB_DWRR_FRM_ADJ_FRM_ADJ(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_SCH_ETH_ETH_LB_DWRR_FRM_ADJ_FRM_ADJ     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_SCH_ETH_ETH_LB_DWRR_FRM_ADJ_FRM_ADJ(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief ETH leaky bucked frame adjustment
 *
 * \details
 * Register: \a SCH:ETH:ETH_LB_DWRR_CFG
 *
 * @param ri Replicator: x_SCH_DEVETHVD_CNT (??), 0-32
 */
#define VTSS_SCH_ETH_ETH_LB_DWRR_CFG(ri)     VTSS_IOREG(VTSS_TO_SCH,0xcf9 + (ri))

/** 
 * \brief
 * If enabled, the value configured in ETH_LB_DWRR_FRM_ADJ.FRM_ADJ will be
 * added to the frame length for each frame. 
 * 
 * The modified frame length will be used by both the leaky bucket and DWRR
 * algorithm. 
 *
 * \details 
 * 0:Disable frame length adjustment.
 * 1:Enable frame length adjustment.
 *
 * Field: ::VTSS_SCH_ETH_ETH_LB_DWRR_CFG . FRM_ADJ_ENA
 */
#define  VTSS_F_SCH_ETH_ETH_LB_DWRR_CFG_FRM_ADJ_ENA  VTSS_BIT(0)


/** 
 * \brief Ethernet device scheduler deficit weighted round robin control register
 *
 * \details
 * Register: \a SCH:ETH:ETH_DWRR_CFG
 *
 * @param ri Replicator: x_SCH_DEVETHVD_CNT (??), 0-32
 */
#define VTSS_SCH_ETH_ETH_DWRR_CFG(ri)        VTSS_IOREG(VTSS_TO_SCH,0xd1a + (ri))

/** 
 * \brief
 * Configure DWRR scheduling for port. Weighted- and strict prioritization
 * can be configured.
 *
 * \details 
 * 0: All priorities are scheduled strict
 * 1: The two highest priorities (6 ,7) are strict. The rest are DWRR.

 *
 * Field: ::VTSS_SCH_ETH_ETH_DWRR_CFG . DWRR_MODE
 */
#define  VTSS_F_SCH_ETH_ETH_DWRR_CFG_DWRR_MODE  VTSS_BIT(30)

/** 
 * \brief
 * DWRR cost configuration per queue.
 *
 * \details 
 * 0: Cost 1
 * 1: Cost 2
 * ...
 * 31: Cost 32
 *
 * Field: ::VTSS_SCH_ETH_ETH_DWRR_CFG . COST_CFG
 */
#define  VTSS_F_SCH_ETH_ETH_DWRR_CFG_COST_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,30)
#define  VTSS_M_SCH_ETH_ETH_DWRR_CFG_COST_CFG     VTSS_ENCODE_BITMASK(0,30)
#define  VTSS_X_SCH_ETH_ETH_DWRR_CFG_COST_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,30)


/** 
 * \brief Ethernet device scheduler shaping control register
 *
 * \details
 * Register: \a SCH:ETH:ETH_SHAPING_CTRL
 *
 * @param ri Replicator: x_SCH_DEVETHVD_CNT (??), 0-32
 */
#define VTSS_SCH_ETH_ETH_SHAPING_CTRL(ri)    VTSS_IOREG(VTSS_TO_SCH,0xd3c + (ri))

/** 
 * \brief
 * Enable port and priority shaping for port. 
 * 
 * If only some shapers need to be enabled, this can be achieved by setting
 * SHAPING_ENA for the port, and setting the rate and threshold to maximum
 * for any shapers that shall effectively be disabled.
 *
 * \details 
 * 0:  Disable port and priority shapers for port.
 * 1:  Enable port and priority shapers for port.
 *
 * Field: ::VTSS_SCH_ETH_ETH_SHAPING_CTRL . SHAPING_ENA
 */
#define  VTSS_F_SCH_ETH_ETH_SHAPING_CTRL_SHAPING_ENA  VTSS_BIT(0)

/** 
 * \brief
 * Allow this queue to use excess bandwidth in case none of the priorities
 * are allowed (by their priority LB) to transmit.
 * 
 * The resulting BW of a queue is a function of the port- and queue LBs,
 * the DWRR and the excess enable bit:
 * 1)	  Port LB closed: Hold back frames except for discards.
 * 2)	  Port LB open: Use DWRR to distribute traffic between open Queue
 * LBs
 * 3)	  All Queue LBs closed: Hold back frames except for discards and
 * Queues which have PRIO_LB_EXS_ENA set. The excess BW is distributed
 * using DWRR. 

 *
 * \details 
 * xxx1: Enable excess BW for Prio 0
 * xx1x: Enable excess BW for Prio 1
 * ...
 * 1xxx: Enable excess BW for Prio N
 *
 * Field: ::VTSS_SCH_ETH_ETH_SHAPING_CTRL . PRIO_LB_EXS_ENA
 */
#define  VTSS_F_SCH_ETH_ETH_SHAPING_CTRL_PRIO_LB_EXS_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_SCH_ETH_ETH_SHAPING_CTRL_PRIO_LB_EXS_ENA     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_SCH_ETH_ETH_SHAPING_CTRL_PRIO_LB_EXS_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,8)


/** 
 * \brief ETH leaky bucket control
 *
 * \details
 * Register: \a SCH:ETH:ETH_LB_CTRL
 */
#define VTSS_SCH_ETH_ETH_LB_CTRL             VTSS_IOREG(VTSS_TO_SCH,0xd5d)

/** 
 * \brief
 * Set to 1 to force a complete re-initialization of state and
 * configuration of leaky bucket,
 * Automatically cleared whether re-initialization is done.
 * 
 * Initialization must be performed before rates are configured and leaky
 * buckets enabled. Normally this should be done during startup.
 *
 * \details 
 * 0: No Action
 * 1: Force initialization.
 *
 * Field: ::VTSS_SCH_ETH_ETH_LB_CTRL . LB_INIT
 */
#define  VTSS_F_SCH_ETH_ETH_LB_CTRL_LB_INIT   VTSS_BIT(2)

/**
 * Register Group: \a SCH:ETH_LB
 *
 * ETH leaky bucket SCH registers
 */


/** 
 * \brief Ethernet leaky bucket threshold
 *
 * \details
 * Register: \a SCH:ETH_LB:ETH_LB_THRES
 *
 * @param ri Replicator: x_SCH_ETH_LB_CNT (??), 0-296
 */
#define VTSS_SCH_ETH_LB_ETH_LB_THRES(ri)     VTSS_IOREG(VTSS_TO_SCH,0x0 + (ri))

/** 
 * \brief
 * Burst capacity of leaky buckets
 * 
 * The unit is 4KB (1KB = 1024Bytes). The largest supported threshold is
 * 252KB
 * when the register value is set to all "1"s.
 * 
 * Only applicable if ETH_SHAPING_CTRL.SHAPING_ENA=1. 
 * 
 * Prior to leaky bucket configuration, an initialization must be
 * performed, ref. ETH_LB_CTRL.LB_INIT.
 *
 * \details 
 * 0: Always closed
 * 1: Burst capacity = 4096 bytes
 * ...
 * n: Burst capacity = n x 4096 bytes
 *
 * Field: ::VTSS_SCH_ETH_LB_ETH_LB_THRES . LB_THRES
 */
#define  VTSS_F_SCH_ETH_LB_ETH_LB_THRES_LB_THRES(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_SCH_ETH_LB_ETH_LB_THRES_LB_THRES     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_SCH_ETH_LB_ETH_LB_THRES_LB_THRES(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Ethernet leaky bucket rate 
 *
 * \details
 * Register: \a SCH:ETH_LB:ETH_LB_RATE
 *
 * @param ri Replicator: x_SCH_ETH_LB_CNT (??), 0-296
 */
#define VTSS_SCH_ETH_LB_ETH_LB_RATE(ri)      VTSS_IOREG(VTSS_TO_SCH,0x200 + (ri))

/** 
 * \brief
 * Leaky bucket rate in units of 100160 bps for shaping of port/priority.
 * 
 * Only applicable if ETH_SHAPING_CTRL.SHAPING_ENA=1. 
 * 
 * Prior to leaky bucket configuration, an initialization must be
 * performed, ref. ETH_LB_CTRL.LB_INIT.
 *
 * \details 
 * 0: Open until burst capacity is used, then closed.
 * 1: Rate = 100160 bps
 * n: Rate = n x 100160 bps

 *
 * Field: ::VTSS_SCH_ETH_LB_ETH_LB_RATE . LB_RATE
 */
#define  VTSS_F_SCH_ETH_LB_ETH_LB_RATE_LB_RATE(x)  VTSS_ENCODE_BITFIELD(x,0,17)
#define  VTSS_M_SCH_ETH_LB_ETH_LB_RATE_LB_RATE     VTSS_ENCODE_BITMASK(0,17)
#define  VTSS_X_SCH_ETH_LB_ETH_LB_RATE_LB_RATE(x)  VTSS_EXTRACT_BITFIELD(x,0,17)

/**
 * Register Group: \a SCH:CPU
 *
 * Registers related to CPU Scheduler
 */

/**
 * Register Group: \a SCH:CBC
 *
 * CBC SCH registers
 */


/** 
 * \brief CBC Length Configuration
 *
 * \details
 * Register: \a SCH:CBC:CBC_LEN_CFG
 */
#define VTSS_SCH_CBC_CBC_LEN_CFG             VTSS_IOREG(VTSS_TO_SCH,0xd64)

/** 
 * \brief
 * Length of Cell Bus Calendar.
 * 
 * The content (i.e. the device IDs) of the calendar is configured in
 * CBC_DEV_ID_CFG.CBC_DEV_ID.
 * 
 * If CBC_LEN is set to 0, then the Scheduler will not schedule any
 * cells/frames for transmission.
 * 
 * While writing device IDs to CBC_DEV_ID.DEV_ID, CBC_LEN must be set to 0.
 * 
 * 
 * Pls. also refer to CBC_DEV_ID.DEV_ID.

 *
 * \details 
 * Field: ::VTSS_SCH_CBC_CBC_LEN_CFG . CBC_LEN
 */
#define  VTSS_F_SCH_CBC_CBC_LEN_CFG_CBC_LEN(x)  VTSS_ENCODE_BITFIELD(x,0,11)
#define  VTSS_M_SCH_CBC_CBC_LEN_CFG_CBC_LEN     VTSS_ENCODE_BITMASK(0,11)
#define  VTSS_X_SCH_CBC_CBC_LEN_CFG_CBC_LEN(x)  VTSS_EXTRACT_BITFIELD(x,0,11)

/**
 * Register Group: \a SCH:CBC_DEV_ID_CFG
 *
 * Configuration of Device IDs in Cell Bus Calendar
 */


/** 
 * \brief Configuration of Device IDs in Cell Bus Calendar
 *
 * \details
 * Register: \a SCH:CBC_DEV_ID_CFG:CBC_DEV_ID
 *
 * @param ri Replicator: x_SCH_CBC_RAM_DEPTH (??), 0-1023
 */
#define VTSS_SCH_CBC_DEV_ID_CFG_CBC_DEV_ID(ri)  VTSS_IOREG(VTSS_TO_SCH,0x400 + (ri))

/** 
 * \brief
 * Device IDs in the Cell Bus Calendar.
 * 
 * The device IDs must be written to the addresses 0 to <calendar length> -
 * 1.
 * 
 * During configuration of the device IDs in the Cell Bus Calendar,
 * CBC_LEN_CFG.CBC_LEN must be set to 0. Once all the desired device IDs
 * have been written, <calendar length> must be written to
 * CBC_LEN_CFG.CBC_LEN.
 *
 * \details 
 * Devices are numbered from 0 - <number of devices>-1. 

 *
 * Field: ::VTSS_SCH_CBC_DEV_ID_CFG_CBC_DEV_ID . DEV_ID
 */
#define  VTSS_F_SCH_CBC_DEV_ID_CFG_CBC_DEV_ID_DEV_ID(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_SCH_CBC_DEV_ID_CFG_CBC_DEV_ID_DEV_ID     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_SCH_CBC_DEV_ID_CFG_CBC_DEV_ID_DEV_ID(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a SCH:MISC
 *
 * Miscellaneous SCH registers
 */


/** 
 * \brief Era Generator Control Register
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
 * ARB::ERA_CTRL
 * SCH::QSIF_ETH_PORT_CTRL.ETH_AGING_DIS
 * ARB::PORT_AGING_ENA_VEC0
 * ARB::PORT_AGING_ENA_VEC1
 * ARB::ERA_PRESCALER
 * SCH::ERA_PRESCALER

 *
 * Register: \a SCH:MISC:ERA_CTRL
 */
#define VTSS_SCH_MISC_ERA_CTRL               VTSS_IOREG(VTSS_TO_SCH,0xd65)

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
 * '0': Disable era generator
 * '1': Enable era generator

 *
 * Field: ::VTSS_SCH_MISC_ERA_CTRL . ERA_ENA
 */
#define  VTSS_F_SCH_MISC_ERA_CTRL_ERA_ENA     VTSS_BIT(0)

/** 
 * \brief
 * Configure the aging interval. See also register ERA_PRESCALER.
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
 * Field: ::VTSS_SCH_MISC_ERA_CTRL . ERA_MODE
 */
#define  VTSS_F_SCH_MISC_ERA_CTRL_ERA_MODE(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_SCH_MISC_ERA_CTRL_ERA_MODE     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_SCH_MISC_ERA_CTRL_ERA_MODE(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Current era value.
 * 
 * ERA_CTRL.ERA_ENA must be set to 0 when writing to ERA_CTRL.ERA.

 *
 * \details 
 * 0-3: the current aging era.
 *
 * Field: ::VTSS_SCH_MISC_ERA_CTRL . ERA
 */
#define  VTSS_F_SCH_MISC_ERA_CTRL_ERA(x)      VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_SCH_MISC_ERA_CTRL_ERA         VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_SCH_MISC_ERA_CTRL_ERA(x)      VTSS_EXTRACT_BITFIELD(x,8,2)


/** 
 * \brief Era Generator Prescaler Register
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
 * Register: \a SCH:MISC:ERA_PRESCALER
 */
#define VTSS_SCH_MISC_ERA_PRESCALER          VTSS_IOREG(VTSS_TO_SCH,0xd66)

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
 * Field: ::VTSS_SCH_MISC_ERA_PRESCALER . PRESCALER_LOAD_VALUE
 */
#define  VTSS_F_SCH_MISC_ERA_PRESCALER_PRESCALER_LOAD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,28)
#define  VTSS_M_SCH_MISC_ERA_PRESCALER_PRESCALER_LOAD_VALUE     VTSS_ENCODE_BITMASK(0,28)
#define  VTSS_X_SCH_MISC_ERA_PRESCALER_PRESCALER_LOAD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,28)


/** 
 * \brief RAM integrity errors register.
 *
 * \details
 * Register: \a SCH:MISC:RAM_INTEGRITY_ERR_STICKY
 */
#define VTSS_SCH_MISC_RAM_INTEGRITY_ERR_STICKY  VTSS_IOREG(VTSS_TO_SCH,0xd67)

/** 
 * \brief
 * Integrity error indication for control information in MCTX RAM.
 * Chip must be reset if such error occurs.
 * 
 * Bit is cleared by writing a '1' to this position.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': RAM integrity check error occurred

 *
 * Field: ::VTSS_SCH_MISC_RAM_INTEGRITY_ERR_STICKY . RAM_INTEGRITY_ERR_QSIF_MCTX_STICKY
 */
#define  VTSS_F_SCH_MISC_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERR_QSIF_MCTX_STICKY  VTSS_BIT(0)

/** 
 * \brief
 * Integrity error indication for frame request in FIFO RAM.
 * Chip must be reset if such error occurs.
 * 
 * Bit is cleared by writing a '1' to this position.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': RAM integrity check error occurred

 *
 * Field: ::VTSS_SCH_MISC_RAM_INTEGRITY_ERR_STICKY . RAM_INTEGRITY_ERR_QSIF_FRF_STICKY
 */
#define  VTSS_F_SCH_MISC_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERR_QSIF_FRF_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Integrity error indication in calendar RAM.
 * Chip must be reset if such error occurs.
 * 
 * Bit is cleared by writing a '1' to this position.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': RAM integrity check error occurred

 *
 * Field: ::VTSS_SCH_MISC_RAM_INTEGRITY_ERR_STICKY . RAM_INTEGRITY_ERR_CBC_STICKY
 */
#define  VTSS_F_SCH_MISC_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERR_CBC_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Integrity error indication in leaky bucket RAMs of the Ethernet
 * scheduler.
 * 
 * Bit is cleared by writing a '1' to this position.
 *
 * \details 
 * Field: ::VTSS_SCH_MISC_RAM_INTEGRITY_ERR_STICKY . RAM_INTEGRITY_ERR_EDS_LB_STICKY
 */
#define  VTSS_F_SCH_MISC_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERR_EDS_LB_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Integrity error indication in leaky bucket RAMs of HM scheduler.
 * 
 * Bit is cleared by writing a '1' to this position. 
 *
 * \details 
 * Field: ::VTSS_SCH_MISC_RAM_INTEGRITY_ERR_STICKY . RAM_INTEGRITY_ERR_HMDS_LB_STICKY
 */
#define  VTSS_F_SCH_MISC_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERR_HMDS_LB_STICKY  VTSS_BIT(4)


#endif /* _VTSS_JAGUAR_REGS_SCH_H_ */
