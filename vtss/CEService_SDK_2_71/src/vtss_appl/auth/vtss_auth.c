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

/*
  The Auth module is responsible for authenticate a combination of a username
  and a password. The authentication can be done against a local "database", a
  RADIUS server or a TACACS+ server.

  In order to work across a stack the authentication is partitioned into a client
  and and a server part, which always communicates via the msg module.

  The server runs on the master and the clients runs on both master and slaves.

 */

#include "main.h"
#include "msg_api.h"
#include "conf_api.h"
#include "sysutil_api.h"
#include "misc_api.h"
#include "vtss_auth_api.h"
#include "critd_api.h"
#include "vtss_auth.h"
#include <netdb.h>
#ifdef VTSS_SW_OPTION_WEB
#include <cyg/athttpd/auth.h>
#endif

#ifdef VTSS_SW_OPTION_RADIUS
#include "vtss_radius_api.h"
/*
 * We don't want to use the following defines directly from the RADIUS module
 * as a change here would change the configuration size in flash.
 * Instead we #error if they do not match our expectations, and let the system
 * designer/integrator decide what to do.
 */
#if VTSS_RADIUS_NUMBER_OF_SERVERS != VTSS_AUTH_NUMBER_OF_SERVERS
#error VTSS_RADIUS_NUMBER_OF_SERVERS != VTSS_AUTH_NUMBER_OF_SERVERS
#endif
#if VTSS_RADIUS_HOST_LEN != VTSS_AUTH_HOST_LEN
#error VTSS_RADIUS_HOST_LEN != VTSS_AUTH_HOST_LEN
#endif
#if VTSS_RADIUS_KEY_LEN != VTSS_AUTH_KEY_LEN
#error VTSS_RADIUS_KEY_LEN != VTSS_AUTH_KEY_LEN
#endif

#endif

#ifdef VTSS_SW_OPTION_TACPLUS
#include <libtacplus.h>
#endif

#ifdef VTSS_SW_OPTION_USERS
#include "vtss_users_api.h"
#endif

#if defined(VTSS_SW_OPTION_CLI_TELNET) || defined(VTSS_AUTH_ENABLE_CONSOLE)
#include "cli_api.h"
#endif

#ifdef VTSS_SW_OPTION_SSH
#include "vtss_ssh_api.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "vtss_auth_icfg.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_AUTH

//#define VTSS_AUTH_CHECK_HEAP      1 /* check for loss of memory on the heap */

#if !defined(VTSS_AUTH_ATTRIBUTE_PACKED)
#define VTSS_AUTH_ATTRIBUTE_PACKED __attribute__((packed)) /* GCC defined */
#endif

#define VTSS_AUTH_CLIENT_TIMEOUT (60 * 60) /* seconds that client waits for reply from master */
#define VTSS_AUTH_RADIUS_ALLOC_MAX_RETRY 10

#define VTSS_UPTIME              ((ulong)(cyg_current_time() / CYGNUM_HAL_RTC_DENOMINATOR)) /* Uptime in seconds */

/* ================================================================= *
 *  Configuration
 * ================================================================= */

typedef struct {
    vtss_auth_global_host_conf_t global_conf;
    vtss_auth_host_conf_t        host_conf[VTSS_AUTH_NUMBER_OF_SERVERS];
} vtss_auth_proto_conf_t;

typedef struct {
    vtss_auth_radius_conf_t radius;                      /* RADIUS specific configuration */
    vtss_auth_proto_conf_t  proto[VTSS_AUTH_PROTO_LAST]; /* Protocol specific configuration */
    vtss_auth_agent_conf_t  agent[VTSS_AUTH_AGENT_LAST]; /* Agent configuration */
} vtss_auth_conf_t;

/* ================================================================= *
 *  Configuration block
 * ================================================================= */

#define VTSS_AUTH_CONF_VERSION 1

typedef struct {
    ulong            version; /* Block version */
    vtss_auth_conf_t config;  /* Configuration */
} vtss_auth_conf_blk_t;


/* ================================================================= *
 *  Auth message
 *  This is the messages that is sent beween the client and the server
 *  The client sends a request and waits for a reply.
 *  The server waits for a request, validates it and sends a reply.
 * ================================================================= */

#define VTSS_AUTH_MAX_USERNAME_LENGTH VTSS_SYS_STRING_LEN
#define VTSS_AUTH_MAX_PASSWORD_LENGTH VTSS_SYS_PASSWD_LEN

#define VTSS_AUTH_MSG_VERSION 1

typedef enum {
    VTSS_AUTH_MSG_ID_AUTH_REQUEST,
    VTSS_AUTH_MSG_ID_AUTH_REPLY,
    VTSS_AUTH_MSG_ID_AUTH_NEW_MASTER
} vtss_auth_msg_id_t;

typedef struct {
    ulong                  version;                                     /* msg version */
    vtss_auth_msg_id_t     msg_id;                                      /* request, reply or new_master */

    /* Data for request/reply only */
    struct {
        ulong                  seq_num;                                 /* used for matching replies with requests */
        ulong                  isid;                                    /* the isid of the client - only used internally in the server */
        vtss_auth_agent_t      agent;                                   /* The agent that is authenticating */
        char                   username[VTSS_AUTH_MAX_USERNAME_LENGTH];
        char                   password[VTSS_AUTH_MAX_PASSWORD_LENGTH];
        int                    userlevel;                               /* userlevel is only used in replies (set by the server) */
        vtss_rc                rc;                                      /* return code is only used in replies (set by the server) */
    } re;
} vtss_auth_msg_t;

/* ================================================================= *
 *  Auth message buffer and pool
 *  The pool contains an array of messages buffers.
 *  Instead of having two pools (a pool of free buffers and a pool of
 *  active buffers) all buffers are located in the same pool, and an
 *  in_use flag are used to show the current status of the buffer.
 * ================================================================= */
typedef struct {
    BOOL            in_use;  /* buffer free/in_use state */
    cyg_mutex_t     mutex;   /* buffer accesss protection */
    cyg_cond_t      cond;    /* wait here until we get a reply */
    vtss_auth_msg_t msg;     /* the message */
} vtss_auth_client_buf_t;

#define VTSS_AUTH_CLIENT_POOL_LENGTH 2
typedef struct {
    cyg_mutex_t            mutex;   /* pool accesss protection */
    cyg_cond_t             cond;    /* wait here until we get a buffer */
    ulong                  seq_num; /* Sequence number - incremented for each client request */
    vtss_auth_client_buf_t buf[VTSS_AUTH_CLIENT_POOL_LENGTH]; /* the buffers */
} vtss_auth_client_pool_t;


/* ================================================================= *
 *  Thread variables structure
 * ================================================================= */
typedef struct {
    cyg_handle_t            handle;
    cyg_thread              state;
    char                    stack[THREAD_DEFAULT_STACK_SIZE * 4];
    cyg_handle_t            mbox_handle;
    cyg_mbox                mbox;
} vtss_auth_thread_t;

#ifdef VTSS_SW_OPTION_RADIUS
#define VTSS_AUTH_RADIUS_VENDOR_VALUE_MAX_LENGTH 64
/* ================================================================= *
 *  RADIUS vendor specific data structure
 *  Used for decoding of vendor specific attributes
 *  See RFC 2865 section 5.26
 *  NOTE : Must be packed
 * ================================================================= */
typedef struct {
    u32 vendor_id;
    u8  vendor_type;
    u8  vendor_length;
    u8  vendor_value[VTSS_AUTH_RADIUS_VENDOR_VALUE_MAX_LENGTH];
} VTSS_AUTH_ATTRIBUTE_PACKED vtss_auth_radius_vendor_specific_data_t;

/* ================================================================= *
 *  RADIUS shared data structure
 *  Used for transferring data from vtss_auth_radius_rx_callback()
 *  to vtss_auth_radius()
 *  This is how the asynchronius RADIUS interface is converted to a
 *  synchronius interface
 * ================================================================= */
typedef struct {
    BOOL                             ready;   /* condition variable */
    cyg_mutex_t                      mutex;   /* shared data accesss protection */
    cyg_cond_t                       cond;    /* wait here until we get a reply */
    /* shared data follows here */
    u8                               handle;
    void                             *ctx;
    vtss_radius_access_codes_e       code;
    vtss_radius_rx_callback_result_e res;
    int                              userlevel;
} vtss_auth_radius_data_t;
#endif

/* ================================================================= *
 *  HTTPD authentication cache
 * ================================================================= */
#define VTSS_AUTH_HTTPD_CACHE_LIFE_TIME  60 /* seconds */
#define VTSS_AUTH_HTTPD_CACHE_SIZE       10 /* number of concurrent (different) usernames */
typedef struct {
    char  username[VTSS_AUTH_MAX_USERNAME_LENGTH];
    char  password[VTSS_AUTH_MAX_PASSWORD_LENGTH];
    int   userlevel;
    ulong expires; /* the time in seconds from boot that entry is valid */
} vtss_auth_httpd_cache_t;

/* ================================================================= *
 *  Global variables
 * ================================================================= */
/* IMPLEMENTATION REQUIRES LOWER CASE HERE !! */
const char *const vtss_auth_agent_names[VTSS_AUTH_AGENT_LAST] = {
    [VTSS_AUTH_AGENT_CONSOLE] = "console",
    [VTSS_AUTH_AGENT_TELNET]  = "telnet",
    [VTSS_AUTH_AGENT_SSH]     = "ssh",
    [VTSS_AUTH_AGENT_HTTP]    = "http",
};

/* IMPLEMENTATION REQUIRES LOWER CASE HERE !! */
const char *const vtss_auth_method_names[VTSS_AUTH_METHOD_LAST] = {
    [VTSS_AUTH_METHOD_NONE]   = "no",
    [VTSS_AUTH_METHOD_LOCAL]  = "local",
    [VTSS_AUTH_METHOD_RADIUS] = "radius",
    [VTSS_AUTH_METHOD_TACACS] = "tacacs",
};

const vtss_auth_agent_conf_t vtss_auth_agent_conf_default = {{VTSS_AUTH_METHOD_LOCAL}};

/* Use upper and or lower case. */
const char *const vtss_auth_proto_names[VTSS_AUTH_PROTO_LAST] = {
    [VTSS_AUTH_PROTO_RADIUS] = "RADIUS",
    [VTSS_AUTH_PROTO_TACACS] = "TACACS+",
};

/* ================================================================= *
 *  Private variables
 * ================================================================= */
static vtss_auth_thread_t      thread;                /* Thread specific stuff */
static critd_t                 crit_config;           /* Configuration critical region protection */
static critd_t                 crit_cache;            /* HTTPD cache critical region protection */
static vtss_auth_conf_t        config;                /* Current configuration */
static BOOL                    config_changed = TRUE; /* Configuration has changed */

static vtss_auth_client_pool_t pool;                  /* Pool for client messages */

#ifdef VTSS_SW_OPTION_RADIUS
static vtss_auth_radius_data_t radius_data;
#endif /* VTSS_SW_OPTION_RADIUS */

// Avoid httpd_cache not used (in configuration where Web module is not included)
/*lint --e{551} */
static vtss_auth_httpd_cache_t httpd_cache[VTSS_AUTH_HTTPD_CACHE_SIZE];

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "Auth",
    .descr     = "Authentication Module"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT_CONFIG] = {
        .name      = "crit_conf",
        .descr     = "Configuration critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT_CACHE] = {
        .name      = "crit_cache",
        .descr     = "HTTPD cache critical regions ",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
    }
};

