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
#include "main.h"
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "misc_api.h"
#include "packet_api.h"
#include "access_mgmt_api.h"
#include "access_mgmt.h"

#include <cyg/hal/basetype.h>

#if CYG_BYTEORDER == CYG_MSBFIRST
#define BYTE_ORDER BIG_ENDIAN
#else
#define BYTE_ORDER LITTLE_ENDIAN
#endif
#define LITTLE_ENDIAN   1234
#define BIG_ENDIAN  4321
#include <netinet/in.h>
#ifdef VTSS_SW_OPTION_IPV6
#include <netinet/ip6.h>
#endif
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include "ip2_api.h"
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
#include "vlan_api.h"
#endif /* VTSS_FEATURE_VLAN_PORT_V2 */

#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif

#include <network.h>

#ifdef VTSS_SW_OPTION_VCLI
#include "access_mgmt_cli.h"
#endif

#ifdef VTSS_SW_OPTION_ICFG
#include "access_mgmt_icfg.h"
#endif

#define VTSS_ALLOC_MODULE_ID    VTSS_MODULE_ID_ACCESS_MGMT

/* Registration for SNMP access management callbacks */
typedef enum {
    IP_STACK_GLUE_APPL_TYPE_NONE,
    IP_STACK_GLUE_APPL_TYPE_ALL         = 0x1,
    IP_STACK_GLUE_APPL_TYPE_SSH         = 0X2,  /* TCP 22 */
    IP_STACK_GLUE_APPL_TYPE_TELNET      = 0x4,  /* TCP 23 */
    IP_STACK_GLUE_APPL_TYPE_HTTP        = 0x8,  /* TCP 80 */
    IP_STACK_GLUE_APPL_TYPE_SNMP        = 0x10, /* UDP 161 */
    IP_STACK_GLUE_APPL_TYPE_HTTPS       = 0x20, /* TCP 443 */
    IP_STACK_GLUE_APPL_TYPE_ACCESS_MGMT = IP_STACK_GLUE_APPL_TYPE_HTTP | IP_STACK_GLUE_APPL_TYPE_HTTPS | IP_STACK_GLUE_APPL_TYPE_SNMP | IP_STACK_GLUE_APPL_TYPE_TELNET | IP_STACK_GLUE_APPL_TYPE_SSH
} ip_stack_glue_appl_type_t;

typedef struct {
    unsigned short  eth_type;
    unsigned char   ip_version;
    unsigned char   ip_protocol;
    unsigned long   sip;
    unsigned long   dip;
    unsigned short  sport;
    unsigned short  dport;
#ifdef VTSS_SW_OPTION_IPV6
    unsigned char   sip_v6[16];
    unsigned char   dip_v6[16];
#endif /* VTSS_SW_OPTION_IPV6 */
    ip_stack_glue_appl_type_t appl_type;
} ip_stack_glue_frame_info_t;

typedef BOOL (*ip_stack_glue_callback_t)(ip_stack_glue_frame_info_t *frame_info, const unsigned char *frame);

/* Access management statistics */
static u32 ACCESS_MGMT_http_receive_cnt     = 0, ACCESS_MGMT_http_discard_cnt   = 0;
static u32 ACCESS_MGMT_https_receive_cnt    = 0, ACCESS_MGMT_https_discard_cnt  = 0;
static u32 ACCESS_MGMT_snmp_receive_cnt     = 0, ACCESS_MGMT_snmp_discard_cnt   = 0;
static u32 ACCESS_MGMT_telnet_receive_cnt   = 0, ACCESS_MGMT_telnet_discard_cnt = 0;
static u32 ACCESS_MGMT_ssh_receive_cnt      = 0, ACCESS_MGMT_ssh_discard_cnt    = 0;

/* Callback function ptr */
static access_mgmt_filter_reject_callback_t ACCESS_MGMT_filter_reject_cb = NULL;


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static access_mgmt_global_t ACCESS_MGMT_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t ACCESS_MGMT_trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "accessmgmt",
    .descr     = "access management"
};

