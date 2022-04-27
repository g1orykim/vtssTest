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

#include "cli.h"
#include "main.h"
#include "ping.h"
#include "ip2_api.h"
#include "ping_api.h"

#ifdef VTSS_SW_OPTION_WEB
#include "web_api.h"
#endif /* VTSS_SW_OPTION_WEB */


#include <network.h>
#include <arpa/inet.h>
#include <netinet/ip_var.h>
#ifdef VTSS_SW_OPTION_IPV6
#include <netinet/icmp6.h>
#endif /* VTSS_SW_OPTION_IPV6 */

/* Thread variables */
#define PING_THREAD_NAME_MAX    16      /* Maximum thread name */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_PING

#define PRINTF(...) \
    if(pr) \
        (void)(*pr)(__VA_ARGS__)

/* Protected by thread */
/*lint -esym(459, ping_thread_name, ping_thread_stack, ping_thread_data, ping_thread_handle) */
#ifdef VTSS_SW_OPTION_WEB
static char ping_thread_name[PING_MAX_CLIENT][PING_THREAD_NAME_MAX];
static char ping_thread_stack[PING_MAX_CLIENT][THREAD_DEFAULT_STACK_SIZE];
static cyg_thread ping_thread_data[PING_MAX_CLIENT];
static cyg_handle_t ping_thread_handle[PING_MAX_CLIENT];
#endif /* VTSS_SW_OPTION_WEB */


typedef struct {
    char          ip[256];      /* Input arg - IP address */
    size_t        len;          /* Input arg - len of ping */
    size_t        cnt;          /* Input arg - cnt of ping */
    size_t        interval;     /* Input arg - interval of ping (seconds) */
    cli_iolayer_t *io;          /* Input arg - Ping output layer*/
#ifdef VTSS_SW_OPTION_IPV6
    int           is_ipv6;
    vtss_ipv6_t   ipv6_address;
    vtss_vid_t    vid;
#endif
    int           in_use;
} t_ping_req;

/* Protected by thread */
/*lint -esym(459, ping_io) */
#ifdef VTSS_SW_OPTION_WEB
static t_ping_req ping_io[PING_MAX_CLIENT];
#endif /* VTSS_SW_OPTION_WEB */

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "ping",
    .descr     = "Ping worker"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

// Compute INET checksum
int
inet_cksum(u_short *addr, int len)
{
    register int nleft = len;
    register u_short *w = addr;
    register u_short answer;
    register u_int sum = 0;
    u_short odd_byte = 0;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while ( nleft > 1 )  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if ( nleft == 1 ) {
        *(u_char *)(&odd_byte) = *(u_char *)w;
        sum += odd_byte;
    }

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0x0000ffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);                     /* add carry */
    answer = ~sum;                          /* truncate to 16 bits */
    return (answer);
}

static int
show_icmp(vtss_ip2_cli_pr *pr, unsigned char *pkt, int len,
          struct sockaddr_in *from, struct sockaddr_in *to)
{
    cyg_tick_count_t *tp, tv;
    struct ip *ip;
    struct icmp *icmp;
    tv = PING_TIME_STAMP_VALUE;
    ip = (struct ip *)pkt;

    if ((len < (int) sizeof(*ip)) || ip->ip_v != IPVERSION) {
        PRINTF("%s: Short packet or not IP! - Len: %d, Version: %d\n",
               inet_ntoa(from->sin_addr), len, ip->ip_v);
        return -1;
    }
    icmp = (struct icmp *)(pkt + sizeof(*ip));
    // Deduct ICMP header (20 bytes)
    len -= sizeof(*ip);
    tp = (cyg_tick_count_t *)icmp->icmp_data;
    if (icmp->icmp_type != ICMP_ECHOREPLY) {
        PRINTF("%s: Invalid ICMP - type: %d\n",
               inet_ntoa(from->sin_addr), icmp->icmp_type);
        return -1;
    }
    if (icmp->icmp_id != (unsigned short) cyg_thread_self()) {
        PRINTF("%s: ICMP received for wrong id - sent: %x, recvd: %x\n",
               inet_ntoa(from->sin_addr), (unsigned short) cyg_thread_self(),
               icmp->icmp_id);
    }

