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

#ifdef VTSS_SW_OPTION_IP2
#ifndef VTSS_SW_OPTION_NTP
#ifdef VTSS_SW_OPTION_SNTP

#include "main.h"
#include "conf_api.h"
#include "misc_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "cli_api.h"
#include "mgmt_api.h"
#include "port_api.h"

#ifdef VTSS_SW_OPTION_SYSUTIL
#include "sysutil_api.h"
#endif

#ifdef VTSS_SW_OPTION_SNMP
#include "vtss_snmp_api.h"
#endif

#include <sys/param.h>
#include <sys/sysctl.h>
#undef _KERNEL
#include <sys/socket.h>
#include <sys/ioctl.h>
#define _KERNEL

#include <net/route.h>
#include <network.h>

#include <cyg/sntp/sntp.h>
#include <net/netdb.h>

#ifdef VTSS_SW_OPTION_IPV6
#define _KERNEL
#include <net/if.h>
#include <net/route.h>
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <net/if_var.h>
#include <netinet/in.h>
#include <netinet/ip_var.h>
#include <netinet6/in6_var.h>
#include <netinet6/in6_ifattach.h>
#include <netinet/ip6.h>
#include <netinet6/ip6_var.h>
#include <netinet6/nd6.h>
#include <netinet/icmp6.h>
#include <netinet6/scope6_var.h>
#else
#define _KERNEL
#include <net/if_var.h>
#endif /* VTSS_SW_OPTION_IPV6 */

#include "vtss_sntp.h"

#ifdef VTSS_SW_OPTION_ICFG
#include "vtss_sntp_icfg.h"
#endif /* VTSS_SW_OPTION_ICFG */


#if 0
//#include <dhcp.h>

//#include <netinet6/in6_var.h>

//#if defined(IP_MGMT_USING_ACL_ENTRIES) || defined(IP_MGMT_USING_MAC_ACL_ENTRIES)
//#include "acl_api.h"
//#endif

//#if VTSS_SWITCH_STACKABLE
//#include "topo_api.h"
//#endif

//#ifdef VTSS_SW_OPTION_LLDP
//#include "lldp_api.h"
//#endif

//#ifdef VTSS_SW_OPTION_SSH
//#include "vtss_ssh_api.h"
//#endif

//#ifdef VTSS_SW_OPTION_UPNP
//#include "vtss_upnp_api.h"
//#endif
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_SNTP

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/
static sntp_global_t sntp_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "sntp",
    .descr     = "SNTP"
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
#define SNTP_CRIT_ENTER() critd_enter(&sntp_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define SNTP_CRIT_EXIT()  critd_exit( &sntp_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define SNTP_CRIT_ENTER() critd_enter(&sntp_global.crit)
#define SNTP_CRIT_EXIT()  critd_exit( &sntp_global.crit)
#endif /* VTSS_TRACE_ENABLED */

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* sntp, Set system defaults */
static void _sntp_default_set(sntp_conf_t *conf)
{
    conf->mode = FALSE;
    memset(conf->sntp_server, 0, sizeof(uchar) * VTSS_SYS_HOSTNAME_LEN);

    return;
}

/* determine if sntp configuration has changed */
static int _sntp_conf_changed(sntp_conf_t *old, sntp_conf_t *new)
{
    return (memcmp(new, old, sizeof(*new)));
}

