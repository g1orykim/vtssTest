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
#if 1 /* CP, 2012/10/03 12:33, Bugzilla#9873 - accept any format */
#define __IS_NUMBER(c) \
    ( ICLI_IS_DIGIT(c) || \
      ((c) == 'b') || ((c) == 'B') || \
      ((c) == 'o') || ((c) == 'O') || \
      ((c) == 'x') || ((c) == 'X') || \
      ((c) >= 'A'  &&  (c) <= 'F') || \
      ((c) >= 'a'  &&  (c) <= 'f')  )
#endif

#define _ERR_MATCH_RETURN() \
    if ( err_pos ) {\
        (*err_pos) = c - word;\
    }\
    return ICLI_RC_ERR_MATCH

#define _U_INT_MIN      0x80000000

/*
==============================================================================

    Type Definition

==============================================================================
*/
typedef struct {
    char    *name;          /* the string of constant in icli_variable_type_t               */
    char    *variable;      /* variable string enclosed by '<>' used in ICLI script         */
    char    *decl_type;     /* data type in icli_variable_value_t declared for the variable */
    char    *init_val;      /* initial value of the variable                                */
    BOOL    b_pointer_type; /* if the data type is a pointer or not according to decl_type  */
    BOOL    b_string_type;  /* if the data type is a string, that is, char *                */
    BOOL    b_has_range;    /* if the data has range checking                               */
} icli_variable_data_t;

typedef struct {
    char    *name;          /* complete name */
    char    *short_name;    /* short name    */
    char    *help;          /* help string   */
} icli_port_type_data_t;

/*
==============================================================================

    Static Variable

==============================================================================
*/
/*
    NOTICE :
        the size and order MUST BE the same with icli_variable_type_t.
*/
static icli_variable_data_t     g_variable_data[] = {
    {
        "ICLI_VARIABLE_MAC_ADDR",
        "mac_addr",
        "vtss_mac_t",
        "{{0,0,0,0,0,0}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_MAC_UCAST",
        "mac_ucast",
        "vtss_mac_t",
        "{{0,0,0,0,0,0}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_MAC_MCAST",
        "mac_mcast",
        "vtss_mac_t",
        "{{0,0,0,0,0,0}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV4_ADDR",
        "ipv4_addr",
        "vtss_ip_t",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV4_NETMASK",
        "ipv4_netmask",
        "vtss_ip_t",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV4_WILDCARD",
        "ipv4_wildcard",
        "vtss_ip_t",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV4_UCAST",
        "ipv4_ucast",
        "vtss_ip_t",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV4_MCAST",
        "ipv4_mcast",
        "vtss_ip_t",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV4_NMCAST",
        "ipv4_nmcast",
        "vtss_ip_t",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV4_ABC",
        "ipv4_abc",
        "vtss_ip_t",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV4_SUBNET",
        "ipv4_subnet",
        "icli_ipv4_subnet_t",
        "{0,0}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV4_PREFIX",
        "ipv4_prefix",
        "u32",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV6_ADDR",
        "ipv6_addr",
        "vtss_ipv6_t",
        "{{0}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV6_NETMASK",
        "ipv6_netmask",
        "vtss_ipv6_t",
        "{{0}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV6_UCAST",
        "ipv6_ucast",
        "vtss_ipv6_t",
        "{{0}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV6_MCAST",
        "ipv6_mcast",
        "vtss_ipv6_t",
        "{{0}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV6_LINKLOCAL",
        "ipv6_linklocal",
        "vtss_ipv6_t",
        "{{0}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV6_SUBNET",
        "ipv6_subnet",
        "icli_ipv6_subnet_t",
        "{{{0}}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_IPV6_PREFIX",
        "ipv6_prefix",
        "u32",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_INT8",
        "int8",
        "i8",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_INT16",
        "int16",
        "i16",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_INT",
        "int",
        "i32",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_UINT8",
        "uint8",
        "u8",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_UINT16",
        "uint16",
        "u16",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_UINT",
        "uint",
        "u32",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_DATE",
        "date",
        "icli_date_t",
        "{0}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_TIME",
        "time",
        "icli_time_t",
        "{0}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_HHMM",
        "hhmm",
        "icli_time_t",
        "{0}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_WORD",
        "word",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_KWORD",
        "kword",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_CWORD",
        "cword",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_DWORD",
        "dword",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_FWORD",
        "fword",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_STRING",
        "string",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_LINE",
        "line",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_PORT_ID",
        "port_id",
        "icli_switch_port_range_t",
        "{ICLI_PORT_TYPE_NONE}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_PORT_LIST",
        "port_list",
        "icli_stack_port_range_t*",
        "NULL",
        TRUE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_VLAN_ID",
        "vlan_id",
        "u32",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_VLAN_LIST",
        "vlan_list",
        "icli_unsigned_range_t*",
        "NULL",
        TRUE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_RANGE_LIST",
        "range_list",
        "icli_range_t*",
        "NULL",
        TRUE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_PORT_TYPE",
        "port_type",
        "icli_port_type_t",
        "ICLI_PORT_TYPE_NONE",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_PORT_TYPE_ID",
        "port_type_id",
        "icli_switch_port_range_t",
        "{ICLI_PORT_TYPE_NONE}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_PORT_TYPE_LIST",
        "port_type_list",
        "icli_stack_port_range_t*",
        "NULL",
        TRUE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_OUI",
        "oui",
        "icli_oui_t",
        "{{0,0,0}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_PCP",
        "pcp",
        "icli_unsigned_range_t*",
        "NULL",
        TRUE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_KEYWORD",
        "dscp",
        "u8",
        "0xFF",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_DPL",
        "dpl",
        "u8",
        "0xFF",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_HOSTNAME",
        "hostname",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_CLOCK_ID",
        "clock_id",
        "icli_clock_id_t",
        "{{0,0,0,0,0,0,0,0}}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_VCAP_VR",
        "vcap_vr",
        "icli_vcap_vr_t",
        "{0,0}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_HEXVAL",
        "hexval",
        "icli_hexval_t",
        "{0}",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_VWORD",
        "vword",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_SWITCH_ID",
        "switch_id",
        "u32",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_SWITCH_LIST",
        "switch_list",
        "icli_unsigned_range_t*",
        "NULL",
        TRUE,
        FALSE,
        FALSE,
    },

    //------ add above
    // the followings are internal use
    {
        "ICLI_VARIABLE_GREP",
        "grep",
        "u32",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_GREP_BEGIN",
        "grep_begin",
        "u32",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_GREP_INCLUDE",
        "grep_include",
        "u32",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_GREP_EXCLUDE",
        "grep_exclude",
        "u32",
        "0",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_GREP_STRING",
        "grep_string",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_KEYWORD",
        "keyword",
        "BOOL",
        "FALSE",
        FALSE,
        FALSE,
        FALSE,
    },
    {
        "ICLI_VARIABLE_RANGE_INT",
        "range_int",
        "i32",
        "0",
        FALSE,
        FALSE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_UINT",
        "range_uint",
        "u32",
        "0",
        FALSE,
        FALSE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_INT_LIST",
        "range_int_list",
        "icli_signed_range_t*",
        "NULL",
        TRUE,
        FALSE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_UINT_LIST",
        "range_uint_list",
        "icli_unsigned_range_t*",
        "NULL",
        TRUE,
        FALSE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_WORD",
        "range_word",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_KWORD",
        "range_kword",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_DWORD",
        "range_dword",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_FWORD",
        "range_fword",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_STRING",
        "range_string",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_LINE",
        "range_line",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_HEXVAL",
        "range_hexval",
        "icli_hexval_t",
        "{0}",
        FALSE,
        FALSE,
        TRUE,
    },
    {
        "ICLI_VARIABLE_RANGE_VWORD",
        "range_vword",
        "char*",
        "NULL",
        TRUE,
        TRUE,
        TRUE,
    },
};

static icli_port_type_data_t    g_port_type_data[ICLI_PORT_TYPE_MAX] = {
    {
        "None",
        "None",
        "None",
    },
    {
        ICLI_INTERFACE_WILDCARD,
        ICLI_INTERFACE_WILDCARD,
        "All switches or All ports",
    },
    {
        "FastEthernet",
        "Fa",
        "Fast Ethernet Port",
    },
    {
        "GigabitEthernet",
        "Gi",
        "1 Gigabit Ethernet Port",
    },
    {
        "2.5GigabitEthernet",
        "2.5G",
        "2.5 Gigabit Ethernet Port",
    },
    {
        "5GigabitEthernet",
        "5G",
        "5 Gigabit Ethernet Port",
    },
    {
        "10GigabitEthernet",
        "10G",
        "10 Gigabit Ethernet Port",
    },
};

#if 1 /* CP, 2012/09/25 10:47, <dscp> */
static icli_dscp_wvh_t      g_dscp_wvh[ICLI_DSCP_MAX_CNT] = {
    {"be",    0, "Default PHB(DSCP 0) for best effort traffic"},
    {"af11", 10, "Assured Forwarding PHB AF11(DSCP 10)"},
    {"af12", 12, "Assured Forwarding PHB AF12(DSCP 12)"},
    {"af13", 14, "Assured Forwarding PHB AF13(DSCP 14)"},
    {"af21", 18, "Assured Forwarding PHB AF21(DSCP 18)"},
    {"af22", 20, "Assured Forwarding PHB AF22(DSCP 20)"},
    {"af23", 22, "Assured Forwarding PHB AF23(DSCP 22)"},
    {"af31", 26, "Assured Forwarding PHB AF31(DSCP 26)"},
    {"af32", 28, "Assured Forwarding PHB AF32(DSCP 28)"},
    {"af33", 30, "Assured Forwarding PHB AF33(DSCP 30)"},
    {"af41", 34, "Assured Forwarding PHB AF41(DSCP 34)"},
    {"af42", 36, "Assured Forwarding PHB AF42(DSCP 36)"},
    {"af43", 38, "Assured Forwarding PHB AF43(DSCP 38)"},
    {"cs1",   8, "Class Selector PHB CS1 precedence 1(DSCP 8)"},
    {"cs2",  16, "Class Selector PHB CS2 precedence 2(DSCP 16)"},
    {"cs3",  24, "Class Selector PHB CS3 precedence 3(DSCP 24)"},
    {"cs4",  32, "Class Selector PHB CS4 precedence 4(DSCP 32)"},
    {"cs5",  40, "Class Selector PHB CS5 precedence 5(DSCP 40)"},
    {"cs6",  48, "Class Selector PHB CS6 precedence 6(DSCP 48)"},
    {"cs7",  56, "Class Selector PHB CS7 precedence 7(DSCP 56)"},
    {"ef",   46, "Expedited Forwarding PHB(DSCP 46)"},
    {"va",   44, "Voice Admit PHB(DSCP 44)"},
};
#endif

/*
==============================================================================

    Static Function
    ICLI_VARIABLE_TYPE_MODIFY

==============================================================================
*/
/*
    INPUT
        c : character
    OUTPUT
        n/a
    RETURN
        0-15  if successful
        -1    if invalid
*/
static i32 _binary_get_c(
    IN  i8  c
)
{
    if ( (c >= '0') && (c <= '1') ) {
        return ( c - '0' );
    }
    return -1;
}

/*
    INPUT
        c : character
    OUTPUT
        n/a
    RETURN
        0-15  if successful
        -1    if invalid
*/
static i32 _octal_get_c(
    IN  i8  c
)
{
    if ( (c >= '0') && (c <= '7') ) {
        return ( c - '0' );
    }
    return -1;
}

/*
    INPUT
        c : character
    OUTPUT
        n/a
    RETURN
        0-9   if successful
        -1    if invalid
*/
static i32 _digit_get_c(
    IN  i8  c
)
{
    if ( ICLI_IS_DIGIT(c) ) {
        return ( c - '0' );
    }
    return -1;
}

/*
    INPUT
        c : character
    OUTPUT
        n/a
    RETURN
        0-15  if successful
        -1    if invalid
*/
static i32 _hex_get_c(
    IN  i8  c
)
{
    if ( ICLI_IS_DIGIT(c) ) {
        return ( c - '0' );
    } else if ( (c >= 'A') && (c <= 'F') ) {
        return ( c - 'A' + 10 );
    } else if ( (c >= 'a') && (c <= 'f') ) {
        return ( c - 'a' + 10 );
    }
    return -1;
}

#if 1 /* CP, 2012/10/03 12:33, Bugzilla#9873 - accept any format */
/*
    check if unsigned 32-bit binary
    "0b" or "0B" is the required prefix

    INPUT
        word : user input word

    OUTPUT
        val     : unsigned 32-bit binary value
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 __binary_get(
    IN  char    *word,
    OUT u32     *val,
    OUT u32     *err_pos
)
{
    //common
    char        *c;
    //by type
    i32         i;
    char        *b = 0;
    u32         u = 0;

    c = word;
    if ( (*c) != '0' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    } else if ( (*c) != 'b' && (*c) != 'B' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        i = _binary_get_c(*c);
        if ( i == -1 ) {
            _ERR_MATCH_RETURN();
        }
        //get begin position of 1 happen
        if ( b == 0 ) {
            if ( i ) {
                b = c;
            }
        } else {
            // check overflow
            if ( (c - b) > 31 ) {
                _ERR_MATCH_RETURN();
            }
        }
        //update value
        u = (u << 1) + i;
    }

    (*val) = u;
    return ICLI_RC_OK;
}

/*
    check if unsigned 32-bit octal integer
    "0o" or "0O" is the required prefix

    INPUT
        word : user input word

    OUTPUT
        val     : unsigned octal value
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 __octal_get(
    IN  char    *word,
    OUT u32     *val,
    OUT u32     *err_pos
)
{
    //common
    char        *c;
    //by type
    i32         i;
    u32         v;
    u32         d;

    c = word;
    if ( (*c) != '0' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    } else if ( (*c) != 'o' && (*c) != 'O' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    v = 0;
    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        i = _octal_get_c(*c);
        if ( i == -1 ) {
            _ERR_MATCH_RETURN();
        }

        // check overflow
        if ( v ) {
            d = ICLI_MAX_UINT / v;
            if ( d < 8 ) {
                _ERR_MATCH_RETURN();
            } else if ( d == 8 ) {
                if ( (u32)i > (ICLI_MAX_UINT - (v << 3)) ) {
                    _ERR_MATCH_RETURN();
                }
            }
        }

        //update u
        v = (v << 3) + i;
    }

    (*val) = v;
    return ICLI_RC_OK;
}

/*
    check if unsigned 32-bit hex
    "0x" or "0X" is the required prefix

    INPUT
        word : user input word

    OUTPUT
        val     : unsigned 32-bit hex value
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 __hex_get(
    IN  char    *word,
    OUT u32     *val,
    OUT u32     *err_pos
)
{
    //common
    char        *c;
    //by type
    i32         i;
    char        *b = 0;
    u32         u = 0;

    c = word;
    if ( (*c) != '0' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    } else if ( (*c) != 'x' && (*c) != 'X' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        i = _hex_get_c(*c);
        if ( i == -1 ) {
            _ERR_MATCH_RETURN();
        }

        //get begin position of 1 happen
        if ( b == 0 ) {
            if ( i ) {
                b = c;
            }
        } else {
            // check overflow
            if ( (c - b) > 7 ) {
                _ERR_MATCH_RETURN();
            }
        }
        //update value
        u = (u << 4) + i;
    }

    (*val) = u;
    return ICLI_RC_OK;
}

/*
    get 32-bit unsigned decimal

    INPUT
        word : user input word

    OUTPUT
        val     : unsigned 32-bit decimal value
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 __decimal_get(
    IN  char    *word,
    OUT u32     *val,
    OUT u32     *err_pos
)
{
    //common
    char        *c;

    //by type
    i32         i;
    u32         j;
    u32         v;
    u32         d;

    c = word;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    v = 0;
    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        i = _digit_get_c(*c);
        if ( i == -1 ) {
            _ERR_MATCH_RETURN();
        }

        // check overflow
        j = (u32)i;
        if ( v ) {
            d = ICLI_MAX_UINT / v;
            if ( d < 10 ) {
                _ERR_MATCH_RETURN();
            } else if ( d == 10 ) {
                if ( j > (ICLI_MAX_UINT - (v * 10)) ) {
                    _ERR_MATCH_RETURN();
                }
            }
        }

        // update u
        v = v * 10 + j;
    }

    (*val) = v;
    return ICLI_RC_OK;
}

/*
    check if 32-bit signed binary

    INPUT
        word : user input word

    OUTPUT
        val     : 32-bit signed binary
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 __signed_binary_get(
    IN  char    *word,
    OUT i32     *val,
    OUT u32     *err_pos
)
{
    //common
    char        *c;

    //by type
    i32         i;
    i32         v;
    BOOL        b_neg;
    u32         u;
    i32         d;

    c = word;

    // check negative
    b_neg = ICLI_IS_(MINUS, *c);

    if ( b_neg ) {
        ++c;
        if ( ICLI_IS_(EOS, *c) ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    if ( (*c) != '0' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    } else if ( (*c) != 'b' && (*c) != 'B' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    v = 0;
    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        i = _binary_get_c(*c);
        if ( i == -1 ) {
            _ERR_MATCH_RETURN();
        }

        // check overflow
        if ( b_neg ) {
            u = (u32)(v << 1) + (u32)i;
            if ( u > _U_INT_MIN ) {
                _ERR_MATCH_RETURN();
            }
        } else {
            if ( v ) {
                d = ICLI_MAX_INT / v;
                if ( d < 2 ) {
                    _ERR_MATCH_RETURN();
                } else if ( d == 2 ) {
                    if ( i > (ICLI_MAX_INT - (v * 2)) ) {
                        _ERR_MATCH_RETURN();
                    }
                }
            }
        }

        // update value
        v = (v << 1) + i;
    }

    if ( b_neg ) {
        (*val) = 0 - v;
    } else {
        (*val) = v;
    }
    return ICLI_RC_OK;
}

/*
    check if 32-bit signed octal

    INPUT
        word : user input word

    OUTPUT
        val     : 32-bit signed octal
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 __signed_octal_get(
    IN  char    *word,
    OUT i32     *val,
    OUT u32     *err_pos
)
{
    //common
    char        *c;

    //by type
    i32         i;
    i32         v;
    BOOL        b_neg;
    u32         u;
    i32         d;

    c = word;

    // check negative
    b_neg = ICLI_IS_(MINUS, *c);

    if ( b_neg ) {
        ++c;
        if ( ICLI_IS_(EOS, *c) ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    if ( (*c) != '0' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    } else if ( (*c) != 'o' && (*c) != 'O' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    v = 0;
    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        i = _octal_get_c(*c);
        if ( i == -1 ) {
            _ERR_MATCH_RETURN();
        }

        // check overflow
        if ( b_neg ) {
            u = (u32)(v << 3) + (u32)i;
            if ( u > _U_INT_MIN ) {
                _ERR_MATCH_RETURN();
            }
        } else {
            if ( v ) {
                d = ICLI_MAX_INT / v;
                if ( d < 8 ) {
                    _ERR_MATCH_RETURN();
                } else if ( d == 8 ) {
                    if ( i > (ICLI_MAX_INT - (v * 8)) ) {
                        _ERR_MATCH_RETURN();
                    }
                }
            }
        }

        // update alue
        v = (v << 3) + i;
    }

    if ( b_neg ) {
        (*val) = 0 - v;
    } else {
        (*val) = v;
    }
    return ICLI_RC_OK;
}

/*
    check if 32-bit signed hex

    INPUT
        word : user input word

    OUTPUT
        val     : 32-bit signed hex
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 __signed_hex_get(
    IN  char    *word,
    OUT i32     *val,
    OUT u32     *err_pos
)
{
    //common
    char        *c;

    //by type
    i32         i;
    i32         v;
    BOOL        b_neg;
    u32         u;
    i32         d;

    c = word;

    // check negative
    b_neg = ICLI_IS_(MINUS, *c);

    if ( b_neg ) {
        ++c;
        if ( ICLI_IS_(EOS, *c) ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    if ( (*c) != '0' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    } else if ( (*c) != 'x' && (*c) != 'X' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    v = 0;
    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        i = _hex_get_c(*c);
        if ( i == -1 ) {
            _ERR_MATCH_RETURN();
        }

        // check overflow
        if ( b_neg ) {
            u = (u32)(v << 4) + (u32)i;
            if ( u > _U_INT_MIN ) {
                _ERR_MATCH_RETURN();
            }
        } else {
            if ( v ) {
                d = ICLI_MAX_INT / v;
                if ( d < 16 ) {
                    _ERR_MATCH_RETURN();
                } else if ( d == 16 ) {
                    if ( i > (ICLI_MAX_INT - (v * 16)) ) {
                        _ERR_MATCH_RETURN();
                    }
                }
            }
        }

        // update value
        v = (v << 4) + i;
    }

    if ( b_neg ) {
        (*val) = 0 - v;
    } else {
        (*val) = v;
    }
    return ICLI_RC_OK;
}

/*
    check if 32-bit signed decimal

    INPUT
        word : user input word

    OUTPUT
        val     : 32-bit signed decimal
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 __signed_decimal_get(
    IN  char    *word,
    OUT i32     *val,
    OUT u32     *err_pos
)
{
    //common
    char        *c;

    //by type
    i32         i;
    i32         v;
    BOOL        b_neg;
    u32         u;
    i32         d;

    c = word;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    // check negative
    b_neg = ICLI_IS_(MINUS, *c);

    if ( b_neg ) {
        ++c;
        if ( ICLI_IS_(EOS, *c) ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    v = 0;
    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        i = _digit_get_c(*c);
        if ( i == -1 ) {
            _ERR_MATCH_RETURN();
        }

        // check overflow
        if ( b_neg ) {
            u = (u32)(v * 10) + (u32)i;
            if ( u > _U_INT_MIN ) {
                _ERR_MATCH_RETURN();
            }
        } else {
            if ( v ) {
                d = ICLI_MAX_INT / v;
                if ( d < 10 ) {
                    _ERR_MATCH_RETURN();
                } else if ( d == 10 ) {
                    if ( i > (ICLI_MAX_INT - (v * 10)) ) {
                        _ERR_MATCH_RETURN();
                    }
                }
            }
        }

        // update v
        v = v * 10 + i;
    }

    if ( b_neg ) {
        (*val) = 0 - v;
    } else {
        (*val) = v;
    }
    return ICLI_RC_OK;
}

/*
    get 32-bit unsigned integer
    this will try any format, decimal, hex, binary, then octal.

    INPUT
        word : user input word

    OUTPUT
        val     : unsigned 32-bit integer value
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 __u32_get(
    IN  char    *word,
    OUT u32     *val,
    OUT u32     *err_pos
)
{
    i32     rc;
    i32     r = ICLI_RC_ERR_MATCH;

    /* decimal */
    rc = __decimal_get(word, val, err_pos);
    switch ( rc ) {
    case ICLI_RC_OK:
        return ICLI_RC_OK;

    case ICLI_RC_ERR_INCOMPLETE:
        r = ICLI_RC_ERR_INCOMPLETE;
        break;

    default:
        break;
    }

    /* hex */
    rc = __hex_get(word, val, err_pos);
    switch ( rc ) {
    case ICLI_RC_OK:
        return ICLI_RC_OK;

    case ICLI_RC_ERR_INCOMPLETE:
        r = ICLI_RC_ERR_INCOMPLETE;
        break;

    default:
        break;
    }

    /* binary */
    rc = __binary_get(word, val, err_pos);
    switch ( rc ) {
    case ICLI_RC_OK:
        return ICLI_RC_OK;

    case ICLI_RC_ERR_INCOMPLETE:
        r = ICLI_RC_ERR_INCOMPLETE;
        break;

    default:
        break;
    }

    /* octal */
    rc = __octal_get(word, val, err_pos);
    switch ( rc ) {
    case ICLI_RC_OK:
        return ICLI_RC_OK;

    case ICLI_RC_ERR_INCOMPLETE:
        r = ICLI_RC_ERR_INCOMPLETE;
        break;

    default:
        break;
    }

    return r;
}