    PRINTF("%d bytes from %s: ", len, inet_ntoa(from->sin_addr));
    PRINTF("icmp_seq=%d", (int)(ntohs(icmp->icmp_seq)));
    if (tv < *tp) {
        /* wrap around */
        cyg_tick_count_t    time_wrap = PING_TIME_STAMP_BASE - *tp + 1;

        PRINTF(", time=%dms\n", (int)(tv + time_wrap) * ECOS_MSECS_PER_HWTICK);
    } else {
        PRINTF(", time=%dms\n", (int)(tv - *tp) * ECOS_MSECS_PER_HWTICK);
    }

    return (int)(ntohs(icmp->icmp_seq));
}

/* return TRUE: ICMP ID not matched, FALSE: ICMP ID matched */
static BOOL
ping_icmp_id_not_matched(unsigned char *pkt, int len)
{
    struct ip *ip;
    struct icmp *icmp;

    ip = (struct ip *)pkt;
    if ((len < (int) sizeof(*ip)) || ip->ip_v != IPVERSION) {
        return FALSE;
    }

    icmp = (struct icmp *)(pkt + sizeof(*ip));
    if (icmp->icmp_id != (unsigned short) cyg_thread_self()) {
        return TRUE;
    }

    /* someone else's packet */
    return FALSE;
}

