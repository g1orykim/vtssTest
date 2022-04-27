/*

 Vitesse API software.

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

/* NB: This is including API header files...? */
#include "vtss_options.h"
#include "vtss_os.h"

#if (defined(VTSS_SW_OPTION_L3RT) || defined(VTSS_ARCH_JAGUAR_1)) && \
    !defined(IP2_SOFTWARE_ROUTING)

#include "msg_api.h"
#include "port_api.h"
#include "critd_api.h"
#include "ip2_api.h"
#include "ip2_utils.h"
#include "ip2_trace.h"
#include "packet_api.h"
#include "ip2_chip_api.h"
#include "vtss_trace_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_IP2
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_IP2
#define __GRP VTSS_TRACE_IP2_GRP_CHIP
#define E(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_ERROR, _fmt, ##__VA_ARGS__)
#define W(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_WARNING, _fmt, ##__VA_ARGS__)
#define I(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_INFO, _fmt, ##__VA_ARGS__)
#define D(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_DEBUG, _fmt, ##__VA_ARGS__)
#define N(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_NOISE, _fmt, ##__VA_ARGS__)
#define R(_fmt, ...) T(__GRP, VTSS_TRACE_LVL_RACKET, _fmt, ##__VA_ARGS__)

static critd_t crit;

#if VTSS_TRACE_ENABLED
#  define IP2_CRIT_ENTER()          \
      critd_enter(&crit,                   \
                  VTSS_TRACE_IP2_GRP_CRIT, \
                  VTSS_TRACE_LVL_NOISE,    \
                  __FILE__, __LINE__)

#  define IP2_CRIT_EXIT()          \
      critd_exit(&crit,                   \
                 VTSS_TRACE_IP2_GRP_CRIT, \
                 VTSS_TRACE_LVL_NOISE,    \
                 __FILE__, __LINE__)

#  define IP2_CRIT_ASSERT_LOCKED()          \
      critd_assert_locked(&crit,                   \
                          VTSS_TRACE_IP2_GRP_CRIT, \
                          __FILE__, __LINE__)

#else
// Leave out function and line arguments
#  define IP2_CRIT_ENTER() critd_enter(&crit)
#  define IP2_CRIT_EXIT() critd_exit(&crit)
#  define IP2_CRIT_ASSERT_LOCKED() critd_assert_locked(&crit)
#endif

#define IP2_CRIT_RETURN(T, X) \
do {                          \
    T __val = (X);            \
    IP2_CRIT_EXIT();          \
    return __val;             \
} while(0)

#define IP2_CRIT_RETURN_RC(X)   \
    IP2_CRIT_RETURN(vtss_rc, X)


#define LOCAL_OR_GLOBAL(ISID) \
    msg_switch_is_local(isid) || (isid == VTSS_ISID_GLOBAL)

#define R_STACK_MASTER_FUNCTION_INIT(ISID_)                   \
    N("Enter %s ISID: %u", __FUNCTION__, ISID_);              \
    if (!IP2_is_master) {                                     \
        return IP2_ERROR_NOT_MASTER;                          \
    }

#define R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(ISID, ID, DATA)        \
    IP2_CRIT_ASSERT_LOCKED();                                            \
    if (isid == VTSS_ISID_GLOBAL) {                                      \
        int __SIT;                                                       \
        ip2_msg_req_t *__MSG;                                            \
        int cnt = IP2_slave_active_cnt();                                \
        if (!cnt) {                                                      \
            return VTSS_RC_OK;                                           \
        }                                                                \
        /*lint --e{69} */                                                \
        __MSG = IP2_msg_req_alloc(ID, cnt);                              \
        if (__MSG == NULL) {                                             \
            E("Alloc error!");                                           \
            return VTSS_RC_ERROR;                                        \
        }                                                                \
        __MSG->u = (union ip2_msg_req_u) DATA;                           \
                                                                         \
        for (__SIT = VTSS_ISID_START; __SIT <= VTSS_ISID_CNT; ++__SIT) { \
            if (IP2_slave_active[__SIT] &&                               \
                !msg_switch_is_local(__SIT)) {                           \
                IP2_msg_tx(__MSG, __SIT, sizeof(DATA));                  \
            }                                                            \
        }                                                                \
    } else {                                                             \
        /*lint --e{69} */                                                \
        ip2_msg_req_t *__MSG;                                            \
        __MSG = IP2_msg_req_alloc(ID, 1);                                \
        if (__MSG == NULL) {                                             \
            E("Alloc error!");                                           \
            return VTSS_RC_ERROR;                                        \
        }                                                                \
        __MSG->u = (union ip2_msg_req_u) DATA;                           \
        IP2_msg_tx(__MSG, ISID, sizeof(DATA));                           \
    }

