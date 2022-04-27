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
    > CP.Wang, 05/29/2013 13:16
        - create

==============================================================================
*/
#ifndef __VTSS_ICLI_VARIABLE_H__
#define __VTSS_ICLI_VARIABLE_H__
//****************************************************************************

/*
==============================================================================

    Include File

==============================================================================
*/
#include "vtss_icli_type.h"

/*
==============================================================================

    Constant and Macro

==============================================================================
*/
#if 1 /* CP, 2012/09/25 10:47, <dscp> */
#define ICLI_DSCP_MAX_CNT   22
#endif

/*
==============================================================================

    Type Definition

==============================================================================
*/
#if 1 /* CP, 2012/09/25 10:47, <dscp> */
typedef struct {
    char    *word;
    u32     value;
    char    *help;
} icli_dscp_wvh_t;
#endif

/*
==============================================================================

    Public Function

==============================================================================
*/
/*
    parse string to get signed/unsigned range

    input -
        range_str : string of range
        delimiter : the delimiter for range

    output -
        range : range

    return -
        icli_rc_t

    comment -
        n/a
*/
i32 vtss_icli_variable_range_get(
    IN  char            *range_str,
    IN  char            delimiter,
    OUT icli_range_t    *range
);

/*
    get variable type for the name

    INPUT
        name : variable name

    OUTPUT
        type  : variable type
        range : value range

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_variable_type_get(
    IN  char                    *name,
    OUT icli_variable_type_t    *type,
    OUT icli_range_t            *range
);

/*
    get value from the word for the type

    input -
        type  : variable type
        word  : user input word
        range : range checking

    output -
        value   : corresponding value of the type if successful
        err_pos : error position if failed

    return -
        icli_rc_t

    comment -
        n/a
*/
i32 vtss_icli_variable_get(
    IN  icli_variable_type_t    type,
    IN  char                    *word,
    IN  icli_range_t            *range,
    OUT icli_variable_value_t   *value,
    OUT u32                     *err_pos
);

/*
    get name string for the port type

    INPUT
        type : port type

    OUTPUT
        n/a

    RETURN
        name string of the type
            if the type is invalid. then "Unknown" is return

    COMMENT
        n/a
*/
char *vtss_icli_variable_port_type_get_name(
    IN  icli_port_type_t    type
);

/*
    get short name string for the port type

    INPUT
        type : port type

    OUTPUT
        n/a

    RETURN
        short name string of the type
            if the type is invalid. then "Unkn" is return

    COMMENT
        n/a
*/
char *vtss_icli_variable_port_type_get_short_name(
    IN  icli_port_type_t    type
);

/*
    get help string for the port type

    INPUT
        type : port type

    OUTPUT
        n/a

    RETURN
        help string of the type
            if the type is invalid. then "Unknown" is return

    COMMENT
        n/a
*/
char *vtss_icli_variable_port_type_get_help(
    IN  icli_port_type_t    type
);

#if 1 /* CP, 2012/09/25 10:47, <dscp> */
/*
    get database pointer for DSCP word, value, help

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        icli_dscp_wvh_t *

    COMMENT
        n/a
*/
icli_dscp_wvh_t *vtss_icli_variable_dscp_wvh_get(
    void
);
#endif

#if ICLI_RANDOM_MUST_NUMBER
/*
    get 32-bit unsigned decimal

    INPUT
        word : user input word

    OUTPUT
        val  : unsigned 32-bit decimal value

    RETURN
        TRUE  - get successfully
        FALSE - fail to get
*/
BOOL vtss_icli_variable_decimal_get(
    IN  char    *word,
    OUT u32     *val
);
#endif

char *vtss_icli_variable_data_name_get(
    IN  icli_variable_type_t    type
);

char *vtss_icli_variable_data_variable_get(
    IN  icli_variable_type_t    type
);

char *vtss_icli_variable_data_decl_type_get(
    IN  icli_variable_type_t    type
);

char *vtss_icli_variable_data_init_val_get(
    IN  icli_variable_type_t    type
);

BOOL vtss_icli_variable_data_pointer_type_get(
    IN  icli_variable_type_t    type
);

BOOL vtss_icli_variable_data_string_type_get(
    IN  icli_variable_type_t    type
);

BOOL vtss_icli_variable_data_has_range_get(
    IN  icli_variable_type_t    type
);

//****************************************************************************
#endif //__VTSS_ICLI_VARIABLE_H__

