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
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_web_api.h"
#endif

#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif /* VTSS_SWITCH_STACKABLE */

#define PORT_WEB_BUF_LEN 512

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>
/* ============== */

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

#define PORT_WEB_BUF_LEN 512

static size_t port_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buf[PORT_WEB_BUF_LEN];
    (void) snprintf(buf, PORT_WEB_BUF_LEN,
                    "var configPortFrameSizeMin = %d;\n"
                    "var configPortFrameSizeMax = %d;\n",
                    VTSS_MAX_FRAME_LENGTH_STANDARD, 
                    VTSS_MAX_FRAME_LENGTH_MAX);
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buf);
}

/****************************************************************************/
/*  JS lib_config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(port_lib_config_js);

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/

#if VTSS_UI_OPT_VERIPHY == 1
static cyg_int32 handler_config_veriphy(CYG_HTTPD_STATE* p)
{
    vtss_isid_t               sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    int			      val;
    vtss_port_no_t            iport;
    vtss_uport_no_t           selected_uport = 0;
    port_isid_port_info_t     info;
    BOOL                      is_post_request_in_disguise = FALSE;
    int                       cnt;
    vtss_phy_veriphy_result_t veriphy_result;
    BOOL                      first = TRUE;
    u32                       port_count = port_isid_port_count(sid);

    if(redirectUnmanagedOrInvalid(p, sid)) /* Redirect unmanaged/invalid access to handler */
        return -1;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PING))
        return -1;
#endif

    // This function does not support the HTTP POST method because
    // running VeriPHY on a 10/100 connection will cause the PHY to
    // link down, which in turn may cause the browser to not get any
    // reply to a HTTP POST request, causing it to timeout. This is
    // not the case with GET requests. If it's a request, the URL
    // has a "?port=<valid_port_number>". If it's a status get, the URL
    // has a "?port=-1" or no port at all.
    if(p->method == CYG_HTTPD_METHOD_POST) {
        redirect(p, "/veriphy.htm");
        return -1;
    }

    if(cyg_httpd_form_varable_int(p, "port", &val)) {
        selected_uport = val;
        if(selected_uport == 0) {
            is_post_request_in_disguise = TRUE;
        } else {
            if(uport2iport(selected_uport) < VTSS_PORT_NO_END)
                is_post_request_in_disguise = TRUE;
        }
    }
    
    if(is_post_request_in_disguise) {
        port_veriphy_mode_t veriphy_mode[VTSS_PORT_ARRAY_SIZE];
        
        // Fill-in array telling which ports to run VeriPHY on.
        // Even when VeriPHY is already running on a given port, it's safe to
        // call port_mgmt_veriphy_start() with PORT_VERIPHY_MODE_FULL again.
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            if ((selected_uport == 0 || uport2iport(selected_uport) == iport) && 
                port_isid_port_info_get(sid, iport, &info) == VTSS_OK && 
                (info.cap & PORT_CAP_1G_PHY)) {
                veriphy_mode[iport] = PORT_VERIPHY_MODE_FULL;
            } else {
                veriphy_mode[iport] = PORT_VERIPHY_MODE_NONE;
            }
        }

        // Don't care about return code.
        (void) port_mgmt_veriphy_start(sid, veriphy_mode);
    }

    // Always reply.
    cyg_httpd_start_chunked("html");

    // Only data for ports with a PHY will be transferred to the browser.
    // Format of data:
    // port_1/veriphy_status_1/status_1_1/length_1_1/status_1_2/length_1_2/status_1_3/length_1_3/status_1_4/length_1_4;
    // port_2/veriphy_status_2/status_2_1/length_2_1/status_2_2/length_2_2/status_2_3/length_2_3/status_2_4/length_2_4;
    // ...
    // port_n/veriphy_status_n/status_n_1/length_n_1/status_n_2/length_n_2/status_n_3/length_n_3/status_n_4/length_n_4;
    // If veriphy_status_n is 0, VeriPHY has not been run in this port, and the remaining data is invalid.
    // If veriphy_status_n is 1, VeriPHY is in progress, and the remaining data is invalid.
    // If veriphy_status_n is 2, VeriPHY has been run on this port, and the remaining data is ok.
    for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
        if (port_isid_port_info_get(sid, iport, &info) == VTSS_OK && 
            (info.cap & PORT_CAP_1G_PHY)) {
            vtss_rc rc = port_mgmt_veriphy_get(sid, iport, &veriphy_result, 0);
            if(rc == VTSS_OK || rc == PORT_ERROR_INCOMPLETE || rc == PORT_ERROR_GEN) {
                // Other port errors we don't bother to handle
                cnt = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s%d/%u/%u/%u/%u/%u/%u/%u/%u/%u",
                    first ? "" : ";",
                    iport2uport(iport),
                    rc == PORT_ERROR_GEN ? 0 : (rc == PORT_ERROR_INCOMPLETE ? 1 : 2),
                    veriphy_result.status[0],
                    veriphy_result.length[0],
                    veriphy_result.status[1],
                    veriphy_result.length[1],
                    veriphy_result.status[2],
                    veriphy_result.length[2],
                    veriphy_result.status[3],
                    veriphy_result.length[3]);
                cyg_httpd_write_chunked(p->outbuffer, cnt);
                first = FALSE;
            }
        }
    }

    cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}