/*
    get 32-bit signed integer
    if positive, this will try any format, decimal, hex, binary, then octal.

    INPUT
        word : user input word

    OUTPUT
        val     : 32-bit signed integer value
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 __i32_get(
    IN  char    *word,
    OUT i32     *val,
    OUT u32     *err_pos
)
{
    i32     rc;
    i32     r = ICLI_RC_ERR_MATCH;

    /* decimal */
    rc = __signed_decimal_get(word, val, err_pos);
    switch ( rc ) {
    case ICLI_RC_OK:
        return ICLI_RC_OK;

    case ICLI_RC_ERR_INCOMPLETE:
        r = ICLI_RC_ERR_INCOMPLETE;
        break;

    default:
        break;
    }

    /* hex */
    rc = __signed_hex_get(word, val, err_pos);
    switch ( rc ) {
    case ICLI_RC_OK:
        return ICLI_RC_OK;

    case ICLI_RC_ERR_INCOMPLETE:
        r = ICLI_RC_ERR_INCOMPLETE;
        break;

    default:
        break;
    }

    /* binary */
    rc = __signed_binary_get(word, val, err_pos);
    switch ( rc ) {
    case ICLI_RC_OK:
        return ICLI_RC_OK;

    case ICLI_RC_ERR_INCOMPLETE:
        r = ICLI_RC_ERR_INCOMPLETE;
        break;

    default:
        break;
    }

    /* octal */
    rc = __signed_octal_get(word, val, err_pos);
    switch ( rc ) {
    case ICLI_RC_OK:
        return ICLI_RC_OK;

    case ICLI_RC_ERR_INCOMPLETE:
        r = ICLI_RC_ERR_INCOMPLETE;
        break;

    default:
        break;
    }

    return r;
}
#endif /* CP, 2012/10/03 12:33, Bugzilla#9873 - accept any format */

/*
    translate IPv4 VLSM(Variable Length Subnet Mask) to netmask

    INPUT
        vlsm : Variable Length Subnet Mask, <= 32
    OUTPUT
        netmask : IPv4 netmask
    RETURN
        icli_rc_t
*/
static i32 _ipv4_vlsm_to_netmask(
    IN  u32         vlsm,
    OUT vtss_ip_t   *netmask
)
{
    u32   i;

    if ( vlsm > 32 ) {
        return ICLI_RC_ERR_PARAMETER;
    }

    (*netmask) = 0;
    for ( i = 0; i < vlsm; ++i ) {
        (*netmask) |= ( 0x1L << (31 - i) );
    }
    return ICLI_RC_OK;
}

/*
    translate IPv6 VLSM(Variable Length Subnet Mask) to netmask

    INPUT
        vlsm : Variable Length Subnet Mask, <= 128
    OUTPUT
        netmask : IPv6 netmask
    RETURN
        icli_rc_t
*/
static i32 _ipv6_vlsm_to_netmask(
    IN  u32             vlsm,
    OUT vtss_ipv6_t     *netmask
)
{
    u32   i;

    if ( vlsm > 128 ) {
        return ICLI_RC_ERR_PARAMETER;
    }
    memset( netmask, 0, sizeof(vtss_ipv6_t) );
    for ( i = 0; i < vlsm; ++i ) {
        netmask->addr[i / 8] |= ( 0x01 << ( 7 - (i % 8) ) );
    }
    return ICLI_RC_OK;
}

/*
    check if IPv6 address is 0 or not

    INPUT
        ipv6 : IPv6 address
    OUTPUT
        n/a
    RETURN
        TRUE  : is 0
        FALSE : not 0
*/
static BOOL _ipv6_is_0(
    IN  vtss_ipv6_t     *ipv6
)
{
    u32   i;

    for ( i = 0; i < 16; ++i ) {
        if ( ipv6->addr[i] != 0 ) {
            return FALSE;
        }
    }
    return TRUE;
}

/*
    check if 32-bit signed integer

    INPUT
        word : user input word
        min  : minimum value of the range
        max  : maximum value of the range
    OUTPUT
        val     : signed integer value
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _int_get_in_range(
    IN  char    *word,
    IN  i32     min,
    IN  i32     max,
    OUT i32     *val,
    OUT u32     *err_pos
)
#if 1 /* CP, 2012/10/03 12:33, Bugzilla#9873 - accept any format */
{
    i32     rc;
    i32     v = 0;

    rc = __i32_get(word, &v, err_pos);
    switch ( rc ) {
    case ICLI_RC_OK:
        if ( v >= min && v <= max ) {
            (*val) = v;
            return ICLI_RC_OK;
        } else {
#if 1 /* just error */
            return ICLI_RC_ERR_MATCH;
#else
            if ( v < 0 ) {
                if ( v > max ) {
                    return ICLI_RC_ERR_INCOMPLETE;
                } else {
                    return ICLI_RC_ERR_MATCH;
                }
            } else {
                if ( v < min ) {
                    return ICLI_RC_ERR_INCOMPLETE;
                } else {
                    return ICLI_RC_ERR_MATCH;
                }
            }
#endif
        }

    default:
        return rc;
    }
}
#else /* CP, 2012/10/03 12:33, Bugzilla#9873 - accept any format */
{
    //common
    char    *c = word;

    //by type
    u32     i,
            d,
            comp;
    i32     j;
    BOOL    neg;

    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    //check negative
    neg = ((*c) == '-');

    if ( max < 0 ) {
        if ( neg == FALSE ) {
            _ERR_MATCH_RETURN();
        }
    }

    if ( min > 0 ) {
        if ( neg ) {
            _ERR_MATCH_RETURN();
        }
    }

    if ( neg ) {
        if ((*(c + 1)) == 0) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
        ++c;
    }

    if ( neg ) {
        comp = (u32)(0 - min);
    } else {
        comp = (u32)max;
    }

    for ( i = 0; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_DIGIT(*c) ) {
            //get value
            if ( i ) {
                d = comp / i;
                if ( d < 10 ) {
                    _ERR_MATCH_RETURN();
                } else if ( d == 10 ) {
                    if ( (u32)((*c) - '0') > (u32)(comp - (i * 10)) ) {
                        _ERR_MATCH_RETURN();
                    }
                }
            }
            i = (i * 10) + ((*c) - '0');
            if ( i > comp ) {
                _ERR_MATCH_RETURN();
            }
        } else {
            _ERR_MATCH_RETURN();
        }
    }

    if ( neg ) {
        j = 0 - i;
        if ( j > max ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    } else {
        j = (i32)i;
        if ( j < min ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    *val = j;
    return ICLI_RC_OK;
}
#endif /* CP, 2012/10/03 12:33, Bugzilla#9873 - accept any format */

/*
    check if 8-bit signed integer

    INPUT
        word : user input word
    OUTPUT
        val     : signed integer value
        err_pos : position where error happens
    RETURN
        0  if successful
        -1 if invalid
*/
static i32 _int8_get(
    IN  char    *word,
    OUT i8      *val,
    OUT u32     *err_pos
)
{
    i32     i, r;

    r = _int_get_in_range(word, ICLI_MIN_INT8, ICLI_MAX_INT8, &i, err_pos);
    if ( r == ICLI_RC_OK ) {
        *val = (i8)i;
    }
    return r;
}

/*
    check if 16-bit signed integer

    INPUT
        word : user input word
    OUTPUT
        val     : signed integer value
        err_pos : position where error happens
    RETURN
        0  if successful
        -1 if invalid
*/
static i32 _int16_get(
    IN  char    *word,
    OUT i16     *val,
    OUT u32     *err_pos
)
{
    i32     i, r;

    r = _int_get_in_range(word, ICLI_MIN_INT16, ICLI_MAX_INT16, &i, err_pos);
    if ( r == ICLI_RC_OK ) {
        *val = (i16)i;
    }
    return r;
}

/*
    check if 32-bit signed integer

    INPUT
        word : user input word
    OUTPUT
        val     : signed integer value
        err_pos : position where error happens
    RETURN
        0  if successful
        -1 if invalid
*/
static i32 _int_get(
    IN  char    *word,
    OUT i32     *val,
    OUT u32     *err_pos
)
{
    return _int_get_in_range(word, ICLI_MIN_INT, ICLI_MAX_INT, val, err_pos);
}

/*
    check if unsigned 32-bit integer in a range

    INPUT
        word : user input word
        min  : minimum value of the range
        max  : maximum value of the range
    OUTPUT
        val     : unsigned 32-bit integer value
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _uint_get_in_range(
    IN  char    *word,
    IN  u32     min,
    IN  u32     max,
    OUT u32     *val,
    OUT u32     *err_pos
)
#if 1 /* CP, 2012/10/03 12:33, Bugzilla#9873 - accept any format */
{
    i32     rc;
    u32     v = 0;

    rc = __u32_get(word, &v, err_pos);
    switch ( rc ) {
    case ICLI_RC_OK:
        if ( v >= min && v <= max ) {
            (*val) = v;
            return ICLI_RC_OK;
        } else {
#if 1 /* just error */
            return ICLI_RC_ERR_MATCH;
#else
            if ( v < min ) {
                return ICLI_RC_ERR_INCOMPLETE;
            } else {
                return ICLI_RC_ERR_MATCH;
            }
#endif
        }

    default:
        return rc;
    }
}
#else /* CP, 2012/10/03 12:33, Bugzilla#9873 - accept any format */
{
    //common
    char    *c = word;

    //by type
    u32     i = 0,
            d;

    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_DIGIT(*c) ) {
            if ( i ) {
                d = max / i;
                if ( d < 10 ) {
                    _ERR_MATCH_RETURN();
                } else if ( d == 10 ) {
                    if ( (u32)((*c) - '0') > (u32)(max - (i * 10)) ) {
                        _ERR_MATCH_RETURN();
                    }
                }
            }
            i = (i * 10) + ((*c) - '0');
            if ( i > max ) {
                _ERR_MATCH_RETURN();
            }
            continue;
        }
        _ERR_MATCH_RETURN();
    }

    if ( i < min ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    *val = i;
    return ICLI_RC_OK;
}
#endif /* CP, 2012/10/03 12:33, Bugzilla#9873 - accept any format */

/*
    check if unsigned 8-bit integer

    INPUT
        word : user input word
    OUTPUT
        val     : unsigned 8-bit integer value
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _uint8_get(
    IN  char    *word,
    OUT u8      *val,
    OUT u32     *err_pos
)
{
    u32     i;
    i32     r;

    r = _uint_get_in_range(word, ICLI_MIN_UINT8, ICLI_MAX_UINT8, &i, err_pos);
    if ( r == ICLI_RC_OK ) {
        *val = (u8)i;
    }
    return r;
}

/*
    check if unsigned 16-bit integer

    INPUT
        word : user input word
    OUTPUT
        val     : unsigned 16-bit integer value
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _uint16_get(
    IN  char    *word,
    OUT u16     *val,
    OUT u32     *err_pos
)
{
    u32     i;
    i32     r;

    r = _uint_get_in_range(word, ICLI_MIN_UINT16, ICLI_MAX_UINT16, &i, err_pos);
    if ( r == ICLI_RC_OK ) {
        *val = (u16)i;
    }
    return r;
}

/*
    check if unsigned 32-bit integer

    INPUT
        word : user input word
    OUTPUT
        val     : unsigned 32-bit integer value
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _uint_get(
    IN  char    *word,
    OUT u32     *val,
    OUT u32     *err_pos
)
{
    return _uint_get_in_range(word, ICLI_MIN_UINT, ICLI_MAX_UINT, val, err_pos);
}

/*
    check if range ID in unsigned int

    INPUT
        word : user input word
    OUTPUT
        val     : unsigned int
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _range_uint_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT u32             *val,
    OUT u32             *err_pos
)
{
    u32                     i;
    u32                     j;
    icli_unsigned_range_t   *ur;
    i32                     rc;

    if ( range->type != ICLI_RANGE_TYPE_UNSIGNED ) {
        return ICLI_RC_ERR_PARAMETER;
    }

    rc = ICLI_RC_ERROR;
    ur = &(range->u.ur);
    for ( i = 0; i < ur->cnt; ++i ) {
        rc = _uint_get_in_range( word,
                                 ur->range[i].min,
                                 ur->range[i].max,
                                 &j,
                                 err_pos );
        if ( rc == ICLI_RC_OK ) {
            *val = j;
            return ICLI_RC_OK;
        }
    }
    return rc;
}

/*
    check if range ID in signed int

    INPUT
        word : user input word
    OUTPUT
        val     : signed int
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _range_int_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT i32             *val,
    OUT u32             *err_pos
)
{
    u32                     i;
    i32                     j;
    icli_signed_range_t     *sr;
    i32                     rc;

    if ( range->type != ICLI_RANGE_TYPE_SIGNED ) {
        return ICLI_RC_ERR_PARAMETER;
    }

    rc = ICLI_RC_ERROR;
    sr = &(range->u.sr);
    for ( i = 0; i < sr->cnt; ++i ) {
        rc = _int_get_in_range( word,
                                sr->range[i].min,
                                sr->range[i].max,
                                &j,
                                err_pos );
        if ( rc == ICLI_RC_OK ) {
            *val = j;
            return ICLI_RC_OK;
        }
    }
    return rc;
}

/*
    check if any Ethernet MAC address
    hh:hh:hh:hh:hh:hh

    INPUT
        word : user input word
    OUTPUT
        val     : any mac address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _mac_addr_get_0(
    IN  char        *word,
    OUT vtss_mac_t  *val,
    OUT u32         *err_pos
)
{
    //common
    char        *c;
    //by type
    i32         mac[6] = { 0, 0, 0, 0, 0, 0 };
    i32         mac_index = 0;
    i32         h;
    i32         h_cnt = 0;

    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        h = _hex_get_c(*c);
        if ( h != -1 ) {
            //valid hex value
            ++h_cnt;
            //get mac
            mac[mac_index] = (mac[mac_index] << 4) + h;
            if ( mac[mac_index] > 0xFF ) {
                _ERR_MATCH_RETURN();
            }
        } else if ( ICLI_IS_(MAC_A_DELIMITER, *c) || ICLI_IS_(MAC_Z_DELIMITER, *c) ) {
            //continuous ::
            if ( h_cnt == 0 ) {
                _ERR_MATCH_RETURN();
            }
            //too long
            if ( (++mac_index) > 5 ) {
                _ERR_MATCH_RETURN();
            }
            //update
            h_cnt = 0;
        } else {
            //invalid character
            _ERR_MATCH_RETURN();
        }
    }

    if ( mac_index < 5 || h_cnt == 0 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    for ( h_cnt = 0; h_cnt < 6; ++h_cnt ) {
        val->addr[h_cnt] = (u8)(mac[h_cnt]);
    }
    return ICLI_RC_OK;
}

/*
    check if unicast Ethernet MAC address
    hm:hh:hh:hh:hh:hh, (hm & 0x01) != 0x01

    INPUT
        word : user input word
    OUTPUT
        val     : unicast mac address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _mac_ucast_get_0(
    IN  char        *word,
    OUT vtss_mac_t  *val,
    OUT u32         *err_pos
)
{
    //common
    char        *c;
    //by type
    vtss_mac_t  mac_ucast;
    i32         rc;

    rc = _mac_addr_get_0(word, &mac_ucast, err_pos);
    if ( rc != ICLI_RC_OK ) {
        //error
        return rc;
    }

    if ((mac_ucast.addr[0] & 0x01) == 0x01) {
        //find the first dilimiter
        for ( c = word; ICLI_NOT_(MAC_A_DELIMITER, *c) && ICLI_NOT_(MAC_Z_DELIMITER, *c); ++c ) {
            ;
        }
        --c;
        _ERR_MATCH_RETURN();
    }

    memcpy(val, &mac_ucast, sizeof(vtss_mac_t));
    return ICLI_RC_OK;
}

/*
    check if multicast Ethernet MAC address
    hm:hh:hh:hh:hh:hh, (hm & 0x01) == 0x01

    INPUT
        word : user input word
    OUTPUT
        val     : multicast mac address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _mac_mcast_get_0(
    IN  char        *word,
    OUT vtss_mac_t  *val,
    OUT u32         *err_pos
)
{
    //common
    char        *c;
    //by type
    vtss_mac_t  mac_mcast;
    i32         rc;

    rc = _mac_addr_get_0(word, &mac_mcast, err_pos);
    if ( rc != ICLI_RC_OK ) {
        //error
        return rc;
    }

    if ((mac_mcast.addr[0] & 0x01) != 0x01) {
        //find the first dilimiter
        for ( c = word; ICLI_NOT_(MAC_A_DELIMITER, *c) && ICLI_NOT_(MAC_Z_DELIMITER, *c); ++c ) {
            ;
        }
        --c;
        _ERR_MATCH_RETURN();
    }

    memcpy(val, &mac_mcast, sizeof(vtss_mac_t));
    return ICLI_RC_OK;
}

/*
    check if any clock ID
    hh:hh:hh:hh:hh:hh:hh:hh

    INPUT
        word : user input word
    OUTPUT
        val     : clock ID
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _clock_id_get_0(
    IN  char                *word,
    OUT icli_clock_id_t     *val,
    OUT u32                 *err_pos
)
{
    //common
    char        *c;
    //by type
    i32         mac[ICLI_CLOCK_ID_SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    i32         mac_index = 0;
    i32         h;
    i32         h_cnt = 0;

    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        h = _hex_get_c(*c);
        if ( h != -1 ) {
            //valid hex value
            ++h_cnt;
            //get mac
            mac[mac_index] = (mac[mac_index] << 4) + h;
            if ( mac[mac_index] > 0xFF ) {
                _ERR_MATCH_RETURN();
            }
        } else if ( ICLI_IS_(MAC_A_DELIMITER, *c) || ICLI_IS_(MAC_Z_DELIMITER, *c) ) {
            //continuous ::
            if ( h_cnt == 0 ) {
                _ERR_MATCH_RETURN();
            }
            //too long
            if ( (++mac_index) > (ICLI_CLOCK_ID_SIZE - 1) ) {
                _ERR_MATCH_RETURN();
            }
            //update
            h_cnt = 0;
        } else {
            //invalid character
            _ERR_MATCH_RETURN();
        }
    }

    if ( mac_index < (ICLI_CLOCK_ID_SIZE - 1) || h_cnt == 0 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    for ( h_cnt = 0; h_cnt < ICLI_CLOCK_ID_SIZE; ++h_cnt ) {
        val->id[h_cnt] = (u8)(mac[h_cnt]);
    }
    return ICLI_RC_OK;
}

/*
    check if any Ethernet MAC address
    hhhh.hhhh.hhhh

    INPUT
        word : user input word
    OUTPUT
        val     : any mac address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _mac_addr_get_1(
    IN  char        *word,
    OUT vtss_mac_t  *val,
    OUT u32         *err_pos
)
{
    //common
    char        *c;
    //by type
    i32         mac[3] = {0, 0, 0};
    i32         mac_index = 0;
    i32         h = 0;
    i32         h_cnt = 0;

    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        h = _hex_get_c(*c);
        if ( h != -1 ) {
            //valid hex value
            ++h_cnt;
            //get mac
            mac[mac_index] = (mac[mac_index] << 4) + h;
            if ( mac[mac_index] > 0xffFF ) {
                _ERR_MATCH_RETURN();
            }
        } else if ( ICLI_IS_(MAC_C_DELIMITER, *c) ) {
            //continuous ..
            if ( h_cnt == 0 ) {
                _ERR_MATCH_RETURN();
            }
            //too long
            if ( (++mac_index) > 2 ) {
                _ERR_MATCH_RETURN();
            }
            //update
            h_cnt = 0;
        } else {
            //invalid character
            _ERR_MATCH_RETURN();
        }
    }

    if ( mac_index < 2 || h_cnt == 0 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    val->addr[0] = (u8) ((mac[0] & 0xFF00) >> 8);
    val->addr[1] = (u8) ((mac[0] & 0x00FF));
    val->addr[2] = (u8) ((mac[1] & 0xFF00) >> 8);
    val->addr[3] = (u8) ((mac[1] & 0x00FF));
    val->addr[4] = (u8) ((mac[2] & 0xFF00) >> 8);
    val->addr[5] = (u8) ((mac[2] & 0x00FF));
    return ICLI_RC_OK;
}

/*
    check if unicast Ethernet MAC address
    hmhh.hhhh.hhhh, (hm & 0x01) != 0x01

    INPUT
        word : user input word
    OUTPUT
        val     : unicast mac address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _mac_ucast_get_1(
    IN  char        *word,
    OUT vtss_mac_t  *val,
    OUT u32         *err_pos
)
{
    //common
    char        *c;
    //by type
    vtss_mac_t  mac_ucast;
    i32         rc;

    rc = _mac_addr_get_1(word, &mac_ucast, err_pos);
    if ( rc != ICLI_RC_OK ) {
        //error
        return rc;
    }

    if ((mac_ucast.addr[0] & 0x01) == 0x01) {
        //find the first dilimiter
        for ( c = word; ICLI_NOT_(MAC_C_DELIMITER, *c); ++c ) {
            ;
        }
        switch (c - word) {
        case 1 :
            c -= 1;
            break;
        case 2 :
            c -= 2;
            break;
        case 3 :
        case 4 :
        default :
            c -= 3;
            break;
        }
        _ERR_MATCH_RETURN();
    }

    memcpy(val, &mac_ucast, sizeof(vtss_mac_t));
    return ICLI_RC_OK;
}

/*
    check if multicast Ethernet MAC address
    hmhh.hhhh.hhhh, (hm & 0x01) == 0x01

    INPUT
        word : user input word
    OUTPUT
        val     : multicast mac address
        err_pos : position where error happens
    RETURN
       icli_rc_t
*/
static i32 _mac_mcast_get_1(
    IN  char        *word,
    OUT vtss_mac_t  *val,
    OUT u32         *err_pos
)
{
    //common
    char        *c;
    //by type
    vtss_mac_t  mac_mcast;
    i32         rc;

    rc = _mac_addr_get_1(word, &mac_mcast, err_pos);
    if ( rc != ICLI_RC_OK ) {
        //error
        return rc;
    }

    if ((mac_mcast.addr[0] & 0x01) != 0x01) {
        //find the first dilimiter
        for ( c = word; ICLI_NOT_(MAC_C_DELIMITER, *c); ++c ) {
            ;
        }
        switch (c - word) {
        case 1 :
            c -= 1;
            break;
        case 2 :
            c -= 2;
            break;
        case 3 :
        case 4 :
        default :
            c -= 3;
            break;
        }
        _ERR_MATCH_RETURN();
    }

    memcpy(val, &mac_mcast, sizeof(vtss_mac_t));
    return ICLI_RC_OK;
}

/*
    check if clock ID
    hhhh.hhhh.hhhh.hhhh

    INPUT
        word : user input word
    OUTPUT
        val     : clock ID
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _clock_id_get_1(
    IN  char                *word,
    OUT icli_clock_id_t     *val,
    OUT u32                 *err_pos
)
{
    //common
    char        *c;
    //by type
    i32         mac[ICLI_CLOCK_ID_SIZE / 2] = {0, 0, 0, 0};
    i32         mac_index = 0;
    i32         h = 0;
    i32         h_cnt = 0;

    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        h = _hex_get_c(*c);
        if ( h != -1 ) {
            //valid hex value
            ++h_cnt;
            //get mac
            mac[mac_index] = (mac[mac_index] << 4) + h;
            if ( mac[mac_index] > 0xffFF ) {
                _ERR_MATCH_RETURN();
            }
        } else if ( ICLI_IS_(MAC_C_DELIMITER, *c) ) {
            //continuous ..
            if ( h_cnt == 0 ) {
                _ERR_MATCH_RETURN();
            }
            //too long
            if ( (++mac_index) > (ICLI_CLOCK_ID_SIZE / 2 - 1) ) {
                _ERR_MATCH_RETURN();
            }
            //update
            h_cnt = 0;
        } else {
            //invalid character
            _ERR_MATCH_RETURN();
        }
    }

    if ( mac_index < (ICLI_CLOCK_ID_SIZE / 2 - 1) || h_cnt == 0 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    val->id[0] = (u8) ((mac[0] & 0xFF00) >> 8);
    val->id[1] = (u8) ((mac[0] & 0x00FF));
    val->id[2] = (u8) ((mac[1] & 0xFF00) >> 8);
    val->id[3] = (u8) ((mac[1] & 0x00FF));
    val->id[4] = (u8) ((mac[2] & 0xFF00) >> 8);
    val->id[5] = (u8) ((mac[2] & 0x00FF));
    val->id[6] = (u8) ((mac[3] & 0xFF00) >> 8);
    val->id[7] = (u8) ((mac[3] & 0x00FF));
    return ICLI_RC_OK;
}

/*
    check if any IPv4 address < max
    ddd.ddd.ddd.ddd

    INPUT
        word : user input word
        max : max value for each ddd
    OUTPUT
        val     : any ipv4 address < max
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv4_addr_get_in_range(
    IN  char        *word,
    IN  i32         max,
    OUT vtss_ip_t   *val,
    OUT u32         *err_pos
)
{
    //common
    char    *c;
    //by type
    i32     ip[4] = {0, 0, 0, 0},
                    ip_index = 0,
                    d,
                    d_cnt = 0;

    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        d = _digit_get_c( *c );
        if ( d != -1 ) {
            //valid digit
            ++d_cnt;
            //get ip
            ip[ip_index] = (ip[ip_index] * 10) + d;
            if (ip[ip_index] > max) {
                _ERR_MATCH_RETURN();
            }
        } else if ( ICLI_IS_(IPV4_DELIMITER, *c) ) {
            //continous ..
            if ( d_cnt == 0 ) {
                _ERR_MATCH_RETURN();
            }
            //too long
            if ( (++ip_index) > 3 ) {
                _ERR_MATCH_RETURN();
            }
            //update
            d_cnt = 0;
        } else {
            _ERR_MATCH_RETURN();
        }
    }

    if ( ip_index < 3 || d_cnt == 0 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    /* Little endian */
    (*val) = (vtss_ip_t)( (ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3] );
    return ICLI_RC_OK;
}

