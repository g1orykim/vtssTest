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
#include "icli_api.h"
#include "icli_porting_util.h"
#include "ptp.h" // For Trace
#include "ptp_api.h"
#include "ptp_cli.h"
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#include "ptp_1pps_sync.h"
#include "ptp_1pps_closed_loop.h"
#endif

#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
#include "zl_3034x_api_pdv_api.h"
#endif

#include "tod_api.h"
#include "misc_api.h"
#include "vtss_ptp_local_clock.h"

/***************************************************************************/
/*  Internal types                                                         */
/***************************************************************************/

/***************************************************************************/
/*  Internal functions                                                     */
/***************************************************************************/


static void ptp_icli_traverse_ports(i32 session_id, int clockinst,
                                    icli_stack_port_range_t *port_type_list_p,
                                    void (*show_function)(i32 session_id, int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr))
{
    u32             range_idx, cnt_idx;
    vtss_isid_t     isid;
    vtss_port_no_t  uport;
    switch_iter_t   sit;
    port_iter_t     pit;
    BOOL            first = TRUE;

    if (port_type_list_p) { //at least one range input
        for (range_idx = 0; range_idx < port_type_list_p->cnt; range_idx++) {
            isid = port_type_list_p->switch_range[range_idx].isid;
            for (cnt_idx = 0; cnt_idx < port_type_list_p->switch_range[range_idx].port_cnt; cnt_idx++) {
                uport = port_type_list_p->switch_range[range_idx].begin_uport + cnt_idx;

                //ignore stacking port
                if (port_isid_port_no_is_stack(isid, uport2iport(uport))) {
                    continue;
                }
                show_function(session_id, clockinst, uport, first, icli_session_self_printf);
                first = FALSE;
            }
        }
    } else { //show all port configuraton
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID_CFG);
        while (switch_iter_getnext(&sit)) {
            (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                show_function(session_id, clockinst, pit.uport, first, icli_session_self_printf);
                first = FALSE;
            }
        }
    }
}


static void ptp_icli_config_traverse_ports(i32 session_id, int clockinst,
                                           icli_stack_port_range_t *port_type_list_p, void *port_cfg,
                                           void (*cfg_function)(i32 session_id, int inst, vtss_port_no_t uport, void *cfg))
{
    u32             range_idx, cnt_idx;
    vtss_isid_t     isid;
    vtss_port_no_t  uport;
    switch_iter_t   sit;
    port_iter_t     pit;

    if (port_type_list_p) { //at least one range input
        for (range_idx = 0; range_idx < port_type_list_p->cnt; range_idx++) {
            isid = port_type_list_p->switch_range[range_idx].isid;
            for (cnt_idx = 0; cnt_idx < port_type_list_p->switch_range[range_idx].port_cnt; cnt_idx++) {
                uport = port_type_list_p->switch_range[range_idx].begin_uport + cnt_idx;

                //ignore stacking port
                if (port_isid_port_no_is_stack(isid, uport2iport(uport))) {
                    continue;
                }
                cfg_function(session_id, clockinst, uport, port_cfg);
            }
        }
    } else { //show all port configuration
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID_CFG);
        while (switch_iter_getnext(&sit)) {
            (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                cfg_function(session_id, clockinst, pit.uport, port_cfg);
            }
        }
    }
}



static void icli_show_clock_port_state_ds(i32 session_id, int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    ptp_show_clock_port_state_ds(inst, uport, first, pr);
}

static void icli_show_clock_port_ds_ds(i32 session_id, int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    ptp_show_clock_port_ds_ds(inst, uport, first, pr);
}

static void icli_show_clock_wireless_ds(i32 session_id, int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    ptp_show_clock_wireless_ds(inst, uport, first, pr);
}

static void icli_show_clock_foreign_master_record_ds(i32 session_id, int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    ptp_show_clock_foreign_master_record_ds(inst, uport, first, pr);
}

static void icli_wireless_mode_enable(i32 session_id, int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    if (!ptp_port_wireless_delay_mode_set(TRUE, uport, inst)) {
        ICLI_PRINTF("Wireless mode not available for ptp instance %d, port %u\n", inst, uport);
    }
}

static void icli_wireless_mode_disable(i32 session_id, int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    if (!ptp_port_wireless_delay_mode_set(FALSE, uport, inst)) {
        ICLI_PRINTF("Wireless mode not available for ptp instance %d, port %u\n", inst, uport);
    }
}

static void icli_wireless_pre_not(i32 session_id, int inst, vtss_port_no_t uport, BOOL first, vtss_ptp_cli_pr *pr)
{
    if (!ptp_port_wireless_delay_pre_notif(uport, inst)) {
        ICLI_PRINTF("Wireless mode not available for ptp instance %d, port %u\n", inst, uport);
    }
}

#if defined(VTSS_FEATURE_PHY_TIMESTAMP) && defined(VTSS_ARCH_JAGUAR_1)
static char *ref_clock_freq_txt(vtss_phy_ts_clockfreq_t freq)
{
    switch (freq) {
        case VTSS_PHY_TS_CLOCK_FREQ_125M:  return "125 MHz ";
        case VTSS_PHY_TS_CLOCK_FREQ_15625M: return "156.25 MHz";
        case VTSS_PHY_TS_CLOCK_FREQ_250M: return "250 MHz";
        default: return "INVALID";
    }
}

