/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#include "icli_porting_util.h"
#include "ptp_api.h"
#include "vtss_ptp_local_clock.h"
#include "vtss_tod_api.h"
#include "ptp.h" // For Trace 
#include "msg_api.h" // To check is an isid is valid 
#include "misc_api.h"
#include "tod_api.h"
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "ptp_1pps_sync.h"
#include "ptp_1pps_closed_loop.h"
#endif
#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
#include "zl_3034x_api_pdv_api.h"
#endif
#endif

/***************************************************************************/
/*  Internal types                                                         */
/***************************************************************************/

/***************************************************************************/
/*  Internal functions                                                     */
/***************************************************************************/
static char *  device_type_2_string(u8 type)
{
	switch (type) {
        case PTP_DEVICE_NONE:            return "none";
        case PTP_DEVICE_ORD_BOUND:       return "boundary";
        case PTP_DEVICE_P2P_TRANSPARENT: return "p2ptransparent";
        case PTP_DEVICE_E2E_TRANSPARENT: return "e2etransparent";
        case PTP_DEVICE_SLAVE_ONLY:      return "slave";
        case PTP_DEVICE_MASTER_ONLY:     return "master";
        default:                         return "?";
	}
}

static char *protocol_2_string(u8 p)
{
    switch (p) {
        case PTP_PROTOCOL_ETHERNET:     return "ethernet";
        case PTP_PROTOCOL_IP4MULTI:     return "ip4multi";
        case PTP_PROTOCOL_IP4UNI:       return "ip4uni";
        case PTP_PROTOCOL_OAM:          return "oam";
        case PTP_PROTOCOL_1PPS:         return "onepps";
        default:                        return "?";
    }
}

static char *one_pps_mode_2_string(ptp_ext_clock_1pps_t m)
{
    switch (m) {
        case VTSS_PTP_ONE_PPS_DISABLE: return "disable";
        case VTSS_PTP_ONE_PPS_OUTPUT: return "output";
        case VTSS_PTP_ONE_PPS_INPUT: return "input";
        default: return "unknown";
    }
}

#if defined(VTSS_ARCH_SERVAL)
static char *cli_rs422_mode_2_string(ptp_rs422_mode_t m)
{
    switch (m) {
        case VTSS_PTP_RS422_DISABLE: return "disable";
        case VTSS_PTP_RS422_MAIN_AUTO: return "main-auto";
        case VTSS_PTP_RS422_SUB: return "sub";
        case VTSS_PTP_RS422_MAIN_MAN: return "main-man";
        default: return "unknown";
    }
}

static char *cli_rs422_proto_2_string(ptp_rs422_protocol_t m)
{
    switch (m) {
        case VTSS_PTP_RS422_PROTOCOL_SER: return "ser";
        case VTSS_PTP_RS422_PROTOCOL_PIM: return "pim";
        default: return "unknown";
    }
}

#endif /* defined(VTSS_ARCH_SERVAL) */

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static char *cli_pps_sync_mode_2_string(vtss_1pps_sync_mode_t m)
{
    switch (m) {
        case VTSS_PTP_1PPS_SYNC_MAIN_MAN: return "main-man";
        case VTSS_PTP_1PPS_SYNC_MAIN_AUTO: return "main-auto";
        case VTSS_PTP_1PPS_SYNC_SUB: return "sub";
        case VTSS_PTP_1PPS_SYNC_DISABLE: return "disable";
        default: return "unknown";
    }
}

static char *cli_pps_ser_tod_2_string(vtss_1pps_ser_tod_mode_t m)
{
    switch (m) {
        case VTSS_PTP_1PPS_SER_TOD_MAN: return "ser-man";
        case VTSS_PTP_1PPS_SER_TOD_AUTO: return "ser-auto";
        case VTSS_PTP_1PPS_SER_TOD_DISABLE: return " ";
        default: return "unknown";
    }
}

#endif /* defined(VTSS_FEATURE_PHY_TIMESTAMP) */


