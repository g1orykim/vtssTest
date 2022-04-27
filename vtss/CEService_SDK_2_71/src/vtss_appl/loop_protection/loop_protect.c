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


#include <network.h>            /* for the ntohs/htons */

#include "main.h"
#include "conf_api.h"
#include "misc_api.h"
#include "port_api.h"
#include "packet_api.h"
#include "acl_api.h"
#include "loop_protect.h"
#include "msg_api.h"
#include "topo_api.h"
#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif
#ifdef VTSS_SW_OPTION_LOOP_DETECT
#include "vtss_lb_api.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "loop_protect_icfg.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_LOOP_PROTECT

/* ================================================================= *
 *  LOOP_PROTECT module private data
 * ================================================================= */

/* State and config */
static lprot_conf_t lprot_conf;

typedef struct {
    uint port;                  /* Port_no or aggr_id */
    int transmit_timer;
    int shutdown_timer;
    u32 last_disable;
    int ttl_timer;
    loop_protect_port_info_t info;
} lprot_port_state_t;

typedef struct {
    u32 last_refresh;
} lprot_switch_state_t;

static struct {
    vtss_usid_t usid;           /* My usid */
    u8 master_mac[6];           /* MAC from master */
    u8 master_key[SHA1_HASH_SIZE]; /* Master/Stack key */
    lprot_switch_state_t switches[VTSS_ISID_END];
    lprot_port_state_t   ports[VTSS_PORT_ARRAY_SIZE];
} lprot_state;

static struct {
    loop_protect_port_info_t ports[VTSS_PORT_ARRAY_SIZE];
} lprot_master_state[VTSS_ISID_END];

/* Frame data */
static const u8  dmac[6] = {0x01, 0x01, 0xc1, 0x00, 0x00, 0x00}; /* 01-01-c1-00-00-00 */
static       u8  switchmac[6];
static const u16 etype = 0x9003;
static void *loop_filter_id;

/* Crypto variables */
static int lprot_hash;
static u8  lprot_key[SHA1_HASH_SIZE];

/* Thread variables */
static critd_t      lprot_crit;          /* module critical section */
static cyg_flag_t   lprot_control_flags; /* thread control */
static cyg_flag_t   lprot_status_flags;
static cyg_handle_t lprot_thread_handle;
static cyg_thread   lprot_thread_block;
static char         lprot_thread_stack[THREAD_DEFAULT_STACK_SIZE];

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "lprot",
    .descr     = "Loop Protection"
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
#define LPROT_CRIT_ENTER()         critd_enter(        &lprot_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define LPROT_CRIT_EXIT()          critd_exit(         &lprot_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define LPROT_CRIT_ASSERT_LOCKED() critd_assert_locked(&lprot_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else
#define LPROT_CRIT_ENTER()         critd_enter(        &lprot_crit)
#define LPROT_CRIT_EXIT()          critd_exit(         &lprot_crit)
#define LPROT_CRIT_ASSERT_LOCKED() critd_assert_locked(&lprot_crit)
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

static void lprot_enable(lprot_port_state_t *pstate);

