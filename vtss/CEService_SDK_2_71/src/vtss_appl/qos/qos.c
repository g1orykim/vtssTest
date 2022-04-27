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
#include "misc_api.h"
#include "msg_api.h"
#include "port_api.h"
#include "critd_api.h"
#include "qos_api.h"
#include "qos.h"

#ifdef VTSS_SW_OPTION_ICFG
#include "qos_icfg.h"
#endif

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
#include "ce_max_api.h"
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_QOS

/* Global structure */
static qos_global_t qos;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "QOS",
    .descr     = "QoS Control Lists"
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
#define QOS_CRIT_ENTER()         critd_enter(        &qos.qos_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define QOS_CRIT_EXIT()          critd_exit(         &qos.qos_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define QOS_CRIT_ASSERT_LOCKED() critd_assert_locked(&qos.qos_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#define QOS_CB_CRIT_ENTER()      critd_enter(        &qos.rt.crit,  TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define QOS_CB_CRIT_EXIT()       critd_exit(         &qos.rt.crit,  TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define QOS_CRIT_ENTER()         critd_enter(        &qos.qos_crit)
#define QOS_CRIT_EXIT()          critd_exit(         &qos.qos_crit)
#define QOS_CRIT_ASSERT_LOCKED() critd_assert_locked(&qos.qos_crit)
#define QOS_CB_CRIT_ENTER()      critd_enter(        &qos.rt.crit)
#define QOS_CB_CRIT_EXIT()       critd_exit(         &qos.rt.crit)
#endif /* VTSS_TRACE_ENABLED */

#define QOS_QCL_PORT_MAX 12 /* Number of QCLs per port */

#if defined(VTSS_FEATURE_QCL_V2)

const char *const qcl_user_names[QCL_USER_CNT] = {
    [QCL_USER_STATIC] = "Static",
#ifdef VTSS_SW_OPTION_VOICE_VLAN
    [QCL_USER_VOICE_VLAN] = "Voice VLAN",
#endif
};

static vtss_rc qcl_list_qce_get(vtss_isid_t isid,
                                vtss_qcl_id_t qcl_id,
                                vtss_qce_id_t id,
                                qos_qce_entry_conf_t *conf,
                                BOOL next);

#endif

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
static vtss_rc qos_lport_conf_set(vtss_lport_no_t lport_no, const qos_lport_conf_t *const conf);
static void qos_lport_build_hw_conf(vtss_qos_lport_conf_t *const hw_conf, const qos_lport_conf_t *const conf);
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */
/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2) || defined(VTSS_FEATURE_QOS_TAG_REMARK_V2)

/* Default mapping of PCP to QoS Class */
/* Can also be used for default mapping of QoS class to PCP */
/* This is the IEEE802.1Q-2011 recommended priority to traffic class mappings */
static u32 qos_pcp2qos(u32 pcp)
{
    switch (pcp) {
    case 0:
        return 1;
    case 1:
        return 0;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        return pcp;
    default:
        T_E("Invalid PCP (%u)", pcp);
        return 0;
    }
}
#endif /* defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2) || defined(VTSS_FEATURE_QOS_TAG_REMARK_V2) */

/* Call registered QoS port configuration change callbacks */
static void qos_port_conf_change_event(vtss_isid_t isid, vtss_port_no_t iport, const qos_port_conf_t *const conf)
{
    int                        i;
    qos_port_conf_change_reg_t *reg;
    cyg_tick_count_t           ticks;

    QOS_CB_CRIT_ENTER();
    for (i = 0; i < qos.rt.count; i++) {
        reg = &qos.rt.reg[i];
        if (reg->global == (isid != VTSS_ISID_LOCAL)) {
            T_D("callback, i: %d (%s), isid: %d, iport: %u", i, vtss_module_names[reg->module_id], isid, iport);
            ticks = cyg_current_time();
            reg->callback(isid, iport, conf);
            ticks = (cyg_current_time() - ticks);
            if (ticks > reg->max_ticks) {
                reg->max_ticks = ticks;
            }
            T_D("callback done, i: %d (%s), isid: %d, iport: %u, %llu tics, %llu msec", i, vtss_module_names[reg->module_id], isid, iport, ticks, VTSS_OS_TICK2MSEC(ticks));
        }
    }
    QOS_CB_CRIT_EXIT();
}

/* Set global QOS configuration to defaults */
static void qos_conf_default_set(qos_conf_t *conf)
{
    memset(conf, 0, sizeof(*conf));
    conf->prio_no = QOS_CLASS_CNT;

#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH)
    conf->policer_mc = 1;
    conf->policer_mc_status = RATE_STATUS_DISABLE;
#endif /* defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH)
    conf->policer_bc = 1;
    conf->policer_bc_status = RATE_STATUS_DISABLE;
#endif /* defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) */
#if defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH)
    conf->policer_uc = 1;
    conf->policer_uc_status = RATE_STATUS_DISABLE;
#endif /* defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) */
#if defined(VTSS_FEATURE_QOS_WRED)
    {
        int i;
        for (i = 0; i < QOS_PORT_WEIGHTED_QUEUE_CNT; i++) {
            conf->wred[i].enable = FALSE;
            conf->wred[i].min_th = 0;
            conf->wred[i].max_prob_1 = 1;
            conf->wred[i].max_prob_2 = 5;
            conf->wred[i].max_prob_3 = 10;
        }
    }
#elif defined(VTSS_FEATURE_QOS_WRED_V2)
    {
        int queue, dpl;
        for (queue = 0; queue < QOS_PORT_QUEUE_CNT; queue++) {
            for (dpl = 0; dpl < 2; dpl++) {
                conf->wred[queue][dpl].enable   = FALSE;
                conf->wred[queue][dpl].min_fl   = 0;
                conf->wred[queue][dpl].max      = 50;
                conf->wred[queue][dpl].max_unit = VTSS_WRED_V2_MAX_DP; /* Defaults to 50% drop probability at 100% fill level */
            }
        }
    }
#endif /* VTSS_FEATURE_QOS_WRED_V2 */
#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2) || defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
    {
        int i;
        for (i = 0; i < 64; i++) {
#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
            conf->dscp.dscp_trust[i] = FALSE;
            conf->dscp.dscp_qos_class_map[i] = 0;
            conf->dscp.dscp_dp_level_map[i] = 0;
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
            conf->dscp.translate_map[i] = i;
            conf->dscp.ingress_remark[i] = FALSE;
            conf->dscp.egress_remap[i] = i;
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
            conf->dscp.egress_remap_dp1[i] = i;
#endif
        }
        for (i = 0; i < VTSS_PRIO_ARRAY_SIZE; i++) {
            conf->dscp.qos_class_dscp_map[i] = 0;
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
            conf->dscp.qos_class_dscp_map_dp1[i] = 0;
#endif
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
        }
    }
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2) || defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2) */
#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
    conf->header_size = 0x12;
#endif /* VTSS_FEATURE_QOS_RATE_LIMITER */
#ifdef VTSS_FEATURE_VSTAX_V2
    conf->cmef_disable = FALSE; /* Default enabled */
#endif /* VTSS_FEATURE_VSTAX_V2 */
}

/* Check global QOS configuration */
static vtss_rc qos_conf_check(qos_conf_t *conf)
{
    if (conf->prio_no > QOS_CLASS_CNT) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }

#if defined(VTSS_FEATURE_QOS_WRED)
    {
        int i;
        for (i = 0; i < QOS_PORT_WEIGHTED_QUEUE_CNT; i++) {
            if (conf->wred[i].min_th > 100) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
            if (conf->wred[i].max_prob_1 > 100) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
            if (conf->wred[i].max_prob_2 > 100) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
            if (conf->wred[i].max_prob_3 > 100) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
        }
    }
#elif defined(VTSS_FEATURE_QOS_WRED_V2)
    {
        int queue, dpl;

        for (queue = 0; queue < QOS_PORT_QUEUE_CNT; queue++) {
            for (dpl = 0; dpl < 2; dpl++) {
                vtss_red_v2_t *red = &conf->wred[queue][dpl];
                if (red->min_fl > 100) {
                    T_W("Invalid min_fl (%d)", red->min_fl);
                    return QOS_ERROR_PARM;
                }
                if ((red->max < 1) || (red->max > 100)) {
                    T_W("Invalid max (%d) on queue %d, dpl %d", red->max, queue, dpl);
                    return QOS_ERROR_PARM;
                }
                if ((red->max < 1) || (red->max > 100)) {
                    T_W("Invalid max (%d) on queue %d, dpl %d", red->max, queue, dpl);
                    return QOS_ERROR_PARM;
                }
                if ((red->max_unit != VTSS_WRED_V2_MAX_DP) && (red->max_unit != VTSS_WRED_V2_MAX_FL)) {
                    T_W("Invalid max_unit (%d) on queue %d, dpl %d", red->max_unit, queue, dpl);
                    return QOS_ERROR_PARM;
                }
                if ((red->max_unit == VTSS_WRED_V2_MAX_FL) && (red->min_fl >= red->max)) {
                    T_W("min_fl (%u) >= max fl (%u) on queue %d, dpl %d", red->min_fl, red->max, queue, dpl);
                    return QOS_ERROR_PARM;
                }
            }
        }
    }
#endif /* VTSS_FEATURE_QOS_WRED_V2 */

#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2) || defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
    {
        int i;
        for (i = 0; i < 64; i++) {
#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
            if (conf->dscp.dscp_qos_class_map[i] > QOS_CLASS_CNT) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
            if (conf->dscp.dscp_dp_level_map[i] > QOS_DPL_MAX) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */

#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
            if (conf->dscp.translate_map[i] > 63) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
            if (conf->dscp.egress_remap[i] > 63) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
            if (conf->dscp.egress_remap_dp1[i] > 63) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
#endif
        }
        for (i = 0; i < VTSS_PRIO_ARRAY_SIZE; i++) {
            if (conf->dscp.qos_class_dscp_map[i] > 63) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
            if (conf->dscp.qos_class_dscp_map_dp1[i] > 63) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
#endif
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
        }
    }
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2) || defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2) */

#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
    if ((conf->header_size < 1) || (conf->header_size > 31)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
#endif /* VTSS_FEATURE_QOS_RATE_LIMITER */
    return VTSS_OK;
}

/* Apply global QOS configuration to local switch */
static void qos_conf_apply(qos_conf_t *conf)
{
    vtss_qos_conf_t   qos_ll_conf;
    vtss_rc           rc;
#ifdef VTSS_FEATURE_VSTAX_V2
    vtss_vstax_conf_t vstax_conf;

    vtss_appl_api_lock();
    if ((rc = vtss_vstax_conf_get(NULL, &vstax_conf)) != VTSS_OK) {
        T_E("Error getting VStaX configuration - %s\n", error_txt(rc));
    } else {
        vstax_conf.cmef_disable = conf->cmef_disable;
        if ((rc = vtss_vstax_conf_set(NULL, &vstax_conf)) != VTSS_OK) {
            T_E("Error setting VStaX configuration - %s\n", error_txt(rc));
        }
    }
    vtss_appl_api_unlock();
#endif /* VTSS_FEATURE_VSTAX_V2 */

    if ((rc = vtss_qos_conf_get(NULL, &qos_ll_conf)) != VTSS_OK) {
        T_E("Error getting QoS configuration - %s\n", error_txt(rc));
    } else {
        qos_ll_conf.prios = conf->prio_no;

#if defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH)
        if (conf->policer_mc_status != RATE_STATUS_DISABLE) {
            qos_ll_conf.policer_mc = conf->policer_mc;
        } else {
            qos_ll_conf.policer_mc = VTSS_PACKET_RATE_DISABLED;
        }
#endif /* defined(VTSS_FEATURE_QOS_POLICER_MC_SWITCH) */

#if defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH)
        if (conf->policer_bc_status != RATE_STATUS_DISABLE) {
            qos_ll_conf.policer_bc = conf->policer_bc;
        } else {
            qos_ll_conf.policer_bc = VTSS_PACKET_RATE_DISABLED;
        }
#endif /* defined(VTSS_FEATURE_QOS_POLICER_BC_SWITCH) */

#if defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH)
        if (conf->policer_uc_status != RATE_STATUS_DISABLE) {
            qos_ll_conf.policer_uc = conf->policer_uc;
        } else {
            qos_ll_conf.policer_uc = VTSS_PACKET_RATE_DISABLED;
        }
#endif /* defined(VTSS_FEATURE_QOS_POLICER_UC_SWITCH) */

#if defined(VTSS_FEATURE_QOS_CLASSIFICATION_V2)
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
        {
            int i;
            for (i = 0; i < 64; i++) {
                qos_ll_conf.dscp_trust[i] = conf->dscp.dscp_trust[i];
                qos_ll_conf.dscp_qos_class_map[i] = conf->dscp.dscp_qos_class_map[i];
                qos_ll_conf.dscp_dp_level_map[i] = conf->dscp.dscp_dp_level_map[i];
            }
        }
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */

#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
        {
            int i;
            for (i = 0; i < 64; i++) {
                qos_ll_conf.dscp_remark[i] = conf->dscp.ingress_remark[i];
                qos_ll_conf.dscp_translate_map[i] = conf->dscp.translate_map[i];
                qos_ll_conf.dscp_remap[i] = conf->dscp.egress_remap[i];
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
                qos_ll_conf.dscp_remap_dp1[i] = conf->dscp.egress_remap_dp1[i];
#endif
            }
            for (i = 0; i < VTSS_PRIO_ARRAY_SIZE; i++) {
                qos_ll_conf.dscp_qos_map[i] = conf->dscp.qos_class_dscp_map[i];
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
                qos_ll_conf.dscp_qos_map_dp1[i] = conf->dscp.qos_class_dscp_map_dp1[i];
#endif
            }
        }
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */
#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
        qos_ll_conf.header_size = conf->header_size;
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */

#if defined(VTSS_FEATURE_QOS_WRED_V2)
        {
            int queue, dpl;
            for (queue = 0; queue < QOS_PORT_QUEUE_CNT; queue++) {
                for (dpl = 0; dpl < 2; dpl++) {
                    qos_ll_conf.red_v2[queue][dpl] = conf->wred[queue][dpl];
                }
            }
        }
#endif /* VTSS_FEATURE_QOS_WRED_V2 */

        /* apply new data to driver layer */
        if ((rc = vtss_qos_conf_set(NULL, &qos_ll_conf)) != VTSS_OK) {
            T_E("Error setting QoS configuration - %s\n", error_txt(rc));
        }
    }
#if defined(VTSS_FEATURE_QOS_WRED)
    {
        vtss_qos_port_conf_t qos_port_ll_conf;
        port_iter_t          pit;

        /*
         * In the application this is a global configuration but in the API it is per port.
         * Apply the global configuration to all ports here
         */
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            if ((rc = vtss_qos_port_conf_get(NULL, pit.iport, &qos_port_ll_conf)) != VTSS_OK) {
                T_E("Error getting QoS port configuration - %s\n", error_txt(rc));
            } else {
                int i;
                for (i = 0; i < QOS_PORT_WEIGHTED_QUEUE_CNT; i++) {
                    qos_port_ll_conf.red[i].enable     = conf->wred[i].enable;
                    qos_port_ll_conf.red[i].min_th     = conf->wred[i].min_th;
                    qos_port_ll_conf.red[i].max_prob_1 = conf->wred[i].max_prob_1;
                    qos_port_ll_conf.red[i].max_prob_2 = conf->wred[i].max_prob_2;
                    qos_port_ll_conf.red[i].max_prob_3 = conf->wred[i].max_prob_3;
                }
                if ((rc = vtss_qos_port_conf_set(NULL, pit.iport, &qos_port_ll_conf)) != VTSS_OK) {
                    T_E("Error setting QoS port configuration - %s\n", error_txt(rc));
                }
            }
        }
    }
#endif /* VTSS_FEATURE_QOS_WRED */
}

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
static void qos_lport_conf_default_set(qos_lport_conf_blk_t *blk)
{
    u8 tmp_lport;
    u8 tmp_queue;

    /* By default, we fill maximum configuration */
    /* At present, UI(CLI) takes care of all the logic associated with the host mode */
    for (tmp_lport = 0; tmp_lport < VTSS_LPORTS; tmp_lport++) {
        blk->conf[tmp_lport].shaper_host_status = FALSE;
        blk->conf[tmp_lport].pct = 17;
        /* Initialize the per queue WRED values */
        for (tmp_queue = 0; tmp_queue < VTSS_PRIOS; tmp_queue++) {
            blk->conf[tmp_lport].red[tmp_queue].enable = FALSE;
            blk->conf[tmp_lport].red[tmp_queue].max_th = 100;
            blk->conf[tmp_lport].red[tmp_queue].min_th = 0;
            blk->conf[tmp_lport].red[tmp_queue].max_prob_1 = 1;
            blk->conf[tmp_lport].red[tmp_queue].max_prob_2 = 5;
            blk->conf[tmp_lport].red[tmp_queue].max_prob_3 = 10;
        }
        /* disable the lport shapers by default */
        for (tmp_queue = 0; tmp_queue < 2; tmp_queue++) {
            /*disable the host queue shapers by default */
            blk->conf[tmp_lport].scheduler.shaper_queue_status[tmp_queue] = FALSE;
            blk->conf[tmp_lport].scheduler.queue[tmp_queue].rate = 100;
        }
        blk->conf[tmp_lport].shaper.rate = 100; /*disable the lport shapers by default */
        for (tmp_queue = 0; tmp_queue < 6; tmp_queue++) {
            /* Set the host queue weights */
            blk->conf[tmp_lport].scheduler.queue_pct[tmp_queue] = 17;
        }
    }
    blk->mode_change = FALSE; /* Set mode change to default */
}
static void qos_lport_conf_apply(qos_lport_conf_blk_t *blk)
{
    u8                    tmp_lport;

    if (blk->mode_change == TRUE) { /* Host mode change happened. Apply only defaults */
        qos_lport_conf_default_set(blk);
    }
    for (tmp_lport = 0; tmp_lport < VTSS_LPORTS; tmp_lport++) {
        /* Apply the logical configuration */
        (void)qos_lport_conf_set(tmp_lport, &blk->conf[tmp_lport]);
    }
}
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