#define R_CHECK(expr, M, ...)                                 \
{                                                             \
    vtss_rc __rc__ = (expr);                                  \
    if (__rc__ != VTSS_RC_OK) {                               \
        E("Check failed: " #expr " ARGS: " M, ##__VA_ARGS__); \
        return __rc__;                                        \
    }                                                         \
}

/*lint --e{123} */
#define R_CHECK_FMT(expr, L, expr_fmt, arg)                   \
{                                                             \
    vtss_rc __rc__ = (expr);                                  \
    if (__rc__ != VTSS_RC_OK) {                               \
        char buf[128];                                        \
        (void)expr_fmt(buf, 128, arg);                        \
        L("Check failed: " #expr " ARGS: " #arg "=%s", buf);  \
        return __rc__;                                        \
    }                                                         \
}

#define DO(expr)                 \
{                                \
    if (rc == VTSS_RC_OK) {      \
        rc = (expr);             \
        if (rc != VTSS_RC_OK) {  \
            E("Failed: " #expr); \
        }                        \
    }                            \
}

#define RC_UPDATE(expr)          \
    {                            \
        vtss_rc lrc = (expr);    \
        if (lrc != VTSS_OK) {    \
            E("Failed: " #expr); \
            rc = lrc;            \
        }                        \
    }

#define PRINTF(...)                                         \
    if (size - s > 0) {                                     \
        int res = snprintf(buf + s, size - s, __VA_ARGS__); \
        if (res >0 ) {                                      \
            s += res;                                       \
        }                                                   \
    }

#define PRINTFUNC(F, ...)                       \
    if (size - s > 0) {                         \
        s += F(buf + s, size - s, __VA_ARGS__); \
    }


enum {
    IP2_ERROR_ISID = MODULE_ERROR_START(VTSS_MODULE_ID_IP2_CHIP),
    IP2_ERROR_INVALID_RLEG,
    IP2_ERROR_INVALID_NOADD,
    IP2_ERROR_PORT,
    IP2_ERROR_SLAVE,
    IP2_ERROR_VALUE,
};

static vtss_mac_t base_mac;

#define IP2_CONF_VERSION  1

/* Keep track on when slaves are added, and when they say hello */
static BOOL           IP2_is_master = FALSE;
static int            IP2_master_id = -1;
static int            IP2_pending_master_id = -1;
static BOOL           IP2_slave_active[VTSS_ISID_CNT + 1];
static BOOL           IP2_hello_pending_res = FALSE;

/******************************************************************************
 * Counter definitions
 *****************************************************************************/
#define IP2_CT_TIMEOUT 5000 /* Counter request timeout (mS) */
#define IP2_CT_TTL     1000 /* Counter request time to live (mS) */

// maps between ip2_counters_t.rleg[idx] and vlan
// Used on local node (slave or master)
static vtss_vid_t IP2_vlan_index[VTSS_RLEG_CNT];

/* Combined counters */
typedef struct {
    vtss_vid_t         vlan;
    vtss_l3_counters_t counters;
} ip2_counter_t;

typedef struct {
    ip2_counter_t      rleg[VTSS_RLEG_CNT];
} ip2_counters_t;

#if defined(VTSS_SW_OPTION_L3RT)
static ip2_counters_t IP2_counters_stack;
static u32            IP2_counters_seq_nr; /* latest seq_nr transmitted in CT_REQ */
static vtss_mtimer_t  IP2_counters_timer;  /* Counters expired timer */
static cyg_flag_t     IP2_counters_flag;   /* Counters event flag */
#endif /* VTSS_SW_OPTION_L3RT) */

/******************************************************************************
 * Message handling functions, structures, and state.
 *****************************************************************************/
#define IP2_MSG_REQ_BUFS 16 /* Number of msg buffers in request pool */
#define IP2_MSG_REP_BUFS  1 /* Number of msg buffers in reply pool */
#define IP2_MSG_VERSION   1 /* Current version of IP Routing messages */

/******************************************************************************
 * Message IDs
 *****************************************************************************/
typedef enum {
    IP2_MSG_ID_HELLO_REQ, /* M->S Request all slaves to say hello */
    IP2_MSG_ID_HELLO_RES, /* S->M Slaves says hello */

    IP2_MSG_ID_FLUSH,     /* M->S Flush everything in the L3 API */
    IP2_MSG_ID_SYNC_DONE, /* M->S Synchronization done (only for debugging) */

    IP2_MSG_ID_COMMON,    /* M->S Common settings */

    IP2_MSG_ID_RL_ADD,    /* M->S Router leg add */
    IP2_MSG_ID_RL_DEL,    /* M->S Router leg del */

    IP2_MSG_ID_RT_ADD,    /* M->S Route add */
    IP2_MSG_ID_RT_DEL,    /* M->S Route delete */

    IP2_MSG_ID_NB_ADD,    /* M->S Neighbour add */
    IP2_MSG_ID_NB_DEL,    /* M->S Neighbour delete */

    IP2_MSG_ID_CT_CLR,    /* M->S Clear counters request */
    IP2_MSG_ID_CT_REQ,    /* M->S Get counters request */
    IP2_MSG_ID_CT_REP,    /* S->M Get counters reply */

    IP2_MSG_ID_MAC_SUB,   /* M->S Add entry in mac table */
    IP2_MSG_ID_MAC_UNSUB, /* M->S Del entry in mac table */
} ip2_chip_msg_id_t;

/******************************************************************************
 * Message Identification Header
 *****************************************************************************/
typedef struct {
    // Message Version Number
    u32 version; // Set to IP2_MSG_VERSION

    // Message ID
    ip2_chip_msg_id_t msg_id;
} ip2_msg_hdr_t;

/******************************************************************************
 * Private functions
 *****************************************************************************/
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static char *IP2_msg_id_to_str(ip2_chip_msg_id_t msg_id)
{
#define CASE(X) case X: return #X
    switch (msg_id) {
        CASE(IP2_MSG_ID_HELLO_REQ);
        CASE(IP2_MSG_ID_HELLO_RES);
        CASE(IP2_MSG_ID_FLUSH);
        CASE(IP2_MSG_ID_RL_ADD);
        CASE(IP2_MSG_ID_RL_DEL);
        CASE(IP2_MSG_ID_RT_ADD);
        CASE(IP2_MSG_ID_RT_DEL);
        CASE(IP2_MSG_ID_NB_ADD);
        CASE(IP2_MSG_ID_NB_DEL);
        CASE(IP2_MSG_ID_SYNC_DONE);
        CASE(IP2_MSG_ID_CT_CLR);
        CASE(IP2_MSG_ID_CT_REQ);
        CASE(IP2_MSG_ID_CT_REP);
        CASE(IP2_MSG_ID_MAC_SUB);
        CASE(IP2_MSG_ID_MAC_UNSUB);
    default:
        return "***Unknown Message ID***";
    }
#undef CASE
}
#endif /* VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG */

/******************************************************************************
 * Definition of messages.
 *****************************************************************************/
typedef struct {
    vtss_l3_common_conf_t entry; /* Common rleg configuration */
} ip2_msg_rleg_cset_t;

/* Router leg to add, del */
typedef struct {
    vtss_l3_rleg_conf_t        entry;
} ip2_msg_rl_t;

/* Mac address subscribe/unsubscribe */
typedef struct {
    vtss_mac_t mac;
    vtss_vid_t vlan;
} ip2_msg_mac_t;

/* Router entry to add or delete */
typedef struct {
    vtss_routing_entry_t       entry;
} ip2_msg_rt_t;

/* Neighbour entry to add or delete */
typedef struct {
    vtss_l3_neighbour_t        entry;
} ip2_msg_nb_t;

/* Sequence number to be returned in the reply */
typedef struct {
    u32                        seq_nr;
} ip2_msg_ct_req_t;

typedef struct {
    vtss_vid_t                 vlan;
} ip2_msg_ct_clr_t;

typedef struct {
    vtss_rc            rc;     /* Return code from vtss_l3_rleg_counters_get */
    u32                seq_nr; /* Sequence number to match with request */
    ip2_counter_t      rleg_cnt[VTSS_RLEG_CNT]; /* Rleg counters */
} ip2_msg_ct_rep_t;

/******************************************************************************
 * Message request.
 * This struct contains a union, whose primary purpose is to give the
 * size of the biggest of the contained structures.
 *****************************************************************************/
union ip2_msg_req_u {
    ip2_msg_rleg_cset_t common;
    ip2_msg_rl_t        rl;
    ip2_msg_mac_t       mac;
    ip2_msg_rt_t        rt;
    ip2_msg_nb_t        nb;
    ip2_msg_ct_req_t    ct_req;
    ip2_msg_ct_clr_t    ct_clr;
};

typedef struct {
    // Header stuff
    ip2_msg_hdr_t hdr;

    // Request message (FLUSH, SYNC_DONE and CT_CLR uses only the header!)
    union ip2_msg_req_u u;
} ip2_msg_req_t;

/******************************************************************************
 * Message reply.
 * This struct contains a union, whose primary purpose is to give the
 * size of the biggest of the contained structures.
 *****************************************************************************/
typedef struct {
    // Header stuff
    ip2_msg_hdr_t hdr;

    // Reply message
    union {
        ip2_msg_ct_rep_t ct_rep;
    } u;
} ip2_msg_rep_t;

static int IP2_slave_active_cnt(void)
{
    int i, cnt = 0;
    for (i = VTSS_ISID_START; i <= VTSS_ISID_CNT; ++i) {
        if (IP2_slave_active[i] && !msg_switch_is_local(i)) {
            ++cnt;
        }
    }
    return cnt;
}

static vtss_rc IP2_counter_vlan_cache_add_dry(vtss_vid_t vlan)
{
    int i;
    int _free = -1;

    IP2_CRIT_ASSERT_LOCKED();

    for (i = 0; i < (int)VTSS_RLEG_CNT; i++) {
        if (IP2_vlan_index[i] == vlan) {
            return VTSS_RC_ERROR;
        }

        if (IP2_vlan_index[i] == 0) {
            _free = i;
        }
    }

    if (_free >= 0) {
        return VTSS_RC_OK;
    }

    return VTSS_RC_ERROR;
}

static vtss_rc IP2_counter_vlan_cache_add(vtss_vid_t vlan)
{
#if defined(VTSS_SW_OPTION_L3RT)
    int i;
    int _free = -1;

    IP2_CRIT_ASSERT_LOCKED();

    for (i = 0; i < (int)VTSS_RLEG_CNT; i++) {
        if (IP2_vlan_index[i] == vlan) {
            return VTSS_RC_ERROR;
        }

        if (IP2_vlan_index[i] == 0) {
            _free = i;
        }
    }

    if (_free >= 0) {
        IP2_vlan_index[_free] = vlan;
        memset(&IP2_counters_stack.rleg[_free], 0, sizeof(ip2_counter_t));
        return VTSS_RC_OK;
    }

    return VTSS_RC_ERROR;
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_L3RT) */
}

static vtss_rc IP2_counter_vlan_cache_del_dry(vtss_vid_t vlan)
{
#if defined(VTSS_SW_OPTION_L3RT)
    int i;

    IP2_CRIT_ASSERT_LOCKED();

    for (i = 0; i < (int)VTSS_RLEG_CNT; i++) {
        if (IP2_vlan_index[i] == vlan) {
            return VTSS_RC_OK;
        }
    }

    return VTSS_RC_ERROR;
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_L3RT */
}

static vtss_rc IP2_counter_vlan_cache_del(vtss_vid_t vlan)
{
#if defined(VTSS_SW_OPTION_L3RT)
    int i;

    IP2_CRIT_ASSERT_LOCKED();

    for (i = 0; i < (int)VTSS_RLEG_CNT; i++) {
        if (IP2_vlan_index[i] == vlan) {
            IP2_vlan_index[i] = 0;
            memset(&IP2_counters_stack.rleg[i], 0, sizeof(ip2_counter_t));
            return VTSS_RC_OK;
        }
    }

    return VTSS_RC_ERROR;
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_L3RT) */
}

#if defined(VTSS_SW_OPTION_L3RT)
static vtss_rc IP2_counter_vlan_cache_idx(vtss_vid_t vlan, int *idx)
{
    int i;

    IP2_CRIT_ASSERT_LOCKED();
    for (i = 0; i < (int)VTSS_RLEG_CNT; i++) {
        if (IP2_vlan_index[i] == vlan) {
            *idx = i;
            return VTSS_RC_OK;
        }
    }
    return VTSS_RC_ERROR;
}
#endif /* VTSS_SW_OPTION_L3RT */

int vtss_l3_neighbour_to_txt(char                      *buf,
                             int                        size,
                             const vtss_l3_neighbour_t *const nb)
{
    int s = 0;
    PRINTF("{DMAC: "VTSS_MAC_FORMAT" VLAN: %u IP: ",
           VTSS_MAC_ARGS(nb->dmac), nb->vlan);
    PRINTFUNC(vtss_ip2_ip_addr_to_txt, &(nb->dip));
    PRINTF("}");
    return s;
}

#define MAX_MAC_SUBSCRIPTIONS 1024
static ip2_msg_mac_t mac_sub[MAX_MAC_SUBSCRIPTIONS];

int IP2_msg_req_to_txt(char *buf, int size, const ip2_msg_req_t *msg)
{
    int s = 0;

    PRINTF("{");
    switch (msg->hdr.msg_id) {
    case IP2_MSG_ID_FLUSH:
        PRINTF("Mode: %d, MAC: "VTSS_MAC_FORMAT,
               msg->u.common.entry.rleg_mode,
               VTSS_MAC_ARGS(msg->u.common.entry.base_address));
        break;

    case IP2_MSG_ID_RL_ADD:
    case IP2_MSG_ID_RL_DEL:
        PRINTF("IPv4-UC: %s IPv6-UC: %s, IPv4-ICMP: %s, IPv4-ICMP: %s, "
               "VLAN: %d",
               (msg->u.rl.entry.ipv4_unicast_enable ? "ENA" : "DIS"),
               (msg->u.rl.entry.ipv6_unicast_enable ? "ENA" : "DIS"),
               (msg->u.rl.entry.ipv4_icmp_redirect_enable ? "ENA" : "DIS"),
               (msg->u.rl.entry.ipv6_icmp_redirect_enable ? "ENA" : "DIS"),
               msg->u.rl.entry.vlan);
        break;

    case IP2_MSG_ID_RT_ADD:
    case IP2_MSG_ID_RT_DEL:
        PRINTFUNC(vtss_ip2_route_entry_to_txt, &(msg->u.rt.entry));
        break;

    case IP2_MSG_ID_NB_ADD:
    case IP2_MSG_ID_NB_DEL:
        PRINTF("MAC: "VTSS_MAC_FORMAT", Vlan: %u, DIP: ",
               VTSS_MAC_ARGS(msg->u.nb.entry.dmac),
               msg->u.nb.entry.vlan);
        PRINTFUNC(vtss_ip2_ip_addr_to_txt, &(msg->u.nb.entry.dip));
        break;

    case IP2_MSG_ID_MAC_UNSUB:
    case IP2_MSG_ID_MAC_SUB:
        PRINTF("MAC: "VTSS_MAC_FORMAT", Vlan: %u",
               VTSS_MAC_ARGS(msg->u.mac.mac),
               msg->u.mac.vlan);
        break;

    case IP2_MSG_ID_SYNC_DONE:
    case IP2_MSG_ID_HELLO_REQ:
    case IP2_MSG_ID_HELLO_RES:
    case IP2_MSG_ID_CT_CLR:
    case IP2_MSG_ID_CT_REQ:
    case IP2_MSG_ID_CT_REP:
        // No data used
        break;

    default:
        PRINTF("UNKNOWN");
    }
    PRINTF("}");

    return s;
}

/******************************************************************************
 * Msg related variables
 *****************************************************************************/
static void *msg_req_pool;  /* Request buffers */
#if defined(VTSS_SW_OPTION_L3RT)
static void *msg_rep_pool;  /* Reply buffer */
#endif

/* Blocks until a buffer is available, then takes and returns it. */
static ip2_msg_req_t *IP2_msg_req_alloc(ip2_chip_msg_id_t msg_id, u32 ref_cnt)
{
    ip2_msg_req_t *msg;

    if (ref_cnt == 0) {
        return NULL;
    }

    msg = msg_buf_pool_get(msg_req_pool);
    VTSS_ASSERT(msg);
    if (ref_cnt > 1) {
        msg_buf_pool_ref_cnt_set(msg, ref_cnt);
    }

    msg->hdr.version = IP2_MSG_VERSION;
    msg->hdr.msg_id = msg_id;
    return msg;
}

#if defined(VTSS_SW_OPTION_L3RT)
/* Blocks until a buffer is available, then takes and returns it. */
static ip2_msg_rep_t *IP2_msg_rep_alloc(ip2_chip_msg_id_t msg_id)
{
    ip2_msg_rep_t *msg = msg_buf_pool_get(msg_rep_pool);
    VTSS_ASSERT(msg);
    msg->hdr.version = IP2_MSG_VERSION;
    msg->hdr.msg_id = msg_id;
    return msg;
}
#endif /* VTSS_SW_OPTION_L3RT */

/* Called when message is successfully or unsuccessfully transmitted. */
static void IP2_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    // Release the message back to the message buffer pool
    (void)msg_buf_pool_put(msg);
}

static void IP2_msg_tx(void *msg, vtss_isid_t isid, size_t len)
{
    /* Avoid "Warning -- Constant value Boolean" Lint warning, due to the use
     * of MSG_TX_DATA_HDR_LEN_MAX() below */
    /*lint -save -e506 */

    char buf[128];

    (void)IP2_msg_req_to_txt(buf, 128, (ip2_msg_req_t *)msg);
    D("%s %u %s %s",
      __FUNCTION__,
      isid,
      IP2_msg_id_to_str(((ip2_msg_hdr_t *)msg)->msg_id),
      buf);

    msg_tx_adv(NULL,
               IP2_msg_tx_done,
               MSG_TX_OPT_DONT_FREE,
               VTSS_MODULE_ID_IP2_CHIP,
               isid,
               msg,
               len + MSG_TX_DATA_HDR_LEN_MAX(ip2_msg_req_t, u,
                                             ip2_msg_rep_t, u));
    /*lint -restore */
}

#if defined(VTSS_SW_OPTION_L3RT)
/* Transmits a messages containing all counters to master */
static vtss_rc IP2_tx_counters_build(ip2_msg_rep_t *msg_rep, u32 seq)
{
    unsigned i;
    vtss_rc rc = VTSS_RC_OK;
    IP2_CRIT_ASSERT_LOCKED();

    /* Get local rleg counters */
    for (i = 0; i < VTSS_RLEG_CNT; i++) {
        msg_rep->u.ct_rep.rleg_cnt[i].vlan = IP2_vlan_index[i];

        if (IP2_vlan_index[i] == 0) {
            continue;
        }

        RC_UPDATE(vtss_l3_counters_rleg_get(NULL, IP2_vlan_index[i],
                                            &msg_rep->u.ct_rep.rleg_cnt[i].counters));
    }
    msg_rep->u.ct_rep.rc = rc;
    msg_rep->u.ct_rep.seq_nr = seq;

    return rc;
}
#endif /* VTSS_SW_OPTION_L3RT */

#if defined(VTSS_SW_OPTION_L3RT)
/* Transmits a messages containing all counters to master */
static vtss_rc IP2_tx_counters(const ip2_msg_req_t *msg_req, vtss_isid_t isid)
{
    vtss_rc rc;
    ip2_msg_rep_t *msg_rep;
    IP2_CRIT_ASSERT_LOCKED();

    msg_rep = IP2_msg_rep_alloc(IP2_MSG_ID_CT_REP);
    if (msg_rep == 0) {
        return VTSS_RC_ERROR;
    }

    rc = IP2_tx_counters_build(msg_rep, msg_req->u.ct_req.seq_nr);

    /* Transmit the reply back to the master */
    I("%u counter tx", isid);
    IP2_msg_tx(msg_rep, isid, sizeof(ip2_msg_ct_rep_t));
    return rc;
}
#endif /* VTSS_SW_OPTION_L3RT */

#if defined(VTSS_SW_OPTION_L3RT)
/* Process the messages transmitted from IP2_tx_counters */
static vtss_rc IP2_rx_counters_process(const ip2_msg_rep_t *msg_rep,
                                       vtss_isid_t isid)
{
    unsigned i;
    IP2_CRIT_ASSERT_LOCKED();
    if (msg_rep->u.ct_rep.seq_nr != IP2_counters_seq_nr) {
        W("Wrong seq_nr in reply: Got: %u, expected: %u",
          msg_rep->u.ct_rep.seq_nr, IP2_counters_seq_nr);
        return IP2_ERROR_SLAVE;
    }

    if (msg_rep->u.ct_rep.rc != VTSS_OK) {
        W("Error from slave: %u (%s)", msg_rep->u.ct_rep.rc,
          error_txt(msg_rep->u.ct_rep.rc));
        return IP2_ERROR_SLAVE;
    }

#define UPDATE_CNT(__NAME__)                          \
    IP2_counters_stack.rleg[idx].counters.__NAME__ += \
        msg_rep->u.ct_rep.rleg_cnt[i].counters.__NAME__

    /* Update aggregated rleg counters */
    for (i = 0; i < VTSS_RLEG_CNT; i++) {
        int idx;

        if (msg_rep->u.ct_rep.rleg_cnt[i].vlan == 0) {
            continue;
        }

        if (IP2_counter_vlan_cache_idx(msg_rep->u.ct_rep.rleg_cnt[i].vlan,
                                       &idx) != VTSS_RC_OK) {
            E("Failed to find local index for vlan = %u, isid = %u",
              msg_rep->u.ct_rep.rleg_cnt[i].vlan, isid);
            continue;
        }

        UPDATE_CNT(ipv4uc_received_octets);
        UPDATE_CNT(ipv4uc_received_frames);
        UPDATE_CNT(ipv6uc_received_octets);
        UPDATE_CNT(ipv6uc_received_frames);
        UPDATE_CNT(ipv4uc_transmitted_octets);
        UPDATE_CNT(ipv4uc_transmitted_frames);
        UPDATE_CNT(ipv6uc_transmitted_octets);
        UPDATE_CNT(ipv6uc_transmitted_frames);
    }
#undef UPDATE_CNT

    I("Updated counter from %u", isid);
    return VTSS_OK;
}
#endif /* VTSS_SW_OPTION_L3RT */

#if defined(VTSS_SW_OPTION_L3RT)
/* Process the messages transmitted from IP2_tx_counters */
static vtss_rc IP2_rx_counters(const ip2_msg_rep_t *msg_rep, vtss_isid_t isid)
{
    vtss_rc rc = IP2_rx_counters_process(msg_rep, isid);
    cyg_flag_setbits(&IP2_counters_flag, 1 << isid);
    return rc;
}
#endif /* VTSS_SW_OPTION_L3RT */

#if defined(VTSS_SW_OPTION_L3RT)
/******************************************************************************
 * IP2_counters_check()
 * Check if the counters has expired the time to live.
 * If counters are expired then send a CT_REQ to all switches in the stack and wait for a CT_REP
 * from all of them (with timeout).
 *
 * Returns FALSE if not expired or reload was successful.
 * Returns TRUE if reload failed (missing response from one or more switches).
 **************************************************************************************************/
static BOOL IP2_counters_check(void)
{
    u32 seq;
    int cnt, sit;
    BOOL expired;
    ip2_msg_req_t *msg = 0;

    cyg_flag_value_t oflag = 0;
    cyg_flag_value_t iflag = 0;
    cyg_tick_count_t abstime = cyg_current_time() +
                               VTSS_OS_MSEC2TICK(IP2_CT_TIMEOUT);

    IP2_CRIT_ASSERT_LOCKED();
    expired = VTSS_MTIMER_TIMEOUT(&IP2_counters_timer);

    if (!expired) {
        return FALSE;
    }
    memset(&IP2_counters_stack, 0, sizeof(IP2_counters_stack));
    cnt = IP2_slave_active_cnt();
    seq = ++IP2_counters_seq_nr;

    I("Counters expired, sending CT_REQ to %u switches", cnt);
    if (cnt > 0) {
        msg = IP2_msg_req_alloc(IP2_MSG_ID_CT_REQ, cnt);
        if (msg == NULL) {
            E("Alloc error!");
            return TRUE;
        }
        msg->u.ct_req.seq_nr = seq;
        cyg_flag_maskbits(&IP2_counters_flag, 0);
    }

    for (sit = VTSS_ISID_START; sit <= VTSS_ISID_CNT; ++sit) {
        if (msg_switch_is_local(sit)) {
            I("Updating local counters: %u, seq: %u", sit, seq);
            ip2_msg_rep_t msg_rep;
            (void) IP2_tx_counters_build(&msg_rep, seq);
            (void) IP2_rx_counters_process(&msg_rep, sit);

        } else if (IP2_slave_active[sit]) {
            iflag |= (1 << sit);
            I("Request counters from %u, seq: %u", sit, seq);
            IP2_msg_tx(msg, sit, sizeof(ip2_msg_ct_req_t));

        }
    }
    /*lint --e{455} */
    /*lint --e{454} */
    IP2_CRIT_EXIT();
    oflag = cyg_flag_timed_wait(&IP2_counters_flag, iflag,
                                CYG_FLAG_WAITMODE_AND, abstime);
    IP2_CRIT_ENTER();

    if ((oflag & iflag) == iflag) {
        VTSS_MTIMER_START(&IP2_counters_timer, IP2_CT_TTL); /* Restart timer */
        return FALSE;

    } else {
        W("timeout, oflag: %x, iflag: %x", oflag, iflag);
        return TRUE;

    }
}
#endif /* VTSS_SW_OPTION_L3RT */

const char *ip2_chip_error_txt(vtss_rc rc)
{
    switch (rc) {
    case IP2_ERROR_ISID:
        return "Invalid Switch ID";
    case IP2_ERROR_INVALID_RLEG:
        return "Invalid router leg parameters";
    case IP2_ERROR_INVALID_NOADD:
        return "Unable to add more router legs";
    case IP2_ERROR_PORT:
        return "Invalid port number";
    case IP2_ERROR_SLAVE:
        return "Could not get data from slave switch";
    case IP2_ERROR_VALUE:
        return "Invalid value";
    default:
        return "";
    }
}

static inline
BOOL mac_equal(const vtss_mac_t *const a,
               const vtss_mac_t *const b)
{
    u32 i;

    for (i = 0; i < 6; ++i) {
        if (a->addr[i] != b->addr[i]) {
            return FALSE;
        }
    }

    return TRUE;
}

static inline
BOOL msg_mac_equal(const ip2_msg_mac_t *const a,
                   const ip2_msg_mac_t *const b)
{
    if (!mac_equal(&a->mac, &b->mac)) {
        return FALSE;
    }

    if (a->vlan != b->vlan) {
        return FALSE;
    }

    return TRUE;
}


static vtss_rc IP2_mac_table_del(const vtss_vid_mac_t *m)
{
    vtss_rc rc;

    rc = vtss_mac_table_del(NULL, m);
    if (rc != VTSS_RC_OK) {
        D("vtss_mac_table_del({%u, "VTSS_MAC_FORMAT"}) failed",
          m->vid, VTSS_MAC_ARGS(m->mac));
    }

    return rc;
}

static vtss_rc IP2_mac_table_add(const vtss_mac_table_entry_t *e)
{
    vtss_rc rc;

    rc = vtss_mac_table_add(NULL, e);
    if (rc != VTSS_RC_OK) {
        D("vtss_mac_table_add({%u, "VTSS_MAC_FORMAT", %d, %d}) failed",
          e->vid_mac.vid, VTSS_MAC_ARGS(e->vid_mac.mac), e->locked,
          e->copy_to_cpu);
    }

    return rc;
}

static vtss_rc IP2_mac_subscribe_impl(const ip2_msg_mac_t *mac)
{
    vtss_rc rc;
    BOOL found_one = FALSE;
    vtss_mac_table_entry_t entry;
    int i, match_cnt = 0, mac_sub_store_idx = 0;
    int retry_cnt = 0;

    for (i = 0; i < MAX_MAC_SUBSCRIPTIONS; ++i) {
        if (msg_mac_equal(&mac_sub[i], mac)) {
            match_cnt ++;
        }

        if (mac_sub[i].vlan != 0) {
            continue;
        }

        if (!found_one) {
            mac_sub_store_idx = i;
            mac_sub[i] = *mac;
            found_one = TRUE;
        }
    }

    if (!found_one) {
        W("No free mac entries in the cache");
        return VTSS_RC_ERROR;
    }

    if (match_cnt) {
        W("Mac entry allready exists: {%u, "VTSS_MAC_FORMAT"}",
          mac->vlan, VTSS_MAC_ARGS(mac->mac));
        return VTSS_RC_ERROR;
    }

    memset(&entry, 0x0, sizeof(vtss_mac_table_entry_t));
    entry.vid_mac.vid = mac->vlan;
    entry.vid_mac.mac = mac->mac;
    entry.locked = TRUE;
#if defined(VTSS_FEATURE_MAC_CPU_QUEUE)
    // Is this correct???
    // what about ipv6 broadcast??? or other broadcast addresses??
    entry.cpu_queue = (mac->mac.addr[0] == 0xff ?
                       PACKET_XTR_QU_BC : PACKET_XTR_QU_MGMT_MAC);
#endif /* VTSS_FEATURE_MAC_CPU_QUEUE */
    if (IP2_is_master) {
        entry.copy_to_cpu = TRUE;
    } else {
        entry.copy_to_cpu = FALSE;
    }

    // only multicast address are added this way
    port_iter_t pit;
    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL,
                          PORT_ITER_SORT_ORDER_IPORT,
                          PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        entry.destination[pit.iport] = TRUE;
    }

    D("%d MAC-ADD: %u " VTSS_MAC_FORMAT, retry_cnt, entry.vid_mac.vid,
      VTSS_MAC_ARGS(entry.vid_mac.mac));
    rc = IP2_mac_table_add(&entry);

    if (rc != VTSS_RC_OK) {
        E("IP2_mac_table_add({%u, "VTSS_MAC_FORMAT", %d, %d}) failed",
          entry.vid_mac.vid, VTSS_MAC_ARGS(entry.vid_mac.mac),
          entry.locked, entry.copy_to_cpu);
        mac_sub[mac_sub_store_idx].vlan = 0;
    }

    return rc;
}

