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
    > CP.Wang, 05/29/2013 13:08
        - create

==============================================================================
*/
#ifndef __VTSS_ICLI_TYPE_H__
#define __VTSS_ICLI_TYPE_H__
//****************************************************************************
/*
==============================================================================

    Include File

==============================================================================
*/
#include "icli_def.h"

/*
==============================================================================

    Constant and Macro

==============================================================================
*/
#define ICLI_MIN_INT8               (0-128)
#define ICLI_MAX_INT8               0x0000007F

#define ICLI_MIN_INT16              (0-65536)
#define ICLI_MAX_INT16              0x00007Fff

#define ICLI_MIN_INT                (-1-2147483647)
#define ICLI_MAX_INT                0x7FffFFff

#define ICLI_MIN_UINT8              0x0
#define ICLI_MAX_UINT8              0x000000FF

#define ICLI_MIN_UINT16             0x0
#define ICLI_MAX_UINT16             0x0000FFff

#define ICLI_MIN_UINT               0x0
#define ICLI_MAX_UINT               0xFFffFFff

#define ICLI_OUI_SIZE               3
#define ICLI_CLOCK_ID_SIZE          8

#define ICLI_HEXVAL_MAX_LEN         (ICLI_VALUE_STR_MAX_LEN / 2)

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
#define ICLI_SWITCH_PORT_ALL        0xFFff
#endif

/*
==============================================================================

    Type Definition

==============================================================================
*/
typedef struct {
    vtss_ip_t   ip;
    vtss_ip_t   netmask;
} icli_ipv4_subnet_t;

typedef struct {
    vtss_ipv6_t     ip;
    vtss_ipv6_t     netmask;
} icli_ipv6_subnet_t;

typedef struct {
    u8          hour;
    u8          min;
    u8          sec;
    u8          resv;
} icli_time_t;

typedef struct {
    u16         year;
    u8          month;
    u8          day;
} icli_date_t;

typedef struct {
    u8          mac[ICLI_OUI_SIZE];
    u8          resv;
} icli_oui_t;

typedef struct {
    u8          id[ICLI_CLOCK_ID_SIZE];
} icli_clock_id_t;

typedef struct {
    u32         len;
    u8          hex[ICLI_HEXVAL_MAX_LEN];
} icli_hexval_t;

typedef enum {
    /* successful code */
    ICLI_RC_OK = 0,

    /* failed code */
    ICLI_RC_ERROR           = -1000,
    ICLI_RC_ERR_PARAMETER   = -999,
    ICLI_RC_ERR_MATCH       = -998,
    ICLI_RC_ERR_AMBIGUOUS   = -997,
    ICLI_RC_ERR_INCOMPLETE  = -996,
    ICLI_RC_ERR_EXCESSIVE   = -995,
    ICLI_RC_ERR_MORE        = -994,
    ICLI_RC_ERR_EMPTY       = -993,
    ICLI_RC_ERR_MEMORY      = -992,
    ICLI_RC_ERR_OVERFLOW    = -991,
    ICLI_RC_ERR_SEMA        = -990,
    ICLI_RC_ERR_EXPIRED     = -989,
    ICLI_RC_ERR_FKEY        = -988,
    ICLI_RC_ERR_THREAD      = -987,
    ICLI_RC_ERR_REDO        = -986,
    ICLI_RC_ERR_BYPASS      = -985,
    ICLI_RC_ERR_RANGE       = -984,
} icli_rc_t;

typedef struct {
    u32     cnt;
    struct {
        int     min;
        int     max;
    } range[ICLI_RANGE_LIST_CNT];
} icli_signed_range_t;

typedef struct {
    u32     cnt;
    struct {
        unsigned int     min;
        unsigned int     max;
    } range[ICLI_RANGE_LIST_CNT];
} icli_unsigned_range_t;

typedef enum {
    ICLI_RANGE_TYPE_INVALID = -1,
    //------- add below
    ICLI_RANGE_TYPE_NONE,
    ICLI_RANGE_TYPE_SIGNED,
    ICLI_RANGE_TYPE_UNSIGNED,
} icli_range_type_t;

typedef struct {
    u32                         type; /* icli_range_type_t */
    union {
        icli_signed_range_t     sr;
        icli_unsigned_range_t   ur;
    } u;
} icli_range_t;

typedef enum {
    ICLI_PORT_TYPE_INVALID = -1,
    ICLI_PORT_TYPE_NONE,

#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
    ICLI_PORT_TYPE_ALL,
#endif

    //------- add below

    ICLI_PORT_TYPE_FAST_ETHERNET,
    ICLI_PORT_TYPE_GIGABIT_ETHERNET,
    ICLI_PORT_TYPE_2_5_GIGABIT_ETHERNET,
    ICLI_PORT_TYPE_FIVE_GIGABIT_ETHERNET,
    ICLI_PORT_TYPE_TEN_GIGABIT_ETHERNET,

    //------- add above
    ICLI_PORT_TYPE_MAX,
} icli_port_type_t;

typedef struct {
    u16     port_type;      /* icli_port_type_t                             */
    u16     switch_id;      /* switch id                                    */
    u16     begin_port;     /* switch port for a specific port type         */
    u16     port_cnt;       /* port count for both of switch port and uport */
    u16     usid;           /* the corresponding usid to switch id          */
    u16     begin_uport;    /* the corresponding uport to switch port       */
    u16     isid;           /* the corresponding isid to switch id          */
    u16     begin_iport;    /* the corresponding iport to switch port       */
} icli_switch_port_range_t;

