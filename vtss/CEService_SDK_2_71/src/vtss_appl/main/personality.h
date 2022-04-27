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

#ifndef _VTSS_PERSONALITY_H_
#define _VTSS_PERSONALITY_H_

#define VTSS_SWITCH 1 /* Vitesse turnkey SW */

#if defined(VTSS_PERSONALITY_MANAGED)
#define VTSS_SWITCH_MANAGED   1
#define VTSS_SWITCH_UNMANAGED 0
#else
#define VTSS_SWITCH_MANAGED   0
#define VTSS_SWITCH_UNMANAGED 1
#endif

#if defined(VTSS_PERSONALITY_STACKABLE)
#define VTSS_SWITCH_STACKABLE  1
#define VTSS_SWITCH_STANDALONE 0
#define VTSS_STACK_TYPE	       "stackable"
#else
#define VTSS_SWITCH_STACKABLE  0
#define VTSS_SWITCH_STANDALONE 1
#define VTSS_STACK_TYPE	       "standalone"
#endif


// Define a default production name in case that it isn't define by the make system
#ifdef VTSS_PRODUCT_NAME
#else
#define VTSS_PRODUCT_NAME "E-Stax-34"
#endif

/* LINTLIBRARY */

#ifndef __ASSEMBLER__

/*
 * Inline Personality Functions
 */

static __inline__ int vtss_switch_unmgd(void) __attribute__ ((const));
static __inline__ int vtss_switch_unmgd(void)
{
    return VTSS_SWITCH_UNMANAGED;
}

static __inline__ int vtss_switch_mgd(void) __attribute__ ((const));
static __inline__ int vtss_switch_mgd(void)
{
    return VTSS_SWITCH_MANAGED;
}

static __inline__ int vtss_switch_stackable(void) __attribute__ ((const));
static __inline__ int vtss_switch_stackable(void)
{
    return VTSS_SWITCH_STACKABLE;
}

static __inline__ int vtss_switch_standalone(void) __attribute__ ((const));
static __inline__ int vtss_switch_standalone(void)
{
    return VTSS_SWITCH_STANDALONE;
}

#endif  /* __ASSEMBLER__ */

#endif /* _VTSS_PERSONALITY_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
