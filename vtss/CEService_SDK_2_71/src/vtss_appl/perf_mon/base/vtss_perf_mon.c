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

#include "main.h"
#include "critd_api.h"
#include "perf_mon_api.h"
#include "vtss_perf_mon.h"
#include "vtss_types.h"
#include "misc_api.h" /* For misc_url_XXX() */
#include "conf_api.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <tftp_support.h>

#define PM_GZIP_COMPRESS
#ifdef  PM_GZIP_COMPRESS
#include <cyg/compress/zlib.h>
#endif

#include "../../mep/base/vtss_mep.h"

#include "evc_api.h"
#include "port_api.h"

#ifdef VTSS_SW_OPTION_ICFG
#include "perf_mon_icfg.h"
#endif

#ifdef VTSS_SW_OPTION_SYSUTIL
#include "sysutil_api.h"
#endif

#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
//LINT #include "daylight_saving_api.h"
#endif

#ifdef VTSS_SW_OPTION_LIBFETCH
#include "fetch.h"
#endif /* VTSS_SW_OPTION_LIBFETCH */

#define VTSS_ALLOC_MODULE_ID                VTSS_MODULE_ID_PERF_MON
#define PM_FIRST_INTERVAL_ID                1

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "perf_mon",
    .descr     = "PERF_MON"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};

#define PERF_MON_CRIT_ENTER() critd_enter(&pm_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PERF_MON_CRIT_EXIT()  critd_exit( &pm_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define PERF_MON_CRIT_ENTER() critd_enter(&pm_global.crit)
#define PERF_MON_CRIT_EXIT()  critd_exit( &pm_global.crit)
#endif /* VTSS_TRACE_ENABLED */

/* Global structure */
#define TRANSFER_MINUTE_TO_SECOND           60
#define PM_MAX_RANDOM_OFFSET                (15 * TRANSFER_MINUTE_TO_SECOND)
static perf_mon_global_t                    pm_global;

/* Thread variables */
#define PM_CERT_THREAD_STACK_SIZE           16384

static cyg_handle_t pm_thread_handle;
static cyg_thread   pm_thread_block;
static char         pm_thread_stack[PM_CERT_THREAD_STACK_SIZE];


/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

static inline vtss_evc_id_t evc_id_i2u(vtss_evc_id_t evc_id)
{
    return ((evc_id + 1) > EVC_ID_COUNT ? EVC_ID_COUNT :  evc_id + 1);
}


/**
  * \brief Decompose URL string for PM upload
  *
  * \return
  *   TRUE: URL string is decomposed succesfully
  *   FALSE: Invalid/Unsupported URL string
  */
BOOL vtss_perf_mon_decompose_url(const i8 *src_or_dest_text,
                                 const i8 *url_str,
                                 int url_len,
                                 void *myurl)
{
    misc_url_parts_t    *url_parts;
    i8                  *pm_url;

    if (!src_or_dest_text || !url_str || !url_len || !myurl ||
        (pm_url = (i8 *)VTSS_MALLOC(PM_STR_LENGTH)) == NULL) {
        return FALSE;
    }

    memset(pm_url, 0x0, PM_STR_LENGTH);
    strncpy(pm_url, url_str, url_len);
    if (url_len < (PM_STR_LENGTH - 1) &&
        *(pm_url + url_len - 1) != '/') {
        *(pm_url + url_len) = '/';
    }

    url_parts = (misc_url_parts_t *)myurl;
    if (misc_url_decompose(pm_url, url_parts)) {
        if (strncmp(url_parts->protocol, "http", 4) != 0  &&  strncmp(url_parts->protocol, "tftp", 4) != 0) {
            T_W("%% Invalid %s file system type (must be http: or tftp:)\n", src_or_dest_text);
        } else {
            i8  opt_port[6];

            memset(opt_port, 0x0, sizeof(opt_port));
            if (url_parts->port) {
                int cnt;

                cnt = snprintf(opt_port, sizeof(opt_port), "%u", url_parts->port);
                if ((cnt < 0) || ((size_t)cnt >= sizeof(opt_port))) {
                    memset(opt_port, 0x0, sizeof(opt_port));
                }
            }
            T_D("Decompose->%s://%s%s%s%s",
                url_parts->protocol,
                url_parts->host,
                url_parts->port ? ":" : "",
                opt_port,
                url_parts->path);

            VTSS_FREE(pm_url);
            return TRUE;
        }
    }

    VTSS_FREE(pm_url);
    T_D("Invalid %s URL, expected http://server[:port]/path-to-file or tftp://server[:port]/path-to-file\n", src_or_dest_text);
    return FALSE;
}

#ifdef VTSS_SW_OPTION_LIBFETCH
static BOOL _pm_http_upload(const i8 *target, u32 dlen, const i8 *data)
{
    fetchIO *fp;

    if (!target || !data) {
        return FALSE;
    }

    T_D("Fetching(%s) with LEN=%u...\n", target, dlen);
    if ((fp = fetchPutURL(target, "x", dlen, data)) != NULL) {
        fetchIO_close(fp);
    }

    return TRUE;
}
#endif /* VTSS_SW_OPTION_LIBFETCH */

/**
  * \brief PM Utility for sending colloected statistics via HTTP PUT
  *
  * \return
  *   FALSE on any error, otherwise TRUE.
  */
static BOOL _perf_mon_util_save_to_http(i8 *buf, u32 buf_len, const misc_url_parts_t *url_parts)
{
    int cnt;
#ifdef VTSS_SW_OPTION_LIBFETCH
    int url_sz;
    i8  *url_str, opt_port[6];

    if (!buf || !url_parts) {
        return FALSE;
    }

    url_sz = sizeof(misc_url_parts_t) - sizeof(u16) + sizeof(opt_port);
    if ((url_sz <= 0) || ((url_str = VTSS_MALLOC(url_sz)) == NULL)) {
        return FALSE;
    }

    memset(url_str, 0x0, url_sz);
    memset(opt_port, 0x0, sizeof(opt_port));
    if (url_parts->port) {
        cnt = snprintf(opt_port, sizeof(opt_port), "%u", url_parts->port);
        if ((cnt < 0) || ((size_t)cnt >= sizeof(opt_port))) {
            memset(opt_port, 0x0, sizeof(opt_port));
        }
    }

    cnt = snprintf(url_str, url_sz, "%s://%s%s%s%s",
                   url_parts->protocol,
                   url_parts->host,
                   url_parts->port ? ":" : "",
                   opt_port,
                   url_parts->path);
    if ((cnt > 0) && (cnt < url_sz)) {
        T_I("Saving %u bytes to HTTP server %s\n", buf_len, url_str);
        if (!_pm_http_upload(url_str, buf_len, buf)) {
            cnt = -1;
        }
    } else {
        cnt = 0;
    }

    VTSS_FREE(url_str);
#else
    cnt = 1;
    T_W("HTTP PUT not implement\n");
#endif /* VTSS_SW_OPTION_LIBFETCH */

    return (cnt > 0);
}



static BOOL save_to_tftp(char                     *buf,
                         uLong                    len,
                         const misc_url_parts_t   *url_parts)
{
    u32                          total    = 0;
    int                          status;
    int                          tftp_err;

    // Our TFTP put function expects a contiguous buffer, so we may have to
    // create one and copy all the blocks into it. We do try to avoid it,
    // though.

    total = len;

    T_I("%% Saving %u bytes to TFTP server %s: %s\n",
        total, url_parts->host, url_parts->path);

    status = tftp_client_put(url_parts->path,
                             url_parts->host,
                             url_parts->port,
                             buf,
                             total,
                             TFTP_OCTET,
                             &tftp_err);

    if (status < 0) {
        T_W("%% TFTP save error\n");
        return FALSE;
    }

    return TRUE;
}



static BOOL save_config(char                     *buf,
                        const misc_url_parts_t   *url_parts)
{

    uLong   len;
    i8      opt_port[6];
#ifdef  PM_GZIP_COMPRESS
#define COMPRESS_OVERHEAD 1024
    int     buf_size = strlen(buf) + COMPRESS_OVERHEAD;
    uchar  *packed;
    BOOL    rc;

    packed = VTSS_MALLOC(buf_size);
    if (!packed) {
        T_E("%% Not enough free RAM memory for PM save operation (needed %u bytes).\n", buf_size);
        return FALSE;
    }

    len = buf_size;
    if (compress(packed, &len, (uchar *)buf, strlen(buf)) != 0) {
        T_W("Calling compress() failed\n");
        VTSS_FREE(packed);
        return FALSE;
    }
#endif
    memset(opt_port, 0x0, sizeof(opt_port));
    if (url_parts->port) {
        int cnt;

        cnt = snprintf(opt_port, sizeof(opt_port), "%u", url_parts->port);
        if ((cnt < 0) || ((size_t)cnt >= sizeof(opt_port))) {
            memset(opt_port, 0x0, sizeof(opt_port));
        }
    }
    T_D("Dispatching %s://%s%s%s%s",
        url_parts->protocol,
        url_parts->host,
        url_parts->port ? ":" : "",
        opt_port,
        url_parts->path);

    if (strcmp(url_parts->protocol, "http") == 0) {
#ifdef  PM_GZIP_COMPRESS
        rc = _perf_mon_util_save_to_http((char *)packed, len, url_parts);
        VTSS_FREE(packed);
        return rc;
#else
        len = strlen(buf);
        return _perf_mon_util_save_to_http(buf, len, url_parts);
#endif
    }

    if (strcmp(url_parts->protocol, "tftp") == 0) {
#ifdef  PM_GZIP_COMPRESS
        rc = save_to_tftp((char *)packed, len, url_parts);
        VTSS_FREE(packed);
        return rc;
#else
        len = strlen(buf);
        return save_to_tftp(buf, len, url_parts);
#endif
    }

#ifdef  PM_GZIP_COMPRESS
    VTSS_FREE(packed);
#endif
    T_W("Protocol not supported: only http and tftp are supported\n");
    return FALSE;
}

static vtss_rc transfer_lm_data_set(misc_url_parts_t *url_parts)
{
    system_conf_t               system_conf;
    time_t                      now;
    vtss_rc                     rc        = VTSS_RC_ERROR;
    BOOL                        save_ok   = FALSE;
    char                        *buf_ptr = NULL;
    int                         ct, path_len;

    BOOL                        status = FALSE;
    vtss_perf_mon_lm_info_t     data[PM_LM_DATA_SET_LIMIT];
    u32                         idx;
    char                        tmp_buf[PM_BUF_LENGTH];
    char                        *p;
    int                         total = 0;

    if (!url_parts || ((size_t)(path_len = strlen(url_parts->path)) >= sizeof(url_parts->path))) {
        return rc;
    }

    /* create filename */
    if (system_get_config(&system_conf) != VTSS_OK) {
        return rc;
    }

    now = time(NULL);
    if (now >= 0) {
        struct tm *timeinfo_p;

        perf_mon_util_adjust_time_tz(&now);
        timeinfo_p = localtime(&now);
        ct = snprintf(url_parts->path + path_len, sizeof(url_parts->path) - path_len, "%s%s_history_%04d-%02d-%02d_%02dh.%02dm.%02ds_LM_all.csv%s",
                      (path_len > 0 && (url_parts->path[path_len - 1] != '/')) ? "/" : "",
                      system_conf.sys_name,
                      timeinfo_p->tm_year + 1900,
                      timeinfo_p->tm_mon + 1,
                      timeinfo_p->tm_mday,
                      timeinfo_p->tm_hour,
                      timeinfo_p->tm_min,
                      timeinfo_p->tm_sec,
#ifdef PM_GZIP_COMPRESS
                      ".gz"
#else
                      ""
#endif /* PM_GZIP_COMPRESS */
                     );
    } else {
        ct = snprintf(url_parts->path + path_len, sizeof(url_parts->path) - path_len, "%s%s_history_no-time_LM_all.csv%s",
                      (path_len > 0 && (url_parts->path[path_len - 1] != '/')) ? "/" : "",
                      system_conf.sys_name,
#ifdef PM_GZIP_COMPRESS
                      ".gz"
#else
                      ""
#endif /* PM_GZIP_COMPRESS */
                     );
    }

    if ((ct < 0) || ((size_t)(ct + path_len) >= sizeof(url_parts->path))) {
        return rc;
    }

    if ((buf_ptr = (char *)VTSS_MALLOC(PM_ALLOCATE_LM_BUFFER)) == NULL) {
        T_E("%% Not enough free RAM memory for PM save operation (needed %u bytes).\n", PM_ALLOCATE_LM_BUFFER);
        return rc;
    }

    /* read data from DB */
    memset(buf_ptr, 0x0, PM_ALLOCATE_LM_BUFFER);
    p = buf_ptr;
    total = 0;

    // create file header
    memset(tmp_buf, 0, sizeof(tmp_buf));
    ct = snprintf(tmp_buf, sizeof(tmp_buf), "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                  "interval_id",
                  "instance_id",
                  "port",
                  "priority",
                  "rate",
                  "tx_count",
                  "rx_count",
                  "near_end_count",
                  "near_end_ratio",
                  "far_end_count",
                  "far_end_ratio",
                  "domain",
                  "direction",
                  "level",
                  "flow",
                  "vid",
                  "mep_id",
                  "mac",
                  "peer_mep_id",
                  "peer_mac");

    if (ct) {
        memcpy(p, tmp_buf, ct);
        p += ct;
        total = total + ct;
    }

    // read data
    status = vtss_perf_mon_lm_data_get_first(&data[0]);
    while (status) {
        for (idx = 0; idx < PM_LM_DATA_SET_LIMIT; idx++) {

            if (data[idx].measurement_interval_id == 0) {
                break;
            }

            memset(tmp_buf, 0, sizeof(tmp_buf));
            ct = snprintf(tmp_buf, sizeof(tmp_buf), "%u,%u,%u,%u,%s,%u,%u,%u,%u,%u,%u,%s,%s,%u,%u,%u,%u,%02x-%02x-%02x-%02x-%02x-%02x,%u,%02x-%02x-%02x-%02x-%02x-%02x\n",
                          data[idx].measurement_interval_id,
                          evc_id_i2u(data[idx].mep_instance),
                          iport2uport(data[idx].mep_residence_port),
                          data[idx].tx_priority,
                          vtss_mep_period_to_string(data[idx].tx_rate),
                          data[idx].tx_cnt,
                          data[idx].rx_cnt,
                          data[idx].near_end_loss_cnt,
                          data[idx].near_end_frame_loss_ratio,
                          data[idx].far_end_loss_cnt,
                          data[idx].far_end_loss_ratio,
                          vtss_mep_domain_to_string(data[idx].mep_domain),
                          vtss_mep_direction_to_string(data[idx].mep_direction),
                          data[idx].mep_level,
                          (data[idx].mep_domain != VTSS_MEP_MGMT_VLAN) ? evc_id_i2u(data[idx].mep_flow) : data[idx].mep_flow,
                          data[idx].mep_vlan,
                          data[idx].mep_id,
                          data[idx].mep_mac[0],
                          data[idx].mep_mac[1],
                          data[idx].mep_mac[2],
                          data[idx].mep_mac[3],
                          data[idx].mep_mac[4],
                          data[idx].mep_mac[5],
                          data[idx].mep_peer_id,
                          data[idx].mep_peer_mac[0],
                          data[idx].mep_peer_mac[1],
                          data[idx].mep_peer_mac[2],
                          data[idx].mep_peer_mac[3],
                          data[idx].mep_peer_mac[4],
                          data[idx].mep_peer_mac[5]);

            if (ct) {
                memcpy(p, tmp_buf, ct);
                p += ct;
                total = total + ct;
            }
        }

        status = vtss_perf_mon_lm_data_get_next(&data[0]);
    }

    if (p) {
        *p = '\0';
    }

    /* save data to tftp or http */
    save_ok = save_config(buf_ptr, url_parts);

    VTSS_FREE(buf_ptr);

    rc = save_ok ? VTSS_RC_OK : VTSS_RC_ERROR;

    return rc;
}