#endif /* VTSS_UI_OPT_VERIPHY == 1 */
static cyg_int32 handler_stat_ports(CYG_HTTPD_STATE* p)
{
    vtss_isid_t          sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t       iport;
    vtss_port_counters_t counters;
    int                  ct;
    BOOL		 clear = (cyg_httpd_form_varable_find(p, "clear") != NULL);
    u32                  port_count;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_PORT))
        return -1;
#endif

    cyg_httpd_start_chunked("html");

    if(VTSS_ISID_LEGAL(sid) &&
       msg_switch_exists(sid)) {
        port_count = port_isid_port_count(sid);
        for (iport = VTSS_PORT_NO_START; iport < port_count; iport++) {
            if(clear) {      /* Clear? */
                if(port_mgmt_counters_clear(sid, iport) != VTSS_OK)
                    break;              /* Most likely stack error - bail out */
                memset(&counters, 0, sizeof(counters)); /* Cheating a little... */
            } else {
                /* Normal read */
                if (port_mgmt_counters_get(sid, iport, &counters) != VTSS_OK)
                    break;              /* Most likely stack error - bail out */
            }
            /* Output the counters */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu%s",
                          iport2uport(iport),
                          counters.rmon.rx_etherStatsPkts,
                          counters.rmon.tx_etherStatsPkts,
                          counters.rmon.rx_etherStatsOctets,
                          counters.rmon.tx_etherStatsOctets,
                          counters.if_group.ifInErrors,
                          counters.if_group.ifOutErrors,
                          counters.rmon.rx_etherStatsDropEvents,
                          counters.rmon.tx_etherStatsDropEvents,
                          counters.bridge.dot1dTpPortInDiscards,
                          iport == (port_count - 1) ? "" : "|");
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }

    cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

/* Support for a given feature is encoded like this: 
   0: No supported
   1: Supported on other ports
   2: Supported on this port */
static u8 port_feature_support(port_cap_t cap, port_cap_t port_cap, port_cap_t mask) 
{
    return ((cap & mask) ? ((port_cap & mask) ? 2 : 1) : 0);
}

static cyg_int32 handler_config_ports(CYG_HTTPD_STATE* p)
{
    vtss_isid_t      sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_uport_no_t  uport;
    vtss_port_no_t   iport;
    port_conf_t      conf;
    int              ct;
    u32              port_count = port_isid_port_count(sid);
    port_isid_info_t info;
    port_cap_t       cap;
    char             buf[80];

    if(redirectUnmanagedOrInvalid(p, sid)) /* Redirect unmanaged/invalid access to handler */
        return -1;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PORT))
        return -1;
