/*

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
#include "conf_api.h"
#include "msg_api.h"
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif /* VTSS_SWITCH_STACKABLE */
#include "misc_api.h"

#include "ipmc_lib.h"
#include "ipmc_lib_porting.h"

#include "ipmc_lib_conf.h"
#ifdef VTSS_SW_OPTION_VCLI
#include "ipmc_lib_cli.h"
#endif /* VTSS_SW_OPTION_VCLI */
#ifdef VTSS_SW_OPTION_ICFG
#include "ipmc_lib_icfg.h"
#endif /* VTSS_SW_OPTION_ICFG */


/* ************************************************************************ **
 *
 * Defines
 *
 * ************************************************************************ */
#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_IPMC_LIB


/* ************************************************************************ **
 *
 * Public data
 *
 * ************************************************************************ */


/* ************************************************************************ **
 *
 * Local data
 *
 * ************************************************************************ */
#if VTSS_TRACE_ENABLED
static vtss_trace_reg_t             ipmc_lib_trace_reg = {
    .module_id      = VTSS_MODULE_ID_IPMC_LIB,
    .name           = "IPMC_LIB",
    .descr          = "IPMC Library"
};
static vtss_trace_grp_t             ipmc_lib_trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name       = "default",
        .descr      = "Default",
        .lvl        = VTSS_TRACE_LVL_WARNING,
        .timestamp  = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name       = "crit",
        .descr      = "Critical regions",
        .lvl        = VTSS_TRACE_LVL_ERROR,
        .timestamp  = 1,
    }
};
#endif /* VTSS_TRACE_ENABLED */

static BOOL                         init_local_ipmc_profile = FALSE;
static struct {
    critd_t                         crit;

    ipmc_lib_configuration_t        configuration;

    /* Request message buffer pool */
    void                            *request_buf_pool;

    BOOL                            stackable;
    vtss_isid_t                     my_isid;
} ipmc_lib_global;

#if VTSS_TRACE_ENABLED
#define IPMC_LIB_ENTER()            critd_enter(&ipmc_lib_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define IPMC_LIB_EXIT()             critd_exit(&ipmc_lib_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define IPMC_LIB_ENTER()            critd_enter(&ipmc_lib_global.crit)
#define IPMC_LIB_EXIT()             critd_exit(&ipmc_lib_global.crit)
#endif /* VTSS_TRACE_ENABLED */

#define IPMC_LIB_STACKABLE          ipmc_lib_global.stackable
#define IPMC_LIB_ISID_INIT          0xFF
#define IPMC_LIB_ISID_ANY           0xFE
#define IPMC_LIB_ISID_ASYNC         0xFD
#define IPMC_LIB_ISID_TRANSIT       VTSS_ISID_START
#define IPMC_LIB_MY_ISID            ipmc_lib_global.my_isid


/* Allocate a message buffer */
static ipmc_lib_msg_req_t *ipmc_lib_msg_alloc(ipmc_lib_msg_id_t msg_id)
{
    ipmc_lib_msg_req_t *msg = msg_buf_pool_get(ipmc_lib_global.request_buf_pool);

    VTSS_ASSERT(msg);
    msg->msg_id = msg_id;
    return msg;
}

/* Release stack message buffer */
static void ipmc_lib_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    (void) msg_buf_pool_put(msg);
}

/* Transmit message buffer */
static void ipmc_lib_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
    msg_tx_adv(NULL, ipmc_lib_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_IPMC_LIB, isid, msg, len + MSG_TX_DATA_HDR_LEN(ipmc_lib_msg_req_t, req));
}

