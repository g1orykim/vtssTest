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

#include "ipmc.h"

#ifdef VTSS_SW_OPTION_VCLI
#include "ipmc_cli.h"
#endif /* VTSS_SW_OPTION_VCLI */
#include "misc_api.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "ipmc_snp_icfg.h"
#endif /* VTSS_SW_OPTION_ICFG */

#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_IPMC
#define VTSS_ALLOC_MODULE_ID        VTSS_MODULE_ID_IPMC
#define IPMC_THREAD_EXE_SUPP        0

static struct {
    critd_t                         crit;
    critd_t                         get_crit;

    ipmc_configuration_t            ipv6_conf;                              /* Current configuration for MLD */
    ipmc_configuration_t            ipv4_conf;                              /* Current configuration for IGMP */

    cyg_flag_t                      vlan_entry_flags;
    ipmc_prot_intf_entry_param_t    interface_entry;
    ipmc_prot_intf_group_entry_t    intf_group_entry;
    ipmc_prot_group_srclist_t       group_srclist_entry;

    cyg_flag_t                      dynamic_router_ports_getting_flags;
    BOOL                            dynamic_router_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];

    /* Request semaphore */
    struct {
        vtss_os_sem_t   sem;
    } request;

    /* Reply semaphore */
    struct {
        vtss_os_sem_t   sem;
    } reply;

    /* MSG Buffer */
    u8                  *msg[IPMC_MSG_MAX_ID];
    u32                 msize[IPMC_MSG_MAX_ID];
} ipmc_global;

/* Thread variables */
/* IPMC Timer Thread */
static cyg_handle_t ipmc_sm_thread_handle;
static cyg_thread   ipmc_sm_thread_block;
static char         ipmc_sm_thread_stack[IPMC_THREAD_STACK_SIZE];

#if VTSS_TRACE_ENABLED

static vtss_trace_reg_t snp_trace_reg = {
    .module_id = VTSS_MODULE_ID_IPMC,
    .name      = "IPMC",
    .descr     = "IPMC Snooping"
};

