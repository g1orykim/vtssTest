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
    > CP.Wang, 05/29/2013 11:47
        - create

==============================================================================
*/
/*
==============================================================================

    Include File

==============================================================================
*/
#include "vtss_icli.h"
#include <time.h>

/*
==============================================================================

    Constant

==============================================================================
*/
#define __ICLI_CR_STR               "<cr>"
#define __ICLI_HELP_MAX_LEN         256

/*
==============================================================================

    Constant

==============================================================================
*/
#define _CWORD_HELP_MAX_LEN     2048

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
static icli_session_handle_t    g_session_handle[ICLI_SESSION_CNT];
static u32                      g_max_sessions;
static char                     g_cword_help[_CWORD_HELP_MAX_LEN + 4];

/*
==============================================================================

    Static Function

==============================================================================
*/

static void _goback_exec_mode(
    IN icli_session_handle_t   *handle
)
{
    i32     rc = 1;

    while ( rc ) {
        rc = vtss_icli_session_mode_exit( handle->session_id );
        if ( rc < 0 ) {
            break;
        }
    }
}

static void _runtime_data_default(
    IN icli_session_handle_t   *handle
)
{
    u32     i;

    for ( i = 0; i < (ICLI_HISTORY_CMD_CNT - 1); i++ ) {
        handle->runtime_data.history_cmd_buf[i].next = &( handle->runtime_data.history_cmd_buf[i + 1] );
    }
    handle->runtime_data.history_cmd_free_list = &( handle->runtime_data.history_cmd_buf[0] );

    handle->runtime_data.err_display_mode = ICLI_ERR_DISPLAY_MODE_PRINT;

    if ( handle->config_data->width == 0 ) {
        handle->runtime_data.width = ICLI_MAX_WIDTH;
    } else if ( handle->config_data->width < ICLI_MIN_WIDTH ) {
        handle->runtime_data.width = ICLI_MIN_WIDTH;
    } else {
        handle->runtime_data.width = handle->config_data->width;
    }

    if ( handle->config_data->lines == 0 ) {
        handle->runtime_data.lines = 0;
    } else if ( handle->config_data->lines < ICLI_MIN_LINES ) {
        handle->runtime_data.lines = ICLI_MIN_LINES;
    } else {
        handle->runtime_data.lines = handle->config_data->lines;
    }
}

static void _runtime_data_clear(
    IN icli_session_handle_t   *handle
)
{
    /* free mode */
    _goback_exec_mode( handle );

    /* free match_para */
    if ( handle->runtime_data.match_para ) {
        vtss_icli_exec_para_list_free( &(handle->runtime_data.match_para) );
    }

    /* free cmd_var */
    if ( handle->runtime_data.cmd_var ) {
        vtss_icli_exec_para_list_free( &(handle->runtime_data.cmd_var) );
    }

    /* free match_sort_list */
    if ( handle->runtime_data.match_sort_list ) {
        vtss_icli_exec_para_list_free( &(handle->runtime_data.match_sort_list) );
    }

    /* clear to 0 */
    memset(&(handle->runtime_data), 0, sizeof(icli_session_runtime_data_t));

    /* set to default */
    _runtime_data_default( handle );
}

static i32 _session_get(void)
{
    u32     i;

    for ( i = 0; i < g_max_sessions; ++i ) {
        if ( g_session_handle[i].in_used == FALSE ) {
            g_session_handle[i].session_id = i;
            g_session_handle[i].in_used    = TRUE;
            return (i32)i;
        }
    }
    return -1;
}

static void _session_free(
    IN i32      session_id
)
{
    icli_session_handle_t   *handle = &(g_session_handle[session_id]);

    // clear runtime data
    _runtime_data_clear( handle );
    // make session available
    handle->in_used = FALSE;
}

/*
    init APP session
*/
static BOOL _usr_app_init(
    IN  icli_session_handle_t   *handle
)
{
    icli_session_app_init_t *app_init;
    i32                     app_id;
    BOOL                    b;

    if ( handle->open_data.app_init == NULL ) {
        return TRUE;
    }

    app_init = handle->open_data.app_init;
    app_id = handle->open_data.app_id;

    icli_sema_give();

    b = app_init( app_id );

    icli_sema_take();

    if ( b == FALSE ) {
        T_E("fail to init app\n");
    }
    return b;
}

/*
    close APP session
*/
static void _usr_app_close(
    IN  icli_session_handle_t           *handle,
    IN  icli_session_close_reason_t     reason
)
{
    icli_session_app_close_t    *app_close;
    i32                         app_id;

    if ( handle->open_data.app_close == NULL ) {
        return;
    }

    app_close = handle->open_data.app_close;
    app_id = handle->open_data.app_id;

    icli_sema_give();

    app_close( app_id, reason );

    icli_sema_take();
}

static void _history_cmd_add(
    IN  icli_session_handle_t   *handle
)
{
    icli_history_cmd_t  *hcmd;

    /* history command is disable */
    if ( ICLI_HISTORY_MAX_CNT == 0 ) {
        return;
    }

    /* empty command */
    if ( vtss_icli_str_len(handle->runtime_data.cmd) == 0 ) {
        return;
    }

    /* command is the same with the latest history */
    if ( ! vtss_icli_sutil_history_cmd_empty(handle) ) {
        if ( vtss_icli_str_cmp(handle->runtime_data.cmd, handle->runtime_data.history_cmd_head->cmd) == 0 ) {
            return;
        }
    }

    if ( vtss_icli_sutil_history_cmd_cnt(handle) >= ICLI_HISTORY_MAX_CNT ) {
        ___NEXT( handle->runtime_data.history_cmd_head );
    } else {
        hcmd = vtss_icli_sutil_history_cmd_alloc( handle );
        if ( hcmd ) {
            if ( handle->runtime_data.history_cmd_head ) {
                hcmd->next = handle->runtime_data.history_cmd_head->next;
                hcmd->prev = handle->runtime_data.history_cmd_head;

                handle->runtime_data.history_cmd_head->next = hcmd;
                hcmd->next->prev = hcmd;
            } else {
                hcmd->next = hcmd;
                hcmd->prev = hcmd;
            }
            handle->runtime_data.history_cmd_head = hcmd;
        } else {
            // FULL !!!
            ___NEXT( handle->runtime_data.history_cmd_head );
        }
    }
    (void)vtss_icli_str_cpy(handle->runtime_data.history_cmd_head->cmd, handle->runtime_data.cmd);
}

static void _cmd_display(
    IN  icli_session_handle_t   *handle
)
{
    /* usr input */
    switch ( _SESSON_INPUT_STYLE ) {
    case ICLI_INPUT_STYLE_SINGLE_LINE :
        vtss_icli_c_cmd_display( handle );
        break;

    case ICLI_INPUT_STYLE_MULTIPLE_LINE :
        vtss_icli_a_cmd_display( handle );
        break;

    case ICLI_INPUT_STYLE_SIMPLE :
        vtss_icli_z_cmd_display( handle );
        break;

    default :
        T_E("invalid input_style = %d\n", _SESSON_INPUT_STYLE);
        break;
    }
}

static void _cmd_redisplay(
    IN  icli_session_handle_t   *handle
)
{
    /* usr input */
    switch ( _SESSON_INPUT_STYLE ) {
    case ICLI_INPUT_STYLE_SINGLE_LINE :
        vtss_icli_c_cmd_redisplay( handle );
        break;

    case ICLI_INPUT_STYLE_MULTIPLE_LINE :
        vtss_icli_a_cmd_redisplay( handle );
        break;

    case ICLI_INPUT_STYLE_SIMPLE :
        vtss_icli_z_cmd_display( handle );
        break;

    default :
        T_E("invalid input_style = %d\n", _SESSON_INPUT_STYLE);
        break;
    }
}

