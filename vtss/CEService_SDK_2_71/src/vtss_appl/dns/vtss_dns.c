/*

 Vitesse Switch Software.

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

#include "main.h"
#include "msg_api.h"
#include "port_api.h"
#include "critd_api.h"
#include "ip_dns.h"
#include "ip2_api.h"
#include "ip2_iterators.h"
#include "misc_api.h"

#include <sys/param.h>
#undef _KERNEL
#include <sys/socket.h>
#include <sys/ioctl.h>
#define _KERNEL
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <tftp_support.h>
#include <time.h>
#include <network.h>


/* Ethernet data link layer frame */
typedef struct {
    uchar           da[6];
    uchar           sa[6];
    ushort          eth_type;
}  __attribute__((packed)) eth_frame_header_t;

#ifndef IP_VHL_HL
#define IP_VHL_HL(vhl)      ((vhl) & 0x0f)
#endif /* IP_VHL_HL */

#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_IP_DNS

void vtss_dns_init(void)
{
}

// Warning, this structure is copied from ip_stack_glue to make some refacturing
// possible. This module has nothing to do with ip_stack_glue and thid
// dependency should be avoided.
typedef struct {
    unsigned char   da[6];
    unsigned char   sa[6];
    unsigned short  vid;
    unsigned short  eth_type;
    unsigned char   ip_version;
    unsigned char   ip_protocol;
    unsigned long   ip_header_len;
    unsigned long   sip;
    unsigned long   dip;
    unsigned short  sport;
    unsigned short  dport;
#ifdef VTSS_SW_OPTION_IPV6
    unsigned char   sip_v6[16];
    unsigned char   dip_v6[16];
#endif /* VTSS_SW_OPTION_IPV6 */
} frame_info_t;