static vtss_trace_grp_t snp_trace_grps[TRACE_GRP_CNT] = {
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

#define IPMC_CRIT_ENTER() critd_enter(&ipmc_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define IPMC_CRIT_EXIT()  critd_exit(&ipmc_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)

#define IPMC_GET_CRIT_ENTER() critd_enter(&ipmc_global.get_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)
#define IPMC_GET_CRIT_EXIT()  critd_exit(&ipmc_global.get_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  __FILE__, __LINE__)

#else

#define IPMC_CRIT_ENTER() critd_enter(&ipmc_global.crit)
#define IPMC_CRIT_EXIT()  critd_exit(&ipmc_global.crit)

#define IPMC_GET_CRIT_ENTER() critd_enter(&ipmc_global.get_crit)
#define IPMC_GET_CRIT_EXIT()  critd_exit(&ipmc_global.get_crit)

#endif /* VTSS_TRACE_ENABLED */


#ifdef VTSS_SW_OPTION_PACKET
static vtss_packet_rx_info_t  ipmc_snp_rx_info;
#endif /* VTSS_SW_OPTION_PACKET */
static u8                   ipmcsnp_frame[IPMC_LIB_PKT_BUF_SZ];
static ipmc_lib_log_t       ipmcsnp_rx_packet_log;
static i8                   ipmcsnp_rx_pktlog_msg[IPMC_LIB_PKT_LOG_MSG_SZ_VAL];
static BOOL ipmcsnp_rx_packet_callback(void *contxt, const u8 *const frm, const vtss_packet_rx_info_t *const rx_info, BOOL next_ipmc)
{
    vtss_rc             rc = VTSS_OK;
    ipmc_ip_eth_hdr     *etherHdr;
    ipmc_ip_version_t   version = IPMC_IP_VERSION_INIT;
    ipmc_port_bfs_t     ipmcsnp_fwd_map;
    BOOL                ingress_chk, ingress_member[VTSS_PORT_ARRAY_SIZE];
    vtss_vid_t          ingress_vid;
    vtss_glag_no_t      glag_no;
    u32                 idx, local_port_cnt;

    T_D("%s Frame with length %d received on port %d with vid %d",
        ipmc_lib_frm_tagtype_txt(rx_info->tag_type, IPMC_TXT_CASE_CAPITAL),
        rx_info->length, rx_info->port_no, rx_info->tag.vid);
    T_D_HEX(frm, rx_info->length);

    glag_no = rx_info->glag_no;
    T_D("With glag_no=%u in SNP", glag_no);

    memset(&ipmcsnp_fwd_map, 0x0, sizeof(ipmc_port_bfs_t));
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
    idx = PORT_NO_STACK_0;
    ingress_member[idx] = TRUE;
    idx = PORT_NO_STACK_1;
    ingress_member[idx] = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */

    local_port_cnt = ipmc_lib_get_system_local_port_cnt();

#ifdef VTSS_SW_OPTION_PACKET
    if (ingress_chk) {
        char    snpBufPort[MGMT_PORT_BUF_SIZE];

        IPMC_CRIT_ENTER();
        memcpy(&ipmc_snp_rx_info, rx_info, sizeof(ipmc_snp_rx_info));
        ipmc_lib_packet_strip_vtag(frm, &ipmcsnp_frame[0], rx_info->tag_type, &ipmc_snp_rx_info);

        rc = RX_ipmcsnp(version, contxt, &ipmcsnp_frame[0], &ipmc_snp_rx_info, &ipmcsnp_fwd_map);
        IPMC_CRIT_EXIT();

        for (idx = 0; idx < local_port_cnt; idx++) {
            if (idx != rx_info->port_no) {
                ingress_member[idx] &= VTSS_PORT_BF_GET(ipmcsnp_fwd_map.member_ports, idx);
            } else {
                ingress_member[idx] = FALSE;
            }

            VTSS_PORT_BF_SET(ipmcsnp_fwd_map.member_ports,
                             idx,
                             ingress_member[idx]);
        }
        T_D("ipmcsnp_fwd_map(RC=%d) is %s", rc, mgmt_iport_list2txt(ingress_member, snpBufPort));
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
        IPMC_CRIT_ENTER();
        vtss_ipmc_calculate_dst_ports(ingress_vid, rx_info->port_no, &ipmcsnp_fwd_map, version);

        memset(&ipmcsnp_rx_packet_log, 0x0, sizeof(ipmc_lib_log_t));
        ipmcsnp_rx_packet_log.vid = ingress_vid;
        ipmcsnp_rx_packet_log.port = rx_info->port_no + 1;
        ipmcsnp_rx_packet_log.version = version;
        ipmcsnp_rx_packet_log.event.message.data = ipmcsnp_rx_pktlog_msg;

        memset(ipmcsnp_rx_pktlog_msg, 0x0, sizeof(ipmcsnp_rx_pktlog_msg));
        if (rc == IPMC_ERROR_PKT_CONTENT) {
            sprintf(ipmcsnp_rx_pktlog_msg, "(IPMC_ERROR_PKT_CONTENT)->RX frame with bad content!");
        }
        if (rc == IPMC_ERROR_PKT_RESERVED) {
            sprintf(ipmcsnp_rx_pktlog_msg, "(IPMC_ERROR_PKT_RESERVED)->RX reserved group address registration!");
        }
        if (rc == IPMC_ERROR_PKT_ADDRESS) {
            sprintf(ipmcsnp_rx_pktlog_msg, "(IPMC_ERROR_PKT_ADDRESS)->RX frame with bad address!");
        }
        if (rc == IPMC_ERROR_PKT_VERSION) {
            sprintf(ipmcsnp_rx_pktlog_msg, "(IPMC_ERROR_PKT_ADDRESS)->RX frame with bad version!");
        }
        IPMC_LIB_LOG_MSG(&ipmcsnp_rx_packet_log, IPMC_SEVERITY_Warning);
        IPMC_CRIT_EXIT();

        break;
    default:
#ifdef VTSS_FEATURE_AGGR_GLAG
        if (glag_no != VTSS_GLAG_NO_NONE) {
            memset(ingress_member, 0x0, sizeof(ingress_member));
            // filter out all ports with the same glag
            if (vtss_aggr_glag_members_get(NULL, glag_no, ingress_member) == VTSS_OK) {
                for (idx = 0; idx < local_port_cnt; idx++) {
                    if (ingress_member[idx] == TRUE) {
                        VTSS_PORT_BF_SET(ipmcsnp_fwd_map.member_ports, idx, FALSE);
                        if (port_isid_port_no_is_stack(VTSS_ISID_LOCAL, rx_info->port_no)) {
                            IPMC_CRIT_ENTER();
                            vtss_ipmc_process_glag(idx, ingress_vid, frm, rx_info->length, IPMC_IP_VERSION_IGMP);
                            vtss_ipmc_process_glag(idx, ingress_vid, frm, rx_info->length, IPMC_IP_VERSION_MLD);
                            IPMC_CRIT_EXIT();
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
        idx = PORT_NO_STACK_0;
        VTSS_PORT_BF_SET(ipmcsnp_fwd_map.member_ports, idx, FALSE);
        idx = PORT_NO_STACK_1;
        VTSS_PORT_BF_SET(ipmcsnp_fwd_map.member_ports, idx, FALSE);
    }
#endif /* VTSS_SWITCH_STACKABLE */

    if (IPMC_LIB_BFS_HAS_MEMBER(ipmcsnp_fwd_map.member_ports)) {
        BOOL    src_fast_leave;

        IPMC_CRIT_ENTER();
        src_fast_leave = vtss_ipmc_get_static_fast_leave_ports(rx_info->port_no, version);
        IPMC_CRIT_EXIT();

        (void) ipmc_lib_packet_tx(&ipmcsnp_fwd_map,
                                  FALSE,
                                  src_fast_leave,
                                  rx_info->port_no,
                                  IPMC_PKT_SRC_SNP,
                                  rx_info->tag.vid,
                                  rx_info->tag.dei,
                                  rx_info->tag.pcp,
                                  glag_no,
                                  frm,
                                  rx_info->length);
    }

    if (rc != VTSS_OK) {
        return FALSE;
    } else {
        /* SNP has done the processing; Others shouldn't take care of it. */
        return TRUE;
    }
}

#if VTSS_SWITCH_STACKABLE
static BOOL ipmc_in_stacking_ready = FALSE;
#endif /* VTSS_SWITCH_STACKABLE */

/* DEBUG */
BOOL ipmc_debug_pkt_send(ipmc_ctrl_pkt_t type,
                         ipmc_ip_version_t version,
                         vtss_vid_t vid,
                         vtss_ipv6_t *grp,
                         u8 iport,
                         u8 cnt,
                         BOOL untag)
{
    ipmc_intf_entry_t   *entry, intf;
    u8                  idx;
    BOOL                retVal;
    char                buf[40];

    if (!grp) {
        return FALSE;
    }

    IPMC_CRIT_ENTER();
    entry = vtss_ipmc_get_intf_entry(vid, version);
    if (entry == NULL) {
        IPMC_CRIT_EXIT();
        return FALSE;
    }
    memcpy(&intf, entry, sizeof(ipmc_intf_entry_t));
    IPMC_CRIT_EXIT();

    memset(intf.vlan_ports, 0x0, sizeof(intf.vlan_ports));
    VTSS_PORT_BF_SET(intf.vlan_ports, iport, TRUE);
    retVal = TRUE;
    for (idx = 0; idx < cnt; idx++) {
        memset(buf, 0x0, sizeof(buf));
        T_D("vtss_ipmc_debug_pkt_tx(%u/%d, %d, %s, %u, %s)",
            intf.param.vid, intf.ipmc_version,
            type,
            misc_ipv6_txt(grp, buf),
            iport,
            untag ? "TRUE" : "FALSE");
        IPMC_CRIT_ENTER();
        if (!vtss_ipmc_debug_pkt_tx(&intf, type, grp, iport, untag)) {
            retVal = FALSE;
        }
        IPMC_CRIT_EXIT();
    }

    return retVal;
}

BOOL ipmc_debug_mem_get_info(ipmc_mem_info_t *info)
{
    ipmc_mem_t  idx;

    if (!info) {
        return FALSE;
    }

    info->mem_current_cnt = ipmc_lib_mem_debug_get_cnt();
    for (idx = 0; idx < IPMC_MEM_TYPE_MAX; idx++) {
        ipmc_lib_mem_debug_get_info(idx, &info->mem_pool_info[idx]);
    }

    return TRUE;
}

#define IPMC_THREAD_SECOND_DEF      1000        /* 1000 msec = 1 second */
#define IPMC_THREAD_GC_TIME         11000       /* 11000 msec = 11 second */
#define IPMC_THREAD_TICK_TIME       1000        /* 1000 msec */
#define IPMC_THREAD_TIME_UNIT_BASE  (IPMC_THREAD_SECOND_DEF / IPMC_THREAD_TICK_TIME)
#define IPMC_THREAD_GCT_UNIT_BASE   (IPMC_THREAD_GC_TIME / IPMC_THREAD_TICK_TIME)
#define IPMC_THREAD_START_DELAY_CNT 15
#define IPMC_THREAD_MIN_DELAY_CNT   3

#define IPMC_EVENT_ANY              0xFFFFFFFF  // Any possible bit...
#define IPMC_EVENT_SM_TIME_WAKEUP   0x00000001
#define IPMC_EVENT_SWITCH_ADD       0x00000002
#define IPMC_EVENT_SWITCH_DEL       0x00000004
#define IPMC_EVENT_CONFBLK_COMMIT   0x00000008
#define IPMC_EVENT_DB_TIME_WAKEUP   0x00000010
#define IPMC_EVENT_PKT4_HANDLER     0x00000100
#define IPMC_EVENT_PKT6_HANDLER     0x00001000
#define IPMC_EVENT_PROXY_LOCAL      0x00010000
#define IPMC_EVENT_GARBAGE_COLLECT4 0x00100000
#define IPMC_EVENT_GARBAGE_COLLECT6 0x00200000
#define IPMC_EVENT_MASTER_UP        0x01000000
#define IPMC_EVENT_MASTER_DOWN      0x02000000

/* IPMC Event Values */
#define IPMC_EVENT_VALUE_INIT       0x0
#define IPMC_EVENT_VALUE_SW_ADD     0x00000001
#define IPMC_EVENT_VALUE_SW_DEL     0x00000010

/*lint -esym(459, ipmc_sm_events) */
static cyg_flag_t   ipmc_sm_events;

static cyg_alarm    ipmc_sm_alarm;
static cyg_handle_t ipmc_sm_alarm_handle;

static u32          ipmc_switch_event_value[VTSS_ISID_END];

static void ipmc_sm_event_set(cyg_flag_value_t flag)
{
    cyg_flag_setbits(&ipmc_sm_events, flag);
}

static void ipmc_sm_timer_isr(cyg_handle_t alarm, cyg_addrword_t data)
{
    if (alarm || data) { /* avoid warning */
    }

    ipmc_sm_event_set(IPMC_EVENT_SM_TIME_WAKEUP);
}

static void _ipmc_trans_cfg_intf_info(ipmc_prot_intf_entry_param_t *intf_param, ipmc_conf_intf_entry_t *entry)
{
    ipmc_querier_sm_t   *querier;

    if (!entry || !entry->valid || !intf_param) {
        return;
    }

    intf_param->vid = entry->vid;
    intf_param->cfg_compatibility = entry->compatibility;
    intf_param->priority = entry->priority;
    querier = &intf_param->querier;
    querier->querier_enabled = entry->querier_status;
    querier->QuerierAdrs4 = entry->querier4_address;
    querier->RobustVari = entry->robustness_variable;
    querier->QueryIntvl = entry->query_interval;;
    querier->MaxResTime = entry->query_response_interval;
    querier->LastQryItv = entry->last_listener_query_interval;
    querier->LastQryCnt = querier->RobustVari;
    querier->UnsolicitR = entry->unsolicited_report_interval;
}

static void _ipmc_reset_conf_intf(ipmc_conf_intf_entry_t *entry, BOOL valid, vtss_vid_t vid)
{
    entry->valid = valid;
    if (valid) {
        entry->vid = vid;
    } else {
        entry->vid = 0x0;
    }
    entry->protocol_status = IPMC_DEF_INTF_STATE_VALUE;
    entry->querier_status = IPMC_DEF_INTF_QUERIER_VALUE;
    entry->proxy_status = FALSE;
    entry->compatibility = IPMC_PARAM_DEF_COMPAT;
    entry->priority = IPMC_PARAM_DEF_PRIORITY;
    entry->querier4_address = IPMC_PARAM_DEF_QUERIER_ADRS4;
    entry->robustness_variable = IPMC_PARAM_DEF_RV;
    entry->query_interval = IPMC_PARAM_DEF_QI;
    entry->query_response_interval = IPMC_PARAM_DEF_QRI;
    entry->last_listener_query_interval = IPMC_PARAM_DEF_LLQI;
    entry->unsolicited_report_interval = IPMC_PARAM_DEF_URI;
}

static u16 ipmc_conf_intf_entry_get_next(vtss_vid_t *vid, ipmc_ip_version_t ipmc_version)
{
    u16                     idx, ret = IPMC_VID_INIT;
    vtss_vid_t              next;
    ipmc_configuration_t    *conf;
    ipmc_conf_intf_entry_t  *intf;

    if (!vid || (*vid > VTSS_IPMC_VID_MAX)) {
        return ret;
    }

    conf = NULL;
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        conf = &ipmc_global.ipv4_conf;

        break;
    case IPMC_IP_VERSION_MLD:
        conf = &ipmc_global.ipv6_conf;

        break;
    default:
        break;
    }

    if (!conf) {
        return ret;
    }

    next = 0;
    for (idx = 0; idx < IPMC_VLAN_MAX; idx++) {
        intf = &conf->ipmc_conf_intf_entries[idx];

        if (intf && intf->valid && (intf->vid > *vid)) {
            if (next) {
                if (intf->vid < next) {
                    next = intf->vid;
                    ret = idx;
                }
            } else {
                next = intf->vid;
                ret = idx;
            }
        }
    }

    if (next) {
        *vid = next;
    }

    return ret;
}

static u16 ipmc_conf_intf_entry_get(vtss_vid_t vid, ipmc_ip_version_t ipmc_version)
{
    u16                     idx, ret = IPMC_VID_INIT;
    ipmc_configuration_t    *conf;
    ipmc_conf_intf_entry_t  *intf;

    if (vid > VTSS_IPMC_VID_MAX) {
        return ret;
    }

    conf = NULL;
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        conf = &ipmc_global.ipv4_conf;

        break;
    case IPMC_IP_VERSION_MLD:
        conf = &ipmc_global.ipv6_conf;

        break;
    default:
        break;
    }

    if (!conf) {
        return ret;
    }

    for (idx = 0; idx < IPMC_VLAN_MAX; idx++) {
        intf = &conf->ipmc_conf_intf_entries[idx];

        if (intf && intf->valid && (intf->vid == vid)) {
            ret = idx;
            break;
        }
    }

    return ret;
}

static char *ipmc_msg_id_txt(ipmc_msg_id_t msg_id)
{
    char    *txt;

    switch ( msg_id ) {
    case IPMC_MSG_ID_MODE_SET_REQ:
        txt = "IPMC_MSG_ID_MODE_SET_REQ";
        break;
    case IPMC_MSG_ID_SYS_MGMT_SET_REQ:
        txt = "IPMC_MSG_ID_SYS_MGMT_SET_REQ";
        break;
    case IPMC_MSG_ID_LEAVE_PROXY_SET_REQ:
        txt = "IPMC_MSG_ID_LEAVE_PROXY_SET_REQ";
        break;
    case IPMC_MSG_ID_PROXY_SET_REQ:
        txt = "IPMC_MSG_ID_PROXY_SET_REQ";
        break;
    case IPMC_MSG_ID_SSM_RANGE_SET_REQ:
        txt = "IPMC_MSG_ID_SSM_RANGE_SET_REQ";
        break;
    case IPMC_MSG_ID_ROUTER_PORT_SET_REQ:
        txt = "IPMC_MSG_ID_ROUTER_PORT_SET_REQ";
        break;
    case IPMC_MSG_ID_FAST_LEAVE_PORT_SET_REQ:
        txt = "IPMC_MSG_ID_FAST_LEAVE_PORT_SET_REQ";
        break;
    case IPMC_MSG_ID_UNREG_FLOOD_SET_REQ:
        txt = "IPMC_MSG_ID_UNREG_FLOOD_SET_REQ";
        break;
    case IPMC_MSG_ID_THROLLTING_MAX_NO_SET_REQ:
        txt = "IPMC_MSG_ID_THROLLTING_MAX_NO_SET_REQ";
        break;
    case IPMC_MSG_ID_PORT_GROUP_FILTERING_SET_REQ:
        txt = "IPMC_MSG_ID_PORT_GROUP_FILTERING_SET_REQ";
        break;
    case IPMC_MSG_ID_VLAN_SET_REQ:
        txt = "IPMC_MSG_ID_VLAN_SET_REQ";
        break;
    case IPMC_MSG_ID_VLAN_ENTRY_SET_REQ:
        txt = "IPMC_MSG_ID_VLAN_ENTRY_SET_REQ";
        break;
    case IPMC_MSG_ID_VLAN_ENTRY_GET_REQ:
        txt = "IPMC_MSG_ID_VLAN_ENTRY_GET_REQ";
        break;
    case IPMC_MSG_ID_VLAN_ENTRY_GET_REP:
        txt = "IPMC_MSG_ID_VLAN_ENTRY_GET_REP";
        break;
    case IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ:
        txt = "IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ";
        break;
    case IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP:
        txt = "IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP";
        break;
    case IPMC_MSG_ID_GROUP_SRCLIST_WALK_REQ:
        txt = "IPMC_MSG_ID_GROUP_SRCLIST_WALK_REQ";
        break;
    case IPMC_MSG_ID_GROUP_SRCLIST_WALK_REP:
        txt = "IPMC_MSG_ID_GROUP_SRCLIST_WALK_REP";
        break;
    case IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REQ:
        txt = "IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REQ";
        break;
    case IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REP:
        txt = "IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REP";
        break;
    case IPMC_MSG_ID_STAT_COUNTER_CLEAR_REQ:
        txt = "IPMC_MSG_ID_STAT_COUNTER_CLEAR_REQ";
        break;
    case IPMC_MSG_ID_STP_PORT_CHANGE_REQ:
        txt = "IPMC_MSG_ID_STP_PORT_CHANGE_REQ";
        break;
    default:
        txt = "?";
        break;
    }

    return txt;
}

/* Allocate request/reply buffer */
static void ipmc_msg_alloc(vtss_isid_t isid, ipmc_msg_id_t msg_id, ipmc_msg_buf_t *buf, BOOL from_get, BOOL request)
{
    u32 msg_size;

    T_I("(ISID%d)msg-%s (%d)", isid, ipmc_msg_id_txt(msg_id), msg_id);

    if (from_get) {
        IPMC_GET_CRIT_ENTER();
    } else {
        IPMC_CRIT_ENTER();
    }
    buf->sem = (request ? &ipmc_global.request.sem : &ipmc_global.reply.sem);
    buf->msg = ipmc_global.msg[msg_id];
    msg_size = ipmc_global.msize[msg_id];
    if (from_get) {
        IPMC_GET_CRIT_EXIT();
    } else {
        IPMC_CRIT_EXIT();
    }

    (void) VTSS_OS_SEM_WAIT(buf->sem);
    T_I("(ISID%d)msg-%s %s-LOCK", isid, ipmc_msg_id_txt(msg_id), request ? "REQ" : "REP");
    memset(buf->msg, 0x0, msg_size);
}

static void ipmc_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    ipmc_msg_id_t   msg_id = *(ipmc_msg_id_t *)msg;

    (void) VTSS_OS_SEM_POST(contxt);
    T_I("msg_id: %d->%s-UNLOCK", msg_id, ipmc_msg_id_txt(msg_id));
}

static void ipmc_msg_tx(ipmc_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
    ipmc_msg_id_t   msg_id = *(ipmc_msg_id_t *)buf->msg;

    T_I("msg_id: %d->%s, len: %zd, isid: %s-%d",
        msg_id,
        ipmc_msg_id_txt(msg_id),
        len,
        ipmc_lib_isid_is_local(isid) ? "LOC" : "REM",
        isid);
    msg_tx_adv(buf->sem, ipmc_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_IPMC, isid, buf->msg, len);
}

static BOOL ipmc_msg_rx(void *contxt, const void *const rx_msg, size_t len, vtss_module_id_t modid, u32 isid)
{
    ipmc_msg_id_t       msg_id = *(ipmc_msg_id_t *)rx_msg;
    port_iter_t         pit;
    int                 i;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    T_D("msg_id: %d->%s, len: %zd, isid: %u", msg_id, ipmc_msg_id_txt(msg_id), len, isid);

    switch ( msg_id ) {
    case IPMC_MSG_ID_MODE_SET_REQ: {
        ipmc_msg_mode_set_req_t *msg;
        BOOL                    apply_mode;

        msg = (ipmc_msg_mode_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_MODE_SET_REQ(Version=%d|Mode=%s)",
            msg->version,
            msg->ipmc_mode_enabled ? "TRUE" : "FALSE");

        if (msg->ipmc_mode_enabled == VTSS_IPMC_TRUE) {
            apply_mode = TRUE;
        } else {
            apply_mode = FALSE;
        }

        IPMC_CRIT_ENTER();
        if (msg->version == IPMC_IP_VERSION_IGMP) {
            ipmc_global.ipv4_conf.global.ipmc_mode_enabled = apply_mode;
        } else {
            ipmc_global.ipv6_conf.global.ipmc_mode_enabled = apply_mode;
        }
        vtss_ipmc_set_mode(apply_mode, msg->version);
        IPMC_CRIT_EXIT();

        if (msg->version == IPMC_IP_VERSION_IGMP) {
            ipmc_sm_event_set(IPMC_EVENT_PKT4_HANDLER);
        } else {
            ipmc_sm_event_set(IPMC_EVENT_PKT6_HANDLER);
        }

        break;
    }
    case IPMC_MSG_ID_SYS_MGMT_SET_REQ: {
        ipmc_msg_sys_mgmt_set_req_t *msg;
        ipmc_lib_mgmt_info_t        *sys_mgmt;
        ipmc_mgmt_ipif_t            *ifp;
        u32                         ifx;

        if (msg_switch_is_master() ||
            !IPMC_MEM_SYSTEM_MTAKE(sys_mgmt, sizeof(ipmc_lib_mgmt_info_t))) {
            break;
        }

        msg = (ipmc_msg_sys_mgmt_set_req_t *)rx_msg;
        T_I("Receiving IPMC_MSG_ID_SYS_MGMT_SET_REQ:MAC[%s]", misc_mac2str(msg->mgmt_mac));

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
    case IPMC_MSG_ID_LEAVE_PROXY_SET_REQ: {
        ipmc_msg_leave_proxy_set_req_t  *msg;

        msg = (ipmc_msg_leave_proxy_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_LEAVE_PROXY_SET_REQ");

        IPMC_CRIT_ENTER();
        if (msg->ipmc_leave_proxy_enabled == VTSS_IPMC_TRUE) {
            vtss_ipmc_set_leave_proxy(TRUE, msg->version);
        } else {
            vtss_ipmc_set_leave_proxy(FALSE, msg->version);
        }
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_PROXY_SET_REQ: {
        ipmc_msg_proxy_set_req_t    *msg;

        msg = (ipmc_msg_proxy_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_PROXY_SET_REQ");

        IPMC_CRIT_ENTER();
        if (msg->ipmc_proxy_enabled == VTSS_IPMC_TRUE) {
            vtss_ipmc_set_proxy(TRUE, msg->version);
        } else {
            vtss_ipmc_set_proxy(FALSE, msg->version);
        }
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_SSM_RANGE_SET_REQ: {
        ipmc_msg_ssm_range_set_req_t    *msg;

        msg = (ipmc_msg_ssm_range_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_SSM_RANGE_SET_REQ");

        IPMC_CRIT_ENTER();
        vtss_ipmc_set_ssm_range(msg->version, &msg->prefix);
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_ROUTER_PORT_SET_REQ: {
        ipmc_msg_router_port_set_req_t  *msg;
        ipmc_port_bfs_t                 static_router_port_mask_temp;

        msg = (ipmc_msg_router_port_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_ROUTER_PORT_SET_REQ");

        VTSS_PORT_BF_CLR(static_router_port_mask_temp.member_ports);
        if (msg_switch_is_master()) {
            (void) port_iter_init(&pit, NULL, msg->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        } else {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        }
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            if (msg->ipmc_router_ports[i]) {
                VTSS_PORT_BF_SET(static_router_port_mask_temp.member_ports, i, TRUE);
            }
        }

        IPMC_CRIT_ENTER();
        vtss_ipmc_set_static_router_ports(&static_router_port_mask_temp, msg->version);
        IPMC_CRIT_EXIT();

        break;
    }

    case IPMC_MSG_ID_FAST_LEAVE_PORT_SET_REQ: {
        ipmc_msg_fast_leave_port_set_req_t  *msg;
        ipmc_port_bfs_t                     static_fast_leave_port_mask_temp;

        msg = (ipmc_msg_fast_leave_port_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_FAST_LEAVE_PORT_SET_REQ");

        VTSS_PORT_BF_CLR(static_fast_leave_port_mask_temp.member_ports);
        if (msg_switch_is_master()) {
            (void) port_iter_init(&pit, NULL, msg->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        } else {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        }
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            if (msg->ipmc_fast_leave_ports[i]) {
                VTSS_PORT_BF_SET(static_fast_leave_port_mask_temp.member_ports, i, TRUE);
            }
        }

        IPMC_CRIT_ENTER();
        vtss_ipmc_set_static_fast_leave_ports(&static_fast_leave_port_mask_temp, msg->version);
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_THROLLTING_MAX_NO_SET_REQ: {
        ipmc_msg_throllting_max_no_set_req_t    *msg;
        ipmc_port_throttling_t                  static_ipmc_port_throttling;

        msg = (ipmc_msg_throllting_max_no_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_THROLLTING_MAX_NO_SET_REQ");

        memset(&static_ipmc_port_throttling, 0x0, sizeof(ipmc_port_throttling_t));
        if (msg_switch_is_master()) {
            (void) port_iter_init(&pit, NULL, msg->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        } else {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        }
        while (port_iter_getnext(&pit)) {
            static_ipmc_port_throttling.max_no[pit.iport] = msg->ipmc_throttling_max_no[pit.iport];
        }

        IPMC_CRIT_ENTER();
        vtss_ipmc_set_static_port_throttling_max_no(&static_ipmc_port_throttling, msg->version);
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_PORT_GROUP_FILTERING_SET_REQ: {
        ipmc_msg_port_group_filtering_set_req_t *msg;
        ipmc_port_group_filtering_t             static_ipmc_port_group_filtering_entry;

        msg = (ipmc_msg_port_group_filtering_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_PORT_GROUP_FILTERING_SET_REQ");

        memset(&static_ipmc_port_group_filtering_entry, 0x0, sizeof(ipmc_port_group_filtering_t));
        if (msg_switch_is_master()) {
            (void) port_iter_init(&pit, NULL, msg->isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        } else {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        }
        while (port_iter_getnext(&pit)) {
            i = pit.iport;
            static_ipmc_port_group_filtering_entry.profile_index[i] = msg->ipmc_port_group_filtering[i];
        }

        IPMC_CRIT_ENTER();
        vtss_ipmc_set_static_port_group_filtering(&static_ipmc_port_group_filtering_entry, msg->version);
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_UNREG_FLOOD_SET_REQ: {
        ipmc_msg_unreg_flood_set_req_t  *msg;

        msg = (ipmc_msg_unreg_flood_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_UNREG_FLOOD_SET_REQ");

        IPMC_CRIT_ENTER();
        vtss_ipmc_set_unreg_flood(msg->ipmc_unreg_flood_enabled, msg->version);
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_VLAN_SET_REQ: {
        ipmc_msg_vlan_set_req_t *msg;

        msg = (ipmc_msg_vlan_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_VLAN_SET_REQ");
        T_D("Action: %s (%d->%s/%s[VER-%d])",
            msg->action ? "ADD" : "DEL",
            msg->vlan_entry.vid,
            msg->vlan_entry.protocol_status ? "Enable" : "Disable",
            msg->vlan_entry.querier_status ? "Q" : "!Q",
            msg->version);

        IPMC_CRIT_ENTER();
        if (msg->action == IPMC_OP_DEL) {
            (void) vtss_ipmc_del_intf_entry(msg->vlan_entry.vid, msg->version);
        } else {
            (void) vtss_ipmc_set_intf_entry(msg->vlan_entry.vid,
                                            msg->vlan_entry.protocol_status,
                                            msg->vlan_entry.querier_status,
                                            &msg->vlan_member,
                                            msg->version);
        }
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_VLAN_ENTRY_SET_REQ: {
        ipmc_msg_vlan_set_req_t *msg;

        msg = (ipmc_msg_vlan_set_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_VLAN_ENTRY_SET_REQ");

        IPMC_CRIT_ENTER();
        if (!vtss_ipmc_upd_intf_entry(&msg->vlan_entry, msg->version)) {
            T_D("IPMC_MSG_ID_VLAN_ENTRY_SET_REQ Failed!");
        }
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_VLAN_ENTRY_GET_REQ: {
        ipmc_msg_vlan_entry_get_req_t   *msg;
        ipmc_intf_entry_t               *intf;
        ipmc_msg_vlan_entry_get_rep_t   *msg_rep;
        ipmc_msg_buf_t                  buf;
        BOOL                            valid = FALSE;

        msg = (ipmc_msg_vlan_entry_get_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_VLAN_ENTRY_GET_REQ (VID:%d/VER:%d)", msg->vid, msg->version);

        ipmc_msg_alloc(isid, IPMC_MSG_ID_VLAN_ENTRY_GET_REP, &buf, FALSE, FALSE);
        IPMC_CRIT_ENTER();
        msg_rep = (ipmc_msg_vlan_entry_get_rep_t *)buf.msg;
        if ((intf = vtss_ipmc_get_intf_entry(msg->vid, msg->version)) != NULL) {
            memcpy(&msg_rep->interface_entry, &intf->param, sizeof(ipmc_prot_intf_entry_param_t));
            valid = TRUE;
        } else {
            ipmc_conf_intf_entry_t  cur_intf;
            u16                     intf_idx;

            if ((intf_idx = ipmc_conf_intf_entry_get(msg->vid, msg->version)) != IPMC_VID_INIT) {
                switch ( msg->version ) {
                case IPMC_IP_VERSION_IGMP:
                    memcpy(&cur_intf, &ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx], sizeof(ipmc_conf_intf_entry_t));
                    valid = TRUE;

                    break;
                case IPMC_IP_VERSION_MLD:
                    memcpy(&cur_intf, &ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx], sizeof(ipmc_conf_intf_entry_t));
                    valid = TRUE;

                    break;
                default:

                    break;
                }

                if (valid) {
                    memset(&msg_rep->interface_entry, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
                    _ipmc_trans_cfg_intf_info(&msg_rep->interface_entry, &cur_intf);
                }
            }
        }
        IPMC_CRIT_EXIT();

        msg_rep->msg_id = IPMC_MSG_ID_VLAN_ENTRY_GET_REP;
        if (!valid) {
            msg_rep->interface_entry.vid = 0;
            T_D("no such vlan (%d)", msg->vid);
        }
        ipmc_msg_tx(&buf, isid, sizeof(*msg_rep));

        break;
    }
    case IPMC_MSG_ID_VLAN_ENTRY_GET_REP: {
        ipmc_msg_vlan_entry_get_rep_t   *msg;

        msg = (ipmc_msg_vlan_entry_get_rep_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_VLAN_ENTRY_GET_REP");

        IPMC_CRIT_ENTER();
        memcpy(&ipmc_global.interface_entry, &msg->interface_entry, sizeof(ipmc_prot_intf_entry_param_t));
        cyg_flag_setbits(&ipmc_global.vlan_entry_flags, 1 << isid);
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ: {
        ipmc_msg_vlan_entry_get_req_t       *msg;
        ipmc_msg_vlan_group_entry_get_rep_t *msg_rep;
        ipmc_msg_buf_t                      buf;
        ipmc_group_entry_t                  grp;
        BOOL                                valid = FALSE;

        msg = (ipmc_msg_vlan_entry_get_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ");

        grp.ipmc_version = msg->version;
        grp.vid = msg->vid;
        memcpy(&grp.group_addr, &msg->group_addr, sizeof(vtss_ipv6_t));

        ipmc_msg_alloc(isid, IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP, &buf, FALSE, FALSE);
        IPMC_CRIT_ENTER();
        if (vtss_ipmc_get_next_intf_group_entry(msg->vid, &grp, msg->version)) {
            valid = TRUE;
        }
        IPMC_CRIT_EXIT();

        msg_rep = (ipmc_msg_vlan_group_entry_get_rep_t *)buf.msg;
        msg_rep->msg_id = IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP;
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
        ipmc_msg_tx(&buf, isid, sizeof(*msg_rep));

        break;
    }
    case IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP: {
        ipmc_msg_vlan_group_entry_get_rep_t *msg;

        msg = (ipmc_msg_vlan_group_entry_get_rep_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP");

        IPMC_CRIT_ENTER();
        memcpy(&ipmc_global.intf_group_entry, &msg->intf_group_entry, sizeof(ipmc_prot_intf_group_entry_t));
        cyg_flag_setbits(&ipmc_global.vlan_entry_flags, 1 << isid);
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_GROUP_SRCLIST_WALK_REQ: {
        ipmc_msg_vlan_entry_get_req_t       *msg;
        ipmc_msg_group_srclist_get_rep_t    *msg_rep;
        ipmc_msg_buf_t                      buf;
        ipmc_group_entry_t                  grp;
        BOOL                                valid = FALSE;

        msg = (ipmc_msg_vlan_entry_get_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_GROUP_SRCLIST_WALK_REQ");

        grp.ipmc_version = msg->version;
        grp.vid = msg->vid;
        memcpy(&grp.group_addr, &msg->group_addr, sizeof(vtss_ipv6_t));

        ipmc_msg_alloc(isid, IPMC_MSG_ID_GROUP_SRCLIST_WALK_REP, &buf, FALSE, FALSE);
        IPMC_CRIT_ENTER();
        if (vtss_ipmc_get_intf_group_entry(msg->vid, &grp, msg->version)) {
            valid = TRUE;
        }
        IPMC_CRIT_EXIT();

        msg_rep = (ipmc_msg_group_srclist_get_rep_t *)buf.msg;
        msg_rep->msg_id = IPMC_MSG_ID_GROUP_SRCLIST_WALK_REP;

        msg_rep->group_srclist_entry.valid = FALSE;
        if (valid) {
            ipmc_db_ctrl_hdr_t  *slist;

            if (msg->srclist_type) {
                slist = grp.info->db.ipmc_sf_do_forward_srclist;
            } else {
                slist = grp.info->db.ipmc_sf_do_not_forward_srclist;
            }

            if (slist &&
                ((msg_rep->group_srclist_entry.cntr = IPMC_LIB_DB_GET_COUNT(slist)) != 0)) {
                ipmc_sfm_srclist_t  entry;

                memcpy(&entry.src_ip, &msg->srclist_addr, sizeof(vtss_ipv6_t));
                if (ipmc_lib_srclist_buf_get_next(slist, &entry)) {
                    msg_rep->group_srclist_entry.type = msg->srclist_type;
                    memcpy(&msg_rep->group_srclist_entry.srclist, &entry, sizeof(ipmc_sfm_srclist_t));
                    IPMC_SRCT_TIMER_DELTA_GET(&msg_rep->group_srclist_entry.srclist);

                    msg_rep->group_srclist_entry.valid = TRUE;
                }
            }
        }
        ipmc_msg_tx(&buf, isid, sizeof(*msg_rep));

        break;
    }
    case IPMC_MSG_ID_GROUP_SRCLIST_WALK_REP: {
        ipmc_msg_group_srclist_get_rep_t    *msg;

        msg = (ipmc_msg_group_srclist_get_rep_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_GROUP_SRCLIST_WALK_REP");

        IPMC_CRIT_ENTER();
        memcpy(&ipmc_global.group_srclist_entry, &msg->group_srclist_entry, sizeof(ipmc_prot_group_srclist_t));
        cyg_flag_setbits(&ipmc_global.vlan_entry_flags, 1 << isid);
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REQ: {
        ipmc_msg_dyn_rtpt_get_req_t             *msg;
        ipmc_msg_dynamic_router_ports_get_rep_t *msg_rep;
        ipmc_msg_buf_t                          buf;
        ipmc_port_bfs_t                         router_port_bitmask;

        msg = (ipmc_msg_dyn_rtpt_get_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REQ");

        ipmc_msg_alloc(isid, IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REP, &buf, FALSE, FALSE);
        IPMC_CRIT_ENTER();
        ipmc_lib_get_discovered_router_port_mask(msg->version, &router_port_bitmask);
        IPMC_CRIT_EXIT();

        msg_rep = (ipmc_msg_dynamic_router_ports_get_rep_t *)buf.msg;
        msg_rep->msg_id = IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REP;
        msg_rep->version = msg->version;
        if (msg_switch_is_master()) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        } else {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        }
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            if (VTSS_PORT_BF_GET(router_port_bitmask.member_ports, i)) {
                msg_rep->dynamic_router_ports.ports[i] = TRUE;
            } else {
                msg_rep->dynamic_router_ports.ports[i] = FALSE;
            }
        }
        ipmc_msg_tx(&buf, isid, sizeof(*msg_rep));

        break;
    }
    case IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REP: {
        ipmc_msg_dynamic_router_ports_get_rep_t *msg;

        msg = (ipmc_msg_dynamic_router_ports_get_rep_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REP");

        IPMC_CRIT_ENTER();
        if (msg_switch_is_master()) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        } else {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        }
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            ipmc_global.dynamic_router_ports[isid - VTSS_ISID_START][i] = msg->dynamic_router_ports.ports[i];
        }
        cyg_flag_setbits(&ipmc_global.dynamic_router_ports_getting_flags, 1 << isid);
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_STAT_COUNTER_CLEAR_REQ: {
        ipmc_msg_stat_counter_clear_req_t   *msg;

        msg = (ipmc_msg_stat_counter_clear_req_t *)rx_msg;
        T_D("Receiving IPMC_MSG_ID_STAT_COUNTER_CLEAR_REQ");

        IPMC_CRIT_ENTER();
        vtss_ipmc_clear_stat_counter(msg->version, msg->vid);
        IPMC_CRIT_EXIT();

        break;
    }
    case IPMC_MSG_ID_STP_PORT_CHANGE_REQ: {
        ipmc_msg_stp_port_change_set_req_t  *msg;
        ipmc_port_bfs_t                     router_port_bitmask;

        msg = (ipmc_msg_stp_port_change_set_req_t *) rx_msg;
        T_D("Receiving IPMC_MSG_ID_STP_PORT_CHANGE_REQ");

        VTSS_PORT_BF_CLR(router_port_bitmask.member_ports);

        IPMC_CRIT_ENTER();
        /* We should filter sendout process if this port belongs to dynamic router port. */
        switch ( msg->version ) {
        case IPMC_IP_VERSION_ALL:
            ipmc_lib_get_discovered_router_port_mask(IPMC_IP_VERSION_IGMP, &router_port_bitmask);
            if (!VTSS_PORT_BF_GET(router_port_bitmask.member_ports, msg->port)) {
                vtss_ipmc_stp_port_state_change_handle(IPMC_IP_VERSION_IGMP, msg->port, msg->new_state);
            }

            VTSS_PORT_BF_CLR(router_port_bitmask.member_ports);
            ipmc_lib_get_discovered_router_port_mask(IPMC_IP_VERSION_MLD, &router_port_bitmask);
            if (!VTSS_PORT_BF_GET(router_port_bitmask.member_ports, msg->port)) {
                vtss_ipmc_stp_port_state_change_handle(IPMC_IP_VERSION_MLD, msg->port, msg->new_state);
            }

            break;
        case IPMC_IP_VERSION_IGMP:
        case IPMC_IP_VERSION_MLD:
            ipmc_lib_get_discovered_router_port_mask(msg->version, &router_port_bitmask);
            if (!VTSS_PORT_BF_GET(router_port_bitmask.member_ports, msg->port)) {
                vtss_ipmc_stp_port_state_change_handle(msg->version, msg->port, msg->new_state);
            }

            break;
        default:

            break;
        }
        IPMC_CRIT_EXIT();

        break;
    }
    default:
        T_E("unknown message ID: %d", msg_id);

        break;
    }

    T_D("ipmc_msg_rx(%u) consumes ID%u:%u ticks", (ulong)msg_id, isid, (ulong)(cyg_current_time() - exe_time_base));

    return TRUE;
}

static u16 _ipmc_conf_intf_entry_add(vtss_vid_t vid, ipmc_ip_version_t ipmc_version)
{
    u16                     idx, ret;
    ipmc_configuration_t    *conf;
    ipmc_conf_intf_entry_t  *intf;

    if (vid > VTSS_IPMC_VID_MAX) {
        return IPMC_VID_INIT;
    }

    conf = NULL;
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        conf = &ipmc_global.ipv4_conf;

        break;
    case IPMC_IP_VERSION_MLD:
        conf = &ipmc_global.ipv6_conf;

        break;
    default:
        break;
    }

    if (!conf) {
        return IPMC_VID_INIT;
    }

    ret = IPMC_VID_INIT;
    for (idx = 0; idx < IPMC_VLAN_MAX; idx++) {
        if ((intf = &conf->ipmc_conf_intf_entries[idx]) == NULL) {
            continue;
        }

        if (intf->valid) {
            if (intf->vid == vid) {
                return IPMC_VID_INIT;
            }
        } else {
            ret = idx;  /* pick last available index */
        }
    }

    if (ret == IPMC_VID_INIT) {
        return IPMC_VID_INIT;   /* Table Full */
    }

    _ipmc_reset_conf_intf(&conf->ipmc_conf_intf_entries[ret], TRUE, vid);
    return ret;
}

static BOOL _ipmc_conf_intf_entry_del(vtss_vid_t vid, ipmc_ip_version_t ipmc_version)
{
    u16                     idx, chk;
    ipmc_configuration_t    *conf;
    ipmc_conf_intf_entry_t  *intf;

    if (vid > VTSS_IPMC_VID_MAX) {
        return FALSE;
    }

    conf = NULL;
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        conf = &ipmc_global.ipv4_conf;

        break;
    case IPMC_IP_VERSION_MLD:
        conf = &ipmc_global.ipv6_conf;

        break;
    default:
        break;
    }

    if (!conf) {
        return FALSE;
    }

    chk = IPMC_VID_INIT;
    for (idx = 0; idx < IPMC_VLAN_MAX; idx++) {
        if ((intf = &conf->ipmc_conf_intf_entries[idx]) == NULL) {
            continue;
        }

        if (intf->valid && (intf->vid == vid)) {
            chk = idx;
            break;
        }
    }

    if (chk == IPMC_VID_INIT) {
        return FALSE;   /* Not Found */
    }

    _ipmc_reset_conf_intf(&conf->ipmc_conf_intf_entries[chk], FALSE, vid);
    return TRUE;
}

static BOOL _ipmc_conf_intf_entry_upd(ipmc_conf_intf_entry_t *entry, ipmc_ip_version_t ipmc_version)
{
    u16                     idx, chk;
    vtss_vid_t              vid;
    ipmc_configuration_t    *conf;
    ipmc_conf_intf_entry_t  *intf;

    if (!entry || ((vid = entry->vid) > VTSS_IPMC_VID_MAX)) {
        return FALSE;
    }

    conf = NULL;
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        conf = &ipmc_global.ipv4_conf;

        break;
    case IPMC_IP_VERSION_MLD:
        conf = &ipmc_global.ipv6_conf;

        break;
    default:
        break;
    }

    if (!conf) {
        return FALSE;
    }

    chk = IPMC_VID_INIT;
    for (idx = 0; idx < IPMC_VLAN_MAX; idx++) {
        if ((intf = &conf->ipmc_conf_intf_entries[idx]) == NULL) {
            continue;
        }

        if (intf->valid && (intf->vid == vid)) {
            chk = idx;
            break;
        }
    }

    if (chk == IPMC_VID_INIT) {
        return FALSE;   /* Not Found */
    }

    intf = &conf->ipmc_conf_intf_entries[chk];
    memcpy(intf, entry, sizeof(ipmc_conf_intf_entry_t));
    memset(intf->reserved, 0x0, sizeof(intf->reserved));
    intf->valid = TRUE;
    return TRUE;
}

/* Set STACK IPMC VLAN ENTRY */
static vtss_rc ipmc_stacking_set_intf(vtss_isid_t isid_add, ipmc_operation_action_t action, ipmc_conf_intf_entry_t *vlan_entry, ipmc_port_bfs_t *vlan_ports, ipmc_ip_version_t ipmc_version)
{
    ipmc_msg_buf_t              buf;
    ipmc_msg_vlan_set_req_t     *msg;
    switch_iter_t               sit;
    vtss_isid_t                 isid;

    if (!msg_switch_is_master() || !vlan_entry || !vlan_ports) {
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
            IPMC_CRIT_ENTER();
            if (action == IPMC_OP_DEL) {
                (void) vtss_ipmc_del_intf_entry(vlan_entry->vid, ipmc_version);
            } else {
                (void) vtss_ipmc_set_intf_entry(vlan_entry->vid,
                                                vlan_entry->protocol_status,
                                                vlan_entry->querier_status,
                                                vlan_ports,
                                                ipmc_version);
            }
            IPMC_CRIT_EXIT();
        } else {
            ipmc_msg_alloc(isid, IPMC_MSG_ID_VLAN_SET_REQ, &buf, FALSE, TRUE);
            msg = (ipmc_msg_vlan_set_req_t *)buf.msg;

            msg->msg_id = IPMC_MSG_ID_VLAN_SET_REQ;
            msg->action = action;
            msg->vlan_entry.valid = vlan_entry->valid;
            msg->vlan_entry.vid = vlan_entry->vid;
            msg->vlan_entry.protocol_status = vlan_entry->protocol_status;
            msg->vlan_entry.querier_status = vlan_entry->querier_status;
            msg->vlan_entry.compatibility = vlan_entry->compatibility;
            msg->vlan_entry.priority = vlan_entry->priority;
            msg->vlan_entry.querier4_address = vlan_entry->querier4_address;
            msg->vlan_entry.robustness_variable = vlan_entry->robustness_variable;
            msg->vlan_entry.query_interval = vlan_entry->query_interval;
            msg->vlan_entry.query_response_interval = vlan_entry->query_response_interval;
            msg->vlan_entry.last_listener_query_interval = vlan_entry->last_listener_query_interval;
            msg->vlan_entry.unsolicited_report_interval = vlan_entry->unsolicited_report_interval;
            msg->vlan_member = *vlan_ports;
            msg->version = ipmc_version;
            ipmc_msg_tx(&buf, isid, sizeof(*msg));
        }
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* IPMC error text */
char *ipmc_error_txt(ipmc_error_t rc)
{
    char *txt;

    switch ( rc ) {
    case IPMC_ERROR_GEN:
        txt = "IPMC generic error";
        break;
    case IPMC_ERROR_PARM:
        txt = "IPMC parameter error";
        break;
    case IPMC_ERROR_VLAN_NOT_FOUND:
        txt = "IPMC no such VLAN ID";
        break;
    default:
        txt = "IPMC unknown error";
        break;
    }

    return txt;
}

static vtss_rc ipmc_stacking_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = ipmc_msg_rx;
    filter.modid = VTSS_MODULE_ID_IPMC;
    return msg_rx_filter_register(&filter);
}

/* Set STACK IPMC MODE */
static BOOL mode_setting;
static vtss_rc ipmc_stacking_set_mode(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version, BOOL force_off)
{
    ipmc_msg_buf_t          buf;
    ipmc_msg_mode_set_req_t *msg;
    switch_iter_t           sit;
    vtss_isid_t             isid;
    BOOL                    apply_mode;

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
            IPMC_CRIT_ENTER();
            switch ( ipmc_version ) {
            case IPMC_IP_VERSION_IGMP:
                if (force_off) {
                    mode_setting = VTSS_IPMC_FALSE;
                    ipmc_global.ipv4_conf.global.ipmc_mode_enabled = mode_setting;
                } else {
                    mode_setting = ipmc_global.ipv4_conf.global.ipmc_mode_enabled;
                }

                break;
            case IPMC_IP_VERSION_MLD:
                if (force_off) {
                    mode_setting = VTSS_IPMC_FALSE;
                    ipmc_global.ipv6_conf.global.ipmc_mode_enabled = mode_setting;
                } else {
                    mode_setting = ipmc_global.ipv6_conf.global.ipmc_mode_enabled;
                }

                break;
            default:
                mode_setting = VTSS_IPMC_FALSE;

                break;
            }
            apply_mode = mode_setting;
            IPMC_CRIT_EXIT();

            if (apply_mode == VTSS_IPMC_TRUE) {
                IPMC_CRIT_ENTER();
                vtss_ipmc_set_mode(TRUE, ipmc_version);
                IPMC_CRIT_EXIT();
            } else {
                IPMC_CRIT_ENTER();
                vtss_ipmc_set_mode(FALSE, ipmc_version);
                IPMC_CRIT_EXIT();
            }

            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                ipmc_sm_event_set(IPMC_EVENT_PKT4_HANDLER);
            } else {
                ipmc_sm_event_set(IPMC_EVENT_PKT6_HANDLER);
            }

            continue;
        }

        ipmc_msg_alloc(isid, IPMC_MSG_ID_MODE_SET_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_mode_set_req_t *)buf.msg;

        if (force_off) {
            msg->ipmc_mode_enabled = VTSS_IPMC_FALSE;
        } else {
            IPMC_CRIT_ENTER();
            switch ( ipmc_version ) {
            case IPMC_IP_VERSION_IGMP:
                msg->ipmc_mode_enabled = ipmc_global.ipv4_conf.global.ipmc_mode_enabled;

                break;
            case IPMC_IP_VERSION_MLD:
                msg->ipmc_mode_enabled = ipmc_global.ipv6_conf.global.ipmc_mode_enabled;

                break;
            default:
                msg->ipmc_mode_enabled = VTSS_IPMC_FALSE;

                break;
            }
            IPMC_CRIT_EXIT();
        }

        msg->msg_id = IPMC_MSG_ID_MODE_SET_REQ;
        msg->version = ipmc_version;
        T_D("SND IPMC_MSG_ID_MODE_SET_REQ(Version=%d|Mode=%s)",
            msg->version,
            msg->ipmc_mode_enabled ? "TRUE" : "FALSE");
        ipmc_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Set STACK IPMC Leave Proxy */
static BOOL leaveproxy_setting;
static vtss_rc ipmc_stacking_set_leave_proxy(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version)
{
    ipmc_msg_buf_t                  buf;
    ipmc_msg_leave_proxy_set_req_t  *msg;
    switch_iter_t                   sit;
    vtss_isid_t                     isid;

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
            IPMC_CRIT_ENTER();
            switch ( ipmc_version ) {
            case IPMC_IP_VERSION_IGMP:
                leaveproxy_setting = ipmc_global.ipv4_conf.global.ipmc_leave_proxy_enabled;

                break;
            case IPMC_IP_VERSION_MLD:
                leaveproxy_setting = ipmc_global.ipv6_conf.global.ipmc_leave_proxy_enabled;

                break;
            default:
                leaveproxy_setting = VTSS_IPMC_FALSE;

                break;
            }

            if (leaveproxy_setting == VTSS_IPMC_TRUE) {
                vtss_ipmc_set_leave_proxy(TRUE, ipmc_version);
            } else {
                vtss_ipmc_set_leave_proxy(FALSE, ipmc_version);
            }
            IPMC_CRIT_EXIT();

            continue;
        }

        ipmc_msg_alloc(isid, IPMC_MSG_ID_LEAVE_PROXY_SET_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_leave_proxy_set_req_t *)buf.msg;

        IPMC_CRIT_ENTER();
        switch ( ipmc_version ) {
        case IPMC_IP_VERSION_IGMP:
            msg->ipmc_leave_proxy_enabled = ipmc_global.ipv4_conf.global.ipmc_leave_proxy_enabled;

            break;
        case IPMC_IP_VERSION_MLD:
            msg->ipmc_leave_proxy_enabled = ipmc_global.ipv6_conf.global.ipmc_leave_proxy_enabled;

            break;
        default:
            msg->ipmc_leave_proxy_enabled = VTSS_IPMC_FALSE;

            break;
        }
        IPMC_CRIT_EXIT();

        msg->msg_id = IPMC_MSG_ID_LEAVE_PROXY_SET_REQ;
        msg->version = ipmc_version;
        ipmc_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Set STACK IPMC Proxy */
static BOOL proxy_setting;
static vtss_rc ipmc_stacking_set_proxy(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version)
{
    ipmc_msg_buf_t              buf;
    ipmc_msg_proxy_set_req_t    *msg;
    switch_iter_t               sit;
    vtss_isid_t                 isid;

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
            IPMC_CRIT_ENTER();
            switch ( ipmc_version ) {
            case IPMC_IP_VERSION_IGMP:
                proxy_setting = ipmc_global.ipv4_conf.global.ipmc_proxy_enabled;

                break;
            case IPMC_IP_VERSION_MLD:
                proxy_setting = ipmc_global.ipv6_conf.global.ipmc_proxy_enabled;

                break;
            default:
                proxy_setting = VTSS_IPMC_FALSE;

                break;
            }

            if (proxy_setting == VTSS_IPMC_TRUE) {
                vtss_ipmc_set_proxy(TRUE, ipmc_version);
            } else {
                vtss_ipmc_set_proxy(FALSE, ipmc_version);
            }
            IPMC_CRIT_EXIT();

            continue;
        }

        ipmc_msg_alloc(isid, IPMC_MSG_ID_PROXY_SET_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_proxy_set_req_t *)buf.msg;

        IPMC_CRIT_ENTER();
        switch ( ipmc_version ) {
        case IPMC_IP_VERSION_IGMP:
            msg->ipmc_proxy_enabled = ipmc_global.ipv4_conf.global.ipmc_proxy_enabled;

            break;
        case IPMC_IP_VERSION_MLD:
            msg->ipmc_proxy_enabled = ipmc_global.ipv6_conf.global.ipmc_proxy_enabled;

            break;
        default:
            msg->ipmc_proxy_enabled = VTSS_IPMC_FALSE;

            break;
        }
        IPMC_CRIT_EXIT();

        msg->msg_id = IPMC_MSG_ID_PROXY_SET_REQ;
        msg->version = ipmc_version;
        ipmc_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Set STACK IPMC SSM Range */
static ipmc_prefix_t    prefix_setting;
static vtss_rc ipmc_stacking_set_ssm_range(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version)
{
    ipmc_msg_buf_t                  buf;
    ipmc_msg_ssm_range_set_req_t    *msg;
    switch_iter_t                   sit;
    vtss_isid_t                     isid;

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
            IPMC_CRIT_ENTER();
            switch ( ipmc_version ) {
            case IPMC_IP_VERSION_IGMP:
                memcpy(&prefix_setting, &ipmc_global.ipv4_conf.global.ssm_range, sizeof(ipmc_prefix_t));

                break;
            case IPMC_IP_VERSION_MLD:
                memcpy(&prefix_setting, &ipmc_global.ipv6_conf.global.ssm_range, sizeof(ipmc_prefix_t));

                break;
            default:
                memset(&prefix_setting, 0x0, sizeof(ipmc_prefix_t));

                break;
            }

            vtss_ipmc_set_ssm_range(ipmc_version, &prefix_setting);
            IPMC_CRIT_EXIT();

            continue;
        }

        ipmc_msg_alloc(isid, IPMC_MSG_ID_SSM_RANGE_SET_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_ssm_range_set_req_t *)buf.msg;

        IPMC_CRIT_ENTER();
        switch ( ipmc_version ) {
        case IPMC_IP_VERSION_IGMP:
            memcpy(&msg->prefix, &ipmc_global.ipv4_conf.global.ssm_range, sizeof(ipmc_prefix_t));

            break;
        case IPMC_IP_VERSION_MLD:
            memcpy(&msg->prefix, &ipmc_global.ipv6_conf.global.ssm_range, sizeof(ipmc_prefix_t));

            break;
        default:
            memset(&msg->prefix, 0x0, sizeof(ipmc_prefix_t));

            break;
        }
        IPMC_CRIT_EXIT();

        msg->msg_id = IPMC_MSG_ID_SSM_RANGE_SET_REQ;
        msg->version = ipmc_version;
        ipmc_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Set STACK IPMC ROUTER PORT */
static ipmc_port_bfs_t  routerport_setting;
static vtss_rc ipmc_stacking_set_router_port(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version)
{
    ipmc_msg_buf_t                  buf;
    ipmc_msg_router_port_set_req_t  *msg;
    switch_iter_t                   sit;
    vtss_isid_t                     isid;
    port_iter_t                     pit;
    u32                             temp_port_mask;
    int                             i;

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
            IPMC_CRIT_ENTER();
            VTSS_PORT_BF_CLR(routerport_setting.member_ports);
            switch ( ipmc_version ) {
            case IPMC_IP_VERSION_IGMP:
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    i = pit.iport;

                    if (ipmc_global.ipv4_conf.ipmc_router_ports[isid - VTSS_ISID_START][i]) {
                        VTSS_PORT_BF_SET(routerport_setting.member_ports, i, TRUE);
                    }
                }

                break;
            case IPMC_IP_VERSION_MLD:
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    i = pit.iport;

                    if (ipmc_global.ipv6_conf.ipmc_router_ports[isid - VTSS_ISID_START][i]) {
                        VTSS_PORT_BF_SET(routerport_setting.member_ports, i, TRUE);
                    }
                }

                break;
            default:
                break;
            }

            vtss_ipmc_set_static_router_ports(&routerport_setting, ipmc_version);
            IPMC_CRIT_EXIT();

            continue;
        }

        ipmc_msg_alloc(isid, IPMC_MSG_ID_ROUTER_PORT_SET_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_router_port_set_req_t *)buf.msg;

        IPMC_CRIT_ENTER();
        switch ( ipmc_version ) {
        case IPMC_IP_VERSION_IGMP:
            memcpy(msg->ipmc_router_ports, ipmc_global.ipv4_conf.ipmc_router_ports[isid - VTSS_ISID_START], VTSS_PORT_ARRAY_SIZE * sizeof(BOOL));

            break;
        case IPMC_IP_VERSION_MLD:
            memcpy(msg->ipmc_router_ports, ipmc_global.ipv6_conf.ipmc_router_ports[isid - VTSS_ISID_START], VTSS_PORT_ARRAY_SIZE * sizeof(BOOL));

            break;
        default:
            memset(msg->ipmc_router_ports, 0x0, VTSS_PORT_ARRAY_SIZE * sizeof(BOOL));

            break;
        }
        IPMC_CRIT_EXIT();

        temp_port_mask = 0;
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            if (msg->ipmc_router_ports[i]) {
                temp_port_mask |= (1 << i);
            }
        }

        msg->msg_id = IPMC_MSG_ID_ROUTER_PORT_SET_REQ;
        msg->isid = isid;
        msg->version = ipmc_version;
        ipmc_msg_tx(&buf, isid, sizeof(*msg));

        T_D("(TX) (ISID = %d)temp_port_mask = 0x%x", isid_add, temp_port_mask);
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Set STACK IPMC FAST LEAVE PORT */
static ipmc_port_bfs_t  fastleave_setting;
static vtss_rc ipmc_stacking_set_fastleave_port(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version)
{
    ipmc_msg_buf_t                      buf;
    ipmc_msg_fast_leave_port_set_req_t  *msg;
    switch_iter_t                       sit;
    vtss_isid_t                         isid;
    port_iter_t                         pit;
    ulong                               temp_port_mask;
    int                                 i;

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
            IPMC_CRIT_ENTER();
            VTSS_PORT_BF_CLR(fastleave_setting.member_ports);
            switch ( ipmc_version ) {
            case IPMC_IP_VERSION_IGMP:
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    i = pit.iport;

                    if (ipmc_global.ipv4_conf.ipmc_fast_leave_ports[isid - VTSS_ISID_START][i]) {
                        VTSS_PORT_BF_SET(fastleave_setting.member_ports, i, TRUE);
                    }
                }

                break;
            case IPMC_IP_VERSION_MLD:
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    i = pit.iport;

                    if (ipmc_global.ipv6_conf.ipmc_fast_leave_ports[isid - VTSS_ISID_START][i]) {
                        VTSS_PORT_BF_SET(fastleave_setting.member_ports, i, TRUE);
                    }
                }

                break;
            default:
                break;
            }

            vtss_ipmc_set_static_fast_leave_ports(&fastleave_setting, ipmc_version);
            IPMC_CRIT_EXIT();

            continue;
        }

        ipmc_msg_alloc(isid, IPMC_MSG_ID_FAST_LEAVE_PORT_SET_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_fast_leave_port_set_req_t *)buf.msg;

        IPMC_CRIT_ENTER();
        switch ( ipmc_version ) {
        case IPMC_IP_VERSION_IGMP:
            memcpy(msg->ipmc_fast_leave_ports, ipmc_global.ipv4_conf.ipmc_fast_leave_ports[isid - VTSS_ISID_START], VTSS_PORT_ARRAY_SIZE * sizeof(BOOL));

            break;
        case IPMC_IP_VERSION_MLD:
            memcpy(msg->ipmc_fast_leave_ports, ipmc_global.ipv6_conf.ipmc_fast_leave_ports[isid - VTSS_ISID_START], VTSS_PORT_ARRAY_SIZE * sizeof(BOOL));

            break;
        default:
            memset(msg->ipmc_fast_leave_ports, 0x0, VTSS_PORT_ARRAY_SIZE * sizeof(BOOL));

            break;
        }
        IPMC_CRIT_EXIT();

        temp_port_mask = 0;
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            i = pit.iport;

            if (msg->ipmc_fast_leave_ports[i]) {
                temp_port_mask |= (1 << i);
            }
        }

        msg->msg_id = IPMC_MSG_ID_FAST_LEAVE_PORT_SET_REQ;
        msg->isid = isid;
        msg->version = ipmc_version;
        ipmc_msg_tx(&buf, isid, sizeof(*msg));

        T_D("(TX) (ISID = %d)temp_port_mask = 0x%x", isid_add, temp_port_mask);
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Set STACK IPMC Throttling Max NO. */
static ipmc_port_throttling_t   throttling_setting;
static vtss_rc ipmc_stacking_set_throttling_number(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version)
{
    ipmc_msg_buf_t                          buf;
    ipmc_msg_throllting_max_no_set_req_t    *msg;
    switch_iter_t                           sit;
    vtss_isid_t                             isid;

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
            port_iter_t pit;

            IPMC_CRIT_ENTER();
            memset(&throttling_setting, 0x0, sizeof(ipmc_port_throttling_t));
            switch ( ipmc_version ) {
            case IPMC_IP_VERSION_IGMP:
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    throttling_setting.max_no[pit.iport] = ipmc_global.ipv4_conf.ipmc_throttling_max_no[isid - VTSS_ISID_START][pit.iport];
                }

                break;
            case IPMC_IP_VERSION_MLD:
                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    throttling_setting.max_no[pit.iport] = ipmc_global.ipv6_conf.ipmc_throttling_max_no[isid - VTSS_ISID_START][pit.iport];
                }

                break;
            default:
                break;
            }

            vtss_ipmc_set_static_port_throttling_max_no(&throttling_setting, ipmc_version);
            IPMC_CRIT_EXIT();

            continue;
        }

        ipmc_msg_alloc(isid, IPMC_MSG_ID_THROLLTING_MAX_NO_SET_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_throllting_max_no_set_req_t *)buf.msg;

        IPMC_CRIT_ENTER();
        switch ( ipmc_version ) {
        case IPMC_IP_VERSION_IGMP:
            memcpy(msg->ipmc_throttling_max_no, ipmc_global.ipv4_conf.ipmc_throttling_max_no[isid - VTSS_ISID_START], VTSS_PORT_ARRAY_SIZE * sizeof(int));

            break;
        case IPMC_IP_VERSION_MLD:
            memcpy(msg->ipmc_throttling_max_no, ipmc_global.ipv6_conf.ipmc_throttling_max_no[isid - VTSS_ISID_START], VTSS_PORT_ARRAY_SIZE * sizeof(int));

            break;
        default:
            memset(msg->ipmc_throttling_max_no, 0x0, VTSS_PORT_ARRAY_SIZE * sizeof(int));

            break;
        }
        IPMC_CRIT_EXIT();

        msg->msg_id = IPMC_MSG_ID_THROLLTING_MAX_NO_SET_REQ;
        msg->isid = isid;
        msg->version = ipmc_version;
        ipmc_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Set STACK IPMC port group filtering */
static ipmc_port_group_filtering_t  grpfilter_setting;
static vtss_rc ipmc_stacking_set_grp_filtering(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version)
{
    int                                     i;
    ipmc_msg_buf_t                          buf;
    ipmc_msg_port_group_filtering_set_req_t *msg;
    switch_iter_t                           sit;
    vtss_isid_t                             isid;
    port_iter_t                             pit;
    ipmc_configuration_t                    *conf;

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
            IPMC_CRIT_ENTER();
            memset(&grpfilter_setting, 0x0, sizeof(ipmc_port_group_filtering_t));
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                conf = &ipmc_global.ipv4_conf;
            } else {
                conf = &ipmc_global.ipv6_conf;
            }

            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                i = pit.iport;
                grpfilter_setting.profile_index[i] = conf->ipmc_port_group_filtering[isid - VTSS_ISID_START][i];
            }

            vtss_ipmc_set_static_port_group_filtering(&grpfilter_setting, ipmc_version);
            IPMC_CRIT_EXIT();

            continue;
        }

        ipmc_msg_alloc(isid, IPMC_MSG_ID_PORT_GROUP_FILTERING_SET_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_port_group_filtering_set_req_t *)buf.msg;

        IPMC_CRIT_ENTER();
        if (ipmc_version == IPMC_IP_VERSION_IGMP) {
            conf = &ipmc_global.ipv4_conf;
        } else {
            conf = &ipmc_global.ipv6_conf;
        }

        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            i = pit.iport;
            msg->ipmc_port_group_filtering[i] = conf->ipmc_port_group_filtering[isid - VTSS_ISID_START][i];
        }
        IPMC_CRIT_EXIT();

        msg->msg_id = IPMC_MSG_ID_PORT_GROUP_FILTERING_SET_REQ;
        msg->isid = isid;
        msg->version = ipmc_version;
        ipmc_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Set STACK IPMC UNREG FLOOD */
static BOOL flood_setting;
static vtss_rc ipmc_stacking_set_unreg_flood(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version)
{
    ipmc_msg_buf_t                  buf;
    ipmc_msg_unreg_flood_set_req_t  *msg;
    switch_iter_t                   sit;
    vtss_isid_t                     isid;

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
            IPMC_CRIT_ENTER();
            switch ( ipmc_version ) {
            case IPMC_IP_VERSION_IGMP:
                flood_setting = ipmc_global.ipv4_conf.global.ipmc_unreg_flood_enabled;

                break;
            case IPMC_IP_VERSION_MLD:
                flood_setting = ipmc_global.ipv6_conf.global.ipmc_unreg_flood_enabled;

                break;
            default:
                flood_setting = FALSE;

                break;
            }

            vtss_ipmc_set_unreg_flood(flood_setting, ipmc_version);
            IPMC_CRIT_EXIT();

            continue;
        }

        ipmc_msg_alloc(isid, IPMC_MSG_ID_UNREG_FLOOD_SET_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_unreg_flood_set_req_t *)buf.msg;

        IPMC_CRIT_ENTER();
        switch ( ipmc_version ) {
        case IPMC_IP_VERSION_IGMP:
            msg->ipmc_unreg_flood_enabled = ipmc_global.ipv4_conf.global.ipmc_unreg_flood_enabled;

            break;
        case IPMC_IP_VERSION_MLD:
            msg->ipmc_unreg_flood_enabled = ipmc_global.ipv6_conf.global.ipmc_unreg_flood_enabled;

            break;
        default:
            msg->ipmc_unreg_flood_enabled = FALSE;

            break;
        }
        IPMC_CRIT_EXIT();

        msg->msg_id = IPMC_MSG_ID_UNREG_FLOOD_SET_REQ;
        msg->version = ipmc_version;
        ipmc_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

/* Send request and wait for response */
static cyg_flag_value_t ipmc_stacking_send_req(vtss_isid_t isid, ipmc_ip_version_t ipmc_version,
                                               vtss_vid_t vid, vtss_ipv6_t *group_addr,
                                               ipmc_msg_id_t msg_id,
                                               BOOL type, vtss_ipv6_t *srclist)
{
    cyg_flag_t                      *flags;
    ipmc_msg_vlan_entry_get_req_t   *msg_vlan;
    ipmc_msg_dyn_rtpt_get_req_t     *msg_rtpt;
    ipmc_msg_buf_t                  buf;
    cyg_flag_value_t                flag, retVal;
    cyg_tick_count_t                time_wait;

    T_D("enter(isid: %d)", isid);

    retVal = 0;
    flag = (1 << isid);
    ipmc_msg_alloc(isid, msg_id, &buf, TRUE, TRUE);

    if (msg_id != IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REQ) {
        msg_vlan = (ipmc_msg_vlan_entry_get_req_t *)buf.msg;
        msg_vlan->msg_id = msg_id;
        msg_vlan->version = ipmc_version;

        /* Default: IPMC_MSG_ID_VLAN_ENTRY_GET_REQ */
        msg_vlan->vid = vid;
        if (msg_id == IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ) {
            if (group_addr) {
                memcpy(&msg_vlan->group_addr, group_addr, sizeof(vtss_ipv6_t));
            }
        }
        if (msg_id == IPMC_MSG_ID_GROUP_SRCLIST_WALK_REQ) {
            if (group_addr) {
                memcpy(&msg_vlan->group_addr, group_addr, sizeof(vtss_ipv6_t));
            }
            if (srclist) {
                memcpy(&msg_vlan->srclist_addr, srclist, sizeof(vtss_ipv6_t));
            }
            msg_vlan->srclist_type = type;
        }

        IPMC_GET_CRIT_ENTER();
        flags = &ipmc_global.vlan_entry_flags;
        cyg_flag_maskbits(flags, ~flag);
        IPMC_GET_CRIT_EXIT();

        ipmc_msg_tx(&buf, isid, sizeof(*msg_vlan));
    } else {
        msg_rtpt = (ipmc_msg_dyn_rtpt_get_req_t *)buf.msg;
        msg_rtpt->msg_id = msg_id;
        msg_rtpt->version = ipmc_version;

        IPMC_GET_CRIT_ENTER();
        flags = &ipmc_global.dynamic_router_ports_getting_flags;
        cyg_flag_maskbits(flags, ~flag);
        IPMC_GET_CRIT_EXIT();

        ipmc_msg_tx(&buf, isid, sizeof(*msg_rtpt));
    }

    time_wait = cyg_current_time() + VTSS_OS_MSEC2TICK(IPMC_REQ_TIMEOUT);
    IPMC_GET_CRIT_ENTER();
    if (!(cyg_flag_timed_wait(flags, flag, CYG_FLAG_WAITMODE_OR, time_wait) & flag)) {
        retVal = 1;
    }
    IPMC_GET_CRIT_EXIT();

    return retVal;
}

static vtss_rc ipmc_stacking_set_intf_entry(vtss_isid_t isid_add, ipmc_prot_intf_entry_param_t *intf_param, ipmc_ip_version_t ipmc_version)
{
    ipmc_msg_buf_t              buf;
    ipmc_msg_vlan_set_req_t     *msg;
    switch_iter_t               sit;
    vtss_isid_t                 isid;

    if (!msg_switch_is_master() || !intf_param) {
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
            ipmc_prot_intf_basic_t  vlan_entry;

            memset(&vlan_entry, 0x0, sizeof(ipmc_prot_intf_basic_t));
            vlan_entry.valid = TRUE;
            vlan_entry.vid = intf_param->vid;
            vlan_entry.compatibility = intf_param->cfg_compatibility;
            vlan_entry.priority = intf_param->priority;
            vlan_entry.querier4_address = intf_param->querier.QuerierAdrs4;
            vlan_entry.robustness_variable = intf_param->querier.RobustVari;
            vlan_entry.query_interval = intf_param->querier.QueryIntvl;
            vlan_entry.query_response_interval = intf_param->querier.MaxResTime;
            vlan_entry.last_listener_query_interval = intf_param->querier.LastQryItv;
            vlan_entry.unsolicited_report_interval = intf_param->querier.UnsolicitR;

            IPMC_CRIT_ENTER();
            if (!vtss_ipmc_upd_intf_entry(&vlan_entry, ipmc_version)) {
                T_D("IPMC_MSG_ID_VLAN_ENTRY_SET_REQ Failed!");
            }
            IPMC_CRIT_EXIT();
        } else {
            ipmc_msg_alloc(isid, IPMC_MSG_ID_VLAN_ENTRY_SET_REQ, &buf, FALSE, TRUE);
            msg = (ipmc_msg_vlan_set_req_t *)buf.msg;

            msg->msg_id = IPMC_MSG_ID_VLAN_ENTRY_SET_REQ;
            msg->version = ipmc_version;
            msg->vlan_entry.valid = TRUE;
            msg->vlan_entry.vid = intf_param->vid;
            msg->vlan_entry.compatibility = intf_param->cfg_compatibility;
            msg->vlan_entry.priority = intf_param->priority;
            msg->vlan_entry.querier4_address = intf_param->querier.QuerierAdrs4;
            msg->vlan_entry.robustness_variable = intf_param->querier.RobustVari;
            msg->vlan_entry.query_interval = intf_param->querier.QueryIntvl;
            msg->vlan_entry.query_response_interval = intf_param->querier.MaxResTime;
            msg->vlan_entry.last_listener_query_interval = intf_param->querier.LastQryItv;
            msg->vlan_entry.unsolicited_report_interval = intf_param->querier.UnsolicitR;
            ipmc_msg_tx(&buf, isid, sizeof(*msg));
        }
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

static ipmc_conf_intf_entry_t   cur_intf_g_s;
static vtss_rc ipmc_stacking_get_intf_entry(vtss_isid_t isid_add, ushort vid, ipmc_prot_intf_entry_param_t *intf_param, ipmc_ip_version_t ipmc_version)
{
    vtss_ipv6_t         zero_addr;
    cyg_tick_count_t    exe_time_base;

    if (!msg_switch_is_master() || !intf_param) {
        return VTSS_RC_ERROR;
    }
    exe_time_base = cyg_current_time();

    T_D("enter, isid: %d, vid_no:%d/ver:%d", isid_add, vid, ipmc_version);

    if ((isid_add == VTSS_ISID_GLOBAL) ||
        (isid_add == VTSS_ISID_LOCAL) ||
        ipmc_lib_isid_is_local(isid_add)) {
        ipmc_intf_entry_t   *entry;
        u16                 intf_idx;

        IPMC_GET_CRIT_ENTER();
        if ((entry = vtss_ipmc_get_intf_entry(vid, ipmc_version)) != NULL) {
            memcpy(intf_param, &entry->param, sizeof(ipmc_prot_intf_entry_param_t));
        } else {
            if ((intf_idx = ipmc_conf_intf_entry_get(vid, ipmc_version)) != IPMC_VID_INIT) {
                switch ( ipmc_version ) {
                case IPMC_IP_VERSION_IGMP:
                    memcpy(&cur_intf_g_s, &ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx], sizeof(ipmc_conf_intf_entry_t));

                    break;
                case IPMC_IP_VERSION_MLD:
                    memcpy(&cur_intf_g_s, &ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx], sizeof(ipmc_conf_intf_entry_t));

                    break;
                default:
                    IPMC_GET_CRIT_EXIT();
                    return VTSS_RC_ERROR;
                }

                memset(intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
                _ipmc_trans_cfg_intf_info(intf_param, &cur_intf_g_s);
            }
        }
        IPMC_GET_CRIT_EXIT();

        T_D("Done-Local(isid=%d)", isid_add);
    } else {
        if (!IPMC_LIB_ISID_EXIST(isid_add)) {
            return VTSS_RC_ERROR;
        }

        IPMC_GET_CRIT_ENTER();
        ipmc_lib_get_all_zero_ipv6_addr(&zero_addr);
        memset(&ipmc_global.interface_entry, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
        IPMC_GET_CRIT_EXIT();

        if (ipmc_stacking_send_req(isid_add, ipmc_version,
                                   vid, &zero_addr,
                                   IPMC_MSG_ID_VLAN_ENTRY_GET_REQ,
                                   FALSE, NULL)) {
            T_E("timeout, IPMC_MSG_ID_VLAN_ENTRY_GET_REQ(isid=%d)", isid_add);
            return VTSS_RC_ERROR;
        }

        IPMC_GET_CRIT_ENTER();
        memcpy(intf_param, &ipmc_global.interface_entry, sizeof(ipmc_prot_intf_entry_param_t));
        IPMC_GET_CRIT_EXIT();

        T_D("Done-Remote(isid=%d)", isid_add);
    }

    T_D("exit, isid: %d, vid_no: %d (%u-Ticks)", isid_add, vid, (ulong)(cyg_current_time() - exe_time_base));
    return VTSS_OK;
}

static vtss_rc ipmc_stacking_get_next_group_srclist_entry(vtss_isid_t isid_add, ipmc_ip_version_t ipmc_version, vtss_vid_t vid, vtss_ipv6_t *addr, ipmc_prot_group_srclist_t *srclist)
{
    char                buf[40];
    cyg_tick_count_t    exe_time_base;

    if (!msg_switch_is_master() || !addr || !srclist) {
        return VTSS_RC_ERROR;
    }
    exe_time_base = cyg_current_time();

    memset(buf, 0x0, sizeof(buf));
    T_D("enter->isid:%d, VID:%d/VER:%d, SRC=%s[%s]",
        isid_add,
        vid,
        ipmc_version,
        srclist->type ? "ALLOW" : "BLOCK",
        misc_ipv6_txt(&srclist->srclist.src_ip, buf));

    if ((isid_add == VTSS_ISID_GLOBAL) ||
        (isid_add == VTSS_ISID_LOCAL) ||
        ipmc_lib_isid_is_local(isid_add)) {
        ipmc_group_entry_t  grp;
        BOOL                grp_found;

        grp.ipmc_version = ipmc_version;
        grp.vid = vid;
        memcpy(&grp.group_addr, addr, sizeof(vtss_ipv6_t));

        grp_found = FALSE;
        IPMC_GET_CRIT_ENTER();
        if (vtss_ipmc_get_intf_group_entry(vid, &grp, ipmc_version)) {
            grp_found = TRUE;
        }
        IPMC_GET_CRIT_EXIT();

        srclist->valid = FALSE;
        if (grp_found) {
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
                    /* srclist->type remains unchanged */
                    memcpy(&srclist->srclist, &entry, sizeof(ipmc_sfm_srclist_t));
                    srclist->valid = TRUE;
                }
            }
        }
    } else {
        if (!IPMC_LIB_ISID_EXIST(isid_add)) {
            return VTSS_RC_ERROR;
        }

        IPMC_GET_CRIT_ENTER();
        memset(&ipmc_global.group_srclist_entry, 0x0, sizeof(ipmc_prot_group_srclist_t));
        IPMC_GET_CRIT_EXIT();

        if (ipmc_stacking_send_req(isid_add, ipmc_version,
                                   vid, addr,
                                   IPMC_MSG_ID_GROUP_SRCLIST_WALK_REQ,
                                   srclist->type, &srclist->srclist.src_ip)) {
            T_E("timeout, IPMC_MSG_ID_GROUP_SRCLIST_WALK_REQ(isid=%d)", isid_add);
            return VTSS_RC_ERROR;
        }

        IPMC_GET_CRIT_ENTER();
        memcpy(srclist, &ipmc_global.group_srclist_entry, sizeof(ipmc_prot_group_srclist_t));
        IPMC_GET_CRIT_EXIT();
    }

    T_D("exit->isid:%d, (%u-Ticks-GOT-%s)%s-vid:%d/source_address = %s",
        isid_add,
        (ulong)(cyg_current_time() - exe_time_base),
        srclist->valid ? "VALID" : "INVALID",
        srclist->type ? "ALLOW" : "BLOCK",
        vid,
        misc_ipv6_txt(&srclist->srclist.src_ip, buf));
    return VTSS_OK;
}

static vtss_rc ipmc_stacking_get_next_intf_group_entry(vtss_isid_t isid_add, vtss_vid_t vid, ipmc_prot_intf_group_entry_t *intf_group, ipmc_ip_version_t ipmc_version)
{
    char                buf[40];
    cyg_tick_count_t    exe_time_base;

    if (!msg_switch_is_master() || !intf_group) {
        return VTSS_RC_ERROR;
    }
    exe_time_base = cyg_current_time();

    memset(buf, 0x0, sizeof(buf));
    T_D("enter->isid: %d, vid_no:%d/ver:%d, group_address = %s",
        isid_add,
        vid,
        ipmc_version,
        misc_ipv6_txt(&intf_group->group_addr, buf));

    if ((isid_add == VTSS_ISID_GLOBAL) ||
        (isid_add == VTSS_ISID_LOCAL) ||
        ipmc_lib_isid_is_local(isid_add)) {
        ipmc_group_entry_t  grp;

        intf_group->valid = FALSE;
        grp.ipmc_version = ipmc_version;
        grp.vid = vid;
        memcpy(&grp.group_addr, &intf_group->group_addr, sizeof(vtss_ipv6_t));
        IPMC_GET_CRIT_ENTER();
        if (vtss_ipmc_get_next_intf_group_entry(vid, &grp, ipmc_version)) {
            intf_group->valid = TRUE;
        }
        IPMC_GET_CRIT_EXIT();

        if (intf_group->valid) {
            intf_group->ipmc_version = grp.ipmc_version;
            intf_group->vid = grp.vid;
            memcpy(&intf_group->group_addr, &grp.group_addr, sizeof(vtss_ipv6_t));
            memcpy(&intf_group->db, &grp.info->db, sizeof(ipmc_group_db_t));

            if (!msg_switch_is_master()) {
                intf_group->db.ipmc_sf_do_forward_srclist = NULL;
                intf_group->db.ipmc_sf_do_not_forward_srclist = NULL;
            }
        } else {
            T_D("no more group entry in vlan-%d", vid);
        }
    } else {
        if (!IPMC_LIB_ISID_EXIST(isid_add)) {
            return VTSS_RC_ERROR;
        }

        IPMC_GET_CRIT_ENTER();
        memset(&ipmc_global.intf_group_entry, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
        IPMC_GET_CRIT_EXIT();

        if (ipmc_stacking_send_req(isid_add, ipmc_version,
                                   vid, &intf_group->group_addr,
                                   IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ,
                                   FALSE, NULL)) {
            T_E("timeout, IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ(isid=%d)", isid_add);
            return VTSS_RC_ERROR;
        }

        IPMC_GET_CRIT_ENTER();
        memcpy(intf_group, &ipmc_global.intf_group_entry, sizeof(ipmc_prot_intf_group_entry_t));
        IPMC_GET_CRIT_EXIT();
    }

    T_D("exit->isid: %d, (%u-Ticks-GOT)%s-vid:%d/group_address = %s",
        isid_add,
        (ulong)(cyg_current_time() - exe_time_base),
        intf_group->valid ? "VALID" : "INVALID",
        vid,
        misc_ipv6_txt(&intf_group->group_addr, buf));
    return VTSS_OK;
}

static vtss_rc ipmc_stacking_get_dynamic_router_port(vtss_isid_t isid_add, ipmc_dynamic_router_port_t *router_port, ipmc_ip_version_t ipmc_version)
{
    port_iter_t         pit;
    u32                 idx;
    BOOL                stacking;
    ipmc_port_bfs_t     router_port_bitmask;
    vtss_ipv6_t         zero_addr;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!msg_switch_is_master() || !router_port) {
        return VTSS_RC_ERROR;
    }

    T_D("enter, isid: %d", isid_add);

    VTSS_PORT_BF_CLR(router_port_bitmask.member_ports);
    if ((isid_add == VTSS_ISID_GLOBAL) ||
        (isid_add == VTSS_ISID_LOCAL) ||
        ipmc_lib_isid_is_local(isid_add)) {
        stacking = FALSE;

        IPMC_GET_CRIT_ENTER();
        ipmc_lib_get_discovered_router_port_mask(ipmc_version, &router_port_bitmask);
        IPMC_GET_CRIT_EXIT();
    } else {
        if (!IPMC_LIB_ISID_EXIST(isid_add)) {
            return VTSS_RC_ERROR;
        }

        stacking = TRUE;

        IPMC_GET_CRIT_ENTER();
        ipmc_lib_get_all_zero_ipv6_addr(&zero_addr);
        memset(ipmc_global.dynamic_router_ports, 0x0, sizeof(ipmc_global.dynamic_router_ports));
        IPMC_GET_CRIT_EXIT();

        if (ipmc_stacking_send_req(isid_add, ipmc_version,
                                   0, &zero_addr,
                                   IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REQ,
                                   FALSE, NULL)) {
            T_E("timeout, IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REQ(isid=%d)", isid_add);
            return VTSS_RC_ERROR;
        }
    }

    if (msg_switch_is_master()) {
        (void) port_iter_init(&pit, NULL, isid_add, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    } else {
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    }
    while (port_iter_getnext(&pit)) {
        idx = pit.iport;

        if (stacking) {
            IPMC_GET_CRIT_ENTER();
            router_port->ports[idx] = ipmc_global.dynamic_router_ports[isid_add - 1][idx];
            IPMC_GET_CRIT_EXIT();
        } else {
            if (VTSS_PORT_BF_GET(router_port_bitmask.member_ports, idx)) {
                router_port->ports[idx] = TRUE;
            } else {
                router_port->ports[idx] = FALSE;
            }
        }
    }

    T_D("exit, isid: %d (%u-Ticks)", isid_add, (ulong)(cyg_current_time() - exe_time_base));
    return VTSS_OK;
}

/* STACK IPMC clear STAT Counter */
static vtss_rc ipmc_stacking_clear_statistics(vtss_isid_t isid_add,
                                              ipmc_ip_version_t ipmc_version,
                                              vtss_vid_t vid)
{
    ipmc_msg_buf_t                      buf;
    ipmc_msg_stat_counter_clear_req_t   *msg;
    switch_iter_t                       sit;
    vtss_isid_t                         isid;

    if (!msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    T_D("enter, isid: %d", isid_add);

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if ((isid_add != VTSS_ISID_GLOBAL) && (isid_add != isid)) {
            continue;
        }

        ipmc_msg_alloc(isid, IPMC_MSG_ID_STAT_COUNTER_CLEAR_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_stat_counter_clear_req_t *)buf.msg;

        msg->msg_id = IPMC_MSG_ID_STAT_COUNTER_CLEAR_REQ;
        msg->version = ipmc_version;
        msg->vid = vid;
        ipmc_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

static vtss_rc ipmc_stacking_sync_mgmt_conf(vtss_isid_t isid_add, ipmc_lib_mgmt_info_t *sys_mgmt)
{
    ipmc_msg_buf_t              buf;
    ipmc_msg_sys_mgmt_set_req_t *msg;
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

        ipmc_msg_alloc(isid, IPMC_MSG_ID_SYS_MGMT_SET_REQ, &buf, FALSE, TRUE);
        msg = (ipmc_msg_sys_mgmt_set_req_t *)buf.msg;

        msg->msg_id = IPMC_MSG_ID_SYS_MGMT_SET_REQ;
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
        ipmc_msg_tx(&buf, isid, sizeof(*msg));
    }

    T_D("exit, isid: %d", isid_add);
    return VTSS_OK;
}

static BOOL ipmc_port_status[VTSS_ISID_END][VTSS_PORT_ARRAY_SIZE];

/* Port state callback function  This function is called if a GLOBAL port state change occur.  */
static void ipmc_global_port_state_change_callback(vtss_isid_t isid, vtss_port_no_t port_no, port_info_t *info)
{
    T_D("enter: Port(%u/Stack:%s)->%s", port_no, info->stack ? "TRUE" : "FALSE", info->link ? "UP" : "DOWN");

    IPMC_CRIT_ENTER();
    if (msg_switch_is_master() && !info->stack) {
        if (info->link) {
            ipmc_port_status[ipmc_lib_isid_convert(TRUE, isid)][port_no] = TRUE;
        } else {
            ipmc_port_status[ipmc_lib_isid_convert(TRUE, isid)][port_no] = FALSE;
        }

        if (ipmc_lib_isid_is_local(isid)) {
            ipmc_port_status[VTSS_ISID_LOCAL][port_no] = ipmc_port_status[ipmc_lib_isid_convert(TRUE, isid)][port_no];
        }
    }
    IPMC_CRIT_EXIT();

    T_D("exit: Port(%u/Stack:%s)->%s", port_no, info->stack ? "TRUE" : "FALSE", info->link ? "UP" : "DOWN");
}

static vtss_rc ipmc_resolve_vlan_member(vtss_isid_t isid_in, vlan_mgmt_entry_t *vlan_conf, BOOL next)
{
    switch_iter_t   sit;
    vtss_isid_t     isid;
    vtss_vid_t      vid;

    if (!vlan_conf) {
        return VTSS_RC_ERROR;
    }

    /*
        Only msg_switch_is_master() will come here
        VTSS_ISID_GLOBAL cannot resolve VLAN member!
    */
    if (isid_in == VTSS_ISID_GLOBAL) {
        return VTSS_RC_ERROR;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;
        if (!IPMC_LIB_ISID_PASS(isid_in, isid)) {
            continue;
        }

        /* Expecting to retrieve all the information in vlan_mgmt_entry_t */
        vid = vlan_conf->vid;
#ifdef VTSS_SW_OPTION_MVR
        if (next) {
            vlan_mgmt_entry_t   chk_mvr;
            BOOL                found = FALSE;

            if (isid == VTSS_ISID_LOCAL) {
                while (vlan_mgmt_vlan_get(VTSS_ISID_LOCAL, vid, vlan_conf, TRUE, VLAN_USER_ALL) == VTSS_OK) {
                    vid = vlan_conf->vid;

                    chk_mvr.vid = vid;
                    if (vlan_mgmt_vlan_get(VTSS_ISID_LOCAL, vid, &chk_mvr, FALSE, VLAN_USER_MVR) != VTSS_OK) {
                        found = TRUE;
                        break;
                    }
                }
            } else {
                while (vlan_mgmt_vlan_get(isid, vid, vlan_conf, TRUE, VLAN_USER_STATIC) == VTSS_OK) {
                    vid = vlan_conf->vid;

                    chk_mvr.vid = vid;
                    if (vlan_mgmt_vlan_get(isid, vid, &chk_mvr, FALSE, VLAN_USER_MVR) != VTSS_OK) {
                        found = TRUE;
                        break;
                    }
                }
            }

            if (found) {
                return VTSS_OK;
            } else {
                return VTSS_RC_ERROR;
            }
        } else {
            if (isid == VTSS_ISID_LOCAL) {
                if (vlan_mgmt_vlan_get(VTSS_ISID_LOCAL, vid, vlan_conf, FALSE, VLAN_USER_MVR) == VTSS_OK) {
                    return VTSS_RC_ERROR;
                }

                return vlan_mgmt_vlan_get(VTSS_ISID_LOCAL, vid, vlan_conf, FALSE, VLAN_USER_ALL);
            } else {
                if (vlan_mgmt_vlan_get(isid, vid, vlan_conf, FALSE, VLAN_USER_MVR) == VTSS_OK) {
                    return VTSS_RC_ERROR;
                }

                return vlan_mgmt_vlan_get(isid, vid, vlan_conf, FALSE, VLAN_USER_STATIC);
            }
        }
#else
        if (isid == VTSS_ISID_LOCAL) {
            return vlan_mgmt_vlan_get(VTSS_ISID_LOCAL, vid, vlan_conf, next, VLAN_USER_ALL);
        } else {
            return vlan_mgmt_vlan_get(isid, vid, vlan_conf, next, VLAN_USER_STATIC);
        }
#endif /* VTSS_SW_OPTION_MVR */
    }

    return VTSS_RC_ERROR;
}

static void _ipmc_intf_walk_translate(ipmc_intf_map_t *entry, ipmc_intf_entry_t *intf)
{
    if (!entry || !intf) {
        return;
    }

    entry->ipmc_version = intf->ipmc_version;
    memcpy(&entry->param, &intf->param, sizeof(ipmc_prot_intf_entry_param_t));
    if (entry->ipmc_version == IPMC_IP_VERSION_MLD) {
        if (!ipmc_lib_get_eui64_linklocal_addr(&entry->QuerierAdrs.mld.adrs)) {
            IPMC_LIB_ADRS_SET(&entry->QuerierConf.mld.adrs, 0x0);
        }
    } else {
        if (!ipmc_lib_get_ipintf_igmp_adrs(intf, &entry->QuerierAdrs.igmp.adrs)) {
            entry->QuerierAdrs.igmp.adrs = 0;
        }
    }
    entry->op_state = intf->op_state;
    entry->proxy_report_timeout = intf->proxy_report_timeout;
    memcpy(entry->vlan_ports, intf->vlan_ports, sizeof(entry->vlan_ports));
}

BOOL ipmc_mgmt_intf_op_allow(vtss_isid_t isid)
{
    if (msg_switch_is_master() && ipmc_lib_isid_is_local(isid)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

vtss_rc ipmc_mgmt_intf_walk(BOOL next, ipmc_intf_map_t *entry)
{
    vtss_rc             rc;
    ipmc_intf_entry_t   *intf;

    if (!entry) {
        return VTSS_RC_ERROR;
    }

    rc = VTSS_RC_ERROR;
    IPMC_CRIT_ENTER();
    if (next) {
        if ((intf = vtss_ipmc_get_next_intf_entry(entry->param.vid, entry->ipmc_version)) != NULL) {
            _ipmc_intf_walk_translate(entry, intf);
            rc = VTSS_OK;
        }
    } else {
        if ((intf = vtss_ipmc_get_intf_entry(entry->param.vid, entry->ipmc_version)) != NULL) {
            _ipmc_intf_walk_translate(entry, intf);
            rc = VTSS_OK;
        }
    }
    IPMC_CRIT_EXIT();

    return rc;
}

vtss_rc ipmc_mgmt_set_intf_state_querier(ipmc_operation_action_t action,
                                         vtss_vid_t vid,
                                         BOOL *state_enable,
                                         BOOL *querier_enable,
                                         ipmc_ip_version_t ipmc_version)
{
    switch_iter_t           sit;
    vtss_isid_t             isid;
    port_iter_t             pit;
    vlan_mgmt_entry_t       vlan_conf;
    ipmc_port_bfs_t         vlan_ports;
    u16                     intf_idx;
    BOOL                    create_flag, apply_flag;
    ipmc_conf_intf_entry_t  cur_intf;
    cyg_tick_count_t        exe_time_base;

    if (!msg_switch_is_master()) {
        return VTSS_OK;
    }
    exe_time_base = cyg_current_time();

    IPMC_CRIT_ENTER();
    intf_idx = ipmc_conf_intf_entry_get(vid, ipmc_version);
    IPMC_CRIT_EXIT();

    create_flag = FALSE;
    if (action != IPMC_OP_SET) {
        if (intf_idx != IPMC_VID_INIT) {
            if (action == IPMC_OP_ADD) {
                return VTSS_RC_ERROR;
            }
        } else {
            if (action != IPMC_OP_ADD) {
                return VTSS_RC_ERROR;
            }

            create_flag = TRUE;
        }
    } else {
        if (intf_idx == IPMC_VID_INIT) {
            create_flag = TRUE;
        }
    }

    switch ( action ) {
    case IPMC_OP_ADD:
    case IPMC_OP_SET:
        if (create_flag) {
            IPMC_CRIT_ENTER();
            if ((intf_idx = _ipmc_conf_intf_entry_add(vid, ipmc_version)) == IPMC_VID_INIT) {
                IPMC_CRIT_EXIT();
                return IPMC_ERROR_TABLE_IS_FULL;
            }
            IPMC_CRIT_EXIT();
        }

        break;
    case IPMC_OP_DEL:
    case IPMC_OP_UPD:

        break;
    default:

        return VTSS_RC_ERROR;
    }

    if (intf_idx >= IPMC_VLAN_MAX) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        memcpy(&cur_intf, &ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx], sizeof(ipmc_conf_intf_entry_t));

        break;
    case IPMC_IP_VERSION_MLD:
        memcpy(&cur_intf, &ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx], sizeof(ipmc_conf_intf_entry_t));

        break;
    default:
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }
    IPMC_CRIT_EXIT();

    if ((action == IPMC_OP_UPD) ||
        ((action == IPMC_OP_SET) && !create_flag)) {
        if ((cur_intf.protocol_status == *state_enable) &&
            (cur_intf.querier_status == *querier_enable)) {
            return VTSS_OK;
        }
    }

    VTSS_PORT_BF_CLR(vlan_ports.member_ports);
    if (*state_enable) {
        vlan_conf.vid = vid;
        if (ipmc_resolve_vlan_member(VTSS_ISID_LOCAL, &vlan_conf, FALSE) == VTSS_OK) {
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                VTSS_PORT_BF_SET(vlan_ports.member_ports, pit.iport, vlan_conf.ports[pit.iport]);
            }
        }
    }

    if (action != IPMC_OP_DEL) {
        apply_flag = TRUE;
        cur_intf.valid = TRUE;
        cur_intf.vid = vid;
        cur_intf.protocol_status = *state_enable;
        cur_intf.querier_status = *querier_enable;

        IPMC_CRIT_ENTER();
        if (vtss_ipmc_set_intf_entry(vid, *state_enable, *querier_enable, &vlan_ports, ipmc_version) == NULL) {
            cur_intf.protocol_status = IPMC_DEF_INTF_STATE_VALUE;
            apply_flag = FALSE;
        }

        if (apply_flag) {
            (void) _ipmc_conf_intf_entry_upd(&cur_intf, ipmc_version);
        }
        IPMC_CRIT_EXIT();

        if (!apply_flag) {
            return IPMC_ERROR_TABLE_IS_FULL;
        }
    } else {
        IPMC_CRIT_ENTER();
        (void) vtss_ipmc_del_intf_entry(vid, ipmc_version);
        (void) _ipmc_conf_intf_entry_del(vid, ipmc_version);
        IPMC_CRIT_EXIT();
    }

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);

    /* apply this setting to stacking only */
    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG);
    while (switch_iter_getnext(&sit)) {
        isid = sit.isid;

        if (ipmc_lib_isid_is_local(isid)) {
            continue;
        } else {
            VTSS_PORT_BF_CLR(vlan_ports.member_ports);
            if (action != IPMC_OP_DEL) {
                if (*state_enable) {
                    vlan_conf.vid = vid;
                    if (ipmc_resolve_vlan_member(isid, &vlan_conf, FALSE) == VTSS_OK) {
                        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                        while (port_iter_getnext(&pit)) {
                            VTSS_PORT_BF_SET(vlan_ports.member_ports, pit.iport, vlan_conf.ports[pit.iport]);
                        }
                    }
                }

                (void) ipmc_stacking_set_intf(isid, IPMC_OP_SET, &cur_intf, &vlan_ports, ipmc_version);
            } else {
                (void) ipmc_stacking_set_intf(isid, IPMC_OP_DEL, &cur_intf, &vlan_ports, ipmc_version);
            }
        }
    }

    T_D("VID-%u consumes %u ticks", vid, (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_intf_state_querier(BOOL conf, vtss_vid_t *vid, u8 *state_enable, u8 *querier_enable, BOOL next, ipmc_ip_version_t ipmc_version)
{
    vtss_rc             rc;
    BOOL                found;
    vtss_vid_t          chk_vid, intf_idx;
    cyg_tick_count_t    exe_time_base;

    if (!vid || !state_enable || !querier_enable) {
        return VTSS_RC_ERROR;
    }

    found = FALSE;
    chk_vid = *vid;
    intf_idx = IPMC_VID_INIT;
    exe_time_base = cyg_current_time();

    IPMC_CRIT_ENTER();
    if (next) {
        if (conf) {
            if ((intf_idx = ipmc_conf_intf_entry_get_next(&chk_vid, ipmc_version)) != IPMC_VID_INIT) {
                found = TRUE;
            }
        } else {
            ipmc_intf_entry_t   *ipmc_vid_entry = NULL;

            while ((ipmc_vid_entry = vtss_ipmc_get_next_intf_entry(chk_vid, ipmc_version)) != NULL) {
                chk_vid = ipmc_vid_entry->param.vid;
                if (ipmc_vid_entry->op_state) {
                    break;
                }
            }

            if (ipmc_vid_entry != NULL) {
                found = TRUE;
                intf_idx = ipmc_conf_intf_entry_get(chk_vid, ipmc_version);
            }
        }
    } else {
        if (conf) {
            if ((intf_idx = ipmc_conf_intf_entry_get(chk_vid, ipmc_version)) != IPMC_VID_INIT) {
                found = TRUE;
            }
        } else {
            ipmc_intf_entry_t   *ipmc_vid_entry = vtss_ipmc_get_intf_entry(chk_vid, ipmc_version);

            if ((ipmc_vid_entry != NULL) && ipmc_vid_entry->op_state) {
                found = TRUE;
                intf_idx = ipmc_conf_intf_entry_get(chk_vid, ipmc_version);
            }
        }
    }

    if ((intf_idx >= IPMC_VLAN_MAX) || (intf_idx == IPMC_VID_INIT)) {
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    if (!found) {
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    } else {
        *vid = chk_vid;
    }

    rc = VTSS_OK;
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        *state_enable = ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx].protocol_status;
        *querier_enable = ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx].querier_status;

        break;
    case IPMC_IP_VERSION_MLD:
        *state_enable = ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx].protocol_status;
        *querier_enable = ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx].querier_status;

        break;
    default:
        rc = VTSS_RC_ERROR;

        break;
    }

    IPMC_CRIT_EXIT();

    T_D("Consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return rc;
}

vtss_rc ipmc_mgmt_set_intf_info(vtss_isid_t isid, ipmc_prot_intf_entry_param_t *intf_param, ipmc_ip_version_t ipmc_version)
{
    vtss_rc                 rc = VTSS_OK;
    u16                     intf_idx;
    ipmc_conf_intf_entry_t  *cur_intf;
    ipmc_prot_intf_basic_t  entry;
    cyg_tick_count_t        exe_time_base;

    if (!msg_switch_is_master()) {
        return rc;
    }
    exe_time_base = cyg_current_time();

    IPMC_CRIT_ENTER();

    if ((intf_idx = ipmc_conf_intf_entry_get(intf_param->vid, ipmc_version)) == IPMC_VID_INIT) {
        rc = VTSS_RC_ERROR;

        IPMC_CRIT_EXIT();
        return rc;
    }

    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        cur_intf = &ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx];

        break;
    case IPMC_IP_VERSION_MLD:
        if (intf_param->querier.QuerierAdrs4) {
            rc = VTSS_RC_ERROR;
            IPMC_CRIT_EXIT();
            return rc;
        }

        cur_intf = &ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx];

        break;
    default:
        rc = VTSS_RC_ERROR;
        IPMC_CRIT_EXIT();
        return rc;
    }

    IPMC_CRIT_EXIT();

    entry.valid = TRUE;
    entry.vid = intf_param->vid;
    entry.compatibility = intf_param->cfg_compatibility;
    entry.priority = intf_param->priority;
    entry.querier4_address = intf_param->querier.QuerierAdrs4;
    entry.robustness_variable = intf_param->querier.RobustVari;
    entry.query_interval = intf_param->querier.QueryIntvl;
    entry.query_response_interval = intf_param->querier.MaxResTime;
    entry.last_listener_query_interval = intf_param->querier.LastQryItv;
    entry.unsolicited_report_interval = intf_param->querier.UnsolicitR;

    if (ipmc_lib_isid_is_local(isid)) {
        IPMC_CRIT_ENTER();
        if (!vtss_ipmc_upd_intf_entry(&entry, ipmc_version)) {
            rc = VTSS_RC_ERROR;
            IPMC_CRIT_EXIT();
            return rc;
        }
        IPMC_CRIT_EXIT();
    } else {
        rc = ipmc_stacking_set_intf_entry(isid, intf_param, ipmc_version);
        if (rc != VTSS_OK) {
            return rc;
        }
    }

    IPMC_CRIT_ENTER();
    if ((cur_intf->compatibility == intf_param->cfg_compatibility) &&
        (cur_intf->priority == intf_param->priority) &&
        (cur_intf->querier4_address == intf_param->querier.QuerierAdrs4) &&
        (cur_intf->robustness_variable == intf_param->querier.RobustVari) &&
        (cur_intf->query_interval == intf_param->querier.QueryIntvl) &&
        (cur_intf->query_response_interval == intf_param->querier.MaxResTime) &&
        (cur_intf->last_listener_query_interval == intf_param->querier.LastQryItv) &&
        (cur_intf->unsolicited_report_interval == intf_param->querier.UnsolicitR)) {
        IPMC_CRIT_EXIT();
        return rc;
    }

    cur_intf->compatibility = entry.compatibility;
    cur_intf->priority = entry.priority;
    cur_intf->querier4_address = entry.querier4_address;
    cur_intf->robustness_variable = entry.robustness_variable;
    cur_intf->query_interval = entry.query_interval;
    cur_intf->query_response_interval = entry.query_response_interval;
    cur_intf->last_listener_query_interval = entry.last_listener_query_interval;
    cur_intf->unsolicited_report_interval = entry.unsolicited_report_interval;
    IPMC_CRIT_EXIT();

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);

    T_D("ipmc_mgmt_set_intf_info(%u) consumes ID%d:%u ticks", intf_idx, isid, (ulong)(cyg_current_time() - exe_time_base));

    return rc;
}

vtss_rc ipmc_mgmt_get_intf_info(vtss_isid_t isid, vtss_vid_t vid, ipmc_prot_intf_entry_param_t *intf_param, ipmc_ip_version_t ipmc_version)
{
    if (!intf_param) {
        return VTSS_RC_ERROR;
    }

    memset(intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
    if ((isid == VTSS_ISID_GLOBAL) || ipmc_lib_isid_is_local(isid)) {
        ipmc_conf_intf_entry_t  *cur_intf_get, *intf_get_bak;
        ipmc_intf_entry_t       *entry;
        u16                     intf_idx;

        if (!IPMC_MEM_SYSTEM_MTAKE(cur_intf_get, sizeof(ipmc_conf_intf_entry_t))) {
            return VTSS_RC_ERROR;
        }
        intf_get_bak = cur_intf_get;

        IPMC_GET_CRIT_ENTER();
        if ((entry = vtss_ipmc_get_intf_entry(vid, ipmc_version)) != NULL) {
            memcpy(intf_param, &entry->param, sizeof(ipmc_prot_intf_entry_param_t));
        } else {
            if ((intf_idx = ipmc_conf_intf_entry_get(vid, ipmc_version)) != IPMC_VID_INIT) {
                switch ( ipmc_version ) {
                case IPMC_IP_VERSION_IGMP:
                    memcpy(cur_intf_get, &ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx], sizeof(ipmc_conf_intf_entry_t));

                    break;
                case IPMC_IP_VERSION_MLD:
                    memcpy(cur_intf_get, &ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx], sizeof(ipmc_conf_intf_entry_t));

                    break;
                default:
                    IPMC_GET_CRIT_EXIT();
                    IPMC_MEM_SYSTEM_MGIVE(intf_get_bak);
                    return VTSS_RC_ERROR;
                }

                _ipmc_trans_cfg_intf_info(intf_param, cur_intf_get);
            }
        }
        IPMC_GET_CRIT_EXIT();
        IPMC_MEM_SYSTEM_MGIVE(intf_get_bak);
    } else {
        if (ipmc_stacking_get_intf_entry(isid, vid, intf_param, ipmc_version) != VTSS_OK) {
            return VTSS_RC_ERROR;
        }
    }

    if (intf_param->vid == 0) {
        return VTSS_RC_ERROR;
    }

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_next_group_srclist_info(vtss_isid_t isid, ipmc_ip_version_t ipmc_version, vtss_vid_t vid, vtss_ipv6_t *addr, ipmc_prot_group_srclist_t *group_srclist_entry)
{
    vtss_rc rc = VTSS_OK;

    if (!addr || !group_srclist_entry) {
        return VTSS_RC_ERROR;
    }

    if ((rc = ipmc_stacking_get_next_group_srclist_entry(isid, ipmc_version, vid, addr, group_srclist_entry)) == VTSS_OK) {
        if (group_srclist_entry->valid == FALSE) {
            rc = VTSS_RC_ERROR;
        }
    }

    return rc;
}

vtss_rc ipmc_mgmt_get_next_intf_group_info(vtss_isid_t isid, vtss_vid_t vid, ipmc_prot_intf_group_entry_t *intf_group_entry, ipmc_ip_version_t ipmc_version)
{
    vtss_rc rc = VTSS_OK;

    if (!intf_group_entry) {
        return VTSS_RC_ERROR;
    }

    if ((rc = ipmc_stacking_get_next_intf_group_entry(isid, vid, intf_group_entry, ipmc_version)) == VTSS_OK) {
        if (intf_group_entry->valid == FALSE) {
            rc = VTSS_RC_ERROR;
        }
    }

    return rc;
}

vtss_rc ipmc_mgmt_set_router_port(vtss_isid_t isid, ipmc_conf_router_port_t *router_port, ipmc_ip_version_t ipmc_version)
{
    port_iter_t         pit;
    BOOL                apply_flag = FALSE, *router;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!router_port) {
        return VTSS_RC_ERROR;
    }

    if (!msg_switch_is_master()) {
        return VTSS_OK;
    }

    IPMC_CRIT_ENTER();
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        router = ipmc_global.ipv4_conf.ipmc_router_ports[isid - VTSS_ISID_START];
    } else if (ipmc_version == IPMC_IP_VERSION_MLD) {
        router = ipmc_global.ipv6_conf.ipmc_router_ports[isid - VTSS_ISID_START];
    } else {
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        if (router_port->ports[pit.iport] != router[pit.iport]) {
            apply_flag = TRUE;
            break;
        }
    }

    if (!apply_flag && ipmc_lib_isid_is_local(isid)) {
        IPMC_CRIT_EXIT();
        return VTSS_OK;
    }

    if (apply_flag) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            router[pit.iport] = router_port->ports[pit.iport];
        }
    }
    IPMC_CRIT_EXIT();

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);
    (void) ipmc_stacking_set_router_port(isid, ipmc_version);

    T_D("ipmc_mgmt_set_router_port consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_static_router_port(vtss_isid_t isid, ipmc_conf_router_port_t *router_port, ipmc_ip_version_t ipmc_version)
{
    port_iter_t         pit;
    BOOL                *router;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!router_port) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        router = ipmc_global.ipv4_conf.ipmc_router_ports[isid - VTSS_ISID_START];
    } else if (ipmc_version == IPMC_IP_VERSION_MLD) {
        router = ipmc_global.ipv6_conf.ipmc_router_ports[isid - VTSS_ISID_START];
    } else {
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    if (msg_switch_is_master()) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    } else {
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    }
    while (port_iter_getnext(&pit)) {
        router_port->ports[pit.iport] = router[pit.iport];
    }
    IPMC_CRIT_EXIT();

    T_D("ipmc_mgmt_get_static_router_port consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_dynamic_router_ports(vtss_isid_t isid, ipmc_dynamic_router_port_t *router_port, ipmc_ip_version_t ipmc_version)
{
    vtss_rc rc;

    if (!router_port) {
        return VTSS_RC_ERROR;
    }

    rc = ipmc_stacking_get_dynamic_router_port(isid, router_port, ipmc_version);

    return rc;
}

vtss_rc ipmc_mgmt_set_fast_leave_port(vtss_isid_t isid, ipmc_conf_fast_leave_port_t *fast_leave_port, ipmc_ip_version_t ipmc_version)
{
    port_iter_t         pit;
    BOOL                apply_flag = FALSE, *fast_leave;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!fast_leave_port) {
        return VTSS_RC_ERROR;
    }

    if (!msg_switch_is_master()) {
        return VTSS_OK;
    }

    IPMC_CRIT_ENTER();
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        fast_leave = ipmc_global.ipv4_conf.ipmc_fast_leave_ports[isid - VTSS_ISID_START];
    } else if (ipmc_version == IPMC_IP_VERSION_MLD) {
        fast_leave = ipmc_global.ipv6_conf.ipmc_fast_leave_ports[isid - VTSS_ISID_START];
    } else {
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        if (fast_leave_port->ports[pit.iport] != fast_leave[pit.iport]) {
            apply_flag = TRUE;
            break;
        }
    }

    if (!apply_flag && ipmc_lib_isid_is_local(isid)) {
        IPMC_CRIT_EXIT();
        return VTSS_OK;
    }

    if (apply_flag) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            fast_leave[pit.iport] = fast_leave_port->ports[pit.iport];
        }
    }
    IPMC_CRIT_EXIT();

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);
    (void) ipmc_stacking_set_fastleave_port(isid, ipmc_version);

    T_D("ipmc_mgmt_set_fast_leave_port consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_fast_leave_port(vtss_isid_t isid, ipmc_conf_fast_leave_port_t *fast_leave_port, ipmc_ip_version_t ipmc_version)
{
    port_iter_t         pit;
    BOOL                *fast_leave;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!fast_leave_port) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        fast_leave = ipmc_global.ipv4_conf.ipmc_fast_leave_ports[isid - VTSS_ISID_START];
    } else if (ipmc_version == IPMC_IP_VERSION_MLD) {
        fast_leave = ipmc_global.ipv6_conf.ipmc_fast_leave_ports[isid - VTSS_ISID_START];
    } else {
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    if (msg_switch_is_master()) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    } else {
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    }
    while (port_iter_getnext(&pit)) {
        fast_leave_port->ports[pit.iport] = fast_leave[pit.iport];
    }
    IPMC_CRIT_EXIT();

    T_D("ipmc_mgmt_get_fast_leave_port consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_set_throttling_max_count(vtss_isid_t isid, ipmc_conf_throttling_max_no_t *ipmc_throttling_max_no, ipmc_ip_version_t ipmc_version)
{
    int                 *throttling_number;
    port_iter_t         pit;
    BOOL                apply_flag = FALSE;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!ipmc_throttling_max_no) {
        return VTSS_RC_ERROR;
    }

    if (!msg_switch_is_master()) {
        return VTSS_OK;
    }

    IPMC_CRIT_ENTER();
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        throttling_number = ipmc_global.ipv4_conf.ipmc_throttling_max_no[isid - VTSS_ISID_START];
    } else if (ipmc_version == IPMC_IP_VERSION_MLD) {
        throttling_number = ipmc_global.ipv6_conf.ipmc_throttling_max_no[isid - VTSS_ISID_START];
    } else {
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        if (ipmc_throttling_max_no->ports[pit.iport] != throttling_number[pit.iport]) {
            apply_flag = TRUE;
            break;
        }
    }

    if (!apply_flag && ipmc_lib_isid_is_local(isid)) {
        IPMC_CRIT_EXIT();
        return VTSS_OK;
    }

    if (apply_flag) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            throttling_number[pit.iport] = ipmc_throttling_max_no->ports[pit.iport];
        }
    }
    IPMC_CRIT_EXIT();

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);
    (void) ipmc_stacking_set_throttling_number(isid, ipmc_version);

    T_D("ipmc_mgmt_set_throttling_max_count consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_throttling_max_count(vtss_isid_t isid, ipmc_conf_throttling_max_no_t *ipmc_throttling_max_no, ipmc_ip_version_t ipmc_version)
{
    int                 *throttling_number;
    port_iter_t         pit;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!ipmc_throttling_max_no) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        throttling_number = ipmc_global.ipv4_conf.ipmc_throttling_max_no[isid - VTSS_ISID_START];
    } else if (ipmc_version == IPMC_IP_VERSION_MLD) {
        throttling_number = ipmc_global.ipv6_conf.ipmc_throttling_max_no[isid - VTSS_ISID_START];
    } else {
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    if (msg_switch_is_master()) {
        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    } else {
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    }
    while (port_iter_getnext(&pit)) {
        ipmc_throttling_max_no->ports[pit.iport] = throttling_number[pit.iport];
    }
    IPMC_CRIT_EXIT();

    T_D("ipmc_mgmt_get_throttling_max_count consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_set_port_group_filtering(vtss_isid_t isid, ipmc_conf_port_group_filtering_t *ipmc_port_group_filtering, ipmc_ip_version_t ipmc_version)
{
    int                         str_len;
    u32                         pdx;
    ipmc_configuration_t        *conf;
    cyg_tick_count_t            exe_time_base;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *profile;
    ipmc_lib_profile_t          *data;

    if (!ipmc_port_group_filtering) {
        return VTSS_RC_ERROR;
    }

    if (!msg_switch_is_master()) {
        return VTSS_OK;
    }

    if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return VTSS_RC_ERROR;
    }

    exe_time_base = cyg_current_time();
    profile = &pfm->profile;
    data = &profile->data;

    memset(profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
    memcpy(data->name, &ipmc_port_group_filtering->addr.profile.name[0], VTSS_IPMC_NAME_MAX_LEN);
    if (ipmc_lib_mgmt_fltr_profile_get(profile, TRUE) != VTSS_OK) {
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return VTSS_RC_ERROR;
    }
    pdx = data->index;
    memset(profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));

    IPMC_CRIT_ENTER();
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        conf = &ipmc_global.ipv4_conf;
    } else {
        conf = &ipmc_global.ipv6_conf;
    }

    str_len = 0;
    data->index = conf->ipmc_port_group_filtering[isid - VTSS_ISID_START][ipmc_port_group_filtering->port_no];
    if (ipmc_lib_mgmt_fltr_profile_get(profile, FALSE) == VTSS_OK) {
        str_len = strlen(data->name);
    }
    /* check whether entry existing or not */
    if (str_len) {
        if (!strncmp(data->name, ipmc_port_group_filtering->addr.profile.name, str_len)) {
            /* if this entry is found, just do nothing */
            if (ipmc_lib_isid_is_local(isid)) {
                IPMC_CRIT_EXIT();
                IPMC_MEM_PROFILE_MGIVE(pfm);
                return VTSS_OK;
            }
        }

        conf->ipmc_port_group_filtering[isid - VTSS_ISID_START][ipmc_port_group_filtering->port_no] = IPMC_LIB_FLTR_PROFILE_IDX_NONE;
    }

    conf->ipmc_port_group_filtering[isid - VTSS_ISID_START][ipmc_port_group_filtering->port_no] = pdx;
    IPMC_CRIT_EXIT();
    IPMC_MEM_PROFILE_MGIVE(pfm);

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);
    /* sync this update to the corresponding switch */
    (void) ipmc_stacking_set_grp_filtering(isid, ipmc_version);

    T_D("ipmc_mgmt_set_port_group_filtering consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_del_port_group_filtering(vtss_isid_t isid, ipmc_conf_port_group_filtering_t *ipmc_port_group_filtering, ipmc_ip_version_t ipmc_version)
{
    u32                         pdx;
    ipmc_configuration_t        *conf;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *profile;
    ipmc_lib_profile_t          *data;
    cyg_tick_count_t            exe_time_base;

    if (!ipmc_port_group_filtering) {
        return VTSS_RC_ERROR;
    }

    if (!msg_switch_is_master()) {
        return VTSS_OK;
    }

    if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return VTSS_RC_ERROR;
    }

    exe_time_base = cyg_current_time();

    profile = &pfm->profile;
    data = &profile->data;
    memset(profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
    memcpy(data->name, &ipmc_port_group_filtering->addr.profile.name[0], VTSS_IPMC_NAME_MAX_LEN);
    if (ipmc_lib_mgmt_fltr_profile_get(profile, TRUE) != VTSS_OK) {
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        conf = &ipmc_global.ipv4_conf;
    } else {
        conf = &ipmc_global.ipv6_conf;
    }

    pdx = conf->ipmc_port_group_filtering[isid - VTSS_ISID_START][ipmc_port_group_filtering->port_no];
    if ((pdx == IPMC_LIB_FLTR_PROFILE_IDX_NONE) ||
        (pdx != data->index)) {
        IPMC_CRIT_EXIT();
        IPMC_MEM_PROFILE_MGIVE(pfm);
        return VTSS_RC_ERROR;
    }

    conf->ipmc_port_group_filtering[isid - VTSS_ISID_START][ipmc_port_group_filtering->port_no] = IPMC_LIB_FLTR_PROFILE_IDX_NONE;
    IPMC_CRIT_EXIT();
    IPMC_MEM_PROFILE_MGIVE(pfm);

    /* sync this update to the corresponding switch */
    (void) ipmc_stacking_set_grp_filtering(isid, ipmc_version);
    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);

    T_D("ipmc_mgmt_del_port_group_filtering consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_port_group_filtering(vtss_isid_t isid, ipmc_conf_port_group_filtering_t *ipmc_port_group_filtering, ipmc_ip_version_t ipmc_version)
{
    ipmc_configuration_t        *conf;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *profile;
    ipmc_lib_profile_t          *data;
    cyg_tick_count_t            exe_time_base;

    if (!ipmc_port_group_filtering || !IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return VTSS_RC_ERROR;
    }

    exe_time_base = cyg_current_time();
    profile = &pfm->profile;
    data = &profile->data;

    IPMC_CRIT_ENTER();
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        conf = &ipmc_global.ipv4_conf;
    } else {
        conf = &ipmc_global.ipv6_conf;
    }

    data->index = conf->ipmc_port_group_filtering[isid - VTSS_ISID_START][ipmc_port_group_filtering->port_no];
    if (ipmc_lib_mgmt_fltr_profile_get(profile, FALSE) == VTSS_OK) {
        memcpy(&ipmc_port_group_filtering->addr.profile.name[0], &data->name[0], VTSS_IPMC_NAME_MAX_LEN);
    } else {
        IPMC_CRIT_EXIT();
        IPMC_MEM_PROFILE_MGIVE(pfm);
        memset(&ipmc_port_group_filtering->addr.profile.name[0], 0x0, VTSS_IPMC_NAME_MAX_LEN);
        return VTSS_RC_ERROR;
    }
    IPMC_CRIT_EXIT();
    IPMC_MEM_PROFILE_MGIVE(pfm);

    T_D("ipmc_mgmt_get_port_group_filtering consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_intf_version(vtss_isid_t isid, ipmc_intf_query_host_version_t *vlan_version_entry, ipmc_ip_version_t ipmc_version)
{
    ipmc_prot_intf_entry_param_t    intf_param;

    if (!vlan_version_entry) {
        return VTSS_RC_ERROR;
    }

    memset(&intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
    if ((isid == VTSS_ISID_GLOBAL) || ipmc_lib_isid_is_local(isid)) {
        ipmc_intf_entry_t   *entry;

        IPMC_GET_CRIT_ENTER();
        if ((entry = vtss_ipmc_get_intf_entry(vlan_version_entry->vid, ipmc_version)) != NULL) {
            memcpy(&intf_param, &entry->param, sizeof(ipmc_prot_intf_entry_param_t));
        }
        IPMC_GET_CRIT_EXIT();
    } else {
        if (ipmc_stacking_get_intf_entry(isid, vlan_version_entry->vid, &intf_param, ipmc_version) != VTSS_OK) {
            return VTSS_RC_ERROR;
        }
    }

    if (intf_param.vid == 0) {
        return VTSS_RC_ERROR;
    }

    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        if (intf_param.cfg_compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) {
            if (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_OLD) {
                vlan_version_entry->host_version = VTSS_IPMC_IGMP_VERSION1;
                vlan_version_entry->query_version = VTSS_IPMC_IGMP_VERSION1;
            } else if (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_GEN) {
                vlan_version_entry->host_version = VTSS_IPMC_IGMP_VERSION2;
                vlan_version_entry->query_version = VTSS_IPMC_IGMP_VERSION2;
            } else if (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_SFM) {
                vlan_version_entry->host_version = VTSS_IPMC_IGMP_VERSION3;
                vlan_version_entry->query_version = VTSS_IPMC_IGMP_VERSION3;
            } else {
                vlan_version_entry->host_version = VTSS_IPMC_IGMP_VERSION_DEF;
                vlan_version_entry->query_version = VTSS_IPMC_IGMP_VERSION_DEF;
            }

            break;
        }

        if (intf_param.hst_compatibility.old_present_timer) {
            vlan_version_entry->host_version = VTSS_IPMC_IGMP_VERSION1;
        } else {
            if (intf_param.hst_compatibility.gen_present_timer) {
                vlan_version_entry->host_version = VTSS_IPMC_IGMP_VERSION2;
            } else {
                if (intf_param.hst_compatibility.sfm_present_timer) {
                    vlan_version_entry->host_version = VTSS_IPMC_IGMP_VERSION3;
                } else {
                    vlan_version_entry->host_version = VTSS_IPMC_IGMP_VERSION_DEF;
                }
            }
        }

        if (intf_param.rtr_compatibility.old_present_timer) {
            vlan_version_entry->query_version = VTSS_IPMC_IGMP_VERSION1;
        } else {
            if (intf_param.rtr_compatibility.gen_present_timer) {
                vlan_version_entry->query_version = VTSS_IPMC_IGMP_VERSION2;
            } else {
                if (intf_param.rtr_compatibility.sfm_present_timer) {
                    vlan_version_entry->query_version = VTSS_IPMC_IGMP_VERSION3;
                } else {
                    vlan_version_entry->query_version = VTSS_IPMC_IGMP_VERSION_DEF;
                }
            }
        }

        break;
    case IPMC_IP_VERSION_MLD:
        if (intf_param.cfg_compatibility != VTSS_IPMC_COMPAT_MODE_AUTO) {
            if (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_GEN) {
                vlan_version_entry->host_version = VTSS_IPMC_MLD_VERSION1;
                vlan_version_entry->query_version = VTSS_IPMC_MLD_VERSION1;
            } else if (intf_param.cfg_compatibility == VTSS_IPMC_COMPAT_MODE_SFM) {
                vlan_version_entry->host_version = VTSS_IPMC_MLD_VERSION2;
                vlan_version_entry->query_version = VTSS_IPMC_MLD_VERSION2;
            } else {
                vlan_version_entry->host_version = VTSS_IPMC_MLD_VERSION_DEF;
                vlan_version_entry->query_version = VTSS_IPMC_MLD_VERSION_DEF;
            }

            break;
        }

        if (intf_param.hst_compatibility.gen_present_timer) {
            vlan_version_entry->host_version = VTSS_IPMC_MLD_VERSION1;
        } else {
            if (intf_param.hst_compatibility.sfm_present_timer) {
                vlan_version_entry->host_version = VTSS_IPMC_MLD_VERSION2;
            } else {
                vlan_version_entry->host_version = VTSS_IPMC_MLD_VERSION_DEF;
            }
        }

        if (intf_param.rtr_compatibility.gen_present_timer) {
            vlan_version_entry->query_version = VTSS_IPMC_MLD_VERSION1;
        } else {
            if (intf_param.rtr_compatibility.sfm_present_timer) {
                vlan_version_entry->query_version = VTSS_IPMC_MLD_VERSION2;
            } else {
                vlan_version_entry->query_version = VTSS_IPMC_MLD_VERSION_DEF;
            }
        }

        break;
    default:
        break;
    }

    return VTSS_OK;
}

static void ipmc_update_vlan_tab(vtss_isid_t isid, ipmc_ip_version_t ipmc_version)
{
    vlan_mgmt_entry_t       vlan_conf;
    ipmc_port_bfs_t         vlan_ports;
    port_iter_t             pit;
    BOOL                    found;
    vtss_vid_t              vid;
    u16                     intf_idx;
    ipmc_conf_intf_entry_t  *conf_intf, ipmc_intf;

    if (!msg_switch_is_master()) {
        return;
    }

    T_D("enter ipmc_update_vlan_tab (isid:%d/ver:%d)", isid, ipmc_version);

    vid = 0;
    IPMC_CRIT_ENTER();
    found = ((intf_idx = ipmc_conf_intf_entry_get_next(&vid, ipmc_version)) != IPMC_VID_INIT);
    IPMC_CRIT_EXIT();
    while (found) {
        IPMC_CRIT_ENTER();
        switch ( ipmc_version ) {
        case IPMC_IP_VERSION_IGMP:
            conf_intf = &ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx];

            break;
        case IPMC_IP_VERSION_MLD:
            conf_intf = &ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx];

            break;
        default:
            found = ((intf_idx = ipmc_conf_intf_entry_get_next(&vid, ipmc_version)) != IPMC_VID_INIT);
            IPMC_CRIT_EXIT();

            continue;
        }

        memcpy(&ipmc_intf, conf_intf, sizeof(ipmc_conf_intf_entry_t));
        IPMC_CRIT_EXIT();

        T_D("ipmc_update_vlan_tab idx:%u", intf_idx);

        VTSS_PORT_BF_CLR(vlan_ports.member_ports);
        vlan_conf.vid = vid;
        if (ipmc_resolve_vlan_member(isid, &vlan_conf, FALSE) == VTSS_OK) {
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                VTSS_PORT_BF_SET(vlan_ports.member_ports, pit.iport, vlan_conf.ports[pit.iport]);
            }
        }

        if (ipmc_intf.protocol_status) {
            ipmc_prot_intf_entry_param_t    intf_param;

            (void) ipmc_stacking_set_intf(isid, IPMC_OP_SET, &ipmc_intf, &vlan_ports, ipmc_version);

            memset(&intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
            intf_param.vid = vid;
            intf_param.cfg_compatibility = ipmc_intf.compatibility;
            intf_param.priority = ipmc_intf.priority;
            intf_param.querier.QuerierAdrs4 = ipmc_intf.querier4_address;
            intf_param.querier.RobustVari = ipmc_intf.robustness_variable;
            intf_param.querier.QueryIntvl = ipmc_intf.query_interval;
            intf_param.querier.MaxResTime = ipmc_intf.query_response_interval;
            intf_param.querier.LastQryItv = ipmc_intf.last_listener_query_interval;
            intf_param.querier.LastQryCnt = intf_param.querier.RobustVari;
            intf_param.querier.UnsolicitR = ipmc_intf.unsolicited_report_interval;

            (void) ipmc_stacking_set_intf_entry(isid, &intf_param, ipmc_version);
        } else {
            (void) ipmc_stacking_set_intf(isid, IPMC_OP_SET, &ipmc_intf, &vlan_ports, ipmc_version);
        }

        IPMC_CRIT_ENTER();
        found = ((intf_idx = ipmc_conf_intf_entry_get_next(&vid, ipmc_version)) != IPMC_VID_INIT);
        IPMC_CRIT_EXIT();
    }

    T_D("exit ipmc_update_vlan_tab (isid:%d/ver:%d)", isid, ipmc_version);
}

static void ipmc_vlan_changed(vtss_isid_t isid, vtss_vid_t vid, vlan_membership_change_t *changes)
{
    u16                     intf_idx4, intf_idx6, idx;
    ipmc_port_bfs_t         vlan_ports;
    ipmc_conf_intf_entry_t  ipmc_intf;
    cyg_tick_count_t        exe_time_base;

    if (!IPMC_LIB_ISID_PASS(isid, isid)) {
        return;
    }

    /*
        VLAN Change
        CfgExists + VlanExists:   ipmc_stacking_set_intf (VLAN Member Change)
        CfgExists + !VlanExists:  ipmc_stacking_set_intf (Delete Running Tables)
        !CfgExists + VlanExists:  Do Nothing
        !CfgExists + !VlanExists: Do Nothing
    */
    IPMC_CRIT_ENTER();
    intf_idx4 = ipmc_conf_intf_entry_get(vid, IPMC_IP_VERSION_IGMP);
    intf_idx6 = ipmc_conf_intf_entry_get(vid, IPMC_IP_VERSION_MLD);
    IPMC_CRIT_EXIT();

    if (intf_idx4 == IPMC_VID_INIT && intf_idx6 == IPMC_VID_INIT) {
        // !CfgExists
        return;
    }

    exe_time_base = cyg_current_time();
    T_I("Enter ISID:%d/VID:%u", isid, vid);

    memset(&vlan_ports, 0x0, sizeof(ipmc_port_bfs_t));
    if (changes->static_vlan_exists) {   /* CfgExists + VlanExists */
        // Resulting configuration for static user must be computed based on both the static and the forbidden VLAN users' configuration.
        // However, forbidden ports should only work for GVRP and thus IPMC-SNP won't refer to forbidden membership.
        for (idx = 0; idx < VTSS_PORT_BF_SIZE; idx++) {
            vlan_ports.member_ports[idx] = changes->static_ports.ports[idx];
        }
        T_D("IPMC(%s/%s) update VID %u for ISID %d",
            intf_idx4 != IPMC_VID_INIT ? "IGMP" : "NA",
            intf_idx6 != IPMC_VID_INIT ? "MLD" : "NA",
            vid,
            isid);
    } else {                        /* CfgExists + !VlanExists */
        T_D("IPMC(%s/%s) delete VID %u for ISID %d",
            intf_idx4 != IPMC_VID_INIT ? "IGMP" : "NA",
            intf_idx6 != IPMC_VID_INIT ? "MLD" : "NA",
            vid,
            isid);
    }

    /* IGMP */
    if (intf_idx4 != IPMC_VID_INIT) {
        IPMC_CRIT_ENTER();
        memcpy(&ipmc_intf, &ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx4], sizeof(ipmc_conf_intf_entry_t));
        IPMC_CRIT_EXIT();

        if (ipmc_intf.protocol_status &&
            ipmc_stacking_set_intf(isid, IPMC_OP_SET, &ipmc_intf, &vlan_ports, IPMC_IP_VERSION_IGMP) == VTSS_OK) {
            T_D("Done for IGMP VID-%u/ISID-%d", vid, isid);
        }
    }

    /* MLD */
    if (intf_idx6 != IPMC_VID_INIT) {
        IPMC_CRIT_ENTER();
        memcpy(&ipmc_intf, &ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx6], sizeof(ipmc_conf_intf_entry_t));
        IPMC_CRIT_EXIT();

        if (ipmc_intf.protocol_status &&
            ipmc_stacking_set_intf(isid, IPMC_OP_SET, &ipmc_intf, &vlan_ports, IPMC_IP_VERSION_MLD) == VTSS_OK) {
            T_D("Done for MLD VID-%u/ISID-%d", vid, isid);
        }
    }

    T_I("Exit %u ticks", (u32)(cyg_current_time() - exe_time_base));
}

