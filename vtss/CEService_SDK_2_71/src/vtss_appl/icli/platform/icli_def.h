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
    > CP.Wang, 2011/05/10 14:39
        - create

==============================================================================
*/
#ifndef __ICLI_DEF_H__
#define __ICLI_DEF_H__
//****************************************************************************
/*
==============================================================================

    Include File

==============================================================================
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef ICLI_TARGET

#include "vtss_types.h"
#include "main_types.h"
#include "vtss_trace_api.h"

#else // ICLI_TARGET

/*

    Note:
    The following types are used by ICLI tool that run on LINUX.
    Because when compiling for LINUX, the above include files, ex,
    vtss_types.h, main_types.h and vtss_trace_api.h, will introduce errors
    that can not be fixed for LINUX, THEREFORE, they are duplicated here.

*/

typedef signed char         i8;   /*  8-bit signed */
typedef signed short        i16;  /* 16-bit signed */
typedef signed int          i32;  /* 32-bit signed */

typedef unsigned char       u8;   /*  8-bit unsigned */
typedef unsigned short      u16;  /* 16-bit unsigned */
typedef unsigned int        u32;  /* 32-bit unsigned */

typedef unsigned char       BOOL; /* Boolean implemented as 8-bit unsigned */

//VTSS return code
typedef int                 vtss_rc;

enum {
    VTSS_RC_OK         =  0,  /**< Success */
    VTSS_RC_ERROR      = -1,  /**< Unspecified error */
    VTSS_RC_INV_STATE  = -2,  /**< Invalid state for operation */
    VTSS_RC_INCOMPLETE = -3,  /**< Incomplete result */
}; // Leave it anonymous.

/** \brief IPv4 address/mask */
typedef u32                 vtss_ip_t;

/** \brief IPv6 address/mask */
typedef struct {
    u8 addr[16];
} vtss_ipv6_t;

/** \brief MAC Address */
typedef struct {
    u8 addr[6];   /* Network byte order */
} vtss_mac_t;

#endif // ICLI_TARGET

/*
==============================================================================

    Constant

==============================================================================
*/
#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef NULL
#define NULL    0
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef INOUT
#define INOUT
#endif

/*
==============================================================================

    ICLI spec

==============================================================================
*/
/* + 512: 2013/03/11 10:21 */
#define ICLI_CMD_CNT                    2048
#define ICLI_SESSION_CNT                17
#define ICLI_SESSION_DEFAULT_CNT        17
#define ICLI_SESSION_ID_NONE            0xFFffFFff
#define ICLI_CMD_WORD_CNT               512
#define ICLI_RANGE_LIST_CNT             128
#define ICLI_HISTORY_CMD_CNT            32
#define ICLI_PARAMETER_MEM_CNT          512
#define ICLI_NODE_PROPERTY_CNT          4096
#define ICLI_RUNTIME_CNT                ICLI_SESSION_CNT
#define ICLI_RANDOM_OPTIONAL_DEPTH      3
#define ICLI_RANDOM_OPTIONAL_CNT        ICLI_CMD_WORD_CNT
#define ICLI_BYWORD_MAX_LEN             80
#define ICLI_HELP_MAX_LEN               256
#define ICLI_CWORD_MAX_CNT              256

#define ICLI_USERNAME_MAX_LEN           32
#define ICLI_PASSWORD_MAX_LEN           32
#define ICLI_AUTH_MAX_CNT               5

#define ICLI_DEFAULT_PRIVILEGED_LEVEL   2

#define ICLI_STR_MAX_LEN                4096
#define ICLI_PUT_MAX_LEN                ICLI_STR_MAX_LEN
#define ICLI_ERR_MAX_LEN                512
#define ICLI_FILE_MAX_LEN               256
#define ICLI_NAME_MAX_LEN               32
#define ICLI_PROMPT_MAX_LEN             32
#define ICLI_DIGIT_MAX_LEN              64
#define ICLI_VALUE_STR_MAX_LEN          256
#define ICLI_DEFAULT_WAIT_TIME          (10 * 60)  //in seconds
#define ICLI_MODE_MAX_LEVEL             5
#define ICLI_BANNER_MAX_LEN             255
#define ICLI_DEV_NAME_MAX_LEN           255

#define ICLI_HOSTNAME_MAX_LEN           45

#define ICLI_DEFAULT_WIDTH              80
#define ICLI_MAX_WIDTH                  0x0fFFffFF
#define ICLI_MIN_WIDTH                  40

#define ICLI_DEFAULT_LINES              24
#define ICLI_MIN_LINES                  3

#define ICLI_MIN_VLAN_ID                1
#define ICLI_MAX_VLAN_ID                4095