typedef struct {
    u32                         cnt;
    icli_switch_port_range_t    switch_range[ICLI_RANGE_LIST_CNT];
} icli_stack_port_range_t;

typedef struct {
    u32     b_negative;
    i32     number_before_point;
    i32     number_after_point;
} icli_float_t;

typedef struct {
    u16     min;            /* minimum value in range */
    u16     max;            /* maximum value in range */
    BOOL    b_odd_range;    /* not do range check, accept odd range */
} icli_ask_vcap_vr_t;

typedef struct {
    u16     low;    /* low value of user input */
    u16     high;   /* high value of user input */
} icli_vcap_vr_t;

/*
    there are many other types or initializations related this definition.
    so, if this has any change, find "ICLI_VARIABLE_TYPE_MODIFY" to modify
    all as necessary.
*/
typedef enum {
    ICLI_VARIABLE_INVALID = -1,
    //------- add below
    ICLI_VARIABLE_MAC_ADDR,
    ICLI_VARIABLE_MAC_UCAST,
    ICLI_VARIABLE_MAC_MCAST,
    ICLI_VARIABLE_IPV4_ADDR,
    ICLI_VARIABLE_IPV4_NETMASK,
    ICLI_VARIABLE_IPV4_WILDCARD,
    ICLI_VARIABLE_IPV4_UCAST,
    ICLI_VARIABLE_IPV4_MCAST,
    ICLI_VARIABLE_IPV4_NMCAST,
    ICLI_VARIABLE_IPV4_ABC,
    ICLI_VARIABLE_IPV4_SUBNET,
    ICLI_VARIABLE_IPV4_PREFIX,
    ICLI_VARIABLE_IPV6_ADDR,
    ICLI_VARIABLE_IPV6_NETMASK,
    ICLI_VARIABLE_IPV6_UCAST,
    ICLI_VARIABLE_IPV6_MCAST,
    ICLI_VARIABLE_IPV6_LINKLOCAL,
    ICLI_VARIABLE_IPV6_SUBNET,
    ICLI_VARIABLE_IPV6_PREFIX,
    ICLI_VARIABLE_INT8,
    ICLI_VARIABLE_INT16,
    ICLI_VARIABLE_INT,
    ICLI_VARIABLE_UINT8,
    ICLI_VARIABLE_UINT16,
    ICLI_VARIABLE_UINT,
    ICLI_VARIABLE_DATE,
    ICLI_VARIABLE_TIME,
    ICLI_VARIABLE_HHMM,
    ICLI_VARIABLE_WORD,
    ICLI_VARIABLE_KWORD,
    ICLI_VARIABLE_CWORD,
    ICLI_VARIABLE_DWORD,
    ICLI_VARIABLE_FWORD,
    ICLI_VARIABLE_STRING,
    ICLI_VARIABLE_LINE,
    ICLI_VARIABLE_PORT_ID,
    ICLI_VARIABLE_PORT_LIST,
    ICLI_VARIABLE_VLAN_ID,
    ICLI_VARIABLE_VLAN_LIST,
    ICLI_VARIABLE_RANGE_LIST,
    ICLI_VARIABLE_PORT_TYPE,
    ICLI_VARIABLE_PORT_TYPE_ID,
    ICLI_VARIABLE_PORT_TYPE_LIST,
    ICLI_VARIABLE_OUI,
    ICLI_VARIABLE_PCP,
    ICLI_VARIABLE_DSCP,
    ICLI_VARIABLE_DPL,
    ICLI_VARIABLE_HOSTNAME,
    ICLI_VARIABLE_CLOCK_ID,
    ICLI_VARIABLE_VCAP_VR,
    ICLI_VARIABLE_HEXVAL,
    ICLI_VARIABLE_VWORD,
    ICLI_VARIABLE_SWITCH_ID,
    ICLI_VARIABLE_SWITCH_LIST,

    //------ add above
    // the followings are internal use
    ICLI_VARIABLE_GREP,
    ICLI_VARIABLE_GREP_BEGIN,
    ICLI_VARIABLE_GREP_INCLUDE,
    ICLI_VARIABLE_GREP_EXCLUDE,
    ICLI_VARIABLE_GREP_STRING,
    ICLI_VARIABLE_KEYWORD,
    ICLI_VARIABLE_RANGE_INT,
    ICLI_VARIABLE_RANGE_UINT,
    ICLI_VARIABLE_RANGE_INT_LIST,
    ICLI_VARIABLE_RANGE_UINT_LIST,
    ICLI_VARIABLE_RANGE_WORD,
    ICLI_VARIABLE_RANGE_KWORD,
    ICLI_VARIABLE_RANGE_DWORD,
    ICLI_VARIABLE_RANGE_FWORD,
    ICLI_VARIABLE_RANGE_STRING,
    ICLI_VARIABLE_RANGE_LINE,
    ICLI_VARIABLE_RANGE_HEXVAL,
    ICLI_VARIABLE_RANGE_VWORD,

    //------ add above
    ICLI_VARIABLE_MAX

} icli_variable_type_t;

