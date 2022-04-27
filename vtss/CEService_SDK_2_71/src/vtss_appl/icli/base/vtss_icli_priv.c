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
    > CP.Wang, 09/10/2013 16:47
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

/*
==============================================================================

    Type Definition

==============================================================================
*/
typedef struct _priv_cmd_t {
    /* config data */
    icli_priv_cmd_conf_t    conf;

    /* runtime data */
    icli_parsing_node_t     *node[ ICLI_PRIV_MATCH_NODE_CNT ];

    // list
    struct _priv_cmd_t      *prev;
    struct _priv_cmd_t      *next;
} icli_priv_cmd_t;

/*
==============================================================================

    Static Variable

==============================================================================
*/
static icli_priv_cmd_t      g_priv_cmd[ ICLI_PRIV_CMD_MAX_CNT ];
static icli_priv_cmd_t      *g_cmd_list;
static icli_priv_cmd_t      *g_free_list;

/*
==============================================================================

    Static Function

==============================================================================
*/
/*
    allocate a free command from free list

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        free command, or NULL if use out

    COMMENT
        n/a
*/
static icli_priv_cmd_t *_cmd_alloc(
    void
)
{
    icli_priv_cmd_t     *cmd;

    if ( g_free_list ) {
        cmd = g_free_list;
        g_free_list = g_free_list->next;
        memset( cmd, 0, sizeof(icli_priv_cmd_t) );
        return cmd;
    } else {
        return NULL;
    }
}

/*
    free a command and add into free list

    INPUT
        cmd : command to free

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
static void _cmd_free(
    IN icli_priv_cmd_t      *cmd
)
{
    cmd->next = g_free_list->next;
    g_free_list = cmd;
}

/*
    separate each word by one space only

    INPUT
        cmd : original command string

    OUTPUT
        cmd : formatted command string

    RETURN
        n/a

    COMMENT
        n/a
*/
static void _cmd_format(
    INOUT char      *cmd
)
{
    u32     cmd_len;
    char    *c;
    char    *fcmd;
    char    *f;
    u32     n;

    /* skip space at beginning */
    ICLI_SPACE_SKIP( c, cmd );
    cmd_len = vtss_icli_str_len( c );
    if ( cmd_len == 0 ) {
        // set to empty string
        *cmd = 0;
        return;
    }

    cmd_len += 4;
    fcmd = icli_malloc( cmd_len );
    if ( fcmd == NULL ) {
        T_E("memory insufficient\n");
        return;
    }

    memset( fcmd, 0, cmd_len );

    n = 0;
    f = fcmd;
    for ( c = cmd; ICLI_NOT_(EOS, (*c)); ++c ) {
        if ( ICLI_IS_(SPACE, (*c)) ) {
            if ( n ) {
                continue;
            } else {
                ++n;
            }
        } else {
            n = 0;
        }
        *f++ = *c;
    }

    (void)vtss_icli_str_cpy( cmd, fcmd );
    icli_free( fcmd );
}

/*
    Double ring utility : add to tail

    INPUT
        cmd : the one to add

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
static void _dring_add_to_tail(
    IN icli_priv_cmd_t      *cmd
)
{
    if ( g_cmd_list ) {
        // link prev
        g_cmd_list->prev->next = cmd;
        cmd->prev = g_cmd_list->prev;

        // link next
        g_cmd_list->prev = cmd;
        cmd->next = g_cmd_list;
    } else {
        cmd->prev  = cmd;
        cmd->next  = cmd;
        g_cmd_list = cmd;
    }
}

/*
    Double ring utility : remove a entry from double ring

    INPUT
        cmd : the one to remove

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
static void _dring_remove(
    IN icli_priv_cmd_t      *cmd
)
{
    if ( g_cmd_list == cmd ) {
        if ( g_cmd_list->prev == cmd ) {
            // only one remain, to be empty
            g_cmd_list = NULL;
            _cmd_free( cmd );
            return;
        } else {
            g_cmd_list = cmd->next;
        }
    }

    cmd->prev->next = cmd->next;
    cmd->next->prev = cmd->prev;
}

/*
    find privilege per command

    INPUT
        conf : privilege command configuration, index - mode, cmd

    OUTPUT
        n/a

    RETURN
        icli_priv_cmd_t *
        if failed then NULL

    COMMENT
        n/a
*/
static icli_priv_cmd_t *_priv_cmd_find(
    IN  icli_priv_cmd_conf_t    *conf
)
{
    icli_priv_cmd_t     *p;

    // format command string
    _cmd_format( conf->cmd );

    if ( g_cmd_list == NULL ) {
        return NULL;
    }

    p = g_cmd_list;
    do {
        if ( p->conf.mode == conf->mode && vtss_icli_str_cmp(p->conf.cmd, conf->cmd) == 0 ) {
            return p;
        }
        ___NEXT( p );
    } while ( p != g_cmd_list );

    return NULL;
}

