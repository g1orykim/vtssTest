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

#include <stdio.h>
#include "main.h"
#include "web_api.h"
#include "topo_api.h"
#include "msg_api.h"
#include "port_api.h"
#include "conf_api.h"
#ifdef VTSS_SW_OPTION_PRIV_LVL
#include "vtss_privilege_api.h"
#include "vtss_privilege_web_api.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_TOPO




#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_WEB
#include <vtss_trace_api.h>


static
const topo_switch_t *
stack_find_dev(topo_switch_list_t *tsl_p, vtss_mac_t *mac)
{
    cyg_uint32 i;
    for (i = 0; i < ARRSZ(tsl_p->ts); i++) {
        topo_switch_t *ts_p = &tsl_p->ts[i];
        if (ts_p->vld &&
            memcmp(&ts_p->mac_addr, &mac->addr, sizeof(mac->addr)) == 0) {
            return ts_p;
        }
    }
    return NULL;
}





typedef struct {
    vtss_port_no_t port_0;
    vtss_port_no_t port_1;
} web_stack_port_pair_t;

#define MAX_STACK_PORT_PAIRS 6
typedef struct {
    uint                  count;
    uint                  key;
    BOOL                  dirty;
    web_stack_port_pair_t pair[MAX_STACK_PORT_PAIRS];
} web_stack_port_conf_t;


static void web_stack_conf_get(vtss_isid_t isid, web_stack_port_conf_t *conf)
{
    web_stack_port_pair_t *pair;
    stack_config_t        stack_conf;
    port_isid_port_info_t info;
    u32                   port_count;
    vtss_port_no_t        iport, port_0, port_1;
    u8                    stack_group[VTSS_PORTS];
    BOOL                  two_chips = 0;

    memset(conf, 0, sizeof(*conf));
    if (topo_stack_config_get(isid, &stack_conf, &conf->dirty) == VTSS_OK) {
        
        port_0 = stack_conf.port_0;
        port_1 = stack_conf.port_1;
        if (port_0 > port_1) {
            stack_conf.port_0 = port_1;
            stack_conf.port_1 = port_0;
        }

        port_count = port_isid_port_count(isid);
        for (iport = 0; iport < port_count; iport++) {
            stack_group[iport] = 0;
            if (port_isid_port_info_get(isid, iport, &info) == VTSS_OK) {
                if (info.cap & PORT_CAP_STACKING) {
                    stack_group[iport] = (info.chip_no + 1);
                }
                if (info.chip_no) {
                    two_chips = 1;
                }
            }
        }
        for (port_0 = 0; port_0 < port_count; port_0++) {
            if (stack_group[port_0] == 0) {
                continue;
            }
            for (port_1 = (port_0 + 1); port_1 < port_count; port_1++) {
                if (stack_group[port_1] == 0 || (two_chips && stack_group[port_0] == stack_group[port_1]) || conf->count >= MAX_STACK_PORT_PAIRS) {
                    continue;
                }
                pair = &conf->pair[conf->count];
                pair->port_0 = port_0;
                pair->port_1 = port_1;
                if (port_0 == stack_conf.port_0 && port_1 == stack_conf.port_1) {
                    conf->key = conf->count;
                }
                conf->count++;
            }
        }
    }
}

static cyg_int32 handler_config_stack(CYG_HTTPD_STATE *p)
{
    topo_switch_list_t    *tsl_p = NULL;
    char                  prod_name[MSG_MAX_PRODUCT_NAME_LEN], prod_name_encoded[3 * (MSG_MAX_PRODUCT_NAME_LEN)];
    mac_addr_t            mac_addr;
    char                  macstr[sizeof "aa-bb-cc-dd-dd-ff"];
    int                   ct, i, j;
    BOOL                  cur_stack_enabled, conf_stack_enabled = FALSE;
    vtss_isid_t           isid, local_isid = VTSS_ISID_START;
    vtss_usid_t           local_usid;
    web_stack_port_conf_t conf;
    stack_config_t        stack_conf;
    vtss_rc               rc;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_TOPO)) {
        return -1;
    }
