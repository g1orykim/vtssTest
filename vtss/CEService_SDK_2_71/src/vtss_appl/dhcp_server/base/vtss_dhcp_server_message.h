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
 *      vtss_dhcp_server_message.h
 *
 *  \brief
 *      DHCP message format
 *
 *  \author
 *      CP Wang
 *
 *  \date
 *      05/09/2013 11:46
 */
//----------------------------------------------------------------------------
#ifndef __DHCP_SERVER_MESSAGE_H__
#define __DHCP_SERVER_MESSAGE_H__
//----------------------------------------------------------------------------

/*
==============================================================================

    Include File

==============================================================================
*/
#include <vtss_types.h>

/*
==============================================================================

    Constant

==============================================================================
*/
#define DHCP_SERVER_OPTION_MAGIC_COOKIE     { 0x63, 0x82, 0x53, 0x63 } /**< the first four octets of the vendor information field */

#define DHCP_SERVER_MESSAGE_CHADDR_LEN      16      /**< Length of client hardware address */
#define DHCP_SERVER_MESSAGE_SNAME_LEN       63      /**< Length of server host name */
#define DHCP_SERVER_MESSAGE_FILE_LEN        127     /**< Length of boot file name */
#define DHCP_SERVER_OPTION_OFFSET           236     /**< offset from the head of dhcp_server_message_t to options field */

/*
    RFC-2132
    Supported option codes
    not listed are not supported
*/
#define DHCP_SERVER_MESSAGE_OPTION_CODE_PAD                     0   /**< Pad option to cause subsequent fields to align on word boundary */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_SUBNET_MASK             1   /**< Subent mask option to specify the client's subnet mask */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_ROUTER                  3   /**< Router option to specify a list of IP addresses for routers on the client's subnet */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_DNS_SERVER              6   /**< Domain name server option to specify a list of Domain Name System name servers available to the client */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_HOST_NAME               12  /**< Host name option to specify the name of client */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_DOMAIN_NAME             15  /**< Domain name option to specify the domain name that client should use when resolving hostname via DNS */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_BROADCAST               28  /**< Broadcast option to specify the broadcast address in use on the client's subnet */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_NIS_DOMAIN_NAME         40  /**< Network information service domain option to specify the name of the client's NIS domain */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_NIS_SERVER              41  /**< Network information servers option to specify a list of IP addresses indicating NIS servers available to the client */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_NTP_SERVER              42  /**< Network time protocol option to specify a list of IP addresses indicating NTP servers available to the client */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_VENDOR_SPECIFIC_INFO    43  /**< Vendor specific information according to option 60 vendor class identifier */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_NAME_SERVER     44  /**< Netbios over TCP/IP name server option to specify a list of NBNS name servers listed in order of preference */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_NODE_TYPE       46  /**< Netbios node type option to allow Netbios over TCP/IP clients which are configurable to be configured */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_NETBIOS_SCOPE           47  /**< Netbios scope option to scope identifier */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_REQUESTED_IP            50  /**< Requested IP address to be used in a client request(DHCPDISCOVER) to allow the client to request that a particular IP address be assigned */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_LEASE_TIME              51  /**< IP address lease time to allow the client to request a lease time for the IP address */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_OPTION_OVERLOAD         52  /**< Option overload to indicate that the DHCP 'sname' or 'file' fields are being overloaded by using them to carry DHCP options */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_MESSAGE_TYPE            53  /**< DHCP message type to convey the type of the DHCP message */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_SERVER_ID               54  /**< Server identifier to be used in OFFER and REQUEST messages to allow the client to distinguish between lease offers */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_PARAMETER_LIST          55  /**< Parameter request list to be used by a DHCP client to request values for specified configuration parameters */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_MAX_MESSAGE_SIZE        57  /**< Maximum DHCP message size to specify the maximum length DHCP message that it is willing to accept */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_RENEWAL_TIME            58  /**< Renewal (T1) time value to specify the time interval from address assignment until the client transitions to the RENEWING state */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_REBINDING_TIME          59  /**< Rebinding (T2) time value to specify the time interval from address assignment until the client transitions to the REBINDING state */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_VENDOR_CLASS_ID         60  /**< Vendor class identifier to be used by DHCP client to optionally identify the vendor type and configuration of a DHCP client */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_CLIENT_ID               61  /**< Client identifier to be used by DHCP clients to specify their unique identifier */
#define DHCP_SERVER_MESSAGE_OPTION_CODE_END                     255 /**< End option to mark the end of valid information in the option field */

#define DHCP_SERVER_OPTION_IS_(op, c)       ( DHCP_SERVER_MESSAGE_OPTION_CODE_##op == (c) )
#define DHCP_SERVER_OPTION_NOT_(op, c)      ( DHCP_SERVER_MESSAGE_OPTION_CODE_##op != (c) )

