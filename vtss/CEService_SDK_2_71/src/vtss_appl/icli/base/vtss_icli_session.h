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
    > CP.Wang, 05/29/2013 11:46
        - create

==============================================================================
*/
#ifndef __VTSS_ICLI_SESSION_H__
#define __VTSS_ICLI_SESSION_H__
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
/* Visible keys */
#define ICLI_VISIBLE_KEY_BEGIN      32
#define ICLI_VISIBLE_KEY_END        126

/*
    Windows function key,
 */
#define ICLI_KEY_FUNCTION0          0
#define ICLI_FKEY0_F01              59
#define ICLI_FKEY0_F02              60
#define ICLI_FKEY0_F03              61
#define ICLI_FKEY0_F04              62
#define ICLI_FKEY0_F05              63
#define ICLI_FKEY0_F06              64
#define ICLI_FKEY0_F07              65
#define ICLI_FKEY0_F08              66
#define ICLI_FKEY0_F09              67
#define ICLI_FKEY0_F10              68


#define ICLI_KEY_FUNCTION1          224
#define ICLI_FKEY1_UP               72
#define ICLI_FKEY1_DOWN             80
#define ICLI_FKEY1_LEFT             75
#define ICLI_FKEY1_RIGHT            77
#define ICLI_FKEY1_HOME             71
#define ICLI_FKEY1_END              79
#define ICLI_FKEY1_DEL              83
#define ICLI_FKEY1_F11              133
#define ICLI_FKEY1_F12              134

/*
    VT100 function key on eCos

    esc     ^[          27

    backspace           8
    tab                 9
    del                 127

    f1      ^[[11~      27 91 49 49 126
    f2      ^[[12~      27 91 49 50 126

    home    ^[[1~       27 91 49 126
    insert  ^[[2~       27 91 50 126
    delete  ^[[3~       27 91 51 126
    end     ^[[4~       27 91 52 126
    pg up   ^[[5~       27 91 53 126
    pg dn   ^[[6~       27 91 54 126

    up      ^[[A        27 91 65
    down    ^[[B        27 91 66
    right   ^[[C        27 91 67
    left    ^[[D        27 91 68
 */
#define ICLI_HYPER_TERMINAL         27 /* 0x1B */
#define ICLI_HT_BEGIN               91 /* 0x5B */
#define ICLI_HT_END                 126


#define ICLI_VKEY_UP                65
#define ICLI_VKEY_DOWN              66
#define ICLI_VKEY_RIGHT             67
#define ICLI_VKEY_LEFT              68

#define ICLI_VKEY_HOME              49
#define ICLI_VKEY_INSERT            50
#define ICLI_VKEY_DEL               51
#define ICLI_VKEY_END               52
#define ICLI_VKEY_PGUP              53
#define ICLI_VKEY_PGDOWN            54

#define _SESSON_INPUT_STYLE        (handle->config_data->input_style)
#define ICLI_KEY_VISIBLE(c)        ( (c) >= ICLI_VISIBLE_KEY_BEGIN && (c) <= ICLI_VISIBLE_KEY_END )

#define _CURSOR_AT_CMD_BEGIN       ( handle->runtime_data.cmd_pos == 0 )
#define _CURSOR_AT_CMD_END         ( handle->runtime_data.cmd_pos == handle->runtime_data.cmd_len )

#define ICLI_MORE_ENTER             -1000

/*
==============================================================================

    Type Definition

==============================================================================
*/
typedef struct _icli_mode_para {
    /*
     *  current mode,
     */
    icli_cmd_mode_t         mode;

    /*
     *  1. the parameter list for the current mode,
     *  2. it would be the parameter list of the command that changes
     *     into this mode,
     */
    icli_parameter_t        *cmd_var;

} icli_mode_para_t;