static BOOL
ping_host(vtss_ip2_cli_pr *pr, int s, struct sockaddr_in *host,
          size_t icmp_len, size_t count, size_t interval)
{
    // Tx packet. The +28 refers to sizeof(normal IP header) = 20 plus
    // sizeof ICMP header (8 bytes)
    struct icmp *icmp = (struct icmp *)VTSS_MALLOC(PING_MAX_PACKET_LEN + 28);

    // Rx packet. The +28 refers to sizeof(normal IP header) = 20 plus
    // sizeof ICMP header (8 bytes)
    unsigned char *rx_pkt = VTSS_MALLOC(PING_MAX_PACKET_LEN + 28);
    size_t seq;
    int sent, ok_recv, bogus_recv, recv_per_cnt;
    cyg_tick_count_t *tp;
    long *dp;
    struct sockaddr_in from;
    size_t i, time_diff;
    int len;
    socklen_t fromlen;
    BOOL    echo_responce;
    time_t start_time, end_time;
    u32 sleep_time;

    T_D("Ping start");

    if (!icmp || !rx_pkt) {
        PRINTF("Memory allocated fail.\n");

        if (icmp) {
            VTSS_FREE(icmp);
        }
        if (rx_pkt) {
            VTSS_FREE(rx_pkt);
        }

        return FALSE;
    }

    sent = ok_recv = bogus_recv = 0;
    PRINTF("PING server %s, %d %s of data.\n",
           inet_ntoa(host->sin_addr), icmp_len,
           icmp_len == 1 ? "byte" : "bytes");
    for (seq = 0;  seq < count; seq++) {
        // Build ICMP packet
        icmp->icmp_type = ICMP_ECHO;
        icmp->icmp_code = 0;
        icmp->icmp_cksum = 0;
        icmp->icmp_seq = htons(seq);
        icmp->icmp_id = (unsigned short) cyg_thread_self();
        // Set up ping data
        tp = (cyg_tick_count_t *)icmp->icmp_data;
        *tp++ = PING_TIME_STAMP_VALUE;
        dp = (long *)tp;
        for (i = sizeof(*tp); i < icmp_len; i += sizeof(*dp)) {
            *dp++ = i;
        }
        // Add checksum
        icmp->icmp_cksum = inet_cksum( (u_short *)icmp, icmp_len + 8);
        // Send it off
        if (sendto(s, icmp, icmp_len + 8, 0,
                   (struct sockaddr *)host, sizeof(*host)) < 0) {
            PRINTF("sendto: %s\n", strerror(errno));
            if (cli_getkey(CTLC)) {
                break;
            }
            continue;
        }
        sent++;                 /* One more in the air OK */
        ICMP_STAT_TYPE_OUT_INC(1, ICMP_ECHO, 1);
        T_D("Sent %d requests", sent);
        // Wait for a response
        echo_responce = FALSE;
        recv_per_cnt = 0;
        start_time = time(NULL);
        while (!echo_responce) {
            fromlen = sizeof(from);
            memset(rx_pkt, 0 , PING_MAX_PACKET_LEN + 28);
            len = recvfrom(s, rx_pkt, PING_MAX_PACKET_LEN + 28, 0, (struct sockaddr *)&from, &fromlen);
            T_D("Recv returns %d", len);

            /* Only process this received packet when ICMP ID matched */
            if (ping_icmp_id_not_matched(rx_pkt, len)) {
                continue;
            }

            end_time = time(NULL);
            if (len < 0) {
                BOOL dump_msg = FALSE;

                if (htonl(host->sin_addr.s_addr) < 0xE0000000) { // unicast
                    if (errno != ETIMEDOUT) {
                        bogus_recv++;
                    }

                    dump_msg = TRUE;
                } else {
                    if (errno == ETIMEDOUT) {
                        if (recv_per_cnt == 0) {
                            dump_msg = TRUE;
                        }
                    } else {
                        bogus_recv++;
                        dump_msg = TRUE;
                    }
                }

                if (dump_msg) {
                    PRINTF("recvfrom: %s\n", strerror(errno));
                }

                echo_responce = TRUE;
            } else {
                int _seq = show_icmp(pr, rx_pkt, len, &from, host);

                if (_seq >= 0) {
                    ok_recv++;
                    recv_per_cnt++;

                    if (_seq == seq) {
                        echo_responce = TRUE;
                    }

                } else {
                    bogus_recv++;
                }
            }
        }

        sleep_time = 0;
        time_diff = end_time > start_time ? (size_t) (end_time - start_time) : 0;
        if (len && interval && seq != (count - 1)) {
            sleep_time = interval * 1000 - time_diff * 10; //seconds
        }

        if (cli_getkey(CTLC)) {
            break;
        }

        if (sleep_time) {
            VTSS_OS_MSLEEP(sleep_time);
        }
    }

    PRINTF("Sent %d packets, received %d OK, %d bad\n",
           sent, ok_recv, bogus_recv);

    VTSS_FREE(icmp);
    VTSS_FREE(rx_pkt);

    if (sent) {
        return (sent == ok_recv ? TRUE : FALSE);
    } else {
        return FALSE;
    }
}