#define AUTH_CRIT_CONFIG_ENTER() critd_enter(&crit_config, TRACE_GRP_CRIT_CONFIG, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define AUTH_CRIT_CONFIG_EXIT()  critd_exit( &crit_config, TRACE_GRP_CRIT_CONFIG, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define AUTH_CRIT_CACHE_ENTER()  critd_enter(&crit_cache,  TRACE_GRP_CRIT_CACHE,  VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define AUTH_CRIT_CACHE_EXIT()   critd_exit( &crit_cache,  TRACE_GRP_CRIT_CACHE,  VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define AUTH_CRIT_CONFIG_ENTER() critd_enter(&crit_config)
#define AUTH_CRIT_CONFIG_EXIT()  critd_exit( &crit_config)
#define AUTH_CRIT_CACHE_ENTER()  critd_enter(&crit_cache)
#define AUTH_CRIT_CACHE_EXIT()   critd_exit( &crit_cache)
#endif /* VTSS_TRACE_ENABLED */


/* ================================================================= *
 *
 * Local functions starts here
 *
 * ================================================================= */

/* ================================================================= *
 *  vtss_auth_client_pool_init()
 *  Initialize the pool and buffer management.
 * ================================================================= */
static void vtss_auth_client_pool_init(void)
{
    T_N("enter");
    memset(&pool, 0, sizeof(pool));
    cyg_mutex_init(&pool.mutex);
    cyg_cond_init(&pool.cond, &pool.mutex);
    T_N("exit");
}

/* ================================================================= *
 *  vtss_auth_client_buf_find_free()
 *  This is a helper function for vtss_auth_client_buf_alloc() only.
 *  Must be called with pool mutex locked!
 *  Returns a free buffer (or NULL)
 * ================================================================= */
static vtss_auth_client_buf_t *vtss_auth_client_buf_find_free(void)
{
    int i;

    T_N("enter");
    for (i = 0; i < VTSS_AUTH_CLIENT_POOL_LENGTH; i++) {
        if (pool.buf[i].in_use == FALSE) {
            T_N("exit - free buffer found");
            return &pool.buf[i];
        }
    }
    T_N("exit - no free buffer found");
    return NULL;
}

/* ================================================================= *
 *  vtss_auth_client_buf_alloc()
 *  Allocates a buffer for a client request.
 *  Blocks until a free buffer is available.
 * ================================================================= */
static vtss_auth_client_buf_t *vtss_auth_client_buf_alloc(vtss_auth_msg_id_t id)
{
    vtss_auth_client_buf_t *buf;

    T_N("enter");
    (void)cyg_mutex_lock(&pool.mutex);
    while ((buf = vtss_auth_client_buf_find_free()) == NULL) {
        T_N("waiting");
        (void) cyg_cond_wait(&pool.cond); /* Wait here for a free buffer */
    }
    buf->in_use = TRUE;

    /* Initialize buffer and part of message */
    cyg_mutex_init(&buf->mutex);
    cyg_cond_init(&buf->cond, &buf->mutex);
    buf->msg.version = VTSS_AUTH_MSG_VERSION;
    buf->msg.msg_id = id;
    buf->msg.re.seq_num = pool.seq_num++;

    (void)cyg_mutex_unlock(&pool.mutex);
    T_N("exit");
    return buf;
}

/* ================================================================= *
 *  vtss_auth_client_buf_free()
 *  Returns a buffer to the pool.
 * ================================================================= */
static void vtss_auth_client_buf_free(vtss_auth_client_buf_t *buf )
{
    T_N("enter");
    (void)cyg_mutex_lock(&pool.mutex);
    buf->in_use = FALSE;
    cyg_cond_signal(&pool.cond); /* Wake up clients that waits for a buffer */
    (void)cyg_mutex_unlock(&pool.mutex);
    T_N("exit");
}

/* ================================================================= *
 *  vtss_auth_client_buf_lookup()
 *  Search the client request buffers, for a matching sequence_number
 *  Returns the buffer or NULL if no matching buffer is found.
 * ================================================================= */
static vtss_auth_client_buf_t *vtss_auth_client_buf_lookup(ulong seq_num)
{
    vtss_auth_client_buf_t *buf = NULL;
    int i;

    T_N("enter");
    (void)cyg_mutex_lock(&pool.mutex);
    for (i = 0; i < VTSS_AUTH_CLIENT_POOL_LENGTH; i++) {
        if (pool.buf[i].in_use && (pool.buf[i].msg.re.seq_num == seq_num)) {
            buf = &pool.buf[i];
            T_N("match");
            break;
        }
    }
    (void)cyg_mutex_unlock(&pool.mutex);
    T_N("exit");
    return buf;
}

/* ================================================================= *
 *  vtss_auth_client_buf_unlock_all()
 *  Unlock all clients and ask them to retry the request.
 * ================================================================= */
static void vtss_auth_client_buf_unlock_all(void)
{
    vtss_auth_client_buf_t *buf;
    int i;

    T_N("enter");
    (void)cyg_mutex_lock(&pool.mutex);
    /* Unlock all waiting clients */
    for (i = 0; i < VTSS_AUTH_CLIENT_POOL_LENGTH; i++) {
        if (pool.buf[i].in_use) {
            buf = &pool.buf[i];
            (void)cyg_mutex_lock(&buf->mutex);
            buf->msg.re.rc = VTSS_AUTH_ERROR_CLIENT_MSG_TX_FAILED; /* Make the client retry */
            T_N("cyg_cond_signal");
            cyg_cond_signal(&buf->cond);
            (void)cyg_mutex_unlock(&buf->mutex);
        }
    }
    (void)cyg_mutex_unlock(&pool.mutex);
    T_N("exit");
}

/* ================================================================= *
 *  vtss_auth_new_master()
 *  Send new master notification to added switch
 * ================================================================= */
static void vtss_auth_new_master(vtss_isid_t isid)
{
    vtss_auth_msg_t *m = (vtss_auth_msg_t *)VTSS_MALLOC(sizeof(vtss_auth_msg_t));
    if (m) {
        m->version = VTSS_AUTH_MSG_VERSION;
        m->msg_id = VTSS_AUTH_MSG_ID_AUTH_NEW_MASTER;
        msg_tx(VTSS_MODULE_ID_AUTH, isid, m, MSG_TX_DATA_HDR_LEN(vtss_auth_msg_t, re));
    } else {
        T_W("out of memory");
    }
}

/* ================================================================= *
 *  vtss_auth_msg_rx()
 *  Stack message callback function
 * ================================================================= */
static BOOL vtss_auth_msg_rx(void *contxt, const void *const rx_msg, const size_t len,
                             const vtss_module_id_t modid, const ulong isid)
{
    vtss_auth_msg_t *msg = (vtss_auth_msg_t *)rx_msg;
    T_N("msg received: len: %d, modid: %d, isid: %d", (int)len, modid, (int)isid);

    // Check if we support this version of the message. If not, print a warning and return.
    if (msg->version != VTSS_AUTH_MSG_VERSION) {
        T_W("Unsupported version of the message (%u)", msg->version);
        return TRUE;
    }

    switch (msg->msg_id) {
    case VTSS_AUTH_MSG_ID_AUTH_REQUEST: {
        T_N("msg VTSS_AUTH_MSG_ID_AUTH_REQUEST received");
        if (msg_switch_is_master()) {
            vtss_auth_msg_t *m = (vtss_auth_msg_t *)VTSS_MALLOC(sizeof(vtss_auth_msg_t));
            if (m) {
                memcpy(m, msg, sizeof(vtss_auth_msg_t));   /* make a copy of the message, */
                m->re.isid = isid;                         /* remember where it came from */
                T_N("cyg_mbox_put");
                (void)cyg_mbox_put(thread.mbox_handle, m); /* hand it over to the auth_thread */
            } else {
                T_W("out of memory");
            }
        } else {
            T_W("server request received on a slave");
        }
        break;
    }
    case VTSS_AUTH_MSG_ID_AUTH_REPLY: {
        vtss_auth_client_buf_t *buf;

        T_N("msg VTSS_AUTH_MSG_ID_AUTH_REPLY received");
        buf = vtss_auth_client_buf_lookup(msg->re.seq_num);

        if (buf) { /* if a match is found then copy return parameters and unblock the client */
            (void) cyg_mutex_lock(&buf->mutex);
            buf->msg.re.userlevel = msg->re.userlevel;
            buf->msg.re.rc = msg->re.rc;
            T_N("cyg_cond_signal");
            cyg_cond_signal(&buf->cond);
            (void) cyg_mutex_unlock(&buf->mutex);
        }
        break;
    }
    case VTSS_AUTH_MSG_ID_AUTH_NEW_MASTER:
        vtss_auth_client_buf_unlock_all();
        break;
    default:
        T_W("Unknown message ID: %d", msg->msg_id);
        break;
    }
    return TRUE;
}

#ifdef VTSS_SW_OPTION_RADIUS
/* ================================================================= *
 *  vtss_auth_radius_init()
 *  Initialize RADIUS specific data
 * ================================================================= */
static void vtss_auth_radius_init(void)
{
    T_N("enter");
    memset(&radius_data, 0, sizeof(vtss_auth_radius_data_t));
    cyg_mutex_init(&radius_data.mutex);
    cyg_cond_init(&radius_data.cond, &radius_data.mutex);
    T_N("exit");
}

/* ================================================================= *
 *  vtss_auth_radius_config_update()
 *  Update RADIUS configuration
 * ================================================================= */
static vtss_rc vtss_auth_radius_config_update(const vtss_auth_radius_conf_t *rconf, const vtss_auth_proto_conf_t *conf)
{
    vtss_radius_cfg_s radius_cfg;
    vtss_rc rc = VTSS_OK;
    int i;

    T_N("enter");
    memset(&radius_cfg, 0, sizeof(radius_cfg));
    for (i = 0; i < VTSS_AUTH_NUMBER_OF_SERVERS; i++) {
        if (!conf->host_conf[i].host[0]) {
            break;
        }
        if (conf->host_conf[i].auth_port) {
            misc_strncpyz(radius_cfg.servers_auth[i].host, conf->host_conf[i].host, sizeof(radius_cfg.servers_auth[i].host));
            radius_cfg.servers_auth[i].port = conf->host_conf[i].auth_port;
            radius_cfg.servers_auth[i].timeout = conf->host_conf[i].timeout ? conf->host_conf[i].timeout : conf->global_conf.timeout;
            radius_cfg.servers_auth[i].retransmit = conf->host_conf[i].retransmit ? conf->host_conf[i].retransmit : conf->global_conf.retransmit;
            misc_strncpyz(radius_cfg.servers_auth[i].key, conf->host_conf[i].key[0] ? conf->host_conf[i].key : conf->global_conf.key, sizeof(radius_cfg.servers_auth[i].key));
        }
        if (conf->host_conf[i].acct_port) {
            misc_strncpyz(radius_cfg.servers_acct[i].host, conf->host_conf[i].host, sizeof(radius_cfg.servers_acct[i].host));
            radius_cfg.servers_acct[i].port = conf->host_conf[i].acct_port;
            radius_cfg.servers_acct[i].timeout = conf->host_conf[i].timeout ? conf->host_conf[i].timeout : conf->global_conf.timeout;
            radius_cfg.servers_acct[i].retransmit = conf->host_conf[i].retransmit ? conf->host_conf[i].retransmit : conf->global_conf.retransmit;
            misc_strncpyz(radius_cfg.servers_acct[i].key, conf->host_conf[i].key[0] ? conf->host_conf[i].key : conf->global_conf.key, sizeof(radius_cfg.servers_acct[i].key));
        }
    }
    radius_cfg.dead_time_secs = conf->global_conf.deadtime * 60;
    radius_cfg.nas_ip_address_enable = rconf->nas_ip_address_enable;
    radius_cfg.nas_ip_address = htonl(rconf->nas_ip_address);
    radius_cfg.nas_ipv6_address_enable = rconf->nas_ipv6_address_enable;
    radius_cfg.nas_ipv6_address = rconf->nas_ipv6_address;
    misc_strncpyz(radius_cfg.nas_identifier, rconf->nas_identifier, sizeof(radius_cfg.nas_identifier));

    if (msg_switch_is_master()) {
        rc = vtss_radius_cfg_set(&radius_cfg);
    }
    T_N("exit");
    return rc;
}

