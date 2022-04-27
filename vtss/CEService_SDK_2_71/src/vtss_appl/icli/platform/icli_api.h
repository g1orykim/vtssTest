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
    > CP.Wang, 2011/04/12 14:45
        - create

==============================================================================
*/
#ifndef __ICLI_API_H__
#define __ICLI_API_H__
//****************************************************************************

/*
==============================================================================

    Include File

==============================================================================
*/
#include "vtss_icli_type.h"
#include "vtss_icli_util.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
==============================================================================

    Macro

==============================================================================
*/
#define ICLI_VTSS_RC(expr)   { i32 __rc__ = (expr); if (__rc__ != ICLI_RC_OK) return __rc__; }

#define ICLI_CLOSE() \
    icli_session_close(session_id)

#define ICLI_CMD_EXEC(cmd, b_exec) \
    icli_session_cmd_exec(session_id, cmd, b_exec)

#define ICLI_CMD_EXEC_ERR_DISPLAY(cmd, b_exec, app_err_msg, b_display_err_syntax) \
    icli_session_cmd_exec_err_display(session_id, cmd, b_exec, app_err_msg, b_display_err_syntax)

#define ICLI_CMD_EXEC_ERR_FILE_LINE(cmd, b_exec, filename, line_number, b_display_err_syntax) \
    icli_session_cmd_exec_err_file_line(session_id, cmd, b_exec, filename, line_number, b_display_err_syntax)

#define ICLI_CMD_PARSING_BEGIN() \
    icli_session_cmd_parsing_begin(session_id);

#define ICLI_CMD_PARSING_END() \
    icli_session_cmd_parsing_end(session_id);

#define ICLI_PRINTF(...) \
    (void)icli_session_printf(session_id, __VA_ARGS__)

#define ICLI_PRINTF_LSTR(lstr) \
    (void)icli_session_printf_lstr(session_id, lstr)

#define ICLI_USR_STR_GET(input_type, str, str_size, end_key) \
    icli_session_usr_str_get(session_id, input_type, str, str_size, end_key)

#define ICLI_CTRL_C_GET(wait_time) \
    icli_session_ctrl_c_get(session_id, wait_time)

#define ICLI_MODE_GET(mode) \
    icli_session_mode_get(session_id, mode)

#define ICLI_MODE_ENTER(mode) \
    icli_session_mode_enter(session_id, mode)

#define ICLI_MODE_EXIT() \
    icli_session_mode_exit(session_id)

#define ICLI_PRIVILEGE_GET(privilege) \
    icli_session_privilege_get(session_id, privilege)

#define ICLI_PRIVILEGE_SET(privilege) \
    icli_session_privilege_set(session_id, privilege)

#define ICLI_TIMEOUT_SET(timeout) \
    icli_session_wait_time_set(session_id, timeout)

#define ICLI_WIDTH_SET(width) \
    icli_session_width_set(session_id, width)

#define ICLI_LINES_SET(lines) \
    icli_session_lines_set(session_id, lines)

#define ICLI_LINE_MODE_GET(line_mode) \
    icli_session_line_mode_get(session_id, line_mode)

#define ICLI_LINE_MODE_SET(line_mode) \
    icli_session_line_mode_set(session_id, line_mode)

#define ICLI_INPUT_STYLE_GET(input_style) \
    icli_session_input_style_get(session_id, input_style)

#define ICLI_INPUT_STYLE_SET(input_style) \
    icli_session_input_style_set(session_id, input_style)

#define ICLI_HISTORY_SIZE_GET(history_size) \
    icli_session_history_size_get(session_id, history_size)

#define ICLI_HISTORY_SIZE_SET(history_size) \
    icli_session_history_size_set(session_id, history_size)

#define ICLI_EXEC_BANNER_ENABLE(b_exec_banner) \
    icli_session_exec_banner_enable(session_id, b_exec_banner)

#define ICLI_MOTD_BANNER_ENABLE(b_exec_banner) \
    icli_session_motd_banner_enable(session_id, b_exec_banner)

#define ICLI_LOCATION_SET(location) \
    icli_session_location_set(session_id, location)

#define ICLI_PRIVILEGED_LEVEL_SET(privileged_level) \
    icli_session_privileged_level_set(session_id, privileged_level)

#define ICLI_SELF_PRINTF(...) \
    (void)icli_session_self_printf(__VA_ARGS__)

#define ICLI_SELF_PRINTF_LSTR(lstr) \
    (void)icli_session_self_printf_lstr(lstr)

#define ICLI_RC_CHECK_SETUP(msg) \
    icli_session_rc_check_setup(session_id, msg)

#define ICLI_RC_CHECK(rc, msg) \
    if ( icli_session_rc_check(session_id, rc, msg) != VTSS_RC_OK ) { \
        return ICLI_RC_ERROR; \
    }