BOOL ping_test(vtss_ip2_cli_pr *pr, const char *ip_address, size_t len,
               size_t count, size_t interval)
{
    struct sockaddr_in  host;
    int                 s;
    struct protoent     *p;
    struct timeval      tv;
    struct hostent      *hp;
    BOOL                rc = FALSE;

    if (!ip_address) {
        return rc;
    }

    // Check on length for safety (no reason to rely on CLI or Web to check it correctly):
    if (len < PING_MIN_PACKET_LEN) {
        len = PING_MIN_PACKET_LEN;
    } else if (len > PING_MAX_PACKET_LEN) {
        len = PING_MAX_PACKET_LEN;
    }

    if (count < PING_MIN_PACKET_CNT) {
        count = PING_MIN_PACKET_CNT;
    } else if (count > PING_MAX_PACKET_CNT) {
        count = PING_MAX_PACKET_CNT;
    }

#if PING_MIN_PACKET_INTERVAL
    if (interval < PING_MIN_PACKET_INTERVAL) {
        interval = PING_MIN_PACKET_INTERVAL;
    } else
#endif
        if (interval > PING_MAX_PACKET_INTERVAL) {
            interval = PING_MAX_PACKET_INTERVAL;
        }

    // Set up host address
    host.sin_family = AF_INET;
    host.sin_len = sizeof(host);
    if (!inet_aton(ip_address, &host.sin_addr)) {
        hp = gethostbyname(ip_address);
        if (hp == NULL) {
            PRINTF("*** Failed to resolve ip address for: %s\n", ip_address);

            return rc;
        } else {
            memmove(&host.sin_addr, hp->h_addr, hp->h_length);
        }
    }

    if ((p = getprotobyname("icmp")) == (struct protoent *)0) {
        PRINTF("*** getprotobyname: %s\n", strerror(errno));
        return rc;
    }

    s = socket(AF_INET, SOCK_RAW, p->p_proto);
    if (s < 0) {
        PRINTF("*** socket: %s\n", strerror(errno));
        return rc;
    }

    tv.tv_sec = interval ? interval : 1;
    tv.tv_usec = 0;
    (void) setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    host.sin_port = 0;
    rc = ping_host(pr, s, &host, len, count, interval);

    close(s);

    return rc;
}

#ifdef VTSS_SW_OPTION_IPV6
typedef struct {
    struct icmp6_hdr    header;
    u8                  payload[PING_MAX_PACKET_LEN];
} vtss_icmp6_pkt;

static BOOL
show6_icmp(vtss_ip2_cli_pr *pr, unsigned char *pkt, int len,
           const struct sockaddr_in6 *from, const struct sockaddr_in6 *to)
{
    cyg_tick_count_t    *tp, tv;
    vtss_icmp6_pkt      *icmp6_pkt;
    struct icmp6_hdr    *icmp;
    char                fromnamebuf[128];
    char                tonamebuf[128];
    int                 error, tval;

    error = getnameinfo((struct sockaddr *)from, sizeof(*from),
                        fromnamebuf, sizeof(fromnamebuf),
                        NULL, 0,
                        NI_NUMERICHOST);
    if (error) {
        PRINTF("getnameinfo(from)\n");
        return FALSE;
    }

    error = getnameinfo((struct sockaddr *)to, sizeof(*to),
                        tonamebuf, sizeof(tonamebuf),
                        NULL, 0,
                        NI_NUMERICHOST);
    if (error) {
        PRINTF("getnameinfo(to)\n");
        return FALSE;
    }

    tv = PING_TIME_STAMP_VALUE;
    icmp6_pkt = (vtss_icmp6_pkt *)pkt;
    icmp = &icmp6_pkt->header;
    tp = (cyg_tick_count_t *)&icmp6_pkt->payload[0];

    if (icmp->icmp6_type != ICMP6_ECHO_REPLY) {
        return FALSE;
    }
    if (icmp->icmp6_id != (unsigned short) cyg_thread_self()) {
        PRINTF("%s: ICMP received for wrong id - sent: %x, recvd: %x\n",
               fromnamebuf, (unsigned short) cyg_thread_self(), icmp->icmp6_id);
    }

    PRINTF("%d bytes from %s: icmp_seq=%d, time=",
           len, fromnamebuf, (int)(ntohs(icmp->icmp6_seq)));

    tval = 0;
    if (tv < *tp) {
        /* wrap around */
        cyg_tick_count_t    time_wrap = PING_TIME_STAMP_BASE - *tp + 1;

        tval = (int)(tv + time_wrap) * ECOS_MSECS_PER_HWTICK;
    } else {
        tval = (int)(tv - *tp) * ECOS_MSECS_PER_HWTICK;
    }
    PRINTF("%dms\n", tval);

    if (to->sin6_addr.__u6_addr.__u6_addr8[0] != 0xFF) {
        if (!memcmp(&from->sin6_addr, &to->sin6_addr, sizeof(from->sin6_addr))) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return TRUE;
    }
}