/***************************************************************************/
/* ICFG callback functions */
/****************************************************************************/
#ifdef VTSS_SW_OPTION_ICFG
static vtss_rc ptp_icfg_global_conf(const vtss_icfg_query_request_t *req,
                                    vtss_icfg_query_result_t *result)
{
    char str1 [40];
    char buf1[20];
    uint ix;

    int inst;
    ptp_clock_default_ds_t bs;
    ptp_clock_default_ds_t default_bs;
    ptp_clock_timeproperties_ds_t prop;
    ptp_clock_timeproperties_ds_t default_prop;
    vtss_ptp_default_filter_config_t offset;
    vtss_ptp_default_filter_config_t default_offset;
    vtss_ptp_default_delay_filter_config_t delay;
    vtss_ptp_default_delay_filter_config_t default_delay;
    vtss_ptp_default_servo_config_t servo;
    vtss_ptp_default_servo_config_t default_servo;
    vtss_ptp_unicast_slave_config_state_t uni_conf_state;
    vtss_ptp_ext_clock_mode_t ext_clk_mode;
    vtss_ptp_ext_clock_mode_t default_ext_clk_mode;
    ptp_clock_slave_cfg_t slave_cfg;
    ptp_clock_slave_cfg_t default_slave_cfg;

#if defined(VTSS_ARCH_SERVAL)
    vtss_ptp_rs422_conf_t rs422_mode;
    vtss_ptp_rs422_conf_t default_rs422_mode;
#endif    

    ptp_get_default_clock_default_ds(&default_bs);
    ptp_get_clock_default_timeproperties_ds(&default_prop);
    ptp_default_filter_default_parameters_get(&default_offset);
    ptp_default_delay_filter_default_parameters_get(&default_delay);
    ptp_default_servo_default_parameters_get(&default_servo);
    vtss_ext_clock_out_default_get(&default_ext_clk_mode);
    ptp_get_default_clock_slave_config(&default_slave_cfg);
#if defined(VTSS_ARCH_SERVAL)
    vtss_ext_clock_rs422_default_conf_get(&default_rs422_mode);
#endif    

#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
    /* Set external PDV option */ 
    char zl_config[150];
    u32 phase, my_apr;
    BOOL has_one_hz, enable;
    VTSS_RC(apr_server_one_hz_get(&has_one_hz));
    VTSS_RC(apr_adj_min_get(&phase));
    VTSS_RC(apr_config_parameters_get(&my_apr, zl_config));
    VTSS_RC(ptp_get_ext_pdv_config(&enable));
    if (enable)  {
        VTSS_RC(vtss_icfg_printf(result, "ptp ms-pdv %s min-phase %d apr %d\n",
                                 has_one_hz ? "one-hz" : "", phase, my_apr));
    } else if(req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, "no ptp ms-pdv\n"));
    }

#endif    
    /*Set internal TC mode */
    vtss_tod_internal_tc_mode_t tc_mode;
    if (!tod_tc_mode_get(&tc_mode)) {
        tc_mode = VTSS_TOD_INTERNAL_TC_MODE_30BIT;
    }
    if (tc_mode != VTSS_TOD_INTERNAL_TC_MODE_30BIT) {
        VTSS_RC(vtss_icfg_printf(result, "ptp tc-internal mode %d\n", tc_mode));
    }

#if defined(VTSS_FEATURE_PHY_TIMESTAMP) && defined(VTSS_ARCH_JAGUAR_1)
    /* set 1588 referene clock */
    vtss_phy_ts_clockfreq_t freq;
    
    if (tod_ref_clock_freg_get(&freq)) {
        if (freq != VTSS_PHY_TS_CLOCK_FREQ_250M) {
        VTSS_RC(vtss_icfg_printf(result, "ptp ref-clock %s\n",
                                 freq == VTSS_PHY_TS_CLOCK_FREQ_125M ? "mhz125" : 
                                 freq == VTSS_PHY_TS_CLOCK_FREQ_15625M ? "mhz156p25" : "mhz250"));
        } else if(req->all_defaults) {
            VTSS_RC(vtss_icfg_printf(result, "no ptp ref-clock\n"));
        }
    }
