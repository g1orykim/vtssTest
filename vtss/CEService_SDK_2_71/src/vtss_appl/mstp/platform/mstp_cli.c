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

#include "mstp_cli.h"
#include "cli.h"
#include "mstp_api.h"
#include "cli_trace_def.h"

#if defined(VTSS_MSTP_FULL)
#include "vlan_api.h"
#define _MSTP_MSTI_PARAM      " <msti>"
#define _MSTP_MSTI_CPARAM     " [<msti>]"
#define _MSTP_MSTI_INST       " Msti"
#define _MSTP_MSTI_PORTINST   " Msti Port"
#else
#define _MSTP_MSTI_PARAM  
#define _MSTP_MSTI_CPARAM 
#define _MSTP_MSTI_INST       " Bridge"
#define _MSTP_MSTI_PORTINST   " Port"
#endif /* VTSS_MSTP_FULL */

typedef struct {
    ulong    cost;
    uint     ppriority;
    uint     bpriority;
    uint     time;
    uchar    msti;
    BOOL     msti_set;
    uchar    forceVersion;
    uchar    maxhops;
    ulong    val;
    uint     vid_start, vid_end;

    /* Keywords */
    BOOL     cos;
    BOOL     auto_keyword;
} mstp_cli_req_t;

/****************************************************************************/
/*  Initialization                                                          */
/****************************************************************************/

void mstp_cli_init(void)
{
    /* register the size required for mstp req. structure */
    cli_req_size_register(sizeof(mstp_cli_req_t));
}

/****************************************************************************/
/*  Command functions                                                       */
/****************************************************************************/

static void mstp_display_port(uchar msti, l2_port_no_t l2port)
{
    mstp_port_mgmt_status_t status, *ps = &status;
    if(mstp_get_port_status(msti, l2port, ps) && ps->active) {
        if(ps->parent != L2_NULL) {
            char buf[16];
            (void) snprintf(buf, sizeof(buf), "Aggr(%s)", l2port2str(ps->parent));
            CPRINTF("%9s  %-14s  %-10s\n", l2port2str(l2port), buf, ps->fwdstate);
        } else {
            CPRINTF("%9s  %-14.14s  %-10.10s  %3u  %8u  %-4s  %-3s  %s\n",
                    l2port2str(l2port),
                    ps->core.rolestr, ps->core.statestr,
                    (uint) (ps->core.portId[0] & 0xf0),
                    ps->core.pathCost,
                    ps->core.operEdgePort ? "Yes" : "No",
                    ps->core.operPointToPointMAC ? "Yes" : "No",
                    misc_time2interval(ps->core.uptime));
        }
    }
}

static void mstp_display_status(cli_req_t *req)
{
    mstp_bridge_status_t status, *bs = &status;
    mstp_cli_req_t *mstp_req = req->module_req;
    uchar msti = mstp_req->msti;
    l2port_iter_t l2pit;
    char str[32];

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req))
        return;

    if(mstp_get_bridge_status(msti, bs)) {
        CPRINTF("%s Bridge STP Status\n", msti_name(msti));
        (void) vtss_mstp_bridge2str(str, sizeof(str), bs->bridgeId);
        CPRINTF("Bridge ID    : %s\n", str);
        (void) vtss_mstp_bridge2str(str, sizeof(str), bs->designatedRoot);
        CPRINTF("Root ID      : %s\n", str);
        CPRINTF("Root Port    : %s\n", bs->rootPort != L2_NULL ? l2port2str(bs->rootPort) : "-");
        CPRINTF("Root PathCost: %u\n", bs->rootPathCost);
        if(msti == 0) {
            (void) vtss_mstp_bridge2str(str, sizeof(str), bs->cistRegionalRoot);
            CPRINTF("Regional Root: %s\n", str);
            CPRINTF("Int. PathCost: %u\n", bs->cistInternalPathCost);
            CPRINTF("Max Hops     : %u\n", bs->maxHops);
        }
        CPRINTF("TC Flag      : %s\n", bs->topologyChange ? "Changing" : "Steady");
        CPRINTF("TC Count     : %u\n", bs->topologyChangeCount);
        CPRINTF("TC Last      : %s\n",
                bs->timeSinceTopologyChange == MSTP_TIMESINCE_NEVER ?
                "-" : misc_time2interval(bs->timeSinceTopologyChange));
    } else {
        CPRINTF("%s not active.\n", msti_name(msti));
        return;
    }
    cli_table_header("Port       Port Role       State       Pri  PathCost  Edge  P2P  Uptime       " "  ");
    (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_ALL | L2PORT_ITER_USID_ORDER);
    while(l2port_iter_getnext(&l2pit)) {
        /* usid check: Ports and LLAGs */
        if(L2PIT_TYPE(&l2pit, L2PORT_ITER_TYPE_PHYS | L2PORT_ITER_TYPE_LLAG) &&
           (req->stack.isid[l2pit.usid]) == VTSS_ISID_END)
            continue;
        /* Port check: Ports */
        if(L2PIT_TYPE(&l2pit, L2PORT_ITER_TYPE_PHYS) && (req->uport_list[l2pit.uport] == 0))
            continue;
        mstp_display_port(msti, l2pit.l2port);
    }
}

static void mstp_display_port_statistics(l2_port_no_t l2port, BOOL clear)
{
    mstp_port_statistics_t stats, *ps = &stats;
    if(mstp_get_port_statistics(l2port, ps, clear)) {
        CPRINTF("%9s  %8u  %8u  %8u  %8u  %7u  %7u  %7u  %7u  %7u  %7u\n",
                l2port2str(l2port),
                ps->mstp_frame_recvs,
                ps->mstp_frame_xmits,
                ps->rstp_frame_recvs,
                ps->rstp_frame_xmits,
                ps->stp_frame_recvs,
                ps->stp_frame_xmits,
                ps->tcn_frame_recvs,
                ps->tcn_frame_xmits,
                ps->illegal_frame_recvs,
                ps->unknown_frame_recvs);
    }
}

static void mstp_display_statistics(cli_req_t *req)
{
    l2port_iter_t l2pit;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req))
        return;

    cli_table_header("Port       Rx MSTP   Tx MSTP   Rx RSTP   Tx RSTP   Rx STP   Tx STP   Rx TCN   Tx TCN   Rx Ill.  Rx Unk.");
    (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_ALL | L2PORT_ITER_USID_ORDER);
    while(l2port_iter_getnext(&l2pit)) {
        /* usid check: Ports and LLAGs */
        if(L2PIT_TYPE(&l2pit, L2PORT_ITER_TYPE_PHYS|L2PORT_ITER_TYPE_LLAG) &&
           (req->stack.isid[l2pit.usid]) == VTSS_ISID_END)
            continue;
        /* Port check: Ports */
        if(L2PIT_TYPE(&l2pit, L2PORT_ITER_TYPE_PHYS) && (req->uport_list[l2pit.uport] == 0))
            continue;
        mstp_display_port_statistics(l2pit.l2port, req->clear);
    }
}