/* ================================================================= *
 *  vtss_auth_radius_rx_callback()
 *  Called when the RADIUS module receives a response
 * ================================================================= */
static void vtss_auth_radius_rx_callback(u8 handle, void *ctx, vtss_radius_access_codes_e code, vtss_radius_rx_callback_result_e res)
{
    T_N("enter");
    (void) cyg_mutex_lock(&radius_data.mutex);
    /* copy the data */
    radius_data.ready     = TRUE;
    radius_data.handle    = handle;
    radius_data.ctx       = ctx;
    radius_data.code      = code;
    radius_data.res       = res;
    radius_data.userlevel = 1;

    if ((res == VTSS_RADIUS_RX_CALLBACK_OK) && (code == VTSS_RADIUS_CODE_ACCESS_ACCEPT)) { // Check if the response contains a privilege level
        vtss_radius_attributes_e                       type;
        u8                                             len;
        const vtss_auth_radius_vendor_specific_data_t *val;
        BOOL                                           found = FALSE;
        BOOL                                           first = TRUE;

        while (vtss_radius_tlv_iterate(handle, &type, &len, (const u8 **)&val, first) == VTSS_OK) {
            // Next time, get the next TLV
            first = FALSE;
            if ((type == VTSS_RADIUS_ATTRIBUTE_VENDOR_SPECIFIC) && (len <= sizeof(vtss_auth_radius_vendor_specific_data_t))) {
                char buf[VTSS_AUTH_RADIUS_VENDOR_VALUE_MAX_LENGTH];
                if ((((ntohl(val->vendor_id) == 9) && (val->vendor_type == 1)) ||     // vendor_id 9 is Cisco and vendor_type 1 is cisco-avpair
                     ((ntohl(val->vendor_id) == 890) && (val->vendor_type == 3))) &&  // vendor_id 890 is Zyxel and vendor_type 3 is Zyxel-Privilege-avpair
                    ((val->vendor_length - 2) < (u8)sizeof(buf))) {  // Vendor length is within our limits and there is enough space for null termination
                    // Vendor length includes type and length so the length of value is length - 2
                    int i;
                    memset(buf, 0, sizeof(buf));
                    memcpy(buf, val->vendor_value, val->vendor_length - 2); // Make a modifiable null terminated copy of vendor value
                    for (i = 0; i < (int)strlen(buf); i++) {
                        buf[i] = tolower(buf[i]);
                    }
                    // vendor_value syntax: "shell:priv-lvl=x" where x is an integer from 0 to 15
                    T_D("vi %u, vt %u, vl %u, vv %s", ntohl(val->vendor_id), val->vendor_type, val->vendor_length, buf);
                    if (strstr(buf, "priv-lvl")) {
                        char *c = strrchr(buf, '=');
                        if (c) {
                            i = atoi(++c);                // fetch the number right after the last "="
                            if ((i >= 0) && (i <= 15)) {
                                radius_data.userlevel = i;
                                T_I("userlevel is set to %d", i);
                            } else {
                                T_W("userlevel is out of range %d", i);
                            }
                            found = TRUE;
                        } else {
                            T_W("missing \"=\" in %s", buf);
                        }
                    }
                }
            }
        }
        if (!found) {
            T_I("avpair \"priv-lvl=x\" not returned from RADIUS server");
        }
    }
    /* and wake him up */
    cyg_cond_signal(&radius_data.cond);
    (void) cyg_mutex_unlock(&radius_data.mutex);
    T_N("exit");
}

/* ================================================================= *
 *  vtss_auth_radius()
 *  Authenticate via RADIUS
 * ================================================================= */
static vtss_rc vtss_auth_radius(vtss_auth_agent_t agent, char *username, char *passwd, int *userlevel)
{
    int      i;
    vtss_rc  rc;
    u8       handle;

#ifdef VTSS_AUTH_CHECK_HEAP
    struct mallinfo m_before;
    struct mallinfo m_after;
    int m_loss;
    m_before = mallinfo();
#endif

    T_N("enter");

    for (i = 0; i < VTSS_AUTH_RADIUS_ALLOC_MAX_RETRY; i++) {
        // Allocate a RADIUS handle (i.e. a RADIUS ID).
        if ((rc = vtss_radius_alloc(&handle, VTSS_RADIUS_CODE_ACCESS_REQUEST)) == VTSS_OK) {
            T_N("got a RADIUS handle");
            break;
        }
        VTSS_OS_MSLEEP(100); // Allow some handlers to be returned to RADIUS
    }
    if (i == VTSS_AUTH_RADIUS_ALLOC_MAX_RETRY) {
        T_W("Got \"%s\" from vtss_radius_alloc()", vtss_radius_error_txt(rc));
        return VTSS_AUTH_ERROR_SERVER_TIMEOUT;
    }

    // Add the required attributes
    if (((rc = vtss_radius_tlv_set(handle, VTSS_RADIUS_ATTRIBUTE_USER_NAME,     strlen(username), (u8 *)username, TRUE)) != VTSS_OK) ||
        ((rc = vtss_radius_tlv_set(handle, VTSS_RADIUS_ATTRIBUTE_USER_PASSWORD, strlen(passwd),   (u8 *)passwd,   TRUE)) != VTSS_OK)) {
        T_W("Got \"%s\" from vtss_tlv_set() on required attribute", vtss_radius_error_txt(rc));
        return VTSS_AUTH_ERROR_GEN;
    }

    (void) cyg_mutex_lock(&radius_data.mutex);
    radius_data.ready = FALSE;

    // Transmit the RADIUS frame and ask to be called back whenever a response arrives.
    // The RADIUS module takes care of retransmitting, changing server, etc.
    if ((rc = vtss_radius_tx(handle, NULL, vtss_auth_radius_rx_callback)) != VTSS_OK) {
        T_W("vtss_radius_tx() returned \"%s\" (%d)", vtss_radius_error_txt(rc), rc);
    } else {
        while (radius_data.ready == FALSE) {
            /* We know that the callback will be called, so it is safe to wait without timeout */
            T_D("waiting for callback");
            (void) cyg_cond_wait(&radius_data.cond);
        }
        if (radius_data.res == VTSS_RADIUS_RX_CALLBACK_OK) {
            if (radius_data.code == VTSS_RADIUS_CODE_ACCESS_ACCEPT) {
                *userlevel = radius_data.userlevel;
                rc = VTSS_OK;
            } else {
                rc = VTSS_AUTH_ERROR_SERVER_REJECT;
            }
            rc = radius_data.code == VTSS_RADIUS_CODE_ACCESS_ACCEPT ? VTSS_OK : (vtss_rc)VTSS_AUTH_ERROR_SERVER_REJECT;
        } else if (radius_data.res == VTSS_RADIUS_RX_CALLBACK_TIMEOUT) {
            rc = VTSS_AUTH_ERROR_SERVER_TIMEOUT;
        } else {
            T_W("vtss_auth_radius_rx_callback() returned %d", radius_data.res);
            rc = VTSS_AUTH_ERROR_SERVER_BADRESP;
        }
    }
    (void) cyg_mutex_unlock(&radius_data.mutex);

#ifdef VTSS_AUTH_CHECK_HEAP
    sleep(1); // Give the memory a little time to get freed
    m_after = mallinfo();
    m_loss = m_before.fordblks - m_after.fordblks;
    if (m_loss) {
        T_E("Lost %d (0x%x) bytes in Radius client", m_loss, m_loss);
    }
#endif

    T_N("exit");
    return rc;
}
#endif /* VTSS_SW_OPTION_RADIUS */

#ifdef VTSS_SW_OPTION_TACPLUS
/* ================================================================= *
 *  vtss_auth_tac_connect()
 *  Connect to one of the configured TACACS+ servers.
 *  Handles dead time.
 *  If only one server is configured, dead time is ignored.
 * ================================================================= */
static struct session *vtss_auth_tac_connect(vtss_auth_proto_conf_t *conf)
{
    static ulong   deadtime[VTSS_AUTH_NUMBER_OF_SERVERS];
    ulong          time_now = VTSS_UPTIME;
    int            host_count = 0;
    int            i;
    struct session *session = NULL;

    T_N("enter");
    for (i = 0; i < VTSS_AUTH_NUMBER_OF_SERVERS; i++) {
        if (conf->host_conf[i].host[0]) {
            host_count++;
        } else {
            break;
        }
    }

    if (host_count) {
        for (i = 0; i < host_count; i++) {
            vtss_auth_host_conf_t *h = &conf->host_conf[i];
            T_D("server %d (%s) is configured - time is now %u", i + 1, h->host, time_now);
            if ((host_count == 1) || (time_now >= deadtime[i])) { /* only one server or we have passed the last dead time - try it */
                session = tac_connect(h->host,
                                      h->timeout ? h->timeout : conf->global_conf.timeout,
                                      h->key[0] ? h->key : conf->global_conf.key,
                                      h->auth_port);
                if (session) { /* It's alive */
                    T_D("connected to server %d (%s)", i + 1, h->host);
                    break;
                } else { /* It's dead. Store the next time we are allowed to contact it */
                    T_D("unable to connect to server %d (%s) - will wakeup at %u", i + 1, h->host, time_now + (conf->global_conf.deadtime * 60));
                    deadtime[i] = time_now + (conf->global_conf.deadtime * 60);
                }
            } else {
                T_D("server %d (%s) - dead timer is active until %u", i + 1, h->host, deadtime[i]);
            }
        }
    } else {
        T_W("No TACACS+ servers configured");
    }
    T_N("exit");
    return session;
}


/* ================================================================= *
 *  vtss_auth_tacplus()
 *  Authenticate via TACACS+
 * ================================================================= */