/* Receive stack message */
static BOOL ipmc_lib_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const u32 isid)
{
    ipmc_lib_msg_req_t          *msg;
    vtss_rc                     rc;
    BOOL                        mode, clr_cfg;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *profile, *p;
    ipmc_lib_grp_fltr_entry_t   entry, *q;
    ipmc_lib_rule_t             *r;
    ipmc_time_t                 exe_time_base, exe_time_diff;

    (void) ipmc_lib_time_curr_get(&exe_time_base);

    msg = (ipmc_lib_msg_req_t *)rx_msg;
    if (!msg) {
        return FALSE;
    }
    T_D("id: %d, len: %zd, isid: %u", msg->msg_id, len, isid);

    if (msg_switch_is_master()) {
        return TRUE;
    }

    rc = VTSS_RC_ERROR;
    switch ( msg->msg_id ) {
    case IPMCLIB_MSG_ID_PROFILE_CLEAR_REQ:
        clr_cfg = msg->req.clear_ctrl.clr_cfg;

        T_D("Clear Flag: %s", clr_cfg ? "TRUE" : "FALSE");

        if (clr_cfg) {
            if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
                break;
            }

            rc = VTSS_OK;
            profile = &pfm->profile;
            memset(profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
            memset(&entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));

            IPMC_LIB_ENTER();
            while ((p = ipmc_lib_fltr_profile_get_next(profile, FALSE)) != NULL) {
                memcpy(profile, p, sizeof(ipmc_lib_grp_fltr_profile_t));
                rc |= ipmc_lib_fltr_profile_set(IPMC_OP_DEL, profile);
            }

            while ((q = ipmc_lib_fltr_entry_get_next(&entry, FALSE)) != NULL) {
                memcpy(&entry, q, sizeof(ipmc_lib_grp_fltr_entry_t));
                rc |= ipmc_lib_fltr_entry_set(IPMC_OP_DEL, &entry);
            }
            IPMC_LIB_EXIT();
            IPMC_MEM_PROFILE_MGIVE(pfm);
        } else {
            rc = VTSS_OK;
        }

        T_D("Complete with RC=%d", rc);

        break;
    case IPMCLIB_MSG_ID_FLTR_STATE_SET_REQ:
        IPMC_LIB_ENTER();
        rc = ipmc_lib_profile_state_get(&mode);
        if ((rc == VTSS_OK) && (mode == msg->req.ctrl_state.mode)) {
            IPMC_LIB_EXIT();
            break;
        }

        mode = msg->req.ctrl_state.mode;
        rc = ipmc_lib_profile_state_set(mode);
        IPMC_LIB_EXIT();

        break;
    case IPMCLIB_MSG_ID_FLTR_ENTRY_SET_REQ:
        T_D("%s %s", ipmc_lib_op_action_txt(msg->req.pf_entry.action), msg->req.pf_entry.entry.name);

        IPMC_LIB_ENTER();
        rc = ipmc_lib_fltr_entry_set(msg->req.pf_entry.action, &msg->req.pf_entry.entry);
        IPMC_LIB_EXIT();

        break;
    case IPMCLIB_MSG_ID_FLTR_PROFILE_SET_REQ:
        T_D("%s %s", ipmc_lib_op_action_txt(msg->req.pf_data.action), msg->req.pf_data.data.name);

        if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
            break;
        }

        profile = &pfm->profile;
        memcpy(&profile->data, &msg->req.pf_data.data, sizeof(ipmc_lib_profile_t));

        IPMC_LIB_ENTER();
        if ((p = ipmc_lib_fltr_profile_get(profile, FALSE)) != NULL) {
            memcpy(profile->rule, p->rule, sizeof(p->rule));
        } else {
            u32 rdx;

            memset(profile->rule, 0x0, sizeof(profile->rule));
            for (rdx = 0; rdx < IPMC_LIB_FLTR_ENTRY_MAX_CNT; rdx++) {
                r = &profile->rule[rdx];

                r->idx = rdx;
                r->entry_index = IPMC_LIB_FLTR_RULE_IDX_INIT;
                r->next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
            }
        }

        rc = ipmc_lib_fltr_profile_set(msg->req.pf_data.action, profile);
        IPMC_LIB_EXIT();
        IPMC_MEM_PROFILE_MGIVE(pfm);

        break;
    case IPMCLIB_MSG_ID_FLTR_RULE_SET_REQ:
        T_D("%s PDX:%u/RDX:%u/EDX:%u/NDX:%u/%s/%s",
            ipmc_lib_op_action_txt(msg->req.pf_rule.action),
            msg->req.pf_rule.pf_idx,
            msg->req.pf_rule.rule.idx,
            msg->req.pf_rule.rule.entry_index,
            msg->req.pf_rule.rule.next_rule_idx,
            ipmc_lib_fltr_action_txt(msg->req.pf_rule.rule.action, IPMC_TXT_CASE_CAPITAL),
            msg->req.pf_rule.rule.log ? "LOG" : "BYPASS");

        IPMC_LIB_ENTER();
        rc = ipmc_lib_fltr_profile_rule_set(msg->req.pf_rule.action, msg->req.pf_rule.pf_idx, &msg->req.pf_rule.rule);
        IPMC_LIB_EXIT();

        break;
    default:
        T_E("illegal msg_id: %d", msg->msg_id);

        break;
    }

    (void) ipmc_lib_time_diff_get(TRUE, FALSE, "IPMC_LIB_MSG_RX", &exe_time_base, &exe_time_diff);

    if (rc == VTSS_OK) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static void ipmc_lib_conf_default(ipmc_lib_configuration_t *conf)
{
    u32                             i, j;
    ipmc_lib_conf_profile_t         *profile;
    ipmc_lib_conf_fltr_entry_t      *pf_entry;
    ipmc_lib_conf_fltr_profile_t    *pf_object;
    ipmc_lib_profile_t              *pf_data;
    ipmc_lib_rule_t                 *pf_rule;

    if (!conf) {
        return;
    }

    /* Use default configuration */
    memset(conf, 0x0, sizeof(ipmc_lib_configuration_t));
    profile = &conf->profile;
    profile->global_ctrl_state = IPMC_LIB_FLTR_PROFILE_DEF_STATE;

    for (i = 0; i < IPMC_LIB_FLTR_ENTRY_MAX_CNT; i++) {
        pf_entry = &profile->fltr_entry_pool[i];

        pf_entry->index = i + 1;
        pf_entry->version = IPMC_IP_VERSION_INIT;
    }

    for (i = 0; i < IPMC_LIB_FLTR_PROFILE_MAX_CNT; i++) {
        pf_object = &profile->fltr_profile_pool[i];
        pf_data = &pf_object->data;

        pf_data->index = i + 1;
        pf_data->rule_head_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
        pf_data->version = IPMC_IP_VERSION_INIT;

        for (j = 0; j < IPMC_LIB_FLTR_ENTRY_MAX_CNT; j++) {
            pf_rule = &pf_object->rule[j];

            pf_rule->idx = j;
            pf_rule->entry_index = IPMC_LIB_FLTR_RULE_IDX_INIT;
            pf_rule->next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
        }
    }
}

static BOOL _ipmc_lib_conf_copy(ipmc_lib_configuration_t *cfg_src, ipmc_lib_configuration_t *cfg_dst)
{
    if (cfg_src && cfg_dst) {
        memcpy(cfg_dst, cfg_src, sizeof(ipmc_lib_configuration_t));
    } else {
        T_W("Invalid IPMCLIB Configuration Block");
        return FALSE;
    }

    return TRUE;
}

static BOOL ipmc_lib_conf_transition(u32 blk_ver,
                                     u32 conf_sz,
                                     void *blk_conf,
                                     BOOL *new_blk,
                                     ipmc_lib_configuration_t *tar_conf)
{
    BOOL                        conf_reset, create_blk;
    ipmc_lib_configuration_t    *conf_blk;

    if (!blk_conf || !tar_conf) {
        return FALSE;
    }

    create_blk = conf_reset = FALSE;
    conf_blk = NULL;
    switch ( blk_ver ) {
    case 1:
    default:
        conf_blk = (ipmc_lib_configuration_t *)blk_conf;

        break;
    }

    if (blk_ver > IPMC_LIB_CONF_VERSION) {
        /* Down-grade is not expected, just reset the current configuration */
        create_blk = TRUE;
        conf_reset = TRUE;
    } else if (blk_ver < IPMC_LIB_CONF_VERSION) {
        /* Up-grade is allowed, do seamless transition */
        create_blk = TRUE;

        switch ( blk_ver ) {
        default:
            if (blk_ver == IPMC_LIB_CONF_VERINIT) {
                create_blk = FALSE;
            }

            conf_reset = TRUE;

            break;
        }
    } else {
        if (sizeof(ipmc_lib_configuration_t) != conf_sz) {
            create_blk = TRUE;
            conf_reset = TRUE;
        } else {
            if (!_ipmc_lib_conf_copy(conf_blk, tar_conf)) {
                return FALSE;
            }
        }
    }

    *new_blk = create_blk;
    if (conf_reset) {
        T_W("Creating IPMCLIB default configurations");
        ipmc_lib_conf_default(tar_conf);
    }

    return TRUE;
}