#if defined(VTSS_MSTP_FULL)
static void mstp_cname(cli_req_t *req)
{
    mstp_msti_config_t conf;
    if(mstp_get_msti_config(&conf, NULL)) {
        if(req->set) {
            strncpy(conf.configname, req->parm, sizeof(conf.configname));
            if(req->int_values[0] >= 0 && req->int_values[0] < 0xffff)
                conf.revision = (u16) req->int_values[0];
            (void) mstp_set_msti_config(&conf);
        } else {
            CPRINTF("Configuration name: %.*s\n", (int) sizeof(conf.configname), conf.configname);
            CPRINTF("Configuration rev.: %d\n", conf.revision);
        }
    }
}

static void mstp_show_mstimapping(const mstp_msti_config_t *conf, u8 msti)
{
    char vlanrange[(4096/2)*4]; /* Worst case */

    CPRINTF("%s  ", msti_name(msti));
    (void) mstp_mstimap2str(conf, msti, vlanrange, sizeof(vlanrange));
    if(strlen(vlanrange) == 0)
        strcpy(vlanrange, "No VLANs mapped");
    cli_puts(vlanrange);
    cli_putchar('\n');
}

static void mstp_mstimap(cli_req_t *req)
{
    mstp_msti_config_t conf;
    mstp_cli_req_t *mstp_req = req->module_req;

    if(mstp_get_msti_config(&conf, NULL)) {
        if(req->clear) {
            BOOL ok = TRUE;
            if(mstp_req->msti_set) {
                if(mstp_req->msti) {
                    vtss_vid_t vid;
                    CPRINTF("Deleting %s VID mappings\n", msti_name(mstp_req->msti));
                    for(vid = 1; vid < ARRSZ(conf.map.map); vid++) {
                        if(conf.map.map[vid] == mstp_req->msti) {
                            conf.map.map[vid] = 0;
                            CPRINTF("Mapping VID %d to CIST\n", vid);
                        }
                    }
                } else {
                    CPRINTF("Error: Cannot unmap CIST\n");
                    ok = FALSE;
                }
            } else {
                CPRINTF("Mapping all VLANs to CIST\n");
                memset(conf.map.map, 0, ARRSZ(conf.map.map));
            }
            if(ok)
                (void) mstp_set_msti_config(&conf);
        } else {
            cli_parm_header("MSTI  VLANs mapped to MSTI");
            if(mstp_req->msti_set && mstp_req->msti) {
                mstp_show_mstimapping(&conf, mstp_req->msti);
            } else {
                u8 msti;
                for(msti = 1; msti < N_MSTI_MAX; msti++)
                    mstp_show_mstimapping(&conf, msti);
            }
        }
    }
}

static void mstp_mstimap_add(cli_req_t *req)
{
    mstp_msti_config_t conf;
    mstp_cli_req_t *mstp_req = req->module_req;

    if(mstp_req->vid_start == mstp_req->vid_end) {
        CPRINTF("Add VLAN %d to %s\n", mstp_req->vid_start, msti_name(mstp_req->msti));
    } else {
        CPRINTF("Add VLANs %d-%d to %s\n", mstp_req->vid_start, mstp_req->vid_end, msti_name(mstp_req->msti));
    }

    if(mstp_get_msti_config(&conf, NULL)) {
        uint i;
        for(i = mstp_req->vid_start; i <=  mstp_req->vid_end; i++)
            conf.map.map[i] = mstp_req->msti;
        (void) mstp_set_msti_config(&conf);
    }
}
#endif  /* VTSS_MSTP_FULL */

/* MSTP configuration */
static void cli_mstp_bridge_conf(cli_req_t *req,
                                 BOOL age, BOOL delay, BOOL version, 
                                 BOOL txhold, BOOL maxhops,
                                 BOOL bpdu_filtering, BOOL bpdu_guard, BOOL recovery)
{
    mstp_bridge_param_t       sc;
    mstp_cli_req_t *mstp_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req))
        return;

    (void) mstp_get_system_config(&sc);

    if (req->set) {
        if (age)
            sc.bridgeMaxAge = mstp_req->time;
        if (delay)
            sc.bridgeForwardDelay = mstp_req->time;
        if (version)
            sc.forceVersion = mstp_req->forceVersion;
        if (txhold)
            sc.txHoldCount = mstp_req->val;
        if (maxhops)
            sc.MaxHops = mstp_req->maxhops;
#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
        if (bpdu_filtering)
            sc.bpduFiltering = req->enable;
        if (bpdu_guard)
            sc.bpduGuard = req->enable;
        if (recovery)
            sc.errorRecoveryDelay = mstp_req->val;
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */
        (void) mstp_set_system_config(&sc);
    } else {
        if (version)
            CPRINTF("Protocol Version: %s\n",
                    sc.forceVersion == MSTP_PROTOCOL_VERSION_MSTP ? "MSTP" :
                    sc.forceVersion == MSTP_PROTOCOL_VERSION_RSTP ? "RSTP" : "Compatible (STP)");
        if (age)
            CPRINTF("Max Age         : %u\n", sc.bridgeMaxAge);
        if (delay)
            CPRINTF("Forward Delay   : %u\n", sc.bridgeForwardDelay);
        if (txhold)
            CPRINTF("Tx Hold Count   : %u\n", sc.txHoldCount);
        if (maxhops)
            CPRINTF("Max Hop Count   : %u\n", sc.MaxHops);
#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
        if (bpdu_filtering)
            CPRINTF("BPDU Filtering  : %s\n", cli_bool_txt(sc.bpduFiltering));
        if (bpdu_guard)
            CPRINTF("BPDU Guard      : %s\n", cli_bool_txt(sc.bpduGuard));
        if (recovery && sc.errorRecoveryDelay)
            CPRINTF("Error Recovery  : %u seconds\n", sc.errorRecoveryDelay);
        if (recovery && !sc.errorRecoveryDelay)
            CPRINTF("Error Recovery  : Disabled\n");
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */
    }
}

