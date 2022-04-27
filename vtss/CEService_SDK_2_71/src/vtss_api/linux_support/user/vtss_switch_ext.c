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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

#include "vtss_switch_usermode.h"

extern int __portfd;

 /************************************
  * Port config
  */

vtss_rc port_conf_set(const vtss_port_no_t port_no,
                      const port_conf_t * conf)
{
    struct _port_conf_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    ioc.conf = *conf;
    /* Input params end */
    if ((ret = ioctl(__portfd, POIOC_vtss_port_conf_set, &ioc)) != 0) {
	printf("%s: Returns %d\n", "port_conf_set", ret);
	return (vtss_rc) ret;
    }
    /* No data returned except return value */
    return VTSS_RC_OK;
}

vtss_rc port_conf_get(const vtss_port_no_t port_no,
                      port_conf_t * conf)
{
    struct _port_conf_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(__portfd, POIOC_vtss_port_conf_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_conf_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *conf = ioc.conf;
    /* Output params end */
    return VTSS_RC_OK;
}

vtss_rc port_cap_get(const vtss_port_no_t port_no,
                     port_cap_t * cap)
{
    struct _port_cap_ioc ioc;
    int ret;
    /* Input params start */
    ioc.port_no = port_no;
    /* Input params end */
    if ((ret = ioctl(__portfd, POIOC_vtss_port_cap_get, &ioc)) != 0) {
	printf("%s: Returns %d\n", "vtss_port_cap_get", ret);
	return (vtss_rc) ret;
    }
    /* Output params start */
    *cap = ioc.cap;
    /* Output params end */
    return VTSS_RC_OK;
}

 /************************************
  * Frame inject
  */

struct vtssx_inject_handle {
    int sock_fd;
    struct sockaddr_nl dst_addr;
    struct nlmsghdr *nlh;
};

vtss_rc vtssx_inject_init(struct vtssx_inject_handle **handle)
{
    struct vtssx_inject_handle *ihandle = malloc(sizeof(struct vtssx_inject_handle));
    if (ihandle) {
	ihandle->sock_fd =
	    socket(PF_NETLINK, SOCK_RAW, NETLINK_VTSS_FRAMEIO);
	if (ihandle->sock_fd > 0) {
            struct nlmsghdr *nlh;
            /* Initialize dst_add */
	    memset(&ihandle->dst_addr, 0, sizeof(ihandle->dst_addr));
	    ihandle->dst_addr.nl_family = AF_NETLINK;
	    ihandle->dst_addr.nl_pid = 0;
	    ihandle->dst_addr.nl_groups = 0;
            nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(sizeof(vtss_netlink_inject_t)));
            if(nlh) {
                /* Initialize nlh */
                memset(nlh, 0, NLMSG_SPACE(0));
                nlh->nlmsg_pid = getpid();
                ihandle->nlh = nlh; /* Attach nlh buffer */
                *handle = ihandle;
                return VTSS_RC_OK;
            }
            close(ihandle->sock_fd);
	} else
            perror("socket");
	free(ihandle);
    }
    return VTSS_RC_ERROR;
}

vtss_rc vtssx_inject_deinit(struct vtssx_inject_handle ** handle)
{
    struct vtssx_inject_handle *ihandle = *handle;
    if (ihandle) {
	close(ihandle->sock_fd);
	free(ihandle->nlh);
	free(ihandle);
    }
    *handle = NULL;
    return VTSS_RC_OK;
}

vtss_rc vtssx_inject_frame(struct vtssx_inject_handle * handle, 
                           const u8 *frame, 
                           size_t length, /* Length of frame data - ex. fcs */
                           const vtssx_inject_opts_t * opts)
{
    vtss_rc rc;
    if (!handle) {
	rc = VTSS_RC_ERROR;
    } else {
	struct iovec iov[2];
	struct msghdr msg;
	vtss_netlink_inject_t *header;
        struct nlmsghdr *nlh = handle->nlh;

        /* Set total length */
	nlh->nlmsg_len = NLMSG_SPACE(sizeof(vtss_netlink_inject_t) + length);

	/* Set up injection props (part of pre-allocated nlh) */
	header = NLMSG_DATA(nlh);
	header->length = length;
	header->switch_frm = opts->switch_frm;
	header->vlan = opts->vlan;
	header->qos_class = opts->qos_class;
	header->port_mask = opts->port_mask;

	/* IOV Sg list */
	memset(iov, 0, sizeof(iov));
	iov[0].iov_base = (void *) nlh;
	iov[0].iov_len = NLMSG_SPACE(sizeof(vtss_netlink_inject_t));
	iov[1].iov_base = (void *) frame;
	iov[1].iov_len = length;

	/* msg name & iov */
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *) &handle->dst_addr;
	msg.msg_namelen = sizeof(handle->dst_addr);
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	/* send it */
	rc = sendmsg(handle->sock_fd, &msg, 0) < 0 ? VTSS_RC_ERROR : VTSS_RC_OK;
    }
    return rc;
}
