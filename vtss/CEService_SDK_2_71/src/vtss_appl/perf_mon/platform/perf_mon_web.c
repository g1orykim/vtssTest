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
#include "perf_mon_api.h"

#include "evc_api.h"
#include "../../mep/base/vtss_mep.h"

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

#define PM_WEB_BUF_LEN 256

enum {
    PM_INTERVAL_TYPE_LM,
    PM_INTERVAL_TYPE_DM,
    PM_INTERVAL_TYPE_EVC,
    PM_INTERVAL_TYPE_ECE,
    PM_INTERVAL_TYPE_ALL
};

/****************************************************************************/
/*  local API  functions                                                    */
/****************************************************************************/

/****************************************************************************/
// Convert from internal to user.
/****************************************************************************/
static inline vtss_evc_id_t evc_id_i2u(vtss_evc_id_t evc_id)
{
    return ((evc_id + 1) > EVC_ID_COUNT ? EVC_ID_COUNT :  evc_id + 1);
}

/****************************************************************************/
/*  Module JS lib config routine                                            */
/****************************************************************************/

static size_t interval_info_lib_config_js(char **base_ptr, char **cur_ptr, size_t *length)
{
    char buff[PM_WEB_BUF_LEN];

    (void) snprintf(buff, PM_WEB_BUF_LEN,
                    "var configPerfMonLMInstanceMax = %u;\n"
                    "var configPerfMonDMInstanceMax = %u;\n"
                    "var configPerfMonEVCInstanceMax = %u;\n"
                    "var configPerfMonECEInstanceMax = %u;\n"
                    , PM_LM_DATA_SET_LIMIT
                    , PM_DM_DATA_SET_LIMIT
                    , PM_EVC_DATA_SET_LIMIT
                    , PM_ECE_DATA_SET_LIMIT
                   );
    return webCommonBufferHandler(base_ptr, cur_ptr, length, buff);
}

/****************************************************************************/
/*  Web Handler  functions                                                  */
/****************************************************************************/
/* PM Session and Storage Configuration */
static cyg_int32 handler_config_perf_mon_conf_table(CYG_HTTPD_STATE *p)
{
    int                     ct;
    perf_mon_conf_t         global_conf;
    char                    search_str[64];
    int                     mode = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PERF_MON)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        memset(&global_conf, 0x0, sizeof(global_conf));
        perf_mon_conf_get(&global_conf);

        /* Loss Measurement */
        sprintf(search_str, "enable_session_0");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            global_conf.lm_session_mode = TRUE;
        } else {
            global_conf.lm_session_mode = FALSE;
        }

        sprintf(search_str, "enable_storage_0");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            global_conf.lm_storage_mode = TRUE;
        } else {
            global_conf.lm_storage_mode = FALSE;
        }

        sprintf(search_str, "enable_interval_0");
        (void) cyg_httpd_form_varable_int(p, search_str, &mode);
        global_conf.lm_interval = mode;

        T_D("%d,%d,%d", global_conf.lm_session_mode, global_conf.lm_storage_mode, global_conf.lm_interval);

        /* Delay Measurement */
        sprintf(search_str, "enable_session_1");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            global_conf.dm_session_mode = TRUE;
        } else {
            global_conf.dm_session_mode = FALSE;
        }

        sprintf(search_str, "enable_storage_1");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            global_conf.dm_storage_mode = TRUE;
        } else {
            global_conf.dm_storage_mode = FALSE;
        }

        sprintf(search_str, "enable_interval_1");
        (void) cyg_httpd_form_varable_int(p, search_str, &mode);
        global_conf.dm_interval = mode;

        T_D("%d,%d,%d", global_conf.dm_session_mode, global_conf.dm_storage_mode, global_conf.dm_interval);

        /* EVC */
        sprintf(search_str, "enable_session_2");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            global_conf.evc_session_mode = TRUE;
        } else {
            global_conf.evc_session_mode = FALSE;
        }

        sprintf(search_str, "enable_storage_2");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            global_conf.evc_storage_mode = TRUE;
        } else {
            global_conf.evc_storage_mode = FALSE;
        }

        sprintf(search_str, "enable_interval_2");
        (void) cyg_httpd_form_varable_int(p, search_str, &mode);
        global_conf.evc_interval = mode;

        T_D("%d,%d,%d", global_conf.evc_session_mode, global_conf.evc_storage_mode, global_conf.evc_interval);

        /* ECE */
        sprintf(search_str, "enable_session_3");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            global_conf.ece_session_mode = TRUE;
        } else {
            global_conf.ece_session_mode = FALSE;
        }

        sprintf(search_str, "enable_storage_3");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            global_conf.ece_storage_mode = TRUE;
        } else {
            global_conf.ece_storage_mode = FALSE;
        }

        sprintf(search_str, "enable_interval_3");
        (void) cyg_httpd_form_varable_int(p, search_str, &mode);
        global_conf.ece_interval = mode;

        T_D("%d,%d,%d", global_conf.ece_session_mode, global_conf.ece_storage_mode, global_conf.ece_interval);

        perf_mon_conf_set(&global_conf);
        redirect(p, "/perf_mon_conf.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        memset(&global_conf, 0x0, sizeof(global_conf));
        perf_mon_conf_get(&global_conf);

        /* Loss Measurement */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "Loss Measurement/%d/%d/%d|",
                      global_conf.lm_session_mode,
                      global_conf.lm_storage_mode,
                      global_conf.lm_interval);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* Delay Measurement */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "Delay Measurement/%d/%d/%d|",
                      global_conf.dm_session_mode,
                      global_conf.dm_storage_mode,
                      global_conf.dm_interval);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* EVC */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "EVC/%d/%d/%d|",
                      global_conf.evc_session_mode,
                      global_conf.evc_storage_mode,
                      global_conf.evc_interval);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* ECE */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "ECE/%d/%d/%d",
                      global_conf.ece_session_mode,
                      global_conf.ece_storage_mode,
                      global_conf.ece_interval);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/* PM Transfer Configuration */
