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
#include "conf_api.h"
#include "msg_api.h"

#include "vtss_ntp_api.h"
#include "vtss_ntp.h"
#include "vtss_os.h"
#include "misc_api.h"
#include "port_api.h"
#include "ip2_api.h"

#include <network.h>
#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT) && defined(CYGBLD_HAL_TIMEADJ_H)
#include CYGBLD_HAL_TIMEADJ_H
#endif
#include <arpa/inet.h>

#define NTP_USING_NTP_PACKAGE   1

#if NTP_USING_NTP_PACKAGE
#include <pthread.h>
#include "ntp_ecos.h"
#include "ntp_callout.h"
#endif /* NTP_USING_NTP_PACKAGE */

#ifdef VTSS_SW_OPTION_ICFG
#include "vtss_ntp_icfg.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_NTP

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static ntp_global_t         ntp_global;
static ntp_conf_t           old_ntp_conf;
static int                  ntp_initial = 0;
static ntp_server_config_t  config_server[NTP_MAX_SERVER_COUNT];

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "ntp",
    .descr     = "ntp Module"
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
        .descr     = "Critical regions",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};
#define NTP_CRIT_ENTER() critd_enter(&ntp_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define NTP_CRIT_EXIT()  critd_exit( &ntp_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define NTP_CRIT_ENTER() critd_enter(&ntp_global.crit)
#define NTP_CRIT_EXIT()  critd_exit( &ntp_global.crit)
#endif /* VTSS_TRACE_ENABLED */

/* Pthread variables */
static char ntp_pthread_stack[THREAD_DEFAULT_STACK_SIZE * 4];
/* Thread variables */
#define NTP_CERT_THREAD_STACK_SIZE       8192
static cyg_handle_t ntp_thread_handle;
static cyg_thread   ntp_thread_block;
static char         ntp_thread_stack[NTP_CERT_THREAD_STACK_SIZE];

static ntp_server_status_t server_info[NTP_MAX_SERVER_COUNT];
static uchar      ip_addr_string[NTP_MAX_SERVER_COUNT][VTSS_SYS_HOSTNAME_LEN];

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* Determine if ntp configuration has changed */
static int ntp_conf_changed(ntp_conf_t *old, ntp_conf_t *new)
{
    return (memcmp(new, old, sizeof(ntp_conf_t)));
}

/* Set ntp defaults */
static void ntp_default_set(ntp_conf_t *conf)
{
    conf->mode_enabled = NTP_MGMT_DISABLED;
    conf->interval_min = 6;
    conf->interval_max = 8;
    conf->drift_valid  = 1;
    conf->drift_data = 0; /* The value was 22,experiment result, before BugZilla# 1022 fixed */
    conf->drift_trained  = 1;
    memset(&conf->server[0], 0, sizeof(ntp_server_config_t)*NTP_MAX_SERVER_COUNT);
}


/****************************************************************************/
/*  Module core functions                                                    */
/****************************************************************************/

#if NTP_USING_NTP_PACKAGE
void
vtss_ntp_server_info_del(char *ip_str)
{
    int i;

    for (i = 0; i < 5; i++) {
        if (memcmp(server_info[i].ip_string, ip_str,
                   sizeof(server_info[i].ip_string)) == 0) {

            memset(&server_info[i], 0,
                   sizeof(ntp_server_status_t));
            break;
        }
    }
}

