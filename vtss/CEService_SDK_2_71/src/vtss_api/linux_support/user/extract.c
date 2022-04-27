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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <getopt.h>

#include <sys/socket.h>
#include <linux/netlink.h>

#include "vtss_switch_usermode.h"

#ifdef VTSS_FEATURE_FDMA

#define MAX_PAYLOAD (2*1024)  /* maximum payload size*/

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
int sock_fd;

static void
dump(const unsigned char *buf, size_t len)
{
    int i = 0;
    while (i < len) {
        int j = 0;
        printf("%04x:", i);
        while (j+i < len && j < 16) {
            printf(" %02x", buf[i+j]); 
            j++;
        }
        putchar('\n');
        i += 16;
    }
}

int main(int argc, char **argv)
{
    int option, verbose = 0, do_dump = 0, dumplength = 0;
    static const struct option options[] = {
        { "verbose",     no_argument,       NULL, 'V' },
        { "help",        no_argument,       NULL, 'h' },
        { "dump",        no_argument,       NULL, 'd' },
        { "dumplength",  required_argument, NULL, 'l' },
        {}
    };

    while (1) {
        option = getopt_long(argc, argv, "Vhdl:", options, NULL);
        if (option == -1)
            break;
        switch (option) {
        case 'V':
            verbose++;
            break;
        case 'd':
            do_dump++;
            break;
        case 'l':
            do_dump++;
            dumplength = atoi(optarg);
            break;
        case 'h':
            printf("Usage: %s [-hV] [-d] [-l <length>]\n"
                   " -d/--dump           : Dump frames\n"
                   " -l/--length <length>: Length of frame to dump (max)\n"
                   " -V/--verbose        : Verbose operation\n"
                   " -h/--help           : Display this\n"
                   , argv[0]);
        default:
            exit(-1);
        }
    }

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_VTSS_FRAMEIO);

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();  /* self pid */
    /* interested in group 1<<0 */
    src_addr.nl_groups = 1;

    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

    memset(&dest_addr, 0, sizeof(dest_addr));

    nlh = (struct nlmsghdr *) malloc( NLMSG_SPACE(MAX_PAYLOAD) );
    if(!nlh) {
        fprintf(stderr, "Malloc failure at %s:%d\n", __FILE__, __LINE__);
        exit(1);
    }
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));

    printf("Waiting for message from kernel\n");

    /* Read message from kernel */
    int len;
    while((len = recv(sock_fd, (void*) nlh, NLMSG_SPACE(MAX_PAYLOAD), 0))) {
        vtss_netlink_extract_t *xtr = NLMSG_DATA(nlh);
        vtss_fdma_xtr_props_t *props = XTR_PROPS(xtr);
        printf("Port %d:%u - length %d (copied %d) queue %d vid %d acl %d dei %d pcp %d\n", 
               props->chip_no, props->src_port + 1, xtr->length, xtr->copied, 
               props->xtr_qu, props->vid, props->acl_hit, props->dei, props->pcp);
        if(do_dump)
            dump((void*)xtr->frame, dumplength && dumplength < xtr->copied ? dumplength : xtr->copied);
    }
    close(sock_fd);

    return 0;
}
#else
int main(int argc, char **argv)
{
    printf("Sorry, this is only for running on an internal CPU\n");
    return 1;
}
#endif