void ptp_icli_ptp_ref_clock_set(i32 session_id, BOOL has_mhz125, BOOL has_mhz156p25, BOOL has_mhz250)
{
    vtss_phy_ts_clockfreq_t freq;

    if (!tod_ref_clock_freg_get(&freq)) {
        ICLI_PRINTF("Error getting ref clock frequency\n");
    }
    freq = has_mhz125 ? VTSS_PHY_TS_CLOCK_FREQ_125M :
                has_mhz156p25 ? VTSS_PHY_TS_CLOCK_FREQ_15625M : 
                has_mhz250 ? VTSS_PHY_TS_CLOCK_FREQ_250M : VTSS_PHY_TS_CLOCK_FREQ_250M;
    if (!tod_ref_clock_freg_set(&freq)) {
        ICLI_PRINTF("Error setting ref clock frequency\n");
    } else {
        ICLI_PRINTF("System ref clock frequency (%s)\n", ref_clock_freq_txt(freq));
    }

}
#endif /*VTSS_FEATURE_PHY_TIMESTAMP */



/***************************************************************************/
/*  Functions called by iCLI                                               */
/***************************************************************************/

//  see ptp_icli_functions.h
void ptp_icli_show(i32 session_id, int clockinst, BOOL has_default, BOOL has_current, BOOL has_parent, BOOL has_time_property,
                   BOOL has_filter, BOOL has_servo, BOOL has_clk, BOOL has_ho, BOOL has_uni, BOOL has_master_table_unicast,
                   BOOL has_slave, BOOL has_port_state, BOOL has_port_ds, BOOL has_wireless, BOOL has_foreign_master_record,
                   BOOL has_interface, icli_stack_port_range_t *port_type_list_p)
{
    if (has_default) {
        ptp_show_clock_default_ds(clockinst, icli_session_self_printf);
    }
    if (has_current) {
        ptp_show_clock_current_ds(clockinst, icli_session_self_printf);
    }
    if (has_parent) {
        ptp_show_clock_parent_ds(clockinst, icli_session_self_printf);
    }
    if (has_time_property) {
        ptp_show_clock_time_property_ds(clockinst, icli_session_self_printf);
    }
    if (has_filter) {
        ptp_show_clock_filter_ds(clockinst, icli_session_self_printf);
    }
    if (has_servo) {
        ptp_show_clock_servo_ds(clockinst, icli_session_self_printf);
    }
    if (has_clk) {
        ptp_show_clock_clk_ds(clockinst, icli_session_self_printf);
    }
    if (has_ho) {
        ptp_show_clock_ho_ds(clockinst, icli_session_self_printf);
    }
    if (has_uni) {
        ptp_show_clock_uni_ds(clockinst, icli_session_self_printf);
    }
    if (has_master_table_unicast) {
        ptp_show_clock_master_table_unicast_ds(clockinst, icli_session_self_printf);
    }
    if (has_slave) {
        ptp_show_clock_slave_ds(clockinst, icli_session_self_printf);
    }
    if (has_port_state) {
        ptp_icli_traverse_ports(session_id, clockinst, port_type_list_p, icli_show_clock_port_state_ds);
    }
    if (has_port_ds) {
        ptp_icli_traverse_ports(session_id, clockinst, port_type_list_p, icli_show_clock_port_ds_ds);
    }
    if (has_wireless) {
        ptp_icli_traverse_ports(session_id, clockinst, port_type_list_p, icli_show_clock_wireless_ds);
    }
    if (has_foreign_master_record) {
        ptp_icli_traverse_ports(session_id, clockinst, port_type_list_p, icli_show_clock_foreign_master_record_ds);
    }

}

void ptp_icli_ext_clock_mode_show(i32 session_id)
{
    ptp_show_ext_clock_mode(icli_session_self_printf);
}

#if defined(VTSS_ARCH_SERVAL)
void ptp_icli_rs422_clock_mode_show(i32 session_id)
{
    ptp_show_rs422_clock_mode(icli_session_self_printf);
}
#endif

void ptp_icli_local_clock_set(i32 session_id, int clockinst, BOOL has_update, BOOL has_ratio, i32 ratio)
{
    vtss_timestamp_t t;
    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "clockinst %d has_update %d has_ratio %d ratio  %d", clockinst, has_update, has_ratio, ratio);

    if (has_update) {
        /* update the local clock to the system clock */
        t.sec_msb = 0;
        t.seconds = time(NULL);
        t.nanoseconds = 0;
        vtss_local_clock_time_set(&t,clockinst);
    }
    if (has_ratio) {
        /* set the local clock master ratio */
        if (ratio == 0) {
            vtss_local_clock_ratio_clear(clockinst);
        } else {
            vtss_local_clock_ratio_set(ratio, clockinst);
        }
    }
}

void ptp_icli_local_clock_show(i32 session_id, int clockinst)
{
    ptp_show_local_clock(clockinst, icli_session_self_printf);
}

void ptp_icli_slave_cfg_set(i32 session_id, int clockinst, BOOL has_stable_offset, u32 stable_offset, BOOL has_offset_ok, u32 offset_ok, BOOL has_offset_fail, u32 offset_fail)
{
    ptp_clock_slave_cfg_t slave_cfg;

    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "clockinst %d, has_stable_offset %d, stable_offset %d, has_offset_ok %d, offset_ok %d, has_offset_fail %d, offset_fail %d", clockinst, has_stable_offset, stable_offset, has_offset_ok, offset_ok, has_offset_fail, offset_fail);
    if (ptp_get_clock_slave_config(&slave_cfg,clockinst)) {
        if (has_stable_offset) slave_cfg.stable_offset = stable_offset;
        if (has_offset_ok) slave_cfg.offset_ok = offset_ok;
        if (has_offset_fail) slave_cfg.offset_fail = offset_fail;
        if (!ptp_set_clock_slave_config(&slave_cfg,clockinst)) {
            ICLI_PRINTF("Failed setting slave-cfg\n");
        }
    } else {
        ICLI_PRINTF("Clock instance %d : does not exist\n", clockinst);
    }
}