static vtss_rc IP2_mac_unsubscribe_impl(const ip2_msg_mac_t *mac)
{
    int i;
    vtss_rc rc;
    int cnt = 0;
    vtss_vid_mac_t vid_mac;

    for (i = 0; i < MAX_MAC_SUBSCRIPTIONS; ++i) {
        if (msg_mac_equal(&mac_sub[i], mac)) {
            // mark as invalid
            mac_sub[i].vlan = 0;
            cnt ++;
        }
    }

    if (cnt != 1) {
        W("Found %d entries in the cache: {%u, "VTSS_MAC_FORMAT"}",
          cnt, mac->vlan, VTSS_MAC_ARGS(mac->mac));
    }

    vid_mac.vid = mac->vlan;
    vid_mac.mac = mac->mac;
    D("MAC-DEL: %u " VTSS_MAC_FORMAT, vid_mac.vid,
      VTSS_MAC_ARGS(vid_mac.mac));
    rc = IP2_mac_table_del(&vid_mac);

    if (rc != VTSS_RC_OK) {
        I("IP2_mac_table_del({%u, "VTSS_MAC_FORMAT"}) failed",
          vid_mac.vid, VTSS_MAC_ARGS(vid_mac.mac));
    }
    return rc;
}

static void IP2_mac_unsubscribe_all_impl(void)
{
    int i;
    vtss_rc rc;
    vtss_vid_mac_t vid_mac;

    for (i = 0; i < MAX_MAC_SUBSCRIPTIONS; ++i) {
        if (mac_sub[i].vlan == 0) {
            continue;
        }

        mac_sub[i].vlan = 0;
        vid_mac.vid = mac_sub[i].vlan;
        vid_mac.mac = mac_sub[i].mac;
        D("MAC-DEL: %u " VTSS_MAC_FORMAT, vid_mac.vid,
          VTSS_MAC_ARGS(vid_mac.mac));
        rc = IP2_mac_table_del(&vid_mac);
        if (rc != VTSS_RC_OK) {
            D("IP2_mac_table_del({%u, "VTSS_MAC_FORMAT"}) failed", vid_mac.vid,
              VTSS_MAC_ARGS(vid_mac.mac));
        }
    }
}