/* Set QOS port configuration to defaults */
static void qos_port_conf_default_set(qos_port_conf_t *conf)
{
    int i;

    memset(conf, 0, sizeof(*conf));

    conf->default_prio = 0;                         /* assign default queue to low */
    conf->usr_prio = 0;

    for (i = 0; i < VTSS_PORT_POLICERS; i++) {
        conf->port_policer[i].enabled = FALSE;
        conf->port_policer[i].policer.rate = QOS_BITRATE_DEF;

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT)

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS)
        conf->port_policer_ext[i].frame_rate           = FALSE;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FPS */

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL)
        conf->port_policer_ext[i].dp_bypass_level      = 0;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL */

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM)
#if defined(VTSS_SW_OPTION_JR_STORM_POLICERS)
        if (i == QOS_STORM_POLICER_UNICAST) {             /* Modify this policer to police unicast only and limit CPU traffic */
            conf->port_policer_ext[i].unicast              = TRUE;
            conf->port_policer_ext[i].multicast            = FALSE;
            conf->port_policer_ext[i].broadcast            = FALSE;
            conf->port_policer_ext[i].uc_no_flood          = TRUE; /* Exclude flooded unicast frames */
            conf->port_policer_ext[i].mc_no_flood          = FALSE;
            conf->port_policer_ext[i].flooded              = FALSE;
            conf->port_policer_ext[i].learning             = FALSE;
            conf->port_policer_ext[i].to_cpu               = FALSE;
            {
                int q;
                for (q = 0; q < VTSS_PORT_POLICER_CPU_QUEUES; q++) {
                    conf->port_policer_ext[i].cpu_queue[q]     = FALSE;
                }
            }
            conf->port_policer_ext[i].limit_noncpu_traffic = TRUE;
            conf->port_policer_ext[i].limit_cpu_traffic    = TRUE;
        } else if (i == QOS_STORM_POLICER_BROADCAST) {           /* Modify this policer to police broadcast only and limit CPU traffic */
            conf->port_policer_ext[i].unicast              = FALSE;
            conf->port_policer_ext[i].multicast            = FALSE;
            conf->port_policer_ext[i].broadcast            = TRUE;
            conf->port_policer_ext[i].uc_no_flood          = FALSE;
            conf->port_policer_ext[i].mc_no_flood          = FALSE;
            conf->port_policer_ext[i].flooded              = FALSE;
            conf->port_policer_ext[i].learning             = FALSE;
            conf->port_policer_ext[i].to_cpu               = FALSE;
            {
                int q;
                for (q = 0; q < VTSS_PORT_POLICER_CPU_QUEUES; q++) {
                    conf->port_policer_ext[i].cpu_queue[q]     = FALSE;
                }
            }
            conf->port_policer_ext[i].limit_noncpu_traffic = TRUE;
            conf->port_policer_ext[i].limit_cpu_traffic    = TRUE;
        } else if (i == QOS_STORM_POLICER_UNKNOWN) {      /* Modify this policer to police unknown (flooded) only and limit CPU traffic */
            conf->port_policer_ext[i].unicast              = FALSE;
            conf->port_policer_ext[i].multicast            = FALSE;
            conf->port_policer_ext[i].broadcast            = FALSE;
            conf->port_policer_ext[i].uc_no_flood          = FALSE;
            conf->port_policer_ext[i].mc_no_flood          = FALSE;
            conf->port_policer_ext[i].flooded              = TRUE;
            conf->port_policer_ext[i].learning             = FALSE;
            conf->port_policer_ext[i].to_cpu               = FALSE;
            {
                int q;
                for (q = 0; q < VTSS_PORT_POLICER_CPU_QUEUES; q++) {
                    conf->port_policer_ext[i].cpu_queue[q]     = FALSE;
                }
            }
            conf->port_policer_ext[i].limit_noncpu_traffic = TRUE;
            conf->port_policer_ext[i].limit_cpu_traffic    = TRUE;
        } else
#endif /* defined(VTSS_SW_OPTION_JR_STORM_POLICERS) */
        {
            conf->port_policer_ext[i].unicast              = TRUE;
            conf->port_policer_ext[i].multicast            = TRUE;
            conf->port_policer_ext[i].broadcast            = TRUE;
            conf->port_policer_ext[i].uc_no_flood          = FALSE;
            conf->port_policer_ext[i].mc_no_flood          = FALSE;
            conf->port_policer_ext[i].flooded              = TRUE;
            conf->port_policer_ext[i].learning             = TRUE;
            conf->port_policer_ext[i].to_cpu               = FALSE;
            {
                int q;
                for (q = 0; q < VTSS_PORT_POLICER_CPU_QUEUES; q++) {
                    conf->port_policer_ext[i].cpu_queue[q]     = FALSE;
                }
            }
            conf->port_policer_ext[i].limit_noncpu_traffic = TRUE;
            conf->port_policer_ext[i].limit_cpu_traffic    = FALSE;
        }
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_TTM */
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_FC)
        conf->port_policer_ext[i].flow_control         = FALSE;
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_FC */

#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT */
    }

#if defined(VTSS_SW_OPTION_BUILD_CE)
#ifdef VTSS_FEATURE_QOS_QUEUE_POLICER
    for (i = 0; i < QOS_PORT_QUEUE_CNT; i++) {
        conf->queue_policer[i].enabled      = RATE_STATUS_DISABLE;
        conf->queue_policer[i].policer.rate = QOS_BITRATE_DEF;
    }
#endif /* VTSS_FEATURE_QOS_QUEUE_POLICER */
#endif /* defined(VTSS_SW_OPTION_BUILD_CE) */

#ifdef VTSS_FEATURE_QOS_WFQ_PORT
    conf->wfq_enable = 0;
    conf->weight[VTSS_QUEUE_START] = VTSS_WEIGHT_1;
    conf->weight[VTSS_QUEUE_START + 1] = VTSS_WEIGHT_2;
    conf->weight[VTSS_QUEUE_START + 2] = VTSS_WEIGHT_4;
    conf->weight[VTSS_QUEUE_START + 3] = VTSS_WEIGHT_8;
#endif /* VTSS_FEATURE_QOS_WFQ_PORT */

#ifdef VTSS_FEATURE_QOS_SCHEDULER_V2
    conf->dwrr_enable = FALSE;
    for (i = 0; i < QOS_PORT_WEIGHTED_QUEUE_CNT; i++) {
        conf->queue_pct[i] = 17;  /* This will give each queue an equal initial weight of 17% (16,7%) */
    }
#endif /* VTSS_FEATURE_QOS_SCHEDULER_V2 */

    conf->shaper_status = RATE_STATUS_DISABLE;
    conf->shaper_rate   = QOS_BITRATE_DEF;
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
    conf->shaper_cbs    = QOS_BURSTSIZE_DEF;
    conf->shaper_dlb    = RATE_STATUS_DISABLE;
    conf->shaper_eir    = QOS_BITRATE_DEF;
    conf->shaper_ebs    = QOS_BURSTSIZE_DEF;
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */

#ifdef VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS
    for (i = 0; i < QOS_PORT_QUEUE_CNT; i++) {
        conf->queue_shaper[i].enable = RATE_STATUS_DISABLE;
        conf->queue_shaper[i].rate   = QOS_BITRATE_DEF;
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
        conf->queue_shaper[i].cbs    = QOS_BURSTSIZE_DEF;
        conf->queue_shaper[i].dlb    = RATE_STATUS_DISABLE;
        conf->queue_shaper[i].eir    = QOS_BITRATE_DEF;
        conf->queue_shaper[i].ebs    = QOS_BURSTSIZE_DEF;
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
        conf->excess_enable[i]       = FALSE;
    }
#endif /* VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS */

#ifdef VTSS_FEATURE_QOS_CLASSIFICATION_V2
    conf->default_dpl = 0;
    conf->default_dei = 0;
    conf->tag_class_enable = FALSE;
    for (i = VTSS_PCP_START; i < VTSS_PCP_ARRAY_SIZE; i++) {
        int dei;
        for (dei = VTSS_DEI_START; dei < VTSS_DEI_ARRAY_SIZE; dei++) {
            conf->qos_class_map[i][dei] = qos_pcp2qos(i);
            conf->dp_level_map[i][dei] = dei; // Defaults to same value as DEI
        }
    }

#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
    conf->dscp_class_enable = FALSE; /* Default: no DSCP based classification */
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */

#ifdef VTSS_FEATURE_QOS_TAG_REMARK_V2
    conf->tag_remark_mode = VTSS_TAG_REMARK_MODE_CLASSIFIED;
    conf->tag_default_pcp = 0;
    conf->tag_default_dei = 0;
    for (i = VTSS_PRIO_START; i < QOS_PORT_PRIO_CNT; i++) {
        int dpl;
        for (dpl = 0; dpl < 2; dpl++) {
            conf->tag_pcp_map[i][dpl] = qos_pcp2qos(i);
            conf->tag_dei_map[i][dpl] = dpl; // Defaults to same value as DP level
        }
    }
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */

#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
    conf->dscp_translate = FALSE; /* Default: No Ingress DSCP translate */
    conf->dscp_imode = VTSS_DSCP_MODE_NONE; /* No DSCP ingress classification */
    conf->dscp_emode = VTSS_DSCP_EMODE_DISABLE; /* NO DSCP egress remark */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */
#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
    conf->tx_rate_limiter.preamble_in_payload = TRUE;
    conf->tx_rate_limiter.frame_rate          = 0x40;
    conf->tx_rate_limiter.payload_rate        = 100;
    conf->tx_rate_limiter.frame_overhead      = 12;
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */
#if defined(VTSS_ARCH_SERVAL)
    conf->key_type = VTSS_VCAP_KEY_TYPE_NORMAL;
#endif /* defined(VTSS_ARCH_SERVAL) */
}

/* Check port QOS configuration */
static vtss_rc qos_port_conf_check(qos_port_conf_t *conf)
{
    int i;

    if (conf->default_prio > iprio2uprio(QOS_PORT_PRIO_CNT - 1)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }

    if (conf->usr_prio > (VTSS_PCPS - 1)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }

    for (i = 0; i < VTSS_PORT_POLICERS; i++) {
        if ((conf->port_policer[i].policer.rate < QOS_BITRATE_MIN) || (conf->port_policer[i].policer.rate > QOS_BITRATE_MAX)) {
            T_W("Invalid parameter");
            return QOS_ERROR_PARM;
        }

#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT)
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL)
        if (conf->port_policer_ext[i].dp_bypass_level > QOS_DPL_MAX) {
            T_W("Invalid parameter");
            return QOS_ERROR_PARM;
        }
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT_DPBL */
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT */
    }

#if defined(VTSS_SW_OPTION_BUILD_CE)
#ifdef VTSS_FEATURE_QOS_QUEUE_POLICER
    for (i = 0; i < QOS_PORT_QUEUE_CNT; i++) {
        if ((conf->queue_policer[i].policer.rate < QOS_BITRATE_MIN) || (conf->queue_policer[i].policer.rate > QOS_BITRATE_MAX)) {
            T_W("Invalid parameter");
            return QOS_ERROR_PARM;
        }
    }
#endif /* VTSS_FEATURE_QOS_QUEUE_POLICER */
#endif /* defined(VTSS_SW_OPTION_BUILD_CE) */

#ifdef VTSS_FEATURE_QOS_WFQ_PORT
    for (i = 0; i < QOS_PORT_QUEUE_CNT; i++) {
        if (conf->weight[i] > VTSS_WEIGHT_8) {
            T_W("Invalid parameter");
            return QOS_ERROR_PARM;
        }
    }
#endif /* VTSS_FEATURE_QOS_WFQ_PORT */

#ifdef VTSS_FEATURE_QOS_SCHEDULER_V2
    for (i = 0; i < QOS_PORT_WEIGHTED_QUEUE_CNT; i++) {
        if ((conf->queue_pct[i] < 1) || (conf->queue_pct[i] > 100)) {
            T_W("Invalid parameter");
            return QOS_ERROR_PARM;
        }
    }
