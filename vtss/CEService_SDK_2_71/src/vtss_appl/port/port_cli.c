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


#include "cli.h"
#include "port_cli.h"
#include "conf_api.h"
#include "port_api.h"
#include "port_custom_api.h"
#if defined(VTSS_SW_OPTION_QOS) || defined(VTSS_FEATURE_PVLAN)
#include "mgmt_api.h"
#endif
#include "cli_trace_def.h"
#include "vtss_api_if_api.h"

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
#include "ce_max_api.h"
#endif

typedef struct {
    /* Port speed/duplex */
    vtss_port_speed_t speed;
    vtss_fiber_port_speed_t fiber_speed;
    BOOL              fdx;
    BOOL              hdx;
    ulong             max_length;

    /* Keywords */
    BOOL              auto_keyword;
    BOOL              packets;
    BOOL              bytes;
    BOOL              errors;
    BOOL              discards;
    BOOL              discard;
    BOOL              filtered;
    BOOL              up;
    BOOL              down;

    /* NPI Port Debug command */
    u32          cpu_q_mask;
    
    u8                max_tags;
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    BOOL              host_interface_flag;
    BOOL              host_interface_list[VTSS_PORT_NO_END+1]; /* Can improve it */
    BOOL              xtend_reach_flag;
    BOOL              xtend_reach;
    BOOL              channel_status;
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

} port_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

static char port_mode_txt[128];
static char port_mode_hlp[512];
static char port_mode_cmd[128];

static char port_max_frame_hlp[128];

static char port_fc_cmd_ro[128];
static char port_fc_cmd_rw[128];

static char port_sfp_cmd[128];

static BOOL port_fc_cmd_disabled;

typedef struct {
    port_cap_t cap;
    const char *txt;
    const char *hlp;
} port_cap_entry_t;

static const port_cap_entry_t port_cap_table[] = {
    { PORT_CAP_AUTONEG,  "auto",    "Auto negotiation of speed and duplex" },
    { PORT_CAP_10M_HDX,  "10hdx",   "10 Mbps, half duplex" },
    { PORT_CAP_10M_FDX,  "10fdx",   "10 Mbps, full duplex" },
    { PORT_CAP_100M_HDX, "100hdx",  "100 Mbps, half duplex" },
    { PORT_CAP_100M_FDX, "100fdx",  "100 Mbps, full duplex" },
    { PORT_CAP_1G_FDX,   "1000fdx", "1 Gbps, full duplex" },
    { PORT_CAP_2_5G_FDX, "2500fdx", "2.5 Gbps, full duplex" },
    { PORT_CAP_10G_FDX,  "10gfdx",  "10 Gbps, full duplex" },
    { PORT_CAP_SFP_DETECT,      "sfp_auto_ams", "Auto detection of SFP" },
    { PORT_CAP_DUAL_FIBER,      "1000x_ams",    "1000BASE-X with automatic media sense" },
    { PORT_CAP_DUAL_FIBER_100FX,"100fx_ams",    "100BASE-FX with automatic media sense" },
    { PORT_CAP_DUAL_FIBER,          "1000x",    "1000BASE-X" },
    { PORT_CAP_DUAL_FIBER_100FX,    "100fx",    "100BASE-FX" },

    /* Last entry */
    { PORT_CAP_NONE, NULL, NULL }
};


static const char *mac_if2txt(vtss_port_interface_t mac_if_type)
{
    switch (mac_if_type) {
    case VTSS_PORT_INTERFACE_NO_CONNECTION: return "No connection";
    case VTSS_PORT_INTERFACE_LOOPBACK:      return "Internal loopback in MAC ";
    case VTSS_PORT_INTERFACE_INTERNAL:      return "Internal interface ";
    case VTSS_PORT_INTERFACE_MII:           return "MII (RMII does not exist) ";
    case VTSS_PORT_INTERFACE_GMII:          return "GMII ";
    case VTSS_PORT_INTERFACE_RGMII:         return "RGMII ";
    case VTSS_PORT_INTERFACE_TBI:           return "TBI ";
    case VTSS_PORT_INTERFACE_RTBI:          return "RTBI ";
    case VTSS_PORT_INTERFACE_SGMII:         return "SGMII ";
    case VTSS_PORT_INTERFACE_SERDES:        return "SERDES ";
    case VTSS_PORT_INTERFACE_VAUI:          return "VAUI ";
    case VTSS_PORT_INTERFACE_100FX:         return "100FX ";
    case VTSS_PORT_INTERFACE_XAUI:          return "XAUI ";
    case VTSS_PORT_INTERFACE_RXAUI:         return "RXAUI ";
    case VTSS_PORT_INTERFACE_XGMII:         return "XGMII ";
    case VTSS_PORT_INTERFACE_SPI4:          return "SPI4 ";
    case VTSS_PORT_INTERFACE_SGMII_CISCO:   return "SGMII_CISCO ";
    case VTSS_PORT_INTERFACE_QSGMII:        return "QSGMII ";
    default:                                return "Mac Interface not defined";
    }
}