void ipmc_refresh_enable(ipmc_ip_version_t ipmc_version)
{
    port_iter_t                     pit;
    u16                             intf_idx;
    ipmc_intf_entry_t               *ipmc_vid_entry;
    ipmc_conf_intf_entry_t          ipmc_intf;
    ipmc_prot_intf_entry_param_t    intf_param;
    ipmc_port_bfs_t                 vlan_ports_temp;
    u16                             i = 0;
    ipmc_ip_version_t               version = ipmc_version;
    BOOL                            global_enable_status, found;

    IPMC_CRIT_ENTER();
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        global_enable_status = ipmc_global.ipv4_conf.global.ipmc_mode_enabled;

        break;
    case IPMC_IP_VERSION_MLD:
        global_enable_status = ipmc_global.ipv6_conf.global.ipmc_mode_enabled;

        break;
    default:
        IPMC_CRIT_EXIT();
        return;
    }

    ipmc_vid_entry = vtss_ipmc_get_next_intf_entry(i, version);
    if (ipmc_vid_entry) {
        found = TRUE;

        version = ipmc_vid_entry->ipmc_version;
        i = ipmc_vid_entry->param.vid;

        if (global_enable_status) {
            memcpy(&intf_param, &ipmc_vid_entry->param, sizeof(ipmc_prot_intf_entry_param_t));
        } else {
            VTSS_PORT_BF_CLR(vlan_ports_temp.member_ports);

            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                VTSS_PORT_BF_SET(vlan_ports_temp.member_ports,
                                 pit.iport,
                                 VTSS_PORT_BF_GET(ipmc_vid_entry->vlan_ports, pit.iport));
            }
        }
    } else {
        found = FALSE;
    }

    IPMC_CRIT_EXIT();

    while (found) {
        IPMC_CRIT_ENTER();
        if ((intf_idx = ipmc_conf_intf_entry_get(i, ipmc_version)) != IPMC_VID_INIT) {
            if (ipmc_version == IPMC_IP_VERSION_IGMP) {
                memcpy(&ipmc_intf, &ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx], sizeof(ipmc_conf_intf_entry_t));
            } else {
                memcpy(&ipmc_intf, &ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx], sizeof(ipmc_conf_intf_entry_t));
            }
        } else {
            memset(&ipmc_intf, 0x0, sizeof(ipmc_conf_intf_entry_t));
        }
        IPMC_CRIT_EXIT();

        if (intf_idx != IPMC_VID_INIT) {
            if (!global_enable_status) {
                /* deleting all VLANs and adding them again (restoring ipmc vlan tab to default state) */
                (void) ipmc_stacking_set_intf(VTSS_ISID_GLOBAL, IPMC_OP_DEL, &ipmc_intf, &vlan_ports_temp, ipmc_version);
                (void) ipmc_stacking_set_intf(VTSS_ISID_GLOBAL, IPMC_OP_SET, &ipmc_intf, &vlan_ports_temp, ipmc_version);
            } /* if (!global_enable_status) */

            intf_param.vid = i;
            intf_param.cfg_compatibility = ipmc_intf.compatibility;
            intf_param.priority = ipmc_intf.priority;
            intf_param.querier.QuerierAdrs4 = ipmc_intf.querier4_address;
            intf_param.querier.RobustVari = ipmc_intf.robustness_variable;
            intf_param.querier.QueryIntvl = ipmc_intf.query_interval;
            intf_param.querier.MaxResTime = ipmc_intf.query_response_interval;
            intf_param.querier.LastQryItv = ipmc_intf.last_listener_query_interval;
            intf_param.querier.LastQryCnt = intf_param.querier.RobustVari;
            intf_param.querier.UnsolicitR = ipmc_intf.unsolicited_report_interval;

            (void) ipmc_stacking_set_intf_entry(VTSS_ISID_GLOBAL, &intf_param, ipmc_version);
        } /* if (intf_idx != IPMC_VID_INIT) */

        IPMC_CRIT_ENTER();
        ipmc_vid_entry = vtss_ipmc_get_next_intf_entry(i, version);
        if (ipmc_vid_entry) {
            found = TRUE;

            version = ipmc_vid_entry->ipmc_version;
            i = ipmc_vid_entry->param.vid;

            if (global_enable_status) {
                memcpy(&intf_param, &ipmc_vid_entry->param, sizeof(ipmc_prot_intf_entry_param_t));
            } else {
                VTSS_PORT_BF_CLR(vlan_ports_temp.member_ports);

                (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
                while (port_iter_getnext(&pit)) {
                    VTSS_PORT_BF_SET(vlan_ports_temp.member_ports,
                                     pit.iport,
                                     VTSS_PORT_BF_GET(ipmc_vid_entry->vlan_ports, pit.iport));
                }
            }
        } else {
            found = FALSE;
        }
        IPMC_CRIT_EXIT();
    } /* while (ipmc_vid_entry != NULL) */
}