#define ICLI_CMD_VALUE_GET(word_id, value) \
    icli_session_cmd_value_get(session_id, word_id, value)

#define ICLI_WAY_GET(way) \
    icli_session_way_get(session_id, way)

/*
==============================================================================

    Util Definition

==============================================================================
*/
#define icli_to_upper_case              vtss_icli_to_upper_case
#define icli_to_lower_case              vtss_icli_to_lower_case
#define icli_str_len                    vtss_icli_str_len
#define icli_str_cpy                    vtss_icli_str_cpy
#define icli_str_ncpy                   vtss_icli_str_ncpy
#define icli_str_cmp                    vtss_icli_str_cmp
#define icli_str_sub                    vtss_icli_str_sub
#define icli_str_concat                 vtss_icli_str_concat
#define icli_str_str                    vtss_icli_str_str
#define icli_ipv4_to_str                vtss_icli_ipv4_to_str
#define icli_ipv4_class_get             vtss_icli_ipv4_class_get
#define icli_ipv4_prefix_to_netmask     vtss_icli_ipv4_prefix_to_netmask
#define icli_ipv4_netmask_to_prefix     vtss_icli_ipv4_netmask_to_prefix
#define icli_ipv6_netmask_to_prefix     vtss_icli_ipv6_netmask_to_prefix
#define icli_ipv6_to_str                vtss_icli_ipv6_to_str
#define icli_mac_to_str                 vtss_icli_mac_to_str
#define icli_str_to_int                 vtss_icli_str_to_int
#define icli_str_prefix                 vtss_icli_str_prefix

/*
==============================================================================

    Public Function

==============================================================================
*/
/*
    Initialize the ICLI module

    return
       VTSS_OK.
*/
vtss_rc icli_init(
    IN vtss_init_data_t     *data
);

/*
    get text string for each icli_rc_t

    INPUT
        rc : icli_rc_t

    OUTPUT
        n/a

    RETURN
        text string

    COMMENT
        n/a
*/
char *icli_error_txt(
    IN  vtss_rc   rc
);

/*
    get ICLI engine config data

    INPUT
        n/a

    OUTPUT
        conf - config data to get

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_conf_get(
    OUT icli_conf_data_t    *conf
);

/*
    set config data to ICLI engine

    INPUT
        conf - config data to apply

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_conf_set(
    IN  icli_conf_data_t     *conf
);

/*
    reset config data to default

    INPUT
        n/a

    OUTPUT
        conf - config data to reset default

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_conf_default(
    void
);

/*
    get whick input_style the ICLI engine perform

    input -
        n/a

    output -
        input_style: current input style

    return -
        icli_rc_t

    comment -
        n/a
*/
i32 icli_input_style_get(
    OUT icli_input_style_t  *input_style
);

/*
    set whick input_style the ICLI engine perform

    input -
        input_style : input style

    output -
        n/a

    return -
        icli_rc_t

    comment -
        n/a
*/
i32 icli_input_style_set(
    IN icli_input_style_t   input_style
);

/*
    register ICLI command

    INPUT
        cmd_register : command data

    OUTPUT
        n/a

    RETURN
        >= 0 : command ID
        < 0  : icli_rc_t

    COMMENT
        n/a
*/
i32 icli_cmd_register(
    IN  icli_cmd_register_t     *cmd_register
);

/*
    get if the command is enabled or disabled

    INPUT
        cmd_id : command ID

    OUTPUT
        enable : TRUE  - the command is enabled
                 FALSE - the command is disabled

    RETURN
        ICLI_RC_OK : get successfully
        ICLI_RC_ERR_PARAMETER : fail to get

    COMMENT
        n/a
*/
i32 icli_cmd_is_enable(
    IN  u32     cmd_id,
    OUT BOOL    *enable
);

/*
    enable the command

    INPUT
        cmd_id : command ID

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_cmd_enable(
    IN  u32     cmd_id
);

/*
    disable the command

    INPUT
        cmd_id : command ID

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_cmd_disable(
    IN  u32     cmd_id
);

/*
    get if the command is visible or invisible

    INPUT
        cmd_id : command ID

    OUTPUT
        visible : TRUE  - the command is visible
                  FALSE - the command is invisible

    RETURN
        ICLI_RC_OK : get successfully
        ICLI_RC_ERR_PARAMETER : fail to get

    COMMENT
        n/a
*/
i32 icli_cmd_is_visible(
    IN  u32     cmd_id,
    OUT BOOL    *visible
);

