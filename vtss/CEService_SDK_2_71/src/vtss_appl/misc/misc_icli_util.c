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
 
 $Id$
 $Revision$

*/

#include "main.h"
#include "misc_api.h"
#include "icli_api.h"
#include "icli_porting_util.h"
#include "misc_icli_util.h"

void misc_icli_chip(misc_icli_req_t *req)
{
    u32            session_id = req->session_id;
    vtss_chip_no_t chip_no;

    if (req->chip_no == MISC_CHIP_NO_NONE) {
        if (misc_chip_no_get(&chip_no) == VTSS_RC_OK) {
            ICLI_PRINTF("Chip Number: ");
            if (chip_no == VTSS_CHIP_NO_ALL)
                ICLI_PRINTF("All\n");
            else
                ICLI_PRINTF("%u\n", chip_no);
        } else
            ICLI_PRINTF("GET failed");
    } else if (misc_chip_no_set(req->chip_no) != VTSS_OK) {
        ICLI_PRINTF("SET failed");
    }
}

#define MISC_ICLI_SESSION_NONE 0xffffffff

/* Global variable for session ID used by debug api print */
/*lint -esym(459, misc_icli_session_id) */
static u32 misc_icli_session_id = MISC_ICLI_SESSION_NONE;

static void misc_icli_printf(const char *fmt, ...)
{
    u32     session_id = misc_icli_session_id;
    va_list args = NULL;
    char    buf[256];

    /*lint --e{454,455,456} ... We are called within a critical region */
    va_start(args, fmt);
    (void)vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    VTSS_API_EXIT();
    ICLI_PRINTF(buf);
    VTSS_API_ENTER();
    va_end(args);
}

void misc_icli_debug_api(misc_icli_req_t *req)
{
    u32               session_id = req->session_id;
    port_iter_t       pit;
    vtss_debug_info_t *info = &req->debug_info;

    (void)icli_port_iter_init(&pit, VTSS_ISID_START, PORT_ITER_FLAGS_NORMAL);
    while (icli_port_iter_getnext(&pit, req->port_list)) {
        info->port_list[pit.iport] = 1;
    }
    if (misc_chip_no_get(&info->chip_no) == VTSS_RC_OK) {
        if (misc_icli_session_id == MISC_ICLI_SESSION_NONE) {
            misc_icli_session_id = session_id;
            if (vtss_debug_info_print(misc_phy_inst_get(), misc_icli_printf, info) != VTSS_RC_OK) {
                ICLI_PRINTF("debug print failed\n");
            }
            misc_icli_session_id = MISC_ICLI_SESSION_NONE;
        } else {
            ICLI_PRINTF("debug api is already running\n");
        }
    }
}

void misc_icli_suspend_resume(misc_icli_req_t *req)
{
    vtss_init_data_t data;
    
    data.cmd = INIT_CMD_SUSPEND_RESUME;
    data.resume = req->resume;
    (void)init_modules(&data);
}

#ifdef VTSS_FEATURE_10GBASE_KR
static char *slewrate2txt(u32 value)
{
    switch (value) {
        case VTSS_SLEWRATE_25PS:  return "25PS";
        case VTSS_SLEWRATE_35PS:  return "35PS";
        case VTSS_SLEWRATE_55PS:  return "55PS";
        case VTSS_SLEWRATE_70PS:  return "70PS"; 
        case VTSS_SLEWRATE_120PS: return "120PS";
    }
    return "INVALID";
}
#endif /* VTSS_FEATURE_10GBASE_KR */