vtss_rc ipmc_mgmt_set_mode(BOOL *mode, ipmc_ip_version_t ipmc_version)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    IPMC_CRIT_ENTER();

    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        if (ipmc_global.ipv4_conf.global.ipmc_mode_enabled == *mode) {
            IPMC_CRIT_EXIT();
            return VTSS_OK;
        }

        break;
    case IPMC_IP_VERSION_MLD:
        if (ipmc_global.ipv6_conf.global.ipmc_mode_enabled == *mode) {
            IPMC_CRIT_EXIT();
            return VTSS_OK;
        }

        break;
    default:
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        ipmc_global.ipv4_conf.global.ipmc_mode_enabled = *mode;
    } else {
        ipmc_global.ipv6_conf.global.ipmc_mode_enabled = *mode;
    }

    IPMC_CRIT_EXIT();

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);
    (void) ipmc_stacking_set_mode(VTSS_ISID_GLOBAL, ipmc_version, FALSE);
    (void) ipmc_stacking_set_unreg_flood(VTSS_ISID_GLOBAL, ipmc_version);
    ipmc_refresh_enable(ipmc_version);

    T_D("ipmc_mgmt_set_mode consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_mode(BOOL *mode, ipmc_ip_version_t ipmc_version)
{
    vtss_rc             rc = VTSS_OK;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!mode) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        *mode = ipmc_global.ipv4_conf.global.ipmc_mode_enabled;

        break;
    case IPMC_IP_VERSION_MLD:
        *mode = ipmc_global.ipv6_conf.global.ipmc_mode_enabled;

        break;
    default:
        rc = VTSS_RC_ERROR;

        break;
    }
    IPMC_CRIT_EXIT();

    T_D("ipmc_mgmt_get_mode consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return rc;
}

