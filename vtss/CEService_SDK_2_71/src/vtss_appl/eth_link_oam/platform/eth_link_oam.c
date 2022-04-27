/*

 Vitesse ETH Link OAM software.

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

#include "main.h"                       /* For Link OAM module initialization hookups */
#include "conf_api.h"                   /* For persisting the Link OAM Configuration  */
#include "port_api.h"                   /* For port events callbacks */
#include "packet_api.h"                 /* For OAM frames handlers   */
#include "eth_link_oam_api.h"           /* For ETH Link OAM module   */
#include "vtss_types.h"
#include "acl_api.h"                    /* For ACE management APIs   */
#include "misc_api.h"                   /* instantiate MAC */
#ifdef VTSS_SW_OPTION_VCLI
#include "cli.h"                        /* For cprintf*/
#include "eth_link_oam_cli.h"           /* For eth_link_oam_cli_req_init */
#endif /* VTSS_SW_OPTION_VCLI */
#ifdef VTSS_SW_OPTION_ICFG
#include "eth_link_oam_icfg.h"
#endif
#include "netdb.h"                      /* For byte-order swap functions */

/*lint -sem( vtss_eth_link_oam_crit_oper_data_lock, thread_lock ) */
/*lint -sem( vtss_eth_link_oam_crit_oper_data_unlock, thread_unlock ) */

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>             /* For Link OAM module ID */
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_ETH_LINK_OAM
#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_ETH_LINK_OAM

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

/* end */
#include <vtss_trace_api.h>

/*****************************************************************************/
/*  Static declarations                                                      */
/*****************************************************************************/
#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
static void eth_link_oam_port_local_lost_link_timer_done(vtss_port_no_t port_no);
#endif

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

// Filter id for subscribing Link OAM frame.
void                          *eth_link_oam_filter_id = NULL;

/* Global module/API Lock for Critical OAM data protection */
critd_t                            oam_data_lock;

/* Module Lock for critical OAM operaional data(OAM client data) protection */
critd_t                            oam_oper_lock;