/*lint -e{502} */
static cyg_int32 handler_config_perf_mon_transfer_table(CYG_HTTPD_STATE *p)
{
    int                     ct;
    perf_mon_conf_t         global_conf;
    char                    search_str[64];
    int                     mode = 0;
    char                    encoded_string[3 * 64];
    int                     i, val;
    const char              *id;
    size_t                  nlen;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PERF_MON)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        memset(&global_conf, 0x0, sizeof(global_conf));
        perf_mon_conf_get(&global_conf);

        /* transfer mode */
        sprintf(search_str, "pm_gmode");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &mode);
        global_conf.transfer_mode = mode;

        /* scheduled hours */
        sprintf(search_str, "pm_hours");
        id = cyg_httpd_form_varable_string(p, search_str, &nlen);
        T_D("id = %s, nlen = %d", id, nlen);
        if (id != NULL && nlen > 0) {
            val = atoi(id);
            VTSS_PM_BF_SET(global_conf.transfer_scheduled_hours, val, TRUE);

            for (i = 0; i < 24; ++i) {
                if (i == val) {
                    continue;
                }
                sprintf(search_str, "pm_hours=%d", i);
                if (strstr(id, search_str) != NULL) {
                    VTSS_PM_BF_SET(global_conf.transfer_scheduled_hours, i, TRUE);
                } else {
                    VTSS_PM_BF_SET(global_conf.transfer_scheduled_hours, i, FALSE);
                }
            }
        }

        /* scheduled minutes */
        sprintf(search_str, "pm_minutes_0");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            VTSS_PM_BF_SET(global_conf.transfer_scheduled_minutes, 0, TRUE);
        } else {
            VTSS_PM_BF_SET(global_conf.transfer_scheduled_minutes, 0, FALSE);
        }

        sprintf(search_str, "pm_minutes_1");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            VTSS_PM_BF_SET(global_conf.transfer_scheduled_minutes, 1, TRUE);
        } else {
            VTSS_PM_BF_SET(global_conf.transfer_scheduled_minutes, 1, FALSE);
        }

        sprintf(search_str, "pm_minutes_2");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            VTSS_PM_BF_SET(global_conf.transfer_scheduled_minutes, 2, TRUE);
        } else {
            VTSS_PM_BF_SET(global_conf.transfer_scheduled_minutes, 2, FALSE);
        }

        sprintf(search_str, "pm_minutes_3");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            VTSS_PM_BF_SET(global_conf.transfer_scheduled_minutes, 3, TRUE);
        } else {
            VTSS_PM_BF_SET(global_conf.transfer_scheduled_minutes, 3, FALSE);
        }

        /* scheduled offset */
        sprintf(search_str, "pm_soffset");
        (void) cyg_httpd_form_varable_int(p, search_str, &mode);
        global_conf.transfer_scheduled_offset = mode;

        /* random offset */
        sprintf(search_str, "pm_roffset");
        (void) cyg_httpd_form_varable_int(p, search_str, &mode);
        global_conf.transfer_scheduled_random_offset = mode;

        /* url */
        sprintf(search_str, "pm_url");
        id = cyg_httpd_form_varable_string(p, search_str, &nlen);
        T_D("id = %s, nlen = %d", id, nlen);

        memset(global_conf.transfer_server_url, 0x0, sizeof(global_conf.transfer_server_url));
        if (id != NULL && nlen > 0) {
            (void) cgi_unescape(id, global_conf.transfer_server_url, nlen, sizeof(global_conf.transfer_server_url));
        }

        /* interval mode */
        sprintf(search_str, "pm_mode");
        id = cyg_httpd_form_varable_string(p, search_str, &nlen);
        T_D("id = %s, nlen = %d", id, nlen);

        if (id != NULL && nlen > 0) {
            if (memcmp(id, "pm_mode_0", nlen) == 0) {
                VTSS_PM_BF_SET(global_conf.transfer_interval_mode, 0, TRUE);
            } else {
                VTSS_PM_BF_SET(global_conf.transfer_interval_mode, 0, FALSE);
            }

            if (memcmp(id, "pm_mode_1", nlen) == 0) {
                VTSS_PM_BF_SET(global_conf.transfer_interval_mode, 1, TRUE);
            } else {
                VTSS_PM_BF_SET(global_conf.transfer_interval_mode, 1, FALSE);
            }

            if (memcmp(id, "pm_mode_2", nlen) == 0) {
                VTSS_PM_BF_SET(global_conf.transfer_interval_mode, 2, TRUE);
            } else {
                VTSS_PM_BF_SET(global_conf.transfer_interval_mode, 2, FALSE);
            }
        }

        /* number of intervals */
        sprintf(search_str, "pm_interval_num");
        (void) cyg_httpd_form_varable_int(p, search_str, &mode);
        global_conf.transfer_interval_num = mode;

        /* options */
        sprintf(search_str, "pm_options");
        if (cyg_httpd_form_varable_find(p, search_str)) {
            global_conf.transfer_incompleted = TRUE;
        } else {
            global_conf.transfer_incompleted = FALSE;
        }

        perf_mon_conf_set(&global_conf);
        redirect(p, "/perf_mon_transfer.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        memset(&global_conf, 0x0, sizeof(global_conf));
        perf_mon_conf_get(&global_conf);

        /* Transfer mode */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|",
                      global_conf.transfer_mode);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "1|");
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* Scheduled hours */
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|",
        //              global_conf.transfer_scheduled_hours);
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);
        for (i = 0; i < 24; ++i) {
            if (VTSS_PM_BF_GET(global_conf.transfer_scheduled_hours, i)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "1");
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0");
            }
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            if ( i < 23 ) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0,0,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0|");
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* Scheduled minutes */
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|",
        //              global_conf.transfer_scheduled_minutes);
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);
        for (i = 0; i < 4; ++i) {
            if (VTSS_PM_BF_GET(global_conf.transfer_scheduled_minutes, i)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "1");
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0");
            }
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            if ( i < 3 ) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0,1,0,1|");
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* Scheduled offset */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|",
                      global_conf.transfer_scheduled_offset);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "22|");
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* Random offset */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|",
                      global_conf.transfer_scheduled_random_offset);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "33|");
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* Server URL */
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|",
        //              global_conf.transfer_server_url);
        ct = cgi_escape(global_conf.transfer_server_url, encoded_string);
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s|", encoded_string);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* Interval mode */
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|",
        //              global_conf.transfer_interval_mode);
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);
        for (i = 0; i < 3; ++i) {
            if (VTSS_PM_BF_GET(global_conf.transfer_interval_mode, i)) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "1");
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "0");
            }
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            if ( i < 2 ) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), ",");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
        }
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "1,0,0|");
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* Interval num */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d|",
                      global_conf.transfer_interval_num);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "4|");
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* Options */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%d",
                      global_conf.transfer_incompleted);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);
        //ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "1");
        //(void)cyg_httpd_write_chunked(p->outbuffer, ct);

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/* Perf Mon LM Statistics */
static cyg_int32 handler_stat_perf_mon_lm_table(CYG_HTTPD_STATE *p)
{
    int                         ct;
    BOOL                        status = FALSE;
    vtss_perf_mon_lm_info_t     data[PM_LM_DATA_SET_LIMIT];
    u32                         idx;

    char                        search_str[64];
    u32                         ctrl_value = 0;
    u32                         interval_value = 0;
    u32                         instance_value = 0;
    int                         debug_counter = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PERF_MON)) {
        return -1;
    }
