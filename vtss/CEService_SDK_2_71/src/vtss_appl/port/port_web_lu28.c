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

#include "web_api.h"
#include "port_api.h"
#include "port_custom_api.h"
#include "msg_api.h"
#include "vtss_api_if_api.h"

#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#else
#define PORT_NO_STACK_0     24
#endif /* VTSS_SWITCH_STACKABLE */

/* Stock resources in sw_mgd_html/images/jack_.... */
#define PORT_ICON_WIDTH     32
#define PORT_ICON_HEIGHT    23

static const char *port_type(vtss_port_no_t iport)
{
    if(port_custom_table[iport].cap & PORT_CAP_5G_FDX)
        return "hdmi";

    switch(port_custom_table[iport].mac_if) {
    case VTSS_PORT_INTERFACE_SGMII:
        return "copper";
    default:
        return "sfp";
    }
}

/*
 * The (basic) portstate handler
 */

void stat_portstate_switch(CYG_HTTPD_STATE* p, vtss_usid_t usid, vtss_isid_t isid)
{
    vtss_port_no_t     iport;
    vtss_uport_no_t    uport;
    port_conf_t        conf;
    port_status_t      port_status;
    int                ct, xoff, yoff, row, col;
    BOOL	       lu28ref = (vtss_board_type() == VTSS_BOARD_ESTAX_34_REF);

    /* Backround, SID, managed */
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|",
                  usid,
                  lu28ref ? "switch.png" : "switch_enzo.png");
    cyg_httpd_write_chunked(p->outbuffer, ct);

    /* Emit port icons */
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        uport = iport2uport(iport);
        if(port_mgmt_conf_get(isid, iport, &conf) < 0 ||
           port_mgmt_status_get(isid, iport, &port_status) < 0)
            break;              /* Most likely stack error - bail out */

        char *state = conf.enable ? (port_status.status.link ? "link" : "down") : "disabled";
        char *speed = conf.enable ?
            (port_status.status.link ?
             port_mgmt_mode_txt(port_status.status.speed, port_status.status.fdx, port_status.fiber) :
             "Down") : "Disabled";
        BOOL vaui = port_custom_table[iport].mac_if == VTSS_PORT_INTERFACE_VAUI;
        if((vtss_switch_stackable() && PORT_NO_IS_STACK(iport)) || vaui) {
#if VTSS_SWITCH_STACKABLE
            topo_switch_stat_t ts;
            (void) topo_switch_stat_get(isid, &ts);
            if(ts.stack_port[iport - PORT_NO_STACK_0].proto_up)
                state = "sprout";
#endif /* VTSS_SWITCH_STACKABLE */
            // Position for the stacking ports
            if(lu28ref) {
                col = 14 + (iport - PORT_NO_STACK_0);
                xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + 20;
            } else {
                col = 16 + (iport - PORT_NO_STACK_0);
                if(port_custom_table[iport].cap & PORT_CAP_5G_FDX)
                    col++;      /* Center in case of HDMI - only 2 ports vs 4 */
                xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + (12 * (col / 4));
            }
            row = 0;
            yoff = PORT_ICON_HEIGHT * (2 - row);
        } else {
            /* Two columns */
            row = (iport - VTSS_PORT_NO_START) % 2;
            col = (iport - VTSS_PORT_NO_START) / 2;
            xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + (12 * (col / 4));
            yoff = PORT_ICON_HEIGHT * (2 - row);
        }

        // If the active link is fiber and the port has link then
        // change the state to down for the copper port.
        if (port_custom_table[iport].cap & (PORT_CAP_DUAL_COPPER | PORT_CAP_DUAL_FIBER) &&
            port_status.fiber && port_status.status.link)
            state = "down";

        // Update the copper ports
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                      "%u/jack_%s_%s_%s.png/Port %u: %s/%u/%u/%u/%u/%d|",
                      uport,
                      port_type(iport),
                      state, 
                      (row == 0 ? "bottom" : "top"),
                      uport, speed,
                      xoff,     /* xoff */
                      yoff,     /* yoff */
                      PORT_ICON_WIDTH,
                      PORT_ICON_HEIGHT,
                      row == 0 ? 1 : -1);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        // Update the fiber ports
        if (port_custom_table[iport].cap & (PORT_CAP_DUAL_COPPER | PORT_CAP_DUAL_FIBER)) {
            row = 0;
            col = 12 + (iport - 20);
            xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + (12 * (col / 4));
            yoff = PORT_ICON_HEIGHT * (2 - row);

            // If the active link is cobber and the port has link then
            // change the state to down for the fiber port.
            if (!port_status.fiber && port_status.status.link)
                state = "down";

            // Transmit fiber port infomation.
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                          "%u/jack_%s_%s_%s.png/Port %u: %s/%u/%u/%u/%u/%d|",
                          uport,
                          "sfp", state, 
                          "bottom",
                          uport, speed,
                          xoff,     /* xoff */
                          yoff,     /* yoff */
                          PORT_ICON_WIDTH,
                          PORT_ICON_HEIGHT,
                          1);
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }


    /* LCD mimic - stackable */
    if(vtss_switch_stackable()) {
        BOOL master = msg_switch_is_local(isid);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                      "lcd-%02d.png/%d/%u/%u/%u/%u///|",
                      usid,
                      usid,
                      (PORT_ICON_WIDTH - 18) / 2, /* xoff */
                      30,                         /* yoff */
                      20,
                      18);
        cyg_httpd_write_chunked(p->outbuffer, ct);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                      "led-%s-small.gif/%s/%u/%u/%u/%u///|",
                      (master ? "green" : "off"),
                      (master ? "Master" : "Slave"),
                      (PORT_ICON_WIDTH - 12) / 2, /* xoff */
                      50,                         /* yoff */
                      12,
                      12);
        cyg_httpd_write_chunked(p->outbuffer, ct);
    }
    cyg_httpd_write_chunked(",", 1);
}
