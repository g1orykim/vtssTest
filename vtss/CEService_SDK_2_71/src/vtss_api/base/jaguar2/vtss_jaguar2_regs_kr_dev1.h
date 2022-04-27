#ifndef _VTSS_JAGUAR2_REGS_KR_DEV1_H_
#define _VTSS_JAGUAR2_REGS_KR_DEV1_H_

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
 * Target: \a KR_DEV1
 *
 * \see vtss_target_KR_DEV1_e
 *
 * 
 *
 ***********************************************************************/

/**
 * Register Group: \a KR_DEV1:KR_1x0096
 *
 * Not documented
 */


/** 
 * \brief KR PMD control
 *
 * \details
 * Register: \a KR_DEV1:KR_1x0096:KR_1x0096
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_KR_1x0096_KR_1x0096(target)  VTSS_IOREG(target,0x6)

/** 
 * \brief
 * Training enable  
 *
 * \details 
 * 1: Enable KR start-up protocol 
 * 0: Disable KR start-up protocol
 *
 * Field: ::VTSS_KR_DEV1_KR_1x0096_KR_1x0096 . tr_enable
 */
#define  VTSS_F_KR_DEV1_KR_1x0096_KR_1x0096_tr_enable  VTSS_BIT(1)

/** 
 * \brief
 * Restart training (SC) 
 *
 * \details 
 * 1: Reset KR start-up protocol 
 * 0: Normal operation
 *
 * Field: ::VTSS_KR_DEV1_KR_1x0096_KR_1x0096 . tr_restart
 */
#define  VTSS_F_KR_DEV1_KR_1x0096_KR_1x0096_tr_restart  VTSS_BIT(0)

/**
 * Register Group: \a KR_DEV1:KR_1x0097
 *
 * Not documented
 */


/** 
 * \brief KR PMD status
 *
 * \details
 * Register: \a KR_DEV1:KR_1x0097:KR_1x0097
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_KR_1x0097_KR_1x0097(target)  VTSS_IOREG(target,0xf)

/** 
 * \brief
 * Training failure 
 *
 * \details 
 * 1: Training failure has been detected 
 * 0: Training failure has not been detected
 *
 * Field: ::VTSS_KR_DEV1_KR_1x0097_KR_1x0097 . tr_fail
 */
#define  VTSS_F_KR_DEV1_KR_1x0097_KR_1x0097_tr_fail  VTSS_BIT(3)

/** 
 * \brief
 * Startup protocol status 
 *
 * \details 
 * 1: Start-up protocol in progress 
 * 0: Start-up protocol complete
 *
 * Field: ::VTSS_KR_DEV1_KR_1x0097_KR_1x0097 . stprot
 */
#define  VTSS_F_KR_DEV1_KR_1x0097_KR_1x0097_stprot  VTSS_BIT(2)

/** 
 * \brief
 * Frame lock 
 *
 * \details 
 * 1: Training frame delineation detected, 
 * 0: Training frame delineation not detected
 *
 * Field: ::VTSS_KR_DEV1_KR_1x0097_KR_1x0097 . frlock
 */
#define  VTSS_F_KR_DEV1_KR_1x0097_KR_1x0097_frlock  VTSS_BIT(1)

/** 
 * \brief
 * Receiver status 
 *
 * \details 
 * 1: Receiver trained and ready to receive data 
 * 0: Receiver training
 *
 * Field: ::VTSS_KR_DEV1_KR_1x0097_KR_1x0097 . rcvr_rdy
 */
#define  VTSS_F_KR_DEV1_KR_1x0097_KR_1x0097_rcvr_rdy  VTSS_BIT(0)

/**
 * Register Group: \a KR_DEV1:KR_1x0098
 *
 * Not documented
 */


/** 
 * \brief KR LP coefficient update
 *
 * \details
 * Register: \a KR_DEV1:KR_1x0098:KR_1x0098
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_KR_1x0098_KR_1x0098(target)  VTSS_IOREG(target,0x7)

/** 
 * \brief
 * Received coefficient update field
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_KR_1x0098_KR_1x0098 . lpcoef
 */
#define  VTSS_F_KR_DEV1_KR_1x0098_KR_1x0098_lpcoef(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_KR_1x0098_KR_1x0098_lpcoef     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_KR_1x0098_KR_1x0098_lpcoef(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:KR_1x0099
 *
 * Not documented
 */


/** 
 * \brief KR LP status report
 *
 * \details
 * Register: \a KR_DEV1:KR_1x0099:KR_1x0099
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_KR_1x0099_KR_1x0099(target)  VTSS_IOREG(target,0x8)

/** 
 * \brief
 * Received status report field
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_KR_1x0099_KR_1x0099 . lpstat
 */
#define  VTSS_F_KR_DEV1_KR_1x0099_KR_1x0099_lpstat(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_KR_1x0099_KR_1x0099_lpstat     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_KR_1x0099_KR_1x0099_lpstat(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:KR_1x009A
 *
 * Not documented
 */


/** 
 * \brief KR LD coefficient update
 *
 * \details
 * Register: \a KR_DEV1:KR_1x009A:KR_1x009A
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_KR_1x009A_KR_1x009A(target)  VTSS_IOREG(target,0x9)

/** 
 * \brief
 * Transmitted coefficient update field
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_KR_1x009A_KR_1x009A . ldcoef
 */
