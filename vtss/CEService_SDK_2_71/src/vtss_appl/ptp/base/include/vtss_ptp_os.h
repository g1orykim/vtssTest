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

#ifndef _VTSS_PTP_OS_H_
#define _VTSS_PTP_OS_H_


#if defined(__LINUX__)

#include <stdlib.h>
#include <stdio.h>              /* For snprintf() */
#include <string.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<time.h>
#include<sys/time.h>
#include<sys/timex.h>
#include<sys/socket.h>
#include<sys/select.h>
#include<sys/ioctl.h>
#include<arpa/inet.h>
#include "vtss_ptp_types.h"

#include<endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define VTSS_BIG_ENDIAN
#endif




#define vtss_ptp_gettimeofday(x,y) gettimeofday(x,y)

typedef enum
{
    PTP_TRACE_RACKET,
    PTP_TRACE_NOISE,
    PTP_TRACE_DEBUG,
    PTP_TRACE_INFO,
    PTP_TRACE_WARNING,
    PTP_TRACE_ERROR,
} ptp_errlevel_t;

#ifndef S_SPLINT_S
#define T_E(fmt, ...) ptp_trace(PTP_TRACE_ERROR,   __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define T_W(fmt, ...) ptp_trace(PTP_TRACE_WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define T_I(fmt, ...) ptp_trace(PTP_TRACE_INFO,    __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define T_D(fmt, ...) ptp_trace(PTP_TRACE_DEBUG,   __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define T_N(fmt, ...) ptp_trace(PTP_TRACE_NOISE,   __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define T_R(fmt, ...) ptp_trace(PTP_TRACE_RACKET,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)

void ptp_trace(ptp_errlevel_t lvl, const char *location, int line_no, const char *fmt, ...)
__attribute__ ((format (printf, 4, 5)));
#else

#define T_E dummy_trace
#define T_W dummy_trace
#define T_I dummy_trace
#define T_D dummy_trace
#define T_N dummy_trace
#define T_R dummy_trace

void /*null*/ dummy_trace(const char *fmt, ...)
__attribute__ ((format (printf, 1, 2)));

#endif

#define VTSS_ABORT()   abort()
#define VTSS_ASSERT(x) do { if(!(x)) { T_E("Assertion failed: %s", #x); VTSS_ABORT(); } } while(0)

#elif defined(VTSS_OPSYS_ECOS)

#include <stdlib.h>             /* abort() */
#include <stdio.h>              /* For snprintf() */
#include <string.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_PTP

#include "vtss_trace_lvl_api.h"
#include "vtss_trace_api.h"
#include "vtss_os.h"

#define VTSS_ABORT() abort()


#else

#error "You must supply os definitions for your platform OS"

#endif

/* bit array manipulation */
#define getFlag(x,y)  (((x) & (y)) != 0)
#define setFlag(x,y)    ((x) |= (y))
#define clearFlag(x,y)  ((x) &= ~(y))

u16 vtss_ptp_get_rand(u32 *seed);
void *vtss_ptp_malloc(size_t sz);
void *vtss_ptp_calloc(size_t nmemb, size_t sz);
void  vtss_ptp_free(void *ptr);

#endif /* _VTSS_PTP_OS_H_ */
