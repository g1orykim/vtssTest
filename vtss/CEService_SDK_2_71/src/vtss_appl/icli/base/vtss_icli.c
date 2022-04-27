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
#define _PARAMETER_CHECK(x)\
    if ( (x) == NULL ) {\
        T_E(#x" == NULL\n");\
        return ICLI_RC_ERR_PARAMETER;\
    }

#define _INIT_CHECK()\
    if ( g_is_init == FALSE ) {\
        T_E("not initialized yet\n");\
        return ICLI_RC_ERROR;\
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
static BOOL                 g_is_init = FALSE;
static icli_init_data_t     g_init_data;

#if 1 /* CP, 2012/09/14 10:38, conf */

static icli_conf_data_t     g_conf_data;

#else /* CP, 2012/09/14 10:38, conf */

/* this must be initialized one by one mapping even empty string */
static char     g_mode_name[ICLI_CMD_MODE_MAX][ICLI_PROMPT_MAX_LEN + 1];

/* device name */
static char     g_dev_name[ICLI_NAME_MAX_LEN + 1];

/* banner */
static char     g_banner_login[ICLI_BANNER_MAX_LEN + 1];
static char     g_banner_motd[ICLI_BANNER_MAX_LEN + 1];
static char     g_banner_exec[ICLI_BANNER_MAX_LEN + 1];

/* enable password for each privilege */
static char     g_enable_password[ICLI_PRIVILEGE_MAX][ICLI_PASSWORD_MAX_LEN + 1];

/* TRUE: secret password, FALSE: clear password */
static BOOL     g_b_enable_secret[ICLI_PRIVILEGE_MAX];

#endif /* CP, 2012/09/14 10:38, conf */

static u8       g_enable_key[] = {0x77, 0x89, 0x51, 0x03, 0x42, 0x90, 0x68, 0x00, 0x14, 0x97, 0x88};
static size_t   g_enable_key_len = 11;

/* port config */
static icli_stack_port_range_t      g_port_range;
static icli_stack_port_range_t      g_port_range_backup;

#if 1 /* CP, 2012/10/08 14:31, debug command, debug prompt */
static char     g_debug_prompt[ICLI_NAME_MAX_LEN + 1];
#endif

/*
==============================================================================

    Static Function

==============================================================================
*/
/*
    check if port in (begin_port, port_cnt)
*/
static BOOL _port_in_range(
    IN u32      port,
    IN u32      begin_port,
    IN u32      port_cnt
)
{
    if ( port < begin_port ) {
        return FALSE;
    }

    if ( port > (begin_port + port_cnt - 1) ) {
        return FALSE;
    }

    return TRUE;
}

/*
    port compare
    INDEX
        switch_range->switch_id  : switch ID
        switch_range->port_type  : port type
        switch_range->begin_port : port ID

    RETURN
        1 : a > b
        0 > a = b
       -1 : a < b
*/
static i32 _port_compare(
    IN icli_switch_port_range_t  *a,
    IN icli_switch_port_range_t  *b
)
{
    if ( a->switch_id > b->switch_id ) {
        return 1;
    }
    if ( a->switch_id < b->switch_id ) {
        return -1;
    }

    if ( a->port_type > b->port_type ) {
        return 1;
    }
    if ( a->port_type < b->port_type ) {
        return -1;
    }

    if ( a->begin_port > b->begin_port ) {
        return 1;
    }
    if ( a->begin_port < b->begin_port ) {
        return -1;
    }

    return 0;
}

static void _port_range_update(void)
{
    u32                         i;
    icli_switch_port_range_t    *r;

    for ( i = 0; i < g_port_range.cnt; ++i ) {
        r = &(g_port_range.switch_range[i]);

        r->usid        = icli_isid2usid( r->isid );
        r->begin_uport = icli_iport2uport( r->begin_iport );
        r->switch_id   = icli_usid2switchid( r->usid );
    }
}

static char _hex_to_char(
    IN u8 hex
)
{
    if ( hex < 10 ) {
        return '0' + hex;
    } else {
        return 'A' + hex - 10;
    }
}

static void _mac_to_string(
    IN  u8      *mac,
    OUT char    *str
)
{
    u32 i;

    // translate into string
    for ( i = 0; i < 16; ++i ) {
        str[2 * i]   = _hex_to_char( mac[i] & 0x0F );
        str[2 * i + 1] = _hex_to_char( (mac[i] & 0xF0) >> 4 );
    }
    str[ICLI_PASSWORD_MAX_LEN] = ICLI_EOS;
}

/*
    Index:
        port_type
        switch_id
        begin_port

    Return:
         1 : spr1 > spr2
         0 : spr1 == spr2
        -1 : spr1 < spr2
*/
static int _switch_port_range_compare(
    IN  icli_switch_port_range_t    *spr1,
    IN  icli_switch_port_range_t    *spr2
)
{
    if ( spr1->port_type > spr2->port_type ) {
        return 1;
    }

    if ( spr1->port_type < spr2->port_type ) {
        return -1;
    }

    if ( spr1->switch_id > spr2->switch_id ) {
        return 1;
    }

    if ( spr1->switch_id < spr2->switch_id ) {
        return -1;
    }

    if ( spr1->begin_port > spr2->begin_port ) {
        return 1;
    }

    if ( spr1->begin_port < spr2->begin_port ) {
        return -1;
    }

    /* all equal */
    return 0;
}

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
/*
    add only one port at one time
    spr->port_cnt is ignored !!!
*/
static BOOL _switch_range_result_add(
    IN  icli_switch_port_range_t    *spr,
    OUT icli_stack_port_range_t     *result
)
{
    u32     i;
    u32     j;

    i = 0;
    if ( result->cnt ) {
        /*
            find continuous one
            The checking is enough by uport because uport will not be continuous.
            port is always continuous so it does not need to check port.
         */
        for ( i = 0; i < result->cnt; ++i ) {
            if ( spr->port_type == result->switch_range[i].port_type && spr->usid == result->switch_range[i].usid ) {
                if ( spr->begin_uport == (result->switch_range[i].begin_uport + result->switch_range[i].port_cnt) ) {
                    // continuous, so add port count then done
                    ++( result->switch_range[i].port_cnt );
                    // check if next is continuous
                    if ( i < result->cnt - 1 ) {
                        if ( result->switch_range[i].port_type == result->switch_range[i + 1].port_type &&
                             result->switch_range[i].usid == result->switch_range[i + 1].usid &&
                             (result->switch_range[i].begin_uport + result->switch_range[i].port_cnt) == result->switch_range[i + 1].begin_uport ) {
                            // add
                            result->switch_range[i].port_cnt += result->switch_range[i + 1].port_cnt;
                            // shift
                            for ( j = i + 1; j < result->cnt - 1; ++j ) {
                                result->switch_range[j] = result->switch_range[j + 1];
                            }
                            --( result->cnt );
                        }
                    }
                    return TRUE;
                } else if ( spr->begin_uport == (result->switch_range[i].begin_uport - 1) ) {
                    // continuous at the front
                    --( result->switch_range[i].begin_port );
                    --( result->switch_range[i].begin_uport );
                    --( result->switch_range[i].begin_iport );
                    ++( result->switch_range[i].port_cnt );
                    // check if previous is continuous
                    if ( i ) {
                        if ( result->switch_range[i].port_type == result->switch_range[i - 1].port_type &&
                             result->switch_range[i].usid == result->switch_range[i - 1].usid &&
                             result->switch_range[i].begin_uport == (result->switch_range[i - 1].begin_uport + result->switch_range[i - 1].port_cnt) ) {
                            // add
                            result->switch_range[i - 1].port_cnt += result->switch_range[i].port_cnt;
                            // shift
                            for ( j = i; j < result->cnt - 1; ++j ) {
                                result->switch_range[j] = result->switch_range[j + 1];
                            }
                            --( result->cnt );
                        }
                    }
                    return TRUE;
                } else if ( _port_in_range(spr->begin_uport, result->switch_range[i].begin_uport, result->switch_range[i].port_cnt) ) {
                    // already in range
                    return TRUE;
                }
            }
        }

        // new one
        i = result->cnt;
        if ( i >= ICLI_RANGE_LIST_CNT ) {
            T_E("result list is full\n");
            return FALSE;
        }

        // sorted add, port type, switch id, begin port
        for ( i = 0; i < result->cnt; ++i ) {
            if ( _switch_port_range_compare(spr, &(result->switch_range[i])) == 1 ) {
                continue;
            }
            for ( j = result->cnt; j > i; --j ) {
                result->switch_range[j] = result->switch_range[j - 1];
            }
            break;
        }
    }

    ++( result->cnt );
    result->switch_range[i] = *spr;
    return TRUE;
}

/*
    get valid port range for port type

    INPUT
        port_type    : port type
        switch_range : port range to check

    OUTPUT
        result    : valid port range, add after result->cnt
        switch_id : if invalid, then this is the invalid switch ID
        port_id   : if invalid, then this is the invalid port ID

    RETURN
        TRUE  : switch_range are valid for port_type
        FALSE : not valid

    COMMENT
        n/a
*/
static BOOL _switch_range_get(
    IN  icli_port_type_t            port_type,
    IN  icli_switch_port_range_t    *switch_range,
    OUT icli_stack_port_range_t     *result,
    OUT u16                         *switch_id,
    OUT u16                         *port_id
)
{
    u16                         port;
    u16                         end_port;
    icli_switch_port_range_t    spr;

    // end port number
    end_port = switch_range->begin_port + switch_range->port_cnt;

    // check each port
    for ( port = switch_range->begin_port; port < end_port; ++port ) {
        spr.port_type  = port_type;
        spr.switch_id  = switch_range->switch_id;
        spr.begin_port = port;

        if ( vtss_icli_port_get(&spr) == FALSE ) {
            if ( switch_id && port_id ) {
                *switch_id = switch_range->switch_id;
                *port_id   = port;
            }
            return FALSE;
        }

        // add into result
        if ( _switch_range_result_add(&spr, result) == FALSE ) {
            T_E("Failed on _switch_range_result_add()\n");
            return FALSE;
        }
    } // for

    return TRUE;
}

/*
    get valid port range for port type

    INPUT
        port_type    : port type
        switch_range : port range to check

    OUTPUT
        result : valid port range, add after result->cnt

    RETURN
        TRUE  : switch_range are valid for port_type
        FALSE : not valid

    COMMENT
        n/a
*/
static BOOL _switch_range_wildcard_get(
    IN  icli_port_type_t            port_type,
    IN  icli_switch_port_range_t    *switch_range,
    OUT icli_stack_port_range_t     *result
)
{
    icli_port_type_t            pt;
    u16                         sid;
    u16                         port;
    u32                         valid_cnt;
    icli_switch_port_range_t    spr;

    valid_cnt = 0;
    for ( pt = ICLI_PORT_TYPE_FAST_ETHERNET; pt < ICLI_PORT_TYPE_MAX; ++pt ) {
        if ( port_type != ICLI_PORT_TYPE_ALL && port_type != pt ) {
            continue;
        }

        for ( sid = 1; sid < ICLI_MAX_SWITCH_ID; ++sid ) {
            if ( switch_range->switch_id != ICLI_SWITCH_PORT_ALL && switch_range->switch_id != sid ) {
                continue;
            }

            for ( port = 1; port < ICLI_MAX_PORT_ID; ++port ) {
                if ( switch_range->begin_port != ICLI_SWITCH_PORT_ALL &&
                     ! _port_in_range(port, switch_range->begin_port, switch_range->port_cnt) ) {
                    continue;
                }

                spr.port_type  = pt;
                spr.switch_id  = sid;
                spr.begin_port = port;

                // get port range and add into result
                if ( vtss_icli_port_get(&spr) && _switch_range_result_add(&spr, result) ) {
                    ++valid_cnt;
                }
            } // for port
        } // for sid
    } // for pt

    if ( valid_cnt == 0 ) {
        // no valid port
        return FALSE;
    }
    return TRUE;
}
#endif

static void _stack_port_range_sort(
    IN  icli_stack_port_range_t  *src,
    OUT icli_stack_port_range_t  *dst
)
{
    u32     i;
    u32     j;
    u32     k;

    memset( dst, 0, sizeof(icli_stack_port_range_t) );
    j = 0;
    for ( i = 0; i < src->cnt; ++i ) {
        if ( dst->cnt ) {
            for ( j = 0; j < dst->cnt; ++j ) {
                if ( _switch_port_range_compare(&(src->switch_range[i]), &(dst->switch_range[j])) == 1 ) {
                    continue;
                }
                // shift
                for ( k = dst->cnt; k > j; --k ) {
                    dst->switch_range[k] = dst->switch_range[k - 1];
                }
                break;
            }
        }
        ++( dst->cnt );
        dst->switch_range[j] = src->switch_range[i];
    }
}

#if 1 /* Bugzilla#13354 - reset to default */
/*
    reset config data to default

    INPUT
        n/a

    OUTPUT
        conf - config data to reset default

    RETURN
        TRUE  - successful
        FALSE - failed

    COMMENT
        n/a
*/
void _cfg_default(
    OUT  icli_conf_data_t     *conf
)
{
    u32     i;

    // reset all
    memset(conf, 0, sizeof(icli_conf_data_t));

    // device name
    (void)vtss_icli_str_ncpy(conf->dev_name, ICLI_DEFAULT_DEVICE_NAME, ICLI_NAME_MAX_LEN);

    // mode name
    for ( i = 0; i < ICLI_CMD_MODE_MAX; ++i ) {
        (void)vtss_icli_str_ncpy(conf->mode_prompt[i], icli_mode_prompt_get(i), ICLI_PROMPT_MAX_LEN);
    }

    // reset session config data to default
    for ( i = 0; i < ICLI_SESSION_CNT; ++i ) {
        (void)vtss_icli_session_config_data_default( &(conf->session_config[i]) );
    }
}
#endif

/*
==============================================================================

    Public Function

==============================================================================
*/
/*
    initialize ICLI engine

    INPUT
        init : data for initialization

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_init(
    IN  icli_init_data_t     *init_data
)
{
    if ( init_data == NULL ) {
        T_E("init_data == NULL\n");
        return ICLI_RC_ERROR;
    }

    if ( g_is_init ) {
        T_E("initialized already\n");
        return ICLI_RC_ERROR;
    }

    memset(&g_conf_data, 0, sizeof(g_conf_data));

    /* reset port range */
    vtss_icli_port_range_reset();

#if 1 /* CP, 2012/10/08 14:31, debug command, debug prompt */
    memset(g_debug_prompt, 0, sizeof(g_debug_prompt));
#endif

    /* get init data */
    memcpy( &g_init_data, init_data, sizeof(icli_init_data_t) );

    /* init parsing */
    if ( vtss_icli_parsing_init() != ICLI_RC_OK ) {
        T_E("fail to init parsing\n");
        return ICLI_RC_ERROR;
    }

    /* init register */
    if ( vtss_icli_register_init() != ICLI_RC_OK ) {
        T_E("fail to init register\n");
        return ICLI_RC_ERROR;
    }

    /* init session */
    if ( vtss_icli_session_init() != ICLI_RC_OK ) {
        T_E("fail to init session\n");
        return ICLI_RC_ERROR;
    }

    /* init execution */
    if ( vtss_icli_exec_init() != ICLI_RC_OK ) {
        T_E("fail to init execution\n");
        return ICLI_RC_ERROR;
    }

    /* init priv */
    if ( vtss_icli_priv_init() != ICLI_RC_OK ) {
        T_E("fail to init priv\n");
        return ICLI_RC_ERROR;
    }

    /* init vlan */
    if ( vtss_icli_vlan_init() != ICLI_RC_OK ) {
        T_E("fail to init vlan\n");
        return ICLI_RC_ERROR;
    }

    g_is_init = TRUE;
    return ICLI_RC_OK;
}

/*
    get address of conf data
    this is for internal use only

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        address of conf data

    COMMENT
        n/a
*/
icli_conf_data_t *vtss_icli_conf_get(
    void
)
{
    return &g_conf_data;
}

#if 1 /* Bugzilla#13354 - reset to default */
/*
    set config data to ICLI engine

    INPUT
        conf - config data to set

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
void vtss_icli_conf_set(
    IN  icli_conf_data_t     *conf
)
{
    icli_session_handle_t   *handle;
    u32                     i;

    memcpy(&g_conf_data, conf, sizeof(icli_conf_data_t));

    // apply to runtime
    for ( i = 0; i < ICLI_SESSION_CNT; ++i ) {
        handle = vtss_icli_session_handle_get(i);

        // width
        if ( conf->session_config[i].width == 0 ) {
            handle->runtime_data.width = ICLI_MAX_WIDTH;
        } else if ( conf->session_config[i].width < ICLI_MIN_WIDTH ) {
            handle->runtime_data.width = ICLI_MIN_WIDTH;
        } else {
            handle->runtime_data.width = conf->session_config[i].width;
        }

        // lines
        if ( conf->session_config[i].lines == 0 ) {
            handle->runtime_data.lines = 0;
        } else if ( conf->session_config[i].lines < ICLI_MIN_LINES ) {
            handle->runtime_data.lines = ICLI_MIN_LINES;
        } else {
            handle->runtime_data.lines = conf->session_config[i].lines;
        }
    }
}

/*
    reset to default for whole ICLI

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_conf_default(
    void
)
{
    icli_conf_data_t     *conf;

    conf = icli_malloc( sizeof(icli_conf_data_t) );
    if ( conf == NULL ) {
        T_E("memory insufficient\n");
        return ICLI_RC_ERR_MEMORY;
    }

    _cfg_default( conf );
    vtss_icli_conf_set( conf );
    icli_free( conf );

    /* init priv */
    if ( vtss_icli_priv_init() != ICLI_RC_OK ) {
        T_E("fail to init priv\n");
        return ICLI_RC_ERROR;
    }

    /* init vlan */
    if ( vtss_icli_vlan_init() != ICLI_RC_OK ) {
        T_E("fail to init vlan\n");
        return ICLI_RC_ERROR;
    }

    return ICLI_RC_OK;
}
#endif