static vtss_rc vtss_auth_tacplus(vtss_auth_agent_t agent, vtss_auth_proto_conf_t *conf, char *username, char *passwd, int *userlevel)
{
    vtss_rc rc = VTSS_AUTH_ERROR_SERVER_REJECT;
    int     tac_rc;
    char    serv_msg[256];
    char    data_msg[256];
    struct session *session;

#ifdef VTSS_AUTH_CHECK_HEAP
    struct mallinfo m_before;
    struct mallinfo m_after;
    int m_loss;
    m_before = mallinfo();
#endif

    T_N("enter");

    *userlevel = 1;

    if ((session = vtss_auth_tac_connect(conf)) == NULL) {
        rc = VTSS_AUTH_ERROR_SERVER_TIMEOUT;
    } else {
        tac_rc = tac_authen_send_start(session, vtss_auth_agent_names[agent], username, TACACS_ASCII_LOGIN, "");
        if (tac_rc == 0) {
            T_W("tac_authen_send_start() failure");
        } else {
            tac_rc = tac_authen_get_reply(session, serv_msg, sizeof(serv_msg), data_msg, sizeof(data_msg));
            if (tac_rc <= 0) {
                T_W("tac_authen_get_reply() 1 failure");
            } else {
                T_D("returned from get reply 1 = %d, \"%s\", \"%s\"", tac_rc, serv_msg, data_msg);
                tac_rc = tac_authen_send_cont(session, passwd, "");
                if (tac_rc == 0) {
                    T_W("tac_authen_send_cont() failure");
                } else {
                    tac_rc = tac_authen_get_reply(session, serv_msg, sizeof(serv_msg), data_msg, sizeof(data_msg));
                    if (tac_rc <= 0) {
                        T_W("tac_authen_get_reply() 2 failure");
                    } else {
                        T_D("return from get reply 2 = %d, \"%s\", \"%s\"", tac_rc, serv_msg, data_msg);
                        if (tac_rc == TAC_PLUS_AUTHEN_STATUS_PASS) {
                            rc = VTSS_OK;
                        }
                    }
                }
            }
        }
        tac_close(session);
    }
    if (rc == VTSS_OK) { // Authentication succeeded - now try to get the authorization privilege level
        if ((session = vtss_auth_tac_connect(conf)) != NULL) {
            BOOL found = FALSE;
            char *avpair[255];

            avpair[0] = VTSS_STRDUP("service=shell");
            avpair[1] = VTSS_STRDUP("cmd=");
            avpair[2] = NULL;

            tac_rc = tac_author_send_request(session, TAC_PLUS_AUTHEN_METH_TACACSPLUS, TAC_PLUS_PRIV_LVL_MIN,
                                             TAC_PLUS_AUTHEN_TYPE_ASCII, TAC_PLUS_AUTHEN_SVC_LOGIN,
                                             username, vtss_auth_agent_names[agent], avpair);
            tac_free_avpairs(avpair);

            if (tac_rc == 0) {
                T_W("tac_author_send_request() failure");
            } else {
                tac_rc = tac_author_get_response(session, serv_msg, sizeof(serv_msg), data_msg, sizeof(data_msg), avpair);
                if (tac_rc <= 0) {
                    T_W("tac_author_get_response() failure");
                } else {
                    T_D("returned from get response = %d, \"%s\", \"%s\"", tac_rc, serv_msg, data_msg);
                    int i, av = 0;
                    while (avpair[av] != NULL) {
                        char *buf = avpair[av++];
                        for (i = 0; i < (int)strlen(buf); i++) {
                            buf[i] = tolower(buf[i]);
                        }
                        // avpair syntax: "priv-lvl=x" where x is an integer from 0 to 15
                        T_D("avpair \"%s\"", buf);
                        if (strstr(buf, "priv-lvl")) {
                            char *c = strrchr(buf, '=');
                            if (c) {
                                i = atoi(++c);                // fetch the number right after the last "="
                                if ((i >= 0) && (i <= 15)) {
                                    *userlevel = i;
                                    T_I("userlevel is set to %d", i);
                                } else {
                                    T_W("userlevel is out of range %d", i);
                                }
                                found = TRUE;
                            } else {
                                T_W("missing \"=\" in %s", buf);
                            }
                        }
                    }
                }
                tac_free_avpairs(avpair);
            }
            tac_close(session);
            if (!found) {
                T_I("avpair \"priv-lvl=x\" not returned from TACACS+ server");
            }
        } else {
            T_W("Unable to connect to TACACS+ authorization server");
        }
    }

#ifdef VTSS_AUTH_CHECK_HEAP
    sleep(1); // Give the memory a little time to get freed
    m_after = mallinfo();
    m_loss = m_before.fordblks - m_after.fordblks;
    if (m_loss) {
        T_E("Lost %d (0x%x) bytes in Tacacs client library", m_loss, m_loss);
    }
#endif

    T_N("exit");
    return rc;
}
#endif /* VTSS_SW_OPTION_TACPLUS */

/* ================================================================= *
 *  vtss_auth_local()
 *  Authenticate via the local database
 *  The system default username is "VTSS_SYS_ADMIN_NAME" defined in sysutil_api.h
 * ================================================================= */
static vtss_rc vtss_auth_local(char *username, char *passwd, int *userlevel)
{
    vtss_rc rc = VTSS_OK;
#ifdef VTSS_SW_OPTION_USERS
    users_conf_t conf;
#endif

    T_N("enter");

#ifdef VTSS_SW_OPTION_USERS
    misc_strncpyz(conf.username, username, sizeof(conf.username));
    misc_strncpyz(conf.password, passwd, sizeof(conf.password));
    if (vtss_users_mgmt_conf_get(&conf, 0) != VTSS_OK || strncmp(conf.password, passwd, sizeof(conf.password))) {
        rc = VTSS_AUTH_ERROR_SERVER_REJECT;
    } else {
        *userlevel = conf.privilege_level;
    }
#else
    if ((strncmp(username, VTSS_SYS_ADMIN_NAME, VTSS_AUTH_MAX_USERNAME_LENGTH)) ||
        (strncmp(passwd, system_get_passwd(), VTSS_AUTH_MAX_PASSWORD_LENGTH))) {
        rc = VTSS_AUTH_ERROR_SERVER_REJECT;
    }
    *userlevel = 15;
#endif
    T_I("userlevel is set to %d", *userlevel);

    T_N("exit");
    return rc;
}

/* ================================================================= *
 *  vtss_auth_httpd_cache_expire()
 *  Expire everything in the cache.
 *  Called when configuration is changed in order to force a
 *  reauthentication in the web browser.
 * ================================================================= */
static void vtss_auth_httpd_cache_expire(void)
{
    int i;

    T_N("enter");
    AUTH_CRIT_CACHE_ENTER();
    for (i = 0; i < VTSS_AUTH_HTTPD_CACHE_SIZE; i++) {
        httpd_cache[i].expires = 0;
    }
    AUTH_CRIT_CACHE_EXIT();
    T_N("exit");
}

#ifdef VTSS_SW_OPTION_WEB
/* ================================================================= *
 *  vtss_auth_httpd_cache_lookup()
 *  Lookup an entry in the cache.
 *  This function will ALWAYS return an entry.
 *  The return code shows what kind of entry it is:
 *  VTSS_OK:
 *    The entry is valid and has not expired. The caller must update
 *    the expire time.
 *
 *  VTSS_AUTH_ERROR_CACHE_EXPIRED:
 *    The entry matches username and password but is expired.
 *    The caller must reauthenticate username and password.
 *    If authentication is ok, the caller must update the userlevel
 *    and the expire time.
 *
 *  VTSS_AUTH_ERROR_CACHE_INVALID:
 *    The entry does not match username and password.
 *    The caller must reauthenticate username and password.
 *    If authentication is ok, the caller must update the username,
 *    the password, the userlevel and the expire time.
 *
 *  By using this strategy, we will only have to lookup once.
 *  Remember that this function is often called more than one time
 *  for each http request.
 * ================================================================= */
static vtss_rc vtss_auth_httpd_cache_lookup(char *username, char *password, vtss_auth_httpd_cache_t **entry)
{
    int i;
    int oldest_index = 0;
    ulong oldest = ULONG_MAX;
    int username_index = -1;
    ulong time_now = VTSS_UPTIME;

    for (i = 0; i < VTSS_AUTH_HTTPD_CACHE_SIZE; i++) {
        if (httpd_cache[i].expires < oldest) { /* remember the oldest entry in case of no match at all */
            oldest = httpd_cache[i].expires;
            oldest_index = i;
        }
        if (strncmp(httpd_cache[i].username, username, VTSS_AUTH_MAX_USERNAME_LENGTH) == 0) {
            username_index = i; /* remember that we have found a username match */
            if (strncmp(httpd_cache[i].password, password, VTSS_AUTH_MAX_PASSWORD_LENGTH) == 0) {
                *entry = &httpd_cache[i];
                if (time_now <= httpd_cache[i].expires) { /* match and not expired */
                    return VTSS_OK;
                } else { /* match but expired */
                    return VTSS_AUTH_ERROR_CACHE_EXPIRED;
                }
            }
        }
    }

    if (username_index != -1) { /* we found an entry that matches the username - reuse this one */
        *entry = &httpd_cache[username_index];
    } else { /* no match at all - return the oldest entry in the cache */
        *entry = &httpd_cache[oldest_index];
    }
    return VTSS_AUTH_ERROR_CACHE_INVALID;
}


/* ================================================================= *
 *  vtss_auth_httpd_callback()
 *  Callback for the httpd server
 *  Returns 0 on success, 1 on error
 * ================================================================= */
static int vtss_auth_httpd_callback(char *username, char *password, int *userlevel)
{
    vtss_auth_httpd_cache_t *entry;
    vtss_rc rc;
    int status;

    T_D("Authenticating '%s','%s'", username, password);
    if (!username[0]) {
        T_D("Auth failure (empty username)");
        return 1;
    }
    /* The invalid username "~" is reserved for logging out from Safari and Chrome */
    if ((username[0] == '~') && !username[1]) {
        T_D("Auth failure (reserved username)");
        return 1;
    }

    AUTH_CRIT_CACHE_ENTER();
    if ((rc = vtss_auth_httpd_cache_lookup(username, password, &entry)) == VTSS_OK) {
        entry->expires = VTSS_UPTIME + VTSS_AUTH_HTTPD_CACHE_LIFE_TIME;
        *userlevel = entry->userlevel;
        T_D("Match and not expired");
        status = 0;
    } else {
        if (rc == VTSS_AUTH_ERROR_CACHE_EXPIRED) {
            T_D("Match but expired");
        } else {
            T_D("No match");
        }

        if (vtss_authenticate(VTSS_AUTH_AGENT_HTTP, username, password, userlevel) == VTSS_OK) {
            entry->expires = VTSS_UPTIME + VTSS_AUTH_HTTPD_CACHE_LIFE_TIME;
            entry->userlevel = *userlevel;
            if (rc == VTSS_AUTH_ERROR_CACHE_INVALID) {
                misc_strncpyz(entry->username, username, VTSS_AUTH_MAX_USERNAME_LENGTH);
                misc_strncpyz(entry->password, password, VTSS_AUTH_MAX_PASSWORD_LENGTH);
            }
            status = 0;
        } else {
            T_D("Auth failure");
            status = 1;
        }
    }
    AUTH_CRIT_CACHE_EXIT();
    return status;
}
#endif /* VTSS_SW_OPTION_WEB */

/* ================================================================= *
 *  vtss_auth_thread()
 *  Runs the server part of the auth module
 *  The server keeps a local copy of the configuration, in order to
 *  avoid locking of the configuration several times during a lengthy
 *  autentication.
 * ================================================================= */