/*
    make the command visible to user

    INPUT
        cmd_id : command ID

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_cmd_visible(
    IN  u32     cmd_id
);

/*
    make the command invisible to user

    INPUT
        cmd_id : command ID

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_cmd_invisible(
    IN  u32     cmd_id
);

/*
    get privilege of a command

    INPUT
        cmd_id : command ID

    OUTPUT
        privilege : session privilege

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_cmd_privilege_get(
    IN  u32     cmd_id,
    OUT u32     *privilege
);

/*
    set privilege of a command

    INPUT
        cmd_id    : command ID
        privilege : session privilege

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_cmd_privilege_set(
    IN u32     cmd_id,
    IN u32     privilege
);

/*
    ICLI engine for each session

    This API can be called only by the way of ICLI_SESSION_WAY_THREAD_CONSOLE,
    ICLI_SESSION_WAY_THREAD_TELNET, ICLI_SESSION_WAY_THREAD_SSH.

    The engine handles user input, execute command and display command output.
    User input includes input style, TAB key, ? key, comment, etc.
    And, it works for one command only. So you need to call it in a loop for
    the continuous service. For example,
        while( 1 ) {
            vtss_icli_session_engine( session_id );
        }

    INPUT
        session_id : session ID

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_engine(
    IN i32      session_id
);

/*
    open a ICLI session

    INPUT
        open_data : data for session open

    OUTPUT
        session_id : ID of session opened successfully

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_open(
    IN  icli_session_open_data_t    *open_data,
    OUT u32                         *session_id
);

/*
    close a ICLI session

    INPUT
        session_id : session ID from icli_session_open()

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_close(
    IN u32  session_id
);

/*
    close all ICLI sessions

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
void icli_session_all_close(void);

/*
    get max number of sessions

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        max number of sessions

    COMMENT
        n/a
*/
u32 icli_session_max_get(void);

/*
    set max number of sessions

    INPUT
        max_sessions : max number of sessions

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_max_set(
    IN u32 max_sessions
);

/*
    execute or parse a command on a session

    If parsing commands
    before parsing, begin the transaction by ICLI_CMD_PARSING_BEGIN()
    and end the transaction after finishing the parsing by ICLI_CMD_PARSING_END()
    the flow is as follows.

        ICLI_CMD_PARSING_BEGIN();
        ICLI_CMD_EXEC(..., FALSE);
        ...
        ICLI_CMD_EXEC(..., FALSE);
        ICLI_CMD_PARSING_END();

    INPUT
        session : session ID
        cmd     : command to be executed
        b_exec  : TRUE  - execute the command function
                  FALSE - parse the command only to check if the command is legal or not,
                          but not execute

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_cmd_exec(
    IN u32      session_id,
    IN char     *cmd,
    IN BOOL     b_exec
);

/*
    execute or parsing a command on a session and display error message from application

    If parsing commands
    before parsing, begin the transaction by ICLI_CMD_PARSING_BEGIN()
    and end the transaction after finishing the parsing by ICLI_CMD_PARSING_END()
    the flow is as follows.

        ICLI_CMD_PARSING_BEGIN();
        ICLI_CMD_EXEC_ERR_DISPLAY(..., FALSE, ...);
        ...
        ICLI_CMD_EXEC_ERR_DISPLAY(..., FALSE, ...);
        ICLI_CMD_PARSING_END();

    INPUT
        session              : session ID
        cmd                  : command to be executed
        b_exec               : TRUE  - execute the command function
                               FALSE - parse the command only to
                                       check if the command is legal syntax,
                                       but not execute
        app_err_msg          : error message from application
        b_display_err_syntax : TRUE  - display error syntax
                               FALSE - not display

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_cmd_exec_err_display(
    IN u32      session_id,
    IN char     *cmd,
    IN BOOL     b_exec,
    IN char     *app_err_msg,
    IN BOOL     b_display_err_syntax
);

/*
    execute or parsing a command on a session and display error message from application

    If parsing commands
    before parsing, begin the transaction by ICLI_CMD_PARSING_BEGIN()
    and end the transaction after finishing the parsing by ICLI_CMD_PARSING_END()
    the flow is as follows.

        ICLI_CMD_PARSING_BEGIN();
        ICLI_CMD_EXEC_ERR_DISPLAY(..., FALSE, ...);
        ...
        ICLI_CMD_EXEC_ERR_DISPLAY(..., FALSE, ...);
        ICLI_CMD_PARSING_END();

    INPUT
        session_id           : session ID
        cmd                  : command to be executed
        b_exec               : TRUE  - execute the command function
                               FALSE - parse the command only to
                                       check if the command is legal syntax,
                                       but not execute
        filename             : config file name of the command
        line_number          : the line number in the config file for the command
        b_display_err_syntax : TRUE  - display error syntax
                               FALSE - not display

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_cmd_exec_err_file_line(
    IN u32      session_id,
    IN char     *cmd,
    IN BOOL     b_exec,
    IN char     *filename,
    IN u32      line_number,
    IN BOOL     b_display_err_syntax
);

/*
    begin transaction to parse a batch of commands on a session

    INPUT
        session : session ID

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_cmd_parsing_begin(
    IN u32      session_id
);

/*
    end transaction to parse commands on a session

    INPUT
        session : session ID

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_cmd_parsing_end(
    IN u32      session_id
);

/*
    output formated string to a specific session
    the maximum length to print is (ICLI_STR_MAX_LEN + 64),
    where ICLI_STR_MAX_LEN depends on project.

    * if the length is > (ICLI_STR_MAX_LEN + 64),
      then please use icli_session_self_printf_lstr()

    INPUT
        session_id : the session ID
        format     : string format
        ...        : parameters of format

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_printf(
    IN  u32         session_id,
    IN  const char  *format,
    IN  ...
#if VTSS_OPSYS_ECOS
) __attribute__ ((format (printf, 2, 3)));
#else
);
#endif

/*
    output long string to a specific session,
    where the length of long string is larger than (ICLI_STR_MAX_LEN),
    where ICLI_STR_MAX_LEN depends on project

    the long string must be a char array or dynamic allocated memory(write-able),
    and must not be a const string(read-only).

    INPUT
        session_id : the session ID
        lstr       : long string to print
                     *** str must be a char array or dynamic allocated memory(write-able),
                         and must not a const string(read_only).

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_printf_lstr(
    IN  u32         session_id,
    IN  char        *lstr
);

/*
    output formated string to all sessions
    this does not check ICLI_LINE_MODE_BYPASS
    because this is used for system message

    INPUT
        format     : string format
        ...        : parameters of format

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_printf_to_all(
    IN  const char  *format,
    IN  ...
#if VTSS_OPSYS_ECOS
) __attribute__ ((format (printf, 1, 2)));
#else
);
#endif

/*
    get string from usr input

    INPUT
        session_id : the session ID
        type       : input type
                     NORMAL   - echo the input char
                     PASSWORD - echo '*'
        str_size   : the buffer size of str

    OUTPUT
        str        : user input string
        str_size   : the length of user input
        end_key    : the key to terminate the input,
                     Enter, New line, or Ctrl-C

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_usr_str_get(
    IN      u32                     session_id,
    IN      icli_usr_input_type_t   input_type,
    OUT     char                    *str,
    INOUT   i32                     *str_size,
    OUT     i32                     *end_key
);

/*
    get Ctrl-C from user

    INPUT
        session_id : the session ID
        wait_time  : time to wait in milli-seconds
                     must be 2147483647 >= wait_time > 0

    OUTPUT
        n/a

    RETURN
        ICLI_RC_OK            : yes, the user press Ctrl-C
        ICLI_RC_ERR_EXPIRED   : time expired
        ICLI_RC_ERR_PARAMETER : input paramter error

    COMMENT
        n/a
*/
i32 icli_session_ctrl_c_get(
    IN  u32     session_id,
    IN  u32     wait_time
);

