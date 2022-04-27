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
//----------------------------------------------------------------------------
/**
 *  \file
 *      dhcp_server_web.c
 *
 *  \brief
 *      Web page implementation
 *
 *  \author
 *      CP Wang
 *
 *  \date
 *      05/09/2013 11:50
 */
//----------------------------------------------------------------------------
#include "web_api.h"
#include "dhcp_server_api.h"
#include "dhcp_server.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define _ID_STR_LEN      128

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
/**
 * \brief
 *      escape special characters.
 */
static void _all_escape(
    const char  *from,
    char        *to
)
{
    char    c;

    while ( *from ) {
        c = *from;
        if ( (c >= '0' && c <= '9') ||
             (c >= 'A' && c <= 'Z') ||
             (c >= 'a' && c <= 'z')  ) {
            *to = *from;
            from += 1;
            to   += 1;
        } else {
            sprintf(to, "%%%02X", *from);
            from += 1;
            to   += 3;
        }
    }
    *to = 0;
}

/**
 * \brief
 *      escape special characters.
 */
static void _ecos_escape(
    const char  *from,
    char        *to
)
{
    char    c;

    while ( *from ) {
        c = *from;
        if ( (c >= '0' && c <= '9') ||
             (c >= 'A' && c <= 'Z') ||
             (c >= 'a' && c <= 'z') ||
             (c == '@')             ||
             (c == '*')             ||
             (c == '_')             ||
             (c == '-')             ||
             (c == '.')              ) {
            *to = *from;
            from += 1;
            to   += 1;
        } else {
            sprintf(to, "%%%02X", *from);
            from += 1;
            to   += 3;
        }
    }
    *to = 0;
}

/**
 * \brief
 *      enable/disable VLAN mode.
 */
static void _vlan_update(
    IN CYG_HTTPD_STATE  *p
)
{
    char    *low_prefix  = "vlan_low_";
    char    *high_prefix = "vlan_high_";
    char    *mode_prefix = "vlan_mode_";
    char    id_str[_ID_STR_LEN + 1];
    int     i, j;
    int     low, high, mode;
    BOOL    b_low, b_high;

    for ( i = 1; i <= 8; i++ ) {
        /* get low */
        memset(id_str, 0, sizeof(id_str));
        sprintf(id_str, "%s%d", low_prefix, i);
        b_low = cyg_httpd_form_varable_int(p, id_str, &low);

        /* get high */
        memset(id_str, 0, sizeof(id_str));
        sprintf(id_str, "%s%d", high_prefix, i);
        b_high = cyg_httpd_form_varable_int(p, id_str, &high);

        if ( b_low == FALSE && b_high == FALSE ) {
            continue;
        }

        if ( b_low && b_high == FALSE ) {
            high = low;
        }

        if ( b_low == FALSE && b_high ) {
            low = high;
        }

        /* get mode */
        memset(id_str, 0, sizeof(id_str));
        sprintf(id_str, "%s%d", mode_prefix, i);
        if ( cyg_httpd_form_varable_int(p, id_str, &mode) == FALSE ) {
            continue;
        }

        /* add */
        for ( j = low; j <= high; j++ ) {
            (void)dhcp_server_vlan_enable_set((vtss_vid_t)j, (BOOL)mode);
        }
    }
}

/**
 * \brief
 *      add/delete excluded IP addresses.
 */
static void _excluded_update(
    IN CYG_HTTPD_STATE  *p
)
{
    dhcp_server_excluded_ip_t   excluded;
    char                        *delete_prefix = "excluded_delete_";
    char                        *low_prefix    = "excluded_low_";
    char                        *high_prefix   = "excluded_high_";
    char                        id_str[_ID_STR_LEN + 1];
    char                        low_str[20];
    char                        high_str[20];
    int                         i;
    BOOL                        b_low, b_high;

    /* DELETE */
    memset(&excluded, 0, sizeof(excluded));
    while ( dhcp_server_excluded_get_next(&excluded) == DHCP_SERVER_RC_OK ) {
        memset(id_str, 0, sizeof(id_str));
        sprintf(id_str, "%s%s_%s", delete_prefix, misc_ipv4_txt(excluded.low_ip, low_str), misc_ipv4_txt(excluded.high_ip, high_str));
        if ( cyg_httpd_form_varable_find(p, id_str) ) {
            (void)dhcp_server_excluded_delete( &excluded );
        }
    }

    /* ADD */
    for ( i = 1; i <= DHCP_SERVER_EXCLUDED_MAX_CNT; i++ ) {
        /* get low */
        memset(id_str, 0, sizeof(id_str));
        sprintf(id_str, "%s%d", low_prefix, i);
        b_low = cyg_httpd_form_varable_ipv4(p, id_str, &(excluded.low_ip));

        /* get high */
        memset(id_str, 0, sizeof(id_str));
        sprintf(id_str, "%s%d", high_prefix, i);
        b_high = cyg_httpd_form_varable_ipv4(p, id_str, &(excluded.high_ip));

        if ( b_low == FALSE && b_high == FALSE ) {
            continue;
        }

        if ( b_low && b_high == FALSE ) {
            excluded.high_ip = excluded.low_ip;
        }

        if ( b_low == FALSE && b_high ) {
            excluded.low_ip = excluded.high_ip;
        }

        /* add */
        (void)dhcp_server_excluded_add( &excluded );
    }
}

/**
 * \brief
 *      add/delete DHCP pools.
 */
