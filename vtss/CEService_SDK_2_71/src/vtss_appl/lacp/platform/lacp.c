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
#include "lacp.h"
#include "lacp_api.h"
#include "msg_api.h"
#include "aggr_api.h"
#include "port_api.h"
#include "conf_api.h"
#include "packet_api.h"
#include "vtss_ecos_mutex_api.h"
#include "misc_api.h"
#include "vtss_lacp.h"
#if defined(VTSS_SW_OPTION_DOT1X)
#include "dot1x_api.h"
#endif /* VTSS_SW_OPTION_DOT1X */
#ifdef VTSS_SW_OPTION_VCLI
#include "lacp_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICLI
#include "lacp_icfg.h"
#endif

#include "topo_api.h"
#define FLAG_ABORT         (1 << 0)
#define FLAG_LINK_UP       (1 << 1)
#define FLAG_LACP_ENABLED  (1 << 2)
static cyg_flag_t lacp_flags;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "lacp",
    .descr     = "LACP Module"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

static cyg_handle_t       lacp_thread_handle;
static cyg_thread         lacp_thread_block;
static char               lacp_thread_stack[THREAD_DEFAULT_STACK_SIZE];
static BOOL               lacp_disable;
#ifndef VTSS_SW_OPTION_MSTP
static cyg_handle_t       lacp_stp_thread_handle;
static cyg_thread         lacp_stp_thread_block;
static char               lacp_stp_thread_stack[THREAD_DEFAULT_STACK_SIZE];
#endif /* VTSS_SW_OPTION_MSTP */

static l2_port_no_t l2port_2_usidport[VTSS_ISID_CNT *VTSS_PORTS];
static l2_port_no_t l2usid_2_l2port[VTSS_ISID_CNT *VTSS_PORTS];

static struct {
    vtss_ecos_mutex_t coremutex;            /* LACP core library serialization */
    vtss_ecos_mutex_t datamutex;            /* LACP data region protection */
    lacp_conf_t conf;                       /* Current configuration */
} lacp_global;

typedef struct {
    ulong       version; /* Block version */
    lacp_conf_t conf;
} lacp_conf_blk_t;
#define LACP_CONF_VERSION 1

// Protects calls into the base module
#define LACP_CORE_LOCK()    VTSS_ECOS_MUTEX_LOCK(&lacp_global.coremutex);
#define LACP_CORE_UNLOCK()  VTSS_ECOS_MUTEX_UNLOCK(&lacp_global.coremutex);

// Protects lacp_global.conf
#define LACP_DATA_LOCK()    VTSS_ECOS_MUTEX_LOCK(&lacp_global.datamutex);
#define LACP_DATA_UNLOCK()  VTSS_ECOS_MUTEX_UNLOCK(&lacp_global.datamutex);

const vtss_common_macaddr_t vtss_lacp_slowmac   = {VTSS_LACP_MULTICAST_MACADDR};
const vtss_common_macaddr_t vtss_common_zeromac = {{ 0, 0, 0, 0, 0, 0 }};

/****************************************************************************/
// Port state callback function.
// This function is called if a port state change occurs.
/****************************************************************************/
static void lacp_port_state_change_callback(vtss_isid_t isid, vtss_port_no_t port_no, port_info_t *info)
{
    if (msg_switch_is_master() && !info->stack) {
        l2_port_no_t l2port = L2PORT2PORT(isid, port_no);

        T_D("port:%d %s\n", l2port, info->link ? "up" : "down");

        LACP_CORE_LOCK();
        vtss_lacp_linkstate_changed(l2port, info->link ?
                                    VTSS_COMMON_LINKSTATE_UP :
                                    VTSS_COMMON_LINKSTATE_DOWN);
        LACP_CORE_UNLOCK();

#ifndef VTSS_SW_OPTION_MSTP
        vtss_lacp_port_config_t pconf;
        /* When an LACP-enabled port get a link-up event, port is set to delayed forwarding. */
        /* This will give LACP time to form aggregation.                                     */
        (void)lacp_mgmt_port_conf_get(isid, port_no, &pconf);
        if (info->link) {
            if (pconf.enable_lacp) {
                /* Enable delayed forwarding  */
                cyg_flag_setbits(&lacp_flags, FLAG_LINK_UP);
            } else {
                vtss_os_set_stpstate(l2port, VTSS_STP_STATE_FORWARDING);
            }
        } else {
            vtss_os_set_stpstate(l2port, VTSS_STP_STATE_DISCARDING);
        }
#endif /* VTSS_SW_OPTION_MSTP */
    }
}

