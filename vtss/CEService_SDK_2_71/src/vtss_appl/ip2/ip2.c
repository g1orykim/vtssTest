/*

 Vitesse API software.

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

// Thread conventions:
// IP2_*() static functions must not acquire lock
// IP2_*() may call IP2_*()
// IP2_*() may NOT call vtss_ip2_*()

// vtss_ip2_*() public functions may acquire lock
// vtss_ip2_*() may call IP2_*() in LOCKED sections
// vtss_ip2_*() may call vtss_ip2_* in UNLOCKED sections
// vtss_ip2_*() may NOT call vtss_ip2_* in LOCKED sections

#include "ip2_api.h"
#include "ip2_priv.h"
#include "ip2_utils.h"
#include "ip2_os_api.h"
#include "ip2_chip_api.h"
#include "ip2_trace.h"
#include "ip2_misc_api.h"
#ifdef VTSS_SW_OPTION_ICLI
#include "ip2_icli_priv.h"
#endif
#include "ip2_ecos_driver.h"
#ifdef VTSS_SW_OPTION_SNMP
#include "ip2_snmp.h"
#endif
#ifdef VTSS_SW_OPTION_DNS
#include "ip_dns_api.h"
#endif /* VTSS_SW_OPTION_DNS */
#include "l2proto_api.h"        /* For L2 port state */
#include "msg_api.h"            /* msg_switch_is_master() */
#include "vtss_trace_api.h"
#include "conf_api.h"
#include "critd_api.h"
#include "misc_api.h"           /* misc_ipv6_txt() */
#include "vtss_intrusive_list.h"
#include "dhcp_client_api.h"
#include "vtss_avl_tree_api.h"
#include "vlan_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_IP2
/*
 * Semaphore stuff.
 */
static critd_t ip2_crit;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_MODULE_ID_IP2,
    .name      = "ip2",
    .descr     = "IP2"
};

