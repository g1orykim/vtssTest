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

 $Id$
 $Revision$

*/

#ifndef VTSS_MGMT_API_H
#define VTSS_MGMT_API_H

#include "main.h"          /* For MODULE_ERROR_START (and module id's). */

#ifdef VTSS_SW_OPTION_ACL
#include "acl_api.h"
#endif

#if defined(VTSS_SW_OPTION_QOS) || defined(VTSS_SW_OPTION_ACL)
/****************************************************************************/
/*  QoS                                                                     */
/****************************************************************************/

/* ACL IP text (Also used by QoS) */
char *mgmt_acl_ipv4_txt(vtss_ace_ip_t *ip, char *buf, BOOL lower);

/* Convert text to packet rate */
#if defined(VTSS_FEATURE_ACL_V2)
vtss_rc mgmt_txt2rate_v2(char *buf, ulong *rate, ulong rate_unit, ulong max_pkt_rate, ulong max_bit_rate, ulong bit_rate_granularity);
#endif /* VTSS_FEATURE_ACL_V2 */

/* Convert text to packet rate */
vtss_rc mgmt_txt2rate(char *buf, ulong *rate, ulong n);

/* Convert priority to text */
const char *mgmt_prio2txt(vtss_prio_t prio, BOOL lower);

/* Convert text to priority */
vtss_rc mgmt_txt2prio(char *buf, vtss_prio_t *class);
#endif



#ifdef VTSS_SW_OPTION_ACL
/****************************************************************************/
/*  ACL                                                                     */
/****************************************************************************/

/* Get ACE ID for SIP/SMAC binding */
vtss_ace_id_t mgmt_acl_ace_id_bind_get(vtss_isid_t isid, vtss_port_no_t port_no, BOOL sip);

/* Update ACL wizard based setup when PVID is changed */
vtss_rc mgmt_vlan_pvid_change(vtss_isid_t isid, vtss_port_no_t port_no,
                              vtss_vid_t pvid_new, vtss_vid_t pvid_old);

/* ACL frame type text */
char *mgmt_acl_type_txt(vtss_ace_type_t type, char *buf, BOOL lower);

/* ACL uchar text */
char *mgmt_acl_uchar_txt(vtss_ace_u8_t *data, char *buf, BOOL lower);

/* ACL uchar2 text */
char *mgmt_acl_uchar2_txt(vtss_ace_u16_t *data, char *buf, BOOL lower);

/* ACL uchar6 text */
char *mgmt_acl_uchar6_txt(vtss_ace_u48_t *data, char *buf, BOOL lower);

/* ACL ulong text */
char *mgmt_acl_ulong_txt(ulong value, ulong mask, char *buf, BOOL lower);

/* ACL IPv6 text */
char *mgmt_acl_ipv6_txt(vtss_ace_u128_t *ip, char *buf, BOOL lower);

/* ACL DMAC flags text */
char *mgmt_acl_dmac_txt(acl_entry_conf_t *ace, char *buf, BOOL lower);

/* ACL ARP opcode flags text */
char *mgmt_acl_opcode_txt(acl_entry_conf_t *ace, char *buf, BOOL lower);

/* ACL flag text */
char *mgmt_acl_flag_txt(acl_entry_conf_t *ace, acl_flag_t flag, BOOL lower);

/* UDP/TCP port text */
char *mgmt_acl_port_txt(vtss_ace_udp_tcp_t *port, char *buf, BOOL lower);

/* ACL IP protocol number */
uchar mgmt_acl_ip_proto(acl_entry_conf_t *ace);
#endif



#ifdef VTSS_SW_OPTION_PORT
/****************************************************************************/
/*  Port                                                                    */
/****************************************************************************/

char *mgmt_non_portlist2txt(BOOL *list, int min, int max, char *buf);

/* Convert a list of booleans to ulong
 */
u32 mgmt_bool_list2ulong(BOOL *bool_list);

/* Convert a ulong to a list of booleans
 */
BOOL *mgmt_ulong2bool_list(ulong number,BOOL *bool_list);

/* Convert list to text
 * The resulting buf will contain numbers in range [min + 1; max + 1].
 */
char *mgmt_list2txt(BOOL *list, int min, int max, char *buf);

/* Size of buffer given to mgmt_iport_list2txt(). Currently, 80 bytes are sufficient:
   0         1         2         3         4         5         6         7         
   01234567890123456789012345678901234567890123456789012345678901234567890123456789
   "2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54"
*/
#if (VTSS_PORTS < 55)
#define MGMT_PORT_BUF_SIZE 80
#else
#error "MGMT_PORT_BUF_SIZE must be increased"
#endif

/* Convert port list to text
 * list is indexed by internal port numbers.
 * and the resulting list is in user ports.
 */
char *mgmt_iport_list2txt(BOOL *list, char *buf);

/* Convert text to list */
vtss_rc mgmt_txt2list(char *buf, BOOL *list, ulong min, ulong max, BOOL def);

/* Convert text to bit field */
vtss_rc mgmt_txt2bf(char *buf, BOOL *list, ulong min, ulong max, BOOL def);

/* Convert user-port text to uport list */
vtss_rc mgmt_uport_txt2list(char *buf, BOOL *list);

/* Convert text to range */
vtss_rc mgmt_txt2range(char *buf, ulong *req_min, ulong *req_max, ulong min, ulong max);
#endif

