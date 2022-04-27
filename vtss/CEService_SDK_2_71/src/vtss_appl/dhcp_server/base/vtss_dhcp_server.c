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
//----------------------------------------------------------------------------
/**
 *  \file
 *      vtss_dhcp_server.c
 *
 *  \brief
 *      DHCP server engine
 *
 *  \author
 *      CP Wang
 *
 *  \date
 *      05/09/2013 11:45
 */
//----------------------------------------------------------------------------
/*
==============================================================================

    Include File

==============================================================================
*/
#include "vtss_dhcp_server_type.h"
#include "vtss_dhcp_server.h"
#include "vtss_dhcp_server_message.h"
#include "vtss_dhcp_server_platform.h"
#include "vtss_avl_tree_api.h"
#include "vtss_free_list_api.h"

/*
==============================================================================

    Constant

==============================================================================
*/
#define _PARAMETER_LIST_MAX_LEN     128 /**< max length of option 55 */
#define _HTYPE_ETHERNET             1

/*
==============================================================================

    Macro

==============================================================================
*/
#define STATISTICS_INC(_f_)         ++( g_statistics._f_ )
#define STATISTICS_DEC(_f_)         --( g_statistics._f_ );

/* bit array macro's for VLAN mode */
#define _VLAN_BF_SIZE               VTSS_BF_SIZE( VTSS_VIDS )
#define _VLAN_BF_GET(vid)           VTSS_BF_GET( g_vlan_bit, vid )
#define _VLAN_BF_SET(vid, val)      VTSS_BF_SET( g_vlan_bit, vid, val )
#define _VLAN_BF_CLR_ALL()          VTSS_BF_CLR( g_vlan_bit, VTSS_VIDS )

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
static BOOL                         g_enable;   /**< global mode */
static dhcp_server_statistics_t     g_statistics; /**< statistics database */
static u8                           g_vlan_bit[ _VLAN_BF_SIZE ]; /**< VLAN mode database */

/* use u32 to make sure the 4-byte alignment */
static u32                          _send_message_buf[ DHCP_SERVER_MESSAGE_MAX_LEN / 4 ]; /**< message buffer for sending */
static u8                           *g_message_buf = (u8 *)_send_message_buf;

/*
==============================================================================

    Compare Function

==============================================================================
*/
/**
 *  \brief
 *      compare IP address.
 */
static i32 _u32_cmp(
    IN u32      a,
    IN u32      b
)
{
    if ( a > b ) {
        return VTSS_AVL_TREE_CMP_RESULT_A_LARGER;
    } else if ( a < b ) {
        return VTSS_AVL_TREE_CMP_RESULT_A_SMALLER;
    }

    /* all equal */
    return VTSS_AVL_TREE_CMP_RESULT_A_B_SAME;
}

/**
 *  \brief
 *      compare name string.
 */
static i32 _str_cmp(
    IN char     *a,
    IN char     *b
)
{
    int     r;

    if ( a == NULL && b == NULL ) {
        return VTSS_AVL_TREE_CMP_RESULT_A_B_SAME;
    }

    if ( a == NULL ) {
        return VTSS_AVL_TREE_CMP_RESULT_A_SMALLER;
    }

    if ( b == NULL ) {
        return VTSS_AVL_TREE_CMP_RESULT_A_LARGER;
    }

    r = strcmp(a, b);
    if ( r < 0 ) {
        return VTSS_AVL_TREE_CMP_RESULT_A_SMALLER;
    } else if ( r == 0 ) {
        return VTSS_AVL_TREE_CMP_RESULT_A_B_SAME;
    } else { // r > 0
        return VTSS_AVL_TREE_CMP_RESULT_A_LARGER;
    }
}

/**
 *  \brief
 *      compare MAC address.
 */
static i32 _mac_cmp(
    IN vtss_mac_t   *a,
    IN vtss_mac_t   *b
)
{
    int     r;

    r = memcmp(a->addr, b->addr, DHCP_SERVER_MAC_LEN);
    if ( r > 0 ) {
        return VTSS_AVL_TREE_CMP_RESULT_A_LARGER;
    } else if ( r < 0 ) {
        return VTSS_AVL_TREE_CMP_RESULT_A_SMALLER;
    }

    /* all equal */
    return VTSS_AVL_TREE_CMP_RESULT_A_B_SAME;
}

/**
 *  \brief
 *      compare client identifier.
 */
static i32 _client_identifier_cmp(
    IN  dhcp_server_client_identifier_t     *a,
    IN  dhcp_server_client_identifier_t     *b
)
{
    int     r;

    /* compare type */
    r = _u32_cmp(a->type, b->type);
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    switch ( a->type ) {
    case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE:
        break;

    case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_FQDN:
        /* compare FQDN */
        r = _str_cmp(a->u.fqdn, b->u.fqdn);
        break;

    case DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_MAC:
        /* compare MAC */
        r = _mac_cmp( &(a->u.mac), &(b->u.mac) );
        break;
    }

    return r;
}

/**
 *  \brief
 *      index: low_ip, high_ip.
 */
static i32 _excluded_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    dhcp_server_excluded_ip_t   *a;
    dhcp_server_excluded_ip_t   *b;
    i32                         r;

    a = (dhcp_server_excluded_ip_t *)data_a;
    b = (dhcp_server_excluded_ip_t *)data_b;

    r = _u32_cmp(a->low_ip, b->low_ip);
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    r = _u32_cmp(a->high_ip, b->high_ip);
    return r;
}

/**
 *  \brief
 *      index: pool_name.
 */
static i32 _pool_name_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    dhcp_server_pool_t      *a;
    dhcp_server_pool_t      *b;
    i32                     r;

    a = (dhcp_server_pool_t *)data_a;
    b = (dhcp_server_pool_t *)data_b;

    r = _str_cmp(a->pool_name, b->pool_name);
    return r;
}

/**
 *  \brief
 *      index: subnet_mask, ip & subnet_mask.
 */
static i32 _pool_ip_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    dhcp_server_pool_t      *a;
    dhcp_server_pool_t      *b;
    i32                     r;

    a = (dhcp_server_pool_t *)data_a;
    b = (dhcp_server_pool_t *)data_b;

    r = _u32_cmp(a->subnet_mask, b->subnet_mask);
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    r = _u32_cmp(a->ip & a->subnet_mask, b->ip & b->subnet_mask);
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    r = _u32_cmp(a->type, b->type);
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    /* if both pools are type of network, then they are the same */
    if ( a->type == DHCP_SERVER_POOL_TYPE_NETWORK ) {
        return r;
    }

    /* if both pools are type of host, then compare ip */
    r = _u32_cmp(a->ip, b->ip);
    return r;
}

/**
 *  \brief
 *      index: client identifier, subnet_mask, ip & subnet_mask
 */
static i32 _pool_id_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    dhcp_server_pool_t      *a;
    dhcp_server_pool_t      *b;
    i32                     r;

    a = (dhcp_server_pool_t *)data_a;
    b = (dhcp_server_pool_t *)data_b;

    /* compare client identifier */
    r = _client_identifier_cmp( &(a->client_identifier), &(b->client_identifier) );
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    /* compare subnet mask, longest match first */
    r = _u32_cmp(a->subnet_mask, b->subnet_mask);
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    /* compare subnet */
    r = _u32_cmp(a->ip & a->subnet_mask, b->ip & b->subnet_mask);
    return r;
}

/**
 *  \brief
 *      index: chaddr, subnet_mask, ip & subnet_mask
 */
static i32 _pool_chaddr_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    dhcp_server_pool_t      *a;
    dhcp_server_pool_t      *b;
    i32                     r;

    a = (dhcp_server_pool_t *)data_a;
    b = (dhcp_server_pool_t *)data_b;

    /* compare chaddr */
    r = _mac_cmp( &(a->client_haddr), &(b->client_haddr) );
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    /* compare subnet mask, longest match first */
    r = _u32_cmp(a->subnet_mask, b->subnet_mask);
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    /* compare subnet */
    r = _u32_cmp(a->ip & a->subnet_mask, b->ip & b->subnet_mask);
    return r;
}

/**
 *  \brief
 *      index: ip.
 */
static i32 _binding_ip_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    dhcp_server_binding_t   *a;
    dhcp_server_binding_t   *b;
    i32                     r;

    a = (dhcp_server_binding_t *)data_a;
    b = (dhcp_server_binding_t *)data_b;

    r = _u32_cmp(a->ip, b->ip);
    return r;
}

/**
 *  \brief
 *      index: identifier, ip & subnet_mask
 */
static i32 _binding_id_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    dhcp_server_binding_t   *a;
    dhcp_server_binding_t   *b;
    i32                     r;

    a = (dhcp_server_binding_t *)data_a;
    b = (dhcp_server_binding_t *)data_b;

    /* compare client identifier */
    r = _client_identifier_cmp( &(a->identifier), &(b->identifier) );
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    /* compare IP & subnet mask */
    r = _u32_cmp(a->ip & a->subnet_mask, b->ip & b->subnet_mask);
    return r;
}

/**
 *  \brief
 *      index: chaddr, ip & subnet_mask
 */
static i32 _binding_chaddr_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    dhcp_server_binding_t   *a;
    dhcp_server_binding_t   *b;
    i32                     r;

    a = (dhcp_server_binding_t *)data_a;
    b = (dhcp_server_binding_t *)data_b;

    /* compare chaddr */
    r = _mac_cmp( &(a->chaddr), &(b->chaddr) );
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    /* compare IP & subnet mask */
    r = _u32_cmp(a->ip & a->subnet_mask, b->ip & b->subnet_mask);
    return r;
}

/**
 *  \brief
 *      index: pool_name, ip.
 */
static i32 _binding_name_ip_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    dhcp_server_binding_t   *a;
    dhcp_server_binding_t   *b;
    i32                     r;

    a = (dhcp_server_binding_t *)data_a;
    b = (dhcp_server_binding_t *)data_b;

    /* compare pool_name */
    r = _str_cmp(a->pool_name, b->pool_name);
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    /* compare IP */
    r = _u32_cmp(a->ip, b->ip);
    return r;
}

/**
 *  \brief
 *      index: expire_time, ip
 */
static i32 _binding_time_ip_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    dhcp_server_binding_t   *a;
    dhcp_server_binding_t   *b;
    i32                     r;

    a = (dhcp_server_binding_t *)data_a;
    b = (dhcp_server_binding_t *)data_b;

    /* compare expire_time */
    r = _u32_cmp(a->expire_time, b->expire_time);
    if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
        return r;
    }

    /* compare IP */
    r = _u32_cmp(a->ip, b->ip);
    return r;
}

/**
 *  \brief
 *      index: ip.
 */
static i32 _decline_ip_cmp_func(
    IN  void    *data_a,
    IN  void    *data_b
)
{
    vtss_ipv4_t     *a;
    vtss_ipv4_t     *b;
    i32             r;

    a = (vtss_ipv4_t *)data_a;
    b = (vtss_ipv4_t *)data_b;

    r = _u32_cmp(*a, *b);
    return r;
}

/*
==============================================================================

    AVL tree and Free list

==============================================================================
*/
/*
    Excluded IP list
*/
// Free list
VTSS_FREE_LIST(g_excluded_ip_flist, dhcp_server_excluded_ip_t, DHCP_SERVER_EXCLUDED_MAX_CNT)
// AVL tree, index : low_ip, high_ip
VTSS_AVL_TREE(g_excluded_ip_avlt, "EXCLUDED_IP", 0, _excluded_cmp_func, DHCP_SERVER_EXCLUDED_MAX_CNT)

/*
    Pool list
*/
// Free list
VTSS_FREE_LIST(g_pool_flist, dhcp_server_pool_t, DHCP_SERVER_POOL_MAX_CNT)
// AVL tree for both of network and host
// index: pool_name
VTSS_AVL_TREE(g_pool_name_avlt, "POOL_NAME", 0, _pool_name_cmp_func, DHCP_SERVER_POOL_MAX_CNT)
// AVL tree for network
// index: subnet_mask, ip & subnet_mask
VTSS_AVL_TREE(g_pool_ip_avlt, "POOL_IP", 0, _pool_ip_cmp_func, DHCP_SERVER_POOL_MAX_CNT)
// AVL tree for host
// index: client identifier, subnet_mask, ip & subnet_mask
VTSS_AVL_TREE(g_pool_id_avlt, "POOL_ID", 0, _pool_id_cmp_func, DHCP_SERVER_POOL_MAX_CNT)
// AVL tree for host
// index: chaddr, subnet_mask, ip & subnet_mask
VTSS_AVL_TREE(g_pool_chaddr_avlt, "POOL_CHADDR", 0, _pool_chaddr_cmp_func, DHCP_SERVER_POOL_MAX_CNT)

/*
    Binding list

    when the binding is retrieved from free list, it will be added into ip, id,
    chaddr and name avlts. If it is in use, then it is in lease avlt. Otherwise,
    if it is expired, then it is in expired avlt.
*/
// Free List
VTSS_FREE_LIST(g_binding_flist, dhcp_server_binding_t, DHCP_SERVER_BINDING_MAX_CNT)
// AVL tree, index: ip
VTSS_AVL_TREE(g_binding_ip_avlt, "BINDING_IP", 0, _binding_ip_cmp_func, DHCP_SERVER_BINDING_MAX_CNT)
// AVL tree, index: identifier, ip & subnet_mask
VTSS_AVL_TREE(g_binding_id_avlt, "BINDING_ID", 0, _binding_id_cmp_func, DHCP_SERVER_BINDING_MAX_CNT)
// AVL tree, index: chaddr, ip & subnet_mask
VTSS_AVL_TREE(g_binding_chaddr_avlt, "BINDING_CHADDR", 0, _binding_chaddr_cmp_func, DHCP_SERVER_BINDING_MAX_CNT)
// AVL tree, index: pool_name, ip
// pool_name is not unique, so the second key, ip, is needed.
VTSS_AVL_TREE(g_binding_name_avlt, "BINDING_NAME", 0, _binding_name_ip_cmp_func, DHCP_SERVER_BINDING_MAX_CNT)

// AVL tree, index: expire_time, ip
// expire_time is not unique, so the second key, ip, is needed.
VTSS_AVL_TREE(g_binding_lease_avlt, "BINDING_LEASE", 0, _binding_time_ip_cmp_func, DHCP_SERVER_BINDING_MAX_CNT)
// AVL tree, index: ip
VTSS_AVL_TREE(g_binding_expired_avlt, "BINDING_EXPIRED", 0, _binding_ip_cmp_func, DHCP_SERVER_BINDING_MAX_CNT)

/*
    Decline IP list
*/
// Free List
VTSS_FREE_LIST(g_decline_flist, vtss_ipv4_t, DHCP_SERVER_BINDING_MAX_CNT)
// AVL tree, index: ip
VTSS_AVL_TREE(g_decline_ip_avlt, "DECLINE_IP", 0, _decline_ip_cmp_func, DHCP_SERVER_BINDING_MAX_CNT)

/*
==============================================================================

    Static Function

==============================================================================
*/
/**
 *  \brief
 *      check if the MAC address is not empty.
 */