static void _pool_update(
    IN CYG_HTTPD_STATE  *p
)
{
    dhcp_server_pool_t      pool;
    char                    *delete_prefix = "pool_delete_";
    char                    *add_prefix    = "pool_add_";
    char                    id_str[_ID_STR_LEN + 1];
    char                    encoded_string[3 * DHCP_SERVER_POOL_NAME_LEN + 1];
    u32                     i;
    const char              *pool_name;
    size_t                  len;

    /* DELETE */
    memset(&pool, 0, sizeof(pool));
    while ( dhcp_server_pool_get_next(&pool) == DHCP_SERVER_RC_OK ) {
        // escape pool name
        memset(encoded_string, 0, sizeof(encoded_string));
        _ecos_escape(pool.pool_name, encoded_string);

        // pack delete id
        memset(id_str, 0, sizeof(id_str));
        sprintf(id_str, "%s%s", delete_prefix, encoded_string);

        // find
        if ( cyg_httpd_form_varable_find(p, id_str) ) {
            (void)dhcp_server_pool_delete( &pool );
        }
    }

    /* ADD */
    for ( i = 1; i <= DHCP_SERVER_POOL_MAX_CNT; i++ ) {

        // pack id string
        memset(id_str, 0, sizeof(id_str));
        sprintf(id_str, "%s%d", add_prefix, i);

        // find id
        pool_name = cyg_httpd_form_varable_string(p, id_str, &len);
        if ( pool_name == NULL || len == 0 ) {
            // not found
            continue;
        }

        // make pool default values
        (void)dhcp_server_pool_default( &pool );

        // unescape pool name
        if ( cgi_unescape(pool_name, pool.pool_name, len, sizeof(pool.pool_name)) == FALSE ) {
            T_E("cgi_unescape( %s )\n", pool_name);
            continue;
        }

        // already existed?
        if ( dhcp_server_pool_get(&pool) == DHCP_SERVER_RC_OK ) {
            continue;
        }

        // create new pool
        if ( dhcp_server_pool_set(&pool) != DHCP_SERVER_RC_OK ) {
            T_E("dhcp_server_pool_set( %s )\n", pool.pool_name);
        }
    }
}

/**
 * \brief
 *      translate second to day string.
 */
static char *_day_str_get(
    IN  u32     second,
    OUT char    *str
)
{
    u32     minute;
    u32     hour;
    u32     day;

    day    = second / (24 * 60 * 60);
    hour   = ( second % (24 * 60 * 60) ) / (60 * 60);
    minute = ( second % (60 * 60) ) / 60;

    if ( day ) {
        sprintf(str, "%u days %u hours %u minutes", day, hour, minute);
    } else if ( hour ) {
        sprintf(str, "%u hours %u minutes", hour, minute);
    } else {
        sprintf(str, "%u minutes", minute);
    }
    return str;
}

#define __STRING_GET(_s_) \
{ \
    char    *__s_s__ = _s_; \
    if ( strlen(__s_s__) ) { \
        sn = snprintf( p->outbuffer, sizeof(p->outbuffer), "%s", (__s_s__) ); \
        (void)cyg_httpd_write_chunked( p->outbuffer, sn ); \
    } \
}

#define __IP_GET(_ip_) \
{ \
    char    *__s__ = misc_ipv4_txt(_ip_, tmp_buf); \
    __STRING_GET( __s__ ); \
}

#define __MAC_GET(_mac_) \
{ \
    char    *__s__ = misc_mac_txt(_mac_, tmp_buf); \
    __STRING_GET( __s__ ); \
}

#define __UINT_GET(_u_) \
    sn = snprintf( p->outbuffer, sizeof(p->outbuffer), "%u", (_u_) ); \
    (void)cyg_httpd_write_chunked( p->outbuffer, sn );

/**
 * \brief
 *      web handler for mode page.
 */
static cyg_int32 handler_config_dhcp_server_mode(
    IN CYG_HTTPD_STATE  *p
)
{
    int             i;
    BOOL            b_enable;
    int             sn;
    vtss_vid_t      vid;
    vtss_vid_t      first;
    vtss_vid_t      last;
    BOOL            b_first;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if ( web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_DHCP) ) {
        return -1;
    }
#endif

    if ( p->method == CYG_HTTPD_METHOD_POST ) {

        /* Global Mode */
        (void) cyg_httpd_form_varable_int(p, "global_mode", &i);
        if ( dhcp_server_enable_set((BOOL)i) != DHCP_SERVER_RC_OK ) {
            T_E("Fail to set global mode, %d\n", i);
        }

        /* VLAN Mode */
        _vlan_update( p );

        // refresh the page
        redirect(p, "/dhcp_server_mode.htm");

    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        //
        //  Format: <global_mode>|<vlan_list>
        //
        //      <global_mode>   : 0 - disabled, 1 - enabled
        //      <vlan_list>     : 1,3,5-9,1000
        //

        /* start */
        (void)cyg_httpd_start_chunked("html");

        /* Global Mode */
        //      <global_mode>   : 0 - disabled, 1 - enabled
        b_enable = FALSE;
        (void)dhcp_server_enable_get( &b_enable );
        __UINT_GET( b_enable );

        // separator
        __STRING_GET("|");

        /* VLAN Mode */
        //      <vlan_list>     : 1,3,5-9,1000
        first   = 0;
        last    = 0;
        b_first = TRUE;
        for ( vid = 1; vid < VTSS_VIDS; vid++ ) {
            if ( dhcp_server_vlan_enable_get(vid, &b_enable) == DHCP_SERVER_RC_OK ) {
                if ( b_enable ) {
                    if ( first ) {
                        if ( vid == last + 1 ) {
                            last = vid;
                        } else {
                            if ( b_first ) {
                                b_first = FALSE;
                            } else {
                                __STRING_GET(",");
                            }
                            __UINT_GET( first );
                            if ( last != first ) {
                                __STRING_GET("-");
                                __UINT_GET( last );
                            }

                            first   = vid;
                            last    = vid;
                        }
                    } else {
                        first   = vid;
                        last    = vid;
                    }
                }
            } else {
                T_E("%% Fail to get DHCP server enabled or disabled on VLAN %u.\n", vid);
            }
        }

        if ( first ) {
            __STRING_GET(",");
            __UINT_GET( first );
            if ( last != first ) {
                __STRING_GET("-");
                __UINT_GET( last );
            }
        }

        /* end */
        (void)cyg_httpd_end_chunked();
    }

    // Do not further search the file system.
    return -1;
}

