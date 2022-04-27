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
    > CP.Wang, 05/29/2013 12:00
        - create for ICLI_INPUT_STYLE_MULTIPLE_LINE

==============================================================================
*/
/*
==============================================================================

    Include File

==============================================================================
*/
#include "vtss_icli.h"

/*
==============================================================================

    Constant

==============================================================================
*/
#define _MORE_PROMPT            "-- more --, next page: Space, continue: g, quit: ^C"
#define _MORE_SIMPLE_PROMPT     "-- more --"

/*
==============================================================================

    Macro

==============================================================================
*/
#define _CMD_IS_EMPTY \
    ( handle->runtime_data.cmd_len == 0 )

#define _PRINT_CHAR(c) \
{ \
    (void)vtss_icli_sutil_usr_char_put(handle, (char)(c)); \
    _INC_1(handle->runtime_data.cursor_pos); \
}

/*
==============================================================================

    Type Definition

==============================================================================
*/

/*
==============================================================================

    Static Variable

==============================================================================
*/

/*
==============================================================================

    Static Function

==============================================================================
*/
#ifndef WIN32
/*
    if the last char is at the end of line,
    then the cursor will stay at that position,
    but will not wrap to the next line.
    so put a space to make it wrap.
*/
static void _auto_wrap(
    IN  icli_session_handle_t   *handle
)
{
    i32     x, y;

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

    if ( x == 0 ) {
        ICLI_PUT_SPACE;
        ICLI_PUT_BACKSPACE;
    }
}
#endif

/*
    display rest command from cmd_pos
    and update cursor_pos
*/
static void _display_rest_cmd(
    IN  icli_session_handle_t   *handle
)
{
    vtss_icli_session_str_put(handle, handle->runtime_data.cmd + handle->runtime_data.cmd_pos);
    handle->runtime_data.cursor_pos += (handle->runtime_data.cmd_len - handle->runtime_data.cmd_pos);

#ifndef WIN32
    _auto_wrap( handle );
#endif
}

static void _goto_end(
    IN  icli_session_handle_t   *handle
)
{
    i32     x, y;
    i32     ex, ey;

    /* already at the end */
    if ( _CURSOR_AT_CMD_END ) {
        return;
    }

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

    // get end x,y
    vtss_icli_sutil_end_xy_get(handle, &ex, &ey);

    // go to the end
    icli_cursor_offset(handle, ex - x, ey - y);
}

static void _fkey_backspace(
    IN  icli_session_handle_t   *handle
)
{
    i32     i;
    i32     x, y;
    i32     nx, ny;

    /* already at the begin */
    if ( _CURSOR_AT_CMD_BEGIN ) {
        ICLI_PLAY_BELL;
        return;
    }

    /* update cmd */
    for ( i = handle->runtime_data.cmd_pos; i <= handle->runtime_data.cmd_len; ++i ) {
        handle->runtime_data.cmd[i - 1] = handle->runtime_data.cmd[i];
    }

    /* update cmd_pos and cmd_len */
    _DEC_1( handle->runtime_data.cmd_pos );
    _DEC_1( handle->runtime_data.cmd_len );

    if ( _CURSOR_AT_CMD_END ) {
        /* print rest command and move cursor back */
        icli_cursor_backspace( handle );
    } else {
        // move cursor backward
        icli_cursor_backward(handle);
        _DEC_1( handle->runtime_data.cursor_pos );

        // get orginal x,y
        vtss_icli_sutil_current_xy_get(handle, &x, &y);

        // display rest command
        _display_rest_cmd( handle );

        // display last space
        _PRINT_CHAR( ICLI_SPACE );

#ifndef WIN32
        _auto_wrap( handle );
#endif

        // get current x,y
        vtss_icli_sutil_current_xy_get(handle, &nx, &ny);

        // go back to the original x,y
        icli_cursor_offset(handle, x - nx, y - ny);

        // update cursor position
        handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos;
    }
}

