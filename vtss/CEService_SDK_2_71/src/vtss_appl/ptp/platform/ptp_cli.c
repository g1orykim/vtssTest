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

#include "main.h"
#include "cli.h"
#include "cli_api.h"
#include "ptp_api.h"
#include "ptp_cli.h"
#include "ptp.h"
#include "vtss_ptp_local_clock.h"
#include "vtss_tod_api.h"
#include "vtss_module_id.h"
#include "conf_api.h"
#include "vlan_api.h"
#if defined(VTSS_ARCH_SERVAL)
#include "ptp_pim_api.h"
#endif 


#include "vtss_ptp_local_clock.h"

typedef struct {
    uint     ptp_instance_no;
    uchar    ptp_device_type;
    uchar    ptp_clock_id [8];
    longlong ptp_ingresslatency;
    longlong ptp_egresslatency;
    uchar    ptp_pri1;
    uchar    ptp_pri2;
    uchar    ptp_domain;
    u8       ptp_protocol;
    BOOL     ptp_one_way;
    BOOL     ptp_two_step;
    i16      ptp_currentUtcOffset;
    BOOL     ptp_currentUtcOffsetValid;
    BOOL     ptp_leap59;
    BOOL     ptp_leap61;
    BOOL     ptp_timeTraceable;
    BOOL     ptp_frequencyTraceable;
    BOOL     ptp_ptpTimescale;
    uchar    ptp_timeSource;
    uchar    ptp_logAnnounceInterval;
    uchar    ptp_announceReceiptTimeout;
    uchar    ptp_logSyncInterval;
    uchar    ptp_delayMechanism;
    uchar    ptp_logMinPdelayReqInterval;
    longlong ptp_delayAsymmetry;
    uchar    ptp_localClockCmd;
    long     ptp_localClockRatio;
    BOOL     ptp_displayStats;
    BOOL     ptp_p_reg;
    BOOL     ptp_i_reg;
    BOOL     ptp_d_reg;
    short    ptp_delayfilter;
    short    ptp_ap;
    short    ptp_ai;
    short    ptp_ad;
    int      ptp_period;
    int      ptp_dist;
    vtss_ptp_srv_clock_option_t ptp_srv_option;
    short    ptp_synce_threshold;
    short    ptp_synce_ap;
    u32      ptp_duration;
    u32      ptp_ho_filter;
    i32      ptp_adj_threshold;
    uint     ptp_index;
    i8       ptp_request_period;
    ulong    ptp_ip_working;
    cli_spec_t    ptp_ip_working_spec;
    uchar    ptp_egr_lat_cmd;
    ptp_ext_clock_1pps_t       ptp_one_pps_mode;
#if defined(VTSS_ARCH_SERVAL)
    ptp_rs422_mode_t       ptp_rs422_mode;
    u32      ptp_rs422_delay;
    ptp_rs422_protocol_t   ptp_int_protocol_mode;
    u32      ptp_proto_port;
#endif /* defined(VTSS_ARCH_SERVAL) */
    BOOL     ptp_ext_clock_enable;
    BOOL     ptp_vcxo_enable;
    BOOL     ptp_alt_clk;
    u32      ptp_ext_clock_freq;
    int      ptp_ext_clock_action;
    int      ptp_debug_mode;
    BOOL     ptp_tagging_enable;
    u16      ptp_vid;
    u8       ptp_pcp;
    longlong ptp_base_delay;
    longlong ptp_incr_delay;
    BOOL     ptp_internal;
    u32      ptp_mep_inst;
    u32      ptp_stable_offset;
    u32      ptp_offset_ok;
    u32      ptp_offset_fail;
} ptp_cli_req_t;


void ptp_cli_req_init (void)
{
    /* register the size required for ptp req. structure */
    cli_req_size_register(sizeof(ptp_cli_req_t));
}

static char *cli_bool_disp(BOOL b)
{
    return (b ? "True" : "False");
}

static char *cli_one_pps_mode_disp(u8 m)
{
    switch (m) {
        case VTSS_PTP_ONE_PPS_DISABLE: return "Disable";
        case VTSS_PTP_ONE_PPS_OUTPUT: return "Output";
        case VTSS_PTP_ONE_PPS_INPUT: return "Input";
        default: return "unknown";
    }
}

#if defined(VTSS_ARCH_SERVAL)
static char *cli_rs422_mode_disp(ptp_rs422_mode_t m)
{
    switch (m) {
        case VTSS_PTP_RS422_DISABLE: return "Disable";
        case VTSS_PTP_RS422_MAIN_AUTO: return "Main_Auto";
        case VTSS_PTP_RS422_SUB: return "sub";
        case VTSS_PTP_RS422_MAIN_MAN: return "Main_Man";
        default: return "unknown";
    }
}

static char *cli_rs422_proto_disp(ptp_rs422_protocol_t m)
{
    switch (m) {
        case VTSS_PTP_RS422_PROTOCOL_SER: return "Serial";
        case VTSS_PTP_RS422_PROTOCOL_PIM: return "PIM";
        default: return "unknown";
    }
}

#endif /* defined(VTSS_ARCH_SERVAL) */

static char *cli_state_disp(u8 b)
{

    switch (b) {
        case PTP_COMM_STATE_IDLE:
            return "IDLE";
        case PTP_COMM_STATE_INIT:
            return "INIT";
        case PTP_COMM_STATE_CONN:
            return "CONN";
        case PTP_COMM_STATE_SELL:
            return "SELL";
        case PTP_COMM_STATE_SYNC:
            return "SYNC";
        default:
            return "?";
    }
}

static char *cli_protocol_disp(u8 p)
{
    switch (p) {
    case PTP_PROTOCOL_ETHERNET:
        return "Ethernet";
    case PTP_PROTOCOL_IP4MULTI:
        return "IPv4Multi";
    case PTP_PROTOCOL_IP4UNI:
        return "IPv4Uni";
    case PTP_PROTOCOL_OAM:
        return "Oam";
    case PTP_PROTOCOL_1PPS:
        return "1pps";
    default:
        return "?";
    }
}

static char *cli_srv_opt_disp(vtss_ptp_srv_clock_option_t p)
{
    switch (p) {
        case VTSS_PTP_CLOCK_FREE:
            return "free";
        case VTSS_PTP_CLOCK_SYNCE:
            return "synce";
        default:
            return "?";
    }
}

static void ptp_cli_table_header(char *txt, vtss_ptp_cli_pr *pr)
{
    int i, j, len, count = 0;

    cli_printf("%s\n", txt);
    while (*txt == ' ') {
        (void)pr(" ");
        txt++;
    }
    len = strlen(txt);
    for (i = 0; i < len; i++) {
        if (txt[i] == ' ') {
            count++;
        } else {
            for (j = 0; j < count; j++) {
                (void)pr("%c", count > 1 && (j >= (count - 2)) ? ' ' : '-');
            }
            (void)pr("-");
            count = 0;
        }
    }
    for (j = 0; j < count; j++) {
        (void)pr("%c", count > 1 && (j >= (count - 2)) ? ' ' : '-');
    }
    (void)pr("\n");
}

void ptp_show_clock_default_ds(int inst, vtss_ptp_cli_pr *pr)
{
    ptp_clock_default_ds_t bs;
    char str1 [40];
    char str2 [40];

    ptp_cli_table_header("ClockId  DeviceType  2StepFlag  Ports  ClockIdentity            Dom", pr);

    if (ptp_get_clock_default_ds(&bs,inst)) {
        if (bs.deviceType == PTP_DEVICE_NONE) {
            (void)pr("%-9d%-12s\n",
                        inst,
                        DeviceTypeToString(bs.deviceType));
        } else {
            (void)pr("%-9d%-12s%-11s%-7d%-25s%-5d\n",
                        inst,
                        DeviceTypeToString(bs.deviceType),
                        cli_bool_disp(bs.twoStepFlag), bs.numberPorts, ClockIdentityToString(bs.clockIdentity, str1),
                        bs.domainNumber);

            (void)pr("\n");
            cli_table_header("ClockQuality               Pri1  Pri2  ");
            (void)pr("%-27s%-6d%-6d\n",
                        ClockQualityToString(&bs.clockQuality, str2), bs.priority1, bs.priority2);
            (void)pr("\n");
            cli_table_header("Protocol   One-Way    VLAN Tag Enable    VID    PCP ");
            (void)pr("%-11s%-11s%-19s%-7d%-5d\n",
                        cli_protocol_disp(bs.protocol), cli_bool_disp(bs.oneWay),
                        cli_bool_disp(bs.tagging_enable), bs.configured_vid, bs.configured_pcp);
#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
            (void)pr("\n");
            cli_table_header("Mep Id ");
            (void)pr("%-6d\n", bs.mep_instance);
#endif
        }
    } else {
        (void)pr("%-9d%-12s\n", inst, DeviceTypeToString(PTP_DEVICE_NONE));
    }
}


static void ptp_display_clock_default_ds(cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = ( ptp_cli_req_t * )req->module_req;
    ptp_show_clock_default_ds(ptp_req->ptp_instance_no, cli_printf);
}

static void cli_cmd_ptp_clock_create(cli_req_t *req)
{
    ptp_init_clock_ds_t init_ds;
    ptp_clock_default_ds_t bs;
    ptp_cli_req_t * ptp_req = NULL;

    ptp_req = req->module_req;

    if (req->set && ptp_req->ptp_device_type != PTP_DEVICE_NONE) {
        if (ptp_get_clock_default_ds(&bs,ptp_req->ptp_instance_no)) {
            CPRINTF("Cannot create clock instance %d : a %s clock type already exists\n",
                    ptp_req->ptp_instance_no,
                    DeviceTypeToString(bs.deviceType));
        } else {
            init_ds.deviceType = ptp_req->ptp_device_type;
            init_ds.twoStepFlag = ptp_req->ptp_two_step;
            memcpy(init_ds.clockIdentity, ptp_req->ptp_clock_id, sizeof(init_ds.clockIdentity));
            init_ds.oneWay = ptp_req->ptp_one_way;
            init_ds.protocol = ptp_req->ptp_protocol;
            init_ds.tagging_enable = ptp_req->ptp_tagging_enable;
            init_ds.configured_vid = ptp_req->ptp_vid;
            init_ds.configured_pcp = ptp_req->ptp_pcp;
            init_ds.mep_instance = ptp_req->ptp_mep_inst;
            if (!ptp_clock_create(&init_ds,ptp_req->ptp_instance_no)) {
                CPRINTF("Cannot create clock: tried to create more than one transparent clock!\n");
            }
        }
    } else {
        ptp_display_clock_default_ds(req);
    }
}