#endif

    memset(data, 0x0, PM_LM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_lm_info_t));

    if (p->method == CYG_HTTPD_METHOD_POST) {

        redirect(p, "/perf_mon_lm_statistics.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */

        // Format: [ctrl_flag]
        // 0:init 1:refresh 2:firstPage 3:previousPage 4:nextPage 5:lastPage 6:deleteall

        sprintf(search_str, "ctrl_flag");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &ctrl_value);
        T_D("GET ctrl_flag = %d", ctrl_value);

        sprintf(search_str, "interval_id");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &interval_value);
        T_D("GET interval_id = %d", interval_value);

        sprintf(search_str, "instance_id");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &instance_value);
        T_D("GET instance_id = %d", instance_value);

        (void)cyg_httpd_start_chunked("html");

        // Format: [selected_interval_id];[selected_instance_id];
        //         [interval_id]/[instance_id]/[port]/
        //         [priority]/[rate]/
        //         [tx_count]/[rx_count]/[near_end_count]/[near_end_ratio]/[far_end_count]/[far_end_ratio]/
        //         [domain]/[direction]/[level]/[flow]/[vid]/[mep_id]/[mac]/[peer_mep_id]/[peer_mac]|...

        switch (ctrl_value) {
        case 6: // 6:deleteall
            /* delete all data set */
            status = vtss_perf_mon_lm_data_clear();

            /* get last status */
            status = vtss_perf_mon_lm_data_get_last(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 0: // 0:init
        case 5: // 5:lastPage
            /* get last status */
            status = vtss_perf_mon_lm_data_get_last(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 1: // 1:refresh
            /* get status */
            status = vtss_perf_mon_lm_data_get_first(&data[0]);

            /* selected_interval_id */
            if (interval_value == 0) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            /* selected_instance_id */
            if (instance_value == 0) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", instance_value);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            break;
        case 2: // 2:firstPage
            /* get first status */
            status = vtss_perf_mon_lm_data_get_first(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 3: // 3:previousPage
            /* get previous status */
            data[0].measurement_interval_id = interval_value;
            status = vtss_perf_mon_lm_data_get_previous(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                status = vtss_perf_mon_lm_data_get(&data[0]);
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 4: // 4:nextPage
            /* get next status */
            data[0].measurement_interval_id = interval_value;
            status = vtss_perf_mon_lm_data_get_next(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                status = vtss_perf_mon_lm_data_get(&data[0]);
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        }

        while (status) {
            // error handler
            debug_counter++;
            if (debug_counter == PM_DEBUG_LM_MAX_LOOP) {
                T_W("Exit max loop!!\n");
                break;
            }
            for (idx = 0; idx < PM_LM_DATA_SET_LIMIT; idx++) {

                if (data[idx].measurement_interval_id == 0) {
                    break;
                }

                if (interval_value != 0) {
                    if (interval_value != data[idx].measurement_interval_id) {
                        continue;
                    }
                }

                if (instance_value != 0) {
                    if (instance_value != evc_id_i2u(data[idx].mep_instance)) {
                        continue;
                    }
                }

                /* interval_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].measurement_interval_id);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* instance_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", evc_id_i2u(data[idx].mep_instance));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* port */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", iport2uport(data[idx].mep_residence_port));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* priority */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].tx_priority);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* rate */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", vtss_mep_period_to_string(data[idx].tx_rate));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* tx_count */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].tx_cnt);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* rx_count */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].rx_cnt);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* near_end_count */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].near_end_loss_cnt);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* near_end_ratio */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].near_end_frame_loss_ratio);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* far_end_count */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].far_end_loss_cnt);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* far_end_ratio */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].far_end_loss_ratio);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* domain */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", vtss_mep_domain_to_string(data[idx].mep_domain));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* direction */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", vtss_mep_direction_to_string(data[idx].mep_direction));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* level */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].mep_level);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* flow */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", (data[idx].mep_domain != VTSS_MEP_MGMT_VLAN) ? evc_id_i2u(data[idx].mep_flow) : data[idx].mep_flow);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* vid */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].mep_vlan);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* mep_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].mep_id);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* mac */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%02x-%02x-%02x-%02x-%02x-%02x/",
                              data[idx].mep_mac[0],
                              data[idx].mep_mac[1],
                              data[idx].mep_mac[2],
                              data[idx].mep_mac[3],
                              data[idx].mep_mac[4],
                              data[idx].mep_mac[5]);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* peer_mep_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].mep_peer_id);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* peer_mac */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%02x-%02x-%02x-%02x-%02x-%02x",
                              data[idx].mep_peer_mac[0],
                              data[idx].mep_peer_mac[1],
                              data[idx].mep_peer_mac[2],
                              data[idx].mep_peer_mac[3],
                              data[idx].mep_peer_mac[4],
                              data[idx].mep_peer_mac[5]);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            status = vtss_perf_mon_lm_data_get_next(&data[0]);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/* Perf Mon DM Statistics */
static cyg_int32 handler_stat_perf_mon_dm_table(CYG_HTTPD_STATE *p)
{
    int                         ct;
    BOOL                        status = FALSE;
    vtss_perf_mon_dm_info_t     data[PM_DM_DATA_SET_LIMIT];
    u32                         idx;

    char                        search_str[64];
    u32                         ctrl_value = 0;
    u32                         interval_value = 0;
    u32                         instance_value = 0;
    int                         debug_counter = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PERF_MON)) {
        return -1;
    }