static void vtss_auth_thread(cyg_addrword_t data)
{
    vtss_auth_conf_t local_config;

#ifdef VTSS_SW_OPTION_RADIUS
    vtss_auth_radius_init();
#endif /* VTSS_SW_OPTION_RADIUS */

#ifdef VTSS_SW_OPTION_WEB
    cyg_httpd_auth_callback_register(vtss_auth_httpd_callback);
#endif /* VTSS_SW_OPTION_WEB */

    for (;;) {
        vtss_auth_msg_t *msg = (vtss_auth_msg_t *)cyg_mbox_get(thread.mbox_handle);
        T_N("begin server");

        AUTH_CRIT_CONFIG_ENTER();
        if (config_changed) {
            T_D("config changed");
            config_changed = FALSE;
            local_config = config;
        }
        AUTH_CRIT_CONFIG_EXIT();

        if (msg_switch_is_master()) {
            int                     m;
            vtss_rc                 rc;
            vtss_auth_agent_conf_t *client = &local_config.agent[msg->re.agent];
            if (msg->re.agent < VTSS_AUTH_AGENT_LAST) {
                for (m = 0; m < VTSS_AUTH_METHOD_LAST; m++) {
                    switch (client->method[m]) {
                    case VTSS_AUTH_METHOD_LOCAL:
                        rc = vtss_auth_local(msg->re.username, msg->re.password, &msg->re.userlevel);
                        break;

                    case VTSS_AUTH_METHOD_RADIUS:
#ifdef VTSS_SW_OPTION_RADIUS
                        rc = vtss_auth_radius(msg->re.agent, msg->re.username, msg->re.password, &msg->re.userlevel);
#else
                        rc = VTSS_AUTH_ERROR_NO_RADIUS;
#endif /* VTSS_SW_OPTION_RADIUS */
                        break;

                    case VTSS_AUTH_METHOD_TACACS:
#ifdef VTSS_SW_OPTION_TACPLUS
                        rc = vtss_auth_tacplus(msg->re.agent, &local_config.proto[VTSS_AUTH_PROTO_TACACS], msg->re.username, msg->re.password, &msg->re.userlevel);
#else
                        T_E("TACACS+ is not included in current build");
                        rc = VTSS_AUTH_ERROR_NO_TACPLUS;
#endif /* VTSS_SW_OPTION_RADIUS */
                        break;

                    default:
                        rc = VTSS_AUTH_ERROR_SERVER_REJECT; /* Authentication not allowed */
                        break;
                    }
                    if ((rc == VTSS_OK) || (rc == VTSS_AUTH_ERROR_SERVER_REJECT)) {
                        break;
                    }
                }
                if (m == VTSS_AUTH_METHOD_LAST) {
                    msg->re.rc = VTSS_AUTH_ERROR_SERVER_REJECT; /* Authentication not allowed */
                } else {
                    msg->re.rc = rc;
                }
            } else {
                T_E("Invalid client (%d)", msg->re.agent);
            }
            msg->msg_id = VTSS_AUTH_MSG_ID_AUTH_REPLY;
            msg_tx(VTSS_MODULE_ID_AUTH, msg->re.isid, msg, sizeof(vtss_auth_msg_t));
        } else {
            T_W("server request received on a slave");
            VTSS_FREE(msg);
        }
        T_N("end server");
    }
}

/* ================================================================= *
 *  vtss_auth_msg_tx_done()
 *  Callback invoked when a message transmission is complete,
 *  successfully or unsuccessfully.
 * ================================================================= */
static void vtss_auth_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
    if (rc != MSG_TX_RC_OK && msg) {
        // A message tx error occurred. Signal the waiter.
        vtss_auth_msg_t *auth_msg = (vtss_auth_msg_t *)msg;
        T_N("msg_tx error");
        vtss_auth_client_buf_t *buf = vtss_auth_client_buf_lookup(auth_msg->re.seq_num);
        if (buf) {
            (void)cyg_mutex_lock(&buf->mutex);
            buf->msg.re.rc = VTSS_AUTH_ERROR_CLIENT_MSG_TX_FAILED; /* Make the client retry */
            cyg_cond_signal(&buf->cond);
            (void)cyg_mutex_unlock(&buf->mutex);
        }
    }
}

/* ================================================================= *
 *  If VTSS_AUTH_SILENT_UPGRADE is defined then enable silent upgrade
 *  of auth configuration from 2.80 to current version.
 *
 *  If NOT defined then create defaults as usual.
 *
 *  Using a separate define makes it possible to test this feature
 *  without requiring VTSS_SW_OPTION_SILENT_UPGRADE to be defined.
 * ================================================================= */
#define VTSS_AUTH_SILENT_UPGRADE
#if defined(VTSS_AUTH_SILENT_UPGRADE)
/* ================================================================= *
 *  Old flash configuration layout - used for silent upgrade.
 * ================================================================= */
#define VTSS_AUTH_CONF_VERSION_OLD        1
#define VTSS_AUTH_SECRET_MAX_LENGTH_OLD  30
#define VTSS_AUTH_NUMBER_OF_SERVERS_OLD   5

typedef enum {
    VTSS_AUTH_METHOD_NONE_OLD,
    VTSS_AUTH_METHOD_LOCAL_OLD,
    VTSS_AUTH_METHOD_RADIUS_OLD,
    VTSS_AUTH_METHOD_TACPLUS_OLD,
    VTSS_AUTH_METHOD_LAST_OLD
} vtss_auth_method_t_old;

typedef enum {
    VTSS_AUTH_AGENT_TELNET_OLD,
    VTSS_AUTH_AGENT_SSH_OLD,
    VTSS_AUTH_AGENT_WEB_OLD,
    VTSS_AUTH_AGENT_CONSOLE_OLD,
    VTSS_AUTH_AGENT_LAST_OLD
} vtss_auth_agent_t_old;

typedef struct {
    BOOL   enabled;
    uchar  server_ip_string[255];
    ushort port;
    char   secret[VTSS_AUTH_SECRET_MAX_LENGTH_OLD];
} vtss_auth_server_info_t_old;

typedef struct {
    vtss_auth_method_t_old auth_method;
    BOOL                   use_local;
} vtss_auth_agent_info_t_old;

typedef struct {
    ulong                       server_timeout;
    ulong                       server_dead_time;
    vtss_auth_server_info_t_old server_radius[VTSS_AUTH_NUMBER_OF_SERVERS_OLD];
    vtss_auth_server_info_t_old server_radius_acct[VTSS_AUTH_NUMBER_OF_SERVERS_OLD];
    vtss_auth_server_info_t_old server_tacplus[VTSS_AUTH_NUMBER_OF_SERVERS_OLD];
    vtss_auth_agent_info_t_old  agent[VTSS_AUTH_AGENT_LAST_OLD];
} vtss_auth_conf_t_old;

typedef struct {
    ulong                version;
    vtss_auth_conf_t_old conf;
} vtss_auth_conf_blk_t_old;

/* ================================================================= *
 *  vtss_auth_conf_flash_upgrade()
 *  Try to upgrade from old configuration layout.
 *  Returns a (malloc'ed) pointer to the upgraded new configuration
 *  or NULL if conversion failed.
 * ================================================================= */
static vtss_auth_conf_blk_t *vtss_auth_conf_flash_upgrade(const void *blk, size_t size)
{
    vtss_auth_conf_blk_t *new_blk = NULL;

    if (size == sizeof(vtss_auth_conf_blk_t_old)) {
        if ((new_blk = (vtss_auth_conf_blk_t *)VTSS_MALLOC(sizeof(*new_blk)))) {
            /* Suppress "Relational operator '<' always evaluates to 'false'" and "non-negative quantity is never less than zero" */
            /*lint --e{568, 685} */
            vtss_auth_conf_blk_t_old *old_blk = (vtss_auth_conf_blk_t_old *)blk;
            int i;
            int dix; // destination index
            u32 val;
            BOOL acct_done[VTSS_AUTH_NUMBER_OF_SERVERS] = {FALSE};

            memset(new_blk, 0, sizeof(*new_blk));
            new_blk->version = VTSS_AUTH_CONF_VERSION;
            /* new_blk->config.radius is all zero - this is fine */

            /* upgrade new_blk->config.agent[0..3] */
            new_blk->config.agent[VTSS_AUTH_AGENT_CONSOLE].method[0] = (vtss_auth_method_t)old_blk->conf.agent[VTSS_AUTH_AGENT_CONSOLE_OLD].auth_method;
            if (old_blk->conf.agent[VTSS_AUTH_AGENT_CONSOLE_OLD].use_local) {
                new_blk->config.agent[VTSS_AUTH_AGENT_CONSOLE].method[1] = VTSS_AUTH_METHOD_LOCAL;
            }
            new_blk->config.agent[VTSS_AUTH_AGENT_TELNET].method[0] = (vtss_auth_method_t)old_blk->conf.agent[VTSS_AUTH_AGENT_TELNET_OLD].auth_method;
            if (old_blk->conf.agent[VTSS_AUTH_AGENT_TELNET_OLD].use_local) {
                new_blk->config.agent[VTSS_AUTH_AGENT_TELNET].method[1] = VTSS_AUTH_METHOD_LOCAL;
            }
            new_blk->config.agent[VTSS_AUTH_AGENT_SSH].method[0] = (vtss_auth_method_t)old_blk->conf.agent[VTSS_AUTH_AGENT_SSH_OLD].auth_method;
            if (old_blk->conf.agent[VTSS_AUTH_AGENT_SSH_OLD].use_local) {
                new_blk->config.agent[VTSS_AUTH_AGENT_SSH].method[1] = VTSS_AUTH_METHOD_LOCAL;
            }
            new_blk->config.agent[VTSS_AUTH_AGENT_HTTP].method[0] = (vtss_auth_method_t)old_blk->conf.agent[VTSS_AUTH_AGENT_WEB_OLD].auth_method;
            if (old_blk->conf.agent[VTSS_AUTH_AGENT_WEB_OLD].use_local) {
                new_blk->config.agent[VTSS_AUTH_AGENT_HTTP].method[1] = VTSS_AUTH_METHOD_LOCAL;
            }

            /* upgrade new_blk->config.proto[0..2].global_conf */
            val = old_blk->conf.server_timeout;
            if (val < VTSS_AUTH_TIMEOUT_MIN) {
                T_D("timeout %u -> %u", val, VTSS_AUTH_TIMEOUT_MIN);
                val = VTSS_AUTH_TIMEOUT_MIN;
            } else if (val > VTSS_AUTH_TIMEOUT_MAX) {
                T_D("timeout %u -> %u", val, VTSS_AUTH_TIMEOUT_MAX);
                val = VTSS_AUTH_TIMEOUT_MAX;
            }
            new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].global_conf.timeout =
                new_blk->config.proto[VTSS_AUTH_PROTO_TACACS].global_conf.timeout = val;

            new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].global_conf.retransmit =
                new_blk->config.proto[VTSS_AUTH_PROTO_TACACS].global_conf.retransmit = VTSS_AUTH_RETRANSMIT_DEFAULT;

            val = old_blk->conf.server_dead_time / 60;
            if (val < VTSS_AUTH_DEADTIME_MIN) {
                T_D("deadtime %u -> %u", val, VTSS_AUTH_DEADTIME_MIN);
                val = VTSS_AUTH_DEADTIME_MIN;
            } else if (val > VTSS_AUTH_DEADTIME_MAX) {
                T_D("deadtime %u -> %u", val, VTSS_AUTH_DEADTIME_MAX);
                val = VTSS_AUTH_DEADTIME_MAX;
            }
            new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].global_conf.deadtime =
                new_blk->config.proto[VTSS_AUTH_PROTO_TACACS].global_conf.deadtime = val;

            /* upgrade new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].host_conf[0..4]
             * This is a little tricky - We are trying to put up to ten different
             * servers into five slots amd this is not always possible!
             * Loop through each enabled auth server and add it.
             * Search for an acct server on the same host and add it in the same entry.
             * If the acct key is different from the auth key, the acct key is discarded.
             * If there are more than 5 different auth and acct servers (host names) in
             * the old configuration, the auth servers will have first priority and the
             * remaing acct servers are discarded.
             */
            dix = 0;
            for (i = 0; i < VTSS_AUTH_NUMBER_OF_SERVERS; i++) {
                if (old_blk->conf.server_radius[i].enabled) {
                    int j;
                    T_D("RADIUS upgrading auth #%d: %s:%u:%s",
                        i + 1,
                        old_blk->conf.server_radius[i].server_ip_string,
                        old_blk->conf.server_radius[i].port,
                        old_blk->conf.server_radius[i].secret);
                    strncpy(new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].host_conf[dix].host,
                            (char *)old_blk->conf.server_radius[i].server_ip_string,
                            VTSS_AUTH_HOST_LEN - 1);
                    new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].host_conf[dix].auth_port = old_blk->conf.server_radius[i].port;
                    strncpy(new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].host_conf[dix].key,
                            old_blk->conf.server_radius[i].secret,
                            VTSS_AUTH_KEY_LEN - 1);

                    for (j = 0; j < VTSS_AUTH_NUMBER_OF_SERVERS; j++) {
                        if (!acct_done[j] &&
                            old_blk->conf.server_radius_acct[j].enabled &&
                            strncmp((char *)old_blk->conf.server_radius[i].server_ip_string,
                                    (char *)old_blk->conf.server_radius_acct[j].server_ip_string,
                                    255) == 0) {
                            T_D("RADIUS adding acct #%d: %s:%u (discarding %s)",
                                j + 1,
                                old_blk->conf.server_radius_acct[j].server_ip_string,
                                old_blk->conf.server_radius_acct[j].port,
                                old_blk->conf.server_radius_acct[j].secret);
                            new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].host_conf[dix].acct_port = old_blk->conf.server_radius_acct[j].port;
                            acct_done[j] = TRUE; // We have finished this entry
                        }
                    }
                    dix++;
                }

            }

            /* Loop through each remaining enabled acct server and add them (if there is room for it) */
            for (i = 0; i < VTSS_AUTH_NUMBER_OF_SERVERS; i++) {
                if (!acct_done[i] && old_blk->conf.server_radius_acct[i].enabled) {
                    T_D("RADIUS %s acct #%d: %s:%u:%s",
                        (dix < VTSS_AUTH_NUMBER_OF_SERVERS) ? "upgrading" : "discarding",
                        i + 1,
                        old_blk->conf.server_radius_acct[i].server_ip_string,
                        old_blk->conf.server_radius_acct[i].port,
                        old_blk->conf.server_radius_acct[i].secret);
                    if (dix < VTSS_AUTH_NUMBER_OF_SERVERS) {
                        strncpy(new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].host_conf[dix].host,
                                (char *)old_blk->conf.server_radius_acct[i].server_ip_string,
                                VTSS_AUTH_HOST_LEN - 1);
                        new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].host_conf[dix].acct_port = old_blk->conf.server_radius_acct[i].port;
                        strncpy(new_blk->config.proto[VTSS_AUTH_PROTO_RADIUS].host_conf[dix].key,
                                old_blk->conf.server_radius_acct[i].secret,
                                VTSS_AUTH_KEY_LEN - 1);
                        dix++;
                    }
                }

            }

            /* upgrade new_blk->config.proto[VTSS_AUTH_PROTO_TACACS].host_conf[0..4] */
            dix = 0;
            for (i = 0; i < VTSS_AUTH_NUMBER_OF_SERVERS; i++) {
                if (old_blk->conf.server_tacplus[i].enabled) {
                    T_D("TACACS upgrading #%d: %s:%u:%s",
                        i + 1,
                        old_blk->conf.server_tacplus[i].server_ip_string,
                        old_blk->conf.server_tacplus[i].port,
                        old_blk->conf.server_tacplus[i].secret);
                    strncpy(new_blk->config.proto[VTSS_AUTH_PROTO_TACACS].host_conf[dix].host,
                            (char *)old_blk->conf.server_tacplus[i].server_ip_string,
                            VTSS_AUTH_HOST_LEN - 1);
                    new_blk->config.proto[VTSS_AUTH_PROTO_TACACS].host_conf[dix].auth_port = old_blk->conf.server_tacplus[i].port;
                    strncpy(new_blk->config.proto[VTSS_AUTH_PROTO_TACACS].host_conf[dix].key,
                            old_blk->conf.server_tacplus[i].secret,
                            VTSS_AUTH_KEY_LEN - 1);
                    dix++;
                }
            }
        }
    }
    return new_blk;
}
#endif /* defined(VTSS_AUTH_SILENT_UPGRADE) */