static vtss_rc transfer_dm_data_set(misc_url_parts_t *url_parts)
{
    system_conf_t               system_conf;
    time_t                      now;
    vtss_rc                     rc        = VTSS_RC_ERROR;
    BOOL                        save_ok   = FALSE;
    char                        *buf_ptr = NULL;
    int                         ct, path_len;

    BOOL                        status = FALSE;
    vtss_perf_mon_dm_info_t     data[PM_DM_DATA_SET_LIMIT];
    u32                         idx;
    char                        tmp_buf[PM_BUF_LENGTH];
    char                        *p;
    int                         total = 0;

    if (!url_parts || ((size_t)(path_len = strlen(url_parts->path)) >= sizeof(url_parts->path))) {
        return rc;
    }

    /* create filename */
    if (system_get_config(&system_conf) != VTSS_OK) {
        return rc;
    }

    now = time(NULL);
    if (now >= 0) {
        struct tm *timeinfo_p;

        perf_mon_util_adjust_time_tz(&now);
        timeinfo_p = localtime(&now);
        ct = snprintf(url_parts->path + path_len, sizeof(url_parts->path) - path_len, "%s%s_history_%04d-%02d-%02d_%02dh.%02dm.%02ds_DM_all.csv%s",
                      (path_len > 0 && (url_parts->path[path_len - 1] != '/')) ? "/" : "",
                      system_conf.sys_name,
                      timeinfo_p->tm_year + 1900,
                      timeinfo_p->tm_mon + 1,
                      timeinfo_p->tm_mday,
                      timeinfo_p->tm_hour,
                      timeinfo_p->tm_min,
                      timeinfo_p->tm_sec,
#ifdef PM_GZIP_COMPRESS
                      ".gz"
#else
                      ""
#endif /* PM_GZIP_COMPRESS */
                     );
    } else {
        ct = snprintf(url_parts->path + path_len, sizeof(url_parts->path) - path_len, "%s%s_history_no-time_DM_all.csv%s",
                      (path_len > 0 && (url_parts->path[path_len - 1] != '/')) ? "/" : "",
                      system_conf.sys_name,
#ifdef PM_GZIP_COMPRESS
                      ".gz"
#else
                      ""
#endif /* PM_GZIP_COMPRESS */
                     );
    }

    if ((ct < 0) || ((size_t)(ct + path_len) >= sizeof(url_parts->path))) {
        return rc;
    }

    if ((buf_ptr = (char *)VTSS_MALLOC(PM_ALLOCATE_DM_BUFFER)) == NULL) {
        T_E("%% Not enough free RAM memory for PM save operation (needed %u bytes).\n", PM_ALLOCATE_DM_BUFFER);
        return rc;
    }

    /* read data from DB */
    memset(buf_ptr, 0x0, PM_ALLOCATE_DM_BUFFER);
    p = buf_ptr;
    total = 0;

    // create file header
    memset(tmp_buf, 0, sizeof(tmp_buf));
    ct = snprintf(tmp_buf, sizeof(tmp_buf), "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                  "interval_id",
                  "instance_id",
                  "port",
                  "priority",
                  "rate",
                  "unit",
                  "tx_count",
                  "rx_count",
                  "oneway_F2N_average",
                  "oneway_F2N_variation",
                  "oneway_F2N_min",
                  "oneway_F2N_max",
                  "oneway_N2F_average",
                  "oneway_N2F_variation",
                  "oneway_N2F_min",
                  "oneway_N2F_max",
                  "twoway_F2N_average",
                  "twoway_F2N_variation",
                  "twoway_F2N_min",
                  "twoway_F2N_max",
                  "domain",
                  "direction",
                  "level",
                  "flow",
                  "vid",
                  "mep_id",
                  "mac",
                  "peer_mep_id",
                  "peer_mac");

    if (ct) {
        memcpy(p, tmp_buf, ct);
        p += ct;
        total = total + ct;
    }

    // read data
    status = vtss_perf_mon_dm_data_get_first(&data[0]);
    while (status) {
        for (idx = 0; idx < PM_DM_DATA_SET_LIMIT; idx++) {

            if (data[idx].measurement_interval_id == 0) {
                break;
            }

            memset(tmp_buf, 0, sizeof(tmp_buf));
            ct = snprintf(tmp_buf, sizeof(tmp_buf), "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%s,%s,%u,%u,%u,%u,%02x-%02x-%02x-%02x-%02x-%02x,%u,%02x-%02x-%02x-%02x-%02x-%02x\n",
                          data[idx].measurement_interval_id,
                          evc_id_i2u(data[idx].mep_instance),
                          iport2uport(data[idx].mep_residence_port),
                          data[idx].tx_priority,
                          data[idx].tx_rate,
                          data[idx].measurement_unit,
                          data[idx].tx_cnt,
                          data[idx].rx_cnt,
                          data[idx].far_to_near_avg_delay,
                          data[idx].far_to_near_avg_delay_variation,
                          data[idx].far_to_near_min_delay,
                          data[idx].far_to_near_max_delay,
                          data[idx].near_to_far_avg_delay,
                          data[idx].near_to_far_avg_delay_variation,
                          data[idx].near_to_far_min_delay,
                          data[idx].near_to_far_max_delay,
                          data[idx].two_way_avg_delay,
                          data[idx].two_way_avg_delay_variation,
                          data[idx].two_way_min_delay,
                          data[idx].two_way_max_delay,
                          vtss_mep_domain_to_string(data[idx].mep_domain),
                          vtss_mep_direction_to_string(data[idx].mep_direction),
                          data[idx].mep_level,
                          (data[idx].mep_domain != VTSS_MEP_MGMT_VLAN) ? evc_id_i2u(data[idx].mep_flow) : data[idx].mep_flow,
                          data[idx].mep_vlan,
                          data[idx].mep_id,
                          data[idx].mep_mac[0],
                          data[idx].mep_mac[1],
                          data[idx].mep_mac[2],
                          data[idx].mep_mac[3],
                          data[idx].mep_mac[4],
                          data[idx].mep_mac[5],
                          data[idx].mep_peer_id,
                          data[idx].mep_peer_mac[0],
                          data[idx].mep_peer_mac[1],
                          data[idx].mep_peer_mac[2],
                          data[idx].mep_peer_mac[3],
                          data[idx].mep_peer_mac[4],
                          data[idx].mep_peer_mac[5]);

            if (ct) {
                memcpy(p, tmp_buf, ct);
                p += ct;
                total = total + ct;
            }
        }

        status = vtss_perf_mon_dm_data_get_next(&data[0]);
    }

    if (p) {
        *p = '\0';
    }

    /* save data to tftp or http */
    save_ok = save_config(buf_ptr, url_parts);

    VTSS_FREE(buf_ptr);

    rc = save_ok ? VTSS_RC_OK : VTSS_RC_ERROR;

    return rc;
}

static vtss_rc transfer_evc_data_set(misc_url_parts_t *url_parts)
{
    system_conf_t               system_conf;
    time_t                      now;
    vtss_rc                     rc        = VTSS_RC_ERROR;
    BOOL                        save_ok   = FALSE;
    char                        *buf_ptr = NULL;
    int                         ct, path_len;

    BOOL                        status = FALSE;
    vtss_perf_mon_evc_info_t    data[PM_EVC_DATA_SET_LIMIT];
    u32                         idx;
    char                        tmp_buf[PM_BUF_LENGTH];
    char                        *p;
    int                         total = 0;

    if (!url_parts || ((size_t)(path_len = strlen(url_parts->path)) >= sizeof(url_parts->path))) {
        return rc;
    }

    /* create filename */
    if (system_get_config(&system_conf) != VTSS_OK) {
        return rc;
    }

    now = time(NULL);
    if (now >= 0) {
        struct tm *timeinfo_p;

        perf_mon_util_adjust_time_tz(&now);
        timeinfo_p = localtime(&now);
        ct = snprintf(url_parts->path + path_len, sizeof(url_parts->path) - path_len, "%s%s_history_%04d-%02d-%02d_%02dh.%02dm.%02ds_EVC_all.csv%s",
                      (path_len > 0 && (url_parts->path[path_len - 1] != '/')) ? "/" : "",
                      system_conf.sys_name,
                      timeinfo_p->tm_year + 1900,
                      timeinfo_p->tm_mon + 1,
                      timeinfo_p->tm_mday,
                      timeinfo_p->tm_hour,
                      timeinfo_p->tm_min,
                      timeinfo_p->tm_sec,
#ifdef PM_GZIP_COMPRESS
                      ".gz"
#else
                      ""
#endif /* PM_GZIP_COMPRESS */
                     );
    } else {
        ct = snprintf(url_parts->path + path_len, sizeof(url_parts->path) - path_len, "%s%s_history_no-time_EVC_all.csv%s",
                      (path_len > 0 && (url_parts->path[path_len - 1] != '/')) ? "/" : "",
                      system_conf.sys_name,
#ifdef PM_GZIP_COMPRESS
                      ".gz"
#else
                      ""
#endif /* PM_GZIP_COMPRESS */
                     );
    }

    if ((ct < 0) || ((size_t)(ct + path_len) >= sizeof(url_parts->path))) {
        return rc;
    }

    if ((buf_ptr = (char *)VTSS_MALLOC(PM_ALLOCATE_EVC_BUFFER)) == NULL) {
        T_E("%% Not enough free RAM memory for PM save operation (needed %u bytes).\n", PM_ALLOCATE_EVC_BUFFER);
        return rc;
    }

    /* read data from DB */
    memset(buf_ptr, 0x0, PM_ALLOCATE_EVC_BUFFER);
    p = buf_ptr;
    total = 0;

    // create file header
    memset(tmp_buf, 0, sizeof(tmp_buf));
    ct = snprintf(tmp_buf, sizeof(tmp_buf), "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                  "interval_id",
                  "instance_id",
                  "port",
                  "green_f_rx",
                  "green_f_tx",
                  "green_b_rx",
                  "green_b_tx",
                  "yellow_f_rx",
                  "yellow_f_tx",
                  "yellow_b_rx",
                  "yellow_b_tx",
                  "red_f_rx",
                  "red_b_tx",
                  "discard_f_rx",
                  "discard_f_tx",
                  "discard_b_rx",
                  "discard_b_tx");

    if (ct) {
        memcpy(p, tmp_buf, ct);
        p += ct;
        total = total + ct;
    }

    // read data
    status = vtss_perf_mon_evc_data_get_first(&data[0]);
    while (status) {
        for (idx = 0; idx < PM_EVC_DATA_SET_LIMIT; idx++) {

            if (data[idx].measurement_interval_id == 0) {
                break;
            }

            memset(tmp_buf, 0, sizeof(tmp_buf));
            ct = snprintf(tmp_buf, sizeof(tmp_buf), "%u,%u,%u,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n",
                          data[idx].measurement_interval_id,
                          evc_id_i2u(data[idx].evc_instance),
                          iport2uport(data[idx].evc_port),
                          data[idx].rx_green,
                          data[idx].tx_green,
                          data[idx].rx_green_b,
                          data[idx].tx_green_b,
                          data[idx].rx_yellow,
                          data[idx].tx_yellow,
                          data[idx].rx_yellow_b,
                          data[idx].tx_yellow_b,
                          data[idx].rx_red,
                          data[idx].tx_red,
                          data[idx].rx_discard,
                          data[idx].tx_discard,
                          data[idx].rx_discard_b,
                          data[idx].tx_discard_b);

            if (ct) {
                memcpy(p, tmp_buf, ct);
                p += ct;
                total = total + ct;
            }
        }

        status = vtss_perf_mon_evc_data_get_next(&data[0]);
    }

    if (p) {
        *p = '\0';
    }

    /* save data to tftp or http */
    save_ok = save_config(buf_ptr, url_parts);

    VTSS_FREE(buf_ptr);

    rc = save_ok ? VTSS_RC_OK : VTSS_RC_ERROR;

    return rc;
}