static vtss_rc IP2_flush(void)
{
    IP2_mac_unsubscribe_all_impl();
    (void)vtss_l3_flush(NULL);
    return VTSS_RC_OK;
}

static vtss_rc IP2_tx_hello_res(void)
{
    ip2_msg_req_t *msg;
    D("TX hello response");

    if (IP2_is_master) {
        return IP2_ERROR_NOT_MASTER;
    }

    msg = IP2_msg_req_alloc(IP2_MSG_ID_HELLO_RES, 1);
    if (msg == NULL) {
        E("Alloc error!");
        return VTSS_RC_ERROR;
    }

    IP2_msg_tx(msg, 0, 0);
    return VTSS_RC_OK;
}

static vtss_rc IP2_rleg_add_local(const vtss_l3_rleg_conf_t *const rl)
{
    R_CHECK(IP2_counter_vlan_cache_add_dry(rl->vlan),
            "vlan = %u", rl->vlan);
    R_CHECK(vtss_l3_rleg_add(NULL, rl),
            "rl={ipv4_uc=%d, ipv6_uc=%d, ipv4_icmp=%d, ipv6_icmp=%d, vlan=%u}",
            rl->ipv4_unicast_enable, rl->ipv6_unicast_enable,
            rl->ipv4_icmp_redirect_enable, rl->ipv6_icmp_redirect_enable,
            rl->vlan);
    R_CHECK(IP2_counter_vlan_cache_add(rl->vlan), "vlan = %u", rl->vlan);
    return VTSS_RC_OK;
}