/*
    get current command mode of the session

    INPUT
        session_id : the session ID

    OUTPUT
        mode : current command mode

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_mode_get(
    IN  u32                 session_id,
    OUT icli_cmd_mode_t     *mode
);

/*
    make session entering the mode

    INPUT
        session_id : the session ID
        mode       : mode entering

    OUTPUT
        n/a

    RETURN
        >= 0 : successful and the return value is mode level
        -1   : failed

    COMMENT
        n/a
*/
i32 icli_session_mode_enter(
    IN  u32                 session_id,
    IN  icli_cmd_mode_t     mode
);

/*
    make session exit the top mode

    INPUT
        session_id : the session ID

    OUTPUT
        n/a

    RETURN
        >= 0 : successful and the return value is mode level
        -1   : failed

    COMMENT
        n/a
*/
i32 icli_session_mode_exit(
    IN  u32     session_id
);

/*
    get configuration data of the session

    INPUT
        session_id : the session ID, INDEX

    OUTPUT
        data: data of the session

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_data_get(
    INOUT icli_session_data_t   *data
);

/*
    get configuration data of the next session

    INPUT
        session_id : the next session of the session ID, INDEX

    OUTPUT
        data : data of the next session

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_data_get_next(
    INOUT icli_session_data_t   *data
);

#if 1 /* CP, 2012/08/29 09:25, history max count is configurable */
/*
    set history size of a session

    INPUT
        session_id   : session ID

    OUTPUT
        history_size : the size of history commands
                       0 means to disable history function

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_history_size_get(
    IN  u32     session_id,
    OUT u32     *history_size
);

/*
    set history size of a session

    INPUT
        session_id   : session ID
        history_size : the size of history commands
                       0 means to disable history function

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_history_size_set(
    IN  u32     session_id,
    IN  u32     history_size
);
#endif /* CP, 2012/08/29 09:25, history max count is configurable */