static BOOL _not_empty_mac(
    IN  vtss_mac_t  *mac
)
{
    u8 empty_mac[DHCP_SERVER_MAC_LEN] = {0, 0, 0, 0, 0, 0};

    if ( memcmp(mac, empty_mac, DHCP_SERVER_MAC_LEN) ) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 *  \brief
 *      check if ip is in the range of excluded.
 */
static BOOL _ip_in_excluded(
    IN dhcp_server_excluded_ip_t    *excluded,
    IN vtss_ipv4_t                  ip
)
{
    if ( ip >= excluded->low_ip && ip <= excluded->high_ip ) {
        return TRUE;
    }
    return FALSE;
}

/**
 *  \brief
 *      check if ip is excluded.
 */
static BOOL _ip_in_all_excluded(
    IN vtss_ipv4_t      ip
)
{
    dhcp_server_excluded_ip_t     excluded;
    dhcp_server_excluded_ip_t     *ep;

    memset(&excluded, 0, sizeof(excluded));
    ep = &excluded;
    while ( vtss_avl_tree_get(&g_excluded_ip_avlt, (void **)&ep, VTSS_AVL_TREE_GET_NEXT) ) {
        if ( _ip_in_excluded(ep, ip) ) {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 *  \brief
 *      check if ip is declined.
 */
static BOOL _ip_is_declined(
    IN vtss_ipv4_t      ip
)
{
    vtss_ipv4_t     *i;

    i = &ip;
    if ( vtss_avl_tree_get(&g_decline_ip_avlt, (void **)&i, VTSS_AVL_TREE_GET) ) {
        return TRUE;
    }
    return FALSE;
}

/**
 *  \brief
 *      check if ip is in the address pool.
 */
static BOOL _ip_in_pool(
    IN dhcp_server_pool_t   *pool,
    IN vtss_ipv4_t          ip
)
{
    if ( pool->type == DHCP_SERVER_POOL_TYPE_NONE ) {
        /* IP and subnet_mask not configured yet */
        return FALSE;
    }

    if ( (pool->ip & pool->subnet_mask) != (ip & pool->subnet_mask) ) {
        return FALSE;
    }
    return TRUE;
}

/**
 *  \brief
 *      check if pool is in the subnet of (vlan_ip, vlan_netmask).
 */
static BOOL _pool_in_subnet(
    IN dhcp_server_pool_t   *pool,
    IN vtss_ipv4_t          vlan_ip,
    IN vtss_ipv4_t          vlan_netmask
)
{
    vtss_ipv4_t     netmask;

    if ( pool->subnet_mask > vlan_netmask ) {
        netmask = vlan_netmask;
    } else {
        netmask = pool->subnet_mask;
    }

    if ( (pool->ip & netmask) != (vlan_ip & netmask) ) {
        return FALSE;
    }
    return TRUE;
}

/**
 *  \brief
 *      check if ip is in the subnet of (vlan_ip, vlan_netmask).
 */
static BOOL _ip_in_subnet(
    IN vtss_ipv4_t      ip,
    IN vtss_ipv4_t      vlan_ip,
    IN vtss_ipv4_t      vlan_netmask
)
{
    if ( ip == vlan_ip ) {
        return FALSE;
    }

    /*
        check VLAN interface subnet
    */
    // avoid the first 0 address
    if ( (ip & (~vlan_netmask)) == 0 ) {
        return FALSE;
    }

    // avoid broadcast address
    if ( (ip | vlan_netmask) == 0xFFffFFff ) {
        return FALSE;
    }

    // in subnet
    if ( (vlan_ip & vlan_netmask) != (ip & vlan_netmask) ) {
        return FALSE;
    }
    return TRUE;
}

/**
 *  \brief
 *      get old binding according to the received DHCP message
 *
 *  \param
 *      message [IN] : DHCP message.
 *      binding [OUT]: encap identifier and chaddr
 *
 *  \return
 *      n/a
 */
static void _binding_id_encap(
    IN  dhcp_server_message_t     *message,
    OUT dhcp_server_binding_t     *binding
)
{
    u8      identifier[DHCP_SERVER_CLIENT_IDENTIFIER_LEN + 4];
    u32     length;

    memset(&(binding->identifier), 0, sizeof(binding->identifier));
    memset(binding->chaddr.addr, 0, sizeof(binding->chaddr.addr));

    memset(identifier, 0, sizeof(identifier));

    length = DHCP_SERVER_CLIENT_IDENTIFIER_LEN + 4;
    if ( vtss_dhcp_server_message_option_get(message, DHCP_SERVER_MESSAGE_OPTION_CODE_CLIENT_ID, identifier, &length) ) {
        switch ( identifier[0] ) {
        case DHCP_SERVER_MESSAGE_CLIENT_IDENTIFIER_TYPE_FQDN:
            binding->identifier.type = DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_FQDN;
            strcpy(binding->identifier.u.fqdn, (char *)(identifier + 1));
            break;

        case DHCP_SERVER_MESSAGE_CLIENT_IDENTIFIER_TYPE_MAC:
            binding->identifier.type = DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_MAC;
            memcpy(&(binding->identifier.u.mac), &(identifier[1]), sizeof(binding->identifier.u.mac));
            break;

        default:
            T_W("Client ID type %u not supported\n", identifier[0]);
            break;
        }
    }

    // then, check chaddr
    if ( message->htype != _HTYPE_ETHERNET || message->hlen != DHCP_SERVER_MAC_LEN ) {
        T_W("hardware type %02x not supported\n", message->htype);
        return;
    }

    // get chaddr
    memcpy(binding->chaddr.addr, message->chaddr, DHCP_SERVER_MAC_LEN);
}

/**
 *  \brief
 *      get binding type according to pool
 *
 *  \param
 *      pool    [IN]: pool
 *
 *  \return
 *      dhcp_server_binding_type_t
 */
static dhcp_server_binding_type_t _binding_type_get(
    IN  dhcp_server_pool_t  *pool
)
{
    if ( pool->type == DHCP_SERVER_POOL_TYPE_HOST ) {
        return DHCP_SERVER_BINDING_TYPE_MANUAL;
    }
    return DHCP_SERVER_BINDING_TYPE_AUTOMATIC;
}

/**
 *  \brief
 *      increase statistics for binding type
 *
 *  \param
 *      type [IN]: binding type
 *
 *  \return
 *      n/a
 */
static void _binding_type_statistic_inc(
    IN  dhcp_server_binding_type_t      type
)
{
    switch ( type ) {
    case DHCP_SERVER_BINDING_TYPE_NONE:
    default:
        break;

    case DHCP_SERVER_BINDING_TYPE_AUTOMATIC:
        STATISTICS_INC( automatic_binding_cnt );
        break;

    case DHCP_SERVER_BINDING_TYPE_MANUAL:
        STATISTICS_INC( manual_binding_cnt );
        break;

    case DHCP_SERVER_BINDING_TYPE_EXPIRED:
        STATISTICS_INC( expired_binding_cnt );
        break;
    }
}

/**
 *  \brief
 *      decrease statistics for binding type
 *
 *  \param
 *      type [IN]: binding type
 *
 *  \return
 *      n/a
 */
static void _binding_type_statistic_dec(
    IN  dhcp_server_binding_type_t      type
)
{
    switch ( type ) {
    case DHCP_SERVER_BINDING_TYPE_NONE:
    default:
        break;

    case DHCP_SERVER_BINDING_TYPE_AUTOMATIC:
        STATISTICS_DEC( automatic_binding_cnt );
        break;

    case DHCP_SERVER_BINDING_TYPE_MANUAL:
        STATISTICS_DEC( manual_binding_cnt );
        break;

    case DHCP_SERVER_BINDING_TYPE_EXPIRED:
        STATISTICS_DEC( expired_binding_cnt );
        break;
    }
}

/**
 *  \brief
 *      get a new binding from free list
 *
 *  \param
 *      message [IN]: DHCP message.
 *      vlan_ip [IN]: IP address of the interface from that DHCP message comes
 *
 *  \return
 *      *    : successful.
 *      NULL : failed
 */
static dhcp_server_binding_t *_binding_new(
    IN  dhcp_server_binding_state_t     state,
    IN  dhcp_server_pool_t              *pool,
    IN  dhcp_server_message_t           *message,
    IN  vtss_ipv4_t                     vlan_ip,
    IN  vtss_ipv4_t                     vlan_netmask,
    IN  vtss_ipv4_t                     yiaddr
)
{
    dhcp_server_binding_t       *binding;

    binding = (dhcp_server_binding_t *)vtss_free_list_malloc( &g_binding_flist );
    if ( binding == NULL ) {
        T_W("no free memory in g_binding_flist\n");
        return NULL;
    }

    binding->ip            = yiaddr;
    binding->subnet_mask   = vlan_netmask;
    binding->state         = state;
    binding->type          = _binding_type_get( pool );
    binding->server_id     = vlan_ip;
    binding->pool_name     = pool->pool_name;
    binding->lease         = (state == DHCP_SERVER_BINDING_STATE_ALLOCATED) ? DHCP_SERVER_ALLOCATION_EXPIRE_TIME : pool->lease;
    binding->time_to_start = dhcp_server_current_time_get();
    binding->expire_time   = binding->time_to_start + binding->lease;

    _binding_id_encap( message, binding );

    if ( vtss_avl_tree_add(&g_binding_ip_avlt, (void *)binding) == FALSE ) {
        vtss_free_list_free( &g_binding_flist, binding );
        T_W("Full in g_binding_ip_avlt\n");
        return NULL;
    }

    if ( binding->identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
        if ( vtss_avl_tree_add(&g_binding_id_avlt, (void *)binding) == FALSE ) {
            vtss_free_list_free( &g_binding_flist, binding );
            T_W("Full in g_binding_id_avlt\n");
            return NULL;
        }
    } else if ( _not_empty_mac(&(binding->chaddr)) ) {
        if ( vtss_avl_tree_add(&g_binding_chaddr_avlt, (void *)binding) == FALSE ) {
            vtss_free_list_free( &g_binding_flist, binding );
            T_W("Full in g_binding_chaddr_avlt\n");
            return NULL;
        }
    }

    if ( vtss_avl_tree_add(&g_binding_name_avlt, (void *)binding) == FALSE ) {
        vtss_free_list_free( &g_binding_flist, binding );
        T_W("Full in g_binding_name_avlt\n");
        return NULL;
    }

    // if with lease, add into lease tree
    // if not lease then it is automatic allocation with permanent IP address
    if ( binding->lease ) {
        if ( vtss_avl_tree_add(&g_binding_lease_avlt, (void *)binding) == FALSE ) {
            vtss_free_list_free( &g_binding_flist, binding );
            T_W("Full in g_binding_lease_avlt\n");
            return NULL;
        }
    }

    _binding_type_statistic_inc( binding->type );

    ++( pool->alloc_cnt );
    return binding;
}

/**
 *  \brief
 *      remove binding from all avl trees and put it back to free list.
 *
 *  \param
 *      binding  [IN]: binding to be removed.
 *
 *  \return
 *      n/a.
 */
static void _binding_remove(
    IN  dhcp_server_binding_t     *binding
)
{
    dhcp_server_pool_t      *pool;

    switch ( binding->state ) {
    case DHCP_SERVER_BINDING_STATE_COMMITTED:
    case DHCP_SERVER_BINDING_STATE_ALLOCATED:
        pool = (dhcp_server_pool_t *)(binding->pool_name);
        --( pool->alloc_cnt );
        break;

    case DHCP_SERVER_BINDING_STATE_EXPIRED:
    case DHCP_SERVER_BINDING_STATE_NONE:
    default:
        break;
    }

    _binding_type_statistic_dec( binding->type );

    (void)vtss_avl_tree_delete(&g_binding_ip_avlt,      (void **)&binding);
    (void)vtss_avl_tree_delete(&g_binding_id_avlt,      (void **)&binding);
    (void)vtss_avl_tree_delete(&g_binding_chaddr_avlt,  (void **)&binding);
    (void)vtss_avl_tree_delete(&g_binding_name_avlt,    (void **)&binding);
    (void)vtss_avl_tree_delete(&g_binding_lease_avlt,   (void **)&binding);
    (void)vtss_avl_tree_delete(&g_binding_expired_avlt, (void **)&binding);

    vtss_free_list_free(&g_binding_flist, binding);
}

/**
 *  \brief
 *      move binding to expired state.
 *
 *  \param
 *      binding [INOUT]: binding.
 *
 *  \return
 *      n/a.
 */
static void _binding_expired_move(
    INOUT  dhcp_server_binding_t     *binding
)
{
    dhcp_server_pool_t      *pool;

    switch ( binding->state ) {
    case DHCP_SERVER_BINDING_STATE_COMMITTED:
    case DHCP_SERVER_BINDING_STATE_ALLOCATED:
        pool = (dhcp_server_pool_t *)(binding->pool_name);
        --( pool->alloc_cnt );
        break;

    case DHCP_SERVER_BINDING_STATE_EXPIRED:
    case DHCP_SERVER_BINDING_STATE_NONE:
    default:
        break;
    }

    _binding_type_statistic_dec( binding->type );

    // remove from lease
    (void)vtss_avl_tree_delete(&g_binding_lease_avlt, (void **)&binding);
    (void)vtss_avl_tree_delete(&g_binding_expired_avlt, (void **)&binding);

    // update state
    binding->type        = DHCP_SERVER_BINDING_TYPE_EXPIRED;
    binding->state       = DHCP_SERVER_BINDING_STATE_EXPIRED;
    binding->expire_time = dhcp_server_current_time_get();

    // add into expired
    if ( vtss_avl_tree_add(&g_binding_expired_avlt, (void *)binding) == FALSE ) {
        T_W("Full in g_binding_expired_avlt\n");
    }

    _binding_type_statistic_inc( binding->type );
}

/**
 *  \brief
 *      Move all lease bindings to expired.
 *
 *  \param
 *      n/a.
 *
 *  \return
 *      n/a.
 */
static void _binding_expired_move_all(
    void
)
{
    dhcp_server_binding_t     binding;
    dhcp_server_binding_t     *bp;

    memset(&binding, 0, sizeof(binding));
    bp = &binding;
    while ( vtss_avl_tree_get(&g_binding_lease_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
        _binding_expired_move( bp );
    }
}

/**
 *  \brief
 *      move binding to allocated state.
 *
 *  \param
 *      binding [INOUT]: binding.
 *
 *  \return
 *      n/a.
 */
static void _binding_allocated_move(
    INOUT   dhcp_server_binding_t     *binding
)
{
    dhcp_server_pool_t      *pool;
    u32                     old_state;

    // remove from lease and expired
    (void)vtss_avl_tree_delete(&g_binding_lease_avlt, (void **)&binding);
    (void)vtss_avl_tree_delete(&g_binding_expired_avlt, (void **)&binding);

    _binding_type_statistic_dec( binding->type );

    // get old state
    old_state = binding->state;

    // update state
    binding->state         = DHCP_SERVER_BINDING_STATE_ALLOCATED;
    binding->type          = _binding_type_get( (dhcp_server_pool_t *)(binding->pool_name) );
    binding->lease         = DHCP_SERVER_ALLOCATION_EXPIRE_TIME;
    binding->time_to_start = dhcp_server_current_time_get();
    binding->expire_time   = binding->time_to_start + binding->lease;

    _binding_type_statistic_inc( binding->type );

    // add into expired
    if ( vtss_avl_tree_add(&g_binding_lease_avlt, (void *)binding) == FALSE ) {
        T_W("Full in g_binding_lease_avlt\n");
    }

    if ( old_state == DHCP_SERVER_BINDING_STATE_EXPIRED ) {
        pool = (dhcp_server_pool_t *)(binding->pool_name);
        ++( pool->alloc_cnt );
    }
}

/**
 *  \brief
 *      move binding to committed state.
 *
 *  \param
 *      binding [INOUT]: binding.
 *
 *  \return
 *      n/a.
 */
static void _binding_committed_move(
    INOUT   dhcp_server_binding_t     *binding
)
{
    dhcp_server_pool_t      *pool;
    u32                     old_state;

    // remove from lease and expired
    (void)vtss_avl_tree_delete(&g_binding_lease_avlt, (void **)&binding);
    (void)vtss_avl_tree_delete(&g_binding_expired_avlt, (void **)&binding);

    pool = (dhcp_server_pool_t *)(binding->pool_name);

    _binding_type_statistic_dec( binding->type );

    // get old state
    old_state = binding->state;

    // update state
    binding->state         = DHCP_SERVER_BINDING_STATE_COMMITTED;
    binding->type          = _binding_type_get( pool );
    binding->lease         = pool->lease;
    binding->time_to_start = dhcp_server_current_time_get();
    binding->expire_time   = binding->time_to_start + binding->lease;

    _binding_type_statistic_inc( binding->type );

    // add into expired
    if ( binding->lease ) {
        if ( vtss_avl_tree_add(&g_binding_lease_avlt, (void *)binding) == FALSE ) {
            T_W("Full in g_binding_lease_avlt\n");
        }
    }

    if ( old_state == DHCP_SERVER_BINDING_STATE_EXPIRED ) {
        ++( pool->alloc_cnt );
    }
}

/**
 *  \brief
 *      move old binding to allocated state.
 */
static BOOL _old_binding_allocated(
    IN    dhcp_server_pool_t      *pool,
    IN    dhcp_server_message_t   *message,
    IN    vtss_ipv4_t             vlan_ip,
    IN    vtss_ipv4_t             vlan_netmask,
    INOUT dhcp_server_binding_t   *binding
)
{
    dhcp_server_binding_type_t  old_type;
    u32                         old_state;

    /* remove from lease and expired */
    (void)vtss_avl_tree_delete(&g_binding_lease_avlt, (void **)&binding);
    (void)vtss_avl_tree_delete(&g_binding_expired_avlt, (void **)&binding);

    /* remove from id or chaddr tree */
    if ( binding->identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
        (void)vtss_avl_tree_delete(&g_binding_id_avlt, (void **)&binding);
    } else if ( _not_empty_mac(&(binding->chaddr)) ) {
        (void)vtss_avl_tree_delete(&g_binding_chaddr_avlt, (void **)&binding);
    }

    /* get new submask */
    binding->subnet_mask = vlan_netmask;

    /* get new ID */
    _binding_id_encap( message, binding );

    if ( binding->identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
        if ( vtss_avl_tree_add(&g_binding_id_avlt, (void *)binding) == FALSE ) {
            vtss_free_list_free( &g_binding_flist, binding );
            T_W("Full in g_binding_id_avlt\n");
            return FALSE;
        }
    } else if ( _not_empty_mac(&(binding->chaddr)) ) {
        if ( vtss_avl_tree_add(&g_binding_chaddr_avlt, (void *)binding) == FALSE ) {
            vtss_free_list_free( &g_binding_flist, binding );
            T_W("Full in g_binding_chaddr_avlt\n");
            return FALSE;
        }
    }

    /* update state */
    old_state = binding->state;
    old_type  = binding->type;

    // ip is reused, so not updated
    binding->state         = DHCP_SERVER_BINDING_STATE_ALLOCATED;
    binding->type          = _binding_type_get( pool );
    binding->server_id     = vlan_ip;
    binding->pool_name     = pool->pool_name;
    binding->lease         = DHCP_SERVER_ALLOCATION_EXPIRE_TIME;
    binding->time_to_start = dhcp_server_current_time_get();
    binding->expire_time   = binding->time_to_start + binding->lease;

    /* add into lease tree */
    if ( vtss_avl_tree_add(&g_binding_lease_avlt, (void *)binding) == FALSE ) {
        vtss_free_list_free(&g_binding_flist, binding);
        T_W("Full in g_binding_lease_avlt\n");
        return FALSE;
    }

    if ( old_type != binding->type ) {
        _binding_type_statistic_dec( old_type );
        _binding_type_statistic_inc( binding->type );
    }

    if ( old_state == DHCP_SERVER_BINDING_STATE_EXPIRED ) {
        ++( pool->alloc_cnt );
    }

    return TRUE;
}

static BOOL _ip_is_manual(
    IN  vtss_ipv4_t     ip
)
{
    dhcp_server_pool_t    pool;
    dhcp_server_pool_t    *p;

    memset(&pool, 0, sizeof(pool));
    p = &pool;
    while ( vtss_avl_tree_get(&g_pool_name_avlt, (void **)&p, VTSS_AVL_TREE_GET_NEXT) ) {
        if ( p->type == DHCP_SERVER_POOL_TYPE_HOST && p->ip == ip ) {
            return TRUE;
        }
    }
    return FALSE;
}

/**
 *  \brief
 *      check if ip is not used.
 */
static BOOL _ip_is_free(
    IN  vtss_ipv4_t             ip,
    IN  vtss_ipv4_t             netmask,
    IN  vtss_ipv4_t             vlan_ip,
    IN  vtss_ipv4_t             vlan_netmask,
    OUT dhcp_server_binding_t   **expired
)
{
    dhcp_server_binding_t     binding;
    dhcp_server_binding_t     *bp;

    /*
        check pool subnet
    */
    // avoid the first 0 address
    if ( (ip & (~netmask)) == 0 ) {
        return FALSE;
    }

    // avoid broadcast address
    if ( (ip | netmask) == 0xFFffFFff ) {
        return FALSE;
    }

    /* ip in the VLAN subnet? */
    if ( _ip_in_subnet(ip, vlan_ip, vlan_netmask) == FALSE ) {
        return FALSE;
    }

    /* ip is manual IP? */
    if ( _ip_is_manual(ip) ) {
        return FALSE;
    }

    /* ip is excluded? */
    if ( _ip_in_all_excluded(ip) ) {
        return FALSE;
    }

    /* ip is declined? */
    if ( _ip_is_declined(ip) ) {
        return FALSE;
    }

    // check binding
    memset(&binding, 0, sizeof(binding));
    binding.ip = ip;
    bp = &binding;
    if ( vtss_avl_tree_get(&g_binding_ip_avlt, (void **)&bp, VTSS_AVL_TREE_GET) ) {
        switch ( bp->state ) {
        case DHCP_SERVER_BINDING_STATE_COMMITTED:
        case DHCP_SERVER_BINDING_STATE_ALLOCATED:
            /* in used */
            return FALSE;

        case DHCP_SERVER_BINDING_STATE_EXPIRED:
        case DHCP_SERVER_BINDING_STATE_NONE:
        default:
            /* free */
            *expired = bp;
            break;
        }
    }

    return TRUE;
}

/**
 *  \brief
 *      delete declined IP from avl tree and put it back to free list
 *
 *  \param
 *      declined_ip [IN]: declined IP to be deleted
 *
 *  \return
 *      n/a.
 */
static void _declined_ip_delete(
    IN vtss_ipv4_t      *declined_ip
)
{
    vtss_ipv4_t     *ipp;

    ipp = declined_ip;
    if ( vtss_avl_tree_delete(&g_decline_ip_avlt, (void **)&ipp) ) {
        vtss_free_list_free(&g_decline_flist, ipp);
        STATISTICS_DEC( declined_cnt );
    }
}

#define _FREE_IP_GET(_init_, _end_) \
    ip = pool->ip & pool->subnet_mask; \
    for ( i = (_init_); i < (_end_); i++ ) { \
        ep = NULL; \
        if ( _ip_is_free(ip + i, pool->subnet_mask, vlan_ip, vlan_netmask, &ep) ) { \
            if ( ep ) { \
                if ( longest_expired ) { \
                    if ( ep->expire_time < longest_expired->expire_time ) { \
                        longest_expired = ep; \
                    } \
                } else { \
                    longest_expired = ep; \
                } \
            } else { \
                return (ip + i); \
            } \
        } \
    }

/**
 *  \brief
 *      get a new free IP address in IP pool
 *
 *  \param
 *      pool         [IN]: IP pool.
 *      vlan_ip      [IN]: ip address of VLAN interface
 *      vlan_netmask [IN]: subnet mask of VLAN interface
 *      binding     [OUT]: expired binding of the free IP
 *
 *  \return
 *      * : free IP.
 *      0 : no free IP.
 */
static vtss_ipv4_t _free_ip_get(
    IN  dhcp_server_pool_t      *pool,
    IN  vtss_ipv4_t             vlan_ip,
    IN  vtss_ipv4_t             vlan_netmask,
    OUT dhcp_server_binding_t   **expired
)
{
    u32                     i;
    vtss_ipv4_t             ip;
    u32                     count;
    vtss_ipv4_t             *ipp;
    dhcp_server_binding_t   *ep;
    dhcp_server_binding_t   *longest_expired;

    longest_expired = NULL;

    /* 1. get free IP first */
    if ( pool->subnet_mask > vlan_netmask ) {
        ip = pool->ip & pool->subnet_mask;
        count = ( ~(pool->subnet_mask) ) + 1;
    } else {
        ip = vlan_ip & vlan_netmask;
        count = ( ~vlan_netmask ) + 1;
    }

    for ( i = 0; i < count; i++ ) {
        ep = NULL;
        if ( _ip_is_free(ip + i, pool->subnet_mask, vlan_ip, vlan_netmask, &ep) ) {
            if ( ep ) {
                if ( longest_expired ) {
                    if ( ep->expire_time < longest_expired->expire_time ) {
                        longest_expired = ep;
                    }
                } else {
                    longest_expired = ep;
                }
            } else {
                return (ip + i);
            }
        }
    }

    /* 2. no free IP, then use longest expired IP */
    if ( longest_expired ) {
        *expired = longest_expired;
        return longest_expired->ip;
    }

    /* 3. no expired IP, then use declined IP */
    ip  = 0;
    ipp = &ip;

    while ( vtss_avl_tree_get(&g_decline_ip_avlt, (void **)&ipp, VTSS_AVL_TREE_GET_NEXT) ) {
        if ( _ip_in_pool(pool, *ipp) ) {
            ip = *ipp;
            _declined_ip_delete( ipp );
            return ip;
        }
    }

    /* 4. all are in used, no any IP is available */
    return 0;
}

/**
 *  \brief
 *      translate the value to RFC
 *
 */
static u8 _netbios_node_type_get(
    IN  u32     netbios_node_type
)
{
    switch ( netbios_node_type ) {
    case DHCP_SERVER_NETBIOS_NODE_TYPE_NONE:
    default:
        return 0;

    case DHCP_SERVER_NETBIOS_NODE_TYPE_B:
        return 1;

    case DHCP_SERVER_NETBIOS_NODE_TYPE_P:
        return 2;

    case DHCP_SERVER_NETBIOS_NODE_TYPE_M:
        return 4;

    case DHCP_SERVER_NETBIOS_NODE_TYPE_H:
        return 8;
    }
}

#define _server_option_pack(_op_, _f_) \
    if ( pool->_f_[0] == 0 ) { \
        break; \
    } \
    option[option_len++] = _op_; \
    len_index = option_len++; \
    for ( j = 0; j < DHCP_SERVER_SERVER_MAX_CNT; j++ ) { \
        if ( pool->_f_[j] ) { \
            n = htonl( pool->_f_[j] ); \
            memcpy(&(option[option_len]), &n, 4); \
            option_len += 4; \
        } else { \
            break; \
        } \
    } \
    option[len_index] = j * 4;

#define _name_option_pack(_op_, _f_) \
    n = strlen( pool->_f_ ) + 1; \
    if ( n == 1 ) { \
        break; \
    } \
    option[option_len++] = _op_; \
    option[option_len++] = (u8)n; \
    s = (char *)&( option[option_len] ); \
    strcpy(s, pool->_f_); \
    option_len += n;

#define _u32_option_pack(_op_, _v_) \
    option[option_len++] = _op_; \
    option[option_len++] = 4; \
    n = htonl( _v_ ); \
    memcpy(&(option[option_len]), &n, 4); \
    option_len += 4;

/**
 *  \brief
 *      pack options according to the incoming message and
 *      the corresponding pool
 */
static u32  _option_pack(
    IN  dhcp_server_message_t   *message,
    IN  u8                      message_type,
    IN  dhcp_server_pool_t      *pool,
    IN  vtss_ipv4_t             vlan_ip,
    IN  BOOL                    b_lease,
    OUT u8                      *option
)
{
    u32     length;
    u8      paramter[_PARAMETER_LIST_MAX_LEN];
    u32     option_len;
    u32     i;
    u8      j;
    u32     n;
    u32     len_index;
    char    *s;
    char    class_id[DHCP_SERVER_VENDOR_CLASS_ID_LEN + 1];

    option_len = 0;

    /* magic cookie */
    option[option_len++] = 0x63;
    option[option_len++] = 0x82;
    option[option_len++] = 0x53;
    option[option_len++] = 0x63;

    /* message type */
    option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_MESSAGE_TYPE;
    option[option_len++] = 1;
    option[option_len++] = message_type;

    /* option 60 */
    memset(class_id, 0, sizeof(class_id));
    length = DHCP_SERVER_VENDOR_CLASS_ID_LEN;
    if ( vtss_dhcp_server_message_option_get(message, DHCP_SERVER_MESSAGE_OPTION_CODE_VENDOR_CLASS_ID, class_id, &length) ) {
        for ( i = 0; i < DHCP_SERVER_VENDOR_CLASS_INFO_CNT; i++ ) {
            if ( strcmp(pool->class_info[i].class_id, class_id) == 0 ) {
                if ( pool->class_info[i].specific_info_len ) {
                    option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_VENDOR_SPECIFIC_INFO;
                    option[option_len++] = (u8)( pool->class_info[i].specific_info_len );
                    for ( n = 0; n < pool->class_info[i].specific_info_len; n++ ) {
                        option[option_len++] = pool->class_info[i].specific_info[n];
                    }
                }
                break;
            }
        }
    }

    /* pack option */
    // DHCP_SERVER_MESSAGE_OPTION_CODE_SERVER_ID:
    _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_SERVER_ID, vlan_ip);

    // DHCP_SERVER_MESSAGE_OPTION_CODE_SUBNET_MASK:
    _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_SUBNET_MASK, pool->subnet_mask);

    /* pack lease */
    if ( b_lease ) {
        if ( pool->lease ) {
            // DHCP_SERVER_MESSAGE_OPTION_CODE_LEASE_TIME
            _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_LEASE_TIME, pool->lease);
            // DHCP_SERVER_MESSAGE_OPTION_CODE_RENEWAL_TIME
            _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_RENEWAL_TIME, pool->lease / 2);
            // DHCP_SERVER_MESSAGE_OPTION_CODE_REBINDING_TIME
            _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_REBINDING_TIME, pool->lease * 3 / 4);
        } else {
            // DHCP_SERVER_MESSAGE_OPTION_CODE_LEASE_TIME
            _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_LEASE_TIME, 0xFFffFFff);
        }
    }

    /* parameter list */
    length = _PARAMETER_LIST_MAX_LEN;
    if ( vtss_dhcp_server_message_option_get(message, DHCP_SERVER_MESSAGE_OPTION_CODE_PARAMETER_LIST, paramter, &length) == FALSE ) {
        /* END */
        option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_END;
        return option_len;
    }

    for ( i = 0; i < length; i++ ) {
        switch ( paramter[i] ) {
#if 0
        case DHCP_SERVER_MESSAGE_OPTION_CODE_SUBNET_MASK:
            _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_SUBNET_MASK, pool->subnet_mask);
            break;
#endif
        case DHCP_SERVER_MESSAGE_OPTION_CODE_ROUTER:
            _server_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_ROUTER, default_router);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_DNS_SERVER:
            _server_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_DNS_SERVER, dns_server);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_HOST_NAME:
            _name_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_HOST_NAME, client_name);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_DOMAIN_NAME:
            _name_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_DOMAIN_NAME, domain_name);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_BROADCAST:
            if ( pool->subnet_broadcast == 0 ) {
                break;
            }
            _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_BROADCAST, pool->subnet_broadcast);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_NIS_DOMAIN_NAME:
            _name_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_NIS_DOMAIN_NAME, nis_domain_name);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_NIS_SERVER:
            _server_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_NIS_SERVER, nis_server);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_NTP_SERVER:
            _server_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_NTP_SERVER, ntp_server);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_NAME_SERVER:
            _server_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_NAME_SERVER, netbios_name_server);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_NODE_TYPE:
            if ( pool->netbios_node_type == DHCP_SERVER_NETBIOS_NODE_TYPE_NONE ) {
                break;
            }
            option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_NODE_TYPE;
            option[option_len++] = 1;
            option[option_len++] = _netbios_node_type_get( pool->netbios_node_type );
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_SCOPE:
            _name_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_SCOPE, netbios_scope);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_MAX_MESSAGE_SIZE:
            _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_MAX_MESSAGE_SIZE, DHCP_SERVER_DEFAULT_MAX_MESSAGE_SIZE);
            break;

        case DHCP_SERVER_MESSAGE_OPTION_CODE_VENDOR_CLASS_ID:
        case DHCP_SERVER_MESSAGE_OPTION_CODE_CLIENT_ID:
        case DHCP_SERVER_MESSAGE_OPTION_CODE_REQUESTED_IP:
        case DHCP_SERVER_MESSAGE_OPTION_CODE_OPTION_OVERLOAD:
        case DHCP_SERVER_MESSAGE_OPTION_CODE_MESSAGE_TYPE:
        case DHCP_SERVER_MESSAGE_OPTION_CODE_PARAMETER_LIST:
            break;

        default:
            //T_W("option %u not supported\n", paramter[i]);
            break;
        }
    }

    /* END */
    option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_END;

    return option_len;
}