/**
 * \brief
 *      web handler for excluded IP page.
 */
static cyg_int32 handler_config_dhcp_server_excluded(
    IN CYG_HTTPD_STATE  *p
)
{
    int                         sn;
    dhcp_server_excluded_ip_t   excluded;
    char                        tmp_buf[80 + 1];
    BOOL                        b_first;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if ( web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_DHCP) ) {
        return -1;
    }
#endif

    if ( p->method == CYG_HTTPD_METHOD_POST ) {

        /* Excluded IP Address */
        _excluded_update( p );

        // refresh the page
        redirect(p, "/dhcp_server_excluded.htm");

    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        //
        //  Format: <excluded_list>
        //
        //      <excluded_list> : 10.1.1.1,20.1.1.1-20.2.1.1,30.1.1.1
        //

        /* start */
        (void)cyg_httpd_start_chunked("html");

        /* Excluded IP Setting */
        //      <excluded_list> : 10.1.1.1,20.1.1.1-20.2.1.1,30.1.1.1
        b_first = TRUE;
        memset(&excluded, 0, sizeof(excluded));
        while ( dhcp_server_excluded_get_next(&excluded) == DHCP_SERVER_RC_OK ) {
            if ( b_first ) {
                b_first = FALSE;
            } else {
                __STRING_GET(",");
            }
            __IP_GET( excluded.low_ip );
            __STRING_GET("-");
            __IP_GET( excluded.high_ip );
        }

        /* end */
        (void)cyg_httpd_end_chunked();
    }

    // Do not further search the file system.
    return -1;
}

/**
 * \brief
 *      web handler for pool page.
 */
static cyg_int32 handler_config_dhcp_server_pool(CYG_HTTPD_STATE *p)
{
    int                         sn;
    char                        tmp_buf[80 + 1];
    BOOL                        b_first;
    dhcp_server_pool_t          pool;
    char                        encoded_string[3 * DHCP_SERVER_POOL_NAME_LEN + 1];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if ( web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_DHCP) ) {
        return -1;
    }
#endif

    if ( p->method == CYG_HTTPD_METHOD_POST ) {

        /* Pool Setting */
        _pool_update( p );

        // refresh the page
        redirect(p, "/dhcp_server_pool.htm");

    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        //
        //  Format: <pool_list>
        //
        //      <pool_list>     : <pool_entry>|<pool_entry>|...
        //      <pool_entry>    : <name>,type,ip,netmask,lease
        //

        /* start */
        (void)cyg_httpd_start_chunked("html");

        /* Pool Setting */
        //      <pool_list>     : <pool_entry>|<pool_entry>|...
        //      <pool_entry>    : <name>,type,ip,netmask,lease
        b_first = TRUE;
        memset(&pool, 0, sizeof(dhcp_server_pool_t));
        while ( dhcp_server_pool_get_next(&pool) == DHCP_SERVER_RC_OK ) {
            // encode name
            memset(encoded_string, 0, sizeof(encoded_string));
            _all_escape(pool.pool_name, encoded_string);

            if ( b_first ) {
                b_first = FALSE;
            } else {
                __STRING_GET("|");
            }
            __STRING_GET( encoded_string );
            __STRING_GET(",");

            switch ( pool.type ) {
            case DHCP_SERVER_POOL_TYPE_NONE:
            default:
                __STRING_GET("-");
                break;
            case DHCP_SERVER_POOL_TYPE_NETWORK:
                __STRING_GET("Network");
                break;
            case DHCP_SERVER_POOL_TYPE_HOST:
                __STRING_GET("Host");
                break;
            }
            __STRING_GET(",");

            if ( pool.type != DHCP_SERVER_POOL_TYPE_NONE ) {
                __IP_GET( pool.ip );
            } else {
                __STRING_GET("-");
            }
            __STRING_GET(",");

            if ( pool.type != DHCP_SERVER_POOL_TYPE_NONE ) {
                __IP_GET( pool.subnet_mask );
            } else {
                __STRING_GET("-");
            }
            __STRING_GET(",");

            if ( pool.lease ) {
                __STRING_GET( _day_str_get(pool.lease, tmp_buf) );
            } else {
                __STRING_GET("Infinite");
            }
        }

        /* end */
        (void)cyg_httpd_end_chunked();
    }

    // Do not further search the file system.
    return -1;
}

#define __STRING_POST(_id_, _f_) \
    len = 0; \
    var_string = cyg_httpd_form_varable_string( p, _id_, &len ); \
    if ( var_string && len ) { \
        if ( cgi_unescape(var_string, _f_, len, sizeof(_f_)) == FALSE ) { \
            T_E("cgi_unescape( %s )\n", _id_); \
        } \
    }

#define __SERVER_POST(_id_, _f_) \
    memset( ip, 0, sizeof(ip) ); \
    (void)cyg_httpd_form_varable_ipv4( p, _id_"_0", &ip[0] ); \
    (void)cyg_httpd_form_varable_ipv4( p, _id_"_1", &ip[1] ); \
    (void)cyg_httpd_form_varable_ipv4( p, _id_"_2", &ip[2] ); \
    (void)cyg_httpd_form_varable_ipv4( p, _id_"_3", &ip[3] ); \
    k = 0; \
    for ( i = 0; i < DHCP_SERVER_SERVER_MAX_CNT; i++ ) { \
        if ( ip[i] ) { \
            pool._f_[k++] = ip[i]; \
        } \
    }