/* MSTI (priority) configuration */
static void cli_mstp_msti_conf(cli_req_t *req)
{
    mstp_cli_req_t *mstp_req = req->module_req;

    if (req->set) {
        if(mstp_req->msti_set) {
            (void) mstp_set_msti_priority(mstp_req->msti, mstp_req->bpriority >> 8);
        } else {
            u8 msti;
            for(msti = 0; msti < N_MSTI_MAX; msti++)
                (void) mstp_set_msti_priority(msti, mstp_req->bpriority >> 8);
        }
    } else {
        cli_parm_header("MSTI#  Bridge Priority");
        if(mstp_req->msti_set) {
            CPRINTF("%-5.5s  %5u\n", msti_name(mstp_req->msti), mstp_get_msti_priority(mstp_req->msti) << 8);
        } else {
            mstp_bridge_param_t sc;
            u8 msti, mstimax = N_MSTI_MAX;
            if(mstp_get_system_config(&sc) && sc.forceVersion < MSTP_PROTOCOL_VERSION_MSTP)
                mstimax = 1;
            for(msti = 0; msti < mstimax; msti++)
                CPRINTF("%-5.5s  %5u\n", msti_name(msti), mstp_get_msti_priority(msti) << 8);
        }
    }
}

static void cli_mstp_port_conf_port(const l2port_iter_t *l2pit,
                                    cli_req_t *req,
                                    BOOL mode, BOOL edge, BOOL p2p, BOOL autoedge,
                                    BOOL rRole, BOOL rTcn, BOOL bpduGuard)
{
    BOOL                      enable;
    BOOL                      isPort = (l2pit->iport != VTSS_PORT_NO_NONE);
    mstp_port_param_t         conf;
    char                      buf[80], *p;
    mstp_cli_req_t            *mstp_req = req->module_req;

    T_N("Called usid %d iport %d uport %d", l2pit->usid, l2pit->iport, l2pit->uport);

    if((req->uport_list[l2pit->uport] == 0) ||
       (isPort && (req->stack.isid[l2pit->usid] == VTSS_ISID_END)))
        return;                 /* Not Selected */

    T_D("Selected usid %d iport %d uport %d", l2pit->usid, l2pit->iport, l2pit->uport);

    if(!mstp_get_port_config(l2pit->isid, l2pit->iport, &enable, &conf)) {
        T_E("mstp_get_port_config: error");
        return;                 /* Not Active/valid */
    }

    if (req->set) {
        if (mode) {
            enable = req->enable;
        }
        if (edge)
            conf.adminEdgePort = req->enable;
        if (autoedge)
            conf.adminAutoEdgePort = req->enable;
        if (p2p)
            conf.adminPointToPointMAC = (mstp_req->auto_keyword ? P2P_AUTO :
                                         req->enable ? P2P_FORCETRUE : P2P_FORCEFALSE);
        if (rRole)
            conf.restrictedRole = req->enable;
        if (rTcn)
            conf.restrictedTcn = req->enable;
#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
        if (bpduGuard)
            conf.bpduGuard = req->enable;
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */
        if (!mstp_set_port_config(l2pit->isid, l2pit->iport, enable, &conf))
            CPRINTF("Port %s: set config failed\n", isPort ? cli_l2port2uport_str(l2pit->l2port) : "0");
    } else {
        if (!isPort || l2pit->pit.first) {
            if (!isPort) {
                CPRINTF("\n");
            } else {
                cli_cmd_usid_print(l2pit->usid, req, 1);
            }
            p = &buf[0];
            p += sprintf(p, "Port  ");
            if (mode)
                p += sprintf(p, "Mode      ");
            if (edge)
                p += sprintf(p, "AdminEdge ");
            if (autoedge)
                p += sprintf(p, "AutoEdge  ");
            if (rRole)
                p += sprintf(p, "restrRole ");
            if (rTcn)
                p += sprintf(p, "restrTcn  ");
#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
            if (bpduGuard)
                p += sprintf(p, "bpduGuard ");
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */
            if (p2p)
                p += sprintf(p, "Point2point");
            cli_table_header(buf);
        }
        if (!isPort)
            CPRINTF("Aggr  ");
        else
            CPRINTF("%-2u    ", l2pit->uport);
        if (mode)
            CPRINTF("%s  ", cli_bool_txt(enable));
        if (edge)
            CPRINTF("%s  ", cli_bool_txt(conf.adminEdgePort));
        if (autoedge)
            CPRINTF("%s  ", cli_bool_txt(conf.adminAutoEdgePort));
        if (rRole)
            CPRINTF("%s  ", cli_bool_txt(conf.restrictedRole));
        if (rTcn)
            CPRINTF("%s  ", cli_bool_txt(conf.restrictedTcn));
#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
        if (bpduGuard)
            CPRINTF("%s  ", cli_bool_txt(conf.bpduGuard));
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */
        if (p2p)
            CPRINTF("%s",
                    conf.adminPointToPointMAC == P2P_AUTO ? "Auto" :
                    cli_bool_txt(conf.adminPointToPointMAC == P2P_FORCETRUE ? 1 : 0));
        CPRINTF("\n");
    }
}

static void cli_mstp_port_conf(cli_req_t *req,
                               BOOL mode, BOOL edge, BOOL p2p, BOOL autoedge,
                               BOOL rRole, BOOL rTcn, BOOL bpduGuard)
{
    l2port_iter_t l2pit;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req))
        return;
    (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_PHYS | L2PORT_ITER_ISID_CFG | L2PORT_ITER_USID_ORDER);
    /* Special case aggr == VTSS_PORT_NO_NONE (isid == don't care) */
    l2pit.iport = VTSS_PORT_NO_NONE; l2pit.uport = 0;
    cli_mstp_port_conf_port(&l2pit, req, mode, edge, p2p, autoedge, rRole, rTcn, bpduGuard);
    while(l2port_iter_getnext(&l2pit))
        cli_mstp_port_conf_port(&l2pit, req, mode, edge, p2p, autoedge, rRole, rTcn, bpduGuard);
}

static void cli_mstp_mstiport_conf_port(const l2port_iter_t *l2pit,
                                        cli_req_t *req,
                                        BOOL cost, BOOL prio)
{
    BOOL                      isPort = (l2pit->iport != VTSS_PORT_NO_NONE);
    mstp_msti_port_param_t    conf;
    char                      buf[80], *p;
    mstp_cli_req_t *mstp_req = req->module_req;

    if((isPort &&
        ((req->stack.isid[l2pit->usid] == VTSS_ISID_END) || 
         (req->uport_list[l2pit->uport] == 0))) ||
       !mstp_get_msti_port_config(l2pit->isid, mstp_req->msti, l2pit->iport, &conf))
        return;
    
    if (req->set) {
        if (cost)
            conf.adminPathCost = mstp_req->cost;
        if (prio)
            conf.adminPortPriority = mstp_req->ppriority;
        if (!mstp_set_msti_port_config(l2pit->isid, mstp_req->msti, l2pit->iport, &conf))
            CPRINTF("Port %s: set config failed\n", isPort ? cli_l2port2uport_str(l2pit->l2port) : "0");
    } else {
        if (!isPort || l2pit->pit.first) {
            if (!isPort) {
                CPRINTF("\n");
            } else {
                cli_cmd_usid_print(l2pit->usid, req, 1);
            }
            p = &buf[0];
            p += sprintf(p, "MSTI  Port  ");
            if (cost)
                p += sprintf(p, "Path Cost   ");
            if (prio)
                p += sprintf(p, "Priority  ");
            cli_table_header(buf);
        }
        CPRINTF("%s  ", msti_name(mstp_req->msti));
        if (!isPort)
            CPRINTF("Aggr  ");
        else
            CPRINTF("%-2u    ", l2pit->uport);
        if (cost) {
            if (conf.adminPathCost == MSTP_PORT_PATHCOST_AUTO)
                strcpy(buf, "Auto");
            else
                sprintf(buf, "%u", conf.adminPathCost);
            CPRINTF("%-12s", buf);
        }
        if (prio)
            CPRINTF("%-10d", conf.adminPortPriority);
        CPRINTF("\n");
    }
}