static vtss_rc transfer_ece_data_set(misc_url_parts_t *url_parts)
{
    system_conf_t               system_conf;
    time_t                      now;
    vtss_rc                     rc        = VTSS_RC_ERROR;
    BOOL                        save_ok   = FALSE;
    char                        *buf_ptr = NULL;
    int                         ct, path_len;

    BOOL                        status = FALSE;
    vtss_perf_mon_evc_info_t    data[PM_ECE_DATA_SET_LIMIT];
    u32                         idx;
    char                        tmp_buf[PM_BUF_LENGTH];
    char                        *p;
    int                         total = 0;

    if (!url_parts || ((size_t)(path_len = strlen(url_parts->path)) >= sizeof(url_parts->path))) {
        return rc;
    }

    /* create filename */
    if (system_get_config(&system_conf) != VTSS_OK) {
        return rc;
    }

    now = time(NULL);
    if (now >= 0) {
        struct tm *timeinfo_p;

        perf_mon_util_adjust_time_tz(&now);
        timeinfo_p = localtime(&now);
        ct = snprintf(url_parts->path + path_len, sizeof(url_parts->path) - path_len, "%s%s_history_%04d-%02d-%02d_%02dh.%02dm.%02ds_ECE_all.csv%s",
                      (path_len > 0 && (url_parts->path[path_len - 1] != '/')) ? "/" : "",
                      system_conf.sys_name,
                      timeinfo_p->tm_year + 1900,
                      timeinfo_p->tm_mon + 1,
                      timeinfo_p->tm_mday,
                      timeinfo_p->tm_hour,
                      timeinfo_p->tm_min,
                      timeinfo_p->tm_sec,
#ifdef PM_GZIP_COMPRESS
                      ".gz"
#else
                      ""
#endif /* PM_GZIP_COMPRESS */
                     );
    } else {
        ct = snprintf(url_parts->path + path_len, sizeof(url_parts->path) - path_len, "%s%s_history_no-time_ECE_all.csv%s",
                      (path_len > 0 && (url_parts->path[path_len - 1] != '/')) ? "/" : "",
                      system_conf.sys_name,
#ifdef PM_GZIP_COMPRESS
                      ".gz"
#else
                      ""
#endif /* PM_GZIP_COMPRESS */
                     );
    }

    if ((ct < 0) || ((size_t)(ct + path_len) >= sizeof(url_parts->path))) {
        return rc;
    }

    if ((buf_ptr = (char *)VTSS_MALLOC(PM_ALLOCATE_ECE_BUFFER)) == NULL) {
        T_E("%% Not enough free RAM memory for PM save operation (needed %u bytes).\n", PM_ALLOCATE_ECE_BUFFER);
        return rc;
    }

    /* read data from DB */
    memset(buf_ptr, 0x0, PM_ALLOCATE_ECE_BUFFER);
    p = buf_ptr;
    total = 0;

    // create file header
    memset(tmp_buf, 0, sizeof(tmp_buf));
    ct = snprintf(tmp_buf, sizeof(tmp_buf), "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
                  "interval_id",
                  "instance_id",
                  "port",
                  "green_f_rx",
                  "green_f_tx",
                  "green_b_rx",
                  "green_b_tx",
                  "yellow_f_rx",
                  "yellow_f_tx",
                  "yellow_b_rx",
                  "yellow_b_tx",
                  "red_f_rx",
                  "red_b_tx",
                  "discard_f_rx",
                  "discard_f_tx",
                  "discard_b_rx",
                  "discard_b_tx");

    if (ct) {
        memcpy(p, tmp_buf, ct);
        p += ct;
        total = total + ct;
    }

    // read data
    status = vtss_perf_mon_ece_data_get_first(&data[0]);
    while (status) {
        for (idx = 0; idx < PM_ECE_DATA_SET_LIMIT; idx++) {

            if (data[idx].measurement_interval_id == 0) {
                break;
            }

            memset(tmp_buf, 0, sizeof(tmp_buf));
            ct = snprintf(tmp_buf, sizeof(tmp_buf), "%u,%u,%u,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu\n",
                          data[idx].measurement_interval_id,
                          evc_id_i2u(data[idx].evc_instance),
                          iport2uport(data[idx].evc_port),
                          data[idx].rx_green,
                          data[idx].tx_green,
                          data[idx].rx_green_b,
                          data[idx].tx_green_b,
                          data[idx].rx_yellow,
                          data[idx].tx_yellow,
                          data[idx].rx_yellow_b,
                          data[idx].tx_yellow_b,
                          data[idx].rx_red,
                          data[idx].tx_red,
                          data[idx].rx_discard,
                          data[idx].tx_discard,
                          data[idx].rx_discard_b,
                          data[idx].tx_discard_b);

            if (ct) {
                memcpy(p, tmp_buf, ct);
                p += ct;
                total = total + ct;
            }
        }

        status = vtss_perf_mon_ece_data_get_next(&data[0]);
    }

    if (p) {
        *p = '\0';
    }

    /* save data to tftp or http */
    save_ok = save_config(buf_ptr, url_parts);

    VTSS_FREE(buf_ptr);

    rc = save_ok ? VTSS_RC_OK : VTSS_RC_ERROR;

    return rc;
}

/**
  * \brief PM sending colloected statistics functions
  *
  * \return
  *   FALSE on any error, otherwise TRUE.
  */
BOOL vtss_perf_mon_transfer_lm_data_set(void *pm_url_dir)
{
    misc_url_parts_t    *url_parts;

    if (!pm_url_dir) {
        return FALSE;
    }

    url_parts = (misc_url_parts_t *)pm_url_dir;
    return (transfer_lm_data_set(url_parts) == VTSS_RC_OK);
}

BOOL vtss_perf_mon_transfer_dm_data_set(void *pm_url_dir)
{
    misc_url_parts_t    *url_parts;

    if (!pm_url_dir) {
        return FALSE;
    }

    url_parts = (misc_url_parts_t *)pm_url_dir;
    return (transfer_dm_data_set(url_parts) == VTSS_RC_OK);
}

BOOL vtss_perf_mon_transfer_evc_data_set(void *pm_url_dir)
{
    misc_url_parts_t    *url_parts;

    if (!pm_url_dir) {
        return FALSE;
    }

    url_parts = (misc_url_parts_t *)pm_url_dir;
    return (transfer_evc_data_set(url_parts) == VTSS_RC_OK);
}

BOOL vtss_perf_mon_transfer_ece_data_set(void *pm_url_dir)
{
    misc_url_parts_t    *url_parts;

    if (!pm_url_dir) {
        return FALSE;
    }

    url_parts = (misc_url_parts_t *)pm_url_dir;
    return (transfer_ece_data_set(url_parts) == VTSS_RC_OK);
}

/**
  * \brief Get Random Number
  *
  * \return
  *   hour index id (u32).
  */
static int
perf_mon_arc4random(void)
{
    cyg_uint32 res;
    static unsigned long seed = 0xDEADB00B;
    HAL_CLOCK_READ(&res);  // Not so bad... (but often 0..N where N is small)
    seed = ((seed & 0x007F00FF) << 7) ^
           ((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
           (res << 13) ^ (res >> 9);    // using the clock too!
    return (int)seed;
}

/**
  * \brief Get current hour index
  *
  * \return
  *   hour index id (u32).
  */
static u32 perf_mon_current_hour_idx_get(void)
{
    struct tm   *timeinfo_p;
    int         now;

    now = time(NULL);
    perf_mon_util_adjust_time_tz(&now);
    timeinfo_p = localtime(&now);

    return timeinfo_p->tm_hour;
}

/**
  * \brief Get current minutes index
  *
  * \return
  *   minute index id (u32).
  */
static u32 perf_mon_current_minutes_idx_get(void)
{
    struct tm   *timeinfo_p;
    int         now;

    now = time(NULL);
    perf_mon_util_adjust_time_tz(&now);
    timeinfo_p = localtime(&now);

    if (timeinfo_p->tm_min == 0) {
        return 0;
    } else if (timeinfo_p->tm_min == 15) {
        return 1;
    } else if (timeinfo_p->tm_min == 30) {
        return 2;
    } else if (timeinfo_p->tm_min == 45) {
        return 3;
    }

    return 99;
}

/**
  * \brief Get current seconds index
  *
  * \return
  *   minute index id (u32).
  */
static u32 perf_mon_current_seconds_idx_get(void)
{
    struct tm   *timeinfo_p;
    int         now;

    now = time(NULL);
    perf_mon_util_adjust_time_tz(&now);
    timeinfo_p = localtime(&now);

    return timeinfo_p->tm_sec;
}

/**
  * \brief Calculate the random offset by user setting
  *
  * \return
  *   random id (u16).
  */
static u16 perf_mon_cal_random_offset(u16 offset)
{
    return (perf_mon_arc4random() % (offset + 1));
}

static char *perf_mon_data_type_txt(pm_data_t type)
{
    char    *txt;

    switch ( type ) {
    case PM_DATA_TYPE_LM:
        txt = "LM";
        break;
    case PM_DATA_TYPE_DM:
        txt = "DM";
        break;
    case PM_DATA_TYPE_EVC:
        txt = "EVC";
        break;
    case PM_DATA_TYPE_ECE:
        txt = "ECE";
        break;
    default:
        txt = "N/A";
        break;
    }

    return txt;
}

/**
  * \brief Calculate the interval slot
  *
  * \return
  *   slot id (u32).
  */
static u32 perf_mon_cal_interval_slot(pm_data_t pm_type, u32 id)
{
    if (id < PM_FIRST_INTERVAL_ID) {
        T_W("%s internal error on interval id %u !",
            perf_mon_data_type_txt(pm_type), id);
        return 0;
    }

    return ((id - 1) % PM_MEASUREMENT_INTERVAL_LIMIT);
}

/**
  * \brief Reset PERF_MON time tick
  *
  * \return
  *   None.
  */
static void perf_mon_time_tick_reset(void)
{
    perf_mon_time_tick_t *conf_p;

    PERF_MON_CRIT_ENTER();
    conf_p = &pm_global.pm_time;
    memset(conf_p, 0x0, sizeof(perf_mon_time_tick_t));
    PERF_MON_CRIT_EXIT();

    return;
}

/**
  * \brief Increase PERF_MON time tick
  *
  * \return
  *   None.
  */
static void perf_mon_time_tick_increase(void)
{
    perf_mon_time_tick_t *conf_p;

    PERF_MON_CRIT_ENTER();
    conf_p = &pm_global.pm_time;
    conf_p->lm_tick++;
    conf_p->dm_tick++;
    conf_p->evc_tick++;
    conf_p->ece_tick++;
    PERF_MON_CRIT_EXIT();

    return;
}

/**
  * \brief Set PERF_MON defaults
  *
  * \return
  *   None.
  */
static void perf_mon_default_set(perf_mon_conf_t *conf)
{
    /* reset default */
    memset(conf, 0x0, sizeof(perf_mon_conf_t));

    /* init default value on all intervals */
    conf->lm_interval = VTSS_PM_DEFAULT_LM_INTERVAL;
    conf->dm_interval = VTSS_PM_DEFAULT_DM_INTERVAL;
    conf->evc_interval = VTSS_PM_DEFAULT_EVC_INTERVAL;
    conf->ece_interval = VTSS_PM_DEFAULT_ECE_INTERVAL;

    conf->transfer_scheduled_random_offset = VTSS_PM_DEFAULT_RANDOM_OFFSET;
    conf->transfer_scheduled_offset = VTSS_PM_DEFAULT_FIXED_OFFSET;

    conf->transfer_interval_num = VTSS_PM_DEFAULT_NUM_OF_INTERVAL;

    return;
}

/**
  * \brief Set PERF_MON default action
  *
  * \return
  *   None.
  */
static void perf_mon_default_conf_action(void)
{
    perf_mon_conf_t *conf_p;

    PERF_MON_CRIT_ENTER();
    conf_p = &pm_global.perf_mon_conf;
    perf_mon_default_set(conf_p);
    PERF_MON_CRIT_EXIT();

    /* reset time tick on pm module */
    perf_mon_time_tick_reset();

    return;
}

/**
  * \brief Collect data from loss measurement
  *
  * \return
  *   TRUE: at least one entry is added
  *   FALSE: no entry is added
  *   The value to indicate if the interval_id is still available
  */
static BOOL _perf_mon_lm_data_collect(u32 interval_id)
{
    vtss_mep_mgmt_conf_t        config;
    vtss_mep_mgmt_pm_conf_t     pm_config;
    vtss_mep_mgmt_lm_conf_t     lm_config;
    vtss_mep_mgmt_lm_state_t    lm_state;
    u32                         i;
    u8                          mac[6];
    perf_mon_lm_conf_t          *lm_p;
    u32                         pm_idx;
    u32                         limit = 0;

    T_D("enter");

    /* reset lm data on this measurement slot */
    lm_p = &pm_global.lm;
    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, interval_id);

    /* get data from lm module */
    for (i = 0; i < VTSS_MEP_INSTANCE_MAX; ++i) {
        if ((vtss_mep_mgmt_conf_get(i, mac, &config)) != VTSS_RC_OK) {
            continue;
        }
        if (config.enable) {
            if ((vtss_mep_mgmt_pm_conf_get(i, &pm_config) ) != VTSS_RC_OK) {
                continue;
            }
            if (!pm_config.enable) {
                continue;
            }
            if ((vtss_mep_mgmt_lm_conf_get(i, &lm_config) ) != VTSS_RC_OK) {
                continue;
            }
            if (lm_config.enable) {
                if ((vtss_mep_mgmt_lm_state_get (i, &lm_state) ) != VTSS_RC_OK) {
                    continue;
                }
                /* clear counter */
                if ((vtss_mep_mgmt_lm_state_clear_set (i) ) != VTSS_RC_OK) {
                    continue;
                }

                /*
                 * come here really have data to write
                 * clean the database when the first time
                 */
                if (limit == 0) {
                    memset(lm_p->lm_data_set[pm_idx], 0x0, PM_LM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_lm_info_t));
                }

                /* Handle the LM state information */
                if (limit < PM_LM_DATA_SET_LIMIT) {
                    lm_p->lm_data_set[pm_idx][limit].valid = TRUE;
                    lm_p->lm_data_set[pm_idx][limit].measurement_interval_id = interval_id;
                    lm_p->lm_data_set[pm_idx][limit].mep_instance = i;

                    lm_p->lm_data_set[pm_idx][limit].tx_cnt = lm_state.tx_counter;
                    lm_p->lm_data_set[pm_idx][limit].rx_cnt = lm_state.rx_counter;
                    lm_p->lm_data_set[pm_idx][limit].near_end_loss_cnt = lm_state.near_los_counter;
                    lm_p->lm_data_set[pm_idx][limit].far_end_loss_cnt = lm_state.far_los_counter;
                    lm_p->lm_data_set[pm_idx][limit].near_end_frame_loss_ratio = lm_state.near_los_ratio;
                    lm_p->lm_data_set[pm_idx][limit].far_end_loss_ratio = lm_state.far_los_ratio;

                    lm_p->lm_data_set[pm_idx][limit].tx_rate = lm_config.period;
                    lm_p->lm_data_set[pm_idx][limit].tx_priority = lm_config.prio;

                    lm_p->lm_data_set[pm_idx][limit].mep_flow = config.flow;
                    lm_p->lm_data_set[pm_idx][limit].mep_vlan = config.vid;
                    lm_p->lm_data_set[pm_idx][limit].mep_residence_port = config.port;
                    lm_p->lm_data_set[pm_idx][limit].mep_direction = config.direction;
                    lm_p->lm_data_set[pm_idx][limit].mep_domain = config.domain;
                    lm_p->lm_data_set[pm_idx][limit].mep_level = config.level;
                    lm_p->lm_data_set[pm_idx][limit].mep_id = config.mep;
                    lm_p->lm_data_set[pm_idx][limit].mep_peer_id = config.peer_mep[0];

                    memcpy(lm_p->lm_data_set[pm_idx][limit].mep_mac,
                           mac,
                           sizeof(mac));
                    memcpy(lm_p->lm_data_set[pm_idx][limit].mep_peer_mac,
                           config.peer_mac[0],
                           sizeof(mac));
                    limit++;

                    T_D("%u/%u/%u/%u/%u/%u/%u",
                        i,
                        lm_state.tx_counter,
                        lm_state.rx_counter,
                        lm_state.near_los_counter,
                        lm_state.far_los_counter,
                        lm_state.near_los_ratio,
                        lm_state.far_los_ratio);
                } else {
                    break;  // no slot to store counters
                }
            }
        }
    }

    T_D("exit");
    if (limit) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
  * \brief Collect data from delay measurement
  *
  * \return
  *   TRUE: at least one entry is added
  *   FALSE: no entry is added
  *   The value to indicate if the interval_id is still available
  */
static BOOL _perf_mon_dm_data_collect(u32 interval_id)
{
    vtss_mep_mgmt_conf_t        config;
    vtss_mep_mgmt_pm_conf_t     pm_config;
    vtss_mep_mgmt_dm_conf_t     dm_config;
    vtss_mep_mgmt_dm_state_t    dm_state, dm_state_f_to_n, dm_state_n_to_f;
    u32                         i;
    u8                          mac[6];
    perf_mon_dm_conf_t          *dm_p;
    u32                         pm_idx;
    u32                         limit = 0;

    T_D("enter");

    /* reset dm data on this measurement slot */
    dm_p = &pm_global.dm;
    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, interval_id);

    for (i = 0; i < VTSS_MEP_INSTANCE_MAX; ++i) {
        if ((vtss_mep_mgmt_conf_get(i, mac, &config)) != VTSS_RC_OK) {
            continue;
        }
        if (config.enable) {
            if ((vtss_mep_mgmt_pm_conf_get(i, &pm_config) ) != VTSS_RC_OK) {
                continue;
            }
            if (!pm_config.enable) {
                continue;
            }
            if ((vtss_mep_mgmt_dm_conf_get(i, &dm_config) ) != VTSS_RC_OK) {
                continue;
            }
            if (dm_config.enable) {
                if ((vtss_mep_mgmt_dm_state_get (i, &dm_state, &dm_state_f_to_n, &dm_state_n_to_f) ) != VTSS_RC_OK) {
                    continue;
                }
                /* clear counter */
                if ((vtss_mep_mgmt_dm_state_clear_set (i) ) != VTSS_RC_OK) {
                    continue;
                }

                /*
                 * come here really have data to write
                 * clean the database when the first time
                 */
                if (limit == 0) {
                    memset(dm_p->dm_data_set[pm_idx], 0x0, PM_DM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_dm_info_t));
                }
                /* Handle the DM state information */
                if (limit < PM_DM_DATA_SET_LIMIT) {
                    dm_p->dm_data_set[pm_idx][limit].valid = TRUE;
                    dm_p->dm_data_set[pm_idx][limit].measurement_interval_id = interval_id;
                    dm_p->dm_data_set[pm_idx][limit].mep_instance = i;

                    dm_p->dm_data_set[pm_idx][limit].tx_cnt = dm_state.tx_cnt;
                    dm_p->dm_data_set[pm_idx][limit].rx_cnt = dm_state.rx_cnt;
                    dm_p->dm_data_set[pm_idx][limit].two_way_avg_delay = dm_state.avg_delay;
                    dm_p->dm_data_set[pm_idx][limit].two_way_avg_delay_variation = dm_state.avg_delay_var;
                    dm_p->dm_data_set[pm_idx][limit].two_way_max_delay = dm_state.max_delay;
                    dm_p->dm_data_set[pm_idx][limit].two_way_min_delay = dm_state.min_delay;
                    dm_p->dm_data_set[pm_idx][limit].far_to_near_avg_delay = dm_state_f_to_n.avg_delay;
                    dm_p->dm_data_set[pm_idx][limit].far_to_near_avg_delay_variation = dm_state_f_to_n.avg_delay_var;
                    dm_p->dm_data_set[pm_idx][limit].far_to_near_max_delay = dm_state_f_to_n.max_delay;
                    dm_p->dm_data_set[pm_idx][limit].far_to_near_min_delay = dm_state_f_to_n.min_delay;
                    dm_p->dm_data_set[pm_idx][limit].near_to_far_avg_delay = dm_state_n_to_f.avg_delay;
                    dm_p->dm_data_set[pm_idx][limit].near_to_far_avg_delay_variation = dm_state_n_to_f.avg_delay_var;
                    dm_p->dm_data_set[pm_idx][limit].near_to_far_max_delay = dm_state_n_to_f.max_delay;
                    dm_p->dm_data_set[pm_idx][limit].near_to_far_min_delay = dm_state_n_to_f.min_delay;

                    dm_p->dm_data_set[pm_idx][limit].tx_rate = dm_config.interval; //??
                    dm_p->dm_data_set[pm_idx][limit].tx_priority = dm_config.prio;
                    dm_p->dm_data_set[pm_idx][limit].measurement_unit = dm_config.tunit;

                    dm_p->dm_data_set[pm_idx][limit].mep_flow = config.flow;
                    dm_p->dm_data_set[pm_idx][limit].mep_vlan = config.vid;
                    dm_p->dm_data_set[pm_idx][limit].mep_residence_port = config.port;
                    dm_p->dm_data_set[pm_idx][limit].mep_direction = config.direction;
                    dm_p->dm_data_set[pm_idx][limit].mep_domain = config.domain;
                    dm_p->dm_data_set[pm_idx][limit].mep_level = config.level;
                    dm_p->dm_data_set[pm_idx][limit].mep_id = config.mep;
                    dm_p->dm_data_set[pm_idx][limit].mep_peer_id = config.peer_mep[0];

                    memcpy(dm_p->dm_data_set[pm_idx][limit].mep_mac,
                           mac,
                           sizeof(mac));
                    memcpy(dm_p->dm_data_set[pm_idx][limit].mep_peer_mac,
                           config.peer_mac[0],
                           sizeof(mac));

                    limit++;

                    T_D("%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u/%u",
                        i,
                        dm_state.tx_cnt,
                        dm_state.rx_err_cnt,
                        dm_state.ovrflw_cnt,
                        dm_state.late_txtime,
                        dm_state.avg_delay,
                        dm_state.avg_n_delay,
                        dm_state.avg_delay_var,
                        dm_state.avg_n_delay_var,
                        dm_state.min_delay,
                        dm_state.max_delay,
                        dm_state.min_delay_var,
                        dm_state.max_delay_var);
                } else {
                    break;  // no slot to store counters
                }
            }
        }
    }

    T_D("exit");
    if (limit) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
  * \brief Collect data from EVC statistics
  *
  * \return
  *   TRUE: at least one entry is added
  *   FALSE: no entry is added
  *   The value to indicate if the interval_id is still available
  */