/* Set configuration to sntp kernel */
static void sntp_kernel_set(sntp_conf_t *conf)
{
    sntp_server_list_t  sntp_srv_rst_list[2];
    cyg_uint32          num_servers;
#ifdef VTSS_SW_OPTION_IPV6
#if 0
    u8                  ip6_scop;
    vtss_ipv6_t         ip6_addr;
    struct sockaddr_in6 *addr6;
    BOOL                try_next_time;
    int                 idx;
#endif
#endif /* VTSS_SW_OPTION_IPV6 */

    T_D("enter");

    num_servers = 0;
    memset(&sntp_srv_rst_list, 0, sizeof(sntp_srv_rst_list));

    if (conf->mode) {
        if (conf->ip_type == SNTP_IP_TYPE_IPV4) {
            sntp_srv_rst_list[num_servers].ip_sntp_server.sa_family = AF_INET;
            strncpy((char *)sntp_srv_rst_list[num_servers].ip_sntp_server_string, (char *)conf->sntp_server, sizeof(conf->sntp_server));
            num_servers++;
#ifdef VTSS_SW_OPTION_IPV6
#warning Unsupported configuration SNTP+IPv6!
#if 0
        } else if (conf->ip_type == SNTP_IP_TYPE_IPV6) {

            try_next_time = TRUE;
            if (_ipv6_linklocal_addr_get(&ip6_scop, &ip6_addr)) {
                try_next_time = FALSE;
            }

            memset(&ip6_addr, 0x0, sizeof(vtss_ipv6_t));
            if (!try_next_time && memcmp(&conf->ipv6_addr, &ip6_addr, sizeof(vtss_ipv6_t))) {

                addr6 = (struct sockaddr_in6 *)&sntp_srv_rst_list[num_servers].ip_sntp_server;
                addr6->sin6_len = sizeof(*addr6);
                addr6->sin6_family = AF_INET6;

                /* It is tricky. For using link-local addresses, the index of interface must
                 * be assigned explictly. Here is 2 for eth0. In our system, 1 is for lo.
                 * Refer to the function of scope6_check_id in
                 * eCos/packages/net/bsd_tcpip/current/src/sys/netinet6/scope6.c
                */
                addr6->sin6_scope_id = ip6_scop;
                for (idx = 0; idx < 16; idx++) {
                    addr6->sin6_addr.__u6_addr.__u6_addr8[idx] = conf->ipv6_addr.addr[idx];
                }

                num_servers++;
            }
#endif
#endif
        }
    }

    if (num_servers != 0) {
        cyg_sntp_set_servers(sntp_srv_rst_list, num_servers);
        T_D("SNTP: %d server%s", num_servers, (num_servers > 1) ? "s" : "");

#ifdef VTSS_SW_OPTION_SNMP
#ifdef SNMP_SUPPORT_V3
        snmpv3_mgmt_ntp_post_conf();
#endif /* SNMP_SUPPORT_V3 */
#endif /* VTSS_SW_OPTION_SNMP */

    } else {
        cyg_sntp_set_servers(NULL, 0);
        T_D("SNTP: No server");
    }

    T_D("exit");
    return;
}

/****************************************************************************/
/*  Various global functions                                                */
/****************************************************************************/

