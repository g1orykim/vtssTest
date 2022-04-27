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
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "misc_api.h"
#include "ip2_api.h"
#include "packet_api.h"
#include "dhcp_relay_api.h"
#include "dhcp_relay.h"
#include "port_api.h"
#include "dhcp_helper_api.h"
#include "mac_api.h"
#include "vlan_api.h"

#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"
#endif

#include <network.h>
#include <netinet/udp.h>

#define DHCP_RELAY_USING_ISCDHCP_PACKAGE      1

#define CYG_DHCP_RELAY_MSEC2TICK(msec) (msec / (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000))

#if DHCP_RELAY_USING_ISCDHCP_PACKAGE
#include "iscdhcp_ecos.h"
#include "dhcp_relay_callout.h"
#endif /* DHCP_RELAY_USING_ISCDHCP_PACKAGE */

#ifdef VTSS_SW_OPTION_VCLI
#include "dhcp_relay_cli.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "dhcp_relay_icfg.h"
#endif /* VTSS_SW_OPTION_ICFG */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_DHCP_RELAY

/* Callback functions */
static int DHCP_RELAY_update_circuit_id_callback(u8 *mac, uint transaction_id, u8 *circuit_id);
static int DHCP_RELAY_check_circuit_id_callback(u8 *mac, uint transaction_id, u8 *circuit_id);
static int DHCP_RELAY_send_client_callback(char *raw, size_t len, struct sockaddr_in *to, u8 *mac, uint transaction_id);
static void DHCP_RELAY_send_server_callback(char *raw, size_t len, unsigned long srv_ip);
static void DHCP_RELAY_fill_giaddr_callback(u8 *mac, uint transaction_id, vtss_ipv4_t *agent_ipv4_addr);
static void dhcp_relay_local_stats_clear(void);


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static dhcp_relay_global_t DHCP_RELAY_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t DHCP_RELAY_trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "dhcp_relay",
    .descr     = "DHCP relay"
};