static void cli_cmd_ptp_clock_delete(cli_req_t *req)
{
    ptp_clock_default_ds_t bs;
    ptp_cli_req_t * ptp_req = NULL;

    ptp_req = req->module_req;

    if (req->set) {
        if (ptp_get_clock_default_ds(&bs,ptp_req->ptp_instance_no)) {
            if (bs.deviceType == ptp_req->ptp_device_type) {
                if (!ptp_clock_delete(ptp_req->ptp_instance_no)) {
                    CPRINTF("Invalid Clock instance %d\n",
                            ptp_req->ptp_instance_no);
                }

            } else {
                CPRINTF("Cannot delete clock instance %d : it is not a %s clock type\n",
                        ptp_req->ptp_instance_no,
                        DeviceTypeToString(ptp_req->ptp_device_type));
            }
        } else {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        }

    } else {
        ptp_display_clock_default_ds(req);
    }
}
static void cli_cmd_ptp_clock_default(cli_req_t *req)
{
    ptp_set_clock_ds_t default_ds;
    ptp_cli_req_t * ptp_req = NULL;

    ptp_req = req->module_req;

    if (req->set) {

        default_ds.priority1 = ptp_req->ptp_pri1;
        default_ds.priority2 = ptp_req->ptp_pri2;
        default_ds.domainNumber = ptp_req->ptp_domain;
        if (!ptp_set_clock_default_ds(&default_ds,ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        }
    } else {
        ptp_display_clock_default_ds(req);
    }
}

void ptp_show_slave_cfg(int inst, vtss_ptp_cli_pr *pr)
{
    ptp_clock_slave_cfg_t slave_cfg;

    ptp_cli_table_header("Stable Offset  Offset Ok      Offset Fail", pr);
    if (ptp_get_clock_slave_config(&slave_cfg,inst)) {
        (void)pr("%-13u  %-13u  %-13u\n", slave_cfg.stable_offset, slave_cfg.offset_ok, slave_cfg.offset_fail);
    }
}

static void cli_cmd_ptp_slave_config(cli_req_t *req)
{
    ptp_clock_slave_cfg_t slave_cfg;
    ptp_cli_req_t * ptp_req = NULL;

    ptp_req = req->module_req;

    if (req->set) {

        slave_cfg.stable_offset = ptp_req->ptp_stable_offset;
        slave_cfg.offset_ok = ptp_req->ptp_offset_ok;
        slave_cfg.offset_fail = ptp_req->ptp_offset_fail;
        if (!ptp_set_clock_slave_config(&slave_cfg,ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        }
    } else {
        ptp_show_slave_cfg(ptp_req->ptp_instance_no, cli_printf);
    }
}

void ptp_show_clock_current_ds(int inst, vtss_ptp_cli_pr *pr)
{
    ptp_clock_current_ds_t bs;
    char str1[40];
    char str2[40];

    ptp_cli_table_header("stpRm  OffsetFromMaster    MeanPathDelay       ", pr);

    if (ptp_get_clock_current_ds(&bs,inst)) {
        (void)pr("%-7d%-20s%s\n",
                bs.stepsRemoved, vtss_tod_TimeInterval_To_String(&bs.offsetFromMaster, str1,','),
                vtss_tod_TimeInterval_To_String(&bs.meanPathDelay, str2,','));
    }
}

static void ptp_display_clock_current_ds(cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;
    ptp_show_clock_current_ds(ptp_req->ptp_instance_no, cli_printf);
}

static char *cli_delaymechanism_disp(uchar d)
{
    switch (d) {
    case 1:
        return "e2e";
    case 2:
        return "p2p";
    default:
        return "?\?\?";
    }
}

#if 0
static char *cli_timesource_disp(uchar t)
{
    char * str;
    switch (t) {
        case 0x10: str = "ATOMIC_CLK"; break;
        case 0x20: str = "GPS"; break;
        case 0x30: str = "TRRS_RADIO"; break;
        case 0x40: str = "PTP"; break;
        case 0x50: str = "NTP"; break;
        case 0x60: str = "HAND_SET"; break;
        case 0x90: str = "OTHER"; break;
        case 0xA0: str = "INTRNL_OSC"; break;
        default: 
            if (t >= 0xF0 && t <= 0xFE) str = "Alternate";
            else  str = "Reserved";
            break;
    }
    return str;
}
#endif

void ptp_show_clock_time_property_ds(int inst, vtss_ptp_cli_pr *pr)
{
    ptp_clock_timeproperties_ds_t bs;
    
    ptp_cli_table_header("UtcOffset  Valid  leap59  leap61  TimeTrac  FreqTrac  ptpTimeScale  TimeSource", pr);
    if (ptp_get_clock_timeproperties_ds(&bs,inst)) {
        (void)pr("%-11d%-7s%-8s%-8s%-10s%-10s%-14s%-10d\n",
                bs.currentUtcOffset,
                cli_bool_disp(bs.currentUtcOffsetValid),
                cli_bool_disp(bs.leap59),
                cli_bool_disp(bs.leap61),
                cli_bool_disp(bs.timeTraceable),
                cli_bool_disp(bs.frequencyTraceable),
                cli_bool_disp(bs.ptpTimescale),
                bs.timeSource);
    }
}

static void ptp_display_clock_timeproperties_ds(cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;
    ptp_show_clock_time_property_ds(ptp_req->ptp_instance_no, cli_printf);
}

void ptp_show_clock_port_ds_ds(int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    ptp_port_ds_t bs;
    char str1[24];
    char str2[24];
    char str3[24];
    char str4[24];

    if (first) {
        ptp_cli_table_header("Port  Stat  MDR  PeerMeanPathDel  Anv  ATo  Syv  Dlm  MPR  DelayAsymmetry   IngressLatency   EgressLatency    Ver", pr);
    }
    if (ptp_get_port_ds(&bs, uport, inst)) {
        CPRINTF("%-6d%-6s%-5d%-17s%-5d%-5d%-3d%s%-5s%-5d%-17s%-17s%-17s%-3d\n",
                bs.portIdentity.portNumber,
                PortStateToString(bs.portState),bs.logMinDelayReqInterval,
                vtss_tod_TimeInterval_To_String(&bs.peerMeanPathDelay, str1,','),bs.logAnnounceInterval,
                bs.announceReceiptTimeout,bs.logSyncInterval, bs.syncIntervalError ? "* " : "  ",
                cli_delaymechanism_disp(bs.delayMechanism),
                bs.logMinPdelayReqInterval,
                vtss_tod_TimeInterval_To_String(&bs.delayAsymmetry, str2,','),
                vtss_tod_TimeInterval_To_String(&bs.ingressLatency, str3,','),
                vtss_tod_TimeInterval_To_String(&bs.egressLatency, str4,','),
                bs.versionNumber);
    }
}

static void ptp_display_port_ds(cli_req_t *req)
{

    vtss_port_no_t     port_no;
    ptp_cli_req_t * ptp_req = req->module_req;
    BOOL first = TRUE;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        ptp_show_clock_port_ds_ds(ptp_req->ptp_instance_no, iport2uport(port_no), first, cli_printf);
        first = FALSE;
    }
}

void ptp_show_clock_parent_ds(int inst, vtss_ptp_cli_pr *pr)
{
    ptp_clock_parent_ds_t bs;
    char str1 [40];
    char str2 [40];
    char str3 [40];
    ptp_cli_table_header("ParentPortIdentity      port  Pstat  Var  ChangeRate  ", pr);
    if (ptp_get_clock_parent_ds(&bs,inst)) {
        (void)pr("%-24s%-6d%-7s%-5d%-12d\n",
                ClockIdentityToString(bs.parentPortIdentity.clockIdentity, str1), bs.parentPortIdentity.portNumber,
                cli_bool_disp(bs.parentStats),
                bs.observedParentOffsetScaledLogVariance,
                bs.observedParentClockPhaseChangeRate);
        (void)pr("\n");
        ptp_cli_table_header("GrandmasterIdentity      GrandmasterClockQuality    Pri1  Pri2", pr);
        (void)pr("%-25s%-27s%-6d%-6d\n",
                ClockIdentityToString(bs.grandmasterIdentity, str2),
                ClockQualityToString(&bs.grandmasterClockQuality, str3),
                bs.grandmasterPriority1,
                bs.grandmasterPriority2);
    }
}

static void ptp_display_clock_parent_ds(cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;
    ptp_show_clock_parent_ds(ptp_req->ptp_instance_no, cli_printf);
}

/* Set clock default data set */
static void cli_cmd_ptp_clock_timeproperties(cli_req_t *req)
{
    ptp_clock_timeproperties_ds_t bs;
    ptp_cli_req_t * ptp_req = NULL;

    ptp_req = req->module_req;

    if (req->set) {

        bs.currentUtcOffset = ptp_req->ptp_currentUtcOffset;
        bs.currentUtcOffsetValid = ptp_req->ptp_currentUtcOffsetValid;
        bs.leap59 = ptp_req->ptp_leap59;
        bs.leap61 = ptp_req->ptp_leap61;
        bs.timeTraceable = ptp_req->ptp_timeTraceable;
        bs.frequencyTraceable = ptp_req->ptp_frequencyTraceable;
        bs.ptpTimescale = ptp_req->ptp_ptpTimescale;
        bs.timeSource = ptp_req->ptp_timeSource;
        if (!ptp_set_clock_timeproperties_ds(&bs,ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        }

    } else {
        ptp_display_clock_timeproperties_ds(req);
    }
}
static void cli_cmd_ptp_port_dataset(cli_req_t *req)
{

    ptp_set_port_ds_t bs;
    vtss_port_no_t     port_no;
    ptp_cli_req_t * ptp_req = NULL;

    ptp_req = req->module_req;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        if (req->set) {
            if (!ptp_get_port_cfg_ds(iport2uport(port_no),&bs,ptp_req->ptp_instance_no)) {
                CPRINTF("Clock instance %d : does not exist\n",
                        ptp_req->ptp_instance_no);
            }
            bs.logAnnounceInterval = ptp_req->ptp_logAnnounceInterval;
            bs.announceReceiptTimeout = ptp_req->ptp_announceReceiptTimeout;
            bs.logSyncInterval = ptp_req->ptp_logSyncInterval;
            bs.delayMechanism = ptp_req->ptp_delayMechanism;
            bs.logMinPdelayReqInterval = ptp_req->ptp_logMinPdelayReqInterval;
            bs.delayAsymmetry = ptp_req->ptp_delayAsymmetry<<16;
            bs.ingressLatency = ptp_req->ptp_ingresslatency<<16;
            bs.egressLatency = ptp_req->ptp_egresslatency<<16;
            bs.versionNumber = 2;
            if (!ptp_set_port_ds(iport2uport(port_no),&bs,ptp_req->ptp_instance_no)) {
                CPRINTF("Clock instance %d : does not exist\n",
                        ptp_req->ptp_instance_no);
            }
        }
    }
    if (!req->set) {
        ptp_display_port_ds(req);
    }
}

void ptp_show_clock_foreign_master_record_ds(int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    ptp_foreign_ds_t bs;
    char str2 [40];
    char str3 [40];
    i16 ix;
    if(first) {
    ptp_cli_table_header("Port  ForeignmasterIdentity         ForeignmasterClockQality     Pri1  Pri2  Qualif  Best ", pr);
    }
    for (ix = 0; ix < DEFAULT_MAX_FOREIGN_RECORDS; ix++) {
        if (ptp_get_port_foreign_ds(&bs,uport, ix, inst)) {
            (void)pr("%-4d  %-27s%-4d  %-27s%-6d%-6d%-8s%-5s\n",
                    uport,
                    ClockIdentityToString(bs.foreignmasterIdentity.clockIdentity, str2),
                    bs.foreignmasterIdentity.portNumber,
                    ClockQualityToString(&bs.foreignmasterClockQuality, str3),
                    bs.foreignmasterPriority1,
                    bs.foreignmasterPriority2,
                    cli_bool_disp(bs.qualified),
                    cli_bool_disp(bs.best));
        } else {
            continue;
        }
    }
    
}

static void ptp_display_port_foreign_masters(cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;
    vtss_port_no_t     port_no;
    BOOL first = TRUE;
    
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        /* do */
        ptp_show_clock_foreign_master_record_ds(ptp_req->ptp_instance_no, iport2uport(port_no), first, cli_printf);
    }
}

void ptp_show_local_clock(int inst, vtss_ptp_cli_pr *pr)
{
    u32 hw_time;
    vtss_timestamp_t t;
    char str [14];
    static const char * const clock_adjustment_method_txt [] = {
        "Internal Timer",
        "VCXO/(VC)OCXO option",
        "DAC option",
        "Software"
    };
    /* show the local clock  */
    vtss_local_clock_time_get(&t,inst, &hw_time);
    (void)pr("PTP Time (%d)    : %s %s\n", inst, misc_time2str(t.seconds), vtss_tod_ns2str(t.nanoseconds, str,','));
    (void)pr("Clock Adjustment method: %s\n", clock_adjustment_method_txt[vtss_ptp_adjustment_method(inst)]);
}

static void cli_cmd_ptp_local_clock(cli_req_t *req)
{
    vtss_timestamp_t t;
    ptp_cli_req_t * ptp_req = req->module_req;

    if (ptp_req->ptp_localClockCmd == 1) {
        /* update the local clock to the system clock */
        t.sec_msb = 0;
        t.seconds = time(NULL);
        t.nanoseconds = 0;
        vtss_local_clock_time_set(&t,ptp_req->ptp_instance_no);
    } else if (ptp_req->ptp_localClockCmd == 3) {
        /* set the local clock master ratio */
        if (ptp_req->ptp_localClockRatio == 0) {
            vtss_local_clock_ratio_clear(ptp_req->ptp_instance_no);
        } else {
            vtss_local_clock_ratio_set(ptp_req->ptp_localClockRatio,ptp_req->ptp_instance_no);
        }
    } else if (ptp_req->ptp_localClockCmd == 4) {
        /* subtract offset from the local clock */
        vtss_local_clock_adj_offset(ptp_req->ptp_localClockRatio,ptp_req->ptp_instance_no);
    } else {
        /* show the local clock  */
        ptp_show_local_clock(ptp_req->ptp_instance_no, cli_printf);
    }
}

void ptp_show_clock_servo_ds(int inst, vtss_ptp_cli_pr *pr)
{
    vtss_ptp_default_servo_config_t default_servo;
    /* show the servo parameters  */
    ptp_cli_table_header("Display  P-enable  I-enable  D-enable  'P'constant  'I'constant  'D'constant",pr);
    if (ptp_default_servo_parameters_get(&default_servo, inst)) {
        (void)pr("%-9s%-10s%-10s%-10s%-13d%-13d%-13d\n",
                cli_bool_disp(default_servo.display_stats),
                cli_bool_disp(default_servo.p_reg),
                cli_bool_disp(default_servo.i_reg),
                cli_bool_disp(default_servo.d_reg),
                default_servo.ap,
                default_servo.ai,
                default_servo.ad);
    }
}