/**
 *  \brief
 *      pack options to the default value
 */
static u32  _default_option_pack(
    IN  u8                  message_type,
    IN  dhcp_server_pool_t  *pool,
    IN  vtss_ipv4_t         vlan_ip,
    OUT u8                  *option
)
{
    u32     option_len;
    u32     n;

    option_len = 0;

    /* magic cookie */
    option[option_len++] = 0x63;
    option[option_len++] = 0x82;
    option[option_len++] = 0x53;
    option[option_len++] = 0x63;

    /* message type */
    option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_MESSAGE_TYPE;
    option[option_len++] = 1;
    option[option_len++] = message_type;

    /* subnet mask */
    _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_SUBNET_MASK, pool->subnet_mask);

    /* server identifier */
    option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_SERVER_ID;
    option[option_len++] = 4;
    n = htonl( vlan_ip );
    memcpy(&(option[option_len]), &n, 4);
    option_len += 4;

    /* lease */
    _u32_option_pack( DHCP_SERVER_MESSAGE_OPTION_CODE_LEASE_TIME, pool->lease );

    /* renew */
    _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_LEASE_TIME, pool->lease / 2);

    /* rebind */
    _u32_option_pack(DHCP_SERVER_MESSAGE_OPTION_CODE_LEASE_TIME, pool->lease * 3 / 4);

    /* END */
    option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_END;

    return option_len;
}