static void ntp_del_ntp_old_config(unsigned char entry_idx)
{
    //int rc;
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t zero_ipv6addr;
    char        ipv6_server[40] , *ptr = NULL;

    memset(&zero_ipv6addr, 0, sizeof(zero_ipv6addr));
    memset(ipv6_server, 0, sizeof(ipv6_server));
#endif /* VTSS_SW_OPTION_IPV6 */

#ifdef VTSS_SW_OPTION_IPV6
    if ((strcmp((char *)&ip_addr_string[entry_idx][0], "") != 0) || (memcmp(&old_ntp_conf.server[entry_idx].ipv6_addr, &zero_ipv6addr, sizeof(zero_ipv6addr)) != 0)) {
#else
    if (strcmp((char *)&ip_addr_string[entry_idx][0], "") != 0) {
#endif
        vtss_ntp_server_info_del((char *)&ip_addr_string[entry_idx][0]);
        if (strcmp((char *)&ip_addr_string[entry_idx][0], "") != 0) {
            (void)ntp_unset_ntp_server(old_ntp_conf.server[entry_idx].ip_type, (char *)&ip_addr_string[entry_idx][0]);
        }
#ifdef VTSS_SW_OPTION_IPV6
        else {
            ptr = misc_ipv6_txt(&old_ntp_conf.server[entry_idx].ipv6_addr, ipv6_server);
            if (ptr != NULL) {
                (void)ntp_unset_ntp_server(old_ntp_conf.server[entry_idx].ip_type, ipv6_server);
            }
        }
#endif
    }
}


static vtss_rc ntp_set_ntp_config(ntp_conf_t *conf)
{
    unsigned char      i;
    int                rc = -1;
    struct sockaddr_in host;
    struct hostent     *hp;
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t        zero_ipv6addr;
    char               ipv6_server[40], *ptr = NULL;

    memset(&zero_ipv6addr, 0, sizeof(zero_ipv6addr));
    memset(ipv6_server, 0, sizeof(ipv6_server));
#endif /* VTSS_SW_OPTION_IPV6 */

    if (!ntp_initial)
        /* do nothing if not initialed yet. ntp thread will set proper configuration
           to the ntp module after initialed */
    {
        return VTSS_RC_OK;
    }

    if (conf->mode_enabled != old_ntp_conf.mode_enabled) {
        /*
         * Change the mode from disabled to eanbled
         * Add all servers configured to the NTP module
         */
        if (conf->mode_enabled == TRUE) {
            rc = ntp_set_ntp_interval((int)conf->interval_min,
                                      (int)conf->interval_max);
            if (rc != 0) {
                T_W("ntp_set_ntp_interval error\n");
            }

            // Set up host address
            host.sin_family = AF_INET;
            host.sin_len = sizeof(host);
            for (i = 0; i < NTP_MAX_SERVER_COUNT; i++) {
                if (conf->server[i].ip_type == NTP_IP_TYPE_IPV4) {
                    if (strcmp((char *)conf->server[i].ip_host_string, "") != 0) {
                        if (inet_aton((char *)conf->server[i].ip_host_string, &host.sin_addr)) {
                            strcpy((char *)&ip_addr_string[i][0], (char *)conf->server[i].ip_host_string);
                            rc = ntp_set_ntp_server((int)conf->server[i].ip_type, (char *)conf->server[i].ip_host_string);
                            if (rc != 0) {
                                T_D("ntp_set_ntp_server error - 1\n");
                            }
                        } else {
                            hp = gethostbyname((char *)conf->server[i].ip_host_string);
                            if (hp != NULL) {
                                struct in_addr ip;
                                bcopy(hp->h_addr_list[0], (char *)&ip, sizeof(ip));
                                strcpy((char *)&ip_addr_string[i][0], inet_ntoa(ip));
                                rc = ntp_set_ntp_server((int)conf->server[i].ip_type, (char *)&ip_addr_string[i][0]);
                                if (rc != 0) {
                                    T_D("ntp_set_ntp_server error - 2\n");
                                }
                            }
                        }
                    }
                }
#ifdef VTSS_SW_OPTION_IPV6
                else if (conf->server[i].ip_type == NTP_IP_TYPE_IPV6) {
                    if (memcmp(&conf->server[i].ipv6_addr, &zero_ipv6addr, sizeof(zero_ipv6addr)) != 0) {
                        ptr = misc_ipv6_txt(&conf->server[i].ipv6_addr, ipv6_server);
                        rc = ntp_set_ntp_server((int)conf->server[i].ip_type, ipv6_server);
                        if (rc != 0) {
                            T_D("ntp_set_ntp_server error - 3\n");
                        }
                    }
                }
#endif /* VTSS_SW_OPTION_IPV6 */
                else {
                    return VTSS_RC_ERROR;
                }
            }
        } else {
            /*
             * Change the mode from eanbled to disabled
             * Remove all servers configured to the NTP module
             */
            for (i = 0; i < NTP_MAX_SERVER_COUNT; i++) {
                ntp_del_ntp_old_config(i);
            }
        }
    } else {
        if (conf->interval_min != old_ntp_conf.interval_min ||
            conf->interval_max != old_ntp_conf.interval_max) {
            rc = ntp_set_ntp_interval((int)conf->interval_min,
                                      (int)conf->interval_max);
            if (rc != 0) {
                T_W("ntp_set_ntp_interval error\n");
            }
        }

        if (conf->mode_enabled == FALSE) {
            /* When enalbed, the servers configured are added to the NTP Module. When
               disabled, the server are removed. And therefore we can return here because
               no server needs to be added.
             */
            return VTSS_RC_OK;
        }
        for (i = 0; i < NTP_MAX_SERVER_COUNT; i++) {
            if (memcmp(&conf->server[i], &config_server[i], sizeof(conf->server[i])) == 0) {
                continue;
            }
            if (conf->server[i].ip_type == NTP_IP_TYPE_IPV4) {
                if (strcmp((char *)conf->server[i].ip_host_string, "") != 0) {
                    ntp_del_ntp_old_config(i);
                    if (inet_aton((char *)conf->server[i].ip_host_string, &host.sin_addr)) {
                        strcpy((char *)&ip_addr_string[i][0], (char *)conf->server[i].ip_host_string);
                        rc = ntp_set_ntp_server((int)conf->server[i].ip_type, (char *)&ip_addr_string[i][0]);

                        if (rc != 0) {
                            T_D("ntp_set_ntp_server error - a\n");
                        }
                    } else {
                        hp = gethostbyname((char *)conf->server[i].ip_host_string);
                        if (hp != NULL) {
                            struct in_addr ip;
                            bcopy(hp->h_addr_list[0], (char *)&ip, sizeof(ip));
                            strcpy((char *)&ip_addr_string[i][0], inet_ntoa(ip));
                            rc = ntp_set_ntp_server((int)conf->server[i].ip_type, (char *)&ip_addr_string[i][0]);
                            if (rc != 0) {
                                T_D("ntp_set_ntp_server error - b\n");
                            }
                        }
                    }
                } else {
                    ntp_del_ntp_old_config(i);
                }
            }
#ifdef VTSS_SW_OPTION_IPV6
            else if (conf->server[i].ip_type == NTP_IP_TYPE_IPV6) {
                ntp_del_ntp_old_config(i);
                if (memcmp(&conf->server[i].ipv6_addr, &zero_ipv6addr, sizeof(zero_ipv6addr)) != 0) {
                    ptr = misc_ipv6_txt(&conf->server[i].ipv6_addr, ipv6_server);
                    rc = ntp_set_ntp_server(conf->server[i].ip_type, ipv6_server);
                    if (rc != 0) {
                        T_D("ntp_set_ntp_server error -f \n");
                    }
                } else {
                    ntp_del_ntp_old_config(i);
                }
            }
#endif /* VTSS_SW_OPTION_IPV6 */
            else {
                return VTSS_RC_ERROR;
            }
        }
        memcpy(config_server, conf->server, sizeof (config_server));

    }

#ifdef VTSS_SW_OPTION_IPV6
    if (rc != 0 || ptr == NULL)
#else
    if (rc != 0)
#endif
        return VTSS_RC_ERROR;
    else {
        return VTSS_RC_OK;
    }
}
#endif /* NTP_USING_NTP_PACKAGE */


/****************************************************************************/
/*  Stack/switch functions                                                  */
/****************************************************************************/

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static char *ntp_msg_id_txt(ntp_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case NTP_MSG_ID_NTP_CONF_SET_REQ:
        txt = "NTP_MSG_ID_NTP_CONF_SET_REQ";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_TRACE_LVL_DEBUG */

/* Allocate request buffer */
static ntp_msg_req_t *ntp_msg_req_alloc(ntp_msg_buf_t *buf, ntp_msg_id_t msg_id)
{
    ntp_msg_req_t *msg = &ntp_global.request.msg;

    buf->sem = &ntp_global.request.sem;
    buf->msg = msg;
    (void)VTSS_OS_SEM_WAIT(buf->sem);
    msg->msg_id = msg_id;
    return msg;
}

/* Free request/reply buffer */
static void ntp_msg_free(vtss_os_sem_t *sem)
{
    VTSS_OS_SEM_POST(sem);
}

static void ntp_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    ntp_msg_id_t msg_id = *(ntp_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s", msg_id, ntp_msg_id_txt(msg_id));
    ntp_msg_free(contxt);
}

static void ntp_msg_tx(ntp_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    ntp_msg_id_t msg_id = *(ntp_msg_id_t *)buf->msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s, len: %zu, isid: %d", msg_id, ntp_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(buf->sem, ntp_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_MODULE_ID_NTP, isid, buf->msg, len + MSG_TX_DATA_HDR_LEN(ntp_msg_req_t, req));
}

static BOOL ntp_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    ntp_msg_id_t msg_id = *(ntp_msg_id_t *)rx_msg;

    T_D("msg_id: %d, %s, len: %zu, isid: %u", msg_id, ntp_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
    case NTP_MSG_ID_NTP_CONF_SET_REQ: {
#if NTP_USING_NTP_PACKAGE
        NTP_CRIT_ENTER();
        ntp_msg_req_t  *msg;

        msg = (ntp_msg_req_t *)rx_msg;
        (void)ntp_set_ntp_config(&msg->req.conf_set.conf);
        NTP_CRIT_EXIT();
#endif /* NTP_USING_NTP_PACKAGE */
        break;
    }
    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}

static vtss_rc ntp_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = ntp_msg_rx;
    filter.modid = VTSS_MODULE_ID_NTP;
    return msg_rx_filter_register(&filter);
}

/* Set stack ntp configuration */
static void ntp_stack_ntp_conf_set(vtss_isid_t isid_add)
{
    ntp_msg_req_t  *msg;
    ntp_msg_buf_t  buf;
    vtss_isid_t    isid;

    T_D("enter, isid_add: %d", isid_add);
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if ((isid_add != VTSS_ISID_GLOBAL && isid_add != isid) ||
            !msg_switch_exists(isid)) {
            continue;
        }
        NTP_CRIT_ENTER();
        msg = ntp_msg_req_alloc(&buf, NTP_MSG_ID_NTP_CONF_SET_REQ);
        msg->req.conf_set.conf = ntp_global.ntp_conf;
        NTP_CRIT_EXIT();
        ntp_msg_tx(&buf, isid, sizeof(msg->req.conf_set.conf));
    }

    T_D("exit, isid_add: %d", isid_add);
}