vtss_rc ipmc_mgmt_set_leave_proxy(BOOL *mode, ipmc_ip_version_t ipmc_version)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    IPMC_CRIT_ENTER();

    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        if (ipmc_global.ipv4_conf.global.ipmc_leave_proxy_enabled == *mode) {
            IPMC_CRIT_EXIT();
            return VTSS_OK;
        }

        break;
    case IPMC_IP_VERSION_MLD:
        if (ipmc_global.ipv6_conf.global.ipmc_leave_proxy_enabled == *mode) {
            IPMC_CRIT_EXIT();
            return VTSS_OK;
        }

        break;
    default:
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        ipmc_global.ipv4_conf.global.ipmc_leave_proxy_enabled = *mode;
    } else {
        ipmc_global.ipv6_conf.global.ipmc_leave_proxy_enabled = *mode;
    }
    IPMC_CRIT_EXIT();

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);
    (void) ipmc_stacking_set_leave_proxy(VTSS_ISID_GLOBAL, ipmc_version);

    T_D("ipmc_mgmt_set_leave_proxy consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_leave_proxy(BOOL *mode, ipmc_ip_version_t ipmc_version)
{
    vtss_rc             rc = VTSS_OK;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!mode) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        *mode = ipmc_global.ipv4_conf.global.ipmc_leave_proxy_enabled;

        break;
    case IPMC_IP_VERSION_MLD:
        *mode = ipmc_global.ipv6_conf.global.ipmc_leave_proxy_enabled;

        break;
    default:
        rc = VTSS_RC_ERROR;

        break;
    }
    IPMC_CRIT_EXIT();

    T_D("ipmc_mgmt_get_leave_proxy consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return rc;
}