static vtss_trace_grp_t ACCESS_MGMT_trace_grps[TRACE_GRP_CNT] = {
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
#define ACCESS_MGMT_CRIT_ENTER() critd_enter(&ACCESS_MGMT_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define ACCESS_MGMT_CRIT_EXIT()  critd_exit(&ACCESS_MGMT_global.crit,  TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define ACCESS_MGMT_CRIT_ENTER() critd_enter(&ACCESS_MGMT_global.crit)
#define ACCESS_MGMT_CRIT_EXIT()  critd_exit(&ACCESS_MGMT_global.crit)
#endif /* VTSS_TRACE_ENABLED */


/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/
static int ACCESS_MGMT_register_ip_stack_glue_flag = 0;

/* Determine if access_mgmt configuration has changed */
static int ACCESS_MGMT_conf_changed(access_mgmt_conf_t *old, access_mgmt_conf_t *new)
{
    return (memcmp(new, old, sizeof(*new)));
}

/* Set access_mgmt defaults */
static void ACCESS_MGMT_default_set(access_mgmt_conf_t *conf)
{
    memset(conf, 0x0, sizeof(*conf));
    conf->mode = ACCESS_MGMT_DISABLED;
}

#ifdef VTSS_SW_OPTION_SYSLOG
/* access_mgmt service type text */
char *access_mgmt_service_type_txt(ip_stack_glue_appl_type_t service_type)
{
    char *txt;

    switch (service_type) {
    case IP_STACK_GLUE_APPL_TYPE_SSH:
        txt = "SSH";
        break;
    case IP_STACK_GLUE_APPL_TYPE_TELNET:
        txt = "TELNET";
        break;
    case IP_STACK_GLUE_APPL_TYPE_HTTP:
        txt = "HTTP";
        break;
    case IP_STACK_GLUE_APPL_TYPE_SNMP:
        txt = "SNMP";
        break;
    case IP_STACK_GLUE_APPL_TYPE_HTTPS:
        txt = "HTTPS";
        break;
    default:
        txt = "access_mgmt unknown service type";
        break;
    }
    return txt;
}
#endif /* VTSS_SW_OPTION_SYSLOG */

/* Callback function for IP stack glue layer
   Return 1: not allowed to hit TCP/IP stack
   Return 0: allowed to hit TCP/IP stack */
static BOOL ACCESS_MGMT_ip_stack_glue_callback(vtss_vid_t vid, ip_stack_glue_frame_info_t *frame_info)
{
    int access_id, service_type;
    access_mgmt_conf_t  conf;
#ifdef VTSS_SW_OPTION_SYSLOG
    char buf[40];
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t sip_v6;
#endif
#endif /* VTSS_SW_OPTION_SYSLOG */

#ifdef VTSS_SW_OPTION_IPV6
    if (frame_info->eth_type != 0x86DD && frame_info->eth_type != 0x0800) {
        return 0;
    }
#else
    if (frame_info->ip_version != 4 || frame_info->eth_type != 0x0800) {
        return 0;
    }
#endif /* VTSS_SW_OPTION_IPV6 */

    /* Check incoming frame is WEB, SNMP or Telnet */
    ACCESS_MGMT_CRIT_ENTER();
    if (frame_info->appl_type == IP_STACK_GLUE_APPL_TYPE_TELNET) {          /* TCP 23 - Telnet */
        service_type = ACCESS_MGMT_SERVICES_TYPE_TELNET;
        ACCESS_MGMT_telnet_receive_cnt++;
#ifdef VTSS_SW_OPTION_SSH
    } else if (frame_info->appl_type == IP_STACK_GLUE_APPL_TYPE_SSH) {      /* TCP 22 - SSH */
        service_type = ACCESS_MGMT_SERVICES_TYPE_TELNET;
        ACCESS_MGMT_ssh_receive_cnt++;
#endif /* VTSS_SW_OPTION_SSH */
    } else if (frame_info->appl_type == IP_STACK_GLUE_APPL_TYPE_HTTP) {     /* TCP 80 - WEB */
        service_type = ACCESS_MGMT_SERVICES_TYPE_WEB;
        ACCESS_MGMT_http_receive_cnt++;
#ifdef VTSS_SW_OPTION_HTTPS
    } else if (frame_info->appl_type == IP_STACK_GLUE_APPL_TYPE_HTTPS) {    /* TCP 443 - HTTPS */
        service_type = ACCESS_MGMT_SERVICES_TYPE_WEB;
        ACCESS_MGMT_https_receive_cnt++;
#endif /* VTSS_SW_OPTION_HTTPS */
    } else if (frame_info->appl_type == IP_STACK_GLUE_APPL_TYPE_SNMP) {   /* UDP 161 - SNMP */
        service_type = ACCESS_MGMT_SERVICES_TYPE_SNMP;
        ACCESS_MGMT_snmp_receive_cnt++;
    } else {
        ACCESS_MGMT_CRIT_EXIT();
        return 0;
    }
    ACCESS_MGMT_CRIT_EXIT();

    if (access_mgmt_conf_get(&conf) != VTSS_OK) {
        return 0;
    }

    if (conf.mode == 0) {
        return 0;
    }

    for (access_id = ACCESS_MGMT_ACCESS_ID_START; access_id < ACCESS_MGMT_ACCESS_ID_START + ACCESS_MGMT_MAX_ENTRIES; access_id++) {
        if (conf.entry[access_id].valid
            && (service_type & conf.entry[access_id].service_type)
            && conf.entry[access_id].vid == vid
           ) {
            if (frame_info->ip_version == 4) {
                if (conf.entry[access_id].entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV6) {
                    continue;
                }
                if ((frame_info->sip >= conf.entry[access_id].start_ip) && (frame_info->sip <= conf.entry[access_id].end_ip)) {
                    return 0;
                }
#ifndef VTSS_SW_OPTION_IPV6
            }
#else
            } else {
                if (conf.entry[access_id].entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4) {
                    continue;
                }
                if (memcmp(frame_info->sip_v6, &conf.entry[access_id].start_ipv6, sizeof(vtss_ipv6_t)) >= 0 && memcmp(frame_info->sip_v6, &conf.entry[access_id].end_ipv6, sizeof(vtss_ipv6_t)) <= 0) {
                    return 0;
                }
            }
#endif /* VTSS_SW_OPTION_IPV6 */
        }
    }

    /* Counter drop frame */
    ACCESS_MGMT_CRIT_ENTER();

    if (frame_info->appl_type == IP_STACK_GLUE_APPL_TYPE_TELNET)            /* TCP 23 - Telnet */
    {
        ACCESS_MGMT_telnet_discard_cnt++;
#ifdef VTSS_SW_OPTION_SSH
    } else if (frame_info->appl_type == IP_STACK_GLUE_APPL_TYPE_SSH)        /* TCP 22 - SSH */
    {
        ACCESS_MGMT_ssh_discard_cnt++;
#endif /* VTSS_SW_OPTION_SSH */
    } else if (frame_info->appl_type == IP_STACK_GLUE_APPL_TYPE_HTTP)       /* TCP 80 - WEB */
    {
        ACCESS_MGMT_http_discard_cnt++;
#ifdef VTSS_SW_OPTION_HTTPS
    } else if (frame_info->appl_type == IP_STACK_GLUE_APPL_TYPE_HTTPS)      /* TCP 443 - HTTPS */
    {
        ACCESS_MGMT_https_discard_cnt++;
#endif /* VTSS_SW_OPTION_HTTPS */
    } else if (frame_info->appl_type == IP_STACK_GLUE_APPL_TYPE_SNMP)       /* UDP 161 - SNMP */
    {
        ACCESS_MGMT_snmp_discard_cnt++;
    }

#ifdef VTSS_SW_OPTION_SYSLOG
#ifdef VTSS_SW_OPTION_IPV6
    if (frame_info->ip_version == 4)
    {
        S_I("Access management filter reject %s access from IP address %s.", access_mgmt_service_type_txt(frame_info->appl_type), misc_ipv4_txt(frame_info->sip, buf));
    } else
    {
        memcpy(sip_v6.addr, frame_info->sip_v6, 16);
        S_I("Access management filter reject %s access from IP address %s.", access_mgmt_service_type_txt(frame_info->appl_type), misc_ipv6_txt(&sip_v6, buf));
    }
#else
    S_I("Access management filter reject %s access from IP address %s.", access_mgmt_service_type_txt(frame_info->appl_type), misc_ipv4_txt(frame_info->sip, buf));
#endif /* VTSS_SW_OPTION_IPV6 */
#endif /* VTSS_SW_OPTION_SYSLOG */

    //filter reject callback
    if (ACCESS_MGMT_filter_reject_cb)
    {
        access_mgmt_filter_reject_info_t filter_reject_info;
        filter_reject_info.service_type = service_type;
        filter_reject_info.ip_addr      = frame_info->sip;
#ifdef VTSS_SW_OPTION_IPV6
        filter_reject_info.ip_version   = frame_info->ip_version;
        memcpy(filter_reject_info.ipv6_addr, frame_info->sip_v6, sizeof(filter_reject_info.ipv6_addr));
#endif /* VTSS_SW_OPTION_IPV6 */
        ACCESS_MGMT_filter_reject_cb(filter_reject_info);
    }

    ACCESS_MGMT_CRIT_EXIT();

    return 1;
}