static void cli_cmd_ptp_clock_servo(cli_req_t *req)
{
    vtss_ptp_default_servo_config_t default_servo;
    ptp_cli_req_t * ptp_req = req->module_req;

    if (req->set) {
        if (!ptp_default_servo_parameters_get(&default_servo, ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n", ptp_req->ptp_instance_no);
            return;
        }
        default_servo.display_stats = ptp_req->ptp_displayStats;
        default_servo.p_reg = ptp_req->ptp_p_reg;
        default_servo.i_reg = ptp_req->ptp_i_reg;
        default_servo.d_reg = ptp_req->ptp_d_reg;
        default_servo.ap = ptp_req->ptp_ap;
        default_servo.ai = ptp_req->ptp_ai;
        default_servo.ad = ptp_req->ptp_ad;
        if (!ptp_default_servo_parameters_set(&default_servo,ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        }
    } else {
        ptp_show_clock_servo_ds(ptp_req->ptp_instance_no, cli_printf);
    }
}

void ptp_show_clock_clk_ds(int inst, vtss_ptp_cli_pr *pr)
{
    vtss_ptp_default_servo_config_t default_servo;
    /* show the servo parameters  */
    ptp_cli_table_header("Option  threshold  'P'constant", pr);
    if (ptp_default_servo_parameters_get(&default_servo, inst)) {
        CPRINTF("%-9s%-11d%-13d\n",
                cli_srv_opt_disp(default_servo.srv_option),
                default_servo.synce_threshold,
                default_servo.synce_ap);
    }
}

static void cli_cmd_ptp_clock_servo_options(cli_req_t *req)
{
    vtss_ptp_default_servo_config_t default_servo;
    ptp_cli_req_t * ptp_req = NULL;

    ptp_req = req->module_req;

    if (!ptp_default_servo_parameters_get(&default_servo, ptp_req->ptp_instance_no)) {
        CPRINTF("Clock instance %d : does not exist\n", ptp_req->ptp_instance_no);
        return;
    }
    if (req->set) {
        default_servo.srv_option = ptp_req->ptp_srv_option;
        default_servo.synce_threshold = ptp_req->ptp_synce_threshold;
        default_servo.synce_ap = ptp_req->ptp_synce_ap;
        
        if (!ptp_default_servo_parameters_set(&default_servo,ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        }
    } else {
        /* show the servo parameters  */
        ptp_show_clock_clk_ds(ptp_req->ptp_instance_no, cli_printf);
    }
}

void ptp_show_clock_ho_ds(int inst, vtss_ptp_cli_pr *pr)
{
    vtss_ptp_default_servo_config_t default_servo;
    vtss_ptp_servo_status_t status;
    /* show the holdover parameters  */
    ptp_cli_table_header("Holdover filter  Adj threshold (ppb)", pr);
    if (ptp_default_servo_parameters_get(&default_servo, inst)) {
        (void)pr("%15d  %17d.%d\n",
                default_servo.ho_filter,
                (i32)default_servo.stable_adj_threshold/10,(i32)default_servo.stable_adj_threshold%10);
    }
    
    /* show the holdover status  */
    if (ptp_default_servo_status_get(&status, inst)) {
        ptp_cli_table_header("Holdover Ok  Holdover offset (ppb)", pr);
        (void)pr("%-11s  %19d.%ld\n",
                status.holdover_ok ? "TRUE" : "FALSE",
                (i32)status.holdover_adj/10, VTSS_LABS((i32)status.holdover_adj%10));
    }
}

static void cli_cmd_ptp_clock_servo_holdover(cli_req_t *req)
{
    vtss_ptp_default_servo_config_t default_servo;
    ptp_cli_req_t * ptp_req = req->module_req;

    if (!ptp_default_servo_parameters_get(&default_servo, ptp_req->ptp_instance_no)) {
        CPRINTF("Clock instance %d : does not exist\n", ptp_req->ptp_instance_no);
        return;
    }
    if (req->set) {
        default_servo.ho_filter = ptp_req->ptp_ho_filter;
        default_servo.stable_adj_threshold = (i64)ptp_req->ptp_adj_threshold*10;
        
        if (!ptp_default_servo_parameters_set(&default_servo,ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        }
    } else {
        /* show the holdover parameters  */
        ptp_show_clock_ho_ds(ptp_req->ptp_instance_no, cli_printf);
    }
}

void ptp_show_clock_filter_ds(int inst, vtss_ptp_cli_pr *pr)
{
    vtss_ptp_default_filter_config_t default_offset;
    vtss_ptp_default_delay_filter_config_t default_delay;
    /* show the filter parameters  */
    ptp_cli_table_header("DelayFilter  period  dist", pr);
    if (ptp_default_filter_parameters_get(&default_offset, inst) &&
            ptp_default_delay_filter_parameters_get(&default_delay, inst)) {
        (void)pr("%-13d%-8d%-8d\n",
                default_delay.delay_filter,
                default_offset.period,
                default_offset.dist);
    }
}

static void cli_cmd_ptp_clock_filter(cli_req_t *req)
{
    vtss_ptp_default_filter_config_t default_offset;
    vtss_ptp_default_delay_filter_config_t default_delay;
    ptp_cli_req_t * ptp_req = req->module_req;

    if (req->set) {
        default_delay.delay_filter = ptp_req->ptp_delayfilter;
        default_offset.period = ptp_req->ptp_period;
        default_offset.dist = ptp_req->ptp_dist;
        if (!ptp_default_filter_parameters_set(&default_offset, ptp_req->ptp_instance_no) ||
                !ptp_default_delay_filter_parameters_set(&default_delay, ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        }
    } else {
        /* show the filter parameters  */
        ptp_show_clock_filter_ds(ptp_req->ptp_instance_no, cli_printf);
    }
}

void ptp_show_clock_port_state_ds(int inst, vtss_uport_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    ptp_port_ds_t bs;
    vtss_ptp_port_link_state_t ds;
    if (first) {
        ptp_cli_table_header("Port  PTP-State  Internal  Link  Port-Timer  Vlan-forw  Phy-timestamper  Peer-delay", pr);
    }
    if (ptp_get_port_ds(&bs, uport,inst) &&
            ptp_get_port_link_state(&ds, uport, inst)) {
        (void)pr("%4d  %-9s  %-8s  %-4s  %-10s  %-9s  %-15s  %-10s\n",
                bs.portIdentity.portNumber,
                PortStateToString(bs.portState),
                bs.initPortInternal ? "TRUE" : "FALSE",
                ds.link_state ? "Up" : "Down",
                ds.in_sync_state ? "In Sync" : "OutOfSync",
                ds.forw_state ? "Forward" : "Discard",
                ds.phy_timestamper ? "TRUE" : "FALSE",
                bs.peer_delay_ok ? "OK" : "FAIL");
    }
}

static void cli_cmd_ptp_port_state(cli_req_t *req)
{
    vtss_port_no_t     port_no;
    BOOL               first;
    ptp_cli_req_t * ptp_req = req->module_req;
    BOOL first_err = FALSE;
    vtss_rc rc;

    first = 1;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        if (req->set) {
            if (req->enable || ptp_req->ptp_internal) {
                if ((rc = ptp_port_ena(ptp_req->ptp_internal, iport2uport(port_no),ptp_req->ptp_instance_no)) != VTSS_RC_OK && !first_err) {
                    CPRINTF("Error enabling inst %d port %d (%s)\n",
                            ptp_req->ptp_instance_no, iport2uport(port_no), error_txt(rc));
                    first_err = TRUE;
                }
            } else if (req->disable) {
                if (!ptp_port_dis( iport2uport(port_no),ptp_req->ptp_instance_no) && !first_err) {
                    CPRINTF("Clock instance %d : does not exist\n",
                            ptp_req->ptp_instance_no);
                    first_err = TRUE;
                }
            }
        } else {
            ptp_show_clock_port_state_ds(ptp_req->ptp_instance_no, iport2uport(port_no), first, cli_printf);
            first = 0;
        }
    }
}

//    PTP UnicastSlaveConf <clockinst> [<index>] [<duration>]  [<ip_addr>]

void ptp_show_clock_uni_ds(int inst, vtss_ptp_cli_pr *pr)
{
    vtss_ptp_unicast_slave_config_state_t conf_state;
    char buf1[20];
    uint ix;
    /* show the unicast parameters  */
    ptp_cli_table_header("index  duration  ip_address       grant  CommState", pr);
    ix = 0;
    while (ptp_uni_slave_conf_state_get(&conf_state, ix++, inst)) {
        (void)pr("%-7d%-10d%-17s%-7d%-9s\n",
                ix-1,
                conf_state.duration,
                misc_ipv4_txt(conf_state.ip_addr, buf1),
                conf_state.log_msg_period,
                cli_state_disp(conf_state.comm_state));
    }
}

static void cli_cmd_ptp_clock_unicast_conf(cli_req_t *req)
{
    vtss_ptp_unicast_slave_config_t uni_slave;
    ptp_cli_req_t * ptp_req = req->module_req;

    if (req->set) {
        uni_slave.duration = ptp_req->ptp_duration;
        uni_slave.ip_addr = ptp_req->ptp_ip_working;
        if (!ptp_uni_slave_conf_set(&uni_slave, ptp_req->ptp_index, ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        }
    } else {
        /* show the unicast parameters  */
        ptp_show_clock_uni_ds(ptp_req->ptp_instance_no, cli_printf);
    }
}

void ptp_show_clock_slave_table_unicast_ds(int inst, vtss_ptp_cli_pr *pr)
{
    vtss_ptp_unicast_slave_table_t uni_slave_table;
    char str1[20];
    char str2[30];
    char str3[30];
    int ix = 0;

    ptp_cli_table_header("ip_addr          stat  mac_addr           port  sourceportidentity           grant ", pr);
    while (ptp_clock_unicast_table_get(&uni_slave_table, ix++, inst)) {
        if (uni_slave_table.conf_master_ip) {
            if (uni_slave_table.comm_state >= PTP_COMM_STATE_CONN) {
                (void)pr("%-17s%-6s%-19s%-6d%-24s,%-3d %-5d\n",
                        misc_ipv4_txt(uni_slave_table.conf_master_ip,str1),
                        cli_state_disp(uni_slave_table.comm_state),
                        misc_mac_txt(uni_slave_table.master.mac,str2),
                        uni_slave_table.port,
                        ClockIdentityToString(uni_slave_table.sourcePortIdentity.clockIdentity,str3),
                        uni_slave_table.sourcePortIdentity.portNumber,
                        uni_slave_table.log_msg_period);
            } else {
                (void)pr("%-17s%-6s\n",
                        misc_ipv4_txt(uni_slave_table.conf_master_ip,str1),
                        cli_state_disp(uni_slave_table.comm_state));
            }
        }
    }
}
//    "PTP SlaveTableUnicast <clockinst> <ip> <state> <mac>  <port>  <source> [<p_grant>]",
static void cli_cmd_ptp_clock_unicast_table(cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;
    ptp_show_clock_slave_table_unicast_ds(ptp_req->ptp_instance_no, cli_printf);
}

void ptp_show_clock_master_table_unicast_ds(int inst, vtss_ptp_cli_pr *pr)
{
    vtss_ptp_unicast_master_table_t uni_master_table;
    char str1[20];
    char str2[30];
    int ix = 0;
    
    ptp_cli_table_header("ip_addr          mac_addr           port  Ann  Sync ", pr);
    while (ptp_clock_unicast_master_table_get(&uni_master_table, ix++, inst)) {
        if (uni_master_table.ann) {
            if (uni_master_table.sync) {
                (void)pr("%-17s%-19s%-6d%-4d %-4d\n",
                        misc_ipv4_txt(uni_master_table.slave.ip,str1),
                        misc_mac_txt(uni_master_table.slave.mac,str2),
                        uni_master_table.port,
                        uni_master_table.ann_log_msg_period,
                        uni_master_table.log_msg_period);
            } else {
                (void)pr("%-17s%-19s%-6d%-4d %-4s\n",
                        misc_ipv4_txt(uni_master_table.slave.ip,str1),
                        misc_mac_txt(uni_master_table.slave.mac,str2),
                        uni_master_table.port,
                        uni_master_table.ann_log_msg_period,
                        "N.A.");
            }
        }
    }
}

//    "PTP MasterTableUnicast <clockinst> <ip> <mac>  <port>  [<ann>] [<sync>]",
static void cli_cmd_ptp_clock_unicast_master_table(cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;

    ptp_show_clock_master_table_unicast_ds(ptp_req->ptp_instance_no, cli_printf);
}

//    "PTP DebugMode <clockinst> <[<debug_mode>]",
static void cli_cmd_ptp_clock_debug_mode(cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ptp_req = req->module_req;
    int debug_mode;

    if (req->set) {
        if (!ptp_debug_mode_set(ptp_req->ptp_debug_mode, ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        }
    } else {
        if (!ptp_debug_mode_get(&debug_mode, ptp_req->ptp_instance_no)) {
            CPRINTF("Clock instance %d : does not exist\n",
                    ptp_req->ptp_instance_no);
        } else {
            /* show the debug mode  */
            cli_table_header("inst  debug_mode");
            CPRINTF("%-4d  %-11d\n",
                        ptp_req->ptp_instance_no,
                        debug_mode);
            }
    }
}

//    "PTP EgressLatency",
static void cli_cmd_ptp_clock_egress_latency(cli_req_t *req)
{
    observed_egr_lat_t lat;
    ptp_cli_req_t * ptp_req = NULL;
    char str1[20];
    char str2[20];
    char str3[20];
    ptp_req = req->module_req;
    if (ptp_req->ptp_egr_lat_cmd == 1) {
        /* clear the Observed Egress latency */
        ptp_clock_egress_latency_clear();
        CPRINTF("Observed Egress Latency counters cleared\n");
    } else {
        cli_table_header("min             mean            max             count ");
        ptp_clock_egress_latency_get(&lat);
        CPRINTF("%-16s%-16s%-16s%-5d\n",
                vtss_tod_TimeInterval_To_String(&lat.min,str1,','),
                vtss_tod_TimeInterval_To_String(&lat.mean,str2,','),
                vtss_tod_TimeInterval_To_String(&lat.max,str3,','),
                lat.cnt);
    }
}

void ptp_show_ext_clock_mode(vtss_ptp_cli_pr *pr)
{
    vtss_ptp_ext_clock_mode_t mode;
    /* show the local clock  */
    vtss_ext_clock_out_get(&mode);
    (void)pr("PTP External One PPS mode: %s, Clock output enabled: %s, frequency : %d, VCXO enable: %s\n", 
            cli_one_pps_mode_disp(mode.one_pps_mode), cli_bool_disp(mode.clock_out_enable), mode.freq,
            cli_bool_disp(mode.vcxo_enable));
}

static void cli_cmd_ptp_ext_clock_mode(cli_req_t *req)
{
    vtss_ptp_ext_clock_mode_t mode;
    ptp_cli_req_t * ptp_req = NULL;
    ptp_req = req->module_req;
    
    if (req->set) {
        /* update the local clock to the system clock */
        mode.one_pps_mode = ptp_req->ptp_one_pps_mode;
        mode.clock_out_enable = ptp_req->ptp_ext_clock_enable;
        mode.vcxo_enable = ptp_req->ptp_vcxo_enable;
        mode.freq = ptp_req->ptp_ext_clock_freq;
        if (mode.clock_out_enable && !ptp_req->ptp_ext_clock_freq) {
            mode.freq = 1;
        }
#if defined(VTSS_ARCH_LUTON26)
        if (mode.one_pps_mode != VTSS_PTP_ONE_PPS_DISABLE && mode.clock_out_enable) {
            CPRINTF("One_pps_mode overrules clock_out_enable, i.e. clock_out_enable is set to false\n");
            mode.clock_out_enable = FALSE;
        }
#endif        
#if defined(VTSS_ARCH_SERVAL)
        if ((mode.one_pps_mode == VTSS_PTP_ONE_PPS_OUTPUT) && mode.clock_out_enable) {
            CPRINTF("One_pps_output overrules clock_out_enable, i.e. clock_out_enable is set to false\n");
            mode.clock_out_enable = FALSE;
        }
#endif
        vtss_ext_clock_out_set(&mode);
    } else {
        /* show the local clock  */
        ptp_show_ext_clock_mode(cli_printf);
    }
}

#if defined(VTSS_ARCH_SERVAL)
void ptp_show_rs422_clock_mode(vtss_ptp_cli_pr *pr)
{
    vtss_ptp_rs422_conf_t mode;
    vtss_ext_clock_rs422_conf_get(&mode);
    (void)pr("PTP RS422 clock mode: %s, delay : %d, Protocol: %s, port: %d\n", 
            cli_rs422_mode_disp(mode.mode), mode.delay,
            cli_rs422_proto_disp(mode.proto), iport2uport(mode.port));
}

static void cli_cmd_ptp_rs422_mode(cli_req_t *req)
{
    vtss_ptp_rs422_conf_t mode;
    ptp_cli_req_t * ptp_req = NULL;
    ptp_req = req->module_req;
    
    if (req->set) {
        /* update the local clock to the system clock */
        mode.mode = ptp_req->ptp_rs422_mode;
        mode.delay = ptp_req->ptp_rs422_delay;
        mode.proto = ptp_req->ptp_int_protocol_mode;
        mode.port  = ptp_req->ptp_proto_port;
        vtss_ext_clock_rs422_conf_set(&mode);
    } else {
        /* show the local clock  */
        ptp_show_rs422_clock_mode(cli_printf);
    }
}
static void cli_cmd_ptp_pim_statistics(cli_req_t *req)
{
    ptp_pim_frame_statistics_t stati;
    vtss_port_no_t                 iport;
    vtss_uport_no_t                uport;
    
    /* show the statistics  */
    for (iport = VTSS_PORT_NO_START; iport < VTSS_PORT_NO_END; iport++) {
        uport = iport2uport(iport);
        if (req->uport_list[uport] == 0) {
            continue;
        }
        ptp_pim_statistics_get(iport, &stati, req->clear);
        if (req->clear) {
            continue;
        }
    
        CPRINTF("Port %d Statistics:\n", uport);
        CPRINTF("%-24s%12u\n", "Requests", stati.request);
        CPRINTF("%-24s%12u\n", "Replies", stati.reply);
        CPRINTF("%-24s%12u\n", "Events", stati.event);
        CPRINTF("%-24s%12u\n", "RX-Dropped", stati.dropped);
        CPRINTF("%-24s%12u\n", "TX-Dropped", stati.tx_dropped);
        CPRINTF("%-24s%12u\n", "Errors", stati.errors);
    }
}
#endif /* VTSS_ARCH_SERVAL */




static void cli_cmd_ptp_ext_clock_action(cli_req_t *req)
{
    vtss_ptp_one_pps_statistic_t one_pps;
    /* one pps action */
    vtss_one_pps_external_input_statistic_get(&one_pps, req->set);
    CPRINTF("One pps statistic: min %d, mean %d, max %d, Signal: %s\n", 
            one_pps.min, one_pps.mean, one_pps.max, one_pps.dLos ? "Fail" : "Ok");
    
    if (req->set) {
        CPRINTF("One pps statistic cleared\n");
    }
}

static void cli_cmd_ptp_wireless_mode(cli_req_t *req)
{
    vtss_port_no_t     port_no;
    BOOL               first;
    BOOL mode;
    ptp_cli_req_t * ptp_req = NULL;

    ptp_req = req->module_req;


    first = 1;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        if (req->set) {
            if (!ptp_port_wireless_delay_mode_set(req->enable, iport2uport(port_no), ptp_req->ptp_instance_no)) {
                CPRINTF("wireless mode not available for ptp instance %d, port %u\n",
                            ptp_req->ptp_instance_no, iport2uport(port_no));
            }
        }
        if (first) {
            cli_table_header("Port  Wireless Mode ");
            first = 0;
        }
        if (ptp_port_wireless_delay_mode_get(&mode, iport2uport(port_no),ptp_req->ptp_instance_no)) {
            CPRINTF("%4u  %-13s\n", iport2uport(port_no), mode ? "Enabled" : "Disabled");
        }
    }
}

static void cli_cmd_ptp_wireless_pre_notif(cli_req_t *req)
{
    vtss_port_no_t     port_no;
    ptp_cli_req_t * ptp_req = NULL;
    ptp_req = req->module_req;

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        if (!ptp_port_wireless_delay_pre_notif(iport2uport(port_no), ptp_req->ptp_instance_no)) {
                CPRINTF("Wireless mode not available for ptp instance %d, port %u\n",
                        ptp_req->ptp_instance_no, iport2uport(port_no));
        } else {
            CPRINTF("Wireless pre notification set for ptp instance %d, port %u\n",
                    ptp_req->ptp_instance_no, iport2uport(port_no));
        }
    }
}

void ptp_show_clock_wireless_ds(int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    BOOL mode;
    vtss_ptp_delay_cfg_t delay_cfg;
    if (first) {
        ptp_cli_table_header("Port  Wireless Mode  Base_delay(ns)  Incr_delay(ns) ", pr);
    }
    if (ptp_port_wireless_delay_mode_get(&mode, uport,inst) &&
            ptp_port_wireless_delay_get(&delay_cfg, uport, inst)) {
        (void)pr("%4u  %-13s  %10d.%03d  %10d.%03d\n", 
                uport, mode ? "Enabled" : "Disabled",
                VTSS_INTERVAL_NS(delay_cfg.base_delay),VTSS_INTERVAL_PS(delay_cfg.base_delay),
                VTSS_INTERVAL_NS(delay_cfg.incr_delay),VTSS_INTERVAL_PS(delay_cfg.incr_delay));
    }
}

static void cli_cmd_ptp_wireless_delay(cli_req_t *req)
{
    vtss_port_no_t     port_no;
    BOOL               first;
    vtss_ptp_delay_cfg_t delay_cfg;
    ptp_cli_req_t * ptp_req = NULL;

    ptp_req = req->module_req;


    first = 1;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        if (req->set) {
            delay_cfg.base_delay = ptp_req->ptp_base_delay*0x10000/1000;
            delay_cfg.incr_delay = ptp_req->ptp_incr_delay*0x10000/1000;
            if (!ptp_port_wireless_delay_set(&delay_cfg, iport2uport(port_no), ptp_req->ptp_instance_no)) {
                CPRINTF("Wireless mode not available for ptp instance %d, port %u\n",
                        ptp_req->ptp_instance_no, iport2uport(port_no));
            }
        }
        ptp_show_clock_wireless_ds(ptp_req->ptp_instance_no, iport2uport(port_no), first, cli_printf);
        if (first) {
            first = 0;
        }
    }
}

void ptp_show_clock_slave_ds(int inst, vtss_ptp_cli_pr *pr)
{
    ptp_clock_slave_ds_t ss;
    char str1 [40];

    ptp_cli_table_header("Slave port  Slave state    Holdover(ppb)  ", pr);
    if (ptp_get_clock_slave_ds(&ss,inst)) {
        (void)pr("%-10d  %-13s  %-13s\n",
                ss.port_number,
                vtss_ptp_slave_state_2_text(ss.slave_state),
                vtss_ptp_ho_state_2_text(ss.holdover_stable, ss.holdover_adj, str1));
    }
}

static void ptp_display_clock_slave_ds(cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;
    ptp_show_clock_slave_ds(ptp_req->ptp_instance_no, cli_printf);
}

#if defined(VTSS_FEATURE_TIMESTAMP)
static void cli_cmd_ptp_backplane_mode(cli_req_t *req)
{
    vtss_port_no_t     port_no;
    BOOL               first;
    vtss_ts_operation_mode_t mode_cfg;

    first = 1;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (req->uport_list[iport2uport(port_no)] == 0)
            continue;
        if (req->set) {
            mode_cfg.mode = req->enable ? TS_MODE_INTERNAL : TS_MODE_EXTERNAL;
            if (VTSS_RC_OK != vtss_ts_operation_mode_set(NULL, port_no, &mode_cfg)) {
                CPRINTF("Could not set portmode for port %u\n",
                        iport2uport(port_no));
            }
        }
        if (first) {
            cli_table_header("Port  Port mode ");
            first = 0;
        }
        if (VTSS_RC_OK == vtss_ts_operation_mode_get(NULL, port_no, &mode_cfg)) {
            CPRINTF("%4u  %-13s\n", 
                    iport2uport(port_no), mode_cfg.mode == TS_MODE_INTERNAL ? "Backplane" : "Normal");
        }
    }
}
#endif /* VTSS_FEATURE_TIMESTAMP */

static void ptp_display_conf(cli_req_t *req, BOOL all)
{
    uint inst;
    ptp_clock_default_ds_t bs;
    ptp_cli_req_t * ptp_req = NULL;

    ptp_req = req->module_req;

    for (inst = 0; inst < PTP_CLOCK_INSTANCES; inst++) {
        if ((inst == ptp_req->ptp_instance_no || all) && (ptp_get_clock_default_ds(&bs,inst))) {
            ptp_req->ptp_instance_no = inst;
            CPRINTF("PTP Clock Config, Clock instance %d\n",ptp_req->ptp_instance_no);
            CPRINTF("*******************************************************************************\n");

            CPRINTF("\nClock Default Data Set:\n");
            ptp_display_clock_default_ds(req);

            CPRINTF("\nClock Current Data Set:\n");
            ptp_display_clock_current_ds(req);

            CPRINTF("\nClock Parent Data Set:\n");
            ptp_display_clock_parent_ds(req);

            CPRINTF("\nClock Slave Data:\n");
            ptp_display_clock_slave_ds(req);

            CPRINTF("\nClock Time Properties Data Set:\n");
            ptp_display_clock_timeproperties_ds(req);

            CPRINTF("\nServo parameters:\n");
            cli_cmd_ptp_clock_servo(req);

            CPRINTF("\nClock optional Servo parameters:\n");
            cli_cmd_ptp_clock_servo_options(req);
            
            CPRINTF("\nFilter parameters:\n");
            cli_cmd_ptp_clock_filter(req);

            CPRINTF("\nUnicast Slave Configuration:\n");
            cli_cmd_ptp_clock_unicast_conf(req);

            CPRINTF("\nPort Data Set(s):\n");
            ptp_display_port_ds(req);

            CPRINTF("\nLocal Clock Current Time:\n");
            cli_cmd_ptp_local_clock(req);
        }
    }
}

static void ptp_display_config ( cli_req_t * req )
{
    ptp_cli_req_t * ptp_req = NULL;

    if (!req->set) {
        cli_header("PTP Configuration", 1);
    }

    ptp_req = req->module_req;

    ptp_display_conf(req, ptp_req->ptp_instance_no == PTP_CLOCK_INSTANCES ? TRUE : FALSE);
}


/* PTP Clock Id , e.g. 00:11:22:ff:ff:33:44:55 */
static int32_t cli_parse_clock_id(char *cmd, char *cmd2,
                                  char *stx, char *cmd_org, cli_req_t *req)
{
    uint i, id[8];
    ptp_cli_req_t * ptp_req = NULL;
    int error = 1;


    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (sscanf(cmd, "%2x:%2x:%2x:%2x:%2x:%2x:%2x:%2x",
               &id[0], &id[1], &id[2], &id[3], &id[4], &id[5], &id[6], &id[7]) == 8) {
        for (i = 0; i < 8; i++)
            ptp_req->ptp_clock_id[i] = (id[i] & 0xff);
        error = 0;
    }
    return error;
}

static int32_t cli_parm_parse_clockinst(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, PTP_CLOCK_INSTANCES-1);
    if (!error) ptp_req->ptp_instance_no = value;

    return (error);
}

static int32_t cli_parm_parse_devtype(char *cmd, char *cmd2,
                                      char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "p2p"))) {
        ptp_req->ptp_device_type = PTP_DEVICE_P2P_TRANSPARENT;
    } else if (0 == (error = cli_parse_word(cmd, "e2e"))) {
        ptp_req->ptp_device_type = PTP_DEVICE_E2E_TRANSPARENT;
    } else if (0 == (error = cli_parse_word(cmd, "ord"))) {
        ptp_req->ptp_device_type = PTP_DEVICE_ORD_BOUND;
    } else if (0 == (error = cli_parse_word(cmd, "slv"))) {
        ptp_req->ptp_device_type = PTP_DEVICE_SLAVE_ONLY;
    } else if (0 == (error = cli_parse_word(cmd, "mst"))) {
        ptp_req->ptp_device_type = PTP_DEVICE_MASTER_ONLY;
    }

    return error;
}