#define  VTSS_F_KR_DEV1_KR_1x009A_KR_1x009A_ldcoef(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_KR_1x009A_KR_1x009A_ldcoef     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_KR_1x009A_KR_1x009A_ldcoef(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:KR_1x009B
 *
 * Not documented
 */


/** 
 * \brief KR LD status report
 *
 * \details
 * Register: \a KR_DEV1:KR_1x009B:KR_1x009B
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_KR_1x009B_KR_1x009B(target)  VTSS_IOREG(target,0xa)

/** 
 * \brief
 * Transmitted status report field
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_KR_1x009B_KR_1x009B . ldstat
 */
#define  VTSS_F_KR_DEV1_KR_1x009B_KR_1x009B_ldstat(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_KR_1x009B_KR_1x009B_ldstat     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_KR_1x009B_KR_1x009B_ldstat(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:tr_cfg0
 *
 * Not documented
 */


/** 
 * \brief VS training config 0
 *
 * \details
 * Register: \a KR_DEV1:tr_cfg0:tr_cfg0
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_cfg0_tr_cfg0(target)  VTSS_IOREG(target,0x10)

/** 
 * \brief
 * Clock divider value for timer clocks.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg0_tr_cfg0 . tmr_dvdr
 */
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_tmr_dvdr(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_KR_DEV1_tr_cfg0_tr_cfg0_tmr_dvdr     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_KR_DEV1_tr_cfg0_tr_cfg0_tmr_dvdr(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Use directly connected APC signals
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg0_tr_cfg0 . apc_drct_en
 */
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_apc_drct_en  VTSS_BIT(11)

/** 
 * \brief
 * Invert recieved prbs11 within training frame
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg0_tr_cfg0 . rx_inv
 */
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_rx_inv  VTSS_BIT(10)

/** 
 * \brief
 * Invert transmitted prbs11 within training frame
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg0_tr_cfg0 . tx_inv
 */
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_tx_inv  VTSS_BIT(9)

/** 
 * \brief
 * Set local taps starting point 
 *
 * \details 
 * 0: Set to INITIALIZE 
 * 1: Set to PRESET
 *
 * Field: ::VTSS_KR_DEV1_tr_cfg0_tr_cfg0 . ld_pre_init
 */
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_ld_pre_init  VTSS_BIT(4)

/** 
 * \brief
 * Send first LP request 
 *
 * \details 
 * 0: Send   INITIALIZE 
 * 1: Send   PRESET
 *
 * Field: ::VTSS_KR_DEV1_tr_cfg0_tr_cfg0 . lp_pre_init
 */
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_lp_pre_init  VTSS_BIT(3)

/** 
 * \brief
 * Update taps regardless of v2,vp sum.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg0_tr_cfg0 . nosum
 */
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_nosum  VTSS_BIT(2)

/** 
 * \brief
 * Enable partial OB tap configuration.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg0_tr_cfg0 . part_cfg_en
 */
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_part_cfg_en  VTSS_BIT(1)

/** 
 * \brief
 * Allow LP to to control tap settings.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg0_tr_cfg0 . tapctl_en
 */
#define  VTSS_F_KR_DEV1_tr_cfg0_tr_cfg0_tapctl_en  VTSS_BIT(0)

/**
 * Register Group: \a KR_DEV1:tr_cfg1
 *
 * Not documented
 */


/** 
 * \brief VS training config 1
 *
 * \details
 * Register: \a KR_DEV1:tr_cfg1:tr_cfg1
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_cfg1_tr_cfg1(target)  VTSS_IOREG(target,0x11)

/** 
 * \brief
 * Freeze timers. Bit set... 
 *
 * \details 
 * 0: wait 
 * 1: max_wait 
 * 2: 1g 
 * 3: 3g 
 * 4: 10g 
 * 5: pgdet 
 * 6: link_fail 
 * 7: an_wait 
 * 8: break_link
 *
 * Field: ::VTSS_KR_DEV1_tr_cfg1_tr_cfg1 . tmr_hold
 */
#define  VTSS_F_KR_DEV1_tr_cfg1_tr_cfg1_tmr_hold(x)  VTSS_ENCODE_BITFIELD(x,0,9)
#define  VTSS_M_KR_DEV1_tr_cfg1_tr_cfg1_tmr_hold     VTSS_ENCODE_BITMASK(0,9)
#define  VTSS_X_KR_DEV1_tr_cfg1_tr_cfg1_tmr_hold(x)  VTSS_EXTRACT_BITFIELD(x,0,9)

/**
 * Register Group: \a KR_DEV1:tr_cfg2
 *
 * Not documented
 */


/** 
 * \brief VS training config 2
 *
 * \details
 * Register: \a KR_DEV1:tr_cfg2:tr_cfg2
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_cfg2_tr_cfg2(target)  VTSS_IOREG(target,0x12)

/** 
 * \brief
 * max	settings for vp sum.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg2_tr_cfg2 . vp_max
 */
#define  VTSS_F_KR_DEV1_tr_cfg2_tr_cfg2_vp_max(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg2_tr_cfg2_vp_max     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg2_tr_cfg2_vp_max(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * min	settings for v2 sum.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg2_tr_cfg2 . v2_min
 */
#define  VTSS_F_KR_DEV1_tr_cfg2_tr_cfg2_v2_min(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg2_tr_cfg2_v2_min     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg2_tr_cfg2_v2_min(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a KR_DEV1:tr_cfg3
 *
 * Not documented
 */


/** 
 * \brief VS training config 3
 *
 * \details
 * Register: \a KR_DEV1:tr_cfg3:tr_cfg3
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_cfg3_tr_cfg3(target)  VTSS_IOREG(target,0x13)

/** 
 * \brief
 * max	settings for local transmitter.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg3_tr_cfg3 . cp_max
 */
#define  VTSS_F_KR_DEV1_tr_cfg3_tr_cfg3_cp_max(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg3_tr_cfg3_cp_max     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg3_tr_cfg3_cp_max(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * min	settings for local transmitter.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg3_tr_cfg3 . cp_min
 */
#define  VTSS_F_KR_DEV1_tr_cfg3_tr_cfg3_cp_min(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg3_tr_cfg3_cp_min     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg3_tr_cfg3_cp_min(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a KR_DEV1:tr_cfg4
 *
 * Not documented
 */


/** 
 * \brief VS training config 4
 *
 * \details
 * Register: \a KR_DEV1:tr_cfg4:tr_cfg4
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_cfg4_tr_cfg4(target)  VTSS_IOREG(target,0x14)

/** 
 * \brief
 * max	settings for local transmitter.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg4_tr_cfg4 . c0_max
 */
#define  VTSS_F_KR_DEV1_tr_cfg4_tr_cfg4_c0_max(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg4_tr_cfg4_c0_max     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg4_tr_cfg4_c0_max(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * min	settings for local transmitter.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg4_tr_cfg4 . c0_min
 */
#define  VTSS_F_KR_DEV1_tr_cfg4_tr_cfg4_c0_min(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg4_tr_cfg4_c0_min     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg4_tr_cfg4_c0_min(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a KR_DEV1:tr_cfg5
 *
 * Not documented
 */


/** 
 * \brief VS training config 5
 *
 * \details
 * Register: \a KR_DEV1:tr_cfg5:tr_cfg5
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_cfg5_tr_cfg5(target)  VTSS_IOREG(target,0x15)

/** 
 * \brief
 * max	settings for local transmitter.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg5_tr_cfg5 . cm_max
 */
#define  VTSS_F_KR_DEV1_tr_cfg5_tr_cfg5_cm_max(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg5_tr_cfg5_cm_max     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg5_tr_cfg5_cm_max(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * min	settings for local transmitter.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg5_tr_cfg5 . cm_min
 */
#define  VTSS_F_KR_DEV1_tr_cfg5_tr_cfg5_cm_min(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg5_tr_cfg5_cm_min     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg5_tr_cfg5_cm_min(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a KR_DEV1:tr_cfg6
 *
 * Not documented
 */


/** 
 * \brief VS training config 6
 *
 * \details
 * Register: \a KR_DEV1:tr_cfg6:tr_cfg6
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_cfg6_tr_cfg6(target)  VTSS_IOREG(target,0x16)

/** 
 * \brief
 * initialize settings for local transmitter.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg6_tr_cfg6 . cp_init
 */
#define  VTSS_F_KR_DEV1_tr_cfg6_tr_cfg6_cp_init(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg6_tr_cfg6_cp_init     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg6_tr_cfg6_cp_init(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * initialize settings for local transmitter.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg6_tr_cfg6 . c0_init
 */
#define  VTSS_F_KR_DEV1_tr_cfg6_tr_cfg6_c0_init(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg6_tr_cfg6_c0_init     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg6_tr_cfg6_c0_init(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a KR_DEV1:tr_cfg7
 *
 * Not documented
 */


/** 
 * \brief VS training config 7
 *
 * \details
 * Register: \a KR_DEV1:tr_cfg7:tr_cfg7
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_cfg7_tr_cfg7(target)  VTSS_IOREG(target,0x17)

/** 
 * \brief
 * initialize settings for local transmitter.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg7_tr_cfg7 . cm_init
 */
#define  VTSS_F_KR_DEV1_tr_cfg7_tr_cfg7_cm_init(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_KR_DEV1_tr_cfg7_tr_cfg7_cm_init     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_KR_DEV1_tr_cfg7_tr_cfg7_cm_init(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * Signed value to adjust final LP C(+1) tap position from calculated
 * optimal setting.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg7_tr_cfg7 . dfe_ofs
 */
#define  VTSS_F_KR_DEV1_tr_cfg7_tr_cfg7_dfe_ofs(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_cfg7_tr_cfg7_dfe_ofs     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_cfg7_tr_cfg7_dfe_ofs(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a KR_DEV1:tr_cfg8
 *
 * Not documented
 */


/** 
 * \brief VS training config 8
 *
 * \details
 * Register: \a KR_DEV1:tr_cfg8:tr_cfg8
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_cfg8_tr_cfg8(target)  VTSS_IOREG(target,0x18)

/** 
 * \brief
 * Weighted average calculation of DFE tap 1
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg8_tr_cfg8 . wt1
 */
#define  VTSS_F_KR_DEV1_tr_cfg8_tr_cfg8_wt1(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_KR_DEV1_tr_cfg8_tr_cfg8_wt1     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_KR_DEV1_tr_cfg8_tr_cfg8_wt1(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * Weighted average calculation of DFE tap 2
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg8_tr_cfg8 . wt2
 */
#define  VTSS_F_KR_DEV1_tr_cfg8_tr_cfg8_wt2(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_KR_DEV1_tr_cfg8_tr_cfg8_wt2     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_KR_DEV1_tr_cfg8_tr_cfg8_wt2(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Weighted average calculation of DFE tap 3
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg8_tr_cfg8 . wt3
 */
#define  VTSS_F_KR_DEV1_tr_cfg8_tr_cfg8_wt3(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_KR_DEV1_tr_cfg8_tr_cfg8_wt3     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_KR_DEV1_tr_cfg8_tr_cfg8_wt3(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Weighted average calculation of DFE tap 4
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg8_tr_cfg8 . wt4
 */
#define  VTSS_F_KR_DEV1_tr_cfg8_tr_cfg8_wt4(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_KR_DEV1_tr_cfg8_tr_cfg8_wt4     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_KR_DEV1_tr_cfg8_tr_cfg8_wt4(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a KR_DEV1:tr_cfg9
 *
 * Not documented
 */


/** 
 * \brief VS training config 9
 *
 * \details
 * Register: \a KR_DEV1:tr_cfg9:tr_cfg9
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_cfg9_tr_cfg9(target)  VTSS_IOREG(target,0x19)

/** 
 * \brief
 * Number of training frames used for BER calculation.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_cfg9_tr_cfg9 . frcnt_ber
 */
#define  VTSS_F_KR_DEV1_tr_cfg9_tr_cfg9_frcnt_ber(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_cfg9_tr_cfg9_frcnt_ber     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_cfg9_tr_cfg9_frcnt_ber(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:tr_gain
 *
 * Not documented
 */


/** 
 * \brief VS training gain target and margin values
 *
 * \details
 * Register: \a KR_DEV1:tr_gain:tr_gain
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_gain_tr_gain(target)  VTSS_IOREG(target,0x1a)

/** 
 * \brief
 * LP C(0) optimized when GAIN is gain_targ +/- 2*gain_marg
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_gain_tr_gain . gain_marg
 */
#define  VTSS_F_KR_DEV1_tr_gain_tr_gain_gain_marg(x)  VTSS_ENCODE_BITFIELD(x,10,6)
#define  VTSS_M_KR_DEV1_tr_gain_tr_gain_gain_marg     VTSS_ENCODE_BITMASK(10,6)
#define  VTSS_X_KR_DEV1_tr_gain_tr_gain_gain_marg(x)  VTSS_EXTRACT_BITFIELD(x,10,6)

/** 
 * \brief
 * Target value of GAIN setting during LP C(0) optimization.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_gain_tr_gain . gain_targ
 */
#define  VTSS_F_KR_DEV1_tr_gain_tr_gain_gain_targ(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_KR_DEV1_tr_gain_tr_gain_gain_targ     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_KR_DEV1_tr_gain_tr_gain_gain_targ(x)  VTSS_EXTRACT_BITFIELD(x,0,10)

/**
 * Register Group: \a KR_DEV1:tr_coef_ovrd
 *
 * Not documented
 */


/** 
 * \brief VS training coefficient update override
 *
 * \details
 * Register: \a KR_DEV1:tr_coef_ovrd:tr_coef_ovrd
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_coef_ovrd_tr_coef_ovrd(target)  VTSS_IOREG(target,0x1b)

/** 
 * \brief
 * Override Coef_update field to transmit
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_coef_ovrd_tr_coef_ovrd . coef_ovrd
 */
#define  VTSS_F_KR_DEV1_tr_coef_ovrd_tr_coef_ovrd_coef_ovrd(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_coef_ovrd_tr_coef_ovrd_coef_ovrd     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_coef_ovrd_tr_coef_ovrd_coef_ovrd(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:tr_stat_ovrd
 *
 * Not documented
 */


/** 
 * \brief VS training status report override
 *
 * \details
 * Register: \a KR_DEV1:tr_stat_ovrd:tr_stat_ovrd
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_stat_ovrd_tr_stat_ovrd(target)  VTSS_IOREG(target,0x1c)

/** 
 * \brief
 * Override Stat_report field to transmit
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_stat_ovrd_tr_stat_ovrd . stat_ovrd
 */
#define  VTSS_F_KR_DEV1_tr_stat_ovrd_tr_stat_ovrd_stat_ovrd(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_stat_ovrd_tr_stat_ovrd_stat_ovrd     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_stat_ovrd_tr_stat_ovrd_stat_ovrd(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:tr_ovrd
 *
 * Not documented
 */


/** 
 * \brief VS training override
 *
 * \details
 * Register: \a KR_DEV1:tr_ovrd:tr_ovrd
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_ovrd_tr_ovrd(target)  VTSS_IOREG(target,0xb)

/** 
 * \brief
 * Enable manual training
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_ovrd_tr_ovrd . ovrd_en
 */
#define  VTSS_F_KR_DEV1_tr_ovrd_tr_ovrd_ovrd_en  VTSS_BIT(4)

/** 
 * \brief
 * Control of rx_trained variable for training SM
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_ovrd_tr_ovrd . rxtrained_ovrd
 */
#define  VTSS_F_KR_DEV1_tr_ovrd_tr_ovrd_rxtrained_ovrd  VTSS_BIT(3)

/** 
 * \brief
 * Generate BER enable pulse (SC)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_ovrd_tr_ovrd . ber_en_ovrd
 */
#define  VTSS_F_KR_DEV1_tr_ovrd_tr_ovrd_ber_en_ovrd  VTSS_BIT(2)

/** 
 * \brief
 * Generate Coef_update_valid pulse (SC)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_ovrd_tr_ovrd . coef_ovrd_vld
 */
#define  VTSS_F_KR_DEV1_tr_ovrd_tr_ovrd_coef_ovrd_vld  VTSS_BIT(1)

/** 
 * \brief
 * Generate Stat_report_valid pulse (SC)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_ovrd_tr_ovrd . stat_ovrd_vld
 */
#define  VTSS_F_KR_DEV1_tr_ovrd_tr_ovrd_stat_ovrd_vld  VTSS_BIT(0)

/**
 * Register Group: \a KR_DEV1:tr_step
 *
 * Not documented
 */


/** 
 * \brief VS training state step
 *
 * \details
 * Register: \a KR_DEV1:tr_step:tr_step
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_step_tr_step(target)  VTSS_IOREG(target,0xc)

/** 
 * \brief
 * Step to next lptrain state (if at breakpoint) (SC)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_step_tr_step . step
 */
#define  VTSS_F_KR_DEV1_tr_step_tr_step_step  VTSS_BIT(0)

/**
 * Register Group: \a KR_DEV1:tr_mthd
 *
 * Not documented
 */


/** 
 * \brief VS training method
 *
 * \details
 * Register: \a KR_DEV1:tr_mthd:tr_mthd
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_mthd_tr_mthd(target)  VTSS_IOREG(target,0x1d)

/** 
 * \brief
 * Training method for remote C(+1) 
 *
 * \details 
 * 0 : BER method
 * 1 : Gain method
 * 2 : DFE method
 *
 * Field: ::VTSS_KR_DEV1_tr_mthd_tr_mthd . mthd_cp
 */
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_mthd_cp(x)  VTSS_ENCODE_BITFIELD(x,10,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_mthd_cp     VTSS_ENCODE_BITMASK(10,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_mthd_cp(x)  VTSS_EXTRACT_BITFIELD(x,10,2)

/** 
 * \brief
 * Training method for remote C(0)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_mthd_tr_mthd . mthd_c0
 */
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_mthd_c0(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_mthd_c0     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_mthd_c0(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Training method for remote C(-1)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_mthd_tr_mthd . mthd_cm
 */
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_mthd_cm(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_mthd_cm     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_mthd_cm(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * remote tap to optimize first 
 *
 * \details 
 * 0 : C(-1)
 * 1 : C(0)
 * 2 : C(+1)
 *
 * Field: ::VTSS_KR_DEV1_tr_mthd_tr_mthd . ord1
 */
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_ord1(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_ord1     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_ord1(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * remote tap to optimize second
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_mthd_tr_mthd . ord2
 */
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_ord2(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_ord2     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_ord2(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * remote tap to optimize third
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_mthd_tr_mthd . ord3
 */
#define  VTSS_F_KR_DEV1_tr_mthd_tr_mthd_ord3(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_KR_DEV1_tr_mthd_tr_mthd_ord3     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_KR_DEV1_tr_mthd_tr_mthd_ord3(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a KR_DEV1:tr_ber_thr
 *
 * Not documented
 */


/** 
 * \brief VS training BER threshold settings
 *
 * \details
 * Register: \a KR_DEV1:tr_ber_thr:tr_ber_thr
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_ber_thr_tr_ber_thr(target)  VTSS_IOREG(target,0x1e)

/** 
 * \brief
 * Only consider error count > ber_err_th
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_ber_thr_tr_ber_thr . ber_err_th
 */
#define  VTSS_F_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_err_th(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_err_th     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_err_th(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Only consider errored range > ber_wid_th
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_ber_thr_tr_ber_thr . ber_wid_th
 */
#define  VTSS_F_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_wid_th(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_wid_th     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_KR_DEV1_tr_ber_thr_tr_ber_thr_ber_wid_th(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a KR_DEV1:tr_ber_ofs
 *
 * Not documented
 */


/** 
 * \brief VS training BER offset setting
 *
 * \details
 * Register: \a KR_DEV1:tr_ber_ofs:tr_ber_ofs
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_ber_ofs_tr_ber_ofs(target)  VTSS_IOREG(target,0x1f)

/** 
 * \brief
 * Signed value to adjust final tap position from calculated optimal
 * setting.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_ber_ofs_tr_ber_ofs . ber_ofs
 */
#define  VTSS_F_KR_DEV1_tr_ber_ofs_tr_ber_ofs_ber_ofs(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_KR_DEV1_tr_ber_ofs_tr_ber_ofs_ber_ofs     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_KR_DEV1_tr_ber_ofs_tr_ber_ofs_ber_ofs(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a KR_DEV1:tr_lutsel
 *
 * Not documented
 */


/** 
 * \brief VS training LUT selection
 *
 * \details
 * Register: \a KR_DEV1:tr_lutsel:tr_lutsel
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_lutsel_tr_lutsel(target)  VTSS_IOREG(target,0x20)

/** 
 * \brief
 * Selects LUT table entry (0 to 63).
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_lutsel_tr_lutsel . lut_row
 */
#define  VTSS_F_KR_DEV1_tr_lutsel_tr_lutsel_lut_row(x)  VTSS_ENCODE_BITFIELD(x,3,6)
#define  VTSS_M_KR_DEV1_tr_lutsel_tr_lutsel_lut_row     VTSS_ENCODE_BITMASK(3,6)
#define  VTSS_X_KR_DEV1_tr_lutsel_tr_lutsel_lut_row(x)  VTSS_EXTRACT_BITFIELD(x,3,6)

/** 
 * \brief
 * Selects LUT for lut_o 
 *
 * \details 
 * 0: Gain 
 * 1: DFE_1
 * 2: DFE_2
 * 3: DFE_avg_1 
 * 4: DFE_avg_2 
 * 5: BER_1
 * 6: BER_2
 * 7: BER_3
 *
 * Field: ::VTSS_KR_DEV1_tr_lutsel_tr_lutsel . lut_sel
 */
#define  VTSS_F_KR_DEV1_tr_lutsel_tr_lutsel_lut_sel(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_KR_DEV1_tr_lutsel_tr_lutsel_lut_sel     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_KR_DEV1_tr_lutsel_tr_lutsel_lut_sel(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a KR_DEV1:tr_brkmask
 *
 * Not documented
 */


/** 
 * \brief VS training break_mask lsw
 *
 * \details
 * Register: \a KR_DEV1:tr_brkmask:brkmask_lsw
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_brkmask_brkmask_lsw(target)  VTSS_IOREG(target,0x21)

/** 
 * \brief
 * Select lptrain state machine breakpoints. Each bit correpsonds to a
 * state (see design doc)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_brkmask_brkmask_lsw . brkmask_lsw
 */
#define  VTSS_F_KR_DEV1_tr_brkmask_brkmask_lsw_brkmask_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_brkmask_brkmask_lsw_brkmask_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_brkmask_brkmask_lsw_brkmask_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief VS training break_mask msw
 *
 * \details
 * Register: \a KR_DEV1:tr_brkmask:tr_brkmask_msw
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_brkmask_tr_brkmask_msw(target)  VTSS_IOREG(target,0x22)

/** 
 * \brief
 * Select lptrain state machine breakpoints. Each bit correpsonds to a
 * state (see design doc)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_brkmask_tr_brkmask_msw . brkmask_msw
 */
#define  VTSS_F_KR_DEV1_tr_brkmask_tr_brkmask_msw_brkmask_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_brkmask_tr_brkmask_msw_brkmask_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_brkmask_tr_brkmask_msw_brkmask_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:tr_romadr
 *
 * Not documented
 */


/** 
 * \brief VS training ROM address for gain
 *
 * \details
 * Register: \a KR_DEV1:tr_romadr:romadr1
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_romadr_romadr1(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * ROM starting address of Initial GAIN routine
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_romadr_romadr1 . romadr_gain1
 */
#define  VTSS_F_KR_DEV1_tr_romadr_romadr1_romadr_gain1(x)  VTSS_ENCODE_BITFIELD(x,7,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr1_romadr_gain1     VTSS_ENCODE_BITMASK(7,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr1_romadr_gain1(x)  VTSS_EXTRACT_BITFIELD(x,7,7)

/** 
 * \brief
 * ROM starting address of Iterative GAIN routine
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_romadr_romadr1 . romadr_gain2
 */
#define  VTSS_F_KR_DEV1_tr_romadr_romadr1_romadr_gain2(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr1_romadr_gain2     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr1_romadr_gain2(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief VS training ROM address for dfe
 *
 * \details
 * Register: \a KR_DEV1:tr_romadr:romadr2
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_romadr_romadr2(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * ROM starting address of Initial DFE routine
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_romadr_romadr2 . romadr_dfe1
 */
#define  VTSS_F_KR_DEV1_tr_romadr_romadr2_romadr_dfe1(x)  VTSS_ENCODE_BITFIELD(x,7,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr2_romadr_dfe1     VTSS_ENCODE_BITMASK(7,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr2_romadr_dfe1(x)  VTSS_EXTRACT_BITFIELD(x,7,7)

/** 
 * \brief
 * ROM starting address of Iterative DFE routine
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_romadr_romadr2 . romadr_dfe2
 */
#define  VTSS_F_KR_DEV1_tr_romadr_romadr2_romadr_dfe2(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr2_romadr_dfe2     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr2_romadr_dfe2(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief VS training ROM address for ber
 *
 * \details
 * Register: \a KR_DEV1:tr_romadr:romadr3
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_romadr_romadr3(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * ROM starting address of Initial BER routine
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_romadr_romadr3 . romadr_ber1
 */
#define  VTSS_F_KR_DEV1_tr_romadr_romadr3_romadr_ber1(x)  VTSS_ENCODE_BITFIELD(x,7,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr3_romadr_ber1     VTSS_ENCODE_BITMASK(7,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr3_romadr_ber1(x)  VTSS_EXTRACT_BITFIELD(x,7,7)

/** 
 * \brief
 * ROM starting address of Iterative BER routine
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_romadr_romadr3 . romadr_ber2
 */
#define  VTSS_F_KR_DEV1_tr_romadr_romadr3_romadr_ber2(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr3_romadr_ber2     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr3_romadr_ber2(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief VS training ROM address for end and obcfg
 *
 * \details
 * Register: \a KR_DEV1:tr_romadr:romadr4
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_romadr_romadr4(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * ROM starting address of post-training routing
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_romadr_romadr4 . romadr_end
 */
#define  VTSS_F_KR_DEV1_tr_romadr_romadr4_romadr_end(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_romadr_romadr4_romadr_end     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_romadr_romadr4_romadr_end(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

/**
 * Register Group: \a KR_DEV1:obcfg_addr
 *
 * Not documented
 */


/** 
 * \brief VS training ROM address for end and obcfg
 *
 * \details
 * Register: \a KR_DEV1:obcfg_addr:obcfg_addr
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_obcfg_addr_obcfg_addr(target)  VTSS_IOREG(target,0x23)

/** 
 * \brief
 * Address of OB tap configuration settings
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_obcfg_addr_obcfg_addr . obcfg_addr
 */
#define  VTSS_F_KR_DEV1_obcfg_addr_obcfg_addr_obcfg_addr(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_obcfg_addr_obcfg_addr_obcfg_addr     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_obcfg_addr_obcfg_addr_obcfg_addr(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

/**
 * Register Group: \a KR_DEV1:apc_tmr
 *
 * Not documented
 */


/** 
 * \brief VS training apc_timer
 *
 * \details
 * Register: \a KR_DEV1:apc_tmr:apc_tmr
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_apc_tmr_apc_tmr(target)  VTSS_IOREG(target,0x24)

/** 
 * \brief
 * Delay between LP tap update, and capture of direct-connect apc values
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_apc_tmr_apc_tmr . apc_tmr
 */
#define  VTSS_F_KR_DEV1_apc_tmr_apc_tmr_apc_tmr(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_apc_tmr_apc_tmr_apc_tmr     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_apc_tmr_apc_tmr_apc_tmr(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:wt_tmr
 *
 * Not documented
 */


/** 
 * \brief VS training wait_timer
 *
 * \details
 * Register: \a KR_DEV1:wt_tmr:wt_tmr
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_wt_tmr_wt_tmr(target)   VTSS_IOREG(target,0x25)

/** 
 * \brief
 * wait_timer for training state machine to allow extra training frames to
 * be exchanged
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_wt_tmr_wt_tmr . wt_tmr
 */
#define  VTSS_F_KR_DEV1_wt_tmr_wt_tmr_wt_tmr(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_wt_tmr_wt_tmr_wt_tmr     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_wt_tmr_wt_tmr_wt_tmr(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:mw_tmr
 *
 * Not documented
 */


/** 
 * \brief VS training maxwait_timer lsw
 *
 * \details
 * Register: \a KR_DEV1:mw_tmr:mw_tmr_lsw
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_mw_tmr_mw_tmr_lsw(target)  VTSS_IOREG(target,0x26)

/** 
 * \brief
 * maxwait_timer, when training expires and failure declared. 500ms
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_mw_tmr_mw_tmr_lsw . mw_tmr_lsw
 */
#define  VTSS_F_KR_DEV1_mw_tmr_mw_tmr_lsw_mw_tmr_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_mw_tmr_mw_tmr_lsw_mw_tmr_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_mw_tmr_mw_tmr_lsw_mw_tmr_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief VS training maxwait_timer msw
 *
 * \details
 * Register: \a KR_DEV1:mw_tmr:mw_tmr_msw
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_mw_tmr_mw_tmr_msw(target)  VTSS_IOREG(target,0x27)

/** 
 * \brief
 * maxwait_timer, when training expires and failure declared. 500ms
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_mw_tmr_mw_tmr_msw . mw_tmr_msw
 */
#define  VTSS_F_KR_DEV1_mw_tmr_mw_tmr_msw_mw_tmr_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_mw_tmr_mw_tmr_msw_mw_tmr_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_mw_tmr_mw_tmr_msw_mw_tmr_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:tr_sts1
 *
 * Not documented
 */


/** 
 * \brief VS training status 1
 *
 * \details
 * Register: \a KR_DEV1:tr_sts1:tr_sts1
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_sts1_tr_sts1(target)  VTSS_IOREG(target,0xd)

/** 
 * \brief
 * Indicates prbs11 checker is active
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_sts1_tr_sts1 . ber_busy
 */
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_ber_busy  VTSS_BIT(12)

/** 
 * \brief
 * Training state machine
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_sts1_tr_sts1 . tr_sm
 */
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_tr_sm(x)  VTSS_ENCODE_BITFIELD(x,9,3)
#define  VTSS_M_KR_DEV1_tr_sts1_tr_sts1_tr_sm     VTSS_ENCODE_BITMASK(9,3)
#define  VTSS_X_KR_DEV1_tr_sts1_tr_sts1_tr_sm(x)  VTSS_EXTRACT_BITFIELD(x,9,3)

/** 
 * \brief
 * LP training state machine
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_sts1_tr_sts1 . lptrain_sm
 */
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_lptrain_sm(x)  VTSS_ENCODE_BITFIELD(x,4,5)
#define  VTSS_M_KR_DEV1_tr_sts1_tr_sts1_lptrain_sm     VTSS_ENCODE_BITMASK(4,5)
#define  VTSS_X_KR_DEV1_tr_sts1_tr_sts1_lptrain_sm(x)  VTSS_EXTRACT_BITFIELD(x,4,5)

/** 
 * \brief
 * Indicates gain_target was not reached during LP training
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_sts1_tr_sts1 . gain_fail
 */
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_gain_fail  VTSS_BIT(3)

/** 
 * \brief
 * training variable from training state machine
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_sts1_tr_sts1 . training
 */
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_training  VTSS_BIT(2)

/** 
 * \brief
 * Indicates a DME violation has occured (LH)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_sts1_tr_sts1 . dme_viol
 */
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_dme_viol  VTSS_BIT(1)

/** 
 * \brief
 * Indicates that local and remote training has completed
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_sts1_tr_sts1 . tr_done
 */
#define  VTSS_F_KR_DEV1_tr_sts1_tr_sts1_tr_done  VTSS_BIT(0)

/**
 * Register Group: \a KR_DEV1:tr_sts2
 *
 * Not documented
 */


/** 
 * \brief VS training status 2
 *
 * \details
 * Register: \a KR_DEV1:tr_sts2:tr_sts2
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_sts2_tr_sts2(target)  VTSS_IOREG(target,0xe)

/** 
 * \brief
 * CP range error (LH)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_sts2_tr_sts2 . cp_range_err
 */
#define  VTSS_F_KR_DEV1_tr_sts2_tr_sts2_cp_range_err  VTSS_BIT(2)

/** 
 * \brief
 * C0 range error (LH)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_sts2_tr_sts2 . c0_range_err
 */
#define  VTSS_F_KR_DEV1_tr_sts2_tr_sts2_c0_range_err  VTSS_BIT(1)

/** 
 * \brief
 * CM range error (LH)
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_sts2_tr_sts2 . cm_range_err
 */
#define  VTSS_F_KR_DEV1_tr_sts2_tr_sts2_cm_range_err  VTSS_BIT(0)

/**
 * Register Group: \a KR_DEV1:tr_tapval
 *
 * Not documented
 */


/** 
 * \brief VS tap CM value
 *
 * \details
 * Register: \a KR_DEV1:tr_tapval:tr_cmval
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_tapval_tr_cmval(target)  VTSS_IOREG(target,0x28)

/** 
 * \brief
 * CM value
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_tapval_tr_cmval . cm_val
 */
#define  VTSS_F_KR_DEV1_tr_tapval_tr_cmval_cm_val(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_tapval_tr_cmval_cm_val     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_tapval_tr_cmval_cm_val(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief VS tap C0 value
 *
 * \details
 * Register: \a KR_DEV1:tr_tapval:tr_c0val
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_tapval_tr_c0val(target)  VTSS_IOREG(target,0x29)

/** 
 * \brief
 * C0 value
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_tapval_tr_c0val . c0_val
 */
#define  VTSS_F_KR_DEV1_tr_tapval_tr_c0val_c0_val(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_tapval_tr_c0val_c0_val     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_tapval_tr_c0val_c0_val(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief VS tap CP value
 *
 * \details
 * Register: \a KR_DEV1:tr_tapval:tr_cpval
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_tapval_tr_cpval(target)  VTSS_IOREG(target,0x2a)

/** 
 * \brief
 * CP value
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_tapval_tr_cpval . cp_val
 */
#define  VTSS_F_KR_DEV1_tr_tapval_tr_cpval_cp_val(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_KR_DEV1_tr_tapval_tr_cpval_cp_val     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_KR_DEV1_tr_tapval_tr_cpval_cp_val(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

/**
 * Register Group: \a KR_DEV1:tr_frames_sent
 *
 * Not documented
 */


/** 
 * \brief VS training frames_sent lsw
 *
 * \details
 * Register: \a KR_DEV1:tr_frames_sent:frsent_lsw
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_frames_sent_frsent_lsw(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Number of training frames sent to complete training.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_frames_sent_frsent_lsw . frsent_lsw
 */
#define  VTSS_F_KR_DEV1_tr_frames_sent_frsent_lsw_frsent_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_frames_sent_frsent_lsw_frsent_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_frames_sent_frsent_lsw_frsent_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief VS training frames_sent lsw
 *
 * \details
 * Register: \a KR_DEV1:tr_frames_sent:frsent_msw
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_frames_sent_frsent_msw(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * Number of training frames sent to complete training.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_frames_sent_frsent_msw . frsent_msw
 */
#define  VTSS_F_KR_DEV1_tr_frames_sent_frsent_msw_frsent_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_frames_sent_frsent_msw_frsent_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_frames_sent_frsent_msw_frsent_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:tr_lut
 *
 * Not documented
 */


/** 
 * \brief VS training lut_read lsw
 *
 * \details
 * Register: \a KR_DEV1:tr_lut:lut_lsw
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_lut_lut_lsw(target)  VTSS_IOREG(target,0x2b)

/** 
 * \brief
 * Measured value of selected LUT.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_lut_lut_lsw . lut_lsw
 */
#define  VTSS_F_KR_DEV1_tr_lut_lut_lsw_lut_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_lut_lut_lsw_lut_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_lut_lut_lsw_lut_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief VS training lut_read msw
 *
 * \details
 * Register: \a KR_DEV1:tr_lut:lut_msw
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_lut_lut_msw(target)  VTSS_IOREG(target,0x2c)

/** 
 * \brief
 * Measured value of selected LUT.
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_lut_lut_msw . lut_msw
 */
#define  VTSS_F_KR_DEV1_tr_lut_lut_msw_lut_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_lut_lut_msw_lut_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_lut_lut_msw_lut_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a KR_DEV1:tr_errcnt
 *
 * Not documented
 */


/** 
 * \brief VS training prbs11 error_count
 *
 * \details
 * Register: \a KR_DEV1:tr_errcnt:tr_errcnt
 *
 * @param target A \a ::vtss_target_KR_DEV1_e target
 */
#define VTSS_KR_DEV1_tr_errcnt_tr_errcnt(target)  VTSS_IOREG(target,0x2d)

/** 
 * \brief
 * bit error count of prbs11 checker
 *
 * \details 
 * Field: ::VTSS_KR_DEV1_tr_errcnt_tr_errcnt . errcnt
 */
#define  VTSS_F_KR_DEV1_tr_errcnt_tr_errcnt_errcnt(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_KR_DEV1_tr_errcnt_tr_errcnt_errcnt     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_KR_DEV1_tr_errcnt_tr_errcnt_errcnt(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_JAGUAR2_REGS_KR_DEV1_H_ */