#endif

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (msg_switch_is_local(isid)) {
            local_isid = isid;
            break;
        }
    }

    local_usid = topo_isid2usid(local_isid);
    cur_stack_enabled = vtss_stacking_enabled();
    if (cur_stack_enabled && ((tsl_p = VTSS_MALLOC(sizeof(topo_switch_list_t))) == NULL || topo_switch_list_get(tsl_p) != VTSS_OK)) {
        goto end;
    }

    if (p->method == CYG_HTTPD_METHOD_POST) {
        char var_name[32];
        vtss_mac_t mac;
        memset(&mac, 0, sizeof(mac));

        
        ct = 0;
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (msg_switch_exists(isid)) {
                ct++;
            }
        }
        if (ct < 2) {
            conf_stack_enabled = (cyg_httpd_form_varable_string(p, "conf_stack_enabled", NULL) ? TRUE : FALSE);
        } else {
            
            conf_stack_enabled = TRUE;
        }

        for (isid = VTSS_ISID_START; ct < 2 && isid < VTSS_ISID_END; isid++) {
            if ((rc = topo_stack_config_get(isid, &stack_conf, &conf.dirty)) == VTSS_OK && conf_stack_enabled != stack_conf.stacking) {
                stack_conf.stacking = conf_stack_enabled;
                if ((rc = topo_stack_config_set(isid, &stack_conf)) != VTSS_OK) {
                    T_W("topo_stack_conf_set failed for isid %u: %s", isid, error_txt(rc));
                }
            }
        }

        for (i = 0; (tsl_p != NULL || cur_stack_enabled == FALSE) && i < ARRSZ(tsl_p->ts); i++) {
            const topo_switch_t *ts_p = NULL;
            sprintf(var_name, "mac_%d", i);
            if (cur_stack_enabled == FALSE || (cyg_httpd_form_variable_mac(p, var_name, &mac) && topo_switch_list_get(tsl_p) == VTSS_OK && (ts_p = stack_find_dev(tsl_p, &mac)) != NULL)) {
                misc_mac_txt(&mac.addr[0], macstr);
                sprintf(var_name, "del_%d", i);
                if (cur_stack_enabled && cyg_httpd_form_varable_find(p, var_name)) {
                    rc = topo_isid_delete(ts_p->isid);
                    T_D("Delete switch %s: %s", macstr, rc ? error_txt(rc) : "OK");
                } else {
                    int usid, prio, ports;
                    sprintf(var_name, "sid_%d", i);
                    if (! (cyg_httpd_form_varable_int(p, var_name, &usid) && VTSS_USID_LEGAL(usid))) {
                        continue;
                    }
                    isid = topo_usid2isid(usid);
                    if (cur_stack_enabled) {
                        if (usid != ts_p->usid && VTSS_ISID_LEGAL(isid)) {
                            
                            if ((rc = topo_isid_assign(isid, ts_p->mac_addr)) != VTSS_OK) {
                                
                                if ((rc == TOPO_ERROR_SID_IN_USE || rc == TOPO_ERROR_SWITCH_HAS_SID) && VTSS_USID_LEGAL(ts_p->usid)) {
                                    T_D("Assign of %d to %s failed, swap(%d,%d)", usid, macstr, usid, ts_p->usid);
                                    rc = topo_usid_swap(usid, ts_p->usid);
                                    isid = ts_p->isid;
                                }
                            } else {
                                T_D("Assigned SID %d to %s (was %d)", usid, macstr, ts_p->usid);
                            }
                            if (rc != VTSS_OK) {
                                T_W("Assigning SID %d to %s failed: %s", usid, macstr, error_txt(rc));
                            }
                        } else {
                            T_D("%s sid %d already set to %d", macstr, usid, ts_p->usid);
                        }
                        sprintf(var_name, "prio_%d", i);
                        if (cyg_httpd_form_varable_int(p, var_name, &prio) && VTSS_ISID_LEGAL(isid) && prio != ts_p->mst_elect_prio) {
                            rc = topo_parm_set(isid, TOPO_PARM_MST_ELECT_PRIO, prio);
                            T_D("Set Master priority SID %d MAC %s to %d (was %d): %s", usid, macstr, prio, ts_p->mst_elect_prio, rc ? error_txt(rc) : "OK");
                        }
                    }

                    
                    sprintf(var_name, "ports_%d", i);
                    if (VTSS_ISID_LEGAL(isid) && cyg_httpd_form_varable_int(p, var_name, &ports)) {
                        web_stack_conf_get(isid, &conf);
                        if (conf.count && ports < conf.count && ports != conf.key) {
                            stack_conf.stacking = conf_stack_enabled;
                            stack_conf.port_0 = conf.pair[ports].port_0;
                            stack_conf.port_1 = conf.pair[ports].port_1;
                            if ((rc = topo_stack_config_set(isid, &stack_conf)) != VTSS_OK) {
                                T_W("topo_stack_conf_set failed for isid %u: %s", isid, error_txt(rc));
                            }
                        }
                    }
                }
            }
        }

        if (cyg_httpd_form_varable_find(p, "reelect")) {
            T_D("Forcing master election");
            (void) topo_parm_set(0, TOPO_PARM_MST_TIME_IGNORE, 1);
        }
        VTSS_OS_MSLEEP(500);
        redirect(p, "/stack_config.htm");
    } else {
        cyg_httpd_start_chunked("html");

        
        
        

        
        for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
            if (msg_switch_is_local(isid) && topo_stack_config_get(isid, &stack_conf, &conf.dirty) == VTSS_OK) {
                conf_stack_enabled = stack_conf.stacking;
            }
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|%u|", cur_stack_enabled, conf_stack_enabled);
        cyg_httpd_write_chunked(p->outbuffer, ct);

        
        if (cur_stack_enabled) {
            for (i = 0; tsl_p != NULL && i < ARRSZ(tsl_p->ts); i++) {
                topo_switch_t *ts_p = &tsl_p->ts[i];
                if (ts_p->vld) {
                    misc_mac_txt(ts_p->mac_addr, macstr);
                    if (msg_switch_is_master()) {
                        if (VTSS_ISID_LEGAL(ts_p->isid)) {
                            msg_product_name_get(ts_p->isid, prod_name);
                        } else {
                            
                            
                            strcpy(prod_name, "N/A");
                        }
                    } else {
                        strcpy(prod_name, "-");
                    }
                    (void)cgi_escape(prod_name, prod_name_encoded);
                    int master = (memcmp(&ts_p->mac_addr, &tsl_p->mst_switch_mac_addr, sizeof(mac_addr_t)) == 0);
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d/%d/%d/%d/%s",
                                  ts_p->present,
                                  macstr,
                                  ts_p->usid,
                                  master,
                                  ts_p->mst_capable,
                                  ts_p->mst_elect_prio,
                                  prod_name_encoded);
                    cyg_httpd_write_chunked(p->outbuffer, ct);

                    web_stack_conf_get(ts_p->isid, &conf);
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u",
                                  conf.dirty, conf.count, conf.key);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                    for (j = 0; j < conf.count; j++) {
                        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                                      iport2uport(conf.pair[j].port_0),
                                      iport2uport(conf.pair[j].port_1));
                        cyg_httpd_write_chunked(p->outbuffer, ct);
                    }
                }
                if (i != ARRSZ(tsl_p->ts) - 1) {
                    cyg_httpd_write_chunked("|", 1);
                }
            }
        } else {
            
            msg_product_name_get(local_isid, prod_name);
            (void)cgi_escape(prod_name, prod_name_encoded);
            
            (void)conf_mgmt_mac_addr_get(mac_addr, 0);
            misc_mac_txt(mac_addr, macstr);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d/%d/%d/%d/%s",
                          TRUE, 
                          macstr,
                          topo_isid2usid(local_isid),
                          TRUE, 
                          TRUE, 
                          0,    
                          prod_name_encoded);
            cyg_httpd_write_chunked(p->outbuffer, ct);

            web_stack_conf_get(local_isid, &conf);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u/%u", conf.dirty, conf.count, conf.key);
            cyg_httpd_write_chunked(p->outbuffer, ct);
            for (j = 0; j < conf.count; j++) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "/%u/%u",
                              iport2uport(conf.pair[j].port_0),
                              iport2uport(conf.pair[j].port_1));
                cyg_httpd_write_chunked(p->outbuffer, ct);
            }

        }
        cyg_httpd_end_chunked();
    }