/* Module Lock for critical OAM control layer date(OAM control layer) protection */
critd_t                            oam_control_layer_lock;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "LINK_OAM",
    .descr     = "Eth Link OAM module."
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
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    }
};
#define CRIT_ENTER(crit) critd_enter(&crit,\
        TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define CRIT_EXIT(crit)  critd_exit(&crit, \
        TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#else
#define CRIT_ENTER(crit) critd_enter(&crit)
#define CRIT_EXIT(crit)  critd_exit(&crit)
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/
typedef struct {
    ulong                          version;                          /* Block version */
    vtss_eth_link_oam_control_t    oam_control[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    vtss_eth_link_oam_mode_t       oam_mode[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    BOOL                           oam_mib_retrival_support[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    BOOL                           oam_remote_loop_back_support[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    BOOL                           oam_link_monitoring_support[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    u16                            oam_error_frame_event_window[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    u32                            oam_error_frame_event_threshold[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    u64                            oam_symbol_period_error_event_window[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    u64                            oam_symbol_period_error_event_threshold[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    u32                            oam_symbol_period_error_event_rxthreshold[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    u32                            oam_frame_period_error_event_window[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    u32                            oam_frame_period_error_event_threshold[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    u32                            oam_frame_period_error_event_rxthreshold[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    u16                            oam_error_frame_secs_summary_event_window[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];
    u32                            oam_error_frame_secs_summary_event_threshold[VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT];

} eth_link_oam_conf_t;

static cyg_handle_t    eth_link_oam_control_timer_thread_handle;
static cyg_thread      eth_link_oam_control_timer_thread_block;
static char            eth_link_oam_control_timer_thread_stack[THREAD_DEFAULT_STACK_SIZE];

static cyg_handle_t    eth_link_oam_client_thread_handle;
static cyg_thread      eth_link_oam_client_thread_block;
static char            eth_link_oam_client_thread_stack[THREAD_DEFAULT_STACK_SIZE];

/* MBOX Variables */
static cyg_handle_t    eth_link_oam_mbhandle;
static cyg_mbox        eth_link_oam_mbox;
static cyg_sem_t       eth_link_oam_var_sem;
static cyg_sem_t       eth_link_oam_rlb_sem;


static u8              ETH_LINK_OAM_MULTICAST_MACADDR[] = { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x02 };
static u8              sys_mac_addr[VTSS_ETH_LINK_OAM_MAC_LEN];

static vtss_rc rc_conv(u32 oam_rc)
{
    vtss_rc rc = VTSS_RC_ERROR;

    switch (oam_rc) {
    case VTSS_ETH_LINK_OAM_RC_OK:
        rc = VTSS_RC_OK;
        break;
    case VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER:
        rc = ETH_LINK_OAM_RC_INVALID_PARAMETER;
        break;
    case VTSS_ETH_LINK_OAM_RC_NOT_ENABLED:
        rc = ETH_LINK_OAM_RC_NOT_ENABLED;
        break;
    case VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED:
        rc = ETH_LINK_OAM_RC_ALREADY_CONFIGURED;
        break;
    case VTSS_ETH_LINK_OAM_RC_NO_MEMORY:
        rc = ETH_LINK_OAM_RC_NO_MEMORY;
        break;
    case VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED:
        rc = ETH_LINK_OAM_RC_NOT_SUPPORTED;
        break;
    case VTSS_ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY:
        rc = ETH_LINK_OAM_RC_NO_SUFFIFIENT_MEMORY;
        break;
    case VTSS_ETH_LINK_OAM_RC_INVALID_STATE:
        rc = ETH_LINK_OAM_RC_INVALID_STATE;
        break;
    case VTSS_ETH_LINK_OAM_RC_INVALID_FLAGS:
        rc = ETH_LINK_OAM_RC_INVALID_FLAGS;
        break;
    case VTSS_ETH_LINK_OAM_RC_INVALID_CODES:
        rc = ETH_LINK_OAM_RC_INVALID_CODES;
        break;
    case VTSS_ETH_LINK_OAM_RC_INVALID_PDU_CNT:
        rc = ETH_LINK_OAM_RC_INVALID_PDU_CNT;
        break;
    case VTSS_ETH_LINK_OAM_RC_TIMED_OUT:
        rc = ETH_LINK_OAM_RC_TIMED_OUT;
        break;
    default:
        T_E("Invalid OAM module error is noticed:- %u", oam_rc);
        break;
    }
    return rc;
}

/* Enables/Disables the OAM Control on the port                               */
vtss_rc eth_link_oam_mgmt_port_control_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const vtss_eth_link_oam_control_t conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    do {
        rc = rc_conv(
                 vtss_eth_link_oam_mgmt_port_control_conf_set(l2port, conf));
        if (rc == VTSS_RC_OK) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            eth_link_oam_conf_t      *blk;
            ulong                    size;

            blk = conf_sec_open(CONF_SEC_GLOBAL,
                                CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
            if (blk != NULL) {
                blk->oam_control[l2port] = conf;
                conf_sec_close(CONF_SEC_GLOBAL,
                               CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
            } else {
                rc = VTSS_RC_ERROR; //Is this enough?
                T_E("Unable to save the specified configuration on port(%d/%u)",
                    isid, port_no);
            }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL);

    return rc;
}

/* Retrieve the OAM Control on the port                                     */
vtss_rc eth_link_oam_mgmt_port_control_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_control_t *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_control_conf_get(l2port, conf));
    return rc;
}

/* Configures the Port's OAM mode                                   */
vtss_rc eth_link_oam_mgmt_port_mode_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const vtss_eth_link_oam_mode_t conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_mode_conf_set(l2port, conf, FALSE));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL,
                            CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_mode[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL,
                           CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* Retrieves the Link OAM port mode                                   */
vtss_rc eth_link_oam_mgmt_port_mode_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_mode_t *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(vtss_eth_link_oam_mgmt_port_mode_conf_get(l2port, conf));
    return rc;
}

/* Configures the Port's MIB retrival support */
vtss_rc eth_link_oam_mgmt_port_mib_retrival_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const BOOL conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(vtss_eth_link_oam_mgmt_port_mib_retrival_conf_set(l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL,
                            CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_mib_retrival_support[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL,
                           CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* Retrieves the port's MIB retrival support                                   */
vtss_rc eth_link_oam_mgmt_port_mib_retrival_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, BOOL *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(vtss_eth_link_oam_mgmt_port_mib_retrival_conf_get(l2port, conf));
    return rc;
}

/* Configures the Port's remote loopback support */
vtss_rc eth_link_oam_mgmt_port_remote_loopback_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const BOOL conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_remote_loopback_conf_set(l2port, conf, FALSE));

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk = NULL;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL,
                            CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_remote_loop_back_support[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL,
                           CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* Retrieves the port's remote loopback  support                                   */
vtss_rc eth_link_oam_mgmt_port_remote_loopback_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, BOOL *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(vtss_eth_link_oam_mgmt_port_remote_loopback_conf_get(l2port, conf));
    return rc;
}

/* Configures the Port's link monitoring support */
vtss_rc eth_link_oam_mgmt_port_link_monitoring_conf_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const BOOL conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(vtss_eth_link_oam_mgmt_port_link_monitoring_conf_set(l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_link_monitoring_support[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* Retrieves the port's remote loopback  support                                   */
vtss_rc eth_link_oam_mgmt_port_link_monitoring_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, BOOL *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(vtss_eth_link_oam_mgmt_port_link_monitoring_conf_get(
                     l2port, conf));
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Window                           */
/* Configures the Error Frame Event Window Configuration                      */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_window_set (const vtss_isid_t isid, const u32  port_no, const u16 conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_error_frame_window_set(
                 l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL,
                            CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_error_frame_event_window[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Window                           */
/* Retrieves the Error Frame Event Window Configuration                       */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_window_get (const vtss_isid_t isid, const u32  port_no, u16 *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_error_frame_window_get(
                 l2port, conf));
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Threshold                        */
/* Configures the Error Frame Event Threshold Configuration                   */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_threshold_set(const vtss_isid_t isid, const u32  port_no, const u32 conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(vtss_eth_link_oam_mgmt_port_link_error_frame_threshold_set(
                     l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk = NULL;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL,
                            CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_error_frame_event_threshold[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Event Threshold                        */
/* Retrieves the Error Frame Event Threshold Configuration                    */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_threshold_get (const vtss_isid_t isid, const u32  port_no, u32 *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_error_frame_threshold_get(
                 l2port, conf));
    return rc;

}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error Window                         */
/* Configures the Symbol Period Error Window Configuration                    */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_window_set (const vtss_isid_t isid, const u32  port_no, const u64  conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_symbol_period_error_window_set(
                 l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL,
                            CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_symbol_period_error_event_window[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL,
                           CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error Window                         */
/* Retrieves the Symbol Period Error Window Configuration                     */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_window_get (const vtss_isid_t isid, const u32  port_no, u64 *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_symbol_period_error_window_get(
                 l2port, conf));
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error Period Threshold               */
/* Configures the Symbol Period Error Period Threshold Configuration          */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_threshold_set (const vtss_isid_t isid, const u32  port_no, const u64  conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_symbol_period_error_threshold_set(
                 l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL,
                            CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_symbol_period_error_event_threshold[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error Period Threshold               */
/* Retrieves the Symbol Period Error Period Threshold Configuration           */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_threshold_get (const vtss_isid_t isid, const u32  port_no, u64 *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_symbol_period_error_threshold_get(
                 l2port, conf));
    return rc;

}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error RxPackets Threshold            */
/* Configures the Symbol Period Error RxPackets Threshold Configuration       */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_set (const vtss_isid_t isid, const u32  port_no, const u64  conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_set(
                 l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_symbol_period_error_event_rxthreshold[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Symbol Period Error RxPackets Threshold            */
/* Retreives the Symbol Period Error RxPackets Threshold Configuration        */
vtss_rc eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_get (const vtss_isid_t isid, const u32  port_no, u64 *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_get(
                 l2port, conf));
    return rc;
}

/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error Window                          */
/* Configures the Frame Period Error Window Configuration                     */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_window_set (const vtss_isid_t isid, const u32  port_no, const u32 conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_frame_period_error_window_set(
                 l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL,
                            CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_frame_period_error_event_window[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error Window                          */
/* Retrieves the Frame Period Error Window Configuration                      */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_window_get (const vtss_isid_t isid, const u32  port_no, u32 *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_frame_period_error_window_get(
                 l2port, conf));
    return rc;
}

/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error Period Threshold                */
/* Configures the Frame Period Error Period Threshold Configuration           */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_threshold_set (const vtss_isid_t isid, const u32  port_no, const u32  conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_frame_period_error_threshold_set(
                 l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL,
                            CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_frame_period_error_event_threshold[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error Period Threshold                */
/* Retrieves the Frame Period Error Period Threshold Configuration            */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_threshold_get (const vtss_isid_t isid, const u32  port_no, u32        *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_frame_period_error_threshold_get(
                 l2port, conf));
    return rc;
}

/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error RxPackets Threshold             */
/* Configures the Frame Period Error RxPackets Threshold Configuration        */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_set (const vtss_isid_t isid, const u32  port_no, const u64  conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_set(
                 l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_frame_period_error_event_rxthreshold[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* isid   :    slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Frame Period Error RxPackets Threshold             */
/* Retrives the Frame Period Error RxPackets Threshold Configuration          */
vtss_rc eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_get (const vtss_isid_t isid, const u32  port_no, u64 *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_get(
                 l2port, conf));
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Seconds Summary Event Window           */
/* Configures the Error Frame Seconds Summary Event Window Configuration      */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_set (const vtss_isid_t isid, const u32  port_no, const u16 conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_set(
                 l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_error_frame_secs_summary_event_window[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Seconds Summary Event Window           */
/* Retrieves the Error Frame Seconds Summary Event Window Configuration       */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_get (const vtss_isid_t isid, const u16  port_no, u16 *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_get(
                 l2port, conf));
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Seconds Summary Event Threshold        */
/* Configures the Error Frame Seconds Summary Event Threshold Configuration   */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_set(const vtss_isid_t isid, const u32  port_no, const u16 conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_set(
                 l2port, conf));
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (rc == VTSS_RC_OK ) {
        eth_link_oam_conf_t      *blk;
        ulong                    size;
        blk = conf_sec_open(CONF_SEC_GLOBAL,
                            CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
        if (blk != NULL) {
            blk->oam_error_frame_secs_summary_event_threshold[l2port] = conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {

            T_E("Unable to save the specified configuration on port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return rc;
}

/* isid   :    Slot Number                                                    */
/* port_no:    l2 port number                                                 */
/* conf:       Port's link Error Frame Seconds Summary Event Threshold        */
/* Retrieves the Error Frame Seconds Summary Event Threshold Configuration    */
vtss_rc eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_get (const vtss_isid_t isid, const u16  port_no, u16 *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_get(
                 l2port, conf));
    return rc;

}
/* End */
/* Configures the Port's MIB retrival support */
vtss_rc eth_link_oam_mgmt_port_mib_retrival_oper_set (const vtss_isid_t isid, const vtss_port_no_t port_no, const u8 conf)

{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;
    u8                       oam_pdu[VTSS_ETH_LINK_OAM_PDU_MIN_LEN];
    u8                       var_branch = VTSS_ETH_LINK_OAM_VAR_ATTRIBUTE;
    u16                      var_leaf[5];
    u16                      current_len = VTSS_ETH_LINK_OAM_PDU_HDR_LEN;
    u8                       tmp_index = 0, total_leaves = 0;
    vtss_eth_link_oam_mode_t oam_conf;
    BOOL                     mib_ret_support = FALSE;

    do {

        rc = vtss_eth_link_oam_mgmt_port_mode_conf_get(l2port, &oam_conf);

        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_E("Unable to retrive the mode of the port(%d/%u)", isid, port_no);
            break;
        }

        if (oam_conf == VTSS_ETH_LINK_OAM_MODE_PASSIVE) {
            rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
            break;
        }

        rc = vtss_eth_link_oam_mgmt_port_mib_retrival_conf_get(l2port,
                                                               &mib_ret_support);

        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_E("Unable to retrive the configuration of the port(%d/%u)", isid, port_no);
            break;
        }

        if (mib_ret_support == FALSE) {
            T_D("MIB retrival support is not supported on port(%d/%u)", isid, port_no);
            rc = VTSS_ETH_LINK_OAM_RC_NOT_ENABLED;
            break;
        }

        if (conf == 1) { /* Local_info */
            var_leaf[0] = VTSS_ETH_LINK_OAM_ID;
            var_leaf[1] = VTSS_ETH_LINK_OAM_LOCAL_CONF;
            var_leaf[2] = VTSS_ETH_LINK_OAM_LOCAL_PDU_CONF;
            var_leaf[3] = VTSS_ETH_LINK_OAM_LOCAL_REVISION_CONF;
            var_leaf[4] = VTSS_ETH_LINK_OAM_LOCAL_STATE;
            total_leaves = 5;
        } else { /* remote info */
            var_leaf[0] = VTSS_ETH_LINK_OAM_REMOTE_CONF;
            var_leaf[1] = VTSS_ETH_LINK_OAM_REMOTE_PDU_CONF;
            var_leaf[2] = VTSS_ETH_LINK_OAM_REMOTE_REVISION;
            var_leaf[3] = VTSS_ETH_LINK_OAM_REMOTE_STATE;
            total_leaves = 4;
        }

        memset(oam_pdu, '\0', VTSS_ETH_LINK_OAM_PDU_MIN_LEN);

        for (tmp_index = 0; tmp_index < total_leaves; tmp_index++) {

            rc = vtss_eth_link_oam_client_port_build_var_descriptor(l2port,
                                                                    oam_pdu,
                                                                    var_branch,
                                                                    var_leaf[tmp_index],
                                                                    &current_len,
                                                                    VTSS_ETH_LINK_OAM_PDU_MIN_LEN);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                T_E("Error:%u occured while building the MIB variable of the port(%d/%u)",
                    rc, isid, port_no);
                break;
            }
        }
        rc = vtss_eth_link_oam_mgmt_control_port_non_info_pdu_xmit(port_no,
                                                                   oam_pdu,
                                                                   VTSS_ETH_LINK_OAM_PDU_MIN_LEN,
                                                                   VTSS_ETH_LINK_OAM_CODE_TYPE_VAR_REQ);

        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_E("Error:%u occured while building MIB variable on port(%d/%u)",
                rc, isid, port_no);
            break;
        }

    } while (VTSS_ETH_LINK_OAM_NULL);

    return (rc_conv(rc));
}

vtss_rc eth_link_oam_mgmt_port_remote_loopback_oper_conf_set(const vtss_isid_t isid, const vtss_port_no_t port_no, const BOOL conf)
{

    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(vtss_eth_link_oam_mgmt_port_remote_loopback_oper_conf_set(l2port, conf));

    if (rc == VTSS_RC_OK) {
        T_D("Remote loopback operation is enabled on port(%u/%u)", isid, port_no);
        if (conf == FALSE) {
            rc = rc_conv(vtss_eth_link_oam_mgmt_port_state_conf_set(l2port,
                                                                    VTSS_ETH_LINK_OAM_MUX_FWD_STATE,
                                                                    VTSS_ETH_LINK_OAM_PARSER_FWD_STATE));
        }
    } else if (rc == ETH_LINK_OAM_RC_TIMED_OUT) {
        T_D("Remote loopback operation is timed out on port(%u/%u)", isid, port_no);
    }
    return rc;
}

/******************************************************************************/
/* Link OAM's utility functions                                               */
/******************************************************************************/

u16 vtss_eth_link_oam_htons(u16 num)
{
    return HOST2NETS(num);
}

u32 vtss_eth_link_oam_htonl(u32 num)
{
    return HOST2NETL(num);
}

u16 vtss_eth_link_oam_ntohs(u16 num)
{
    return NET2HOSTS(num);
}

u32 vtss_eth_link_oam_ntohl(u32 num)
{
    return NET2HOSTL(num);
}

u16 vtss_eth_link_oam_ntohs_from_bytes(u8 tmp[])
{
    u16 num;

    memcpy(&num, tmp, sizeof(u16));
    return NET2HOSTS(num);
}

u32 vtss_eth_link_oam_ntohl_from_bytes(u8 tmp[])
{
    u32 num;

    memcpy(&num, tmp, sizeof(u32));
    return NET2HOSTL(num);
}



u64 vtss_eth_link_oam_swap64(u64 num)
{
    num = ( ( (num & 0xff00000000000000ULL) >> 56 ) |
            ( (num & 0x00000000000000ffULL) << 56 ) |
            ( (num & 0x00ff000000000000ULL) >> 40 ) |
            ( (num & 0x000000000000ff00ULL) << 40 ) |
            ( (num & 0x0000ff0000000000ULL) >> 24 ) |
            ( (num & 0x0000000000ff0000ULL) << 24 ) |
            ( (num & 0x000000ff00000000ULL) >> 8 ) |
            ( (num & 0x00000000ff000000ULL) << 8 )
          );

    return num;
}

u64 vtss_eth_link_oam_swap64_from_bytes(u8 tmp_num[])
{
    u64 num;
    memcpy (&num, tmp_num, 8);
    num = ( ( (num & 0xff00000000000000ULL) >> 56 ) |
            ( (num & 0x00000000000000ffULL) << 56 ) |
            ( (num & 0x00ff000000000000ULL) >> 40 ) |
            ( (num & 0x000000000000ff00ULL) << 40 ) |
            ( (num & 0x0000ff0000000000ULL) >> 24 ) |
            ( (num & 0x0000000000ff0000ULL) << 24 ) |
            ( (num & 0x000000ff00000000ULL) >> 8 ) |
            ( (num & 0x00000000ff000000ULL) << 8 )
          );

    return num;
}

void *vtss_eth_link_oam_malloc(size_t sz)
{
    return VTSS_MALLOC(sz);
}

u32 vtss_is_port_info_get(const u32 port_no, BOOL  *is_up)
{
    port_info_t                         port_info;
    u32                                 rc = VTSS_ETH_LINK_OAM_RC_OK;

    if (is_up == NULL) {
        return VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
    }
    if (port_info_get(port_no, &port_info) == VTSS_OK) {
        *is_up = port_info.link;
    } else {
        rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
    }

    return rc;

}



void vtss_eth_link_oam_trace(const vtss_eth_link_oam_trace_level_t trace_level, const char  *string, const u32   param1, const u32   param2, const u32   param3, const u32   param4)

{
    do {
        switch (trace_level) {
        case VTSS_ETH_LINK_OAM_TRACE_LEVEL_ERROR:
            T_E("%s - port(%u), error(%u), %u, %u", string,
                param1, param2, param3, param4);
            break;
        case VTSS_ETH_LINK_OAM_TRACE_LEVEL_INFO:
            T_I("%s - %u, %u, %u, %u", string,
                param1, param2, param3, param4);
            break;
        case VTSS_ETH_LINK_OAM_TRACE_LEVEL_DEBUG:
            T_D("%s - port(%u), %u, %u, %u", string,
                param1, param2, param3, param4);
            break;
        case VTSS_ETH_LINK_OAM_TRACE_LEVEL_NOISE:
            T_N("%s - %u, %u, %u, %u", string,
                param1, param2, param3, param4);
            break;
        default:
            T_N("Invalid trace level %s - %u, %u, %u, %u", string,
                param1, param2, param3, param4);
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL);

    return;
}


/* Retrieve the OAM Configuration for the port                               */
vtss_rc eth_link_oam_mgmt_port_conf_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_conf_t *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;
    BOOL                     ready;
    if (rc_conv(vtss_eth_link_oam_ready_conf_get(&ready)) == VTSS_RC_OK && ready != FALSE) {
        if (rc_conv(vtss_eth_link_oam_mgmt_port_control_conf_get(
                        l2port, &conf->oam_control)) != VTSS_RC_OK ||
            rc_conv(vtss_eth_link_oam_mgmt_port_mode_conf_get(
                        l2port, &conf->oam_mode)) != VTSS_RC_OK  ||
            rc_conv(vtss_eth_link_oam_mgmt_port_remote_loopback_conf_get(
                        l2port, &conf->oam_remote_loop_back_support)) != VTSS_RC_OK ||
            rc_conv(vtss_eth_link_oam_mgmt_port_link_monitoring_conf_get(
                        l2port, &conf->oam_link_monitoring_support)) != VTSS_RC_OK  ||
            rc_conv(vtss_eth_link_oam_mgmt_port_link_error_frame_window_get(
                        l2port, &conf->oam_error_frame_event_conf.window)) != VTSS_RC_OK  ||
            rc_conv(vtss_eth_link_oam_mgmt_port_link_error_frame_threshold_get(
                        l2port, &conf->oam_error_frame_event_conf.threshold)) != VTSS_RC_OK  ||
            rc_conv(vtss_eth_link_oam_mgmt_port_link_symbol_period_error_window_get(
                        l2port, &conf->oam_symbol_period_event_conf.window)) != VTSS_RC_OK  ||
            rc_conv(vtss_eth_link_oam_mgmt_port_link_symbol_period_error_threshold_get(
                        l2port, &conf->oam_symbol_period_event_conf.threshold)) != VTSS_RC_OK  ||
            rc_conv(vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_get(
                        l2port, &conf->oam_error_frame_secs_summary_event_conf.window)) != VTSS_RC_OK  ||
            rc_conv(vtss_eth_link_oam_mgmt_port_mib_retrival_conf_get(
                        l2port, &conf->oam_mib_retrival_support)) != VTSS_RC_OK ) {

            T_E("Unable to retrive the configuration of port(%d/%u)",
                isid, port_no);
            rc = VTSS_RC_ERROR;
        }
    }
    return rc;
}

i8 *pdu_tx_control_to_str(vtss_eth_link_oam_pdu_control_t tx_control)
{
    switch (tx_control) {
    case VTSS_ETH_LINK_OAM_PDU_CONTROL_RX_INFO:
        return (i8 *)"Receive only";
    case VTSS_ETH_LINK_OAM_PDU_CONTROL_LF_INFO:
        return (i8 *)"Link fault";
    case VTSS_ETH_LINK_OAM_PDU_CONTROL_INFO:
        return (i8 *)"Info exchange";
    case VTSS_ETH_LINK_OAM_PDU_CONTROL_ANY:
        return (i8 *)"Any";
    default:
        return (i8 *)"";
    }
}

i8 *discovery_state_to_str(vtss_eth_link_oam_discovery_state_t discovery_state)
{
    switch (discovery_state) {
    case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_FAULT:
        return (i8 *)"Fault state";
    case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_ACTIVE_SEND_LOCAL:
        return (i8 *)"Active state";
    case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_PASSIVE_WAIT:
        return (i8 *)"Passive state";
    case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE:
        return (i8 *)"SEND_LOCAL_REMOTE_STATE";
    case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_LOCAL_REMOTE_OK:
        return (i8 *)"SEND_LOCAL_REMOTE_OK_STATE";
    case VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY:
        return (i8 *)"SEND_ANY_STATE";
    default:
        return (i8 *)"";
    }
}
i8 *mux_state_to_str(vtss_eth_link_oam_mux_state_t mux_state)
{
    switch (mux_state) {
    case VTSS_ETH_LINK_OAM_MUX_FWD_STATE:
        return (i8 *)"Forwarding ";
    case VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE:
        return (i8 *)"Discarding ";
    default:
        return (i8 *)"";
    }
}

i8 *parser_state_to_str(vtss_eth_link_oam_parser_state_t parse_state)
{
    switch (parse_state) {
    case VTSS_ETH_LINK_OAM_PARSER_FWD_STATE:
        return (i8 *)"Forwarding ";
    case VTSS_ETH_LINK_OAM_PARSER_LB_STATE:
        return (i8 *)"Loop back ";
    case VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE:
        return (i8 *)"Discarding ";
    default:
        return (i8 *)"";
    }
}

void vtss_eth_link_oam_send_response_to_cli(i8 *var_response)
{
    T_D("Unlocking the MIB semaphore");
    vtss_eth_link_oam_mib_retrival_opr_unlock();
}

/******************************************************************************/
/* Link OAM Port's Control status reterival functions                         */
/******************************************************************************/
vtss_rc eth_link_oam_control_layer_port_pdu_control_status_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_pdu_control_t *pdu_control)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);

    rc = vtss_eth_link_oam_mgmt_control_port_pdu_control_status_get(l2port,
                                                                    pdu_control);

    return (rc_conv(rc));
}

vtss_rc eth_link_oam_control_layer_port_discovery_state_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_discovery_state_t *state)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);

    rc = vtss_eth_link_oam_mgmt_control_port_discovery_state_get(l2port, state);

    return (rc_conv(rc));
}

vtss_rc eth_link_oam_control_layer_port_pdu_stats_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_pdu_stats_t *pdu_stats)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);

    rc = vtss_eth_link_oam_mgmt_control_port_pdu_stats_get(l2port, pdu_stats);

    return (rc_conv(rc));
}
vtss_rc eth_link_oam_control_layer_port_critical_event_pdu_stats_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_critical_event_pdu_stats_t *ce_pdu_stats)

{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);

    rc = vtss_eth_link_oam_mgmt_control_port_critical_event_pdu_stats_get(l2port,
                                                                          ce_pdu_stats);

    return (rc_conv(rc));
}
vtss_rc eth_link_oam_control_port_flags_conf_set(const vtss_isid_t isid, const vtss_port_no_t port_no, const u8 flag, const BOOL enable_flag)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);

    rc = vtss_eth_link_oam_mgmt_control_port_flags_conf_set(l2port,
                                                            flag,
                                                            enable_flag);
    if ( (rc == VTSS_ETH_LINK_OAM_RC_OK) &&
         (flag == VTSS_ETH_LINK_OAM_FLAG_LINK_FAULT)) {
        rc = vtss_eth_link_oam_mgmt_port_fault_conf_set(l2port, enable_flag);
    }

    return rc_conv(rc);

}

vtss_rc eth_link_oam_control_port_flags_conf_get(const vtss_isid_t isid, const vtss_port_no_t port_no, u8 *flag)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);

    rc = vtss_eth_link_oam_mgmt_control_port_flags_conf_get(l2port,
                                                            flag);
    return rc_conv(rc);
}

/*Clear the Link OAM statistics*/
vtss_rc eth_link_oam_clear_statistics(l2_port_no_t l2port)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;

    rc = vtss_eth_link_oam_mgmt_control_clear_statistics(l2port);
    return rc_conv(rc);
}

/******************************************************************************/
/* Link OAM Port's Client status reterival functions                          */
/******************************************************************************/

vtss_rc eth_link_oam_client_port_control_conf_get (const vtss_isid_t isid, const vtss_port_no_t port_no, u8 *conf)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_client_port_control_conf_get(
                 l2port, conf));
    return rc;
}

vtss_rc eth_link_oam_client_port_local_info_get (const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_info_tlv_t *local_info)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_client_port_local_info_get(
                 l2port, local_info));
    return rc;
}

vtss_rc eth_link_oam_client_port_remote_info_get (const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_info_tlv_t *remote_info)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_client_port_remote_info_get(
                 l2port, remote_info));
    return rc;
}

vtss_rc eth_link_oam_client_port_remote_seq_num_get (const vtss_isid_t isid, const vtss_port_no_t port_no, u16 *remote_info)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_client_port_remote_seq_num_get(
                 l2port, remote_info));
    return rc;
}

vtss_rc eth_link_oam_client_port_remote_mac_addr_info_get (const vtss_isid_t isid, const vtss_port_no_t port_no, u8 *remote_mac_addr)
{
    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_client_port_remote_mac_addr_info_get(l2port,
                                                                         remote_mac_addr));
    return rc;
}

vtss_rc eth_link_oam_client_port_frame_error_info_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_error_frame_event_tlv_t  *local_error_info, vtss_eth_link_oam_error_frame_event_tlv_t  *remote_error_info)

{

    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_client_port_frame_error_info_get(l2port,
                                                                     local_error_info,
                                                                     remote_error_info));
    return rc;
}

vtss_rc eth_link_oam_client_port_frame_period_error_info_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_error_frame_period_event_tlv_t  *local_info, vtss_eth_link_oam_error_frame_period_event_tlv_t   *remote_info)

{

    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_client_port_frame_period_error_info_get(l2port,
                                                                            local_info,
                                                                            remote_info));
    return rc;
}

vtss_rc eth_link_oam_client_port_symbol_period_error_info_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_error_symbol_period_event_tlv_t  *local_info, vtss_eth_link_oam_error_symbol_period_event_tlv_t   *remote_info)

{

    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_client_port_symbol_period_error_info_get(l2port,
                                                                             local_info,
                                                                             remote_info));
    return rc;
}

vtss_rc eth_link_oam_client_port_error_frame_secs_summary_info_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t   *local_info, vtss_eth_link_oam_error_frame_secs_summary_event_tlv_t  *remote_info)

{

    l2_port_no_t             l2port = L2PORT2PORT(isid, port_no);
    vtss_rc                  rc = VTSS_RC_OK;

    rc = rc_conv(
             vtss_eth_link_oam_mgmt_client_port_error_frame_secs_summary_info_get(
                 l2port,
                 local_info,
                 remote_info));
    return rc;
}



/****************************************************************************/
/*  ETH Link OAM Control layer interface                                    */
/****************************************************************************/
u32 vtss_eth_link_oam_control_port_conf_init(const u32 port_no, const u8 *data)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;
#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_conf_init(port_no, data);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif
    return rc;
}

u32 vtss_eth_link_oam_control_port_oper_init(const u32 port_no, BOOL is_port_active)
{
    u32 rc;
#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_oper_init(port_no, is_port_active);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif
    return rc;
}

u32 vtss_eth_link_oam_control_port_control_conf_set(const u32 port_no, const u8 oam_control)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_control_conf_set(port_no, oam_control);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_control_conf_get(const u32 port_no, u8 *oam_control)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    if (oam_control == NULL) {
        return VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
    }
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_control_conf_get(port_no,
                                                               oam_control);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_flags_conf_set(const u32 port_no, const u8 flag, const BOOL enable_flag)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_flags_conf_set(
             port_no, flag, enable_flag);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_flags_conf_get(const u32 port_no, u8 *flag)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_flags_conf_get(port_no, flag);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}


u32 vtss_eth_link_oam_control_supported_code_conf_set (const u32 port_no, const u8 oam_code, const BOOL support_enable)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_supported_codes_conf_set(
             port_no,
             oam_code,
             support_enable);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_supported_code_conf_get (const u32 port_no, const u8 oam_code, BOOL *support_enable)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    if (support_enable == NULL) {
        return VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
    }
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_supported_codes_conf_get(port_no, oam_code,
                                                                  support_enable);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}


u32 vtss_eth_link_oam_control_port_data_set(const u32 port_no, const u8 *oam_data, BOOL  reset_port_oper, BOOL  is_port_active)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    if (oam_data == NULL) {
        return VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
    }
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_data_set(port_no,
                                                       oam_data, reset_port_oper, is_port_active);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_remote_state_valid_set(const u32 port_no)
{

    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_remote_state_valid_set(port_no);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_local_satisfied_set(const u32 port_no, const BOOL is_local_satisfied)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_local_satisfied_set(port_no,
                                                                  is_local_satisfied);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_remote_stable_set(const u32 port_no, const BOOL is_remote_stable)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_remote_stable_set(port_no,
                                                                is_remote_stable);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_pdu_control_status_get(const u32 port_no, vtss_eth_link_oam_pdu_control_t *pdu_control)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_pdu_control_status_get(port_no,
                                                                     pdu_control);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_local_lost_timer_conf_set(const u32 port_no)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_local_lost_timer_conf_set(port_no);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_discovery_state_get(const u32 port_no, vtss_eth_link_oam_discovery_state_t *state)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_discovery_state_get(
             port_no, state);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_pdu_stats_get(const u32 port_no, vtss_eth_link_oam_pdu_stats_t *pdu_stats)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_pdu_stats_get(port_no,
                                                            pdu_stats);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

u32 vtss_eth_link_oam_control_port_critical_event_pdu_stats_get(const u32 port_no, vtss_eth_link_oam_critical_event_pdu_stats_t *pdu_stats)

{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_critical_event_pdu_stats_get(
             port_no,
             pdu_stats);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}

vtss_rc vtss_eth_link_oam_control_layer_port_mac_conf_get(const u32 port_no, u8 *port_mac_addr)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;
    vtss_isid_t isid;
    vtss_port_no_t switch_port;

    do {
        if (port_mac_addr == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        if (l2port2port(port_no, &isid, &switch_port) == FALSE) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }
        misc_instantiate_mac(port_mac_addr,
                             sys_mac_addr, switch_port + 1 - VTSS_PORT_NO_START);

    } while (VTSS_ETH_LINK_OAM_NULL);

    return rc;

}

vtss_rc vtss_eth_link_oam_control_port_mux_parser_conf_set(const u32 port_no, const vtss_eth_link_oam_mux_state_t  mux_state, const vtss_eth_link_oam_parser_state_t parser_state, u32   *oam_ace_id, u32   *lb_ace_id, const BOOL update_only_mux_state)
{
    u32            rc = VTSS_ETH_LINK_OAM_RC_OK;
#if defined(VTSS_FEATURE_ACL_V2)
    vtss_port_no_t iport;
#endif /* VTSS_FEATURE_ACL_V2 */

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)

    acl_entry_conf_t         lb_ace_conf;
    acl_entry_conf_t         oam_ace_conf;

    u8                       dmac[6] = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x02};
    u8                       dmac_mask[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    u8                       eth_type[2] = {0x88, 0x09};
    u8                       eth_type_mask[2] = {0xff, 0xff};
    u8                       eth_data[2] = {0x03, 0x00};
    u8                       eth_data_mask[2] = {0xff, 0x00};

    do {
        if ( (lb_ace_id == NULL) ||
             (oam_ace_id == NULL) ) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
            break;
        }

        if ((rc = acl_mgmt_ace_init(VTSS_ACE_TYPE_ANY, &lb_ace_conf)) != VTSS_OK) {
            return rc;
        }
        if ((rc = acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &oam_ace_conf)) != VTSS_OK) {
            return rc;
        }
#if defined(VTSS_FEATURE_ACL_V2)
        /* Set the ACE's and ACE action's port list to FALSE */
        for (iport = VTSS_PORT_NO_START; iport < VTSS_FRONT_PORT_COUNT; iport++) {
            lb_ace_conf.port_list[iport] = FALSE;
            lb_ace_conf.action.port_list[iport] = FALSE;
            oam_ace_conf.port_list[iport] = FALSE;
            oam_ace_conf.action.port_list[iport] = FALSE;
        }
#endif /* VTSS_FEATURE_ACL_V2 */


        lb_ace_conf.isid = VTSS_ISID_LOCAL;
        lb_ace_conf.id = ACE_ID_NONE;//MAX;
        lb_ace_conf.action.policer = VTSS_ACL_POLICY_NO_NONE;
#if defined(VTSS_FEATURE_ACL_V2)
        lb_ace_conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
        lb_ace_conf.port_list[port_no] = TRUE;
#else
        lb_ace_conf.action.permit = FALSE;
        lb_ace_conf.action.port_no = VTSS_PORT_NO_NONE;
        lb_ace_conf.port_no = port_no;
#endif /* VTSS_FEATURE_ACL_V2 */

        oam_ace_conf.isid = VTSS_ISID_LOCAL;
        oam_ace_conf.id = ACE_ID_NONE;//MAX;
        oam_ace_conf.action.policer = VTSS_ACL_POLICY_NO_NONE;
#if defined(VTSS_FEATURE_ACL_V2)
        oam_ace_conf.action.port_action = VTSS_ACL_PORT_ACTION_NONE;
        for (iport = VTSS_PORT_NO_START; iport < VTSS_FRONT_PORT_COUNT; iport++) {
            oam_ace_conf.action.port_list[iport] = TRUE;
        }
        oam_ace_conf.port_list[port_no] = TRUE;
#else
        oam_ace_conf.action.permit = TRUE;
        oam_ace_conf.action.port_no = VTSS_PORT_NO_NONE;
        oam_ace_conf.port_no = port_no;
#endif /* VTSS_FEATURE_ACL_V2 */

        memcpy(oam_ace_conf.frame.etype.dmac.value, dmac, 6);
        memcpy(oam_ace_conf.frame.etype.dmac.mask, dmac_mask, 6);
        memcpy(oam_ace_conf.frame.etype.etype.value, eth_type, 2);
        memcpy(oam_ace_conf.frame.etype.etype.mask, eth_type_mask, 2);
        memcpy(oam_ace_conf.frame.etype.data.value, eth_data, 2);
        memcpy(oam_ace_conf.frame.etype.data.mask, eth_data_mask, 2);

        switch (mux_state) {
        case VTSS_ETH_LINK_OAM_MUX_FWD_STATE:
            rc = vtss_port_forward_state_set(NULL, port_no, VTSS_PORT_FORWARD_ENABLED);
            if (rc != VTSS_RC_OK) {
                rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                break;
            }
            break;
        case VTSS_ETH_LINK_OAM_MUX_DISCARD_STATE:
            rc = vtss_port_forward_state_set(NULL, port_no, VTSS_PORT_FORWARD_DISABLED);
            if (rc != VTSS_RC_OK) {
                rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                break;
            }
            break;
        }
        if (update_only_mux_state == TRUE) {
            /* Update only mux state */
            break;
        }
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            break;
        }
        switch (parser_state) {

        case VTSS_ETH_LINK_OAM_PARSER_FWD_STATE:
            rc = acl_mgmt_ace_del(ACL_USER_LINK_OAM, *lb_ace_id);
            if (rc == VTSS_OK) {
                *lb_ace_id = 0;
            } else {
                rc = VTSS_ETH_LINK_OAM_RC_INVALID_PARAMETER;
                break;
            }
            rc = acl_mgmt_ace_del(ACL_USER_LINK_OAM, *oam_ace_id);
            if (rc == VTSS_OK) {
                *oam_ace_id = 0;
            }
            break;

        case VTSS_ETH_LINK_OAM_PARSER_DISCARD_STATE:
            rc = acl_mgmt_ace_add(ACL_USER_LINK_OAM,
                                  ACE_ID_NONE,
                                  &oam_ace_conf);
            if (rc == VTSS_OK) {
                *oam_ace_id = oam_ace_conf.id;
            } else {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_ENABLED;
                break;
            }

            rc = acl_mgmt_ace_add(ACL_USER_LINK_OAM,
                                  ACE_ID_NONE,
                                  &lb_ace_conf);
            if (rc == VTSS_OK) {
                *lb_ace_id = lb_ace_conf.id;
            } else {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_ENABLED;
                break;
            }
            break;

        case VTSS_ETH_LINK_OAM_PARSER_LB_STATE:
            /* Install remote loopback specific rules */
#if defined(VTSS_FEATURE_ACL_V2)
            lb_ace_conf.action.port_action = VTSS_ACL_PORT_ACTION_REDIR;
            lb_ace_conf.action.port_list[port_no] = TRUE;
            lb_ace_conf.port_list[port_no] = TRUE;
#else
            lb_ace_conf.action.port_no = port_no;
#endif /* VTSS_FEATURE_ACL_V2 */

            rc = acl_mgmt_ace_add(ACL_USER_LINK_OAM,
                                  ACE_ID_NONE,
                                  &oam_ace_conf);
            if (rc == VTSS_OK) {
                *oam_ace_id = oam_ace_conf.id;
            } else {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_ENABLED;
                break;
            }

            rc = acl_mgmt_ace_add(ACL_USER_LINK_OAM,
                                  ACE_ID_NONE,
                                  &lb_ace_conf);
            if (rc == VTSS_OK) {
                *lb_ace_id = lb_ace_conf.id;
            } else {
                rc = VTSS_ETH_LINK_OAM_RC_NOT_ENABLED;
                break;
            }
            break;

        default :
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return (rc);
}

vtss_rc vtss_eth_link_oam_control_port_parser_conf_get(const u32 port_no, vtss_eth_link_oam_parser_state_t *parser_state)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_port_parser_conf_get(
             port_no, parser_state);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return (rc);
}

