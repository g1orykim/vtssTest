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
#include "conf_api.h"
#include "msg_api.h"
#include "port_api.h"
#include "critd_api.h"
#include "vlan_api.h"
#include "mgmt_api.h"
#include "vtss_ecos_mutex_api.h"

#include "mvr.h"

#ifdef VTSS_SW_OPTION_VCLI
#include "mvr_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "mvr_icfg.h"
#endif /* VTSS_SW_OPTION_ICFG */
#include "misc_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_MVR

#define MVR_VLAN_TYPE               VLAN_USER_MVR   /* VLAN_USER_STATIC */
#define MVR_THREAD_EXE_SUPP         0

#define MVR_THREAD_SECOND_DEF       1000        /* 1000 msec = 1 second */
#define MVR_THREAD_TICK_TIME        1000        /* 1000 msec */
#define MVR_THREAD_TIME_UNIT_BASE   (MVR_THREAD_SECOND_DEF / MVR_THREAD_TICK_TIME)
#define MVR_THREAD_TIME_MISC_BASE   (MVR_THREAD_TIME_UNIT_BASE * 5)
#define MVR_THREAD_START_DELAY_CNT  15
#define MVR_THREAD_MIN_DELAY_CNT    3

#define MVR_EVENT_ANY               0xFFFFFFFF  /* Any possible bit */
#define MVR_EVENT_SM_TIME_WAKEUP    0x00000001
#define MVR_EVENT_SWITCH_ADD        0x00000010
#define MVR_EVENT_SWITCH_DEL        0x00000100
#define MVR_EVENT_CONFBLK_COMMIT    0x00001000
#define MVR_EVENT_DB_TIME_WAKEUP    0x00010000
#define MVR_EVENT_PKT_HANDLER       0x00100000
#define MVR_EVENT_VLAN_PORT_CHG     0x01000000
#define MVR_EVENT_MASTER_UP         0x10000000
#define MVR_EVENT_MASTER_DOWN       0x20000000

/* MVR Event Values */
#define MVR_EVENT_VALUE_INIT        0x0
#define MVR_EVENT_VALUE_SW_ADD      0x00000001
#define MVR_EVENT_VALUE_SW_DEL      0x00000010

/* Thread variables */
/* MVR Timer Thread */
static cyg_handle_t                 mvr_sm_thread_handle;
static cyg_thread                   mvr_sm_thread_block;
static char                         mvr_sm_thread_stack[IPMC_THREAD_STACK_SIZE];

static struct {
    critd_t                         crit;
    critd_t                         get_crit;

    mvr_configuration_t             mvr_conf;

    cyg_flag_t                      vlan_entry_flags;
    ipmc_prot_intf_entry_param_t    interface_entry;
    ipmc_prot_intf_group_entry_t    intf_group_entry;
    ipmc_prot_group_srclist_t       group_srclist_entry;

    /* Request semaphore */
    struct {
        vtss_os_sem_t   sem;
    } request;

    /* Reply semaphore */
    struct {
        vtss_os_sem_t   sem;
    } reply;

    /* MSG Buffer */
    u8                  *msg[MVR_MSG_MAX_ID];
    u32                 msize[MVR_MSG_MAX_ID];
} mvr_running;

#if VTSS_TRACE_ENABLED

static vtss_trace_reg_t mvr_trace_reg = {
    .module_id = VTSS_MODULE_ID_MVR,
    .name      = "MVR",
    .descr     = "Multicast VLAN"
};

static vtss_trace_grp_t mvr_trace_grps[TRACE_GRP_CNT] = {
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

#endif /* VTSS_TRACE_ENABLED */

#if VTSS_TRACE_ENABLED

#define MVR_CRIT_ENTER() critd_enter(&mvr_running.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define MVR_CRIT_EXIT()  critd_exit(&mvr_running.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)

#define MVR_GET_CRIT_ENTER() critd_enter(&mvr_running.get_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define MVR_GET_CRIT_EXIT()  critd_exit(&mvr_running.get_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)

#else

#define MVR_CRIT_ENTER() critd_enter(&mvr_running.crit)
#define MVR_CRIT_EXIT()  critd_exit(&mvr_running.crit)

#define MVR_GET_CRIT_ENTER() critd_enter(&mvr_running.get_crit)
#define MVR_GET_CRIT_EXIT()  critd_exit(&mvr_running.get_crit)

#endif /* VTSS_TRACE_ENABLED */

#ifdef VTSS_SW_OPTION_PACKET
static vtss_packet_rx_info_t    ipmc_mvr_rx_info;
#endif /* VTSS_SW_OPTION_PACKET */
static u8                       ipmcmvr_frame[IPMC_LIB_PKT_BUF_SZ];
static ipmc_lib_log_t           ipmcmvr_rx_packet_log;
static i8                       ipmcmvr_rx_pktlog_msg[IPMC_LIB_PKT_LOG_MSG_SZ_VAL];
static BOOL ipmcmvr_rx_packet_callback(void *contxt, const uchar *const frm, const vtss_packet_rx_info_t *const rx_info, BOOL next_ipmc)
{
    vtss_rc             rc = VTSS_OK;
    ipmc_ip_eth_hdr     *etherHdr;
    ipmc_ip_version_t   version = IPMC_IP_VERSION_INIT;
    ipmc_port_bfs_t     ipmcmvr_fwd_map;
    u16                 ingress_vid;
    BOOL                ingress_chk, ingress_member[VTSS_PORT_ARRAY_SIZE];
    vtss_mvr_intf_tx_t  pcp[MVR_NUM_OF_INTF_PER_VERSION];
    vtss_glag_no_t      glag_no;
    u32                 i, local_port_cnt;

    T_D("%s Frame with length %d received on port %d with vid %d",
        ipmc_lib_frm_tagtype_txt(rx_info->tag_type, IPMC_TXT_CASE_CAPITAL),
        rx_info->length, rx_info->port_no, rx_info->tag.vid);
    T_D_HEX(frm, rx_info->length);

    glag_no = rx_info->glag_no;
    T_D("With glag_no=%u in MVR", glag_no);

    memset(&ipmcmvr_fwd_map, 0x0, sizeof(ipmc_port_bfs_t));
    memset(ingress_member, 0x0, sizeof(ingress_member));
    etherHdr = (ipmc_ip_eth_hdr *) frm;
    if (ntohs(etherHdr->type) == IP_MULTICAST_V6_ETHER_TYPE) {
        version = IPMC_IP_VERSION_MLD;
    } else {
        if (ntohs(etherHdr->type) == IP_MULTICAST_V4_ETHER_TYPE) {
            version = IPMC_IP_VERSION_IGMP;
        }
    }

    ingress_chk = TRUE;
    memset(pcp, 0x0, sizeof(pcp));
    /* (VLAN) Ingress Filtering */
    ingress_vid = rx_info->tag.vid;
    if ((vtss_vlan_port_members_get(NULL, ingress_vid, &ingress_member[0]) != VTSS_OK) ||
        !ingress_member[rx_info->port_no]) {
#if VTSS_SWITCH_STACKABLE
        if (!port_isid_port_no_is_stack(VTSS_ISID_LOCAL, rx_info->port_no)) {
            rc = IPMC_ERROR_PKT_INGRESS_FILTER;
            ingress_chk = FALSE;
        }
#else
        rc = IPMC_ERROR_PKT_INGRESS_FILTER;
        ingress_chk = FALSE;
#endif /* VTSS_SWITCH_STACKABLE */
    }

#if VTSS_SWITCH_STACKABLE
    /* Stacking Ports are default members */
    i = PORT_NO_STACK_0;
    ingress_member[i] = TRUE;
    i = PORT_NO_STACK_1;
    ingress_member[i] = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */

    local_port_cnt = ipmc_lib_get_system_local_port_cnt();

#ifdef VTSS_SW_OPTION_PACKET
    if (ingress_chk) {
        MVR_CRIT_ENTER();
        memcpy(&ipmc_mvr_rx_info, rx_info, sizeof(ipmc_mvr_rx_info));
        ipmc_lib_packet_strip_vtag(frm, &ipmcmvr_frame[0], rx_info->tag_type, &ipmc_mvr_rx_info);

        rc = RX_ipmcmvr(version, ingress_vid, contxt, &ipmcmvr_frame[0], &ipmc_mvr_rx_info, &ipmcmvr_fwd_map, pcp);
        MVR_CRIT_EXIT();
    }
#endif /* VTSS_SW_OPTION_PACKET */

    switch ( rc ) {
    case IPMC_ERROR_PKT_INGRESS_FILTER:
    case IPMC_ERROR_PKT_CHECKSUM:
    case IPMC_ERROR_PKT_FORMAT:
        /* silently ignored */
        return FALSE;
    case IPMC_ERROR_PKT_CONTENT:
    case IPMC_ERROR_PKT_ADDRESS:
    case IPMC_ERROR_PKT_VERSION:
    case IPMC_ERROR_PKT_RESERVED:
        /* we shall flood this one */
        MVR_CRIT_ENTER();
        vtss_mvr_calculate_dst_ports(FALSE, FALSE, ingress_vid, rx_info->port_no, &ipmcmvr_fwd_map, version);

        memset(&ipmcmvr_rx_packet_log, 0x0, sizeof(ipmc_lib_log_t));
        ipmcmvr_rx_packet_log.vid = ingress_vid;
        ipmcmvr_rx_packet_log.port = rx_info->port_no + 1;
        ipmcmvr_rx_packet_log.version = version;
        ipmcmvr_rx_packet_log.event.message.data = ipmcmvr_rx_pktlog_msg;

        memset(ipmcmvr_rx_pktlog_msg, 0x0, sizeof(ipmcmvr_rx_pktlog_msg));
        if (rc == IPMC_ERROR_PKT_CONTENT) {
            sprintf(ipmcmvr_rx_pktlog_msg, "(IPMC_ERROR_PKT_CONTENT)->RX frame with bad content!");
        }
        if (rc == IPMC_ERROR_PKT_RESERVED) {
            sprintf(ipmcmvr_rx_pktlog_msg, "(IPMC_ERROR_PKT_RESERVED)->RX reserved group address registration!");
        }
        if (rc == IPMC_ERROR_PKT_ADDRESS) {
            sprintf(ipmcmvr_rx_pktlog_msg, "(IPMC_ERROR_PKT_ADDRESS)->RX frame with bad address!");
        }
        if (rc == IPMC_ERROR_PKT_VERSION) {
            sprintf(ipmcmvr_rx_pktlog_msg, "(IPMC_ERROR_PKT_ADDRESS)->RX frame with bad version!");
        }
        IPMC_LIB_LOG_MSG(&ipmcmvr_rx_packet_log, IPMC_SEVERITY_Warning);
        MVR_CRIT_EXIT();

        break;
    default:
        for (i = 0; i < local_port_cnt; i++) {
            if (i != rx_info->port_no) {
                if ((rc == VTSS_OK) ||
                    (rc == IPMC_ERROR_PKT_IS_QUERY) ||
                    (rc == IPMC_ERROR_PKT_GROUP_FILTER) ||
                    (rc == IPMC_ERROR_PKT_GROUP_NOT_FOUND)) {
                    VTSS_PORT_BF_SET(ipmcmvr_fwd_map.member_ports,
                                     i,
                                     ingress_member[i] & VTSS_PORT_BF_GET(ipmcmvr_fwd_map.member_ports, i));
                } else {
                    VTSS_PORT_BF_SET(ipmcmvr_fwd_map.member_ports,
                                     i,
                                     ingress_member[i]);
                }
            } else {
                VTSS_PORT_BF_SET(ipmcmvr_fwd_map.member_ports,
                                 i,
                                 FALSE);
            }
        }

#ifdef VTSS_FEATURE_AGGR_GLAG
        if (glag_no != VTSS_GLAG_NO_NONE) {
            // filter out all ports with the same glag
            memset(ingress_member, 0x0, sizeof(ingress_member));
            if (vtss_aggr_glag_members_get(NULL, glag_no, ingress_member) == VTSS_OK) {
                for (i = 0; i < local_port_cnt; i++) {
                    if (ingress_member[i] == TRUE) {
                        VTSS_PORT_BF_SET(ipmcmvr_fwd_map.member_ports, i, FALSE);
                        if (port_isid_port_no_is_stack(VTSS_ISID_LOCAL, rx_info->port_no)) {
                            MVR_CRIT_ENTER();
                            vtss_mvr_process_glag(i, ingress_vid, frm, rx_info->length, IPMC_IP_VERSION_IGMP);
                            vtss_mvr_process_glag(i, ingress_vid, frm, rx_info->length, IPMC_IP_VERSION_MLD);
                            MVR_CRIT_EXIT();
                        }
                    }
                }
            }
        }
#endif /* VTSS_FEATURE_AGGR_GLAG */

        break;
    }

#if VTSS_SWITCH_STACKABLE
    if (port_isid_port_no_is_stack(VTSS_ISID_LOCAL, rx_info->port_no)) {
        /* filter out Stacking Ports */
        i = PORT_NO_STACK_0;
        VTSS_PORT_BF_SET(ipmcmvr_fwd_map.member_ports, i, FALSE);
        i = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(ipmcmvr_fwd_map.member_ports, i, FALSE);
    }
#endif /* VTSS_SWITCH_STACKABLE */

    if (IPMC_LIB_BFS_HAS_MEMBER(ipmcmvr_fwd_map.member_ports)) {
        BOOL    src_fast_leave;

        MVR_CRIT_ENTER();
        src_fast_leave = vtss_mvr_get_fast_leave_ports(rx_info->port_no);
        MVR_CRIT_EXIT();

        if (ingress_chk &&
            ((rc == VTSS_OK) ||
             (!next_ipmc &&
              ((rc == IPMC_ERROR_PKT_IS_QUERY) ||
               (rc == IPMC_ERROR_PKT_GROUP_FILTER) ||
               (rc == IPMC_ERROR_PKT_GROUP_NOT_FOUND))))) {
            for (i = 0; i < MVR_NUM_OF_INTF_PER_VERSION; i++) {
                if (!pcp[i].vid) {
                    break;
                }

                if (ipmc_lib_packet_tx(&ipmcmvr_fwd_map,
                                       (pcp[i].vtag == IPMC_INTF_UNTAG) ? TRUE : FALSE,
                                       src_fast_leave,
                                       rx_info->port_no,
                                       pcp[i].src_type,
                                       pcp[i].vid,
                                       rx_info->tag.dei,
                                       pcp[i].priority,
                                       glag_no,
                                       frm,
                                       rx_info->length) != VTSS_OK) {
                    T_W("Failure in ipmc_lib_packet_tx");
                }
            }
        } else {
            if (!next_ipmc) {
                if (ipmc_lib_packet_tx(&ipmcmvr_fwd_map,
                                       FALSE,
                                       src_fast_leave,
                                       rx_info->port_no,
                                       IPMC_PKT_SRC_MVR,
                                       rx_info->tag.vid,
                                       rx_info->tag.dei,
                                       rx_info->tag.pcp,
                                       glag_no,
                                       frm,
                                       rx_info->length) != VTSS_OK) {
                    T_W("Failure in ipmc_lib_packet_tx");
                }
            }
        }
    }

    if (rc != VTSS_OK) {
        if (rc == IPMC_ERROR_PKT_IS_QUERY) {
            return FALSE;
        } else {
            if ((rc == IPMC_ERROR_PKT_GROUP_FILTER) ||
                (rc == IPMC_ERROR_PKT_GROUP_NOT_FOUND)) {
                if (next_ipmc) {
                    return FALSE;
                } else {
                    return TRUE;
                }
            } else {
                return FALSE;
            }
        }
    } else {
        /* MVR has done the processing; Others shouldn't take care of it. */
        return TRUE;
    }
}

#if VTSS_SWITCH_STACKABLE
static BOOL mvr_in_stacking_ready = FALSE;
#endif /* VTSS_SWITCH_STACKABLE */

/*lint -esym(459, mvr_sm_events) */
static cyg_flag_t   mvr_sm_events;

static cyg_alarm    mvr_sm_alarm;
static cyg_handle_t mvr_sm_alarm_handle;

static u32          mvr_switch_event_value[VTSS_ISID_END];

static void mvr_sm_event_set(cyg_flag_value_t flag)
{
    cyg_flag_setbits(&mvr_sm_events, flag);
}

static void mvr_sm_timer_isr(cyg_handle_t alarm, cyg_addrword_t data)
{
    if (alarm || data) { /* avoid warning */
    }

    mvr_sm_event_set(MVR_EVENT_SM_TIME_WAKEUP);
}

static void mvr_conf_reset_intf_role(vtss_isid_t isid, vtss_vid_t vid, mvr_conf_intf_role_t *role, BOOL clear)
{
    port_iter_t             pit;
    u32                     i;
    mvr_conf_port_role_t    *port_role;

    if (!role) {
        return;
    }

    for (i = 0; i < MVR_VLAN_MAX; i++) {
        port_role = &role->intf[i];
        if ((vid != MVR_VID_VOID) && (port_role->vid != vid)) {
            continue;
        }

        if (clear) {
            memset(port_role, 0x0, sizeof(mvr_conf_port_role_t));
        }

        if (msg_switch_is_master()) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        } else {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        }
        while (port_iter_getnext(&pit)) {
            port_role->ports[pit.iport] = MVR_CONF_DEF_PORT_ROLE;
        }
    }
}

static void mvr_conf_reset_intf(mvr_conf_intf_entry_t *intf, BOOL clear)
{
    if (!intf) {
        return;
    }

    if (clear) {
        memset(intf, 0x0, sizeof(mvr_conf_intf_entry_t));
    }

    intf->mode = MVR_CONF_DEF_INTF_MODE;
    intf->vtag = MVR_CONF_DEF_INTF_VTAG;
    intf->querier4_address = MVR_CONF_DEF_INTF_ADRS4;
    intf->priority = MVR_CONF_DEF_INTF_PRIO;
    intf->profile_index = MVR_CONF_DEF_INTF_PROFILE;
    intf->protocol_status = VTSS_IPMCMVR_ENABLED;
    intf->querier_status = VTSS_IPMCMVR_DISABLED;
    intf->compatibility = IPMC_PARAM_DEF_COMPAT;
    intf->robustness_variable = IPMC_PARAM_DEF_RV;
    intf->query_interval = IPMC_PARAM_DEF_QI;
    intf->query_response_interval = IPMC_PARAM_DEF_QRI;
    intf->last_listener_query_interval = MVR_CONF_DEF_INTF_LLQI;
    intf->unsolicited_report_interval = IPMC_PARAM_DEF_URI;
}

static void mvr_conf_default(mvr_configuration_t *conf)
{
    int                     i;
    switch_iter_t           sit;
    vtss_isid_t             j;
    mvr_conf_global_t       *global;
    mvr_conf_intf_entry_t   *intf;
    mvr_conf_intf_role_t    *intf_role;

    if (!conf) {
        return;
    }

    /* Use default configuration */
    memset(conf, 0x0, sizeof(mvr_configuration_t));

    global = &conf->mvr_conf_global;
    global->mvr_state = MVR_CONF_DEF_GLOBAL_MODE;

    for (i = 0; i < MVR_VLAN_MAX; i++) {
        intf = &conf->mvr_conf_vlan[i];
        mvr_conf_reset_intf(intf, FALSE);
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        j = sit.isid;

        intf_role = &conf->mvr_conf_role[ipmc_lib_isid_convert(TRUE, j)];
        mvr_conf_reset_intf_role(j, MVR_VID_VOID, intf_role, FALSE);
    }
    intf_role = &conf->mvr_conf_role[VTSS_ISID_LOCAL];
    mvr_conf_reset_intf_role(VTSS_ISID_LOCAL, MVR_VID_VOID, intf_role, FALSE);
}

typedef struct {
    ushort                      vid;
    BOOL                        mvr_status;
    BOOL                        querier_status;
    BOOL                        fast_leave_status;
} obs1_mvr_conf_intf_entry_t;

typedef struct {
    BOOL                        mvr_state;
    ushort                      mvid;
    BOOL                        mvr_unreg_flood_enabled;
    BOOL                        mvr_leave_proxy_enabled;
    BOOL                        mvr_proxy_enabled;
    BOOL                        router_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];             /* Dest. ports */
    BOOL                        fast_leave_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];             /* Dest. ports */
    BOOL                        mvr_port_mode[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];             /* Dest. ports */
    int                         mvr_port_type[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];             /* Dest. ports */
    int                         mvr_throttling_max_no[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];       /* "0" means disabled */
    obs1_mvr_conf_intf_entry_t  mvr_conf_vlan;
} mvr_conf_v1_t;

#define MVR_OBS2_VLAN_MAX       8
#define MVR_OBS2_GROUP_MAX      (16 * MVR_OBS2_VLAN_MAX)
#define MVR_OBS2_NAME_LEN       33  /* 32 + 1 */

typedef struct {
    u16                         vid;
    vtss_ipv6_t                 addr;

    i8                          name[MVR_OBS2_NAME_LEN];

    BOOL                        cnt_big;
    u32                         grp_cnt;
    vtss_ipv6_t                 grp_bnd;
} ipmc_channel_t;

typedef struct {
    BOOL                        valid;

    ipmc_channel_t              mvr_channel;
} mvr_conf_channel_t;

typedef struct {
    mvr_conf_port_role_t        intf[MVR_OBS2_VLAN_MAX];
} obs2_mvr_conf_intf_role_t;

typedef struct {
    BOOL                        valid;

    u16                         vid;
    i8                          name[MVR_OBS2_NAME_LEN];

    mvr_intf_mode_t             mode;
    ipmc_intf_vtag_t            vtag;
    u8                          priority;

    BOOL                        protocol_status;
    BOOL                        querier_status;

    u32                         compatibility;
    u32                         robustness_variable;
    u32                         query_interval;
    u32                         query_response_interval;
    u32                         last_listener_query_interval;
    u32                         unsolicited_report_interval;
} obs2_mvr_conf_intf_entry_t;

