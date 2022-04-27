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

#ifndef _VTSS_SWITCH_NETLINK_H
#define _VTSS_SWITCH_NETLINK_H

#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/filter.h>
#include <linux/sockios.h>      /* SIOCDEVPRIVATE */

#if !defined(NETLINK_VTSS_PORTEVENT)
#define NETLINK_VTSS_PORTEVENT	(MAX_LINKS-1)
#endif

typedef struct {
    vtss_port_no_t     port_no;
    vtss_port_status_t status;
} vtss_netlink_portevent_t;

#if !defined(NETLINK_VTSS_FRAMEIO)
#define NETLINK_VTSS_FRAMEIO 	(MAX_LINKS-2)
#endif

typedef struct {
    size_t length;              /* Length of frame data - ex. fcs */
    u8     switch_frm;          /* Whether to switch frame or inject directly to port(s) */
    u8     qos_class;           /* QoS class to use */
    u16    vlan;                /* VLAN to inject as */
    u64    port_mask;           /* Portmask if switch_frm == 0 */
    u8     frame[];             /* Actual frame data */
} vtss_netlink_inject_t;

/*
 * Note: At XTR_PROPS_ALIGN_TO alignment, after the frame data, a
 * vtss_fdma_xtr_props_t structture can be found.
 */
typedef struct {
    size_t                length; /* Length of original frame data - ex. FCS */
    size_t                copied; /* Length of frame data copied */
    u8                    frame[]; /* Received frame data */
} vtss_netlink_extract_t;

#define XTR_PROPS_ALIGN_TO	4
#define XTR_PROPS_ALIGN_MASK	(XTR_PROPS_ALIGN_TO-1)
#define XTR_PROPS_ALIGN(len)	((((u32)(len)) + XTR_PROPS_ALIGN_MASK) & ~(XTR_PROPS_ALIGN_MASK))
#define XTR_PROPS(nl_xtr)       (vtss_fdma_xtr_props_t*) (&nl_xtr->frame[NLMSG_ALIGN(nl_xtr->copied)])

struct SIOCETHDRV_filter {
    unsigned length;
    struct sock_filter *netlink_filter;
};

enum ETHDRV_ioctl_cmds {
	SIOCETHDRVRESERVED = SIOCDEVPRIVATE,
	SIOCETHDRVSETFILT,      /* Set netlink packet filter */
	SIOCETHDRVCLRFILT       /* Clear netlink packet filter */
};

#endif  /* ifndef _VTSS_SWITCH_NETLINK_H */