/****************************************************************************/
// Apply the configuration to the whole stack
/****************************************************************************/
static void lacp_propagate_conf(void)
{
    l2_port_no_t l2port;
    BOOL lacp_ena = 0;

    T_I("Making configuration effective");
    LACP_CORE_LOCK();
    vtss_lacp_set_config(&lacp_global.conf.system);

    for (l2port = VTSS_PORT_NO_START; l2port < (VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START); l2port++)  {
        vtss_lacp_set_portconfig(l2port, &lacp_global.conf.ports[l2port]);
        if (lacp_global.conf.ports[l2port].enable_lacp) {
            lacp_ena = 1;
        }
    }

    if (lacp_ena) {
        /* Wake up LACP thread */
        cyg_flag_setbits(&lacp_flags, FLAG_LACP_ENABLED);
        lacp_disable = 0;
    } else {
        /* Put LACP thread back to sleep */
        cyg_flag_maskbits(&lacp_flags, ~FLAG_LACP_ENABLED);
        lacp_disable = 1;
    }

    LACP_CORE_UNLOCK();
}

/****************************************************************************/
/****************************************************************************/
static void lacp_conf_read(BOOL force_defaults)
{
    lacp_conf_blk_t *lacp_blk;
    BOOL            do_create = force_defaults;
    ulong           size;

    T_D("enter, lacp_conf_read");

    if (misc_conf_read_use()) {
        /* Open or create LACP configuration block */
        if ((lacp_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_LACP_CONF, &size)) == NULL || size != sizeof(*lacp_blk)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            lacp_blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_LACP_CONF, sizeof(*lacp_blk));
            do_create = 1;
        } else if (lacp_blk->version != LACP_CONF_VERSION) {
            T_W("Version mismatch, creating defaults");
            do_create = 1;
        }
    } else {
        lacp_blk  = NULL;
        do_create = 1;
    }

    LACP_DATA_LOCK();
    if (do_create) {
        int i;

        /* Use default values */
        lacp_global.conf.system.system_prio = VTSS_LACP_DEFAULT_SYSTEMPRIO;
        memcpy(lacp_global.conf.system.system_id.macaddr, VTSS_COMMON_ZEROMAC, sizeof(VTSS_COMMON_ZEROMAC));

        for (i = 0; i < ARRSZ(lacp_global.conf.ports); i++) {
            lacp_global.conf.ports[i].port_prio = VTSS_LACP_DEFAULT_PORTPRIO;
            lacp_global.conf.ports[i].port_key = VTSS_LACP_AUTOKEY;
            lacp_global.conf.ports[i].enable_lacp = FALSE;
            lacp_global.conf.ports[i].xmit_mode = VTSS_LACP_DEFAULT_FSMODE;
            lacp_global.conf.ports[i].active_or_passive = VTSS_LACP_DEFAULT_ACTIVITY_MODE;
        }

        if (lacp_blk != NULL) {
            lacp_blk->conf = lacp_global.conf;
        }
    } else {
        /* Use stored configuration */
        T_D("Getting stored config");
        if (lacp_blk != NULL) {  // Quiet lint
            lacp_global.conf = lacp_blk->conf;
        }
    }
    LACP_DATA_UNLOCK();

#ifndef VTSS_SW_OPTION_MSTP
    {
        l2port_iter_t l2pit;

        /* Need to set all ports to STP forwarding - if not done by STP module */
        (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_PHYS | L2PORT_ITER_ISID_ALL);
        while (l2port_iter_getnext(&l2pit)) {
            if (lacp_global.conf.ports[l2pit.l2port].enable_lacp) {
                vtss_os_set_stpstate(l2pit.l2port, VTSS_STP_STATE_DISCARDING);
            } else {
                vtss_os_set_stpstate(l2pit.l2port, VTSS_STP_STATE_FORWARDING);
            }
        }
    }