// Warning, this function assumes that the data field is either an IPv4 or IPv6
// header, directly followed by a TCP or UDP header. This need not to be the
// case!
//
// TODO, consider IP fragmentation
//
// This function is an adaptor function to provide the same functionallity which
// ealier has been provided by the ip_stack_glue module.
#define FILTER_ALLOW 0
#define FILTER_DENY  1

int ip_stack_glue_adaptor_cb_function(vtss_vid_t vlan, unsigned length, const char *data)
{
    ip_stack_glue_frame_info_t frame_info;
    struct ip      *ip4_hdr;
#ifdef VTSS_SW_OPTION_IPV6
    struct ip6_hdr *ip6_hdr;
#endif /* VTSS_SW_OPTION_IPV6 */
    struct tcphdr  *tcp_hdr = NULL;
    struct udphdr  *udp_hdr = NULL;
    int            data_offset = 12;
    u16            *vlan_hdr = (u16 *)(data + data_offset);
    u16            *proto_type;

#if defined(VTSS_FEATURE_VLAN_PORT_V2)
    vtss_etype_t   tpid;

    if (vlan_mgmt_s_custom_etype_get(&tpid) != VTSS_OK) {
        return FILTER_ALLOW;
    }
#endif /* VTSS_FEATURE_VLAN_PORT_V2 */

    // vlan header
    if (ntohs(*vlan_hdr) == 0x88A8
#if defined(VTSS_FEATURE_VLAN_PORT_V2)
        || ntohs(*vlan_hdr) == tpid
#endif /* VTSS_FEATURE_VLAN_PORT_V2 */
       ) {
        data_offset += 8;
    } else if (ntohs(*vlan_hdr) == 0x8100) {
        data_offset += 4;
    }

    // protocol type
    proto_type = (u16 *)(data + data_offset);
    if (ntohs(*proto_type) != 0x0800
#ifdef VTSS_SW_OPTION_IPV6
        && ntohs(*proto_type) != 0x86DD
#endif /* VTSS_SW_OPTION_IPV6 */
       ) {
        return FILTER_ALLOW;
    }

    // ip header
    data_offset += 2;
    ip4_hdr = (struct ip *)(data + data_offset);
#ifdef VTSS_SW_OPTION_IPV6
    ip6_hdr = (struct ip6_hdr *)(data + data_offset);
#endif /* VTSS_SW_OPTION_IPV6 */

    // Check that we at least have a full IPv4 header
    if (length < (data_offset + sizeof(struct ip))) {
        return FILTER_ALLOW;
    }

    // Extract IP informations and setup poointer for next layer
    switch (ip4_hdr->ip_v) {
    case 4:
        // Ignore fragment frame
        if (ntohs(ip4_hdr->ip_off) & 0x1FFF) {
            return FILTER_ALLOW;
        }

        // Check that we have a full IPv4 and UDP/TCP header
        if ((ip4_hdr->ip_p == 6 && (length < sizeof(struct ip) + sizeof(struct tcphdr))) ||
            (ip4_hdr->ip_p == 17 && (length < sizeof(struct ip) + sizeof(struct udphdr)))) {
            return FILTER_ALLOW;
        }

        tcp_hdr = (struct tcphdr *)(data + data_offset + sizeof(struct ip));
        udp_hdr = (struct udphdr *)(data + data_offset + sizeof(struct ip));
        frame_info.eth_type = ETYPE_IPV4;
        frame_info.ip_version = ip4_hdr->ip_v;
        frame_info.ip_protocol = ip4_hdr->ip_p;
        frame_info.sip = ntohl(ip4_hdr->ip_src.s_addr);
        frame_info.dip = ntohl(ip4_hdr->ip_dst.s_addr);
        break;

#ifdef VTSS_SW_OPTION_IPV6
    case 6:
        // Check that we have a full IPv6 and UDP/TCP header
        if ((ip4_hdr->ip_p == 6 && (length < sizeof(struct ip6_hdr) + sizeof(struct tcphdr))) ||
            (ip4_hdr->ip_p == 17 && (length < sizeof(struct ip6_hdr) + sizeof(struct udphdr)))) {
            return FILTER_ALLOW;
        }

        tcp_hdr = (struct tcphdr *)(data + data_offset + sizeof(struct ip6_hdr));
        udp_hdr = (struct udphdr *)(data + data_offset + sizeof(struct ip6_hdr));
        frame_info.eth_type = ETYPE_IPV6;
        frame_info.ip_version = ip4_hdr->ip_v;
        frame_info.ip_protocol = ip6_hdr->ip6_ctlun.ip6_un1.ip6_un1_nxt;
        memcpy(frame_info.sip_v6,
               ip6_hdr->ip6_src.__u6_addr.__u6_addr8,
               sizeof(frame_info.sip_v6));
        memcpy(frame_info.dip_v6,
               ip6_hdr->ip6_dst.__u6_addr.__u6_addr8,
               sizeof(frame_info.dip_v6));
        break;
#endif /* VTSS_SW_OPTION_IPV6 */

    default:
        return FILTER_ALLOW;
    }

    // Extract the source and destination port from the TCP or UDP header
    switch (frame_info.ip_protocol) {
    case IP_PROTO_TCP:
        frame_info.sport = ntohs(tcp_hdr->th_sport);
        frame_info.dport = ntohs(tcp_hdr->th_dport);
        break;

    case IP_PROTO_UDP:
        frame_info.sport = ntohs(udp_hdr->uh_sport);
        frame_info.dport = ntohs(udp_hdr->uh_dport);
        break;

    default:
        return FILTER_ALLOW;
    }

    // Set the service bit in .appl_type
    if (frame_info.ip_protocol == IP_PROTO_TCP &&
        frame_info.dport == TCP_PROTO_SSH) {
        frame_info.appl_type = IP_STACK_GLUE_APPL_TYPE_SSH;

    } else if (frame_info.ip_protocol == IP_PROTO_TCP &&
               frame_info.dport == TCP_PROTO_TELNET) {
        frame_info.appl_type = IP_STACK_GLUE_APPL_TYPE_TELNET;

    } else if (frame_info.ip_protocol == IP_PROTO_TCP &&
               frame_info.dport == TCP_PROTO_HTTP) {
        frame_info.appl_type = IP_STACK_GLUE_APPL_TYPE_HTTP;

    } else if (frame_info.ip_protocol == IP_PROTO_UDP &&
               frame_info.dport == UDP_PROTO_SNMP) {
        frame_info.appl_type = IP_STACK_GLUE_APPL_TYPE_SNMP;

    } else if (frame_info.ip_protocol == IP_PROTO_TCP &&
               frame_info.dport == TCP_PROTO_HTTPS) {
        frame_info.appl_type = IP_STACK_GLUE_APPL_TYPE_HTTPS;
    } else {
        return FILTER_ALLOW; // Do not try to enforce access control on other
        // protocols than ssh,telnet,http(s) and snmp
    }

    // Call the original callback function
    if (!ACCESS_MGMT_ip_stack_glue_callback(vlan, &frame_info)) {
        return FILTER_ALLOW;
    }

    return FILTER_DENY;
}

