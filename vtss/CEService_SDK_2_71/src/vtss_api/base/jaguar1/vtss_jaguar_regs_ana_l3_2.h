#ifndef _VTSS_JAGUAR_REGS_ANA_L3_2_H_
#define _VTSS_JAGUAR_REGS_ANA_L3_2_H_

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
 * Target: \a ANA_L3_2
 *
 * Layer 3 sub block of the Analyzer
 *
 ***********************************************************************/

/**
 * Register Group: \a ANA_L3_2:BMSTP
 *
 * BMSTP table configuration
 */


/** 
 * \brief MSTP Learning Control
 *
 * \details
 * Configuration of learning state per MSTP in	B-Domain
 *
 * Register: \a ANA_L3_2:BMSTP:BMSTP_LRN_CFG
 *
 * @param gi Replicator: x_ANA_L3_NUM_MSTP (??), 0-65
 */
#define VTSS_ANA_L3_2_BMSTP_BMSTP_LRN_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4e00,gi,2,0,0)


/** 
 * \brief MSTP Forwarding Control in B-Domain
 *
 * \details
 * Configuration of forwarding state per MSTP in B-Domain
 *
 * Register: \a ANA_L3_2:BMSTP:BMSTP_FWD_CFG
 *
 * @param gi Replicator: x_ANA_L3_NUM_MSTP (??), 0-65
 */
#define VTSS_ANA_L3_2_BMSTP_BMSTP_FWD_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4e00,gi,2,0,1)

/**
 * Register Group: \a ANA_L3_2:INTEGRITY_IDENT
 *
 * Memory Integrity error information
 */


/** 
 * \brief RAM integrity errors
 *
 * \details
 * Register with 1 bit per RAM to store integrity errors
 *
 * Register: \a ANA_L3_2:INTEGRITY_IDENT:INTEGRITY_IDENT
 */
#define VTSS_ANA_L3_2_INTEGRITY_IDENT_INTEGRITY_IDENT  VTSS_IOREG(VTSS_TO_ANA_L3,0x4e84)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_L3_2_INTEGRITY_IDENT_INTEGRITY_IDENT . MEM_REMAP_RAM_ERR
 */
#define  VTSS_F_ANA_L3_2_INTEGRITY_IDENT_INTEGRITY_IDENT_MEM_REMAP_RAM_ERR  VTSS_BIT(2)

/**
 * Register Group: \a ANA_L3_2:L3_STICKY_MASK
 *
 * Sticky diagnostic global port counter event configuration
 */


/** 
 * \brief Enable per concurrent counter for each sticky bit
 *
 * \details
 * Each ANA_L3 sticky bit can be counted in each of the concurrent
 * counters.
 *
 * Register: \a ANA_L3_2:L3_STICKY_MASK:RLEG_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4e85,gi,4,0,0)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_UC_IP_OPT_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_UC_IP_OPT_REDIR_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_UC_HIT_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_UC_HIT_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_UC_HDR_ERR_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_UC_HDR_ERR_REDIR_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_MC_TTL_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_MC_TTL_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_MC_IP_OPT_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_MC_IP_OPT_REDIR_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_MC_HIT_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_MC_HIT_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_MC_HDR_ERR_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_MC_HDR_ERR_REDIR_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_UC_NONIP_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_UC_NONIP_REDIR_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Enable per concurrent counter for each sticky bit
 *
 * \details
 * Each ANA_L3 sticky bit can be counted in each of the concurrent
 * counters.
 *
 * Register: \a ANA_L3_2:L3_STICKY_MASK:ROUT_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4e85,gi,4,0,1)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . NO_MC_VMID_AVAIL_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_NO_MC_VMID_AVAIL_STICKY_MASK  VTSS_BIT(14)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . ENTRY_NOT_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_ENTRY_NOT_FOUND_STICKY_MASK  VTSS_BIT(13)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . MC_LOOPED_CP_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_MC_LOOPED_CP_STICKY_MASK  VTSS_BIT(12)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . UC_ZERO_DMAC_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_UC_ZERO_DMAC_FOUND_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . UC_TTL_FILTERING_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_UC_TTL_FILTERING_STICKY_MASK  VTSS_BIT(10)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . UC_ICMP_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_UC_ICMP_REDIR_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . UC_ENTRY_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_UC_ENTRY_FOUND_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . NO_MC_FWD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_NO_MC_FWD_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . L3_MC_FWD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_L3_MC_FWD_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . MC_RPF_FILTER_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_MC_RPF_FILTER_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . L2_MC_FWD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_L2_MC_FWD_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . MC_ENTRY_NOT_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_MC_ENTRY_NOT_FOUND_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . LPM_IP6UC_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_LPM_IP6UC_FOUND_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . LPM_IP4UC_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_LPM_IP4UC_FOUND_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK . LPM_IP4MC_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_ROUT_STICKY_MASK_LPM_IP4MC_FOUND_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Enable per concurrent counter for each sticky bit
 *
 * \details
 * Each ANA_L3 sticky bit can be counted in each of the concurrent
 * counters.
 *
 * Register: \a ANA_L3_2:L3_STICKY_MASK:SECUR_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4e85,gi,4,0,2)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP6_DIP_MATCH_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP6_DIP_MATCH_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP4_SIP_MATCH_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP4_SIP_MATCH_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_SIP_FAIL_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_SIP_FAIL_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP6_SIP_MATCH_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP6_SIP_MATCH_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP6_LPM_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP6_LPM_FOUND_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP4_DIP_MATCH_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP4_DIP_MATCH_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP4_LPM_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP4_LPM_FOUND_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_DIP_FAIL_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_DIP_FAIL_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Enable per concurrent counter for each sticky bit
 *
 * \details
 * Each ANA_L3 sticky bit can be counted in each of the concurrent
 * counters.
 *
 * Register: \a ANA_L3_2:L3_STICKY_MASK:VLAN_MSTP_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4e85,gi,4,0,3)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . BMSTP_FWD_ALLOWED_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_BMSTP_FWD_ALLOWED_STICKY_MASK  VTSS_BIT(14)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . BMSTP_DISCARD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_BMSTP_DISCARD_STICKY_MASK  VTSS_BIT(13)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . BVLAN_LRN_DENY_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_BVLAN_LRN_DENY_STICKY_MASK  VTSS_BIT(12)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . BMSTP_LRN_DENY_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_BMSTP_LRN_DENY_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . BMSTP_LRN_ALLOWED_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_BMSTP_LRN_ALLOWED_STICKY_MASK  VTSS_BIT(10)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . BVLAN_LOOKUP_INVLD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_BVLAN_LOOKUP_INVLD_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . BVLAN_IGR_FILTER_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_BVLAN_IGR_FILTER_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . MSTP_FWD_ALLOWED_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_MSTP_FWD_ALLOWED_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . MSTP_DISCARD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_MSTP_DISCARD_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . VLAN_LRN_DENY_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_VLAN_LRN_DENY_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . MSTP_LRN_DENY_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_MSTP_LRN_DENY_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . MSTP_LRN_ALLOWED_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_MSTP_LRN_ALLOWED_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . VLAN_LOOKUP_INVLD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_VLAN_LOOKUP_INVLD_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Enable/Disable counting of sticky event per concurrent counter.
 *
 * \details 
 * '0': Respective sticky event is not counted
 * '1': Respective sticky event is counted
 *
 * Field: ::VTSS_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . VLAN_IGR_FILTER_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_2_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_VLAN_IGR_FILTER_STICKY_MASK  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L3_2:COMMON
 *
 * Common configurations for all ports
 */


/** 
 * \brief BVLAN Filter Control
 *
 * \details
 * Configuration of BVLAN ingress filtering per port
 *
 * Register: \a ANA_L3_2:COMMON:BVLAN_FILTER_CTRL
 */
#define VTSS_ANA_L3_2_COMMON_BVLAN_FILTER_CTRL  VTSS_IOREG(VTSS_TO_ANA_L3,0x4e95)


/** 
 * \brief VLAN Control
 *
 * \details
 * Basic VLAN related configuration
 *
 * Register: \a ANA_L3_2:COMMON:VLAN_CTRL
 */
#define VTSS_ANA_L3_2_COMMON_VLAN_CTRL       VTSS_IOREG(VTSS_TO_ANA_L3,0x4e97)

/** 
 * \brief
 * Dual domain enable. Determines whether B-component related lookup or
 * routing related lookup is enabled.
 *
 * \details 
 * '0': L3 lookup is enabled
 * '1': Dual domain lookup is enabled
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_VLAN_CTRL . DUAL_DOMAIN_ENA
 */
#define  VTSS_F_ANA_L3_2_COMMON_VLAN_CTRL_DUAL_DOMAIN_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable/disable VLAN lookup. This field must be enabled to allow VLAN and
 * MSTP filtering. For VLAN unaware operation, this field can be disabled.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_VLAN_CTRL . VLAN_ENA
 */
#define  VTSS_F_ANA_L3_2_COMMON_VLAN_CTRL_VLAN_ENA  VTSS_BIT(0)


/** 
 * \brief Unicast routing control configuration
 *
 * \details
 * Enable of L3 unicast traffic per port
 *
 * Register: \a ANA_L3_2:COMMON:L3_UC_ENA
 */
#define VTSS_ANA_L3_2_COMMON_L3_UC_ENA       VTSS_IOREG(VTSS_TO_ANA_L3,0x4e98)