static loop_protect_msg_t *
loop_protect_alloc_message(loop_protect_msg_id_t msg_id)
{
    loop_protect_msg_t *msg = VTSS_MALLOC(sizeof(loop_protect_msg_t));
    if(msg)
        msg->msg_id = msg_id;
    T_N("msg type %d => %p", msg_id, msg);
    return msg;
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* LOOP_PROTECT error text */
char *loop_protect_error_txt(vtss_rc rc)
{
    switch (rc) {
    case LOOP_PROTECT_ERROR_GEN:
        return "LOOP_PROTECT generic error";
    case LOOP_PROTECT_ERROR_PARM:
        return "LOOP_PROTECT parameter error";
    case LOOP_PROTECT_ERROR_TIMEOUT:
        return "LOOP_PROTECT timeout error";
    case LOOP_PROTECT_ERROR_MSGALLOC:
        return "LOOP_PROTECT message allocation error";
    default:
        return "LOOP_PROTECT unknown error";
    }
}

static void loop_protect_redir_cpu(void)
{
    static BOOL acl_installed;
    acl_entry_conf_t conf;
    port_iter_t pit;
    uint ports = 0;

    /* Init ACL structure */
    (void) acl_mgmt_ace_init(VTSS_ACE_TYPE_ETYPE, &conf);

    /* Count how many ports are actively protected */
    if(lprot_conf.global.enabled) {         
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, 
                              PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
#if defined(VTSS_FEATURE_ACL_V2)
        memset(conf.port_list, 0, sizeof(conf.port_list)); /* Apply to none by default */
#endif /* VTSS_FEATURE_ACL_V2 */
        while (port_iter_getnext(&pit)) {
            const loop_protect_port_conf_t *pconf = &lprot_conf.ports[VTSS_ISID_LOCAL][pit.iport];
            if(pconf->enabled) { /* This port active? */
                ports++;
#if defined(VTSS_FEATURE_ACL_V2)
                conf.port_list[pit.iport] = TRUE;
#endif /* VTSS_FEATURE_ACL_V2 */
            }
        }
    }

    /* Install rule if any ports are active */
    if(ports) {
        conf.id = LPROT_ACE_ID;
        conf.isid = VTSS_ISID_LOCAL;
        conf.action.force_cpu = TRUE;
        conf.action.cpu_queue = PACKET_XTR_QU_BPDU; /* High! */
#if defined(VTSS_FEATURE_ACL_V1)
        /* Permit "off" */
        conf.action.permit = FALSE;
        /* Mask/filter out all egress ports */
        conf.action.port_no = VTSS_PORT_NO_NONE;
#endif
#if defined(VTSS_FEATURE_ACL_V2)
        /* "Filter" action */
        conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
        /* Mask/filter out all egress ports */
        memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
#endif /* VTSS_FEATURE_ACL_V2 */
        memcpy(conf.frame.etype.dmac.value, dmac, sizeof(conf.frame.etype.dmac.value));
        memset(conf.frame.etype.dmac.mask, 0xFF, sizeof(conf.frame.etype.dmac.mask));
        memcpy(conf.frame.etype.smac.value, lprot_state.master_mac, sizeof(conf.frame.etype.smac.value));
        memset(conf.frame.etype.smac.mask, 0xFF, sizeof(conf.frame.etype.smac.mask));
        memset(conf.frame.etype.etype.mask, 0xFF, sizeof(conf.frame.etype.etype.mask));
        conf.frame.etype.etype.value[0] = (etype >> 8);
        conf.frame.etype.etype.value[1] = etype & 0xff;
        T_D("Setting up ACL redirect for %u ports", ports);
        if (acl_mgmt_ace_add(ACL_USER_LOOP_PROTECT, ACE_ID_NONE, &conf) == VTSS_OK)  
            acl_installed = TRUE;
        else
            T_E("loop_protect_redir_cpu() failed");
    } else {
        /* Delete old ACL rule */
        if(acl_installed) {
            T_D("Delete ACL redirect ID = %d", LPROT_ACE_ID);
            if(acl_mgmt_ace_del(ACL_USER_LOOP_PROTECT, LPROT_ACE_ID) != VTSS_OK)  
                T_E("loop_protect_redir_cpu() ACL delete failed");
            acl_installed = FALSE;
        }
    }
}

static void lprot_conf_apply_local(void)
{
    port_iter_t pit;
    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    LPROT_CRIT_ENTER();
    while (port_iter_getnext(&pit)) {                    
        /* Enable disabled ports that are no longer configured protected */
        if(lprot_state.ports[pit.iport].info.loop_detect) {
            lprot_enable(&lprot_state.ports[pit.iport]);
        }
    }
    loop_protect_redir_cpu();
    LPROT_CRIT_EXIT();
}

static void loop_protect_conf_send_port(vtss_isid_t isid, vtss_port_no_t port_no)
{
    loop_protect_msg_t *msg = loop_protect_alloc_message(LOOP_PROTECT_MSG_ID_CONF_PORT);
    if(msg) {
        msg->data.port_conf.port_no = port_no;
        msg->data.port_conf.port_conf = lprot_conf.ports[isid][port_no];
        msg_tx(VTSS_MODULE_ID_LOOP_PROTECT, isid, msg, sizeof(*msg));
    }
}

static void loop_protect_conf_send(vtss_isid_t isid)
{
    loop_protect_msg_t *msg = loop_protect_alloc_message(LOOP_PROTECT_MSG_ID_CONF);
    if(msg) {
        LPROT_CRIT_ENTER();
        memcpy(msg->data.unit_conf.key, lprot_key, sizeof(lprot_key));
        memcpy(msg->data.unit_conf.mac, switchmac, sizeof(switchmac));
        msg->data.unit_conf.usid = topo_isid2usid(isid);
        msg->data.unit_conf.global_conf = lprot_conf.global;
        memcpy(msg->data.unit_conf.port_conf, lprot_conf.ports[isid], sizeof(lprot_conf.ports[isid]));
        LPROT_CRIT_EXIT();
        msg_tx(VTSS_MODULE_ID_LOOP_PROTECT, isid, msg, sizeof(*msg));
    }
}

static void lprot_conf_apply(void)
{
    switch_iter_t sit;

    T_N("enter");
    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        loop_protect_conf_send(sit.isid);
    }
    T_N("exit");
}

static vtss_rc loop_protect_conf_flash_save(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    loop_protect_conf_blk_t *loop_protect_class_no_blk;

    /* Save new data to conf module */
    if ((loop_protect_class_no_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_LOOP_PROTECT, NULL)) == NULL) {
        return LOOP_PROTECT_ERROR_GEN;
    }
    loop_protect_class_no_blk->conf = lprot_conf;
    conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_LOOP_PROTECT);
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
    return VTSS_OK;
}

/* Set Loop_Protect General configuration  */
vtss_rc loop_protect_conf_set(const loop_protect_conf_t *conf)
{
    vtss_rc rc;
    T_N("enter");

    if (!msg_switch_is_master()) {
        T_W("Not master");
        rc = PORT_ERROR_STACK_STATE;
    } else {
        LPROT_CRIT_ENTER();

        /* Move new data to database */
        lprot_conf.global = *conf;

        /* Save in flash */
        rc = loop_protect_conf_flash_save();

        LPROT_CRIT_EXIT();

        lprot_conf_apply();         /* Apply config */
    }

    T_N("exit");
    return rc;
}

