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
#include "msg_api.h"

/* Stock resources in sw_mgd_html/images/jack_.... */
#define PORT_ICON_WIDTH     32
#define PORT_ICON_HEIGHT    23
#define PORT_BLOCK_GAP      12

/*
 * The (basic) portstate handler
 */

void stat_portstate_switch(CYG_HTTPD_STATE* p, vtss_usid_t usid, vtss_isid_t isid)
{
    vtss_port_no_t     iport;
    vtss_uport_no_t    uport;
    port_conf_t        conf;
    port_status_t      port_status;
    port_isid_info_t   info;
    int                ct, xoff = 0, row, xoff_add, block_size, big;
    int                board_type;
    u32                port_count, front1g_ports;
    const char         *placement, *type;

    /* Determine Unit type and port count */
    if(port_isid_info_get(isid, &info) == VTSS_OK) {
        board_type = info.board_type;
        port_count = info.port_count;
    } else {
        board_type = VTSS_BOARD_UNKNOWN;
        port_count = VTSS_PORTS;
    }

    /* Determine switch image and 1G block size based on board type */
    switch (board_type) {
    case VTSS_BOARD_JAG_CU48_REF:
        block_size = 12; /* 12x1G port blocks */
        front1g_ports = 48;
        big = 1;
        break;
    default:
        block_size = 8; /* 8x1G port blocks */
        front1g_ports = 24;
        big = 0;
        break;
    } 

    /* Backround, SID, managed */
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|",
                  usid, big ? "switch_48.png" : "switch_enzo.png");
    cyg_httpd_write_chunked(p->outbuffer, ct);

    /* Emit port icons */
    for (iport = 0; iport < port_count; iport++) {
        port_isid_port_info_t pinfo;
        port_cap_t cap = 0;

        /* Get port capability */
        if(port_isid_port_info_get(isid, iport, &pinfo) == VTSS_OK)
            cap = pinfo.cap;

        uport = iport2uport(iport);
        if(port_mgmt_conf_get(isid, iport, &conf) < 0 ||
           port_mgmt_status_get(isid, iport, &port_status) < 0)
            break;              /* Most likely stack error - bail out */

        char *state = conf.enable ? (port_status.status.link ? "link" : "down") : "disabled";
        char *speed = conf.enable ?
            (port_status.status.link ?
             port_mgmt_mode_txt(iport, port_status.status.speed, port_status.status.fdx, port_status.fiber) :
             "Down") : "Disabled";

        row = 0;
        placement = "bottom";
        xoff_add = PORT_ICON_WIDTH;
        if (cap & PORT_CAP_10G_FDX) {
            /* 10G port, one row */
            type = (iport == 26 || iport == 27 ? "sfp" : "x2");
            if ((iport % 4) == 0)
                xoff_add += PORT_BLOCK_GAP;
        } else {
            type = ((cap & PORT_CAP_COPPER) ? "copper" : "sfp");
            /* PORT_CAP_COPPER */
            if (iport < front1g_ports) {
                /* 1G port, two rows */
                if (iport & 1) {
                    row = 1;
                    if (cap & PORT_CAP_COPPER)
                        placement = "top";
                    xoff_add = 0;
                } else if (iport != 0 && (iport % block_size) == 0) {
                    xoff_add += PORT_BLOCK_GAP;
                }
            } else {
                /* NPI port, one row */
                placement = "top";
                xoff_add += PORT_BLOCK_GAP;
            }
        }
        xoff += xoff_add;

        // Update the copper ports
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                      "%u/jack_%s_%s_%s.png/Port %u: %s/%u/%u/%u/%u/%d|",
                      uport,
                      type,
                      state, 
                      placement,
                      uport, speed,
                      xoff,     /* xoff */
                      PORT_ICON_HEIGHT*(2 - row),     /* yoff */
                      PORT_ICON_WIDTH,
                      PORT_ICON_HEIGHT,
                      row == 0 ? 1 : -1);
        cyg_httpd_write_chunked(p->outbuffer, ct);
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