/** 
 * \brief Common table control
 *
 * \details
 * Register contains bits that allow to force initialization of various RAM
 * tables
 *
 * Register: \a ANA_L3_2:COMMON:TABLE_CTRL
 */
#define VTSS_ANA_L3_2_COMMON_TABLE_CTRL      VTSS_IOREG(VTSS_TO_ANA_L3,0x4e99)

/** 
 * \brief
 * Initialize the IP multicast table.
 *
 * \details 
 * 0: Idle (no clear)
 * 1: Initialize table
 * The bit is cleared by HW when Init is complete
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_TABLE_CTRL . INIT_IPMC_SHOT
 */
#define  VTSS_F_ANA_L3_2_COMMON_TABLE_CTRL_INIT_IPMC_SHOT  VTSS_BIT(3)

/** 
 * \brief
 * Initialize the ARP table.
 *
 * \details 
 * 0: Idle (no clear)
 * 1: Initialize table
 * The bit is cleared by HW when Init is complete
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_TABLE_CTRL . INIT_ARP_SHOT
 */
#define  VTSS_F_ANA_L3_2_COMMON_TABLE_CTRL_INIT_ARP_SHOT  VTSS_BIT(2)

/** 
 * \brief
 * Initialize the VMID table.
 *
 * \details 
 * 0: Idle (no clear)
 * 1: Initialize table
 * The bit is cleared by HW when Init is complete
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_TABLE_CTRL . INIT_VMID_SHOT
 */
#define  VTSS_F_ANA_L3_2_COMMON_TABLE_CTRL_INIT_VMID_SHOT  VTSS_BIT(1)

/** 
 * \brief
 * Initialize the VLAN table.
 * 
 * The VLAN table entries are set to their default values except for VLAN
 * addresses 0,1 and 4095:
 * For these addresses VLAN_PORT_MASK is set to '1...1'.
 *
 * \details 
 * 0: Idle (no clear)
 * 1: Initialize table
 * The bit is cleared by HW when Init is complete
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_TABLE_CTRL . INIT_VLAN_SHOT
 */
#define  VTSS_F_ANA_L3_2_COMMON_TABLE_CTRL_INIT_VLAN_SHOT  VTSS_BIT(0)


/** 
 * \brief Multicast routing control Configuration
 *
 * \details
 * IP multicast traffic enable per port
 *
 * Register: \a ANA_L3_2:COMMON:L3_MC_ENA
 */
#define VTSS_ANA_L3_2_COMMON_L3_MC_ENA       VTSS_IOREG(VTSS_TO_ANA_L3,0x4e9a)


/** 
 * \brief Forwarding Control
 *
 * \details
 * Configuration of Forwarding state per port
 *
 * Register: \a ANA_L3_2:COMMON:PORT_FWD_CTRL
 */
#define VTSS_ANA_L3_2_COMMON_PORT_FWD_CTRL   VTSS_IOREG(VTSS_TO_ANA_L3,0x4e9b)


/** 
 * \brief Learning Control
 *
 * \details
 * Configuration of learning state per port
 *
 * Register: \a ANA_L3_2:COMMON:PORT_LRN_CTRL
 */
#define VTSS_ANA_L3_2_COMMON_PORT_LRN_CTRL   VTSS_IOREG(VTSS_TO_ANA_L3,0x4e9c)


/** 
 * \brief VLAN Filter Control
 *
 * \details
 * Configuration of VLAN ingress filtering per port
 *
 * Register: \a ANA_L3_2:COMMON:VLAN_FILTER_CTRL
 */
#define VTSS_ANA_L3_2_COMMON_VLAN_FILTER_CTRL  VTSS_IOREG(VTSS_TO_ANA_L3,0x4e9d)


/** 
 * \brief Switch Mode
 *
 * \details
 * Configuration of routing checks
 *
 * Register: \a ANA_L3_2:COMMON:ROUTING_CFG
 */
#define VTSS_ANA_L3_2_COMMON_ROUTING_CFG     VTSS_IOREG(VTSS_TO_ANA_L3,0x4e9e)

/** 
 * \brief
 * Enable a switched copy (ERLEG = IRLEG) per IPMC frame regardless of
 * whether the IVMID is enabled in the respective EVMID mask or not.
 *
 * \details 
 * '0': Forward based on EVMID mask
 * '1': Enable switched copy
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_ROUTING_CFG . ALWAYS_SWITCHED_COPY_ENA
 */
#define  VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_ALWAYS_SWITCHED_COPY_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Enable for range violation checks of the DIP. If enabled (1) and the
 * frame's DIP falls into the given range then the frame is not forwarded
 * to a front port but redirected to CPU.
 * 
 * Bit	   prohibited DIP range
 * ---------------------------------------------------------
 * 0	      0.0.0.0	-	0.255.255.255
 * 1	  127.0.0.0   -   127.255.255.255
 * 2	  240.0.0.0   -   255.255.255.254
 * 3*	  224.0.0.0   -   239.255.255.255
 * 
 * *) This range violation check is only applied to frames with a unicast
 * Router Leg hit.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Frame is forwarded to front port
 * '1': Frame is redirected to CPU if SIP falls into given range
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_ROUTING_CFG . DIP_ADDR_VIOLATION_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_DIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_ANA_L3_2_COMMON_ROUTING_CFG_DIP_ADDR_VIOLATION_REDIR_ENA     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_ANA_L3_2_COMMON_ROUTING_CFG_DIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Enable for range violation checks of the SIP. If enabled (1) and the
 * frame's SIP falls into the given range then the frame is not forwarded
 * to a front port but redirected to CPU.
 * 
 * Bit	   prohibited SIP range
 * ---------------------------------------------------------
 * 0	      0.0.0.0	-	0.255.255.255
 * 1	  127.0.0.0   -   127.255.255.255
 * 2	  224.0.0.0   -   255.255.255.255
 * 
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * '0': Frame is forwarded to front port
 * '1': Frame is redirected to CPU if SIP falls into given range
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_ROUTING_CFG . SIP_ADDR_VIOLATION_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_SIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_ANA_L3_2_COMMON_ROUTING_CFG_SIP_ADDR_VIOLATION_REDIR_ENA     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_ANA_L3_2_COMMON_ROUTING_CFG_SIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Control CPU redirection of IPv4 error frames.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Disable redirection (errored frames are discarded)
 * 1: Enable redirection to CPU queue
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_ROUTING_CFG . CPU_RLEG_IP_HDR_FAIL_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_CPU_RLEG_IP_HDR_FAIL_REDIR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable / Disable redirection of IPv6 frames with HOP by HOP options
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_ROUTING_CFG . CPU_IP6_HOPBYHOP_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_CPU_IP6_HOPBYHOP_REDIR_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable / Disable redirection of IPv4 frames with IP4 options
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_ROUTING_CFG . CPU_IP4_OPTIONS_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_CPU_IP4_OPTIONS_REDIR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable/disable the redirection to CPU of IPv6 UC frames with a hop-limit
 * less than 2.
 * If enabled and hop limit < 2 then frame is redirected to CPU queue
 * specified by 'CPU_IP_TTL_FAIL_QU', else the frame is discarded.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_ROUTING_CFG . IP6_HC_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_IP6_HC_REDIR_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable/disable the redirection to CPU of IPv4 UC frames with a TTL less
 * than 2.
 * If enabled and TTL < 2 then frame is redirected to CPU queue specified
 * by 'CPU_IP_TTL_FAIL_QU', else the frame is dicarded.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_ROUTING_CFG . IP4_TTL_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_2_COMMON_ROUTING_CFG_IP4_TTL_REDIR_ENA  VTSS_BIT(0)


/** 
 * \brief VLAN isolated port mask
 *
 * \details
 * Configuration of isolated port mask. See description of
 * VLAN::VLAN_CFG.VLAN_PRIVATE_ENA
 *
 * Register: \a ANA_L3_2:COMMON:VLAN_ISOLATED_CFG
 */
#define VTSS_ANA_L3_2_COMMON_VLAN_ISOLATED_CFG  VTSS_IOREG(VTSS_TO_ANA_L3,0x4e9f)


/** 
 * \brief Router Leg base MAC address
 *
 * \details
 * Configuration of Router Leg base MAC address
 *
 * Register: \a ANA_L3_2:COMMON:RLEG_CFG_0
 */
#define VTSS_ANA_L3_2_COMMON_RLEG_CFG_0      VTSS_IOREG(VTSS_TO_ANA_L3,0x4ea0)

/** 
 * \brief
 * Router Leg MAC address (Least significant bits).
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * Bit 0: MAC bit 0
 * ...
 * Bit 23: MAC bit 23
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_RLEG_CFG_0 . RLEG_MAC_LSB
 */
#define  VTSS_F_ANA_L3_2_COMMON_RLEG_CFG_0_RLEG_MAC_LSB(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_ANA_L3_2_COMMON_RLEG_CFG_0_RLEG_MAC_LSB     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_ANA_L3_2_COMMON_RLEG_CFG_0_RLEG_MAC_LSB(x)  VTSS_EXTRACT_BITFIELD(x,8,24)


/** 
 * \brief Router Leg base MAC address
 *
 * \details
 * Configuration of Common Router Leg base MAC address
 *
 * Register: \a ANA_L3_2:COMMON:RLEG_CFG_1
 */