static void cli_mstp_mstiport_conf(cli_req_t *req,
                                   BOOL cost, BOOL prio)
{
    l2port_iter_t l2pit;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req))
        return;
    
    (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_PHYS | L2PORT_ITER_ISID_CFG | L2PORT_ITER_USID_ORDER);
    if(req->uport_list[0]) {
        /* Special case aggr == VTSS_PORT_NO_NONE (isid == don't care) */
        l2pit.iport = VTSS_PORT_NO_NONE;
        cli_mstp_mstiport_conf_port(&l2pit, req, cost, prio);
    }
    while(l2port_iter_getnext(&l2pit))
        cli_mstp_mstiport_conf_port(&l2pit, req, cost, prio);
}

static void cli_cmd_mstp_conf(cli_req_t *req)
{
    if(!req->set) {
        cli_header("STP Configuration", 1);
    }
    cli_mstp_bridge_conf(req, 1, 1, 1, 1, 1, 1, 1, 1);
}

static void cli_cmd_mstp_version(cli_req_t *req)
{
    cli_mstp_bridge_conf(req, 0, 0, 1, 0, 0, 0, 0, 0);
}

static void cli_cmd_mstp_txholdct(cli_req_t *req)
{
    cli_mstp_bridge_conf(req, 0, 0, 0, 1, 0, 0, 0, 0);
}

static void cli_cmd_mstp_maxhops(cli_req_t *req)
{
    cli_mstp_bridge_conf(req, 0, 0, 0, 0, 1, 0, 0, 0);
}

