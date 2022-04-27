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

#ifndef _VTSS_SWITCH_USERMODE_H_
#define _VTSS_SWITCH_USERMODE_H_

#include <sys/socket.h>
#include <linux/netlink.h>

#include <vtss_api.h>
#include <vtss_switch-ioctl.h>
#include <vtss_switch-port.h>
#include <vtss_switch-netlink.h>

/*
 * Port configuration extension API
 */

vtss_rc port_conf_set(const vtss_port_no_t port_no,
                      const port_conf_t * conf);

vtss_rc port_conf_get(const vtss_port_no_t port_no,
                      port_conf_t * conf);

vtss_rc port_cap_get(const vtss_port_no_t port_no,
                     port_cap_t * cap);

/*
 * Packet injection extension API
 */

struct vtssx_inject_handle;

typedef struct {
    u8  switch_frm; /* Whether to switch frame or inject directly to port(s) */
    u8  qos_class;  /* QoS class to use */
    u16 vlan;       /* VLAN to inject as */
    u64 port_mask;  /* Portmask if switch_frm == 0 */
} vtssx_inject_opts_t;

vtss_rc vtssx_inject_init(struct vtssx_inject_handle **handle);

vtss_rc vtssx_inject_deinit(struct vtssx_inject_handle **handle);

vtss_rc vtssx_inject_frame(struct vtssx_inject_handle *handle,
                           const u8 *frame,
                           size_t length, /* Length of frame data - ex. fcs */
                           const vtssx_inject_opts_t *opts);

#endif /* _VTSS_SWITCH_USERMODE_H_ */