#endif

    if(p->method == CYG_HTTPD_METHOD_POST) {
        int errors = 0;


        for (iport = 0; iport < port_count; iport++) {
            port_conf_t newconf;
            uport = iport2uport(iport);
            int enable, autoneg, fiber_speed, speed, fdx, max, val;
            const char *str;

            if(port_mgmt_conf_get(sid, iport, &conf) < 0) {
                T_E("Could not get port_mgmt_conf_get, sid = %d, iport =%u", sid, iport);
                errors++;   /* Probably stack error */
                continue;
            }
            newconf = conf;

            if((str = cyg_httpd_form_variable_str_fmt(p, NULL, "speed_%d", uport)) &&
               sscanf(str, "%uA%uA%uA%uA%u", &enable, &autoneg, &speed, &fdx, &fiber_speed) == 5
                ) {
                T_I("Port %2d: enb:%d auto:%d speed:%d fdx:%d fiber:%d", iport, enable, autoneg, speed, fdx, fiber_speed);
                if((newconf.enable = enable)) {
                    if(!(newconf.autoneg = autoneg)) {
                        newconf.speed = speed;
                        newconf.fdx = fdx;
                    }
                    newconf.dual_media_fiber_speed = fiber_speed; // We must make sure that number in htm corresponds to the number defined in vtss_fiber_port_speed_t
                }
            }

            /* flow_%d: CHECKBOX */
            newconf.flow_control = cyg_httpd_form_variable_check_fmt(p, "flow_%d", uport);

            /* max_%d: TEXT */
            if(cyg_httpd_form_variable_int_fmt(p, &max, "max_%d", uport) &&
               max >= VTSS_MAX_FRAME_LENGTH_STANDARD &&
               max <= VTSS_MAX_FRAME_LENGTH_MAX) {
                newconf.max_length = max;
            }

            /* exc_%d: INT */
            if(cyg_httpd_form_variable_int_fmt(p, &val, "exc_%d", uport))
                newconf.exc_col_cont = val;

            if((str = cyg_httpd_form_variable_str_fmt(p, NULL, "fiber_speed_%d", uport)) &&
               sscanf(str, "%uA", &fiber_speed) == 1) {
//                newconf.fiber_speed = fiber_speed; // We musk make sure that number in htm corresponds to the number defined in vtss_fiber_port_speed_t
            }



            if(memcmp(&newconf, &conf, sizeof(newconf)) != 0) {
                T_D("port_mgmt_conf_set(%u,%d)", sid, iport);
                if (port_mgmt_conf_set(sid, iport, &newconf) < 0) {
                    T_E("Could not set port_mgmt_conf_get, sid = %d, iport =%u", sid, iport);
                    errors++; /* Probably stack error */
                }
            }
        }
        T_D("errors = %d", errors);
        redirect(p, errors ? STACK_ERR_URL : "/ports.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        cyg_httpd_start_chunked("html");
        
        cap = (port_isid_info_get(sid, &info) == VTSS_RC_OK ? info.cap : 0);
        for (iport = 0; iport < port_count; iport++) {
            port_status_t      port_status;
            vtss_port_status_t *status = &port_status.status;
            port_vol_status_t  vol_status;
            BOOL               rx_pause, tx_pause;

            if(port_mgmt_conf_get(sid, iport, &conf) < 0 ||
               port_mgmt_status_get_all(sid, iport, &port_status) < 0)
                break;          /* Probably stack error - bail out */
            if(!port_isid_port_no_is_stack(sid, iport)) {
                if (status->link)
                    strcpy(buf, port_mgmt_mode_txt(iport, status->speed, status->fdx, port_status.fiber));
                else if (port_vol_status_get(PORT_USER_CNT, sid, iport, &vol_status) == VTSS_OK && 
                         vol_status.conf.disable && vol_status.user != PORT_USER_STATIC)
                    sprintf(buf, "Down (%s)", vol_status.name);
                else
                    strcpy(buf, "Down");


                rx_pause = (conf.autoneg ? (status->link ? status->aneg.obey_pause : 0) :
                            conf.flow_control);
                tx_pause = (conf.autoneg ? (status->link ? status->aneg.generate_pause : 0) :
                            conf.flow_control);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%u/%u/%u/%u/%u/%u/%u/%u/%u/%s/%s/%u/%u/%u/%u/%u/%u/%u/%u|",
                              iport2uport(iport),
                              port_status.cap,
                              conf.enable,
                              conf.autoneg,
                              conf.speed,
                              conf.fdx,
                              conf.max_length,
                              port_feature_support(cap, port_status.cap, PORT_CAP_FLOW_CTRL),
                              conf.flow_control,
                              status->link ? "Up" : "Down",
                              buf,
                              rx_pause,
                              tx_pause,
                              port_feature_support(cap, port_status.cap, PORT_CAP_HDX),
                              conf.exc_col_cont,
                              0, // Power savings Legacy - Not used anymore
                              0, // Power savings Legacy - Not used anymore
                              port_feature_support(cap, port_status.cap, PORT_CAP_TRI_SPEED_DUAL_ANY_FIBER),
                              conf.dual_media_fiber_speed);
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

// handler_stat_port()
static cyg_int32 handler_stat_port(CYG_HTTPD_STATE* p)
{
    vtss_isid_t          sid = web_retrieve_request_sid(p); /* Includes USID = ISID */
    vtss_port_no_t       iport = VTSS_PORT_NO_START;
    vtss_port_counters_t counters;
    int ct, val;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_PORT))
        return -1;
#endif

    cyg_httpd_start_chunked("html");

    if(!VTSS_ISID_LEGAL(sid) ||
       !msg_switch_exists(sid))
        goto out;             /* Most likely stack error - bail out */

    if(cyg_httpd_form_varable_int(p, "port", &val))
        iport = uport2iport(val);
    if(iport > VTSS_PORT_NO_START + port_isid_port_count(sid))
        iport = VTSS_PORT_NO_START;

    if((cyg_httpd_form_varable_find(p, "clear") != NULL)) {          /* Clear? */
        if(port_mgmt_counters_clear(sid, iport) != VTSS_OK)
            goto out;         /* Most likely stack error - bail out */
        memset(&counters, 0, sizeof(counters)); /* Cheating a little... */
    } else
        /* Normal read */
        if (port_mgmt_counters_get(sid, iport, &counters) != VTSS_OK)
            goto out;         /* Most likely stack error - bail out */

    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u,%u,%d/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu",
                  iport2uport(iport),
                  port_isid_port_count(sid),
                  1,            /* STD counters */
                  /* 1 */ counters.rmon.rx_etherStatsPkts,
                  /* 2 */ counters.rmon.tx_etherStatsPkts,
                  /* 3 */ counters.rmon.rx_etherStatsOctets,
                  /* 4 */ counters.rmon.tx_etherStatsOctets,
                  /* 5 */ counters.rmon.rx_etherStatsDropEvents,
                  /* 6 */ counters.rmon.tx_etherStatsDropEvents,
                  /* 7 */ counters.if_group.ifInErrors,
                  /* 8 */ counters.if_group.ifOutErrors);
    cyg_httpd_write_chunked(p->outbuffer, ct);
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%llu/%llu/%llu/%llu/%llu/%llu",
                  2,            /* CAST counters */
                  /* 1 */ counters.if_group.ifInUcastPkts,
                  /* 2 */ counters.if_group.ifOutUcastPkts,
                  /* 3 */ counters.rmon.rx_etherStatsMulticastPkts,
                  /* 4 */ counters.rmon.tx_etherStatsMulticastPkts,
                  /* 5 */ counters.rmon.rx_etherStatsBroadcastPkts,
                  /* 6 */ counters.rmon.tx_etherStatsBroadcastPkts);
    cyg_httpd_write_chunked(p->outbuffer, ct);
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%llu/%llu",
                  3,            /* PAUSE counters */
                  /* 1 */ counters.ethernet_like.dot3InPauseFrames,
                  /* 2 */ counters.ethernet_like.dot3OutPauseFrames);
    cyg_httpd_write_chunked(p->outbuffer, ct);
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu",
                  4,            /* RMON ADV counters */
                  /* 1 */ counters.rmon.rx_etherStatsPkts64Octets,
                  /* 2 */ counters.rmon.tx_etherStatsPkts64Octets,
                  /* 3 */ counters.rmon.rx_etherStatsPkts65to127Octets,
                  /* 4 */ counters.rmon.tx_etherStatsPkts65to127Octets,
                  /* 5 */ counters.rmon.rx_etherStatsPkts128to255Octets,
                  /* 6 */ counters.rmon.tx_etherStatsPkts128to255Octets,
                  /* 7 */ counters.rmon.rx_etherStatsPkts256to511Octets,
                  /* 8 */ counters.rmon.tx_etherStatsPkts256to511Octets,
                  /* 9 */ counters.rmon.rx_etherStatsPkts512to1023Octets,
                  /* 10 */ counters.rmon.tx_etherStatsPkts512to1023Octets,
                  /* 11 */ counters.rmon.rx_etherStatsPkts1024to1518Octets,
                  /* 12 */ counters.rmon.tx_etherStatsPkts1024to1518Octets,
                  /* 13 */ counters.rmon.rx_etherStatsCRCAlignErrors,
                  /* 14 */ counters.if_group.ifOutErrors,
                  /* 15 */ counters.rmon.rx_etherStatsUndersizePkts,
                  /* 16 */ counters.rmon.rx_etherStatsOversizePkts,
                  /* 17 */ counters.rmon.rx_etherStatsFragments,
                  /* 18 */ counters.rmon.rx_etherStatsJabbers);
    cyg_httpd_write_chunked(p->outbuffer, ct);
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%llu/%llu",
                  5,            /* JUMBO counters */
                  /* 1 */ counters.rmon.rx_etherStatsPkts1519toMaxOctets,
                  /* 2 */ counters.rmon.tx_etherStatsPkts1519toMaxOctets);
    cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_FEATURE_PORT_CNT_ETHER_LIKE)
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%llu/%llu/%llu/%llu/%llu/%llu",
                  6,            /* ETHER_LIKE counters */
                  /* 1 */ counters.rmon.rx_etherStatsCRCAlignErrors,
                  /* 2 */ counters.ethernet_like.dot3StatsLateCollisions,
                  /* 3 */ counters.ethernet_like.dot3StatsSymbolErrors,
                  /* 4 */ counters.ethernet_like.dot3StatsExcessiveCollisions,
                  /* 5 */ counters.rmon.rx_etherStatsUndersizePkts,
                  /* 6 */ counters.ethernet_like.dot3StatsCarrierSenseErrors);
    cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_FEATURE_PORT_CNT_ETHER_LIKE */

