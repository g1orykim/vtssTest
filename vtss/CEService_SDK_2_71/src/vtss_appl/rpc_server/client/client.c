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

#include "rpc_udp.h"

#include <netdb.h>  // For socket
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <getopt.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "rpc_api.h"

static int server_sock;
static int debug;
static struct sockaddr_in tx_si;
static u32 msg_seq;
static struct sockaddr_in rx_si;
static struct timeval tv = {1, 0};
static socklen_t rx_si_len = sizeof(rx_si);

static void udp_send_message(rpc_msg_t *msg, struct sockaddr_in *addr)
{
    rpc_udp_header_t rhdr;
    struct iovec frags[2];
    struct msghdr mh;

    if(msg->type != MSG_TYPE_RSP) {
        msg->seq = msg_seq++;       /* New sequence number unless response message */
    }

    /* UDP encapsulation header */
    rhdr.rc     = RPC_HTONL(msg->rc);
    rhdr.type   = RPC_HTONL(msg->type);
    rhdr.seq    = RPC_HTONL(msg->seq);
    rhdr.id     = RPC_HTONL(msg->id);

    /* Send 2 fragments: UDP encapsulation + data */
    frags[0].iov_base = &rhdr;
    frags[0].iov_len = sizeof(rhdr);
    frags[1].iov_base = msg->head;
    frags[1].iov_len = msg->length;

    /* sendmsg arguments setup */
    mh.msg_name = addr;
    mh.msg_namelen = sizeof(*addr);
    mh.msg_iov = frags;
    mh.msg_iovlen = 2;
    mh.msg_control = NULL;
    mh.msg_controllen = 0;
    mh.msg_flags = 0;

    if(sendmsg(server_sock, &mh, 0) < 0) {
        fprintf(stderr, "sendto: %s\n", strerror(errno));
    }
}

int main(int argc, char **argv)
{
    int option, port;
    const char *addr = "127.0.0.1";
    static const struct option options[] = {
        { "debug",    0, NULL, 'd' },
        {}
    };

    while (1) {
        option = getopt_long(argc, argv, "d", options, NULL);
        if (option == -1)
            break;

        switch (option) {
        case 'd':
            debug++;
            break;

        default:;
        }
    }

    if((argc - optind) != 1) {
        printf("Usage: %s [-d] <target-ip>\n", argv[0]);
        exit(1);
    } else {
        addr = argv[optind];
    }

    memset(&tx_si, 0, sizeof(tx_si));
    tx_si.sin_family      = AF_INET;
    tx_si.sin_port        = htons(RPC_PORT);
    if (!inet_aton(addr, &tx_si.sin_addr)) {
        fprintf(stderr, "Address %s is invalid\n", addr);
        exit(1);
    }

    if ((server_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(1);
    }

    if (setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt");
    }

    for(port = VTSS_PORT_NO_START; port < VTSS_PORTS; port++) {
        vtss_port_status_t status;
        vtss_rc rc;
        if((rc = rpc_vtss_port_status_get(NULL, port, &status)) == VTSS_RC_OK) {
            printf("port(%2d): Link %d, speed %d, fdx %d\n", port, status.link, status.speed, status.fdx);
        } else {
            printf("vtss_port_status_get(%d): Failure - rc = 0x%0x\n", port, rc);
        }
    }

    exit(0);
}

/*
 * RPC interfaces
 */

rpc_msg_t *rpc_message_alloc(void)
{
    rpc_msg_t *msg;
    if((msg = malloc(sizeof(*msg) + RPC_BUFSIZE))) {
        memset(msg, 0, sizeof(*msg));
        rpc_msg_init(msg, (void*) &msg[1], RPC_BUFSIZE);
    }
    return msg;
}

void rpc_message_dispose(rpc_msg_t *msg)
{
    free(msg);
}

vtss_rc rpc_message_exec(rpc_msg_t *msg)
{
    udp_send_message(msg, &tx_si);
    if(msg->type == MSG_TYPE_REQ) {
        rpc_msg_t *rsp = rpc_message_alloc();
        int nbytes = recvfrom(server_sock, rsp->head, rsp->available, 0, (struct sockaddr *)&rx_si, &rx_si_len);
        if (nbytes > 0) {
            rpc_udp_header_t *hdr;
            if(debug) fprintf(stderr, "Received packet %d bytes from %s:%d\n", nbytes, inet_ntoa(rx_si.sin_addr), ntohs(rx_si.sin_port));
            rpc_msg_put(rsp, nbytes);
            if((hdr = (rpc_udp_header_t*) rpc_msg_pull(rsp, sizeof(rpc_udp_header_t))) != NULL) {

                /* Scrape off UDP encapsulation */
                rsp->rc     = RPC_NTOHL(hdr->rc);
                rsp->type   = RPC_NTOHL(hdr->type);
                rsp->seq    = RPC_NTOHL(hdr->seq);
                rsp->id     = RPC_NTOHL(hdr->id);

                /* Was this a real response? */
                if(rsp->type == MSG_TYPE_RSP) {
                    if(msg->id == rsp->id &&
                       msg->seq == rsp->seq) {
                        /* Hook into request */
                        msg->rsp = rsp;
                    } else {
                        /* Drop for now */
                        printf("Dropping unexpected response: id %d seq %d\n", rsp->id, rsp->seq);
                        rpc_message_dispose(rsp);
                    }
                } else {
                    printf("Dropping unexpected PDU: type %d id %d seq %d\n", rsp->type, rsp->id, rsp->seq);
                    rpc_message_dispose(rsp);
                }
            } else {
                /* Smallish */
                printf("Dropping undersized PDU: length %d\n", nbytes);
                rpc_message_dispose(rsp);
            }
        } else {
            perror("recvfrom");
            rpc_message_dispose(rsp);
        }
    }
    return msg->rsp ? msg->rsp->rc : msg->rc;
}