/*
    set privilege per command

    INPUT
        cmd : set cmd->conf.privilege to cmd->node property

    OUTPUT
        n/a

    RETURN
        icli_priv_cmd_t *
        if failed then NULL

    COMMENT
        n/a
*/
static void _node_priv_set(
    IN  icli_priv_cmd_t     *cmd
)
{
    node_property_t     *prop;
    u32                 i;

    /* set privilege */
    for ( i = 0; i < ICLI_PRIV_MATCH_NODE_CNT; ++i ) {
        if ( cmd->node[i] == NULL ) {
            break;
        }
        for ( prop = &(cmd->node[i]->node_property); prop; ___NEXT(prop) ) {
            // not allow to change debug command privilege
            if ( prop->cmd_property->orig_priv < ICLI_PRIVILEGE_DEBUG ) {
                prop->cmd_property->privilege = cmd->conf.privilege;
            }
        }
    }
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
i32 vtss_icli_priv_init(
    void
)
{
    i32     i;

    memset( g_priv_cmd, 0, sizeof(g_priv_cmd) );

    g_free_list = &( g_priv_cmd[0] );
    for ( i = 1; i < ICLI_PRIV_CMD_MAX_CNT; i++ ) {
        g_priv_cmd[i].next = g_free_list;
        g_free_list = &( g_priv_cmd[i] );
    }

    g_cmd_list = NULL;
    return ICLI_RC_OK;
}

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
i32 vtss_icli_priv_set(
    IN  icli_priv_cmd_conf_t    *conf
)
{
    icli_session_handle_t   *handle;
    icli_priv_cmd_t         *cmd;
    icli_parsing_node_t     *match_node[ICLI_PRIV_MATCH_NODE_CNT];
    i32                     rc;
    u32                     session_id;
    icli_priv_cmd_t         *p;

    if ( conf == NULL ) {
        T_E("conf == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    /* find if exist */
    p = _priv_cmd_find( conf );

    /*
        already exist
    */
    if ( p ) {
        // remove from list
        _dring_remove( p );
        /* add to tail */
        _dring_add_to_tail( p );
        /* update privilege */
        p->conf.privilege = conf->privilege;
        /* set privilege */
        _node_priv_set( p );
        return ICLI_RC_OK;
    }

    /*
        not exist, create new one
    */
    session_id = vtss_icli_session_self_id();
    if ( session_id >= ICLI_SESSION_CNT ) {
        T_E("invalid session %d\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( vtss_icli_session_alive(session_id) == FALSE ) {
        T_E("session %d not alive\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    handle = vtss_icli_session_handle_get( session_id );
    if ( handle == NULL ) {
        T_E("session %d handle is NULL\n", session_id);
        return ICLI_RC_ERR_PARAMETER;
    }

    /* get match node */
    memset( match_node, 0, sizeof(match_node) );
    rc = vtss_icli_exec_priv_parsing( handle, conf, match_node );
    if ( rc != ICLI_RC_OK ) {
        return rc;
    }

    /* get free memory before setting privilege to avoid if memory insufficient */
    cmd = _cmd_alloc();
    if ( cmd == NULL ) {
        return ICLI_RC_ERR_MEMORY;
    }

    /* pack cmd */
    cmd->conf = *conf;
    memcpy( cmd->node, match_node, sizeof(match_node) );

    /* add to tail */
    _dring_add_to_tail( cmd );

    /* set privilege */
    _node_priv_set( cmd );

    return ICLI_RC_OK;
}

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
i32 vtss_icli_priv_delete(
    IN icli_priv_cmd_conf_t    *conf
)
{
    icli_priv_cmd_t     *p;
    icli_priv_cmd_t     *q;
    node_property_t     *prop;
    node_property_t     *q_prop;
    i32                 cmd_id;
    BOOL                b_found;
    u32                 i;
    u32                 j;

    if ( conf == NULL ) {
        T_E("conf == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    /* find if exist */
    p = _priv_cmd_find( conf );
    if ( p == NULL ) {
        return ICLI_RC_ERROR;
    }

    /* remove from list */
    _dring_remove( p );

    /* update the privileges that still in list */
    if ( g_cmd_list ) {
        for ( i = 0; i < ICLI_PRIV_MATCH_NODE_CNT; ++i ) {
            if ( p->node[i] == NULL ) {
                break;
            }
            for ( prop = &(p->node[i]->node_property); prop; ___NEXT(prop) ) {
                /* get command ID */
                cmd_id = prop->cmd_property->cmd_id;

                /*
                    find from latest one that contains the cmd_id
                    if found, apply its command privilege
                    if not found, then reset to default privilege
                */
                b_found = FALSE;
                q = g_cmd_list->prev;
                do {
                    for ( j = 0; j < ICLI_PRIV_MATCH_NODE_CNT; ++j ) {
                        if ( q->node[j] == NULL ) {
                            break;
                        }
                        for ( q_prop = &(q->node[j]->node_property); q_prop; ___NEXT(q_prop) ) {
                            if ( q_prop->cmd_property->cmd_id == cmd_id ) {
                                // found, apply its privilege
                                // not allow to change debug command privilege
                                if ( q_prop->cmd_property->orig_priv < ICLI_PRIVILEGE_DEBUG ) {
                                    q_prop->cmd_property->privilege = q->conf.privilege;
                                }
                                b_found = TRUE;
                                break;
                            }
                        }
                        if ( b_found ) {
                            break;
                        }
                    }
                    if ( b_found ) {
                        break;
                    }
                    ___PREV( q );
                } while ( q != g_cmd_list->prev );

                // not found, reset to default privilege
                if ( b_found == FALSE ) {
                    prop->cmd_property->privilege = prop->cmd_property->orig_priv;
                }
            }
        }
    } else {
        // reset the privilege
        for ( i = 0; i < ICLI_PRIV_MATCH_NODE_CNT; ++i ) {
            if ( p->node[i] == NULL ) {
                break;
            }
            for ( prop = &(p->node[i]->node_property); prop; ___NEXT(prop) ) {
                prop->cmd_property->privilege = prop->cmd_property->orig_priv;
            }
        }
    }

    /* free p */
    _cmd_free( p );

    return ICLI_RC_OK;
}

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
i32 vtss_icli_priv_get_first(
    OUT icli_priv_cmd_conf_t    *conf
)
{
    if ( conf == NULL ) {
        T_E("conf == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( g_cmd_list == NULL ) {
        return ICLI_RC_ERROR;
    }

    *conf = g_cmd_list->conf;
    return ICLI_RC_OK;
}

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
i32 vtss_icli_priv_get(
    INOUT icli_priv_cmd_conf_t    *conf
)
{
    icli_priv_cmd_t     *p;

    if ( conf == NULL ) {
        T_E("conf == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    p = _priv_cmd_find( conf );
    if ( p == NULL ) {
        return ICLI_RC_ERROR;
    }

    *conf = p->conf;
    return ICLI_RC_OK;
}

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
i32 vtss_icli_priv_get_next(
    INOUT icli_priv_cmd_conf_t    *conf
)
{
    icli_priv_cmd_t     *p;

    if ( conf == NULL ) {
        T_E("conf == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    /* find current one */
    p = _priv_cmd_find( conf );
    if ( p == NULL ) {
        return ICLI_RC_ERROR;
    }

    /* this is the last one, so no next one */
    if ( p->next == g_cmd_list ) {
        return ICLI_RC_ERROR;
    }

    *conf = p->next->conf;
    return ICLI_RC_OK;
}
