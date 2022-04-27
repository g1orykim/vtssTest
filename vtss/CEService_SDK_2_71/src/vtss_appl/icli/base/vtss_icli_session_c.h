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
    > CP.Wang, 05/29/2013 12:02
        - create for ICLI_INPUT_STYLE_SINGLE_LINE

==============================================================================
*/
#ifndef __VTSS_ICLI_SESSION_C_H__
#define __VTSS_ICLI_SESSION_C_H__
//****************************************************************************

/*
==============================================================================

    Include File

==============================================================================
*/

/*
==============================================================================

    Constant and Macro

==============================================================================
*/

/*
==============================================================================

    Type Definition

==============================================================================
*/

/*
==============================================================================

    Public Function

==============================================================================
*/
/*
    get command from usr input
    1. function key will be provided
    2. the command will be stored in handle->runtime_data.cmd

    INPUT
        handle : session handle

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_c_usr_cmd_get(
    IN  icli_session_handle_t   *handle
);

/*
    More page

    INPUT
        handle : session handle

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_c_line_mode(
    IN icli_session_handle_t    *handle
);

void vtss_icli_c_cmd_display(
    IN  icli_session_handle_t   *handle
);

void vtss_icli_c_cmd_redisplay(
    IN  icli_session_handle_t   *handle
);

void vtss_icli_c_line_clear(
    IN  icli_session_handle_t   *handle
);

//****************************************************************************
#endif //__VTSS_ICLI_SESSION_C_H__