static void cli_cmd_mstp_maxage(cli_req_t *req)
{
    cli_mstp_bridge_conf(req, 1, 0, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_mstp_fwddelay(cli_req_t *req)
{
    cli_mstp_bridge_conf(req, 0, 1, 0, 0, 0, 0, 0, 0);
}

#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
static void cli_cmd_mstp_bpdu_filtering(cli_req_t *req)
{
    cli_mstp_bridge_conf(req, 0, 0, 0, 0, 0, 1, 0, 0);
}

static void cli_cmd_mstp_bpdu_guard(cli_req_t *req)
{
    cli_mstp_bridge_conf(req, 0, 0, 0, 0, 0, 0, 1, 0);
}

static void cli_cmd_mstp_recovery_timeout(cli_req_t *req)
{
    cli_mstp_bridge_conf(req, 0, 0, 0, 0, 0, 0, 0, 1);
}

#endif /*VTSS_SW_OPT_MSTP_BPDU_ENH*/

#if defined(VTSS_MSTP_FULL)
static void cli_cmd_mstp_cname(cli_req_t *req)
{
    mstp_cname(req); 
}
#endif  /* VTSS_MSTP_FULL */

static void cli_cmd_mstp_status(cli_req_t *req)
{
    mstp_display_status(req);
}

static void cli_cmd_mstp_msti_priority(cli_req_t *req)
{
    cli_mstp_msti_conf(req);
}

#if defined(VTSS_MSTP_FULL)
static void cli_cmd_mstp_mstimap(cli_req_t *req)
{
    mstp_mstimap(req);
}

static void cli_cmd_mstp_mstimap_add(cli_req_t *req)
{
    mstp_mstimap_add(req);
}
#endif  /* VTSS_MSTP_FULL */

static void cli_cmd_mstp_port_conf(cli_req_t *req)
{
    cli_mstp_port_conf(req, 1, 1, 1, 1, 1, 1, 0);
}

static void cli_cmd_mstp_port_mode(cli_req_t *req)
{
    cli_mstp_port_conf(req, 1, 0, 0, 0, 0, 0, 0);
}

static void cli_cmd_mstp_port_edge(cli_req_t *req)
{
    cli_mstp_port_conf(req, 0, 1, 0, 0, 0, 0, 0);
}

static void cli_cmd_mstp_port_autoedge(cli_req_t *req)
{
    cli_mstp_port_conf(req, 0, 0, 0, 1, 0, 0, 0);
}

static void cli_cmd_mstp_port_p2p(cli_req_t *req)
{
    cli_mstp_port_conf(req, 0, 0, 1, 0, 0, 0, 0);
}

static void cli_cmd_mstp_port_restricted_role(cli_req_t *req)
{
    cli_mstp_port_conf(req, 0, 0, 0, 0, 1, 0, 0);
}

static void cli_cmd_mstp_port_restricted_tcn(cli_req_t *req)
{
    cli_mstp_port_conf(req, 0, 0, 0, 0, 0, 1, 0);
}

#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
static void cli_cmd_mstp_port_bpdu_guard(cli_req_t *req)
{
    cli_mstp_port_conf(req, 0, 0, 0, 0, 0, 0, 1);
}
#endif /*VTSS_SW_OPT_MSTP_BPDU_ENH*/

static void cli_cmd_mstp_port_statistics(cli_req_t *req)
{
    mstp_display_statistics(req);
}

/* MSTP port MCHECK set */
static void cli_cmd_mstp_port_mcheck(cli_req_t *req)
{
    l2port_iter_t l2pit;

    if (cli_cmd_switch_none(req) || cli_cmd_slave(req))
        return;

    (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_ALL);
    while(l2port_iter_getnext(&l2pit)) {
        (void)mstp_set_port_mcheck(l2pit.l2port);
    }
}

#if defined(VTSS_MSTP_FULL)
static void cli_cmd_mstp_mstiport_conf(cli_req_t *req)
{
    cli_mstp_mstiport_conf(req, 1, 1);
}
#endif  /* VTSS_MSTP_FULL */

static void cli_cmd_mstp_mstiport_cost(cli_req_t *req)
{
    cli_mstp_mstiport_conf(req, 1, 0);
}

static void cli_cmd_mstp_mstiport_priority(cli_req_t *req)
{
    cli_mstp_mstiport_conf(req, 0, 1);
}

static void cli_cmd_mstp_debug_port(cli_req_t *req)
{
    const char *strportstate[] = { "Discarding", "Learning", "Forwarding"};
    mstp_cli_req_t *mstp_req = req->module_req;

    l2_port_no_t l2port = req->int_values[0];
    if(l2port_is_valid(l2port)) {
        if(l2port_is_port(l2port)) {
            vtss_stp_state_t api_state = VTSS_STP_STATE_DISCARDING;
            (void) vtss_stp_port_state_get(NULL, l2port, &api_state);
            CPRINTF("User port %s API stpstate %s, L2 stpstate %s\n",
                    l2port2str(l2port),
                    strportstate[api_state],
                    strportstate[vtss_os_get_stpstate(l2port)]);
            CPRINTF("%s L2 PORT stpstate %s\n", msti_name(mstp_req->msti),
                    strportstate[l2_get_msti_stpstate(mstp_req->msti, l2port)]);
            vtss_rc rc = vtss_mstp_port_msti_state_get(NULL,
                                                       l2port,
                                                       mstp_req->msti,
                                                       &api_state);
            if(rc == VTSS_OK)
                CPRINTF("%s Switch API stpstate %s\n", msti_name(mstp_req->msti),
                        strportstate[api_state]);
        } else {
            CPRINTF("Aggregated port %d = %s, valid %d port %d poag %d glag %d\n",
                    l2port, l2port2str(l2port),
                    l2port_is_valid(l2port), l2port_is_port(l2port),
                    l2port_is_poag(l2port), l2port_is_glag(l2port));
            CPRINTF("L2_MAX_PORTS %d, L2_MAX_LLAGS %d, L2_MAX_GLAGS %d, L2_MAX_POAGS %d\n",
                    L2_MAX_PORTS,
                    L2_MAX_LLAGS,
                    L2_MAX_GLAGS,
                    L2_MAX_POAGS);
        }
        mstp_display_port(mstp_req->msti, l2port);
    } else {
        CPRINTF("Port %d is invalid. Ports [%d-%d], LLAGs [%d-%d], GLAGs [%d-%d].\n",
                l2port,
                VTSS_PORT_NO_START,
                VTSS_PORT_NO_START+L2_MAX_PORTS-1,
                VTSS_PORT_NO_START+L2_MAX_PORTS,
                VTSS_PORT_NO_START+L2_MAX_PORTS+L2_MAX_LLAGS-1,
                VTSS_PORT_NO_START+L2_MAX_PORTS+L2_MAX_LLAGS,
                VTSS_PORT_NO_START+L2_MAX_PORTS+L2_MAX_LLAGS+L2_MAX_GLAGS-1);
    }
}

static void log_bridge_vector(const char *str, const mstp_bridge_vector_t *vec)
{
    char root[32], regRoot[32], desg[32];
    (void) vtss_mstp_bridge2str(root, sizeof(root), vec->rootBridgeId);
    (void) vtss_mstp_bridge2str(regRoot, sizeof(regRoot), vec->regRootBridgeId);
    (void) vtss_mstp_bridge2str(desg, sizeof(desg), vec->DesignatedBridgeId);
    CPRINTF("%s: %s-[%d]-%s-[%d] desg %s portId %02x-%02x\n", str, 
            root, 
            vec->extRootPathCost,
            regRoot,
            vec->intRootPathCost,
            desg,
            vec->DesignatedPortId[0], 
            vec->DesignatedPortId[1]);
}

static void cli_cmd_mstp_debug_pstate(cli_req_t *req)
{
    mstp_cli_req_t *mstp_req = req->module_req;
    l2_port_no_t l2port = req->int_values[0];
    mstp_port_mgmt_status_t status, *ps = &status;
    mstp_port_vectors_t vectors;
    if(mstp_get_port_status(mstp_req->msti, l2port, ps) && ps->active &&
       mstp_get_port_vectors(mstp_req->msti, l2port, &vectors)) {
        CPRINTF("Port      : %s %s\n", l2port2str(l2port), msti_name(mstp_req->msti));
        CPRINTF("Role      : %s\n", ps->core.rolestr);
        CPRINTF("State     : %s\n", ps->core.statestr);
        CPRINTF("tcAck     : %s\n", ps->core.tcAck ? "True" : "False");
        CPRINTF("disputed  : %s\n", ps->core.disputed ? "True" : "False");
        CPRINTF("Vectors : root-[extCost]-regRoot-[intCost] Designated Bridge, PortID\n");
        log_bridge_vector("Designated", &vectors.designated);
        log_bridge_vector("Port      ", &vectors.port);
        log_bridge_vector("Message   ", &vectors.message);
        CPRINTF("InfoIs    : %s\n", vectors.infoIs);
    } else {
        CPRINTF("Port %s in %s is not active\n", 
                l2port2str(l2port), msti_name(mstp_req->msti));
    }
}

/****************************************************************************/
/*  Parameter functions                                                     */
/****************************************************************************/

static int mstp_cli_parm_parse_port_list(char *cmd, char *cmd2,
                        char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    ulong max = VTSS_PORTS;

    req->parm_parsed = 1;
    error = (cli_parse_all(cmd) && cli_parm_parse_list(cmd, req->uport_list, 0, max, 1));
    return(error);
}

static int mstp_cli_parm_parse_keyword(char *cmd, char *cmd2,
                        char *stx, char *cmd_org, cli_req_t *req)
{
    mstp_cli_req_t *mstp_req = req->module_req;
    char *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;
    if (found != NULL) {
        if (!strncmp(found, "auto", 4)) {
            mstp_req->auto_keyword = 1;
        } else if (!strncmp(found, "disable", 7)) {
            req->disable = 1;
        } else if (!strncmp(found, "enable", 6)) {
            req->enable = 1;
        }
    }

    return (found == NULL ? 1 : 0);
}

#if defined(VTSS_MSTP_FULL)
static int mstp_cli_parm_parse_msti(char *cmd, char *cmd2,
                        char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    mstp_cli_req_t *mstp_req = req->module_req;
    ulong value = 0;

    req->parm_parsed = 1;
    if((error = cli_parse_ulong(cmd, &value, 0, 7)) == 0) {
        mstp_req->msti_set = TRUE;
        mstp_req->msti = value;
    }
    return(error);
}
#endif  /* VTSS_MSTP_FULL */

static int mstp_cli_parm_parse_max_age(char *cmd, char *cmd2,
                        char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    mstp_cli_req_t *mstp_req = req->module_req;
    ulong value = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 6, 40);
    if(!error) {
        mstp_bridge_param_t sc;
        (void) mstp_get_system_config(&sc);
        error = !( value <= ((sc.bridgeForwardDelay-1)*2) );
    }
    mstp_req->time = value;
    return(error);
}

static int mstp_cli_parm_parse_delay(char *cmd, char *cmd2,
                        char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    mstp_cli_req_t *mstp_req = req->module_req;
    ulong value = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 4, 30);
    if(!error) {
        mstp_bridge_param_t sc;
        (void) mstp_get_system_config(&sc);
        error = !( sc.bridgeMaxAge <= ((value-1)*2) );
    }
    mstp_req->time = value;
    return(error);
}