#endif

    memset(data, 0x0, PM_DM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_dm_info_t));

    if (p->method == CYG_HTTPD_METHOD_POST) {

        redirect(p, "/perf_mon_dm_statistics.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */

        // Format: [ctrl_flag]
        // 0:init 1:refresh 2:firstPage 3:previousPage 4:nextPage 5:lastPage 6:deleteall

        sprintf(search_str, "ctrl_flag");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &ctrl_value);
        T_D("GET ctrl_flag = %d", ctrl_value);

        sprintf(search_str, "interval_id");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &interval_value);
        T_D("GET interval_id = %d", interval_value);

        sprintf(search_str, "instance_id");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &instance_value);
        T_D("GET instance_id = %d", instance_value);

        (void)cyg_httpd_start_chunked("html");

        // Format: [selected_interval_id];[selected_instance_id];
        //         [interval_id]/[instance_id]/
        //         [port]/[priority]/[rate]/[unit]/
        //         [tx_count]/[rx_count]/
        //         [oneway_F2N_average]/[oneway_F2N_variation]/[oneway_F2N_min]/[oneway_F2N_max]/
        //         [oneway_N2F_average]/[oneway_N2F_variation]/[oneway_N2F_min]/[oneway_N2F_max]/
        //         [twoway_F2N_average]/[twoway_F2N_variation]/[twoway_F2N_min]/[twoway_F2N_max]/
        //         [domain]/[direction]/[level]/[flow]/[vid]/[mep_id]/[mac]/[peer_mep_id]/[peer_mac]|...

        switch (ctrl_value) {
        case 6: // 6:deleteall
            /* delete all data set */
            status = vtss_perf_mon_dm_data_clear();

            /* get last status */
            status = vtss_perf_mon_dm_data_get_last(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 0: // 0:init
        case 5: // 5:lastPage
            /* get last status */
            status = vtss_perf_mon_dm_data_get_last(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 1: // 1:refresh
            /* get status */
            status = vtss_perf_mon_dm_data_get_first(&data[0]);

            /* selected_interval_id */
            if (interval_value == 0) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            /* selected_instance_id */
            if (instance_value == 0) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", instance_value);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            break;
        case 2: // 2:firstPage
            /* get first status */
            status = vtss_perf_mon_dm_data_get_first(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 3: // 3:previousPage
            /* get previous status */
            data[0].measurement_interval_id = interval_value;
            status = vtss_perf_mon_dm_data_get_previous(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                status = vtss_perf_mon_dm_data_get(&data[0]);
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 4: // 4:nextPage
            /* get next status */
            data[0].measurement_interval_id = interval_value;
            status = vtss_perf_mon_dm_data_get_next(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                status = vtss_perf_mon_dm_data_get(&data[0]);
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        }

        while (status) {
            // error handler
            debug_counter++;
            if (debug_counter == PM_DEBUG_DM_MAX_LOOP) {
                T_W("Exit max loop!!\n");
                break;
            }
            for (idx = 0; idx < PM_DM_DATA_SET_LIMIT; idx++) {

                if (data[idx].measurement_interval_id == 0) {
                    break;
                }

                if (interval_value != 0) {
                    if (interval_value != data[idx].measurement_interval_id) {
                        continue;
                    }
                }

                if (instance_value != 0) {
                    if (instance_value != evc_id_i2u(data[idx].mep_instance)) {
                        continue;
                    }
                }

                /* interval_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].measurement_interval_id);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* instance_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", evc_id_i2u(data[idx].mep_instance));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* port */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", iport2uport(data[idx].mep_residence_port));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* priority */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].tx_priority);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* rate */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].tx_rate);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* unit */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", vtss_mep_unit_to_string(data[idx].measurement_unit));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* tx_count */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].tx_cnt);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* rx_count */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].rx_cnt);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* oneway_F2N_average */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].far_to_near_avg_delay);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* oneway_F2N_variation */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].far_to_near_avg_delay_variation);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* oneway_F2N_min */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].far_to_near_min_delay);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* oneway_F2N_max */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].far_to_near_max_delay);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* oneway_N2F_average */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].near_to_far_avg_delay);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* oneway_N2F_variation */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].near_to_far_avg_delay_variation);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* oneway_N2F_min */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].near_to_far_min_delay);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* oneway_N2F_max */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].near_to_far_max_delay);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* twoway_F2N_average */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].two_way_avg_delay);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* twoway_F2N_variation */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].two_way_avg_delay_variation);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* twoway_F2N_min */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].two_way_min_delay);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* twoway_F2N_max */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].two_way_max_delay);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* domain */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", vtss_mep_domain_to_string(data[idx].mep_domain));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* direction */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", vtss_mep_direction_to_string(data[idx].mep_direction));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* level */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].mep_level);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* flow */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", (data[idx].mep_domain != VTSS_MEP_MGMT_VLAN) ? evc_id_i2u(data[idx].mep_flow) : data[idx].mep_flow);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* vid */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].mep_vlan);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* mep_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].mep_id);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* mac */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%02x-%02x-%02x-%02x-%02x-%02x/",
                              data[idx].mep_mac[0],
                              data[idx].mep_mac[1],
                              data[idx].mep_mac[2],
                              data[idx].mep_mac[3],
                              data[idx].mep_mac[4],
                              data[idx].mep_mac[5]);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* peer_mep_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].mep_peer_id);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* peer_mac */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%02x-%02x-%02x-%02x-%02x-%02x",
                              data[idx].mep_peer_mac[0],
                              data[idx].mep_peer_mac[1],
                              data[idx].mep_peer_mac[2],
                              data[idx].mep_peer_mac[3],
                              data[idx].mep_peer_mac[4],
                              data[idx].mep_peer_mac[5]);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            status = vtss_perf_mon_dm_data_get_next(&data[0]);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/* Perf Mon EVC Statistics */
static cyg_int32 handler_stat_perf_mon_evc_table(CYG_HTTPD_STATE *p)
{
    int                         ct;
    BOOL                        status = FALSE;
    vtss_perf_mon_evc_info_t    data[PM_EVC_DATA_SET_LIMIT];
    u32                         idx;

    char                        search_str[64];
    u32                         ctrl_value = 0;
    u32                         interval_value = 0;
    u32                         instance_value = 0;
    int                         debug_counter = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PERF_MON)) {
        return -1;
    }