/*
    get whick input_style the ICLI engine perform

    input -
        n/a

    output -
        n/a

    return -
        icli_input_style_t

    comment -
        n/a
*/
icli_input_style_t vtss_icli_input_style_get(void)
{
    return g_init_data.input_style;
}

/*
    set whick input_style the ICLI engine perform

    input -
        input_style : input style

    output -
        n/a

    return -
        icli_rc_t

    comment -
        n/a
*/
i32 vtss_icli_input_style_set(
    IN icli_input_style_t   input_style
)
{
    if ( input_style >= ICLI_INPUT_STYLE_MAX ) {
        return ICLI_RC_ERR_PARAMETER;
    }
    g_init_data.input_style = input_style;
    return ICLI_RC_OK;
}

/*
    get case sensitive or not

    input -
        n/a

    output -
        n/a

    return -
        1 - yes
        0 - not

    comment -
        n/a
*/
u32 vtss_icli_case_sensitive_get(void)
{
    return g_init_data.case_sensitive;
}

/*
    get if console is always alive or not

    input -
        n/a

    output -
        n/a

    return -
        1 - yes
        0 - not

    comment -
        n/a
*/
u32 vtss_icli_console_alive_get(void)
{
    return g_init_data.console_alive;
}

/*
    get mode name without semaphore

    INPUT
        mode : command mode

    OUTPUT
        n/a

    RETURN
        not NULL : successful
        NULL     : failed

    COMMENT
        n/a
*/
char *vtss_icli_mode_prompt_get(
    IN  icli_cmd_mode_t     mode
)
{
    if ( mode >= ICLI_CMD_MODE_MAX ) {
        return NULL;
    }

    return g_conf_data.mode_prompt[ mode ];
}