static void ACCESS_MGMT_conf_apply(void)
{
    if (msg_switch_is_master()) {
        ACCESS_MGMT_CRIT_ENTER();
        if (ACCESS_MGMT_register_ip_stack_glue_flag == 0 && ACCESS_MGMT_global.access_mgmt_conf.mode) {
            /* Register for IP filtering callbacks */
            if (vtss_ip2_if_filter_reg(ip_stack_glue_adaptor_cb_function) == VTSS_RC_OK) {
                ACCESS_MGMT_register_ip_stack_glue_flag = 1;
            } else {
                T_E("Failed to registerer filter function");
            }
        } else if (ACCESS_MGMT_register_ip_stack_glue_flag == 1 && !ACCESS_MGMT_global.access_mgmt_conf.mode) {
            /* Unregister for IP filtering callbacks */
            if (vtss_ip2_if_filter_unreg(ip_stack_glue_adaptor_cb_function) != VTSS_RC_OK) {
                T_E("Failed to unregisterer filter function (we are not registered in the first place");
            }
            ACCESS_MGMT_register_ip_stack_glue_flag = 0;
        }
        ACCESS_MGMT_CRIT_EXIT();
    }
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* access_mgmt error text */
char *access_mgmt_error_txt(vtss_rc rc)
{
    char *txt;

    switch (rc) {
    case ACCESS_MGMT_ERROR_GEN:
        txt = "access_mgmt generic error";
        break;
    case ACCESS_MGMT_ERROR_PARM:
        txt = "access_mgmt parameter error";
        break;
    case ACCESS_MGMT_ERROR_STACK_STATE:
        txt = "access_mgmt stack state error";
        break;
    default:
        txt = "access_mgmt unknown error";
        break;
    }
    return txt;
}

/* Get access_mgmt configuration */
vtss_rc access_mgmt_conf_get(access_mgmt_conf_t *conf)
{
    T_D("enter");

    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ACCESS_MGMT_ERROR_STACK_STATE;
    }

    ACCESS_MGMT_CRIT_ENTER();

    *conf = ACCESS_MGMT_global.access_mgmt_conf;

    ACCESS_MGMT_CRIT_EXIT();
    T_D("exit");

    return VTSS_OK;
}

