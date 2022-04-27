/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#include <pkgconf/system.h>
#include <network.h>
#include <arpa/inet.h>

#include <cyg/fileio/fileio.h>
#include <cyg/athttpd/http.h>
#include <cyg/athttpd/socket.h>

#include "main.h"
#include "web_api.h"
#include "port_api.h"
#include "msg_api.h"
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif /* VTSS_SWITCH_STACKABLE */

/* Stock resources in sw_mgd_html/images/jack_.... */
#define PORT_ICON_WIDTH		32
#define PORT_ICON_HEIGHT	23

/*
 * The (basic) portstate handler
 */
void
stat_portstate_switch(CYG_HTTPD_STATE* p, vtss_usid_t usid, vtss_isid_t isid)
{
    vtss_port_no_t     port_no;
    port_conf_t        conf;
    port_status_t      port_status;
    int                ct, xoff, yoff, row, col;

#if VTSS_SWITCH_STACKABLE
    topo_switch_stat_t ts;
    topo_switch_stat_get(isid, &ts);
#endif /* VTSS_SWITCH_STACKABLE */

    /* Backround, SID */
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|", usid, "switch.png");
    cyg_httpd_write_chunked(p->outbuffer, ct);

    /* Emit port icons */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        BOOL stack = PORT_NO_IS_STACK(port_no);
        if(port_mgmt_conf_get(isid, port_no, &conf) < 0 ||
           port_mgmt_status_get(isid, port_no, &port_status) < 0)
            break;              /* Most likely stack error - bail out */
        char *state = conf.enable ? (port_status.status.link ? "link" : "down") : "disabled";
        char *speed = conf.enable ? 
            (port_status.status.link ? 
             port_mgmt_mode_txt(port_status.status.speed, port_status.status.fdx) : 
             "Down") : "Disabled";
        if(vtss_switch_stackable() && PORT_NO_IS_STACK(port_no)) {
#if VTSS_SWITCH_STACKABLE
            if(ts.stack_port[port_no - PORT_NO_STACK_0].proto_up)
                state = "sprout";
#endif /* VTSS_SWITCH_STACKABLE */
            row = 0;
            yoff = PORT_ICON_HEIGHT * (2-row);
            col = 14 + (port_no - PORT_NO_STACK_0);
            xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + 20;
        } else {
            row = (port_no - VTSS_PORT_NO_START) % 2;
            yoff = PORT_ICON_HEIGHT * (2-row);
            col = (port_no - VTSS_PORT_NO_START) / 2;
            xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + (12 * (col / 4));
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), 
                      "%u/jack_%s_%s_%s.png/Port %u: %s/%u/%u/%u/%u/%d|", 
                      port_no,
                      (stack ? "hdmi" : "copper"), state, (row == 0 ? "bottom" : "top"),
                      port_no, speed, 
                      xoff,     /* xoff */
                      yoff,     /* yoff */
                      PORT_ICON_WIDTH,
                      PORT_ICON_HEIGHT,
                      row == 0 ? 1 : -1);
        cyg_httpd_write_chunked(p->outbuffer, ct);
    }
    /* LCD mimic - stackable */
    if(vtss_switch_stackable()) {
        BOOL master = msg_switch_is_local(isid);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), 
                      "led-%s-small.gif/%s/%u/%u/%u/%u/%d/%s/%d|", 
                      (master ? "green" : "off"), 
                      (master ? "Master" : "Slave"), 
                      (PORT_ICON_WIDTH-18)/2, /* xoff */
                      40,     /* yoff */
                      18,
                      9,
                      usid,
                      "sidLabel",
                      1);
        cyg_httpd_write_chunked(p->outbuffer, ct);
    }
    cyg_httpd_write_chunked(",", 1);
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
