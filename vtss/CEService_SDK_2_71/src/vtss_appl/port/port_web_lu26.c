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

/* Stock resources in sw_mgd_html/images/jack_.... */
#define PORT_ICON_WIDTH     32
#define PORT_ICON_HEIGHT    23

static const char *port_type(vtss_port_no_t iport)
{
    switch(port_custom_table[iport].mac_if) {
    case VTSS_PORT_INTERFACE_SGMII:
    case VTSS_PORT_INTERFACE_QSGMII:
        return "copper";
    case VTSS_PORT_INTERFACE_XAUI:
        return port_custom_table[iport].cap & PORT_CAP_VTSS_10G_PHY ? "sfp" : "x2";
    case VTSS_PORT_INTERFACE_SERDES:
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
    char               combo_port = 0;
    char               sfp_port = 0;
    BOOL	       lu26 = (vtss_board_type() == VTSS_BOARD_LUTON26_REF);

    /* Backround, SID, managed */
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|",
                  usid,
                  lu26 ? "switch_enzo.png" : "switch_small.png");
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
             port_mgmt_mode_txt(iport, port_status.status.speed, port_status.status.fdx, port_status.fiber) :
             "Down") : "Disabled";
        if (port_custom_table[iport].cap & PORT_CAP_SFP_DETECT) {
            if(lu26) {
                row = 0;
                col = 18 + (iport + 2 - VTSS_PORTS); // 2x SFP ports
                xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + 20;
                yoff = PORT_ICON_HEIGHT * (2 - row);
            } else {
                row = 0;
                col = 10 + sfp_port;
                xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + (12 * (col / 4 + (sfp_port++)));
                yoff = PORT_ICON_HEIGHT * 2;
            }
        } else {
            if(lu26) {
                /* Two columns */
                row = (iport - VTSS_PORT_NO_START) % 2;
                col = (iport - VTSS_PORT_NO_START) / 2;
                xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + (12 * (col / 6));
                yoff = PORT_ICON_HEIGHT * (2 - row);
            } else {
                /* Single column */
                row = 0;
                col = iport;
                xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + (12 * (col / 4));
                yoff = PORT_ICON_HEIGHT * 2;
            }
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
                      lu26 && row != 0 ? "top" : "bottom",
                      uport, speed,
                      xoff,     /* xoff */
                      yoff,     /* yoff */
                      PORT_ICON_WIDTH,
                      PORT_ICON_HEIGHT,
                      row == 0 ? 1 : -1);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        // Update the fiber ports
        if (port_custom_table[iport].cap & (PORT_CAP_DUAL_COPPER | PORT_CAP_DUAL_FIBER)) {
            col = 12 + (combo_port);
            xoff = PORT_ICON_WIDTH + col * PORT_ICON_WIDTH + (12 * (col / 4));
            yoff = PORT_ICON_HEIGHT * 2;
            combo_port++;
            
            // If the active link is cobber and the port has link then
            // change the state to down for the fiber port.
            if (!conf.enable) {
                state = "disabled";
            } else if (!port_status.fiber || !port_status.status.link) {
                state = "down";
            } else {
                state = "link";
            }

            // Transmit fiber port infomation.
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                          "%u/jack_%s_%s_%s.png/Port %u: %s/%u/%u/%u/%u/%d|",
                          uport,
                          "sfp", 
                          state, 
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

    cyg_httpd_write_chunked(",", 1);
}