static int32_t cli_parm_parse_ingress_latency(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_longlong(cmd, &ptp_req->ptp_ingresslatency, -100000, 100000);
    return (error);
}

static int32_t cli_parm_parse_egress_latency(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error =  cli_parse_longlong(cmd, &ptp_req->ptp_egresslatency, -100000, 100000);
    return (error);
}

static int32_t cli_parm_parse_priority1(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 255);
    if (!error) ptp_req->ptp_pri1 = value;
    return (error);
}
static int32_t cli_parm_parse_priority2(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 255);
    if (!error) ptp_req->ptp_pri2 = value;
    return (error);
}

static int32_t cli_parm_parse_domain_no(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0, 127);
    if (!error) ptp_req->ptp_domain = value;
    return (error);
}

static int32_t cli_parm_parse_timesource(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    ulong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error =  cli_parse_ulong(cmd, &value, 0, 255);
    if (!error) ptp_req->ptp_timeSource = value;

    return (error);
}

static int32_t cli_parm_parse_announceintv(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    long value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error=  cli_parse_long(cmd, &value, -3, 4);
    if (!error) ptp_req->ptp_logAnnounceInterval = value;
    return (error);
}
static int32_t cli_parm_parse_announceto(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    long value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error =  cli_parse_long(cmd, &value, -1, 10);
    if (!error) ptp_req->ptp_announceReceiptTimeout = value;
    return (error);
}
static int32_t cli_parm_parse_syncintv(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    long value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_long(cmd, &value, -7, 4);
    if (!error) ptp_req->ptp_logSyncInterval = value;
    return (error);
}
static int32_t cli_parm_parse_mindelayreqintv(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    long value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error =  cli_parse_long(cmd, &value, -7, 5);
    if (!error) ptp_req->ptp_logMinPdelayReqInterval = value;
    return (error);
}
static int32_t cli_parm_parse_delayasymmetry(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_longlong(cmd, &ptp_req->ptp_delayAsymmetry, -100000, 100000);
    return (error);
}
static int32_t cli_parm_parse_delayfilter(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    longlong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_longlong(cmd, &value, 1,6);
    if (!error) ptp_req->ptp_delayfilter = value;
    return (error);
}