static void
ping6_host(vtss_ip2_cli_pr *pr, int s, const struct sockaddr_in6 *host, size_t length, size_t count, size_t interval)
{
    vtss_icmp6_pkt      *icmp6_pkt = (vtss_icmp6_pkt *)VTSS_MALLOC(PING_MAX_PACKET_LEN + 28); // Tx packet. The +28 refers to sizeof(normal IP header) = 20 plus sizeof ICMP header (8 bytes)
    struct icmp6_hdr    *icmp;
    unsigned char *rx_pkt = VTSS_MALLOC(PING_MAX_PACKET_LEN + 28);   // Rx packet. The +28 refers to sizeof(normal IP header) = 20 plus sizeof ICMP header (8 bytes)
    int icmp_len = length;
    int seq, sent, ok_recv, bogus_recv, recv_per_cnt, cnt = count;
    cyg_tick_count_t *tp;
    long *dp;
    struct sockaddr_in6 from;
    int i, len;
    size_t fromlen;
    char namebuf[128];
    int error;
    BOOL echo_responce;
    time_t start_time, end_time, time_diff;
    u32 sleep_time;

    T_D("Start IPv6 host ping");

    if (!icmp6_pkt || !rx_pkt) {
        PRINTF("Memory allocated fail.\n");

        if (icmp6_pkt) {
            VTSS_FREE(icmp6_pkt);
        }
        if (rx_pkt) {
            VTSS_FREE(rx_pkt);
        }
        return;
    }

    sent = ok_recv = bogus_recv = 0;
    error = getnameinfo((struct sockaddr *)host, sizeof(*host),
                        namebuf, sizeof(namebuf),
                        NULL, 0,
                        NI_NUMERICHOST);
    if (error) {
        PRINTF("getnameinfo\n");
        VTSS_FREE(icmp6_pkt);
        VTSS_FREE(rx_pkt);

        return;
    } else {
        PRINTF("PING6 server %s, %d %s of data.\n",
               namebuf, length, length == 1 ? "byte" : "bytes");
    }

    icmp = &icmp6_pkt->header;
    for (seq = 0;  seq < cnt; seq++) {
        // Build ICMP packet
        icmp->icmp6_type = ICMP6_ECHO_REQUEST;
        icmp->icmp6_code = 0;
        icmp->icmp6_cksum = 0;
        icmp->icmp6_seq = htons(seq);
        icmp->icmp6_id = (unsigned short) cyg_thread_self();

        // Set up ping data
        tp = (cyg_tick_count_t *)&icmp6_pkt->payload[0];
        *tp++ = PING_TIME_STAMP_VALUE;
        dp = (long *)tp;
        for (i = sizeof(*tp); i < icmp_len; i += sizeof(*dp)) {
            *dp++ = i;
        }
        // Add checksum
        icmp->icmp6_cksum = inet_cksum((u_short *)icmp6_pkt, icmp_len + 8);
        // Send it off
        if (sendto(s, (void *)icmp6_pkt, icmp_len + 8, 0,
                   (struct sockaddr *)host, sizeof(*host)) < 0) {
            PRINTF("sendto\n");
            if (cli_getkey(CTLC)) {
                break;
            }
            continue;
        }
        sent++;
//IPv6 goes to ICMP6 TX        ICMP_STAT_TYPE_OUT_INC(-1, ICMP6_ECHO_REQUEST, 1);
        T_D("Sent %d requests", sent);
        // Wait for a response. We get our own ECHO_REQUEST and the responce
        echo_responce = FALSE;
        recv_per_cnt = 0;
        start_time = time(NULL);
        while (!echo_responce) {
            fromlen = sizeof(from);
            memset(rx_pkt, 0 , PING_MAX_PACKET_LEN + 28);
            len = recvfrom(s, rx_pkt, PING_MAX_PACKET_LEN + 28, 0, (struct sockaddr *)&from, &fromlen);
            T_D("Recv returns %d", len);

            /* Only process this received packet when ICMP ID matched */
            if (ping_icmp_id_not_matched(rx_pkt, len)) {
                continue;
            }

            end_time = time(NULL);

            if (len < 0) {
                BOOL    dump_msg = FALSE;

                if (host->sin6_addr.__u6_addr.__u6_addr8[0] != 0xFF) {
                    if (errno != ETIMEDOUT) {
                        bogus_recv++;
                    }

                    dump_msg = TRUE;
                } else {
                    if (errno == ETIMEDOUT) {
                        if (recv_per_cnt == 0) {
                            dump_msg = TRUE;
                        }
                    } else {
                        bogus_recv++;
                        dump_msg = TRUE;
                    }
                }

                if (dump_msg) {
                    PRINTF("recvfrom: %s\n", strerror(errno));
                }

                echo_responce = TRUE;
            } else {
                if (show6_icmp(pr, rx_pkt, len, &from, host)) {
                    ok_recv++;
                    recv_per_cnt++;
                    if (host->sin6_addr.__u6_addr.__u6_addr8[0] != 0xFF) {
                        echo_responce = TRUE;
                    }
                }
            }
        }

        sleep_time = 0;
        time_diff = end_time > start_time ? (size_t) (end_time - start_time) : 0;
        if (len && interval && seq != (count - 1)) {
            sleep_time = interval * 1000 - time_diff * 10; //seconds
        }

        if (cli_getkey(CTLC)) {
            break;
        }

        if (sleep_time) {
            VTSS_OS_MSLEEP(sleep_time);
        }
    }

    PRINTF("Sent %d packets, received %d OK, %d bad\n",
           sent, ok_recv, bogus_recv);

    VTSS_FREE(icmp6_pkt);
    VTSS_FREE(rx_pkt);
}

