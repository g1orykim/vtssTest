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

#ifndef _VTSS_TRACE_LVL_API_H_
#define _VTSS_TRACE_LVL_API_H_

/* Named trace levels */
/* Note: If adding more trace levels, support must be added in cli.c */
#define VTSS_TRACE_LVL_NONE    10 /* No trace                                         */
#define VTSS_TRACE_LVL_ERROR    9 /* Code error encountered                           */
#define VTSS_TRACE_LVL_WARNING  8 /* Potential code error, manual inspection required */
#define VTSS_TRACE_LVL_INFO     6 /* Useful information                               */
#define VTSS_TRACE_LVL_DEBUG    4 /* Some debug information                           */
#define VTSS_TRACE_LVL_NOISE    2 /* Lot's of debug information                       */
#define VTSS_TRACE_LVL_RACKET   1 /* Even more ...                                    */

#define VTSS_TRACE_LVL_ALL      0 /* Enable all trace                                 */

/* Max string length of trace level */
#define VTSS_TRACE_MAX_LVL_LEN 7


#endif /* _VTSS_TRACE_LVL_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