/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* ntp error text */
char *ntp_error_txt(ntp_error_t rc)
{
    char *txt;

    switch (rc) {
    case NTP_ERROR_GEN:
        txt = "ntp generic error";
        break;
    case NTP_ERROR_PARM:
        txt = "ntp parameter error";
        break;
    case NTP_ERROR_STACK_STATE:
        txt = "ntp stack state error";
        break;
    default:
        txt = "ntp unknown error";
        break;
    }
    return txt;
}

/* Get ntp configuration */
vtss_rc ntp_mgmt_conf_get(ntp_conf_t *conf)
{
    T_D("enter");
    NTP_CRIT_ENTER();

    *conf = ntp_global.ntp_conf;

    NTP_CRIT_EXIT();
    T_D("exit");

    return VTSS_OK;
}

/* Set ntp configuration */
vtss_rc ntp_mgmt_conf_set(ntp_conf_t *conf)
{
    vtss_rc         rc      = VTSS_OK;
    int             changed = 0;

    T_D("enter, mode: %ld", (long)conf->mode_enabled);

    /* check illegal parameter */
    if (conf->mode_enabled != NTP_MGMT_ENABLED && conf->mode_enabled != NTP_MGMT_DISABLED) {
        return ((vtss_rc)NTP_ERROR_PARM);
    }
    if (conf->interval_min < NTP_MININTERVAL || conf->interval_max > NTP_MAXINTERVAL) {
        return ((vtss_rc)NTP_ERROR_PARM);
    }

    NTP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        changed = ntp_conf_changed(&ntp_global.ntp_conf, conf);
        old_ntp_conf = ntp_global.ntp_conf;
        ntp_global.ntp_conf = *conf;
    } else {
        T_W("not master");
        rc = (vtss_rc)NTP_ERROR_STACK_STATE;
    }
    NTP_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t  blk_id  = CONF_BLK_NTP_CONF;
        ntp_conf_blk_t *ntp_conf_blk_p;
        if ((ntp_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open ntp table");
        } else {
            ntp_conf_blk_p->ntp_conf = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

        /* Activate changed configuration */
        ntp_stack_ntp_conf_set(VTSS_ISID_GLOBAL);
    }

    T_D("exit");

    return rc;
}


