/*

 Vitesse Switch API software.

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _ZL_3034X_SYNCE_CLOCK_API_H_
#define _ZL_3034X_SYNCE_CLOCK_API_H_

#include "vtss_types.h"
//#include "zl_3034x_api.h"


vtss_rc zl_3034x_clock_init(BOOL  cold_init);
vtss_rc zl_3034x_clock_startup(BOOL  cold_init);

vtss_rc zl_3034x_clock_selection_mode_set(const clock_selection_mode_t   mode,
                                          const uint                     clock_input);

vtss_rc zl_3034x_clock_selector_state_get(clock_selector_state_t  *const selector_state,
        uint                    *const clock_input);

vtss_rc zl_3034x_clock_priority_set(const uint   clock_input,
                                    const uint   priority);

vtss_rc zl_3034x_clock_priority_get(const uint   clock_input,
                                    uint         *const priority);

vtss_rc zl_3034x_clock_los_get(const uint   clock_input,
                               BOOL         *const los);

vtss_rc   zl_3034x_clock_losx_state_get(BOOL *const state);

vtss_rc   zl_3034x_clock_lol_state_get(BOOL         *const state);

vtss_rc   zl_3034x_clock_dhold_state_get(BOOL         *const state);

void zl_3034x_clock_event_poll(BOOL interrupt,  clock_event_type_t *ev_mask);

void zl_3034x_clock_event_enable(clock_event_type_t ev_mask);

vtss_rc zl_3034x_station_clk_out_freq_set(const u32 freq_khz);

vtss_rc zl_3034x_station_clk_in_freq_set(const u32 freq_khz);

vtss_rc zl_3034x_eec_option_set(const clock_eec_option_t clock_eec_option);

#endif // _ZL_3034X_SYNCE_CLOCK_API_H_


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
