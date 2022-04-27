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

#ifndef _VTSS_SILABS_CLK_H_
#define _VTSS_SILABS_CLK_H_

#if defined(VTSS_CHIP_JAGUAR_1)
#define VTSS_PHY_TS_SILABS_CLK_DLL 1
#endif /* VTSS_CHIP_JAGUAR_1 */


#include "vtss_api.h"

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#if defined(VTSS_PHY_TS_SILABS_CLK_DLL)
vtss_rc si5326_dll_init(void);
#endif /* VTSS_PHY_TS_SILABS_CLK_DLL */
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#endif /* _VTSS_SILABS_CLK_H_ */