static int ntp_dppm = NTP_DEFAULT_DRIFT;
static ulong ntp_updatime = 0;
static double ntp_offset = 0;
static int ntp_drift = 0;
static int ntp_first_dppm = 0;
static int ntp_max_dppm = 0;
static int ntp_min_dppm = 0;
static ulong ntp_step_count = 0;
static ulong ntp_step_time[STEP_ENTRY_NO];
static char ntp_per_server[16];
static char ntp_last_offset_in_range;
static double ntp_last_offest;
static int ntp_current_status;
static ntp_freq_data_t ntp_freq;
static unsigned char timer_reset_count = 0;

void vtss_ntp_timer_reset_counter(void)
{
    timer_reset_count++;
}


void vtss_ntp_timer_reset(void)
{
    ntp_timer_reset();
}

void vtss_ntp_timer_init(void)
{
    ntp_timer_init();
}


void vtss_ntp_last_sys_info(char in_range, double offset, int status)
{
    ntp_last_offset_in_range = in_range;
    ntp_last_offest = offset;
    ntp_current_status = status;
}

void vtss_ntp_freq_info(ulong mu, double curr_offset, double last_offset, double result_frequency)
{
    ntp_freq.mu = mu;
    ntp_freq.curr_offset = curr_offset;
    ntp_freq.last_offset = last_offset;
    ntp_freq.result_frequency = result_frequency;
}