static BOOL _perf_mon_evc_data_collect(u32 interval_id)
{
    vtss_evc_id_t           evc_id;
    evc_mgmt_conf_t         conf;
    evc_mgmt_ece_conf_t     ece_conf;
    port_iter_t             pit;
    vtss_port_no_t          iport = VTSS_PORT_NO_START;
    vtss_evc_counters_t     counters;
    perf_mon_evc_conf_t     *evc_p;
    u32                     pm_idx;
    u32                     limit = 0;

    /* reset evc data on this measurement slot */
    evc_p = &pm_global.evc;
    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, interval_id);

    evc_id = EVC_ID_FIRST;
    while (evc_mgmt_get(&evc_id, &conf, TRUE) == VTSS_OK) {
        /* In this loop you will get all created EVC */

        ece_conf.conf.id = EVC_ECE_ID_FIRST;
        while (evc_mgmt_ece_get(ece_conf.conf.id, &ece_conf, TRUE) == VTSS_OK) {
            /* In this loop you will get all created ECE */

            if (ece_conf.conf.action.evc_id != evc_id) {
                /* This ECE is related to EVC ．evc_id・ */
                continue;
            }

            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                /* This loop will find all ports related to this EVC ．evc_id・ */
                iport = pit.iport;
                if (ece_conf.conf.key.port_list[iport] != VTSS_ECE_PORT_NONE) {
                    conf.conf.network.pb.nni[iport] = TRUE;
                }
            }
        }

        port_iter_init_local(&pit);
        while (port_iter_getnext(&pit)) {
            /* This loop will collect counters for all ports related to this EVC */
            iport = pit.iport;
            if (!conf.conf.network.pb.nni[iport] ||
                vtss_evc_counters_get(NULL, evc_id, iport, &counters) != VTSS_OK) {
                continue;
            }

            /*
             * come here really have data to write
             * clean the database when the first time
             */
            if (limit == 0) {
                memset(evc_p->evc_data_set[pm_idx], 0x0, PM_EVC_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
            }

            /* Handle the EVC state information */
            if (limit < PM_EVC_DATA_SET_LIMIT) {
                evc_p->evc_data_set[pm_idx][limit].valid = TRUE;
                evc_p->evc_data_set[pm_idx][limit].measurement_interval_id = interval_id;
                evc_p->evc_data_set[pm_idx][limit].evc_instance = evc_id;
                evc_p->evc_data_set[pm_idx][limit].evc_port = iport;

                evc_p->evc_data_set[pm_idx][limit].rx_green = counters.rx_green.frames;
                evc_p->evc_data_set[pm_idx][limit].rx_yellow = counters.rx_yellow.frames;
                evc_p->evc_data_set[pm_idx][limit].rx_red = counters.rx_red.frames;
                evc_p->evc_data_set[pm_idx][limit].rx_discard = counters.rx_discard.frames;
                evc_p->evc_data_set[pm_idx][limit].tx_green = counters.tx_green.frames;
                evc_p->evc_data_set[pm_idx][limit].tx_yellow = counters.tx_yellow.frames;
                evc_p->evc_data_set[pm_idx][limit].tx_red = 0;
                evc_p->evc_data_set[pm_idx][limit].tx_discard = counters.tx_discard.frames;
                evc_p->evc_data_set[pm_idx][limit].rx_green_b = counters.rx_green.bytes;
                evc_p->evc_data_set[pm_idx][limit].rx_yellow_b = counters.rx_yellow.bytes;
                evc_p->evc_data_set[pm_idx][limit].rx_red_b = counters.rx_red.bytes;
                evc_p->evc_data_set[pm_idx][limit].rx_discard_b = counters.rx_discard.bytes;
                evc_p->evc_data_set[pm_idx][limit].tx_green_b = counters.tx_green.bytes;
                evc_p->evc_data_set[pm_idx][limit].tx_yellow_b = counters.tx_yellow.bytes;
                evc_p->evc_data_set[pm_idx][limit].tx_red_b = 0;
                evc_p->evc_data_set[pm_idx][limit].tx_discard_b = counters.tx_discard.bytes;

                limit++;

                T_D("%u/%u/%u/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu",
                    evc_id,
                    ece_conf.conf.id,
                    iport,
                    counters.rx_green.frames,
                    counters.tx_green.frames,
                    counters.rx_green.bytes,
                    counters.tx_green.bytes,
                    counters.rx_yellow.frames,
                    counters.tx_yellow.frames,
                    counters.rx_yellow.bytes,
                    counters.tx_yellow.bytes,
                    counters.rx_red.frames,
                    counters.rx_red.bytes,
                    counters.rx_discard.frames,
                    counters.tx_discard.frames,
                    counters.rx_discard.bytes,
                    counters.tx_discard.bytes);
            } else {
                break;  // no slot to store counters
            }

            /* clear evc port counter */
            (void) vtss_evc_counters_clear(NULL, evc_id, iport);
        }
    } /* End of EVC loop */

    if (limit) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
  * \brief Collect data from ECE statistics
  *
  * \return
  *   TRUE: at least one entry is added
  *   FALSE: no entry is added
  *   The value to indicate if the interval_id is still available
  */