#endif /*VTSS_FEATURE_PHY_TIMESTAMP */

    for (inst = 0 ; inst < PTP_CLOCK_INSTANCES; inst++) {
        if (ptp_get_clock_default_ds(&bs, inst)) {
            if (bs.deviceType != default_bs.deviceType) {
                VTSS_RC(vtss_icfg_printf(result, "ptp %d mode %s %s %s %s id %s vid %d %d %s\n",
                                             inst,
                                             device_type_2_string(bs.deviceType),
                                             bs.twoStepFlag ? "twostep" : "onestep",
                                             protocol_2_string(bs.protocol),
                                             bs.oneWay ? "oneway" : "twoway",
                                             ClockIdentityToString(bs.clockIdentity, str1),
                                             bs.configured_vid,
                                             bs.configured_pcp,
                                             bs.tagging_enable ? "tag" : ""));
                if ((bs.priority1 != default_bs.priority1) || (req->all_defaults)) {
                    VTSS_RC(vtss_icfg_printf(result, "ptp %d priority1 %d\n",
                                             inst, bs.priority1));
                }
                if ((bs.priority2 != default_bs.priority2) || (req->all_defaults)) {
                    VTSS_RC(vtss_icfg_printf(result, "ptp %d priority2 %d\n",
                                             inst, bs.priority2));
                }
                if ((bs.domainNumber != default_bs.domainNumber) || (req->all_defaults)) {
                    VTSS_RC(vtss_icfg_printf(result, "ptp %d domain %d\n",
                                             inst, bs.domainNumber));
                }
                if (ptp_get_clock_cfg_timeproperties_ds(&prop, inst)) {
                    if (memcmp(&default_prop,&prop, sizeof(prop)) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, "ptp %d time-property utc-offset %d %s%s%s%s%stime-source %u\n",
                                                 inst,
                                                 prop.currentUtcOffset,
                                                 prop.currentUtcOffsetValid ? "valid " : "",
                                                 prop.leap59 ? "leap-59 " : prop.leap61 ? "leap-61 " : "",
                                                 prop.timeTraceable ? "time-traceable " : "",
                                                 prop.frequencyTraceable ? "freq-traceable " : "",
                                                 prop.ptpTimescale ? "ptptimescale " : "",
                                                 prop.timeSource));
                    }
                }
                if (ptp_default_filter_parameters_get(&offset,inst) && ptp_default_delay_filter_parameters_get(&delay, inst)) {
                    if (memcmp(&default_offset,&offset, sizeof(offset)) || memcmp(&default_delay,&delay, sizeof(delay)) || req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, "ptp %d filter delay %d period %d dist %d\n",
                                                 inst,
                                                 delay.delay_filter,
                                                 offset.period,
                                                 offset.dist));
                    }
                }
                if (ptp_default_servo_parameters_get(&servo, inst)) {
                    if (servo.display_stats != default_servo.display_stats || req->all_defaults) {
                        if (servo.display_stats) {
                            VTSS_RC(vtss_icfg_printf(result, "ptp %d servo displaystates\n", inst));
                        } else {
                            VTSS_RC(vtss_icfg_printf(result, "no ptp %d servo displaystates\n", inst));
                        }
                    }
                    if (servo.p_reg != default_servo.p_reg || servo.ap != default_servo.ap || req->all_defaults) {
                        if (servo.p_reg) {
                            VTSS_RC(vtss_icfg_printf(result, "ptp %d servo ap %d\n", inst, servo.ap));
                        } else {
                            VTSS_RC(vtss_icfg_printf(result, "no ptp %d servo ap\n", inst));
                        }
                    }
                    if (servo.i_reg != default_servo.i_reg || servo.ai != default_servo.ai || req->all_defaults) {
                        if (servo.i_reg) {
                            VTSS_RC(vtss_icfg_printf(result, "ptp %d servo ai %d\n", inst, servo.ai));
                        } else {
                            VTSS_RC(vtss_icfg_printf(result, "no ptp %d servo ai\n", inst));
                        }
                    }
                    if (servo.d_reg != default_servo.d_reg || servo.ad != default_servo.ad || req->all_defaults) {
                        if (servo.d_reg) {
                            VTSS_RC(vtss_icfg_printf(result, "ptp %d servo ad %d\n", inst, servo.ad));
                        } else {
                            VTSS_RC(vtss_icfg_printf(result, "no ptp %d servo ad\n", inst));
                        }
                    }
                    if (servo.srv_option != default_servo.srv_option || servo.synce_threshold != default_servo.synce_threshold || 
                            servo.synce_ap != default_servo.synce_ap || req->all_defaults) {
                        if (servo.srv_option) {
                            VTSS_RC(vtss_icfg_printf(result, "ptp %d clk sync %d ap %d\n", inst, servo.srv_option, servo.synce_ap ));
                        } else {
                            VTSS_RC(vtss_icfg_printf(result, "no ptp %d clk\n", inst));
                        }
                    }
                    if (servo.ho_filter != default_servo.ho_filter || servo.stable_adj_threshold != default_servo.ho_filter || 
                            req->all_defaults) {
                        VTSS_RC(vtss_icfg_printf(result, "ptp %d ho filter %d adj-threshold %d\n", inst, servo.ho_filter, servo.ho_filter ));
                    }

                    ix = 0;
                    while (ptp_uni_slave_conf_state_get(&uni_conf_state, ix++, inst)) {
                        if (uni_conf_state.ip_addr) {
                            VTSS_RC(vtss_icfg_printf(result, "ptp %d uni %d duration %d %s\n", inst, ix-1, uni_conf_state.duration, misc_ipv4_txt(uni_conf_state.ip_addr, buf1) ));
                        } else if (req->all_defaults) {
                            VTSS_RC(vtss_icfg_printf(result, "no ptp %d uni %d\n", inst, ix-1));
                        }
                    }
                    
                    
                }
                if (ptp_get_clock_slave_config(&slave_cfg, inst)) {
                    if (slave_cfg.offset_fail != default_slave_cfg.offset_fail || slave_cfg.offset_ok != default_slave_cfg.offset_ok || 
                        slave_cfg.stable_offset != default_slave_cfg.stable_offset || req->all_defaults) {
                            VTSS_RC(vtss_icfg_printf(result, "ptp %d slave-cfg offset-fail %d offset-ok %d stable-offset %d\n", inst, slave_cfg.offset_fail, slave_cfg.offset_ok, slave_cfg.stable_offset));
                    }
                }
            } else if (req->all_defaults) {
                VTSS_RC(vtss_icfg_printf(result, "no ptp %d mode %s\n",
                                         inst,
                                         device_type_2_string(bs.deviceType)));
            }
        }
    }
    /* global config */
    /* show the local clock  */
    vtss_ext_clock_out_get(&ext_clk_mode);
    if (ext_clk_mode.one_pps_mode != default_ext_clk_mode.one_pps_mode ||
            ext_clk_mode.clock_out_enable != default_ext_clk_mode.clock_out_enable ||
            ext_clk_mode.vcxo_enable != default_ext_clk_mode.vcxo_enable ||
            ext_clk_mode.freq != default_ext_clk_mode.freq) {
        if (ext_clk_mode.clock_out_enable) {
            VTSS_RC(vtss_icfg_printf(result, "ptp ext %s ext %u %s\n",  one_pps_mode_2_string(ext_clk_mode.one_pps_mode), 
                                     ext_clk_mode.freq, ext_clk_mode.vcxo_enable ? "vcxo" : "" ));
        } else {
            VTSS_RC(vtss_icfg_printf(result, "ptp ext %s %s\n",  one_pps_mode_2_string(ext_clk_mode.one_pps_mode), 
                                     ext_clk_mode.vcxo_enable ? "vcxo" : "" ));
        }
    } else if (req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, "no ptp ext\n"));
    }
