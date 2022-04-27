/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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
/* debug msg can be enabled by cmd "debug trace module level ipc default debug" */

#ifndef _VTSS_DUALCPU_APPL_H_
#define _VTSS_DUALCPU_APPL_H_

#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */
#include "dualcpu_api.h"

#define IOERROR(...) printf(__VA_ARGS__)
#if 0
#define IOTRACE(...) printf(__VA_ARGS__)
#else
#define IOTRACE(...)
#endif

#define RMT_RD(io)        remote_read         (&(io))
#define RMT_WR(io, v)     remote_write        (&(io), v)
#define RMT_WRM(io, v, m) remote_write_masked (&(io), v, m)
#define RMT_CLR(io, m)    remote_write_masked (&(io), 0, m)
#define RMT_SET(io, m)    remote_write_masked (&(io), m, m)

u32  remote_read        (volatile u32 *addr);
void remote_write       (volatile u32 *addr, u32 val);
void remote_write_masked(volatile u32 *addr, u32 value, u32 mask);

void dualcpu_ddr_init(void);

#endif /* _VTSS_DUALCPU_APPL_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