void misc_icli_10g_kr_conf(u32 session_id, BOOL has_cm1, i32 cm_1, BOOL has_c0, i32 c_0, BOOL has_cp1, i32 c_1, 
                           BOOL has_ampl, u32 amp_val, BOOL has_ps25, BOOL has_ps35, BOOL has_ps55, BOOL has_ps70,
                           BOOL has_ps120, BOOL has_en_ob, BOOL has_dis_ob, BOOL has_ser_inv, BOOL has_ser_no_inv,
                           icli_stack_port_range_t *v_port_type_list)
{
#ifdef VTSS_FEATURE_10GBASE_KR
    vtss_phy_10g_base_kr_conf_t kr_conf;
    u32             range_idx, cnt_idx;
    vtss_isid_t     isid;
    vtss_port_no_t  uport;
    vtss_rc rc;
    float r, v1, v2, v3;
    u32 max_dac;
    
    if (v_port_type_list) { //at least one range input
        for (range_idx = 0; range_idx < v_port_type_list->cnt; range_idx++) {
            isid = v_port_type_list->switch_range[range_idx].isid;
            for (cnt_idx = 0; cnt_idx < v_port_type_list->switch_range[range_idx].port_cnt; cnt_idx++) {
                uport = v_port_type_list->switch_range[range_idx].begin_uport + cnt_idx;
                //ignore stacking port
                if (port_isid_port_no_is_stack(isid, uport2iport(uport))) {
                    continue;
                }
                if (VTSS_RC_OK != (rc = vtss_phy_10g_base_kr_conf_get(misc_phy_inst_get(), uport2iport(uport), &kr_conf))) {
                    ICLI_PRINTF("Error getting KR configuration for port %d (%s)\n", uport, error_txt(rc));
                } else {
                    if (has_cm1) kr_conf.cm1 = cm_1;
                    if (has_c0) kr_conf.c0 = c_0;
                    if (has_cp1) kr_conf.c1 = c_1;
                    if (has_ampl) kr_conf.ampl = amp_val;
                    kr_conf.slewrate = has_ps25 ? VTSS_SLEWRATE_25PS : has_ps35 ? VTSS_SLEWRATE_35PS : has_ps55 ? VTSS_SLEWRATE_55PS : 
                                       has_ps70 ? VTSS_SLEWRATE_70PS : has_ps120 ? VTSS_SLEWRATE_120PS : kr_conf.slewrate;
                    kr_conf.en_ob = has_en_ob ? TRUE : has_dis_ob ? FALSE : kr_conf.en_ob;
                    kr_conf.ser_inv = has_ser_inv ? TRUE : has_ser_no_inv ? FALSE : kr_conf.ser_inv;
                    r = kr_conf.ampl/62.0;
                    v1 = kr_conf.c0 - kr_conf.c1 + kr_conf.cm1;
                    v2 = kr_conf.c0 + kr_conf.c1 + kr_conf.cm1;
                    v3 = kr_conf.c0 + kr_conf.c1 - kr_conf.cm1;
                    max_dac = VTSS_LABS(kr_conf.c0) + VTSS_LABS(kr_conf.c1) + VTSS_LABS(kr_conf.cm1);
                    
                    ICLI_PRINTF("KR configuration for port %d\n", uport);
                    ICLI_PRINTF("----------------------------\n");
                    ICLI_PRINTF("cm1     = %3d\n", kr_conf.cm1);
                    ICLI_PRINTF("c0      = %3d\n", kr_conf.c0);
                    ICLI_PRINTF("cp1     = %3d\n", kr_conf.c1);
                    ICLI_PRINTF("Slewrate= %s\n", slewrate2txt(kr_conf.slewrate));
                    ICLI_PRINTF("Rpre    = %1.3f\n", v3/v2);
                    ICLI_PRINTF("Rpst    = %1.3f\n", v1/v2);
                    ICLI_PRINTF("Ampl    = %4d   mVppd\n", kr_conf.ampl);
                    ICLI_PRINTF("max     = %5.1f mVppd\n", 2.0 * r * max_dac);
                    ICLI_PRINTF("v2      = %5.1f mV\n", r * v2);
                    ICLI_PRINTF("maxdac  = %3u\n", max_dac);
                    ICLI_PRINTF("enable  = %s\n", kr_conf.en_ob ? "TRUE" : "FALSE");
                    ICLI_PRINTF("invert  = %s\n", kr_conf.ser_inv ? "TRUE" : "FALSE");
                    if (VTSS_RC_OK != (rc = vtss_phy_10g_base_kr_conf_set(misc_phy_inst_get(), uport2iport(uport), &kr_conf))) {
                        ICLI_PRINTF("Error setting KR configuration for port %d (%s)\n", uport, error_txt(rc));
                    }
                }
            }
        }
    }
#endif /* VTSS_FEATURE_10GBASE_KR */
    
}

