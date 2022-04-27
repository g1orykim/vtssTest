/*

 Vitesse Switch Application software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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
/*
******************************************************************************

    Revision history
    > CP.Wang, 2012/08/09 13:57
        - Create

******************************************************************************
*/
#ifndef __VTSS_FREE_LIST_H__
#define __VTSS_FREE_LIST_H__

/*
******************************************************************************

    Include File

******************************************************************************
*/

/*
******************************************************************************

    Constant and Macro

******************************************************************************
*/
/*
    for debug only
*/
#ifdef WIN32
#define DBG_PRINTF          printf
#else
#define DBG_PRINTF          (void)diag_printf
#endif

/* CP, 2012/08/08 16:17, for debugging only */
#if 1
#ifndef WIN32
#include <cyg/infra/diag.h>
#endif
#define _FLIST_DEBUG(args)  { DBG_PRINTF args; }
#else
#define _FLIST_DEBUG(args)
#endif

#define _FLIST_T_E(...)     _FLIST_DEBUG(("Error: %s, %s, %d, ", __FILE__, __FUNCTION__, __LINE__)); \
                            _FLIST_DEBUG((__VA_ARGS__));

/*
******************************************************************************

    Type Definition

******************************************************************************
*/

//****************************************************************************
#endif /* __VTSS_FREE_LIST_H__ */