vtss_rc ipmc_mgmt_set_proxy(BOOL *mode, ipmc_ip_version_t ipmc_version)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    IPMC_CRIT_ENTER();

    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        if (ipmc_global.ipv4_conf.global.ipmc_proxy_enabled == *mode) {
            IPMC_CRIT_EXIT();
            return VTSS_OK;
        }

        break;
    case IPMC_IP_VERSION_MLD:
        if (ipmc_global.ipv6_conf.global.ipmc_proxy_enabled == *mode) {
            IPMC_CRIT_EXIT();
            return VTSS_OK;
        }

        break;
    default:
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        ipmc_global.ipv4_conf.global.ipmc_proxy_enabled = *mode;
    } else {
        ipmc_global.ipv6_conf.global.ipmc_proxy_enabled = *mode;
    }
    IPMC_CRIT_EXIT();

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);
    (void) ipmc_stacking_set_proxy(VTSS_ISID_GLOBAL, ipmc_version);

    T_D("ipmc_mgmt_set_proxy consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_proxy(BOOL *mode, ipmc_ip_version_t ipmc_version)
{
    vtss_rc             rc = VTSS_OK;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!mode) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        *mode = ipmc_global.ipv4_conf.global.ipmc_proxy_enabled;

        break;
    case IPMC_IP_VERSION_MLD:
        *mode = ipmc_global.ipv6_conf.global.ipmc_proxy_enabled;

        break;
    default:
        rc = VTSS_RC_ERROR;

        break;
    }
    IPMC_CRIT_EXIT();

    T_D("ipmc_mgmt_get_proxy consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return rc;
}

vtss_rc ipmc_mgmt_set_unreg_flood(BOOL *mode, ipmc_ip_version_t ipmc_version)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    IPMC_CRIT_ENTER();

    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        if (ipmc_global.ipv4_conf.global.ipmc_unreg_flood_enabled == *mode) {
            IPMC_CRIT_EXIT();
            return VTSS_OK;
        }

        break;
    case IPMC_IP_VERSION_MLD:
        if (ipmc_global.ipv6_conf.global.ipmc_unreg_flood_enabled == *mode) {
            IPMC_CRIT_EXIT();
            return VTSS_OK;
        }

        break;
    default:
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        ipmc_global.ipv4_conf.global.ipmc_unreg_flood_enabled = *mode;
    } else {
        ipmc_global.ipv6_conf.global.ipmc_unreg_flood_enabled = *mode;
    }
    IPMC_CRIT_EXIT();

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);
    (void) ipmc_stacking_set_unreg_flood(VTSS_ISID_GLOBAL, ipmc_version);

    T_D("ipmc_mgmt_set_unreg_flood consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_unreg_flood(BOOL *mode, ipmc_ip_version_t ipmc_version)
{
    vtss_rc             rc = VTSS_OK;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!mode) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        *mode = ipmc_global.ipv4_conf.global.ipmc_unreg_flood_enabled;

        break;
    case IPMC_IP_VERSION_MLD:
        *mode = ipmc_global.ipv6_conf.global.ipmc_unreg_flood_enabled;

        break;
    default:
        rc = VTSS_RC_ERROR;

        break;
    }
    IPMC_CRIT_EXIT();

    T_D("ipmc_mgmt_get_unreg_flood consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return rc;
}

vtss_rc ipmc_mgmt_set_ssm_range(ipmc_ip_version_t ipmc_version, ipmc_prefix_t *range)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();
    ipmc_prefix_t       prefix;

    if (!msg_switch_is_master()) {
        return VTSS_OK;
    }

    if (!range) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();

    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        memset(range->addr.value.reserved, 0x0, sizeof(range->addr.value.reserved));
        if (!memcmp(&ipmc_global.ipv4_conf.global.ssm_range, range, sizeof(ipmc_prefix_t))) {
            IPMC_CRIT_EXIT();
            return VTSS_OK;
        }

        break;
    case IPMC_IP_VERSION_MLD:
        if (!memcmp(&ipmc_global.ipv6_conf.global.ssm_range, range, sizeof(ipmc_prefix_t))) {
            IPMC_CRIT_EXIT();
            return VTSS_OK;
        }

        break;
    default:
        IPMC_CRIT_EXIT();
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_EXIT();

    if (!ipmc_lib_prefix_maskingNchecking(ipmc_version, TRUE, range, &prefix)) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        memcpy(&ipmc_global.ipv4_conf.global.ssm_range, &prefix, sizeof(ipmc_prefix_t));
    } else {
        memcpy(&ipmc_global.ipv6_conf.global.ssm_range, &prefix, sizeof(ipmc_prefix_t));
    }
    IPMC_CRIT_EXIT();

    ipmc_sm_event_set(IPMC_EVENT_CONFBLK_COMMIT);
    (void) ipmc_stacking_set_ssm_range(VTSS_ISID_GLOBAL, ipmc_version);

    T_D("ipmc_mgmt_set_ssm_range consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_OK;
}

vtss_rc ipmc_mgmt_get_ssm_range(ipmc_ip_version_t ipmc_version, ipmc_prefix_t *range)
{
    vtss_rc             rc = VTSS_OK;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    if (!range) {
        return VTSS_RC_ERROR;
    }

    IPMC_CRIT_ENTER();
    switch ( ipmc_version ) {
    case IPMC_IP_VERSION_IGMP:
        memcpy(range, &ipmc_global.ipv4_conf.global.ssm_range, sizeof(ipmc_prefix_t));

        break;
    case IPMC_IP_VERSION_MLD:
        memcpy(range, &ipmc_global.ipv6_conf.global.ssm_range, sizeof(ipmc_prefix_t));

        break;
    default:
        rc = VTSS_RC_ERROR;

        break;
    }
    IPMC_CRIT_EXIT();

    T_D("ipmc_mgmt_get_ssm_range consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return rc;
}

/* Check whether default IPMC SSM Range is changed for not  */
BOOL ipmc_mgmt_def_ssm_range_chg(ipmc_ip_version_t version, ipmc_prefix_t *range)
{
    BOOL    chg = FALSE;

    if (!range) {
        return chg;
    }

    switch ( version ) {
    case IPMC_IP_VERSION_IGMP:
        if (range->len != VTSS_IPMC_SSM4_RANGE_LEN) {
            chg = TRUE;
        } else {
            if (range->addr.value.prefix != VTSS_IPMC_SSM4_RANGE_PREFIX) {
                chg = TRUE;
            }
        }

        break;
    case IPMC_IP_VERSION_MLD:
        if (range->len != VTSS_IPMC_SSM6_RANGE_LEN) {
            chg = TRUE;
        } else {
            u8  idx;

            for (idx = 0; idx < 15; idx++) {
                if (idx == 0) {
                    if (range->addr.array.prefix.addr[0] != ((VTSS_IPMC_SSM6_RANGE_PREFIX >> 24) & 0xFF)) {
                        chg = TRUE;
                    }
                } else if (idx == 1) {
                    if (range->addr.array.prefix.addr[1] != ((VTSS_IPMC_SSM6_RANGE_PREFIX >> 16) & 0xFF)) {
                        chg = TRUE;
                    }
                } else if (idx == 2) {
                    if (range->addr.array.prefix.addr[2] != ((VTSS_IPMC_SSM6_RANGE_PREFIX >> 8) & 0xFF)) {
                        chg = TRUE;
                    }
                } else if (idx == 3) {
                    if (range->addr.array.prefix.addr[3] != ((VTSS_IPMC_SSM6_RANGE_PREFIX >> 0) & 0xFF)) {
                        chg = TRUE;
                    }
                } else {
                    if (range->addr.array.prefix.addr[idx]) {
                        chg = TRUE;
                    }
                }

                if (chg) {
                    break;
                }
            }
        }

        break;
    default:

        break;
    }

    return chg;
}

vtss_rc ipmc_mgmt_clear_stat_counter(vtss_isid_t isid, ipmc_ip_version_t ipmc_version, vtss_vid_t vid)
{
    vtss_rc             rc;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    rc = ipmc_stacking_clear_statistics(isid, ipmc_version, vid);

    T_D("ipmc_mgmt_clear_stat_counter consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

    return rc;
}

static void ipmc_conf_default(ipmc_configuration_t *conf, ipmc_ip_version_t ipmc_version)
{
    int                     i;
    ipmc_conf_global_t      *global;

    if (!conf) {
        return;
    }

    /* Use default configuration */
    memset(conf, 0x0, sizeof(ipmc_configuration_t));
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        conf->version = IPMC_IP_VERSION_IGMP;
    } else {
        conf->version = IPMC_IP_VERSION_MLD;
    }

    global = &conf->global;
    global->ipmc_mode_enabled = IPMC_DEF_GLOBAL_STATE_VALUE;
    global->ipmc_unreg_flood_enabled = IPMC_DEF_UNREG_FLOOD_VALUE;
    global->ipmc_leave_proxy_enabled = IPMC_DEF_LEAVE_PROXY_VALUE;
    global->ipmc_proxy_enabled = IPMC_DEF_PROXY_VALUE;
    if (ipmc_version == IPMC_IP_VERSION_IGMP) {
        global->ssm_range.addr.value.prefix = VTSS_IPMC_SSM4_RANGE_PREFIX;
        global->ssm_range.len = VTSS_IPMC_SSM4_RANGE_LEN;
    } else {
        global->ssm_range.addr.array.prefix.addr[0] = (VTSS_IPMC_SSM6_RANGE_PREFIX >> 24) & 0xFF;
        global->ssm_range.addr.array.prefix.addr[1] = (VTSS_IPMC_SSM6_RANGE_PREFIX >> 16) & 0xFF;
        global->ssm_range.addr.array.prefix.addr[2] = (VTSS_IPMC_SSM6_RANGE_PREFIX >> 8) & 0xFF;
        global->ssm_range.addr.array.prefix.addr[3] = (VTSS_IPMC_SSM6_RANGE_PREFIX >> 0) & 0xFF;
        global->ssm_range.len = VTSS_IPMC_SSM6_RANGE_LEN;
    }

    for (i = 0; i < IPMC_VLAN_MAX; i++) {
        _ipmc_reset_conf_intf(&conf->ipmc_conf_intf_entries[i], FALSE, 0x0);
    }
}

#define SNP_PORT_MAX_GRP_FILTER_NUM     5
#define IPMC_NO_OF_PORT_GROUP_FILTERING SNP_PORT_MAX_GRP_FILTER_NUM

typedef struct {
    vtss_vid_t                  vid;
    BOOL                        ipmc_status;
    BOOL                        querier_status;
    BOOL                        fast_leave_status;
} obs1_snp_conf_vid_entry_t;

typedef struct {
    BOOL                        ipmc_mode_enabled;
    BOOL                        ipmc_unreg_flood_enabled;
    BOOL                        ipmc_leave_proxy_enabled;
    BOOL                        ipmc_proxy_enabled;

    BOOL                        ipmc_router_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];         /* Dest. ports */
    BOOL                        ipmc_fast_leave_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];     /* Dest. ports */
    int                         ipmc_throttling_max_no[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];    /* "0" means disabled */
    vtss_ipv6_t                 ipmc_port_group_filtering[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE][SNP_PORT_MAX_GRP_FILTER_NUM];

    obs1_snp_conf_vid_entry_t   ipmc_conf_vlan_entries[VTSS_IPMC_VID_MAX];
} ipmc_conf_v1_t;

typedef struct {
    BOOL                        valid;

    vtss_vid_t                  vid;

    BOOL                        protocol_status;
    BOOL                        querier_status;
    u32                         compatibility;

    u32                         robustness_variable;
    u32                         query_interval;
    u32                         query_response_interval;
    u32                         last_listener_query_interval;
    u32                         unsolicited_report_interval;
} obs2_snp_conf_intf_entry_t;

typedef struct {
    ipmc_ip_version_t           version;

    ipmc_conf_global_t          global;

    obs2_snp_conf_intf_entry_t  ipmc_conf_vlan_entries[VTSS_IPMC_VID_MAX];

    BOOL                        ipmc_router_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];         /* Dest. ports */
    BOOL                        ipmc_fast_leave_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];     /* Dest. ports */
    int                         ipmc_throttling_max_no[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];    /* "0" means disabled */
    vtss_ipv6_t                 ipmc_port_group_filtering[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE][IPMC_NO_OF_PORT_GROUP_FILTERING];
} ipmc_conf_v2_t;

typedef struct {
    BOOL                        valid;
    BOOL                        protocol_status;
    BOOL                        querier_status;
    BOOL                        proxy_status;

    u32                         compatibility;

    u32                         robustness_variable;
    u32                         query_interval;
    u32                         query_response_interval;
    u32                         last_listener_query_interval;
    u32                         unsolicited_report_interval;

    vtss_vid_t                  vid;
    u8                          priority;
    u8                          reserved[5];
} obs34_snp_conf_intf_entry_t;

typedef struct {
    ipmc_ip_version_t           version;

    ipmc_conf_global_t          global;

    obs34_snp_conf_intf_entry_t ipmc_conf_intf_entries[IPMC_VLAN_MAX];

    BOOL                        ipmc_router_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];         /* Dest. ports */
    BOOL                        ipmc_fast_leave_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];     /* Dest. ports */
    int                         ipmc_throttling_max_no[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];    /* "0" means disabled */
    vtss_ipv6_t                 ipmc_port_group_filtering[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE][IPMC_NO_OF_PORT_GROUP_FILTERING];
} ipmc_conf_v3_t;