#endif /* VTSS_FEATURE_QOS_SCHEDULER_V2 */

    if ((conf->shaper_rate < QOS_BITRATE_MIN) || (conf->shaper_rate > QOS_BITRATE_MAX)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
    if ((conf->shaper_cbs < QOS_BURSTSIZE_MIN) || (conf->shaper_cbs > QOS_BURSTSIZE_MAX)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
    if ((conf->shaper_eir < QOS_BITRATE_MIN) || (conf->shaper_eir > QOS_BITRATE_MAX)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
    if ((conf->shaper_ebs < QOS_BURSTSIZE_MIN) || (conf->shaper_ebs > QOS_BURSTSIZE_MAX)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */

#ifdef VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS
    for (i = 0; i < QOS_PORT_QUEUE_CNT; i++) {
        if ((conf->queue_shaper[i].rate < QOS_BITRATE_MIN) || (conf->queue_shaper[i].rate > QOS_BITRATE_MAX)) {
            T_W("Invalid parameter");
            return QOS_ERROR_PARM;
        }
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
        if ((conf->queue_shaper[i].cbs < QOS_BURSTSIZE_MIN) || (conf->queue_shaper[i].cbs > QOS_BURSTSIZE_MAX)) {
            T_W("Invalid parameter");
            return QOS_ERROR_PARM;
        }
        if ((conf->queue_shaper[i].eir < QOS_BITRATE_MIN) || (conf->queue_shaper[i].eir > QOS_BITRATE_MAX)) {
            T_W("Invalid parameter");
            return QOS_ERROR_PARM;
        }
        if ((conf->queue_shaper[i].ebs < QOS_BURSTSIZE_MIN) || (conf->queue_shaper[i].ebs > QOS_BURSTSIZE_MAX)) {
            T_W("Invalid parameter");
            return QOS_ERROR_PARM;
        }
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
    }
#endif /* VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS */

#ifdef VTSS_FEATURE_QOS_CLASSIFICATION_V2
    if (conf->default_dpl > QOS_DPL_MAX) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
    if (conf->default_dei > (VTSS_DEIS - 1)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
    for (i = VTSS_PCP_START; i < VTSS_PCP_ARRAY_SIZE; i++) {
        int dei;
        for (dei = VTSS_DEI_START; dei < VTSS_DEI_ARRAY_SIZE; dei++) {
            if (conf->qos_class_map[i][dei] > (QOS_CLASS_CNT - 1)) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
            if (conf->dp_level_map[i][dei] > QOS_DPL_MAX) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
        }
    }
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */

#ifdef VTSS_FEATURE_QOS_TAG_REMARK_V2
    if (conf->tag_remark_mode > VTSS_TAG_REMARK_MODE_MAPPED) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
    if (conf->tag_default_pcp > (VTSS_PCPS - 1)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
    if (conf->tag_default_dei > (VTSS_DEIS - 1)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
    for (i = VTSS_PRIO_START; i < QOS_PORT_PRIO_CNT; i++) {
        int dpl;
        for (dpl = 0; dpl < 2; dpl++) {
            if (conf->tag_pcp_map[i][dpl] > (VTSS_PCPS - 1)) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
            if (conf->tag_dei_map[i][dpl] > (VTSS_DEIS - 1)) {
                T_W("Invalid parameter");
                return QOS_ERROR_PARM;
            }
        }
    }
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */

#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
    if (conf->dscp_imode > VTSS_DSCP_MODE_ALL) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE)
    if (conf->dscp_emode > VTSS_DSCP_EMODE_REMAP_DPA) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
#else
    if (conf->dscp_emode > VTSS_DSCP_EMODE_REMAP) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
#endif /* defined(VTSS_FEATURE_QOS_DSCP_REMARK_DP_AWARE) */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */

#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
    if ((conf->tx_rate_limiter.frame_rate < 12) || (conf->tx_rate_limiter.frame_rate > 16383)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
    if (conf->tx_rate_limiter.payload_rate > 100) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
    if ((conf->tx_rate_limiter.frame_overhead < 12) || (conf->tx_rate_limiter.frame_overhead > 261120)) {
        T_W("Invalid parameter");
        return QOS_ERROR_PARM;
    }
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */
    return VTSS_OK;
}

/* Apply QOS port configuration to local switch */
static void qos_port_conf_apply(vtss_port_no_t port_no, qos_port_conf_t *qos_port_conf)
{
    vtss_qos_port_conf_t qos_port_ll_conf;
    vtss_rc              rc;
    int                  i;
    vtss_prio_t          max_prio = msg_max_user_prio(); /* bug#9163 */

    qos_port_conf_change_event(VTSS_ISID_LOCAL, port_no, qos_port_conf); // Call local callbacks

    if ((rc = vtss_qos_port_conf_get(NULL, port_no, &qos_port_ll_conf)) != VTSS_OK) {
        T_E("Error getting QoS port %u configuration - %s\n", port_no, error_txt(rc));
    } else {
        qos_port_ll_conf.default_prio = MIN(max_prio, qos_port_conf->default_prio);

#if defined(QOS_USE_FIXED_PCP_QOS_MAP)
        /*
         * Due to bug#7119 we cannot use both QoS class (default_prio) and PCP (usr_prio) on stackable jaguar or 48 port jaguar.
         * Instead we use a fixed mapping between QoS class and PCP and ignore the PCP value from the application.
         * See also bug#6918, bug#8704 and bug#8758
         */

        /*
         * Suppress Lint warning: "Expression with side effects passed to repeated parameter 2 in macro 'MIN'"
         * Function qos_pcp2qos() has no side effects!
         */
        /*lint -e{666} */
        qos_port_ll_conf.usr_prio = MIN(max_prio, qos_pcp2qos(qos_port_conf->default_prio)); // Use fixed mapping
#else
        qos_port_ll_conf.usr_prio = MIN(max_prio, qos_port_conf->usr_prio);                  // Use configured value
#endif /* defined(QOS_USE_FIXED_PCP_QOS_MAP) */

#ifdef VTSS_FEATURE_QOS_WFQ_PORT
        qos_port_ll_conf.wfq_enable = qos_port_conf->wfq_enable;
        qos_port_ll_conf.weight[VTSS_QUEUE_START] = qos_port_conf->weight[VTSS_QUEUE_START];
        qos_port_ll_conf.weight[VTSS_QUEUE_START + 1] = qos_port_conf->weight[VTSS_QUEUE_START + 1];
        qos_port_ll_conf.weight[VTSS_QUEUE_START + 2] = qos_port_conf->weight[VTSS_QUEUE_START + 2];
        qos_port_ll_conf.weight[VTSS_QUEUE_START + 3] = qos_port_conf->weight[VTSS_QUEUE_START + 3];
#endif /* VTSS_FEATURE_QOS_WFQ_PORT */

#ifdef VTSS_FEATURE_QOS_SCHEDULER_V2
        qos_port_ll_conf.dwrr_enable = qos_port_conf->dwrr_enable;
        for (i = 0; i < QOS_PORT_WEIGHTED_QUEUE_CNT; i++) {
            qos_port_ll_conf.queue_pct[i] = qos_port_conf->queue_pct[i];
        }
#endif /* VTSS_FEATURE_QOS_SCHEDULER_V2 */

#ifdef VTSS_FEATURE_QOS_CLASSIFICATION_V2
        qos_port_ll_conf.default_dpl         = qos_port_conf->default_dpl;

#if defined(QOS_USE_FIXED_PCP_QOS_MAP)
        /*
         * Bug#7119 limit some of the features on stackable jaguar or 48 port jaguar.
         * We use a fixed mapping between DP level and DEI and ignore the DEI value from the application.
         * We always enable tag classification and use a fixed mapping table for See also bug#6918, bug#8704 and bug#8758
         */
        qos_port_ll_conf.default_dei      = qos_port_conf->default_dpl ? 1 : 0; // Use fixed mapping
        qos_port_ll_conf.tag_class_enable = TRUE;                               // Always enable classification from tag
        for (i = VTSS_PCP_START; i < VTSS_PCP_ARRAY_SIZE; i++) {
            int dei;
            for (dei = VTSS_DEI_START; dei < VTSS_DEI_ARRAY_SIZE; dei++) {
                /*
                 * Suppress Lint warning: "Expression with side effects passed to repeated parameter 2 in macro 'MIN'"
                 * Function qos_pcp2qos() has no side effects!
                 */
                /*lint -e{666} */
                qos_port_ll_conf.qos_class_map[i][dei] = MIN(max_prio, qos_pcp2qos(i)); // Use fixed mapping
                qos_port_ll_conf.dp_level_map[i][dei]  = dei;                           // Use fixed mapping
            }
        }
#else
        qos_port_ll_conf.default_dei         = qos_port_conf->default_dei;      // Use configured value
        qos_port_ll_conf.tag_class_enable    = qos_port_conf->tag_class_enable; // Use configured value
        for (i = VTSS_PCP_START; i < VTSS_PCP_ARRAY_SIZE; i++) {
            int dei;
            for (dei = VTSS_DEI_START; dei < VTSS_DEI_ARRAY_SIZE; dei++) {
                qos_port_ll_conf.qos_class_map[i][dei] = MIN(max_prio, qos_port_conf->qos_class_map[i][dei]); // Use configured value
                qos_port_ll_conf.dp_level_map[i][dei]  = qos_port_conf->dp_level_map[i][dei];                 // Use configured value
            }
        }
#endif /* defined(QOS_USE_FIXED_PCP_QOS_MAP) */

#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
        qos_port_ll_conf.dscp_class_enable = qos_port_conf->dscp_class_enable;
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */

#ifdef VTSS_FEATURE_QOS_TAG_REMARK_V2
        qos_port_ll_conf.tag_remark_mode = qos_port_conf->tag_remark_mode;
        qos_port_ll_conf.tag_default_pcp = qos_port_conf->tag_default_pcp;
        qos_port_ll_conf.tag_default_dei = qos_port_conf->tag_default_dei;
        for (i = VTSS_PRIO_START; i < QOS_PORT_PRIO_CNT; i++) {
            int dpl;
            for (dpl = 0; dpl < 2; dpl++) {
                qos_port_ll_conf.tag_pcp_map[i][dpl] = qos_port_conf->tag_pcp_map[i][dpl];
                qos_port_ll_conf.tag_dei_map[i][dpl] = qos_port_conf->tag_dei_map[i][dpl];
            }
        }
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */

#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
        qos_port_ll_conf.dscp_translate = qos_port_conf->dscp_translate;
        qos_port_ll_conf.dscp_mode = qos_port_conf->dscp_imode;
        qos_port_ll_conf.dscp_emode = qos_port_conf->dscp_emode;
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */

        for (i = 0; i < VTSS_PORT_POLICERS; i++) {
            if (qos_port_conf->port_policer[i].enabled) {
                qos_port_ll_conf.policer_port[i].rate = qos_port_conf->port_policer[i].policer.rate;
            } else {
                qos_port_ll_conf.policer_port[i].rate = VTSS_BITRATE_DISABLED;
            }
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT)
            qos_port_ll_conf.policer_ext_port[i] = qos_port_conf->port_policer_ext[i];
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT */
        }
#if defined(VTSS_SW_OPTION_BUILD_CE)
#ifdef VTSS_FEATURE_QOS_QUEUE_POLICER
        for (i = 0; i < QOS_PORT_QUEUE_CNT; i++) {
            if (qos_port_conf->queue_policer[i].enabled != RATE_STATUS_DISABLE) {
                qos_port_ll_conf.policer_queue[i].rate = qos_port_conf->queue_policer[i].policer.rate;
            } else {
                qos_port_ll_conf.policer_queue[i].rate = VTSS_BITRATE_DISABLED;
            }
        }
#endif /* VTSS_FEATURE_QOS_QUEUE_POLICER */
#endif /* defined(VTSS_SW_OPTION_BUILD_CE) */

        if (qos_port_conf->shaper_status != RATE_STATUS_DISABLE) {
            qos_port_ll_conf.shaper_port.rate = qos_port_conf->shaper_rate;
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
            qos_port_ll_conf.shaper_port.level = qos_port_conf->shaper_cbs;
            if (qos_port_conf->shaper_dlb) {
                qos_port_ll_conf.shaper_port.eir = qos_port_conf->shaper_eir;
                qos_port_ll_conf.shaper_port.ebs = qos_port_conf->shaper_ebs;
            } else {
                qos_port_ll_conf.shaper_port.eir = VTSS_BITRATE_DISABLED;
            }
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
        } else {
            qos_port_ll_conf.shaper_port.rate = VTSS_BITRATE_DISABLED;
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
            qos_port_ll_conf.shaper_port.eir = VTSS_BITRATE_DISABLED;
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
        }

#ifdef VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS
        for (i = 0; i < QOS_PORT_QUEUE_CNT; i++) {
            if (qos_port_conf->queue_shaper[i].enable != RATE_STATUS_DISABLE) {
                qos_port_ll_conf.shaper_queue[i].rate = qos_port_conf->queue_shaper[i].rate;
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
                qos_port_ll_conf.shaper_queue[i].level = qos_port_conf->queue_shaper[i].cbs;
                if (qos_port_conf->queue_shaper[i].dlb) {
                    qos_port_ll_conf.shaper_queue[i].eir = qos_port_conf->queue_shaper[i].eir;
                    qos_port_ll_conf.shaper_queue[i].ebs = qos_port_conf->queue_shaper[i].ebs;
                } else {
                    qos_port_ll_conf.shaper_queue[i].eir = VTSS_BITRATE_DISABLED;
                }
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
            } else {
                qos_port_ll_conf.shaper_queue[i].rate = VTSS_BITRATE_DISABLED;
#if defined(VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB)
                qos_port_ll_conf.shaper_queue[i].eir = VTSS_BITRATE_DISABLED;
#endif /* VTSS_FEATURE_QOS_EGRESS_SHAPERS_DLB */
            }
            qos_port_ll_conf.excess_enable[i] = qos_port_conf->excess_enable[i];
        }
#endif /* VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS */

#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
        qos_port_ll_conf.tx_rate_limiter = qos_port_conf->tx_rate_limiter;
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */

#ifdef VTSS_FEATURE_QCL_DMAC_DIP
        qos_port_ll_conf.dmac_dip = qos_port_conf->dmac_dip;
#endif /* VTSS_FEATURE_QCL_DMAC_DIP */

#if defined(VTSS_ARCH_SERVAL)
        qos_port_ll_conf.key_type = qos_port_conf->key_type;
#endif

        if ((rc = vtss_qos_port_conf_set(NULL, port_no, &qos_port_ll_conf)) != VTSS_OK) {
            T_E("Error setting QoS port %u configuration - %s\n", port_no, error_txt(rc));
        }
    }
}

static vtss_rc qos_qcl_conf_commit(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    conf_blk_id_t   blk_id;
    qcl_qce_blk_t   *qce_blk;
    qcl_qce_t       *qce;
    ulong           i, j;

    QOS_CRIT_ASSERT_LOCKED();
    blk_id = CONF_BLK_QOS_QCL_QCE_TABLE;
    if ((qce_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
        T_E("Failed to open QCL QCE table");
        return QOS_ERROR_GEN;
    }
    for (i = 0; i < QCL_MAX; i++) {
        for (qce = qos.qcl_qce_stack_list[i].qce_used_list, j = 0; qce != NULL; qce = qce->next, j++) {
            qce_blk->table[(i * QCE_MAX) + j] = qce->conf;
        }
        qce_blk->count[i] = j;
    }
    conf_sec_close(CONF_SEC_GLOBAL, blk_id);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return VTSS_OK;
}

#if defined(VTSS_FEATURE_QCL_V2)
static void qce_range_set_u16(vtss_vcap_vr_t *dest, qos_qce_vr_u16_t *src)
{
    if (src->in_range) {
        dest->type = VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE;
        dest->vr.r.low = src->vr.r.low;
        dest->vr.r.high = src->vr.r.high;
    } else {
        dest->type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
        dest->vr.v.value = src->vr.v.value;
        dest->vr.v.mask = src->vr.v.mask;
    }
}

static void qce_range_set_u8(vtss_vcap_vr_t *dest, qos_qce_vr_u8_t *src)
{
    if (src->in_range) {
        dest->type = VTSS_VCAP_VR_TYPE_RANGE_INCLUSIVE;
        dest->vr.r.low = src->vr.r.low;
        dest->vr.r.high = src->vr.r.high;
    } else {
        dest->type = VTSS_VCAP_VR_TYPE_VALUE_MASK;
        dest->vr.v.value = src->vr.v.value;
        dest->vr.v.mask = src->vr.v.mask;
    }
}


#endif  /* VTSS_FEATURE_QCL_V2 */

/* Add QCE via switch API */
static vtss_rc qce_entry_add(vtss_qcl_id_t qcl_id, vtss_qce_id_t qce_id, qos_qce_entry_conf_t *conf)
{
    vtss_qce_t           qce;
    int                  i;
#if defined(VTSS_FEATURE_QCL_V2)
    port_iter_t          pit;
#endif

#if defined(VTSS_FEATURE_QCL_V2)
    /* Make sure control application and H/W are synced with config */
    QOS_CRIT_ASSERT_LOCKED();

    (void) vtss_qce_init(NULL, conf->type, &qce);
    qce.id = conf->id;

    /* port list (skip stack ports here) */
    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        if (VTSS_PORT_BF_GET(conf->port_list, pit.iport)) {
            qce.key.port_list[pit.iport] = TRUE;
        }
    }
    /* action */
    if (QCE_ENTRY_CONF_ACTION_GET(conf->action.action_bits,
                                  QOS_QCE_ACTION_PRIO)) {
        vtss_prio_t max_prio = msg_max_user_prio(); /* bug#9163 */
        qce.action.prio = MIN(max_prio, conf->action.prio);
        qce.action.prio_enable = TRUE;
    }
    if (QCE_ENTRY_CONF_ACTION_GET(conf->action.action_bits,
                                  QOS_QCE_ACTION_DP)) {
        qce.action.dp =  conf->action.dp;
        qce.action.dp_enable = TRUE;
    }
    if (QCE_ENTRY_CONF_ACTION_GET(conf->action.action_bits,
                                  QOS_QCE_ACTION_DSCP)) {
        qce.action.dscp =  conf->action.dscp;
        qce.action.dscp_enable = TRUE;
    }

#if defined(VTSS_ARCH_SERVAL)
    if (QCE_ENTRY_CONF_ACTION_GET(conf->action.action_bits,
                                  QOS_QCE_ACTION_PCP_DEI)) {
        qce.action.pcp = conf->action.pcp;
        qce.action.dei = conf->action.dei;
        qce.action.pcp_dei_enable = TRUE;
    }
    if (QCE_ENTRY_CONF_ACTION_GET(conf->action.action_bits,
                                  QOS_QCE_ACTION_POLICY)) {
        qce.action.policy_no = conf->action.policy_no;
        qce.action.policy_no_enable = TRUE;
    }
#endif /* defined(VTSS_ARCH_SERVAL) */

    /* vlan */
    qce_range_set_u16(&qce.key.tag.vid, &conf->key.vid);
    qce.key.tag.pcp = conf->key.pcp;
    qce.key.tag.tagged = QCE_ENTRY_CONF_KEY_GET(conf->key.key_bits, QOS_QCE_VLAN_TAG);
    qce.key.tag.dei = QCE_ENTRY_CONF_KEY_GET(conf->key.key_bits, QOS_QCE_VLAN_DEI);
#if defined(VTSS_ARCH_SERVAL)
    qce.key.tag.s_tag = QCE_ENTRY_CONF_KEY_GET(conf->key.key_bits, QOS_QCE_VLAN_S_TAG);
    qce.key.inner_tag = conf->key.inner_tag;

    /* MAC */
    qce.key.mac.dmac = conf->key.dmac;
#endif /* defined(VTSS_ARCH_SERVAL) */
    memcpy(&qce.key.mac.smac, &conf->key.smac, sizeof(qce.key.mac.smac));
    switch (QCE_ENTRY_CONF_KEY_GET(conf->key.key_bits, QOS_QCE_DMAC_TYPE)) {
    case QOS_QCE_DMAC_TYPE_ANY:
        qce.key.mac.dmac_mc = VTSS_VCAP_BIT_ANY;
        qce.key.mac.dmac_bc = VTSS_VCAP_BIT_ANY;
        break;
    case QOS_QCE_DMAC_TYPE_UC:
        qce.key.mac.dmac_mc = VTSS_VCAP_BIT_0;
        qce.key.mac.dmac_bc = VTSS_VCAP_BIT_0;
        break;
    case QOS_QCE_DMAC_TYPE_MC:
        qce.key.mac.dmac_mc = VTSS_VCAP_BIT_1;
        qce.key.mac.dmac_bc = VTSS_VCAP_BIT_0;
        break;
    case QOS_QCE_DMAC_TYPE_BC:
        qce.key.mac.dmac_mc = VTSS_VCAP_BIT_0;
        qce.key.mac.dmac_bc = VTSS_VCAP_BIT_1;
        break;
    }
#endif /* VTSS_FEATURE_QCL_V2 */

    switch (conf->type) {

#if defined(VTSS_FEATURE_QCL_V2)
    case VTSS_QCE_TYPE_ETYPE: {
        qce.key.frame.etype.etype = conf->key.frame.etype.etype;
        break;
    }
    case VTSS_QCE_TYPE_LLC: {
        /* DSAP, SSAP and Control are first 3 bytes in API structure */
        if (conf->key.frame.llc.dsap.mask) {
            qce.key.frame.llc.data.value[0] = conf->key.frame.llc.dsap.value;
            qce.key.frame.llc.data.mask[0] = conf->key.frame.llc.dsap.mask;
        }
        if (conf->key.frame.llc.ssap.mask) {
            qce.key.frame.llc.data.value[1] = conf->key.frame.llc.ssap.value;
            qce.key.frame.llc.data.mask[1] = conf->key.frame.llc.ssap.mask;
        }
        if (conf->key.frame.llc.control.mask) {
            qce.key.frame.llc.data.value[2] = conf->key.frame.llc.control.value;
            qce.key.frame.llc.data.mask[2] = conf->key.frame.llc.control.mask;
        }
        break;
    }
    case VTSS_QCE_TYPE_SNAP: {
        /* bytes 3 and 4 are the PID */
        qce.key.frame.snap.data.value[3] = conf->key.frame.snap.pid.value[0];
        qce.key.frame.snap.data.value[4] = conf->key.frame.snap.pid.value[1];
        qce.key.frame.snap.data.mask[3] = conf->key.frame.snap.pid.mask[0];
        qce.key.frame.snap.data.mask[4] = conf->key.frame.snap.pid.mask[1];
        break;
    }
    case VTSS_QCE_TYPE_IPV4: {
        qce.key.frame.ipv4.fragment = QCE_ENTRY_CONF_KEY_GET(conf->key.key_bits, QOS_QCE_IPV4_FRAGMENT);
        qce.key.frame.ipv4.proto = conf->key.frame.ipv4.proto;
        qce.key.frame.ipv4.sip = conf->key.frame.ipv4.sip;
#if defined(VTSS_ARCH_SERVAL)
        qce.key.frame.ipv4.dip = conf->key.frame.ipv4.dip;
#endif /* defined(VTSS_ARCH_SERVAL) */
        qce_range_set_u8(&qce.key.frame.ipv4.dscp, &conf->key.frame.ipv4.dscp);
        qce_range_set_u16(&qce.key.frame.ipv4.sport, &conf->key.frame.ipv4.sport);
        qce_range_set_u16(&qce.key.frame.ipv4.dport, &conf->key.frame.ipv4.dport);
        break;
    }
    case VTSS_QCE_TYPE_IPV6: {
        qce.key.frame.ipv6.proto = conf->key.frame.ipv6.proto;
        for (i = 0; i < 4; i++) {
            qce.key.frame.ipv6.sip.value[i + 12] = conf->key.frame.ipv6.sip.value[i];
            qce.key.frame.ipv6.sip.mask[i + 12] = conf->key.frame.ipv6.sip.mask[i];
        }
#if defined(VTSS_ARCH_SERVAL)
        for (i = 0; i < 4; i++) {
            qce.key.frame.ipv6.dip.value[i + 12] = conf->key.frame.ipv6.dip.value[i];
            qce.key.frame.ipv6.dip.mask[i + 12] = conf->key.frame.ipv6.dip.mask[i];
        }
#endif /* defined(VTSS_ARCH_SERVAL) */
        qce_range_set_u8(&qce.key.frame.ipv6.dscp, &conf->key.frame.ipv6.dscp);
        qce_range_set_u16(&qce.key.frame.ipv6.sport, &conf->key.frame.ipv6.sport);
        qce_range_set_u16(&qce.key.frame.ipv6.dport, &conf->key.frame.ipv6.dport);
        break;
    }
#endif /* VTSS_FEATURE_QCL_V2 */
    default:
        break;
    }

    return vtss_qce_add(NULL, qcl_id, qce_id, &qce);
}

/*
 * Local functions for manipulating qce lists.
 * They are modelled after the corresponding functions in the API
 */

#if defined(VTSS_FEATURE_QCL_V2)
/* Change QCE's conflict status */
static void qcl_qce_conflict_set(vtss_qcl_id_t qcl_id,
                                 vtss_qce_id_t qce_id, BOOL conflict)
{
    qcl_qce_t            *qce;
    qcl_qce_list_table_t *list;

    QOS_CRIT_ASSERT_LOCKED();
    T_D("enter, QCE id: %d", qce_id);

    list = &qos.qcl_qce_switch_list[qcl_id];
    for (qce = list->qce_used_list; qce != NULL; qce = qce->next) {
        if (qce->conf.id == qce_id) { /* Found existing entry */
            qce->conf.conflict = conflict;
            break;
        }
    }
}

/* Solve conflict QCE */
static void qcl_mgmt_conflict_solve(vtss_qcl_id_t qcl_id)
{
    qcl_qce_list_table_t *list;
    qcl_qce_t           *qce, *qce_next;
    vtss_qce_id_t       next_id;

    QOS_CRIT_ASSERT_LOCKED();
    T_D("enter");

    list = &qos.qcl_qce_switch_list[qcl_id];
    /* multiple QCE can be added into H/W, specially if those use
       same range checker */
    for (qce = list->qce_used_list; qce != NULL; qce = qce->next) {
        if (qce->conf.conflict == TRUE) {
            /* Next_id (i.e. qce_id) should exist in ASIC i.e. find the first id
               starting from the current location id with non-conflict status */
            next_id = QCE_ID_NONE;
            for (qce_next = qce->next; qce_next != NULL; qce_next = qce_next->next) {
                if (!qce_next->conf.conflict) {
                    next_id = qce_next->conf.id;
                    break;
                }
            }

            /* Set this entry to ASIC layer */
            if (qce_entry_add(qcl_id, next_id, &qce->conf) == VTSS_OK) {
                qce->conf.conflict = FALSE;
            }
        }
    }

    T_D("exit");
}

/*
 * Local functions for manipulating qce lists.
 * They are modelled after the corresponding functions in the API
 */

/**
 * \brief qos_qce_entry_add
 * Adds a qce entry to a list. If an entry with the same id exists, it is reused and probably moved.
 * \param list [in] The list to operate on.
 * \param qce_id [IN] The new qce will be added before this qce. If qce_id == QCE_ID_NONE, the new qce is added to the end of list.
 * \param conf [IN] The new qce to be added. if conf->id == QCE_ID_NONE, it is assigned to the lowest unused id.
 * \param isid_del [OUT] Switch ID from where qce should be deleted.
 * \param found [OUT] TRUE if entry is already in the list or FALSE if not exist.
 * \param conflict [OUT] TRUE if entry is already in the list with conflict state; FALSE otherwise.
 *
 * \return : Return code
 */
/*lint -e{593} */
/* There is a lint error message: Custodial pointer 'new' possibly not freed or returned. We skip the lint error cause we freed it in qos_qce_entry_del() */
static vtss_rc qos_qce_entry_add(qcl_qce_list_table_t *list,
                                 vtss_qce_id_t *qce_id,
                                 qos_qce_entry_conf_t *conf,
                                 vtss_isid_t *isid_del,
                                 BOOL *found,
                                 BOOL *conflict,
                                 BOOL local)
{
    vtss_rc       rc = VTSS_OK;
    qcl_qce_t     *qce, *prev, *ins = NULL, *ins_prev = NULL;
    qcl_qce_t     *new = NULL, *new_prev = NULL;
    uchar         id_used[QCE_ID_END];
    vtss_qce_id_t i;
    vtss_isid_t   isid;
    int           isid_count[VTSS_ISID_END + 1], found_insertion_place = 0;

    QOS_CRIT_ASSERT_LOCKED();
    T_D("enter, qce_id: %u, conf->id: %u", *qce_id, conf->id);

    /* Check that the QCE ID is valid */
    if (QCL_QCE_ID_GET(*qce_id) == QCL_QCE_ID_GET(conf->id) &&
        QCL_QCE_ID_GET(*qce_id) != 0) {
        T_W("illegal qce id: %d", *qce_id);
        return QOS_ERROR_PARM;
    }

    memset(id_used, 0, sizeof(id_used));
    memset(isid_count, 0, sizeof(isid_count));
    isid_count[conf->isid]++;

    *found = FALSE;
    *conflict = FALSE;
    /* Search for existing entry and place to add */
    for (qce = list->qce_used_list, prev = NULL; qce != NULL; prev = qce, qce = qce->next) {
        if (qce->conf.id == conf->id) {
            /* Entry already exists */
            *found = TRUE;
            *conflict = qce->conf.conflict;
            new = qce;
            new_prev = prev;
        } else {
            isid_count[qce->conf.isid]++;
        }

        if (qce->conf.id == *qce_id) {
            /* Found insertion point */
            ins = qce;
            ins_prev = prev;
        } else if (QCL_QCE_ID_GET(*qce_id) == QCE_ID_NONE) {
            if (found_insertion_place == 0 &&
                QCL_USER_ID_GET(conf->id) > QCL_USER_ID_GET(qce->conf.id)) {
                /* Found insertion place by ordered priority */
                ins = qce;
                ins_prev = prev;
                found_insertion_place = 1;
            }
        }

        /* Mark ID as used */
        if (QCL_USER_ID_GET(conf->id) == QCL_USER_ID_GET(qce->conf.id)) {
            id_used[QCL_QCE_ID_GET(qce->conf.id) - QCE_ID_START] = 1;
        }
    }

    if (QCL_QCE_ID_GET(*qce_id) == QCE_ID_NONE && found_insertion_place == 0) {
        ins_prev = prev;
    }

    /* Check if the place to insert was found */
    if (QCL_QCE_ID_GET(*qce_id) != QCE_ID_NONE && ins == NULL) {
        T_W("user id: %d, qce_id: %d not found", QCL_USER_ID_GET(*qce_id), QCL_QCE_ID_GET(*qce_id));
        rc = QOS_ERROR_QCE_NOT_FOUND;
    }

    /* Check that the QCL is not full for any switch: Since for volatile qce
       it keeps extra space, static entry is added through master and it
       updates the master stack list, switch list for any slave can not be
       full unless master stack list is full */
    if (local == FALSE && QCL_USER_ID_GET(conf->id) == QCL_USER_STATIC) {
        for (isid = VTSS_ISID_START;
             rc == VTSS_OK && isid < VTSS_ISID_END; isid++) {
            if ((isid_count[isid] + isid_count[VTSS_ISID_GLOBAL]) > QCE_MAX) {
                T_W("table is full, isid %d", isid);
                rc = QOS_ERROR_QCE_TABLE_FULL;
            }
        }
    }

    if (rc == VTSS_OK) {
        T_D("using from %s list", new == NULL ? "free" : "used");
        *isid_del = VTSS_ISID_LOCAL;
        if (new == NULL) {
            /* Use first entry in free list */
            new = list->qce_free_list;
            /* 'new' can not be NULL as qce_free_list never be NULL */
            if (new == NULL) {
                T_W("QCE table is full, isid %d", conf->isid);
                return QOS_ERROR_QCE_TABLE_FULL;
            }
            new->conf.conflict = FALSE;
            list->qce_free_list = new->next;
        } else {
            /* Take existing entry out of used list */
            if (ins_prev == new) {
                ins_prev = new_prev;
            }
            if (new_prev == NULL) {
                list->qce_used_list = new->next;
            } else {
                new_prev->next = new->next;
            }

            /* If changing to a specific SID, delete QCE on old SIDs */
            if (new->conf.isid != conf->isid && conf->isid != VTSS_ISID_GLOBAL) {
                *isid_del = new->conf.isid;
            }
        }

        /* assign ID */
        if (QCL_QCE_ID_GET(conf->id) == QCE_ID_NONE) {
            /* Use next available ID */
            for (i = QCE_ID_START; i <= QCE_ID_END; i++) {
                if (!id_used[i - QCE_ID_START]) {
                    conf->id =  QCL_COMBINED_ID_SET(QCL_USER_ID_GET(conf->id), i);;
                    break;
                }
            }
            if (i > QCE_ID_END) {
                /* This will never happen */
                T_W("QCE Auto-assigned fail");
                return QOS_ERROR_QCE_TABLE_FULL;
            }
        }

        conf->conflict = new->conf.conflict;
        new->conf = *conf;

        if (ins_prev == NULL) {
            T_D("inserting first");
            new->next = list->qce_used_list;
            list->qce_used_list = new;
        } else {
            T_D("inserting after ID %d", ins_prev->conf.id);
            new->next = ins_prev->next;
            ins_prev->next = new;
        }

        /* Update the next_id */
        *qce_id = QCE_ID_NONE;
        if (new->next) {
            *qce_id = new->next->conf.id;
        }
        T_D("qce_id: %d, conf->id: %d", *qce_id, conf->id);
    }

    return rc;
}

/**
 * \brief qos_qce_entry_del
 * Delete a qce entry from a list.
 * \param list [in] The list to operate on.
 * \param qce_id [IN] The qce to delete
 *
 * \return : Return code
 */
static vtss_rc qos_qce_entry_del(qcl_qce_list_table_t *list,
                                 const vtss_qce_id_t qce_id,
                                 vtss_isid_t *isid_del,
                                 BOOL *conflict)
{
    qcl_qce_t *qce, *prev;
    BOOL found = FALSE;

    QOS_CRIT_ASSERT_LOCKED();
    T_D("enter, qce_id: %u", qce_id);

    for (qce = list->qce_used_list, prev = NULL; qce != NULL; prev = qce, qce = qce->next) {
        if (qce->conf.id == qce_id) {
            *isid_del = qce->conf.isid;
            /* Move entry from used list to free list */
            if (prev == NULL) {
                list->qce_used_list = qce->next;
            } else {
                prev->next = qce->next;
            }
            *conflict = qce->conf.conflict;
            /* Move entry from used list to free list */
            qce->next = list->qce_free_list;
            list->qce_free_list = qce;
            found = TRUE;
            break;
        }
    }

    if (found) {
        T_D("exit, id: %d not found", qce_id);
    } else {
        T_D("exit, id: %d found", qce_id);
    }
    return (found ? VTSS_OK : QOS_ERROR_QCE_NOT_FOUND);
}
#endif  /* VTSS_FEATURE_QCL_V2 */

/****************************************************************************/
/*  Stack/switch functions                                                  */
/****************************************************************************/

static char *qos_msg_id_txt(qos_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case QOS_MSG_ID_CONF_SET_REQ:
        txt = "QOS_MSG_ID_CONF_SET_REQ";
        break;
    case QOS_MSG_ID_PORT_CONF_SET_REQ:
        txt = "QOS_MSG_ID_PORT_CONF_SET_REQ";
        break;
    case QOS_MSG_ID_PORT_CONF_ALL_SET_REQ:
        txt = "QOS_MSG_ID_PORT_CONF_ALL_SET_REQ";
        break;
    case QOS_MSG_ID_QCE_ADD_REQ:
        txt = "QOS_MSG_ID_QCE_ADD_REQ";
        break;
    case QOS_MSG_ID_QCE_DEL_REQ:
        txt = "QOS_MSG_ID_QCE_DEL_REQ";
        break;
    case QOS_MSG_ID_QCL_CLEAR_REQ:
        txt = "QOS_MSG_ID_QCL_CLEAR_REQ";
        break;
#if defined(VTSS_FEATURE_QCL_V2)
    case QOS_MSG_ID_QCE_GET_REQ:
        txt = "QOS_MSG_ID_QCE_GET_REQ";
        break;
    case QOS_MSG_ID_QCE_GET_REP:
        txt = "QOS_MSG_ID_QCE_GET_REP";
        break;
    case QOS_MSG_ID_QCE_CONFLICT_RSLVD_REQ:
        txt = "QOS_MSG_ID_CONFLICT_RESOLVED_REQ";
        break;
#endif
    default:
        txt = "?";
        break;
    }
    return txt;
}

/* Allocate request/reply buffer */
static qos_msg_req_t *qos_msg_alloc(qos_msg_id_t msg_id, u32 ref_cnt)
{
    qos_msg_req_t *msg;

    if (ref_cnt == 0) {
        return NULL;
    }

    msg = msg_buf_pool_get(qos.request);
    VTSS_ASSERT(msg);
    if (ref_cnt > 1) {
        msg_buf_pool_ref_cnt_set(msg, ref_cnt);
    }
    msg->msg_id = msg_id;
    return msg;
}

static void qos_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    qos_msg_id_t msg_id = *(qos_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */
    u32 ref_cnt;

    ref_cnt = msg_buf_pool_put(msg);
    T_D("msg_id: %d, %s, msg: %p, ref_cnt: %u", msg_id, qos_msg_id_txt(msg_id), msg, ref_cnt);
}

static void qos_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
    size_t req_len = len + MSG_TX_DATA_HDR_LEN(qos_msg_req_t, req);

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    qos_msg_id_t msg_id = *(qos_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s, len: %zd, isid: %d, msg: %p", msg_id, qos_msg_id_txt(msg_id), req_len, isid, msg);
    msg_tx_adv(NULL, qos_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_QOS, isid, msg, req_len);
}

static BOOL qos_msg_rx(void *contxt, const void *const rx_msg, size_t len, vtss_module_id_t modid, ulong isid)
{
    qos_msg_req_t *msg = (qos_msg_req_t *)rx_msg;
    qos_msg_id_t   msg_id = msg->msg_id;

    T_D("msg_id: %d, %s, len: %zd, isid: %u", msg_id, qos_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
    case QOS_MSG_ID_CONF_SET_REQ: {
        qos_conf_t *conf;

        conf = &msg->req.conf_set.conf;
        QOS_CRIT_ENTER();
        qos.qos_local_conf = *conf;
        QOS_CRIT_EXIT();
        qos_conf_apply(conf);
        break;
    }
    case QOS_MSG_ID_PORT_CONF_SET_REQ: {
        vtss_port_no_t  port_no = msg->req.port_conf_set.port_no;
        qos_port_conf_t *qos_port_conf;

        if (port_isid_port_no_is_front_port(VTSS_ISID_LOCAL, port_no)) {
            qos_port_conf = &msg->req.port_conf_set.conf;
            QOS_CRIT_ENTER();
            qos.qos_port_conf[VTSS_ISID_LOCAL][port_no - VTSS_PORT_NO_START] = *qos_port_conf;
            QOS_CRIT_EXIT();
            qos_port_conf_apply(port_no, qos_port_conf);
        }
        break;
    }
    case QOS_MSG_ID_PORT_CONF_ALL_SET_REQ: {
        port_iter_t     pit;
        qos_port_conf_t *qos_port_conf;

        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            qos_port_conf = &msg->req.port_conf_all_set.conf[pit.iport];
            QOS_CRIT_ENTER();
            qos.qos_port_conf[VTSS_ISID_LOCAL][pit.iport - VTSS_PORT_NO_START] = *qos_port_conf;
            QOS_CRIT_EXIT();
            qos_port_conf_apply(pit.iport, qos_port_conf);
        }
        break;
    }
    case QOS_MSG_ID_QCE_ADD_REQ: {
#if defined(VTSS_FEATURE_QCL_V2)
        qos_qce_entry_conf_t *conf;
        qcl_qce_list_table_t *list;
        vtss_isid_t          dummy;
        qcl_qce_t            *qce;
        vtss_qce_id_t        qce_id;
        vtss_rc              rc;
        BOOL                 found, flag;
        BOOL                 old_conflict;

        conf = &msg->req.qce_add.conf;
        QOS_CRIT_ENTER();
        list = &qos.qcl_qce_switch_list[msg->req.qce_add.qcl_id];
        rc = qos_qce_entry_add(list, &msg->req.qce_add.qce_id, conf,
                               &dummy, &found, &old_conflict, 1);
        if (rc == VTSS_OK) {
            /* Set this entry to ASIC layer */
            /* Next_id (i.e. qce_id) should exist in ASIC i.e. if qce_id is not
               QCE_ID_NONE, find the first qce_id starting from the mentioned
               id with non-conflict status */
            if ((qce_id = msg->req.qce_add.qce_id) != QCE_ID_NONE) {
                flag = FALSE;
                qce_id = QCE_ID_NONE;
                for (qce = list->qce_used_list; qce != NULL; qce = qce->next) {
                    if (!flag && qce->conf.id == msg->req.qce_add.qce_id) {
                        flag = TRUE;
                    }
                    if (flag && !qce->conf.conflict) {
                        qce_id = qce->conf.id;
                        break;
                    }
                }
            }

            rc = qce_entry_add(msg->req.qce_add.qcl_id, qce_id, conf);
            if (rc == VTSS_OK) {
                conf->conflict = FALSE;
            } else {
                /* Check if the entry already exists with non-conflict status.
                   In that case if add fails, old entry should be deleted */
                if (found && !old_conflict) {
                    if (vtss_qce_del(NULL, msg->req.qce_del.qcl_id, conf->id) != VTSS_OK) {
                        T_W("Calling vtss_qce_del() failed\n");
                    }
                }

                conf->conflict = TRUE;
            }
            qcl_qce_conflict_set(msg->req.qce_add.qcl_id,
                                 conf->id, conf->conflict);
        }
        QOS_CRIT_EXIT();
#endif /* VTSS_FEATURE_QCL_V2 */
        break;
    }
    case QOS_MSG_ID_QCE_DEL_REQ: {
#if defined(VTSS_FEATURE_QCL_V2)
        qcl_qce_list_table_t *list;
        vtss_isid_t           dummy;
        vtss_qce_id_t         qce_id;
        BOOL                  conflict = TRUE;
        vtss_rc               rc;

        qce_id = msg->req.qce_del.qce_id;
        QOS_CRIT_ENTER();
        list = &qos.qcl_qce_switch_list[msg->req.qce_del.qcl_id];
        rc = qos_qce_entry_del(list, qce_id, &dummy, &conflict);

        if (rc == VTSS_OK && conflict == FALSE) {
            /* Remove this entry from ASIC layer */
            if (vtss_qce_del(NULL, msg->req.qce_del.qcl_id, qce_id) != VTSS_OK) {
                T_W("Calling vtss_qce_del() failed\n");
            }
            qcl_mgmt_conflict_solve(msg->req.qce_del.qcl_id);
        }
        QOS_CRIT_EXIT();
#endif

        break;
    }
    case QOS_MSG_ID_QCL_CLEAR_REQ: {
#if defined(VTSS_FEATURE_QCL_V2)
        qcl_qce_t            *qce;
        qcl_qce_list_table_t *list;
        int                   i;

        QOS_CRIT_ENTER();
        for (i = 0; i < QCL_MAX + RESERVED_QCL_CNT; i++) {
            list = &qos.qcl_qce_switch_list[i];
            while (list->qce_used_list != NULL) {
                qce = list->qce_used_list;
                list->qce_used_list = qce->next;
                if (qce->conf.conflict == FALSE) {
                    /* Remove this entry from ASIC layer */
                    if (vtss_qce_del(NULL, i, qce->conf.id) != VTSS_OK) {
                        T_W("Calling vtss_qce_del() failed\n");
                    }
                }
                /* Move to free list */
                qce->next = list->qce_free_list;
                list->qce_free_list = qce;
            }
        }
        QOS_CRIT_EXIT();
#endif /* VTSS_FEATURE_QCL_V2 */
        break;
    }
#if defined(VTSS_FEATURE_QCL_V2)
    case QOS_MSG_ID_QCE_GET_REQ: {
        qos_msg_req_t *rep_msg;
        qos_qce_entry_conf_t conf;
        vtss_rc rc;

        rc = qcl_list_qce_get(VTSS_ISID_LOCAL, msg->req.qce_get.qcl_id, msg->req.qce_get.qce_id, &conf, msg->req.qce_get.next);

        rep_msg = qos_msg_alloc(QOS_MSG_ID_QCE_GET_REP, 1);
        if (rc == VTSS_OK) {
            rep_msg->req.qce_get.conf   = conf;
            rep_msg->req.qce_get.qcl_id = msg->req.qce_get.qcl_id;
            rep_msg->req.qce_get.qce_id = msg->req.qce_get.qce_id;
        } else {
            rep_msg->req.qce_get.conf.id = QCE_ID_NONE;
        }
        qos_msg_tx(rep_msg, 0, sizeof(rep_msg->req.qce_get));
        break;
    }
    case QOS_MSG_ID_QCE_GET_REP: {

        QOS_CRIT_ENTER();
        qos.qcl_qce_conf_get_rep_info[isid] = msg->req.qce_get.conf;
        cyg_flag_setbits(&qos.qcl_qce_conf_get_flags, 1 << isid);
        QOS_CRIT_EXIT();
        break;
    }
    case QOS_MSG_ID_QCE_CONFLICT_RSLVD_REQ: {
        QOS_CRIT_ENTER();
        qcl_mgmt_conflict_solve(msg->req.qce_conflict.qcl_id);
        QOS_CRIT_EXIT();
        break;
    }
#endif /* VTSS_FEATURE_QCL_V2 */
    default:
        T_E("unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}


static void qos_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = qos_msg_rx;
    filter.modid = VTSS_MODULE_ID_QOS;
    (void) msg_rx_filter_register(&filter);
}

/* Set QOS configuration */
static vtss_rc qos_stack_conf_set(vtss_isid_t isid_add)
{
    qos_msg_req_t *msg;
    switch_iter_t  sit;

    T_N("enter, isid: %d", isid_add);

    (void) switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID);

    /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
    if ((msg = qos_msg_alloc(QOS_MSG_ID_CONF_SET_REQ, sit.remaining)) != NULL) {
        msg->req.conf_set.conf = qos.qos_conf;
        while (switch_iter_getnext(&sit)) {
            qos_msg_tx(msg, sit.isid, sizeof(msg->req.conf_set));
        }
    }
    T_N("exit");
    return VTSS_OK;
}

/* Set QOS PORT configuration. port_no == VTSS_PORT_NO_NONE means all ports */
static vtss_rc qos_stack_port_conf_set(vtss_isid_t isid_add, vtss_port_no_t  port_no)
{
    qos_msg_req_t *msg;
    switch_iter_t  sit;

    // Lint is confused by the unlock/lock of critd here, so we help it a little.
    /*lint --e{454, 455, 456} */
    QOS_CRIT_ASSERT_LOCKED();
    T_N("enter, isid: %d", isid_add);
    (void) switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        if (port_no == VTSS_PORT_NO_NONE) { /* all ports */
            port_iter_t pit;
            msg = qos_msg_alloc(QOS_MSG_ID_PORT_CONF_ALL_SET_REQ, 1);
            memcpy(msg->req.port_conf_all_set.conf, qos.qos_port_conf[sit.isid], sizeof(msg->req.port_conf_all_set.conf));
            /* override default prio if volatile is set */
            (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                if (qos.volatile_default_prio[sit.isid - VTSS_ISID_START][pit.iport - VTSS_PORT_NO_START] != QOS_PORT_PRIO_UNDEF) {
                    msg->req.port_conf_all_set.conf[pit.iport - VTSS_PORT_NO_START].default_prio =
                        qos.volatile_default_prio[sit.isid - VTSS_ISID_START][pit.iport - VTSS_PORT_NO_START];
                }
                QOS_CRIT_EXIT(); // Temporary leave critical section before doing callbacks
                qos_port_conf_change_event(sit.isid, pit.iport, &msg->req.port_conf_all_set.conf[pit.iport - VTSS_PORT_NO_START]); // Call global callbacks
                QOS_CRIT_ENTER(); // Enter critical section again
            }
            qos_msg_tx(msg, sit.isid, sizeof(msg->req.port_conf_all_set));
        } else {
            if (port_isid_port_no_is_front_port(sit.isid, port_no)) {
                msg = qos_msg_alloc(QOS_MSG_ID_PORT_CONF_SET_REQ, 1);
                msg->req.port_conf_set.port_no = port_no;
                msg->req.port_conf_set.conf = qos.qos_port_conf[sit.isid][port_no - VTSS_PORT_NO_START];
                /* override default prio if volatile is set */
                if (qos.volatile_default_prio[sit.isid - VTSS_ISID_START][port_no - VTSS_PORT_NO_START] != QOS_PORT_PRIO_UNDEF) {
                    msg->req.port_conf_set.conf.default_prio = qos.volatile_default_prio[sit.isid - VTSS_ISID_START][port_no - VTSS_PORT_NO_START];
                }
                QOS_CRIT_EXIT(); // Temporary leave critical section before doing callbacks
                qos_port_conf_change_event(sit.isid, port_no, &msg->req.port_conf_set.conf); // Call global callbacks
                QOS_CRIT_ENTER(); // Enter critical section again
                qos_msg_tx(msg, sit.isid, sizeof(msg->req.port_conf_set));
            }
        }
    }
    T_N("exit");
    return VTSS_OK;
}

#if defined(VTSS_FEATURE_QCL_V2)
/* Add QCE to the stack */
static vtss_rc qce_entry_stack_add(vtss_isid_t isid_add, vtss_qcl_id_t qcl_id,
                                   qos_qce_entry_conf_t *conf, BOOL is_next)
{
    qos_msg_req_t *msg;
    switch_iter_t  sit;
    qcl_qce_t      *qce, *stack_pos = NULL;

    QOS_CRIT_ASSERT_LOCKED();
    T_N("enter, isid: %d", isid_add);
    /* find the conf location in used_list, this should be the starting point
       to look next qce for a stack */
    if (is_next) {
        for (stack_pos = qos.qcl_qce_stack_list[qcl_id].qce_used_list;
             stack_pos != NULL; stack_pos = stack_pos->next) {
            if (stack_pos->conf.id == conf->id) {
                break;
            }
        }
    }

    (void) switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        msg = qos_msg_alloc(QOS_MSG_ID_QCE_ADD_REQ, 1);
        msg->req.qce_add.qcl_id = qcl_id;
        /* default next qce is NONE */
        msg->req.qce_add.qce_id = QCE_ID_NONE;
        /* start from conf location, find first qce for isid */
        if (is_next && stack_pos) {
            for (qce = stack_pos->next; qce != NULL; qce = qce->next) {
                if (qce->conf.isid == VTSS_ISID_GLOBAL || qce->conf.isid == sit.isid) {
                    msg->req.qce_add.qce_id = qce->conf.id;
                    break;
                }
            }
        }
        msg->req.qce_add.conf = *conf;
        qos_msg_tx(msg, sit.isid, sizeof(msg->req.qce_add));
    }
    T_N("exit");
    return VTSS_OK;
}
#endif /* VTSS_FEATURE_QCL_V2 */

/* Del QCE to the stack */
static vtss_rc qce_entry_stack_del(vtss_isid_t isid_add, vtss_qcl_id_t qcl_id, vtss_qce_id_t qce_id)
{
    qos_msg_req_t *msg;
    switch_iter_t  sit;

    T_N("enter, isid: %d, qce_id: %u", isid_add, qce_id);

    (void) switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID);

    /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
    if ((msg = qos_msg_alloc(QOS_MSG_ID_QCE_DEL_REQ, sit.remaining)) != NULL) {
        msg->req.qce_del.qcl_id = qcl_id;
        msg->req.qce_del.qce_id = qce_id;

        while (switch_iter_getnext(&sit)) {
            qos_msg_tx(msg, sit.isid, sizeof(msg->req.qce_del));
        }
    }
    T_N("exit");
    return VTSS_OK;
}

/* Clear QCL configuration on a specific switch*/
static vtss_rc qos_stack_qcl_clear(vtss_isid_t isid_add)
{
    qos_msg_req_t  *msg;
    switch_iter_t  sit;

    T_N("enter, isid: %d", isid_add);

    (void) switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID);

    /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
    if ((msg = qos_msg_alloc(QOS_MSG_ID_QCL_CLEAR_REQ, sit.remaining)) != NULL) {
        while (switch_iter_getnext(&sit)) {
            qos_msg_tx(msg, sit.isid, 0);
        }
    }
    T_N("exit");
    return VTSS_OK;
}

