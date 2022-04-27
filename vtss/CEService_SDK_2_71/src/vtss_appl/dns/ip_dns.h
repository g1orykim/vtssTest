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
/* debug msg can be enabled by cmd "debug trace module level qos default debug" */

#ifndef _VTSS_IP_DNS_H_
#define _VTSS_IP_DNS_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#include "vtss_dns.h"

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>


#define DNS_CONF_VERINIT        0
#define DNS_CONF_VERSION        3

/* ================================================================= *
 *  DNS global structure
 * ================================================================= */

/* DNS global structure */
typedef struct {
    critd_t                     dns_crit;
    critd_t                     dns_pkt;
    dns_conf_t                  conf;
    void                        *filter_id;         /* Packet filter ID */
} dns_global_conf_t;

typedef struct {
    ulong                       version;            /* Block version */
    dns_conf_t                  conf;               /* Configuration */
} dns_conf_blk_t;


typedef struct dns_proxy_query_item {
    vtss_vid_t  querier_vid;
    u8          querier_mac[6];
    vtss_ipv4_t querier_ip;
    vtss_ipv4_t querier_dns_dip;
    i8          querier_question[256];
    u16         querier_udp_port;
    u16         querier_dns_transaction_id;
    u32         querier_incoming_port;
    u32         querier_opcode;
} dns_proxy_query_item_t;


/**
 * Representation of a 48-bit Ethernet address.
 */
typedef struct {
    uchar addr[6];
} __attribute__((packed)) dns_eth_addr;



/**
 * The Ethernet header.
 */
typedef struct {
    dns_eth_addr  dest;
    dns_eth_addr  src;
    ushort        type;
} __attribute__((packed)) dns_eth_hdr;


/**
 * The VLAN header.
 */
typedef struct {
    ushort ether_type;
    ushort vid;
} __attribute__((packed)) dns_vlan_hdr;


/* The IP headers. */
typedef struct {
    /* IP header. */
    uchar     vhl;
    uchar     tos;
    ushort    len;
    uchar     ipid[2];
    uchar     ipoffset[2];
    uchar     ttl;
    uchar     proto;
    ushort    ipchksum;
    ulong     srcipaddr,
              destipaddr;
} __attribute__((packed)) dns_ip_hdr;


/* The UDP headers. */
typedef struct {
    ushort  sport;      /* source port */
    ushort  dport;      /* destination port */
    ushort  ulen;           /* udp length */
    ushort  csum;           /* udp checksum */
}  __attribute__ ((aligned(1), packed)) dns_udp_hdr;



typedef struct {
    ushort          id;         /* query identification number */
    ushort          flags;
    ushort          qdcount;    /* number of question entries */
    ushort          ancount;    /* number of answer entries */
    ushort          nscount;    /* number of authority entries */
    ushort          arcount;    /* number of resource entries */
} __attribute__((packed)) dns_dns_hdr;



typedef struct {
    ushort    domain;
    ushort    rr_type;        /* Type of resourse */
    ushort    class;          /* Class of resource */
    ulong     ttl;            /* Time to live of this record */
    ushort    rdlength;       /* Lenght of data to follow */
    uchar      rdata [4];      /* Resource DATA */
} __attribute__((packed)) dns_dns_answer;


/* DNS TYPEs */
#define DNS_TYPE_A     1   /* Host address */
#define DNS_TYPE_NS    2   /* Authoritative name server */
#define DNS_TYPE_CNAME 5   /* Canonical name for an alias */
#define DNS_TYPE_PTR   12  /* Domain name pointer */
#define DNS_TYPE_AAAA  28  /* IPv6 host address */

/* DNS CLASSs */
#define DNS_CLASS_IN   1   /* Internet */

void dns_proxy_send_query_data(dns_proxy_query_item_t *query_item);

#endif /* _VTSS_IP_DNS_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
