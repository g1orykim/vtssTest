/*

 Vitesse API software.

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

#ifndef _VTSS_PHY_TS_H_
#define _VTSS_PHY_TS_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)

#ifdef VTSS_FEATURE_WARM_START
/**
 * \brief This is used to sync the vtss_state to PHY.
 *
 * \param port_no      [IN]      port number
 *
 * \return Return code.
 **/
extern vtss_rc vtss_phy_ts_sync(vtss_state_t *vtss_state, const vtss_port_no_t port_no);

extern void vtss_phy_ts_api_debug_print(vtss_state_t *vtss_state,
                                        const vtss_debug_printf_t pr,
                                        const vtss_debug_info_t   *const info);
#endif /* VTSS_FEATURE_WARM_START */

#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#endif /* _VTSS_PHY_TS_H_ */

