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

#ifndef _TOD_API_H_
#define _TOD_API_H_



vtss_rc tod_init(vtss_init_data_t *data);

typedef enum  {
    VTSS_TOD_INTERNAL_TC_MODE_30BIT,         /* 30 BIT mode, reserved field */
    VTSS_TOD_INTERNAL_TC_MODE_32BIT,         /* 32 BIT mode, reserved field */
    VTSS_TOD_INTERNAL_TC_MODE_44BIT,         /* 44 BIT mode, Sub and Add */
    VTSS_TOD_INTERNAL_TC_MODE_48BIT,         /* 48 BIT mode, Sub and Add */
    VTSS_TOD_INTERNAL_TC_MODE_MAX,           /* invalid mode */
    
} vtss_tod_internal_tc_mode_t;

BOOL tod_tc_mode_get(vtss_tod_internal_tc_mode_t *mode);
BOOL tod_tc_mode_set(vtss_tod_internal_tc_mode_t *mode);

BOOL tod_port_phy_ts_get(BOOL *ts, vtss_port_no_t portnum);
BOOL tod_port_phy_ts_set(BOOL *ts, vtss_port_no_t portnum);

#if defined(VTSS_FEATURE_PHY_TIMESTAMP) && defined(VTSS_ARCH_JAGUAR_1)
BOOL tod_ref_clock_freg_get(vtss_phy_ts_clockfreq_t *freq);
BOOL tod_ref_clock_freg_set(vtss_phy_ts_clockfreq_t *freq);
#endif

BOOL tod_ready(void);

#endif // _tod_API_H_


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