/**
 *  \brief
 *      move the binding of ip to expired state
 */
static void _ip_binding_expired(
    IN  vtss_ipv4_t         ip
)
{
    dhcp_server_binding_t     binding;
    dhcp_server_binding_t     *bp;

    memset(&binding, 0, sizeof(binding));
    binding.ip = ip;
    bp = &binding;
    if ( vtss_avl_tree_get(&g_binding_ip_avlt, (void **)&bp, VTSS_AVL_TREE_GET) ) {
        _binding_expired_move( bp );
    }
}

/**
 *  \brief
 *      find binding according to the incoming message
 */
static dhcp_server_binding_t *_binding_get_by_pkt(
    IN  dhcp_server_message_t   *message,
    IN  vtss_ipv4_t             vlan_ip,
    IN  vtss_ipv4_t             vlan_netmask
)
{
    dhcp_server_binding_t     binding;
    dhcp_server_binding_t     *bp;

    memset(&binding, 0, sizeof(binding));

    binding.ip          = vlan_ip;
    binding.subnet_mask = vlan_netmask;
    _binding_id_encap( message, &binding );

    // find binding avlt
    bp = &binding;
    if ( bp->identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
        if ( vtss_avl_tree_get(&g_binding_id_avlt, (void **)&bp, VTSS_AVL_TREE_GET) ) {
            return bp;
        }
    } else if ( _not_empty_mac(&(bp->chaddr)) ) {
        if ( vtss_avl_tree_get(&g_binding_chaddr_avlt, (void **)&bp, VTSS_AVL_TREE_GET) ) {
            return bp;
        }
    }
    return NULL;
}

/**
 *  \brief
 *      get old binding according to the received DHCP message
 *
 *  \param
 *      message      [IN]: DHCP message.
 *      vlan_ip      [IN]: IP address of the interface from that DHCP message comes
 *      vlan_netmask [IN]: netmask of the interface from that DHCP message comes
 *
 *  \return
 *      *    : successful.
 *      NULL : failed
 */
static dhcp_server_binding_t *_binding_get_old(
    IN  dhcp_server_message_t   *message,
    IN  vtss_ipv4_t             vlan_ip,
    IN  vtss_ipv4_t             vlan_netmask
)
{
    dhcp_server_binding_t     *bp;
    dhcp_server_pool_t        *p;

    // find binding avlt
    bp = _binding_get_by_pkt( message, vlan_ip, vlan_netmask );
    if ( bp == NULL ) {
        return NULL;
    }

    p = (dhcp_server_pool_t *)(bp->pool_name);
    if ( _pool_in_subnet(p, vlan_ip, vlan_netmask) && _ip_in_subnet(bp->ip, vlan_ip, vlan_netmask) ) {
        return bp;
    }

    /* pool is not correct, return NULL to get new */
    _binding_remove( bp );
    return NULL;
}

/**
 *  \brief
 *      create a new binding according to the received DHCP message
 *
 *  \param
 *      message [IN]: DHCP message.
 *      vlan_ip [IN]: IP address of the interface from that DHCP message comes
 *
 *  \return
 *      *    : successful.
 *      NULL : failed
 */
static dhcp_server_binding_t *_binding_get_new(
    IN  dhcp_server_message_t   *message,
    IN  vtss_ipv4_t             vlan_ip,
    IN  vtss_ipv4_t             vlan_netmask
)
{
    u8                      identifier[DHCP_SERVER_CLIENT_IDENTIFIER_LEN + 4];
    u32                     length;
    dhcp_server_pool_t      pool;
    dhcp_server_pool_t      *p;
    dhcp_server_pool_t      *pool_expired;
    dhcp_server_binding_t   *binding;
    dhcp_server_binding_t   *binding_expired;
    BOOL                    b_get;
    vtss_ipv4_t             requested_ip;
    vtss_ipv4_t             ip;
    i32                     r;

    /*
        find host
    */
    memset(&pool, 0, sizeof(pool));
    memset(identifier, 0, sizeof(identifier));

    ip = 0;
    length = DHCP_SERVER_CLIENT_IDENTIFIER_LEN + 4;
    if ( vtss_dhcp_server_message_option_get(message, DHCP_SERVER_MESSAGE_OPTION_CODE_CLIENT_ID, identifier, &length) ) {
        p = NULL;
        switch ( identifier[0] ) {
        case DHCP_SERVER_MESSAGE_CLIENT_IDENTIFIER_TYPE_FQDN:
            pool.client_identifier.type = DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_FQDN;
            strcpy(pool.client_identifier.u.fqdn, (char *)(identifier + 1));
            p = &pool;
            break;

        case DHCP_SERVER_MESSAGE_CLIENT_IDENTIFIER_TYPE_MAC:
            pool.client_identifier.type = DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_MAC;
            memcpy(&(pool.client_identifier.u.mac), &(identifier[1]), sizeof(pool.client_identifier.u.mac));
            p = &pool;
            break;

        default:
            T_W("Client ID type %u not supported\n", identifier[0]);
            break;
        }

        if ( p ) {
            p->subnet_mask = 0xFFffFFff;
            while ( vtss_avl_tree_get(&g_pool_id_avlt, (void **)&p, VTSS_AVL_TREE_GET_PREV) ) {
                /* compare client identifier */
                r = _client_identifier_cmp( &(p->client_identifier), &(pool.client_identifier) );
                if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
                    break;
                }
                if ( _pool_in_subnet(p, vlan_ip, vlan_netmask) && _ip_in_subnet(p->ip, vlan_ip, vlan_netmask) &&
                     ( ! _ip_in_all_excluded(p->ip) ) && ( ! _ip_is_declined(p->ip) ) ) {
                    binding = _binding_new( DHCP_SERVER_BINDING_STATE_ALLOCATED, p, message, vlan_ip, vlan_netmask, p->ip );
                    if ( binding == NULL ) {
                        T_W("fail to get a new binding\n");
                    }
                    return binding;
                } else {
                    /* the host is not valid, find free IP in pool */
                }
            }
        }
    } else {
        // then, check chaddr
        if ( message->htype == _HTYPE_ETHERNET && message->hlen == DHCP_SERVER_MAC_LEN ) {
            memcpy(pool.client_haddr.addr, message->chaddr, DHCP_SERVER_MAC_LEN);
            p = &pool;
            p->subnet_mask = 0xFFffFFff;
            while ( vtss_avl_tree_get(&g_pool_chaddr_avlt, (void **)&p, VTSS_AVL_TREE_GET_PREV) ) {
                /* compare chaddr */
                r = _mac_cmp( &(p->client_haddr), &(pool.client_haddr) );
                if ( r != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
                    break;
                }
                if ( _pool_in_subnet(p, vlan_ip, vlan_netmask) && _ip_in_subnet(p->ip, vlan_ip, vlan_netmask) &&
                     ( ! _ip_in_all_excluded(p->ip) ) && ( ! _ip_is_declined(p->ip) ) ) {
                    binding = _binding_new( DHCP_SERVER_BINDING_STATE_ALLOCATED, p, message, vlan_ip, vlan_netmask, p->ip );
                    if ( binding == NULL ) {
                        T_W("fail to new a binding\n");
                    }
                    return binding;
                } else {
                    /* the host is not valid, find free IP in pool */
                }
            }
        }
    }

    /*
        find network
    */
    // get client request IP from message
    length = 4;
    requested_ip = 0;
    (void)vtss_dhcp_server_message_option_get( message, DHCP_SERVER_MESSAGE_OPTION_CODE_REQUESTED_IP, &requested_ip, &length );

    // go to avlt
    pool_expired    = NULL;
    binding_expired = NULL;
    b_get = FALSE;

    memset(&pool, 0, sizeof(pool));
    pool.ip          = 0xffFFffFF;
    pool.subnet_mask = 0xffFFffFF;
    p                = &pool;

    /*
        hint: go ip avlt from last by PREV, so the netmask will be longest match
    */
    while ( vtss_avl_tree_get(&g_pool_ip_avlt, (void **)&p, VTSS_AVL_TREE_GET_PREV) ) {
        if ( p->type == DHCP_SERVER_POOL_TYPE_HOST ) {
            continue;
        }

        if ( _pool_in_subnet(p, vlan_ip, vlan_netmask) == FALSE ) {
            continue;
        }

        // if client request IP, check this IP first
        if ( requested_ip ) {
            if ( _ip_in_pool(p, requested_ip) ) {
                binding = NULL;
                if ( _ip_is_free(requested_ip, p->subnet_mask, vlan_ip, vlan_netmask, &binding) ) {
                    if ( binding ) {
                        pool_expired    = p;
                        binding_expired = binding;
                    } else {
                        b_get = TRUE;
                        ip = requested_ip;
                    }
                    break;
                }
            }
        }

        // requested IP is invalid, find free IP
        binding = NULL;
        ip = _free_ip_get( p, vlan_ip, vlan_netmask, &binding );
        if ( ip ) {
            if ( binding ) {
                if ( pool_expired == NULL ) {
                    pool_expired    = p;
                    binding_expired = binding;
                }
            } else {
                b_get = TRUE;
                break;
            }
        }
    }

    binding = NULL;
    if ( b_get ) {
        binding = _binding_new( DHCP_SERVER_BINDING_STATE_ALLOCATED, p, message, vlan_ip, vlan_netmask, ip );
        if ( binding == NULL ) {
            T_W("fail to new a binding\n");
            return NULL;
        }
    } else if ( pool_expired ) {
        binding = binding_expired;
        if ( _old_binding_allocated( pool_expired, message, vlan_ip, vlan_netmask, binding_expired ) == FALSE ) {
            T_W("fail to move expired binding to allocated\n");
            return NULL;
        }
    }

    return binding;
}

/**
 *  \brief
 *      send DHCP NAK message
 */
static void _nak_send(
    IN  dhcp_server_message_t   *message,
    IN  vtss_vid_t              vid,
    IN  vtss_ipv4_t             vlan_ip
)
{
    dhcp_server_message_t   *send_message;
    u8                      *option;
    u32                     option_len;
    u8                      dmac[DHCP_SERVER_MAC_LEN];
    u32                     n;

    memset(g_message_buf, 0, DHCP_SERVER_MESSAGE_MAX_LEN);
    send_message = (dhcp_server_message_t *)g_message_buf;

    /* encap message */
    send_message->op     = 2;
    send_message->htype  = _HTYPE_ETHERNET;
    send_message->hlen   = DHCP_SERVER_MAC_LEN;
    send_message->xid    = message->xid;
    send_message->flags  = 0x8000;
    memcpy(send_message->chaddr, message->chaddr, DHCP_SERVER_MESSAGE_CHADDR_LEN);

    /* option */
    option = g_message_buf + DHCP_SERVER_OPTION_OFFSET;
    option_len = 0;

    /* magic cookie */
    option[option_len++] = 0x63;
    option[option_len++] = 0x82;
    option[option_len++] = 0x53;
    option[option_len++] = 0x63;

    /* message type */
    option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_MESSAGE_TYPE;
    option[option_len++] = 1;
    option[option_len++] = DHCP_SERVER_MESSAGE_TYPE_NAK;

    /* Server ID */
    option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_SERVER_ID;
    option[option_len++] = 4;
    n = htonl( vlan_ip );
    memcpy(&(option[option_len]), &n, 4);
    option_len += 4;

    /* END */
    option[option_len++] = DHCP_SERVER_MESSAGE_OPTION_CODE_END;

#if 0 /* CP, 05/28/2013 14:44, CC-11017, already in 2-byte alignment */
    /* PAD */
    if ( option_len % 2 ) {
        option_len++;
    }
#endif

    /* client IP and MAC */
    /*
        RFC-2131 p23, always broadcast DHCPNAK when giaddr = 0
    */
    memset( dmac, 0xFF, DHCP_SERVER_MAC_LEN );

    /* send message */
    if ( dhcp_server_packet_tx(send_message, option_len, vid, vlan_ip, dmac, INADDR_BROADCAST) ) {
        STATISTICS_INC( nak_cnt );
    } else {
        T_W("fail to send NAK message\n");
    }
}