void port_cli_init(void)
{
    port_isid_info_t       info;
    char                   *txt, *hlp;
    BOOL                   first = 1;
    const port_cap_entry_t *entry;

    /* register the size required for port req. structure */
    cli_req_size_register(sizeof(port_cli_req_t));

    (void)port_isid_info_get(VTSS_ISID_LOCAL, &info);
    
    /* Build port mode syntax and help */
    txt = port_mode_txt;
    hlp = port_mode_hlp;
    for (entry = port_cap_table; entry->cap != PORT_CAP_NONE; entry++) {
        if ((entry->cap & info.cap) == 0)
            continue;
        txt += sprintf(txt, "%s%s", first ? "" : "|", entry->txt);
        hlp += sprintf(hlp, "%-11s: %s\n", entry->txt, entry->hlp);
        first = 0;
    }
    strcpy(hlp, "(default: Show configured and current mode)");
    sprintf(port_mode_cmd, "Port Mode [<port_list>] [%s]", port_mode_txt);

    /* If no ports support flow control, it is a debug command */
    txt = port_fc_cmd_ro;
    if ((info.cap & PORT_CAP_FLOW_CTRL) == 0) {
        txt += sprintf(txt, "Debug ");
        port_fc_cmd_disabled = 1;
    }
    sprintf(txt, "Port Flow Control [<port_list>]");
    sprintf(port_fc_cmd_rw, "%s [enable|disable]", port_fc_cmd_ro);

    sprintf(port_max_frame_hlp, 
            "Port maximum frame size (%u-%u), default: Show maximum frame size",
            VTSS_MAX_FRAME_LENGTH_STANDARD, VTSS_MAX_FRAME_LENGTH_MAX);

    /* If no ports support SFP detection, it is a debug command */
    txt = port_sfp_cmd;
    if ((info.cap & PORT_CAP_SFP_DETECT || info.cap & PORT_CAP_DUAL_SFP_DETECT) == 0)
        txt += sprintf(txt, "Debug ");
    sprintf(txt, "Port SFP [<port_list>]");
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

/* Port command flags */
#define PORT_FLAGS_MODE       0x0001
#define PORT_FLAGS_FC         0x0002
#define PORT_FLAGS_STATE      0x0004
#define PORT_FLAGS_MAXLEN     0x0008
#define PORT_FLAGS_EXC        0x0020
#define PORT_FLAGS_ADV        0x0040
#define PORT_FLAGS_TAGS       0x0080
#define PORT_FLAGS_SFP        0x0100
#define PORT_FLAGS_FIBER_MODE 0x0200
#define PORT_FLAGS_LOOP       0x0400
#define PORT_FLAGS_CHANGE     0x0800
#define PORT_FLAGS_MAC_IF     0x1000
#define PORT_FLAGS_DEBUG      0x8000
#define PORT_FLAGS_ALL        (0xffff - PORT_FLAGS_ADV - PORT_FLAGS_TAGS - PORT_FLAGS_SFP -  PORT_FLAGS_LOOP - PORT_FLAGS_CHANGE -PORT_FLAGS_MAC_IF)

static void cli_cmd_port_error(vtss_usid_t usid, vtss_uport_no_t uport, const char *msg)
{
    if (vtss_stacking_enabled()) {
        CPRINTF("Switch %u, port %u", usid, uport);
    } else {
        CPRINTF("Port %u", uport);
    }
    CPRINTF(" %s\n", msg);
}


/* Port configuration */
static void cli_cmd_port_conf(cli_req_t *req, u16 flags)
{
    vtss_usid_t        usid;
    vtss_isid_t        isid;
    vtss_uport_no_t    uport;
    vtss_port_no_t     iport;
    u32                port_count;
    port_conf_t        conf;
    port_status_t      port_status;
    port_vol_status_t  vol_status;
    vtss_port_status_t *status = &port_status.status;
    BOOL               first, all = (flags & PORT_FLAGS_STATE);
    u8                 adv;
    char               buf[180], *p;
    port_cli_req_t     *port_req = req->module_req;
    port_isid_port_info_t info;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req))
        return;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        first = 1;
        port_count = port_isid_port_count(isid);
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            uport = iport2uport(iport);
            if (req->uport_list[uport] == 0 ||
                (req->set && port_isid_port_no_is_stack(isid, iport)) ||
                port_mgmt_conf_get(isid, iport, &conf) != VTSS_OK ||
                port_mgmt_status_get_all(isid, iport, &port_status) != VTSS_OK ||
                (port_req->down && status->link) || 
                (port_req->up && !status->link))
                continue;

            if (port_isid_port_info_get(isid, iport, &info) != VTSS_OK)
                continue;

            if (req->set || req->clear) {
                if (flags & PORT_FLAGS_MODE) {
                    conf.autoneg = port_req->auto_keyword;
                    if (!conf.autoneg) {
                        conf.speed = port_req->speed;
                        conf.fdx = port_req->fdx;
                    }
                }

                if (flags & PORT_FLAGS_FIBER_MODE){
                    if (info.cap & PORT_CAP_SPEED_DUAL_ANY_FIBER) {
                        T_I_PORT(iport, "fiber_speed = %d, conf.autoneg:%d", port_req->fiber_speed, conf.autoneg);
                        if (req->disable) {
                            conf.dual_media_fiber_speed = VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED;
                        } else {
                            conf.dual_media_fiber_speed = port_req->fiber_speed;
                        }
                    } else {
                        if (port_req->fiber_speed != VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED) {
                            cli_cmd_port_error(usid, uport, "does not support this fiber mode");
                            conf.dual_media_fiber_speed = VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED;
                            continue;
                        }
                    }
                } 

                if (flags & PORT_FLAGS_STATE)
                    conf.enable = req->enable;
                if (flags & PORT_FLAGS_FC)
                    conf.flow_control = req->enable;
                if (flags & PORT_FLAGS_MAXLEN)
                    conf.max_length = port_req->max_length;

                if (flags & PORT_FLAGS_EXC)
                    conf.exc_col_cont = (port_req->discard ? 0 : 1);
                if (flags & PORT_FLAGS_ADV) {
                  switch (port_req->speed) {
                  case VTSS_SPEED_1G:
                    adv = PORT_ADV_DIS_1G_FDX;
                    break;
                  case VTSS_SPEED_100M:
                    adv = (port_req->fdx ? PORT_ADV_DIS_100M_FDX : PORT_ADV_DIS_100M_HDX);
                    break;
                  case VTSS_SPEED_10M:
                    adv = (port_req->fdx ? PORT_ADV_DIS_10M_FDX : PORT_ADV_DIS_10M_HDX);
                    break;
                  default:
                    adv = PORT_ADV_DIS_ALL;
                    break;
                  }
                  if (req->enable)
                    conf.adv_dis &= ~adv;
                  else
                    conf.adv_dis |= adv;
                }
                
                if (flags & PORT_FLAGS_LOOP) {
                  if (req->disable)
                    conf.adv_dis &= ~PORT_ADV_UP_MEP_LOOP;
                  else
                    conf.adv_dis |= PORT_ADV_UP_MEP_LOOP;
                }
                conf.adv_dis &= ~(PORT_ADV_DIS_100M | PORT_ADV_DIS_10M);

                if (flags & PORT_FLAGS_CHANGE) {
                    if (req->clear && port_mgmt_counters_clear(isid, iport) != VTSS_OK) {
                        cli_cmd_port_error(usid, uport, "clear failed");
                    }
                    continue;
                }
                if (flags & PORT_FLAGS_TAGS) {
                    conf.max_tags = port_req->max_tags;
                }
                if (port_mgmt_conf_set(isid, iport, &conf) == PORT_ERROR_PARM)
                    cli_cmd_port_error(usid, uport, "does not support this mode");
            } else {
                if (first) {
                    cli_cmd_usid_print(usid, req, 1);
                    p = &buf[0];
                    p += sprintf(p, "Port  ");
                    if (flags & PORT_FLAGS_STATE)
                        p += sprintf(p, "State     ");

                    if (flags & PORT_FLAGS_MODE)
                        p += sprintf(p, "Mode         ");

                    if (flags & PORT_FLAGS_FC) {
                        p += sprintf(p, "Flow Control  ");
                        if (!all)
                            p += sprintf(p, "Rx Pause  Tx Pause  ");
                    }
                    if (flags & (PORT_FLAGS_MAXLEN | PORT_FLAGS_TAGS))
                        p += sprintf(p, "MaxFrame  ");
#if !defined(VTSS_ARCH_JAGUAR_1_CE_MAC)                    
                    if (flags & PORT_FLAGS_EXC)
                        p += sprintf(p, "Excessive  ");
#else
                    p += sprintf(p, "Port Role  ");
#endif
                    if (flags & PORT_FLAGS_SFP)
                        p += sprintf(p, "%-15s%-20s%-20s%-5s","SFP type","Vendor name","Vendor PN","Rev");

                    if (flags & PORT_FLAGS_MAC_IF)
                        p += sprintf(p, "MAC_IF      ");

                    if (flags & (PORT_FLAGS_MODE | PORT_FLAGS_FIBER_MODE))
                        p += sprintf(p, "Link  ");

                    if (flags & PORT_FLAGS_ADV) 
                        p += sprintf(p, "10hdx     10fdx     100hdx    100fdx    1000fdx");

                    if (flags & PORT_FLAGS_LOOP)
                        p += sprintf(p, "Loopback  ");

                    if (flags & PORT_FLAGS_CHANGE)
                        p += sprintf(p, "Link Up     Link Down    ");

                    cli_table_header(buf);
                    first = 0;
                }
                CPRINTF("%-6u", uport);
                if (flags & PORT_FLAGS_STATE)
                    CPRINTF("%-10s", cli_bool_txt(conf.enable));

                if (flags & PORT_FLAGS_MODE) {
                    BOOL no_fiber = (conf.dual_media_fiber_speed == VTSS_SPEED_FIBER_NOT_SUPPORTED_OR_DISABLED);
                    T_I_PORT(iport, "no_fiber = %d, fiber speed = %d auto neg = %d, fiber_speed_tx = %s, port_status.fiber:%d", 
                             no_fiber, conf.dual_media_fiber_speed,conf.autoneg, port_fiber_mgmt_mode_txt(conf.dual_media_fiber_speed, conf.autoneg), port_status.fiber);
                    if (no_fiber) {
                        CPRINTF("%-13s",
                                conf.autoneg ? "Auto" : port_mgmt_mode_txt(iport, conf.speed, conf.fdx, FALSE));
                    } else {
                        CPRINTF("%-13s",
                                port_fiber_mgmt_mode_txt(conf.dual_media_fiber_speed, conf.autoneg));
                    }
                }

                if (flags & PORT_FLAGS_FC) {
                    BOOL rx = (conf.autoneg ? (status->link ? status->aneg.obey_pause : 0) :
                               conf.flow_control);
                    BOOL tx = (conf.autoneg ? (status->link ? status->aneg.generate_pause : 0) :
                               conf.flow_control);
                    CPRINTF("%-14s", cli_bool_txt(conf.flow_control));
                    if (!all)
                        CPRINTF("%s  %s  ", cli_bool_txt(rx), cli_bool_txt(tx));
                }
                if (flags & (PORT_FLAGS_MAXLEN | PORT_FLAGS_TAGS)) {
                    CPRINTF("%-5u", conf.max_length);
                    CPRINTF("%-5s", (flags & PORT_FLAGS_TAGS) ? 
                            (conf.max_tags == PORT_MAX_TAGS_NONE ? "" :
                             conf.max_tags == PORT_MAX_TAGS_ONE ? "+4" : "+8") : "");
                }
#if !defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
                if (flags & PORT_FLAGS_EXC)
                    CPRINTF("%-11s", conf.exc_col_cont ? "Restart" : "Discard");
#else
                if (vtss_port_is_host(0, uport-1) == TRUE) {
                    CPRINTF("%-11s", "Host");
                } else {
                    CPRINTF("%-11s", "Line");
                }
#endif
                if (flags & (PORT_FLAGS_MODE | PORT_FLAGS_FIBER_MODE)) {
                    if (status->link) {
                        sprintf(buf, "%s", 
                                port_mgmt_mode_txt(iport, status->speed, status->fdx, port_status.fiber));
                    } else if (isid != VTSS_ISID_LOCAL &&
                               port_vol_status_get(PORT_USER_CNT, isid, iport, 
                                                   &vol_status) == VTSS_OK &&
                               vol_status.conf.disable && vol_status.user != PORT_USER_STATIC)
                        sprintf(buf, "Down (%s)", vol_status.name);
                    else
                        strcpy(buf, "Down");
                    CPRINTF(buf);
                }

                if (flags & PORT_FLAGS_SFP) {
                    CPRINTF("%-15s",sfp_if2txt(port_status.sfp.type));
                    CPRINTF("%-20s",port_status.sfp.vendor_name);
                    CPRINTF("%-20s",port_status.sfp.vendor_pn);
                    CPRINTF("%-5s",port_status.sfp.vendor_rev);
                }
                if (flags & PORT_FLAGS_MAC_IF) {
                    CPRINTF("%-12s", mac_if2txt(port_status.mac_if));
                }

                if (flags & PORT_FLAGS_ADV) {
                  adv = ~conf.adv_dis;
                  CPRINTF("%-10s", cli_bool_txt(adv & PORT_ADV_DIS_10M_HDX));
                  CPRINTF("%-10s", cli_bool_txt(adv & PORT_ADV_DIS_10M_FDX));
                  CPRINTF("%-10s", cli_bool_txt(adv & PORT_ADV_DIS_100M_HDX));
                  CPRINTF("%-10s", cli_bool_txt(adv & PORT_ADV_DIS_100M_FDX));
                  CPRINTF("%-10s", cli_bool_txt(adv & PORT_ADV_DIS_1G_FDX));
                }
                if (flags & PORT_FLAGS_LOOP) {
                    CPRINTF("%-10s", cli_bool_txt(conf.adv_dis & PORT_ADV_UP_MEP_LOOP));
                }

                if (flags & PORT_FLAGS_CHANGE) {
                    CPRINTF("%-10u  %-10u", 
                            port_status.port_up_count, port_status.port_down_count);
                }
                
                CPRINTF("\n");
            }
        }
    }
}

