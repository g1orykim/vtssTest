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
#define _SCROLL_SIGN            '$'

/*
==============================================================================

    Macro

==============================================================================
*/
#define _CMD_LINE_MAX_LEN \
    (_CMD_LINE_LEN - 2)

// 1 is the space for cursor
#define _SCROLL_NEEDED \
    (handle->runtime_data.cmd_len > _CMD_LINE_MAX_LEN)

#define _CURSOR_AT_LINE_BEGIN \
    ((handle->runtime_data.left_scroll) ? \
     (handle->runtime_data.cursor_pos == 1) : \
     (handle->runtime_data.cursor_pos == 0))

#define _CURSOR_AT_LINE_ALMOST_BEGIN \
    ((handle->runtime_data.left_scroll) ? \
     (handle->runtime_data.cursor_pos <= 2) : \
     (handle->runtime_data.cursor_pos == 0))

#define _CURSOR_AT_LINE_END \
    ((handle->runtime_data.right_scroll) ? \
     (handle->runtime_data.cursor_pos == (_CMD_LINE_MAX_LEN - 2)) : \
     (handle->runtime_data.cursor_pos == (_CMD_LINE_MAX_LEN)))

#define _PRINT_CHAR(c) \
{ \
    (void)vtss_icli_sutil_usr_char_put(handle, (char)(c)); \
    _INC_1(handle->runtime_data.cursor_pos); \
}

#define _SCROLL_SIZE \
    (i32)((handle->runtime_data.width > _SCROLL_FACTOR) ? (handle->runtime_data.width / _SCROLL_FACTOR) : 1)

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
/*
    the remaining number of char's that can be displayed in the line.
*/
static i32  _remain_chars_get(
    IN  icli_session_handle_t   *handle
)
{
    return _CMD_LINE_MAX_LEN - handle->runtime_data.cursor_pos;
}

/*
    update cursor_pos, but not cmd_pos and cmd_len
*/
static void _backward(
    IN  icli_session_handle_t   *handle
)
{
    // go backward
    icli_cursor_backward( handle );

    /* decrease pos */
    _DEC_1(handle->runtime_data.cursor_pos);
}

/*
    del from cursor to line begin
    handle->runtime_data.cursor_pos : updated
    handle->runtime_data.cmd_pos    : not updated
*/
static void  _del_to_line_begin(
    IN  icli_session_handle_t   *handle
)
{
    i32     i, cursor_pos;

    if ( handle->runtime_data.cursor_pos == 0 ) {
        return;
    }

    cursor_pos = handle->runtime_data.cursor_pos;

    // go to the begin
    icli_cursor_offset(handle, 0 - cursor_pos, 0);

    // display space
    for ( i = cursor_pos; i > 0; --i ) {
        ICLI_PUT_SPACE;
    }

    // go back to the begin
    icli_cursor_offset(handle, 0 - cursor_pos, 0);

    // update cursor_pos
    handle->runtime_data.cursor_pos = 0;
}

/*
    del from cursor to line end
    handle->runtime_data.cursor_pos : not updated
    handle->runtime_data.cmd_pos    : not updated
*/
static void  _del_to_line_end(
    IN  icli_session_handle_t   *handle
)
{
    i32     i, j;
    i32     x, y;
    i32     nx, ny;
    i32     cursor_pos;

    // save cursor position
    cursor_pos = handle->runtime_data.cursor_pos;

    // get original x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

    if ( handle->runtime_data.right_scroll ) {
        j = _remain_chars_get( handle );
        for ( i = 0; i < j - 1; ++i ) {
            _PRINT_CHAR(ICLI_SPACE);
        }
        //for _SCROLL_SIGN
        _PRINT_CHAR(ICLI_SPACE);
    } else {
        for ( i = handle->runtime_data.cmd_pos; i < handle->runtime_data.cmd_len; ++i ) {
            _PRINT_CHAR(ICLI_SPACE);
        }
    }

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &nx, &ny);

    // go back to the original x,y
    icli_cursor_offset(handle, x - nx, y - ny);

    // restore cursor position
    handle->runtime_data.cursor_pos = cursor_pos;
}

/*
    del whole line
    handle->runtime_data.cursor_pos : update to be 0
    handle->runtime_data.cmd_pos    : not updated
*/
static void  _del_line(
    IN  icli_session_handle_t   *handle
)
{
    if ( handle->runtime_data.cmd_len == 0 ) {
        return;
    }

    _del_to_line_end( handle );
    _del_to_line_begin( handle );
}

