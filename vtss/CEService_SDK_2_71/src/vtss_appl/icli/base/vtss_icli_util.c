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

    utility for ICLI engine

    Revision history
    > CP.Wang, 05/29/2013 13:09
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
/*
    count the number of words in a command string.
    without ID and ID Delimiter

    INPUT
        cmd : command string

    OUTPUT
        n/a

    RETURN
        >=0: number of words in command string
        <0 : failed, icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_word_count(
    IN  char *cmd
)
{
    char    *c;
    i32     state;
    i32     n;

    ICLI_PARAMETER_NULL_CHECK( cmd );

    c = cmd;
    n = 0;
    state = ICLI_WORD_STATE_SPACE;

    for ( c = cmd; ; ++c ) {

        switch (state) {
        case ICLI_WORD_STATE_SPACE:
            if ( ICLI_IS_KEYWORD(*c) ) {
                state = ICLI_WORD_STATE_WORD;
                continue;
            }
            if ( ICLI_IS_SPACE_CHAR(*c) ) {
#if ICLI_RANDOM_MUST_NUMBER
                if ( ICLI_IS_(MANDATORY_END, *c) ) {
                    if ( ICLI_IS_RANDOM_MUST(c) ) {
                        c += 2;
                    }
                }
#endif
                continue;
            }
            if ( ICLI_IS_(VARIABLE_BEGIN, (*c)) ) {
                state = ICLI_WORD_STATE_VARIABLE_BEGIN;
                continue;
            }
            if ( ICLI_IS_(EOS, (*c)) ) {
                return n;
            }
            break;

        case ICLI_WORD_STATE_WORD:
            if ( ICLI_IS_KEYWORD(*c) ) {
                continue;
            }
            if ( ICLI_IS_SPACE_CHAR(*c) ) {
                state = ICLI_WORD_STATE_SPACE;
                ++n;
#if ICLI_RANDOM_MUST_NUMBER
                if ( ICLI_IS_(MANDATORY_END, *c) ) {
                    if ( ICLI_IS_RANDOM_MUST(c) ) {
                        c += 2;
                    }
                }
#endif
                continue;
            }
            if ( ICLI_IS_(VARIABLE_BEGIN, (*c)) ) {
                state = ICLI_WORD_STATE_VARIABLE_BEGIN;
                ++n;
                continue;
            }
            if ( ICLI_IS_(EOS, (*c)) ) {
                return ++n;
            }
            break;

        case ICLI_WORD_STATE_VARIABLE_BEGIN:
            if ( ICLI_IS_VARNAME(*c) || ICLI_IS_(SQUOTE, *c) ) {
                state = ICLI_WORD_STATE_VARIABLE;
                continue;
            }
            break;

        case ICLI_WORD_STATE_VARIABLE:
            if ( ICLI_IS_VARNAME(*c) || ICLI_IS_(SQUOTE, *c) ) {
                continue;
            }
            if ( ICLI_IS_(VARIABLE_END, (*c)) ) {
                state = ICLI_WORD_STATE_VARIABLE_END;
                continue;
            }
            break;

        case ICLI_WORD_STATE_VARIABLE_END:
            if ( ICLI_IS_SPACE_CHAR(*c) ) {
                state = ICLI_WORD_STATE_SPACE;
                ++n;
#if ICLI_RANDOM_MUST_NUMBER
                if ( ICLI_IS_(MANDATORY_END, *c) ) {
                    if ( ICLI_IS_RANDOM_MUST(c) ) {
                        c += 2;
                    }
                }
#endif
                continue;
            }
            if ( ICLI_IS_(EOS, (*c)) ) {
                return ++n;
            }
            break;

        default:
            break;
        }
        return ICLI_RC_ERROR;

    }//for
}

/*
    get next word, keyword or variable

    INPUT
        cmd : command string

    OUTPUT
        cmd : the string behind the word got

    RETURN
        char * : word
        NULL   : no word

    COMMENT
        cmd will be damaged.
*/
char *vtss_icli_word_get_next(
    INOUT  char **cmd
)
{
    char    *c,
            *w;
    i32     state;

    if ( cmd == NULL ) {
        return NULL;
    }

    c     = *cmd;
    w     = NULL;
    state = ICLI_WORD_STATE_SPACE;

    for ( c = *cmd; ; ++c ) {

        switch (state) {
        case ICLI_WORD_STATE_SPACE:
            if ( ICLI_IS_KEYWORD(*c) ) {
                state = ICLI_WORD_STATE_WORD;
                w = c;
                continue;
            }
            if ( ICLI_IS_SPACE_CHAR(*c) ) {
#if ICLI_RANDOM_MUST_NUMBER
                if ( ICLI_IS_(MANDATORY_END, *c) ) {
                    if ( ICLI_IS_RANDOM_MUST(c) ) {
                        c += 2;
                    }
                }
#endif
                continue;
            }
            if ( ICLI_IS_(VARIABLE_BEGIN, (*c)) ) {
                state = ICLI_WORD_STATE_VARIABLE_BEGIN;
                w = c;
                continue;
            }
            if ( ICLI_IS_(EOS, (*c)) ) {
                *cmd = c;
                return w;
            }
            break;

        case ICLI_WORD_STATE_WORD:
            if ( ICLI_IS_KEYWORD(*c) ) {
                continue;
            }
            if ( ICLI_IS_SPACE_CHAR(*c) ) {
                *c   = 0;
#if ICLI_RANDOM_MUST_NUMBER
                if ( ICLI_IS_(MANDATORY_END, *c) ) {
                    if ( ICLI_IS_RANDOM_MUST(c) ) {
                        c += 2;
                    }
                }
#endif
                *cmd = c + 1;
                return w;
            }
            if ( ICLI_IS_(EOS, (*c)) ) {
                *cmd = c;
                return w;
            }
            break;

        case ICLI_WORD_STATE_VARIABLE_BEGIN:
            if ( ICLI_IS_VARNAME(*c) || ICLI_IS_(SQUOTE, *c) ) {
                state = ICLI_WORD_STATE_VARIABLE;
                continue;
            }
            break;

        case ICLI_WORD_STATE_VARIABLE:
            if ( ICLI_IS_VARNAME(*c) || ICLI_IS_(SQUOTE, *c) ) {
                continue;
            }
            if ( ICLI_IS_(VARIABLE_END, (*c)) ) {
                state = ICLI_WORD_STATE_VARIABLE_END;
                continue;
            }
            break;

        case ICLI_WORD_STATE_VARIABLE_END:
            if ( ICLI_IS_SPACE_CHAR(*c) ) {
                *c   = 0;
#if ICLI_RANDOM_MUST_NUMBER
                if ( ICLI_IS_(MANDATORY_END, *c) ) {
                    if ( ICLI_IS_RANDOM_MUST(c) ) {
                        c += 2;
                    }
                }
#endif
                *cmd = c + 1;
                return w;
            }
            if ( ICLI_IS_(EOS, (*c)) ) {
                *cmd = c;
                return w;
            }
            break;

        default:
            break;
        }
        return NULL;

    }//for
}