/* Print two counters in columns */
static void cli_cmd_stats(const char *col1, const char *col2, ulonglong c1, ulonglong c2)
{
    char buf[80];

    sprintf(buf, "Rx %s:", col1);
    CPRINTF("%-19s%19llu   ", buf, c1);
    if (col2 != NULL) {
        sprintf(buf, "Tx %s:", strlen(col2) ? col2 : col1);
        CPRINTF("%-19s%19llu", buf, c2);
    }
    CPRINTF("\n");
}

/* Print counters in two columns with header */
static void cli_cmd_stat_port(vtss_uport_no_t uport, BOOL *first, const char *name, BOOL tx,
                              ulonglong c1, ulonglong c2)
{
    char buf[80], *p;

    if (*first) {
        *first = 0;
        p = &buf[0];
        p += sprintf(p, "Port  Rx %-17s", name);
        if (tx)
            sprintf(p, "Tx %-17s", name);
        cli_table_header(buf);
    }
    CPRINTF("%-2u    %-20llu", uport, c1);
    if (tx)
        CPRINTF("%-20llu", c2);
    CPRINTF("\n");
}

/* Port statistics */
static void cli_cmd_port_stats_internal(cli_req_t *req, BOOL managed)
{
    vtss_usid_t          usid;
    vtss_isid_t          isid;
    vtss_uport_no_t      uport;
    vtss_port_no_t       iport;
    u32                  port_count;
    port_status_t        port_status;
    vtss_port_status_t   *status = &port_status.status;
    vtss_port_counters_t counters;
    BOOL                 first;
#ifdef VTSS_SW_OPTION_QOS
    vtss_prio_t          prio;
#endif
    port_cli_req_t       *port_req = req->module_req;

    if (cli_cmd_switch_none(req))
        return;

    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END) {
            continue;
        }

        first = 1;
        port_count = port_isid_port_count(isid);
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            uport = iport2uport(iport);
            if (req->uport_list[uport] == 0 ||
                port_mgmt_status_get_all(isid, iport, &port_status) != VTSS_OK ||
                (port_req->down && status->link) || 
                (port_req->up && !status->link))
                continue;

            /* Handle 'clear' command */
            if (req->clear) {
                if (port_mgmt_counters_clear(isid, iport) != VTSS_OK)
                    return;
                continue;
            }

            if (first)
                cli_cmd_usid_print(usid, req, 0);

            /* Get counters for remaining commands */
            if (port_mgmt_counters_get(isid, iport, &counters) != VTSS_OK)
                continue;

            /* Handle 'packet' command */
            if (port_req->packets) {
                cli_cmd_stat_port(uport, &first, "Packets", 1,
                                  counters.rmon.rx_etherStatsPkts,
                                  counters.rmon.tx_etherStatsPkts);
                continue;
            }

            /* Handle 'bytes' command */
            if (port_req->bytes) {
                cli_cmd_stat_port(uport, &first, "Octets", 1,
                                  counters.rmon.rx_etherStatsOctets,
                                  counters.rmon.tx_etherStatsOctets);
                continue;
            }

            /* Handle 'errors' command */
            if (port_req->errors) {
                cli_cmd_stat_port(uport, &first, "Errors", 1,
                                  counters.if_group.ifInErrors,
                                  counters.if_group.ifOutErrors);
                continue;
            }

            /* Handle 'discards' command */
            if (port_req->discards) {
                cli_cmd_stat_port(uport, &first, "Discards", 1,
                                  counters.if_group.ifInDiscards,
                                  counters.if_group.ifOutDiscards);
                continue;
            }

            /* Handle 'filtered' command */
            if (port_req->filtered) {
                cli_cmd_stat_port(uport, &first, "Filtered", 0,
                                  counters.bridge.dot1dTpPortInDiscards,
                                  0);
                continue;
            }

#ifdef VTSS_SW_OPTION_QOS
            /* Handle '<class>' specified */
            if (req->class_spec == CLI_SPEC_VAL) {
                cli_cmd_stat_port(uport, &first, mgmt_prio2txt(req->class_, 0), 1,
                                  counters.prop.rx_prio[req->class_ - VTSS_PRIO_START],
                                  counters.prop.tx_prio[req->class_ - VTSS_PRIO_START]);
                continue;
            }
#endif /* VTSS_SW_OPTION_QOS */
            /* Handle default command */
            CPRINTF("%sPort %u Statistics:\n\n", first ? "" : "\n", uport);
            first = 0;
            cli_cmd_stats("Packets", "",
                          counters.rmon.rx_etherStatsPkts,
                          counters.rmon.tx_etherStatsPkts);
            cli_cmd_stats("Octets", "",
                          counters.rmon.rx_etherStatsOctets,
                          counters.rmon.tx_etherStatsOctets);
            cli_cmd_stats("Unicast", "",
                          counters.if_group.ifInUcastPkts,
                          counters.if_group.ifOutUcastPkts);
            cli_cmd_stats("Multicast", "",
                          counters.rmon.rx_etherStatsMulticastPkts,
                          counters.rmon.tx_etherStatsMulticastPkts);
            cli_cmd_stats("Broadcast", "",
                          counters.rmon.rx_etherStatsBroadcastPkts,
                          counters.rmon.tx_etherStatsBroadcastPkts);
            cli_cmd_stats("Pause", "",
                          counters.ethernet_like.dot3InPauseFrames,
                          counters.ethernet_like.dot3OutPauseFrames);
            CPRINTF("\n");
            cli_cmd_stats("64", "",
                          counters.rmon.rx_etherStatsPkts64Octets,
                          counters.rmon.tx_etherStatsPkts64Octets);
            cli_cmd_stats("65-127", "",
                          counters.rmon.rx_etherStatsPkts65to127Octets,
                          counters.rmon.tx_etherStatsPkts65to127Octets);
            cli_cmd_stats("128-255", "",
                          counters.rmon.rx_etherStatsPkts128to255Octets,
                          counters.rmon.tx_etherStatsPkts128to255Octets);
            cli_cmd_stats("256-511", "",
                          counters.rmon.rx_etherStatsPkts256to511Octets,
                          counters.rmon.tx_etherStatsPkts256to511Octets);
            cli_cmd_stats("512-1023", "",
                          counters.rmon.rx_etherStatsPkts512to1023Octets,
                          counters.rmon.tx_etherStatsPkts512to1023Octets);
            cli_cmd_stats("1024-1526", "",
                          counters.rmon.rx_etherStatsPkts1024to1518Octets,
                          counters.rmon.tx_etherStatsPkts1024to1518Octets);
            cli_cmd_stats("1527-    ", "",
                          counters.rmon.rx_etherStatsPkts1519toMaxOctets,
                          counters.rmon.tx_etherStatsPkts1519toMaxOctets);
            CPRINTF("\n");
#ifdef VTSS_SW_OPTION_QOS
            for (prio = VTSS_PRIO_START; prio < VTSS_PRIO_END; prio++) {
                cli_cmd_stats(mgmt_prio2txt(prio, 0), "",
                              counters.prop.rx_prio[prio - VTSS_PRIO_START],
                              counters.prop.tx_prio[prio - VTSS_PRIO_START]);
            }
            CPRINTF("\n");
