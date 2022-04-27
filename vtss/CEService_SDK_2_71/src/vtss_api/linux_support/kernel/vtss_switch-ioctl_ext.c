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


#include <linux/module.h>	/* can't do without it */
#include <linux/version.h>	/* and this too */

#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>

#include "vtss_switch.h"
#include "vtss_switch-netlink.h"

extern int debug;

#define DEBUG(args...) do {if(debug) printk(args); } while(0)

#if defined(VTSS_OPT_CCM_OFFLOAD) && VTSS_OPT_CCM_OFFLOAD 
#define FCS_SIZE_BYTES	4

static struct session_desc {
    BOOL            inuse;
    u8              *frame;
    vtssx_ccm_opt_t opt;
} sessions[VTSS_CCM_SESSIONS];

static vtss_fdma_list_t *ccm_dcb;

static void session_cancelled(void *cntxt, 
                              struct tag_vtss_fdma_list *list, 
                              vtss_fdma_ch_t ch, 
                              BOOL dropped)
{
    int session = (int) list->user;
    if(session >= 0 && session < VTSS_CCM_SESSIONS) {
        struct session_desc *sesp = &sessions[session];
        sesp->inuse = FALSE;
    }
}

#endif  /* VTSS_OPT_CCM_OFFLOAD */

vtss_rc vtssx_ccm_start(const vtss_inst_t inst,      /* Instance */
                        const void *frame,           /* Frame data */
                        size_t length,               /* Length of frame data - ex. fcs */
                        const vtssx_ccm_opt_t * opt, /* CCM offload opts */
                        vtssx_ccm_session_t * sess)  /* CCM frame session ID (returned) */
{
    vtss_rc rc = VTSS_RC_ERROR;
#if defined(VTSS_OPT_CCM_OFFLOAD) && VTSS_OPT_CCM_OFFLOAD 
    int i;
    for(i = 0; i < VTSS_CCM_SESSIONS; i++) {
        struct session_desc *sesp = &sessions[i];
        if(!sesp->inuse) {
            vtss_fdma_list_t *dcb = &ccm_dcb[i];
            u32 buflen = length + VTSS_FDMA_HDR_SIZE_BYTES + FCS_SIZE_BYTES;
            u8 *kframe = kmalloc(buflen, GFP_KERNEL);
            if(kframe &&
               copy_from_user(kframe + VTSS_FDMA_HDR_SIZE_BYTES, frame, length) == 0) {
                vtss_fdma_inj_props_t props;
                sesp->frame = kframe;
                sesp->opt = *opt;
                memset(dcb, 0, sizeof(*dcb));
                dcb->data = kframe;
                dcb->act_len = buflen;
                dcb->user = (void*) i;
                dcb->inj_post_cb = session_cancelled;
                memset(&props, 0, sizeof(props));
                props.ccm_fps = opt->ccm_fps;
                props.port_mask = VTSS_BIT(opt->port_no);
                props.qos_class = opt->qos_class;
                props.ccm_enable_counting = opt->ccm_count;
                if(vtss_fdma_inj(NULL, dcb, VTSS_FDMA_CCM_CH_AUTO, length, &props) == VTSS_RC_OK) {
                    sesp->inuse = TRUE;
                    *sess = (vtssx_ccm_session_t)i;
                    rc = VTSS_RC_OK;
                } else
                    printk("CCM_start session %d failed (len %zd)\n", i, length);
            }
            if(rc != VTSS_RC_OK)
                kfree(kframe);  /* Copy/Start failed */
            break;
        }
    }
#endif  /* VTSS_OPT_CCM_OFFLOAD */
    return rc;
}

vtss_rc vtssx_ccm_status_get(const vtss_inst_t inst,   /* Instance */
                             vtssx_ccm_session_t sess, /* CCM frame session ID */
                             vtssx_ccm_status_t *status) /* CCM session status */
{
    vtss_rc rc = VTSS_RC_ERROR;
#if defined(VTSS_OPT_CCM_OFFLOAD) && VTSS_OPT_CCM_OFFLOAD 
    int i = (int) sess;
    if(i >= 0 && i < VTSS_CCM_SESSIONS) {
        struct session_desc *sesp = &sessions[i];
        vtss_fdma_list_t *dcb = &ccm_dcb[i];
        memset(status, 0, sizeof(*status));
        status->ccm_frm_cnt = dcb->ccm_frm_cnt;
        status->active = sesp->inuse;
        rc = VTSS_RC_OK;
    }
#endif  /* VTSS_OPT_CCM_OFFLOAD */
    return rc;
}

vtss_rc vtssx_ccm_cancel(const vtss_inst_t inst,    /* Instance */
                         vtssx_ccm_session_t sess) /* CCM frame session ID */
{
    vtss_rc rc = VTSS_RC_ERROR;
#if defined(VTSS_OPT_CCM_OFFLOAD) && VTSS_OPT_CCM_OFFLOAD 
    int i = (int) sess;
    if(i >= 0 && i < VTSS_CCM_SESSIONS && sessions[i].inuse) {
        struct session_desc *sesp = &sessions[i];
        vtss_fdma_list_t *dcb = &ccm_dcb[i];
        vtss_fdma_inj_props_t props;
        memset(&props, 0, sizeof(props));
        props.ccm_cancel = TRUE;
        if(vtss_fdma_inj(NULL, dcb, VTSS_FDMA_CCM_CH_AUTO, 0, &props) != VTSS_RC_OK) {
            printk("CCM_cancel session %d failed, dcb %p, frame %p\n", i, dcb, dcb->data);
        } else {
            sesp->inuse = FALSE;
            kfree(sesp->frame);
            rc = VTSS_RC_OK;
        }
    }
#endif  /* VTSS_OPT_CCM_OFFLOAD */
    return rc;
}

void vtss_ioctl_ext_init(void)
{
#if defined(VTSS_OPT_CCM_OFFLOAD) && VTSS_OPT_CCM_OFFLOAD
    DEBUG("CCM init\n");
    // Allocate DCBs
    ccm_dcb = kmalloc(VTSS_CCM_SESSIONS*sizeof(vtss_fdma_list_t), GFP_KERNEL);
#endif  /* VTSS_OPT_CCM_OFFLOAD */
}

void vtss_ioctl_ext_exit(void)
{
#if defined(VTSS_OPT_CCM_OFFLOAD) && VTSS_OPT_CCM_OFFLOAD
    int i;
    DEBUG("CCM deinit\n");
    for(i = 0; i < VTSS_CCM_SESSIONS; i++) {
        struct session_desc *sesp = &sessions[i];
        if(sesp->inuse) {
            vtss_fdma_list_t *dcb = &ccm_dcb[i];
            dcb->inj_post_cb = NULL; /* Avoid callback/waiting */
            vtssx_ccm_cancel(NULL, (vtssx_ccm_session_t)i);
        }
    }
    kfree(ccm_dcb);
#endif  /* VTSS_OPT_CCM_OFFLOAD */
}