static ipmc_error_t ipmc_lib_conf_read(BOOL create)
{
    ipmc_lib_conf_blk_t         *blk = NULL;
    u32                         size = 0;
    ipmc_lib_configuration_t    *conf;
    BOOL                        do_create = FALSE, do_default = FALSE;
    ipmc_time_t                 exe_time_base, exe_time_diff;

    (void) ipmc_lib_time_curr_get(&exe_time_base);

    if (misc_conf_read_use()) {
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_IPMCLIB_CONF, &size)) == NULL) {
            T_W("conf_sec_open failed, creating IPMCLIB defaults");
            blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_IPMCLIB_CONF, sizeof(*blk));

            if (blk == NULL) {
                T_E("ipmc_lib_conf_read failed");
                return IPMC_ERROR_GEN;
            }

            blk->blk_version = IPMC_LIB_CONF_VERINIT;
            do_default = TRUE;
        } else {
#if ((defined(VTSS_SW_OPTION_SMB_IPMC) || defined(VTSS_SW_OPTION_MVR)) && defined(VTSS_SW_OPTION_IPMC_LIB))
            do_default = create;
#else
            do_default = TRUE;
#endif /* (VTSS_SW_OPTION_SMB_IPMC || VTSS_SW_OPTION_MVR) && VTSS_SW_OPTION_IPMC_LIB */
        }
    } else {
        do_create = TRUE;
    }

    IPMC_LIB_ENTER();
    if (!do_default  &&  blk) {
        BOOL    new_blk;
        u32     orig_conf_size = size - sizeof(blk->blk_version);

        conf = &ipmc_lib_global.configuration;
        new_blk = FALSE;
        if (ipmc_lib_conf_transition(blk->blk_version,
                                     orig_conf_size,
                                     (void *)&blk->ipmc_lib_conf,
                                     &new_blk, conf)) {
            if (!do_create) {
                do_create = new_blk;
            }
        } else {
            IPMC_LIB_EXIT();
            T_W("ipmc_lib_conf_transition failed");
            return IPMC_ERROR_GEN;
        }
    } else {
        conf = &ipmc_lib_global.configuration;
        ipmc_lib_conf_default(conf);
    }
    IPMC_LIB_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (do_create) {
        blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_IPMCLIB_CONF, sizeof(*blk));

        if (blk == NULL) {
            T_E("conf_sec_create failed");
            return IPMC_ERROR_GEN;
        }
    }

    if (blk) {  // Quiet lint
        IPMC_LIB_ENTER();
        blk->ipmc_lib_conf = ipmc_lib_global.configuration;
        IPMC_LIB_EXIT();

        blk->blk_version = IPMC_LIB_CONF_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_IPMCLIB_CONF);
    }
#else
    (void) do_create;  // Quiet lint
#endif

    (void) ipmc_lib_time_diff_get(TRUE, FALSE, "IPMCLIB_CONF", &exe_time_base, &exe_time_diff);

    return VTSS_RC_OK;
}

BOOL ipmc_lib_conf_sync(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    ipmc_lib_conf_blk_t *blk_ptr = NULL;
    u32                 blk_size = 0;

    if ((blk_ptr = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_IPMCLIB_CONF, &blk_size)) != NULL) {
        IPMC_LIB_ENTER();
        memcpy(&blk_ptr->ipmc_lib_conf, &ipmc_lib_global.configuration, sizeof(ipmc_lib_configuration_t));
        IPMC_LIB_EXIT();

        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_IPMCLIB_CONF);
    } else {
        return FALSE;
    }
#endif
    return TRUE;
}

static vtss_rc ipmc_lib_stacking_profile_entry_set(vtss_isid_t isid_add,
                                                   ipmc_operation_action_t action,
                                                   ipmc_lib_grp_fltr_entry_t *entry)
{
    ipmc_lib_msg_req_t  *msg;
    switch_iter_t       sit;
    vtss_isid_t         isid;

    if (!entry || !msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if (!IPMC_LIB_MSG_ISID_PASS_SLV(isid_add, isid)) {
            continue;
        }

        /* Allocate and send message */
        T_D("sending to isid %d %s %s", isid, ipmc_lib_op_action_txt(action), entry->name);
        msg = ipmc_lib_msg_alloc(IPMCLIB_MSG_ID_FLTR_ENTRY_SET_REQ);
        msg->req.pf_entry.action = action;
        memcpy(&msg->req.pf_entry.entry, entry, sizeof(ipmc_lib_grp_fltr_entry_t));
        ipmc_lib_msg_tx(msg, isid, sizeof(msg->req.pf_entry));
    }

    return VTSS_OK;
}

static vtss_rc ipmc_lib_stacking_profile_rule_set(vtss_isid_t isid_add,
                                                  ipmc_operation_action_t action,
                                                  u32 pdx,
                                                  ipmc_lib_rule_t *entry)
{
    ipmc_lib_msg_req_t  *msg;
    switch_iter_t       sit;
    vtss_isid_t         isid;

    if (!entry || !msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if (!IPMC_LIB_MSG_ISID_PASS_SLV(isid_add, isid)) {
            continue;
        }

        /* Allocate and send message */
        T_D("sending to isid %d %s PDX:%u/RDX:%u/EDX:%u/NDX:%u/%s/%s",
            isid, ipmc_lib_op_action_txt(action), pdx,
            entry->idx,
            entry->entry_index,
            entry->next_rule_idx,
            ipmc_lib_fltr_action_txt(entry->action, IPMC_TXT_CASE_CAPITAL),
            entry->log ? "LOG" : "BYPASS");

        msg = ipmc_lib_msg_alloc(IPMCLIB_MSG_ID_FLTR_RULE_SET_REQ);
        msg->req.pf_rule.action = action;
        msg->req.pf_rule.pf_idx = pdx;
        memcpy(&msg->req.pf_rule.rule, entry, sizeof(ipmc_lib_rule_t));
        ipmc_lib_msg_tx(msg, isid, sizeof(msg->req.pf_entry));
    }

    return VTSS_OK;
}