#endif

    memset(data, 0x0, PM_EVC_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));

    if (p->method == CYG_HTTPD_METHOD_POST) {

        redirect(p, "/perf_mon_evc_statistics.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */

        // Format: [ctrl_flag]
        // 0:init 1:refresh 2:firstPage 3:previousPage 4:nextPage 5:lastPage 6:deleteall

        sprintf(search_str, "ctrl_flag");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &ctrl_value);
        T_D("GET ctrl_flag = %d", ctrl_value);

        sprintf(search_str, "interval_id");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &interval_value);
        T_D("GET interval_id = %d", interval_value);

        sprintf(search_str, "instance_id");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &instance_value);
        T_D("GET instance_id = %d", instance_value);

        (void)cyg_httpd_start_chunked("html");

        // Format: [selected_interval_id];[selected_instance_id];
        //         [interval_id]/[instance_id]/[port_no]/
        //         [green_f_rx]/[green_f_tx]/[green_b_rx]/[green_b_tx]/
        //         [yellow_f_rx]/[yellow_f_tx]/[yellow_b_rx]/[yellow_b_tx]/
        //         [red_f_rx]/[red_b_tx]/
        //         [discard_f_rx]/[discard_f_tx]/[discard_b_rx]/[discard_b_tx]|...

        switch (ctrl_value) {
        case 6: // 6:deleteall
            /* delete all data set */
            status = vtss_perf_mon_evc_data_clear();

            /* get last status */
            status = vtss_perf_mon_evc_data_get_last(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 0: // 0:init
        case 5: // 5:lastPage
            /* get last status */
            status = vtss_perf_mon_evc_data_get_last(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 1: // 1:refresh
            /* get status */
            status = vtss_perf_mon_evc_data_get_first(&data[0]);

            /* selected_interval_id */
            if (interval_value == 0) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            /* selected_instance_id */
            if (instance_value == 0) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", instance_value);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            break;
        case 2: // 2:firstPage
            /* get first status */
            status = vtss_perf_mon_evc_data_get_first(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 3: // 3:previousPage
            /* get previous status */
            data[0].measurement_interval_id = interval_value;
            status = vtss_perf_mon_evc_data_get_previous(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                status = vtss_perf_mon_evc_data_get(&data[0]);
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 4: // 4:nextPage
            /* get next status */
            data[0].measurement_interval_id = interval_value;
            status = vtss_perf_mon_evc_data_get_next(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                status = vtss_perf_mon_evc_data_get(&data[0]);
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        }

        while (status) {
            // error handler
            debug_counter++;
            if (debug_counter == PM_DEBUG_EVC_MAX_LOOP) {
                T_W("Exit max loop!!\n");
                break;
            }
            for (idx = 0; idx < PM_EVC_DATA_SET_LIMIT; idx++) {

                if (data[idx].measurement_interval_id == 0) {
                    break;
                }

                if (interval_value != 0) {
                    if (interval_value != data[idx].measurement_interval_id) {
                        continue;
                    }
                }

                if (instance_value != 0) {
                    if (instance_value != evc_id_i2u(data[idx].evc_instance)) {
                        continue;
                    }
                }

                /* interval_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].measurement_interval_id);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* instance_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", evc_id_i2u(data[idx].evc_instance));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* port_no */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", iport2uport(data[idx].evc_port));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* green_f_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_green);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* green_f_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_green);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* green_b_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_green_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* green_b_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_green_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* yellow_f_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_yellow);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* yellow_f_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_yellow);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* yellow_b_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_yellow_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* yellow_b_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_yellow_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* red_f_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_red);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* red_b_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_red);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* discard_f_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_discard);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* discard_f_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_discard);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* discard_b_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_discard_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* discard_b_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu", data[idx].tx_discard_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            status = vtss_perf_mon_evc_data_get_next(&data[0]);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/* Perf Mon ECE Statistics */
static cyg_int32 handler_stat_perf_mon_evc_ece_table(CYG_HTTPD_STATE *p)
{
    int                         ct;
    BOOL                        status = FALSE;
    vtss_perf_mon_evc_info_t    data[PM_ECE_DATA_SET_LIMIT];
    u32                         idx;

    char                        search_str[64];
    u32                         ctrl_value = 0;
    u32                         interval_value = 0;
    u32                         instance_value = 0;
    int                         debug_counter = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PERF_MON)) {
        return -1;
    }
#endif

    memset(data, 0x0, PM_ECE_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));

    if (p->method == CYG_HTTPD_METHOD_POST) {

        redirect(p, "/perf_mon_evc_ece_statistics.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */

        // Format: [ctrl_flag]
        // 0:init 1:refresh 2:firstPage 3:previousPage 4:nextPage 5:lastPage 6:deleteall

        sprintf(search_str, "ctrl_flag");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &ctrl_value);
        T_D("GET ctrl_flag = %d", ctrl_value);

        sprintf(search_str, "interval_id");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &interval_value);
        T_D("GET interval_id = %d", interval_value);

        sprintf(search_str, "instance_id");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &instance_value);
        T_D("GET instance_id = %d", instance_value);

        (void)cyg_httpd_start_chunked("html");

        // Format: [selected_interval_id];[selected_instance_id];
        //         [interval_id]/[instance_id]/[port_no]/
        //         [green_f_rx]/[green_f_tx]/[green_b_rx]/[green_b_tx]/
        //         [yellow_f_rx]/[yellow_f_tx]/[yellow_b_rx]/[yellow_b_tx]/
        //         [red_f_rx]/[red_b_tx]/
        //         [discard_f_rx]/[discard_f_tx]/[discard_b_rx]/[discard_b_tx]|...

        switch (ctrl_value) {
        case 6: // 6:deleteall
            /* delete all data set */
            status = vtss_perf_mon_ece_data_clear();

            /* get last status */
            status = vtss_perf_mon_ece_data_get_last(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 0: // 0:init
        case 5: // 5:lastPage
            /* get last status */
            status = vtss_perf_mon_ece_data_get_last(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 1: // 1:refresh
            /* get status */
            status = vtss_perf_mon_ece_data_get_first(&data[0]);

            /* selected_interval_id */
            if (interval_value == 0) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }

            /* selected_instance_id */
            if (instance_value == 0) {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            } else {
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", instance_value);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            }
            break;
        case 2: // 2:firstPage
            /* get first status */
            status = vtss_perf_mon_ece_data_get_first(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                interval_value = 1;
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 3: // 3:previousPage
            /* get previous status */
            data[0].measurement_interval_id = interval_value;
            status = vtss_perf_mon_ece_data_get_previous(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                status = vtss_perf_mon_ece_data_get(&data[0]);
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        case 4: // 4:nextPage
            /* get next status */
            data[0].measurement_interval_id = interval_value;
            status = vtss_perf_mon_ece_data_get_next(&data[0]);

            if (status) {
                interval_value = data[0].measurement_interval_id;
            } else {
                status = vtss_perf_mon_ece_data_get(&data[0]);
            }

            /* selected_interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", interval_value);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* selected_instance_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "All;");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            break;
        }

        while (status) {
            // error handler
            debug_counter++;
            if (debug_counter == PM_DEBUG_ECE_MAX_LOOP) {
                T_W("Exit max loop!!\n");
                break;
            }
            for (idx = 0; idx < PM_ECE_DATA_SET_LIMIT; idx++) {

                if (data[idx].measurement_interval_id == 0) {
                    break;
                }

                if (interval_value != 0) {
                    if (interval_value != data[idx].measurement_interval_id) {
                        continue;
                    }
                }

                if (instance_value != 0) {
                    if (instance_value != evc_id_i2u(data[idx].evc_instance)) {
                        continue;
                    }
                }

                /* interval_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].measurement_interval_id);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* instance_id */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data[idx].evc_instance);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* port_no */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", iport2uport(data[idx].evc_port));
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* green_f_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_green);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* green_f_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_green);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* green_b_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_green_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* green_b_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_green_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* yellow_f_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_yellow);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* yellow_f_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_yellow);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* yellow_b_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_yellow_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* yellow_b_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_yellow_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* red_f_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_red);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* red_b_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_red);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* discard_f_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_discard);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* discard_f_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].tx_discard);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* discard_b_rx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu/", data[idx].rx_discard_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                /* discard_b_tx */
                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%llu", data[idx].tx_discard_b);
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

                ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
                (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            }
            status = vtss_perf_mon_ece_data_get_next(&data[0]);
        }

        (void)cyg_httpd_end_chunked();
    }

    return -1; // Do not further search the file system.
}