#if defined(VTSS_SW_OPTION_IP2)
/****************************************************************************/
/*  IP                                                                      */
/****************************************************************************/
#include "ip2_api.h"

/* Convert IP string a.b.c.d[/n] */
vtss_rc mgmt_txt2ipv4(const char *buf, vtss_ipv4_t *ip, vtss_ipv4_t *mask, BOOL is_mask);

/* Convert IP string a.b.c.d[/n] and optionally allow loopback and IPMC */
vtss_rc mgmt_txt2ipv4_ext(const char *buf, vtss_ipv4_t *ip, vtss_ipv4_t *mask, BOOL is_mask, BOOL allow_loopback, BOOL allow_ipmc);

/* Convert IP MC string a.b.c.d */
vtss_rc mgmt_txt2ipmcv4(const char *buf, vtss_ipv4_t *ip);

typedef enum {
    TXT2IP6_ADDR_TYPE_INVALID = 0,

    TXT2IP6_ADDR_TYPE_MCAST,
    TXT2IP6_ADDR_TYPE_UCAST,
    TXT2IP6_ADDR_TYPE_LINK_LOCAL,
    TXT2IP6_ADDR_TYPE_UNSPECIFIED,
    TXT2IP6_ADDR_TYPE_LOOPBACK,
    TXT2IP6_ADDR_TYPE_IPV4_MAPPED,
    TXT2IP6_ADDR_TYPE_IPV4_COMPAT,
    TXT2IP6_ADDR_TYPE_GEN
} txt2ip6_addr_type_t;

#define IPV6_ADDR_STR_LEN_LONG  0
#if IPV6_ADDR_STR_LEN_LONG
#define IPV6_ADDR_STR_MAX_LEN   254
#else
#define IPV6_ADDR_STR_MAX_LEN   45
#endif /* IPV6_ADDR_STR_LEN_LONG */
#define IPV6_ADDR_STR_MIN_LEN   2

#define IPV6_ADDR_IBUF_MAX_LEN  (IPV6_ADDR_STR_MAX_LEN + 1)
#define IPV6_ADDR_IBUF_MIN_LEN  (IPV6_ADDR_STR_MIN_LEN + 1)
#define IPV6_ADDR_OBUF_MAX_LEN  40  /* 39 + 1 */
#define IPV6_ADDR_OBUF_MIN_LEN  (IPV6_ADDR_STR_MIN_LEN + 1)

/* Convert IPv6 string */
vtss_rc             mgmt_txt2ipv6     (const char *s, vtss_ipv6_t *addr);
vtss_rc             mgmt_txt2ipmcv6   (const char *s, vtss_ipv6_t *addr);
txt2ip6_addr_type_t mgmt_txt2ipv6_type(const char *s, vtss_ipv6_t *addr);
#endif /* defined(VTSS_SW_OPTION_IP2) */

#ifdef VTSS_SW_OPTION_AGGR
#include "aggr_api.h"

/****************************************************************************/
/*  Aggregation                                                             */
/****************************************************************************/

/* VSTAX_V1: Aggregation IDs, 1-2: GLAGs, 3-14: LLAGs */
/* VSTAX_V2: Aggregation IDs, 1-32:GLAGs  LLAGS:None  */
#define MGMT_AGGR_ID_START    1
#define MGMT_AGGR_ID_END      (MGMT_AGGR_ID_START + AGGR_LAG_CNT)
#define MGMT_AGGR_ID_GLAG_END (MGMT_AGGR_ID_START + AGGR_GLAG_CNT)

/* Convert aggregation ID to aggregation number */
aggr_mgmt_group_no_t mgmt_aggr_id2no(aggr_mgmt_group_no_t aggr_id);

/* Convert aggregation number to aggregation ID */
aggr_mgmt_group_no_t mgmt_aggr_no2id(aggr_mgmt_group_no_t aggr_no);

/* Check that aggregation can be added */
vtss_rc mgmt_aggr_add_check(vtss_isid_t isid, aggr_mgmt_group_no_t aggr_id, BOOL *port_list, char *buf);
#endif

#ifdef VTSS_SW_OPTION_SNMP
/****************************************************************************/
/*  SNMP                                                                    */
/****************************************************************************/

/* Convert text (e.g. .1.2.*.3) to OID */
vtss_rc mgmt_txt2oid(char *buf, int len,
                     ulong *oid, uchar *oid_mask, ulong *oid_len);
#endif

/****************************************************************************/
/*  General                                                                 */
/****************************************************************************/

/* Convert text to ulong */
vtss_rc mgmt_txt2ulong(char *buf, ulong *val, ulong min, ulong max);

/* Convert text to signed long */
vtss_rc mgmt_txt2long(char *buf, long *val, long min, long max);

vtss_rc mgmt_str_float2long(char *string_value, long *value, long min, ulong max, uchar digi_cnt);

/*
 * Prints into #string_value #long_value with #decimals decimals.
 *
 * Examples:
 *   value = 123, decimals = 0 => "123."
 *   value = 123, decimals = 1 => "12.3"
 *   value = 123, decimals = 2 => "1.23"
 *   value = 123, decimals = 3 => "0.123"
 */
void mgmt_long2str_float(char *string_value, long value, char decimals);

#endif /* VTSS_MGMT_API_H */