#endif /* VTSS_SW_OPTION_QOS */
            cli_cmd_stats("Drops", "",
                          counters.rmon.rx_etherStatsDropEvents,
                          counters.rmon.tx_etherStatsDropEvents);
            cli_cmd_stats("CRC/Alignment", "Late/Exc. Coll.",
                          counters.rmon.rx_etherStatsCRCAlignErrors,
                          counters.if_group.ifOutErrors);
            cli_cmd_stats("Undersize", NULL, counters.rmon.rx_etherStatsUndersizePkts, 0);
            cli_cmd_stats("Oversize", NULL, counters.rmon.rx_etherStatsOversizePkts, 0);
            cli_cmd_stats("Fragments", NULL, counters.rmon.rx_etherStatsFragments, 0);
            cli_cmd_stats("Jabbers", NULL, counters.rmon.rx_etherStatsJabbers, 0);
            /* Bridge counters */
            cli_cmd_stats("Filtered", NULL, counters.bridge.dot1dTpPortInDiscards, 0);
        } /* Port loop */
    } /* USID loop */
}

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
static void cli_cmd_port_oob_flow_control(cli_req_t *req)
{
    port_cli_req_t    *port_req = req->module_req;
    vtss_port_no_t    hmda = ce_mac_mgmt_hmdx_get(TRUE), hmdb = ce_mac_mgmt_hmdx_get(FALSE);
    BOOL              hmda_flag = FALSE, hmdb_flag = FALSE;
    vtss_host_mode_t  host_mode;
    port_oobfc_conf_t oobfc;
    vtss_rc           rc;

    if (port_req->host_interface_flag == FALSE) { /* Default case */
        /* Initialize the host interface list */
        memset(port_req->host_interface_list, 0, sizeof(port_req->host_interface_list));
        port_req->host_interface_list[hmda+1] = TRUE;
        if (hmdb) {
            port_req->host_interface_list[hmdb+1] = TRUE;
        }
    }

    if (ce_max_mgmt_current_host_mode_get(&host_mode) != VTSS_RC_OK) {
        return;
    }

    if (port_req->host_interface_list[hmda+1] == TRUE) {
        hmda_flag = TRUE;
    }

    if (port_req->host_interface_list[hmdb+1] == TRUE) {
        if (host_mode == VTSS_HOST_MODE_2 || host_mode == VTSS_HOST_MODE_3) {
            CPRINTF ("Current HOST mode does not support HMDB\n");
            return;
        }
        hmdb_flag = TRUE;
    }

    if (hmda_flag == FALSE && hmdb_flag == FALSE) {
        return;
    }

    if (!req->set) {
        CPRINTF ("PORT  CHANNEL STATUS  EXTENDED REACH \n");
        CPRINTF ("----  --------------  -------------- \n");

        if (hmda_flag == TRUE) {

            rc = port_mgmt_oobfc_conf_get(hmda, &oobfc);
            if (rc != VTSS_RC_OK) {
                return;
            }

            CPRINTF("%-4u  %-14s  %-11s\n", hmda + 1,
                    oobfc.channel_status ? "enabled" : "disabled",
                    oobfc.xtend_reach ? "enabled" : "disabled");
        }
        if (hmdb_flag == TRUE) {

            rc = port_mgmt_oobfc_conf_get(hmdb, &oobfc);
            if (rc != VTSS_RC_OK) {
                return;
            }

            CPRINTF("%-4u  %-14s  %-11s\n", hmdb + 1,
                    oobfc.channel_status ? "enabled" : "disabled",
                    oobfc.xtend_reach ? "enabled" : "disabled");
        }
    } else {

        do {
            if (hmda_flag == TRUE) { /* Configure HMDA */
                rc = port_mgmt_oobfc_conf_get(hmda, &oobfc);
                if (rc != VTSS_RC_OK) {
                    break;
                }
                oobfc.channel_status = port_req->channel_status;
                if (port_req->xtend_reach_flag == TRUE) {
                    oobfc.xtend_reach = port_req->xtend_reach;
                }
                rc = port_mgmt_oobfc_conf_set(hmda, &oobfc);
                if (rc != VTSS_RC_OK) {
                    break;
                }
            }
            if (hmdb_flag == TRUE) {
                rc = port_mgmt_oobfc_conf_get(hmdb, &oobfc);
                if (rc != VTSS_RC_OK) {
                    break;
                }
                oobfc.channel_status = port_req->channel_status;
                if (port_req->xtend_reach_flag == TRUE) {
                    oobfc.xtend_reach = port_req->xtend_reach;
                }
                rc = port_mgmt_oobfc_conf_set(hmdb, &oobfc);
                if (rc != VTSS_RC_OK) {
                    break;
                }
            }
        } while(0); /* end of do-while */
    }
}
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

static void cli_cmd_port_disp(cli_req_t *req)
{
    u16 flags = PORT_FLAGS_ALL;
    
    if(!req->set) {
        cli_header("Port Configuration", 1);
    }

    if (port_fc_cmd_disabled)
        flags -= PORT_FLAGS_FC;

    cli_cmd_port_conf(req, flags);
}

static void cli_cmd_port_mode(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_MODE | PORT_FLAGS_FIBER_MODE);
}

static void cli_cmd_port_sfp(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_SFP | PORT_FLAGS_MAC_IF);
}

static void cli_cmd_port_flow_control(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_FC);
}

static void cli_cmd_port_state(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_STATE);
}

static void cli_cmd_port_max_frame(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_MAXLEN);
}

static void cli_cmd_port_exc_col_cont(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_EXC);
}

static void cli_cmd_port_stats(cli_req_t *req)
{
    cli_cmd_port_stats_internal(req, 1);
}

#if VTSS_UI_OPT_VERIPHY
static void cli_cmd_port_veriphy(cli_req_t *req)
{

    vtss_usid_t               usid;
    vtss_isid_t               isid;
    BOOL                      first;
    port_isid_port_info_t     info;
    port_veriphy_mode_t       mode[VTSS_PORT_ARRAY_SIZE];
    vtss_phy_veriphy_result_t veriphy;
    char                      buf[80], *p;
    int                       i;
    port_iter_t               pit;

    if (cli_cmd_switch_none(req))
        return;

    /* Start veriPHY on ports */
    CPRINTF("Starting VeriPHY, please wait\n");
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END)
            continue;

        // Loop through all front ports
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
          while (port_iter_getnext(&pit)) {
            mode[pit.iport] = (req->uport_list[pit.uport] == 0 ||
                               port_isid_port_info_get(isid, pit.iport, &info) != VTSS_OK ||
                               (info.cap & PORT_CAP_1G_PHY) == 0 ? PORT_VERIPHY_MODE_NONE :
                               PORT_VERIPHY_MODE_FULL);
          }
        }
        
        if (port_mgmt_veriphy_start(isid, mode) != VTSS_OK) {
            T_E("Failed starting VeriPhy");
            return;
        }
    }

    /* Get veriPHY result */
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END)
            continue;

        first = 1;
        if (port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI) == VTSS_OK) {
          while (port_iter_getnext(&pit)) {
            T_N("%d, %d, %d %d", (req->uport_list[pit.uport] == 0), (port_isid_port_info_get(isid, pit.iport, &info) != VTSS_OK),
                (info.cap & PORT_CAP_1G_PHY) == 0, (port_mgmt_veriphy_get(isid, pit.iport, &veriphy, 20) != VTSS_OK));

            if (req->uport_list[pit.uport] == 0 ||
                port_isid_port_info_get(isid, pit.iport, &info) != VTSS_OK ||
                (info.cap & PORT_CAP_1G_PHY) == 0 ||
                port_mgmt_veriphy_get(isid, pit.iport, &veriphy, 40) != VTSS_OK)
                continue;

            if (first) {
                cli_cmd_usid_print(usid, req, 0);
                cli_table_header("Port   Pair A   Length   Pair B   Length   Pair C   Length   Pair D   Length");
                first = 0;
            }
            p = &buf[0];
            p += sprintf(p, "%-2u   ", pit.uport);
            for (i = 0; i < 4; i++) {
                p += sprintf(p, "  %-8s %-3d    ",
                             port_mgmt_veriphy_txt(veriphy.status[i]), veriphy.length[i]);
            }
            CPRINTF("%s\n", buf);
          }
        }
    }
}
#endif

#if defined(CLI_CUSTOM_PORT_TXT)
static void cli_cmd_port_numbers_req(cli_req_t *req)
{
    cli_cmd_port_numbers();
}
#endif

static void cli_cmd_port_debug_conf(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_ALL | PORT_FLAGS_TAGS | PORT_FLAGS_DEBUG);
}

static void cli_cmd_port_debug_stats(cli_req_t *req)
{
    cli_cmd_port_stats_internal(req, 0);
}

static void cli_cmd_port_debug_adv(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_ADV);
}

static void cli_cmd_port_debug_loop(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_LOOP);
}

static void cli_cmd_port_debug_tags(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_TAGS);
}

#if defined(VTSS_FEATURE_NPI)
static void cli_cmd_port_debug_npi(cli_req_t *req)
{
    vtss_packet_rx_conf_t    rx_conf;
    port_cli_req_t           *port_req = req->module_req;

    vtss_appl_api_lock();
    if (vtss_packet_rx_conf_get(NULL, &rx_conf) == VTSS_OK) {
        int i;
        for (i = 0; i < VTSS_PACKET_RX_QUEUE_CNT; i++) {
            rx_conf.queue[i].npi.enable = !!(port_req->cpu_q_mask & VTSS_BIT(i));
        }

        if (vtss_packet_rx_conf_set(0, &rx_conf) != VTSS_OK) {
            CPRINTF("Error: vtss_packet_rx_conf_set() failed\n");
        } else {
            vtss_npi_conf_t conf;
            conf.enable  = req->enable;
            conf.port_no = uport2iport(req->uport);
            if (vtss_npi_conf_set(0, &conf) != VTSS_OK) {
                CPRINTF("Error: vtss_npi_conf_set() failed\n");
            }
        }
    }
    vtss_appl_api_unlock();

    return;
}
#endif /* VTSS_FEATURE_NPI */

#if defined(VTSS_ARCH_LUTON28)
/* Print counters in two columns */
static void cli_cmd_port_cnt(char *col1, char *col2, ulonglong c1, ulonglong c2)
{
    char buf[80];

    sprintf(buf, "c_rx%s:", col1);
    CPRINTF("%-15s%20llu   ", buf, c1);
    if (col2 != NULL) {
        sprintf(buf, "c_tx%s:", strlen(col2) ? col2 : col1);
        CPRINTF("%-15s%20llu", buf, c2);
    }
    CPRINTF("\n");
}