static int32_t cli_parm_parse_period(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    longlong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_longlong(cmd, &value, 1,10000);
    if (!error) ptp_req->ptp_period = value;
    return (error);
}

static int32_t cli_parm_parse_dist(char *cmd, char *cmd2,
                                   char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    longlong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_longlong(cmd, &value, 1,10);
    if (!error) ptp_req->ptp_dist = value;
    return (error);
}


static int32_t cli_parm_parse_srv_option_threshold(char *cmd, char *cmd2,
                                   char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    longlong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_longlong(cmd, &value, 1,1000);
    if (!error) ptp_req->ptp_synce_threshold = value;
    return (error);
}


static int32_t cli_parm_parse_srv_option_ap(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    longlong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_longlong(cmd, &value, 1,40);
    if (!error) ptp_req->ptp_synce_ap = value;
    return (error);
}

static int32_t cli_parm_parse_srv_ho_filter(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    long value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_long(cmd, &value, 60,86400);
    if (!error) ptp_req->ptp_ho_filter = value;
    return (error);
}


static int32_t cli_parm_parse_srv_adj_threshold(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    long value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_long(cmd, &value, 1, 1000);
    if (!error) ptp_req->ptp_adj_threshold = value;
    return (error);
}

static int32_t cli_parm_parse_ap_enable(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true"))) {
        ptp_req->ptp_p_reg = TRUE;
    } else {
        if (0 == (error = cli_parse_word(cmd, "false"))) {
            ptp_req->ptp_p_reg = FALSE;
        }
    }
    return (error);
}

static int32_t cli_parm_parse_ai_enable(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true"))) {
        ptp_req->ptp_i_reg = TRUE;
    } else {
        if (0 == (error = cli_parse_word(cmd, "false"))) {
            ptp_req->ptp_i_reg = FALSE;
        }
    }
    return (error);
}

static int32_t cli_parm_parse_ad_enable(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true"))) {
        ptp_req->ptp_d_reg = TRUE;
    } else {
        if (0 == (error = cli_parse_word(cmd, "false"))) {
            ptp_req->ptp_d_reg = FALSE;
        }
    }
    return (error);
}

static int32_t cli_parm_parse_servo_ap(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    longlong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_longlong(cmd, &value, 1,1000);
    if (!error) ptp_req->ptp_ap = value;
    return (error);
}

static int32_t cli_parm_parse_servo_ai(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    longlong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_longlong(cmd, &value, 1,10000);
    if (!error) ptp_req->ptp_ai = value;
    return (error);
}

static int32_t cli_parm_parse_servo_ad(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    longlong value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_longlong(cmd, &value, 1,10000);
    if (!error) ptp_req->ptp_ad = value;
    return (error);
}

static int32_t cli_parm_parse_delaystates(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true"))) {
        ptp_req->ptp_displayStats = TRUE;
    } else {
        error = cli_parse_word(cmd, "false");
    }

    return (error);
}

static int32_t cli_parm_parse_update_show(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "update"))) {
        ptp_req->ptp_localClockCmd = 1;
    } else if (0 == (error = cli_parse_word(cmd, "ratio"))) {
        ptp_req->ptp_localClockCmd = 3;
    } else if (0 == (error = cli_parse_word(cmd, "offset"))) {
        ptp_req->ptp_localClockCmd = 4;
    } else {
        if (0 == (error = cli_parse_word(cmd, "show")))
            ptp_req->ptp_localClockCmd = 2;
    }
    return (error);
}

static int32_t cli_parm_parse_port_state(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;
    int error = 0;

    req->parm_parsed = 1;

    if (0 == (error = cli_parse_word(cmd, "enable"))) {
        req->enable = 1;
    } else if (0 == (error = cli_parse_word(cmd, "disable"))) {
        req->disable = 1;
    } else if (0 == (error = cli_parse_word(cmd, "internal"))) {
        ptp_req->ptp_internal = 1;
    }
    return (error);
}

static int32_t cli_parm_parse_synce_free(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "synce"))) {
        ptp_req->ptp_srv_option = VTSS_PTP_CLOCK_SYNCE;
    } else {
        if (0 == (error = cli_parse_word(cmd, "free")))
            ptp_req->ptp_srv_option = VTSS_PTP_CLOCK_FREE;
    }
    return (error);
}

static int32_t cli_parm_parse_show_clear(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "clear"))) {
        ptp_req->ptp_egr_lat_cmd = 1;
    } else {
        if (0 == (error = cli_parse_word(cmd, "show")))
            ptp_req->ptp_egr_lat_cmd = 2;
    }
    return (error);
}



static int32_t cli_parm_parse_clockratio(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    error = cli_parse_long(cmd, &ptp_req->ptp_localClockRatio, -10000000, 10000000);
    return (error);
}


static int32_t cli_parm_parse_delaymech(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "e2e"))) {
        ptp_req->ptp_delayMechanism = 1;
    } else {
        if (0 == (error = cli_parse_word(cmd, "p2p")))
            ptp_req->ptp_delayMechanism = 2;
    }

    return (error);
}

static int32_t cli_parm_parse_utcoffset(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;
    long value;
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error =  cli_parse_long(cmd, &value, -32768, 32767);
    if (!error) ptp_req->ptp_currentUtcOffset = value;
    return (error);
}

static int32_t cli_parm_parse_valid_offset(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true")))
        ptp_req->ptp_currentUtcOffsetValid = TRUE;
    else
        error = cli_parse_word(cmd, "false");

    return (error);

}
static int32_t cli_parm_parse_leap59(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true")))
        ptp_req->ptp_leap59 = TRUE;
    else
        error = cli_parse_word(cmd, "false");

    return (error);
}

static int32_t cli_parm_parse_timetrac(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true")))
        ptp_req->ptp_timeTraceable = TRUE;
    else
        error = cli_parse_word(cmd, "false");

    return (error);
}

static int32_t cli_parm_parse_freqtrac(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true")))
        ptp_req->ptp_frequencyTraceable = TRUE;
    else
        error = cli_parse_word(cmd, "false");

    return (error);
}

static int32_t cli_parm_parse_ptptimescale(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true")))
        ptp_req->ptp_ptpTimescale = TRUE;
    else
        error = cli_parse_word(cmd, "false");

    return (error);
}

static int32_t cli_parm_parse_leap61(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true")))
        ptp_req->ptp_leap61 = TRUE;
    else
        error = cli_parse_word(cmd, "false");

    return (error);
}


static int32_t cli_parm_parse_protocol(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    if (0 == (error = cli_parse_word(cmd, "ip4multi"))) {
        ptp_req->ptp_protocol = PTP_PROTOCOL_IP4MULTI;
    } else if (0 == (error = cli_parse_word(cmd, "ip4uni"))) {
        ptp_req->ptp_protocol = PTP_PROTOCOL_IP4UNI;
    } else if (0 == (error = cli_parse_word(cmd, "oam"))) {
        ptp_req->ptp_protocol = PTP_PROTOCOL_OAM;
    } else if (0 == (error = cli_parse_word(cmd, "1pps"))) {
        ptp_req->ptp_protocol = PTP_PROTOCOL_1PPS;
    } else {
        error = cli_parse_word(cmd, "ethernet");
    }
    return (error);
}
static int32_t cli_parm_parse_one_way(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true"))) {
        ptp_req->ptp_one_way = TRUE;
    } else {
        error = cli_parse_word(cmd, "false");
    }

    return (error);
}

static int32_t cli_parm_parse_two_step(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true"))) {
        ptp_req->ptp_two_step = TRUE;
    } else {
        error = cli_parse_word(cmd, "false");
    }

    return (error);
}
static int32_t cli_parm_parse_tag_enable(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;

    req->parm_parsed = 1;
    ptp_req = req->module_req;

    if (0 == (error = cli_parse_word(cmd, "true"))) {
        ptp_req->ptp_tagging_enable = TRUE;
    } else if (0 == (error = cli_parse_word(cmd, "false"))){
        ptp_req->ptp_tagging_enable = FALSE;
    } else {
        error = cli_parse_word(cmd, "false");
    }

    return (error);
}
static int32_t cli_parse_vid (char *cmd, char *cmd2, char *stx, 
        char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    ptp_cli_req_t *ptp_req = NULL;

    ptp_req = req->module_req;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, VTSS_VID_NULL, VLAN_ID_MAX);
    if (!error) {
        ptp_req->ptp_vid = value;
    }

    return error;
}
static int32_t cli_parse_pcp (char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    int           error = 0;
    ulong         value = 0;
    ptp_cli_req_t *ptp_req = req->module_req;

    error = cli_parse_ulong(cmd, &value, 0, 7);
    if (!error) {
        ptp_req->ptp_pcp  = value;
    }
    return error;
}

static int32_t cli_parm_parse_duration(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    u32 value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 10,1000);
    if (!error) ptp_req->ptp_duration = value;
    return (error);
}

static int32_t cli_parm_parse_index(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    u32 value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0,MAX_UNICAST_MASTERS_PR_SLAVE-1);
    if (!error) ptp_req->ptp_index = value;
    return (error);
}

static int32_t cli_parm_parse_request_period(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    long value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_long(cmd, &value, -7, 4);
    if (!error) ptp_req->ptp_request_period = value;
    return (error);
}

static int32_t cli_parm_parse_ptp_ip_working(char *cmd, char *cmd2,
                                     char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;
    req->parm_parsed = 1;
    return cli_parse_ipv4(cmd, &ptp_req->ptp_ip_working, NULL, &ptp_req->ptp_ip_working_spec, 0);
}

static int32_t cli_parm_parse_one_pps_mode(char *cmd, char *cmd2,
                                               char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;
    
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    
    if (0 == (error = cli_parse_word(cmd, "output"))) {
        ptp_req->ptp_one_pps_mode = VTSS_PTP_ONE_PPS_OUTPUT;
    } else if (0 == (error = cli_parse_word(cmd, "input"))) {
        ptp_req->ptp_one_pps_mode = VTSS_PTP_ONE_PPS_INPUT;
    } else if (0 == (error = cli_parse_word(cmd, "disable"))) {
        ptp_req->ptp_one_pps_mode = VTSS_PTP_ONE_PPS_DISABLE;
    }
    return (error);
}