static BOOL _perf_mon_ece_data_collect(u32 interval_id)
{
    //vtss_ece_id_t           ece_id = 0;
    evc_mgmt_ece_conf_t     ece_conf;
    vtss_rc                 rc = VTSS_RC_ERROR;
    vtss_evc_id_t           evc_id;
    vtss_port_no_t          iport = VTSS_PORT_NO_START;
    port_iter_t             pit;
    evc_mgmt_conf_t         evc_conf;
    vtss_evc_counters_t     counters;
    perf_mon_ece_conf_t     *ece_p;
    u32                     pm_idx;
    u32                     limit = 0;

    //ece_id = EVC_ECE_ID_FIRST;
    //rc = evc_mgmt_ece_get(ece_id, &ece_conf, TRUE);

    /* reset ece data on this measurement slot */
    ece_p = &pm_global.ece;
    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, interval_id);

    ece_conf.conf.id = EVC_ECE_ID_FIRST;
    while ((rc = evc_mgmt_ece_get(ece_conf.conf.id, &ece_conf, TRUE)) == VTSS_OK) {
        /* In this loop you will get all created ECE */

        if (rc == VTSS_OK) {
            evc_id = ece_conf.conf.action.evc_id;
            if (evc_id == VTSS_EVC_ID_NONE || evc_mgmt_get(&evc_id, &evc_conf, 0) != VTSS_RC_OK) {
                memset(&evc_conf, 0, sizeof(evc_conf));
            }

            port_iter_init_local(&pit);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                if ((ece_conf.conf.key.port_list[iport] == VTSS_ECE_PORT_NONE && evc_conf.conf.network.pb.nni[iport] == FALSE) ||
                    vtss_ece_counters_get(NULL, ece_conf.conf.id, iport, &counters) != VTSS_OK) {
                    continue;
                }

                /*
                 * come here really have data to write
                 * clean the database when the first time
                 */
                if (limit == 0) {
                    memset(ece_p->ece_data_set[pm_idx], 0x0, PM_ECE_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
                }


                /* Handle the ECE state information */
                if (limit < PM_ECE_DATA_SET_LIMIT) {
                    ece_p->ece_data_set[pm_idx][limit].valid = TRUE;
                    ece_p->ece_data_set[pm_idx][limit].measurement_interval_id = interval_id;
                    ece_p->ece_data_set[pm_idx][limit].evc_instance = ece_conf.conf.id;
                    ece_p->ece_data_set[pm_idx][limit].evc_port = iport;

                    ece_p->ece_data_set[pm_idx][limit].rx_green = counters.rx_green.frames;
                    ece_p->ece_data_set[pm_idx][limit].rx_yellow = counters.rx_yellow.frames;
                    ece_p->ece_data_set[pm_idx][limit].rx_red = counters.rx_red.frames;
                    ece_p->ece_data_set[pm_idx][limit].rx_discard = counters.rx_discard.frames;
                    ece_p->ece_data_set[pm_idx][limit].tx_green = counters.tx_green.frames;
                    ece_p->ece_data_set[pm_idx][limit].tx_yellow = counters.tx_yellow.frames;
                    ece_p->ece_data_set[pm_idx][limit].tx_red = 0;
                    ece_p->ece_data_set[pm_idx][limit].tx_discard = counters.tx_discard.frames;
                    ece_p->ece_data_set[pm_idx][limit].rx_green_b = counters.rx_green.bytes;
                    ece_p->ece_data_set[pm_idx][limit].rx_yellow_b = counters.rx_yellow.bytes;
                    ece_p->ece_data_set[pm_idx][limit].rx_red_b = counters.rx_red.bytes;
                    ece_p->ece_data_set[pm_idx][limit].rx_discard_b = counters.rx_discard.bytes;
                    ece_p->ece_data_set[pm_idx][limit].tx_green_b = counters.tx_green.bytes;
                    ece_p->ece_data_set[pm_idx][limit].tx_yellow_b = counters.tx_yellow.bytes;
                    ece_p->ece_data_set[pm_idx][limit].tx_red_b = 0;
                    ece_p->ece_data_set[pm_idx][limit].tx_discard_b = counters.tx_discard.bytes;

                    limit++;

                    T_D("%u/%u/%u/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu/%llu",
                        evc_id,
                        ece_conf.conf.id,
                        iport,
                        counters.rx_green.frames,
                        counters.tx_green.frames,
                        counters.rx_green.bytes,
                        counters.tx_green.bytes,
                        counters.rx_yellow.frames,
                        counters.tx_yellow.frames,
                        counters.rx_yellow.bytes,
                        counters.tx_yellow.bytes,
                        counters.rx_red.frames,
                        counters.rx_red.bytes,
                        counters.rx_discard.frames,
                        counters.tx_discard.frames,
                        counters.rx_discard.bytes,
                        counters.tx_discard.bytes);
                } else {
                    break;  // no slot to store counters
                }

                /* clear ece port counter */
                (void) vtss_ece_counters_clear(NULL, ece_conf.conf.id, iport);
            }
        }
    }

    if (limit) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
  * \brief Write LM reports on the FLASH
  *
  * \return
  *   (vtss_rc)
  */
static vtss_rc _perf_mon_lm_report_write(void)
{
    vtss_rc                     rc = VTSS_OK;
    conf_blk_id_t               blk_id = CONF_BLK_PM_LM_REPORTS;
    perf_mon_lm_conf_blk_t      *perf_mon_lm_conf_blk_p;
    perf_mon_lm_conf_t          *ptr_src;
    perf_mon_lm_conf_t          *ptr_dst;
    u32                         size;

    T_D("enter");

    if ((perf_mon_lm_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
        size != sizeof(*perf_mon_lm_conf_blk_p)) {
        T_D("failed to open PM_LM report table");
        perf_mon_lm_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*perf_mon_lm_conf_blk_p));
        if (perf_mon_lm_conf_blk_p) {
            perf_mon_lm_conf_blk_p->version = PM_CURRENT_VERSION;
            ptr_src = &pm_global.lm;
            ptr_dst = &perf_mon_lm_conf_blk_p->lm_reports;
            memcpy(ptr_dst, ptr_src, sizeof(perf_mon_lm_conf_t));
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        } else {
            T_D("failed to create PM_LM report table");
            rc = PM_ERROR_CREATE_LM_FLASH_BLOCK;
        }
    } else {
        ptr_src = &pm_global.lm;
        ptr_dst = &perf_mon_lm_conf_blk_p->lm_reports;
        memcpy(ptr_dst, ptr_src, sizeof(perf_mon_lm_conf_t));
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);

        T_D("lm_current_interval_id = %u, lm_current_interval_cnt = %u", pm_global.lm.lm_current_interval_id, pm_global.lm.lm_current_interval_cnt);
    }

    T_D("exit, rc = %d", rc);
    return rc;
}

/**
  * \brief Write DM reports on the FLASH
  *
  * \return
  *   (vtss_rc)
  */
static vtss_rc _perf_mon_dm_report_write(void)
{
    vtss_rc                     rc = VTSS_OK;
    conf_blk_id_t               blk_id = CONF_BLK_PM_DM_REPORTS;
    perf_mon_dm_conf_blk_t      *perf_mon_dm_conf_blk_p;
    perf_mon_dm_conf_t          *ptr_src;
    perf_mon_dm_conf_t          *ptr_dst;
    u32                         size;

    T_D("enter");

    if ((perf_mon_dm_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
        size != sizeof(*perf_mon_dm_conf_blk_p)) {
        T_D("failed to open PM_DM report table");
        perf_mon_dm_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*perf_mon_dm_conf_blk_p));
        if (perf_mon_dm_conf_blk_p) {
            perf_mon_dm_conf_blk_p->version = PM_CURRENT_VERSION;
            ptr_src = &pm_global.dm;
            ptr_dst = &perf_mon_dm_conf_blk_p->dm_reports;
            memcpy(ptr_dst, ptr_src, sizeof(perf_mon_dm_conf_t));
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        } else {
            T_D("failed to create PM_DM report table");
            rc = PM_ERROR_CREATE_DM_FLASH_BLOCK;
        }
    } else {
        ptr_src = &pm_global.dm;
        ptr_dst = &perf_mon_dm_conf_blk_p->dm_reports;
        memcpy(ptr_dst, ptr_src, sizeof(perf_mon_dm_conf_t));
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    T_D("exit, rc = %d", rc);
    return rc;
}

/**
  * \brief Write EVC reports on the FLASH
  *
  * \return
  *   (vtss_rc)
  */
static vtss_rc _perf_mon_evc_report_write(void)
{
    vtss_rc                     rc = VTSS_OK;
    conf_blk_id_t               blk_id = CONF_BLK_PM_EVC_REPORTS;
    perf_mon_evc_conf_blk_t     *perf_mon_evc_conf_blk_p;
    perf_mon_evc_conf_t         *ptr_src;
    perf_mon_evc_conf_t         *ptr_dst;
    u32                         size;

    T_D("enter");

    if ((perf_mon_evc_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
        size != sizeof(*perf_mon_evc_conf_blk_p)) {
        T_D("failed to open PM_EVC report table");
        perf_mon_evc_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*perf_mon_evc_conf_blk_p));
        if (perf_mon_evc_conf_blk_p) {
            perf_mon_evc_conf_blk_p->version = PM_CURRENT_VERSION;
            ptr_src = &pm_global.evc;
            ptr_dst = &perf_mon_evc_conf_blk_p->evc_reports;
            memcpy(ptr_dst, ptr_src, sizeof(perf_mon_evc_conf_t));
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        } else {
            T_D("failed to create PM_EVC report table");
            rc = PM_ERROR_CREATE_EVC_FLASH_BLOCK;
        }
    } else {
        ptr_src = &pm_global.evc;
        ptr_dst = &perf_mon_evc_conf_blk_p->evc_reports;
        memcpy(ptr_dst, ptr_src, sizeof(perf_mon_evc_conf_t));
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    T_D("exit, rc = %d", rc);
    return rc;
}

/**
  * \brief Write ECE reports on the FLASH
  *
  * \return
  *   (vtss_rc)
  */
static vtss_rc _perf_mon_ece_report_write(void)
{
    vtss_rc                     rc = VTSS_OK;
    conf_blk_id_t               blk_id = CONF_BLK_PM_ECE_REPORTS;
    perf_mon_ece_conf_blk_t     *perf_mon_ece_conf_blk_p;
    perf_mon_ece_conf_t         *ptr_src;
    perf_mon_ece_conf_t         *ptr_dst;
    u32                         size;

    T_D("enter");

    if ((perf_mon_ece_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
        size != sizeof(*perf_mon_ece_conf_blk_p)) {
        T_D("failed to open PM_ECE report table");
        perf_mon_ece_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*perf_mon_ece_conf_blk_p));
        if (perf_mon_ece_conf_blk_p) {
            perf_mon_ece_conf_blk_p->version = PM_CURRENT_VERSION;
            ptr_src = &pm_global.ece;
            ptr_dst = &perf_mon_ece_conf_blk_p->ece_reports;
            memcpy(ptr_dst, ptr_src, sizeof(perf_mon_ece_conf_t));
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        } else {
            T_D("failed to create PM_ECE report table");
            rc = PM_ERROR_CREATE_ECE_FLASH_BLOCK;
        }
    } else {
        ptr_src = &pm_global.ece;
        ptr_dst = &perf_mon_ece_conf_blk_p->ece_reports;
        memcpy(ptr_dst, ptr_src, sizeof(perf_mon_ece_conf_t));
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    T_D("exit, rc = %d", rc);
    return rc;
}

/**
  * \brief Read LM reports from the FLASH
  *
  * \return
  *   (vtss_rc)
  */
static vtss_rc _perf_mon_lm_report_read(BOOL create)
{
    vtss_rc                     rc = VTSS_OK;
    conf_blk_id_t               blk_id = CONF_BLK_PM_LM_REPORTS;
    perf_mon_lm_conf_blk_t      *perf_mon_lm_conf_blk_p;
    perf_mon_lm_conf_t          *ptr_src;
    perf_mon_lm_conf_t          *ptr_dst;
    u32                         size;

    T_D("enter");

    if ((perf_mon_lm_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
        size != sizeof(*perf_mon_lm_conf_blk_p)) {
        T_D("failed to load PM_LM report table");
        rc = PM_ERROR_LOAD_LM_FLASH_BLOCK;
        if (create) {
            perf_mon_lm_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*perf_mon_lm_conf_blk_p));
            if (perf_mon_lm_conf_blk_p) {
                memset(perf_mon_lm_conf_blk_p, 0x0, sizeof(perf_mon_lm_conf_blk_t));
                perf_mon_lm_conf_blk_p->version = PM_CURRENT_VERSION;
                conf_sec_close(CONF_SEC_GLOBAL, blk_id);
            } else {
                T_D("failed to create PM_LM report table");
                rc = PM_ERROR_CREATE_LM_FLASH_BLOCK;
            }
        }
    } else {
        if (perf_mon_lm_conf_blk_p->version != PM_CURRENT_VERSION) {
            memset(&pm_global.lm, 0x0, sizeof(perf_mon_lm_conf_t));
            memset(perf_mon_lm_conf_blk_p, 0x0, sizeof(perf_mon_lm_conf_blk_t));
            perf_mon_lm_conf_blk_p->version = PM_CURRENT_VERSION;
        } else {
            ptr_dst = &pm_global.lm;
            ptr_src = &perf_mon_lm_conf_blk_p->lm_reports;
            memcpy(ptr_dst, ptr_src, sizeof(perf_mon_lm_conf_t));
        }
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);

        T_D("lm_current_interval_id = %u, lm_current_interval_cnt = %u", pm_global.lm.lm_current_interval_id, pm_global.lm.lm_current_interval_cnt);
    }

    T_D("exit, rc = %d", rc);
    return rc;
}

/**
  * \brief Read DM reports from the FLASH
  *
  * \return
  *   (vtss_rc)
  */
static vtss_rc _perf_mon_dm_report_read(BOOL create)
{
    vtss_rc                     rc = VTSS_OK;
    conf_blk_id_t               blk_id = CONF_BLK_PM_DM_REPORTS;
    perf_mon_dm_conf_blk_t      *perf_mon_dm_conf_blk_p;
    perf_mon_dm_conf_t          *ptr_src;
    perf_mon_dm_conf_t          *ptr_dst;
    u32                         size;

    T_D("enter");

    if ((perf_mon_dm_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
        size != sizeof(*perf_mon_dm_conf_blk_p)) {
        T_D("failed to load PM_DM report table");
        rc = PM_ERROR_LOAD_DM_FLASH_BLOCK;
        if (create) {
            perf_mon_dm_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*perf_mon_dm_conf_blk_p));
            if (perf_mon_dm_conf_blk_p) {
                memset(perf_mon_dm_conf_blk_p, 0x0, sizeof(perf_mon_dm_conf_blk_t));
                perf_mon_dm_conf_blk_p->version = PM_CURRENT_VERSION;
                conf_sec_close(CONF_SEC_GLOBAL, blk_id);
            } else {
                T_W("failed to create PM_DM report table");
                rc = PM_ERROR_CREATE_DM_FLASH_BLOCK;
            }
        }
    } else {
        if (perf_mon_dm_conf_blk_p->version != PM_CURRENT_VERSION) {
            memset(&pm_global.dm, 0x0, sizeof(perf_mon_dm_conf_t));
            memset(perf_mon_dm_conf_blk_p, 0x0, sizeof(perf_mon_dm_conf_blk_t));
            perf_mon_dm_conf_blk_p->version = PM_CURRENT_VERSION;
        } else {
            ptr_dst = &pm_global.dm;
            ptr_src = &perf_mon_dm_conf_blk_p->dm_reports;
            memcpy(ptr_dst, ptr_src, sizeof(perf_mon_dm_conf_t));
        }
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    T_D("exit, rc = %d", rc);
    return rc;
}

/**
  * \brief Read EVC reports from the FLASH
  *
  * \return
  *   (vtss_rc)
  */
static vtss_rc _perf_mon_evc_report_read(BOOL create)
{
    vtss_rc                     rc = VTSS_OK;
    conf_blk_id_t               blk_id = CONF_BLK_PM_EVC_REPORTS;
    perf_mon_evc_conf_blk_t     *perf_mon_evc_conf_blk_p;
    perf_mon_evc_conf_t         *ptr_src;
    perf_mon_evc_conf_t         *ptr_dst;
    u32                         size;

    T_D("enter");

    if ((perf_mon_evc_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
        size != sizeof(*perf_mon_evc_conf_blk_p)) {
        T_D("failed to load PM_EVC report table");
        rc = PM_ERROR_LOAD_EVC_FLASH_BLOCK;
        if (create) {
            perf_mon_evc_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*perf_mon_evc_conf_blk_p));
            if (perf_mon_evc_conf_blk_p) {
                memset(perf_mon_evc_conf_blk_p, 0x0, sizeof(perf_mon_evc_conf_blk_t));
                perf_mon_evc_conf_blk_p->version = PM_CURRENT_VERSION;
                conf_sec_close(CONF_SEC_GLOBAL, blk_id);
            } else {
                T_W("failed to create PM_EVC report table");
                rc = PM_ERROR_CREATE_EVC_FLASH_BLOCK;
            }
        }
    } else {
        if (perf_mon_evc_conf_blk_p->version != PM_CURRENT_VERSION) {
            memset(&pm_global.evc, 0x0, sizeof(perf_mon_evc_conf_t));
            memset(perf_mon_evc_conf_blk_p, 0x0, sizeof(perf_mon_evc_conf_blk_t));
            perf_mon_evc_conf_blk_p->version = PM_CURRENT_VERSION;
        } else {
            ptr_dst = &pm_global.evc;
            ptr_src = &perf_mon_evc_conf_blk_p->evc_reports;
            memcpy(ptr_dst, ptr_src, sizeof(perf_mon_evc_conf_t));
        }
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    T_D("exit, rc = %d", rc);
    return rc;
}

