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
#include "conf_api.h"
#include "port_api.h"
#include "ptp_api.h"
#include "vtss_ptp_types.h"
#include "vtss_ptp_local_clock.h"
#include "vtss_tod_api.h"

#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */
static char *ptp_one_pps_mode_disp(u8 m)
{
    switch (m) {
        case VTSS_PTP_ONE_PPS_DISABLE:
            return "Disable";
        case VTSS_PTP_ONE_PPS_OUTPUT:
            return "Output";
        case VTSS_PTP_ONE_PPS_INPUT:
            return "Input";
        default:
            return "unknown";
    }
}

static BOOL ptp_get_one_pps_mode_from_str(char *one_pps_mode_str, u8 *one_pps_mode)
{
    if(!strcmp(one_pps_mode_str, "Disable")) {
        *one_pps_mode = VTSS_PTP_ONE_PPS_DISABLE;
        return TRUE;
    }
    if(!strcmp(one_pps_mode_str,"Output")) {
        *one_pps_mode = VTSS_PTP_ONE_PPS_OUTPUT;
        return TRUE;
    }
    if(!strcmp(one_pps_mode_str,"Input")) {
        *one_pps_mode = VTSS_PTP_ONE_PPS_INPUT;
        return TRUE;
    }
    return FALSE;
}    

static char *ptp_bool_disp(BOOL b)
{
    return (b ? "True" : "False");
}

static int ptp_get_bool_value(char *bool_var)
{
    if (!strcmp(bool_var,"True"))
        return TRUE;
    else
        return FALSE;
}

static char *ptp_state_disp(u8 b)
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

static char *ptp_protocol_disp(u8 p)
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
            return "OnePPS";
        default:
            return "?";
    }
}


static int ptp_get_protocol(char *prot)
{
    if (!strcmp(prot,"Ethernet")) {
        return PTP_PROTOCOL_ETHERNET;
    } else if (!strcmp(prot,"IPv4Multi")) {
        return PTP_PROTOCOL_IP4MULTI;
    } else if (!strcmp(prot,"IPv4Uni")) {
        return PTP_PROTOCOL_IP4UNI;
    }
    return PTP_PROTOCOL_ETHERNET;
}

static char *ptp_delaymechanism_disp(uchar d)
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

static BOOL ptp_get_delaymechanism_type(char *dmStr,u8 *mechType)
{
    BOOL rc = FALSE;
    if ( 0 == strcmp(dmStr, "e2e")) {
        *mechType = 1;
        rc = TRUE;
    } else if ( 0 == strcmp(dmStr, "p2p")) {
        *mechType = 2;
        rc =  TRUE;
    }
    return rc;
}

static const char * const clock_adjustment_method_txt [] = {
    "Internal Timer",
    "VCXO/(VC)OCXO option",
    "DAC option",
    "Software"
};

static BOOL ptp_get_devtype_from_str(char * deviceTypeStr, u8 *devType)
{
    BOOL rc = FALSE;
    *devType = PTP_DEVICE_NONE;
    if (0 == strcmp(deviceTypeStr,"Inactive")) {
        *devType = PTP_DEVICE_NONE;
        rc = TRUE;
    } else if (0 == strcmp(deviceTypeStr,"Ord-Bound")) {
        *devType = PTP_DEVICE_ORD_BOUND;
        rc = TRUE;
    } else if (0 == strcmp(deviceTypeStr,"P2pTransp")) {
        *devType = PTP_DEVICE_P2P_TRANSPARENT;
        rc = TRUE;
    } else if (0 == strcmp(deviceTypeStr,"E2eTransp")) {
        *devType = PTP_DEVICE_E2E_TRANSPARENT;
        rc = TRUE;
    } else if (0 == strcmp(deviceTypeStr,"Mastronly")) {
        *devType = PTP_DEVICE_MASTER_ONLY;
        rc = TRUE;
    } else if (0 == strcmp(deviceTypeStr,"Slaveonly")) {
        *devType = PTP_DEVICE_SLAVE_ONLY;
        rc = TRUE;
    }

    return rc;
}


/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

static cyg_int32 handler_config_ptp(CYG_HTTPD_STATE* p)
{
    i32    ct;
    u8     dev_type;
    u8     one_pps_mode;
    int    var_int;
    ulong  var_value;
    mac_addr_t my_sysmac;
    uint inst, clock_identity[8];
    uint new_clock_inst;
    ptp_clock_default_ds_t clock_bs;
    ptp_init_clock_ds_t clock_init_bs;
    ptp_port_ds_t port_bs;
    vtss_ptp_ext_clock_mode_t mode;
    uint port_no;
    BOOL dataSet = FALSE;
    const i8 *var_string;
    size_t len = 0;
    i8 *str;
    i8 search_str[32];
    i32 i = 0;
    i8 str1 [40];
    i8 str2 [40];
    port_iter_t       pit;
    vtss_rc rc;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PTP))
        return -1;