static void cli_cmd_port_debug_counters(cli_req_t *req)
{
    vtss_rc         rc;
    vtss_isid_t     isid;
    vtss_uport_no_t uport;
    vtss_port_no_t  iport;
    u32             port_count;
    uint            chip_port, chip_end, i, blk, sub;
    ulong           addr, msb, lsb;
    ulonglong       c[64];

    memset(c, 0, sizeof(c));

    isid = req->stack.isid_debug;
    port_count = port_isid_port_count(isid);
    for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0)
            continue;

        chip_port = port_custom_table[iport].map.chip_port;
        chip_end = chip_port;
#if VTSS_OPT_INT_AGGR
        /* Two chip ports per VAUI port */
        if (port_custom_table[iport].mac_if == VTSS_PORT_INTERFACE_VAUI)
            chip_end++;
#endif /* VTSS_OPT_INT_AGGR */

        for ( ; chip_port <= chip_end; chip_port++) {

            /* Calculate block and subblock */
            blk = (chip_port < 16 ? 1 : 6);
            sub = (chip_port & 0xf);

            /* Stop port polling */
            if (misc_suspend_resume(isid, 0) != VTSS_OK)
                continue;
            
            /* Read and store counter values */
            addr = misc_l28_reg_addr(blk, sub, 0xdd);
            if ((rc = misc_debug_reg_write(isid, 0, addr, 0x3000)) == VTSS_OK) {
                addr = misc_l28_reg_addr(blk, sub, 0xdc);
                for (i = 0; i < 64; i++) {
                    if ((rc = misc_debug_reg_read(isid, 0, addr, &msb)) != VTSS_OK ||
                        (rc = misc_debug_reg_read(isid, 0, addr, &lsb)) != VTSS_OK)
                        break;
                    c[i] = msb;
                    c[i] = ((c[i] << 32) | lsb);
                }
            }

            /* Restart port polling */
            if (misc_suspend_resume(isid, 1) != VTSS_OK || rc != VTSS_OK)
                continue;
            
            /* Print counters */
            CPRINTF("\nPort %u (chip port %d) Counters:\n\n", uport, chip_port);
            cli_cmd_port_cnt("pkt", "", c[1], c[33]);
            cli_cmd_port_cnt("oct", "", c[0], c[32]);
            cli_cmd_port_cnt("uc", "", c[29], c[56]);
            cli_cmd_port_cnt("mc", "", c[5], c[37]);
            cli_cmd_port_cnt("bc", "", c[4], c[36]);
            cli_cmd_port_cnt("bcmc", "", c[2], c[34]);
            cli_cmd_port_cnt("pause", "", c[13], c[45]);
            cli_cmd_port_cnt("badpkt", "", c[3], c[35]);
            cli_cmd_port_cnt("goodpkt", "", c[23], c[50]);
            CPRINTF("\n");
            cli_cmd_port_cnt("64", "", c[6], c[38]);
            cli_cmd_port_cnt("65", "", c[7], c[39]);
            cli_cmd_port_cnt("128", "", c[8], c[40]);
            cli_cmd_port_cnt("256", "", c[9], c[41]);
            cli_cmd_port_cnt("512", "", c[10], c[42]);
            cli_cmd_port_cnt("1024", "", c[11], c[43]);
            cli_cmd_port_cnt("jumbo", "", c[12], c[44]);
            CPRINTF("\n");
            cli_cmd_port_cnt("class0", "", c[24], c[51]);
            cli_cmd_port_cnt("class1", "", c[25], c[52]);
            cli_cmd_port_cnt("class2", "", c[26], c[53]);
            cli_cmd_port_cnt("class3", "", c[27], c[54]);
            CPRINTF("\n");
            cli_cmd_port_cnt("totdrop", "", c[28], c[55]);
            cli_cmd_port_cnt("drop", "ovfl", c[14], c[46]);
            cli_cmd_port_cnt("localdrop", "drop", c[15], c[47]);
            cli_cmd_port_cnt("catdrop", "col", c[16], c[48]);
            cli_cmd_port_cnt("crc", "cfidrop", c[17], c[49]);
            cli_cmd_port_cnt("sht", "ageing", c[57], 0);
            cli_cmd_port_cnt("long", NULL, c[19], 0);
            cli_cmd_port_cnt("frag", NULL, c[20], 0);
            cli_cmd_port_cnt("jab", NULL, c[21], 0);
            cli_cmd_port_cnt("ctrl", NULL, c[22], 0);
        }
    }
}
#endif /* VTSS_ARCH_LUTON28 */

static void cli_cmd_port_reg(BOOL *first, vtss_module_id_t module_id, cyg_tick_count_t ticks)
{
    ulong msec;
    
    if (*first) {
        *first = 0;
        cli_table_header("Module           Max Callback [msec]");
    } 
    msec = 1000*ticks/CYGNUM_HAL_RTC_DENOMINATOR;
    CPRINTF("%-15.15s  %u\n", vtss_module_names[module_id], msec);
}

#if 0 /* Test code, currently not used */
/* BPDU forwarding test using port 0-3 */
static void bpdu_test(void)
{
    vtss_packet_rx_conf_t rx_conf;
    vtss_port_no_t        port_no;
    BOOL                  member[VTSS_PORT_ARRAY_SIZE];
    vtss_vlan_tx_tag_t    tx_tag[VTSS_PORT_ARRAY_SIZE];
    vtss_vcl_port_conf_t  port_conf;
    vtss_vce_t            vce;
    int                   i;

    /* Disable CPU redirect of BPDUs */
    if (vtss_packet_rx_conf_get(NULL, &rx_conf) == VTSS_RC_OK) {
        rx_conf.reg.bpdu_cpu_only = 0;
        (void)vtss_packet_rx_conf_set(NULL, &rx_conf);
    }

    /* Disable forwarding for port 0-3 in MSTI 0 */
    for (port_no = 0; port_no < 4; port_no++) {
        (void)vtss_mstp_port_msti_state_set(NULL, port_no, 0, VTSS_STP_STATE_DISCARDING);
    }

    /* Map VLAN 4095 to MSTI 1 */
    (void)vtss_mstp_vlan_msti_set(NULL, 4095, 1);

    /* Enable forwarding for port 0-3 in MSTI 1 */
    for (port_no = 0; port_no < 4; port_no++) {
        (void)vtss_mstp_port_msti_state_set(NULL, port_no, 1, VTSS_STP_STATE_FORWARDING);
    }
       
    /* Include port 0-3 in VLAN 4095 */
    if (vtss_vlan_port_members_get(NULL, 4095, member) == VTSS_RC_OK) {
        for (port_no = 0; port_no < 4; port_no++) {
            member[port_no] = 1;
        }
        (void)vtss_vlan_port_members_set(NULL, 4095, member);
    }

    /* Transmit VLAN 4095 as untagged on port 0-3 */
    if (vtss_vlan_tx_tag_get(NULL, 4095, tx_tag) == VTSS_RC_OK) {
        for (port_no = 0; port_no < 4; port_no++) {
            tx_tag[port_no] = VTSS_VLAN_TX_TAG_DISABLE;
        }
        (void)vtss_vlan_tx_tag_set(NULL, 4095, tx_tag);
    }

    /* Enable VCL DMAC/DIP matching for port 0-3 */
    for (port_no = 0; port_no < 4; port_no++) {
        if (vtss_vcl_port_conf_get(NULL, port_no, &port_conf) == VTSS_RC_OK) {
            port_conf.dmac_dip = 1;
            (void)vtss_vcl_port_conf_set(NULL, port_no, &port_conf);
        }
    }

    /* Add VCE to classify BPDUs to VLAN 4095 for port 0-3 */
    if (vtss_vce_init(NULL, VTSS_VCE_TYPE_ANY, &vce) == VTSS_RC_OK) {
        vce.id = 1;
        for (port_no = 0; port_no < 4; port_no++) {
            vce.key.port_list[port_no] = 1;
        }
        vce.key.mac.smac.value[0] = 0x01;
        vce.key.mac.smac.value[1] = 0x80;
        vce.key.mac.smac.value[2] = 0xc2;
        for (i = 0; i < 6; i++) {
            vce.key.mac.smac.mask[i] = (i == 5 ? 0xf0 : 0xff);
        }
        vce.action.vid = 4095;
        (void)vtss_vce_add(NULL, VTSS_VCE_ID_LAST, &vce);
    }
}
#endif