#ifdef VTSS_CHIP_10G_PHY
static char *lb_type2txt(const vtss_phy_10g_loopback_t *lb)
{
    switch (lb->lb_type) {
        case VTSS_LB_NONE:                  return "No looback   ";
        case VTSS_LB_SYSTEM_XS_SHALLOW:     return "System Loopback B,  XAUI -> XS -> XAUI   4x800E.13";
        case VTSS_LB_SYSTEM_XS_DEEP:        return "System Loopback C,  XAUI -> XS -> XAUI   4x800F.2";
        case VTSS_LB_SYSTEM_PCS_SHALLOW:    return "System Loopback E,  XAUI -> PCS FIFO -> XAUI 3x8005.2";
        case VTSS_LB_SYSTEM_PCS_DEEP:       return "System Loopback G,  XAUI -> PCS -> XAUI  3x0000.14";
        case VTSS_LB_SYSTEM_PMA:            return "System Loopback J,  XAUI -> PMA -> XAUI  1x0000.0";
        case VTSS_LB_NETWORK_XS_SHALLOW:    return "Network Loopback D,  XFI -> XS -> XFI   4x800F.1";
        case VTSS_LB_NETWORK_XS_DEEP:       return "Network Loopback A,  XFI -> XS -> XFI   4x0000.1  4x800E.13=0";
        case VTSS_LB_NETWORK_PCS:           return "Network Loopback F,  XFI -> PCS -> XFI  3x8005.3";
        case VTSS_LB_NETWORK_WIS:           return "Network Loopback H,  XFI -> WIS -> XFI  2xE600.0";
        case VTSS_LB_NETWORK_PMA:           return "Network Loopback K,  XFI -> PMA -> XFI  1x8000.8";
            /* Venice specific loopbacks, the Venice implementation is different, and therefore the loopbacks are not exactly the same */
        case VTSS_LB_H2:                    return "Host Loopback 2, 40-bit XAUI-PHY interface Mirror XAUI data";
        case VTSS_LB_H3:                    return "Host Loopback 3, 64-bit PCS after the gearbox FF00 repeating IEEE PCS system loopback";
        case VTSS_LB_H4:                    return "Host Loopback 4, 64-bit WIS FF00 repeating IEEE WIS system loopback";
        case VTSS_LB_H5:                    return "Host Loopback 5, 1-bit SFP+ after SerDes Mirror XAUI data IEEE PMA system loopback";
        case VTSS_LB_H6:                    return "Host Loopback 6, 32-bit XAUI-PHY interface Mirror XAUI data";
        case VTSS_LB_L0:                    return "Line Loopback 0, 4-bit XAUI before SerDes Mirror SFP+ data";
        case VTSS_LB_L1:                    return "Line Loopback 1, 4-bit XAUI after SerDes Mirror SFP+ data IEEE PHY-XS network loopback";
        case VTSS_LB_L2:                    return "Line Loopback 2, 64-bit XGMII after FIFO Mirror SFP+ data";
        case VTSS_LB_L3:                    return "Loopback 3, 64-bit PMA interface Mirror SFP+ data";
    }
    return "INVALID";
}