/**
 *  \brief
 *      check if the binding is for the incoming message
 */
static BOOL _binding_id_check(
    IN  dhcp_server_message_t     *message,
    IN  dhcp_server_binding_t     *binding
)
{
    u8      identifier[DHCP_SERVER_CLIENT_IDENTIFIER_LEN + 4];
    u32     length;

    // check client identifier first
    memset(identifier, 0, sizeof(identifier));

    length = DHCP_SERVER_CLIENT_IDENTIFIER_LEN + 4;
    if ( vtss_dhcp_server_message_option_get(message, DHCP_SERVER_MESSAGE_OPTION_CODE_CLIENT_ID, identifier, &length) ) {
        // get cllient identifer
        switch ( identifier[0] ) {
        case DHCP_SERVER_MESSAGE_CLIENT_IDENTIFIER_TYPE_FQDN:
            if ( binding->identifier.type == DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_FQDN ) {
                if ( strcmp(binding->identifier.u.fqdn, (char *)(identifier + 1)) == 0 ) {
                    return TRUE;
                }
            }
            break;

        case DHCP_SERVER_MESSAGE_CLIENT_IDENTIFIER_TYPE_MAC:
            if ( binding->identifier.type == DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_MAC ) {
                if ( memcmp(binding->identifier.u.mac.addr, &(identifier[1]), sizeof(binding->identifier.u.mac.addr)) == 0 ) {
                    return TRUE;
                }
            }
            break;

        default:
            T_W("Client ID type %u not supported\n", identifier[0]);
            break;
        }
        return FALSE;
    }

    // then, check chaddr
    if ( message->htype != _HTYPE_ETHERNET || message->hlen != DHCP_SERVER_MAC_LEN ) {
        T_W("hardware type %02x not supported\n", message->htype);
        return FALSE;
    }

    if ( memcmp(binding->chaddr.addr, message->chaddr, DHCP_SERVER_MAC_LEN) ) {
        return FALSE;
    }
    return TRUE;
}

/**
 *  \brief
 *      get destination MAC and IP to send DHCP message
 *      This follows RFC-2131 p23
 */
static void _destination_get(
    IN  dhcp_server_message_t   *message,
    IN  dhcp_server_binding_t   *binding,
    OUT vtss_ipv4_t             *dip,
    OUT u8                      *dmac
)
{
    if ( message->ciaddr ) {
        /*
            if ciaddr != 0 then unicast ciaddr
        */
        *dip = ntohl( message->ciaddr );
        memcpy( dmac, message->chaddr, DHCP_SERVER_MAC_LEN );
    } else {
        /*
            if ciaddr == 0
            then if broadcast bit is set
                    then broadcast
                    else unicast yiaddr
        */
        if ( message->flags & DHCP_SERVER_MESSAGE_FLAG_BROADCAST ) {
            *dip = INADDR_BROADCAST;
            memset( dmac, 0xFF, DHCP_SERVER_MAC_LEN );
        } else {
            *dip = binding->ip;
            memcpy( dmac, message->chaddr, DHCP_SERVER_MAC_LEN );
        }
    }
}

/**
 *  \brief
 *      send DHCP ACK message
 */
static void _ack_send(
    IN  dhcp_server_message_t   *message,
    IN  vtss_vid_t              vid,
    IN  vtss_ipv4_t             vlan_ip,
    IN  dhcp_server_binding_t   *binding,
    IN  BOOL                    b_lease
)
{
    dhcp_server_message_t   *send_message;
    dhcp_server_pool_t      *pool;
    u32                     option_len;
    vtss_ipv4_t             dip;
    u8                      dmac[DHCP_SERVER_MAC_LEN];

    /* pool */
    pool = (dhcp_server_pool_t *)(binding->pool_name);

    /* pack message to send */
    memset(g_message_buf, 0, DHCP_SERVER_MESSAGE_MAX_LEN);
    send_message = (dhcp_server_message_t *)g_message_buf;

    /* options */
    option_len = _option_pack( message, DHCP_SERVER_MESSAGE_TYPE_ACK, pool, vlan_ip, b_lease, (u8 *) & (send_message->options) );
    if ( option_len == 0 ) {
        if ( b_lease ) {
            option_len = _default_option_pack(DHCP_SERVER_MESSAGE_TYPE_ACK, pool, vlan_ip, (u8 *) & (send_message->options) );
        } else {
            T_W("fail to pack options\n");
            return;
        }
    }

    /* encap message */
    send_message->op     = 2;
    send_message->htype  = _HTYPE_ETHERNET;
    send_message->hlen   = DHCP_SERVER_MAC_LEN;
    send_message->hops   = 0;
    send_message->xid    = message->xid;
    send_message->secs   = 0;
    send_message->ciaddr = 0;
    send_message->yiaddr = b_lease ? htonl( binding->ip ) : 0;
    send_message->siaddr = 0;
    send_message->flags  = 0;
    send_message->giaddr = 0;
    memcpy(send_message->chaddr, message->chaddr, DHCP_SERVER_MESSAGE_CHADDR_LEN);
    // sname, not supported
    // file,  not supported

    /* get destination IP and MAC */
    _destination_get( message, binding, &dip, dmac );

    /* send message */
    if ( dhcp_server_packet_tx(send_message, option_len, vid, vlan_ip, dmac, dip) ) {
        STATISTICS_INC( ack_cnt );
    } else {
        if ( b_lease ) {
            _ip_binding_expired( dip );
        }
        T_W("fail to send ACK message\n");
    }
}

/**
 *  \brief
 *      process DHCPREQUEST message for OFFER
 *
 *  \return
 *      n/a.
 */
static void _request_by_old_binding(
    IN  dhcp_server_message_t   *message,
    IN  vtss_vid_t              vid,
    IN  vtss_ipv4_t             vlan_ip,
    IN  vtss_ipv4_t             vlan_netmask,
    IN  vtss_ipv4_t             client_ip
)
{
    dhcp_server_binding_t     binding;
    dhcp_server_binding_t     *bp;

    // check if ip is still valid
    // if VLAN IP interface is changed then this IP should be invalid
    if ( _ip_in_subnet(client_ip, vlan_ip, vlan_netmask) == FALSE ) {
        // send NAK to renew IP
        // ip is not in subnet, client can get this NAK? haha...
        _nak_send(message, vid, vlan_ip);
        return;
    }

    // check client identifier first
    memset(&binding, 0, sizeof(binding));
    binding.ip = client_ip;
    bp = &binding;
    if ( vtss_avl_tree_get(&g_binding_ip_avlt, (void **)&bp, VTSS_AVL_TREE_GET) == FALSE ) {
        _nak_send(message, vid, vlan_ip);
        return;
    }

    // if the client is valid
    if ( _binding_id_check(message, bp) == FALSE ) {
        /* this binding is not for the client, it means the requested IP is used by other client */
        _nak_send(message, vid, vlan_ip);
        return;
    }

    // update vid
    bp->vid = vid;

    // move to committed state
    _binding_committed_move( bp );

    // send DHCP ACK
    _ack_send( message, vid, vlan_ip, bp, TRUE );
}

/**
 *  \brief
 *      Get DHCP message type.
 *
 *  \param
 *      message [IN] : DHCP message.
 *      type    [OUT]: message type, DHCP_SERVER_MESSAGE_TYPE_XXX
 *
 *  \return
 *      TRUE  : successful.\n
 *      FALSE : failed.
 */
static BOOL _pkt_type_get(
    IN      dhcp_server_message_t   *message,
    OUT     u8                      *type
)
{
    u32     length = 1;

    if ( vtss_dhcp_server_message_option_get(message, DHCP_SERVER_MESSAGE_OPTION_CODE_MESSAGE_TYPE, type, &length) == FALSE ) {
        return FALSE;
    }
    return TRUE;
}

/**
 *  \brief
 *      send DHCP OFFER message
 */
static void _offer_send(
    IN  dhcp_server_message_t   *message,
    IN  vtss_vid_t              vid,
    IN  vtss_ipv4_t             vlan_ip,
    IN  dhcp_server_binding_t   *binding
)
{
    dhcp_server_message_t   *send_message;
    dhcp_server_pool_t      *pool;
    u32                     option_len;
    vtss_ipv4_t             yiaddr;
    vtss_ipv4_t             dip;
    u8                      dmac[DHCP_SERVER_MAC_LEN];

    // client IP
    yiaddr = binding->ip;

    /* pool */
    pool = (dhcp_server_pool_t *)(binding->pool_name);

    /* pack message to send */
    memset(g_message_buf, 0, DHCP_SERVER_MESSAGE_MAX_LEN);
    send_message = (dhcp_server_message_t *)g_message_buf;

    /* options */
    option_len = _option_pack( message, DHCP_SERVER_MESSAGE_TYPE_OFFER, pool, vlan_ip, TRUE, (u8 *) & (send_message->options) );
    if ( option_len == 0 ) {
        _ip_binding_expired( yiaddr );
        T_W("fail to pack options\n");
        return;
    }

    /* encap message */
    send_message->op     = 2;
    send_message->htype  = _HTYPE_ETHERNET;
    send_message->hlen   = DHCP_SERVER_MAC_LEN;
    send_message->hops   = 0;
    send_message->xid    = message->xid;
    send_message->secs   = 0;
    send_message->ciaddr = 0;
    send_message->yiaddr = htonl( yiaddr );
    send_message->siaddr = 0;
    send_message->flags  = 0;
    send_message->giaddr = 0;
    memcpy(send_message->chaddr, message->chaddr, DHCP_SERVER_MESSAGE_CHADDR_LEN);
    // sname, not supported
    // file,  not supported

    /* get destination IP and MAC */
    _destination_get( message, binding, &dip, dmac );

    /* send message */
    if ( dhcp_server_packet_tx(send_message, option_len, vid, vlan_ip, dmac, dip) == FALSE ) {
        _ip_binding_expired( yiaddr );
        T_W("fail to send OFFER message\n");
        return;
    }

    // binding VID
    binding->vid = vid;

    STATISTICS_INC( offer_cnt );
}

/**
 *  \brief
 *      process DHCP DISCOVER message
 *
 *  \param
 *      message      [IN]: DHCP DISCOVER message.
 *      vid          [IN]: VLAN ID of the message.
 *      vlan_ip      [IN]: IP address of the VLAN
 *      vlan_netmask [IN]: subnet mask of the VLAN
 *
 *  \return
 *      n/a.
 */
static void _dhcp_discover(
    IN  dhcp_server_message_t   *message,
    IN  vtss_vid_t              vid,
    IN  vtss_ipv4_t             vlan_ip,
    IN  vtss_ipv4_t             vlan_netmask
)
{
    dhcp_server_binding_t       *binding;

    /* yiaddr */
    binding = _binding_get_old( message, vlan_ip, vlan_netmask );
    if ( binding ) {
        /* move binding to allocated state */
        _binding_allocated_move( binding );
    } else {
        /* get a new binding */
        binding = _binding_get_new( message, vlan_ip, vlan_netmask );
        if ( binding == NULL ) {
            return;
        }
    }

    // send OFFER
    _offer_send( message, vid, vlan_ip, binding );
}

/**
 *  \brief
 *      process DHCP REQUEST message
 *
 *  \param
 *      message      [IN]: DHCP REQUEST message.
 *      vid          [IN]: VLAN ID of the message.
 *      vlan_ip      [IN]: IP address of the VLAN
 *      vlan_netmask [IN]: subnet mask of the VLAN
 *
 *  \return
 *      n/a.
 */
static void _dhcp_request(
    IN  dhcp_server_message_t   *message,
    IN  vtss_vid_t              vid,
    IN  vtss_ipv4_t             vlan_ip,
    IN  vtss_ipv4_t             vlan_netmask
)
{
    u32             length;
    vtss_ipv4_t     server_id;
    vtss_ipv4_t     requested_ip;

    // get Server ID
    length    = 4;
    server_id = 0;
    if ( vtss_dhcp_server_message_option_get(message, DHCP_SERVER_MESSAGE_OPTION_CODE_SERVER_ID, &server_id, &length) ) {
        // the request is for offer, check if it is for us
        if ( vlan_ip != server_id ) {
            // clear binding by allocation timer, so do nothing here
            return;
        }
    }

    // get requested IP
    length       = 4;
    requested_ip = 0;
    if ( vtss_dhcp_server_message_option_get(message, DHCP_SERVER_MESSAGE_OPTION_CODE_REQUESTED_IP, &requested_ip, &length) == FALSE ) {
        requested_ip = ntohl( message->ciaddr );
    }

    if ( requested_ip == 0 ) {
        T_W("Both of requested IP and ciaddr not exist\n");
        return;
    }

    _request_by_old_binding(message, vid, vlan_ip, vlan_netmask, requested_ip);
}

/**
 *  \brief
 *      process DHCP DECLINE message
 *
 *  \param
 *      message [IN]: DHCP DECLINE message.
 *
 *  \return
 *      n/a.
 */
static void _dhcp_decline(
    IN  dhcp_server_message_t   *message,
    IN  vtss_ipv4_t             vlan_ip,
    IN  vtss_ipv4_t             vlan_netmask
)
{
    dhcp_server_binding_t   binding;
    dhcp_server_binding_t   *bp;
    vtss_ipv4_t             ip;
    vtss_ipv4_t             *ipp;
    BOOL                    b_get;

    memset(&binding, 0, sizeof(binding));

    binding.ip          = vlan_ip;
    binding.subnet_mask = vlan_netmask;
    _binding_id_encap( message, &binding );

    /* get binding by chaddr */
    b_get = FALSE;
    bp = &binding;
    if ( bp->identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
        if ( vtss_avl_tree_get(&g_binding_id_avlt, (void **)&bp, VTSS_AVL_TREE_GET) ) {
            b_get = TRUE;
        }
    } else if ( _not_empty_mac(&(bp->chaddr)) ) {
        if ( vtss_avl_tree_get(&g_binding_chaddr_avlt, (void **)&bp, VTSS_AVL_TREE_GET) ) {
            b_get = TRUE;
        }
    }

    if ( b_get == FALSE ) {
        return;
    }

    ip = bp->ip;

    dhcp_server_syslog("DHCP Server: IP %u.%u.%u.%u is declined.\n",
                       (ip & 0xff000000) >> 24,
                       (ip & 0x00ff0000) >> 16,
                       (ip & 0x0000ff00) >>  8,
                       (ip & 0x000000ff) >>  0);

    /* remove binding */
    _binding_remove( bp );

    /* add decline IP */
    ipp = (vtss_ipv4_t *)vtss_free_list_malloc( &g_decline_flist );
    if ( ipp == NULL ) {
        T_W("no free memory in g_decline_flist\n");
        return;
    }

    *ipp = ip;

    if ( vtss_avl_tree_add(&g_decline_ip_avlt, (void *)ipp) == FALSE ) {
        vtss_free_list_free( &g_decline_flist, ipp );
        //T_W("g_decline_ip_avlt is full\n");
        return;
    }

    STATISTICS_INC( declined_cnt );
}