static void cli_cmd_port_debug_regs(cli_req_t *req)
{
    port_change_reg_t        local_reg;
    port_global_change_reg_t global_reg;
    port_shutdown_reg_t      shutdown_reg;
    BOOL                     first;

    /* Local port change registrations */
    local_reg.module_id = VTSS_MODULE_ID_NONE;
    first = 1;
    while (port_change_reg_get(&local_reg, req->clear) == VTSS_RC_OK) {
        if (first)
            cli_header("Local Port Change Registrations", 1);
        cli_cmd_port_reg(&first, local_reg.module_id, local_reg.max_ticks);
    }

    /* Global port change registrations */
    global_reg.module_id = VTSS_MODULE_ID_NONE;
    first = 1;
    while (port_global_change_reg_get(&global_reg, req->clear) == VTSS_RC_OK) {
        if (first)
            cli_header("Global Port Change Registrations", 1);
        cli_cmd_port_reg(&first, global_reg.module_id, global_reg.max_ticks);
    }
    
    /* Local port shutdown registrations */
    shutdown_reg.module_id = VTSS_MODULE_ID_NONE;
    first = 1;
    while (port_shutdown_reg_get(&shutdown_reg, req->clear) == VTSS_RC_OK) {
        if (first)
            cli_header("Local Port Shutdown Registrations", 1);
        cli_cmd_port_reg(&first, shutdown_reg.module_id, shutdown_reg.max_ticks);
    }
}

static void cli_cmd_port_debug_change(cli_req_t *req)
{
    cli_cmd_port_conf(req, PORT_FLAGS_CHANGE);
}

static void cli_cmd_port_debug_caps(cli_req_t *req)
{
    vtss_usid_t        usid;
    vtss_isid_t        isid;
    vtss_uport_no_t    uport;
    vtss_port_no_t     iport;
    port_status_t      port_status;
    vtss_port_status_t *status = &port_status.status;
    BOOL               first = 1;
    u32                port_count, i, m;
    port_cli_req_t     *port_req = req->module_req;
        
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END)
            continue;
        
        first = 1;
        port_count = port_isid_port_count(isid);
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            uport = iport2uport(iport);
            if (req->uport_list[uport] == 0 || 
                port_mgmt_status_get_all(isid, iport, &port_status) != VTSS_OK ||
                (port_req->down && status->link) || 
                (port_req->up && !status->link))
                continue;

            if (first) {
                cli_cmd_usid_print(usid, req, 1);
                cli_table_header("Port  CAP         Capabilities");
            }
            first = 1;
            CPRINTF("%-6u0x%08x  ", uport, port_status.cap);
            for (i = 0; i < 32; i++) {
                m = (1 << i);
                if (port_status.cap & m) {
                    CPRINTF("%s%s", 
                            first ? "" : "-",
                            m == PORT_CAP_AUTONEG ? "ANEG" : 
                            m == PORT_CAP_10M_HDX ? "10MH" : 
                            m == PORT_CAP_10M_FDX ? "10MF" :
                            m == PORT_CAP_100M_HDX ? "100MH" : 
                            m == PORT_CAP_100M_FDX ? "100MF" :
                            m == PORT_CAP_1G_FDX ? "1GF" : 
                            m == PORT_CAP_2_5G_FDX ? "2.5GF" :
                            m == PORT_CAP_5G_FDX ? "5GF" : 
                            m == PORT_CAP_10G_FDX ? "10GF" :
                            m == PORT_CAP_FLOW_CTRL ? "FC" : 
                            m == PORT_CAP_COPPER ? "CU" :
                            m == PORT_CAP_FIBER ? "FI" : 
                            m == PORT_CAP_DUAL_COPPER ? "DUAL_CU" :
                            m == PORT_CAP_DUAL_FIBER ? "DUAL_FI" : 
                            m == PORT_CAP_SD_ENABLE ? "SD_ENA" :
                            m == PORT_CAP_SD_HIGH ? "SD_HIGH" :
                            m == PORT_CAP_SD_INTERNAL ? "SD_INT" :
                            m == PORT_CAP_DUAL_FIBER_100FX ? "DUAL_FI_100FX" :
                            m == PORT_CAP_DUAL_COPPER_100FX ? "DUAL_CU_100FX" :
                            m == PORT_CAP_XAUI_LANE_FLIP ? "XAUI_FLIP" :
                            m == PORT_CAP_VTSS_10G_PHY ? "10G_PHY" :
                            m == PORT_CAP_SFP_DETECT ? "SFP_DET" :
                            m == PORT_CAP_STACKING ? "STACK" : 
			    m == PORT_CAP_DUAL_SFP_DETECT ? "PORT_CAP_DUAL_SFP_DETECT" : "?");
                    first = 0;
                }
            }
            CPRINTF("%s\n", first ? "None" : "");
            first = 0;
        }   
    }
}

static void cli_cmd_port_debug_info(cli_req_t *req)
{
    vtss_usid_t      usid;
    vtss_isid_t      isid;
    port_isid_info_t info;
    BOOL             header = 1;
    int              i, type;
    vtss_port_no_t   iport;
    
    for (usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        if ((isid = req->stack.isid[usid]) == VTSS_ISID_END)
            continue;
        if (port_isid_info_get(isid, &info) != VTSS_OK)
            continue;
        if (header)
            cli_table_header("USID  ISID  Board Type                        Port Count  Port_0  Port_1");
        header = 0;
        type = info.board_type;
        CPRINTF("%-6u%-6u%-28s(%u)  %-12u", 
                usid,
                isid, 
                type == VTSS_BOARD_UNKNOWN ? "Unknown" :
                type == VTSS_BOARD_ESTAX_34_REF ? "ESTAX_34_REF" :
                type == VTSS_BOARD_ESTAX_34_ENZO ? "ESTAX_34_ENZO" :
                type == VTSS_BOARD_ESTAX_34_ENZO_SFP ? "ESTAX_34_ENZO_SFP" :
                type == VTSS_BOARD_LUTON10_REF ? "LUTON10_REF" :
                type == VTSS_BOARD_LUTON26_REF ? "LUTON26_REF" :
                type == VTSS_BOARD_JAG_CU24_REF ? "JAG_CU24_REF" :
                type == VTSS_BOARD_JAG_SFP24_REF ? "JAG_SFP24_REF" :
                type == VTSS_BOARD_JAG_CU48_REF ? "JAG_CU48_REF" :
                type == VTSS_BOARD_SERVAL_REF ? "SERVAL_REF" :
                type == VTSS_BOARD_SERVAL_PCB106_REF ? "VTSS_BOARD_SERVAL_PCB106_REF" : "?",
                type,
                info.port_count);
        for (i = 0; i < 2; i++) {
            iport = (i == 0 ? info.stack_port_0 : info.stack_port_1);
            if (iport == VTSS_PORT_NO_NONE)
                CPRINTF("%-8s", "None");
            else
                CPRINTF("%-8u", iport2uport(iport));
        }
        CPRINTF("\n");
    }
}

#if defined(VTSS_FEATURE_LAYER2)
static void cli_cmd_port_debug_group(cli_req_t *req)
{
    vtss_uport_no_t         uport;
    vtss_port_no_t          iport;
    vtss_dgroup_port_conf_t conf;
    BOOL                    first = 1;
    u32                     port_count = port_isid_port_count(VTSS_ISID_LOCAL);
        
    for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0 || 
            vtss_dgroup_port_conf_get(NULL, iport, &conf) != VTSS_OK)
            continue;
        if (req->set) {
            conf.dgroup_no = uport2iport(req->uport);
            (void)vtss_dgroup_port_conf_set(NULL, iport, &conf);
        } else {
            if (first)
                cli_table_header("Port  Group");
            first = 0;
            CPRINTF("%-6u%u\n", uport, iport2uport(conf.dgroup_no));
        }
    }
}
#endif /* VTSS_FEATURE_LAYER2 */