/*
    get the first history command of a session

    INPUT
        session_id  : session ID

    OUTPUT
        history_cmd : the first history command of the session

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_history_cmd_get_first(
    IN  u32                         session_id,
    OUT icli_session_history_cmd_t  *history
);

/*
    get history command of the session

    INPUT
        session_id           : session ID
        history->history_pos : INDEX

    OUTPUT
        history_cmd : the history command of the session

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_history_cmd_get(
    IN    u32                         session_id,
    INOUT icli_session_history_cmd_t  *history
);

/*
    get the next history command of the session

    INPUT
        session_id           : session ID
        history->history_pos : INDEX

    OUTPUT
        history_cmd : the next history command of the session

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_history_cmd_get_next(
    IN    u32                         session_id,
    INOUT icli_session_history_cmd_t  *history
);

/*
    get privilege of a session

    INPUT
        session_id : session ID

    OUTPUT
        privilege  : session privilege

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_privilege_get(
    IN  u32                 session_id,
    OUT icli_privilege_t    *privilege
);

/*
    set privilege of a session

    INPUT
        session_id : session ID
        privilege  : session privilege

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_privilege_set(
    IN  u32                 session_id,
    IN  icli_privilege_t    privilege
);

/*
    set width of a session

    INPUT
        session_id : session ID
        width      : width (in number of characters) of the session

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_width_set(
    IN  u32     session_id,
    IN  u32     width
);

/*
    set lines of a session

    INPUT
        session_id : session ID
        lines      : number of lines on a screen

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_lines_set(
    IN  u32     session_id,
    IN  u32     lines
);

/*
    set waiting time of a session

    INPUT
        session_id : session ID
        wait_time  : time to wait user input, in seconds
                     = 0 means wait forever

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_wait_time_set(
    IN  u32     session_id,
    IN  u32     wait_time
);

/*
    get line mode of the session

    INPUT
        session_id : the session ID

    OUTPUT
        line_mode : line mode of the session

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_line_mode_get(
    IN  u32                 session_id,
    OUT icli_line_mode_t    *line_mode
);

/*
    set line mode of the session

    INPUT
        session_id : the session ID
        line_mode  : line mode of the session

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_line_mode_set(
    IN  u32                 session_id,
    IN  icli_line_mode_t    line_mode
);

/*
    get input style of the session

    INPUT
        session_id : the session ID

    OUTPUT
        input_style : input style

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_input_style_get(
    IN  u32                 session_id,
    OUT icli_input_style_t  *input_style
);

/*
    set input style of the session

    INPUT
        session_id  : the session ID
        input_style : input style of the session

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_input_style_set(
    IN  u32                 session_id,
    IN  icli_input_style_t  input_style
);

#if 1 /* CP, 2012/08/31 07:51, enable/disable banner per line */
/*
    enable/disable the display of EXEC banner of the session

    INPUT
        session_id    : the session ID
        b_exec_banner : TRUE - enable, FALSE - disable

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_exec_banner_enable(
    IN  u32     session_id,
    IN  BOOL    b_exec_banner
);

/*
    enable/disable the display of Day banner of the session

    INPUT
        session_id    : the session ID
        b_motd_banner : TRUE - enable, FALSE - disable

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_motd_banner_enable(
    IN  u32     session_id,
    IN  BOOL    b_motd_banner
);
#endif

#if 1 /* CP, 2012/08/31 09:31, location and default privilege */
/*
    set location of the session

    INPUT
        session_id : the session ID
        location   : where you are

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_location_set(
    IN  u32                 session_id,
    IN  char                *location
);

/*
    set privileged level of the session

    INPUT
        session_id        : the session ID
        default_privilege : default privilege level

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_privileged_level_set(
    IN  u32                 session_id,
    IN  icli_privilege_t    privileged_level
);
#endif

#if 1 /* CP, 2012/09/04 16:46, session user name */
/*
    set user name of the session

    INPUT
        session_id : the session ID
        user_name  : who login

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_user_name_set(
    IN  u32     session_id,
    IN  char    *user_name
);
#endif

#if 1 /* CP, 2012/09/11 14:08, Use thread information to get session ID */
/*
    output formated string to a specific session
    the maximum length to print is (ICLI_STR_MAX_LEN + 64),
    where ICLI_STR_MAX_LEN depends on project.

    * if the length is > (ICLI_STR_MAX_LEN + 64),
      then please use icli_session_self_printf_lstr()

    INPUT
        format     : string format
        ...        : parameters of format

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
int icli_session_self_printf(
    IN  const char  *format,
    IN  ...
#if VTSS_OPSYS_ECOS
) __attribute__ ((format (printf, 1, 2)));
#else
);
#endif

/*
    output long string to a specific session
    the length of long string is larger than (ICLI_STR_MAX_LEN),
    where ICLI_STR_MAX_LEN depends on project

    the long string must be a char array or dynamic allocated memory(write-able),
    and must not be a const string(read-only).

    INPUT
        session_id : the session ID
        lstr       : long string to print
                     *** str must be a char array or dynamic allocated memory(write-able),
                         and must not a const string(read_only).

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_self_printf_lstr(
    IN  char        *lstr
);
#endif /* CP, 2012/09/11 14:08, Use thread information to get session ID */