/*
    check if any IPv4 address
    ddd.ddd.ddd.ddd

    INPUT
        word : user input word
    OUTPUT
        val     : any ipv4 address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv4_addr_get(
    IN  char        *word,
    OUT vtss_ip_t   *val,
    OUT u32         *err_pos
)
{
    return _ipv4_addr_get_in_range( word, 0xFF, val, err_pos );
}

/*
    check if IPv4 unicast address
    ddd.ddd.ddd.ddd outside 224.0.0.0 to 239.255.255.255,
    and check ip is ddd.ddd.ddd.0 or ddd.ddd.ddd.255

    INPUT
        word : user input word
    OUTPUT
        val     : unicast ipv4 address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv4_ucast_get(
    IN  char        *word,
    OUT vtss_ip_t   *val,
    OUT u32         *err_pos
)
{
    //common
    char    *c;
    //by type
    vtss_ip_t   ucast_ip;
    u8          u;
    i32         rc;

    rc = _ipv4_addr_get(word, &ucast_ip, err_pos);
    if ( rc != ICLI_RC_OK ) {
        return rc;
    }

    /* check first byte */
    u = (u8)( (ucast_ip & 0xff000000) >> 24 );

    //go to first non-zero digit
    for ( c = word; *c == '0'; ++c ) {
        ;
    }

    /*
        Check for valid unicast address
        the checking follows web IP address checking
    */
    if ( u == 0 || u == 127 ) {
        _ERR_MATCH_RETURN();
    } else if ( u >= 224 ) {
        if ( *(c + 1) == '2' ) {
            c += 2;
        } else {
            c += 1;
        }
        _ERR_MATCH_RETURN();
    }

#if 0 /* wrong checking */
    /* check last byte */
    u = (u8)( (ucast_ip & 0x000000ff) );
    if ( u == 0 || u == 255 ) {
        c = word + vtss_icli_str_len(word) - 1;
        _ERR_MATCH_RETURN();
    }
#endif

    *val = ucast_ip;
    return ICLI_RC_OK;
}

/*
    check if IPv4 multicast address
    ddd.ddd.ddd.ddd outside 224.0.0.0 to 239.255.255.255

    INPUT
        word : user input word
    OUTPUT
        val     : multicast ipv4 address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv4_mcast_get(
    IN  char        *word,
    OUT vtss_ip_t   *val,
    OUT u32         *err_pos
)
{
    //common
    char        *c;
    //by type
    vtss_ip_t   mcast_ip;
    u8          u;
    i32         rc;

    rc = _ipv4_addr_get(word, &mcast_ip, err_pos);
    if ( rc != ICLI_RC_OK ) {
        return rc;
    }

    u = (u8)( (mcast_ip & 0xff000000) >> 24 );

    //go to first non-zero digit
    for ( c = word; *c == '0'; ++c ) {
        ;
    }

    /* Check for valid multicast address */
    if ( u < 224 ) {
        if ( u > 199 ) {
            if ( *(c + 1) == '2' ) {
                c += 2;
            } else {
                c += 1;
            }
        }
        _ERR_MATCH_RETURN();
    } else if ( u > 239 ) {
        c += 1;
        _ERR_MATCH_RETURN();
    }

    *val = mcast_ip;
    return ICLI_RC_OK;
}

/*
    check if IPv4 non-multicast address
    that is IP address outside 224.0.0.0 to 239.255.255.255,

    INPUT
        word : user input word

    OUTPUT
        val     : non-multicast ipv4 address
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 _ipv4_nmcast_get(
    IN  char        *word,
    OUT vtss_ip_t   *val,
    OUT u32         *err_pos
)
{
    //common
    char    *c;
    //by type
    vtss_ip_t   ucast_ip;
    u8          u;
    i32         rc;

    rc = _ipv4_addr_get(word, &ucast_ip, err_pos);
    if ( rc != ICLI_RC_OK ) {
        return rc;
    }

    /* check first byte */
    u = (u8)( (ucast_ip & 0xff000000) >> 24 );

    //go to first non-zero digit
    for ( c = word; *c == '0'; ++c ) {
        ;
    }

    /* Check for valid unicast address */
    if ( (u >= 224) && (u <= 239) ) {
        if ( *(c + 1) == '2' ) {
            c += 2;
        } else if ( *(c + 1) == '3' ) {
            c += 1;
        }
        _ERR_MATCH_RETURN();
    }

    *val = ucast_ip;
    return ICLI_RC_OK;
}

/*
    check if IPv4 address is in class A,B,C
    that is 0 - 223

    INPUT
        word : user input word

    OUTPUT
        val     : non-multicast ipv4 address
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 _ipv4_abc_get(
    IN  char        *word,
    OUT vtss_ip_t   *val,
    OUT u32         *err_pos
)
{
    //common
    char        *c = word;
    //by type
    vtss_ip_t   ucast_ip;
    u32         u;
    i32         rc;

    rc = _ipv4_addr_get(word, &ucast_ip, err_pos);
    if ( rc != ICLI_RC_OK ) {
        return rc;
    }

    /* check first byte */
    u = (ucast_ip & 0xff000000) >> 24;

    /* Check for class A,B,C */
    if ( u > 223 ) {
        _ERR_MATCH_RETURN();
    }

    *val = ucast_ip;
    return ICLI_RC_OK;
}

/*
    check if IPv4 netmask
    ddd.ddd.ddd.ddd with continuous-bit 1 then 0 block

    INPUT
        word : user input word
    OUTPUT
        val     : ipv4 address
        err_pos : position where error happens
    RETURN
        icli_rc_t
    COMMENT
        for C, err_pos is not important because it is not possible to
        indicate the exact position for error
*/
static i32 _ipv4_netmask_get(
    IN  char        *word,
    OUT vtss_ip_t   *val,
    OUT u32         *err_pos
)
{
    //common
    char    *c;
    //by type
    i32     ip[4] = {0, 0, 0, 0},
                    ip_index = 0,
                    d,
                    d_cnt = 0,
                    b_one = 1,
                    n,
                    bit;

    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        d = _digit_get_c( *c );
        if ( d != -1 ) {
            //valid digit
            ++d_cnt;
            //get ip
            ip[ip_index] = (ip[ip_index] * 10) + d;
            //overflow
            if (ip[ip_index] > 0xFF) {
                _ERR_MATCH_RETURN();
            }
            //check netmask 1
            if ( b_one == 0 && ip[ip_index] ) {
                _ERR_MATCH_RETURN();
            }
        } else if ( ICLI_IS_(IPV4_DELIMITER, *c) ) {
            //continous ..
            if ( d_cnt == 0 ) {
                _ERR_MATCH_RETURN();
            }
            //check netmask 2
            if ( ip[ip_index] ) {
                //for b_one == 1 only, because b_one == 0 at check netmask 1
                if ( ip[ip_index] != 0xFF ) {
                    for ( n = 7; n >= 0; --n ) {
                        bit = (ip[ip_index]) & ( 0x1L << n );
                        if ( bit ) {
                            if ( b_one == 0 ) {
                                --c;
                                _ERR_MATCH_RETURN();
                            }
                        } else if ( b_one ) {
                            b_one = 0;
                        }
                    }
                }
            } else if ( b_one ) {
                b_one = 0;
            }
            //too long
            if ( (++ip_index) > 3 ) {
                _ERR_MATCH_RETURN();
            }
            //update
            d_cnt = 0;
        } else {
            _ERR_MATCH_RETURN();
        }
    }

    if ( ip_index == 3 && d_cnt ) {
        //check netmask 3
        if ( ip[ip_index] ) {
            //for b_one == 1 only, because b_one == 0 at check netmask 1
            if ( ip[ip_index] != 0xFF ) {
                for ( n = 7; n >= 0; --n ) {
                    bit = (ip[ip_index]) & ( 0x1L << n );
                    if ( bit ) {
                        if ( b_one == 0 ) {
                            --c;
                            _ERR_MATCH_RETURN();
                        }
                    } else if ( b_one ) {
                        b_one = 0;
                    }
                }
            }
        }
    } else {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    /* Little endian */
    (*val) = (vtss_ip_t)( (ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3] );
    return ICLI_RC_OK;
}

/*
    check if IPv4 wildcard
    ddd.ddd.ddd.ddd with continuous-bit 0 then 1 block

    INPUT
        word : user input word
    OUTPUT
        val     : ipv4 address
        err_pos : position where error happens
    RETURN
        icli_rc_t
    COMMENT
        for C, err_pos is not important because it is not possible to
        indicate the exact position for error
*/
static i32 _ipv4_wildcard_get(
    IN  char        *word,
    OUT vtss_ip_t   *val,
    OUT u32         *err_pos
)
{
    //common
    char    *c;
    //by type
    i32     ip[4] = {0, 0, 0, 0},
                    ip_index = 0,
                    d,
                    d_cnt = 0,
                    b_one = 0,
                    n,
                    bit;

    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        d = _digit_get_c( *c );
        if ( d != -1 ) {
            //valid digit
            ++d_cnt;
            //get ip
            ip[ip_index] = (ip[ip_index] * 10) + d;
            //overflow
            if (ip[ip_index] > 0xFF) {
                _ERR_MATCH_RETURN();
            }
        } else if ( ICLI_IS_(IPV4_DELIMITER, *c) ) {
            //continous ..
            if ( d_cnt == 0 ) {
                _ERR_MATCH_RETURN();
            }
            //check netmask 2
            if ( ip[ip_index] ) {
                for ( n = 7; n >= 0; --n ) {
                    bit = (ip[ip_index]) & ( 0x1L << n );
                    if ( bit ) {
                        b_one = 1;
                    } else if ( b_one ) {
                        --c;
                        _ERR_MATCH_RETURN();
                    }
                }
            } else if ( b_one ) {
                --c;
                _ERR_MATCH_RETURN();
            }
            //too long
            if ( (++ip_index) > 3 ) {
                _ERR_MATCH_RETURN();
            }
            //update
            d_cnt = 0;
        } else {
            _ERR_MATCH_RETURN();
        }
    }

    if ( ip_index == 3 && d_cnt ) {
        //check netmask 3
        if ( ip[ip_index] ) {
            for ( n = 7; n >= 0; --n ) {
                bit = (ip[ip_index]) & ( 0x1L << n );
                if ( bit ) {
                    b_one = 1;
                } else if ( b_one ) {
                    _ERR_MATCH_RETURN();
                }
            }
        } else if ( b_one ) {
            _ERR_MATCH_RETURN();
        }
    } else {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    /* Little endian */
    (*val) = (vtss_ip_t)( (ip[0] << 24) + (ip[1] << 16) + (ip[2] << 8) + ip[3] );
    return ICLI_RC_OK;
}