typedef enum {
    ICLI_EXEC_TYPE_INVALID = -1,
    //------- add below
    ICLI_EXEC_TYPE_CMD,
    ICLI_EXEC_TYPE_TAB,
    ICLI_EXEC_TYPE_QUESTION,
    ICLI_EXEC_TYPE_PARSING,
    ICLI_EXEC_TYPE_REDISPLAY,
#if 1 /* CP, 08/13/2013 13:00, Bugzilla#12423 - show full command syntax */
    ICLI_EXEC_TYPE_FULL_CMD,
#endif
} icli_exec_type_t;

typedef enum {
    ICLI_AFTER_CMD_ACT_INVALID = -1,
    //------- add below
    ICLI_AFTER_CMD_ACT_NONE,
    ICLI_AFTER_CMD_ACT_GOTO_PREVIOUS_MODE,
    ICLI_AFTER_CMD_ACT_GOTO_EXEC_MODE,
} icli_after_cmd_act_t;

typedef enum {
    ICLI_ERR_DISPLAY_MODE_INVALID = -1,
    //------- add below
    ICLI_ERR_DISPLAY_MODE_DROP,
    ICLI_ERR_DISPLAY_MODE_PRINT,
    ICLI_ERR_DISPLAY_MODE_ERR_BUFFER,
} icli_err_display_mode_t;

typedef struct _history_cmd {
    /* history command */
    char                    cmd[ICLI_STR_MAX_LEN + 4];

    /* double ring */
    struct _history_cmd     *prev;
    struct _history_cmd     *next;
} icli_history_cmd_t;

