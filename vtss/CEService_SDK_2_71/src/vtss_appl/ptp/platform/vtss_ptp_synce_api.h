/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_PTP_SYNCE_API_H_
#define _VTSS_PTP_SYNCE_API_H_
#include "vtss_ptp_types.h"

typedef struct init_synce_t {
	u64 init_rfreq;
    u8  n1Low;
	u32 magic_word;
} init_synce_t;

#define SYNCE_CONF_MAGIC_WORD 0xff00aa55

/**
 * \brief Adjust the SyncE clock frequency ratio
 *
 * \param adj Clock ratio frequency offset in 0,1 ppb (parts pr billion).
 *      adj > 0 => clock runs faster
 */
void vtss_synce_clock_set_adjtimer(Integer32 adj);

/**
 * \brief Initialize the SyncE clock frequency ratio control.
 * \returns 0 if synce clock adjustable XC is not present
 *          1 if synce clock adjustable XC is present
 * The internal initial clock frequency reference is read from HW and saved
 * for later use.
 * The clock is based on the Si570 variable XO, it can be adjusted +/- 3500 PPM
 * The si570 is accessed via the I2C bus.
 */
int vtss_synce_clock_init_adjtimer(init_synce_t *init_synce, BOOL cold);


#endif // _VTSS_PTP_SYNCE_API_H_


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