void vtss_ntp_freq_get(ntp_freq_data_t *freq)
{
    freq->mu = ntp_freq.mu;
    freq->curr_offset = ntp_freq.curr_offset;
    freq->last_offset = ntp_freq.last_offset;
    freq->result_frequency = ntp_freq.result_frequency;
}

/* Set ntp defaults */
void vtss_ntp_default_set(ntp_conf_t *conf)
{
    ntp_default_set(conf);
    return;
}

/* Set ntp configuration */
void vtss_ntp_adj_time(double dppm, ulong updatime, double offset, char *ipstr)
{
    int     deltappm;
#if 0
    ulong   reload_value = hal_get_reload_value();
#endif

    deltappm = -((int)(dppm * 1e6) + ntp_drift);
    if (deltappm != ntp_dppm) {
#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)
        hal_clock_set_adjtimer(deltappm);
#endif
    }

#if 0
    printf("hal_clock_set_adjtimer set IPaddr %s(%lu) %d PPM (%.6f) -- %x\n", ipstr, updatime, deltappm, offset, reload_value);
    printf("hal_clock_set_adjtimer set IPaddr %s(%lu) %d PPM (%.6f)\n", ipstr, updatime, deltappm, offset);
#endif


    ntp_dppm = deltappm;
    ntp_updatime = updatime;
    ntp_offset = offset;
    memset(ntp_per_server, 0, sizeof(ntp_per_server));
    strcpy(ntp_per_server, ipstr);

    if (ntp_first_dppm  == 0) {
        ntp_max_dppm = ntp_min_dppm = ntp_first_dppm = deltappm;
    }

    if (ntp_min_dppm > deltappm) {
        ntp_min_dppm = deltappm;
    }

    if (ntp_max_dppm < deltappm) {
        ntp_max_dppm = deltappm;
    }
}

extern u_long   current_time;       /* current time (s) */

void ntp_mgmt_sys_status_get(ntp_sys_status_t *status)
{
    NTP_CRIT_ENTER();
    status->currentime = current_time;
    status->currentppm = ntp_dppm;
    status->updatime = ntp_updatime;
    status->offset = ntp_offset;
    status->drift = ntp_drift;
    status->first_dppm = ntp_first_dppm;
    status->max_dppm = ntp_max_dppm;
    status->min_dppm = ntp_min_dppm;
    status->step_count = ntp_step_count;
    //status->step_time = ntp_step_time;
    memcpy(status->step_time, ntp_step_time, sizeof(status->step_time));
    memcpy(status->ip_string, ntp_per_server, sizeof(status->ip_string));
    status->timer_rest_count = timer_reset_count;
    status->last_offset_in_range = ntp_last_offset_in_range;
    status->last_offest = ntp_last_offest;
    status->current_status = ntp_current_status;
    NTP_CRIT_EXIT();
}


void vtss_ntp_step_count_set(unsigned long  count)
{
    static ulong  i = 0;

    ntp_step_count = count;
    ntp_step_time[i] = current_time;

    i = (i + 1) % STEP_ENTRY_NO;
}


void
vtss_ntp_server_info(unsigned long curr_time, char *ip_str, int flag,
                     int poll, int burst, unsigned long lastupdate,
                     unsigned long nextupdate, double offset)
{
    int i, avi_index = -1;

    for (i = 0; i < 5; i++) {
        if (memcmp(server_info[i].ip_string, ip_str,
                   sizeof(server_info[i].ip_string)) == 0) {

            server_info[i].curr_time = curr_time;
            server_info[i].flag = flag;
            server_info[i].poll_int = poll;
            server_info[i].burst = burst;
            server_info[i].lastupdate = lastupdate;
            server_info[i].nextupdate = nextupdate;
            server_info[i].offset = offset;
            break;
        } else if (avi_index == -1 && server_info[i].poll_int == 0) {
            avi_index = i;
        }
    }

    if (i == 5) {
        /* not found */
        memcpy(server_info[avi_index].ip_string, ip_str,
               sizeof(server_info[avi_index].ip_string));
        server_info[avi_index].curr_time = curr_time;
        server_info[avi_index].flag = flag;
        server_info[avi_index].poll_int = poll;
        server_info[avi_index].burst = burst;
        server_info[avi_index].lastupdate = lastupdate;
        server_info[avi_index].nextupdate = nextupdate;
    }
}