#endif /* VTSS_SW_OPTION_MSTP */

    lacp_propagate_conf();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (lacp_blk == NULL) {
        T_E("Failed to open LACP config table");
    } else {
        lacp_blk->version = LACP_CONF_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_LACP_CONF);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("Exit: lacp_config_read");
}

/****************************************************************************/
// LACP packet reception on any switch
/****************************************************************************/
static BOOL rx_lacp(void  *contxt, const uchar *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    T_R("port_no: %d len %d vid %d glag %u", rx_info->port_no, rx_info->length, rx_info->tag.vid, rx_info->glag_no);

    // Check that the subtype is '1'
    if (frm[14] != 1) {
        return FALSE;  // Not a LACP frame
    }

    // NB: Null out the GLAG (port is 1st in aggr)
    l2_receive_indication(VTSS_MODULE_ID_LACP, frm, rx_info->length, rx_info->port_no, rx_info->tag.vid, (vtss_glag_no_t)(VTSS_GLAG_NO_START - 1));

    return FALSE; // Allow other subscribers to receive the packet
}

/****************************************************************************/
// Master only. Received packet from l2proto - send it to base
/****************************************************************************/
static void lacp_stack_receive(const void *packet,
                               size_t len,
                               vtss_vid_t vid,
                               l2_port_no_t l2port)
{
    T_R("RX port %d len %zd", l2port, len);
    if (msg_switch_is_master()) {
        LACP_CORE_LOCK();
        vtss_lacp_receive(l2port, packet, len);
        LACP_CORE_UNLOCK();
    }
}

/****************************************************************************/
// The LACP thread - if the switch is a master
/****************************************************************************/
static void lacp_thread(cyg_addrword_t data)
{
    void *filter_id;
    packet_rx_filter_t rx_filter;
    u32 lacp_sleep_soon = 0;

    /* Initialize LACP */
    vtss_lacp_init();

    /* Port change callback */
    (void) port_global_change_register(VTSS_MODULE_ID_LACP, lacp_port_state_change_callback);
    // Registration for LACP frames
    memset(&rx_filter, 0, sizeof(rx_filter));
    rx_filter.modid                 = VTSS_MODULE_ID_LACP;
    rx_filter.match                 = PACKET_RX_FILTER_MATCH_DMAC | PACKET_RX_FILTER_MATCH_ETYPE;
    rx_filter.cb                    = rx_lacp;
    rx_filter.prio                  = PACKET_RX_FILTER_PRIO_NORMAL;
    memcpy(rx_filter.dmac, VTSS_LACP_LACPMAC, sizeof(rx_filter.dmac));
    rx_filter.etype                 = 0x8809; // LACP

    (void) packet_rx_filter_register(&rx_filter, &filter_id);
    l2_receive_register(VTSS_MODULE_ID_LACP, lacp_stack_receive);

    /* Tick the LACP 'VTSS_LACP_TICKS_PER_SEC' per sec   */
    while (1) {
        while (msg_switch_is_master()) {
            /* If LACP gets disabled it goes to sleep after cleaning up the protocol states */
            if (lacp_disable) {
                lacp_sleep_soon++;
            }
            if (lacp_sleep_soon == 10) {
                cyg_flag_wait(&lacp_flags, FLAG_LACP_ENABLED, CYG_FLAG_WAITMODE_OR);
                lacp_sleep_soon = 0;
                lacp_disable = 0;
            }

            LACP_CORE_LOCK();
            vtss_lacp_tick();
            LACP_CORE_UNLOCK();
            VTSS_OS_MSLEEP(1000 / VTSS_LACP_TICKS_PER_SEC);
        }
        LACP_CORE_LOCK();
        vtss_lacp_deinit();
        LACP_CORE_UNLOCK();
        T_I("Suspending LACP thread (became slave)");
        cyg_thread_suspend(lacp_thread_handle);
        vtss_lacp_init();
    }
}

