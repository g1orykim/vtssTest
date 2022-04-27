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
 
 $Id$
 $Revision$

*/

#ifndef _VTSS_TRACE_H_
#define _VTSS_TRACE_H_

/* Check defines */
#if !defined(VTSS_SWITCH) 
#define VTSS_SWITCH 0
#elif (VTSS_SWITCH != 0 && VTSS_SWITCH != 1)
#error VTSS_SWITCH must be set to 0 or 1
#endif

#if !defined(VTSS_TRACE_MULTI_THREAD) || (VTSS_TRACE_MULTI_THREAD != 0 && VTSS_TRACE_MULTI_THREAD != 1)
#error VTSS_TRACE_MULTI_THREAD not specified. Must be set to 0 or 1.
#endif

#include <stdio.h>
#include <string.h>

#include "vtss_trace_lvl_api.h"
#include "vtss_trace_api.h"
#if VTSS_SWITCH
#include "vtss_trace_vtss_switch_api.h"
#endif
#include "vtss_trace_io.h"

#if VTSS_TRACE_MULTI_THREAD
/* Semaphore for IO registrations */
extern vtss_os_sem_t trace_io_crit;
#if !VTSS_OPSYS_ECOS
extern vtss_os_sem_t trace_rb_crit;
#endif
#endif

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_TRACE

#define TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CNT     1

#include <vtss_trace_api.h>
/* ============== */


#ifndef TRACE_ASSERT
#define TRACE_ASSERT(expr, msg) { \
    if (!(expr)) { \
        T_E("ASSERTION FAILED"); \
        T_E msg; \
        VTSS_ASSERT(expr); \
    } \
}

#endif

#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)	((a) > (b) ? (b) : (a))

/* Max size of Error message (size of buffer used to hand the message to syslog_flash_log) */
/* Make it large enough! */
#define TRACE_ERR_BUF_SIZE 4096

#define VTSS_TRACE_REG_T_COOKIE 0xBABEFEED
#define VTSS_TRACE_GRP_T_COOKIE 0xCAFEBABE

#endif /* _VTSS_TRACE_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