/* Get Loop_Protect General configuration  */
vtss_rc loop_protect_conf_get(loop_protect_conf_t *conf)
{
    vtss_rc rc;
    T_N("enter");

    if (!msg_switch_is_master()) {
        T_W("Not master");
        rc = PORT_ERROR_STACK_STATE;
    } else {
        LPROT_CRIT_ENTER();
        *conf = lprot_conf.global; // Get the 'real' configuration
        LPROT_CRIT_EXIT();
        rc = VTSS_OK;
    }

    T_N("exit");
    return rc;
}

/* Set port Loop_Protect configuration  */
vtss_rc loop_protect_conf_port_set(vtss_isid_t isid,
                                   vtss_port_no_t port_no, 
                                   const loop_protect_port_conf_t *conf)
{
    vtss_rc rc = LOOP_PROTECT_ERROR_PARM;
    T_N("enter, port_no: %u", port_no);

    if (!msg_switch_is_master()) {
        T_W("Not master");
        rc = PORT_ERROR_STACK_STATE;
    } else {
        LPROT_CRIT_ENTER();
        if(isid < VTSS_ISID_END && port_no < VTSS_PORTS) {
            BOOL changed = memcmp(&lprot_conf.ports[isid][port_no], conf, sizeof(*conf)) != 0;
            if (changed) {
                lprot_conf.ports[isid][port_no] = *conf;
                /* Save in flash */
                rc = loop_protect_conf_flash_save();
                loop_protect_conf_send_port(isid, port_no);
            } else {
                rc = VTSS_OK;
            }
        }
        LPROT_CRIT_EXIT();
    }

    T_N("exit");
    return rc;
}

/* Get port Loop_Protect configuration  */
vtss_rc loop_protect_conf_port_get(vtss_isid_t isid,
                                   vtss_port_no_t port_no, 
                                   loop_protect_port_conf_t *conf)
{
    vtss_rc rc = LOOP_PROTECT_ERROR_PARM;
    T_N("enter, port_no: %u", port_no);

    if (!msg_switch_is_master()) {
        T_W("Not master");
        rc = PORT_ERROR_STACK_STATE;
    } else {
        if(isid < VTSS_ISID_END && port_no < VTSS_PORTS) {
            LPROT_CRIT_ENTER();
            *conf = lprot_conf.ports[isid][port_no];
            LPROT_CRIT_EXIT();
            rc = VTSS_OK;
        }
    }

    T_N("exit");
    return rc;
}

/* Get Loop_Protect port info  */
vtss_rc loop_protect_port_info_get(vtss_isid_t isid, 
                                   vtss_port_no_t port_no, 
                                   loop_protect_port_info_t *info)
{
    vtss_rc rc = LOOP_PROTECT_ERROR_PARM;
    T_N("enter, sid %d, port_no: %u", isid, port_no);

    if (!msg_switch_is_master()) {
        T_W("Not master");
        rc = PORT_ERROR_STACK_STATE;
    } else {
        LPROT_CRIT_ENTER();
        if(isid < VTSS_ISID_END && port_no < VTSS_PORTS) {
            if(lprot_conf.ports[isid][port_no].enabled) {
                if(lprot_state.switches[isid].last_refresh != time(NULL)) {
                    loop_protect_msg_t *msg = loop_protect_alloc_message(LOOP_PROTECT_MSG_ID_PORT_STATUS_REQ);
                    if(msg) {
                        cyg_flag_value_t flag = (1<<isid);
                        cyg_flag_t *flags = &lprot_status_flags;
                        cyg_tick_count_t wtime = cyg_current_time() + VTSS_OS_MSEC2TICK(3 * 1000);
                        msg_tx(VTSS_MODULE_ID_LOOP_PROTECT, isid, msg, sizeof(*msg));
                        cyg_flag_maskbits(flags, ~flag);
                        LPROT_CRIT_EXIT();
                        if (cyg_flag_timed_wait(flags, flag, CYG_FLAG_WAITMODE_OR, wtime) & flag) {
                            LPROT_CRIT_ENTER();
                            *info = lprot_master_state[isid].ports[port_no];
                            rc = VTSS_OK;
                        } else {
                            LPROT_CRIT_ENTER();
                            rc = LOOP_PROTECT_ERROR_TIMEOUT;
                        }
                    }
                } else {
                    /* Up-to date info */
                    *info = lprot_master_state[isid].ports[port_no];
                    rc = VTSS_OK;
                }
            } else {
                rc = LOOP_PROTECT_ERROR_INACTIVE; /* Not active */
            }
        }
        LPROT_CRIT_EXIT();
    }

    T_N("exit");
    return rc;
}