static vtss_rc IP2_rleg_del_local(const vtss_vid_t vlan)
{
    R_CHECK(IP2_counter_vlan_cache_del_dry(vlan), "vlan = %u", vlan);
    R_CHECK(vtss_l3_rleg_del(NULL, vlan), "vlan=%u", vlan);
    R_CHECK(IP2_counter_vlan_cache_del(vlan), "vlan = %u", vlan);
    return VTSS_RC_OK;
}

vtss_rc IP2_new_master(void)
{
    IP2_master_id = IP2_pending_master_id;
    IP2_hello_pending_res = FALSE;
    memset(IP2_vlan_index, 0, sizeof(IP2_vlan_index));
    return IP2_tx_hello_res();
}

static vtss_rc IP2_sync_node(vtss_isid_t isid);
static vtss_rc IP2_process_msg(const ip2_msg_req_t *msg, vtss_isid_t isid)
{
#define SLAVE_CHECK()                                 \
    if (IP2_is_master) {                              \
        I("This msg is not expected on the master!"); \
        return IP2_ERROR_NOT_MASTER;                  \
    }

#define MASTER_CHECK()                                \
    if (!IP2_is_master) {                             \
        I("This msg is not expected on a slave!");    \
        return IP2_ERROR_NOT_MASTER;                  \
    }

    char buf[512];
    vtss_rc rc;

    (void)IP2_msg_req_to_txt(buf, 512, msg);
    D("Processing msg: %u %d %s %s",
      isid,
      msg->hdr.msg_id,
      IP2_msg_id_to_str(msg->hdr.msg_id),
      buf);

    switch (msg->hdr.msg_id) {
    case IP2_MSG_ID_FLUSH:
        SLAVE_CHECK();
        base_mac = msg->u.common.entry.base_address;
        (void)IP2_flush();
        rc = vtss_l3_common_set(NULL, &msg->u.common.entry);
        break;

    case IP2_MSG_ID_RL_ADD:
        SLAVE_CHECK();
        rc = IP2_rleg_add_local(&msg->u.rl.entry);
        break;

    case IP2_MSG_ID_RL_DEL:
        SLAVE_CHECK();
        rc = IP2_rleg_del_local(msg->u.rl.entry.vlan);
        break;

#if defined(VTSS_SW_OPTION_L3RT)
    case IP2_MSG_ID_RT_ADD:
        SLAVE_CHECK();
        rc = vtss_l3_route_add(NULL, &msg->u.rt.entry);
        break;

    case IP2_MSG_ID_RT_DEL:
        SLAVE_CHECK();
        rc = vtss_l3_route_del(NULL, &msg->u.rt.entry);
        break;

    case IP2_MSG_ID_NB_ADD:
        SLAVE_CHECK();
        rc = vtss_l3_neighbour_add(NULL, &msg->u.nb.entry);
        break;

    case IP2_MSG_ID_NB_DEL:
        SLAVE_CHECK();
        rc = vtss_l3_neighbour_del(NULL, &msg->u.nb.entry);
        break;
#endif /* VTSS_SW_OPTION_L3RT */

    case IP2_MSG_ID_COMMON:
        SLAVE_CHECK();
        rc = vtss_l3_common_set(NULL, &msg->u.common.entry);
        break;

    case IP2_MSG_ID_SYNC_DONE:
        SLAVE_CHECK();
        D("Sync done");
        rc = VTSS_RC_OK;
        break;

    case IP2_MSG_ID_HELLO_REQ:
        IP2_pending_master_id = isid;
        if (IP2_is_master) {
            D("Set pending hello flag");
            IP2_hello_pending_res = TRUE;
            rc = VTSS_RC_OK;
        } else {
            rc = IP2_new_master();
        }
        break;

    case IP2_MSG_ID_HELLO_RES:
        MASTER_CHECK();
        D("Slave %u says hello", isid);
        rc = IP2_sync_node(isid);
        break;

#if defined(VTSS_SW_OPTION_L3RT)
    case IP2_MSG_ID_CT_CLR:
        SLAVE_CHECK();
        rc = vtss_l3_counters_rleg_clear(NULL, msg->u.ct_clr.vlan);
        break;

    case IP2_MSG_ID_CT_REQ:
        // Applies both to master and slave
        rc = IP2_tx_counters(msg, isid);
        break;

    case IP2_MSG_ID_CT_REP:
        MASTER_CHECK();
        rc = IP2_rx_counters((const ip2_msg_rep_t *)msg, isid);
        break;
#endif /* VTSS_SW_OPTION_L3RT */

    case IP2_MSG_ID_MAC_SUB:
        SLAVE_CHECK();
        rc = IP2_mac_subscribe_impl(&msg->u.mac);
        break;

    case IP2_MSG_ID_MAC_UNSUB:
        SLAVE_CHECK();
        rc = IP2_mac_unsubscribe_impl(&msg->u.mac);
        break;

    default:
        rc = IP2_ERROR_VALUE;
        break;
    }

#undef SLAVE_CHECK
#undef MASTER_CHECK

    if (rc != VTSS_RC_OK) {
        E("Failed: %s %u %s", IP2_msg_id_to_str(msg->hdr.msg_id),
          msg->hdr.msg_id, buf);
    }

    return rc;
}