/**
  * \brief Read ECE reports from the FLASH
  *
  * \return
  *   (vtss_rc)
  */
static vtss_rc _perf_mon_ece_report_read(BOOL create)
{
    vtss_rc                     rc = VTSS_OK;
    conf_blk_id_t               blk_id = CONF_BLK_PM_ECE_REPORTS;
    perf_mon_ece_conf_blk_t     *perf_mon_ece_conf_blk_p;
    perf_mon_ece_conf_t         *ptr_src;
    perf_mon_ece_conf_t         *ptr_dst;
    u32                         size;

    T_D("enter");

    if ((perf_mon_ece_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
        size != sizeof(*perf_mon_ece_conf_blk_p)) {
        T_D("failed to load PM_ECE report table");
        rc = PM_ERROR_LOAD_ECE_FLASH_BLOCK;
        if (create) {
            perf_mon_ece_conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*perf_mon_ece_conf_blk_p));
            if (perf_mon_ece_conf_blk_p) {
                memset(perf_mon_ece_conf_blk_p, 0x0, sizeof(perf_mon_ece_conf_blk_t));
                perf_mon_ece_conf_blk_p->version = PM_CURRENT_VERSION;
                conf_sec_close(CONF_SEC_GLOBAL, blk_id);
            } else {
                T_W("failed to create PM_ECE report table");
                rc = PM_ERROR_CREATE_ECE_FLASH_BLOCK;
            }
        }
    } else {
        if (perf_mon_ece_conf_blk_p->version != PM_CURRENT_VERSION) {
            memset(&pm_global.ece, 0x0, sizeof(perf_mon_ece_conf_t));
            memset(perf_mon_ece_conf_blk_p, 0x0, sizeof(perf_mon_ece_conf_blk_t));
            perf_mon_ece_conf_blk_p->version = PM_CURRENT_VERSION;
        } else {
            ptr_dst = &pm_global.ece;
            ptr_src = &perf_mon_ece_conf_blk_p->ece_reports;
            memcpy(ptr_dst, ptr_src, sizeof(perf_mon_ece_conf_t));
        }
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }

    T_D("exit, rc = %d", rc);
    return rc;
}

/**
  * \brief Execute LM session and storage
  *
  * \return
  *   None.
  */
static void perf_mon_lm_check(void)
{
    perf_mon_conf_t         *conf_p;
    perf_mon_time_tick_t    *time_p;
    perf_mon_lm_conf_t      *lm_p;
    u32                     pm_idx;
    u32                     orig_id;

    T_N("enter");

    PERF_MON_CRIT_ENTER();

    conf_p = &pm_global.perf_mon_conf;
    if (!conf_p->lm_session_mode) {
        /* quit as the monitor is not eanbled */
        PERF_MON_CRIT_EXIT();
        return;
    }

    time_p = &pm_global.pm_time;
    lm_p = &pm_global.lm;

    if (time_p->lm_tick >= ( conf_p->lm_interval * TRANSFER_MINUTE_TO_SECOND )) {

        orig_id = lm_p->lm_current_interval_id;
        lm_p->lm_current_interval_id++;

        /* collect data */
        T_D("lm_collect_data, tick = %d", time_p->lm_tick);
        if (_perf_mon_lm_data_collect(lm_p->lm_current_interval_id) == FALSE) {
            /* No entry is added, the id is still available*/
            lm_p->lm_current_interval_id = orig_id;
        } else {
            /* record the interval information of measurement */
            if (lm_p->lm_current_interval_cnt < PM_MEASUREMENT_INTERVAL_LIMIT) {
                lm_p->lm_current_interval_cnt++;
            }

            /* Update interval */
            pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, lm_p->lm_current_interval_id);
            lm_p->lm_minfo[pm_idx].type = PM_DATA_TYPE_LM;
            lm_p->lm_minfo[pm_idx].measurement_interval_id = lm_p->lm_current_interval_id;    /* record this interval id */
            lm_p->lm_minfo[pm_idx].interval_end_time = time(NULL);                            /* record the end time in this interval */
            lm_p->lm_minfo[pm_idx].elapsed_time = time_p->lm_tick;                            /* record total seconds in this interval */

            if (conf_p->lm_storage_mode) {
                T_D("lm_storage_mode, tick = %d", time_p->lm_tick);
                (void) _perf_mon_lm_report_write();
            }
        }
        /* reset lm time tick */
        time_p->lm_tick = 0;
    }

    PERF_MON_CRIT_EXIT();

    T_N("exit");
}

/**
  * \brief Execute DM session and storage
  *
  * \return
  *   None.
  */
static void perf_mon_dm_check(void)
{
    perf_mon_conf_t         *conf_p;
    perf_mon_time_tick_t    *time_p;
    perf_mon_dm_conf_t      *dm_p;
    u32                     pm_idx;
    u32                     orig_id;

    T_N("enter");

    PERF_MON_CRIT_ENTER();

    conf_p = &pm_global.perf_mon_conf;
    if (!conf_p->dm_session_mode) {
        /* quit as the monitor is not eanbled */
        PERF_MON_CRIT_EXIT();
        return;
    }

    time_p = &pm_global.pm_time;
    dm_p = &pm_global.dm;

    if (time_p->dm_tick >= ( conf_p->dm_interval * TRANSFER_MINUTE_TO_SECOND )) {

        orig_id = dm_p->dm_current_interval_id;
        dm_p->dm_current_interval_id++;

        /* collect data */
        T_D("dm_collect data, tick = %d", time_p->dm_tick);
        if (_perf_mon_dm_data_collect(dm_p->dm_current_interval_id) == FALSE) {
            /* No entry is added, the id is still available*/
            dm_p->dm_current_interval_id = orig_id;
        } else {
            /* record the interval information of measurement */
            if (dm_p->dm_current_interval_cnt < PM_MEASUREMENT_INTERVAL_LIMIT) {
                /* record the interval information of measurement */
                dm_p->dm_current_interval_cnt++;
            }

            /* Update interval */
            pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, dm_p->dm_current_interval_id);
            dm_p->dm_minfo[pm_idx].type = PM_DATA_TYPE_DM;
            dm_p->dm_minfo[pm_idx].measurement_interval_id = dm_p->dm_current_interval_id;    /* record this interval id */
            dm_p->dm_minfo[pm_idx].interval_end_time = time(NULL);                            /* record the end time in this interval */
            dm_p->dm_minfo[pm_idx].elapsed_time = time_p->dm_tick;                            /* record total seconds in this interval */

            /* do action: dm storage */
            if (conf_p->dm_storage_mode) {
                T_D("dm_storage_mode, tick = %d", time_p->dm_tick);
                (void) _perf_mon_dm_report_write();
            }
        }

        /* reset dm time tick */
        time_p->dm_tick = 0;
    }

    PERF_MON_CRIT_EXIT();

    T_N("exit");
}

/**
  * \brief Execute EVC session and storage
  *
  * \return
  *   None.
  */
static void perf_mon_evc_check(void)
{
    perf_mon_conf_t         *conf_p;
    perf_mon_time_tick_t    *time_p;
    perf_mon_evc_conf_t     *evc_p;
    u32                     pm_idx;
    u32                     orig_id;

    T_N("enter");

    PERF_MON_CRIT_ENTER();

    conf_p = &pm_global.perf_mon_conf;
    if (!conf_p->evc_session_mode) {
        /* quit as the monitor is not eanbled */
        PERF_MON_CRIT_EXIT();
        return;
    }

    time_p = &pm_global.pm_time;
    evc_p = &pm_global.evc;

    if (time_p->evc_tick >= ( conf_p->evc_interval * TRANSFER_MINUTE_TO_SECOND )) {

        orig_id = evc_p->evc_current_interval_id;
        evc_p->evc_current_interval_id++;

        /* collect data */
        T_D("evc_collect data, tick = %d", time_p->evc_tick);
        if (_perf_mon_evc_data_collect(evc_p->evc_current_interval_id) == FALSE) {
            /* No entry is added, the id is still available*/
            evc_p->evc_current_interval_id = orig_id;
        } else {
            /* record the interval information of measurement */
            if (evc_p->evc_current_interval_cnt < PM_MEASUREMENT_INTERVAL_LIMIT) {
                evc_p->evc_current_interval_cnt++;
            }

            /* Update interval */
            pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, evc_p->evc_current_interval_id);
            evc_p->evc_minfo[pm_idx].type = PM_DATA_TYPE_EVC;
            evc_p->evc_minfo[pm_idx].measurement_interval_id = evc_p->evc_current_interval_id;   /* record this interval id */
            evc_p->evc_minfo[pm_idx].interval_end_time = time(NULL);                             /* record the end time in this interval */
            evc_p->evc_minfo[pm_idx].elapsed_time = time_p->evc_tick;                            /* record total seconds in this interval */

            /* do action: evc storage */
            if (conf_p->evc_storage_mode) {
                T_D("evc_storage_mode, tick = %d", time_p->evc_tick);
                (void) _perf_mon_evc_report_write();
            }
        }

        /* reset evc time tick */
        time_p->evc_tick = 0;
    }

    PERF_MON_CRIT_EXIT();

    T_N("exit");
}

/**
  * \brief Execute ECE session and storage
  *
  * \return
  *   None.
  */
static void perf_mon_ece_check(void)
{
    perf_mon_conf_t         *conf_p;
    perf_mon_time_tick_t    *time_p;
    perf_mon_ece_conf_t     *ece_p;
    u32                     pm_idx;
    u32                     orig_id;

    T_N("enter");

    PERF_MON_CRIT_ENTER();

    conf_p = &pm_global.perf_mon_conf;
    if (!conf_p->ece_session_mode) {
        /* quit as the monitor is not eanbled */
        PERF_MON_CRIT_EXIT();
        return;
    }

    time_p = &pm_global.pm_time;
    ece_p = &pm_global.ece;

    if (time_p->ece_tick >= ( conf_p->ece_interval * TRANSFER_MINUTE_TO_SECOND )) {

        orig_id = ece_p->ece_current_interval_id;
        ece_p->ece_current_interval_id++;

        /* collect data */
        T_D("ece_collect data, tick = %d", time_p->ece_tick);
        if (_perf_mon_ece_data_collect(ece_p->ece_current_interval_id) == FALSE) {
            /* No entry is added, the id is still available*/
            ece_p->ece_current_interval_id = orig_id;
        } else {
            /* record the interval information of measurement */
            if (ece_p->ece_current_interval_cnt < PM_MEASUREMENT_INTERVAL_LIMIT) {
                ece_p->ece_current_interval_cnt++;
            }

            pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, ece_p->ece_current_interval_id);
            ece_p->ece_minfo[pm_idx].type = PM_DATA_TYPE_ECE;
            ece_p->ece_minfo[pm_idx].measurement_interval_id = ece_p->ece_current_interval_id;   /* record this interval id */
            ece_p->ece_minfo[pm_idx].interval_end_time = time(NULL);                             /* record the end time in this interval */
            ece_p->ece_minfo[pm_idx].elapsed_time = time_p->ece_tick;                            /* record total seconds in this interval */

            /* do action: dm storage */
            if (conf_p->dm_storage_mode) {
                T_D("dm_storage_mode, tick = %d", time_p->dm_tick);
                (void) _perf_mon_ece_report_write();
            }
        }

        /* reset ece time tick */
        time_p->ece_tick = 0;
    }

    PERF_MON_CRIT_EXIT();

    T_N("exit");
}

/**
  * \brief Execute transfer action
  *
  * \return
  *   None.
  */
static void perf_mon_transfer_action(void)
{
    perf_mon_conf_t         *conf_p;
    i8                      *url_str;
    misc_url_parts_t        url_parts;
    int                     str_len;

    T_D("enter");

    url_str = NULL;

    PERF_MON_CRIT_ENTER();
    conf_p = &pm_global.perf_mon_conf;

    /* check path */
    if ((str_len = strlen(conf_p->transfer_server_url)) > 0) {
        if ((url_str = (i8 *)VTSS_MALLOC(PM_STR_LENGTH)) != NULL) {
            memset(url_str, 0x0, PM_STR_LENGTH);
            strncpy(url_str, conf_p->transfer_server_url, str_len);
        }
    }
    PERF_MON_CRIT_EXIT();

    if (!url_str) {
        return;
    }

    T_D("Sending to %s", url_str);

    /* transfer lm data */
    if (vtss_perf_mon_decompose_url("destination", url_str, str_len, &url_parts)) {
        (void) vtss_perf_mon_transfer_lm_data_set(&url_parts);
    }

    /* transfer dm data */
    if (vtss_perf_mon_decompose_url("destination", url_str, str_len, &url_parts)) {
        (void) vtss_perf_mon_transfer_dm_data_set(&url_parts);
    }

    /* transfer evc data */
    if (vtss_perf_mon_decompose_url("destination", url_str, str_len, &url_parts)) {
        (void) vtss_perf_mon_transfer_evc_data_set(&url_parts);
    }

    /* transfer ece data */
    if (vtss_perf_mon_decompose_url("destination", url_str, str_len, &url_parts)) {
        (void) vtss_perf_mon_transfer_ece_data_set(&url_parts);
    }

    VTSS_FREE(url_str);

    T_D("exit");
}


/**
  * \brief Execute transfer check
  *
  * \return
  *   None.
  */