/*
    convert the string into upper case

    INPUT
        s : string

    OUTPUT
        s : converted string

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_to_upper_case(
    INOUT char  *s
)
{
    ICLI_PARAMETER_NULL_CHECK( s );

    for ( ; (*s) != 0; ++s ) {
        *s = ((*s) >= 'a' && (*s) <= 'z') ? (*s) + ('A' - 'a') : (*s);
    }
    return ICLI_RC_OK;
}

/*
    convert the string to lower case

    INPUT
        s : string

    OUTPUT
        s : converted string

    RETURN
        icli_rc_t

    COMMENT
        n/a
*/
i32 vtss_icli_to_lower_case(
    INOUT char  *s
)
{
    ICLI_PARAMETER_NULL_CHECK( s );

    for ( ; (*s) != 0; ++s ) {
        *s = ((*s) >= 'A' && (*s) <= 'Z') ? (*s) + ('a' - 'A') : (*s);
    }
    return 0;
}

/*
    get the length of string

    INPUT
        str : string

    OUTPUT
        n/a

    RETURN
        length of string
*/
u32 vtss_icli_str_len(
    IN  const char  *str
)
{
    const char   *c = str;

    if ( str == NULL || c == NULL ) {
        return 0;
    }

    while ( *c++ ) {
        ;
    }

    return (u32)(c - str - 1);

}