#if defined(VTSS_FEATURE_QCL_V2)
/* Del QCE to the stack */
static vtss_rc qce_entry_stack_conflict_resolve(vtss_isid_t isid_add, vtss_qcl_id_t  qcl_id)
{
    qos_msg_req_t *msg;
    switch_iter_t  sit;

    T_N("enter, isid: %d", isid_add);

    (void) switch_iter_init(&sit, isid_add, SWITCH_ITER_SORT_ORDER_ISID);

    /* Allocate a message with a ref-count corresponding to the number of times switch_iter_getnext() will return TRUE. */
    if ((msg = qos_msg_alloc(QOS_MSG_ID_QCE_CONFLICT_RSLVD_REQ, sit.remaining)) != NULL) {
        msg->req.qce_conflict.qcl_id = qcl_id;
        while (switch_iter_getnext(&sit)) {
            qos_msg_tx(msg, sit.isid, sizeof(msg->req.qce_conflict));
        }
    }
    T_N("exit");
    return VTSS_OK;
}

/* Send complete QCL configuration to a specific switch*/
/* qos_stact_qcl_clear() must have been called first */
static vtss_rc qos_stack_qcl_update(vtss_isid_t isid_add)
{
    int        i;
    qcl_qce_t *qce;

    T_N("enter, isid: %d", isid_add);
    for (i = 0; i < QCL_MAX + RESERVED_QCL_CNT; i++) {
        for (qce = qos.qcl_qce_stack_list[i].qce_used_list; qce != NULL; qce = qce->next) {
            if (qce->conf.isid == VTSS_ISID_GLOBAL || qce->conf.isid == isid_add) {
                T_D("add qcl_id: %d, qce id: %u", i, qce->conf.id);
                (void) qce_entry_stack_add(isid_add, i, &qce->conf, FALSE);
            }
        }
    }
    T_N("exit");
    return VTSS_OK;
}
#endif /* VTSS_FEATURE_QCL_V2 */