#ifdef VTSS_ARCH_LUTON28
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu",
                  7,
                  counters.prop.rx_prio[0],
                  counters.prop.tx_prio[0],
                  counters.prop.rx_prio[1],
                  counters.prop.tx_prio[1],
                  counters.prop.rx_prio[2],
                  counters.prop.tx_prio[2],
                  counters.prop.rx_prio[3],
                  counters.prop.tx_prio[3]);
#else
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu",
                  7,
                  counters.prop.rx_prio[0],
                  counters.prop.tx_prio[0],
                  counters.prop.rx_prio[1],
                  counters.prop.tx_prio[1],
                  counters.prop.rx_prio[2],
                  counters.prop.tx_prio[2],
                  counters.prop.rx_prio[3],
                  counters.prop.tx_prio[3],
                  counters.prop.rx_prio[4],
                  counters.prop.tx_prio[4],
                  counters.prop.rx_prio[5],
                  counters.prop.tx_prio[5],
                  counters.prop.rx_prio[6],
                  counters.prop.tx_prio[6],
                  counters.prop.rx_prio[7],
                  counters.prop.tx_prio[7]);
#endif
    cyg_httpd_write_chunked(p->outbuffer, ct);

#if defined(VTSS_FEATURE_PORT_CNT_BRIDGE)
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|%d/%llu",
                  8,
                  counters.bridge.dot1dTpPortInDiscards);
    cyg_httpd_write_chunked(p->outbuffer, ct);