/*
    copy string from src to dst

    INPUT
        src : source string

    OUTPUT
        dst : destined string

    RETURN
        dst  : successful
        NULL : dst or src is NULL
*/
char *vtss_icli_str_cpy(
    OUT char        *dst,
    IN  const char  *src
)
{
    char    *d = dst;

    if (dst == NULL || src == NULL) {
        return NULL;
    }

    /* copy src over dst */
    while ( *src ) {
        *d++ = *src++;
    }

    *d = 0;
    return ( dst );

}

/*
    copy string from src to dst
    the max number(cnt) of char's to be copied

    INPUT
        src : source string
        cnt : number of char copied

    OUTPUT
        dst : destined string
              dst's buffer length should be more than cnt

    RETURN
        dst  : successful
        NULL : dst or src is NULL, or cnt is 0
*/
char *vtss_icli_str_ncpy(
    OUT char            *dst,
    IN  const char      *src,
    IN  u32             cnt
)
{
    char    *d = dst;

    if (dst == NULL || src == NULL || cnt == 0) {
        return NULL;
    }

    /* copy string */
    while ( cnt && (*src) ) {
        *d++ = *src++;
        --cnt;
    }
    *d = 0;

    /* pad out with zero */
    if ( cnt ) {
        while ( --cnt ) {
            *d++ = 0;
        }
    }

    return dst;
}

/*
    compare two strings, s1 and s2
    case sensitive

    INPUT
        s1 : string 1
        s2 : string 2

    OUTPUT
        n/a

    RETURN
         1 : s1 > s2
         0 : s1 == s2
        -1 : s1 < s2
        -2 : parameter error
*/
i32 vtss_icli_str_cmp(
    IN  const char  *s1,
    IN  const char  *s2
)
{
    i32     r;

    if ( s1 == NULL || s2 == NULL ) {
        return -2;
    }

    for ( r = 0; !r && *s1; ++s1, ++s2 ) {
        r = *s1 - *s2;
    }

    if ( r == 0 ) {
        if ( *s1 ) {
            return 1;
        }
        if ( *s2 ) {
            return -1;
        }
    }
    return (r < 0) ? -1 : (r == 0) ? 0 : 1;
}

/*
    check if sub is a sub-string of s

    INPUT
        sub : sub-string
        s   : string
        case_sensitive : check in case-sensitive or not
                         0 : case-insensitive
                         1 : case-sensitive

    OUTPUT
        err_pos : error position start from 0
                  work when it is present

    RETURN
         1 : sub-string
         0 : exactly match
        -1 : not even sub
*/
i32 vtss_icli_str_sub(
    IN  const char  *sub,
    IN  const char  *s,
    IN  u32         case_sensitive,
    OUT u32         *err_pos
)
{
    i32     r;
    u32     i;

    for ( r = 0, i = 0; !r && *sub; ++sub, ++s, ++i ) {
        r = *sub - *s;
        if ( r && case_sensitive == 0 ) {
            if ( 'a' <= (*sub) && (*sub) <= 'z' ) {
                if ( r == 'a' - 'A' ) {
                    r = 0;
                }
            } else if ( 'A' <= (*sub) && (*sub) <= 'Z' ) {
                if ( r == 'A' - 'a' ) {
                    r = 0;
                }
            }
        }
    }

    if ( r ) {
        if ( err_pos ) {
            *err_pos = i;
        }
        return -1;
    } else if ( *s ) {
        return 1;
    } else {
        return 0;
    }
}