typedef struct {
    mvr_conf_global_t           mvr_conf_global;
    obs2_mvr_conf_intf_entry_t  mvr_conf_vlan[MVR_OBS2_VLAN_MAX];

    obs2_mvr_conf_intf_role_t   mvr_conf_role[VTSS_ISID_END];
    mvr_conf_channel_t          mvr_conf_channel[MVR_OBS2_GROUP_MAX];
    mvr_conf_fast_leave_t       mvr_conf_fast_leave[VTSS_ISID_END];
} mvr_conf_v2_t;

#define MVR_OBS34_NAME_LEN      17  /* 16 + 1 */

typedef struct {
    BOOL                        valid;

    u16                         vid;
    i8                          name[MVR_OBS34_NAME_LEN];

    mvr_intf_mode_t             mode;
    ipmc_intf_vtag_t            vtag;
    u8                          priority;
    u32                         profile_index;

    BOOL                        protocol_status;
    BOOL                        querier_status;

    u32                         compatibility;
    u32                         robustness_variable;
    u32                         query_interval;
    u32                         query_response_interval;
    u32                         last_listener_query_interval;
    u32                         unsolicited_report_interval;
} obs3_mvr_conf_intf_entry_t;

typedef struct {
    mvr_conf_global_t           mvr_conf_global;
    obs3_mvr_conf_intf_entry_t  mvr_conf_vlan[MVR_VLAN_MAX];

    mvr_conf_intf_role_t        mvr_conf_role[VTSS_ISID_END];
    mvr_conf_fast_leave_t       mvr_conf_fast_leave[VTSS_ISID_END];
} mvr_conf_v3_t;

typedef struct {
    BOOL                        valid;

    vtss_vid_t                  vid;
    mvr_port_role_t             ports[VTSS_PORT_ARRAY_SIZE];
} obs4_mvr_conf_port_role_t;

typedef struct {
    obs4_mvr_conf_port_role_t   intf[MVR_VLAN_MAX];
} obs4_mvr_conf_intf_role_t;

typedef struct {
    BOOL                        ports[VTSS_PORT_BF_SIZE];
} obs4_mvr_conf_fast_leave_t;

typedef struct {
    BOOL                        valid;

    BOOL                        protocol_status;
    BOOL                        querier_status;
    u8                          priority;

    vtss_ipv4_t                 querier4_address;
    mvr_intf_mode_t             mode;
    ipmc_intf_vtag_t            vtag;
    u32                         profile_index;

    u32                         compatibility;
    u32                         robustness_variable;
    u32                         query_interval;
    u32                         query_response_interval;
    u32                         last_listener_query_interval;
    u32                         unsolicited_report_interval;

    vtss_vid_t                  vid;
    i8                          name[MVR_OBS34_NAME_LEN];
    u8                          reserved[5];
} obs4_mvr_conf_intf_entry_t;

typedef struct {
    BOOL                        mvr_state;
} obs4_mvr_conf_global_t;

typedef struct {
    obs4_mvr_conf_global_t      mvr_conf_global;
    obs4_mvr_conf_intf_entry_t  mvr_conf_vlan[MVR_VLAN_MAX];

    obs4_mvr_conf_intf_role_t   mvr_conf_role[VTSS_ISID_END];
    obs4_mvr_conf_fast_leave_t  mvr_conf_fast_leave[VTSS_ISID_END];
} mvr_conf_v4_t;

static mvr_conf_v2_t            obs_mvr_conf_v2;
static mvr_conf_v3_t            obs_mvr_conf_v3;
static mvr_conf_v4_t            obs_mvr_conf_v4;

static BOOL _mvr_conf_copy(mvr_configuration_t *cfg_src, mvr_configuration_t *cfg_dst)
{
    if (cfg_src && cfg_dst) {
        memcpy(cfg_dst, cfg_src, sizeof(mvr_configuration_t));
    } else {
        T_W("Invalid Configuration Block");
        return FALSE;
    }

    return TRUE;
}

static BOOL mvr_conf_transition(u32 blk_ver,
                                u32 conf_sz,
                                void *blk_conf,
                                BOOL *new_blk,
                                mvr_configuration_t *tar_conf)
{
    BOOL                conf_reset, create_blk;
    mvr_conf_v1_t       *conf_blk1;
    mvr_conf_v2_t       *conf_blk2;
    mvr_conf_v3_t       *conf_blk3;
    mvr_conf_v4_t       *conf_blk4;
    mvr_configuration_t *conf_blk;

    if (!blk_conf || !new_blk || !tar_conf) {
        return FALSE;
    }

    create_blk = conf_reset = FALSE;
    conf_blk = NULL;
    switch ( blk_ver ) {
    case 1:
        conf_blk1 = (mvr_conf_v1_t *)blk_conf;

        if (conf_blk1) {
//            memcpy(&obs_mvr_conf_v1, conf_blk1, sizeof(mvr_conf_v1_t));
        }

        break;
    case 2:
        conf_blk2 = (mvr_conf_v2_t *)blk_conf;

        if (conf_blk2) {
            memcpy(&obs_mvr_conf_v2, conf_blk2, sizeof(mvr_conf_v2_t));
        }

        break;
    case 3:
        conf_blk3 = (mvr_conf_v3_t *)blk_conf;

        if (conf_blk3) {
            memcpy(&obs_mvr_conf_v3, conf_blk3, sizeof(mvr_conf_v3_t));
        }

        break;
    case 4:
        conf_blk4 = (mvr_conf_v4_t *)blk_conf;

        if (conf_blk4) {
            memcpy(&obs_mvr_conf_v4, conf_blk4, sizeof(mvr_conf_v4_t));
        }

        break;
    case 5:
    default:
        conf_blk = (mvr_configuration_t *)blk_conf;

        break;
    }

    if (blk_ver > MVR_CONF_VERSION) {
        /* Down-grade is not expected, just reset the current configuration */
        create_blk = TRUE;
        conf_reset = TRUE;
    } else if (blk_ver < MVR_CONF_VERSION) {
        obs2_mvr_conf_intf_entry_t  *obs2_intf;
        obs2_mvr_conf_intf_role_t   *obs2_role;
        obs3_mvr_conf_intf_entry_t  *obs3_intf;
        obs4_mvr_conf_intf_entry_t  *obs4_intf;
        mvr_conf_intf_entry_t       *intf;
        mvr_conf_port_role_t        *role;
        u16                         i, j, intf_idx, intf_cnt;

        /* Up-grade is allowed, do seamless transition */
        create_blk = TRUE;

        switch ( blk_ver ) {
        case 1:
            conf_reset = TRUE;

            break;
        case 2:
            mvr_conf_default(tar_conf);

            memcpy(&tar_conf->mvr_conf_global, &obs_mvr_conf_v2.mvr_conf_global, sizeof(mvr_conf_global_t));
            memcpy(tar_conf->mvr_conf_fast_leave, obs_mvr_conf_v2.mvr_conf_fast_leave, sizeof(tar_conf->mvr_conf_fast_leave));

            intf_cnt = 0;
            for (intf_idx = 0; intf_idx < MVR_OBS2_VLAN_MAX; intf_idx++) {
                obs2_intf = &obs_mvr_conf_v2.mvr_conf_vlan[intf_idx];
                if (!obs2_intf || !obs2_intf->valid || !obs2_intf->vid) {
                    continue;
                }

                if (!(intf_cnt < MVR_VLAN_MAX)) {
                    break;
                }

                intf = &tar_conf->mvr_conf_vlan[intf_cnt];
                intf->valid = TRUE;
                intf->vid = obs2_intf->vid;
                if (strlen(obs2_intf->name) < MVR_NAME_MAX_LEN) {
                    strncpy(intf->name, obs2_intf->name, strlen(obs2_intf->name));
                } else {
                    strncpy(intf->name, obs2_intf->name, MVR_NAME_MAX_LEN - 1);
                }

                intf->protocol_status = obs2_intf->protocol_status;;
                intf->querier_status = obs2_intf->querier_status;
                intf->mode = obs2_intf->mode;
                intf->vtag = obs2_intf->vtag;
                intf->priority = obs2_intf->priority;

                intf->compatibility = obs2_intf->compatibility;
                intf->robustness_variable = obs2_intf->robustness_variable;
                intf->query_interval = obs2_intf->query_interval;
                intf->query_response_interval = obs2_intf->query_response_interval;
                intf->last_listener_query_interval = obs2_intf->last_listener_query_interval;
                intf->unsolicited_report_interval = obs2_intf->unsolicited_report_interval;

                for (i = 0; i < VTSS_ISID_END; ++i) {
                    obs2_role = &obs_mvr_conf_v2.mvr_conf_role[i];
                    for (j = 0; j < MVR_OBS2_VLAN_MAX; ++j) {
                        role = &obs2_role->intf[j];
                        if (role->valid && (role->vid == intf->vid)) {
                            memcpy(&tar_conf->mvr_conf_role[i].intf[intf_cnt], role, sizeof(mvr_conf_port_role_t));
                            break;
                        }
                    }
                }

                ++intf_cnt;
            }

            break;
        case 3:
            memset(tar_conf, 0x0, sizeof(mvr_configuration_t));

            memcpy(&tar_conf->mvr_conf_global, &obs_mvr_conf_v3.mvr_conf_global, sizeof(mvr_conf_global_t));
            memcpy(tar_conf->mvr_conf_role, obs_mvr_conf_v3.mvr_conf_role, sizeof(tar_conf->mvr_conf_role));
            memcpy(tar_conf->mvr_conf_fast_leave, obs_mvr_conf_v3.mvr_conf_fast_leave, sizeof(tar_conf->mvr_conf_fast_leave));

            intf_cnt = 0;
            for (intf_idx = 0; intf_idx < MVR_VLAN_MAX; intf_idx++) {
                obs3_intf = &obs_mvr_conf_v3.mvr_conf_vlan[intf_idx];
                if (!obs3_intf || !obs3_intf->valid || !obs3_intf->vid) {
                    continue;
                }

                intf = &tar_conf->mvr_conf_vlan[intf_cnt];
                intf->valid = TRUE;
                intf->vid = obs3_intf->vid;
                if (strlen(obs3_intf->name) < MVR_NAME_MAX_LEN) {
                    strncpy(intf->name, obs3_intf->name, strlen(obs3_intf->name));
                } else {
                    strncpy(intf->name, obs3_intf->name, MVR_NAME_MAX_LEN - 1);
                }

                intf->protocol_status = obs3_intf->protocol_status;;
                intf->querier_status = obs3_intf->querier_status;
                intf->mode = obs3_intf->mode;
                intf->vtag = obs3_intf->vtag;
                intf->priority = obs3_intf->priority;
                intf->profile_index = obs3_intf->profile_index;

                intf->compatibility = obs3_intf->compatibility;
                intf->robustness_variable = obs3_intf->robustness_variable;
                intf->query_interval = obs3_intf->query_interval;
                intf->query_response_interval = obs3_intf->query_response_interval;
                intf->last_listener_query_interval = obs3_intf->last_listener_query_interval;
                intf->unsolicited_report_interval = obs3_intf->unsolicited_report_interval;

                ++intf_cnt;
            }

            break;
        case 4:
            /*
                NOTE THAT:
                Since Version-4, iCFG is already applied.
                However, to consider the CFG compatibility issue, we might still need to
                take care of the possible CFG changes.
                For example, customer requests to change the MVR VLAN name length in the
                middle of development cycle.
            */
            memset(tar_conf, 0x0, sizeof(mvr_configuration_t));

            memcpy(&tar_conf->mvr_conf_global, &obs_mvr_conf_v4.mvr_conf_global, sizeof(mvr_conf_global_t));
            memcpy(tar_conf->mvr_conf_role, obs_mvr_conf_v4.mvr_conf_role, sizeof(tar_conf->mvr_conf_role));
            memcpy(tar_conf->mvr_conf_fast_leave, obs_mvr_conf_v4.mvr_conf_fast_leave, sizeof(tar_conf->mvr_conf_fast_leave));

            intf_cnt = 0;
            for (intf_idx = 0; intf_idx < MVR_VLAN_MAX; intf_idx++) {
                obs4_intf = &obs_mvr_conf_v4.mvr_conf_vlan[intf_idx];
                if (!obs4_intf || !obs4_intf->valid || !obs4_intf->vid) {
                    continue;
                }

                intf = &tar_conf->mvr_conf_vlan[intf_cnt];
                intf->valid = TRUE;
                intf->vid = obs4_intf->vid;
                if (strlen(obs4_intf->name) < MVR_NAME_MAX_LEN) {
                    strncpy(intf->name, obs4_intf->name, strlen(obs4_intf->name));
                } else {
                    strncpy(intf->name, obs4_intf->name, MVR_NAME_MAX_LEN - 1);
                }

                intf->protocol_status = obs4_intf->protocol_status;;
                intf->querier_status = obs4_intf->querier_status;
                intf->mode = obs4_intf->mode;
                intf->vtag = obs4_intf->vtag;
                intf->priority = obs4_intf->priority;
                intf->profile_index = obs4_intf->profile_index;

                intf->compatibility = obs4_intf->compatibility;
                intf->robustness_variable = obs4_intf->robustness_variable;
                intf->query_interval = obs4_intf->query_interval;
                intf->query_response_interval = obs4_intf->query_response_interval;
                intf->last_listener_query_interval = obs4_intf->last_listener_query_interval;
                intf->unsolicited_report_interval = obs4_intf->unsolicited_report_interval;

                ++intf_cnt;
            }

            break;
        default:
            if (blk_ver == MVR_CONF_VERINIT) {
                create_blk = FALSE;
            }

            conf_reset = TRUE;

            break;
        }
    } else {
        if (sizeof(mvr_configuration_t) != conf_sz) {
            create_blk = TRUE;
            conf_reset = TRUE;
        } else {
            if (!_mvr_conf_copy(conf_blk, tar_conf)) {
                return FALSE;
            }
        }
    }

    *new_blk = create_blk;
    if (conf_reset) {
        T_W("Creating MVR default configurations");
        mvr_conf_default(tar_conf);
    }

    return TRUE;
}

static ipmc_error_t mvr_conf_read(BOOL create)
{
    mvr_conf_blk_t         *blk = NULL;
    u32                     size;
    mvr_configuration_t    *conf;
    BOOL                    do_create = FALSE, do_default = FALSE;
    cyg_tick_count_t        exe_time_base = cyg_current_time();

    if (misc_conf_read_use()) {
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MVR_CONF, &size)) == NULL) {
            T_W("conf_sec_open failed, creating MVR defaults");
            blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_MVR_CONF, sizeof(*blk));

            if (blk == NULL) {
                T_E("mvr_conf_read failed");
                return IPMC_ERROR_GEN;
            }

            blk->blk_version = MVR_CONF_VERINIT;
            do_default = TRUE;
        } else {
            do_default = create;
        }
    } else {
        blk        = NULL;
        do_default = 1;
        size       = 0;
    }

    MVR_CRIT_ENTER();
    if (!do_default  &&  blk != NULL) {
        BOOL    new_blk;
        u32     orig_conf_size = (size - sizeof(blk->blk_version));

        conf = &mvr_running.mvr_conf;
        new_blk = FALSE;
        if (mvr_conf_transition(blk->blk_version,
                                orig_conf_size,
                                (void *)&blk->mvr_conf,
                                &new_blk, conf)) {
            if (!do_create) {
                do_create = new_blk;
            }
        } else {
            MVR_CRIT_EXIT();
            T_W("mvr_conf_transition failed");
            return IPMC_ERROR_GEN;
        }

        conf = &mvr_running.mvr_conf;
    } else {
        conf = &mvr_running.mvr_conf;
        mvr_conf_default(conf);
    }
    MVR_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (do_create) {
        blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_MVR_CONF, sizeof(*blk));

        if (blk == NULL) {
            T_E("conf_sec_create failed");
            return IPMC_ERROR_GEN;
        }
    }

    if (blk) {  // Quiet lint
        MVR_CRIT_ENTER();
        blk->mvr_conf = *conf;
        MVR_CRIT_EXIT();

        blk->blk_version = MVR_CONF_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MVR_CONF);
    }
#else
    (void) conf;       // Quiet lint
    (void) do_create;
#endif

    T_D("mvr_conf_read consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

    return VTSS_RC_OK;
}

static void _mvr_mgmt_transit_intf(vtss_isid_t isid_in,
                                   vtss_isid_t isid,
                                   vtss_mvr_interface_t *intf,
                                   mvr_mgmt_interface_t *entry,
                                   ipmc_ip_version_t version)
{
    port_iter_t                     pit;
    u32                             idx;
    ipmc_intf_entry_t               *p;
    ipmc_prot_intf_entry_param_t    *param;
    mvr_conf_intf_entry_t           *iif;
    mvr_conf_intf_role_t            *pif;
    mvr_conf_port_role_t            *ptr;

    if (!intf || !entry) {
        return;
    }

    p = &intf->basic;
    param = &p->param;
    iif = &entry->intf;

    p->ipmc_version = version;
    param->vid = entry->vid;
    if (vtss_mvr_interface_get(intf) != VTSS_OK) {
        memset(intf, 0x0, sizeof(vtss_mvr_interface_t));
    }

    p->ipmc_version = version;
    param->vid = entry->vid;
    memcpy(intf->name, iif->name, MVR_NAME_MAX_LEN);
    intf->name[MVR_NAME_MAX_LEN - 1] = 0;
    intf->mode = iif->mode;
    intf->vtag = iif->vtag;
    intf->priority = iif->priority;
    intf->profile_index = iif->profile_index;
    param->querier.QuerierAdrs4 = iif->querier4_address;
    param->querier.RobustVari = MVR_QUERIER_ROBUST_VARIABLE;
    param->querier.QueryIntvl = MVR_QUERIER_QUERY_INTERVAL;
    param->querier.MaxResTime = MVR_QUERIER_MAX_RESP_TIME;
    param->querier.LastQryItv = iif->last_listener_query_interval;
    param->querier.LastQryCnt = param->querier.RobustVari;
    param->querier.UnsolicitR = MVR_QUERIER_UNSOLICIT_REPORT;
    param->cfg_compatibility = IPMC_PARAM_DEF_COMPAT;
    param->rtr_compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;
    param->hst_compatibility.mode = VTSS_IPMC_COMPAT_MODE_SFM;

    if (isid_in != VTSS_ISID_GLOBAL) {
        ptr = &entry->role;
        if (msg_switch_is_master()) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        } else {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        }
        while (port_iter_getnext(&pit)) {
            idx = pit.iport;

            intf->ports[idx] = ptr->ports[idx];

            if (intf->ports[idx] != MVR_PORT_ROLE_INACT) {
                VTSS_PORT_BF_SET(p->vlan_ports, idx, TRUE);
            }
        }
    } else {
        if (msg_switch_is_master()) {
            pif = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid)];
            for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
                ptr = &pif->intf[idx];

                if (ptr->valid && (ptr->vid == param->vid)) {
                    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                    while (port_iter_getnext(&pit)) {
                        idx = pit.iport;

                        intf->ports[idx] = ptr->ports[idx];

                        if (intf->ports[idx] != MVR_PORT_ROLE_INACT) {
                            VTSS_PORT_BF_SET(p->vlan_ports, idx, TRUE);
                        }
                    }

                    break;
                }
            }
        }
    }
}

static BOOL mvr_get_port_role_entry(vtss_isid_t isid_in, mvr_conf_port_role_t *entry)
{
    u16                     idx;
    switch_iter_t           sit;
    vtss_isid_t             isid;
    mvr_conf_intf_role_t    *p;
    mvr_conf_port_role_t    *q;

    if (!entry || (entry->vid > MVR_VID_MAX)) {
        return FALSE;
    }

    if (isid_in == VTSS_ISID_LOCAL) {
        p = &mvr_running.mvr_conf.mvr_conf_role[VTSS_ISID_LOCAL];
        for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
            q = &p->intf[idx];

            if (q->valid && (q->vid == entry->vid)) {
                memcpy(entry, q, sizeof(mvr_conf_port_role_t));
                return TRUE;
            }
        }

        return FALSE;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if (isid != isid_in) {
            continue;
        }

        p = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid)];
        for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
            q = &p->intf[idx];

            if (q->valid && (q->vid == entry->vid)) {
                memcpy(entry, q, sizeof(mvr_conf_port_role_t));
                return TRUE;
            }
        }
    }

    return FALSE;
}

static void _mvr_stacking_reset_port_type(vtss_isid_t isid)
{
    port_iter_t         pit;
    u32                 idx;
    vlan_port_conf_t    vlan_port_conf;

    if (!msg_switch_is_master()) {
        return;
    }

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        vtss_rc rc;

        idx = pit.iport;

        memset(&vlan_port_conf, 0x0, sizeof(vlan_port_conf_t));
        if ((rc = vlan_mgmt_port_conf_set(isid, idx, &vlan_port_conf, MVR_VLAN_TYPE)) != VTSS_RC_OK) {
            T_E("Failure in _mvr_stacking_reset_port_type on isid %d port %u. Error: %s", isid, idx, error_txt(rc));
        }
    }
}

