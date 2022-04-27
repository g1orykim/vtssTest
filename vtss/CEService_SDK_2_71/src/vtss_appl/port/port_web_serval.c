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
#include "vtss_api_if_api.h"
#include "msg_api.h"
#ifdef VTSS_SW_OPTION_PTP
#include "ptp_api.h"
#endif
#ifdef VTSS_SW_OPTION_SYNCE
#include "synce.h"
#endif

/* Stock resources in sw_mgd_html/images/jack_.... */
#define PORT_ICON_WIDTH     32
#define PORT_ICON_HEIGHT    23
#define PORT_BLOCK_GAP      12
#define NON_ETH_PORT_NO     0

static void port_httpd_write(CYG_HTTPD_STATE *p, vtss_uport_no_t uport, int type, char *state,
                             char *speed, int xoff, int row, BOOL non_eth_port)
{
    int ct;
    char *status;
    if (!non_eth_port)
    {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                        "%u/jack_%s_%s_%s.png/Port %u: %s/%u/%u/%u/%u/%d|",
                        uport,
                        type ? "sfp" : "copper",
                        state,
                        row && !type ? "top" : "bottom",
                        uport,
                        speed,
                        xoff,
                        PORT_ICON_HEIGHT*(2 - row),
                        PORT_ICON_WIDTH,
                        PORT_ICON_HEIGHT,
                        row ? -1 : 1);
    } else {
        if(!strcmp(state,"link")||!strcmp(state,"enabled"))
            status = "Enabled";
        else
            status = "Disabled";
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                         "jack_%s_%s_%s.png/%s: %s/%u/%u/%u/%u/|",
                         type ? "sma" : "copper",
                         state,
                         "bottom",
                         speed,
                         status,
                         xoff,
                         type ? (PORT_ICON_HEIGHT * 2 + 8) : (PORT_ICON_HEIGHT * 2),
                         type ? (4*PORT_ICON_WIDTH/5) : PORT_ICON_WIDTH,
                         type ? (4*PORT_ICON_HEIGHT/5) : PORT_ICON_HEIGHT);
    }
    cyg_httpd_write_chunked(p->outbuffer, ct);
}