/*
    This union contains all variable types defined in ICLI engine.

    Because the length of u_word, u_kword and u_string is ICLI_STR_MAX_LEN + 1
    and too long, they are allocated dynamically and will be freed after use.

    ICLI_VARIABLE_TYPE_MODIFY
*/
typedef struct {
    u32                             type; /* icli_variable_type_t */
    union {
        vtss_mac_t                  u_mac_addr;         // ICLI_VARIABLE_MAC_ADDR
        vtss_mac_t                  u_mac_ucast;        // ICLI_VARIABLE_MAC_UCAST
        vtss_mac_t                  u_mac_mcast;        // ICLI_VARIABLE_MAC_MCAST
        vtss_ip_t                   u_ipv4_addr;        // ICLI_VARIABLE_IPV4_ADDR
        vtss_ip_t                   u_ipv4_netmask;     // ICLI_VARIABLE_IPV4_NETMASK
        vtss_ip_t                   u_ipv4_wildcard;    // ICLI_VARIABLE_IPV4_WILDCARD
        vtss_ip_t                   u_ipv4_ucast;       // ICLI_VARIABLE_IPV4_UCAST
        vtss_ip_t                   u_ipv4_mcast;       // ICLI_VARIABLE_IPV4_MCAST
        vtss_ip_t                   u_ipv4_nmcast;      // ICLI_VARIABLE_IPV4_NMCAST
        vtss_ip_t                   u_ipv4_abc;         // ICLI_VARIABLE_IPV4_ABC
        icli_ipv4_subnet_t          u_ipv4_subnet;      // ICLI_VARIABLE_IPV4_SUBNET
        u32                         u_ipv4_prefix;      // ICLI_VARIABLE_IPV4_PREFIX
        vtss_ipv6_t                 u_ipv6_addr;        // ICLI_VARIABLE_IPV6_ADDR
        vtss_ipv6_t                 u_ipv6_netmask;     // ICLI_VARIABLE_IPV6_NETMASK
        vtss_ipv6_t                 u_ipv6_ucast;       // ICLI_VARIABLE_IPV6_UCAST
        vtss_ipv6_t                 u_ipv6_mcast;       // ICLI_VARIABLE_IPV6_MCAST
        vtss_ipv6_t                 u_ipv6_linklocal;   // ICLI_VARIABLE_IPV6_LINKLOCAL
        icli_ipv6_subnet_t          u_ipv6_subnet;      // ICLI_VARIABLE_IPV6_SUBNET
        u32                         u_ipv6_prefix;      // ICLI_VARIABLE_IPV6_PREFIX
        i8                          u_int8;             // ICLI_VARIABLE_INT8
        i16                         u_int16;            // ICLI_VARIABLE_INT16
        i32                         u_int;              // ICLI_VARIABLE_INT
        u8                          u_uint8;            // ICLI_VARIABLE_UINT8
        u16                         u_uint16;           // ICLI_VARIABLE_UINT16
        u32                         u_uint;             // ICLI_VARIABLE_UINT
        icli_date_t                 u_date;             // ICLI_VARIABLE_DATE
        icli_time_t                 u_time;             // ICLI_VARIABLE_TIME
        icli_time_t                 u_hhmm;             // ICLI_VARIABLE_HHMM
        char                        u_word[ICLI_VALUE_STR_MAX_LEN + 4];     // ICLI_VARIABLE_WORD
        char                        u_kword[ICLI_VALUE_STR_MAX_LEN + 4];    // ICLI_VARIABLE_KWORD
        char                        u_cword[ICLI_VALUE_STR_MAX_LEN + 4];    // ICLI_VARIABLE_CWORD
        char                        u_dword[ICLI_VALUE_STR_MAX_LEN + 4];    // ICLI_VARIABLE_DWORD
        char                        u_fword[ICLI_VALUE_STR_MAX_LEN + 4];    // ICLI_VARIABLE_FWORD
        char                        u_string[ICLI_VALUE_STR_MAX_LEN + 4];   // ICLI_VARIABLE_STRING
        char                        u_line[ICLI_VALUE_STR_MAX_LEN + 4];     // ICLI_VARIABLE_LINE, ICLI_VARIABLE_GREP_STRING
        icli_port_type_t            u_port_type;        // ICLI_VARIABLE_PORT_TYPE
        icli_switch_port_range_t    u_port_id;          // ICLI_VARIABLE_PORT_ID
        icli_stack_port_range_t     u_port_list;        // ICLI_VARIABLE_PORT_LIST
        u32                         u_vlan_id;          // ICLI_VARIABLE_VLAN_ID
        icli_unsigned_range_t       u_vlan_list;        // ICLI_VARIABLE_VLAN_LIST
        icli_range_t                u_range_list;       // ICLI_VARIABLE_RANGE_LIST
        icli_switch_port_range_t    u_port_type_id;     // ICLI_VARIABLE_PORT_TYPE_ID
        icli_stack_port_range_t     u_port_type_list;   // ICLI_VARIABLE_PORT_TYPE_LIST
        icli_oui_t                  u_oui;              // ICLI_VARIABLE_OUI
        icli_unsigned_range_t       u_pcp;              // ICLI_VARIABLE_PCP
        u8                          u_dscp;             // ICLI_VARIABLE_DSCP
        u8                          u_dpl;              // ICLI_VARIABLE_DPL
        char                        u_hostname[ICLI_VALUE_STR_MAX_LEN + 4]; // ICLI_VARIABLE_HOSTNAME
        icli_clock_id_t             u_clock_id;         // ICLI_VARIABLE_CLOCK_ID
        icli_vcap_vr_t              u_vcap_vr;          // ICLI_VARIABLE_VCAP_VR
        icli_hexval_t               u_hexval;           // ICLI_VARIABLE_HEXVAL
        char                        u_vword[ICLI_VALUE_STR_MAX_LEN + 4];    // ICLI_VARIABLE_VWORD
        u32                         u_switch_id;        // ICLI_VARIABLE_SWITCH_ID
        icli_unsigned_range_t       u_switch_list;      // ICLI_VARIABLE_SWITCH_LIST

        //------ add above
        // the followings are internal use
        u32                         u_grep;             // ICLI_VARIABLE_GREP
        u32                         u_grep_begin;       // ICLI_VARIABLE_GREP_BEGIN
        u32                         u_grep_include;     // ICLI_VARIABLE_GREP_INCLUDE
        u32                         u_grep_exclude;     // ICLI_VARIABLE_GREP_EXCLUDE
        i32                         u_range_int;        // ICLI_VARIABLE_RANGE_INT
        u32                         u_range_uint;       // ICLI_VARIABLE_RANGE_UINT
        icli_signed_range_t         u_range_int_list;   // ICLI_VARIABLE_RANGE_INT_LIST
        icli_unsigned_range_t       u_range_uint_list;  // ICLI_VARIABLE_RANGE_UINT_LIST
        char                        u_range_word[ICLI_VALUE_STR_MAX_LEN + 4];   // ICLI_VARIABLE_RANGE_WORD
        char                        u_range_kword[ICLI_VALUE_STR_MAX_LEN + 4];  // ICLI_VARIABLE_RANGE_KWORD
        char                        u_range_dword[ICLI_VALUE_STR_MAX_LEN + 4];  // ICLI_VARIABLE_RANGE_DWORD
        char                        u_range_fword[ICLI_VALUE_STR_MAX_LEN + 4];  // ICLI_VARIABLE_RANGE_FWORD
        char                        u_range_string[ICLI_VALUE_STR_MAX_LEN + 4]; // ICLI_VARIABLE_RANGE_STRING
        char                        u_range_line[ICLI_VALUE_STR_MAX_LEN + 4];   // ICLI_VARIABLE_RANGE_LINE
        icli_hexval_t               u_range_hexval;     // ICLI_VARIABLE_RANGE_HEXVAL
        char                        u_range_vword[ICLI_VALUE_STR_MAX_LEN + 4];  // ICLI_VARIABLE_RANGE_VWORD
    } u;
} icli_variable_value_t;