/* ================================================================= *
 *  vtss_auth_conf_flash_read()
 *  Read the auth configuration from flash. @create indicates that a
 *  new default configuration block should be created.
 * ================================================================= */
static void vtss_auth_conf_flash_read(BOOL create)
{
    vtss_auth_conf_blk_t *blk;
    ulong                size = 0;
    BOOL                 do_create = FALSE;
    int                  i;

    if (misc_conf_read_use()) {
        if ((blk = (vtss_auth_conf_blk_t *)conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_AUTH_TABLE, &size)) == NULL) {
            T_W("conf_sec_open failed, creating defaults");
            blk = (vtss_auth_conf_blk_t *)conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_AUTH_TABLE, sizeof(*blk));
            do_create = TRUE;
        } else if (size != sizeof(*blk)) {
#if defined(VTSS_AUTH_SILENT_UPGRADE)
            vtss_auth_conf_blk_t *new_blk;
            T_I("size mismatch, try upgrade");
            new_blk = (vtss_auth_conf_blk_t *)vtss_auth_conf_flash_upgrade(blk, size);
            blk = (vtss_auth_conf_blk_t *)conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_AUTH_TABLE, sizeof(*blk));
            if (new_blk && blk) {
                T_I("upgrade ok");
                *blk = *new_blk;
            } else {
                T_W("upgrade failed, creating defaults");
                do_create = TRUE;
            }
            if (new_blk) {
                VTSS_FREE(new_blk);
            }
#else
            T_W("size mismatch, creating defaults");
            blk = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_AUTH_TABLE, sizeof(*blk));
            do_create = TRUE;
#endif /* defined(VTSS_AUTH_SILENT_UPGRADE) */
        } else if (blk->version != VTSS_AUTH_CONF_VERSION) {
            T_W("version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        T_N("no silent upgrade; creating defaults");
        do_create = TRUE;
        blk       = NULL;
    }

    AUTH_CRIT_CONFIG_ENTER();
    if (do_create) {
        /* Create default configuration */
        memset(&config, 0, sizeof(config));
        for (i = 0; i < VTSS_AUTH_PROTO_LAST; i++) {
            config.proto[i].global_conf.timeout    = VTSS_AUTH_TIMEOUT_DEFAULT;
            config.proto[i].global_conf.retransmit = VTSS_AUTH_RETRANSMIT_DEFAULT;
            config.proto[i].global_conf.deadtime   = VTSS_AUTH_DEADTIME_DEFAULT;
        }
        for (i = 0; i < VTSS_AUTH_AGENT_LAST; i++ ) {
            config.agent[i] = vtss_auth_agent_conf_default;
        }
        if (blk != NULL) {
            blk->config = config;
        }
    } else {
        if (blk != NULL) {          // Quiet lint
            config = blk->config;
        }
    }

    /* Sanity check */
    for (i = 0; i < VTSS_AUTH_AGENT_LAST; i++ ) {
        int m;
        for (m = 0; m < VTSS_AUTH_METHOD_LAST; m++) {
#ifndef VTSS_SW_OPTION_RADIUS
            if (config.agent[i].method[m] == VTSS_AUTH_METHOD_RADIUS) {
                config.agent[i].method[m] = VTSS_AUTH_METHOD_LOCAL; /* Enforce a valid configuration */
            }
#endif /* VTSS_SW_OPTION_RADIUS */
#ifndef VTSS_SW_OPTION_TACPLUS
            if (config.agent[i].method[m] == VTSS_AUTH_METHOD_TACACS) {
                config.agent[i].method[m] = VTSS_AUTH_METHOD_LOCAL; /* Enforce a valid configuration */
            }
#endif /* VTSS_SW_OPTION_TACPLUS */
        }
    }

#ifdef VTSS_SW_OPTION_RADIUS
    {
        vtss_rc rc;
        if ((rc = vtss_auth_radius_config_update(&config.radius, &config.proto[VTSS_AUTH_PROTO_RADIUS])) != VTSS_OK) {
            T_W("vtss_auth_radius_config_update() failed: \"%s\"", error_txt(rc));
        }
    }
#endif /* VTSS_SW_OPTION_RADIUS */
    config_changed = TRUE;
    AUTH_CRIT_CONFIG_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (blk == NULL) {
        T_W("failed to open auth config");
    } else {
        blk->version = VTSS_AUTH_CONF_VERSION;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_AUTH_TABLE);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/* ================================================================= *
 *  vtss_auth_conf_flash_write()
 *  Writes the auth configuration into flash.
 * ================================================================= */
static void vtss_auth_conf_flash_write(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    vtss_auth_conf_blk_t *blk;

    if ((blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_AUTH_TABLE, NULL)) == NULL) {
        T_W("Failed to open flash configuration");
    } else {
        AUTH_CRIT_CONFIG_ENTER();
        blk->config = config;
        AUTH_CRIT_CONFIG_EXIT();
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_AUTH_TABLE);
    }
#else
    T_N("Silent-upgrade build: Not saving to conf");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/* ================================================================= *
 *  vtss_auth_ssh_authenticate()
 *  This function authenticates as an ssh agent, even if the ssh
 *  module supplies a wrong agent (which is does).
 * ================================================================= */
#ifdef VTSS_SW_OPTION_SSH
static vtss_rc vtss_auth_ssh_authenticate(vtss_auth_agent_t agent, char *username, char *password, int *userlevel)
{
    return vtss_authenticate(VTSS_AUTH_AGENT_SSH, username, password, userlevel);
}
#endif

/* ================================================================= *
 *
 * Global functions starts here
 *
 * ================================================================= */

/* ================================================================= *
 * vtss_auth_error_txt()
 * ================================================================= */
char *vtss_auth_error_txt(vtss_rc rc)
{
    switch (rc) {
    case VTSS_AUTH_ERROR_GEN:
        return "General error";
    case VTSS_AUTH_ERROR_CLIENT_WAITING:
        return "Client is waiting for reply";
    case VTSS_AUTH_ERROR_CLIENT_TIMEOUT:
        return "Request timed out in client";
    case VTSS_AUTH_ERROR_SERVER_TIMEOUT:
        return "Request timed out in server";
    case VTSS_AUTH_ERROR_SERVER_REJECT:
        return "Invalid username or password";
    case VTSS_AUTH_ERROR_SERVER_BADRESP:
        return "Invalid response received from server";
    case VTSS_AUTH_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";
    case VTSS_AUTH_ERROR_CFG_TIMEOUT:
        return "Invalid timeout configuration parameter";
    case VTSS_AUTH_ERROR_CFG_RETRANSMIT:
        return "Invalid retransmit configuration parameter";
    case VTSS_AUTH_ERROR_CFG_DEADTIME:
        return "Invalid deadtime configuration parameter";
    case VTSS_AUTH_ERROR_CFG_HOST:
        return "Invalid host name or IP address";
    case VTSS_AUTH_ERROR_CFG_PORT:
        return "Invalid port configuration parameter";
    case VTSS_AUTH_ERROR_CFG_HOST_PORT:
        return "Invalid host and port combination";
    case VTSS_AUTH_ERROR_CFG_HOST_TABLE_FULL:
        return "Host table is full";
    case VTSS_AUTH_ERROR_CFG_HOST_NOT_FOUND:
        return "Host not found";
    case VTSS_AUTH_ERROR_CFG_AGENT_AUTH_METHOD:
        return "Invalid agent authentication method";
    case VTSS_AUTH_ERROR_NO_RADIUS:
        return "RADIUS is not present in this build";
    case VTSS_AUTH_ERROR_NO_TACPLUS:
        return "TACACS+ is not present in this build";
    case VTSS_AUTH_ERROR_CACHE_EXPIRED:
        return "HTTPD cache entry is expired";
    case VTSS_AUTH_ERROR_CACHE_INVALID:
        return "HTTPD cache has no valid entry";
    case VTSS_AUTH_ERROR_CLIENT_MSG_TX_FAILED:
        return "Client message Tx error";
    default:
        return "Unknown auth error code";
    }
}