static void perf_mon_transfer_check(void)
{
    perf_mon_conf_t         *conf_p;
    u32                     idx;
    u16                     total_seconds;
    static u16              local_random_offset = 0;
    static u16              local_cnt = 0;
    BOOL                    action_flag = FALSE;

    T_N("enter");

    PERF_MON_CRIT_ENTER();

    conf_p = &pm_global.perf_mon_conf;

    // check global flag
    if (conf_p->transfer_mode == FALSE) {
        goto OUT;
    }

    if (conf_p->transfer_scheduled_hours == 0 && conf_p->transfer_scheduled_minutes == 0) {
        // the administrator doesn't set the scheduled time, do nothing.
        goto OUT;
    }

    // check fixed offset and random offset
    if (local_random_offset) {
        local_cnt++;
        if (local_cnt == local_random_offset) {
            local_random_offset = 0;
            local_cnt = 0;
            // do action
            action_flag = TRUE;
        } else {
            // speed up checking
            goto OUT;
        }
    }

    // we only run check per 30 seconds.
    // The system boot has to spend some times, so we delay the check time.
    if (perf_mon_current_seconds_idx_get() != 30) {
        goto OUT;
    }

    // check hours
    if (conf_p->transfer_scheduled_hours > 0) {
        // check hours is correct or not?
        idx = perf_mon_current_hour_idx_get();
        if (!VTSS_PM_BF_GET(conf_p->transfer_scheduled_hours, idx)) {
            goto OUT;
        }
    }

    // check minutes
    if (conf_p->transfer_scheduled_minutes > 0) {
        // check minutes is correct or not?
        idx = perf_mon_current_minutes_idx_get();
        if (idx > 3) {
            goto OUT; // not 0, 15, 30, 45
        }
        if (!VTSS_PM_BF_GET(conf_p->transfer_scheduled_minutes, idx)) {
            goto OUT;
        }
    }

    // the administrator doesn't set offset
    if (conf_p->transfer_scheduled_offset == 0 && conf_p->transfer_scheduled_random_offset == 0) {
        // do action
        action_flag = TRUE;
    } else {
        total_seconds = ((conf_p->transfer_scheduled_offset) * TRANSFER_MINUTE_TO_SECOND) + perf_mon_cal_random_offset(conf_p->transfer_scheduled_random_offset);

        // set fixed offset and random offset
        if (total_seconds > PM_MAX_RANDOM_OFFSET) {
            local_random_offset = PM_MAX_RANDOM_OFFSET;
        } else {
            local_random_offset = total_seconds;
        }
    }

OUT:
    PERF_MON_CRIT_EXIT();

#if 0
    if (perf_mon_current_seconds_idx_get() == 30) {
        action_flag = TRUE;
    } //James for test
#endif

    if (action_flag) {
        T_D("Start perf_mon_transfer_action");
        perf_mon_transfer_action();
        T_D("Done");
    }

    T_N("exit");
}

/****************************************************************************/
/*  global functions                                                        */
/****************************************************************************/

/**
  * \brief Get performance monitor configuration
  *
  * \return
  *   None.
  */
void perf_mon_conf_get(perf_mon_conf_t *conf)
{
    PERF_MON_CRIT_ENTER();
    memcpy(conf, &pm_global.perf_mon_conf, sizeof(perf_mon_conf_t));
    PERF_MON_CRIT_EXIT();

    return;
}

/**
  * \brief Set performance monitor configuration
  *
  * \return
  *   None.
  */
void perf_mon_conf_set(perf_mon_conf_t *conf)
{
    PERF_MON_CRIT_ENTER();
    memcpy(&pm_global.perf_mon_conf, conf, sizeof(perf_mon_conf_t));
    PERF_MON_CRIT_EXIT();

    /* reset time tick on pm module */
    perf_mon_time_tick_reset();

    return;
}

