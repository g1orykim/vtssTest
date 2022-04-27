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
    > CP.Wang, 05/29/2013 12:04
        - create

==============================================================================
*/
#ifndef __VTSS_ICLI_SESSION_UTIL_H__
#define __VTSS_ICLI_SESSION_UTIL_H__
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
#define _INC_1(n)           ++( n )
#define _DEC_1(n)           --( n )

#define _SCROLL_FACTOR      8

#define _FKEY_WAIT_TIME     1000 //milli-seconds, because SSH needs more time

#if 1
/*
    No wait,
    but this mechanism will not be started on Telent/SSH
    because they are slow
*/
#define _BUF_WAIT_TIME      ICLI_TIMEOUT_NO_WAIT
#else
/*
    wait 10 msec,
    this works on Telnet/SSH
    but when user keep pressing a key
    the display will stop until release the key
*/
#define _BUF_WAIT_TIME      10   //milli-seconds
#endif

#define ICLI_HISTORY_MAX_CNT    ( handle->config_data->history_size )

#define _CMD_LINE_LEN \
    (i32)( handle->runtime_data.width - handle->runtime_data.prompt_len - 1 )

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
icli_history_cmd_t *vtss_icli_sutil_history_cmd_alloc(
    IN  icli_session_handle_t   *handle
);

void vtss_icli_sutil_history_cmd_free(
    IN  icli_session_handle_t   *handle,
    IN  icli_history_cmd_t      *hcmd
);

void vtss_icli_sutil_history_cmd_free_n(
    IN  icli_session_handle_t   *handle,
    IN  u32                     n
);

BOOL vtss_icli_sutil_history_cmd_full(
    IN  icli_session_handle_t   *handle
);

BOOL vtss_icli_sutil_history_cmd_empty(
    IN  icli_session_handle_t   *handle
);

u32 vtss_icli_sutil_history_cmd_cnt(
    IN  icli_session_handle_t   *handle
);

void vtss_icli_sutil_history_cmd_pos_prev(
    IN  icli_session_handle_t   *handle
);

void vtss_icli_sutil_history_cmd_pos_next(
    IN  icli_session_handle_t   *handle
);

void vtss_icli_sutil_cmd_reset(
    IN  icli_session_handle_t   *handle
);

/*
    get session input by char

    INPUT
        handle    - session handle
        wait_time - in millisecond
                    = 0 - no wait
                    < 0 - forever

    OUTPUT
        c - user input char

    RETURN
        TRUE  - successful
        FALSE - timeout
*/
BOOL vtss_icli_sutil_usr_char_get(
    IN  icli_session_handle_t   *handle,
    IN  i32                     wait_time,
    OUT i32                     *c
);

/*
    get user input char according to session wait time

    INPUT
        handle - session handle
            handle->wait_time - in millisecond
                                = 0 - forever

    OUTPUT
        c - user input char

    RETURN
        TRUE  - successful
        FALSE - timeout
*/
BOOL vtss_icli_sutil_usr_char_get_by_session(
    IN  icli_session_handle_t   *handle,
    OUT i32                     *c
);

/*
    output a char on session
*/
BOOL vtss_icli_sutil_usr_char_put(
    IN  icli_session_handle_t   *handle,
    IN  char                    c
);

/*
    output string on session
*/
BOOL vtss_icli_sutil_usr_str_put(
    IN  icli_session_handle_t   *handle
);

void vtss_icli_sutil_tab_reset(
    IN  icli_session_handle_t   *handle
);

void vtss_icli_sutil_more_prompt_clear(
    IN icli_session_handle_t    *handle
);

BOOL vtss_icli_sutil_cmd_char_add(
    IN  icli_session_handle_t   *handle,
    IN  i32                     c
);

void vtss_icli_sutil_current_xy_get(
    IN  icli_session_handle_t   *handle,
    OUT i32                     *x,
    OUT i32                     *y
);

void vtss_icli_sutil_end_xy_get(
    IN  icli_session_handle_t   *handle,
    OUT i32                     *x,
    OUT i32                     *y
);

//****************************************************************************
#endif //__VTSS_ICLI_SESSION_UTIL_H__

