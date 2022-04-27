/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _ACL_ICFG_H_
#define _ACL_ICFG_H_

/*
******************************************************************************

    Include files

******************************************************************************
*/

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define ACL_NO_FORM_TEXT                "no "
#define ACL_ACCESS_LIST_TEXT            "access-list"
#define ACL_ACE_TEXT                    "access-list ace"
#define ACL_RATE_LIMITER_TEXT           "rate-limiter"
#define ACL_POLICY_TEXT                 "policy"
#define ACL_POLICY_BITMASK_TEXT         "policy-bitmask"
#define ACL_INGRESS_TEXT                "ingress"
#define ACL_SWITCH_TEXT                 "switch"
#define ACL_VID_TEXT                    "vid"
#define ACL_TAG_TEXT                    "tag"
#define ACL_TAG_PRIORITY_TEXT           "tag-priority"
#define ACL_DMAC_TYPE_TEXT              "dmac-type"
#define ACL_ACTION_TEXT                 "action"
#define ACL_PERMIT_TEXT                 "permit"
#define ACL_DENY_TEXT                   "deny"
#define ACL_FILTER_TEXT                 "filter"
#define ACL_PORT_COPY_TEXT              "port-copy"
#define ACL_REDIRECT_TEXT               "redirect"
#define ACL_EVC_POLICER_TEXT            "evc-policer"
#define ACL_REDIRECT_TEXT               "redirect"
#define ACL_MIRROR_TEXT                 "mirror"
#define ACL_LOGGING_TEXT                "logging"
#define ACL_SHUTDOWN_TEXT               "shutdown"
#define ACL_INTERFACE_TEXT              "interface"
#define ACL_SWITCHPORT_TEXT             "switchport"
#define ACL_FRAMETYPE_TEXT              "frame-type"
#define ACL_ANY_TEXT                    "any"
#define ACL_ARP_TEXT                    "arp"
#define ACL_IP_TEXT                     "ipv4"
#define ACL_ICMP_TEXT                   "ipv4-icmp"
#define ACL_UDP_TEXT                    "ipv4-udp"
#define ACL_TCP_TEXT                    "ipv4-tcp"
#define ACL_IPV6_TEXT                   "ipv6"
#define ACL_IPV6_ICMP_TEXT              "ipv6-icmp"
#define ACL_IPV6_UDP_TEXT               "ipv6-udp"
#define ACL_IPV6_TCP_TEXT               "ipv6-tcp"
#define ACL_ETYPE_TEXT                  "etype"
#define ACL_ETYPE_VALUE_TEXT            "etype-value"
#define ACL_SMAC_TEXT                   "smac"
#define ACL_DMAC_TEXT                   "dmac"
#define ACL_SIP_TEXT                    "sip"
#define ACL_SIP_BITMASK_TEXT            "sip-bitmask"
#define ACL_DIP_TEXT                    "dip"
#define ACL_SPORT_TEXT                  "sport"
#define ACL_DPORT_TEXT                  "dport"
#define ACL_ARP_OPCODE_TEXT             "arp-opcode"
#define ACL_ARP_FLAG_REQUEST_TEXT       "arp-request"
#define ACL_ARP_FLAG_SMAC_TEXT          "arp-smac"
#define ACL_ARP_FLAG_TMAC_TEXT          "arp-tmac"
#define ACL_ARP_FLAG_LEN_TEXT           "arp-len"
#define ACL_ARP_FLAG_IP_TEXT            "arp-ip"
#define ACL_ARP_FLAG_ETHER_TEXT         "arp-ether"
#define ACL_IP_PROTOCOL_TEXT            "ip-protocol"
#define ACL_IP_FLAG_TTL_TEXT            "ip-ttl"
#define ACL_IP_FLAG_OPT_TEXT            "ip-options"
#define ACL_IP_FLAG_FRAG_TEXT           "ip-fragment"
#define ACL_TCP_FLAG_FIN_TEXT           "tcp-fin"
#define ACL_TCP_FLAG_SYN_TEXT           "tcp-syn"
#define ACL_TCP_FLAG_RST_TEXT           "tcp-rst"
#define ACL_TCP_FLAG_PSH_TEXT           "tcp-psh"
#define ACL_TCP_FLAG_ACK_TEXT           "tcp-ack"
#define ACL_TCP_FLAG_URG_TEXT           "tcp-urg"
#define ACL_ICMP_TYPE_TEXT              "icmp-type"
#define ACL_ICMP_CODE_TEXT              "icmp-code"
#define ACL_NEXT_HEADER_TEXT            "next-header"
#define ACL_HOP_LIMIT_TEXT              "hop-limit"
#define ACL_IP_FLAG_TEXT                "ip-flag"
#define ACL_TCP_FLAG_TEXT               "tcp-flag"

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/**
 * \file acl_icfg.h
 * \brief This file defines the interface to the DHCP snooping module's ICFG commands.
 */

/**
  * \brief Initialization function.
  *
  * Call once, preferably from the INIT_CMD_INIT section of
  * the module's _init() function.
  */
vtss_rc acl_icfg_init(void);

#endif /* _ACL_ICFG_H_ */