u32 vtss_eth_link_oam_control_port_info_pdu_xmit(const u32 port_no)
{
    u8                          *oam_pdu = NULL;
    vtss_common_bufref_t        bufref = NULL;
    u32                         rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
        if (PORT_NO_IS_STACK(port_no)) {
            break;
        }

        oam_pdu = vtss_os_alloc_xmit(port_no, VTSS_ETH_LINK_OAM_PDU_MIN_LEN, &bufref);
        if (oam_pdu == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
            break; //It's should be fatal
        }
        memset(oam_pdu, '\0', VTSS_ETH_LINK_OAM_PDU_MIN_LEN);
        CRIT_ENTER(oam_control_layer_lock);
        rc = vtss_eth_link_oam_control_layer_fill_info_data(port_no, oam_pdu);
        CRIT_EXIT(oam_control_layer_lock);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            VTSS_FREE(bufref);
            break;
        }
        rc = vtss_os_xmit(port_no, oam_pdu, VTSS_ETH_LINK_OAM_PDU_MIN_LEN, bufref);
        if (rc != VTSS_COMMON_CC_OK) {
            /* LOG the message */
        }
    } while (0);
#endif /* VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL */
    return rc;
}

u32 vtss_eth_link_oam_control_port_non_info_pdu_xmit(const u32 port_no, u8  *oam_client_pdu, const u16 oam_pdu_len, const u8 oam_pdu_code)
{
    u32 rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    u8                       *oam_pdu = NULL;
    vtss_common_bufref_t     bufref = NULL;
#if 0
    u16                      pdu_max_len;
#endif
    vtss_eth_link_oam_discovery_state_t  state;

    CRIT_ENTER(oam_control_layer_lock);
    do {

        rc = vtss_eth_link_oam_control_layer_port_discovery_state_get(port_no,
                                                                      &state);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_D("Error:%u occured while retriving the port(%u) configuration",
                rc, port_no);
            break;
        }
        if (state != VTSS_ETH_LINK_OAM_DISCOVERY_STATE_SEND_ANY) {
            rc = VTSS_ETH_LINK_OAM_RC_INVALID_STATE;
            break;
        }
        oam_pdu = vtss_os_alloc_xmit(port_no, oam_pdu_len, &bufref);
        if (oam_pdu == NULL) {
            rc = VTSS_ETH_LINK_OAM_RC_NO_MEMORY;
            break;
        }
        memcpy(oam_pdu, oam_client_pdu, oam_pdu_len);
        rc = vtss_eth_link_oam_control_layer_fill_header(port_no,
                                                         oam_pdu, oam_pdu_code);
        if (rc == VTSS_ETH_LINK_OAM_RC_OK) {
            rc = vtss_os_xmit(port_no, oam_pdu, oam_pdu_len, bufref);
        } else {
            T_D("Error:%u occured while sending the OAM PDU on port(%u)",
                rc, port_no);
            VTSS_FREE(bufref);
            break;
        }
    } while (VTSS_ETH_LINK_OAM_NULL); /*
                                        Ending of the filling the NON info OAM pdu */
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif
    return rc;
}


