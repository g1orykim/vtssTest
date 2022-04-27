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
    > CP.Wang, 05/29/2013 11:39
        - create
        - this file is NOT re-entry

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

/*
==============================================================================

    Type Definition

==============================================================================
*/
typedef struct _icli_node_list {
    icli_parsing_node_t         *node;
    struct _icli_node_list      *next;
} icli_node_list_t;

typedef struct {
    char                        mode_prompt[ICLI_PROMPT_MAX_LEN + 1];
    icli_parsing_node_t         *tree;
} icli_mode_tree_t;

/*
==============================================================================

    Static Variable

==============================================================================
*/
//array index is the mode
static icli_mode_tree_t         g_mode_tree[ICLI_CMD_MODE_MAX];

/* the following 4 are for building parsing tree */
static icli_parsing_node_t      *g_cmd_node;
static u32                      g_number_of_nodes;
static icli_cmd_property_t      *g_cmd_property;
static icli_cmd_execution_t     *g_cmd_execution;
static u32                      g_cmd_mode;

static icli_parsing_node_t      *g_free_parsing_node = NULL;
static node_property_t          g_free_node_property[ICLI_NODE_PROPERTY_CNT];
static i32                      g_free_node_cnt = ICLI_NODE_PROPERTY_CNT;

#if 1 /* CP, 2012/10/18 17:20, performance enhancement: do not check same random nodes */
static char     g_random_optional_node[ICLI_CMD_WORD_CNT];
#endif

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
static char     g_or_head = 0;
#endif

/*
==============================================================================

    Static Function

==============================================================================
*/
static void _free_parsing_node_add(
    IN  icli_parsing_node_t     *free_node
)
{
    free_node->child = g_free_parsing_node;
    g_free_parsing_node = free_node;
}

#if 0 /* not used */
static icli_parsing_node_t *_free_parsing_node_get(void)
{
    icli_parsing_node_t *free_node;

    if ( g_free_parsing_node == NULL ) {
        return NULL;
    }

    free_node = g_free_parsing_node;
    g_free_parsing_node = g_free_parsing_node->child;
    return free_node;
}
#endif

static node_property_t *_free_node_property_get(
    IN i32  word_id
)
{
    node_property_t *new_property;

    //pool is empty, allocate new pool
    if ( g_free_node_cnt <= 0 ) {
        T_E("memory is used out\n");
        return NULL;
    }
    new_property = g_free_node_property + (--g_free_node_cnt);
    new_property->word_id      = word_id;
    new_property->cmd_property = g_cmd_property;
    new_property->next = NULL;
    return new_property;
}

#define _WORD_TYPE_GET(t) \
    if ( ICLI_IS_(t, (*c)) ) {\
        if ( ICLI_IS_(EOS,*(c+1)) ) {\
            *cmd_str = c + 1;\
        } else {\
            *(c+1) = ICLI_EOS;\
            *cmd_str = c + 2;\
        }\
        *w = c;\
        return ICLI_WORD_TYPE_##t;\
    }

/*
    get the first word in cmd_str

    input -
        cmd_str : command string
    output -
        cmd_str : point to the second word
        w       : the first word
    return -
        icli_word_type_t
    comment -
        n/a
*/
static i32  _first_word_get(
    INOUT   char                **cmd_str,
    OUT     char                **w
)
{
    char    *c;

    //skip space
    ICLI_SPACE_SKIP(c, (*cmd_str));

    //check type
    if ( ICLI_IS_KEYWORD(*c) ) {
        *w = c;
        for ( ; ICLI_IS_KEYWORD(*c); ++c ) {
            ;
        }
        if ( ICLI_IS_(SPACE, *(c)) ) {
            *c++ = ICLI_EOS;
        }
        *cmd_str = c;
        return ICLI_WORD_TYPE_KEYWORD;
    }

    if ( ICLI_IS_(VARIABLE_BEGIN, (*c)) ) {
        *w = c;
        for ( ++c; ICLI_IS_VARNAME(*c); ++c ) {
            ;
        }
        if ( ICLI_NOT_(VARIABLE_END, *(c)) ) {
            return ICLI_WORD_TYPE_INVALID;
        }
        if ( ICLI_IS_(EOS, *(c + 1)) ) {
            *cmd_str = c + 1;
        } else {
            *(c + 1) = ICLI_EOS;
            *cmd_str = c + 2;
        }
        return ICLI_WORD_TYPE_VARIABLE;
    }

    _WORD_TYPE_GET( MANDATORY_BEGIN );
    _WORD_TYPE_GET( MANDATORY_END );
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
    _WORD_TYPE_GET( LOOP_BEGIN );
    _WORD_TYPE_GET( LOOP_END );
#endif
    _WORD_TYPE_GET( OPTIONAL_BEGIN );
    _WORD_TYPE_GET( OPTIONAL_END );
    _WORD_TYPE_GET( OR );

    if ( ICLI_IS_(EOS, (*c)) ) {
        return ICLI_WORD_TYPE_EOS;
    }

    return ICLI_WORD_TYPE_INVALID;
}