static vtss_trace_grp_t trace_grps[VTSS_TRACE_IP2_GRP_CNT] = {
    [VTSS_TRACE_IP2_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_RXPKT_DUMP] = {
        .name      = "rxpktdump",
        .descr     = "Dump of received packets (lvl = noise)",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [VTSS_TRACE_GRP_TXPKT_DUMP] = {
        .name      = "txpktdump",
        .descr     = "Dump of transmitted packets (lvl = noise)",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [VTSS_TRACE_IP2_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [VTSS_TRACE_IP2_GRP_CHIP] = {
        .name      = "chip",
        .descr     = "Chip layer",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_IP2_GRP_OS] = {
        .name      = "os",
        .descr     = "OS layer",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_IP2_GRP_OS_ROUTER_SOCKET] = {
        .name      = "os-rt",
        .descr     = "OS router socket",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [VTSS_TRACE_IP2_GRP_OS_DRIVER] = {
        .name      = "os-drv",
        .descr     = "OS driver",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .ringbuf   = 1, // Do not attempt to print directly to the console, since that'll use mutexes to get hold of the print buffers.
        .irq       = 1, // Do not attempt to wait for anything, dear trace module.
    },
    [VTSS_TRACE_IP2_GRP_SNMP] = {
        .name      = "snmp",
        .descr     = "SNMP helper",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
};

#  define IP2_CRIT_ENTER()                                                  \
      critd_enter(&ip2_crit, VTSS_TRACE_IP2_GRP_CRIT, VTSS_TRACE_LVL_NOISE, \
                  __FILE__, __LINE__)
#  define IP2_CRIT_EXIT()                                                   \
      critd_exit(&ip2_crit, VTSS_TRACE_IP2_GRP_CRIT, VTSS_TRACE_LVL_NOISE,  \
                 __FILE__, __LINE__)

#  define IP2_CRIT_ASSERT_LOCKED()                 \
    critd_assert_locked(&ip2_crit,                 \
                        TRACE_GRP_CRIT,            \
                        __FILE__, __LINE__)
#else
#  define IP2_CRIT_ENTER() critd_enter(&ip2_crit)
#  define IP2_CRIT_EXIT()  critd_exit( &ip2_crit)
#  define IP2_CRIT_ASSERT_LOCKED() critd_assert_locked(&ip2_crit)
#endif /* VTSS_TRACE_ENABLED */

#define IP2_CRIT_RETURN(T, X) \
do {                          \
    T __val = (X);            \
    IP2_CRIT_EXIT();          \
    return __val;             \
} while(0)

#define IP2_CRIT_RETURN_RC(X)   \
    IP2_CRIT_RETURN(vtss_rc, X)

/* IP2 messages IDs */
typedef enum {
    IP2_MSG_ID_HELLO_REQ,       /* M->S Request all slaves to say hello */
    IP2_MSG_ID_HELLO_RES,       /* S->M Slaves says hello */

    IP2_MSG_ID_FWD_STATE_REQ,   /* VID fwd state request */
    IP2_MSG_ID_FWD_STATE_RSP,   /* VID fwd state request */
    IP2_MSG_ID_IF_STATE,        /* M->S Update the cached if status on slaves */
} ip2_msg_id_t;

static char *IP2_msg_id_to_str(ip2_msg_id_t msg_id)
{
#define CASE(X) case X: return #X
    switch (msg_id) {
        CASE(IP2_MSG_ID_HELLO_REQ);
        CASE(IP2_MSG_ID_HELLO_RES);
        CASE(IP2_MSG_ID_FWD_STATE_REQ);
        CASE(IP2_MSG_ID_FWD_STATE_RSP);
        CASE(IP2_MSG_ID_IF_STATE);
    default:
        return "***Unknown Message ID***";
    }
#undef CASE
}

typedef struct {
    struct {
        vtss_vid_t vid;
        ip2_vif_fwd_t fwd;
    } iface[IP2_MAX_INTERFACES];
} fwd_state_t;

typedef struct {
    u32 size;
    vtss_if_status_t data[IP2_MAX_STATUS_OBJS];
} if_status_t;

typedef struct ip2_msg {
    ip2_msg_id_t msg_id;

    /* Message data, depending on message ID */
    union {
        /* IP2_MSG_ID_FWD_STATE_REQ, IP2_MSG_ID_FWD_STATE_RSP */
        fwd_state_t fwd_state;
        if_status_t if_status;
    } data;
} ip2_msg_t;

#define IP2_MSG_BUFS  2
static void *IP2_msg_pool;
static BOOL IP2_is_master = FALSE;
//static int  IP2_master_id = -1;
static BOOL IP2_slave_active[VTSS_ISID_END];
static BOOL IP2_hello_pending_res = FALSE;

#define MAX_SUBSCRIBER (32)
static vtss_ip2_if_callback_t IP2_subscribers[MAX_SUBSCRIBER];
static if_status_t IP2_if_status_cache;
static u32 IP2_vlan_notify_bf[VTSS_BF_SIZE(VTSS_VIDS)];

typedef struct {
    int  users;
    ip2_route_entry_t route;
} ip2_route_db_entry_t;

/*
 * Forward declarations
 */
static void IP2_flash_write(void);
static vtss_rc IP2_route_add(const vtss_routing_entry_t  *const rt,
                             const vtss_routing_params_t *const params);
static vtss_rc IP2_route_del(const vtss_routing_entry_t  *const rt,
                             const vtss_routing_params_t *const params);
static vtss_rc IP2_ip_clear_type(vtss_if_id_vlan_t  if_id,
                                 vtss_ip_type_t     type);
static void IP2_if_cache_update(void);
static BOOL IP2_if_exists(vtss_if_id_vlan_t if_id);
static vtss_rc IP2_if_del(vtss_if_id_vlan_t if_id);

static vtss_rc IP2_ipv4_conf_set(vtss_if_id_vlan_t            if_id,
                                 const vtss_ip_conf_t        *conf,
                                 BOOL force);
static vtss_rc IP2_ipv6_conf_set(vtss_if_id_vlan_t            if_id,
                                 const vtss_ip_conf_t        *conf);
static vtss_rc IP2_ifs_status_get(vtss_if_status_type_t   type,
                                  const u32               max,
                                  u32                    *cnt,
                                  vtss_if_status_t       *status);
static vtss_rc IP2_if_status_get(vtss_if_status_type_t        type,
                                 vtss_if_id_vlan_t            id,
                                 const u32                    max,
                                 u32                         *cnt,
                                 vtss_if_status_t            *status);
static vtss_rc IP2_if_status_get_first(vtss_if_status_type_t        type,
                                       vtss_if_id_vlan_t            id,
                                       vtss_if_status_t            *status);

static vtss_rc IP2_ip_dhcpc_v4_stop_no_reapply(vtss_if_id_vlan_t if_id);

static int IP2_slave_active_cnt(void)
{
    vtss_isid_t isid;
    int         cnt = 0;

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; ++isid) {
        if (IP2_slave_active[isid] && !msg_switch_is_local(isid)) {
            ++cnt;
        }
    }
    return cnt;
}

static ip2_msg_t *IP2_msg_alloc(ip2_msg_id_t msg_id, u32 ref_cnt)
{
    ip2_msg_t *msg;

    if (ref_cnt == 0) {
        return NULL;
    }

    msg = msg_buf_pool_get(IP2_msg_pool);
    if (!msg) {
        T_E("msg_buf_pool_get(IP2_msg_pool) returned NULL");
        return NULL;
    }

    if (ref_cnt > 1) {
        msg_buf_pool_ref_cnt_set(msg, ref_cnt);
    }

    T_N("msg alloc len type %d => %p", msg_id, msg);
    msg->msg_id = msg_id;
    return msg;
}

static void IP2_msg_free(void *contxt, void *msg, msg_tx_rc_t rc)
{
    T_N("Freeing msg %p", msg);
    (void)msg_buf_pool_put(msg);
}

static void IP2_msg_tx(ip2_msg_t *msg, vtss_isid_t isid, size_t len)
{
    size_t _len = offsetof(ip2_msg_t, data) + len;

    T_N("TX: Sid %u, tx %zd bytes, msg %d %s", isid, _len, msg->msg_id,
        IP2_msg_id_to_str(msg->msg_id));

    msg_tx_adv(NULL, IP2_msg_free, MSG_TX_OPT_DONT_FREE,
               VTSS_MODULE_ID_IP2, isid, msg, _len);
}

static vtss_rc IP2_tx_hello_res(void)
{
    ip2_msg_t *msg;
    T_D("TX hello response");

    if (IP2_is_master) {
        return VTSS_RC_ERROR;
    }

    msg = IP2_msg_alloc(IP2_MSG_ID_HELLO_RES, 1);
    if (msg == NULL) {
        T_E("Alloc error!");
        return VTSS_RC_ERROR;
    }

    IP2_msg_tx(msg, 0, 0);
    return VTSS_RC_OK;
}

static vtss_rc IP2_tx_hello_req(vtss_isid_t isid)
{
    ip2_msg_t *msg;

    T_D("TX hello request to %u", isid);
    if (!IP2_is_master) {
        return VTSS_RC_ERROR;
    }

    msg = IP2_msg_alloc(IP2_MSG_ID_HELLO_REQ, 1);
    if (msg == NULL) {
        T_E("Alloc error!");
        return VTSS_RC_ERROR;
    }

    IP2_msg_tx(msg, isid, 0);
    return VTSS_RC_OK;
}

static void IP2_sync_node(vtss_isid_t isid)
{
    T_I("Syncing node: %u", isid);
    IP2_slave_active[isid] = TRUE;
    IP2_if_cache_update();
}

static i32 route_db_compare(void *elm1, void *elm2)
{
    return vtss_route_compare(&((ip2_route_db_entry_t *) elm1)->route.route,
                              &((ip2_route_db_entry_t *) elm2)->route.route);
}

VTSS_AVL_TREE(IP2_route_db, "IP_routes", VTSS_MODULE_ID_IP2,
              route_db_compare, IP2_MAX_ROUTES_DB)

/* Module config */
static ip2_stack_conf_t IP2_stack_conf;

/*
 * State
 */

#define IFNO_INVALID 0xFF

typedef struct IP2_dhcp_fallback_timer_t_ {
    VTSS_LINKED_LIST_DECL_ITEM(IP2_dhcp_fallback_timer_t_);
    cyg_tick_count_t timeout;
    vtss_vid_t vlan;
} IP2_dhcp_fallback_timer_t;

typedef VTSS_LINKED_LIST_TYPE(IP2_dhcp_fallback_timer_t_) IP2_dhcp_fallback_timer_list_t;
IP2_dhcp_fallback_timer_list_t IP2_dhcp_fallback_timer_list;

// TODO, simplify state model
typedef struct ip2_ifstate {
    /* State data */
    uint                        if_no;              /**< Interface number [0; IP2_MAX_INTERFACES[ */
    vtss_mac_t                  mac_address;        /**< Interface MAC address */
    vtss_vid_t                  vid;                /**< VLAN Id */
    BOOL                        dhcp_v4_valid;      /**< dhcp_v4_address valid */
    vtss_ipv4_t                 dhcp_v4_gw;         /**< dhcp default gw */
    vtss_ip_network_t           cur_ipv4;           /**< Current IPv4 address */
    vtss_ip_network_t           cur_ipv6;           /**< Current IPv6 address */
    IP2_dhcp_fallback_timer_t   dhcpc_start_timer;  /**< Point in time when DHCPC started */

    /* Config data */
    vtss_interface_ip_conf_t   *config;
} ip2_ifstate_t;

static struct {
    vtss_mac_t    master_mac;
    u8            vid2if[VTSS_VIDS];
    ip2_ifstate_t interfaces[IP2_MAX_INTERFACES];
    int           static_routes;
    /* Per unit vid fwd state */
    ip2_vif_fwd_t unit_fwd[VTSS_ISID_CNT][IP2_MAX_INTERFACES];
    /* Combined state */
    ip2_vif_fwd_t master_fwd[IP2_MAX_INTERFACES];
} IP2_state;

static cyg_handle_t ip2_thread_handle;
static cyg_thread   ip2_thread_block;
static char         ip2_thread_stack[THREAD_DEFAULT_STACK_SIZE];

#define CTLFLAG_IFCHANGE            VTSS_BIT(0)
#define CTLFLAG_RTREFRESH           VTSS_BIT(1)
#define CTLFLAG_IFNOTIFY            VTSS_BIT(2)
#define CTLFLAG_NEW_TIMER           VTSS_BIT(3)
static cyg_flag_t   ip2_control_flags;

static void IP2_dhcpc_fallback_timer_consider(ip2_ifstate_t *ifs, BOOL reinit);

/* Internally exposed  */
ip2_ifstate_t *IP2_if2state(vtss_if_id_vlan_t if_id)
{
    ip2_ifstate_t *ifs = NULL;
    uint if_no;

    IP2_CRIT_ASSERT_LOCKED();
    if (if_id >= VLAN_ID_MIN && if_id <= VLAN_ID_MAX && (if_no = IP2_state.vid2if[if_id]) != IFNO_INVALID) {
        ifs = &IP2_state.interfaces[if_no];
    }
    return ifs;
}

vtss_rc ip2_state2if(const ip2_ifstate_t *ifs, vtss_if_id_vlan_t *if_id)
{
    *if_id = ifs->vid;
    return VTSS_RC_OK;
}

static void IP2_ifflags_force_sync(int if_no)
{
    IP2_CRIT_ASSERT_LOCKED();
    /* Force Sync of iff-flags */
    T_I("Poke CTLFLAG_IFCHANGE - ifno %d", if_no);
    IP2_state.master_fwd[if_no] = VIF_FWD_UNDEFINED;
    cyg_flag_setbits(&ip2_control_flags, CTLFLAG_IFCHANGE);
}

static void IP2_if_init(ip2_ifstate_t *if_state,
                        uint if_no,
                        struct ip2_iface_entry *if_conf,
                        vtss_mac_t *mac)
{
    IP2_CRIT_ASSERT_LOCKED();
    memset(if_state, 0, sizeof(ip2_ifstate_t));
    if_state->if_no = if_no;
    if_state->vid = if_conf->vlan_id;
    if_state->dhcpc_start_timer.vlan = if_conf->vlan_id;
    if_state->mac_address = *mac;
    if_state->config = &if_conf->conf;
    IP2_state.vid2if[if_state->vid] = if_no; /* Index 'backlink' */

    /* Force re-evaluation of if-state */
    IP2_ifflags_force_sync(if_no);
}

static ip2_ifstate_t *IP2_if_new(vtss_if_id_vlan_t if_id)
{
    uint if_no;
    ip2_ifstate_t *ifs = NULL;

    IP2_CRIT_ASSERT_LOCKED();
    for (if_no = 0; if_no < IP2_MAX_INTERFACES; if_no++) {
        ip2_ifstate_t *if_state = &IP2_state.interfaces[if_no];
        if (if_state->vid == VTSS_VID_NULL) {
            struct ip2_iface_entry *if_conf = &IP2_stack_conf.interfaces[if_no];
            if_conf->vlan_id = if_id; /* Allocate entry */
            T_I("IP2_if_new: Adding new vlanif %u is ifno: %d", if_id, if_no);
            IP2_if_init(if_state, if_no, if_conf, &IP2_state.master_mac);
            ifs = if_state;
            break;
        }
    }
    return ifs;
}

static vtss_rc IP2_dhcp_clear(vtss_if_id_vlan_t if_id)
{
    ip2_ifstate_t *ifs;
    vtss_rc rc = VTSS_RC_OK;

    IP2_CRIT_ASSERT_LOCKED();

    T_D("IP2_dhcp_clear %u", if_id);
    ifs = IP2_if2state(if_id);

    if (!ifs) {
        return VTSS_RC_ERROR;
    }

    if (ifs->dhcp_v4_gw) {
        vtss_routing_entry_t rt;
        vtss_routing_params_t param;

        vtss_ipv4_default_route(ifs->dhcp_v4_gw, &rt);
        param.owner = VTSS_ROUTING_PARAM_OWNER_DHCP;

        if (IP2_route_del(&rt, &param) != VTSS_RC_OK) {
            T_I("Failed to delete route: " VTSS_IPV4_UC_FORMAT,
                VTSS_IPV4_UC_ARGS(rt.route.ipv4_uc));
        }

        ifs->dhcp_v4_gw = 0;
    }

    if (ifs->dhcp_v4_valid) {
        rc = IP2_ip_clear_type(if_id, VTSS_IP_TYPE_IPV4);
        if (rc != VTSS_RC_OK) {
            T_I("Failed to delete IP address");
        }
        ifs->dhcp_v4_valid = 0;
    }

    return VTSS_RC_OK;
}

static BOOL IP2_if_ip_check(const vtss_ip_network_t *addr,
                            vtss_vid_t ignore)
{
#define MAX_IF 130
#define BUF_SIZE 128
    u32 cnt, i;
    BOOL rc, ipv4;
    vtss_if_status_t *st;
    char buf1[BUF_SIZE];
    char buf2[BUF_SIZE];

    IP2_CRIT_ASSERT_LOCKED();

    if (addr->address.type == VTSS_IP_TYPE_IPV4) {
        ipv4 = TRUE;
    } else if (addr->address.type == VTSS_IP_TYPE_IPV6) {
        ipv4 = FALSE;
    } else {
        T_I("Un-supported address type");
        return FALSE;
    }

    st = VTSS_CALLOC(MAX_IF, sizeof(vtss_if_status_t));
    if (st == NULL) {
        T_E("Alloc error");
        return FALSE;
    }


    //////////////////////////////////////////////////////////////////////////
    // Check against active IP address, regardless where they come from

    // get status IPs
    vtss_if_status_type_t status_type =
        ipv4 ? VTSS_IF_STATUS_TYPE_IPV4 : VTSS_IF_STATUS_TYPE_IPV6;
    if (IP2_ifs_status_get(status_type, MAX_IF, &cnt, st) != VTSS_RC_OK) {
        T_E("Failed to get if status, ipv4=%d", ipv4);
        rc = FALSE;
        goto DONE;
    }

    rc = TRUE;
    for (i = 0; i < cnt; ++i) {
        vtss_ip_network_t tmp;

        if (st[i].if_id.type == VTSS_ID_IF_TYPE_VLAN &&
            st[i].if_id.u.vlan == ignore) {
            continue;
        }

        if (ipv4) {
            tmp.address.type = VTSS_IP_TYPE_IPV4;
            tmp.address.addr.ipv4 = st[i].u.ipv4.net.address;
            tmp.prefix_size = st[i].u.ipv4.net.prefix_size;
        } else {
            tmp.address.type = VTSS_IP_TYPE_IPV6;
            tmp.address.addr.ipv6 = st[i].u.ipv6.net.address;
            tmp.prefix_size = st[i].u.ipv6.net.prefix_size;
        }

        if (vtss_ip_net_overlap(addr, &tmp)) {
            (void) vtss_ip2_ip_network_to_txt(buf1, BUF_SIZE, addr);
            (void) vtss_ip2_ip_network_to_txt(buf2, BUF_SIZE, &tmp);
            T_I("Address %s conflicts with %s", buf1, buf2);
            rc = FALSE;
            goto DONE;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Check against static configured addresses
    for (i = 0; i < IP2_MAX_INTERFACES; ++i) {
        const ip2_ifstate_t *const ifs = &IP2_state.interfaces[i];
        const vtss_ip_conf_t *conf;
        vtss_ip_type_t type;

        if (ifs->vid == VTSS_VID_NULL) {
            continue;
        }

        if (ifs->vid == ignore) {
            continue;
        }

        if (ipv4) {
            type = VTSS_IP_TYPE_IPV4;
            conf = &ifs->config->ipv4;
        } else {
            type = VTSS_IP_TYPE_IPV6;
            conf = &ifs->config->ipv6;
        }

        // No ip configured, no conflict
        if (conf->network.address.type != type) {
            continue;
        }

        // check static ip address
        if (vtss_ip_net_overlap(addr, &conf->network)) {
            (void) vtss_ip2_ip_network_to_txt(buf1, BUF_SIZE, addr);
            (void) vtss_ip2_ip_network_to_txt(buf2, BUF_SIZE, &conf->network);
            T_I("Address %s conflicts with %s", buf1, buf2);
            rc = FALSE;
            goto DONE;
        }
    }

DONE:
    VTSS_FREE(st);
    return rc;
#undef MAX_IF
#undef BUF_SIZE
}

static BOOL IP2_if_ipv4_check(const vtss_ip_network_t *n, vtss_vid_t ignore)
{
    if (n->address.type == VTSS_IP_TYPE_NONE) {
        // We can always accept to delete the address
        return TRUE;
    }

    if (n->address.type != VTSS_IP_TYPE_IPV4) {
        return FALSE;
    }

    return IP2_if_ip_check(n, ignore);
}

static BOOL IP2_if_ipv4_check2(const vtss_ipv4_network_t *v4, vtss_vid_t ignore)
{
    // IPv4 address from here
    vtss_ip_network_t n;
    n.address.type = VTSS_IP_TYPE_IPV4;
    n.prefix_size = v4->prefix_size;
    n.address.addr.ipv4 = v4->address;
    return IP2_if_ip_check(&n, ignore);
}

static BOOL IP2_if_ipv6_check(const vtss_ip_network_t *n, vtss_vid_t ignore)
{
    if (n->address.type == VTSS_IP_TYPE_NONE) {
        // We can always accept to delete the address
        return TRUE;
    }

    if (n->address.type != VTSS_IP_TYPE_IPV6) {
        return FALSE;
    }

    return IP2_if_ip_check(n, ignore);
}

static vtss_rc IP2_if_ipv4_set(ip2_ifstate_t           *ifs,
                               const vtss_ip_network_t *const net)
{
    if (net->address.type == VTSS_IP_TYPE_IPV6) {
        T_E("IPv6 address treated as IPv4");
        return VTSS_RC_ERROR;
    }

    if (!vtss_ip_network_equal(&ifs->cur_ipv4, net)) {
        vtss_rc rc = VTSS_RC_OK;

        if (ifs->cur_ipv4.address.type == VTSS_IP_TYPE_IPV4) {
            T_I("Delete IPv4 address "VTSS_IPV4_FORMAT"/%u on Vlan=%u",
                VTSS_IPV4_ARGS(ifs->cur_ipv4.address.addr.ipv4),
                ifs->cur_ipv4.prefix_size, ifs->vid);
            rc = vtss_ip2_os_ip_del(ifs->vid, &ifs->cur_ipv4);

            if (rc != VTSS_RC_OK) {
                T_E("Failed to delete IPv4 address "VTSS_IPV4_FORMAT"/%u on Vlan=%u",
                    VTSS_IPV4_ARGS(ifs->cur_ipv4.address.addr.ipv4),
                    ifs->cur_ipv4.prefix_size, ifs->vid);
            }
        }

        if (net->address.type == VTSS_IP_TYPE_IPV4) {
            T_I("Set new IPv4 address "VTSS_IPV4_FORMAT"/%u on Vlan=%u ",
                VTSS_IPV4_ARGS(net->address.addr.ipv4),
                net->prefix_size, ifs->vid);
            rc = vtss_ip2_os_ip_add(ifs->vid, net);
        }

        // Update cache on success
        if (rc == VTSS_RC_OK) {
            ifs->cur_ipv4 = *net;
        }

        return rc;
    } else {
        T_D("Ignore interface update doe to cache. Vlan=%u", ifs->vid);
    }

    return VTSS_RC_OK;
}

static vtss_rc IP2_ip_dhcpc_cb_apply(vtss_vid_t vlan)
{
    vtss_rc rc;
    ip2_ifstate_t *ifs;
    vtss_ip_network_t ip;
    vtss_dhcp_fields_t fields;

    IP2_CRIT_ASSERT_LOCKED();
    ifs = IP2_if2state(vlan);
    if (!ifs) {
        T_W("Could not translate vlan %u to interface state", vlan);
        return VTSS_RC_ERROR;
    }

    // get the new IP address (if we got one)
    rc = vtss_dhcp_client_fields_get(vlan, &fields);
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    if (!IP2_if_ipv4_check2(&fields.ip, vlan)) {
        T_W("The ack'ed dhcp ip address is not valid in this system");
        (void) vtss_dhcp_client_release(vlan);
        return VTSS_RC_ERROR;
    }

    // Apply IP settings
    ip.prefix_size = fields.ip.prefix_size;
    ip.address.type = VTSS_IP_TYPE_IPV4;
    ip.address.addr.ipv4 = fields.ip.address;

    rc = IP2_if_ipv4_set(ifs, &ip);
    if (rc == VTSS_RC_OK) {
        ifs->dhcp_v4_valid = TRUE;

    } else {
        T_W("Failed to set IPv4 interface on vlan: %u ip: " VTSS_IPV4_FORMAT,
            vlan, VTSS_IPV4_ARGS(ip.address.addr.ipv4));
        return rc;
    }

    // Apply default GW
    if (fields.has_default_gateway) {
        vtss_routing_entry_t rt;
        vtss_routing_params_t params;

        rt.type = VTSS_ROUTING_ENTRY_TYPE_IPV4_UC;
        rt.route.ipv4_uc.network.address = 0;
        rt.route.ipv4_uc.network.prefix_size = 0;
        rt.route.ipv4_uc.destination = fields.default_gateway;
        params.owner = VTSS_ROUTING_PARAM_OWNER_DHCP;

        T_D("Using default route from dhcp: " VTSS_IPV4_UC_FORMAT,
            VTSS_IPV4_UC_ARGS(rt.route.ipv4_uc));
        rc = IP2_route_add(&rt, &params);
        if (rc == VTSS_RC_OK) {
            ifs->dhcp_v4_gw = fields.default_gateway;

        } else {
            T_I("Failed to add default route. vlan: %u route: "
                VTSS_IPV4_UC_FORMAT,
                vlan, VTSS_IPV4_UC_ARGS(rt.route.ipv4_uc));
        }
    }

    return rc;
}

static void IP2_ip_dhcpc_cb_ack(vtss_vid_t vlan, vtss_dhcp_client_status_t *st)
{
    unsigned i;

    IP2_CRIT_ASSERT_LOCKED();

    for (i = 0; i < st->offers.valid_offers; ++i) {
        if (IP2_if_ipv4_check2(&st->offers.list[i].ip, vlan)) {
            T_I("Accepting offer %d "VTSS_IPV4N_FORMAT, i,
                VTSS_IPV4N_ARG(st->offers.list[i].ip));
            (void) vtss_dhcp_client_offer_accept(vlan, i);
        }
    }
}

static vtss_rc IP2_if_ipv4_signal(ip2_ifstate_t *ifs)
{
    // derive the interface address based on configuration and dhcp client state
    vtss_rc rc = VTSS_RC_OK;
    vtss_ip_network_t no_address;
    vtss_dhcp_client_status_t status;

    T_D("IP2_if_ipv4_signal vlan=%u", ifs->vid);
    no_address.address.type = VTSS_IP_TYPE_NONE;

    if (!ifs->config->ipv4.dhcpc) {
        // Simple apply the static configured IPv4 address
        return IP2_if_ipv4_set(ifs, &(ifs->config->ipv4.network));
    }

    // DHCP is active
    rc = vtss_dhcp_client_status(ifs->vid, &status);
    if (rc != VTSS_RC_OK) {
        rc = IP2_if_ipv4_set(ifs, &no_address);
        return rc;
    }

    switch (status.state) {
    case DHCP4C_STATE_SELECTING:
        rc = IP2_if_ipv4_set(ifs, &no_address);
        IP2_ip_dhcpc_cb_ack(ifs->vid, &status);
        break;

    case DHCP4C_STATE_REBINDING:
    case DHCP4C_STATE_BOUND:
    case DHCP4C_STATE_RENEWING:
        T_I("Apply dhcp client addresses");
        rc = IP2_ip_dhcpc_cb_apply(ifs->vid);
        break;

    case DHCP4C_STATE_FALLBACK:
        T_I("Set fallback address");
        rc = IP2_if_ipv4_set(ifs, &(ifs->config->ipv4.network));
        break;

    default:
        T_I("DHCP client have no IP address, clear all addresses");
        rc = IP2_if_ipv4_set(ifs, &no_address);
        break;
    }

#ifdef VTSS_SW_OPTION_DNS
    // DNS settings must be applied after routes has been configured.
    vtss_dns_signal();
#endif /* VTSS_SW_OPTION_DNS */

    return rc;
}

static void IP2_ip_dhcpc_cb(vtss_vid_t vlan)
{
    ip2_ifstate_t *ifs;

    IP2_CRIT_ASSERT_LOCKED();
    ifs = IP2_if2state(vlan);

    if (!ifs) {
        T_W("Could not translate vlan %u to interface state", vlan);
        return;
    }

    // The fallback timer must react on current state. It must be activated when
    // we go into SELECTING, and otherwise disabled
    IP2_dhcpc_fallback_timer_consider(ifs, FALSE);

    // Update interface settings
    (void) IP2_if_ipv4_signal(ifs);
}

static void vtss_ip2_ip_dhcpc_cb(vtss_vid_t vlan)
{
    // Lock even though we are static... This function is called through
    // callbacks from dhc dhcp client
    IP2_CRIT_ENTER();
    T_D("Got dhcp callback on vlan %u", vlan);
    IP2_ip_dhcpc_cb(vlan);
    IP2_CRIT_EXIT();
}

static cyg_tick_count_t IP2_dhcpc_fallback_timer_evaluate(cyg_tick_count_t t)
{
    IP2_dhcp_fallback_timer_t *i;
    cyg_tick_count_t cur = cyg_current_time();

    while (!VTSS_LINKED_LIST_EMPTY(IP2_dhcp_fallback_timer_list)) {
        i = VTSS_LINKED_LIST_FRONT(IP2_dhcp_fallback_timer_list);

        if (i->timeout <= cur) {
            ip2_ifstate_t *ifs;
            vtss_vid_t vid = i->vlan;
            T_D("Time is up %llu, %llu, vlan=%u", i->timeout, cur, vid);

            // time is up
            VTSS_LINKED_LIST_POP_FRONT(IP2_dhcp_fallback_timer_list);

            ifs = IP2_if2state(vid);
            if (ifs) {
                // Put the dhcp-client in fallback mode
                if (vtss_dhcp_client_fallback(ifs->vid) != VTSS_RC_OK) {
                    T_E("Failed to stop DHCP server on %u", ifs->vid);
                }

                // signal the interface
                (void) IP2_if_ipv4_signal(ifs);

            } else {
                T_E("Failed to map %u to ifs", vid);
            }

        } else {
            // we got a new timeout
            T_D("Next timer: %llu, %llu", t, i->timeout);
            return (t < i->timeout) ? t : i->timeout;
        }
    }

    // no timers, return input as minimum
    return t;
}

static void IP2_dhcpc_fallback_timer_start(cyg_tick_count_t timeout,
                                           IP2_dhcp_fallback_timer_t *timer)
{
    IP2_dhcp_fallback_timer_t *i;
    VTSS_LINKED_LIST_UNLINK(*timer);
    cyg_tick_count_t cur = cyg_current_time();

    T_I("DHCPC: Start fallback timer %llu, vlan=%u", timeout, timer->vlan);
    timer->timeout = cur + timeout;

    VTSS_LINKED_LIST_FOREACH(IP2_dhcp_fallback_timer_list, i) {
        if (i->timeout > timer->timeout) {
            break;
        }
    }

    VTSS_LINKED_LIST_INSERT_BEFORE(IP2_dhcp_fallback_timer_list, i, *timer);
    cyg_flag_setbits(&ip2_control_flags, CTLFLAG_NEW_TIMER);
}

static void IP2_dhcpc_fallback_timer_consider(ip2_ifstate_t *ifs, BOOL reinit)
{

    vtss_dhcp_client_status_t  status;

    if (ifs->config->ipv4.network.address.type != VTSS_IP_TYPE_IPV4) {
        goto UNLINK;
    }

    if (ifs->config->ipv4.fallback_timeout == 0) {
        goto UNLINK;
    }

    if (vtss_dhcp_client_status(ifs->vid, &status) != VTSS_RC_OK) {
        goto UNLINK;
    }

    if (status.state != DHCP4C_STATE_SELECTING) {
        goto UNLINK;
    }

    if (!ifs->config->ipv4.dhcpc) {
        goto UNLINK;
    }

    if (!VTSS_LINKED_LIST_IS_LINKED(ifs->dhcpc_start_timer) || reinit) {
        cyg_tick_count_t timeout;

        timeout = ifs->config->ipv4.fallback_timeout;
        timeout *= 1000;
        timeout /= ECOS_MSECS_PER_HWTICK;
        IP2_dhcpc_fallback_timer_start(timeout, &ifs->dhcpc_start_timer);
    }
    return;

UNLINK:
    if (VTSS_LINKED_LIST_IS_LINKED(ifs->dhcpc_start_timer)) {
        T_I("Unlink timer, vlan=%u", ifs->vid);
        VTSS_LINKED_LIST_UNLINK(ifs->dhcpc_start_timer);
    }
}

static void IP2_dhcpc_fallback_timer_update(ip2_ifstate_t *ifs)
{
    // Delete existing timer
    if (ifs->config->ipv4.fallback_timeout == 0) {
        VTSS_LINKED_LIST_UNLINK(ifs->dhcpc_start_timer);
        return;
    }

    // Set new timeout
    IP2_dhcpc_fallback_timer_consider(ifs, TRUE);
}

static vtss_rc IP2_dhcpc_v4_start(ip2_ifstate_t *ifs)
{
    vtss_rc rc = VTSS_RC_OK;

    IP2_CRIT_ASSERT_LOCKED();

    T_I("Starting dhcp client on vlan %u", ifs->vid);
    if (!ifs) {
        T_D("Failed to get infterface status obj");
        return IP2_ERROR_PARAMS;
    }

    rc = vtss_dhcp_client_start(ifs->vid);
    if (rc != VTSS_RC_OK) {
        T_I("Failed to start dhcp client");
        return rc;
    }

    rc = vtss_dhcp_client_callback_add(ifs->vid, vtss_ip2_ip_dhcpc_cb);
    if (rc != VTSS_RC_OK) {
        T_E("Failed to start dhcp CB");
        return rc;
    }

    // Do a explicit callback to react on current state
    IP2_ip_dhcpc_cb(ifs->vid);

    return VTSS_RC_OK;
}

/*
 * Start interface - OS & chip
 */
static vtss_rc IP2_if_start(uint if_no)
{
    vtss_rc rc = VTSS_OK;
    struct ip2_iface_entry *if_conf;

    IP2_CRIT_ASSERT_LOCKED();

    if_conf = &IP2_stack_conf.interfaces[if_no];
    if (if_conf->vlan_id != VTSS_VID_NULL) {
        T_I("Start if: %d vid=%u", if_no, if_conf->vlan_id);
        ip2_ifstate_t *if_state = &IP2_state.interfaces[if_no];
        IP2_if_init(if_state, if_no, if_conf, &IP2_state.master_mac);
        rc = vtss_ip2_os_if_add(if_state->vid);
        if (rc != VTSS_RC_OK) {
            return rc;
        }

        VTSS_ASSERT(&if_conf->conf == if_state->config);
        (void)IP2_ipv4_conf_set(if_state->vid, &(if_state->config->ipv4), TRUE);
        if (vtss_ip2_hasipv6()) {
            (void)IP2_ipv6_conf_set(if_state->vid, &(if_state->config->ipv6));
        }
    }
    return rc;
}

/*
 * Stop interface - OS & chip
 */
static vtss_rc IP2_if_teardown(ip2_ifstate_t *if_state)
{
    int i, if_no;
    vtss_vid_t vid;
    vtss_rc rc = VTSS_OK;

    IP2_CRIT_ASSERT_LOCKED();

    vid = if_state->vid;
    if_no = if_state->if_no;

    // unlink potential timers
    VTSS_LINKED_LIST_UNLINK(if_state->dhcpc_start_timer);

    T_I("Delete former IP interface: vid=%u, if_no=%d", vid, if_no);
    if (if_state->config->ipv4.dhcpc) {
        (void)IP2_ip_dhcpc_v4_stop_no_reapply(vid);
    }

    rc = vtss_ip2_os_if_del(if_state->vid);

    /* Now Nuke state */
    memset(if_state, 0, sizeof(*if_state));

    /* And nuke index backlink */
    IP2_state.vid2if[vid] = IFNO_INVALID;

    // Clear forwarding information
    for (i = 0; i < VTSS_ISID_CNT; ++i) {
        IP2_state.unit_fwd[i][if_no] = VIF_FWD_UNDEFINED;
    }

    IP2_ifflags_force_sync(if_no);

    return rc;
}

/* Stack communication */

static ip2_msg_t *alloc_message(size_t size, ip2_msg_id_t msg_id)
{
    ip2_msg_t *msg = VTSS_MALLOC(size);
    if (msg) {
        msg->msg_id = msg_id;
    }
    T_N("msg len %zd, type %d => %p", size, msg_id, msg);
    return msg;
}

static void respond_fwdstate(ulong isid, const ip2_msg_t *req)
{
    ip2_msg_t *msg = alloc_message(sizeof(*msg), IP2_MSG_ID_FWD_STATE_RSP);
    if (msg) {
        uint if_no;
        vtss_packet_port_info_t   info;
        (void) vtss_packet_port_info_init(&info);
        for (if_no = 0; if_no < IP2_MAX_INTERFACES; if_no++) {
            vtss_packet_port_filter_t filter[VTSS_PORT_ARRAY_SIZE];
            ip2_vif_fwd_t fwd = VIF_FWD_UNDEFINED;
            // get port information by vid
            info.vid = req->data.fwd_state.iface[if_no].vid;
            if (info.vid != VTSS_VID_NULL && vtss_packet_port_filter_get(NULL, &info, filter) == VTSS_OK) {
                port_iter_t pit;
                (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_UP);
                fwd = VIF_FWD_BLOCKING;
                while (port_iter_getnext(&pit)) {
                    vtss_packet_filter_t pf = filter[pit.iport].filter;
                    if (pf != VTSS_PACKET_FILTER_DISCARD) {
                        T_N("Port %d on vid%d is fwd: %d", pit.uport, info.vid, pf);
                        fwd = VIF_FWD_FORWARDING;
                        break;      /* At least one is enough data */
                    }
                }
            }
            msg->data.fwd_state.iface[if_no].vid = info.vid;
            msg->data.fwd_state.iface[if_no].fwd = fwd;
        }
        msg_tx(VTSS_MODULE_ID_IP2, isid, msg, offsetof(ip2_msg_t, data) + sizeof(msg->data.fwd_state));
    }
}

static void IP2_request_fwdstate(void)
{
    static ip2_msg_t msg;
    switch_iter_t sit;
    uint if_no;

    IP2_CRIT_ASSERT_LOCKED();
    T_N("Build request");

    msg.msg_id = IP2_MSG_ID_FWD_STATE_REQ;
    for (if_no = 0; if_no < IP2_MAX_INTERFACES; if_no++) {
        msg.data.fwd_state.iface[if_no].vid = IP2_state.interfaces[if_no].vid;
        msg.data.fwd_state.iface[if_no].fwd = FALSE; /* Unused */
    }
    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        T_N("Request sid %d", sit.usid);
        msg_tx_adv(NULL, NULL, MSG_TX_OPT_DONT_FREE,
                   VTSS_MODULE_ID_IP2, sit.isid,
                   (const void *)&msg, offsetof(ip2_msg_t, data) + sizeof(msg.data.fwd_state));
    }
}

static void IP2_process_fwdstate(ulong isid, const ip2_msg_t *rsp)
{
    uint if_no;
    int changes = 0;

    IP2_CRIT_ASSERT_LOCKED();
    for (if_no = 0; if_no < IP2_MAX_INTERFACES; if_no++) {
        if (IP2_state.interfaces[if_no].vid == VTSS_VID_NULL) {
            continue;
        }

        if (IP2_state.interfaces[if_no].vid != rsp->data.fwd_state.iface[if_no].vid) {
            T_I("if%d: Vlan mismatch %d != %d, skipping", if_no,
                IP2_state.interfaces[if_no].vid, rsp->data.fwd_state.iface[if_no].vid);
            continue;
        }

        T_N("isid %d if %d fwd %d", isid, if_no, rsp->data.fwd_state.iface[if_no].fwd);
        if (IP2_state.unit_fwd[isid - VTSS_ISID_START][if_no] != rsp->data.fwd_state.iface[if_no].fwd) {
            IP2_state.unit_fwd[isid - VTSS_ISID_START][if_no] = rsp->data.fwd_state.iface[if_no].fwd;
            T_I("isid %d if %d: Changed to state %d", isid, if_no, IP2_state.unit_fwd[isid - VTSS_ISID_START][if_no]);
            changes++;
        }
    }

    if (changes) {
        /* Have main thread update combined state */
        cyg_flag_setbits(&ip2_control_flags, CTLFLAG_IFCHANGE);
    }
}

/*
 * Message indication function
 */
static BOOL
vtss_ip2_msg_rx(void *contxt,
                const void *rx_msg,
                size_t len,
                vtss_module_id_t modid,
                ulong isid)
{
    const ip2_msg_t *msg = (void *)rx_msg;

    IP2_CRIT_ENTER();
    T_N("RX: Sid %u, rx %zd bytes, msg %d %s", isid, len, msg->msg_id,
        IP2_msg_id_to_str(msg->msg_id));

    switch (msg->msg_id) {
    case IP2_MSG_ID_FWD_STATE_REQ:
        T_N("IP2_MSG_ID_FWD_STATE_REQ");
        respond_fwdstate(isid, msg);
        break;

    case IP2_MSG_ID_FWD_STATE_RSP:
        T_N("IP2_MSG_ID_FWD_STATE_RSP");
        if (msg_switch_is_master()) {
            IP2_process_fwdstate(isid, msg);
        }
        break;

    case IP2_MSG_ID_IF_STATE:
        T_N("IP2_MSG_ID_IF_STATE");
        if (!msg_switch_is_master()) {
            memcpy(&IP2_if_status_cache, &msg->data.if_status,
                   sizeof(IP2_if_status_cache));
        }
        break;

    case IP2_MSG_ID_HELLO_REQ:
        if (IP2_is_master) {
            T_D("Set pending hello flag");
            IP2_hello_pending_res = TRUE;
        } else {
            (void)IP2_tx_hello_res();
        }
        //IP2_master_id = isid;
        break;

    case IP2_MSG_ID_HELLO_RES:
        if (IP2_is_master && VTSS_ISID_LEGAL(isid)) {
            T_D("Slave %u says hello", isid);
            IP2_sync_node(isid);
        } else {
            T_D("IP2_MSG_ID_HELLO_RES is only valid on master");
        }
        break;

    default:
        T_W("Unhandled msg %d", msg->msg_id);
    }
    IP2_CRIT_EXIT();
    return TRUE;
}


static void
stack_register(void)
{
    msg_rx_filter_t filter;
    memset(&filter, 0, sizeof(filter));
    filter.cb = vtss_ip2_msg_rx;
    filter.modid = VTSS_MODULE_ID_IP2;
    vtss_rc rc =  msg_rx_filter_register(&filter);
    VTSS_ASSERT(rc == VTSS_OK);
}

/****************************************************************************
 * IP route database operations
 */

/* Initialize rte and lookup data */
static ip2_route_db_entry_t *IP2_route_db_lookup(ip2_route_db_entry_t *rte,
                                                 const vtss_routing_entry_t  *const rt)
{
    IP2_CRIT_ASSERT_LOCKED();

    ip2_route_db_entry_t *dbp;
    rte->route.route = *rt;
    dbp = rte;
    if (vtss_avl_tree_get(&IP2_route_db, (void **) &dbp, VTSS_AVL_TREE_GET) == TRUE) {
        return dbp;             /* Updated iff found */
    }
    return NULL;                /* Not found */
}


/* To reach a host using a link-local address, it is necessary to know what
 * interface to use. This is because all link-local address are located on the
 * same IPv6 network, and the egress interface can thereby not be derived from
 * the routing table. eCos encodes this information directly into the IPv6
 * address as: fe80:if-index::. The following two functions converts this from
 * normal config encoding to the specific eCos encoding.  */
static
vtss_rc IP2_route_convert_os_to_conf(const vtss_routing_entry_t *const os,
                                     vtss_routing_entry_t       *const conf)
{
    int if_idx;
    int zero = 0;
    vtss_vid_t vid;

    if (os != conf) {
        *conf = *os;
    }

    if ((os->type != VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) ||
        !vtss_ipv6_addr_is_link_local(&os->route.ipv6_uc.destination)) {
        T_N("No conversion needed");
        return VTSS_RC_OK;
    }

    IP2_IPV6_RTNH_LLA_IFID_GET(if_idx, os);
    vid = vtss_ip2_ecos_driver_if_index_to_vid(if_idx);

    IP2_IPV6_RTNH_LLA_IFID_SET(zero, conf);
    conf->vlan = vid;

    return VTSS_RC_OK;
}

static
vtss_rc IP2_route_convert_conf_to_os(const vtss_routing_entry_t *const conf,
                                     vtss_routing_entry_t       *const os)
{
    vtss_rc             rc;
    vtss_if_status_t    ifst;

    if (os != conf) {
        *os = *conf;
    }

    if (conf->type != VTSS_ROUTING_ENTRY_TYPE_IPV6_UC ||
        !vtss_ipv6_addr_is_link_local(&conf->route.ipv6_uc.destination)) {
        T_N("No conversion needed");
        return VTSS_RC_OK;
    }

    IP2_CRIT_ASSERT_LOCKED();
    if (!IP2_VALID_VLAN_ID(conf->vlan)) {
        T_D("%u is not a valid vlan", conf->vlan);
        return IP2_ERROR_PARAMS;
    }

    rc = IP2_if_status_get_first(VTSS_IF_STATUS_TYPE_LINK, conf->vlan, &ifst);
    if (rc != VTSS_RC_OK) {
        T_D("IP2_if_status_get_first(LINK, %u, ptr) failed", conf->vlan);
        return rc;
    }

    IP2_IPV6_RTNH_LLA_IFID_SET(ifst.u.link.os_if_index, os);

    return rc;
}

static vtss_rc IP2_os_route_add(const vtss_routing_entry_t *const conf_rt)
{
    vtss_rc rc;
    vtss_routing_entry_t os_rt;

    IP2_CRIT_ASSERT_LOCKED();

    rc = IP2_route_convert_conf_to_os(conf_rt, &os_rt);
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    return vtss_ip2_os_route_add(&os_rt);
}

static vtss_rc IP2_os_route_del(const vtss_routing_entry_t *const conf_rt)
{
    vtss_rc rc;
    vtss_routing_entry_t os_rt;

    IP2_CRIT_ASSERT_LOCKED();

    rc = IP2_route_convert_conf_to_os(conf_rt, &os_rt);
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    return vtss_ip2_os_route_del(&os_rt);
}

static void IP2_route_db_apply(void)
{
    ip2_route_db_entry_t *dbp = NULL;

    IP2_CRIT_ASSERT_LOCKED();
    T_I("Refreshing routes");
    while (vtss_avl_tree_get(&IP2_route_db, (void **) &dbp, dbp == NULL ? VTSS_AVL_TREE_GET_FIRST : VTSS_AVL_TREE_GET_NEXT) && dbp) {
        if (IP2_os_route_add(&dbp->route.route) == VTSS_OK) {
            T_I("Activated route");
        } else {
            T_D("Route not accepted");
        }
    }
}

static void IP2_route_db_reset(void)
{
    ip2_route_db_entry_t *dbp = NULL;

    IP2_CRIT_ASSERT_LOCKED();
    T_I("Empty routes");
    while (vtss_avl_tree_get(&IP2_route_db, (void **) &dbp, VTSS_AVL_TREE_GET_FIRST) && dbp) {
        if (vtss_avl_tree_delete(&IP2_route_db, (void **) &dbp)) {
            (void) IP2_os_route_del(&dbp->route.route);
            VTSS_FREE(dbp);
        } else {
            T_W("Delete traversal failed");
            break;
        }
    }
}

static void IP2_route_db_static_save(void)
{
    ip2_route_entry_t routes[IP2_MAX_ROUTES];
    ip2_route_db_entry_t *dbp = NULL;
    int i = 0;

    IP2_CRIT_ASSERT_LOCKED();
    memset(routes, 0, sizeof(routes));

    while (vtss_avl_tree_get(&IP2_route_db, (void **) &dbp, dbp == NULL ? VTSS_AVL_TREE_GET_FIRST : VTSS_AVL_TREE_GET_NEXT) && dbp) {
        if (dbp->users & VTSS_BIT(VTSS_ROUTING_PARAM_OWNER_STATIC_USER)) {
            if (i < IP2_MAX_ROUTES) {
                routes[i++] = dbp->route;
            } else {
                T_E("Excessive static routes");
            }
        }
    }

    IP2_state.static_routes = i;

    /* Do we need to save to flash? */
    if (memcmp(IP2_stack_conf.routes, routes, sizeof(IP2_stack_conf.routes)) != 0) {
        memcpy(IP2_stack_conf.routes, routes, sizeof(IP2_stack_conf.routes));
        IP2_flash_write();
    }
}

static vtss_rc IP2_route_db_add(const ip2_route_db_entry_t *rte, int owner)
{
    ip2_route_db_entry_t *rtcopy = VTSS_MALLOC(sizeof(*rtcopy));
    vtss_rc rc;

    IP2_CRIT_ASSERT_LOCKED();
    if (rtcopy) {
        *rtcopy = *rte;
        if (vtss_avl_tree_add(&IP2_route_db, rtcopy)) {
            rtcopy->users = VTSS_BIT(owner); /* Mark owner */
            /* Apply in OS layer */
            IP2_route_db_apply();
            rc = VTSS_OK;
        } else {
            T_E("route db add failed");
            VTSS_FREE(rtcopy);
            rc = VTSS_RC_ERROR; /* ??? */
        }
    } else {
        rc = IP2_ERROR_NOSPACE;
    }
    return rc;
}

static vtss_rc IP2_route_db_del_all_owners(ip2_route_db_entry_t *rte)
{
    IP2_CRIT_ASSERT_LOCKED();

    ip2_route_db_entry_t *dbp = rte;
    if (!vtss_avl_tree_delete(&IP2_route_db, (void **) &dbp)) {
        T_E("Unable to delete route in DB");
        return VTSS_RC_ERROR;
    }

    if (!dbp) {
        T_E("Null-ptr after delete route in DB");
        return VTSS_RC_ERROR;
    }

    /* Delete at OS layer */
    if (IP2_os_route_del(&dbp->route.route) != VTSS_OK) {
        T_I("Deleting inactive route");
    }

    /* Reapply other routes */
    IP2_route_db_apply();
    VTSS_FREE(dbp);
    return VTSS_OK;

}

static vtss_rc IP2_route_db_del(ip2_route_db_entry_t *rte, int owner)
{
    IP2_CRIT_ASSERT_LOCKED();

    /* Drop user */
    rte->users &= ~VTSS_BIT(owner);
    if (rte->users == 0) {
        if (IP2_route_db_del_all_owners(rte) != VTSS_RC_OK) {
            T_E("Failed to delete route");
        } else {
            T_D("Purged route");
        }
    }
    T_D("Removed user %d from users, have %x", owner, rte->users);
    return VTSS_OK;
}

static void IP2_delete_all_routes(void)
{
    ip2_route_db_entry_t *dbp = NULL;

    while (vtss_avl_tree_get(&IP2_route_db, (void **) &dbp,
                             VTSS_AVL_TREE_GET_FIRST)) {
        if (!dbp) {
            break;
        }

        (void) IP2_route_db_del_all_owners(dbp);
        dbp = NULL;
    }
}

static void IP2_delete_all_interfaces(void)
{
    ip2_ifstate_t *ifs;
    uint i, vid;
    vtss_rc rc;

    IP2_CRIT_ASSERT_LOCKED();
    for (i = 0; i < VTSS_VIDS; i++) {
        if (!IP2_if_exists(i)) {
            continue;
        }

        ifs = IP2_if2state(i);
        if (!ifs) {
            continue;
        }

        vid = ifs->vid;
        rc = IP2_if_teardown(ifs);
        if (rc != VTSS_RC_OK) {
            T_E("IP2_if_teardown failed: if_no=%d, vid=%d msg=%s",
                i, vid, error_txt(rc));
        }
    }
}

/*
 * IP2_conf_default()
 * Initialize configuration to default.
 */
static void IP2_conf_default(ip2_stack_conf_t *cfg)
{
    IP2_CRIT_ASSERT_LOCKED();

    // Gracefully remove existing configuration
    IP2_delete_all_interfaces();
    IP2_delete_all_routes();

    memset(cfg, 0, sizeof(*cfg));
    cfg->global.enable_routing = FALSE;

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Until ICFG/ICLI fully supports stacking, we need to config a default
    // VLAN1 interface.
    cfg->interfaces[0].vlan_id = 1;
    cfg->interfaces[0].conf.link_layer.mtu = 1500;
    cfg->interfaces[0].conf.ipv4.dhcpc = FALSE;
    cfg->interfaces[0].conf.ipv4.network.address.type = VTSS_IP_TYPE_IPV4;
    cfg->interfaces[0].conf.ipv4.network.address.addr.ipv4 = 0xc0000201;
    cfg->interfaces[0].conf.ipv4.network.prefix_size = 24;
#endif
}

/*
 * Re-apply current configuration - router legs
 */
static void IP2_flash_apply_interfaces(BOOL defaults)
{
    vtss_rc rc;
    uint i;

    IP2_CRIT_ASSERT_LOCKED();
    if (defaults) {
        IP2_delete_all_interfaces();
    }

    for (i = 0; i < IP2_MAX_INTERFACES; i++) {
        rc = IP2_if_start(i);
        if (rc != VTSS_OK) {
            T_E("IP interface %d: %s", i, error_txt(rc));
        }
    }
}

/***************************************************************************************************
 * Re-apply current configuration - routes
 **************************************************************************************************/
static void IP2_flash_apply_routes(BOOL defaults)
{
    int i;

    IP2_CRIT_ASSERT_LOCKED();
    IP2_state.static_routes = 0;
    if (defaults) {
        ip2_route_db_entry_t *dbp = NULL;
        while (vtss_avl_tree_get(&IP2_route_db, (void **) &dbp, dbp == NULL ? VTSS_AVL_TREE_GET_FIRST : VTSS_AVL_TREE_GET_NEXT) && dbp) {
            if (dbp->users & VTSS_BIT(VTSS_ROUTING_PARAM_OWNER_STATIC_USER)) {
                /* Remove user from route, possibly purge route */
                (void) IP2_route_db_del(dbp, VTSS_ROUTING_PARAM_OWNER_STATIC_USER);
                dbp = NULL;     /* Unable to do get-next now, restart */
            }
        }

    }

    for (i = 0; i < IP2_MAX_ROUTES; i++) {
        ip2_route_entry_t *rtn = &IP2_stack_conf.routes[i];
        if (rtn->route.type != VTSS_ROUTING_ENTRY_TYPE_INVALID) {
            ip2_route_db_entry_t entry, *dbp;
            IP2_state.static_routes++;
            if ((dbp = IP2_route_db_lookup(&entry, &rtn->route)) != NULL) {
                /* Mark extra owner of route */
                dbp->users |= VTSS_BIT(VTSS_ROUTING_PARAM_OWNER_STATIC_USER);
            } else {
                vtss_rc rc;
                /* Add to route DB */
                if ((rc = IP2_route_db_add(&entry, VTSS_ROUTING_PARAM_OWNER_STATIC_USER)) != VTSS_OK) {
                    T_I("Unable to add static route: %s", error_txt(rc));
                }
            }
        }
    }

    /* Push routes to the IP stack */
    IP2_route_db_apply();
    return;
}

/***************************************************************************************************
 * ip2_flash_apply()
 * Re-apply current configuration
 **************************************************************************************************/
static void IP2_flash_apply(BOOL defaults)
{
    vtss_rc rc;

    IP2_CRIT_ASSERT_LOCKED();

    rc = vtss_ip2_os_global_param_set(&IP2_stack_conf.global);
    if (rc != VTSS_OK) {
        T_W("Global config apply error - %s", error_txt(rc));
    }
    IP2_flash_apply_interfaces(defaults);
    IP2_flash_apply_routes(defaults);
#ifdef VTSS_SW_OPTION_DNS
    vtss_dns_signal();
#endif /* VTSS_SW_OPTION_DNS */
}

typedef struct {
    vtss_ipv4_t ipv4_router; // OK
    BOOL        dhcp;        // OK
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t ipv6_router;                   // OK
    BOOL        ipv6_autoconfig;               // No-longer-available
    ulong       ipv6_autoconfig_fallback_time; // No-longer-available
#endif
    vtss_ipv4_t ipv4_addr;   // OK
    vtss_ipv4_t ipv4_mask;   // OK
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t ipv6_addr;   // ok
    ulong       ipv6_prefix; // ok
#endif
    vtss_vid_t  vid;         // OK
    uchar       ipv4_sntp_string[46]; // MISSING
#ifdef VTSS_SW_OPTION_DNS
    vtss_ipv4_t ipv4_dns;    // Done in DNS module
#endif
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t ipv6_sntp;   // MISSING
#endif
} ip_conf_2_80_3_t;

typedef struct {
    ulong            version;    /* Block Version */
    ip_conf_2_80_3_t conf;       /* Configuration */
} ip_conf_blk_2_80_3_t;

static vtss_rc IP2_flash_build_stack_conf(ip2_flash_conf_t *flash_conf,
                                          ulong             size,
                                          ip2_stack_conf_t *target)
{
    vtss_rc rc;
    unsigned route_index = 0;

    if (flash_conf == NULL) {
        T_W("Could not open flash");
        return VTSS_RC_ERROR;
    }

    if (size == sizeof(ip2_flash_conf_t) &&
        flash_conf->version == IP2_CONF_VERSION) {
        T_I("Flash config is up to date - use as-is");
        *target = flash_conf->stack_conf;
        return VTSS_RC_OK;

    }

    if (size == sizeof(ip_conf_blk_2_80_3_t) &&
        flash_conf->version != IP2_CONF_VERSION) {
        T_I("Upgrade from 2_80-3");
        memset(&IP2_stack_conf, 0, sizeof(IP2_stack_conf));
        ip_conf_2_80_3_t *old = (ip_conf_2_80_3_t *)(&flash_conf->stack_conf);

        // Install ipv4_router as default router in the routing table
        if (old->ipv4_router) {
            ip2_route_entry_t *rt = &target->routes[route_index++];
            rt->route.type = VTSS_ROUTING_ENTRY_TYPE_IPV4_UC;
            rt->route.route.ipv4_uc.destination = old->ipv4_router;
            target->global.enable_routing = FALSE;
        }

        // Create a new IP interface on the old management VLAN
        if (old->vid) {
            target->interfaces[0].vlan_id = old->vid;
            vtss_interface_ip_conf_t *if_conf = &target->interfaces[0].conf;
            vtss_if_default_param(&if_conf->link_layer);
            if_conf->ipv4.dhcpc = old->dhcp;
            if (old->ipv4_addr) {
                rc = vtss_build_ipv4_network(&if_conf->ipv4.network,
                                             old->ipv4_addr,
                                             old->ipv4_mask);
                if (rc != VTSS_RC_OK) {
                    T_W("Invalid netmask");
                    return rc;
                }

            } else {
                T_I("No IPv4 address");
                if_conf->ipv4.network.address.type = VTSS_IP_TYPE_NONE;
            }

#ifdef VTSS_SW_OPTION_IPV6
            // IPv6 Route
            if (!vtss_ipv6_addr_is_zero(&old->ipv6_router)) {
                ip2_route_entry_t *rt = &target->routes[route_index++];
                rt->route.type = VTSS_ROUTING_ENTRY_TYPE_IPV6_UC;
                rt->route.route.ipv6_uc.destination = old->ipv6_router;
                target->global.enable_routing = FALSE;
            }

            // IPv6 address
            if (!vtss_ipv6_addr_is_zero(&old->ipv6_addr) &&
                vtss_ipv6_addr_is_mgmt_support(&old->ipv6_addr)) {
                if_conf->ipv6.network.address.type = VTSS_IP_TYPE_IPV6;
                if_conf->ipv6.network.address.addr.ipv6 = old->ipv6_addr;
                if_conf->ipv6.network.prefix_size = old->ipv6_prefix;
            } else {
                T_I("No IPv6 address");
                if_conf->ipv6.network.address.type = VTSS_IP_TYPE_NONE;
            }
#endif
        }

        return VTSS_RC_OK;
    }

    T_I("No upgrade path found!");
    return VTSS_RC_ERROR;
}

/*
 * IP2_flash_read()
 * Read the ip configuration from flash.
 * If read fails or create_default == TRUE then create default configuration.
 */
static void IP2_flash_read(BOOL create_defaults)
{
    vtss_rc rc;
    ulong size_read = 0;
    ip2_flash_conf_t *flash_write_;
    BOOL do_create = create_defaults;

    IP2_CRIT_ASSERT_LOCKED();
    T_I("IP2_flash_read %s", create_defaults ? "create_defaults" : "");

    if (misc_conf_read_use()) {
        // Open or create configuration block
        ip2_flash_conf_t *flash_read_ = conf_sec_open(CONF_SEC_GLOBAL,
                                                      CONF_BLK_IP_ROUTING,
                                                      &size_read);

        if (flash_read_ && size_read == sizeof(ip2_flash_conf_t)) {
            flash_write_ = flash_read_;
        } else {
            flash_write_ = conf_sec_create(CONF_SEC_GLOBAL,
                                           CONF_BLK_IP_ROUTING,
                                           sizeof(ip2_flash_conf_t));
        }

        if (!flash_read_) {
            // second priority flash block
            flash_read_ = conf_sec_open(CONF_SEC_GLOBAL,
                                        CONF_BLK_IP_CONF,
                                        &size_read);
        }

        rc = IP2_flash_build_stack_conf(flash_read_,
                                        size_read,
                                        &IP2_stack_conf);
        if (rc == VTSS_RC_OK) {
            T_I("Got stack conf");
        } else {
            T_W("Failed to build stack conf, fallback to defaults");
            do_create = TRUE;
        }

    } else {
        flash_write_ = NULL;
        do_create  = TRUE;
    }

    if (do_create) { // Create new default configuration
        T_I("Set default config");
        IP2_conf_default(&IP2_stack_conf);

        // Write back into flash
        if (flash_write_) {
            flash_write_->stack_conf = IP2_stack_conf;
        }
    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (flash_write_) {
        flash_write_->version = IP2_CONF_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_IP_ROUTING);
    } else {
        T_W("Failed to open flash configuration");
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    IP2_flash_apply(create_defaults);
}

/*
 * Write the ip configuration to flash.
 */
static void IP2_flash_write(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    ip2_flash_conf_t *flash_conf;
    ulong size = 0;

    IP2_CRIT_ASSERT_LOCKED();
    T_D("IP2_flash_write");

    if ((flash_conf = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_IP_ROUTING, &size)) != NULL &&
        (size == sizeof(*flash_conf))) {
        flash_conf->version = IP2_CONF_VERSION;
        flash_conf->stack_conf = IP2_stack_conf;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_IP_ROUTING);
    } else {
        T_D("Config not written, size =%d", size);
    }

    T_D("exit");
#else
    T_N("Silent-upgrade build: Not saving to conf");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

static BOOL IP2_if_exists(vtss_if_id_vlan_t if_id)
{
    IP2_CRIT_ASSERT_LOCKED();
    return IP2_if2state(if_id) != NULL;
}

static void IP2_if_cache_update_slave(void)
{
    int cnt = 0;
    ip2_msg_t *msg;
    vtss_isid_t isid;

    cnt = IP2_slave_active_cnt();

    if (cnt == 0) {
        return;
    }

    msg = IP2_msg_alloc(IP2_MSG_ID_IF_STATE, cnt);
    if (!msg) {
        T_E("Alloc error");
        return;
    }

    msg->data.if_status = IP2_if_status_cache;
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; ++isid) {
        if (IP2_slave_active[isid] && !msg_switch_is_local(isid)) {
            IP2_msg_tx(msg, isid, sizeof(if_status_t));
        }
    }
}

static void IP2_if_cache_update(void)
{
    vtss_rc rc;

    IP2_CRIT_ASSERT_LOCKED();
    rc = vtss_ip2_os_if_status_all(VTSS_IF_STATUS_TYPE_ANY,
                                   IP2_MAX_STATUS_OBJS,
                                   &IP2_if_status_cache.size,
                                   IP2_if_status_cache.data);

    if (rc != VTSS_RC_OK) {
        T_E("vtss_ip2_os_if_status_all failed");
        return;
    } else {
        T_D("vtss_ip2_os_if_status_all: %u valid entries",
            IP2_if_status_cache.size);
    }

    IP2_if_cache_update_slave();
}

static void IP2_if_signal(vtss_if_id_vlan_t if_id)
{
    IP2_CRIT_ASSERT_LOCKED();
    T_I("Signal vlan id: %u", if_id);

    /* update the cached if status */
    // TODO, only update  the notified vlan
    IP2_if_cache_update();

    VTSS_BF_SET(IP2_vlan_notify_bf, if_id, 1);

    /* Synch routes when interfaces change */
    cyg_flag_setbits(&ip2_control_flags, CTLFLAG_RTREFRESH);
    cyg_flag_setbits(&ip2_control_flags, CTLFLAG_IFNOTIFY);
}

void vtss_ip2_if_signal(vtss_if_id_vlan_t if_id)
{
    IP2_CRIT_ENTER();
    IP2_if_signal(if_id);
    IP2_CRIT_EXIT();

#ifdef VTSS_SW_OPTION_DNS
    vtss_dns_signal();
#endif /* VTSS_SW_OPTION_DNS */
}


static void IP2_if_all_update_states(void)
{
    size_t i, if_cnt;
    vtss_if_status_t *status = VTSS_CALLOC(IP2_MAX_STATUS_OBJS, sizeof(vtss_if_status_t));

    if (status &&
        vtss_ip2_ifs_status_get(VTSS_IF_STATUS_TYPE_ANY, IP2_MAX_STATUS_OBJS, &if_cnt, status) == VTSS_RC_OK) {

        /* Reset state */
        IP2_CRIT_ENTER();
        for (i = 0; i < IP2_MAX_INTERFACES; i++) {
            ip2_ifstate_t *if_state = &IP2_state.interfaces[i];
            if_state->cur_ipv4.address.type = VTSS_IP_TYPE_NONE;
            if_state->cur_ipv6.address.type = VTSS_IP_TYPE_NONE;
        }
        IP2_CRIT_EXIT();

        /* Update state */
        for (i = 0; i < if_cnt; i++) {
            vtss_if_status_t *ifstatus = &status[i];
            if (ifstatus->if_id.type == VTSS_ID_IF_TYPE_VLAN) {
                ip2_ifstate_t *ifs;
                IP2_CRIT_ENTER();
                ifs = IP2_if2state(ifstatus->if_id.u.vlan);
                if (ifs) {
                    if (ifstatus->type == VTSS_IF_STATUS_TYPE_IPV4) {
                        ifs->cur_ipv4.address.type = VTSS_IP_TYPE_IPV4;
                        ifs->cur_ipv4.address.addr.ipv4 = ifstatus->u.ipv4.net.address;
                        ifs->cur_ipv4.prefix_size = ifstatus->u.ipv4.net.prefix_size;
                    } else if (vtss_ip2_hasipv6() && ifstatus->type == VTSS_IF_STATUS_TYPE_IPV6) {
                        if (!vtss_ipv6_addr_is_link_local(&ifstatus->u.ipv6.net.address)) {
                            ifs->cur_ipv6.address.type = VTSS_IP_TYPE_IPV6;
                            ifs->cur_ipv6.address.addr.ipv6 = ifstatus->u.ipv6.net.address;
                            ifs->cur_ipv6.prefix_size = ifstatus->u.ipv6.net.prefix_size;
                        }
                    }
                }
                IP2_CRIT_EXIT();
            }
        }
    }

    if (status) {
        VTSS_FREE(status);
    }
}

static void vtss_ip2_if_callback(void)
{
    u32 i, vlan;
    u32 bf[VTSS_BF_SIZE(VTSS_VIDS)];

    IP2_if_all_update_states();

    IP2_CRIT_ENTER();
    memcpy(bf, IP2_vlan_notify_bf, sizeof(bf));
    memset(IP2_vlan_notify_bf, 0, sizeof(bf));
    IP2_CRIT_EXIT();

    for (vlan = 0; vlan < VTSS_VIDS; ++vlan) {
        if (!VTSS_BF_GET(bf, vlan)) {
            continue;
        }

        T_I("Callback vlan id: %u", vlan);
        for (i = 0; i < MAX_SUBSCRIBER; ++i) {
            vtss_ip2_if_callback_t cb;

            // WARNING, Raise condition.... what if we copy a pointer to a
            // callback, release the semaphore, the callee deletes the callback,
            // we invoke the callback after it has been deleted.
            IP2_CRIT_ENTER();
            cb = IP2_subscribers[i];
            IP2_CRIT_EXIT();

            if (cb == NULL) {
                continue;
            }

            (*cb)(vlan);
        }
    }
}

static vtss_rc IP2_ip_dhcpc_v4_stop_no_reapply(vtss_if_id_vlan_t if_id)
{
    vtss_rc rc = VTSS_RC_OK;

    IP2_CRIT_ASSERT_LOCKED();
    T_D("IP2_ip_dhcpc_v4_stop_no_reapply");

    rc = IP2_dhcp_clear(if_id);
    if (rc != VTSS_RC_OK) {
        T_W("Failed to clear dhcp settings");
    }

    rc = vtss_dhcp_client_kill(if_id);

#ifdef VTSS_SW_OPTION_DNS
    vtss_dns_signal();
#endif /* VTSS_SW_OPTION_DNS */

    return rc;
}

static vtss_rc IP2_if_del(vtss_if_id_vlan_t if_id)
{
    vtss_rc rc;
    int vid, if_no;
    ip2_ifstate_t *ifs;
    struct ip2_iface_entry *if_conf;

    IP2_CRIT_ASSERT_LOCKED();

    T_D("%s - vid %d", __FUNCTION__, if_id);

    ifs = IP2_if2state(if_id);
    if (!ifs) {
        T_W("Could not translate vlan %u to interface state", if_id);
        return IP2_ERROR_PARAMS;
    }

    vid = ifs->vid;
    if_no = ifs->if_no;
    if_conf = &IP2_stack_conf.interfaces[if_no];

    (void) vtss_ip2_os_ip_del(if_id, &ifs->cur_ipv4);
    if (vtss_ip2_hasipv6()) {
        if (vtss_ip2_os_ip_del(if_id, &ifs->cur_ipv6) == VTSS_OK) {
            ifs->cur_ipv6.address.type = VTSS_IP_TYPE_NONE;
        }
    }

    rc = IP2_if_teardown(ifs);

    /* Nuke interface */
    memset(if_conf, 0, sizeof(*if_conf));

    /* Save Config */
    IP2_flash_write();

    /* Notify here, as we can not convert if_index to vlan after this */
    IP2_if_signal(vid);

    return rc;
}

static vtss_rc IP2_ipv4_conf_set(vtss_if_id_vlan_t     if_id,
                                 const vtss_ip_conf_t *conf,
                                 BOOL                  force)
{
    ip2_ifstate_t *ifs;
    vtss_rc rc = VTSS_RC_OK;
    vtss_ip_conf_t *cur;

    IP2_CRIT_ASSERT_LOCKED();

    T_D("%s", __FUNCTION__);
    ifs = IP2_if2state(if_id);
    if (!ifs) {
        return IP2_ERROR_PARAMS;
    }

    cur = &(ifs->config->ipv4);

    // enable/disable dhcp client
    if (cur->dhcpc != conf->dhcpc || force) {
        T_I("Apply dhcp conf, vlan=%u", if_id);

        // Accept config, no validation needed
        cur->dhcpc = conf->dhcpc;
        IP2_flash_write();

        if (conf->dhcpc) {
            T_I("Start DHCP client on vlan if: %d", if_id);
            rc = IP2_dhcpc_v4_start(ifs);

        } else {
            T_I("Stop DHCP client on vlan if: %d", if_id);
            rc = IP2_ip_dhcpc_v4_stop_no_reapply(ifs->vid);
        }

        // This may affect the derived interface address
        (void) IP2_if_ipv4_signal(ifs);
    }

    // Static IPv4 conf (may be used as callback)
    if (!vtss_ip_network_equal(&cur->network, &conf->network) || force) {
        T_I("Apply address conf, vlan=%u", if_id);

        if (!(vtss_ip_ifaddr_valid(&conf->network) ||
              conf->network.address.type == VTSS_IP_TYPE_NONE)) {
            T_I("Invalid ip address, vlan=%u", if_id);
            rc = IP2_ERROR_PARAMS;

        } else if (!IP2_if_ipv4_check(&conf->network, if_id)) {
            T_I("IP address checks failed, vlan=%u", if_id);
            rc = IP2_ERROR_ADDRESS_CONFLICT;

        } else {
            // Accept new conf
            cur->network = conf->network;
            IP2_flash_write();
            (void) IP2_if_ipv4_signal(ifs);
        }
    }

    // Fallback timeout
    if (cur->fallback_timeout != conf->fallback_timeout || force) {
        T_I("Apply fallback timer conf, vlan=%u", if_id);
        cur->fallback_timeout = conf->fallback_timeout;
        IP2_dhcpc_fallback_timer_update(ifs);
        IP2_flash_write();
    }

    IP2_ifflags_force_sync(ifs->if_no);
    return rc;
}

static vtss_rc IP2_ipv6_clear_conf_set(ip2_ifstate_t           *ifs)
{
    IP2_CRIT_ASSERT_LOCKED();
    ifs->config->ipv6.dhcpc = FALSE;
    ifs->config->ipv6.network.address.type = VTSS_IP_TYPE_NONE;

    IP2_flash_write();
    if (vtss_ip2_os_ip_del(ifs->vid, &ifs->cur_ipv6) == VTSS_OK) {
        ifs->cur_ipv6.address.type = VTSS_IP_TYPE_NONE;
        return VTSS_OK;
    } else {
        return IP2_ERROR_FAILED;
    }
}

static vtss_rc IP2_ipv6_static_conf_set(ip2_ifstate_t           *ifs,
                                        const vtss_ip_network_t *network)
{
    unsigned i;

    IP2_CRIT_ASSERT_LOCKED();

    if (!vtss_ip_ifaddr_valid(network)) {
        T_W("Invalid interface address for vlan %d", ifs->vid);
        return IP2_ERROR_PARAMS;
    }

    /* Check for duplicates */
    if (network->address.type == VTSS_IP_TYPE_IPV6) {

        for (i = 0; i < IP2_MAX_INTERFACES; i++) {
            ip2_ifstate_t *ifo = &IP2_state.interfaces[i];

            if (ifo->vid == VTSS_VID_NULL || /* Invalid entry */
                ifs == ifo) {                /* This is us... */
                continue;
            }

            if (vtss_ip_net_equal(network, &ifo->cur_ipv6)) {
                T_W("Trying to add identical IP network on VLAN %d, also on "
                    "VLAN %d", ifs->vid, ifo->vid);
                return IP2_ERROR_EXISTS;
            }
        }
    }

    /* Rid old address, if any */
    if (ifs->cur_ipv6.address.type == VTSS_IP_TYPE_IPV6) {
        (void) vtss_ip2_os_ip_del(ifs->vid, &ifs->cur_ipv6);
    }

    ifs->cur_ipv6 = ifs->config->ipv6.network = *network;
    IP2_flash_write();
    return vtss_ip2_os_ip_add(ifs->vid, network);
}

static vtss_rc IP2_ipv6_conf_set(vtss_if_id_vlan_t            if_id,
                                 const vtss_ip_conf_t        *conf)
{
    ip2_ifstate_t *ifs;
    vtss_rc rc = VTSS_UNSPECIFIED_ERROR;

    IP2_CRIT_ASSERT_LOCKED();

    T_D("%s", __FUNCTION__);
    ifs = IP2_if2state(if_id);
    if (!ifs) {
        return IP2_ERROR_PARAMS;
    }

    if (conf->dhcpc) {
        // enable dhcp client
        T_I("Configuring DHCP on vlan if: %d", if_id);
        rc = VTSS_UNSPECIFIED_ERROR; /* No IPv6 DHCP support */

    } else if (conf->network.address.type == VTSS_IP_TYPE_IPV6) {
        // set static address
        T_I("Configuring static IPv6 on vlan if: %u " VTSS_IPV6_FORMAT "/%d",
            if_id, VTSS_IPV6_ARGS(conf->network.address.addr.ipv6),
            conf->network.prefix_size);

        if (!IP2_if_ipv6_check(&conf->network, if_id)) {
            return IP2_ERROR_ADDRESS_CONFLICT;
        }

        rc = IP2_ipv6_static_conf_set(ifs, &conf->network);

    } else {
        // clear all ipv6 addresses
        T_I("Clearing IPv6 addresses on vlan if %d ", if_id);
        rc = IP2_ipv6_clear_conf_set(ifs);
    }

    IP2_ifflags_force_sync(ifs->if_no);
    return rc;
}

static vtss_rc IP2_ip_clear_type(vtss_if_id_vlan_t  if_id,
                                 vtss_ip_type_t     type)
{
    ip2_ifstate_t *ifs;

    IP2_CRIT_ASSERT_LOCKED();

    ifs = IP2_if2state(if_id);
    if (!ifs) {
        T_W("No such interface: %d", if_id);
        return IP2_ERROR_PARAMS;
    }

    switch (type) {
    case VTSS_IP_TYPE_IPV4:
        ifs->config->ipv4.network.address.type = VTSS_IP_TYPE_NONE;
        return vtss_ip2_os_ip_del(ifs->vid, &ifs->cur_ipv4);
    case VTSS_IP_TYPE_IPV6:
        ifs->config->ipv6.network.address.type = VTSS_IP_TYPE_NONE;
        if (vtss_ip2_os_ip_del(ifs->vid, &ifs->cur_ipv6) == VTSS_OK) {
            ifs->cur_ipv6.address.type = VTSS_IP_TYPE_NONE;
            return VTSS_OK;
        } else {
            return IP2_ERROR_FAILED;
        }
    default:
        T_W("Unknown type: %d", type);
        return IP2_ERROR_PARAMS;
    }
}

static BOOL IP2_route_check_owner(vtss_routing_param_owner_t owner)
{
    IP2_CRIT_ASSERT_LOCKED();
    switch (owner) {
    case VTSS_ROUTING_PARAM_OWNER_STATIC_USER:
    case VTSS_ROUTING_PARAM_OWNER_DYNAMIC_USER:
    case VTSS_ROUTING_PARAM_OWNER_DHCP:
        return TRUE;
    default:
        T_E("Invalid owner: %d", owner);
    }
    return FALSE;
}

static vtss_rc IP2_route_add(const vtss_routing_entry_t  *const conf_rt,
                             const vtss_routing_params_t *const params)
{
    vtss_routing_entry_t os_rt;
    ip2_route_db_entry_t entry, *dbp;
    vtss_rc rc;

    IP2_CRIT_ASSERT_LOCKED();

    if (!vtss_ip_route_valid(conf_rt) ||
        !IP2_route_check_owner(params->owner) ||
        !vtss_ip_route_nh_valid(conf_rt)) {
        T_I("Failed to validate route");
        return IP2_ERROR_PARAMS;
    }

    rc = IP2_route_convert_conf_to_os(conf_rt, &os_rt);
    if (rc != VTSS_RC_OK) {
        T_I("Failed to convert route from conf domain to os domain");
        return rc;
    }

    /* Do we have this already? */
    if ((dbp = IP2_route_db_lookup(&entry, &os_rt)) != NULL) {
        if (dbp->users & VTSS_BIT(params->owner)) {
            return IP2_ERROR_EXISTS;
        }
        /* Mark extra owner of route */
        dbp->users |= VTSS_BIT(params->owner);
    } else {

        if (params->owner == VTSS_ROUTING_PARAM_OWNER_STATIC_USER &&
            IP2_state.static_routes == IP2_MAX_ROUTES) {
            return IP2_ERROR_NOSPACE;
        }

        /* Add to route DB */
        if ((rc = IP2_route_db_add(&entry, params->owner)) != VTSS_OK) {
            return rc;
        }
    }
    if (params->owner == VTSS_ROUTING_PARAM_OWNER_STATIC_USER) {
        IP2_state.static_routes++;
        IP2_route_db_static_save();
    }
    return VTSS_OK;
}

static vtss_rc IP2_route_del(const vtss_routing_entry_t  *const conf_rt,
                             const vtss_routing_params_t *const params)
{
    vtss_routing_entry_t os_rt;
    ip2_route_db_entry_t entry, *dbp;
    vtss_rc rc;

    IP2_CRIT_ASSERT_LOCKED();
    if (!IP2_route_check_owner(params->owner)) {
        return IP2_ERROR_PARAMS;
    }

    rc = IP2_route_convert_conf_to_os(conf_rt, &os_rt);
    if (rc != VTSS_RC_OK) {
        T_I("Failed to convert route from conf domain to os domain");
        return rc;
    }

    if ((dbp = IP2_route_db_lookup(&entry, &os_rt)) != NULL && (dbp->users & VTSS_BIT(params->owner))) {
        /* Remove user from route, possibly purge route */
        if ((rc = IP2_route_db_del(dbp, params->owner)) != VTSS_OK) {
            return rc;
        }
        if (params->owner == VTSS_ROUTING_PARAM_OWNER_STATIC_USER) {
            IP2_state.static_routes--;
            IP2_route_db_static_save();
        }
        return VTSS_OK;
    }
    return IP2_ERROR_NOTFOUND;
}

static vtss_rc IP2_route_get(vtss_routing_entry_type_t type,
                             u32                       max,
                             vtss_routing_status_t     *rt,
                             u32                       *const cnt)
{
    u32 i;
    vtss_rc rc;
    ip2_route_db_entry_t entry, *dbp;

    IP2_CRIT_ASSERT_LOCKED();
    rc = vtss_ip2_os_route_get(type, max, rt, cnt);
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    for (i = 0; i < *cnt; ++i, ++rt) {
        rc = IP2_route_convert_os_to_conf(&rt->rt, &rt->rt);
        if (rc != VTSS_RC_OK) {
            T_D("Failed to convert route to os");
            return rc;
        }
        if ((dbp = IP2_route_db_lookup(&entry, &rt->rt)) != NULL) {
            rt->params.owner = (u32) dbp->users;
            T_D("Owner found: %x", rt->params.owner);
        } else {
            rt->params.owner = 0;
            T_D("No owner found: %x", rt->params.owner);
        }
    }

    return VTSS_RC_OK;
}


inline static void IP2_sync_ifflags(int if_no, BOOL forwarding)
{
    ip2_ifstate_t *if_state;
    vtss_rc rc;

    IP2_CRIT_ASSERT_LOCKED();
    if_state = &IP2_state.interfaces[if_no];
    T_I("vlan%d set %s", if_state->vid, forwarding ? "up" : "down");
    if ((rc = vtss_ip2_os_if_ctl(if_state->vid, forwarding)) != VTSS_OK) {
        T_W("vlan%d: IFF flags set %s: %s",  if_state->vid, forwarding ? "up" : "down", error_txt(rc));
    }
}

inline static void IP2_if_poll_fwd_state(uint if_no)
{
    IP2_CRIT_ASSERT_LOCKED();

    if (IP2_state.interfaces[if_no].vid != VTSS_VID_NULL) {
        ip2_vif_fwd_t new_fwd = VIF_FWD_BLOCKING; /* Assume blocking */
        ip2_vif_fwd_t old_fwd = IP2_state.master_fwd[if_no];
        switch_iter_t sit;

        T_D("iface %d cur state %d (%s)", if_no, old_fwd, old_fwd == VIF_FWD_FORWARDING ? "forwarding" : "blocking");

        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
        while (switch_iter_getnext(&sit)) {
            if (IP2_state.unit_fwd[sit.isid - VTSS_ISID_START][if_no] == VIF_FWD_FORWARDING) {
                new_fwd = VIF_FWD_FORWARDING;
                T_D("iface %d unit %d is forwarding\n", if_no, sit.usid);
                break; /* We have at least one unit forwarding, done */
            }
        }

        T_D("iface %d new state %d (%s)", if_no, new_fwd, new_fwd == VIF_FWD_FORWARDING ? "forwarding" : "blocking");

        if (old_fwd != new_fwd) {
            IP2_state.master_fwd[if_no] = new_fwd;
            T_I("iface %d change state from %d to %d", if_no, old_fwd, new_fwd);
            IP2_sync_ifflags(if_no, new_fwd == VIF_FWD_FORWARDING);
        }
    }
}

static void ip2_thread(cyg_addrword_t data)
{
    cyg_flag_value_t flags;
    cyg_tick_count_t wakeup;
    cyg_tick_count_t sleep_time;
    wakeup = cyg_current_time() + (3000 / ECOS_MSECS_PER_HWTICK);

    for (;;) {
        T_I("Master up");
        while (msg_switch_is_master()) {
            if (cyg_current_time() >= wakeup) {
                wakeup = cyg_current_time() + (3000 / ECOS_MSECS_PER_HWTICK);

                IP2_CRIT_ENTER();
                IP2_request_fwdstate(); /* @ each 3 seconds */
                IP2_CRIT_EXIT();
            }

            IP2_CRIT_ENTER();
            sleep_time = IP2_dhcpc_fallback_timer_evaluate(wakeup);
            IP2_CRIT_EXIT();

            while ((flags = cyg_flag_timed_wait(&ip2_control_flags, 0xffff,
                                                CYG_FLAG_WAITMODE_OR |
                                                CYG_FLAG_WAITMODE_CLR, sleep_time)) &&
                   msg_switch_is_master()) {
                if (flags & CTLFLAG_IFCHANGE) {
                    uint if_no;

                    IP2_CRIT_ENTER();
                    for (if_no = 0; if_no < IP2_MAX_INTERFACES; if_no++) {
                        IP2_if_poll_fwd_state(if_no);
                    }
                    IP2_CRIT_EXIT();
                }
                if (flags & CTLFLAG_RTREFRESH) {
                    IP2_CRIT_ENTER();
                    IP2_route_db_apply();
                    IP2_CRIT_EXIT();
                }
                if (flags & CTLFLAG_IFNOTIFY) {
                    vtss_ip2_if_callback();
                }
            }
            T_R("Tick");
        }

        /* Cleanup route DB */
        IP2_CRIT_ENTER();
        IP2_route_db_reset();
        IP2_CRIT_EXIT();

        T_I("Slave, suspending");
        cyg_thread_suspend(ip2_thread_handle);
    }
}


/* -------------------------------------------------------------------------
   Implementation of public API starts here
   ------------------------------------------------------------------------- */
const char *ip2_error_txt(vtss_rc rc)
{
    switch (rc) {
    case IP2_ERROR_EXISTS:
        return "Already exists";
    case IP2_ERROR_NOTFOUND:
        return "Not Found";
    case IP2_ERROR_NOSPACE:
        return "No space";
    case IP2_ERROR_PARAMS:
        return "Invalid parameters";
    case IP2_ERROR_FAILED:
        return "Operation failed";
    case IP2_ERROR_ADDRESS_CONFLICT:
        return "Address conflict";
    case IP2_ERROR_NOT_MASTER:
        return "Not master";
    default:
        return "IP: \?\?\?";
    }
}

/* Global config ----------------------------------------------------------- */
vtss_rc vtss_ip2_global_param_set(const vtss_ip2_global_param_t *const param)
{
    vtss_rc rc;

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    IP2_stack_conf.global = *param;
#if !defined(VTSS_SW_OPTION_L3RT)
    IP2_stack_conf.global.enable_routing = 0;
#endif
    rc = vtss_ip2_os_global_param_set(param);

#ifdef VTSS_SW_OPTION_SNMP
    // we need to update a timer when forwarding is enabled/disabled
    vtss_ip2_snmp_signal_global_changes();
#endif

    IP2_flash_write();
    IP2_CRIT_EXIT();
    return rc;
}

vtss_rc vtss_ip2_global_param_get(vtss_ip2_global_param_t *param)
{
    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    *param = IP2_stack_conf.global;
#if !defined(VTSS_SW_OPTION_L3RT)
    param->enable_routing = 0;
#endif
    IP2_CRIT_EXIT();
    return VTSS_OK;
}

/* Interface functions ----------------------------------------------------- */
BOOL vtss_ip2_if_exists(vtss_if_id_vlan_t if_id)
{
    if (!msg_switch_is_master()) {
        return FALSE;
    }

    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN(BOOL, IP2_if_exists(if_id));
}

vtss_rc IP2_if_id_next(vtss_if_id_vlan_t  current,
                       vtss_if_id_vlan_t *const next)
{
    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    T_R("Get-next iface %d", current);

    // First candidat is the next
    current++;
    for (; current <= VLAN_ID_MAX; ++current ) {
        if (IP2_state.vid2if[current] != IFNO_INVALID) {
            *next = current;
            return VTSS_RC_OK;
        }
    }
    return VTSS_RC_ERROR;
}

vtss_rc vtss_ip2_if_id_next(vtss_if_id_vlan_t  current,
                            vtss_if_id_vlan_t *const next)
{
    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(IP2_if_id_next(current, next));
}

vtss_rc vtss_ip2_if_conf_get(vtss_if_id_vlan_t  if_id,
                             vtss_if_param_t   *const param)
{
    ip2_ifstate_t *ifs;

    T_N("%s - vid %d", __FUNCTION__, if_id);

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    if ((ifs = IP2_if2state(if_id))) {
        /* Get params */
        *param = ifs->config->link_layer;
        IP2_CRIT_RETURN_RC(VTSS_OK);
    }

    IP2_CRIT_RETURN_RC(IP2_ERROR_PARAMS);
}

vtss_rc vtss_ip2_if_conf_set(vtss_if_id_vlan_t      vlan,
                             const vtss_if_param_t *const param)
{
    vtss_rc rc;
    ip2_ifstate_t *ifs;

    T_N("%s - vid %d", __FUNCTION__, vlan);

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();

    if (vlan < VLAN_ID_MIN || vlan > VLAN_ID_MAX) {
        IP2_CRIT_RETURN_RC(IP2_ERROR_PARAMS);
    }

    ifs = IP2_if2state(vlan);
    if (!ifs) {
        // Add new interface
        ifs = IP2_if_new(vlan);
        if (ifs == NULL) {
            T_W("IP2_if_new failed vlan: %u", vlan);
            IP2_CRIT_RETURN_RC(IP2_ERROR_NOSPACE);
        }

        rc = vtss_ip2_os_if_add(ifs->vid);
        if (rc != VTSS_RC_OK) {
            T_W("vtss_ip2_os_if_add failed vlan: %u", vlan);
        }
    }

    // update interface settings
    rc = vtss_ip2_os_if_set(ifs->vid, param);
    if (rc != VTSS_RC_OK) {
        T_E("Failed to apply interface parameters. Vlan: %u", vlan);
    }

    ifs->config->link_layer = *param;
    IP2_if_signal(ifs->vid);
    IP2_flash_write();
    IP2_CRIT_RETURN_RC(rc);
}

vtss_rc vtss_ip2_if_conf_del(vtss_if_id_vlan_t if_id)
{
    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(IP2_if_del(if_id));
}

static void _ifs_statistics_merge(vtss_ip_type_t         version,
                                  vtss_if_id_t           *ifidx,
                                  vtss_ip_stat_data_t    *entry)
{
    vtss_l3_counters_t  *hwstat, hw_cntr;

    if (!ifidx || (ifidx->type != VTSS_ID_IF_TYPE_VLAN) || !entry) {
        return;
    }

    memset(&hw_cntr, 0x0, sizeof(vtss_l3_counters_t));
    if (vtss_ip2_chip_counters_vlan_get(ifidx->u.vlan, &hw_cntr) == VTSS_OK) {
        hwstat = &hw_cntr;

        if (version == VTSS_IP_TYPE_IPV4) {
            entry->HCInReceives += hwstat->ipv4uc_received_frames;
            entry->HCInOctets += hwstat->ipv4uc_received_octets;
            entry->HCOutTransmits += hwstat->ipv4uc_transmitted_frames;
            entry->HCOutOctets += hwstat->ipv4uc_transmitted_octets;
        } else if (version == VTSS_IP_TYPE_IPV6) {
            entry->HCInReceives += hwstat->ipv6uc_received_frames;
            entry->HCInOctets += hwstat->ipv6uc_received_octets;
            entry->HCOutTransmits += hwstat->ipv6uc_transmitted_frames;
            entry->HCOutOctets += hwstat->ipv6uc_transmitted_octets;
        } else {
            return;
        }

        entry->InReceives = (u32)((entry->HCInReceives << 32) >> 32);
        entry->InOctets = (u32)((entry->HCInOctets << 32) >> 32);
        entry->OutTransmits = (u32)((entry->HCOutTransmits << 32) >> 32);
        entry->OutOctets = (u32)((entry->HCOutOctets << 32) >> 32);
    }
}

vtss_rc IP2_if_chip_status_merge(const vtss_if_status_type_t    type,
                                 const u32                      *cnt,
                                 vtss_if_status_t               *status)
{
    u32                 idx;
    vtss_if_status_t    *ifs;

    if ((type != VTSS_IF_STATUS_TYPE_ANY) &&
        (type != VTSS_IF_STATUS_TYPE_STAT_IPV4) &&
        (type != VTSS_IF_STATUS_TYPE_STAT_IPV6)) {
        return VTSS_OK;
    }

    for (idx = 0; idx < *cnt; idx++) {
        ifs = &status[idx];

        if (ifs->type == VTSS_IF_STATUS_TYPE_STAT_IPV4) {
            _ifs_statistics_merge(VTSS_IP_TYPE_IPV4, &ifs->if_id, &ifs->u.ip_stat.data);
        }
        if (ifs->type == VTSS_IF_STATUS_TYPE_STAT_IPV6) {
            _ifs_statistics_merge(VTSS_IP_TYPE_IPV6, &ifs->if_id, &ifs->u.ip_stat.data);
        }
    }

    return VTSS_OK;
}

vtss_rc IP2_if_non_os_status_get(vtss_if_status_type_t        type,
                                 vtss_if_id_vlan_t            id,
                                 const u32                    max,
                                 u32                         *cnt,
                                 vtss_if_status_t            *status)
{
    ip2_ifstate_t *ifs;

    ifs = IP2_if2state(id);
    T_N("Considering vlan %u", id);
    if (ifs == NULL) {
        T_E("Failed to get ifs for vlan %u", id);
        return VTSS_RC_ERROR;
    }

    *cnt = 0;

    // check if this interface uses a dhcp v4 client
    if (*cnt < max && // room for more status objects
        (type == VTSS_IF_STATUS_TYPE_DHCP ||
         type == VTSS_IF_STATUS_TYPE_ANY) &&
        ifs->config->ipv4.dhcpc) {

        vtss_rc rc_;
        vtss_dhcp_client_status_t status_;

        T_N("Vlan %u is using dhcp", id);

        // get dhcp v4 client status
        rc_ = vtss_dhcp_client_status(id, &status_);

        if (rc_ == VTSS_RC_OK) {
            status[*cnt].if_id.type = VTSS_ID_IF_TYPE_VLAN;
            status[*cnt].if_id.u.vlan = id;
            status[*cnt].type = VTSS_IF_STATUS_TYPE_DHCP;
            status[*cnt].u.dhcp4c = status_;
            *cnt = (*cnt) + 1;

        } else {
            T_E("Failed to get DHCP status for vlan %u", id);

        }
    }

    // Add other interface status objects here

    return VTSS_RC_OK;
}

vtss_rc IP2_if_status_get(vtss_if_status_type_t        type,
                          vtss_if_id_vlan_t            id,
                          const u32                    max,
                          u32                         *cnt,
                          vtss_if_status_t            *status)
{
    u32 cnt2;
    vtss_rc rc;


    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ASSERT_LOCKED();
    T_R("Get status type %d vlan %d", type, id);

    *cnt = 0;
    cnt2 = 0;

    // get os status
    rc = vtss_ip2_os_if_status(type, max, cnt, id, status);
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    // merge statistics from both sw & hw
    rc = IP2_if_chip_status_merge(type, cnt, status);
    if (rc != VTSS_OK) {
        return rc;
    }

    // get non-os status
    rc = IP2_if_non_os_status_get(type, id, max - *cnt, &cnt2, &status[*cnt]);
    *cnt += cnt2;

    return rc;
}

vtss_rc vtss_ip2_if_status_get(vtss_if_status_type_t        type,
                               vtss_if_id_vlan_t            id,
                               const u32                    max,
                               u32                         *cnt,
                               vtss_if_status_t            *status)
{
    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(IP2_if_status_get(type, id, max, cnt, status));
}

vtss_rc IP2_if_status_get_first(vtss_if_status_type_t        type,
                                vtss_if_id_vlan_t            id,
                                vtss_if_status_t            *status)
{
    u32 cnt;
    vtss_rc rc;

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ASSERT_LOCKED();

    cnt = 0;
    rc = IP2_if_status_get(type, id, 1, &cnt, status);

    if (rc != VTSS_RC_OK) {
        return rc;
    }

    if (cnt != 1) {
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_ip2_if_status_get_first(vtss_if_status_type_t        type,
                                     vtss_if_id_vlan_t            id,
                                     vtss_if_status_t            *status)
{
    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(IP2_if_status_get_first(type, id, status));
}

static vtss_rc IP2_ifs_status_get(vtss_if_status_type_t   type,
                                  const u32               max,
                                  u32                    *cnt,
                                  vtss_if_status_t       *status)
{
    /*lint --e{429} */
    vtss_rc rc;
    vtss_vid_t cur, next;
    u32 os_cnt, non_os_cnt, tmp;
    vtss_if_status_t *os_status, *non_os_status;

    IP2_CRIT_ASSERT_LOCKED();

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    os_cnt = 0;
    non_os_cnt = 0;
    os_status = status;

    // get all os status
    rc = vtss_ip2_os_if_status_all(type, max, &os_cnt, os_status);
    if (rc != VTSS_RC_OK) {
        return rc;
    }

    // merge statistics from both sw & hw
    rc = IP2_if_chip_status_merge(type, &os_cnt, status);
    if (rc != VTSS_OK) {
        return rc;
    }

    // get all non os status
    non_os_status = &status[os_cnt];
    for (cur = 0; IP2_if_id_next(cur, &next) == VTSS_RC_OK; cur = next) {
        tmp = 0;
        rc = IP2_if_non_os_status_get(type, next, max - non_os_cnt, &tmp,
                                      &(non_os_status[non_os_cnt]));

        if (rc == VTSS_RC_OK) {
            non_os_cnt += tmp;
        } else {
            T_E("IP2_if_non_os_status_get failed for vlan %u", next);
        }
    }

    *cnt = os_cnt + non_os_cnt;
    return VTSS_RC_OK;
}

vtss_rc vtss_ip2_ifs_status_get(vtss_if_status_type_t   type,
                                const u32               max,
                                u32                    *cnt,
                                vtss_if_status_t       *status)
{
    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(IP2_ifs_status_get(type, max, cnt, status));
}

vtss_rc vtss_ip2_if_inject(vtss_if_id_vlan_t         if_id,
                           u32                  length,
                           const u8            *const data)
{
    ip2_ifstate_t *ifs;

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    ifs = IP2_if2state(if_id);
    if (!ifs) {
        IP2_CRIT_RETURN_RC(IP2_ERROR_PARAMS);
    }
    IP2_CRIT_RETURN_RC(vtss_ip2_os_inject(ifs->vid, length, data));
}

vtss_rc vtss_ip2_if_callback_add(const vtss_ip2_if_callback_t cb)
{
    int i;

    IP2_CRIT_ENTER();
    for (i = 0; i < MAX_SUBSCRIBER; ++i) {
        if (IP2_subscribers[i] != NULL) {
            continue;
        }

        IP2_subscribers[i] = cb;
        IP2_CRIT_RETURN_RC(VTSS_RC_OK);
    }
    IP2_CRIT_RETURN_RC(VTSS_RC_ERROR);
}

vtss_rc vtss_ip2_if_callback_del(const vtss_ip2_if_callback_t cb)
{
    int i;

    IP2_CRIT_ENTER();
    for (i = 0; i < MAX_SUBSCRIBER; ++i) {
        if (IP2_subscribers[i] != cb) {
            continue;
        }

        IP2_subscribers[i] = NULL;
        IP2_CRIT_RETURN_RC(VTSS_RC_OK);
    }
    IP2_CRIT_RETURN_RC(VTSS_RC_ERROR);
}

void vtss_if_default_param(vtss_if_param_t *param)
{
    param->mtu = 1500;
}

/* IP address functions ---------------------------------------------------- */
vtss_rc vtss_ip2_ipv4_conf_get(vtss_if_id_vlan_t  if_id,
                               vtss_ip_conf_t    *conf)
{
    ip2_ifstate_t *ifs;

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    ifs = IP2_if2state(if_id);

    if (!ifs) {
        IP2_CRIT_RETURN_RC(IP2_ERROR_PARAMS);
    }

    *conf = ifs->config->ipv4;
    IP2_CRIT_RETURN_RC(VTSS_RC_OK);
}

vtss_rc vtss_ip2_ipv4_conf_set(vtss_if_id_vlan_t            if_id,
                               const vtss_ip_conf_t        *conf)
{
    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(IP2_ipv4_conf_set(if_id, conf, FALSE));
}

vtss_rc vtss_ip2_ipv4_dhcp_restart(vtss_if_id_vlan_t if_id)
{
    ip2_ifstate_t *ifs;

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    ifs = IP2_if2state(if_id);

    if (!ifs) {
        IP2_CRIT_RETURN_RC(IP2_ERROR_PARAMS);
    }

    if (!ifs->config->ipv4.dhcpc) {
        IP2_CRIT_RETURN_RC(VTSS_RC_ERROR);
    }

    (void) vtss_ip2_os_ip_del(ifs->vid, &ifs->cur_ipv4);

    IP2_CRIT_RETURN_RC(vtss_dhcp_client_start(if_id));
}

vtss_rc vtss_ip2_ipv6_conf_get(vtss_if_id_vlan_t            if_id,
                               vtss_ip_conf_t              *conf)
{
    ip2_ifstate_t *ifs;

    if (!vtss_ip2_hasipv6() || !msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    ifs = IP2_if2state(if_id);

    if (!ifs) {
        IP2_CRIT_RETURN_RC(IP2_ERROR_PARAMS);
    }

    *conf = ifs->config->ipv6;
    IP2_CRIT_RETURN_RC(VTSS_RC_OK);
}

vtss_rc vtss_ip2_ipv6_conf_set(vtss_if_id_vlan_t            if_id,
                               const vtss_ip_conf_t        *conf)
{
    ip2_ifstate_t   *ifs;

    if (!conf || !vtss_ip2_hasipv6() || !msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    if ((ifs = IP2_if2state(if_id)) == NULL) {
        IP2_CRIT_RETURN_RC(IP2_ERROR_PARAMS);
    }

    if (!memcmp(conf, &ifs->config->ipv6, sizeof(vtss_ip_conf_t))) {
        IP2_CRIT_RETURN_RC(VTSS_RC_OK);
    }

    IP2_CRIT_RETURN_RC(IP2_ipv6_conf_set(if_id, conf));
}

// No need to lock, does not use global data, and calls only functions which
// will acquire the needed locks on its own.
vtss_rc vtss_ip2_ip_to_if(const vtss_ip_addr_t *const ip,
                          vtss_if_status_t     *if_status)
{
    // NOTE: This could be optimized by inspected the kernel space interface
    // structures instead. If this is done, there would be no need for a memory
    // allocation.
    u32 cnt, i;
    vtss_if_status_t *status;
    vtss_rc rc = VTSS_RC_ERROR;

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    status = VTSS_CALLOC(IP2_MAX_STATUS_OBJS, sizeof(vtss_if_status_t));

    if (status == NULL) {
        T_E("Calloc failed");
        return VTSS_RC_ERROR;
    }

    rc = vtss_ip2_ifs_status_get(VTSS_IF_STATUS_TYPE_ANY,
                                 IP2_MAX_STATUS_OBJS,
                                 &cnt,
                                 status);
    if (rc != VTSS_RC_OK) {
        goto done;
    }

    for (i = 0; i < cnt; ++i, ++status) {
        if (vtss_if_status_match_ip(ip, status)) {
            *if_status = *status;
            rc = VTSS_RC_OK;
            goto done;
        }
    }

done:
    /*lint --e{673} */
    VTSS_FREE(status);
    return rc;
}

vtss_rc vtss_ip2_ip_by_vlan(const vtss_vid_t  vlan,
                            vtss_ip_type_t    type,
                            vtss_ip_addr_t   *const src)
{
    u32 i;

    IP2_CRIT_ENTER();
    for (i = 0; i < IP2_if_status_cache.size; ++i) {
        if (!vtss_if_status_match_ip_type(type, &IP2_if_status_cache.data[i])) {
            continue;
        }

        if (!vtss_if_status_match_vlan(vlan, &IP2_if_status_cache.data[i])) {
            continue;
        }

        // We found one
        IP2_CRIT_RETURN_RC(vtss_if_status_to_ip(&IP2_if_status_cache.data[i],
                                                src));
    }

    T_I("Failed to get ip by vlan, where vlan = %u", vlan);
    IP2_CRIT_RETURN_RC(VTSS_RC_ERROR);
}

// Function for getting ip for a specific port. The IP returned is based upon
// the PVID for the given port.
vtss_rc vtss_ip2_ip_by_port(const vtss_port_no_t  iport,
                            vtss_ip_type_t        type,
                            vtss_ip_addr_t       *const src)
{
    vtss_rc rc;
    vlan_port_conf_t vlan_conf;

    // Getting PVID
    rc = vlan_mgmt_port_conf_get(VTSS_ISID_LOCAL, iport,
                                 &vlan_conf, VLAN_USER_ALL);

    if (rc != VTSS_RC_OK) {
        T_I("Failed to convert port %u to a vlan", iport);
        return rc;
    }

    // Getting the IP for the VLAN with the PVID
    return vtss_ip2_ip_by_vlan(vlan_conf.pvid, type, src); // acquire lock
}

/* IP route functions ------------------------------------------------------ */
vtss_rc vtss_ip2_route_add(const vtss_routing_entry_t  *const rt,
                           const vtss_routing_params_t *const params)
{
    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(IP2_route_add(rt, params));
}

vtss_rc vtss_ip2_route_del(const vtss_routing_entry_t  *const rt,
                           const vtss_routing_params_t *const params)
{
    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(IP2_route_del(rt, params));
}

vtss_rc vtss_ip2_route_get(vtss_routing_entry_type_t    type,
                           u32                          max,
                           vtss_routing_status_t       *rt,
                           u32                         *const cnt)
{
    vtss_rc rc;

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    rc = IP2_route_get(type, max, rt, cnt);
    IP2_CRIT_RETURN_RC(rc);
}

vtss_rc vtss_ip2_route_getnext(     const vtss_routing_entry_t  *const key,
                                    vtss_routing_entry_t        *const next)
{
    ip2_route_db_entry_t entry, *dbp;
    BOOL found;

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    dbp = &entry;
    if (key) {
        dbp->route.route = *key;
    }
    found = vtss_avl_tree_get(&IP2_route_db,
                              (void **) &dbp,
                              key == NULL ?
                              VTSS_AVL_TREE_GET_FIRST : VTSS_AVL_TREE_GET_NEXT);
    if (found) {
        VTSS_ASSERT(next != NULL && dbp != NULL);
        *next = dbp->route.route;
    }
    IP2_CRIT_RETURN_RC(found ? VTSS_OK : IP2_ERROR_FAILED);
}

vtss_rc vtss_ip2_route_conf_get(const int              const max,
                                vtss_routing_entry_t   *rto,
                                int                    *const cnt)
{
    /*lint --e{429} */

    int i, j;
    T_D("%s", __FUNCTION__);

    if (!msg_switch_is_master()) {
        return IP2_ERROR_FAILED;
    }

    IP2_CRIT_ENTER();
    for (i = j = 0; i < IP2_MAX_ROUTES && j < max; i++) {
        ip2_route_entry_t *rtc = &IP2_stack_conf.routes[i];
        if (rtc->route.type != VTSS_ROUTING_ENTRY_TYPE_INVALID) {
            T_D("Get route in entry %d -> slot %d", i, j);
            rto[j++] = rtc->route;
        }
    }
    *cnt = j;
    IP2_CRIT_RETURN_RC(VTSS_OK);
}

vtss_rc vtss_ip2_route_get_info(    const vtss_routing_entry_t  *const key,
                                    vtss_routing_info_t         *const info)
{
    ip2_route_db_entry_t entry, *dbp;
    IP2_CRIT_ENTER();
    if ((dbp = IP2_route_db_lookup(&entry, key)) != NULL) {
        if (info) {
            info->owners = (u32) dbp->users;
        }
    }
    IP2_CRIT_RETURN_RC(dbp ? VTSS_OK : IP2_ERROR_FAILED);
}

/* Statistics Section: RFC-4293 */
static void IP2_ips_statistics_collect(vtss_ip_stat_data_t *entry,
                                       vtss_ip_stat_data_t *stat)
{
    if (!entry || !stat) {
        return;
    }

    entry->InReceives += stat->InReceives;
    entry->HCInReceives += stat->HCInReceives;
    entry->InOctets += stat->InOctets;
    entry->HCInOctets += stat->HCInOctets;
    entry->InHdrErrors += stat->InHdrErrors;
    entry->InNoRoutes += stat->InNoRoutes;
    entry->InAddrErrors += stat->InAddrErrors;
    entry->InUnknownProtos += stat->InUnknownProtos;
    entry->InTruncatedPkts += stat->InTruncatedPkts;
    entry->InForwDatagrams += stat->InForwDatagrams;
    entry->HCInForwDatagrams += stat->HCInForwDatagrams;
    entry->ReasmReqds += stat->ReasmReqds;
    entry->ReasmOKs += stat->ReasmOKs;
    entry->ReasmFails += stat->ReasmFails;
    entry->InDiscards += stat->InDiscards;
    entry->InDelivers += stat->InDelivers;
    entry->HCInDelivers += stat->InDelivers;
    entry->OutRequests += stat->OutRequests;
    entry->HCOutRequests += stat->OutRequests;
    entry->OutForwDatagrams += stat->OutForwDatagrams;
    entry->HCOutForwDatagrams += stat->OutForwDatagrams;
    entry->OutDiscards += stat->OutDiscards;
    entry->OutFragReqds += stat->OutFragReqds;
    entry->OutFragOKs += stat->OutFragOKs;
    entry->OutFragFails += stat->OutFragFails;
    entry->OutFragCreates += stat->OutFragCreates;
    entry->OutTransmits += stat->OutTransmits;
    entry->HCOutTransmits += stat->OutTransmits;
    entry->OutOctets += stat->OutOctets;
    entry->HCOutOctets += stat->OutOctets;
    entry->InMcastPkts += stat->InMcastPkts;
    entry->HCInMcastPkts += stat->InMcastPkts;
    entry->InMcastOctets += stat->InMcastOctets;
    entry->HCInMcastOctets += stat->InMcastOctets;
    entry->OutMcastPkts += stat->OutMcastPkts;
    entry->HCOutMcastPkts += stat->OutMcastPkts;
    entry->OutMcastOctets += stat->OutMcastOctets;
    entry->HCOutMcastOctets += stat->OutMcastOctets;
    entry->InBcastPkts += stat->InBcastPkts;
    entry->HCInBcastPkts += stat->InBcastPkts;
    entry->OutBcastPkts += stat->OutBcastPkts;
    entry->HCOutBcastPkts += stat->OutBcastPkts;

    /* most recent occasion */
    if (!entry->DiscontinuityTime.sec_msb &&
        !entry->DiscontinuityTime.seconds &&
        !entry->DiscontinuityTime.nanoseconds) {
        entry->DiscontinuityTime.sec_msb = 0;
        entry->DiscontinuityTime.seconds = stat->DiscontinuityTime.seconds;
        entry->DiscontinuityTime.nanoseconds = stat->DiscontinuityTime.nanoseconds;
    } else {
        if (entry->DiscontinuityTime.seconds < stat->DiscontinuityTime.seconds) {
            entry->DiscontinuityTime.sec_msb = 0;
            entry->DiscontinuityTime.seconds = stat->DiscontinuityTime.seconds;
            entry->DiscontinuityTime.nanoseconds = stat->DiscontinuityTime.nanoseconds;
        } else {
            if ((entry->DiscontinuityTime.seconds == stat->DiscontinuityTime.seconds) &&
                (entry->DiscontinuityTime.nanoseconds < stat->DiscontinuityTime.nanoseconds)) {
                entry->DiscontinuityTime.sec_msb = 0;
                entry->DiscontinuityTime.seconds = stat->DiscontinuityTime.seconds;
                entry->DiscontinuityTime.nanoseconds = stat->DiscontinuityTime.nanoseconds;
            }
        }
    }
    entry->RefreshRate = IP2_STAT_REFRESH_RATE;
}

vtss_rc vtss_ip2_ips_status_get(    vtss_ips_status_type_t       type,
                                    const u32                    max,
                                    u32                         *cnt,
                                    vtss_ips_status_t           *status)
{
    vtss_rc                 rc;
    vtss_ips_status_type_t  mint, maxt;
    vtss_if_status_type_t   ift;
    vtss_if_status_t        *fdx, *ifs;
    vtss_ip_type_t          version;
    u32                     _cnt, ifc, idx, ipoutnoroute;

    if (!cnt || !status || !msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    mint = maxt = VTSS_IPS_STATUS_TYPE_ANY;
    if (type == VTSS_IPS_STATUS_TYPE_INVALID) {
        return VTSS_RC_ERROR;
    } else if (type == VTSS_IPS_STATUS_TYPE_ANY) {
        mint = VTSS_IPS_STATUS_TYPE_STAT_IPV4;
        maxt = VTSS_IPS_STATUS_TYPE_STAT_ICMP6;
    } else {
        mint = maxt = type;
    }

    ifs = VTSS_CALLOC(IP2_MAX_INTERFACES, sizeof(vtss_if_status_t));
    if (ifs == NULL) {
        T_E("Alloc error");
        return VTSS_RC_ERROR;
    }

    rc = VTSS_OK;
    _cnt = 0;
    IP2_CRIT_ENTER();
    for (; mint <= maxt; mint++) {
        if (_cnt >= max) {
            break;
        }

        switch ( mint ) {
        case VTSS_IPS_STATUS_TYPE_STAT_IPV4:
        case VTSS_IPS_STATUS_TYPE_STAT_IPV6:
            ifc = ipoutnoroute = 0;
            ift = (mint == VTSS_IPS_STATUS_TYPE_STAT_IPV4) ? VTSS_IF_STATUS_TYPE_STAT_IPV4 : VTSS_IF_STATUS_TYPE_STAT_IPV6;
            version = (mint == VTSS_IPS_STATUS_TYPE_STAT_IPV4) ? VTSS_IP_TYPE_IPV4 : VTSS_IP_TYPE_IPV6;
            rc = vtss_ip2_os_if_status_all(ift,
                                           IP2_MAX_INTERFACES,
                                           &ifc,
                                           ifs);
            // merge statistics from both sw & hw
            if (rc == VTSS_OK) {
                rc = IP2_if_chip_status_merge(ift, &ifc, ifs);
            }

            if ((rc == VTSS_OK) &&
                (vtss_ip2_os_stat_ipoutnoroute_get(version, &ipoutnoroute) == VTSS_OK)) {
                status->version = version;
                status->imsg = IP2_STAT_IMSG_MAX;
                status->type = mint;
                status->u.ip_stat.IPVersion = status->version;
                memset(&status->u.ip_stat.data, 0x0, sizeof(vtss_ip_stat_data_t));
                /* ipSystemStatsOutNoRoutes */
                status->u.ip_stat.data.OutNoRoutes = ipoutnoroute;
                for (idx = 0; idx < ifc ; idx++) {
                    fdx = &ifs[idx];
                    IP2_ips_statistics_collect(&status->u.ip_stat.data, &fdx->u.ip_stat.data);
                }

                _cnt++;
                status++;
            }

            break;
        case VTSS_IPS_STATUS_TYPE_STAT_ICMP4:
        case VTSS_IPS_STATUS_TYPE_STAT_ICMP6:
            version = (mint == VTSS_IPS_STATUS_TYPE_STAT_ICMP4) ? VTSS_IP_TYPE_IPV4 : VTSS_IP_TYPE_IPV6;
            if (vtss_ip2_os_stat_icmp_cntr_get(version, &status->u.icmp_stat) == VTSS_OK) {
                status->version = version;
                status->imsg = IP2_STAT_IMSG_MAX;
                status->type = mint;

                _cnt++;
                status++;
            }

            break;
        default:
            rc = VTSS_RC_ERROR;

            break;
        }

        if (rc != VTSS_OK) {
            break;
        }
    }
    IP2_CRIT_EXIT();

    *cnt = _cnt;
    VTSS_FREE(ifs);
    return rc;
}

/**
 * Clear system's IP statistics.
 *
 * \param version (IN) - Specify the IP version (IPv4/IPv6) to clear counters.
 * version MUST be either IPv4 or IPv6.
 * \note Only stacking master is allowed for this operation.
 *
 * \return VTSS_OK iff operation is done successfully.
 *
 */
vtss_rc vtss_ip2_stat_syst_cntr_clear(vtss_ip_type_t version)
{
    vtss_rc             rc;
    vtss_if_id_vlan_t   idx, nxt;

    if (!msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    IP2_CRIT_ENTER();
    if (vtss_ip2_os_stat_syst_cntr_clear(version) != VTSS_OK) {
        IP2_CRIT_EXIT();
        T_D("Software system IP counter Clear failure!");
        return VTSS_RC_ERROR;
    }

    rc = VTSS_OK;
    for (idx = 0; (IP2_if_id_next(idx, &nxt) == VTSS_RC_OK); idx = nxt) {
        if ((rc = vtss_ip2_chip_counters_vlan_clear(nxt)) != VTSS_OK) {
            break;
        }
    }
    IP2_CRIT_EXIT();

    return rc;
}

/**
 * Clear per IP interface's statistics.
 *
 * \param version (IN) - Specify the IP version (IPv4/IPv6) to clear counters.
 * \param ifidx (IN) - Specify the interface index to clear counters.
 * version MUST be either IPv4 or IPv6; ifidx MUST NOT be NULL.
 * The indexes used are IP version type (IPv4/IPv6) AND
 * interface type (VTSS_ID_IF_TYPE_VLAN/VTSS_ID_IF_TYPE_OS_ONLY) AND
 * valid interface id (vid from VTSS_ID_IF_TYPE_VLAN/ifno from VTSS_ID_IF_TYPE_OS_ONLY).
 * \note Only stacking master is allowed for this operation.
 *
 * \return VTSS_OK iff operation is done successfully.
 *
 */
vtss_rc vtss_ip2_stat_intf_cntr_clear(vtss_ip_type_t version, vtss_if_id_t *ifidx)
{
    if (!ifidx || !msg_switch_is_master()) {
        return VTSS_RC_ERROR;
    }

    IP2_CRIT_ENTER();
    if (vtss_ip2_os_stat_intf_cntr_clear(version, ifidx) != VTSS_OK) {
        IP2_CRIT_EXIT();
        T_D("Software IP-INTF counter Clear failure!");
        return VTSS_RC_ERROR;
    }

    IP2_CRIT_RETURN_RC(vtss_ip2_chip_counters_vlan_clear(ifidx->u.vlan));
}

/**
 * Clear system's ICMP statistics.
 *
 * \param version (IN) - Specify the IP version (IPv4/IPv6) to clear counters.
 * version MUST be either IPv4 or IPv6.
 * \note Only stacking master is allowed for this operation.
 *
 * \return TRUE iff operation is done successfully.
 *
 */
vtss_rc vtss_ip2_stat_icmp_cntr_clear(vtss_ip_type_t version)
{
    if (!msg_switch_is_master() ||
        ((version != VTSS_IP_TYPE_IPV4) && (version != VTSS_IP_TYPE_IPV6))) {
        return VTSS_RC_ERROR;
    }

    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(vtss_ip2_os_stat_icmp_cntr_clear(version));
}

/**
 * Return the counters of the matched ICMP message type.
 *
 * \param version (IN) - IP version used as input key.
 * \param icmp_msg (IN) - ICMP message type value used as input key.
 *  Zero is a valid index.
 *
 * \param entry (OUT) - the returned statistics from the IP stack.
 *
 * \note Only stacking master is allowed for this operation.
 *
 * \return VTSS_OK iff entry is found.
 *
 */
vtss_rc vtss_ip2_stat_imsg_cntr_get(vtss_ip_type_t              version,
                                    u32                         icmp_msg,
                                    vtss_ips_icmp_stat_t        *entry)
{
    if (!entry || !msg_switch_is_master()) {
        return FALSE;
    }

    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(vtss_ip2_os_stat_imsg_cntr_get(version, icmp_msg, entry));
}

vtss_rc vtss_ip2_stat_imsg_cntr_getfirst(vtss_ip_type_t         version,
                                         u32                    icmp_msg,
                                         vtss_ips_icmp_stat_t   *entry)
{
    if (!entry || !msg_switch_is_master()) {
        return FALSE;
    }

    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(vtss_ip2_os_stat_imsg_cntr_get_first(version, icmp_msg, entry));
}

vtss_rc vtss_ip2_stat_imsg_cntr_getnext(vtss_ip_type_t          version,
                                        u32                     icmp_msg,
                                        vtss_ips_icmp_stat_t    *entry)
{
    if (!entry || !msg_switch_is_master()) {
        return FALSE;
    }

    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(vtss_ip2_os_stat_imsg_cntr_get_next(version, icmp_msg, entry));
}

/**
 * Clear per ICMP type's statistics.
 *
 * \param version (IN) - Specify the IP version (IPv4/IPv6) to clear counters.
 * \param type (IN) - Specify the ICMP message type to clear counters.
 * version MUST be either IPv4 or IPv6; type MUST be valid ICMP message value (0 ~ 255).
 * \note Only stacking master is allowed for this operation.
 *
 * \return TRUE iff operation is done successfully.
 *
 */
vtss_rc vtss_ip2_stat_imsg_cntr_clear(vtss_ip_type_t version, u32 type)
{
    if (!msg_switch_is_master() ||
        ((version != VTSS_IP_TYPE_IPV4) && (version != VTSS_IP_TYPE_IPV6)) ||
        (type > 255)) {
        return VTSS_RC_ERROR;
    }

    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(vtss_ip2_os_stat_imsg_cntr_clear(version, type));
}


/* CLI/ICLI helpers -------------------------------------------------------- */
int vtss_ip2_if_print(vtss_ip2_cli_pr *pr, BOOL vlan_only,
                      vtss_if_status_type_t type)
{
#define BUF_SIZE 1024
#define MAX_IF 130
    u32 i;
    vtss_rc rc;
    int res = 0;
    BOOL first = TRUE;
    u32 start, if_cnt = 0, if_st_cnt = 0;
    char buf[BUF_SIZE];

    //Ahh, very inefficient... way too small stack size....
    vtss_if_status_t *status = VTSS_CALLOC(IP2_MAX_STATUS_OBJS,
                                           sizeof(vtss_if_status_t));

    if (status == NULL) {
        return -1;
    }

    rc = vtss_ip2_ifs_status_get(type, IP2_MAX_STATUS_OBJS, &if_st_cnt, status);
    if (rc != VTSS_RC_OK) {
        res = 0;
        goto DONE;
    }
    vtss_ip2_if_status_sort(if_st_cnt, status);


    for (i = 0; i < if_st_cnt; ++i) {
        vtss_if_id_t id;

        if (vlan_only && status[i].if_id.type != VTSS_ID_IF_TYPE_VLAN) {
            continue;
        }

        if (!first) {
            (void)(*pr)("\n");
        }

        start = i;
        if_cnt = 1;
        id = status[i].if_id;

        // count matching interfaces in a row
        while (i < if_st_cnt) {
            // Check if next IF is not equal to this IF
            if (!vtss_if_id_equal(&id, &(status[i + 1].if_id))) {
                break;
            }
            ++i, ++if_cnt;
        }

        res += vtss_ip2_if_status_to_txt(buf, BUF_SIZE,
                                         &status[start], if_cnt);
        (void)(*pr)("%s", buf);
        first = FALSE;
    }
#undef BUF_SIZE

DONE:
    if (status) {
        VTSS_FREE(status);
    }

    return res;
}

int vtss_ip2_if_brief_print(vtss_ip2_cli_pr *pr)
{
#define BUF_SIZE 128
#define MAX_IF 130
    vtss_rc rc;
    vtss_vid_t cur, vlan;

    (void)(*pr)("Vlan Address              Method   Status\n");
    (void)(*pr)("---- -------------------- -------- ------\n");

    for (cur = 0; vtss_ip2_if_id_next(cur, &vlan) == VTSS_RC_OK; cur = vlan) {
        vtss_if_status_t ipv4;
        vtss_if_status_t link;
        vtss_ip_conf_t conf;
        char if_addr[BUF_SIZE], state[BUF_SIZE], dhcp[BUF_SIZE];

        (void) vtss_ip2_ipv4_conf_get(vlan, &conf);
        if (conf.dhcpc) {
            (void) snprintf(dhcp, BUF_SIZE, "DHCP");
        } else {
            (void) snprintf(dhcp, BUF_SIZE, "Manual");
        }

        rc = vtss_ip2_if_status_get_first(VTSS_IF_STATUS_TYPE_IPV4, vlan, &ipv4);
        if (rc == VTSS_RC_OK) {
            (void) snprintf(if_addr, BUF_SIZE, VTSS_IPV4N_FORMAT,
                            VTSS_IPV4N_ARG(ipv4.u.ipv4.net));
        } else {
            continue;
        }

        rc = vtss_ip2_if_status_get_first(VTSS_IF_STATUS_TYPE_LINK, vlan, &link);
        if (rc == VTSS_RC_OK) {
            if (link.u.link.flags & VTSS_IF_LINK_FLAG_UP) {
                (void) snprintf(state, BUF_SIZE, "UP");
            } else {
                (void) snprintf(state, BUF_SIZE, "DOWN");
            }
        } else {
            (void) snprintf(state, BUF_SIZE, "DOWN");
        }

        (void)(*pr)("%4d %-20s %-8s %-6s\n", vlan, if_addr, dhcp, state);
    }

    return 0;
#undef BUF_SIZE
}

int vtss_ip2_route_print(vtss_routing_entry_type_t  type,
                         vtss_ip2_cli_pr           *pr)
{
#define SIZE 2048
#define BUF_SIZE 128
    u32 i, cnt = 0, res = 0;
    vtss_rc rc = VTSS_RC_OK;
    vtss_routing_status_t *rts;
    char buf[BUF_SIZE];

    rts = VTSS_CALLOC(SIZE, sizeof(vtss_routing_status_t));

    if (rts == NULL) {
        return VTSS_RC_ERROR;
    }

#define P(...)                          \
    {                                   \
        int __r__ = (*pr)(__VA_ARGS__); \
        if (__r__ >= 0) {               \
            res += __r__;               \
        } else {                        \
            res = __r__;                \
            goto DONE;                  \
        }                               \
    }

    rc = vtss_ip2_route_get(type, SIZE, rts, &cnt);
    if (rc != VTSS_RC_OK) {
        goto DONE;
    }

    for (i = 0; i < cnt; ++i) {
        if (rts[i].rt.type == type) {
            vtss_routing_entry_t *rt = &rts[i].rt;
            if (rt->type == VTSS_ROUTING_ENTRY_TYPE_IPV4_UC) {
                // Net
                P(VTSS_IPV4N_FORMAT" via ", VTSS_IPV4N_ARG(rt->route.ipv4_uc.network));

                // via..
                if (rt->route.ipv4_uc.destination == 0) {
                    // Interface
                    (void)vtss_ip2_if_id_to_txt(buf, BUF_SIZE, &rts[i].interface);
                    P("%s", buf);
                } else {
                    // IP address
                    P(VTSS_IPV4_FORMAT, VTSS_IPV4_ARGS(rt->route.ipv4_uc.destination));
                }
            }  else if (rt->type == VTSS_ROUTING_ENTRY_TYPE_IPV6_UC) {
                if (vtss_ipv6_addr_is_link_local(&rt->route.ipv6_uc.network.address)) {
                    continue;
                }

                if (vtss_ipv6_addr_is_multicast(&rt->route.ipv6_uc.network.address)) {
                    continue;
                }

                // Net
                (void) misc_ipv6_txt(&rt->route.ipv6_uc.network.address, buf);
                P("%s/%d via ", buf, rt->route.ipv6_uc.network.prefix_size);

                // Destination
                if (vtss_ipv6_addr_is_zero(&rt->route.ipv6_uc.destination)) {
                    // Interface
                    (void)vtss_ip2_if_id_to_txt(buf, BUF_SIZE, &rts[i].interface);
                    P("%s", buf);
                } else {
                    // IP address
                    (void) misc_ipv6_txt(&rt->route.ipv6_uc.destination, buf);
                    P("%s", buf);
                }
            }

            (void)vtss_routing_flags_to_txt(buf, BUF_SIZE, rts[i].flags);
            P(" <%s>\n", buf);
        }
    }

DONE:
    VTSS_FREE(rts);
    return res;
#undef BUF_SIZE
#undef SIZE
#undef P
}

vtss_rc vtss_ip2_nb_clear(vtss_ip_type_t type)
{
    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(vtss_ip2_os_nb_clear(type));
}

vtss_rc vtss_ip2_nb_status_get(vtss_ip_type_t               type,
                               const u32                    max,
                               u32                         *cnt,
                               vtss_neighbour_status_t     *status)
{
    IP2_CRIT_ENTER();
    IP2_CRIT_RETURN_RC(vtss_ip2_os_nb_status_get(type, max, cnt, status));
}

int vtss_ip2_nb_print(vtss_ip_type_t type,
                      vtss_ip2_cli_pr *pr)
{
#define BUF 256
#define ARP_MAX 1024
    vtss_rc rc;
    char buf[BUF];
    u32 cnt, i, s = 0;
    vtss_neighbour_status_t *status;

    status = VTSS_CALLOC(ARP_MAX, sizeof(vtss_neighbour_status_t));
    if (!status) {
        T_W("Alloc error");
        return 0;
    }

    rc = vtss_ip2_nb_status_get(type, ARP_MAX, &cnt, status);
    if (rc != VTSS_RC_OK) {
        s += (*pr)("Failed to get neighbour cache\n");
        VTSS_FREE(status);
        return s;
    }

    for (i = 0; i < cnt; ++i) {
        if (vtss_ip2_neighbour_status_to_txt(buf, BUF, &status[i])) {
            s += (*pr)("%s\n", buf);
        }
    }

#undef ARP_MAX
#undef BUF
    VTSS_FREE(status);
    return s;
}

/* Module initialization --------------------------------------------------- */
vtss_rc vtss_ip2_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
    if (data->cmd == INIT_CMD_INIT) {
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, VTSS_TRACE_IP2_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x",
        data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_I("INIT");
        (void) vtss_avl_tree_init(&IP2_route_db);
        critd_init(&ip2_crit, "ip2.crit",
                   VTSS_MODULE_ID_IP2, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        IP2_CRIT_EXIT();

        (void)vtss_ip2_os_init();
        (void)vtss_ip2_chip_init();
#ifdef VTSS_SW_OPTION_VCLI
        vtss_ip2_cli_init();
        vtss_ip2_misc_cli_init();
#endif
        IP2_msg_pool = msg_buf_pool_create(VTSS_MODULE_ID_IP2, "IP2",
                                           IP2_MSG_BUFS, sizeof(ip2_msg_t));
        cyg_flag_init(&ip2_control_flags);
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          ip2_thread,
                          0,
                          "IP2.main",
                          ip2_thread_stack,
                          sizeof(ip2_thread_stack),
                          &ip2_thread_handle,
                          &ip2_thread_block);

        break;

    case INIT_CMD_START: {
        T_I("START");
        /* Register for stack messages */
        stack_register();

        /* XXX - GROSS CRUDE HACK - THIS MUST BE FIXED */
        extern void vtss_start_ips_tick_signal(void);
        extern void vtss_start_ips_netint_signal(int flag);
#ifdef VTSS_SW_OPTION_IPV6
        vtss_start_ips_netint_signal(1);
#else
        vtss_start_ips_netint_signal(0);
#endif /* VTSS_SW_OPTION_IPV6 */
        vtss_start_ips_tick_signal();

        (void)vtss_ip2_chip_start();

#ifdef VTSS_SW_OPTION_SNMP
        (void)vtss_ip2_snmp_init();
#endif
#ifdef VTSS_SW_OPTION_ICLI
        (void) vtss_ip2_ipv4_icfg_init();
#endif

        break;
    }

    case INIT_CMD_CONF_DEF:
        T_I("CONF_DEF, isid: %u", isid);
        if (isid == VTSS_ISID_GLOBAL) {
            // Reset global configuration (no local or per switch configuration here)
#if defined(VTSS_SW_OPTION_SILENT_UPGRADE)
            // "Keep IP" flag (data->flags & INIT_CMD_PARM2_FLAGS_IP == 0) is
            // consumed by the ICFG thread now; it used to be consumed here.
            IP2_CRIT_ENTER();
            IP2_flash_read(TRUE);
            IP2_CRIT_EXIT();
#else
            if ((data->flags & INIT_CMD_PARM2_FLAGS_IP)) {
                IP2_CRIT_ENTER();
                IP2_flash_read(TRUE);
                IP2_CRIT_EXIT();
            }
#endif
        }
        break;

    case INIT_CMD_MASTER_UP: {

        IP2_CRIT_ENTER();
        T_I("MASTER_UP");
        memset(&IP2_state, 0, sizeof(IP2_state));
        memset(IP2_state.vid2if, IFNO_INVALID, sizeof(IP2_state.vid2if));

        (void)conf_mgmt_mac_addr_get(IP2_state.master_mac.addr, 0);
        (void)vtss_ip2_chip_master_up(&IP2_state.master_mac);

        /* Read stack configuration */
        IP2_hello_pending_res = FALSE;
        vtss_ip2_routing_monitor_enable();
        IP2_flash_read(FALSE);
        memset(IP2_slave_active, 0, sizeof(IP2_slave_active));
        memset(&IP2_if_status_cache, 0, sizeof(IP2_if_status_cache));
        IP2_is_master = TRUE;
        T_I("MASTER_UP - completed");
        IP2_CRIT_EXIT();

        cyg_thread_resume(ip2_thread_handle);
        break;
    }

    case INIT_CMD_MASTER_DOWN:
        IP2_CRIT_ENTER();
        T_I("MASTER_DOWN");
        vtss_ip2_routing_monitor_disable();
        IP2_route_db_reset();
        (void)vtss_ip2_os_nb_clear(VTSS_IP_TYPE_IPV4);
        (void)vtss_ip2_os_nb_clear(VTSS_IP_TYPE_IPV6);
        IP2_delete_all_interfaces();
        (void)vtss_ip2_chip_master_down();
        memset(&IP2_if_status_cache, 0, sizeof(IP2_if_status_cache));

        IP2_is_master = FALSE;
        if (IP2_hello_pending_res) {
            (void) IP2_tx_hello_res();
            IP2_hello_pending_res = FALSE;
        }

        T_I("MASTER_DOWN - completed");
        IP2_CRIT_EXIT();
        break;

    case INIT_CMD_SWITCH_ADD:
        IP2_CRIT_ENTER();
        T_I("SWITCH_ADD, isid: %u", isid);
        (void)vtss_ip2_chip_switch_add(isid);
        if (!msg_switch_is_local(isid)) {
            (void)IP2_tx_hello_req(isid);
        } else {
            T_D("Do not add self!");
        }
        T_I("SWITCH_ADD, isid: %u - completed", isid);
        IP2_CRIT_EXIT();
        break;

    case INIT_CMD_SWITCH_DEL:
        IP2_CRIT_ENTER();
        T_I("SWITCH_DEL, isid: %u", isid);
        IP2_slave_active[isid] = FALSE;
        (void)vtss_ip2_chip_switch_del(isid);
        T_I("SWITCH_DEL, isid: %u - completed", isid);
        IP2_CRIT_EXIT();
        break;

    default:
        T_I("UNKNOWN CMD: %d, isid: %u", data->cmd, isid);
        break;
    }

    T_D("exit");
    return VTSS_OK;
}