#define VTSS_ANA_L3_2_COMMON_RLEG_CFG_1      VTSS_IOREG(VTSS_TO_ANA_L3,0x4ea1)

/** 
 * \brief
 * Specifies how Router Leg MAC addresses are specified.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: RLEG_MAC := RLEG_MAC_MSB(23:0) & ((RLEG_MAC_LSB(23:0) + VMID(7:0))
 * mod 2**24)
 * 1: RLEG_MAC := RLEG_MAC_MSB(23:0) & ((RLEG_MAC_LSB(23:0) + VID(11:0))
 * mod 2**24)
 * Others: RLEG_MAC := RLEG_MAC_MSB(23:0) & RLEG_MAC_LSB(23:0)	# common
 * addr for all VLANs
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_RLEG_CFG_1 . RLEG_MAC_TYPE_SEL
 */
#define  VTSS_F_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * Router Leg base MAC address (Most significant bits).
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * Bit 0: MAC bit 24
 * ...
 * Bit 23: MAC bit 47
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_RLEG_CFG_1 . RLEG_MAC_MSB
 */
#define  VTSS_F_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_MSB     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_ANA_L3_2_COMMON_RLEG_CFG_1_RLEG_MAC_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief VLAN Community port mask
 *
 * \details
 * Configuration of Community port mask. See description of
 * VLAN::VLAN_CFG.VLAN_PRIVATE_ENA
 *
 * Register: \a ANA_L3_2:COMMON:VLAN_COMMUNITY_CFG
 */
#define VTSS_ANA_L3_2_COMMON_VLAN_COMMUNITY_CFG  VTSS_IOREG(VTSS_TO_ANA_L3,0x4ea2)


/** 
 * \brief CPU Queue
 *
 * \details
 * Configuration of CPU queues relevant for routing
 *
 * Register: \a ANA_L3_2:COMMON:CPU_QU_CFG
 */
#define VTSS_ANA_L3_2_COMMON_CPU_QU_CFG      VTSS_IOREG(VTSS_TO_ANA_L3,0x4ea3)

/** 
 * \brief
 * CPU queue number for non-IP unicast frames matching an Ingress Router
 * Leg .
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_CPU_QU_CFG . CPU_RLEG_QU
 */
#define  VTSS_F_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_RLEG_QU(x)  VTSS_ENCODE_BITFIELD(x,28,3)
#define  VTSS_M_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_RLEG_QU     VTSS_ENCODE_BITMASK(28,3)
#define  VTSS_X_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_RLEG_QU(x)  VTSS_EXTRACT_BITFIELD(x,28,3)

/** 
 * \brief
 * CPU queue number for IPv4 frames and IPv6 frames with options. For IPv4
 * frames this only applies to unicast frames matching the (VID, DMAC) pair
 * for an Ingress Router Leg and IPv4 multicast frames matching the VID for
 * an Ingress Router Leg enabled for multicast and a DMAC in the range
 * 01-00-5E-00-00-00 to 01-00-5E-7F-FF-FF. For IPv6 frames this only
 * applies to unicast frames matching the (VID, DMAC) pair for an Ingress
 * Router Leg.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_CPU_QU_CFG . CPU_RLEG_IP_OPT_QU
 */
#define  VTSS_F_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_RLEG_IP_OPT_QU(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_RLEG_IP_OPT_QU     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_RLEG_IP_OPT_QU(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * CPU queue number for IPv4 frames with IP header errors. This only
 * applies to IPv4 unicast frames matching the (VID, DMAC) pair for an
 * Ingress Router Leg and IPv4 multicast frames matching the VID for an
 * Ingress Router Leg enabled for multicast and a DMAC in the range
 * 01-00-5E-00-00-00 to 01-00-5E-7F-FF-FF.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_CPU_QU_CFG . CPU_RLEG_IP_HDR_FAIL_QU
 */
#define  VTSS_F_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_RLEG_IP_HDR_FAIL_QU(x)  VTSS_ENCODE_BITFIELD(x,20,3)
#define  VTSS_M_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_RLEG_IP_HDR_FAIL_QU     VTSS_ENCODE_BITMASK(20,3)
#define  VTSS_X_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_RLEG_IP_HDR_FAIL_QU(x)  VTSS_EXTRACT_BITFIELD(x,20,3)

/** 
 * \brief
 * CPU queue for IPv4 unicast packets destined for own address. These are
 * identified by a valid ARP entry with DMAC zero.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_CPU_QU_CFG . CPU_UC_QU
 */
#define  VTSS_F_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_UC_QU(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_UC_QU     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_UC_QU(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * CPU queue for failed IPv4 multicast lookup or RPF check.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_CPU_QU_CFG . CPU_MC_FAIL_QU
 */
#define  VTSS_F_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_MC_FAIL_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_MC_FAIL_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_MC_FAIL_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * CPU queue for failed IPv4/IPv6 unicast lookup, invalid ARP entry or
 * failed ICMP redirect check.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_CPU_QU_CFG . CPU_UC_FAIL_QU
 */
#define  VTSS_F_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_UC_FAIL_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_UC_FAIL_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_UC_FAIL_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * CPU queue for IPv4 unicast frames with a TTL less than 2 with successful
 * ARP lookup giving a non-zero DMAC.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_CPU_QU_CFG . CPU_IP_TTL_FAIL_QU
 */
#define  VTSS_F_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_IP_TTL_FAIL_QU(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_IP_TTL_FAIL_QU     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_L3_2_COMMON_CPU_QU_CFG_CPU_IP_TTL_FAIL_QU(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Virtual Router Leg Protocol Configuration
 *
 * \details
 * Configuration of VRRP base address
 *
 * Register: \a ANA_L3_2:COMMON:VRRP_CFG_0
 */
#define VTSS_ANA_L3_2_COMMON_VRRP_CFG_0      VTSS_IOREG(VTSS_TO_ANA_L3,0x4ea4)

/** 
 * \brief
 * Virtual Router Redundancy Protocol MAC address.  Mid part of VRRP MAC
 * Address.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * VRID MAC address Bit 31 downto 8.
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_VRRP_CFG_0 . VRRP_BASE_MAC_MID
 */
#define  VTSS_F_ANA_L3_2_COMMON_VRRP_CFG_0_VRRP_BASE_MAC_MID(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_ANA_L3_2_COMMON_VRRP_CFG_0_VRRP_BASE_MAC_MID     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_ANA_L3_2_COMMON_VRRP_CFG_0_VRRP_BASE_MAC_MID(x)  VTSS_EXTRACT_BITFIELD(x,8,24)


/** 
 * \brief Source based SMAC compare enable
 *
 * \details
 * Bit per port including VD and CPU that enables a comparison between a
 * configurable MAC and the frame's SMAC
 *
 * Register: \a ANA_L3_2:COMMON:SIP_SECURE_ENA
 */
#define VTSS_ANA_L3_2_COMMON_SIP_SECURE_ENA  VTSS_IOREG(VTSS_TO_ANA_L3,0x4ea5)


/** 
 * \brief Destination based DMAC compare enable
 *
 * \details
 * Bit per port including VD and CPU that enables a comparison between a
 * configurable MAC and the frame's DMAC
 *
 * Register: \a ANA_L3_2:COMMON:DIP_SECURE_ENA
 */
#define VTSS_ANA_L3_2_COMMON_DIP_SECURE_ENA  VTSS_IOREG(VTSS_TO_ANA_L3,0x4ea6)


/** 
 * \brief Virtual Router Leg Protocol Configuration
 *
 * \details
 * Configuration of VRRP base address
 *
 * Register: \a ANA_L3_2:COMMON:VRRP_CFG_1
 */
#define VTSS_ANA_L3_2_COMMON_VRRP_CFG_1      VTSS_IOREG(VTSS_TO_ANA_L3,0x4ea7)

/** 
 * \brief
 * Virtual Router Redundancy Protocol MAC address.  High part of VRRP MAC
 * Address.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * VRID MAC address Bit 47 downto 32.
 *
 * Field: ::VTSS_ANA_L3_2_COMMON_VRRP_CFG_1 . VRRP_BASE_MAC_HIGH
 */
#define  VTSS_F_ANA_L3_2_COMMON_VRRP_CFG_1_VRRP_BASE_MAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L3_2_COMMON_VRRP_CFG_1_VRRP_BASE_MAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L3_2_COMMON_VRRP_CFG_1_VRRP_BASE_MAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask containing I-component ports
 *
 * \details
 * One of two masks that allow to distinguish between I-component and
 * B-component ports when doing VLAN related lookups
 *
 * Register: \a ANA_L3_2:COMMON:VLAN_PORT_TYPE_MASK_CFG
 */
#define VTSS_ANA_L3_2_COMMON_VLAN_PORT_TYPE_MASK_CFG  VTSS_IOREG(VTSS_TO_ANA_L3,0x4ea8)

/**
 * Register Group: \a ANA_L3_2:VLAN
 *
 * VLAN table configuration
 */


/** 
 * \brief VLAN Configuration for B-component
 *
 * \details
 * Various configuration of B-VLAN handles
 *
 * Register: \a ANA_L3_2:VLAN:BVLAN_CFG
 *
 * @param gi Replicator: x_ANA_L3_NUM_VID (??), 0-4095
 */
#define VTSS_ANA_L3_2_VLAN_BVLAN_CFG(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x0,gi,4,0,0)