#define __VENDOR_POST(_n_) \
    memset( class_id, 0, sizeof(class_id) ); \
    memset( specific_str, 0, sizeof(specific_str) ); \
    memset( specific_info, 0, sizeof(specific_info) ); \
    __STRING_POST( "pool_class_identifier_"#_n_, class_id ); \
    __STRING_POST( "pool_specific_info_"#_n_, specific_str ); \
    if ( strlen(class_id) && strlen(specific_str) ) { \
        if ( _hexval_get(specific_str, specific_info, &specific_info_len) ) { \
            j = k; \
            if ( k ) { \
                for ( j = 0; j < k; j++ ) { \
                    if ( strcmp(pool.class_info[j].class_id, class_id) == 0 ) { \
                        break; \
                    } \
                } \
                if ( j < k ) { \
                    pool.class_info[j].specific_info_len = specific_info_len; \
                    memcpy(pool.class_info[j].specific_info, specific_info, DHCP_SERVER_VENDOR_SPECIFIC_INFO_LEN); \
                } \
            } \
            if ( j == k ) { \
                strcpy(pool.class_info[k].class_id, class_id); \
                pool.class_info[k].specific_info_len = specific_info_len; \
                memcpy(pool.class_info[k].specific_info, specific_info, DHCP_SERVER_VENDOR_SPECIFIC_INFO_LEN); \
                k++; \
            } \
        } \
    }

#define __SERVER_GET(_f_) \
    b_first = TRUE; \
    for ( i = 0; i < DHCP_SERVER_SERVER_MAX_CNT; i++ ) { \
        if ( pool._f_[i] ) { \
            if ( b_first ) { \
                b_first = FALSE; \
            } else { \
                __STRING_GET("/"); \
            } \
            __IP_GET( pool._f_[i] ); \
        } \
    } \
    __STRING_GET(",");

/**
 * \brief
 *      translate character to integer value.
 */
static i32 _hex_get_c(
    IN  i8  c
)
{
    if ( c >= '0' && c <= '9' ) {
        return ( c - '0' );
    } else if ( (c >= 'A') && (c <= 'F') ) {
        return ( c - 'A' + 10 );
    } else if ( (c >= 'a') && (c <= 'f') ) {
        return ( c - 'a' + 10 );
    }
    return -1;
}

/**
 * \brief
 *      translate string to integer value.
 */
static BOOL _hexval_get(
    IN  char    *str,
    OUT u8      *hval,
    OUT u32     *len
)
{
    //common
    char            *c;
    //by type
    i32             i;
    u32             k;
    BOOL            b_first;
    BOOL            b_begin;

    c = str;
    if ( (*c) != '0' ) {
        return FALSE;
    }

    c++;
    if ( *c == 0 ) {
        return FALSE;
    } else if ( (*c) != 'x' && (*c) != 'X' ) {
        return FALSE;
    }

    c++;
    if ( *c == 0 ) {
        return FALSE;
    }

    b_begin = TRUE;
    b_first = TRUE;
    k = 0;
    for ( ; *c; c++ ) {
        i = _hex_get_c(*c);
        if ( i == -1 ) {
            return FALSE;
        }

        // get begin position of 1 happen
        if ( b_begin ) {
            if ( strlen(str) % 2 ) {
                b_first = FALSE;
            } else {
                b_first = TRUE;
            }
            b_begin = FALSE;
        }

        if ( b_first ) {
            i <<= 4;
            hval[ k ] = (u8)i;
            b_first = FALSE;
        } else {
            hval[ k ] += (u8)i;
            ( k )++;
            b_first = TRUE;
        }
    }

    (*len) = k;
    return TRUE;
}

/**
 * \brief
 *      translate integer value to string.
 */
static char *_hexval_str_get(
    IN  u8      *val,
    IN  u32     len,
    OUT char    *str
)
{
    u32     i;
    char    *s;

    if ( len == 0 ) {
        *str = 0;
        return str;
    }

    s = str;
    sprintf(s, "0x");
    s += 2;
    for ( i = 0; i < len; i++ ) {
        sprintf(s, "%02x", val[i]);
        s += 2;
    }
    *s = 0;

    return str;
}

/**
 * \brief
 *      web handler for pool configuration page.
 */
static cyg_int32 handler_config_dhcp_server_pool_config(CYG_HTTPD_STATE *p)
{
    const char          *var_string;
    size_t              len;
    dhcp_server_pool_t  pool;
    char                encoded_string[3 * DHCP_SERVER_DOMAIN_NAME_LEN + 1];
    int                 i, j, k;
    vtss_ipv4_t         ip[DHCP_SERVER_SERVER_MAX_CNT];
    char                class_id[DHCP_SERVER_VENDOR_CLASS_ID_LEN + 1];
    char                specific_str[2 + 2 * DHCP_SERVER_VENDOR_SPECIFIC_INFO_LEN + 1];
    u8                  specific_info[DHCP_SERVER_VENDOR_SPECIFIC_INFO_LEN];
    u32                 specific_info_len;
    char                tmp_buf[80 + 1];
    BOOL                b_first;
    int                 sn;
    char                empty_mac[DHCP_SERVER_MAC_LEN] = {0, 0, 0, 0, 0, 0};

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if ( web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_DHCP) ) {
        return -1;
    }
#endif

    if ( p->method == CYG_HTTPD_METHOD_POST ) {
        /* reset pool */
        memset( &pool, 0, sizeof(pool) );

        /* pool name */
        len = 0;
        var_string = cyg_httpd_form_varable_string( p, "pool_name", &len );
        if ( var_string == NULL || len == 0 ) {
            T_E("fail to get pool name\n");
            redirect(p, "/dhcp_server_pool.htm");
            return -1;
        }
        if ( cgi_unescape(var_string, pool.pool_name, len, sizeof(pool.pool_name)) == FALSE ) {
            T_E("cgi_unescape( %s )\n", var_string);
            redirect(p, "/dhcp_server_pool.htm");
            return -1;
        }

        /* IP */
        (void)cyg_httpd_form_varable_ipv4( p, "pool_ip", &(pool.ip) );

        /* Netmask */
        (void)cyg_httpd_form_varable_ipv4( p, "pool_netmask", &(pool.subnet_mask) );

        /* Type */
        (void)cyg_httpd_form_varable_int( p, "pool_type", (int *) & (pool.type) );

        /* Lease */
        i = j = k = 0;
        (void)cyg_httpd_form_varable_int( p, "pool_lease_days",    &i );
        (void)cyg_httpd_form_varable_int( p, "pool_lease_hours",   &j );
        (void)cyg_httpd_form_varable_int( p, "pool_lease_minutes", &k );
        pool.lease = i * 24 * 60 * 60 + j * 60 * 60 + k * 60;

        /* Domain name */
        __STRING_POST( "pool_domain_name", pool.domain_name );

        /* Subnet broadcast address */
        (void)cyg_httpd_form_varable_ipv4( p, "pool_broadcast_addr", &(pool.subnet_broadcast) );

        /* Default router */
        __SERVER_POST( "pool_default_router", default_router );

        /* DNS server */
        __SERVER_POST( "pool_dns_server", dns_server );

        /* NTP server */
        __SERVER_POST( "pool_ntp_server", ntp_server );

        /* Netbios type */
        (void)cyg_httpd_form_varable_int( p, "pool_netbios_node_type", (int *) & (pool.netbios_node_type) );

        /* Netbios scope */
        __STRING_POST( "pool_netbios_scope", pool.netbios_scope );

        /* Netbios name server */
        __SERVER_POST( "pool_netbios_name_server", netbios_name_server );

        /* NIS domain name */
        __STRING_POST( "pool_nis_domain_name", pool.nis_domain_name );

        /* NIS server */
        __SERVER_POST( "pool_nis_server", nis_server );

        /* Client identifier */
        (void)cyg_httpd_form_varable_int( p, "pool_client_id_type", (int *) & (pool.client_identifier.type) );
        switch ( pool.client_identifier.type ) {
        case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE:
        default:
            break;

        case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_FQDN:
            __STRING_POST( "pool_client_id_value", pool.client_identifier.u.fqdn );
            break;

        case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_MAC:
            (void)cyg_httpd_form_variable_mac( p, "pool_client_id_value", &(pool.client_identifier.u.mac) );
            break;
        }

        /* Hardware address */
        (void)cyg_httpd_form_variable_mac( p, "pool_hardware_address", &(pool.client_haddr) );

        /* Client name */
        __STRING_POST( "pool_client_name", pool.client_name );

        /* Vendor specific information */
        k = 0;

        __VENDOR_POST( 0 );
        __VENDOR_POST( 1 );
        __VENDOR_POST( 2 );
        __VENDOR_POST( 3 );
        __VENDOR_POST( 4 );
        __VENDOR_POST( 5 );
        __VENDOR_POST( 6 );
        __VENDOR_POST( 7 );

        // for lint
        if ( k ) {}

        /* save */
        if ( dhcp_server_pool_set(&pool) != DHCP_SERVER_RC_OK ) {
            T_E("dhcp_server_pool_set( %s )\n", pool.pool_name);
        }

        // refresh the page
        redirect(p, "/dhcp_server_pool.htm");

    } else { /* CYG_HTTPD_METHOD_GET (+HEAD) */
        //
        // Format: <pool_name_list>|<pool_config>
        //   <pool_name_list> : name1,name2,...
        //   <pool_config>    : name,type,ip,netmask,lease,domain-name,broadcast-address,<default_router_list>,<dns_server_list>,<ntp_server_list>,netbios-node-type,netbios-scope,<netbios_name_server_list>,nis-domain-name,<nis_server_list>,client-identifier,hardware-addr,client-name,<class-identifier-specific-info-list>
        //

        /* Start */
        (void)cyg_httpd_start_chunked("html");

        // get pool name for GET
        len = 0;
        var_string = cyg_httpd_form_varable_string(p, "pool_name", &len);
        if ( var_string == NULL || len == 0 ) {
            /* add new pool */
            __STRING_GET("|");
            (void)cyg_httpd_end_chunked();
            return -1;
        }

        /* <pool_name_list> */
        b_first = TRUE;
        memset(&pool, 0, sizeof(dhcp_server_pool_t));
        while ( dhcp_server_pool_get_next(&pool) == DHCP_SERVER_RC_OK ) {
            // encode name
            memset(encoded_string, 0, sizeof(encoded_string));
            _all_escape(pool.pool_name, encoded_string);

            if ( b_first ) {
                b_first = FALSE;
            } else {
                __STRING_GET(",");
            }
            __STRING_GET( encoded_string );
        }

        // separator
        __STRING_GET("|");

        /* <pool_config> */
        memset( &pool, 0, sizeof(pool) );
        if ( cgi_unescape(var_string, pool.pool_name, len, sizeof(pool.pool_name)) == FALSE ) {
            T_E("cgi_unescape( %s )\n", var_string);
            (void)cyg_httpd_end_chunked();
            return -1;
        }

        if ( dhcp_server_pool_get(&pool) != DHCP_SERVER_RC_OK ) {
            T_E("fail to get pool = %s\n", pool.pool_name);
            (void)cyg_httpd_end_chunked();
            return -1;
        }

        // name
        memset(encoded_string, 0, sizeof(encoded_string));
        _all_escape(pool.pool_name, encoded_string);
        __STRING_GET( encoded_string );
        __STRING_GET(",");

        // type
        __UINT_GET( pool.type );
        __STRING_GET(",");

        // ip
        if ( pool.type != DHCP_SERVER_POOL_TYPE_NONE ) {
            __IP_GET( pool.ip );
        }
        __STRING_GET(",");

        // subnet netmask
        if ( pool.type != DHCP_SERVER_POOL_TYPE_NONE ) {
            __IP_GET( pool.subnet_mask );
        }
        __STRING_GET(",");

        // lease
        __UINT_GET( pool.lease / (24 * 60 * 60) );
        __STRING_GET("/");
        __UINT_GET( ( pool.lease % (24 * 60 * 60) ) / (60 * 60) );
        __STRING_GET("/");
        __UINT_GET( ( pool.lease % (60 * 60) ) / 60 );
        __STRING_GET(",");

        // domain name
        __STRING_GET( pool.domain_name );
        __STRING_GET(",");

        // broadcast address
        if ( pool.subnet_broadcast ) {
            __IP_GET( pool.subnet_broadcast );
        }
        __STRING_GET(",");

        // default_router_list
        __SERVER_GET( default_router );

        // dns_server_list
        __SERVER_GET( dns_server );

        // ntp_server_list
        __SERVER_GET( ntp_server );

        // netbios-node-type
        __UINT_GET( pool.netbios_node_type );
        __STRING_GET(",");

        // netbios-scope
        __STRING_GET( pool.netbios_scope );
        __STRING_GET(",");

        // netbios_name_server_list
        __SERVER_GET( netbios_name_server );

        // nis-domain-name
        __STRING_GET( pool.nis_domain_name );
        __STRING_GET(",");

        // nis_server_list
        __SERVER_GET( nis_server );

        // client-identifier
        __UINT_GET( pool.client_identifier.type );
        __STRING_GET("/");

        switch ( pool.client_identifier.type ) {
        case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE:
        default:
            break;

        case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_FQDN:
            __STRING_GET( pool.client_identifier.u.fqdn );
            break;

        case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_MAC:
            if ( memcmp(pool.client_identifier.u.mac.addr, empty_mac, DHCP_SERVER_MAC_LEN) ) {
                __MAC_GET( pool.client_identifier.u.mac.addr );
            }
            break;
        }
        __STRING_GET(",");

        // hardware-addr
        if ( memcmp(pool.client_haddr.addr, empty_mac, DHCP_SERVER_MAC_LEN) ) {
            __MAC_GET( pool.client_haddr.addr );
        }
        __STRING_GET(",");

        // client-name
        __STRING_GET( pool.client_name );
        __STRING_GET(",");

        // vendor class-identifier-specific-info-list
        b_first = TRUE;
        for ( i = 0; i < DHCP_SERVER_VENDOR_CLASS_INFO_CNT; i++ ) {
            if ( b_first ) {
                b_first = FALSE;
            } else {
                __STRING_GET(",");
            }
            if ( pool.class_info[i].class_id[0] ) {
                __STRING_GET( pool.class_info[i].class_id );
                __STRING_GET("/");
                __STRING_GET( _hexval_str_get(pool.class_info[i].specific_info, pool.class_info[i].specific_info_len, tmp_buf) );
            }
        }

        /* End */
        (void)cyg_httpd_end_chunked();
    }

    // Do not further search the file system.
    return -1;
}

/**
 * \brief
 *      web handler for statistics page.
 */
static cyg_int32 handler_stat_dhcp_server(CYG_HTTPD_STATE *p)
{
    dhcp_server_statistics_t    stats;
    int                         sn;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if ( web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_DHCP) ) {
        return -1;
    }
#endif

    /* Clear */
    if ( cyg_httpd_form_varable_find(p, "clear") != NULL) {
        dhcp_server_statistics_clear();
    }

    /* start HTML */
    (void)cyg_httpd_start_chunked("html");

    if ( dhcp_server_statistics_get(&stats) != DHCP_SERVER_RC_OK ) {
        T_E("dhcp_server_statistics_get()\n");
        (void)cyg_httpd_end_chunked();
        return -1;
    }

    /* database */
    __UINT_GET( stats.pool_cnt );
    __STRING_GET(",");

    __UINT_GET( stats.excluded_cnt );
    __STRING_GET(",");

    __UINT_GET( stats.declined_cnt );
    __STRING_GET("|");

    __UINT_GET( stats.automatic_binding_cnt );
    __STRING_GET(",");

    __UINT_GET( stats.manual_binding_cnt );
    __STRING_GET(",");

    __UINT_GET( stats.expired_binding_cnt );
    __STRING_GET("|");

    /* message recv */
    __UINT_GET( stats.discover_cnt );
    __STRING_GET(",");

    __UINT_GET( stats.request_cnt );
    __STRING_GET(",");

    __UINT_GET( stats.decline_cnt );
    __STRING_GET(",");

    __UINT_GET( stats.release_cnt );
    __STRING_GET(",");

    __UINT_GET( stats.inform_cnt );
    __STRING_GET("|");

    /* message sent */
    __UINT_GET( stats.offer_cnt );
    __STRING_GET(",");

    __UINT_GET( stats.ack_cnt );
    __STRING_GET(",");

    __UINT_GET( stats.nak_cnt );
    __STRING_GET("|");

    /* end HTML */
    (void)cyg_httpd_end_chunked();

    // Do not further search the file system.
    return -1;
}