/* port_no:         port number                                               */
/* Resets the port counters                                                   */
u32 vtss_eth_link_oam_control_clear_statistics(const u32 port_no)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_clear_statistics(port_no);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return (rc);
}

u32 vtss_eth_link_oam_control_rx_pdu_handler(const u32 port_no, const u8 *pdu, const u16 pdu_len, const u8  oam_code)
{
    u32                      rc = VTSS_ETH_LINK_OAM_RC_OK;
#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    CRIT_ENTER(oam_control_layer_lock);
    rc = vtss_eth_link_oam_control_layer_rx_pdu_handler(port_no,
                                                        pdu,
                                                        pdu_len,
                                                        oam_code);
    CRIT_EXIT(oam_control_layer_lock);
#else
    rc = VTSS_ETH_LINK_OAM_RC_NOT_SUPPORTED;
#endif

    return rc;
}


/****************************************************************************/
/*  ETH Link OAM platform interface                                         */
/****************************************************************************/
void vtss_eth_link_oam_crit_data_lock(void)
{
    CRIT_ENTER(oam_data_lock);
}

void vtss_eth_link_oam_crit_data_unlock(void)
{
    CRIT_EXIT(oam_data_lock);
}

void vtss_eth_link_oam_crit_oper_data_lock(void)
{
    CRIT_ENTER(oam_oper_lock);
}

