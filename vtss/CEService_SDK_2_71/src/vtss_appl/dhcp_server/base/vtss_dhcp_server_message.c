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
 *      vtss_dhcp_server_pkt.c
 *
 *  \brief
 *      DHCP message decode
 *
 *  \author
 *      CP Wang
 *
 *  \date
 *      05/09/2013 11:47
 */
//----------------------------------------------------------------------------

/*
==============================================================================

    Include File

==============================================================================
*/
#include <string.h>
#include "vtss_dhcp_server_type.h"
#include "vtss_dhcp_server_message.h"
#include "vtss_dhcp_server_platform.h"

/*
==============================================================================

    Constant

==============================================================================
*/
#define _OPTION_OVERLOAD_FILE       1   /**< the 'file' field is used to hold options */
#define _OPTION_OVERLOAD_SNAME      2   /**< the 'sname' field is used to hold options */

/*
==============================================================================

    Macro

==============================================================================
*/

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

/*
==============================================================================

    Static Function

==============================================================================
*/
/**
 *  \brief
 *      get 4-byte unsigned value from option buffer.
 *
 *  \param
 *      addr   [IN] : start memory to get avlue.
 *      count  [IN] : number of 4-byte unsigned values
 *      option [OUT]: option value of code
 *      length [OUT]: length of retrieved option value
 *
 *  \return
 *      n/a.
 */
static void _option_value_get(
    IN      u8      *addr,
    IN      u32     count,
    OUT     void    *option,
    OUT     u32     *length
)
{
    u32     *value;
    u32     i;

    value = (u32 *)option;
    for ( i = 0; i < count; i++ ) {
        addr = addr + i * 4;
        *value = ((*(addr)) << 24) + ((*(addr + 1)) << 16) + ((*(addr + 2)) << 8) + (*(addr + 3));
        value++;
    }
    *length = count * 4;
}

/**
 *  \brief
 *      get value for code from option buffer.
 *
 *  \param
 *      message [IN] : DHCP message.
 *      buf     [IN] : option buffer.
 *      code    [IN] : option code, defined by DHCP_SERVER_MESSAGE_OPTION_CODE_XXX
 *      option  [OUT]: option value of code
 *      length  [IN] : memory length of option
 *              [OUT]: TRUE  - length of retrieved option value
 *                     FALSE - if 0, then it means the code does not exist,
 *                             else it means the memory length of option is not correct or not enough
 *                                  and this is the correct length
 *
 *  \return
 *      TRUE  : successful.\n
 *      FALSE : failed.
 */