static void
ping6_test_1(vtss_ip2_cli_pr *pr, struct sockaddr_in6 *host, size_t len, size_t count, size_t interval)
{
    struct protoent *p;
    struct timeval  tv;
    int             s;

    T_D("ping6 setup");

    if ((p = getprotobyname("ipv6-icmp")) == (struct protoent *)0) {
        PRINTF("getprotobyname\n");
        return;
    }
    s = socket(AF_INET6, SOCK_RAW, p->p_proto);
    if (s < 0) {
        PRINTF("socket\n");
        return;
    }

    if (host->sin6_addr.s6_addr[0] == 0xFF) {
        u_int   hlim = 255;

        if (setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_IF, &host->sin6_scope_id, sizeof(host->sin6_scope_id)) < 0) {
            close(s);
            return;
        }

        if ((host->sin6_addr.s6_addr[1] & 0xF) != 0xE) {
            hlim = 1;
        }

        if (setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &hlim, sizeof(hlim)) < 0) {
            close(s);
            return;
        }
    } else {
        struct in6_addr *ipa6 = &host->sin6_addr;

        /* Remove scope-id for link-local destination */
        if (IN6_IS_ADDR_LINKLOCAL(ipa6)) {
            ipa6->__u6_addr.__u6_addr16[1] = 0x0;
        }
    }

    tv.tv_sec = interval ? interval : 1;
    tv.tv_usec = 0;
    (void) setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    ping6_host(pr, s, host, len, count, interval);
    close(s);
}