/*
    set mode name shown in command prompt

    INPUT
        mode : command mode
        name : mode name

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_mode_prompt_set(
    IN  icli_cmd_mode_t     mode,
    IN  char                *prompt
)
{
    if ( mode >= ICLI_CMD_MODE_MAX ) {
        T_E("invalud mode = %d\n", mode);
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( prompt == NULL ) {
        T_E("prompt == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    (void)vtss_icli_str_ncpy(g_conf_data.mode_prompt[mode], prompt, ICLI_PROMPT_MAX_LEN);
    g_conf_data.mode_prompt[mode][ICLI_PROMPT_MAX_LEN] = 0;

    return ICLI_RC_OK;
}

/*
    get device name without semaphore

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        g_conf_data.dev_name

    COMMENT
        n/a
*/
char *vtss_icli_dev_name_get(void)
{
    return g_conf_data.dev_name;
}

/*
    set device name shown in command prompt

    INPUT
        dev_name : device name

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_dev_name_set(
    IN  char    *dev_name
)
{
    if ( dev_name == NULL ) {
        T_E("dev_name == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    (void)vtss_icli_str_ncpy(g_conf_data.dev_name, dev_name, ICLI_DEV_NAME_MAX_LEN);
    g_conf_data.dev_name[ICLI_DEV_NAME_MAX_LEN] = 0;
    return ICLI_RC_OK;
}

/*
    get LOGIN banner without semaphore

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        g_conf_data.banner_login

    COMMENT
        n/a
*/
char *vtss_icli_banner_login_get(void)
{
    return g_conf_data.banner_login;
}