const char *loop_protect_action2string(loop_protect_action_t action)
{
    static const char * const action_string[] = {
        [LOOP_PROTECT_ACTION_SHUTDOWN] = "Shutdown",
        [LOOP_PROTECT_ACTION_SHUT_LOG] = "Shutdown+Log",
        [LOOP_PROTECT_ACTION_LOG_ONLY] = "Log Only",
    };
    if(action <= LOOP_PROTECT_ACTION_LOG_ONLY)
        return action_string[action];
    return "Undefined";
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

static void
loop_protect_conf_read(BOOL create)
{
    loop_protect_conf_blk_t *blk;
    ulong size;
    BOOL do_create;

    T_D("Read config - %s", create ? "Force New" : "Normal");

    if (misc_conf_read_use()) {
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_LOOP_PROTECT, &size)) == NULL ||
            size != sizeof(*blk)) {
            blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_LOOP_PROTECT, sizeof(*blk));
            T_I("conf_sec_open failed or size mismatch, creating defaults");
            do_create = TRUE;
        } else if(blk->version != LOOP_PROTECT_CONF_BLK_VERSION) {
            T_I("version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        blk       = NULL;
        do_create = TRUE;
    }
    
    if (do_create) {
        uint i, j;

        /* Use default configuration */

        lprot_conf.global.enabled           = LOOP_PROTECT_DEFAULT_GLOBAL_ENABLED;       /* Disabled */
        lprot_conf.global.transmission_time = LOOP_PROTECT_DEFAULT_GLOBAL_TX_TIME;       /* 5 seconds */
        lprot_conf.global.shutdown_time     = LOOP_PROTECT_DEFAULT_GLOBAL_SHUTDOWN_TIME; /* 3 minutes */
        for(i = 0; i < VTSS_ISID_END; i++) {
            for(j = 0; j < VTSS_PORT_ARRAY_SIZE; j++) {
                lprot_conf.ports[i][j].enabled  = LOOP_PROTECT_DEFAULT_PORT_ENABLED;
                lprot_conf.ports[i][j].action   = LOOP_PROTECT_DEFAULT_PORT_ACTION;
                lprot_conf.ports[i][j].transmit = LOOP_PROTECT_DEFAULT_PORT_TX_MODE;
            }
        }
        if (blk != NULL) {
            memset(blk, 0, sizeof(*blk));
            blk->conf = lprot_conf;
        }
    } else {
        if (blk != NULL) {
            lprot_conf = blk->conf;
        }
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (blk != NULL) {
        blk->version = LOOP_PROTECT_CONF_BLK_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_LOOP_PROTECT);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    /* Apply config */
    lprot_conf_apply();
}

static void lprot_port_disable_master(vtss_isid_t isid, 
                                      vtss_port_no_t port_no, 
                                      BOOL dis)
{
    port_vol_conf_t conf;
    port_user_t     user = PORT_USER_LOOP_PROTECT;
        
    T_D("%sable port: isid: %u, port_no: %u", dis ? "dis" : "en", isid, port_no);
    if (port_vol_conf_get(user, isid, port_no, &conf) == VTSS_OK &&
        conf.disable != dis) {
        conf.disable = dis;
        if (port_vol_conf_set(user, isid, port_no, &conf) != VTSS_OK)
            T_E("port_vol_conf_set(isid: %u, port_no: %u) failed", isid, port_no);
    }
}

static BOOL lprot_disable_ratelimit(lprot_port_state_t *pstate)
{
    u32 now = time(NULL);
    if (lprot_state.ports[pstate->port].last_disable != now) {
        lprot_state.ports[pstate->port].last_disable = now;
        return TRUE;
    }
    return FALSE;
}

static void lprot_port_disable(vtss_port_no_t port_no, BOOL disable)
{
    loop_protect_msg_t *msg = loop_protect_alloc_message(LOOP_PROTECT_MSG_ID_PORT_CTL);
    if(msg) {
        msg->data.port_ctl.port_no = port_no;
        msg->data.port_ctl.disable = disable;
        msg_tx(VTSS_MODULE_ID_LOOP_PROTECT, 0, msg, sizeof(*msg));
        if (disable) {
            lprot_state.ports[port_no].last_disable = time(NULL);
        }
    } else {
        T_W("Unable to %sable port %d", disable ? "dis" : "en", iport2uport(port_no));
    }
}

static void lprot_enable(lprot_port_state_t *pstate)
{
    T_I("Re-enable %s %d", "port", iport2uport(pstate->port));
    pstate->info.disabled = FALSE;
    pstate->info.loop_detect = FALSE;
    pstate->transmit_timer = 0;
    lprot_port_disable(pstate->port, FALSE);
}

static void lprot_disable(lprot_port_state_t *pstate)
{
    const loop_protect_port_conf_t *pconf;
    T_W("Loop detected: %s %d", "Port", iport2uport(pstate->port));
    pconf = &lprot_conf.ports[VTSS_ISID_LOCAL][pstate->port];
    if(pconf->action == LOOP_PROTECT_ACTION_SHUTDOWN ||
       pconf->action == LOOP_PROTECT_ACTION_SHUT_LOG) {
        pstate->shutdown_timer = 0;
        pstate->info.disabled = TRUE;
        if (lprot_disable_ratelimit(pstate)) {
            lprot_port_disable(pstate->port, TRUE);
        } else {
            T_N("Skip disable, already sent this very second, port %d", pstate->port);
        }
    }
    if(pconf->action == LOOP_PROTECT_ACTION_SHUT_LOG ||
       pconf->action == LOOP_PROTECT_ACTION_LOG_ONLY) {
        if(!pstate->info.loop_detect) { /* Only 1st time detected */
            S_W("Loop Detected: %s %d %s", 
                "Port", iport2uport(pstate->port),
                pconf->action == LOOP_PROTECT_ACTION_SHUT_LOG ? "shut down" : "");
        }
    }
    pstate->info.loops++;
    pstate->info.last_loop = time(NULL);
    pstate->info.loop_detect = TRUE;
}

static BOOL lprot_calc_digest(const loop_prot_pdu_t *pdu, u8 *digest)
{
    unsigned long digest_length;
    digest_length = SHA1_HASH_SIZE;
    if(hmac_memory(lprot_hash, lprot_state.master_key, sizeof(lprot_state.master_key), 
                   (u8*) pdu, sizeof(*pdu), digest, &digest_length) != CRYPT_OK ||
       digest_length != SHA1_HASH_SIZE) {
        T_E("Packet Authenitication error");
    } else {
        T_N("Digest %ld bytes: %02x-%02x-%02x-%02x", digest_length, 
            digest[0], digest[1], digest[2], digest[3]);
        return TRUE;
    }
    return FALSE;
}

static BOOL lprot_egress_filter(vtss_port_no_t egress_port)
{
    vtss_vlan_port_conf_t pconf;
    vtss_packet_filter_t     filter;
    vtss_packet_frame_info_t info;

    if (vtss_vlan_port_conf_get(NULL, egress_port, &pconf) != VTSS_OK) {
        T_E("Unable to get port vlan config for port %d", iport2uport(egress_port));
        return FALSE;
    }

    vtss_packet_frame_info_init(&info);
    info.vid = pconf.pvid;
    info.port_tx = egress_port; // tx filtering
    info.aggr_tx_disable = TRUE;

    if (vtss_packet_frame_filter(NULL, &info, &filter) == VTSS_RC_OK &&
        filter == VTSS_PACKET_FILTER_DISCARD) {
        T_I("frame discarded by egress filtering, port: %u, vid: %u",
            iport2uport(egress_port), info.vid);
        return TRUE;
    }

    return FALSE;
}

static void lprot_transmit(vtss_usid_t usid, vtss_port_no_t egress_port, lprot_port_state_t *pstate)
{
    if (!lprot_egress_filter(egress_port)) {
        vtss_uport_no_t uport = iport2uport(pstate->port);
        loop_prot_pdu_t *pdu = (loop_prot_pdu_t*) packet_tx_alloc(sizeof(*pdu));
        T_N("transmit check %d", uport);
        if(pdu) {
            packet_tx_props_t tx_props;
            u8 digest[SHA1_HASH_SIZE];

            T_D("transmit link %d", uport);
            packet_tx_props_init(&tx_props);
            tx_props.packet_info.modid     = VTSS_MODULE_ID_LOOP_PROTECT;
            tx_props.packet_info.frm[0]    = (void*) pdu;
            tx_props.packet_info.len[0]    = sizeof(*pdu);
            tx_props.tx_info.dst_port_mask = VTSS_BIT64(egress_port);

            /* Frame */
            memset(pdu, 0, sizeof(*pdu));
            memcpy(pdu->dst, dmac, sizeof(pdu->dst));
            memcpy(pdu->src, lprot_state.master_mac, sizeof(pdu->dst));
            pdu->oui = htons(etype);
            pdu->version = LPROT_PROTVERSION;
            pdu->lport = egress_port;
            pdu->usid = (u16) usid;
            pdu->tstamp = cyg_current_time();
            memcpy(pdu->switchmac, switchmac, sizeof(pdu->switchmac)); /* Own MAC */
            if(lprot_calc_digest(pdu, digest))
                memcpy(pdu->authcode, digest, sizeof(digest));
            if(packet_tx(&tx_props) != VTSS_RC_OK)
                T_E("transmit fail port %d", uport);
        } else {
            T_E("transmit malloc fail port %d", uport);
        }
    }
    pstate->transmit_timer = 0;
}

/*
 * Main thread helper
 */ 

static void lprot_poag_tick(lprot_port_state_t *pstate, const loop_protect_port_conf_t *pconf)
{
    BOOL tx = pconf->transmit;
    vtss_port_no_t egress_port = pstate->port;
    T_N("Tick: %s %d (%sabled, tx: %d), shutdown %d", 
        "Port", iport2uport(pstate->port), 
        pstate->info.disabled ? "En" : "Dis", tx, pstate->shutdown_timer);
    if(tx) {
        port_info_t port_info;
        if(port_info_get(pstate->port, &port_info) == VTSS_OK) {
            tx = port_info.link;
        } else {
            T_W("port_info_get(%u) failed, skip port tx", pstate->port);
                tx = FALSE;
        }
    }
    if(pstate->ttl_timer)
        --pstate->ttl_timer; /* Decrement TTL nocheck timer */
    if(pstate->info.disabled) { /* Has the port been shut down? */
        pstate->shutdown_timer++;
        if(lprot_conf.global.shutdown_time != 0 && 
           pstate->shutdown_timer >= lprot_conf.global.shutdown_time) {
            lprot_enable(pstate);
            if(tx)
                lprot_transmit(lprot_state.usid, egress_port, pstate); /* Immediately TX */
        }
    } else {
        if(pconf->transmit) { /* Are we an active transmitter? */
            pstate->transmit_timer++;
            if(tx &&
               pstate->transmit_timer >= lprot_conf.global.transmission_time) {
                lprot_transmit(lprot_state.usid, egress_port, pstate);
            }
        }
    }
}

/*
 * Main thread
 */

static void loop_periodic(void)
{
    T_N("loop protect tick");
    if(lprot_conf.global.enabled) {         /* Do we do anything ? */
        port_iter_t pit;
        /* Ports */
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, 
                              PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            const loop_protect_port_conf_t *pconf = &lprot_conf.ports[VTSS_ISID_LOCAL][pit.iport];
            lprot_port_state_t *pstate = &lprot_state.ports[pit.iport];
            /* Tick active ports */
            if(pconf->enabled)
                lprot_poag_tick(pstate, pconf);
        }
    }
}