static BOOL _pkt_option_get(
    IN      dhcp_server_message_t   *message,
    IN      u8                      *buf,
    IN      u8                      code,
    OUT     void                    *option,
    INOUT   u32                     *length
)
{
    u8      pcode;
    u8      len;
    u8      *addr;
    BOOL    b_same;
    u8      *v8;
    u16     *v16;
    u8      min;

    pcode  = *(buf);
    len    = *(buf + 1);
    addr   = (buf + 2);
    b_same = ( pcode == code );

    switch ( pcode ) {
    case DHCP_SERVER_MESSAGE_OPTION_CODE_SUBNET_MASK:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_BROADCAST:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_REQUESTED_IP:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_LEASE_TIME:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_SERVER_ID:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_RENEWAL_TIME:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_REBINDING_TIME:
        if ( b_same ) {
            /* check length */
            if ( len != 4 ) {
                T_E("incorrect length %u for %s\n", len,
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_SUBNET_MASK)  ? "SUBNET_MASK"  :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_BROADCAST)    ? "BROADCAST"    :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_REQUESTED_IP) ? "REQUESTED_IP" :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_LEASE_TIME)   ? "LEASE_TIME"   :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_SERVER_ID)    ? "SERVER_ID"    :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_RENEWAL_TIME) ? "RENEWAL_TIME" :
                    "REBINDING_TIME");
                return FALSE;
            }

            if ( *length != 4 ) {
                *length = len;
                return FALSE;
            }

            /* retrieve value */
            _option_value_get(addr, 1, option, length);
            return TRUE;
        }
        break;

    case DHCP_SERVER_MESSAGE_OPTION_CODE_ROUTER:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_DNS_SERVER:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_NIS_SERVER:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_NTP_SERVER:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_NAME_SERVER:
        if ( b_same ) {
            /* check length */
            if ( ( len % 4 ) != 0 ) {
                T_E("incorrect length %u for %s\n", len,
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_ROUTER)     ? "ROUTER" :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_DNS_SERVER) ? "DNS_SERVER" :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_NIS_SERVER) ? "NIS_SERVER" :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_NTP_SERVER) ? "NTP_SERVER" :
                    "NETNIOS_NAME_SERVER");
                return FALSE;
            }

            if ( *length < len ) {
                *length = len;
                return FALSE;
            }

            /* retrieve value */
            _option_value_get(addr, len / 4, option, length);
            return TRUE;
        }
        break;

    case DHCP_SERVER_MESSAGE_OPTION_CODE_HOST_NAME:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_DOMAIN_NAME:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_NIS_DOMAIN_NAME:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_PARAMETER_LIST:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_VENDOR_CLASS_ID:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_CLIENT_ID:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_SCOPE:
        if ( b_same ) {
            /* check length */
            if ( pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_CLIENT_ID ) {
                min = 2;
            } else {
                min = 1;
            }

            if ( len < min ) {
                T_E("incorrect length %u for %s\n", len,
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_HOST_NAME)       ? "HOST_NAME"       :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_DOMAIN_NAME)     ? "DOMAIN_NAME"     :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_NIS_DOMAIN_NAME) ? "NIS_DOMAIN_NAME" :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_PARAMETER_LIST)  ? "PARAMETER_LIST"  :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_VENDOR_CLASS_ID) ? "VENDOR_CLASS_ID" :
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_CLIENT_ID)       ? "CLIENT_ID"       :
                    "NETBIOS_SCOPE");
                return FALSE;
            }

            if ( *length < len ) {
                *length = len;
                return FALSE;
            }

            /* retrieve value */
            memcpy(option, addr, len);
            *length = len;
            return TRUE;
        }
        break;

    case DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_NODE_TYPE:
    case DHCP_SERVER_MESSAGE_OPTION_CODE_MESSAGE_TYPE:
        if ( b_same ) {
            /* check length */
            if ( len != 1 ) {
                T_E("incorrect length %u for %s\n", len,
                    (pcode == DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_NODE_TYPE) ? "NETBIOS_NODE_TYPE" :
                    "PKT_TYPE");
                return FALSE;
            }

            if ( *length != 1 ) {
                *length = len;
                return FALSE;
            }

            /* retrieve value */
            v8  = (u8 *)option;
            *v8 = *addr;
            return TRUE;
        }
        break;

    case DHCP_SERVER_MESSAGE_OPTION_CODE_OPTION_OVERLOAD:
        if ( b_same ) {
            /* check length */
            if ( len != 1 ) {
                T_E("incorrect length %u for OPTION_OVERLOAD\n", len);
                return FALSE;
            }

            if ( *length != 1 ) {
                *length = len;
                return FALSE;
            }

            /* overload on 'file' */
            if ( (*addr) & 0x01 ) {
                if ( _pkt_option_get(message, message->file, code, option, length) ) {
                    return TRUE;
                }
            }

            /* overload on 'sname' */
            if ( (*addr) & 0x02 ) {
                if ( _pkt_option_get(message, message->sname, code, option, length) ) {
                    return TRUE;
                }
            }
        }
        break;

    case DHCP_SERVER_MESSAGE_OPTION_CODE_MAX_MESSAGE_SIZE:
        if ( b_same ) {
            /* check length */
            if ( len != 2 ) {
                T_E("incorrect length %u for %s\n", len, "PKT_MAX_SIZE");
                return FALSE;
            }

            if ( *length != 2 ) {
                *length = len;
                return FALSE;
            }

            /* retrieve value */
            v16  = (u16 *)option;
            *v16 = (*(addr) << 8) + (*(addr + 1));
            return TRUE;
        }
        break;

    case DHCP_SERVER_MESSAGE_OPTION_CODE_END:
        /* the end, 0 means not exist */
        *length = 0;
        return FALSE;

    case DHCP_SERVER_MESSAGE_OPTION_CODE_PAD:
        /* recursively check next option */
        return _pkt_option_get(message, buf + 1, code, option, length);

    default:
        break;
    }

    /* recursively check next option */
    return _pkt_option_get(message, buf + 1 + 1 + len, code, option, length);
}

/*
==============================================================================

    Public Function

==============================================================================
*/
/**
 *  \brief
 *      Get DHCP option.
 *
 *  \param
 *      message [IN] : DHCP message.
 *      code    [IN] : option code, defined by DHCP_SERVER_MESSAGE_OPTION_CODE_XXX
 *      option  [OUT]: option value of code
 *      length  [IN] : memory length of option
 *              [OUT]: TRUE  - length of retrieved option value
 *                     FALSE - if 0, then it means the code does not exist,
 *                             else it means the memory length of option is not correct or not enough
 *                                  and this is the correct length
 *
 *  \return
 *      TRUE  : successful.\n
 *      FALSE : failed.
 */
BOOL vtss_dhcp_server_message_option_get(
    IN      dhcp_server_message_t   *message,
    IN      u8                      code,
    OUT     void                    *option,
    INOUT   u32                     *length
)
{
    u8      cookie[] = DHCP_SERVER_OPTION_MAGIC_COOKIE;
    u32     len = sizeof( cookie );
    u8      *message_options;

    if ( message == NULL ) {
        T_E("message == NULL\n");
        return FALSE;
    }

    if ( option == NULL ) {
        T_E("option == NULL\n");
        return FALSE;
    }

    if ( length == NULL ) {
        T_E("length == NULL\n");
        return FALSE;
    }

    /* check cookie */
    message_options = (u8 *) & ( message->options );
    if ( memcmp(message_options, cookie, len) != 0 ) {
        return FALSE;
    }

    /* get option value */
    if ( _pkt_option_get(message, message_options + len, code, option, length) == FALSE ) {
        return FALSE;
    }

    /* successful */
    return TRUE;
}