end:
    if (tsl_p) {
        VTSS_FREE(tsl_p);
    }
    return -1; 
}

static cyg_int32 handler_stat_topo(CYG_HTTPD_STATE *p)
{
    topo_switch_stat_t ts;
    topo_switch_list_t *tsl_p = NULL;
    char               macstr[sizeof "aa-bb-cc-dd-dd-ff"];
    int                ct, i, chip_idx, idx[2];
    BOOL               cur_stack_enabled, dirty;
    vtss_isid_t        isid, local_isid = VTSS_ISID_START;
    stack_config_t     stack_conf;
    char               prod_name[MSG_MAX_PRODUCT_NAME_LEN], prod_name_encoded[3 * (MSG_MAX_PRODUCT_NAME_LEN)];
    char               ver_str[MSG_MAX_VERSION_STRING_LEN], ver_str_encoded[3 * (MSG_MAX_VERSION_STRING_LEN)];

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_TOPO)) {
        return -1;
    }
#endif

    cyg_httpd_start_chunked("html");

    cur_stack_enabled = vtss_stacking_enabled();

    if (cur_stack_enabled && (topo_switch_stat_get(VTSS_ISID_LOCAL, &ts) != VTSS_OK || !(tsl_p = VTSS_MALLOC(sizeof(topo_switch_list_t))) || topo_switch_list_get(tsl_p) != VTSS_OK)) {
        goto end;
    }

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (msg_switch_is_local(isid)) {
            local_isid = isid;
            break;
        }
    }
    (void)topo_stack_config_get(local_isid, &stack_conf, &dirty);

    
    
    
    

    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%d|%d|",
                  cur_stack_enabled,
                  stack_conf.stacking,
                  dirty);
    cyg_httpd_write_chunked(p->outbuffer, ct);

    if (cur_stack_enabled) {
        misc_mac_txt(tsl_p->mst_switch_mac_addr, macstr);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%d|%s|%s|%s|",
                      ts.topology_type,
                      ts.switch_cnt,
                      misc_time2str(ts.topology_change_time),
                      macstr,
                      misc_time2str(tsl_p->mst_change_time)
                     );
    } else {
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|%d|%s|%s|%s|",
                      0,
                      0,
                      "0",
                      "0",
                      "0");
    }
    cyg_httpd_write_chunked(p->outbuffer, ct);

    
    if (port_no_stack(0) < port_no_stack(1)) {
        idx[0] = 0;
        idx[1] = 1;
    } else {
        idx[0] = 1;
        idx[1] = 0;
    }
    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u|%u|", iport2uport(port_no_stack(idx[0])), iport2uport(port_no_stack(idx[1])));
    cyg_httpd_write_chunked(p->outbuffer, ct);

    if (cur_stack_enabled == FALSE) {
        goto end;
    }

    
    for (i = 0; i < ARRSZ(tsl_p->ts); i++) {
        topo_switch_t *ts_p = &tsl_p->ts[i];
        topo_chip_t   *ts_c;
        if (ts_p->vld && ts_p->present) {
            for (chip_idx = 0; chip_idx < ts_p->chip_cnt; chip_idx++) {
                ts_c = &ts_p->chip[chip_idx];
                misc_mac_txt(ts_p->mac_addr, macstr);

                if (VTSS_ISID_LEGAL(ts_p->isid)) {
                    msg_product_name_get(ts_p->isid, prod_name);
                    msg_version_string_get(ts_p->isid, ver_str);
                } else {
                    
                    
                    strcpy(prod_name, "N/A");
                    strcpy(ver_str, "N/A");
                }
                (void)cgi_escape(prod_name, prod_name_encoded);
                (void)cgi_escape(ver_str,   ver_str_encoded);
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer),
                              "%d/%s/%d/%s/%s/%s/%s/%s/%s/%s/",
                              chip_idx, macstr, ts_p->usid,
                              vtss_sprout_port_mask_to_str(ts_c->ups_port_mask[0] | ts_c->ups_port_mask[1]),
                              ts_c->dist_str[idx[0]],
                              ts_c->dist_str[idx[1]],
                              topo_stack_port_fwd_mode_to_str(ts_c->stack_port_fwd_mode[idx[0]]),
                              topo_stack_port_fwd_mode_to_str(ts_c->stack_port_fwd_mode[idx[1]]),
                              prod_name_encoded,
                              ver_str_encoded);
                cyg_httpd_write_chunked(p->outbuffer, ct);

                
                if (ts_p->mst_capable) {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d/%s/%d",
                                  ts_p->mst_elect_prio,
                                  ts_p->mst_time ? misc_time2interval(ts_p->mst_time) : "-",
                                  ts_p->mst_time_ignore);
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                } else {
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "-/-/2");
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
                cyg_httpd_write_chunked("|", 1);
            }
        }
    }