/*
    ICLI_ASK_PRESENT    : ask if the word is present or not
    ICLI_ASK_BYWORD     : ask byword, and this works on non-keyword
    ICLI_ASK_HELP       : ask help string
    ICLI_ASK_RANGE      : ask integer range for signed or unsigned,
                          this works on variables for all signed and
                          unsigned integer or integet list
    ICLI_ASK_PORT_RANGE : ask port type and list for the port range,
                          this works on <port_type_id>, <port_type_list>,
                          <port_type>, <port_id>, <port_list>
    ICLI_ASK_CWORD      : ask all possible customized words for <cword>
                          use 'NULL' for the end
    ICLI_ASK_VCAP_VR    : ask range for vcap_vr
*/
typedef enum {
    ICLI_ASK_PRESENT,
    ICLI_ASK_BYWORD,
    ICLI_ASK_HELP,
    ICLI_ASK_RANGE,
    ICLI_ASK_PORT_RANGE,
    ICLI_ASK_CWORD,
    ICLI_ASK_VCAP_VR,
} icli_runtime_ask_t;

typedef union {
    BOOL                        present;
    char                        byword[ICLI_BYWORD_MAX_LEN + 4];
    char                        help[ICLI_HELP_MAX_LEN + 4];
    icli_range_t                range;
    icli_stack_port_range_t     port_range;
    char                        *cword[ICLI_CWORD_MAX_CNT];
    icli_ask_vcap_vr_t          vcap_vr;
} icli_runtime_t;

/*
    INPUT
        session_id : session ID
        ask        : what is asked at runtime

    OUTPUT
        runtime
            ICLI_ASK_PRESENT    : runtime.present
            ICLI_ASK_BYWORD     : runtime.byword
            ICLI_ASK_HELP       : runtime.help
            ICLI_ASK_VALUE      : runtime.range
            ICLI_ASK_PORT_RANGE : runtime.port_range
            ICLI_ASK_CWORD      : runtime.cword
            ICLI_ASK_VCAP_VR    : runtime.vcap_vr

    RETURN
        TRUE
            ICLI engine will check the output value in runtime.

            ICLI_ASK_PRESENT    : runtime.present == TRUE,  enable the word
                                  runtime.present == FALSE, disable the word
            ICLI_ASK_BYWORD     : use runtime.byword
            ICLI_ASK_HELP       : use runtime.help
            ICLI_ASK_VALUE      : use runtime.range
            ICLI_ASK_PORT_RANGE : use runtime.port_range
            ICLI_ASK_CWORD      : use runtime.cword
            ICLI_ASK_VCAP_VR    : use runtime.vcap_vr

        FALSE
            ICLI engine will ignore the output value in runtime.

            ICLI_ASK_PRESENT    : the word is present
            ICLI_ASK_BYWORD     : use original one in *.icli
            ICLI_ASK_HELP       : use original one in *.icli
            ICLI_ASK_VALUE      : use original one in *.icli
            ICLI_ASK_PORT_RANGE : use system port range
            ICLI_ASK_CWORD      : <cword> works as <word>
            ICLI_ASK_VCAP_VR    : no range limit
*/
typedef BOOL (icli_runtime_cb_t)(
    IN  u32                   session_id,
    IN  icli_runtime_ask_t    ask,
    OUT icli_runtime_t        *runtime
);