static vtss_trace_grp_t DHCP_RELAY_trace_grps[TRACE_GRP_CNT] = {
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
#define DHCP_RELAY_CRIT_ENTER() critd_enter(&DHCP_RELAY_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define DHCP_RELAY_CRIT_EXIT()  critd_exit( &DHCP_RELAY_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define DHCP_RELAY_CRIT_ENTER() critd_enter(&DHCP_RELAY_global.crit)
#define DHCP_RELAY_CRIT_EXIT()  critd_exit( &DHCP_RELAY_global.crit)
#endif /* VTSS_TRACE_ENABLED */

/* Thread variables */
#define DHCP_RELAY_CERT_THREAD_STACK_SIZE       16384
static cyg_handle_t DHCP_RELAY_thread_handle;
static cyg_thread   DHCP_RELAY_thread_block;
static char         DHCP_RELAY_thread_stack[DHCP_RELAY_CERT_THREAD_STACK_SIZE];

/* Circuit ID (Sub-option 1):
   It indicates the information when agent receives DHCP message.
   The circuit ID field is 4 bytes in length and the format is ��<vlan_id><module_id><port_no>��.
   <vlan_id>:   The first two bytes represent the VLAN ID.
   <module_id>: The third byte is the module ID(in standalone switch it always equal 0, in stackable switch it means switch ID).
   <port_no>:   The fourth byte is the port number (1-based). */

typedef struct {
    u16 vlan_id;
    u8  module_id;
    u8  port_no;
} dhcp_relay_circuit_id_t;


/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* Determine if DHCP relay configuration has changed */
static int DHCP_RELAY_conf_changed(dhcp_relay_conf_t *old, dhcp_relay_conf_t *new)
{
    return (new->relay_mode != old->relay_mode
            || new->relay_server_cnt != old->relay_server_cnt
            || memcmp(new->relay_server, old->relay_server, sizeof(new->relay_server))
            || new->relay_info_mode != old->relay_info_mode
            || new->relay_info_policy != old->relay_info_policy);
}

/* Set DHCP relay defaults */
static void DHCP_RELAY_default_set(dhcp_relay_conf_t *conf)
{
    conf->relay_mode = DHCP4R_DEF_MODE;
    conf->relay_server_cnt = DHCP4R_DEF_SRV_CNT;
    memset(conf->relay_server, 0x0, sizeof(conf->relay_server));
    conf->relay_info_mode = DHCP4R_DEF_INFO_MODE;
    conf->relay_info_policy = DHCP4R_DEF_INFO_POLICY;
}

#ifndef IP_VHL_HL
#define IP_VHL_HL(vhl)      ((vhl) & 0x0f)
#endif /* IP_VHL_HL */

/*  Master only. Receive packet from DHCP helper - send it to TCP/IP stack  */
static BOOL DHCP_RELAY_stack_receive_callback(const u8 *const packet, size_t len, vtss_vid_t vid, vtss_isid_t src_isid, vtss_port_no_t src_port, vtss_glag_no_t glag_no)
{
    vtss_if_id_vlan_t   ifid;
    vtss_if_param_t     if_conf;

    T_D("enter, RX port:%d, glag_no:%d, vid:%d, len:%zd.", src_port, glag_no, vid, len);

    if (!msg_switch_is_master()) {
        return FALSE;
    }

    /* The incoming VLAN maybe not binding any IPv4 address.
       That we just need get any one valid interface ID to
       transmit the DHCP packet via socket layer */
    if (vtss_ip2_if_conf_get((vtss_if_id_vlan_t) vid, &if_conf) == VTSS_RC_OK) {
        if (vtss_ip2_if_inject((vtss_if_id_vlan_t) vid, len, packet)) {
            T_D("Calling vtss_ip2_if_inject(ifid:%d, len:%d) failed.\n", vid, len);
        }
    } else {
        for (ifid = VLAN_ID_MIN; ifid <= VLAN_ID_MAX; ifid++) {
            if (vtss_ip2_if_conf_get(ifid, &if_conf) == VTSS_RC_OK) {
                if (vtss_ip2_if_inject(ifid, len, packet)) {
                    T_D("Calling vtss_ip2_if_inject(ifid:%d, len:%d) failed.\n", ifid, len);
                }
                break;
            }
        }
    }

    T_D("exit");
    return TRUE;
}

#if DHCP_RELAY_USING_ISCDHCP_PACKAGE
/* Setup DHCP relay configuration to engine */
static void DHCP_RELAY_engine_conf_set(dhcp_relay_conf_t *conf)
{
    uint idx;

    iscdhcp_relay_mode_set((conf->relay_mode && conf->relay_server_cnt) ? DHCP_RELAY_MGMT_ENABLED : DHCP_RELAY_MGMT_DISABLED);
    iscdhcp_clear_dhcp_server();
    for (idx = 0; idx < DHCP_RELAY_MGMT_MAX_DHCP_SERVER; idx++) {
        iscdhcp_add_dhcp_server(conf->relay_server[idx]);
    }
    iscdhcp_set_agent_info_mode(conf->relay_info_mode);
    iscdhcp_set_relay_info_policy(conf->relay_info_policy);
}
#endif /* DHCP_RELAY_USING_ISCDHCP_PACKAGE */

static void DHCP_RELAY_conf_apply(void)
{
    if (msg_switch_is_master()) {
        DHCP_RELAY_CRIT_ENTER();
        if (DHCP_RELAY_global.dhcp_relay_conf.relay_mode && DHCP_RELAY_global.dhcp_relay_conf.relay_server_cnt) {
            dhcp_helper_user_receive_register(DHCP_HELPER_USER_RELAY, DHCP_RELAY_stack_receive_callback);
            dhcp_helper_user_clear_local_stat_register(DHCP_HELPER_USER_RELAY, dhcp_relay_local_stats_clear);
        } else {
            dhcp_helper_user_receive_unregister(DHCP_HELPER_USER_RELAY);
        }
#if DHCP_RELAY_USING_ISCDHCP_PACKAGE
        DHCP_RELAY_engine_conf_set(&DHCP_RELAY_global.dhcp_relay_conf);
#endif /* DHCP_RELAY_USING_ISCDHCP_PACKAGE */
        DHCP_RELAY_CRIT_EXIT();
    }
}

// Avoid Custodial pointer 'status' has not been freed or returned,
// the all_status_p is freed by the message module.
/*lint -e{429} */
/* Get first valid IPv4 interface status */
static BOOL DHCP_RELAY_get_linkup_ipv4_interface_status(vtss_if_status_t *status)
{
    vtss_if_id_vlan_t   search_vid = VTSS_VID_NULL, vid;
    vtss_if_status_t    ifstat;
    u32                 ifct;

    while (vtss_ip2_if_id_next(search_vid, &vid) == VTSS_OK) {
        if (vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_IPV4, vid, 1, &ifct, &ifstat) == VTSS_OK &&
            ifct == 1 &&
            ifstat.type == VTSS_IF_STATUS_TYPE_IPV4) {
            *status = ifstat;
            return TRUE;
        }
        search_vid = vid;
    }

    return FALSE;
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* DHCP relay error text */
char *dhcp_relay_error_txt(vtss_rc rc)
{
    switch (rc) {
    case DHCP_RELAY_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case DHCP_RELAY_ERROR_INV_PARAM:
        return "Invalid parameter supplied to function";

    default:
        return "DHCP Relay: Unknown error code";
    }
}

/* Get DHCP relay configuration */
vtss_rc dhcp_relay_mgmt_conf_get(dhcp_relay_conf_t *glbl_cfg)
{
    T_D("enter");

    if (glbl_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return DHCP_RELAY_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return DHCP_RELAY_ERROR_MUST_BE_MASTER;
    }

    DHCP_RELAY_CRIT_ENTER();
    *glbl_cfg = DHCP_RELAY_global.dhcp_relay_conf;
    DHCP_RELAY_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Set DHCP relay configuration */
vtss_rc dhcp_relay_mgmt_conf_set(dhcp_relay_conf_t *glbl_cfg)
{
    vtss_rc rc      = VTSS_OK;
    int     changed = 0;

    T_D("enter, relay mode: %d, relay server: %d, relay information mode: %d, relay information policy: %d", glbl_cfg->relay_mode, glbl_cfg->relay_server[0], glbl_cfg->relay_info_mode, glbl_cfg->relay_info_policy);

    if (glbl_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return DHCP_RELAY_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return DHCP_RELAY_ERROR_MUST_BE_MASTER;
    }

    /* check illegal parameter */
    if (glbl_cfg->relay_info_mode == DHCP_RELAY_MGMT_DISABLED && glbl_cfg->relay_info_policy == DHCP_RELAY_INFO_POLICY_REPLACE) {
        return DHCP_RELAY_ERROR_INV_PARAM;
    }

    DHCP_RELAY_CRIT_ENTER();
    changed = DHCP_RELAY_conf_changed(&DHCP_RELAY_global.dhcp_relay_conf, glbl_cfg);
    DHCP_RELAY_global.dhcp_relay_conf = *glbl_cfg;
    DHCP_RELAY_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t         blk_id = CONF_BLK_DHCP_RELAY_CONF;
        dhcp_relay_conf_blk_t *dhcp_relay_conf_blk_p;
        if ((dhcp_relay_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open DHCP relay table");
        } else {
            dhcp_relay_conf_blk_p->dhcp_relay_conf = *glbl_cfg;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        /* Activate changed configuration */
        DHCP_RELAY_conf_apply();
    }

    T_D("exit");

    return rc;
}

/****************************************************************************
 * Module thread
 ****************************************************************************/
/* We create a new thread to do it for instead of in 'Init Modules' thread.
   That we don't need wait a long time in 'Init Modules' thread. */
static void DHCP_RELAY_thread(cyg_addrword_t data)
{
#if DHCP_RELAY_USING_ISCDHCP_PACKAGE
    vtss_if_status_t status;

    while (1) {
        if (msg_switch_is_master()) {
            if (DHCP_RELAY_get_linkup_ipv4_interface_status(&status)) {
                break;
            }
        } else {
            // No reason for using CPU ressources when we're a slave
            T_D("Suspending DHCP relay thread");
            cyg_thread_suspend(DHCP_RELAY_thread_handle);
            T_D("Resumed DHCP relay thread");
        }
        cyg_thread_delay(CYG_DHCP_RELAY_MSEC2TICK(1000));
    };

    /* There's a forever loop in the function */
    (void) iscdhcp_init();
#else
    while (1) {
        VTSS_OS_MSLEEP(1000);
    }
#endif /* DHCP_RELAY_USING_ISCDHCP_PACKAGE */
}


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create DHCP relay stack configuration */
static void DHCP_RELAY_conf_read_stack(BOOL create)
{
    int                     changed;
    BOOL                    do_create;
    u32                     size;
    dhcp_relay_conf_t       *old_dhcp_relay_conf_p, new_dhcp_relay_conf;
    dhcp_relay_conf_blk_t   *conf_blk_p;
    conf_blk_id_t           blk_id;
    u32                     blk_version;

    T_D("enter, create: %d", create);

    blk_id = CONF_BLK_DHCP_RELAY_CONF;
    blk_version = DHCP_RELAY_CONF_BLK_VERSION;

    if (misc_conf_read_use()) {
        /* Read/create DHCP relay configuration */
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
    DHCP_RELAY_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        DHCP_RELAY_default_set(&new_dhcp_relay_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->dhcp_relay_conf = new_dhcp_relay_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {
            new_dhcp_relay_conf = conf_blk_p->dhcp_relay_conf;
        }
    }
    old_dhcp_relay_conf_p = &DHCP_RELAY_global.dhcp_relay_conf;
    if (DHCP_RELAY_conf_changed(old_dhcp_relay_conf_p, &new_dhcp_relay_conf)) {
        changed = 1;
    }
    DHCP_RELAY_global.dhcp_relay_conf = new_dhcp_relay_conf;
    DHCP_RELAY_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open DHCP relay table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed && create) { //Always set when topology change
        DHCP_RELAY_conf_apply();
    }

    T_D("exit");
}

/* Module start */
static void DHCP_RELAY_start(void)
{
    dhcp_relay_conf_t *conf_p;

    T_D("enter");

    /* Initialize DHCP relay configuration */
    conf_p = &DHCP_RELAY_global.dhcp_relay_conf;
    DHCP_RELAY_default_set(conf_p);

    /* Create semaphore for critical regions */
    critd_init(&DHCP_RELAY_global.crit, "DHCP_RELAY_global.crit", VTSS_MODULE_ID_DHCP_RELAY, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    DHCP_RELAY_CRIT_EXIT();

    /* Create DHCP relay thread */
    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      DHCP_RELAY_thread,
                      0,
                      "DHCP Relay",
                      DHCP_RELAY_thread_stack,
                      sizeof(DHCP_RELAY_thread_stack),
                      &DHCP_RELAY_thread_handle,
                      &DHCP_RELAY_thread_block);

    T_D("exit");
}

/* Initialize module */
vtss_rc dhcp_relay_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
    u8          mac[6];

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&DHCP_RELAY_trace_reg, DHCP_RELAY_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&DHCP_RELAY_trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        DHCP_RELAY_start();
#ifdef VTSS_SW_OPTION_VCLI
        dhcp_relay_cli_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if (dhcp_relay_icfg_init() != VTSS_OK) {
            T_E("dhcp_relay_icfg_init failed!");
        }
#endif /* VTSS_SW_OPTION_ICFG */
        break;
    case INIT_CMD_START:
        T_D("START");
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            DHCP_RELAY_conf_read_stack(1);
            dhcp_relay_stats_clear();
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");

        /* Read stack and switch configuration */
        DHCP_RELAY_conf_read_stack(0);

        if (conf_mgmt_mac_addr_get(mac, 0) >= 0) {
#if DHCP_RELAY_USING_ISCDHCP_PACKAGE
            iscdhcp_set_platform_mac(mac);
            iscdhcp_set_remote_id(mac);
            iscdhcp_reply_update_circuit_id_register(DHCP_RELAY_update_circuit_id_callback);
            iscdhcp_reply_check_circuit_id_register(DHCP_RELAY_check_circuit_id_callback);
            iscdhcp_reply_send_client_register(DHCP_RELAY_send_client_callback);
            iscdhcp_reply_send_server_register(DHCP_RELAY_send_server_callback);
            iscdhcp_reply_fill_giaddr_register(DHCP_RELAY_fill_giaddr_callback);
#endif /* DHCP_RELAY_USING_ISCDHCP_PACKAGE */
        }

        /* Starting DHCP relay thread (became master) */
        cyg_thread_resume(DHCP_RELAY_thread_handle);
        dhcp_relay_stats_clear();
        break;
    }
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply all configuration to switch */
        if (msg_switch_is_local(isid)) {
            DHCP_RELAY_conf_apply();
        }
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
/*  Statistics functions                                                    */
/****************************************************************************/

/* Get DHCP relay statistics */
void dhcp_relay_stats_get(dhcp_relay_stats_t *stats)
{
#if DHCP_RELAY_USING_ISCDHCP_PACKAGE
    iscdhcp_get_couters((iscdhcp_relay_counter_t *)stats);
#else
    memset(stats, 0x0, sizeof(*stats));
#endif /* DHCP_RELAY_USING_ISCDHCP_PACKAGE */
}

/* Clear DHCP relay local statistics */
static void dhcp_relay_local_stats_clear(void)
{
#if DHCP_RELAY_USING_ISCDHCP_PACKAGE
    iscdhcp_clear_couters();
#endif /* DHCP_RELAY_USING_ISCDHCP_PACKAGE */
}

/* Clear DHCP relay statistics */
void dhcp_relay_stats_clear(void)
{
    /* Clear the local statistics and DHCP helper detail statistics */
    (void) dhcp_helper_stats_clear_by_user(DHCP_HELPER_USER_RELAY);
}


/****************************************************************************/
/*  Recored system IP address functions                                     */
/****************************************************************************/

/* Notify DHCP relay module when system IP address changed */
void dhcp_realy_sysip_changed(u32 ip_addr)
{
    struct in_addr ia;

    if (!ip_addr) {
        return;
    }
    ia.s_addr = htonl(ip_addr);
#if DHCP_RELAY_USING_ISCDHCP_PACKAGE
    iscdhcp_change_interface_addr("eth", &ia);
#endif /* DHCP_RELAY_USING_ISCDHCP_PACKAGE */
}


/****************************************************************************/
/*  Send to client functions                                                */
/****************************************************************************/

/* Do IP checksum */
static u16 DHCP_RELAY_do_ip_chksum(u16 ip_hdr_len, u16 ip_hdr[])
{
    u16  padd = (ip_hdr_len % 2);
    u16  word16;
    u32 sum = 0;
    int i;

    /* Calculate the sum of all 16 bit words */
    for (i = 0; i < (ip_hdr_len / 2); i++) {
        word16 = ip_hdr[i];
        sum += (u32)word16;
    }

    /* Add odd byte if needed */
    if (padd == 1) {
        word16 = ip_hdr[(ip_hdr_len / 2)] & 0xFF00;
        sum += (u32)word16;
    }

    /* Keep only the last 16 bits */
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    /* One's complement of sum */
    sum = ~sum;

    return ((u16) sum);
}

/* do UDP checksum */
static u16 DHCP_RELAY_do_udp_chksum(u16 udp_len, u16 src_addr[], u16 dest_addr[], u16 udp_hdr[])
{
    u16  protocol_udp = htons(17);
    u16  padd = (udp_len % 2);
    u16  word16;
    u32 sum = 0;
    int i;

    /* Calculate the sum of all 16 bit words */
    for (i = 0; i < (udp_len / 2); i++) {
        word16 = udp_hdr[i];
        sum += (u32)word16;
    }

    /* Add odd byte if needed */
    if (padd == 1) {
        word16 = udp_hdr[(udp_len / 2)] & htons(0xFF00);
        sum += (u32)word16;
    }

    /* Calculate the UDP pseudo header */
    for (i = 0; i < 2; i++) {   //SIP
        word16 = src_addr[i];
        sum += (u32)word16;
    }
    for (i = 0; i < 2; i++) {   //DIP
        word16 = dest_addr[i];
        sum += (u32)word16;
    }
    sum += (u32)(protocol_udp + htons(udp_len));  //Protocol number and UDP length

    /* Keep only the last 16 bits */
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    /* One's complement of sum */
    sum = ~sum;

    return ((u16) sum);
}

/* Callback function for update circut ID
   Return -1: circut ID is invalid
   Return  0: circut ID is valid */
static int DHCP_RELAY_update_circuit_id_callback(u8 *mac, uint transaction_id, u8 *circuit_id)
{
    dhcp_helper_frame_info_t    client_info;
    dhcp_relay_circuit_id_t     temp_circuit_id;

    if (!circuit_id || !dhcp_helper_frame_info_lookup(mac, 0, transaction_id, &client_info)) {
        /* Could not find the client interface */
        return -1;
    }

    /* caculate option 82 information */
    temp_circuit_id.vlan_id = htons(client_info.vid);

#if VTSS_SWITCH_STACKABLE
    temp_circuit_id.module_id = (u8)(topo_isid2usid(client_info.isid));
#else
    temp_circuit_id.module_id = 0;
#endif /* VTSS_SWITCH_STACKABLE */

    temp_circuit_id.port_no = (u8)(iport2uport(client_info.port_no));

    memcpy(circuit_id, &temp_circuit_id, sizeof(temp_circuit_id));

    return 0;
}

/* Callback function for check circut ID
   Return -1: circut ID is invalid
   Return  0: circut ID is valid */
static int DHCP_RELAY_check_circuit_id_callback(u8 *mac, uint transaction_id, u8 *circuit_id)
{
    dhcp_helper_frame_info_t    client_info;
    dhcp_relay_circuit_id_t     temp_circuit_id;

    if (!circuit_id || !dhcp_helper_frame_info_lookup(mac, 0, transaction_id, &client_info)) {
        /* Could not find the client interface */
        return -1;
    }

    /* caculate option 82 information */
    temp_circuit_id.vlan_id = htons(client_info.vid);

#if VTSS_SWITCH_STACKABLE
    temp_circuit_id.module_id = (u8)(topo_isid2usid(client_info.isid));
#else
    temp_circuit_id.module_id = 0;
#endif /* VTSS_SWITCH_STACKABLE */

    temp_circuit_id.port_no = (u8)(iport2uport(client_info.port_no));

    if (memcmp(&temp_circuit_id, circuit_id, sizeof(temp_circuit_id))) {
        return -1;
    }

    return 0;
}

/* Callback function for send DHCP message to client
   Return -1: send packet fail
   Return  0: send packet success */
static int DHCP_RELAY_send_client_callback(char *raw, size_t len, struct sockaddr_in *to, u8 *mac, uint transaction_id)
{
    u8                          *pkt_buf;
    void                        *bufref;
    int                         pkt_len = 14 + 20 + 8 + len;
    u8                          broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    struct ip                   *ip_hdr;
    struct udphdr               *udp_hdr;
    u8                          system_mac_addr[6];
    vtss_if_status_t            ifstat;
    u32                         ifct;
    dhcp_helper_frame_info_t    client_info;
    static u16                  DHCP_RELAY_send_to_client_ip_id = 0;
    int                         rc;

    if (!dhcp_helper_frame_info_lookup(mac, 0, transaction_id, &client_info)) {
        /* Could not find the client interface */
        return -1;
    }

    if ((pkt_buf = dhcp_helper_alloc_xmit(pkt_len, client_info.isid, &bufref)) == NULL) {
        /* Allocate fail */
        return -1;
    }

    //da, sa, type
    memset(pkt_buf, 0x0, sizeof(*pkt_buf));
    memcpy(pkt_buf, broadcast_mac, 6);
    if (conf_mgmt_mac_addr_get(system_mac_addr, 0) < 0) {
        return -1;
    }
    memcpy(pkt_buf + 6, system_mac_addr, 6);
    *(pkt_buf + 12) = 0x08; //IP
    *(pkt_buf + 13) = 0x0;

    /* IP header */
    ip_hdr = (struct ip *)(pkt_buf + 14);
    ip_hdr->ip_hl = 0x5;
    ip_hdr->ip_v = 0x4;
    ip_hdr->ip_tos = 0x0;
    DHCP_RELAY_CRIT_ENTER();
    ip_hdr->ip_id = htons((++DHCP_RELAY_send_to_client_ip_id));
    DHCP_RELAY_CRIT_EXIT();
    ip_hdr->ip_off = 0x0;
    ip_hdr->ip_ttl = 128;
    ip_hdr->ip_p = 0x11; //UDP
    if ((vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_IPV4, (vtss_if_id_vlan_t) client_info.vid, 1, &ifct, &ifstat) == VTSS_OK &&
         ifct == 1 &&
         ifstat.type == VTSS_IF_STATUS_TYPE_IPV4) ||
        (DHCP_RELAY_get_linkup_ipv4_interface_status(&ifstat))) {
        ip_hdr->ip_src.s_addr = htonl(ifstat.u.ipv4.net.address);
    }
    ip_hdr->ip_dst.s_addr = 0xFFFFFFFF; //IP broadcast
    ip_hdr->ip_len = htons(20 + 8 + len);
    ip_hdr->ip_sum = 0; //clear before do checksum
    ip_hdr->ip_sum = DHCP_RELAY_do_ip_chksum(20, (u16 *)&pkt_buf[14]);

    /* UDP header */
    udp_hdr = (struct udphdr *)(pkt_buf + 14 + 20);
    udp_hdr->uh_sport = htons(67);
    udp_hdr->uh_dport = htons(68);
    udp_hdr->uh_ulen = htons(8 + len);

    //dhcp message
    memcpy(pkt_buf + 14 + 20 + 8, raw, len);
    udp_hdr->uh_sum = 0; //clear before do checksum
    udp_hdr->uh_sum = DHCP_RELAY_do_udp_chksum((u16)(8 + len), (u16 *)&pkt_buf[14 + 12], (u16 *)&pkt_buf[14 + 16], (u16 *)&pkt_buf[14 + 20]);

    rc = dhcp_helper_xmit(DHCP_HELPER_USER_RELAY, pkt_buf, pkt_len, client_info.vid, client_info.isid, VTSS_BIT64(client_info.port_no), VTSS_ISID_END, VTSS_PORT_NO_NONE, VTSS_GLAG_NO_NONE, bufref);
    return (rc);
}

/* Callback function for DHCP Relay base module after send out the DHCP packet successfully.
   It is used to count the per-port statistic. */
static void DHCP_RELAY_send_server_callback(char *raw, size_t len, unsigned long srv_ip)
{
    vtss_isid_t             isid_idx;
    vtss_vid_mac_t          vid_mac;
    vtss_mac_table_entry_t  mac_entry;
    char                    ip_str_buf[16];
    u32                     cnt, i, j;
#define DHCP_RELAY_ARP_MAX  1024
    vtss_neighbour_status_t *status = VTSS_MALLOC(sizeof(vtss_neighbour_status_t) * DHCP_RELAY_ARP_MAX);

    if (status == NULL) {
        T_W("Internal Allocate memory ");
        return;
    }

    if (vtss_ip2_nb_status_get(VTSS_IP_TYPE_IPV4, DHCP_RELAY_ARP_MAX, &cnt, status) != VTSS_OK) {
        VTSS_FREE(status);
        return;
    }

    /* Lookup MAC address in ARP table */
    memset(&vid_mac, 0, sizeof(vid_mac));
    for (i = 0; i < cnt; i++) {
        if (status[i].ip_address.addr.ipv4 != srv_ip) {
            continue;
        }
        T_D("Find DHCP Relay server ip address %s", misc_ipv4_txt((vtss_ipv4_t)srv_ip, ip_str_buf));

        vid_mac.vid = status[i].interface.u.vlan;
        for (j = 0; j < 6; j++) {
            vid_mac.mac.addr[j] = status[i].mac_address.addr[j];
        }
    }
    VTSS_FREE(status);

    /* Lookup source port in MAC address table */
    if (vid_mac.vid) {  // Found ARP entry
        for (isid_idx = VTSS_ISID_START; isid_idx < VTSS_ISID_END; isid_idx++) {
            if (!msg_switch_exists(isid_idx)) {
                continue;
            }
            if (mac_mgmt_table_get_next(isid_idx, &vid_mac, &mac_entry, FALSE) == VTSS_OK) {
                vtss_port_no_t port_idx;
                for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
                    if (mac_entry.destination[port_idx]) {
                        struct bootp bp;
                        u8           dhcp_message;

                        memcpy(&bp, raw, sizeof(bp));
                        dhcp_message = bp.bp_vend[6];
                        DHCP_HELPER_stats_add(DHCP_HELPER_USER_RELAY, isid_idx, VTSS_BIT64(port_idx), dhcp_message, DHCP_HELPER_DIRECTION_TX);
                        break;
                    }
                }
                break;
            }
        }
    }
}