#endif /* VTSS_FEATURE_PORT_CNT_BRIDGE */

 out:
    cyg_httpd_end_chunked();
    return -1; // Do not further search the file system.
}

static notice_callback_t port_web_notice_callback = NULL;

void port_web_set_notice_callback(notice_callback_t new_callback_function)
{
    if (port_web_notice_callback && new_callback_function)
        T_E("Notice callback being overridden (0x08%x -> 0x%08x)",
            (int)port_web_notice_callback, (int)new_callback_function);

    port_web_notice_callback = new_callback_function;
}

notice_callback_t port_web_get_notice_callback(void)
{
    return port_web_notice_callback;
}

/*
 * Toplevel portstate handler
 */
static cyg_int32 handler_stat_portstate(CYG_HTTPD_STATE* p)
{
    int cnt;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_PORT))
        return -1;
#endif

    cyg_httpd_start_chunked("html");

#if VTSS_SWITCH_STACKABLE
    vtss_usid_t usid;
    for(usid = VTSS_USID_START; usid < VTSS_USID_END; usid++) {
        vtss_isid_t isid = topo_usid2isid(usid);
        if(msg_switch_exists(isid))
            stat_portstate_switch(p, usid, isid);
    }
#else /* VTSS_SWITCH_STACKABLE */
    stat_portstate_switch(p, VTSS_USID_START, VTSS_ISID_START);
