/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#include <sys/socket.h>
#include <linux/netlink.h>

#include "vtss_switch_usermode.h"
#include "vtss_switch-netlink.h"

#define MAX_PAYLOAD (2*1024)  /* maximum payload size*/

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
int sock_fd;

int main(int argc, char **argv)
{
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_VTSS_PORTEVENT);

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
        vtss_netlink_portevent_t *pev = NLMSG_DATA(nlh);
        printf("Port %2d - link %d, speed %d, fdx %d\n", 
               pev->port_no,
               pev->status.link, pev->status.speed, pev->status.fdx);
    }
    close(sock_fd);

    return 0;
}