#ifndef VTSS_SW_OPTION_MSTP
/****************************************************************************/
/****************************************************************************/
static void lacp_stp_thread(cyg_addrword_t data)
{
    l2port_iter_t         l2pit;

    while (1) {
        /* Wait for a link up event */
        cyg_flag_value_t flags = cyg_flag_wait(&lacp_flags,
                                               FLAG_LINK_UP,
                                               CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
        /* Now pause for few seconds */
        flags = cyg_flag_timed_wait(&lacp_flags,
                                    FLAG_ABORT,
                                    CYG_FLAG_WAITMODE_OR,
                                    cyg_current_time() + VTSS_OS_MSEC2TICK(4 * 1000));

        /* Set ports STP state to forwarding */
        (void)l2port_iter_init(&l2pit, VTSS_ISID_GLOBAL, L2PORT_ITER_TYPE_PHYS);
        while (l2port_iter_getnext(&l2pit)) {
            if (vtss_os_get_stpstate(l2pit.l2port) == VTSS_STP_STATE_DISCARDING) {
                T_I("Setting state to forwarding port:%d", l2pit.l2port);
                vtss_os_set_stpstate(l2pit.l2port, VTSS_STP_STATE_FORWARDING);
            }
        }
    }
}
#endif /* VTSS_SW_OPTION_MSTP */

/****************************************************************************/
// Semi-public functions
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
void vtss_os_set_hwaggr(vtss_lacp_agid_t aid, vtss_common_port_t new_port)
{
    if (!msg_switch_is_master()) {
        return;
    }

    if (aggr_mgmt_lacp_members_add(aid, new_port) == AGGR_ERROR_REG_TABLE_FULL)  {
        /* Not possible to aggregate this port. Disable LACP for that port */
    }
}

/****************************************************************************/
/****************************************************************************/
void vtss_os_clear_hwaggr(vtss_lacp_agid_t aid, vtss_common_port_t old_port)
{
    if (!msg_switch_is_master()) {
        return;
    }
    aggr_mgmt_lacp_member_del(aid, old_port);
}

/****************************************************************************/
/****************************************************************************/
const char *vtss_common_str_macaddr(const vtss_common_macaddr_t VTSS_COMMON_PTR_ATTRIB *mac)
{
    static char VTSS_COMMON_DATA_ATTRIB buf[24];

    sprintf(buf, "%02X-%02X-%02X-%02X-%02X-%02X",
            mac->macaddr[0], mac->macaddr[1], mac->macaddr[2],
            mac->macaddr[3], mac->macaddr[4], mac->macaddr[5]);
    return buf;
}

/****************************************************************************/
// vtss_os_make_key - Generate a key for a port that reflects its relationship
/****************************************************************************/
vtss_lacp_key_t vtss_os_make_key(vtss_common_port_t portno, vtss_lacp_key_t new_key)
{
    int speed;

    if (new_key == VTSS_LACP_AUTOKEY) {
        /* We don't really care what the value is when the link is down */
        speed = vtss_os_get_linkspeed(portno);

        if (speed == 12000) {
            return 7;
        } else if (speed == 10000) {
            return 6;
        } else if (speed == 5000) {
            return 5;
        } else if (speed == 2500) {
            return 4;
        } else if (speed == 1000) {
            return 3;
        } else if (speed == 100) {
            return 2;
        }  else {
            return 1;
        }
    }
    return new_key;
}

/****************************************************************************/
// Convert between l2 ports and usid ports
/****************************************************************************/
vtss_common_port_t vtss_os_translate_port(vtss_common_port_t l2port, BOOL from_core)
{
    if (from_core) {
        return l2port_2_usidport[l2port] + 1;    /* l2port -> usid port, plus one to make port 0 appear as port 1 */
    } else {
        return l2usid_2_l2port[l2port] + 1;    /* usid port -> l2port */
    }
}

/****************************************************************************/
/*  API functions                                                           */
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
char *lacp_error_txt(vtss_rc rc)
{
    switch (rc) {
    case LACP_ERROR_GEN:
        return "LACP generic error";
    case LACP_ERROR_STATIC_AGGR_ENABLED:
        return "Static aggregation is enabled";
    case LACP_ERROR_DOT1X_ENABLED:
        return "DOT1X is enabled";
    default:
        return "LACP unknown error";
    }
}

/****************************************************************************/
/****************************************************************************/
vtss_rc lacp_mgmt_system_conf_get(vtss_lacp_system_config_t *conf)
{
    if (!msg_switch_is_master()) {
        return VTSS_UNSPECIFIED_ERROR;
    }

    LACP_DATA_LOCK();
    *conf = lacp_global.conf.system;
    LACP_DATA_UNLOCK();
    return VTSS_RC_OK;
}

/****************************************************************************/
/****************************************************************************/
vtss_rc lacp_mgmt_system_conf_set(const vtss_lacp_system_config_t *conf)
{
    if (!msg_switch_is_master()) {
        return VTSS_UNSPECIFIED_ERROR;
    }

    LACP_DATA_LOCK();
    lacp_global.conf.system = *conf;
    LACP_DATA_UNLOCK();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    {
        lacp_conf_blk_t *blk;
        ulong size;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_LACP_CONF, &size)) != NULL) {
            if (size == sizeof(*blk)) {
                T_I("Saving configuration");
                blk->conf = lacp_global.conf;
            }
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_LACP_CONF);
        } else {
            return VTSS_UNSPECIFIED_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    /* Make effective in LACP core */
    LACP_CORE_LOCK();
    vtss_lacp_set_config(conf);
    LACP_CORE_UNLOCK();
    return VTSS_RC_OK;
}