#if defined(VTSS_ARCH_SERVAL)
static int32_t cli_parm_parse_rs422_mode(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;
    
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    
    if (0 == (error = cli_parse_word(cmd, "disable"))) {
        ptp_req->ptp_rs422_mode = VTSS_PTP_RS422_DISABLE;
    } else if (0 == (error = cli_parse_word(cmd, "main_auto"))) {
        ptp_req->ptp_rs422_mode = VTSS_PTP_RS422_MAIN_AUTO;
    } else if (0 == (error = cli_parse_word(cmd, "main_man"))) {
        ptp_req->ptp_rs422_mode = VTSS_PTP_RS422_MAIN_MAN;
    } else if (0 == (error = cli_parse_word(cmd, "sub"))) {
        ptp_req->ptp_rs422_mode = VTSS_PTP_RS422_SUB;
    }
    return (error);
}
static int32_t cli_parm_parse_protocol_mode(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;
    
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    
    if (0 == (error = cli_parse_word(cmd, "ser"))) {
        ptp_req->ptp_int_protocol_mode = VTSS_PTP_RS422_PROTOCOL_SER;
    } else if (0 == (error = cli_parse_word(cmd, "pim"))) {
        ptp_req->ptp_int_protocol_mode = VTSS_PTP_RS422_PROTOCOL_PIM;
    }
    return (error);
}
#endif /* defined(VTSS_ARCH_SERVAL) */

static int32_t cli_parm_parse_ext_clock_enable(char *cmd, char *cmd2,
                                               char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;
    
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    
    if (0 == (error = cli_parse_word(cmd, "true"))) {
        ptp_req->ptp_ext_clock_enable = TRUE;
    } else {
        if (0 == (error = cli_parse_word(cmd, "false"))) {
            ptp_req->ptp_ext_clock_enable = FALSE;
        }
    }
    return (error);
}

static int32_t cli_parm_parse_vcxo_enable(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;
    
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    
    if (0 == (error = cli_parse_word(cmd, "true"))) {
        ptp_req->ptp_vcxo_enable = TRUE;
    } else {
        if (0 == (error = cli_parse_word(cmd, "false"))) {
            ptp_req->ptp_vcxo_enable = FALSE;
        }
    }
    return (error);
}

#if defined (VTSS_ARCH_SERVAL)
static int32_t cli_parm_parse_alt_clk(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    int error = 0;
    
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    
    if (0 == (error = cli_parse_word(cmd, "alternative"))) {
        ptp_req->ptp_alt_clk = TRUE;
    } else {
        if (0 == (error = cli_parse_word(cmd, "primary"))) {
            ptp_req->ptp_alt_clk = FALSE;
        }
    }
    return (error);
}
static int32_t cli_parm_parse_pps_delay(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    u32 value;
    
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0,999999999);
    if (!error) ptp_req->ptp_rs422_delay = value;
    return (error);
}

static int32_t cli_parm_parse_pim_port(char *cmd, char *cmd2,
                                        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    u32 value;
    
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1, port_isid_port_count(VTSS_ISID_LOCAL));
    if (!error) ptp_req->ptp_proto_port = uport2iport(value);
    return (error);
}


#endif
static int32_t cli_parm_parse_ext_clock_freq(char *cmd, char *cmd2,
                                             char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    u32 value;
    
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 1,25000000);
    if (!error) ptp_req->ptp_ext_clock_freq = value;
    return (error);
}

static int32_t cli_parm_parse_ext_clock_action(char *cmd, char *cmd2,
                                             char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    u32 value;
    
    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0,3);
    if (!error) ptp_req->ptp_ext_clock_action = value;
    return (error);
}

static int32_t cli_parm_parse_debug_mode(char *cmd, char *cmd2,
                                       char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = NULL;
    ulong error = 0;
    u32 value;

    req->parm_parsed = 1;
    ptp_req = req->module_req;
    error = cli_parse_ulong(cmd, &value, 0,4);
    if (!error) ptp_req->ptp_debug_mode = value;
    return (error);
}

static int32_t cli_parm_parse_base_delay(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;;
    ulong error = 0;
    req->parm_parsed = 1;
    error = cli_parse_longlong(cmd, &ptp_req->ptp_base_delay, 0, 1000000000);
    return (error);
}

static int32_t cli_parm_parse_incr_delay(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;;
    ulong error = 0;
    req->parm_parsed = 1;
    error = cli_parse_longlong(cmd, &ptp_req->ptp_incr_delay, 0, 1000000);
    return (error);
}

static int32_t cli_parm_parse_stable_offset(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;;
    ulong error = 0;
    ulong value = 0;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 0, 1000000);
    if (!error) {
        ptp_req->ptp_stable_offset = value;
    }
    return (error);
}

static int32_t cli_parm_parse_offset_ok(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;;
    ulong error = 0;
    ulong value = 0;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 0, 1000000);
    if (!error) {
        ptp_req->ptp_offset_ok = value;
    }
    return (error);
}

static int32_t cli_parm_parse_offset_fail(char *cmd, char *cmd2,
        char *stx, char *cmd_org, cli_req_t *req)
{
    ptp_cli_req_t * ptp_req = req->module_req;;
    ulong error = 0;
    ulong value = 0;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 0, 1000000);
    if (!error) {
        ptp_req->ptp_offset_fail = value;
    }
    return (error);
}


#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
static int32_t cli_parse_mepid (char *cmd, char *cmd2, char *stx, 
                              char *cmd_org, cli_req_t *req)
{
    i32 error = 0;
    ulong value = 0;
    ptp_cli_req_t *ptp_req = NULL;

    ptp_req = req->module_req;
    req->parm_parsed = 1;
    error = cli_parse_ulong(cmd, &value, 1, 100); 
    if (!error) {
        ptp_req->ptp_mep_inst = value;
    }
    return error;
}
#endif /* defined(VTSS_ARCH_SERVAL) */


static void ptp_cli_req_default_set ( cli_req_t * req )
{

    ptp_cli_req_t * ptp_req = NULL;
    ptp_req = req->module_req;
    memset(ptp_req, 0, sizeof(ptp_cli_req_t));
    
    
    ptp_req->ptp_ip_working_spec = CLI_SPEC_VAL;
    ptp_req->ptp_instance_no = PTP_CLOCK_INSTANCES;

    ptp_req->ptp_pri1 = 128;
    ptp_req->ptp_pri2 = 128;
    ptp_req->ptp_two_step = TRUE;
    ptp_req->ptp_logAnnounceInterval = DEFAULT_ANNOUNCE_INTERVAL;
    ptp_req->ptp_announceReceiptTimeout = DEFAULT_ANNOUNCE_RECEIPT_TIMEOUT;
    ptp_req->ptp_logSyncInterval = DEFAULT_SYNC_INTERVAL;
    ptp_req->ptp_delayMechanism = DELAY_MECH_E2E;
    ptp_req->ptp_logMinPdelayReqInterval = DEFAULT_DELAY_REQ_INTERVAL;

    ptp_req->ptp_displayStats = FALSE;
    ptp_req->ptp_p_reg = TRUE;
    ptp_req->ptp_i_reg = TRUE;
    ptp_req->ptp_d_reg = FALSE;
    ptp_req->ptp_delayfilter = 6;
    ptp_req->ptp_ap = 10;
    ptp_req->ptp_ai = 1000;
    ptp_req->ptp_ad = 1000;
    ptp_req->ptp_period = 1;
    ptp_req->ptp_dist = 2;
    ptp_req->ptp_ho_filter = 60;
    ptp_req->ptp_adj_threshold = 30;
    ptp_req->ptp_synce_threshold = 100;
    ptp_req->ptp_synce_ap = 10;
    
}

static void ptp_cli_req_clockc_default_set ( cli_req_t * req )
{
    mac_addr_t my_sysmac;

    ptp_cli_req_t * ptp_req = NULL;
    ptp_req = req->module_req;
    ptp_req->ptp_ip_working_spec = CLI_SPEC_VAL;
    ptp_req->ptp_instance_no = PTP_CLOCK_INSTANCES;
    /* default clock id */
    (void) conf_mgmt_mac_addr_get(my_sysmac, 0);
    memcpy(ptp_req->ptp_clock_id, my_sysmac, 3);
    ptp_req->ptp_clock_id[3] =  0xff;
    ptp_req->ptp_clock_id[4] =  0xfe;
    memcpy(ptp_req->ptp_clock_id+5, my_sysmac+3, 3);
    ptp_req->ptp_vid = 1;
    ptp_req->ptp_mep_inst = 1;
}