#if 1 /* CP, 2012/10/16 17:02, ICLI_RC_CHECK */
/*
    set general error text string when error happen
    this error string will print out before error_txt(rc)

    INPUT
        session_id : ID of session
        msg        : general error message

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
void icli_session_rc_check_setup(
    IN  u32         session_id,
    IN  const char  *msg
);

/*
    check if rc is ok or not.
    if rc is error, then print out sequence is as follows.
        1. msg (from this API)
        2. rc_context_string (from icli_session_rc_check_setup())
        3. error_txt(rc).

    INPUT
        session_id : ID of session
        rc         : error code
        msg        : general error message

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
vtss_rc icli_session_rc_check(
    IN  u32         session_id,
    IN  vtss_rc     rc,
    IN  const char  *msg
);
#endif

/*
    get value of the specific command word with word_id
    where word_id is from 0.
    for example, the command,
                  show ip interface
        word_id = 0    1  2

    INPUT
        session_id : session ID
        word_id    : command word ID

    OUTPUT
        value : value resulting from user input

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_cmd_value_get(
    IN  u32                     session_id,
    IN  u32                     word_id,
    OUT icli_variable_value_t   *value
);

/*
    get session way

    INPUT
        session_id : session ID

    OUTPUT
        way : session way

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_way_get(
    IN  u32                     session_id,
    OUT icli_session_way_t      *way
);

#if 1 /* CP, 06/24/2013 13:57, Bugzilla#12076 - slient upgrade */

/*
    get silent upgrade allow config

    INPUT
        n/a

    OUTPUT
        suac:
            TRUE  - allow
            FLASE - not allow

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_suac_get(
    IN  u32     session_id,
    OUT BOOL    *suac
);

/*
    set silent upgrade allow config

    INPUT
        suac:
            TRUE  - allow
            FLASE - not allow

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_session_suac_set(
    IN  u32     session_id,
    IN  BOOL    suac
);

#endif

/*
    set mode name shown in command prompt

    INPUT
        mode : command mode
        name : mode name

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_mode_name_set(
    IN  icli_cmd_mode_t     mode,
    IN  char                *name
);

/*
    set device name shown in command prompt

    INPUT
        dev_name : device name

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_dev_name_set(
    IN  char    *dev_name
);

/*
    get device name

    INPUT
        n/a

    OUTPUT
        dev_name : device name

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_dev_name_get(
    OUT char    *dev_name
);

/*
    get LOGIN banner.
    the maximum length is 255 chars, ICLI_BANNER_MAX_LEN.

    INPUT
        n/a

    OUTPUT
        banner_login : LOGIN banner

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_banner_login_get(
    OUT  char    *banner_login
);

/*
    set LOGIN banner.
    the maximum length is 255 chars, ICLI_BANNER_MAX_LEN.

    INPUT
        banner_login : LOGIN banner

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_banner_login_set(
    IN  char    *banner_login
);

/*
    get MOTD banner.
    the maximum length is 255 chars, ICLI_BANNER_MAX_LEN.

    INPUT
        n/a

    OUTPUT
        banner_motd : MOTD banner

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_banner_motd_get(
    OUT  char    *banner_motd
);

/*
    set MOTD banner.
    the maximum length is 255 chars, ICLI_BANNER_MAX_LEN.

    INPUT
        banner_motd : MOTD banner

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_banner_motd_set(
    IN  char    *banner_motd
);

/*
    get EXEC banner.
    the maximum length is 255 chars, ICLI_BANNER_MAX_LEN.

    INPUT
        n/a

    OUTPUT
        banner_exec : EXEC banner

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_banner_exec_get(
    OUT  char    *banner_exec
);

/*
    set EXEC banner.
    the maximum length is 255 chars, ICLI_BANNER_MAX_LEN.

    INPUT
        banner_exec : EXEC banner

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_banner_exec_set(
    IN  char    *banner_exec
);

/*
    get the time elapsed from system start in milliseconds
*/
u32 icli_current_time_get(void);

/*
    check if the port type is present in this device

    INPUT
        type : port type

    OUTPUT
        n/a

    RETURN
        TRUE  : yes, the device has ports belong to this port type
        FALSE : no

    COMMENT
        n/a
*/
BOOL icli_port_type_present(
    IN icli_port_type_t     type
);

/*
    get port type for a specific switch port

    INPUT
        type : port type

    OUTPUT
        n/a

    RETURN
        name of the type
            if the type is invalid. then "Unknown" is return

    COMMENT
        n/a
*/
char *icli_port_type_get_name(
    IN  icli_port_type_t    type
);