static int mstp_cli_parm_parse_holdcount(char *cmd, char *cmd2,
                                         char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    mstp_cli_req_t *mstp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &mstp_req->val, 1, 10);
    return(error);
}

static int mstp_cli_parm_parse_maxhops(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    mstp_cli_req_t *mstp_req = req->module_req;
    ulong value = 0;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 6, 40);
    mstp_req->maxhops = value;
    return(error);
}

#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
static int mstp_cli_parm_parse_recovery_timeout(char *cmd, char *cmd2,
                                                char *stx, char *cmd_org, cli_req_t *req)
{
    int   error = 0;
    mstp_cli_req_t *mstp_req = req->module_req;

    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &mstp_req->val, 0, 86400); /* 24h */
    error = error || (mstp_req->val != 0 && mstp_req->val < 30); /* 0 disables, otherwise 30 minimum */

    return(error);
}
#endif /*VTSS_SW_OPT_MSTP_BPDU_ENH*/

static int mstp_cli_parm_parse_stp_version(char *cmd, char *cmd2,
                                           char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    mstp_cli_req_t *mstp_req = req->module_req;

    req->parm_parsed = 1;
    if (!(error = cli_parse_word(cmd, "mstp")))
        mstp_req->forceVersion = MSTP_PROTOCOL_VERSION_MSTP;
    else if (!(error = cli_parse_word(cmd, "rstp")))
        mstp_req->forceVersion = MSTP_PROTOCOL_VERSION_RSTP;
    else if (!(error = cli_parse_word(cmd, "stp")))
        mstp_req->forceVersion = MSTP_PROTOCOL_VERSION_COMPAT;
    return(error);
}

static int mstp_cli_parm_parse_path_cost(char *cmd, char *cmd2,
                        char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    mstp_cli_req_t *mstp_req = req->module_req;

    req->parm_parsed = 1;
    if ((error = cli_parse_word(cmd, "auto")) != 0)
        error = cli_parse_ulong(cmd, &mstp_req->cost, 1, 200000000);
    return(error);
}

static int mstp_cli_parm_parse_ppriority(char *cmd, char *cmd2,
                                         char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    mstp_cli_req_t *mstp_req = req->module_req;
    ulong value = 0;

    req->parm_parsed = 1;
    error = (cli_parse_ulong(cmd, &value, 0, 240) || (value % 16));
    mstp_req->ppriority = value;
    return(error);
}

static int mstp_cli_parm_parse_bpriority(char *cmd, char *cmd2,
                                         char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;
    mstp_cli_req_t *mstp_req = req->module_req;
    ulong value = 0;
    
    req->parm_parsed = 1;
    error = (cli_parse_ulong(cmd, &value, 0, 61440) || (value % 4096));
    mstp_req->bpriority = value;
    return(error);
}

#if defined(VTSS_MSTP_FULL)
static int mstp_cli_parm_parse_vidrange(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    int            error;
    ulong          start = 0, end = 0;
    mstp_cli_req_t *mstp_req = req->module_req;

    error = cli_parse_range(cmd, &start, &end, VLAN_ID_MIN, MIN(VLAN_ID_MAX, 4094));
    if (error) {
        error = cli_parse_ulong(cmd, &start, VLAN_ID_MIN, MIN(VLAN_ID_MAX, 4094));
        end = start;
    }

    if(!error) {
        mstp_req->vid_start = start;
        mstp_req->vid_end = end;
    }

    return error;
}

static int mstp_cli_parm_parse_conf_name(char *cmd, char *cmd2,
                                         char *stx, char *cmd_org, cli_req_t *req)
{
    int error = 0;

    req->parm_parsed = 1;
    error = cli_parse_text(cmd_org, req->parm, 32+1);
    return(error);
}
#endif

/****************************************************************************/
/*  Parameter table                                                         */
/****************************************************************************/