/**
 *  \brief
 *      process DHCP RELEASE message
 *
 *  \param
 *      message [IN]: DHCP RELEASE message.
 *
 *  \return
 *      n/a.
 */
static void _dhcp_release(
    IN dhcp_server_message_t    *message,
    IN  vtss_ipv4_t             vlan_ip,
    IN  vtss_ipv4_t             vlan_netmask
)
{
    dhcp_server_binding_t       binding;
    dhcp_server_binding_t       *bp;
    BOOL                        b_get;

    memset(&binding, 0, sizeof(binding));

    binding.ip          = vlan_ip;
    binding.subnet_mask = vlan_netmask;
    _binding_id_encap( message, &binding );

    /* move binding to expired */
    b_get = FALSE;
    bp = &binding;
    if ( bp->identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
        if ( vtss_avl_tree_get(&g_binding_id_avlt, (void **)&bp, VTSS_AVL_TREE_GET) ) {
            b_get = TRUE;
        }
    } else if ( _not_empty_mac(&(bp->chaddr)) ) {
        if ( vtss_avl_tree_get(&g_binding_chaddr_avlt, (void **)&bp, VTSS_AVL_TREE_GET) ) {
            b_get = TRUE;
        }
    }

    /* move binding to expired avlt */
    if ( b_get ) {
        _binding_expired_move( bp );
    }
}

/**
 *  \brief
 *      process DHCP INFORM message
 *
 *  \param
 *      message      [IN]: DHCP INFORM message.
 *      vid          [IN]: VLAN ID of the message.
 *      vlan_ip      [IN]: IP address of the VLAN
 *
 *  \return
 *      n/a.
 */
static void _dhcp_inform(
    IN  dhcp_server_message_t   *message,
    IN  vtss_vid_t              vid,
    IN  vtss_ipv4_t             vlan_ip
)
{
    u32                     ciaddr;
    dhcp_server_binding_t   binding;
    dhcp_server_binding_t   *bp;

    /* get ciaddr */
    ciaddr = ntohl( message->ciaddr );

    /* find pool */
    // check client identifier first
    memset(&binding, 0, sizeof(binding));
    binding.ip = ciaddr;
    bp = &binding;
    if ( vtss_avl_tree_get(&g_binding_ip_avlt, (void **)&bp, VTSS_AVL_TREE_GET) == FALSE ) {
        return;
    }

    // if the client is valid
    if ( _binding_id_check(message, bp) == FALSE ) {
        /* this binding is not for the client, it means the requested IP is used by other client */
        return;
    }

    // send DHCP ACK
    _ack_send( message, vid, vlan_ip, bp, FALSE );
}

/**
 *  \brief
 *      process DHCP message from DHCP client.
 *
 *  \param
 *      message [IN]: DHCP message
 *      vid     [IN]: VLAN ID of the message.
 *
 *  \return
 *      TRUE  : successful.\n
 *      FALSE : failed.
 */
static BOOL _message_process(
    IN  dhcp_server_message_t   *message,
    IN  vtss_vid_t              vid
)
{
    vtss_ipv4_t     vlan_ip;
    vtss_ipv4_t     vlan_netmask;
    u8              message_type;

    /* check if the VLAN has IP interface. if yes, then get IP and netmask */
    if ( dhcp_server_vid_info_get(vid, &vlan_ip, &vlan_netmask) == FALSE ) {
        return FALSE;
    }

    /* DHCP client message process */
    if ( _pkt_type_get(message, &message_type) == FALSE ) {
        T_W("fail to get message type\n");
        return FALSE;
    }

    switch ( message_type ) {
    case DHCP_SERVER_MESSAGE_TYPE_DISCOVER:
        STATISTICS_INC( discover_cnt );
        _dhcp_discover( message, vid, vlan_ip, vlan_netmask );
        break;

    case DHCP_SERVER_MESSAGE_TYPE_REQUEST:
        STATISTICS_INC( request_cnt );
        _dhcp_request( message, vid, vlan_ip, vlan_netmask );
        break;

    case DHCP_SERVER_MESSAGE_TYPE_DECLINE:
        STATISTICS_INC( decline_cnt );
        _dhcp_decline( message, vlan_ip, vlan_netmask );
        break;

    case DHCP_SERVER_MESSAGE_TYPE_RELEASE:
        STATISTICS_INC( release_cnt );
        _dhcp_release( message, vlan_ip, vlan_netmask );
        break;

    case DHCP_SERVER_MESSAGE_TYPE_INFORM:
        STATISTICS_INC( inform_cnt );
        _dhcp_inform( message, vid, vlan_ip );
        break;

    default:
        T_W("invalid message type %u\n", message_type);

    // fall through

    case DHCP_SERVER_MESSAGE_TYPE_OFFER:
    case DHCP_SERVER_MESSAGE_TYPE_ACK:
    case DHCP_SERVER_MESSAGE_TYPE_NAK:
        return FALSE;
    }

    return TRUE;
}

/**
 *  \brief
 *      Enable/Dsiable DHCP server.
 *
 *  \param
 *      b_enable [IN]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      TRUE  : successful.\n
 *      FALSE : failed.
 */
static BOOL _enable(
    IN  BOOL    b_enable
)
{
    if ( g_enable != b_enable ) {
        if ( b_enable ) {
            dhcp_server_packet_rx_register();
        } else {
            dhcp_server_packet_rx_deregister();
        }
        g_enable = b_enable;
    }

    /* move all binding to expired */
    _binding_expired_move_all();

    return TRUE;
}

/**
 *  \brief
 *      remove binding by pool name
 *
 *  \param
 *      pool [IN]: pool->pool_name
 *
 *  \return
 *      n/a
 */
