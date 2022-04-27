/*

 Vitesse Switch API software.

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
******************************************************************************

    Revision history
    > CP.Wang, 2012/10/18 10:10
        - create

******************************************************************************
*/

/*
******************************************************************************

    Include files

******************************************************************************
*/
#include <stdlib.h>
#include "icfg_api.h"
#include "vtss_users_api.h"
#include "sysutil_api.h"
#include "cyg/athttpd/auth.h"

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/*
******************************************************************************

    Static Function

******************************************************************************
*/
/* ICFG callback functions */
static vtss_rc _users_icfg(
    IN const vtss_icfg_query_request_t  *req,
    IN vtss_icfg_query_result_t         *result
)
{
    users_conf_t    conf;
    u32             max_digest_length = ((VTSS_SYS_INPUT_PASSWD_LEN / 3 + ((VTSS_SYS_INPUT_PASSWD_LEN % 3) ? 1 : 0)) * 4);
    char            digest[max_digest_length];

    if ( req == NULL ) {
        return VTSS_RC_ERROR;
    }

    if ( result == NULL ) {
        return VTSS_RC_ERROR;
    }

    switch ( req->cmd_mode ) {
    case ICLI_CMD_MODE_GLOBAL_CONFIG:
        memset(&conf, 0x0, sizeof(conf));
        while (vtss_users_mgmt_conf_get(&conf, TRUE) == VTSS_OK) {
            if ( conf.password[0] ) {
                memset(digest, 0, sizeof(digest));
                (void)cyg_httpd_base64_encode(digest, conf.password, icli_str_len(conf.password));
                (void)vtss_icfg_printf(result, "username %s privilege %d password encrypted %s\n",
                                       conf.username, conf.privilege_level, digest);
            } else {
                (void)vtss_icfg_printf(result, "username %s privilege %d password none\n",
                                       conf.username, conf.privilege_level);
            }
        }
        break;

    default:
        /* no config in other modes */
        break;
    }
    return VTSS_OK;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc vtss_users_icfg_init(void)
{
    vtss_rc rc;

    /*
        Register Global config callback functions to ICFG module.
    */
    rc = vtss_icfg_query_register(VTSS_ICFG_GLOBAL_USERS, "user", _users_icfg);
    return rc;
}
