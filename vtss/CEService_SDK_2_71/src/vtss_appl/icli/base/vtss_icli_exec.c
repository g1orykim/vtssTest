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
    > CP.Wang, 05/29/2013 11:25
        - create

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

    Constant and Macro

==============================================================================
*/
#define ICLI_EXEC_PUT_SPACE     vtss_icli_session_char_put(handle, ICLI_SPACE)
#define ICLI_EXEC_PUT_NEWLINE   vtss_icli_session_char_put(handle, ICLI_NEWLINE)

#define ICLI_EXEC_OPTIONAL_HEAD_NUM     ICLI_RANDOM_MUST_CNT

/*
==============================================================================

    Type Definition

==============================================================================
*/
static i32  _match_node_find(
    IN icli_session_handle_t    *handle,
    IN icli_parsing_node_t      *tree,
    IN char                     *w,
    IN BOOL                     b_enable_chk,
    IN BOOL                     b_cnt_reset
);

/*
==============================================================================

    Static Variable

==============================================================================
*/
static icli_parameter_t         g_parameter[ ICLI_PARAMETER_MEM_CNT ];
static icli_parameter_t         *g_para_free;
static i32                      g_para_cnt;

#if ICLI_RANDOM_OPTIONAL
/* use node->word_id as index, TRUE - visited, FALSE - not visited */
static BOOL                     g_random_optional_node[ ICLI_RANDOM_OPTIONAL_CNT ];
#endif

#if 1 /* CP, 2012/10/11 16:09, performance enhancement: do not check same optional nodes */
static u32                      g_match_optional_node[ICLI_CMD_WORD_CNT];
#endif

static icli_signed_range_t      g_int_range_list;
static icli_unsigned_range_t    g_uint_range_list;

#if 1 /* CP, 08/13/2013 13:00, Bugzilla#12423 - show full command syntax */
static char                     *g_full_cmd_list[ICLI_CMD_CNT];
#endif

#if 1 /* CP, 10/26/2013 18:29, do not match visited nodes */
/* use node->word_id as index, TRUE - visited, FALSE - not visited */
static BOOL                     g_matched_node[ ICLI_RANDOM_OPTIONAL_CNT ];
#endif

/*
==============================================================================

    Static Function

==============================================================================
*/
static icli_match_type_t _match_type_get(
    IN icli_variable_type_t     type,
    IN icli_rc_t                rc
)
{
    switch ( rc ) {
    case ICLI_RC_OK:
        if ( vtss_icli_variable_data_string_type_get(type) ) {
            return ICLI_MATCH_TYPE_PARTIAL;
        } else {
            return ICLI_MATCH_TYPE_EXACTLY;
        }

    case ICLI_RC_ERR_INCOMPLETE:
#if 1 /* CP, 2013/03/14 18:02, processing <string> */
        switch ( type ) {
        case ICLI_VARIABLE_STRING:
        case ICLI_VARIABLE_RANGE_STRING:
            return ICLI_MATCH_TYPE_PARTIAL;
        default:
            break;
        }
#endif
        return ICLI_MATCH_TYPE_INCOMPLETE;

    default:
        return ICLI_MATCH_TYPE_ERR;
    }
}

#if ICLI_RANDOM_MUST_NUMBER
#if 0 /* CP, 2012/10/24 11:01, avoid compile warning */
/*
    check if node is in the family of the ancestor

    return -
        TRUE  : yes, they are
        FALSE : no, they are not
*/
static BOOL _is_family(
    IN icli_parsing_node_t  *ancestor,
    IN icli_parsing_node_t  *node
)
{
    icli_parsing_node_t  *child;

    if ( ancestor->child == NULL ) {
        return FALSE;
    }

    for ( child = ancestor->child; child; ___SIBLING(child) ) {
        if ( node == child ) {
            return TRUE;
        }
        if ( _is_family(child, node) ) {
            return TRUE;
        }
    }
    return FALSE;
}
#endif

/*
    check if the node in the current random optional
    the node MUST be the leading node in the main path
    and must not be a sibling

    INPUT
        handle : current session handler
        node   : the node to be checked

    OUTPUT
        n/a

    RETURN
        TRUE  - yes, it is in
        FALSE - no

    COMMENT
        n/a
*/
static BOOL  _in_random_must(
    IN  icli_parsing_node_t     *must_head,
    IN  icli_parsing_node_t     *node
)
{
    u32                     must_level;
    icli_parsing_node_t     *child;

    if ( must_head == NULL ) {
        return FALSE;
    }

    if ( node == NULL ) {
        return FALSE;
    }

    must_level = must_head->random_must_level;

    if ( vtss_icli_parsing_random_head(must_head, node, NULL) ) {
        return TRUE;
    }
    if ( ___BIT_MASK_GET(node->optional_end, must_level) ) {
        return FALSE;
    }

    for ( child = node->child; child != NULL; ___CHILD(child) ) {
        if ( vtss_icli_parsing_random_head(must_head, child, NULL) ) {
            return TRUE;
        }
        if ( ___BIT_MASK_GET(child->optional_begin, must_level) ) {
            return FALSE;
        }
        if ( ___BIT_MASK_GET(child->optional_end, must_level) ) {
            return FALSE;
        }
    }
    return FALSE;
}