#endif

    memset(&clock_init_bs,0,sizeof(clock_init_bs));


    memset(&clock_bs,0,sizeof(clock_bs));
    memset(&port_bs,0,sizeof(port_bs));
    if (p->method == CYG_HTTPD_METHOD_POST) {
        /* show the local clock  */
        vtss_ext_clock_out_get(&mode);
        var_string = cyg_httpd_form_varable_string(p, "one_pps_mode", &len);
        if (len > 0) {
           (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
           /* Change the Device Type of Clock Instance */
           if (ptp_get_one_pps_mode_from_str(str2, &one_pps_mode)) {
               mode.one_pps_mode = one_pps_mode;
           }
        }
        var_string = cyg_httpd_form_varable_string(p, "external_enable", &len);
        if (len > 0) {
           (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
           mode.clock_out_enable = ptp_get_bool_value(str2);
        }
        var_value = 0;
        if (cyg_httpd_form_varable_long_int(p, "clock_freq", &var_value)) {
            mode.freq = var_value;
        }

        var_string = cyg_httpd_form_varable_string(p, "vcxo_enable", &len);
        if (len > 0) {
           (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
           mode.vcxo_enable = ptp_get_bool_value(str2);
        }

#if defined(VTSS_ARCH_LUTON26)
        if (mode.one_pps_mode != VTSS_PTP_ONE_PPS_DISABLE && mode.clock_out_enable) {
            mode.clock_out_enable = FALSE;
        }
#endif
        if (mode.clock_out_enable && !mode.freq) {
            mode.freq = 1;
        }

        vtss_ext_clock_out_set(&mode);


        for (inst = 0; inst < PTP_CLOCK_INSTANCES; inst++)
        {
            sprintf(search_str, "clock_inst_new_%d", inst);
            if (cyg_httpd_form_varable_int(p, search_str, &var_int)) {
                new_clock_inst = var_int;
                T_D(" Creating a new clock_instance-[%d]",new_clock_inst);
                /* Got to Create the new instance here */
                sprintf(search_str, "devType_new_%d", inst);
                var_string = cyg_httpd_form_varable_string(p, search_str, &len);
                if (len > 0) {
                    (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    /* Change the Device Type of Clock Instance */
                    if (ptp_get_devtype_from_str(str2, &dev_type)) {
                        T_D("dev_type-[%d]",dev_type);
                        clock_init_bs.deviceType = dev_type;
                    }
                }
                sprintf(str1,"2_step_flag_new_%d",inst);
                var_string = cyg_httpd_form_varable_string(p, str1, &len);
                if (len > 0)
                    (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));

                clock_init_bs.twoStepFlag =  ptp_get_bool_value(str2);
                sprintf(str1,"clock_identity_new_%d",inst);
                var_string = cyg_httpd_form_varable_string(p, str1, &len);
                if (len > 0)
                    (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                else
                    strcpy(str2, "");

                if (sscanf(str2, "%2x:%2x:%2x:%2x:%2x:%2x:%2x:%2x",
                            &clock_identity[0], &clock_identity[1], &clock_identity[2],
                            &clock_identity[3], &clock_identity[4], &clock_identity[5],
                            &clock_identity[6], &clock_identity[7]) == 8) {
                    for (i = 0; i < 8; i++) {
                        clock_init_bs.clockIdentity[i] = clock_identity[i] & 0xff;
                    }
                }

                sprintf(str1,"one_way_new_%d",inst);
                var_string = cyg_httpd_form_varable_string(p, str1, &len);
                if (len > 0)
                    (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                clock_init_bs.oneWay =  ptp_get_bool_value(str2);

                sprintf(str1,"protocol_method_new_%d",inst);
                var_string = cyg_httpd_form_varable_string(p, str1, &len);
                if (len > 0)
                    (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));

                clock_init_bs.protocol = ptp_get_protocol(str2);
                
                sprintf(search_str, "vlan_ena_new_%d", inst);
                if (cyg_httpd_form_varable_find(p, search_str)){
                    /* VLAN Tagging support is enabled*/
                    clock_init_bs.tagging_enable = TRUE;
                }

                sprintf(search_str, "vid_new_%d", inst);
                if (cyg_httpd_form_varable_int(p, search_str, &var_int)) {
                    clock_init_bs.configured_vid = var_int;
                }

                sprintf(search_str, "pcp_new_%d", inst);
                if (cyg_httpd_form_varable_int(p, search_str, &var_int)) {
                    clock_init_bs.configured_pcp = var_int;
                }

                if (!ptp_clock_create(&clock_init_bs, new_clock_inst)) {
                    T_D(" Cannot Create Clock: Tried to Create more than one transparent clock ");
                }
                /* Look for all the ports that have been configured
                   for this new Clock */
                port_iter_init_local(&pit);
                while (port_iter_getnext(&pit)) {
                    port_no = pit.iport;
                    sprintf(search_str, "mask_new_%d_%d", inst,(uint)iport2uport(port_no));
                    if (cyg_httpd_form_varable_find(p, search_str)){
                        T_D("Enabling the port -[%d] ",port_no);
                        if ((rc = ptp_port_ena(FALSE, iport2uport(port_no), new_clock_inst)) != VTSS_RC_OK) {
                            T_D("clock_instance [%d], port %d not enabled (%s)",new_clock_inst, iport2uport(port_no), error_txt(rc));
                        }
                    }
                }
            } else {
                sprintf(search_str, "delete_%d", inst);

                if (cyg_httpd_form_varable_find(p, search_str)){
                    /* Delete the Clock Instance */
                    (void)ptp_clock_delete(inst);
                }
                if ((ptp_get_clock_default_ds(&clock_bs,inst))) {
                    port_iter_init_local(&pit);
                    while (port_iter_getnext(&pit)) {
                        port_no = pit.iport;
                        sprintf(search_str, "mask_%d_%d", inst,(uint)iport2uport(port_no));
                        if (cyg_httpd_form_varable_find(p, search_str)){
                            (void)ptp_port_ena(FALSE, iport2uport(port_no), inst);
                        } else {
                            (void)ptp_port_dis(iport2uport(port_no), inst);
                        }
                    }
                }
            }
        }
        redirect(p, "/ptp_config.htm");
    } else {
        if(!cyg_httpd_start_chunked("html"))
            return -1;

        T_D("Inside Else Part");
        /* default clock id */
        memset(str2, 0, sizeof(str2));
        (void) conf_mgmt_mac_addr_get(my_sysmac, 0);

        /* Send the Dynamic Parameters */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "^Disable/Output/Input");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        (void)cyg_httpd_write_chunked("^", 1);
        for (inst = 0; inst < PTP_CLOCK_INSTANCES; inst++)
        {
            sprintf(str2, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                        (int)my_sysmac[0], (int)my_sysmac[1], (int)my_sysmac[2],
                        0xff, 0xfe, (int)my_sysmac[3], (int)my_sysmac[4],
                        (int)(my_sysmac[5] + inst));
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", str2);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            if (inst != (PTP_CLOCK_INSTANCES - 1)) {
                (void)cyg_httpd_write_chunked("|", 1);
            }
        }

        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "^Ethernet/IPv4Multi/IPv4Uni");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* show the local clock  */
        vtss_ext_clock_out_get(&mode);

        (void)cyg_httpd_write_chunked("^", 1);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|%s|%d|%s",
                                      ptp_one_pps_mode_disp(mode.one_pps_mode),
                                      ptp_bool_disp(mode.clock_out_enable),
                                      mode.freq,
                                      ptp_bool_disp(mode.vcxo_enable));
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        (void)cyg_httpd_write_chunked("^", 1);

        dev_type = PTP_DEVICE_NONE;
        str = DeviceTypeToString(dev_type);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s", str);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        while (1) {
            dev_type++;
            str = DeviceTypeToString(dev_type);
            if (strcmp(str,"?") != 0 ) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%s", str);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                break;
            }
        }

        (void)cyg_httpd_write_chunked("#", 1);
        for (inst = 0; inst < PTP_CLOCK_INSTANCES; inst++)
        {
            T_D("Inst - %d",inst);
            if ((ptp_get_clock_default_ds(&clock_bs,inst))) {
                dataSet = TRUE;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%u", inst);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s", DeviceTypeToString(clock_bs.deviceType));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                port_iter_init_local(&pit);
                while (port_iter_getnext(&pit)) {
                    port_no = pit.iport;
                    if (ptp_get_port_ds(&port_bs, iport2uport(port_no),inst)) {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u", 1);
                    } else {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u", 0);
                    }
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }
            }
        }
        if (!dataSet)
            (void)cyg_httpd_write_chunked("",1);

        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