/**
  * \brief Get LM Basic Configuration
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_conf_get(vtss_perf_mon_conf_t *data)
{
    PERF_MON_CRIT_ENTER();
    data->current_interval_cnt = pm_global.lm.lm_current_interval_cnt;
    data->current_interval_id = pm_global.lm.lm_current_interval_id;
    PERF_MON_CRIT_EXIT();

    return TRUE;
}

/**
  * \brief Get LM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_data_get(vtss_perf_mon_lm_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    /* check normal range */
    if (!data || data->measurement_interval_id < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check lm interval range */
    if ((pm_global.lm.lm_current_interval_id - (data->measurement_interval_id)) >= pm_global.lm.lm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, data->measurement_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.lm.lm_minfo[pm_idx], pm_global.lm.lm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.lm.lm_data_set[pm_idx], PM_LM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_lm_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get First LM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_data_get_first(vtss_perf_mon_lm_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.lm.lm_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, pm_global.lm.lm_current_interval_id - pm_global.lm.lm_current_interval_cnt + 1);
    PM_CHK_DATA_SET_VALID(pm_global.lm.lm_minfo[pm_idx], pm_global.lm.lm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.lm.lm_data_set[pm_idx], PM_LM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_lm_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Next LM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_data_get_next(vtss_perf_mon_lm_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) + 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check lm interval range */
    if (next > pm_global.lm.lm_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check lm interval range */
    if ((pm_global.lm.lm_current_interval_id - next) >= pm_global.lm.lm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, next);
    PM_CHK_DATA_SET_VALID(pm_global.lm.lm_minfo[pm_idx], pm_global.lm.lm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.lm.lm_data_set[pm_idx], PM_LM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_lm_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Previous LM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_data_get_previous(vtss_perf_mon_lm_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) - 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check lm interval range */
    if (next > pm_global.lm.lm_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check lm interval range */
    if ((pm_global.lm.lm_current_interval_id - next) >= pm_global.lm.lm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, next);
    PM_CHK_DATA_SET_VALID(pm_global.lm.lm_minfo[pm_idx], pm_global.lm.lm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.lm.lm_data_set[pm_idx], PM_LM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_lm_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Last LM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_data_get_last(vtss_perf_mon_lm_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.lm.lm_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, pm_global.lm.lm_current_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.lm.lm_minfo[pm_idx], pm_global.lm.lm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.lm.lm_data_set[pm_idx], PM_LM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_lm_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Clear LM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_data_clear(void)
{
    PERF_MON_CRIT_ENTER();
    memset(&pm_global.lm, 0, sizeof(perf_mon_lm_conf_t));
    PERF_MON_CRIT_EXIT();

    return TRUE;
}

/**
  * \brief Get DM Basic Configuration
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_conf_get(vtss_perf_mon_conf_t *data)
{
    PERF_MON_CRIT_ENTER();
    data->current_interval_cnt = pm_global.dm.dm_current_interval_cnt;
    data->current_interval_id = pm_global.dm.dm_current_interval_id;
    PERF_MON_CRIT_EXIT();

    return TRUE;
}

/**
  * \brief Get DM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_data_get(vtss_perf_mon_dm_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    /* check normal range */
    if (!data || data->measurement_interval_id < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check dm interval range */
    if ((pm_global.dm.dm_current_interval_id - (data->measurement_interval_id)) >= pm_global.dm.dm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, data->measurement_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.dm.dm_minfo[pm_idx], pm_global.dm.dm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.dm.dm_data_set[pm_idx], PM_DM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_dm_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get First DM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_data_get_first(vtss_perf_mon_dm_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.dm.dm_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }
    T_D("pm_global.dm.dm_current_interval_id:%d", pm_global.dm.dm_current_interval_id);
    T_D("pm_global.dm.dm_current_interval_cnt:%d", pm_global.dm.dm_current_interval_cnt);

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, pm_global.dm.dm_current_interval_id - pm_global.dm.dm_current_interval_cnt + 1);
    T_D("pm_idx:%d ", pm_idx);


    PM_CHK_DATA_SET_VALID(pm_global.dm.dm_minfo[pm_idx], pm_global.dm.dm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.dm.dm_data_set[pm_idx], PM_DM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_dm_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Next DM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_data_get_next(vtss_perf_mon_dm_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) + 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check dm interval range */
    if (next > pm_global.dm.dm_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check dm interval range */
    if ((pm_global.dm.dm_current_interval_id - next) >= pm_global.dm.dm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, next);
    PM_CHK_DATA_SET_VALID(pm_global.dm.dm_minfo[pm_idx], pm_global.dm.dm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.dm.dm_data_set[pm_idx], PM_DM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_dm_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Previous DM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_data_get_previous(vtss_perf_mon_dm_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) - 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check dm interval range */
    if (next > pm_global.dm.dm_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check dm interval range */
    if ((pm_global.dm.dm_current_interval_id - next) >= pm_global.dm.dm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, next);
    PM_CHK_DATA_SET_VALID(pm_global.dm.dm_minfo[pm_idx], pm_global.dm.dm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.dm.dm_data_set[pm_idx], PM_DM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_dm_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Last DM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_data_get_last(vtss_perf_mon_dm_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.dm.dm_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, pm_global.dm.dm_current_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.dm.dm_minfo[pm_idx], pm_global.dm.dm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.dm.dm_data_set[pm_idx], PM_DM_DATA_SET_LIMIT * sizeof(vtss_perf_mon_dm_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Clear DM data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_data_clear(void)
{
    PERF_MON_CRIT_ENTER();
    memset(&pm_global.dm, 0, sizeof(perf_mon_dm_conf_t));
    PERF_MON_CRIT_EXIT();

    return TRUE;
}

/**
  * \brief Get EVC Basic Configuration
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_conf_get(vtss_perf_mon_conf_t *data)
{
    PERF_MON_CRIT_ENTER();
    data->current_interval_cnt = pm_global.evc.evc_current_interval_cnt;
    data->current_interval_id = pm_global.evc.evc_current_interval_id;
    PERF_MON_CRIT_EXIT();

    return TRUE;
}

/**
  * \brief Get EVC data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_data_get(vtss_perf_mon_evc_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    /* check normal range */
    if (!data || data->measurement_interval_id < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check evc interval range */
    if ((pm_global.evc.evc_current_interval_id - (data->measurement_interval_id)) >= pm_global.evc.evc_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, data->measurement_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.evc.evc_minfo[pm_idx], pm_global.evc.evc_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.evc.evc_data_set[pm_idx], PM_EVC_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get First EVC data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_data_get_first(vtss_perf_mon_evc_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.evc.evc_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, pm_global.evc.evc_current_interval_id - pm_global.evc.evc_current_interval_cnt + 1);
    PM_CHK_DATA_SET_VALID(pm_global.evc.evc_minfo[pm_idx], pm_global.evc.evc_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.evc.evc_data_set[pm_idx], PM_EVC_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Next EVC data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_data_get_next(vtss_perf_mon_evc_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) + 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check evc interval range */
    if (next > pm_global.evc.evc_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check evc interval range */
    if ((pm_global.evc.evc_current_interval_id - next) >= pm_global.evc.evc_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, next);
    PM_CHK_DATA_SET_VALID(pm_global.evc.evc_minfo[pm_idx], pm_global.evc.evc_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.evc.evc_data_set[pm_idx], PM_EVC_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Previous EVC data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_data_get_previous(vtss_perf_mon_evc_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) - 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check evc interval range */
    if (next > pm_global.evc.evc_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check evc interval range */
    if ((pm_global.evc.evc_current_interval_id - next) >= pm_global.evc.evc_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, next);
    PM_CHK_DATA_SET_VALID(pm_global.evc.evc_minfo[pm_idx], pm_global.evc.evc_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.evc.evc_data_set[pm_idx], PM_EVC_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Last EVC data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_data_get_last(vtss_perf_mon_evc_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.evc.evc_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, pm_global.evc.evc_current_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.evc.evc_minfo[pm_idx], pm_global.evc.evc_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.evc.evc_data_set[pm_idx], PM_EVC_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Clear EVC data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_data_clear(void)
{
    PERF_MON_CRIT_ENTER();
    memset(&pm_global.evc, 0, sizeof(perf_mon_evc_conf_t));
    PERF_MON_CRIT_EXIT();

    return TRUE;
}

/**
  * \brief Get ECE Basic Configuration
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_conf_get(vtss_perf_mon_conf_t *data)
{
    PERF_MON_CRIT_ENTER();
    data->current_interval_cnt = pm_global.ece.ece_current_interval_cnt;
    data->current_interval_id = pm_global.ece.ece_current_interval_id;
    PERF_MON_CRIT_EXIT();

    return TRUE;
}

/**
  * \brief Get ECE data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_data_get(vtss_perf_mon_evc_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    /* check normal range */
    if (!data || data->measurement_interval_id < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check ece interval range */
    if ((pm_global.ece.ece_current_interval_id - (data->measurement_interval_id)) >= pm_global.ece.ece_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, data->measurement_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.ece.ece_minfo[pm_idx], pm_global.ece.ece_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.ece.ece_data_set[pm_idx], PM_ECE_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get First ECE data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_data_get_first(vtss_perf_mon_evc_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.ece.ece_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, pm_global.ece.ece_current_interval_id - pm_global.ece.ece_current_interval_cnt + 1);
    PM_CHK_DATA_SET_VALID(pm_global.ece.ece_minfo[pm_idx], pm_global.ece.ece_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.ece.ece_data_set[pm_idx], PM_ECE_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Next ECE data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_data_get_next(vtss_perf_mon_evc_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) + 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check ece interval range */
    if (next > pm_global.ece.ece_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check ece interval range */
    if ((pm_global.ece.ece_current_interval_id - next) >= pm_global.ece.ece_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, next);
    PM_CHK_DATA_SET_VALID(pm_global.ece.ece_minfo[pm_idx], pm_global.ece.ece_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.ece.ece_data_set[pm_idx], PM_ECE_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Previous ECE data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_data_get_previous(vtss_perf_mon_evc_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) - 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check ece interval range */
    if (next > pm_global.ece.ece_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check ece interval range */
    if ((pm_global.ece.ece_current_interval_id - next) >= pm_global.ece.ece_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, next);
    PM_CHK_DATA_SET_VALID(pm_global.ece.ece_minfo[pm_idx], pm_global.ece.ece_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.ece.ece_data_set[pm_idx], PM_ECE_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Last ECE data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_data_get_last(vtss_perf_mon_evc_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.ece.ece_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, pm_global.ece.ece_current_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.ece.ece_minfo[pm_idx], pm_global.ece.ece_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, pm_global.ece.ece_data_set[pm_idx], PM_ECE_DATA_SET_LIMIT * sizeof(vtss_perf_mon_evc_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Clear ECE data set
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_data_clear(void)
{
    PERF_MON_CRIT_ENTER();
    memset(&pm_global.ece, 0, sizeof(perf_mon_ece_conf_t));
    PERF_MON_CRIT_EXIT();

    return TRUE;
}

/**
  * \brief Get LM measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_interval_get(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    /* check normal range */
    if (!data || data->measurement_interval_id < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check lm interval range */
    if ((pm_global.lm.lm_current_interval_id - (data->measurement_interval_id)) >= pm_global.lm.lm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, data->measurement_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.lm.lm_minfo[pm_idx], pm_global.lm.lm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.lm.lm_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get First LM measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_interval_get_first(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.lm.lm_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, pm_global.lm.lm_current_interval_id - pm_global.lm.lm_current_interval_cnt + 1);
    PM_CHK_DATA_SET_VALID(pm_global.lm.lm_minfo[pm_idx], pm_global.lm.lm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.lm.lm_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Next LM measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_interval_get_next(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) + 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        T_D("next < PM_FIRST_INTERVAL_ID");
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check lm interval range */
    if (next > pm_global.lm.lm_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        T_D("next > pm_global.lm.lm_current_interval_id");
        return FALSE;
    }

    /* check lm interval range */
    if ((pm_global.lm.lm_current_interval_id - next) >= pm_global.lm.lm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        T_D("pm_global.lm.lm_current_interval_id - next");
        return FALSE;
    }

    do {
        pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, next);
        PM_CHK_DATA_SET_VALID(pm_global.lm.lm_minfo[pm_idx], pm_global.lm.lm_data_set[pm_idx], rv);
        if (rv) {
            memcpy(data, &pm_global.lm.lm_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
            break;
        } else {
            next++;
            T_D("if (rv)");
        }
    } while (next <= pm_global.lm.lm_current_interval_id);
    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Previous LM measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_interval_get_previous(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) - 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check lm interval range */
    if (next > pm_global.lm.lm_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check lm interval range */
    if ((pm_global.lm.lm_current_interval_id - next) >= pm_global.lm.lm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, next);
    PM_CHK_DATA_SET_VALID(pm_global.lm.lm_minfo[pm_idx], pm_global.lm.lm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.lm.lm_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Last LM measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_lm_interval_get_last(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.lm.lm_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_LM, pm_global.lm.lm_current_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.lm.lm_minfo[pm_idx], pm_global.lm.lm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.lm.lm_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get DM measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_interval_get(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    /* check normal range */
    if (!data || data->measurement_interval_id < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check dm interval range */
    if ((pm_global.dm.dm_current_interval_id - (data->measurement_interval_id)) >= pm_global.dm.dm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, data->measurement_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.dm.dm_minfo[pm_idx], pm_global.dm.dm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.dm.dm_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get First DM measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_interval_get_first(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.dm.dm_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }
    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, pm_global.dm.dm_current_interval_id - pm_global.dm.dm_current_interval_cnt + 1);
    PM_CHK_DATA_SET_VALID(pm_global.dm.dm_minfo[pm_idx], pm_global.dm.dm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.dm.dm_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }


    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Next DM measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_interval_get_next(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) + 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check dm interval range */
    if (next > pm_global.dm.dm_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check dm interval range */
    if ((pm_global.dm.dm_current_interval_id - next) >= pm_global.dm.dm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    do {
        pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, next);
        PM_CHK_DATA_SET_VALID(pm_global.dm.dm_minfo[pm_idx], pm_global.dm.dm_data_set[pm_idx], rv);
        if (rv) {
            memcpy(data, &pm_global.dm.dm_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
            break;
        } else {
            next++;
            T_D("if (rv)");
        }
    } while (next <= pm_global.dm.dm_current_interval_id);

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Previous DM measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_interval_get_previous(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) - 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check dm interval range */
    if (next > pm_global.dm.dm_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check dm interval range */
    if ((pm_global.dm.dm_current_interval_id - next) >= pm_global.dm.dm_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, next);
    PM_CHK_DATA_SET_VALID(pm_global.dm.dm_minfo[pm_idx], pm_global.dm.dm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.dm.dm_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Last DM measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_dm_interval_get_last(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.dm.dm_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_DM, pm_global.dm.dm_current_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.dm.dm_minfo[pm_idx], pm_global.dm.dm_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.dm.dm_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get EVC measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_interval_get(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    /* check normal range */
    if (!data || data->measurement_interval_id < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check evc interval range */
    if ((pm_global.evc.evc_current_interval_id - (data->measurement_interval_id)) >= pm_global.evc.evc_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, data->measurement_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.evc.evc_minfo[pm_idx], pm_global.evc.evc_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.evc.evc_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get First EVC measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_interval_get_first(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.evc.evc_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, pm_global.evc.evc_current_interval_id - pm_global.evc.evc_current_interval_cnt + 1);
    PM_CHK_DATA_SET_VALID(pm_global.evc.evc_minfo[pm_idx], pm_global.evc.evc_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.evc.evc_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Next EVC measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_interval_get_next(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) + 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check evc interval range */
    if (next > pm_global.evc.evc_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check evc interval range */
    if ((pm_global.evc.evc_current_interval_id - next) >= pm_global.evc.evc_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    do {
        pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, next);
        PM_CHK_DATA_SET_VALID(pm_global.evc.evc_minfo[pm_idx], pm_global.evc.evc_data_set[pm_idx], rv);
        if (rv) {
            memcpy(data, &pm_global.evc.evc_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
            break;
        } else {
            next++;
            T_D("if (rv)");
        }
    } while (next <= pm_global.evc.evc_current_interval_id);

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Previous EVC measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_interval_get_previous(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) - 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check evc interval range */
    if (next > pm_global.evc.evc_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check evc interval range */
    if ((pm_global.evc.evc_current_interval_id - next) >= pm_global.evc.evc_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, next);
    PM_CHK_DATA_SET_VALID(pm_global.evc.evc_minfo[pm_idx], pm_global.evc.evc_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.evc.evc_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Last EVC measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_evc_interval_get_last(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.evc.evc_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_EVC, pm_global.evc.evc_current_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.evc.evc_minfo[pm_idx], pm_global.evc.evc_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.evc.evc_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get ECE measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_interval_get(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    /* check normal range */
    if (!data || data->measurement_interval_id < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check ece interval range */
    if ((pm_global.ece.ece_current_interval_id - (data->measurement_interval_id)) >= pm_global.ece.ece_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, data->measurement_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.ece.ece_minfo[pm_idx], pm_global.ece.ece_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.ece.ece_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get First ECE measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_interval_get_first(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.ece.ece_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, pm_global.ece.ece_current_interval_id - pm_global.ece.ece_current_interval_cnt + 1);
    PM_CHK_DATA_SET_VALID(pm_global.ece.ece_minfo[pm_idx], pm_global.ece.ece_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.ece.ece_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Next ECE measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_interval_get_next(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) + 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check ece interval range */
    if (next > pm_global.ece.ece_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check ece interval range */
    if ((pm_global.ece.ece_current_interval_id - next) >= pm_global.ece.ece_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    do {
        pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, next);
        PM_CHK_DATA_SET_VALID(pm_global.ece.ece_minfo[pm_idx], pm_global.ece.ece_data_set[pm_idx], rv);
        if (rv) {
            memcpy(data, &pm_global.ece.ece_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
            break;
        } else {
            next++;
            T_D("if (rv)");
        }
    } while (next <= pm_global.ece.ece_current_interval_id);

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Previous ECE measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_interval_get_previous(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    u32     next;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    next = (data->measurement_interval_id) - 1;

    /* check normal range */
    if (next < PM_FIRST_INTERVAL_ID) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    /* check ece interval range */
    if (next > pm_global.ece.ece_current_interval_id) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    /* check ece interval range */
    if ((pm_global.ece.ece_current_interval_id - next) >= pm_global.ece.ece_current_interval_cnt) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, next);
    PM_CHK_DATA_SET_VALID(pm_global.ece.ece_minfo[pm_idx], pm_global.ece.ece_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.ece.ece_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief Get Last ECE measurement interval information
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_ece_interval_get_last(vtss_perf_mon_measurement_info_t *data)
{
    u32     pm_idx;
    BOOL    rv;

    if (!data) {
        return FALSE;
    }

    PERF_MON_CRIT_ENTER();

    if (pm_global.ece.ece_current_interval_id < PM_FIRST_INTERVAL_ID) {
        PERF_MON_CRIT_EXIT();
        return FALSE;
    }

    pm_idx = perf_mon_cal_interval_slot(PM_DATA_TYPE_ECE, pm_global.ece.ece_current_interval_id);
    PM_CHK_DATA_SET_VALID(pm_global.ece.ece_minfo[pm_idx], pm_global.ece.ece_data_set[pm_idx], rv);
    if (rv) {
        memcpy(data, &pm_global.ece.ece_minfo[pm_idx], sizeof(vtss_perf_mon_measurement_info_t));
    }

    PERF_MON_CRIT_EXIT();

    return rv;
}

/**
  * \brief PM Utility for adjusting current time w.r.t time zone
  *
  * \return
  *   None.
  */
void perf_mon_util_adjust_time_tz(void *now)
{
#ifdef VTSS_SW_OPTION_SYSUTIL
    if (now) {
        int tz_off = system_get_tz_off();

        /* Adjust for TZ minutes => seconds*/
        *((int *)now) += (tz_off * 60);
#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
        /* Correct for DST */
//LINT        *((int *)now) += (int)(time_dst_get_offset() * 60);
#endif /* VTSS_SW_OPTION_DAYLIGHT_SAVING */
    }
#endif /* VTSS_SW_OPTION_SYSUTIL */
}

/**
  * \brief Clear All Collection on performance monitor
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_clear_all_collection(void)
{
    BOOL    status = FALSE;

    /* delete all lm data set */
    status |= vtss_perf_mon_lm_data_clear();
    if (!status) {
        T_D("clear lm data set error!!");
    }

    /* delete all dm data set */
    status |= vtss_perf_mon_dm_data_clear();
    if (!status) {
        T_D("clear dm data set error!!");
    }

    /* delete all evc data set */
    status |= vtss_perf_mon_evc_data_clear();
    if (!status) {
        T_D("clear evc data set error!!");
    }

    /* delete all ece data set */
    status |= vtss_perf_mon_ece_data_clear();
    if (!status) {
        T_D("clear ece data set error!!");
    }

    return status;
}

/**
  * \brief Reset flash data to default
  *
  * \return
  *   TRUE: find the data set.
  *   FALSE: not in the interval range.
  */
BOOL vtss_perf_mon_reset_flash_data(void)
{
    BOOL    status = vtss_perf_mon_clear_all_collection();

    if (!status) {
        T_D("clear all collection error!!");
    }

    PERF_MON_CRIT_ENTER();

    if (_perf_mon_lm_report_write() != VTSS_OK) {
        status = FALSE;
        T_D("write lm report on FLASH error!!");
    }

    if (_perf_mon_dm_report_write() != VTSS_OK) {
        status = FALSE;
        T_D("write dm report on FLASH error!!");
    }

    if (_perf_mon_evc_report_write() != VTSS_OK) {
        status = FALSE;
        T_D("write evc report on FLASH error!!");
    }

    if (_perf_mon_ece_report_write() != VTSS_OK) {
        status = FALSE;
        T_D("write ece report on FLASH error!!");
    }

    PERF_MON_CRIT_EXIT();

    return status;
}

/****************************************************************************/
/*  PM system functions                                                     */
/****************************************************************************/

/**
  * \brief Gathering and Storage thread for performance monitor (PM)
  *
  * \return
  *   None.
  */
static void perf_mon_thread_handler(cyg_addrword_t data)
{
    T_D("enter");

    /* reset the gathering thread time tick */
    perf_mon_time_tick_reset();

    while ( TRUE ) {
        VTSS_OS_MSLEEP(1000);



        /* increase the time tick on all tick count */
        perf_mon_time_tick_increase();

        /* LM session and storage */
        perf_mon_lm_check();

        /* DM session and storage */
        perf_mon_dm_check();

        /* EVC session and storage */
        perf_mon_evc_check();

        /* ECE session and storage */
        perf_mon_ece_check();

        /* transfer */
        perf_mon_transfer_check();
    }

    T_D("exit");
}

/**
  * \brief Read/Create PERF_MON reports
  *
  * \return
  *   None.
  */
static void perf_mon_read_report(BOOL create)
{
    T_D("enter, create: %d", create);

    PERF_MON_CRIT_ENTER();

    /* read LM report from FLASH */
    (void) _perf_mon_lm_report_read(create);

    /* read DM report from FLASH */
    (void) _perf_mon_dm_report_read(create);

    /* read EVC report from FLASH */
    (void) _perf_mon_evc_report_read(create);

    /* read ECE report from FLASH */
    (void) _perf_mon_ece_report_read(create);

    /* reset report to default on the RAM */
    if (create) {
        /* reset all data set */
        memset(&pm_global.lm, 0, sizeof(perf_mon_lm_conf_t));
        memset(&pm_global.dm, 0, sizeof(perf_mon_dm_conf_t));
        memset(&pm_global.evc, 0, sizeof(perf_mon_evc_conf_t));
        memset(&pm_global.ece, 0, sizeof(perf_mon_ece_conf_t));
    }

    PERF_MON_CRIT_EXIT();

    T_D("exit");
    return;
}

/**
  * \brief Module start
  *
  * \return
  *   None.
  */
static void perf_mon_start(BOOL init)
{
    perf_mon_conf_t *conf_p;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize Performance Monitor configuration */
        conf_p = &pm_global.perf_mon_conf;
        perf_mon_default_set(conf_p);

        /* Create semaphore for critical regions */
        critd_init(&pm_global.crit, "pm_global.crit", VTSS_MODULE_ID_PERF_MON, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        PERF_MON_CRIT_EXIT();

        /* create PM thread for gathering, storage and transfer process */
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          perf_mon_thread_handler,
                          0,
                          "Performance.Monitor",
                          pm_thread_stack,
                          sizeof(pm_thread_stack),
                          &pm_thread_handle,
                          &pm_thread_block);
        cyg_thread_resume(pm_thread_handle);
    }

    T_D("exit");
    return;
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/**
  * \brief Initialize module
  *
  * \return
  *   (vtss_rc).
  */
vtss_rc perf_mon_init(vtss_init_data_t *data)
{
    vtss_rc     rc = VTSS_OK;
    vtss_isid_t isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        perf_mon_start(TRUE);
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = vtss_perf_mon_icfg_init()) != VTSS_OK) {
            T_D("Calling vtss_perf_mon_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
        perf_mon_start(FALSE);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);

        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset global configuration */
            if (!vtss_perf_mon_reset_flash_data()) {
                perf_mon_read_report(TRUE);
            }
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }

        /* reset the configuration to default */
        perf_mon_default_conf_action();
        break;
    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");

        /* Read All reports from FLASH */
        perf_mon_read_report(FALSE);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        break;
    default:
        break;
    }

    T_D("exit");
    return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