/*
    check if IPv4 subnet
    ddd.ddd.ddd.ddd/ddd.ddd.ddd.ddd
    ddd.ddd.ddd.ddd/mask-bits

    zero subnet mask (/0 or /0.0.0.0) is not allowed with a non-zero IP.
    Only default route is allowed (0.0.0.0/0.0.0.0 or 0.0.0.0/0)

    INPUT
        word : user input word
    OUTPUT
        val     : ipv4 subnet address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv4_subnet_get(
    IN  char                *word,
    OUT icli_ipv4_subnet_t  *val,
    OUT u32                 *err_pos
)
{
    char        *c,
                *ip_str,
                *netmask_str;
    vtss_ip_t   ip, netmask;
    BOOL        b_delimiter,
                b_subnet;
    u32         vlsm;
    i32         rc;

    //find '/'
    for ( c = word; ICLI_NOT_(SUBNET_DELIMITER, *c) && ICLI_NOT_(EOS, *c); ++c ) {
        ;
    }

    if ( ICLI_IS_(SUBNET_DELIMITER, *c) ) {
        b_subnet = TRUE;
        //seperate ip and netmask
        (*c) = 0;
    } else {
        b_subnet = FALSE;
    }

    //check ip
    ip_str = word;
    rc = _ipv4_addr_get(ip_str, &ip, err_pos);

    //return '/' first
    if ( b_subnet ) {
        *(c) = ICLI_SUBNET_DELIMITER;
    }

    if ( rc != ICLI_RC_OK ) {
        if ( rc == ICLI_RC_ERR_INCOMPLETE && b_subnet ) {
            _ERR_MATCH_RETURN();
        }
        return rc;
    }

    if ( b_subnet == FALSE ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    //without netmask
    if ( ICLI_IS_(EOS, *(c + 1)) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    //get netmask string
    netmask_str = c + 1;

    //check netmask
    for ( c = netmask_str, b_delimiter = FALSE; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_(IPV4_DELIMITER, *c) ) {
            b_delimiter = TRUE;
            break;
        }
    }

    if ( b_delimiter ) {
        rc = _ipv4_netmask_get(netmask_str, &netmask, err_pos);
        if ( rc != ICLI_RC_OK ) {
            if ( rc == ICLI_RC_ERR_MATCH && err_pos ) {
                (*err_pos) += (netmask_str - word);
            }
            return rc;
        }
    } else {
        rc = _uint_get_in_range(netmask_str, 0, 32, &vlsm, err_pos);
        if ( rc != ICLI_RC_OK ) {
            if ( rc == ICLI_RC_ERR_MATCH && err_pos ) {
                (*err_pos) += (netmask_str - word);
            }
            return rc;
        } else {
            (void)_ipv4_vlsm_to_netmask(vlsm, &netmask);
        }
    }

    //check netmask == 0
    if ( netmask == 0 && ip != 0 ) {
        return _ipv4_addr_get_in_range( word, 0, &ip, err_pos );
    }

    //update
    val->ip = ip;
    val->netmask = netmask;
    return ICLI_RC_OK;
}

/*
    check if IPv4 prefix length, /0-32

    INPUT
        word : user input word
    OUTPUT
        val     : prefix length
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv4_prefix_get(
    IN  char    *word,
    OUT u32     *val,
    OUT u32     *err_pos
)
{
    char        *c = word;
    i32         rc;

    // check if '/'
    if ( ICLI_NOT_(SUBNET_DELIMITER, *c) ) {
        _ERR_MATCH_RETURN();
    }

    rc = _uint_get_in_range(c + 1, 0, 32, val, err_pos);
    if ( rc == ICLI_RC_ERR_MATCH && err_pos ) {
        (*err_pos) += 1;
    }
    return rc;
}

#define _IPv4_CHECK() \
            for ( p = c; _hex_get_c(*p) != -1; ++p ); \
            if ( ICLI_IS_(IPV4_DELIMITER,*p) ) { \
                if ( cnt_index == 0 && v6_cnt != 6 ) { \
                    c = p; \
                    _ERR_MATCH_RETURN(); \
                } \
                b_ipv4 = TRUE; \
                \
                rc = _ipv4_addr_get(c, &ipv4, &n); \
                switch ( rc ) { \
                case ICLI_RC_ERR_MATCH: \
                    c += n; \
                    _ERR_MATCH_RETURN(); \
                \
                case ICLI_RC_ERR_INCOMPLETE: \
                    return rc; \
                } \
                /* ICLI_RC_OK */ \
                break; \
            }

/*
    check if any IPv6 address < max
    hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
    hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:ddd.ddd.ddd.ddd
    "::" to skip some values, but can happen once only.
    ex, 1234::5678:90ab
        1234::10.9.52.134

    INPUT
        word : user input word
        max : max value for each hhhh
    OUTPUT
        val     : any ipv6 address < max
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv6_addr_get_in_range(
    IN  char            *word,
    IN  i32             max,
    OUT vtss_ipv6_t     *val,
    OUT u32             *err_pos
)
{
    //common
    char        *c, *p;
    //by type
    i32         state,
                /* return of _hex_get_c() */
                h,
                /* number of hex blocks before[0] and after[1] "::" */
                cnt[2] = { 0, 0 },
                         cnt_index = 0,
                         /* ipv6 address value */
                         v6[8] = {0, 0, 0, 0, 0, 0, 0, 0},
                                 v6_cnt = 0,
                                 rc;
    u32         n;
    vtss_ip_t   ipv4 = 0;
    BOOL        b_ipv4;

    /* init state */
    state       = 0;
    cnt_index   = 0;
    v6_cnt      = 0;
    b_ipv4      = FALSE;

    for ( c = word; (! b_ipv4) && ICLI_NOT_(EOS, *c); ++c ) {
        h = _hex_get_c( *c );

        /* invalid char */
        if ( h == -1 && ICLI_NOT_(IPV6_DELIMITER, *c) ) {
            _ERR_MATCH_RETURN();
        }

        switch ( state ) {
        case 0: /* init state */
            if ( h != -1 ) {
                /* hex, goto state 2 */
                state = 2;
                //get hex
                v6[v6_cnt] = h;
            } else if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
                /* colon goto state 1 */
                state = 1;
            }
            break;

        case 1: /* colon */
            if ( h != -1 ) {
                /* hex, error */
                _ERR_MATCH_RETURN();
            } else if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
                /* continuous colons, goto state 4 */
                state = 4;

                /* increase cnt_index */
                cnt_index = 1;
            }
            break;

        case 2: /* hex */
            if ( h != -1 ) {
                /* hex, goto state 2 */
                v6[v6_cnt] = (v6[v6_cnt] << 4) + h;
                if ( v6[v6_cnt] > max ) {
                    _ERR_MATCH_RETURN();
                }
            } else if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
                /* colon goto state 3 */
                state = 3;
                /* update index */
                ++( cnt[cnt_index] );
                ++v6_cnt;
                if ( v6_cnt > 7 ) {
                    _ERR_MATCH_RETURN();
                }
            }
            break;

        case 3: /* colon */
            /* check if follow by IPv4 */
            _IPv4_CHECK();

            /* IPv6 */
            if ( h != -1 ) {
                /* hex, goto state 2 */
                state = 2;
                //get hex
                v6[v6_cnt] = h;
            } else if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
                /* continuous colons, goto state 4 */
                state = 4;

                /* increase cnt_index */
                cnt_index = 1;
            }
            break;

        case 4: /* colon */
            /* check if follow by IPv4 */
            _IPv4_CHECK();

            /* IPv6 */
            if ( h != -1 ) {
                /* hex, goto state 5 */
                state = 5;
                //get hex
                v6[v6_cnt] = h;
            } else if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
                /* colon, error */
                _ERR_MATCH_RETURN();
            }
            break;

        case 5: /* hex */
            if ( h != -1 ) {
                /* hex, goto state 5 */
                v6[v6_cnt] = (v6[v6_cnt] << 4) + h;
                if ( v6[v6_cnt] > max ) {
                    _ERR_MATCH_RETURN();
                }
            } else if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
                /* colon goto state 6 */
                state = 6;
                /* update index */
                ++( cnt[cnt_index] );
                ++v6_cnt;
                if ( v6_cnt > 7 ) {
                    _ERR_MATCH_RETURN();
                }
            }
            break;

        case 6: /* colon */
            /* check if follow by IPv4 */
            _IPv4_CHECK();

            /* IPv6 */
            if ( h != -1 ) {
                /* hex, goto state 5 */
                state = 5;
                //get hex
                v6[v6_cnt] = h;
            } else if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
                /* colon, error */
                _ERR_MATCH_RETURN();
            }
            break;
        }
    }


    /* incomplete */
    if ( cnt_index == 0 ) {
        if ( b_ipv4 ) {
            if ( v6_cnt != 6 ) {
                _ERR_MATCH_RETURN();
            }
        } else if ( v6_cnt != 7 ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    /* check tail */
    if ( ! b_ipv4 ) {
        if ( ICLI_IS_(IPV6_DELIMITER, *(c - 1)) ) {
            if ( ICLI_NOT_(IPV6_DELIMITER, *(c - 2)) ) {
                --c;
                _ERR_MATCH_RETURN();
            }
        } else {
            //last one is hex, then check the hex counts
            ++( cnt[cnt_index] );
        }
    }

    /* get val */
    memset( val, 0, sizeof(vtss_ipv6_t) );
    for ( h = 0, n = 0, v6_cnt = 0; h < cnt[0]; ++h, ++v6_cnt ) {
        val->addr[n++] = (u8)( (v6[v6_cnt] & 0xff00) >> 8 );
        val->addr[n++] = (u8)( (v6[v6_cnt] & 0x00ff) );
    }

    /* second block start index */
    n += ((8 - cnt[0] - cnt[1]) * 2) - (b_ipv4 ? 4 : 0);

    for ( h = 0; h < cnt[1]; ++h, ++v6_cnt ) {
        val->addr[n++] = (u8)( (v6[v6_cnt] & 0xff00) >> 8 );
        val->addr[n++] = (u8)( (v6[v6_cnt] & 0x00ff) );
    }

    /* get IPv4 address */
    if ( b_ipv4 ) {
        val->addr[12] = (u8)(( ipv4 & 0xFF000000 ) >> 24);
        val->addr[13] = (u8)(( ipv4 & 0x00FF0000 ) >> 16);
        val->addr[14] = (u8)(( ipv4 & 0x0000FF00 ) >>  8);
        val->addr[15] = (u8)(( ipv4 & 0x000000FF ) >>  0);
    }

    return ICLI_RC_OK;
}

/*
    check if any IPv6 address
    hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
    "::" to skip some values, but can happen once only.
    ex, 1234::5678:90ab

    INPUT
        word : user input word
    OUTPUT
        val     : any ipv6 address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv6_addr_get(
    IN  char            *word,
    OUT vtss_ipv6_t     *val,
    OUT u32             *err_pos
)
{
    return _ipv6_addr_get_in_range( word, 0xFFFF, val, err_pos );
}

/*
    check if IPv6 unicast address
    uuhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
    uu must NOT be "FF".

    INPUT
        word : user input word
    OUTPUT
        val     : unicast ipv6 address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv6_ucast_get(
    IN  char            *word,
    OUT vtss_ipv6_t     *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    //by type
    vtss_ipv6_t     ipv6;
    i32             rc;

    rc = _ipv6_addr_get(word, &ipv6, err_pos);
    if ( rc != ICLI_RC_OK ) {
        return rc;
    }

    if ( ipv6.addr[0] == 0xFF ) {
        //find first f or F
        for ( c = word; *c != 'f' && *c != 'F'; ++c ) {
            ;
        }
        _ERR_MATCH_RETURN();
    }

    memcpy( val, &ipv6, sizeof(vtss_ipv6_t) );
    return ICLI_RC_OK;
}

/*
    check if IPv6 multicast address
    hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
    mmhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
    mm must be "FF".

    INPUT
        word : user input word
    OUTPUT
        val     : multicast ipv6 address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv6_mcast_get(
    IN  char            *word,
    OUT vtss_ipv6_t     *val,
    OUT u32             *err_pos
)
{
    vtss_ipv6_t     ipv6;
    i32             rc;

    rc = _ipv6_addr_get(word, &ipv6, err_pos);
    if ( rc != ICLI_RC_OK ) {
        return rc;
    }
    if ( ipv6.addr[0] != 0xFF ) {
        *err_pos = 0;
        return ICLI_RC_ERROR;
    }

    memcpy( val, &ipv6, sizeof(vtss_ipv6_t) );
    return ICLI_RC_OK;
}

/*
    check if IPv6 link local address

    INPUT
        word : user input word

    OUTPUT
        val     : ipv6 link local address
        err_pos : position where error happens

    RETURN
        icli_rc_t
*/
static i32 _ipv6_linklocal_get(
    IN  char            *word,
    OUT vtss_ipv6_t     *val,
    OUT u32             *err_pos
)
{
    vtss_ipv6_t     ipv6;
    i32             rc;

    // IPv6 address
    rc = _ipv6_addr_get(word, &ipv6, err_pos);
    if ( rc != ICLI_RC_OK ) {
        return rc;
    }

    // link local address
    if ( ipv6.addr[0] != 0xfe || (ipv6.addr[1] & 0xc0) != 0x80 ) {
        *err_pos = 0;
        return ICLI_RC_ERROR;
    }

    memcpy( val, &ipv6, sizeof(vtss_ipv6_t) );
    return ICLI_RC_OK;
}

/*
    check if any IPv6 netmask
    hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
    "::" to skip some values, but can happen once only.
    ex, ffff:ffff:ff00::

    INPUT
        word : user input word
    OUTPUT
        val     : ipv6 netmask
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv6_netmask_get(
    IN  char            *word,
    OUT vtss_ipv6_t     *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    //by type
    i32     h_cnt = 0,
            h,
            //number of hex blocks before(0) and after(1) "::"
            cnt[2] = { 0, 0 },
                     cnt_index = 0,
                     //ipv6 address
                     v6[8] = {0, 0, 0, 0, 0, 0, 0, 0},
                             v6_cnt = 0,
                             n,
                             bit,
                             b_one = 1;

    //preprocess ::
    c = word;
    if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
        ++c;
        if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
            ++c;
            ++cnt_index;
            b_one = 0;
        } else {
            _ERR_MATCH_RETURN();
        }
    }

    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        h = _hex_get_c( *c );
        if ( h != -1 ) {
            //valid hex
            ++h_cnt;
            //get hex
            v6[v6_cnt] = (v6[v6_cnt] << 4) + h;
            //overflow
            if ( v6[v6_cnt] > 0xffFF ) {
                _ERR_MATCH_RETURN();
            }
            //check netmask 1
            if ( b_one == 0 && v6[v6_cnt] ) {
                _ERR_MATCH_RETURN();
            }
        } else if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
            //too long
            if ( cnt_index == 1 && cnt[1] == 6 ) {
                _ERR_MATCH_RETURN();
            }
            //continous ::
            if ( h_cnt == 0 ) {
                if ( cnt_index == 0 ) {
                    ++cnt_index;
                    b_one = 0;
                } else {
                    _ERR_MATCH_RETURN();
                }
            } else {
                //check netmask 2
                if ( v6[v6_cnt] ) {
                    //for b_one == 1 only, because b_one == 0 at check netmask 1
                    if ( v6[v6_cnt] != 0xFFff ) {
                        for ( n = 15; n >= 0; --n ) {
                            bit = (v6[v6_cnt]) & ( 0x1L << n );
                            if ( bit ) {
                                if ( b_one == 0 ) {
                                    c -= ((n / 4) + 1);
                                    _ERR_MATCH_RETURN();
                                }
                            } else if ( b_one ) {
                                b_one = 0;
                            }
                        }
                    }
                } else {
                    if ( b_one ) {
                        b_one = 0;
                    }
                }
                //update
                ++( cnt[cnt_index] );
                h_cnt = 0;
                ++v6_cnt;
            }
            //too long
            if ( v6_cnt > 7 ) {
                _ERR_MATCH_RETURN();
            }

        } else {
            _ERR_MATCH_RETURN();
        }
    }

    //incomplete
    if ( cnt_index == 0 ) {
        //check netmask 3
        if ( ICLI_NOT_(IPV6_DELIMITER, *(c - 1)) ) {
            if ( v6[v6_cnt] ) {
                //for b_one == 1 only, because b_one == 0 at check netmask 1
                if ( v6[v6_cnt] != 0xFFff ) {
                    for ( n = 15; n >= 0; --n ) {
                        bit = (v6[v6_cnt]) & ( 0x1L << n );
                        if ( bit ) {
                            if ( b_one == 0 ) {
                                c -= ((n / 4) + 1);
                                _ERR_MATCH_RETURN();
                            }
                        } else if ( b_one ) {
                            b_one = 0;
                        }
                    }
                }
            }
        }

        if ( v6_cnt < 7 ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    if ( ICLI_IS_(IPV6_DELIMITER, *(c - 1)) ) {
        if ( ICLI_NOT_(IPV6_DELIMITER, *(c - 2)) ) {
            --c;
            _ERR_MATCH_RETURN();
        }
    } else {
        //last one is hex, then check the hex counts
        ++( cnt[cnt_index] );
    }

    memset( val, 0, sizeof(vtss_ipv6_t) );
    for ( h = 0, n = 0, v6_cnt = 0; h < cnt[0]; ++h, ++v6_cnt ) {
        val->addr[n++] = (u8)( (v6[v6_cnt] & 0xff00) >> 8 );
        val->addr[n++] = (u8)( (v6[v6_cnt] & 0x00ff) );
    }

    n += ((8 - cnt[0] - cnt[1]) * 2);

    for ( h = 0; h < cnt[1]; ++h, ++v6_cnt ) {
        val->addr[n++] = (u8)( (v6[v6_cnt] & 0xff00) >> 8 );
        val->addr[n++] = (u8)( (v6[v6_cnt] & 0x00ff) );
    }
    return ICLI_RC_OK;
}

/*
    check if IPv6 subnet address
    hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh/hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
    hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh/mask-bits

    zero subnet mask (/0 or /0.0.0.0) is not allowed with a non-zero IP.
    Only default route is allowed (0.0.0.0/0.0.0.0 or 0.0.0.0/0)

    INPUT
        word : user input word
    OUTPUT
        val     : ipv6 subnet address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv6_subnet_get(
    IN  char                *word,
    OUT icli_ipv6_subnet_t  *val,
    OUT u32                 *err_pos
)
{
    char            *c,
                    *ip_str,
                    *netmask_str;
    vtss_ipv6_t     ip,
                    netmask;
    BOOL            b_delimiter,
                    b_subnet;
    u32             vlsm;
    i32             rc;

    //find subnet delimiter
    for ( c = word; ICLI_NOT_(SUBNET_DELIMITER, *c) && ICLI_NOT_(EOS, *c); ++c ) {
        ;
    }

    if ( ICLI_IS_(SUBNET_DELIMITER, *c) ) {
        b_subnet = TRUE;
        //seperate ip and netmask
        (*c) = 0;
    } else {
        b_subnet = FALSE;
    }

    //check ip
    ip_str = word;
    rc = _ipv6_addr_get(ip_str, &ip, err_pos);

    //return '/' first
    if ( b_subnet ) {
        *(c) = ICLI_SUBNET_DELIMITER;
    }

    if ( rc != ICLI_RC_OK ) {
        if ( rc == ICLI_RC_ERR_INCOMPLETE && b_subnet ) {
            _ERR_MATCH_RETURN();
        }
        return rc;
    }

    if ( ! b_subnet ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    //without netmask
    if ( ICLI_IS_(EOS, *(c + 1)) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    //get netmask string
    netmask_str = c + 1;

    //check netmask
    for ( c = netmask_str, b_delimiter = FALSE; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_(IPV6_DELIMITER, *c) ) {
            b_delimiter = TRUE;
            break;
        }
    }

    if ( b_delimiter ) {
        rc = _ipv6_netmask_get(netmask_str, &netmask, err_pos);
        if ( rc != ICLI_RC_OK ) {
            if ( rc == ICLI_RC_ERR_MATCH && err_pos ) {
                (*err_pos) += (netmask_str - word);
            }
            return rc;
        }
    } else {
        rc = _uint_get_in_range(netmask_str, 0, 128, &vlsm, err_pos);
        if ( rc != ICLI_RC_OK ) {
            if ( rc == ICLI_RC_ERR_MATCH && err_pos ) {
                (*err_pos) += (netmask_str - word);
            }
            return rc;
        } else {
            (void)_ipv6_vlsm_to_netmask(vlsm, &netmask);
        }
    }

    //check netmask == 0
    if ( _ipv6_is_0(&netmask) == TRUE && _ipv6_is_0(&ip) == FALSE ) {
        return _ipv6_addr_get_in_range( word, 0, &ip, err_pos );
    }

    //update
    memcpy( &(val->ip), &ip, sizeof(vtss_ipv6_t) );
    memcpy( &(val->netmask), &netmask, sizeof(vtss_ipv6_t) );
    return ICLI_RC_OK;
}

/*
    check if IPv6 prefix length, /0-128

    INPUT
        word : user input word
    OUTPUT
        val     : prefix length
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _ipv6_prefix_get(
    IN  char    *word,
    OUT u32     *val,
    OUT u32     *err_pos
)
{
    char        *c = word;
    i32         rc;

    // check if '/'
    if ( ICLI_NOT_(SUBNET_DELIMITER, *c) ) {
        _ERR_MATCH_RETURN();
    }

    rc = _uint_get_in_range(c + 1, 0, 128, val, err_pos);
    if ( rc == ICLI_RC_ERR_MATCH && err_pos ) {
        (*err_pos) += 1;
    }
    return rc;
}

static i32  g_month_day[ICLI_YEAR_MAX_MONTH + 1] =
{ 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*
    leap month, 29 days
    1. mod by 4, but not mod by 100
    2. mod by 400
*/
#define _LEAP_YEAR ( (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0) )

static BOOL _month_day_max_check(
    IN i32  year,
    IN i32  month,
    IN i32  day
)
{
    if ( year < 1 ) {
        T_E("invalid year = %d\n", year);
        return FALSE;
    }

    if ( month < ICLI_YEAR_MIN_MONTH || month > ICLI_YEAR_MAX_MONTH ) {
        T_E("invalid month = %d\n", month);
        return FALSE;
    }

    if ( month == 2 ) {
        /* check leap month */
        if ( day > (g_month_day[month] + (_LEAP_YEAR ? 1 : 0) ) ) {
            return FALSE;
        }
    } else {
        if ( day > g_month_day[month] ) {
            return FALSE;
        }
    }
    return TRUE;
}

/*
    check if date
    yyyy/mm/dd, yyyy=1970-2037, mm=1-12, dd=1-31

    INPUT
        word : user input word
    OUTPUT
        val     : date struct
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _date_get(
    IN  char            *word,
    OUT icli_date_t     *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    //by type
    i32     d,
            year  = 0,
            month = 0,
            day   = 0,
            state;

    state = 0;
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        d = _digit_get_c(*c);

        /* invalid char */
        if ( d == -1 && ICLI_NOT_(DATE_DELIMITER, *c) ) {
            _ERR_MATCH_RETURN();
        }

        switch ( state ) {
        case 0: /* year */
            if ( d != -1 ) {
                /* digit, get year */
                year = year * 10 + d;
                /* check if too large */
                if ( year > ICLI_MAX_YEAR ) {
                    _ERR_MATCH_RETURN();
                }
            } else if ( ICLI_IS_(DATE_DELIMITER, *c) ) {
                /* check if year is too less */
                if ( year < ICLI_MIN_YEAR ) {
                    --c;
                    _ERR_MATCH_RETURN();
                }
                /* goto month */
                state = 1;
            }
            break;

        case 1: /* month */
            if ( d != -1 ) {
                /* digit, get month */
                month = month * 10 + d;
                /* check if too large */
                if ( month > ICLI_YEAR_MAX_MONTH ) {
                    _ERR_MATCH_RETURN();
                }
            } else if ( ICLI_IS_(DATE_DELIMITER, *c) ) {
                /* check if month is too less */
                if ( month < ICLI_YEAR_MIN_MONTH ) {
                    --c;
                    _ERR_MATCH_RETURN();
                }
                /* goto day */
                state = 2;
            }
            break;

        case 2: /* day */
            if ( d != -1 ) {
                /* digit, get day */
                day = day * 10 + d;
                /* check if too large */
                if ( _month_day_max_check(year, month, day) == FALSE ) {
                    _ERR_MATCH_RETURN();
                }
            } else if ( ICLI_IS_(DATE_DELIMITER, *c) ) {
                _ERR_MATCH_RETURN();
            }
            break;
        }
    }

    if ( state == 2 ) {
        /* check if day is too less */
        if ( day < 1 ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    } else {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    val->year  = (u16)(year);
    val->month = (u8)(month);
    val->day   = (u8)(day);
    return ICLI_RC_OK;
}

/*
    check if time
    HH:mm:ss, HH=0-23, mm=0-59, ss=0-59

    INPUT
        word : user input word
    OUTPUT
        val     : time struct
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _time_get(
    IN  char            *word,
    OUT icli_time_t     *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    //by type
    i32     d,
            d_cnt = 0,
            hour = 0,
            min  = 0,
            sec  = 0,
            state;

    state = 0;
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        d = _digit_get_c(*c);

        /* invalid char */
        if ( d == -1 && ICLI_NOT_(TIME_DELIMITER, *c) ) {
            _ERR_MATCH_RETURN();
        }

        switch ( state ) {
        case 0: /* hour */
            if ( d != -1 ) {
                /* digit, get hour */
                hour = hour * 10 + d;
                /* check if too large */
                if ( hour > 23 ) {
                    _ERR_MATCH_RETURN();
                }
            } else if ( ICLI_IS_(TIME_DELIMITER, *c) ) {
                /* goto min */
                state = 1;
            }
            break;

        case 1: /* min */
            if ( d != -1 ) {
                /* digit, get min */
                min = min * 10 + d;
                /* check if too large */
                if ( min > 59 ) {
                    _ERR_MATCH_RETURN();
                }
            } else if ( ICLI_IS_(TIME_DELIMITER, *c) ) {
                /* goto sec */
                state = 2;
                d_cnt = 0;
            }
            break;

        case 2: /* sec */
            if ( d != -1 ) {
                /* digit, get sec */
                sec = sec * 10 + d;
                ++d_cnt;
                /* check if too large */
                if ( sec > 59 ) {
                    _ERR_MATCH_RETURN();
                }
            } else if ( ICLI_IS_(TIME_DELIMITER, *c) ) {
                _ERR_MATCH_RETURN();
            }
            break;
        }
    }

    if ( state == 2 ) {
        /* check if no input */
        if ( d_cnt == 0 ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    } else {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    val->hour  = (u8)(hour);
    val->min   = (u8)(min);
    val->sec   = (u8)(sec);
    return ICLI_RC_OK;
}

/*
    check if time
    HH:mm, HH=0-23, mm=0-59

    INPUT
        word : user input word
    OUTPUT
        val     : time struct
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _hhmm_get(
    IN  char            *word,
    OUT icli_time_t     *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    //by type
    i32     d,
            d_cnt = 0,
            hour = 0,
            min  = 0,
            state;

    state = 0;
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        d = _digit_get_c(*c);

        /* invalid char */
        if ( d == -1 && ICLI_NOT_(TIME_DELIMITER, *c) ) {
            _ERR_MATCH_RETURN();
        }

        switch ( state ) {
        case 0: /* hour */
            if ( d != -1 ) {
                /* digit, get hour */
                hour = hour * 10 + d;
                /* check if too large */
                if ( hour > 23 ) {
                    _ERR_MATCH_RETURN();
                }
            } else if ( ICLI_IS_(TIME_DELIMITER, *c) ) {
                /* goto min */
                state = 1;
            }
            break;

        case 1: /* min */
            if ( d != -1 ) {
                /* digit, get min */
                min = min * 10 + d;
                /* increase count */
                ++d_cnt;
                /* check if too large */
                if ( min > 59 ) {
                    _ERR_MATCH_RETURN();
                }
            } else if ( ICLI_IS_(TIME_DELIMITER, *c) ) {
                _ERR_MATCH_RETURN();
            }
            break;
        }
    }

    if ( state == 1 ) {
        /* check if no input */
        if ( d_cnt == 0 ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    } else {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    val->hour  = (u8)(hour);
    val->min   = (u8)(min);
    val->sec   = 0;
    return ICLI_RC_OK;
}

/*
    check if single word without space

    INPUT
        word : user input word
    OUTPUT
        val     : a genral word
        err_pos : position where error happens
    RETURN
        icli_rc_t
    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _word_get(
    IN  char            *word,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;

    // no space inside
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_(SPACE, *c) ) {
            _ERR_MATCH_RETURN();
        }
    }

    if ( vtss_icli_str_len(word) > ICLI_VALUE_STR_MAX_LEN ) {
        *err_pos = ICLI_VALUE_STR_MAX_LEN;
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

/*
    check if single word without space and begin with alphabet

    INPUT
        word : user input word
    OUTPUT
        val     : a keyword word
        err_pos : position where error happens
    RETURN
        icli_rc_t
    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _kword_get(
    IN  char            *word,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c = word;

    //first one should be alphabet
    if ( ! ICLI_IS_ALPHABET(*c) ) {
        _ERR_MATCH_RETURN();
    }

    //can not contain SPACE
    for ( ++c; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_(SPACE, *c) ) {
            _ERR_MATCH_RETURN();
        }
    }

    if ( vtss_icli_str_len(word) > ICLI_VALUE_STR_MAX_LEN ) {
        *err_pos = ICLI_VALUE_STR_MAX_LEN;
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

/*
    check if single word without space

    INPUT
        word : user input word
    OUTPUT
        val     : a genral word
        err_pos : position where error happens
    RETURN
        icli_rc_t
    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _cword_get(
    IN  char            *word,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;

#if 1 /* CP, 07/11/2013 10:27, Bugzilla#12206 - <cword> */
    if ( val ) {}
#endif

    // no space inside
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_(SPACE, *c) ) {
            _ERR_MATCH_RETURN();
        }
    }

    if ( vtss_icli_str_len(word) > ICLI_VALUE_STR_MAX_LEN ) {
        *err_pos = ICLI_VALUE_STR_MAX_LEN;
        return ICLI_RC_ERR_MATCH;
    }