/**
 * \brief
 *      web handler for statistics page.
 */
static cyg_int32 handler_stat_dhcp_server_binding(CYG_HTTPD_STATE *p)
{
    dhcp_server_binding_t   binding;
    BOOL                    b_first;
    int                     sn;
    char                    tmp_buf[80 + 1];
    char                    clear_str[16];
    u32                     i;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if ( web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_DHCP) ) {
        return -1;
    }
#endif

    /* Clear */
    if ( cyg_httpd_form_varable_find(p, "clear_automatic") ) {
        dhcp_server_binding_clear_by_type( DHCP_SERVER_BINDING_TYPE_AUTOMATIC );
    } else if ( cyg_httpd_form_varable_find(p, "clear_manual") ) {
        dhcp_server_binding_clear_by_type( DHCP_SERVER_BINDING_TYPE_MANUAL );
    } else if ( cyg_httpd_form_varable_find(p, "clear_expired") ) {
        dhcp_server_binding_clear_by_type( DHCP_SERVER_BINDING_TYPE_EXPIRED );
    } else if ( cyg_httpd_form_varable_find(p, "clear_0") ) {
        for ( i = 0; i < DHCP_SERVER_BINDING_MAX_CNT; i++ ) {
            sprintf(clear_str, "clear_%u", i);
            if ( cyg_httpd_form_varable_ipv4(p, clear_str, &(binding.ip)) ) {
                (void)dhcp_server_binding_delete( &binding );
            }
        }
    }

    //
    //  Format: <binding IP database>|<binding IP database>|...
    //
    //      <binding IP database> = IP, type, state, pool name, server ID
    //

    /* start HTML */
    (void)cyg_httpd_start_chunked("html");

    b_first = TRUE;
    memset(&binding, 0, sizeof(dhcp_server_binding_t));
    while ( dhcp_server_binding_get_next(&binding) == DHCP_SERVER_RC_OK ) {
        if ( b_first ) {
            b_first = FALSE;
        } else {
            __STRING_GET("|");
        }

        // IP
        __IP_GET( binding.ip );
        __STRING_GET(",");

        // type
        switch ( binding.type ) {
        case DHCP_SERVER_BINDING_TYPE_AUTOMATIC:
            __STRING_GET("Automatic");
            break;
        case DHCP_SERVER_BINDING_TYPE_MANUAL:
            __STRING_GET("Manual");
            break;
        case DHCP_SERVER_BINDING_TYPE_EXPIRED:
            __STRING_GET("Expired");
            break;
        default:
            __STRING_GET("-");
            break;
        }
        __STRING_GET(",");

        // state
        switch ( binding.state ) {
        case DHCP_SERVER_BINDING_STATE_ALLOCATED:
            __STRING_GET("Allocated");
            break;
        case DHCP_SERVER_BINDING_STATE_COMMITTED:
            __STRING_GET("Committed");
            break;
        case DHCP_SERVER_BINDING_STATE_EXPIRED:
            __STRING_GET("Expired");
            break;
        case DHCP_SERVER_BINDING_STATE_NONE:
        default:
            __STRING_GET("-");
            break;
        }
        __STRING_GET(",");

        // pool name
        __STRING_GET( binding.pool_name );
        __STRING_GET(",");

        // server id
        __IP_GET( binding.server_id );
    }

    /* end HTML */
    (void)cyg_httpd_end_chunked();

    // Do not further search the file system.
    return -1;
}