typedef struct {
    ipmc_ip_version_t           version;

    ipmc_conf_global_t          global;

    obs34_snp_conf_intf_entry_t ipmc_conf_intf_entries[IPMC_VLAN_MAX];

    BOOL                        ipmc_router_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];         /* Dest. ports */
    BOOL                        ipmc_fast_leave_ports[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];     /* Dest. ports */
    int                         ipmc_throttling_max_no[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];    /* "0" means disabled */
    u32                         ipmc_port_group_filtering[VTSS_ISID_CNT][VTSS_PORT_ARRAY_SIZE];
} ipmc_conf_v4_t;

static ipmc_conf_v3_t           obs_ipmc_conf_v3;
static ipmc_conf_v4_t           obs_ipmc_conf_v4;

static BOOL _ipmc_conf_copy(ipmc_ip_version_t ver, ipmc_configuration_t *cfg_src, ipmc_configuration_t *cfg_dst)
{
    if (cfg_src && cfg_dst) {
        memcpy(cfg_dst, cfg_src, sizeof(ipmc_configuration_t));
    } else {
        T_W("Invalid %s Configuration Block",
            ipmc_lib_version_txt(ver, IPMC_TXT_CASE_UPPER));
        return FALSE;
    }

    return TRUE;
}

static BOOL ipmc_conf_transition(ipmc_ip_version_t ip_ver,
                                 u32 blk_ver,
                                 u32 conf_sz,
                                 void *blk_conf,
                                 BOOL *new_blk,
                                 ipmc_configuration_t *tar_conf)
{
    BOOL                    conf_reset, create_blk;
    ipmc_conf_v1_t          *conf_blk1;
    ipmc_conf_v2_t          *conf_blk2;
    ipmc_conf_v3_t          *conf_blk3;
    ipmc_conf_v4_t          *conf_blk4;
    ipmc_configuration_t    *conf_blk;

    if (!blk_conf || !new_blk || !tar_conf) {
        return FALSE;
    }

    create_blk = conf_reset = FALSE;
    conf_blk = NULL;
    switch ( blk_ver ) {
    case 1:
        conf_blk1 = NULL;
        if (ip_ver == IPMC_IP_VERSION_IGMP) {
            conf_blk1 = (ipmc_conf_v1_t *)blk_conf;
        }
        if (ip_ver == IPMC_IP_VERSION_MLD) {
            conf_blk1 = (ipmc_conf_v1_t *)blk_conf + 1;
        }

        if (conf_blk1) {
//            memcpy(&obs_ipmc_conf_v1, conf_blk1, sizeof(ipmc_conf_v1_t));
        }

        break;
    case 2:
        conf_blk2 = NULL;
        if (ip_ver == IPMC_IP_VERSION_IGMP) {
            conf_blk2 = (ipmc_conf_v2_t *)blk_conf;
        }
        if (ip_ver == IPMC_IP_VERSION_MLD) {
            conf_blk2 = (ipmc_conf_v2_t *)blk_conf + 1;
        }

        if (conf_blk2) {
//            memcpy(&obs_ipmc_conf_v2, conf_blk2, sizeof(ipmc_conf_v2_t));
        }

        break;
    case 3:
        conf_blk3 = NULL;
        if (ip_ver == IPMC_IP_VERSION_IGMP) {
            conf_blk3 = (ipmc_conf_v3_t *)blk_conf;
        }
        if (ip_ver == IPMC_IP_VERSION_MLD) {
            conf_blk3 = (ipmc_conf_v3_t *)blk_conf + 1;
        }

        if (conf_blk3) {
            memcpy(&obs_ipmc_conf_v3, conf_blk3, sizeof(ipmc_conf_v3_t));
        }

        break;
    case 4:
        conf_blk4 = NULL;
        if (ip_ver == IPMC_IP_VERSION_IGMP) {
            conf_blk4 = (ipmc_conf_v4_t *)blk_conf;
        }
        if (ip_ver == IPMC_IP_VERSION_MLD) {
            conf_blk4 = (ipmc_conf_v4_t *)blk_conf + 1;
        }

        if (conf_blk4) {
            memcpy(&obs_ipmc_conf_v4, conf_blk4, sizeof(ipmc_conf_v4_t));
        }

        break;
    case 5:
    default:
        if (ip_ver == IPMC_IP_VERSION_IGMP) {
            conf_blk = (ipmc_configuration_t *)blk_conf;
        }
        if (ip_ver == IPMC_IP_VERSION_MLD) {
            conf_blk = (ipmc_configuration_t *)blk_conf + 1;
        }

        break;
    }

    if (blk_ver > IPMC_CONF_VERSION) {
        /* Down-grade is not expected, just reset the current configuration */
        create_blk = TRUE;
        conf_reset = TRUE;
    } else if (blk_ver < IPMC_CONF_VERSION) {
        obs34_snp_conf_intf_entry_t *obs34_intf;
        ipmc_conf_intf_entry_t      *intf;
        u16                         intf_idx, intf_cnt;

        /* Up-grade is allowed, do seamless transition */
        create_blk = TRUE;

        switch ( blk_ver ) {
        case 1:
        case 2:
            conf_reset = TRUE;

            break;
        case 3:
            memset(tar_conf, 0x0, sizeof(ipmc_configuration_t));

            tar_conf->version = obs_ipmc_conf_v3.version;
            memcpy(&tar_conf->global, &obs_ipmc_conf_v3.global, sizeof(ipmc_conf_global_t));
            memcpy(tar_conf->ipmc_router_ports, obs_ipmc_conf_v3.ipmc_router_ports, sizeof(tar_conf->ipmc_router_ports));
            memcpy(tar_conf->ipmc_fast_leave_ports, obs_ipmc_conf_v3.ipmc_fast_leave_ports, sizeof(tar_conf->ipmc_fast_leave_ports));
            memcpy(tar_conf->ipmc_throttling_max_no, obs_ipmc_conf_v3.ipmc_throttling_max_no, sizeof(tar_conf->ipmc_throttling_max_no));

            intf_cnt = 0;
            for (intf_idx = 0; intf_idx < IPMC_VLAN_MAX; intf_idx++) {
                obs34_intf = &obs_ipmc_conf_v3.ipmc_conf_intf_entries[intf_idx];
                if (!obs34_intf || !obs34_intf->valid ||
                    !obs34_intf->vid || !obs34_intf->protocol_status) {
                    continue;
                }

                intf = &tar_conf->ipmc_conf_intf_entries[intf_cnt];
                intf->valid = TRUE;
                intf->vid = obs34_intf->vid;
                intf->protocol_status = TRUE;

                intf->querier_status = obs34_intf->querier_status;
                intf->proxy_status = obs34_intf->proxy_status;
                intf->priority = obs34_intf->priority;
                intf->compatibility = obs34_intf->compatibility;

                intf->robustness_variable = obs34_intf->robustness_variable;
                intf->query_interval = obs34_intf->query_interval;
                intf->query_response_interval = obs34_intf->query_response_interval;
                intf->last_listener_query_interval = obs34_intf->last_listener_query_interval;
                intf->unsolicited_report_interval = obs34_intf->unsolicited_report_interval;

                ++intf_cnt;
            }

            break;
        case 4:
            memset(tar_conf, 0x0, sizeof(ipmc_configuration_t));

            tar_conf->version = obs_ipmc_conf_v4.version;
            memcpy(&tar_conf->global, &obs_ipmc_conf_v4.global, sizeof(ipmc_conf_global_t));
            memcpy(tar_conf->ipmc_router_ports, obs_ipmc_conf_v4.ipmc_router_ports, sizeof(tar_conf->ipmc_router_ports));
            memcpy(tar_conf->ipmc_fast_leave_ports, obs_ipmc_conf_v4.ipmc_fast_leave_ports, sizeof(tar_conf->ipmc_fast_leave_ports));
            memcpy(tar_conf->ipmc_throttling_max_no, obs_ipmc_conf_v4.ipmc_throttling_max_no, sizeof(tar_conf->ipmc_throttling_max_no));
            memcpy(tar_conf->ipmc_port_group_filtering, obs_ipmc_conf_v4.ipmc_port_group_filtering, sizeof(tar_conf->ipmc_port_group_filtering));

            intf_cnt = 0;
            for (intf_idx = 0; intf_idx < IPMC_VLAN_MAX; intf_idx++) {
                obs34_intf = &obs_ipmc_conf_v4.ipmc_conf_intf_entries[intf_idx];
                if (!obs34_intf || !obs34_intf->valid ||
                    !obs34_intf->vid || !obs34_intf->protocol_status) {
                    continue;
                }

                intf = &tar_conf->ipmc_conf_intf_entries[intf_cnt];
                intf->valid = TRUE;
                intf->vid = obs34_intf->vid;
                intf->protocol_status = TRUE;

                intf->querier_status = obs34_intf->querier_status;
                intf->proxy_status = obs34_intf->proxy_status;
                intf->priority = obs34_intf->priority;
                intf->compatibility = obs34_intf->compatibility;

                intf->robustness_variable = obs34_intf->robustness_variable;
                intf->query_interval = obs34_intf->query_interval;
                intf->query_response_interval = obs34_intf->query_response_interval;
                intf->last_listener_query_interval = obs34_intf->last_listener_query_interval;
                intf->unsolicited_report_interval = obs34_intf->unsolicited_report_interval;

                ++intf_cnt;
            }

            break;
        default:
            if (blk_ver == IPMC_CONF_VERINIT) {
                create_blk = FALSE;
            }

            conf_reset = TRUE;

            break;
        }
    } else {
        if (sizeof(ipmc_configuration_t) != conf_sz) {
            create_blk = TRUE;
            conf_reset = TRUE;
        } else {
            if (!_ipmc_conf_copy(ip_ver, conf_blk, tar_conf)) {
                return FALSE;
            }
        }
    }

    *new_blk = create_blk;
    if (conf_reset) {
        T_W("Creating %s-Snooping default configurations",
            ipmc_lib_version_txt(ip_ver, IPMC_TXT_CASE_UPPER));
        ipmc_conf_default(tar_conf, ip_ver);
    }

    return TRUE;
}

static ipmc_error_t ipmc_conf_read(BOOL create)
{
    ipmc_conf_blk_t         *blk = NULL;
    u32                     size = 0;
    ipmc_configuration_t    *conf;
    BOOL                    do_create = FALSE, do_default = FALSE;
    cyg_tick_count_t        exe_time_base = cyg_current_time();

    if (misc_conf_read_use()) {
        if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_IPMC_CONF, &size)) == NULL) {
            T_W("conf_sec_open failed, creating IPMC defaults");
            blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_IPMC_CONF, sizeof(*blk));

            if (blk == NULL) {
                T_E("ipmc_conf_read failed");
                return IPMC_ERROR_GEN;
            }

            blk->blk_version = IPMC_CONF_VERINIT;
            do_default = TRUE;
        } else {
            do_default = create;
        }
    } else {
        do_default = TRUE;
    }

    IPMC_CRIT_ENTER();
    if (!do_default && blk) {
        BOOL    new_blk;
        u32     orig_conf_size = (size - sizeof(blk->blk_version)) / IPMC_NUM_OF_SUPPORTED_IP_VER;

        conf = &ipmc_global.ipv4_conf;
        new_blk = FALSE;
        if (ipmc_conf_transition(IPMC_IP_VERSION_IGMP,
                                 blk->blk_version,
                                 orig_conf_size,
                                 (void *)&blk->ipv4_conf,   /* Give it the init. address */
                                 &new_blk, conf)) {
            if (!do_create) {
                do_create = new_blk;
            }
        } else {
            IPMC_CRIT_EXIT();
            T_W("ipmc_conf_transition failed");
            return IPMC_ERROR_GEN;
        }

        conf = &ipmc_global.ipv6_conf;
#ifdef VTSS_SW_OPTION_SMB_IPMC
        new_blk = FALSE;
        if (ipmc_conf_transition(IPMC_IP_VERSION_MLD,
                                 blk->blk_version,
                                 orig_conf_size,
                                 (void *)&blk->ipv4_conf,   /* Give it the init. address */
                                 &new_blk, conf)) {
            if (!do_create) {
                do_create = new_blk;
            }
        } else {
            IPMC_CRIT_EXIT();
            T_W("ipmc_conf_transition failed");
            return IPMC_ERROR_GEN;
        }
#else
        ipmc_conf_default(conf, IPMC_IP_VERSION_MLD);
#endif /* VTSS_SW_OPTION_SMB_IPMC */
    } else {
        conf = &ipmc_global.ipv4_conf;
        ipmc_conf_default(conf, IPMC_IP_VERSION_IGMP);

        conf = &ipmc_global.ipv6_conf;
        ipmc_conf_default(conf, IPMC_IP_VERSION_MLD);
    }
    IPMC_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (do_create) {
        blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_IPMC_CONF, sizeof(*blk));

        if (blk == NULL) {
            T_E("conf_sec_create failed");
            return IPMC_ERROR_GEN;
        }
    }

    if (blk) {  // Quiet lint
        IPMC_CRIT_ENTER();
        blk->ipv6_conf = ipmc_global.ipv6_conf;
        blk->ipv4_conf = ipmc_global.ipv4_conf;
        IPMC_CRIT_EXIT();

        blk->blk_version = IPMC_CONF_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_IPMC_CONF);
    }
#else
    (void) do_create;  // Quiet lint
#endif

    T_D("ipmc_conf_read consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

    return VTSS_RC_OK;
}

void ipmc_port_state_change_callback(vtss_port_no_t port_no, port_info_t *info)
{
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    IPMC_CRIT_ENTER();
    vtss_ipmc_port_state_change_handle(port_no, info);
    IPMC_CRIT_EXIT();

    T_D("ipmc_port_state_change_callback consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));
}

void ipmc_stp_port_state_change_callback(vtss_common_port_t l2port, vtss_common_stpstate_t new_state)
{
    switch_iter_t                       sit;
    vtss_isid_t                         isid, l2_isid;
    vtss_port_no_t                      iport;
    ipmc_msg_buf_t                      buf;
    ipmc_msg_stp_port_change_set_req_t  *msg;
    BOOL                                v4_mode, v6_mode;
    BOOL                                v4_proxy, v6_proxy;
    BOOL                                v4_rtr_port, v6_rtr_port;
    ipmc_ip_version_t                   ipmc_version = IPMC_IP_VERSION_INIT;
    cyg_tick_count_t                    exe_time_base = cyg_current_time();

    /* Only process it when master and forwarding state */
    if (!msg_switch_is_master() || (new_state != VTSS_COMMON_STPSTATE_FORWARDING)) {
        return;
    }

    IPMC_CRIT_ENTER();
    /* MODE */
    if (ipmc_global.ipv4_conf.global.ipmc_mode_enabled == VTSS_IPMC_TRUE) {
        v4_mode = TRUE;
    } else {
        v4_mode = FALSE;
    }
    if (ipmc_global.ipv6_conf.global.ipmc_mode_enabled == VTSS_IPMC_TRUE) {
        v6_mode = TRUE;
    } else {
        v6_mode = FALSE;
    }
    IPMC_CRIT_EXIT();

    if (!v4_mode && !v6_mode) {
        return;
    }

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

                        IPMC_CRIT_ENTER();
                        if (!ipmc_port_status[ipmc_lib_isid_convert(TRUE, isid)][iport]) {
                            IPMC_CRIT_EXIT();
                            continue;
                        }
                        IPMC_CRIT_EXIT();

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
                T_W("ipmc_stp_port_state_change_callback(%s): fail to translate logical ports\n", l2port2str(l2port));
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

                    IPMC_CRIT_ENTER();
                    if (!ipmc_port_status[ipmc_lib_isid_convert(TRUE, isid)][iport]) {
                        IPMC_CRIT_EXIT();
                        continue;
                    }
                    IPMC_CRIT_EXIT();

                    if (logical_members[isid_idx].entry.member[iport]) {
                        aggr_found = TRUE;
                        break;
                    }
                }
            }

            if (!aggr_found) {
                T_W("ipmc_stp_port_state_change_callback(%s): fail to translate logical ports\n", l2port2str(l2port));
                return;
            }
        } else {
            T_W("ipmc_stp_port_state_change_callback(%s): fail to translate logical ports\n", l2port2str(l2port));
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

    /*
        If global/proxy mode are enabled and it isn't static router port,
        sendout a specfic query for collecting the group information.
        Note: We should filter sendout process if this port belong to
        dynamic router port.  Check it when receiving the message on each switch.
    */
    IPMC_CRIT_ENTER();
    /* PROXY */
    if (ipmc_global.ipv4_conf.global.ipmc_proxy_enabled == VTSS_IPMC_TRUE) {
        v4_proxy = TRUE;
    } else {
        v4_proxy = FALSE;
    }
    if (ipmc_global.ipv6_conf.global.ipmc_proxy_enabled == VTSS_IPMC_TRUE) {
        v6_proxy = TRUE;
    } else {
        v6_proxy = FALSE;
    }
    /* ROUTER-PORT */
    if ((isid == VTSS_ISID_LOCAL) || (isid == VTSS_ISID_GLOBAL)) {
        v4_rtr_port = ipmc_global.ipv4_conf.ipmc_router_ports[msg_master_isid() - VTSS_ISID_START][iport];
        v6_rtr_port = ipmc_global.ipv6_conf.ipmc_router_ports[msg_master_isid() - VTSS_ISID_START][iport];
    } else {
        v4_rtr_port = ipmc_global.ipv4_conf.ipmc_router_ports[isid - VTSS_ISID_START][iport];
        v6_rtr_port = ipmc_global.ipv6_conf.ipmc_router_ports[isid - VTSS_ISID_START][iport];
    }
    IPMC_CRIT_EXIT();

    if (v4_mode && v4_proxy && !v4_rtr_port) {
        ipmc_version = IPMC_IP_VERSION_IGMP;
    }

    if (v6_mode && v6_proxy && !v6_rtr_port) {
        if (ipmc_version != IPMC_IP_VERSION_INIT) {
            ipmc_version = IPMC_IP_VERSION_ALL;
        } else {
            ipmc_version = IPMC_IP_VERSION_MLD;
        }
    }

    if (ipmc_version != IPMC_IP_VERSION_INIT) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
            isid = sit.isid;
            if (!IPMC_LIB_ISID_PASS(l2_isid, isid)) {
                continue;
            }

            ipmc_msg_alloc(isid, IPMC_MSG_ID_STP_PORT_CHANGE_REQ, &buf, FALSE, TRUE);
            msg = (ipmc_msg_stp_port_change_set_req_t *) buf.msg;

            msg->msg_id = IPMC_MSG_ID_STP_PORT_CHANGE_REQ;
            msg->port = iport;
            msg->new_state = new_state;
            msg->version = ipmc_version;
            ipmc_msg_tx(&buf, isid, sizeof(*msg));
        }
    }

    T_D("ipmc_stp_port_state_change_callback consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));
}

#if defined(VTSS_FEATURE_SFM)
/* SFM Timer Thread */
static cyg_handle_t         sfm_thread_handle;
static cyg_thread           sfm_thread_block;
static char                 sfm_thread_stack[IPMC_THREAD_STACK_SIZE];

static cyg_flag_t           sfm_events;

static sfm_intf_event_t     conflict_chk;

static void sfm_event_set(ulong flag)
{
    cyg_flag_value_t    event;

    event = (cyg_flag_value_t)flag;
    cyg_flag_setbits(&sfm_events, event);
}

static void sfm_timer_isr(cyg_handle_t alarm, cyg_addrword_t data)
{
    if (alarm || data) { /* avoid warning */
    }

    sfm_event_set(SFM_EVENT_TIME_WAKEUP);
}

void sfm_thread(cyg_addrword_t data)
{
    static cyg_alarm    sfm_alarm;
    static cyg_handle_t sfm_alarm_handle;
    cyg_handle_t        sfmCounter;
    cyg_flag_value_t    events;
    ulong               ticks = 0, ticks_overflow = 0;

    vtss_rc             rc;
    uchar               idx;
    vtss_sfm_t          entry;

    /* Initialize EVENT groups */
    cyg_flag_init(&sfm_events);

    /* Initialize Periodical Wakeup Timer(Alarm)  */
    cyg_clock_to_counter(cyg_real_time_clock(), &sfmCounter);
    cyg_alarm_create(sfmCounter,
                     sfm_timer_isr,
                     0,
                     &sfm_alarm_handle,
                     &sfm_alarm);
    cyg_alarm_initialize(sfm_alarm_handle, cyg_current_time() + 1, SFM_THREAD_TICK_TIME / ECOS_MSECS_PER_HWTICK);
    cyg_alarm_enable(sfm_alarm_handle);

    while (TRUE) {
        events = cyg_flag_wait(&sfm_events, SFM_EVENT_ANY,
                               CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);

        if (events & SFM_EVENT_CONFLICT) {
            for (idx = 0; idx < SFM_NO_OF_SRC_INTF; idx++) {
                if (conflict_chk.conflict_vid[idx] == 0) {
                    continue;
                }

                memset(&entry, 0x0, sizeof(vtss_sfm_t));
                while (vtss_sfm_get_next_entry(&entry) == VTSS_OK) {
                    if (entry.fid != conflict_chk.conflict_vid[idx]) {
                        continue;
                    }

                    /* Adjust Conflict Items */
                    entry.op = SFM_OP_STATUS_CONFLICT;
                }
            }
        }

        if (events & SFM_EVENT_TIME_WAKEUP) {
            if (!(ticks % 10)) {
                cyg_tick_count_t    exe_time_base = cyg_current_time();

                T_D("sfm_thread: 100MSEC-TICKS(%lu | %lu)", ticks_overflow, ticks);

                if ((rc = vtss_sfm_tick()) != VTSS_OK) {
                    T_D("vtss_sfm_tick failed by returning rc=%u", rc);
                }

                T_D("sfm_thread consumes %lu ticks", (ulong)(cyg_current_time() - exe_time_base));
            }

            if ((vtss_sfm_get_interface_event_info(&conflict_chk) == VTSS_OK) &&
                (conflict_chk.event_flag != 0)) {
                for (idx = 0; idx < SFM_NO_OF_SRC_INTF; idx++) {
                    if (conflict_chk.conflict_vid[idx] == 0) {
                        continue;
                    }

                    memset(&entry, 0x0, sizeof(vtss_sfm_t));
                    while (vtss_sfm_get_next_entry(&entry) == VTSS_OK) {
                        if (!entry.active) {
                            continue;
                        }
                        if (entry.fid == entry.vid) {
                            continue;
                        }

                        if (entry.fid != conflict_chk.conflict_vid[idx]) {
                            continue;
                        }

                        /* Adjust Conflict Items */
                        entry.op = SFM_OP_STATUS_CONFLICT;
                    }
                }
            }

            if ((ticks + 1) == 0xFFFFFFFF) {
                ticks = 0xFFFFFFFF % 10;
                ticks_overflow++;
            } else {
                ticks++;
            }
        }
    }
}
#endif /* VTSS_FEATURE_SFM */

static BOOL ipmcReady = FALSE;