/* Process incomming stack messages */
/*lint -sem(IP2_msg_rx, thread_protected) */
static BOOL IP2_msg_rx(void *contxt,
                       const void *const rx_msg,
                       const size_t len,
                       const vtss_module_id_t modid,
                       const ulong isid)
{
    ip2_msg_req_t *msg = (ip2_msg_req_t *)rx_msg;

    IP2_CRIT_ENTER();

    // Check if we support this version of the message.
    // If not, print a warning and return.
    if (msg->hdr.version != IP2_MSG_VERSION) {
        W("Unsupported version of the message (%u)", msg->hdr.version);
        IP2_CRIT_RETURN(BOOL, TRUE);
    }

    N("msg_id: %d, %s, ver: %u, len: %zu, isid: %u",
      msg->hdr.msg_id, IP2_msg_id_to_str(msg->hdr.msg_id),
      msg->hdr.version, len, isid);

    if (IP2_is_master ||
        isid == IP2_master_id ||
        msg->hdr.msg_id == IP2_MSG_ID_HELLO_REQ) {
        (void)IP2_process_msg(msg, isid);
    } else {
        D("Skipping msg: %d %s master_status=%d master_id=%d",
          msg->hdr.msg_id, IP2_msg_id_to_str(msg->hdr.msg_id),
          IP2_is_master, IP2_master_id);
    }

    IP2_CRIT_RETURN(BOOL, TRUE);
}

static vtss_rc IP2_common_set(vtss_isid_t isid,
                              const vtss_l3_common_conf_t *const conf)
{
    ip2_chip_msg_id_t id = IP2_MSG_ID_COMMON;
    ip2_msg_rleg_cset_t msg;
    R_STACK_MASTER_FUNCTION_INIT(isid);

    D("%s", __FUNCTION__);

    if (LOCAL_OR_GLOBAL(isid)) {
        (void)vtss_l3_common_set(0, conf);
    }

    msg.entry = *conf;

    R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(isid, id, msg);
    return VTSS_RC_OK;
}


static vtss_rc IP2_rleg_add(vtss_isid_t isid,
                            const vtss_l3_rleg_conf_t *const rl)
{
    ip2_chip_msg_id_t id = IP2_MSG_ID_RL_ADD;
    ip2_msg_rl_t msg;
    R_STACK_MASTER_FUNCTION_INIT(isid);

    D("%s", __FUNCTION__);

    if (LOCAL_OR_GLOBAL(isid)) {
        (void)IP2_rleg_add_local(rl);
    }

    msg.entry = *rl;

    R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(isid, id, msg);
    return VTSS_RC_OK;
}

static vtss_rc IP2_rleg_del(vtss_isid_t isid,
                            const vtss_l3_rleg_conf_t *const rl)
{
    ip2_msg_rl_t msg;
    R_STACK_MASTER_FUNCTION_INIT(isid);

    if (LOCAL_OR_GLOBAL(isid)) {
        (void)IP2_rleg_del_local(rl->vlan);
    }
    msg.entry = *rl;

    R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(isid, IP2_MSG_ID_RL_DEL, msg);
    return VTSS_RC_OK;
}

#if defined(VTSS_SW_OPTION_L3RT)
static vtss_rc IP2_route_add(vtss_isid_t isid,
                             const vtss_routing_entry_t *const rt)
{
    ip2_chip_msg_id_t id = IP2_MSG_ID_RT_ADD;
    ip2_msg_rt_t msg;
    R_STACK_MASTER_FUNCTION_INIT(isid);

    if (LOCAL_OR_GLOBAL(isid)) {
        R_CHECK_FMT(vtss_l3_route_add(NULL, rt),
                    E, vtss_ip2_route_entry_to_txt, rt);
    }
    msg.entry = *rt;

    R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(isid, id, msg);
    return VTSS_RC_OK;
}
#endif /* VTSS_SW_OPTION_L3RT */

#if defined(VTSS_SW_OPTION_L3RT)
static vtss_rc IP2_route_del(vtss_isid_t isid,
                             const vtss_routing_entry_t *const rt)
{
    ip2_msg_rt_t msg;
    R_STACK_MASTER_FUNCTION_INIT(isid);

    if (LOCAL_OR_GLOBAL(isid)) {
        R_CHECK_FMT(vtss_l3_route_del(NULL, rt),
                    E, vtss_ip2_route_entry_to_txt, rt);
    }
    msg.entry = *rt;

    R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(isid, IP2_MSG_ID_RT_DEL, msg);
    return VTSS_RC_OK;
}
#endif /* VTSS_SW_OPTION_L3RT */

#if defined(VTSS_SW_OPTION_L3RT)
static vtss_rc IP2_neighbour_add(vtss_isid_t isid,
                                 const vtss_l3_neighbour_t *const nb)
{
    ip2_chip_msg_id_t id = IP2_MSG_ID_NB_ADD;
    ip2_msg_nb_t msg;
    R_STACK_MASTER_FUNCTION_INIT(isid);

    if (LOCAL_OR_GLOBAL(isid)) {
        R_CHECK_FMT(vtss_l3_neighbour_add(NULL, nb),
                    E, vtss_l3_neighbour_to_txt, nb);
    }
    msg.entry = *nb;

    R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(isid, id, msg);
    return VTSS_RC_OK;
}
#endif /* VTSS_SW_OPTION_L3RT */

#if defined(VTSS_SW_OPTION_L3RT)
static vtss_rc IP2_neighbour_del(vtss_isid_t isid,
                                 const vtss_l3_neighbour_t *const nb)
{
    ip2_msg_nb_t msg;
    R_STACK_MASTER_FUNCTION_INIT(isid);

    if (LOCAL_OR_GLOBAL(isid)) {
        R_CHECK_FMT(vtss_l3_neighbour_del(NULL, nb),
                    I, vtss_l3_neighbour_to_txt, nb);
    }
    msg.entry = *nb;

    R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(isid, IP2_MSG_ID_NB_DEL, msg);
    return VTSS_RC_OK;
}
#endif /* VTSS_SW_OPTION_L3RT */

static vtss_rc IP2_tx_hello_req(vtss_isid_t isid)
{
    ip2_msg_req_t *msg;

    D("TX hello request to %u", isid);
    if (!IP2_is_master) {
        return IP2_ERROR_NOT_MASTER;
    }

    msg = IP2_msg_req_alloc(IP2_MSG_ID_HELLO_REQ, 1);
    if (msg == NULL) {
        E("Alloc error!");
        return VTSS_RC_ERROR;
    }

    IP2_msg_tx(msg, isid, 0);
    return VTSS_RC_OK;
}

static vtss_rc IP2_tx_flush(vtss_isid_t isid)
{
    ip2_msg_req_t *msg;

    if (!IP2_is_master) {
        return IP2_ERROR_NOT_MASTER;
    }

    if (LOCAL_OR_GLOBAL(isid)) {
        return VTSS_RC_ERROR;
    }

    msg = IP2_msg_req_alloc(IP2_MSG_ID_FLUSH, 1);
    if (msg == NULL) {
        E("Alloc error!");
        return VTSS_RC_ERROR;
    }

    msg->u.common.entry.rleg_mode = VTSS_ROUTING_RLEG_MAC_MODE_SINGLE;
    msg->u.common.entry.base_address = base_mac;
    IP2_msg_tx(msg, isid, sizeof(ip2_msg_rleg_cset_t));
    return VTSS_RC_OK;
}

static vtss_rc IP2_sync_done(vtss_isid_t isid)
{
    ip2_msg_req_t *msg;

    if (!IP2_is_master) {
        return IP2_ERROR_NOT_MASTER;
    }

    if (LOCAL_OR_GLOBAL(isid)) {
        return VTSS_RC_ERROR;
    }

    msg = IP2_msg_req_alloc(IP2_MSG_ID_SYNC_DONE, 1);
    if (msg == NULL) {
        E("Alloc error!");
        return VTSS_RC_ERROR;
    }

    IP2_slave_active[isid] = TRUE;
    IP2_msg_tx(msg, isid, 0);
    return VTSS_RC_OK;
}

static vtss_rc IP2_mac_subscribe(vtss_isid_t isid,
                                 const vtss_mac_t *mac,
                                 const vtss_vid_t vlan)
{
    vtss_rc rc;
    ip2_chip_msg_id_t id = IP2_MSG_ID_MAC_SUB;
    ip2_msg_mac_t msg;

    D("%s", __FUNCTION__);
    R_STACK_MASTER_FUNCTION_INIT(isid);

    msg.mac = *mac;
    msg.vlan = vlan;

    if (LOCAL_OR_GLOBAL(isid)) {
        rc = IP2_mac_subscribe_impl(&msg);
        if (rc != VTSS_RC_OK) {
            E("IP2_mac_subscribe_impl failed: vid=%u, mac="VTSS_MAC_FORMAT,
              vlan, VTSS_MAC_ARGS(*mac));
            return rc;
        }
    }

    R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(isid, id, msg);
    return VTSS_RC_OK;
}

static vtss_rc IP2_mac_unsubscribe(vtss_isid_t isid,
                                   const vtss_mac_t *mac,
                                   const vtss_vid_t vlan)
{
    ip2_msg_mac_t msg;
    R_STACK_MASTER_FUNCTION_INIT(isid);

    msg.mac = *mac;
    msg.vlan = vlan;

    if (LOCAL_OR_GLOBAL(isid)) {
        (void) IP2_mac_unsubscribe_impl(&msg);
    }

    R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(isid, IP2_MSG_ID_MAC_UNSUB, msg);
    return VTSS_RC_OK;
}