/*
    do not check the syntax here exactly,
    because this has been done in icli_cmd_gen

    return -
        *    : successul, symbol or '|'
        NULL : failed
*/
static char *_find_next_symbol(
    IN char     *cmd,
    IN char     symbol
)
{
    char    *c;
    i32     stack_cnt = 0;

    for ( c = cmd; ICLI_NOT_(EOS, *c); ++c ) {
        //match symbol and stack is empty
        if ( *c == symbol && stack_cnt == 0 ) {
            return c;
        }

        switch (*c) {
        case ICLI_OR:
            //stack empty
            if ( stack_cnt == 0 ) {
                return c;
            }
            break;

        case ICLI_MANDATORY_BEGIN:
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
        case ICLI_LOOP_BEGIN:
#endif
        case ICLI_OPTIONAL_BEGIN:
            //push
            ++stack_cnt;
            break;

        case ICLI_MANDATORY_END:
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
        case ICLI_LOOP_END:
#endif
        case ICLI_OPTIONAL_END:
            //pop
            if ( stack_cnt == 0 ) {
                T_E("stack is empty\n");
                return NULL;
            }
            --stack_cnt;
            break;
        }

    }//for
    return NULL;
}

/*
    check if node is the immediate child of the parent

    return -
        TRUE  : yes, they are
        FALSE : no, they are not
*/
static BOOL _is_parent_child(
    IN icli_parsing_node_t  *parent,
    IN icli_parsing_node_t  *child
)
{
    icli_parsing_node_t  *n;

    if ( parent->child == NULL ) {
        return FALSE;
    }

    for ( n = parent->child; n; ___SIBLING(n) ) {
        if ( child == n ) {
            return TRUE;
        }
    }
    return FALSE;
}

/*
    declare for use by _parent_set_terminal()
*/
static BOOL _optional_set_terminal(
    IN icli_parsing_node_t  *node
);

