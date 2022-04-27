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
    > CP.Wang, 05/29/2013 11:43
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
#define __PARAMETER_CHECK(cmd_id)\
    if ( cmd_id >= ICLI_CMD_CNT ) {\
        T_E("invalid command ID = %d\n", cmd_id);\
        return ICLI_RC_ERR_PARAMETER;\
    }\
    if ( g_cmd_register[cmd_id] == NULL ) {\
        T_E("empty property for command ID = %d\n", cmd_id);\
        return ICLI_RC_ERR_PARAMETER;\
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
static i32                      g_cmd_id = 0;
static icli_cmd_register_t      *g_cmd_register[ICLI_CMD_CNT];

/*
==============================================================================

    Static Function

==============================================================================
*/

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
i32 vtss_icli_register_init(
    void
)
{
    g_cmd_id = 0;
    memset(g_cmd_register, 0, sizeof(g_cmd_register));

    return ICLI_RC_OK;
}

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
i32 vtss_icli_register_cmd(
    IN  icli_cmd_register_t     *cmd_register
)
{
    i32     r;

    ICLI_PARAMETER_NULL_CHECK( cmd_register );
    ICLI_PARAMETER_NULL_CHECK( cmd_register->cmd );
    ICLI_PARAMETER_NULL_CHECK( cmd_register->original_cmd );
    ICLI_PARAMETER_NULL_CHECK( cmd_register->node );
    ICLI_PARAMETER_NULL_CHECK( cmd_register->cmd_property );
    ICLI_PARAMETER_NULL_CHECK( cmd_register->cmd_execution );

    if ( cmd_register->cmd_mode >= ICLI_CMD_MODE_MAX ) {
        T_E("invalid command mode = %d\n", cmd_register->cmd_mode);
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( g_cmd_id >= ICLI_CMD_CNT ) {
        T_E("too many command = %d\n", g_cmd_id);
        return ICLI_RC_ERR_MEMORY;
    }

    //assign command ID
    cmd_register->cmd_property->cmd_id  = g_cmd_id;
    cmd_register->cmd_execution->cmd_id = g_cmd_id;

    //build parsing tree
    r = vtss_icli_parsing_build( cmd_register );
    if ( r != ICLI_RC_OK ) {
        T_E("fail to build parsing tree for command = %s\n", cmd_register->original_cmd);
        return r;
    }

    //add command property into list
    g_cmd_register[g_cmd_id] = cmd_register;

    //successful
    return g_cmd_id++;
}

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
i32 vtss_icli_register_cmd_is_enable(
    IN  u32     cmd_id,
    OUT BOOL    *enable
)
{
    __PARAMETER_CHECK( cmd_id );

    if ( enable == NULL ) {
        T_E("enable == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    *enable = ICLI_CMD_IS_ENABLE( g_cmd_register[cmd_id]->cmd_property->property );
    return ICLI_RC_OK;
}

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
i32 vtss_icli_register_cmd_enable(
    IN  u32     cmd_id
)
{
    __PARAMETER_CHECK( cmd_id );

    ICLI_CMD_ENABLE( &(g_cmd_register[cmd_id]->cmd_property->property) );
    return ICLI_RC_OK;

}

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
i32 vtss_icli_register_cmd_disable(
    IN  u32     cmd_id
)
{
    __PARAMETER_CHECK( cmd_id );

    ICLI_CMD_DISABLE( &(g_cmd_register[cmd_id]->cmd_property->property) );
    return ICLI_RC_OK;

}

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
i32 vtss_icli_register_cmd_is_visible(
    IN  u32     cmd_id,
    OUT BOOL    *visible
)
{
    __PARAMETER_CHECK( cmd_id );

    if ( visible == NULL ) {
        T_E("visible == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    *visible = ICLI_CMD_IS_VISIBLE( g_cmd_register[cmd_id]->cmd_property->property );
    return ICLI_RC_OK;
}

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
i32 vtss_icli_register_cmd_visible(
    IN  u32     cmd_id
)
{
    __PARAMETER_CHECK( cmd_id );

    ICLI_CMD_VISIBLE( &(g_cmd_register[cmd_id]->cmd_property->property) );
    return ICLI_RC_OK;

}

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
i32 vtss_icli_register_cmd_invisible(
    IN  u32     cmd_id
)
{
    __PARAMETER_CHECK( cmd_id );

    ICLI_CMD_INVISIBLE( &(g_cmd_register[cmd_id]->cmd_property->property) );
    return ICLI_RC_OK;

}

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
i32 vtss_icli_register_cmd_privilege_get(
    IN  u32     cmd_id,
    OUT u32     *privilege
)
{
    __PARAMETER_CHECK( cmd_id );

    if ( privilege == NULL ) {
        T_E("privilege == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    *privilege = g_cmd_register[cmd_id]->cmd_property->privilege;
    return ICLI_RC_OK;
}

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
i32 vtss_icli_register_cmd_privilege_set(
    IN u32     cmd_id,
    IN u32     privilege
)
{
    __PARAMETER_CHECK( cmd_id );

    if ( privilege >= ICLI_PRIVILEGE_MAX ) {
        T_E("invalid privilege = %d\n", privilege);
        return ICLI_RC_ERR_PARAMETER;
    }

    g_cmd_register[cmd_id]->cmd_property->privilege = privilege;
    return ICLI_RC_OK;
}

/*
    get original command string of a command

    INPUT
        cmd_id : command ID

    OUTPUT
        n/a

    RETURN
        original command string

    COMMENT
        n/a
*/
char *vtss_icli_register_cmd_str_get(
    IN  u32     cmd_id
)
{
    if ( cmd_id >= ICLI_CMD_CNT ) {
        T_E("invalid command ID = %d\n", cmd_id);
        return NULL;
    }

    if ( g_cmd_register[cmd_id] == NULL ) {
        T_E("empty property for command ID = %d\n", cmd_id);
        return NULL;
    }

    return g_cmd_register[cmd_id]->original_cmd;
}

#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
/*
    get var_cmd string of a command

    INPUT
        cmd_id : command ID

    OUTPUT
        n/a

    RETURN
        var_cmd string

    COMMENT
        n/a
*/
char *vtss_icli_register_var_cmd_get(
    IN  u32     cmd_id
)
{
    if ( cmd_id >= ICLI_CMD_CNT ) {
        T_E("invalid command ID = %d\n", cmd_id);
        return NULL;
    }

    if ( g_cmd_register[cmd_id] == NULL ) {
        T_E("empty property for command ID = %d\n", cmd_id);
        return NULL;
    }

    return g_cmd_register[cmd_id]->var_cmd;
}
#endif