static vtss_rc ipmc_lib_stacking_profile_object_set(vtss_isid_t isid_add,
                                                    ipmc_operation_action_t action,
                                                    ipmc_lib_grp_fltr_profile_t *entry)
{
    ipmc_lib_msg_req_t  *msg;
    switch_iter_t       sit;
    vtss_isid_t         isid;

    if (!entry || !msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if (!IPMC_LIB_MSG_ISID_PASS_SLV(isid_add, isid)) {
            continue;
        }

        /* Allocate and send message */
        T_D("sending to isid %d %s %s", isid, ipmc_lib_op_action_txt(action), entry->data.name);
        msg = ipmc_lib_msg_alloc(IPMCLIB_MSG_ID_FLTR_PROFILE_SET_REQ);
        msg->req.pf_data.action = action;
        memcpy(&msg->req.pf_data.data, &entry->data, sizeof(ipmc_lib_profile_t));
        ipmc_lib_msg_tx(msg, isid, sizeof(msg->req.pf_data));
    }

    return VTSS_OK;
}

static vtss_rc ipmc_lib_stacking_profile_mode_set(vtss_isid_t isid_add, BOOL mode)
{
    ipmc_lib_msg_req_t  *msg;
    switch_iter_t       sit;
    vtss_isid_t         isid;

    if (!msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if (!IPMC_LIB_MSG_ISID_PASS_SLV(isid_add, isid)) {
            continue;
        }

        /* Allocate and send message */
        T_D("sending to isid %d %s", isid, mode ? "Enable" : "Disable");
        msg = ipmc_lib_msg_alloc(IPMCLIB_MSG_ID_FLTR_STATE_SET_REQ);
        msg->req.ctrl_state.mode = mode;
        ipmc_lib_msg_tx(msg, isid, sizeof(msg->req.ctrl_state));
    }

    return VTSS_OK;
}

static void ipmc_lib_stacking_profile_clear(vtss_isid_t isid, BOOL clr_cfg)
{
    ipmc_lib_msg_req_t  *msg;

    if (!msg_switch_is_master() || ipmc_lib_isid_is_local(isid)) {
        return;
    }


    /* Allocate and send message */
    T_I("sending to isid %d", isid);
    msg = ipmc_lib_msg_alloc(IPMCLIB_MSG_ID_PROFILE_CLEAR_REQ);
    msg->req.clear_ctrl.clr_cfg = clr_cfg;
    ipmc_lib_msg_tx(msg, isid, sizeof(msg->req.clear_ctrl));
}