/****************************************************************************/
/****************************************************************************/
vtss_rc lacp_mgmt_port_conf_get(vtss_isid_t isid, vtss_port_no_t port_no, vtss_lacp_port_config_t *pconf)
{
    l2_port_no_t l2port = L2PORT2PORT(isid, port_no);
    if (!msg_switch_is_master()) {
        return VTSS_UNSPECIFIED_ERROR;
    }

    LACP_DATA_LOCK();
    *pconf = lacp_global.conf.ports[l2port];
    LACP_DATA_UNLOCK();
    return VTSS_RC_OK;
}

/****************************************************************************/
/****************************************************************************/
vtss_rc lacp_mgmt_port_conf_set(vtss_isid_t isid, vtss_port_no_t port_no, const vtss_lacp_port_config_t *pconf)
{
    l2_port_no_t l2port = L2PORT2PORT(isid, port_no);
    aggr_mgmt_group_no_t  aggr_no;
    aggr_mgmt_group_member_t group;
    BOOL lacp_enabled = 0;

    if (!msg_switch_is_master()) {
        return VTSS_UNSPECIFIED_ERROR;
    }

    /* Check if the ports is a member of static aggregation */
    for (aggr_no = AGGR_MGMT_GROUP_NO_START; aggr_no < AGGR_MGMT_GROUP_NO_END; aggr_no++) {
        if (aggr_mgmt_port_members_get(isid, aggr_no, &group, 0) == VTSS_RC_OK) {
            if (group.entry.member[port_no]) {
                return LACP_ERROR_STATIC_AGGR_ENABLED;
            }
        }
    }

#if defined(VTSS_SW_OPTION_DOT1X)
    dot1x_switch_cfg_t switch_cfg;

    /* Check if dot1x is enabled */
    if (dot1x_mgmt_switch_cfg_get(isid, &switch_cfg) != VTSS_RC_OK) {
        return VTSS_UNSPECIFIED_ERROR;
    }
    if (switch_cfg.port_cfg[port_no - VTSS_PORT_NO_START].admin_state != NAS_PORT_CONTROL_FORCE_AUTHORIZED) {
        return LACP_ERROR_DOT1X_ENABLED;
    }
#endif /* VTSS_SW_OPTION_DOT1X */

    LACP_DATA_LOCK();
    lacp_global.conf.ports[l2port] = *pconf;
    LACP_DATA_UNLOCK();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    {
        lacp_conf_blk_t *blk;
        ulong size;
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_LACP_CONF, &size)) != NULL) {
            if (size == sizeof(*blk)) {
                T_I("Saving configuration");
                blk->conf = lacp_global.conf;
            }
            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_LACP_CONF);
        } else {
            return VTSS_UNSPECIFIED_ERROR;
        }
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    LACP_CORE_LOCK();
    vtss_lacp_set_portconfig((l2port), pconf);
    LACP_CORE_UNLOCK();

    for (l2port = VTSS_PORT_NO_START; l2port < (VTSS_LACP_MAX_PORTS + VTSS_PORT_NO_START); l2port++)  {
        if (lacp_global.conf.ports[l2port].enable_lacp) {
            lacp_enabled = 1;
            break;
        }
    }

    if (lacp_enabled) {
        /* Wake up LACP thread */
        cyg_flag_setbits( &lacp_flags, FLAG_LACP_ENABLED);
        lacp_disable = 0;
    } else {
        /* Put LACP thread back to sleep */
        cyg_flag_maskbits( &lacp_flags, ~FLAG_LACP_ENABLED);
        lacp_disable = 1;
    }
    return VTSS_RC_OK;
}

