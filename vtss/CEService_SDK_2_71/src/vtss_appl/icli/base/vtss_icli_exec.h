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
#ifndef __VTSS_ICLI_EXEC_H__
#define __VTSS_ICLI_EXEC_H__
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
i32 vtss_icli_exec_init(void);

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
);

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
icli_parameter_t *vtss_icli_exec_parameter_get(void);

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
);

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
);

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
);

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
);
#endif

void vtss_icli_exec_cword_runtime_sort(
    INOUT icli_runtime_t    *runtime
);

BOOL vtss_icli_exec_cword_runtime_get(
    IN  icli_session_handle_t   *handle,
    IN  icli_runtime_cb_t       *runtime_cb,
    OUT icli_runtime_t          *runtime
);

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
);

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
);

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
);

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
);

//****************************************************************************
#endif //__VTSS_ICLI_EXEC_H__