/*
    display the command
    from_start_pos
    1. from the beginning of command line. (TRUE)
       cmd_pos and cursor_pos will be updated accordingly.
    2. from current position. (FALSE)
       cmd_pos and cursor_pos should not be updated.

*/
/*
    display user command according to the current settings, that is,
    start_pos, cmd_pos, cursor_pos

    INPUT
        from_start_pos : the session ID
            TRUE  - from the start position, start_pos.
                    That is from the beginning of command line.
            FALSE - from current position, (cmd_pos, cursor_pos).
                    cmd_pos and cursor_pos should not be updated.
        keep_pos : keep original cmd_pos and cursor_pos after displaying the command.
            TRUE  - keep the orginal pos
            FALSE - not keep, but update to the latest pos

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static void  _display_cmd(
    IN  icli_session_handle_t   *handle,
    IN  BOOL                    from_start_pos,
    IN  BOOL                    keep_pos
)
{
    i32     i,
            r,
            pos,
            cmd_pos = 0,
            cursor_pos = 0;
    char    *c;

    /* store position first */
    if ( keep_pos ) {
        cmd_pos    = handle->runtime_data.cmd_pos;
        cursor_pos = handle->runtime_data.cursor_pos;
    }

    if ( from_start_pos ) {
        /* display whole command */

        /* reset cursor_pos */
        handle->runtime_data.cursor_pos = 0;

        //check left scroll
        if ( handle->runtime_data.start_pos ) {
            handle->runtime_data.left_scroll = 1;
            _PRINT_CHAR( _SCROLL_SIGN );
        } else {
            handle->runtime_data.left_scroll = 0;
        }
        //get pos
        pos = handle->runtime_data.start_pos;
    } else {
        /* display rest command */
        if ( _CURSOR_AT_CMD_END ) {
            return;
        }
        //get pos
        pos = handle->runtime_data.cmd_pos;
    }

    //check right scroll
    r = _remain_chars_get( handle );
    if ( handle->runtime_data.cmd_len - pos > r ) {
        handle->runtime_data.right_scroll = 1;
        //decrease 1 for right scroll
        _DEC_1( r );
    } else {
        handle->runtime_data.right_scroll = 0;
        //get r
        r = handle->runtime_data.cmd_len - pos;
    }

    //display remaining cmd
    c = handle->runtime_data.cmd + pos;
    for ( i = 0; i < r; ++i ) {
        _PRINT_CHAR( *(c + i) );
    }

    //get cmd_pos
    if ( ! keep_pos ) {
        handle->runtime_data.cmd_pos = pos + i;
    }

    //process right scroll
    if ( handle->runtime_data.right_scroll ) {
        _PRINT_CHAR( _SCROLL_SIGN );

        // get back 2
        icli_cursor_backward( handle );
        icli_cursor_backward( handle );

        // update pos
        handle->runtime_data.cursor_pos -= 2;
        _DEC_1( handle->runtime_data.cmd_pos );
    }

    //restore cmd_pos and cursor_pos
    if ( keep_pos ) {
        // restore cmd_pos
        handle->runtime_data.cmd_pos = cmd_pos;

        // get back original cursor pos
        for ( i = handle->runtime_data.cursor_pos - cursor_pos; i > 0; --i ) {
            icli_cursor_backward( handle );
        }

        // restore cursor_pos
        handle->runtime_data.cursor_pos = cursor_pos;
    }
}