/* Perf Mon Interval Info */
static cyg_int32 handler_stat_perf_mon_interval_info_table(CYG_HTTPD_STATE *p)
{
    int                                 ct;
    vtss_perf_mon_measurement_info_t    data;
    char                                search_str[64];
    char                                buf[256];

    u32                                 ctrl_value = 0;
    u32                                 info_type_value = 0;
    u32                                 start_id_value = 0;
    u32                                 entries_num_value = 0;
    u32                                 total_cnt = 1;
    u32                                 pm_idx;
    BOOL                                status = FALSE;
    BOOL                                lm_flag = FALSE;
    BOOL                                dm_flag = FALSE;
    BOOL                                evc_flag = FALSE;
    BOOL                                ece_flag = FALSE;
    int                                 debug_counter = 0;

#ifdef VTSS_SW_OPTION_PRIV_LVL
    if (web_process_priv_lvl(p, VTSS_PRIV_LVL_CONFIG_TYPE, VTSS_MODULE_ID_PERF_MON)) {
        return -1;
    }
#endif

    if (p->method == CYG_HTTPD_METHOD_POST) {

        redirect(p, "/perf_mon_interval_info.htm");
    } else {                    /* CYG_HTTPD_METHOD_GET (+HEAD) */
        (void)cyg_httpd_start_chunked("html");

        // Format: [ctrl_flag]
        // 0:init 1:refresh 2:firstPage 3:previousPage 4:nextPage 5:lastPage

        sprintf(search_str, "ctrl_flag");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &ctrl_value);
        T_D("GET ctrl_flag = %d", ctrl_value);

        sprintf(search_str, "page_info_type");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &info_type_value);
        T_D("GET page_info_type = %d", info_type_value);

        sprintf(search_str, "page_start_id");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &start_id_value);
        T_D("GET page_start_id = %d", start_id_value);

        sprintf(search_str, "page_entries_num");
        (void) cyg_httpd_form_varable_long_int(p, search_str, &entries_num_value);
        T_D("GET page_entries_num = %d", entries_num_value);

        // Format: [page_info_type];[page_start_id];[page_entries_num];
        //         [info_type]/[interval_id]/[start_time]/[end_time]/[elspsed_time]|...

        if (info_type_value == PM_INTERVAL_TYPE_LM) {
            switch (ctrl_value) {
            case 1: // 1:refresh
                /* get status */
                break;
            case 0: // 0:init
            case 2: // 2:firstPage
                /* get first status */
                status = vtss_perf_mon_lm_interval_get_first(&data);

                if (status) {
                    start_id_value = data.measurement_interval_id;
                } else {
                    start_id_value = 1;
                }
                break;
            case 3: // 3:previousPage
                /* get previous status */
                data.measurement_interval_id = start_id_value;
                status = vtss_perf_mon_lm_interval_get_previous(&data);

                // find the correct index id
                if (status) {
                    start_id_value = data.measurement_interval_id;
                    for (pm_idx = 1; pm_idx <= (entries_num_value - 1); pm_idx++) {
                        status = vtss_perf_mon_lm_interval_get_previous(&data);
                        if (status) {
                            start_id_value = data.measurement_interval_id;
                        }
                    }
                }
                break;
            case 4: // 4:nextPage
                /* get next status */
                data.measurement_interval_id = start_id_value;
                status = vtss_perf_mon_lm_interval_get_next(&data);

                if (status) {
                    start_id_value = data.measurement_interval_id;
                }
                break;
            case 5: // 5:lastPage
                /* get last status */
                status = vtss_perf_mon_lm_interval_get_last(&data);

                // find the correct index id
                if (status) {
                    start_id_value = data.measurement_interval_id;
                    for (pm_idx = 1; pm_idx <= (entries_num_value - 1); pm_idx++) {
                        status = vtss_perf_mon_lm_interval_get_previous(&data);
                        if (status) {
                            start_id_value = data.measurement_interval_id;
                        }
                    }
                } else {
                    start_id_value = 1;
                }
                break;
            }
        } else if (info_type_value == PM_INTERVAL_TYPE_DM) {
            switch (ctrl_value) {
            case 1: // 1:refresh
                /* get status */
                break;
            case 0: // 0:init
            case 2: // 2:firstPage
                /* get first status */
                status = vtss_perf_mon_dm_interval_get_first(&data);

                if (status) {
                    start_id_value = data.measurement_interval_id;
                } else {
                    start_id_value = 1;
                }
                break;
            case 3: // 3:previousPage
                /* get previous status */
                data.measurement_interval_id = start_id_value;
                status = vtss_perf_mon_dm_interval_get_previous(&data);

                // find the correct index id
                if (status) {
                    start_id_value = data.measurement_interval_id;
                    for (pm_idx = 1; pm_idx <= (entries_num_value - 1); pm_idx++) {
                        status = vtss_perf_mon_dm_interval_get_previous(&data);
                        if (status) {
                            start_id_value = data.measurement_interval_id;
                        }
                    }
                }
                break;
            case 4: // 4:nextPage
                /* get next status */
                data.measurement_interval_id = start_id_value;
                status = vtss_perf_mon_dm_interval_get_next(&data);

                if (status) {
                    start_id_value = data.measurement_interval_id;
                }
                break;
            case 5: // 5:lastPage
                /* get last status */
                status = vtss_perf_mon_dm_interval_get_last(&data);

                // find the correct index id
                if (status) {
                    start_id_value = data.measurement_interval_id;
                    for (pm_idx = 1; pm_idx <= (entries_num_value - 1); pm_idx++) {
                        status = vtss_perf_mon_dm_interval_get_previous(&data);
                        if (status) {
                            start_id_value = data.measurement_interval_id;
                        }
                    }
                } else {
                    start_id_value = 1;
                }
                break;
            }
        } else if (info_type_value == PM_INTERVAL_TYPE_EVC) {
            switch (ctrl_value) {
            case 1: // 1:refresh
                /* get status */
                break;
            case 0: // 0:init
            case 2: // 2:firstPage
                /* get first status */
                status = vtss_perf_mon_evc_interval_get_first(&data);

                if (status) {
                    start_id_value = data.measurement_interval_id;
                } else {
                    start_id_value = 1;
                }
                break;
            case 3: // 3:previousPage
                /* get previous status */
                data.measurement_interval_id = start_id_value;
                status = vtss_perf_mon_evc_interval_get_previous(&data);

                // find the correct index id
                if (status) {
                    start_id_value = data.measurement_interval_id;
                    for (pm_idx = 1; pm_idx <= (entries_num_value - 1); pm_idx++) {
                        status = vtss_perf_mon_evc_interval_get_previous(&data);
                        if (status) {
                            start_id_value = data.measurement_interval_id;
                        }
                    }
                }
                break;
            case 4: // 4:nextPage
                /* get next status */
                data.measurement_interval_id = start_id_value;
                status = vtss_perf_mon_evc_interval_get_next(&data);

                if (status) {
                    start_id_value = data.measurement_interval_id;
                }
                break;
            case 5: // 5:lastPage
                /* get last status */
                status = vtss_perf_mon_evc_interval_get_last(&data);

                // find the correct index id
                if (status) {
                    start_id_value = data.measurement_interval_id;
                    for (pm_idx = 1; pm_idx <= (entries_num_value - 1); pm_idx++) {
                        status = vtss_perf_mon_evc_interval_get_previous(&data);
                        if (status) {
                            start_id_value = data.measurement_interval_id;
                        }
                    }
                } else {
                    start_id_value = 1;
                }
                break;
            }
        } else if (info_type_value == PM_INTERVAL_TYPE_ECE) {
            switch (ctrl_value) {
            case 1: // 1:refresh
                /* get status */
                break;
            case 0: // 0:init
            case 2: // 2:firstPage
                /* get first status */
                status = vtss_perf_mon_ece_interval_get_first(&data);

                if (status) {
                    start_id_value = data.measurement_interval_id;
                } else {
                    start_id_value = 1;
                }
                break;
            case 3: // 3:previousPage
                /* get previous status */
                data.measurement_interval_id = start_id_value;
                status = vtss_perf_mon_ece_interval_get_previous(&data);

                // find the correct index id
                if (status) {
                    start_id_value = data.measurement_interval_id;
                    for (pm_idx = 1; pm_idx <= (entries_num_value - 1); pm_idx++) {
                        status = vtss_perf_mon_ece_interval_get_previous(&data);
                        if (status) {
                            start_id_value = data.measurement_interval_id;
                        }
                    }
                }
                break;
            case 4: // 4:nextPage
                /* get next status */
                data.measurement_interval_id = start_id_value;
                status = vtss_perf_mon_ece_interval_get_next(&data);

                if (status) {
                    start_id_value = data.measurement_interval_id;
                }
                break;
            case 5: // 5:lastPage
                /* get last status */
                status = vtss_perf_mon_ece_interval_get_last(&data);

                // find the correct index id
                if (status) {
                    start_id_value = data.measurement_interval_id;
                    for (pm_idx = 1; pm_idx <= (entries_num_value - 1); pm_idx++) {
                        status = vtss_perf_mon_ece_interval_get_previous(&data);
                        if (status) {
                            start_id_value = data.measurement_interval_id;
                        }
                    }
                } else {
                    start_id_value = 1;
                }
                break;
            }
        }

        /* page_info_type */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", info_type_value);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* page_start_id */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", start_id_value);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* page_entries_num */
        ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u;", entries_num_value);
        (void)cyg_httpd_write_chunked(p->outbuffer, ct);

        /* Loss Measurement */
        debug_counter = 0;
        status = vtss_perf_mon_lm_interval_get_first(&data);
        while (status && (info_type_value == PM_INTERVAL_TYPE_LM)) {
            // error handler
            debug_counter++;
            if (debug_counter == PM_DEBUG_LM_MAX_LOOP) {
                T_W("Exit max loop!!\n");
                break;
            }

            if (data.measurement_interval_id == 0) {
                status = vtss_perf_mon_lm_interval_get_next(&data);
                continue;
            }
#if 0
            if (info_type_value != PM_INTERVAL_TYPE_ALL) {
                if (info_type_value != PM_INTERVAL_TYPE_LM) {
                    status = vtss_perf_mon_lm_interval_get_next(&data);
                    continue;
                }
            }
#endif
            if (!lm_flag && data.measurement_interval_id < start_id_value) {
                status = vtss_perf_mon_lm_interval_get_next(&data);
                continue;
            } else {
                lm_flag = TRUE;
            }

            if (total_cnt > entries_num_value) {
                break;
            }

            /* info_type */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", PM_INTERVAL_TYPE_LM);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data.measurement_interval_id);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* start_time */
            (void)misc_time2str_r(data.interval_end_time - data.elapsed_time, buf);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", buf);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* end_time */
            (void)misc_time2str_r(data.interval_end_time, buf);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", buf);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* elspsed_time */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u", data.elapsed_time);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            total_cnt++;
            status = vtss_perf_mon_lm_interval_get_next(&data);
        }

        /* Delay Measurement */
        debug_counter = 0;
        status = vtss_perf_mon_dm_interval_get_first(&data);
        while (status && (info_type_value == PM_INTERVAL_TYPE_DM)) {
            // error handler
            debug_counter++;
            if (debug_counter == PM_DEBUG_DM_MAX_LOOP) {
                T_W("Exit max loop!!\n");
                break;
            }

            if (data.measurement_interval_id == 0) {
                status = vtss_perf_mon_dm_interval_get_next(&data);
                continue;
            }
#if 0
            if (info_type_value != PM_INTERVAL_TYPE_ALL) {
                if (info_type_value != PM_INTERVAL_TYPE_DM) {
                    status = vtss_perf_mon_dm_interval_get_next(&data);
                    continue;
                }
            }
#endif
            if (!dm_flag && data.measurement_interval_id < start_id_value) {
                status = vtss_perf_mon_dm_interval_get_next(&data);
                continue;
            } else {
                dm_flag = TRUE;
            }

            if (total_cnt > entries_num_value) {
                break;
            }

            /* info_type */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", PM_INTERVAL_TYPE_DM);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data.measurement_interval_id);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* start_time */
            (void)misc_time2str_r(data.interval_end_time - data.elapsed_time, buf);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", buf);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* end_time */
            (void)misc_time2str_r(data.interval_end_time, buf);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", buf);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* elspsed_time */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u", data.elapsed_time);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            total_cnt++;
            status = vtss_perf_mon_dm_interval_get_next(&data);
        }

        /* EVC */
        debug_counter = 0;
        status = vtss_perf_mon_evc_interval_get_first(&data);
        while (status && (info_type_value == PM_INTERVAL_TYPE_EVC)) {
            // error handler
            debug_counter++;
            if (debug_counter == PM_DEBUG_EVC_MAX_LOOP) {
                T_W("Exit max loop!!\n");
                break;
            }

            if (data.measurement_interval_id == 0) {
                status = vtss_perf_mon_evc_interval_get_next(&data);
                continue;
            }
#if 0
            if (info_type_value != PM_INTERVAL_TYPE_ALL) {
                if (info_type_value != PM_INTERVAL_TYPE_EVC) {
                    status = vtss_perf_mon_evc_interval_get_next(&data);
                    continue;
                }
            }
#endif
            if (!evc_flag && data.measurement_interval_id < start_id_value) {
                status = vtss_perf_mon_evc_interval_get_next(&data);
                continue;
            } else {
                evc_flag = TRUE;
            }

            if (total_cnt > entries_num_value) {
                break;
            }

            /* info_type */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", PM_INTERVAL_TYPE_EVC);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data.measurement_interval_id);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* start_time */
            (void)misc_time2str_r(data.interval_end_time - data.elapsed_time, buf);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", buf);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* end_time */
            (void)misc_time2str_r(data.interval_end_time, buf);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", buf);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* elspsed_time */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u", data.elapsed_time);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            total_cnt++;
            status = vtss_perf_mon_evc_interval_get_next(&data);
        }

        /* ECE */
        debug_counter = 0;
        status = vtss_perf_mon_ece_interval_get_first(&data);
        while (status && (info_type_value == PM_INTERVAL_TYPE_ECE)) {
            // error handler
            debug_counter++;
            if (debug_counter == PM_DEBUG_ECE_MAX_LOOP) {
                T_W("Exit max loop!!\n");
                break;
            }

            if (data.measurement_interval_id == 0) {
                status = vtss_perf_mon_ece_interval_get_next(&data);
                continue;
            }
#if 0
            if (info_type_value != PM_INTERVAL_TYPE_ALL) {
                if (info_type_value != PM_INTERVAL_TYPE_ECE) {
                    status = vtss_perf_mon_ece_interval_get_next(&data);
                    continue;
                }
            }
#endif
            if (!ece_flag && data.measurement_interval_id < start_id_value) {
                status = vtss_perf_mon_ece_interval_get_next(&data);
                continue;
            } else {
                ece_flag = TRUE;
            }

            if (total_cnt > entries_num_value) {
                break;
            }

            /* info_type */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", PM_INTERVAL_TYPE_ECE);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* interval_id */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u/", data.measurement_interval_id);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* start_time */
            (void)misc_time2str_r(data.interval_end_time - data.elapsed_time, buf);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", buf);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* end_time */
            (void)misc_time2str_r(data.interval_end_time, buf);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%s/", buf);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            /* elspsed_time */
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "%u", data.elapsed_time);
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);
            ct = snprintf(p->outbuffer, sizeof(p->outbuffer), "|");
            (void)cyg_httpd_write_chunked(p->outbuffer, ct);

            total_cnt++;
            status = vtss_perf_mon_ece_interval_get_next(&data);
        }

        (void)cyg_httpd_end_chunked();

    }

    return -1; // Do not further search the file system.
}

/****************************************************************************/
/*  JS lib config table entry                                               */
/****************************************************************************/

web_lib_config_js_tab_entry(interval_info_lib_config_js);

/****************************************************************************/
/*  HTTPD Table Entries                                                     */
/****************************************************************************/

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_perf_mon_conf_table, "/config/perf_mon_conf", handler_config_perf_mon_conf_table);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_config_perf_mon_transfer_table, "/config/perf_mon_transfer", handler_config_perf_mon_transfer_table);

CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_perf_mon_lm_table, "/stat/perf_mon_lm_statistics", handler_stat_perf_mon_lm_table);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_perf_mon_dm_table, "/stat/perf_mon_dm_statistics", handler_stat_perf_mon_dm_table);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_perf_mon_evc_table, "/stat/perf_mon_evc_statistics", handler_stat_perf_mon_evc_table);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_perf_mon_evc_ece_table, "/stat/perf_mon_evc_ece_statistics", handler_stat_perf_mon_evc_ece_table);
CYG_HTTPD_HANDLER_TABLE_ENTRY(get_cb_stat_perf_mon_interval_info_table, "/stat/perf_mon_interval_info", handler_stat_perf_mon_interval_info_table);

/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