/******************************************************************************/
// RX_dns()
// Receive a DNS Protocol frame.
/******************************************************************************/
BOOL vtss_rx_dns(void *contxt, const uchar *const frame, const vtss_packet_rx_info_t *const rx_info)
{
    int                     ip_header_len = sizeof(struct ip);
    frame_info_t            frame_info;
    struct udphdr           *udp_header;
#define UDP_HEADER_LEN sizeof(struct udphdr)
    unsigned char           *ptr, *orig;
    unsigned char           qname[256];
    i8                      *qname_ptr;
    dns_proxy_query_item_t  dns_proxy_query_msg;

    // For us. Analyze it.
    T_D("Frame with length %d received on port %d with vid %d", rx_info->length, rx_info->port_no, rx_info->tag.vid);
    T_D_HEX(frame, rx_info->length);
    eth_frame_header_t *eth_frame = (eth_frame_header_t *)frame;

    memset(&frame_info, 0x0, sizeof(frame_info));

    //da, sa
    memcpy(frame_info.da, eth_frame->da, 6);
    memcpy(frame_info.sa, eth_frame->sa, 6);

    //vid
    frame_info.vid = rx_info->tag.vid;

    //eth_type
    frame_info.eth_type = ntohs(eth_frame->eth_type);

    switch (frame_info.eth_type) {
    case 0x0800: { /* IP */
        register struct ip *ip = (struct ip *)((caddr_t)eth_frame + sizeof(*eth_frame));

        ip_header_len = IP_VHL_HL(ip->ip_hl) << 2;

        //ip_version, ip_protocol, ip_header_len
        frame_info.ip_version = ip->ip_v;
        frame_info.ip_protocol = ip->ip_p;
        frame_info.ip_header_len = ip_header_len;

        if (frame_info.ip_version == 4) { /* IPv4 */
            //sip, dip
            frame_info.sip = ntohl(ip->ip_src.s_addr);
            frame_info.dip = ntohl(ip->ip_dst.s_addr);
        }
        if (frame_info.ip_protocol == 17) { /* UDP */
            /* Get UDP header */
            udp_header = (struct udphdr *)((caddr_t)eth_frame + sizeof(*eth_frame) + ip_header_len);

            //sport, dport
            frame_info.sport = ntohs(udp_header->uh_sport);
            frame_info.dport = ntohs(udp_header->uh_dport);
        }
        break;
    }
    default: {
        //T_W("DNS got Non-IPv4 packets");
        return FALSE;
    }
    }
    if (frame_info.dport == 53) {
        BOOL                    dns2me;
        i8                      adrBuf[40];
        ip2_iter_intf_ifadr_t   chk_intf;
        vtss_rc                 chk_rc;

        T_D("RCV from INTF-VID-%u", rx_info->tag.vid);
        dns2me = FALSE;
        VTSS_IP2_ITER_INTF4_ADDR(chk_rc, frame_info.dip, &chk_intf);

        if (chk_rc == VTSS_OK) {
            i8  adrBuf1[40];

            memset(adrBuf, 0x0, sizeof(adrBuf));
            memset(adrBuf1, 0x0, sizeof(adrBuf1));
            T_D("INTF-ADR-%s is %s (DST=%s)",
                misc_ipv4_txt(chk_intf.ipa.addr.ipv4, adrBuf),
                (chk_intf.if_info.enable_status == IP2_ITER_IFINFST_ENA_UP) ? "Up" : "Down",
                misc_ipv4_txt(frame_info.dip, adrBuf1));

            if (chk_intf.if_info.enable_status == IP2_ITER_IFINFST_ENA_UP) {
                dns2me = TRUE;
            }
        }

        if (!dns2me) {
            memset(adrBuf, 0x0, sizeof(adrBuf));
            T_D("Frame destined to %s is not for me!", misc_ipv4_txt(frame_info.dip, adrBuf));
            return FALSE;
        }

        // Warning, potential buffer overflow.
        // In case of IPv4 fragmentation, then the assumed length might not be
        // true
        ptr = orig = (unsigned char *)((caddr_t)eth_frame + sizeof(*eth_frame) + ip_header_len + UDP_HEADER_LEN);
        ptr = ptr + 12;
        memset(qname, 0x0, sizeof(qname));
        cyg_get_dns_query_name(orig, ptr, qname);
        qname_ptr = (i8 *)&qname[0];
        ptr = ptr + strlen(qname_ptr) + 1;
        uchar abc1 = * ptr;
        uchar abc2 = * (ptr + 1);
        dns_proxy_query_msg.querier_opcode = ((abc1 << 8) | abc2);

        // Warning, this is not proxing...
        // This method will not work for routed VLANs
        memcpy(dns_proxy_query_msg.querier_mac, frame_info.sa, 6);
        strcpy(dns_proxy_query_msg.querier_question, qname_ptr);
        dns_proxy_query_msg.querier_ip = frame_info.sip;
        dns_proxy_query_msg.querier_dns_dip = frame_info.dip;
        dns_proxy_query_msg.querier_udp_port = frame_info.sport;
        memcpy(&dns_proxy_query_msg.querier_dns_transaction_id, orig, 2);
        dns_proxy_query_msg.querier_incoming_port = rx_info->port_no;
        dns_proxy_query_msg.querier_vid = rx_info->tag.vid;
        dns_proxy_send_query_data(&dns_proxy_query_msg);
        return TRUE;
    } else if (frame_info.sport == 53) {
        return FALSE;
    } else {
        return FALSE;
    }
}

BOOL vtss_dns_ipa_valid(vtss_ip_addr_t *ipa)
{
    u8  idx;

    if (!ipa) {
        return FALSE;
    }

    switch ( ipa->type ) {
    case VTSS_IP_TYPE_NONE:
        for (idx = 0; idx < 16; idx++) {
            if (ipa->addr.ipv6.addr[idx]) {
                return FALSE;
            }
        }

        break;
    case VTSS_IP_TYPE_IPV4:
        idx = (u8)((ipa->addr.ipv4 >> 24) & 0xFF);
        if ((idx == 127) ||
            ((idx > 223) && (idx < 240))) {
            return FALSE;
        }

        break;
    case VTSS_IP_TYPE_IPV6:
        idx = ipa->addr.ipv6.addr[0];
        if (idx == 0xFF) {
            return FALSE;
        }

        for (idx = 0; idx < 16; idx++) {
            if (ipa->addr.ipv6.addr[idx]) {
                break;
            } else {
                if ((idx == 15) && (ipa->addr.ipv6.addr[idx] == 1)) {
                    return FALSE;
                }
            }
        }

        break;
    default:
        return FALSE;
    }

    return TRUE;
}