void ptp_icli_slave_cfg_show(i32 session_id, int clockinst)
{
    ptp_show_slave_cfg(clockinst, icli_session_self_printf);
}

void ptp_icli_slave_table_unicast_show(i32 session_id, int clockinst)
{
    ptp_show_clock_master_table_unicast_ds(clockinst, icli_session_self_printf);
}


//  see ptp_icli_function
void ptp_icli_wireless_mode_set(i32 session_id, int clockinst, BOOL enable, icli_stack_port_range_t *port_type_list_p)
{
    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "Wireless mode set clockinst %d, enable %d", clockinst, enable);
    if (enable) {
        ptp_icli_traverse_ports(session_id, clockinst, port_type_list_p, icli_wireless_mode_enable);
    } else {
        ptp_icli_traverse_ports(session_id, clockinst, port_type_list_p, icli_wireless_mode_disable);
    }

    //VTSS_ICLI_ERR_PRINT(1234);
}

void ptp_icli_wireless_pre_notification(i32 session_id, int clockinst, icli_stack_port_range_t *port_type_list_p)
{
    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "Wireless mode pre clockinst %d", clockinst);
    ptp_icli_traverse_ports(session_id, clockinst, port_type_list_p, icli_wireless_pre_not);
}

static void my_port_wireless_delay_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    vtss_ptp_delay_cfg_t *my_cfg = cfg;

    if (!ptp_port_wireless_delay_set(my_cfg, uport, inst)) {
        ICLI_PRINTF("Wireless mode not available for ptp instance %d, port %u\n",
                inst, uport);
    }
}

void ptp_icli_wireless_delay(i32 session_id, int clockinst, i32 base_delay, i32 incr_delay, icli_stack_port_range_t *v_port_type_list)
{
    vtss_ptp_delay_cfg_t delay_cfg;
    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "clockinst %d, base_delay %d, incr_delay %d", clockinst, base_delay, incr_delay);
    delay_cfg.base_delay = ((vtss_timeinterval_t)base_delay)*0x10000/1000;
    delay_cfg.incr_delay = ((vtss_timeinterval_t)incr_delay)*0x10000/1000;
    ptp_icli_config_traverse_ports(session_id, clockinst, v_port_type_list, &delay_cfg, my_port_wireless_delay_set);
}

void ptp_icli_mode(i32 session_id, int clockinst,
                   BOOL has_boundary, BOOL has_e2etransparent, BOOL has_p2ptransparent, BOOL has_master, BOOL has_slave,
                   BOOL has_onestep, BOOL has_twostep,
                   BOOL has_ethernet, BOOL has_ip4multi, BOOL has_ip4unicast, BOOL has_oam, BOOL has_onepps,
                   BOOL has_oneway, BOOL has_twoway,
                   BOOL has_id, icli_clock_id_t *v_clock_id,
                   BOOL has_vid, u32 vid, u32 prio, BOOL has_tag, BOOL has_mep, u32 mep_id)
{
    ptp_init_clock_ds_t init_ds;
    ptp_clock_default_ds_t bs;
    if (ptp_get_clock_default_ds(&bs,clockinst)) {
        ICLI_PRINTF("Cannot create clock instance %d : a %s clock type already exists\n",
                clockinst,
                DeviceTypeToString(bs.deviceType));
    } else {
        ptp_get_default_clock_default_ds(&bs);
        init_ds.deviceType = has_boundary ? PTP_DEVICE_ORD_BOUND : has_e2etransparent ? PTP_DEVICE_E2E_TRANSPARENT :
                             has_p2ptransparent ? PTP_DEVICE_P2P_TRANSPARENT : has_master ? PTP_DEVICE_MASTER_ONLY :
                             has_slave ? PTP_DEVICE_SLAVE_ONLY : PTP_DEVICE_NONE;
#if !defined(VTSS_ARCH_SERVAL)
        if (has_oam) {
            ICLI_PRINTF("OAM encapsulation is only possible in Serval\n");
            has_oam = FALSE;
        }
#endif
        init_ds.twoStepFlag = has_onestep ? FALSE : has_twostep ? TRUE : bs.twoStepFlag;
        if (has_id) {
            memcpy(init_ds.clockIdentity, v_clock_id, sizeof(init_ds.clockIdentity));
        } else {
            bs.clockIdentity[7] += clockinst;
            memcpy(init_ds.clockIdentity, bs.clockIdentity, sizeof(init_ds.clockIdentity));
        }
        init_ds.oneWay = has_oneway ? TRUE : has_twoway ? FALSE : bs.oneWay;
        init_ds.protocol = has_ethernet ? PTP_PROTOCOL_ETHERNET : has_ip4multi ? PTP_PROTOCOL_IP4MULTI :
                           has_ip4unicast ? PTP_PROTOCOL_IP4UNI : has_oam ? PTP_PROTOCOL_OAM :
                           has_onepps ? PTP_PROTOCOL_1PPS : PTP_PROTOCOL_ETHERNET;
        init_ds.tagging_enable = has_tag;
        init_ds.configured_vid = has_vid ? vid : bs.configured_vid;
        init_ds.configured_pcp = bs.configured_pcp;
        init_ds.mep_instance = has_mep ? mep_id : bs.mep_instance;
        T_IG(VTSS_TRACE_GRP_PTP_ICLI, "clockinst %d, device_type %s", clockinst, DeviceTypeToString(init_ds.deviceType));
        if (!ptp_clock_create(&init_ds,clockinst)) {
            ICLI_PRINTF("Cannot create clock: tried to create more than one transparent clock!\n");
        }
    }
}