/* Determine if port and ISID are valid */
static BOOL qos_mgmt_port_sid_invalid(vtss_isid_t isid, vtss_port_no_t port_no, BOOL set)
{
    /* Check ISID */
    if (isid >= VTSS_ISID_END) {
        T_W("illegal isid: %d", isid);
        return 1;
    }
    if (port_no >= port_isid_port_count(isid)) {
        T_W("illegal port_no: %u, isid: %u", port_no, isid);
        return 1;
    }

    if (set && isid == VTSS_ISID_LOCAL) {
        T_W("SET not allowed, isid: %d", isid);
        return 1;
    }

    return 0;
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* QOS error text */
char *qos_error_txt(vtss_rc rc)
{
    switch (rc) {
    case QOS_ERROR_GEN:
        return "QOS generic error";
    case QOS_ERROR_PARM:
        return "QOS parameter error";
    case QOS_ERROR_QCE_NOT_FOUND:
        return "QCE not found";
    case QOS_ERROR_QCE_TABLE_FULL:
        return "QCE is full";
#if defined(VTSS_FEATURE_QCL_V2)
    case QOS_ERROR_QCL_USER_NOT_FOUND:
        return "QoS QCL User not found";
    case QOS_ERROR_STACK_STATE:
        return "QoS stack state error";
    case QOS_ERROR_REQ_TIMEOUT:
        return "Msg Request wait timeout error";
#endif
    default:
        return "QOS unknown error";
    }
}

static const char *const qos_dscp_names[64] = {
    "0  (BE)",  "1",  "2",         "3",  "4",         "5",  "6",         "7",
    "8  (CS1)", "9",  "10 (AF11)", "11", "12 (AF12)", "13", "14 (AF13)", "15",
    "16 (CS2)", "17", "18 (AF21)", "19", "20 (AF22)", "21", "22 (AF23)", "23",
    "24 (CS3)", "25", "26 (AF31)", "27", "28 (AF32)", "29", "30 (AF33)", "31",
    "32 (CS4)", "33", "34 (AF41)", "35", "36 (AF42)", "37", "38 (AF43)", "39",
    "40 (CS5)", "41", "42",        "43", "44",        "45", "46 (EF)",   "47",
    "48 (CS6)", "49", "50",        "51", "52",        "53", "54",        "55",
    "56 (CS7)", "57", "58",        "59", "60",        "61", "62",        "63"
};

const char *qos_dscp2str(vtss_dscp_t dscp)
{
    return (dscp > 63) ? "?" : qos_dscp_names[dscp];
}

#ifdef VTSS_FEATURE_QOS_TAG_REMARK_V2
/* QoS tag remarking mode text string */
char *qos_port_tag_remarking_mode_txt(vtss_tag_remark_mode_t mode)
{
    switch (mode) {
    case VTSS_TAG_REMARK_MODE_CLASSIFIED:
        return ("Classified");
    case VTSS_TAG_REMARK_MODE_DEFAULT:
        return ("Default");
    case VTSS_TAG_REMARK_MODE_MAPPED:
        return ("Mapped");
    default:
        return "?";
    }
}
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */

/* Set QoS General configuration  */
vtss_rc qos_conf_set(qos_conf_t *conf)
{
    vtss_rc        rc = VTSS_OK;

    T_N("enter");
    QOS_CRIT_ENTER();

    /* Move new data to database */
    qos.qos_conf = *conf;

    /* Apply new data to msg layer */
    (void) qos_stack_conf_set(VTSS_ISID_GLOBAL);

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    {
        /* Save new data to conf module */
        qos_conf_blk_t *qos_class_no_blk;
        if ((qos_class_no_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_QOS_TABLE, NULL)) == NULL) {
            rc = QOS_ERROR_GEN;
            goto exit_func;
        }
        qos_class_no_blk->conf = qos.qos_conf;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_QOS_TABLE);
    }
exit_func:
#endif
    QOS_CRIT_EXIT();
    T_N("exit");
    return rc;
}

/* Get QoS General configuration  */
vtss_rc qos_conf_get(qos_conf_t *conf)
{
    T_N("enter");
    if (msg_switch_is_master()) {
        QOS_CRIT_ENTER();
        *conf = qos.qos_conf; // Get the 'real' configuration
        QOS_CRIT_EXIT();
    } else {
        QOS_CRIT_ENTER();
        *conf = qos.qos_local_conf; // get the 'local' configuration
        QOS_CRIT_EXIT();
    }
    T_N("exit");
    return VTSS_OK;
}

/* Get default QoS General configuration  */
vtss_rc qos_conf_get_default(qos_conf_t *conf)
{
    VTSS_ASSERT(conf);
    qos_conf_default_set(conf);
    return VTSS_OK;
}