static cyg_int32 handler_clock_config_ptp(CYG_HTTPD_STATE* p)
{
    i32    ct;
    char   str [14];
    size_t len;
    vtss_timestamp_t t;
    uint inst = 0;
    int    var_value = 0;
    ulong ip_address;
    ptp_clock_default_ds_t clock_bs;
    ptp_clock_current_ds_t clock_current_bs;
    ptp_clock_parent_ds_t clock_parent_bs;
    ptp_clock_timeproperties_ds_t clock_time_properties_bs;
    ptp_set_clock_ds_t set_clock_bs;
#ifdef  PTP_OFFSET_FILTER_CUSTOM
    ptp_clock_servo_con_ds_t servo;
#else
    vtss_ptp_default_servo_config_t default_servo;
#endif /* PTP_OFFSET_FILTER_CUSTOM */
    vtss_ptp_default_filter_config_t default_offset;
    vtss_ptp_default_delay_filter_config_t default_delay;
    vtss_ptp_unicast_slave_config_t uni_slave;
    vtss_ptp_unicast_slave_config_state_t uni_conf_state;
    BOOL dataSet = FALSE;
    BOOL sync_2_sys_clock = FALSE;
    const i8 *var_string;
    i8 str1 [40];
    i8 str2 [40];
    i8 str3 [40]; /* Change this */
    u32 hw_time;
    u32 ix;


#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PTP))
        return -1;
#endif
    if (cyg_httpd_form_varable_int(p, "clock_inst", &var_value)) {
        inst = var_value;
    }

    // bool="true" || "false"
    if ((var_string = cyg_httpd_form_varable_string(p, "bool", &len)) != NULL && len >= 4) {
      if (strncasecmp(var_string, "true", 4) == 0) {
        sync_2_sys_clock = TRUE;
      } else if (strncasecmp(var_string, "false", 5) == 0) {
        sync_2_sys_clock = FALSE;
      }
    }

    if (sync_2_sys_clock == TRUE) {
        t.sec_msb = 0;
        t.seconds = time(NULL);
        t.nanoseconds = 0;
        vtss_local_clock_time_set(&t, inst);
        T_D("True is set,clock_instance-[%d] ",inst);
    }
    if (p->method == CYG_HTTPD_METHOD_POST) {
        T_D("Inside CYG_HTTPD_METHOD_POST post_data-[%s]",p->post_data);
        if ((ptp_get_clock_default_ds(&clock_bs,inst))) {
            set_clock_bs.domainNumber = clock_bs.domainNumber;
            set_clock_bs.priority1    = clock_bs.priority1;
            set_clock_bs.priority2    = clock_bs.priority2;

            sprintf(str1,"domain_%d",inst);
            if (cyg_httpd_form_varable_int(p, str1, &var_value) )
                set_clock_bs.domainNumber = var_value;

            sprintf(str1,"prio_1_%d",inst);
            if (cyg_httpd_form_varable_int(p, str1, &var_value))
                set_clock_bs.priority1 = var_value;

            sprintf(str1,"prio_2_%d",inst);
            if (cyg_httpd_form_varable_int(p, str1, &var_value))
                set_clock_bs.priority2 = var_value;

            if (!ptp_set_clock_default_ds(&set_clock_bs,inst)) {
                T_D("Clock instance %d : does not exist", inst);
            }

            if (ptp_get_clock_timeproperties_ds(&clock_time_properties_bs,inst)) {

                sprintf(str1,"uct_offset_%d",inst);
                if (cyg_httpd_form_varable_int(p, str1, &var_value))
                    clock_time_properties_bs.currentUtcOffset = var_value;


                sprintf(str1,"valid_%d",inst);
                if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                    if (len > 0)
                        (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    clock_time_properties_bs.currentUtcOffsetValid = ptp_get_bool_value(str2);
                }
                sprintf(str1,"leap59_%d",inst);
                if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                    if (len > 0)
                        (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    clock_time_properties_bs.leap59 = ptp_get_bool_value(str2);
                }

                sprintf(str1,"leap61_%d",inst);
                if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                    if (len > 0)
                        (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    clock_time_properties_bs.leap61 = ptp_get_bool_value(str2);
                }

                sprintf(str1,"time_trac_%d",inst);
                if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                    if (len > 0)
                        (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    clock_time_properties_bs.timeTraceable = ptp_get_bool_value(str2);
                }

                sprintf(str1,"freq_trac_%d",inst);
                if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                    if (len > 0)
                        (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    clock_time_properties_bs.frequencyTraceable = ptp_get_bool_value(str2);
                }

                sprintf(str1,"ptp_time_scale_%d",inst);
                if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                    if (len > 0)
                        (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    clock_time_properties_bs.ptpTimescale = ptp_get_bool_value(str2);
                }

                sprintf(str1,"time_source_%d",inst);
                if (cyg_httpd_form_varable_int(p, str1, &var_value))
                    clock_time_properties_bs.timeSource = var_value;

                if (!ptp_set_clock_timeproperties_ds(&clock_time_properties_bs,inst)) {
                    T_D("Clock instance %d : does not exist", inst);
                }
            }

            if (ptp_default_servo_parameters_get(&default_servo, inst)) {
                sprintf(str1,"display_%d",inst);
                if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                    if (len > 0)
                        (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    default_servo.display_stats =  ptp_get_bool_value(str2);
                }

                sprintf(str1,"p_enable_%d",inst);
                if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                    if (len > 0)
                        (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    default_servo.p_reg =  ptp_get_bool_value(str2);
                }
                sprintf(str1,"i_enable_%d",inst);
                if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                    if (len > 0)
                        (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    default_servo.i_reg =  ptp_get_bool_value(str2);
                }
                sprintf(str1,"d_enable_%d",inst);
                if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                    if (len > 0)
                        (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                    default_servo.d_reg =  ptp_get_bool_value(str2);
                }
                sprintf(str1,"p_const_%d",inst);
                if (cyg_httpd_form_varable_int(p, str1, &var_value))
                    default_servo.ap = var_value;

                sprintf(str1,"i_const_%d",inst);
                if (cyg_httpd_form_varable_int(p, str1, &var_value))
                    default_servo.ai = var_value;

                sprintf(str1,"d_const_%d",inst);
                if (cyg_httpd_form_varable_int(p, str1, &var_value))
                    default_servo.ad = var_value;

                if (!ptp_default_servo_parameters_set(&default_servo,inst)) {
                    T_D("Clock instance %d : does not exist", inst);
                }
            }
            if (ptp_default_delay_filter_parameters_get(&default_delay, inst)) {
                sprintf(str1,"delay_filter_%d",inst);
                if (cyg_httpd_form_varable_int(p, str1, &var_value)) {
                    default_delay.delay_filter  = var_value;
                    if (!ptp_default_delay_filter_parameters_set(&default_delay, inst)) {
                        T_D("Clock instance %d : does not exist", inst);
                    }
                }
            }
            if (ptp_default_filter_parameters_get(&default_offset, inst)) {
                sprintf(str1,"period_%d",inst);
                if (cyg_httpd_form_varable_int(p, str1, &var_value)) {
                    default_offset.period = var_value;
                }

                sprintf(str1,"dist_%d",inst);
                if (cyg_httpd_form_varable_int(p, str1, &var_value)) {
                    default_offset.dist = var_value;
                }

                if (!ptp_default_filter_parameters_set(&default_offset, inst)) {
                    T_D("Clock instance %d : does not exist", inst);
                }
            }
            ix = 0;
            while (ptp_uni_slave_conf_state_get(&uni_conf_state, ix++, inst)) {
                uni_slave.duration = uni_conf_state.duration;
                uni_slave.ip_addr  = uni_conf_state.ip_addr;

                sprintf(str1,"uc_dura_%d_%d",inst,(int)(ix-1));
                if (cyg_httpd_form_varable_int(p, str1, &var_value)) {
                    uni_slave.duration = var_value;
                }

                sprintf(str1,"uc_ip_%d_%d",inst,(int)(ix-1));
                if (cyg_httpd_form_varable_ipv4(p, str1, &ip_address)) {
                    uni_slave.ip_addr = ip_address;
                }

                if (!ptp_uni_slave_conf_set(&uni_slave, (ix-1) , inst)) {
                    T_D("Clock instance %d : does not exist",inst);
                }
            }
        }
        sprintf(str1, "/ptp_clock_config.htm?clock_inst=%d", inst);
        redirect(p, str1);
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        T_D("Get Method");
        if(!cyg_httpd_start_chunked("html"))
            return -1;
        if ((ptp_get_clock_default_ds(&clock_bs, inst))) {
            dataSet = TRUE;
            /* Send the Dynamic Parameters */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "^Ethernet/IPv4Multi/IPv4Uni");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            (void)cyg_httpd_write_chunked("^", 1);
            /* send the local clock  */
            vtss_local_clock_time_get(&t, inst, &hw_time);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%s %s", misc_time2str(t.seconds), vtss_tod_ns2str(t.nanoseconds, str,','));
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s", clock_adjustment_method_txt[vtss_ptp_adjustment_method(inst)]);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            /* Send the Default clock Data Set */
            if (clock_bs.deviceType == PTP_DEVICE_NONE) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        " #%d /%s", inst,
                        DeviceTypeToString(clock_bs.deviceType));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%d/%s/%s/%d/%s/%d",
                        inst, DeviceTypeToString(clock_bs.deviceType),
                        (clock_bs.twoStepFlag) ? "True" : "False", clock_bs.numberPorts, ClockIdentityToString(clock_bs.clockIdentity, str1), clock_bs.domainNumber);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "/%s/%d/%d",
                        ClockQualityToString(&clock_bs.clockQuality, str2), clock_bs.priority1, clock_bs.priority2);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "/%s/%s", ptp_protocol_disp(clock_bs.protocol),
                        (clock_bs.oneWay) ? "True" : "False");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer), "/%s/%d/%d", 
                         (clock_bs.tagging_enable) ? "True" : "False",
                         (clock_bs.configured_vid),
                         (clock_bs.configured_pcp));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            }

            /* Send the current Clock Data Set */
            if (ptp_get_clock_current_ds(&clock_current_bs, inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%d/%s/%s",
                        clock_current_bs.stepsRemoved, vtss_tod_TimeInterval_To_String(&clock_current_bs.offsetFromMaster, str1,','),
                        vtss_tod_TimeInterval_To_String(&clock_current_bs.meanPathDelay, str2,','));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            if (ptp_get_clock_parent_ds(&clock_parent_bs, inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%s/%d/%s/%d/%d",
                        ClockIdentityToString(clock_parent_bs.parentPortIdentity.clockIdentity, str1), clock_parent_bs.parentPortIdentity.portNumber,
                        ptp_bool_disp(clock_parent_bs.parentStats),
                        clock_parent_bs.observedParentOffsetScaledLogVariance,
                        clock_parent_bs.observedParentClockPhaseChangeRate);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "/%s/%s/%d/%d",
                        ClockIdentityToString(clock_parent_bs.grandmasterIdentity, str2),
                        ClockQualityToString(&clock_parent_bs.grandmasterClockQuality, str3),
                        clock_parent_bs.grandmasterPriority1,
                        clock_parent_bs.grandmasterPriority2);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            if (ptp_get_clock_timeproperties_ds(&clock_time_properties_bs, inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%d/%s/%s/%s/%s/%s/%s/%d",
                        clock_time_properties_bs.currentUtcOffset,
                        ptp_bool_disp(clock_time_properties_bs.currentUtcOffsetValid),
                        ptp_bool_disp(clock_time_properties_bs.leap59),
                        ptp_bool_disp(clock_time_properties_bs.leap61),
                        ptp_bool_disp(clock_time_properties_bs.timeTraceable),
                        ptp_bool_disp(clock_time_properties_bs.frequencyTraceable),
                        ptp_bool_disp(clock_time_properties_bs.ptpTimescale),
                        clock_time_properties_bs.timeSource);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            /* send the servo parameters  */
            if (ptp_default_servo_parameters_get(&default_servo, inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%s/%s/%s/%s/%d/%d/%d",
                        ptp_bool_disp(default_servo.display_stats),
                        ptp_bool_disp(default_servo.p_reg),
                        ptp_bool_disp(default_servo.i_reg),
                        ptp_bool_disp(default_servo.d_reg),
                        default_servo.ap,
                        default_servo.ai,
                        default_servo.ad);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            /* send the filter parameters  */
            if (ptp_default_filter_parameters_get(&default_offset, inst) &&
                    ptp_default_delay_filter_parameters_get(&default_delay, inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%d/%d/%d",
                        default_delay.delay_filter,
                        default_offset.period,
                        default_offset.dist);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            /* send the unicast slave parameters */
            (void)cyg_httpd_write_chunked("#", 1);
            ix = 0;
            while (ptp_uni_slave_conf_state_get(&uni_conf_state, ix++, inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "%u/%d/%s/%d/%s|",
                        (ix-1),
                        uni_conf_state.duration,
                        misc_ipv4_txt(uni_conf_state.ip_addr, str1),
                        uni_conf_state.log_msg_period,
                        ptp_state_disp(uni_conf_state.comm_state));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        if (!dataSet)
            (void)cyg_httpd_write_chunked("",1);

        cyg_httpd_end_chunked();

    }
    return -1; // Do not further search the file system.
}

static cyg_int32 handler_clock_ports_config_ptp(CYG_HTTPD_STATE* p)
{
    i32    ct;
    size_t len;
    u8 mech_type;
    uint inst = 0;
    int    var_value;
    vtss_timeinterval_t latency;
    ptp_clock_default_ds_t clock_bs;
    ptp_set_port_ds_t  set_port_bs;
    ptp_port_ds_t port_bs;
    vtss_port_no_t    port_no;
    BOOL dataSet = FALSE;
    const i8 *var_string;
    i8 str1 [50];
    i8 str2 [40];
    port_iter_t       pit;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PTP))
        return -1;
#endif

    if (cyg_httpd_form_varable_int(p, "clock_inst", &var_value)) {
        inst = var_value;
    }
    if (p->method == CYG_HTTPD_METHOD_POST) {
        T_D("Inside CYG_HTTPD_METHOD_POST post_data-[%s]",p->post_data);
        if ((ptp_get_clock_default_ds(&clock_bs,inst))) {

            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                port_no = pit.iport;
                if (ptp_get_port_ds(&port_bs, iport2uport(port_no), inst)) {
                    set_port_bs.logAnnounceInterval    = port_bs.logAnnounceInterval;
                    set_port_bs.announceReceiptTimeout = port_bs.announceReceiptTimeout;
                    set_port_bs.logSyncInterval        = port_bs.logSyncInterval;
                    set_port_bs.delayMechanism         = port_bs.delayMechanism;
                    set_port_bs.logMinPdelayReqInterval = port_bs.logMinPdelayReqInterval;
                    set_port_bs.delayAsymmetry         = port_bs.delayAsymmetry;
                    set_port_bs.versionNumber          = port_bs.versionNumber;
                    set_port_bs.ingressLatency         = port_bs.ingressLatency;
                    set_port_bs.egressLatency          = port_bs.egressLatency;

                    sprintf(str1,"anv_%d_%d",inst,(uint)iport2uport(port_no));
                    if (cyg_httpd_form_varable_int(p, str1, &var_value)) {
                        set_port_bs.logAnnounceInterval = var_value;
                    }

                    sprintf(str1,"ato_%d_%d",inst,(uint)iport2uport(port_no));
                    if (cyg_httpd_form_varable_int(p,str1,&var_value)) {
                        set_port_bs.announceReceiptTimeout = var_value;
                    }

                    sprintf(str1,"syv_%d_%d",inst,(uint)iport2uport(port_no));
                    if (cyg_httpd_form_varable_int(p,str1,&var_value)) {
                        set_port_bs.logSyncInterval = var_value;
                    }

                    sprintf(str1,"dlm_%d_%d",inst,(uint)iport2uport(port_no));
                    if ((var_string = cyg_httpd_form_varable_string(p, str1, &len))) {
                        if (len > 0)
                            (void)cgi_unescape(var_string, &str2[0], len, sizeof(str2));
                        if (ptp_get_delaymechanism_type(str2,&mech_type)) {
                            set_port_bs.delayMechanism = mech_type;
                        }
                    }

                    sprintf(str1,"mpr_%d_%d",inst,(uint)iport2uport(port_no));
                    if (cyg_httpd_form_varable_int(p,str1,&var_value)) {
                        set_port_bs.logMinPdelayReqInterval = var_value;
                    }
                    sprintf(str1,"delay_assymetry_%d_%d",inst,(uint)iport2uport(port_no));
                    var_value = 0;
                    if (cyg_httpd_form_varable_int(p, str1, &var_value)) {
                        latency = 0;
                        latency = (vtss_timeinterval_t) var_value << 16;
                        set_port_bs.delayAsymmetry = latency;
                    }
                    sprintf(str1,"ingress_latency_%d_%d",inst,(uint)iport2uport(port_no));
                    var_value = 0;
                    if (cyg_httpd_form_varable_int(p, str1, &var_value) ) {
                        latency = 0;
                        latency = (vtss_timeinterval_t) var_value << 16;
                        set_port_bs.ingressLatency = latency;
                    }
                    sprintf(str1,"egress_latency_%d_%d",inst,(uint)iport2uport(port_no));
                    var_value = 0;
                    if (cyg_httpd_form_varable_int(p, str1, &var_value)) {
                        latency = 0;
                        latency = (vtss_timeinterval_t) var_value << 16;
                        set_port_bs.egressLatency = latency;
                    }
                    if (set_port_bs.logAnnounceInterval   != port_bs.logAnnounceInterval     ||
                        set_port_bs.announceReceiptTimeout != port_bs.announceReceiptTimeout ||
                        set_port_bs.logSyncInterval        != port_bs.logSyncInterval ||
                        set_port_bs.delayMechanism         != port_bs.delayMechanism  ||
                        set_port_bs.logMinPdelayReqInterval != port_bs.logMinPdelayReqInterval ||
                        set_port_bs.delayAsymmetry         != port_bs.delayAsymmetry ||
                        set_port_bs.versionNumber          != port_bs.versionNumber  ||
                        set_port_bs.ingressLatency         != port_bs.ingressLatency ||
                        set_port_bs.egressLatency          != port_bs.egressLatency) {
                        if (!ptp_set_port_ds(iport2uport(port_no), &set_port_bs,inst)) {
                            T_D("Clock instance %d : does not exist",inst);
                        }
                    }
                }
            }
        }
        sprintf(str1, "/ptp_clock_ports_config.htm?clock_inst=%d", inst);
        redirect(p, str1);
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        T_D("Get Method");
        if(!cyg_httpd_start_chunked("html"))
            return -1;
        if ((ptp_get_clock_default_ds(&clock_bs, inst))) {
            dataSet = TRUE;
            /* Delay Mechanism */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "^e2e/p2p");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            (void)cyg_httpd_write_chunked("^", 1);
            (void)cyg_httpd_write_chunked("#", 1);
            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                port_no = pit.iport;
                if (ptp_get_port_ds(&port_bs, iport2uport(port_no), inst)) {
                    ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                            "$%d/%s/%d/%s/%d/%d/%d/%s/%d/%lld/%lld/%lld/%d",
                            port_bs.portIdentity.portNumber,
                            PortStateToString(port_bs.portState),port_bs.logMinDelayReqInterval,
                            vtss_tod_TimeInterval_To_String(&port_bs.peerMeanPathDelay, str1,','),
                            port_bs.logAnnounceInterval,
                            port_bs.announceReceiptTimeout,port_bs.logSyncInterval,ptp_delaymechanism_disp(port_bs.delayMechanism),
                            port_bs.logMinPdelayReqInterval,
                            (port_bs.delayAsymmetry >> 16),
                            (port_bs.ingressLatency >> 16),
                            (port_bs.egressLatency >> 16),
                            port_bs.versionNumber);
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }
            }
        }
        if (!dataSet)
            (void)cyg_httpd_write_chunked("",1);

        cyg_httpd_end_chunked();

    }
    return -1; // Do not further search the file system.
}