static BOOL _mvr_stacking_set_port_type(vtss_isid_t isid, vtss_mvr_interface_t *entry)
{
    port_iter_t             pit;
    u32                     i, idx;
    u16                     uvid, chk_vid;
    BOOL                    flag;
    vlan_port_conf_t        vlan_port_conf;
    mvr_conf_intf_role_t    *p;
    mvr_conf_port_role_t    *q, chk_role;
    vtss_rc                 rc;

    if (!entry) {
        return FALSE;
    }

    if (!msg_switch_is_master() || (isid == VTSS_ISID_GLOBAL)) {
        return TRUE;
    }

    chk_vid = entry->basic.param.vid;
    memset(&chk_role, 0x0, sizeof(mvr_conf_port_role_t));
    chk_role.vid = chk_vid;
    MVR_CRIT_ENTER();
    if (!mvr_get_port_role_entry(isid, &chk_role)) {
        MVR_CRIT_EXIT();
        return FALSE;
    }
    MVR_CRIT_EXIT();

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        idx = pit.iport;

        memset(&vlan_port_conf, 0x0, sizeof(vlan_port_conf_t));

        if (chk_role.ports[idx] == MVR_PORT_ROLE_INACT) {
            u8 active_cnt = 0;

            uvid = 0;
            flag = FALSE;   /* flag to denote source port */

            MVR_CRIT_ENTER();
            p = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid)];
            for (i = 0; i < MVR_VLAN_MAX; i++) {
                q = &p->intf[i];

                if (q->valid && (q->vid != chk_vid)) {
                    if (q->ports[idx] != MVR_PORT_ROLE_INACT) {
                        ++active_cnt;
                        uvid = q->vid;

                        /* pre-defined: same role in a port */
                        if (q->ports[idx] == MVR_PORT_ROLE_SOURC) {
                            flag = TRUE;
                        }
                    }
                }
            }
            MVR_CRIT_EXIT();

            if (active_cnt) {
                if (vlan_mgmt_port_conf_get(isid, idx, &vlan_port_conf, MVR_VLAN_TYPE) == VTSS_OK) {
                    if (active_cnt == 1) {
                        vlan_port_conf.untagged_vid = uvid;
                        if (flag) {
                            vlan_port_conf.tx_tag_type = VLAN_TX_TAG_TYPE_TAG_THIS;
                        }
                    }
                }
            }

            T_D("Set isid-%d port-%u pvid=%u/uvid=%u/frm_t=%d/ing_fltr=%s/tx_tag_t=%d/port_t=%d/flag=%u",
                isid,
                idx,
                vlan_port_conf.pvid,
                vlan_port_conf.untagged_vid,
                vlan_port_conf.frame_type,
                vlan_port_conf.ingress_filter ? "TRUE" : "FALSE",
                vlan_port_conf.tx_tag_type,
                vlan_port_conf.port_type,
                vlan_port_conf.flags);
            if ((rc = vlan_mgmt_port_conf_set(isid, idx, &vlan_port_conf, MVR_VLAN_TYPE)) != VTSS_RC_OK) {
                T_E("Failure in MVR_PORT_ROLE_INACT on isid %d port %u (%s)", isid, idx, error_txt(rc));
            }

            continue;
        }

        if (vlan_mgmt_port_conf_get(isid, idx, &vlan_port_conf, MVR_VLAN_TYPE) != VTSS_OK) {
            continue;
        }

        uvid = entry->basic.param.vid;
        flag = FALSE;   /* For overlapped UVID checking */

        MVR_CRIT_ENTER();
        p = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid)];
        for (i = 0; i < MVR_VLAN_MAX; i++) {
            q = &p->intf[i];

            if (q->valid && (q->vid != chk_vid)) {
                if (q->ports[idx] == chk_role.ports[idx]) {
                    flag = TRUE;
                    break;
                }
            }
        }
        MVR_CRIT_EXIT();

        if (flag) {
            vlan_port_conf_t    vlan_port_chk;

            memset(&vlan_port_chk, 0x0, sizeof(vlan_port_conf_t));
            if (vlan_mgmt_port_conf_get(isid, idx, &vlan_port_chk, VLAN_USER_STATIC) == VTSS_OK) {
                uvid = vlan_port_chk.pvid;
            } else {
                uvid = VLAN_ID_DEFAULT;
            }
        }

        vlan_port_conf.flags |= (VLAN_PORT_FLAGS_AWARE | VLAN_PORT_FLAGS_RX_TAG_TYPE | VLAN_PORT_FLAGS_TX_TAG_TYPE);
        vlan_port_conf.untagged_vid = uvid;
        vlan_port_conf.port_type = VLAN_PORT_TYPE_C;
        if (chk_role.ports[idx] == MVR_PORT_ROLE_SOURC) {
            /* Sourcer Ports */
            vlan_port_conf.frame_type = VTSS_VLAN_FRAME_ALL;
            if (flag) {
                vlan_port_conf.tx_tag_type = VLAN_TX_TAG_TYPE_UNTAG_THIS;
            } else {
                vlan_port_conf.tx_tag_type = VLAN_TX_TAG_TYPE_TAG_THIS;
            }
        } else {
            /* Receiver Ports */
            vlan_port_conf.frame_type = VTSS_VLAN_FRAME_ALL;
            vlan_port_conf.tx_tag_type = VLAN_TX_TAG_TYPE_UNTAG_ALL;
        }

        T_D("Set isid-%d port-%u pvid=%u/uvid=%u/frm_t=%d/ing_fltr=%s/tx_tag_t=%d/port_t=%d/flag=%u",
            isid,
            idx,
            vlan_port_conf.pvid,
            vlan_port_conf.untagged_vid,
            vlan_port_conf.frame_type,
            vlan_port_conf.ingress_filter ? "TRUE" : "FALSE",
            vlan_port_conf.tx_tag_type,
            vlan_port_conf.port_type,
            vlan_port_conf.flags);
        if ((rc = vlan_mgmt_port_conf_set(isid, idx, &vlan_port_conf, MVR_VLAN_TYPE)) != VTSS_RC_OK) {
            T_E("Failure(%s) on isid %d port %u (%s)", (chk_role.ports[idx] == MVR_PORT_ROLE_SOURC) ? "SRC" : "RCV", isid, idx, error_txt(rc));
        }
    }

    return TRUE;
}

static BOOL _mvr_stacking_refine_port_type(vtss_isid_t isid)
{
    port_iter_t             pit;
    u32                     i, idx;
    u16                     uvid, active_cnt;
    BOOL                    flag;
    vlan_port_conf_t        vlan_port_conf;
    mvr_conf_intf_role_t    *p;
    mvr_conf_port_role_t    *q;
    vtss_rc                 rc;

    if (!msg_switch_is_master() || (isid == VTSS_ISID_GLOBAL)) {
        return TRUE;
    }

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
    while (port_iter_getnext(&pit)) {
        idx = pit.iport;

        memset(&vlan_port_conf, 0x0, sizeof(vlan_port_conf_t));
        uvid = active_cnt = 0;
        flag = FALSE;   /* flag to denote source port */
        MVR_CRIT_ENTER();
        p = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid)];
        for (i = 0; i < MVR_VLAN_MAX; i++) {
            q = &p->intf[i];

            if (!q->valid) {
                continue;
            }

            if (q->ports[idx] != MVR_PORT_ROLE_INACT) {
                ++active_cnt;
                uvid = q->vid;

                /* pre-defined: same role in a port */
                if (q->ports[idx] == MVR_PORT_ROLE_SOURC) {
                    flag = TRUE;
                }
            }
        }
        MVR_CRIT_EXIT();

        if (active_cnt) {
            if (vlan_mgmt_port_conf_get(isid, idx, &vlan_port_conf, MVR_VLAN_TYPE) == VTSS_OK) {
                if (active_cnt == 1) {
                    vlan_port_conf.untagged_vid = uvid;
                    if (flag) {
                        vlan_port_conf.tx_tag_type = VLAN_TX_TAG_TYPE_TAG_THIS;
                    }
                }
            }
        }

        T_D("Refine isid-%d port-%u pvid=%u/uvid=%u/frm_t=%d/ing_fltr=%s/tx_tag_t=%d/port_t=%d/flag=%u",
            isid,
            idx,
            vlan_port_conf.pvid,
            vlan_port_conf.untagged_vid,
            vlan_port_conf.frame_type,
            vlan_port_conf.ingress_filter ? "TRUE" : "FALSE",
            vlan_port_conf.tx_tag_type,
            vlan_port_conf.port_type,
            vlan_port_conf.flags);
        if ((rc = vlan_mgmt_port_conf_set(isid, idx, &vlan_port_conf, MVR_VLAN_TYPE)) != VTSS_RC_OK) {
            T_E("Failure in setting isid %d port %u (%s)", isid, idx, error_txt(rc));
        }
    }

    return TRUE;
}

static char *mvr_msg_id_txt(mvr_msg_id_t msg_id)
{
    char    *txt;

    switch ( msg_id ) {
    case MVR_MSG_ID_GLOBAL_SET_REQ:
        txt = "MVR_MSG_ID_GLOBAL_SET_REQ";

        break;
    case MVR_MSG_ID_SYS_MGMT_SET_REQ:
        txt = "MVR_MSG_ID_SYS_MGMT_SET_REQ";

        break;
    case MVR_MSG_ID_GLOBAL_PURGE_REQ:
        txt = "MVR_MSG_ID_GLOBAL_PURGE_REQ";

        break;
    case MVR_MSG_ID_VLAN_ENTRY_SET_REQ:
        txt = "MVR_MSG_ID_VLAN_ENTRY_SET_REQ";

        break;
    case MVR_MSG_ID_VLAN_ENTRY_GET_REQ:
        txt = "MVR_MSG_ID_VLAN_ENTRY_GET_REQ";

        break;
    case MVR_MSG_ID_VLAN_ENTRY_GET_REP:
        txt = "MVR_MSG_ID_VLAN_ENTRY_GET_REP";

        break;
    case MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ:
        txt = "MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ";

        break;
    case MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP:
        txt = "MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP";

        break;
    case MVR_MSG_ID_GROUP_SRCLIST_WALK_REQ:
        txt = "MVR_MSG_ID_GROUP_SRCLIST_WALK_REQ";

        break;
    case MVR_MSG_ID_GROUP_SRCLIST_WALK_REP:
        txt = "MVR_MSG_ID_GROUP_SRCLIST_WALK_REP";

        break;
    case MVR_MSG_ID_PORT_FAST_LEAVE_SET_REQ:
        txt = "MVR_MSG_ID_PORT_FAST_LEAVE_SET_REQ";

        break;
    case MVR_MSG_ID_STAT_COUNTER_CLEAR_REQ:
        txt = "MVR_MSG_ID_STAT_COUNTER_CLEAR_REQ";

        break;
    case MVR_MSG_ID_STP_PORT_CHANGE_REQ:
        txt = "MVR_MSG_ID_STP_PORT_CHANGE_REQ";

        break;
    default:
        txt = "?";

        break;
    }

    return txt;
}

/* Allocate request/reply buffer */
static void mvr_msg_alloc(mvr_msg_id_t msg_id, mvr_msg_buf_t *buf, BOOL from_get, BOOL request)
{
    u32 msg_size;

    T_I("msg-%s (%d)", mvr_msg_id_txt(msg_id), msg_id);

    if (from_get) {
        MVR_GET_CRIT_ENTER();
    } else {
        MVR_CRIT_ENTER();
    }
    buf->sem = (request ? &mvr_running.request.sem : &mvr_running.reply.sem);
    buf->msg = mvr_running.msg[msg_id];
    msg_size = mvr_running.msize[msg_id];
    if (from_get) {
        MVR_GET_CRIT_EXIT();
    } else {
        MVR_CRIT_EXIT();
    }

    (void) VTSS_OS_SEM_WAIT(buf->sem);
    T_I("msg_id: %d->%s(%s-LOCK)", msg_id, mvr_msg_id_txt(msg_id), request ? "REQ" : "REP");
    memset(buf->msg, 0x0, msg_size);
}

static void mvr_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    mvr_msg_id_t    msg_id = *(mvr_msg_id_t *)msg;

    (void) VTSS_OS_SEM_POST(contxt);
    T_I("msg_id: %d->%s-UNLOCK", msg_id, mvr_msg_id_txt(msg_id));
}