end:
    cyg_httpd_end_chunked();
    if (tsl_p) {
        VTSS_FREE(tsl_p);
    }
    return -1; 
}

static cyg_int32 handler_stat_sid(CYG_HTTPD_STATE *p)
{
    topo_switch_list_t *tsl_p;
    int ct, i;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_STAT_TYPE, VTSS_MODULE_ID_TOPO)) {
        return -1;
    }
#endif

    if (!(tsl_p = VTSS_MALLOC(sizeof(topo_switch_list_t)))) {
        T_W("VTSS_MALLOC() failed, size=%zu", sizeof(topo_switch_list_t));
        return -1;
    }

    cyg_httpd_start_chunked("html");

    if (msg_switch_is_master()) {
        if (topo_switch_list_get(tsl_p) == VTSS_OK) {
            for (i = 0; (i < ARRSZ(tsl_p->ts)) && tsl_p->ts[i].vld; i++) {
                topo_switch_t *ts_p = &tsl_p->ts[i];
                port_isid_info_t pinfo;
                if (ts_p->vld && ts_p->present && ts_p->mst_capable &&
                    (port_isid_info_get(ts_p->isid, &pinfo) == VTSS_OK)) {
                    const char *flags = (memcmp(tsl_p->mst_switch_mac_addr, ts_p->mac_addr, sizeof(mac_addr_t)) == 0) ? "M"  : "";
                    ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u:%s:%u:%u:%u,",
                                  ts_p->usid, flags, pinfo.port_count,
                                  iport2uport(pinfo.stack_port_0), iport2uport(pinfo.stack_port_1));
                    cyg_httpd_write_chunked(p->outbuffer, ct);
                }
            }
        } else {
            
            vtss_isid_t           isid = VTSS_ISID_START;
            vtss_usid_t           usid;
            for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
                if (msg_switch_is_local(isid)) {
                    break;
                }
            }
            usid = topo_isid2usid(isid);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d:M:%u:0:0",
                          usid, port_isid_port_count(VTSS_ISID_LOCAL));
            cyg_httpd_write_chunked(p->outbuffer, ct);
        }
    }

    cyg_httpd_end_chunked();

    VTSS_FREE(tsl_p);

    return -1; 
}





CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_stack, "/config/stack", handler_config_stack);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_topo, "/stat/topo", handler_stat_topo);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_sid, "/stat/sid", handler_stat_sid);