/*
    concatenate src to the end of dst

    INPUT
        src : source string

    OUTPUT
        dst : destined string

    RETURN
        dst  : successful
        NULL : dst or src is NULL
*/
char *vtss_icli_str_concat(
    OUT char        *dst,
    IN  const char  *src
)
{
    char    *d = dst;

    if (dst == NULL || src == NULL) {
        return NULL;
    }

    /* go to the end of dst */
    while ( *d++ ) {
        ;
    }
    --d;

    /* append src */
    while ( *src ) {
        *d++ = *src++;
    }

    *d = 0;
    return ( dst );
}

/*
    find if sub is a substring IN s

    INPUT
        sub : substring
        s   : string

    OUTPUT
        none

    RETURN
        char* : start ptr in dst for src
        NULL  : sub or s is NULL or fail to find
*/
char *vtss_icli_str_str(
    IN  const char   *sub,
    IN  const char   *s
)
{
    int         len;
    const char  *psub,
          *ps;

    /* check src and dst */
    if ( sub == NULL || s == NULL ) {
        return NULL;
    }

    /* check length */
    if (vtss_icli_str_len(sub) > vtss_icli_str_len(s)) {
        return NULL;
    }
    len = vtss_icli_str_len(s) - vtss_icli_str_len(sub) + 1;

    /* find the first same char */
    for ( ; len > 0; ++s, --len ) {

        /* not the same, check next char */
        if ( (*s) != (*sub) ) {
            continue;
        }

        /* the same, check remaining of sub */
        psub = sub + 1;
        ps   = s   + 1;
        while ( (*psub) ) {
            if ( (*psub) == (*ps) ) {
                ++psub;
                ++ps;
            } else {
                break;/* while */
            }
        }

        if ( (*psub) == 0 ) {
            /* got it */
            return (char *)s;
        }
    }

    /* fail */
    return NULL;
}

/*
    IPv4 address: u32 to string

    INPUT
        ip : IPv4 address

    OUTPUT
        s : its buffer size must be at least 16 bytes,
            output string in the format of xxx.xxx.xxx.xxx

    RETURN
        s    : successful
        NULL : failed
*/
char *vtss_icli_ipv4_to_str(
    IN  vtss_ip_t   ip,
    OUT char        *s
)
{
    if ( s == NULL ) {
        return NULL;
    }
    icli_sprintf(s, "%u.%u.%u.%u",
                 (ip & 0xff000000) >> 24,
                 (ip & 0x00ff0000) >> 16,
                 (ip & 0x0000ff00) >>  8,
                 (ip & 0x000000ff) >>  0);
    return s;
}

/*
    get address class by IP

    INPUT
        ip : IPv4 address

    OUTPUT
        n/a

    RETURN
        icli_ipv4_class_t
*/
icli_ipv4_class_t vtss_icli_ipv4_class_get(
    IN  vtss_ip_t   ip
)
{
    u32     i;

    i = (ip & 0xff000000) >> 24;

    if ( i < 128 ) {
        return ICLI_IPV4_CLASS_A;
    } else if ( i < 192 ) {
        return ICLI_IPV4_CLASS_B;
    } else if ( i < 224 ) {
        return ICLI_IPV4_CLASS_C;
    } else if ( i < 240 ) {
        return ICLI_IPV4_CLASS_D;
    } else {
        return ICLI_IPV4_CLASS_E;
    }
}

/*
    get netmask by prefix

    INPUT
        prefix : 0 - 32

    OUTPUT
        netmask : 0.0.0.0 - 255.255.255.255

    RETURN
        TRUE  - successful
        FALSE - failed
*/
BOOL vtss_icli_ipv4_prefix_to_netmask(
    IN  u32         prefix,
    OUT vtss_ip_t   *netmask
)
{
    u32     i, j;

    if ( prefix > 32 ) {
        return FALSE;
    }

    if ( netmask == NULL ) {
        return FALSE;
    }

    *netmask = 0;
    for ( i = 0; i < prefix; ++i ) {
        j = 31 - i;
        (*netmask) |= (0x1L << j);
    }

    return TRUE;
}