#if 0 /* CP, 07/11/2013 10:27, Bugzilla#12206 - <cword> */
    /* the copy is done in _node_match(), vtss_icli_exec.c */
    (void)vtss_icli_str_cpy(val, word);
#endif
    return ICLI_RC_OK;
}

/*
    check if single word in 0-9

    INPUT
        word : user input word
    OUTPUT
        val     : a digital word
        err_pos : position where error happens
    RETURN
        icli_rc_t
    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _dword_get(
    IN  char            *word,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;

    // in 0-9 and can not contain SPACE
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ! ICLI_IS_DIGIT(*c) ) {
            _ERR_MATCH_RETURN();
        }
    }

    if ( vtss_icli_str_len(word) > ICLI_VALUE_STR_MAX_LEN ) {
        *err_pos = ICLI_VALUE_STR_MAX_LEN;
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

/*
    check if single word in 0-9 and at most one dot

    INPUT
        word : user input word
    OUTPUT
        val     : a floating word
        err_pos : position where error happens
    RETURN
        icli_rc_t
    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _fword_get(
    IN  char            *word,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    u32     dot_n = 0;

    // in 0-9 and at most one dot
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_(DOT, *c) ) {
            if ( (++dot_n) > 1 ) {
                _ERR_MATCH_RETURN();
            }
        } else if ( ! ICLI_IS_DIGIT(*c) ) {
            _ERR_MATCH_RETURN();
        }
    }

    // check length
    if ( vtss_icli_str_len(word) > ICLI_VALUE_STR_MAX_LEN ) {
        *err_pos = ICLI_VALUE_STR_MAX_LEN;
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

/*
    check if string enclosed in ""

    INPUT
        word : user input word
    OUTPUT
        val     : a string
        err_pos : position where error happens
    RETURN
        icli_rc_t
    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _string_get(
    IN  char            *word,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c = word;
    //by type
    i32     len = vtss_icli_str_len( word );

    //check " at the begin
    if ( ICLI_NOT_(STRING_BEGIN, *c) ) {
        _ERR_MATCH_RETURN();
    }

    //only single "
    if ( len == 1 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    //go to the end
    for ( ++c; ICLI_NOT_(EOS, *c); ++c ) {
        ;
    }

    //check " at the tail
    if ( ICLI_NOT_(STRING_END, *(c - 1)) ) {
        if ( len > (ICLI_VALUE_STR_MAX_LEN + 1) ) {
            *err_pos = ICLI_VALUE_STR_MAX_LEN;
            return ICLI_RC_ERR_MATCH;
        } else {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    //check max length, 2 is for " and "
    if ( len > (ICLI_VALUE_STR_MAX_LEN + 2) ) {
        *err_pos = ICLI_VALUE_STR_MAX_LEN;
        return ICLI_RC_ERR_MATCH;
    }

    //do not copy ""
    (void)vtss_icli_str_ncpy(val, word + 1, len - 2);
    val[len - 2] = 0;
    return ICLI_RC_OK;
}

/*
    string, including "" and all chars

    INPUT
        word : user input word
    OUTPUT
        val     : a string
        err_pos : position where error happens
    RETURN
        icli_rc_t
    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _line_get(
    IN  char            *word,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    u32     len = vtss_icli_str_len( word );

    // no string
    if ( len == 0 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    if ( len > ICLI_VALUE_STR_MAX_LEN ) {
        if ( err_pos ) {
            *err_pos = ICLI_VALUE_STR_MAX_LEN;
        }
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_ncpy(val, word, len);
    val[len] = 0;

    return ICLI_RC_OK;
}

/*
    check if signed range list in the range of min and max

    INPUT
        word : user input word
        min  : min signed integer
        max  : max signed integer

    OUTPUT
        val     : range list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _int_range_list_get_in_range(
    IN  char                    *word,
    IN  char                    delimiter,
    IN  i32                     min,
    IN  i32                     max,
    OUT icli_signed_range_t     *val,
    OUT u32                     *err_pos
)
{
    char                    *c;
    char                    *last_c;
    char                    number[ICLI_DIGIT_MAX_LEN + 1];
    u32                     pos;
    i32                     i;
    i32                     n;
    i32                     last_n;
    i32                     cnt;
    icli_signed_range_t     sr;
    BOOL                    b_begin;

    c = word;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    memset( &sr, 0, sizeof(sr) );

    cnt     = 0;
    b_begin = TRUE;
    last_n  = 0;
    for (;;) {
        // skip space
        for ( ; ICLI_IS_(SPACE, *c); ++c ) {
            ;
        }

        // get number
        if ( ICLI_IS_(MINUS, *c) || ICLI_IS_DIGIT(*c) ) {
            i = 0;
            number[i] = *c;
            last_c = c;
            for ( ++i, ++c; __IS_NUMBER(*c) && i < ICLI_DIGIT_MAX_LEN; ++i, ++c ) {
                number[i] = *c;
            }
            number[i] = ICLI_EOS;

            switch ( _int_get_in_range(number, min, max, &n, &pos) ) {
            case ICLI_RC_OK:
                break;

            case ICLI_RC_ERR_INCOMPLETE:
                return ICLI_RC_ERR_INCOMPLETE;

            case ICLI_RC_ERR_MATCH:
            default:
                c = last_c + pos;
                _ERR_MATCH_RETURN();
            }

            if ( cnt == 0 && b_begin ) {
                sr.range[cnt].min = n;
                last_n = n;
            } else {
                if ( b_begin ) {
                    sr.range[cnt].min = n;
                } else {
                    if ( n <= last_n ) {
                        if ( err_pos ) {
                            (*err_pos) = c - word - 1;
                        }
                        return ICLI_RC_ERR_RANGE;
                    }
                    sr.range[cnt].max = n;
                }
                last_n = n;
            }
        } else if ( ICLI_IS_(EOS, *c) ) {
            return ICLI_RC_ERR_INCOMPLETE;
        } else {
            _ERR_MATCH_RETURN();
        }

        // skip space
        for ( ; ICLI_IS_(SPACE, *c); ++c ) {
            ;
        }

        // next char
        if ( ICLI_IS_(EOS, *c) ) {
            if ( b_begin ) {
                sr.range[cnt].max = last_n;
            }
            sr.cnt = cnt + 1;
            memcpy( val, &sr, sizeof(icli_signed_range_t) );
            return ICLI_RC_OK;
        }

        if ( ICLI_IS_(COMMA, *c) ) {
            if ( b_begin ) {
                sr.range[cnt].max = last_n;
            }

            // check cnt
            if ( (++cnt) >= ICLI_RANGE_LIST_CNT ) {
                _ERR_MATCH_RETURN();
            }

            b_begin = TRUE;
            ++c;
        } else if ( *c == delimiter ) {
            b_begin = FALSE;
            ++c;
        } else {
            _ERR_MATCH_RETURN();
        }
    } // for (;;)
}

/*
    check if unsigned range list in the range of min and max

    INPUT
        word : user input word
        min  : min unsigned integer
        max  : max unsigned integer

    OUTPUT
        val     : range list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _uint_range_list_get_in_range(
    IN  char                    *word,
    IN  const char              delimiter,
    IN  u32                     min,
    IN  u32                     max,
    OUT icli_unsigned_range_t   *val,
    OUT u32                     *err_pos
)
{
    char                    *c;
    char                    *last_c;
    char                    number[ICLI_DIGIT_MAX_LEN + 1];
    u32                     m;
    u32                     last_m;
    u32                     pos;
    i32                     i;
    i32                     cnt;
    icli_unsigned_range_t   ur;
    BOOL                    b_begin;

    c = word;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    memset( &ur, 0, sizeof(ur) );

    cnt = 0;
    b_begin = TRUE;
    last_m = 0;

    for (;;) {
        // skip space
        for ( ; ICLI_IS_(SPACE, *c); ++c ) {
            ;
        }

        if ( ICLI_IS_DIGIT(*c) ) {
            /* get number */
            i = 0;
            number[i] = *c;
            last_c = c;
            for ( ++i, ++c; __IS_NUMBER(*c) && i < ICLI_DIGIT_MAX_LEN; ++i, ++c ) {
                number[i] = *c;
            }
            number[i] = ICLI_EOS;

            /* get m */
            switch ( _uint_get_in_range(number, min, max, &m, &pos) ) {
            case ICLI_RC_OK:
                break;

            case ICLI_RC_ERR_INCOMPLETE:
                return ICLI_RC_ERR_INCOMPLETE;

            case ICLI_RC_ERR_MATCH:
            default:
                c = last_c + pos;
                _ERR_MATCH_RETURN();
            }

            if ( cnt == 0 && b_begin ) {
                ur.range[cnt].min = m;
                last_m = m;
            } else {
                if ( b_begin ) {
                    ur.range[cnt].min = m;
                } else {
                    if ( m <= last_m ) {
                        if ( err_pos ) {
                            (*err_pos) = c - word - 1;
                        }
                        return ICLI_RC_ERR_RANGE;
                    }
                    ur.range[cnt].max = m;
                }
                last_m = m;
            }
        } else if ( ICLI_IS_(EOS, *c) ) {
            return ICLI_RC_ERR_INCOMPLETE;
        } else {
            _ERR_MATCH_RETURN();
        }

        // skip space
        for ( ; ICLI_IS_(SPACE, *c); ++c ) {
            ;
        }

        // next char
        if ( ICLI_IS_(EOS, *c) ) {
            if ( b_begin ) {
                ur.range[cnt].max = last_m;
            }
            ur.cnt = cnt + 1;
            *val = ur;
            return ICLI_RC_OK;
        }

        if ( ICLI_IS_(COMMA, *c) ) {
            if ( b_begin ) {
                ur.range[cnt].max = last_m;
            }

            // check cnt
            if ( (++cnt) >= ICLI_RANGE_LIST_CNT ) {
                _ERR_MATCH_RETURN();
            }

            b_begin = TRUE;
            ++c;
        } else if ( *c == delimiter ) {
            b_begin = FALSE;
            ++c;
        } else {
            _ERR_MATCH_RETURN();
        }
    }//while
}