/*
    get short port type name for a specific port type

    INPUT
        type : port type

    OUTPUT
        n/a

    RETURN
        short name of the type
            if the type is invalid. then "Unkn" is returned

    COMMENT
        n/a
*/
char *icli_port_type_get_short_name(
    IN  icli_port_type_t    type
);

/*
    reset port setting

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
void icli_port_range_reset( void );

/*
    get port range

    INPUT

    OUTPUT
        range : port range

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_port_range_get(
    OUT icli_stack_port_range_t  *range
);

/*
    set port range on a specific port type

    INPUT
        range  : port range set on the port type

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        CIRTICAL ==> before set, port range must be reset by icli_port_range_reset().
        otherwise, the set may be failed because this set will check the
        port definition to avoid duplicate definitions.
*/
BOOL icli_port_range_set(
    IN icli_stack_port_range_t  *range
);

/*
    get switch range from usid and uport

    INPUT
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->port_cnt    : number of ports

    OUTPUT
        switch_range

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_port_from_usid_uport(
    INOUT icli_switch_port_range_t  *switch_range
);

/*
    get switch range from isid and iport

    INPUT
        switch_range->isid        : isid
        switch_range->begin_iport : iport
        switch_range->port_cnt    : number of ports

    OUTPUT
        switch_range

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_port_from_isid_iport(
    INOUT icli_switch_port_range_t  *switch_range
);

/*
    get first switch_id/type/port

    INPUT
        n/a

    OUTPUT
        switch_range->port_type   : first port type
        switch_range->switch_id   : first switch ID
        switch_range->begin_port  : first port ID
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->isid        : isid
        switch_range->begin_iport : iport

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_port_get_first(
    OUT icli_switch_port_range_t  *switch_range
);

/*
    get from switch_id/type/port

    INPUT
        index:
            switch_range->switch_id   : switch ID
            switch_range->port_type   : port type
            switch_range->begin_port  : port ID

    OUTPUT
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->isid        : isid
        switch_range->begin_iport : iport

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_port_get(
    INOUT icli_switch_port_range_t  *switch_range
);

/*
    get next switch_id/type/port

    INPUT
        index:
            switch_range->switch_id   : switch ID
            switch_range->port_type   : port type
            switch_range->begin_port  : port ID

    OUTPUT
        switch_range->switch_id   : next switch ID
        switch_range->port_type   : next port type
        switch_range->begin_port  : next port ID
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->isid        : isid
        switch_range->begin_iport : iport

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_port_get_next(
    INOUT icli_switch_port_range_t  *switch_range
);

/*
    check if the switch port range is valid or not

    INPUT
        switch_port : the followings are checking parameters
            switch_port->port_type
            switch_port->switch_id
            switch_port->begin_port
            switch_port->port_cnt

    OUTPUT
        switch_id, port_id : the port is not valid
                             put NULL if these are not needed

    RETURN
        TRUE  : all the switch ports are valid
        FALSE : at least one of switch port is not valid

    COMMENT
        n/a
*/
BOOL icli_port_switch_range_valid(
    IN  icli_switch_port_range_t    *switch_port,
    OUT u16                         *switch_id,
    OUT u16                         *port_id
);

/*
    get switch_id by usid

    INPUT
        usid

    OUTPUT
        n/a

    RETURN
        switch ID

    COMMENT
        n/a
*/
u16 icli_usid2switchid(
    IN u16  usid
);

/*
    get switch_id by isid

    INPUT
        isid

    OUTPUT
        n/a

    RETURN
        switch ID

    COMMENT
        n/a
*/
u16 icli_isid2switchid(
    IN u16  isid
);