/* Set port QoS configuration  */
vtss_rc qos_port_conf_set(vtss_isid_t isid, vtss_port_no_t port_no, qos_port_conf_t *conf)
{
    vtss_rc rc = VTSS_OK;
    int port_no_idx = port_no - VTSS_PORT_NO_START;
    BOOL changed;

    T_N("enter, isid: %u, port_no: %u", isid, port_no);

    if (qos_mgmt_port_sid_invalid(isid, port_no, 1)) {
        return QOS_ERROR_PARM;
    }

    QOS_CRIT_ENTER();

    /* Move new data to database */
    changed = memcmp(&qos.qos_port_conf[isid][port_no_idx], conf, sizeof(*conf)) != 0;
    if (!changed) {
        goto exit_func;    // Nothing to do
    }

    qos.qos_port_conf[isid][port_no_idx] = *conf;

    /* Apply new data to msg layer */
    (void) qos_stack_port_conf_set(isid, port_no);

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    {
        /* Save new data to conf module */
        qos_port_conf_blk_t *qos_port_blk;
        int                 isid_idx = isid - VTSS_ISID_START;
        if ((qos_port_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_QOS_PORT_TABLE, NULL)) == NULL) {
            goto exit_func;
        }
        qos_port_blk->conf[isid_idx][port_no_idx] = qos.qos_port_conf[isid][port_no_idx];
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_QOS_PORT_TABLE);
    }
#endif
exit_func:
    QOS_CRIT_EXIT();
    T_N("exit");
    return rc;
}

/* Get port QoS configuration  */
vtss_rc qos_port_conf_get(vtss_isid_t isid, vtss_port_no_t port_no, qos_port_conf_t *conf)
{
    T_N("enter, isid: %u, port_no: %u", isid, port_no);

    if (qos_mgmt_port_sid_invalid(isid, port_no, 0)) {
        return QOS_ERROR_PARM;
    }

    QOS_CRIT_ENTER();
    *conf = qos.qos_port_conf[isid][port_no - VTSS_PORT_NO_START];
    QOS_CRIT_EXIT();
    T_N("exit");
    return VTSS_OK;
}

/* Get default port QoS configuration  */
vtss_rc qos_port_conf_get_default(qos_port_conf_t *conf)
{
    VTSS_ASSERT(conf);
    qos_port_conf_default_set(conf);
    return VTSS_OK;
}

/* Set port QoS volatile default priority. Use QOS_PORT_PRIO_UNDEF to disable */
vtss_rc qos_port_volatile_set_default_prio(vtss_isid_t isid, vtss_port_no_t port_no, vtss_prio_t default_prio)
{
    vtss_rc rc = VTSS_OK;
    int isid_idx    = isid - VTSS_ISID_START;
    int port_no_idx = port_no - VTSS_PORT_NO_START;

    T_N("enter, isid: %u, port_no: %u, default_prio: %u", isid, port_no, default_prio);

    if (qos_mgmt_port_sid_invalid(isid, port_no, 1)) {
        return QOS_ERROR_PARM;
    }

    QOS_CRIT_ENTER();

    /* Check if anything has changed */
    if (qos.volatile_default_prio[isid_idx][port_no_idx] == default_prio) {
        goto exit_func; // Nothing to do
    }
    qos.volatile_default_prio[isid_idx][port_no_idx] = default_prio;

    /* Apply new data to msg layer */
    (void) qos_stack_port_conf_set(isid, port_no);

exit_func:
    QOS_CRIT_EXIT();
    T_N("exit");
    return rc;
}

/* Get port QoS volatile default priority. Returns QOS_PORT_PRIO_UNDEF if not set or called on a slave */
vtss_rc qos_port_volatile_get_default_prio(vtss_isid_t isid, vtss_port_no_t port_no, vtss_prio_t *default_prio)
{
    T_N("enter, isid: %u, port_no: %u", isid, port_no);

    if (qos_mgmt_port_sid_invalid(isid, port_no, 0)) {
        return QOS_ERROR_PARM;
    }

    if (msg_switch_is_master()) {
        QOS_CRIT_ENTER();
        *default_prio = qos.volatile_default_prio[isid - VTSS_ISID_START][port_no - VTSS_PORT_NO_START];
        QOS_CRIT_EXIT();
    } else {
        *default_prio = QOS_PORT_PRIO_UNDEF;
    }
    T_N("exit");
    return VTSS_OK;
}

vtss_rc qos_port_conf_change_register(BOOL global, vtss_module_id_t module_id, qos_port_conf_change_cb_t callback)
{
    vtss_rc rc = VTSS_OK;

    VTSS_ASSERT(callback);

    if (module_id >= VTSS_MODULE_ID_NONE) {
        return QOS_ERROR_PARM;
    }

    QOS_CB_CRIT_ENTER();
    if (qos.rt.count < QOS_PORT_CONF_CHANGE_REG_MAX) {
        qos.rt.reg[qos.rt.count].global    = global;
        qos.rt.reg[qos.rt.count].module_id = module_id;
        qos.rt.reg[qos.rt.count].callback  = callback;
        qos.rt.count++;
    } else {
        T_E("qos port change table full!");
        rc = QOS_ERROR_GEN;
    }
    QOS_CB_CRIT_EXIT();

    return rc;
}

vtss_rc qos_port_conf_change_reg_get(qos_port_conf_change_reg_t *entry, BOOL clear)
{
    vtss_rc rc = QOS_ERROR_GEN;
    int     i;

    QOS_CB_CRIT_ENTER();
    for (i = 0; i < qos.rt.count; i++) {
        qos_port_conf_change_reg_t *reg = &qos.rt.reg[i];
        if (clear) {
            /* Clear all entries */
            reg->max_ticks = 0;
        } else if (entry->module_id == VTSS_MODULE_ID_NONE) {
            /* Get first */
            *entry = *reg;
            rc = VTSS_RC_OK;
            break;
        } else if (entry->global == reg->global && entry->module_id == reg->module_id && entry->callback == reg->callback) {
            /* Get next */
            entry->module_id = VTSS_MODULE_ID_NONE;
        }
    }
    QOS_CB_CRIT_EXIT();

    return rc;
}

#if defined(VTSS_FEATURE_QCL_V2)
static BOOL qcl_mgmt_isid_invalid(vtss_isid_t isid)
{
    if (isid > VTSS_ISID_END) {
        T_W("illegal isid: %d", isid);
        return 1;
    }

    if (isid != VTSS_ISID_LOCAL && !msg_switch_is_master()) {
        T_W("not master");
        return 1;
    }

    if (VTSS_ISID_LEGAL(isid) && !msg_switch_configurable(isid)) {
        T_W("isid %d not active", isid);
        return 1;
    }

    return 0;
}

/* Get QCE or next QCE (use QCE_ID_NONE to get first) */
static vtss_rc qcl_list_qce_get(vtss_isid_t isid,
                                vtss_qcl_id_t qcl_id,
                                vtss_qce_id_t id,
                                qos_qce_entry_conf_t *conf,
                                BOOL next)
{
    vtss_rc              rc = VTSS_OK;
    qcl_qce_t            *qce;
    qcl_qce_list_table_t *list;
    BOOL                 use_next = 0;

    T_D("enter, isid: %d, id: %d, next: %d", isid, id, next);

    QOS_CRIT_ENTER();
    list = (isid == VTSS_ISID_LOCAL ? &qos.qcl_qce_switch_list[qcl_id] :
            &qos.qcl_qce_stack_list[qcl_id]);
    for (qce = list->qce_used_list; qce != NULL; qce = qce->next) {
        /* Skip QCEs for other switches */
        if ((isid != VTSS_ISID_LOCAL && isid != VTSS_ISID_GLOBAL && qce->conf.isid != isid) ||
            QCL_USER_ID_GET(qce->conf.id) != QCL_USER_ID_GET(id)) {
            continue;
        }

        if (QCL_QCE_ID_GET(id) == QCE_ID_NONE) {
            /* Found first QCE */
            break;
        }

        if (use_next) {
            /* Found next QCE */
            break;
        }

        if (qce->conf.id == id) {
            /* Found QCE */
            if (next) {
                use_next = 1;
            } else {
                break;
            }
        }
    }
    if (qce != NULL) {
        /* Found it */
        *conf = qce->conf;
    }
    QOS_CRIT_EXIT();

    T_D("exit, id %d %s found", id, qce == NULL ? "not " : "");

    return (qce == NULL ? QOS_ERROR_QCE_NOT_FOUND : rc);
}

/* Get QCE or next QCE in the specific QCL (use QCE_ID_NONE to get first) (if "qce_id" = QCE_ID_NONE , the value of "next" will be discarded) */
vtss_rc qos_mgmt_qce_entry_get(vtss_isid_t isid, qcl_user_t user_id,
                               vtss_qcl_id_t qcl_id, vtss_qce_id_t qce_id,
                               qos_qce_entry_conf_t *conf, BOOL next)
{
    vtss_rc rc = VTSS_OK;

    T_D("enter, user_id: %d, isid: %d, id: %d, next: %d", user_id, isid, qce_id, next);

    /* Check stack role */
    if (qcl_mgmt_isid_invalid(isid)) {
        T_D("exit");
        return QOS_ERROR_STACK_STATE;
    }

    /* Check user ID */
    if (user_id >= QCL_USER_CNT) {
        T_W("user_id: %d not exist", user_id);
        T_D("exit");
        return QOS_ERROR_QCL_USER_NOT_FOUND;
    }

    /* Convert qcl_id from 1-based to 0-based and check that it is valid */
    if (qcl_id) {
        qcl_id--;
    }

    if (qcl_id >= (QCL_MAX + RESERVED_QCL_CNT)) {
        T_W("Illegal qcl_id: %u", qcl_id + 1);
        return QOS_ERROR_PARM;
    }

    /* Check QCE ID */
    if (qce_id > QCE_ID_END) {
        T_W("id: %d out of range", qce_id);
        T_D("exit");
        return QOS_ERROR_PARM;
    }

    if (msg_switch_is_local(isid)) {
        isid = VTSS_ISID_LOCAL;
    }

    if (isid == VTSS_ISID_LOCAL || isid == VTSS_ISID_GLOBAL) {
        rc = qcl_list_qce_get(isid, qcl_id, QCL_COMBINED_ID_SET(user_id, qce_id), conf, next);
    } else {
        qos_msg_req_t    *req_msg;
        cyg_flag_value_t flag;
        cyg_tick_count_t time_tick;

        req_msg = qos_msg_alloc(QOS_MSG_ID_QCE_GET_REQ, 1);
        req_msg->req.qce_get.qcl_id = qcl_id;
        req_msg->req.qce_get.qce_id = QCL_COMBINED_ID_SET(user_id, qce_id);
        req_msg->req.qce_get.next = next;
        flag = (1 << isid);
        cyg_flag_maskbits(&qos.qcl_qce_conf_get_flags, ~flag);
        qos_msg_tx(req_msg, isid, sizeof(req_msg->req.qce_get));

        time_tick = cyg_current_time() + VTSS_OS_MSEC2TICK(QCL_REQ_TIMEOUT * 1000);
        if (cyg_flag_timed_wait(&qos.qcl_qce_conf_get_flags, flag, CYG_FLAG_WAITMODE_OR, time_tick) & flag ? 0 : 1) {
            T_W("timeout, QCL_MSG_ID_VOLATILE_QCE_CONF_GET_REQ");
            return QOS_ERROR_REQ_TIMEOUT;
        }

        QOS_CRIT_ENTER();
        if (qos.qcl_qce_conf_get_rep_info[isid].id == QCE_ID_NONE) {
            QOS_CRIT_EXIT();
            return QOS_ERROR_QCE_NOT_FOUND;
        } else {
            *conf = qos.qcl_qce_conf_get_rep_info[isid];
        }
        QOS_CRIT_EXIT();
    }

    /* Restore independent QCE ID */
    conf->id = QCL_QCE_ID_GET(conf->id);

    return rc;
}

/* Add QCE entry before given QCE or last (QCE_ID_NONE) to the specific QCL */
vtss_rc qos_mgmt_qce_entry_add(qcl_user_t user_id, vtss_qcl_id_t qcl_id,
                               vtss_qce_id_t qce_id, qos_qce_entry_conf_t *conf)
{
    qcl_qce_list_table_t *list;
    vtss_rc              rc;
    vtss_isid_t          isid_del;
    qcl_qce_t            *qce;
    vtss_qce_id_t        next_id;
    BOOL                 found, flag;
    BOOL                 old_conflict;

    T_N("enter, user id: %d, qcl: %u, qce: %u", user_id, qcl_id, qce_id);

    /* Convert qcl_id from 1-based to 0-based and check that it is valid */
    if (qcl_id) {
        qcl_id--;
    }

    if (qcl_id >= (QCL_MAX + RESERVED_QCL_CNT)) {
        T_W("Illegal qcl_id: %u", qcl_id + 1);
        return QOS_ERROR_PARM;
    }

    /* Check user ID */
    if (user_id >= QCL_USER_CNT) {
        T_W("user_id: %d not exist", user_id);
        T_D("exit");
        return QOS_ERROR_QCL_USER_NOT_FOUND;
    }

    /* Check stack role */
    if (qcl_mgmt_isid_invalid(conf->isid)) {
        T_D("exit");
        return QOS_ERROR_STACK_STATE;
    }

    /* Convert to combined ID */
    conf->id = QCL_COMBINED_ID_SET(user_id, conf->id);
    qce_id = QCL_COMBINED_ID_SET(user_id, qce_id);
    if (conf->isid == VTSS_ISID_LOCAL) {
        QOS_CRIT_ENTER();
        list = &qos.qcl_qce_switch_list[qcl_id];
        rc = qos_qce_entry_add(list, &qce_id, conf, &isid_del,
                               &found, &old_conflict, 1);
        if (rc == VTSS_OK) {
            /* Set this entry to ASIC layer */
            /* Next_id (i.e. qce_id) should exist in ASIC i.e. if qce_id is not
               QCE_ID_NONE, find the first qce_id starting from the mentioned
               id with non-conflict status */
            if (qce_id != QCE_ID_NONE) {
                flag = FALSE;
                next_id = QCE_ID_NONE;
                for (qce = list->qce_used_list; qce != NULL; qce = qce->next) {
                    if (!flag && qce->conf.id == qce_id) {
                        flag = TRUE;
                    }
                    if (flag && !qce->conf.conflict) {
                        next_id = qce->conf.id;
                        break;
                    }
                }
                qce_id = next_id;
            }

            if ((rc = qce_entry_add(qcl_id, qce_id, conf)) == VTSS_OK) {
                conf->conflict = FALSE;
            } else {
                /* Check if the entry already exists with non-conflict status.
                   In that case if add fails, old entry should be deleted */
                if (found && !old_conflict) {
                    if (vtss_qce_del(NULL, qcl_id, conf->id) != VTSS_OK) {
                        T_W("Calling vtss_qce_del() failed\n");
                    }
                }

                conf->conflict = TRUE;
            }
            qcl_qce_conflict_set(qcl_id, conf->id, conf->conflict);
        }
        QOS_CRIT_EXIT();

        /* Restore independent QCE ID */
        conf->id = QCL_QCE_ID_GET(conf->id);

        T_D("exit");
        return rc;
    } else {
        QOS_CRIT_ENTER();
        list = &qos.qcl_qce_stack_list[qcl_id];
        rc = qos_qce_entry_add(list, &qce_id, conf, &isid_del,
                               &found, &old_conflict, 0);
        if (rc == VTSS_OK && user_id == QCL_USER_STATIC) {
            rc = qos_qcl_conf_commit();
        }
        if (rc == VTSS_OK && isid_del != VTSS_ISID_LOCAL) {
            rc = qce_entry_stack_del(isid_del, qcl_id, conf->id);
        }
        if (rc == VTSS_OK) {
            rc = qce_entry_stack_add(conf->isid, qcl_id, conf, TRUE);
        }

        /* Restore independent QCE ID */
        conf->id = QCL_QCE_ID_GET(conf->id);
        QOS_CRIT_EXIT();

        return rc;
    }
}

/* Delete QCE in the specific QCL */
vtss_rc qos_mgmt_qce_entry_del(vtss_isid_t isid, qcl_user_t user_id,
                               vtss_qcl_id_t qcl_id, vtss_qce_id_t qce_id)
{
    qcl_qce_list_table_t *list;
    vtss_rc               rc;
    vtss_isid_t           isid_del;
    BOOL                  conflict = TRUE;

    T_N("enter, user id:%d,  qcl: %u, qce: %u", user_id, qcl_id, qce_id);

    /* Check user ID */
    if (user_id >= QCL_USER_CNT) {
        T_W("user_id: %d not exist", user_id);
        T_D("exit");
        return QOS_ERROR_QCL_USER_NOT_FOUND;
    }

    /* Check QCE ID */
    if (qce_id > QCE_ID_END) {
        T_W("id: %d out of range", qce_id);
        T_D("exit");
        return QOS_ERROR_PARM;
    }

    /* Check stack role */
    if (qcl_mgmt_isid_invalid(isid)) {
        T_D("exit");
        return QOS_ERROR_STACK_STATE;
    }

    /* Convert qcl_id from 1-based to 0-based and check that it is valid */
    if (qcl_id) {
        qcl_id--;
    }
    if (qcl_id >= (QCL_MAX + RESERVED_QCL_CNT)) {
        T_W("Illegal qcl_id: %u", qcl_id + 1);
        return QOS_ERROR_PARM;
    }

    /* Convert to combined ID */
    qce_id = QCL_COMBINED_ID_SET(user_id, qce_id);
    if (isid == VTSS_ISID_LOCAL) {
        /* delete from local list */
        QOS_CRIT_ENTER();
        list = &qos.qcl_qce_switch_list[qcl_id];
        rc = qos_qce_entry_del(list, qce_id, &isid_del, &conflict);

        if (rc == VTSS_OK && conflict == FALSE) {
            /* Remove this entry from ASIC layer */
            if ((rc = vtss_qce_del(NULL, qcl_id, qce_id)) != VTSS_OK) {
                T_W("Calling vtss_qce_del() failed\n");
            }
            qcl_mgmt_conflict_solve(qcl_id);
        }
        QOS_CRIT_EXIT();
    } else {
        /* delete from global list */
        QOS_CRIT_ENTER();
        list = &qos.qcl_qce_stack_list[qcl_id];
        rc = qos_qce_entry_del(list, qce_id, &isid_del, &conflict);

        if (rc == VTSS_OK && user_id == QCL_USER_STATIC) {
            rc = qos_qcl_conf_commit();
        }

        if (rc == VTSS_OK) {
            rc = qce_entry_stack_del(isid_del, qcl_id, qce_id);
        }
        QOS_CRIT_EXIT();
    }

    T_N("exit");
    return rc;
}