/** 
 * \brief
 * Each VLAN is associated with one STP instance through this pointer.
 *
 * \details 
 * 0: Default CIST (Common and Internal Spanning Tree) Pointer
 * 1-> Num(MSTP): Multiple Spanning Tree Protocol (MSTP) Pointer
 * others: Undefined
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_BVLAN_CFG . BVLAN_MSTP_PTR
 */
#define  VTSS_F_ANA_L3_2_VLAN_BVLAN_CFG_BVLAN_MSTP_PTR(x)  VTSS_ENCODE_BITFIELD(x,5,7)
#define  VTSS_M_ANA_L3_2_VLAN_BVLAN_CFG_BVLAN_MSTP_PTR     VTSS_ENCODE_BITMASK(5,7)
#define  VTSS_X_ANA_L3_2_VLAN_BVLAN_CFG_BVLAN_MSTP_PTR(x)  VTSS_EXTRACT_BITFIELD(x,5,7)

/** 
 * \brief
 * Disables forwarding of frames from a B-VLAN to ports of the I-component.
 *
 * \details 
 * 0: Forwarding to CBP is allowed.
 * 1: Forwarding to CBP is not allowed.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_BVLAN_CFG . BVLAN_CBP_MBR_DIS
 */
#define  VTSS_F_ANA_L3_2_VLAN_BVLAN_CFG_BVLAN_CBP_MBR_DIS  VTSS_BIT(4)

/** 
 * \brief
 * Enable secure forwarding.
 * Disable forwarding of frames with unknown SMAC on a per VLAN basis
 * related to B-VLAN lookup.
 *
 * \details 
 * 0: Forwarding is allowed independent of SMAC.
 * 1: Forwarding is only allowed for frames with known SMAC.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_BVLAN_CFG . BVLAN_SEC_FWD_ENA
 */
#define  VTSS_F_ANA_L3_2_VLAN_BVLAN_CFG_BVLAN_SEC_FWD_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Disable flooding of frames with unknown DMAC on a per VLAN basis related
 * to B-VLAN lookup.
 *
 * \details 
 * 0: Flooding is allowed.
 * 1: Flooding is disabled.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_BVLAN_CFG . BVLAN_FLOOD_DIS
 */
#define  VTSS_F_ANA_L3_2_VLAN_BVLAN_CFG_BVLAN_FLOOD_DIS  VTSS_BIT(2)

/** 
 * \brief
 * Enable / Disable learning of frames coming from this B-VLAN
 *
 * \details 
 * '0': Enabled
 * '1': Disabled
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_BVLAN_CFG . BVLAN_LRN_DIS
 */
#define  VTSS_F_ANA_L3_2_VLAN_BVLAN_CFG_BVLAN_LRN_DIS  VTSS_BIT(1)

/** 
 * \brief
 * VLAN mirror enable flag. 
 * If this field is set then frames classified to this B-VLAN ID are
 * mirrored.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_BVLAN_CFG . BVLAN_MIRROR_ENA
 */
#define  VTSS_F_ANA_L3_2_VLAN_BVLAN_CFG_BVLAN_MIRROR_ENA  VTSS_BIT(0)


/** 
 * \brief VLAN mapping configuration
 *
 * \details
 * Virtual Mapped (Router) Interface (RLEG=VMID) address per VLAN
 *
 * Register: \a ANA_L3_2:VLAN:VMID_CFG
 *
 * @param gi Replicator: x_ANA_L3_NUM_VID (??), 0-4095
 */
#define VTSS_ANA_L3_2_VLAN_VMID_CFG(gi)      VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x0,gi,4,0,1)

/** 
 * \brief
 * Mapped VLAN ID (VMID) = RLEG.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: RLEG 0
 * ...
 * N: RLEG N
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_VMID_CFG . VMID
 */
#define  VTSS_F_ANA_L3_2_VLAN_VMID_CFG_VMID(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_ANA_L3_2_VLAN_VMID_CFG_VMID     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_ANA_L3_2_VLAN_VMID_CFG_VMID(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief VLAN Configuration
 *
 * \details
 * Various configuration of VLAN handles
 *
 * Register: \a ANA_L3_2:VLAN:VLAN_CFG
 *
 * @param gi Replicator: x_ANA_L3_NUM_VID (??), 0-4095
 */
#define VTSS_ANA_L3_2_VLAN_VLAN_CFG(gi)      VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x0,gi,4,0,2)

/** 
 * \brief
 * Each VLAN is associated with one STP instance through this pointer.
 *
 * \details 
 * 0: Default CIST (Common and Internal Spanning Tree) Pointer
 * 1-> Num(MSTP): Multiple Spanning Tree Protocol (MSTP) Pointer
 * others: Undefined
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_VLAN_CFG . VLAN_MSTP_PTR
 */
#define  VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_MSTP_PTR(x)  VTSS_ENCODE_BITFIELD(x,20,7)
#define  VTSS_M_ANA_L3_2_VLAN_VLAN_CFG_VLAN_MSTP_PTR     VTSS_ENCODE_BITMASK(20,7)
#define  VTSS_X_ANA_L3_2_VLAN_VLAN_CFG_VLAN_MSTP_PTR(x)  VTSS_EXTRACT_BITFIELD(x,20,7)

/** 
 * \brief
 * FID to be used for learning and forwarding
 *
 * \details 
 * 0: FID 0
 * 1: FID 1
 * ...
 * N: FID N
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_VLAN_CFG . VLAN_FID
 */
#define  VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_FID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_ANA_L3_2_VLAN_VLAN_CFG_VLAN_FID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_ANA_L3_2_VLAN_VLAN_CFG_VLAN_FID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Consider all ethernet front ports in port_mask as C-ports, and allow
 * standard pb bridgeing in C-domain. Disables forwarding in B-domain.
 *
 * \details 
 * 0: Consider only ports configured in PORT_TYPE_MASK as C-ports
 * 1: Consider all ethernet front ports as C-ports
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_VLAN_CFG . VLAN_PB_BRIDGED_ENA
 */
#define  VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_PB_BRIDGED_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Enable secure forwarding.
 * Disable forwarding of frames with unknown SMAC on a per VLAN basis
 * related to C-VLAN lookup.
 *
 * \details 
 * 0: Forwarding is allowed independent of SMAC.
 * 1: Forwarding is only allowed for frames with known SMAC.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_VLAN_CFG . VLAN_SEC_FWD_ENA
 */
#define  VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_SEC_FWD_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Disable flooding of frames with unknown DMAC on a per VLAN basis related
 * to C-VLAN lookup.
 *
 * \details 
 * 0: Flooding is allowed.
 * 1: Flooding is disabled.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_VLAN_CFG . VLAN_FLOOD_DIS
 */
#define  VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_FLOOD_DIS  VTSS_BIT(4)

/** 
 * \brief
 * Enable / Disable learning of frames coming from this VLAN
 *
 * \details 
 * '0': Enabled
 * '1': Disabled
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_VLAN_CFG . VLAN_LRN_DIS
 */
#define  VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_LRN_DIS  VTSS_BIT(3)

/** 
 * \brief
 * Enable / disable Router Leg per VLAN.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_VLAN_CFG . VLAN_RLEG_ENA
 */
#define  VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_RLEG_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable / Disable this VLAN being a Private VLAN. 
 * 
 * When traffic comes in on a promiscuous port in a private VLAN, the VLAN
 * mask from the VLAN table is applied. That is, all ports that are members
 * of the private VLAN are potential recipients of frames received on a
 * promiscuous port. 
 * 
 * When traffic comes in on a community port, the VLAN_ISOLATED_MASK is
 * applied in addition to the VLAN mask from the VLAN table to mask out
 * isolated ports. Thereby the ports to which forwarding is possible is
 * reduced to promiscuous ports and the community ports within the private
 * VLAN. 
 * 
 * When traffic comes in on an isolated port, the VLAN_ISOLATED_MASK and
 * the VLAN_COMMUNITY_MASK is applied in addition to the VLAN mask from the
 * VLAN table. This reduces the ports to which forwarding is possible to
 * just the promiscuous ports within the private VLAN.
 *
 * \details 
 * 0: Disabled private vlan based filtering
 * 1: Enabled use of community and isolated port mask
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_VLAN_CFG . VLAN_PRIVATE_ENA
 */
#define  VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_PRIVATE_ENA  VTSS_BIT(1)

/** 
 * \brief
 * VLAN mirror enable flag. If this field is set, frames classified to this
 * ingress VLAN mapped ID are mirrored.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_VLAN_CFG . VLAN_MIRROR_ENA
 */
#define  VTSS_F_ANA_L3_2_VLAN_VLAN_CFG_VLAN_MIRROR_ENA  VTSS_BIT(0)


/** 
 * \brief VLAN Port Mask Configuration
 *
 * \details
 * Configuration of VLAN portmask
 *
 * Register: \a ANA_L3_2:VLAN:VLAN_MASK_CFG
 *
 * @param gi Replicator: x_ANA_L3_NUM_VID (??), 0-4095
 */
#define VTSS_ANA_L3_2_VLAN_VLAN_MASK_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x0,gi,4,0,3)

/**
 * Register Group: \a ANA_L3_2:MSTP
 *
 * MSTP table configuration
 */


/** 
 * \brief MSTP Forwarding Control
 *
 * \details
 * Configuration of forwarding state per MSTP
 *
 * Register: \a ANA_L3_2:MSTP:MSTP_FWD_CFG
 *
 * @param gi Replicator: x_ANA_L3_NUM_MSTP (??), 0-65
 */