/*
    get command from usr input
    1. function key will be provided
    2. the command will be stored in icli_session_handle_t.cmd

    INPUT
        session_id : the session ID

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 _usr_cmd_get(
    IN  u32     session_id
)
{
    icli_session_handle_t   *handle;
    i32                     rc;

    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    /* get handle */
    handle = &g_session_handle[ session_id ];

    /* check alive or not */
    if ( handle->runtime_data.alive == FALSE ) {
        T_E("session_id = %d is not alive\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    rc = ICLI_RC_ERROR;
    handle->runtime_data.after_cmd_act = ICLI_AFTER_CMD_ACT_NONE;

    /* usr input */
    switch ( _SESSON_INPUT_STYLE ) {
    case ICLI_INPUT_STYLE_SINGLE_LINE :
        rc = vtss_icli_c_usr_cmd_get( handle );
        break;

    case ICLI_INPUT_STYLE_MULTIPLE_LINE :
        rc = vtss_icli_a_usr_cmd_get( handle );
        break;

    case ICLI_INPUT_STYLE_SIMPLE :
        rc = vtss_icli_z_usr_cmd_get( handle );
        //line count
        handle->runtime_data.line_cnt = (handle->runtime_data.prompt_len + handle->runtime_data.cmd_len + 1) / (i32)(handle->runtime_data.width) + 1;
        break;

    default :
        T_E("invalid input_style = %d\n", _SESSON_INPUT_STYLE);
        break;
    }

    return rc;

}

static void _prompt_display(
    IN icli_session_handle_t   *handle
)
{
    icli_cmd_mode_t     mode;
    char                *mode_prompt;
    char                prompt[ICLI_NAME_MAX_LEN + ICLI_PROMPT_MAX_LEN + 10];
    i32                 len = ICLI_NAME_MAX_LEN + ICLI_PROMPT_MAX_LEN + 5;
    char                dev_name[ICLI_NAME_MAX_LEN + 1];
    char                *dname;

#if 1 /* CP, 2012/09/04 13:33, switch information for prompt when stacking */
    icli_switch_info_t  switch_info;

    // initialize first for the case of returning FALSE
    switch_info.b_master = TRUE;
    switch_info.usid     = 0;

    // get switch information
    icli_switch_info_get( &switch_info );
#endif

    //get mode prompt
    mode = handle->runtime_data.mode_para[ handle->runtime_data.mode_level ].mode;
    mode_prompt = vtss_icli_mode_prompt_get( mode );

    //get device name
#if 1 /* CP, 2012/10/08 14:31, debug command, debug prompt */
    dname = vtss_icli_debug_prompt_get();
    if ( ICLI_IS_(EOS, *dname) ) {
        dname = vtss_icli_dev_name_get();
    }
    (void)vtss_icli_str_ncpy(dev_name, dname, ICLI_NAME_MAX_LEN);
    dev_name[ICLI_NAME_MAX_LEN] = 0;
#else
    dev_name = vtss_icli_dev_name_get();
#endif

    /* pack prompt */
    memset(prompt, 0, sizeof(prompt));

#if 1 /* CP, 2012/09/04 13:33, switch information for prompt when stacking */
    if ( switch_info.b_master ) {
        if ( vtss_icli_str_len(mode_prompt) ) {
            (void)icli_snprintf(prompt, len,
                                "%s(%s)%c ", dev_name, mode_prompt, handle->runtime_data.privilege >= handle->config_data->privileged_level ? '#' : '>');
        } else {
            (void)icli_snprintf(prompt, len, "%s%c ",
                                dev_name, handle->runtime_data.privilege >= handle->config_data->privileged_level ? '#' : '>');
        }
    } else {
        if ( vtss_icli_str_len(mode_prompt) ) {
            (void)icli_snprintf(prompt, len,
                                "Slave_%u(%s)%c ", switch_info.usid, mode_prompt, handle->runtime_data.privilege >= handle->config_data->privileged_level ? '#' : '>');
        } else {
            (void)icli_snprintf(prompt, len, "Slave_%u%c ",
                                switch_info.usid, handle->runtime_data.privilege >= handle->config_data->privileged_level ? '#' : '>');
        }
    }
#else
    if ( vtss_icli_str_len(mode_prompt) ) {
        (void)icli_snprintf(prompt, len,
                            "%s(%s)%c ", dev_name, mode_prompt, handle->runtime_data.privilege >= handle->config_data->privileged_level ? '#' : '>');
    } else {
        (void)icli_snprintf(prompt, len, "%s%c ",
                            dev_name, handle->runtime_data.privilege >= handle->config_data->privileged_level ? '#' : '>');
    }
#endif

    vtss_icli_session_str_put(handle, prompt);
    handle->runtime_data.prompt_len = vtss_icli_str_len( prompt );
}

static char *_cmd_word_get(
    IN  icli_session_handle_t   *handle,
    IN  icli_parsing_node_t     *node
)
{
    icli_runtime_cb_t   *runtime_cb;
    char                *byword;
    u32                 b;
    icli_byword_t       *bw;

    if ( node->type != ICLI_VARIABLE_KEYWORD ) {
        /* check runtime first */
        runtime_cb = node->node_property.cmd_property->runtime_cb[node->word_id];
        if ( runtime_cb ) {
            b = vtss_icli_exec_runtime( handle, runtime_cb, ICLI_ASK_BYWORD, &(handle->runtime_data.runtime) );
            if ( b ) {
                byword = handle->runtime_data.runtime.byword;
                return byword;
            }
        }

        /* check command property */
        bw = &(node->node_property.cmd_property->byword[node->word_id]);
        byword = bw->word;
        if ( vtss_icli_str_len(byword) ) {
            if ( bw->para_cnt ) {
                /*
                    this byword does not need to be freed because it is linked
                    into node_property of the parsing tree and it can be used
                    forever. And this is allocated once only because bw->para_cnt
                    will be set to 0 and next time it will not go into
                    if ( bw->para_cnt ) anymore.
                */
                byword = (char *)icli_malloc(ICLI_BYWORD_MAX_LEN + 1);
                if ( byword == NULL ) {
                    T_E("memory insufficient\n");
                    return FALSE;
                }
                memset(byword, 0, ICLI_BYWORD_MAX_LEN + 1);
                if ( bw->para_cnt == 1 ) {
                    (void)icli_snprintf(byword, ICLI_BYWORD_MAX_LEN, bw->word, bw->para[0]);
                } else {
                    (void)icli_snprintf(byword, ICLI_BYWORD_MAX_LEN, bw->word, bw->para[0], bw->para[1]);
                }
                bw->word = byword;
                bw->para_cnt = 0;
            }
            return byword;
        }
    }
    return node->word;
}

static i32 _cword_max_len_get(
    IN icli_session_handle_t    *handle,
    IN icli_parameter_t         *p
)
{
    icli_runtime_cb_t   *runtime_cb;
    BOOL                b;
    char                *cword;
    i32                 max_len;
    i32                 len;
    i32                 i;

    cword      = p->value.u.u_cword;
    runtime_cb = p->match_node->node_property.cmd_property->runtime_cb[p->word_id];

    b = vtss_icli_exec_cword_runtime_get(handle, runtime_cb, &(handle->runtime_data.runtime));
    if ( b == FALSE ) {
        return -1;
    }

    if ( handle->runtime_data.runtime.cword[0] == NULL ) {
        return -1;
    }

    max_len = -1;
    for ( i = 0; i < ICLI_CWORD_MAX_CNT; ++i ) {
        if ( handle->runtime_data.runtime.cword[i] == NULL ) {
            break;
        }

        if ( *cword ) {
            if ( vtss_icli_str_sub(cword, handle->runtime_data.runtime.cword[i], 0, NULL) == -1 ) {
                continue;
            }
        }
        len = vtss_icli_str_len( handle->runtime_data.runtime.cword[i] );
        if ( len > max_len ) {
            max_len = len;
        }
    }

    return max_len;
}

static i32 _max_word_len_get(
    IN  icli_session_handle_t   *handle,
    IN  BOOL                    b_use_exec_present
)
{
    i32                 max_word_len;
    i32                 len;
    i32                 l;
    icli_parameter_t    *p;
    icli_port_type_t    port_type;

    max_word_len = 0;
    for ( p = handle->runtime_data.match_sort_list; p != NULL; p = p->next ) {
        len = 0;
        switch ( p->match_node->type ) {
        case ICLI_VARIABLE_PORT_TYPE:
            for ( port_type = 1; port_type < ICLI_PORT_TYPE_MAX; ++port_type ) {
                if ( b_use_exec_present ) {
                    if ( vtss_icli_exec_port_type_present(handle, p->match_node, port_type) ) {
                        l = vtss_icli_str_len( vtss_icli_variable_port_type_get_name(port_type) );
                        if ( l > len ) {
                            len = l;
                        }
                    }
                } else {
                    if ( vtss_icli_port_type_present(port_type) ) {
                        l = vtss_icli_str_len( vtss_icli_variable_port_type_get_name(port_type) );
                        if ( l > len ) {
                            len = l;
                        }
                    }
                }
            }
            break;

        case ICLI_VARIABLE_CWORD:
            if ( b_use_exec_present ) {
                len = _cword_max_len_get( handle, p );
                if ( len == -1 ) {
                    len = vtss_icli_str_len( _cmd_word_get(handle, p->match_node) );
                }
            } else {
                len = vtss_icli_str_len("CWORD");
            }
            break;

        default:
            len = vtss_icli_str_len( _cmd_word_get(handle, p->match_node) );
            break;
        }
        if ( len > max_word_len ) {
            max_word_len = len;
        }
    }
    if ( handle->runtime_data.b_cr ) {
        if ( (i32)vtss_icli_str_len(__ICLI_CR_STR) > max_word_len ) {
            max_word_len = vtss_icli_str_len( __ICLI_CR_STR );
        }
    }
    return max_word_len;
}

static void _line_clear(
    IN  icli_session_handle_t   *handle
)
{
    /* usr input */
    switch ( _SESSON_INPUT_STYLE ) {
    case ICLI_INPUT_STYLE_SINGLE_LINE :
        vtss_icli_c_line_clear( handle );
        break;

    case ICLI_INPUT_STYLE_MULTIPLE_LINE :
        vtss_icli_a_line_clear( handle );
        break;

    case ICLI_INPUT_STYLE_SIMPLE :
        vtss_icli_z_line_clear( handle );
        break;

    default :
        T_E("invalid input_style = %d\n", _SESSON_INPUT_STYLE);
        break;
    }
}

#define __TAB_WORD_DISPLAY(word) \
    if ( i && (i % word_cnt == 0) ) { \
        ICLI_PUT_NEWLINE; \
    } \
    w = word; \
    vtss_icli_session_str_put(handle, w); \
    j = max_word_len - vtss_icli_str_len(w); \
    for ( ; j > 0; --j ) { \
        ICLI_PUT_SPACE; \
    } \
    vtss_icli_session_str_put(handle, "  "); \
    ++i;

#define __TAB_WORD_SET(word) \
    for ( j = 0; j < ICLI_CWORD_MAX_CNT; ++j ) { \
        if ( irt.cword[j] == NULL ) { \
            break; \
        } \
    } \
    if ( j < ICLI_CWORD_MAX_CNT ) { \
        irt.cword[j] = word; \
    } else { \
        T_E("tab words are full\n"); \
    }

static BOOL _exactly_in_cword(
    IN icli_session_handle_t    *handle,
    IN icli_parameter_t         *p
)
{
    icli_runtime_cb_t   *runtime_cb;
    BOOL                b;
    i32                 r;
    i32                 exact_cnt;
    i32                 sub_cnt;
    char                *cword;
    i32                 i;

    cword      = p->value.u.u_cword;
    runtime_cb = p->match_node->node_property.cmd_property->runtime_cb[p->word_id];

    b = vtss_icli_exec_cword_runtime_get(handle, runtime_cb, &(handle->runtime_data.runtime));
    if ( b == FALSE ) {
        return FALSE;
    }

    if ( handle->runtime_data.runtime.cword[0] == NULL ) {
        return FALSE;
    }

    exact_cnt = 0;
    sub_cnt   = 0;

    for ( i = 0; i < ICLI_CWORD_MAX_CNT; ++i ) {
        if ( handle->runtime_data.runtime.cword[i] == NULL ) {
            break;
        }

        r = vtss_icli_str_sub(cword, handle->runtime_data.runtime.cword[i], 0, NULL);
        switch ( r ) {
        case 1:
            ++sub_cnt;
            break;
        case 0:
            ++exact_cnt;
            break;
        case -1:
        default:
            break;
        }
    }

    if ( exact_cnt ) {
        if ( exact_cnt > 1 ) {
            return FALSE;
        }
    } else if ( sub_cnt ) {
        if ( sub_cnt > 1 ) {
            return FALSE;
        }
    }

    return TRUE;
}

static void _keyword_cword_copy(
    IN icli_session_handle_t    *handle,
    IN icli_parameter_t         *p,
    IN char                     *w
)
{
    icli_runtime_cb_t   *runtime_cb;
    BOOL                b;
    i32                 r;
    char                *cword;
    i32                 i;
    i32                 k;

    cword      = p->value.u.u_cword;
    runtime_cb = p->match_node->node_property.cmd_property->runtime_cb[p->word_id];

    b = vtss_icli_exec_cword_runtime_get(handle, runtime_cb, &(handle->runtime_data.runtime));
    if ( b == FALSE ) {
        return;
    }

    if ( handle->runtime_data.runtime.cword[0] == NULL ) {
        return;
    }

    k = -1;
    for ( i = 0; i < ICLI_CWORD_MAX_CNT; ++i ) {
        if ( handle->runtime_data.runtime.cword[i] == NULL ) {
            break;
        }

        r = vtss_icli_str_sub(cword, handle->runtime_data.runtime.cword[i], 0, NULL);
        switch ( r ) {
        case 1:
            k = i;
            break;

        case 0:
            (void)vtss_icli_str_cpy(w, handle->runtime_data.runtime.cword[i]);
            return;

        case -1:
        default:
            break;
        }
    }

    if ( k >= 0 ) {
        (void)vtss_icli_str_cpy(w, handle->runtime_data.runtime.cword[k]);
    }
}

static BOOL _cword_tab_get(
    IN  icli_session_handle_t   *handle,
    IN  icli_parameter_t        *p,
    OUT i16                     *cindex
)
{
    icli_runtime_cb_t   *runtime_cb;
    BOOL                b;
    char                *cword;
    i16                 i;
    i32                 j;

    cword      = p->value.u.u_cword;
    runtime_cb = p->match_node->node_property.cmd_property->runtime_cb[p->word_id];

    b = vtss_icli_exec_cword_runtime_get(handle, runtime_cb, &(handle->runtime_data.runtime));
    if ( b == FALSE ) {
        return FALSE;
    }

    if ( handle->runtime_data.runtime.cword[0] == NULL ) {
        return FALSE;
    }

    j = 0;
    for ( i = 0; i < ICLI_CWORD_MAX_CNT; ++i ) {
        if ( handle->runtime_data.runtime.cword[i] == NULL ) {
            break;
        }

        if ( *cword ) {
            if ( vtss_icli_str_sub(cword, handle->runtime_data.runtime.cword[i], 0, NULL) == -1 ) {
                continue;
            }
        }
        cindex[j++] = i;
    }

    cindex[j] = -1;
    return TRUE;
}

static char *_cword_prefix_get(
    IN  icli_session_handle_t   *handle,
    IN  icli_parameter_t        *p,
    OUT u32                     *prefix_num
)
{
    icli_runtime_cb_t   *runtime_cb;
    BOOL                b;
    char                *cword;
    i32                 i;
    u32                 len;
    char                *c;

    cword      = p->value.u.u_cword;
    runtime_cb = p->match_node->node_property.cmd_property->runtime_cb[p->word_id];

    b = vtss_icli_exec_cword_runtime_get(handle, runtime_cb, &(handle->runtime_data.runtime));
    if ( b == FALSE ) {
        return NULL;
    }

    if ( handle->runtime_data.runtime.cword[0] == NULL ) {
        return NULL;
    }

    len = 0;
    c   = NULL;
    for ( i = 0; i < ICLI_CWORD_MAX_CNT; ++i ) {
        if ( handle->runtime_data.runtime.cword[i] == NULL ) {
            break;
        }

        if ( *cword ) {
            if ( vtss_icli_str_sub(cword, handle->runtime_data.runtime.cword[i], 0, NULL) == -1 ) {
                continue;
            }
        }

        if ( c ) {
            len = vtss_icli_str_prefix( handle->runtime_data.runtime.cword[i], c, len, 0);
        } else {
            c = handle->runtime_data.runtime.cword[i];
            len = vtss_icli_str_len( c );
        }
    }

    if ( len == 0 ) {
        return NULL;
    }

    *prefix_num = len;
    return c;
}

static i32 _match_cnt_in_cword(
    IN  icli_session_handle_t   *handle,
    IN  icli_parameter_t        *p,
    OUT icli_runtime_t          *runtime
)
{
    icli_runtime_cb_t   *runtime_cb;

    runtime_cb = p->match_node->node_property.cmd_property->runtime_cb[p->word_id];
    return vtss_icli_exec_cword_match_cnt( handle, p->value.u.u_cword, runtime_cb, runtime, NULL, NULL );
}

static void _tab_display(
    IN  icli_session_handle_t   *handle
)
{
    i32                 max_word_len;
    i32                 word_cnt;
    i32                 i;
    i32                 j;
    i32                 k;
    icli_parameter_t    *p = NULL;
    icli_parameter_t    *q;
    char                *w = NULL;
    char                *c;
    char                *s;
    u32                 keyword = 0;
    icli_port_type_t    port_type;
    u32                 prefix_num;
    u32                 s_num;
    BOOL                b_last_cmd;
    i16                 cindex[ICLI_CWORD_MAX_CNT];
    icli_runtime_t      irt;

#if 1 /* CP, 2012/10/16 14:59, Bugzilla#10006 */
    u32                 cmd_len;
    u32                 width;
#endif

#if 1 /* CP, 08/15/2013 11:22, Bugzilla#12458 - port type id should not get wildcard */
    icli_port_type_t    start_type;
#endif

    if ( handle->runtime_data.match_sort_list ) {
        if ( handle->runtime_data.cmd_len ) {
            w = &( handle->runtime_data.cmd[handle->runtime_data.cmd_len - 1] );
            if ( ICLI_IS_(SPACE, *w) ) {
                keyword = 0;
            } else {
                keyword = 1;
            }
        } else {
            keyword = 0;
        }

        if ( keyword ) {
            if ( handle->runtime_data.match_sort_list->next ) {
                /*
                    multiple match,
                    if only one keyword then complete the keyword
                 */
                keyword = 0;
                for ( q = handle->runtime_data.match_sort_list; q != NULL; q = q->next ) {
                    switch ( q->match_node->type ) {
                    case ICLI_VARIABLE_KEYWORD:
                    case ICLI_VARIABLE_PORT_TYPE:
                    case ICLI_VARIABLE_GREP:
                    case ICLI_VARIABLE_GREP_BEGIN:
                    case ICLI_VARIABLE_GREP_INCLUDE:
                    case ICLI_VARIABLE_GREP_EXCLUDE:
                        p = q;
                        ++keyword;
                        break;

                    case ICLI_VARIABLE_CWORD:
                        k = _match_cnt_in_cword(handle, q, &(handle->runtime_data.runtime));
                        if ( k > 0 ) {
                            p = q;
                            keyword += k;
                        }
                        break;

                    default:
                        break;
                    }
                }
                if ( keyword != 1 ) {
                    keyword = 0;
                }
            } else {
                /* single match */
                p = handle->runtime_data.match_sort_list;

                switch ( p->match_node->type ) {
                case ICLI_VARIABLE_KEYWORD:
                case ICLI_VARIABLE_PORT_TYPE:
                case ICLI_VARIABLE_GREP:
                case ICLI_VARIABLE_GREP_BEGIN:
                case ICLI_VARIABLE_GREP_INCLUDE:
                case ICLI_VARIABLE_GREP_EXCLUDE:
                    break;

                case ICLI_VARIABLE_CWORD:
                    if ( _exactly_in_cword(handle, p) == FALSE ) {
                        keyword = 0;
                    }
                    break;

                default:
                    keyword = 0;
                    break;
                }
            }
        }
    }

    if ( keyword ) {
        if ( p == NULL ) {
            T_E("p == NULL\n");
            vtss_icli_sutil_tab_reset( handle );
            return;
        }

        handle->runtime_data.tab_cnt = 1;

        if ( (p->match_node->type == ICLI_VARIABLE_PORT_TYPE) &&
             (handle->runtime_data.tab_port_type == ICLI_PORT_TYPE_NONE) ) {
            handle->runtime_data.tab_port_type = vtss_icli_port_next_present_type( ICLI_PORT_TYPE_NONE );
        }

        // find position to append
        if ( handle->runtime_data.cmd_len ) {
            w = &( handle->runtime_data.cmd[handle->runtime_data.cmd_len - 1] );
            if ( ICLI_IS_(SPACE, *w) ) {
                ++w;
            } else {
                *w = ICLI_EOS;
                for ( i = handle->runtime_data.cmd_len - 1; i > 0; --i ) {
                    --w;
                    if ( ICLI_IS_(SPACE, *w) ) {
                        ++w;
                        break;
                    } else {
                        *w = ICLI_EOS;
                    }
                }
            }
        } else {
            w = handle->runtime_data.cmd;
        }

        // copy keyword
        switch (p->match_node->type) {
        case ICLI_VARIABLE_KEYWORD:
            (void)vtss_icli_str_cpy(w, p->match_node->word);
            break;

        case ICLI_VARIABLE_PORT_TYPE:
            if ( p->value.type == ICLI_VARIABLE_PORT_TYPE ) {
                (void)vtss_icli_str_cpy(w, vtss_icli_variable_port_type_get_name(p->value.u.u_port_type));
            } else {
                (void)vtss_icli_str_cpy(w, vtss_icli_variable_port_type_get_name(handle->runtime_data.tab_port_type));
            }
            break;

        case ICLI_VARIABLE_GREP:
            (void)vtss_icli_str_cpy(w, ICLI_GREP_KEYWORD);
            break;

        case ICLI_VARIABLE_GREP_BEGIN:
            (void)vtss_icli_str_cpy(w, ICLI_GREP_BEGIN_KEYWORD);
            break;

        case ICLI_VARIABLE_GREP_INCLUDE:
            (void)vtss_icli_str_cpy(w, ICLI_GREP_INCLUDE_KEYWORD);
            break;

        case ICLI_VARIABLE_GREP_EXCLUDE:
            (void)vtss_icli_str_cpy(w, ICLI_GREP_EXCLUDE_KEYWORD);
            break;

        case ICLI_VARIABLE_CWORD:
            _keyword_cword_copy(handle, p, w);
            break;

        default:
            break;
        }

        /* append " " at the end */
        for ( ; ICLI_NOT_(EOS, *w); ++w ) {
            ;
        }
        *w++ = ICLI_SPACE;
        *w   = ICLI_EOS;

        // clear line
        _line_clear( handle );

#if 1 /* CP, 2012/10/16 14:59, Bugzilla#10006 */
        // check if right scroll
        cmd_len = vtss_icli_str_len( handle->runtime_data.cmd );
        width = handle->runtime_data.width - handle->runtime_data.prompt_len - 1;
        if ( (cmd_len - handle->runtime_data.start_pos) > (width - _SCROLL_FACTOR) ) {
            handle->runtime_data.start_pos = cmd_len - (width - _SCROLL_FACTOR);
        }
#endif

        // display again
        _cmd_display( handle );

    } else {

        ICLI_PLAY_BELL;

        ICLI_PUT_NEWLINE;

        i = 0;
        word_cnt = 0;
        if ( handle->runtime_data.match_sort_list ) {
            memset( &irt, 0, sizeof(icli_runtime_t) );

            max_word_len = _max_word_len_get(handle, TRUE);
            word_cnt = ((i32)(handle->runtime_data.width) - 1) / (max_word_len + 2);
            for ( i = 0, p = handle->runtime_data.match_sort_list; p != NULL; p = p->next ) {
                switch ( p->match_node->type ) {
                case ICLI_VARIABLE_PORT_TYPE:
#if 1 /* CP, 08/15/2013 11:22, Bugzilla#12458 - port type id should not get wildcard */
                    if ( p->match_node->child && p->match_node->child->type == ICLI_VARIABLE_PORT_TYPE_ID ) {
                        start_type = ICLI_PORT_TYPE_FAST_ETHERNET;
                    } else {
                        start_type = ICLI_PORT_TYPE_ALL;
                    }
                    for ( port_type = start_type; port_type < ICLI_PORT_TYPE_MAX; ++port_type ) {
                        if ( vtss_icli_exec_port_type_present(handle, p->match_node, port_type) ) {
                            if ( (p->value.type == ICLI_VARIABLE_MAX) ||
                                 (p->value.type == ICLI_VARIABLE_PORT_TYPE && p->value.u.u_port_type == port_type)) {
                                __TAB_WORD_SET( vtss_icli_variable_port_type_get_name(port_type) );
                            }
                        }
                    }
#else
                    for ( port_type = 1; port_type < ICLI_PORT_TYPE_MAX; ++port_type ) {
                        if ( vtss_icli_exec_port_type_present(handle, p->match_node, port_type) ) {
                            if ( (p->value.type == ICLI_VARIABLE_MAX) ||
                                 (p->value.type == ICLI_VARIABLE_PORT_TYPE && p->value.u.u_port_type == port_type)) {
                                __TAB_WORD_SET( vtss_icli_variable_port_type_get_name(port_type) );
                            }
                        }
                    }
#endif
                    break;

                case ICLI_VARIABLE_CWORD:
                    cindex[0] = -1;
                    if ( _cword_tab_get(handle, p, cindex) ) {
                        for ( k = 0; k < ICLI_CWORD_MAX_CNT; ++k ) {
                            if ( cindex[k] == -1 ) {
                                break;
                            }
                            __TAB_WORD_SET( handle->runtime_data.runtime.cword[ cindex[k] ] );
                        }
                    } else {
                        __TAB_WORD_SET( _cmd_word_get(handle, p->match_node) );
                    }
                    break;

                default:
                    __TAB_WORD_SET( _cmd_word_get(handle, p->match_node) );
                    break;
                }
            } // for i

            vtss_icli_exec_cword_runtime_sort( &irt );
            for ( k = 0; k < ICLI_CWORD_MAX_CNT; ++k ) {
                if ( irt.cword[k] == NULL ) {
                    break;
                }
                __TAB_WORD_DISPLAY( irt.cword[k] );
            } // for k
        }

        if ( handle->runtime_data.b_cr ) {
            if ( i && word_cnt && (i % word_cnt == 0) ) {
                ICLI_PUT_NEWLINE;
            }
            vtss_icli_session_str_put(handle, __ICLI_CR_STR);
        }
        ICLI_PUT_NEWLINE;

        /*
            process prefix to append last command word to the longest matched
        */
        keyword = 1;
        for ( p = handle->runtime_data.match_sort_list; p != NULL; p = p->next ) {
            switch ( p->match_node->type ) {
            case ICLI_VARIABLE_KEYWORD:
                break;

            case ICLI_VARIABLE_CWORD:
                if ( _cword_max_len_get(handle, p) == -1 ) {
                    keyword = 0;
                }
                break;

            default:
                keyword = 0;
                break;
            }
        }

        if ( keyword == 0 ) {
            vtss_icli_sutil_tab_reset( handle );
            return;
        }

        /* get the beginning of the last command word */
        b_last_cmd = FALSE;
        if ( handle->runtime_data.cmd_len ) {
            i = handle->runtime_data.cmd_len - 1;
            w = &( handle->runtime_data.cmd[i] );
            if ( ICLI_NOT_(SPACE, *w) ) {
                b_last_cmd = TRUE;
                for ( ; i > 0; --i ) {
                    --w;
                    if ( ICLI_IS_(SPACE, *w) ) {
                        ++w;
                        break;
                    }
                }
            }
        }

        if ( b_last_cmd == FALSE ) {
            vtss_icli_sutil_tab_reset( handle );
            return;
        }

        /* get the number of same prefix in multiple matches */
        if ( handle->runtime_data.match_sort_list ) {
            p = handle->runtime_data.match_sort_list;
            if ( p->match_node->type == ICLI_VARIABLE_KEYWORD ) {
                c = p->match_node->word;
                prefix_num = vtss_icli_str_len( c );
            } else {
                prefix_num = 0;
                c = _cword_prefix_get(handle, p, &prefix_num);
                if ( c == NULL ) {
                    T_E("Fail to get cword prefix\n");
                    vtss_icli_sutil_tab_reset( handle );
                    return;
                }
            }

            for ( ___NEXT(p); p != NULL; ___NEXT(p) ) {
                if ( p->match_node->type == ICLI_VARIABLE_KEYWORD ) {
                    prefix_num = vtss_icli_str_prefix( p->match_node->word, c, prefix_num, 0);
                } else {
                    s = _cword_prefix_get(handle, p, &s_num);
                    if ( s == NULL ) {
                        T_E("Fail to get cword prefix\n");
                        vtss_icli_sutil_tab_reset( handle );
                        return;
                    }

                    if ( s_num < prefix_num ) {
                        prefix_num = s_num;
                    }
                    prefix_num = vtss_icli_str_prefix( s, c, prefix_num, 0);
                }
            }

            j = vtss_icli_str_len(w);
            i = prefix_num - j;
            c = c + j;
            w = &( handle->runtime_data.cmd[handle->runtime_data.cmd_len] );
            for ( ; i > 0; --i ) {
                *w++ = *c++;
            }
            *w = ICLI_EOS;
        }

        /* reset handle */
        vtss_icli_sutil_tab_reset( handle );
    }
}

static char *_help_get(
    IN  icli_session_handle_t   *handle,
    IN  icli_parsing_node_t     *node,
    OUT BOOL                    *b_runtime
)
{
    u32                 b;
    char                *help;
    icli_runtime_cb_t   *runtime_cb;
    node_property_t     *np;

    /* check runtime first */
    runtime_cb = node->node_property.cmd_property->runtime_cb[node->word_id];
    if ( runtime_cb ) {
        b = vtss_icli_exec_runtime( handle, runtime_cb, ICLI_ASK_HELP, &(handle->runtime_data.runtime) );
        if ( b ) {
            help = handle->runtime_data.runtime.help;
            if ( b_runtime ) {
                *b_runtime = TRUE;
            }
            return help;
        }
    }

    /* check command property */
    help = "";
    for ( np = &(node->node_property); np; ___NEXT(np) ) {
        help = np->cmd_property->help[node->word_id];
        if ( *help ) {
            break;
        }
    }

    if ( b_runtime ) {
        *b_runtime = FALSE;
    }
    return help;
}

/*
    output formated string to a specific session

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
static void _session_printf(
    IN  icli_session_handle_t   *handle,
    IN  const char              *format,
    IN  ...
)
{
    va_list     arglist;

    switch ( handle->open_data.way ) {
    case ICLI_SESSION_WAY_CONSOLE:
    case ICLI_SESSION_WAY_TELNET:
    case ICLI_SESSION_WAY_SSH:
    case ICLI_SESSION_WAY_THREAD_CONSOLE:
    case ICLI_SESSION_WAY_THREAD_TELNET:
    case ICLI_SESSION_WAY_THREAD_SSH:
        if ( handle->runtime_data.line_mode == ICLI_LINE_MODE_BYPASS ) {
            return;
        }
        break;

    case ICLI_SESSION_WAY_APP_EXEC:
        return;

    default:
        return;
    }

    /*lint -e{530} ... 'arglist' is initialized by va_start() */
    va_start( arglist, format );
    (void)vtss_icli_session_va_str_put(handle->session_id, format, arglist);
    va_end( arglist );
}