/*
    port ID, switch-id/port-id
    example: 3/2

    INPUT
        word : user input word
    OUTPUT
        val     : port ID
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _port_id_get(
    IN  char                        *word,
    OUT icli_switch_port_range_t    *val,
    OUT u32                         *err_pos
)
{
    char    *c = word;
    char    *deli;
    u16     switch_id;
    u16     port_id;
    i32     r;

    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    /* separate switch ID and port ID */
    for ( ; ICLI_NOT_(PORT_DELIMITER, *c) && ICLI_NOT_(EOS, *c); ++c ) {
        ;
    }

    if ( ICLI_IS_(EOS, *c) ) {
        r = _uint16_get(word, &switch_id, err_pos);
        if ( r == ICLI_RC_OK ) {
            return ICLI_RC_ERR_INCOMPLETE;
        } else {
            return r;
        }
    }

    deli = c;
    ++c;
    *deli = ICLI_EOS;

    /* get switch ID */
    r = _uint16_get(word, &switch_id, err_pos);

    //revert separation
    *deli = ICLI_PORT_DELIMITER;

    if ( r != ICLI_RC_OK ) {
        return r;
    }

    /* get port ID */
    r = _uint16_get(c, &port_id, err_pos);
    if ( r != ICLI_RC_OK ) {
        (*err_pos) += (c - word);
        return r;
    }

    /* output */
    val->port_type   = ICLI_PORT_TYPE_NONE;
    val->switch_id   = switch_id;
    val->begin_port  = port_id;
    val->port_cnt    = 1;
    val->usid        = 0;
    val->begin_uport = 0;
    val->isid        = 0;
    val->begin_iport = 0;

    return ICLI_RC_OK;
}

/*
    port list, switch-id list/port-id list;...
    example: 1-3/2,5,6-9,12
             1/2,5,6-9,12;3/7,11

    INPUT
        word : user input word

    OUTPUT
        val     : port list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _port_list_get(
    IN  char                    *word,
    OUT icli_stack_port_range_t *val,
    OUT u32                     *err_pos
)
{
    char                        *c;
    char                        *switch_list;
    char                        *port_list;
    char                        *deli;
    icli_unsigned_range_t       switch_range;
    icli_unsigned_range_t       port_range;
    i32                         r;
    i32                         m;
    u32                         i;
    u32                         j;
    u32                         k;
    icli_stack_port_range_t     stack_range;
    u32                         ep;
    BOOL                        b_eos;

    /* get word */
    c = word;

    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    /* get memory */
    memset( &switch_range, 0, sizeof(switch_range) );
    memset( &port_range, 0, sizeof(port_range) );
    memset( &stack_range, 0, sizeof(stack_range) );

    r = ICLI_RC_OK;
    ep = 0;
    for (;;) {
        /* separate switch List and port List */
        switch_list = c;
        for ( ; ICLI_NOT_(PORT_DELIMITER, *c) && ICLI_NOT_(EOS, *c); ++c ) {
            ;
        }

        if ( ICLI_IS_(EOS, *c) ) {
#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
            if ( vtss_icli_str_cmp(switch_list, ICLI_INTERFACE_WILDCARD) == 0 ) {
                val->cnt = 1;
                val->switch_range[0].port_type   = ICLI_PORT_TYPE_NONE;
                val->switch_range[0].switch_id   = ICLI_SWITCH_PORT_ALL;
                val->switch_range[0].begin_port  = ICLI_SWITCH_PORT_ALL;
                val->switch_range[0].port_cnt    = 1;
                val->switch_range[0].usid        = 0;
                val->switch_range[0].begin_uport = 0;
                val->switch_range[0].isid        = 0;
                val->switch_range[0].begin_iport = 0;

                return ICLI_RC_OK;
            }
#endif

            /* get switch List */
            r = _uint_range_list_get_in_range( switch_list,
                                               ICLI_DASH,
                                               ICLI_MIN_UINT16,
                                               ICLI_MAX_UINT16,
                                               &switch_range,
                                               &ep );
            if ( r == ICLI_RC_OK ) {
                return ICLI_RC_ERR_INCOMPLETE;
            } else {
                if ( r == ICLI_RC_ERR_MATCH ) {
                    *err_pos = ep + (switch_list - word);
                }
                return r;
            }
        }

        deli = c;
        ++c;
        *deli = ICLI_EOS;

        /* get switch List */
#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
        if ( vtss_icli_str_cmp(switch_list, ICLI_INTERFACE_WILDCARD) ) {
            r = _uint_range_list_get_in_range( switch_list,
                                               ICLI_DASH,
                                               ICLI_MIN_UINT16,
                                               ICLI_MAX_UINT16,
                                               &switch_range,
                                               &ep );
        } else {
            switch_range.cnt = 1;
            switch_range.range[0].min = ICLI_SWITCH_PORT_ALL;
            switch_range.range[0].max = ICLI_SWITCH_PORT_ALL;
        }
#else
        r = _uint_range_list_get_in_range( switch_list,
                                           ICLI_DASH,
                                           ICLI_MIN_UINT16,
                                           ICLI_MAX_UINT16,
                                           &switch_range,
                                           &ep );
#endif

        // revert separation
        *deli = ICLI_PORT_DELIMITER;

        if ( r != ICLI_RC_OK ) {
            if ( r == ICLI_RC_ERR_MATCH ) {
                *err_pos = ep + (switch_list - word);
            }
            return r;
        }

        /* separate switch List and port List */
        port_list = c;
        for ( ; ICLI_NOT_(PLIST_DELIMITER, *c) && ICLI_NOT_(EOS, *c); ++c ) {
            ;
        }

        b_eos = TRUE;
        if ( ICLI_NOT_(EOS, *c) ) {
            b_eos = FALSE;
            deli = c;
            ++c;
            *deli = ICLI_EOS;
        }

        /* get port List */
#if 1 /* CP, 07/18/2013 15:48, Bugzilla#11844 - interface wildcard */
        if ( vtss_icli_str_cmp(port_list, ICLI_INTERFACE_WILDCARD) ) {
            r = _uint_range_list_get_in_range( port_list,
                                               ICLI_DASH,
                                               ICLI_MIN_UINT16,
                                               ICLI_MAX_UINT16,
                                               &port_range,
                                               &ep );
        } else {
            port_range.cnt = 1;
            port_range.range[0].min = ICLI_SWITCH_PORT_ALL;
            port_range.range[0].max = ICLI_SWITCH_PORT_ALL;
        }
#else
        r = _uint_range_list_get_in_range( port_list,
                                           ICLI_DASH,
                                           ICLI_MIN_UINT16,
                                           ICLI_MAX_UINT16,
                                           &port_range,
                                           &ep );
#endif

        // revert separation
        if ( b_eos == FALSE ) {
            *deli = ICLI_PLIST_DELIMITER;
        }

        if ( r != ICLI_RC_OK ) {
            if ( r == ICLI_RC_ERR_MATCH ) {
                *err_pos = ep + (port_list - word);
            }
            return r;
        }

        /* get switch List and port List */
        for ( i = 0; i < switch_range.cnt; ++i ) {
            for ( j = switch_range.range[i].min; j <= switch_range.range[i].max; ++j ) {
                for ( k = 0; k < port_range.cnt; ++k ) {
                    m = stack_range.cnt;
                    stack_range.switch_range[m].port_type   = ICLI_PORT_TYPE_NONE;
                    stack_range.switch_range[m].switch_id   = (u16)j;
                    stack_range.switch_range[m].begin_port  = (u16)(port_range.range[k].min);
                    stack_range.switch_range[m].port_cnt    = (u16)(port_range.range[k].max - port_range.range[k].min + 1);
                    stack_range.switch_range[m].usid        = 0;
                    stack_range.switch_range[m].begin_uport = 0;
                    stack_range.switch_range[m].isid        = 0;
                    stack_range.switch_range[m].begin_iport = 0;
                    ++( stack_range.cnt );
                }
            }
        }

        /* finish */
        if ( ICLI_IS_(EOS, *c) ) {
            *val = stack_range;
            return r;
        }
    }
}

/*
    check if vlan ID

    INPUT
        word : user input word
    OUTPUT
        val     : VLAN ID
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _vlan_id_get(
    IN  char    *word,
    OUT u32     *val,
    OUT u32     *err_pos
)
{
    return _uint_get_in_range( word,
                               ICLI_MIN_VLAN_ID,
                               ICLI_MAX_VLAN_ID,
                               val,
                               err_pos );
}

/*
    check if VLAN list

    INPUT
        word : user input word

    OUTPUT
        val     : VLAN list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _vlan_list_get(
    IN  char                    *word,
    OUT icli_unsigned_range_t   *val,
    OUT u32                     *err_pos
)
{
    return _uint_range_list_get_in_range( word,
                                          ICLI_DASH,
                                          ICLI_MIN_VLAN_ID,
                                          ICLI_MAX_VLAN_ID,
                                          val,
                                          err_pos);
}

/*
    check if range list

    INPUT
        word      : user input word
        delimiter : the delimiter for range
        range     : the valid range for the value

    OUTPUT
        val     : range list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _range_list_get_with_delimiter(
    IN  char            *word,
    IN  char            delimiter,
    IN  icli_range_t    *range,
    OUT icli_range_t    *val,
    OUT u32             *err_pos
)
{
    char                    *c;
    icli_signed_range_t     sr,
                            *psr;
    icli_unsigned_range_t   ur,
                            *pur;
    BOOL                    b_signed;
    i32                     r;

    sr.cnt = 0;
    ur.cnt = 0;

    //skip space
    for ( c = word; ICLI_IS_(SPACE, *c); ++c ) {
        ;
    }

    //get number
    if ( range ) {
        if ( range->type == ICLI_RANGE_TYPE_SIGNED ) {
            psr = &( range->u.sr );
            if ( psr->cnt != 1 ) {
                T_E("cnt = %u\n", psr->cnt);
                return ICLI_RC_ERROR;
            }

            b_signed = TRUE;
            r = _int_range_list_get_in_range( word,
                                              delimiter,
                                              psr->range[0].min,
                                              psr->range[0].max,
                                              &sr,
                                              err_pos );
        } else {
            pur = &( range->u.ur );
            if ( pur->cnt != 1 ) {
                T_E("cnt = %u\n", pur->cnt);
                return ICLI_RC_ERROR;
            }

            b_signed = FALSE;
            r = _uint_range_list_get_in_range( word,
                                               delimiter,
                                               pur->range[0].min,
                                               pur->range[0].max,
                                               &ur,
                                               err_pos );
        }
    } else {
        b_signed = FALSE;
        r = _uint_range_list_get_in_range( word,
                                           delimiter,
                                           ICLI_MIN_UINT,
                                           ICLI_MAX_UINT,
                                           &ur,
                                           err_pos );
        switch ( r ) {
        case ICLI_RC_OK:
            break;

        case ICLI_RC_ERR_RANGE:
            return r;

        default:
            b_signed = TRUE;
            r = _int_range_list_get_in_range( word,
                                              delimiter,
                                              ICLI_MIN_INT,
                                              ICLI_MAX_INT,
                                              &sr,
                                              err_pos );
            break;
        }
    }

    if ( r != ICLI_RC_OK ) {
        return r;
    }

    if ( b_signed ) {
        val->type = ICLI_RANGE_TYPE_SIGNED;
        val->u.sr = sr;
    } else {
        val->type = ICLI_RANGE_TYPE_UNSIGNED;
        val->u.ur = ur;
    }
    return ICLI_RC_OK;
}

/*
    check if range list

    INPUT
        word : user input word

    OUTPUT
        val     : range list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _range_list_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT icli_range_t    *val,
    OUT u32             *err_pos
)
{
    return _range_list_get_with_delimiter( word, ICLI_DASH, range, val, err_pos);
}

/*
    check if signed int range list

    INPUT
        word  : user input word
        range : range for the list

    OUTPUT
        val     : signed range list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _range_int_list_get(
    IN  char                    *word,
    IN  icli_range_t            *range,
    OUT icli_signed_range_t     *val,
    OUT u32                     *err_pos
)
{
    i32                     min,
                            max;
    icli_signed_range_t     *pr;

    if ( range ) {
        if ( range->type != ICLI_RANGE_TYPE_SIGNED ) {
            T_E("invalid range type %u\n", range->type);
            return ICLI_RC_ERROR;
        }

        pr = &( range->u.sr );
        if ( pr->cnt != 1 ) {
            T_E("cnt = %u\n", pr->cnt);
            return ICLI_RC_ERROR;
        }

        min = pr->range[0].min;
        max = pr->range[0].max;
    } else {
        min = ICLI_MIN_INT;
        max = ICLI_MAX_INT;
    }

    return _int_range_list_get_in_range( word,
                                         ICLI_DASH,
                                         min,
                                         max,
                                         val,
                                         err_pos );
}

/*
    check if unsigned int range list

    INPUT
        word  : user input word
        range : range for the list

    OUTPUT
        val     : unsigned range list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _range_uint_list_get(
    IN  char                    *word,
    IN  icli_range_t            *range,
    OUT icli_unsigned_range_t   *val,
    OUT u32                     *err_pos
)
{
    u32                     min,
                            max;
    icli_unsigned_range_t   *pr;

    if ( range ) {
        if ( range->type != ICLI_RANGE_TYPE_UNSIGNED ) {
            T_E("invalid range type %u\n", range->type);
            return ICLI_RC_ERROR;
        }

        pr = &( range->u.ur );
        if ( pr->cnt != 1 ) {
            T_E("cnt = %u\n", pr->cnt);
            return ICLI_RC_ERROR;
        }

        min = pr->range[0].min;
        max = pr->range[0].max;
    } else {
        min = ICLI_MIN_UINT;
        max = ICLI_MAX_UINT;
    }

    return _uint_range_list_get_in_range( word,
                                          ICLI_DASH,
                                          min,
                                          max,
                                          val,
                                          err_pos );
}

#define __PORT_TYPE_CHECK_AND_ASSIGN(t, x) \
    if ( vtss_icli_str_sub(word, g_port_type_data[t].name, FALSE, &(x)) >= 0 ) { \
        *val = t; \
        return ICLI_RC_OK; \
    }

/*
    check if port type

    INPUT
        word : user input word

    OUTPUT
        val     : port type
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _port_type_get(
    IN  char                *word,
    OUT icli_port_type_t    *val,
    OUT u32                 *err_pos
)
{
    u32                 x[ICLI_PORT_TYPE_MAX];
    u32                 max;
    icli_port_type_t    port_type;

    memset( x, 0, sizeof(x) );
    for ( port_type = 1; port_type < ICLI_PORT_TYPE_MAX; ++port_type ) {
        __PORT_TYPE_CHECK_AND_ASSIGN( port_type, x[port_type] );
    }

    max = 0;
    for ( port_type = 1; port_type < ICLI_PORT_TYPE_MAX; ++port_type ) {
        if ( x[port_type] > max ) {
            max = x[port_type];
        }
    }

    *err_pos = max;
    return ICLI_RC_ERR_MATCH;
}

/*
    port type ID, port-type switch-id/port-id
    example:
        fastethernet 1/1
        giga 3/2
        tengiga 1/5

    INPUT
        word : user input word
    OUTPUT
        val     : port ID
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _port_type_id_get(
    IN  char                        *word,
    OUT icli_switch_port_range_t    *val,
    OUT u32                         *err_pos
)
{
    return _port_id_get(word, val, err_pos);
}

/*
    port list, switch-id list/port-id list;...
    example: 1-3/2,5,6-9,12
             1/2,5,6-9,12;3/7,11

    INPUT
        word : user input word

    OUTPUT
        val     : port list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _port_type_list_get(
    IN  char                    *word,
    OUT icli_stack_port_range_t *val,
    OUT u32                     *err_pos
)
{
    return _port_list_get(word, val, err_pos);
}

/*
    check if OUI
    hm:hh:hh or hm-hh-hh, where (hm & 0x01) != 0x01

    INPUT
        word : user input word
    OUTPUT
        val     : unicast mac address
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _oui_get(
    IN  char        *word,
    OUT icli_oui_t  *val,
    OUT u32         *err_pos
)
{
    //common
    char        *c;
    //by type
    i32         mac[ICLI_OUI_SIZE] = { 0, 0, 0 },
                                     mac_index = 0,
                                     h,
                                     h_cnt = 0;

    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        h = _hex_get_c(*c);
        if ( h != -1 ) {
            //valid hex value
            ++h_cnt;
            //get mac
            mac[mac_index] = (mac[mac_index] << 4) + h;
            if ( mac[mac_index] > 0xFF ) {
                _ERR_MATCH_RETURN();
            }
        } else if ( ICLI_IS_(MAC_A_DELIMITER, *c) || ICLI_IS_(MAC_Z_DELIMITER, *c) ) {
            //continuous ::
            if ( h_cnt == 0 ) {
                _ERR_MATCH_RETURN();
            }
#if 0 /* Bugzilla#11465 - do not check multicast per Peter request */
            //unicast
            if ( mac_index == 0 ) {
                if ((mac[0] & 0x01) == 0x01) {
                    _ERR_MATCH_RETURN();
                }
            }
#endif
            //too long
            if ( (++mac_index) > (ICLI_OUI_SIZE - 1) ) {
                _ERR_MATCH_RETURN();
            }
            //update
            h_cnt = 0;
        } else {
            //invalid character
            _ERR_MATCH_RETURN();
        }
    }

    if ( mac_index < (ICLI_OUI_SIZE - 1) || h_cnt == 0 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    for ( h_cnt = 0; h_cnt < ICLI_OUI_SIZE; ++h_cnt ) {
        val->mac[h_cnt] = (u8)(mac[h_cnt]);
    }
    return ICLI_RC_OK;
}

/*
    check if PCP
    specific(0, 1, 2, 3, 4, 5, 6, 7) or range(0-1, 2-3, 4-5, 6-7, 0-3, 4-7).

    INPUT
        word : user input word

    OUTPUT
        val     : PCP
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _pcp_get(
    IN  char                    *word,
    OUT icli_unsigned_range_t   *val,
    OUT u32                     *err_pos
)
{
    i32     rc;

    rc = _uint_range_list_get_in_range( word,
                                        ICLI_DASH,
                                        0,
                                        7,
                                        val,
                                        err_pos );
    if ( rc != ICLI_RC_OK ) {
        return rc;
    }

    *err_pos = 0;
    if ( val->cnt != 1 ) {
        return ICLI_RC_ERR_MATCH;
    }

    switch ( val->range[0].min ) {
    case 0:
        if ( val->range[0].max != 0 &&
             val->range[0].max != 1 &&
             val->range[0].max != 3 &&
             val->range[0].max != 7  ) {
            return ICLI_RC_ERR_MATCH;
        }
        break;

    case 1:
        if ( val->range[0].max != 1 ) {
            return ICLI_RC_ERR_MATCH;
        }
        break;

    case 2:
        if ( val->range[0].max != 2 &&
             val->range[0].max != 3  ) {
            return ICLI_RC_ERR_MATCH;
        }
        break;

    case 3:
        if ( val->range[0].max != 3  ) {
            return ICLI_RC_ERR_MATCH;
        }
        break;

    case 4:
        if ( val->range[0].max != 4 &&
             val->range[0].max != 5 &&
             val->range[0].max != 7  ) {
            return ICLI_RC_ERR_MATCH;
        }
        break;

    case 5:
        if ( val->range[0].max != 5 ) {
            return ICLI_RC_ERR_MATCH;
        }
        break;

    case 6:
        if ( val->range[0].max != 6 &&
             val->range[0].max != 7  ) {
            return ICLI_RC_ERR_MATCH;
        }
        break;

    case 7:
        if ( val->range[0].max != 7 ) {
            return ICLI_RC_ERR_MATCH;
        }
        break;

    default:
        return ICLI_RC_ERR_MATCH;
    }
    return ICLI_RC_OK;
}

/*
    check if DPL
    specific(0, 1, 2, 3, 4, 5, 6, 7) or range(0-1, 2-3, 4-5, 6-7, 0-3, 4-7).

    INPUT
        word : user input word

    OUTPUT
        val     : PCP
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _dpl_get(
    IN  char    *word,
    OUT u8      *val,
    OUT u32     *err_pos
)
{
    u32     dpl;
    i32     rc;

#ifdef VTSS_ARCH_JAGUAR_1
    rc = _uint_get_in_range(word, 0, 3, &dpl, err_pos);
#else
    rc = _uint_get_in_range(word, 0, 1, &dpl, err_pos);
#endif

    if ( rc == ICLI_RC_OK ) {
        (*val) = (u8)dpl;
    }
    return rc;
}

/*
    check if grep '|'

    INPUT
        word : user input word

    OUTPUT
        val     : 1-yes, 0-no
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _grep_get(
    IN  char            *word,
    OUT u32             *val,
    OUT u32             *err_pos
)
{
    if ( vtss_icli_str_sub(word, ICLI_GREP_KEYWORD, 0, err_pos) == -1 ) {
        return ICLI_RC_ERR_MATCH;
    }

    *val = TRUE;
    return ICLI_RC_OK;
}

/*
    check if "begin" for grep

    INPUT
        word : user input word

    OUTPUT
        val     : 1-yes, 0-no
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _grep_begin_get(
    IN  char            *word,
    OUT u32             *val,
    OUT u32             *err_pos
)
{
    if ( vtss_icli_str_sub(word, ICLI_GREP_BEGIN_KEYWORD, 0, err_pos) == -1 ) {
        return ICLI_RC_ERR_MATCH;
    }

    *val = TRUE;
    return ICLI_RC_OK;
}

/*
    check if "include" for grep

    INPUT
        word : user input word

    OUTPUT
        val     : 1-yes, 0-no
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _grep_include_get(
    IN  char            *word,
    OUT u32             *val,
    OUT u32             *err_pos
)
{
    if ( vtss_icli_str_sub(word, ICLI_GREP_INCLUDE_KEYWORD, 0, err_pos) == -1 ) {
        return ICLI_RC_ERR_MATCH;
    }

    *val = TRUE;
    return ICLI_RC_OK;
}

/*
    check if "exclude" for grep

    INPUT
        word : user input word

    OUTPUT
        val     : 1-yes, 0-no
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _grep_exclude_get(
    IN  char            *word,
    OUT u32             *val,
    OUT u32             *err_pos
)
{
    if ( vtss_icli_str_sub(word, ICLI_GREP_EXCLUDE_KEYWORD, 0, err_pos) == -1 ) {
        return ICLI_RC_ERR_MATCH;
    }

    *val = TRUE;
    return ICLI_RC_OK;
}

/*
    word with length limitation

    INPUT
        word  : user input word
        range : length limitation

    OUTPUT
        val     : range list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _range_word_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    u32     len;

#if 1 /* CP, 2012/10/24 14:08, Bugzilla#10092 - runtime variable word length */
    if ( range->type != ICLI_RANGE_TYPE_UNSIGNED ) {
        T_E("range type must be ICLI_RANGE_TYPE_UNSIGNED, but not %d\n", range->type);
        return ICLI_RC_ERR_PARAMETER;
    }