void ptp_icli_no_mode(i32 session_id, int clockinst,
                      BOOL has_boundary, BOOL has_e2etransparent, BOOL has_p2ptransparent, BOOL has_master, BOOL has_slave)
{
    ptp_clock_default_ds_t bs;
    u8 device_type = has_boundary ? PTP_DEVICE_ORD_BOUND : has_e2etransparent ? PTP_DEVICE_E2E_TRANSPARENT :
                         has_p2ptransparent ? PTP_DEVICE_P2P_TRANSPARENT : has_master ? PTP_DEVICE_MASTER_ONLY :
                         has_slave ? PTP_DEVICE_SLAVE_ONLY : PTP_DEVICE_NONE;
    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "clockinst %d, device_type %s", clockinst, DeviceTypeToString(device_type));
    if (ptp_get_clock_default_ds(&bs,clockinst)) {
        if (bs.deviceType == device_type) {
            if (!ptp_clock_delete(clockinst)) {
                ICLI_PRINTF("Invalid Clock instance %d\n", clockinst);
            }
        } else {
            ICLI_PRINTF("Cannot delete clock instance %d : it is not a %s clock type\n", clockinst, DeviceTypeToString(device_type));
        }
    } else {
        ICLI_PRINTF("Clock instance %d : does not exist\n", clockinst);
    }
}

//  see ptp_icli_functions.h
void ptp_icli_priority1_set(i32 session_id, int clockinst, u8 priority1)
{
    ptp_set_clock_ds_t default_ds;
    ptp_set_clock_ds_t bs;

    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "Priority1 set clockinst %d, priority1 %d", clockinst, priority1);
    if (ptp_get_clock_set_default_ds(&bs,clockinst)) {
        default_ds.priority1 = priority1;
        default_ds.priority2 = bs.priority2;
        default_ds.domainNumber = bs.domainNumber;

        if (!ptp_set_clock_default_ds(&default_ds,clockinst)) {
            T_IG(VTSS_TRACE_GRP_PTP_ICLI, "Clock instance %d : does not exist",clockinst);
        }
    }
}

void ptp_icli_priority2_set(i32 session_id, int clockinst, u8 priority2)
{
    ptp_set_clock_ds_t default_ds;
    ptp_set_clock_ds_t bs;

    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "Priority2 set clockinst %d, priority2 %d", clockinst, priority2);
    if (ptp_get_clock_set_default_ds(&bs,clockinst)) {
        default_ds.priority1 = bs.priority1;
        default_ds.priority2 = priority2;
        default_ds.domainNumber = bs.domainNumber;

        if (!ptp_set_clock_default_ds(&default_ds,clockinst)) {
            T_IG(VTSS_TRACE_GRP_PTP_ICLI, "Clock instance %d : does not exist",clockinst);
        }
    }
}

void ptp_icli_domain_set(i32 session_id, int clockinst, u8 domain)
{
    ptp_set_clock_ds_t default_ds;
    ptp_set_clock_ds_t bs;

    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "Domain set clockinst %d, domain %d", clockinst, domain);
    if (ptp_get_clock_set_default_ds(&bs,clockinst)) {
        default_ds.priority1 = bs.priority1;
        default_ds.priority2 = bs.priority2;
        default_ds.domainNumber = domain;

        if (!ptp_set_clock_default_ds(&default_ds,clockinst)) {
            T_IG(VTSS_TRACE_GRP_PTP_ICLI, "Clock instance %d : does not exist",clockinst);
        }
    }
}

void ptp_icli_time_property_set(i32 session_id, int clockinst, BOOL has_utc_offset, i32 utc_offset, BOOL has_valid,
                                BOOL has_leapminus_59, BOOL has_leapminus_61, BOOL has_time_traceable,
                                BOOL has_freq_traceable, BOOL has_ptptimescale,
                                BOOL has_time_source, u8 time_source)
{
    ptp_clock_timeproperties_ds_t prop;
    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "icli timeproperty instance %d", clockinst);
    ptp_get_clock_default_timeproperties_ds(&prop);
    if (has_utc_offset)  prop.currentUtcOffset = utc_offset;
    prop.currentUtcOffsetValid = has_valid;
    prop.leap59 = has_leapminus_59;
    prop.leap61 = has_leapminus_61;
    prop.timeTraceable = has_time_traceable;
    prop.frequencyTraceable = has_freq_traceable;
    prop.ptpTimescale = has_ptptimescale;
    if (has_time_source) prop.timeSource = time_source;
    if (!ptp_set_clock_timeproperties_ds(&prop,clockinst)) {
        ICLI_PRINTF("Error writing timing properties instance %d\n", clockinst);
    }
}

void ptp_icli_filter_set(i32 session_id, int clockinst, BOOL has_delay, u32 delay, BOOL has_period, u32 period, BOOL has_dist, u32 dist)
{
    vtss_ptp_default_filter_config_t default_offset;
    vtss_ptp_default_delay_filter_config_t default_delay;

    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "icli filter instance %d ", clockinst);
    ptp_default_filter_default_parameters_get(&default_offset);
    ptp_default_delay_filter_default_parameters_get(&default_delay);
    if (has_delay) default_delay.delay_filter = delay;
    if (has_period) default_offset.period = period;
    if (has_dist) default_offset.dist = dist;
    if (!ptp_default_filter_parameters_set(&default_offset, clockinst) ||
            !ptp_default_delay_filter_parameters_set(&default_delay, clockinst)) {
        ICLI_PRINTF("Error writing filter parameters, instance %d\n", clockinst);
    }
}