#define VTSS_ANA_L3_2_MSTP_MSTP_FWD_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4ea9,gi,2,0,0)


/** 
 * \brief MSTP Learning Control
 *
 * \details
 * Configuration of learning state per MSTP in C-Domain
 *
 * Register: \a ANA_L3_2:MSTP:MSTP_LRN_CFG
 *
 * @param gi Replicator: x_ANA_L3_NUM_MSTP (??), 0-65
 */
#define VTSS_ANA_L3_2_MSTP_MSTP_LRN_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4ea9,gi,2,0,1)

/**
 * Register Group: \a ANA_L3_2:VMID
 *
 * Router Leg table configuration
 */


/** 
 * \brief Router Interface control
 *
 * \details
 * Configuration of Router interface
 *
 * Register: \a ANA_L3_2:VMID:RLEG_CTRL
 *
 * @param gi Replicator: x_ANA_NUM_VMID (??), 0-127
 */
#define VTSS_ANA_L3_2_VMID_RLEG_CTRL(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4c00,gi,4,0,0)

/** 
 * \brief
 * Egress VID to be used for EVMID to EVID mapping when routing
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * Bit 0: Egress VLAN ID bit 0
 * ...
 * Bit 11: Egress VLAN ID bit 11
 *
 * Field: ::VTSS_ANA_L3_2_VMID_RLEG_CTRL . RLEG_EVID
 */
#define  VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_EVID(x)  VTSS_ENCODE_BITFIELD(x,16,12)
#define  VTSS_M_ANA_L3_2_VMID_RLEG_CTRL_RLEG_EVID     VTSS_ENCODE_BITMASK(16,12)
#define  VTSS_X_ANA_L3_2_VMID_RLEG_CTRL_RLEG_EVID(x)  VTSS_EXTRACT_BITFIELD(x,16,12)

/** 
 * \brief
 * IPv4 multicast TTL limit
 * Note: There is an additional check for TTL < 2 for all routed traffic.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Router Leg based MC TTL check disabled
 * 1: If Frame's TTL is < 1 the frame is filtered out
 * 2: If Frame's TTL is < 2 the frame is filtered out
 * 3: If Frame's TTL is < 3 the frame is filtered out
 * ...
 * 255: If Frame's TTL is < 255 the frame is filtered out
 *
 * Field: ::VTSS_ANA_L3_2_VMID_RLEG_CTRL . RLEG_IP4_MC_TTL
 */
#define  VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP4_MC_TTL(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP4_MC_TTL     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP4_MC_TTL(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Enable / disable Virtual Router Legs.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Disabled
 * 1: Enable
 *
 * Field: ::VTSS_ANA_L3_2_VMID_RLEG_CTRL . RLEG_VRID_ENA
 */
#define  VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_VRID_ENA  VTSS_BIT(6)

/** 
 * \brief
 * IPv6 ICMP redirection  enable flag.
 * If enabled then IPv6 frames with IVMD==EVMID are redirected to CPU.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Redirection to CPU is disabled.
 * 1: Redirection to CPU is enabled.
 *
 * Field: ::VTSS_ANA_L3_2_VMID_RLEG_CTRL . RLEG_IP6_ICMP_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP6_ICMP_REDIR_ENA  VTSS_BIT(5)

/** 
 * \brief
 * IPv6 unicast routing enable flag per Router Leg.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Routing is disabled for IPv6 UC frames.
 * 1: Routing is allowed for IPv6 UC frames.
 *
 * Field: ::VTSS_ANA_L3_2_VMID_RLEG_CTRL . RLEG_IP6_UC_ENA
 */
#define  VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP6_UC_ENA  VTSS_BIT(4)

/** 
 * \brief
 * IPv4 multicast enable flag.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Routing of IPv4 MC frames is enabled
 * 1: Routing of IPv4 MC frames is disabled
 *
 * Field: ::VTSS_ANA_L3_2_VMID_RLEG_CTRL . RLEG_IP4_MC_ENA
 */
#define  VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP4_MC_ENA  VTSS_BIT(3)

/** 
 * \brief
 * IPv4 ICMP redirection  enable flag.
 * If enabled then IPv4 frames with IVMD==EVMID are redirected to CPU
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Redirection to CPU is disabled.
 * 1: Redirection to CPU is enabled.
 *
 * Field: ::VTSS_ANA_L3_2_VMID_RLEG_CTRL . RLEG_IP4_ICMP_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP4_ICMP_REDIR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * IPv4 unicast enable flag.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_ANA_L3_2_VMID_RLEG_CTRL . RLEG_IP4_UC_ENA
 */
#define  VTSS_F_ANA_L3_2_VMID_RLEG_CTRL_RLEG_IP4_UC_ENA  VTSS_BIT(1)


/** 
 * \brief Virtual Router Leg configuration
 *
 * \details
 * Configuration of Virtual Router Interface MAC address
 *
 * Register: \a ANA_L3_2:VMID:VRRP_CFG
 *
 * @param gi Replicator: x_ANA_NUM_VMID (??), 0-127
 * @param ri Replicator: x_ANA_L3_NUM_VRID (??), 0-1
 */
#define VTSS_ANA_L3_2_VMID_VRRP_CFG(gi,ri)   VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4c00,gi,4,ri,1)

/** 
 * \brief
 * Virtual Router Legs.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * Bit 0: Router MAC bit 0
 * ...
 * Bit 7: Router MAC bit 7
 *
 * Field: ::VTSS_ANA_L3_2_VMID_VRRP_CFG . RLEG_VRID
 */
#define  VTSS_F_ANA_L3_2_VMID_VRRP_CFG_RLEG_VRID(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_L3_2_VMID_VRRP_CFG_RLEG_VRID     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_L3_2_VMID_VRRP_CFG_RLEG_VRID(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a ANA_L3_2:LPM
 *
 * Longest Prefix Matching TCAM configuration
 */


/** 
 * \brief LPM and REMAP Access control
 *
 * \details
 * Configurations for accessing Longest Prefix Matching TCAM and Remapping
 * table
 *
 * Register: \a ANA_L3_2:LPM:ACCESS_CTRL
 */
#define VTSS_ANA_L3_2_LPM_ACCESS_CTRL        VTSS_IOREG(VTSS_TO_ANA_L3,0x4f2d)

/** 
 * \brief
 * Index for indirect access to LPM and REMAP table.
 * The IDX is always specified as a multiple of quad-word entries.
 * If a half-word entry is used the last bit must be 0
 * If a full-word entry is used the last two bits must be 0
 *
 * \details 
 * 0 : LPM entry 0
 * ...
 * n: LPM entry n
 *
 * Field: ::VTSS_ANA_L3_2_LPM_ACCESS_CTRL . ACCESS_IDX
 */
#define  VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(x)  VTSS_ENCODE_BITFIELD(x,6,10)
#define  VTSS_M_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX     VTSS_ENCODE_BITMASK(6,10)
#define  VTSS_X_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_IDX(x)  VTSS_EXTRACT_BITFIELD(x,6,10)

/** 
 * \brief
 * Specifies whether to do a full-word, half-word or quad-word access when
 * reading or writing.
 * Access is specified by ACCESS_CMD
 *
 * \details 
 * 0: quad word
 * 1: half word
 * 2: full word
 * 3: reserved
 *
 * Field: ::VTSS_ANA_L3_2_LPM_ACCESS_CTRL . ACCESS_WID
 */
#define  VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_WID(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Command when accessing LPM and REMAP table
 * Note that the LPM can be initialized completely by writing bit TCAM_CTRL
 * in register TCAM_BIST of block ANA_L3 to 1. This bit is self-clearing
 * after initialization completed.
 *
 * \details 
 * 0: IDLE
 * 1: Read LPM and REMAP (specified by ACCESS_WID).
 * 2: Write LPM and REMAP (specified by ACCESS_WID).
 * 3: MOVE LPM and REMAP at address ACCESS_IDX through ACCESS_IDX+MV_SIZE-1
 * up MV_NUM_POS positions.
 * 4: MOVE LPM and REMAP at address ACCESS_IDX through ACCESS_IDX+MV_SIZE-1
 * down MV_NUM_POS positions.
 * 5: Init LPM and REMAP entry.
 * other: Reserved
 *
 * Field: ::VTSS_ANA_L3_2_LPM_ACCESS_CTRL . ACCESS_CMD
 */
#define  VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_CMD(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * Perform the LPM/REMAP table access which is specified by ACCESS_CMD
 *
 * \details 
 * 0: Idle
 * 1: Initiate CPU based LPM/REMAP table access
 * The bit is cleared upon completion
 *
 * Field: ::VTSS_ANA_L3_2_LPM_ACCESS_CTRL . ACCESS_SHOT
 */
#define  VTSS_F_ANA_L3_2_LPM_ACCESS_CTRL_ACCESS_SHOT  VTSS_BIT(0)


/** 
 * \brief Configuration for Move Access Operations
 *
 * \details
 * Configurations for moving entrires in Longest Prefix Matching TCAM and
 * Remapping table
 *
 * Register: \a ANA_L3_2:LPM:ACCESS_MV_CFG
 */
#define VTSS_ANA_L3_2_LPM_ACCESS_MV_CFG      VTSS_IOREG(VTSS_TO_ANA_L3,0x4f2e)

