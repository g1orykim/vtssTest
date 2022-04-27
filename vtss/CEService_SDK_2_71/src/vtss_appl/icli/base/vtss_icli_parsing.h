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
    > CP.Wang, 05/29/2013 11:38
        - create

==============================================================================
*/
#ifndef __VTSS_ICLI_PARSING_H__
#define __VTSS_ICLI_PARSING_H__
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
);

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
);

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
);

/*
    check if head is the random optiona head of node

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
);

//****************************************************************************
#endif //__VTSS_ICLI_PARSING_H__