static void _fkey_backward_char(
    IN  icli_session_handle_t   *handle
)
{
    /* already at the begin */
    if ( _CURSOR_AT_CMD_BEGIN ) {
        ICLI_PLAY_BELL;
        return;
    }
    icli_cursor_backward( handle );
    _DEC_1( handle->runtime_data.cursor_pos );
    _DEC_1( handle->runtime_data.cmd_pos );
}

static void _fkey_forward_char(
    IN  icli_session_handle_t   *handle
)
{
    /* already at the end */
    if ( _CURSOR_AT_CMD_END ) {
        ICLI_PLAY_BELL;
        return;
    }
    icli_cursor_forward( handle );
    _INC_1( handle->runtime_data.cmd_pos );
    _INC_1( handle->runtime_data.cursor_pos );
}

static void _fkey_goto_begin(
    IN  icli_session_handle_t   *handle
)
{
    i32     x, y;
    i32     nx, ny;

    /* already at the begin */
    if ( _CURSOR_AT_CMD_BEGIN ) {
        ICLI_PLAY_BELL;
        return;
    }

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

    // get x,y of the command beginning
    nx = handle->runtime_data.prompt_len;
    ny = 0;

    // go back to the original x,y
    icli_cursor_offset(handle, nx - x, ny - y);

    // reset pos
    handle->runtime_data.cmd_pos = 0;
    handle->runtime_data.cursor_pos = 0;
}

static void _fkey_goto_end(
    IN  icli_session_handle_t   *handle
)
{
    /* already at the end */
    if ( _CURSOR_AT_CMD_END ) {
        ICLI_PLAY_BELL;
        return;
    }

    // go to end
    _goto_end( handle );

    // update pos
    handle->runtime_data.cmd_pos = handle->runtime_data.cmd_len;
    handle->runtime_data.cursor_pos = handle->runtime_data.cmd_len;
}

static void _fkey_del_char(
    IN  icli_session_handle_t   *handle
)
{
    i32     x, y;
    i32     nx, ny;
    i32     i;

    if ( _CURSOR_AT_CMD_END ) {
        ICLI_PLAY_BELL;
        return;
    }

    /* update cmd */
    for ( i = handle->runtime_data.cmd_pos; i < handle->runtime_data.cmd_len; ++i ) {
        handle->runtime_data.cmd[i] = handle->runtime_data.cmd[i + 1];
    }

    /* update cmd_pos and cmd_len */
    _DEC_1( handle->runtime_data.cmd_len );

    // get orginal x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

    // display rest command
    _display_rest_cmd( handle );

    // display last space
    _PRINT_CHAR( ICLI_SPACE );

#ifndef WIN32
    _auto_wrap( handle );
#endif

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &nx, &ny);

    // go back to the original x,y
    icli_cursor_offset(handle, x - nx, y - ny);

    // update cursor position
    handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos;
}

static void _fkey_del_word(
    IN  icli_session_handle_t   *handle
)
{
    /* already at the begin */
    if ( _CURSOR_AT_CMD_BEGIN ) {
        ICLI_PLAY_BELL;
        return;
    }

    /* delete all near spaces */
    while ( handle->runtime_data.cmd_pos > 0 && handle->runtime_data.cmd[handle->runtime_data.cmd_pos - 1] == ICLI_SPACE ) {
        _fkey_backspace( handle );
    }

    /* delete the near one word */
    while ( handle->runtime_data.cmd_pos > 0 && handle->runtime_data.cmd[handle->runtime_data.cmd_pos - 1] != ICLI_SPACE ) {
        _fkey_backspace( handle );
    }

    return;
}

static void _del_line(
    IN  icli_session_handle_t   *handle
)
{
    i32     x, y;
    i32     nx, ny;
    i32     i;

    if ( _CMD_IS_EMPTY ) {
        ICLI_PLAY_BELL;
        return;
    }

    _fkey_goto_begin( handle );

    // get original x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

    // erase command
    for ( i = 0; i < handle->runtime_data.cmd_len; ++i ) {
        _PRINT_CHAR( ICLI_SPACE );
    }

#ifndef WIN32
    _auto_wrap( handle );
#endif

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &nx, &ny);

    // go back to the original x,y
    icli_cursor_offset(handle, x - nx, y - ny);
}

