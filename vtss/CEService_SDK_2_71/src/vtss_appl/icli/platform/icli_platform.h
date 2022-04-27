/*

 Vitesse Switch Application software.

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

/*
==============================================================================

    Revision history
    > CP.Wang, 05/29/2013 14:18
        - create

==============================================================================
*/
#ifndef __ICLI_PLATFORM_H__
#define __ICLI_PLATFORM_H__
//****************************************************************************

/*
==============================================================================

    Constant

==============================================================================
*/

/*
==============================================================================

    Type

==============================================================================
*/
typedef struct {
    icli_cmd_mode_t     mode;
    char                *str;
    char                *desc;
    char                *prompt;
    char                *cmd;
    BOOL                b_mode_var;
} icli_cmd_mode_info_t;

/*
==============================================================================

    Macro Definition

==============================================================================
*/

/*
==============================================================================

    Public Function

==============================================================================
*/

//****************************************************************************
#endif //__ICLI_PLATFORM_H__

