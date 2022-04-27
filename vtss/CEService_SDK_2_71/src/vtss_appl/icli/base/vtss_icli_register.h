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
    > CP.Wang, 2011/04/14 11:28
        - create

==============================================================================
*/
#ifndef __VTSS_ICLI_REGISTER_H__
#define __VTSS_ICLI_REGISTER_H__
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
i32 vtss_icli_register_cmd(
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
i32 vtss_icli_register_cmd_is_enable(
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
i32 vtss_icli_register_cmd_enable(
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
i32 vtss_icli_register_cmd_disable(
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
i32 vtss_icli_register_cmd_is_visible(
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
i32 vtss_icli_register_cmd_visible(
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
i32 vtss_icli_register_cmd_invisible(
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
i32 vtss_icli_register_cmd_privilege_get(
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
i32 vtss_icli_register_cmd_privilege_set(
    IN u32     cmd_id,
    IN u32     privilege
);

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
);

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
);
#endif

//****************************************************************************
#endif //__VTSS_ICLI_REGISTER_H__