void ipmc_sm_thread(cyg_addrword_t data)
{
    cyg_handle_t            ipmcCounter;
    cyg_flag_value_t        events;
#if IPMC_THREAD_EXE_SUPP
    cyg_tick_count_t        last_exe_time;
#endif /* IPMC_THREAD_EXE_SUPP */
    ipmc_time_t             exe_time_base, exe_time_diff;
    switch_iter_t           sit;
    port_iter_t             pit;
    BOOL                    sync_mgmt_flag, sync_mgmt_done, proxy_done, ipmc_link = FALSE;
    u32                     idx, ticks = 0, ticks_overflow = 0, delay_cnt = 0;
    ipmc_lib_mgmt_info_t    *sys_mgmt;

    T_D("enter ipmc_sm_thread");

    IPMC_CRIT_ENTER();
    cyg_flag_init(&ipmc_global.vlan_entry_flags);
    cyg_flag_init(&ipmc_global.dynamic_router_ports_getting_flags);

    /* Initialize running data structure */
    vtss_ipmc_init();
    /* Initialize MSG-RX */
    (void) ipmc_stacking_register();
    IPMC_CRIT_EXIT();

    /* wait for IP task/stack is ready */
    T_I("ipmc_sm_thread init delay start");
    while (!ipmc_link && (delay_cnt < IPMC_THREAD_START_DELAY_CNT)) {
#if VTSS_SWITCH_STACKABLE
        if (!ipmc_in_stacking_ready) {
            VTSS_OS_MSLEEP(IPMC_THREAD_SECOND_DEF);
            delay_cnt++;

            continue;
        }
#endif /* VTSS_SWITCH_STACKABLE */

        if (delay_cnt > IPMC_THREAD_MIN_DELAY_CNT) {
            IPMC_CRIT_ENTER();
            (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
            while (port_iter_getnext(&pit)) {
                if (ipmc_port_status[VTSS_ISID_LOCAL][pit.iport]) {
                    ipmc_link = TRUE;
                    break;
                }
            }
            IPMC_CRIT_EXIT();
        }

        if (!ipmc_link) {
            VTSS_OS_MSLEEP(IPMC_THREAD_SECOND_DEF);
            delay_cnt++;
        }
    }
    T_I("ipmc_sm_thread init delay done");

    /* Initialize Periodical Wakeup Timer(Alarm)  */
    cyg_clock_to_counter(cyg_real_time_clock(), &ipmcCounter);
    cyg_alarm_create(ipmcCounter,
                     ipmc_sm_timer_isr,
                     0,
                     &ipmc_sm_alarm_handle,
                     &ipmc_sm_alarm);
    cyg_alarm_initialize(ipmc_sm_alarm_handle, cyg_current_time() + 1, IPMC_THREAD_TICK_TIME / ECOS_MSECS_PER_HWTICK);
    cyg_alarm_enable(ipmc_sm_alarm_handle);

#if defined(VTSS_FEATURE_SFM)
    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      sfm_thread,
                      0,
                      "SFM",
                      sfm_thread_stack,
                      sizeof(sfm_thread_stack),
                      &sfm_thread_handle,
                      &sfm_thread_block);
    /* Start SFM Thread */
    cyg_thread_resume(sfm_thread_handle);
#endif /* VTSS_FEATURE_SFM */

    if (!IPMC_MEM_SYSTEM_MTAKE(sys_mgmt, sizeof(ipmc_lib_mgmt_info_t))) {
        T_E("SNP sys_mgmt allocation failure");
    }
    T_D("ipmc_sm_thread start");
    ipmcReady = TRUE;
    sync_mgmt_flag = TRUE;

#if IPMC_THREAD_EXE_SUPP
    last_exe_time = cyg_current_time();
#endif /* IPMC_THREAD_EXE_SUPP */
    while (ipmcReady && sys_mgmt) {
        events = cyg_flag_wait(&ipmc_sm_events, IPMC_EVENT_ANY,
                               CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        if (events & IPMC_EVENT_CONFBLK_COMMIT) {
            ipmc_conf_blk_t *blk_ptr = NULL;
            u32             blk_size = 0;

            if ((blk_ptr = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_IPMC_CONF, &blk_size)) != NULL) {
                IPMC_CRIT_ENTER();
                memcpy(&blk_ptr->ipv4_conf, &ipmc_global.ipv4_conf, sizeof(ipmc_configuration_t));
                memcpy(&blk_ptr->ipv6_conf, &ipmc_global.ipv6_conf, sizeof(ipmc_configuration_t));
                IPMC_CRIT_EXIT();

                conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_IPMC_CONF);
            }
        }
#endif

        if (events & IPMC_EVENT_MASTER_UP) {
            sync_mgmt_flag = TRUE;
        }

        if (events & IPMC_EVENT_MASTER_DOWN) {
            sync_mgmt_flag = FALSE;
        }

        if (events & IPMC_EVENT_PKT4_HANDLER) {
            BOOL    v4_mode;

            IPMC_CRIT_ENTER();
            v4_mode = ipmc_global.ipv4_conf.global.ipmc_mode_enabled;
            IPMC_CRIT_EXIT();

            if (v4_mode) {
                (void) ipmc_lib_packet_register(IPMC_OWNER_SNP4, ipmcsnp_rx_packet_callback);
            } else {
                (void) ipmc_lib_packet_unregister(IPMC_OWNER_SNP4);
            }
        }

        if (events & IPMC_EVENT_PKT6_HANDLER) {
            BOOL    v6_mode;

            IPMC_CRIT_ENTER();
            v6_mode = ipmc_global.ipv6_conf.global.ipmc_mode_enabled;
            IPMC_CRIT_EXIT();

            if (v6_mode) {
                (void) ipmc_lib_packet_register(IPMC_OWNER_SNP6, ipmcsnp_rx_packet_callback);
            } else {
                (void) ipmc_lib_packet_unregister(IPMC_OWNER_SNP6);
            }
        }

        if (events & IPMC_EVENT_SWITCH_DEL) {
            vtss_isid_t isid_del;

            T_D("IPMC_EVENT_SWITCH_DEL");

            (void) ipmc_lib_time_curr_get(&exe_time_base);

            IPMC_CRIT_ENTER();

            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                isid_del = sit.isid;

                if (!(ipmc_switch_event_value[ipmc_lib_isid_convert(TRUE, isid_del)] & IPMC_EVENT_VALUE_SW_DEL)) {
                    continue;
                }

                ipmc_switch_event_value[ipmc_lib_isid_convert(TRUE, isid_del)] &= ~IPMC_EVENT_VALUE_SW_DEL;
                if (ipmc_lib_isid_is_local(isid_del)) {
                    ipmc_switch_event_value[VTSS_ISID_LOCAL] &= ~IPMC_EVENT_VALUE_SW_DEL;
                }
            }

            IPMC_CRIT_EXIT();

            (void) ipmc_lib_time_diff_get(TRUE, FALSE, "IPMC_EVENT_SWITCH_DEL", &exe_time_base, &exe_time_diff);
        }

        if (events & IPMC_EVENT_SWITCH_ADD) {
            vtss_isid_t isid_add;
            u32         isid_set;

            T_D("IPMC_EVENT_SWITCH_ADD");

            (void) ipmc_lib_time_curr_get(&exe_time_base);

            sync_mgmt_done = FALSE;
            if (!ipmc_lib_system_mgmt_info_cpy(sys_mgmt)) {
                sync_mgmt_done = TRUE;  /* Not Ready Yet! */
            }

            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
            while (switch_iter_getnext(&sit)) {
                isid_add = sit.isid;

                IPMC_CRIT_ENTER();
                isid_set = ipmc_switch_event_value[ipmc_lib_isid_convert(TRUE, isid_add)] & IPMC_EVENT_VALUE_SW_ADD;
                IPMC_CRIT_EXIT();

                if (isid_set == 0) {
                    continue;
                }

                /* IGMP */
                (void) ipmc_stacking_set_leave_proxy(isid_add, IPMC_IP_VERSION_IGMP);
                (void) ipmc_stacking_set_proxy(isid_add, IPMC_IP_VERSION_IGMP);
                (void) ipmc_stacking_set_router_port(isid_add, IPMC_IP_VERSION_IGMP);
                (void) ipmc_stacking_set_fastleave_port(isid_add, IPMC_IP_VERSION_IGMP);
                (void) ipmc_stacking_set_unreg_flood(isid_add, IPMC_IP_VERSION_IGMP);
                (void) ipmc_stacking_set_ssm_range(isid_add, IPMC_IP_VERSION_IGMP);
                ipmc_update_vlan_tab(isid_add, IPMC_IP_VERSION_IGMP);
                (void) ipmc_stacking_set_throttling_number(isid_add, IPMC_IP_VERSION_IGMP);
                (void) ipmc_stacking_set_grp_filtering(isid_add, IPMC_IP_VERSION_IGMP);
                (void) ipmc_stacking_set_mode(isid_add, IPMC_IP_VERSION_IGMP, FALSE);
                /* MLD */
                (void) ipmc_stacking_set_leave_proxy(isid_add, IPMC_IP_VERSION_MLD);
                (void) ipmc_stacking_set_proxy(isid_add, IPMC_IP_VERSION_MLD);
                (void) ipmc_stacking_set_router_port(isid_add, IPMC_IP_VERSION_MLD);
                (void) ipmc_stacking_set_fastleave_port(isid_add, IPMC_IP_VERSION_MLD);
                (void) ipmc_stacking_set_unreg_flood(isid_add, IPMC_IP_VERSION_MLD);
                (void) ipmc_stacking_set_ssm_range(isid_add, IPMC_IP_VERSION_MLD);
                ipmc_update_vlan_tab(isid_add, IPMC_IP_VERSION_MLD);
                (void) ipmc_stacking_set_throttling_number(isid_add, IPMC_IP_VERSION_MLD);
                (void) ipmc_stacking_set_grp_filtering(isid_add, IPMC_IP_VERSION_MLD);
                (void) ipmc_stacking_set_mode(isid_add, IPMC_IP_VERSION_MLD, FALSE);

                if (!sync_mgmt_done) {
                    (void) ipmc_stacking_sync_mgmt_conf(isid_add, sys_mgmt);
                }

                IPMC_CRIT_ENTER();
                ipmc_switch_event_value[ipmc_lib_isid_convert(TRUE, isid_add)] &= ~IPMC_EVENT_VALUE_SW_ADD;
                if (ipmc_lib_isid_is_local(isid_add)) {
                    ipmc_switch_event_value[VTSS_ISID_LOCAL] &= ~IPMC_EVENT_VALUE_SW_ADD;
                }
                IPMC_CRIT_EXIT();
            }

            (void) ipmc_lib_time_diff_get(TRUE, FALSE, "IPMC_EVENT_SWITCH_ADD", &exe_time_base, &exe_time_diff);
        }

        sync_mgmt_done = proxy_done = FALSE;
        if (events & IPMC_EVENT_SM_TIME_WAKEUP) {
            u8  exe_round = 1;
#if IPMC_THREAD_EXE_SUPP
            u32 supply_ticks = 0, diff_exe_time = ipmc_lib_diff_u32_wrap_around((u32)last_exe_time, (u32)cyg_current_time());

            if ((diff_exe_time * ECOS_MSECS_PER_HWTICK) > (2 * IPMC_THREAD_TICK_TIME)) {
                supply_ticks = ((diff_exe_time * ECOS_MSECS_PER_HWTICK) / IPMC_THREAD_TICK_TIME) - 1;
            }

            if (supply_ticks > 0) {
                exe_round += ipmc_lib_calc_thread_tick(&ticks, supply_ticks, IPMC_THREAD_TIME_UNIT_BASE, &ticks_overflow);
            }

            last_exe_time = cyg_current_time();
#endif /* IPMC_THREAD_EXE_SUPP */

            if (msg_switch_is_master() && ipmc_lib_system_mgmt_info_chg(sys_mgmt)) {
                sync_mgmt_done = (ipmc_stacking_sync_mgmt_conf(VTSS_ISID_GLOBAL, sys_mgmt) == VTSS_OK);
            }

            if (!(ticks % IPMC_THREAD_TIME_UNIT_BASE)) {
                for (; exe_round > 0; exe_round--) {
                    if (msg_switch_is_master() && sync_mgmt_flag) {
                        if (!sync_mgmt_done) {
                            if (ipmc_lib_system_mgmt_info_cpy(sys_mgmt)) {
                                sync_mgmt_flag = (ipmc_stacking_sync_mgmt_conf(VTSS_ISID_GLOBAL, sys_mgmt) != VTSS_OK);
                            }
                        } else {
                            sync_mgmt_flag = FALSE;
                        }
                    }

                    T_N("ipmc_sm_thread: %dMSEC-TICKS(%u | %u)", IPMC_THREAD_TICK_TIME, ticks_overflow, ticks);

                    IPMC_CRIT_ENTER();
                    (void) ipmc_lib_time_curr_get(&exe_time_base);

                    vtss_ipmc_tick_gen();

                    if (ipmc_global.ipv4_conf.global.ipmc_mode_enabled ||
                        ipmc_global.ipv6_conf.global.ipmc_mode_enabled) {
                        for (idx = 0; idx < SNP_NUM_OF_SUPPORTED_INTF; idx++) {
                            vtss_ipmc_tick_intf_tmr(idx);
                        }

                        vtss_ipmc_tick_intf_rxmt();
                        vtss_ipmc_tick_group_tmr();
                    }

                    if (events & IPMC_EVENT_PROXY_LOCAL) {
                        vtss_ipmc_tick_proxy(TRUE);
                    } else {
                        vtss_ipmc_tick_proxy(FALSE);
                    }
                    proxy_done = TRUE;

                    (void) ipmc_lib_time_diff_get(FALSE, FALSE, NULL, &exe_time_base, &exe_time_diff);
                    IPMC_CRIT_EXIT();

                    T_N("IPMC_EVENT_SM_TIME_WAKEUP consumes %u.%um%uu",
                        exe_time_diff.sec, exe_time_diff.msec, exe_time_diff.usec);
                }
            }

            (void) ipmc_lib_calc_thread_tick(&ticks, 1, IPMC_THREAD_TIME_UNIT_BASE, &ticks_overflow);
        }

        if (events & IPMC_EVENT_PROXY_LOCAL) {
            if (!proxy_done) {
                IPMC_CRIT_ENTER();
                vtss_ipmc_tick_proxy(TRUE);
                IPMC_CRIT_EXIT();
            }
        }
    }

    IPMC_MEM_SYSTEM_MGIVE(sys_mgmt);
    ipmcReady = FALSE;
    T_W("exit ipmc_sm_thread");
}

#if 0 /* etliang */
static cyg_handle_t ipmc_db_thread_handle;
static cyg_thread   ipmc_db_thread_block;
static char         ipmc_db_thread_stack[IPMC_THREAD_STACK_SIZE];

/*lint -esym(459, ipmc_db_events) */
static cyg_flag_t   ipmc_db_events;
static cyg_alarm    ipmc_db_alarm;
static cyg_handle_t ipmc_db_alarm_handle;

static void ipmc_db_event_set(cyg_flag_value_t flag)
{
    cyg_flag_setbits(&ipmc_db_events, flag);
}

static void ipmc_db_timer_isr(cyg_handle_t alarm, cyg_addrword_t data)
{
    if (alarm || data) { /* avoid warning */
    }

    ipmc_db_event_set(IPMC_EVENT_DB_TIME_WAKEUP);
}

void ipmc_db_thread(cyg_addrword_t data)
{
    cyg_handle_t        ipmcCounter;
    cyg_flag_value_t    events;
#if IPMC_THREAD_EXE_SUPP
    cyg_tick_count_t    last_exe_time;
#endif /* IPMC_THREAD_EXE_SUPP */
    cyg_tick_count_t    exe_time_base;
    u32                 idx, ticks = 0, ticks_overflow = 0;

    T_D("enter ipmc_db_thread");

    /* wait for IPMC is ready */
    while (!ipmcReady) {
        VTSS_OS_MSLEEP(IPMC_THREAD_SECOND_DEF);
    }

    /* Initialize Periodical Wakeup Timer(Alarm)  */
    cyg_clock_to_counter(cyg_real_time_clock(), &ipmcCounter);
    cyg_alarm_create(ipmcCounter,
                     ipmc_db_timer_isr,
                     0,
                     &ipmc_db_alarm_handle,
                     &ipmc_db_alarm);
    cyg_alarm_initialize(ipmc_db_alarm_handle, cyg_current_time() + 1, IPMC_THREAD_TICK_TIME / ECOS_MSECS_PER_HWTICK);
    cyg_alarm_enable(ipmc_db_alarm_handle);

    T_D("ipmc_db_thread start");

#if IPMC_THREAD_EXE_SUPP
    last_exe_time = cyg_current_time();
#endif /* IPMC_THREAD_EXE_SUPP */
    while (ipmcReady) {
        events = cyg_flag_wait(&ipmc_db_events, IPMC_EVENT_ANY,
                               CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);

        if (events & IPMC_EVENT_DB_TIME_WAKEUP) {
            u8  exe_round = 1;
#if IPMC_THREAD_EXE_SUPP
            u32 supply_ticks = 0, diff_exe_time = ipmc_lib_diff_u32_wrap_around((u32)last_exe_time, (u32)cyg_current_time());

            if ((diff_exe_time * ECOS_MSECS_PER_HWTICK) > (2 * IPMC_THREAD_TICK_TIME)) {
                supply_ticks = ((diff_exe_time * ECOS_MSECS_PER_HWTICK) / IPMC_THREAD_TICK_TIME) - 1;
            }

            if (supply_ticks > 0) {
                exe_round += ipmc_lib_calc_thread_tick(&ticks, supply_ticks, IPMC_THREAD_TIME_UNIT_BASE, &ticks_overflow);
            }

            last_exe_time = cyg_current_time();
#endif /* IPMC_THREAD_EXE_SUPP */
            if (!(ticks % IPMC_THREAD_TIME_UNIT_BASE)) {
                for (; exe_round > 0; exe_round--) {
                    IPMC_CRIT_ENTER();
                    T_N("ipmc_db_thread: %dMSEC-TICKS(%lu | %lu)", IPMC_THREAD_TICK_TIME, ticks_overflow, ticks);
                    exe_time_base = cyg_current_time();

                    for (idx = 0; idx < SNP_NUM_OF_SUPPORTED_INTF; idx++) {
                        vtss_ipmc_tick_intf_grp_tmr(idx);
                    }

                    T_N("IPMC_EVENT_DB_TIME_WAKEUP consumes %lu ticks", (ulong)(cyg_current_time() - exe_time_base));
                    IPMC_CRIT_EXIT();
                }
            }

            (void) ipmc_lib_calc_thread_tick(&ticks, 1, IPMC_THREAD_TIME_UNIT_BASE, &ticks_overflow);
        }
    }

    T_W("exit ipmc_db_thread");
}
#endif /* etliang */

vtss_rc ipmc_init(vtss_init_data_t *data)
{
    ipmc_msg_id_t       idx;
    vtss_isid_t         isid = data->isid;
    cyg_tick_count_t    exe_time_base = cyg_current_time();

    switch ( data->cmd ) {
    case INIT_CMD_INIT:
#if VTSS_TRACE_ENABLED
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&snp_trace_reg, snp_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&snp_trace_reg);
#endif /* VTSS_TRACE_ENABLED */

        for (idx = IPMC_MSG_ID_MODE_SET_REQ; idx < IPMC_MSG_MAX_ID; idx++) {
            switch ( idx ) {
            case IPMC_MSG_ID_MODE_SET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_mode_set_req_t);
                break;
            case IPMC_MSG_ID_SYS_MGMT_SET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_sys_mgmt_set_req_t);
                break;
            case IPMC_MSG_ID_LEAVE_PROXY_SET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_leave_proxy_set_req_t);
                break;
            case IPMC_MSG_ID_PROXY_SET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_proxy_set_req_t);
                break;
            case IPMC_MSG_ID_SSM_RANGE_SET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_ssm_range_set_req_t);
                break;
            case IPMC_MSG_ID_UNREG_FLOOD_SET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_unreg_flood_set_req_t);
                break;
            case IPMC_MSG_ID_ROUTER_PORT_SET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_router_port_set_req_t);
                break;
            case IPMC_MSG_ID_FAST_LEAVE_PORT_SET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_fast_leave_port_set_req_t);
                break;
            case IPMC_MSG_ID_THROLLTING_MAX_NO_SET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_throllting_max_no_set_req_t);
                break;
            case IPMC_MSG_ID_VLAN_SET_REQ:
            case IPMC_MSG_ID_VLAN_ENTRY_SET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_vlan_set_req_t);
                break;
            case IPMC_MSG_ID_STAT_COUNTER_CLEAR_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_stat_counter_clear_req_t);
                break;
            case IPMC_MSG_ID_STP_PORT_CHANGE_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_stp_port_change_set_req_t);
                break;
            case IPMC_MSG_ID_VLAN_ENTRY_GET_REQ:
            case IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REQ:
            case IPMC_MSG_ID_GROUP_SRCLIST_WALK_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_vlan_entry_get_req_t);
                break;
            case IPMC_MSG_ID_VLAN_ENTRY_GET_REP:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_vlan_entry_get_rep_t);
                break;
            case IPMC_MSG_ID_VLAN_GROUP_ENTRY_WALK_REP:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_vlan_group_entry_get_rep_t);
                break;
            case IPMC_MSG_ID_GROUP_SRCLIST_WALK_REP:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_group_srclist_get_rep_t);
                break;
            case IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REQ:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_dyn_rtpt_get_req_t);
                break;
            case IPMC_MSG_ID_DYNAMIC_ROUTER_PORTS_GET_REP:
                ipmc_global.msize[idx] = sizeof(ipmc_msg_dynamic_router_ports_get_rep_t);
                break;
            case IPMC_MSG_ID_PORT_GROUP_FILTERING_SET_REQ:
            default:
                /* Give the MAX */
                ipmc_global.msize[idx] = sizeof(ipmc_msg_port_group_filtering_set_req_t);
                break;
            }

            ipmc_global.msg[idx] = VTSS_MALLOC(ipmc_global.msize[idx]);
            if (ipmc_global.msg[idx] == NULL) {
                T_W("IPMC_ASSERT(INIT_CMD_INIT)");
                for (;;) {}
            }
        }

        /* Create semaphore for critical regions */
        critd_init(&ipmc_global.crit, "IPMC_global.crit", VTSS_MODULE_ID_IPMC, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        IPMC_CRIT_EXIT();

        critd_init(&ipmc_global.get_crit, "IPMC_global.get_crit", VTSS_MODULE_ID_IPMC, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        IPMC_GET_CRIT_EXIT();

        (void) ipmc_lib_common_init();
        /* Initialized RX semaphore */
        (void) ipmc_lib_packet_init();

#ifdef VTSS_SW_OPTION_VCLI
        ipmc_cli_req_init();
#endif /* VTSS_SW_OPTION_VCLI */

#ifdef VTSS_SW_OPTION_ICFG
        if (ipmc_snp_icfg_init() != VTSS_OK) {
            T_E("ipmc_snp_icfg_init failed!");
        }
#endif /* VTSS_SW_OPTION_ICFG */

        vlan_membership_change_register(VTSS_MODULE_ID_IPMC, ipmc_vlan_changed);

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&ipmc_global.request.sem, 1);
        VTSS_OS_SEM_CREATE(&ipmc_global.reply.sem, 1);

        memset(ipmc_switch_event_value, 0x0, sizeof(ipmc_switch_event_value));

        /* Initialize IPMC-EVENT groups */
        cyg_flag_init(&ipmc_sm_events);
#if 0 /* etliang */
        cyg_flag_init(&ipmc_db_events);
#endif /* etliang */

        memset(ipmc_port_status, 0x0, sizeof(ipmc_port_status));

        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          ipmc_sm_thread,
                          0,
                          "IPMC_SNP",
                          ipmc_sm_thread_stack,
                          sizeof(ipmc_sm_thread_stack),
                          &ipmc_sm_thread_handle,
                          &ipmc_sm_thread_block);
        cyg_thread_resume(ipmc_sm_thread_handle);

        T_I("IPMC-INIT_CMD_INIT consumes ID%d:%u ticks", isid, (ulong)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_START:
        T_I("START: ISID->%d", isid);

        (void) port_change_register(VTSS_MODULE_ID_IPMC, ipmc_port_state_change_callback);
        (void) l2_stp_state_change_register(ipmc_stp_port_state_change_callback);
        /* Register for Port GLOBAL change callback */
        (void) port_global_change_register(VTSS_MODULE_ID_IPMC, ipmc_global_port_state_change_callback);

        T_D("IPMC-INIT_CMD_START consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_CONF_DEF:
        T_I("CONF_DEF: ISID->%d", isid);

        if (isid == VTSS_ISID_GLOBAL) {
            vtss_vid_t              intf_idx;
            ipmc_port_bfs_t         vlan_ports;
            ipmc_conf_intf_entry_t  ipmc_intf, *entry;

            for (intf_idx = 0; intf_idx < IPMC_VLAN_MAX; intf_idx++) {
                IPMC_CRIT_ENTER();
                entry = &ipmc_global.ipv4_conf.ipmc_conf_intf_entries[intf_idx];
                if (!entry->valid) {
                    IPMC_CRIT_EXIT();
                    continue;
                }
                if (vtss_ipmc_get_intf_entry(entry->vid, IPMC_IP_VERSION_IGMP) == NULL) {
                    IPMC_CRIT_EXIT();
                    continue;
                }
                memcpy(&ipmc_intf, entry, sizeof(ipmc_conf_intf_entry_t));
                IPMC_CRIT_EXIT();

                memset(&vlan_ports, 0x0, sizeof(ipmc_port_bfs_t));
                (void) ipmc_stacking_set_intf(VTSS_ISID_GLOBAL, IPMC_OP_DEL, &ipmc_intf, &vlan_ports, IPMC_IP_VERSION_IGMP);
            }

            for (intf_idx = 0; intf_idx < IPMC_VLAN_MAX; intf_idx++) {
                IPMC_CRIT_ENTER();
                entry = &ipmc_global.ipv6_conf.ipmc_conf_intf_entries[intf_idx];
                if (!entry->valid) {
                    IPMC_CRIT_EXIT();
                    continue;
                }
                if (vtss_ipmc_get_intf_entry(entry->vid, IPMC_IP_VERSION_MLD) == NULL) {
                    IPMC_CRIT_EXIT();
                    continue;
                }
                memcpy(&ipmc_intf, entry, sizeof(ipmc_conf_intf_entry_t));
                IPMC_CRIT_EXIT();

                memset(&vlan_ports, 0x0, sizeof(ipmc_port_bfs_t));
                (void) ipmc_stacking_set_intf(VTSS_ISID_GLOBAL, IPMC_OP_DEL, &ipmc_intf, &vlan_ports, IPMC_IP_VERSION_MLD);
            }

            /* Turn-Off protocol anyway */
            (void) ipmc_stacking_set_mode(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_IGMP, TRUE);
            (void) ipmc_stacking_set_mode(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_MLD, TRUE);

            /* Reset stack configuration */
            (void) ipmc_conf_read(TRUE);

            /* IGMP */
            (void) ipmc_stacking_set_unreg_flood(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_IGMP);
            (void) ipmc_stacking_set_leave_proxy(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_IGMP);
            (void) ipmc_stacking_set_proxy(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_IGMP);
            (void) ipmc_stacking_set_ssm_range(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_IGMP);
            (void) ipmc_stacking_set_router_port(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_IGMP);
            (void) ipmc_stacking_set_fastleave_port(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_IGMP);
            (void) ipmc_stacking_set_throttling_number(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_IGMP);
            (void) ipmc_stacking_set_grp_filtering(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_IGMP);
            /* MLD */
            (void) ipmc_stacking_set_unreg_flood(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_MLD);
            (void) ipmc_stacking_set_leave_proxy(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_MLD);
            (void) ipmc_stacking_set_proxy(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_MLD);
            (void) ipmc_stacking_set_ssm_range(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_MLD);
            (void) ipmc_stacking_set_router_port(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_MLD);
            (void) ipmc_stacking_set_fastleave_port(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_MLD);
            (void) ipmc_stacking_set_throttling_number(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_MLD);
            (void) ipmc_stacking_set_grp_filtering(VTSS_ISID_GLOBAL, IPMC_IP_VERSION_MLD);
        }

        T_D("IPMC-INIT_CMD_CONF_DEF consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_MASTER_UP:
        T_I("MASTER_UP: ISID->%d", isid);

        /* Read configuration */
        (void) ipmc_conf_read(FALSE);
        ipmc_sm_event_set(IPMC_EVENT_MASTER_UP);

        T_D("IPMC-INIT_CMD_MASTER_UP consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_MASTER_DOWN:
        T_I("MASTER_DOWN: ISID->%d", isid);

#if VTSS_SWITCH_STACKABLE
        ipmc_in_stacking_ready = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */
        ipmc_sm_event_set(IPMC_EVENT_MASTER_DOWN);

        T_D("IPMC-INIT_CMD_MASTER_DOWN consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_SWITCH_ADD:
        T_I("SWITCH_ADD: ISID->%d", isid);

        IPMC_CRIT_ENTER();
        ipmc_switch_event_value[ipmc_lib_isid_convert(TRUE, isid)] |= IPMC_EVENT_VALUE_SW_ADD;
        if (ipmc_lib_isid_is_local(isid)) {
            ipmc_switch_event_value[VTSS_ISID_LOCAL] |= IPMC_EVENT_VALUE_SW_ADD;
        }
        IPMC_CRIT_EXIT();

        ipmc_sm_event_set(IPMC_EVENT_SWITCH_ADD);

#if VTSS_SWITCH_STACKABLE
        ipmc_in_stacking_ready = TRUE;
#endif /* VTSS_SWITCH_STACKABLE */

        T_D("IPMC-INIT_CMD_SWITCH_ADD consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

        break;
    case INIT_CMD_SWITCH_DEL:
        T_I("SWITCH_DEL: ISID->%d", isid);

        IPMC_CRIT_ENTER();
        ipmc_switch_event_value[ipmc_lib_isid_convert(TRUE, isid)] |= IPMC_EVENT_VALUE_SW_DEL;
        if (ipmc_lib_isid_is_local(isid)) {
            ipmc_switch_event_value[VTSS_ISID_LOCAL] |= IPMC_EVENT_VALUE_SW_DEL;
        }
        IPMC_CRIT_EXIT();

        ipmc_sm_event_set(IPMC_EVENT_SWITCH_DEL);

        T_D("IPMC-INIT_CMD_SWITCH_DEL consumes %u ticks", (ulong)(cyg_current_time() - exe_time_base));

        break;
    default:
        break;
    }

    return 0;
}