/** 
 * \brief
 * This field holds the number of positions the entries must be moved up or
 * down during a MOVE operation.
 *
 * \details 
 * 0 : No entries or actions are moved.
 * 1 : Each entry is to be moved 1 position up or down.
 * 2 : Each entry is to be moved 2 positions up or down.
 * 3 : Each entry is to be moved 3 positions up or down.
 * ...
 *
 * Field: ::VTSS_ANA_L3_2_LPM_ACCESS_MV_CFG . MV_NUM_POS
 */
#define  VTSS_F_ANA_L3_2_LPM_ACCESS_MV_CFG_MV_NUM_POS(x)  VTSS_ENCODE_BITFIELD(x,16,10)
#define  VTSS_M_ANA_L3_2_LPM_ACCESS_MV_CFG_MV_NUM_POS     VTSS_ENCODE_BITMASK(16,10)
#define  VTSS_X_ANA_L3_2_LPM_ACCESS_MV_CFG_MV_NUM_POS(x)  VTSS_EXTRACT_BITFIELD(x,16,10)

/** 
 * \brief
 * This field holds the number of entries that must be moved up or down
 * during a MOVE operation.
 *
 * \details 
 * 0: No entry or action is moved.
 * 1: The entry and/or action at address ACCESS_CTRL.ACCESS_IDX is moved up
 * or down.
 * 2: The entries and/or actions at address ACCESS_CTRL.ACCESS_IDX through
 * ACCESS_CTRL.ACCESS_IDX+1 are moved up or down.
 * 3: The entries and/or actions at address ACCESS_CTRL.ACCESS_IDX through
 * ACCESS_CTRL.ACCESS_IDX+2 are moved up or down.
 * ...
 *
 * Field: ::VTSS_ANA_L3_2_LPM_ACCESS_MV_CFG . MV_SIZE
 */
#define  VTSS_F_ANA_L3_2_LPM_ACCESS_MV_CFG_MV_SIZE(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_ANA_L3_2_LPM_ACCESS_MV_CFG_MV_SIZE     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_ANA_L3_2_LPM_ACCESS_MV_CFG_MV_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief Longest Prefix DATA configuration
 *
 * \details
 * Data pattern used for read and write access to LPM
 *
 * Register: \a ANA_L3_2:LPM:LPM_DATA_CFG
 *
 * @param ri Replicator: x_ANA_LPM_ENTRIES_PER_ROW (??), 0-3
 */
#define VTSS_ANA_L3_2_LPM_LPM_DATA_CFG(ri)   VTSS_IOREG(VTSS_TO_ANA_L3,0x4f2f + (ri))


/** 
 * \brief Longest Prefix MASK configuration
 *
 * \details
 * Register used to mask LPM entry data; masked LPM data are left
 * unconsidered during comparison lookup
 *
 * Register: \a ANA_L3_2:LPM:LPM_MASK_CFG
 *
 * @param ri Replicator: x_ANA_LPM_ENTRIES_PER_ROW (??), 0-3
 */
#define VTSS_ANA_L3_2_LPM_LPM_MASK_CFG(ri)   VTSS_IOREG(VTSS_TO_ANA_L3,0x4f33 + (ri))


/** 
 * \brief IP Prefix usage configuration
 *
 * \details
 * Configration of Entry usage (Routing and/or Security)
 * Each LPM row consists of parts:
 * When reading / writing a full-word then all parts are used.
 * When reading / writing a half-word then half of the parts is used.
 * When reading / writing a quad-word then 1 part is used.
 * Note that per bitgroup all values per word must be equal (e.g for a
 * full-word the values per bitgroup must be equal for all parts)
 *
 * Register: \a ANA_L3_2:LPM:LPM_USAGE_CFG
 *
 * @param ri Replicator: x_ANA_LPM_ENTRIES_PER_ROW (??), 0-3
 */
#define VTSS_ANA_L3_2_LPM_LPM_USAGE_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_L3,0x4f37 + (ri))

/** 
 * \brief
 * Entry usage mask. Specify whether an entry is relevant for source /
 * destination specific or don't care.
 *
 * \details 
 * Per slice:
 * 0: care (entry is source or destination only)
 * 1: don't care (entry is valid for both source and destination lookup)
 *
 * Field: ::VTSS_ANA_L3_2_LPM_LPM_USAGE_CFG . LPM_DST_FLAG_MASK
 */
#define  VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_DST_FLAG_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Entry usage. Specify whether an entry is relevant for source lookup
 * (security) or destination lookup (routing/dest. security). If the
 * corresponding mask (LPM_DST_FLAG_MASK) is don't care, the entry is
 * relevant for both source and destination lookup.
 * Note that each masked data bit is 0 when read back regardless of the
 * written value; this behavior is related to the fact that data and mask
 * are stored as ternary value in the TCAM
 *
 * \details 
 * Per slice:
 * 0: Source entry (Security)
 * 1: Destination entry (LPM)
 *
 * Field: ::VTSS_ANA_L3_2_LPM_LPM_USAGE_CFG . LPM_DST_FLAG_DATA
 */
#define  VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_DST_FLAG_DATA  VTSS_BIT(2)

/** 
 * \brief
 * Specifies the entry type of a 32-bits data word.
 * For IPv4 MC entries 2 consecutive 32-bits data words have to use an IPv4
 * multicast entry type to build a valid IPv4 multicast entry.
 * For IPv6 UC entries 4 consecutive 32-bits data words have to use an IPv6
 * unicast entry type to build a valid IPv6 unicast entry.
 *
 * \details 
 * Per slice:
 * 0: Invalid
 * 1: IPv4 unicast entry
 * 2: IPv4 multicast entry (two entries at ((ACCESS_IDX & ~1) and
 * (ACCESS_IDX & ~1)+1 are accessed)
 * 3: IPv6 unicast entry (four entries at((ACCESS_IDX & ~3) to (ACCESS_IDX
 * & ~3)+3 are accessed)
 *
 * Field: ::VTSS_ANA_L3_2_LPM_LPM_USAGE_CFG . LPM_TYPE
 */
#define  VTSS_F_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_L3_2_LPM_LPM_USAGE_CFG_LPM_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a ANA_L3_2:REMAP
 *
 * REMAP table configuration
 */


/** 
 * \brief ARP/L3MC table addressing
 *
 * \details
 * Configuration of remapping table
 *
 * Register: \a ANA_L3_2:REMAP:REMAP_CFG
 */
#define VTSS_ANA_L3_2_REMAP_REMAP_CFG        VTSS_IOREG(VTSS_TO_ANA_L3,0x4f3b)

/** 
 * \brief
 * Select the number of Equal Cost Multiple Paths (ECMP) for unicast
 * lookup.
 *
 * \details 
 * 0: No equal cost multiple path (ECMP) exists
 * 1: Two equal cost multiple paths (ECMP) exist
 *
 * Field: ::VTSS_ANA_L3_2_REMAP_REMAP_CFG . ECMP_CNT
 */
#define  VTSS_F_ANA_L3_2_REMAP_REMAP_CFG_ECMP_CNT  VTSS_BIT(17)

/** 
 * \brief
 * Base pointer into ARP table or into L3MC table based ENTRY_TYPE.
 *
 * \details 
 * Unicast entry:
 * 0: Entry 0 in ARP table
 * 1: Entry 1 in ARP table
 * ...
 * n: Entry n in ARP table.
 * Multicast entry:
 * 0: Entry 0 in L3MC table
 * 1: Entry 1 in L3MC table
 * ...
 * m: Entry m in L3MC table.
 *
 * Field: ::VTSS_ANA_L3_2_REMAP_REMAP_CFG . BASE_PTR
 */
#define  VTSS_F_ANA_L3_2_REMAP_REMAP_CFG_BASE_PTR(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_ANA_L3_2_REMAP_REMAP_CFG_BASE_PTR     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_ANA_L3_2_REMAP_REMAP_CFG_BASE_PTR(x)  VTSS_EXTRACT_BITFIELD(x,0,10)

/**
 * Register Group: \a ANA_L3_2:ARP
 *
 * ARP table configuration
 */


/** 
 * \brief ARP Data
 *
 * \details
 * Configuration registers for ARP table
 *
 * Register: \a ANA_L3_2:ARP:ARP_CFG_0
 *
 * @param gi Replicator: x_ANA_NUM_ARP (??), 0-1023
 */
#define VTSS_ANA_L3_2_ARP_ARP_CFG_0(gi)      VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4000,gi,2,0,0)

/** 
 * \brief
 * 16 MSB of DMAC (ARP entry) or SMAC (SECURITY entry).
 *
 * \details 
 * Bit 0: MAC bit 32
 * ...
 * Bit 15: MAC bit 47
 *
 * Field: ::VTSS_ANA_L3_2_ARP_ARP_CFG_0 . MAC_MSB
 */
#define  VTSS_F_ANA_L3_2_ARP_ARP_CFG_0_MAC_MSB(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_ANA_L3_2_ARP_ARP_CFG_0_MAC_MSB     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_ANA_L3_2_ARP_ARP_CFG_0_MAC_MSB(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Egress mapped VLAN ID when routing. 
 * Expected Ingress mapped VLAN ID for security lookup when
 * SECUR_MATCH_VMID_ENA is enabled.
 *
 * \details 
 * Bit 0: Egress RLEG ID bit 0
 * ...
 * Bit n: Egress RLEG ID bit n
 *
 * Field: ::VTSS_ANA_L3_2_ARP_ARP_CFG_0 . ARP_VMID
 */