/* Get sntp configuration */
vtss_rc sntp_config_get(sntp_conf_t *conf)
{
    T_D("enter");

    SNTP_CRIT_ENTER();
    *conf = sntp_global.system_conf;
    SNTP_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Set sntp configuration */
vtss_rc sntp_config_set(sntp_conf_t *conf)
{
    vtss_rc rc      = VTSS_OK;
    int     changed = FALSE;

    T_D("enter");

    SNTP_CRIT_ENTER();
    if (msg_switch_is_master()) {
        changed = _sntp_conf_changed(&sntp_global.system_conf, conf);
        sntp_global.system_conf = *conf;
    } else {
        T_W("not master");
        rc = VTSS_UNSPECIFIED_ERROR;
    }
    SNTP_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        conf_blk_id_t        blk_id  = CONF_BLK_SNTP_CONF;
        vtss_sntp_conf_blk_t *conf_blk;
        /* Save changed configuration */
        if ((conf_blk = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_E("failed to open configuration table");
        } else {
            conf_blk->conf = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
        /* Apply sntp to system */
        sntp_kernel_set(conf);
    }

    T_D("exit");
    return rc;
}

/* Set sntp defaults */
void vtss_sntp_default_set(sntp_conf_t *conf)
{
    T_D("enter");

    SNTP_CRIT_ENTER();

    _sntp_default_set(conf);

    SNTP_CRIT_EXIT();

    T_D("exit");
    return;
}

/****************************************************************************/
/*  Configuration silent upgrade                                            */
/****************************************************************************/

#define IP_CONF_REF_VERSION         3
#define VTSS_SYS_HOSTNAME_REF_LEN   46

/* IP configuration */
typedef struct {
    /* Global (Per-System) Management Section - BEGIN */
    /* IPv4 default router */
    vtss_ipv4_t ipv4_router;
    /* DHCP client */
    BOOL        dhcp;

#ifdef VTSS_SW_OPTION_IPV6
    /* IPv6 default router */
    vtss_ipv6_t ipv6_router;
    /* Steteless */
    BOOL        ipv6_autoconfig;
    ulong       ipv6_autoconfig_fallback_time;
    /* Steteful */
#endif /* VTSS_SW_OPTION_IPV6 */
    /* Global(Per-System) Management Section - END */

    /* Per-Interface Management Section - BEGIN */
    vtss_ipv4_t ipv4_addr;      /* IPv4 address */
    vtss_ipv4_t ipv4_mask;      /* IPv4 mask */

#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t ipv6_addr;      /* IPv6 address */
    ulong       ipv6_prefix;    /* IPv6 prefix length */
#endif /* VTSS_SW_OPTION_IPV6 */

    vtss_vid_t  vid;            /* VLAN ID: LL-Info */
    /* Per-Interface Management Section - END */

    /* IP-Misc. Management Section - BEGIN */
    /* IPv4 SNTP server name */
    uchar       ipv4_sntp_string[VTSS_SYS_HOSTNAME_REF_LEN];

#ifdef VTSS_SW_OPTION_DNS
    /* DNS server */
    vtss_ipv4_t ipv4_dns;
#endif /* VTSS_SW_OPTION_DNS */

#ifdef VTSS_SW_OPTION_IPV6
    /* IPv6 SNTP server address */
    vtss_ipv6_t ipv6_sntp;
#endif /* VTSS_SW_OPTION_IPV6 */
    /* IP-Misc. Management Section - END */
} ip_conf_v3_t;

typedef struct {
    ulong               version;    /* Block Version */
    ip_conf_v3_t        conf;       /* Configuration */
} ip_conf_blk_v3_t;


/* Silent upgrade from old configuration to new one.
 * Returns a (malloc'ed) pointer to the upgraded new configuration
 * or NULL if conversion failed.
 */
static vtss_sntp_conf_blk_t *sntp_conf_flash_silent_upgrade(const void *blk, u32 old_ver)
{
    vtss_sntp_conf_blk_t *new_blk = NULL;

    ip_conf_blk_v3_t    *ref_ip_cont_blk;
    ip_conf_v3_t        *ref_ip_conf;

    if ((new_blk = VTSS_MALLOC(sizeof(*new_blk)))) {

        /* Use default values */
        _sntp_default_set(&new_blk->conf);

        ref_ip_conf = NULL;
        if ((ref_ip_cont_blk = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_IP_CONF, NULL)) != NULL) {
            if (ref_ip_cont_blk->version == IP_CONF_REF_VERSION) {
                ref_ip_conf = &ref_ip_cont_blk->conf;
            }

            conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_IP_CONF);
        }

        if (ref_ip_conf) {
            new_blk->conf.ip_type = SNTP_IP_TYPE_IPV4;
            new_blk->version = SNTP_CONF_BLK_VERSION2;
            if (strlen(ref_ip_conf->ipv4_sntp_string) > 0) {
                new_blk->conf.mode = TRUE;
                memcpy(new_blk->conf.sntp_server, ref_ip_conf->ipv4_sntp_string, sizeof(new_blk->conf.sntp_server));
            } else {
                new_blk->conf.mode = FALSE;
                memset(&new_blk->conf.sntp_server, 0x0, sizeof(new_blk->conf.sntp_server));
            }
        }
    }

    return new_blk;
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/
#if 0
/* Read/create system switch configuration */
static vtss_rc sntp_conf_read_switch(vtss_isid_t isid_add)
{
    BOOL                    do_create;
    ulong                   size;
    vtss_isid_t             isid;
    sntp_conf_t             *conf_p, new_conf;
    vtss_sntp_conf_blk_t    *conf_blk_p;
    conf_blk_id_t           blk_id      = CONF_BLK_SNTP_CONF;
    ulong                   blk_version = SNTP_CONF_BLK_VERSION2;
    vtss_rc                 rc = VTSS_OK;

    T_D("enter, isid_add: %d", isid_add);

    if (misc_conf_read_use()) {
        /* read configuration */
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL ||
            size != sizeof(*conf_blk_p)) {
            T_W("conf_sec_open failed or size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = TRUE;
        } else if (conf_blk_p->version != blk_version) {
            T_W("version mismatch, creating defaults");
            do_create = TRUE;
        } else {
            do_create = (isid_add != VTSS_ISID_GLOBAL);
        }
    } else {
        conf_blk_p = NULL;
        do_create  = TRUE;
    }

    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if (isid_add != VTSS_ISID_GLOBAL && isid_add != isid) {
            continue;
        }

        SNTP_CRIT_ENTER();
        if (do_create) {
            /* Use default values */
            _sntp_default_set(&new_conf);
            if (conf_blk_p != NULL) {
                conf_blk_p->conf = new_conf;
            }
        } else {
            /* Use new configuration */
            if (conf_blk_p != NULL) {  // Quiet lint
                new_conf = conf_blk_p->conf;
            }
        }

        conf_p = &sntp_global.system_conf;
        *conf_p = new_conf;
        SNTP_CRIT_EXIT();

    }

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open system table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    T_D("exit");
    return rc;
}
#endif