void misc_icli_10g_phy_loopback(u32 session_id, BOOL has_a, BOOL has_b, BOOL has_c, BOOL has_d, BOOL has_e, BOOL has_f, 
                                BOOL has_g, BOOL has_h, BOOL has_j, BOOL has_k, BOOL has_h2, BOOL has_h3, BOOL has_h4, 
                                BOOL has_h5, BOOL has_h6, BOOL has_l0, BOOL has_l1, BOOL has_l2, BOOL has_l3, 
                                BOOL has_enable, BOOL has_disable, 
                                icli_stack_port_range_t *v_port_type_list)
{
    vtss_phy_10g_loopback_t lb;
    u32             range_idx, cnt_idx;
    vtss_isid_t     isid;
    vtss_port_no_t  uport;
    vtss_rc rc;
    
    if (v_port_type_list) { //at least one range input
        for (range_idx = 0; range_idx < v_port_type_list->cnt; range_idx++) {
            isid = v_port_type_list->switch_range[range_idx].isid;
            for (cnt_idx = 0; cnt_idx < v_port_type_list->switch_range[range_idx].port_cnt; cnt_idx++) {
                uport = v_port_type_list->switch_range[range_idx].begin_uport + cnt_idx;
                //ignore stacking port
                if (port_isid_port_no_is_stack(isid, uport2iport(uport))) {
                    continue;
                }
                if (VTSS_RC_OK != (rc = vtss_phy_10g_loopback_get(misc_phy_inst_get(), uport2iport(uport), &lb))) {
                    ICLI_PRINTF("Error getting loopback configuration for port %d (%s)\n", uport, error_txt(rc));
                } else {
                    lb.enable = has_enable ? TRUE : has_disable ? FALSE : lb.enable;
                    lb.lb_type = has_a ? VTSS_LB_NETWORK_XS_DEEP : 
                                 has_b ? VTSS_LB_SYSTEM_XS_SHALLOW : 
                                 has_c ? VTSS_LB_SYSTEM_XS_DEEP : 
                                 has_d ? VTSS_LB_NETWORK_XS_SHALLOW : 
                                 has_e ? VTSS_LB_SYSTEM_PCS_SHALLOW : 
                                 has_f ? VTSS_LB_NETWORK_PCS : 
                                 has_g ? VTSS_LB_SYSTEM_PCS_DEEP : 
                                 has_h ? VTSS_LB_NETWORK_WIS : 
                                 has_j ? VTSS_LB_SYSTEM_PMA : 
                                 has_k ? VTSS_LB_NETWORK_PMA : 
                                 has_h2 ? VTSS_LB_H2 : 
                                 has_h3 ? VTSS_LB_H3 : 
                                 has_h4 ? VTSS_LB_H4 : 
                                 has_h5 ? VTSS_LB_H5 : 
                                 has_h6 ? VTSS_LB_H6 : 
                                 has_l0 ? VTSS_LB_L0 : 
                                 has_l1 ? VTSS_LB_L1 : 
                                 has_l2 ? VTSS_LB_L2 : 
                                 has_l3 ? VTSS_LB_L3 : 
                                 lb.lb_type;
                    ICLI_PRINTF("Port %d, loopback %s : %s\n", uport, lb.enable ? "enable" : "disable", lb.enable ? lb_type2txt(&lb) : "");
                    if (has_enable || has_disable) {
                        if (VTSS_RC_OK != (rc = vtss_phy_10g_loopback_set(misc_phy_inst_get(), uport2iport(uport), &lb))) {
                            ICLI_PRINTF("Error setting loopback configuration for port %d (%s)\n", uport, error_txt(rc));
                        }
                    }
                }
            }
        }
    }
}
 
#if defined(VTSS_FEATURE_SYNCE_10G) 

static char *rcvrdclk_type2txt(vtss_recvrd_clkout_t r)
{
    switch (r) {
        case VTSS_RECVRD_CLKOUT_DISABLE:            return "Disable";
        case VTSS_RECVRD_CLKOUT_LINE_SIDE_RX_CLOCK: return "Line Side Rx Clock";
        case VTSS_RECVRD_CLKOUT_LINE_SIDE_TX_CLOCK: return "Line Side Tx Clock";
    }
    return "INVALID";
}