void vtss_eth_link_oam_crit_oper_data_unlock(void)
{
    CRIT_EXIT(oam_oper_lock);
}

BOOL vtss_eth_link_oam_mib_retrival_opr_lock(void)
{

    BOOL rc = FALSE;
    T_D("Locking the MIB retrival operation semaphore");
    rc = cyg_semaphore_timed_wait(&eth_link_oam_var_sem,
                                  cyg_current_time() + VTSS_OS_MSEC2TICK(VTSS_ETH_LINK_OAM_SLEEP_TIME));

    T_D("Result of the MIB retrival operation is %u", rc);
    return rc;
}

void vtss_eth_link_oam_mib_retrival_opr_unlock(void)
{

    T_D("Unlocking the MIB retrival operation semaphore");
    cyg_semaphore_post(&eth_link_oam_var_sem);
}

BOOL vtss_eth_link_oam_rlb_opr_lock(void)
{

    BOOL rc = FALSE;
    T_D("Locking the remote loopback operation semaphore");
    rc = cyg_semaphore_timed_wait(&eth_link_oam_rlb_sem,
                                  cyg_current_time() +  VTSS_OS_MSEC2TICK(1000));
    T_D("Result of the remote loopback operation is %u", rc);
    return rc;
}

void vtss_eth_link_oam_rlb_opr_unlock(void)
{
    T_D("Unlocking the remote loopback operation semaphore");
    cyg_semaphore_post(&eth_link_oam_rlb_sem);
}


void vtss_eth_link_oam_message_post(vtss_eth_link_oam_message_t *message)
{
    T_D("Post OAM event:%u on port(%u)", message->event_code,
        message->event_on_port);

    if (cyg_mbox_put(eth_link_oam_mbhandle, message) != TRUE) {
        VTSS_FREE(message);
    }
}

static void eth_link_oam_client_thread(cyg_addrword_t data)
{
    BOOL thread_continue = TRUE;
    vtss_eth_link_oam_message_t *event_message;
    u32                         rc = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        event_message = cyg_mbox_get(eth_link_oam_mbhandle);

        T_D("Get OAM event:%u on port(%u)", event_message->event_code,
            event_message->event_on_port);

        vtss_eth_link_oam_crit_oper_data_lock();

        rc = vtss_eth_link_oam_message_handler(event_message);
        if ( (rc != VTSS_ETH_LINK_OAM_RC_OK) &&
             (rc != VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED) ) {
            /* LOG-Message */
            T_D(":%u occured while processing the OAM event:%u on port(%u)",
                rc, event_message->event_code, event_message->event_on_port);
        }
        VTSS_FREE(event_message);
        vtss_eth_link_oam_crit_oper_data_unlock();

    } while (thread_continue);
}
static void eth_link_oam_control_error_frame_event_send(const u32 port_no, BOOL *is_error_frame_xmit_needed, vtss_port_counters_t *port_counters)
{
    BOOL                          is_timer_expired  = FALSE;
    u32                           rc;

    vtss_eth_link_oam_client_link_event_oper_conf_t link_event_oper_conf;

    /* Check if timer has expired */
    memset(&link_event_oper_conf, 0,
           sizeof(vtss_eth_link_oam_client_link_event_oper_conf_t));
    rc = vtss_eth_link_oam_client_is_error_frame_window_expired(
             port_no, &is_timer_expired);
    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
        T_D("Error:%u occured while calculating port(%u) timer values",
            rc, port_no);
        return;
    }
    if (is_timer_expired == TRUE) {
        /* Update the PDU Error Events if Any */
        link_event_oper_conf.ifInErrors_at_timeout =
            (  port_counters->rmon.rx_etherStatsUndersizePkts +
               port_counters->rmon.rx_etherStatsOversizePkts  +
               port_counters->rmon.rx_etherStatsCRCAlignErrors );

        /* update the oper_event_window field from the mgmt
         * structure */
        rc = vtss_eth_link_oam_client_error_frame_oper_conf_update(
                 port_no, &link_event_oper_conf,
                 VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_SET |
                 VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_TIMEOUT_SET);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_D("Error:%u occured while updating port(%u) timer configurations",
                rc, port_no);
            return;
        }
        rc = vtss_eth_link_oam_client_error_frame_event_conf_set (port_no,
                                                                  is_error_frame_xmit_needed);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_D("Error:%u occured while updating port(%u) timer configurations",
                rc, port_no);
            return;
        }
        /* After Sending Update the Errors at Start */
        rc = vtss_eth_link_oam_client_error_frame_oper_conf_update(port_no
                                                                   , &link_event_oper_conf,
                                                                   VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_START_SET);
    } else {

        link_event_oper_conf.oper_event_window = VTSS_ETH_LINK_OAM_SLEEP_TIME_IN_100_MS;
        /* Reduce the oper time period */
        rc = vtss_eth_link_oam_client_error_frame_oper_conf_update(
                 port_no, &link_event_oper_conf,
                 VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_DELETE_OFFSET);
    }
    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
        T_D("Error:%u occured while updating port(%u) timer configurations",
            rc, port_no);
        return;
    }
}

static void eth_link_oam_control_symbol_period_error_event_send(const u32 port_no, BOOL *is_symbol_period_xmit_needed, vtss_port_counters_t *port_counters)
{


    BOOL                          is_timer_expired  = FALSE;
    u32                           rc;
    vtss_eth_link_oam_client_link_error_symbol_period_event_oper_conf_t symbol_period_oper_conf;

    /* Send Symbol Period Error Events Here */
    /* Check if timer has expired */
    memset(&symbol_period_oper_conf, 0,
           sizeof(vtss_eth_link_oam_client_link_error_symbol_period_event_oper_conf_t));
    rc = vtss_eth_link_oam_client_is_symbol_period_frame_window_expired (port_no,
                                                                         &is_timer_expired);
    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {

        T_D("Error:%u occured while calculating port(%u) timer configurations",
            rc, port_no);
        return;
    }
    if ( is_timer_expired == TRUE)  {
        /* Update the PDU Error Events if Any */
#if defined(VTSS_FEATURE_PORT_CNT_ETHER_LIKE)
        symbol_period_oper_conf.symbolErrors_at_timeout =
            port_counters->ethernet_like.dot3StatsSymbolErrors;
#else
        //symbol_period_oper_conf.symbolErrors_at_timeout = VTSS_ETH_LINK_OAM_NULL;
        symbol_period_oper_conf.symbolErrors_at_timeout = port_counters->rmon.rx_etherStatsCRCAlignErrors;
#endif
        /* update the oper_event_window field from the mgmt structure */
        rc = vtss_eth_link_oam_client_symbol_period_error_oper_conf_update(port_no, &symbol_period_oper_conf,
                                                                           VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_SET |
                                                                           VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_TIMEOUT_SET);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_D("Error:%u occured while updating port(%u) timer configurations",
                rc, port_no);
            return;
        }
        rc = vtss_eth_link_oam_client_symbol_period_error_conf_set(port_no, is_symbol_period_xmit_needed);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_D("Error:%u occured while updating port(%u) timer configurations",
                rc, port_no);
            return;
        }
        /* After Sending Update the Errors at Start */
        rc = vtss_eth_link_oam_client_symbol_period_error_oper_conf_update(port_no,
                                                                           &symbol_period_oper_conf,
                                                                           VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_START_SET);
    } else {
        /* Reduce the oper time period */
        symbol_period_oper_conf.oper_event_window     = VTSS_ETH_LINK_OAM_SLEEP_TIME_IN_100_MS;
        rc = vtss_eth_link_oam_client_symbol_period_error_oper_conf_update(port_no, &symbol_period_oper_conf,
                                                                           VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_DELETE_OFFSET);
    }
    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
        T_D("Error:%u occured while updating port(%u) timer configurations",
            rc, port_no);
        return;
    }
}