static void _fkey_del_line(
    IN  icli_session_handle_t   *handle
)
{
    _del_line( handle );

    // reset command
    vtss_icli_sutil_cmd_reset( handle );
}

static void _fkey_del_to_begin(
    IN  icli_session_handle_t   *handle
)
{
    i32     cmd_pos, cmd_len;
    i32     x, y;
    char    *c;

    /* already at the begin */
    if ( _CURSOR_AT_CMD_BEGIN ) {
        ICLI_PLAY_BELL;
        return;
    }

    // keep original
    cmd_pos = handle->runtime_data.cmd_pos;
    cmd_len = handle->runtime_data.cmd_len;

    // delete all line
    _del_line( handle );

    // update cursor position
    handle->runtime_data.cursor_pos = 0;

    // print rest string
    vtss_icli_session_str_put(handle, handle->runtime_data.cmd + cmd_pos);

    // update cursor position
    handle->runtime_data.cursor_pos = (handle->runtime_data.cmd_len - cmd_pos);

#ifndef WIN32
    _auto_wrap( handle );
#endif

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

    // go back to the original x,y
    icli_cursor_offset(handle, handle->runtime_data.prompt_len - x, 0 - y);

    // update command
    (void)vtss_icli_str_cpy(handle->runtime_data.cmd, handle->runtime_data.cmd + cmd_pos);
    for ( c = handle->runtime_data.cmd + (cmd_len - cmd_pos + 1); ICLI_NOT_(EOS, *c); ++c ) {
        *c = ICLI_EOS;
    }

    // update pos
    handle->runtime_data.cmd_len = cmd_len - cmd_pos;
    handle->runtime_data.cmd_pos = 0;
    handle->runtime_data.cursor_pos = 0;
}

static void _fkey_del_to_end(
    IN  icli_session_handle_t   *handle
)
{
    i32     x, y;
    i32     nx, ny;
    i32     i;

    /* already at the end */
    if ( _CURSOR_AT_CMD_END ) {
        ICLI_PLAY_BELL;
        return;
    }

    // get original x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

    // erase command
    for ( i = handle->runtime_data.cmd_pos; i < handle->runtime_data.cmd_len; ++i ) {
        _PRINT_CHAR( ICLI_SPACE );
        handle->runtime_data.cmd[i] = ICLI_EOS;
    }

#ifndef WIN32
    _auto_wrap( handle );
#endif

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &nx, &ny);

    // go back to the original x,y
    icli_cursor_offset(handle, x - nx, y - ny);

    // update command length
    handle->runtime_data.cmd_len = handle->runtime_data.cmd_pos;

    // update cursor position
    handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos;
}

static void _fkey_redisplay(
    IN  icli_session_handle_t   *handle
)
{
    // go to end
    _goto_end( handle );

    // new line for next command line
    ICLI_PUT_NEWLINE;
    handle->runtime_data.exec_type = ICLI_EXEC_TYPE_REDISPLAY;
}

#if 1 /* CP, 08/13/2013 13:00, Bugzilla#12423 - show full command syntax */
static void _fkey_full_cmd(
    IN  icli_session_handle_t   *handle
)
{
    handle->runtime_data.exec_type = ICLI_EXEC_TYPE_FULL_CMD;
}
#endif

static void _fkey_prev_cmd(
    IN  icli_session_handle_t   *handle
)
{
    /* empty */
    if ( vtss_icli_sutil_history_cmd_empty(handle) ) {
        ICLI_PLAY_BELL;
        return;
    }

    /* alread at tail */
    if ( handle->runtime_data.history_cmd_pos == handle->runtime_data.history_cmd_head->next ) {
        ICLI_PLAY_BELL;
        return;
    }

    /* delete line */
    _fkey_del_line( handle );

    /* update history pos */
    vtss_icli_sutil_history_cmd_pos_prev( handle );

    /* get history command */
    (void)vtss_icli_str_cpy( handle->runtime_data.cmd, handle->runtime_data.history_cmd_pos->cmd);
    vtss_icli_session_str_put( handle, handle->runtime_data.cmd );
    handle->runtime_data.cmd_len = vtss_icli_str_len(handle->runtime_data.cmd);
    handle->runtime_data.cmd_pos = handle->runtime_data.cmd_len;
    handle->runtime_data.cursor_pos = handle->runtime_data.cmd_len;

#ifndef WIN32
    _auto_wrap( handle );
#endif
}