void ptp_icli_filter_default_set(i32 session_id, int clockinst)
{
    vtss_ptp_default_filter_config_t default_offset;
    vtss_ptp_default_delay_filter_config_t default_delay;

    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "icli filter default, instance %d ", clockinst);
    ptp_default_filter_default_parameters_get(&default_offset);
    ptp_default_delay_filter_default_parameters_get(&default_delay);
    if (!ptp_default_filter_parameters_set(&default_offset, clockinst) ||
            !ptp_default_delay_filter_parameters_set(&default_delay, clockinst)) {
        ICLI_PRINTF("Error writing filter parameters, instance %d\n", clockinst);
    }
}

void ptp_icli_servo_displaystate_set(i32 session_id, int clockinst, BOOL enable)
{
    vtss_ptp_default_servo_config_t default_servo;

    if (ptp_default_servo_parameters_get(&default_servo, clockinst)) {
        default_servo.display_stats = enable;
        if (!ptp_default_servo_parameters_set(&default_servo,clockinst)) {
            ICLI_PRINTF("Error writing servo parameters, instance %d\n", clockinst);
        }
    } else {
        ICLI_PRINTF("Clock instance %d : does not exist\n", clockinst);
    }
}

void ptp_icli_servo_ap_set(i32 session_id, int clockinst, BOOL enable, u32 ap)
{
    vtss_ptp_default_servo_config_t default_servo;

    if (ptp_default_servo_parameters_get(&default_servo, clockinst)) {
        default_servo.ap = ap;
        default_servo.p_reg = enable;
        if (!ptp_default_servo_parameters_set(&default_servo,clockinst)) {
            ICLI_PRINTF("Error writing servo parameters, instance %d\n", clockinst);
        }
    } else {
        ICLI_PRINTF("Clock instance %d : does not exist\n", clockinst);
    }
}

void ptp_icli_servo_ai_set(i32 session_id, int clockinst, BOOL enable, u32 ai)
{
    vtss_ptp_default_servo_config_t default_servo;

    if (ptp_default_servo_parameters_get(&default_servo, clockinst)) {
        default_servo.ai = ai;
        default_servo.i_reg = enable;
        if (!ptp_default_servo_parameters_set(&default_servo,clockinst)) {
            ICLI_PRINTF("Error writing servo parameters, instance %d\n", clockinst);
        }
    } else {
        ICLI_PRINTF("Clock instance %d : does not exist\n", clockinst);
    }
}

void ptp_icli_servo_ad_set(i32 session_id, int clockinst, BOOL enable, u32 ad)
{
    vtss_ptp_default_servo_config_t default_servo;

    if (ptp_default_servo_parameters_get(&default_servo, clockinst)) {
        default_servo.ad = ad;
        default_servo.d_reg = enable;
        if (!ptp_default_servo_parameters_set(&default_servo,clockinst)) {
            ICLI_PRINTF("Error writing servo parameters, instance %d\n", clockinst);
        }
    } else {
        ICLI_PRINTF("Clock instance %d : does not exist\n", clockinst);
    }
}

void ptp_icli_clock_servo_options_set(i32 session_id, int clockinst, BOOL synce, u32 threshold, u32 ap)
{
    vtss_ptp_default_servo_config_t default_servo;
    if (ptp_default_servo_parameters_get(&default_servo, clockinst)) {
        default_servo.srv_option = synce;
        default_servo.synce_threshold = threshold;
        default_servo.synce_ap = ap;
        if (!ptp_default_servo_parameters_set(&default_servo,clockinst)) {
            ICLI_PRINTF("Error writing servo parameters, instance %d\n", clockinst);
        }
    } else {
        ICLI_PRINTF("Clock instance %d : does not exist\n", clockinst);
    }
}

void ptp_icli_clock_slave_holdover_set(i32 session_id, int clockinst, BOOL has_filter, u32 ho_filter, BOOL has_adj_threshold, u32 adj_threshold)
{
    vtss_ptp_default_servo_config_t default_servo;
    vtss_ptp_default_servo_config_t default_servo_default;
    ptp_default_servo_default_parameters_get(&default_servo_default);
    if (ptp_default_servo_parameters_get(&default_servo, clockinst)) {
        default_servo.ho_filter = has_filter ? ho_filter : default_servo_default.ho_filter;
        default_servo.stable_adj_threshold = has_adj_threshold ? adj_threshold : default_servo_default.stable_adj_threshold;

        if (!ptp_default_servo_parameters_set(&default_servo,clockinst)) {
            ICLI_PRINTF("Error writing servo parameters, instance %d\n", clockinst);
        }
    } else {
        ICLI_PRINTF("Clock instance %d : does not exist\n", clockinst);
    }
}

void ptp_icli_clock_unicast_conf_set(i32 session_id, int clockinst, int idx, BOOL has_duration, u32 duration, u32 ip)
{
    vtss_ptp_unicast_slave_config_t uni_slave;
    uni_slave.duration = has_duration ? duration : 100;
    uni_slave.ip_addr = ip;
    if (!ptp_uni_slave_conf_set(&uni_slave, idx, clockinst)) {
        ICLI_PRINTF("Clock instance %d : does not exist\n", clockinst);
    }
}