static cli_parm_t ptp_cli_parm_table[] = {
    {
        "enable|disable|internal",
        "  enable   : Enable PTP port\n"
        "  disable  : Disable PTP port\n"
        "  internal : Enable PTP port as internal (in a distributed environment)\n"
        "(default: Show actual port state)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_port_state,
        cli_cmd_ptp_port_state,
    },
    {
        "<clockinst>",
        "clock instance number [0..3]",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_clockinst,
        NULL,
    },
    {
        "<devtype>",
        "The devtype parameter takes the following values:\n"
        "  ord           : Ordinary/Boundary clock\n"
        "  p2p           : Peer-to-peer transparent clock\n"
        "  e2e           : End-to-end transparent clock\n"
        "  mst           : Master only clock\n"
        "  slv           : Slave only clock\n"
        "(default: Show actual init parameters)",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_devtype,
        NULL,
    },
    {
        "<twostep>",
        "The twostep parameter takes the following values:\n"
        "  true     : Follow-up messages are used\n"
        "  false    : No follow-up messages are used",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_two_step,
        cli_cmd_ptp_clock_create,
    },
    {
        "<protocol>",
        "The protocol parameter takes the following values:\n"
        "  ethernet : The clock uses multicast Ethernet protocol\n"
        "  ip4multi : The clock uses IPv4 multicast protocol\n"
        "  ip4uni   : The clock uses IPv4 unicast protocol\n"
        "      Note : IPv4 unicast protocol only works in Master only and Slave only clocks\n"
        "             See parameter <devtype>\n"
        "             In a unicast Slave only clock you also need configure which master clocks\n"
        "             to request Announce and Sync messages from. See command UniConfig"
#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
        "\n  oam      : The clock uses OAM demay measurement protocol\n"
        "      Note : oam protocol is a special case used for internal synchronization in a distributed\n"
        "             TC for Serval. A port MEP must be defined for this purpose in the MEP component.\n"
        "             Parameter <mepid> defines the instance number of the port MEP"
#endif
        ,
        CLI_PARM_FLAG_SET,
        cli_parm_parse_protocol,
        cli_cmd_ptp_clock_create,
    },
    {
        "<oneway>",
        "The oneway parameter takes the following values:\n"
        "  true     : The clock slave uses one-way measurements, i.e. no delay requests\n"
        "  false    : The clock slave uses two-way measurements",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_one_way,
        cli_cmd_ptp_clock_create,
    },
    {
        "<clockid>",
        "8 byte clock identity( xx:xx:xx:xx:xx:xx:xx:xx)",
        CLI_PARM_FLAG_SET,
        cli_parse_clock_id,
        cli_cmd_ptp_clock_create,
    },
    {
        "<tag_enable>",
        "The tag_enable parameter takes the following values:\n"
        "  true     : The ptp frames are tagged with the VLAN tag specified in the VID field.\n"
        "      Note : Packets are only tagged if the port is configured for vlan tagging. i.e:\n"
        "               Port Type != Unaware and PortVLAN mode == None.\n"
        "  false    : The ptp frames are sent untagged.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_tag_enable,
        cli_cmd_ptp_clock_create,
    },
    {
        "<prio>",
        "The Prio parameter takes the following values:\n"
        "  0 - 7 : The range of Priorities ptp can use in the tagged frames",
        CLI_PARM_FLAG_SET,
        cli_parse_pcp,
        cli_cmd_ptp_clock_create,
    },
    {
        "<vid>",
        "The VID parameter takes the following values:\n"
        "  0 - 4095 : The range of VID's ptp can use to send tagged frames",
        CLI_PARM_FLAG_SET,
        cli_parse_vid,
        cli_cmd_ptp_clock_create,
    },
    {
        "<ingresslatency>",
        "ingress latency measured in ns",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_ingress_latency,
        NULL,
    },
    {
        "<egresslatency>",
        "egress latency measured in ns",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_egress_latency,
        NULL,
    },
    {
        "<priority1>",
        "[0..255] Clock priority 1 for PTP BMC algorithm",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_priority1,
        cli_cmd_ptp_clock_default,
    },
    {
        "<priority2>",
        "[0..255] Clock Priority 2 for PTP BMC algorithm",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_priority2,
        cli_cmd_ptp_clock_default,
    },
    {
        "<domain>",
        "[0..127] PTP clock domain id (0 = default) for PTP",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_domain_no,
        cli_cmd_ptp_clock_default,
    },
    {
        "<utcoffset>",
        "PTP clock offset between UTC and TAI in seconds",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_utcoffset,
        cli_cmd_ptp_clock_timeproperties,
    },
    {
        "<valid>",
        "The offsetvalid parameter takes the following values:\n"
        "  false       : The UTC offset is not valid\n"
        "  true        : The UTC offset is valid\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_valid_offset,
        cli_cmd_ptp_clock_timeproperties,
    },
    {
        "<leap59>",
        "The leap59 parameter takes the following values:\n"
        "  false       : no leap59 in current day\n"
        "  true        : last minute of current day contains 59 sec.\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_leap59,
        cli_cmd_ptp_clock_timeproperties,
    },
    {
        "<leap61>",
        "The leap61 parameter takes the following values:\n"
        "  false       : no leap61 in current day\n"
        "  true        : last minute of current day contains 61 sec.\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_leap61,
        cli_cmd_ptp_clock_timeproperties,
    },
    {
        "<timetrac>",
        "The timetraceable parameter takes the following values:\n"
        "  false       : timing is not traceable\n"
        "  true        : timing is traceable.\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_timetrac,
        cli_cmd_ptp_clock_timeproperties,
    },
    {
        "<freqtrac>",
        "The freqtraceable parameter takes the following values:\n"
        "  false       : frequency is not traceable\n"
        "  true        : frequency is traceable.\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_freqtrac,
        cli_cmd_ptp_clock_timeproperties,
    },
    {
        "<ptptimescale>",
        "The timescale parameter takes the following values:\n"
        "  false       : timing is not a PTP time scale\n"
        "  true        : timing is a PTP time scale.\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_ptptimescale,
        cli_cmd_ptp_clock_timeproperties,
    },
    {
        "<timesource>",
        "[0..255] Time source.\n"
        "         16 (0x10) ATOMIC_CLOCK\n"
        "         32 (0x20) GPS \n"
        "         48 (0x30) TERRESTRIAL_RADIO\n"
        "         64 (0x40) PTP\n"
        "         80 (0x50) NTP\n"
        "         96 (0x60) HAND_SET\n"
        "        144 (0x90) OTHER\n"
        "        160 (0xA0) INTERNAL_OSCILLATOR\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_timesource,
        cli_cmd_ptp_clock_timeproperties,
    },
    {
        "<announceintv>",
        "[-3..4] Log2 of mean announce interval in sec.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_announceintv,
        cli_cmd_ptp_port_dataset,
    },
    {
        "<announceto>",
        "[-1..10] Log2 of announce receipt timeout in sec.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_announceto,
        cli_cmd_ptp_port_dataset,
    },
    {
        "<syncintv>",
        "[-7..4] Log2 of sync interval in sec.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_syncintv,
        cli_cmd_ptp_port_dataset,
    },
    {
        "<delaymech>",
        "The delaymech parameter takes the following values:\n"
        "  e2e             : The port is configured to use the delay request-response\n"
        "                    mechanism\n"
        "  p2p             : The port is configured to use the peer delay mechanism",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_delaymech,
        cli_cmd_ptp_port_dataset,
    },
    {
        "<minpdelayreqintv>",
        "[-7..5] Log2 of min delay req interval in sec.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_mindelayreqintv,
        cli_cmd_ptp_port_dataset,
    },
    {
        "<delayasymmetry>",
        "path delay asymmetry measured in ns",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_delayasymmetry,
        cli_cmd_ptp_port_dataset,
    },
    {
        "update|show|ratio|offset",
        "PTP local clock\n"
        "  update   : The local clock is synchronized to the eCos system clock\n"
        "  show     : The local clock current time is shown\n"
        "  ratio    : Set the local master clock frequency ratio in units of 0,1 PPB\n"
        "             (ratio > 0 => faster clock, ratio < 0 => slower clock)\n"
        "  offset   : Subtract offset from the local clock in units of ns",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_update_show,
        cli_cmd_ptp_local_clock,
    },
    {
        "<clockratio>",
        "[-10.000.000..+10.000.000] Clock frequency ratio in 0,1 PPB.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_clockratio,
        cli_cmd_ptp_local_clock,
    },



    {
        "<displaystates>",
        "The displaystates parameter takes the following values:\n"
        "  true         : Display clock state and measurements\n"
        "  false        : don't display",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_delaystates,
        cli_cmd_ptp_clock_servo,
    },
    {
        "<delayfilter>",
        "[1..6] Log2 of timeconstant in delay filter.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_delayfilter,
        cli_cmd_ptp_clock_servo,
    },
    {
        "<ap_enable>",
        "  true         : Enable the 'P' component in regulator\n"
        "  false        : Disable the 'P' component in regulator\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_ap_enable,
        cli_cmd_ptp_clock_servo,
    },
    {
        "<ai_enable>",
        "  true         : Enable the 'I' component in regulator\n"
        "  false        : Disable the 'I' component in regulator\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_ai_enable,
        cli_cmd_ptp_clock_servo,
    },
    {
        "<ad_enable>",
        "  true         : Enable the 'D' component in regulator\n"
        "  false        : Disable the 'D' component in regulator\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_ad_enable,
        cli_cmd_ptp_clock_servo,
    },
    {
        "<ap>",
        "[1..1000] 'P' component in regulator",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_servo_ap,
        cli_cmd_ptp_clock_servo,
    },
    {
        "<ai>",
        "[1..10000] 'I' component in regulator.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_servo_ai,
        cli_cmd_ptp_clock_servo,
    },
    {
        "<ad>",
        "[1..10000] 'D' component in regulator.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_servo_ad,
        cli_cmd_ptp_clock_servo,
    },
    {
        "<def_delay_filt>",
        "[1..6] Log2 of timeconstant in delay filter.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_delayfilter,
        cli_cmd_ptp_clock_filter,
    },
    {
        "<period>",
        "[1..10000] Measurement period in number of sync events.\n"
        "        Note: In configurations with Timestamp enabled PHYs, the period is automatically increased,\n"
        "              if (period*dist < SyncPackets pr sec/4), i.e. max 4 adjustments are made pr sec.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_period,
        cli_cmd_ptp_clock_filter,
    },
    {
        "<dist>",
        "[1..10] Distance between servo update n number of measurement periods,\n"
        "        If Distance is 1 the offset is averaged over the 'period',\n"
        "        If Distance is >1 the offset is calculated using 'min' offset.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_dist,
        cli_cmd_ptp_clock_filter,
    },
    {
        "synce|free",
        "PTP clock servo option\n"
        "  synce    : The PTP slave clock oscilator is locked to the master clock by SyncE\n"
        "  free     : The PTP slave clock oscilator is free running",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_synce_free,
        cli_cmd_ptp_clock_servo_options,
    },
    {
        "<threshold>",
        "[1..1000] Threshold in ns for offsetFromMaster defines when the offset increment/decrement mode is entered.\n"
        "                       (default = 100)",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_srv_option_threshold,
        cli_cmd_ptp_clock_servo_options,
    },
    {
        "<ap>",
        "[1..40]  The offset increment/decrement adjustment factor (default = 10)",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_srv_option_ap,
        cli_cmd_ptp_clock_servo_options,
    },
    {
        "<ho_filter>",
        "[60..86400] Holdover filter and stabilization period.\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_srv_ho_filter,
        cli_cmd_ptp_clock_servo_holdover,
    },
    {
        "<adj_threshold>",
        "[1..1000]  max frequency adjustment change within the holdover stabilization period (ppb)",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_srv_adj_threshold,
        cli_cmd_ptp_clock_servo_holdover,
    },
    {
        "<duration>",
        "[10..1000] Number of seconds for which the Announce/Sync messages are requested.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_duration,
        cli_cmd_ptp_clock_unicast_conf,
    },
    {
        "<index>",
        "[0..4] Index in the slave table.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_index,
        cli_cmd_ptp_clock_unicast_conf,
    },
    {
        "<request_period>",
        "[-7..4] Log2 number of seconds for Sync message transmission.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_request_period,
        cli_cmd_ptp_clock_unicast_conf,
    },
    {
        "<ip_addr>",
        "IPv4 address of requested master clock.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_ptp_ip_working,
        cli_cmd_ptp_clock_unicast_conf,
    },
    {
        "show|clear",
        "PTP Ingress latency\n"
        "  show     : Show the observed Egress latency\n"
        "  clear    : Clear the observed Egress latency\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_show_clear,
        cli_cmd_ptp_clock_egress_latency,
    },
    {
        "<one_pps_mode>",
        "output       : Enable the 1 pps clock output\n"
        "                input        : Enable the 1 pps clock input\n"
        "                disable      : Disable the 1 pps clock in/out-put\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_one_pps_mode,
        cli_cmd_ptp_ext_clock_mode,
    },
    {
        "<ext_enable>",
        "true         : Enable the external clock output\n"
        "                false        : Disable the external clock output\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_ext_clock_enable,
        cli_cmd_ptp_ext_clock_mode,
    },
    {
        "<clockfreq>",
        "[1..25.000.000] External Clock output frequency in Hz.\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_ext_clock_freq,
        cli_cmd_ptp_ext_clock_mode,
    },
    {
        "<vcxo_enable>",
        "true         : Enable the external VCXO rate adjustment\n"
        "                false        : Disable the external VCXO rate adjustment\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_vcxo_enable,
        cli_cmd_ptp_ext_clock_mode,
    },
#if defined (VTSS_ARCH_SERVAL)
    {
        "primary|alternative",
        "alternative  : Select alternative clock instance in Serval\n"
        "                primary      : Select primary clock instance\n",
        CLI_PARM_FLAG_NONE,
        cli_parm_parse_alt_clk,
        cli_cmd_ptp_ext_clock_mode,
    },
    {
        "disabled|main_auto|main_man|sub",
        "\n        disabled  : Disable RS422 clock\n"
        "        main_auto : RS422 clock in main_auto mode (1PPS out, save time at L/S input,\n"
        "                  : the pps_delay value sent to the submodule is 'turnaround time'/2 + 19 ns.\n"
        "        main_man  : RS422 clock in main_man mode (1PPS out, send configured pps_delay to sub module.\n"
        "        sub       : RS422 clock in sub mode (save time and load new time at L/S input.\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_rs422_mode,
        cli_cmd_ptp_rs422_mode,
    },
    {
        "<pps_delay>",
        "\n        [0..999.999.999] 1PPS signal delay in ns.\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_pps_delay,
        cli_cmd_ptp_rs422_mode,
    },
    {
        "ser|pim",
        "\n        ser       : Use Serial interface to transfer 1PPS information\n"
        "        pim       : Use pim protocol to transfer 1PPS information over a switch port\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_protocol_mode,
        cli_cmd_ptp_rs422_mode,
    },
    {
        "<pim_port>",
        "\n        [1..N] Switch port used for the PIM protocol in 'pim' mode.\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_pim_port,
        cli_cmd_ptp_rs422_mode,
    },
#endif
    {
        "<debug_mode>",
        "[0..4] Debug mode.",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_debug_mode,
        cli_cmd_ptp_clock_debug_mode,
    },
    { 
        "<one_pps_clear>",
        "default    Dump statistics\n"
        "[1]        Clear statistics.\n",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_ext_clock_action,
        cli_cmd_ptp_ext_clock_action,
    },
    {
        "enable|disable",
        "  enable   : Enable port Wireless mode\n"
        "  disable  : Disable port Wireless mode\n"
        "(default: Show actual port state)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ptp_wireless_mode,
    },
    {
        "<base_delay>",
        "[0..1.000.000.000] Base wireless transmission delay (in picco seconds).",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_base_delay,
        cli_cmd_ptp_wireless_delay,
    },
    {
        "<incr_delay>",
        "[0..1.000.000] Incremental wireless transmission delay pr. byte (in picco seconds).",
        CLI_PARM_FLAG_SET,
        cli_parm_parse_incr_delay,
        cli_cmd_ptp_wireless_delay,
    },
#if defined(VTSS_FEATURE_TIMESTAMP)
    {
        "enable|disable",
        "  enable   : Enable port backplane mode\n"
        "  disable  : Disable port backplane mode\n"
        "(default: Show actual port backplane mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_port_state,
        cli_cmd_ptp_backplane_mode,
    },
#endif /* VTSS_FEATURE_TIMESTAMP */
#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
    {
        "<mepid>",
        "The Mep Id identifies the OAM MEP instance. parameter takes the following values:\n"
        "  1 - n : The range of MEP instances",
        CLI_PARM_FLAG_SET,
        cli_parse_mepid,
        cli_cmd_ptp_clock_create,
    },
#endif /* defined(VTSS_ARCH_SERVAL) */
 {
     "<stable_offset>",
     "[0..1.000.000] Stable offset threshold (in nano seconds).",
     CLI_PARM_FLAG_SET,
     cli_parm_parse_stable_offset,
     cli_cmd_ptp_slave_config,
 },
 {
     "<offset_ok>",
     "[0..1.000.000] Offset threshold for entering the LOCKED state (in nano seconds).",
     CLI_PARM_FLAG_SET,
     cli_parm_parse_offset_ok,
     cli_cmd_ptp_slave_config,
 },
 {
     "<offset_fail>",
     "[0..1.000.000] Offset threshold for leaving the LOCKED state (in nano seconds).",
     CLI_PARM_FLAG_SET,
     cli_parm_parse_offset_fail,
     cli_cmd_ptp_slave_config,
 },
    
    {
        NULL,
        NULL,
        0,
        0,
        NULL,
    },


};