static void _fkey_next_cmd(
    IN  icli_session_handle_t   *handle
)
{
    /* delete line */
    _fkey_del_line( handle );

    /* empty */
    if ( vtss_icli_sutil_history_cmd_empty(handle) ) {
        ICLI_PLAY_BELL;
        return;
    }

    /* no next */
    if ( handle->runtime_data.history_cmd_pos == NULL ) {
        ICLI_PLAY_BELL;
        return;
    }

    /* at head */
    if ( handle->runtime_data.history_cmd_pos == handle->runtime_data.history_cmd_head ) {
        handle->runtime_data.history_cmd_pos = NULL;
        ICLI_PLAY_BELL;
        return;
    }

    /* update history pos */
    vtss_icli_sutil_history_cmd_pos_next( handle );

    /* get history command */
    (void)vtss_icli_str_cpy( handle->runtime_data.cmd, handle->runtime_data.history_cmd_pos->cmd);
    vtss_icli_session_str_put( handle, handle->runtime_data.cmd );
    handle->runtime_data.cmd_len = vtss_icli_str_len(handle->runtime_data.cmd);
    handle->runtime_data.cmd_pos = handle->runtime_data.cmd_len;
    handle->runtime_data.cursor_pos = handle->runtime_data.cmd_len;

#ifndef WIN32
    _auto_wrap( handle );
#endif
}

static void _fkey_tab(
    IN  icli_session_handle_t   *handle
)
{
    handle->runtime_data.exec_type = ICLI_EXEC_TYPE_TAB;
}

static void _fkey_question(
    IN  icli_session_handle_t   *handle
)
{
#if 1 /* CP, 08/22/2013 14:55, Bugzilla#12494 - double '?' to display full syntax */
    if ( handle->runtime_data.exec_type == ICLI_EXEC_TYPE_QUESTION ) {
        _fkey_full_cmd( handle );
    } else {
        handle->runtime_data.exec_type = ICLI_EXEC_TYPE_QUESTION;
    }
#else
    handle->runtime_data.exec_type = ICLI_EXEC_TYPE_QUESTION;
#endif
}