void ptp_icli_ext_clock_set(i32 session_id, BOOL has_output, BOOL has_input, BOOL has_ext, u32 clockfreq, BOOL has_vcxo)
{
    vtss_ptp_ext_clock_mode_t mode;

    /* update the local clock to the system clock */
    mode.one_pps_mode = has_output ? VTSS_PTP_ONE_PPS_OUTPUT : has_input ? VTSS_PTP_ONE_PPS_INPUT : VTSS_PTP_ONE_PPS_DISABLE;
    mode.clock_out_enable = has_ext;
    mode.vcxo_enable = has_vcxo;
    mode.freq = clockfreq;
    if (!clockfreq) {
        mode.freq = 1;
    }
#if defined(VTSS_ARCH_LUTON26)
    if (mode.one_pps_mode != VTSS_PTP_ONE_PPS_DISABLE && mode.clock_out_enable) {
        ICLI_PRINTF("One_pps_mode overrules clock_out_enable, i.e. clock_out_enable is set to false\n");
        mode.clock_out_enable = FALSE;
    }
#endif
#if defined(VTSS_ARCH_SERVAL)
    if ((mode.one_pps_mode == VTSS_PTP_ONE_PPS_OUTPUT) && mode.clock_out_enable) {
        ICLI_PRINTF("One_pps_output overrules clock_out_enable, i.e. clock_out_enable is set to false\n");
        mode.clock_out_enable = FALSE;
    }
#endif
    vtss_ext_clock_out_set(&mode);
}

#if defined(VTSS_ARCH_SERVAL)
void ptp_icli_rs422_clock_set(i32 session_id, BOOL has_main_auto, BOOL has_main_man, BOOL has_sub, BOOL has_pps_delay, u32 pps_delay, BOOL has_ser, BOOL has_pim, u32 pim_port)
{
    vtss_ptp_rs422_conf_t mode;

    /* update the local clock to the system clock */
    mode.mode = has_main_auto ? VTSS_PTP_RS422_MAIN_AUTO : has_main_man ? VTSS_PTP_RS422_MAIN_MAN : has_sub ? VTSS_PTP_RS422_SUB : VTSS_PTP_RS422_DISABLE;
    if (has_pps_delay) mode.delay = pps_delay;
    mode.proto = has_ser ? VTSS_PTP_RS422_PROTOCOL_SER : VTSS_PTP_RS422_PROTOCOL_PIM;
    mode.port  = pim_port;
    vtss_ext_clock_rs422_conf_set(&mode);
}
#endif /* VTSS_ARCH_SERVAL */

void ptp_icli_debug_log_mode_set(i32 session_id, int clockinst, u32 debug_mode)
{
    if (!ptp_debug_mode_set(debug_mode, clockinst)) {
        ICLI_PRINTF("Clock instance %d : does not exist\n", clockinst);
    }
}

typedef struct ptp_icli_port_state_s {
    BOOL enable;
    BOOL internal;
} ptp_icli_port_state_t;

static void my_port_state_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    ptp_icli_port_state_t  *my_cfg = cfg;
    vtss_rc rc;
    if (my_cfg->enable || my_cfg->internal) {
        if ((rc = ptp_port_ena(my_cfg->internal, uport,inst)) != VTSS_RC_OK) {
            ICLI_PRINTF("Error enabling inst %d port %d (%s)\n",inst, uport, error_txt(rc));
        }
    } else {
        if (!ptp_port_dis(uport,inst)) {
            ICLI_PRINTF("Error disabling inst %d port %d\n",inst, uport);
        }
    }
}

void ptp_icli_port_state_set(i32 session_id, int clockinst, BOOL enable, BOOL has_internal, icli_stack_port_range_t *v_port_type_list)
{
    ptp_icli_port_state_t port_cfg;
    port_cfg.enable = enable;
    port_cfg.internal = has_internal;
    ptp_icli_config_traverse_ports(session_id, clockinst, v_port_type_list, &port_cfg, my_port_state_set);

}

typedef struct ptp_icli_announce_state_s {
    BOOL has_interval;
    i8 interval;
    BOOL has_timeout;
    u8 timeout;
} ptp_icli_announce_state_t;

static void my_port_ann_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    ptp_icli_announce_state_t *my_cfg = cfg;
    ptp_set_port_ds_t ds_cfg;

    if (ptp_get_port_cfg_ds(uport, &ds_cfg, inst)) {
        if (my_cfg->has_interval) ds_cfg.logAnnounceInterval = my_cfg->interval;
        if (my_cfg->has_timeout) ds_cfg.announceReceiptTimeout = my_cfg->timeout;
        if (!ptp_set_port_ds(uport,&ds_cfg,inst)) {
            ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
        }
    } else {
        ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
    }
}

void ptp_icli_port_announce_set(i32 session_id, int clockinst, BOOL has_interval, i8 interval, BOOL has_timeout, u8 timeout, icli_stack_port_range_t *v_port_type_list)
{
    ptp_icli_announce_state_t ann_cfg;
    ann_cfg.has_interval = has_interval;
    ann_cfg.interval = interval;
    ann_cfg.has_timeout = has_timeout;
    ann_cfg.timeout = timeout;
    ptp_icli_config_traverse_ports(session_id, clockinst, v_port_type_list, &ann_cfg, my_port_ann_set);
}

static void my_port_sync_interval_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    i8 *interval = cfg;
    ptp_set_port_ds_t ds_cfg;

    if (ptp_get_port_cfg_ds(uport, &ds_cfg, inst)) {
        ds_cfg.logSyncInterval = *interval;
        if (!ptp_set_port_ds(uport,&ds_cfg,inst)) {
            ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
        }
    } else {
        ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
    }
}