typedef struct {
    u32                         alive;
    u32                         privilege; // icli_privilege_t
    u32                         connect_time; //in milli-seconds
    u32                         idle_time;    //in milli-seconds

#if 1 /* CP, 2012/09/11 14:08, Use thread information to get session ID */
    u32                         thread_id;
#endif

#if 1 /* CP, 2012/09/04 16:46, session user name */
    char                        user_name[ICLI_USERNAME_MAX_LEN + 4];
#endif

    /*------------------------------------------------------------
        Command Mode
    ------------------------------------------------------------*/
    //List of parent modes
    icli_mode_para_t            mode_para[ICLI_MODE_MAX_LEVEL];
    i32                         mode_level;

    /*------------------------------------------------------------
        command execution
    ------------------------------------------------------------*/
    //buffer for command
    char                        cmd[ICLI_STR_MAX_LEN + 4];
    //length of command
    i32                         cmd_len;
    //current input position of cmd, index of cmd
    i32                         cmd_pos;
    //length of prompt
    i32                         prompt_len;
    //previous char to avoid the case CR+LF
    i32                         prev_char;
    //buffer for pre-get in cmd_put
    i32                         buffer_char;

    /* Scroll */
    //start position to display cmd, index of cmd
    i32                         start_pos;
    //where the cursor is in line, start from 0 to (width - prompt_len - 1)
    i32                         cursor_pos;
    //scroll or not, 0: no scroll, 1: scroll
    i32                         left_scroll;
    i32                         right_scroll;

    //execution type
    i32                         exec_type; // icli_exec_type_t
    i32                         after_cmd_act; // icli_after_cmd_act_t

    // if the execution is by API from application
    i32                         b_exec_by_api;
    // error message from application, print only if b_exec_by_api == TRUE
    // also to used to store filename
    char                        *app_err_msg;
    // if line_number == ICLI_INVALID_LINE_NUMBER
    // then app_err_msg = file name
    // else app_err_msg = application error message
    i32                         line_number;
    // if b_exec_by_api == TRUE, then it indicates whether to print error syntax or not
    i32                         err_display_mode; // icli_err_display_mode_t
    // the buffer is to store error messages when err_display_mode == ICLI_ERR_DISPLAY_MODE_BUFFER
    char                        err_buf[ICLI_ERR_MAX_LEN + 4];

    //execution result code
    i32                         rc;

    //parsing session
    i32                         b_in_parsing;
    u32                         current_parsing_mode; // icli_cmd_mode_t

    //current command parsing
    icli_parameter_t            *match_para;
    i32                         exactly_cnt;
    i32                         partial_cnt;
    i32                         total_match_cnt;
#if 1 /* CP, 2012/12/12 18:09, match_sort_list */
    icli_parameter_t            *match_sort_list;
#endif

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
    // for loop ()
    i32                         b_in_loop;
#endif

    //tab process
    // if printf <cr>, that is if the command is normal ending
    i32                         b_cr;

    //current command variable values
    icli_parameter_t            *cmd_var;
    char                        cmd_copy[ICLI_STR_MAX_LEN + 4];
    char                        *cmd_walk;
    char                        *cmd_start;
#if 0 /* CP, 2012/08/30 09:12, do command by CLI command */
    char                        *cmd_do;
#endif
    i32                         last_port_type; // icli_port_type_t

    /*------------------------------------------------------------
        Temp Buffer for printf, ?, TAB
    ------------------------------------------------------------*/
    char                        str_buf[ICLI_PUT_MAX_LEN + 4];
    char                        *put_str;
    u32                         more_print;

    // 0: in ICLI engine operation
    // 1: in executing command callback
    // so we can know how to process the output
    i32                         b_in_exec_cb;

    /*------------------------------------------------------------
        TAB
    ------------------------------------------------------------*/
    u32                         tab_cnt;
    i32                         tab_port_type; // icli_port_type_t

    /*------------------------------------------------------------
        History command
    ------------------------------------------------------------*/
    icli_history_cmd_t          history_cmd_buf[ICLI_HISTORY_CMD_CNT];
    icli_history_cmd_t          *history_cmd_free_list;
    icli_history_cmd_t          *history_cmd_head;
    icli_history_cmd_t          *history_cmd_pos;
    u32                         history_cmd_cnt;

    /*------------------------------------------------------------
        More page
    ------------------------------------------------------------*/
    i32                         line_cnt;
    i32                         line_mode; // icli_line_mode_t

    /*------------------------------------------------------------
        GREP
    ------------------------------------------------------------*/
    icli_parameter_t            *grep_var;
    i32                         grep_begin;

    /*------------------------------------------------------------
        Terminal runtime
    ------------------------------------------------------------*/
    u32                         width;
    u32                         lines;

#if 1 /* CP, 2012/10/16 17:02, ICLI_RC_CHECK */
    const char                  *rc_context_string;
#endif

#if ICLI_RANDOM_MUST_NUMBER
    icli_parsing_node_t         *random_must_head[ICLI_RANDOM_MUST_CNT];
    u32                         random_must_match[ICLI_RANDOM_MUST_CNT];
#endif

    /* static database for icli_session_engine_thread */
    u32                         b_comment;
    char                        *comment_pos;
    u32                         comment_ch;

#if 1 /* CP, 2013/03/14 18:02, processing <string> */
    u32                         b_string_end;
#endif

#if 1 /* CP, 06/24/2013 13:57, Bugzilla#12076 - slient upgrade */
    u32                         b_silent_upgrade_allow_config;
#endif

    /*
        if this is set to TRUE, then _handle_para_free(handle) should be
        called later to make sure all parameter lists are freed
    */
    u32                         b_keep_match_para;

    icli_runtime_t              runtime;

} icli_session_runtime_data_t;

typedef struct {
    u32                         session_id;
    u32                         in_used;

    /* runtime data */
    icli_session_runtime_data_t runtime_data;

    /* config data */
#if 1 /* CP, 2012/09/14 10:38, conf */
    // link to icli_conf_data_t.session_config
    icli_session_config_data_t  *config_data;
#else
    icli_session_config_data_t  config_data;
#endif

    /* APP config data */
    icli_session_open_data_t    open_data;

} icli_session_handle_t;

/*
==============================================================================

    Public Function

==============================================================================
*/
/*
    initialization

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_session_init(void);

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
i32 vtss_icli_session_open(
    IN  icli_session_open_data_t    *open_data,
    OUT u32                         *session_id
);

/*
    ICLI engine for each session

    INPUT
        session_id : session ID

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_session_engine(
    IN i32      session_id
);

/*
    close a ICLI session

    INPUT
        session_id : session ID from vtss_icli_session_open()

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_session_close(
    IN u32  session_id
);

/*
    get session handle according to session ID

    INPUT
        session_id : the session ID

    OUTPUT
        n/a

    RETURN
        not NULL : icli_session_handle_t *
        NULL     : failed to get

    COMMENT
        n/a
*/
icli_session_handle_t *vtss_icli_session_handle_get(
    IN u32  session_id
);

