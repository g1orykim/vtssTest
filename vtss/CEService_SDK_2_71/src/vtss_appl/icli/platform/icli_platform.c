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
    > CP.Wang, 2011/09/16 10:16
        - create

==============================================================================
*/

/*
==============================================================================

    Include File

==============================================================================
*/
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "vtss_icli.h"
#include "icli_platform.h"
#include "icli_porting_trace.h"

#ifdef ICLI_TARGET

#ifdef VTSS_SW_OPTION_AUTH
#include "vtss_auth_api.h"
#endif

#ifdef VTSS_SW_OPTION_MD5
#include "vtss_md5_api.h"
#endif

#include "msg_api.h"
#include "led_api.h"
#include "misc_api.h"
#include "topo_api.h"

#endif // ICLI_TARGET

/*
==============================================================================

    Constant and Macro

==============================================================================
*/
#define _MAX_STR_BUF_SIZE       128

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
/* modify as icli_cmd_mode_t */
static const icli_cmd_mode_info_t g_cmd_mode_info[ICLI_CMD_MODE_MAX] = {
    {
        ICLI_CMD_MODE_EXEC,
        "ICLI_CMD_MODE_EXEC",
        "User EXEC Mode",
        "",
        "",
        FALSE
    },
    {
        ICLI_CMD_MODE_GLOBAL_CONFIG,
        "ICLI_CMD_MODE_GLOBAL_CONFIG",
        "Global Configuration Mode",
        "config",
        "configure terminal",
        FALSE
    },
    {
        ICLI_CMD_MODE_CONFIG_VLAN,
        "ICLI_CMD_MODE_CONFIG_VLAN",
        "VLAN Configuration Mode",
        "config-vlan",
        "vlan <vlan_list>",
        TRUE
    },
    {
        ICLI_CMD_MODE_INTERFACE_PORT_LIST,
        "ICLI_CMD_MODE_INTERFACE_PORT_LIST",
        "Port List Interface Mode",
        "config-if",
        "interface <port_type> <port_type_list>",
        TRUE
    },
    {
        ICLI_CMD_MODE_INTERFACE_VLAN,
        "ICLI_CMD_MODE_INTERFACE_VLAN",
        "VLAN Interface Mode",
        "config-if-vlan",
        "interface vlan <vlan_list>",
        TRUE
    },
    {
        ICLI_CMD_MODE_CONFIG_LINE,
        "ICLI_CMD_MODE_CONFIG_LINE",
        "Line Configuration Mode",
        "config-line",
        "line { <0~16> | console 0 | vty <0~15> }",
        TRUE
    },
    {
        ICLI_CMD_MODE_IPMC_PROFILE,
        "ICLI_CMD_MODE_IPMC_PROFILE",
        "IPMC Profile Mode",
        "config-ipmc-profile",
        "ipmc profile <word16>",
        TRUE
    },
    {
        ICLI_CMD_MODE_SNMPS_HOST,
        "ICLI_CMD_MODE_SNMPS_HOST",
        "SNMP Server Host Mode",
        "config-snmps-host",
        "snmp-server host <word32>",
        TRUE
    },
    {
        ICLI_CMD_MODE_STP_AGGR,
        "ICLI_CMD_MODE_STP_AGGR",
        "STP Aggregation Mode",
        "config-stp-aggr",
        "spanning-tree aggregation",
        FALSE
    },
    {
        ICLI_CMD_MODE_DHCP_POOL,
        "ICLI_CMD_MODE_DHCP_POOL",
        "DHCP Pool Configuration Mode",
        "config-dhcp-pool",
        "ip dhcp pool <word32>",
        TRUE
    },
    {
        ICLI_CMD_MODE_RFC2544_PROFILE,
        "ICLI_CMD_MODE_RFC2544_PROFILE",
        "RFC2544 Profile Mode",
        "config-rfc2544-profile",
        "rfc2544 profile <word32>",
        TRUE
    },
};