static void mvr_msg_tx(mvr_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
    mvr_msg_id_t    msg_id = *(mvr_msg_id_t *)buf->msg;

    T_D("msg_id: %d->%s, len: %zd, isid: %d", msg_id, mvr_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(buf->sem, mvr_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_MVR, isid, buf->msg, len);
}

static vtss_mvr_interface_t mvr_intf4msg;
static BOOL mvr_msg_rx(void *contxt, const void *const rx_msg, size_t len, vtss_module_id_t modid, u32 isid)
{
    mvr_msg_id_t        msg_id = *(mvr_msg_id_t *)rx_msg;
    int                 i;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    T_D("msg_id: %d->%s, len: %zd, isid: %u", msg_id, mvr_msg_id_txt(msg_id), len, isid);

    switch ( msg_id ) {
    case MVR_MSG_ID_GLOBAL_SET_REQ: {
        mvr_msg_global_set_req_t    *msg;

        msg = (mvr_msg_global_set_req_t *)rx_msg;
        T_D("Receiving MVR_MSG_ID_GLOBAL_SET_REQ");

        if (msg->global_setting.mvr_state == VTSS_IPMCMVR_ENABLED) {
            BOOL                    next;
            vtss_mvr_interface_t    mvrif;

            memset(&mvrif, 0x0, sizeof(vtss_mvr_interface_t));
            MVR_CRIT_ENTER();
            next = (vtss_mvr_interface_get_next(&mvrif) == VTSS_OK);
            MVR_CRIT_EXIT();
            while (next) {
                if (_mvr_stacking_set_port_type(msg->isid, &mvrif) != TRUE) {
                    T_D("MVR set VLAN-%u tagging failed in isid-%d", mvrif.basic.param.vid, msg->isid);
                }

                MVR_CRIT_ENTER();
                next = (vtss_mvr_interface_get_next(&mvrif) == VTSS_OK);
                MVR_CRIT_EXIT();
            }

            MVR_CRIT_ENTER();
            mvr_running.mvr_conf.mvr_conf_global.mvr_state = TRUE;
            vtss_mvr_set_mode(TRUE);
            MVR_CRIT_EXIT();
        } else {
            _mvr_stacking_reset_port_type(msg->isid);

            MVR_CRIT_ENTER();
            mvr_running.mvr_conf.mvr_conf_global.mvr_state = FALSE;
            vtss_mvr_set_mode(FALSE);
            MVR_CRIT_EXIT();
        }
        mvr_sm_event_set(MVR_EVENT_PKT_HANDLER);

        break;
    }
    case MVR_MSG_ID_SYS_MGMT_SET_REQ: {
        mvr_msg_sys_mgmt_set_req_t  *msg;
        ipmc_lib_mgmt_info_t        *sys_mgmt;
        ipmc_mgmt_ipif_t            *ifp;
        u32                         ifx;

        if (msg_switch_is_master() ||
            !IPMC_MEM_SYSTEM_MTAKE(sys_mgmt, sizeof(ipmc_lib_mgmt_info_t))) {
            break;
        }

        msg = (mvr_msg_sys_mgmt_set_req_t *)rx_msg;
        T_I("Receiving MVR_MSG_ID_SYS_MGMT_SET_REQ:MAC[%s]", misc_mac2str(msg->mgmt_mac));

        memset(sys_mgmt, 0x0, sizeof(ipmc_lib_mgmt_info_t));
        IPMC_MGMT_SYSTEM_CHANGE(sys_mgmt) = TRUE;
        IPMC_MGMT_MAC_ADR_SET(sys_mgmt, msg->mgmt_mac);
        for (ifx = 0; ifx < msg->intf_cnt; ifx++) {
            ifp = &msg->ip_addr[ifx];
            if (ipmc_mgmt_intf_live(ifp)) {
                T_I("RCV(%u) INTF-ADR-%u(VID:%u)/%s",
                    (ifx + 1),
                    ipmc_mgmt_intf_adr4(ifp),
                    ipmc_mgmt_intf_vidx(ifp),
                    ipmc_mgmt_intf_opst(ifp) ? "Up" : "Down");

                IPMC_MGMT_IPIF_IDVLN(sys_mgmt, ifx) = ipmc_mgmt_intf_vidx(ifp);
                IPMC_MGMT_IPIF_VALID(sys_mgmt, ifx) = ipmc_mgmt_intf_live(ifp);
                IPMC_MGMT_IPIF_ADRS4(sys_mgmt, ifx) = ipmc_mgmt_intf_adr4(ifp);
                IPMC_MGMT_IPIF_STATE(sys_mgmt, ifx) = ipmc_mgmt_intf_opst(ifp);
            }
        }

        if (!ipmc_lib_system_mgmt_info_set(sys_mgmt)) {
            T_D("Failure in ipmc_lib_system_mgmt_info_set()");
        }
        IPMC_MEM_SYSTEM_MGIVE(sys_mgmt);

        break;
    }
    case MVR_MSG_ID_GLOBAL_PURGE_REQ: {
        mvr_msg_purge_req_t *msg;

        msg = (mvr_msg_purge_req_t *)rx_msg;
        T_D("Receiving MVR_MSG_ID_GLOBAL_PURGE_REQ");

        MVR_CRIT_ENTER();
        (void)vtss_mvr_purge(msg->isid, FALSE, FALSE);
        MVR_CRIT_EXIT();

        break;
    }
    case MVR_MSG_ID_VLAN_ENTRY_SET_REQ: {
        mvr_msg_vlan_set_req_t  *msg;
        vtss_mvr_interface_t    *mvrif;
        port_iter_t             pit;
        char                    mvrBufPort[MGMT_PORT_BUF_SIZE];
        BOOL                    mvrFwdMap[VTSS_PORT_ARRAY_SIZE];

        msg = (mvr_msg_vlan_set_req_t *)rx_msg;

        memset(mvrFwdMap, 0x0, sizeof(mvrFwdMap));
        if (msg_switch_is_master()) {
            (void) port_iter_init(&pit, NULL, msg->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        } else {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        }
        while (port_iter_getnext(&pit)) {
            if (msg->vlan_entry.role.ports[pit.iport] != MVR_PORT_ROLE_INACT) {
                mvrFwdMap[pit.iport] = TRUE;
            }
        }

        T_D("Receiving MVR_MSG_ID_VLAN_ENTRY_SET_REQ(%u) with Ports=%s",
            msg->vlan_entry.vid,
            mgmt_iport_list2txt(mvrFwdMap, mvrBufPort));

        MVR_CRIT_ENTER();
        mvrif = &mvr_intf4msg;
        _mvr_mgmt_transit_intf(VTSS_ISID_LOCAL, msg->isid, mvrif, &msg->vlan_entry, msg->version);
        if (vtss_mvr_interface_set(msg->action, mvrif) != VTSS_OK) {
            T_D("MVR_MSG_ID_VLAN_ENTRY_SET_REQ(%s-%u) Action:%d Failed!",
                ipmc_lib_version_txt(msg->version, IPMC_TXT_CASE_UPPER),
                mvrif->basic.param.vid,
                msg->action);
        }
        MVR_CRIT_EXIT();

        break;
    }
    case MVR_MSG_ID_VLAN_ENTRY_GET_REQ: {
        mvr_msg_vlan_entry_get_req_t    *msg;
        mvr_msg_vlan_entry_get_rep_t    *msg_rep;
        mvr_msg_buf_t                   buf;
        vtss_mvr_interface_t            *intf;
        ipmc_intf_entry_t               *entry;
        BOOL                            valid;

        msg = (mvr_msg_vlan_entry_get_req_t *)rx_msg;
        T_D("Receiving MVR_MSG_ID_VLAN_ENTRY_GET_REQ (VID:%d/VER:%d)", msg->vid, msg->version);

        MVR_CRIT_ENTER();
        intf = &mvr_intf4msg;
        memset(intf, 0x0, sizeof(vtss_mvr_interface_t));
        entry = &intf->basic;
        entry->ipmc_version =  msg->version;
        entry->param.vid = msg->vid;

        valid = vtss_mvr_interface_get(intf) == VTSS_OK;
        MVR_CRIT_EXIT();

        mvr_msg_alloc(MVR_MSG_ID_VLAN_ENTRY_GET_REP, &buf, FALSE, FALSE);
        msg_rep = (mvr_msg_vlan_entry_get_rep_t *)buf.msg;

        msg_rep->msg_id = MVR_MSG_ID_VLAN_ENTRY_GET_REP;
        if (valid) {
            memcpy(&msg_rep->interface_entry, &entry->param, sizeof(ipmc_prot_intf_entry_param_t));
        } else {
            msg_rep->interface_entry.vid = 0;
            T_D("no such vlan (%d)", msg->vid);
        }

        mvr_msg_tx(&buf, isid, sizeof(*msg_rep));

        break;
    }
    case MVR_MSG_ID_VLAN_ENTRY_GET_REP: {
        mvr_msg_vlan_entry_get_rep_t    *msg;

        msg = (mvr_msg_vlan_entry_get_rep_t *)rx_msg;
        T_D("Receiving MVR_MSG_ID_VLAN_ENTRY_GET_REP");

        MVR_CRIT_ENTER();
        memcpy(&mvr_running.interface_entry, &msg->interface_entry, sizeof(ipmc_prot_intf_entry_param_t));
        cyg_flag_setbits(&mvr_running.vlan_entry_flags, 1 << isid);
        MVR_CRIT_EXIT();

        break;
    }
    case MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ: {
        mvr_msg_vlan_entry_get_req_t        *msg;
        mvr_msg_vlan_group_entry_get_rep_t  *msg_rep;
        mvr_msg_buf_t                       buf;
        ipmc_group_entry_t                  grp;
        BOOL                                valid;

        msg = (mvr_msg_vlan_entry_get_req_t *)rx_msg;
        T_D("Receiving MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ");

        grp.ipmc_version = msg->version;
        grp.vid = msg->vid;
        memcpy(&grp.group_addr, &msg->group_addr, sizeof(vtss_ipv6_t));
        MVR_CRIT_ENTER();
        valid = vtss_mvr_intf_group_get_next(&grp);
        MVR_CRIT_EXIT();

        mvr_msg_alloc(MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP, &buf, FALSE, FALSE);
        msg_rep = (mvr_msg_vlan_group_entry_get_rep_t *)buf.msg;

        msg_rep->msg_id = MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP;
        msg_rep->intf_group_entry.valid = FALSE;
        if (valid) {
            msg_rep->intf_group_entry.ipmc_version = grp.ipmc_version;
            msg_rep->intf_group_entry.vid = grp.vid;
            memcpy(&msg_rep->intf_group_entry.group_addr, &grp.group_addr, sizeof(vtss_ipv6_t));
            memcpy(&msg_rep->intf_group_entry.db, &grp.info->db, sizeof(ipmc_group_db_t));

            if (!msg_switch_is_master()) {
                msg_rep->intf_group_entry.db.ipmc_sf_do_forward_srclist = NULL;
                msg_rep->intf_group_entry.db.ipmc_sf_do_not_forward_srclist = NULL;
                IPMC_FLTR_TIMER_DELTA_GET(&msg_rep->intf_group_entry.db);
            }

            msg_rep->intf_group_entry.valid = TRUE;
        } else {
            T_D("no more group entry in vlan-%d", msg->vid);
        }
        mvr_msg_tx(&buf, isid, sizeof(*msg_rep));

        break;
    }
    case MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP: {
        mvr_msg_vlan_group_entry_get_rep_t  *msg;

        msg = (mvr_msg_vlan_group_entry_get_rep_t *)rx_msg;
        T_D("Receiving MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP");

        MVR_CRIT_ENTER();
        memcpy(&mvr_running.intf_group_entry, &msg->intf_group_entry, sizeof(ipmc_prot_intf_group_entry_t));
        cyg_flag_setbits(&mvr_running.vlan_entry_flags, 1 << isid);
        MVR_CRIT_EXIT();

        break;
    }
    case MVR_MSG_ID_GROUP_SRCLIST_WALK_REQ: {
        mvr_msg_vlan_entry_get_req_t    *msg;
        mvr_msg_group_srclist_get_rep_t *msg_rep;
        mvr_msg_buf_t                   buf;
        ipmc_group_entry_t              grp;
        BOOL                            valid;

        msg = (mvr_msg_vlan_entry_get_req_t *)rx_msg;
        T_D("Receiving MVR_MSG_ID_GROUP_SRCLIST_WALK_REQ");

        grp.ipmc_version = msg->version;
        grp.vid = msg->vid;
        memcpy(&grp.group_addr, &msg->group_addr, sizeof(vtss_ipv6_t));
        MVR_CRIT_ENTER();
        valid = vtss_mvr_intf_group_get(&grp);
        MVR_CRIT_EXIT();

        mvr_msg_alloc(MVR_MSG_ID_GROUP_SRCLIST_WALK_REP, &buf, FALSE, FALSE);
        msg_rep = (mvr_msg_group_srclist_get_rep_t *)buf.msg;

        msg_rep->msg_id = MVR_MSG_ID_GROUP_SRCLIST_WALK_REP;
        msg_rep->group_srclist_entry.valid = FALSE;
        if (valid) {
            ipmc_db_ctrl_hdr_t  *srclist;

            if (msg->srclist_type) {
                srclist = grp.info->db.ipmc_sf_do_forward_srclist;
            } else {
                srclist = grp.info->db.ipmc_sf_do_not_forward_srclist;
            }

            if (srclist &&
                ((msg_rep->group_srclist_entry.cntr = IPMC_LIB_DB_GET_COUNT(srclist)) != 0)) {
                ipmc_sfm_srclist_t  entry;

                memcpy(&entry.src_ip, &msg->srclist_addr, sizeof(vtss_ipv6_t));
                if (ipmc_lib_srclist_buf_get_next(srclist, &entry)) {
                    msg_rep->group_srclist_entry.type = msg->srclist_type;
                    memcpy(&msg_rep->group_srclist_entry.srclist, &entry, sizeof(ipmc_sfm_srclist_t));
                    IPMC_SRCT_TIMER_DELTA_GET(&msg_rep->group_srclist_entry.srclist);

                    msg_rep->group_srclist_entry.valid = TRUE;
                }
            }
        }
        mvr_msg_tx(&buf, isid, sizeof(*msg_rep));

        break;
    }
    case MVR_MSG_ID_GROUP_SRCLIST_WALK_REP: {
        mvr_msg_group_srclist_get_rep_t *msg;

        msg = (mvr_msg_group_srclist_get_rep_t *)rx_msg;
        T_D("Receiving MVR_MSG_ID_GROUP_SRCLIST_WALK_REP");

        MVR_CRIT_ENTER();
        memcpy(&mvr_running.group_srclist_entry, &msg->group_srclist_entry, sizeof(ipmc_prot_group_srclist_t));
        cyg_flag_setbits(&mvr_running.vlan_entry_flags, 1 << isid);
        MVR_CRIT_EXIT();

        break;
    }
    case MVR_MSG_ID_PORT_FAST_LEAVE_SET_REQ: {
        mvr_msg_port_fast_leave_set_req_t   *msg;
        port_iter_t                         pit;
        ipmc_port_bfs_t                     fast_leave_mask;

        msg = (mvr_msg_port_fast_leave_set_req_t *)rx_msg;
        T_D("Receiving MVR_MSG_ID_PORT_FAST_LEAVE_SET_REQ");

        VTSS_PORT_BF_CLR(fast_leave_mask.member_ports);
        if (msg_switch_is_master()) {
            (void) port_iter_init(&pit, NULL, msg->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        } else {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        }
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            VTSS_PORT_BF_SET(fast_leave_mask.member_ports,
                             i,
                             VTSS_PORT_BF_GET(msg->fast_leave.ports, i));
        }

        MVR_CRIT_ENTER();
        vtss_mvr_set_fast_leave_ports(&fast_leave_mask);
        MVR_CRIT_EXIT();

        break;
    }
    case MVR_MSG_ID_STAT_COUNTER_CLEAR_REQ: {
        mvr_msg_stat_counter_clear_req_t    *msg;

        msg = (mvr_msg_stat_counter_clear_req_t *)rx_msg;
        T_D("Receiving MVR_MSG_ID_STAT_COUNTER_CLEAR_REQ");

        MVR_CRIT_ENTER();
        vtss_mvr_clear_stat_counter(msg->vid);
        MVR_CRIT_EXIT();

        break;
    }
    case MVR_MSG_ID_STP_PORT_CHANGE_REQ: {
        mvr_msg_stp_port_change_set_req_t   *msg;
        ipmc_port_bfs_t                     router_port_mask;

        msg = (mvr_msg_stp_port_change_set_req_t *) rx_msg;
        T_D("Receiving MVR_MSG_ID_STP_PORT_CHANGE_REQ");

        VTSS_PORT_BF_CLR(router_port_mask.member_ports);

        MVR_CRIT_ENTER();
        /* We should filter sendout process if this port belongs to dynamic router port. */
        ipmc_lib_get_discovered_router_port_mask(IPMC_IP_VERSION_ALL, &router_port_mask);
        if (!VTSS_PORT_BF_GET(router_port_mask.member_ports, msg->port)) {
            vtss_mvr_stp_port_state_change_handle(IPMC_IP_VERSION_ALL, msg->port, msg->new_state);
        }
        MVR_CRIT_EXIT();

        break;
    }
    default:
        T_E("unknown message ID: %d", msg_id);

        break;
    }

    T_D("mvr_msg_rx(%u) consumes ID%u:%u ticks", (u32)msg_id, isid, (u32)(cyg_current_time() - exe_time_base));

    return TRUE;
}

static vtss_rc mvr_stacking_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = mvr_msg_rx;
    filter.modid = VTSS_MODULE_ID_MVR;
    return msg_rx_filter_register(&filter);
}

/* Set STACK IPMC MODE */
static vtss_rc mvr_stacking_set_mode(vtss_isid_t isid_add)
{
    mvr_msg_buf_t               buf;
    mvr_msg_global_set_req_t    *msg;
    switch_iter_t               sit;
    vtss_isid_t                 isid;
    vtss_mvr_interface_t        mvrif;
    BOOL                        next, apply_mode;

    if (!msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    T_D("enter, isid: %d", isid_add);

    MVR_CRIT_ENTER();
    apply_mode = mvr_running.mvr_conf.mvr_conf_global.mvr_state;
    MVR_CRIT_EXIT();

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if ((isid_add != VTSS_ISID_GLOBAL) && (isid_add != isid)) {
            continue;
        }

        if (ipmc_lib_isid_is_local(isid)) {
            if (apply_mode == VTSS_IPMCMVR_ENABLED) {
                memset(&mvrif, 0x0, sizeof(vtss_mvr_interface_t));
                MVR_CRIT_ENTER();
                next = (vtss_mvr_interface_get_next(&mvrif) == VTSS_OK);
                MVR_CRIT_EXIT();
                while (next) {
                    if (_mvr_stacking_set_port_type(isid, &mvrif) != TRUE) {
                        T_D("MVR set VLAN-%u tagging failed in isid-%d", mvrif.basic.param.vid, isid);
                    }

                    MVR_CRIT_ENTER();
                    next = (vtss_mvr_interface_get_next(&mvrif) == VTSS_OK);
                    MVR_CRIT_EXIT();
                }

                MVR_CRIT_ENTER();
                vtss_mvr_set_mode(TRUE);
                MVR_CRIT_EXIT();
            } else {
                _mvr_stacking_reset_port_type(isid);

                MVR_CRIT_ENTER();
                vtss_mvr_set_mode(FALSE);
                MVR_CRIT_EXIT();
            }
            mvr_sm_event_set(MVR_EVENT_PKT_HANDLER);

            continue;
        } else {
            if (apply_mode == VTSS_IPMCMVR_ENABLED) {
                memset(&mvrif, 0x0, sizeof(vtss_mvr_interface_t));
                MVR_CRIT_ENTER();
                next = (vtss_mvr_interface_get_next(&mvrif) == VTSS_OK);
                MVR_CRIT_EXIT();
                while (next) {
                    if (_mvr_stacking_set_port_type(isid, &mvrif) != TRUE) {
                        T_D("MVR set VLAN-%u tagging failed in isid-%d", mvrif.basic.param.vid, isid);
                    }

                    MVR_CRIT_ENTER();
                    next = (vtss_mvr_interface_get_next(&mvrif) == VTSS_OK);
                    MVR_CRIT_EXIT();
                }
            } else {
                _mvr_stacking_reset_port_type(isid);
            }
        }

        mvr_msg_alloc(MVR_MSG_ID_GLOBAL_SET_REQ, &buf, FALSE, TRUE);
        msg = (mvr_msg_global_set_req_t *)buf.msg;

        msg->global_setting.mvr_state = apply_mode;
        msg->isid = isid;
        msg->msg_id = MVR_MSG_ID_GLOBAL_SET_REQ;
        mvr_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Set STACK IPMC FAST LEAVE PORT */
static vtss_rc mvr_stacking_set_fastleave_port(vtss_isid_t isid_add)
{
    mvr_msg_buf_t                       buf;
    mvr_msg_port_fast_leave_set_req_t   *msg;
    switch_iter_t                       sit;
    vtss_isid_t                         isid;
    port_iter_t                         pit;
    mvr_conf_fast_leave_t               *fast_leave;
    ipmc_port_bfs_t                     fastleave_setting;
    u32                                 i, temp_port_mask;

    if (!msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    T_D("enter, isid: %d", isid_add);

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if ((isid_add != VTSS_ISID_GLOBAL) && (isid_add != isid)) {
            continue;
        }

        if (ipmc_lib_isid_is_local(isid)) {
            VTSS_PORT_BF_CLR(fastleave_setting.member_ports);
            MVR_CRIT_ENTER();
            fast_leave = &mvr_running.mvr_conf.mvr_conf_fast_leave[ipmc_lib_isid_convert(TRUE, isid)];
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;

                VTSS_PORT_BF_SET(fastleave_setting.member_ports,
                                 i,
                                 VTSS_PORT_BF_GET(fast_leave->ports, i));
            }

            vtss_mvr_set_fast_leave_ports(&fastleave_setting);
            MVR_CRIT_EXIT();

            continue;
        }

        mvr_msg_alloc(MVR_MSG_ID_PORT_FAST_LEAVE_SET_REQ, &buf, FALSE, TRUE);
        msg = (mvr_msg_port_fast_leave_set_req_t *)buf.msg;

        MVR_CRIT_ENTER();
        memcpy(&msg->fast_leave,
               &mvr_running.mvr_conf.mvr_conf_fast_leave[ipmc_lib_isid_convert(TRUE, isid)],
               sizeof(mvr_conf_fast_leave_t));
        MVR_CRIT_EXIT();

        temp_port_mask = 0;
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            if (VTSS_PORT_BF_GET(msg->fast_leave.ports, i)) {
                temp_port_mask |= (1 << i);
            }
        }

        msg->msg_id = MVR_MSG_ID_PORT_FAST_LEAVE_SET_REQ;
        msg->isid = isid;
        mvr_msg_tx(&buf, isid, sizeof(*msg));

        T_D("(TX) (ISID = %d)temp_port_mask = 0x%x", isid_add, temp_port_mask);
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Send request and wait for response */
static cyg_flag_value_t mvr_stacking_send_req(vtss_isid_t isid, ipmc_ip_version_t ipmc_version,
                                              vtss_vid_t vid, vtss_ipv6_t *group_addr,
                                              mvr_msg_id_t msg_id,
                                              BOOL type, vtss_ipv6_t *srclist)
{
    cyg_flag_t                      *flags;
    mvr_msg_vlan_entry_get_req_t    *msg;
    mvr_msg_buf_t                   buf;
    cyg_flag_value_t                flag, retVal;
    cyg_tick_count_t                time_wait;

    T_D("enter(isid: %d)", isid);

    retVal = 0;
    flag = (1 << isid);

    mvr_msg_alloc(msg_id, &buf, TRUE, TRUE);
    msg = (mvr_msg_vlan_entry_get_req_t *)buf.msg;

    msg->msg_id = msg_id;
    msg->version = ipmc_version;

    /* Default: MVR_MSG_ID_VLAN_ENTRY_GET_REQ */
    msg->vid = vid;
    if (msg_id == MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ) {
        if (group_addr) {
            memcpy(&msg->group_addr, group_addr, sizeof(vtss_ipv6_t));
        }
    }
    if (msg_id == MVR_MSG_ID_GROUP_SRCLIST_WALK_REQ) {
        if (group_addr) {
            memcpy(&msg->group_addr, group_addr, sizeof(vtss_ipv6_t));
        }
        if (srclist) {
            memcpy(&msg->srclist_addr, srclist, sizeof(vtss_ipv6_t));
        }
        msg->srclist_type = type;
    }

    MVR_GET_CRIT_ENTER();
    flags = &mvr_running.vlan_entry_flags;
    cyg_flag_maskbits(flags, ~flag);
    MVR_GET_CRIT_EXIT();

    mvr_msg_tx(&buf, isid, sizeof(*msg));

    time_wait = cyg_current_time() + VTSS_OS_MSEC2TICK(MVR_REQ_TIMEOUT);
    MVR_GET_CRIT_ENTER();
    if (!(cyg_flag_timed_wait(flags, flag, CYG_FLAG_WAITMODE_OR, time_wait) & flag)) {
        retVal = 1;
    }
    MVR_GET_CRIT_EXIT();

    return retVal;
}

static vtss_rc mvr_stacking_set_intf_entry(vtss_isid_t isid,
                                           ipmc_operation_action_t action,
                                           vtss_mvr_interface_t *entry,
                                           ipmc_ip_version_t ipmc_version)
{
    mvr_msg_buf_t                   buf;
    mvr_msg_vlan_set_req_t          *msg;
    vlan_mgmt_entry_t               vlan_entry;
    port_iter_t                     pit;
    char                            mvrBufPort[MGMT_PORT_BUF_SIZE];
    BOOL                            mvrFwdMap[VTSS_PORT_ARRAY_SIZE];
    u32                             idx;
    ipmc_intf_entry_t               *intf;
    ipmc_prot_intf_entry_param_t    *param;

    if (!msg_switch_is_master() || !entry) {
        return VTSS_RC_ERROR;
    }

    T_D("enter, isid: %d", isid);

    intf = &entry->basic;
    param = &intf->param;
    intf->ipmc_version = ipmc_version;

    if (action == IPMC_OP_DEL) {
        memset(&vlan_entry, 0x0, sizeof(vlan_mgmt_entry_t));
        if (vlan_mgmt_vlan_get(isid, param->vid, &vlan_entry, FALSE, VLAN_USER_ALL) == VTSS_OK) {
            if (vlan_mgmt_vlan_del(isid, param->vid, MVR_VLAN_TYPE) != VTSS_OK) {
                T_D("MVR delete VLAN-%u failed in isid-%d", param->vid, isid);
            } else {
                /* Refine specific MVR VLAN Tagging */
                if (_mvr_stacking_refine_port_type(isid) != TRUE) {
                    T_D("MVR refine VLAN-%u tagging failed in isid-%d", param->vid, isid);
                }
            }
        }
    } else {
        memset(&vlan_entry, 0x0, sizeof(vlan_mgmt_entry_t));
        vlan_entry.vid = param->vid;
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            idx = pit.iport;

            if (entry->ports[idx] != MVR_PORT_ROLE_INACT) {
                vlan_entry.ports[idx] = 1;
            }
        }

        if (vlan_mgmt_vlan_add(isid, &vlan_entry, MVR_VLAN_TYPE) != VTSS_OK) {
            T_D("MVR add VLAN-%u failed in isid-%d", param->vid, isid);
        } else {
            /* Set MVR VLAN Tagging */
            if (_mvr_stacking_set_port_type(isid, entry) != TRUE) {
                T_D("MVR set VLAN-%u tagging failed in isid-%d", param->vid, isid);
            }
        }
    }

    if (ipmc_lib_isid_is_local(isid)) {
        MVR_CRIT_ENTER();
        if (vtss_mvr_interface_set(action, entry) != VTSS_OK) {
            T_D("MVR_MSG_ID_VLAN_ENTRY_SET_REQ Failed!");
        }
        MVR_CRIT_EXIT();
    } else {
        mvr_mgmt_interface_t    *mvrif;
        mvr_conf_intf_entry_t   *p;
        mvr_conf_port_role_t    *q;

        T_D("MVR_MSG_ID_VLAN_ENTRY_SET_REQ SndTo(%d) with VID:%u", isid, param->vid);

        mvr_msg_alloc(MVR_MSG_ID_VLAN_ENTRY_SET_REQ, &buf, FALSE, TRUE);
        msg = (mvr_msg_vlan_set_req_t *)buf.msg;

        msg->msg_id = MVR_MSG_ID_VLAN_ENTRY_SET_REQ;
        msg->isid = isid;
        msg->action = action;
        msg->version = ipmc_version;
        mvrif = &msg->vlan_entry;
        mvrif->vid = param->vid;

        p = &mvrif->intf;
        q = &mvrif->role;
        p->vid = q->vid = mvrif->vid;

        p->valid = TRUE;
        memcpy(p->name, entry->name, MVR_NAME_MAX_LEN);
        p->name[MVR_NAME_MAX_LEN - 1] = 0;
        p->mode = entry->mode;
        p->vtag = entry->vtag;
        p->querier4_address = param->querier.QuerierAdrs4;
        p->priority = entry->priority;
        p->profile_index = entry->profile_index;

        p->compatibility = param->cfg_compatibility;
        p->robustness_variable = param->querier.RobustVari;
        p->query_interval = param->querier.QueryIntvl;
        p->query_response_interval = param->querier.MaxResTime;
        p->last_listener_query_interval = param->querier.LastQryItv;
        p->unsolicited_report_interval = param->querier.UnsolicitR;

        memset(mvrFwdMap, 0x0, sizeof(mvrFwdMap));
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            idx = pit.iport;

            q->ports[idx] = entry->ports[idx];
            if (q->ports[idx] != MVR_PORT_ROLE_INACT) {
                mvrFwdMap[idx] = TRUE;
            }
        }
        q->valid = TRUE;

        T_D("SendTo ISID-%d with VID=%u Ports=%s",
            isid, q->vid,
            mgmt_iport_list2txt(mvrFwdMap, mvrBufPort));

        mvr_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid);

    return VTSS_OK;
}

static vtss_mvr_interface_t mvr_intf4get;
static vtss_rc mvr_stacking_get_intf_entry(vtss_isid_t isid_add,
                                           ipmc_prot_intf_entry_param_t *intf_param,
                                           ipmc_ip_version_t ipmc_version)
{
    vtss_ipv6_t zero_addr;

    if (!msg_switch_is_master() || !msg_switch_configurable(isid_add) || !intf_param) {
        return VTSS_RC_ERROR;
    }

    T_D("enter, isid: %d, vid_no:%d/ver:%d", isid_add, intf_param->vid, ipmc_version);

    if (ipmc_lib_isid_is_local(isid_add)) {
        vtss_mvr_interface_t    *entry;
        ipmc_intf_entry_t       *intf;

        MVR_GET_CRIT_ENTER();
        entry = &mvr_intf4get;
        memset(entry, 0x0, sizeof(vtss_mvr_interface_t));
        intf = &entry->basic;
        intf->ipmc_version = ipmc_version;
        intf->param.vid = intf_param->vid;

        if (vtss_mvr_interface_get(entry) == VTSS_OK) {
            memcpy(intf_param, &intf->param, sizeof(ipmc_prot_intf_entry_param_t));
        } else {
            intf_param->vid = 0;
        }
        MVR_GET_CRIT_EXIT();

        T_D("Done-Local(isid=%d)", isid_add);
    } else {
        MVR_GET_CRIT_ENTER();
        ipmc_lib_get_all_zero_ipv6_addr(&zero_addr);
        memset(&mvr_running.interface_entry, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
        MVR_GET_CRIT_EXIT();

        if (mvr_stacking_send_req(isid_add, ipmc_version,
                                  intf_param->vid, &zero_addr,
                                  MVR_MSG_ID_VLAN_ENTRY_GET_REQ,
                                  FALSE, NULL)) {
            T_D("timeout, MVR_MSG_ID_VLAN_ENTRY_GET_REQ(isid=%d)", isid_add);
            return VTSS_RC_ERROR;
        }

        MVR_GET_CRIT_ENTER();
        memcpy(intf_param, &mvr_running.interface_entry, sizeof(ipmc_prot_intf_entry_param_t));
        MVR_GET_CRIT_EXIT();

        T_D("Done-Remote(isid=%d)", isid_add);
    }

    T_D("exit, isid: %d, vid_no: %d", isid_add, intf_param->vid);
    if (intf_param->vid != 0) {
        return VTSS_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

static vtss_rc mvr_stacking_get_next_group_srclist_entry(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version, vtss_vid_t vid, vtss_ipv6_t *addr, ipmc_prot_group_srclist_t *srclist)
{
    char    buf[40];

    if (!msg_switch_is_master() || !msg_switch_exists(isid_add) || !addr || !srclist) {
        return VTSS_RC_ERROR;
    }

    memset(buf, 0x0, sizeof(buf));
    T_D("enter->isid:%d, VID:%d/VER:%d, SRC=%s[%s]",
        isid_add,
        vid,
        ipmc_version,
        srclist->type ? "ALLOW" : "BLOCK",
        misc_ipv6_txt(&srclist->srclist.src_ip, buf));

    if (ipmc_lib_isid_is_local(isid_add)) {
        ipmc_group_entry_t  grp;
        BOOL                valid;

        grp.ipmc_version = ipmc_version;
        grp.vid = vid;
        memcpy(&grp.group_addr, addr, sizeof(vtss_ipv6_t));
        MVR_GET_CRIT_ENTER();
        valid = vtss_mvr_intf_group_get(&grp);
        MVR_GET_CRIT_EXIT();

        srclist->valid = FALSE;
        if (valid) {
            ipmc_db_ctrl_hdr_t  *slist;

            if (srclist->type) {
                slist = grp.info->db.ipmc_sf_do_forward_srclist;
            } else {
                slist = grp.info->db.ipmc_sf_do_not_forward_srclist;
            }

            if (slist &&
                ((srclist->cntr = IPMC_LIB_DB_GET_COUNT(slist)) != 0)) {
                ipmc_sfm_srclist_t  entry;

                memcpy(&entry.src_ip, &srclist->srclist.src_ip, sizeof(vtss_ipv6_t));
                if (ipmc_lib_srclist_buf_get_next(slist, &entry)) {
                    /* srclist->type remains the same */
                    memcpy(&srclist->srclist, &entry, sizeof(ipmc_sfm_srclist_t));
                    srclist->valid = TRUE;
                }
            }
        }
    } else {
        MVR_GET_CRIT_ENTER();
        memset(&mvr_running.group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
        MVR_GET_CRIT_EXIT();

        if (mvr_stacking_send_req(isid_add, ipmc_version,
                                  vid, addr,
                                  MVR_MSG_ID_GROUP_SRCLIST_WALK_REQ,
                                  srclist->type, &srclist->srclist.src_ip)) {
            T_D("timeout, MVR_MSG_ID_GROUP_SRCLIST_WALK_REQ(isid=%d)", isid_add);
            return VTSS_RC_ERROR;
        }

        MVR_GET_CRIT_ENTER();
        memcpy(srclist, &mvr_running.group_srclist_entry, sizeof(ipmc_prot_group_srclist_t));
        MVR_GET_CRIT_EXIT();
    }

    T_D("exit->isid:%d, (GOT-%s)%s-vid:%d/source_address = %s",
        isid_add,
        srclist->valid ? "VALID" : "INVALID",
        srclist->type ? "ALLOW" : "BLOCK",
        vid,
        misc_ipv6_txt(&srclist->srclist.src_ip, buf));
    return VTSS_OK;
}

static vtss_rc mvr_stacking_get_next_intf_group_entry(vtss_isid_t isid_add, vtss_vid_t vid, ipmc_prot_intf_group_entry_t *intf_group, ipmc_ip_version_t ipmc_version)
{
    char    buf[40];

    if (!msg_switch_is_master() || !msg_switch_exists(isid_add) || !intf_group) {
        return VTSS_RC_ERROR;
    }

    memset(buf, 0x0, sizeof(buf));
    T_D("enter->isid: %d, vid_no:%d/ver:%d, group_address = %s",
        isid_add,
        vid,
        ipmc_version,
        misc_ipv6_txt(&intf_group->group_addr, buf));

    if (ipmc_lib_isid_is_local(isid_add)) {
        ipmc_group_entry_t  grp;

        grp.ipmc_version = ipmc_version;
        grp.vid = vid;
        memcpy(&grp.group_addr, &intf_group->group_addr, sizeof(vtss_ipv6_t));
        intf_group->valid = FALSE;
        MVR_GET_CRIT_ENTER();
        intf_group->valid = vtss_mvr_intf_group_get_next(&grp);
        MVR_GET_CRIT_EXIT();

        if (intf_group->valid) {
            intf_group->ipmc_version = grp.ipmc_version;
            intf_group->vid = grp.vid;
            memcpy(&intf_group->group_addr, &grp.group_addr, sizeof(vtss_ipv6_t));
            memcpy(&intf_group->db, &grp.info->db, sizeof(ipmc_group_db_t));
        } else {
            T_D("no more group entry in vlan-%d", vid);
        }
    } else {
        MVR_GET_CRIT_ENTER();
        memset(&mvr_running.intf_group_entry, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
        MVR_GET_CRIT_EXIT();

        if (mvr_stacking_send_req(isid_add, ipmc_version,
                                  vid, &intf_group->group_addr,
                                  MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ,
                                  FALSE, NULL)) {
            T_D("timeout, MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ(isid=%d)", isid_add);
            return VTSS_RC_ERROR;
        }

        MVR_GET_CRIT_ENTER();
        memcpy(intf_group, &mvr_running.intf_group_entry, sizeof(ipmc_prot_intf_group_entry_t));
        MVR_GET_CRIT_EXIT();
    }

    T_D("exit->isid: %d, (GOT)%s-vid:%d/group_address = %s",
        isid_add,
        intf_group->valid ? "VALID" : "INVALID",
        vid,
        misc_ipv6_txt(&intf_group->group_addr, buf));
    return VTSS_OK;
}

/* STACK IPMC clear STAT Counter */
static vtss_rc mvr_stacking_clear_statistics(vtss_isid_t isid_add, vtss_vid_t vid)
{
    mvr_msg_buf_t                       buf;
    mvr_msg_stat_counter_clear_req_t    *msg;
    switch_iter_t                       sit;
    vtss_isid_t                         isid;

    T_D("enter, isid: %d", isid_add);

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if ((isid_add != VTSS_ISID_GLOBAL) && (isid_add != isid)) {
            continue;
        }

        mvr_msg_alloc(MVR_MSG_ID_STAT_COUNTER_CLEAR_REQ, &buf, FALSE, TRUE);
        msg = (mvr_msg_stat_counter_clear_req_t *)buf.msg;

        msg->msg_id = MVR_MSG_ID_STAT_COUNTER_CLEAR_REQ;
        msg->isid = isid;
        msg->vid = vid;
        mvr_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

static vtss_rc mvr_stacking_sync_mgmt_conf(vtss_isid_t isid_add, ipmc_lib_mgmt_info_t *sys_mgmt)
{
    mvr_msg_buf_t               buf;
    mvr_msg_sys_mgmt_set_req_t  *msg;
    switch_iter_t               sit;
    vtss_isid_t                 isid;
    ipmc_mgmt_ipif_t            *ifp;
    u32                         ifx;

    if (!msg_switch_is_master() || !sys_mgmt) {
        return VTSS_RC_ERROR;
    }

    T_I("SYNC(isid: %d):MAC[%s]", isid_add, misc_mac2str(sys_mgmt->mac_addr));

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if (((isid_add != VTSS_ISID_GLOBAL) && (isid_add != isid)) ||
            ipmc_lib_isid_is_local(isid)) {
            continue;
        }

        mvr_msg_alloc(MVR_MSG_ID_SYS_MGMT_SET_REQ, &buf, FALSE, TRUE);
        msg = (mvr_msg_sys_mgmt_set_req_t *)buf.msg;

        msg->msg_id = MVR_MSG_ID_SYS_MGMT_SET_REQ;
        IPMC_MGMT_MAC_ADR_GET(sys_mgmt, msg->mgmt_mac);
        msg->intf_cnt = 0;
        for (ifx = 0; ifx < VTSS_IPMC_MGMT_IPIF_MAX_CNT; ifx++) {
            if (IPMC_MGMT_IPIF_VALID(sys_mgmt, ifx)) {
                T_I("SYNC(%u) INTF-ADR-%u(VID:%u) is %s",
                    (msg->intf_cnt + 1),
                    IPMC_MGMT_IPIF_ADRS4(sys_mgmt, ifx),
                    IPMC_MGMT_IPIF_IDVLN(sys_mgmt, ifx),
                    IPMC_MGMT_IPIF_STATE(sys_mgmt, ifx) ? "Up" : "Down");

                ifp = &msg->ip_addr[msg->intf_cnt++];
                ipmc_mgmt_intf_vidx(ifp) = IPMC_MGMT_IPIF_IDVLN(sys_mgmt, ifx);
                ipmc_mgmt_intf_live(ifp) = IPMC_MGMT_IPIF_VALID(sys_mgmt, ifx);
                ipmc_mgmt_intf_adr4(ifp) = IPMC_MGMT_IPIF_ADRS4(sys_mgmt, ifx);
                ipmc_mgmt_intf_opst(ifp) = IPMC_MGMT_IPIF_STATE(sys_mgmt, ifx);
            }
        }
        mvr_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

static BOOL mvr_get_is_vlan_conf_full(void)
{
    u16                     idx;
    mvr_conf_intf_entry_t   *entry;

    for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
        entry = &mvr_running.mvr_conf.mvr_conf_vlan[idx];

        if (!entry->valid) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL mvr_get_vlan_conf_entry(mvr_conf_intf_entry_t *entry)
{
    u16                     idx;
    mvr_conf_intf_entry_t   *p;

    if (!entry || (entry->vid > MVR_VID_MAX)) {
        return FALSE;
    }

    for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
        p = &mvr_running.mvr_conf.mvr_conf_vlan[idx];

        if (p->valid && (p->vid == entry->vid)) {
            memcpy(entry, p, sizeof(mvr_conf_intf_entry_t));
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL mvr_get_next_vlan_conf_entry(mvr_conf_intf_entry_t *entry)
{
    u16                     idx;
    mvr_conf_intf_entry_t   *p, *q;

    if (!entry || (entry->vid > MVR_VID_MAX)) {
        return FALSE;
    }

    q = NULL;
    for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
        p = &mvr_running.mvr_conf.mvr_conf_vlan[idx];

        if (!p->valid) {
            continue;
        }

        if (p->vid > entry->vid) {
            if (!q) {
                q = p;
            } else {
                if (p->vid < q->vid) {
                    q = p;
                }
            }
        }
    }

    if (q != NULL) {
        memcpy(entry, q, sizeof(mvr_conf_intf_entry_t));
        return TRUE;
    } else {
        return FALSE;
    }
}

static i32 _mvr_intf_name_vid_cmp_func(void *elm1, void *elm2)
{
    mvr_conf_intf_entry_t   *element1, *element2;
    i8                      name1[MVR_NAME_MAX_LEN], name2[MVR_NAME_MAX_LEN];

    if (!elm1 || !elm2) {
        T_W("MVR_ASSERT(_mvr_intf_name_vid_cmp_func)");
        for (;;) {}
    }

    element1 = (mvr_conf_intf_entry_t *)elm1;
    element2 = (mvr_conf_intf_entry_t *)elm2;
    memset(name1, 0x0, sizeof(name1));
    strncpy(name1, element1->name, strlen(element1->name));
    memset(name2, 0x0, sizeof(name2));
    strncpy(name2, element2->name, strlen(element2->name));

    if (strncmp(name1, name2, MVR_NAME_MAX_LEN) > 0) {
        return 1;
    } else if (strncmp(name1, name2, MVR_NAME_MAX_LEN) < 0) {
        return -1;
    } else if (element1->vid > element2->vid) {
        return 1;
    } else if (element1->vid < element2->vid) {
        return -1;
    } else {
        return 0;
    }
}

static BOOL mvr_get_vlan_conf_entry_by_name(mvr_conf_intf_entry_t *entry)
{
    u16                     idx;
    int                     len;
    mvr_conf_intf_entry_t   *p;

    if (!entry) {
        return FALSE;
    }

    len = strlen(entry->name);
    for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
        p = &mvr_running.mvr_conf.mvr_conf_vlan[idx];

        if (!p->valid) {
            continue;
        }

        if (len != strlen(p->name)) {
            continue;
        }

        if (!strncmp(entry->name, p->name, len)) {
            memcpy(entry, p, sizeof(mvr_conf_intf_entry_t));
            return TRUE;
        }
    }

    return FALSE;
}

static BOOL mvr_get_next_vlan_conf_entry_by_name(mvr_conf_intf_entry_t *entry)
{
    u16                     idx;
    mvr_conf_intf_entry_t   *p, *q;

    if (!entry) {
        return FALSE;
    }

    q = NULL;
    for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
        p = &mvr_running.mvr_conf.mvr_conf_vlan[idx];

        if (!p->valid) {
            continue;
        }

        if (_mvr_intf_name_vid_cmp_func((void *)p, (void *)entry) > 0) {
            if (!q) {
                q = p;
            } else {
                if (_mvr_intf_name_vid_cmp_func((void *)p, (void *)q) < 0) {
                    q = p;
                }
            }
        }
    }

    if (q != NULL) {
        memcpy(entry, q, sizeof(mvr_conf_intf_entry_t));
        return TRUE;
    } else {
        return FALSE;
    }
}

/* Obsoleted API for MVID */
vtss_rc mvr_mgmt_get_mvid(u16 *obs_mvid)
{
    mvr_conf_intf_entry_t   entry;

    if (!obs_mvid) {
        return VTSS_RC_ERROR;
    }

    memset(&entry, 0x0, sizeof(mvr_conf_intf_entry_t));
    MVR_CRIT_ENTER();
    if (mvr_get_next_vlan_conf_entry(&entry) != VTSS_OK) {
        MVR_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }
    MVR_CRIT_EXIT();

    *obs_mvid = entry.vid;
    return VTSS_OK;
}

static BOOL _mvr_validate_profile_overlap(u16 mvid, u32 pdx, vtss_rc *rc)
{
    u32                         vdx;
    BOOL                        flag, first_permit, tmp;
    mvr_conf_intf_entry_t       intf;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *pf;
    ipmc_lib_rule_t             *rule;
    ipmc_profile_rule_t         *ptr, *bgn, *end, *chk, buf;

    if (!rc) {
        return FALSE;
    }

    if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
        *rc = IPMC_ERROR_MEMORY_NG;
        return FALSE;
    }

    *rc = VTSS_OK;
    if (!pdx) {
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return TRUE;
    }

    pf = &pfm->profile;
    pf->data.index = pdx;
    if (!ipmc_lib_fltr_profile_get(pf, FALSE)) {
        *rc = IPMC_ERROR_ENTRY_NOT_FOUND;
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return FALSE;
    }

    flag = TRUE;
    first_permit = FALSE;
    bgn = end = ptr = NULL;
    while ((ptr = ipmc_lib_profile_tree_get_next(pdx, ptr, &tmp)) != NULL) {
        if ((rule = ptr->rule) == NULL) {
            continue;
        }

        if (bgn) {
            first_permit = FALSE;

            if (bgn->version != ptr->version) {
                end = NULL;
                bgn = ptr;

                continue;
            }

            if (rule->action == IPMC_ACTION_PERMIT) {
                end = ptr;
            } else {
                end = NULL;
                bgn = ptr;
            }
        } else {
            bgn = ptr;
            if (rule->action == IPMC_ACTION_PERMIT) {
                first_permit = TRUE;
                end = ptr;
            }
        }

        if (!end) {
            continue;
        }

        memset(&intf, 0x0, sizeof(mvr_conf_intf_entry_t));
        while (mvr_get_next_vlan_conf_entry(&intf)) {
            if (intf.vid == mvid) {
                continue;
            }
            if (intf.profile_index == MVR_CONF_DEF_INTF_PROFILE) {
                continue;
            }

            if (intf.profile_index == pdx) {
                flag = FALSE;
                break;
            }

            pf = &pfm->profile;
            memset(pf, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
            pf->data.index = intf.profile_index;
            if ((pf = ipmc_lib_fltr_profile_get(pf, FALSE)) != NULL) {
                vdx = pf->data.index;

                memcpy(&buf, bgn, sizeof(ipmc_profile_rule_t));
                if (first_permit) {
                    IPMC_LIB_ADRS_SET(&buf.grp_adr, 0x0);
                }
                chk = &buf;
                if ((chk = ipmc_lib_profile_tree_get_next(vdx, chk, &tmp)) != NULL) {
                    if (chk->version != bgn->version) {
                        continue;
                    }

                    if ((rule = chk->rule) != NULL) {
                        if (rule->action == IPMC_ACTION_PERMIT) {
                            flag = FALSE;
                            break;
                        }
                    }

                    memcpy(&buf, end, sizeof(ipmc_profile_rule_t));
                    buf.vir_idx = 0x0;
                    chk = &buf;
                    if ((chk = ipmc_lib_profile_tree_get_next(vdx, chk, &tmp)) != NULL) {
                        if (chk->version != end->version) {
                            continue;
                        }

                        if ((rule = chk->rule) != NULL) {
                            if (rule->action == IPMC_ACTION_PERMIT) {
                                flag = FALSE;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if (!flag) {
            break;
        } else {
            bgn = end;
            end = NULL;
        }
    }

    if (!flag) {
        *rc = IPMC_ERROR_ENTRY_OVERLAPPED;
    }

    IPMC_MEM_PROFILE_MGIVE(pfm);
    return flag;
}

static BOOL _mvr_sanity_chk_interface_entry(vtss_isid_t isid,
                                            mvr_mgmt_interface_t *entry,
                                            ipmc_operation_action_t action,
                                            vtss_rc *rc)
{
    u16                     vid;
    port_iter_t             pit;
    u32                     idx;
    mvr_conf_intf_entry_t   chk, *p;
    mvr_conf_port_role_t    *q;

    if (!entry || !rc) {
        return FALSE;
    }

    *rc = VTSS_OK;
    vid = entry->vid;
    if (vid < MVR_VID_MIN || vid > MVR_VID_MAX) {
        *rc = IPMC_ERROR_PARM;
        return FALSE;
    }

    chk.vid = vid;
    MVR_CRIT_ENTER();
    if (mvr_get_vlan_conf_entry(&chk)) {
        if (action == IPMC_OP_ADD) {
            *rc = IPMC_ERROR_ENTRY_INVALID;
        }
    } else {
        if (action == IPMC_OP_ADD) {
            if (mvr_get_is_vlan_conf_full()) {
                *rc = IPMC_ERROR_TABLE_IS_FULL;
            }
        } else {
            *rc = IPMC_ERROR_VLAN_NOT_FOUND;
        }
    }

    if (*rc == VTSS_OK && strlen(entry->intf.name)) {
        memset(chk.name, 0x0, MVR_NAME_MAX_LEN);
        strncpy(chk.name, entry->intf.name, strlen(entry->intf.name));
        if (mvr_get_vlan_conf_entry_by_name(&chk) &&
            chk.vid != vid) {
            *rc = IPMC_ERROR_ENTRY_NAME_DUPLICATED;
        }
    }
    MVR_CRIT_EXIT();

    if (*rc != VTSS_OK) {
        return FALSE;
    }

    if (action == IPMC_OP_ADD) {
        vlan_mgmt_entry_t   vlan_entry;

        vlan_entry.vid = vid;
        if (vlan_mgmt_vlan_get(isid, vid, &vlan_entry, FALSE, VLAN_USER_ALL) == VTSS_OK) {
            *rc = IPMC_ERROR_VLAN_ACTIVE;
        }
    }

    p = &entry->intf;
    q = &entry->role;

    if ((p->mode != MVR_INTF_MODE_DYNA) &&
        (p->mode != MVR_INTF_MODE_COMP) &&
        (p->mode != MVR_INTF_MODE_INIT)) {
        *rc = IPMC_ERROR_PARM;
        return FALSE;
    }

    if ((p->vtag != IPMC_INTF_UNTAG) &&
        (p->vtag != IPMC_INTF_TAGED)) {
        *rc = IPMC_ERROR_PARM;
        return FALSE;
    }
    if (p->priority > IPMC_PARAM_MAX_PRIORITY) {
        *rc = IPMC_ERROR_PARM;
        return FALSE;
    }

    if (p->querier4_address) {
        u8  adrc = (p->querier4_address >> 24) & 0xFF;

        if ((adrc == 127) || (adrc > 223)) {
            *rc = IPMC_ERROR_PARM;
            return FALSE;
        }
    }

    if (p->last_listener_query_interval > 31744) {
        *rc = IPMC_ERROR_PARM;
        return FALSE;
    }

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        idx = pit.iport;

        if ((q->ports[idx] != MVR_PORT_ROLE_INACT) &&
            (q->ports[idx] != MVR_PORT_ROLE_SOURC) &&
            (q->ports[idx] != MVR_PORT_ROLE_RECVR)) {
            *rc = IPMC_ERROR_PARM;
            return FALSE;
        }

        if (q->ports[idx] != MVR_PORT_ROLE_INACT) {
            BOOL                    next;
            vtss_mvr_interface_t    mvrif;

            memset(&mvrif, 0x0, sizeof(vtss_mvr_interface_t));
            MVR_CRIT_ENTER();
            next = (vtss_mvr_interface_get_next(&mvrif) == VTSS_OK);
            MVR_CRIT_EXIT();
            while (next) {
                if ((entry->vid != mvrif.basic.param.vid) &&
                    (mvrif.ports[idx] != MVR_PORT_ROLE_INACT)) {
                    if (q->ports[idx] != mvrif.ports[idx]) {
                        *rc = IPMC_ERROR_PARM;
                        return FALSE;
                    }
                }

                MVR_CRIT_ENTER();
                next = (vtss_mvr_interface_get_next(&mvrif) == VTSS_OK);
                MVR_CRIT_EXIT();
            }
        }
    }

    return TRUE;
}

/* Only mvr_mgmt_set_intf_entry can access this func */
static BOOL _mvr_mgmt_set_intf_role(vtss_isid_t isid, ipmc_operation_action_t action, mvr_conf_port_role_t *entry)
{
    port_iter_t             pit;
    u32                     idx;
    mvr_conf_port_role_t    *p;

    switch ( action ) {
    case IPMC_OP_ADD:
        for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
            p = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid)].intf[idx];

            if (!p->valid) {
                memcpy(p, entry, sizeof(mvr_conf_port_role_t));
                p->valid = TRUE;

                if (ipmc_lib_isid_is_local(isid)) {
                    memcpy(&mvr_running.mvr_conf.mvr_conf_role[VTSS_ISID_LOCAL].intf[idx],
                           p,
                           sizeof(mvr_conf_port_role_t));
                }

                return TRUE;
            }
        }

        break;
    case IPMC_OP_DEL:
        for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
            p = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid)].intf[idx];

            if (!p->valid || (p->vid != entry->vid)) {
                continue;
            }

            memset(p, 0x0, sizeof(mvr_conf_port_role_t));
            /* p->valid = FALSE; */
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                p->ports[pit.iport] = MVR_CONF_DEF_PORT_ROLE;
            }

            if (ipmc_lib_isid_is_local(isid)) {
                memcpy(&mvr_running.mvr_conf.mvr_conf_role[VTSS_ISID_LOCAL].intf[idx],
                       p,
                       sizeof(mvr_conf_port_role_t));
            }

            return TRUE;
        }

        break;
    case IPMC_OP_UPD:
    case IPMC_OP_SET:
        for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
            p = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid)].intf[idx];

            if (!p->valid || (p->vid != entry->vid)) {
                continue;
            }

            if (memcmp(p, entry, sizeof(mvr_conf_port_role_t))) {
                memcpy(p, entry, sizeof(mvr_conf_port_role_t));
            }

            p->valid = TRUE;

            if (ipmc_lib_isid_is_local(isid)) {
                memcpy(&mvr_running.mvr_conf.mvr_conf_role[VTSS_ISID_LOCAL].intf[idx],
                       p,
                       sizeof(mvr_conf_port_role_t));
            }

            return TRUE;
        }

        break;
    default:

        break;
    }

    return FALSE;
}

static BOOL _mvr_mgmt_set_intf_intf(ipmc_operation_action_t action, mvr_conf_intf_entry_t *entry)
{
    switch_iter_t           sit;
    u32                     idx, i;
    mvr_conf_intf_entry_t   *p;
    mvr_conf_intf_role_t    *q;

    switch ( action ) {
    case IPMC_OP_ADD:
        for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
            p = &mvr_running.mvr_conf.mvr_conf_vlan[idx];

            if (!p->valid) {
                u32 roleIdx;

                memcpy(p, entry, sizeof(mvr_conf_intf_entry_t));
                p->valid = TRUE;

                (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
                while (switch_iter_getnext(&sit)) {
                    i = sit.isid;

                    q = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, i)];
                    mvr_conf_reset_intf_role(i, (vtss_vid_t)entry->vid, q, TRUE);
                    for (roleIdx = 0; roleIdx < MVR_VLAN_MAX; roleIdx++) {
                        if (q->intf[roleIdx].valid) {
                            continue;
                        }

                        q->intf[roleIdx].valid = TRUE;
                        q->intf[roleIdx].vid = entry->vid;
                        break;
                    }

                    if (ipmc_lib_isid_is_local(i)) {
                        q = &mvr_running.mvr_conf.mvr_conf_role[VTSS_ISID_LOCAL];
                        mvr_conf_reset_intf_role(VTSS_ISID_LOCAL, (vtss_vid_t)entry->vid, q, TRUE);
                        for (roleIdx = 0; roleIdx < MVR_VLAN_MAX; roleIdx++) {
                            if (q->intf[roleIdx].valid) {
                                continue;
                            }

                            q->intf[roleIdx].valid = TRUE;
                            q->intf[roleIdx].vid = entry->vid;
                            break;
                        }
                    }
                }

                return TRUE;
            }
        }

        break;
    case IPMC_OP_DEL:
        for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
            p = &mvr_running.mvr_conf.mvr_conf_vlan[idx];

            if (!p->valid || (p->vid != entry->vid)) {
                continue;
            }

            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
            while (switch_iter_getnext(&sit)) {
                i = sit.isid;

                q = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, i)];
                mvr_conf_reset_intf_role(i, (vtss_vid_t)entry->vid, q, TRUE);

                if (ipmc_lib_isid_is_local(i)) {
                    q = &mvr_running.mvr_conf.mvr_conf_role[VTSS_ISID_LOCAL];
                    mvr_conf_reset_intf_role(VTSS_ISID_LOCAL, (vtss_vid_t)entry->vid, q, TRUE);
                }
            }
            mvr_conf_reset_intf(p, TRUE);
            p->valid = FALSE;

            return TRUE;
        }

        break;
    case IPMC_OP_UPD:
    case IPMC_OP_SET:
        for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
            p = &mvr_running.mvr_conf.mvr_conf_vlan[idx];

            if (!p->valid || (p->vid != entry->vid)) {
                continue;
            }

            if (memcmp(p, entry, sizeof(mvr_conf_intf_entry_t))) {
                memset(p->name, 0x0, sizeof(p->name));
                memcpy(p, entry, sizeof(mvr_conf_intf_entry_t));
                p->valid = TRUE;
                return TRUE;
            }

            break;
        }

        break;
    default:

        break;
    }

    return FALSE;
}