/* ================================================================= *
 *  vtss_authenticate()
 *  Make a client request, send it to the server and wait for a reply.
 * ================================================================= */
vtss_rc vtss_authenticate(vtss_auth_agent_t agent, char *username, char *password, int *userlevel)
{
    vtss_auth_client_buf_t *buf;
    cyg_tick_count_t       timeout;
    vtss_rc                rc;

    T_D("Authenticating '%s','%s'", username, password);
    if (!username[0]) {
        T_D("Auth failure (empty username)");
        return VTSS_AUTH_ERROR_SERVER_REJECT;
    }

    buf = vtss_auth_client_buf_alloc(VTSS_AUTH_MSG_ID_AUTH_REQUEST);

    /* Initialize message */
    buf->msg.re.agent = agent;
    misc_strncpyz(buf->msg.re.username, username, VTSS_AUTH_MAX_USERNAME_LENGTH);
    misc_strncpyz(buf->msg.re.password, password, VTSS_AUTH_MAX_PASSWORD_LENGTH);
    buf->msg.re.password[VTSS_AUTH_MAX_PASSWORD_LENGTH - 1] = '\0'; /* force null termination */
    buf->msg.re.rc = VTSS_AUTH_ERROR_CLIENT_WAITING; /* also used here as condition variable!!! */
    timeout = cyg_current_time() +  VTSS_OS_MSEC2TICK(VTSS_AUTH_CLIENT_TIMEOUT * 1000);

    /* Wait for a reply from server */
    (void) cyg_mutex_lock(&buf->mutex);
    while (buf->msg.re.rc == VTSS_AUTH_ERROR_CLIENT_WAITING) {

        /* Transmit message to the server on the master */
        msg_tx_adv(NULL, vtss_auth_msg_tx_done, MSG_TX_OPT_DONT_FREE, VTSS_MODULE_ID_AUTH, 0, &buf->msg, sizeof(vtss_auth_msg_t));

        T_N("Waiting with timeout");
        if (cyg_cond_timed_wait(&buf->cond, timeout) == FALSE) {
            T_N("Timeout");
            buf->msg.re.rc = VTSS_AUTH_ERROR_CLIENT_TIMEOUT;
        } else if (buf->msg.re.rc == VTSS_AUTH_ERROR_CLIENT_MSG_TX_FAILED) {
            T_N("msg_tx error");
            // Try again after a while
            buf->msg.re.rc = VTSS_AUTH_ERROR_CLIENT_WAITING;
            VTSS_OS_MSLEEP(1000);
        } else {
            T_N("Got valid reply: %s", error_txt(buf->msg.re.rc));
        }
    }
    /* Copy everything before releasing the buffer */
    *userlevel = buf->msg.re.userlevel;
    rc = buf->msg.re.rc;
    (void) cyg_mutex_unlock(&buf->mutex);

    vtss_auth_client_buf_free(buf);
    return rc;
}

/* ================================================================= *
 *  vtss_auth_dbg()
 *  Test of vtss_authenticate()
 * ================================================================= */
void vtss_auth_dbg(vtss_auth_dbg_printf_t dbg_printf, vtss_auth_agent_t agent, char *username, char *password)
{
    int userlevel;
    vtss_rc rc;
    rc = vtss_authenticate(agent, username, password, &userlevel);
    (void) dbg_printf("%s %s %s: %s (userlevel: %d)\n", vtss_auth_agent_names[agent], username, password, (rc == VTSS_OK) ? "SUCCESS" : error_txt(rc), userlevel);
}

/* ================================================================= *
 *  vtss_auth_mgmt_agent_conf_get()
 *  Returns the current agent configuration
 * ================================================================= */
vtss_rc vtss_auth_mgmt_agent_conf_get(vtss_auth_agent_t agent, vtss_auth_agent_conf_t *const conf)
{
    VTSS_ASSERT(agent < VTSS_AUTH_AGENT_LAST);
    VTSS_ASSERT(conf);

    if (!msg_switch_is_master()) {
        return VTSS_AUTH_ERROR_MUST_BE_MASTER;
    }

    AUTH_CRIT_CONFIG_ENTER();
    *conf = config.agent[agent];
    AUTH_CRIT_CONFIG_EXIT();
    return VTSS_OK;
}

/* ================================================================= *
 *  vtss_auth_mgmt_agent_conf_set()
 *  Set the current agent configuration and save it to flash
 * ================================================================= */
vtss_rc vtss_auth_mgmt_agent_conf_set(vtss_auth_agent_t agent, const vtss_auth_agent_conf_t *const conf)
{
    int  i;
    BOOL changed = FALSE;
#ifdef VTSS_AUTH_ENABLE_CONSOLE
    BOOL close_serial = FALSE;
#endif
#ifdef VTSS_SW_OPTION_CLI_TELNET
    BOOL close_telnet = FALSE;
#endif
#ifdef VTSS_SW_OPTION_SSH
    BOOL close_ssh = FALSE;
#endif
#ifdef VTSS_SW_OPTION_WEB
    BOOL close_http = FALSE;
#endif

    VTSS_ASSERT(agent < VTSS_AUTH_AGENT_LAST);
    VTSS_ASSERT(conf);

    if (!msg_switch_is_master()) {
        return VTSS_AUTH_ERROR_MUST_BE_MASTER;
    }

    /* verify agent methods */
    for (i = 0; i < (int)VTSS_AUTH_METHOD_LAST; i++ ) {
        if (conf->method[i] >= VTSS_AUTH_METHOD_LAST) {
            return VTSS_AUTH_ERROR_CFG_AGENT_AUTH_METHOD;
        }
    }

    AUTH_CRIT_CONFIG_ENTER();
    if (memcmp(&config.agent[agent], conf, sizeof(*conf))) {
        config.agent[agent] = *conf;
        changed = TRUE;
        config_changed = TRUE;

#ifdef VTSS_AUTH_ENABLE_CONSOLE
        if (agent == VTSS_AUTH_AGENT_CONSOLE) {
            close_serial = TRUE;
        }
#endif
#ifdef VTSS_SW_OPTION_CLI_TELNET
        if (agent == VTSS_AUTH_AGENT_TELNET) {
            close_telnet = TRUE;
        }
#endif
#ifdef VTSS_SW_OPTION_SSH
        if (agent == VTSS_AUTH_AGENT_SSH) {
            close_ssh = TRUE;
        }
#endif
#ifdef VTSS_SW_OPTION_WEB
        if (agent == VTSS_AUTH_AGENT_HTTP) {
            close_http = TRUE;
        }
#endif
    }
    AUTH_CRIT_CONFIG_EXIT();

    if (changed) {
        vtss_auth_conf_flash_write();

        /* Closing down sessions must be done outside critical section as it might be activated from one of the closing sessions */
#ifdef VTSS_AUTH_ENABLE_CONSOLE
        if (close_serial) {
            cli_serial_close();
        }
#endif
#ifdef VTSS_SW_OPTION_CLI_TELNET
        if (close_telnet) {
            cli_telnet_close();
        }
#endif
#ifdef VTSS_SW_OPTION_SSH
        if (close_ssh) {
            ssh_close_all_session();
        }
#endif
#ifdef VTSS_SW_OPTION_WEB
        if (close_http) {
            vtss_auth_httpd_cache_expire(); /* force the httpd cache to be expired */
        }
#endif
    }

    return VTSS_OK;
}

/* ================================================================= *
 *  vtss_auth_mgmt_radius_conf_get()
 *  Returns the current RADIUS specific configuration
 * ================================================================= */
vtss_rc vtss_auth_mgmt_radius_conf_get(vtss_auth_radius_conf_t *const conf)
{
    VTSS_ASSERT(conf);

    if (!msg_switch_is_master()) {
        return VTSS_AUTH_ERROR_MUST_BE_MASTER;
    }

    AUTH_CRIT_CONFIG_ENTER();
    *conf = config.radius;
    AUTH_CRIT_CONFIG_EXIT();
    return VTSS_OK;
}

/* ================================================================= *
 *  vtss_auth_mgmt_radius_conf_set()
 *  Set the current RADIUS specific configuration and save it to flash
 * ================================================================= */
vtss_rc vtss_auth_mgmt_radius_conf_set(const vtss_auth_radius_conf_t *const conf)
{
    BOOL    changed = FALSE;
    vtss_rc rc      = VTSS_OK;

    VTSS_ASSERT(conf);

    if (!msg_switch_is_master()) {
        return VTSS_AUTH_ERROR_MUST_BE_MASTER;
    }

    AUTH_CRIT_CONFIG_ENTER();
    if (memcmp(&config.radius, conf, sizeof(*conf))) {
        config.radius = *conf;
        changed = TRUE;
        config_changed = TRUE;
#ifdef VTSS_SW_OPTION_RADIUS
        rc = vtss_auth_radius_config_update(&config.radius, &config.proto[VTSS_AUTH_PROTO_RADIUS]);
#endif /* VTSS_SW_OPTION_RADIUS */

    }
    AUTH_CRIT_CONFIG_EXIT();

    if ((rc == VTSS_OK) && changed) {
        vtss_auth_conf_flash_write();
    }

    return rc;
}

/* ================================================================= *
 *  vtss_auth_mgmt_global_host_conf_get()
 *  Returns the current global host configuration
 * ================================================================= */
vtss_rc vtss_auth_mgmt_global_host_conf_get(vtss_auth_proto_t proto, vtss_auth_global_host_conf_t *const conf)
{
    VTSS_ASSERT(proto < VTSS_AUTH_PROTO_LAST);
    VTSS_ASSERT(conf);

    if (!msg_switch_is_master()) {
        return VTSS_AUTH_ERROR_MUST_BE_MASTER;
    }

    AUTH_CRIT_CONFIG_ENTER();
    *conf = config.proto[proto].global_conf;
    AUTH_CRIT_CONFIG_EXIT();
    return VTSS_OK;
}

/* ================================================================= *
 *  vtss_auth_mgmt_global_host_conf_set()
 *  Set the current global host configuration and save it to flash
 * ================================================================= */