static BOOL _fkey_backspace(
    IN  icli_session_handle_t   *handle
)
{
    i32     i;

    /* already at the begin */
    if ( _CURSOR_AT_CMD_BEGIN ) {
        ICLI_PLAY_BELL;
        return TRUE;
    }

    /* update cmd */
    for ( i = handle->runtime_data.cmd_pos; i <= handle->runtime_data.cmd_len; ++i ) {
        handle->runtime_data.cmd[i - 1] = handle->runtime_data.cmd[i];
    }

    /* update cmd_pos and cmd_len */
    _DEC_1( handle->runtime_data.cmd_pos );
    _DEC_1( handle->runtime_data.cmd_len );

    if ( _CURSOR_AT_LINE_ALMOST_BEGIN ) {

        /* delete whole line */
        _del_line( handle );

        /* update start_pos */
        if ( handle->runtime_data.left_scroll ) {
            if (handle->runtime_data.start_pos < _SCROLL_SIZE) {
                handle->runtime_data.start_pos = 0;
            } else {
                if ( (handle->runtime_data.cmd_len - handle->runtime_data.cmd_pos) < _SCROLL_SIZE ) {
                    if ( handle->runtime_data.start_pos < (_CMD_LINE_MAX_LEN - _SCROLL_SIZE) ) {
                        handle->runtime_data.start_pos = 0;
                    } else {
                        handle->runtime_data.start_pos -= (_CMD_LINE_MAX_LEN - _SCROLL_SIZE);
                    }
                } else {
                    handle->runtime_data.start_pos -= _SCROLL_SIZE;
                }
            }
        }

        /* get cursor pos */
        if ( handle->runtime_data.start_pos ) {
            handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos - handle->runtime_data.start_pos + 1;
        } else {
            handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos;
        }

        /* display from begin */
        _display_cmd(handle, TRUE, TRUE);

    } else if ( _CURSOR_AT_CMD_END ) {

        /* backespace then done */
        icli_cursor_backspace( handle );

    } else {
        // go backward
        _backward( handle );

        /* update cursor_pos */
        _PRINT_CHAR( handle->runtime_data.cmd[handle->runtime_data.cmd_pos] );

        /* clear rest line */
        _del_to_line_end( handle );

        // go backward
        _backward( handle );

        /* display rest cmd */
        _display_cmd(handle, FALSE, TRUE);
    }

    return TRUE;
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

    if ( _CURSOR_AT_LINE_END ) {
        /* delete whole line */
        _del_line( handle );

        /* update start_pos */
        handle->runtime_data.start_pos += _SCROLL_SIZE;

        /* increase cmd position */
        _INC_1( handle->runtime_data.cmd_pos );

        /* get cursor position */
        handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos - handle->runtime_data.start_pos + 1;

        /* display cmd from begin */
        _display_cmd( handle, TRUE, TRUE );
    } else {
        // go forward
        icli_cursor_forward( handle );

        /* increase pos */
        _INC_1(handle->runtime_data.cursor_pos);
        _INC_1( handle->runtime_data.cmd_pos );
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

    if ( _CURSOR_AT_LINE_BEGIN ) {
        /* delete whole line */
        _del_line( handle );

        /* update start_pos */
        if ( handle->runtime_data.left_scroll ) {
            if (handle->runtime_data.start_pos < _SCROLL_SIZE) {
                handle->runtime_data.start_pos = 0;
            } else {
                handle->runtime_data.start_pos -= _SCROLL_SIZE;
            }
        }

        /* decrease position */
        _DEC_1( handle->runtime_data.cmd_pos );

        /* get cursor pos */
        if ( handle->runtime_data.start_pos ) {
            handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos - handle->runtime_data.start_pos + 1;
        } else {
            handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos;
        }

        /* display cmd from begin */
        _display_cmd( handle, TRUE, TRUE );
    } else {
        // go backward
        _backward( handle );

        /* decrease pos */
        _DEC_1( handle->runtime_data.cmd_pos );
    }
}

static void _fkey_goto_begin(
    IN  icli_session_handle_t   *handle
)
{
    /* already at the begin */
    if ( _CURSOR_AT_CMD_BEGIN ) {
        ICLI_PLAY_BELL;
        return;
    }

    if ( handle->runtime_data.left_scroll ) {
        /* delete whole line */
        _del_line( handle );

        handle->runtime_data.cmd_pos    = 0;
        handle->runtime_data.cursor_pos = 0;
        handle->runtime_data.start_pos  = 0;

        _display_cmd(handle, TRUE, TRUE);
    } else {
        /* go to the end */
        icli_cursor_offset(handle, 0 - handle->runtime_data.cursor_pos, 0);

        handle->runtime_data.cmd_pos    = 0;
        handle->runtime_data.cursor_pos = 0;
        handle->runtime_data.start_pos  = 0;
    }
}

static void _fkey_goto_end(
    IN  icli_session_handle_t   *handle
)
{
    i32     offset;

    /* already at the end */
    if ( _CURSOR_AT_CMD_END ) {
        ICLI_PLAY_BELL;
        return;
    }

    if ( handle->runtime_data.right_scroll ) {
        /* delete whole line */
        _del_line( handle );
        /* get start_pos */
        handle->runtime_data.start_pos = handle->runtime_data.cmd_len - (_CMD_LINE_MAX_LEN - _SCROLL_SIZE - 1);
        /* display command from start_pos */
        _display_cmd(handle, TRUE, FALSE);
    } else {
        offset = handle->runtime_data.cmd_len - handle->runtime_data.cmd_pos;
        /* go to the end */
        icli_cursor_offset(handle, offset, 0);
        handle->runtime_data.cmd_pos += offset;
        handle->runtime_data.cursor_pos += offset;
    }
}

static void _fkey_del_char(
    IN  icli_session_handle_t   *handle
)
{
    i32     i;

    /* already at the begin */
    if ( _CURSOR_AT_CMD_END ) {
        ICLI_PLAY_BELL;
        return;
    }

    /* update cmd */
    for ( i = handle->runtime_data.cmd_pos; i < handle->runtime_data.cmd_len; ++i ) {
        handle->runtime_data.cmd[i] = handle->runtime_data.cmd[i + 1];
    }

    /* clear rest line */
    _del_to_line_end( handle );

    /* update cmd_len only */
    _DEC_1( handle->runtime_data.cmd_len );

    /* display cmd */
    _display_cmd(handle, FALSE, TRUE);
}

static void _fkey_del_word(
    IN  icli_session_handle_t   *handle
)
{
    i32     cmd_pos, cursor_pos;
    char    *c;

    /* already at the begin */
    if ( _CURSOR_AT_CMD_BEGIN ) {
        ICLI_PLAY_BELL;
        return;
    }

    cmd_pos = handle->runtime_data.cmd_pos - 1;
    c = handle->runtime_data.cmd + cmd_pos;
    cursor_pos = handle->runtime_data.cursor_pos;

    /* delete whole line */
    _del_line( handle );

    /* find out where we should delete to, cmd_pos is the target position */

    // skip space
    for ( ; cmd_pos > 0 && ICLI_IS_(SPACE, *c); --cmd_pos, --c ) {
        ;
    }
    // skip word
    if ( ICLI_NOT_(SPACE, *c) ) {
        for ( ; cmd_pos > 0 && ICLI_NOT_(SPACE, *c); --cmd_pos, --c ) {
            ;
        }
        if ( ICLI_IS_(SPACE, *c) ) {
            ++c;
            ++cmd_pos;
        }
    }

    /* update cmd */
    (void)vtss_icli_str_cpy(handle->runtime_data.cmd + cmd_pos, handle->runtime_data.cmd + handle->runtime_data.cmd_pos);

    /* update cmd and length */
    handle->runtime_data.cmd_len -= (handle->runtime_data.cmd_pos - cmd_pos);
    for ( c = handle->runtime_data.cmd + handle->runtime_data.cmd_len + 1; ICLI_NOT_(EOS, *c); ++c ) {
        *c = ICLI_EOS;
    }

    /* update pos */
    cursor_pos -= (handle->runtime_data.cmd_pos - cmd_pos);

    if ( (handle->runtime_data.left_scroll) && (cursor_pos <= _SCROLL_FACTOR) ) {
        if ( cmd_pos > _SCROLL_FACTOR ) {
            handle->runtime_data.start_pos = cmd_pos - _SCROLL_FACTOR;
            handle->runtime_data.cursor_pos = cmd_pos - handle->runtime_data.start_pos + 1;
        } else {
            handle->runtime_data.start_pos = 0;
            handle->runtime_data.cursor_pos = cmd_pos;
        }
        handle->runtime_data.cmd_pos = cmd_pos;
    } else {
        handle->runtime_data.cmd_pos = cmd_pos;
        handle->runtime_data.cursor_pos = cursor_pos;
    }

    /* display from begin */
    _display_cmd(handle, TRUE, TRUE);
}

static void _fkey_del_line(
    IN  icli_session_handle_t   *handle
)
{
    /* delete whole line */
    _del_line( handle );

    //reset cmd
    vtss_icli_sutil_cmd_reset( handle );
}

static void _fkey_del_to_begin(
    IN  icli_session_handle_t   *handle
)
{
    i32     i, len;

    /* delete whole line */
    _del_line( handle );

    if ( _CURSOR_AT_CMD_END ) {
        //reset cmd
        vtss_icli_sutil_cmd_reset( handle );
    } else {
        //shift cmd
        len = handle->runtime_data.cmd_len - handle->runtime_data.cmd_pos;
        for ( i = 0; i < len; ++i ) {
            handle->runtime_data.cmd[i] = handle->runtime_data.cmd[handle->runtime_data.cmd_pos + i];
        }
        handle->runtime_data.cmd[i] = 0;

        //update data
        handle->runtime_data.cmd_len    = len;
        handle->runtime_data.cmd_pos    = 0;
        handle->runtime_data.start_pos  = 0;
        handle->runtime_data.cursor_pos = 0;

        //display cmd
        _display_cmd(handle, TRUE, TRUE);
    }
}

static void _fkey_del_to_end(
    IN  icli_session_handle_t   *handle
)
{
    i32     i, len;

    if ( _CURSOR_AT_CMD_END ) {
        ICLI_PLAY_BELL;
        return;
    }

    /* clear display to line end */
    _del_to_line_end( handle );

    //update cmd
    len = handle->runtime_data.cmd_len - handle->runtime_data.cmd_pos;
    for ( i = 0; i < len; ++i ) {
        handle->runtime_data.cmd[handle->runtime_data.cmd_pos + i] = 0;
    }

    //update length
    handle->runtime_data.cmd_len = handle->runtime_data.cmd_pos;
    handle->runtime_data.right_scroll = 0;
}

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

    /* delete command line */
    _fkey_del_line( handle );

    /* update history pos */
    vtss_icli_sutil_history_cmd_pos_prev( handle );

    /* get history command */
    (void)vtss_icli_str_cpy( handle->runtime_data.cmd, handle->runtime_data.history_cmd_pos->cmd );
    handle->runtime_data.cmd_len = vtss_icli_str_len(handle->runtime_data.cmd);
    handle->runtime_data.cmd_pos = handle->runtime_data.cmd_len;

    /* get start_pos */
    if ( _SCROLL_NEEDED ) {
        handle->runtime_data.start_pos = handle->runtime_data.cmd_len - (_CMD_LINE_MAX_LEN - _SCROLL_SIZE - 1);
    } else {
        handle->runtime_data.start_pos = 0;
    }

    /* display command from start_pos */
    _display_cmd(handle, TRUE, FALSE);
}