enum {
    PRIO_ID_PTP_CONF,
    PRIO_ID_PTP_PORT_STATE,
    PRIO_ID_PTP_CLOCK_CREATE,
    PRIO_ID_PTP_CLOCK_DELETE,
    PRIO_ID_PTP_CLOCK_DEFAULT,
    PRIO_ID_PTP_CLOCK_CURRENT_DS,
    PRIO_ID_PTP_CLOCK_PARENT_DS,
    PRIO_ID_PTP_CLOCK_TIME_PROPERTIES_DS,
    PRIO_ID_PTP_PORT_DATASET,
    PRIO_ID_PTP_LOCAL_CLOCK,
    PRIO_ID_PTP_CLOCK_SERVO,
    PRIO_ID_PTP_CLOCK_SRV_OPTION,
    PRIO_ID_PTP_CLOCK_SRV_HOLDOVER,
    PRIO_ID_PTP_CLOCK_UNI_SLAVE,
    PRIO_ID_PTP_PORT_FOREIGN_MASTERS,
    PRIO_ID_PTP_CLOCK_UNI_MASTER,
    PRIO_ID_PTP_EXT_CLOCK_OUT,
    PRIO_ID_PTP_RS422_CLOCK,
    PRIO_ID_PTP_EXT_CLOCK_ACTION,
    PRIO_ID_PTP_DEBUG_MODE,
    PRIO_ID_PTP_WIRELESS_MODE,
    PRIO_ID_PTP_WIRELESS_PRE_NOTIF,
    PRIO_ID_PTP_WIRELESS_DELAY,
    PRIO_ID_PTP_BACKPLANE_MODE,
    PRIO_ID_PTP_CLOCK_SLAVE_DS,
    PRIO_ID_PTP_PIM_STATISTICS,
    PRIO_ID_PTP_CLOCK_SLAVE_CFG,
};

cli_cmd_tab_entry (
    "PTP Configuration [<clockinst>]",
    NULL,
    "Show all PTP Clock and Port configuration and status",
    PRIO_ID_PTP_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PTP,
    ptp_display_config,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);
#if defined(VTSS_ARCH_SERVAL) && defined(VTSS_SW_OPTION_MEP)
cli_cmd_tab_entry (
    "PTP ClockCreate <clockinst> [<devtype>] [<twostep>] [<protocol>] [<oneway>] [<clockid>] [<tag_enable>] [<vid>] [<prio>] [<mepid>]",
    NULL,
    "Create or show a PTP clock instance data",
    PRIO_ID_PTP_CLOCK_CREATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_create,
    ptp_cli_req_clockc_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#else
cli_cmd_tab_entry (
    "PTP ClockCreate <clockinst> [<devtype>] [<twostep>] [<protocol>] [<oneway>] [<clockid>] [<tag_enable>] [<vid>] [<prio>]",
    NULL,
    "Create or show a PTP clock instance data",
    PRIO_ID_PTP_CLOCK_CREATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_create,
    ptp_cli_req_clockc_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif    

cli_cmd_tab_entry (
    "PTP ClockDelete <clockinst> [<devtype>]",
    NULL,
    "Delete a PTP clock instance",
    PRIO_ID_PTP_CLOCK_DELETE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_delete,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "PTP DefaultDS <clockinst> [<priority1>] [<priority2>] [<domain>]",
    NULL,
    "Set or show PTP clock Default Data set \n"
    "priority1 and priority2 are used in the best master \n"
    "clock algorithm. Lower values take precedence",
    PRIO_ID_PTP_CLOCK_DEFAULT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_default,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug PTP SlaveCFG <clockinst> [<stable_offset>] [<offset_ok>] [<offset_fail>]",
    NULL,
    "Set or show PTP clock Slave Configuration \n"
    "These parameters defines when the slave state changes.\n"
    "When the offsetFromMaster variation is < <stable_offset>, then the state changes from FREQ_LOCKING to PHASE_LOCKING\n"
    "When the offsetFromMaster is < <offset_ok>, then the state changes from PHASE_LOCKING to LOCKED\n"
    "When the offsetFromMaster is > <offset_fail>, then the state changes from LOCKED to PHASE_LOCKING",
    PRIO_ID_PTP_CLOCK_SLAVE_CFG,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_slave_config,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "PTP CurrentDS <clockinst>",
    NULL,
    "Show PTP clock Current Data set",
    PRIO_ID_PTP_CLOCK_CURRENT_DS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PTP,
    ptp_display_clock_current_ds,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "PTP ParentDS <clockinst>",
    NULL,
    "Show PTP clock Parent Data set.\n",
    PRIO_ID_PTP_CLOCK_PARENT_DS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PTP,
    ptp_display_clock_parent_ds,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "PTP Timingproperties <clockinst> [<utcoffset>] [<valid>] [<leap59>] [<leap61>] [<timetrac>] [<freqtrac>] [<ptptimescale>] [<timesource>]",
    NULL,
    "Set or show PTP clock Timing properties Data set.\n",
    PRIO_ID_PTP_CLOCK_TIME_PROPERTIES_DS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_timeproperties,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "PTP PortDataSet <clockinst> [<port_list>] [<announceintv>] [<announceto>] [<syncintv>] [<delaymech>] [<minpdelayreqintv>] [<delayasymmetry>] [<ingressLatency>] [<egressLatency>]",
    NULL,
    "Set or show PTP port data set",
    PRIO_ID_PTP_PORT_DATASET,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_port_dataset,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "PTP ForeignMasters <clockinst> [<port_list>]",
    NULL,
    "Show PTP port foreign masters data set",
    PRIO_ID_PTP_PORT_FOREIGN_MASTERS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    ptp_display_port_foreign_masters,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "PTP LocalClock <clockinst> [update|show|ratio|offset] [<clockratio>]",
    NULL,
    "Update or show PTP current time, or set master clock ratio",
    PRIO_ID_PTP_LOCAL_CLOCK,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_local_clock,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_ARCH_LUTON26)
cli_cmd_tab_entry (
    "PTP ExtClockMode [<one_pps_mode>] [<ext_enable>] [<clockfreq>] [<vcxo_enable>] ",
    NULL,
    "Update or show the 1PPS and External clock output configuration\n"
        "and vcxo frequency rate adjustment option.\n"
        "Luton26 has only one physical port, i.e. the one pps mode overrules the external clock output,\n"
        "therefore if one_pps_mode != disable, the ext_enable is ignored.\n"
        "(if vcxo mode is changed, the node must be restarted)",
    PRIO_ID_PTP_EXT_CLOCK_OUT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_ext_clock_mode,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif
#if defined(VTSS_ARCH_JAGUAR_1)
cli_cmd_tab_entry (
    "PTP ExtClockMode [<one_pps_mode>] [<ext_enable>] [<clockfreq>] [<vcxo_enable>] ",
    NULL,
    "Update or show the 1PPS and External clock output configuration\n"
    "and vcxo frequency rate adjustment option.\n"
    "(if vcxo mode is changed, the node must be restarted)",
    PRIO_ID_PTP_EXT_CLOCK_OUT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_ext_clock_mode,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

#if defined(VTSS_ARCH_SERVAL)
cli_cmd_tab_entry (
    "PTP ExtClockMode [<one_pps_mode>] [<ext_enable>] [<clockfreq>] [<vcxo_enable>] [primary|alternative]",
    NULL,
    "Update or show the 1PPS and External clock output configuration\n"
    "and vcxo frequency rate adjustment option.\n"
    "Serval has two physical external clocks, and each port can be configured as either 1pps in, one pps out\n"
    "or refclock output. i.e. the one pps mode overrules the external clock outout,\n"
    "therefore if one_pps_mode != disable, the ext_enable is ignored.\n"
    "As default the Synce port is used\n"
    "(if vcxo mode is changed, the node must be restarted)",
    PRIO_ID_PTP_EXT_CLOCK_OUT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_ext_clock_mode,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP RS422ClockMode [disabled|main_auto|main_man|sub] [<pps_delay>] [ser|pim] [<pim_port>]",
    NULL,
    "Set or show the RS422 clock configuration\n"
    "Serval has two physical external clocks, this command sets the alternative clock,\nthat is connected to the RS422 connector.\n"
    "In main_auto mode pps_delay is read only, i.e. it is the turn around delay measured on the 1PPS signal.\n"
    "In main_man mode pps_delay is read/write, i.e. it is the configured nanosec 1PPS delay sent to the sub module.\n"
    "In sub mode pps_delay is read/write, i.e. it is the configured nanosec offset loaded at the L/S input,\n"
    "  this configured value is only used if no pps_delay is received from the main module. I.e. it may be overwritten\n"
    "As default the clock is disabled",
    PRIO_ID_PTP_RS422_CLOCK,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_rs422_mode,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "Debug PTP PIM Statistics [<port_list>]",
    "Debug PTP PIM Statistics [<port_list>] [clear]",
    "Show or clear the PIM protocol statistics",
    PRIO_ID_PTP_PIM_STATISTICS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_pim_statistics,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif

cli_cmd_tab_entry (
    "Debug PTP OnePpsAction [<one_pps_clear>] ",
    NULL,
    "Show [and clear] One PPS statistics",
    PRIO_ID_PTP_EXT_CLOCK_ACTION,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_ext_clock_action,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);


cli_cmd_tab_entry (
    "PTP Servo <clockinst> [<displaystates>] [<ap_enable>] [<ai_enable>] [<ad_enable>] [<ap>] [<ai>] [<ad>]",
    NULL,
    "Set or show PTP clock servo data",
    PRIO_ID_PTP_CLOCK_SERVO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_servo,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP ClkOptions <clockinst> [synce|free] [<threshold>] [<ap>]",
    NULL,
    "Set or show PTP clock options, i.e. if the clock is SyncE locked or freerunning\n"
    "In SyncE mode, the servo is changed from frequency adjustment to offset increment/decrement,\n"
    "when the offset becomes below <threshold> ns.\n"
    "In the offset increment/decrement mode, the offset is changed by offsetFromMaster/<ap>",
    PRIO_ID_PTP_CLOCK_SRV_OPTION,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_servo_options,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP Holdover <clockinst> [<ho_filter>] [<adj_threshold>]",
    NULL,
    "Set or show PTP Servo holdover parameters.",
    PRIO_ID_PTP_CLOCK_SRV_HOLDOVER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_servo_holdover,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP Filter <clockinst> [<def_delay_filt>]  [<period>] [<dist>]",
    NULL,
    "Set or show PTP clock filter data",
    PRIO_ID_PTP_CLOCK_SERVO,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_filter,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP PortState <clockinst> [<port_list>] [enable|disable|internal]",
    NULL,
    "Set or show the PTP port state",
    PRIO_ID_PTP_PORT_STATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_port_state,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
    );
cli_cmd_tab_entry (
    "PTP DebugMode <clockinst> [<debug_mode>]",
    NULL,
    "Set or show the PTP debug mode\n"
    "    debug mode = 0 : no debug mode\n"
    "    debug mode = 1 : dump offset from master on console, and disable clock rate adjustment\n"
    "    debug mode = 2 : dump sync send time, receive time, corr on console, and disable clock rate adjustment\n"
    "    debug mode = 3 : dump delay_Req send time, receive time, corr on console , and disable clock rate adjustment\n"
    "    debug mode = 4 : dump both sync and delay_Req (same as mode 2+3)",
    PRIO_ID_PTP_DEBUG_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_debug_mode,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP UniConfig <clockinst> [<index>] [<duration>]  [<ip_addr>] ",
    NULL,
    "Set or show the Unicast Slave configuration",
    PRIO_ID_PTP_CLOCK_UNI_SLAVE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_unicast_conf,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP SlaveTableUnicast <clockinst>",
    NULL,
    "Show the Unicast slave table of the requested unicast masters",
    PRIO_ID_PTP_CLOCK_UNI_SLAVE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_unicast_table,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP MasterTableUnicast <clockinst>",
    NULL,
    "Show the Unicast master table of the slaves that have requested unicast communication",
    PRIO_ID_PTP_CLOCK_UNI_MASTER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_unicast_master_table,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "Debug PTP EgressLatency [show|clear]",
    NULL,
    "Show or clear the One-step egress latency observed in systems where the timestamping is done in SW",
    PRIO_ID_PTP_CLOCK_UNI_MASTER,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_clock_egress_latency,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP Wireless mode <clockinst> [<port_list>] [enable|disable]",
    NULL,
    "Show or set the wireless mode (this mode is only valid for 2-step BC)",
    PRIO_ID_PTP_WIRELESS_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_wireless_mode,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP Wireless pre notification <clockinst> <port_list>",
    NULL,
    "Set the wireless delay change pre notification (this command is only valid if wireless mode is enabled)",
    PRIO_ID_PTP_WIRELESS_PRE_NOTIF,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_wireless_pre_notif,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP Wireless delay <clockinst> [<port_list>] [<base_delay>] [<incr_delay>]",
    NULL,
    "Show or set the wireless delay",
    PRIO_ID_PTP_WIRELESS_DELAY,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_wireless_delay,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    "PTP SlaveDS <clockinst>",
    NULL,
    "Show PTP clock Slave status Data set.\n",
    PRIO_ID_PTP_CLOCK_SLAVE_DS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_PTP,
    ptp_display_clock_slave_ds,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

#if defined(VTSS_FEATURE_TIMESTAMP)
cli_cmd_tab_entry (
    "Debug PTP backplane mode [<port_list>] [enable|disable]",
    NULL,
    "Show or set the port backplane mode for Serval",
    PRIO_ID_PTP_BACKPLANE_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_PTP,
    cli_cmd_ptp_backplane_mode,
    ptp_cli_req_default_set,
    ptp_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* VTSS_FEATURE_TIMESTAMP */


