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
    > CP.Wang, 2012/09/03 10:33
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
#include "icli_api.h"
#include "icli_porting_trace.h"

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define __VISIBLE_MIN_CHAR      33
#define __VISIBLE_MAX_CHAR      126
#define __VISIBLE_SIZE          (__VISIBLE_MAX_CHAR - __VISIBLE_MIN_CHAR + 1)
/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/
static BOOL                 g_banner_deli[__VISIBLE_SIZE];
static char                 g_banner[ICLI_BANNER_MAX_LEN + 1];
static char                 g_password[ICLI_PASSWORD_MAX_LEN + 1];
static char                 g_dev_name[ICLI_DEV_NAME_MAX_LEN + 1];
static icli_session_data_t  g_session_data;
static cyg_mutex_t          g_mutex;

/*
******************************************************************************

    Static Function

******************************************************************************
*/
static vtss_rc _banner_print(
    IN vtss_icfg_query_result_t     *result,
    IN char                         *type
)
{
    vtss_rc     rc;
    char        *c;
    char        i;

    if ( g_banner[0] ) {
        /* find delimiter */
        memset(g_banner_deli, 0, sizeof(g_banner_deli));

        // the comment token can not be used for delimiter
        g_banner_deli['!' - __VISIBLE_MIN_CHAR] = TRUE;
        g_banner_deli['#' - __VISIBLE_MIN_CHAR] = TRUE;

        for ( c = g_banner; (*c) != 0; c++ ) {
            if ( (*c) >= __VISIBLE_MIN_CHAR && (*c) <= __VISIBLE_MAX_CHAR ) {
                g_banner_deli[(*c) - __VISIBLE_MIN_CHAR] = TRUE;
            }
        }
        for ( i = 0; i < __VISIBLE_SIZE; i++ ) {
            if ( g_banner_deli[(int)i] == FALSE ) {
                break;
            }
        }
        if ( i < __VISIBLE_SIZE ) {
            i += __VISIBLE_MIN_CHAR;
        } else {
            i = '#';
        }

        /*  */
        rc = vtss_icfg_printf(result, "banner %s %c%s%c\n", type, i, g_banner, i);
        if ( rc != VTSS_RC_OK ) {
            T_E("fail to print to icfg\n");
            return VTSS_RC_ERROR;
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc _enable_password_print(
    IN vtss_icfg_query_result_t     *result,
    IN icli_privilege_t             priv
)
{
    vtss_rc     rc;

    if ( g_password[0] ) {
        if ( icli_enable_password_if_secret_get(priv) ) {
            rc = vtss_icfg_printf(result, "enable secret 5 level %d %s\n",
                                  priv, g_password);
            if ( rc != VTSS_RC_OK ) {
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        } else {
            rc = vtss_icfg_printf(result, "enable password level %d %s\n",
                                  priv, g_password);
            if ( rc != VTSS_RC_OK ) {
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }
    }
    return VTSS_RC_OK;
}

/* ICFG callback functions */
static vtss_rc _icli_icfg(
    IN const vtss_icfg_query_request_t  *req,
    IN vtss_icfg_query_result_t         *result
)
{
    vtss_rc                 rc;
    BOOL                    b_print;
    icli_privilege_t        priv;
    i32                     sec;
    icli_priv_cmd_conf_t    cmd_conf;

    if ( req == NULL ) {
        T_E("req == NULL\n");
        return VTSS_RC_ERROR;
    }

    if ( result == NULL ) {
        T_E("result == NULL\n");
        return VTSS_RC_ERROR;
    }

    if ( cyg_mutex_lock(&g_mutex) == FALSE ) {
        cyg_mutex_unlock( &g_mutex );
        T_E("cyg_mutex_lock()\n");
        return VTSS_RC_ERROR;
    }

    switch ( req->cmd_mode ) {
    case ICLI_CMD_MODE_GLOBAL_CONFIG:
        /* banner motd */
        if ( icli_banner_motd_get(g_banner) != ICLI_RC_OK ) {
            cyg_mutex_unlock( &g_mutex );
            T_E("fail to get motd banner\n");
            return VTSS_RC_ERROR;
        }
        if ( _banner_print(result, "motd") != VTSS_RC_OK ) {
            cyg_mutex_unlock( &g_mutex );
            return VTSS_RC_ERROR;
        }

        /* banner exec */
        if ( icli_banner_exec_get(g_banner) != ICLI_RC_OK ) {
            cyg_mutex_unlock( &g_mutex );
            T_E("fail to get exec banner\n");
            return VTSS_RC_ERROR;
        }
        if ( _banner_print(result, "exec") != VTSS_RC_OK ) {
            cyg_mutex_unlock( &g_mutex );
            return VTSS_RC_ERROR;
        }

        /* banner login */
        if ( icli_banner_login_get(g_banner) != ICLI_RC_OK ) {
            cyg_mutex_unlock( &g_mutex );
            T_E("fail to get login banner\n");
            return VTSS_RC_ERROR;
        }
        if ( _banner_print(result, "login") != VTSS_RC_OK ) {
            cyg_mutex_unlock( &g_mutex );
            return VTSS_RC_ERROR;
        }

        /* enable password */
        for (priv = 0; priv < ICLI_PRIVILEGE_DEBUG - 1; priv++) {
            if ( icli_enable_password_get(priv, g_password) == FALSE ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to get enable password at priv %d\n", priv);
                return VTSS_RC_ERROR;
            }
            if ( _enable_password_print(result, priv) != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                return VTSS_RC_ERROR;
            }
        }
        // highest privilege has default value, so process individually
        if ( icli_enable_password_get(priv, g_password) == FALSE ) {
            cyg_mutex_unlock( &g_mutex );
            T_E("fail to get enable password at priv %d\n", priv);
            return VTSS_RC_ERROR;
        }
        // check if print
        b_print = FALSE;
        if ( req->all_defaults ) {
            b_print = TRUE;
        } else if ( icli_str_cmp(g_password, ICLI_DEFAULT_ENABLE_PASSWORD) != 0 ) {
            // different with default
            b_print = TRUE;
        }
        if ( b_print ) {
            if ( _enable_password_print(result, priv) != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                return VTSS_RC_ERROR;
            }
        }

        /* hostname */
        if ( icli_dev_name_get(g_dev_name) != ICLI_RC_OK ) {
            cyg_mutex_unlock( &g_mutex );
            T_E("fail to get device name\n");
            return VTSS_RC_ERROR;
        }
        // check if print
        b_print = FALSE;
        if ( req->all_defaults ) {
            b_print = TRUE;
        } else if ( icli_str_cmp(g_dev_name, ICLI_DEFAULT_DEVICE_NAME) != 0 ) {
            // different with default
            b_print = TRUE;
        }
        if ( b_print && g_dev_name[0] ) {
            rc = vtss_icfg_printf(result, "hostname %s\n", g_dev_name);
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }

        /* command privilege */
        if ( icli_priv_get_first(&cmd_conf) == ICLI_RC_OK ) {
            rc = vtss_icfg_printf(result, "privilege %s level %u %s\n", icli_priv_mode_name(cmd_conf.mode), cmd_conf.privilege, cmd_conf.cmd);
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
            while ( icli_priv_get_next(&cmd_conf) == ICLI_RC_OK ) {
                rc = vtss_icfg_printf(result, "privilege %s level %u %s\n", icli_priv_mode_name(cmd_conf.mode), cmd_conf.privilege, cmd_conf.cmd);
                if ( rc != VTSS_RC_OK ) {
                    cyg_mutex_unlock( &g_mutex );
                    T_E("fail to print to icfg\n");
                    return VTSS_RC_ERROR;
                }
            }
        }

        break;

    case ICLI_CMD_MODE_CONFIG_LINE:
        /* get session data */
        g_session_data.session_id = req->instance_id.line;
        if ( icli_session_data_get(&g_session_data) != ICLI_RC_OK ) {
            cyg_mutex_unlock( &g_mutex );
            T_E("fail to get session data of session %d.\n", g_session_data.session_id);
            return VTSS_RC_ERROR;
        }

        /* editing */
        // check if print
        if ( req->all_defaults ) {
            if ( g_session_data.input_style == ICLI_INPUT_STYLE_SINGLE_LINE ) {
                rc = vtss_icfg_printf(result, " editing\n");
                if ( rc != VTSS_RC_OK ) {
                    cyg_mutex_unlock( &g_mutex );
                    T_E("fail to print to icfg\n");
                    return VTSS_RC_ERROR;
                }
            } else {
                rc = vtss_icfg_printf(result, " no editing\n");
                if ( rc != VTSS_RC_OK ) {
                    cyg_mutex_unlock( &g_mutex );
                    T_E("fail to print to icfg\n");
                    return VTSS_RC_ERROR;
                }
            }
        } else if ( g_session_data.input_style != ICLI_INPUT_STYLE_SINGLE_LINE ) {
            // different with default
            rc = vtss_icfg_printf(result, " no editing\n");
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }

        /* exec-banner */
        b_print = FALSE;
        if ( req->all_defaults ) {
            if ( g_session_data.b_exec_banner ) {
                rc = vtss_icfg_printf(result, " exec-banner\n");
                if ( rc != VTSS_RC_OK ) {
                    cyg_mutex_unlock( &g_mutex );
                    T_E("fail to print to icfg\n");
                    return VTSS_RC_ERROR;
                }
            } else {
                b_print = TRUE;
            }
        } else if ( g_session_data.b_exec_banner != TRUE ) {
            // different with default
            b_print = TRUE;
        }
        if ( b_print ) {
            rc = vtss_icfg_printf(result, " no exec-banner\n");
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }

        /* exec-timeout */
        sec = (g_session_data.wait_time <= 0) ? 0 : g_session_data.wait_time;
        b_print = FALSE;
        if ( req->all_defaults ) {
            b_print = TRUE;
        } else if ( sec != ICLI_DEFAULT_WAIT_TIME ) {
            // different with default
            b_print = TRUE;
        }
        if ( b_print ) {
            rc = vtss_icfg_printf(result, " exec-timeout %d %d\n", sec / 60, sec % 60);
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }

        /* history size */
        b_print = FALSE;
        if ( req->all_defaults ) {
            b_print = TRUE;
        } else if ( g_session_data.history_size != ICLI_HISTORY_CMD_CNT ) {
            // different with default
            b_print = TRUE;
        }
        if ( b_print ) {
            rc = vtss_icfg_printf(result, " history size %u\n", g_session_data.history_size);
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }

        /* length */
        b_print = FALSE;
        if ( req->all_defaults ) {
            b_print = TRUE;
        } else if ( g_session_data.lines != ICLI_DEFAULT_LINES ) {
            // different with default
            b_print = TRUE;
        }
        if ( b_print ) {
            rc = vtss_icfg_printf(result, " length %u\n", g_session_data.lines);
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }

        /* location */
        if ( g_session_data.location[0] ) {
            rc = vtss_icfg_printf(result, " location %s\n", g_session_data.location);
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }

        /* motd-banner */
        b_print = FALSE;
        if ( req->all_defaults ) {
            if ( g_session_data.b_motd_banner ) {
                rc = vtss_icfg_printf(result, " motd-banner\n");
                if ( rc != VTSS_RC_OK ) {
                    cyg_mutex_unlock( &g_mutex );
                    T_E("fail to print to icfg\n");
                    return VTSS_RC_ERROR;
                }
            } else {
                b_print = TRUE;
            }
        } else if ( g_session_data.b_motd_banner != TRUE ) {
            // different with default
            b_print = TRUE;
        }
        if ( b_print ) {
            rc = vtss_icfg_printf(result, " no motd-banner\n");
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }


        /* privilege level */
        b_print = FALSE;
        if ( req->all_defaults ) {
            b_print = TRUE;
        } else if ( g_session_data.privileged_level != ICLI_DEFAULT_PRIVILEGED_LEVEL ) {
            // different with default
            b_print = TRUE;
        }
        if ( b_print ) {
            rc = vtss_icfg_printf(result, " privilege level %d\n", g_session_data.privileged_level);
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }

        /* width */
        b_print = FALSE;
        if ( req->all_defaults ) {
            b_print = TRUE;
        } else if ( g_session_data.width != ICLI_DEFAULT_WIDTH ) {
            // different with default
            b_print = TRUE;
        }
        if ( b_print ) {
            rc = vtss_icfg_printf(result, " width %u\n", g_session_data.width);
            if ( rc != VTSS_RC_OK ) {
                cyg_mutex_unlock( &g_mutex );
                T_E("fail to print to icfg\n");
                return VTSS_RC_ERROR;
            }
        }
        break;

    default:
        /* no config in other modes */
        break;
    }
    cyg_mutex_unlock( &g_mutex );
    return VTSS_OK;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc icli_icfg_init(void)
{
    vtss_rc rc;

    /*
        Register Global config callback functions to ICFG module.
    */
    rc = vtss_icfg_query_register(VTSS_ICFG_GLOBAL_ICLI, "icli", _icli_icfg);
    if ( rc != VTSS_OK ) {
        return rc;
    }

    /*
        Register Line config callback functions to ICFG module.
    */
    rc = vtss_icfg_query_register(VTSS_ICFG_LINE_ICLI, "icli", _icli_icfg);
    if ( rc != VTSS_OK ) {
        return rc;
    }

    /* init mutex */
    cyg_mutex_init(&g_mutex);

    return VTSS_OK;
}