/* Read/create system stack configuration */
static vtss_rc sntp_conf_read_stack(BOOL create)
{
    int                     changed;
    BOOL                    do_create;
    ulong                   size;
    sntp_conf_t             *old_conf_p, new_conf;
    vtss_sntp_conf_blk_t    *conf_blk_p;
    conf_blk_id_t           blk_id;
    ulong                   blk_version;
    vtss_rc                 rc = VTSS_OK;

    T_D("enter, create: %d", create);

    blk_id      = CONF_BLK_SNTP_CONF;
    blk_version = SNTP_CONF_BLK_VERSION2;

    if (misc_conf_read_use()) {
        /* Read/create configuration */
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) {
            vtss_sntp_conf_blk_t *new_blk;

            T_I("conf_sec_open failed, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));

            T_I("version upgrade, run silent upgrade, %lu", conf_blk_p->version);
            new_blk = sntp_conf_flash_silent_upgrade(conf_blk_p, conf_blk_p->version);

            if (new_blk && conf_blk_p) {
                *conf_blk_p = *new_blk;
                VTSS_FREE(new_blk);
                do_create = FALSE;
            } else {
                T_W("upgrade failed, creating defaults");
                do_create = TRUE;
            }
        } else if (size != sizeof(*conf_blk_p)) {
            T_W("size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = TRUE;
        } else {
            do_create = create;
        }
    } else {
        conf_blk_p = NULL;
        do_create  = TRUE;
    }

    changed = 0;
    SNTP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        _sntp_default_set(&new_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->conf = new_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {  // Quiet lint
            new_conf = conf_blk_p->conf;
        }
    }
    old_conf_p = &sntp_global.system_conf;
    if (_sntp_conf_changed(old_conf_p, &new_conf)) {
        changed = TRUE;
    }

    sntp_global.system_conf = new_conf;
    SNTP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open configuration table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed) {
        /* Apply configuration to system */
        sntp_kernel_set(&new_conf);
        T_D("Apply configuration to system");
    }

    T_D("exit");
    return rc;
}

/* Module start */
static void vtss_sntp_start(BOOL init)
{
    sntp_conf_t       *conf_p;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize configuration */
        conf_p = &sntp_global.system_conf;
        _sntp_default_set(conf_p);

        /* Create semaphore for critical regions */
        critd_init(&sntp_global.crit, "sntp.crit", VTSS_MODULE_ID_SNTP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        SNTP_CRIT_EXIT();

    } else {

        /* start sntp client on ecos */
        cyg_sntp_set_servers(NULL, 0);

    }

    T_D("exit");
    return;
}

/* Initialize module */
vtss_rc vtss_sntp_init(vtss_init_data_t *data)
{
    vtss_rc         rc = VTSS_OK;
    vtss_isid_t     isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_I("INIT");
        vtss_sntp_start(TRUE);
#ifdef VTSS_SW_OPTION_ICFG
        if (vtss_sntp_icfg_init() != VTSS_OK) {
            T_D("Calling vtss_sntp_icfg_init() failed!");
        }
#endif
        break;

    case INIT_CMD_START:
        /* Register for stack messages */
        T_I("START");
        vtss_sntp_start(FALSE);
        break;

    case INIT_CMD_CONF_DEF:
        T_I("CONF_DEF, isid: %u", isid);

        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            rc = sntp_conf_read_stack(TRUE);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
            //rc = sntp_conf_read_switch(isid);
        }
        break;

    case INIT_CMD_MASTER_UP:
        T_I("MASTER_UP");
        /* Read stack and switch configuration */
        if ((rc = sntp_conf_read_stack(FALSE)) != VTSS_OK) {
            return rc;
        }
        //rc = sntp_conf_read_switch(VTSS_ISID_GLOBAL);
        break;

    case INIT_CMD_MASTER_DOWN:
        T_I("MASTER_DOWN");
        break;

    case INIT_CMD_SWITCH_ADD:
        T_I("SWITCH_ADD, isid: %u", isid);
        break;

    case INIT_CMD_SWITCH_DEL:
        T_I("SWITCH_DEL, isid: %u", isid);
        break;

    default:
        break;
    }

    T_D("exit");
    return VTSS_OK;
}

#endif
#endif
#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

