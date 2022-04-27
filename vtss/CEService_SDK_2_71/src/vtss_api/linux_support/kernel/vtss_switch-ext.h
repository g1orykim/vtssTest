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


#ifndef _VTSS_SWITCH_EXT_H
#define _VTSS_SWITCH_EXT_H

#include <linux/ioctl.h>
#include <linux/types.h>

#include "vtss_api.h"

/*
 * CCM Offload API (Extension of switch API)
 */

/* CCM frame params */
typedef struct {
    vtss_port_no_t port_no; /* Port to inject CCM frame on */
    u8  qos_class;          /* QoS class to use */
    u32 ccm_fps;            /* CCM rate (fps) */
    BOOL ccm_count;        /* Framecounting enabled */
} vtssx_ccm_opt_t;

/* CCM session status */
typedef struct {
    u64 ccm_frm_cnt;
    BOOL active;
} vtssx_ccm_status_t;

typedef void* vtssx_ccm_session_t;

vtss_rc vtssx_ccm_start(const vtss_inst_t inst,     /* Instance */
                        const void *frame,          /* Frame data */
                        size_t length,              /* Length of frame data - ex. fcs */
                        const vtssx_ccm_opt_t *opt, /* CCM offload opts */
                        vtssx_ccm_session_t *sess); /* CCM frame session ID (returned) */

vtss_rc vtssx_ccm_status_get(const vtss_inst_t inst,   /* Instance */
                             vtssx_ccm_session_t sess, /* CCM frame session ID */
                             vtssx_ccm_status_t *status); /* CCM session status */

vtss_rc vtssx_ccm_cancel(const vtss_inst_t inst,    /* Instance */
                         vtssx_ccm_session_t sess); /* CCM frame session ID */

#endif /* ifndef _VTSS_SWITCH_EXT_H */