static void DHCP_RELAY_fill_giaddr_callback(u8 *mac, uint transaction_id, vtss_ipv4_t *agent_ipv4_addr)
{
    dhcp_helper_frame_info_t    client_info;
    vtss_if_status_t            status;
    u32                         ifct;

    if (dhcp_helper_frame_info_lookup(mac, 0, transaction_id, &client_info) &&
        (vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_IPV4, client_info.vid, 1, &ifct, &status) == VTSS_OK &&
         ifct == 1 &&
         status.type == VTSS_IF_STATUS_TYPE_IPV4)) {
        *agent_ipv4_addr = htonl(status.u.ipv4.net.address);
    } else { // Find the first link-up interface
        if (DHCP_RELAY_get_linkup_ipv4_interface_status(&status)) {
            *agent_ipv4_addr = htonl(status.u.ipv4.net.address);
        }
    }
}

#if DHCP_RELAY_USING_ISCDHCP_PACKAGE
/****************************************************************************/
/*  DHCP Base Module callout implementations                                */
/****************************************************************************/
void *dhcp_relay_callout_malloc(size_t size)
{
    return VTSS_MALLOC(size);
}

void dhcp_relay_callout_free(void *ptr)
{
    VTSS_FREE(ptr);
}

char *dhcp_relay_callout_strdup(const char *str)
{
    return VTSS_STRDUP(str);
}
#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