vtss_rc vtss_auth_mgmt_global_host_conf_set(vtss_auth_proto_t proto, const vtss_auth_global_host_conf_t *const conf)
{
    BOOL    changed = FALSE;
    vtss_rc rc      = VTSS_OK;

    VTSS_ASSERT(proto < VTSS_AUTH_PROTO_LAST);
    VTSS_ASSERT(conf);

    if (!msg_switch_is_master()) {
        return VTSS_AUTH_ERROR_MUST_BE_MASTER;
    }
    if ((conf->timeout < VTSS_AUTH_TIMEOUT_MIN) || (conf->timeout > VTSS_AUTH_TIMEOUT_MAX)) {
        return VTSS_AUTH_ERROR_CFG_TIMEOUT;
    }
    if ((conf->retransmit < VTSS_AUTH_RETRANSMIT_MIN) || (conf->timeout > VTSS_AUTH_RETRANSMIT_MAX)) {
        return VTSS_AUTH_ERROR_CFG_RETRANSMIT;
    }
    // If VTSS_AUTH_XXXX_MIN is 0, lint will complain
    /*lint --e{685, 568} */
    if ((conf->deadtime < VTSS_AUTH_DEADTIME_MIN) || (conf->deadtime > VTSS_AUTH_DEADTIME_MAX)) {
        return VTSS_AUTH_ERROR_CFG_DEADTIME;
    }

    AUTH_CRIT_CONFIG_ENTER();
    if (memcmp(&config.proto[proto].global_conf, conf, sizeof(*conf))) {
        config.proto[proto].global_conf = *conf;
        changed = TRUE;
        config_changed = TRUE;
#ifdef VTSS_SW_OPTION_RADIUS
        if (proto == VTSS_AUTH_PROTO_RADIUS) {
            rc = vtss_auth_radius_config_update(&config.radius, &config.proto[VTSS_AUTH_PROTO_RADIUS]);
        }
#endif /* VTSS_SW_OPTION_RADIUS */

    }
    AUTH_CRIT_CONFIG_EXIT();

    if ((rc == VTSS_OK) && changed) {
        vtss_auth_conf_flash_write();
    }

    return rc;
}

/* ================================================================= *
 *  vtss_auth_mgmt_host_add()
 *  Add a host configuration
 * ================================================================= */
vtss_rc vtss_auth_mgmt_host_add(vtss_auth_proto_t proto, vtss_auth_host_conf_t *const conf)
{
    int     i;
    vtss_rc rc = VTSS_OK;

    VTSS_ASSERT(proto < VTSS_AUTH_PROTO_LAST);
    VTSS_ASSERT(conf);

    T_D("h: %s, au: %u, ac: %u, t: %u, r: %u, k: %s", conf->host, conf->auth_port, conf->acct_port, conf->timeout, conf->retransmit, conf->key);

    if (!msg_switch_is_master()) {
        return VTSS_AUTH_ERROR_MUST_BE_MASTER;
    }
    if (!conf->host[0]) {
        return VTSS_AUTH_ERROR_CFG_HOST;
    }
    if (conf->auth_port > 0xFFFF) {
        return VTSS_AUTH_ERROR_CFG_PORT;
    }
    if (conf->acct_port > 0xFFFF) {
        return VTSS_AUTH_ERROR_CFG_PORT;
    }
    if (conf->timeout && ((conf->timeout < VTSS_AUTH_TIMEOUT_MIN) || (conf->timeout > VTSS_AUTH_TIMEOUT_MAX))) {
        return VTSS_AUTH_ERROR_CFG_TIMEOUT;
    }
    if (conf->retransmit && ((conf->retransmit < VTSS_AUTH_RETRANSMIT_MIN) || (conf->timeout > VTSS_AUTH_RETRANSMIT_MAX))) {
        return VTSS_AUTH_ERROR_CFG_RETRANSMIT;
    }

    AUTH_CRIT_CONFIG_ENTER();
    for (i = 0; i < VTSS_AUTH_NUMBER_OF_SERVERS; i++) {
        vtss_auth_host_conf_t *hc = &config.proto[proto].host_conf[i];
        if (!(hc->host[0])) {
            *hc = *conf; // Entry was empty
            break;
        } else if (strcmp(hc->host, conf->host) == 0) {
            if ((hc->auth_port == conf->auth_port) && (hc->acct_port == conf->acct_port)) {
                *hc = *conf; // Entry matched
                break;
            } else if ((proto == VTSS_AUTH_PROTO_RADIUS) &&
                       ((hc->auth_port == conf->auth_port) || (hc->acct_port == conf->acct_port) ||
                        (hc->auth_port == conf->acct_port) || (hc->acct_port == conf->auth_port))) {
                rc = VTSS_AUTH_ERROR_CFG_HOST_PORT;
                break;
            }
        }
    }
    if (i == VTSS_AUTH_NUMBER_OF_SERVERS) {
        rc = VTSS_AUTH_ERROR_CFG_HOST_TABLE_FULL;
    }
    if (rc == VTSS_OK) {
        config_changed = TRUE;
#ifdef VTSS_SW_OPTION_RADIUS
        if (proto == VTSS_AUTH_PROTO_RADIUS) {
            rc = vtss_auth_radius_config_update(&config.radius, &config.proto[VTSS_AUTH_PROTO_RADIUS]);
        }
#endif /* VTSS_SW_OPTION_RADIUS */
    }
    AUTH_CRIT_CONFIG_EXIT();

    if (rc == VTSS_OK) {
        vtss_auth_conf_flash_write();
    }

    return rc;
}

/* ================================================================= *
 *  vtss_auth_mgmt_host_add()
 *  Delete a host configuration
 * ================================================================= */
vtss_rc vtss_auth_mgmt_host_del(vtss_auth_proto_t proto, vtss_auth_host_conf_t *const conf)
{
    int     i;
    vtss_rc rc = VTSS_OK;

    VTSS_ASSERT(proto < VTSS_AUTH_PROTO_LAST);
    VTSS_ASSERT(conf);

    T_D("h: %s, au: %u, ac: %u", conf->host, conf->auth_port, conf->acct_port);

    if (!msg_switch_is_master()) {
        return VTSS_AUTH_ERROR_MUST_BE_MASTER;
    }
    if (!conf->host[0]) {
        return VTSS_AUTH_ERROR_CFG_HOST;
    }

    AUTH_CRIT_CONFIG_ENTER();
    for (i = 0; i < VTSS_AUTH_NUMBER_OF_SERVERS; i++) {
        vtss_auth_host_conf_t *hc = &config.proto[proto].host_conf[i];
        if ((strcmp(hc->host, conf->host) == 0) && (hc->auth_port == conf->auth_port) && (hc->acct_port == conf->acct_port)) {
            int j;
            memset(&config.proto[proto].host_conf[i], 0, sizeof(config.proto[proto].host_conf[i])); // Delete entry
            for (j = i + 1; j < (VTSS_AUTH_NUMBER_OF_SERVERS); j++) { // Move all 1 up
                config.proto[proto].host_conf[j - 1] = config.proto[proto].host_conf[j];
                if (!config.proto[proto].host_conf[j].host[0]) {
                    break;
                } else {
                    memset(&config.proto[proto].host_conf[j], 0, sizeof(config.proto[proto].host_conf[j]));
                }
            }
            break;
        }
    }
    if (i == VTSS_AUTH_NUMBER_OF_SERVERS) {
        rc = VTSS_AUTH_ERROR_CFG_HOST_NOT_FOUND;
    }
    if (rc == VTSS_OK) {
        config_changed = TRUE;
#ifdef VTSS_SW_OPTION_RADIUS
        if (proto == VTSS_AUTH_PROTO_RADIUS) {
            rc = vtss_auth_radius_config_update(&config.radius, &config.proto[VTSS_AUTH_PROTO_RADIUS]);
        }
#endif /* VTSS_SW_OPTION_RADIUS */
    }
    AUTH_CRIT_CONFIG_EXIT();

    if (rc == VTSS_OK) {
        vtss_auth_conf_flash_write();
    }
    return rc;
}

/* ================================================================= *
 *  vtss_auth_mgmt_host_iterate()
 *  Iterates through a host configuration and calls a user specified
 *  callback function for each host.
 * ================================================================= */
vtss_rc vtss_auth_mgmt_host_iterate(vtss_auth_proto_t proto, const void *const contxt, const vtss_auth_host_cb_t cb, int *count)
{
    int     i;
    int     cnt = 0;

    VTSS_ASSERT(proto < VTSS_AUTH_PROTO_LAST);
    VTSS_ASSERT(cb);

    if (!msg_switch_is_master()) {
        return VTSS_AUTH_ERROR_MUST_BE_MASTER;
    }

    AUTH_CRIT_CONFIG_ENTER();
    for (i = 0; i < VTSS_AUTH_NUMBER_OF_SERVERS; i++) {
        if (config.proto[proto].host_conf[i].host[0]) {
            cb(proto, contxt, &config.proto[proto].host_conf[i], cnt);
            cnt++;
        } else {
            break;
        }
    }
    AUTH_CRIT_CONFIG_EXIT();
    if (count) {
        *count = cnt;
    }
    return VTSS_OK;
}

/* ================================================================= *
 *  vtss_auth_mgmt_httpd_cache_expire()
 *  Clear the HTTPD cache.
 * ================================================================= */
void vtss_auth_mgmt_httpd_cache_expire(void)
{
    vtss_auth_httpd_cache_expire();
}

/* ================================================================= *
 *  vtss_auth_init()
 *  Initialize the auth module
 * ================================================================= */
vtss_rc vtss_auth_init(vtss_init_data_t *data)
{
    vtss_isid_t     isid = data->isid;
    msg_rx_filter_t filter;

    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        T_N("INIT");
        critd_init(&crit_config, "auth.crit.config", VTSS_MODULE_ID_AUTH, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        AUTH_CRIT_CONFIG_EXIT();
        critd_init(&crit_cache,  "auth.crit.cache",  VTSS_MODULE_ID_AUTH, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        AUTH_CRIT_CACHE_EXIT();

        vtss_auth_client_pool_init();
#ifdef VTSS_SW_OPTION_SSH
        ssh_user_auth_register(vtss_auth_ssh_authenticate);
#endif

#ifdef VTSS_SW_OPTION_ICFG
        if (vtss_auth_icfg_init() != VTSS_RC_OK) {
            T_E("ICFG not initialized correctly");
        }
#endif

        /* Initialize thread data */
        memset(&thread, 0, sizeof(thread));
        cyg_mbox_create(&thread.mbox_handle, &thread.mbox);
        break;

    case INIT_CMD_START:
        T_N("START");
        /* Register for stack messages */
        memset(&filter, 0, sizeof(filter));
        filter.cb = vtss_auth_msg_rx;
        filter.modid = VTSS_MODULE_ID_AUTH;
        (void) msg_rx_filter_register(&filter);

        /* Create thread */
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          vtss_auth_thread,
                          0,
                          "Authentication",
                          thread.stack,
                          sizeof(thread.stack),
                          &thread.handle,
                          &thread.state);
        break;

    case INIT_CMD_CONF_DEF:
        T_N("CONF_DEF (isid = %d)", isid);
        if (isid == VTSS_ISID_GLOBAL) {
            vtss_auth_conf_flash_read(TRUE); /* Create default configuration */
            vtss_auth_httpd_cache_expire(); /* force the httpd cache to be expired */
        }
        break;

    case INIT_CMD_MASTER_UP:
        T_N("MASTER_UP");
        vtss_auth_conf_flash_read(FALSE); /* Read configuration */
        cyg_thread_resume(thread.handle); /* Resume thread now that config is valid */
        break;

    case INIT_CMD_SWITCH_ADD:
        T_N("SWITCH_ADD");
        vtss_auth_new_master(isid); /* Send new master notification */
        break;

    default:
        break;
    }

    return VTSS_OK;
}