/*
    RFC-2132
    DHCP message type
*/
#define DHCP_SERVER_MESSAGE_TYPE_DISCOVER       1   /**< DHCPDISCOVER   */
#define DHCP_SERVER_MESSAGE_TYPE_OFFER          2   /**< DHCPOFFER      */
#define DHCP_SERVER_MESSAGE_TYPE_REQUEST        3   /**< DHCPREQUEST    */
#define DHCP_SERVER_MESSAGE_TYPE_DECLINE        4   /**< DHCPDECLINE    */
#define DHCP_SERVER_MESSAGE_TYPE_ACK            5   /**< DHCPACK        */
#define DHCP_SERVER_MESSAGE_TYPE_NAK            6   /**< DHCPNAK        */
#define DHCP_SERVER_MESSAGE_TYPE_RELEASE        7   /**< DHCPRELEASE    */
#define DHCP_SERVER_MESSAGE_TYPE_INFORM         8   /**< DHCPINDORM     */

/*
    Flag
*/
#define DHCP_SERVER_MESSAGE_FLAG_BROADCAST      0x0080

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
/**
 *  \brief
 *      Ethernet header format
 */
typedef struct {
    u8      dmac[DHCP_SERVER_MAC_LEN];
    u8      smac[DHCP_SERVER_MAC_LEN];
    u8      etype[2];
#ifdef DHCP_SERVER_TARGET
} __attribute__((packed)) dhcp_server_eth_header_t;
#else
} dhcp_server_eth_header_t;
#endif

/**
 *  \brief
 *      IP header format
 */
typedef struct {
    u8      vhl;
    u8      tos;
    u16     len;
    u16     ident;
    u16     flag;
    u8      ttl;
    u8      proto;
    u16     chksum;
    u32     sip;
    u32     dip;
#ifdef DHCP_SERVER_TARGET
} __attribute__((packed)) dhcp_server_ip_header_t;
#else
} dhcp_server_ip_header_t;
#endif

/**
 *  \brief
 *      UDP header format
 */
typedef struct {
    u16     sport;
    u16     dport;
    u16     len;
    u16     chksum;
#ifdef DHCP_SERVER_TARGET
} __attribute__((packed)) dhcp_server_udp_header_t;
#else
} dhcp_server_udp_header_t;
#endif

/**
 *  \brief
 *      Type of data used for client identifier.
 *
 *      http://www.iana.org/assignments/arp-parameters/arp-parameters.xml
 *      if 0, it contains an identifier, e.g. a fully qualified domain name,
 *          other than a hardware address.
 *      if 1, Ethernet MAC address.
 *      others, ha, we do not understand it.
 */
enum {
    DHCP_SERVER_MESSAGE_CLIENT_IDENTIFIER_TYPE_FQDN,  /**< Fully qualified domain name */
    DHCP_SERVER_MESSAGE_CLIENT_IDENTIFIER_TYPE_MAC,   /**< Ethernet MAC address */
};

/**
 *  \brief
 *      Format of DHCP message
 *      Refer to RFC-2131, page 9-10
 *
 *      htype: refer to
 *          http://www.iana.org/assignments/arp-parameters/arp-parameters.xml
 *          if 1, Ethernet MAC address.
 *          others, HA, we do not understand it.
 */
typedef struct {
    u8      op;         /**< Packet op code */
    u8      htype;      /**< Hardware address type */
    u8      hlen;       /**< Hardware address length */
    u8      hops;       /**< Optionally used by relay agents */
    u32     xid;        /**< Transaction ID */
    u16     secs;       /**< Seconds elapsed since client began address/renew request */
    u16     flags;      /**< Bit 0 broadcast bit */
    u32     ciaddr;     /**< Client IP address */
    u32     yiaddr;     /**< 'your' (client) IP address */
    u32     siaddr;     /**< IP address of next server to use in DHCP */
    u32     giaddr;     /**< Relay agent IP address */
    u8      chaddr[DHCP_SERVER_MESSAGE_CHADDR_LEN];     /**< Client hardware address */
    u8      sname[DHCP_SERVER_MESSAGE_SNAME_LEN + 1];   /**< Optional server host name */
    u8      file[DHCP_SERVER_MESSAGE_FILE_LEN + 1];     /**< Boot file name */
    u8      *options;                               /**< Optional parameters field, variable length */
#ifdef DHCP_SERVER_TARGET
} __attribute__((packed)) dhcp_server_message_t;
#else
} dhcp_server_message_t;
#endif

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
);

//----------------------------------------------------------------------------
#endif //__DHCP_SERVER_MESSAGE_H__
