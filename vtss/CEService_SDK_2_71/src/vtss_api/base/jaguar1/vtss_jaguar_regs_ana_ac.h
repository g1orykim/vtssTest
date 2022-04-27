#ifndef _VTSS_JAGUAR_REGS_ANA_AC_H_
#define _VTSS_JAGUAR_REGS_ANA_AC_H_

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
 * Target: \a ANA_AC
 *
 * Controls mask handling etc.
 *
 ***********************************************************************/

/**
 * Register Group: \a ANA_AC:PS_COMMON
 *
 * Common configurations for all ports
 */


/** 
 * \brief Miscellaneously Control
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:MISC_CTRL
 */
#define VTSS_ANA_AC_PS_COMMON_MISC_CTRL      VTSS_IOREG(VTSS_TO_ANA_AC,0x1ba63)

/** 
 * \brief
 * Allows CPU dest mask in IFH.CL_RES.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_MISC_CTRL . CPU_DEST_MASK_IN_CL_RES_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_MISC_CTRL_CPU_DEST_MASK_IN_CL_RES_ENA  VTSS_BIT(9)

/** 
 * \brief
 * Enable use of vid as counter index.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_MISC_CTRL . USE_VID_AS_ISDX_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_MISC_CTRL_USE_VID_AS_ISDX_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Increase DP level for flooded traffic.
 *
 * \details 
 * 0: Disabled
 * 1: Increase DP by 1 for flooded traffic
 * 2: Increase DP by 2 for flooded traffic
 * 3: Increase DP by 3 for flooded traffic
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_MISC_CTRL . BMAC_FLOOD_DP_INC
 */
#define  VTSS_F_ANA_AC_PS_COMMON_MISC_CTRL_BMAC_FLOOD_DP_INC(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_ANA_AC_PS_COMMON_MISC_CTRL_BMAC_FLOOD_DP_INC     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_ANA_AC_PS_COMMON_MISC_CTRL_BMAC_FLOOD_DP_INC(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * Increase DP level for flooded traffic.
 *
 * \details 
 * 0: Disabled
 * 1: Increase DP by 1 for flooded traffic
 * 2: Increase DP by 2 for flooded traffic
 * 3: Increase DP by 3 for flooded traffic
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_MISC_CTRL . CMAC_FLOOD_DP_INC
 */
#define  VTSS_F_ANA_AC_PS_COMMON_MISC_CTRL_CMAC_FLOOD_DP_INC(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_ANA_AC_PS_COMMON_MISC_CTRL_CMAC_FLOOD_DP_INC     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_ANA_AC_PS_COMMON_MISC_CTRL_CMAC_FLOOD_DP_INC(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Configures how Rx timestamp is added to the frame.
 * See OAM_RX_TSTAMP_IN_FCS_ENA for full description.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_MISC_CTRL . OAM_RX_TSTAMP_FRM_EXTEND_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_MISC_CTRL_OAM_RX_TSTAMP_FRM_EXTEND_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Configures how Rx timestamp is added to frames, which have been selected
 * for timestamping in VCAP_IS2 (ref.
 * VCAP_IS2:BASETYPE_ACTION:A.OAM_RX_TIMESTAMP_ENA).
 * 
 * In combination with OEM_RX_TSTAMP_FRM_EXTEND_ENA three different modes
 * for providing the Rx timestamp value inband with an Ethernet frame to
 * the CPU are available:
 * 
 * 1) Overwriting the frame's FCS field with the Rx timestamp value. The
 * frame length is unchanged. This frame format is proprietary and
 * performing FCS checks on these frames will fail. This option must be
 * used when copying the frame to CPU (with Rx timestamp written to frame's
 * FCS field) while the original frame is forwarded as normal (with correct
 * FCS). 
 * To select this mode set 
 * OAM_RX_TSTAMP_IN_FCS_ENA=1
 * OAM_RX_TSTAMP_FRM_EXTEND_ENA=0
 * 
 * 2) Adding the timestamp value as a four-byte frame extension after the
 * frame's FCS field. FCS is not recalculated. In this mode, the original
 * frame content is unchanged. The frame format is proprietary. This mode
 * can be used while redirecting timestamped frames to CPU. 
 * To select this mode set 
 * OAM_RX_TSTAMP_IN_FCS_ENA=0
 * OAM_RX_TSTAMP_FRM_EXTEND_ENA=1
 * 
 * 3) Adding the timestamp value as a four-byte frame extension just in
 * front of the frame's FCS field (last four bytes of frame payload). FCS
 * is recalculated to include the timestamp value so the frame is changed
 * from its original. The frame format is a standard Ethernet frame, but
 * includes proprietary information. This mode can be used while
 * redirecting timestamped frames to CPU. 
 * To select this mode set 
 * OAM_RX_TSTAMP_IN_FCS_ENA=1
 * OAM_RX_TSTAMP_FRM_EXTEND_ENA=1
 * 
 * The second and third mode only works for Ethernet frames of size less
 * than 132 bytes.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_MISC_CTRL . OAM_RX_TSTAMP_IN_FCS_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_MISC_CTRL_OAM_RX_TSTAMP_IN_FCS_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Avoid CPU sending back to CPU.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_MISC_CTRL . CPU_TO_CPU_KILL_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_MISC_CTRL_CPU_TO_CPU_KILL_ENA  VTSS_BIT(1)


/** 
 * \brief Host mode configuration 
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:HM_CFG
 *
 * @param ri Replicator: x_FFL_ANA_NUM_HIH_DEVS (??), 0-3
 */
#define VTSS_ANA_AC_PS_COMMON_HM_CFG(ri)     VTSS_IOREG(VTSS_TO_ANA_AC,0x1ba64 + (ri))

/** 
 * \brief
 * Port using HIH based forwarding if valid (ANA_AC::HM_CFG.HMD_PORT_VLD).
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
 * \details 
 * Field: ::VTSS_ANA_AC_PS_COMMON_HM_CFG . HMD_PORT
 */