#if defined(VTSS_FEATURE_PVLAN)
static void cli_cmd_port_debug_apvlan(cli_req_t *req)
{
    vtss_uport_no_t uport;
    vtss_port_no_t  iport, iport_cur;
    BOOL            iport_list[VTSS_PORT_ARRAY_SIZE];
    BOOL            first = 1;
    u32             port_count = port_isid_port_count(VTSS_ISID_LOCAL);
    char            buf[MGMT_PORT_BUF_SIZE];
        
    for (iport_cur = VTSS_PORT_NO_START; iport_cur < port_count; iport_cur++) {
        uport = iport2uport(iport_cur);
        if (req->uport != 0 && req->uport != uport)
            continue;

        if (req->set) {
            for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
                iport_list[iport] = req->uport_list[iport2uport(iport)];
            }
            (void)vtss_apvlan_port_members_set(NULL, iport_cur, iport_list);
        } else if (vtss_apvlan_port_members_get(NULL, iport_cur, iport_list) == VTSS_OK) {
            if (first)
                cli_table_header("Port  Members");
            first = 0;
            CPRINTF("%-6u%s\n", uport, cli_iport_list_txt(iport_list, buf));
        }
    }
}
#endif /* VTSS_FEATURE_PVLAN */

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int port_cli_parm_parse_keyword(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    port_cli_req_t *port_req = req->module_req;
    char *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "10hdx", 5)) {
            port_req->speed = VTSS_SPEED_10M;
            port_req->hdx = 1;
        } else if (!strncmp(found, "10fdx", 5)) {
            port_req->speed = VTSS_SPEED_10M;
            port_req->fdx = 1;
        } else if (!strncmp(found, "100hdx", 6)) {
            port_req->speed = VTSS_SPEED_100M;
            port_req->hdx = 1;
        } else if (!strncmp(found, "100fdx", 6)) {
            port_req->speed = VTSS_SPEED_100M;
            port_req->fdx = 1;
        } else if (!strncmp(found, "1000fdx", 7)) {
            port_req->speed = VTSS_SPEED_1G;
            port_req->fdx = 1;
        } else if (!strncmp(found, "2500fdx", 7)) {
            port_req->speed = VTSS_SPEED_2500M;
            port_req->fdx = 1;
        } else if (!strncmp(found, "10g", 3)) {
            port_req->speed = VTSS_SPEED_10G;
            port_req->fdx = 1;
        } else if (!strncmp(found, "100fx_ams", 9)) {
            port_req->fiber_speed = VTSS_SPEED_FIBER_100FX;
            port_req->auto_keyword = 1;
        } else if (!strncmp(found, "1000x_ams", 9)) {
            port_req->fiber_speed = VTSS_SPEED_FIBER_1000X;
            port_req->auto_keyword = 1;
        } else if (!strncmp(found, "100fx", 5)) {
            port_req->fiber_speed = VTSS_SPEED_FIBER_100FX;
            port_req->speed = VTSS_SPEED_100M;
            port_req->fdx = 1;
        } else if (!strncmp(found, "1000x", 5)) {
            port_req->fiber_speed = VTSS_SPEED_FIBER_1000X;
            port_req->speed = VTSS_SPEED_1G;
            port_req->fdx = 1;
        } else if (!strncmp(found, "sfp_auto_ams", 12)) {
            port_req->fiber_speed = VTSS_SPEED_FIBER_AUTO;
            port_req->auto_keyword = 1;
        } else if (!strncmp(found, "auto", 4)) {
            port_req->auto_keyword = 1;
        } else if (!strncmp(found, "bytes", 5)) {
            port_req->bytes = 1;
        } else if (!strncmp(found, "clear", 5)) {
            req->clear = 1;
        } else if (!strncmp(found, "discards", 8)) {
            port_req->discards = 1;
        } else if (!strncmp(found, "discard", 7)) {
            port_req->discard = 1;
        } else if (!strncmp(found, "disable", 7)) {
            req->disable = 1;
        } else if (!strncmp(found, "down", 4)) {
            port_req->down = 1;
        } else if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
        } else if (!strncmp(found, "errors", 6)) {
            port_req->errors = 1;
        } else if (!strncmp(found, "filtered", 8)) {
            port_req->filtered = 1;
        } else if (!strncmp(found, "none", 4)) {
            port_req->max_tags = PORT_MAX_TAGS_NONE;
        } else if (!strncmp(found, "one", 3)) {
            port_req->max_tags = PORT_MAX_TAGS_ONE;
        } else if (!strncmp(found, "packets", 7)) {
            port_req->packets = 1;
        } else if (!strncmp(found, "restart", 7)) {
        } else if (!strncmp(found, "two", 3)) {
            port_req->max_tags = PORT_MAX_TAGS_TWO;
        } else if (!strncmp(found, "up", 2)) {
            port_req->up = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
static int port_cli_oobfc_parm_parse_keyword(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    port_cli_req_t *port_req = req->module_req;
    int32_t        error = 0;

    req->parm_parsed = 1;
    error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, port_req->host_interface_list, 0, VTSS_PORT_NO_END, 1));
    if (error == 0) {
        port_req->host_interface_flag = TRUE;
    }

    return error;
}

static int port_cli_channel_status_parm_parse_keyword(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{

    int32_t        error = 0;
    port_cli_req_t *port_req = req->module_req;

    req->parm_parsed = 1;

    port_req->xtend_reach_flag = FALSE;
    if( !(error = cli_parse_word(cmd, "enable")) ) {
        port_req->channel_status = TRUE;
    } else if ( !(error = cli_parse_word(cmd, "disable")) ) {
        port_req->channel_status = FALSE;
    } else {
        error = -1;
    }

    return error;
}

static int port_cli_xtend_reach_parm_parse_keyword(char *cmd, char *cmd2,
                                                     char *stx, char *cmd_org, cli_req_t *req)
{

    int32_t        error = 0;
    port_cli_req_t *port_req = req->module_req;

    req->parm_parsed = 1;

    port_req->xtend_reach_flag = TRUE;
    if( !(error = cli_parse_word(cmd, "enable")) ) {
        port_req->xtend_reach = TRUE;
    } else if ( !(error = cli_parse_word(cmd, "disable")) ) {
        port_req->xtend_reach = FALSE;
    } else {
        error = -1;
    }

    return error;
}

#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */



static int port_cli_parm_parse_max_frame(char *cmd, char *cmd2,
                                         char *stx, char *cmd_org, cli_req_t *req)
{
    port_cli_req_t *port_req = req->module_req;

    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &port_req->max_length,
                           VTSS_MAX_FRAME_LENGTH_STANDARD, VTSS_MAX_FRAME_LENGTH_MAX);
}

static int port_cli_parm_parse_command(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;

    error = port_cli_parm_parse_keyword(cmd, cmd2, 
             "clear|packets|bytes|errors|discards|filtered", cmd_org, req);

#ifdef VTSS_SW_OPTION_QOS
    if (error) {
        error = cli_parm_parse_class(cmd, cmd2,stx, cmd_org, req);
    }
#endif

    return error;
}