#define  VTSS_F_ANA_L3_2_ARP_ARP_CFG_0_ARP_VMID(x)  VTSS_ENCODE_BITFIELD(x,4,7)
#define  VTSS_M_ANA_L3_2_ARP_ARP_CFG_0_ARP_VMID     VTSS_ENCODE_BITMASK(4,7)
#define  VTSS_X_ANA_L3_2_ARP_ARP_CFG_0_ARP_VMID(x)  VTSS_EXTRACT_BITFIELD(x,4,7)

/** 
 * \brief
 * Enable Security VMID check. 
 * 
 * SIP check: If security check is enabled for mapped VLAN IDs and the
 * IVMID is not equal to the VMID in the ARP table (ARP_VMID) when looking
 * up SIP then the input to VCAP will be deasserted.
 * 
 * DIP check: If security check is enabled for mapped VLAN Ids and the
 * IVMID is not equal to the VMID in the ARP table (ARP_VMID) when looking
 * up DIP and no Router Leg is hit for the frame then the input to VCAP
 * will be deasserted.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_L3_2_ARP_ARP_CFG_0 . SECUR_MATCH_VMID_ENA
 */
#define  VTSS_F_ANA_L3_2_ARP_ARP_CFG_0_SECUR_MATCH_VMID_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable Security MAC check. 
 * 
 * SIP check: If security check is enabled for MAC addresses and the
 * frame's SMAC is not equal to the MAC in the ARP table (ARP_MAC) when
 * looking up SIP then the input to VCAP will be deasserted.
 * 
 * DIP check: If security check is enabled for MAC addresses and the
 * frame's DMAC is not equal to the MAC in the ARP table (ARP_MAC) when
 * looking up DIP and no Router Leg is hit for the frame then the input to
 * VCAP will be deasserted.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_L3_2_ARP_ARP_CFG_0 . SECUR_MATCH_MAC_ENA
 */
#define  VTSS_F_ANA_L3_2_ARP_ARP_CFG_0_SECUR_MATCH_MAC_ENA  VTSS_BIT(1)

/** 
 * \brief
 * ARP valid flag for entry.
 *
 * \details 
 * 0: Invalid
 * 1: Valid.
 *
 * Field: ::VTSS_ANA_L3_2_ARP_ARP_CFG_0 . ARP_ENA
 */
#define  VTSS_F_ANA_L3_2_ARP_ARP_CFG_0_ARP_ENA  VTSS_BIT(0)


/** 
 * \brief ARP Data 1
 *
 * \details
 * Configuration registers for ARP table
 *
 * Register: \a ANA_L3_2:ARP:ARP_CFG_1
 *
 * @param gi Replicator: x_ANA_NUM_ARP (??), 0-1023
 */
#define VTSS_ANA_L3_2_ARP_ARP_CFG_1(gi)      VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4000,gi,2,0,1)

/**
 * Register Group: \a ANA_L3_2:L3MC
 *
 * IPMC table configuration
 */


/** 
 * \brief Multicast VLAN replication mask
 *
 * \details
 * Configuration registers for L3MC table
 *
 * Register: \a ANA_L3_2:L3MC:EVMID_MASK_CFG
 *
 * @param gi Replicator: x_ANA_NUM_IPMC_MASKS (??), 0-127
 * @param ri Replicator: x_ANA_NUM_VMID_DIV32_CEIL (??), 0-3
 */
#define VTSS_ANA_L3_2_L3MC_EVMID_MASK_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4800,gi,8,ri,0)


/** 
 * \brief L3 multicast control register
 *
 * \details
 * L3 multicast control register
 *
 * Register: \a ANA_L3_2:L3MC:L3MC_CTRL
 *
 * @param gi Replicator: x_ANA_NUM_IPMC_MASKS (??), 0-127
 */
#define VTSS_ANA_L3_2_L3MC_L3MC_CTRL(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x4800,gi,8,0,4)

/** 
 * \brief
 * Expected IVMID if RPF check is enabled.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: IVMID 0
 * 1: IVMID 1
 * ...
 * n: IVMID n.
 *
 * Field: ::VTSS_ANA_L3_2_L3MC_L3MC_CTRL . RPF_VMID
 */
#define  VTSS_F_ANA_L3_2_L3MC_L3MC_CTRL_RPF_VMID(x)  VTSS_ENCODE_BITFIELD(x,4,7)
#define  VTSS_M_ANA_L3_2_L3MC_L3MC_CTRL_RPF_VMID     VTSS_ENCODE_BITMASK(4,7)
#define  VTSS_X_ANA_L3_2_L3MC_L3MC_CTRL_RPF_VMID(x)  VTSS_EXTRACT_BITFIELD(x,4,7)

/** 
 * \brief
 * Enable/Disable RPF check.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_L3_2_L3MC_L3MC_CTRL . RPF_CHK_ENA
 */
#define  VTSS_F_ANA_L3_2_L3MC_L3MC_CTRL_RPF_CHK_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable/disable CPU redirection operation of IP multicast frames with TTL
 * less than 2 to the CPU queue for IPv4 header errors. This only applies
 * to IPv4 multicast frames to be routed.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_L3_2_L3MC_L3MC_CTRL . IPMC_TTL_COPY_ENA
 */
#define  VTSS_F_ANA_L3_2_L3MC_L3MC_CTRL_IPMC_TTL_COPY_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L3_2:LPM_REMAP_STICKY
 *
 * Sticky diagnostic status
 */


/** 
 * \brief Router Leg Diagnostic
 *
 * \details
 * Diagnostic registers related to Longest prefix matching TCAM access and
 * remapping table access
 *
 * Register: \a ANA_L3_2:LPM_REMAP_STICKY:L3_LPM_REMAP_STICKY
 */
#define VTSS_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY  VTSS_IOREG(VTSS_TO_ANA_L3,0x4f3c)

/** 
 * \brief
 * Set an IPv6 security longest prefix match entry has been found. Write 1
 * to clear.
 * Note that this bit is only set for source (SIP) based LPM lookups
 *
 * \details 
 * 0: Not found
 * 1: Found.
 *
 * Field: ::VTSS_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . SECUR_IP6_LPM_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_SECUR_IP6_LPM_FOUND_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set an IPv4 security longest prefix match entry has been found. Write 1
 * to clear.
 *
 * \details 
 * 0: Not found
 * 1: Found.
 *
 * Field: ::VTSS_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . SECUR_IP4_LPM_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_SECUR_IP4_LPM_FOUND_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if an IPv6 unicast longest prefix match entry has been found. Write
 * 1 to clear.
 *
 * \details 
 * 0: Not found
 * 1: Found.
 *
 * Field: ::VTSS_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . LPM_IP6UC_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_LPM_IP6UC_FOUND_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if an IPv4 unicast longest prefix match entry has been found. Write
 * 1 to clear.
 *
 * \details 
 * 0: Not found
 * 1: Found.
 *
 * Field: ::VTSS_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . LPM_IP4UC_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_LPM_IP4UC_FOUND_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if an IPv4 Multicast longest prefix match entry has been found and
 * the returned remapped address is less than the number of IPMC table
 * entries. Write 1 to clear.
 *
 * \details 
 * 0: Not found
 * 1: Found.
 *
 * Field: ::VTSS_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . LPM_IP4MC_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_2_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_LPM_IP4MC_FOUND_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L3_2:VLAN_ARP_L3MC_STICKY
 *
 * Sticky diagnostic status
 */


/** 
 * \brief VLAN Diagnostic
 *
 * \details
 * Register: \a ANA_L3_2:VLAN_ARP_L3MC_STICKY:VLAN_STICKY
 */
#define VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY  VTSS_IOREG(VTSS_TO_ANA_L3,0x4f3d)

/** 
 * \brief
 * Set if MSTP forwarding is allowed. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . BMSTP_FWD_ALLOWED_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_BMSTP_FWD_ALLOWED_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * Set if frame has been discarded due to MSTP port fwd state. Write 1 to
 * clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . BMSTP_DISCARD_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_BMSTP_DISCARD_STICKY  VTSS_BIT(13)

/** 
 * \brief
 * Set if learning is denied. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . BVLAN_LRN_DENY_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_BVLAN_LRN_DENY_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * Set if learning is denied by MSTP. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . BMSTP_LRN_DENY_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_BMSTP_LRN_DENY_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * Set if MSTP learning is allowed. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . BMSTP_LRN_ALLOWED_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_BMSTP_LRN_ALLOWED_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * Set if an invalid B-VLAN entry has been looked up. Write 1 to clear.
 *
 * \details 
 * 0: Not invalid
 * 1: Invalid.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . BVLAN_LOOKUP_INVLD_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_BVLAN_LOOKUP_INVLD_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Set if VLAN ingress filtering has been done. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . BVLAN_IGR_FILTER_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_BVLAN_IGR_FILTER_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Set if MSTP forwarding is allowed. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . MSTP_FWD_ALLOWED_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_MSTP_FWD_ALLOWED_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set if frame has been discarded due to MSTP port fwd state. Write 1 to
 * clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . MSTP_DISCARD_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_MSTP_DISCARD_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set if learning is denied. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . VLAN_LRN_DENY_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_VLAN_LRN_DENY_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if learning is denied by MSTP. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . MSTP_LRN_DENY_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_MSTP_LRN_DENY_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if MSTP learning is allowed. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . MSTP_LRN_ALLOWED_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_MSTP_LRN_ALLOWED_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if an invalid VLAN entry has been looked up. Write 1 to clear.
 *
 * \details 
 * 0: Not invalid
 * 1: Invalid.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . VLAN_LOOKUP_INVLD_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_VLAN_LOOKUP_INVLD_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if VLAN ingress filtering has been done. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . VLAN_IGR_FILTER_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_VLAN_IGR_FILTER_STICKY  VTSS_BIT(0)


/** 
 * \brief ARP and L3MC table related diagnostic registers
 *
 * \details
 * L3 ARP / IPMC  Diagnostic
 *
 * Register: \a ANA_L3_2:VLAN_ARP_L3MC_STICKY:L3_ARP_IPMC_STICKY
 */