static void rand_add(hash_state *hsp, const void *data)
{
    (void) tomcrypt_sha1_process(hsp, data, strlen(data));
}

// VLAN/MSTP/ERPS/.. ingress filtering
static BOOL lprot_ingress_filter(u32 src_port, vtss_vid_t vid)
{
    vtss_packet_filter_t     filter;
    vtss_packet_frame_info_t info;

    if (src_port == VTSS_PORT_NO_NONE) {
        return TRUE;            /* Filter frames on interconnect ports */
    }

    vtss_packet_frame_info_init(&info);
    info.port_no = src_port;
    info.vid = vid;

    if (vtss_packet_frame_filter(NULL, &info, &filter) == VTSS_RC_OK &&
        filter == VTSS_PACKET_FILTER_DISCARD) {
        T_I("frame discarded by ingress filtering, port: %u, vid: %u",
            iport2uport(src_port), info.vid);
        return TRUE;
    }

    return FALSE;
}

static BOOL rx_frame(void *contxt, 
                     const u8 *const frame, 
                     const vtss_packet_rx_info_t *rx_info)
{
    loop_prot_pdu_t *pdu = (loop_prot_pdu_t*) frame;
    u32 in_port = rx_info->port_no, out_port = pdu->lport;
    lprot_port_state_t *pstate;
    const loop_protect_port_conf_t *pconf;

    /* Apply ingress filtering */
    if (lprot_ingress_filter(rx_info->port_no, rx_info->tag.vid)) {
        return FALSE;
    }

    LPROT_CRIT_ENTER();

    pstate = &lprot_state.ports[in_port];
    pconf = &lprot_conf.ports[VTSS_ISID_LOCAL][in_port];

    T_D("Rx: %02x-%02x-%02x-%02x-%02x-%02x on port %u ", frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], iport2uport(in_port));

    /* Enabled for loop protection ? */
    if(!lprot_conf.global.enabled || !pconf->enabled) {
        T_D("Discard - not enabled globally or on port");
        goto discard;
    }

    if(memcmp(pdu->dst, dmac, sizeof(pdu->dst)) != 0 ||     /* Proper DST */
       memcmp(pdu->src, lprot_state.master_mac, sizeof(pdu->src)) != 0 || /* Proper SRC */
       pdu->oui != htons(etype) ||                          /* Proper OUI */
       pdu->version != LPROT_PROTVERSION) {                 /* Proper version */
        T_D("Discard - Basic PDU check fails (sda/sa, oui, protocol, version)");
    } else {
        u32 tdelta = (cyg_current_time() - pdu->tstamp);
        T_I("Rx: checking looped pdu port %u (from port %u:%u), delta(t): %u, ttl_timer %d", 
            iport2uport(in_port), pdu->usid, iport2uport(out_port), tdelta, pstate->ttl_timer);
        if((pstate->ttl_timer > 0) || /* Port just came up */
           pdu->usid != lprot_state.usid ||                     /* Other unit */
           (VTSS_OS_TICK2MSEC(tdelta) < LPROT_MAX_TTL_MSECS)) { /* OR recent PDU */
            u8 digest_pdu[SHA1_HASH_SIZE], digest_calc[SHA1_HASH_SIZE];
            memcpy(digest_pdu, pdu->authcode, sizeof(digest_pdu));
            memset(pdu->authcode, 0, sizeof(pdu->authcode));
            BOOL valid;
            if(lprot_calc_digest(pdu, digest_calc) &&
               memcmp(digest_pdu, digest_calc, sizeof(digest_calc)) == 0) {
                T_N("Digest checks OK");
                valid = TRUE;
            } else {
                T_I("Digest NOT OK");
                valid = FALSE;
            }
#ifdef VTSS_SW_OPTION_LOOP_DETECT
            /*
             * Give loop detection time to do its work on the
             * appropriate ports in the given time.
             */
            if(valid && vtss_lb_port(in_port) && vtss_lb_port(out_port)) {
                T_D("Rx: Dropping loopback on (active) loopback detect ports [%d,%d]", 
                    in_port, out_port);
                valid = FALSE;
            }
#endif
            if(valid) {
                lprot_disable(pstate);
            } else {
                T_N("Rx: Dropping frame");
            }
        } else {
            T_D("Rx: Frame too old (%u)", tdelta);
        }
    }