void ptp_icli_port_sync_interval_set(i32 session_id, int clockinst, i8 interval, icli_stack_port_range_t *v_port_type_list)
{
    ptp_icli_config_traverse_ports(session_id, clockinst, v_port_type_list, &interval, my_port_sync_interval_set);
}

static void my_port_delay_mechanism_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    u8 *dly = cfg;
    ptp_set_port_ds_t ds_cfg;

    if (ptp_get_port_cfg_ds(uport, &ds_cfg, inst)) {
        ds_cfg.delayMechanism = *dly;
        if (!ptp_set_port_ds(uport,&ds_cfg,inst)) {
            ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
        }
    } else {
        ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
    }
}

void ptp_icli_port_delay_mechanism_set(i32 session_id, int clockinst, BOOL has_e2e, BOOL has_p2p, icli_stack_port_range_t *v_port_type_list)
{
    u8 dly;
    dly = has_e2e ? DELAY_MECH_E2E : DELAY_MECH_P2P;
    ptp_icli_config_traverse_ports(session_id, clockinst, v_port_type_list, &dly, my_port_delay_mechanism_set);
}

static void my_port_min_pdelay_interval_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    i8 *interval = cfg;
    ptp_set_port_ds_t ds_cfg;

    if (ptp_get_port_cfg_ds(uport, &ds_cfg, inst)) {
        ds_cfg.logMinPdelayReqInterval = *interval;
        if (!ptp_set_port_ds(uport,&ds_cfg,inst)) {
            ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
        }
    } else {
        ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
    }
}

void ptp_icli_port_min_pdelay_interval_set(i32 session_id, int clockinst, i8 interval, icli_stack_port_range_t *v_port_type_list)
{
    ptp_icli_config_traverse_ports(session_id, clockinst, v_port_type_list, &interval, my_port_min_pdelay_interval_set);
}

static void my_port_delay_asymmetry_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    vtss_timeinterval_t *asy = cfg;
    ptp_set_port_ds_t ds_cfg;

    if (ptp_get_port_cfg_ds(uport, &ds_cfg, inst)) {
        ds_cfg.delayAsymmetry = *asy;
        if (!ptp_set_port_ds(uport,&ds_cfg,inst)) {
            ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
        }
    } else {
        ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
    }
}

void ptp_icli_delay_asymmetry_set(i32 session_id, int clockinst, i32 delay_asymmetry, icli_stack_port_range_t *v_port_type_list)
{
    vtss_timeinterval_t asy = ((vtss_timeinterval_t)delay_asymmetry)<<16;
    ptp_icli_config_traverse_ports(session_id, clockinst, v_port_type_list, &asy, my_port_delay_asymmetry_set);
}

static void my_port_ingress_latency_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    vtss_timeinterval_t *igr = cfg;
    ptp_set_port_ds_t ds_cfg;

    if (ptp_get_port_cfg_ds(uport, &ds_cfg, inst)) {
        ds_cfg.ingressLatency = *igr;
        if (!ptp_set_port_ds(uport,&ds_cfg,inst)) {
            ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
        }
    } else {
        ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
    }
}

void ptp_icli_ingress_latency_set(i32 session_id, int clockinst, i32 ingress_latency, icli_stack_port_range_t *v_port_type_list)
{
    vtss_timeinterval_t igr = ((vtss_timeinterval_t)ingress_latency)<<16;
    ptp_icli_config_traverse_ports(session_id, clockinst, v_port_type_list, &igr, my_port_ingress_latency_set);
}

static void my_port_egress_latency_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    vtss_timeinterval_t *egr = cfg;
    ptp_set_port_ds_t ds_cfg;

    if (ptp_get_port_cfg_ds(uport, &ds_cfg, inst)) {
        ds_cfg.egressLatency = *egr;
        if (!ptp_set_port_ds(uport,&ds_cfg,inst)) {
            ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
        }
    } else {
        ICLI_PRINTF("Error setting port data instance %d port %d\n", inst, uport);
    }
}

void ptp_icli_egress_latency_set(i32 session_id, int clockinst, i32 egress_latency, icli_stack_port_range_t *v_port_type_list)
{
    vtss_timeinterval_t egr = ((vtss_timeinterval_t)egress_latency)<<16;
    ptp_icli_config_traverse_ports(session_id, clockinst, v_port_type_list, &egr, my_port_egress_latency_set);
}

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
static void my_port_1pps_mode_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    vtss_1pps_sync_conf_t *pps_conf = cfg;
    vtss_rc rc;

    if (VTSS_RC_OK != (rc = ptp_1pps_sync_mode_set(uport2iport(uport), pps_conf))) {
        ICLI_PRINTF("Error setting 1pps sync for port %d (%s)\n", uport, error_txt(rc));
    }
}

void ptp_icli_port_1pps_mode_set(i32 session_id, BOOL has_main_auto, BOOL has_main_man, BOOL has_sub, BOOL has_pps_phase, i32 pps_phase,
                                 BOOL has_cable_asy, i32 cable_asy, BOOL has_ser_man, BOOL has_ser_auto, icli_stack_port_range_t *v_port_type_list)
{
    vtss_1pps_sync_conf_t pps_conf;
    pps_conf.mode = has_main_auto ? VTSS_PTP_1PPS_SYNC_MAIN_AUTO : has_main_man ? VTSS_PTP_1PPS_SYNC_MAIN_MAN :
                    has_sub ? VTSS_PTP_1PPS_SYNC_SUB : VTSS_PTP_1PPS_SYNC_DISABLE;
    pps_conf.pulse_delay = has_pps_phase ? pps_phase : 0;
    pps_conf.cable_asy = has_cable_asy ? cable_asy : 0;
    pps_conf.serial_tod = has_ser_man ? VTSS_PTP_1PPS_SER_TOD_MAN : has_ser_auto ? VTSS_PTP_1PPS_SER_TOD_AUTO : VTSS_PTP_1PPS_SER_TOD_DISABLE;
    ptp_icli_config_traverse_ports(session_id, 0, v_port_type_list, &pps_conf, my_port_1pps_mode_set);
}