static void eth_link_oam_control_frame_period_error_event_send(const u32 port_no, BOOL *is_frame_period_xmit_needed, vtss_port_counters_t *port_counters)
{
    BOOL                          is_timer_expired  = FALSE;
    u32                           rc;
    vtss_eth_link_oam_client_link_error_frame_period_event_oper_conf_t frame_period_oper_conf;

    /* Send Frame Period Error Events Here */
    /* Check if timer has expired */
    memset(&frame_period_oper_conf, 0,
           sizeof(vtss_eth_link_oam_client_link_error_frame_period_event_oper_conf_t));

    rc = vtss_eth_link_oam_client_is_frame_period_frame_window_expired(port_no, &is_timer_expired);
    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
        T_D("Error:%u occured while updating port(%u) timer configurations", rc, port_no);
        return;
    }
    if (is_timer_expired == TRUE)  {
        /* Update the PDU Error Events if Any */
        frame_period_oper_conf.frameErrors_at_timeout =
            (  port_counters->rmon.rx_etherStatsUndersizePkts +
               port_counters->rmon.rx_etherStatsOversizePkts  +
               port_counters->rmon.rx_etherStatsCRCAlignErrors );

        /* update the oper_event_window field from the mgmt structure */
        rc = vtss_eth_link_oam_client_frame_period_error_oper_conf_update(
                 port_no, &frame_period_oper_conf,
                 VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_SET |
                 VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_TIMEOUT_SET);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_D("Error:%u occured while updating port(%u) timer configurations",
                rc, port_no);
            return;
        }
        rc = vtss_eth_link_oam_client_frame_period_error_conf_set(port_no,
                                                                  is_frame_period_xmit_needed);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_D("Error:%u occured while updating port(%u) timer configurations",
                rc, port_no);
            return;
        }
        /* After Sending Update the Errors at Start */
        rc = vtss_eth_link_oam_client_frame_period_error_oper_conf_update(port_no, &frame_period_oper_conf,
                                                                          VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_START_SET);
    } else {
        /* Reduce the oper time period */
        frame_period_oper_conf.oper_event_window     = VTSS_ETH_LINK_OAM_SLEEP_TIME_IN_100_MS;
        rc = vtss_eth_link_oam_client_frame_period_error_oper_conf_update(port_no, &frame_period_oper_conf,
                                                                          VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_DELETE_OFFSET);
    }
    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {

        T_I("Error:%u occured while updating port(%u) timer configurations",
            rc, port_no);
        return;
    }
}

static void eth_link_oam_control_error_frame_secs_summary_event_send(const u32 port_no, BOOL *is_error_frame_secs_summary_xmit_needed, vtss_port_counters_t *port_counters)
{
    BOOL                          is_timer_expired  = FALSE;
    u32                           rc;
    vtss_eth_link_oam_client_error_frame_secs_summary_event_oper_conf_t
    link_event_oper_conf;

    /* Check if timer has expired */
    memset(&link_event_oper_conf, 0,
           sizeof(
               vtss_eth_link_oam_client_error_frame_secs_summary_event_oper_conf_t));

    link_event_oper_conf.secErrors_at_timeout = (  port_counters->rmon.rx_etherStatsUndersizePkts +
                                                   port_counters->rmon.rx_etherStatsOversizePkts  +
                                                   port_counters->rmon.rx_etherStatsCRCAlignErrors );

    rc = vtss_eth_link_oam_client_is_error_frame_secs_summary_window_expired(
             port_no, &is_timer_expired);
    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
        T_I("Error:%u occured while updating port(%u) timer configurations",
            rc, port_no);
        return;
    }

    if (is_timer_expired == TRUE) {
        /* update the oper_event_window field from the mgmt
         * structure */
        rc = vtss_eth_link_oam_client_error_frame_secs_summary_oper_conf_update(
                 port_no, &link_event_oper_conf,
                 VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_SET |
                 VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_TIMEOUT_SET);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_D("Error:%u occured while updating port(%u) timer configurations",
                rc, port_no);
            return;
        }
        rc = vtss_eth_link_oam_client_error_frame_secs_summary_event_conf_set (port_no, TRUE,
                                                                               is_error_frame_secs_summary_xmit_needed);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_D("Error:%u occured while updating port(%u) timer configurations",
                rc, port_no);
            return;
        }
        /* After Sending Update the Error Frame Events at Start */
        rc = vtss_eth_link_oam_client_error_frame_secs_summary_oper_conf_update(port_no
                                                                                , &link_event_oper_conf,
                                                                                VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_START_SET);
    } else {

        /* update the oper_event_window field from the mgmt
         * structure */
        rc = vtss_eth_link_oam_client_error_frame_secs_summary_oper_conf_update(
                 port_no, &link_event_oper_conf,
                 VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_TIMEOUT_SET);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            T_D("Error:%u occured while updating port(%u) timer configurations",
                rc, port_no);
            return;
        }
        rc = vtss_eth_link_oam_client_error_frame_secs_summary_event_conf_set (port_no, FALSE,
                                                                               is_error_frame_secs_summary_xmit_needed);
        /* After Sending Update the Error Frame Events at Start */
        rc = vtss_eth_link_oam_client_error_frame_secs_summary_oper_conf_update(port_no
                                                                                , &link_event_oper_conf,
                                                                                VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_ERROR_AT_START_SET);


        link_event_oper_conf.oper_event_window = VTSS_ETH_LINK_OAM_SLEEP_TIME_IN_100_MS;
        /* Reduce the oper time period */
        rc = vtss_eth_link_oam_client_error_frame_secs_summary_oper_conf_update(
                 port_no, &link_event_oper_conf,
                 VTSS_ETH_LINK_OAM_CLIENT_LINK_OPER_CONF_EVENT_WINDOW_DELETE_OFFSET);
    }
    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
        T_I("Error:%u occured while updating port(%u) timer configurations",
            rc, port_no);
        return;
    }
}

void vtss_eth_link_oam_mgmt_sys_reboot_action_handler(void)
{
    u32                         rc;
    u32                         port_no = 0;
    u8                          *oam_pdu = NULL;
    vtss_common_bufref_t        bufref = NULL;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    BOOL                        is_xmit_needed  = FALSE;
#endif

    do {

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)

        for (port_no = 0; port_no < VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT; port_no++) {

            if (PORT_NO_IS_STACK(port_no)) {
                continue;
            }
            vtss_eth_link_oam_crit_oper_data_lock();
            rc = vtss_eth_link_oam_control_layer_port_pdu_cnt_conf_set(port_no);
            rc = vtss_eth_link_oam_control_layer_is_periodic_xmit_needed(port_no, &is_xmit_needed);
            if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                vtss_eth_link_oam_crit_oper_data_unlock();
                continue;
            }
            vtss_eth_link_oam_crit_oper_data_unlock();
            rc = vtss_eth_link_oam_mgmt_control_port_flags_conf_set(port_no,
                                                                    VTSS_ETH_LINK_OAM_FLAG_DYING_GASP, TRUE);
            if (rc == VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED) {
                rc = VTSS_ETH_LINK_OAM_RC_OK;
            }
            if (!is_xmit_needed) {
                continue;
            }

            if (rc == VTSS_ETH_LINK_OAM_RC_OK) {

                oam_pdu = vtss_os_alloc_xmit(port_no, VTSS_ETH_LINK_OAM_PDU_MIN_LEN, &bufref);
                if (oam_pdu == NULL) {
                    continue; //It's should be fatal
                }
                memset(oam_pdu, '\0', VTSS_ETH_LINK_OAM_PDU_MIN_LEN);
                vtss_eth_link_oam_crit_oper_data_lock();
                rc = vtss_eth_link_oam_control_layer_fill_info_data(port_no, oam_pdu);
                vtss_eth_link_oam_crit_oper_data_unlock();
                if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                    VTSS_FREE(bufref);
                    continue;
                }
                rc = vtss_os_xmit(port_no, oam_pdu, VTSS_ETH_LINK_OAM_PDU_MIN_LEN, bufref);
                if (rc != VTSS_COMMON_CC_OK) {
                    /* LOG the message */
                }
            }
        }
    } while (0);
#endif /* end of periodic transmission of OAM info PDUs */
}

static void eth_link_oam_control_timer_thread(cyg_addrword_t data)
{
    BOOL                        thread_continue = TRUE;
    BOOL                        is_update_needed  = FALSE;
    u16                         pdu_max_len = 0;
    u32                         rc;
    u32                         port_no = 0;
    u8                          *oam_pdu = NULL, pkt_buffer_length = 0;
    vtss_common_bufref_t        bufref = NULL;
    BOOL                        is_error_frame_xmit_needed = FALSE;
    BOOL                        is_symbol_period_xmit_needed = FALSE;
    BOOL                        is_frame_period_xmit_needed = FALSE;
    BOOL                        is_error_frame_secs_summary_xmit_needed = FALSE;

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
    BOOL                        is_xmit_needed  = FALSE;
    BOOL                        is_local_lost_link_timer_done = FALSE;
#endif


    vtss_port_counters_t        port_counters;

    do {


        VTSS_OS_MSLEEP(1000);

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)

        for (port_no = 0; port_no < VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT; port_no++) {

            if (PORT_NO_IS_STACK(port_no)) {
                continue;
            }
            vtss_eth_link_oam_crit_oper_data_lock();
            rc = vtss_eth_link_oam_control_layer_port_pdu_cnt_conf_set(port_no);
            rc = vtss_eth_link_oam_control_layer_is_periodic_xmit_needed(port_no, &is_xmit_needed);
            vtss_eth_link_oam_crit_oper_data_unlock();

            if ( (rc == VTSS_ETH_LINK_OAM_RC_OK) &&
                 (is_xmit_needed == TRUE)
               ) {

                oam_pdu = vtss_os_alloc_xmit(port_no, VTSS_ETH_LINK_OAM_PDU_MIN_LEN, &bufref);
                if (oam_pdu == NULL) {
                    continue; //It's should be fatal
                }
                memset(oam_pdu, '\0', VTSS_ETH_LINK_OAM_PDU_MIN_LEN);
                vtss_eth_link_oam_crit_oper_data_lock();
                rc = vtss_eth_link_oam_control_layer_fill_info_data(port_no, oam_pdu);
                vtss_eth_link_oam_crit_oper_data_unlock();
                if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                    VTSS_FREE(bufref);
                    continue;
                }
                rc = vtss_os_xmit(port_no, oam_pdu, VTSS_ETH_LINK_OAM_PDU_MIN_LEN, bufref);
                if (rc != VTSS_COMMON_CC_OK) {
                    /* LOG the message */
                }
                vtss_eth_link_oam_crit_oper_data_lock();
                rc = vtss_eth_link_oam_control_layer_is_local_lost_link_timer_done(port_no,
                                                                                   &is_local_lost_link_timer_done);
                vtss_eth_link_oam_crit_oper_data_unlock();

                if ( (rc == VTSS_ETH_LINK_OAM_RC_OK) &&
                     (is_local_lost_link_timer_done == TRUE)
                   ) {
                    eth_link_oam_port_local_lost_link_timer_done(port_no);
                }
            }
        }