discard:
    LPROT_CRIT_EXIT();
    return(TRUE);
}

static void loop_protect_packet_register(void)
{
    packet_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.modid = VTSS_MODULE_ID_LOOP_PROTECT;
    filter.match = PACKET_RX_FILTER_MATCH_DMAC | PACKET_RX_FILTER_MATCH_SMAC | 
        PACKET_RX_FILTER_MATCH_ETYPE | PACKET_RX_FILTER_MATCH_ACL;
    filter.prio  = PACKET_RX_FILTER_PRIO_NORMAL;
    filter.etype = etype;
    memcpy(filter.dmac, dmac, sizeof(filter.dmac));
    memcpy(filter.smac, lprot_state.master_mac, sizeof(filter.smac));
    filter.cb    = rx_frame;

    if (loop_filter_id) {
        if (packet_rx_filter_change(&filter, &loop_filter_id) != VTSS_OK) {
            T_E("packet rx re-register failed");
        }
    } else if (packet_rx_filter_register(&filter, &loop_filter_id) != VTSS_OK) {
        T_E("packet rx register failed");
    }
}

static void port_change_callback(vtss_port_no_t port_no, port_info_t *info)
{
    if (!info->stack) {
        LPROT_CRIT_ENTER();
        lprot_port_state_t *pstate = &lprot_state.ports[port_no];
        if (info->link && lprot_conf.global.enabled) {
            const loop_protect_port_conf_t *pconf = &lprot_conf.ports[VTSS_ISID_LOCAL][port_no];
            if (!pstate->info.disabled)
                pstate->ttl_timer = LPROT_TTL_NOCHECK;
            if (!pstate->info.disabled && pconf->enabled && pconf->transmit) {
                lprot_transmit(lprot_state.usid, port_no, pstate);
            }
        }
        if (!info->link && pstate->info.loop_detect) {
            /* Reset state to initial state if link down and loop was active. */
            lprot_enable(pstate);
        }
        LPROT_CRIT_EXIT();
    }
}