static void 
non_eth_port_state(CYG_HTTPD_STATE* p)
{
    char *name =            "<unknown>";
    char *state_freq_in =   "<unknown>";
    char *state_freq_out =  "<unknown>";
    char *state_ptp_in =    "<unknown>";
    char *state_ptp_out =   "<unknown>";
    char *state_rs422_in =  "<unknown>";
    char *state_rs422_out = "<unknown>";
    char *state_bits =      "<unknown>";
    int  xoff, is_sma_connector,row = 0;

#if defined(VTSS_SW_OPTION_PTP) && defined(VTSS_SW_OPTION_SYNCE) 
    vtss_ptp_rs422_conf_t     mode;
    vtss_ptp_ext_clock_mode_t mode_pp;
    synce_mgmt_frequency_t    freq;
    
    if (synce_mgmt_station_clock_in_get(&freq) == SYNCE_RC_OK) {
        switch (freq) {
        case SYNCE_MGMT_STATION_CLK_DIS :
            state_freq_in = "disabled"; 
            break;
        case SYNCE_MGMT_STATION_CLK_1544_KHZ :
        case SYNCE_MGMT_STATION_CLK_2048_KHZ :
        case SYNCE_MGMT_STATION_CLK_10_MHZ :
        case SYNCE_MGMT_STATION_CLK_MAX :
            state_freq_in = "enabled";
        }
    }
    vtss_ext_clock_out_get(&mode_pp);
    switch (mode_pp.one_pps_mode) {
    case VTSS_PTP_ONE_PPS_DISABLE :
        state_ptp_in = "disabled";
        state_ptp_out = "disabled";
        break;
    case VTSS_PTP_ONE_PPS_OUTPUT :
        state_ptp_out = "enabled";
        state_ptp_in  = "disabled";
        break;
    case VTSS_PTP_ONE_PPS_INPUT :
        state_ptp_out = "disabled";
        state_ptp_in  = "enabled";
    }
    if (synce_mgmt_station_clock_out_get(&freq) == SYNCE_RC_OK) {
        switch (freq) {
        case SYNCE_MGMT_STATION_CLK_DIS :
            state_freq_out = "disabled";
            break;
        case SYNCE_MGMT_STATION_CLK_1544_KHZ :
        case SYNCE_MGMT_STATION_CLK_2048_KHZ :
        case SYNCE_MGMT_STATION_CLK_10_MHZ :
        case SYNCE_MGMT_STATION_CLK_MAX :
            state_freq_out = "enabled";
        }
    }
    vtss_ext_clock_rs422_conf_get(&mode);
    switch (mode.mode) {
    case VTSS_PTP_RS422_DISABLE :
        state_rs422_in = "down";
        state_rs422_out = "down";
        break;
    case VTSS_PTP_RS422_MAIN_AUTO :
    case VTSS_PTP_RS422_MAIN_MAN :
        state_rs422_in = "down";
        state_rs422_out = "link";
        break;
    case VTSS_PTP_RS422_SUB :
        state_rs422_in = "link";
        state_rs422_out = "down";
    }
#else
        state_freq_in   = "disabled";
        state_freq_out  = "disabled";
        state_ptp_in   = "disabled";
        state_ptp_out   = "disabled";
        state_rs422_in  = "down";
        state_rs422_out = "down";
#endif

    /* for SMA freq i/p connector */
    xoff = PORT_ICON_WIDTH;
    is_sma_connector = 1;
    name = "Frequency Input";
    port_httpd_write(p, NON_ETH_PORT_NO, is_sma_connector, state_freq_in, name, xoff, row, TRUE);
    /* for 1 PPS i/p and o/p */
    name = "1PPS Input";
    xoff += (PORT_BLOCK_GAP + PORT_ICON_WIDTH);
    port_httpd_write(p, NON_ETH_PORT_NO, is_sma_connector, state_ptp_in, name, xoff, row, TRUE);
    name = "1PPS Output";
    xoff += (PORT_BLOCK_GAP + PORT_ICON_WIDTH);
    port_httpd_write(p, NON_ETH_PORT_NO, is_sma_connector, state_ptp_out, name, xoff, row, TRUE);
    /* for SMA freq o/p connector */
    name = "Frequency Output";
    xoff += (PORT_BLOCK_GAP + PORT_ICON_WIDTH);
    port_httpd_write(p, NON_ETH_PORT_NO, is_sma_connector, state_freq_out, name, xoff, row, TRUE);
    /* for time interface i/p and o/p */
    is_sma_connector = 0;
    name = "Time Input";
    xoff += PORT_BLOCK_GAP + PORT_ICON_WIDTH;
    port_httpd_write(p, NON_ETH_PORT_NO, is_sma_connector, state_rs422_in, name, xoff, row, TRUE);
    name = "Time Output";
    xoff += (PORT_BLOCK_GAP + PORT_ICON_WIDTH);
    port_httpd_write(p, NON_ETH_PORT_NO, is_sma_connector, state_rs422_out, name, xoff, row, TRUE);
    /* for BITS interface ports*/
    state_bits = "down";
    name = "BITS Interface";
    xoff += (PORT_BLOCK_GAP + PORT_ICON_WIDTH);
    port_httpd_write(p, NON_ETH_PORT_NO, is_sma_connector, state_bits, name, xoff, row, TRUE);
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
    vtss_port_status_t *ps = &port_status.status;
    BOOL               fiber;
    char               *state, *speed;
    int                ct, xoff_add, xoff, xoff_sfp = 0, row, sfp, pcb106;
    vtss_board_info_t  board_info;

    /* Board type */
    vtss_board_info_get(&board_info);
    pcb106 = (board_info.board_type == VTSS_BOARD_SERVAL_PCB106_REF ? 1 : 0);

    /* Backround, SID, managed */
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%s|", usid, pcb106 ? "switch_serval.png" : "switch_small.png");
    xoff = (pcb106 ? 8*(PORT_ICON_WIDTH + PORT_BLOCK_GAP) : 0);
    cyg_httpd_write_chunked(p->outbuffer, ct);

    /* Emit port icons */
    for (iport = VTSS_PORT_NO_START; iport < board_info.port_count; iport++) {
        uport = iport2uport(iport);
        if (port_mgmt_conf_get(isid, iport, &conf) < 0 ||
            port_mgmt_status_get(isid, iport, &port_status) < 0)
            break;

        fiber = port_status.fiber;
        state = (conf.enable ? ((ps->link && !fiber) ? "link" : "down") : "disabled");
        speed = (conf.enable ?
                 (ps->link ? port_mgmt_mode_txt(iport, ps->speed, ps->fdx, fiber) : "Down") : "Disabled");

        sfp = 0;
        row = 0;
        xoff_add = PORT_ICON_WIDTH;
        if (iport == (board_info.port_count - 1)) {
            /* NPI port */
            if (pcb106) {
                xoff = 7*(PORT_ICON_WIDTH + PORT_BLOCK_GAP);
            } else {
                xoff_add += PORT_BLOCK_GAP;
            }
        } else if (port_status.cap & PORT_CAP_10M_HDX) {
            /* Copper port */
            xoff_sfp = PORT_BLOCK_GAP;
            if (pcb106 && (iport & 1)) {
                /* Odd ports on PCB106 are in the top row */
                row = 1;
                xoff_add = 0;
            }
        } else {
            /* SFP port */
            sfp = 1;
            if (pcb106) {
                if (port_status.cap & PORT_CAP_2_5G_FDX) {
                    /* SFP 2.5G ports are on the bottom row */
                    xoff_add += ((iport & 1) ? 0 : PORT_BLOCK_GAP);
                } else if (iport & 1) {
                    /* Odd ports on PCB106 are in the top row */
                    row = 1;
                    xoff_add = 0;
                }
            }
            if (xoff_sfp) {
                xoff_add += xoff_sfp;
                xoff_sfp = 0;
            }
        }
        xoff += xoff_add;
        port_httpd_write(p, uport, sfp, state, speed, xoff, row, FALSE);
        
        if (port_status.cap & PORT_CAP_DUAL_SFP_DETECT) {
            /* Dual media fiber port */
            sfp = 1;
            state = (conf.enable ? ((fiber && ps->link) ? "link" : "down") : "disabled");
            xoff_sfp = (2 * PORT_ICON_WIDTH + PORT_BLOCK_GAP);
            port_httpd_write(p, uport, sfp, state, speed, xoff + xoff_sfp, row, FALSE);
        }
    }
    if (pcb106) {
        non_eth_port_state(p);
    }
    cyg_httpd_write_chunked(",", 1);
}