static cyg_int32 handler_stat_ptp(CYG_HTTPD_STATE* p)
{
    i32         ct;
    uint inst;
    ptp_clock_default_ds_t clock_bs;
    vtss_ptp_ext_clock_mode_t mode;
    ptp_port_ds_t port_bs;
    vtss_port_no_t    port_no;
    BOOL dataSet = FALSE;
    port_iter_t       pit;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PTP))
        return -1;
#endif
    if (p->method == CYG_HTTPD_METHOD_POST) {
       T_D("Inside CYG_HTTPD_METHOD_POST ");
       redirect(p, "/ptp.htm");
    } else {
        if(!cyg_httpd_start_chunked("html"))
            return -1;
        /* show the local clock  */
        vtss_ext_clock_out_get(&mode);

        (void)cyg_httpd_write_chunked("^", 1);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|%s|%d|%s",
                                      ptp_one_pps_mode_disp(mode.one_pps_mode),
                                      ptp_bool_disp(mode.clock_out_enable),
                                      mode.freq,
                                      ptp_bool_disp(mode.vcxo_enable));
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        (void)cyg_httpd_write_chunked("^", 1);
        for (inst = 0; inst < PTP_CLOCK_INSTANCES; inst++)
        {
            T_D("Inst - %d",inst);
            if ((ptp_get_clock_default_ds(&clock_bs,inst))) {
                dataSet = TRUE;
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%u", inst);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s", DeviceTypeToString(clock_bs.deviceType));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                port_iter_init_local(&pit);
                while (port_iter_getnext(&pit)) {
                    port_no = pit.iport;
                    if (ptp_get_port_ds(&port_bs, iport2uport(port_no),inst)) {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u", 1);
                    } else {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u", 0);
                    }
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }
            }
        }
        if (!dataSet)
            (void)cyg_httpd_write_chunked("No entries",10);
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}