#if 1 /* CP, 2012/09/25 10:47, <dscp> */
static void _help_display(
    IN  icli_session_handle_t   *handle,
    IN  char                    *word,
    IN  char                    *help
)
{
    i32                 max_word_len,
                        help_len,
                        space_len,
                        display_len,
                        n,
                        i;
    char                *c, *s;

    //4 for the gap to help string
    max_word_len = _max_word_len_get(handle, FALSE) + 4;

    //4 for first spaces and 4 for the gap to help string
    space_len    = 4 + max_word_len;

    //1 for EOS
    display_len  = handle->runtime_data.width - space_len - 1;

    //word
    _session_printf(handle, "    ");
    _session_printf(handle, word);

    for ( i = vtss_icli_str_len(word); i < max_word_len; ++i ) {
        _session_printf(handle, " ");
    }

    //help
    help_len = vtss_icli_str_len( help );
    if ( help_len > display_len ) {
        for ( c = help, n = 0, s = c; ; ++c, ++n ) {

            //count number of char in a word
            if ( ICLI_NOT_(SPACE, *c) && ICLI_NOT_(EOS, *c) ) {
                continue;
            }

            //check if over one line
            if ( n > display_len ) {
                n = c - s;

                _session_printf(handle, "\n");

                for ( i = 0; i < space_len; ++i ) {
                    _session_printf(handle, " ");
                }
                //skip the first space
                if ( ICLI_IS_(SPACE, *s) ) {
                    ++s;
                    --n;
                }
            }

            //display word
            for ( ; s != c; ++s ) {
                _session_printf(handle, "%c", *s);
            }

            //end of string
            if ( ICLI_IS_(EOS, *c) ) {
                _session_printf(handle, "\n");
                break;
            }
        }
    } else {
        _session_printf(handle, help);
        _session_printf(handle, "\n");
    }
}
#endif