/*
    set parents to be terminal nodes

    return -
        TRUE  : yes, they are
        FALSE : no, they are not
*/
static BOOL _parent_set_terminal(
    IN icli_parsing_node_t  *parent_node,
    IN icli_parsing_node_t  *node
)
{
    icli_parsing_node_t     *child_node;

    if ( parent_node->child == NULL ) {
        return TRUE;
    }

#if 1 /* CP, 2012/10/15 22:04, parent is always at the front, that is, with smaller word_id */
    if ( parent_node->word_id > node->word_id ) {
        return TRUE;
    }
#endif

#if 1 /* CP, 2012/10/11 15:27, performance enhance on parsing */
    if ( parent_node->execution == g_cmd_execution ) {
        return TRUE;
    }
#endif

    for ( child_node = parent_node->child; child_node != NULL; ___SIBLING(child_node) ) {
        if ( child_node == node ) {
            if ( parent_node->execution ) {
                if ( parent_node->execution != g_cmd_execution ) {
                    T_E("execution is not the same\n");
                    return FALSE;
                }
                continue;
            }
            parent_node->execution = g_cmd_execution;
            //parent node is also an optional end node
            if ( parent_node->optional_end ) {
                (void)_optional_set_terminal( parent_node );
            }
        } else {
            if ( _parent_set_terminal(child_node, node) == FALSE ) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

/*
    terminal node is an optional node
    find the parent nodes of the corresponding optional begin nodes
    these parent nodes also should be terminal nodes

    return -
        TRUE  : successful
        FALSE : failed because tree has something wrong
*/
static BOOL _optional_set_terminal(
    IN icli_parsing_node_t  *node
)
{
    i32                     i;
    icli_parsing_node_t     *parent_node,
                            *child_node;

    for ( i = 0; i < 32; ++i ) {

#if 1 /* CP, 2012/10/11 15:27, performance enhance on parsing */
        if ( ((node->optional_end) >> i) == 0 ) {
            return TRUE;
        }
#endif

        if ( ___BIT_MASK_GET(node->optional_end, i) == 0 ) {
            continue;
        }

        /* step 1, find the optional begin nodes */
        for ( parent_node = node; parent_node != NULL; ___PARENT(parent_node) ) {
            if ( ___BIT_MASK_GET(parent_node->optional_begin, i) ) {
                //find
                break;
            }
        }

        if ( parent_node == NULL ) {
            T_E("fail to find begin node\n");
            return FALSE;
        }

#if ICLI_RANDOM_MUST_NUMBER
        /* if the optional has must, then parent does not need termination */
        if ( parent_node->random_must_begin ) {
            continue;
        }
#endif

        /* step 2, find parents of the begin nodes */
        child_node = parent_node;
        parent_node = parent_node->parent;
        if ( parent_node == NULL ) {
#if 1 /* CP, 2012/10/16 14:17, subtree may have no parent */
            return TRUE;
#else
            T_E("parent of begin node is NULL\n");
            return FALSE;
#endif
        }

        /* check parent's siblings also */
        for ( ; parent_node != NULL; ___SIBLING(parent_node) ) {
            if ( _parent_set_terminal(parent_node, child_node) == FALSE ) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

#if ICLI_RANDOM_OPTIONAL
static BOOL _optional_set_random(
    IN icli_parsing_node_t  *optional_end_node,
    IN i32                  optional_level,
    IN icli_parsing_node_t  *random_optional_node,
    IN i32                  random_optional_level
);

static BOOL _random_optional_set(
    IN icli_parsing_node_t  *node,
    IN icli_parsing_node_t  *random_optional_node,
    IN i32                  random_optional_level
)
{
    i32     i;

    for ( i = 0; i < ICLI_RANDOM_OPTIONAL_DEPTH; ++i ) {
        // existed
        if ( node->random_optional[i] == random_optional_node ) {
            return TRUE;
        }
        // empty
        if ( node->random_optional[i] == NULL ) {
            node->random_optional[i]       = random_optional_node;
            node->random_optional_level[i] = random_optional_level;
            return TRUE;
        }
    }
    T_E("%s, random_optional is too deep\n", node->word);
    return FALSE;
}

/*
    set immediate parent's random_optional

    INPUT
        parent_node          : the parent node of the child node, but may not immediate
        child_node           : the child node
        random_optional_node : the first optional node in continuous optional block
                               and random_optional should link to the first optional node

    RETURN
        TRUE  : successful
        FALSE : failed
*/
static BOOL _parent_set_random(
    IN icli_parsing_node_t  *parent_node,
    IN icli_parsing_node_t  *child_node,
    IN i32                  optional_level,
    IN icli_parsing_node_t  *random_optional_node,
    IN i32                  random_optional_level
)
{
    BOOL    b = FALSE;

    if ( parent_node == NULL ) {
        return TRUE;
    }

#if 1 /* CP, 2012/10/18 17:20, performance enhancement: do not check same random nodes */
    /* already visited */
    if ( g_random_optional_node[parent_node->word_id] ) {
        return TRUE;
    } else {
        g_random_optional_node[parent_node->word_id] = 1;
    }
#endif

    if ( _is_parent_child(parent_node, child_node) ) {
        if ( _random_optional_set(parent_node, random_optional_node, random_optional_level) == FALSE ) {
            return FALSE;
        }
        b = _optional_set_random(parent_node, optional_level, random_optional_node, random_optional_level);
    } else {
        // find child
        b = _parent_set_random(parent_node->child, child_node, optional_level, random_optional_node, random_optional_level);
    }
    if ( b == FALSE ) {
        return FALSE;
    }

    // find sibling
    b = _parent_set_random(parent_node->sibling, child_node, optional_level, random_optional_node, random_optional_level);

    if ( b == FALSE ) {
        return FALSE;
    }

    return TRUE;
}

/*
    set parent's random_optional and the parent is in the deeper optional level

    INPUT
        optional_end_node    : the optional end node with random optional
        random_optional_node : the first optional node in continuous optional block
                               and random_optional should link to the first optional node
        optional_level       : the next deeper optional level to find its parent

    RETURN
        TRUE  : successful
        FALSE : failed
*/
static BOOL _optional_set_random(
    IN icli_parsing_node_t  *optional_end_node,
    IN i32                  optional_level,
    IN icli_parsing_node_t  *random_optional_node,
    IN i32                  random_optional_level
)
{
    BOOL                    b;
    icli_parsing_node_t     *optional_begin_node;

    if ( ___BIT_MASK_GET(optional_end_node->optional_end, optional_level) == 0 ) {
        return TRUE;
    }

    /* step 1, find the optional begin nodes */
    for ( optional_begin_node = optional_end_node; optional_begin_node != NULL; ___PARENT(optional_begin_node) ) {
        if ( ___BIT_MASK_GET(optional_begin_node->optional_begin, optional_level) ) {
            //find
            break;
        }
    }

    if ( optional_begin_node == NULL ) {
        T_E("fail to find begin node\n");
        return FALSE;
    }

#if ICLI_RANDOM_MUST_NUMBER
    /* if the optional has must, then parent does not need random optional */
    if ( optional_begin_node->random_must_begin ) {
        return TRUE;
    }
#endif

    // optional_begin_node->parent may not be the immediate parent of optional_begin_node.
    b = _parent_set_random(optional_begin_node->parent, optional_begin_node, optional_level, random_optional_node, random_optional_level);
    return b;
}
#endif

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
static void _or_head_set(
    IN icli_parsing_node_t  *tree
)
{
    icli_parsing_node_t     *t;

    if ( tree->sibling == NULL ) {
        return;
    }

    ++g_or_head;
    for ( t = tree; t; ___SIBLING(t) ) {
        t->or_head = g_or_head;
    }
}
#endif

#define ___APPEND_TREE() \
    if ( b_prev ) {\
        i32     parent_cnt;\
        for ( i = prev_begin_id; i<=prev_end_id; ++i ) {\
            if ( g_cmd_node[i].child == NULL ) {\
                g_cmd_node[i].child = m_tree;\
                ++( m_tree->parent_cnt );\
            }\
        }\
        parent_cnt = m_tree->parent_cnt;\
        for ( ; m_tree != NULL; ___SIBLING(m_tree) ) {\
            m_tree->parent = &g_cmd_node[prev_begin_id];\
            m_tree->parent_cnt = parent_cnt;\
        }\
    } else {\
        if ( tree_tail ) {\
            tree_tail->child = m_tree;\
            for ( ; m_tree != NULL; ___SIBLING(m_tree) ) {\
                m_tree->parent = tree_tail;\
                ++( m_tree->parent_cnt );\
            }\
        } else {\
            tree = m_tree;\
        }\
    }

/*
    build tree

    INPUT
        cmd     : command string
        word_id : starting word id
        b_execution : assign execution to the end nodes to be terminal nodes

    OUTPUT
        n/a

    RETURN
        *    : successful
        NULL : failed

    COMMENT
        n/a
*/
static icli_parsing_node_t *_parsing_tree(
    IN  char    *cmd,
    IN  i32     word_id,
    IN  i32     optional_level,
    IN  BOOL    b_execution
)
{
    i32                     i,
                            w_type,
#if ICLI_RANDOM_OPTIONAL
                            prev_type = ICLI_WORD_TYPE_KEYWORD,
                            first_optional_id = -1,
#endif
                            prev_begin_id = 0,
                            prev_end_id = 0,
                            current_begin_id,
                            current_end_id,
                            level;
    char                    *w,
                            *sub_cmd,
                            symbol;
    icli_parsing_node_t     *tree = NULL;
    icli_parsing_node_t     *tree_tail = NULL;
    icli_parsing_node_t     *new_tree = NULL;
    icli_parsing_node_t     *m_tree;
    icli_parsing_node_t     *sibling_tail;
    icli_parsing_node_t     *node;
    BOOL                    b_prev = FALSE;
    i32                     orig_word_id = word_id;
#if ICLI_RANDOM_MUST_NUMBER
    icli_parsing_node_t     *must_head;
    u32                     must_number;
    u32                     must_level;
    u32                     n;
#endif

    for ( w_type = _first_word_get(&cmd, &w);
          w_type != ICLI_WORD_TYPE_EOS;
          w_type = _first_word_get(&cmd, &w) ) {

        /* get first word */
        switch ( w_type ) {
        case ICLI_WORD_TYPE_KEYWORD:
        case ICLI_WORD_TYPE_VARIABLE:
            /* get word */
            g_cmd_node[word_id].word = w;

            /* append tree */
            m_tree = &g_cmd_node[word_id];
            ___APPEND_TREE();

            /* update data */
            b_prev = FALSE;
            tree_tail = &g_cmd_node[word_id];
            ++word_id;
            break;

        case ICLI_WORD_TYPE_MANDATORY_BEGIN:
        case ICLI_WORD_TYPE_OPTIONAL_BEGIN:
            current_begin_id = word_id;
            m_tree = NULL;
            sibling_tail = NULL;
            if ( w_type == ICLI_WORD_TYPE_MANDATORY_BEGIN ) {
                symbol = ICLI_MANDATORY_END;
                level = optional_level;
            } else {
                symbol = ICLI_OPTIONAL_END;
                level = optional_level + 1;
            }

            for (;;) {
                /* get sub-command */
                sub_cmd = cmd;
                cmd = _find_next_symbol(cmd, symbol);
                if ( cmd == NULL ) {
                    T_E("fail to find '%c'\n", symbol);
                    return NULL;
                }
                *(cmd - 1) = ICLI_EOS;

                /* get count first because sub_cmd will be altered after _parsing_tree() */
                i = vtss_icli_word_count(sub_cmd);

                /* build tree for sub-command */
                new_tree = _parsing_tree( sub_cmd, word_id, level, FALSE);
                if ( new_tree == NULL ) {
                    T_E("fail to _parsing_tree(), command = %s\n", sub_cmd);
                    return NULL;
                }

                // update word_id
                word_id += i;

                // append to tree
                if ( sibling_tail ) {
                    // add to sibling
                    sibling_tail->sibling = new_tree;
                } else {
                    // new
                    m_tree = new_tree;
                }

                // get tail
                for ( ; new_tree->sibling; ___SIBLING(new_tree) ) {
                    ;
                }
                sibling_tail = new_tree;

                /* next sub-command */
                if ( *cmd == symbol ) {
                    break; //while
                }
                ++cmd;
            }

            // for lint
            if ( m_tree == NULL ) {
                T_E("empty m_tree\n");
                return NULL;
            }

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
            _or_head_set( m_tree );
#endif

            /* update current_end_id */
            current_end_id = word_id - 1;

            /* optional bit mask */
            if ( w_type == ICLI_WORD_TYPE_OPTIONAL_BEGIN ) {
                // begin bit mask
                for (new_tree = m_tree; new_tree; ___SIBLING(new_tree) ) {

                    /*
                        BUG:
                        if 0, then there is a bug
                        the case is "a [b] [[c] d]"
                        then > a d ?
                        it will shows b and c.
                        but c should not be included.

                        on the other hand, [[[w]]] should also be accepted.
                    */

#if ICLI_RANDOM_OPTIONAL
                    // not allow multiple optional begin
                    if ( new_tree->optional_begin ) {
                        T_E("Multiple optional begin on \"%s\", "
                            "*** Please refer to AN1047, Appendix B.2\n",
                            new_tree->word);
                        return NULL;
                    }
#endif
                    ___BIT_MASK_SET(new_tree->optional_begin, optional_level);
                }

                // end bit mask
                for ( i = current_begin_id; i <= current_end_id; ++i ) {
                    if ( g_cmd_node[i].child == NULL ) {
                        // set optional end bit
                        ___BIT_MASK_SET(g_cmd_node[i].optional_end, optional_level);
#if ICLI_RANDOM_OPTIONAL
                        if ( prev_type == ICLI_WORD_TYPE_OPTIONAL_BEGIN ) {
                            // set random optional
                            if ( _random_optional_set(&g_cmd_node[i], &g_cmd_node[first_optional_id], optional_level) == FALSE ) {
                                T_E("_random_optional_set(%d, %d, %d)", i, first_optional_id, optional_level);
                                return NULL;
                            }

                            /*
                                if the optional end node is also in the deeper optional,
                                then the parent of the deeper optional also needs random_optional.
                                "a [b] [ c [d] ]", c->random_optional = b also, but not only
                                                   d->random_optional = b
                            */
                            // set from deepest
                            for ( level = 31; level > optional_level; --level ) {

                                if ( ___BIT_MASK_GET(g_cmd_node[i].optional_end, level) == 0 ) {
                                    continue;
                                }

#if 1 /* CP, 2012/10/18 17:20, performance enhancement: do not check same random nodes */
                                memset(g_random_optional_node, 0, ICLI_CMD_WORD_CNT);
#endif

                                if ( _optional_set_random( &g_cmd_node[i],
                                                           level,
                                                           &g_cmd_node[first_optional_id],
                                                           optional_level ) == FALSE ) {
                                    T_E("_optional_set_random(%d, %d, %d, %d)", i, level, first_optional_id, optional_level);
                                    return NULL;
                                }
                            }
                        }
#endif
                    }
                }

#if ICLI_RANDOM_OPTIONAL
                if ( prev_type != ICLI_WORD_TYPE_OPTIONAL_BEGIN ) {
                    first_optional_id = current_begin_id;
                }
#endif
            } // ICLI_WORD_TYPE_OPTIONAL_BEGIN

            /* append tree */
            ___APPEND_TREE();

            /* update data */
            b_prev = TRUE;
            prev_begin_id = current_begin_id;
            prev_end_id = current_end_id;
            cmd = cmd + 1;

#if ICLI_RANDOM_MUST_NUMBER
            /* get random optional must number */
            if ( symbol == ICLI_MANDATORY_END ) {
                if ( ICLI_IS_RANDOM_MUST(cmd - 1) ) {
                    if ( vtss_icli_parsing_random_head(&g_cmd_node[current_begin_id], &g_cmd_node[current_end_id], &must_level) ) {
                        /* get must number */
                        must_number = *(cmd + 1) - '0';

                        /* get number of random optional and compare with must number */
                        n = 0;
                        for ( node = &(g_cmd_node[current_begin_id]); node; ___CHILD(node) ) {
                            if ( ___BIT_MASK_GET(node->optional_begin, must_level) ) {
                                ++n;
                            }
                        }
                        if ( must_number > n ) {
                            must_number = n;
                        }

                        /* get optional level */
                        must_head = &(g_cmd_node[current_begin_id]);

                        /* mark begin node's siblings to be 'must begin' */
                        for ( node = must_head; node; ___SIBLING(node) ) {
                            ++( node->random_must_begin );
                        }

                        /* mark each end node to be 'must end' */
                        for ( i = current_begin_id; i <= current_end_id; ++i ) {
                            if ( ___BIT_MASK_GET(g_cmd_node[i].optional_begin, must_level) ) {
                                g_cmd_node[i].random_must_head   = must_head;
                                g_cmd_node[i].random_must_number = must_number;
                                g_cmd_node[i].random_must_level  = must_level;
                            } else if ( g_cmd_node[i].child ) {
                                ++( g_cmd_node[i].random_must_middle );
                            } else {
                                ++( g_cmd_node[i].random_must_end );
                            }
                        }

                        /* skip "}*n" */
                        cmd += 2;
                    } else {
                        T_E("no random optional for must number\n");
                        return NULL;
                    }
                }
            }
#endif
            break;

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
        case ICLI_WORD_TYPE_LOOP_BEGIN:
            current_begin_id = word_id;

            symbol = ICLI_LOOP_END;
            level  = optional_level;

            /* get sub-command */
            sub_cmd = cmd;
            cmd = _find_next_symbol(cmd, symbol);

            /* next sub-command */
            if ( cmd == NULL ) {
                T_E("fail to find '%c'\n", symbol);
                return NULL;
            }

            if ( *cmd != symbol ) {
                T_E("fail to find '%c'\n", symbol);
                return NULL;
            }

            /* next sub-command */
            *(cmd - 1) = ICLI_EOS;

            /* get count first because sub_cmd will be altered after _parsing_tree() */
            i = vtss_icli_word_count(sub_cmd);

            /* build tree for sub-command */
            m_tree = _parsing_tree( sub_cmd, word_id, level, FALSE);
            if ( m_tree == NULL ) {
                T_E("fail to _parsing_tree(), command = %s\n", sub_cmd);
                return NULL;
            }

            // loop
            for (node = m_tree; node->child != NULL; node = node->child) {
                ;
            }
            node->loop = m_tree;

            //update word_id
            word_id += i;

            /* update current_end_id */
            current_end_id = word_id - 1;

            /* append tree */
            ___APPEND_TREE();

            /* update data */
            b_prev = TRUE;
            prev_begin_id = current_begin_id;
            prev_end_id = current_end_id;
            cmd = cmd + 1;
            break;
#endif

        default:
            T_E("fail to get the first word, %s, type = %d\n", w, w_type);
            return NULL;
        }

#if ICLI_RANDOM_OPTIONAL
        prev_type = w_type;
#endif

    }//for

    //find end nodes and set them to be terminal nodes
    if ( b_execution ) {
        for ( i = (word_id - 1); i >= orig_word_id; --i ) {
            //end nodes
            if ( g_cmd_node[i].child == NULL ) {
                //set terminal node
                g_cmd_node[i].execution = g_cmd_execution;

                /*
                    terminal node is an optional end node
                    find the parent nodes of the corresponding optional begin nodes
                    these parent nodes also should be terminal nodes
                 */
                if ( g_cmd_node[i].optional_end ) {
                    if ( _optional_set_terminal( &g_cmd_node[i] ) == FALSE ) {
                        return NULL;
                    }
                }
            }
        }
    }
    return tree;
}

/*
    build parsing tree

    input -
        cmd : command string
    output -
        n/a
    return -
        icli_rc_t
    comment -
        n/a
*/
static i32  _tree_build_and_add(
    IN  char    *cmd
)
{
    icli_parsing_node_t     *new_tree;

    // new tree
    new_tree = _parsing_tree(cmd, 0, 0, TRUE);
    if ( new_tree == NULL ) {
        T_E("fail to build new tree\n");
        return ICLI_RC_ERROR;
    }

    // insert into mode tree
    new_tree->sibling = g_mode_tree[g_cmd_mode].tree;
    g_mode_tree[g_cmd_mode].tree = new_tree;

    // successful
    return ICLI_RC_OK;
}

#define ___APPEND_NODE_PROPERTY() \
    new_node_property = _free_node_property_get( word_id );\
    if ( new_node_property == NULL ) {\
        return ICLI_RC_ERR_MEMORY;\
    }\
    new_node_property->next = tree->node_property.next;\
    tree->node_property.next = new_node_property;

//TRUE: same, FALSE: different
static BOOL _node_is_same(
    IN  icli_parsing_node_t     *n1,
    IN  icli_parsing_node_t     *n2
)
{
    /*
        id not only means the sequence of word
        but also used as var_id in callback
        so the same node should be with the same id
        otherwise, it will be confused about the id in auto-generated C file
        for callback(icli_cmd_cb_t)
    */
    // id
    if ( n1->word_id != n2->word_id ) {
        return FALSE;
    }
    // word
    if ( vtss_icli_str_cmp(n1->word, n2->word) != 0 ) {
        return FALSE;
    }
    // optional_begin
    if ( n1->optional_begin != n2->optional_begin ) {
        return FALSE;
    }
    // optional_end
    if ( n1->optional_end != n2->optional_end ) {
        return FALSE;
    }
    return TRUE;
}

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
#define ___PORT_TYPE_LIST_CHECK( t ) \
    if ( t->type != ICLI_VARIABLE_PORT_TYPE ) { \
        return ICLI_RC_ERROR; \
    } \
    if ( t->child == NULL || t->child->type != ICLI_VARIABLE_PORT_TYPE_LIST ) { \
        return ICLI_RC_ERROR; \
    } \
    if ( t->child->sibling ) { \
        return ICLI_RC_ERROR; \
    }

static i32 _port_type_list_merge(
    IN  icli_parsing_node_t     *old_tree,
    IN  icli_parsing_node_t     *new_tree
)
{
    icli_parsing_node_t     *tree;
    icli_parsing_node_t     *child;
    u32                     word_id;
    node_property_t         *new_node_property;

    /* check if this is for <port_type_list> without any other sibling */
    ___PORT_TYPE_LIST_CHECK( old_tree );
    ___PORT_TYPE_LIST_CHECK( new_tree );

    /* the optional should be the totally same */
    if ( old_tree->child->optional_begin == 0 ||
         old_tree->child->optional_end   == 0 ||
         old_tree->child->optional_begin != new_tree->child->optional_begin ||
         old_tree->child->optional_end   != new_tree->child->optional_end  ) {
        return ICLI_RC_ERROR;
    }

    /* merge children */
    for ( child = new_tree->child->child; child; ___SIBLING(child) ) {
        child->parent = old_tree;
    }

    if ( old_tree->child->child ) {
        new_tree->child->child->sibling = old_tree->child->child->sibling;
        old_tree->child->child->sibling = new_tree->child->child;
    } else {
        old_tree->child->child = new_tree->child->child;
        old_tree->sibling->child = new_tree->child->child;
    }

    /* <port_type> collect un-used node and append command property */
    word_id = new_tree->word_id;
    _free_parsing_node_add( &(g_cmd_node[word_id]) );

    tree = old_tree;
    ___APPEND_NODE_PROPERTY();

    /* <port_type_list> collect un-used node and append command property */
    ++word_id;
    _free_parsing_node_add( &(g_cmd_node[word_id]) );

    tree = old_tree->child;
    ___APPEND_NODE_PROPERTY();

    // the new tree may be optional node with execution
    // so the parent nodes should be with execution also
    for ( ++word_id; word_id < g_number_of_nodes; ++word_id) {
        if ( g_cmd_node[word_id].execution && g_cmd_node[word_id].optional_end ) {
            if ( _optional_set_terminal( &g_cmd_node[word_id] ) == FALSE ) {
                return ICLI_RC_ERROR;
            }
        }
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
    initialization

    input -
        n/a
    output -
        n/a
    return -
        icli_rc_t
    comment -
        n/a
*/
i32 vtss_icli_parsing_init(
    void
)
{
    memset( g_mode_tree, 0, sizeof(g_mode_tree) );

    g_free_parsing_node = NULL;
    g_free_node_cnt = ICLI_NODE_PROPERTY_CNT;
    memset(g_free_node_property, 0, sizeof(g_free_node_property));

    return ICLI_RC_OK;
}

/*
    build parsing tree

    input -
        cmd_register : command data, string with word ID
    output -
        n/a
    return -
        icli_rc_t
    comment -
        n/a
*/
i32 vtss_icli_parsing_build(
    IN  icli_cmd_register_t     *cmd_register
)
{
    u32                     word_id;
    char                    *c,
                            *w;
    icli_parsing_node_t     *tree,
                            *new_tree,
                            *pre_tree,
                            *t,
                            *n;
    node_property_t         *new_node_property;

#if 0 /* CP, 2012/12/06 11:01, check loop overlap */
    BOOL                    b_loop = FALSE;
    u32                     loop_overlap_cnt = 0;
#endif

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
    BOOL                    b_loop = FALSE;
#endif

    // update global
    g_cmd_node        = cmd_register->node;
    g_number_of_nodes = cmd_register->number_of_nodes;
    g_cmd_property    = cmd_register->cmd_property;
    g_cmd_execution   = cmd_register->cmd_execution;
    g_cmd_mode        = cmd_register->cmd_mode;

    // get corresponding mode tree
    tree = g_mode_tree[g_cmd_mode].tree;
    if ( tree == NULL ) {
        return _tree_build_and_add( cmd_register->cmd );
    }

    /*
        tree is not null, find overlap
        this check is straight-forward only, that is,
        if mandatory or option, then stop and go to build tree.
        The overlap will also be checked after building and before add.
    */

    // init word id
    word_id = 0;

    // get first word
    c = cmd_register->cmd;
    if ( _first_word_get(&c, &w) != ICLI_WORD_TYPE_KEYWORD ) {
        T_E("The first word must be a keyword\n");
        return ICLI_RC_ERROR;
    }

    // assign word
    g_cmd_node[word_id].word = w;

    // find overlap
    for ( ; tree != NULL; ___SIBLING(tree) ) {
        if ( _node_is_same(&g_cmd_node[word_id], tree) ) {
            break;
        }
    }

    // no overlap
    if ( tree == NULL ) {
        *(c - 1) = ICLI_SPACE;
        return _tree_build_and_add( w );
    }

    /* overlap */
    // collect un-used node
    _free_parsing_node_add( &(g_cmd_node[word_id]) );

    // append command property
    ___APPEND_NODE_PROPERTY();

    /*
        allow plain merge only, that is, without any mandatory or optional
        for example,
            "a b c" and "a b [ d | e ]" are allowed.
            "a b c" and "a b [ c | d ]" are NOT allowed.
    */
    for ( pre_tree = tree, ___CHILD(tree); tree != NULL; pre_tree = tree, ___CHILD(tree) ) {
        /*
            check if can merge, if parent_cnt then can NOT merge
            TODO : this need to seperate the parents if merge
        */
        if ( tree->parent_cnt > 1 ) {
            T_E("command overlap, but can not merge\n");
            return ICLI_RC_ERROR;
        }

#if 0 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
_icli_parsing_build_check_next_word:
#endif
        // get first word
        switch ( _first_word_get(&c, &w) ) {
        case ICLI_WORD_TYPE_KEYWORD:
        case ICLI_WORD_TYPE_VARIABLE:
            // update word id
            ++word_id;
            break;

#if 0 /* CP, 2012/12/06 11:01, check loop overlap */
        case ICLI_WORD_TYPE_LOOP_BEGIN:
            b_loop = TRUE;
            loop_overlap_cnt = 0;
            goto _icli_parsing_build_check_next_word;

        case ICLI_WORD_TYPE_LOOP_END:
            b_loop = FALSE;
            loop_overlap_cnt = 0;
            goto _icli_parsing_build_check_next_word;
#endif

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
        case ICLI_WORD_TYPE_LOOP_BEGIN:
            b_loop = TRUE;

            // fall through
#endif

        case ICLI_WORD_TYPE_MANDATORY_BEGIN:
        case ICLI_WORD_TYPE_OPTIONAL_BEGIN:
            // get new tree
            *(c - 1) = ICLI_SPACE;
            new_tree = _parsing_tree(w, word_id + 1, 0, TRUE);
            if ( new_tree == NULL ) {
                T_E("fail to build new tree\n");
                return ICLI_RC_ERROR;
            }

            // check if duplicate
            for ( n = new_tree; n; ___SIBLING(n) ) {
                for ( t = tree; t; ___SIBLING(t) ) {
                    if ( _node_is_same(n, t) ) {
#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
                        if ( b_loop && _port_type_list_merge(t, n) == ICLI_RC_OK ) {
                            return ICLI_RC_OK;
                        }
#endif
                        T_E("Duplicate word \"%s\". "
                            "The conflict commands are \"%s\" and \"%s\". "
                            "*** Please refer to AN1047, Appendix B.1\n",
                            n->word,
                            vtss_icli_register_cmd_str_get(t->node_property.cmd_property->cmd_id),
                            cmd_register->original_cmd);
                        return ICLI_RC_ERROR;
                    }
                }
            }

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
            _or_head_set( new_tree );
#endif

            // insert tree
            if ( pre_tree->child ) {
                // find last sibling
                for ( n = pre_tree->child; n->sibling != NULL; ___SIBLING(n) ) {
                    ;
                }
                // append tree
                n->sibling = new_tree;
            } else {
                pre_tree->child = new_tree;
            }

            // connect parent
            for ( n = new_tree; n; ___SIBLING(n) ) {
                n->parent = pre_tree;
                ++( n->parent_cnt );
            }

            // the new tree may be optional node with execution
            // so the parent nodes should be with execution also
            for ( ++word_id; word_id < g_number_of_nodes; ++word_id ) {
                if ( g_cmd_node[word_id].execution && g_cmd_node[word_id].optional_end ) {
                    if ( _optional_set_terminal( &g_cmd_node[word_id] ) == FALSE ) {
                        return ICLI_RC_ERROR;
                    }
                }
            }
            return ICLI_RC_OK;

        case ICLI_WORD_TYPE_EOS:
            if ( pre_tree->execution ) {
                T_E("duplicate command\n");
                return ICLI_RC_ERROR;
            }
            // this command is a sub-command
            pre_tree->execution = g_cmd_execution;
            if ( pre_tree->optional_end ) {
                if ( _optional_set_terminal( pre_tree ) == FALSE ) {
                    return ICLI_RC_ERROR;
                }
            }
            return ICLI_RC_OK;

        default:
            T_E("fail to get the first word\n");
            return ICLI_RC_ERROR;
        }

        // assign word
        g_cmd_node[word_id].word = w;

        // find overlap
        for ( ; tree != NULL; ___SIBLING(tree) ) {
            if ( _node_is_same(&g_cmd_node[word_id], tree) ) {
                break;
            }
        }

        // no overlap
        if ( tree == NULL ) {
#if 0 /* CP, 2012/12/06 11:01, check loop overlap */
            if ( b_loop ) {
                if ( loop_overlap_cnt ) {
                    T_E("Word \"%s\" in loop not overlapped, but it should.\n"
                        "The conflict commands are \"%s\" and \"%s\". "
                        "*** Please refer to AN1047, Appendix B.1\n",
                        w,
                        vtss_icli_register_cmd_str_get(pre_tree->node_property.cmd_property->cmd_id),
                        cmd_register->original_cmd);
                    return ICLI_RC_ERROR;
                } else {
                    // restore loop syntax
                    *(w - 1) = ICLI_SPACE;
                    w = w - 2;
                }
            }
#endif

            // get new tree
            *(c - 1) = ICLI_SPACE;
            new_tree = _parsing_tree(w, word_id, 0, TRUE);
            if ( new_tree == NULL ) {
                T_E("fail to build new tree\n");
                return ICLI_RC_ERROR;
            }

            // insert it
            new_tree->sibling = pre_tree->child;
            pre_tree->child = new_tree;

#if 1 /* CP, 2012/10/16 14:18, connect parent */
            // connect parent
            for ( n = new_tree; n != NULL; ___SIBLING(n) ) {
                n->parent = pre_tree;
                ++( n->parent_cnt );
            }
#endif
            return ICLI_RC_OK;
        }

        /* overlap */

#if 0 /* CP, 2012/12/06 11:01, check loop overlap */
        if ( b_loop ) {
            ++loop_overlap_cnt;
        }
#endif

        // collect un-used node
        _free_parsing_node_add( &(g_cmd_node[word_id]) );

        // append command property
        ___APPEND_NODE_PROPERTY();

        // continue
    }

    /* all overlap, just append it */
    // get new tree
#if 1 /* CP, 2012/08/22 15:35, w is also overlap one, so skip w and go to c */
    new_tree = _parsing_tree(c, word_id + 1, 0, TRUE);
#else
    *(c - 1) = ICLI_SPACE;
    new_tree = _parsing_tree(w, word_id, 0, TRUE);
#endif
    if ( new_tree == NULL ) {
        T_E("fail to build new tree\n");
        return ICLI_RC_ERROR;
    }

    // append it
    pre_tree->child = new_tree;

#if 1 /* CP, 2012/10/16 14:18, connect parent */
    // connect parent
    for ( n = new_tree; n != NULL; ___SIBLING(n) ) {
        n->parent = pre_tree;
        ++( n->parent_cnt );
    }

    // the new tree may be optional node with execution
    // so the parent nodes should be with execution also
    for ( ++word_id; word_id < g_number_of_nodes; ++word_id ) {
        if ( g_cmd_node[word_id].execution && g_cmd_node[word_id].optional_end ) {
            if ( _optional_set_terminal( &g_cmd_node[word_id] ) == FALSE ) {
                return ICLI_RC_ERROR;
            }
        }
    }
#endif

    // successful
    return ICLI_RC_OK;
}

/*
    get parsing tree according to the mode

    input -
        mode : command mode
    output -
        n/a
    return -
        not NULL : successful, tree
        NULL     : failed
    comment -
        n/a
*/
icli_parsing_node_t *vtss_icli_parsing_tree_get(
    IN  icli_cmd_mode_t         mode
)
{
    if ( mode >= ICLI_CMD_MODE_MAX ) {
        T_E("vtss_icli_parsing_tree_get : invalid mode = %d\n", mode);
        return NULL;
    }

    return g_mode_tree[mode].tree;
}

/*
    check if head is the random optional head of node

    INPUT
        head : random optional head
        node : the node to be checked

    OUTPUT
        n/a

    RETURN
        TRUE  - yes, it is
        FALSE - no

    COMMENT
        n/a
*/
BOOL vtss_icli_parsing_random_head(
    IN  icli_parsing_node_t     *head,
    IN  icli_parsing_node_t     *node,
    OUT u32                     *optional_level
)
{
    u32     i;

    if ( head == NULL ) {
        return FALSE;
    }

    if ( node == NULL ) {
        return FALSE;
    }

    for ( i = 0; i < ICLI_RANDOM_OPTIONAL_DEPTH; ++i ) {
        if ( node->random_optional[i] == head ) {
            if ( optional_level ) {
                *optional_level = node->random_optional_level[i];
            }
            return TRUE;
        }
    }
    return FALSE;
}