static cyg_int32 handler_clock_stat_ptp(CYG_HTTPD_STATE* p)
{
    i32         ct;
    uint  request_inst = 0;
    vtss_timestamp_t t;
    i8 str [14];
    int    var_value;
    ptp_clock_default_ds_t clock_bs;
    ptp_clock_current_ds_t clock_current_bs;
    ptp_clock_slave_ds_t clock_slave_bs;

    ptp_clock_parent_ds_t clock_parent_bs;
    ptp_clock_timeproperties_ds_t clock_time_properties_bs;
#ifdef  PTP_OFFSET_FILTER_CUSTOM
    ptp_clock_servo_con_ds_t servo;
#else
    vtss_ptp_default_servo_config_t default_servo;
#endif /* PTP_OFFSET_FILTER_CUSTOM */
    vtss_ptp_default_filter_config_t default_offset;
    vtss_ptp_default_delay_filter_config_t default_delay;
    vtss_ptp_unicast_slave_config_state_t uni_conf_state;
    BOOL dataSet = FALSE;
    i8 str1 [40];
    i8 str2 [40];
    i8 str3 [40];
    u32 hw_time;
    i32 ix;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PTP))
        return -1;
#endif
    if (cyg_httpd_form_varable_int(p, "clock_inst", &var_value)) {
        request_inst = var_value;
    }

    if (p->method == CYG_HTTPD_METHOD_POST) {
        T_D("Inside CYG_HTTPD_METHOD_POST ");
        redirect(p, "/ptp_clock.htm");
    } else {
        if(!cyg_httpd_start_chunked("html"))
            return -1;
        if ((ptp_get_clock_default_ds(&clock_bs, request_inst))) {
            dataSet = TRUE;
            /* send the local clock  */
            vtss_local_clock_time_get(&t, request_inst, &hw_time);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "#%s %s", misc_time2str(t.seconds), vtss_tod_ns2str(t.nanoseconds, str,','));
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%s", clock_adjustment_method_txt[vtss_ptp_adjustment_method(request_inst)]);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            /* Send the Default clock Data Set */
            if (clock_bs.deviceType == PTP_DEVICE_NONE) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        " #%d /%s", request_inst,
                        DeviceTypeToString(clock_bs.deviceType));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%d/%s/%s/%d/%s/%d",
                        request_inst, DeviceTypeToString(clock_bs.deviceType),
                        (clock_bs.twoStepFlag) ? "True" : "False", clock_bs.numberPorts, ClockIdentityToString(clock_bs.clockIdentity, str1), clock_bs.domainNumber);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "/%s/%d/%d",
                        ClockQualityToString(&clock_bs.clockQuality, str2), clock_bs.priority1, clock_bs.priority2);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "/%s/%s", ptp_protocol_disp(clock_bs.protocol),
                        (clock_bs.oneWay) ? "True" : "False");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer), "/%s/%d/%d", 
                         (clock_bs.tagging_enable) ? "True" : "False",
                         (clock_bs.configured_vid),
                         (clock_bs.configured_pcp));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            /* Send the current Clock Data Set */
            if (ptp_get_clock_current_ds(&clock_current_bs,request_inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%d/%s/%s",
                        clock_current_bs.stepsRemoved, vtss_tod_TimeInterval_To_String(&clock_current_bs.offsetFromMaster, str1,','),
                        vtss_tod_TimeInterval_To_String(&clock_current_bs.meanPathDelay, str2,','));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            /* Send the Clock Slave Data Set */
            if (ptp_get_clock_slave_ds(&clock_slave_bs,request_inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                              "/%d/%s/%s",
                              clock_slave_bs.port_number,
                              vtss_ptp_slave_state_2_text(clock_slave_bs.slave_state),
                              vtss_ptp_ho_state_2_text(clock_slave_bs.holdover_stable, clock_slave_bs.holdover_adj, str1));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            if (ptp_get_clock_parent_ds(&clock_parent_bs,request_inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%s/%d/%s/%d/%d",
                        ClockIdentityToString(clock_parent_bs.parentPortIdentity.clockIdentity, str1), clock_parent_bs.parentPortIdentity.portNumber,
                        ptp_bool_disp(clock_parent_bs.parentStats),
                        clock_parent_bs.observedParentOffsetScaledLogVariance,
                        clock_parent_bs.observedParentClockPhaseChangeRate);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "/%s/%s/%d/%d",
                        ClockIdentityToString(clock_parent_bs.grandmasterIdentity, str2),
                        ClockQualityToString(&clock_parent_bs.grandmasterClockQuality, str3),
                        clock_parent_bs.grandmasterPriority1,
                        clock_parent_bs.grandmasterPriority2);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            if (ptp_get_clock_timeproperties_ds(&clock_time_properties_bs,request_inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%d/%s/%s/%s/%s/%s/%s/%d",
                        clock_time_properties_bs.currentUtcOffset,
                        ptp_bool_disp(clock_time_properties_bs.currentUtcOffsetValid),
                        ptp_bool_disp(clock_time_properties_bs.leap59),
                        ptp_bool_disp(clock_time_properties_bs.leap61),
                        ptp_bool_disp(clock_time_properties_bs.timeTraceable),
                        ptp_bool_disp(clock_time_properties_bs.frequencyTraceable),
                        ptp_bool_disp(clock_time_properties_bs.ptpTimescale),
                        clock_time_properties_bs.timeSource);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            /* send the servo parameters  */
            if (ptp_default_servo_parameters_get(&default_servo, request_inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%s/%s/%s/%s/%d/%d/%d",
                        ptp_bool_disp(default_servo.display_stats),
                        ptp_bool_disp(default_servo.p_reg),
                        ptp_bool_disp(default_servo.i_reg),
                        ptp_bool_disp(default_servo.d_reg),
                        default_servo.ap,
                        default_servo.ai,
                        default_servo.ad);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            /* send the filter parameters  */
            if (ptp_default_filter_parameters_get(&default_offset, request_inst) &&
                    ptp_default_delay_filter_parameters_get(&default_delay, request_inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "#%d/%d/%d",
                        default_delay.delay_filter,
                        default_offset.period,
                        default_offset.dist);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            /* send the unicast slave parameters */
            (void)cyg_httpd_write_chunked("#", 1);
            ix = 0;
            while (ptp_uni_slave_conf_state_get(&uni_conf_state, ix++, request_inst)) {
                ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                        "%u/%d/%s/%d/%s|",
                        (ix-1),
                        uni_conf_state.duration,
                        misc_ipv4_txt(uni_conf_state.ip_addr, str1),
                        uni_conf_state.log_msg_period,
                        ptp_state_disp(uni_conf_state.comm_state));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        if (!dataSet)
            (void)cyg_httpd_write_chunked("",1);
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}


static cyg_int32 handler_clock_ports_stat_ptp(CYG_HTTPD_STATE* p)
{
    i32         ct;
    int   var_value;
    uint  request_inst = 0;
    ptp_clock_default_ds_t clock_bs;
    ptp_port_ds_t port_bs;
    vtss_port_no_t    port_no;
    BOOL dataSet = FALSE;
    i8 str1 [40];
    i8 str2 [40];
    i8 str3 [40];
    i8 str4 [40];
    port_iter_t       pit;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PTP))
        return -1;
#endif
    if (cyg_httpd_form_varable_int(p, "clock_inst", &var_value)) {
        request_inst = var_value;
    }
    if (p->method == CYG_HTTPD_METHOD_POST) {
        T_D("Inside CYG_HTTPD_METHOD_POST ");
        redirect(p, "/ptp_clock_ports.htm");
    } else {
        if(!cyg_httpd_start_chunked("html"))
            return -1;
        if ((ptp_get_clock_default_ds(&clock_bs, request_inst))) {
            dataSet = TRUE;
           (void)cyg_httpd_write_chunked("#", 1);
           port_iter_init_local(&pit);
           while (port_iter_getnext(&pit)) {
               port_no = pit.iport;
                if (ptp_get_port_ds(&port_bs, iport2uport(port_no),request_inst)) {
                    ct = snprintf(p->outbuffer,sizeof(p->outbuffer),
                            "$%d/%s/%d/%s/%d/%d/%d/%s/%d/%s/%s/%s/%d",
                            port_bs.portIdentity.portNumber,
                            PortStateToString(port_bs.portState),port_bs.logMinDelayReqInterval,
                            vtss_tod_TimeInterval_To_String(&port_bs.peerMeanPathDelay, str1,','),
                            port_bs.logAnnounceInterval,
                            port_bs.announceReceiptTimeout,port_bs.logSyncInterval,ptp_delaymechanism_disp(port_bs.delayMechanism),
                            port_bs.logMinPdelayReqInterval,
                            vtss_tod_TimeInterval_To_String(&port_bs.delayAsymmetry, str2,','),
                            vtss_tod_TimeInterval_To_String(&port_bs.ingressLatency, str3,','),
                            vtss_tod_TimeInterval_To_String(&port_bs.egressLatency, str4,','),
                            port_bs.versionNumber);
                    (void)cyg_httpd_write_chunked(p->outbuffer, ct);
                }
            }
        }
        if (!dataSet)
            (void)cyg_httpd_write_chunked("",1);
        cyg_httpd_end_chunked();
    }
    return -1; // Do not further search the file system.
}


/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ptp, "/config/ptp_config", handler_config_ptp);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_clock_config_ptp, "/config/ptp_clock_config", handler_clock_config_ptp);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_clock_ports_config_ptp, "/config/ptp_clock_ports_config", handler_clock_ports_config_ptp);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_ptp, "/stat/ptp", handler_stat_ptp);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_ptp_clock, "/stat/ptp_clock", handler_clock_stat_ptp);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_ptp_clock_ports, "/stat/ptp_clock_ports", handler_clock_ports_stat_ptp);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