#if defined(VTSS_ARCH_SERVAL)
    vtss_ext_clock_rs422_conf_get(&rs422_mode);
    char buf[75]; // Buffer for storage of string
    char *port_txt;
    char *int_txt;
    if (rs422_mode.mode != default_rs422_mode.mode) {
        if (rs422_mode.proto == VTSS_PTP_RS422_PROTOCOL_PIM) {
            int_txt = "interface ";
            port_txt = icli_port_info_txt(VTSS_USID_START, iport2uport(rs422_mode.port), buf);
        } else {
            port_txt = "";
            int_txt = "";
        }
        if (rs422_mode.mode == VTSS_PTP_RS422_MAIN_MAN) {
            VTSS_RC(vtss_icfg_printf(result, "ptp rs422 %s pps-delay %u %s %s%s\n",  cli_rs422_mode_2_string(rs422_mode.mode), 
                                     rs422_mode.delay, cli_rs422_proto_2_string(rs422_mode.proto), int_txt, port_txt));
        } else {
            VTSS_RC(vtss_icfg_printf(result, "ptp rs422 %s %s %s%s\n",  cli_rs422_mode_2_string(rs422_mode.mode), 
                                     cli_rs422_proto_2_string(rs422_mode.proto), int_txt, port_txt));
        }
    } else if (req->all_defaults) {
        VTSS_RC(vtss_icfg_printf(result, "no ptp rs422\n"));
    }
#endif /* VTSS_ARCH_SERVAL */
    return VTSS_RC_OK;
}