/*
    get if the session is alive or not

    INPUT
        session_id : the session ID

    OUTPUT
        n/a

    RETURN
        TRUE  : alive
        FALSE : dead

    COMMENT
        n/a
*/
BOOL vtss_icli_session_alive(
    IN u32  session_id
);

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
u32 vtss_icli_session_max_cnt_get(void);

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
i32 vtss_icli_session_max_cnt_set(
    IN u32 max_sessions
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
i32 vtss_icli_session_mode_get(
    IN  u32                 session_id,
    OUT icli_cmd_mode_t     *mode
);

/*
    make session entering the mode

    INPUT
        session_id : the session ID
        mode       : mode entering
        cmd_var    : the variables for the mode

    OUTPUT
        n/a

    RETURN
        >= 0 : successful and the return value is mode level
        -1   : failed

    COMMENT
        n/a
*/
i32 vtss_icli_session_mode_enter(
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
i32 vtss_icli_session_mode_exit(
    IN  u32     session_id
);

/*
    put string buffer to a session

    INPUT
        session_id : the session ID
        format     : output format
        arglist    : argument value list

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        none
*/
i32 vtss_icli_session_va_str_put(
    IN u32          session_id,
    IN const char   *format,
    IN va_list      arglist
);

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
i32 vtss_icli_session_usr_str_get(
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
i32 vtss_icli_session_ctrl_c_get(
    IN  u32     session_id,
    IN  u32     wait_time
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
i32 vtss_icli_session_data_get(
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
i32 vtss_icli_session_data_get_next(
    INOUT icli_session_data_t   *data
);

/*
    put one char directly to the session
    no line checking
    no buffering

    INPUT
        session_id : the session ID
        c          : the char for put

    OUTPUT
        n/a

    RETURN
        n/a
*/
void vtss_icli_session_char_put(
    IN  icli_session_handle_t   *handle,
    IN  char                    c
);

/*
    put string directly to the session
    no line checking
    no buffering

    INPUT
        session_id : the session ID
        str        : output string

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        none
*/
void vtss_icli_session_str_put(
    IN  icli_session_handle_t   *handle,
    IN  char                    *str
);

/*
    put string directly to the session
    no line checking
    no buffering

    INPUT
        handle : the session handle
        format : string format
        ...    : parameters of format

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
void vtss_icli_session_str_printf(
    IN  icli_session_handle_t   *handle,
    IN  const char              *format,
    IN  ...
);

/*
    put error message to a temp buffer(cmd_copy) for pending display

    INPUT
        handle : the session handle
        format : string format
        ...    : parameters of format

    OUTPUT
        n/a

    RETURN
        TRUE  - successful
        FALSE - failed

    COMMENT
        n/a
*/
BOOL vtss_icli_session_error_printf(
    IN  icli_session_handle_t   *handle,
    IN  const char              *format,
    IN  ...
);

#if 1 /* CP, 2012/09/11 14:08, Use thread information to get session ID */
/*
    get session ID by thread ID

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        my session ID    - successful
        ICLI_SESSION_CNT - failed

    COMMENT
        n/a
*/
u32 vtss_icli_session_self_id(
    void
);
#endif

/*
    reset session config data to default

    INPUT
        config_data - session config data to reset default

    OUTPUT
        n/a

    RETURN
        TRUE  - successful
        FALSE - failed

    COMMENT
        n/a
*/
BOOL vtss_icli_session_config_data_default(
    IN icli_session_config_data_t   *config_data
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
i32 vtss_icli_session_suac_get(
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
i32 vtss_icli_session_suac_set(
    IN  u32     session_id,
    IN  BOOL    suac
);

#endif

//****************************************************************************
#endif //__VTSS_ICLI_SESSION_H__