BOOL ping6_test(vtss_ip2_cli_pr *pr, const vtss_ipv6_t *ipv6_address, size_t len, size_t count, size_t interval, vtss_vid_t vid)
{
    struct sockaddr_in6 host;
    u32                 edx;
    int                 i;

    if (!ipv6_address) {
        PRINTF("%% Failed to perform PING6 operation!\n");
        return FALSE;
    }

    edx = 0;
    if (vid != PING_DEF_EGRESS_INTF_VID) {
        vtss_if_status_t    *if_st;
        u32                 if_cnt;
        BOOL                intf_fnd = FALSE;

        if ((if_st = VTSS_CALLOC(PING_IPV6_MAX_IFST_CNT, sizeof(vtss_if_status_t))) == NULL) {
            PRINTF("%% Unable to perform PING6 operation!\n");
            PRINTF("%% Memory allocation failure.\n");
            return FALSE;
        }

        /* Find the specific interface with given index */
        if (vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_IPV6, vid, PING_IPV6_MAX_IFST_CNT, &if_cnt, if_st) == VTSS_OK) {
            for (; (if_cnt != 0); if_cnt--) {
                if (if_cnt && (if_st[if_cnt - 1].type == VTSS_IF_STATUS_TYPE_IPV6)) {
                    edx = if_st[if_cnt - 1].u.ipv6.os_if_index;
                    intf_fnd = TRUE;
                    break;
                }
            }
        }

        VTSS_FREE(if_st);
        if (!intf_fnd) {
            PRINTF("%% Unable to perform PING6 operation on VLAN %u!\n", vid);
            PRINTF("%% Please specify correct egress IPv6 interface.\n");
            return FALSE;
        }
    }

    memset(&host, 0x0, sizeof(struct sockaddr_in6));
    // Set up host address
    for (i = 0 ; i < 16; i++) {
        host.sin6_addr.__u6_addr.__u6_addr8[i] = ipv6_address->addr[i];
    }

    if (edx == 0) {
        if (IN6_IS_ADDR_LINKLOCAL(&host.sin6_addr) ||
            IN6_IS_ADDR_MULTICAST(&host.sin6_addr)) {
            PRINTF("%% Please specify correct egress IPv6 interface.\n");
            return FALSE;
        }
    } else {
        if (IN6_IS_ADDR_LOOPBACK(&host.sin6_addr)) {
            PRINTF("%% Do not specify egress IPv6 interface for loopback.\n");
            return FALSE;
        }
    }

    host.sin6_family = AF_INET6;
    host.sin6_len = sizeof(host);
    host.sin6_port = 0;
    host.sin6_scope_id = edx;

    // Check on length for safety (no reason to rely on CLI or Web to check it correctly):
    if (len < PING_MIN_PACKET_LEN) {
        len = PING_MIN_PACKET_LEN;
    }
    if (len > PING_MAX_PACKET_LEN) {
        len = PING_MAX_PACKET_LEN;
    }

    if (count < PING_MIN_PACKET_CNT) {
        count = PING_MIN_PACKET_CNT;
    }
    if (count > PING_MAX_PACKET_CNT) {
        count = PING_MAX_PACKET_CNT;
    }

#if PING_MIN_PACKET_INTERVAL
    if (interval < PING_MIN_PACKET_INTERVAL) {
        interval = PING_MIN_PACKET_INTERVAL;
    }
#endif
    if (interval > PING_MAX_PACKET_INTERVAL) {
        interval = PING_MAX_PACKET_INTERVAL;
    }

    ping6_test_1(pr, &host, len, count, interval);
    return TRUE;
}
#endif /* VTSS_SW_OPTION_IPV6 */

#ifdef VTSS_SW_OPTION_WEB
static void ping_thread(cyg_addrword_t data)
{
    t_ping_req *pReq = (t_ping_req *) data;
    cli_iolayer_t *pIO = pReq->io;

    // make it possible to use cli_printf
    cli_set_io_handle(pReq->io);

#ifdef VTSS_SW_OPTION_IPV6
    if (pReq->is_ipv6) {
        (void) ping6_test(cli_printf, &pReq->ipv6_address, pReq->len, pReq->cnt, pReq->interval, pReq->vid);
    } else
#endif /* VTSS_SW_OPTION_IPV6 */
    {
        (void) ping_test(cli_printf, pReq->ip, pReq->len, pReq->cnt, pReq->interval);
    }

    /* Done the job, terminate thread */
    if (pIO->cli_close) {
        pIO->cli_close(pIO);
    }
    pReq->in_use = 0;
    cyg_thread_exit();
}
#endif /* VTSS_SW_OPTION_WEB */