#if defined(VTSS_SW_OPTION_L3RT)
static vtss_rc IP2_counters_vlan_clear(vtss_isid_t isid,
                                       const vtss_vid_t vlan)
{
    ip2_msg_ct_clr_t msg;
    R_STACK_MASTER_FUNCTION_INIT(isid);

    msg.vlan = vlan;

    if (LOCAL_OR_GLOBAL(isid)) {
        R_CHECK(vtss_l3_counters_rleg_clear(NULL, vlan), " vlan=%u", vlan);
    }

    R_STACK_MASTER_FUNCTION_FOR_ALL_NON_LOCAL(isid, IP2_MSG_ID_CT_CLR, msg);
    return VTSS_RC_OK;
}
#endif /* VTSS_SW_OPTION_L3RT */

static vtss_rc IP2_sync_node(vtss_isid_t isid)
{
    u32 i;
    void *buf;
    vtss_rc rc = VTSS_RC_OK;
    const u32 buf_size = MAX(MAX(sizeof(vtss_l3_rleg_conf_t) * VTSS_RLEG_CNT,
                                 sizeof(vtss_routing_entry_t) * VTSS_LPM_CNT),
                             sizeof(vtss_l3_neighbour_t) * VTSS_ARP_CNT);

    D("%s %u", __FUNCTION__, isid);

    if (!IP2_is_master) {
        return IP2_ERROR_NOT_MASTER;
    }

    if (LOCAL_OR_GLOBAL(isid)) {
        return VTSS_RC_ERROR;
    }

    buf = VTSS_MALLOC(buf_size);

    if (buf == NULL) {
        E("Alloc error, size: %u", buf_size);
        return VTSS_RC_ERROR;
    }

#define CHECK_(expr)            \
{                               \
    vtss_rc __rc__ = (expr);    \
    if (__rc__ != VTSS_RC_OK) { \
        E("Check failed");      \
        rc = __rc__;            \
        goto DONE;              \
    }                           \
}

    CHECK_(IP2_tx_flush(isid));
    {
        // Sync all routing legs
        u32 cnt = 0;

        vtss_l3_rleg_conf_t *_buf = (vtss_l3_rleg_conf_t *)buf;
        CHECK_(vtss_l3_rleg_get(0, &cnt, _buf));

        for (i = 0; i < cnt; ++i) {
            (void)IP2_rleg_add(isid, &(_buf[i]));
        }
    }

#if defined(VTSS_SW_OPTION_L3RT)
    {
        // Sync all routes
        u32 cnt = 0;
        vtss_routing_entry_t *_buf = (vtss_routing_entry_t *)buf;
        CHECK_(vtss_l3_route_get(0, &cnt, _buf));

        for (i = 0; i < cnt; ++i) {
            char xx[128];
            (void)vtss_ip2_route_entry_to_txt(xx, 128, &(_buf[i]));

            D("Sync-add-route: ");
            (void)IP2_route_add(isid, &(_buf[i]));
        }
    }
#endif /* VTSS_SW_OPTION_L3RT */

#if defined(VTSS_SW_OPTION_L3RT)
    {
        // Sync all neighbours
        u32 cnt = 0;
        vtss_l3_neighbour_t *_buf = (vtss_l3_neighbour_t *)buf;
        CHECK_(vtss_l3_neighbour_get(0, &cnt, _buf));

        for (i = 0; i < cnt; ++i) {
            (void)IP2_neighbour_add(isid, &(_buf[i]));
        }
    }
#endif /* VTSS_SW_OPTION_L3RT */

    {
        // Sync all mac entries
        for (i = 0; i < MAX_MAC_SUBSCRIPTIONS; ++i) {
            if (mac_sub[i].vlan == 0) {
                continue;
            }

            (void)IP2_mac_subscribe(isid, &mac_sub[i].mac, mac_sub[i].vlan);
        }
    }
    IP2_slave_active[isid] = TRUE;
    CHECK_(IP2_sync_done(isid));

DONE:
    VTSS_FREE(buf);
    return rc;
#undef CHECK_
}

static BOOL IP2_is_base_mac(const vtss_mac_t *mac)
{
    return (mac->addr[0] == base_mac.addr[0] &&
            mac->addr[1] == base_mac.addr[1] &&
            mac->addr[2] == base_mac.addr[2] &&
            mac->addr[3] == base_mac.addr[3] &&
            mac->addr[4] == base_mac.addr[4] &&
            mac->addr[5] == base_mac.addr[5]);
}

static BOOL IP2_is_broadcast_mac(const vtss_mac_t *mac)
{
    return (mac->addr[0] & 0x1);
}

vtss_rc vtss_ip2_chip_init(void)
{
    D("%s", __FUNCTION__);

    msg_req_pool = msg_buf_pool_create(VTSS_MODULE_ID_IP2_CHIP,
                                       "Request",
                                       IP2_MSG_REQ_BUFS, /* number of msg request buffers */
                                       sizeof(ip2_msg_req_t));

#if defined(VTSS_SW_OPTION_L3RT)
    msg_rep_pool = msg_buf_pool_create(VTSS_MODULE_ID_IP2_CHIP,
                                       "Reply",
                                       IP2_MSG_REP_BUFS, /* number of msg reply buffers */
                                       sizeof(ip2_msg_rep_t));
#endif

    critd_init(&crit,
               "ip.chip.crit",
               VTSS_MODULE_ID_IP2_CHIP,
               VTSS_TRACE_MODULE_ID,
               CRITD_TYPE_MUTEX);
    IP2_CRIT_RETURN_RC(VTSS_RC_OK);
}

vtss_rc vtss_ip2_chip_start(void)
{
    // Start up the message protocol
    msg_rx_filter_t filter;
    vtss_rc         rc;

    IP2_CRIT_ENTER();
    D("%s", __FUNCTION__);
    memset(&filter, 0, sizeof(filter));
    filter.cb = IP2_msg_rx;
    filter.modid = VTSS_MODULE_ID_IP2_CHIP;
    if ((rc = msg_rx_filter_register(&filter)) != VTSS_RC_OK) {
        E("Failed to register for ip routing messages (%s)", error_txt(rc));
        IP2_CRIT_RETURN_RC(VTSS_RC_ERROR);
    }

    IP2_CRIT_RETURN_RC(VTSS_RC_OK);
}

vtss_rc vtss_ip2_chip_master_up(const vtss_mac_t *const mac)
{
    vtss_rc rc = VTSS_RC_OK;

    IP2_CRIT_ENTER();
    D("%s "VTSS_MAC_FORMAT, __FUNCTION__, VTSS_MAC_ARGS(*mac));

    base_mac = *mac;
    vtss_l3_common_conf_t common;

    DO(vtss_l3_common_get(0, &common));
    common.rleg_mode = VTSS_ROUTING_RLEG_MAC_MODE_SINGLE;
    common.base_address = *mac;

    IP2_mac_unsubscribe_all_impl();
    IP2_hello_pending_res = FALSE;
    DO(vtss_l3_flush(0));
    DO(vtss_l3_common_set(0, &common));
    memset(IP2_slave_active, 0, sizeof(IP2_slave_active));
    memset(IP2_vlan_index, 0, sizeof(IP2_vlan_index));
#if defined(VTSS_SW_OPTION_L3RT)
    memset(&IP2_counters_stack, 0, sizeof(IP2_counters_stack));
#endif /* VTSS_SW_OPTION_L3RT */
    IP2_is_master = TRUE;

    IP2_CRIT_RETURN_RC(VTSS_RC_OK);
}

vtss_rc vtss_ip2_chip_master_down(void)
{
    vtss_rc rc = VTSS_RC_OK;

    IP2_CRIT_ENTER();
    D("%s", __FUNCTION__);
    (void)IP2_flush();
    IP2_is_master = FALSE;
    if (IP2_hello_pending_res) {
        rc = IP2_new_master();
    }
    IP2_CRIT_RETURN_RC(rc);
}

vtss_rc vtss_ip2_chip_switch_add(const vtss_isid_t id)
{
    vtss_rc rc = VTSS_RC_OK;
    IP2_CRIT_ENTER();
    D("%s %u", __FUNCTION__, id);
    if (!msg_switch_is_local(id)) {
        (void)IP2_tx_hello_req(id);
    } else {
        D("Do not add self!");
    }
    IP2_CRIT_RETURN_RC(rc);
}

vtss_rc vtss_ip2_chip_switch_del(const vtss_isid_t id)
{
    D("%s %u", __FUNCTION__, id);
    IP2_slave_active[id] = FALSE;
    return VTSS_RC_OK;
}

vtss_rc vtss_ip2_chip_routing_enable(BOOL enable)
{
    vtss_rc rc = VTSS_RC_OK;

    IP2_CRIT_ENTER();
    if (!IP2_is_master) {
        I("Only on master: %s", __FUNCTION__);
        IP2_CRIT_RETURN_RC(IP2_ERROR_NOT_MASTER);
    }

    D("%s(%s)", __FUNCTION__, enable ? "TRUE" : "FALSE");

    vtss_l3_common_conf_t common;
    DO(vtss_l3_common_get(0, &common));
    common.routing_enable = enable;

    IP2_CRIT_RETURN_RC(IP2_common_set(VTSS_ISID_GLOBAL, &common));
}