#endif /* end of periodic transmission of OAM info PDUs */


        for (port_no = 0; port_no < VTSS_ETH_LINK_OAM_PHYS_PORTS_CNT; port_no++) {

            vtss_eth_link_oam_crit_oper_data_lock();
            rc = vtss_eth_link_oam_client_is_link_event_stats_update_needed(
                     port_no, &is_update_needed);
            vtss_eth_link_oam_crit_oper_data_unlock();
            if ( (rc == VTSS_ETH_LINK_OAM_RC_OK) &&
                 (is_update_needed == TRUE)
               ) {
                is_error_frame_xmit_needed = is_symbol_period_xmit_needed = is_frame_period_xmit_needed = FALSE;
                /* Read the Statistics and Update */
                /* Since the port thread updates the counters , it is
                 * better to read it using port_stack_couters_get */
#if defined VTSS_SW_OPTION_ETH_LINK_OAM_DEBUG_IPORT
                rc = port_mgmt_counters_get(VTSS_ISID_LOCAL,
                                            VTSS_SW_OPTION_ETH_LINK_OAM_DEBUG_IPORT, &port_counters);
#else
                rc = port_mgmt_counters_get(VTSS_ISID_LOCAL, port_no, &port_counters);
#endif
                if (rc != VTSS_RC_OK) {
                    T_I("Error:%u occured while getting port(%u) counters",
                        rc, port_no);
                    continue; /* It should be fatal */
                }
                vtss_eth_link_oam_crit_oper_data_lock();
                rc = vtss_eth_link_oam_client_pdu_max_len(port_no, &pdu_max_len);

                if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                    T_I("Unable to retrive the PDU max length of port(%u)",
                        port_no);
                    vtss_eth_link_oam_crit_oper_data_unlock();
                    continue;
                }

                (void) eth_link_oam_control_error_frame_event_send(port_no,
                                                                   &is_error_frame_xmit_needed, &port_counters);
                (void) eth_link_oam_control_symbol_period_error_event_send(port_no ,
                                                                           &is_symbol_period_xmit_needed, &port_counters);
                (void) eth_link_oam_control_frame_period_error_event_send(port_no,
                                                                          &is_frame_period_xmit_needed, &port_counters);
                (void) eth_link_oam_control_error_frame_secs_summary_event_send(port_no,
                                                                                &is_error_frame_secs_summary_xmit_needed, &port_counters);
                vtss_eth_link_oam_crit_oper_data_unlock();
                pkt_buffer_length = VTSS_ETH_LINK_OAM_PDU_HDR_LEN +
                                    VTSS_ETH_LINK_OAM_ERROR_FRAME_PERIOD_EVENT_SEQUENCE_NUMBER_LEN;

                if (is_error_frame_xmit_needed == TRUE) {
                    pkt_buffer_length +=
                        VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_EVENT_LEN;
                }
                if (is_symbol_period_xmit_needed == TRUE) {
                    pkt_buffer_length +=
                        VTSS_ETH_LINK_OAM_LINK_MONITORING_SYMBOL_PERIOD_EVENT_LEN;
                }
                if (is_frame_period_xmit_needed == TRUE) {
                    pkt_buffer_length +=
                        VTSS_ETH_LINK_OAM_LINK_MONITORING_FRAME_PERIOD_EVENT_LEN;
                }
                if (is_error_frame_secs_summary_xmit_needed == TRUE) {
                    pkt_buffer_length +=
                        VTSS_ETH_LINK_OAM_LINK_MONITORING_ERROR_FRAME_SECS_SUM_EVENT_LEN;
                }

                if (pkt_buffer_length < VTSS_ETH_LINK_OAM_PDU_MIN_LEN) {
                    pkt_buffer_length = VTSS_ETH_LINK_OAM_PDU_MIN_LEN;
                }

                if (pdu_max_len < pkt_buffer_length) {
                    T_D("Frame length exceeds the max PDU length on port(%u)",
                        port_no);
                    continue;
                }

                if ((is_error_frame_xmit_needed == TRUE)   ||
                    (is_symbol_period_xmit_needed == TRUE) ||
                    (is_frame_period_xmit_needed == TRUE)  ||
                    (is_error_frame_secs_summary_xmit_needed == TRUE)) {
                    /* Construct the PDU */
                    oam_pdu = vtss_os_alloc_xmit(port_no,  pkt_buffer_length, &bufref);
                    if (oam_pdu == NULL) {
                        T_I("Unable to allocate memory to send out frame on port(%u)",
                            port_no);
                        continue; //It's should be fatal
                    }
                    memset(oam_pdu,  0,  pkt_buffer_length);
                    vtss_eth_link_oam_crit_oper_data_lock();
                    rc = vtss_eth_link_oam_control_layer_fill_header(port_no,
                                                                     oam_pdu,
                                                                     VTSS_ETH_LINK_OAM_CODE_TYPE_EVENT);
                    vtss_eth_link_oam_crit_oper_data_unlock();

                    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {

                        if (rc != VTSS_ETH_LINK_OAM_RC_INVALID_STATE) {

                            T_E("Error %u occured while sending out OAM frame on port(%u)",
                                rc,
                                port_no);
                        }
                        VTSS_FREE(bufref);
                        continue;
                    }
                    vtss_eth_link_oam_crit_oper_data_lock();
                    rc = vtss_eth_link_oam_client_link_monitoring_pdu_fill_info_data(
                             port_no, oam_pdu,
                             is_error_frame_xmit_needed, is_symbol_period_xmit_needed,
                             is_frame_period_xmit_needed,
                             is_error_frame_secs_summary_xmit_needed);
                    vtss_eth_link_oam_crit_oper_data_unlock();

                    if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
                        T_E("Error %u occured while sending out OAM frame on port(%u)",
                            rc,
                            port_no);
                        VTSS_FREE(bufref);
                        continue;
                    }

                    /* Send the PDU to the client */
                    rc = vtss_os_xmit(port_no, oam_pdu,  pkt_buffer_length, bufref);
                    if (rc != VTSS_COMMON_CC_OK) {
                        T_E("Error %u occured while sending out OAM frame on port(%u)",
                            rc,
                            port_no);
                        VTSS_FREE(bufref);
                        continue;
                    }
                }
            }
        }

    } while (thread_continue);
}