#if defined(VTSS_FEATURE_NPI)
static int port_cli_parm_npi_qmask_keyword(char *cmd, char *cmd2,
                                           char *stx, char *cmd_org, cli_req_t *req)
{
    port_cli_req_t *port_req = req->module_req;

    req->parm_parsed = 1;
    return cli_parse_ulong(cmd, &port_req->cpu_q_mask, 0, (1 << VTSS_PACKET_RX_QUEUE_CNT) - 1);
}
#endif /* VTSS_FEATURE_NPI */
/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t port_cli_parm_table[] = {
    {
        "enable|disable",
        "enable     : Enable flow control\n"
        "disable    : Disable flow control\n"
        "(default: Show flow control mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_port_flow_control
    },
    {
        "enable|disable",
        "enable     : Enable port\n"
        "disable    : Disable port\n"
        "(default: Show administrative mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_port_state,
    },
    {
        "enable|disable",
        "enable     : Enable advertisement\n"
        "disable    : Disable advertisement\n"
        "(default: Show advertisement)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_port_debug_adv,
    },
    {
        "enable|disable",
        "enable     : Enable port loop\n"
        "disable    : Disable port loop\n"
        "(default: Show port loop)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_port_debug_loop,
    },
#if defined(VTSS_FEATURE_NPI)
    {
        "enable|disable",
        "enable     : Enable NPI port\n"
        "disable    : Disable NPI port\n"
        "(default: Show debug mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_port_debug_npi,
    },
#endif
    {
        "10hdx|10fdx|100hdx|100fdx|1000fdx",
        "10hdx      : 10 Mbps, half duplex\n"
        "10fdx      : 10 Mbps, full duplex\n"
        "100hdx     : 100 Mbps, half duplex\n"
        "100fdx     : 100 Mbps, full duplex\n"
        "1000fdx    : 1 Gbps, full duplex\n"
        "(default: All modes)",
        CLI_PARM_FLAG_NO_TXT,
        port_cli_parm_parse_keyword,
        NULL
    },
    {
        "10hdx|10fdx|100hdx|100fdx|1000fdx",
        "10hdx      : 10 Mbps, half duplex\n"
        "10fdx      : 10 Mbps, full duplex\n"
        "100hdx     : 100 Mbps, half duplex\n"
        "100fdx     : 100 Mbps, full duplex\n"
        "1000fdx    : 1 Gbps, full duplex\n"
        "(default: All modes)",
        CLI_PARM_FLAG_NO_TXT,
        port_cli_parm_parse_keyword,
        NULL
    },
    {
        "none|one|two",
        "none       : No tags allowed\n"
        "one        : One tag allowed\n"
        "two        : Two tags allowed\n"
        "(default: Show tag mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        port_cli_parm_parse_keyword,
        NULL
    },
    {
        port_mode_txt,
        port_mode_hlp,
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        port_cli_parm_parse_keyword,
        NULL
    },
    {
        "<max_frame>",
        port_max_frame_hlp,
        CLI_PARM_FLAG_NONE | CLI_PARM_FLAG_SET,
        port_cli_parm_parse_max_frame,
        NULL
    },
    {
        "discard|restart",
        "discard    : Discard frame after 16 collisions\n"
        "restart    : Restart backoff algorithm after 16 collisions\n"
        "(default: Show mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        port_cli_parm_parse_keyword,
        NULL
    },
#if defined(VTSS_FEATURE_NPI)
    {
        "<cpu_qmask>",
        "cpu_qmask    : number of CPU Queues to be redirected towards NPI Port\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        port_cli_parm_npi_qmask_keyword,
        cli_cmd_port_debug_npi
    },
#endif  /* VTSS_FEATURE_NPI */
    {
        "up|down",
        "up         : Show ports, which are up\n"
        "down       : Show ports, which are down",
        CLI_PARM_FLAG_NO_TXT,
        port_cli_parm_parse_keyword,
        NULL
    },
    {
        "<command>",
        "The command parameter takes the following values:\n"
        "clear      : Clear port statistics\n"
        "packets    : Show packet statistics\n"
        "bytes      : Show byte statistics\n"
        "errors     : Show error statistics\n"
        "discards   : Show discard statistics\n"
        "filtered   : Show filtered statistics\n"
#ifdef VTSS_SW_OPTION_QOS
 #ifdef VTSS_ARCH_LUTON28
        "low        : Show low priority statistics\n"
        "normal     : Show normal priority statistics\n"
        "medium     : Show medium priority statistics\n"
        "high       : Show high priority statistics\n"
 #else
        "0..7       : Show priority statistics\n"
 #endif
#endif
        "(default: Show all port statistics)",
        CLI_PARM_FLAG_NONE,
        port_cli_parm_parse_command,
        NULL
    },
    {
        "clear",
        "Clear port change counters",
        CLI_PARM_FLAG_NONE,
        port_cli_parm_parse_keyword,
        NULL
    },
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    {
        "<host_interface_list>",
        "host interface list",
        CLI_PARM_FLAG_NONE,
        port_cli_oobfc_parm_parse_keyword,
        NULL
    },
    {
        "<channel_status>",
        "The command parameter takes the following values:\n"
            " enable              : Enable OOB Flow control channel status\n"
            " disable             : Disable OOB Flow control channel status",
        CLI_PARM_FLAG_SET,
        port_cli_channel_status_parm_parse_keyword,
        cli_cmd_port_oob_flow_control
    },
    {
        "<xtend_reach>",
        "The command parameter takes the following values:\n"
            " enable              : Enable Extended reach of OOB Flow control status\n"
            " disable             : Disable Extended reach of OOB Flow control status",
        CLI_PARM_FLAG_SET,
        port_cli_xtend_reach_parm_parse_keyword,
        cli_cmd_port_oob_flow_control
    },
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */
#if defined(VTSS_FEATURE_LAYER2)
    {
        "<group_no>",
        "Destination port group number",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_port,
        NULL
    },
#endif /* VTSS_FEATURE_LAYER2 */
#if defined(VTSS_FEATURE_PVLAN)
    {
        "<port_list>",
        "Destination port list",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_port_list,
        cli_cmd_port_debug_apvlan
    },
#endif /* VTSS_FEATURE_PVLAN */
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

enum {
    CLI_CMD_PORT_CONF_PRIO = 0,
    CLI_CMD_PORT_MODE_PRIO,
    CLI_CMD_PORT_FLOW_CONTROL_PRIO,
    CLI_CMD_PORT_STATE_PRIO,
    CLI_CMD_PORT_MAX_FRAME_PRIO,
    CLI_CMD_PORT_EXC_COL_CONT_PRIO,
    CLI_CMD_PORT_STATS_PRIO,
#if VTSS_UI_OPT_VERIPHY
    CLI_CMD_PORT_VERIPHY_PRIO,
#endif /* VTSS_UI_OPT_VERIPHY */
    CLI_CMD_PORT_NUMBERS_PRIO,
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    CLI_CMD_PORT_OOBC_FLOW_CONTROL_PRIO,
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */
    CLI_CMD_PORT_SFP_PRIO,
    CLI_CMD_DEBUG_PORT_CONF_PRIO = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_ADV = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_TAGS = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_STATS_PRIO = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_COUNTERS_PRIO = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_REGS = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_CHANGE = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_NPI = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_CAPS = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_INFO = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_LOOP = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_GROUP = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_DEBUG_PORT_APVLAN = CLI_CMD_SORT_KEY_DEFAULT,
    CLI_CMD_PORT_FIBER_MODE,
};

cli_cmd_tab_entry(
    "Port Configuration [<port_list>] [up|down]",
    NULL,
    "Show port configuration",
    CLI_CMD_PORT_CONF_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_disp,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry(
    "Port Mode [<port_list>]",
    port_mode_cmd,
    "Set or show the port speed and duplex mode",
    CLI_CMD_PORT_MODE_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_mode,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry(
    port_sfp_cmd,
    NULL,
    "Show the detected sfp type",
    CLI_CMD_PORT_SFP_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_sfp,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    port_fc_cmd_ro,
    port_fc_cmd_rw,
    "Set or show the port flow control mode",
    CLI_CMD_PORT_FLOW_CONTROL_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_flow_control,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Port State [<port_list>]",
    "Port State [<port_list>] [enable|disable]",
    "Set or show the port administrative state",
    CLI_CMD_PORT_STATE_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_state,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Port MaxFrame [<port_list>]",
    "Port MaxFrame [<port_list>] [<max_frame>]",
    "Set or show the port maximum frame size",
    CLI_CMD_PORT_MAX_FRAME_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_max_frame,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry(
    "Port Excessive [<port_list>]",
    "Port Excessive [<port_list>] [discard|restart]",
    "Set or show the port excessive collision mode",
    CLI_CMD_PORT_EXC_COL_CONT_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_exc_col_cont,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Port Statistics [<port_list>]",
    "Port Statistics [<port_list>] [<command>] [up|down]",
    "Show port statistics",
    CLI_CMD_PORT_STATS_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_stats,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if VTSS_UI_OPT_VERIPHY
cli_cmd_tab_entry(
    NULL,
    "Port VeriPHY [<port_list>]",
    "Run cable diagnostics",
    CLI_CMD_PORT_VERIPHY_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PING,
    cli_cmd_port_veriphy,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_UI_OPT_VERIPHY */

#if defined(CLI_CUSTOM_PORT_TXT)
cli_cmd_tab_entry(
    "Port Numbers",
    NULL,
    "Show port numbering",
    CLI_CMD_PORT_NUMBERS_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_numbers_req,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* CLI_CUSTOM_PORT_TXT */

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
cli_cmd_tab_entry(
    "Port OOBFC [<host_interface_list>]",
    "Port OOBFC [<host_interface_list>] [<channel_status>] [<xtend_reach>]",
    "Set or show the Host interface OOB flow control options",
    CLI_CMD_PORT_OOBC_FLOW_CONTROL_PRIO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PORT,
    cli_cmd_port_oob_flow_control,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */


cli_cmd_tab_entry(
    "Debug Port Configuration [<port_list>] [up|down]",
    NULL,
    "Show port configuration",
    CLI_CMD_DEBUG_PORT_CONF_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_conf,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug Port Advertise [<port_list>]",
    "Debug Port Advertise [<port_list>] [10hdx|10fdx|100hdx|100fdx|1000fdx] [enable|disable]",
    "Set or show auto-negotiation advertisements",
    CLI_CMD_DEBUG_PORT_ADV,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_adv,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_SW_OPTION_UP_MEP)
cli_cmd_tab_entry(
    "Debug Port Loop [<port_list>] [up|down]",
    "Debug Port Loop [<port_list>] [enable|disable] [up|down]",
    "Set or show port loop back enable",
    CLI_CMD_DEBUG_PORT_LOOP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_loop,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

cli_cmd_tab_entry(
    "Debug Port Tags [<port_list>] [up|down]",
    "Debug Port Tags [<port_list>] [none|one|two] [up|down]",
    "Set or show number of tags",
    CLI_CMD_DEBUG_PORT_TAGS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_tags,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug Port Statistics [<port_list>] [up|down]",
    "Debug Port Statistics [<port_list>] [<command>] [up|down]",
    "Show port statistics",
    CLI_CMD_DEBUG_PORT_STATS_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_stats,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_FEATURE_NPI)
cli_cmd_tab_entry(
    "Debug Port NPI <port>",
    "Debug Port NPI <port> <cpu_qmask> [enable|disable]",
    "queue_mask : specify the queue mask for the CPU Queues \n"
    "to be redirected towards NPI Port \n"
    "incase of Luton26, CPU Queues are 8 and Jaguar number of CPU Queues are 10 \n"
    "enable or disable NPI port",
    CLI_CMD_DEBUG_PORT_NPI,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_npi,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif


#if defined(VTSS_ARCH_LUTON28)
cli_cmd_tab_entry(
    "Debug Port Counters [<port_list>]",
    NULL,
    "Show chip port statistics",
    CLI_CMD_DEBUG_PORT_COUNTERS_PRIO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_counters,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_ARCH_LUTON28 */

cli_cmd_tab_entry(
    "Debug Port Registrations [clear]",
    NULL,
    "Show or clear port module registrations",
    CLI_CMD_DEBUG_PORT_REGS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_regs,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug Port Change [<port_list>] [up|down] [clear]",
    NULL,
    "Show or clear port change event counters",
    CLI_CMD_DEBUG_PORT_CHANGE,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_change,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug Port Capabilities [<port_list>] [up|down]",
    NULL,
    "Show port capabilities",
    CLI_CMD_DEBUG_PORT_CAPS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_caps,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry(
    "Debug Port Info",
    NULL,
    "Show information for each switch",
    CLI_CMD_DEBUG_PORT_INFO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_info,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_FEATURE_LAYER2)
cli_cmd_tab_entry(
    "Debug Port Group [<port_list>] [<group_no>]",
    NULL,
    "Set or show destination port groups",
    CLI_CMD_DEBUG_PORT_GROUP,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_group,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_FEATURE_LAYER2 */

#if defined(VTSS_FEATURE_PVLAN)
cli_cmd_tab_entry(
    "Debug APVLAN [<port>] [<port_list>]",
    NULL,
    "Set or show assymmetric private VLANs",
    CLI_CMD_DEBUG_PORT_APVLAN,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_port_debug_apvlan,
    NULL,
    port_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_FEATURE_PVLAN */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