static vtss_mvr_interface_t mvr_intf4set;
/* Set MVR VLAN Interface */
vtss_rc mvr_mgmt_set_intf_entry(vtss_isid_t isid_in, ipmc_operation_action_t action, mvr_mgmt_interface_t *entry)
{
    vtss_rc                 sanity_rc;
    switch_iter_t           sit;
    vtss_isid_t             isid;
    BOOL                    flag;
    mvr_conf_intf_entry_t   *p, bkup;
    vtss_mvr_interface_t    *ifp;
    cyg_tick_count_t        exe_time_base;

    if (!entry || !msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    exe_time_base = cyg_current_time();
    sanity_rc = VTSS_RC_ERROR;
    if (isid_in == VTSS_ISID_GLOBAL) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
        while (switch_iter_getnext(&sit)) {
            if (!_mvr_sanity_chk_interface_entry(sit.isid, entry, action, &sanity_rc)) {
                return sanity_rc;
            }
        }
    } else {
        if (!_mvr_sanity_chk_interface_entry(isid_in, entry, action, &sanity_rc)) {
            return sanity_rc;
        }
    }

    MVR_CRIT_ENTER();
    if (action != IPMC_OP_DEL) {
        if (!_mvr_validate_profile_overlap(entry->vid, entry->intf.profile_index, &sanity_rc)) {
            MVR_CRIT_EXIT();
            return sanity_rc;
        }
    }

    if (action == IPMC_OP_ADD) {
        p = &entry->intf;
        memcpy(&bkup, p, sizeof(mvr_conf_intf_entry_t));
        mvr_conf_reset_intf(p, FALSE);

        p->mode = bkup.mode;
        p->vtag = bkup.vtag;
        p->querier4_address = bkup.querier4_address;
        p->priority = bkup.priority;
        p->profile_index = bkup.profile_index;
        p->last_listener_query_interval = bkup.last_listener_query_interval;
    }

    entry->intf.vid = entry->role.vid = entry->vid;
    flag = _mvr_mgmt_set_intf_intf(action, &entry->intf);
    if (isid_in != VTSS_ISID_GLOBAL) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
        while (switch_iter_getnext(&sit)) {
            isid = sit.isid;

            if (isid_in != isid) {
                continue;
            }

            flag |= _mvr_mgmt_set_intf_role(isid, action, &entry->role);
        }
    }

    ifp = &mvr_intf4set;
    MVR_CRIT_EXIT();

    if (flag) {
        mvr_sm_event_set(MVR_EVENT_CONFBLK_COMMIT);
    } else {
        return VTSS_OK;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if ((isid_in != VTSS_ISID_GLOBAL) && (isid_in != isid)) {
            continue;
        }

        MVR_CRIT_ENTER();
        _mvr_mgmt_transit_intf(isid_in, isid, ifp, entry, IPMC_IP_VERSION_IGMP);
        MVR_CRIT_EXIT();
        (void) mvr_stacking_set_intf_entry(isid, action, ifp, IPMC_IP_VERSION_IGMP);
        MVR_CRIT_ENTER();
        _mvr_mgmt_transit_intf(isid_in, isid, ifp, entry, IPMC_IP_VERSION_MLD);
        MVR_CRIT_EXIT();
        (void) mvr_stacking_set_intf_entry(isid, action, ifp, IPMC_IP_VERSION_MLD);
    }

    T_D("mvr_mgmt_set_intf_entry(%u) consumes %u ticks", entry->vid, (u32)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

/* Validate MVR VLAN Channel */
vtss_rc mvr_mgmt_validate_intf_channel(void)
{
    u16                     idx;
    vtss_rc                 ret;
    mvr_conf_intf_entry_t   *p;

    ret = VTSS_OK;
    MVR_CRIT_ENTER();
    if (mvr_running.mvr_conf.mvr_conf_global.mvr_state) {
        for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
            p = &mvr_running.mvr_conf.mvr_conf_vlan[idx];

            if (!p->valid || !p->profile_index) {
                continue;
            }

            if (!_mvr_validate_profile_overlap(p->vid, p->profile_index, &ret)) {
                break;
            }
        }
    }
    MVR_CRIT_EXIT();

    return ret;
}

