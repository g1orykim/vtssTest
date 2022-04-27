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

#ifndef _VTSS_QOS_ICFG_H_
#define _VTSS_QOS_ICFG_H_

/**
 * \file qos_icfg.h
 * \brief This file defines the interface to the QoS module's ICFG commands.
 */

/**
  * \brief Convert from storm rate to text.
  *
  * Used for displaying storm rates.
  */
#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH)
char *qos_icfg_storm_rate_txt(u32 rate, char *buf);
#endif /* defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) && defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) */

/**
  * \brief Convert from key type to text.
  *
  * Used for displaying key types.
  */
#if defined(VTSS_ARCH_SERVAL)
const char *qos_icfg_key_type_txt(vtss_vcap_key_type_t key_type);
#endif /* defined(VTSS_ARCH_SERVAL) */

/**
  * \brief Convert from tag type to text.
  *
  * Used for displaying tag types.
  */
const char *qos_icfg_tag_type_text(vtss_vcap_bit_t tagged, vtss_vcap_bit_t s_tag);

/**
  * \brief Initialization function.
  *
  * Call once, preferably from the INIT_CMD_INIT section of
  * the module's _init() function.
  */
vtss_rc qos_icfg_init(void);

#endif /* _VTSS_QOS_ICFG_H_ */