static void _fkey_next_cmd(
    IN  icli_session_handle_t   *handle
)
{
    /* delete command line */
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
    (void)vtss_icli_str_cpy( handle->runtime_data.cmd, handle->runtime_data.history_cmd_pos->cmd );
    handle->runtime_data.cmd_len = vtss_icli_str_len(handle->runtime_data.cmd);
    handle->runtime_data.cmd_pos = handle->runtime_data.cmd_len;

    /* get start_pos */
    if ( _SCROLL_NEEDED ) {
        handle->runtime_data.start_pos = handle->runtime_data.cmd_len - (_CMD_LINE_MAX_LEN - _SCROLL_SIZE - 1);
    } else {
        handle->runtime_data.start_pos = 0;
    }

    /* display command from start_pos */
    _display_cmd(handle, TRUE, FALSE);
}

static void _fkey_tab(
    IN  icli_session_handle_t   *handle
)
{
    handle->runtime_data.exec_type = ICLI_EXEC_TYPE_TAB;
}

static void _fkey_redisplay(
    IN  icli_session_handle_t   *handle
)
{
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
    i32     i;
    i32     cmd_len;
    BOOL    b_end;
    BOOL    b_buf;

    /* get original cmd len for later display */
    cmd_len = handle->runtime_data.cmd_len;
    b_end = _CURSOR_AT_CMD_END;

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
        /* display command */
        if ( b_end ) {
            if ( cmd_len ) {
                for ( i = handle->runtime_data.cursor_pos; i > 0; --i ) {
                    (void)vtss_icli_sutil_usr_char_put(handle, ICLI_BACKSPACE);
                }
                for ( i = handle->runtime_data.cursor_pos; i > 0; --i ) {
                    (void)vtss_icli_sutil_usr_char_put(handle, ICLI_SPACE);
                }
                for ( i = handle->runtime_data.cursor_pos; i > 0; --i ) {
                    (void)vtss_icli_sutil_usr_char_put(handle, ICLI_BACKSPACE);
                }
            }
            /* get start_pos */
            if ( _SCROLL_NEEDED ) {
                handle->runtime_data.start_pos = handle->runtime_data.cmd_len - (_CMD_LINE_MAX_LEN - _SCROLL_SIZE - 1);
            } else {
                handle->runtime_data.start_pos = 0;
            }
            /* display cmd from start position */
            _display_cmd( handle, TRUE, FALSE );
        } else {
            /* the cursor must be at cmd_pos */

            // go to begin of line
            for ( i = handle->runtime_data.cursor_pos; i > 0; --i ) {
                (void)vtss_icli_sutil_usr_char_put(handle, ICLI_BACKSPACE);
            }
            // erase all chars
            for ( i = _CMD_LINE_MAX_LEN; i > 0; --i ) {
                (void)vtss_icli_sutil_usr_char_put(handle, ICLI_SPACE);
            }
            // go back to the begin
            for ( i = _CMD_LINE_MAX_LEN; i > 0; --i ) {
                (void)vtss_icli_sutil_usr_char_put(handle, ICLI_BACKSPACE);
            }

            // display command
            if ( handle->runtime_data.cmd_len <= _CMD_LINE_MAX_LEN ||
                 handle->runtime_data.cmd_pos <= (_CMD_LINE_MAX_LEN - _SCROLL_SIZE) ) {
                handle->runtime_data.start_pos = 0;
                handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos;
            } else {
                if ( handle->runtime_data.cursor_pos < _SCROLL_SIZE ) {
                    handle->runtime_data.cursor_pos = _SCROLL_SIZE;
                } else if ( handle->runtime_data.cmd_pos > (_CMD_LINE_MAX_LEN - _SCROLL_SIZE) ) {
                    handle->runtime_data.cursor_pos = _CMD_LINE_MAX_LEN - _SCROLL_SIZE;
                }
                handle->runtime_data.start_pos = handle->runtime_data.cmd_pos - handle->runtime_data.cursor_pos + 1;
            }
            _display_cmd( handle, TRUE, TRUE );
        }
    } else {
        i = _remain_chars_get( handle );
        if ( _CURSOR_AT_LINE_END ) {
            if ( handle->runtime_data.right_scroll ) {
                ICLI_PUT_SPACE;
                ICLI_PUT_SPACE;
            }

            /* delete whole line */
            for ( i = _CMD_LINE_MAX_LEN; i > 0; --i ) {
                icli_cursor_backward( handle );
                ICLI_PUT_SPACE;
                icli_cursor_backward( handle );
            }

            /* update start_pos */
            handle->runtime_data.start_pos += _SCROLL_SIZE;

            /* get cursor pos */
            handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos - handle->runtime_data.start_pos + 1;

            /* display cmd from begin */
            _display_cmd( handle, TRUE, TRUE );

        } else if ( !_CURSOR_AT_CMD_END && (i == 1 || i == 2) ) {
            for ( ; i > 0; --i ) {
                ICLI_PUT_SPACE;
            }

            /* delete whole line */
            for ( i = _CMD_LINE_MAX_LEN; i > 0; --i ) {
                icli_cursor_backward( handle );
                ICLI_PUT_SPACE;
                icli_cursor_backward( handle );
            }

            /* update start_pos */
            handle->runtime_data.start_pos += _SCROLL_SIZE;

            /* get cursor pos */
            handle->runtime_data.cursor_pos = handle->runtime_data.cmd_pos - handle->runtime_data.start_pos + 1;

            /* display cmd from begin */
            _display_cmd( handle, TRUE, TRUE );
        } else {
            /* display c */
            _PRINT_CHAR( c );

            /* display rest cmd */
            _display_cmd( handle, FALSE, TRUE );
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
i32 vtss_icli_c_usr_cmd_get(
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
            (void)vtss_icli_sutil_usr_char_put(handle, '^');
            (void)vtss_icli_sutil_usr_char_put(handle, 'Q');
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
            (void)_fkey_backspace( handle );
            continue;

        case ICLI_KEY_TAB: /* == Ctrl-I */
            _fkey_tab( handle );
            return ICLI_RC_OK;

        case ICLI_KEY_QUESTION:
            (void)vtss_icli_sutil_usr_char_put(handle, '?');
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
i32 vtss_icli_c_line_mode(
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
} /* vtss_icli_c_line_mode */

void vtss_icli_c_cmd_display(
    IN  icli_session_handle_t   *handle
)
{
    //get command length
    handle->runtime_data.cmd_len = vtss_icli_str_len( handle->runtime_data.cmd );
    //display command
    _display_cmd(handle, TRUE, FALSE);
}

void vtss_icli_c_cmd_redisplay(
    IN  icli_session_handle_t   *handle
)
{
    //get command length
    handle->runtime_data.cmd_len = vtss_icli_str_len( handle->runtime_data.cmd );
    //display command
    _display_cmd(handle, TRUE, TRUE);
}

void vtss_icli_c_line_clear(
    IN  icli_session_handle_t   *handle
)
{
    _del_line( handle );
}