/* modify as icli_privilege_t */
static const char *g_privilege_str[] = {
    "ICLI_PRIVILEGE_0",
    "ICLI_PRIVILEGE_1",
    "ICLI_PRIVILEGE_2",
    "ICLI_PRIVILEGE_3",
    "ICLI_PRIVILEGE_4",
    "ICLI_PRIVILEGE_5",
    "ICLI_PRIVILEGE_6",
    "ICLI_PRIVILEGE_7",
    "ICLI_PRIVILEGE_8",
    "ICLI_PRIVILEGE_9",
    "ICLI_PRIVILEGE_10",
    "ICLI_PRIVILEGE_11",
    "ICLI_PRIVILEGE_12",
    "ICLI_PRIVILEGE_13",
    "ICLI_PRIVILEGE_14",
    "ICLI_PRIVILEGE_15",
    "ICLI_PRIVILEGE_DEBUG",
};

/*
==============================================================================

    Static Function

==============================================================================
*/
#define _SEQUENCE_INDICATOR     91  // [
#define _CURSOR_UP              65  // A
#define _CURSOR_DOWN            66  // B
#define _CURSOR_FORWARD         67  // C
#define _CURSOR_BACKWARD        68  // D

#define _CSI \
    (void)vtss_icli_sutil_usr_char_put(handle, ICLI_KEY_ESC); \
    (void)vtss_icli_sutil_usr_char_put(handle, _SEQUENCE_INDICATOR);

/*
==============================================================================

    Public Function

==============================================================================
*/
/*
    get command mode info by string

    INPUT
        mode_str : string of mode

    OUTPUT
        n/a

    RETURN
        icli_cmd_mode_info_t * : successful
        NULL                   : failed

    COMMENT
        n/a
*/
const icli_cmd_mode_info_t *icli_cmd_mode_info_get_by_str(
    IN  char    *mode_str
)
{
    icli_cmd_mode_t     mode;

    for (mode = 0; mode < ICLI_CMD_MODE_MAX; ++mode ) {
        if ( vtss_icli_str_cmp(mode_str, g_cmd_mode_info[mode].str) == 0 ) {
            break;
        }
    }

    if ( mode == ICLI_CMD_MODE_MAX ) {
        return NULL;
    }

    return &g_cmd_mode_info[mode];
}

/*
    get command mode info by command mode

    INPUT
        mode : command mode

    OUTPUT
        n/a

    RETURN
        icli_cmd_mode_info_t * : successful
        NULL                   : failed

    COMMENT
        n/a
*/
const icli_cmd_mode_info_t *icli_cmd_mode_info_get_by_mode(
    IN  icli_cmd_mode_t     mode
)
{
    if ( mode >= ICLI_CMD_MODE_MAX ) {
        return NULL;
    }
    return &g_cmd_mode_info[mode];
}

/*
    get privilege by string

    INPUT
        priv_str : string of privilege

    OUTPUT
        n/a

    RETURN
        icli_privilege_t   : successful
        ICLI_PRIVILEGE_MAX : failed

    COMMENT
        n/a
*/
icli_privilege_t icli_privilege_get_by_str(
    IN char     *priv_str
)
{
    i32     priv;

    if ( vtss_icli_str_to_int(priv_str, &priv) == 0 ) {
        if ( priv >= 0 && priv < ICLI_PRIVILEGE_MAX ) {
            return priv;
        }
        return ICLI_PRIVILEGE_MAX;
    }

    for ( priv = 0; priv < ICLI_PRIVILEGE_MAX; ++priv ) {
        if ( vtss_icli_str_cmp(priv_str, g_privilege_str[priv]) == 0 ) {
            break;
        }
    }
    return priv;
}

#ifdef ICLI_TARGET