static cli_parm_t mstp_cli_parm_table[] = {
    {
        "clear",
        "Clear the selected port statistics",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        cli_cmd_mstp_port_statistics
    },
    {
        "<stp_port_list>",
        "Port list or 'all'. Port zero means aggregations.",
        CLI_PARM_FLAG_NONE,
        mstp_cli_parm_parse_port_list,
        NULL
    },
    {
        "enable|disable",
        "enable : Enable MSTP protocol\n"
        "disable: Disable MSTP protocol",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_mstp_port_mode
    },
    {
        "enable|disable",
        "enable : Configure MSTP adminEdge to Edge\n"
        "disable: Configure MSTP adminEdge to Non-edge",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_mstp_port_edge
    },
    {
        "enable|disable",
        "enable : Enable MSTP autoEdge\n"
        "disable: Disable MSTP autoEdge",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_mstp_port_autoedge
    },
    {
        "enable|disable|auto",
        "enable : Enable MSTP point2point\n"
        "disable: Disable MSTP point2point\n"
        "auto   : Automatic MSTP point2point detection",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_keyword,
        NULL
    },
    {
        "enable|disable",
        "enable : Enable MSTP restricted role\n"
        "disable: Disable MSTP restricted role",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_mstp_port_restricted_role
    },
    {
        "enable|disable",
        "enable : Enable MSTP restricted TCN\n"
        "disable: Disable MSTP restricted TCN",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_mstp_port_restricted_tcn
    },
#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
    {
        "enable|disable",
        "enable : Enable port BPDU Guard\n"
            "disable: Disable port BPDU Guard",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_mstp_port_bpdu_guard
    },
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */
#if defined(VTSS_MSTP_FULL)
    {
        "<msti>",
        "STP bridge instance no (0-7, CIST=0, MSTI1=1, ...)",
        CLI_PARM_FLAG_NONE,
        mstp_cli_parm_parse_msti,
        NULL
    },
#endif  /* VTSS_MSTP_FULL */
    {
        "<max_age>",
        "STP maximum age time (6-40, and max_age <= (forward_delay-1)*2)",
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_max_age,
        NULL
    },
    {
        "<delay>",
        "MSTP forward delay (4-30, and max_age <= (forward_delay-1)*2))",
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_delay,
        cli_cmd_mstp_fwddelay
    },
    {
        "<holdcount>",
        "STP Transmit Hold Count (1-10)",
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_holdcount,
        cli_cmd_mstp_txholdct
    },
    {
        "<maxhops>",
        "STP BPDU MaxHops (6-40))",
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_maxhops,
        cli_cmd_mstp_maxhops
    },
#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
    {
        "enable|disable",
        "enable or disable BPDU Filtering for Edge ports",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_mstp_bpdu_filtering
    },
    {
        "enable|disable",
        "enable or disable BPDU Guard for Edge ports",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_mstp_bpdu_guard
    },
    {
        "<timeout>",
        "Time before error-disabled ports are reenabled (30-86400 seconds, 0 disables)\n"
            "(default: Show recovery timeout)",
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_recovery_timeout,
        cli_cmd_mstp_recovery_timeout

    },
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */

    {
        "<stp_version>",
#if defined(VTSS_MSTP_FULL)
        "mstp|rstp|stp\n",
#else
        "rstp|stp\n",
#endif
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_stp_version,
        cli_cmd_mstp_version
    },
    {
        "<path_cost>",
        "STP port path cost (1-200000000) or 'auto'",
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_path_cost,
        NULL
    },
    {
        "<priority>",
        "STP port priority (0/16/32/48/.../224/240)",
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_ppriority,
        cli_cmd_mstp_mstiport_priority
    },
    {
        "<priority>",
        "STP bridge priority (0/4096/8192/12288/.../53248/57344/61440)",
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_bpriority,
        cli_cmd_mstp_msti_priority
    },
#if defined (VTSS_MSTP_FULL)
    {
        "<vid-range>",
        "Single VLAN ID (1-4094) or 'xx-yy' VLAN ID range",
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_vidrange,
        cli_cmd_mstp_mstimap_add,
    },
    {
        "clear",
        "Clear VID to MSTI mapping",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_keyword,
        cli_cmd_mstp_mstimap
    },
    {
        "<config-name>",
        "MSTP Configuration name. "
        "A text string up to 32 characters long.\n"
        "Use quotes (\") to embed spaces in name.",
        CLI_PARM_FLAG_SET,
        mstp_cli_parm_parse_conf_name,
        cli_cmd_mstp_cname
    },
#endif  /* VTSS_MSTP_FULL */
    {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
/*  Command table                                                           */
/****************************************************************************/

/* MSTP CLI Command Sorting Order */
enum {
    CLI_CMD_MSTP_CONF_PRIO = 0,
    CLI_CMD_MSTP_VERSION_PRIO,
    CLI_CMD_MSTP_TXHOLDCT_PRIO,
    CLI_CMD_MSTP_MAXHOPS_PRIO,
    CLI_CMD_MSTP_MAXAGE_PRIO,
    CLI_CMD_MSTP_FWDDELAY_PRIO,
    CLI_CMD_MSTP_CNAME_PRIO,
    CLI_CMD_MSTP_BPDU_FILTERING_PRIO,
    CLI_CMD_MSTP_BPDU_GUARD_PRIO,
    CLI_CMD_MSTP_RECOVERY_TIMEOUT_PRIO,
    CLI_CMD_MSTP_STATUS_PRIO,
    CLI_CMD_MSTP_MSTI_PRIORITY_PRIO,
    CLI_CMD_MSTP_MSTIMAP_PRIO,
    CLI_CMD_MSTP_MSTIMAP_ADD_PRIO,
    CLI_CMD_MSTP_PORT_CONF_PRIO,
    CLI_CMD_MSTP_PORT_MODE_PRIO,
    CLI_CMD_MSTP_PORT_EDGE_PRIO,
    CLI_CMD_MSTP_PORT_AUTOEDGE_PRIO,
    CLI_CMD_MSTP_PORT_P2P_PRIO,
    CLI_CMD_MSTP_PORT_RESTRICTED_ROLE_PRIO,
    CLI_CMD_MSTP_PORT_RESTRICTED_TCN_PRIO,
    CLI_CMD_MSTP_PORT_BPDU_GUARD_PRIO,
    CLI_CMD_MSTP_PORT_STATISTICS_PRIO,
    CLI_CMD_MSTP_PORT_MCHECK_PRIO,
    CLI_CMD_MSTP_MSTIPORT_CONF_PRIO,
    CLI_CMD_MSTP_MSTIPORT_COST_PRIO,
    CLI_CMD_MSTP_MSTIPORT_PRIORITY_PRIO,
    CLI_CMD_DEBUG_MSTP_PORT_PRIO = CLI_CMD_SORT_KEY_DEFAULT,
};

/* Command table entries */
cli_cmd_tab_entry(
        "STP Configuration",
        NULL,
        "Show STP Bridge configuration",
        CLI_CMD_MSTP_CONF_PRIO,
        CLI_CMD_TYPE_STATUS,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_conf,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_SYS_CONF
    );

cli_cmd_tab_entry(
        "STP Version",
        "STP Version [<stp_version>]",
        "Set or show the STP Bridge protocol version",
        CLI_CMD_MSTP_VERSION_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_version,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP Txhold",
        "STP Txhold [<holdcount>]",
        "Set or show the STP Bridge Transmit Hold Count parameter",
        CLI_CMD_MSTP_TXHOLDCT_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_txholdct,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP MaxHops",
        "STP MaxHops [<maxhops>]",
        "Set or show the MSTP Bridge Max Hop Count parameter",
        CLI_CMD_MSTP_MAXHOPS_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_maxhops,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP MaxAge",
        "STP MaxAge [<max_age>]",
        "Set or show the bridge instance maximum age",
        CLI_CMD_MSTP_MAXAGE_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_maxage,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP FwdDelay",
        "STP FwdDelay [<delay>]",
        "Set or show the bridge instance forward delay",
        CLI_CMD_MSTP_FWDDELAY_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_fwddelay,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
cli_cmd_tab_entry(
        "STP bpduFilter",
        "STP bpduFilter [enable|disable]",
        "Set or show edge port BPDU Filtering",
        CLI_CMD_MSTP_BPDU_FILTERING_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_bpdu_filtering,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
        );

cli_cmd_tab_entry(
        "STP bpduGuard",
        "STP bpduGuard [enable|disable]",
        "Set or show edge port BPDU Guard",
        CLI_CMD_MSTP_BPDU_GUARD_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_bpdu_guard,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
        );

cli_cmd_tab_entry(
        "STP recovery",
        "STP recovery [<timeout>]",
        "Set or show edge port error recovery timeout",
        CLI_CMD_MSTP_RECOVERY_TIMEOUT_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_recovery_timeout,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
        );
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */

#if defined(VTSS_MSTP_FULL)
cli_cmd_tab_entry(
        "STP CName",
        "STP CName [<config-name>] [<integer>]",
        "Set or Show MSTP configuration name and revision",
        CLI_CMD_MSTP_CNAME_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_cname,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );
#endif  /* Only full MSTP */

cli_cmd_tab_entry(
        "STP Status" _MSTP_MSTI_CPARAM " [<stp_port_list>]",
        NULL,
        "Show STP Bridge status",
        CLI_CMD_MSTP_STATUS_PRIO,
        CLI_CMD_TYPE_STATUS,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_status,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP" _MSTP_MSTI_INST " Priority" _MSTP_MSTI_CPARAM,
        "STP" _MSTP_MSTI_INST " Priority" _MSTP_MSTI_CPARAM " [<priority>]",
        "Set or show the bridge instance priority",
        CLI_CMD_MSTP_MSTI_PRIORITY_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_msti_priority,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_SYS_CONF
    );

#if defined(VTSS_MSTP_FULL)
cli_cmd_tab_entry(
        "STP Msti Map" _MSTP_MSTI_CPARAM,
        "STP Msti Map" _MSTP_MSTI_CPARAM " [clear]",
        "Show or clear MSTP MSTI VLAN mapping configuration",
        CLI_CMD_MSTP_MSTIMAP_PRIO,
        CLI_CMD_TYPE_STATUS,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_mstimap,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        NULL,
        "STP Msti Add" _MSTP_MSTI_PARAM " <vid-range>",
        "Add a VLAN (single or range) to a MSTI",
        CLI_CMD_MSTP_MSTIMAP_ADD_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_mstimap_add,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );
#endif  /* VTSS_MSTP_FULL */

cli_cmd_tab_entry(
        "STP Port Configuration [<stp_port_list>]",
        NULL,
        "Show STP Port configuration",
        CLI_CMD_MSTP_PORT_CONF_PRIO,
        CLI_CMD_TYPE_STATUS,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_port_conf,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_SYS_CONF
    );

cli_cmd_tab_entry(
        "STP Port Mode [<stp_port_list>]",
        "STP Port Mode [<stp_port_list>] [enable|disable]",
        "Set or show the STP enabling for a port",
        CLI_CMD_MSTP_PORT_MODE_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_port_mode,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP Port Edge [<stp_port_list>]",
        "STP Port Edge [<stp_port_list>] [enable|disable]",
        "Set or show the STP adminEdge port parameter",
        CLI_CMD_MSTP_PORT_EDGE_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_port_edge,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP Port AutoEdge [<stp_port_list>]",
        "STP Port AutoEdge [<stp_port_list>] [enable|disable]",
        "Set or show the STP autoEdge port parameter",
        CLI_CMD_MSTP_PORT_AUTOEDGE_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_port_autoedge,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP Port P2P [<stp_port_list>]",
        "STP Port P2P [<stp_port_list>] [enable|disable|auto]",
        "Set or show the STP point2point port parameter",
        CLI_CMD_MSTP_PORT_P2P_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_port_p2p,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP Port RestrictedRole [<stp_port_list>]",
        "STP Port RestrictedRole [<stp_port_list>] [enable|disable]",
        "Set or show the MSTP restrictedRole port parameter",
        CLI_CMD_MSTP_PORT_RESTRICTED_ROLE_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_port_restricted_role,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP Port RestrictedTcn [<stp_port_list>]",
        "STP Port RestrictedTcn [<stp_port_list>] [enable|disable]",
        "Set or show the MSTP restrictedTcn port parameter",
        CLI_CMD_MSTP_PORT_RESTRICTED_TCN_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_port_restricted_tcn,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

#ifdef VTSS_SW_OPT_MSTP_BPDU_ENH
cli_cmd_tab_entry(
        "STP Port bpduGuard [<stp_port_list>]",
        "STP Port bpduGuard [<stp_port_list>] [enable|disable]",
        "Set or show the bpduGuard port parameter",
        CLI_CMD_MSTP_PORT_BPDU_GUARD_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_port_bpdu_guard,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
        );
#endif /* VTSS_SW_OPT_MSTP_BPDU_ENH */

cli_cmd_tab_entry(
        "STP Port Statistics [<stp_port_list>] [clear]",
        NULL,
        "Show STP port statistics",
        CLI_CMD_MSTP_PORT_STATISTICS_PRIO,
        CLI_CMD_TYPE_STATUS,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_port_statistics,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        NULL,
        "STP Port Mcheck [<stp_port_list>]",
        "Set the STP mCheck (Migration Check) variable for ports",
        CLI_CMD_MSTP_PORT_MCHECK_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_port_mcheck,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

#if defined(VTSS_MSTP_FULL)
cli_cmd_tab_entry(
        "STP" _MSTP_MSTI_PORTINST " Configuration" _MSTP_MSTI_CPARAM " [<stp_port_list>]",
        NULL,
        "Show the STP port instance configuration",
        CLI_CMD_MSTP_MSTIPORT_CONF_PRIO,
        CLI_CMD_TYPE_STATUS,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_mstiport_conf,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );
#endif  /* VTSS_MSTP_FULL */

cli_cmd_tab_entry(
        "STP" _MSTP_MSTI_PORTINST " Cost" _MSTP_MSTI_CPARAM " [<stp_port_list>]",
        "STP" _MSTP_MSTI_PORTINST " Cost" _MSTP_MSTI_CPARAM " [<stp_port_list>] [<path_cost>]",
        "Set or show the STP port instance path cost",
        CLI_CMD_MSTP_MSTIPORT_COST_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_mstiport_cost,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "STP" _MSTP_MSTI_PORTINST " Priority" _MSTP_MSTI_CPARAM " [<stp_port_list>]",
        "STP" _MSTP_MSTI_PORTINST " Priority" _MSTP_MSTI_CPARAM " [<stp_port_list>] [<priority>]",
        "Set or show the STP port instance priority",
        CLI_CMD_MSTP_MSTIPORT_PRIORITY_PRIO,
        CLI_CMD_TYPE_CONF,
        VTSS_MODULE_ID_RSTP,
        cli_cmd_mstp_mstiport_priority,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "Debug STP Port" _MSTP_MSTI_PARAM " <integer>",
        NULL,
        "Dump STP L2 port state",
        CLI_CMD_DEBUG_MSTP_PORT_PRIO,
        CLI_CMD_TYPE_STATUS,
        VTSS_MODULE_ID_DEBUG,
        cli_cmd_mstp_debug_port,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );

cli_cmd_tab_entry(
        "Debug STP PState <integer>" _MSTP_MSTI_CPARAM,
        NULL,
        "Dump STP port state and vectors",
        CLI_CMD_DEBUG_MSTP_PORT_PRIO,
        CLI_CMD_TYPE_STATUS,
        VTSS_MODULE_ID_DEBUG,
        cli_cmd_mstp_debug_pstate,
        NULL,
        mstp_cli_parm_table,
        CLI_CMD_FLAG_NONE
    );


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