static void ipmc_lib_stacking_switch_add_set(vtss_isid_t isid)
{
    vtss_rc                     rc;
    u32                         pdx;
    BOOL                        ctrl_state, do_local;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *fltr_profile, *p;
    ipmc_lib_rule_t             fltr_rule, *q;

    if (!msg_switch_is_master() || !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return;
    }

    fltr_profile = &pfm->profile;
    do_local = FALSE;
    if (ipmc_lib_isid_is_local(isid)) {
        IPMC_LIB_ENTER();
        if (init_local_ipmc_profile) {
            IPMC_LIB_EXIT();
            IPMC_MEM_PROFILE_MGIVE(pfm);
            return;
        }

        do_local = init_local_ipmc_profile = TRUE;
        IPMC_LIB_EXIT();
    }

    IPMC_LIB_ENTER();
    rc = ipmc_lib_profile_state_get(&ctrl_state);
    IPMC_LIB_EXIT();
    if (rc == VTSS_OK) {
        (void) ipmc_lib_stacking_profile_mode_set(isid, ctrl_state);
    }

    if (!do_local) {
        ipmc_lib_grp_fltr_entry_t   entry, *ptr;

        ipmc_lib_stacking_profile_clear(isid, TRUE);

        memset(&entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
        IPMC_LIB_ENTER();
        while ((ptr = ipmc_lib_fltr_entry_get_next(&entry, FALSE)) != NULL) {
            memcpy(&entry, ptr, sizeof(ipmc_lib_grp_fltr_entry_t));
            IPMC_LIB_EXIT();
            rc = ipmc_lib_stacking_profile_entry_set(isid, IPMC_OP_ADD, &entry);
            IPMC_LIB_ENTER();
        }
        IPMC_LIB_EXIT();
    }

    memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
    p = fltr_profile;
    IPMC_LIB_ENTER();
    while ((p = ipmc_lib_fltr_profile_get_next(p, FALSE)) != NULL) {
        pdx = p->data.index;
        if (do_local) {
            rc = ipmc_lib_fltr_profile_set(IPMC_OP_SET, p);
        } else {
            memcpy(fltr_profile, p, sizeof(ipmc_lib_grp_fltr_profile_t));
            IPMC_LIB_EXIT();
            rc = ipmc_lib_stacking_profile_object_set(isid, IPMC_OP_ADD, fltr_profile);
            IPMC_LIB_ENTER();
        }

        if ((rc == VTSS_OK) && !do_local) {
            if ((q = ipmc_lib_fltr_profile_rule_get_first(pdx)) != NULL) {
                do {
                    memcpy(&fltr_rule, q, sizeof(ipmc_lib_rule_t));
                    IPMC_LIB_EXIT();
                    /*
                        In order to pass _ipmc_lib_fltr_rule_sanity checking in slave's initialization,
                        Let the rules to be added one after one by not assigning the next rule idx
                    */
                    fltr_rule.next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;

                    (void) ipmc_lib_stacking_profile_rule_set(isid, IPMC_OP_ADD, pdx, &fltr_rule);
                    IPMC_LIB_ENTER();
                } while ((q = ipmc_lib_fltr_profile_rule_get_next(pdx, &fltr_rule)) != NULL);
            }
        }
    }
    IPMC_LIB_EXIT();

    IPMC_MEM_PROFILE_MGIVE(pfm);
}

/* Set Global Filtering Profile State */
vtss_rc ipmc_lib_mgmt_profile_state_set(BOOL profiling)
{
    vtss_rc rc;
    BOOL    mode;

    if (!msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    IPMC_LIB_ENTER();
    rc = ipmc_lib_profile_state_get(&mode);
    if ((rc == VTSS_OK) && (mode == profiling)) {
        IPMC_LIB_EXIT();
        return VTSS_OK;
    }
    rc = ipmc_lib_profile_state_set(profiling);
    IPMC_LIB_EXIT();

    if (rc == VTSS_OK) {
        if (ipmc_lib_stacking_profile_mode_set(VTSS_ISID_GLOBAL, profiling) == VTSS_OK) {
            IPMC_LIB_CONF_SYNC();
        }
    }

    return rc;
}

/* Get Global Filtering Profile State */
vtss_rc ipmc_lib_mgmt_profile_state_get(BOOL *profiling)
{
    vtss_rc rc;

    IPMC_LIB_ENTER();
    rc = ipmc_lib_profile_state_get(profiling);
    IPMC_LIB_EXIT();

    return rc;
}

/* Add/Delete/Update IPMC Profile Entry */
vtss_rc ipmc_lib_mgmt_fltr_entry_set(ipmc_operation_action_t action, ipmc_lib_grp_fltr_entry_t *fltr_entry)
{
    vtss_rc                     rc;
    ipmc_lib_grp_fltr_entry_t   entry, *ptr;
    ipmc_operation_action_t     op;

    if (!fltr_entry || !msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    if (!strlen(fltr_entry->name)) {
        return VTSS_RC_ERROR;
    }

    op = action;
    memcpy(&entry, fltr_entry, sizeof(ipmc_lib_grp_fltr_entry_t));
    IPMC_LIB_ENTER();
    ptr = ipmc_lib_fltr_entry_get(&entry, TRUE);

    if (action != IPMC_OP_SET) {
        if (ptr != NULL) {
            if (action == IPMC_OP_ADD) {
                IPMC_LIB_EXIT();
                return VTSS_RC_ERROR;
            }
        } else {
            if (action != IPMC_OP_ADD) {
                IPMC_LIB_EXIT();
                return VTSS_RC_ERROR;
            }
        }
    } else {
        if (ptr != NULL) {
            op = IPMC_OP_UPD;
        } else {
            op = IPMC_OP_ADD;
        }
    }

    if (op == IPMC_OP_ADD) {
        ptr = &entry;
    } else if (op == IPMC_OP_UPD) {
        if (ptr == NULL) {
            IPMC_LIB_EXIT();
            return VTSS_RC_ERROR;
        }

        if (memcmp(ptr->name, fltr_entry->name, sizeof(fltr_entry->name))) {
            IPMC_LIB_EXIT();
            return VTSS_RC_ERROR;
        }

        if ((ptr->version != fltr_entry->version) ||
            IPMC_LIB_ADRS_CMP6(ptr->grp_bgn, fltr_entry->grp_bgn) ||
            IPMC_LIB_ADRS_CMP6(ptr->grp_end, fltr_entry->grp_end)) {
            ptr = &entry;
        } else {
            IPMC_LIB_EXIT();
            return VTSS_OK;
        }
    } else if (op == IPMC_OP_DEL) {
        ptr = &entry;
    } else {
        IPMC_LIB_EXIT();
        return VTSS_RC_ERROR;
    }

    rc = ipmc_lib_fltr_entry_set(op, ptr);
    IPMC_LIB_EXIT();

    if (rc == VTSS_OK) {
        if (ipmc_lib_stacking_profile_entry_set(VTSS_ISID_GLOBAL, op, ptr) == VTSS_OK) {
            IPMC_LIB_CONF_SYNC();
        }
    }

    return rc;
}

/* Get IPMC Profile Entry */
vtss_rc ipmc_lib_mgmt_fltr_entry_get(ipmc_lib_grp_fltr_entry_t *fltr_entry, BOOL by_name)
{
    vtss_rc                     rc = VTSS_RC_ERROR;
    ipmc_lib_grp_fltr_entry_t   entry, *ptr;

    if (!fltr_entry) {
        return rc;
    }

    memcpy(&entry, fltr_entry, sizeof(ipmc_lib_grp_fltr_entry_t));
    IPMC_LIB_ENTER();
    if ((ptr = ipmc_lib_fltr_entry_get(&entry, by_name)) != NULL) {
        rc = VTSS_OK;
        memcpy(fltr_entry, ptr, sizeof(ipmc_lib_grp_fltr_entry_t));
    }
    IPMC_LIB_EXIT();

    return rc;
}

/* GetNext IPMC Profile Entry */
vtss_rc ipmc_lib_mgmt_fltr_entry_get_next(ipmc_lib_grp_fltr_entry_t *fltr_entry, BOOL by_name)
{
    vtss_rc                     rc = VTSS_RC_ERROR;
    ipmc_lib_grp_fltr_entry_t   entry, *ptr;

    if (!fltr_entry) {
        return rc;
    }

    memcpy(&entry, fltr_entry, sizeof(ipmc_lib_grp_fltr_entry_t));
    IPMC_LIB_ENTER();
    if ((ptr = ipmc_lib_fltr_entry_get_next(&entry, by_name)) != NULL) {
        rc = VTSS_OK;
        memcpy(fltr_entry, ptr, sizeof(ipmc_lib_grp_fltr_entry_t));
    }
    IPMC_LIB_EXIT();

    return rc;
}

/* Add/Delete/Update IPMC Profile */
vtss_rc ipmc_lib_mgmt_fltr_profile_set(ipmc_operation_action_t action, ipmc_lib_grp_fltr_profile_t *fltr_profile)
{
    vtss_rc                     rc;
    ipmc_lib_profile_t          *data;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *ptr;
    ipmc_operation_action_t     op;

    if (!fltr_profile || !msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    data = &fltr_profile->data;
    if (!strlen(data->name)) {
        return VTSS_RC_ERROR;
    }

    if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return VTSS_RC_ERROR;
    }

    op = action;
    memcpy(&pfm->profile.data, data, sizeof(ipmc_lib_profile_t));
    IPMC_LIB_ENTER();
    ptr = ipmc_lib_fltr_profile_get(&pfm->profile, TRUE);

    if (action != IPMC_OP_SET) {
        if (ptr != NULL) {
            if (action == IPMC_OP_ADD) {
                IPMC_LIB_EXIT();
                IPMC_MEM_PROFILE_MGIVE(pfm);
                return VTSS_RC_ERROR;
            }
        } else {
            if (action != IPMC_OP_ADD) {
                IPMC_LIB_EXIT();
                IPMC_MEM_PROFILE_MGIVE(pfm);
                return VTSS_RC_ERROR;
            }
        }
    } else {
        if (ptr != NULL) {
            op = IPMC_OP_UPD;
        } else {
            op = IPMC_OP_ADD;
        }
    }

    if (op == IPMC_OP_ADD) {
        ptr = &pfm->profile;
        data = &ptr->data;
        data->rule_head_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
        data->version = IPMC_IP_VERSION_INIT;
    } else if (op == IPMC_OP_UPD) {
        ipmc_lib_profile_t  *p;

        if (ptr == NULL) {
            IPMC_LIB_EXIT();
            IPMC_MEM_PROFILE_MGIVE(pfm);
            return VTSS_RC_ERROR;
        }

        data = &ptr->data;
        if (memcmp(data->name, fltr_profile->data.name, sizeof(fltr_profile->data.name))) {
            IPMC_LIB_EXIT();
            IPMC_MEM_PROFILE_MGIVE(pfm);
            return VTSS_RC_ERROR;
        }

        p = &fltr_profile->data;
        if (memcmp(data->desc, p->desc, sizeof(p->desc))) {
            ptr = &pfm->profile;
        } else {
            IPMC_LIB_EXIT();
            IPMC_MEM_PROFILE_MGIVE(pfm);
            return VTSS_OK;
        }
    } else if (op == IPMC_OP_DEL) {
        ptr = &pfm->profile;
    } else {
        IPMC_LIB_EXIT();
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return VTSS_RC_ERROR;
    }

    rc = ipmc_lib_fltr_profile_set(op, ptr);
    IPMC_LIB_EXIT();

    if (rc == VTSS_OK) {
        if (ipmc_lib_stacking_profile_object_set(VTSS_ISID_GLOBAL, op, ptr) == VTSS_OK) {
            IPMC_LIB_CONF_SYNC();
        }
    }

    IPMC_MEM_PROFILE_MGIVE(pfm);
    return rc;
}

/* Get IPMC Profile */
vtss_rc ipmc_lib_mgmt_fltr_profile_get(ipmc_lib_grp_fltr_profile_t *fltr_profile, BOOL by_name)
{
    vtss_rc                     rc = VTSS_RC_ERROR;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *ptr;

    if (!fltr_profile || !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return rc;
    }

    memcpy(&pfm->profile, fltr_profile, sizeof(ipmc_lib_grp_fltr_profile_t));
    IPMC_LIB_ENTER();
    if ((ptr = ipmc_lib_fltr_profile_get(&pfm->profile, by_name)) != NULL) {
        rc = VTSS_OK;
        memcpy(fltr_profile, ptr, sizeof(ipmc_lib_grp_fltr_profile_t));
    }
    IPMC_LIB_EXIT();

    IPMC_MEM_PROFILE_MGIVE(pfm);
    return rc;
}

/* GetNext IPMC Profile */
vtss_rc ipmc_lib_mgmt_fltr_profile_get_next(ipmc_lib_grp_fltr_profile_t *fltr_profile, BOOL by_name)
{
    vtss_rc                     rc = VTSS_RC_ERROR;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *ptr;

    if (!fltr_profile || !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return rc;
    }

    memcpy(&pfm->profile, fltr_profile, sizeof(ipmc_lib_grp_fltr_profile_t));
    IPMC_LIB_ENTER();
    if ((ptr = ipmc_lib_fltr_profile_get_next(&pfm->profile, by_name)) != NULL) {
        rc = VTSS_OK;
        memcpy(fltr_profile, ptr, sizeof(ipmc_lib_grp_fltr_profile_t));
    }
    IPMC_LIB_EXIT();

    IPMC_MEM_PROFILE_MGIVE(pfm);
    return rc;
}

/* Add/Delete/Update IPMC Profile Rule */
vtss_rc ipmc_lib_mgmt_fltr_profile_rule_set(ipmc_operation_action_t action, u32 profile_index, ipmc_lib_rule_t *fltr_rule)
{
    vtss_rc                     rc;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_rule_t             entry, *ptr;
    ipmc_operation_action_t     op;

    if (!fltr_rule ||
        !msg_switch_is_master() ||
        !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return VTSS_RC_ERROR;
    }

    pfm->profile.data.index = profile_index;
    IPMC_LIB_ENTER();
    if (!ipmc_lib_fltr_profile_get(&pfm->profile, FALSE)) {
        IPMC_LIB_EXIT();
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return VTSS_RC_ERROR;
    }
    IPMC_MEM_PROFILE_MGIVE(pfm);

    op = action;
    memcpy(&entry, fltr_rule, sizeof(ipmc_lib_rule_t));
    ptr = ipmc_lib_fltr_profile_rule_search(profile_index, entry.entry_index);

    if (action != IPMC_OP_SET) {
        if (ptr != NULL) {
            if (action == IPMC_OP_ADD) {
                IPMC_LIB_EXIT();
                return VTSS_RC_ERROR;
            }
        } else {
            if (action != IPMC_OP_ADD) {
                IPMC_LIB_EXIT();
                return VTSS_RC_ERROR;
            }
        }
    } else {
        if (ptr != NULL) {
            op = IPMC_OP_UPD;
        } else {
            op = IPMC_OP_ADD;
        }
    }

    if (op == IPMC_OP_ADD) {
        ptr = &entry;
    } else if (op == IPMC_OP_UPD) {
        if (ptr == NULL) {
            IPMC_LIB_EXIT();
            return VTSS_RC_ERROR;
        }

        if ((ptr->next_rule_idx != fltr_rule->next_rule_idx) ||
            (ptr->action != fltr_rule->action) ||
            (ptr->log != fltr_rule->log)) {
            ptr = &entry;
        } else {
            IPMC_LIB_EXIT();
            return VTSS_OK;
        }
    } else if (op == IPMC_OP_DEL) {
        ptr = &entry;
    } else {
        IPMC_LIB_EXIT();
        return VTSS_RC_ERROR;
    }

    rc = ipmc_lib_fltr_profile_rule_set(op, profile_index, ptr);
    IPMC_LIB_EXIT();

    if (rc == VTSS_OK) {
        if (ipmc_lib_stacking_profile_rule_set(VTSS_ISID_GLOBAL, op, profile_index, ptr) == VTSS_OK) {
            IPMC_LIB_CONF_SYNC();
        }
    }

    return rc;
}

/* Search IPMC Profile Rule by Entry Index */
vtss_rc ipmc_lib_mgmt_fltr_profile_rule_search(u32 profile_index, u32 entry_index, ipmc_lib_rule_t *fltr_rule)
{
    vtss_rc         rc = VTSS_RC_ERROR;
    ipmc_lib_rule_t *ptr;

    if (!fltr_rule) {
        return rc;
    }

    IPMC_LIB_ENTER();
    if ((ptr = ipmc_lib_fltr_profile_rule_search(profile_index, entry_index)) != NULL) {
        rc = VTSS_OK;
        memcpy(fltr_rule, ptr, sizeof(ipmc_lib_rule_t));
    }
    IPMC_LIB_EXIT();

    return rc;
}

/* Get IPMC Profile Rule */
vtss_rc ipmc_lib_mgmt_fltr_profile_rule_get(u32 profile_index, ipmc_lib_rule_t *fltr_rule)
{
    vtss_rc         rc = VTSS_RC_ERROR;
    ipmc_lib_rule_t entry, *ptr;

    if (!fltr_rule) {
        return rc;
    }

    memcpy(&entry, fltr_rule, sizeof(ipmc_lib_rule_t));
    IPMC_LIB_ENTER();
    if ((ptr = ipmc_lib_fltr_profile_rule_get(profile_index, &entry)) != NULL) {
        rc = VTSS_OK;
        memcpy(fltr_rule, ptr, sizeof(ipmc_lib_rule_t));
    }
    IPMC_LIB_EXIT();

    return rc;
}

/* GetFirst IPMC Profile Rule */
vtss_rc ipmc_lib_mgmt_fltr_profile_rule_get_first(u32 profile_index, ipmc_lib_rule_t *fltr_rule)
{
    vtss_rc         rc = VTSS_RC_ERROR;
    ipmc_lib_rule_t *ptr;

    if (!fltr_rule) {
        return rc;
    }

    IPMC_LIB_ENTER();
    if ((ptr = ipmc_lib_fltr_profile_rule_get_first(profile_index)) != NULL) {
        rc = VTSS_OK;
        memcpy(fltr_rule, ptr, sizeof(ipmc_lib_rule_t));
    }
    IPMC_LIB_EXIT();

    return rc;
}

/* GetNext IPMC Profile Rule */
vtss_rc ipmc_lib_mgmt_fltr_profile_rule_get_next(u32 profile_index, ipmc_lib_rule_t *fltr_rule)
{
    vtss_rc         rc = VTSS_RC_ERROR;
    ipmc_lib_rule_t entry, *ptr;

    if (!fltr_rule) {
        return rc;
    }

    memcpy(&entry, fltr_rule, sizeof(ipmc_lib_rule_t));
    IPMC_LIB_ENTER();
    if ((ptr = ipmc_lib_fltr_profile_rule_get_next(profile_index, &entry)) != NULL) {
        rc = VTSS_OK;
        memcpy(fltr_rule, ptr, sizeof(ipmc_lib_rule_t));
    }
    IPMC_LIB_EXIT();

    return rc;
}

/* Clear all the profile settings and running databases */
vtss_rc ipmc_lib_mgmt_clear_profile(vtss_isid_t isid_add)
{
    switch_iter_t               sit;
    vtss_isid_t                 isid;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *profile, *p;
    ipmc_lib_grp_fltr_entry_t   entry, *q;
    vtss_rc                     rc;

    if (!msg_switch_is_master() ||
        (isid_add != VTSS_ISID_GLOBAL) ||
        !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return VTSS_RC_ERROR;
    }

    profile = &pfm->profile;
    rc = VTSS_OK;
    memset(profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
    memset(&entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));

    IPMC_LIB_ENTER();
    while ((p = ipmc_lib_fltr_profile_get_next(profile, FALSE)) != NULL) {
        memcpy(profile, p, sizeof(ipmc_lib_grp_fltr_profile_t));
        rc |= ipmc_lib_fltr_profile_set(IPMC_OP_DEL, profile);
    }

    while ((q = ipmc_lib_fltr_entry_get_next(&entry, FALSE)) != NULL) {
        memcpy(&entry, q, sizeof(ipmc_lib_grp_fltr_entry_t));
        rc |= ipmc_lib_fltr_entry_set(IPMC_OP_DEL, &entry);
    }
    IPMC_LIB_EXIT();

    if (rc != VTSS_OK) {
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return rc;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if (!IPMC_LIB_MSG_ISID_PASS_SLV(isid_add, isid)) {
            continue;
        }

        ipmc_lib_stacking_profile_clear(isid, TRUE);
    }

    IPMC_MEM_PROFILE_MGIVE(pfm);
    return rc;
}

/* Get Internal IPMC Profile Tree */
BOOL ipmc_lib_mgmt_profile_tree_get(u32 tdx, ipmc_profile_rule_t *entry, BOOL *is_avl)
{
    ipmc_profile_rule_t *ptr;

    if (!entry || !is_avl) {
        return FALSE;
    }

    IPMC_LIB_ENTER();
    if ((ptr = ipmc_lib_profile_tree_get(tdx, entry, is_avl)) == NULL) {
        IPMC_LIB_EXIT();
        return FALSE;
    }
    memcpy(entry, ptr, sizeof(ipmc_profile_rule_t));
    IPMC_LIB_EXIT();

    return TRUE;
}

/* GetNext Internal IPMC Profile Tree */
BOOL ipmc_lib_mgmt_profile_tree_get_next(u32 tdx, ipmc_profile_rule_t *entry, BOOL *is_avl)
{
    ipmc_profile_rule_t *ptr;

    if (!entry || !is_avl) {
        return FALSE;
    }

    IPMC_LIB_ENTER();
    if ((ptr = ipmc_lib_profile_tree_get_next(tdx, entry, is_avl)) == NULL) {
        IPMC_LIB_EXIT();
        return FALSE;
    }
    memcpy(entry, ptr, sizeof(ipmc_profile_rule_t));
    IPMC_LIB_EXIT();

    return TRUE;
}

/* Get Specific Internal IPMC Profile Tree VID */
BOOL ipmc_lib_mgmt_profile_tree_vid_get(u32 tdx, vtss_vid_t *pf_vid)
{
    BOOL    rc = FALSE;

    if (!pf_vid) {
        return rc;
    }

    IPMC_LIB_ENTER();
    rc = ipmc_lib_profile_tree_vid_get(tdx, pf_vid);
    IPMC_LIB_EXIT();

    return rc;
}

/* Get delta time for timers (filter timer & source timer) */
int ipmc_lib_mgmt_calc_delta_time(vtss_isid_t isid, ipmc_time_t *time_v)
{
    ipmc_time_t current_time;

    if (!time_v) {
        return -1;
    }

    if (!ipmc_lib_time_curr_get(&current_time)) {
        return 0;
    }

    if (ipmc_lib_isid_is_local(isid)) {
        if (IPMC_TIMER_LESS(&current_time, time_v)) {
            return (time_v->sec - current_time.sec);
        }
    } else {
        if (msg_switch_is_master()) {
            return time_v->sec;
        }
    }

    return 0;
}

BOOL ipmc_lib_isid_is_local(vtss_isid_t idx)
{
    if (IPMC_LIB_MY_ISID == IPMC_LIB_ISID_ASYNC) {
        return msg_switch_is_local(idx);
    } else if (IPMC_LIB_MY_ISID == IPMC_LIB_ISID_INIT) {
        return FALSE;
    } else if (IPMC_LIB_MY_ISID == IPMC_LIB_ISID_ANY) {
        return ((idx < VTSS_ISID_END) && (VTSS_ISID_START <= idx));
    } else {
        if (msg_switch_is_master()) {
            return (IPMC_LIB_MY_ISID == idx);
        } else {
            return FALSE;
        }
    }
}

vtss_rc ipmc_lib_init(vtss_init_data_t *data)
{
    vtss_isid_t     isid = data->isid;
#if VTSS_SWITCH_STACKABLE
    switch_iter_t   sit;
    stack_config_t  stk;
    BOOL            dty;
#endif /* VTSS_SWITCH_STACKABLE */
    BOOL            stackable;
    msg_rx_filter_t filter;

    switch ( data->cmd ) {
    case INIT_CMD_INIT:
#if VTSS_TRACE_ENABLED
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&ipmc_lib_trace_reg, ipmc_lib_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&ipmc_lib_trace_reg);
#endif /* VTSS_TRACE_ENABLED */

        critd_init(&ipmc_lib_global.crit, "ipmc_lib_crit", VTSS_MODULE_ID_IPMC_LIB, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        IPMC_LIB_EXIT();

        IPMC_LIB_MY_ISID = IPMC_LIB_ISID_INIT;

#ifdef VTSS_SW_OPTION_VCLI
        ipmc_lib_cli_req_init();
#endif /* VTSS_SW_OPTION_VCLI */

#ifdef VTSS_SW_OPTION_ICFG
        if (ipmc_lib_icfg_init() != VTSS_OK) {
            T_E("ipmc_lib_icfg_init failed!");
        }
#endif /* VTSS_SW_OPTION_ICFG */

        if (ipmc_lib_profile_init() != VTSS_OK) {
            T_W("ipmc_lib_profile_init failed!");
        } else {
            if (!ipmc_lib_profile_conf_ptr_set((void *)&ipmc_lib_global.configuration.profile)) {
                T_W("ipmc_lib_profile_conf_ptr_set failed!");
            }
        }

        ipmc_lib_global.request_buf_pool = msg_buf_pool_create(VTSS_MODULE_ID_IPMC_LIB,
                                                               "Request",
                                                               IPMC_LIB_MSG_REQ_BUFS,
                                                               sizeof(ipmc_lib_msg_req_t));

        T_I("IPMC_LIB-INIT_CMD_INIT Done(%d)", isid);

        break;
    case INIT_CMD_START:
        T_I("START: ISID->%d", isid);

        /* Register for stack messages */
        memset(&filter, 0, sizeof(filter));
        filter.cb = ipmc_lib_msg_rx;
        filter.modid = VTSS_MODULE_ID_IPMC_LIB;
        (void)msg_rx_filter_register(&filter);

        break;
    case INIT_CMD_CONF_DEF:
        T_I("CONF_DEF: ISID->%d", isid);

        if (isid == VTSS_ISID_GLOBAL) {
            /* Clear Running Trees */
            (void) ipmc_lib_mgmt_clear_profile(VTSS_ISID_GLOBAL);

            /* Reset stack configuration */
            (void) ipmc_lib_conf_read(TRUE);
        }

        break;
    case INIT_CMD_MASTER_UP:
        T_I("MASTER_UP: ISID->%d", isid);

        stackable = FALSE;
#if VTSS_SWITCH_STACKABLE
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
        while (switch_iter_getnext(&sit)) {
            if (topo_stack_config_get(sit.isid, &stk, &dty) == VTSS_RC_OK) {
                stackable = stk.stacking;
                break;
            }
        }
#endif /* VTSS_SWITCH_STACKABLE */

        IPMC_LIB_ENTER();
        IPMC_LIB_STACKABLE = stackable;
        if (stackable) {
            IPMC_LIB_MY_ISID = IPMC_LIB_ISID_TRANSIT;
        } else {
            IPMC_LIB_MY_ISID = IPMC_LIB_ISID_ANY;
        }
        init_local_ipmc_profile = FALSE;
        IPMC_LIB_EXIT();

        /* Read configuration */
        (void) ipmc_lib_conf_read(FALSE);

        break;
    case INIT_CMD_MASTER_DOWN:
        T_I("MASTER_DOWN: ISID->%d", isid);

        IPMC_LIB_ENTER();
        IPMC_LIB_MY_ISID = IPMC_LIB_ISID_ASYNC;
        init_local_ipmc_profile = FALSE;
        IPMC_LIB_EXIT();

        break;
    case INIT_CMD_SWITCH_ADD:
        T_I("SWITCH_ADD: ISID->%d", isid);

        IPMC_LIB_ENTER();
        IPMC_LIB_MY_ISID = IPMC_LIB_ISID_ASYNC;
        IPMC_LIB_EXIT();

        ipmc_lib_stacking_switch_add_set(isid);

        break;
    case INIT_CMD_SWITCH_DEL:
        T_I("SWITCH_DEL: ISID->%d", isid);

        break;
    default:
        break;
    }

    return 0;
}