/*
    calculate MD5
*/
void icli_hmac_md5(
    IN  const unsigned char *key,
    IN  size_t              key_len,
    IN  const unsigned char *data,
    IN  size_t              data_len,
    OUT unsigned char       *mac
)
{
#ifdef VTSS_SW_OPTION_MD5
    vtss_hmac_md5(key, key_len, data, data_len, mac);
#else
    memset(mac, 0, ICLI_HMAC_MD5_MAX_LEN);
#endif
}

/*
    cursor up
*/
void icli_cursor_up(
    IN icli_session_handle_t    *handle
)
{
    _CSI;
    (void)vtss_icli_sutil_usr_char_put(handle, _CURSOR_UP);
}

/*
    cursor down
*/
void icli_cursor_down(
    IN icli_session_handle_t    *handle
)
{
    _CSI;
    (void)vtss_icli_sutil_usr_char_put(handle, _CURSOR_DOWN);
}

/*
    cursor forward
*/
void icli_cursor_forward(
    IN icli_session_handle_t    *handle
)
{
    i32     x, y;
    i32     max_x;

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

    // get max x
    max_x = handle->runtime_data.width - 1;

#if 0 /* CP_DEBUG */
    T_E("\nx = %d, y = %d\n", x, y);
#endif

    if ( x < max_x ) {
        // just go forward
        _CSI;
        (void)vtss_icli_sutil_usr_char_put(handle, _CURSOR_FORWARD);
    } else {
        // go to the beginning of next line
        icli_cursor_offset(handle, 0 - max_x, 1);
    }
}

/*
    cursor backward
*/
void icli_cursor_backward(
    IN icli_session_handle_t    *handle
)
{
    i32     x, y;

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);

#if 0 /* CP_DEBUG */
    T_E("\nx = %d, y = %d\n", x, y);
#endif

    if ( x ) {
        // just backward
        _CSI;
        (void)vtss_icli_sutil_usr_char_put(handle, _CURSOR_BACKWARD);
    } else {
        // go to the end of previous line
        icli_cursor_offset(handle, handle->runtime_data.width - 1, -1);
    }
}

/*
    cursor go to ( current_x + offser_x, current_y + offset_y )
*/
void icli_cursor_offset(
    IN icli_session_handle_t    *handle,
    IN i32                      offset_x,
    IN i32                      offset_y
)
{
    char    s[32];

#if 0 /* CP_DEBUG */
    T_E("\noffset_x = %d, offset_y = %d\n", offset_x, offset_y);
#endif

    if ( offset_x > 0 ) {
        // forward
        _CSI;
        icli_sprintf(s, "%d", offset_x);
        vtss_icli_session_str_put(handle, s);
        (void)vtss_icli_sutil_usr_char_put(handle, _CURSOR_FORWARD);
    } else if ( offset_x < 0 ) {
        // backward
        _CSI;
        icli_sprintf(s, "%d", 0 - offset_x);
        vtss_icli_session_str_put(handle, s);
        (void)vtss_icli_sutil_usr_char_put(handle, _CURSOR_BACKWARD);
    }

    if ( offset_y > 0 ) {
        // down
        _CSI;
        icli_sprintf(s, "%d", offset_y);
        vtss_icli_session_str_put(handle, s);
        (void)vtss_icli_sutil_usr_char_put(handle, _CURSOR_DOWN);
    } else if ( offset_y < 0 ) {
        // up
        _CSI;
        icli_sprintf(s, "%d", 0 - offset_y);
        vtss_icli_session_str_put(handle, s);
        (void)vtss_icli_sutil_usr_char_put(handle, _CURSOR_UP);
    }
}

/*
    cursor backward and delete that char
    update cursor_pos, but not cmd_pos and cmd_len
*/
void icli_cursor_backspace(
    IN  icli_session_handle_t   *handle
)
{
    i32     x, y;

    icli_cursor_backward(handle);
    ICLI_PUT_SPACE;

    /*
        if just go backward when at the end of postion,
        then the cursor will go up one line.
        the root cause is if put char at the end of position
        then the cursor is still there and will not go to next line.
    */

    // get current x,y
    vtss_icli_sutil_current_xy_get(handle, &x, &y);
    if ( x == 0 ) {
        ICLI_PUT_SPACE;
        ICLI_PUT_BACKSPACE;
    }

    icli_cursor_backward(handle);

    _DEC_1( handle->runtime_data.cursor_pos );
}