void ntp_mgmt_sys_server_get(ntp_server_status_t *status, ulong *curr_time, int num)
{
    NTP_CRIT_ENTER();
    *curr_time = current_time;
    memcpy(status, server_info, num * sizeof(ntp_server_status_t));
    NTP_CRIT_EXIT();
}

/****************************************************************************
 * Module thread
 ****************************************************************************/
static int vtss_ntp_ip_ready(void)
{
#define MAX_IF              130
#define IP2_MAX_STATUS_OBJS 1024
    u32                 i, j;
    int                 res = 1;
    u32                 if_st_cnt = 0;
    vtss_if_status_t    *status;
    vtss_ipv6_t         null_ipv6_addr;
    ntp_conf_t          ntp_conf;

    NTP_CRIT_ENTER();
    ntp_conf = ntp_global.ntp_conf;
    NTP_CRIT_EXIT();

    if (!ntp_conf.mode_enabled ||
        ((status = VTSS_CALLOC(IP2_MAX_STATUS_OBJS, sizeof(vtss_if_status_t))) == NULL)) {
        return res;
    }

    memset(&null_ipv6_addr, 0 , sizeof(null_ipv6_addr));
    for (i = 0; i < NTP_MAX_SERVER_COUNT; i++) {
        if ((ntp_conf.server[i].ip_type == NTP_IP_TYPE_IPV4 && strlen(ntp_conf.server[i].ip_host_string) == 0)
#ifdef VTSS_SW_OPTION_IPV6
            || (ntp_conf.server[i].ip_type == NTP_IP_TYPE_IPV6 && memcmp(ntp_conf.server[i].ipv6_addr.addr, null_ipv6_addr.addr, sizeof(null_ipv6_addr)) == 0)
#endif
           ) {
            continue;
        }

        if (vtss_ip2_ifs_status_get(VTSS_IF_STATUS_TYPE_ANY, IP2_MAX_STATUS_OBJS, &if_st_cnt, status) == VTSS_RC_OK) {
            for (j = 0; j < if_st_cnt; j++) {
                if (status[j].if_id.type == VTSS_ID_IF_TYPE_VLAN &&
                    (status[j].type == VTSS_IF_STATUS_TYPE_IPV4
#ifdef VTSS_SW_OPTION_IPV6
                     || (status[j].type == VTSS_IF_STATUS_TYPE_IPV6 && status[j].u.ipv6.net.address.addr[0] != 0xfe && status[j].u.ipv6.net.address.addr[1] != 0x80)
#endif
                    )) {
                    res = 0;
                    break;
                }
            }
        }
    }

    VTSS_FREE(status);
    return res;
}


static void ntp_base_pthread_create(double *old_drift)
{
    pthread_t ntp_base_thread;
    pthread_attr_t attr;
    struct sched_param schedparam;
    int rc;

    /* Create ntp thread */
    rc = pthread_attr_init( &attr );
    if (rc != 0) {
        T_W("ntp_create_pthread: pthread_attr_init fails");
    }

    schedparam.sched_priority = 24;
    rc = pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    if (rc != 0) {
        T_W("ntp_create_pthread: pthread_attr_setinheritsched fails");
    }
    rc = pthread_attr_setschedpolicy( &attr, SCHED_RR );
    if (rc != 0) {
        T_W("ntp_create_pthread: pthread_attr_setschedpolicy fails");
    }
    rc = pthread_attr_setschedparam( &attr, &schedparam );
    if (rc != 0) {
        T_W("ntp_create_pthread: pthread_attr_setschedparam fails");
    }
    rc = pthread_attr_setstackaddr( &attr, (void *)&ntp_pthread_stack[sizeof(ntp_pthread_stack)] );
    if (rc != 0) {
        T_W("ntp_create_pthread: pthread_attr_setstackaddr fails");
    }
    rc = pthread_attr_setstacksize( &attr, sizeof(ntp_pthread_stack) );
    if (rc != 0) {
        T_W("ntp_create_pthread: pthread_attr_setstacksize fails");
    }

    rc = pthread_create( &ntp_base_thread, &attr, base_ntp_client, (void *) old_drift );
    if (rc != 0) {
        T_W("ntp_create_pthread: pthread_create fails");
    } else {
        T_D( "ntp_init_client\n");
    }
}

