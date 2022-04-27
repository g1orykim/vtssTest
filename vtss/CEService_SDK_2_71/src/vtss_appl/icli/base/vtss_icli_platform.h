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
#ifndef __VTSS_ICLI_PLATFORM_H__
#define __VTSS_ICLI_PLATFORM_H__
//****************************************************************************
/*
==============================================================================

    Include File

==============================================================================
*/
#include "icli_porting_trace.h"
#include "icli_os.h"

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
    BOOL    b_master;
    u32     usid;
} icli_switch_info_t;

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
/*
    take semaphore
*/
void icli_sema_take(
    void
);

/*
    give semaphore
*/
void icli_sema_give(
    void
);

/*
    calculate MD5
*/
void icli_hmac_md5(
    IN  const unsigned char *key,
    IN  size_t              key_len,
    IN  const unsigned char *data,
    IN  size_t              data_len,
    OUT unsigned char       *mac
);

/*
    cursor up
*/
void icli_cursor_up(
    IN icli_session_handle_t    *handle
);

/*
    cursor down
*/
void icli_cursor_down(
    IN icli_session_handle_t    *handle
);

/*
    cursor forward
*/
void icli_cursor_forward(
    IN icli_session_handle_t    *handle
);

/*
    cursor backward
*/
void icli_cursor_backward(
    IN icli_session_handle_t    *handle
);

/*
    cursor go to ( current_x + offser_x, current_y + offset_y )
*/
void icli_cursor_offset(
    IN icli_session_handle_t    *handle,
    IN i32                      offset_x,
    IN i32                      offset_y
);

/*
    cursor backward and delete that char
    update cursor_pos, but not cmd_pos and cmd_len
*/
void icli_cursor_backspace(
    IN  icli_session_handle_t   *handle
);

/*
    get switch information
*/
void icli_switch_info_get(
    OUT icli_switch_info_t  *switch_info
);

/*
    check if auth callback exists

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        TRUE  - yes, the callback exists
        FALSE - no, it is NULL

*/
BOOL icli_has_user_auth(
    void
);

/*
    authenticate user

    INPUT
        session_way  : way to access session
        username     : user name
        password     : password for the user

    OUTPUT
        privilege    : the privilege level for the authenticated user

    RETURN
        icli_rc_t
*/
i32 icli_user_auth(
    IN  icli_session_way_t  session_way,
    IN  char                *username,
    IN  char                *password,
    OUT i32                 *privilege
);

u16 icli_isid2usid(
    IN u16  isid
);

u16 icli_iport2uport(
    IN u16  iport
);

u16 icli_usid2switchid(
    IN u16  usid
);

BOOL icli_is_master(
    void
);

/*
    get mode prompt

    INPUT
        mode : command mode

    OUTPUT
        n/a

    RETURN
        mode prompt
*/
char *icli_mode_prompt_get(
    IN  icli_cmd_mode_t     mode
);

//****************************************************************************
#endif //__VTSS_ICLI_PLATFORM_H__