static char *_port_list_str_get(
    IN  icli_port_type_t    port_type,
    OUT char                *str
)
{
    icli_stack_port_range_t     range;
    u32                         i;
    char                        *s;
    icli_switch_port_range_t    *spr;
    u16                         last_switch_id;
    u16                         last_begin_port;
    u16                         last_end_port;

    switch ( port_type ) {
    case ICLI_PORT_TYPE_ALL:
    case ICLI_PORT_TYPE_FAST_ETHERNET:
    case ICLI_PORT_TYPE_GIGABIT_ETHERNET:
    case ICLI_PORT_TYPE_2_5_GIGABIT_ETHERNET:
    case ICLI_PORT_TYPE_FIVE_GIGABIT_ETHERNET:
    case ICLI_PORT_TYPE_TEN_GIGABIT_ETHERNET:
        break;

    default:
        return NULL;
    }

    /* get port range */
    if ( vtss_icli_port_range_get(&range) == FALSE ) {
        return NULL;
    }

    last_switch_id  = 0;
    last_begin_port = 0;
    last_end_port   = 0;
    s = str;
    for ( i = 0; i < range.cnt; ++i ) {
        spr = &(range.switch_range[i]);
        if ( spr->port_type != (u8)port_type ) {
            continue;
        }
        if ( spr->switch_id != last_switch_id ) {
            if ( last_switch_id ) {
                /* not first */
                if ( last_begin_port == last_end_port ) {
                    icli_sprintf(s, ",%u/%u", spr->switch_id, spr->begin_port);
                } else {
                    icli_sprintf(s, "-%u,%u/%u", last_end_port, spr->switch_id, spr->begin_port);
                }
            } else {
                /* first */
                icli_sprintf(s, "%u/%u", spr->switch_id, spr->begin_port);
            }

        } else {
            /* check if port is continuous */
            if ( spr->begin_port == (last_end_port + 1) ) {
                /* continuous, only need to update last_end_port */
                last_end_port = spr->begin_port + spr->port_cnt - 1;
                continue;
            } else {
                /* not continuous */
                if ( last_begin_port == last_end_port ) {
                    icli_sprintf(s, ",%u", spr->begin_port);
                } else {
                    icli_sprintf(s, "-%u,%u", last_end_port, spr->begin_port);
                }
            }
        }

        /* update s */
        s = str + vtss_icli_str_len( str );

        /* update last data */
        last_switch_id  = spr->switch_id;
        last_begin_port = spr->begin_port;
        last_end_port   = spr->begin_port + spr->port_cnt - 1;
    }

    /* add last_end_port */
    if ( last_begin_port != last_end_port ) {
        icli_sprintf(s, "-%u", last_end_port);
    }

    return str;
}

static BOOL _port_help_display(
    IN  icli_session_handle_t   *handle,
    IN  char                    *cmd_word,
    IN  BOOL                    b_port_list
)
{
    char    cmd_help[__ICLI_HELP_MAX_LEN + 1];
    char    *s;

    switch ( handle->runtime_data.last_port_type ) {
#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
    case ICLI_PORT_TYPE_ALL:
#endif
    case ICLI_PORT_TYPE_FAST_ETHERNET:
    case ICLI_PORT_TYPE_GIGABIT_ETHERNET:
    case ICLI_PORT_TYPE_2_5_GIGABIT_ETHERNET:
    case ICLI_PORT_TYPE_FIVE_GIGABIT_ETHERNET:
    case ICLI_PORT_TYPE_TEN_GIGABIT_ETHERNET:
        break;

    default:
        return FALSE;
    }

    /* allocate memory */
    memset( cmd_help, 0, sizeof(cmd_help) );

    /* make help string */
    s = cmd_help;
    if ( b_port_list ) {
        (void)vtss_icli_str_cpy(s, "Port list in ");
    } else {
        (void)vtss_icli_str_cpy(s, "Port ID in ");
    }
    s = cmd_help + vtss_icli_str_len( cmd_help );

    /* get port list string */
    if ( handle->runtime_data.last_port_type == ICLI_PORT_TYPE_ALL ) {
        (void)vtss_icli_str_cpy( cmd_help, "Port list for all port types" );
    } else {
        (void)_port_list_str_get( handle->runtime_data.last_port_type, s );
    }

    /* display help */
    _help_display( handle, cmd_word, cmd_help );
    return TRUE;
}

static BOOL _cword_help_display(
    IN  icli_session_handle_t   *handle,
    IN  icli_parameter_t        *para,
    IN  icli_runtime_t          *runtime
)
{
    char    *s;
    i32     i;

    if ( runtime->cword[0] == NULL ) {
        return FALSE;
    }

    memset(g_cword_help, 0, sizeof(g_cword_help));

    s = g_cword_help;

    icli_sprintf(s, "Valid words are ");
    s += vtss_icli_str_len( s );

    for ( i = 0; i < ICLI_CWORD_MAX_CNT; ++i ) {
        if ( runtime->cword[i] == NULL ) {
            break;
        }

        if ( para->value.u.u_cword[0] ) {
            if ( vtss_icli_str_sub(para->value.u.u_cword, runtime->cword[i], 0, NULL) < 0 ) {
                continue;
            }
        }

        icli_sprintf(s, "'%s' ", runtime->cword[i]);
        s += vtss_icli_str_len( s );
    }

    /* display help */
    _help_display(handle, "CWORD", g_cword_help);

    return TRUE;
}

static BOOL _switch_id_help_display(
    IN  icli_session_handle_t   *handle,
    IN  char                    *cmd_word,
    IN  BOOL                    b_list
)
{
    char                        cmd_help[__ICLI_HELP_MAX_LEN + 1];
    char                        *s;
    icli_switch_port_range_t    spr;
    u16                         begin_switch_id;
    u16                         end_switch_id;

    /* allocate memory */
    memset( cmd_help, 0, sizeof(cmd_help) );

    /* make help string */
    s = cmd_help;
    if ( b_list ) {
        (void)vtss_icli_str_cpy(s, "Switch ID list in ");
    } else {
        (void)vtss_icli_str_cpy(s, "Switch ID in ");
    }
    s = cmd_help + vtss_icli_str_len( cmd_help );

    /* get current switch ID */
    memset(&spr, 0, sizeof(spr));
    begin_switch_id = 0;
    end_switch_id   = 0;
    while ( vtss_icli_switch_get_next(&spr) ) {
        if ( end_switch_id ) {
            if ( spr.switch_id == (end_switch_id + 1) ) {
                ++end_switch_id;
            } else {
                if ( begin_switch_id == end_switch_id ) {
                    icli_sprintf(s, ",%u", spr.switch_id);
                } else {
                    icli_sprintf(s, "-%u,%u", end_switch_id, spr.switch_id);
                }
                begin_switch_id = spr.switch_id;
                end_switch_id   = begin_switch_id;
            }
        } else {
            begin_switch_id = spr.switch_id;
            end_switch_id   = begin_switch_id;
            icli_sprintf(s, "%u", begin_switch_id);
        }

        /* update s */
        s = cmd_help + vtss_icli_str_len(cmd_help);
    }

    /* add last_end_port */
    if ( end_switch_id != begin_switch_id ) {
        icli_sprintf(s, "-%u", end_switch_id);
    }

    /* display help */
    _help_display(handle, cmd_word, cmd_help);

    return TRUE;
}

#define __PORT_TYPE_HELP(t) \
    if ( vtss_icli_port_type_present(t) ) { \
        if ( (p->value.type == ICLI_VARIABLE_MAX) || \
             (p->value.type == ICLI_VARIABLE_PORT_TYPE && p->value.u.u_port_type == t)) { \
            _help_display(handle, vtss_icli_variable_port_type_get_name(t), vtss_icli_variable_port_type_get_help(t));\
        } \
    }