static void ntp_thread(cyg_addrword_t data)
{
#if NTP_USING_NTP_PACKAGE
    double drift_data;

    while (vtss_ntp_ip_ready() != 0) { /* waiting for IP address ready */
        VTSS_OS_MSLEEP(1000);
    }

    if (ntp_global.ntp_conf.drift_valid == 1) {
        drift_data = (ntp_global.ntp_conf.drift_data);
        ntp_drift = ntp_dppm = (int)drift_data  + NTP_DEFAULT_DRIFT;
#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)
        hal_clock_set_adjtimer(-ntp_drift);
#endif
    } else {
        drift_data = 1e9;
        ntp_drift = ntp_dppm = NTP_DEFAULT_DRIFT;
#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)
        hal_clock_set_adjtimer(-NTP_DEFAULT_DRIFT);
#endif
    }

    if (ntp_global.ntp_conf.drift_trained != 1 ||
        ntp_global.ntp_conf.drift_valid != 1) {
        drift_data = 1e9; // James : test only. test different dirft without drift data
    } else {
        drift_data = 0; /* ntp module never knows the drift */
    }
    ntp_base_pthread_create(&drift_data);


    VTSS_OS_MSLEEP(1000);
    ntp_initial = 1;
    (void)ntp_set_ntp_config(&ntp_global.ntp_conf);


    while (1) {
        int i;
        struct hostent *hp;
        int rc;

        if (msg_switch_is_master()) {
            VTSS_OS_MSLEEP(5000);

            if (ntp_global.ntp_conf.mode_enabled == TRUE) {
                for (i = 0; i < NTP_MAX_SERVER_COUNT; i++) {
                    if (ntp_global.ntp_conf.server[i].ip_type == NTP_IP_TYPE_IPV4) {
                        if (ntp_global.ntp_conf.server[i].ip_host_string[0] != '\0' &&
                            ip_addr_string[i][0] == '\0') {
                            hp = gethostbyname((char *)ntp_global.ntp_conf.server[i].ip_host_string);
                            if (hp != NULL) {
                                struct in_addr ip;
                                bcopy(hp->h_addr_list[0], (char *)&ip, sizeof(ip));
#if 0
                                memcpy(&ip_addr_string[i][0], inet_ntoa(ip), strlen(inet_ntoa(ip)));
                                T_E("ntp_set_ntp_server error -g1  %d   %s\n", i, &ip_addr_string[i][0]);
                                ip_addr_string[i][strlen(inet_ntoa(ip))] = '\0';
                                if (i == 2) {
                                    ip_addr_string[i][strlen(inet_ntoa(ip))] = '1';
                                    ip_addr_string[i][strlen(inet_ntoa(ip)) + 1] = '1';
                                }
#endif
                                strcpy((char *)&ip_addr_string[i][0], inet_ntoa(ip));
#if 0
                                T_E("ntp_set_ntp_server error -g2  %d   %s\n", i, &ip_addr_string[i][0]);
#endif
                                rc = ntp_set_ntp_server((int)ntp_global.ntp_conf.server[i].ip_type, (char *)&ip_addr_string[i][0]);
                                if (rc != 0) {
                                    T_D("ntp_set_ntp_server error -g3  %d   %s\n", i, &ip_addr_string[i][0]);
                                }
                            }
                        }
                    }
                }
            }
        } else {
            /* Suspend ntp thread (Only needed in master mode) */
            cyg_thread_suspend(ntp_thread_handle);
        }
    }

#else
    while (1) {
        VTSS_OS_MSLEEP(1000);
    }
#endif /* NTP_USING_NTP_PACKAGE */
}

#if NTP_USING_NTP_PACKAGE
/****************************************************************************/
/*  NTP Base Module callouts                                                */
/****************************************************************************/
void *ntp_callout_malloc(size_t size)
{
    return VTSS_MALLOC(size);
}

void *ntp_callout_calloc(size_t nmemb, size_t size)
{
    return VTSS_CALLOC(nmemb, size);
}

char *ntp_callout_strdup(const char *str)
{
    return VTSS_STRDUP(str);
}

