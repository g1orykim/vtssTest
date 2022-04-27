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

#ifndef _DOT1X_H_
#define _DOT1X_H_

#include "vtss_types.h"   /* For BOOL               */
#include "vtss_nas_api.h" /* For nas_port_control_t */

// Semi-public functions and macros

/******************************************************************************/
// Port number conversion macros.
// nas_port_t counts from 1.
/******************************************************************************/
#ifndef NAS_PORT_NO_START
#define NAS_PORT_NO_START 1
#endif
// So in the 802.1X core-library, ports are numbered as follows (ex with 16 isids and 26 ports with two as stack ports):
// -------------------------------------------------------------
// 802.1X Core Lib Port Number | ISID | Switch API Port Number |
// -------------------------------------------------------------
//      1                      |  1   |  0                     |
//      2                      |  1   |  1                     |
//    ...                      | ...  | ...                    |
//     26                      |  1   | 25                     |
//     27                      |  2   |  0                     |
//    ...                      | ...  | ...                    |
//    416                      | 16   | 25                     |
// -------------------------------------------------------------
// To confuse it even more, the l2proto module uses other port numbers, since
// it somehow includes GLAGs, but the l2proto module operates on isid and
// switch API port numbers and contains its own conversion functions
// called L2PORT2PORT(isid, api_port) to convert to an l2proto port.
#define DOT1X_NAS_PORT_2_ISID(nas_port)                       (((nas_port - NAS_PORT_NO_START) / VTSS_PORTS) + VTSS_ISID_START)
#define DOT1X_NAS_PORT_2_SWITCH_API_PORT(nas_port)            (((nas_port - NAS_PORT_NO_START) % VTSS_PORTS) + VTSS_PORT_NO_START)
#define DOT1X_ISID_SWITCH_API_PORT_2_NAS_PORT(isid, api_port) ((((isid) - VTSS_ISID_START) * VTSS_PORTS) + ((api_port) - VTSS_PORT_NO_START + NAS_PORT_NO_START))

void dot1x_crit_enter(void);
void dot1x_crit_exit(void);
void dot1x_crit_assert_locked(void);
void dot1x_disable_due_to_soon_boot(void);
BOOL dot1x_glbl_enabled(void);

#endif /* _DOT1X_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