static void _question_display(
    IN  icli_session_handle_t   *handle
)
{
    icli_parameter_t    *p;
    icli_port_type_t    port_type;
    char                cmd_word[ICLI_BYWORD_MAX_LEN + 4];
    char                cmd_help[ICLI_HELP_MAX_LEN + 4];
    icli_runtime_cb_t   *runtime_cb;
    u32                 b;
    BOOL                b_runtime;
#if 1 /* CP, 08/15/2013 11:22, Bugzilla#12458 - port type id should not get wildcard */
    icli_port_type_t    start_type;
#endif

    for ( p = handle->runtime_data.match_sort_list; p != NULL; p = p->next) {
        /* get command word and help */
        (void)vtss_icli_str_ncpy(cmd_word, _cmd_word_get(handle, p->match_node), ICLI_BYWORD_MAX_LEN);
        (void)vtss_icli_str_ncpy(cmd_help, _help_get(handle, p->match_node, &b_runtime), ICLI_HELP_MAX_LEN);

        /* get runtime port range */
        b = FALSE;

        runtime_cb = p->match_node->node_property.cmd_property->runtime_cb[p->word_id];
        if ( runtime_cb ) {
            b = vtss_icli_exec_runtime( handle, runtime_cb, ICLI_ASK_PORT_RANGE, &(handle->runtime_data.runtime) );
            if ( b ) {
                /* backup system port range */
                vtss_icli_port_range_backup();

                /* set system port range for temp use */
                if ( vtss_icli_port_range_set( &(handle->runtime_data.runtime.port_range) ) == FALSE ) {
                    T_E("Fail to set runtime port range\n");
                }
            }
        }

        switch ( p->match_node->type ) {
        case ICLI_VARIABLE_PORT_TYPE:
#if 1 /* CP, 08/15/2013 11:22, Bugzilla#12458 - port type id should not get wildcard */
            if ( p->match_node->child && p->match_node->child->type == ICLI_VARIABLE_PORT_TYPE_ID ) {
                start_type = ICLI_PORT_TYPE_FAST_ETHERNET;
            } else {
#if 1 /* CP, 09/30/2013 14:35, Bugzilla#12896 - '*' is not allowed if RUNTIME port range */
                if ( b ) {
                    start_type = ICLI_PORT_TYPE_FAST_ETHERNET;
                } else {
                    start_type = ICLI_PORT_TYPE_ALL;
                }
#else
                start_type = ICLI_PORT_TYPE_ALL;
#endif
            }
            for ( port_type = start_type; port_type < ICLI_PORT_TYPE_MAX; ++port_type ) {
                __PORT_TYPE_HELP( port_type );
            }
#else
            for ( port_type = 1; port_type < ICLI_PORT_TYPE_MAX; ++port_type ) {
                __PORT_TYPE_HELP( port_type );
            }
#endif
            break;

        case ICLI_VARIABLE_PORT_ID:
        case ICLI_VARIABLE_PORT_TYPE_ID:
            if ( b_runtime || (_port_help_display(handle, cmd_word, FALSE) == FALSE) ) {
                _help_display(handle, cmd_word, cmd_help);
            }
            break;

        case ICLI_VARIABLE_PORT_LIST:
        case ICLI_VARIABLE_PORT_TYPE_LIST:
            if ( b_runtime || (_port_help_display(handle, cmd_word, TRUE) == FALSE) ) {
                _help_display(handle, cmd_word, cmd_help);
            }
            break;

        case ICLI_VARIABLE_CWORD:
            if ( runtime_cb == NULL ) {
                _help_display(handle, cmd_word, cmd_help);
                break;
            }

            memset(&(handle->runtime_data.runtime), 0, sizeof(icli_runtime_t));

            if ( vtss_icli_exec_cword_runtime_get(handle, runtime_cb, &(handle->runtime_data.runtime)) ) {
                if ( _cword_help_display( handle, p, &(handle->runtime_data.runtime) ) ) {
                    break;
                }
            }

            _help_display(handle, cmd_word, cmd_help);
            break;

        case ICLI_VARIABLE_SWITCH_ID:
            if ( b_runtime || (_switch_id_help_display(handle, cmd_word, FALSE) == FALSE) ) {
                _help_display(handle, cmd_word, cmd_help);
            }
            break;

        case ICLI_VARIABLE_SWITCH_LIST:
            if ( b_runtime || (_switch_id_help_display(handle, cmd_word, TRUE) == FALSE) ) {
                _help_display(handle, cmd_word, cmd_help);
            }
            break;

        default:
            _help_display(handle, cmd_word, cmd_help);
            break;
        }

        if ( b ) {
            /* restore system port range */
            vtss_icli_port_range_restore();
        }
    }

    if ( handle->runtime_data.b_cr ) {
        _session_printf(handle, "    ");
        _session_printf(handle, __ICLI_CR_STR);
        _session_printf(handle, "\n");
    }
}

static i32 _line_mode(
    IN icli_session_handle_t    *handle
)
{
    i32     rc = ICLI_RC_ERR_PARAMETER;

    switch ( _SESSON_INPUT_STYLE ) {
    case ICLI_INPUT_STYLE_SINGLE_LINE :
        rc = vtss_icli_c_line_mode( handle );
        break;

    case ICLI_INPUT_STYLE_MULTIPLE_LINE :
        rc = vtss_icli_a_line_mode( handle );
        break;

    case ICLI_INPUT_STYLE_SIMPLE :
        rc = vtss_icli_z_line_mode( handle );
        break;

    default :
        T_E("invalid input_style = %d\n", _SESSON_INPUT_STYLE);
        break;
    }
    return rc;
}

/*
    put string buffer to a session, processed line by line

    INPUT
        session_id : the session ID

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        process MORE page here
*/
static i32 _session_str_put(
    IN icli_session_handle_t   *handle
)
{
    char                *c;
    char                x;
    char                *l;
    i32                 rc;
    i32                 b_in_exec_cb;
    BOOL                b_more;
    BOOL                b_print;
    BOOL                b_first;
    icli_parameter_t    *p;
    i32                 line_cnt;

    //output the string
    if ( handle->runtime_data.b_in_exec_cb == FALSE ) {
        handle->runtime_data.put_str = handle->runtime_data.str_buf;
        (void)vtss_icli_sutil_usr_str_put( handle );
        //empty string buffer
        handle->runtime_data.str_buf[0] = 0;
        return ICLI_RC_OK;
    }

    switch ( handle->runtime_data.line_mode ) {
    case ICLI_LINE_MODE_PAGE:
        b_print = FALSE;
        break;

    case ICLI_LINE_MODE_FLOOD:
#if 1 /* Bugzilla#11274 */
        /* if with grep, then pending too. otherwise, flood it */
        if ( handle->runtime_data.grep_var ) {
            b_print = FALSE;
        } else {
            b_print = TRUE;
        }
#else
        b_print = FALSE;
#endif
#if 1 /* CP, 05/07/2013 19:05, Bugzilla#11725 - ICLI_PRINTF() to application output callback */
        if ( handle->open_data.way == ICLI_SESSION_WAY_APP_EXEC ) {
            b_print = TRUE;
        }
#endif
        break;

    case ICLI_LINE_MODE_BYPASS:
        //empty string buffer
        handle->runtime_data.str_buf[0] = 0;
        return ICLI_RC_OK;

    default :
        T_E("invalid line_mode = %d\n", handle->runtime_data.line_mode);
        return ICLI_RC_ERR_PARAMETER;
    }

    // this flag is mainly used in page mode
    // if the first output string is longer then width
    // then it should be displayed and then more
    // but not more only
    b_first  = FALSE;
    line_cnt = 0;

    // get line count
    x       = 0;
    b_more  = FALSE;
    p       = handle->runtime_data.grep_var;
    l       = (char *)(handle->runtime_data.str_buf);

    for ( c = l; ICLI_NOT_(EOS, *c); ++c ) {
        /* new line ? */
        if ( ICLI_NOT_(NEWLINE, *c) ) {
            continue;
        }

        /* GREP */
        b_print = TRUE;
        if ( p ) {
            switch ( p->next->value.type ) {
            case ICLI_VARIABLE_GREP_BEGIN:
                if ( handle->runtime_data.grep_begin == 0 ) {
                    if ( vtss_icli_str_str(p->next->next->value.u.u_line, l) ) {
                        handle->runtime_data.grep_begin = 1;
                    } else {
                        b_print = FALSE;
                    }
                }
                break;

            case ICLI_VARIABLE_GREP_INCLUDE:
                if ( vtss_icli_str_str(p->next->next->value.u.u_line, l) == NULL ) {
                    b_print = FALSE;
                }
                break;

            case ICLI_VARIABLE_GREP_EXCLUDE:
                if ( vtss_icli_str_str(p->next->next->value.u.u_line, l) != NULL ) {
                    b_print = FALSE;
                }
                break;

            default:
                break;
            }
        }

        /* if print this line */
        if ( b_print == FALSE ) {
            (void)vtss_icli_str_cpy(handle->runtime_data.str_buf, c + 1);
            /* -1 because c++ in for() */
            c = l;
            --c;
            continue;
        }

        /* print by line */
        ++c;
        x  = *c;
        *c = ICLI_EOS;

        /* only PAGE mode to count line number */
        if ( handle->runtime_data.line_mode == ICLI_LINE_MODE_PAGE && handle->runtime_data.lines ) {
            if ( handle->runtime_data.line_cnt == ICLI_MORE_ENTER ) {
                b_more = TRUE;
                /* go output */
                break;
            }

            if ( handle->runtime_data.line_cnt == 0 ) {
                b_first = TRUE;
            }

            line_cnt = ( vtss_icli_str_len(l) - 1 ) / handle->runtime_data.width + 1;
            handle->runtime_data.line_cnt += line_cnt;

            /* check if more */
            if ( b_first == FALSE &&
                 handle->runtime_data.line_cnt > (i32)(handle->runtime_data.lines - 2) ) {
                b_more = TRUE;
            }
        }

        /* go output */
        break;
    }

    if ( b_print == FALSE ) {
        return ICLI_RC_OK;
    }

    if ( b_more ) {
        /* set to 0 to avoid grep in _line_mode() */
        b_in_exec_cb = handle->runtime_data.b_in_exec_cb;
        handle->runtime_data.b_in_exec_cb = FALSE;

        /* get usr input to see the display mode */
        rc = _line_mode( handle );

        /* restore to avoid grep in _line_mode() */
        handle->runtime_data.b_in_exec_cb = b_in_exec_cb;

        /* check result of _line_mode() */
        if ( rc != ICLI_RC_OK ) {
            return rc;
        }
    }

    /* line mode changed by user 'q' or '^c' */
    if ( handle->runtime_data.line_mode == ICLI_LINE_MODE_BYPASS ) {
        // empty string buffer
        handle->runtime_data.str_buf[0] = 0;
        return ICLI_RC_OK;
    }

    // output the line
    handle->runtime_data.put_str = l;
    (void)vtss_icli_sutil_usr_str_put( handle );

    if ( b_more && handle->runtime_data.line_cnt != ICLI_MORE_ENTER ) {
        handle->runtime_data.line_cnt += line_cnt;
    }

    /* remove the line */
    *c = x;
    if ( x ) {
        (void)vtss_icli_str_cpy(handle->runtime_data.str_buf, c);
    } else {
        handle->runtime_data.str_buf[0] = 0;
    }

    /* if there is string behind, then print it */
    if ( x ) {
        // recursive
        (void)_session_str_put( handle );
    }

    return ICLI_RC_OK;
}

/*
    output/display one char on session

    INPUT
        app_id : application ID
        c      : the char for output/display

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed
*/
static BOOL _app_char_put(
    IN  u32     app_id,
    IN  char    c
)
{
    if ( app_id ) {}
    putchar( c );
    (void) fflush(stdout);
    return TRUE;
}

/*
    output/display string on session

    INPUT
        app_id  : application ID
        str     : the string for output/display

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed
*/
static BOOL _app_str_put(
    IN  u32     app_id,
    IN  char    *str
)
{
    while (*str) {
        (void)_app_char_put(app_id, *str);
        ++str;
    }
    return TRUE;
}

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
i32 vtss_icli_session_init(void)
{
    u32                 i;

#if 1 /* CP, 2012/09/14 10:38, conf */
    icli_conf_data_t    *conf = vtss_icli_conf_get();
#endif

    memset(g_session_handle, 0, sizeof(g_session_handle));

    for ( i = 0; i < ICLI_SESSION_CNT; ++i ) {

#if 1 /* CP, 2012/09/14 10:38, conf */
        // link to session config data
        g_session_handle[i].config_data = &( conf->session_config[i] );
#endif

        (void)vtss_icli_session_config_data_default( g_session_handle[i].config_data );
        _runtime_data_default( &(g_session_handle[i]) );
        g_session_handle[i].session_id = i;
    }
    g_max_sessions = ICLI_SESSION_DEFAULT_CNT;
    return ICLI_RC_OK;
}