/* Set access_mgmt configuration */
vtss_rc access_mgmt_conf_set(access_mgmt_conf_t *conf)
{
    vtss_rc         rc      = VTSS_OK;
    int             changed = 0;

    T_D("enter");
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        rc = ACCESS_MGMT_ERROR_STACK_STATE;
    }
    ACCESS_MGMT_CRIT_ENTER();
    changed = ACCESS_MGMT_conf_changed(&ACCESS_MGMT_global.access_mgmt_conf, conf);
    ACCESS_MGMT_global.access_mgmt_conf = *conf;
    ACCESS_MGMT_CRIT_EXIT();

    if (changed) {
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */

        conf_blk_id_t          blk_id = CONF_BLK_ACCESS_MGMT_CONF;
        access_mgmt_conf_blk_t *access_mgmt_conf_blk_p;

        if ((access_mgmt_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open access_mgmt table");
        } else {
            access_mgmt_conf_blk_p->access_mgmt_conf = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif
        ACCESS_MGMT_conf_apply();
    }

    T_D("exit");
    return rc;
}


/****************************************************************************/
/*  Configuration silent upgrade                                            */
/****************************************************************************/

typedef struct {
    BOOL        valid;
    u32         service_type;
    u32         entry_type;
    vtss_ipv4_t start_ip;
    vtss_ipv4_t end_ip;
#ifdef VTSS_SW_OPTION_IPV6
    vtss_ipv6_t start_ipv6;
    vtss_ipv6_t end_ipv6;
#endif /* VTSS_SW_OPTION_IPV6 */
} access_mgmt_entry_v1_t;

typedef struct {
    u32                     mode;
    u32                     entry_num;
    access_mgmt_entry_v1_t  entry[ACCESS_MGMT_ACCESS_ID_START + ACCESS_MGMT_MAX_ENTRIES];
} access_mgmt_conf_v1_t;

typedef struct {
    u32                     version;            /* Block version */
    access_mgmt_conf_v1_t   access_mgmt_conf;   /* Access management configuration */
} access_mgmt_conf_blk_v1_t;

/* Silent upgrade from old configuration to new one.
 * Returns a (malloc'ed) pointer to the upgraded new configuration
 * or NULL if conversion failed.
 */
static access_mgmt_conf_blk_t *ACCESS_MGMT_conf_flash_silent_upgrade(const void *blk, u32 old_ver, u32 new_ver)
{
    access_mgmt_conf_blk_t *new_blk = NULL;

    if (old_ver == 1 && new_ver == 2) {
        if ((new_blk = VTSS_MALLOC(sizeof(*new_blk)))) {
            access_mgmt_conf_blk_v1_t   *old_blk = (access_mgmt_conf_blk_v1_t *)blk;
            int                         access_idx;
            vtss_if_id_vlan_t if_id = 1;

            (void) vtss_ip2_if_id_next(VTSS_VID_NULL, &if_id);

            /* upgrade configuration from v1 to v2 */
            ACCESS_MGMT_default_set(&new_blk->access_mgmt_conf); // Initiate1 with default values
            new_blk->version    = ACCESS_MGMT_CONF_BLK_VERSION;
            new_blk->access_mgmt_conf.mode       = old_blk->access_mgmt_conf.mode;
            new_blk->access_mgmt_conf.entry_num  = old_blk->access_mgmt_conf.entry_num;
            for (access_idx = ACCESS_MGMT_ACCESS_ID_START; access_idx < ACCESS_MGMT_ACCESS_ID_START + ACCESS_MGMT_MAX_ENTRIES; access_idx++) {
                memcpy(&new_blk->access_mgmt_conf.entry[access_idx], &old_blk->access_mgmt_conf.entry[access_idx], sizeof(access_mgmt_entry_v1_t));
                new_blk->access_mgmt_conf.entry[access_idx].vid = if_id;
            }
        }
    }

    return new_blk;
}


/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create access_mgmt stack configuration */
static void ACCESS_MGMT_conf_read_stack(BOOL create)
{
    int                     changed;
    BOOL                    do_create = create;
    u32                     size;
    access_mgmt_conf_t      *old_access_mgmt_conf_p, new_access_mgmt_conf;
    access_mgmt_conf_blk_t  *conf_blk_p;
    conf_blk_id_t           blk_id;
    u32                     blk_version;

    T_D("enter, create: %d", create);

    blk_id = CONF_BLK_ACCESS_MGMT_CONF;
    blk_version = ACCESS_MGMT_CONF_BLK_VERSION;

    if (misc_conf_read_use()) {
        /* Read/create access_mgmt configuration */
        if ((conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, &size)) == NULL) {
            T_W("conf_sec_open failed, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = TRUE;
        } else if (conf_blk_p->version < blk_version) {
            access_mgmt_conf_blk_t *new_blk;
            T_I("version upgrade, run silent upgrade");
            new_blk = ACCESS_MGMT_conf_flash_silent_upgrade(conf_blk_p, conf_blk_p->version, blk_version);
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            if (conf_blk_p && new_blk) {
                T_I("upgrade ok");
                *conf_blk_p = *new_blk;
            } else {
                T_W("upgrade failed, creating defaults");
                do_create = TRUE;
            }
            if (new_blk) {
                VTSS_FREE(new_blk);
            }
        } else if (size != sizeof(*conf_blk_p)) {
            T_W("size mismatch, creating defaults");
            conf_blk_p = conf_sec_create(CONF_SEC_GLOBAL, blk_id, sizeof(*conf_blk_p));
            do_create = TRUE;
        } else if (conf_blk_p->version != blk_version) {
            T_D("version mismatch, creating defaults");
            do_create = 1;
        }
    } else {
        conf_blk_p = NULL;
        do_create  = TRUE;
    }

    changed = 0;
    ACCESS_MGMT_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        ACCESS_MGMT_default_set(&new_access_mgmt_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->access_mgmt_conf = new_access_mgmt_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p) {
            new_access_mgmt_conf = conf_blk_p->access_mgmt_conf;
        }
    }
    old_access_mgmt_conf_p = &ACCESS_MGMT_global.access_mgmt_conf;
    if (ACCESS_MGMT_conf_changed(old_access_mgmt_conf_p, &new_access_mgmt_conf)) {
        changed = 1;
    }
    ACCESS_MGMT_global.access_mgmt_conf = new_access_mgmt_conf;

    if (ACCESS_MGMT_register_ip_stack_glue_flag == 0 && ACCESS_MGMT_global.access_mgmt_conf.mode) {
        /* Register for IP filtering callbacks */
        if (vtss_ip2_if_filter_reg(ip_stack_glue_adaptor_cb_function) == VTSS_RC_OK) {
            ACCESS_MGMT_register_ip_stack_glue_flag = 1;
        } else {
            T_E("Failed to registerer filter function");
        }
    } else if (ACCESS_MGMT_register_ip_stack_glue_flag == 1 && !ACCESS_MGMT_global.access_mgmt_conf.mode) {
        /* Unregister for IP filtering callbacks */
        if (vtss_ip2_if_filter_unreg(ip_stack_glue_adaptor_cb_function) != VTSS_RC_OK) {
            T_E("Failed to unregisterer filter function (we are not registered in the first place");
        }

        ACCESS_MGMT_register_ip_stack_glue_flag = 0;
    }

    ACCESS_MGMT_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open access_mgmt table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif

    if (changed && create) {
        ACCESS_MGMT_conf_apply();
    }

    T_D("exit");
}