/*
    set LOGIN banner.
    the maximum length is 255 chars, ICLI_BANNER_MAX_LEN.

    INPUT
        banner_login : LOGIN banner

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_banner_login_set(
    IN  char    *banner_login
)
{
    if ( banner_login == NULL ) {
        g_conf_data.banner_login[0] = 0;
    } else {
        (void)vtss_icli_str_ncpy(g_conf_data.banner_login, banner_login, ICLI_BANNER_MAX_LEN);
        g_conf_data.banner_login[ICLI_BANNER_MAX_LEN] = 0;
    }
    return ICLI_RC_OK;
}

/*
    get MOTD banner without semaphore

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        g_conf_data.banner_motd

    COMMENT
        n/a
*/
char *vtss_icli_banner_motd_get(void)
{
    return g_conf_data.banner_motd;
}

/*
    set MOTD banner.
    the maximum length is 255 chars, ICLI_BANNER_MAX_LEN.

    INPUT
        banner_motd : MOTD banner

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_banner_motd_set(
    IN  char    *banner_motd
)
{
    if ( banner_motd == NULL ) {
        g_conf_data.banner_motd[0] = 0;
    } else {
        (void)vtss_icli_str_ncpy(g_conf_data.banner_motd, banner_motd, ICLI_BANNER_MAX_LEN);
        g_conf_data.banner_motd[ICLI_BANNER_MAX_LEN] = 0;
    }
    return ICLI_RC_OK;
}

/*
    get EXEC banner without semaphore

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        g_conf_data.banner_exec

    COMMENT
        n/a
*/
char *vtss_icli_banner_exec_get(void)
{
    return g_conf_data.banner_exec;
}

/*
    set EXEC banner.
    the maximum length is 255 chars, ICLI_BANNER_MAX_LEN.

    INPUT
        banner_exec : EXEC banner

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_banner_exec_set(
    IN  char    *banner_exec
)
{
    if ( banner_exec == NULL ) {
        g_conf_data.banner_exec[0] = 0;
    } else {
        (void)vtss_icli_str_ncpy(g_conf_data.banner_exec, banner_exec, ICLI_BANNER_MAX_LEN);
        g_conf_data.banner_exec[ICLI_BANNER_MAX_LEN] = 0;
    }
    return ICLI_RC_OK;
}

/*
    check if the port type is present in this device

    INPUT
        port_type : port type

    OUTPUT
        n/a

    RETURN
        TRUE  : yes, the device has ports belong to this port type
        FALSE : no

    COMMENT
        n/a
*/
BOOL vtss_icli_port_type_present(
    IN icli_port_type_t     port_type
)
{
    u32     i;

    if ( port_type >= ICLI_PORT_TYPE_MAX ) {
        T_E("invalid port type %d\n", port_type);
        return FALSE;
    }

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
    if ( port_type == ICLI_PORT_TYPE_ALL ) {
        return TRUE;
    }
#endif

    for ( i = 0; i < g_port_range.cnt; ++i ) {
        if ( g_port_range.switch_range[i].port_type == port_type ) {
            return TRUE;
        }
    }
    return FALSE;
}

/*
    find the next present type

    INPUT
        port_type : port type

    OUTPUT
        n/a

    RETURN
        others              : next present type
        ICLI_PORT_TYPE_NONE : no next

    COMMENT
        n/a
*/
icli_port_type_t vtss_icli_port_next_present_type(
    IN icli_port_type_t     port_type
)
{
    /* get next type */
    if ( port_type >= (ICLI_PORT_TYPE_MAX - 1) ) {
        return ICLI_PORT_TYPE_NONE;
    }

    for ( port_type++; port_type < ICLI_PORT_TYPE_MAX; ++port_type ) {
        if ( vtss_icli_port_type_present(port_type) ) {
            return port_type;
        }
    }
    return ICLI_PORT_TYPE_NONE;
}

/*
    backup port range
    this is to allow g_port_range for temp use for all API's of
    icli_port_xxx().

    *MUST*  after using, vtss_icli_port_range_restore() is called to restore the
            original system port range

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
void vtss_icli_port_range_backup( void )
{
    g_port_range_backup = g_port_range;
}

/*
    restore port range

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
void vtss_icli_port_range_restore( void )
{
    g_port_range = g_port_range_backup;
}

/*
    reset port range

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        n/a

    COMMENT
        n/a
*/
void vtss_icli_port_range_reset( void )
{
    memset( &g_port_range, 0, sizeof(g_port_range) );
}