#define __AUTH_INPUT_AND_CHECK(s, t)\
    size = sizeof(s);\
    memset(s, 0, size);\
    rc = vtss_icli_session_usr_str_get(session_id, t, s, &size, NULL);\
    /* timeout */\
    if ( rc != ICLI_RC_OK ) {\
        if (handle->open_data.way == ICLI_SESSION_WAY_TELNET || handle->open_data.way == ICLI_SESSION_WAY_SSH) {\
            /* close connection */\
            handle->runtime_data.alive = FALSE;\
        } else {\
            /* console never die */\
            handle->runtime_data.alive = TRUE;\
            auth_cnt = 0;\
        }\
        continue;\
    }

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
)
{
    icli_session_handle_t   *handle;
    char                    *banner;
    BOOL                    do_auth;
    BOOL                    b;
    char                    username[ICLI_USERNAME_MAX_LEN + 1];
    char                    password[ICLI_PASSWORD_MAX_LEN + 1];
    char                    *d;
    i32                     c;
    i32                     rc;
    i32                     size;
    i32                     auth_cnt;
    i32                     b_in_exec_cb;

    if ( session_id < 0 || session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session ID = %d\n", session_id);
        return ICLI_RC_ERROR;
    }

    /* get session handle */
    handle = &g_session_handle[session_id];

    if ( handle->in_used == FALSE ) {
        T_E("session %d is not in used\n", session_id);
        return ICLI_RC_ERROR;
    }

    /* CP, 2012/09/11 14:08, Use thread information to get session ID */
    handle->runtime_data.thread_id = icli_thread_id_get();
    if ( handle->runtime_data.thread_id == 0 ) {
        T_E("invalid thread ID = %u\n", handle->runtime_data.thread_id);
        return ICLI_RC_ERROR;
    }

    switch ( handle->open_data.way ) {
    case ICLI_SESSION_WAY_CONSOLE:
    case ICLI_SESSION_WAY_TELNET:
    case ICLI_SESSION_WAY_SSH:
        /* invoke APP init */
        if ( _usr_app_init(handle) == FALSE ) {
            T_E("APP initialization fails on session %d\n", session_id);
            return ICLI_RC_ERROR;
        }

        /* do authentication */
        do_auth = TRUE;
        break;

    case ICLI_SESSION_WAY_THREAD_CONSOLE:
    case ICLI_SESSION_WAY_THREAD_TELNET:
    case ICLI_SESSION_WAY_THREAD_SSH:
        /* authentication by application thread */
        do_auth = FALSE;
        break;

    case ICLI_SESSION_WAY_APP_EXEC:
    default:
        T_E("invalid type %d of session ID = %d\n", handle->open_data.way, session_id);
        return ICLI_RC_ERROR;
    }

    /* initialization */
    auth_cnt = 0;
    if ( handle->runtime_data.alive == FALSE ) {
        handle->runtime_data.alive = TRUE;
    }

    while ( handle->runtime_data.alive ) {

        if ( do_auth ) {
            /* display MOTD banner */
            if ( handle->config_data->b_motd_banner ) {
                banner = vtss_icli_banner_motd_get();
                if ( vtss_icli_str_len(banner) ) {
                    ICLI_PUT_NEWLINE;
                    vtss_icli_session_str_put(handle, banner);
                    ICLI_PUT_NEWLINE;
                }
            }

            /* reset line */
            handle->runtime_data.line_cnt = 0;

            /* usr authentication */
            switch ( handle->open_data.way ) {
            case ICLI_SESSION_WAY_CONSOLE:
                if ( auth_cnt == 0 ) {
                    ICLI_PUT_NEWLINE;
                    vtss_icli_session_str_put(handle, "press ENTER to get started");
                    for ( b = TRUE, c = 0;
                          b && ICLI_NOT_(KEY_ENTER, c) && ICLI_NOT_(NEWLINE, c);
                          b = vtss_icli_sutil_usr_char_get(handle, ICLI_TIMEOUT_FOREVER, &c) ) {
                        ;
                    }
                    if ( b == FALSE ) {
                        icli_sleep( 1000 );
                        continue;
                    }
                    ICLI_PUT_NEWLINE;
                }

            /* pass through */

            case ICLI_SESSION_WAY_TELNET:
                /* without auth */
                if ( ! icli_has_user_auth() ) {
                    handle->runtime_data.privilege = 0;
                    break;
                }

                /* display LOGIN banner */
                banner = vtss_icli_banner_login_get();
                if ( vtss_icli_str_len(banner) ) {
                    ICLI_PUT_NEWLINE;
                    vtss_icli_session_str_put(handle, banner);
                    ICLI_PUT_NEWLINE;
                }

                vtss_icli_session_str_put(handle, "Username: ");
                __AUTH_INPUT_AND_CHECK(username, ICLI_USR_INPUT_TYPE_NORMAL);

                vtss_icli_session_str_put(handle, "Password: ");
                __AUTH_INPUT_AND_CHECK(password, ICLI_USR_INPUT_TYPE_PASSWORD);

                rc = icli_user_auth(handle->open_data.way, username, password, (i32 *) & (handle->runtime_data.privilege));
                if ( rc != ICLI_RC_OK ) {
                    vtss_icli_session_str_put(handle, "Wrong username or password!\n\n");
                    if ( ++auth_cnt >= ICLI_AUTH_MAX_CNT ) {
                        /* try too many and close connection */
                        if (handle->open_data.way == ICLI_SESSION_WAY_TELNET || handle->open_data.way == ICLI_SESSION_WAY_SSH) {
                            handle->runtime_data.alive = FALSE;
                        }
                    }
                    continue;
                }

                /* CP, 2012/09/04 16:46, session user name */
                (void)vtss_icli_str_ncpy( handle->runtime_data.user_name, username, ICLI_USERNAME_MAX_LEN );
                break;

            case ICLI_SESSION_WAY_SSH:
                /*
                    SSH does AUTH by itself.
                    After SSH does it,
                    icli_ssh_init will set the privilege back.
                */
                break;

            default:
                T_E("invalid type %d of session ID = %d\n", handle->open_data.way, session_id);
                return ICLI_RC_ERROR;
            }

            /* display EXEC banner */
            if ( handle->config_data->b_exec_banner ) {
                banner = vtss_icli_banner_exec_get();
                if ( vtss_icli_str_len(banner) ) {
                    ICLI_PUT_NEWLINE;
                    vtss_icli_session_str_put(handle, banner);
                    ICLI_PUT_NEWLINE;
                }
            }

            do_auth = FALSE;
            handle->runtime_data.history_cmd_head = NULL;
            handle->runtime_data.tab_cnt          = 0;
            handle->runtime_data.exec_type        = ICLI_EXEC_TYPE_CMD;
        }

        if ( handle->runtime_data.connect_time == 0 ) {
            handle->runtime_data.connect_time = icli_current_time_get();
        }

        /* reset line */
        handle->runtime_data.line_cnt = 0;
        if ( handle->runtime_data.lines ) {
            handle->runtime_data.line_mode = ICLI_LINE_MODE_PAGE;
        } else {
            // if lines is 0, then do not page it, just flood it.
            handle->runtime_data.line_mode = ICLI_LINE_MODE_FLOOD;
        }

        /* rewind history */
        handle->runtime_data.history_cmd_pos = NULL;

        /* display prompt */
        if ( handle->runtime_data.tab_cnt == 0 ) {
            _prompt_display( handle );
        }

        /* restore original user command */
        if ( handle->runtime_data.b_comment && handle->runtime_data.comment_pos ) {
            *(handle->runtime_data.comment_pos) = (char)(handle->runtime_data.comment_ch);
        }

        /* reset cmd */
        switch ( handle->runtime_data.exec_type ) {
        case ICLI_EXEC_TYPE_TAB:
            if ( handle->runtime_data.tab_cnt ) {
                vtss_icli_sutil_tab_reset( handle );
                break;
            }
        //fall through

        case ICLI_EXEC_TYPE_QUESTION:
            _cmd_display( handle );
            break;

        case ICLI_EXEC_TYPE_CMD:
        case ICLI_EXEC_TYPE_PARSING:
        default :
            vtss_icli_sutil_cmd_reset( handle );
            break;

#if 1 /* CP, 08/13/2013 13:00, Bugzilla#12423 - show full command syntax */
        case ICLI_EXEC_TYPE_FULL_CMD:
#endif

        case ICLI_EXEC_TYPE_REDISPLAY:
            _cmd_redisplay( handle );
            break;
        }

        /* CP, 2012/10/16 17:02, ICLI_RC_CHECK */
        handle->runtime_data.rc_context_string = NULL;

        /* get usr command */
        rc = _usr_cmd_get( session_id );

        switch ( rc ) {
        case ICLI_RC_OK:

            /* pre-process before command execution */
            switch ( handle->runtime_data.exec_type ) {
            case ICLI_EXEC_TYPE_CMD:
                if ( vtss_icli_str_cmp(handle->runtime_data.cmd, ICLI_DEBUG_PRIVI_CMD) == 0 ) {
                    handle->runtime_data.privilege = ICLI_PRIVILEGE_DEBUG;
                    continue;
                } else {
                    _history_cmd_add( handle );
                }
                break;

            case ICLI_EXEC_TYPE_PARSING:
            case ICLI_EXEC_TYPE_QUESTION:
            case ICLI_EXEC_TYPE_TAB:
#if 1 /* CP, 08/13/2013 13:00, Bugzilla#12423 - show full command syntax */
            case ICLI_EXEC_TYPE_FULL_CMD:
#endif
                break;

            case ICLI_EXEC_TYPE_REDISPLAY:
                continue;

            default:
                break;
            }

            // skip space
            ICLI_SPACE_SKIP(d, handle->runtime_data.cmd);

            // comment
            if ( ICLI_IS_COMMENT(*d) ) {
                continue;
            }

            /* CP, 2013/03/14 15:33, Bugzilla#11262, comment '!' and '#' processing */
            handle->runtime_data.b_comment = FALSE;
            switch ( handle->runtime_data.exec_type ) {
            case ICLI_EXEC_TYPE_QUESTION:
            case ICLI_EXEC_TYPE_TAB:
#if 1 /* CP, 08/13/2013 13:00, Bugzilla#12423 - show full command syntax */
            case ICLI_EXEC_TYPE_FULL_CMD:
#endif
                for ( d = handle->runtime_data.cmd; ICLI_NOT_(EOS, *d); ++d ) {
                    if ( ICLI_IS_COMMENT(*d) ) {
                        handle->runtime_data.comment_pos = d;
                        handle->runtime_data.comment_ch  = *d;
                        handle->runtime_data.b_comment   = TRUE;
                        *d = ICLI_EOS;
                        break;
                    }
                }
                break;

            default:
                break;
            }

            // exec by engine
            handle->runtime_data.b_exec_by_api    = FALSE;
            handle->runtime_data.app_err_msg      = NULL;
            handle->runtime_data.err_display_mode = ICLI_ERR_DISPLAY_MODE_PRINT;

            /* command execution */
            rc = vtss_icli_exec( handle );

            /* get result */
            handle->runtime_data.rc = rc;

            /* process after command execution */
            switch ( rc ) {
            case ICLI_RC_OK :
                switch ( handle->runtime_data.exec_type ) {
                case ICLI_EXEC_TYPE_TAB:
                    // if comment, do nothing
                    if ( handle->runtime_data.b_comment ) {
                        ICLI_PUT_NEWLINE;
                        /* CP, 2013/03/14 15:33, Bugzilla#11262, comment '!' and '#' processing */
                        vtss_icli_sutil_tab_reset( handle );
                    } else {
                        _tab_display( handle );
                    }
                    break;

                case ICLI_EXEC_TYPE_QUESTION:
                    /* set b_in_exec_cb */
                    b_in_exec_cb = handle->runtime_data.b_in_exec_cb;
                    handle->runtime_data.b_in_exec_cb = TRUE;

                    /* display question result */
                    _question_display( handle );

                    /* clear b_in_exec_cb */
                    handle->runtime_data.b_in_exec_cb = b_in_exec_cb;

                    // clear str_buf
                    memset(handle->runtime_data.str_buf, 0, sizeof(handle->runtime_data.str_buf));

                // fall through

                case ICLI_EXEC_TYPE_CMD:
                    // MUST be sure that all will always put \n anyway
                    // flush all output
                    (void)_session_str_put( handle );

                    // clear string buf
                    handle->runtime_data.str_buf[0] = 0;

                    // clear more buf
                    handle->runtime_data.cmd_copy[0] = 0;

                    //fall through

#if 1 /* CP, 08/13/2013 13:00, Bugzilla#12423 - show full command syntax */
                case ICLI_EXEC_TYPE_FULL_CMD:
#endif
                case ICLI_EXEC_TYPE_PARSING:
                default:
                    vtss_icli_exec_para_list_free( &(handle->runtime_data.cmd_var) );
                    vtss_icli_exec_para_list_free( &(handle->runtime_data.match_sort_list) );
                    handle->runtime_data.grep_var   = NULL;
                    handle->runtime_data.grep_begin = 0;
                    break;
                }
                break;

            default :
                break;
            }

        //fall through

        case ICLI_RC_ERR_EMPTY:

            /* action after executing command */
            switch ( handle->runtime_data.after_cmd_act ) {
            case ICLI_AFTER_CMD_ACT_NONE:
            default:
                break;

            case ICLI_AFTER_CMD_ACT_GOTO_PREVIOUS_MODE:
                (void)vtss_icli_session_mode_exit( handle->session_id );
                break;

            case ICLI_AFTER_CMD_ACT_GOTO_EXEC_MODE:
                _goback_exec_mode( handle );
                break;
            }
            break;

        case ICLI_RC_ERR_EXPIRED:
            /* logout */
            handle->runtime_data.alive = FALSE;
            break;

        case ICLI_RC_ERROR:
        case ICLI_RC_ERR_PARAMETER:
            handle->runtime_data.alive = FALSE;
            break;
        }

        switch ( handle->open_data.way ) {
        case ICLI_SESSION_WAY_CONSOLE:
            if ( handle->runtime_data.alive == FALSE ) {
                if ( vtss_icli_console_alive_get() ) {
                    /* redo auth */
                    do_auth  = TRUE;
                    auth_cnt = 0;

                    /* reset runtime data */
                    _runtime_data_clear( handle );

                    /* alive again */
                    handle->runtime_data.alive = TRUE;

                    /* CP, 2012/09/11 14:08, Use thread information to get session ID */
                    handle->runtime_data.thread_id = icli_thread_id_get();
                    if ( handle->runtime_data.thread_id == 0 ) {
                        T_E("invalid thread ID = %u\n", handle->runtime_data.thread_id);
                        return ICLI_RC_ERROR;
                    }
                }
            }
            break;

        case ICLI_SESSION_WAY_TELNET:
        case ICLI_SESSION_WAY_SSH:
            break;

        case ICLI_SESSION_WAY_THREAD_CONSOLE:
        case ICLI_SESSION_WAY_THREAD_TELNET:
        case ICLI_SESSION_WAY_THREAD_SSH:
            if ( handle->runtime_data.alive ) {
                rc = ICLI_RC_OK;
            } else {
                /*
                    the decision that the session is closed or not is by application,
                    so make it alive for the later close decision by application.
                 */
                handle->runtime_data.alive = TRUE;
                rc = ICLI_RC_ERROR;
            }
            return rc;

        case ICLI_SESSION_WAY_APP_EXEC:
        default:
            T_E("invalid type %d of session ID = %d\n", handle->open_data.way, session_id);
            return ICLI_RC_ERROR;
        }

    } //while ( handle->runtime_data.alive )

    /* invoke APP close */
    _usr_app_close( handle, ICLI_CLOSE_REASON_NORMAL );

    /* free session */
    _session_free( session_id );

    return ICLI_RC_OK;

}

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
)
{
    icli_session_handle_t   *handle;
    i32                     sid;

    /* get free session */
    sid = _session_get();
    if ( sid < 0 ) {
        T_E("session full\n");
        return ICLI_RC_ERR_MEMORY;
    }

    /* get handle */
    handle = &g_session_handle[sid];

    /* get session data */
    handle->open_data = *open_data;

    /* create thread accordingly */
    switch ( handle->open_data.way ) {
    case ICLI_SESSION_WAY_CONSOLE:
    case ICLI_SESSION_WAY_TELNET:
    case ICLI_SESSION_WAY_SSH:
        break;

    case ICLI_SESSION_WAY_APP_EXEC:
        /* null the callback to avoid output */
        /* CP, 2013/04/15 10:51, APP to take output */
        if ( handle->open_data.char_put == NULL && handle->open_data.str_put == NULL ) {
            handle->open_data.char_put = _app_char_put;
            handle->open_data.str_put  = _app_str_put;
        }

        /* CP, 05/07/2013 19:05, Bugzilla#11725 - ICLI_PRINTF() to application output callback */
        handle->runtime_data.line_mode = ICLI_LINE_MODE_FLOOD;
        handle->runtime_data.alive = TRUE;

        /* CP, 11/06/2013 12:06, Use thread information to get session ID */
        handle->runtime_data.thread_id = icli_thread_id_get();
        if ( handle->runtime_data.thread_id == 0 ) {
            T_E("invalid thread ID = %u for session %u\n", handle->runtime_data.thread_id, sid);
            handle->in_used = FALSE;
            return ICLI_RC_ERROR;
        }

    // fall through

    case ICLI_SESSION_WAY_THREAD_CONSOLE:
    case ICLI_SESSION_WAY_THREAD_TELNET:
    case ICLI_SESSION_WAY_THREAD_SSH:
        break;

    default:
        T_E("invalid session type %d\n", handle->open_data.way);
        handle->in_used = FALSE;
        return ICLI_RC_ERR_PARAMETER;
    }

    *session_id = (u32)sid;
    return ICLI_RC_OK;

}// vtss_icli_session_open

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
)
{
    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    /* create thread accordingly */
    switch ( g_session_handle[session_id].open_data.way ) {
    case ICLI_SESSION_WAY_CONSOLE:
    case ICLI_SESSION_WAY_TELNET:
    case ICLI_SESSION_WAY_SSH:
        if ( g_session_handle[session_id].runtime_data.alive ) {
            g_session_handle[session_id].runtime_data.alive = FALSE;
            break;
        }

    /* fall through */

    case ICLI_SESSION_WAY_THREAD_CONSOLE:
    case ICLI_SESSION_WAY_THREAD_TELNET:
    case ICLI_SESSION_WAY_THREAD_SSH:
    case ICLI_SESSION_WAY_APP_EXEC:
    default:
        _session_free( session_id );
        break;
    }
    return ICLI_RC_OK;
}

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
)
{
    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return NULL;
    }

    return &(g_session_handle[session_id]);
}

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
)
{
    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return FALSE;
    }

    if ( g_session_handle[session_id].runtime_data.alive ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

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
u32 vtss_icli_session_max_cnt_get(void)
{
    return g_max_sessions;
}

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
)
{
    u32     i;

    if ( max_sessions < 1 || max_sessions > ICLI_SESSION_CNT ) {
        T_E("invalid max_sessions = %d\n", max_sessions);
        return ICLI_RC_ERR_PARAMETER;
    }

    // close sessions
    if ( max_sessions < g_max_sessions ) {
        for ( i = max_sessions; i < g_max_sessions; ++i ) {
            (void)vtss_icli_session_close(i);
        }
    }

    g_max_sessions = max_sessions;
    return ICLI_RC_OK;
}

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
)
{
    icli_session_handle_t   *handle;

    /* get session handle */
    handle = &g_session_handle[session_id];
    *mode = handle->runtime_data.mode_para[handle->runtime_data.mode_level].mode;

    return ICLI_RC_OK;
}

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
)
{
    icli_session_handle_t   *handle;
    icli_mode_para_t        *mode_para;

    /* parameter checking */
    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return -1;
    }

    if ( mode >= ICLI_CMD_MODE_MAX ) {
        T_E("invalid mode = %d\n", mode);
        return -1;
    }

    /* get session handle */
    handle = &g_session_handle[session_id];

    /* check level */
    if ( handle->runtime_data.mode_level >= (ICLI_MODE_MAX_LEVEL - 1) ) {
        return -1;
    }

    /* increase 1 level */
    ++( handle->runtime_data.mode_level );

    /* pack mode */
    mode_para = &( handle->runtime_data.mode_para[handle->runtime_data.mode_level] );
    mode_para->mode    = mode;
    mode_para->cmd_var = handle->runtime_data.cmd_var;

    /* to avoid free after command is over */
    handle->runtime_data.cmd_var = NULL;

    return ( handle->runtime_data.mode_level );
}

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
)
{
    icli_session_handle_t   *handle;
    icli_mode_para_t        *mode_para;

    /* parameter checking */
    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    /* get session handle */
    handle = &g_session_handle[session_id];

    if ( handle->runtime_data.mode_level == 0 ) {
        return 0;
    }

    /* pack mode */
    mode_para = &( handle->runtime_data.mode_para[handle->runtime_data.mode_level] );
    vtss_icli_exec_para_list_free( &(mode_para->cmd_var) );

    /* less 1 level */
    --( handle->runtime_data.mode_level );

    return ( handle->runtime_data.mode_level );
}

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
)
{
    icli_session_handle_t   *handle;
    int                     r;
    char                    *str_buf;
    u32                     len;

    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    handle = &(g_session_handle[session_id]);
    if ( handle->runtime_data.alive == FALSE ) {
        return ICLI_RC_ERR_PARAMETER;
    }

    str_buf = g_session_handle[session_id].runtime_data.str_buf;
    len = vtss_icli_str_len( str_buf );
    r = icli_vsnprintf(str_buf + len, ICLI_PUT_MAX_LEN - len, format, arglist);
    if ( r < 0 ) {
        T_E("fail to format the output string\n");
        return ICLI_RC_ERROR;
    }

    (void)_session_str_put( handle );
    return ICLI_RC_OK;
}