/* Module start */
static void ACCESS_MGMT_start(void)
{
    access_mgmt_conf_t  *conf_p;

    T_D("enter");

    /* Initialize access_mgmt configuration */
    conf_p = &ACCESS_MGMT_global.access_mgmt_conf;
    ACCESS_MGMT_default_set(conf_p);

    /* Create semaphore for critical regions */
    critd_init(&ACCESS_MGMT_global.crit, "ACCESS_MGMT_global.crit", VTSS_MODULE_ID_ACCESS_MGMT, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
    ACCESS_MGMT_CRIT_EXIT();

    T_D("exit");
}

/* Initialize module */
vtss_rc access_mgmt_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
#ifdef VTSS_SW_OPTION_ICFG
    vtss_rc     rc;
#endif

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&ACCESS_MGMT_trace_reg, ACCESS_MGMT_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&ACCESS_MGMT_trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
        ACCESS_MGMT_start();
#ifdef VTSS_SW_OPTION_VCLI
        access_mgmt_cli_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        rc = access_mgmt_icfg_init();
        if (rc != VTSS_OK) {
            T_D("fail to init icfg registration, rc = %s", error_txt(rc));
        }
#endif
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
            ACCESS_MGMT_conf_read_stack(1);
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");

        /* Read stack and switch configuration */
        ACCESS_MGMT_conf_read_stack(0);
        access_mgmt_stats_clear();
        break;
    }
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        /* Apply all configuration to switch */
        if (msg_switch_is_local(isid)) {
            ACCESS_MGMT_conf_apply();
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

/* Get access management entry */
vtss_rc access_mgmt_entry_get(int access_id, access_mgmt_entry_t *entry)
{
    vtss_rc rc;
    access_mgmt_conf_t conf;

    T_D("enter");

    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ACCESS_MGMT_ERROR_STACK_STATE;
    }

    /* Check illegal parameter */
    if (access_id < ACCESS_MGMT_ACCESS_ID_START || access_id >= ACCESS_MGMT_ACCESS_ID_START + ACCESS_MGMT_MAX_ENTRIES) {
        T_D("exit");
        return ACCESS_MGMT_ERROR_PARM;
    }

    if ((rc = access_mgmt_conf_get(&conf)) != VTSS_OK) {
        T_D("exit");
        return rc;
    }

    *entry = conf.entry[access_id];

    T_D("exit");
    return rc;
}