/*
    get port range

    INPUT
        n/a

    OUTPUT
        current stack port range

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_port_range_get(
    OUT icli_stack_port_range_t     *spr
)
{
    _port_range_update();

    if ( spr == NULL ) {
        return FALSE;
    }

    memcpy(spr, &g_port_range, sizeof(icli_stack_port_range_t));
    return TRUE;
}


/*
    set port range on a specific port type

    INPUT
        range  : port range set on the port type

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        CIRTICAL ==> before set, port range must be reset by vtss_icli_port_range_reset().
        otherwise, the set may be failed because this set will check the
        port definition to avoid duplicate definitions.
*/
BOOL vtss_icli_port_range_set(
    IN icli_stack_port_range_t  *range
)
{
    u32                         i;
    u32                         j;
    u16                         begin_port;
    u16                         end_port;
    icli_switch_port_range_t    *switch_range;

    if ( range == NULL ) {
        T_E("range == NULL\n");
        return FALSE;
    }

    /* check range */
    for ( i = 0; i < range->cnt; ++i ) {
        /* get switch_range */
        switch_range = &(range->switch_range[i]);

        /* check port type */
        if ( switch_range->port_type >= ICLI_PORT_TYPE_MAX ) {
            T_E("invalid range->switch_range[%d].port_type == %u\n", i, switch_range->port_type);
            return FALSE;
        }

        /* check switch ID */
        if ( switch_range->switch_id == 0 ) {
            T_E("invalid range->switch_range[%d].switch_id == 0\n", i);
            return FALSE;
        }

        /* check port ID */
        if ( switch_range->begin_port == 0 ) {
            T_E("invalid range->switch_range[%d].begin_port == 0\n", i);
            return FALSE;
        }

        /* check port count */
        if ( switch_range->port_cnt == 0 ) {
            T_E("invalid range->switch_range[%d].port_cnt == 0\n", i);
            return FALSE;
        }

        /* check switch port */
        begin_port = switch_range->begin_port;
        end_port   = switch_range->begin_port + switch_range->port_cnt - 1;
        for ( j = 0; j < range->cnt; ++j ) {
            if ( j == i ) {
                continue;
            }
            if ( switch_range->port_type != range->switch_range[j].port_type ) {
                continue;
            }
            if ( switch_range->switch_id != range->switch_range[j].switch_id ) {
                continue;
            }
            if ( _port_in_range( begin_port,
                                 range->switch_range[j].begin_port,
                                 range->switch_range[j].port_cnt ) ) {
                T_E("switch port overlapped between %d and %d\n", i, j);
                return FALSE;
            }
            if ( _port_in_range( end_port,
                                 range->switch_range[j].begin_port,
                                 range->switch_range[j].port_cnt ) ) {
                T_E("switch port overlapped between %d and %d\n", i, j);
                return FALSE;
            }
        }

        /* check iport */
        begin_port = switch_range->begin_iport;
        end_port   = switch_range->begin_iport + switch_range->port_cnt - 1;
        for ( j = 0; j < range->cnt; ++j ) {
            if ( j == i ) {
                continue;
            }
            if ( switch_range->isid != range->switch_range[j].isid ) {
                continue;
            }
            if ( _port_in_range( begin_port,
                                 range->switch_range[j].begin_iport,
                                 range->switch_range[j].port_cnt ) ) {
                T_E("uport overlapped between %d and %d\n", i, j);
                return FALSE;
            }
            if ( _port_in_range( end_port,
                                 range->switch_range[j].begin_iport,
                                 range->switch_range[j].port_cnt ) ) {
                T_E("uport overlapped between %d and %d\n", i, j);
                return FALSE;
            }
        }
    }

    /* sort and set */
    _stack_port_range_sort( range, &g_port_range );
    return TRUE;
}

/*
    get switch range from usid and uport

    INPUT
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->port_cnt    : number of ports

    OUTPUT
        switch_range

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_port_from_usid_uport(
    INOUT icli_switch_port_range_t  *switch_range
)
{
    u32     i;
    u32     end_uport;

    if ( switch_range == NULL ) {
        T_E("switch_range == NULL\n");
        return FALSE;
    }

    if ( switch_range->port_cnt == 0 ) {
        T_E("port_cnt == 0\n");
        return FALSE;
    }

    _port_range_update();

    end_uport = switch_range->begin_uport + switch_range->port_cnt - 1;
    for ( i = 0; i < g_port_range.cnt; ++i ) {
        if ( g_port_range.switch_range[i].usid != switch_range->usid ) {
            continue;
        }
        if ( _port_in_range( switch_range->begin_uport,
                             g_port_range.switch_range[i].begin_uport,
                             g_port_range.switch_range[i].port_cnt ) == FALSE ) {
            continue;
        }
        if ( switch_range->port_cnt > 1 ) {
            if ( _port_in_range( end_uport,
                                 g_port_range.switch_range[i].begin_uport,
                                 g_port_range.switch_range[i].port_cnt ) == FALSE ) {
                continue;
            }
        }
        /* in range, get data */
        switch_range->port_type   = g_port_range.switch_range[i].port_type;
        switch_range->switch_id   = g_port_range.switch_range[i].switch_id;
        switch_range->begin_port  = g_port_range.switch_range[i].begin_port +
                                    switch_range->begin_uport -
                                    g_port_range.switch_range[i].begin_uport;
        switch_range->isid        = g_port_range.switch_range[i].isid;
        switch_range->begin_iport = g_port_range.switch_range[i].begin_iport +
                                    switch_range->begin_uport -
                                    g_port_range.switch_range[i].begin_uport;
        return TRUE;
    }

    /* not in range */
    return FALSE;
}

/*
    get switch range from isid and iport

    INPUT
        switch_range->isid        : isid
        switch_range->begin_iport : iport
        switch_range->port_cnt    : number of ports

    OUTPUT
        switch_range

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_port_from_isid_iport(
    INOUT icli_switch_port_range_t  *switch_range
)
{
    u32     i;
    u32     end_iport;

    if ( switch_range == NULL ) {
        T_E("switch_range == NULL\n");
        return FALSE;
    }

    if ( switch_range->port_cnt == 0 ) {
        T_E("port_cnt == 0\n");
        return FALSE;
    }

    _port_range_update();

    end_iport = switch_range->begin_iport + switch_range->port_cnt - 1;
    for ( i = 0; i < g_port_range.cnt; ++i ) {
        if ( g_port_range.switch_range[i].isid != switch_range->isid ) {
            continue;
        }
        if ( _port_in_range( switch_range->begin_iport,
                             g_port_range.switch_range[i].begin_iport,
                             g_port_range.switch_range[i].port_cnt ) == FALSE ) {
            continue;
        }
        if ( switch_range->port_cnt > 1 ) {
            if ( _port_in_range( end_iport,
                                 g_port_range.switch_range[i].begin_iport,
                                 g_port_range.switch_range[i].port_cnt ) == FALSE ) {
                continue;
            }
        }
        /* in range, get data */
        switch_range->port_type   = g_port_range.switch_range[i].port_type;
        switch_range->switch_id   = g_port_range.switch_range[i].switch_id;
        switch_range->begin_port  = g_port_range.switch_range[i].begin_port +
                                    switch_range->begin_iport -
                                    g_port_range.switch_range[i].begin_iport;
        switch_range->usid        = g_port_range.switch_range[i].usid;
        switch_range->begin_uport = g_port_range.switch_range[i].begin_uport +
                                    switch_range->begin_iport -
                                    g_port_range.switch_range[i].begin_iport;
        return TRUE;
    }

    /* not in range */
    return FALSE;
}