#define ICLI_MIN_SWICTH_ID              1
#define ICLI_MAX_SWICTH_ID              16

#define ICLI_MAX_YEAR                   2037
#define ICLI_MIN_YEAR                   1970
#define ICLI_YEAR_MAX_MONTH             12
#define ICLI_YEAR_MIN_MONTH             1

#define ICLI_LOCATION_MAX_LEN           32

#define ICLI_HMAC_MD5_MAX_LEN           16

#define ICLI_DEBUG_PRIVI_CMD            "_debug_privilege_"

/*
    default properties of enable privilege password
*/
#define ICLI_DEFAULT_ENABLE_PASSWORD    "enable"
#define ICLI_ENABLE_PASSWORD_RETRY      3
#define ICLI_ENABLE_PASSWORD_WAIT       (30 * 1000) // milli-second

/* default device name */
#define ICLI_DEFAULT_DEVICE_NAME        ""

/*
    the following syntax symbol is used to customize the command
    reference guide.
*/
#define ICLI_HTM_MANDATORY_BEGIN        '{'
#define ICLI_HTM_MANDATORY_END          '}'
#define ICLI_HTM_LOOP_BEGIN             '('
#define ICLI_HTM_LOOP_END               ')'
#define ICLI_HTM_OPTIONAL_BEGIN         '['
#define ICLI_HTM_OPTIONAL_END           ']'
#define ICLI_HTM_VARIABLE_BEGIN         '<'
#define ICLI_HTM_VARIABLE_END           '>'

/*
    variable input style
*/
#define ICLI_VARIABLE_STYLE             0

/*
    support random optional or not
*/
#define ICLI_RANDOM_OPTIONAL            1

/*
    support random optional must number '}*1'
*/
#define ICLI_RANDOM_MUST_NUMBER         1
#define ICLI_RANDOM_MUST_CNT            32

/*
    wildcard symbol for interface port list
*/
#define ICLI_INTERFACE_WILDCARD         "*"

#define ICLI_MAX_SWITCH_ID              33
#define ICLI_MAX_PORT_ID                65

/*
    support privilege per command
*/
#define ICLI_PRIV_CMD_MAX_LEN           128
#define ICLI_PRIV_CMD_MAX_CNT           64

/*
    Command mode
*/
typedef enum {
    ICLI_CMD_MODE_EXEC,
    ICLI_CMD_MODE_GLOBAL_CONFIG,
    ICLI_CMD_MODE_CONFIG_VLAN,
    ICLI_CMD_MODE_INTERFACE_PORT_LIST,
    ICLI_CMD_MODE_INTERFACE_VLAN,
    ICLI_CMD_MODE_CONFIG_LINE,
    ICLI_CMD_MODE_IPMC_PROFILE,
    ICLI_CMD_MODE_SNMPS_HOST,
    ICLI_CMD_MODE_STP_AGGR,
    ICLI_CMD_MODE_DHCP_POOL,
    ICLI_CMD_MODE_RFC2544_PROFILE,
    //------- add above
    ICLI_CMD_MODE_MAX
} icli_cmd_mode_t;

/*
    Privilege
*/
typedef enum {
    ICLI_PRIVILEGE_0,
    ICLI_PRIVILEGE_1,
    ICLI_PRIVILEGE_2,
    ICLI_PRIVILEGE_3,
    ICLI_PRIVILEGE_4,
    ICLI_PRIVILEGE_5,
    ICLI_PRIVILEGE_6,
    ICLI_PRIVILEGE_7,
    ICLI_PRIVILEGE_8,
    ICLI_PRIVILEGE_9,
    ICLI_PRIVILEGE_10,
    ICLI_PRIVILEGE_11,
    ICLI_PRIVILEGE_12,
    ICLI_PRIVILEGE_13,
    ICLI_PRIVILEGE_14,
    ICLI_PRIVILEGE_15,
    //------- add above
    ICLI_PRIVILEGE_DEBUG,
    ICLI_PRIVILEGE_MAX,
} icli_privilege_t;

/*
==============================================================================

    Macro Definition

==============================================================================
*/
#define icli_sprintf        sprintf
#define icli_snprintf       snprintf
#define icli_vsnprintf      vsnprintf

#ifdef ICLI_TARGET
#define icli_malloc(_sz_)   VTSS_MALLOC_MODID(VTSS_MODULE_ID_ICLI, _sz_)
#define icli_free           VTSS_FREE
#else
#define icli_malloc         malloc
#define icli_free           free
#endif

/*
==============================================================================

    Type

==============================================================================
*/

//****************************************************************************
#endif //__ICLI_DEF_H__