/* Add access management entry
   valid access_id: ACCESS_MGMT_ACCESS_ID_START ~ ACCESS_MGMT_MAX_ENTRIES */
vtss_rc access_mgmt_entry_add(int access_id, access_mgmt_entry_t *entry)
{
    vtss_rc rc;
    access_mgmt_conf_t conf;

    T_D("enter");

    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ACCESS_MGMT_ERROR_STACK_STATE;
    }

    /* Check illegal parameter */
    if (access_id < ACCESS_MGMT_ACCESS_ID_START || access_id >= ACCESS_MGMT_ACCESS_ID_START + ACCESS_MGMT_MAX_ENTRIES) {
        T_D("exit");
        return ACCESS_MGMT_ERROR_PARM;
    }

    if (entry->service_type == 0 || entry->service_type & (~ACCESS_MGMT_SERVICES_TYPE)) {
        T_D("exit");
        return ACCESS_MGMT_ERROR_PARM;
    }

    if (entry->entry_type != ACCESS_MGMT_ENTRY_TYPE_IPV4 && entry->entry_type != ACCESS_MGMT_ENTRY_TYPE_IPV6) {
        return ACCESS_MGMT_ERROR_PARM;
    }

    if (entry->entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4) {
        if (entry->start_ip > entry->end_ip) {
            T_D("exit");
            return ACCESS_MGMT_ERROR_PARM;
        }
    }
#ifdef VTSS_SW_OPTION_IPV6
    else if (entry->entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV6) {
        if (memcmp(&entry->start_ipv6, &entry->end_ipv6, sizeof(vtss_ipv6_t)) > 0) {
            T_D("exit");
            return ACCESS_MGMT_ERROR_PARM;
        }
    }
#endif /* VTSS_SW_OPTION_IPV6 */
    else {
        T_D("exit");
        return ACCESS_MGMT_ERROR_PARM;
    }

    if ((rc = access_mgmt_conf_get(&conf)) != VTSS_OK) {
        T_D("exit");
        return rc;
    }

    if (!conf.entry[access_id].valid) {
        conf.entry_num++;
    }

    conf.entry[access_id] = *entry;
    conf.entry[access_id].valid = 1;
    rc = access_mgmt_conf_set(&conf);

    T_D("exit");
    return rc;
}

/* Delete access management entry */
vtss_rc access_mgmt_entry_del(int access_id)
{
    vtss_rc rc;
    access_mgmt_conf_t conf;

    T_D("enter");

    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ACCESS_MGMT_ERROR_STACK_STATE;
    }

    /* Check illegal parameter */
    if (access_id < ACCESS_MGMT_ACCESS_ID_START || access_id >= ACCESS_MGMT_ACCESS_ID_START + ACCESS_MGMT_MAX_ENTRIES) {
        T_D("exit");
        return ACCESS_MGMT_ERROR_PARM;
    }

    if ((rc = access_mgmt_conf_get(&conf)) != VTSS_OK) {
        T_D("exit");
        return rc;
    }

    if (conf.entry_num > 0) {
        conf.entry_num--;
    }

    memset(&conf.entry[access_id], 0x0, sizeof(conf.entry[access_id]));

    rc = access_mgmt_conf_set(&conf);

    T_D("exit");
    return rc;
}