/*
    get first type/switch_id/port

    INPUT
        n/a

    OUTPUT
        switch_range->port_type   : first port type
        switch_range->switch_id   : first switch ID
        switch_range->begin_port  : first port ID
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->isid        : isid
        switch_range->begin_iport : iport

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_port_get_first(
    OUT icli_switch_port_range_t  *switch_range
)
{
    u32     i;
    u32     min = 0;

    if ( switch_range == NULL ) {
        T_E("switch_range == NULL\n");
        return FALSE;
    }

    _port_range_update();

    for ( i = 1; i < g_port_range.cnt; ++i ) {
        if ( _port_compare(&(g_port_range.switch_range[i]), &(g_port_range.switch_range[min])) == -1 ) {
            min = i;
        }
    }

    *switch_range = g_port_range.switch_range[min];
    switch_range->port_cnt = 1;
    return TRUE;
}

/*
    get from switch_id/type/port for only 1 port

    INPUT
        index:
            switch_range->port_type   : port type
            switch_range->switch_id   : switch ID
            switch_range->begin_port  : port ID

    OUTPUT
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->isid        : isid
        switch_range->begin_iport : iport

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_port_get(
    INOUT icli_switch_port_range_t  *switch_range
)
{
    u32     i;

    if ( switch_range == NULL ) {
        T_E("switch_range == NULL\n");
        return FALSE;
    }

    _port_range_update();

    for ( i = 0; i < g_port_range.cnt; ++i ) {
        // same switch id
        if ( g_port_range.switch_range[i].switch_id != switch_range->switch_id ) {
            continue;
        }

        // same port type
        if ( g_port_range.switch_range[i].port_type != switch_range->port_type ) {
            continue;
        }

        // port in range
        if ( _port_in_range( switch_range->begin_port,
                             g_port_range.switch_range[i].begin_port,
                             g_port_range.switch_range[i].port_cnt ) == FALSE ) {
            continue;
        }

        /* in range, get data */
        switch_range->port_cnt    = 1;
        switch_range->usid        = g_port_range.switch_range[i].usid;
        switch_range->begin_uport = g_port_range.switch_range[i].begin_uport +
                                    switch_range->begin_port -
                                    g_port_range.switch_range[i].begin_port;
        switch_range->isid        = g_port_range.switch_range[i].isid;
        switch_range->begin_iport = g_port_range.switch_range[i].begin_iport +
                                    switch_range->begin_port -
                                    g_port_range.switch_range[i].begin_port;
        return TRUE;
    }

    /* not in range */
    return FALSE;
}

/*
    get next switch_id/type/port

    INPUT
        index:
            switch_range->switch_id   : switch ID
            switch_range->port_type   : port type
            switch_range->begin_port  : port ID

    OUTPUT
        switch_range->switch_id   : next switch ID
        switch_range->port_type   : next port type
        switch_range->begin_port  : next port ID
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->isid        : isid
        switch_range->begin_iport : iport

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_port_get_next(
    INOUT icli_switch_port_range_t  *switch_range
)
{
    u32     i;
    u16     port_type;
    u16     switch_id;
    u16     begin_port;

    if ( switch_range == NULL ) {
        T_E("switch_range == NULL\n");
        return FALSE;
    }

    _port_range_update();

    switch_id  = switch_range->switch_id;
    port_type  = switch_range->port_type;
    begin_port = switch_range->begin_port;

    for ( ; switch_id < ICLI_MAX_SWITCH_ID; ++switch_id ) {
        for ( ; port_type < ICLI_PORT_TYPE_MAX; ++port_type ) {
            for ( begin_port++; begin_port < ICLI_MAX_PORT_ID; ++begin_port ) {
                for ( i = 0; i < g_port_range.cnt; ++i ) {
                    // same switch ID
                    if ( g_port_range.switch_range[i].switch_id != switch_id ) {
                        continue;
                    }
                    // same port type
                    if ( g_port_range.switch_range[i].port_type != port_type ) {
                        continue;
                    }
                    // port in range
                    if ( _port_in_range( begin_port,
                                         g_port_range.switch_range[i].begin_port,
                                         g_port_range.switch_range[i].port_cnt ) ) {
                        switch_range->port_type   = g_port_range.switch_range[i].port_type;
                        switch_range->switch_id   = g_port_range.switch_range[i].switch_id;
                        switch_range->begin_port  = begin_port;
                        switch_range->port_cnt    = 1;
                        switch_range->usid        = g_port_range.switch_range[i].usid;
                        switch_range->begin_uport = g_port_range.switch_range[i].begin_uport +
                                                    begin_port -
                                                    g_port_range.switch_range[i].begin_port;
                        switch_range->isid        = g_port_range.switch_range[i].isid;
                        switch_range->begin_iport = g_port_range.switch_range[i].begin_iport +
                                                    begin_port -
                                                    g_port_range.switch_range[i].begin_port;
                        return TRUE;
                    }
                }
            }
            begin_port = 0;
        }
        port_type = 0;
    }
    return FALSE;
}

/*
    check if a specific switch port is defined or not

    INPUT
        port_type : port type
        switch_id : switch id
        port_id   : port id

    OUTPUT
        n/a

    RETURN
        TRUE  : the specific switch port is valid
        FALSE : not valid

    COMMENT
        n/a
*/
BOOL vtss_icli_port_switch_port_valid(
    IN u16      port_type,
    IN u16      switch_id,
    IN u16      port_id
)
{
    u32     i;

    _port_range_update();

    for ( i = 0; i < g_port_range.cnt; ++i ) {
        if ( port_type != g_port_range.switch_range[i].port_type ) {
            continue;
        }
        if ( switch_id != g_port_range.switch_range[i].switch_id ) {
            continue;
        }
        if ( _port_in_range( port_id,
                             g_port_range.switch_range[i].begin_port,
                             g_port_range.switch_range[i].port_cnt) ) {
            /* in range */
            return TRUE;
        }
    }

    /* not in range */
    return FALSE;
}

/*
    check if the switch port range is valid or not

    INPUT
        switch_port : the followings are checking parameters
            switch_port->port_type
            switch_port->switch_id
            switch_port->begin_port
            switch_port->port_cnt

    OUTPUT
        switch_id, port_id : the port is not valid
                             put NULL if these are not needed

    RETURN
        TRUE  : all the switch ports are valid
        FALSE : at least one of switch port is not valid

    COMMENT
        n/a
*/
BOOL vtss_icli_port_switch_range_valid(
    IN  icli_switch_port_range_t    *switch_port,
    OUT u16                         *switch_id,
    OUT u16                         *port_id
)
{
    u16     port;
    u16     end_port;

    if ( switch_port == NULL ) {
        T_E("switch_port == NULL");
        return FALSE;
    }

    _port_range_update();

    end_port = switch_port->begin_port + switch_port->port_cnt - 1;
    for ( port = switch_port->begin_port; port <= end_port; ++port ) {
        if ( vtss_icli_port_switch_port_valid(switch_port->port_type, switch_port->switch_id, port) == FALSE ) {
            if ( switch_id && port_id ) {
                *switch_id = switch_port->switch_id;
                *port_id   = port;
            }
            return FALSE;
        }
    }
    return TRUE;
}

/*
    check if all the stack switch port range is defined or not

    INPUT
        stack_port : switch port range in stack

    OUTPUT
        switch_id, port_id : the port is not valid
                             put NULL if these are not needed

    RETURN
        TRUE  : all the switch ports in stack are valid
        FALSE : at least one of switch port is not valid

    COMMENT
        n/a
*/
BOOL vtss_icli_port_stack_range_valid(
    IN  icli_stack_port_range_t     *stack_port,
    OUT u16                         *switch_id,
    OUT u16                         *port_id
)
{
    u32     i;

    if ( stack_port == NULL ) {
        T_E("stack_port == NULL");
        return FALSE;
    }

    for ( i = 0; i < stack_port->cnt; ++i ) {
        if ( vtss_icli_port_switch_range_valid(&(stack_port->switch_range[i]), switch_id, port_id) == FALSE ) {
            return FALSE;
        }
    }
    return TRUE;
}

