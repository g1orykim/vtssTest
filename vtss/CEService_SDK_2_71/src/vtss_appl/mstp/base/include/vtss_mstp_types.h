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

#ifndef _VTSS_MSTP_TYPES_H_
#define _VTSS_MSTP_TYPES_H_

#if defined(__LINUX__)

#include <sys/types.h>
#include <stddef.h>             /* offsetof() */

/* Shorthand types from linux platform types */
typedef u_int8_t  u8;
typedef u_int16_t u16;
typedef unsigned long u32;
typedef u_int8_t  bool;

#elif defined(VTSS_OPSYS_ECOS)

/* Types part of platform code */
#include "main.h"

#else

#error "You must supply type definitions for your platform OS"

#endif

#if defined(_lint)
/* This is mostly for lint */
#define offsetof(s,m) ((size_t)(unsigned long)&(((s *)0)->m))
#endif /* offsetof() */

#if !defined(MSTP_ATTRIBUTE_PACKED)
#define MSTP_ATTRIBUTE_PACKED __attribute__((packed)) /* GCC defined */
#endif

#if !defined(FALSE)
#define FALSE ((bool) 0)
#endif
#if !defined(TRUE)
#define TRUE  ((bool) 1)
#endif
#define ARR_SZ(a) (sizeof(a)/sizeof(a[0]))

/* Ensure MD5 type compatibility */
#define md5_u8  u8
#define md5_u16 u16
#define md5_u32 u32

#endif /* _VTSS_MSTP_TYPES_H_ */