/*
    get prefix by netmask

    INPUT
        netmask : 0.0.0.0 - 255.255.255.255

    OUTPUT
        prefix : 0 - 32

    RETURN
        TRUE  - successful
        FALSE - failed
*/
BOOL vtss_icli_ipv4_netmask_to_prefix(
    IN   vtss_ip_t   netmask,
    OUT  u32         *prefix
)
{
    u32         i;
    vtss_ip_t   n;

    if ( prefix == NULL ) {
        return FALSE;
    }

    if ( netmask == 0 ) {
        *prefix = 0;
        return TRUE;
    } else {
        for ( i = 1; i <= 32; ++i ) {
            if ( vtss_icli_ipv4_prefix_to_netmask(i, &n) ) {
                if ( netmask == n ) {
                    *prefix = i;
                    return TRUE;
                }
            } else {
                return FALSE;
            }
        }
    }

    return FALSE;
}

/*
    get prefix by IPv6 netmask

    INPUT
        netmask : :: - FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF

    OUTPUT
        prefix : 0 - 128

    RETURN
        TRUE  - successful
        FALSE - failed
*/
BOOL vtss_icli_ipv6_netmask_to_prefix(
    IN   vtss_ipv6_t    *netmask,
    OUT  u32            *prefix
)
{
    u32     i;
    u32     j;
    u32     f;

    if ( netmask == NULL ) {
        return FALSE;
    }

    if ( prefix == NULL ) {
        return FALSE;
    }

    f = 0;
    for ( i = 0; i < 16; ++i ) {
        if ( netmask->addr[i] == 0xFF ) {
            f += 8;
        } else {
            break;
        }
    }

    if ( i == 16 ) {
        *prefix = f;
        return TRUE;
    }

    switch ( netmask->addr[i] ) {
    case 0x00:
        break;
    case 0x80:
        f += 1;
        break;
    case 0xC0:
        f += 2;
        break;
    case 0xE0:
        f += 3;
        break;
    case 0xF0:
        f += 4;
        break;
    case 0xF8:
        f += 5;
        break;
    case 0xFC:
        f += 6;
        break;
    case 0xFE:
        f += 7;
        break;
    default:
        return FALSE;
    }

    for ( j = i + 1; j < 16; ++j ) {
        if ( netmask->addr[j] ) {
            return FALSE;
        }
    }
    *prefix = f;
    return TRUE;
}

/*
    IPv6 address: vtss_ipv6_t to string

    INPUT
        ipv6 : IPv6 address

    OUTPUT
        s : its buffer size MUST be larger than 40 bytes,
            output string in the format of hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh

    RETURN
        s    : successful
        NULL : failed
*/
char *vtss_icli_ipv6_to_str(
    IN  vtss_ipv6_t     ipv6,
    OUT char            *s
)
{
    i32     i;
    i32     start_0     = 0;
    i32     len_0       = 0;
    BOOL    continuous  = FALSE;
    i32     max_start_0 = 0;
    i32     max_len_0   = 0;
    char    *rs         = s;

    if ( s == NULL ) {
        return NULL;
    }

    // find the longest continuous 0
    for ( i = 0; i < 8; ++i ) {
        if ( ipv6.addr[i * 2] == 0 && ipv6.addr[i * 2 + 1] == 0 ) {
            if ( continuous ) {
                ++len_0;
            } else {
                start_0    = i;
                len_0      = 1;
                continuous = TRUE;
            }
        } else {
            if ( continuous ) {
                if ( len_0 > max_len_0 ) {
                    max_start_0 = start_0;
                    max_len_0   = len_0;
                }
                continuous = FALSE;
            }
        }
    }

    if ( continuous ) {
        if ( len_0 > max_len_0 ) {
            max_start_0 = start_0;
            max_len_0   = len_0;
        }
    }

    for ( i = 0; i < 8; ++i ) {
        if ( max_len_0 && i == max_start_0 ) {
            if ( i == 0 ) {
                icli_sprintf(s, ":");
                ++s;
            }
            icli_sprintf(s, ":");
            ++s;
            i += (max_len_0 - 1);
            continue;
        } else {
            if ( ipv6.addr[i * 2] ) {
                icli_sprintf(s, "%x%02x", ipv6.addr[i * 2], ipv6.addr[i * 2 + 1]);
            } else {
                icli_sprintf(s, "%x", ipv6.addr[i * 2 + 1]);
            }
            s += vtss_icli_str_len(s);
        }
        if ( i < 7 ) {
            icli_sprintf(s, ":");
            ++s;
        }
    }
    return rs;
}