/*-- command is enabled or disabled --*/
#define ICLI_CMD_ENABLE_BIT                 ( 0x00000001 << 0 )

// Command property
#define ICLI_CMD_PROP_ENABLE                0x00000000
#define ICLI_CMD_PROP_DISABLE               ICLI_CMD_ENABLE_BIT

#define ICLI_CMD_ENABLE(p)                  *(p) = ( (*(p)) & ~ICLI_CMD_ENABLE_BIT )
#define ICLI_CMD_DISABLE(p)                 *(p) = ( (*(p)) |  ICLI_CMD_ENABLE_BIT )

#define ICLI_CMD_IS_ENABLE(p)               ( ((p) & ICLI_CMD_ENABLE_BIT) == ICLI_CMD_PROP_ENABLE )
#define ICLI_CMD_IS_DISABLE(p)              ( ((p) & ICLI_CMD_ENABLE_BIT) == ICLI_CMD_PROP_DISABLE )

/*-- command is visible or not --*/
#define ICLI_CMD_VISIBLE_BIT                ( 0x00000001 << 1 )

// Command property
#define ICLI_CMD_PROP_VISIBLE               0x00000000
#define ICLI_CMD_PROP_INVISIBLE             ICLI_CMD_VISIBLE_BIT

#define ICLI_CMD_VISIBLE(p)                 *(p) = ( (*(p)) & ~ICLI_CMD_VISIBLE_BIT )
#define ICLI_CMD_INVISIBLE(p)               *(p) = ( (*(p)) |  ICLI_CMD_VISIBLE_BIT )

#define ICLI_CMD_IS_VISIBLE(p)              ( ((p) & ICLI_CMD_VISIBLE_BIT) == ICLI_CMD_PROP_VISIBLE )
#define ICLI_CMD_IS_INVISIBLE(p)            ( ((p) & ICLI_CMD_VISIBLE_BIT) == ICLI_CMD_PROP_INVISIBLE )

/*-- command with grep or not --*/
#define ICLI_CMD_GREP_BIT                   ( 0x00000001 << 2 )

// Command property
#define ICLI_CMD_PROP_GREP                  ICLI_CMD_GREP_BIT
#define ICLI_CMD_PROP_NOT_GREP              0x00000000

#define ICLI_CMD_IS_GREP(p)                 ( ((p) & ICLI_CMD_GREP_BIT) == ICLI_CMD_PROP_GREP )
#define ICLI_CMD_IS_NOT_GREP(p)             ( ((p) & ICLI_CMD_GREP_BIT) == ICLI_CMD_PROP_NOT_GREP )

/*-- command can input more redundant words or not --*/
#define ICLI_CMD_LOOSELY_BIT                ( 0x00000001 << 3 )

// Command property
#define ICLI_CMD_PROP_LOOSELY               ICLI_CMD_LOOSELY_BIT
#define ICLI_CMD_PROP_STRICTLY              0x00000000

#define ICLI_CMD_LOOSELY(p)                 *(p) = ( (*(p)) |  ICLI_CMD_LOOSELY_BIT )
#define ICLI_CMD_STRICTLY(p)                *(p) = ( (*(p)) & ~ICLI_CMD_LOOSELY_BIT )

#define ICLI_CMD_IS_LOOSELY(p)              ( ((p) & ICLI_CMD_LOOSELY_BIT) == ICLI_CMD_PROP_LOOSELY )
#define ICLI_CMD_IS_STRICTLY(p)             ( ((p) & ICLI_CMD_LOOSELY_BIT) == ICLI_CMD_PROP_STRICTLY )

/* keywords for GREP */
#define ICLI_GREP_KEYWORD           "|"
#define ICLI_GREP_BEGIN_KEYWORD     "begin"
#define ICLI_GREP_INCLUDE_KEYWORD   "include"
#define ICLI_GREP_EXCLUDE_KEYWORD   "exclude"

#define ICLI_BYWORD_PARA_MAX_CNT    2

typedef struct _cmd_byword {
    char        *word;
    i32         para_cnt;
    i32         para[ICLI_BYWORD_PARA_MAX_CNT];
} icli_byword_t;

/* because the node may be re-used */
typedef struct _cmd_property {
    i32                         cmd_id;
    u32                         property;
    u32                         privilege; // icli_privilege_t
    u32                         orig_priv; // icli_privilege_t
    u32                         mode_goto;
    u32                         mode_destroy;

    // use word_id to get the corresponding data
    icli_byword_t               *byword;
    char                        **help;
    icli_runtime_cb_t           **runtime_cb;

    // next property
    //struct _cmd_property        *next;
} icli_cmd_property_t;

typedef struct _node_property {
    i32                         word_id;
    icli_cmd_property_t         *cmd_property;
    struct _node_property       *next;
} node_property_t;