/*
 * Message indication function
 */
static BOOL 
loop_protect_msg_rx(void *contxt, 
                    const void *rx_msg, 
                    size_t len, 
                    vtss_module_id_t modid, 
                    ulong isid)
{
    const loop_protect_msg_t *msg = (void*)rx_msg;
    T_D("Sid %u, rx %zd bytes, msg %d", isid, len, msg->msg_id);
    switch (msg->msg_id) {
    case LOOP_PROTECT_MSG_ID_CONF:
    {
        T_D("LOOP_PROTECT_MSG_ID_CONF");
        LPROT_CRIT_ENTER();
        memcpy(lprot_state.master_key, msg->data.unit_conf.key, sizeof(lprot_state.master_key));
        memcpy(lprot_state.master_mac, msg->data.unit_conf.mac, sizeof(lprot_state.master_mac));
        lprot_state.usid = msg->data.unit_conf.usid;
        lprot_conf.global = msg->data.unit_conf.global_conf;
        loop_protect_packet_register();
        memcpy(lprot_conf.ports[VTSS_ISID_LOCAL], msg->data.unit_conf.port_conf, sizeof(msg->data.unit_conf.port_conf));
        LPROT_CRIT_EXIT();
        lprot_conf_apply_local();
        break;
    }
    case LOOP_PROTECT_MSG_ID_CONF_PORT:
    {
        vtss_port_no_t port_no = msg->data.port_conf.port_no;
        T_D("LOOP_PROTECT_MSG_ID_CONF_PORT, port %d", port_no);
        LPROT_CRIT_ENTER();
        lprot_conf.ports[VTSS_ISID_LOCAL][port_no] = msg->data.port_conf.port_conf;
        LPROT_CRIT_EXIT();
        lprot_conf_apply_local();
        break;
    }
    case LOOP_PROTECT_MSG_ID_PORT_CTL:
    {
        vtss_port_no_t port_no = msg->data.port_ctl.port_no;
        BOOL disable = msg->data.port_ctl.disable;
        T_D("LOOP_PROTECT_MSG_ID_PORT_CTL, port %d, disable %d", port_no, disable);
        if(msg_switch_is_master()) {
            lprot_port_disable_master(isid, port_no, disable);
        } else {
            T_W("Skipping on non-master");
        }
        break;
    }
    case LOOP_PROTECT_MSG_ID_PORT_STATUS_REQ:
    {
        loop_protect_msg_t *rep = loop_protect_alloc_message(LOOP_PROTECT_MSG_ID_PORT_STATUS_RSP);
        if(rep) {
            port_iter_t pit;
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, 
                                  PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            LPROT_CRIT_ENTER();
            while (port_iter_getnext(&pit)) {
                rep->data.port_info.ports[pit.iport] = lprot_state.ports[pit.iport].info;
            }
            LPROT_CRIT_EXIT();
            msg_tx(VTSS_MODULE_ID_LOOP_PROTECT, isid, rep, sizeof(*rep));
        }
        break;
    }
    case LOOP_PROTECT_MSG_ID_PORT_STATUS_RSP:
    {
        if(msg_switch_is_master()) {
            port_iter_t pit;
            (void) port_iter_init(&pit, NULL, isid, 
                                  PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
            LPROT_CRIT_ENTER();
            while (port_iter_getnext(&pit)) {
                lprot_master_state[isid].ports[pit.iport] = msg->data.port_info.ports[pit.iport];
            }
            lprot_state.switches[isid].last_refresh = time(NULL);
            cyg_flag_setbits(&lprot_status_flags, 1<<isid);
            LPROT_CRIT_EXIT();
        } else {
            T_W("Skipping on non-master");
        }
        break;
    }
    default:
        T_W("Unhandled msg %d", msg->msg_id);
    }
    return TRUE;
}

/*
 * Stack Register
 */
static void
loop_protect_stack_register(void)
{
    msg_rx_filter_t filter;    
    memset(&filter, 0, sizeof(filter));
    filter.cb = loop_protect_msg_rx;
    filter.modid = VTSS_MODULE_ID_LOOP_PROTECT;
    vtss_rc rc =  msg_rx_filter_register(&filter);
    VTSS_ASSERT(rc == VTSS_OK);
}

static void lprot_init(void)
{
    hash_state hs;
    cyg_uint32 ts;
    uint i;

    /* memset(&lprot_state, 0, sizeof(lprot_state)); - this is BSS */
    for(i = 0; i < ARRSZ(lprot_state.ports); i++) {
        lprot_state.ports[i].port = i;
    }

    /* Initialize */
    (void) conf_mgmt_mac_addr_get(switchmac, 0);

    /* Generate keys */
    hal_clock_read(&ts);
    T_D("Start ts %u", ts);
#if defined(CYG_HAL_IRQCOUNT_SUPPORT)
#if defined(CYGNUM_HAL_INTERRUPT_TIMER0)
    ts += hal_irqcount_read(CYGNUM_HAL_INTERRUPT_TIMER0);
#endif
#if defined(CYGNUM_HAL_INTERRUPT_UART)
    ts += hal_irqcount_read(CYGNUM_HAL_INTERRUPT_UART);
#endif
#if defined(CYGNUM_HAL_INTERRUPT_FDMA)
    ts += hal_irqcount_read(CYGNUM_HAL_INTERRUPT_FDMA);
#endif
    T_D("Skew ts %u", ts);
#endif
    srand(ts);
    ts = rand();
    T_D("Start seed %u", ts);
    (void) tomcrypt_sha1_init(&hs);
    rand_add(&hs, "Vitesse Loopback Protection");
    rand_add(&hs, misc_software_version_txt());
    rand_add(&hs, misc_software_date_txt());
    (void) tomcrypt_sha1_process(&hs, (void*)&ts, sizeof(ts));
    (void) tomcrypt_sha1_done(&hs, lprot_key);
    T_I("Generated key: %02x-%02x-%02x-%02x", lprot_key[0], lprot_key[1], lprot_key[2], lprot_key[3]);

    if((lprot_hash = register_hash (&sha1_desc)) < 0) {
        T_E("Unable to register SHA1 hash function, configuration error");
    }
}

static void
lprot_thread(cyg_addrword_t data)
{
    /* Port change callback */
    (void) port_change_register(VTSS_MODULE_ID_LOOP_PROTECT, port_change_callback);

    /* Message module RX */
    loop_protect_stack_register();

    cyg_flag_init(&lprot_status_flags);

    for(;;) {
        cyg_tick_count_t wakeup = cyg_current_time() + (1000/ECOS_MSECS_PER_HWTICK);
        cyg_flag_value_t flags;
        while((flags = cyg_flag_timed_wait(&lprot_control_flags, 0xffff, 
                                           CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, wakeup))) {
            T_I("loop protect thread event, flags 0x%x", flags);
        }
        loop_periodic();
    }
}

/* Initialize module */
vtss_rc loop_protect_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;

#ifdef VTSS_SW_OPTION_ICFG
    vtss_rc     rc;
#endif

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        memset(&lprot_conf, 0, sizeof(lprot_conf));
        /* Create semaphore for critical regions */
        critd_init(&lprot_crit, "lprot_crit", VTSS_MODULE_ID_LOOP_PROTECT, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        LPROT_CRIT_EXIT();

#ifdef VTSS_SW_OPTION_ICFG
        rc = loop_protect_icfg_init();
        if ( rc != VTSS_OK ) {
            T_D("fail to init icfg registration for loop protect, rc = %s", error_txt(rc));
        }
#endif
        break;

    case INIT_CMD_START:
        T_D("START");
        lprot_init();
        cyg_flag_init( &lprot_control_flags );
        cyg_thread_create(THREAD_DEFAULT_PRIO, 
                          lprot_thread, 
                          0, 
                          "loop_prot", 
                          lprot_thread_stack, 
                          sizeof(lprot_thread_stack),
                          &lprot_thread_handle,
                          &lprot_thread_block);
        cyg_thread_resume(lprot_thread_handle);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset global configuration */
            loop_protect_conf_read(TRUE);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");
        /* Read and apply config */
        loop_protect_conf_read(FALSE);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        loop_protect_conf_send(isid);
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