#define _MIN_STR_SIZE   1
#define _BACKWARD_CHAR \
    if ( pos ) { \
        ICLI_PUT_BACKSPACE; \
        ICLI_PUT_SPACE; \
        ICLI_PUT_BACKSPACE; \
        --pos; \
        str[pos] = 0; \
    } else {\
        ICLI_PLAY_BELL;\
    }

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
)
{
    icli_session_handle_t   *handle;
    i32                     c;
    i32                     pos = 0;
    i32                     loop;

    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( input_type >= ICLI_USR_INPUT_TYPE_MAX ) {
        T_E("invalid input_type = %d\n", input_type);
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( str == NULL ) {
        T_E("invalid str == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( str_size == NULL ) {
        T_E("invalid str_size == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( *str_size <= _MIN_STR_SIZE ) {
        T_E("invalid invalid *str_size = %d, should be > %d\n", *str_size, _MIN_STR_SIZE);
        return ICLI_RC_ERR_PARAMETER;
    }

    /* get handle */
    handle = &g_session_handle[ session_id ];

    /* check alive or not */
    if ( handle->runtime_data.alive == FALSE ) {
        T_E("session_id = %d is not alive\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    loop = 1;
    while ( loop == 1 ) {

        /* get char */
        switch ( input_type ) {
        case ICLI_USR_INPUT_TYPE_NORMAL:
        default:
            if ( vtss_icli_sutil_usr_char_get_by_session(handle, &c) == FALSE ) {
                handle->runtime_data.alive = FALSE;
                return ICLI_RC_ERR_EXPIRED;
            }
            break;

        case ICLI_USR_INPUT_TYPE_PASSWORD:
            /* follow c, the timer is for password only, not for session.
               it just displayed time out and not logout */
            if ( vtss_icli_sutil_usr_char_get(handle, ICLI_ENABLE_PASSWORD_WAIT, &c) == FALSE ) {
                return ICLI_RC_ERR_EXPIRED;
            }
            break;
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

        /* process input char */
        switch (c) {
        case ICLI_KEY_FUNCTION0:
            /*
                function key is not supported
                consume next char
            */
            (void)vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c);
            continue;

        case ICLI_KEY_FUNCTION1:
            if ( vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c) == FALSE ) {
                /* invalid function key, skip it */
                continue;
            }
            switch ( c ) {
            case ICLI_FKEY1_LEFT:
                _BACKWARD_CHAR;
                break;

            default:
                break;
            }
            continue;

        case ICLI_HYPER_TERMINAL:

            /* get HT_BEGIN */
            if ( vtss_icli_sutil_usr_char_get(handle, _FKEY_WAIT_TIME, &c) == FALSE ) {
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
            case ICLI_VKEY_LEFT:
                _BACKWARD_CHAR;
                break;

            default:
                break;
            }
            continue;

        case ICLI_KEY_BACKSPACE:
            /* case ICLI_KEY_CTRL_H: the same */
            _BACKWARD_CHAR;
            continue;

        case ICLI_KEY_ENTER:
        case ICLI_NEWLINE:
        /*
            windows CMD telnet will come ENTER + NEW_LINE
            so, consume NEW_LINE.

            Bug: if copy-paste, then the first char will always be consumed.
                 so, this is not good.
        */
        //vtss_icli_sutil_usr_char_get(handle, _BUF_WAIT_TIME, &c);

        // fall through

        case ICLI_KEY_CTRL_('C'):
            ICLI_PUT_NEWLINE;
            str[pos] = 0;
            *str_size = pos;
            if ( end_key ) {
                *end_key = c;
            }
            return ICLI_RC_OK;

        default :
            break;
        }

        /* not supported keys, skip */
        if ( ! ICLI_KEY_VISIBLE(c) ) {
            continue;
        }

        /* skip SPACE at the begin, otherwise, put into cmd */
        if ( pos == (*str_size - 1) ) {
            continue;
        }
        str[pos] = (char)c;
        ++pos;
        if ( input_type == ICLI_USR_INPUT_TYPE_PASSWORD ) {
            c = '*';
        }
        (void)vtss_icli_sutil_usr_char_put(handle, (char)c);

    }/* while (1) */

    // for lint
    return ICLI_RC_ERROR;

}/* vtss_icli_session_usr_str_get */

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
)
{
    icli_session_handle_t   *handle;
    i32                     c;
    u32                     t;

    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( wait_time == 0 || wait_time > ICLI_MAX_INT) {
        T_E("invalid wait_time = %u\n", wait_time);
        return ICLI_RC_ERR_PARAMETER;
    }

    /* get handle */
    handle = &( g_session_handle[ session_id ] );

    /* check alive or not */
    if ( handle->runtime_data.alive == FALSE ) {
        T_E("session_id = %d is not alive\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    /* get Ctrl-C */
    while (wait_time > 0) {
        /* current time */
        t = icli_current_time_get();

        /* wait Ctrl-C */
        if ( vtss_icli_sutil_usr_char_get(handle, (i32)wait_time, &c) == FALSE ) {
            return ICLI_RC_ERR_EXPIRED;
        }

        /* check if Ctrl-C */
        if ( c == ICLI_KEY_CTRL_('C') ) {
            return ICLI_RC_OK;
        }

        /* elapse time */
        t = icli_current_time_get() - t;

        /* update wait_time */
        if ( t < wait_time ) {
            wait_time = wait_time - t;
        } else {
            return ICLI_RC_ERR_EXPIRED;
        }
    }
    return ICLI_RC_ERR_EXPIRED;
}

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
)
{
    icli_session_handle_t   *handle;

    if ( data == NULL ) {
        T_E("data == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( data->session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", data->session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    handle = &( g_session_handle[ data->session_id ] );

    data->alive          = handle->runtime_data.alive;
    data->mode           = handle->runtime_data.mode_para[handle->runtime_data.mode_level].mode;
    data->way            = handle->open_data.way;
    data->connect_time   = handle->runtime_data.connect_time;

    if ( handle->runtime_data.alive ) {
        data->elapsed_time = (icli_current_time_get() - handle->runtime_data.connect_time) / 1000;
    } else {
        data->elapsed_time = 0;
    }

    if ( handle->runtime_data.alive ) {
        data->idle_time = (icli_current_time_get() - handle->runtime_data.idle_time) / 1000;
    } else {
        data->idle_time = 0;
    }

    data->privilege      = handle->runtime_data.privilege;
    data->width          = handle->config_data->width;
    data->lines          = handle->config_data->lines;
    data->wait_time      = handle->config_data->wait_time;
    data->line_mode      = handle->runtime_data.line_mode;
    data->input_style    = handle->config_data->input_style;

#if 1 /* CP, 2012/08/29 09:25, history max count is configurable */
    data->history_size   = ICLI_HISTORY_MAX_CNT;
#endif

#if 1 /* CP, 2012/08/31 07:51, enable/disable banner per line */
    data->b_exec_banner  = handle->config_data->b_exec_banner;
    data->b_motd_banner  = handle->config_data->b_motd_banner;
#endif

#if 1 /* CP, 2012/08/31 09:31, location and default privilege */
    (void)vtss_icli_str_ncpy(data->location, handle->config_data->location, ICLI_LOCATION_MAX_LEN);
    data->privileged_level = handle->config_data->privileged_level;
#endif

#if 1 /* CP, 2012/09/04 16:46, session user name */
    (void)vtss_icli_str_ncpy(data->user_name, handle->runtime_data.user_name, ICLI_USERNAME_MAX_LEN);

    data->client_ip   = handle->open_data.client_ip;
    data->client_port = handle->open_data.client_port;
#endif

    return ICLI_RC_OK;
}

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
)
{
    if ( data == NULL ) {
        T_E("data == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( data->session_id >= (ICLI_SESSION_CNT - 1) ) {
        T_E("invalid session_id = %d\n", data->session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    /* set to next session */
    data->session_id += 1;

    /* get session data */
    return vtss_icli_session_data_get( data );
}

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
)
{
    u32     len;

    if ( handle->runtime_data.alive == FALSE ) {
        T_E("session_id = %d is not alive\n", handle->session_id);
        return;
    }

    switch ( handle->runtime_data.err_display_mode ) {
    case ICLI_ERR_DISPLAY_MODE_PRINT :
        (void)vtss_icli_sutil_usr_char_put(handle, c);
        break;

    case ICLI_ERR_DISPLAY_MODE_ERR_BUFFER :
        len = vtss_icli_str_len( handle->runtime_data.err_buf );
        if ( len >= ICLI_ERR_MAX_LEN ) {
            T_E("session_id = %d, error buffer is full\n", handle->session_id);
            return;
        }
        handle->runtime_data.err_buf[len] = c;
        handle->runtime_data.err_buf[len + 1] = ICLI_EOS;
        break;

    case ICLI_ERR_DISPLAY_MODE_DROP :
    default :
        break;
    }
}

/*
    put string directly to the session
    no line checking
    no buffering

    INPUT
        handle : the session handle
        str    : output string

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
)
{
    u32     len;
    u32     str_len;

    if ( handle->runtime_data.alive == FALSE ) {
        T_E("session_id = %d is not alive\n", handle->session_id);
        return;
    }

    switch ( handle->runtime_data.err_display_mode ) {
    case ICLI_ERR_DISPLAY_MODE_PRINT :
        handle->runtime_data.put_str = str;
        (void)vtss_icli_sutil_usr_str_put( handle );
        handle->runtime_data.put_str = NULL;
        break;

    case ICLI_ERR_DISPLAY_MODE_ERR_BUFFER :
        len = vtss_icli_str_len( handle->runtime_data.err_buf );
        if ( len >= ICLI_ERR_MAX_LEN ) {
            T_E("session_id = %d, error buffer is full\n", handle->session_id);
            return;
        }
        str_len = vtss_icli_str_len( str );
        if ( (len + str_len) >= ICLI_ERR_MAX_LEN ) {
            T_E("session_id = %d, str is too long to error buffer\n", handle->session_id);
            T_E("str = %s\n", str);
            return;
        }
        // concate str in err_buf
        if ( vtss_icli_str_concat(handle->runtime_data.err_buf, str) == NULL ) {
            T_E("session_id = %d, fail to concate str in error buffer\n", handle->session_id);
        }
        break;

    case ICLI_ERR_DISPLAY_MODE_DROP :
    default :
        break;
    }
}

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
)
{
    va_list     arglist;
    int         r;
    u32         len;
    u32         str_len;
    char        *str;

    if ( handle->runtime_data.alive == FALSE ) {
        T_E("session_id = %d is not alive\n", handle->session_id);
        return;
    }

    switch ( handle->runtime_data.err_display_mode ) {
    case ICLI_ERR_DISPLAY_MODE_PRINT :
        // format string buffer
        /*lint -e{530} ... 'arglist' is initialized by va_start() */
        va_start( arglist, format );
        r = icli_vsnprintf(handle->runtime_data.str_buf, ICLI_PUT_MAX_LEN, format, arglist);
        va_end( arglist );

        // check result
        if ( r < 0 ) {
            T_E("fail to format the output string\n");
            return;
        }

        // set put_str
        handle->runtime_data.put_str = handle->runtime_data.str_buf;

        // output the string
        (void)vtss_icli_sutil_usr_str_put( handle );

        // reset
        handle->runtime_data.str_buf[0] = 0;
        handle->runtime_data.put_str = NULL;
        break;

    case ICLI_ERR_DISPLAY_MODE_ERR_BUFFER :
        // check length of error buffer
        len = vtss_icli_str_len( handle->runtime_data.err_buf );
        if ( len >= ICLI_ERR_MAX_LEN ) {
            T_E("session_id = %d, error buffer is full\n", handle->session_id);
            return;
        }

        // format string buffer
        va_start( arglist, format );
        r = icli_vsnprintf(handle->runtime_data.str_buf, ICLI_PUT_MAX_LEN, format, arglist);
        va_end( arglist );

        // check result
        if ( r < 0 ) {
            T_E("fail to format the output string\n");
            return;
        }

        str = handle->runtime_data.str_buf;
        str_len = vtss_icli_str_len( str );
        if ( (len + str_len) >= ICLI_ERR_MAX_LEN ) {
            T_E("session_id = %d, str is too long to error buffer\n", handle->session_id);
            T_E("str = %s\n", str);
            return;
        }
        // concate str in err_buf
        if ( vtss_icli_str_concat(handle->runtime_data.err_buf, str) == NULL ) {
            T_E("session_id = %d, fail to concate str in error buffer\n", handle->session_id);
        }

        // reset
        handle->runtime_data.str_buf[0] = 0;
        break;

    case ICLI_ERR_DISPLAY_MODE_DROP :
    default :
        break;
    }
}

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
)
{
    va_list     arglist;
    int         r;
    u32         len;

    if ( handle->runtime_data.alive == FALSE ) {
        T_E("session_id = %d is not alive\n", handle->session_id);
        return FALSE;
    }

    len = vtss_icli_str_len( handle->runtime_data.cmd_copy );

    // format string buffer
    /*lint -e{530} ... 'arglist' is initialized by va_start() */
    va_start( arglist, format );

    r = icli_vsnprintf(handle->runtime_data.cmd_copy + len, ICLI_STR_MAX_LEN - len, format, arglist);

    va_end( arglist );

    // check result
    if ( r < 0 ) {
        T_E("fail to format the output string\n");
        return FALSE;
    }

    // raise flag
    handle->runtime_data.more_print = TRUE;
    return TRUE;
}

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
)
{
    u32     i;
    u32     thread_id = icli_thread_id_get();

    for ( i = 0; i < ICLI_SESSION_CNT; ++i ) {
        if ( g_session_handle[i].runtime_data.alive ) {
            if ( g_session_handle[i].runtime_data.thread_id == thread_id ) {
                return i;
            }
        }
    }
    return ICLI_SESSION_CNT;
}
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
)
{
    if ( config_data == NULL ) {
        T_E("config_data == NULL\n");
        return FALSE;
    }

    /* set to default */
    config_data->input_style      = vtss_icli_input_style_get();
    config_data->width            = ICLI_DEFAULT_WIDTH;
    config_data->lines            = ICLI_DEFAULT_LINES;
    config_data->wait_time        = ICLI_DEFAULT_WAIT_TIME;
    config_data->history_size     = ICLI_HISTORY_CMD_CNT;
    config_data->b_exec_banner    = TRUE;
    config_data->b_motd_banner    = TRUE;
    config_data->location[0]      = 0;
    config_data->privileged_level = ICLI_DEFAULT_PRIVILEGED_LEVEL;

    return TRUE;
}

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
)
{
    icli_session_handle_t   *handle;

    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    handle = &(g_session_handle[session_id]);
    if ( handle->runtime_data.alive == FALSE ) {
        return ICLI_RC_ERR_PARAMETER;
    }

    *suac = (BOOL)( handle->runtime_data.b_silent_upgrade_allow_config );
    return ICLI_RC_OK;
}

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
)
{
    icli_session_handle_t   *handle;

    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session_id = %d\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    handle = &(g_session_handle[session_id]);
    if ( handle->runtime_data.alive == FALSE ) {
        return ICLI_RC_ERR_PARAMETER;
    }

    handle->runtime_data.b_silent_upgrade_allow_config = suac;
    return ICLI_RC_OK;
}

#endif