/* Get MVR VLAN Interface By Name(String) */
vtss_rc mvr_mgmt_get_intf_entry_by_name(vtss_isid_t isid, mvr_mgmt_interface_t *entry)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!entry) {
        return VTSS_RC_ERROR;
    }

    if (strlen(entry->intf.name) >= MVR_NAME_MAX_LEN) {
        return VTSS_RC_ERROR;
    }

    MVR_CRIT_ENTER();
    if (mvr_get_vlan_conf_entry_by_name(&entry->intf)) {
        vtss_isid_t isid_get = isid;

        if (isid_get == VTSS_ISID_GLOBAL) {
            isid_get = VTSS_ISID_LOCAL;
        }

        entry->role.vid = entry->intf.vid;
        if (!mvr_get_port_role_entry(isid_get, &entry->role)) {
            MVR_CRIT_EXIT();
            return VTSS_RC_ERROR;
        }

        entry->vid = entry->intf.vid;
    } else {
        MVR_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }
    MVR_CRIT_EXIT();

    T_D("mvr_mgmt_get_intf_entry_by_name consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

/* Get-Next MVR VLAN Interface By Name(String) */
vtss_rc mvr_mgmt_get_next_intf_entry_by_name(vtss_isid_t isid, mvr_mgmt_interface_t *entry)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!entry) {
        return VTSS_RC_ERROR;
    }

    if (strlen(entry->intf.name) >= MVR_NAME_MAX_LEN) {
        return VTSS_RC_ERROR;
    }

    MVR_CRIT_ENTER();
    if (mvr_get_next_vlan_conf_entry_by_name(&entry->intf)) {
        vtss_isid_t isid_get = isid;

        if (isid_get == VTSS_ISID_GLOBAL) {
            isid_get = VTSS_ISID_LOCAL;
        }

        entry->role.vid = entry->intf.vid;
        if (!mvr_get_port_role_entry(isid_get, &entry->role)) {
            MVR_CRIT_EXIT();
            return VTSS_RC_ERROR;
        }

        entry->vid = entry->intf.vid;
    } else {
        MVR_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }
    MVR_CRIT_EXIT();

    T_D("mvr_mgmt_get_next_intf_entry_by_name consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

/* Get MVR VLAN Interface */
vtss_rc mvr_mgmt_get_intf_entry(vtss_isid_t isid, mvr_mgmt_interface_t *entry)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!entry) {
        return VTSS_RC_ERROR;
    }

    entry->intf.vid = entry->vid;
    MVR_CRIT_ENTER();
    if (mvr_get_vlan_conf_entry(&entry->intf)) {
        vtss_isid_t isid_get = isid;

        if (isid_get == VTSS_ISID_GLOBAL) {
            isid_get = VTSS_ISID_LOCAL;
        }

        entry->role.vid = entry->intf.vid;
        if (!mvr_get_port_role_entry(isid_get, &entry->role)) {
            T_D("mvr_get_port_role_entry cannot get port role for VID %u", entry->role.vid);
            MVR_CRIT_EXIT();
            return VTSS_RC_ERROR;
        }

        entry->vid = entry->intf.vid;
    } else {
        MVR_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }
    MVR_CRIT_EXIT();

    T_D("mvr_mgmt_get_intf_entry consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

/* Get-Next MVR VLAN Interface */
vtss_rc mvr_mgmt_get_next_intf_entry(vtss_isid_t isid, mvr_mgmt_interface_t *entry)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!entry) {
        return VTSS_RC_ERROR;
    }

    entry->intf.vid = entry->vid;
    MVR_CRIT_ENTER();
    if (mvr_get_next_vlan_conf_entry(&entry->intf)) {
        vtss_isid_t isid_get = isid;

        if (isid_get == VTSS_ISID_GLOBAL) {
            isid_get = VTSS_ISID_LOCAL;
        }

        entry->role.vid = entry->intf.vid;
        if (!mvr_get_port_role_entry(isid_get, &entry->role)) {
            MVR_CRIT_EXIT();
            return VTSS_RC_ERROR;
        }

        entry->vid = entry->intf.vid;
    } else {
        MVR_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }
    MVR_CRIT_EXIT();

    T_D("mvr_mgmt_get_next_intf_entry consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

static void _mvr_mgmt_revert_intf(mvr_local_interface_t *entry, vtss_mvr_interface_t *mvrif)
{
    ipmc_intf_entry_t               *itfce;
    ipmc_prot_intf_entry_param_t    *param;
    mvr_conf_intf_entry_t           *p;
    ipmc_querier_sm_t               *q;

    if (!entry || !mvrif) {
        return;
    }

    itfce = &mvrif->basic;
    param = &itfce->param;
    p = &entry->intf;
    q = &param->querier;
    entry->version = itfce->ipmc_version;
    p->vid = entry->vid = param->vid;

    memcpy(p->name, mvrif->name, sizeof(p->name));
    p->mode = mvrif->mode;
    p->vtag = mvrif->vtag;
    p->querier4_address = q->QuerierAdrs4;
    p->priority = mvrif->priority;
    p->profile_index = mvrif->profile_index;
    p->protocol_status = itfce->op_state;
    p->querier_status = q->querier_enabled;
    p->compatibility = param->cfg_compatibility;
    p->robustness_variable = q->RobustVari;
    p->query_interval = q->QueryIntvl;
    p->query_response_interval = q->MaxResTime;
    p->last_listener_query_interval = q->LastQryItv;
    p->unsolicited_report_interval = q->UnsolicitR;

    memcpy(entry->role_ports, mvrif->ports, sizeof(entry->role_ports));
    memcpy(entry->vlan_ports, itfce->vlan_ports, sizeof(entry->vlan_ports));
}

/* Get Local MVR VLAN Interface */
vtss_rc mvr_mgmt_local_interface_get(mvr_local_interface_t *entry)
{
    vtss_rc                 retVal;
    vtss_mvr_interface_t    mvrif;
    ipmc_intf_entry_t       *intf;

    if (!entry) {
        return VTSS_RC_ERROR;
    }

    memset(&mvrif, 0x0, sizeof(vtss_mvr_interface_t));
    intf = &mvrif.basic;
    intf->param.vid = entry->vid;
    intf->ipmc_version = entry->version;
    MVR_CRIT_ENTER();
    retVal = vtss_mvr_interface_get(&mvrif);
    MVR_CRIT_EXIT();

    if (retVal == VTSS_OK) {
        _mvr_mgmt_revert_intf(entry, &mvrif);
    }

    return retVal;
}

/* Get-Next Local MVR VLAN Interface */
vtss_rc mvr_mgmt_local_interface_get_next(mvr_local_interface_t *entry)
{
    vtss_rc                 retVal;
    vtss_mvr_interface_t    mvrif;
    ipmc_intf_entry_t       *intf;

    if (!entry) {
        return VTSS_RC_ERROR;
    }

    memset(&mvrif, 0x0, sizeof(vtss_mvr_interface_t));
    intf = &mvrif.basic;
    intf->param.vid = entry->vid;
    intf->ipmc_version = entry->version;
    MVR_CRIT_ENTER();
    retVal = vtss_mvr_interface_get_next(&mvrif);
    MVR_CRIT_EXIT();

    if (retVal == VTSS_OK) {
        _mvr_mgmt_revert_intf(entry, &mvrif);
    }

    return retVal;
}

/* Get MVR VLAN Operational Status */
vtss_rc mvr_mgmt_get_intf_info(vtss_isid_t isid, ipmc_ip_version_t version, ipmc_prot_intf_entry_param_t *entry)
{
    if (!entry) {
        return VTSS_RC_ERROR;
    }

    return mvr_stacking_get_intf_entry(isid, entry, version);
}

/* Get-Next MVR VLAN Operational Status */
vtss_rc mvr_mgmt_get_next_intf_info(vtss_isid_t isid, ipmc_ip_version_t *version, ipmc_prot_intf_entry_param_t *entry)
{
    BOOL                    flag;
    mvr_conf_intf_entry_t   intf;

    if (!version || !entry) {
        return VTSS_RC_ERROR;
    }

    memset(&intf, 0x0, sizeof(mvr_conf_intf_entry_t));
    if (*version == IPMC_IP_VERSION_ALL) {
        intf.vid = entry->vid;
        MVR_CRIT_ENTER();
        flag = mvr_get_next_vlan_conf_entry(&intf);
        MVR_CRIT_EXIT();
        if (!flag) {
            return VTSS_RC_ERROR;
        }

        *version = IPMC_IP_VERSION_IGMP;
        entry->vid = intf.vid;
    } else {
        if (*version == IPMC_IP_VERSION_IGMP) {
            *version = IPMC_IP_VERSION_MLD;
        } else {
            intf.vid = entry->vid;
            MVR_CRIT_ENTER();
            flag = mvr_get_next_vlan_conf_entry(&intf);
            MVR_CRIT_EXIT();
            if (!flag) {
                return VTSS_RC_ERROR;
            }

            *version = IPMC_IP_VERSION_IGMP;
            entry->vid = intf.vid;
        }
    }

    return mvr_stacking_get_intf_entry(isid, entry, *version);
}

vtss_rc mvr_mgmt_get_next_group_srclist(vtss_isid_t isid,
                                        ipmc_ip_version_t ipmc_version,
                                        vtss_vid_t vid,
                                        vtss_ipv6_t *addr,
                                        ipmc_prot_group_srclist_t *group_srclist_entry)
{
    vtss_rc rc = VTSS_OK;

    if (!addr || !group_srclist_entry) {
        return VTSS_RC_ERROR;
    }

    if ((rc = mvr_stacking_get_next_group_srclist_entry(isid, ipmc_version, vid, addr, group_srclist_entry)) == VTSS_OK) {
        if (group_srclist_entry->valid == FALSE) {
            rc = VTSS_RC_ERROR;
        }
    }

    return rc;
}

vtss_rc mvr_mgmt_get_next_intf_group(vtss_isid_t isid,
                                     ipmc_ip_version_t *ipmc_version,
                                     u16 *vid,
                                     ipmc_prot_intf_group_entry_t *intf_group_entry)
{
    vtss_rc rc = VTSS_OK;

    if (!intf_group_entry || !ipmc_version || !vid) {
        return VTSS_RC_ERROR;
    }

    if ((rc = mvr_stacking_get_next_intf_group_entry(isid, *vid, intf_group_entry, *ipmc_version)) == VTSS_OK) {
        if (intf_group_entry->valid == FALSE) {
            rc = VTSS_RC_ERROR;
        } else {
            *vid = intf_group_entry->vid;
            *ipmc_version = intf_group_entry->ipmc_version;
        }
    }

    return rc;
}

vtss_rc mvr_mgmt_set_fast_leave_port(vtss_isid_t isid, mvr_conf_fast_leave_t *fast_leave_port)
{
    port_iter_t             pit;
    u32                     i;
    BOOL                    apply_flag = FALSE;
    mvr_conf_fast_leave_t   *fast_leave;
    cyg_tick_count_t        exe_time_base = cyg_current_time();

    if (!fast_leave_port) {
        return VTSS_RC_ERROR;
    }

    if (!msg_switch_is_master()) {
        return VTSS_OK;
    }

    MVR_CRIT_ENTER();
    fast_leave = &mvr_running.mvr_conf.mvr_conf_fast_leave[ipmc_lib_isid_convert(TRUE, isid)];

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        if (VTSS_PORT_BF_GET(fast_leave_port->ports, i) !=
            VTSS_PORT_BF_GET(fast_leave->ports, i)) {
            apply_flag = TRUE;
            break;
        }
    }

    if (!apply_flag) {
        MVR_CRIT_EXIT();
        return VTSS_OK;
    } else {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            VTSS_PORT_BF_SET(fast_leave->ports,
                             i,
                             VTSS_PORT_BF_GET(fast_leave_port->ports, i));
        }
    }
    MVR_CRIT_EXIT();

    mvr_sm_event_set(MVR_EVENT_CONFBLK_COMMIT);
    (void) mvr_stacking_set_fastleave_port(isid);

    T_D("mvr_mgmt_set_fast_leave_port consumes ID%d:%u ticks", isid, (u32)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc mvr_mgmt_get_fast_leave_port(vtss_isid_t isid, mvr_conf_fast_leave_t *fast_leave_port)
{
    port_iter_t             pit;
    u32                     i;
    mvr_conf_fast_leave_t   *fast_leave;
    cyg_tick_count_t        exe_time_base = cyg_current_time();

    if (!fast_leave_port) {
        return VTSS_RC_ERROR;
    }

    MVR_CRIT_ENTER();
    fast_leave = &mvr_running.mvr_conf.mvr_conf_fast_leave[ipmc_lib_isid_convert(TRUE, isid)];

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        i = pit.iport;

        VTSS_PORT_BF_SET(fast_leave_port->ports,
                         i,
                         VTSS_PORT_BF_GET(fast_leave->ports, i));
    }
    MVR_CRIT_EXIT();

    T_D("mvr_mgmt_get_fast_leave_port consumes ID%d:%u ticks", isid, (u32)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc mvr_mgmt_set_mode(BOOL *mode)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    MVR_CRIT_ENTER();
    if (mvr_running.mvr_conf.mvr_conf_global.mvr_state == *mode) {
        MVR_CRIT_EXIT();
        return VTSS_OK;
    }

    mvr_running.mvr_conf.mvr_conf_global.mvr_state = *mode;

    MVR_CRIT_EXIT();

    mvr_sm_event_set(MVR_EVENT_CONFBLK_COMMIT);
    (void) mvr_stacking_set_mode(VTSS_ISID_GLOBAL);

    T_D("mvr_mgmt_set_mode consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc mvr_mgmt_get_mode(BOOL *mode)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!mode) {
        return VTSS_RC_ERROR;
    }

    MVR_CRIT_ENTER();
    *mode = mvr_running.mvr_conf.mvr_conf_global.mvr_state;
    MVR_CRIT_EXIT();

    T_D("mvr_mgmt_get_mode consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc mvr_mgmt_clear_stat_counter(vtss_isid_t isid, vtss_vid_t vid)
{
    vtss_rc             rc;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    rc = mvr_stacking_clear_statistics(isid, vid);

    T_D("mvr_mgmt_clear_stat_counter consumes ID%d:%u ticks", isid, (u32)(cyg_current_time() - exe_time_base));

    return rc;
}

static BOOL mvr_port_status[VTSS_ISID_END][VTSS_PORT_ARRAY_SIZE], mvrReady = FALSE;
/* Port state callback function  This function is called if a GLOBAL port state change occur.  */
static void mvr_gport_state_change_cb(vtss_isid_t isid, vtss_port_no_t port_no, port_info_t *info)
{
    T_D("enter: Port(%u/Stack:%s)->%s", port_no, info->stack ? "TRUE" : "FALSE", info->link ? "UP" : "DOWN");

    MVR_CRIT_ENTER();
    if (msg_switch_is_master() && !info->stack) {
        if (info->link) {
            mvr_port_status[ipmc_lib_isid_convert(TRUE, isid)][port_no] = TRUE;
        } else {
            mvr_port_status[ipmc_lib_isid_convert(TRUE, isid)][port_no] = FALSE;
        }

        if (ipmc_lib_isid_is_local(isid)) {
            mvr_port_status[VTSS_ISID_LOCAL][port_no] = mvr_port_status[ipmc_lib_isid_convert(TRUE, isid)][port_no];
        }
    }
    MVR_CRIT_EXIT();

    T_D("exit: Port(%u/Stack:%s)->%s", port_no, info->stack ? "TRUE" : "FALSE", info->link ? "UP" : "DOWN");
}

void mvr_port_state_change_cb(vtss_port_no_t port_no, port_info_t *info)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    MVR_CRIT_ENTER();
    vtss_mvr_port_state_change_handle(port_no, info);
    MVR_CRIT_EXIT();

    T_D("mvr_port_state_change_cb consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));
}

void mvr_stp_port_state_change_callback(vtss_common_port_t l2port, vtss_common_stpstate_t new_state)
{
    switch_iter_t                       sit;
    vtss_isid_t                         isid, l2_isid;
    vtss_port_no_t                      iport;
    mvr_msg_buf_t                       buf;
    mvr_msg_stp_port_change_set_req_t   *msg;
    cyg_tick_count_t                    exe_time_base = cyg_current_time();

    /* Only process it when master and forwarding state */
    if (!msg_switch_is_master() || (new_state != VTSS_COMMON_STPSTATE_FORWARDING)) {
        return;
    }

    MVR_CRIT_ENTER();
    if (!mvr_running.mvr_conf.mvr_conf_global.mvr_state) {
        MVR_CRIT_EXIT();
        return;
    }
    MVR_CRIT_EXIT();

    isid = VTSS_ISID_GLOBAL;
    /* Convert l2port to isid and iport */
    if (!l2port2port(l2port, &isid, &iport)) {
#ifdef VTSS_SW_OPTION_AGGR
        vtss_isid_t                 isid_idx;
        vtss_glag_no_t              glag;
        vtss_poag_no_t              poag;
        aggr_mgmt_group_member_t    logical_members[VTSS_ISID_END];
        port_iter_t                 pit;
        BOOL                        aggr_found;

        memset(logical_members, 0x0, sizeof(logical_members));

        if (l2port2glag(l2port, &glag)) {
            aggr_found = FALSE;
            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                isid = sit.isid;
                isid_idx = isid - VTSS_ISID_START;

                if (aggr_mgmt_members_get(isid,
                                          glag,
                                          &logical_members[isid_idx],
                                          FALSE) == VTSS_OK) {
                    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                    while (port_iter_getnext(&pit)) {
                        iport = pit.iport;

                        MVR_CRIT_ENTER();
                        if (!mvr_port_status[ipmc_lib_isid_convert(TRUE, isid)][iport]) {
                            MVR_CRIT_EXIT();
                            continue;
                        }
                        MVR_CRIT_EXIT();

                        if (logical_members[isid_idx].entry.member[iport]) {
                            aggr_found = TRUE;
                            break;
                        }
                    }

                    if (aggr_found) {
                        break;
                    }
                }
            }

            if (!aggr_found) {
                T_W("mvr_stp_port_state_change_callback(%s): fail to translate logical ports\n", l2port2str(l2port));
                return;
            }
        } else if (l2port2poag(l2port, &isid, &poag)) {
            isid_idx = isid - VTSS_ISID_START;
            aggr_found = FALSE;
            if (aggr_mgmt_members_get(isid,
                                      poag,
                                      &logical_members[isid_idx],
                                      FALSE) == VTSS_OK) {
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    iport = pit.iport;

                    MVR_CRIT_ENTER();
                    if (!mvr_port_status[ipmc_lib_isid_convert(TRUE, isid)][iport]) {
                        MVR_CRIT_EXIT();
                        continue;
                    }
                    MVR_CRIT_EXIT();

                    if (logical_members[isid_idx].entry.member[iport]) {
                        aggr_found = TRUE;
                        break;
                    }
                }
            }

            if (!aggr_found) {
                T_W("mvr_stp_port_state_change_callback(%s): fail to translate logical ports\n", l2port2str(l2port));
                return;
            }
        } else {
            T_W("mvr_stp_port_state_change_callback(%s): fail to translate logical ports\n", l2port2str(l2port));
            return;
        }
#else
        T_W("ipmc_stp_port_state_change_callback(%s): fail to translate logical ports\n", l2port2str(l2port));
        return;
#endif /* VTSS_SW_OPTION_AGGR */
    }

    if (!IPMC_LIB_ISID_VALID(isid)) {
        T_W("Failed to get valid ISID(%d) on %s\n", isid, l2port2str(l2port));
        return;
    }
    if (!IPMC_LIB_ISID_CHECK(isid)) {
        T_D("Failed to translate ISID(%d) on %s\n", isid, l2port2str(l2port));
        return;
    }
    l2_isid = isid;

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;
        if (!IPMC_LIB_ISID_PASS(l2_isid, isid)) {
            continue;
        }

        mvr_msg_alloc(MVR_MSG_ID_STP_PORT_CHANGE_REQ, &buf, FALSE, TRUE);
        msg = (mvr_msg_stp_port_change_set_req_t *) buf.msg;

        msg->msg_id = MVR_MSG_ID_STP_PORT_CHANGE_REQ;
        msg->isid = isid;
        msg->port = iport;
        msg->new_state = new_state;
        mvr_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("mvr_stp_port_state_change_callback consumes ID%d:%u ticks", isid, (u32)(cyg_current_time() - exe_time_base));
}

typedef struct {
    BOOL    st;
    u8      chg[VTSS_PORT_BF_SIZE];
    u16     pvid[VTSS_PORT_ARRAY_SIZE];
} mvr_vlan_port_chg_t;
static mvr_vlan_port_chg_t  mvr_vlan_port_chg_buf[VTSS_ISID_END];

static void mvr_vlan_port_change_callback(vtss_isid_t isid, vtss_port_no_t iport, const vlan_port_conf_t *new_conf)
{
    vtss_isid_t isidx;

    /*
        If called back on the master, there is no guarantee that the change has actually taken place in H/W.
        If called back on the local switch, changes are already pushed to H/W.

        When called back on the local switch #isid is VTSS_ISID_LOCAL.
        When called back on the master, #isid is a legal ISID (VTSS_ISID_START ~ VTSS_ISID_END).

        MVR chooses "called back on the master" since VLAN should be treated as centralized.
    */

    if (!mvrReady || !new_conf || !msg_switch_is_master() ||
        ((isidx = ipmc_lib_isid_convert(TRUE, isid)) == VTSS_ISID_UNKNOWN)) {
        return;
    }

    MVR_CRIT_ENTER();
    mvr_vlan_port_chg_buf[isidx].st = TRUE;
    VTSS_PORT_BF_SET(mvr_vlan_port_chg_buf[isidx].chg, iport, TRUE);
    mvr_vlan_port_chg_buf[isidx].pvid[iport] = new_conf->pvid;
    MVR_CRIT_EXIT();

    mvr_sm_event_set(MVR_EVENT_VLAN_PORT_CHG);
}

static void _mvr_vlan_port_change_handler(void)
{
    switch_iter_t           sit;
    vtss_isid_t             isid;
    port_iter_t             pit;
    vtss_port_no_t          iport;
    u8                      i, active_cnt;
    u16                     uvid_chg[VTSS_PORT_ARRAY_SIZE];
    vlan_port_conf_t        vlan_port_conf;
    mvr_conf_intf_role_t    *p;
    mvr_conf_port_role_t    *q;
    vtss_rc                 rc;
    cyg_tick_count_t        exe_time_base;

    if (!msg_switch_is_master()) {
        return;
    }

    exe_time_base = cyg_current_time();

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        /* always Read&Clear mvr_vlan_port_chg_buf[isid] */
        MVR_CRIT_ENTER();
        if (!mvr_vlan_port_chg_buf[isid].st) {
            memset(&mvr_vlan_port_chg_buf[isid], 0x0, sizeof(mvr_vlan_port_chg_t));
            MVR_CRIT_EXIT();
            continue;
        }

        /* prepare the changed uvid buffer for each isid */
        memset(uvid_chg, VTSS_IPMC_VID_NULL, sizeof(uvid_chg));
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            if (VTSS_PORT_BF_GET(mvr_vlan_port_chg_buf[isid].chg, iport)) {
                active_cnt = 0;
                p = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid)];
                for (i = 0; i < MVR_VLAN_MAX; i++) {
                    q = &p->intf[i];
                    if (!q->valid) {
                        continue;
                    }

                    if (q->ports[iport] == MVR_PORT_ROLE_SOURC) {
                        ++active_cnt;
                    }
                }

                if (active_cnt > 1) {
                    uvid_chg[iport] = mvr_vlan_port_chg_buf[isid].pvid[iport];
                }
            }
        }

        memset(&mvr_vlan_port_chg_buf[isid], 0x0, sizeof(mvr_vlan_port_chg_t));
        MVR_CRIT_EXIT();

        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            if ((uvid_chg[iport] == VTSS_IPMC_VID_NULL) ||
                (vlan_mgmt_port_conf_get(isid, iport, &vlan_port_conf, MVR_VLAN_TYPE) != VTSS_RC_OK)) {
                continue;
            }

            if (vlan_port_conf.untagged_vid != uvid_chg[iport]) {
                vlan_port_conf.untagged_vid = uvid_chg[iport];
                if ((rc = vlan_mgmt_port_conf_set(isid, iport, &vlan_port_conf, MVR_VLAN_TYPE)) != VTSS_RC_OK) {
                    T_E("Failure in setting VLAN port conf on isid %u port %u (%s)", isid, iport, error_txt(rc));
                }
            }
        }
    }

    T_D("_mvr_vlan_port_change_handler consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));
}