static void _binding_remove_by_pool_name(
    IN  dhcp_server_pool_t      *pool
)
{
    dhcp_server_binding_t       binding;
    dhcp_server_binding_t       *bp;

    memset( &binding, 0, sizeof(binding) );
    binding.pool_name = pool->pool_name;
    bp = &binding;
    while ( vtss_avl_tree_get(&g_binding_name_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
        if ( bp->pool_name != binding.pool_name ) {
            break;
        }
        _binding_remove( bp );
    }
}

/**
 *  \brief
 *      Delete DHCP pool
 *      the memory is the real memory for delete
 *
 *  \param
 *      pool [IN]: DHCP pool to be deleted.
 *
 *  \return
 *      n/a.
 */
static void _pool_delete(
    IN  dhcp_server_pool_t     *pool
)
{
    dhcp_server_pool_t        *p;

    /* remove from avlt */
    p = pool;
    (void)vtss_avl_tree_delete(&g_pool_name_avlt,   (void **)&p );

    if ( p->type != DHCP_SERVER_POOL_TYPE_NONE ) {
        if ( p->type == DHCP_SERVER_POOL_TYPE_HOST ) {
            if ( p->client_identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
                (void)vtss_avl_tree_delete(&g_pool_id_avlt, (void **)&p);
            } else if ( _not_empty_mac(&(p->client_haddr)) ) {
                (void)vtss_avl_tree_delete(&g_pool_chaddr_avlt, (void **)&p);
            }
        } else {
            (void)vtss_avl_tree_delete(&g_pool_ip_avlt, (void **)&p);
        }
    }

    /* delete all related bindings */
    _binding_remove_by_pool_name( p );

    // free pool
    vtss_free_list_free(&g_pool_flist, pool);

    STATISTICS_DEC( pool_cnt );
}

/**
 *  \brief
 *      move binding to committed state.
 */
static void _committed_binding_update_by_pool(
    INOUT dhcp_server_binding_t     *binding,
    IN    dhcp_server_pool_t        *new_pool
)
{
    // remove from lease and expired
    (void)vtss_avl_tree_delete(&g_binding_lease_avlt, (void **)&binding);
    (void)vtss_avl_tree_delete(&g_binding_expired_avlt, (void **)&binding);

    _binding_type_statistic_dec( binding->type );

    // update state
    binding->state         = DHCP_SERVER_BINDING_STATE_COMMITTED;
    binding->type          = _binding_type_get( new_pool );
    binding->lease         = new_pool->lease;
    binding->time_to_start = dhcp_server_current_time_get();
    binding->expire_time   = binding->time_to_start + binding->lease;

    _binding_type_statistic_inc( binding->type );

    // add into expired
    if ( binding->lease ) {
        if ( vtss_avl_tree_add(&g_binding_lease_avlt, (void *)binding) == FALSE ) {
            T_W("Full in g_binding_lease_avlt\n");
        }
    }
}

/**
 *  \brief
 *      remove binding by IP address
 *
 *  \param
 *      ip [IN]: binding IP address
 *
 *  \return
 *      n/a
 */
static void _binding_remove_by_ip(
    IN  vtss_ipv4_t     ip
)
{
    dhcp_server_binding_t       binding;
    dhcp_server_binding_t       *bp;

    memset( &binding, 0, sizeof(binding) );
    binding.ip = ip;
    bp = &binding;
    if ( vtss_avl_tree_get(&g_binding_ip_avlt, (void **)&bp, VTSS_AVL_TREE_GET) ) {
        _binding_remove( bp );
    }
}

/**
 *  \brief
 *      remove binding those are the same client identifier with pool
 *      and bindging IP address in the pool
 *
 *  \param
 *      pool [IN]: client identifier and subnet address
 *
 *  \return
 *      n/a
 */
static void _binding_remove_by_pool_client_id(
    IN  dhcp_server_pool_t      *pool
)
{
    dhcp_server_binding_t       binding;
    dhcp_server_binding_t       *bp;

    if ( pool->type != DHCP_SERVER_POOL_TYPE_HOST ) {
        return;
    }

    if ( pool->client_identifier.type == DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
        return;
    }

    memset( &binding, 0, sizeof(binding) );
    memcpy( &(binding.identifier), &(pool->client_identifier), sizeof(binding.identifier) );
    bp = &binding;
    while ( vtss_avl_tree_get(&g_binding_id_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
        if ( _client_identifier_cmp(&(binding.identifier), &(bp->identifier)) != VTSS_AVL_TREE_CMP_RESULT_A_B_SAME ) {
            // not same client ID, so break the loop
            break;
        }
        if ( (pool->ip & bp->subnet_mask) == (bp->ip & bp->subnet_mask) ) {
            _binding_remove( bp );
        }
    }
}

/**
 *  \brief
 *      remove binding those are the same client hardware address with pool
 *      and bindging IP address in the pool
 *
 *  \param
 *      pool [IN]: hardware address and subnet address
 *
 *  \return
 *      n/a
 */
static void _binding_remove_by_pool_chaddr(
    IN  dhcp_server_pool_t      *pool
)
{
    dhcp_server_binding_t       binding;
    dhcp_server_binding_t       *bp;

    if ( pool->type != DHCP_SERVER_POOL_TYPE_HOST ) {
        return;
    }

    if ( pool->client_identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
        // using client ID, not mac
        return;
    }

    if ( _not_empty_mac( &(pool->client_haddr) ) == FALSE ) {
        // mac is empty
        return;
    }

    memset( &binding, 0, sizeof(binding) );
    memcpy( binding.chaddr.addr, pool->client_haddr.addr, DHCP_SERVER_MAC_LEN );
    bp = &binding;
    while ( vtss_avl_tree_get(&g_binding_chaddr_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
        if ( memcmp(binding.chaddr.addr, bp->chaddr.addr, DHCP_SERVER_MAC_LEN) ) {
            // not same MAC, so break the loop
            break;
        }
        if ( (pool->ip & bp->subnet_mask) == (bp->ip & bp->subnet_mask) ) {
            _binding_remove( bp );
        }
    }
}

/**
 *  \brief
 *      update bindings by old pool to new pool.
 *
 *  \param
 *      new_pool  [IN]: new DHCP pool.
 *      old_pool  [IN]: old DHCP pool.
 *
 *  \return
 *      n/a.
 */
static void _binding_update_by_pool(
    IN    dhcp_server_pool_t    *new_pool,
    INOUT dhcp_server_pool_t    *old_pool
)
{
    dhcp_server_binding_t       binding;
    dhcp_server_binding_t       *bp;
    int                         diff;
    BOOL                        b_remove_old;

    /*
        1. remove bindings for old pool
    */

    // new pool type is None, remove all old bindings
    if ( new_pool->type == DHCP_SERVER_POOL_TYPE_NONE ) {
        _binding_remove_by_pool_name( old_pool );
        return;
    }

    /*
        2. remove corresponding bindings
    */

    if ( old_pool->type == new_pool->type ) {
        b_remove_old = FALSE;
        /* 1. remove bindings for old pool */
        if ( (old_pool->subnet_mask != new_pool->subnet_mask) ||
             ((old_pool->ip & old_pool->subnet_mask) != (new_pool->ip & new_pool->subnet_mask)) ||
             ((old_pool->type == DHCP_SERVER_POOL_TYPE_HOST) && (old_pool->ip != new_pool->ip)) ) {
            _binding_remove_by_pool_name( old_pool );
            b_remove_old = TRUE;
        }

        /* 2. remove bindings for new pool */
        if ( new_pool->type == DHCP_SERVER_POOL_TYPE_HOST ) {
            // ip
            if ( old_pool->ip != new_pool->ip ) {
                // remove ip to prepare for new
                _binding_remove_by_ip( new_pool->ip );

                if ( new_pool->client_identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
                    _binding_remove_by_pool_client_id( new_pool );
                } else {
                    _binding_remove_by_pool_chaddr( new_pool );
                }
            }

            // client identifier
            diff = memcmp( &(old_pool->client_identifier), &(new_pool->client_identifier), sizeof(old_pool->client_identifier) );
            if ( diff ) {
                // delete old binding
                if ( ! b_remove_old ) {
                    // remove old
                    _binding_remove_by_ip( old_pool->ip );
                }

                // delete binding to prepare for new
                if ( new_pool->client_identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
                    _binding_remove_by_pool_client_id( new_pool );
                }
            }

            // chaddr
            diff = memcmp( &(old_pool->client_haddr), &(new_pool->client_haddr), sizeof(old_pool->client_haddr) );
            if ( diff ) {
                // delete old binding
                if ( ! b_remove_old ) {
                    // remove old
                    _binding_remove_by_ip( old_pool->ip );
                }

                // delete binding of other pool
                _binding_remove_by_pool_chaddr( new_pool );
            }
        }
    } else {
        /* 1. remove bindings for old pool */
        _binding_remove_by_pool_name( old_pool );

        /* 2. remove bindings for new pool */
        if ( new_pool->type == DHCP_SERVER_POOL_TYPE_HOST ) {
            // remove ip to prepare for new
            _binding_remove_by_ip( new_pool->ip );

            if ( new_pool->client_identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
                _binding_remove_by_pool_client_id( new_pool );
            } else {
                _binding_remove_by_pool_chaddr( new_pool );
            }
        }
    }

    /*
        3. if lease time is changed, then update lease time and expired time
    */

    if ( old_pool->lease != new_pool->lease ) {
        memset(&binding, 0, sizeof(binding));
        binding.pool_name = old_pool->pool_name;
        bp = &binding;
        while ( vtss_avl_tree_get(&g_binding_name_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
            if ( bp->pool_name != binding.pool_name ) {
                break;
            }
            if ( bp->state == DHCP_SERVER_BINDING_STATE_COMMITTED ) {
                _committed_binding_update_by_pool( bp, new_pool );
            }
        }
    }
}

/**
 *  \brief
 *      count the number of IP addresses in the pool
 */
static void _pool_total_cnt_get(
    OUT dhcp_server_pool_t    *pool
)
{
    switch ( pool->type ) {
    case DHCP_SERVER_POOL_TYPE_NONE:
    default:
        pool->total_cnt = 0;
        break;

    case DHCP_SERVER_POOL_TYPE_NETWORK:
        pool->total_cnt = ( ~(pool->subnet_mask) ) + 1;
        break;

    case DHCP_SERVER_POOL_TYPE_HOST:
        pool->total_cnt = 1;
        break;
    }
}

/**
 *  \brief
 *      update old pool to new pool.
 *
 *  \param
 *      new_pool  [IN]: new DHCP pool.
 *      old_pool  [IN]: old DHCP pool.
 *      old_pool [OUT]: update to new DHCP pool.
 *
 *  \return
 *      TRUE  : successful
 *      FALSE : failed
 */
static BOOL _pool_update(
    IN    dhcp_server_pool_t    *new_pool,
    INOUT dhcp_server_pool_t    *old_pool
)
{
    dhcp_server_binding_t       binding;
    dhcp_server_binding_t       *bp;

    // remove old from avlt
    switch ( old_pool->type ) {
    case DHCP_SERVER_POOL_TYPE_NONE:
    default:
        break;

    case DHCP_SERVER_POOL_TYPE_NETWORK:
        (void)vtss_avl_tree_delete(&g_pool_ip_avlt, (void **)&old_pool);
        break;

    case DHCP_SERVER_POOL_TYPE_HOST:
        if ( old_pool->client_identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
            (void)vtss_avl_tree_delete(&g_pool_id_avlt, (void **)&old_pool);
        } else if ( _not_empty_mac(&(old_pool->client_haddr)) ) {
            (void)vtss_avl_tree_delete(&g_pool_chaddr_avlt, (void **)&old_pool);
        }
        break;
    }

    // update binding
    _binding_update_by_pool( new_pool, old_pool );

    // update pool
    *old_pool = *new_pool;

    // get new total count
    _pool_total_cnt_get( old_pool );

    // update alloc count
    old_pool->alloc_cnt = 0;
    memset( &binding, 0, sizeof(binding) );
    binding.pool_name = old_pool->pool_name;
    bp = &binding;
    while ( vtss_avl_tree_get(&g_binding_name_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
        if ( bp->pool_name != binding.pool_name ) {
            break;
        }
        if ( bp->state != DHCP_SERVER_BINDING_STATE_EXPIRED ) {
            ++( old_pool->alloc_cnt );
        }
    }

    // add new into avlt
    switch ( old_pool->type ) {
    case DHCP_SERVER_POOL_TYPE_NONE:
    default:
        break;

    case DHCP_SERVER_POOL_TYPE_NETWORK:
        if ( vtss_avl_tree_add(&g_pool_ip_avlt, (void *)old_pool) == FALSE ) {
            T_W("network duplicate\n");
            return FALSE;
        }
        break;

    case DHCP_SERVER_POOL_TYPE_HOST:
        if ( old_pool->client_identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
            if ( vtss_avl_tree_add(&g_pool_id_avlt, (void *)old_pool) == FALSE ) {
                T_W("host id duplicate\n");
                return FALSE;
            }
        } else if ( _not_empty_mac(&(old_pool->client_haddr)) ) {
            if ( vtss_avl_tree_add(&g_pool_chaddr_avlt, (void *)old_pool) == FALSE ) {
                T_W("host mac duplicate\n");
                return FALSE;
            }
        }
        break;
    }

    return TRUE;
}

/**
 *  \brief
 *      update old pool to new pool.
 *
 *  \param
 *      new_pool  [IN]: new DHCP pool.
 *      new_pool [OUT]: update to new DHCP pool.
 *
 *  \return
 *      n/a.
 */
static void _pool_new(
    INOUT dhcp_server_pool_t    *new_pool
)
{
    _pool_total_cnt_get( new_pool );

    new_pool->alloc_cnt = 0;

    STATISTICS_INC( pool_cnt );
}

static void _declined_ip_remove(
    void
)
{
    vtss_ipv4_t     *ipp;
    vtss_ipv4_t     ip;

    ip  = 0;
    ipp = &ip;

    while ( vtss_avl_tree_get(&g_decline_ip_avlt, (void **)&ipp, VTSS_AVL_TREE_GET_NEXT) ) {
        _declined_ip_delete( ipp );
    }
}

/*
==============================================================================

    Public Function

==============================================================================
*/
/**
 *  \brief
 *      Initialize DHCP server engine.
 *
 *  \param
 *      void
 *
 *  \return
 *      TRUE  : successful.\n
 *      FALSE : failed.
 */
BOOL vtss_dhcp_server_init(
    void
)
{
    /* default configuration */
    g_enable = FALSE;
    memset(&g_statistics, 0, sizeof(g_statistics));

    /* clear all VLANs */
    _VLAN_BF_CLR_ALL();

    /*
        Excluded IP list
    */
    if ( vtss_free_list_init(&g_excluded_ip_flist) == FALSE ) {
        T_W("Fail to create Free list for g_excluded_ip_flist\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_excluded_ip_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_excluded_ip_avlt\n");
        return FALSE;
    }

    /*
        Pool list
    */
    if ( vtss_free_list_init(&g_pool_flist) == FALSE ) {
        T_W("Fail to create Free list for g_pool_flist\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_pool_name_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_pool_name_avlt\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_pool_ip_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_pool_ip_avlt\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_pool_id_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_pool_id_avlt\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_pool_chaddr_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_pool_chaddr_avlt\n");
        return FALSE;
    }

    /*
        Binding list
    */
    if ( vtss_free_list_init(&g_binding_flist) == FALSE ) {
        T_W("Fail to create Free list for g_binding_flist\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_binding_ip_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_binding_ip_avlt\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_binding_id_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_binding_id_avlt\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_binding_chaddr_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_binding_chaddr_avlt\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_binding_name_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_binding_name_avlt\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_binding_lease_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_binding_lease_avlt\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_binding_expired_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_binding_expired_avlt\n");
        return FALSE;
    }

    /*
        Decline list
    */
    if ( vtss_free_list_init(&g_decline_flist) == FALSE ) {
        T_W("Fail to create Free list for g_decline_flist\n");
        return FALSE;
    }

    if ( vtss_avl_tree_init(&g_decline_ip_avlt) == FALSE ) {
        T_W("Fail to create AVL tree for g_decline_ip_avlt\n");
        return FALSE;
    }

    return TRUE;
}

/**
 *  \brief
 *      reset DHCP server engine to default configuration.
 *
 *  \param
 *      n/a.
 *
 *  \return
 *      n/a.
 */
void vtss_dhcp_server_reset_to_default(
    void
)
{
    dhcp_server_pool_t          *p;
    dhcp_server_pool_t          pool;
    dhcp_server_excluded_ip_t   excluded;
    dhcp_server_excluded_ip_t   *excluded_p;

    /* disable DHCP server */
    if ( _enable(FALSE) == FALSE ) {
        T_W("fail to disable DHCP server\n");
    }

    /* clear all VLANs */
    _VLAN_BF_CLR_ALL();

    /* free all IP pools and related bindings */
    memset(&pool, 0, sizeof(pool));
    p = &pool;
    while ( vtss_avl_tree_get(&g_pool_name_avlt, (void **)&p, VTSS_AVL_TREE_GET_NEXT) ) {
        _pool_delete( p );
    }

    /* declined IP */
    _declined_ip_remove();

    /* Excluded IP */
    memset(&excluded, 0, sizeof(excluded));
    excluded_p = &excluded;
    while ( vtss_avl_tree_get(&g_excluded_ip_avlt, (void **)&excluded_p, VTSS_AVL_TREE_GET_NEXT) ) {
        if ( vtss_avl_tree_delete(&g_excluded_ip_avlt, (void **)&excluded_p) ) {
            vtss_free_list_free(&g_excluded_ip_flist, excluded_p);
        } else {
            T_W("Fail to delete excluded IP %08x %08x\n", excluded_p->low_ip, excluded_p->high_ip);
        }
    }

    /* clear statistics */
    memset(&g_statistics, 0, sizeof(g_statistics));
}

/**
 *  \brief
 *      Actions for master down.
 *      clear binding, decline and statistics
 *
 *  \param
 *      n/a.
 *
 *  \return
 *      n/a.
 */
void vtss_dhcp_server_stat_clear(
    void
)
{
    /* move all binding to expired */
    _binding_expired_move_all();

    /* remove declined IP's */
    _declined_ip_remove();

    /* clear statistics */
    vtss_dhcp_server_statistics_clear();
}

/**
 *  \brief
 *      Enable/Disable DHCP server.
 *
 *  \param
 *      b_enable [IN]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_enable_set(
    IN  BOOL    b_enable
)
{
    dhcp_server_rc_t  rc;

    if ( _enable(b_enable) ) {
        rc = DHCP_SERVER_RC_OK;
    } else {
        rc = DHCP_SERVER_RC_ERROR;
    }

    return rc;
}

/**
 *  \brief
 *      Get if DHCP server is enabled or not.
 *
 *  \param
 *      b_enable [OUT]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_enable_get(
    OUT BOOL    *b_enable
)
{
    if ( b_enable == NULL ) {
        T_W("b_enable == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    *b_enable = g_enable;

    return DHCP_SERVER_RC_OK;
}

/**
 *  \brief
 *      Add excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      excluded [IN]: IP address range to be excluded.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_excluded_add(
    IN  dhcp_server_excluded_ip_t     *excluded
)
{
    dhcp_server_excluded_ip_t     *ep;
    dhcp_server_binding_t         binding;
    dhcp_server_binding_t         *bp;

    if ( excluded == NULL ) {
        T_W("excluded == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    if ( excluded->low_ip > excluded->high_ip ) {
        T_W("excluded->low_ip(%08x) > excluded->high_ip(%08x)\n", excluded->low_ip, excluded->high_ip);
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    if ( excluded->low_ip == 0 && excluded->high_ip == 0 ) {
        T_W("excluded->low_ip == 0 && excluded->high_ip == 0\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    if ( vtss_avl_tree_get(&g_excluded_ip_avlt, (void **)&excluded, VTSS_AVL_TREE_GET) ) {
        return DHCP_SERVER_RC_ERR_DUPLICATE;
    }

    /* get free memory */
    ep = (dhcp_server_excluded_ip_t *)vtss_free_list_malloc( &g_excluded_ip_flist );
    if ( ep == NULL ) {
        return DHCP_SERVER_RC_ERR_MEMORY;
    }

    /* add */
    *ep = *excluded;
    if ( vtss_avl_tree_add(&g_excluded_ip_avlt, (void *)ep) == FALSE ) {
        vtss_free_list_free(&g_excluded_ip_flist, ep);
        return DHCP_SERVER_RC_ERR_FULL;
    }

    /* remove binding in the new range */
    memset(&binding, 0, sizeof(binding));
    bp = &binding;
    while ( vtss_avl_tree_get(&g_binding_ip_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
        if ( _ip_in_excluded(ep, bp->ip) ) {
            _binding_remove( bp );
        }
    }

    STATISTICS_INC( excluded_cnt );

    return DHCP_SERVER_RC_OK;
}

/**
 *  \brief
 *      Delete excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      excluded [IN]: Excluded IP address range to be deleted.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_excluded_delete(
    IN  dhcp_server_excluded_ip_t     *excluded
)
{
    dhcp_server_rc_t  rc;

    if ( excluded == NULL ) {
        T_W("excluded == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* delete */
    if ( vtss_avl_tree_delete(&g_excluded_ip_avlt, (void **)&excluded) ) {
        vtss_free_list_free(&g_excluded_ip_flist, excluded);
        STATISTICS_DEC( excluded_cnt );
        rc = DHCP_SERVER_RC_OK;
    } else {
        rc = DHCP_SERVER_RC_ERR_NOT_EXIST;
    }

    return rc;
}

/**
 *  \brief
 *      Get excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      excluded [IN] : index.
 *      excluded [OUT]: Excluded IP address range data.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_excluded_get(
    INOUT  dhcp_server_excluded_ip_t     *excluded
)
{
    dhcp_server_excluded_ip_t     *p;

    if ( excluded == NULL ) {
        T_W("excluded == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    p = excluded;
    if ( vtss_avl_tree_get(&g_excluded_ip_avlt, (void **)&p, VTSS_AVL_TREE_GET) ) {
        *excluded = *p;
        return DHCP_SERVER_RC_OK;
    } else {
        return DHCP_SERVER_RC_ERR_NOT_EXIST;
    }
}

/**
 *  \brief
 *      Get next of current excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      excluded [IN] : Currnet excluded IP address range index.
 *      excluded [OUT]: Next excluded IP address range data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_excluded_get_next(
    INOUT  dhcp_server_excluded_ip_t     *excluded
)
{
    dhcp_server_excluded_ip_t     *p;

    if ( excluded == NULL ) {
        T_W("excluded == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    p = excluded;
    if ( vtss_avl_tree_get(&g_excluded_ip_avlt, (void **)&p, VTSS_AVL_TREE_GET_NEXT) ) {
        *excluded = *p;
        return DHCP_SERVER_RC_OK;
    } else {
        return DHCP_SERVER_RC_ERR_NOT_EXIST;
    }
}

/**
 *  \brief
 *      Set DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN]: new or modified DHCP pool.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_pool_set(
    IN  dhcp_server_pool_t     *pool
)
{
    dhcp_server_pool_t          *p;

    if ( pool == NULL ) {
        T_W("pool == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    if ( strlen(pool->pool_name) == 0 ) {
        T_W("strlen(pool->pool_name) == 0\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    switch ( pool->type ) {
    case DHCP_SERVER_POOL_TYPE_NONE:
    default:
        break;

    case DHCP_SERVER_POOL_TYPE_HOST:
        // avoid the first 0 address
        if ( (pool->ip & (~(pool->subnet_mask))) == 0 ) {
            return DHCP_SERVER_RC_ERR_IP;
        }

        /* broadcast IP address is not allowed */
        if ( (pool->ip | pool->subnet_mask) == 0xFFffFFff ) {
            return DHCP_SERVER_RC_ERR_IP;
        }

    // fall through

    case DHCP_SERVER_POOL_TYPE_NETWORK:
        /* check if exist in ip avlt */
        p = pool;
        if ( vtss_avl_tree_get(&g_pool_ip_avlt, (void **)&p, VTSS_AVL_TREE_GET) ) {
            /* check if the same pool */
            if ( strcmp(p->pool_name, pool->pool_name) != 0 ) {
                // different pool, not allowed
                //T_W("duplicate in g_pool_ip_avlt\n");
                return DHCP_SERVER_RC_ERR_DUPLICATE;
            }
        }
        break;
    }

    /* check if exist in name avlt */
    p = pool;
    if ( vtss_avl_tree_get(&g_pool_name_avlt, (void **)&p, VTSS_AVL_TREE_GET) ) {
        if ( _pool_update( pool, p ) == FALSE ) {
            return DHCP_SERVER_RC_ERR_DUPLICATE;
        }
        return DHCP_SERVER_RC_OK;
    }

    /* totally new one */

    /* get free memory */
    p = (dhcp_server_pool_t *)vtss_free_list_malloc( &g_pool_flist );
    if ( p == NULL ) {
        return DHCP_SERVER_RC_ERR_MEMORY;
    }

    /* add */
    *p = *pool;

    // add name avlt
    if ( vtss_avl_tree_add(&g_pool_name_avlt, (void *)p) == FALSE ) {
        vtss_free_list_free( &g_pool_flist, p );
        return DHCP_SERVER_RC_ERR_FULL;
    }

    /*
        the followings do not return FULL and just show error message only
        it is because if full, then it should happen when adding name avlt
        so if the followings are still full, then it is a bug
    */
    switch ( p->type ) {
    case DHCP_SERVER_POOL_TYPE_NONE:
    default:
        break;

    case DHCP_SERVER_POOL_TYPE_NETWORK:
        // add ip valt
        if ( vtss_avl_tree_add(&g_pool_ip_avlt, (void *)p) == FALSE ) {
            vtss_free_list_free( &g_pool_flist, p );
            return DHCP_SERVER_RC_ERR_DUPLICATE;
        }
        break;

    case DHCP_SERVER_POOL_TYPE_HOST:
        // remove ip to prepare for new
        _binding_remove_by_ip( p->ip );

        if ( p->client_identifier.type != DHCP_SERVER_CLIENT_IDENTIFIER_TYPE_NONE ) {
            // add to id AVL tree
            if ( vtss_avl_tree_add(&g_pool_id_avlt, (void *)p) == FALSE ) {
                vtss_free_list_free( &g_pool_flist, p );
                return DHCP_SERVER_RC_ERR_DUPLICATE;
            }

            // delete binding for the new pool
            _binding_remove_by_pool_client_id( p );

        } else if ( _not_empty_mac(&(p->client_haddr)) ) {
            // add to chaddr AVL tree
            if ( vtss_avl_tree_add(&g_pool_chaddr_avlt, (void *)p) == FALSE ) {
                vtss_free_list_free( &g_pool_flist, p );
                return DHCP_SERVER_RC_ERR_DUPLICATE;
            }

            // delete binding for the new pool
            _binding_remove_by_pool_chaddr( p );
        }
        break;
    }

    // new pool
    _pool_new( p );

    return DHCP_SERVER_RC_OK;
}

/**
 *  \brief
 *      Delete DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN]: DHCP pool to be deleted.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_pool_delete(
    IN  dhcp_server_pool_t     *pool
)
{
    dhcp_server_pool_t  *p;
    dhcp_server_rc_t    rc;
    BOOL                b;

    if ( pool == NULL ) {
        T_W("pool == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    p = pool;
    b = vtss_avl_tree_get(&g_pool_name_avlt, (void **)&p, VTSS_AVL_TREE_GET);
    if ( b ) {
        _pool_delete( p );
        rc = DHCP_SERVER_RC_OK;
    } else {
        rc = DHCP_SERVER_RC_ERR_NOT_EXIST;
    }

    return rc;
}

/**
 *  \brief
 *      Get DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN] : index.
 *      pool [OUT]: DHCP pool data.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_pool_get(
    INOUT  dhcp_server_pool_t     *pool
)
{
    dhcp_server_pool_t      *p;

    if ( pool == NULL ) {
        T_W("pool == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    p = pool;
    if ( vtss_avl_tree_get(&g_pool_name_avlt, (void **)&p, VTSS_AVL_TREE_GET) ) {
        *pool = *p;
        return DHCP_SERVER_RC_OK;
    } else {
        return DHCP_SERVER_RC_ERR_NOT_EXIST;
    }
}

/**
 *  \brief
 *      Get next of current DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN] : Currnet DHCP pool index.
 *      pool [OUT]: Next DHCP pool data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_pool_get_next(
    INOUT  dhcp_server_pool_t     *pool
)
{
    dhcp_server_pool_t    *p;

    if ( pool == NULL ) {
        T_W("pool == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    p = pool;
    if ( vtss_avl_tree_get(&g_pool_name_avlt, (void **)&p, VTSS_AVL_TREE_GET_NEXT) ) {
        *pool = *p;
        return DHCP_SERVER_RC_OK;
    } else {
        return DHCP_SERVER_RC_ERR_NOT_EXIST;
    }
}

/**
 *  \brief
 *      Set DHCP pool to be default value.
 *
 *  \param
 *      pool [OUT]: default DHCP pool.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_pool_default(
    IN  dhcp_server_pool_t     *pool
)
{
    if ( pool == NULL ) {
        T_W("pool == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    memset(pool, 0, sizeof(dhcp_server_pool_t));
    pool->lease = DHCP_SERVER_LEASE_DEFAULT;

    return DHCP_SERVER_RC_OK;
}

/**
 *  \brief
 *      Clear DHCP message statistics.
 *
 *  \param
 *      n/a.
 *
 *  \return
 *      n/a.
 */
void vtss_dhcp_server_statistics_clear(
    void
)
{
    g_statistics.discover_cnt = 0;
    g_statistics.offer_cnt    = 0;
    g_statistics.request_cnt  = 0;
    g_statistics.ack_cnt      = 0;
    g_statistics.nak_cnt      = 0;
    g_statistics.decline_cnt  = 0;
    g_statistics.release_cnt  = 0;
    g_statistics.inform_cnt   = 0;
}

/**
 *  \brief
 *      Get DHCP message statistics.
 *
 *  \param
 *      statistics [OUT]: DHCP message statistics data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_statistics_get(
    OUT dhcp_server_statistics_t  *statistics
)
{
    if ( statistics == NULL ) {
        T_W("statistics == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    *statistics = g_statistics;

    return DHCP_SERVER_RC_OK;
}

/**
 *  \brief
 *      Delete DHCP binding.
 *      index: ip
 *
 *  \param
 *      binding [IN]: DHCP binding to be deleted.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_binding_delete(
    IN  dhcp_server_binding_t     *binding
)
{
    if ( binding == NULL ) {
        T_W("binding == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    if ( vtss_avl_tree_get(&g_binding_ip_avlt, (void **)&binding, VTSS_AVL_TREE_GET) == FALSE ) {
        return DHCP_SERVER_RC_ERR_NOT_EXIST;
    }

    /* delete */
    switch ( binding->type ) {
    case DHCP_SERVER_BINDING_TYPE_AUTOMATIC:
    case DHCP_SERVER_BINDING_TYPE_MANUAL:
        _binding_expired_move( binding );
        break;

    case DHCP_SERVER_BINDING_TYPE_EXPIRED:
        _binding_remove( binding );
        break;

    default:
        T_W("invalid binding type : %u of ip %08x\n", binding->type, binding->ip);
        break;
    }

    return DHCP_SERVER_RC_OK;
}

/**
 *  \brief
 *      Get DHCP binding.
 *      index: ip
 *
 *  \param
 *      binding [IN] : index.
 *      binding [OUT]: DHCP binding data.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_binding_get(
    INOUT  dhcp_server_binding_t     *binding
)
{
    dhcp_server_binding_t     *bp;

    if ( binding == NULL ) {
        T_W("binding == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    bp = binding;
    if ( vtss_avl_tree_get(&g_binding_ip_avlt, (void **)&bp, VTSS_AVL_TREE_GET) ) {
        *binding = *bp;
        return DHCP_SERVER_RC_OK;
    } else {
        return DHCP_SERVER_RC_ERR_NOT_EXIST;
    }
}

/**
 *  \brief
 *      Get next of current DHCP binding.
 *      index: ip
 *
 *  \param
 *      binding [IN] : Currnet DHCP binding index.
 *      binding [OUT]: Next DHCP binding data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_binding_get_next(
    INOUT  dhcp_server_binding_t     *binding
)
{
    dhcp_server_binding_t     *bp;

    if ( binding == NULL ) {
        T_W("binding == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    bp = binding;
    if ( vtss_avl_tree_get(&g_binding_ip_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
        *binding = *bp;
        return DHCP_SERVER_RC_OK;
    } else {
        return DHCP_SERVER_RC_ERR_NOT_EXIST;
    }
}

/**
 *  \brief
 *      Clear DHCP bindings by binding type.
 *
 *  \param
 *      type - binding type
 *
 *  \return
 *      n/a.
 */
void vtss_dhcp_server_binding_clear_by_type(
    IN dhcp_server_binding_type_t     type
)
{
    dhcp_server_binding_t     binding;
    dhcp_server_binding_t     *bp;

    memset(&binding, 0, sizeof(binding));
    bp = &binding;

    while ( vtss_avl_tree_get(&g_binding_ip_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
        switch ( type ) {
        case DHCP_SERVER_BINDING_TYPE_AUTOMATIC:
        case DHCP_SERVER_BINDING_TYPE_MANUAL:
            if ( bp->type != type ) {
                break;
            }
            _binding_expired_move( bp );
            break;

        case DHCP_SERVER_BINDING_TYPE_EXPIRED:
            if ( bp->type != type ) {
                break;
            }
            _binding_remove( bp );
            break;

        default:
            T_W("invalid binding type : %u of ip %08x\n", bp->type, bp->ip);
            break;
        }
    }
}

/**
 *  \brief
 *      Enable/Disable DHCP server per VLAN.
 *
 *  \param
 *      vid      [IN]: VLAN ID
 *      b_enable [IN]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_vlan_enable_set(
    IN  vtss_vid_t      vid,
    IN  BOOL            b_enable
)
{
    dhcp_server_binding_t     binding;
    dhcp_server_binding_t     *bp;

    _VLAN_BF_SET( vid, b_enable );

    if ( b_enable == FALSE ) {
        /* move all binding of the VLAN to expired */
        memset(&binding, 0, sizeof(binding));
        bp = &binding;
        while ( vtss_avl_tree_get(&g_binding_lease_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
            if ( bp->vid == vid ) {
                _binding_expired_move( bp );
            }
        }
    }

    return DHCP_SERVER_RC_OK;
}

/**
 *  \brief
 *      Get Enable/Disable DHCP server per VLAN.
 *
 *  \param
 *      vid      [IN] : VLAN ID
 *      b_enable [OUT]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_vlan_enable_get(
    IN  vtss_vid_t      vid,
    OUT BOOL            *b_enable
)
{
    if ( b_enable == NULL ) {
        T_W("b_enable == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    *b_enable = _VLAN_BF_GET( vid );

    return DHCP_SERVER_RC_OK;
}

/**
 *  \brief
 *      process timer related function.
 *
 *  \param
 *      n/a.
 *
 *  \return
 *      n/a.
 */
void vtss_dhcp_server_timer_process(
    void
)
{
    dhcp_server_binding_t   binding;
    dhcp_server_binding_t   *bp;
    u32                     current_time;

    /* get current time in second */
    current_time = dhcp_server_current_time_get();

    /* check lease */
    memset(&binding, 0, sizeof(binding));
    bp = &binding;
    while ( vtss_avl_tree_get(&g_binding_lease_avlt, (void **)&bp, VTSS_AVL_TREE_GET_NEXT) ) {
        if ( current_time >= bp->expire_time ) {
            if ( bp->state == DHCP_SERVER_BINDING_STATE_ALLOCATED ) {
                /* the allocation is failed, so just free it */
                _binding_remove( bp );
            } else {
                /* this binding is expired */
                _binding_expired_move( bp );
            }
        } else {
            break;
        }
    }
}

/**
 *  \brief
 *      receive and process DHCP message from client.
 *
 *  \param
 *      packet      [IN]: Ethernet packet with DHCP message inside.
 *      vid         [IN]: VLAN ID.
 *
 *  \return
 *      TRUE  : DHCP server process this packet.
 *      FALSE : the packet is not processed, so pass it to other application.
 */
BOOL vtss_dhcp_server_packet_rx(
    IN const void       *packet,
    IN vtss_vid_t       vid
)
{
    u32                         ip_len;
    dhcp_server_message_t       *message;
    dhcp_server_ip_header_t     *ip_hdr;
    BOOL                        b;

    /* check global mode */
    if ( g_enable == FALSE ) {
        /*
            global is disabled.
            DHCP server does not process this packet
            and deregister to helper.
        */
        dhcp_server_packet_rx_deregister();
        return FALSE;
    }

    /* check VLAN mode */
    if ( _VLAN_BF_GET(vid) == FALSE ) {
        /*
            global enable, but this VLAN is disabled.
            DHCP server does not process this packet.
        */
        return FALSE;
    }

    /* get ip length */
    ip_hdr = (dhcp_server_ip_header_t *)( (u8 *)packet + sizeof(dhcp_server_eth_header_t) );
    ip_len = ( ip_hdr->vhl & 0x0F ) * 4;

    /* get DHCP message */
    message = (dhcp_server_message_t *)( (u8 *)packet + sizeof(dhcp_server_eth_header_t) + ip_len + sizeof(dhcp_server_udp_header_t) );

    /* not support relay agent */
    if ( message->giaddr ) {
        return FALSE;
    }

    /* support MAC address only */
    if ( message->htype != _HTYPE_ETHERNET || message->hlen != DHCP_SERVER_MAC_LEN ) {
        T_W("hardware type %02x not supported\n", message->htype);
        return FALSE;
    }

    /* process message */
    b = _message_process( message, vid );
    return b;
}

/**
 *  \brief
 *      add a declined IP. This API should be used for debug only.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : index.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_declined_add(
    IN  vtss_ipv4_t     declined_ip
)
{
    vtss_ipv4_t     *ip;

    if ( declined_ip == 0 ) {
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* add decline IP */
    ip = (vtss_ipv4_t *)vtss_free_list_malloc( &g_decline_flist );
    if ( ip == NULL ) {
        return DHCP_SERVER_RC_ERR_FULL;
    }

    *ip = declined_ip;

    if ( vtss_avl_tree_add(&g_decline_ip_avlt, (void *)ip) == FALSE ) {
        vtss_free_list_free( &g_decline_flist, ip );
        return DHCP_SERVER_RC_ERR_FULL;
    }

    STATISTICS_INC( declined_cnt );

    return DHCP_SERVER_RC_OK;
}

