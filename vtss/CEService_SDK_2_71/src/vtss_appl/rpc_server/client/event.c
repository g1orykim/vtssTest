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
static struct sockaddr_in rx_main;
static struct sockaddr_in rx_si;
static socklen_t rx_si_len = sizeof(rx_si);

/* 
 * Consume 'evt_port_state' event (input side)
 */
void evt_port_state_receive(const vtss_port_no_t  port_no , const port_info_t * info )
{
    printf("Port(%d): link %d, speed %d, fdx %d\n",
           port_no+1, info->link, info->speed, info->fdx);
}

int main(int argc, char **argv)
{

    if ((server_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&rx_main, 0, sizeof(rx_main));
    rx_main.sin_family      = AF_INET;
    rx_main.sin_port        = htons(RPC_EVENT_PORT);
    rx_main.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_sock, (struct sockaddr *)&rx_main, sizeof(rx_main)) == -1) {
        perror("bind");
    }

    while(1) {
        rpc_msg_t *rsp = rpc_message_alloc();
        int nbytes = recvfrom(server_sock, rsp->head, rsp->available, 0, (struct sockaddr *)&rx_si, &rx_si_len);
        if (nbytes > 0) {
            rpc_udp_header_t *hdr;
            fprintf(stderr, "Received packet %d bytes from %s:%d\n", nbytes, inet_ntoa(rx_si.sin_addr), ntohs(rx_si.sin_port));
            rpc_msg_put(rsp, nbytes);
            if((hdr = (rpc_udp_header_t*) rpc_msg_pull(rsp, sizeof(rpc_udp_header_t))) != NULL) {

                /* Scrape off UDP encapsulation */
                rsp->rc     = RPC_NTOHL(hdr->rc);
                rsp->type   = RPC_NTOHL(hdr->type);
                rsp->seq    = RPC_NTOHL(hdr->seq);
                rsp->id     = RPC_NTOHL(hdr->id);

                /* Was this a real response? */
                if(rsp->type == MSG_TYPE_EVT) {
                    rpc_event_receive(rsp);
                    rsp = NULL;
                } else {
                    fprintf(stderr, "Dropping type %d from %s:%d", rsp->type, inet_ntoa(rx_si.sin_addr), ntohs(rx_si.sin_port));
                }
            }
        }
        if(rsp) {
            rpc_message_dispose(rsp);
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