/* Clear access management entry */
vtss_rc access_mgmt_entry_clear(void)
{
    vtss_rc rc = VTSS_OK;
    access_mgmt_conf_t conf, old_conf;

    T_D("enter");

    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return ACCESS_MGMT_ERROR_STACK_STATE;
    }

    if ((rc = access_mgmt_conf_get(&old_conf)) != VTSS_OK) {
        T_D("exit");
        return rc;
    }

    memset(&conf, 0x0, sizeof(conf));
    conf.mode = old_conf.mode;
    rc = access_mgmt_conf_set(&conf);

    T_D("exit");
    return rc;
}

/* Get access management statistics */
void access_mgmt_stats_get(access_mgmt_stats_t *stats)
{
    T_D("enter");

    ACCESS_MGMT_CRIT_ENTER();

    stats->http_receive_cnt     = ACCESS_MGMT_http_receive_cnt;
    stats->http_discard_cnt     = ACCESS_MGMT_http_discard_cnt;
    stats->https_receive_cnt    = ACCESS_MGMT_https_receive_cnt;
    stats->https_discard_cnt    = ACCESS_MGMT_https_discard_cnt;
    stats->snmp_receive_cnt     = ACCESS_MGMT_snmp_receive_cnt;
    stats->snmp_discard_cnt     = ACCESS_MGMT_snmp_discard_cnt;
    stats->telnet_receive_cnt   = ACCESS_MGMT_telnet_receive_cnt;
    stats->telnet_discard_cnt   = ACCESS_MGMT_telnet_discard_cnt;
    stats->ssh_receive_cnt      = ACCESS_MGMT_ssh_receive_cnt;
    stats->ssh_discard_cnt      = ACCESS_MGMT_ssh_discard_cnt;

    ACCESS_MGMT_CRIT_EXIT();
    T_D("exit");
}

/* Clear access management statistics */
void access_mgmt_stats_clear(void)
{
    T_D("enter");

    ACCESS_MGMT_CRIT_ENTER();

    ACCESS_MGMT_http_receive_cnt    = 0;
    ACCESS_MGMT_http_discard_cnt    = 0;
    ACCESS_MGMT_https_receive_cnt   = 0;
    ACCESS_MGMT_https_discard_cnt   = 0;
    ACCESS_MGMT_snmp_receive_cnt    = 0;
    ACCESS_MGMT_snmp_discard_cnt    = 0;
    ACCESS_MGMT_telnet_receive_cnt  = 0;
    ACCESS_MGMT_telnet_discard_cnt  = 0;
    ACCESS_MGMT_ssh_receive_cnt     = 0;
    ACCESS_MGMT_ssh_discard_cnt     = 0;

    ACCESS_MGMT_CRIT_EXIT();
    T_D("exit");
}


/****************************************************************************/
/*  Receive register functions                                              */
/****************************************************************************/

/* Register access management filter reject callback function */
void access_mgmt_filter_reject_register(access_mgmt_filter_reject_callback_t cb)
{
    T_D("enter");
    ACCESS_MGMT_CRIT_ENTER();
    if (ACCESS_MGMT_filter_reject_cb) {
        ACCESS_MGMT_CRIT_EXIT();
        T_D("exit");
        return;
    }
    ACCESS_MGMT_filter_reject_cb = cb;
    ACCESS_MGMT_CRIT_EXIT();
    T_D("exit");
}

/* Unregister access management filter reject callback function */
void access_mgmt_filter_reject_unregister(void)
{
    T_D("enter");
    ACCESS_MGMT_CRIT_ENTER();
    if (!ACCESS_MGMT_filter_reject_cb) {
        ACCESS_MGMT_CRIT_EXIT();
        T_D("exit");
        return;
    }
    ACCESS_MGMT_filter_reject_cb = NULL;
    ACCESS_MGMT_CRIT_EXIT();
    T_D("exit");
}

/* Check if entry content is the same as others
   Retrun: 0 - no duplicated, others - duplicated access_id */
int access_mgmt_entry_content_is_duplicated(int access_id, access_mgmt_entry_t *entry)
{
    access_mgmt_conf_t *conf;
    int access_idx, found = 0;

    ACCESS_MGMT_CRIT_ENTER();
    conf = &ACCESS_MGMT_global.access_mgmt_conf;
    for (access_idx = ACCESS_MGMT_ACCESS_ID_START; access_idx < ACCESS_MGMT_ACCESS_ID_START + ACCESS_MGMT_MAX_ENTRIES; access_idx++) {
        if (access_idx == access_id || !conf->entry[access_idx].valid) {
            continue;
        }

        if (conf->entry[access_idx].entry_type == ACCESS_MGMT_ENTRY_TYPE_IPV4) {
            if ((conf->entry[access_idx].start_ip == entry->start_ip) &&
                (conf->entry[access_idx].end_ip == entry->end_ip)) {
                found = access_idx;
                break;
            }
#ifdef VTSS_SW_OPTION_IPV6
        } else {
            if (!memcmp(&conf->entry[access_idx].start_ipv6, &entry->start_ipv6, sizeof(vtss_ipv6_t)) &&
                !memcmp(&conf->entry[access_idx].end_ipv6, &entry->end_ipv6, sizeof(vtss_ipv6_t))) {
                found = access_idx;
                break;
            }
#endif /* VTSS_SW_OPTION_IPV6 */
        }
    }
    ACCESS_MGMT_CRIT_EXIT();

    if (found) {
        return access_idx;
    }
    return 0;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