static void set_conf_to_default(eth_link_oam_conf_t  *blk)
{
    u32  i = 0;
    u32  rc = 0;

    memset(blk, 0, sizeof(eth_link_oam_conf_t));

    for (i = 0; i < VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT; ++i) {
        blk->oam_control[i] = VTSS_ETH_LINK_OAM_DEFAULT_PORT_CONTROL;
        blk->oam_mode[i] = VTSS_ETH_LINK_OAM_DEFAULT_PORT_MODE;
        blk->oam_mib_retrival_support[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_PORT_MIB_RETRIVAL_SUPPORT;
        blk->oam_remote_loop_back_support[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_PORT_REMOTE_LOOPBACK_SUPPORT;
        blk->oam_link_monitoring_support[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_PORT_LINK_MONITORING_SUPPORT;
        blk->oam_error_frame_event_window[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_EVENT_WINDOW_CONF;
        blk->oam_error_frame_event_threshold[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_EVENT_THRESHOLD_CONF;
        blk->oam_symbol_period_error_event_window[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_EVENT_WINDOW_CONF;
        blk->oam_symbol_period_error_event_threshold[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_EVENT_THRESHOLD_CONF;
        blk->oam_symbol_period_error_event_rxthreshold[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_EVENT_RXTHRESHOLD_CONF;
        blk->oam_frame_period_error_event_window[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_EVENT_WINDOW_CONF;
        blk->oam_frame_period_error_event_threshold[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_EVENT_THRESHOLD_CONF;
        blk->oam_frame_period_error_event_rxthreshold[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_EVENT_RXTHRESHOLD_CONF;
        blk->oam_error_frame_secs_summary_event_window[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_EVENT_SECS_SUMMARY_WINDOW_MIN;
        blk->oam_error_frame_secs_summary_event_threshold[i] =
            VTSS_ETH_LINK_OAM_DEFAULT_EVENT_SECS_SUMMARY_THRESHOLD_MIN;

        /* Reset the statistics */
        rc = vtss_eth_link_oam_mgmt_control_clear_statistics(i);
        if (rc != VTSS_ETH_LINK_OAM_RC_OK) {
            continue;
        }
    }
}

static void apply_configuration(eth_link_oam_conf_t  *blk)
{
    u32   i = 0;
    u32  rc = 0;

    for (i = 0; i < VTSS_ETH_LINK_OAM_PORT_CONFIG_CNT; ++i) {
        rc += vtss_eth_link_oam_mgmt_port_conf_init(i);
        rc += vtss_eth_link_oam_mgmt_port_control_conf_set(i, blk->oam_control[i]);
        rc += vtss_eth_link_oam_mgmt_port_mode_conf_set(i, blk->oam_mode[i], TRUE);
        rc += vtss_eth_link_oam_mgmt_port_mib_retrival_conf_set(i,
                                                                blk->oam_mib_retrival_support[i]);
        rc += vtss_eth_link_oam_mgmt_port_remote_loopback_conf_set(i,
                                                                   blk->oam_remote_loop_back_support[i], TRUE);
        rc += vtss_eth_link_oam_mgmt_port_link_monitoring_conf_set(i,
                                                                   blk->oam_link_monitoring_support[i]);
        rc += vtss_eth_link_oam_mgmt_port_link_error_frame_window_set(i,
                                                                      blk->oam_error_frame_event_window[i]);
        rc += vtss_eth_link_oam_mgmt_port_link_error_frame_threshold_set(i,
                                                                         blk->oam_error_frame_event_threshold[i]);
        rc += vtss_eth_link_oam_mgmt_port_link_symbol_period_error_window_set(i,
                                                                              blk->oam_symbol_period_error_event_window[i]);
        rc += vtss_eth_link_oam_mgmt_port_link_symbol_period_error_threshold_set(i,
                                                                                 blk->oam_symbol_period_error_event_threshold[i]);
        rc += vtss_eth_link_oam_mgmt_port_link_symbol_period_error_rxpackets_threshold_set(i,
                blk->oam_symbol_period_error_event_rxthreshold[i]);
        rc += vtss_eth_link_oam_mgmt_port_link_frame_period_error_window_set(i,
                                                                             blk->oam_frame_period_error_event_window[i]);
        rc += vtss_eth_link_oam_mgmt_port_link_frame_period_error_threshold_set(i,
                                                                                blk->oam_frame_period_error_event_threshold[i]);
        rc += vtss_eth_link_oam_mgmt_port_link_frame_period_error_rxpackets_threshold_set(i,
                blk->oam_frame_period_error_event_rxthreshold[i]);
        rc += vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_window_set (i,
                                                                                    blk->oam_error_frame_secs_summary_event_window[i]);
        rc += vtss_eth_link_oam_mgmt_port_link_error_frame_secs_summary_threshold_set (i,
                                                                                       blk->oam_error_frame_secs_summary_event_threshold[i]);
        if (rc) {
            T_D("Error:%u occured during applying the configuration on port(%u)", rc, i);
        }
    }
}

static void eth_link_oam_restore_to_default(void)
{
    eth_link_oam_conf_t *blk = VTSS_MALLOC(sizeof(*blk));
    if (!blk) {
        T_W("Out of memory for Eth Link OAM restore-to-default");
    } else {
        set_conf_to_default(blk);
        apply_configuration(blk);
        VTSS_FREE(blk);
    }
}

static void eth_link_oam_port_change_callback(vtss_port_no_t port_no, port_info_t *info)
{
    vtss_eth_link_oam_message_t       *message = NULL;
    T_D("Port %u Link %u\n", port_no, info->link);

    message = VTSS_MALLOC(sizeof(vtss_eth_link_oam_message_t));
    if (message != NULL) {
        if (info->link) {
            message->event_code = VTSS_ETH_LINK_OAM_PORT_UP_EVENT;
        } else {
            message->event_code = VTSS_ETH_LINK_OAM_PORT_DOWN_EVENT;
        }
        message->event_on_port = port_no;
        vtss_eth_link_oam_message_post(message);
    } else {
        T_E("Unable to allocate the memory to handle the received frame on port(%u)",
            port_no);
    }
    return;
}

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
static void eth_link_oam_port_local_lost_link_timer_done(vtss_port_no_t port_no)
{
    vtss_eth_link_oam_message_t       *message = NULL;

    message = VTSS_MALLOC(sizeof(vtss_eth_link_oam_message_t));
    if (message != NULL) {
        message->event_code = VTSS_ETH_LINK_OAM_PDU_LOCAL_LOST_LINK_TIMER_EVENT;
        message->event_on_port = port_no;
        vtss_eth_link_oam_message_post(message);
    } else {
        T_E("Unable to allocate the memory to handle the port(%u) timer expiry",
            port_no);
    }
    return;
}
#endif


/* ETH Link OAM frame handler */
static BOOL rx_eth_link_oam(void *contxt, const uchar *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    BOOL rc = FALSE; // Allow other subscribers to receive the packet
    vtss_eth_link_oam_message_t        *message = NULL;
    u8                                 conf;
    u32   rc1 = VTSS_ETH_LINK_OAM_RC_OK;

    do {
        if (frm[14] != VTSS_ETH_LINK_OAM_SUB_TYPE) {  //Verify that PDU is for Link OAM
            T_I("Other slow protocol PDU is received on port(%u)", rx_info->port_no);
            break;
        }
        if (vtss_eth_link_oam_mgmt_client_port_control_conf_get(rx_info->port_no, &conf) ==
            VTSS_ETH_LINK_OAM_RC_OK) {
            if (conf == FALSE) {
                //OAM operation is disabled on this port. Ignore the PDUs.
                T_I("OAM PDU is received on OAM disabled port(%u)", rx_info->port_no);
                break;
            }
        } else {
            T_E("Error occured while getting the port(%u) control configuration",
                rx_info->port_no);
            break;
        }
        if (frm[14] == VTSS_ETH_LINK_OAM_SUB_TYPE) {
            if (rx_info->tag_type != VTSS_TAG_TYPE_UNTAGGED) {
                //Verify that PDU is not tagged
                T_D("Tagged OAM PDU is received on port(%u)", rx_info->port_no);
                break;
            }
        }
        /* Needs to consider the untagged frames */
        if ( (rx_info->length < (VTSS_ETH_LINK_OAM_PDU_MIN_LEN - 4)) ||
             (rx_info->length > (VTSS_ETH_LINK_OAM_PDU_MAX_LEN + VTSS_ETH_LINK_OAM_PDU_HDR_LEN))
           ) {
            //Verify that PDU came with sufficient length
            T_E("Invalid length:(%u) OAM frame recived on port(%u)", rx_info->length,
                rx_info->port_no);
            break;
        }
        rc1 = vtss_eth_link_oam_mgmt_control_rx_pdu_handler(rx_info->port_no, frm, rx_info->length, frm[17]);
        if ( (rc1 == VTSS_ETH_LINK_OAM_RC_OK) ||
             (rc1 == VTSS_ETH_LINK_OAM_RC_ALREADY_CONFIGURED)) {
            message = VTSS_MALLOC(sizeof(vtss_eth_link_oam_message_t));
            if (message != NULL) {
                message->event_code      = VTSS_ETH_LINK_OAM_PDU_RX_EVENT;
                message->event_on_port   = rx_info->port_no;
                message->event_data_len  = rx_info->length;
                memcpy(message->event_data, frm, rx_info->length);
                vtss_eth_link_oam_message_post(message);
                break;
            } else {
                T_E("Unable to allocate memory to handle the received frame on port(%u)", rx_info->port_no);
                break;
            }
        }
    } while (VTSS_ETH_LINK_OAM_NULL);

    return rc;
}


/* Initialize module */
vtss_rc eth_link_oam_init(vtss_init_data_t *data)
{
    vtss_rc                vtssrc = VTSS_OK;
    packet_rx_filter_t     rx_filter;
    vtss_isid_t            isid = data->isid;
    eth_link_oam_conf_t    *blk = NULL;
    u32                    size = 0;

    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        T_D("INIT");
        //Initialize the OAM Module elements
        critd_init(&oam_data_lock,
                   "LINK_OAM",
                   VTSS_MODULE_ID_ETH_LINK_OAM,
                   VTSS_TRACE_MODULE_ID,
                   CRITD_TYPE_MUTEX);

        critd_init(&oam_oper_lock,
                   "LINK_OAM",
                   VTSS_MODULE_ID_ETH_LINK_OAM,
                   VTSS_TRACE_MODULE_ID,
                   CRITD_TYPE_MUTEX);

        critd_init(&oam_control_layer_lock,
                   "LINK_OAM",
                   VTSS_MODULE_ID_ETH_LINK_OAM,
                   VTSS_TRACE_MODULE_ID,
                   CRITD_TYPE_MUTEX);

        cyg_semaphore_init(&eth_link_oam_var_sem, 0);
        cyg_semaphore_init(&eth_link_oam_rlb_sem, 0);

        cyg_mbox_create(&eth_link_oam_mbhandle, &eth_link_oam_mbox);

        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          eth_link_oam_control_timer_thread,
                          0,
                          "ETH LINK OAM TIMER",
                          eth_link_oam_control_timer_thread_stack,
                          sizeof(eth_link_oam_control_timer_thread_stack),
                          &eth_link_oam_control_timer_thread_handle,
                          &eth_link_oam_control_timer_thread_block);

        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          eth_link_oam_client_thread,
                          0,
                          "ETH LINK OAM CLIENT",
                          eth_link_oam_client_thread_stack,
                          sizeof(eth_link_oam_client_thread_stack),
                          &eth_link_oam_client_thread_handle,
                          &eth_link_oam_client_thread_block);

        CRIT_EXIT(oam_data_lock);
        CRIT_EXIT(oam_oper_lock);
        CRIT_EXIT(oam_control_layer_lock);

#if defined(VTSS_SW_OPTION_ETH_LINK_OAM_CONTROL)
        vtss_eth_link_oam_control_init();
#endif
#ifdef VTSS_SW_OPTION_VCLI
        (void) eth_link_oam_cli_req_init();
#endif /* VTSS_SW_OPTION_VCLI */
#ifdef VTSS_SW_OPTION_ICFG
        vtssrc = eth_link_oam_icfg_init();
        if (vtssrc != VTSS_OK) {
            T_D("fail to init link oam icfg registration, rc = %s", error_txt(vtssrc));
        }
#endif
        vtssrc = rc_conv(vtss_eth_link_oam_default_set());
        break;
    case INIT_CMD_START:
        T_D("START");
        //Start OAM module threads
        cyg_thread_resume(eth_link_oam_control_timer_thread_handle);
        cyg_thread_resume(eth_link_oam_client_thread_handle);
        vtssrc = vtss_eth_link_oam_ready_conf_set(TRUE);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF");

        //Construct the OAM configurations with factory defaults
        if (isid == VTSS_ISID_LOCAL) {
            if (isid == VTSS_ISID_LOCAL) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
                blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size);
                set_conf_to_default(blk);
                apply_configuration(blk);
                conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
#else
                eth_link_oam_restore_to_default();
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
            }
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");
        //Get the system MAC address
        (void)conf_mgmt_mac_addr_get(sys_mac_addr, 0);

        if (misc_conf_read_use()) {
            //Restore the saved configuration
            if ( (blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE, &size)) == NULL || (size != sizeof(*blk)) ) {
                T_W("conf_sec_open failed or size mismatch, creating defaults");
                if ((blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE, sizeof(*blk))) != NULL) {
                    blk->version = 0;
                    set_conf_to_default(blk);
                } else {
                    T_W("conf_sec_create failed");
                    break;
                }
            }
            apply_configuration(blk);
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_ETH_LINK_OAM_CONF_TABLE);
        } else {
            eth_link_oam_restore_to_default();
        }
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD");
        // Registration for OAM frames
        memset(&rx_filter, 0, sizeof(rx_filter));
        rx_filter.modid                 = VTSS_MODULE_ID_ETH_LINK_OAM;
        rx_filter.match                 = PACKET_RX_FILTER_MATCH_DMAC | PACKET_RX_FILTER_MATCH_ETYPE;
        rx_filter.cb                    = rx_eth_link_oam;
        rx_filter.prio                  = PACKET_RX_FILTER_PRIO_NORMAL;
        memcpy(rx_filter.dmac, ETH_LINK_OAM_MULTICAST_MACADDR, sizeof(rx_filter.dmac));
        rx_filter.etype                 = VTSS_ETH_LINK_OAM_ETH_TYPE; // 0x8809

        vtssrc = packet_rx_filter_register(&rx_filter, &eth_link_oam_filter_id);

        if (vtssrc != VTSS_OK) {
            T_E("Unable to register with Packet module");
            // Can we break here?
            break;
        }
        vtssrc = port_change_register(VTSS_MODULE_ID_ETH_LINK_OAM, eth_link_oam_port_change_callback);

        if (vtssrc != VTSS_OK) {
            T_E("Unable to register with Port module");
            // Can we break here?
            break;
        }

        break;
    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL");
        break;
    case INIT_CMD_SUSPEND_RESUME:
        T_D("SUSPEND_RESUME");
        break;
    default:
        break;
    }
    return vtssrc;
}

vtss_rc eth_link_oam_port_loopback_oper_status_get(const vtss_isid_t isid, const vtss_port_no_t port_no, vtss_eth_link_oam_loopback_status_t *loopback_status)
{
    l2_port_no_t l2port = L2PORT2PORT (isid, port_no);
    vtss_rc rc = VTSS_RC_OK;

    rc =
        vtss_eth_link_oam_mgmt_loopback_oper_status_get (l2port, loopback_status);

    return (rc_conv (rc));
}
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/