typedef enum {
    /* Not match or Error */
    ICLI_MATCH_TYPE_ERR = -100,

    /* exactly for both keyword and variables */
    ICLI_MATCH_TYPE_EXACTLY = 0,

    /* for string type variable */
    ICLI_MATCH_TYPE_PARTIAL,

    /* for variables */
    ICLI_MATCH_TYPE_INCOMPLETE,

#if 0 /* CP, 2013/03/14 18:02, processing <string> */
    /*
        if parsing ICLI_VARIABLE_LINE, then cmd word is re-aligned.
        So, redo the match for all possible next nodes.
    */
    ICLI_MATCH_TYPE_REDO,
#endif
} icli_match_type_t;

//pre-declare for the prior use
typedef struct _icli_parsing_node   icli_parsing_node_t;

//parameter list of command result
typedef struct _icli_parameter {
    //word id
    u32                         word_id;
    //the value for the variable
    icli_variable_value_t       value;

    //match information
    icli_parsing_node_t         *match_node;
    icli_match_type_t           match_type;

#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
    //for loop
    i32                         b_in_loop;
#endif

    //link list
    struct _icli_parameter      *next;
} icli_parameter_t;

//command callback, used in auto-generation C file
typedef i32 (icli_cmd_cb_t)(
    IN u32                      session_id,
    IN icli_parameter_t         *mode_var, //parameter_list_t
    IN icli_parameter_t         *cmd_var, //parameter_list_t
    IN i32                      usr_opt
);

//command callback and the user option for this command
typedef struct {
    i32                         cmd_id;
    icli_cmd_cb_t               *cmd_cb;
    u32                         usr_opt;
} icli_cmd_execution_t;

// node of parsing tree
// will be auto-generated from script to be static memory
// the static memory will be used in building parsing tree
struct _icli_parsing_node {
    // word id
    u32                         word_id;

    // keyword
    char                        *word;

    // type
    icli_variable_type_t        type;

    // depend on type, icli_signed_range_t* or icli_unsigned_range_t*
    icli_range_t                *range;

    // command property
    node_property_t             node_property;

    // callback and user option
    icli_cmd_execution_t        *execution;

    /*
        optional bit mask
        the bit mask depends on optional level, 0x1L << optional_level
        a [ b [ c [ d ] ] ]
        b = optional level 0
        c = optional level 1
        d = optional level 2
    */
    u32                         optional_begin;
    u32                         optional_end;

    // number of parents
    u32                         parent_cnt;

    /*
        the parent is not always the immediate father
        but the beginning node of the previous black
        "a b c d", d->parent = c
        "a { b c } d" or "a [ b c ] d", d->parent = b, but not c
    */
    icli_parsing_node_t         *parent;
    icli_parsing_node_t         *child;
    icli_parsing_node_t         *sibling;
#if 1 /* CP, 2012/09/07 11:52, <port_type_list> */
    icli_parsing_node_t         *loop;
#endif

#if ICLI_RANDOM_OPTIONAL
    /*
        for continuous optional blocks
        optional end points to previous optional begin
        "a [b] [c]",     c->random_optional = b
        "a [b c] [d e]", d->random_optional = NULL, e->random_optional = b
        "a [b] [c [d]]", c->random_optional = b,    d->random_optional = b

        ICLI_RANDOM_OPTIONAL_DEPTH: the nearest optional block use the less depth
        "a [b] [c [d] [e]]", e->random_optional[0] = d,
                             e->random_optional[1] = b
    */
    icli_parsing_node_t         *random_optional[ICLI_RANDOM_OPTIONAL_DEPTH];
    u32                         random_optional_level[ICLI_RANDOM_OPTIONAL_DEPTH];

#if ICLI_RANDOM_MUST_NUMBER /* CP, 2012/10/19 12:15, random optional must number */
    /*
        The number is for the syntax "a {[b|x] [c] [d]}*2"
        and it is stored in the random optional head
        in this example, b is the head and number is 2
        it means at least 2 of b, c, d are present
        => x, c and d will have the same data with b
    */
    icli_parsing_node_t         *random_must_head;
    u32                         random_must_number;
    u32                         random_must_level;

    /*
        0: not begin, 1: begin.
        In this example, b and x are 1.
    */
    u32                         random_must_begin;

#if 1 /* CP, 10/27/2013 22:01 */
    /*
        0: not middle, 1: middle.
        In this example, c is 1.
    */
    u32                         random_must_middle;
#endif

    /*
        0: not end, 1: end.
        In this example, d is 1.
    */
    u32                         random_must_end;
#endif // ICLI_RANDOM_MUST_NUMBER

#endif // ICLI_RANDOM_OPTIONAL

#if 1 /* CP, 06/25/2013 12:08, Bugzilla#12073 - Incorrect ICLI handling of RUNTIME */
    /*
        a { b | c } d
        a [ b | c } d
        - b and c in the above 2 commands are or heads.
    */
    u32                         or_head;
#endif

};

// the struct is an input parameter of command register
typedef struct {
    char                        *cmd;
    char                        *original_cmd;
#if 1 /* CP, 09/30/2013 16:44, Bugzilla#12878 - ICLI presents symbolic constants from COMMANDs to the user */
    char                        *var_cmd;
#endif
    icli_parsing_node_t         *node;
    u32                         number_of_nodes;
    icli_cmd_property_t         *cmd_property;
    icli_cmd_execution_t        *cmd_execution;
    u32                         cmd_mode;
} icli_cmd_register_t;