static void _mvr_vlan_warning_handler(vtss_vid_t vid)
{
    switch_iter_t           sit;
    vtss_isid_t             isid;
    port_iter_t             pit;
    u32                     idx, iport;
    BOOL                    got_warn;
    vlan_mgmt_entry_t       vlan_conf;
    mvr_conf_intf_role_t    *q;
    mvr_conf_port_role_t    *r;

    if (!msg_switch_is_master()) {
        return;
    }

    got_warn = FALSE;
    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;
        vlan_conf.vid = vid;

        if (vlan_mgmt_vlan_get(isid, vid, &vlan_conf, FALSE, VLAN_USER_STATIC) != VTSS_OK) {
            continue;
        }

        /* skip stacking ports */
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;
            if (vlan_conf.ports[iport] == 0) {
                continue;
            }

            /* Warn if this VLAN port is overlapped with same MVR VID source port */
            MVR_CRIT_ENTER();
            q = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid)];
            for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
                r = &q->intf[idx];

                if (r->valid && (r->vid == vid)) {
                    if (r->ports[iport] == MVR_PORT_ROLE_SOURC) {
                        got_warn = TRUE;
                    }

                    break;
                }
            }
            MVR_CRIT_EXIT();

            if (got_warn) {
                break;
            }
        }

        if (got_warn) {
            T_W("Please adjust the management VLAN ports overlapped with MVR source ports!");
            break;
        }
    }
}

void mvr_sm_thread(cyg_addrword_t data)
{
    cyg_handle_t            mvrCounter;
    cyg_flag_value_t        events;
#if MVR_THREAD_EXE_SUPP
    cyg_tick_count_t        last_exe_time;
#endif /* MVR_THREAD_EXE_SUPP */
    cyg_tick_count_t        exe_time_base;
    BOOL                    sync_mgmt_flag, sync_mgmt_done, mvr_link = FALSE;
    switch_iter_t           sit;
    port_iter_t             pit;
    u32                     idx, ticks = 0, ticks_overflow = 0, delay_cnt = 0;
    ipmc_lib_mgmt_info_t    *sys_mgmt;
    vtss_mvr_interface_t    *sm_thread_intf, *sm_thread_intf_bak;

    T_D("enter mvr_sm_thread");

    MVR_CRIT_ENTER();
    cyg_flag_init(&mvr_running.vlan_entry_flags);

    /* Initialize running data structure */
    vtss_mvr_init();
    /* Initialize MSG-RX */
    (void) mvr_stacking_register();
    MVR_CRIT_EXIT();

    /* wait for IP task/stack is ready */
    T_I("mvr_sm_thread init delay start");
    while (!mvr_link && (delay_cnt < MVR_THREAD_START_DELAY_CNT)) {
#if VTSS_SWITCH_STACKABLE
        if (!mvr_in_stacking_ready) {
            VTSS_OS_MSLEEP(MVR_THREAD_SECOND_DEF);
            delay_cnt++;

            continue;
        }
#endif /* VTSS_SWITCH_STACKABLE */

        if (delay_cnt > MVR_THREAD_MIN_DELAY_CNT) {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                if (mvr_port_status[VTSS_ISID_LOCAL][pit.iport]) {
                    mvr_link = TRUE;
                    break;
                }
            }
        }

        if (!mvr_link) {
            VTSS_OS_MSLEEP(MVR_THREAD_SECOND_DEF);
            delay_cnt++;
        }
    }
    T_I("mvr_sm_thread init delay done");

    /* Initialize Periodical Wakeup Timer(Alarm)  */
    cyg_clock_to_counter(cyg_real_time_clock(), &mvrCounter);
    cyg_alarm_create(mvrCounter,
                     mvr_sm_timer_isr,
                     0,
                     &mvr_sm_alarm_handle,
                     &mvr_sm_alarm);
    cyg_alarm_initialize(mvr_sm_alarm_handle, cyg_current_time() + 1, MVR_THREAD_TICK_TIME / ECOS_MSECS_PER_HWTICK);
    cyg_alarm_enable(mvr_sm_alarm_handle);

    if (!IPMC_MEM_SYSTEM_MTAKE(sys_mgmt, sizeof(ipmc_lib_mgmt_info_t))) {
        T_E("MVR sys_mgmt allocation failure!");
    }
    if (!IPMC_MEM_SYSTEM_MTAKE(sm_thread_intf, sizeof(vtss_mvr_interface_t))) {
        T_E("MVR sm_thread_intf allocation failure!");
    }
    sm_thread_intf_bak = sm_thread_intf;

    T_D("mvr_sm_thread start");
    mvrReady = TRUE;
    sync_mgmt_flag = TRUE;

#if MVR_THREAD_EXE_SUPP
    last_exe_time = cyg_current_time();
#endif /* MVR_THREAD_EXE_SUPP */
    while (mvrReady && sm_thread_intf && sys_mgmt) {
        events = cyg_flag_wait(&mvr_sm_events, MVR_EVENT_ANY,
                               CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        if (events & MVR_EVENT_CONFBLK_COMMIT) {
            mvr_conf_blk_t *blk_ptr = NULL;
            u32             blk_size = 0;

            if ((blk_ptr = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_MVR_CONF, &blk_size)) != NULL) {
                MVR_CRIT_ENTER();
                memcpy(&blk_ptr->mvr_conf, &mvr_running.mvr_conf, sizeof(mvr_configuration_t));
                MVR_CRIT_EXIT();

                conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_MVR_CONF);
            }
        }