/*
    get switch information
*/
void icli_switch_info_get(
    OUT icli_switch_info_t  *switch_info
)
{
    switch_info->b_master = msg_switch_is_master();
    switch_info->usid     = led_usid_get();
}

/*
    check if auth callback exists

    INPUT
        n/a

    OUTPUT
        n/a

    RETURN
        TRUE  - yes, the callback exists
        FALSE - no, it is NULL

*/
BOOL icli_has_user_auth(
    void
)
{
#ifdef VTSS_SW_OPTION_AUTH
    return TRUE;
#else
    return FALSE;
#endif
}

/*
    authenticate user

    INPUT
        session_way  : way to access session
        username     : user name
        password     : password for the user

    OUTPUT
        privilege    : the privilege level for the authenticated user

    RETURN
        icli_rc_t
*/
i32 icli_user_auth(
    IN  icli_session_way_t  session_way,
    IN  char                *username,
    IN  char                *password,
    OUT i32                 *privilege
)
{
#ifdef VTSS_SW_OPTION_AUTH
    vtss_auth_agent_t   agent;

    switch (session_way) {
    case ICLI_SESSION_WAY_CONSOLE:
        agent = VTSS_AUTH_AGENT_CONSOLE;
        break;

    case ICLI_SESSION_WAY_TELNET:
        agent = VTSS_AUTH_AGENT_TELNET;
        break;

    case ICLI_SESSION_WAY_SSH:
        agent = VTSS_AUTH_AGENT_SSH;
        break;

    case ICLI_SESSION_WAY_APP_EXEC:
        *privilege = ICLI_PRIVILEGE_15;
        return ICLI_RC_OK;

    default:
        T_E("Invalid session way = %d\n", session_way);
        return ICLI_RC_ERR_PARAMETER;
    }

    if ( vtss_authenticate(agent, username, password, (int *)privilege) == VTSS_OK ) {
        return ICLI_RC_OK;
    }

    return ICLI_RC_ERROR;
#else
    if ( session_way ) {}
    if ( username ) {}
    if ( password ) {}

    if ( privilege ) {
        *privilege = ICLI_PRIVILEGE_DEBUG - 1;
    }

    return ICLI_RC_OK;
#endif
}

u16 icli_isid2usid(
    IN u16  isid
)
{
    return (u16)( topo_isid2usid((vtss_isid_t)isid) );
}

u16 icli_iport2uport(
    IN u16  iport
)
{
    return (u16)( iport2uport((vtss_port_no_t)iport) );
}

/*
    get switch_id by usid

    INPUT
        usid

    OUTPUT
        n/a

    RETURN
        switch ID

    COMMENT
        n/a
*/
u16 icli_usid2switchid(
    IN u16  usid
)
{
    return usid;
}

/*
    get switch_id by isid

    INPUT
        isid

    OUTPUT
        n/a

    RETURN
        switch ID

    COMMENT
        n/a
*/
u16 icli_isid2switchid(
    IN u16  isid
)
{
    u16     usid;

    usid = icli_isid2usid( isid );

    return icli_usid2switchid( usid );
}

/*
    get mode prompt

    INPUT
        mode : command mode

    OUTPUT
        n/a

    RETURN
        mode prompt
*/
char *icli_mode_prompt_get(
    IN  icli_cmd_mode_t     mode
)
{
    if ( mode >= ICLI_CMD_MODE_MAX ) {
        return "";
    }
    return ( g_cmd_mode_info[mode].prompt );
}

#endif // ICLI_TARGET