typedef enum {
    ICLI_INPUT_STYLE_INVALID = -1,
    //------- add below
    ICLI_INPUT_STYLE_SIMPLE,
    ICLI_INPUT_STYLE_SINGLE_LINE,
    ICLI_INPUT_STYLE_MULTIPLE_LINE,

    //-------- add above
    ICLI_INPUT_STYLE_MAX
} icli_input_style_t;

/*
    init data for ICLI engine
*/
typedef struct {
    icli_input_style_t          input_style;
    u32                         case_sensitive;
    u32                         console_alive;
} icli_init_data_t;

/*
    waiting time constants
*/
#define ICLI_TIMEOUT_NO_WAIT        0
#define ICLI_TIMEOUT_FOREVER        -1

/*
    get session input by char

    INPUT
        app_id    : application ID
        wait_time : in millisecond
                    = 0 - no wait
                    < 0 - forever

    OUTPUT
        c : char inputted

    RETURN
        TRUE  : successful
        FALSE : failed due to timeout
*/
typedef BOOL icli_session_char_get_t(
    IN  u32     app_id,
    IN  i32     wait_time,
    OUT i32     *c
);

/*
    get session input by string

    INPUT
        app_id  : application ID
        str_len : max length of str

    OUTPUT
        str : the complete command string

    RETURN
        TRUE  : successful
        FALSE : failed
*/
typedef BOOL icli_session_str_get_t(
    IN  u32     app_id,
    IN  u32     str_len,
    OUT char    *str
);

/*
    output/display one char on session

    INPUT
        app_id : application ID
        c      : the char for output/display

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed
*/
typedef BOOL icli_session_char_put_t(
    IN  u32     app_id,
    IN  char    c
);

/*
    output/display string on session

    INPUT
        app_id  : application ID
        str     : the string for output/display

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed
*/
typedef BOOL icli_session_str_put_t(
    IN  u32     app_id,
    IN  char    *str
);

/*
    init APP session

    INPUT
        app_id  : application ID

    OUTPUT
        n/a

    RETURN
        TRUE  : successful
        FALSE : failed, then the corresponding ICLI session will stop also
*/
typedef BOOL icli_session_app_init_t(
    IN  u32     app_id
);

/*
    reason of session close
*/
typedef enum {
    ICLI_CLOSE_REASON_NORMAL,
    ICLI_CLOSE_REASON_INIT_FAIL,
    ICLI_CLOSE_REASON_LOGIN_FAIL,
} icli_session_close_reason_t;

/*
    close APP session

    INPUT
        app_id : application ID
        reason : why the session is closed

    OUTPUT
        n/a

    RETURN
        n/a
*/
typedef void icli_session_app_close_t(
    IN  u32                             app_id,
    IN  icli_session_close_reason_t     reason
);

/*
    error code of execution
*/
typedef enum {
    ICLI_EXEC_ERRCODE_SUCCESS = 0,
    ICLI_EXEC_ERRCODE_UNRECOGNIZED = -1000,
    ICLI_EXEC_ERRCODE_INCOMPLETE,
    ICLI_EXEC_ERRCODE_AMBIGUOUS,
    ICLI_EXEC_ERRCODE_INVALID,
    ICLI_EXEC_ERRCODE_SYNTAX,
    ICLI_EXEC_ERRCODE_MEMORY,
    ICLI_EXEC_ERRCODE_PARSE,
    ICLI_EXEC_ERRCODE_NULL,
    ICLI_EXEC_ERRCODE_EMPTY,
    ICLI_EXEC_ERRCODE_LONG,
} icli_exec_errcode_t;

/*
    tell APP the execution error

    INPUT
        app_id  : application ID
        cmd     : current command in execution
        errcode : error code

    OUTPUT
        n/a

    RETURN
        n/a
*/
typedef void icli_session_error_get_t(
    IN  i32                     app_id,
    IN  char                    *cmd,
    IN  icli_exec_errcode_t     errcode
);

typedef enum {
    ICLI_IP_ADDR_TYPE_INVALID = -1,
    //------- add below
    ICLI_IP_ADDR_TYPE_NONE,
    ICLI_IP_ADDR_TYPE_IPV4,
    ICLI_IP_ADDR_TYPE_IPV6,
} icli_ip_addr_type_t;

typedef struct {
    icli_ip_addr_type_t     type;
    union {
        vtss_ip_t           ipv4;
        vtss_ipv6_t         ipv6;
    } u;
} icli_ip_addr_t;

/*
    session way
*/
typedef enum {
    ICLI_SESSION_WAY_INVALID = -1,
    //------add below

    ICLI_SESSION_WAY_CONSOLE,
    ICLI_SESSION_WAY_TELNET,
    ICLI_SESSION_WAY_SSH,
    ICLI_SESSION_WAY_APP_EXEC,
    ICLI_SESSION_WAY_THREAD_CONSOLE,
    ICLI_SESSION_WAY_THREAD_TELNET,
    ICLI_SESSION_WAY_THREAD_SSH,

    //------add above
    ICLI_SESSION_WAY_MAX
} icli_session_way_t;