/*
    get valid port range for the port type

    INPUT
        port_type : the port type
        range     : port range to check

    OUTPUT
        range     : valid port range
        switch_id : if invalid, then this is the invalid switch ID
        port_id   : if invalid, then this is the invalid port ID

    RETURN
        TRUE  : there are some valid port ranges
        FALSE : no one is valid

    COMMENT
        if any ICLI_PORT_TYPE_ALL inside, the it is valid if one of range is valid
        if no ICLI_PORT_TYPE_ALL, them they are specific ranges and they all must be valid.
*/
BOOL vtss_icli_port_type_list_get(
    IN    icli_port_type_t          port_type,
    INOUT icli_stack_port_range_t   *range,
    OUT   u16                       *switch_id,
    OUT   u16                       *port_id
)
{
    u32                         i;
    u32                         cnt;
    icli_switch_port_range_t    *switch_range;
    icli_stack_port_range_t     *result;
    BOOL                        b_wildcard;
    BOOL                        b_get;
    BOOL                        b_port_id;

    if ( range == NULL ) {
        T_E("range == NULL\n");
        return FALSE;
    }

    if ( port_type >= ICLI_PORT_TYPE_MAX ) {
        T_E("invalid port type %d\n", port_type);
        return FALSE;
    }

    result = (icli_stack_port_range_t *)icli_malloc( sizeof(icli_stack_port_range_t) );
    if ( result == NULL ) {
        T_E("Failed to malloc\n");
        return FALSE;
    }
    memset( result, 0, sizeof(icli_stack_port_range_t) );

    b_wildcard = FALSE;
    b_get      = FALSE;
    b_port_id  = FALSE;

    cnt = range->cnt;
    for ( i = 0; i < cnt; ++i ) {
        /* get switch_range */
        switch_range = &( range->switch_range[i] );

        if ( switch_range->port_cnt == 0 ) {
            T_E("port count is 0 in range[%d] \n", i);
            icli_free( result );
            return FALSE;
        }

        if ( port_type == ICLI_PORT_TYPE_ALL                  ||
             switch_range->switch_id == ICLI_SWITCH_PORT_ALL  ||
             switch_range->begin_port == ICLI_SWITCH_PORT_ALL ) {
            b_wildcard = TRUE;
            if ( _switch_range_wildcard_get(port_type, switch_range, result) ) {
                b_get = TRUE;
            } else {
                if ( b_port_id == FALSE && port_id ) {
                    b_port_id = TRUE;
                    *port_id = (u16)i;
                }
            }
        } else {
            if ( _switch_range_get(port_type, switch_range, result, switch_id, port_id) ) {
                b_get = TRUE;
            } else {
                icli_free( result );
                return FALSE;
            }
        }
    } // for range cnt

    if ( b_wildcard ) {
        if ( switch_id ) {
            *switch_id = ICLI_SWITCH_PORT_ALL;
        }
        if ( b_get == FALSE ) {
            icli_free( result );
            return FALSE;
        }
    }

    *range = *result;
    icli_free( result );
    return TRUE;
}