/* Resolve QCE conflict */
vtss_rc qos_mgmt_qce_conflict_resolve(vtss_isid_t isid, qcl_user_t user_id,
                                      vtss_qcl_id_t qcl_id)
{
    vtss_rc      rc = VTSS_OK;

    T_N("enter, user id:%d,  qcl: %u", user_id, qcl_id);

    /* Check user ID */
    if (user_id >= QCL_USER_CNT) {
        T_W("user_id: %d not exist", user_id);
        T_D("exit");
        return QOS_ERROR_QCL_USER_NOT_FOUND;
    }

    /* Check stack role */
    if (qcl_mgmt_isid_invalid(isid)) {
        T_D("exit");
        return QOS_ERROR_STACK_STATE;
    }

    /* Convert qcl_id from 1-based to 0-based and check that it is valid */
    if (qcl_id) {
        qcl_id--;
    }
    if (qcl_id >= (QCL_MAX + RESERVED_QCL_CNT)) {
        T_W("Illegal qcl_id: %u", qcl_id + 1);
        return QOS_ERROR_PARM;
    }

    QOS_CRIT_ENTER();
    if (isid == VTSS_ISID_LOCAL) {
        qcl_mgmt_conflict_solve(qcl_id);
    } else {
        rc = qce_entry_stack_conflict_resolve(isid, qcl_id);
    }
    QOS_CRIT_EXIT();

    T_N("exit");
    return rc;
}
#endif /* VTSS_FEATURE_QCL_V2 */

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
static void qos_lport_build_hw_conf(vtss_qos_lport_conf_t *const hw_conf, const qos_lport_conf_t *const conf)
{
    u8 tmp_queue_cnt;

    /* Build HW entry */
    hw_conf->shaper = conf->shaper;
    hw_conf->scheduler.rate = conf->scheduler.rate;
    hw_conf->lport_pct = conf->pct;
    for (tmp_queue_cnt = 0; tmp_queue_cnt < VTSS_PRIOS; tmp_queue_cnt++) {
        hw_conf->red[tmp_queue_cnt].enable = conf->red[tmp_queue_cnt].enable;
        hw_conf->red[tmp_queue_cnt].max_th = conf->red[tmp_queue_cnt].max_th;
        hw_conf->red[tmp_queue_cnt].min_th = conf->red[tmp_queue_cnt].min_th;
        hw_conf->red[tmp_queue_cnt].max_prob_1 = conf->red[tmp_queue_cnt].max_prob_1;
        hw_conf->red[tmp_queue_cnt].max_prob_2 = conf->red[tmp_queue_cnt].max_prob_2;
        hw_conf->red[tmp_queue_cnt].max_prob_3 = conf->red[tmp_queue_cnt].max_prob_3;
    }
    for (tmp_queue_cnt = 0; tmp_queue_cnt < 6; tmp_queue_cnt++) {
        hw_conf->scheduler.queue_pct[tmp_queue_cnt] = conf->scheduler.queue_pct[tmp_queue_cnt];
    }
    for (tmp_queue_cnt = 0; tmp_queue_cnt < 2; tmp_queue_cnt++) {
        hw_conf->scheduler.queue[tmp_queue_cnt] = conf->scheduler.queue[tmp_queue_cnt];
    }
    if (conf->shaper_host_status == FALSE) {
        hw_conf->shaper.rate = 0xffffffff; /* Disable the Host shaper */
    }
    for (tmp_queue_cnt = 0; tmp_queue_cnt < 2; tmp_queue_cnt++) {
        if (conf->scheduler.shaper_queue_status[tmp_queue_cnt] == FALSE) {
            hw_conf->scheduler.queue[tmp_queue_cnt].rate = 0xffffffff;
        }
    }
}

static vtss_rc qos_lport_conf_set(vtss_lport_no_t lport_no, const qos_lport_conf_t *const conf)
{
    vtss_qos_lport_conf_t hw_conf;

    qos.qos_lport[lport_no] = *conf;
    qos_lport_build_hw_conf(&hw_conf, conf);
    return vtss_qos_lport_conf_set(0, lport_no, &hw_conf);
}

vtss_rc qos_mgmt_lport_conf_set(vtss_lport_no_t lport_no, const qos_lport_conf_t *const conf)
{
    vtss_rc               rc = VTSS_RC_OK;

    QOS_CRIT_ENTER();
    do {
        qos.qos_lport[lport_no] = *conf;
        rc = qos_lport_conf_set(lport_no, conf);
        if (rc != VTSS_RC_OK) {
            break;
        }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        {
            /* Save new data to conf module */
            qos_lport_conf_blk_t  *qos_lport_blk = NULL;
            if ((qos_lport_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_QOS_LPORT_TABLE, NULL)) == NULL) {
                QOS_CRIT_EXIT();
                return VTSS_RC_ERROR;
            }
            qos_lport_blk->conf[lport_no] = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_QOS_LPORT_TABLE);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    } while (0); /* end of do-while(0) */

    QOS_CRIT_EXIT();

    return rc;
}

vtss_rc qos_mgmt_lport_conf_get(vtss_lport_no_t lport_no, qos_lport_conf_t *const conf)
{
    vtss_rc rc = VTSS_RC_OK;

    QOS_CRIT_ENTER();
    *conf = qos.qos_lport[lport_no];
    QOS_CRIT_EXIT();

    return rc;
}

static void ce_max_event_cb(const ce_max_cb_events_t events, void *data)
{
    qos_lport_conf_blk_t  *qos_lport_blk = NULL;
    BOOL                  mode_change = FALSE;

    if (events & CE_MAX_CB_EVENT_MODE_CHANGE) {
        mode_change = TRUE;
    } else if (events & CE_MAX_CB_EVENT_NULLIFY_MODE_CHANGE) {
        mode_change = FALSE;
    } else {
        return;
    }
    QOS_CRIT_ENTER();
    /* Save new data to conf module */
    if ((qos_lport_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_QOS_LPORT_TABLE, NULL)) == NULL) {
        QOS_CRIT_EXIT();
        return;
    }
    qos_lport_blk->mode_change = mode_change; /* Remember that mode change occured */
    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_QOS_LPORT_TABLE);
    QOS_CRIT_EXIT();
}


#endif /* VTSS_ARCH_JAGAUR_1_CE_MAC */


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/
/* ================================================================= *
 *  If VTSS_QOS_SILENT_UPGRADE is defined then enable silent upgrade
 *  of qos configuration from 2.80 to current version.
 *
 *  If NOT defined then create defaults as usual.
 *
 *  Using a separate define makes it possible to test this feature
 *  without requiring VTSS_SW_OPTION_SILENT_UPGRADE to be defined.
 * ================================================================= */
#define VTSS_QOS_SILENT_UPGRADE
#if defined(VTSS_QOS_SILENT_UPGRADE)

/* ================================================================= *
 * Old port flash configuration layout - used for silent upgrade.
 * Difference: 'BOOL dmac_dip' (default FALSE) has been added at
 * bottom of new layout.
 * ================================================================= */
typedef struct {
    vtss_prio_t            default_prio;
    vtss_tagprio_t         usr_prio;
    qos_policer_t          port_policer[VTSS_PORT_POLICERS];
#if defined(VTSS_FEATURE_QOS_PORT_POLICER_EXT)
    vtss_policer_ext_t     port_policer_ext[VTSS_PORT_POLICERS];
#endif /* VTSS_FEATURE_QOS_PORT_POLICER_EXT */
#if defined(VTSS_SW_OPTION_BUILD_CE)
#ifdef VTSS_FEATURE_QOS_QUEUE_POLICER
    qos_policer_t          queue_policer[QOS_PORT_QUEUE_CNT];
#endif /* VTSS_FEATURE_QOS_QUEUE_POLICER */
#endif /* defined(VTSS_SW_OPTION_BUILD_CE) */
    vtss_bitrate_t         shaper_rate;
    BOOL                   shaper_status;
#ifdef VTSS_FEATURE_QOS_CLASSIFICATION_V2
    vtss_dp_level_t        default_dpl;
    vtss_dei_t             default_dei;
    BOOL                   tag_class_enable;
    vtss_prio_t            qos_class_map[VTSS_PCP_ARRAY_SIZE][VTSS_DEI_ARRAY_SIZE];
    vtss_dp_level_t        dp_level_map[VTSS_PCP_ARRAY_SIZE][VTSS_DEI_ARRAY_SIZE];
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
    BOOL                   dscp_class_enable;
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_CLASSIFICATION_V2 */
#ifdef VTSS_FEATURE_QOS_WFQ_PORT
    BOOL                   wfq_enable;
    vtss_weight_t          weight[QOS_PORT_QUEUE_CNT];
#endif /* VTSS_FEATURE_QOS_WFQ_PORT */
#ifdef VTSS_FEATURE_QOS_SCHEDULER_V2
    BOOL                   dwrr_enable;
    vtss_pct_t             queue_pct[QOS_PORT_WEIGHTED_QUEUE_CNT];
#endif /* VTSS_FEATURE_QOS_SCHEDULER_V2 */
#ifdef VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS
    qos_shaper_t           queue_shaper[QOS_PORT_QUEUE_CNT];
    BOOL                   excess_enable[QOS_PORT_QUEUE_CNT];
#endif /* VTSS_FEATURE_QOS_EGRESS_QUEUE_SHAPERS */
#ifdef VTSS_FEATURE_QOS_TAG_REMARK_V2
    vtss_tag_remark_mode_t tag_remark_mode;
    vtss_tagprio_t         tag_default_pcp;
    vtss_dei_t             tag_default_dei;
    vtss_dei_t             tag_dp_map[QOS_PORT_TR_DPL_CNT];
    vtss_tagprio_t         tag_pcp_map[QOS_PORT_PRIO_CNT][2];
    vtss_dei_t             tag_dei_map[QOS_PORT_PRIO_CNT][2];
#endif /* VTSS_FEATURE_QOS_TAG_REMARK_V2 */
#ifdef VTSS_FEATURE_QOS_DSCP_REMARK
#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
#if defined(VTSS_FEATURE_QOS_DSCP_REMARK_V2)
    BOOL                   dscp_translate;
    vtss_dscp_mode_t       dscp_imode;
    vtss_dscp_emode_t      dscp_emode;
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK_V2 */
#endif /* defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE) */
#endif /* VTSS_FEATURE_QOS_DSCP_REMARK */
#if defined(VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER)
    vtss_qos_port_dot3ar_rate_limiter_t tx_rate_limiter;
#endif /* VTSS_FEATURE_QOS_DOT3AR_RATE_LIMITER */
} qos_port_conf_t_old;

typedef struct {
    ulong               version;
    qos_port_conf_t_old conf[VTSS_ISID_CNT][VTSS_PORTS];
} qos_port_conf_blk_t_old;

/* ================================================================= *
 *  QOS_port_conf_flash_upgrade()
 *  Try to upgrade from old configuration layout.
 *  Returns a (malloc'ed) pointer to the upgraded new configuration
 *  or NULL if conversion failed.
 * ================================================================= */
static qos_port_conf_blk_t *QOS_port_conf_flash_upgrade(const void *blk, size_t size)
{
    qos_port_conf_blk_t *new_blk = NULL;

    if (size == sizeof(qos_port_conf_blk_t_old)) {
        if ((new_blk = VTSS_MALLOC(sizeof(*new_blk)))) {
            qos_port_conf_blk_t_old *old_blk = (qos_port_conf_blk_t_old *)blk;
            int s, p;

            memset(new_blk, 0, sizeof(*new_blk));
            new_blk->version = QOS_PORT_CONF_BLK_VERSION;
            for (s = 0; s < VTSS_ISID_CNT; s++) {
                for (p = 0; p < VTSS_PORTS; p++) {
                    memcpy(&new_blk->conf[s][p], &old_blk->conf[s][p], sizeof(old_blk->conf[s][p]));
                }
            }
        }
    }
    return new_blk;
}

/* ================================================================= *
 * Old qcl flash configuration layout - used for silent upgrade.
 * Difference: 'smac' field in 'qos_qce_key_t' has been changed from
 * 'vtss_qce_u24_t' to 'vtss_qce_u48_t'.
 * ================================================================= */
#ifdef VTSS_FEATURE_QCL_V2
/* QCE Key */
typedef struct {
    u16                        key_bits;
    qos_qce_vr_u16_t           vid;
    vtss_qce_u8_t              pcp;
    vtss_qce_u24_t             smac;

    union {
        /* Type VTSS_QCE_TYPE_ETYPE */
        struct {
            vtss_qce_u16_t     etype;
        } etype;

        /* Type VTSS_QCE_TYPE_LLC */
        struct {
            vtss_qce_u8_t      dsap;
            vtss_qce_u8_t      ssap;
            vtss_qce_u8_t      control;
        } llc;

        /* Type VTSS_QCE_TYPE_SNAP */
        struct {
            vtss_qce_u16_t     pid;
        } snap;

        /* Type VTSS_QCE_TYPE_IPV4 */
        struct {
            qos_qce_vr_u8_t    dscp;
            vtss_qce_u8_t      proto;
            vtss_qce_ip_t      sip;
            qos_qce_vr_u16_t   sport;
            qos_qce_vr_u16_t   dport;
        } ipv4;

        /* Type VTSS_QCE_TYPE_IPV6 */
        struct {
            qos_qce_vr_u8_t    dscp;
            vtss_qce_u8_t      proto;
            vtss_qce_u32_t     sip;
            qos_qce_vr_u16_t   sport;
            qos_qce_vr_u16_t   dport;
        } ipv6;
    } frame;
} qos_qce_key_t_old;
#endif /* VTSS_FEATURE_QCL_V2 */

typedef struct {
    vtss_qce_id_t     id;
    vtss_qce_type_t   type;
#ifdef VTSS_FEATURE_QCL_V2
    BOOL              conflict;
    vtss_isid_t       isid;
    u8                port_list[VTSS_PORT_BF_SIZE];
    qos_qce_key_t_old key;
    qos_qce_action_t  action;
#endif
} qos_qce_entry_conf_t_old;

/* QOS QCL QCE configuration block */
typedef struct {
    ulong                    version;
    ulong                    count[QCL_MAX + RESERVED_QCL_CNT];
    ulong                    size[QCL_MAX + RESERVED_QCL_CNT];
    qos_qce_entry_conf_t_old table[(QCL_MAX + RESERVED_QCL_CNT) * QCE_ID_END];
} qcl_qce_blk_t_old;

/* ================================================================= *
 *  QOS_qce_conf_flash_upgrade()
 *  Try to upgrade from old configuration layout.
 *  Returns a (malloc'ed) pointer to the upgraded new configuration
 *  or NULL if conversion failed.
 * ================================================================= */
static qcl_qce_blk_t *QOS_qce_conf_flash_upgrade(const void *blk, size_t size)
{
    qcl_qce_blk_t *new_blk = NULL;

    if (size == sizeof(qcl_qce_blk_t_old)) {
        if ((new_blk = VTSS_MALLOC(sizeof(*new_blk)))) {
            qcl_qce_blk_t_old *old_blk = (qcl_qce_blk_t_old *)blk;
            int q;

            memset(new_blk, 0, sizeof(*new_blk));
            new_blk->version = QOS_QCL_QCE_BLK_VERSION;
            for (q = 0; q < (QCL_MAX + RESERVED_QCL_CNT); q++) {
                new_blk->count[q] = old_blk->count[q];
                new_blk->size[q] = old_blk->size[q];
            }
            for (q = 0; q < ((QCL_MAX + RESERVED_QCL_CNT) * QCE_ID_END); q++) {
                new_blk->table[q].id = old_blk->table[q].id;
                new_blk->table[q].type = old_blk->table[q].type;
#ifdef VTSS_FEATURE_QCL_V2
                {
                    int i;
                    new_blk->table[q].conflict = old_blk->table[q].conflict;
                    new_blk->table[q].isid = old_blk->table[q].isid;
                    for (i = 0; i < VTSS_PORT_BF_SIZE; i++) {
                        new_blk->table[q].port_list[i] = old_blk->table[q].port_list[i];
                    }
                    new_blk->table[q].key.key_bits = old_blk->table[q].key.key_bits;
                    new_blk->table[q].key.vid = old_blk->table[q].key.vid;
                    new_blk->table[q].key.pcp = old_blk->table[q].key.pcp;
                    for (i = 0; i < 3; i++) {
                        new_blk->table[q].key.smac.value[i] = old_blk->table[q].key.smac.value[i];
                        new_blk->table[q].key.smac.mask[i] = old_blk->table[q].key.smac.mask[i];
                    }
                    memcpy(&new_blk->table[q].key.frame, &old_blk->table[q].key.frame, sizeof(old_blk->table[q].key.frame));
                    new_blk->table[q].action = old_blk->table[q].action;
                }
#endif
            }
        }
    }
    return new_blk;
}
#endif /* defined(VTSS_QOS_SILENT_UPGRADE) */