static void _cmd_put(
    IN  icli_session_handle_t   *handle,
    IN  i32                     c
)
{
    i32     x, y;
    i32     nx, ny;
    BOOL    b_buf;

    /* add char */
    if ( vtss_icli_sutil_cmd_char_add(handle, c) == FALSE ) {
        return;
    }

    /* check if any char already in buffer */
    if ( vtss_icli_sutil_usr_char_get(handle, _BUF_WAIT_TIME, &c) ) {
        b_buf = TRUE;
        while ( ICLI_NOT_(KEY_ENTER, c) && ICLI_NOT_(NEWLINE, c)) {
            (void)vtss_icli_sutil_cmd_char_add(handle, c);
            if ( vtss_icli_sutil_usr_char_get(handle, _BUF_WAIT_TIME, &c) == FALSE ) {
                /* no char in buffer */
                b_buf = FALSE;
                break;
            }
        }
        /* ENTER or NEW LINE, then buffer it */
        if ( b_buf ) {
            handle->runtime_data.buffer_char = c;
        }
        // display command from cursor_pos
        vtss_icli_session_str_put(handle, handle->runtime_data.cmd + handle->runtime_data.cursor_pos);
        // update cursor_pos
        handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos;
        // update cursor
        if ( ! _CURSOR_AT_CMD_END ) {
            // get orginal x,y
            vtss_icli_sutil_current_xy_get(handle, &x, &y);
            // get end x,y
            vtss_icli_sutil_end_xy_get(handle, &nx, &ny);
            // go back to the original x,y
            icli_cursor_offset(handle, x - nx, y - ny);
        }
    } else {
        /* update cursor position */
        _INC_1( handle->runtime_data.cursor_pos );

        /* print the input char */
        (void)vtss_icli_sutil_usr_char_put(handle, (char)c);

        /* print rest command and move cursor back */
        if ( _CURSOR_AT_CMD_END ) {
#ifndef WIN32
            _auto_wrap( handle );
#endif
        } else {
            // get orginal x,y
            vtss_icli_sutil_current_xy_get(handle, &x, &y);

            // display rest command
            _display_rest_cmd( handle );

            // get current x,y
            vtss_icli_sutil_current_xy_get(handle, &nx, &ny);

            // go back to the original x,y
            icli_cursor_offset(handle, x - nx, y - ny);

            // update cursor position
            handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos;
        }
    }
}

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
i32 vtss_icli_a_usr_cmd_get(
    IN  icli_session_handle_t   *handle
)
{
    i32     c = 0,
            loop;

    /* clean buffer */
    handle->runtime_data.buffer_char = 0;

    /* start loop */
    loop = 1;

#if 1 /* CP, 08/22/2013 14:55, Bugzilla#12494 - double '?' to display full syntax */
    while ( loop ) {
        if ( loop == 1 ) {
            ++loop;
        } else {
            handle->runtime_data.exec_type = ICLI_EXEC_TYPE_CMD;
        }
#else
    while ( loop == 1 ) {
#endif

        /* get char */
        if ( handle->runtime_data.buffer_char ) {
            c = handle->runtime_data.buffer_char;
        } else {
            if ( vtss_icli_sutil_usr_char_get_by_session(handle, &c) == FALSE ) {
                ICLI_PLAY_BELL;
                return ICLI_RC_ERR_EXPIRED;
            }
        }

        /*
            avoid the cases of CR+LF or LF+CR
        */
        switch (c) {
        case ICLI_KEY_ENTER:
            if ( handle->runtime_data.prev_char == ICLI_NEWLINE ) {
                // reset previous input char
                handle->runtime_data.prev_char = 0;
                continue;
            }
            break;

        case ICLI_NEWLINE:
            if ( handle->runtime_data.prev_char == ICLI_KEY_ENTER ) {
                // reset previous input char
                handle->runtime_data.prev_char = 0;
                continue;
            }
            break;
        }

        /* get previous input char */
        handle->runtime_data.prev_char = c;

        /* reset TAB */
        if ( ICLI_NOT_(KEY_TAB, c) ) {
            vtss_icli_sutil_tab_reset( handle );
        }

        /* process input char */
        switch (c) {
        case ICLI_KEY_FUNCTION0:
            /*
                function key is not supported
                consume next char
            */
            (void)vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c);
            ICLI_PLAY_BELL;
            continue;

        case ICLI_KEY_FUNCTION1:
            if ( vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c) == FALSE ) {
                /* invalid function key, skip it */
                continue;
            }

            switch ( c ) {
            case ICLI_FKEY1_UP:
                _fkey_prev_cmd( handle );
                break;

            case ICLI_FKEY1_DOWN:
                _fkey_next_cmd( handle );
                break;

            case ICLI_FKEY1_LEFT:
                _fkey_backward_char( handle );
                break;

            case ICLI_FKEY1_RIGHT:
                _fkey_forward_char( handle );
                break;

            case ICLI_FKEY1_HOME:
                _fkey_goto_begin( handle );
                break;

            case ICLI_FKEY1_END:
                _fkey_goto_end( handle );
                break;

            case ICLI_FKEY1_DEL:
                _fkey_del_char( handle );
                break;

            default:
                ICLI_PLAY_BELL;
                break;
            }
            continue;

        case ICLI_HYPER_TERMINAL:
            /* case ICLI_KEY_ESC: */

            /* get HT_BEGIN */
            if ( vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c) == FALSE ) {
                /* ICLI_KEY_ESC */
                _fkey_del_line( handle );
                continue;
            }

            if ( ICLI_NOT_(HT_BEGIN, c) ) {
                continue;
            }

            /* get VT100 keys */
            if ( vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c) == FALSE ) {
                continue;
            }

            switch ( c ) {
            case ICLI_VKEY_UP:
                _fkey_prev_cmd( handle );
                break;

            case ICLI_VKEY_DOWN:
                _fkey_next_cmd( handle );
                break;

            case ICLI_VKEY_LEFT:
                _fkey_backward_char( handle );
                break;

            case ICLI_VKEY_RIGHT:
                _fkey_forward_char( handle );
                break;

            case ICLI_VKEY_HOME:
                //get next c
                if ( vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c) ) {
                    if ( ICLI_NOT_(HT_END, c) ) {
                        /* it's a function key, consume HT_END */
                        (void)vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c);
                        break;
                    }
                    /* HOME key */
                }

                //goto begin
                _fkey_goto_begin( handle );
                break;

            case ICLI_VKEY_DEL:
                //consume HT_END
                (void)vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c);
                //delete a char
                _fkey_del_char( handle );
                break;

            case ICLI_VKEY_END:
                //consume HT_END
                (void)vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c);
                //goto end
                _fkey_goto_end( handle );
                break;

            case ICLI_VKEY_INSERT:
            case ICLI_VKEY_PGUP:
            case ICLI_VKEY_PGDOWN:
                //consume HT_END
                (void)vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c);
                break;

            default:
                ICLI_PLAY_BELL;
                break;
            }
            continue;

        case ICLI_KEY_CTRL_('A'):
            _fkey_goto_begin( handle );
            continue;

        case ICLI_KEY_CTRL_('B'):
            _fkey_backward_char( handle );
            continue;

        case ICLI_KEY_CTRL_('D'):
            _fkey_del_char( handle );
            continue;

        case ICLI_KEY_CTRL_('E'):
            _fkey_goto_end( handle );
            continue;

        case ICLI_KEY_CTRL_('F'):
            _fkey_forward_char( handle );
            continue;

        case ICLI_KEY_CTRL_('K'):
            _fkey_del_to_end( handle );
            continue;

        case ICLI_KEY_CTRL_('L'):
            _fkey_redisplay( handle );
            return ICLI_RC_OK;

        case ICLI_KEY_CTRL_('N'):
            _fkey_del_line( handle );
            continue;

        case ICLI_KEY_CTRL_('P'):
            _fkey_prev_cmd( handle );
            continue;