#endif

    // no space inside
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_(SPACE, *c) ) {
            _ERR_MATCH_RETURN();
        }
    }

    // check string length
    len = vtss_icli_str_len( word );

    if ( len < range->u.ur.range[0].min ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    if ( len > range->u.ur.range[0].max ) {
        *err_pos = range->u.ur.range[0].max;
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

/*
    kword with length limitation

    INPUT
        word  : user input word
        range : length limitation

    OUTPUT
        val     : a keyword word
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _range_kword_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c = word;
    u32     len;

#if 1 /* CP, 2012/10/24 14:08, Bugzilla#10092 - runtime variable word length */
    if ( range->type != ICLI_RANGE_TYPE_UNSIGNED ) {
        T_E("range type must be ICLI_RANGE_TYPE_UNSIGNED, but not %d\n", range->type);
        return ICLI_RC_ERR_PARAMETER;
    }
#endif

    //first one should be alphabet
    if ( ! ICLI_IS_ALPHABET(*c) ) {
        _ERR_MATCH_RETURN();
    }

    //can not contain SPACE
    for ( ++c; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_(SPACE, *c) ) {
            _ERR_MATCH_RETURN();
        }
    }

    // check string length
    len = vtss_icli_str_len( word );

    if ( len < range->u.ur.range[0].min ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    if ( len > range->u.ur.range[0].max ) {
        *err_pos = range->u.ur.range[0].max;
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

/*
    dword with length limitation

    INPUT
        word  : user input word
        range : length limitation

    OUTPUT
        val     : a digital word
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _range_dword_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    u32     len;

#if 1 /* CP, 2012/10/24 14:08, Bugzilla#10092 - runtime variable word length */
    if ( range->type != ICLI_RANGE_TYPE_UNSIGNED ) {
        T_E("range type must be ICLI_RANGE_TYPE_UNSIGNED, but not %d\n", range->type);
        return ICLI_RC_ERR_PARAMETER;
    }
#endif

    // in 0-9 and can not contain SPACE
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ! ICLI_IS_DIGIT(*c) ) {
            _ERR_MATCH_RETURN();
        }
    }

    // check string length
    len = vtss_icli_str_len( word );

    if ( len < range->u.ur.range[0].min ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    if ( len > range->u.ur.range[0].max ) {
        *err_pos = range->u.ur.range[0].max;
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

/*
    fword with length limitation

    INPUT
        word  : user input word
        range : length limitation

    OUTPUT
        val     : a floating word
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _range_fword_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    u32     len = 0;
    u32     dot_n  = 0;
    char    *dot_c = NULL;

#if 1 /* CP, 2012/10/24 14:08, Bugzilla#10092 - runtime variable word length */
    if ( range->type != ICLI_RANGE_TYPE_UNSIGNED ) {
        T_E("range type must be ICLI_RANGE_TYPE_UNSIGNED, but not %d\n", range->type);
        return ICLI_RC_ERR_PARAMETER;
    }
#endif

    // in 0-9 and at most one dot
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_(DOT, *c) ) {
            if ( (++dot_n) > 1 ) {
                _ERR_MATCH_RETURN();
            }
            dot_c = c;
        } else if ( ! ICLI_IS_DIGIT(*c) ) {
            _ERR_MATCH_RETURN();
        }
    }

    // check length before the dot
    if ( dot_n ) {
        if ( dot_c ) { // for lint
            len = dot_c - word;
        }
    } else {
        len = vtss_icli_str_len( word );
    }

    if ( len < range->u.ur.range[0].min ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    if ( len > range->u.ur.range[0].max ) {
        *err_pos = range->u.ur.range[0].max;
        return ICLI_RC_ERR_MATCH;
    }

    // check length after the dot
    if ( dot_n ) {
        if ( dot_c ) { // for lint
            len = c - dot_c - 1;
        }
    } else {
        len = 0;
    }

    if ( range->u.ur.cnt > 1 ) {
        if ( len < range->u.ur.range[1].min ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }

        if ( len > range->u.ur.range[1].max ) {
            *err_pos = range->u.ur.range[1].max;
            return ICLI_RC_ERR_MATCH;
        }
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

/*
    string with length limitation

    INPUT
        word  : user input word
        range : length limitation

    OUTPUT
        val     : a string
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _range_string_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c = word;
    //by type
    u32     len = vtss_icli_str_len( word );

#if 1 /* CP, 2012/10/24 14:08, Bugzilla#10092 - runtime variable word length */
    if ( range->type != ICLI_RANGE_TYPE_UNSIGNED ) {
        T_E("range type must be ICLI_RANGE_TYPE_UNSIGNED, but not %d\n", range->type);
        return ICLI_RC_ERR_PARAMETER;
    }
#endif

    //check " at the begin
    if ( ICLI_NOT_(STRING_BEGIN, *c) ) {
        _ERR_MATCH_RETURN();
    }

    //only single "
    if ( len == 1 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    //go to the end
    for ( ++c; ICLI_NOT_(EOS, *c); ++c ) {
        ;
    }

    //check " at the tail
    if ( ICLI_NOT_(STRING_END, *(c - 1)) ) {
        if ( (len - 1) > range->u.ur.range[0].max ) {
            *err_pos = range->u.ur.range[0].max;
            return ICLI_RC_ERR_MATCH;
        } else {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    //check max length, 2 is for " and "
    if ( range->u.ur.range[0].max > range->u.ur.range[0].min ) {
        if ( (len - 2) < range->u.ur.range[0].min ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    if ( (len - 2) > range->u.ur.range[0].max ) {
        *err_pos = range->u.ur.range[0].max;
        return ICLI_RC_ERR_MATCH;
    }

    //do not copy ""
    (void)vtss_icli_str_ncpy(val, word + 1, len - 2);
    val[len - 2] = 0;
    return ICLI_RC_OK;
}

/*
    line with length limitation

    INPUT
        word  : user input word
        range : length limitation

    OUTPUT
        val     : a string
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1,
        this check should be before calling this API
*/
static i32 _range_line_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    u32     len = vtss_icli_str_len( word );

    if ( err_pos ) {}

#if 1 /* CP, 2012/10/24 14:08, Bugzilla#10092 - runtime variable word length */
    if ( range->type != ICLI_RANGE_TYPE_UNSIGNED ) {
        T_E("range type must be ICLI_RANGE_TYPE_UNSIGNED, but not %d\n", range->type);
        return ICLI_RC_ERR_PARAMETER;
    }
#endif

    // no string
    if ( len == 0 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    if ( range->u.ur.range[0].max > range->u.ur.range[0].min ) {
        if ( (len) < range->u.ur.range[0].min ) {
            return ICLI_RC_ERR_INCOMPLETE;
        }
    }

    if ( (len) > range->u.ur.range[0].max ) {
        if ( err_pos ) {
            *err_pos = range->u.ur.range[0].max;
        }
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_ncpy(val, word, len);
    val[len] = 0;

    return ICLI_RC_OK;
}

/*
    A valid hostname is a string drawn from the alphabet(A-Za-z), digits(0-9),
    dot(.), hyphen(-). Spaces are not allowed, the first character must be an
    alphanumeric character, and the first and last characters must not be a
    dot or a hyphen.

    INPUT
        word : user input word

    OUTPUT
        val     : a host name
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _hostname_get(
    IN  char            *word,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c = word;

    u32     len = vtss_icli_str_len( word );

    if ( err_pos ) {}

    // no string
    if ( len == 0 ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    if ( len > ICLI_HOSTNAME_MAX_LEN ) {
        if ( err_pos ) {
            *err_pos = ICLI_HOSTNAME_MAX_LEN;
        }
        return ICLI_RC_ERR_MATCH;
    }

    // the first char must be alphabet
    if ( !ICLI_IS_ALPHABET(*c) && !ICLI_IS_DIGIT(*c) ) {
        _ERR_MATCH_RETURN();
    }

    // check each char
    for ( ++c; ICLI_NOT_(EOS, *c); ++c ) {
        if ( !ICLI_IS_ALPHABET(*c) && !ICLI_IS_DIGIT(*c) && ICLI_NOT_(DASH, *c) && ICLI_NOT_(DOT, *c) ) {
            _ERR_MATCH_RETURN();
        }
    }

    // the first char must be alphabet
    --c;
    if ( !ICLI_IS_ALPHABET(*c) && !ICLI_IS_DIGIT(*c) ) {
        _ERR_MATCH_RETURN();
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

static BOOL _compress_unsigned_range(
    INOUT icli_unsigned_range_t     *ur
)
{
    u32     i;

    if ( ur->cnt <= 1 ) {
        return TRUE;
    }

    for ( i = 1; i < ur->cnt; i++ ) {
        if ( ur->range[i].min != (ur->range[0].max + 1) ) {
            return FALSE;
        }

        /* update max */
        ur->range[0].max = ur->range[i].max;
    }

    ur->cnt = 1;
    return TRUE;
}

/*
    check if vcap_vr

    INPUT
        word : user input word

    OUTPUT
        val     : vcap_vr
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _vcap_vr_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT icli_vcap_vr_t  *val,
    OUT u32             *err_pos
)
{
    /*
        ICLI_RANGE_TYPE_SIGNED:   b_odd_range = TRUE
        ICLI_RANGE_TYPE_UNSIGNED: b_odd_range = FALSE
        ur : real range
    */
    BOOL            b_odd_range;
    i32             r;
    icli_range_t    rv;
    u32             mask;
    u32             low;
    u32             high;
    u32             min;
    u32             max;
    BOOL            b_error;

    b_odd_range = FALSE;
    if ( range->type == ICLI_RANGE_TYPE_SIGNED ) {
        b_odd_range = TRUE;
        range->type = ICLI_RANGE_TYPE_UNSIGNED;
    }

    r = _range_list_get_with_delimiter( word, ICLI_DASH, range, &rv, err_pos);
    if ( r != ICLI_RC_OK ) {
        return r;
    }

    if ( _compress_unsigned_range( &(rv.u.ur) ) == FALSE ) {
        return ICLI_RC_ERR_MATCH;
    }

    low  = rv.u.ur.range[0].min;
    high = rv.u.ur.range[0].max;

    /* check odd range */
    if ( b_odd_range == FALSE ) {
        min  = range->u.ur.range[0].min;
        max  = range->u.ur.range[0].max;

        b_error = TRUE;
        for ( mask = 0; mask <= max; mask = (mask * 2 + 1) ) {
            if ( mask < min ) {
                continue;
            }
            if ((low & ~mask) == (high & ~mask) && /* Upper bits match */
                (low & mask)  == 0              && /* Lower bits of 'low' are zero */
                (high | mask) == high)          {  /* Lower bits of 'high are one */
                b_error = FALSE;
                break;
            }
        }

        if ( b_error ) {
            return ICLI_RC_ERR_MATCH;
        }
    }

    val->low  = (u16)low;
    val->high = (u16)high;

    return ICLI_RC_OK;
}

/*
    check if hex value array

    INPUT
        word : user input word

    OUTPUT
        val     : hex values
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _hexval_get(
    IN  char            *word,
    OUT icli_hexval_t   *val,
    OUT u32             *err_pos
)
{
    //common
    char            *c;
    //by type
    i32             i;
    BOOL            b_first;
    BOOL            b_begin;
    icli_hexval_t   hval;

    if ( vtss_icli_str_len(word) > ( ICLI_HEXVAL_MAX_LEN + 1 ) ) {
        *err_pos = ICLI_HEXVAL_MAX_LEN + 1;
        return ICLI_RC_ERR_MATCH;
    }

    c = word;
    if ( (*c) != '0' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    } else if ( (*c) != 'x' && (*c) != 'X' ) {
        _ERR_MATCH_RETURN();
    }

    ++c;
    if ( ICLI_IS_(EOS, *c) ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    memset(&hval, 0, sizeof(hval));
    b_begin = TRUE;
    b_first = TRUE;

    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        i = _hex_get_c(*c);
        if ( i == -1 ) {
            _ERR_MATCH_RETURN();
        }

        //get begin position of 1 happen
        if ( b_begin ) {
            if ( vtss_icli_str_len(word) % 2 ) {
                b_first = FALSE;
            } else {
                b_first = TRUE;
            }
            b_begin = FALSE;
        }

        if ( b_first ) {
            i <<= 4;
            hval.hex[ hval.len ] = (u8)i;
            b_first = FALSE;
        } else {
            hval.hex[ hval.len ] += (u8)i;
            ++( hval.len );
            b_first = TRUE;
        }
    }

    (*val) = hval;
    return ICLI_RC_OK;
}

/*
    hex value string with length limitation

    INPUT
        word  : user input word
        range : length limitation

    OUTPUT
        val     : hex values
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _range_hexval_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT icli_hexval_t   *val,
    OUT u32             *err_pos
)
{
    icli_hexval_t   hval;
    int             rc;

    rc = _hexval_get( word, &hval, err_pos );
    if ( rc != ICLI_RC_OK ) {
        return rc;
    }

    if ( hval.len < range->u.ur.range[0].min ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    if ( hval.len > range->u.ur.range[0].max ) {
        *err_pos = (range->u.ur.range[0].max + 1) * 2;
        return ICLI_RC_ERR_MATCH;
    }

    *val = hval;
    return ICLI_RC_OK;
}

/*
    check if single word 1. can not be all in 0-9, 2. allow a-zA-Z only

    INPUT
        word : user input word

    OUTPUT
        val     : a VLAN word
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        the length of val should be at least vtss_icli_str_len(word)+1, this check
        should be before calling this API
*/
static i32 _vword_get(
    IN  char            *word,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    BOOL    b_digit;

    b_digit = TRUE;
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ! ICLI_IS_DIGIT(*c) ) {
            if ( ICLI_IS_ALPHABET(*c) ) {
                b_digit = FALSE;
            } else {
                break;
            }
        }
    }

    if ( ICLI_NOT_(EOS, *c) && (! ICLI_IS_ALPHABET(*c)) ) {
        _ERR_MATCH_RETURN();
    }

    if ( b_digit ) {
        c = word;
        _ERR_MATCH_RETURN();
    }

    if ( vtss_icli_str_len(word) > ICLI_VALUE_STR_MAX_LEN ) {
        *err_pos = ICLI_VALUE_STR_MAX_LEN;
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

/*
    word with length limitation

    INPUT
        word  : user input word
        range : length limitation

    OUTPUT
        val     : range list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _range_vword_get(
    IN  char            *word,
    IN  icli_range_t    *range,
    OUT char            *val,
    OUT u32             *err_pos
)
{
    //common
    char    *c;
    u32     len;
    BOOL    b_digit;

#if 1 /* CP, 2012/10/24 14:08, Bugzilla#10092 - runtime variable word length */
    if ( range->type != ICLI_RANGE_TYPE_UNSIGNED ) {
        T_E("range type must be ICLI_RANGE_TYPE_UNSIGNED, but not %d\n", range->type);
        return ICLI_RC_ERR_PARAMETER;
    }
#endif

    b_digit = TRUE;
    for ( c = word; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ! ICLI_IS_DIGIT(*c) ) {
            if ( ICLI_IS_ALPHABET(*c) ) {
                b_digit = FALSE;
            } else {
                break;
            }
        }
    }

    if ( ICLI_NOT_(EOS, *c) && (! ICLI_IS_ALPHABET(*c)) ) {
        _ERR_MATCH_RETURN();
    }

    if ( b_digit ) {
        c = word;
        _ERR_MATCH_RETURN();
    }

    // check string length
    len = vtss_icli_str_len( word );

    if ( len < range->u.ur.range[0].min ) {
        return ICLI_RC_ERR_INCOMPLETE;
    }

    if ( len > range->u.ur.range[0].max ) {
        *err_pos = range->u.ur.range[0].max;
        return ICLI_RC_ERR_MATCH;
    }

    (void)vtss_icli_str_cpy(val, word);
    return ICLI_RC_OK;
}

/*
    check if switch ID

    INPUT
        word : user input word
    OUTPUT
        val     : switch ID (unsigned 16-bit integer value)
        err_pos : position where error happens
    RETURN
        icli_rc_t
*/
static i32 _switch_id_get(
    IN  char    *word,
    OUT u32     *val,
    OUT u32     *err_pos
)
{
    return _uint_get_in_range( word,
                               ICLI_MIN_SWICTH_ID,
                               ICLI_MAX_SWICTH_ID,
                               val,
                               err_pos );
}

/*
    check if switch list

    INPUT
        word : user input word

    OUTPUT
        val     : switch list
        err_pos : position where error happens

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
static i32 _switch_list_get(
    IN  char                    *word,
    OUT icli_unsigned_range_t   *val,
    OUT u32                     *err_pos
)
{
    return _uint_range_list_get_in_range( word,
                                          ICLI_DASH,
                                          ICLI_MIN_SWICTH_ID,
                                          ICLI_MAX_SWICTH_ID,
                                          val,
                                          err_pos);
}

#define _WORD_VARIABLE_RANGE_GET( x ) \
    r = vtss_icli_str_sub(vtss_icli_variable_data_variable_get( ICLI_VARIABLE_##x ), name, 1, NULL); \
    if ( r == 1 ) { \
        t = ICLI_VARIABLE_RANGE_##x; \
        range_str = name + vtss_icli_str_len( vtss_icli_variable_data_variable_get( ICLI_VARIABLE_##x ) ); \
        break; \
    }

/*
    get variable type and the corresponding range for the name
    the valid range formats are as follows.

        A       : 0-A
        A-B     : A-B
        A.C     : 0-A 0-C
        A-B.C   : A-B 0-C
        A-B.C-D : A-B C-D

    the number of dot is (ICLI_RANGE_LIST_CNT - 1) that is the number of range list.

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
static i32 _str_word_variable_range(
    IN  char                    *name,
    OUT icli_variable_type_t    *type,
    OUT icli_range_t            *range
)
{
    i32                     r;
    icli_variable_type_t    t;
    char                    *range_str;
    char                    *c;
    char                    *dot_c;
    BOOL                    b_dot;
    i32                     loop;
    icli_unsigned_range_t   ur;
    u32                     n;
    u32                     max_len;
    BOOL                    b_dash;

    r         = 0;
    t         = ICLI_VARIABLE_RANGE_WORD;
    c         = NULL;
    loop      = 1;
    range_str = NULL;

    do {
        _WORD_VARIABLE_RANGE_GET( WORD );    // ICLI_VARIABLE_RANGE_WORD
        _WORD_VARIABLE_RANGE_GET( KWORD );   // ICLI_VARIABLE_RANGE_KWORD
        _WORD_VARIABLE_RANGE_GET( DWORD );   // ICLI_VARIABLE_RANGE_DWORD
        _WORD_VARIABLE_RANGE_GET( FWORD );   // ICLI_VARIABLE_RANGE_FWORD
        _WORD_VARIABLE_RANGE_GET( STRING );  // ICLI_VARIABLE_RANGE_STRING
        _WORD_VARIABLE_RANGE_GET( LINE );    // ICLI_VARIABLE_RANGE_LINE
        _WORD_VARIABLE_RANGE_GET( HEXVAL );  // ICLI_VARIABLE_RANGE_HEXVAL
        _WORD_VARIABLE_RANGE_GET( VWORD );   // ICLI_VARIABLE_RANGE_VWORD

    } while ( loop == 0 );

    if ( r != 1 ) {
        return ICLI_RC_ERR_MATCH;
    }

    // for lint
    if ( range_str == NULL ) {
        return ICLI_RC_ERR_MATCH;
    }

    /* check range_str */
    c = range_str;
    if ( ! ICLI_IS_DIGIT(*c) ) {
        return ICLI_RC_ERR_MATCH;
    }
    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        ;
    }
    if ( ! ICLI_IS_DIGIT(*(c - 1)) ) {
        return ICLI_RC_ERR_MATCH;
    }

    memset( &ur, 0, sizeof(ur) );

    c = range_str;
    max_len = ICLI_VALUE_STR_MAX_LEN;
    while ( ICLI_NOT_(EOS, *c) ) {
        // find dot
        for ( dot_c = c; ICLI_NOT_(DOT, *dot_c) && ICLI_NOT_(EOS, *dot_c); ++dot_c ) {
            ;
        }
        b_dot = FALSE;
        if ( ICLI_IS_(DOT, *dot_c) ) {
            *dot_c = ICLI_EOS;
            b_dot = TRUE;
        }

        // get max_len
        if ( ur.cnt ) {
            max_len -= (ur.range[0].max + 1);
        }

        // get range
        r = _uint_range_list_get_in_range( c, ICLI_DASH, 1, max_len, &ur, NULL );
        if ( r != ICLI_RC_OK ) {
            break;
        }

        // only 1 range is allowed
        if ( ur.cnt > 1 ) {
            r = ICLI_RC_ERR_MATCH;
            break;
        }

        // check min
        if ( ur.range[0].min == ur.range[0].max ) {
            b_dash = FALSE;
            for ( ; ICLI_NOT_(EOS, *c); ++c ) {
                if ( ICLI_IS_(DASH, *c) ) {
                    b_dash = TRUE;
                    break;
                }
            }
            if ( ! b_dash ) {
                ur.range[0].min = 0;
            }
        }

        // put into output
        n = range->u.ur.cnt;
        range->u.ur.range[n] = ur.range[0];
        ++( range->u.ur.cnt );

        // update c
        if ( b_dot ) {
            *dot_c = ICLI_DOT;
            c = dot_c + 1;
        } else {
            c = dot_c;
        }
    }

    if ( r == ICLI_RC_OK ) {
        *type = t;
        range->type = ICLI_RANGE_TYPE_UNSIGNED;
    }

    return r;
}

/*
==============================================================================

    Public Function

==============================================================================
*/
/*
    parse string to get signed/unsigned range

    INPUT
        range_str : string of range
        delimiter : the delimiter for range

    OUTPUT
        range : range

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_variable_range_get(
    IN  char            *range_str,
    IN  char            delimiter,
    OUT icli_range_t    *range
)
{
    u32     err_pos;

    ICLI_PARAMETER_NULL_CHECK( range_str );
    ICLI_PARAMETER_NULL_CHECK( range );

    return _range_list_get_with_delimiter(range_str, delimiter, NULL, range, &err_pos);
}

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
)
{
    i32     i;
    i32     r;
    u32     err_pos;

    ICLI_PARAMETER_NULL_CHECK( name );
    ICLI_PARAMETER_NULL_CHECK( type );
    ICLI_PARAMETER_NULL_CHECK( range );

    for ( i = 0; i < ICLI_VARIABLE_KEYWORD; ++i ) {
        if ( vtss_icli_str_cmp(name, vtss_icli_variable_data_variable_get(i)) == 0 ) {
            *type = (icli_variable_type_t)i;
            return ICLI_RC_OK;
        }
    }

    r = _range_list_get_with_delimiter( name, ICLI_DASH, NULL, range, &err_pos );
    if ( r == ICLI_RC_OK ) {
        if ( range->type == ICLI_RANGE_TYPE_SIGNED ) {
            *type = ICLI_VARIABLE_RANGE_INT;
        } else {
            *type = ICLI_VARIABLE_RANGE_UINT;
        }
        return ICLI_RC_OK;
    }

    if ( r == ICLI_RC_ERR_RANGE ) {
        return r;
    }

    r = _range_list_get_with_delimiter( name, ICLI_TILDE, NULL, range, &err_pos );
    if ( r == ICLI_RC_OK ) {
        if ( range->type == ICLI_RANGE_TYPE_SIGNED ) {
            if ( range->u.sr.cnt != 1 ) {
                T_E("invalid range cnt = %u\n", range->u.sr.cnt);
                return ICLI_RC_ERROR;
            }
        } else {
            if ( range->u.ur.cnt != 1 ) {
                T_E("invalid range cnt = %u\n", range->u.ur.cnt);
                return ICLI_RC_ERROR;
            }
        }

        if ( range->type == ICLI_RANGE_TYPE_SIGNED ) {
            *type = ICLI_VARIABLE_RANGE_INT_LIST;
        } else {
            *type = ICLI_VARIABLE_RANGE_UINT_LIST;
        }
        return ICLI_RC_OK;
    }

    if ( r == ICLI_RC_ERR_RANGE ) {
        return r;
    }

    r = _str_word_variable_range( name, type, range );
    return r;
}

#define CASE_VARIABLE_N(x,y,n) \
    case ICLI_VARIABLE_##x : \
        if ( n ) { \
            rc = _##y##_get_1(word, &(value->u.u_##y), err_pos); \
        } else { \
            rc = _##y##_get_0(word, &(value->u.u_##y), err_pos); \
        } \
        break

#define CASE_VARIABLE(x,y) \
    case ICLI_VARIABLE_##x : \
        rc = _##y##_get(word, &(value->u.u_##y), err_pos); \
        break

#define CASE_VARIABLE_RANGE(x,y) \
    case ICLI_VARIABLE_##x : \
        rc = _##y##_get(word, range, &(value->u.u_##y), err_pos); \
        break

#define CASE_VARIABLE_P(x,y) \
    case ICLI_VARIABLE_##x : \
        rc = _##y##_get(word, value->u.u_##y, err_pos); \
        break

#define CASE_VARIABLE_RANGE_P(x,y) \
    case ICLI_VARIABLE_##x : \
        rc = _##y##_get(word, range, value->u.u_##y, err_pos); \
        break

/*
    get value from the word for the type
    ICLI_VARIABLE_TYPE_MODIFY

    INPUT
        type  : variable type
        word  : user input word
        range : range checking

    OUTPUT
        value   : corresponding value of the type if successful
        err_pos : error position if failed

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_variable_get(
    IN  icli_variable_type_t    type,
    IN  char                    *word,
    IN  icli_range_t            *range,
    OUT icli_variable_value_t   *value,
    OUT u32                     *err_pos
)
{
    i32     rc = -1;
    u32     len;
    char    *c;
    i32     style;

    ICLI_PARAMETER_NULL_CHECK( word );
    ICLI_PARAMETER_NULL_CHECK( value );

    /* pre-processing word for type */
    c = NULL;
    switch (type) {
    case ICLI_VARIABLE_LINE:
    case ICLI_VARIABLE_RANGE_LINE:
    case ICLI_VARIABLE_GREP_STRING:
        break;
    default:
        len = vtss_icli_str_len( word );
        if ( word[len - 1] == ICLI_SPACE ) {
            // find the last non-space char
            for ( c = &(word[len - 1]); ICLI_IS_(SPACE, *c); --c ) {
                ;
            }
            // replace with EOS
            *(++c) = ICLI_EOS;
        }
        break;
    }

    /* get style to avoid compile warning */
    style = ICLI_VARIABLE_STYLE;

    /* get type value */
    switch (type) {

        CASE_VARIABLE_N( MAC_ADDR,  mac_addr,  style );     // ICLI_VARIABLE_MAC_ADDR
        CASE_VARIABLE_N( MAC_UCAST, mac_ucast, style );     // ICLI_VARIABLE_MAC_UCAST
        CASE_VARIABLE_N( MAC_MCAST, mac_mcast, style );     // ICLI_VARIABLE_MAC_MCAST
        CASE_VARIABLE_N( CLOCK_ID,  clock_id,  style );     // ICLI_VARIABLE_CLOCK_ID

        CASE_VARIABLE( IPV4_ADDR,       ipv4_addr );        // ICLI_VARIABLE_IPV4_ADDR
        CASE_VARIABLE( IPV4_NETMASK,    ipv4_netmask );     // ICLI_VARIABLE_IPV4_NETMASK
        CASE_VARIABLE( IPV4_WILDCARD,   ipv4_wildcard );    // ICLI_VARIABLE_IPV4_WILDCARD
        CASE_VARIABLE( IPV4_UCAST,      ipv4_ucast );       // ICLI_VARIABLE_IPV4_UCAST
        CASE_VARIABLE( IPV4_MCAST,      ipv4_mcast );       // ICLI_VARIABLE_IPV4_MCAST
        CASE_VARIABLE( IPV4_NMCAST,     ipv4_nmcast );      // ICLI_VARIABLE_IPV4_NMCAST
        CASE_VARIABLE( IPV4_ABC,        ipv4_abc );         // ICLI_VARIABLE_IPV4_ABC
        CASE_VARIABLE( IPV4_SUBNET,     ipv4_subnet );      // ICLI_VARIABLE_IPV4_SUBNET
        CASE_VARIABLE( IPV4_PREFIX,     ipv4_prefix );      // ICLI_VARIABLE_IPV4_PREFIX

        CASE_VARIABLE( IPV6_ADDR,       ipv6_addr );        // ICLI_VARIABLE_IPV6_ADDR
        CASE_VARIABLE( IPV6_NETMASK,    ipv6_netmask );     // ICLI_VARIABLE_IPV6_NETMASK
        CASE_VARIABLE( IPV6_UCAST,      ipv6_ucast );       // ICLI_VARIABLE_IPV6_UCAST
        CASE_VARIABLE( IPV6_MCAST,      ipv6_mcast );       // ICLI_VARIABLE_IPV6_MCAST
        CASE_VARIABLE( IPV6_LINKLOCAL,  ipv6_linklocal );   // ICLI_VARIABLE_IPV6_LINKLOCAL
        CASE_VARIABLE( IPV6_SUBNET,     ipv6_subnet );      // ICLI_VARIABLE_IPV6_SUBNET
        CASE_VARIABLE( IPV6_PREFIX,     ipv6_prefix );      // ICLI_VARIABLE_IPV6_PREFIX

        CASE_VARIABLE( INT8,    int8 );     // ICLI_VARIABLE_INT8
        CASE_VARIABLE( INT16,   int16 );    // ICLI_VARIABLE_INT16
        CASE_VARIABLE( INT,     int );      // ICLI_VARIABLE_INT
        CASE_VARIABLE( UINT8,   uint8 );    // ICLI_VARIABLE_UINT8
        CASE_VARIABLE( UINT16,  uint16 );   // ICLI_VARIABLE_UINT16
        CASE_VARIABLE( UINT,    uint );     // ICLI_VARIABLE_UINT

        CASE_VARIABLE( PORT_ID,     port_id );      // ICLI_VARIABLE_PORT_ID
        CASE_VARIABLE( PORT_LIST,   port_list );    // ICLI_VARIABLE_PORT_LIST
        CASE_VARIABLE( PORT_TYPE,   port_type );    // ICLI_VARIABLE_PORT_TYPE

        CASE_VARIABLE( PORT_TYPE_ID,    port_type_id );     // ICLI_VARIABLE_PORT_TYPE_ID
        CASE_VARIABLE( PORT_TYPE_LIST,  port_type_list );   // ICLI_VARIABLE_PORT_TYPE_LIST

        CASE_VARIABLE( VLAN_ID,     vlan_id );      // ICLI_VARIABLE_VLAN_ID
        CASE_VARIABLE( VLAN_LIST,   vlan_list );    // ICLI_VARIABLE_VLAN_LIST

        CASE_VARIABLE_RANGE( RANGE_INT,   range_int );      // ICLI_VARIABLE_RANGE_INT
        CASE_VARIABLE_RANGE( RANGE_UINT,  range_uint );     // ICLI_VARIABLE_RANGE_UINT

        CASE_VARIABLE_RANGE( RANGE_LIST,        range_list );       // ICLI_VARIABLE_RANGE_LIST
        CASE_VARIABLE_RANGE( RANGE_INT_LIST,    range_int_list );   // ICLI_VARIABLE_RANGE_INT_LIST
        CASE_VARIABLE_RANGE( RANGE_UINT_LIST,   range_uint_list );  // ICLI_VARIABLE_RANGE_UINT_LIST

        CASE_VARIABLE( DATE, date );    // ICLI_VARIABLE_DATE
        CASE_VARIABLE( TIME, time );    // ICLI_VARIABLE_TIME
        CASE_VARIABLE( HHMM, hhmm );    // ICLI_VARIABLE_HHMM

        CASE_VARIABLE_P( WORD,          word );     // ICLI_VARIABLE_WORD
        CASE_VARIABLE_P( KWORD,         kword );    // ICLI_VARIABLE_KWORD
        CASE_VARIABLE_P( CWORD,         cword );    // ICLI_VARIABLE_CWORD
        CASE_VARIABLE_P( DWORD,         dword );    // ICLI_VARIABLE_DWORD
        CASE_VARIABLE_P( FWORD,         fword );    // ICLI_VARIABLE_FWORD
        CASE_VARIABLE_P( STRING,        string );   // ICLI_VARIABLE_STRING
        CASE_VARIABLE_P( LINE,          line );     // ICLI_VARIABLE_LINE
        CASE_VARIABLE_P( HOSTNAME,      hostname ); // ICLI_VARIABLE_HOSTNAME
        CASE_VARIABLE_P( GREP_STRING,   line );     // ICLI_VARIABLE_GREP_STRING

        CASE_VARIABLE( GREP,            grep );             // ICLI_VARIABLE_GREP
        CASE_VARIABLE( GREP_BEGIN,      grep_begin );       // ICLI_VARIABLE_GREP_BEGIN
        CASE_VARIABLE( GREP_INCLUDE,    grep_include );     // ICLI_VARIABLE_GREP_INCLUDE
        CASE_VARIABLE( GREP_EXCLUDE,    grep_exclude );     // ICLI_VARIABLE_GREP_EXCLUDE

        CASE_VARIABLE_RANGE_P( RANGE_WORD,      range_word );       // ICLI_VARIABLE_RANGE_WORD
        CASE_VARIABLE_RANGE_P( RANGE_KWORD,     range_kword );      // ICLI_VARIABLE_RANGE_KWORD
        CASE_VARIABLE_RANGE_P( RANGE_DWORD,     range_dword );      // ICLI_VARIABLE_RANGE_DWORD
        CASE_VARIABLE_RANGE_P( RANGE_FWORD,     range_fword );      // ICLI_VARIABLE_RANGE_FWORD
        CASE_VARIABLE_RANGE_P( RANGE_STRING,    range_string );     // ICLI_VARIABLE_RANGE_STRING
        CASE_VARIABLE_RANGE_P( RANGE_LINE,      range_line );       // ICLI_VARIABLE_RANGE_LINE

        CASE_VARIABLE( OUI, oui );                  // ICLI_VARIABLE_OUI
        CASE_VARIABLE( PCP, pcp );                  // ICLI_VARIABLE_PCP
        CASE_VARIABLE( DPL, dpl );                  // ICLI_VARIABLE_DPL

        CASE_VARIABLE_RANGE( VCAP_VR, vcap_vr );    // ICLI_VARIABLE_VCAP_VR

        CASE_VARIABLE( HEXVAL, hexval );            // ICLI_VARIABLE_HEXVAL
        CASE_VARIABLE_RANGE( RANGE_HEXVAL, range_hexval );  // ICLI_VARIABLE_RANGE_HEXVAL

        CASE_VARIABLE_P( VWORD, vword ); // ICLI_VARIABLE_VWORD
        CASE_VARIABLE_RANGE_P( RANGE_VWORD, range_vword ); // ICLI_VARIABLE_RANGE_VWORD

        CASE_VARIABLE( SWITCH_ID,   switch_id );    // ICLI_VARIABLE_SWITCH_ID
        CASE_VARIABLE( SWITCH_LIST, switch_list );  // ICLI_VARIABLE_SWITCH_LIST

    default:
        T_E("invalid type: %d\n", type);
        break;

    }//switch (type)

    /* post-processing word for type */
    if ( c ) {
        // put space back
        *c = ICLI_SPACE;
    }

    if ( rc == ICLI_RC_OK ) {
        value->type = type;
    }

    return rc;
}

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
)
{
    if ( type < ICLI_PORT_TYPE_MAX ) {
        return g_port_type_data[type].name;
    } else {
        return "Unknown";
    }
}

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
)
{
    if ( type < ICLI_PORT_TYPE_MAX ) {
        return g_port_type_data[type].short_name;
    } else {
        return "Unkn";
    }
}

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
)
{
    if ( type < ICLI_PORT_TYPE_MAX ) {
        return g_port_type_data[type].help;
    } else {
        return "Unknown";
    }
}

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
)
{
    return g_dscp_wvh;
}
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
)
{
    if ( __decimal_get(word, val, NULL) != ICLI_RC_OK ) {
        return FALSE;
    }
    return TRUE;
}
#endif