#define  VTSS_F_ANA_AC_PS_COMMON_HM_CFG_HMD_PORT(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_ANA_AC_PS_COMMON_HM_CFG_HMD_PORT     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_ANA_AC_PS_COMMON_HM_CFG_HMD_PORT(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Offset the index into the PGID table.
 *
 * \details 
 * 0: No offset (PGID address used = HIH.LPID)
 * 1: Offset PGID address for this device with one times offset (PGID
 * address used = HIH.LPID + HMD_ADDR_OFFSET (see:
 * ANA_AC::PS_COMMON_CFG.HMD_ADDR_OFFSET)
 * 2: Offset PGID address for this device with two times offset (PGID
 * address used = HIH.LPID + 2*HMD_ADDR_OFFSET (see:
 * ANA_AC::PS_COMMON_CFG.HMD_ADDR_OFFSET)
 * 3: Offset PGID address for this device with three times offset (PGID
 * address used = HIH.LPID + 3*HMD_ADDR_OFFSET (see:
 * ANA_AC::PS_COMMON_CFG.HMD_ADDR_OFFSET)
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_HM_CFG . HMD_ADDR_IDX
 */
#define  VTSS_F_ANA_AC_PS_COMMON_HM_CFG_HMD_ADDR_IDX(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_ANA_AC_PS_COMMON_HM_CFG_HMD_ADDR_IDX     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_ANA_AC_PS_COMMON_HM_CFG_HMD_ADDR_IDX(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Configures forwarding of frames not classified to any service (ISDX = 0)
 * received from a HIH enabled port with HIH.LPID less than 48.
 * 
 * This configuration has only effect if PS_COMMON_CFG.HIH_FWD_ENA is
 * enabled and PS_COMMON_CFG.HM_FWD_ENA is disabled 
 *
 * \details 
 * '0': Disabled (frames are bridged if ISDX=0 and HIH.LPID less than 48)
 * '1': Enabled (frames are forwarded based on HIH.LPID if ISDX=0)
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_HM_CFG . HIH_LPID_0_47_FWD_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_HM_CFG_HIH_LPID_0_47_FWD_ENA  VTSS_BIT(2)

/** 
 * \brief
 * set bit for HOST device.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_HM_CFG . HMD_HOST_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_HM_CFG_HMD_HOST_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Valid bit for HMD_PORT.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_HM_CFG . HMD_PORT_VLD
 */
#define  VTSS_F_ANA_AC_PS_COMMON_HM_CFG_HMD_PORT_VLD  VTSS_BIT(0)


/** 
 * \brief Common general configuration
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:PS_COMMON_CFG
 */
#define VTSS_ANA_AC_PS_COMMON_PS_COMMON_CFG  VTSS_IOREG(VTSS_TO_ANA_AC,0x1ba68)

/** 
 * \brief
 * Host Mode Device (HMD) PGID offset per HMD index (configured via
 * ANA_AC::HM_CFG.HMD_ADDR_IDX).
 *
 * \details 
 * 0: 0
 * 1: 64* HMD index (HM_CFG.HMD_ADDR_IDX)
 * 2: 128* HMD index (HM_CFG.HMD_ADDR_IDX)
 * 3: 256* HMD index (HM_CFG.HMD_ADDR_IDX)
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_PS_COMMON_CFG . HMD_ADDR_OFFSET
 */
#define  VTSS_F_ANA_AC_PS_COMMON_PS_COMMON_CFG_HMD_ADDR_OFFSET(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_ANA_AC_PS_COMMON_PS_COMMON_CFG_HMD_ADDR_OFFSET     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_ANA_AC_PS_COMMON_PS_COMMON_CFG_HMD_ADDR_OFFSET(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * Enable forwarding host ports based on LPID received in HIH for traffic
 * not classified to any service (ISDX = 0).
 * 
 * For HM ports (ref. ANA_AC::HM_CFG.HMD_PORT), setting this bit will cause
 * frames that are not classified to any service (ISDX=0) to be forwarded
 * based on HIH.LPID. The LPID is used for lookup in the PGID table (ref.
 * ANA_AC::PGID::PGID_CFG_0/PGID_CFG_1).
 * Among others this can be used to forward all frames to specific line
 * port(s).
 * 
 * Look at register group PGID for further usage.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_PS_COMMON_CFG . HIH_FWD_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_PS_COMMON_CFG_HIH_FWD_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Enable forwarding from line ports based on ingress port number and from
 * host ports based on LPID received in HIH for traffic not classified to
 * any service (ISDX = 0).
 * 
 * For line ports (i.e. any front port not configured for HM in
 * ANA_CL::HM_CFG.HMD_PORT), setting this bit will cause frames that are
 * not classfied to any service (ISDX=0) to be forwarded based on ingress
 * port and QoS. Ingress port and QoS will be used for lookup in the PGID
 * table (ref. ANA_AC::PGID::PGID_CFG_0/PGID_CFG_1). 
 * Among others this can be used to forward all frames on an ingress port
 * to a specific virtual port on a HM port.
 * 
 * For HM ports (ref. ANA_AC::HM_CFG.HMD_PORT), setting this bit will cause
 * frames that are not classified to any service (ISDX=0) to be forwarded
 * based on HIH.LPID. The LPID is used for lookup in the PGID table (ref.
 * ANA_AC::PGID::PGID_CFG_0/PGID_CFG_1).
 * Among others this can be used to forward all frames to specific line
 * port(s).
 * 
 * Look at register group PGID for further usage.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_PS_COMMON_CFG . HM_FWD_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_PS_COMMON_CFG_HM_FWD_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable SFLOW sampler ID in SFLOW stamp.
 *
 * \details 
 * 0: Disable
 * 1: Enable sampler ID placed as tstamp(31:26).
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_PS_COMMON_CFG . SFLOW_SMPL_ID_IN_STAMP_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_PS_COMMON_CFG_SFLOW_SMPL_ID_IN_STAMP_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable SFLOW handling.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_PS_COMMON_CFG . SFLOW_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_PS_COMMON_CFG_SFLOW_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable lookup of logical source port. If disabled, the source mask does
 * not affect the egress port mask.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_PS_COMMON_CFG . SRC_LOOKUP_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_PS_COMMON_CFG_SRC_LOOKUP_ENA  VTSS_BIT(0)


/** 
 * \brief Table initializations
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:PS_TABLE_CLR_CFG
 */
#define VTSS_ANA_AC_PS_COMMON_PS_TABLE_CLR_CFG  VTSS_IOREG(VTSS_TO_ANA_AC,0x1ba69)

/** 
 * \brief
 * Initialize the source table. After Init L2 forwarding back to src port
 * will be filtered. The bit is cleared by HW when Init is complete
 *
 * \details 
 * 0: Idle (no clear)
 * 1: Initialize table
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_PS_TABLE_CLR_CFG . SRC_TABLE_INIT_SHOT
 */
#define  VTSS_F_ANA_AC_PS_COMMON_PS_TABLE_CLR_CFG_SRC_TABLE_INIT_SHOT  VTSS_BIT(1)

/** 
 * \brief
 * Initialize the pgid table. The bit is cleared by HW when Init is
 * complete
 *
 * \details 
 * 0: Idle (no clear)
 * 1: Initialize table
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_PS_TABLE_CLR_CFG . PGID_TABLE_INIT_SHOT
 */
#define  VTSS_F_ANA_AC_PS_COMMON_PS_TABLE_CLR_CFG_PGID_TABLE_INIT_SHOT  VTSS_BIT(0)


/** 
 * \brief sFlow configuration
 *
 * \details
 * This register configures the sFlow sampler.
 *
 * Register: \a ANA_AC:PS_COMMON:SFLOW_CFG
 */
#define VTSS_ANA_AC_PS_COMMON_SFLOW_CFG      VTSS_IOREG(VTSS_TO_ANA_AC,0x1ba6a)

/** 
 * \brief
 * CPU queue for frames mirrored to the CPU.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_SFLOW_CFG . SFLOW_CPU_QU
 */
#define  VTSS_F_ANA_AC_PS_COMMON_SFLOW_CFG_SFLOW_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_ANA_AC_PS_COMMON_SFLOW_CFG_SFLOW_CPU_QU     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_ANA_AC_PS_COMMON_SFLOW_CFG_SFLOW_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,12,3)


/** 
 * \brief sFLOW counter reset 
 *
 * \details
 * This register configures the sFlow sampler reset.
 *
 * Register: \a ANA_AC:PS_COMMON:SFLOW_RESET_CTRL
 */
#define VTSS_ANA_AC_PS_COMMON_SFLOW_RESET_CTRL  VTSS_IOREG(VTSS_TO_ANA_AC,0x1ba6b)


/** 
 * \brief sFLOW sample control 
 *
 * \details
 * This register configures the sFlow sampler.
 *
 * Register: \a ANA_AC:PS_COMMON:SFLOW_CTRL
 *
 * @param ri Replicator: x_ANA_NUM_ETH_PORTS (??), 0-31
 */
#define VTSS_ANA_AC_PS_COMMON_SFLOW_CTRL(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1ba6c + (ri))

/** 
 * \brief
 * Configures the sampler as an ingress, egress or either direction
 * sampler.
 *
 * \details 
 * "00": Disable
 * "01": RX sampler (samples trafficfrom port)
 * "10": TX sampler (samples traffic to port)
 * "11": RX or TX sampler (samples traffic to and from port)
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_SFLOW_CTRL . SFLOW_DIR_SEL
 */
#define  VTSS_F_ANA_AC_PS_COMMON_SFLOW_CTRL_SFLOW_DIR_SEL(x)  VTSS_ENCODE_BITFIELD(x,15,2)
#define  VTSS_M_ANA_AC_PS_COMMON_SFLOW_CTRL_SFLOW_DIR_SEL     VTSS_ENCODE_BITMASK(15,2)
#define  VTSS_X_ANA_AC_PS_COMMON_SFLOW_CTRL_SFLOW_DIR_SEL(x)  VTSS_EXTRACT_BITFIELD(x,15,2)

/** 
 * \brief
 * The sample number (rate) that is compared with the randomly generated
 * number for each frame.
 *
 * \details 
 * '0x0': 0 value (disable)
 * '0x1': 1 value
 * '0x7FFF': 32767 (sample all).
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_SFLOW_CTRL . SFLOW_SAMPLE_RATE
 */
#define  VTSS_F_ANA_AC_PS_COMMON_SFLOW_CTRL_SFLOW_SAMPLE_RATE(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_ANA_AC_PS_COMMON_SFLOW_CTRL_SFLOW_SAMPLE_RATE     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_ANA_AC_PS_COMMON_SFLOW_CTRL_SFLOW_SAMPLE_RATE(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief sFLOW status counts
 *
 * \details
 * This register configures the sFlow sampler.
 *
 * Register: \a ANA_AC:PS_COMMON:SFLOW_CNT
 *
 * @param ri Replicator: x_ANA_NUM_ETH_PORTS (??), 0-31
 */
#define VTSS_ANA_AC_PS_COMMON_SFLOW_CNT(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1ba8c + (ri))

/** 
 * \brief
 * Count the number of sampled frames. The counter is free running.
 *
 * \details 
 * 0 : no entries
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_SFLOW_CNT . SFLOW_SAMPLE_CNT
 */
#define  VTSS_F_ANA_AC_PS_COMMON_SFLOW_CNT_SFLOW_SAMPLE_CNT(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_ANA_AC_PS_COMMON_SFLOW_CNT_SFLOW_SAMPLE_CNT     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_ANA_AC_PS_COMMON_SFLOW_CNT_SFLOW_SAMPLE_CNT(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Count the number of forwarded frames. The counter is free running.
 *
 * \details 
 * 0 : no entries
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_SFLOW_CNT . SFLOW_FRAME_FWD_CNT
 */
#define  VTSS_F_ANA_AC_PS_COMMON_SFLOW_CNT_SFLOW_FRAME_FWD_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_ANA_AC_PS_COMMON_SFLOW_CNT_SFLOW_FRAME_FWD_CNT     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_ANA_AC_PS_COMMON_SFLOW_CNT_SFLOW_FRAME_FWD_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Physical source port aggregation configuration
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:PHYS_SRC_AGGR_CFG
 */
#define VTSS_ANA_AC_PS_COMMON_PHYS_SRC_AGGR_CFG  VTSS_IOREG(VTSS_TO_ANA_AC,0x1baac)


/** 
 * \brief Stack portmask configuration
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:STACK_CFG
 */
#define VTSS_ANA_AC_PS_COMMON_STACK_CFG      VTSS_IOREG(VTSS_TO_ANA_AC,0x1baad)


/** 
 * \brief Stack Link A mask configuration
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:STACK_A_CFG
 */
#define VTSS_ANA_AC_PS_COMMON_STACK_A_CFG    VTSS_IOREG(VTSS_TO_ANA_AC,0x1baae)


/** 
 * \brief Common stack configuration
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:COMMON_VSTAX_CFG
 */
#define VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG  VTSS_IOREG(VTSS_TO_ANA_AC,0x1baaf)

/** 
 * \brief
 * Configures LRN skip for multicast routed frames
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_RT_MC_SRC_LRN_SKIP_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_RT_MC_SRC_LRN_SKIP_ENA  VTSS_BIT(20)

/** 
 * \brief
 * Configures LRN skip	for unicast routed frames
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_RT_UC_SRC_LRN_SKIP_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_RT_UC_SRC_LRN_SKIP_ENA  VTSS_BIT(19)

/** 
 * \brief
 * Configures handling of multicast routing on stack link
 *
 * \details 
 * 0: Disable multicast on both links
 * 1: Enable multicast routing on stack link A
 * 2: Enable multicast routing on stack link B
 * 3: Enable multicast routing on both links
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_MC_ROUTE_TO_STACK_LINK_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_MC_ROUTE_TO_STACK_LINK_ENA(x)  VTSS_ENCODE_BITFIELD(x,17,2)
#define  VTSS_M_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_MC_ROUTE_TO_STACK_LINK_ENA     VTSS_ENCODE_BITMASK(17,2)
#define  VTSS_X_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_MC_ROUTE_TO_STACK_LINK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,17,2)

/** 
 * \brief
 * Forward unicast frames from frontports as fwd_mode==fwd_llookup (instead
 * of fwd_logical)
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_LOGICAL_LLOOKUP_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_LOGICAL_LLOOKUP_ENA  VTSS_BIT(16)

/** 
 * \brief
 * This can be used to enable GLAG in a stack
 * IF enabled the upper GLAG*8 number of PGID multicast entries will be
 * used for GLAG
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_GLAG_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_GLAG_ENA  VTSS_BIT(15)

/** 
 * \brief
 * This feature is to be used in 48 port switches consisting of two units
 * (i.e. chips) and where the CPU is disabled 
 * one of the units. Only the unit with the disabled CPU shall have
 * vstax2_ttl_keep==1
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_TTL_KEEP_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_TTL_KEEP_ENA  VTSS_BIT(14)

/** 
 * \brief
 * Forward unicast frames from frontports taking congestion management into
 * account
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_USE_CM_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_USE_CM_ENA  VTSS_BIT(13)

/** 
 * \brief
 * Forward multicast frames as fwd_mode==fwd_llookup (instead of fwd_mc)
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_MC_LLOOKUP_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_MC_LLOOKUP_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Forward non flooded multicast frames w. fwd_mode==fwd_llookup (instead
 * of fwd_mc)
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_MC_LLOOKUP_NON_FLOOD_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_MC_LLOOKUP_NON_FLOOD_ENA  VTSS_BIT(11)

/** 
 * \brief
 * CPU queue for VSTAX2 frames forwarding err.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_FWD_ERR_QU
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_FWD_ERR_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_FWD_ERR_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_FWD_ERR_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Enable / disable putting ISDX into VSTAX MISC field.
 *
 * \details 
 * 0: Disable (VSTAX MISC contains Aggregation code)
 * 1: Enable (VSTAX MISC contains ISDX)
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . VSTAX2_MISC_ISDX_ENA
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_VSTAX2_MISC_ISDX_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Specifies own upsid.
 * 
 * This must be configured consistently across the following registers:
 * ANA_AC::COMMON_VSTAX_CFG.OWN_UPSID
 * ANA_L2::VSTAX_CTRL.OWN_UPSID
 * ASM::SP_UPSID_CFG.OWN_UPSID
 *
 * \details 
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG . OWN_UPSID
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_OWN_UPSID(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_OWN_UPSID     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_ANA_AC_PS_COMMON_COMMON_VSTAX_CFG_OWN_UPSID(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Common stack configuration
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:COMMON_EQUAL_STACK_LINK_TTL_CFG
 */
#define VTSS_ANA_AC_PS_COMMON_COMMON_EQUAL_STACK_LINK_TTL_CFG  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bab0)

/** 
 * \brief
 * TTL value used for equal cost path (enabled via
 * ANA_AC::STACK_LINK_EQUAL_COST_CFG.STACK_LINK_EQUAL_ENA)
 *
 * \details 
 * Field: ::VTSS_ANA_AC_PS_COMMON_COMMON_EQUAL_STACK_LINK_TTL_CFG . VSTAX2_EQUAL_STACK_LINK_TTL_VAL
 */
#define  VTSS_F_ANA_AC_PS_COMMON_COMMON_EQUAL_STACK_LINK_TTL_CFG_VSTAX2_EQUAL_STACK_LINK_TTL_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_AC_PS_COMMON_COMMON_EQUAL_STACK_LINK_TTL_CFG_VSTAX2_EQUAL_STACK_LINK_TTL_VAL     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_AC_PS_COMMON_COMMON_EQUAL_STACK_LINK_TTL_CFG_VSTAX2_EQUAL_STACK_LINK_TTL_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Per port stack mode configuration
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:VSTAX_CTRL
 *
 * @param ri Replicator: x_ANA_NUM_ETH_PORTS (??), 0-31
 */
#define VTSS_ANA_AC_PS_COMMON_VSTAX_CTRL(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bab1 + (ri))

/** 
 * \brief
 * Specify learn and forwarding operation for the port
 *
 * \details 
 * 0: VStaX2/BF mode 
 * 1: VStaX2/AF mode
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_VSTAX_CTRL . VSTAX2_STACK_PORT_MODE
 */
#define  VTSS_F_ANA_AC_PS_COMMON_VSTAX_CTRL_VSTAX2_STACK_PORT_MODE  VTSS_BIT(0)


/** 
 * \brief Gmirror destination configuration
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:VSTAX_GMIRROR_CFG
 */
#define VTSS_ANA_AC_PS_COMMON_VSTAX_GMIRROR_CFG  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bad1)


/** 
 * \brief CPU handling configuration
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:CPU_CFG
 */
#define VTSS_ANA_AC_PS_COMMON_CPU_CFG        VTSS_IOREG(VTSS_TO_ANA_AC,0x1bada)

/** 
 * \brief
 * Specifies a group of CPU queues which will only receive one frame copy
 * in total
 *
 * \details 
 * '00000000' : A frame copy will be generated for all CPU queues
 * applicable for reception
 * ...
 * 'xxxxxx11' : Only one cpu copy will be generated for CPU queue 1 and 0
 * ...
 * '11111111' : At most one frame copy will be generated for all CPU queues
 * applicable for reception
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_CPU_CFG . ONE_CPU_COPY_ONLY_MASK
 */
#define  VTSS_F_ANA_AC_PS_COMMON_CPU_CFG_ONE_CPU_COPY_ONLY_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_AC_PS_COMMON_CPU_CFG_ONE_CPU_COPY_ONLY_MASK     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_AC_PS_COMMON_CPU_CFG_ONE_CPU_COPY_ONLY_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Memory integrity status registers
 *
 * \details
 * Register: \a ANA_AC:PS_COMMON:RAM_INTGR_ERR_IDENT
 */
#define VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bade)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . PGID_RAM_INTGR_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_PGID_RAM_INTGR_ERR  VTSS_BIT(13)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . SRC_RAM_INTGR_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_SRC_RAM_INTGR_ERR  VTSS_BIT(12)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . STAT_ISDX_IR_RAM_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_STAT_ISDX_IR_RAM_ERR  VTSS_BIT(9)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . STAT_ISDX_CR_RAM_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_STAT_ISDX_CR_RAM_ERR  VTSS_BIT(8)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . STAT_TDLB_IR_RAM_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_STAT_TDLB_IR_RAM_ERR  VTSS_BIT(7)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . STAT_TDLB_CR_RAM_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_STAT_TDLB_CR_RAM_ERR  VTSS_BIT(6)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . STAT_IRLEG_IR_RAM_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_STAT_IRLEG_IR_RAM_ERR  VTSS_BIT(5)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . STAT_IRLEG_CR_RAM_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_STAT_IRLEG_CR_RAM_ERR  VTSS_BIT(4)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . STAT_ERLEG_IR_RAM_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_STAT_ERLEG_IR_RAM_ERR  VTSS_BIT(3)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . STAT_ERLEG_CR_RAM_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_STAT_ERLEG_CR_RAM_ERR  VTSS_BIT(2)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . STAT_PORT_IR_RAM_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_STAT_PORT_IR_RAM_ERR  VTSS_BIT(1)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT . STAT_PORT_CR_RAM_ERR
 */
#define  VTSS_F_ANA_AC_PS_COMMON_RAM_INTGR_ERR_IDENT_STAT_PORT_CR_RAM_ERR  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:MIRROR_PROBE
 *
 * Per Mirror probe configurations
 */


/** 
 * \brief Mirror Probe configuration
 *
 * \details
 * Register: \a ANA_AC:MIRROR_PROBE:PROBE_CFG
 *
 * @param gi Replicator: x_ANA_AC_NUM_PROBE (??), 0-2
 */
#define VTSS_ANA_AC_MIRROR_PROBE_PROBE_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1badf,gi,8,0,0)

/** 
 * \brief
 * The set of destination cpu ports where the traffic is mirrored.
 *
 * \details 
 * 0: CPU port is not mirrored
 * 1: CPU port is mirrored
 *
 * Field: ::VTSS_ANA_AC_MIRROR_PROBE_PROBE_CFG . PROBE_CPU_SET
 */
#define  VTSS_F_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_CPU_SET(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_CPU_SET     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_CPU_SET(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * The Probe VLAN ID used when VLAN mode is "10"
 *
 * \details 
 * Field: ::VTSS_ANA_AC_MIRROR_PROBE_PROBE_CFG . PROBE_VID
 */
#define  VTSS_F_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_VID(x)  VTSS_ENCODE_BITFIELD(x,12,12)
#define  VTSS_M_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_VID     VTSS_ENCODE_BITMASK(12,12)
#define  VTSS_X_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_VID(x)  VTSS_EXTRACT_BITFIELD(x,12,12)

/** 
 * \brief
 * Mode of VLAN filtering
 *
 * \details 
 * "00" : No VLAN filtering
 * "01" : Mirror only traffic in VLANs with the VLAN table mirror bit set
 * "10" : Mirror only traffic in the Probe VID is mirrored.
 * "11" : Mirror only traffic in the Probe BVID is mirrored.
 *
 * Field: ::VTSS_ANA_AC_MIRROR_PROBE_PROBE_CFG . PROBE_VLAN_MODE
 */
#define  VTSS_F_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_VLAN_MODE(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_VLAN_MODE     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_VLAN_MODE(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Mode of MAC address filtering based on the mirror bit in the MAC table
 *
 * \details 
 * "00" : No MAC filtering
 * "01" : Mirror only traffic with mirror bit set for DMAC
 * "10" : Mirror only traffic with mirror bit set for SMAC
 * "11" : Mirror only traffic with mirror bit set for DMAC and/or SMAC
 *
 * Field: ::VTSS_ANA_AC_MIRROR_PROBE_PROBE_CFG . PROBE_MAC_MODE
 */
#define  VTSS_F_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_MAC_MODE(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_MAC_MODE     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_MAC_MODE(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Direction of traffic that is mirrored
 *
 * \details 
 * "00" : Nothing is mirrored
 * "01" : TX - Only traffic to destination ports in the probe port set
 * "10" : RX - Only traffic from source ports in the probe port set
 * "11" : RX+TX - Traffic to and from ports in the probe port set (always
 * mirrored as RX)
 *
 * Field: ::VTSS_ANA_AC_MIRROR_PROBE_PROBE_CFG . PROBE_DIRECTION
 */
#define  VTSS_F_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_DIRECTION(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_DIRECTION     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_DIRECTION(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Mirror Probe Port configuration
 *
 * \details
 * Register: \a ANA_AC:MIRROR_PROBE:PROBE_PORT_CFG
 *
 * @param gi Replicator: x_ANA_AC_NUM_PROBE (??), 0-2
 */
#define VTSS_ANA_AC_MIRROR_PROBE_PROBE_PORT_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1badf,gi,8,0,1)

/**
 * Register Group: \a ANA_AC:AGGR
 *
 * Aggregation table configurations
 */


/** 
 * \brief Aggregation port mask configuration
 *
 * \details
 * Ports that are local link aggregated must be mutually disabled in these
 * masks.
 * 
 * I.e. these masks must have 1 bit set for each logical port.
 * 
 * In a multi-chip configuration, the bits for the stack links must always
 * be set, except for the case of a LLAG stack link (i.e. interconnect
 * between two devices forming a 48 port system) where each mask must only
 * have one of the two physical stack ports enabled.
 *
 * Register: \a ANA_AC:AGGR:AGGR_CFG
 *
 * @param gi Replicator: x_ANA_NUM_AGGR (??), 0-15
 */
#define VTSS_ANA_AC_AGGR_AGGR_CFG(gi)        VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1baf7,gi,1,0,0)

/**
 * Register Group: \a ANA_AC:SRC
 *
 * Source table configurations
 */


/** 
 * \brief Local source port mask configuration
 *
 * \details
 * Controls source port filtering. 
 * Ports that are local link aggregated must be all be disabled in each of
 * the aggregated source port masks 
 *
 * Register: \a ANA_AC:SRC:SRC_CFG
 *
 * @param gi Replicator: x_ANA_NUM_INB_PORTS (??), 0-34
 */
#define VTSS_ANA_AC_SRC_SRC_CFG(gi)          VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1ba40,gi,1,0,0)

/**
 * Register Group: \a ANA_AC:UPSID
 *
 * UPSID table configurations
 */


/** 
 * \brief Equal cost UPSID destination configuration
 *
 * \details
 *  Used for configuring equal distance to UPSID
 *
 * Register: \a ANA_AC:UPSID:STACK_LINK_EQUAL_COST_CFG
 *
 * @param gi Register: UPSID (??), 0-31
 */
#define VTSS_ANA_AC_UPSID_STACK_LINK_EQUAL_COST_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1bb07,gi,2,0,0)

/** 
 * \brief
 * Enable equal cost forwarding to UPSID. I.e. both stack ports (A and B)
 * are used for forwarding to UPSID.
 * 
 * If STACK_LINK_EQUAL_ENA is set, then the stack forwarding mask is
 * calculated as one of the following two, depending on AC:
 *   Even AC:
 *     UPSID.UPSID_PORT_MASK & STACK_A_CFG.STACK_A_MASK
 *   Odd AC:
 *     UPSID.UPSID_PORT_MASK & ~STACK_A_CFG.STACK_A_MASK
 * 
 * Furthermore when STACK_LINK_EQUAL_ENA is set, the TTL value is set to
 *   VSTAX2_EQUAL_STACK_LINK_TTL_VAL
 * (instead of REW::VSTAX_PORT_GRP_CFG.VSTAX_TTL).
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_AC_UPSID_STACK_LINK_EQUAL_COST_CFG . STACK_LINK_EQUAL_ENA
 */
#define  VTSS_F_ANA_AC_UPSID_STACK_LINK_EQUAL_COST_CFG_STACK_LINK_EQUAL_ENA  VTSS_BIT(0)


/** 
 * \brief UPSID destination configuration
 *
 * \details
 * Configures which physical ports to be used for reaching a given UPSID.
 *
 * Register: \a ANA_AC:UPSID:UPSID_CFG
 *
 * @param gi Register: UPSID (??), 0-31
 */
#define VTSS_ANA_AC_UPSID_UPSID_CFG(gi)      VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1bb07,gi,2,0,1)

/**
 * Register Group: \a ANA_AC:GLAG
 *
 * Global Link Aggregation table configurations
 */


/** 
 * \brief GLAG source port mask configuration
 *
 * \details
 * Controls global source port filtering.
 * Ports that are global link aggregated must be all be disabled in the
 * corresponding glag source port masks 
 *
 * Register: \a ANA_AC:GLAG:GLAG_CFG
 *
 * @param gi Replicator: x_ANA_NUM_GLAG (??), 0-31
 */
#define VTSS_ANA_AC_GLAG_GLAG_CFG(gi)        VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1bb47,gi,2,0,0)


/** 
 * \brief GLAG member count configuration
 *
 * \details
 * Controls global port aggregation.
 * This must specify the number of GLAG members and are used for
 * calculating Final Port of Exit 
 *
 * Register: \a ANA_AC:GLAG:MBR_CNT_CFG
 *
 * @param gi Replicator: x_ANA_NUM_GLAG (??), 0-31
 */
#define VTSS_ANA_AC_GLAG_MBR_CNT_CFG(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1bb47,gi,2,0,1)

/** 
 * \brief
 * GLAG member count
 *
 * \details 
 * 0: one member
 * 1: two members
 * ...
 * 7: Eight members
 *
 * Field: ::VTSS_ANA_AC_GLAG_MBR_CNT_CFG . GLAG_MBR_CNT
 */
#define  VTSS_F_ANA_AC_GLAG_MBR_CNT_CFG_GLAG_MBR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_AC_GLAG_MBR_CNT_CFG_GLAG_MBR_CNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_AC_GLAG_MBR_CNT_CFG_GLAG_MBR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a ANA_AC:PGID
 *
 * Port group ID table configurations
 */


/** 
 * \brief PGID port mask / destination configuration
 *
 * \details
 * Register: \a ANA_AC:PGID:PGID_CFG_0
 *
 * @param gi Replicator: x_ANA_NUM_PGID_TBL_ENTRIES (??), 0-1311
 */
#define VTSS_ANA_AC_PGID_PGID_CFG_0(gi)      VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1b000,gi,2,0,0)


/** 
 * \brief PGID mask handling configuration
 *
 * \details
 * Register: \a ANA_AC:PGID:PGID_CFG_1
 *
 * @param gi Replicator: x_ANA_NUM_PGID_TBL_ENTRIES (??), 0-1311
 */
#define VTSS_ANA_AC_PGID_PGID_CFG_1(gi)      VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1b000,gi,2,0,1)

/** 
 * \brief
 * CPU queue for frames forwarded to CPU via PGID lookup.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_AC_PGID_PGID_CFG_1 . PGID_CPU_QU
 */
#define  VTSS_F_ANA_AC_PGID_PGID_CFG_1_PGID_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_AC_PGID_PGID_CFG_1_PGID_CPU_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_AC_PGID_PGID_CFG_1_PGID_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Encoding of associated PORT_MASK bitgroup (See
 * ANA_AC::PGID_CFG_0.PORT_MASK).
 *
 * \details 
 * 0: Entry  contain port_mask
 * 1: Entry contain UPSID, UPSPN, FWD_MODE (and host port when operating as
 * a MAC)
 *
 * Field: ::VTSS_ANA_AC_PGID_PGID_CFG_1 . STACK_TYPE_ENA
 */
#define  VTSS_F_ANA_AC_PGID_PGID_CFG_1_STACK_TYPE_ENA  VTSS_BIT(1)

/** 
 * \brief
 * CPU copy enable flag for PGID entry.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_AC_PGID_PGID_CFG_1 . PGID_CPU_COPY_ENA
 */
#define  VTSS_F_ANA_AC_PGID_PGID_CFG_1_PGID_CPU_COPY_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:PS_STICKY
 *
 * Sticky diagnostic status
 */


/** 
 * \brief Sticky diagnostic status
 *
 * \details
 * Register: \a ANA_AC:PS_STICKY:STICKY
 */
#define VTSS_ANA_AC_PS_STICKY_STICKY         VTSS_IOREG(VTSS_TO_ANA_AC,0x1bb87)

/** 
 * \brief
 * Set if frame is forwarded as line port (port, QOS). Write 1 to clear
 * this field.
 *
 * \details 
 * 0: No redir event
 * 1: A frame was line port forwarded (host mode).
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . HMD_FRONTPORT_FWD_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_HMD_FRONTPORT_FWD_STICKY  VTSS_BIT(26)

/** 
 * \brief
 * Set if frame is forwarded based on HIH.LPID. Write 1 to clear this
 * field.
 *
 * \details 
 * 0: No HMD forward has occured
 * 1: HMD forward based on HIH.LPID.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . HMD_LPID_FWD_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_HMD_LPID_FWD_STICKY  VTSS_BIT(25)

/** 
 * \brief
 * Set if frame is redirected due to VCAP_s2. Write 1 to clear this field.
 *
 * \details 
 * 0: No redir event
 * 1: A frame was redirected.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . ACL_REDIR_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_ACL_REDIR_STICKY  VTSS_BIT(24)

/** 
 * \brief
 * Set if mirrored probe has been triggered. Write 1 to clear this field.
 *
 * \details 
 * 0: Mirror probe not triggered
 * 1: Mirror probe triggered
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . PROBE_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_PROBE_STICKY(x)  VTSS_ENCODE_BITFIELD(x,20,3)
#define  VTSS_M_ANA_AC_PS_STICKY_STICKY_PROBE_STICKY     VTSS_ENCODE_BITMASK(20,3)
#define  VTSS_X_ANA_AC_PS_STICKY_STICKY_PROBE_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,20,3)

/** 
 * \brief
 * Set if it is not possible to exend a frame with OAM rx stamp. Write 1 to
 * clear this field.
 *
 * \details 
 * 0: Not used
 * 1: It is not possible to stamp frame due to frame size.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . RX_OAM_STAMP_ERR_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_RX_OAM_STAMP_ERR_STICKY  VTSS_BIT(19)

/** 
 * \brief
 * Set if a frame was OAM rx stamped. Write 1 to clear this field.
 *
 * \details 
 * 0: Not used
 * 1: Frame was stamped.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . RX_OAM_STAMP_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_RX_OAM_STAMP_STICKY  VTSS_BIT(18)

/** 
 * \brief
 * Set if the egress port mask for a frame was zero and the frame was not
 * sent to the CPU. Write 1 to clear this field.
 *
 * \details 
 * 0: Forwarded to egress port or CPU
 * 1: Not forwarded to any egress ports or CPU.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . ZERO_DST_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_ZERO_DST_STICKY  VTSS_BIT(17)

/** 
 * \brief
 * Set if a frame as been forwarded with a non zero destination set. Write
 * 1 to clear this field.
 *
 * \details 
 * 0: No frames forwarded
 * 1: Frame forward event
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . FRAME_FWD_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_FRAME_FWD_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * Set if a sflow candidate did exist. Write 1 to clear this field.
 *
 * \details 
 * 0: No event
 * 1: A SFLOW candidate was seen
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . SFLOW_CAND_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_SFLOW_CAND_STICKY  VTSS_BIT(15)

/** 
 * \brief
 * Set if a sflow destination sample event has taken place. Write 1 to
 * clear this field.
 *
 * \details 
 * 0: No event
 * 1: A sample event has occured
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . SFLOW_DST_SAMPLE_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_SFLOW_DST_SAMPLE_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * Set if a sflow source sample event has taken place. Write 1 to clear
 * this field.
 *
 * \details 
 * 0: No event
 * 1: A sample event has occured
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . SFLOW_SRC_SAMPLE_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_SFLOW_SRC_SAMPLE_STICKY  VTSS_BIT(13)

/** 
 * \brief
 * Set if a sflow sample event has taken place. Write 1 to clear this
 * field.
 *
 * \details 
 * 0: No event
 * 1: A sample event has occured
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . SFLOW_SAMPLE_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_SFLOW_SAMPLE_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * Set if VLAN mask has been used. Write 1 to clear this field.
 *
 * \details 
 * 0: Not used
 * 1: Used.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . VLAN_CONTRIB_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_VLAN_CONTRIB_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * Set if the GLAG source mask has been used. Write 1 to clear this field.
 *
 * \details 
 * 0: Not used
 * 1: Used.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . GLAG_CONTRIB_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_GLAG_CONTRIB_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Set if the source mask has been used. Write 1 to clear this field.
 *
 * \details 
 * 0: Not used
 * 1: Used.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . SRC_CONTRIB_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_SRC_CONTRIB_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Set if a CPU copy operation from the PGID lookup has been done. Write 1
 * to clear this field.
 *
 * \details 
 * 0: Disabled
 * 1: Enabled.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . PGID_CPU_MASK_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_PGID_CPU_MASK_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Set if the previous blocks in the Analyzer have signaled that a frame
 * must not be forwarded. This may be done for CPU move or discard
 * operations. Write 1 to clear this field.
 *
 * \details 
 * 0: Forward
 * 1: No forward.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . NO_L2_L3_FWD_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_NO_L2_L3_FWD_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set if ip6_mc_ctrl_flood_mask has been used.
 *
 * \details 
 * 0: Not used
 * 1: Used.
 * Write 1 to clear this field.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . IP6_MC_CTRL_FLOOD_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_IP6_MC_CTRL_FLOOD_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set if ip6_mc_data_flood_mask has been used.
 *
 * \details 
 * 0: Not used
 * 1: Used.
 * Write 1 to clear this field.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . IP6_MC_DATA_FLOOD_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_IP6_MC_DATA_FLOOD_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if ip4_mc_ctrl_flood_mask has been used.
 *
 * \details 
 * 0: Not used
 * 1: Used.
 * Write 1 to clear this field.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . IP4_MC_CTRL_FLOOD_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_IP4_MC_CTRL_FLOOD_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if ip4_mc_data_flood_mask has been used.
 *
 * \details 
 * 0: Not used
 * 1: Used.
 * Write 1 to clear this field.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . IP4_MC_DATA_FLOOD_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_IP4_MC_DATA_FLOOD_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if l2_mc_flood_mask has been used.
 *
 * \details 
 * 0: Not used
 * 1: Used.
 * Write 1 to clear this field.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . L2_MC_FLOOD_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_L2_MC_FLOOD_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if the unicast flood mask has been used. Write 1 to clear this
 * field.
 *
 * \details 
 * 0: Not used
 * 1: Used.
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_STICKY . UC_FLOOD_STICKY
 */
#define  VTSS_F_ANA_AC_PS_STICKY_STICKY_UC_FLOOD_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:PS_STICKY_MASK
 *
 * Sticky diagnostic global port counter event configuration
 */


/** 
 * \brief Sticky diagnostic counter masks
 *
 * \details
 * Register: \a ANA_AC:PS_STICKY_MASK:STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1bb88,gi,16,0,0)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . PROBE_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_PROBE_STICKY_MASK(x)  VTSS_ENCODE_BITFIELD(x,20,3)
#define  VTSS_M_ANA_AC_PS_STICKY_MASK_STICKY_MASK_PROBE_STICKY_MASK     VTSS_ENCODE_BITMASK(20,3)
#define  VTSS_X_ANA_AC_PS_STICKY_MASK_STICKY_MASK_PROBE_STICKY_MASK(x)  VTSS_EXTRACT_BITFIELD(x,20,3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . RX_OAM_STAMP_ERR_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_RX_OAM_STAMP_ERR_STICKY_MASK  VTSS_BIT(19)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . RX_OAM_STAMP_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_RX_OAM_STAMP_STICKY_MASK  VTSS_BIT(18)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . ZERO_DST_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_ZERO_DST_STICKY_MASK  VTSS_BIT(17)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . FRAME_FWD_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_FRAME_FWD_STICKY_MASK  VTSS_BIT(16)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . SFLOW_CAND_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_SFLOW_CAND_STICKY_MASK  VTSS_BIT(15)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . SFLOW_DST_SAMPLE_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_SFLOW_DST_SAMPLE_STICKY_MASK  VTSS_BIT(14)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . SFLOW_SRC_SAMPLE_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_SFLOW_SRC_SAMPLE_STICKY_MASK  VTSS_BIT(13)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . SFLOW_SAMPLE_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_SFLOW_SAMPLE_STICKY_MASK  VTSS_BIT(12)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . VLAN_CONTRIB_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_VLAN_CONTRIB_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . GLAG_CONTRIB_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_GLAG_CONTRIB_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . SRC_CONTRIB_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_SRC_CONTRIB_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . PGID_CPU_MASK_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_PGID_CPU_MASK_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . NO_L2_L3_FWD_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_NO_L2_L3_FWD_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . IP6_MC_CTRL_FLOOD_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_IP6_MC_CTRL_FLOOD_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . IP6_MC_DATA_FLOOD_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_IP6_MC_DATA_FLOOD_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . IP4_MC_CTRL_FLOOD_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_IP4_MC_CTRL_FLOOD_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . IP4_MC_DATA_FLOOD_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_IP4_MC_DATA_FLOOD_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . L2_MC_FLOOD_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_L2_MC_FLOOD_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_AC_PS_STICKY_MASK_STICKY_MASK . UC_FLOOD_STICKY_MASK
 */
#define  VTSS_F_ANA_AC_PS_STICKY_MASK_STICKY_MASK_UC_FLOOD_STICKY_MASK  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:POL_ALL_CFG
 *
 * Various policer controls
 */


/** 
 * \brief Configuration of ACL policer rates
 *
 * \details
 * These registers configure the rates of the ACL policers
 *
 * Register: \a ANA_AC:POL_ALL_CFG:POL_ACL_RATE_CFG
 *
 * @param ri Replicator: x_ANA_NUM_ACL_POLICER (??), 0-31
 */
#define VTSS_ANA_AC_POL_ALL_CFG_POL_ACL_RATE_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d630 + (ri))

/** 
 * \brief
 * Policer leaky bucket rate. Unit is 100160 bps.
 *
 * \details 
 * 0: Open until burst capacity is used, then closed.
 * 1: Rate = 100160 bps
 * n: Rate = n x 100160 bps
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ACL_RATE_CFG . ACL_RATE
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ACL_RATE_CFG_ACL_RATE(x)  VTSS_ENCODE_BITFIELD(x,0,17)
#define  VTSS_M_ANA_AC_POL_ALL_CFG_POL_ACL_RATE_CFG_ACL_RATE     VTSS_ENCODE_BITMASK(0,17)
#define  VTSS_X_ANA_AC_POL_ALL_CFG_POL_ACL_RATE_CFG_ACL_RATE(x)  VTSS_EXTRACT_BITFIELD(x,0,17)


/** 
 * \brief Configuration of ACL policer thresholds
 *
 * \details
 * These registers configure the thresholds of the ACL policers
 *
 * Register: \a ANA_AC:POL_ALL_CFG:POL_ACL_THRES_CFG
 *
 * @param ri Replicator: x_ANA_NUM_ACL_POLICER (??), 0-31
 */
#define VTSS_ANA_AC_POL_ALL_CFG_POL_ACL_THRES_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d650 + (ri))

/** 
 * \brief
 * Policer threshold size (a.ka. burst capacity). Unit is 4096 bytes.
 *
 * \details 
 * 0: Always closed
 * 1: Burst capacity = 4096 bytes
 * ...
 * n: Burst capacity = n x 4096 bytes
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ACL_THRES_CFG . ACL_THRES
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ACL_THRES_CFG_ACL_THRES(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_AC_POL_ALL_CFG_POL_ACL_THRES_CFG_ACL_THRES     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_AC_POL_ALL_CFG_POL_ACL_THRES_CFG_ACL_THRES(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Configuration of ACL policers
 *
 * \details
 * These registers configure the basic behavior of the ACL policers
 *
 * Register: \a ANA_AC:POL_ALL_CFG:POL_ACL_CTRL
 *
 * @param ri Replicator: x_ANA_NUM_ACL_POLICER (??), 0-31
 */
#define VTSS_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d670 + (ri))

/** 
 * \brief
 * Configuration of Drop Precedence bypass level. Frames with DP below
 * DP_BYPASS_LVL bypass the policer (frames are never policed and the
 * bucket is not updated with the frames).
 *
 * \details 
 * 0: No frames bypass the policer
 * 1: Frames with DP = 0 bypass the policer
 * 2: Frames with DP = 0 or 1 bypass the policer
 * 3: Frames with DP = 0, 1 or 2 bypass the policer
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL . DP_BYPASS_LVL
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL_DP_BYPASS_LVL(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL_DP_BYPASS_LVL     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL_DP_BYPASS_LVL(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

/** 
 * \brief
 * Value added to each frame before updating the bucket. Gap value range:
 * -64 to +63 in two's complement format.
 * When GAP_VALUE = 20, a line-rate measurement because each frame occupies
 * 12 bytes for the inter-frame gap and 8 bytes for the preamble on the
 * line. When GAP_VALUE = 0, a data-rate measurement is achived.
 *
 * \details 
 * 0x40: -64
 * 0x41: -63
 *    ...
 * 0x7F: -1
 * 0x00: 0
 * 0x01: 1
 *    ...
 * 0x3F: 63
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL . GAP_VALUE
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL_GAP_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,7)
#define  VTSS_M_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL_GAP_VALUE     VTSS_ENCODE_BITMASK(8,7)
#define  VTSS_X_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL_GAP_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,7)

/** 
 * \brief
 * Configures the traffic types to be policed by the policer. Each bit in
 * the mask enables a specific frame type.
 * If a frame type is not set in the mask, the frames of that type only,
 * bypass the policer (never discarded and the bucket is not updated with
 * the frame).

 *
 * \details 
 * 0: Reserved (Policer disabled).
 * 1 : Only frames to frontports are triggering policer and only frontport
 * destinations are policed.
 * 2 : Frames to cpu are triggering policer and only CPU destinations are
 * policed.
 * 3 : Frames to frontports and/or CPU are triggering policer and being
 * policed.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL . ACL_TRAFFIC_TYPE_MASK
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL_ACL_TRAFFIC_TYPE_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL_ACL_TRAFFIC_TYPE_MASK     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL_ACL_TRAFFIC_TYPE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * Enables frame rate mode for the ACL policer, where policer rates are
 * measured in frames per second instead of bits per second.
 *
 * \details 
 * 0: Rates measured in bits per second
 * 1: Rates measured in frames per second
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL . FRAME_RATE_ENA
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ACL_CTRL_FRAME_RATE_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration of port policer flow control
 *
 * \details
 * These registers configure flow control for the port policers
 *
 * Register: \a ANA_AC:POL_ALL_CFG:POL_PORT_FC_CFG
 *
 * @param ri Replicator: x_ANA_NUM_PORT_POLS (??), 0-32
 */
#define VTSS_ANA_AC_POL_ALL_CFG_POL_PORT_FC_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d690 + (ri))

/** 
 * \brief
 * Current flow control state for the port policer.
 *
 * \details 
 * 0: Flow control is inactive
 * 1: Flow control is active
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_PORT_FC_CFG . FC_STATE
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_PORT_FC_CFG_FC_STATE  VTSS_BIT(8)

/** 
 * \brief
 * Enables flow control mode for the port policer. If set, pause frames are
 * transmitted when the configured policer threshold is exceeded. If
 * cleared, frames exceeding the configured policer threshold are
 * discarded.
 * POL_ALL_CFG::PORT_FC_ENA must also be set to generate pause frames. In
 * order to generate pause frames, the DSM must be configured for flow
 * control.
 *
 * \details 
 * 0: Discard mode
 * 1: Flow control mode
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_PORT_FC_CFG . FC_ENA
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_PORT_FC_CFG_FC_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_ANA_AC_POL_ALL_CFG_POL_PORT_FC_CFG_FC_ENA     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_ANA_AC_POL_ALL_CFG_POL_PORT_FC_CFG_FC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Global configuration of policers
 *
 * \details
 * The register configures the basic behavior of all policers in the device
 *
 * Register: \a ANA_AC:POL_ALL_CFG:POL_ALL_CFG
 */
#define VTSS_ANA_AC_POL_ALL_CFG_POL_ALL_CFG  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d6b1)

/** 
 * \brief
 * Enables ajustment of frames sizes according to ingress pop count (VLAN
 * pop count and MAC in MAC pop count).
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ALL_CFG . TDLB_ADJ_INGR_POP_ENA
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ALL_CFG_TDLB_ADJ_INGR_POP_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Enables overall signaling of flow control from the port policers to DSM.
 * Each port policers can enable flow control in POL_PORT_FC_CFG.FC_ENA.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ALL_CFG . PORT_FC_ENA
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ALL_CFG_PORT_FC_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Enables operation of port policers in parallel.
 *
 * \details 
 * 0: Port policer operates in serial mode. In serial mode, each of the
 * active port policers within a port are checked one after another. If a
 * frame is discarded by a policer, the frame is not passed on to
 * subsequent policers.
 * 1: Port policer operates in parallel mode. In parallel mode, each of the
 * active port policers within a port operate on all frames (policing and
 * bucket updates) even though one or more policers might be closed.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ALL_CFG . PORT_POL_IN_PARALLEL_ENA
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ALL_CFG_PORT_POL_IN_PARALLEL_ENA  VTSS_BIT(10)

/** 
 * \brief
 * If set, all priority policers are initialized and buckets are open. The
 * bit must be cleared for normal operation to resume.
 *
 * \details 
 * 0: Normal operation
 * 1: Initialization
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ALL_CFG . PRIO_FORCE_INIT
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ALL_CFG_PRIO_FORCE_INIT  VTSS_BIT(7)

/** 
 * \brief
 * If set, all ACL policers are initialized and buckets are open. The bit
 * must be cleared for normal operation to resume.
 *
 * \details 
 * 0: Normal operation
 * 1: Initialization
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ALL_CFG . ACL_FORCE_INIT
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ALL_CFG_ACL_FORCE_INIT  VTSS_BIT(4)

/** 
 * \brief
 * If set, the port policers operate on logical port numbers in stead of
 * physical port numbers.
 *
 * \details 
 * 0: Policing per physical port
 * 1: Policing per logical port
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ALL_CFG . LPORT_POLICE_ENA
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ALL_CFG_LPORT_POLICE_ENA  VTSS_BIT(1)

/** 
 * \brief
 * If set, all port policers are initialized and buckets are open. The bit
 * must be cleared for normal operation to resume.
 *
 * \details 
 * 0: Normal operation
 * 1: Initialization
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_ALL_CFG . FORCE_INIT
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_ALL_CFG_FORCE_INIT  VTSS_BIT(0)


/** 
 * \brief Policer Diagnostics
 *
 * \details
 * Register: \a ANA_AC:POL_ALL_CFG:POL_STICKY
 */
#define VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY   VTSS_IOREG(VTSS_TO_ANA_AC,0x1d6b2)

/** 
 * \brief
 * Set if frame has been dropped by a DLB policer.
 *
 * \details 
 * 0: No event has occured
 * 1: DLB policer drop event has occured
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_DLB_DROP_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_DLB_DROP_STICKY  VTSS_BIT(28)

/** 
 * \brief
 * Set if a priority policer is active.
 *
 * \details 
 * 0: No event has occured
 * 1: Priority policer is active (frames added to leaky buckets)
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_PRIO_ACTIVE_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PRIO_ACTIVE_STICKY  VTSS_BIT(26)

/** 
 * \brief
 * Set if frame has been dropped due to priority policer.
 *
 * \details 
 * 0: No event has occured
 * 1: PRIO policer drop event has occured
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_PRIO_DROP_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PRIO_DROP_STICKY  VTSS_BIT(25)

/** 
 * \brief
 * Set if a priority policer has been bypassed due to a frame's DP level.
 *
 * \details 
 * 0: No event has occured
 * 1: Priority policer has been bypassed
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_PRIO_BYPASS_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PRIO_BYPASS_STICKY  VTSS_BIT(24)

/** 
 * \brief
 * Set if an ACL policer is active.
 *
 * \details 
 * 0: No event has occured
 * 1: ACL policer is active (frames added to leaky buckets)
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_ACL_ACTIVE_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_ACL_ACTIVE_STICKY  VTSS_BIT(22)

/** 
 * \brief
 * Set if frame has been dropped due to ACL policer.
 *
 * \details 
 * 0: No event has occured
 * 1: ACL policer drop event has occured
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_ACL_DROP_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_ACL_DROP_STICKY  VTSS_BIT(21)

/** 
 * \brief
 * Set if an ACL policer has been bypassed due to a frame's DP level.
 *
 * \details 
 * 0: No event has occured
 * 1: ACL policer has been bypassed
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_ACL_BYPASS_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_ACL_BYPASS_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * Set if a port policer is active.
 *
 * \details 
 * 0: No event has occured
 * 1: Port policer is active (frames added to leaky buckets)
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_PORT_ACTIVE_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PORT_ACTIVE_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PORT_ACTIVE_STICKY     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PORT_ACTIVE_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Set if a port policer's flow control state has been cleared.
 *
 * \details 
 * 0: No event has occured
 * 1: Port policer flow control state has been cleared
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_PORT_FC_CLEAR_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PORT_FC_CLEAR_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Set if a flow control has been active for a port policer.
 *
 * \details 
 * 0: No event has occured
 * 1: Port policer flow control has been activated
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_PORT_FC_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PORT_FC_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Set if a port policer has removed the front ports from a frame's
 * destination set due to policing. 
 *
 * \details 
 * 0: No event has occured
 * 1: A port policer has removed the front ports from a frame's destination
 * set due to policing
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_PORT_DROP_FWD_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PORT_DROP_FWD_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set if a port policer has removed the CPU ports from a frame's
 * destination set due to policer
 *
 * \details 
 * 0: No event has occured
 * 1: A port policer has removed the CPU ports from a frame's destination
 * set due to policing
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_PORT_DROP_CPU_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PORT_DROP_CPU_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set if a port policer has been bypassed due to a frame's DP level.
 *
 * \details 
 * 0: No event has occured
 * 1: Port policer has been bypassed
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_STICKY . POL_PORT_BYPASS_STICKY
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_STICKY_POL_PORT_BYPASS_STICKY  VTSS_BIT(4)


/** 
 * \brief RAM integrity errors
 *
 * \details
 * Register with 1 bit per RAM to store integrity errors
 *
 * Register: \a ANA_AC:POL_ALL_CFG:POL_RAM_INTGR_ERR_IDENT
 */
#define VTSS_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d6b3)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT . POL_CFG_RAM_INTGR_ERR
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT_POL_CFG_RAM_INTGR_ERR  VTSS_BIT(4)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT . POL_QU_RAM_INTGR_ERR
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT_POL_QU_RAM_INTGR_ERR  VTSS_BIT(3)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT . POL_PORT_RAM_INTGR_ERR
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT_POL_PORT_RAM_INTGR_ERR  VTSS_BIT(2)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT . POL_SDLB_RAM_INTGR_ERR
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT_POL_SDLB_RAM_INTGR_ERR  VTSS_BIT(1)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT . POL_TDLB_RAM_INTGR_ERR
 */
#define  VTSS_F_ANA_AC_POL_ALL_CFG_POL_RAM_INTGR_ERR_IDENT_POL_TDLB_RAM_INTGR_ERR  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:POL_PORT_CFG
 *
 * Port policer controls
 */


/** 
 * \brief Port policer upper threshold configuration
 *
 * \details
 * These registers configure the upper thresholds of the port policers. The
 * registers are index by 4 x port number + port policer index.
 *
 * Register: \a ANA_AC:POL_PORT_CFG:POL_PORT_THRES_CFG_0
 *
 * @param ri Replicator: x_ANA_NUM_PORTS_X_NUM_PORT_POL (??), 0-131
 */
#define VTSS_ANA_AC_POL_PORT_CFG_POL_PORT_THRES_CFG_0(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bc00 + (ri))

/** 
 * \brief
 * Threshold size for port policer (a.ka. burst capacity). Unit is 2048
 * bytes.
 *
 * \details 
 * 0: Always closed
 * 1: Burst capacity = 2048 bytes
 * ...
 * n: Burst capacity = n x 2048 bytes
 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CFG_POL_PORT_THRES_CFG_0 . PORT_THRES0
 */
#define  VTSS_F_ANA_AC_POL_PORT_CFG_POL_PORT_THRES_CFG_0_PORT_THRES0(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_AC_POL_PORT_CFG_POL_PORT_THRES_CFG_0_PORT_THRES0     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_AC_POL_PORT_CFG_POL_PORT_THRES_CFG_0_PORT_THRES0(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Port policer lower threshold configuration
 *
 * \details
 * These registers configure the lower thresholds of the port policers. The
 * registers are index by 4 x port number + port policer index.
 *
 * Register: \a ANA_AC:POL_PORT_CFG:POL_PORT_THRES_CFG_1
 *
 * @param ri Replicator: x_ANA_NUM_PORTS_X_NUM_PORT_POL (??), 0-131
 */
#define VTSS_ANA_AC_POL_PORT_CFG_POL_PORT_THRES_CFG_1(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bd00 + (ri))

/** 
 * \brief
 * Hysteresis size for port policer. Unit is 2048 bytes. PORT_THRES1 is
 * used when a port policer is in flow control mode. Flow control is
 * asserted when the bucket level exceeds PORT_THRES0. Flow control is
 * deasserted when the bucket has leaked PORT_THRES1 bytes since the
 * assertion. PORT_THRES1 must be programmed smaller or equal to
 * PORT_THRES0.
 *
 * \details 
 * 0 : No hysteresis
 * 1: Deassert flow control when bucket has leaked 2048 bytes
 * ...
 * n: Deassert flow control when bucket has leaked n * 2048 bytes

 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CFG_POL_PORT_THRES_CFG_1 . PORT_THRES1
 */
#define  VTSS_F_ANA_AC_POL_PORT_CFG_POL_PORT_THRES_CFG_1_PORT_THRES1(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_AC_POL_PORT_CFG_POL_PORT_THRES_CFG_1_PORT_THRES1     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_AC_POL_PORT_CFG_POL_PORT_THRES_CFG_1_PORT_THRES1(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Port policer rate configuration
 *
 * \details
 * These registers configure the rates of the port policers. The registers
 * are index by 4 x port number + port policer index.
 *
 * Register: \a ANA_AC:POL_PORT_CFG:POL_PORT_RATE_CFG
 *
 * @param ri Replicator: x_ANA_NUM_PORTS_X_NUM_PORT_POL (??), 0-131
 */
#define VTSS_ANA_AC_POL_PORT_CFG_POL_PORT_RATE_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1be00 + (ri))

/** 
 * \brief
 * Policer leaky bucket rate. Unit is 100160 bps.
 *
 * \details 
 * 0: Open until burst capacity is used, then closed.
 * 1: Rate = 100160 bps
 * n: Rate = n x 100160 bps
 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CFG_POL_PORT_RATE_CFG . PORT_RATE
 */
#define  VTSS_F_ANA_AC_POL_PORT_CFG_POL_PORT_RATE_CFG_PORT_RATE(x)  VTSS_ENCODE_BITFIELD(x,0,17)
#define  VTSS_M_ANA_AC_POL_PORT_CFG_POL_PORT_RATE_CFG_PORT_RATE     VTSS_ENCODE_BITMASK(0,17)
#define  VTSS_X_ANA_AC_POL_PORT_CFG_POL_PORT_RATE_CFG_PORT_RATE(x)  VTSS_EXTRACT_BITFIELD(x,0,17)

/**
 * Register Group: \a ANA_AC:POL_PORT_CTRL
 *
 * Port policer controls indexed per port
 */


/** 
 * \brief Port policer GAP configuration
 *
 * \details
 * These registers configure the gap value for the port policers, which is
 * common for all port policers within a port
 *
 * Register: \a ANA_AC:POL_PORT_CTRL:POL_PORT_GAP
 *
 * @param gi Replicator: x_ANA_NUM_PORT_POLS (??), 0-32
 */
#define VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_GAP(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1dc00,gi,8,0,0)

/** 
 * \brief
 * Value added to each frame before updating the bucket. Gap value range:
 * -64 to +63 in two's complement format.
 * When GAP_VALUE = 20, a line-rate measurement because each frame occupies
 * 12 bytes for the inter-frame gap and 8 bytes for the preamble on the
 * line. When GAP_VALUE = 0, a data-rate measurement is achived.
 *
 * \details 
 * 0x40: -64
 * 0x41: -63
 *    ...
 * 0x7F: -1
 * 0x00: 0
 * 0x01: 1
 *    ...
 * 0x3F: 63
 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_GAP . GAP_VALUE
 */
#define  VTSS_F_ANA_AC_POL_PORT_CTRL_POL_PORT_GAP_GAP_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_ANA_AC_POL_PORT_CTRL_POL_PORT_GAP_GAP_VALUE     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_ANA_AC_POL_PORT_CTRL_POL_PORT_GAP_GAP_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief Port policer configuration
 *
 * \details
 * These registers configure the behavior of port policer.
 *
 * Register: \a ANA_AC:POL_PORT_CTRL:POL_PORT_CFG
 *
 * @param gi Replicator: x_ANA_NUM_PORT_POLS (??), 0-32
 * @param ri Replicator: x_ANA_NUM_PORT_LB_SET_SIZE (??), 0-3
 */
#define VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1dc00,gi,8,ri,1)

/** 
 * \brief
 * Controls policing of frames to the individual CPU queues for the port
 * policer. If a bit is set in this mask, frames to the corresponding CPU
 * queue are allowed to be policed (frames may get discarded and the bucket
 * is updated with the frame).
 *
 * \details 
 * '00..00': Disable policing of frames to all CPU queues
 * 'xx..x1 ': Allow policing of frames to CPU queue #0
 *     ...
 * '1x..xx' : Allow policing of frames to CPU queue #n
 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG . CPU_QU_MASK
 */
#define  VTSS_F_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_CPU_QU_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_CPU_QU_MASK     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_CPU_QU_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Configuration of Drop Precedence bypass level. Frames with DP below
 * DP_BYPASS_LVL bypass the policer (frames are never policed and the
 * bucket is not updated with the frames).
 *
 * \details 
 * 0: No frames bypass the policer
 * 1: Frames with DP = 0 bypass the policer
 * 2: Frames with DP = 0 or 1 bypass the policer
 * 3: Frames with DP = 0, 1 or 2 bypass the policer
 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG . DP_BYPASS_LVL
 */
#define  VTSS_F_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_DP_BYPASS_LVL(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_DP_BYPASS_LVL     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_DP_BYPASS_LVL(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * Enables frame rate mode for the port policer, where policer rates are
 * measured in frames per second instead of bits per second.
 *
 * \details 
 * 0: Rates measured in bits per second
 * 1: Rates measured in frames per second
 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG . FRAME_RATE_ENA
 */
#define  VTSS_F_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_FRAME_RATE_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Enables removing the front ports from a policed frame's destination set.
 *
 * \details 
 * 0: The policer does not remove the front ports from the destination set
 * for a policed frame.
 * 0: The policer removes the front ports from the destination set for a
 * policed frame.
 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG . LIMIT_NONCPU_TRAFFIC_ENA
 */
#define  VTSS_F_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_LIMIT_NONCPU_TRAFFIC_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Enables removing the CPU ports from a policed frame's destination set.
 *
 * \details 
 * 0: The policer does not remove the CPU ports from the destination set
 * for a policed frame.
 * 1: The policer removes the CPU ports from the destination set for a
 * policed frame.
 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG . LIMIT_CPU_TRAFFIC_ENA
 */
#define  VTSS_F_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_LIMIT_CPU_TRAFFIC_ENA  VTSS_BIT(9)

/** 
 * \brief
 * Enables whether flooded multicast frames are part of the multicast
 * policer category (see TRAFFIC_TYPE_MASK).
 *
 * \details 
 * 0: Multicast frames include flooded multicast frames. 
 * 1: Multicast frames exclude flooded multicast frames. Only known
 * multicast frames are considered multicast frames with respect to the
 * port policer.
 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG . MC_WITHOUT_FLOOD_ENA
 */
#define  VTSS_F_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_MC_WITHOUT_FLOOD_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enables whether flooded unicast frames are part of the unicast policer
 * category (see TRAFFIC_TYPE_MASK).
 *
 * \details 
 * 0: Unicast frames include flooded unicast frames. 
 * 1: Unicast frames exclude flooded unicast frames. Only known unicast
 * frames are considered unicast frames with respect to the port policer.
 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG . UC_WITHOUT_FLOOD_ENA
 */
#define  VTSS_F_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_UC_WITHOUT_FLOOD_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Configures the traffic types to be policed by the policer. Each bit in
 * the mask enables a specific frame type. Multiple types can be enabled at
 * the same time. If the CPU queue bit is set, traffic to CPU queues
 * specified in CPU_QU_MASK are not policed.
 * If the CPU queue bit is cleared, frames destined for CPU queues
 * specified in CPU_QU_MASK bypass the policer even though the frames are
 * of a type marked elsewhere in TRAFFIC_TYPE_MASK. 
 * A frame can belong to multiple traffic types (for instance flooding and
 * multicast). If a frame type is not set in the mask, the frames of that
 * type only, bypass the policer (never discarded and the bucket is not
 * updated with the frame).

 *
 * \details 
 * '00000': Disable policer.
 * 'xxxxx1' : Multicast frames are policed. See for MC_WITHOUT_FLOOD_ENA
 * for configuration of the multicast type.
 * 'xxxx1x' : Broadcast frames are policed.
 * 'xxx1xx' : Unicast frames are policed. See for UC_WITHOUT_FLOOD_ENA for
 * configuration of the unicast type.
 * 'xx1xxx' : Flooded frames are policed.
 * 'x0xxxx' : Frames to a CPU queue selected by CPU_QU_MASK bypass the
 * policer.
 * 'x1xxxx' : Frames to a CPU queue selected by CPU_QU_MASK are policed.
 * '1xxxxx' : Learn frames are policed.

 *
 * Field: ::VTSS_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG . TRAFFIC_TYPE_MASK
 */
#define  VTSS_F_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_TRAFFIC_TYPE_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_TRAFFIC_TYPE_MASK     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_AC_POL_PORT_CTRL_POL_PORT_CFG_TRAFFIC_TYPE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a ANA_AC:POL_PRIO_CFG
 *
 * Priority policer controls
 */


/** 
 * \brief Priority policer rate configuration
 *
 * \details
 * These registers configure the rates of the priority policers. The
 * registers are index queue index.
 *
 * Register: \a ANA_AC:POL_PRIO_CFG:POL_PRIO_RATE_CFG
 *
 * @param ri Replicator: x_ANA_NUM_PRIO_X_NUM_PORTS_X_NUM_PRIO_POL (??), 0-263
 */
#define VTSS_ANA_AC_POL_PRIO_CFG_POL_PRIO_RATE_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d000 + (ri))

/** 
 * \brief
 * Policer leaky bucket rate. Unit is 100160 bps.
 *
 * \details 
 * 0: Open until burst capacity is used, then closed.
 * 1: Rate = 100160 bps
 * n: Rate = n x 100160 bps
 *
 * Field: ::VTSS_ANA_AC_POL_PRIO_CFG_POL_PRIO_RATE_CFG . PRIO_RATE
 */
#define  VTSS_F_ANA_AC_POL_PRIO_CFG_POL_PRIO_RATE_CFG_PRIO_RATE(x)  VTSS_ENCODE_BITFIELD(x,0,17)
#define  VTSS_M_ANA_AC_POL_PRIO_CFG_POL_PRIO_RATE_CFG_PRIO_RATE     VTSS_ENCODE_BITMASK(0,17)
#define  VTSS_X_ANA_AC_POL_PRIO_CFG_POL_PRIO_RATE_CFG_PRIO_RATE(x)  VTSS_EXTRACT_BITFIELD(x,0,17)


/** 
 * \brief Priority policer threshold configuration
 *
 * \details
 * These registers configure the thresholds of the priority policers. The
 * registers uses per queue index.
 *
 * Register: \a ANA_AC:POL_PRIO_CFG:POL_PRIO_THRES_CFG
 *
 * @param ri Replicator: x_ANA_NUM_PRIO_X_NUM_PORTS_X_NUM_PRIO_POL (??), 0-263
 */
#define VTSS_ANA_AC_POL_PRIO_CFG_POL_PRIO_THRES_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d200 + (ri))

/** 
 * \brief
 * Policer threshold size (a.ka. burst capacity). Unit is 2048 bytes.
 *
 * \details 
 * 0: Always closed
 * 1: Burst capacity = 2048 bytes
 * ...
 * n: Burst capacity = n x 2048 bytes
 *
 * Field: ::VTSS_ANA_AC_POL_PRIO_CFG_POL_PRIO_THRES_CFG . PRIO_THRES
 */
#define  VTSS_F_ANA_AC_POL_PRIO_CFG_POL_PRIO_THRES_CFG_PRIO_THRES(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_AC_POL_PRIO_CFG_POL_PRIO_THRES_CFG_PRIO_THRES     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_AC_POL_PRIO_CFG_POL_PRIO_THRES_CFG_PRIO_THRES(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a ANA_AC:POL_PRIO_CTRL
 *
 * Priority policer configuration indexed per queue.
 */


/** 
 * \brief Priority policer GAP configuration
 *
 * \details
 * Register: \a ANA_AC:POL_PRIO_CTRL:POL_PRIO_GAP
 *
 * @param gi Replicator: x_ANA_NUM_PRIO_POLS (??), 0-263
 */
#define VTSS_ANA_AC_POL_PRIO_CTRL_POL_PRIO_GAP(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1d800,gi,2,0,0)

/** 
 * \brief
 * Value added to each frame before updating the bucket. Gap value range:
 * -64 to +63 in two's complement format.
 * When GAP_VALUE = 20, a line-rate measurement because each frame occupies
 * 12 bytes for the inter-frame gap and 8 bytes for the preamble on the
 * line. When GAP_VALUE = 0, a data-rate measurement is achived.
 *
 * \details 
 * 0x40: -64
 * 0x41: -63
 *    ...
 * 0x7F: -1
 * 0x00: 0
 * 0x01: 1
 *    ...
 * 0x3F: 63
 *
 * Field: ::VTSS_ANA_AC_POL_PRIO_CTRL_POL_PRIO_GAP . GAP_VALUE
 */
#define  VTSS_F_ANA_AC_POL_PRIO_CTRL_POL_PRIO_GAP_GAP_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_ANA_AC_POL_PRIO_CTRL_POL_PRIO_GAP_GAP_VALUE     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_ANA_AC_POL_PRIO_CTRL_POL_PRIO_GAP_GAP_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief Priority policer configuration
 *
 * \details
 * Register: \a ANA_AC:POL_PRIO_CTRL:POL_PRIO_CFG
 *
 * @param gi Replicator: x_ANA_NUM_PRIO_POLS (??), 0-263
 */
#define VTSS_ANA_AC_POL_PRIO_CTRL_POL_PRIO_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1d800,gi,2,0,1)

/** 
 * \brief
 * Configuration of Drop Precedence bypass level. Frames with DP below
 * DP_BYPASS_LVL bypass the policer (frames are never policed and the
 * bucket is not updated with the frames).
 *
 * \details 
 * 0: No frames bypass the policer
 * 1: Frames with DP = 0 bypass the policer
 * 2: Frames with DP = 0 or 1 bypass the policer
 * 3: Frames with DP = 0, 1 or 2 bypass the policer
 *
 * Field: ::VTSS_ANA_AC_POL_PRIO_CTRL_POL_PRIO_CFG . DP_BYPASS_LVL
 */
#define  VTSS_F_ANA_AC_POL_PRIO_CTRL_POL_PRIO_CFG_DP_BYPASS_LVL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_AC_POL_PRIO_CTRL_POL_PRIO_CFG_DP_BYPASS_LVL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_AC_POL_PRIO_CTRL_POL_PRIO_CFG_DP_BYPASS_LVL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a ANA_AC:STAT_GLOBAL_CFG_PORT
 *
 * Global configuration.
 */


/** 
 * \brief Event mask for counter.
 *
 * \details
 * Event mask for counter of all flows.
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_PORT:STAT_GLOBAL_EVENT_MASK
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-3
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_PORT_STAT_GLOBAL_EVENT_MASK(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbc8 + (ri))

/** 
 * \brief
 * This value stores the event mask which indicates the counter of all
 * flows to count certain events.
 *  If set to '1' the respective event is not filtered and can trigger the
 * counter.
 *  If set to '0' the respective event is filtered and the counter will
 * treat the frame as if no event has occurred.
 *  
 *  Which type of frame is counted is defined in: STAT_CFG,
 * CFG_CNT_FRM_TYPE.
 * 
 *  The following events apply to port stat:
 *  0. Count events allowed by *_sticky_mask:0
 *  1. Count events allowed by *_sticky_mask:1
 *  ...
 *  n-1. Count events allowed by *_sticky_mask:n 
 *  where n is number of counter event masks
 * 
 *  n. Count port policer:0 drop events
 *  n+1. Count port policer:1 drop events
 *  ...
 *  n+m-1. Count port policer:1 drop events
 *  where m is number of per port policers 
 *
 * \details 
 * If an asserted event is masked out in the GLOBAL_EVENT_MASK, the
 * incoming frame should not be counted for all flows.
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_PORT_STAT_GLOBAL_EVENT_MASK . GLOBAL_EVENT_MASK
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_PORT_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_AC_STAT_GLOBAL_CFG_PORT_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_AC_STAT_GLOBAL_CFG_PORT_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Reset all statistic counter.
 *
 * \details
 * Writing '1' into this register will lead to the reset of STAT.
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_PORT:STAT_RESET
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_PORT_STAT_RESET  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbcc)

/** 
 * \brief
 * Writing '1' into this bit will lead to the reset of STAT (counter and
 * configuration). This bit is auto-cleared after the reset is done.
 *
 * \details 
 * '0': No action
 * '1': to reset STAT (Bit is automatically cleared)
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_PORT_STAT_RESET . RESET
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_PORT_STAT_RESET_RESET  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:STAT_CNT_CFG_PORT
 *
 * Individual configuration.
 */


/** 
 * \brief Sticky bits for events.
 *
 * \details
 * It is the sticky bits of events. If one event is triggered, the
 * corresponding bit is set to '1' before it is cleared. To write '1' into
 * this bit will clear the sticky bit.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_PORT:STAT_EVENTS_STICKY
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-34
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_PORT_STAT_EVENTS_STICKY(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1d400,gi,16,0,0)

/** 
 * \brief
 * These are the sticky bits of events. There is a sticky bit for an event
 * for each flow.
 *
 * \details 
 * '1': The corresponding event is triggered since it is cleared last time.
 * '0': No such event is triggered since it is cleared last time.
 *
 * Field: ::VTSS_ANA_AC_STAT_CNT_CFG_PORT_STAT_EVENTS_STICKY . STICKY_BITS
 */
#define  VTSS_F_ANA_AC_STAT_CNT_CFG_PORT_STAT_EVENTS_STICKY_STICKY_BITS(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_AC_STAT_CNT_CFG_PORT_STAT_EVENTS_STICKY_STICKY_BITS     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_AC_STAT_CNT_CFG_PORT_STAT_EVENTS_STICKY_STICKY_BITS(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Event handling configuration.
 *
 * \details
 * This register group defines how to handle the incoming events.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_PORT:STAT_CFG
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-34
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-3
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1d400,gi,16,ri,1)

/** 
 * \brief
 * This field is to configure the counters of a flow to count frames with
 * certain priorities.
 *
 * \details 
 * If an asserted priority is masked out in the cfg_prio_mask, the incoming
 * frame should not be counted.
 *
 * Field: ::VTSS_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG . CFG_PRIO_MASK
 */
#define  VTSS_F_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG_CFG_PRIO_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,8)
#define  VTSS_M_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG_CFG_PRIO_MASK     VTSS_ENCODE_BITMASK(4,8)
#define  VTSS_X_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG_CFG_PRIO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,8)

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
 * Field: ::VTSS_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG . CFG_CNT_FRM_TYPE
 */
#define  VTSS_F_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG_CFG_CNT_FRM_TYPE(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG_CFG_CNT_FRM_TYPE     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG_CFG_CNT_FRM_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * This field is to configure the counters of a flow to count the byte
 * number or the frame number.
 *
 * \details 
 * '0': The frame number is counted.
 * '1': The byte number is counted.
 *
 * Field: ::VTSS_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG . CFG_CNT_BYTE
 */
#define  VTSS_F_ANA_AC_STAT_CNT_CFG_PORT_STAT_CFG_CFG_CNT_BYTE  VTSS_BIT(0)


/** 
 * \brief Least significant 32 bits.
 *
 * \details
 * This register contains the least significant 32 bits of a counter.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_PORT:STAT_LSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-34
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-3
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_PORT_STAT_LSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1d400,gi,16,ri,5)


/** 
 * \brief Most significant 8 bits.
 *
 * \details
 * This register contains the most significant 8 bits of a counter.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_PORT:STAT_MSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-34
 * @param ri Replicator: x_STAT_CNT_MSB_PER_FLW_ID (??), 0-3
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_PORT_STAT_MSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1d400,gi,16,ri,9)

/** 
 * \brief
 * The field stores the value in the counters of a flow from bit 32 to the
 * most significant bit.
 *
 * \details 
 * Field: ::VTSS_ANA_AC_STAT_CNT_CFG_PORT_STAT_MSB_CNT . MSB_CNT
 */
#define  VTSS_F_ANA_AC_STAT_CNT_CFG_PORT_STAT_MSB_CNT_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_AC_STAT_CNT_CFG_PORT_STAT_MSB_CNT_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_AC_STAT_CNT_CFG_PORT_STAT_MSB_CNT_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a ANA_AC:STAT_GLOBAL_CFG_ISDX
 *
 * Global configuration.
 */


/** 
 * \brief Global configuration register
 *
 * \details
 * Global configuration register
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_ISDX:STAT_GLOBAL_CFG
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbcd + (ri))

/** 
 * \brief
 * This field is to configure the counters of all flows to count the byte
 * number or the frame number.
 *
 * \details 
 * '0': The frame number is counted.
 * '1': The byte number is counted.
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_CFG . GLOBAL_CFG_CNT_BYTE
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_CFG_GLOBAL_CFG_CNT_BYTE  VTSS_BIT(0)


/** 
 * \brief Event mask for counter.
 *
 * \details
 * Event mask for counter of all flows.
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_ISDX:STAT_GLOBAL_EVENT_MASK
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_EVENT_MASK(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbd3 + (ri))

/** 
 * \brief
 * This value stores the event mask which indicates the counter of all
 * flows to count certain events.
 *  If set to '1' the respective event is not filtered and can trigger the
 * counter.
 *  If set to '0' the respective event is filtered and the counter will
 * treat the frame as if no event has occurred.
 *  
 *  Which type of frame is counted is defined in: STAT_GLOBAL_CFG.
 *  
 *  The following events apply to ISDX stat:
 *  For CE:
 *  Bit0: Count GREEN traffic
 *  Bit1: Count YELLOW traffic
 *  Bit2: Count RED traffic
 *  
 *  For SME:
 *  Bit3: Count unicast traffic
 *  Bit4: Count multicast traffic
 *  Bit5: Count flooded traffic 
 *
 * \details 
 * If an asserted event is masked out in the GLOBAL_EVENT_MASK, the
 * incoming frame should not be counted for all flows.
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_EVENT_MASK . GLOBAL_EVENT_MASK
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Reset all statistic counter.
 *
 * \details
 * Writing '1' into this register will lead to the reset of STAT.
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_ISDX:STAT_RESET
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_RESET  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbd9)

/** 
 * \brief
 * Writing '1' into this bit will lead to the reset of STAT (counter and
 * configuration). This bit is auto-cleared after the reset is done.
 *
 * \details 
 * '0': No action
 * '1': to reset STAT (Bit is automatically cleared)
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_RESET . RESET
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_ISDX_STAT_RESET_RESET  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:STAT_CNT_CFG_ISDX
 *
 * Individual configuration.
 */


/** 
 * \brief Least significant 32 bits.
 *
 * \details
 * This register contains the least significant 32 bits of a counter.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_ISDX:STAT_LSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-4095
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_ISDX_STAT_LSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x0,gi,16,ri,0)


/** 
 * \brief Most significant 8 bits.
 *
 * \details
 * This register contains the most significant 8 bits of a counter.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_ISDX:STAT_MSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-4095
 * @param ri Replicator: x_STAT_CNT_MSB_PER_FLW_ID (??), 0-2
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_ISDX_STAT_MSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x0,gi,16,ri,6)

/** 
 * \brief
 * The field stores the value in the counters of a flow from bit 32 to the
 * most significant bit.
 *
 * \details 
 * Field: ::VTSS_ANA_AC_STAT_CNT_CFG_ISDX_STAT_MSB_CNT . MSB_CNT
 */
#define  VTSS_F_ANA_AC_STAT_CNT_CFG_ISDX_STAT_MSB_CNT_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_AC_STAT_CNT_CFG_ISDX_STAT_MSB_CNT_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_AC_STAT_CNT_CFG_ISDX_STAT_MSB_CNT_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a ANA_AC:STAT_GLOBAL_CFG_TUNNEL
 *
 * Global configuration.
 */


/** 
 * \brief Global configuration register
 *
 * \details
 * Global configuration register
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_TUNNEL:STAT_GLOBAL_CFG
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_GLOBAL_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbda + (ri))

/** 
 * \brief
 * This field is to configure the counters of all flows to count the byte
 * number or the frame number.
 *
 * \details 
 * '0': The frame number is counted.
 * '1': The byte number is counted.
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_GLOBAL_CFG . GLOBAL_CFG_CNT_BYTE
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_GLOBAL_CFG_GLOBAL_CFG_CNT_BYTE  VTSS_BIT(0)


/** 
 * \brief Event mask for counter.
 *
 * \details
 * Event mask for counter of all flows.
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_TUNNEL:STAT_GLOBAL_EVENT_MASK
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_GLOBAL_EVENT_MASK(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbe0 + (ri))

/** 
 * \brief
 * This value stores the event mask which indicates the counter of all
 * tunnels to count certain events.
 *  If set to '1' the respective event is not filtered and can trigger the
 * counter.
 *  If set to '0' the respective event is filtered and the counter will
 * treat the frame as if no event has occurred.
 *  
 *  Which type of frame is counted is defined in: STAT_GLOBAL_CFG.
 *  
 *  The following events apply to Tunnel stat:
 *  Bit0: Count GREEN traffic
 *  Bit1: Count YELLOW traffic
 *  Bit2: Count RED traffic 
 *
 * \details 
 * If an asserted event is masked out in the GLOBAL_EVENT_MASK, the
 * incoming frame should not be counted for all flows.
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_GLOBAL_EVENT_MASK . GLOBAL_EVENT_MASK
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Reset all statistic counter.
 *
 * \details
 * Writing '1' into this register will lead to the reset of STAT.
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_TUNNEL:STAT_RESET
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_RESET  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbe6)

/** 
 * \brief
 * Writing '1' into this bit will lead to the reset of STAT (counter and
 * configuration). This bit is auto-cleared after the reset is done.
 *
 * \details 
 * '0': No action
 * '1': to reset STAT (Bit is automatically cleared)
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_RESET . RESET
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_TUNNEL_STAT_RESET_RESET  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:STAT_CNT_CFG_TUNNEL
 *
 * Individual configuration.
 */


/** 
 * \brief Least significant 32 bits.
 *
 * \details
 * This register contains the least significant 32 bits of a counter.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_TUNNEL:STAT_LSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-511
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_TUNNEL_STAT_LSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x18000,gi,16,ri,0)


/** 
 * \brief Most significant 8 bits.
 *
 * \details
 * This register contains the most significant 8 bits of a counter.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_TUNNEL:STAT_MSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-511
 * @param ri Replicator: x_STAT_CNT_MSB_PER_FLW_ID (??), 0-2
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_TUNNEL_STAT_MSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x18000,gi,16,ri,6)

/** 
 * \brief
 * The field stores the value in the counters of a flow from bit 32 to the
 * most significant bit.
 *
 * \details 
 * Field: ::VTSS_ANA_AC_STAT_CNT_CFG_TUNNEL_STAT_MSB_CNT . MSB_CNT
 */
#define  VTSS_F_ANA_AC_STAT_CNT_CFG_TUNNEL_STAT_MSB_CNT_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_AC_STAT_CNT_CFG_TUNNEL_STAT_MSB_CNT_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_AC_STAT_CNT_CFG_TUNNEL_STAT_MSB_CNT_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a ANA_AC:STAT_GLOBAL_CFG_IRLEG
 *
 * Global configuration.
 */


/** 
 * \brief Global configuration register
 *
 * \details
 * Global configuration register
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_IRLEG:STAT_GLOBAL_CFG
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_GLOBAL_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbe7 + (ri))

/** 
 * \brief
 * This field is to configure the counters of all flows to count the byte
 * number or the frame number.
 *
 * \details 
 * '0': The frame number is counted.
 * '1': The byte number is counted.
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_GLOBAL_CFG . GLOBAL_CFG_CNT_BYTE
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_GLOBAL_CFG_GLOBAL_CFG_CNT_BYTE  VTSS_BIT(0)


/** 
 * \brief Event mask for counter.
 *
 * \details
 * Event mask for counter of all flows.
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_IRLEG:STAT_GLOBAL_EVENT_MASK
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_GLOBAL_EVENT_MASK(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbed + (ri))

/** 
 * \brief
 * This value stores the event mask which indicates the counter of all
 * flows to count certain events.
 *  If set to '1' the respective event is not filtered and can trigger the
 * counter.
 *  If set to '0' the respective event is filtered and the counter will
 * treat the frame as if no event has occurred.
 *  
 *  Which type of frame is counted is defined in: STAT_CFG,
 * CFG_CNT_FRM_TYPE.
 * 
 *  The following events apply to irleg stat:
 *  Bit0: Count acl_discarded traffic 
 *   Bit1: Count received traffic 
 *   Bit2: Count vlan_discarded traffic 
 *   Bit3: Count ip4_unicast_routed traffic 
 *   Bit4: Count ip6_unicast_routed traffic 
 *   Bit5: Count ip_multicast_routed traffic 
 *   Bit6: Count ip_multicast_rpf_discarded traffic 
 *  Bit7: Count ip_TTL_discarded traffic
 *
 * \details 
 * If an asserted event is masked out in the GLOBAL_EVENT_MASK, the
 * incoming frame should not be counted for all flows.
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_GLOBAL_EVENT_MASK . GLOBAL_EVENT_MASK
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Reset all statistic counter.
 *
 * \details
 * Writing '1' into this register will lead to the reset of STAT.
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_IRLEG:STAT_RESET
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_RESET  VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbf3)

/** 
 * \brief
 * Writing '1' into this bit will lead to the reset of STAT (counter and
 * configuration). This bit is auto-cleared after the reset is done.
 *
 * \details 
 * '0': No action
 * '1': to reset STAT (Bit is automatically cleared)
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_RESET . RESET
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_IRLEG_STAT_RESET_RESET  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:STAT_CNT_CFG_IRLEG
 *
 * Individual configuration.
 */


/** 
 * \brief Least significant 32 bits.
 *
 * \details
 * This register contains the least significant 32 bits of a counter.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_IRLEG:STAT_LSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-127
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_IRLEG_STAT_LSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1c000,gi,16,ri,0)


/** 
 * \brief Most significant 8 bits.
 *
 * \details
 * This register contains the most significant 8 bits of a counter.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_IRLEG:STAT_MSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-127
 * @param ri Replicator: x_STAT_CNT_MSB_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_IRLEG_STAT_MSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1c000,gi,16,ri,6)

/** 
 * \brief
 * The field stores the value in the counters of a flow from bit 32 to the
 * most significant bit.
 *
 * \details 
 * Field: ::VTSS_ANA_AC_STAT_CNT_CFG_IRLEG_STAT_MSB_CNT . MSB_CNT
 */
#define  VTSS_F_ANA_AC_STAT_CNT_CFG_IRLEG_STAT_MSB_CNT_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_AC_STAT_CNT_CFG_IRLEG_STAT_MSB_CNT_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_AC_STAT_CNT_CFG_IRLEG_STAT_MSB_CNT_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a ANA_AC:STAT_GLOBAL_CFG_ERLEG
 *
 * Global configuration.
 */


/** 
 * \brief Global configuration register
 *
 * \details
 * Global configuration register
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_ERLEG:STAT_GLOBAL_CFG
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_GLOBAL_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d6b4 + (ri))

/** 
 * \brief
 * This field is to configure the counters of all flows to count the byte
 * number or the frame number.
 *
 * \details 
 * '0': The frame number is counted.
 * '1': The byte number is counted.
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_GLOBAL_CFG . GLOBAL_CFG_CNT_BYTE
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_GLOBAL_CFG_GLOBAL_CFG_CNT_BYTE  VTSS_BIT(0)


/** 
 * \brief Event mask for counter.
 *
 * \details
 * Event mask for counter of all flows.
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_ERLEG:STAT_GLOBAL_EVENT_MASK
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_GLOBAL_EVENT_MASK(ri)  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d6ba + (ri))

/** 
 * \brief
 * This value stores the event mask which indicates the counter of all
 * flows to count certain events.
 *  If set to '1' the respective event is not filtered and can trigger the
 * counter.
 *  If set to '0' the respective event is filtered and the counter will
 * treat the frame as if no event has occurred.
 * 
 *  The following events apply to erleg stat:
 *  Bit0: Count acl_discarded traffic 
 *   Bit1: Count received traffic 
 *   Bit2: Count vlan_discarded traffic 
 *   Bit3: Count ip4_unicast_routed traffic 
 *   Bit4: Count ip6_unicast_routed traffic 
 *   Bit5: Count ip_multicast_routed traffic 
 *   Bit6: Count ip_multicast_switched traffic 
 *  Bit7: Count ip_multicast_TTL_discarded traffic
 *
 * \details 
 * If an asserted event is masked out in the GLOBAL_EVENT_MASK, the
 * incoming frame should not be counted for all flows.
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_GLOBAL_EVENT_MASK . GLOBAL_EVENT_MASK
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Reset all statistic counter.
 *
 * \details
 * Writing '1' into this register will lead to the reset of STAT.
 *
 * Register: \a ANA_AC:STAT_GLOBAL_CFG_ERLEG:STAT_RESET
 */
#define VTSS_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_RESET  VTSS_IOREG(VTSS_TO_ANA_AC,0x1d6c0)

/** 
 * \brief
 * Writing '1' into this bit will lead to the reset of STAT (counter and
 * configuration). This bit is auto-cleared after the reset is done.
 *
 * \details 
 * '0': No action
 * '1': to reset STAT (Bit is automatically cleared)
 *
 * Field: ::VTSS_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_RESET . RESET
 */
#define  VTSS_F_ANA_AC_STAT_GLOBAL_CFG_ERLEG_STAT_RESET_RESET  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:STAT_CNT_CFG_ERLEG
 *
 * Individual configuration.
 */


/** 
 * \brief Least significant 32 bits.
 *
 * \details
 * This register contains the least significant 32 bits of a counter.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_ERLEG:STAT_LSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-127
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_ERLEG_STAT_LSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1c800,gi,16,ri,0)


/** 
 * \brief Most significant 8 bits.
 *
 * \details
 * This register contains the most significant 8 bits of a counter.
 *
 * Register: \a ANA_AC:STAT_CNT_CFG_ERLEG:STAT_MSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-127
 * @param ri Replicator: x_STAT_CNT_MSB_PER_FLW_ID (??), 0-5
 */
#define VTSS_ANA_AC_STAT_CNT_CFG_ERLEG_STAT_MSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1c800,gi,16,ri,6)

/** 
 * \brief
 * The field stores the value in the counters of a flow from bit 32 to the
 * most significant bit.
 *
 * \details 
 * Field: ::VTSS_ANA_AC_STAT_CNT_CFG_ERLEG_STAT_MSB_CNT . MSB_CNT
 */
#define  VTSS_F_ANA_AC_STAT_CNT_CFG_ERLEG_STAT_MSB_CNT_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_AC_STAT_CNT_CFG_ERLEG_STAT_MSB_CNT_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_AC_STAT_CNT_CFG_ERLEG_STAT_MSB_CNT_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a ANA_AC:COMMON_SDLB
 *
 * Common DLB configuration and status registers
 */


/** 
 * \brief Configuration of common dual leaky bucket handling
 *
 * \details
 * Register: \a ANA_AC:COMMON_SDLB:DLB_CTRL
 */
#define VTSS_ANA_AC_COMMON_SDLB_DLB_CTRL     VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbf4)

/** 
 * \brief
 * Specifies the number of clock cycles for a tick.
 *
 * \details 
 * 0: 1 clk
 * ...
 * n: 1+n clk
 *
 * Field: ::VTSS_ANA_AC_COMMON_SDLB_DLB_CTRL . BASE_TICK_CNT
 */
#define  VTSS_F_ANA_AC_COMMON_SDLB_DLB_CTRL_BASE_TICK_CNT(x)  VTSS_ENCODE_BITFIELD(x,4,13)
#define  VTSS_M_ANA_AC_COMMON_SDLB_DLB_CTRL_BASE_TICK_CNT     VTSS_ENCODE_BITMASK(4,13)
#define  VTSS_X_ANA_AC_COMMON_SDLB_DLB_CTRL_BASE_TICK_CNT(x)  VTSS_EXTRACT_BITFIELD(x,4,13)

/** 
 * \brief
 * If set, the policers are initialized and all buckets are open. The bit
 * must be cleared for normal operation to resume.
 *
 * \details 
 * 0: Normal operation
 * 1: Initialization
 *
 * Field: ::VTSS_ANA_AC_COMMON_SDLB_DLB_CTRL . DLB_INIT_SHOT
 */
#define  VTSS_F_ANA_AC_COMMON_SDLB_DLB_CTRL_DLB_INIT_SHOT  VTSS_BIT(2)

/** 
 * \brief
 * Enables leaking from the dual leaky buckets.
 *
 * \details 
 * 0: Disable bucket leaking
 * 1: Enable bucket leaking
 *
 * Field: ::VTSS_ANA_AC_COMMON_SDLB_DLB_CTRL . LEAK_ENA
 */
#define  VTSS_F_ANA_AC_COMMON_SDLB_DLB_CTRL_LEAK_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enables adding of frame bytes to the dual leaky buckets.
 *
 * \details 
 * 0: Disable bucket addition
 * 1: Enable bucket addition
 *
 * Field: ::VTSS_ANA_AC_COMMON_SDLB_DLB_CTRL . DLB_ADD_ENA
 */
#define  VTSS_F_ANA_AC_COMMON_SDLB_DLB_CTRL_DLB_ADD_ENA  VTSS_BIT(0)


/** 
 * \brief DLB policer diagnostic
 *
 * \details
 * Register: \a ANA_AC:COMMON_SDLB:DLB_STICKY
 */
#define VTSS_ANA_AC_COMMON_SDLB_DLB_STICKY   VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbf6)

/** 
 * \brief
 * Set when a DLB scan could not start because a scan is already ongoing -
 * BASE_TICK_CNT must be increased.
 *
 * \details 
 * 0: No event has occured
 * 1: Leak scan could not start at time
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_COMMON_SDLB_DLB_STICKY . LEAK_START_DELAYED_STICKY
 */
#define  VTSS_F_ANA_AC_COMMON_SDLB_DLB_STICKY_LEAK_START_DELAYED_STICKY  VTSS_BIT(29)

/** 
 * \brief
 * Set when the frame rate is exceeding the Peak Information Rate.
 *
 * \details 
 * 0: No event has occured
 * 1: PIR exceeded
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_COMMON_SDLB_DLB_STICKY . PIR_EXCEEDED_STICKY
 */
#define  VTSS_F_ANA_AC_COMMON_SDLB_DLB_STICKY_PIR_EXCEEDED_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set when the frame rate is exceeding the Committed Information Rate.
 *
 * \details 
 * 0: No event has occured
 * 1: CIR exceeded
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_COMMON_SDLB_DLB_STICKY . CIR_EXCEEDED_STICKY
 */
#define  VTSS_F_ANA_AC_COMMON_SDLB_DLB_STICKY_CIR_EXCEEDED_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set when the frame rate is below both the Committed Information Rate and
 * the Peak Information Rate.
 *
 * \details 
 * 0: No event has occured
 * 1: Traffic received without triggering CIR and PIR policing
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_COMMON_SDLB_DLB_STICKY . CIR_PIR_OPEN_STICKY
 */
#define  VTSS_F_ANA_AC_COMMON_SDLB_DLB_STICKY_CIR_PIR_OPEN_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:SDLB
 *
 * Dual leaky buckets
 */


/** 
 * \brief Configuration of DLB policer
 *
 * \details
 * Register: \a ANA_AC:SDLB:DLB_CFG
 *
 * @param gi Replicator: x_ANA_NUM_DLB (??), 0-4095
 */
#define VTSS_ANA_AC_SDLB_DLB_CFG(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x10000,gi,8,0,0)

/** 
 * \brief
 * Enables leaky bucket coupling mode.
 * When operating in non coupled mode, yellow frames are bounded by Excess
 * Information Rate only.
 * When operating in coupled mode, yellow frames are bounded by Committed
 * Information Rate + Excess Information Rate
 * meaning that PIR bucket will also be be leaked with unused CIR rate
 * (when CIR bucket is empty)
 * 
 * This is only useful when color aware (see:COLOR_AWARE_LVL)
 *
 * \details 
 * 0: Uncoupled (PIR will only be leaked with PIR RATE_VAL)
 * 1: Coupled (PIR can be leaked with PIR RATE_VAL and unused CIR RATE_VAL)
 *
 * Field: ::VTSS_ANA_AC_SDLB_DLB_CFG . COUPLING_MODE
 */
#define  VTSS_F_ANA_AC_SDLB_DLB_CFG_COUPLING_MODE  VTSS_BIT(31)

/** 
 * \brief
 * Configuration of leaky bucket color awareness.
 * Frames with DP below or equal to level is treated as GREEN and use CIR
 * bandwidth before PIR bandwidth. Frames with DP above level are treated
 * as YELLOW and use PIR bandwidth.
 *
 * \details 
 * 0: DLB policer is color aware. Incoming frames with DP=0 are green and
 * frames with DP>0 are yellow. 
 * 1: DLB policer is color aware. Incoming frames with DP<=1 are green and
 * frames with DP>1 are yellow. 
 * 2: DLB policer is color aware. Incoming frames with DP<=2 are green and
 * frames with DP>2 are yellow. 
 * 3: DLB policer is color unaware. All incoming frames are green.
 *
 * Field: ::VTSS_ANA_AC_SDLB_DLB_CFG . COLOR_AWARE_LVL
 */
#define  VTSS_F_ANA_AC_SDLB_DLB_CFG_COLOR_AWARE_LVL(x)  VTSS_ENCODE_BITFIELD(x,29,2)
#define  VTSS_M_ANA_AC_SDLB_DLB_CFG_COLOR_AWARE_LVL     VTSS_ENCODE_BITMASK(29,2)
#define  VTSS_X_ANA_AC_SDLB_DLB_CFG_COLOR_AWARE_LVL(x)  VTSS_EXTRACT_BITFIELD(x,29,2)

/** 
 * \brief
 * Configuration of the drop precedence change for green frames being
 * remarked to yellow. The DP level for yellow frames is incremented with
 * CIR_INC_DP_VAL. 
 *
 * \details 
 * 0: DP is not increased.
 * n: DP is increased with n for PIR traffic.
 *
 * Field: ::VTSS_ANA_AC_SDLB_DLB_CFG . CIR_INC_DP_VAL
 */
#define  VTSS_F_ANA_AC_SDLB_DLB_CFG_CIR_INC_DP_VAL(x)  VTSS_ENCODE_BITFIELD(x,27,2)
#define  VTSS_M_ANA_AC_SDLB_DLB_CFG_CIR_INC_DP_VAL     VTSS_ENCODE_BITMASK(27,2)
#define  VTSS_X_ANA_AC_SDLB_DLB_CFG_CIR_INC_DP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,27,2)

/** 
 * \brief
 * Value added to each frame before updating the bucket. Gap value range:
 * -64 to +63 in two's complement format.
 * When GAP_VALUE = 20, a line-rate measurement because each frame occupies
 * 12 bytes for the inter-frame gap and 8 bytes for the preamble on the
 * line. When GAP_VALUE = 0, a data-rate measurement is achived..
 *
 * \details 
 * 0x40: -64
 * 0x41: -63
 *    ...
 * 0x7F: -1
 * 0x00: 0
 * 0x01: 1
 *    ...
 * 0x3F: 63
 *
 * Field: ::VTSS_ANA_AC_SDLB_DLB_CFG . GAP_VAL
 */
#define  VTSS_F_ANA_AC_SDLB_DLB_CFG_GAP_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,7)
#define  VTSS_M_ANA_AC_SDLB_DLB_CFG_GAP_VAL     VTSS_ENCODE_BITMASK(16,7)
#define  VTSS_X_ANA_AC_SDLB_DLB_CFG_GAP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,7)

/** 
 * \brief
 * Configures the leaky bucket scale to enable from 16 kbps up to 10Gbps
 * rates.
 *
 * \details 
 * 0: Up to 10Gbps rates configured with 8196721 bps(= 8192 kbps)
 * granularity
 * 1: Up to 2Gbps rates configured with 1024590 bps (= 1024 kbps)
 * granularity
 * 2: Up to 200Mbps rates configured with 128074 bps (= 128 kbps)
 * granularity
 * 3: Up to 32 Mbps rates configured with 16009 bps (= 16 kbps) granularity
 *
 * Field: ::VTSS_ANA_AC_SDLB_DLB_CFG . TIMESCALE_VAL
 */
#define  VTSS_F_ANA_AC_SDLB_DLB_CFG_TIMESCALE_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_AC_SDLB_DLB_CFG_TIMESCALE_VAL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_AC_SDLB_DLB_CFG_TIMESCALE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Configuration of leaky buckets
 *
 * \details
 * LB_CFG:0 configures CIR and LB_CFG:1 configures PIR
 *
 * Register: \a ANA_AC:SDLB:LB_CFG
 *
 * @param gi Replicator: x_ANA_NUM_DLB (??), 0-4095
 * @param ri Register: LB_CFG (??), 0-1
 */
#define VTSS_ANA_AC_SDLB_LB_CFG(gi,ri)       VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x10000,gi,8,ri,1)

/** 
 * \brief
 * Policer threshold size (a.ka. burst capacity). Unit is 2048 bytes
 *
 * \details 
 * 0: Threshold = 0 bytes (no burst allowed)
 * 1: Threshold = 2048 bytes
 * n: Threshold = n x 2048 bytes
 *
 * Field: ::VTSS_ANA_AC_SDLB_LB_CFG . THRES_VAL
 */
#define  VTSS_F_ANA_AC_SDLB_LB_CFG_THRES_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,7)
#define  VTSS_M_ANA_AC_SDLB_LB_CFG_THRES_VAL     VTSS_ENCODE_BITMASK(16,7)
#define  VTSS_X_ANA_AC_SDLB_LB_CFG_THRES_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,7)

/** 
 * \brief
 * Specify rate in steps of configured granularity
 *
 * \details 
 * 0: Disable leak. For THRES_VAL = 0 bucket is always closed. For
 * THRES_VAL > 0, the configured burst size is available.
 * 1: 1 * granularity
 * 2: 2 * granularity
 * ...
 * n-1: (n-1)*granularity
 * n: Disable leaky bucket (always open)
 *
 * Field: ::VTSS_ANA_AC_SDLB_LB_CFG . RATE_VAL
 */
#define  VTSS_F_ANA_AC_SDLB_LB_CFG_RATE_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,11)
#define  VTSS_M_ANA_AC_SDLB_LB_CFG_RATE_VAL     VTSS_ENCODE_BITMASK(0,11)
#define  VTSS_X_ANA_AC_SDLB_LB_CFG_RATE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,11)

/**
 * Register Group: \a ANA_AC:COMMON_TDLB
 *
 * Common DLB configuration and status registers
 */


/** 
 * \brief Configuration of common dual leaky bucket handling
 *
 * \details
 * Register: \a ANA_AC:COMMON_TDLB:DLB_CTRL
 */
#define VTSS_ANA_AC_COMMON_TDLB_DLB_CTRL     VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbf7)

/** 
 * \brief
 * Specifies the number of clock cycles for a tick.
 *
 * \details 
 * 0: 1 clk
 * ...
 * n: 1+n clk
 *
 * Field: ::VTSS_ANA_AC_COMMON_TDLB_DLB_CTRL . BASE_TICK_CNT
 */
#define  VTSS_F_ANA_AC_COMMON_TDLB_DLB_CTRL_BASE_TICK_CNT(x)  VTSS_ENCODE_BITFIELD(x,4,13)
#define  VTSS_M_ANA_AC_COMMON_TDLB_DLB_CTRL_BASE_TICK_CNT     VTSS_ENCODE_BITMASK(4,13)
#define  VTSS_X_ANA_AC_COMMON_TDLB_DLB_CTRL_BASE_TICK_CNT(x)  VTSS_EXTRACT_BITFIELD(x,4,13)

/** 
 * \brief
 * If set, the policers are initialized and all buckets are open. The bit
 * must be cleared for normal operation to resume.
 *
 * \details 
 * 0: Normal operation
 * 1: Initialization
 *
 * Field: ::VTSS_ANA_AC_COMMON_TDLB_DLB_CTRL . DLB_INIT_SHOT
 */
#define  VTSS_F_ANA_AC_COMMON_TDLB_DLB_CTRL_DLB_INIT_SHOT  VTSS_BIT(2)

/** 
 * \brief
 * Enables leaking from the dual leaky buckets.
 *
 * \details 
 * 0: Disable bucket leaking
 * 1: Enable bucket leaking
 *
 * Field: ::VTSS_ANA_AC_COMMON_TDLB_DLB_CTRL . LEAK_ENA
 */
#define  VTSS_F_ANA_AC_COMMON_TDLB_DLB_CTRL_LEAK_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enables adding of frame bytes to the dual leaky buckets.
 *
 * \details 
 * 0: Disable bucket addition
 * 1: Enable bucket addition
 *
 * Field: ::VTSS_ANA_AC_COMMON_TDLB_DLB_CTRL . DLB_ADD_ENA
 */
#define  VTSS_F_ANA_AC_COMMON_TDLB_DLB_CTRL_DLB_ADD_ENA  VTSS_BIT(0)


/** 
 * \brief DLB policer diagnostic
 *
 * \details
 * Register: \a ANA_AC:COMMON_TDLB:DLB_STICKY
 */
#define VTSS_ANA_AC_COMMON_TDLB_DLB_STICKY   VTSS_IOREG(VTSS_TO_ANA_AC,0x1bbf9)

/** 
 * \brief
 * Set when a DLB scan could not start because a scan is already ongoing -
 * BASE_TICK_CNT must be increased.
 *
 * \details 
 * 0: No event has occured
 * 1: Leak scan could not start at time
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_COMMON_TDLB_DLB_STICKY . LEAK_START_DELAYED_STICKY
 */
#define  VTSS_F_ANA_AC_COMMON_TDLB_DLB_STICKY_LEAK_START_DELAYED_STICKY  VTSS_BIT(29)

/** 
 * \brief
 * Set when the frame rate is exceeding the Peak Information Rate.
 *
 * \details 
 * 0: No event has occured
 * 1: PIR exceeded
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_COMMON_TDLB_DLB_STICKY . PIR_EXCEEDED_STICKY
 */
#define  VTSS_F_ANA_AC_COMMON_TDLB_DLB_STICKY_PIR_EXCEEDED_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set when the frame rate is exceeding the Committed Information Rate.
 *
 * \details 
 * 0: No event has occured
 * 1: CIR exceeded
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_COMMON_TDLB_DLB_STICKY . CIR_EXCEEDED_STICKY
 */
#define  VTSS_F_ANA_AC_COMMON_TDLB_DLB_STICKY_CIR_EXCEEDED_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set when the frame rate is below both the Committed Information Rate and
 * the Peak Information Rate.
 *
 * \details 
 * 0: No event has occured
 * 1: Traffic received without triggering CIR and PIR policing
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_AC_COMMON_TDLB_DLB_STICKY . CIR_PIR_OPEN_STICKY
 */
#define  VTSS_F_ANA_AC_COMMON_TDLB_DLB_STICKY_CIR_PIR_OPEN_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ANA_AC:TDLB
 *
 * Dual leaky buckets
 */


/** 
 * \brief Configuration of DLB policer
 *
 * \details
 * Register: \a ANA_AC:TDLB:DLB_CFG
 *
 * @param gi Replicator: x_ANA_NUM_DLB (??), 0-511
 */
#define VTSS_ANA_AC_TDLB_DLB_CFG(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1a000,gi,8,0,0)

/** 
 * \brief
 * Enables leaky bucket coupling mode.
 * When operating in non coupled mode, yellow frames are bounded by Excess
 * Information Rate only.
 * When operating in coupled mode, yellow frames are bounded by Committed
 * Information Rate + Excess Information Rate
 * meaning that PIR bucket will also be be leaked with unused CIR rate
 * (when CIR bucket is empty)
 * 
 * This is only useful when color aware (see:COLOR_AWARE_LVL)
 *
 * \details 
 * 0: Uncoupled (PIR will only be leaked with PIR RATE_VAL)
 * 1: Coupled (PIR can be leaked with PIR RATE_VAL and unused CIR RATE_VAL)
 *
 * Field: ::VTSS_ANA_AC_TDLB_DLB_CFG . COUPLING_MODE
 */
#define  VTSS_F_ANA_AC_TDLB_DLB_CFG_COUPLING_MODE  VTSS_BIT(31)

/** 
 * \brief
 * Configuration of leaky bucket color awareness.
 * Frames with DP below or equal to level is treated as GREEN and use CIR
 * bandwidth before PIR bandwidth. Frames with DP above level are treated
 * as YELLOW and use PIR bandwidth.
 *
 * \details 
 * 0: DLB policer is color aware. Incoming frames with DP=0 are green and
 * frames with DP>0 are yellow. 
 * 1: DLB policer is color aware. Incoming frames with DP<=1 are green and
 * frames with DP>1 are yellow. 
 * 2: DLB policer is color aware. Incoming frames with DP<=2 are green and
 * frames with DP>2 are yellow. 
 * 3: DLB policer is color unaware. All incoming frames are green.
 *
 * Field: ::VTSS_ANA_AC_TDLB_DLB_CFG . COLOR_AWARE_LVL
 */
#define  VTSS_F_ANA_AC_TDLB_DLB_CFG_COLOR_AWARE_LVL(x)  VTSS_ENCODE_BITFIELD(x,29,2)
#define  VTSS_M_ANA_AC_TDLB_DLB_CFG_COLOR_AWARE_LVL     VTSS_ENCODE_BITMASK(29,2)
#define  VTSS_X_ANA_AC_TDLB_DLB_CFG_COLOR_AWARE_LVL(x)  VTSS_EXTRACT_BITFIELD(x,29,2)

/** 
 * \brief
 * Configuration of the drop precedence change for green frames being
 * remarked to yellow. The DP level for yellow frames is incremented with
 * CIR_INC_DP_VAL. 
 *
 * \details 
 * 0: DP is not increased.
 * n: DP is increased with n for PIR traffic.
 *
 * Field: ::VTSS_ANA_AC_TDLB_DLB_CFG . CIR_INC_DP_VAL
 */
#define  VTSS_F_ANA_AC_TDLB_DLB_CFG_CIR_INC_DP_VAL(x)  VTSS_ENCODE_BITFIELD(x,27,2)
#define  VTSS_M_ANA_AC_TDLB_DLB_CFG_CIR_INC_DP_VAL     VTSS_ENCODE_BITMASK(27,2)
#define  VTSS_X_ANA_AC_TDLB_DLB_CFG_CIR_INC_DP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,27,2)

/** 
 * \brief
 * Value added to each frame before updating the bucket. Gap value range:
 * -64 to +63 in two's complement format.
 * When GAP_VALUE = 20, a line-rate measurement because each frame occupies
 * 12 bytes for the inter-frame gap and 8 bytes for the preamble on the
 * line. When GAP_VALUE = 0, a data-rate measurement is achived..
 *
 * \details 
 * 0x40: -64
 * 0x41: -63
 *    ...
 * 0x7F: -1
 * 0x00: 0
 * 0x01: 1
 *    ...
 * 0x3F: 63
 *
 * Field: ::VTSS_ANA_AC_TDLB_DLB_CFG . GAP_VAL
 */
#define  VTSS_F_ANA_AC_TDLB_DLB_CFG_GAP_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,7)
#define  VTSS_M_ANA_AC_TDLB_DLB_CFG_GAP_VAL     VTSS_ENCODE_BITMASK(16,7)
#define  VTSS_X_ANA_AC_TDLB_DLB_CFG_GAP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,7)

/** 
 * \brief
 * Configures the leaky bucket scale to enable from 16 kbps up to 10Gbps
 * rates.
 *
 * \details 
 * 0: Up to 10Gbps rates configured with 8196721 bps(= 8192 kbps)
 * granularity
 * 1: Up to 2Gbps rates configured with 1024590 bps (= 1024 kbps)
 * granularity
 * 2: Up to 200Mbps rates configured with 128074 bps (= 128 kbps)
 * granularity
 * 3: Up to 32 Mbps rates configured with 16009 bps (= 16 kbps) granularity
 *
 * Field: ::VTSS_ANA_AC_TDLB_DLB_CFG . TIMESCALE_VAL
 */
#define  VTSS_F_ANA_AC_TDLB_DLB_CFG_TIMESCALE_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_AC_TDLB_DLB_CFG_TIMESCALE_VAL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_AC_TDLB_DLB_CFG_TIMESCALE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Configuration of leaky buckets
 *
 * \details
 * LB_CFG:0 configures CIR and LB_CFG:1 configures PIR
 *
 * Register: \a ANA_AC:TDLB:LB_CFG
 *
 * @param gi Replicator: x_ANA_NUM_DLB (??), 0-511
 * @param ri Register: LB_CFG (??), 0-1
 */
#define VTSS_ANA_AC_TDLB_LB_CFG(gi,ri)       VTSS_IOREG_IX(VTSS_TO_ANA_AC,0x1a000,gi,8,ri,1)

/** 
 * \brief
 * Policer threshold size (a.ka. burst capacity). Unit is 2048 bytes
 *
 * \details 
 * 0: Threshold = 0 bytes (no burst allowed)
 * 1: Threshold = 2048 bytes
 * n: Threshold = n x 2048 bytes
 *
 * Field: ::VTSS_ANA_AC_TDLB_LB_CFG . THRES_VAL
 */
#define  VTSS_F_ANA_AC_TDLB_LB_CFG_THRES_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,7)
#define  VTSS_M_ANA_AC_TDLB_LB_CFG_THRES_VAL     VTSS_ENCODE_BITMASK(16,7)
#define  VTSS_X_ANA_AC_TDLB_LB_CFG_THRES_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,7)

/** 
 * \brief
 * Specify rate in steps of configured granularity
 *
 * \details 
 * 0: Disable leak. For THRES_VAL = 0 bucket is always closed. For
 * THRES_VAL > 0, the configured burst size is available.
 * 1: 1 * granularity
 * 2: 2 * granularity
 * ...
 * n-1: (n-1)*granularity
 * n: Disable leaky bucket (always open)
 *
 * Field: ::VTSS_ANA_AC_TDLB_LB_CFG . RATE_VAL
 */
#define  VTSS_F_ANA_AC_TDLB_LB_CFG_RATE_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,11)
#define  VTSS_M_ANA_AC_TDLB_LB_CFG_RATE_VAL     VTSS_ENCODE_BITMASK(0,11)
#define  VTSS_X_ANA_AC_TDLB_LB_CFG_RATE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,11)


#endif /* _VTSS_JAGUAR_REGS_ANA_AC_H_ */