/****************************************************************************/
/****************************************************************************/
vtss_common_bool_t lacp_mgmt_aggr_status_get(unsigned int aid, vtss_lacp_aggregatorstatus_t *stat)
{
    vtss_common_bool_t res;
    if (!msg_switch_is_master()) {
        return VTSS_COMMON_BOOL_FALSE;
    }

    LACP_CORE_LOCK();
    res = vtss_lacp_get_aggr_status(aid, stat);
    LACP_CORE_UNLOCK();
    return res;
}

/****************************************************************************/
/****************************************************************************/
vtss_rc lacp_mgmt_port_status_get(l2_port_no_t l2port, vtss_lacp_portstatus_t *stat)
{
    if (!msg_switch_is_master()) {
        return VTSS_UNSPECIFIED_ERROR;
    }

    LACP_CORE_LOCK();
    vtss_lacp_get_port_status(l2port, stat);
    LACP_CORE_UNLOCK();
    return VTSS_RC_OK;
}

/****************************************************************************/
/****************************************************************************/
void lacp_mgmt_statistics_clear(l2_port_no_t l2port)
{
    LACP_CORE_LOCK();
    vtss_lacp_clear_statistics(l2port);
    LACP_CORE_UNLOCK();
}

/****************************************************************************/
/****************************************************************************/
vtss_rc lacp_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
    vtss_port_no_t port_no;
    l2_port_no_t l2port, usidport;

    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        memset(&lacp_global.datamutex, 0, sizeof(lacp_global.datamutex));
        lacp_global.datamutex.type = VTSS_ECOS_MUTEX_TYPE_NORMAL;
        vtss_ecos_mutex_init(&lacp_global.datamutex);

        memset(&lacp_global.coremutex, 0, sizeof(lacp_global.coremutex));
        lacp_global.coremutex.type = VTSS_ECOS_MUTEX_TYPE_NORMAL;
        vtss_ecos_mutex_init(&lacp_global.coremutex);

        cyg_flag_init(&lacp_flags);

#ifdef VTSS_SW_OPTION_VCLI
        lacp_cli_req_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if (lacp_icfg_init() != VTSS_RC_OK) {
            T_D("Calling lacp_icfg_init() failed");
        }
#endif

        // Create thread(s)
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          lacp_thread,
                          0,
                          "LACP Control",
                          lacp_thread_stack,
                          sizeof(lacp_thread_stack),
                          &lacp_thread_handle,
                          &lacp_thread_block);
#ifndef VTSS_SW_OPTION_MSTP
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          lacp_stp_thread,
                          0,
                          "LACP Port FWD",
                          lacp_stp_thread_stack,
                          sizeof(lacp_stp_thread_stack),
                          &lacp_stp_thread_handle,
                          &lacp_stp_thread_block);
#endif /* VTSS_SW_OPTION_MSTP */
        break;

    case INIT_CMD_START:
#ifndef VTSS_SW_OPTION_MSTP
        cyg_thread_resume(lacp_stp_thread_handle);
#endif /* VTSS_SW_OPTION_MSTP */
        break;

    case INIT_CMD_CONF_DEF:
        if (isid == VTSS_ISID_GLOBAL) {
            /* Reset configuration to default */
            lacp_conf_read(TRUE);
        }
        break;

    case INIT_CMD_MASTER_UP:
        lacp_conf_read(FALSE);

        T_I("Starting LACP thread (became master)");
        cyg_thread_resume(lacp_thread_handle);
        break;

    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Convert between usid and l2ports and cache it */
        for (port_no = VTSS_PORT_NO_START; port_no < data->switch_info[isid].port_cnt; port_no++) {
            usidport = L2PORT2PORT(topo_isid2usid(isid), port_no);
            l2port = L2PORT2PORT(isid, port_no);
            l2port_2_usidport[l2port] = usidport;
            l2usid_2_l2port[usidport] = l2port;
        }
        break;

    case INIT_CMD_SWITCH_DEL:
        T_D("SWITCH_DEL, isid: %d", isid);
        break;

    default:
        break;
    }

    return 0;
}