static char *_expired_time_get(
    IN  u32     second,
    OUT char    *str
)
{
    u32     minute;
    u32     hour;
    u32     day;

    day    = second / (24 * 60 * 60);
    hour   = ( second % (24 * 60 * 60) ) / (60 * 60);
    minute = ( second % (60 * 60) ) / 60;
    second = second % 60;

    if ( day ) {
        sprintf(str, "%u days %u hours %u minutes %u seconds", day, hour, minute, second);
    } else if ( hour ) {
        sprintf(str, "%u hours %u minutes %u seconds", hour, minute, second);
    } else if ( minute ) {
        sprintf(str, "%u minutes %u seconds", minute, second);
    } else {
        sprintf(str, "%u seconds", second);
    }
    return str;
}

/**
 * \brief
 *      web handler for statistics page.
 */
static cyg_int32 handler_stat_dhcp_server_binding_data(CYG_HTTPD_STATE *p)
{
    dhcp_server_binding_t   binding;
    BOOL                    b_first;
    int                     sn;
    char                    tmp_buf[80 + 1];
    vtss_ipv4_t             binding_ip;
    char                    encoded_string[3 * DHCP_SERVER_CLIENT_IDENTIFIER_LEN + 1];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if ( web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_DHCP) ) {
        return -1;
    }