/**
 *  \brief
 *      delete a declined IP. This API should be used for debug only.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : index.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_declined_delete(
    IN  vtss_ipv4_t     declined_ip
)
{
    if ( declined_ip == 0 ) {
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    _declined_ip_delete( &declined_ip );
    return DHCP_SERVER_RC_OK;
}

/**
 *  \brief
 *      Get declined IP.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : index.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_declined_get(
    IN  vtss_ipv4_t     *declined_ip
)
{
    vtss_ipv4_t     *ip;

    if ( declined_ip == NULL ) {
        T_W("declined_ip == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    ip = declined_ip;
    if ( vtss_avl_tree_get(&g_decline_ip_avlt, (void **)&ip, VTSS_AVL_TREE_GET) ) {
        *declined_ip = *ip;
        return DHCP_SERVER_RC_OK;
    } else {
        return DHCP_SERVER_RC_ERR_NOT_EXIST;
    }
}

/**
 *  \brief
 *      Get next declined IP.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : Currnet declined IP.
 *      declined_ip [OUT]: Next declined IP to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t vtss_dhcp_server_declined_get_next(
    INOUT   vtss_ipv4_t     *declined_ip
)
{
    vtss_ipv4_t     *ip;

    if ( declined_ip == NULL ) {
        T_W("declined_ip == NULL\n");
        return DHCP_SERVER_RC_ERR_PARAMETER;
    }

    /* check if exist */
    ip = declined_ip;
    if ( vtss_avl_tree_get(&g_decline_ip_avlt, (void **)&ip, VTSS_AVL_TREE_GET_NEXT) ) {
        *declined_ip = *ip;
        return DHCP_SERVER_RC_OK;
    } else {
        return DHCP_SERVER_RC_ERR_NOT_EXIST;
    }
}