void ntp_callout_free(void *ptr)
{
    VTSS_FREE(ptr);
}
#endif /* NTP_USING_NTP_PACKAGE */

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create ntp stack configuration */
static void ntp_conf_read_stack(BOOL create)
{
    int             changed;
    BOOL            do_create;
    ulong           size;
    ntp_conf_t      *old_ntp_conf_p, new_ntp_conf;
    ntp_conf_blk_t  *conf_blk_p;
    conf_blk_id_t   blk_id;
    ulong           blk_version;

    T_D("enter, create: %d", create);

    /* Read/create ntp configuration */
    blk_id = CONF_BLK_NTP_CONF;
    blk_version = NTP_CONF_BLK_VERSION;

    if (misc_conf_read_use()) {
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = 1;
        } else if (conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = 1;
        } else {
            do_create = create;
        }
    } else {
        conf_blk_p = NULL;
        do_create  = TRUE;
    }

    changed = 0;
    NTP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        ntp_default_set(&new_ntp_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->ntp_conf = new_ntp_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {
            new_ntp_conf = conf_blk_p->ntp_conf;
        }
    }
    old_ntp_conf_p = &ntp_global.ntp_conf;
    if (ntp_conf_changed(old_ntp_conf_p, &new_ntp_conf)) {
        changed = 1;
    }
    ntp_global.ntp_conf = new_ntp_conf;
    NTP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open ntp table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed && create) {
        ntp_stack_ntp_conf_set(VTSS_ISID_GLOBAL);
    }

    T_D("exit");
}

/* Module start */
static void ntp_start(BOOL init)
{
    ntp_conf_t *conf_p;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize ntp configuration */
        conf_p = &ntp_global.ntp_conf;
        ntp_default_set(conf_p);

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&ntp_global.request.sem, 1);

        /* Create semaphore for critical regions */
        critd_init(&ntp_global.crit, "ntp_global.crit", VTSS_MODULE_ID_NTP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        NTP_CRIT_EXIT();

        /* Create ntp thread */
#if NTP_USING_NTP_PACKAGE
#if 0 // James
        ntp_base_pthread_create();
#endif
        /* Create NTP thread */
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          ntp_thread,
                          0,
                          "NTP Main",
                          ntp_thread_stack,
                          sizeof(ntp_thread_stack),
                          &ntp_thread_handle,
                          &ntp_thread_block);

#endif
    } else {
        /* Register for stack messages */
        (void)ntp_stack_register();
    }
    T_D("exit");
}

/* Initialize module */
vtss_rc ntp_init(vtss_init_data_t *data)
{
#ifdef VTSS_SW_OPTION_ICFG
    vtss_rc     rc = VTSS_OK;
#endif
    vtss_isid_t isid = data->isid;
    int i;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        ntp_start(1);
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = vtss_ntp_icfg_init()) != VTSS_OK) {
            T_D("Calling vtss_ntp_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
        ntp_start(0);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            ntp_conf_read_stack(1);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");

#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)
        hal_clock_enable_set_adjtimer(1, 0);
#endif
        memset(&old_ntp_conf, 0, sizeof(old_ntp_conf));
        old_ntp_conf.mode_enabled = NTP_MGMT_INITIAL;

        memset(server_info, 0, sizeof(server_info));

        for (i = 0; i < STEP_ENTRY_NO; i++) {
            ntp_step_time[i] = 0;
        }

        for (i = 0; i < NTP_MAX_SERVER_COUNT; i++) {
            ip_addr_string[i][0] = '\0';
            // James1014 memset(&ip_addr_string[i][0], '\0', 46);
        }


        /* Read stack and switch configuration */
        ntp_conf_read_stack(0);

        /* Starting ntp thread (became master) */
        cyg_thread_resume(ntp_thread_handle);
        break;
    }
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
#if NTP_USING_NTP_PACKAGE
        /* James:(todo) disable ntp  */
        ntp_global.ntp_conf.mode_enabled = NTP_MGMT_DISABLED;
        (void)ntp_set_ntp_config(&ntp_global.ntp_conf);
#endif /* NTP_USING_NTP_PACKAGE */
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply all configuration to switch */
        ntp_stack_ntp_conf_set(isid);
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

#if 0

/*
 * The following function is for debug. Move it to the flie named
 * eCos\packages\hal\arm\arm9\vcoreii\current\src\vcoreii_misc.c
 *
 */
unsigned int
hal_get_reload_value()
{
    return VTSS_TIMERS_TIMER_RELOAD_VALUE_0;
}


#endif


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