#ifdef VTSS_SW_OPTION_WEB
static cli_iolayer_t *ping_create_thread(const char *ip_address, size_t len, size_t count, size_t interval, BOOL is_ipv6, const vtss_ipv6_t *ipv6_address, vtss_vid_t vid)
{
    int i;
    cli_iolayer_t *io;

    T_D("Create ping thread %s", ip_address);

    // Allocate available ping IO resource
    for (i = 0 ; i < PING_MAX_CLIENT; i++) {
        if (!ping_io[i].in_use) {
            break;
        }
    }
    if (i == PING_MAX_CLIENT) {
        return NULL;
    }

    io = web_get_iolayer(WEB_CLI_IO_TYPE_PING);

    if (io == NULL) {
        return NULL;
    }

    // Fill ping IO data
    ping_io[i].io = io;
    if (is_ipv6) {
#ifdef VTSS_SW_OPTION_IPV6
        ping_io[i].ipv6_address = *ipv6_address;
        ping_io[i].vid = vid;
#endif /* VTSS_SW_OPTION_IPV6 */
    } else {
        strncpy(ping_io[i].ip, ip_address, sizeof(ping_io[i].ip) - 1);
    }
    ping_io[i].len = len;
    ping_io[i].cnt = count;
    ping_io[i].interval = interval;
#ifdef VTSS_SW_OPTION_IPV6
    ping_io[i].is_ipv6 = is_ipv6;
#endif /* VTSS_SW_OPTION_IPV6 */
    ping_io[i].in_use = 1;

    sprintf(ping_thread_name[i], "Ping %01d", i + 1);

    // Create a thread, so we can run the scheduler and have time 'pass'
    cyg_thread_create(THREAD_DEFAULT_PRIO,              // Priority
                      ping_thread,                      // entry
                      (cyg_addrword_t) &ping_io[i], // entry parameter
                      ping_thread_name[i],          // Name
                      &ping_thread_stack[i][0],     // Stack
                      sizeof(ping_thread_stack[i]), // Size
                      &ping_thread_handle[i],       // Handle
                      &ping_thread_data[i]          // Thread data structure
                     );
    cyg_thread_resume(ping_thread_handle[i]); // Start it
    return io;
}
#endif /* VTSS_SW_OPTION_WEB */


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

vtss_rc ping_init(vtss_init_data_t *data)
{
    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
        break;
    case INIT_CMD_START:
    case INIT_CMD_CONF_DEF:
    default:
        break;
    }

    return 0;
}

/* Main ping6 test - Web version */
#if defined(VTSS_SW_OPTION_IPV6) && defined(VTSS_SW_OPTION_WEB)
cli_iolayer_t *ping6_test_async(const vtss_ipv6_t *ipv6_address, size_t len, size_t count, size_t interval, vtss_vid_t vid)
{
    return ping_create_thread("<ipv6>", len, count, interval, 1, ipv6_address, vid);
}
#endif /* defined(VTSS_SW_OPTION_IPV6) && defined(VTSS_SW_OPTION_WEB) */

/* Main ping test - Web version */
#ifdef VTSS_SW_OPTION_WEB
cli_iolayer_t *ping_test_async(const char *ip_address, size_t len, size_t count, size_t interval)
{
    return ping_create_thread(ip_address, len, count, interval, 0, NULL, PING_DEF_EGRESS_INTF_VID);
}
#endif /* VTSS_SW_OPTION_WEB */

BOOL ping_test_trap_server_exist(const char *ip_address)
{
    return (ping_test(NULL, ip_address, 56, 1, PING_DEF_PACKET_INTERVAL));
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