/*
    add src into dst by sort

    INPUT
        src : source port list

    OUTPUT
        dst : destination port list

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_port_type_list_add(
    IN  icli_stack_port_range_t   *src,
    OUT icli_stack_port_range_t   *dst
)
{
    u32     i;

    if ( src == NULL ) {
        T_E("src == NULL\n");
        return FALSE;
    }

    if ( dst == NULL ) {
        T_E("dst == NULL\n");
        return FALSE;
    }

    for ( i = 0; i < src->cnt; i++ ) {
        if ( _switch_range_get(src->switch_range[i].port_type, &(src->switch_range[i]), dst, NULL, NULL) == FALSE ) {
            T_E("_switch_range_get()\n");
            return FALSE;
        }
    }
    return TRUE;
}

/*
    get switch_id

    INPUT
        index:
            switch_range->switch_id : switch ID

    OUTPUT
        switch_range->switch_id
        switch_range->usid
        switch_range->isid
        others = 0

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_switch_get(
    INOUT icli_switch_port_range_t  *switch_range
)
{
    icli_switch_port_range_t    spr;
    BOOL                        b;

    if ( switch_range == NULL ) {
        T_E("switch_range == NULL\n");
        return FALSE;
    }

    b = FALSE;

    if ( vtss_icli_port_get_first(&spr) ) {
        if ( spr.switch_id == switch_range->switch_id ) {
            b = TRUE;
        } else {
            while ( vtss_icli_port_get_next(&spr) ) {
                if ( spr.switch_id == switch_range->switch_id ) {
                    memset(switch_range, 0, sizeof(icli_switch_port_range_t));
                    switch_range->switch_id = spr.switch_id;
                    switch_range->usid      = spr.usid;
                    switch_range->isid      = spr.isid;
                    b = TRUE;
                    break;
                }
            }
        }
    }

    return b;
}

/*
    get next switch_id

    INPUT
        index:
            switch_range->switch_id : switch ID

    OUTPUT
        switch_range->switch_id : next switch ID
        switch_range->usid      : next usid
        switch_range->isid      : next isid
        others = 0

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_switch_get_next(
    INOUT icli_switch_port_range_t  *switch_range
)
{
    icli_switch_port_range_t    spr;
    BOOL                        b;

    if ( switch_range == NULL ) {
        T_E("switch_range == NULL\n");
        return FALSE;
    }

    b = FALSE;

    memset(&spr, 0, sizeof(spr));
    spr.switch_id = switch_range->switch_id;

    while ( vtss_icli_port_get_next(&spr) ) {
        if ( spr.switch_id > switch_range->switch_id ) {
            // get output
            memset(switch_range, 0, sizeof(icli_switch_port_range_t));
            switch_range->switch_id = spr.switch_id;
            switch_range->usid      = spr.usid;
            switch_range->isid      = spr.isid;
            // return TRUE
            b = TRUE;
            break;
        }
    }

    return b;
}

/*
    get from switch_id/type/port

    INPUT
        index :
            switch_range->switch_id   : switch ID
            switch_range->port_type   : port type
            switch_range->begin_port  : port ID

    OUTPUT
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->isid        : isid
        switch_range->begin_iport : iport

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_switch_port_get(
    INOUT icli_switch_port_range_t  *switch_range
)
{
    return vtss_icli_port_get( switch_range );
}

/*
    get next switch_id/type/port

    INPUT
        index:
            switch_range->switch_id   : switch ID (not changed)
            switch_range->port_type   : port type
            switch_range->begin_port  : port ID

    OUTPUT
        switch_range->switch_id   : current switch ID (not changed)
        switch_range->port_type   : next port type
        switch_range->begin_port  : next port ID
        switch_range->usid        : usid
        switch_range->begin_uport : uport
        switch_range->isid        : isid
        switch_range->begin_iport : iport

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_switch_port_get_next(
    INOUT icli_switch_port_range_t  *switch_range
)
{
    icli_switch_port_range_t    spr;
    BOOL                        b;

    if ( switch_range == NULL ) {
        T_E("switch_range == NULL\n");
        return FALSE;
    }

    b = FALSE;

    memcpy(&spr, switch_range, sizeof(spr));

    while ( vtss_icli_port_get_next(&spr) ) {
        if ( spr.switch_id == switch_range->switch_id ) {
            // get output
            *switch_range = spr;
            // return TRUE
            b = TRUE;
            break;
        }
    }

    return b;
}

/*
    get password of privilege level

    INPUT
        priv : the privilege level

    OUTPUT
        password : the corresponding password with size (ICLI_PASSWORD_MAX_LEN + 1)

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_enable_password_get(
    IN  icli_privilege_t    priv,
    OUT char                *password
)
{
    if ( priv >= ICLI_PRIVILEGE_MAX ) {
        T_E("invalid privilege %u\n", priv);
        return FALSE;
    }

    if ( password == NULL ) {
        T_E("password == NULL\n");
        return FALSE;
    }

    (void)vtss_icli_str_cpy(password, g_conf_data.enable_password[priv]);
    return TRUE;
}

/*
    set password of privilege level

    INPUT
        priv     : the privilege level
        password : the corresponding password with size (ICLI_PASSWORD_MAX_LEN + 1)

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_enable_password_set(
    IN  icli_privilege_t    priv,
    IN  char                *password
)
{
    if ( priv >= ICLI_PRIVILEGE_MAX ) {
        T_E("invalid privilege %u\n", priv);
        return FALSE;
    }

    if ( password == NULL ) {
        memset(g_conf_data.enable_password[priv], 0, ICLI_PASSWORD_MAX_LEN + 1);
        return TRUE;
    }

    if ( vtss_icli_str_len(password) > ICLI_PASSWORD_MAX_LEN ) {
        T_E("the length of password is too long\n");
        return FALSE;
    }

    g_conf_data.b_enable_secret[priv] = FALSE;
    (void)vtss_icli_str_cpy(g_conf_data.enable_password[priv], password);
    return TRUE;
}

/*
    verify clear password of privilege level is correct or not
    according to password or secret

    INPUT
        priv           : the privilege level
        clear_password : the corresponding password with size (ICLI_PASSWORD_MAX_LEN + 1)

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_enable_password_verify(
    IN  icli_privilege_t    priv,
    IN  char                *clear_password
)
{
    u8      mac[ICLI_HMAC_MD5_MAX_LEN];
    char    str[ICLI_PASSWORD_MAX_LEN + 1];

    if ( priv >= ICLI_PRIVILEGE_MAX ) {
        T_E("invalid privilege %u\n", priv);
        return FALSE;
    }

    if ( clear_password == NULL ) {
        T_E("clear_password == NULL\n");
        return FALSE;
    }

    if ( g_conf_data.b_enable_secret[priv] ) {
        // get mac
        icli_hmac_md5(g_enable_key, g_enable_key_len, (u8 *)clear_password, vtss_icli_str_len(clear_password), mac);

        // translate into string
        _mac_to_string(mac, str);

        if ( vtss_icli_str_cmp(str, g_conf_data.enable_password[priv] ) == 0 ) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        if ( vtss_icli_str_cmp(clear_password, g_conf_data.enable_password[priv] ) == 0 ) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
}

/*
    set secret of privilege level

    INPUT
        priv   : the privilege level
        secret : the corresponding secret with size (ICLI_PASSWORD_MAX_LEN + 1)

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_enable_secret_set(
    IN  icli_privilege_t    priv,
    IN  char                *secret
)
{
    if ( priv >= ICLI_PRIVILEGE_MAX ) {
        T_E("invalid privilege %u\n", priv);
        return FALSE;
    }

    if ( secret == NULL ) {
        memset(g_conf_data.enable_password[priv], 0, ICLI_PASSWORD_MAX_LEN + 1);
        return TRUE;
    }

    if ( vtss_icli_str_len(secret) > ICLI_PASSWORD_MAX_LEN ) {
        T_E("the length of secret is too long\n");
        return FALSE;
    }

    g_conf_data.b_enable_secret[priv] = TRUE;
    (void)vtss_icli_str_cpy(g_conf_data.enable_password[priv], secret);
    return TRUE;
}

/*
    translate clear password of privilege level to secret password

    INPUT
        priv           : the privilege level
        clear_password : the corresponding password with size (ICLI_PASSWORD_MAX_LEN + 1)

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed

    COMMENT
        n/a
*/
BOOL vtss_icli_enable_secret_set_clear(
    IN  icli_privilege_t    priv,
    IN  char                *clear_password
)
{
    u8      mac[ICLI_HMAC_MD5_MAX_LEN];

    if ( priv >= ICLI_PRIVILEGE_MAX ) {
        T_E("invalid privilege %u\n", priv);
        return FALSE;
    }

    if ( clear_password == NULL ) {
        memset(g_conf_data.enable_password[priv], 0, ICLI_PASSWORD_MAX_LEN + 1);
        return TRUE;
    }

    // get mac
    icli_hmac_md5(g_enable_key, g_enable_key_len, (u8 *)clear_password, vtss_icli_str_len(clear_password), mac);

    // translate into string
    _mac_to_string(mac, g_conf_data.enable_password[priv]);

    g_conf_data.b_enable_secret[priv] = TRUE;

    return TRUE;
}

/*
    get if the enable password is in secret or not

    INPUT
        priv : the privilege level

    OUTPUT
        n/a

    RETURN
        TRUE  : in secret
        FALSE : clear password

    COMMENT
        n/a
*/
BOOL vtss_icli_enable_password_if_secret_get(
    IN  icli_privilege_t    priv
)
{
    return g_conf_data.b_enable_secret[priv];
}

#if 1 /* CP, 2012/10/08 14:31, debug command, debug prompt */
/*
    get debug prompt without semaphore

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        g_debug_prompt

    COMMENT
        n/a
*/
char *vtss_icli_debug_prompt_get(void)
{
    return g_debug_prompt;
}

/*
    set device name shown in command prompt

    INPUT
        debug_prompt : debug prompt

    OUTPUT
        n/a

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_debug_prompt_set(
    IN  char    *debug_prompt
)
{
    if ( debug_prompt == NULL ) {
        T_E("debug_prompt == NULL\n");
        return ICLI_RC_ERR_PARAMETER;
    }

    (void)vtss_icli_str_ncpy(g_debug_prompt, debug_prompt, ICLI_NAME_MAX_LEN);
    g_debug_prompt[ICLI_NAME_MAX_LEN] = 0;
    return ICLI_RC_OK;
}
#endif