static vtss_rc ptp_icfg_interface_conf(const vtss_icfg_query_request_t *req,
                                       vtss_icfg_query_result_t *result)
{
    int inst;
    ptp_port_ds_t ps;
    ptp_clock_default_ds_t bs;
    vtss_isid_t isid;
    vtss_port_no_t uport;
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    vtss_1pps_sync_conf_t pps_sync_mode;
    vtss_1pps_closed_loop_conf_t pps_cl_mode;
    vtss_port_no_t iport;
    i8 buf[75]; // Buffer for storage of string
#endif
    isid =  req->instance_id.port.isid;
    uport = req->instance_id.port.begin_uport;
    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "isid %d, port %d",isid, uport);
    if (msg_switch_configurable(isid)) {
        for (inst = 0 ; inst < PTP_CLOCK_INSTANCES; inst++) {
            if (ptp_get_clock_default_ds(&bs, inst)) {
                if (bs.deviceType != PTP_DEVICE_NONE) {
                    if (ptp_get_port_ds(&ps, uport, inst)) {
                        VTSS_RC(vtss_icfg_printf(result, " ptp %d %s\n",
                                                 inst,
                                                 ps.initPortInternal ? "internal" : ""));
                        VTSS_RC(vtss_icfg_printf(result, " ptp %d announce interval %d timeout %d\n",
                                                 inst,
                                                 ps.logAnnounceInterval, 
                                                 ps.announceReceiptTimeout));
                        VTSS_RC(vtss_icfg_printf(result, " ptp %d sync-interval %d\n",
                                                 inst,
                                                 ps.logSyncInterval));
                        VTSS_RC(vtss_icfg_printf(result, " ptp %d delay-mechanism %s\n",
                                                 inst,
                                                 ps.delayMechanism == DELAY_MECH_E2E ? "e2e" : "p2p"));
                        VTSS_RC(vtss_icfg_printf(result, " ptp %d delay-req interval %d\n",
                                                 inst,
                                                 ps.logMinPdelayReqInterval));
                        VTSS_RC(vtss_icfg_printf(result, " ptp %d delay-asymmetry %d\n",
                                                 inst,
                                                 VTSS_INTERVAL_NS(ps.delayAsymmetry)));
                        VTSS_RC(vtss_icfg_printf(result, " ptp %d ingress-latency %d\n",
                                                 inst,
                                                 VTSS_INTERVAL_NS(ps.ingressLatency)));
                        VTSS_RC(vtss_icfg_printf(result, " ptp %d egress-latency %d\n",
                                                 inst,
                                                 VTSS_INTERVAL_NS(ps.egressLatency)));
                    }
                } else if (req->all_defaults) {
                    VTSS_RC(vtss_icfg_printf(result, " no ptp %d\n",
                                             inst));
                }
            }
        }
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
        /* one pps configuration functions */
        iport = req->instance_id.port.begin_iport;
        VTSS_RC(ptp_1pps_sync_mode_get(iport, &pps_sync_mode));
        if (pps_sync_mode.mode != VTSS_PTP_1PPS_SYNC_DISABLE) {
            VTSS_RC(vtss_icfg_printf(result, " ptp pps-sync %s cable-asy %d pps-phase %d %s\n",
                                     cli_pps_sync_mode_2_string(pps_sync_mode.mode),
                                     pps_sync_mode.cable_asy,
                                     pps_sync_mode.pulse_delay,
                                     cli_pps_ser_tod_2_string(pps_sync_mode.serial_tod)));
        } else if (req->all_defaults) {
            VTSS_RC(vtss_icfg_printf(result, " no ptp pps-sync\n"));
        }
        VTSS_RC(ptp_1pps_closed_loop_mode_get(iport, &pps_cl_mode));
        if (pps_cl_mode.mode == VTSS_PTP_1PPS_CLOSED_LOOP_AUTO) {
            VTSS_RC(vtss_icfg_printf(result, " ptp pps-delay auto master-port interface %s\n",
                                     icli_port_info_txt(VTSS_USID_START, iport2uport(pps_cl_mode.master_port), buf)));
        } else if (pps_cl_mode.mode == VTSS_PTP_1PPS_CLOSED_LOOP_MAN) {
                VTSS_RC(vtss_icfg_printf(result, " ptp pps-delay man cable-delay %d\n",
                                         pps_cl_mode.cable_delay));
        } else if (req->all_defaults) {
            VTSS_RC(vtss_icfg_printf(result, " no ptp pps-delay\n"));
        }
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */
    }
    return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc ptp_icfg_init(void)
{
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_PTP_GLOBAL_CONF, "ptp", ptp_icfg_global_conf));
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_PTP_INTERFACE_CONF, "ptp", ptp_icfg_interface_conf));
    return VTSS_RC_OK;
}
#endif // VTSS_SW_OPTION_ICFG
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