vtss_rc vtss_ip2_chip_rleg_add(const vtss_vid_t vlan)
{
    IP2_CRIT_ENTER();
    vtss_l3_rleg_conf_t conf = {
        .ipv4_unicast_enable = TRUE,
        .ipv6_unicast_enable = TRUE,
        .ipv4_icmp_redirect_enable = TRUE,
        .ipv6_icmp_redirect_enable = TRUE,
        .vlan = vlan
    };

    // this will be added as a router leg on all nodes
    IP2_CRIT_RETURN_RC(IP2_rleg_add(VTSS_ISID_GLOBAL, &conf));
}

vtss_rc vtss_ip2_chip_rleg_del(const vtss_vid_t vlan)
{
    IP2_CRIT_ENTER();

    vtss_l3_rleg_conf_t conf = {
        .ipv4_unicast_enable = FALSE,
        .ipv6_unicast_enable = FALSE,
        .ipv4_icmp_redirect_enable = FALSE,
        .ipv6_icmp_redirect_enable = FALSE,
        .vlan = vlan
    };

    // this will be added as a router leg on all nodes
    IP2_CRIT_RETURN_RC(IP2_rleg_del(VTSS_ISID_GLOBAL, &conf));
}

vtss_rc vtss_ip2_chip_mac_subscribe(const vtss_vid_t vlan,
                                    const vtss_mac_t *const mac)
{
    IP2_CRIT_ENTER();
    D("%s vlan: %u, "VTSS_MAC_FORMAT,
      __FUNCTION__, vlan, VTSS_MAC_ARGS(*mac));

    if (!IP2_is_master) {
        I("Only on master: %s", __FUNCTION__);
        IP2_CRIT_RETURN_RC(IP2_ERROR_NOT_MASTER);
    }

    if (IP2_is_base_mac(mac)) {
        I("Implicit done when calling vtss_ip2_chip_rleg_add");
        IP2_CRIT_RETURN_RC(VTSS_RC_OK);

    } else if (IP2_is_broadcast_mac(mac)) {
        IP2_CRIT_RETURN_RC(IP2_mac_subscribe(VTSS_ISID_GLOBAL, mac, vlan));

    } else {
        // not supported!
        E("Subscription of mac address "VTSS_MAC_FORMAT" "
          " at vlan %d is not supported",
          VTSS_MAC_ARGS(*mac), vlan);
        IP2_CRIT_RETURN_RC(VTSS_RC_ERROR);
    }
}

vtss_rc vtss_ip2_chip_mac_unsubscribe(const vtss_vid_t vlan,
                                      const vtss_mac_t *const mac)
{
    IP2_CRIT_ENTER();
    D("%s vlan: %u, "VTSS_MAC_FORMAT,
      __FUNCTION__, vlan, VTSS_MAC_ARGS(*mac));

    if (!IP2_is_master) {
        I("Only on master: %s", __FUNCTION__);
        IP2_CRIT_RETURN_RC(IP2_ERROR_NOT_MASTER);
    }

    if (IP2_is_base_mac(mac)) {
        I("Implicit done when calling vtss_ip2_chip_rleg_del");
        IP2_CRIT_RETURN_RC(VTSS_RC_OK);

    } else if (IP2_is_broadcast_mac(mac)) {
        IP2_CRIT_RETURN_RC(IP2_mac_unsubscribe(VTSS_ISID_GLOBAL, mac, vlan));

    } else {
        // not supported!
        IP2_CRIT_RETURN_RC(VTSS_RC_ERROR);
    }
}

vtss_rc vtss_ip2_chip_route_add(const vtss_routing_entry_t *const rt)
{
#if defined(VTSS_SW_OPTION_L3RT)
#  define BUF_SIZE 128
    char buf[BUF_SIZE];

    IP2_CRIT_ENTER();

    // TODO, only format if needed
    (void)vtss_ip2_route_entry_to_txt(buf, BUF_SIZE, rt);
    D("%s %s", __FUNCTION__, buf);

    if (!IP2_is_master) {
        I("Only on master: %s", __FUNCTION__);
        IP2_CRIT_RETURN_RC(IP2_ERROR_NOT_MASTER);
    }

    IP2_CRIT_RETURN_RC(IP2_route_add(VTSS_ISID_GLOBAL, rt));
#  undef BUF_SIZE
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_L3RT */
}

vtss_rc vtss_ip2_chip_route_del(const vtss_routing_entry_t *const rt)
{
#if defined(VTSS_SW_OPTION_L3RT)
#  define BUF_SIZE 128
    char buf[BUF_SIZE];

    IP2_CRIT_ENTER();

    // TODO, only format if needed
    (void)vtss_ip2_route_entry_to_txt(buf, BUF_SIZE, rt);
    D("%s %s", __FUNCTION__, buf);

    if (!IP2_is_master) {
        I("Only on master: %s", __FUNCTION__);
        IP2_CRIT_RETURN_RC(IP2_ERROR_NOT_MASTER);
    }

    IP2_CRIT_RETURN_RC(IP2_route_del(VTSS_ISID_GLOBAL, rt));
#  undef BUF_SIZE
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_L3RT */
}

vtss_rc vtss_ip2_chip_neighbour_add(const vtss_neighbour_t *const nb)
{
#if defined(VTSS_SW_OPTION_L3RT)
#  define BUF_SIZE 128
    char buf[BUF_SIZE];
    vtss_l3_neighbour_t _nb;

    IP2_CRIT_ENTER();

    // TODO, only format if needed
    (void)vtss_ip2_neighbour_to_txt(buf, BUF_SIZE, nb);
    D("%s %s", __FUNCTION__, buf);

    if (!IP2_is_master) {
        I("Only on master: %s", __FUNCTION__);
        IP2_CRIT_RETURN_RC(IP2_ERROR_NOT_MASTER);
    }

    _nb.dmac = nb->dmac;
    _nb.vlan = nb->vlan;
    _nb.dip = nb->dip;

    IP2_CRIT_RETURN_RC(IP2_neighbour_add(VTSS_ISID_GLOBAL, &_nb));
#  undef BUF_SIZE
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_L3RT */
}

vtss_rc vtss_ip2_chip_neighbour_del(const vtss_neighbour_t *const nb)
{
#if defined(VTSS_SW_OPTION_L3RT)
#  define BUF_SIZE 128
    char buf[BUF_SIZE];
    vtss_l3_neighbour_t _nb;

    IP2_CRIT_ENTER();

    // TODO, only format if needed
    (void)vtss_ip2_neighbour_to_txt(buf, BUF_SIZE, nb);
    D("%s %s", __FUNCTION__, buf);


    if (!IP2_is_master) {
        I("Only on master: %s", __FUNCTION__);
        IP2_CRIT_RETURN_RC(IP2_ERROR_NOT_MASTER);
    }

    _nb.dmac = nb->dmac;
    _nb.vlan = nb->vlan;
    _nb.dip = nb->dip;

    IP2_CRIT_RETURN_RC(IP2_neighbour_del(VTSS_ISID_GLOBAL, &_nb));
#  undef BUF_SIZE
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_L3RT */
}

vtss_rc vtss_ip2_chip_counters_vlan_get(const vtss_vid_t vlan,
                                        vtss_l3_counters_t *counters)
{
#if defined(VTSS_SW_OPTION_L3RT)
    int idx;

    IP2_CRIT_ENTER();

    if (!IP2_is_master) {
        I("only on master: %s", __FUNCTION__);
        IP2_CRIT_RETURN_RC(IP2_ERROR_NOT_MASTER);
    }

    if (IP2_counters_check()) {
        IP2_CRIT_RETURN_RC(VTSS_RC_ERROR);
    }

    if (IP2_counter_vlan_cache_idx(vlan, &idx) != VTSS_RC_OK) {
        IP2_CRIT_RETURN_RC(VTSS_RC_ERROR);
    }

    *counters = IP2_counters_stack.rleg[idx].counters;

    IP2_CRIT_RETURN_RC(VTSS_RC_OK);
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_L3RT */
}

vtss_rc vtss_ip2_chip_counters_vlan_clear(const vtss_vid_t vlan)
{
#if defined(VTSS_SW_OPTION_L3RT)
    vtss_rc rc;

    IP2_CRIT_ENTER();
    if (!IP2_is_master) {
        I("only on master: %s", __FUNCTION__);
        IP2_CRIT_RETURN_RC(IP2_ERROR_NOT_MASTER);
    }

    rc = IP2_counters_vlan_clear(VTSS_ISID_GLOBAL, vlan);
    memset(&IP2_counters_stack, 0, sizeof(IP2_counters_stack));
    VTSS_MTIMER_START(&IP2_counters_timer, IP2_CT_TTL); /* Restart timer */

    IP2_CRIT_RETURN_RC(rc);
#else
    return VTSS_RC_OK;
#endif /* VTSS_SW_OPTION_L3RT */
}

#endif /* defined(VTSS_SW_OPTION_L3RT) */