char *vtss_icli_variable_data_name_get(
    IN  icli_variable_type_t    type
)
{
    if ( type >= ICLI_VARIABLE_MAX ) {
        T_E("invalid type = %u\n", type);
        return "";
    }
    return g_variable_data[type].name;
}

char *vtss_icli_variable_data_variable_get(
    IN  icli_variable_type_t    type
)
{
    if ( type >= ICLI_VARIABLE_MAX ) {
        T_E("invalid type = %u\n", type);
        return "";
    }
    return g_variable_data[type].variable;
}

char *vtss_icli_variable_data_decl_type_get(
    IN  icli_variable_type_t    type
)
{
    if ( type >= ICLI_VARIABLE_MAX ) {
        T_E("invalid type = %u\n", type);
        return "";
    }
    return g_variable_data[type].decl_type;
}

char *vtss_icli_variable_data_init_val_get(
    IN  icli_variable_type_t    type
)
{
    if ( type >= ICLI_VARIABLE_MAX ) {
        T_E("invalid type = %u\n", type);
        return "";
    }
    return g_variable_data[type].init_val;
}

BOOL vtss_icli_variable_data_pointer_type_get(
    IN  icli_variable_type_t    type
)
{
    if ( type >= ICLI_VARIABLE_MAX ) {
        T_E("invalid type = %u\n", type);
        return FALSE;
    }
    return g_variable_data[type].b_pointer_type;
}

BOOL vtss_icli_variable_data_string_type_get(
    IN  icli_variable_type_t    type
)
{
    if ( type >= ICLI_VARIABLE_MAX ) {
        T_E("invalid type = %u\n", type);
        return FALSE;
    }
    return g_variable_data[type].b_string_type;
}

BOOL vtss_icli_variable_data_has_range_get(
    IN  icli_variable_type_t    type
)
{
    if ( type >= ICLI_VARIABLE_MAX ) {
        T_E("invalid type = %u\n", type);
        return FALSE;
    }
    return g_variable_data[type].b_has_range;
}