#endif

    /* start HTML */
    (void)cyg_httpd_start_chunked("html");

    // get binding IP for GET
    if ( cyg_httpd_form_varable_ipv4(p, "binding_ip", &binding_ip) == FALSE ) {
        (void)cyg_httpd_end_chunked();
        return -1;
    }

    // check if binding IP exists or not
    memset(&binding, 0, sizeof(dhcp_server_binding_t));
    binding.ip = binding_ip;
    if ( dhcp_server_binding_get(&binding) != DHCP_SERVER_RC_OK ) {
        (void)cyg_httpd_end_chunked();
        return -1;
    }

    //
    //  Format: <binding ip list>|<binding data>
    //
    //      <binding ip list> : ip1,ip2,...
    //      <binding data>    : ip, type, state, pool name, server id,
    //                          vid, subnet mask, client id type,
    //                          client id value, hardware addr, lease,
    //                          expire time
    //

    // <binding ip list>
    b_first = TRUE;
    memset(&binding, 0, sizeof(dhcp_server_binding_t));
    while ( dhcp_server_binding_get_next(&binding) == DHCP_SERVER_RC_OK ) {
        if ( b_first ) {
            b_first = FALSE;
        } else {
            __STRING_GET(",");
        }
        __IP_GET( binding.ip );
    }

    if ( b_first ) {
        (void)cyg_httpd_end_chunked();
        return -1;
    }

    __STRING_GET("|");

    // <binding data>
    memset(&binding, 0, sizeof(dhcp_server_binding_t));
    binding.ip = binding_ip;
    if ( dhcp_server_binding_get(&binding) != DHCP_SERVER_RC_OK ) {
        T_E("fail to get binding = %08x\n", binding_ip);
        (void)cyg_httpd_end_chunked();
        return -1;
    }

    // IP
    __IP_GET( binding.ip );
    __STRING_GET(",");

    // type
    switch ( binding.type ) {
    case DHCP_SERVER_BINDING_TYPE_AUTOMATIC:
        __STRING_GET("Automatic");
        break;
    case DHCP_SERVER_BINDING_TYPE_MANUAL:
        __STRING_GET("Manual");
        break;
    case DHCP_SERVER_BINDING_TYPE_EXPIRED:
        __STRING_GET("Expired");
        break;
    default:
        __STRING_GET("-");
        break;
    }
    __STRING_GET(",");

    // state
    switch ( binding.state ) {
    case DHCP_SERVER_BINDING_STATE_ALLOCATED:
        __STRING_GET("Allocated");
        break;
    case DHCP_SERVER_BINDING_STATE_COMMITTED:
        __STRING_GET("Committed");
        break;
    case DHCP_SERVER_BINDING_STATE_EXPIRED:
        __STRING_GET("Expired");
        break;
    case DHCP_SERVER_BINDING_STATE_NONE:
    default:
        __STRING_GET("-");
        break;
    }
    __STRING_GET(",");

    // pool name
    __STRING_GET( binding.pool_name );
    __STRING_GET(",");

    // server id
    __IP_GET( binding.server_id );
    __STRING_GET(",");

    // vid
    __UINT_GET( binding.vid );
    __STRING_GET(",");

    // Subnet mask
    __IP_GET( binding.subnet_mask );
    __STRING_GET(",");

    // client id
    switch ( binding.identifier.type ) {
    case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE:
    default:
        __STRING_GET("-");
        __STRING_GET(",");
        __STRING_GET("-");
        break;
    case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_FQDN:
        __STRING_GET("FQDN");
        __STRING_GET(",");

        // encode FQDN to avoid ','
        memset(encoded_string, 0, sizeof(encoded_string));
        _all_escape(binding.identifier.u.fqdn, encoded_string);
        __STRING_GET(encoded_string);
        break;
    case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_MAC:
        __STRING_GET("MAC");
        __STRING_GET(",");
        __MAC_GET(binding.identifier.u.mac.addr);
        break;
    }
    __STRING_GET(",");

    // hardware address
    __MAC_GET( binding.chaddr.addr );
    __STRING_GET(",");

    // lease
    if ( binding.lease ) {
        switch ( binding.state ) {
        case DHCP_SERVER_BINDING_STATE_COMMITTED:
        case DHCP_SERVER_BINDING_STATE_ALLOCATED:
            __STRING_GET( _expired_time_get(binding.lease, tmp_buf) );
            break;

        default:
            __STRING_GET("-");
            break;
        }
    } else {
        __STRING_GET("Infinite");
    }
    __STRING_GET(",");

    // expired in
    if ( binding.lease ) {
        switch ( binding.state ) {
        case DHCP_SERVER_BINDING_STATE_COMMITTED:
        case DHCP_SERVER_BINDING_STATE_ALLOCATED:
            __STRING_GET( _expired_time_get(binding.expire_time - dhcp_server_current_time_get(), tmp_buf) );
            break;

        default:
            __STRING_GET("-");
            break;
        }
    } else {
        __STRING_GET("-");
    }

    /* end HTML */
    (void)cyg_httpd_end_chunked();

    // Do not further search the file system.
    return -1;
}