#endif /* VTSS_SWITCH_STACKABLE */

    if (port_web_notice_callback)
    {
        p->outbuffer[0] = ';'; // field separator
        cnt = port_web_notice_callback(p->outbuffer + 1, sizeof(p->outbuffer) - 1);
        if (cnt > 0)
            cyg_httpd_write_chunked(p->outbuffer, cnt);
    }

    cyg_httpd_end_chunked();

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  Module Filter CSS routine                                               */
/****************************************************************************/
static size_t port_lib_filter_css(char **base_ptr, char **cur_ptr, size_t *length)
{
    port_isid_info_t info;
    port_cap_t       cap;
    BOOL             power = 0;

    cap = (port_isid_info_get(VTSS_ISID_LOCAL, &info) == VTSS_RC_OK ? info.cap : 0);
#ifdef VTSS_SW_OPTION_PHY_POWER_CONTROL
    if (cap & PORT_CAP_1G_PHY)
        power = 1;
#endif /* VTSS_SW_OPTION_PHY_POWER_CONTROL */

    char buff[PORT_WEB_BUF_LEN];
    (void) snprintf(buff, PORT_WEB_BUF_LEN, 
                    "%s%s%s%s%s%s",
                    power ? "" : ".PHY_PWR_CTRL { display: none; }\r\n",
                    cap & PORT_CAP_FLOW_CTRL ? "" : ".PORT_FLOW_CTRL { display: none; }\r\n",
                    cap & PORT_CAP_HDX ? "" : ".PORT_EXC_CONT { display: none; }\r\n",
                    cap &  PORT_CAP_ANY_FIBER ? "" : ".PORT_FIBER_CTRL { display: none; }\r\n",
                    cap &  PORT_CAP_DUAL_COPPER ? ".PORT_1000X_AMS_FIBER_PREFERRED { display: none; }\r\n" : ".PORT_1000X_AMS_COPPRT_PREFERRED { display: none; }\r\n",
                    cap &  PORT_CAP_DUAL_COPPER_100FX ? ".PORT_100FX_AMS_FIBER_PREFERRED { display: none; }\r\n" : ".PORT_100FX_AMS_COPPER_PREFERRED { display: none; }\r\n");
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}
/****************************************************************************/
/*  Filter CSS table entry                                                  */
/****************************************************************************/
web_lib_filter_css_tab_entry(port_lib_filter_css);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/
/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

#if VTSS_UI_OPT_VERIPHY == 1
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_veriphy, "/config/veriphy", handler_config_veriphy);
#endif /* VTSS_UI_OPT_VERIPHY == 1 */
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_ports, "/stat/ports", handler_stat_ports);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_ports, "/config/ports", handler_config_ports);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_port, "/stat/port", handler_stat_port);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_portstate, "/stat/portstate", handler_stat_portstate);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