/*
    get switch_id

    INPUT
        index:
            switch_range->switch_id : switch ID

    OUTPUT
        switch_range->switch_id
        switch_range->usid
        switch_range->isid
        others = 0

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_switch_get(
    INOUT icli_switch_port_range_t  *switch_range
);

/*
    get next switch_id

    INPUT
        index:
            switch_range->switch_id : switch ID

    OUTPUT
        switch_range->switch_id : next switch ID
        switch_range->usid      : next usid
        switch_range->isid      : next isid
        others = 0

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_switch_get_next(
    INOUT icli_switch_port_range_t  *switch_range
);

/*
    get from switch_id/type/port

    INPUT
        index :
            switch_range->switch_id   : switch ID
            switch_range->port_type   : port type
            switch_range->begin_port  : port ID

    OUTPUT
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->isid        : isid
        switch_range->begin_iport : iport

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_switch_port_get(
    INOUT icli_switch_port_range_t  *switch_range
);

/*
    get next switch_id/type/port

    INPUT
        index:
            switch_range->switch_id   : switch ID (not changed)
            switch_range->port_type   : port type
            switch_range->begin_port  : port ID

    OUTPUT
        switch_range->switch_id   : current switch ID (not changed)
        switch_range->port_type   : next port type
        switch_range->begin_port  : next port ID
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->isid        : isid
        switch_range->begin_iport : iport

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_switch_port_get_next(
    INOUT icli_switch_port_range_t  *switch_range
);

/*
    get password of privilege level

    INPUT
        priv : the privilege level

    OUTPUT
        password : the corresponding password with size (ICLI_PASSWORD_MAX_LEN + 1)

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_enable_password_get(
    IN  icli_privilege_t    priv,
    OUT char                *password
);

/*
    set password of privilege level

    INPUT
        priv     : the privilege level
        password : the corresponding password with size (ICLI_PASSWORD_MAX_LEN + 1)

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_enable_password_set(
    IN  icli_privilege_t    priv,
    IN  char                *password
);

#if 1 /* CP, 2012/08/31 17:00, enable secret */
/*
    verify clear password of privilege level is correct or not
    according to password or secret

    INPUT
        priv           : the privilege level
        clear_password : the corresponding password with size (ICLI_PASSWORD_MAX_LEN + 1)

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_enable_password_verify(
    IN  icli_privilege_t    priv,
    IN  char                *clear_password
);

/*
    set secret of privilege level

    INPUT
        priv   : the privilege level
        secret : the corresponding secret with size (ICLI_PASSWORD_MAX_LEN + 1)

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_enable_secret_set(
    IN  icli_privilege_t    priv,
    IN  char                *secret
);

/*
    translate clear password of privilege level to secret password

    INPUT
        priv           : the privilege level
        clear_password : the corresponding password with size (ICLI_PASSWORD_MAX_LEN + 1)

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL icli_enable_secret_clear_set(
    IN  icli_privilege_t    priv,
    IN  char                *clear_password
);

/*
    get if the enable password is in secret or not

    INPUT
        priv : the privilege level

    OUTPUT
        n/a

    RETURN
        TRUE  : in secret
        FALSE : clear password

    COMMENT
        n/a
*/
BOOL icli_enable_password_if_secret_get(
    IN  icli_privilege_t    priv
);
#endif

#if 1 /* CP, 2012/10/08 14:31, debug command, debug prompt */
/*
    set debug prompt shown in command prompt

    INPUT
        debug_prompt : debug prompt

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_debug_prompt_set(
    IN  char    *debug_prompt
);

/*
    get debug prompt

    INPUT
        n/a

    OUTPUT
        debug_prompt : debug prompt

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_debug_prompt_get(
    OUT char    *debug_prompt
);
#endif

/*
    set privilege per command

    INPUT
        conf   : privilege command configuration

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_priv_set(
    IN  icli_priv_cmd_conf_t    *conf
);

/*
    delete privilege per command

    INPUT
        conf : privilege command configuration, index - mode, cmd

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_priv_delete(
    IN icli_priv_cmd_conf_t    *conf
);

/*
    get first privilege per command

    INPUT
        n/a

    OUTPUT
        conf : first privilege command configuration, index - mode, cmd

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_priv_get_first(
    OUT icli_priv_cmd_conf_t    *conf
);

/*
    get privilege per command

    INPUT
        conf : privilege command configuration, index - mode, cmd

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_priv_get(
    INOUT icli_priv_cmd_conf_t    *conf
);

/*
    get next privilege per command
    use index - mode, cmd to find the current one and then get the next one
    of the current one. So, if the current one is not found, then this fails.

    INPUT
        conf : privilege command configuration, sorted by time

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_priv_get_next(
    INOUT icli_priv_cmd_conf_t    *conf
);

/*
    get mode name for privilege command

    INPUT
        mode : command mode

    OUTPUT
        n/a

    RETURN
        mode name

    COMMENT
        n/a
*/
char *icli_priv_mode_name(
    IN icli_cmd_mode_t  mode
);

/*
    enable/disable a VLAN interface

    INPUT
        vid      : VLAN interface ID to add
        b_enable : TRUE - enable the VLAN interface, FALSE - disable it

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_vlan_enable_set(
    IN u32      vid,
    IN BOOL     b_enable
);

/*
    get if the VLAN interface is enabled or disabled

    INPUT
        vid : VLAN interface ID to get

    OUTPUT
        b_enable : TRUE - enabled, FALSE - disabled

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_vlan_enable_get(
    IN  u32     vid,
    OUT BOOL    *b_enable
);

/*
    enter/leave a VLAN interface

    INPUT
        vid     : VLAN interface ID to add
        b_enter : TRUE - enter the VLAN interface, FALSE - leave it

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_vlan_enter_set(
    IN u32      vid,
    IN BOOL     b_enter
);

/*
    get if enter or leave the VLAN interface

    INPUT
        vid : VLAN interface ID to get

    OUTPUT
        b_enter : TRUE - enter, FALSE - leave

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 icli_vlan_enter_get(
    IN  u32     vid,
    OUT BOOL    *b_enter
);

//****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //__ICLI_API_H__