/**
 * \brief
 *      web handler for statistics page.
 */
static cyg_int32 handler_stat_dhcp_server_declined(CYG_HTTPD_STATE *p)
{
    int                         sn;
    vtss_ipv4_t                 declined_ip;
    BOOL                        b_first;
    char                        tmp_buf[80 + 1];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if ( web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_DHCP) ) {
        return -1;
    }
#endif

    /* start HTML */
    (void)cyg_httpd_start_chunked("html");

    b_first = TRUE;
    declined_ip = 0;
    while ( dhcp_server_declined_get_next(&declined_ip) == DHCP_SERVER_RC_OK ) {
        if ( b_first ) {
            b_first = FALSE;
        } else {
            __STRING_GET("|");
        }
        __IP_GET( declined_ip );
    }

    /* end HTML */
    (void)cyg_httpd_end_chunked();

    // Do not further search the file system.
    return -1;
}

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

/* Configuration */
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_dhcp_server_mode,        "/config/dhcp_server_mode",        handler_config_dhcp_server_mode);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_dhcp_server_excluded,    "/config/dhcp_server_excluded",    handler_config_dhcp_server_excluded);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_dhcp_server_pool,        "/config/dhcp_server_pool",        handler_config_dhcp_server_pool);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_dhcp_server_pool_config, "/config/dhcp_server_pool_config", handler_config_dhcp_server_pool_config);

/* Monitor */
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_dhcp_server,               "/stat/dhcp_server",               handler_stat_dhcp_server);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_dhcp_server_binding,       "/stat/dhcp_server_binding",       handler_stat_dhcp_server_binding);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_dhcp_server_binding_data,  "/stat/dhcp_server_binding_data",  handler_stat_dhcp_server_binding_data);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_dhcp_server_declined,      "/stat/dhcp_server_declined",      handler_stat_dhcp_server_declined);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