void misc_icli_10g_phy_rxckout(u32 session_id, BOOL has_disable, BOOL has_rx_clock, BOOL has_tx_clock, 
                               BOOL has_pcs_fault_squelch, BOOL has_no_pcs_fault_squelch, 
                               BOOL has_lopc_squelch, BOOL has_no_lopc_squelch, 
                               icli_stack_port_range_t *v_port_type_list)
{
    vtss_phy_10g_rxckout_conf_t ckout;
    u32             range_idx, cnt_idx;
    vtss_isid_t     isid;
    vtss_port_no_t  uport;
    vtss_rc rc;
    
    if (v_port_type_list) { //at least one range input
        for (range_idx = 0; range_idx < v_port_type_list->cnt; range_idx++) {
            isid = v_port_type_list->switch_range[range_idx].isid;
            for (cnt_idx = 0; cnt_idx < v_port_type_list->switch_range[range_idx].port_cnt; cnt_idx++) {
                uport = v_port_type_list->switch_range[range_idx].begin_uport + cnt_idx;
                //ignore stacking port
                if (port_isid_port_no_is_stack(isid, uport2iport(uport))) {
                    continue;
                }
                if (VTSS_RC_OK != (rc = vtss_phy_10g_rxckout_get(misc_phy_inst_get(), uport2iport(uport), &ckout))) {
                    ICLI_PRINTF("Error getting rxckout configuration for port %d (%s)\n", uport, error_txt(rc));
                } else {
                    ckout.mode = has_disable ? VTSS_RECVRD_CLKOUT_DISABLE : has_rx_clock ? VTSS_RECVRD_CLKOUT_LINE_SIDE_RX_CLOCK : 
                                 has_tx_clock ? VTSS_RECVRD_CLKOUT_LINE_SIDE_TX_CLOCK :ckout.mode;
                    ckout.squelch_on_pcs_fault = has_pcs_fault_squelch ? TRUE : 
                                 has_no_pcs_fault_squelch ? FALSE : ckout.squelch_on_pcs_fault;
                    ckout.squelch_on_lopc = has_lopc_squelch ? TRUE : 
                                 has_no_lopc_squelch ? FALSE :  ckout.squelch_on_lopc;
                    ICLI_PRINTF("Port %d, rxckout %s, squelch_on_lopc %s, squelch_on_lopc %s\n", uport,
                                rcvrdclk_type2txt(ckout.mode),
                                ckout.squelch_on_pcs_fault ? "enable" : "disable", 
                                ckout.squelch_on_lopc ? "enable" : "disable");
                    if (has_disable || has_rx_clock || has_tx_clock || has_pcs_fault_squelch ||
                        has_no_pcs_fault_squelch || has_lopc_squelch || has_no_lopc_squelch) {
                        if (VTSS_RC_OK != (rc = vtss_phy_10g_rxckout_set(misc_phy_inst_get(), uport2iport(uport), &ckout))) {
                            ICLI_PRINTF("Error setting rxckout configuration for port %d (%s)\n", uport, error_txt(rc));
                        }
                    }
                }
            }
        }
    }
}

void misc_icli_10g_phy_txckout(u32 session_id, BOOL has_disable, BOOL has_rx_clock, BOOL has_tx_clock, 
                               icli_stack_port_range_t *v_port_type_list)
{
    vtss_phy_10g_txckout_conf_t ckout;
    u32             range_idx, cnt_idx;
    vtss_isid_t     isid;
    vtss_port_no_t  uport;
    vtss_rc rc;
    
    if (v_port_type_list) { //at least one range input
        for (range_idx = 0; range_idx < v_port_type_list->cnt; range_idx++) {
            isid = v_port_type_list->switch_range[range_idx].isid;
            for (cnt_idx = 0; cnt_idx < v_port_type_list->switch_range[range_idx].port_cnt; cnt_idx++) {
                uport = v_port_type_list->switch_range[range_idx].begin_uport + cnt_idx;
                //ignore stacking port
                if (port_isid_port_no_is_stack(isid, uport2iport(uport))) {
                    continue;
                }
                if (VTSS_RC_OK != (rc = vtss_phy_10g_txckout_get(misc_phy_inst_get(), uport2iport(uport), &ckout))) {
                    ICLI_PRINTF("Error getting txckout configuration for port %d (%s)\n", uport, error_txt(rc));
                } else {
                    ckout.mode = has_disable ? VTSS_RECVRD_CLKOUT_DISABLE : has_rx_clock ? VTSS_RECVRD_CLKOUT_LINE_SIDE_RX_CLOCK : 
                                 has_tx_clock ? VTSS_RECVRD_CLKOUT_LINE_SIDE_TX_CLOCK :ckout.mode;
                    ICLI_PRINTF("Port %d, txckout %s\n", uport,
                                rcvrdclk_type2txt(ckout.mode));
                    if (has_disable || has_rx_clock || has_tx_clock) {
                        if (VTSS_RC_OK != (rc = vtss_phy_10g_txckout_set(misc_phy_inst_get(), uport2iport(uport), &ckout))) {
                            ICLI_PRINTF("Error setting txckout configuration for port %d (%s)\n", uport, error_txt(rc));
                        }
                    }
                }
            }
        }
    }
}

#endif // VTSS_FEATURE_SYNCE_10G

#endif // VTSS_CHIP_10G_PHY