// Read/create QoS global configuration
static void QOS_global_conf_flash_read(vtss_isid_t isid_add, BOOL force_defaults)
{
    qos_conf_blk_t  *qos_blk;
    ulong           size;
    BOOL            do_create = force_defaults;

    if (misc_conf_read_use()) {
        // Open or create configuration block
        if ((qos_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_QOS_TABLE, &size)) == NULL || (size != sizeof(qos_conf_blk_t))) {
            T_W("isid:%d, conf_sec_open() failed or size mismatch, creating defaults", isid_add);
            qos_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_QOS_TABLE, sizeof(qos_conf_blk_t));
            do_create = TRUE;
        } else if (qos_blk->version != QOS_CONF_BLK_VERSION) {
            T_W("isid:%d, Version mismatch, creating defaults", isid_add);
            do_create = TRUE;
        }
    } else {
        qos_blk   = NULL;
        do_create = TRUE;
    }

    // Verify existing configuration
    if (!do_create && qos_blk && (qos_conf_check(&qos_blk->conf) != VTSS_OK)) {
        T_W("isid:%d, Invalid configuration, creating defaults", isid_add);
        do_create = TRUE;
    }

    if (do_create) { // Create new default configuration
        qos_conf_default_set(&qos.qos_conf);
        if (qos_blk) {
            qos_blk->conf = qos.qos_conf;
        }
    } else { // Read current configuration
        if (qos_blk) { // Make lint happy. It is never NULL here
            qos.qos_conf = qos_blk->conf;
        }
    }

    if (force_defaults) {
        (void) qos_stack_conf_set(VTSS_ISID_GLOBAL); // Apply this configuration to the whole stack
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (qos_blk) {
        qos_blk->version = QOS_CONF_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_QOS_TABLE);
    } else {
        T_W("failed to open flash configuration");
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/* Read/create QoS port configuration */
static void QOS_port_conf_flash_read(vtss_isid_t isid_add, BOOL force_defaults)
{
    qos_port_conf_blk_t *qos_port_blk;
    ulong               size = 0;
    BOOL                do_create = force_defaults;
    switch_iter_t       sit;
    port_iter_t         pit;
    qos_port_conf_t     qos_port_conf_new;

    if (misc_conf_read_use()) {
        // Open or create configuration block
        if ((qos_port_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_QOS_PORT_TABLE, &size)) == NULL) {
            T_W("isid:%d, conf_sec_open() failed, creating defaults", isid_add);
            qos_port_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_QOS_PORT_TABLE, sizeof(qos_port_conf_blk_t));
            do_create = TRUE;
        } else if (size != sizeof(qos_port_conf_blk_t)) {
#if defined(VTSS_QOS_SILENT_UPGRADE)
            qos_port_conf_blk_t *new_blk;
            T_I("size mismatch, try upgrade");
            new_blk = QOS_port_conf_flash_upgrade(qos_port_blk, size);
            qos_port_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_QOS_PORT_TABLE, sizeof(qos_port_conf_blk_t));
            if (new_blk && qos_port_blk) {
                T_I("upgrade ok");
                *qos_port_blk = *new_blk;
            } else {
                T_W("upgrade failed, creating defaults");
                do_create = TRUE;
            }
            if (new_blk) {
                VTSS_FREE(new_blk);
            }
#else
            T_W("size mismatch, creating defaults");
            qos_port_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_QOS_PORT_TABLE, sizeof(qos_port_conf_blk_t));
            do_create = TRUE;
#endif /* defined(VTSS_QOS_SILENT_UPGRADE) */
        } else if (qos_port_blk->version != QOS_PORT_CONF_BLK_VERSION) {
            T_W("isid:%d, Version mismatch, creating defaults", isid_add);
            do_create = TRUE;
        }
    } else {
        qos_port_blk = NULL;
        do_create    = TRUE;
    }

    // Verify existing configuration
    if (!do_create && qos_port_blk) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
        (void) port_iter_init(&pit, &sit, VTSS_ISID_GLOBAL, PORT_ITER_SORT_ORDER_IPORT_ALL, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            if (qos_port_conf_check(&qos_port_blk->conf[sit.isid - VTSS_ISID_START][pit.iport - VTSS_PORT_NO_START]) != VTSS_OK) {
                T_W("isid:%d, port:%d, Invalid configuration, creating defaults", sit.isid, pit.uport);
                do_create = TRUE;
                break;
            }
        }
    }

    if (do_create) { // Create new default configuration
        qos_port_conf_default_set(&qos_port_conf_new);
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
        (void) port_iter_init(&pit, &sit, VTSS_ISID_GLOBAL, PORT_ITER_SORT_ORDER_IPORT_ALL, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            qos.qos_port_conf[sit.isid][pit.iport - VTSS_PORT_NO_START] = qos_port_conf_new;
            if (qos_port_blk) {
                qos_port_blk->conf[sit.isid - VTSS_ISID_START][pit.iport - VTSS_PORT_NO_START] = qos_port_conf_new;
            }
        }
    } else { // Read current configuration
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
        (void) port_iter_init(&pit, &sit, VTSS_ISID_GLOBAL, PORT_ITER_SORT_ORDER_IPORT_ALL, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            if (qos_port_blk) { // Make lint happy. It is never NULL here
                qos.qos_port_conf[sit.isid][pit.iport - VTSS_PORT_NO_START] = qos_port_blk->conf[sit.isid - VTSS_ISID_START][pit.iport - VTSS_PORT_NO_START];
            }
        }
    }

    if (force_defaults) {
        (void) qos_stack_port_conf_set(VTSS_ISID_GLOBAL, VTSS_PORT_NO_NONE); // Apply this configuration to the whole stack
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (qos_port_blk) {
        qos_port_blk->version = QOS_PORT_CONF_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_QOS_PORT_TABLE);
    } else {
        T_W("failed to open flash configuration");
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/* Read/create QoS QCE configuration */
static void QOS_qce_conf_flash_read(vtss_isid_t isid_add, BOOL force_defaults)
{
    qcl_qce_blk_t *qce_blk = NULL;
    qcl_qce_t     *qce, *prev;
    int           i, j;
    ulong         size = 0;
    BOOL          do_create = force_defaults;

    if (misc_conf_read_use()) {
        /* Read/create QCL QCE configuration */
        if (!do_create) {
            if ((qce_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_QOS_QCL_QCE_TABLE, &size)) == NULL) {
                T_W("conf_sec_open failed, creating defaults");
                do_create = TRUE;
            } else if (size != sizeof(qcl_qce_blk_t)) {
#if defined(VTSS_QOS_SILENT_UPGRADE)
                qcl_qce_blk_t *new_blk;
                T_I("size mismatch, try upgrade");
                new_blk = QOS_qce_conf_flash_upgrade(qce_blk, size);
                qce_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_QOS_QCL_QCE_TABLE, sizeof(qcl_qce_blk_t));
                if (new_blk && qce_blk) {
                    T_I("upgrade ok");
                    *qce_blk = *new_blk;
                } else {
                    T_W("upgrade failed, creating defaults");
                    do_create = TRUE;
                }
                if (new_blk) {
                    VTSS_FREE(new_blk);
                }
#else
                T_W("size mismatch, creating defaults");
                do_create = TRUE;
#endif /* defined(VTSS_QOS_SILENT_UPGRADE) */
            } else if (qce_blk->version != QOS_QCL_QCE_BLK_VERSION) {
                T_W("version mismatch, creating defaults");
                do_create = TRUE;
            }
        }

        if (do_create) {
            if ((qce_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_QOS_QCL_QCE_TABLE, sizeof(qcl_qce_blk_t))) != NULL) {
                qce_blk->version = QOS_QCL_QCE_BLK_VERSION;
                for (i = 0; i < QCL_MAX + RESERVED_QCL_CNT; i++) {
                    qce_blk->count[i] = 0;
                    qce_blk->size[i] = sizeof(qos_qce_entry_conf_t);
                }
            }
        }

        if (qce_blk) {
            if (force_defaults) {
                /* apply this configuration to the whole stack */
                (void) qos_stack_qcl_clear(VTSS_ISID_GLOBAL);
            }

            /* and Add new QCEs for each QCLs */
            for (i = 0; i < QCL_MAX + RESERVED_QCL_CNT; i++) {
                for (qce = qos.qcl_qce_stack_list[i].qce_used_list, prev = NULL; qce != NULL; prev = qce, qce = qce->next) {
                }
                /* prev now points to the last entry in qce_used_list (if any) */
                /* Insert qce_used_list to front of qce_free_list and mark qce_used_list */
                if (prev != NULL) {
                    prev->next = qos.qcl_qce_stack_list[i].qce_free_list;
                    qos.qcl_qce_stack_list[i].qce_free_list = qos.qcl_qce_stack_list[i].qce_used_list;
                    qos.qcl_qce_stack_list[i].qce_used_list = NULL;
                }

                /* Add new QCEs into the specific QCL */
                for (j = qce_blk->count[i]; j != 0; j--) {
                    /* Move entry from free list to used list */
                    if ((qce = qos.qcl_qce_stack_list[i].qce_free_list) == NULL) {
                        break;
                    }
                    qos.qcl_qce_stack_list[i].qce_free_list = qce->next;
                    qce->next = qos.qcl_qce_stack_list[i].qce_used_list;
                    qos.qcl_qce_stack_list[i].qce_used_list = qce;
                    qce->conf = qce_blk->table[(i * QCE_MAX) + j - 1];
                    if (force_defaults) {
                        /* apply this configuration to the whole stack */
#if defined(VTSS_FEATURE_QCL_V2)
                        vtss_port_no_t iport;

                        /* Remove the stack port from QCE */
                        /* TO DO: there is no function to check stack port in slave from
                           master, so we are using PORT_NO_IS_STACK to check stack port,
                           but use the appropriate function/macro once available! */
                        for (iport = VTSS_PORT_NO_START;
                             iport < VTSS_PORT_NO_END; iport++) {
                            if (VTSS_PORT_BF_GET(qce->conf.port_list, iport) &&
                                PORT_NO_IS_STACK(iport)) {
                                VTSS_PORT_BF_SET(qce->conf.port_list, iport, 0);
                            }
                        }
                        (void) qce_entry_stack_add(qce->conf.isid, i, &qce->conf, TRUE);
#endif
                    }
                }
            }
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_QOS_QCL_QCE_TABLE);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        } else {
            T_W("conf_sec_create fail, please do (sys restore default)after system booting done ");
        }
    } else {  // Not misc_conf_read_use(): Unconditionally load defaults
        /* apply this configuration to the whole stack */
        (void) qos_stack_qcl_clear(VTSS_ISID_GLOBAL);

        for (i = 0; i < QCL_MAX + RESERVED_QCL_CNT; i++) {
            for (qce = qos.qcl_qce_stack_list[i].qce_used_list, prev = NULL; qce != NULL; prev = qce, qce = qce->next) {
            }
            /* prev now points to the last entry in qce_used_list (if any) */
            /* Insert qce_used_list to front of qce_free_list and mark qce_used_list */
            if (prev != NULL) {
                prev->next = qos.qcl_qce_stack_list[i].qce_free_list;
                qos.qcl_qce_stack_list[i].qce_free_list = qos.qcl_qce_stack_list[i].qce_used_list;
                qos.qcl_qce_stack_list[i].qce_used_list = NULL;
            }
        }
    }
}

#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
/* Read/create QoS LPort configuration */
static void QOS_lport_conf_flash_read(vtss_isid_t isid_add, BOOL force_defaults)
{
    qos_lport_conf_blk_t *qos_lport_blk = NULL;
    ulong                size;
    BOOL                 do_create = force_defaults;

    /* Read/create LPort QoS configuration */
    if (!do_create) {
        if ((qos_lport_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_QOS_LPORT_TABLE, &size)) == NULL) {
            do_create = 1;
        } else {
            if ((size != (sizeof(qos_lport_conf_blk_t))) || (qos_lport_blk->version != QOS_LPORT_CONF_BLK_VERSION)) {
                do_create = 1;
                T_W("conf_sec_open size mismatch, creating defaults");
            }
        }
    }
    if (do_create) {
        if ((qos_lport_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_QOS_LPORT_TABLE, sizeof(qos_lport_conf_blk_t))) != NULL) {
            qos_lport_blk->version = QOS_LPORT_CONF_BLK_VERSION;
            qos_lport_conf_default_set(qos_lport_blk);
        }
    }

    if (qos_lport_blk) {
        qos_lport_conf_apply(qos_lport_blk);
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_QOS_LPORT_TABLE);
    } else {
        T_W("conf_sec_create fail, please do (sys restore default)after system booting done ");
    }
}
#endif /* defined(VTSS_ARCH_JAGUAR_1_CE_MAC) */

/* Read/create QoS related configurations */
static void qos_conf_read(BOOL force_defaults, vtss_isid_t isid_add)
{
    T_N("enter, force_defaults: %d, isid_add: %d", force_defaults, isid_add);

    QOS_CRIT_ENTER();

    QOS_global_conf_flash_read(isid_add, force_defaults);
    QOS_port_conf_flash_read(isid_add, force_defaults);
    QOS_qce_conf_flash_read(isid_add, force_defaults);
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    QOS_lport_conf_flash_read(isid_add, force_defaults);
#endif /* defined(VTSS_ARCH_JAGUAR_1_CE_MAC) */

    QOS_CRIT_EXIT();

    T_N("exit");
}

/* Initislize QOS port volatile default prio to undefined */
static void qos_port_volatile_default_prio_init(void)
{
    switch_iter_t sit;
    port_iter_t   pit;

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_ALL);
    (void) port_iter_init(&pit, &sit, VTSS_ISID_GLOBAL, PORT_ITER_SORT_ORDER_IPORT_ALL, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        qos.volatile_default_prio[sit.isid - VTSS_ISID_START][pit.iport - VTSS_PORT_NO_START] = QOS_PORT_PRIO_UNDEF;
    }
}

/* Module start */
static void qos_start(BOOL init)
{
    int       i, j;
    qcl_qce_t *qce;

    T_N("enter, init: %d", init);
    if (init) {
        /* init QCL related structs */
        /* Put all QCEs to the unused link list for all QCLs in the stack */
        for (i = 0; i < QCL_MAX + RESERVED_QCL_CNT; i++) {
            qos.qcl_qce_stack_list[i].qce_free_list = NULL;
#if defined(VTSS_FEATURE_QCL_V2)
            /* allocate the memory at the beginning for MAX QCE in stack list */
            if ((qos.qcl_qce_stack_list[i].qce_list = VTSS_MALLOC(sizeof(qcl_qce_t) * QCE_ID_END))) {
                for (j = 0; j < QCE_ID_END; j++) {
                    qce = qos.qcl_qce_stack_list[i].qce_list + j;
                    qce->next = qos.qcl_qce_stack_list[i].qce_free_list;
                    qos.qcl_qce_stack_list[i].qce_free_list = qce;
                }
            }
#endif
            qos.qcl_qce_stack_list[i].qce_used_list = NULL;
        }
        /* Put all QCEs to the unused link list for all QCLs in the local switch */
        for (i = 0; i < QCL_MAX + RESERVED_QCL_CNT; i++) {
            qos.qcl_qce_switch_list[i].qce_free_list = NULL;
#if defined(VTSS_FEATURE_QCL_V2)
            /* allocate the memory at the beginning for MAX QCE in switch list */
            if ((qos.qcl_qce_switch_list[i].qce_list = VTSS_MALLOC(sizeof(qcl_qce_t) * (QCE_MAX + RESERVED_QCE_CNT)))) {
                for (j = 0; j < (QCE_MAX + RESERVED_QCE_CNT); j++) {
                    qce = qos.qcl_qce_switch_list[i].qce_list + j;
                    qce->next = qos.qcl_qce_switch_list[i].qce_free_list;
                    qos.qcl_qce_switch_list[i].qce_free_list = qce;
                }
            }
#endif
            qos.qcl_qce_switch_list[i].qce_used_list = NULL;
        }

        /* Initialize message buffer pool. Initialize it to four entries, since that's what required in a SWITCH_ADD event */
        qos.request = msg_buf_pool_create(VTSS_MODULE_ID_QOS, "Request", 4, sizeof(qos_msg_req_t));

        /* Create mutex for critical regions */
        critd_init(&qos.qos_crit, "qos.crit", VTSS_MODULE_ID_QOS, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        QOS_CRIT_EXIT();

        /* Create mutex for change callback structure */
        critd_init(&qos.rt.crit, "qos.cb_crit", VTSS_MODULE_ID_QOS, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        QOS_CB_CRIT_EXIT();
    } else {
        qos_stack_register();
    }
    T_N("exit");
}

/* Initialize module */
vtss_rc qos_init(vtss_init_data_t *data)
{
    vtss_isid_t               isid = data->isid;
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    ce_max_cb_event_context_t cb_context;
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }
    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        qos_start(1);
#ifdef VTSS_SW_OPTION_ICFG
        if (qos_icfg_init() != VTSS_OK) {
            T_E("ICFG not initialized correctly");
        }
#endif
        break;
    case INIT_CMD_START:
        T_D("START");
        qos_start(0);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset global configuration */
            qos_conf_read(TRUE, isid);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");
        /* Initialize volatile default prio to default (disabled) */
        qos_port_volatile_default_prio_init();
        /* Read stack and switch configuration */
        qos_conf_read(FALSE, VTSS_ISID_GLOBAL);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply all configuration to switch */
        if (VTSS_ISID_LEGAL(isid)) {
            QOS_CRIT_ENTER();
            (void)qos_stack_conf_set(isid);
            (void)qos_stack_port_conf_set(isid, VTSS_PORT_NO_NONE);
            (void)qos_stack_qcl_clear(isid);
            (void)qos_stack_qcl_update(isid);
            QOS_CRIT_EXIT();
        }
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
        /* Register with CE_MAC module for host mode change event */
        cb_context.module_id = VTSS_MODULE_ID_QOS;
        cb_context.cb        = ce_max_event_cb;
        if (ce_max_mgmt_mode_change_event_register(&cb_context) != VTSS_RC_OK) {
            T_E("Unable to register with CE-MAX");
        }
#endif /* VTSS_ARCH_JAGUAR_1_CE_MAC */
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