typedef struct {
    /*----- INPUT----*/
    char                        *name;
    icli_session_way_t          way;

    /*
        app_id:
            a parameter of every I/O callback.
            given by APP.
            it is used by APP to identify the APP session, but not for ICLI.
    */
    u32                         app_id;

    /* I/O callback */
    icli_session_char_get_t     *char_get;
    icli_session_char_put_t     *char_put;
    icli_session_str_put_t      *str_put;

    /* APP session callback */
    icli_session_app_init_t     *app_init;
    icli_session_app_close_t    *app_close;

    /*
        if the session is through network, tell ICLI engine what IP is from.
        this is used for LOG.
    */
    icli_ip_addr_t              client_ip;
    u32                         client_port;

    /* char_put */
    u32                         b_app_output;

} icli_session_open_data_t;

/*
    error code of execution
*/
typedef enum {
    ICLI_USR_INPUT_TYPE_NORMAL,
    ICLI_USR_INPUT_TYPE_PASSWORD,

    //------add above
    ICLI_USR_INPUT_TYPE_MAX
} icli_usr_input_type_t;

typedef enum {
    ICLI_LINE_MODE_INVALID = -1,
    //------- add below
    /* display page by page */
    ICLI_LINE_MODE_PAGE,
    /* display all without page */
    ICLI_LINE_MODE_FLOOD,
    /* not display any */
    ICLI_LINE_MODE_BYPASS,
    ICLI_LINE_MODE_MAX
} icli_line_mode_t;

/*
                    GET       GET_NEXT  SET
                    ========  ========  ========
    session_id      IN        INOUT     IN
    privilege       OUT       OUT       IN
    width           OUT       OUT       IN
    lines           OUT       OUT       IN
    wait_time       OUT       OUT       IN
    line_mode       OUT       OUT       IN
    input_style     OUT       OUT       IN
    alive           OUT       OUT       -
    way             OUT       OUT       -
    connect_time    OUT       OUT       -
    elapsed_time    OUT       OUT       -

    ps: '-' means the parameter has no use in that operation.
*/
typedef struct {
    /* INDEX */
    u32                 session_id;

    u32                 alive;
    icli_session_way_t  way;
    icli_cmd_mode_t     mode;
    u32                 connect_time; //in seconds
    u32                 elapsed_time; //in seconds
    u32                 idle_time;    //in seconds

    icli_privilege_t    privilege;
    u32                 width;
    u32                 lines;
    u32                 wait_time; //in seconds
    icli_line_mode_t    line_mode;
    icli_input_style_t  input_style;

#if 1 /* CP, 2012/08/29 09:25, history max count is configurable */
    u32                 history_size;
#endif

#if 1 /* CP, 2012/08/31 07:51, enable/disable banner per line */
    i32                 b_exec_banner;
    i32                 b_motd_banner;
#endif

#if 1 /* CP, 2012/08/31 09:31, location and default privilege */
    char                location[ICLI_LOCATION_MAX_LEN + 4];
    icli_privilege_t    privileged_level;
#endif

#if 1 /* CP, 2012/09/04 16:46, session user name */
    char                user_name[ICLI_USERNAME_MAX_LEN + 4];
    icli_ip_addr_t      client_ip;
    u32                 client_port;
#endif

} icli_session_data_t;

typedef struct {
    /*
        INDEX - history_pos,
        the position from history_cmd_head->next,
        start from 0
    */
    u32     history_pos;
    char    history_cmd[ICLI_STR_MAX_LEN + 4];
} icli_session_history_cmd_t;

#if 1 /* CP, 2012/09/14 10:38, conf */
typedef struct {
    // user input waiting time
    u32                         wait_time;    //in milli-seconds

    // input style
    icli_input_style_t          input_style;

    // terminal configuration
    u32                         width;
    u32                         lines;

    // history
#if 1 /* CP, 2012/08/29 09:25, history max count is configurable */
    u32                         history_size;
#endif

    // banner
#if 1 /* CP, 2012/08/31 07:51, enable/disable banner per line */
    i32                         b_exec_banner;
    i32                         b_motd_banner;
#endif

#if 1 /* CP, 2012/08/31 09:31, location and default privilege */
    char                        location[ICLI_LOCATION_MAX_LEN + 4];
    u32                         privileged_level; // icli_privilege_t
#endif

} icli_session_config_data_t;

typedef struct {
    /* this must be initialized one by one mapping even empty string */
    char     mode_prompt[ICLI_CMD_MODE_MAX][ICLI_PROMPT_MAX_LEN + 4];

    /* device name */
    char     dev_name[ICLI_DEV_NAME_MAX_LEN + 1];

    /* banner */
    char     banner_login[ICLI_BANNER_MAX_LEN + 4];
    char     banner_motd[ICLI_BANNER_MAX_LEN + 4];
    char     banner_exec[ICLI_BANNER_MAX_LEN + 4];

    /* config data per session */
    icli_session_config_data_t      session_config[ICLI_SESSION_CNT];

    /* enable password for each privilege */
    char     enable_password[ICLI_PRIVILEGE_MAX][ICLI_PASSWORD_MAX_LEN + 4];

    /* TRUE: secret password, FALSE: clear password */
    BOOL     b_enable_secret[ICLI_PRIVILEGE_MAX];

} icli_conf_data_t;
#endif

typedef struct {
    /* config data */
    icli_cmd_mode_t         mode;
    char                    cmd[ICLI_PRIV_CMD_MAX_LEN + 4];
    icli_privilege_t        privilege;
} icli_priv_cmd_conf_t;

//****************************************************************************
#endif //__VTSS_ICLI_TYPE_H__