static void my_port_1pps_delay_set(i32 session_id, int inst, vtss_port_no_t uport, void *cfg)
{
    vtss_1pps_closed_loop_conf_t *pps_delay = cfg;
    vtss_rc rc;

    if (VTSS_RC_OK != (rc = ptp_1pps_closed_loop_mode_set(uport2iport(uport), pps_delay))) {
        ICLI_PRINTF("Error setting 1pps delay for port %d (%s)\n", uport, error_txt(rc));
    }
}

void ptp_icli_port_1pps_delay_set(i32 session_id, BOOL has_auto, u32 master_port, BOOL has_man, u32 cable_delay, icli_stack_port_range_t *v_port_type_list)
{
    vtss_1pps_closed_loop_conf_t pps_delay;
    pps_delay.mode = has_auto ? VTSS_PTP_1PPS_CLOSED_LOOP_AUTO : has_man ? VTSS_PTP_1PPS_CLOSED_LOOP_MAN : VTSS_PTP_1PPS_CLOSED_LOOP_DISABLE;
    pps_delay.master_port = has_auto ? master_port : 0;
    pps_delay.cable_delay = has_man ? cable_delay : 0;
    ptp_icli_config_traverse_ports(session_id, 0, v_port_type_list, &pps_delay, my_port_1pps_delay_set);
}
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */

#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
static char *apr_mode_2_txt(u32 value)
{
    switch (value) {
        case 0: return "FREQ_TCXO";
        case 1: return "FREQ_OCXO_S3E";
        case 2: return "BC_PARTIAL_ON_PATH_FREQ";
        case 3: return "BC_PARTIAL_ON_PATH_PHASE"; 
        case 4: return "BC_PARTIAL_ON_PATH_SYNCE";
        case 5: return "BC_FULL_ON_PATH_FREQ";
        case 6: return "BC_FULL_ON_PATH_PHASE";
        case 7: return "BC_FULL_ON_PATH_SYNCE"; 
        case 8: return "FREQ_ACCURACY_FDD";
    }
    return "INVALID";
}
#endif /* VTSS_SW_OPTION_ZL_3034X_PDV */

void ptp_icli_ms_pdv_set(i32 session_id, BOOL has_one_hz, BOOL has_min_phase, u32 min_phase, BOOL has_apr, u32 apr, BOOL enable)
{
#ifdef VTSS_SW_OPTION_ZL_3034X_PDV
    u32 phase = 20, my_apr = 1;
    char zl_config[150];
    T_DG(VTSS_TRACE_GRP_PTP_ICLI, "ms_pdv_set, enable: %d", enable);
    if (enable) {
        if (apr_adj_min_get(&phase) != VTSS_RC_OK) {
            ICLI_PRINTF("APR adjustment freq min phase get failed!\n");
        }
        phase = has_min_phase ? min_phase : phase;
        if (apr_adj_min_set(phase) != VTSS_RC_OK) {
            ICLI_PRINTF("APR adjustment freq min phase setting failed!\n");
        } else {
            ICLI_PRINTF("APR adjustment freq min phase set to %d ns\n", phase);
        }
        if (apr_config_parameters_get(&my_apr, zl_config) != VTSS_RC_OK) {
            ICLI_PRINTF("APR server mode get failed!\n");
        }
        my_apr = has_apr ? apr : my_apr;
        if (apr_config_parameters_set(my_apr) != VTSS_RC_OK) {
            ICLI_PRINTF("APR server mode setting failed!\n");
        } else {
            ICLI_PRINTF("APR server mode set to %s\n", apr_mode_2_txt(my_apr));
        }
        /* if has_one_hz is set, then it overrules the default 1 Hz set by apr_config_parameters_set */
        if (has_one_hz && (apr_server_one_hz_set(has_one_hz) != VTSS_RC_OK)) {
            ICLI_PRINTF("APR server one hz setting failed!\n");
        }
    }
    if (ptp_set_ext_pdv_config(enable) != VTSS_RC_OK) {
        ICLI_PRINTF("APR enabling external PDV failed!\n");
    }
#else
    ICLI_PRINTF("External PDV filter function not defined\n");
#endif /* VTSS_SW_OPTION_ZL_3034X_PDV */    
}

void ptp_icli_tc_internal_set(i32 session_id, BOOL has_mode, u32 mode)
{
    vtss_tod_internal_tc_mode_t my_mode;
    T_IG(VTSS_TRACE_GRP_PTP_ICLI, "tc_internal_set, has_mode: %d, mode %d", has_mode, mode);
    if(has_mode) {
        my_mode = (vtss_tod_internal_tc_mode_t)mode;
    } else {
        my_mode = VTSS_TOD_INTERNAL_TC_MODE_30BIT;
    }
    if(!tod_tc_mode_set(&my_mode)) {
        ICLI_PRINTF("Failed to set the TC internal mode to %d\n", my_mode);
    } else {
        ICLI_PRINTF("\nSuccessfully set the TC internal mode...\n");
        if (tod_ready()) {
			ICLI_PRINTF("Internal TC mode Configuration has been set, you need to reboot to activate the changed conf.\n");
        }
    }

}


/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