/*
    MAC address: u8 [6] to string

    INPUT
        mac : MAC address

    OUTPUT
        s : its buffer size must be at least 18 bytes,
            output string in the format of xx-xx-xx-xx-xx-xx

    RETURN
        s    : successful
        NULL : failed
*/
char *vtss_icli_mac_to_str(
    IN  u8     *mac,
    OUT char    *s
)
{
    if ( mac == NULL ) {
        return NULL;
    }
    if ( s == NULL ) {
        return NULL;
    }
    icli_sprintf(s, "%02x:%02x:%02x:%02x:%02x:%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return s;
}

/*
    convert a string to an integer

    INPUT
        s : the string to be converted

    OUTPUT
        val : signed integer value

    RETURN
         0 : successful
        -1 : failed
*/
i32 vtss_icli_str_to_int(
    IN  const char  *s,
    OUT i32         *val
)
{
    //common
    const char  *c = s;
    //by type
    u32         i,
                d,
                comp;
    i32         j;
    BOOL        neg;

    i32         min = ICLI_MIN_INT;
    i32         max = ICLI_MAX_INT;

    if ( ICLI_IS_(EOS, *c) ) {
        return -1;
    }

    //check negative
    neg = ((*c) == '-');

#if 0 /* CP, 05/27/2013 09:49, CC-10169, 10170 */
    // for lint
    if ( max < 0 ) {
        if ( neg == FALSE ) {
            return -1;
        }
    }

    // for lint
    if ( min > 0 ) {
        if ( neg ) {
            return -1;
        }
    }
#endif

    if ( neg ) {
        if ((*(c + 1)) == 0) {
            return -1;
        }
        ++c;
    }

    if ( neg ) {
        comp = (u32)(0 - min);
    } else {
        comp = (u32)max;
    }

    i = 0;
    for ( ; ICLI_NOT_(EOS, *c); ++c ) {
        if ( ICLI_IS_DIGIT(*c) ) {
            //get value
            if ( i ) {
                d = comp / i;
                if ( d < 10 ) {
                    return -1;
                } else if ( d == 10 ) {
                    if ( ((*c) - '0') > (char)(comp - (i * 10)) ) {
                        return -1;
                    }
                }
            }
            i = (i * 10) + ((*c) - '0');
            if ( i > comp ) {
                return -1;
            }
        } else {
            return -1;
        }
    }

    if ( neg ) {
        j = 0 - i;
    } else {
        j = (i32)i;
    }

    *val = j;
    return 0;
}

/*
    find the number of the same prefix chars between 2 strings

    INPUT
        s1  : string 1
        s2  : string 2
        len : max length for check
              if 0 then no limit
        cs  : check in case-sensitive or not
              0 : case-insensitive
              1 : case-sensitive

    OUTPUT
        n/a

    RETURN
         >0 : the number of char's for the same prefix
         =0 : no same prefix
*/
u32 vtss_icli_str_prefix(
    IN  const char  *s1,
    IN  const char  *s2,
    IN  u32         len,
    IN  u32         cs
)
{
    i32     r;
    u32     i;

    if ( len == 0 ) {
        len = vtss_icli_str_len( s1 ) + 1;
    }

    for ( r = 0, i = 0; !r && *s1 && *s2 && i < len; ++s1, ++s2, ++i ) {
        r = *s1 - *s2;
        if ( r && cs == 0 ) {
            if ( 'a' <= (*s1) && (*s1) <= 'z' ) {
                if ( r == 'a' - 'A' ) {
                    r = 0;
                }
            } else if ( 'A' <= (*s1) && (*s1) <= 'Z' ) {
                if ( r == 'A' - 'a' ) {
                    r = 0;
                }
            }
        }
        if ( r ) {
            break;
        }
    }
    return i;
}