/*
    check if child of the optional end node is allowed to go forward or not

    INPUT
        handle : session handler
        node   : optional end node

    OUTPUT
        n/a

    RETURN
        TRUE  : child is allowed to go forward
        FALSE : not allowed

    COMMENT
        n/a
*/
static BOOL _must_child_allowed(
    IN  icli_session_handle_t   *handle,
    IN icli_parsing_node_t      *node
)
{
    u32                     i;
    u32                     d;
    u32                     n;
    icli_parsing_node_t     *random_must_head;
    icli_parameter_t        *p;

#if 1 /* CP, 10/27/2013 22:01 */
    icli_parsing_node_t     *optional_head[ICLI_EXEC_OPTIONAL_HEAD_NUM];
    BOOL                    b_counted;
#endif

#if 1 /* Bugzilla#11142,11261 - if the leading word is not present, then allow child */
    u32                     optional_level;
    BOOL                    b_found;
    icli_parsing_node_t     *parent;

    if ( node->optional_end == 0 ) {
        return TRUE;
    }
#endif

    for ( d = 0; d < ICLI_RANDOM_OPTIONAL_DEPTH; ++d ) {
        if ( node->random_optional[d] && node->random_optional[d]->random_must_begin) {
            random_must_head = node->random_optional[d];

#if 1 /* Bugzilla#11142,11261 - if the leading word is not present, then allow child */
            /*
                COMMAND = a [ b {[c] [d]}*1 ] [x]
                COMMAND = a [ b [p] {[c] [d]}*1 ] [x]
                COMMAND = a [ b [p q] {[c] [d]}*1 ] [x]
                if b is not present then x is allowed.
                for example, Switch# a ?
            */
            parent = random_must_head->parent;
            // for lint
            if ( parent == NULL ) {
                T_E("parent == NULL\n");
                return FALSE;
            }
            optional_level = node->random_optional_level[d];
            while ( parent->optional_end ) {
                for ( ; parent; ___PARENT(parent) ) {
                    if ( ___BIT_MASK_GET(parent->optional_begin, optional_level) ) {
                        ___PARENT( parent );
                        // for lint
                        if ( parent == NULL ) {
                            T_E("parent == NULL\n");
                            return FALSE;
                        }
                        break;
                    }
                }
                // for lint
                if ( parent == NULL ) {
                    T_E("parent == NULL\n");
                    return FALSE;
                }
            }
            /* find 'b', then check if 'b' in cmd_var or not */
            b_found = FALSE;
            for (p = handle->runtime_data.cmd_var; p; ___NEXT(p)) {
                if ( p->match_node == parent ) {
                    b_found = TRUE;
                    break;
                }
            }
            if ( b_found == FALSE ) {
                /* 'b' is not in cmd_var, then allow child */
                continue;
            }
#endif

#if 1 /* CP, 10/27/2013 22:01 */
            memset(optional_head, 0, sizeof(optional_head));
#endif

            n = 0;
            for (p = handle->runtime_data.cmd_var; p; ___NEXT(p)) {
                if ( p->match_node->random_must_head == random_must_head ) {
                    ++n;
#if 1 /* CP, 10/27/2013 22:01 */
                    for ( i = 0; i < ICLI_EXEC_OPTIONAL_HEAD_NUM; i++ ) {
                        if ( optional_head[i] == NULL ) {
                            break;
                        }
                    }

                    if ( i >= ICLI_EXEC_OPTIONAL_HEAD_NUM ) {
                        T_W("Too many Optional Head\n");
                        return FALSE;
                    }
                    optional_head[i] = parent;

                } else if ( p->match_node->random_must_middle || p->match_node->random_must_end ) {
                    // check its random must begin as the begin may be not presented
                    for ( parent = p->match_node->parent; parent && parent->random_must_head == NULL; ___PARENT(parent) ) {
                        ;
                    }

                    if ( parent == NULL ) {
                        T_W("parent == NULL\n");
                        return FALSE;
                    }

                    b_counted = FALSE;
                    for ( i = 0; i < ICLI_EXEC_OPTIONAL_HEAD_NUM; i++ ) {
                        if ( optional_head[i] ) {
                            if ( optional_head[i] == parent ) {
                                b_counted = TRUE;
                                break;
                            }
                        } else {
                            break;
                        }
                    }

                    if ( b_counted ) {
                        continue;
                    }

                    if ( i >= ICLI_EXEC_OPTIONAL_HEAD_NUM ) {
                        T_W("Too many Optional Head\n");
                        return FALSE;
                    }

                    if ( parent->random_must_head == random_must_head ) {
                        ++n;
                        optional_head[i] = parent;
                    }
#endif
                }
            }
            if ( n < random_must_head->random_must_number ) {
                if ( _in_random_must(random_must_head, node->child) == FALSE ) {
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

/*
    check if random optional of the optional end node is allowed to go forward or not

    INPUT
        handle : session handler
        node   : optional end node
        deep   : which deep level to check
                 if 0 then it means to check if deep level 0 is enough and
                 whether to be able to go to deep level 1

    OUTPUT
        n/a

    RETURN
        TRUE  : child is allowed to go forward
        FALSE : not allowed

    COMMENT
        n/a
*/
static BOOL _must_random_allowed(
    IN icli_session_handle_t    *handle,
    IN icli_parsing_node_t      *node,
    IN u32                      deep
)
{
    u32                     n;
    icli_parsing_node_t     *random_must_head;
    u32                     i;
#if 0 /* CP, 10/27/2013 22:01 */
    icli_parameter_t        *p;
#endif

    if ( node->random_optional[deep] && node->random_optional[deep]->random_must_begin) {
        random_must_head = node->random_optional[deep];
        n = 0;
#if 1 /* CP, 10/27/2013 22:01 */
        for ( i = 0; i < ICLI_RANDOM_MUST_CNT; ++i ) {
            if ( handle->runtime_data.random_must_head[i] == random_must_head ) {
                n = handle->runtime_data.random_must_match[i];
                break;
            }
        }
#else
        for (p = handle->runtime_data.cmd_var; p; ___NEXT(p)) {
            if ( p->match_node->random_must_head == random_must_head ) {
                ++n;
            }
        }
#endif
        if ( n < random_must_head->random_must_number ) {
            return FALSE;
        }
    }
    return TRUE;
}

static BOOL _random_must_match_increase(
    IN  icli_session_handle_t   *handle,
    IN icli_parsing_node_t      *random_must_head
)
{
    u32     i;

    if ( random_must_head == NULL ) {
        return TRUE;
    }

    for ( i = 0; i < ICLI_RANDOM_MUST_CNT; ++i ) {
        if ( handle->runtime_data.random_must_head[i] ) {
            if ( handle->runtime_data.random_must_head[i] == random_must_head ) {
                break;
            }
        } else {
            break;
        }
    }
    if ( i >= ICLI_RANDOM_MUST_CNT ) {
        T_E("Too many Random Must\n");
        return FALSE;
    }
    if ( handle->runtime_data.random_must_head[i] ) {
        ++( handle->runtime_data.random_must_match[i] );
    } else {
        handle->runtime_data.random_must_head[i]  = random_must_head;
        handle->runtime_data.random_must_match[i] = 1;
    }
    return TRUE;
}

/*
    check if child of the optional end node is allowed to go forward or not

    INPUT
        handle : session handler
        node   : optional end node

    OUTPUT
        n/a

    RETURN
        TRUE  : child is allowed to go forward
        FALSE : not allowed

    COMMENT
        n/a
*/
static BOOL _session_random_must_get(
    IN  icli_session_handle_t   *handle
)
{
    u32                         i;
    icli_parameter_t            *p;
    icli_parsing_node_t         *parent;
    icli_parsing_node_t         *optional_head[ICLI_EXEC_OPTIONAL_HEAD_NUM];
    BOOL                        b_counted;
    u32                         d;

    memset(handle->runtime_data.random_must_head,  0, sizeof(handle->runtime_data.random_must_head));
    memset(handle->runtime_data.random_must_match, 0, sizeof(handle->runtime_data.random_must_match));
    memset(optional_head, 0, sizeof(optional_head));

    for ( p = handle->runtime_data.cmd_var; p; ___NEXT(p) ) {
        parent = p->match_node;
        if ( p->match_node->random_must_head ) {
            for ( i = 0; i < ICLI_EXEC_OPTIONAL_HEAD_NUM && optional_head[i] ; ++i ) {
                ;
            }
            if ( i >= ICLI_EXEC_OPTIONAL_HEAD_NUM ) {
                T_W("Too many Optional Head\n");
                return FALSE;
            }

            optional_head[i] = p->match_node;
            if ( _random_must_match_increase(handle, p->match_node->random_must_head) == FALSE ) {
                return FALSE;
            }
            parent = p->match_node->random_must_head;
        }

        for ( d = 0; d < p->match_node->random_must_middle; ++d ) {
            // check its random must begin as the begin may be not presented
            for ( parent = parent->parent; parent && parent->random_must_head == NULL; ___PARENT(parent) ) {
                ;
            }

            if ( parent == NULL ) {
                T_W("parent == NULL\n");
                return FALSE;
            }

            b_counted = FALSE;
            for ( i = 0; i < ICLI_EXEC_OPTIONAL_HEAD_NUM; i++ ) {
                if ( optional_head[i] ) {
                    if ( optional_head[i] == parent ) {
                        b_counted = TRUE;
                        break;
                    }
                } else {
                    break;
                }
            }

            if ( b_counted ) {
                continue;
            }

            if ( i >= ICLI_EXEC_OPTIONAL_HEAD_NUM ) {
                T_W("Too many Optional Head\n");
                return FALSE;
            }

            optional_head[i] = parent;
            if ( _random_must_match_increase(handle, parent->random_must_head) == FALSE ) {
                return FALSE;
            }
            parent = parent->random_must_head;
        }

        for ( d = 0; d < p->match_node->random_must_end; ++d ) {
            // check its random must begin as the begin may be not presented
            for ( parent = parent->parent; parent && parent->random_must_head == NULL; ___PARENT(parent) ) {
                ;
            }

            if ( parent == NULL ) {
                T_W("parent == NULL\n");
                return FALSE;
            }

            b_counted = FALSE;
            for ( i = 0; i < ICLI_EXEC_OPTIONAL_HEAD_NUM; i++ ) {
                if ( optional_head[i] ) {
                    if ( optional_head[i] == parent ) {
                        b_counted = TRUE;
                        break;
                    }
                } else {
                    break;
                }
            }

            if ( b_counted ) {
                continue;
            }

            if ( i >= ICLI_EXEC_OPTIONAL_HEAD_NUM ) {
                T_W("Too many Optional Head\n");
                return FALSE;
            }

            optional_head[i] = parent;
            if ( _random_must_match_increase(handle, parent->random_must_head) == FALSE ) {
                return FALSE;
            }
        }
    }

    return TRUE;
}
#endif

/*
    get the first word in cmd_str

    INPUT
        cmd : command string
    OUTPUT
        cmd : point to the second word
        w   : the first word
    RETURN
        icli_rc_t
    COMMENT
        n/a
*/
static i32  _cmd_word_get(
    IN  icli_session_handle_t   *handle,
    OUT char                    **w
)
{
    char    *c;

    //skip space
    ICLI_SPACE_SKIP(c, handle->runtime_data.cmd_walk);

    //EOS
    if ( ICLI_IS_(EOS, (*c)) ) {
        *w = c;
        handle->runtime_data.cmd_walk = c;
        return ICLI_RC_ERR_EMPTY;
    }

    //get word
    *w = c;

#if 1 /* CP, 2013/03/14 18:02, processing <string> */
    for ( ; ICLI_NOT_(SPACE, *c) && ICLI_NOT_(EOS, *c); ++c ) {
        ;
    }
#else
    if ( ICLI_IS_(STRING_BEGIN, *c) ) {
        for ( c++; ICLI_NOT_(EOS, *c); ++c ) {
            /*
                If \" then the " is not a STRING_END.
                On the other hand, a space should follow ".
            */
            if ( ICLI_NOT_(BACK_SLASH, *(c - 1)) &&
                 ICLI_IS_(STRING_END, *c)      &&
                 (ICLI_IS_(SPACE, *(c + 1)) || ICLI_IS_(EOS, *(c + 1))) ) {
                ++c;
                break;
            }
        }
    } else {
        for ( ; ICLI_NOT_(SPACE, *c) && ICLI_NOT_(EOS, *c); ++c ) {
            ;
        }
    }
#endif

    if ( ICLI_IS_(SPACE, *c) ) {
        *c = ICLI_EOS;
        ++c;
    }

    //update cmd_walk
    handle->runtime_data.cmd_walk = c;
    return ICLI_RC_OK;
}

static BOOL _node_prop_present(
    IN  icli_session_handle_t   *handle,
    IN  node_property_t         *node_prop
)
{
    icli_runtime_cb_t   *runtime_cb;
    u32                 b;

    runtime_cb = node_prop->cmd_property->runtime_cb[node_prop->word_id];

    if ( runtime_cb ) {
        b = vtss_icli_exec_runtime( handle, runtime_cb, ICLI_ASK_PRESENT, &(handle->runtime_data.runtime) );
        if ( b == FALSE || handle->runtime_data.runtime.present ) {
            return TRUE;
        }
    } else {
        return TRUE;
    }
    return FALSE;
}

/*
    check if the node is present or not

    INPUT
        handle : session handle
        node   : node

    OUTPUT
        n/a

    RETURN
        TRUE  : present
        FALSE : not present

    COMMENT
        n/a
*/
static BOOL _present_chk(
    IN  icli_session_handle_t   *handle,
    IN  icli_parsing_node_t     *node
)
{
    node_property_t     *node_prop;

    for ( node_prop = &(node->node_property); node_prop; ___NEXT(node_prop) ) {
        if ( _node_prop_present(handle, node_prop) ) {
            return TRUE;
        }
    }
    return FALSE;
}

/*
    check if the session has the enough privilege to run the node

    INPUT
        handle : session handle
        node   : node

    OUTPUT
        n/a

    RETURN
        TRUE  : enough privilege
        FALSE : not enough

    COMMENT
        n/a
*/
static BOOL _privilege_chk(
    IN  icli_session_handle_t   *handle,
    IN  icli_parsing_node_t     *node
)
{
    node_property_t     *node_prop;

    for ( node_prop = &(node->node_property); node_prop; ___NEXT(node_prop) ) {
        if ( handle->runtime_data.privilege >= node_prop->cmd_property->privilege ) {
            return TRUE;
        }
    }
    return FALSE;
}

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */

/*
    check if the first parent of the node is present or not

    INPUT
        handle : session handle
        node   : node

    OUTPUT
        n/a

    RETURN
        TRUE  : present
        FALSE : not present

    COMMENT
        n/a
*/
static BOOL _command_present_chk(
    IN  icli_session_handle_t   *handle,
    IN  icli_parsing_node_t     *node
)
{
    node_property_t         *node_prop;
    icli_runtime_cb_t       *runtime_cb;
    u32                     b;
    icli_parsing_node_t     *first_parent;
    node_property_t         *first_parent_prop;

    /* find first parent */
    for ( first_parent = node; first_parent->parent; ___PARENT(first_parent) ) {
        ;
    }

    for ( node_prop = &(node->node_property); node_prop; ___NEXT(node_prop) ) {
        for ( first_parent_prop = &(first_parent->node_property); first_parent_prop; ___NEXT(first_parent_prop) ) {
            if ( first_parent_prop->cmd_property->cmd_id != node_prop->cmd_property->cmd_id ) {
                continue;
            }
            runtime_cb = first_parent_prop->cmd_property->runtime_cb[first_parent_prop->word_id];
            if ( runtime_cb ) {
                b = vtss_icli_exec_runtime( handle, runtime_cb, ICLI_ASK_PRESENT, &(handle->runtime_data.runtime) );
                if ( b == FALSE || handle->runtime_data.runtime.present ) {
                    return TRUE;
                }
            } else {
                return TRUE;
            }
        }
    }
    return FALSE;
}

#endif /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */

/*
    check the node should be enabled or disabled

    INPUT
        node : node

    OUTPUT
        n/a

    RETURN
        TRUE  : enable
        FALSE : disable

    COMMENT
        n/a
*/
static BOOL _enable_chk(
    IN  icli_session_handle_t   *handle,
    IN  icli_parsing_node_t     *node
)
{
    node_property_t     *node_prop;

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
    /*
     *  if the first parent is not present then all nodes of the corresponding
     *  command are disabled and invisible
     */
    if ( _command_present_chk(handle, node) == FALSE ) {
        return FALSE;
    }
#endif

    for ( node_prop = &(node->node_property); node_prop; ___NEXT(node_prop) ) {
        if ( ICLI_CMD_IS_ENABLE(node_prop->cmd_property->property) ) {
            return TRUE;
        }
    }
    return FALSE;
}

/*
    check the node is visible or invisible

    INPUT
        node : node

    OUTPUT
        n/a

    RETURN
        TRUE  : visible
        FALSE : invisible

    COMMENT
        n/a
*/
static BOOL _visible_chk(
    IN  icli_session_handle_t   *handle,
    IN  icli_parsing_node_t     *node
)
{
    node_property_t     *node_prop;

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
    /*
     *  if the first parent is not present then all nodes of the corresponding
     *  command are disabled and invisible
     */
    if ( _command_present_chk(handle, node) == FALSE ) {
        return FALSE;
    }
#endif

    for ( node_prop = &(node->node_property); node_prop; ___NEXT(node_prop) ) {
        if ( ICLI_CMD_IS_VISIBLE(node_prop->cmd_property->property) ) {
            return TRUE;
        }
    }
    return FALSE;
}

static char *_port_list_str_get(
    IN  icli_stack_port_range_t     *spr,
    IN  u32                         i,
    OUT char                        *str
)
{
    icli_switch_port_range_t    *pr;
    char                        *s;

    if ( i >= spr->cnt ) {
        return "";
    }

    pr = &( spr->switch_range[i] );

    s = str;
    if ( pr->switch_id == ICLI_SWITCH_PORT_ALL ) {
        s += icli_sprintf( s, "*/");
    } else {
        s += icli_sprintf( s, "%u/", pr->switch_id );
    }

    if ( pr->begin_port == ICLI_SWITCH_PORT_ALL ) {
        s += icli_sprintf( s, "*");
    } else {
        s += icli_sprintf( s, "%u", pr->begin_port );

        if ( pr->port_cnt > 1 ) {
            icli_sprintf( s, "-%u", pr->begin_port + pr->port_cnt - 1 );
        }
    }

    return str;
}

#if 0
static void _valid_port_display(
    IN  icli_session_handle_t       *handle,
    IN  icli_stack_port_range_t     *spr
)
{
    u32     i;
    char    str[64];

    (void)vtss_icli_session_error_printf( handle, "%% Valid ports: ");

    for ( i = 0; i < spr->cnt; i++ ) {
        if ( i ) {
            (void)vtss_icli_session_error_printf( handle, ", " );
        }
        (void)vtss_icli_session_error_printf( handle, "%s ", vtss_icli_variable_port_type_get_short_name(spr->switch_range[i].port_type) );
        (void)vtss_icli_session_error_printf( handle, "%s ", _port_list_str_get(spr, i, str) );
    }
    (void)vtss_icli_session_error_printf( handle, "\n\n");
}
#endif

static void _unsigned_add_range(
    IN  unsigned int            v,
    OUT icli_unsigned_range_t   *ur
)
{
    u32     i;
    u32     j;

    if ( ur->cnt ) {
        for ( i = 0; i < ur->cnt; ++i ) {
            if ( v < ur->range[i].min ) {
                if ( v == ur->range[i].min - 1 ) {
                    ur->range[i].min = v;
                    // check if previous range is to be continuous now
                    if ( i && (ur->range[i].min == ur->range[i - 1].max + 1) ) {
                        ur->range[i - 1].max = ur->range[i].max;
                        // shift
                        for ( j = i; j < ur->cnt - 1; ++j ) {
                            ur->range[j] = ur->range[j + 1];
                        }
                        --( ur->cnt );
                        ur->range[j].min = 0;
                        ur->range[j].max = 0;
                    }
                } else {
                    /* add into i */
                    if ( ur->cnt >= ICLI_RANGE_LIST_CNT ) {
                        T_E("list is full\n");
                        return;
                    }
                    // shift
                    for ( j = ur->cnt; j > i; --j ) {
                        ur->range[j] = ur->range[j - 1];
                    }
                    // add
                    ++( ur->cnt );
                    ur->range[i].min = v;
                    ur->range[i].max = v;
                }
                return;
            } else if ( v == ur->range[i].min ) {
                /* already in */
                return;
            } else {
                /* v > min, then ... */
                if ( v <= ur->range[i].max ) {
                    /* already in */
                    return;
                } else {
                    /* v > max */
                    if ( v == ur->range[i].max + 1 ) {
                        ur->range[i].max = v;
                        // check if next range is to be continuous now
                        if ( (i < ur->cnt - 1) && (ur->range[i].max == ur->range[i + 1].min - 1) ) {
                            ur->range[i].max = ur->range[i + 1].max;
                            // shift
                            for ( j = i + 1; j < ur->cnt - 1; ++j ) {
                                ur->range[j] = ur->range[j + 1];
                            }
                            --( ur->cnt );
                            ur->range[j].min = 0;
                            ur->range[j].max = 0;
                        }
                        return;
                    }
                }
            }
        }
        // add to the last one
        if ( ur->cnt >= ICLI_RANGE_LIST_CNT ) {
            T_E("list is full\n");
            return;
        }
        // add
        ++( ur->cnt );
        ur->range[i].min = v;
        ur->range[i].max = v;
    } else {
        ur->cnt = 1;
        ur->range[0].min = v;
        ur->range[0].max = v;
    }
}

static void _unsigned_range_sort(
    INOUT icli_unsigned_range_t     *ur
)
{
    u32             i;
    unsigned int    v;

    memset( &g_uint_range_list, 0, sizeof(g_uint_range_list) );
    for ( i = 0; i < ur->cnt; ++i ) {
        for ( v = ur->range[i].min; v <= ur->range[i].max; ++v ) {
            _unsigned_add_range( v, &g_uint_range_list );
        }
    }
    *ur = g_uint_range_list;
}

/*
    check the node is matched or not
    The match is checked by
    - present
    - privilege
    - command property
    - type, variable or keyword
    - enable/visible check

    INPUT
        handle       : session handle
        node         : node
        w            : command word to match
        level_bit    : optional level
        b_enable_chk : TRUE  - enable check
                       FALSE - visible check
        para         : fill the result in this parameter

    OUTPUT
        para : if (*para) is NULL, create a new one and fill the result in

    RETURN
        icli_match_type_t

    COMMENT
        para cannot be NULL
*/
static icli_match_type_t _node_match(
    IN    icli_session_handle_t     *handle,
    IN    icli_parsing_node_t       *node,
    IN    char                      *w,
    IN    BOOL                      b_enable_chk,
    INOUT icli_parameter_t          **para
)
{
    icli_parameter_t            *p;
    icli_parameter_t            *last_node;
    i32                         rc;
    u32                         err_pos;

    icli_runtime_cb_t           *runtime_cb;
    u32                         b;
    icli_match_type_t           match_type;

    icli_variable_type_t        type;
    icli_range_t                *range;
    icli_port_type_t            port_type;

    u16                         switch_id;
    u16                         port_id;

    char                        *c;
    u32                         len;
    i32                         k;
    u32                         n;
    u32                         i;
    u32                         j;
    icli_unsigned_range_t       *pur;
    BOOL                        b_first;
    icli_switch_port_range_t    spr;
    icli_ask_vcap_vr_t          ask_vcap_vr;
    char                        *cword;

    /* check present */
    if ( _present_chk(handle, node) == FALSE ) {
        return ICLI_MATCH_TYPE_ERR;
    }

    /* check privilege */
    if ( _privilege_chk(handle, node) == FALSE ) {
        return ICLI_MATCH_TYPE_ERR;
    }

    /* check command property */
    if ( b_enable_chk ) {
        // check enable or disable
        if ( _enable_chk(handle, node) == FALSE ) {
            return ICLI_MATCH_TYPE_ERR;
        }
    } else {
        // check show or hide
        if ( _visible_chk(handle, node) == FALSE ) {
            return ICLI_MATCH_TYPE_ERR;
        }
    }

    if ( *para ) {
        p = *para;
    } else {
        p = vtss_icli_exec_parameter_get();
        if ( p == NULL ) {
            T_E("memory insufficient\n");
            return ICLI_MATCH_TYPE_ERR;
        }
    }

    /* get memory */
    memset(&(handle->runtime_data.runtime), 0, sizeof(icli_runtime_t));

    switch ( node->type ) {
    case ICLI_VARIABLE_KEYWORD :
        /* compare keyword */
        if ( *w ) {
            /* pre-process w */
            c = NULL;
            len = vtss_icli_str_len( w );
            if ( w[len - 1] == ICLI_SPACE ) {
                // find the last non-space char
                for ( c = &(w[len - 1]); ICLI_IS_(SPACE, *c); --c ) {
                    ;
                }
                // replace with EOS
                *(++c) = ICLI_EOS;
            }
            /* get the result */
            rc = vtss_icli_str_sub(w, node->word, vtss_icli_case_sensitive_get(), NULL);
            /* post-process w */
            if ( c ) {
                // put space back
                *c = ICLI_SPACE;
            }
        } else {
            rc = 1;
        }

        /* get match type */
        switch ( rc ) {
        case 1 : /* partial match */
            match_type = ICLI_MATCH_TYPE_PARTIAL;
            if ( *w ) {
                p->value.type = ICLI_VARIABLE_KEYWORD;
                (void)vtss_icli_str_ncpy(p->value.u.u_word, w, ICLI_VALUE_STR_MAX_LEN);
            } else {
                p->value.type = ICLI_VARIABLE_MAX;
            }
            break;

        case 0 : /* exactly match */
            match_type = ICLI_MATCH_TYPE_EXACTLY;
            p->value.type = ICLI_VARIABLE_KEYWORD;
            (void)vtss_icli_str_ncpy(p->value.u.u_word, w, ICLI_VALUE_STR_MAX_LEN);
            break;

        case -1 : /* not match */
        default :
            match_type = ICLI_MATCH_TYPE_ERR;
            break;
        }
        break;

    case ICLI_VARIABLE_CWORD:
        if ( vtss_icli_str_len(w) == 0 ) {
            match_type = ICLI_MATCH_TYPE_PARTIAL;
            /* actually not match any type yet */
            p->value.type = ICLI_VARIABLE_MAX;
            break;
        } else {
            runtime_cb = node->node_property.cmd_property->runtime_cb[node->word_id];
            k = vtss_icli_exec_cword_match_cnt( handle, w, runtime_cb, &(handle->runtime_data.runtime), &match_type, &cword );
            if ( k >= 1 ) {
                p->value.type = ICLI_VARIABLE_KEYWORD;
                (void)vtss_icli_str_ncpy(p->value.u.u_cword, (k == 1) ? cword : w, ICLI_VALUE_STR_MAX_LEN);
                break;
            } else if ( k == 0 ) {
                break;
            } else if ( k < 0 ) {
                // <cword> is general <word>
            }
        }

    // fall through

    default :
        /* variables, check and get match type */
        if ( vtss_icli_str_len(w) == 0 ) {
            match_type = ICLI_MATCH_TYPE_PARTIAL;
            /* actually not match any type yet */
            p->value.type = ICLI_VARIABLE_MAX;
        } else {
            type       = node->type;
            range      = node->range;
            runtime_cb = node->node_property.cmd_property->runtime_cb[node->word_id];

            if ( type == ICLI_VARIABLE_VCAP_VR ) {
                if ( runtime_cb == NULL ) {
                    T_E("runtime not exist for %s\n", node->word);
                    return ICLI_MATCH_TYPE_ERR;
                }

                b = vtss_icli_exec_runtime( handle, runtime_cb, ICLI_ASK_VCAP_VR, &(handle->runtime_data.runtime) );
                if ( b ) {
                    ask_vcap_vr = handle->runtime_data.runtime.vcap_vr;
                    range = &(handle->runtime_data.runtime.range);
                    if ( ask_vcap_vr.b_odd_range ) {
                        range->type = ICLI_RANGE_TYPE_SIGNED;
                    } else {
                        range->type = ICLI_RANGE_TYPE_UNSIGNED;
                    }
                    range->u.ur.cnt = 1;
                    range->u.ur.range[0].min = ask_vcap_vr.min;
                    range->u.ur.range[0].max = ask_vcap_vr.max;
                } else {
                    T_E("Fail to ask runtime for %s\n", node->word);
                    return ICLI_MATCH_TYPE_ERR;
                }
            } else if ( runtime_cb ) {
                b = vtss_icli_exec_runtime( handle, runtime_cb, ICLI_ASK_RANGE, &(handle->runtime_data.runtime) );
                if ( b ) {
                    range = &(handle->runtime_data.runtime.range);
                    switch ( type ) {
                    case ICLI_VARIABLE_RANGE_LIST:
                        switch ( range->type ) {
                        case ICLI_RANGE_TYPE_NONE:
                        default:
                            T_E("fail to get range type %u\n", range->type);
                            break;

                        case ICLI_RANGE_TYPE_SIGNED:
                            type = ICLI_VARIABLE_RANGE_INT_LIST;
                            break;

                        case ICLI_RANGE_TYPE_UNSIGNED:
                            type = ICLI_VARIABLE_RANGE_UINT_LIST;
                            break;
                        }
                        break;

                    case ICLI_VARIABLE_INT:
                    case ICLI_VARIABLE_UINT:
                        switch ( range->type ) {
                        case ICLI_RANGE_TYPE_NONE:
                        default:
                            T_E("fail to get range type %u\n", range->type);
                            break;

                        case ICLI_RANGE_TYPE_SIGNED:
                            type = ICLI_VARIABLE_RANGE_INT;
                            break;

                        case ICLI_RANGE_TYPE_UNSIGNED:
                            type = ICLI_VARIABLE_RANGE_UINT;
                            break;
                        }
                        break;

#if 1 /* CP, 2012/10/24 14:08, Bugzilla#10092 - runtime variable word length */
                    case ICLI_VARIABLE_WORD:
                        if ( range->type == ICLI_RANGE_TYPE_UNSIGNED ) {
                            type = ICLI_VARIABLE_RANGE_WORD;
                        } else {
                            T_E("Invalid range type %u, must be ICLI_RANGE_TYPE_UNSIGNED\n", range->type);
                        }
                        break;

                    case ICLI_VARIABLE_KWORD:
                        if ( range->type == ICLI_RANGE_TYPE_UNSIGNED ) {
                            type = ICLI_VARIABLE_RANGE_KWORD;
                        } else {
                            T_E("Invalid range type %u, must be ICLI_RANGE_TYPE_UNSIGNED\n", range->type);
                        }
                        break;

                    case ICLI_VARIABLE_DWORD:
                        if ( range->type == ICLI_RANGE_TYPE_UNSIGNED ) {
                            type = ICLI_VARIABLE_RANGE_DWORD;
                        } else {
                            T_E("Invalid range type %u, must be ICLI_RANGE_TYPE_UNSIGNED\n", range->type);
                        }
                        break;

                    case ICLI_VARIABLE_FWORD:
                        if ( range->type == ICLI_RANGE_TYPE_UNSIGNED ) {
                            type = ICLI_VARIABLE_RANGE_FWORD;
                        } else {
                            T_E("Invalid range type %u, must be ICLI_RANGE_TYPE_UNSIGNED\n", range->type);
                        }
                        break;

                    case ICLI_VARIABLE_STRING:
                        if ( range->type == ICLI_RANGE_TYPE_UNSIGNED ) {
                            type = ICLI_VARIABLE_RANGE_STRING;
                        } else {
                            T_E("Invalid range type %u, must be ICLI_RANGE_TYPE_UNSIGNED\n", range->type);
                        }
                        break;

                    case ICLI_VARIABLE_LINE:
                        if ( range->type == ICLI_RANGE_TYPE_UNSIGNED ) {
                            type = ICLI_VARIABLE_RANGE_LINE;
                        } else {
                            T_E("Invalid range type %u, must be ICLI_RANGE_TYPE_UNSIGNED\n", range->type);
                        }
                        break;

                    case ICLI_VARIABLE_HEXVAL:
                        if ( range->type == ICLI_RANGE_TYPE_UNSIGNED ) {
                            type = ICLI_VARIABLE_RANGE_HEXVAL;
                        } else {
                            T_E("Invalid range type %u, must be ICLI_RANGE_TYPE_UNSIGNED\n", range->type);
                        }
                        break;

                    case ICLI_VARIABLE_VWORD:
                        if ( range->type == ICLI_RANGE_TYPE_UNSIGNED ) {
                            type = ICLI_VARIABLE_RANGE_VWORD;
                        } else {
                            T_E("Invalid range type %u, must be ICLI_RANGE_TYPE_UNSIGNED\n", range->type);
                        }
                        break;
#endif

                    default:
                        break;
                    }
                }
            }

            /* parsing and get variable value */
            rc = vtss_icli_variable_get(type, w, range, &(p->value), &err_pos);

#if 1 /* the output of vtss_icli_variable_get() is different from the origianl type, so it need translation */
            switch ( type ) {
            case ICLI_VARIABLE_RANGE_INT_LIST:
                if ( node->type == ICLI_VARIABLE_RANGE_LIST ) {
                    g_int_range_list = p->value.u.u_range_int_list;
                    p->value.u.u_range_list.type = ICLI_RANGE_TYPE_SIGNED;
                    p->value.u.u_range_list.u.sr = g_int_range_list;
                }
                break;

            case ICLI_VARIABLE_RANGE_UINT_LIST:
                if ( node->type == ICLI_VARIABLE_RANGE_LIST ) {
                    g_uint_range_list = p->value.u.u_range_uint_list;
                    p->value.u.u_range_list.type = ICLI_RANGE_TYPE_UNSIGNED;
                    p->value.u.u_range_list.u.ur = g_uint_range_list;
                }
                break;

            case ICLI_VARIABLE_VLAN_LIST:
                _unsigned_range_sort( &(p->value.u.u_vlan_list) );
                break;

            default:
                break;
            }
#endif

            /* get last node */
            port_type = ICLI_PORT_TYPE_NONE;
            if ( handle->runtime_data.last_port_type != ICLI_PORT_TYPE_NONE ) {
                port_type = handle->runtime_data.last_port_type;
                //handle->runtime_data.last_port_type = ICLI_PORT_TYPE_NONE;
            } else if ( handle->runtime_data.cmd_var ) {
                for ( last_node = handle->runtime_data.cmd_var;
                      last_node->next != NULL;
                      last_node = last_node->next ) {
                    ;
                }
                if ( last_node->value.type == ICLI_VARIABLE_PORT_TYPE ) {
                    port_type = last_node->value.u.u_port_type;
                }
            }

            /* runtime check value */
            switch ( rc ) {
            case ICLI_RC_OK:
                switch ( type ) {
                case ICLI_VARIABLE_PORT_ID:
                case ICLI_VARIABLE_PORT_TYPE_ID:
                case ICLI_VARIABLE_PORT_LIST:
                case ICLI_VARIABLE_PORT_TYPE_LIST:
                case ICLI_VARIABLE_SWITCH_ID:
                case ICLI_VARIABLE_SWITCH_LIST:
#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
                case ICLI_VARIABLE_PORT_TYPE:
#endif
                    /* get runtime port range */
                    b = FALSE;
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

                    switch ( type ) {
                    case ICLI_VARIABLE_PORT_ID:
                    case ICLI_VARIABLE_PORT_TYPE_ID:
                        if ( port_type == ICLI_PORT_TYPE_NONE ) {
                            break;
                        }
                        spr.switch_id  = p->value.u.u_port_id.switch_id;
                        spr.begin_port = p->value.u.u_port_id.begin_port;
                        spr.port_type  = port_type;
                        if ( vtss_icli_port_get(&spr) ) {
                            p->value.u.u_port_id.port_type   = port_type;
                            p->value.u.u_port_id.port_cnt    = 1;
                            p->value.u.u_port_id.usid        = spr.usid;
                            p->value.u.u_port_id.begin_uport = spr.begin_uport;
                            p->value.u.u_port_id.isid        = spr.isid;
                            p->value.u.u_port_id.begin_iport = spr.begin_iport;
                        } else {
                            rc = ICLI_RC_ERR_MATCH;
                            handle->runtime_data.cmd_copy[0] = 0;
                            (void)vtss_icli_session_error_printf( handle, "%% No such port: %s %u/%u\n",
                                                                  vtss_icli_variable_port_type_get_name(port_type),
                                                                  p->value.u.u_port_id.switch_id,
                                                                  p->value.u.u_port_id.begin_port );
                        }
                        break;

                    case ICLI_VARIABLE_PORT_LIST:
                    case ICLI_VARIABLE_PORT_TYPE_LIST:
                        switch ( port_type ) {
                        case ICLI_PORT_TYPE_NONE:
                            break;

                        case ICLI_PORT_TYPE_ALL:
                        default:
                            switch_id = 0;
                            if ( vtss_icli_port_type_list_get( port_type, &(p->value.u.u_port_list),
                                                               &switch_id, &port_id ) ) {
                                if ( switch_id == ICLI_SWITCH_PORT_ALL ) {
                                    //_valid_port_display( handle, &(p->value.u.u_port_list) );
                                }
                            } else {
                                rc = ICLI_RC_ERR_MATCH;
                                handle->runtime_data.cmd_copy[0] = 0;
                                if ( switch_id == ICLI_SWITCH_PORT_ALL ) {
                                    char    str[64];

                                    (void)vtss_icli_session_error_printf( handle, "%% No valid port in wildcard, %s %s\n",
                                                                          vtss_icli_variable_port_type_get_name(port_type),
                                                                          _port_list_str_get(&(p->value.u.u_port_list), port_id, str) );
                                } else {
                                    (void)vtss_icli_session_error_printf( handle, "%% No such port: %s %u/%u\n",
                                                                          vtss_icli_variable_port_type_get_name(port_type),
                                                                          switch_id, port_id );
                                }
                            }
                            break;
                        }
                        break;

                    case ICLI_VARIABLE_SWITCH_ID:
                        spr.switch_id = (u16)( p->value.u.u_switch_id );
                        if ( vtss_icli_switch_get(&spr) == FALSE ) {
                            rc = ICLI_RC_ERR_MATCH;
                            handle->runtime_data.cmd_copy[0] = 0;
                            (void)vtss_icli_session_error_printf(handle, "%% No such switch ID: %u\n",
                                                                 spr.switch_id);
                        }
                        break;

                    case ICLI_VARIABLE_SWITCH_LIST:
                        b_first = TRUE;
                        pur = &( p->value.u.u_switch_list );
                        n = pur->cnt;
                        for ( i = 0; i < n; ++i ) {
                            for ( j = pur->range[i].min; j <= pur->range[i].max; ++j ) {
                                spr.switch_id = (u16)j;
                                if ( vtss_icli_switch_get(&spr) == FALSE ) {
                                    if ( b_first ) {
                                        b_first = FALSE;
                                        rc = ICLI_RC_ERR_MATCH;
                                        handle->runtime_data.cmd_copy[0] = 0;
                                        (void)vtss_icli_session_error_printf(handle, "%% No such switch ID: %u", spr.switch_id);
                                    } else {
                                        (void)vtss_icli_session_error_printf(handle, ",%u", spr.switch_id);
                                    }
                                }
                            }
                        }
                        if ( b_first == FALSE ) {
                            (void)vtss_icli_session_error_printf(handle, "\n");
                        }
                        break;

                    default:
                        break;
                    }

                    if ( b ) {
                        /* restore system port range */
                        vtss_icli_port_range_restore();
                    }
                    break;

                default:
                    break;
                }

                if ( rc == ICLI_RC_ERR_MATCH ) {
                    break;
                }

            // fall through

            case ICLI_RC_ERR_INCOMPLETE:
                switch ( type ) {
                case ICLI_VARIABLE_PORT_TYPE:
#if 1 /* CP, 08/15/2013 11:22, Bugzilla#12458 - port type id should not get wildcard */
                    if ( node->child && node->child->type == ICLI_VARIABLE_PORT_TYPE_ID &&
                         p->value.u.u_port_type == ICLI_PORT_TYPE_ALL ) {
                        err_pos = 0;
                        rc = ICLI_RC_ERR_MATCH;
                        break;
                    }
#endif
                    if ( vtss_icli_exec_port_type_present(handle, node, p->value.u.u_port_type) == FALSE ) {
                        err_pos = 0;
                        rc = ICLI_RC_ERR_MATCH;
                    }
                    break;

                default:
                    break;
                }
                break;

            default:
                break;
            }

            /* get match_type */
            match_type = _match_type_get( type, rc );
        }
        break;
    }

    switch ( match_type ) {
    case ICLI_MATCH_TYPE_EXACTLY : /* exactly match */
    case ICLI_MATCH_TYPE_PARTIAL : /* partial match */
        p->word_id    = node->word_id;
        p->match_node = node;
        p->match_type = match_type;

        if ( *para == NULL ) {
            *para = p;
        }
        break;

    default :
        match_type = ICLI_MATCH_TYPE_ERR;

    // fall through

    case ICLI_MATCH_TYPE_ERR : /* not match */
    case ICLI_MATCH_TYPE_INCOMPLETE :
        if ( *para == NULL ) {
            if ( node->type == ICLI_VARIABLE_CWORD ) {
                p->word_id    = node->word_id;
                p->match_node = node;
                p->match_type = ICLI_MATCH_TYPE_PARTIAL;

                *para = p;
            } else {
                vtss_icli_exec_parameter_free( p );
            }
        } else {
            p->match_type = match_type;
        }
        break;
    }
    return match_type;
}

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
/*
    check if the node is matched or not only, but not check other relations
    this is for loop

    INPUT
        handle          : session handle
        node            : the node to match
        w               : command word to match
        b_enable_chk    : TRUE  - enable check
                          FALSE - visible check
        b_cnt_reset     : reset counters or not
        b_duplicate_chk : check if duplicate or not
        b_in_loop       : in loop or not

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        the result is stored in handle,
        match_cnt, match_node, match_para
        Return ICLI_RC_OK does not mean match node is found, but only mean
        the process works fine.
*/
static i32  _match_single_node(
    IN icli_session_handle_t    *handle,
    IN icli_parsing_node_t      *node,
    IN char                     *w,
    IN BOOL                     b_enable_chk,
    IN BOOL                     b_cnt_reset,
    IN BOOL                     b_duplicate_chk,
    IN BOOL                     b_in_loop
)
{
    icli_match_type_t       match_type;
    icli_parameter_t        *para,
                            *p,
                            *prev;

    if ( b_cnt_reset ) {
        handle->runtime_data.exactly_cnt     = 0;
        handle->runtime_data.partial_cnt     = 0;
        handle->runtime_data.total_match_cnt = 0;
    }

    if ( node == NULL ) {
        return ICLI_RC_OK;
    }

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
    /* check if already in */
    if ( b_duplicate_chk ) {
        if ( handle->runtime_data.cmd_var ) {
            for ( p = handle->runtime_data.cmd_var; p; ___NEXT(p) ) {
                if ( p->match_node == node ) {
                    return ICLI_RC_OK;
                }
            }
        }

        if ( handle->runtime_data.match_para ) {
            for ( p = handle->runtime_data.match_para; p; ___NEXT(p) ) {
                if ( p->match_node == node ) {
                    return ICLI_RC_OK;
                }
            }
        }
    }
#endif

    para = NULL;
    match_type = _node_match(handle, node, w, b_enable_chk, &para);
    switch ( match_type ) {
#if 0 /* CP, 2013/03/14 18:02, processing <string> */
    case ICLI_MATCH_TYPE_REDO:
        /*
            if parsing ICLI_VARIABLE_LINE, then cmd word is re-aligned.
            So, redo the match for all possible next nodes.
        */
        return ICLI_RC_ERR_REDO;
#endif

    case ICLI_MATCH_TYPE_EXACTLY : /* exactly match */
    case ICLI_MATCH_TYPE_PARTIAL : /* partial match */
        // chain to match_para
        if ( handle->runtime_data.match_para ) {
            // chain
            p = handle->runtime_data.match_para;
            for ( prev = p, ___NEXT(p); p; prev = p, ___NEXT(p) ) {
                ;
            }
            para->next = NULL;
            prev->next = para;
        } else {
            // first para
            handle->runtime_data.match_para = para;
        }

        // in loop
#if 1 /* CP, 07/02/2013 08:28, Bugzilla#12073, 12135 */
        para->b_in_loop = b_in_loop;
#else
        para->b_in_loop = TRUE;
#endif

        // update cnt
        ++( handle->runtime_data.total_match_cnt );
        if ( match_type == ICLI_MATCH_TYPE_EXACTLY ) {
            ++( handle->runtime_data.exactly_cnt );
            // exactly match first, don't care after
            break;
        } else {
            ++( handle->runtime_data.partial_cnt );
        }
    /* fall through */

    case ICLI_MATCH_TYPE_INCOMPLETE : /* incomplete */
        if ( match_type == ICLI_MATCH_TYPE_INCOMPLETE ) {
            handle->runtime_data.rc = ICLI_RC_ERR_INCOMPLETE;
        }
    /* fall through */

    case ICLI_MATCH_TYPE_ERR : /* not match */
        break;

    default : /* error */
        T_E("invalid match type %d\n", match_type);
        return ICLI_RC_ERROR;
    }
    return ICLI_RC_OK;
}
#endif

#if ICLI_RANDOM_OPTIONAL
static void _random_optional_reset(void)
{
    memset( g_random_optional_node, 0, sizeof(g_random_optional_node) );
}

static i32  _random_optional_match(
    IN icli_session_handle_t    *handle,
    IN icli_parsing_node_t      *node,
    IN char                     *w,
    IN BOOL                     b_enable_chk,
    IN BOOL                     b_cnt_reset,
    IN i32                      optional_level
)
{
    i32                 d;
    i32                 rc;
    icli_parameter_t    *p;
    u32                 word_id;

#if 1 /* CP, 10/27/2013 22:01 */
    u32                 n;
    u32                 i;

    /* if it is random must head, then check itself first */
    if ( node->random_must_begin ) {
        n = 0;
        for ( i = 0; i < ICLI_RANDOM_MUST_CNT; ++i ) {
            if ( handle->runtime_data.random_must_head[i] == node ) {
                n = handle->runtime_data.random_must_match[i];
                break;
            }
        }
        if ( n < node->random_must_number ) {
            return ICLI_RC_OK;
        }
    }
#endif

    for ( d = 0; d < ICLI_RANDOM_OPTIONAL_DEPTH; ++d ) {
#if ICLI_RANDOM_MUST_NUMBER
        /*
            check if the random must is enough at this level
            so that we can go to next deep level
        */
        if ( d ) {
            if ( _must_random_allowed(handle, node, d - 1) == FALSE ) {
                return ICLI_RC_OK;
            }
        }
#endif

        // with random optional
        if ( node->random_optional[d] == NULL ) {
            continue;
        }

        if ( node->type == ICLI_VARIABLE_PORT_TYPE && ___LOOP_HEADER(node) ) {
            if ( handle->runtime_data.last_port_type == ICLI_PORT_TYPE_NONE ) {
                if ( handle->runtime_data.cmd_var ) {
                    for ( p = handle->runtime_data.cmd_var;
                          p->next != NULL;
                          ___NEXT(p) ) {
                        ;
                    }

                    if ( p->value.type == ICLI_VARIABLE_PORT_TYPE &&
                         p->value.u.u_port_type != ICLI_PORT_TYPE_ALL ) {
                        break;
                    }
                }
            } else if ( handle->runtime_data.last_port_type != ICLI_PORT_TYPE_ALL ) {
                break;
            }
        }

        // check optional level
        if ( optional_level >= 0 ) {
            if ( node->random_optional_level[d] != (u32)optional_level ) {
                continue;
            }
        }

        // check if visited
        word_id = node->random_optional[d]->word_id;
        if ( word_id < ICLI_RANDOM_OPTIONAL_CNT ) {
            if ( g_random_optional_node[word_id] ) {
                continue;
            } else {
                g_random_optional_node[word_id] = TRUE;
            }
        } else {
            T_W("word_id %u overflow, must be in %u\n", word_id, ICLI_RANDOM_OPTIONAL_CNT);
        }

        // find match node
        rc = _match_node_find(handle, node->random_optional[d], w, b_enable_chk, b_cnt_reset);
        switch ( rc ) {
        case ICLI_RC_ERR_REDO:
            return ICLI_RC_ERR_REDO;
        default:
            break;
        }
    }
    return ICLI_RC_OK;
}
#endif

/*
    find the match nodes behind optional nodes

    INPUT
        handle       : session handle
        node         : node
        w            : command word to match
        level_bit    : optional level
        b_enable_chk : TRUE  - enable check
                       FALSE - visible check

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        the result is stored in handle,
        match_cnt, match_node, match_para
*/
static i32  _match_node_find_behind_optional(
    IN icli_session_handle_t    *handle,
    IN icli_parsing_node_t      *node,
    IN char                     *w,
    IN i32                      optional_level,
    IN BOOL                     b_enable_chk
)
{
    icli_parsing_node_t     *child_node;
    i32                     rc;

#if 1 /* CP, 2012/10/11 16:09, performance enhancement: do not check same optional nodes */
    /* already visited */
    if ( ___BIT_MASK_GET(g_match_optional_node[node->word_id], optional_level) ) {
        return ICLI_RC_OK;
    } else {
        ___BIT_MASK_SET(g_match_optional_node[node->word_id], optional_level);
    }
#endif

    // end at this node
    if ( ___BIT_MASK_GET(node->optional_end, optional_level) ) {

#if ICLI_RANDOM_MUST_NUMBER
        /*
            check if random must is enough or not
            if not then can not go to child
         */
        if ( _must_child_allowed(handle, node) == FALSE ) {
            return ICLI_RC_OK;
        }
#endif

        // find child of optional end
        rc = _match_node_find( handle, node->child, w, b_enable_chk, FALSE );
        switch ( rc ) {
        case ICLI_RC_ERR_REDO:
            return ICLI_RC_ERR_REDO;
        default:
            break;
        }

#if ICLI_RANDOM_OPTIONAL
        rc = _random_optional_match(handle, node, w, b_enable_chk, FALSE, optional_level);
        switch ( rc ) {
        case ICLI_RC_OK:
            break;
        case ICLI_RC_ERR_REDO:
            return ICLI_RC_ERR_REDO;
        default:
            return ICLI_RC_ERROR;
        }
#endif
        return ICLI_RC_OK;
    }

    // end at child
    for ( child_node = node->child; child_node != NULL; ___SIBLING(child_node) ) {
        rc = _match_node_find_behind_optional( handle, child_node, w, optional_level, b_enable_chk );
        switch ( rc ) {
        case ICLI_RC_ERR_REDO:
            return ICLI_RC_ERR_REDO;
        default:
            break;
        }
    }
    return ICLI_RC_OK;
}

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */

/*
    find the match nodes after the not-present parent

    INPUT
        handle           : session handle
        tree             : the tree to find
        b_parent_cnt     : TRUE  - check parent count
                           FALSE - not check

    OUTPUT
        n/a

    RETURN
        TRUE  - yes
        FALSE - no

    COMMENT
        this checking is not so correct because the sibling may not be the
        same mandatory sibling.
        For example, 2 commands, "a b" and "a c" is different from "a {b | c}"
        but this checking will consider them the same.
        If want different, then we may need the bit mask like optional that
        consider nested also.
*/
static BOOL _all_not_present_check(
    IN    icli_session_handle_t     *handle,
    IN    icli_parsing_node_t       *tree,
    IN    BOOL                      b_parent_cnt,
    IN    BOOL                      b_original_not_present,
    INOUT u32                       *or_head
)
{
    icli_parsing_node_t     *sibling;

    if ( tree == NULL ) {
        return TRUE;
    }

    // check tree
    if ( b_parent_cnt && tree->parent_cnt > 1 ) {
        return TRUE;
    }

    // this OR is already checked by the first node,
    // just return the original result
    if ( or_head && (*or_head) && tree->or_head == (*or_head) ) {
        return b_original_not_present;
    }

    // update or_head
    if ( or_head ) {
        *or_head = tree->or_head;
    }

    // check node itself
    if ( _present_chk(handle, tree) ) {
        return FALSE;
    }

#if 0/* CP, 10/27/2013 22:01, Why why why??? */
    // check tree->child
    if ( _all_not_present_check(handle, tree->child, TRUE, FALSE, NULL) == FALSE ) {
        return FALSE;
    }
#endif

    // check tree->sibling
    if ( tree->or_head && tree->sibling ) {
        for ( sibling = tree->sibling; sibling; ___SIBLING(sibling) ) {
            if ( sibling->or_head != tree->or_head ) {
                continue;
            }
            // check sibling
            if ( _present_chk(handle, sibling) ) {
                return FALSE;
            }
            // check sibling->child
            if ( _all_not_present_check(handle, tree->child, TRUE, FALSE, NULL) == FALSE ) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

/*
    find the match nodes after the not-present parent

    INPUT
        handle           : session handle
        not_present_node : the first not-present parent
        tree             : the tree to find
        w                : command word to match
        b_enable_chk     : TRUE  - enable check
                           FALSE - visible check

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        the result is stored in handle,
        match_cnt, match_node, match_para
        Return ICLI_RC_OK does not mean match node is found, but only mean
        the process works fine.
*/
static void  _not_present_match_node_find(
    IN icli_session_handle_t    *handle,
    IN icli_parsing_node_t      *not_present_node,
    IN icli_parsing_node_t      *node,
    IN char                     *w,
    IN BOOL                     b_enable_chk,
    IN BOOL                     parent_cnt_chk
)
{
    u32                     i;
    icli_parsing_node_t     *child_node;

    if ( node == NULL ) {
        return;
    }

    if ( parent_cnt_chk && node->parent_cnt > 1 ) {
        return;
    }

    // sibling
    if ( node->sibling ) {
        _not_present_match_node_find(handle, not_present_node, node->sibling, w, b_enable_chk, parent_cnt_chk);
    }

    // child
    if ( _present_chk(handle, node) == FALSE ) {
        if ( node->child ) {
            _not_present_match_node_find(handle, not_present_node, node->child, w, b_enable_chk, parent_cnt_chk);
        }
        return;
    }

    // node self
    (void)_match_single_node(handle, node, w, b_enable_chk, FALSE, TRUE, FALSE);

    // node is optional
    if ( node->optional_begin == 0 ) {
        return;
    }

    // if the tree already traverse and it is an optional node
    // then try its following child
    for ( i = 0; i < 32; ++i ) {
#if 1 /* CP, 2012/10/22 14:21, performance enhance on execution */
        if ( ((node->optional_begin) >> i) == 0 ) {
            break;
        }
#endif
        // optional begin
        if ( ___BIT_MASK_GET(node->optional_begin, i) == 0 ) {
            continue;
        }

        // end at itself
        if ( ___BIT_MASK_GET(node->optional_end, i) ) {

#if ICLI_RANDOM_MUST_NUMBER
            /*
                check if random must is enough or not
                if not then can not go to child
             */
            if ( _must_child_allowed(handle, node) == FALSE ) {
                continue;
            }
#endif

            // match
            //(void)_match_node_find( handle, node->child, w, b_enable_chk, FALSE );
            _not_present_match_node_find( handle, not_present_node, node->child, w, b_enable_chk, FALSE );

            //
            //  random optional is not processed here,
            //  because it is processed in cmd_walk() and next_node_find()
            //
            continue;
        }

        // end at child
        for ( child_node = node->child; child_node != NULL; ___SIBLING(child_node) ) {
            (void)_match_node_find_behind_optional( handle, child_node, w, i, b_enable_chk );
        }
    }
}

#endif /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */

static BOOL _loop_optional_disable(
    IN icli_session_handle_t    *handle,
    IN icli_parsing_node_t      *node
)
{
    if ( node->type != ICLI_VARIABLE_PORT_TYPE_LIST ) {
        return FALSE;
    }

    if ( node->loop == NULL ) {
        return FALSE;
    }

    switch ( handle->runtime_data.last_port_type ) {
    case ICLI_PORT_TYPE_FAST_ETHERNET:
    case ICLI_PORT_TYPE_GIGABIT_ETHERNET:
    case ICLI_PORT_TYPE_2_5_GIGABIT_ETHERNET:
    case ICLI_PORT_TYPE_FIVE_GIGABIT_ETHERNET:
    case ICLI_PORT_TYPE_TEN_GIGABIT_ETHERNET:
        return TRUE;

    default:
        break;
    }

    return FALSE;
}

/*
    find the match nodes in sibling or optional child
    so, the match may be multiple
    the results are stored in session handle, match_cnt, match_para

    INPUT
        handle       : session handle
        tree         : the tree to find
        w            : command word to match
        b_enable_chk : TRUE  - enable check
                       FALSE - visible check
        b_cnt_reset  : reset counters or not

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        the result is stored in handle,
        match_cnt, match_node, match_para
        Return ICLI_RC_OK does not mean match node is found, but only mean
        the process works fine.
*/
static i32  _match_node_find(
    IN icli_session_handle_t    *handle,
    IN icli_parsing_node_t      *tree,
    IN char                     *w,
    IN BOOL                     b_enable_chk,
    IN BOOL                     b_cnt_reset
)
{
    i32                     i,
                            rc;
    BOOL                    b;
    icli_match_type_t       match_type;
    icli_parameter_t        *para;
    icli_parameter_t        *p;
    icli_parameter_t        *prev;
    icli_parsing_node_t     *node;
    icli_parsing_node_t     *child_node;

#if ICLI_RANDOM_OPTIONAL
    icli_parsing_node_t     *sibling_node;
#endif

#if 1 /* CP, 10/27/2013 22:01 */
    icli_parsing_node_t     *ss_node;
#endif

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
    i32                     b_in_loop;
#endif

#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
    BOOL                    b_all_not_present;
    u32                     or_head;
#else
    BOOL                    b_sibling;
#endif

#endif

#if 1 /* CP, 10/26/2013 18:29, do not match visited nodes */
    u32                     word_id;
#endif

    if ( b_cnt_reset ) {
        handle->runtime_data.exactly_cnt     = 0;
        handle->runtime_data.partial_cnt     = 0;
        handle->runtime_data.total_match_cnt = 0;

#if 1 /* CP, 10/26/2013 18:29, do not match visited nodes */
        memset( g_matched_node, 0, sizeof(g_matched_node) );
#endif
    }

    if ( tree == NULL ) {
        return ICLI_RC_OK;
    }

#if 1 /* CP, 10/26/2013 18:29, do not match visited nodes */
    // check if visited
    word_id = tree->word_id;
    if ( word_id < ICLI_RANDOM_OPTIONAL_CNT ) {
        if ( g_matched_node[word_id] ) {
            return ICLI_RC_OK;
        } else {
            g_matched_node[word_id] = TRUE;
        }
    } else {
        T_W("word_id %u overflow, must be in %u\n", word_id, ICLI_RANDOM_OPTIONAL_CNT);
    }
#endif

#if ICLI_RANDOM_OPTIONAL
    if ( handle->runtime_data.b_in_loop == FALSE ) {
        node = tree;
        for ( p = handle->runtime_data.cmd_var; p; ___NEXT(p) ) {
            // already traversed,
            // if its sibling already traversed, then it is traversed
            b = FALSE;
            for ( sibling_node = node; sibling_node; ___SIBLING(sibling_node) ) {
                if ( sibling_node == p->match_node  ) {
                    b = TRUE;
                    break;
#if 1 /* CP, 10/27/2013 22:01 */
                } else {
                    if ( sibling_node->optional_begin && _present_chk(handle, sibling_node) == FALSE ) {
                        // because the optional head is not present,
                        // need to find if its child has been visited or not
                        for ( i = 0; i < 32; ++i ) {
                            if ( ((sibling_node->optional_begin) >> i) == 0 ) {
                                break;
                            }

                            // optional begin
                            if ( ___BIT_MASK_GET(sibling_node->optional_begin, i) == 0 ) {
                                continue;
                            }

                            // end at itself
                            if ( ___BIT_MASK_GET(sibling_node->optional_end, i) ) {
                                continue;
                            }

                            for ( child_node = sibling_node->child; child_node; ___CHILD(child_node) ) {
                                for ( ss_node = child_node; ss_node; ___SIBLING(ss_node) ) {
                                    if ( ss_node == p->match_node ) {
                                        b = TRUE;
                                        break;
                                    }
                                }
                                if ( b ) {
                                    break;
                                }

                                // end at itself
                                if ( ___BIT_MASK_GET(child_node->optional_end, i) ) {
                                    continue;
                                }
                            }
                            if ( b ) {
                                break;
                            }
                        } // for ( i )
                    }
#endif
                }
            }

            if ( b == FALSE ) {
                continue;
            }

            // not optional node
            if ( node->optional_begin == 0 ) {
                return ICLI_RC_OK;
            }

            // if the tree already traverse and it is an optional node
            // then try its following child
            for ( i = 0; i < 32; ++i ) {
#if 1 /* CP, 2012/10/22 14:21, performance enhance on execution */
                if ( ((node->optional_begin) >> i) == 0 ) {
                    break;
                }
#endif
                // optional begin
                if ( ___BIT_MASK_GET(node->optional_begin, i) == 0 ) {
                    continue;
                }

                // end at itself
                if ( ___BIT_MASK_GET(node->optional_end, i) ) {

#if ICLI_RANDOM_MUST_NUMBER
                    /*
                        check if random must is enough or not
                        if not then can not go to child
                     */
                    if ( _must_child_allowed(handle, node) == FALSE ) {
                        continue;
                    }
#endif

                    rc = _match_node_find( handle, node->child, w, b_enable_chk, b_cnt_reset );
                    switch ( rc ) {
                    case ICLI_RC_ERR_REDO:
                        return ICLI_RC_ERR_REDO;
                    default:
                        break;
                    }
                    //
                    //  random optional is not processed here,
                    //  because it is processed in cmd_walk() and next_node_find()
                    //
                    continue;
                }

                // end at child
                for ( child_node = node->child; child_node; ___SIBLING(child_node) ) {
                    rc = _match_node_find_behind_optional( handle, child_node, w, i, b_enable_chk );
                    switch ( rc ) {
                    case ICLI_RC_ERR_REDO:
                        return ICLI_RC_ERR_REDO;
                    default:
                        break;
                    }
                }
            } // for ( i )
            return ICLI_RC_OK;
        } // for ( p )
    } // if ( handle->runtime_data.b_in_loop == FALSE )
#endif

    // find match node
#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
    or_head = 0;
    b_all_not_present = FALSE;
#else
    b_sibling = FALSE;
#endif

#endif

    for ( node = tree; node; ___SIBLING(node) ) {

#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
        /*
         *  if the first parent is not present then all nodes of the corresponding
         *  command are disabled and invisible
         */
        if ( _command_present_chk(handle, node) == FALSE ) {
            continue;
        }

        /* check if the node and its all related siblings are not present */
        b_all_not_present = _all_not_present_check( handle, node, FALSE, b_all_not_present, &or_head );

        if ( b_all_not_present ) {
            rc = _match_node_find( handle, node->child, w, b_enable_chk, FALSE );
            switch ( rc ) {
            case ICLI_RC_ERR_REDO:
                return ICLI_RC_ERR_REDO;
            default:
                break;
            }

#if 1 /* CP, 10/27/2013 22:01 */
            // find in random optional
#if ICLI_RANDOM_OPTIONAL

            b_in_loop = handle->runtime_data.b_in_loop;
            handle->runtime_data.b_in_loop = FALSE;
            rc = _random_optional_match(handle, node, w, b_enable_chk, FALSE, -1);
            handle->runtime_data.b_in_loop = b_in_loop;

            switch ( rc ) {
            case ICLI_RC_ERR_REDO:
                return ICLI_RC_ERR_REDO;
            default:
                break;
            }
#endif // ICLI_RANDOM_OPTIONAL
#endif
        } else if ( _present_chk(handle, node) == FALSE ) {
            _not_present_match_node_find( handle, node, node->child, w, b_enable_chk, TRUE );
            continue;
        }
#else
        /* check present */
        if ( _present_chk(handle, node) ) {
            b_sibling = TRUE;
        } else {
            if ( ( b_sibling == FALSE && node->sibling == NULL )  ||
                 ( node->child && node->child->parent_cnt == 1 ) ) {
                rc = _match_node_find( handle, node->child, w, b_enable_chk, FALSE );
                switch ( rc ) {
                case ICLI_RC_ERR_REDO:
                    return ICLI_RC_ERR_REDO;
                default:
                    break;
                }
            }
        }
#endif

#endif

        para = NULL;
        match_type = _node_match(handle, node, w, b_enable_chk, &para);
        switch ( match_type ) {
#if 0 /* CP, 2013/03/14 18:02, processing <string> */
        case ICLI_MATCH_TYPE_REDO:
            /*
                if parsing ICLI_VARIABLE_LINE, then cmd word is re-aligned.
                So, redo the match for all possible next nodes.
            */
            return ICLI_RC_ERR_REDO;
#endif

        case ICLI_MATCH_TYPE_EXACTLY : /* exactly match */
        case ICLI_MATCH_TYPE_PARTIAL : /* partial match */
            // chain to match_para
            if ( handle->runtime_data.match_para ) {
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
                if ( handle->runtime_data.b_in_loop == FALSE ) {
#endif
                    b = FALSE;
                    /* check duplicate */
                    for ( p = handle->runtime_data.match_para; p; ___NEXT(p) ) {
                        if ( p->match_node == node ) {
                            b = TRUE;
                            break;
                        }
                    }
#if ICLI_RANDOM_OPTIONAL
                    /* check if traversal */
                    if ( b == FALSE ) {
                        for ( p = handle->runtime_data.cmd_var; p; ___NEXT(p) ) {
                            // already traversed,
                            // if its sibling already traversed, then it is traversed
                            b = FALSE;
                            for ( sibling_node = node; sibling_node; ___SIBLING(sibling_node) ) {
                                if ( p->match_node == sibling_node ) {
                                    b = TRUE;
                                    break;
                                }
                            }
                            if ( b == TRUE ) {
                                break;
                            }
                        }
                    }
#endif
                    /*
                        already exist, also means ever traversed
                        so, do not need to fall through
                        just go to check next sibling
                    */
                    if ( b == TRUE ) {
                        vtss_icli_exec_parameter_free( para );
                        break;
                    }
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
                }
#endif

                // chain
                p = handle->runtime_data.match_para;
                for ( prev = p, ___NEXT(p); p; prev = p, ___NEXT(p) ) {
                    ;
                }
                para->next = NULL;
                prev->next = para;
            } else {
                // first para
                handle->runtime_data.match_para = para;
            }

            // update cnt
            ++( handle->runtime_data.total_match_cnt );
            if ( match_type == ICLI_MATCH_TYPE_EXACTLY ) {
                ++( handle->runtime_data.exactly_cnt );
                // exactly match first, don't care after
                break;
            } else {
                ++( handle->runtime_data.partial_cnt );
            }
        /* fall through */

        case ICLI_MATCH_TYPE_INCOMPLETE : /* incomplete */
            if ( match_type == ICLI_MATCH_TYPE_INCOMPLETE ) {
                handle->runtime_data.rc = ICLI_RC_ERR_INCOMPLETE;
                if ( node->type == ICLI_VARIABLE_CWORD ) {
                    // chain to match_para
                    if ( handle->runtime_data.match_para ) {
                        // chain
                        p = handle->runtime_data.match_para;
                        for ( prev = p, ___NEXT(p); p; prev = p, ___NEXT(p) ) {
                            ;
                        }
                        para->next = NULL;
                        prev->next = para;
                    } else {
                        // first para
                        handle->runtime_data.match_para = para;
                    }
                    handle->runtime_data.total_match_cnt += 2;
                    handle->runtime_data.partial_cnt += 2;
                }
            }
        /* fall through */

        case ICLI_MATCH_TYPE_ERR : /* not match */

            // not optional node
            if ( node->optional_begin == 0 ) {
                break;
            }

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
            if ( _loop_optional_disable(handle, node) ) {
                break;
            }
#endif

            // optional node, find child
            for ( i = 0; i < 32; ++i ) {
#if 1 /* CP, 2012/10/22 14:21, performance enhance on execution */
                if ( ((node->optional_begin) >> i) == 0 ) {
                    break;
                }
#endif
                // optional begin
                if ( ___BIT_MASK_GET(node->optional_begin, i) == 0 ) {
                    continue;
                }

                // end at itself
                if ( ___BIT_MASK_GET(node->optional_end, i) ) {

#if ICLI_RANDOM_MUST_NUMBER
                    /*
                        check if random must is enough or not
                        if not then can not go to child
                     */
                    if ( _must_child_allowed(handle, node) == FALSE ) {
                        continue;
                    }
#endif

                    rc = _match_node_find( handle, node->child, w, b_enable_chk, FALSE );
                    switch ( rc ) {
                    case ICLI_RC_ERR_REDO:
                        return ICLI_RC_ERR_REDO;
                    default:
                        break;
                    }
#if ICLI_RANDOM_OPTIONAL
                    rc = _random_optional_match(handle, node, w, b_enable_chk, FALSE, i);
                    switch ( rc ) {
                    case ICLI_RC_OK:
                        break;
                    case ICLI_RC_ERR_REDO:
                        return ICLI_RC_ERR_REDO;
                    default:
                        return ICLI_RC_ERROR;
                    }
#endif
                    continue;
                }

                // end at child
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
                b_in_loop = handle->runtime_data.b_in_loop;
                handle->runtime_data.b_in_loop = FALSE;
#endif
                for ( child_node = node->child; child_node != NULL; ___SIBLING(child_node) ) {
                    rc = _match_node_find_behind_optional( handle, child_node, w, i, b_enable_chk );
                    switch ( rc ) {
                    case ICLI_RC_ERR_REDO:
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
                        handle->runtime_data.b_in_loop = b_in_loop;
#endif
                        return ICLI_RC_ERR_REDO;
                    default:
                        break;
                    }
                }
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
                handle->runtime_data.b_in_loop = b_in_loop;
#endif
            }
            break;

        default : /* error */
            T_E("invalid match type %d\n", match_type);
            return ICLI_RC_ERROR;
        }
    }
    return ICLI_RC_OK;
}

static void _error_display(
    IN  icli_session_handle_t   *handle,
    IN  char                    *w,
    IN  char                    *err_descr
)
{
    u32     i, j, pos, w_pos;

#if 1 /* CP, 2012/08/20 11:10, because err_display_mode is for error syntax only, not for application error message */
    BOOL    b_change_mode;
#endif

    // if exec by API then do what application wants
    if ( handle->runtime_data.b_exec_by_api ) {
        if ( handle->runtime_data.app_err_msg ) {
#if 1 /* CP, 2012/08/20 11:10, because err_display_mode is for error syntax only, not for application error message */
            b_change_mode = FALSE;
            if ( handle->runtime_data.err_display_mode == ICLI_ERR_DISPLAY_MODE_DROP ) {
                handle->runtime_data.err_display_mode = ICLI_ERR_DISPLAY_MODE_PRINT;
                b_change_mode = TRUE;
            }
#endif
            if ( handle->runtime_data.line_number == ICLI_INVALID_LINE_NUMBER ) {
                vtss_icli_session_str_put(handle, handle->runtime_data.app_err_msg);
                ICLI_EXEC_PUT_NEWLINE;
#if 0 /* CP, 2013/04/15 10:51, APP to take output */
                if ( handle->runtime_data.err_display_mode == ICLI_ERR_DISPLAY_MODE_DROP ) {
                    ICLI_EXEC_PUT_NEWLINE;
                }
#endif
            } else {
                vtss_icli_session_str_printf(handle, "%% Error in file %s, line %d:\n",
                                             handle->runtime_data.app_err_msg, handle->runtime_data.line_number);
            }
#if 1 /* CP, 2012/08/20 11:10, because err_display_mode is for error syntax only, not for application error message */
            if ( b_change_mode ) {
                handle->runtime_data.err_display_mode = ICLI_ERR_DISPLAY_MODE_DROP;
            }
#endif
        }

        if ( handle->runtime_data.err_display_mode == ICLI_ERR_DISPLAY_MODE_DROP ) {
            return;
        }
    }

    // TAB key, play beep
    switch ( handle->runtime_data.exec_type ) {
    case ICLI_EXEC_TYPE_TAB:
        ICLI_PLAY_BELL;
        break;

    default:
        break;
    }

    // drop error message
    if ( handle->runtime_data.err_display_mode == ICLI_ERR_DISPLAY_MODE_DROP ) {
        return;
    }

    if ( w == NULL ) {
        if ( handle->runtime_data.b_exec_by_api ) {
            if ( handle->runtime_data.app_err_msg ) {
                if ( handle->runtime_data.line_number == ICLI_INVALID_LINE_NUMBER ) {
                    ICLI_EXEC_PUT_NEWLINE;
                }
            }
            // revert the command
            for ( i = 0; i < (u32)(handle->runtime_data.cmd_len); ++i ) {
                if ( ICLI_IS_(EOS, handle->runtime_data.cmd[i]) ) {
                    handle->runtime_data.cmd[i] = ICLI_SPACE;
                }
            }
            vtss_icli_session_str_put(handle, handle->runtime_data.cmd);
            ICLI_EXEC_PUT_NEWLINE;
        }
        vtss_icli_session_str_put(handle, err_descr);
        ICLI_EXEC_PUT_NEWLINE;
        return;
    }

#if 1 /* CP, 2012/11/07 10:48, Bugzilla#10023 */
    /* check more print */
    if ( handle->runtime_data.more_print ) {
        vtss_icli_session_str_put(handle, handle->runtime_data.cmd_copy);
        handle->runtime_data.more_print = FALSE;
        memset(handle->runtime_data.cmd_copy, 0, sizeof(handle->runtime_data.cmd_copy));
        ICLI_EXEC_PUT_NEWLINE;
        return;
    }
#endif

    w_pos = w - handle->runtime_data.cmd_start;

    /* the word position is out of the line or call by api */
    if ( (handle->runtime_data.cmd_len > _CMD_LINE_LEN) ||
         handle->runtime_data.left_scroll               ||
         handle->runtime_data.right_scroll              ||
         handle->runtime_data.b_exec_by_api              ) {
        // revert the command
        for ( i = 0; i < (u32)(handle->runtime_data.cmd_len); ++i ) {
            if ( ICLI_IS_(EOS, handle->runtime_data.cmd[i]) ) {
                handle->runtime_data.cmd[i] = ICLI_SPACE;
            }
        }

        if ( (handle->runtime_data.cmd_len > _CMD_LINE_LEN) ||
             handle->runtime_data.left_scroll               ||
             handle->runtime_data.right_scroll              ||
             (handle->runtime_data.app_err_msg &&
              handle->runtime_data.line_number == ICLI_INVALID_LINE_NUMBER) ) {
            ICLI_EXEC_PUT_NEWLINE;
        }

        for ( i = 0; i < (u32)(handle->runtime_data.cmd_len); ++i ) {
            vtss_icli_session_char_put(handle, handle->runtime_data.cmd[i]);
            if ( (i + 1) % (handle->runtime_data.width - 1) == 0 ) {
                ICLI_EXEC_PUT_NEWLINE;
                if ( w_pos < i ) {
                    pos = w_pos % (handle->runtime_data.width - 1);
                    for ( j = 0; j < pos; ++j ) {
                        ICLI_EXEC_PUT_SPACE;
                    }
                    vtss_icli_session_char_put(handle, '^');
                    ICLI_EXEC_PUT_NEWLINE;
                    // set large to avoid print again
                    w_pos = 0x0000FFff;
                }
            }
        }
        ICLI_EXEC_PUT_NEWLINE;

        if ( w_pos < i ) {
            pos = w_pos % (handle->runtime_data.width - 1);
            for ( j = 0; j < pos; ++j ) {
                ICLI_EXEC_PUT_SPACE;
            }
            vtss_icli_session_char_put(handle, '^');
            ICLI_EXEC_PUT_NEWLINE;
        } else {
            ICLI_EXEC_PUT_NEWLINE;
        }
    } else {
        if ( handle->runtime_data.start_pos ) {
            // 1 is for left scroll
            pos = handle->runtime_data.prompt_len + w_pos - handle->runtime_data.start_pos + 1;
        } else {
            pos = handle->runtime_data.prompt_len + w_pos;
        }

        for ( i = 0; i < pos; ++i ) {
            ICLI_EXEC_PUT_SPACE;
        }
        vtss_icli_session_char_put(handle, '^');
        ICLI_EXEC_PUT_NEWLINE;
    }
    vtss_icli_session_str_printf(handle, "%% %s word detected at '^' marker.\n", err_descr);

#if 0 /* CP, 2012/11/07 10:48, Bugzilla#10023 */
    /* check more print */
    if ( handle->runtime_data.more_print ) {
        vtss_icli_session_str_put(handle, handle->runtime_data.cmd_copy);
        handle->runtime_data.more_print = FALSE;
        memset(handle->runtime_data.cmd_copy, 0, sizeof(handle->runtime_data.cmd_copy));
    }
#endif
    ICLI_EXEC_PUT_NEWLINE;
}

static void _match_para_free(
    IN  icli_session_handle_t   *handle
)
{
    vtss_icli_exec_para_list_free( &(handle->runtime_data.match_para) );
    vtss_icli_exec_para_list_free( &(handle->runtime_data.match_sort_list) );

    handle->runtime_data.exactly_cnt     = 0;
    handle->runtime_data.partial_cnt     = 0;
    handle->runtime_data.total_match_cnt = 0;
}

static void _handle_para_free(
    IN  icli_session_handle_t   *handle
)
{
    vtss_icli_exec_para_list_free( &(handle->runtime_data.cmd_var) );

    _match_para_free( handle );

    handle->runtime_data.grep_var   = NULL;
    handle->runtime_data.grep_begin = 0;
}

static void _cmd_var_put(
    IN  icli_session_handle_t   *handle,
    IN  icli_parameter_t        *para,
    IN  BOOL                    b_sort
)
{
    icli_parameter_t    *p;
    icli_parameter_t    *prev;

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
    if ( handle->runtime_data.b_in_loop ) {
        if ( para->b_in_loop == FALSE ) {
            if ( para->match_node->loop ) {
                para->b_in_loop = TRUE;
            } else {
                handle->runtime_data.b_in_loop = FALSE;
            }
        }
    } else {
        if ( para->b_in_loop ) {
            handle->runtime_data.b_in_loop = TRUE;
        }
    }
#endif

    para->next = NULL;
    if ( handle->runtime_data.cmd_var == NULL ) {
        handle->runtime_data.cmd_var = para;
        return;
    }

    if ( b_sort ) {
        // sort and insert
        p = handle->runtime_data.cmd_var;
        if ( para->match_node->word_id < p->match_node->word_id ) {
            para->next = p;
            handle->runtime_data.cmd_var = para;
        } else {
            for ( prev = p, p = p->next;
                  (p != NULL) && (para->match_node->word_id >= p->match_node->word_id);
                  prev = p, p = p->next ) {
                ;
            }
            // insert
            prev->next = para;
            para->next = p;
        }
    } else {
        // go the last node
        for ( p = handle->runtime_data.cmd_var; p->next; ___NEXT(p) ) {
            ;
        }
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
        if ( para->b_in_loop == TRUE ) {
            p->b_in_loop = TRUE;
        }
#endif
        // append
        p->next = para;
        para->next = NULL;
    }
}

static void _match_sort_put(
    IN  icli_session_handle_t   *handle,
    IN  icli_parameter_t        *para
)
{
    icli_parameter_t    *p,
                        *prev;

    para->next = NULL;
    if ( handle->runtime_data.match_sort_list == NULL ) {
        handle->runtime_data.match_sort_list = para;
        return;
    }

    // sort and insert
    p = handle->runtime_data.match_sort_list;
    if ( vtss_icli_str_cmp(para->match_node->word, p->match_node->word) == -1 ) {
        para->next = p;
        handle->runtime_data.match_sort_list = para;
    } else {
        for ( prev = p, ___NEXT(p);
              (p) && (vtss_icli_str_cmp(para->match_node->word, p->match_node->word) == 1);
              prev = p, ___NEXT(p) ) {
            ;
        }
        // insert
        prev->next = para;
        para->next = p;
    }
}

/*
    walk parsing tree by user command

    INPUT
        handle : session handle

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _cmd_walk(
    IN  icli_session_handle_t   *handle
)
{
    i32                     rc;
    char                    *w;
    icli_parsing_node_t     *tree;
    icli_parameter_t        *p;
    icli_parameter_t        *prev;
    BOOL                    b = TRUE;
    node_property_t         *node_prop;

#if 1 /* CP_LINE */
    char                    *c;
    i32                     forever_loop;
#endif

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
    icli_parameter_t        *para;
    u32                     i;
    icli_stack_port_range_t *para_spr;
#endif

#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */
    icli_parsing_node_t     *node;
    icli_parsing_node_t     *sibling;
    BOOL                    b_sibling;
#endif

#if 1 /* CP, 2013/03/14 15:33, Bugzilla#11262, comment '!' and '#' processing */
    BOOL                    b_comment_redo;
    BOOL                    b_current_b;
#endif

#if 1 /* CP, 2013/03/14 18:02, processing <string> */
    BOOL                    b_string;
    BOOL                    b_remove_string;
    u32                     string_cnt;
    icli_match_type_t       match_type;
    BOOL                    b_remove_line;
#endif

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
    u32                     or_head;
    icli_parsing_node_t     *node_sibling;
#endif

    // empty command
    if ( vtss_icli_str_len(handle->runtime_data.cmd_walk) == 0 ) {
        return ICLI_RC_ERR_EMPTY;
    }

    // get tree
    if ( handle->runtime_data.b_in_parsing ) {
        tree = vtss_icli_parsing_tree_get( handle->runtime_data.current_parsing_mode );
    } else {
        tree = vtss_icli_parsing_tree_get( handle->runtime_data.mode_para[handle->runtime_data.mode_level].mode );
    }

    if ( tree == NULL ) {
        _error_display(handle, NULL, "% No available command.\n");
        return ICLI_RC_ERR_EMPTY;
    }

    // execution
    while ( _cmd_word_get(handle, &w) == ICLI_RC_OK ) {

#if 1 /* CP, 2013/03/14 15:33, Bugzilla#11262, comment '!' and '#' processing */
        if ( tree->child == NULL ) {
            if ( handle->runtime_data.cmd_var ) {
                for ( p = handle->runtime_data.cmd_var; p->next; ___NEXT(p) ) {
                    ;
                }
                // if executable and next word is comment then end it
                if ( p->match_node->execution && p->match_node->execution->cmd_cb ) {
                    if ( ICLI_IS_COMMENT(*w) ) {
                        break;
                    }
                }
            }
        }

        b_comment_redo = FALSE;
#endif

_CMD_WALK_REDO:
        // reset rc
        handle->runtime_data.rc = ICLI_RC_ERR_MATCH;

#if ICLI_RANDOM_OPTIONAL
        // reset random optional
        _random_optional_reset();
#endif

#if 1 /* CP, 2012/10/11 16:09, performance enhancement: do not check same optional nodes */
        memset(g_match_optional_node, 0, sizeof(g_match_optional_node));
#endif

#if ICLI_RANDOM_MUST_NUMBER
        if ( _session_random_must_get(handle) == FALSE ) {
            T_W("_session_random_must_get()\n");
        }
#endif

        // find match nodes
        if ( b ) {
            rc = _match_node_find(handle, tree, w, TRUE, TRUE);
            switch ( rc ) {
            case ICLI_RC_OK:
                break;
            case ICLI_RC_ERR_REDO:
                _match_para_free( handle );
                handle->runtime_data.grep_var   = NULL;
                handle->runtime_data.grep_begin = 0;
                goto _CMD_WALK_REDO;
            default:
                _handle_para_free( handle );
                return ICLI_RC_ERROR;
            }
            b = FALSE;

#if 1 /* CP, 2013/03/14 15:33, Bugzilla#11262, comment '!' and '#' processing */
            b_current_b = TRUE;
#endif
        } else {
#if ICLI_RANDOM_MUST_NUMBER
            /*
                check if random must is enough or not
                if not then can not go to child
             */
            if ( _must_child_allowed(handle, tree) ) {
                rc = _match_node_find(handle, tree->child, w, TRUE, TRUE);
                switch ( rc ) {
                case ICLI_RC_OK:
                    break;
                case ICLI_RC_ERR_REDO:
                    _match_para_free( handle );
                    handle->runtime_data.grep_var   = NULL;
                    handle->runtime_data.grep_begin = 0;
                    goto _CMD_WALK_REDO;
                default:
                    _handle_para_free( handle );
                    return ICLI_RC_ERROR;
                }
#if 1 /* CP, 10/26/2013 18:29, do not match visited nodes */
            } else {
                // clear the previous matched nodes
                memset( g_matched_node, 0, sizeof(g_matched_node) );
#endif
            }
#else
            rc = _match_node_find(handle, tree->child, w, TRUE, TRUE);
            switch ( rc ) {
            case ICLI_RC_OK:
                break;
            case ICLI_RC_ERR_REDO:
                _match_para_free( handle );
                handle->runtime_data.grep_var   = NULL;
                handle->runtime_data.grep_begin = 0;
                goto _CMD_WALK_REDO;
            default:
                _handle_para_free( handle );
                return ICLI_RC_ERROR;
            }
#endif

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
            rc = _match_single_node(handle, tree->loop, w, TRUE, FALSE, FALSE, TRUE);
            switch ( rc ) {
            case ICLI_RC_OK:
                break;
            case ICLI_RC_ERR_REDO:
                _match_para_free( handle );
                handle->runtime_data.grep_var   = NULL;
                handle->runtime_data.grep_begin = 0;
                goto _CMD_WALK_REDO;
            default:
                _handle_para_free( handle );
                return ICLI_RC_ERROR;
            }
#endif

#if 1 /* CP, 2012/10/11 16:09, performance enhancement: do not check same optional nodes */
            memset(g_match_optional_node, 0, sizeof(g_match_optional_node));
#endif

#if ICLI_RANDOM_OPTIONAL
            rc = _random_optional_match(handle, tree, w, TRUE, FALSE, -1);
            switch ( rc ) {
            case ICLI_RC_OK:
                break;
            case ICLI_RC_ERR_REDO:
                _match_para_free( handle );
                handle->runtime_data.grep_var   = NULL;
                handle->runtime_data.grep_begin = 0;
                goto _CMD_WALK_REDO;
            default:
                _handle_para_free( handle );
                return ICLI_RC_ERROR;
            }
#endif

#if 1 /* CP, 2013/03/14 15:33, Bugzilla#11262, comment '!' and '#' processing */
            b_current_b = FALSE;
#endif
        }

#if 1 /* CP_LINE */
        /* pre-processing */
        if ( handle->runtime_data.total_match_cnt &&
             handle->runtime_data.exactly_cnt == 0 ) {
#if 1 /* CP, 2013/03/14 18:02, processing <string> */
            string_cnt = 0;
            b_remove_line = FALSE;
            b_remove_string = FALSE;
#endif
            for ( p = handle->runtime_data.match_para; p; ___NEXT(p) ) {
                switch ( p->match_node->type ) {
                case ICLI_VARIABLE_LINE:
                case ICLI_VARIABLE_RANGE_LINE:
                case ICLI_VARIABLE_GREP_STRING:
#if 1 /* CP, 2013/03/14 18:02, processing <string> */
                    if ( string_cnt ) {
                        T_E("Multiple line and string\n");
                        _handle_para_free( handle );
                        return ICLI_RC_ERR_MATCH;
                    }
                    ++string_cnt;
#endif
                    c = handle->runtime_data.cmd_walk;
                    if ( ICLI_NOT_(EOS, *c) || ICLI_NOT_(EOS, *(c + 1)) ) {

                        if ( ICLI_NOT_(EOS, *c) ) {
                            // for exec_cmd
                            // restore previous SPACE to let w
                            // to include the whole remaining string
                            *(c - 1) = ICLI_SPACE;
                        } else {
                            // for TAB and ?
                            // restore previous SPACE to let w
                            // to include the whole remaining string
                            *(c) = ICLI_SPACE;
                        }

                        // go to the end of the string
                        forever_loop = 1;
                        while ( forever_loop == 1 ) {
                            for ( ++c; ICLI_NOT_(EOS, *c); ++c ) {
                                ;
                            }
                            if ( ICLI_IS_(EOS, *(c + 1)) ) {
                                // all done
                                break;
                            }
                            // restore previous SPACE to let w
                            // to include the whole remaining string
                            *(c) = ICLI_SPACE;
                        }
                        // set cmd_walk to the end
                        // to tell the command walk is over
                        handle->runtime_data.cmd_walk = c;
                        // update value
#if 1 /* CP, 2013/03/14 18:02, processing <string> */
                        match_type = _node_match(handle, p->match_node, w, FALSE, &p);
                        switch ( match_type ) {
                        case ICLI_MATCH_TYPE_EXACTLY : /* exactly match */
                        case ICLI_MATCH_TYPE_PARTIAL : /* partial match for KEYWORD */
                            break;

                        default : /* error, invalid match type */
                            --( handle->runtime_data.total_match_cnt );
                            if ( handle->runtime_data.total_match_cnt ) {
                                // remove string in match_para
                                b_remove_line = TRUE;
                            } else {
#if 1 /* CP, 05/27/2013 09:48, CC-10162 */
                                handle->runtime_data.rc = ICLI_RC_ERR_MATCH;
#else
                                if ( match_type == ICLI_MATCH_TYPE_EXACTLY ) {
                                    handle->runtime_data.rc = ICLI_RC_ERR_INCOMPLETE;
                                } else {
                                    handle->runtime_data.rc = ICLI_RC_ERR_MATCH;
                                }
#endif
                            }
                            break;
                        }// switch
#else
                        if ( p->match_node->type == ICLI_VARIABLE_LINE ||
                             p->match_node->type == ICLI_VARIABLE_GREP_STRING ) {
                            c = p->value.u.u_line;
                        } else {
                            c = p->value.u.u_range_line;
                        }
                        (void)vtss_icli_str_cpy(c, w);
#endif
                    }
                    break;

#if 1 /* CP, 2013/03/14 18:02, processing <string> */
                case ICLI_VARIABLE_STRING:
                case ICLI_VARIABLE_RANGE_STRING:
                    if ( string_cnt ) {
                        T_E("Multiple line and string\n");
                        _handle_para_free( handle );
                        return ICLI_RC_ERR_MATCH;
                    }
                    ++string_cnt;

                    // check if string is already complete
                    for ( c = w; ICLI_NOT_(EOS, *c); ++c ) {
                        ;
                    }
                    if ( (c - 1) != w && ICLI_IS_(STRING_END, *(c - 1)) ) {
                        break;
                    }

                    c = handle->runtime_data.cmd_walk;
                    if ( ICLI_NOT_(EOS, *c) || ICLI_NOT_(EOS, *(c + 1)) ) {

                        if ( ICLI_NOT_(EOS, *c) ) {
                            // for exec_cmd
                            // restore previous SPACE to let w
                            // to include the whole remaining string
                            *(c - 1) = ICLI_SPACE;
                            --c;
                        } else {
                            // for TAB and ?
                            // restore previous SPACE to let w
                            // to include the whole remaining string
                            *(c) = ICLI_SPACE;
                        }

                        // go to find string end
                        b_string = FALSE;
                        forever_loop = 1;
                        while ( forever_loop == 1 ) {
                            for ( ++c; ICLI_NOT_(EOS, *c) && ICLI_NOT_(STRING_END, *c); ++c ) {
                                ;
                            }
                            if ( ICLI_IS_(EOS, *c) ) {
                                if ( ICLI_IS_(EOS, *(c + 1)) ) {
                                    *c++ = ICLI_STRING_END;
                                    b_string = TRUE;
                                    handle->runtime_data.b_string_end = FALSE;
                                    break;
                                }
                            } else {
                                if ( ICLI_IS_(SPACE, *(c + 1)) ) {
                                    // found
                                    ++c;
                                    *c = ICLI_EOS;
                                    ++c;
                                    b_string = TRUE;
                                    break;
                                } else if ( ICLI_IS_(EOS, *(c + 1)) ) {
                                    // found
                                    ++c;
                                    b_string = TRUE;
                                    break;
                                }
                            }

                            // restore previous SPACE to let w
                            // to include the whole remaining string
                            *c = ICLI_SPACE;
                        }
                    } else {
                        if ( ICLI_IS_(EOS, *(c - 1)) ) {
                            *(c - 1) = ICLI_SPACE;
                        } else if ( ICLI_NOT_(SPACE, *(c - 1)) ) {
                            if ( handle->runtime_data.cmd[handle->runtime_data.cmd_len - 1] == ICLI_SPACE ) {
                                *c++ = ICLI_SPACE;
                            }
                        }
                        if ( vtss_icli_str_len(w) == 1 ) {
                            b_string = FALSE;
                            b_remove_string = TRUE;
                        } else {
                            *c++ = ICLI_STRING_END;
                            b_string = TRUE;
                            handle->runtime_data.b_string_end = FALSE;
                        }
                    }

                    // set cmd_walk
                    handle->runtime_data.cmd_walk = c;

                    match_type = ICLI_MATCH_TYPE_EXACTLY;
                    if ( b_string ) {
                        // update value
                        match_type = _node_match(handle, p->match_node, w, FALSE, &p);
                        switch ( match_type ) {
                        case ICLI_MATCH_TYPE_EXACTLY : /* exactly match */
                        case ICLI_MATCH_TYPE_PARTIAL : /* partial match for KEYWORD */
                            break;

                        default : /* error, invalid match type */
                            //T_E("invalid match type(%d)\n", match_type);
                            b_string = FALSE;
                            break;
                        }// switch
                    }
                    if ( b_string == FALSE ) {
                        --( handle->runtime_data.total_match_cnt );
                        if ( handle->runtime_data.total_match_cnt ) {
                            // remove string in match_para
                            b_remove_string = TRUE;
                        } else {
                            if ( match_type == ICLI_MATCH_TYPE_EXACTLY ) {
                                handle->runtime_data.rc = ICLI_RC_ERR_INCOMPLETE;
                            } else {
                                handle->runtime_data.rc = ICLI_RC_ERR_MATCH;
                            }
                        }
                    }
                    break;
#endif /* CP, 2013/03/14 18:02, processing <string> */

                default:
                    break;
                }
            }

#if 1 /* CP, 2013/03/14 18:02, processing <string> */
            if ( b_remove_line ) {
                b_string = FALSE;
                prev = NULL;
                for ( p = handle->runtime_data.match_para; p; ___NEXT(p) ) {
                    switch ( p->match_node->type ) {
                    case ICLI_VARIABLE_LINE:
                    case ICLI_VARIABLE_RANGE_LINE:
                    case ICLI_VARIABLE_GREP_STRING:
                        if ( prev ) {
                            prev->next = p->next;
                        } else {
                            handle->runtime_data.match_para = p->next;
                        }
                        vtss_icli_exec_parameter_free( p );
                        b_string = TRUE;
                        break;
                    default:
                        break;
                    }
                    if ( b_string ) {
                        break;
                    }
                    prev = p;
                }
            } else if ( b_remove_string ) {
                b_string = FALSE;
                prev = NULL;
                for ( p = handle->runtime_data.match_para; p; ___NEXT(p) ) {
                    switch ( p->match_node->type ) {
                    case ICLI_VARIABLE_STRING:
                    case ICLI_VARIABLE_RANGE_STRING:
                        if ( prev ) {
                            prev->next = p->next;
                        } else {
                            handle->runtime_data.match_para = p->next;
                        }
                        vtss_icli_exec_parameter_free( p );
                        b_string = TRUE;
                        break;
                    default:
                        break;
                    }
                    if ( b_string ) {
                        break;
                    }
                    prev = p;
                }
            }
#endif
        }
#endif /* CP_LINE */

#if 1 /* CP, 2013/03/14 15:33, Bugzilla#11262, comment '!' and '#' processing */
        if ( b_comment_redo ) {
            switch ( handle->runtime_data.total_match_cnt ) {
            case 0:
                _error_display(handle, w, "Invalid");
                _handle_para_free( handle );
                return ICLI_RC_ERR_MATCH;

            case 1:
                p = handle->runtime_data.match_para;
                if ( p->match_node->execution == NULL || p->match_node->execution->cmd_cb == NULL ) {
                    _error_display(handle, w, "Invalid");
                    _handle_para_free( handle );
                    return ICLI_RC_ERR_MATCH;
                }
                // terminate command
                *(handle->runtime_data.cmd_walk) = ICLI_EOS;
                break;

            default:
                i = 0;
                for ( p = handle->runtime_data.match_para; p; ___NEXT(p) ) {
                    if ( p->match_node->execution && p->match_node->execution->cmd_cb ) {
                        ++i;
                    }
                }
                if ( i ) {
                    _error_display(handle, w, "Ambiguous");
                } else {
                    _error_display(handle, w, "Invalid");
                }
                _handle_para_free( handle );
                return ICLI_RC_ERR_MATCH;
            }
        }
#endif

        /* check the find result */
        switch ( handle->runtime_data.total_match_cnt ) {
        case 0 : /* no match */
            /* check rc */
            if ( handle->runtime_data.rc == ICLI_RC_ERR_INCOMPLETE ) {
                if ( handle->runtime_data.exec_type == ICLI_EXEC_TYPE_TAB ) {
                    ICLI_PUT_NEWLINE;
                }
                _error_display(handle, w, "Incomplete");
            } else {
                if ( handle->runtime_data.cmd_var ) {
                    // find the last one
                    for ( p = handle->runtime_data.cmd_var; p->next; ___NEXT(p) ) {
                        ;
                    }

                    // check if loosely parsing
#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */
                    if ( p->match_node->execution && p->match_node->execution->cmd_cb ) {
                        for ( node_prop = &(p->match_node->node_property); node_prop; ___NEXT(node_prop) ) {
                            if ( node_prop->cmd_property->cmd_id == p->match_node->execution->cmd_id ) {
                                if ( ICLI_CMD_IS_LOOSELY(node_prop->cmd_property->property) ) {
                                    if ( handle->runtime_data.exec_type == ICLI_EXEC_TYPE_CMD ) {
                                        _match_para_free( handle );
                                    }
                                    /* do not free cmd_var for LOOSELY when parsing. Otherwise, crash */
                                    return ICLI_RC_ERR_EXCESSIVE;
                                }
                                break;
                            }
                        }

                    } else {
#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
                        if ( p->match_node->child ) {
                            or_head = 0;
                            for ( sibling = p->match_node->child; sibling; ___SIBLING(sibling) ) {
                                if ( _command_present_chk(handle, sibling) == FALSE ) {
                                    continue;
                                }
                                if ( _present_chk(handle, sibling) ) {
                                    continue;
                                } else {
                                    if ( sibling->or_head ) {
                                        if ( sibling->or_head != or_head ) {
                                            // not handle yet, very difficult, TBD
                                        }
                                    } else {
                                        for ( node = sibling; node; ___CHILD(node) ) {
                                            if ( _present_chk(handle, node) ) {
                                                break;
                                            } else {
                                                if ( node != sibling ) {
                                                    b_sibling = FALSE;
                                                    for ( node_sibling = node->sibling; node_sibling; ___SIBLING(node_sibling) ) {
                                                        if ( _present_chk(handle, node_sibling) ) {
                                                            b_sibling = TRUE;
                                                            break;
                                                        }
                                                    }
                                                    if ( b_sibling ) {
                                                        continue;
                                                    }
                                                }
                                                if ( node->execution && node->execution->cmd_cb ) {
                                                    for ( node_prop = &(node->node_property); node_prop; ___NEXT(node_prop) ) {
                                                        if ( node_prop->cmd_property->cmd_id == node->execution->cmd_id ) {
                                                            if ( ICLI_CMD_IS_LOOSELY(node_prop->cmd_property->property) ) {
                                                                if ( handle->runtime_data.exec_type == ICLI_EXEC_TYPE_CMD ) {
                                                                    _match_para_free( handle );
                                                                }
                                                                /* do not free cmd_var for LOOSELY when parsing. Otherwise, crash */
                                                                return ICLI_RC_ERR_EXCESSIVE;
                                                            }
                                                            break;
                                                        }
                                                    } // for
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
#else
                        for ( node = p->match_node->child; node; ___CHILD(node) ) {
                            if ( _present_chk(handle, node) ) {
                                break;
                            } else {
                                b_sibling = FALSE;
                                if ( node->sibling && (node->optional_begin == 0) ) {
                                    for ( sibling = node->sibling; sibling; ___SIBLING(sibling) ) {
                                        if ( _present_chk(handle, sibling) ) {
                                            b_sibling = TRUE;
                                            break;
                                        }
                                    }
                                    if ( b_sibling ) {
                                        break;
                                    }
                                }
                                if ( node->execution && node->execution->cmd_cb ) {
                                    for ( node_prop = &(node->node_property); node_prop; ___NEXT(node_prop) ) {
                                        if ( node_prop->cmd_property->cmd_id == node->execution->cmd_id ) {
                                            if ( ICLI_CMD_IS_LOOSELY(node_prop->cmd_property->property) ) {
                                                if ( handle->runtime_data.exec_type == ICLI_EXEC_TYPE_CMD ) {
                                                    _match_para_free( handle );
                                                }
                                                /* do not free cmd_var for LOOSELY when parsing. Otherwise, crash */
                                                return ICLI_RC_ERR_EXCESSIVE;
                                            }
                                            break;
                                        }
                                    } // for
                                }
                            }
                        } // for ( node )
#endif
                    } // if
#endif
                }
                if ( handle->runtime_data.exec_type == ICLI_EXEC_TYPE_TAB ) {
                    ICLI_PUT_NEWLINE;
                }

#if 1 /* CP, 2013/03/14 15:33, Bugzilla#11262, comment '!' and '#' processing */
                if ( ICLI_IS_COMMENT(*w) ) {
                    if ( handle->runtime_data.cmd_var ) {
                        for ( p = handle->runtime_data.cmd_var; p->next; ___NEXT(p) ) {
                            ;
                        }
                        // if executable and next word is comment then end it
                        if ( p->match_node->execution && p->match_node->execution->cmd_cb ) {
                            // terminate command
                            *(handle->runtime_data.cmd_walk) = ICLI_EOS;
                            break;
                        }
                    }
                } else {
                    for ( c = w; ICLI_NOT_(SPACE, *c) && ICLI_NOT_(EOS, *c); ++c ) {
                        if ( ICLI_IS_COMMENT(*c) ) {
                            *c = ICLI_EOS;
                            b = b_current_b;
                            b_comment_redo = TRUE;
                            _match_para_free( handle );
                            handle->runtime_data.grep_var   = NULL;
                            handle->runtime_data.grep_begin = 0;
                            goto _CMD_WALK_REDO;
                        }
                    }
                }
#endif
                _error_display(handle, w, "Invalid");
            }
            _handle_para_free( handle );
            return ICLI_RC_ERR_MATCH;

        case 1 : /* one match */
            _cmd_var_put( handle, handle->runtime_data.match_para, FALSE );
#if 1 /* RANDOM_OPTIONAL */
            tree = handle->runtime_data.match_para->match_node;
#else
            tree = handle->runtime_data.match_para->match_node->child;
#endif
            // use this
            handle->runtime_data.match_para = NULL;
            break;

        default : /* multiple match, then find the exact match */
            // pre-process match_para
            if ( handle->runtime_data.exactly_cnt == 0 ) {
                for ( p = handle->runtime_data.match_para; p; ___NEXT(p) ) {
                    if ( vtss_icli_variable_data_string_type_get(p->value.type) ) {
                        p->match_type = ICLI_MATCH_TYPE_EXACTLY;
                        ++( handle->runtime_data.exactly_cnt );
                    }
                }
            }
#if 1 /* keyword has the highest priority */
            else if ( handle->runtime_data.exactly_cnt > 1 ) {
                u32    keyword_cnt = 0;

                for ( p = handle->runtime_data.match_para; p; ___NEXT(p) ) {
                    if ( p->value.type == ICLI_VARIABLE_KEYWORD ) {
                        ++keyword_cnt;
                    }
                }

                if ( keyword_cnt == 1 ) {
                    for ( p = handle->runtime_data.match_para; p; ___NEXT(p) ) {
                        if ( p->value.type != ICLI_VARIABLE_KEYWORD ) {
                            p->match_type = ICLI_MATCH_TYPE_PARTIAL;
                        }
                    }
                    handle->runtime_data.exactly_cnt = 1;
                }
            }
#endif

            if ( handle->runtime_data.exactly_cnt != 1 ) {
                if ( handle->runtime_data.b_keep_match_para == FALSE ) {
                    _handle_para_free( handle );
                }
                if ( handle->runtime_data.exec_type == ICLI_EXEC_TYPE_TAB ) {
                    ICLI_PUT_NEWLINE;
                }
                _error_display(handle, w, "Ambiguous");
                return ICLI_RC_ERR_AMBIGUOUS;
            }

            // get the exact one
            p = handle->runtime_data.match_para;
#if 1 /* CC-11085 */
            if ( p == NULL ) {
                T_E("p is NULL.\n");
                return ICLI_RC_ERROR;
            }
#endif

            if ( p->match_type == ICLI_MATCH_TYPE_EXACTLY ) {
                handle->runtime_data.match_para = p->next;
            } else {
                for (prev = p, p = p->next; p; prev = p, ___NEXT(p)) {
                    if ( p->match_type == ICLI_MATCH_TYPE_EXACTLY ) {
                        prev->next = p->next;
                        break;
                    }
                }
            }

#if 1 /* RANDOM_OPTIONAL */
            if ( p ) {
                _cmd_var_put( handle, p, FALSE );
                tree = p->match_node;
            } else {
                T_E("p is NULL.\n");
                return ICLI_RC_ERROR;
            }
#else
            tree = p->match_node->child;
#endif
            break;
        }

        _match_para_free( handle );
    }// while

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
    /* integrate loop for execution */
    if ( handle->runtime_data.exec_type == ICLI_EXEC_TYPE_CMD ) {
        b        = FALSE;
        prev     = NULL;
        para     = NULL;
        para_spr = NULL;
        for ( p = handle->runtime_data.cmd_var; p; ___NEXT(p) ) {
            if ( b ) {
                if ( p->b_in_loop == FALSE ) {
                    b    = FALSE;
                    para = NULL;
                    continue;
                }
                switch ( p->value.type ) {
                case ICLI_VARIABLE_PORT_TYPE:
                    prev = p;
                    break;

                case ICLI_VARIABLE_PORT_TYPE_LIST:
                    // integrate
                    if ( para_spr == NULL ) {
                        T_E("para_spr is NULL.\n");
                        return ICLI_RC_ERROR;
                    }

                    if ( vtss_icli_port_type_list_add(&(p->value.u.u_port_type_list), para_spr) == FALSE ) {
                        T_E("vtss_icli_port_type_list_add()\n");
                        break;
                    }

                    // free prev
                    vtss_icli_exec_parameter_free( prev );

                    // free p
                    // for lint
                    if ( para ) {
                        para->next = p->next;
                    } else {
                        T_E("para is NULL.\n");
                        return ICLI_RC_ERROR;
                    }
                    vtss_icli_exec_parameter_free( p );
                    p = para;
                    break;

                default:
                    T_W("the type, %d, not support loop\n", p->value.type);
                    T_W("the word_id, %d, not support loop\n", p->word_id);
                    T_W("the word_id, %s, not support loop\n", p->match_node->word);
                    break;
                }
            } else {
                if ( p->b_in_loop ) {
                    b        = TRUE;
                    para     = p;
                    para_spr = &( p->value.u.u_port_type_list );
                }
            }
        }
    }
#endif

    return ICLI_RC_OK;
}

static void _cmd_buf_cpy(
    IN  icli_session_handle_t   *handle
)
{
    char    *c;
    char    *w;
    char    *prev_w;
    char    *s;
    i32     len;

    // reset
    c = handle->runtime_data.cmd_copy;
    memset(c, 0, sizeof(handle->runtime_data.cmd_copy));

    // check length
    len = handle->runtime_data.cmd_len;
    if ( len == 0 ) {
        return;
    }

    // check if all spaces
    // skip space
    ICLI_SPACE_SKIP(w, handle->runtime_data.cmd);

    // EOS
    if ( ICLI_IS_(EOS, (*w)) ) {
        return;
    }

    // copy to cmd_copy
    *c = ICLI_EOS;
    ++c;
    (void)vtss_icli_str_cpy(c, handle->runtime_data.cmd);
    handle->runtime_data.cmd_walk = c;
    handle->runtime_data.cmd_start = c;

    prev_w = NULL;
    while ( _cmd_word_get(handle, &w) == ICLI_RC_OK ) {
        if ( prev_w ) {
            for ( s = prev_w - 1; ICLI_NOT_(EOS, *s); --s ) {
                ;
            }
            *s = ICLI_SPACE;
        }
        prev_w = w;
    }

    // make w go to EOS
    for ( w = prev_w; ICLI_NOT_(EOS, *w); ++w ) {
        ;
    }

    // check
    if ( handle->runtime_data.cmd_walk != w ) {
        for ( s = prev_w - 1; ICLI_NOT_(EOS, *s); --s ) {
            ;
        }
        *s = ICLI_SPACE;
    }
}

static void _cr_get(
    IN icli_session_handle_t    *handle,
    IN icli_parameter_t         *prev_para,
    IN icli_parameter_t         *para,
    IN BOOL                     b_add,
    IN BOOL                     b_visible_chk
)
{
    int                         d;
    u32                         i;
    icli_switch_port_range_t    spr;
    node_property_t             *node_prop;

    /*
        check if the execution node's privilege is higher than session
        privilege. if not, then it can not be executed.
    */
    for ( node_prop = &(para->match_node->node_property); node_prop; ___NEXT(node_prop) ) {
        if ( node_prop->cmd_property->cmd_id == para->match_node->execution->cmd_id ) {
            if ( node_prop->cmd_property->privilege > handle->runtime_data.privilege ) {
                return;
            }
            break;
        }
    }

    /* check if random must is enough, otherwise never <cr> */
    for ( i = 0; i < ICLI_RANDOM_MUST_CNT; ++i ) {
        if ( handle->runtime_data.random_must_head[i] ) {
            d = handle->runtime_data.random_must_match[i] - handle->runtime_data.random_must_head[i]->random_must_number;
            if ( d < 0 ) {
                if ( d == -1 ) {
                    if ( b_add ) {
                        if ( para->match_node != handle->runtime_data.random_must_head[i] ) {
                            if ( vtss_icli_parsing_random_head(handle->runtime_data.random_must_head[i], para->match_node, NULL) == FALSE ) {
                                return;
                            }
                        }
                    } else {
                        return;
                    }
                } else {
                    return;
                }
            }
        }
    }

    /* get <cr> */
    switch ( para->value.type ) {
    case ICLI_VARIABLE_PORT_ID:
        if ( prev_para && prev_para->value.type == ICLI_VARIABLE_PORT_TYPE ) {
            spr.switch_id  = para->value.u.u_port_id.switch_id;
            spr.begin_port = para->value.u.u_port_id.begin_port;
            spr.port_type  = prev_para->value.u.u_port_type;
            if ( vtss_icli_port_get(&spr) ) {
                if ( b_visible_chk ) {
                    if ( _visible_chk(handle, para->match_node) ) {
                        handle->runtime_data.b_cr = TRUE;
                    }
                } else {
                    handle->runtime_data.b_cr = TRUE;
                }
            }
        }
        break;

    case ICLI_VARIABLE_PORT_LIST:
        if ( prev_para && prev_para->value.type == ICLI_VARIABLE_PORT_TYPE ) {
            if ( vtss_icli_port_type_list_get( prev_para->value.u.u_port_type,
                                               &(para->value.u.u_port_list),
                                               NULL,
                                               NULL ) ) {
                if ( b_visible_chk ) {
                    if ( _visible_chk(handle, para->match_node) ) {
                        handle->runtime_data.b_cr = TRUE;
                    }
                } else {
                    handle->runtime_data.b_cr = TRUE;
                }
            }
        }
        break;

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
    case ICLI_VARIABLE_PORT_TYPE:
#if 1
        if ( para->match_node->child                                       &&
             para->match_node->child->type == ICLI_VARIABLE_PORT_TYPE_LIST &&
             ___LOOP_HEADER(para->match_node)                              ) {
            if ( para->value.u.u_port_type != ICLI_PORT_TYPE_ALL ) {
                return;
            }
        }
        // fall through
#else
        if ( para->match_node->child                                       &&
             para->match_node->child->type == ICLI_VARIABLE_PORT_TYPE_LIST &&
             ___LOOP_HEADER(para->match_node)                              ) {
            if ( para->value.u.u_port_type == ICLI_PORT_TYPE_ALL ) {
                handle->runtime_data.b_cr = TRUE;
            }
        } else {
            handle->runtime_data.b_cr = TRUE;
        }
        break;
#endif
#endif

    default:
        if ( b_visible_chk ) {
            if ( _visible_chk(handle, para->match_node) ) {
                handle->runtime_data.b_cr = TRUE;
            }
        } else {
            handle->runtime_data.b_cr = TRUE;
        }
        break;
    }
}

/*
    find next nodes for the user command
    the result is stored in cmd_var

    INPUT
        handle : session handle

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _next_node_find(
    IN  icli_session_handle_t   *handle
)
{
    char                    *w;
    char                    *c;
    i32                     rc;
    icli_match_type_t       match_type;
    icli_parsing_node_t     *tree;
    icli_parsing_node_t     *line_match_node;
    icli_parameter_t        *p;
    icli_parameter_t        *n;
    icli_parameter_t        *prev_node;
    icli_parameter_t        *last_node;
    node_property_t         *node_prop;

#if 1 /* RANDOM_OPTIONAL */
    BOOL                    b;
#endif

#if ICLI_RANDOM_OPTIONAL
    i32                     b_in_loop;
#endif

#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */
    icli_parsing_node_t     *node;
    icli_parsing_node_t     *sibling;
    icli_parsing_node_t     *exec_link_node;
    BOOL                    b_exec_link;
    BOOL                    b_sibling;
#endif

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
    u32                     or_head;
    icli_parsing_node_t     *node_sibling;
#endif

    // get command into str buffer for further process
    _cmd_buf_cpy( handle );

    // get first word
    handle->runtime_data.cmd_walk = handle->runtime_data.cmd_copy;

#if 1 /* CP, 2013/03/14 18:02, processing <string> */
    handle->runtime_data.b_string_end = TRUE;
#endif

    p = NULL;
    b = FALSE;
    if ( vtss_icli_str_len(handle->runtime_data.cmd_walk) == 0 ) {
        // empty then get first possible node
        if ( handle->runtime_data.b_in_parsing ) {
            tree = vtss_icli_parsing_tree_get( handle->runtime_data.current_parsing_mode );
        } else {
            tree = vtss_icli_parsing_tree_get( handle->runtime_data.mode_para[handle->runtime_data.mode_level].mode );
        }
    } else {
        // execute command
        T_D("_next_node_find, walk:");
        rc = _cmd_walk( handle );
        switch ( rc ) {
        case ICLI_RC_ERR_EXCESSIVE:
            handle->runtime_data.b_cr = TRUE;
            return ICLI_RC_OK;

        case ICLI_RC_OK:
            break;

        default:
            return rc;
        }

#if 1 /* CP, 2013/03/14 18:02, processing <string> */
        // find the last one
        prev_node = NULL;
        for ( p = handle->runtime_data.cmd_var; p->next; ___NEXT(p) ) {
            prev_node = p;
        }

        switch ( p->match_node->type ) {
        case ICLI_VARIABLE_STRING:
        case ICLI_VARIABLE_RANGE_STRING:
            if ( ICLI_IS_(EOS, *(handle->runtime_data.cmd_walk + 1)) ) {
                c = handle->runtime_data.cmd + handle->runtime_data.cmd_len - 1;
                if ( ICLI_IS_(STRING_END, *c) ) {
                    if ( prev_node ) {
                        prev_node->next = NULL;
                        vtss_icli_exec_parameter_free( p );
                        p = prev_node;
                    }
                    break;
                }

                for ( ; ICLI_IS_(SPACE, *c); --c ) {
                    ;
                }
                // it means string is not end yet
                if ( handle->runtime_data.b_string_end == FALSE ) {
                    if ( prev_node ) {
                        prev_node->next = NULL;
                        vtss_icli_exec_parameter_free( p );
                        p = prev_node;
                        // reset cmd_walk to the string start
                        for ( c = handle->runtime_data.cmd_walk; ICLI_NOT_(STRING_BEGIN, *c); --c ) {
                            ;
                        }
                        for ( --c; ICLI_NOT_(STRING_BEGIN, *c); --c ) {
                            ;
                        }
                        handle->runtime_data.cmd_walk = c - 1;
                    }
                }
            }
            break;
        default:
            break;
        }
#else
        // find the last one
        for ( p = handle->runtime_data.cmd_var; p->next; ___NEXT(p) ) {
            ;
        }

#endif

        // get tree
#if 1 /* RANDOM_OPTIONAL */
        b = TRUE;
        tree = p->match_node;
#else
        tree = p->match_node->child;
#endif
    }

    // skip space, get the last word
    ICLI_SPACE_SKIP(w, (handle->runtime_data.cmd_walk + 1));
    // find EOS
    for ( c = w; ICLI_NOT_(EOS, *c); ++c ) {
        ;
    }
    // update cmd_walk
    handle->runtime_data.cmd_walk = c;

    /* get last node */
    prev_node = NULL;
    last_node = NULL;
    if ( handle->runtime_data.cmd_var ) {
        for ( p = handle->runtime_data.cmd_var;
              p->next != NULL;
              prev_node = p, p = p->next ) {
            ;
        }

        if ( p->value.type == ICLI_VARIABLE_PORT_TYPE ) {
            last_node = p;
            if ( prev_node ) {
                prev_node->next = NULL;
            } else {
                handle->runtime_data.cmd_var = NULL;
            }
            handle->runtime_data.last_port_type = p->value.u.u_port_type;
        } else {
            handle->runtime_data.last_port_type = ICLI_PORT_TYPE_NONE;
        }

        if ( prev_node && prev_node->value.type != ICLI_VARIABLE_PORT_TYPE ) {
            prev_node = NULL;
        }
    }

#if ICLI_RANDOM_MUST_NUMBER
    if ( _session_random_must_get(handle) == FALSE ) {
        // free all parameters
        _handle_para_free( handle );
        if ( last_node ) {
            vtss_icli_exec_parameter_free( last_node );
        }
        return ICLI_RC_ERROR;
    }
#endif

    /* check b_cr */
    handle->runtime_data.b_cr = FALSE;
    line_match_node = NULL;
    if ( ICLI_IS_(EOS, *w) && handle->runtime_data.cmd_var ) {
        // check if executable
#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */
        exec_link_node = NULL;
        b_exec_link = FALSE;
        if ( p && (p->match_node->execution == NULL || p->match_node->execution->cmd_cb == NULL) ) {
#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
            if ( p->match_node->child ) {
                or_head = 0;
                for ( sibling = p->match_node->child; sibling; ___SIBLING(sibling) ) {
                    if ( _command_present_chk(handle, sibling) == FALSE ) {
                        continue;
                    }
                    if ( _present_chk(handle, sibling) ) {
                        continue;
                    } else {
                        if ( sibling->or_head ) {
                            if ( sibling->or_head != or_head ) {
                                // not handle yet, very difficult, TBD
                            }
                        } else {
                            for ( node = sibling; node; ___CHILD(node) ) {
                                if ( _present_chk(handle, node) ) {
                                    break;
                                } else {
#if 1 /* CP, 10/27/2013 22:01 */
                                    if ( node->random_must_head && node->child ) {
                                        continue;
                                    }
#endif
                                    if ( node != sibling ) {
                                        b_sibling = FALSE;
                                        for ( node_sibling = node->sibling; node_sibling; ___SIBLING(node_sibling) ) {
                                            if ( _present_chk(handle, node_sibling) ) {
                                                b_sibling = TRUE;
                                                break;
                                            }
                                        }
                                        if ( b_sibling ) {
                                            continue;
                                        }
                                    }
                                    if ( node->execution && node->execution->cmd_cb ) {
                                        p->match_node->execution = node->execution;
                                        b_exec_link = TRUE;
                                        exec_link_node = p->match_node;
                                        break;
                                    }
                                }
                            }
                            if ( b_exec_link ) {
                                break;
                            }
                        }
                    }
                }
            }
#else
            for ( node = p->match_node->child; node; ___CHILD(node) ) {
                if ( _present_chk(handle, node) ) {
                    break;
                } else {
                    b_sibling = FALSE;
                    if ( node->sibling && (node->optional_begin == 0) ) {
                        for ( sibling = node->sibling; sibling; ___SIBLING(sibling) ) {
                            if ( _present_chk(handle, sibling) ) {
                                b_sibling = TRUE;
                                break;
                            }
                        }
                        if ( b_sibling ) {
                            break;
                        }
                    }
                    if ( node->execution && node->execution->cmd_cb ) {
                        p->match_node->execution = node->execution;
                        b_exec_link = TRUE;
                        exec_link_node = p->match_node;
                        break;
                    }
                }
            }
#endif
        }
#endif

        if ( p && p->match_node->execution && p->match_node->execution->cmd_cb ) {
            /* get <cr> */
            _cr_get(handle, prev_node, p, FALSE, TRUE);
            // get line_match_node
            switch ( p->value.type ) {
            case ICLI_VARIABLE_LINE:
            case ICLI_VARIABLE_RANGE_LINE:
            case ICLI_VARIABLE_GREP_STRING:
#if 1 /* CP, 2012/08/30 09:12, do command by CLI command */
                if ( _visible_chk(handle, p->match_node) ) {
                    line_match_node = p->match_node;
                }
#endif
                break;
            default:
                break;
            }
        }

#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */
        if ( b_exec_link && exec_link_node ) {
            exec_link_node->execution = NULL;
        }
#endif
    }

    /* free all parameters first */
#if 1 /* RANDOM_OPTIONAL */
    _match_para_free( handle );
    handle->runtime_data.grep_var   = NULL;
    handle->runtime_data.grep_begin = 0;
#else
    _handle_para_free( handle );
#endif

#if 1 /* CP, 2012/10/11 16:09, performance enhancement: do not check same optional nodes */
    memset(g_match_optional_node, 0, sizeof(g_match_optional_node));
#endif

    // find next possible nodes
    if ( b ) {

#if ICLI_RANDOM_OPTIONAL
        // reset random optional
        _random_optional_reset();
#endif

#if ICLI_RANDOM_MUST_NUMBER
        /*
            check if random must is enough or not
            if not then can not go to child
         */
        if ( _must_child_allowed(handle, tree) ) {
            // find match node
            rc = _match_node_find(handle, tree->child, "", FALSE, TRUE);
            switch ( rc ) {
            case ICLI_RC_ERR_REDO:
            default:
                break;
            }
#if 1 /* CP, 10/26/2013 18:29, do not match visited nodes */
        } else {
            // clear the previous matched nodes
            memset( g_matched_node, 0, sizeof(g_matched_node) );
#endif
        }
#else
        /* find match node */
        rc = _match_node_find(handle, tree->child, "", FALSE, TRUE);
        switch ( rc ) {
        case ICLI_RC_ERR_REDO:
        default:
            break;
        }
#endif

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
        rc = _match_single_node(handle, tree->loop, "", FALSE, FALSE, FALSE, TRUE);
        switch ( rc ) {
        case ICLI_RC_ERR_REDO:
        default:
            break;
        }
#endif

#if 1 /* CP, 2012/10/11 16:09, performance enhancement: do not check same optional nodes */
        memset(g_match_optional_node, 0, sizeof(g_match_optional_node));
#endif

        // find in random optional
#if ICLI_RANDOM_OPTIONAL

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
        b_in_loop = handle->runtime_data.b_in_loop;
        handle->runtime_data.b_in_loop = FALSE;
#endif

        rc = _random_optional_match(handle, tree, "", FALSE, FALSE, -1);

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
        handle->runtime_data.b_in_loop = b_in_loop;
#endif
        switch ( rc ) {
        case ICLI_RC_OK:
            break;

        case ICLI_RC_ERR_REDO:
            if ( last_node ) {
                vtss_icli_exec_parameter_free( last_node );
            }
            return ICLI_RC_ERR_REDO;

        default:
            _handle_para_free( handle );
            if ( last_node ) {
                vtss_icli_exec_parameter_free( last_node );
            }
            return ICLI_RC_ERROR;
        }
#endif // ICLI_RANDOM_OPTIONAL

    } else {
        rc = _match_node_find(handle, tree, "", FALSE, TRUE);
        switch ( rc ) {
        case ICLI_RC_ERR_REDO:
        default:
            break;
        }
    }

    if ( *w &&
         handle->runtime_data.cmd_var &&
         handle->runtime_data.match_para == NULL  ) {
        // find the last one
        for ( p = handle->runtime_data.cmd_var; p->next; ___NEXT(p) ) {
            ;
        }
        // check if loosely parsing
        if ( p->match_node->execution && p->match_node->execution->cmd_cb ) {
            for ( node_prop = &(p->match_node->node_property); node_prop; ___NEXT(node_prop) ) {
                if ( node_prop->cmd_property->cmd_id == p->match_node->execution->cmd_id ) {
                    if ( ICLI_CMD_IS_LOOSELY(node_prop->cmd_property->property) ) {
                        _handle_para_free( handle );
                        handle->runtime_data.b_cr = TRUE;
                        return ICLI_RC_OK;
                    }
                    break;
                }
            }
        }

        for ( node = p->match_node->child; node; node = node->child ) {
            if ( _present_chk(handle, node) ) {
                break;
            } else {
                b_sibling = FALSE;
                if ( node->sibling && (node->optional_begin == 0) ) {
                    for ( sibling = node->sibling; sibling; ___SIBLING(sibling) ) {
                        if ( _present_chk(handle, sibling) ) {
                            b_sibling = TRUE;
                            break;
                        }
                    }
                    if ( b_sibling ) {
                        break;
                    }
                }
                if ( node->execution && node->execution->cmd_cb ) {
                    for ( node_prop = &(node->node_property); node_prop; ___NEXT(node_prop) ) {
                        if ( node_prop->cmd_property->cmd_id == node->execution->cmd_id ) {
                            if ( ICLI_CMD_IS_LOOSELY(node_prop->cmd_property->property) ) {
                                _handle_para_free( handle );
                                handle->runtime_data.b_cr = TRUE;
                                return ICLI_RC_OK;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

#if 0 /* CP, 2012/12/12 18:09, match_sort_list */
    /* clear cmd_var list for prepare the match list */
    vtss_icli_exec_para_list_free( &(handle->runtime_data.cmd_var) );
#endif

    //check the last word
    handle->runtime_data.exactly_cnt     = 0;
    handle->runtime_data.partial_cnt     = 0;
    handle->runtime_data.total_match_cnt = 0;
    for ( p = handle->runtime_data.match_para; p != NULL; p = n ) {
        n = p->next;
        match_type = _node_match(handle, p->match_node, w, FALSE, &p);
        switch ( match_type ) {
#if 0 /* CP, 2013/03/14 18:02, processing <string> */
        case ICLI_MATCH_TYPE_REDO:
            /*
                if parsing ICLI_VARIABLE_LINE, then cmd word is re-aligned.
                So, redo the match for all possible next nodes.
            */
            return ICLI_RC_ERR_REDO;
#endif

        case ICLI_MATCH_TYPE_EXACTLY : /* exactly match */
            ++( handle->runtime_data.exactly_cnt );

        // fall through

        case ICLI_MATCH_TYPE_PARTIAL : /* partial match for KEYWORD */
            ++( handle->runtime_data.total_match_cnt );
            if ( match_type == ICLI_MATCH_TYPE_PARTIAL ) {
#if 1 /* CP, 2013/03/14 18:02, processing <string> */
                switch ( p->match_node->type ) {
                case ICLI_VARIABLE_STRING:
                case ICLI_VARIABLE_RANGE_STRING:
                    if ( vtss_icli_str_len(w) <= 1 ) {
                        p->match_type = ICLI_MATCH_TYPE_INCOMPLETE;
                    } else {
                        ++( handle->runtime_data.partial_cnt );
                    }
                    break;
                default:
                    ++( handle->runtime_data.partial_cnt );
                    break;
                }
#else
                ++( handle->runtime_data.partial_cnt );
#endif
            }

        // fall through

        case ICLI_MATCH_TYPE_INCOMPLETE : /* partial match for variables and can not count into CR */
            _match_sort_put(handle, p);
            break;

        case ICLI_MATCH_TYPE_ERR : /* not match */
            vtss_icli_exec_parameter_free( p );
            break;

        default : /* error, invalid match type */
            T_E("invalid match type(%d)\n", match_type);
            // free all parameters
            _handle_para_free( handle );
            if ( last_node ) {
                vtss_icli_exec_parameter_free( last_node );
            }
            return ICLI_RC_ERROR;
        }// switch
    }// for

    // reset match_para
    handle->runtime_data.match_para = NULL;

    if ( *w ) {
        if ( handle->runtime_data.match_sort_list == NULL ) {
            if ( handle->runtime_data.exec_type == ICLI_EXEC_TYPE_TAB ) {
                ICLI_PUT_NEWLINE;
            }
            _error_display(handle, w, "Invalid");
            if ( last_node ) {
                vtss_icli_exec_parameter_free( last_node );
            }
            return ICLI_RC_ERR_MATCH;
        }

        /* check b_cr */
        if ( handle->runtime_data.b_cr == FALSE && handle->runtime_data.match_sort_list ) {
            switch (handle->runtime_data.total_match_cnt) {
            case 0:
                break;

            case 1: /* only one match */
                p = handle->runtime_data.match_sort_list;
                /* get <cr> */
                if ( p->match_type != ICLI_MATCH_TYPE_INCOMPLETE && p->match_node->execution && p->match_node->execution->cmd_cb ) {
                    _cr_get(handle, last_node, p, TRUE, TRUE);
                }
                break;

            default: /* multiple matches */
                // pre-process match_para
                if ( handle->runtime_data.exactly_cnt == 0 ) {
                    for ( p = handle->runtime_data.match_sort_list; p; ___NEXT(p) ) {
                        if ( vtss_icli_variable_data_string_type_get(p->value.type) ) {
                            p->match_type = ICLI_MATCH_TYPE_EXACTLY;
                            ++( handle->runtime_data.exactly_cnt );
                        }
                    }
                }

                if ( handle->runtime_data.exactly_cnt != 1 ) {
                    break;
                }
                for ( p = handle->runtime_data.match_sort_list; p; ___NEXT(p) ) {
                    if ( p->match_type == ICLI_MATCH_TYPE_EXACTLY && p->match_node->execution && p->match_node->execution->cmd_cb ) {
#if ICLI_RANDOM_MUST_NUMBER
                        _cr_get(handle, last_node, p, TRUE, TRUE);
#else
                        if ( _visible_chk(handle, p->match_node) ) {
                            handle->runtime_data.b_cr = TRUE;
                        }
#endif
                    }
                }
                break;
            }
        }
    }

    // put in match_sort_list
    if ( line_match_node ) {
        p = vtss_icli_exec_parameter_get();
        if ( p ) {
            p->match_node = line_match_node;
            _match_sort_put(handle, p);
        } else {
            T_E("memory insufficient\n");
        }
    }

    if ( last_node ) {
#if 1 /* put it back */
        for ( p = handle->runtime_data.cmd_var;
              p->next != NULL;
              p = p->next ) {
            ;
        }
        p->next = last_node;
#else
        vtss_icli_exec_parameter_free( last_node );
#endif
    }

    return ICLI_RC_OK;
}

static void _switchid_usid_update(
    IN icli_switch_port_range_t     *r
)
{
    r->usid        = icli_isid2usid( r->isid );
    r->begin_uport = icli_iport2uport( r->begin_iport );
    r->switch_id   = icli_usid2switchid( r->usid );
}

static void _mode_var_update(
    IN icli_parameter_t     *mode_var
)
{
    icli_stack_port_range_t     *r;
    u32                         i;

    while ( mode_var ) {
        switch ( mode_var->value.type ) {
        case ICLI_VARIABLE_PORT_ID:
        case ICLI_VARIABLE_PORT_TYPE_ID:
            _switchid_usid_update( &(mode_var->value.u.u_port_id) );
            break;

        case ICLI_VARIABLE_PORT_LIST:
        case ICLI_VARIABLE_PORT_TYPE_LIST:
            r = &( mode_var->value.u.u_port_list );
            for ( i = 0; i < r->cnt; ++i ) {
                _switchid_usid_update( &(r->switch_range[i]) );
            }
            break;

        default:
            break;
        }
        mode_var = mode_var->next;
    }
}

/*
    execute user command

    INPUT
        handle : session handle

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _exec_cmd(
    IN  icli_session_handle_t   *handle
)
{
    i32                     rc;
    icli_parameter_t        *p;
    icli_parameter_t        *q;

    icli_cmd_cb_t           *cmd_cb;
    u32                     session_id;
    icli_parameter_t        *mode_var;
    icli_parameter_t        *cmd_var;
    i32                     usr_opt;
    char                    *c;

#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */
    icli_parsing_node_t     *node;
    icli_parsing_node_t     *sibling;
    icli_parsing_node_t     *exec_link_node;
    BOOL                    b_exec_link;
    BOOL                    b_sibling;
#endif

#if 1 /* Bugzilla#11873 - Parse errors only seen in ICFG session */
    node_property_t         *node_prop;
#endif

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
    u32                     or_head;
    icli_parsing_node_t     *node_sibling;
#endif

#if 1 /* CP, 08/23/2013 14:25, Bugzilla#12424 - ICLI Enhancement - behavior of multiple concurrent sessions */
    BOOL                        b_concurrent;
    icli_cmd_mode_t             cmd_mode;
    char                        *str;
    icli_session_handle_t       *session_h;
    icli_err_display_mode_t     orig_err_display_mode;
#endif

    if ( handle == NULL ) {
        T_E("handle == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( handle->runtime_data.cmd_len == 0 ) {
        return ICLI_RC_ERR_EMPTY;
    }

    /* check if cmd is all SPACE */
    for ( c = handle->runtime_data.cmd; ICLI_IS_(SPACE, *c); ++c ) {
        ;
    }
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_EMPTY;
    }

    // execute command
    handle->runtime_data.cmd_walk = handle->runtime_data.cmd;
    handle->runtime_data.cmd_start = handle->runtime_data.cmd;

    T_D("_exec_cmd, walk:");
    rc = _cmd_walk( handle );
    switch ( rc ) {
    case ICLI_RC_ERR_EXCESSIVE:
    case ICLI_RC_OK:
        break;

    default:
        return rc;
    }

    // find the last one
    for ( q = NULL, p = handle->runtime_data.cmd_var; p->next; q = p, ___NEXT(p) ) {
        ;
    }

    exec_link_node = NULL;
    b_exec_link = FALSE;

    // check if executable
    if ( p->match_node->execution == NULL || p->match_node->execution->cmd_cb == NULL ) {
#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
        if ( p->match_node->child ) {
            or_head = 0;
            for ( sibling = p->match_node->child; sibling; ___SIBLING(sibling) ) {
                if ( _command_present_chk(handle, sibling) == FALSE ) {
                    continue;
                }
                if ( _present_chk(handle, sibling) ) {
                    continue;
                } else {
                    if ( sibling->or_head ) {
                        if ( sibling->or_head != or_head ) {
                            // not handle yet, very difficult, TBD
                        }
                    } else {
                        for ( node = sibling; node; ___CHILD(node) ) {
                            if ( _present_chk(handle, node) ) {
                                break;
                            } else {
                                if ( node != sibling ) {
                                    b_sibling = FALSE;
                                    for ( node_sibling = node->sibling; node_sibling; ___SIBLING(node_sibling) ) {
                                        if ( _present_chk(handle, node_sibling) ) {
                                            b_sibling = TRUE;
                                            break;
                                        }
                                    }
                                    if ( b_sibling ) {
                                        continue;
                                    }
                                }
                                if ( node->execution && node->execution->cmd_cb ) {
                                    p->match_node->execution = node->execution;
                                    b_exec_link = TRUE;
                                    exec_link_node = p->match_node;
                                    break;
                                }
                            }
                        }
                        if ( b_exec_link ) {
                            break;
                        }
                    }
                }
            }
        }
#else
        for ( node = p->match_node->child; node; ___CHILD(node) ) {
            if ( _present_chk(handle, node) ) {
                break;
            } else {
                b_sibling = FALSE;
                if ( node->sibling && (node->optional_begin == 0) ) {
                    for ( sibling = node->sibling; sibling; ___SIBLING(sibling) ) {
                        if ( _present_chk(handle, sibling) ) {
                            b_sibling = TRUE;
                            break;
                        }
                    }
                    if ( b_sibling ) {
                        break;
                    }
                }
                if ( node->execution && node->execution->cmd_cb ) {
                    p->match_node->execution = node->execution;
                    b_exec_link = TRUE;
                    exec_link_node = p->match_node;
                    break;
                }
            }
        }
#endif

        if ( b_exec_link == FALSE ) {
            _handle_para_free( handle );
            _error_display(handle, NULL, "% Incomplete command.\n");
            return ICLI_RC_ERR_INCOMPLETE;
        }
#else
        _handle_para_free( handle );
        _error_display(handle, NULL, "% Incomplete command.\n");
        return ICLI_RC_ERR_INCOMPLETE;
#endif
    }

#if ICLI_RANDOM_MUST_NUMBER
    if ( _session_random_must_get(handle) == FALSE ) {
        // free all parameters
        _handle_para_free( handle );
#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */
        if ( b_exec_link && exec_link_node ) {
            exec_link_node->execution = NULL;
        }
#endif
        return ICLI_RC_ERROR;
    }

    /* check if the number of random must is enough */
    handle->runtime_data.b_cr = FALSE;
    _cr_get(handle, q, p, FALSE, FALSE);

    if ( handle->runtime_data.b_cr == FALSE ) {
        _handle_para_free( handle );
        _error_display(handle, NULL, "% Incomplete command.\n");
#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */
        if ( b_exec_link && exec_link_node ) {
            exec_link_node->execution = NULL;
        }
#endif
        return ICLI_RC_ERR_INCOMPLETE;
    }
#endif

    /* get grep */
    handle->runtime_data.grep_var   = NULL;
    handle->runtime_data.grep_begin = 0;
    if ( p->value.type == ICLI_VARIABLE_GREP_STRING ) {
        for ( q = handle->runtime_data.cmd_var; q; ___NEXT(q) ) {
            if ( q->value.type == ICLI_VARIABLE_GREP ) {
                handle->runtime_data.grep_var = q;
                break;
            }
        }
    }

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
    // find wildcard and make port type list complete
    for ( cmd_var = handle->runtime_data.cmd_var; cmd_var; ___NEXT(cmd_var) ) {
        if ( cmd_var->value.type == ICLI_VARIABLE_PORT_TYPE                          &&
             cmd_var->value.u.u_port_type == ICLI_PORT_TYPE_ALL                      &&
             cmd_var->match_node->child->type == ICLI_VARIABLE_PORT_TYPE_LIST        &&
             ___LOOP_HEADER(cmd_var->match_node)                                     &&
             ( cmd_var->next == NULL || cmd_var->next->match_node->loop != cmd_var->match_node ) ) {
            /*
                user input port type wildcard only,
                so create new parameter for port list
            */
            q = vtss_icli_exec_parameter_get();
            if ( q == NULL ) {
                T_E("memory insufficient\n");
                // free all parameters
                _handle_para_free( handle );
                return ICLI_MATCH_TYPE_ERR;
            }

            // fill parameter
            q->word_id    = cmd_var->word_id + 1;
            q->match_node = cmd_var->match_node->child;
            q->match_type = ICLI_MATCH_TYPE_EXACTLY;
            q->b_in_loop  = 0;
            q->value.type = ICLI_VARIABLE_PORT_TYPE_LIST;
            if ( vtss_icli_exec_port_range_get(handle, q->match_node, &(q->value.u.u_port_type_list)) == FALSE ) {
                T_E("get port range\n");
            }

            // link to cmd_var
            q->next = cmd_var->next;
            cmd_var->next = q;

            // next cmd_var
            cmd_var = q;
        }
    }
#endif

    /* execute command */
    switch ( handle->runtime_data.exec_type ) {
    case ICLI_EXEC_TYPE_CMD:
#if 1 /* CP, 08/23/2013 14:25, Bugzilla#12424 - ICLI Enhancement - behavior of multiple concurrent sessions */
        b_concurrent = FALSE;
        session_id = 0;
        cmd_mode = 0;
        for ( node_prop = &(p->match_node->node_property); node_prop; ___NEXT(node_prop) ) {
            if ( node_prop->cmd_property->cmd_id == p->match_node->execution->cmd_id ) {
                if ( node_prop->cmd_property->mode_destroy != ICLI_CMD_MODE_MAX ) {
                    // this command is to destory command mode
                    // check if another session is in this command mode
                    for ( session_id = 0; session_id < ICLI_SESSION_CNT; ++session_id ) {
                        if ( session_id == handle->session_id ) {
                            // skip current session
                            continue;
                        }

                        if ( vtss_icli_session_alive(session_id) == FALSE ) {
                            // skip not-alive session
                            continue;
                        }

                        // get current mode
                        rc = vtss_icli_session_mode_get( session_id, &cmd_mode );
                        if ( rc != ICLI_RC_OK ) {
                            // skip, but it is a bug
                            T_E("session %u can not get current mode\n", session_id);
                            continue;
                        }

                        // check if it is the same command mode
                        if ( (u32)cmd_mode == node_prop->cmd_property->mode_destroy ) {
                            /* compare if the parameter is the same */
                            session_h = vtss_icli_session_handle_get( session_id );
                            if ( session_h == NULL ) {
                                T_E("session %u can not get handle\n", session_id);
                                continue;
                            }

                            // get other mode parameter
                            mode_var = session_h->runtime_data.mode_para[session_h->runtime_data.mode_level].cmd_var;
                            while ( mode_var ) {
                                if ( mode_var->value.type != ICLI_VARIABLE_KEYWORD ) {
                                    break;
                                }
                                ___NEXT( mode_var );
                            }
                            if ( mode_var == NULL ) {
                                T_E("session %u can not get parameter\n", session_id);
                                continue;
                            }

                            // get current parameter
                            cmd_var = handle->runtime_data.cmd_var;
                            while ( cmd_var ) {
                                if ( cmd_var->value.type != ICLI_VARIABLE_KEYWORD ) {
                                    break;
                                }
                                ___NEXT( cmd_var );
                            }
                            if ( cmd_var == NULL ) {
                                T_E("session %u can not get parameter\n", session_id);
                                continue;
                            }

                            // compare type
                            if ( mode_var->value.type != cmd_var->value.type ) {
                                T_E("different parameter types, %u and %u\n", mode_var->value.type, cmd_var->value.type);
                                continue;
                            }

                            // compare string because all modes are for string
                            // this may need to modify for new mode if its parameter is not string
                            if ( vtss_icli_str_cmp(mode_var->value.u.u_range_word, cmd_var->value.u.u_range_word) == 0 ) {
                                /* the same parameter, display error message and do nothing */
                                switch ( cmd_mode ) {
                                case ICLI_CMD_MODE_IPMC_PROFILE:
                                    str = "IPMC profile";
                                    break;

                                case ICLI_CMD_MODE_SNMPS_HOST:
                                    str = "SNMP host";
                                    break;

                                case ICLI_CMD_MODE_DHCP_POOL:
                                    str = "DHCP pool";
                                    break;

                                case ICLI_CMD_MODE_RFC2544_PROFILE:
                                    str = "RFC2544 profile";
                                    break;

                                default:
                                    str = "";
                                    break;
                                }
                                orig_err_display_mode = handle->runtime_data.err_display_mode;
                                handle->runtime_data.err_display_mode = ICLI_ERR_DISPLAY_MODE_PRINT;
                                vtss_icli_session_str_printf(handle, "%% Cannot delete %s; it's being configured in another session. Please try again later.\n", str);
                                handle->runtime_data.err_display_mode = orig_err_display_mode;

                                b_concurrent = TRUE;
                                break;
                            } // if ( vtss_icli_str_cmp() )
                        } // if ( (u32)cmd_mode == mode_destroy )
                    } // for session_id
                } // if ( mode_destroy != ICLI_CMD_MODE_MAX )
                break;
            } // if ( cmd_id )
        } // for ( node_prop )

        if ( b_concurrent ) {
            break;
        }
#endif

        // prepare command callback
        cmd_cb     = p->match_node->execution->cmd_cb;
        session_id = handle->session_id;
        mode_var   = handle->runtime_data.mode_para[handle->runtime_data.mode_level].cmd_var;
        cmd_var    = handle->runtime_data.cmd_var;
        usr_opt    = p->match_node->execution->usr_opt;

        /* update mode_var for switch_id, usid */
        _mode_var_update( mode_var );

        /* set b_in_exec_cb for GREP */
        handle->runtime_data.b_in_exec_cb = TRUE;

        // give semaphore
        icli_sema_give();

        // execute command callback
        (void)(*cmd_cb)(session_id, mode_var, cmd_var, usr_opt);

        // take semaphore
        icli_sema_take();

        /* clear b_in_exec_cb */
        handle->runtime_data.b_in_exec_cb = FALSE;
        break;

    case ICLI_EXEC_TYPE_PARSING:
        if ( handle->runtime_data.b_in_parsing ) {
#if 1 /* Bugzilla#11873 - Parse errors only seen in ICFG session */
            for ( node_prop = &(p->match_node->node_property); node_prop; ___NEXT(node_prop) ) {
                if ( node_prop->cmd_property->cmd_id == p->match_node->execution->cmd_id ) {
                    if ( node_prop->cmd_property->mode_goto != ICLI_CMD_MODE_MAX ) {
                        handle->runtime_data.current_parsing_mode = node_prop->cmd_property->mode_goto;
                    }
                    break;
                }
            }
#else
            if ( ICLI_CMD_IS_MODE(p->match_node->node_property.cmd_property->property) ) {
                handle->runtime_data.current_parsing_mode =
                    p->match_node->node_property.cmd_property->mode_goto;
            }
#endif
        }
        break;

    default:
        break;
    }

    // free all parameters
    _handle_para_free( handle );

#if 1 /* CP, 2013/01/09 13:00, Bugzilla#10708 - ignore not-present node */
    if ( b_exec_link && exec_link_node ) {
        exec_link_node->execution = NULL;
    }
#endif

    return ICLI_RC_OK;
}

/*
    execute TAB

    INPUT
        handle : session handle

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _exec_tab(
    IN  icli_session_handle_t   *handle
)
{
    i32     rc;

    // find possible nodes in match_sort_list
    rc = _next_node_find( handle );
    return rc;
}

/*
    execute ?, Question

    INPUT
        handle : session handle

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _exec_question(
    IN  icli_session_handle_t   *handle
)
{
    i32     rc;

    // find possible nodes in match_sort_list
    rc = _next_node_find( handle );
    return rc;
}

#if 1 /* CP, 2012/08/17 15:34, try global config mode when interface mode fails */
/*
    display message in error buffer

    INPUT
        handle : session handle

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
static void _exec_err_display(
    IN  icli_session_handle_t   *handle
)
{
    if ( handle->runtime_data.err_buf[0] != ICLI_EOS ) {
        handle->runtime_data.put_str = handle->runtime_data.err_buf;
        (void)vtss_icli_sutil_usr_str_put( handle );
        handle->runtime_data.put_str = NULL;
        handle->runtime_data.err_buf[0] = ICLI_EOS;
    }
}
#endif

#if 1 /* CP, 08/13/2013 13:00, Bugzilla#12423 - show full command syntax */
/*
    output formated string to a specific session
    TOTALLY the same with _session_printf() in vtss_icli_session.c

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
static void _exec_printf(
    IN  icli_session_handle_t   *handle,
    IN  const char              *format,
    IN  ...
)
{
    va_list     arglist;
    i32         b_in_exec_cb;

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

    b_in_exec_cb = handle->runtime_data.b_in_exec_cb;
    handle->runtime_data.b_in_exec_cb = TRUE;

    /*lint -e{530} ... 'arglist' is initialized by va_start() */
    va_start( arglist, format );
    (void)vtss_icli_session_va_str_put(handle->session_id, format, arglist);
    va_end( arglist );

    handle->runtime_data.b_in_exec_cb = b_in_exec_cb;
}

static void _full_cmd_buf_cpy(
    IN  icli_session_handle_t   *handle
)
{
    char    *c;
    char    *w;

    // reset
    c = handle->runtime_data.cmd_copy;
    memset( c, 0, sizeof(handle->runtime_data.cmd_copy) );

    // check length
    if ( handle->runtime_data.cmd_len == 0 ) {
        return;
    }

    /* check if all spaces */
    // skip space
    ICLI_SPACE_SKIP(w, handle->runtime_data.cmd);

    // EOS
    if ( ICLI_IS_(EOS, (*w)) ) {
        return;
    }

    // copy to cmd_copy
    (void)vtss_icli_str_cpy(c, handle->runtime_data.cmd);
    handle->runtime_data.cmd_walk = c;
    handle->runtime_data.cmd_start = c;
}

/*
    this does not care full list,
    full need to be checked before call this API
*/
static void _sort_insert(
    IN  char    *cmd
)
{
    i32     i;
    i32     j;
    i32     r;

    for ( i = 0; i < ICLI_CMD_CNT; ++i ) {
        if ( g_full_cmd_list[i] ) {
            r = vtss_icli_str_cmp( g_full_cmd_list[i], cmd );
            switch ( r ) {
            case 1:
                /* place to insert */
                for ( j = i + 1; j < ICLI_CMD_CNT; ++j ) {
                    if ( g_full_cmd_list[j] == NULL ) {
                        break;
                    }
                }
                if ( j == ICLI_CMD_CNT ) {
                    T_E("List already full.\n");
                    return;
                }
                // j >= 1 for lint
                for ( ; j > i && j >= 1; --j ) {
                    g_full_cmd_list[j] = g_full_cmd_list[j - 1];
                }
                g_full_cmd_list[i] = cmd;
                return;

            case 0:
                //T_W("Two commands are the same. but this should not happen. cmd = %s\n", cmd);
                return;

            case -1:
                break;

            default:
                T_E("Unknown error, %d.\n", r);
                return;
            }
        } else {
            g_full_cmd_list[i] = cmd;
            return;
        }
    }
}

/*
    display all command syntax for the user input

    INPUT
        handle : session handle

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _exec_full_cmd(
    IN  icli_session_handle_t   *handle
)
{
    icli_parameter_t        *p;
    icli_parsing_node_t     *node;
    node_property_t         *node_prop;
    i32                     rc;
    BOOL                    b_next;
    i32                     n;
    i32                     i;

    // get command into str buffer for further process
    _full_cmd_buf_cpy( handle );

    node   = NULL;
    b_next = FALSE;
    if ( vtss_icli_str_len(handle->runtime_data.cmd_walk) == 0 ) {
        // empty then get first possible node
        if ( handle->runtime_data.b_in_parsing ) {
            node = vtss_icli_parsing_tree_get( handle->runtime_data.current_parsing_mode );
        } else {
            node = vtss_icli_parsing_tree_get( handle->runtime_data.mode_para[handle->runtime_data.mode_level].mode );
        }
        b_next = TRUE;
    } else {
        // execute command
        rc = _cmd_walk( handle );
        switch ( rc ) {
        case ICLI_RC_ERR_EXCESSIVE:
        case ICLI_RC_OK:
            break;

        default:
            return rc;
        }

        // find the last node
        if ( handle->runtime_data.cmd_var ) {
            for ( p = handle->runtime_data.cmd_var; p->next; ___NEXT(p) ) {
                ;
            }
            node = p->match_node;
        }
    }

    if ( node == NULL ) {
        T_E("No any match, but why?\n");
        return ICLI_RC_ERR_MATCH;
    }

    memset( g_full_cmd_list, 0, sizeof(g_full_cmd_list) );
    n = 0;

    /* display all full commands*/
    do {
        for ( node_prop = &(node->node_property); node_prop; ___NEXT(node_prop) ) {
            if ( _node_prop_present(handle, node_prop)                                  &&
                 handle->runtime_data.privilege >= node_prop->cmd_property->privilege   &&
                 ICLI_CMD_IS_ENABLE(node_prop->cmd_property->property)                  &&
                 ICLI_CMD_IS_VISIBLE(node_prop->cmd_property->property)                 ) {
                if ( n < ICLI_CMD_CNT ) {
#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
                    _sort_insert( vtss_icli_register_var_cmd_get(node_prop->cmd_property->cmd_id) );
#else
                    _sort_insert( vtss_icli_register_cmd_str_get(node_prop->cmd_property->cmd_id) );
#endif
                    ++n;
                } else {
                    T_E("command count overflow\n");
                    return ICLI_RC_ERR_MEMORY;
                }
            }
        }

        // go to sibling
        ___SIBLING(node);

    } while ( b_next && node );

    for ( i = 0; i < n; ++i ) {
        _exec_printf( handle, "%s\n", g_full_cmd_list[i] );
    }

    return ICLI_RC_OK;
}
#endif

/*
==============================================================================

    Public Function

==============================================================================
*/
/*
    Initialization

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_exec_init(void)
{
    i32     i;

    g_para_cnt = 0;
    memset(g_parameter, 0, sizeof(g_parameter));
    for ( i = 0; i < (ICLI_PARAMETER_MEM_CNT - 1); ++i ) {
        g_parameter[i].next = &(g_parameter[i + 1]);
        ++g_para_cnt;
    }
    ++g_para_cnt;
    g_para_free = g_parameter;

#if 1 /* CP, 2012/10/11 16:09, performance enhancement: do not check same optional nodes */
    memset(g_match_optional_node, 0, sizeof(g_match_optional_node));
#endif

    return ICLI_RC_OK;
}

/*
    execute user command

    INPUT
        handle : session handle

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_exec(
    IN  icli_session_handle_t   *handle
)
{
    icli_rc_t                   rc = ICLI_RC_OK;
    char                        *c;
    i32                         original_mode_level;
#if 0
    i32                         level;
#endif
    icli_err_display_mode_t     original_err_display_mode;
    i32                         i;
    icli_cmd_mode_t             original_parsing_mode;
    i32                         original_exec_by_api;
    icli_parameter_t            *mode_cmd_var;

    if ( handle == NULL ) {
        T_E("handle == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    /* CP, 2012/08/17 09:38, accept space at beginning */
    ICLI_SPACE_SKIP( c, handle->runtime_data.cmd );

    // comment
    if ( ICLI_IS_COMMENT(*c) ) {
        return ICLI_RC_OK;
    }

    /* continuous TABs */
    if ( handle->runtime_data.tab_cnt ) {
        return ICLI_RC_OK;
    }

    /* CP, 2012/09/07 11:52, <port_type_list> */
    handle->runtime_data.b_in_loop = FALSE;

    /* reset last port type */
    handle->runtime_data.last_port_type = ICLI_PORT_TYPE_NONE;

    switch ( handle->runtime_data.exec_type ) {
    case ICLI_EXEC_TYPE_CMD:
        /* CP, 2012/08/17 15:34, try global config mode when interface mode fails */
        if ( handle->runtime_data.mode_para[handle->runtime_data.mode_level].mode <= ICLI_CMD_MODE_GLOBAL_CONFIG ) {
            /* in Exec and Global config mode */
            rc = _exec_cmd( handle );
        } else {
            /* in Interface mode */
            // store original error display mode
            original_err_display_mode = handle->runtime_data.err_display_mode;
            // buffer error message first if failed, but if drop then just drop
            if ( original_err_display_mode == ICLI_ERR_DISPLAY_MODE_PRINT ) {
                handle->runtime_data.err_display_mode = ICLI_ERR_DISPLAY_MODE_ERR_BUFFER;
            }
            // empty error buffer
            handle->runtime_data.err_buf[0] = 0;
            // execute command in interface mode
            rc = _exec_cmd( handle );

            switch ( rc ) {
            case ICLI_RC_OK:
                break;

            case ICLI_RC_ERR_AMBIGUOUS:
            case ICLI_RC_ERR_INCOMPLETE:
                // display previous error message in Interface mode
                _exec_err_display( handle );
                break;

            default:
                // store original mode level
                original_mode_level = handle->runtime_data.mode_level;
                // store mode parameter
                mode_cmd_var = handle->runtime_data.mode_para[original_mode_level].cmd_var;
                // go to Global config mode
                handle->runtime_data.mode_level = 1;
                // try Global config mode silently
                handle->runtime_data.err_display_mode = ICLI_ERR_DISPLAY_MODE_DROP;
                // store original exec_by_api
                original_exec_by_api = handle->runtime_data.b_exec_by_api;
                // trigger by ICLI egnine
                handle->runtime_data.b_exec_by_api = FALSE;
                // recover cmd
                c = handle->runtime_data.cmd;
                for ( i = 0; i < handle->runtime_data.cmd_len; ++i, ++c ) {
                    if ( ICLI_IS_(EOS, (*c)) ) {
                        *c = ICLI_SPACE;
                    }
                }
                // execute command in Global interface mode
                rc = _exec_cmd( handle );
                // restore original exec_by_api
                handle->runtime_data.b_exec_by_api = original_exec_by_api;
                // check exec result by global config mode
                if ( rc == ICLI_RC_OK ) {
                    // automatically go to Global config mode
                    // free Interface mode parameter
#if 1
                    vtss_icli_exec_para_list_free( &mode_cmd_var );
#else
                    for ( level = 2; level <= original_mode_level; ++level ) {
                        vtss_icli_exec_para_list_free( &(handle->runtime_data.mode_para[level].cmd_var) );
                    }
#endif
                } else {
                    // display previous error message in Interface mode
                    _exec_err_display( handle );
                    // restore mode level
                    handle->runtime_data.mode_level = original_mode_level;
                }
                break;
            }
            // restore original error display mode
            handle->runtime_data.err_display_mode = original_err_display_mode;
        }
        break;

    case ICLI_EXEC_TYPE_PARSING:
        if ( handle->runtime_data.b_in_parsing ) {
            if ( handle->runtime_data.current_parsing_mode > ICLI_CMD_MODE_GLOBAL_CONFIG ) {
                original_mode_level = 2;
            } else {
                original_mode_level = 1;
            }
        } else {
            original_mode_level = handle->runtime_data.mode_level;
        }
        if ( original_mode_level < 2 ) {
            /* in Exec and Global config mode */
            rc = _exec_cmd( handle );
        } else {
            /* in Interface mode */
            // store original error display mode
            original_err_display_mode = handle->runtime_data.err_display_mode;

            // buffer error message first if failed, but if drop then just drop
            if ( original_err_display_mode == ICLI_ERR_DISPLAY_MODE_PRINT ) {
                handle->runtime_data.err_display_mode = ICLI_ERR_DISPLAY_MODE_ERR_BUFFER;
            }

            // empty error buffer
            handle->runtime_data.err_buf[0] = 0;

            // execute command in interface mode
            rc = _exec_cmd( handle );

            switch ( rc ) {
            case ICLI_RC_OK:
                break;

            case ICLI_RC_ERR_AMBIGUOUS:
            case ICLI_RC_ERR_INCOMPLETE:
                // display previous error message in Interface mode
                _exec_err_display( handle );
                break;

            default:
                original_parsing_mode = ICLI_CMD_MODE_GLOBAL_CONFIG;
                if ( handle->runtime_data.b_in_parsing ) {
                    original_parsing_mode = handle->runtime_data.current_parsing_mode;
                    handle->runtime_data.current_parsing_mode = ICLI_CMD_MODE_GLOBAL_CONFIG;
                } else {
                    // store original mode level
                    original_mode_level = handle->runtime_data.mode_level;
                    // go to Global config mode
                    handle->runtime_data.mode_level = 1;
                }
                // try Global config mode silently
                handle->runtime_data.err_display_mode = ICLI_ERR_DISPLAY_MODE_DROP;
                // store original exec_by_api
                original_exec_by_api = handle->runtime_data.b_exec_by_api;
                // trigger by ICLI egnine
                handle->runtime_data.b_exec_by_api = FALSE;
                // recover cmd
                c = handle->runtime_data.cmd;
                for ( i = 0; i < handle->runtime_data.cmd_len; ++i, ++c ) {
                    if ( ICLI_IS_(EOS, (*c)) ) {
                        *c = ICLI_SPACE;
                    }
                }
                // execute command in Global interface mode
                rc = _exec_cmd( handle );
                // restore original exec_by_api
                handle->runtime_data.b_exec_by_api = original_exec_by_api;
                // check exec result by global config mode
                if ( rc == ICLI_RC_OK ) {
                    // automatically go to Global config mode
                    if ( handle->runtime_data.b_in_parsing ) {
                        // already in Global config mode
                    } else {
                        // parsing only, not exec, so does not need to change mode
                    }
                } else {
                    // display previous error message in Interface mode
                    _exec_err_display( handle );
                    // restore mode level
                    if ( handle->runtime_data.b_in_parsing ) {
                        handle->runtime_data.current_parsing_mode = original_parsing_mode;
                    } else {
                        handle->runtime_data.mode_level = original_mode_level;
                    }
                }
                break;
            }
            // restore original error display mode
            handle->runtime_data.err_display_mode = original_err_display_mode;
        }
        break;

    case ICLI_EXEC_TYPE_TAB:
        rc = _exec_tab( handle );
        break;

    case ICLI_EXEC_TYPE_QUESTION:
        rc = _exec_question( handle );
        break;

#if 1 /* CP, 08/13/2013 13:00, Bugzilla#12423 - show full command syntax */
    case ICLI_EXEC_TYPE_FULL_CMD:
        rc = _exec_full_cmd( handle );
        break;
#endif

    default:
        rc = ICLI_RC_ERR_PARAMETER;
        break;
    }

    /* CP, 2012/09/07 11:52, <port_type_list> */
    handle->runtime_data.b_in_loop = FALSE;

    /* CP, 2012/10/11 16:09, performance enhancement: do not check same optional nodes */
    memset( g_match_optional_node, 0, sizeof(g_match_optional_node) );

    return rc;
}

/*
    get memory for icli_parameter_t

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        not NULL : successful, icli_parameter_t *
        NULL     : failed

    COMMENT
        n/a
*/
icli_parameter_t *vtss_icli_exec_parameter_get(void)
{
    icli_parameter_t    *p;

    if ( g_para_free == NULL ) {
        T_E("g_para_free == NULL\n");
        return NULL;
    }

    if ( g_para_cnt == 0 ) {
        T_E("g_para_cnt == 0\n");
        return NULL;
    }

    p = g_para_free;
    g_para_free = g_para_free->next;
    --g_para_cnt;
    memset(p, 0, sizeof(icli_parameter_t));
    return p;
}

/*
    free memory for icli_parameter_t

    INPUT
        p - parameter for free

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
void vtss_icli_exec_parameter_free(
    IN icli_parameter_t    *p
)
{
    if ( p == NULL ) {
        return;
    }

    if ( g_para_free ) {
        p->next = g_para_free;
    } else {
        p->next = NULL;
    }
    g_para_free = p;
    ++g_para_cnt;
}

/*
    free parameter list

    INPUT
        *para_list - parameter list to be free

    OUTPUT
        *para_list - reset to NULL

    RETURN
        n/a

    COMMENT
        n/a
*/
void vtss_icli_exec_para_list_free(
    INOUT icli_parameter_t  **para_list
)
{
    icli_parameter_t    *p, *n;

    if (para_list == NULL ) {
        return;
    }

    for ( p = *para_list; p != NULL; p = n ) {
        n = p->next;
        vtss_icli_exec_parameter_free( p );
    }
    *para_list = NULL;
}

#if 1 /* CP, 2012/12/13 16:27, ICLI_ASK_PORT_RANGE */
/*
    check if the port type is present
    1. check runtime
    2. check system

    INPUT
        node      : matched node
        port_type : port type

    OUTPUT
        n/a

    RETURN
        TRUE  : yes, the port type is present
        FALSE : no

    COMMENT
        n/a
*/
BOOL vtss_icli_exec_port_type_present(
    IN icli_session_handle_t    *handle,
    IN icli_parsing_node_t      *node,
    IN icli_port_type_t         port_type
)
{
    icli_runtime_cb_t           *runtime_cb;
    BOOL                        b;

    if ( handle == NULL ) {
        T_E("handle == NULL\n");
        return FALSE;
    }

    if ( node == NULL ) {
        T_E("node == NULL\n");
        return FALSE;
    }

    if ( port_type >= ICLI_PORT_TYPE_MAX ) {
        T_E("invalid port type %d\n", port_type);
        return FALSE;
    }

    b = FALSE;
    runtime_cb = node->node_property.cmd_property->runtime_cb[node->word_id];
    if ( runtime_cb ) {
        b = vtss_icli_exec_runtime( handle, runtime_cb, ICLI_ASK_PORT_RANGE, &(handle->runtime_data.runtime) );
    }

    if ( b ) {

#if 1 /* CP, 09/30/2013 14:35, Bugzilla#12896 - '*' is not allowed if RUNTIME port range */
        if ( port_type == ICLI_PORT_TYPE_ALL ) {
            return FALSE;
        }
#endif

        /* backup system port range */
        vtss_icli_port_range_backup();

        /* set system port range for temp use */
        b = FALSE;
        if ( vtss_icli_port_range_set( &(handle->runtime_data.runtime.port_range) ) ) {
            b = vtss_icli_port_type_present( port_type );
        } else {
            T_E("Fail to set runtime port range\n");
        }

        /* restore system port range */
        vtss_icli_port_range_restore();
    } else {
        /* check system */
        b = vtss_icli_port_type_present( port_type );
    }

    return b;
}

/*
    get port range

    INPUT
        handle    : session handle
        node      : matched node of port type

    OUTPUT
        port_range : current port range

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_exec_port_range_get(
    IN  icli_session_handle_t    *handle,
    IN  icli_parsing_node_t      *node,
    OUT icli_stack_port_range_t  *port_range
)
{
    icli_runtime_cb_t           *runtime_cb;
    BOOL                        b;

    if ( handle == NULL ) {
        T_E("handle == NULL\n");
        return FALSE;
    }

    if ( node == NULL ) {
        T_E("node == NULL\n");
        return FALSE;
    }

    if ( port_range == NULL ) {
        T_E("port_range == NULL\n");
        return FALSE;
    }

    b = FALSE;
    runtime_cb = node->node_property.cmd_property->runtime_cb[node->word_id];
    if ( runtime_cb ) {
        b = vtss_icli_exec_runtime( handle, runtime_cb, ICLI_ASK_PORT_RANGE, &(handle->runtime_data.runtime) );
    }

    if ( b ) {
        /* backup system port range */
        vtss_icli_port_range_backup();

        /* set system port range for temp use */
        b = FALSE;
        if ( vtss_icli_port_range_set( &(handle->runtime_data.runtime.port_range) ) ) {
            b = vtss_icli_port_range_get( port_range );
        } else {
            T_E("Fail to set runtime port range\n");
        }

        /* restore system port range */
        vtss_icli_port_range_restore();
    }

    if ( b == FALSE ) {
        b = vtss_icli_port_range_get( port_range );
    }

    return b;
}
#endif

void vtss_icli_exec_cword_runtime_sort(
    INOUT icli_runtime_t    *runtime
)
{
    icli_runtime_t  irt;
    u32             i;
    u32             j;
    u32             k;
    i32             r;

    memset( &irt, 0, sizeof(icli_runtime_t) );

    for ( i = 0; i < ICLI_CWORD_MAX_CNT; ++i ) {
        if ( runtime->cword[i] == NULL ) {
            break;
        }
        if ( irt.cword[0] ) {
            for ( j = 0; j < ICLI_CWORD_MAX_CNT; ++j ) {
                if ( irt.cword[j] == NULL ) {
                    irt.cword[j] = runtime->cword[i];
                    break;
                }
                r = vtss_icli_str_cmp(irt.cword[j], runtime->cword[i]);
                if ( r > 0) {
                    for ( k = (ICLI_CWORD_MAX_CNT - 1); k >= (j + 1); --k ) {
                        irt.cword[k] = irt.cword[k - 1];
                    }
                    irt.cword[j] = runtime->cword[i];
                    break;
                } else if ( r == 0 ) {
                    break;
                }
            }
        } else {
            irt.cword[0] = runtime->cword[i];
        }
    }

    for ( i = 0; i < ICLI_CWORD_MAX_CNT; ++i ) {
        runtime->cword[i] = irt.cword[i];
    }
}

BOOL vtss_icli_exec_cword_runtime_get(
    IN  icli_session_handle_t   *handle,
    IN  icli_runtime_cb_t       *runtime_cb,
    OUT icli_runtime_t          *runtime
)
{
    BOOL    b;

    if ( runtime_cb == NULL ) {
        return FALSE;
    }

    b = vtss_icli_exec_runtime( handle, runtime_cb, ICLI_ASK_CWORD, runtime );
    if ( b ) {
        vtss_icli_exec_cword_runtime_sort( runtime );
    }
    return b;
}

/*
    get match count for the user input word in p by cword

    INPUT
        handle     - session handle
        word       - cword to match
        runtime_cb - runtime callback for cword

    OUTPUT
        runtime    - sorted result get by runtime_cb
        match_type - match type
        cword      - if match cnt is exactly 1, it is the matched one

    RETURN
        >= 0 - match count
        -1   - the <cword> is the same as general <word>, not need for match

    COMMENT
        n/a
*/
i32 vtss_icli_exec_cword_match_cnt(
    IN  icli_session_handle_t   *handle,
    IN  char                    *word,
    IN  icli_runtime_cb_t       *runtime_cb,
    OUT icli_runtime_t          *runtime,
    OUT icli_match_type_t       *match_type,
    OUT char                    **cword
)
{
    BOOL    b;
    i32     r;
    i32     exact_cnt;
    i32     sub_cnt;
    i32     i;

    b = vtss_icli_exec_cword_runtime_get(handle, runtime_cb, runtime);
    if ( b == FALSE ) {
        return -1;
    }

    if ( runtime->cword[0] == NULL ) {
        return -1;
    }

    exact_cnt = 0;
    sub_cnt   = 0;

    if ( cword ) {
        *cword = NULL;
    }

    for ( i = 0; i < ICLI_CWORD_MAX_CNT; ++i ) {
        if ( runtime->cword[i] == NULL ) {
            break;
        }

        r = vtss_icli_str_sub(word, runtime->cword[i], 0, NULL);
        switch ( r ) {
        case 1:
            ++sub_cnt;
            if ( sub_cnt == 1 && cword && *cword == NULL ) {
                *cword = runtime->cword[i];
            }
            break;
        case 0:
            ++exact_cnt;
            if ( exact_cnt == 1 && cword ) {
                *cword = runtime->cword[i];
            }
            break;
        case -1:
        default:
            break;
        }
    }

    if ( exact_cnt ) {
        if ( match_type ) {
            if ( exact_cnt == 1 ) {
                *match_type = ICLI_MATCH_TYPE_EXACTLY;
            } else {
                *match_type = ICLI_MATCH_TYPE_INCOMPLETE;
            }
        }
        return exact_cnt;
    }

    if ( match_type ) {
        if ( sub_cnt == 1 ) {
            *match_type = ICLI_MATCH_TYPE_PARTIAL;
        } else if ( sub_cnt > 1 ) {
            *match_type = ICLI_MATCH_TYPE_INCOMPLETE;
        } else {
            *match_type = ICLI_MATCH_TYPE_ERR;
        }
    }
    return sub_cnt;
}

/*
    parsing a command on a session

    INPUT
        handle : session handler
        conf   : privilege command configuration, mode and command string

    OUTPUT
        match_node : if return ICLI_RC_OK, then it contains one match node.
                     if return ICLI_RC_ERR_AMBIGUOUS, then it contains
                        multiple match nodes.
                     number of match_node is ICLI_PRIV_MATCH_NODE_CNT
    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_exec_priv_parsing(
    IN  icli_session_handle_t   *handle,
    IN  icli_priv_cmd_conf_t    *conf,
    OUT icli_parsing_node_t     **match_node
)
{
    icli_exec_type_t            orig_exec_type;
    i32                         orig_b_in_parsing;
    icli_parameter_t            *orig_cmd_var;
    icli_cmd_mode_t             orig_current_parsing_mode;
    icli_err_display_mode_t     orig_err_display_mode;
    i32                         orig_b_exec_by_api;
    char                        *orig_app_err_msg;

    i32                         rc;
    char                        *c;
    char                        *cmd;
    icli_parameter_t            *p;
    u32                         cmd_len;
    u32                         i;

    /* session is alive or not */
    if ( handle->runtime_data.alive == FALSE ) {
        T_E("session %d not alive\n", handle->session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    /* skip space at beginning */
    ICLI_SPACE_SKIP( cmd, conf->cmd );
    cmd_len = vtss_icli_str_len( cmd );
    if ( cmd_len == 0 ) {
        return ICLI_RC_ERR_EMPTY;
    }

    /* pack cmd */
    cmd_len += 16;
    c = icli_malloc( cmd_len );
    if ( c == NULL ) {
        T_E("memory insufficient\n");
        return ICLI_RC_ERR_MEMORY;
    }

    // reset all cmd to be 0, otherwise, it will introduce issue for <line>
    memset( c, 0, cmd_len );
    (void)vtss_icli_str_cpy( c, cmd );
    handle->runtime_data.cmd_walk  = c;
    handle->runtime_data.cmd_start = c;

    /* store original setting */
    orig_cmd_var              = handle->runtime_data.cmd_var;
    orig_exec_type            = handle->runtime_data.exec_type;
    orig_current_parsing_mode = handle->runtime_data.current_parsing_mode;
    orig_b_in_parsing         = handle->runtime_data.b_in_parsing;
    orig_err_display_mode     = handle->runtime_data.err_display_mode;
    orig_b_exec_by_api        = handle->runtime_data.b_exec_by_api;
    orig_app_err_msg          = handle->runtime_data.app_err_msg;

    /* set parsing */
    handle->runtime_data.cmd_var              = NULL;
    handle->runtime_data.exec_type            = ICLI_EXEC_TYPE_PARSING;
    handle->runtime_data.current_parsing_mode = conf->mode;
    handle->runtime_data.b_in_parsing         = TRUE;
    handle->runtime_data.err_display_mode     = ICLI_ERR_DISPLAY_MODE_DROP;
    handle->runtime_data.b_exec_by_api        = TRUE;
    handle->runtime_data.app_err_msg          = NULL;
    handle->runtime_data.b_keep_match_para    = TRUE;

    /* parsing command */
    rc = _cmd_walk( handle );

    /* get match node */
    switch ( rc ) {
    case ICLI_RC_ERR_EXCESSIVE:
        rc = ICLI_RC_OK;
    // fall through

    case ICLI_RC_OK:
        // find the last one
        for ( p = handle->runtime_data.cmd_var; p->next; ___NEXT(p) ) {
            ;
        }
        match_node[0] = p->match_node;
        break;

    case ICLI_RC_ERR_AMBIGUOUS:
        for ( p = handle->runtime_data.match_para, i = 0; p; ___NEXT(p), ++i ) {
            if ( i < ICLI_PRIV_MATCH_NODE_CNT ) {
                match_node[i] = p->match_node;
            } else {
                T_E("%% match parameters overflow\n");
                break;
            }
        }
        rc = ICLI_RC_OK;
        break;

    default:
        break;
    }

    /* clear all parameters */
    _handle_para_free( handle );

    /* restore original setting */
    handle->runtime_data.cmd_var              = orig_cmd_var;
    handle->runtime_data.exec_type            = orig_exec_type;
    handle->runtime_data.current_parsing_mode = orig_current_parsing_mode;
    handle->runtime_data.b_in_parsing         = orig_b_in_parsing;
    handle->runtime_data.err_display_mode     = orig_err_display_mode;
    handle->runtime_data.b_exec_by_api        = orig_b_exec_by_api;
    handle->runtime_data.app_err_msg          = orig_app_err_msg;
    handle->runtime_data.b_keep_match_para    = FALSE;

    // free memory
    icli_free( c );

    return rc;
}

/*
    get the number of free parameters

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        the number of free parameters

    COMMENT
        n/a
*/
i32 vtss_icli_exec_para_cnt_get(
    void
)
{
    return g_para_cnt;
}

/*
    run the runtime to get the result at run time

    INPUT
        handle
        runtime_cb
        ask

    OUTPUT
        runtime

    RETURN
        TRUE  - successful
        FALSE - failed

    COMMENT
        n/a
*/
BOOL vtss_icli_exec_runtime(
    IN  icli_session_handle_t   *handle,
    IN  icli_runtime_cb_t       *runtime_cb,
    IN  icli_runtime_ask_t      ask,
    OUT icli_runtime_t          *runtime
)
{
    BOOL    b;

    memset( runtime, 0, sizeof(icli_runtime_t) );

    // give semaphore before call out to avoid deadlock
    icli_sema_give();

    b = (*runtime_cb)(handle->session_id, ask, runtime);

    // take semaphore after call out
    icli_sema_take();

    return b;
}
