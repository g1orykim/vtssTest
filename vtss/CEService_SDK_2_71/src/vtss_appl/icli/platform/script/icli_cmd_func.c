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
    > CP.Wang, 2012/09/27 12:19
        - create

==============================================================================
*/

/*
==============================================================================

    Include File

==============================================================================
*/
#include <stdlib.h>
#include "icli_api.h"
#include "icli_porting_trace.h"

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

    Static Function

==============================================================================
*/

/*
==============================================================================

    Public Function

==============================================================================
*/
BOOL icli_config_go_to_exec_mode(
    IN u32 session_id
)
{
    i32     level;

    for (;;) {
        level = ICLI_MODE_EXIT();
        if ( level < 0 ) {
            return FALSE;
        }

        if ( level == 0 ) {
            return TRUE;
        }
    }
}

BOOL icli_config_user_str_get(
    IN  u32     session_id,
    IN  i32     max_len,
    OUT char    *user_str

)
{
    char        *str;
    char        *c;
    i32         len;
    icli_rc_t   rc;
    BOOL        b_end;
    BOOL        b_loop = TRUE;

    if ( user_str == NULL ) {
        return FALSE;
    }

    // find the end char or EoS
    for ( c = user_str + 1; *c != *user_str && *c != 0; ++c ) {
        ;
    }

    // check if end char
    b_end = FALSE;
    if ( *c == *user_str ) {
        b_end = TRUE;
        *c = 0;
    }

    // check length
    // -1 is for the starting delimiter
    len = vtss_icli_str_len(user_str) - 1;
    if ( len >= max_len ) {
        user_str[max_len + 1] = 0;
        return TRUE;
    }

    // end delimiter is present
    if ( b_end ) {
        return TRUE;
    }

    // allocate memory
    str = (char *)icli_malloc(max_len + 1);
    if ( str == NULL ) {
        T_E("memory insufficient\n");
        return FALSE;
    }

    // prepare for next input
    *c     = '\n';
    *(c + 1) = 0;

    ICLI_PRINTF("Enter TEXT message.  End with the character '%c'.\n", *user_str);
    while ( b_loop ) {
        memset(str, 0, max_len + 1);
        len = max_len;
        rc = ICLI_USR_STR_GET(ICLI_USR_INPUT_TYPE_NORMAL, str, &len, NULL);
        switch ( rc ) {
        case ICLI_RC_OK:
            // find the end char or EoS
            for ( c = str; *c != *user_str && *c != 0; ++c ) {
                ;
            }

            // check if end char
            b_end = FALSE;
            if ( *c == *user_str ) {
                b_end = TRUE;
                *c = 0;
            }

            // check length
            // -1 is for the starting delimiter
            len = vtss_icli_str_len(user_str) - 1 + vtss_icli_str_len(str);
            if ( len >= max_len ) {
                if ( vtss_icli_str_len(user_str) ) {
                    len = max_len - vtss_icli_str_len(user_str) + 1;
                } else {
                    len = max_len;
                }
                *(str + len) = 0;
                (void)vtss_icli_str_concat(user_str, str);
                // free memory
                icli_free(str);
                return TRUE;
            }

            // concat string
            (void)vtss_icli_str_concat(user_str, str);

            // end delimiter is present
            if ( b_end ) {
                // free memory
                icli_free(str);
                return TRUE;
            }

            // prepare for next input
            len = vtss_icli_str_len(user_str);
            user_str[len]   = '\n';
            user_str[len + 1] = 0;
            break;

        case ICLI_RC_ERR_EXPIRED:
            ICLI_PRINTF("\n%% timeout expired!\n");
            // free memory
            icli_free(str);
            return FALSE;

        default:
            ICLI_PRINTF("%% Fail to get from user input\n");
            // free memory
            icli_free(str);
            return FALSE;
        }
    }
    // free memory
    icli_free(str);
    return FALSE;
}