#if 1 /* CP, 08/13/2013 13:00, Bugzilla#12423 - show full command syntax */
        case ICLI_KEY_CTRL_('Q'):
            // print ^Q
            (void)vtss_icli_sutil_usr_char_put(handle, '^');
            (void)vtss_icli_sutil_usr_char_put(handle, 'Q');
            // go to end
            _goto_end( handle );
            ICLI_PUT_NEWLINE;
            _fkey_full_cmd( handle );
            return ICLI_RC_OK;
#endif

        case ICLI_KEY_CTRL_('R'):
            _fkey_redisplay( handle );
            return ICLI_RC_OK;

        case ICLI_KEY_CTRL_('U'):
            _fkey_del_to_begin( handle );
            continue;

        case ICLI_KEY_CTRL_('W'):
            _fkey_del_word( handle );
            continue;

        case ICLI_KEY_CTRL_('X'):
            _fkey_del_to_begin( handle );
            continue;

        case ICLI_KEY_DEL:
            _fkey_del_char( handle );
            continue;

        case ICLI_KEY_BACKSPACE:
            /* case ICLI_KEY_CTRL_('H'): */
            _fkey_backspace( handle );
            continue;

        case ICLI_KEY_TAB: /* == Ctrl-I */
            _fkey_tab( handle );
            return ICLI_RC_OK;

        case ICLI_KEY_QUESTION:
            // print ?
            (void)vtss_icli_sutil_usr_char_put(handle, '?');
            // go to end
            _goto_end( handle );
            ICLI_PUT_NEWLINE;
            _fkey_question( handle );
            return ICLI_RC_OK;

        case ICLI_KEY_CTRL_('Z'):
            handle->runtime_data.after_cmd_act = ICLI_AFTER_CMD_ACT_GOTO_EXEC_MODE;
            (void)vtss_icli_sutil_usr_char_put(handle, '^');
            (void)vtss_icli_sutil_usr_char_put(handle, 'Z');

        //fall through

        case ICLI_KEY_ENTER:
        case ICLI_NEWLINE:
            if ( ! _CURSOR_AT_CMD_END ) {
                _fkey_goto_end( handle );
            }
            ICLI_PUT_NEWLINE;

            if ( handle->runtime_data.cmd_len ) {
                handle->runtime_data.exec_type = ICLI_EXEC_TYPE_CMD;
                handle->runtime_data.cmd[handle->runtime_data.cmd_len] = 0;
                return ICLI_RC_OK;
            } else {
                return ICLI_RC_ERR_EMPTY;
            }

        default :
            break;
        }

        /* put into cmd */
        _cmd_put(handle, c);

    }/* while (1) */

    // for lint
    return ICLI_RC_OK;

}/* vtss_icli_c_usr_cmd_get */

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
i32 vtss_icli_a_line_mode(
    IN icli_session_handle_t    *handle
)
{
    i32     c;

    if ( handle == NULL ) {
        T_E("handle == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    /* print prompt */
    if ( vtss_icli_str_len(_MORE_PROMPT) < handle->runtime_data.width - 2 ) {
        handle->runtime_data.put_str = _MORE_PROMPT;
    } else {
        handle->runtime_data.put_str = _MORE_SIMPLE_PROMPT;
    }
    (void)vtss_icli_sutil_usr_str_put( handle );

    /* get usr input */
    if ( vtss_icli_sutil_usr_char_get_by_session(handle, &c) == FALSE ) {
        handle->runtime_data.alive = FALSE;
        return ICLI_RC_ERR_EXPIRED;
    }

    /* put here to avoid ICLI_LINE_MODE_BYPASS */
    vtss_icli_sutil_more_prompt_clear( handle );

    switch ( c ) {
    /* flooding */
    case 'g':
        handle->runtime_data.line_cnt  = 0;
        handle->runtime_data.line_mode = ICLI_LINE_MODE_FLOOD;
        break;

        /* bypass */
#if 0
    case ICLI_KEY_ESC:
        /* flood all function keys */
        while ( vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c) ) {
            ;
        }

        //fall through
#endif
    case 'q':
    case ICLI_KEY_CTRL_('C'):
        handle->runtime_data.line_mode = ICLI_LINE_MODE_BYPASS;
        break;

    /* next page */
    case ICLI_KEY_SPACE:
    default:
        handle->runtime_data.line_mode = ICLI_LINE_MODE_PAGE;
        // it is already in display more mode and there is not command line
        // so one more line can be displayed
        handle->runtime_data.line_cnt  = -1;
        break;

    /* next line */
    case ICLI_KEY_ENTER:
    case ICLI_NEWLINE:
        handle->runtime_data.line_cnt = ICLI_MORE_ENTER;
        break;
    }
    return ICLI_RC_OK;
}

void vtss_icli_a_cmd_display(
    IN  icli_session_handle_t   *handle
)
{
    vtss_icli_session_str_put(handle, handle->runtime_data.cmd);
    handle->runtime_data.cmd_len = vtss_icli_str_len( handle->runtime_data.cmd );
    handle->runtime_data.cmd_pos = handle->runtime_data.cmd_len;
    handle->runtime_data.cursor_pos = handle->runtime_data.cmd_len;
}

void vtss_icli_a_cmd_redisplay(
    IN  icli_session_handle_t   *handle
)
{
    i32     x, y;
    i32     ex, ey;

    // get original x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

    // display command
    vtss_icli_session_str_put(handle, handle->runtime_data.cmd);

    // set currnet cursor pos
    handle->runtime_data.cursor_pos = handle->runtime_data.cmd_len;

#ifndef WIN32
    _auto_wrap( handle );
#endif

    // get cursor position
    vtss_icli_sutil_end_xy_get(handle, &ex, &ey);

    // go back to original x,y
    icli_cursor_offset(handle, x - ex, y - ey);

    // restore original cursor pos
    handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos;
}

void vtss_icli_a_line_clear(
    IN  icli_session_handle_t   *handle
)
{
    _del_line( handle );
}