#endif

        if (events & MVR_EVENT_MASTER_UP) {
            sync_mgmt_flag = TRUE;
        }
        if (events & MVR_EVENT_MASTER_DOWN) {
            sync_mgmt_flag = FALSE;
        }

        if (events & MVR_EVENT_PKT_HANDLER) {
            BOOL    mvr_mode;

            MVR_CRIT_ENTER();
            mvr_mode = mvr_running.mvr_conf.mvr_conf_global.mvr_state;
            MVR_CRIT_EXIT();

            if (mvr_mode) {
                (void) ipmc_lib_packet_register(IPMC_OWNER_MVR, ipmcmvr_rx_packet_callback);
            } else {
                (void) ipmc_lib_packet_unregister(IPMC_OWNER_MVR);
            }
        }

        if (events & MVR_EVENT_SWITCH_DEL) {
            vtss_isid_t isid_del;

            T_D("MVR_EVENT_SWITCH_DEL");

            exe_time_base = cyg_current_time();

            MVR_CRIT_ENTER();

            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                isid_del = sit.isid;

                if (!(mvr_switch_event_value[ipmc_lib_isid_convert(TRUE, isid_del)] & MVR_EVENT_VALUE_SW_DEL)) {
                    continue;
                }

                mvr_switch_event_value[ipmc_lib_isid_convert(TRUE, isid_del)] &= ~MVR_EVENT_VALUE_SW_DEL;
                if (ipmc_lib_isid_is_local(isid_del)) {
                    mvr_switch_event_value[VTSS_ISID_LOCAL] &= ~MVR_EVENT_VALUE_SW_DEL;
                }
            }

            MVR_CRIT_EXIT();

            T_D("MVR_EVENT_SWITCH_DEL consumes %u ticks per time", (u32)(cyg_current_time() - exe_time_base));
        }

        if (events & MVR_EVENT_SWITCH_ADD) {
            vtss_isid_t             isid_add;
            mvr_msg_buf_t           msgbuf;
            mvr_msg_purge_req_t     *purgemsg;
            u32                     isid_set, i;
            mvr_conf_intf_entry_t   *p;
            mvr_conf_port_role_t    *q;
            vtss_rc                 chk_rc;
            ipmc_operation_action_t action;
            ipmc_intf_entry_t       *ifp;
            mvr_mgmt_interface_t    interface;

            T_D("MVR_EVENT_SWITCH_ADD");

            exe_time_base = cyg_current_time();

            sync_mgmt_done = FALSE;
            if (!ipmc_lib_system_mgmt_info_cpy(sys_mgmt)) {
                sync_mgmt_done = TRUE;  /* Not Ready Yet! */
            }

            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                isid_add = sit.isid;

                MVR_CRIT_ENTER();
                isid_set = mvr_switch_event_value[ipmc_lib_isid_convert(TRUE, isid_add)] & MVR_EVENT_VALUE_SW_ADD;
                MVR_CRIT_EXIT();

                if (isid_set == 0) {
                    continue;
                }

                T_D("MVR_EVENT_SWITCH_ADD (%d/%u)",
                    isid_add,
                    mvr_switch_event_value[ipmc_lib_isid_convert(TRUE, isid_add)]);

                _mvr_stacking_reset_port_type(isid_add);
                /* Initial MVR Protocol Database */
                if (ipmc_lib_isid_is_local(isid_add)) {
                    MVR_CRIT_ENTER();
                    (void)vtss_mvr_purge(isid_add, FALSE, FALSE);
                    MVR_CRIT_EXIT();
                } else {
                    mvr_msg_alloc(MVR_MSG_ID_GLOBAL_PURGE_REQ, &msgbuf, FALSE, TRUE);
                    purgemsg = (mvr_msg_purge_req_t *)msgbuf.msg;

                    purgemsg->isid = isid_add;
                    purgemsg->msg_id = MVR_MSG_ID_GLOBAL_PURGE_REQ;
                    mvr_msg_tx(&msgbuf, isid_add, sizeof(*purgemsg));
                }

                (void) mvr_stacking_set_fastleave_port(isid_add);

                for (idx = 0; idx < MVR_VLAN_MAX; idx++) {
                    MVR_CRIT_ENTER();
                    p = &mvr_running.mvr_conf.mvr_conf_vlan[idx];
                    if (!p->valid) {
                        MVR_CRIT_EXIT();
                        continue;
                    }

                    memcpy(&interface.intf, p, sizeof(mvr_conf_intf_entry_t));
                    memset(&interface.role, 0x0, sizeof(mvr_conf_port_role_t));
                    for (i = 0; i < MVR_VLAN_MAX; i++) {
                        q = &mvr_running.mvr_conf.mvr_conf_role[ipmc_lib_isid_convert(TRUE, isid_add)].intf[i];
                        if (q->valid && (q->vid == p->vid)) {
                            memcpy(&interface.role, q, sizeof(mvr_conf_port_role_t));
                            break;
                        }
                    }
                    interface.vid = p->vid;
                    MVR_CRIT_EXIT();

                    ifp = &sm_thread_intf->basic;
                    ifp->ipmc_version = IPMC_IP_VERSION_IGMP;
                    ifp->param.vid = interface.vid;
                    if (ipmc_lib_isid_is_local(isid_add)) {
                        MVR_CRIT_ENTER();
                        chk_rc = vtss_mvr_interface_get(sm_thread_intf);
                        MVR_CRIT_EXIT();
                    } else {
                        chk_rc = VTSS_RC_ERROR;
                    }

                    if (chk_rc != VTSS_OK) {
                        action = IPMC_OP_ADD;
                    } else {
                        action = IPMC_OP_UPD;
                    }

                    MVR_CRIT_ENTER();
                    _mvr_mgmt_transit_intf(isid_add, isid_add, sm_thread_intf, &interface, IPMC_IP_VERSION_IGMP);
                    MVR_CRIT_EXIT();
                    (void) mvr_stacking_set_intf_entry(isid_add, action, sm_thread_intf, IPMC_IP_VERSION_IGMP);
                    MVR_CRIT_ENTER();
                    _mvr_mgmt_transit_intf(isid_add, isid_add, sm_thread_intf, &interface, IPMC_IP_VERSION_MLD);
                    MVR_CRIT_EXIT();
                    (void) mvr_stacking_set_intf_entry(isid_add, action, sm_thread_intf, IPMC_IP_VERSION_MLD);
                }

                (void) mvr_stacking_set_mode(isid_add);

                if (!sync_mgmt_done) {
                    (void) mvr_stacking_sync_mgmt_conf(isid_add, sys_mgmt);
                }

                MVR_CRIT_ENTER();
                mvr_switch_event_value[ipmc_lib_isid_convert(TRUE, isid_add)] &= ~MVR_EVENT_VALUE_SW_ADD;
                if (ipmc_lib_isid_is_local(isid_add)) {
                    mvr_switch_event_value[VTSS_ISID_LOCAL] &= ~MVR_EVENT_VALUE_SW_ADD;
                }
                MVR_CRIT_EXIT();
            }

            T_D("MVR_EVENT_SWITCH_ADD consumes %u ticks per time", (u32)(cyg_current_time() - exe_time_base));
        }

        sync_mgmt_done = FALSE;
        if (events & MVR_EVENT_SM_TIME_WAKEUP) {
            u8  exe_round = 1;
#if MVR_THREAD_EXE_SUPP
            u32 supply_ticks = 0, diff_exe_time = ipmc_lib_diff_u32_wrap_around((u32)last_exe_time, (u32)cyg_current_time());

            if ((diff_exe_time * ECOS_MSECS_PER_HWTICK) > (2 * MVR_THREAD_TICK_TIME)) {
                supply_ticks = ((diff_exe_time * ECOS_MSECS_PER_HWTICK) / MVR_THREAD_TICK_TIME) - 1;
            }

            if (supply_ticks > 0) {
                exe_round += ipmc_lib_calc_thread_tick(&ticks, supply_ticks, MVR_THREAD_TIME_UNIT_BASE, &ticks_overflow);
            }

            last_exe_time = cyg_current_time();
#endif /* MVR_THREAD_EXE_SUPP */

            if (msg_switch_is_master() && ipmc_lib_system_mgmt_info_chg(sys_mgmt)) {
                sync_mgmt_done = (mvr_stacking_sync_mgmt_conf(VTSS_ISID_GLOBAL, sys_mgmt) == VTSS_OK);
            }

            if (!(ticks % MVR_THREAD_TIME_UNIT_BASE)) {
                for (; exe_round > 0; exe_round--) {
                    if (msg_switch_is_master() && sync_mgmt_flag) {
                        if (!sync_mgmt_done) {
                            if (ipmc_lib_system_mgmt_info_cpy(sys_mgmt)) {
                                sync_mgmt_flag = (mvr_stacking_sync_mgmt_conf(VTSS_ISID_GLOBAL, sys_mgmt) != VTSS_OK);
                            }
                        } else {
                            sync_mgmt_flag = FALSE;
                        }
                    }

                    MVR_CRIT_ENTER();
                    T_N("mvr_sm_thread: %dMSEC-TICKS(%u | %u)", MVR_THREAD_TICK_TIME, ticks_overflow, ticks);
                    exe_time_base = cyg_current_time();

                    vtss_mvr_tick_gen();

                    if (mvr_running.mvr_conf.mvr_conf_global.mvr_state) {
                        memset(sm_thread_intf, 0x0, sizeof(vtss_mvr_interface_t));
                        while (vtss_mvr_interface_get_next(sm_thread_intf) == VTSS_OK) {
                            vtss_mvr_tick_intf_tmr(&sm_thread_intf->basic);
                            if (vtss_mvr_interface_set(IPMC_OP_SET, sm_thread_intf) != VTSS_OK) {
                                T_D("vtss_mvr_interface_set(IPMC_OP_SET) Failed!");
                            }
                        }

                        vtss_mvr_tick_intf_rxmt();
                        vtss_mvr_tick_group_tmr();
                    }

                    T_N("MVR_EVENT_SM_TIME_WAKEUP consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));
                    MVR_CRIT_EXIT();
                }
            }

            if (!(ticks % MVR_THREAD_TIME_MISC_BASE)) {
                BOOL                next;
                ipmc_mgmt_ipif_t    sys_intf;

                memset(sm_thread_intf, 0x0, sizeof(vtss_mvr_interface_t));
                MVR_CRIT_ENTER();
                next = (vtss_mvr_interface_get_next(sm_thread_intf) == VTSS_OK);
                MVR_CRIT_EXIT();
                while (next) {
                    if (ipmc_lib_get_system_mgmt_intf(&sm_thread_intf->basic, &sys_intf)) {
                        _mvr_vlan_warning_handler(ipmc_mgmt_intf_vidx(&sys_intf));
                    }

                    MVR_CRIT_ENTER();
                    next = (vtss_mvr_interface_get_next(sm_thread_intf) == VTSS_OK);
                    MVR_CRIT_EXIT();
                }
            }

            (void) ipmc_lib_calc_thread_tick(&ticks, 1, MVR_THREAD_TIME_UNIT_BASE, &ticks_overflow);
        }

        if (events & MVR_EVENT_VLAN_PORT_CHG) {
            _mvr_vlan_port_change_handler();
        }

        sm_thread_intf = sm_thread_intf_bak;
    }

    IPMC_MEM_SYSTEM_MGIVE(sm_thread_intf_bak);
    IPMC_MEM_SYSTEM_MGIVE(sys_mgmt);
    mvrReady = FALSE;
    T_W("exit mvr_sm_thread");
}

#if 0 /* etliang */
static cyg_handle_t                 mvr_db_thread_handle;
static cyg_thread                   mvr_db_thread_block;
static char                         mvr_db_thread_stack[IPMC_THREAD_STACK_SIZE];

/*lint -esym(459, mvr_db_events) */
static cyg_flag_t   mvr_db_events;
static cyg_alarm    mvr_db_alarm;
static cyg_handle_t mvr_db_alarm_handle;

static void mvr_db_event_set(cyg_flag_value_t flag)
{
    cyg_flag_setbits(&mvr_db_events, flag);
}

static void mvr_db_timer_isr(cyg_handle_t alarm, cyg_addrword_t data)
{
    if (alarm || data) { /* avoid warning */
    }

    mvr_db_event_set(MVR_EVENT_DB_TIME_WAKEUP);
}

static vtss_mvr_interface_t db_thread_intf;
void mvr_db_thread(cyg_addrword_t data)
{
    cyg_handle_t        mvrCounter;
    cyg_flag_value_t    events;
#if MVR_THREAD_EXE_SUPP
    cyg_tick_count_t    last_exe_time;
#endif /* #if MVR_THREAD_EXE_SUPP */
    cyg_tick_count_t    exe_time_base;
    u32                 ticks = 0, ticks_overflow = 0;

    T_D("enter mvr_db_thread");

    /* wait for MVR is ready */
    while (!mvrReady) {
        VTSS_OS_MSLEEP(MVR_THREAD_SECOND_DEF);
    }

    /* Initialize Periodical Wakeup Timer(Alarm)  */
    cyg_clock_to_counter(cyg_real_time_clock(), &mvrCounter);
    cyg_alarm_create(mvrCounter,
                     mvr_db_timer_isr,
                     0,
                     &mvr_db_alarm_handle,
                     &mvr_db_alarm);
    cyg_alarm_initialize(mvr_db_alarm_handle, cyg_current_time() + 1, MVR_THREAD_TICK_TIME / ECOS_MSECS_PER_HWTICK);
    cyg_alarm_enable(mvr_db_alarm_handle);

    T_D("mvr_db_thread start");

#if MVR_THREAD_EXE_SUPP
    last_exe_time = cyg_current_time();
#endif /* MVR_THREAD_EXE_SUPP */
    while (mvrReady) {
        events = cyg_flag_wait(&mvr_db_events, MVR_EVENT_ANY,
                               CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);

        if (events & MVR_EVENT_DB_TIME_WAKEUP) {
            u8  exe_round = 1;
#if MVR_THREAD_EXE_SUPP
            u32 supply_ticks = 0, diff_exe_time = ipmc_lib_diff_u32_wrap_around((u32)last_exe_time, (u32)cyg_current_time());

            if ((diff_exe_time * ECOS_MSECS_PER_HWTICK) > (2 * MVR_THREAD_TICK_TIME)) {
                supply_ticks = ((diff_exe_time * ECOS_MSECS_PER_HWTICK) / MVR_THREAD_TICK_TIME) - 1;
            }

            if (supply_ticks > 0) {
                exe_round += ipmc_lib_calc_thread_tick(&ticks, supply_ticks, MVR_THREAD_TIME_UNIT_BASE, &ticks_overflow);
            }

            last_exe_time = cyg_current_time();
#endif /* MVR_THREAD_EXE_SUPP */
            if (!(ticks % MVR_THREAD_TIME_UNIT_BASE)) {
                for (; exe_round > 0; exe_round--) {
                    MVR_CRIT_ENTER();
                    T_N("mvr_db_thread: %dMSEC-TICKS(%lu | %lu)", MVR_THREAD_TICK_TIME, ticks_overflow, ticks);
                    exe_time_base = cyg_current_time();

                    memset(&db_thread_intf, 0x0, sizeof(vtss_mvr_interface_t));
                    while (vtss_mvr_interface_get_next(&db_thread_intf) == VTSS_OK) {
                        vtss_mvr_tick_intf_grp_tmr(&db_thread_intf.basic);
                        if (vtss_mvr_interface_set(IPMC_OP_SET, &db_thread_intf) != VTSS_OK) {
                            T_D("vtss_mvr_interface_set(IPMC_OP_SET) Failed!");
                        }
                    }

                    T_N("MVR_EVENT_DB_TIME_WAKEUP consumes %lu ticks", (u32)(cyg_current_time() - exe_time_base));
                    MVR_CRIT_EXIT();
                }
            }

            (void) ipmc_lib_calc_thread_tick(&ticks, 1, MVR_THREAD_TIME_UNIT_BASE, &ticks_overflow);
        }
    }

    T_W("exit mvr_db_thread");
}
#endif /* etliang */

vtss_rc mvr_init(vtss_init_data_t *data)
{
    mvr_msg_id_t        idx;
    vtss_isid_t         isid = data->isid;
    BOOL                run_time_action, mvr_mode;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    switch ( data->cmd ) {
    case INIT_CMD_INIT:
#if VTSS_TRACE_ENABLED
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&mvr_trace_reg, mvr_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&mvr_trace_reg);
#endif /* VTSS_TRACE_ENABLED */

        for (idx = MVR_MSG_ID_GLOBAL_SET_REQ; idx < MVR_MSG_MAX_ID; idx++) {
            switch ( idx ) {
            case MVR_MSG_ID_GLOBAL_SET_REQ:
                mvr_running.msize[idx] = sizeof(mvr_msg_global_set_req_t);
                break;
            case MVR_MSG_ID_SYS_MGMT_SET_REQ:
                mvr_running.msize[idx] = sizeof(mvr_msg_sys_mgmt_set_req_t);
                break;
            case MVR_MSG_ID_GLOBAL_PURGE_REQ:
                mvr_running.msize[idx] = sizeof(mvr_msg_purge_req_t);
                break;
            case MVR_MSG_ID_VLAN_ENTRY_GET_REQ:
            case MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ:
            case MVR_MSG_ID_GROUP_SRCLIST_WALK_REQ:
                mvr_running.msize[idx] = sizeof(mvr_msg_vlan_entry_get_req_t);
                break;
            case MVR_MSG_ID_VLAN_ENTRY_GET_REP:
                mvr_running.msize[idx] = sizeof(mvr_msg_vlan_entry_get_rep_t);
                break;
            case MVR_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP:
                mvr_running.msize[idx] = sizeof(mvr_msg_vlan_group_entry_get_rep_t);
                break;
            case MVR_MSG_ID_GROUP_SRCLIST_WALK_REP:
                mvr_running.msize[idx] = sizeof(mvr_msg_group_srclist_get_rep_t);
                break;
            case MVR_MSG_ID_PORT_FAST_LEAVE_SET_REQ:
                mvr_running.msize[idx] = sizeof(mvr_msg_port_fast_leave_set_req_t);
                break;
            case MVR_MSG_ID_STAT_COUNTER_CLEAR_REQ:
                mvr_running.msize[idx] = sizeof(mvr_msg_stat_counter_clear_req_t);
                break;
            case MVR_MSG_ID_STP_PORT_CHANGE_REQ:
                mvr_running.msize[idx] = sizeof(mvr_msg_stp_port_change_set_req_t);
                break;
            case MVR_MSG_ID_VLAN_ENTRY_SET_REQ:
            default:
                /* Give the MAX */
                mvr_running.msize[idx] = sizeof(mvr_msg_vlan_set_req_t);
                break;
            }

            mvr_running.msg[idx] = VTSS_MALLOC(mvr_running.msize[idx]);
            if (mvr_running.msg[idx] == NULL) {
                T_W("MVR_ASSERT(INIT_CMD_INIT)");
                for (;;) {}
            }
        }

        /* Create semaphore for critical regions */
        critd_init(&mvr_running.crit, "MVR_global.crit", VTSS_MODULE_ID_MVR, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        MVR_CRIT_EXIT();

        critd_init(&mvr_running.get_crit, "MVR_global.get_crit", VTSS_MODULE_ID_MVR, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        MVR_GET_CRIT_EXIT();

        (void) ipmc_lib_common_init();
        /* Initialized RX semaphore */
        (void) ipmc_lib_packet_init();

#ifdef VTSS_SW_OPTION_VCLI
        mvr_cli_req_init();
#endif /* VTSS_SW_OPTION_VCLI */

#ifdef VTSS_SW_OPTION_ICFG
        if (ipmc_mvr_icfg_init() != VTSS_OK) {
            T_E("ipmc_mvr_icfg_init failed!");
        }
#endif /* VTSS_SW_OPTION_ICFG */

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&mvr_running.request.sem, 1);
        VTSS_OS_SEM_CREATE(&mvr_running.reply.sem, 1);

        memset(mvr_switch_event_value, 0x0, sizeof(mvr_switch_event_value));

        /* Initialize MVR-EVENT groups */
        cyg_flag_init(&mvr_sm_events);
#if 0 /* etliang */
        cyg_flag_init(&mvr_db_events);
#endif /* etliang */

        memset(mvr_port_status, 0x0, sizeof(mvr_port_status));

        memset(mvr_vlan_port_chg_buf, 0x0, sizeof(mvr_vlan_port_chg_buf));
        /* Register for VLAN port configuration changes */
        vlan_port_conf_change_register(VTSS_MODULE_ID_MVR, mvr_vlan_port_change_callback, TRUE);

        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          mvr_sm_thread,
                          0,
                          "IPMC_MVR",
                          mvr_sm_thread_stack,
                          sizeof(mvr_sm_thread_stack),
                          &mvr_sm_thread_handle,
                          &mvr_sm_thread_block);
        cyg_thread_resume(mvr_sm_thread_handle);

        T_I("MVR-INIT_CMD_INIT consumes ID%d:%u ticks", isid, (u32)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_START:
        T_I("START: ISID->%d", isid);

        (void) port_change_register(VTSS_MODULE_ID_MVR, mvr_port_state_change_cb);
        (void) l2_stp_state_change_register(mvr_stp_port_state_change_callback);
        /* Register for Port GLOBAL change callback */
        (void) port_global_change_register(VTSS_MODULE_ID_MVR, mvr_gport_state_change_cb);

        T_D("MVR-INIT_CMD_START consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_CONF_DEF:
        T_I("CONF_DEF: ISID->%d", isid);

        MVR_CRIT_ENTER();
        run_time_action = mvrReady;
        mvr_mode = mvr_running.mvr_conf.mvr_conf_global.mvr_state;
        MVR_CRIT_EXIT();

        if (msg_switch_is_master()) {
            if (run_time_action) {
                switch_iter_t           sit;
                mvr_msg_buf_t           msgbuf;
                mvr_msg_purge_req_t     *purgemsg;
                BOOL                    flag;
                vlan_mgmt_entry_t       vlan_entry;
                mvr_conf_intf_entry_t   intf;

                memset(&intf, 0x0, sizeof(mvr_conf_intf_entry_t));
                MVR_CRIT_ENTER();
                flag = mvr_get_next_vlan_conf_entry(&intf);
                MVR_CRIT_EXIT();
                while (flag) {
                    memset(&vlan_entry, 0x0, sizeof(vlan_mgmt_entry_t));
                    if (vlan_mgmt_vlan_get(VTSS_ISID_GLOBAL, intf.vid, &vlan_entry, FALSE, VLAN_USER_ALL) == VTSS_OK) {
                        (void)vlan_mgmt_vlan_del(VTSS_ISID_GLOBAL, intf.vid, MVR_VLAN_TYPE);
                    }

                    MVR_CRIT_ENTER();
                    flag = mvr_get_next_vlan_conf_entry(&intf);
                    MVR_CRIT_EXIT();
                }

                if ((isid != VTSS_ISID_GLOBAL) && !ipmc_lib_isid_is_local(isid)) {
                    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
                    while (switch_iter_getnext(&sit)) {
                        if (sit.isid != isid) {
                            continue;
                        }

                        _mvr_stacking_reset_port_type(isid);

                        /* Initial MVR Protocol Database */
                        mvr_msg_alloc(MVR_MSG_ID_GLOBAL_PURGE_REQ, &msgbuf, FALSE, TRUE);
                        purgemsg = (mvr_msg_purge_req_t *)msgbuf.msg;

                        purgemsg->isid = isid;
                        purgemsg->msg_id = MVR_MSG_ID_GLOBAL_PURGE_REQ;
                        mvr_msg_tx(&msgbuf, isid, sizeof(*purgemsg));
                    }
                }

                if (ipmc_lib_isid_is_local(isid)) {
                    _mvr_stacking_reset_port_type(isid);
                }

                if (isid == VTSS_ISID_GLOBAL) {
                    MVR_CRIT_ENTER();
                    (void)vtss_mvr_purge(VTSS_ISID_LOCAL, FALSE, FALSE);
                    MVR_CRIT_EXIT();

                    /* Reset stack configuration */
                    (void) mvr_conf_read(TRUE);

                    if (mvr_mode) {
                        mvr_sm_event_set(MVR_EVENT_PKT_HANDLER);
                    }
                }
            } else {
                /* Reset stack configuration */
                (void) mvr_conf_read(TRUE);

                if (mvr_mode) {
                    mvr_sm_event_set(MVR_EVENT_PKT_HANDLER);
                }
            }
        } else {
            /* Reset stack configuration */
            (void) mvr_conf_read(TRUE);

            if (mvr_mode) {
                mvr_sm_event_set(MVR_EVENT_PKT_HANDLER);
            }
        }

        T_D("MVR-INIT_CMD_CONF_DEF consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_MASTER_UP:
        T_I("MASTER_UP: ISID->%d", isid);

        /* Read configuration */
        (void) mvr_conf_read(FALSE);
        mvr_sm_event_set(MVR_EVENT_MASTER_UP);

        T_D("MVR-INIT_CMD_MASTER_UP consumes ID%d:%u ticks", isid, (u32)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_MASTER_DOWN:
        T_I("MASTER_DOWN: ISID->%d", isid);

#if VTSS_SWITCH_STACKABLE
        mvr_in_stacking_ready = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */
        mvr_sm_event_set(MVR_EVENT_MASTER_DOWN);

        T_D("MVR-INIT_CMD_MASTER_DOWN consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_SWITCH_ADD:
        T_I("SWITCH_ADD: ISID->%d", isid);

        MVR_CRIT_ENTER();
        mvr_switch_event_value[ipmc_lib_isid_convert(TRUE, isid)] |= MVR_EVENT_VALUE_SW_ADD;
        if (ipmc_lib_isid_is_local(isid)) {
            mvr_switch_event_value[VTSS_ISID_LOCAL] |= MVR_EVENT_VALUE_SW_ADD;
        }
        MVR_CRIT_EXIT();

        mvr_sm_event_set(MVR_EVENT_SWITCH_ADD);

#if VTSS_SWITCH_STACKABLE
        mvr_in_stacking_ready = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */

        T_D("MVR-INIT_CMD_SWITCH_ADD consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_SWITCH_DEL:
        T_I("SWITCH_DEL: ISID->%d", isid);

        MVR_CRIT_ENTER();
        mvr_switch_event_value[ipmc_lib_isid_convert(TRUE, isid)] |= MVR_EVENT_VALUE_SW_DEL;
        if (ipmc_lib_isid_is_local(isid)) {
            mvr_switch_event_value[VTSS_ISID_LOCAL] |= MVR_EVENT_VALUE_SW_DEL;
        }
        MVR_CRIT_EXIT();

        mvr_sm_event_set(MVR_EVENT_SWITCH_DEL);

        T_D("MVR-INIT_CMD_SWITCH_DEL consumes %u ticks", (u32)(cyg_current_time() - exe_time_base));

        break;
    default:
        break;
    }

    return 0;
}