#define VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY  VTSS_IOREG(VTSS_TO_ANA_L3,0x4f3e)

/** 
 * \brief
 * Set if IPv4 multicast L3 forwarding fails because no VMID is available
 * in the EVMID mask for that frame copy. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not completed
 * 1: Completed.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . NO_MC_VMID_AVAIL_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_NO_MC_VMID_AVAIL_STICKY  VTSS_BIT(25)

/** 
 * \brief
 * Set if the IPv4 multicast lookup failed due to an address that exceeds
 * the IPMC table address range. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . MC_ENTRY_NOT_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_MC_ENTRY_NOT_FOUND_STICKY  VTSS_BIT(24)

/** 
 * \brief
 * Set if the IPv4 lookup failed due to missing match from LPM/REMAP lookup
 * or if the ARP table lookup returns an invalid entry. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . ENTRY_NOT_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_ENTRY_NOT_FOUND_STICKY  VTSS_BIT(23)

/** 
 * \brief
 * Set if security lookup in ARP table fails enabled security check for
 * DIP. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_DIP_FAIL_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_DIP_FAIL_STICKY  VTSS_BIT(22)

/** 
 * \brief
 * Set if an expected IP MC copy derived from the copy in one of the
 * virtual queues is not sent. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not completed
 * 1: Completed.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . MC_LOOPED_CP_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_MC_LOOPED_CP_STICKY  VTSS_BIT(21)

/** 
 * \brief
 * Set if neither L2 nor L3 IPv4 multicast forwarding has been initialized
 * for an IP MC frame. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not completed
 * 1: Completed.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . NO_MC_FWD_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_NO_MC_FWD_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * Set if IPv4 multicast L3 forwarding has been completed. Write 1 to
 * clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not completed
 * 1: Completed.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . L3_MC_FWD_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_L3_MC_FWD_STICKY  VTSS_BIT(19)

/** 
 * \brief
 * Set if RPF check returns a failure (i.e. check enabled and RPF_VMID !=
 * IVMID). Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . MC_RPF_FILTER_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_MC_RPF_FILTER_STICKY  VTSS_BIT(18)

/** 
 * \brief
 * Set if IPv4 multicast L2 forwarding has been completed. Write 1 to
 * clear.
 *
 * \details 
 * 0: Not completed
 * 1: Completed.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . L2_MC_FWD_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_L2_MC_FWD_STICKY  VTSS_BIT(17)

/** 
 * \brief
 * Set if a CPU move operation has been done for a unicast frame due to the
 * ICMP Redirect check. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . UC_ICMP_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_UC_ICMP_REDIR_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * Set if a CPU move operation has been done for a unicast frame due to a
 * zero DMAC address in the ARP entry. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Non-zero DMAC address
 * 1: Zero DMAC address.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . UC_ZERO_DMAC_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_UC_ZERO_DMAC_FOUND_STICKY  VTSS_BIT(15)

/** 
 * \brief
 * In case of a TTL filtering event (i.e. a UC frame with TTL < 2 that hits
 * a valid ARP table entry and is not filtered out due to an 'all-zero' 
 * DMAC) a sticky bit must be set.
 * Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . UC_TTL_FILTERING_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_UC_TTL_FILTERING_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * Set if a valid ARP entry for a unicast frame has been found that allows
 * the frame to be routed. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not found
 * 1: Found.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . UC_ENTRY_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_UC_ENTRY_FOUND_STICKY  VTSS_BIT(13)

/** 
 * \brief
 * Set if IPv4 multicast forwarding has been done. Only set for the 1st
 * copy of an IPMC frame. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_MC_HIT_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_MC_HIT_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * Set if Ingress Router Leg match has been found for an IPv4 multicast
 * frame with a TTL less than 2. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: No match
 * 1: Match.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_MC_TTL_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_MC_TTL_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * Set if Ingress Router Leg match has been found for an IPv4 multicast
 * frame with options. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: No match
 * 1: Match.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_MC_IP_OPT_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_MC_IP_OPT_REDIR_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * Set if SMAC matches for IP4 SIP address. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_IP4_SIP_MATCH_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_IP4_SIP_MATCH_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Set if SMAC matches for IP4 DIP address. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_IP6_DIP_MATCH_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_IP6_DIP_MATCH_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Set if SMAC matches for IP4 DIP address. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_IP4_DIP_MATCH_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_IP4_DIP_MATCH_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Set if an Ingress Router Leg match has been found for an IPv4 multicast
 * frame with header errors. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: No match
 * 1: Match.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_MC_HDR_ERR_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_MC_HDR_ERR_REDIR_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set if no security match in LPM table or lookup in ARP table fails
 * enabled security check. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_SIP_FAIL_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_SIP_FAIL_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set if SMAC matches for IP6 SIP address. Write 1 to clear.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_IP6_SIP_MATCH_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_IP6_SIP_MATCH_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if an Ingress Router Leg match has been found for a unicast frame.
 * Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: Not active
 * 1: Active.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_UC_HIT_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_UC_HIT_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if an Ingress Router Leg match has been found for an IPv4 unicast
 * frame with IPv4 header options or an IPv6 frame with IPv6 header
 * HopByHop options. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: No match
 * 1: Match.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_UC_IP_OPT_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_UC_IP_OPT_REDIR_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if Ingress Router Leg match has been found for an IPv4 unicast frame
 * with header errors. Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: No match
 * 1: Match.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_UC_HDR_ERR_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_UC_HDR_ERR_REDIR_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if Ingress Router Leg match has been found for a non-IPv4 frame.
 * Write 1 to clear.
 * This bitgroup is unused if routing is disabled permanently.
 *
 * \details 
 * 0: No match
 * 1: Match.
 *
 * Field: ::VTSS_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_UC_NONIP_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_2_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_UC_NONIP_REDIR_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L3_2:TCAM_BIST
 *
 * Build in test for TCAM
 */


/** 
 * \brief Control of the TCAM
 *
 * \details
 * Register: \a ANA_L3_2:TCAM_BIST:TCAM_CTRL
 */
#define VTSS_ANA_L3_2_TCAM_BIST_TCAM_CTRL    VTSS_IOREG(VTSS_TO_ANA_L3,0x4f3f)

/** 
 * \brief
 * Set this field to start manual BIST of the TCAM. This field will be
 * cleared once BIST is complete. The BIST procedure requires that the TCAM
 * is initialized before start, setting TCAM_INIT at the same time as
 * setting this field will first initalize the TCAM and then run BIST.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_2_TCAM_BIST_TCAM_CTRL . TCAM_BIST
 */
#define  VTSS_F_ANA_L3_2_TCAM_BIST_TCAM_CTRL_TCAM_BIST  VTSS_BIT(1)

/** 
 * \brief
 * Set this field to start manual initialization of the TCAM. This field is
 * cleared once initialization is complete.
 * The TCAM has random contents after reset and must be initialized prior
 * to usage.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_2_TCAM_BIST_TCAM_CTRL . TCAM_INIT
 */
#define  VTSS_F_ANA_L3_2_TCAM_BIST_TCAM_CTRL_TCAM_INIT  VTSS_BIT(0)


/** 
 * \brief Status for the TCAM
 *
 * \details
 * Register: \a ANA_L3_2:TCAM_BIST:TCAM_STAT
 */
#define VTSS_ANA_L3_2_TCAM_BIST_TCAM_STAT    VTSS_IOREG(VTSS_TO_ANA_L3,0x4f42)

/** 
 * \brief
 * Set if BIST failed.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_2_TCAM_BIST_TCAM_STAT . BIST_ERR
 */
#define  VTSS_F_ANA_L3_2_TCAM_BIST_TCAM_STAT_BIST_ERR  VTSS_BIT(2)

/** 
 * \brief
 * Set when the BIST is still running. When checking the BIST result this
 * field must be cleared.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_2_TCAM_BIST_TCAM_STAT . BIST_BUSY
 */
#define  VTSS_F_ANA_L3_2_TCAM_BIST_TCAM_STAT_BIST_BUSY  VTSS_BIT(1)

/** 
 * \brief
 * Indicates the current operational state of the TCAM.
 *
 * \details 
 * '0': Busy with initialization.
 * '1': Ready to be used.
 *
 * Field: ::VTSS_ANA_L3_2_TCAM_BIST_TCAM_STAT . TCAM_RDY
 */
#define  VTSS_F_ANA_L3_2_TCAM_BIST_TCAM_STAT_TCAM_RDY  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_ANA_L3_2_H_ */
