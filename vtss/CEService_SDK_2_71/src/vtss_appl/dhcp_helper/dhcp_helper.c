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

#define _DHCP_HELPER_USER_NAME_C_

#include "main.h"
#include "conf_api.h"
#include "msg_api.h"
#include "critd_api.h"
#include "vtss_ecos_mutex_api.h"
#include "misc_api.h"
#include "ip2_api.h"
#include "packet_api.h"
#include "dhcp_helper_api.h"
#include "dhcp_helper.h"
#include "acl_api.h"
#include "port_api.h"
#include "vlan_api.h"
#include "syslog_api.h"
#include "aggr_api.h"
#include "mac_api.h"
#include "vtss_bip_buffer_api.h"
#if defined(VTSS_FEATURE_VSTAX_V2) && defined(VTSS_SWITCH_STACKABLE) && VTSS_SWITCH_STACKABLE
#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()
#endif /* VTSS_SWITCH_STACKABLE */

#include <network.h>
#include <dhcp.h>
#include <netinet/udp.h>

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_DHCP_HELPER

/* DHCP port number */
#define DHCP_SERVER_UDP_PORT    67
#define DHCP_CLINET_UDP_PORT    68

#define DHCP_OPT_SUBNET_MASK_ADDRESS    1
#define DHCP_OPT_ROUTE                  3
#define DHCP_OPT_DOMAIN_NAME_SERVER     6
#define DHCP_OPT_REQUESTED_ADDRESS      50
#define DHCP_OPT_LEASE_TIME             51
#define DHCP_OPT_SERVER_ID              54

#ifndef IP_VHL_HL
#define IP_VHL_HL(vhl)      ((vhl) & 0x0f)
#endif /* IP_VHL_HL */


static void DHCP_HELPER_rx_filter_register(BOOL registerd);


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static dhcp_helper_global_t DHCP_HELPER_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t DHCP_HELPER_trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "dhcphelper",
    .descr     = "DHCP helper, processing all DHCP received packets"
};

static vtss_trace_grp_t DHCP_HELPER_trace_grps[TRACE_GRP_CNT] = {
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
#define DHCP_HELPER_CRIT_ENTER()             critd_enter(        &DHCP_HELPER_global.crit,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define DHCP_HELPER_CRIT_EXIT()              critd_exit(         &DHCP_HELPER_global.crit,     TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define DHCP_HELPER_CRIT_ASSERT_LOCKED()     critd_assert_locked(&DHCP_HELPER_global.crit,     TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#define DHCP_HELPER_BIP_CRIT_ENTER()         critd_enter(        &DHCP_HELPER_global.bip_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define DHCP_HELPER_BIP_CRIT_EXIT()          critd_exit(         &DHCP_HELPER_global.bip_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define DHCP_HELPER_BIP_CRIT_ASSERT_LOCKED() critd_assert_locked(&DHCP_HELPER_global.bip_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else
#define DHCP_HELPER_CRIT_ENTER()             critd_enter(        &DHCP_HELPER_global.crit)
#define DHCP_HELPER_CRIT_EXIT()              critd_exit(         &DHCP_HELPER_global.crit)
#define DHCP_HELPER_CRIT_ASSERT_LOCKED()     critd_assert_locked(&DHCP_HELPER_global.crit)
#define DHCP_HELPER_BIP_CRIT_ENTER()         critd_enter(        &DHCP_HELPER_global.bip_crit)
#define DHCP_HELPER_BIP_CRIT_EXIT()          critd_exit(         &DHCP_HELPER_global.bip_crit)
#define DHCP_HELPER_BIP_CRIT_ASSERT_LOCKED() critd_assert_locked(&DHCP_HELPER_global.bip_crit)
#endif /* VTSS_TRACE_ENABLED */

/* Thread variables */
#define DHCP_HELPER_CERT_THREAD_STACK_SIZE       8192
static cyg_handle_t DHCP_HELPER_thread_handle;
static cyg_thread   DHCP_HELPER_thread_block;
static char         DHCP_HELPER_thread_stack[DHCP_HELPER_CERT_THREAD_STACK_SIZE];

/* BIP buffer Thread variables */
static cyg_handle_t DHCP_HELPER_bip_buffer_thread_handle;
static cyg_thread   DHCP_HELPER_bip_buffer_thread_block;
static char         DHCP_HELPER_bip_buffer_thread_stack[DHCP_HELPER_CERT_THREAD_STACK_SIZE];

/* BIP buffer event declaration */
#define DHCP_HELPER_EVENT_PKT_RX        0x00000001
#define DHCP_HELPER_EVENT_PKT_TX        0x00000002
#define DHCP_HELPER_EVENT_ANY           (DHCP_HELPER_EVENT_PKT_RX | DHCP_HELPER_EVENT_PKT_TX) /* Any possible bit */
static cyg_flag_t   DHCP_HELPER_bip_buffer_thread_events;

/* BIP buffer data declaration */
#define DHCP_HELPER_BIP_BUF_PKT_SIZE    1520  /* 4-byte aligned */
#define DHCP_HELPER_BIP_BUF_CNT         DHCP_HELPER_FRAME_INFO_MAX_CNT

typedef struct {
    unsigned char      pkt[DHCP_HELPER_BIP_BUF_PKT_SIZE]; // Used in tx and rx
    size_t             len;                               // Used in tx and rx
    BOOL               is_rx;                             // Used in tx and rx
    vtss_vid_t         vid;                               // Used in tx and rx
    vtss_isid_t        src_isid;                          // Used in tx and rx
    vtss_port_no_t     src_port_no;                       // Used in tx and rx
    vtss_glag_no_t     src_glag_no;                       // Used in tx and rx
    vtss_isid_t        dst_isid;                          // Used in tx
    u64                dst_port_mask;                     // Used in tx
    dhcp_helper_user_t user;                              // Used in tx
    void               *bufref_p;                         // Used in tx
} dhcp_helper_bip_buf_t;

// Make each entry take a multiple of 4 bytes.
#define DHCP_HELPER_BIP_BUF_ALIGNED_SIZE (4 * ((sizeof(dhcp_helper_bip_buf_t) + 3) / 4))

/* BIP buffer parameters variables */
#define DHCP_HELPER_BIP_BUF_TOTAL_SIZE          (DHCP_HELPER_BIP_BUF_CNT * DHCP_HELPER_BIP_BUF_ALIGNED_SIZE)
static vtss_bip_buffer_t DHCP_HELPER_bip_buf;

/* Record incoming frame information */
#define DHCP_HELPER_FRAME_INFO_MONITOR_INTERVAL     5000    // Unit is msec., 5 seconds
#define DHCP_HELPER_FRAME_INFO_OBSOLETED_TIMEOUT    18000   // Unit is sytem time ticket, 180 seconds (3 minutes)

struct dhcp_helper_frame_info_list {
    struct dhcp_helper_frame_info_list  *next;
    dhcp_helper_frame_info_t            info;
} *dhcp_helper_frame_info;

/* packet rx filter */
static packet_rx_filter_t  DHCP_HELPER_rx_filter;
static void                 *DHCP_HELPER_filter_id = NULL; //Filter id for subscribing dhcp packet.

/* Callback function for the dynamic IP address obtained */
static dhcp_helper_frame_info_callback_t DHCP_HELPER_ip_addr_obtained_cb = NULL, DHCP_HELPER_release_ip_addr_cb = NULL;

/* Callback function when clear all DHCP Helper deatiled statistics
   We need to clear user's local statistics too */
static dhcp_helper_user_clear_local_stat_callback_t DHCP_HELPER_clear_local_stat_cb[DHCP_HELPER_USER_CNT];

/* Callback function when receive DHCP frame */
static dhcp_helper_stack_rx_callback_t DHCP_HELPER_rx_cb[DHCP_HELPER_USER_CNT];

/****************************************************************************/
/*  Frame information maintain functions                                    */
/****************************************************************************/

/* Register Register dynmaic IP address obtained
   Callback after the IP assigned action is completed */
void dhcp_helper_notify_ip_addr_obtained_register(dhcp_helper_frame_info_callback_t cb)
{
    DHCP_HELPER_CRIT_ENTER();
    if (DHCP_HELPER_ip_addr_obtained_cb) {
        DHCP_HELPER_CRIT_EXIT();
        return;
    }

    DHCP_HELPER_ip_addr_obtained_cb = cb;
    DHCP_HELPER_CRIT_EXIT();
}

/* Register dynmaic IP address released
   Callback when receive a DHCP release frame */
void dhcp_helper_release_ip_addr_register(dhcp_helper_frame_info_callback_t cb)
{
    DHCP_HELPER_CRIT_ENTER();
    if (DHCP_HELPER_release_ip_addr_cb) {
        DHCP_HELPER_CRIT_EXIT();
        return;
    }

    DHCP_HELPER_release_ip_addr_cb = cb;
    DHCP_HELPER_CRIT_EXIT();
}

/* Get DHCP frame infomation */
static void DHCP_HELPER_frame_info_parse(u8 *frame_p, size_t len, dhcp_helper_frame_info_t *info)
{
    struct ip       *ip = (struct ip *)(frame_p + 14);
    int             ip_header_len = IP_VHL_HL(ip->ip_hl) << 2;
    struct bootp    bp;
    u32             requested_addr, subnet_mask, dhcp_server_addr, router_addr, dns_addr, lease_time = 0;
    u8 *op, *nextop, *sp, *max;

    /* NULL po*/
    if (!frame_p || !info) {
        return;
    }

    /* Get DHCP message type, chaddr and transaction ID */
    memcpy(&bp, frame_p + 14 + ip_header_len + 8, sizeof(bp)); /* 14:DA+SA+ETYPE, 8:udp header length */

    max = (u8 *)(frame_p + len);
    sp = op = &bp.bp_vend[4];

    while (op < max) {
        if (*op == DHCP_OPT_SUBNET_MASK_ADDRESS) {
            memcpy(&subnet_mask, &op[2], 4);
            info->assigned_mask = ntohl(subnet_mask);
            T_D("[assigned_mask]");
        } else if (*op == DHCP_OPT_ROUTE) {
            memcpy(&router_addr, &op[2], 4);
            info->gateway_ip = ntohl(router_addr);
            T_D("[gateway_ip]");
        } else if (*op == DHCP_OPT_DOMAIN_NAME_SERVER) {
            memcpy(&dns_addr, &op[2], 4);
            info->dns_server_ip = ntohl(dns_addr);
            T_D("[assigned_ip]");
        } else if (*op == DHCP_OPT_REQUESTED_ADDRESS) {
            memcpy(&requested_addr, &op[2], 4);
            info->assigned_ip = ntohl(requested_addr);
            T_D("[assigned_ip]");
        } else if (*op == DHCP_OPT_LEASE_TIME) {
            memcpy(&lease_time, &op[2], 4);
            info->lease_time = ntohl(lease_time);
            T_D("[lease_time]");
        } else if (*op == DHCP_OPT_SERVER_ID) {
            memcpy(&dhcp_server_addr, &op[2], 4);
            info->dhcp_server_ip = ntohl(dhcp_server_addr);
            T_D("[server_ip]");
        } else if (*op == 0x0) {
            if (sp != op) {
                *sp = *op;
            }
            ++op;
            ++sp;
            T_D("[0x0 continue]");
            continue;
        } else if (*op == 0xFF) {  /* Quit immediately if we hit an End option. */
            if (sp != op) {
                *sp++ = *op++;
            }
            T_D("[0xff break]");
            break;
        }

        nextop = op + op[1] + 2;
        if (nextop > max) {
            T_D("[nextop > max]");
            return;
        }
        if (sp != op) {
            memmove(sp, op, op[1] + 2);
            sp += op[1] + 2;
            op = nextop;
        } else {
            op = sp = nextop;
        }
        continue;
    }
}

/* Clear DHCP helper frame information entry */
static void DHCP_HELPER_frame_info_clear_all(void)
{
    struct dhcp_helper_frame_info_list *sp;

    DHCP_HELPER_CRIT_ENTER();
    sp = dhcp_helper_frame_info;
    while (sp) {
        dhcp_helper_frame_info = sp->next;
        VTSS_FREE(sp);
        sp = dhcp_helper_frame_info;
    }
    DHCP_HELPER_global.frame_info_cnt = 0;
    DHCP_HELPER_CRIT_EXIT();
}

/* Clear DHCP helper frame information obsoleted entry */
static void DHCP_HELPER_frame_info_clear_obsoleted(void)
{
    struct dhcp_helper_frame_info_list  *sp, *prev_sp = NULL;
    u32                                 timestamp = cyg_current_time();
    DHCP_HELPER_CRIT_ENTER();
    sp = dhcp_helper_frame_info;

    /* clear entries by mac and also clear time expired entries */
    while (sp) {
        if (timestamp - sp->info.timestamp > DHCP_HELPER_FRAME_INFO_OBSOLETED_TIMEOUT) {
            if (prev_sp) {
                prev_sp->next = sp->next;
                VTSS_FREE(sp);
                sp = prev_sp->next;
            } else {
                dhcp_helper_frame_info = sp->next;
                VTSS_FREE(sp);
                sp = dhcp_helper_frame_info;
            }
            if (DHCP_HELPER_global.frame_info_cnt) {
                DHCP_HELPER_global.frame_info_cnt--;
            }
            continue;
        }
        prev_sp = sp;
        sp = sp->next;
    }
    DHCP_HELPER_CRIT_EXIT();
}

/* Add DHCP helper frame information entry.
   Return TRUE when the IP assigned action is completed.
   Otherwise, retrun FALSE.
 */
static BOOL DHCP_HELPER_frame_info_add(dhcp_helper_frame_info_t *info)
{
    struct dhcp_helper_frame_info_list  *sp, *search_sp;
    BOOL                                found = FALSE;

    if (info->op_code != DHCP_HELPER_MSG_TYPE_DISCOVER &&
        info->op_code != DHCP_HELPER_MSG_TYPE_OFFER &&
        info->op_code != DHCP_HELPER_MSG_TYPE_REQUEST &&
        info->op_code != DHCP_HELPER_MSG_TYPE_ACK) {
        return FALSE;
    }

    DHCP_HELPER_CRIT_ENTER();
    /* search entry exist? */
    for (search_sp = dhcp_helper_frame_info; search_sp; search_sp = search_sp->next) {
        if (!memcmp(search_sp->info.mac, info->mac, 6) &&
            search_sp->info.vid == info->vid &&
            search_sp->info.transaction_id == info->transaction_id) {
            found = TRUE;
            /* found it, update content */
            if ((search_sp->info.op_code == DHCP_HELPER_MSG_TYPE_DISCOVER && info->op_code == DHCP_HELPER_MSG_TYPE_OFFER) ||
                (search_sp->info.op_code == DHCP_HELPER_MSG_TYPE_OFFER && info->op_code == DHCP_HELPER_MSG_TYPE_REQUEST) ||
                (search_sp->info.op_code == DHCP_HELPER_MSG_TYPE_REQUEST && info->op_code == DHCP_HELPER_MSG_TYPE_ACK)) {
                search_sp->info.op_code = info->op_code;
                if (info->op_code == DHCP_HELPER_MSG_TYPE_REQUEST ||
                    info->op_code == DHCP_HELPER_MSG_TYPE_ACK) {
                    if (info->assigned_ip) {
                        search_sp->info.assigned_ip = info->assigned_ip;
                    }
                    if (info->assigned_mask) {
                        search_sp->info.assigned_mask = info->assigned_mask;
                    }
                    if (info->dhcp_server_ip) {
                        search_sp->info.dhcp_server_ip = info->dhcp_server_ip;
                    }
                    if (info->gateway_ip) {
                        search_sp->info.gateway_ip = info->gateway_ip;
                    }
                    if (info->dns_server_ip) {
                        search_sp->info.dns_server_ip = info->dns_server_ip;
                    }
                    if (info->lease_time) {
                        search_sp->info.lease_time = info->lease_time;
                    }
                    search_sp->info.timestamp = cyg_current_time();
                    search_sp->info.local_dhcp_server = info->local_dhcp_server;
                }
            }

            if (info->op_code == DHCP_HELPER_MSG_TYPE_ACK &&
                search_sp->info.assigned_ip &&
                search_sp->info.assigned_mask &&
                search_sp->info.lease_time) {
#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
                if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING] && DHCP_HELPER_ip_addr_obtained_cb) {
                    /* Notice snooping module the IP assigned action is completed. */
                    T_D("[notice snooping]");
                    DHCP_HELPER_ip_addr_obtained_cb(&search_sp->info);
                }
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */
            }
            break;
        }
    }

    /* Not found, create a new one */
    if (!found &&
        (info->op_code == DHCP_HELPER_MSG_TYPE_DISCOVER || info->op_code == DHCP_HELPER_MSG_TYPE_REQUEST)) {
        if (DHCP_HELPER_global.frame_info_cnt > DHCP_HELPER_FRAME_INFO_MAX_CNT) {
            T_D("Reach the DHCP Helper frame information the maximum count");
        } else if ((sp = (struct dhcp_helper_frame_info_list *)VTSS_MALLOC(sizeof(*sp))) == NULL) {
            T_W("Unable to allocate %zu bytes for DHCP frame information", sizeof(*sp));
        } else {
            sp->info = *info;
            sp->info.timestamp = cyg_current_time();
            sp->next = dhcp_helper_frame_info;
            dhcp_helper_frame_info = sp;
            DHCP_HELPER_global.frame_info_cnt++;
        }
    }

    DHCP_HELPER_CRIT_EXIT();

    return FALSE;
}

/* Delete DHCP helper frame information entry */
static void DHCP_HELPER_frame_info_del(dhcp_helper_frame_info_t *info)
{
    struct dhcp_helper_frame_info_list *sp, *prev_sp = NULL;

    DHCP_HELPER_CRIT_ENTER();
    for (sp = dhcp_helper_frame_info; sp; sp = sp->next) {
        if (!memcmp(sp->info.mac, info->mac, sizeof(sp->info.mac)) &&
            sp->info.vid == info->vid &&
            sp->info.transaction_id == info->transaction_id) {
            if (prev_sp == NULL) {
                dhcp_helper_frame_info = sp->next;
            } else {
                prev_sp->next = sp->next;
            }
            VTSS_FREE(sp);
            if (DHCP_HELPER_global.frame_info_cnt) {
                DHCP_HELPER_global.frame_info_cnt--;
            }
            break;
        }
        prev_sp = sp;
    }
    DHCP_HELPER_CRIT_EXIT();
}

/* Delete DHCP helper frame information entry by MAC port */
static void DHCP_HELPER_frame_info_del_by_port(vtss_isid_t isid, vtss_port_no_t port_no)
{
    struct dhcp_helper_frame_info_list *sp, *prev_sp = NULL;

    DHCP_HELPER_CRIT_ENTER();
    sp = dhcp_helper_frame_info;

    /* clear entries by port */
    while (sp) {
        if (sp->info.isid == isid && sp->info.port_no == port_no) {
            if (prev_sp) {
                prev_sp->next = sp->next;
                VTSS_FREE(sp);
                sp = prev_sp->next;
            } else {
                dhcp_helper_frame_info = sp->next;
                VTSS_FREE(sp);
                sp = dhcp_helper_frame_info;
            }
            if (DHCP_HELPER_global.frame_info_cnt) {
                DHCP_HELPER_global.frame_info_cnt--;
            }
            continue;
        }
        prev_sp = sp;
        sp = sp->next;
    }
    DHCP_HELPER_CRIT_EXIT();
}

/* Clear DHCP helper frame information other uncompleted entry.
   The API is called after the IP assigned action is completed. */
static void DHCP_HELPER_frame_info_clear(dhcp_helper_frame_info_t *info, BOOL include_myself)
{
    struct dhcp_helper_frame_info_list *sp, *prev_sp = NULL;

    DHCP_HELPER_CRIT_ENTER();
    sp = dhcp_helper_frame_info;

    /* clear entries by mac */
    while (sp) {
        if (!memcmp(sp->info.mac, info->mac, sizeof(sp->info.mac)) &&
            sp->info.vid == info->vid &&
            (!include_myself || (include_myself && sp->info.transaction_id != info->transaction_id))) {
            if (prev_sp) {
                prev_sp->next = sp->next;
                VTSS_FREE(sp);
                sp = prev_sp->next;
            } else {
                dhcp_helper_frame_info = sp->next;
                VTSS_FREE(sp);
                sp = dhcp_helper_frame_info;
            }
            if (DHCP_HELPER_global.frame_info_cnt) {
                DHCP_HELPER_global.frame_info_cnt--;
            }
            continue;
        }
        prev_sp = sp;
        sp = sp->next;
    }
    DHCP_HELPER_CRIT_EXIT();
}

/* Lookup DHCP helper frame information entry */
BOOL dhcp_helper_frame_info_lookup(u8 *mac, vtss_vid_t vid, uint transaction_id, dhcp_helper_frame_info_t *info)
{
    struct dhcp_helper_frame_info_list *sp;

    DHCP_HELPER_CRIT_ENTER();
    sp = dhcp_helper_frame_info;
    while (sp) {
        if (!memcmp(sp->info.mac, mac, 6) &&
            (vid == 0 || (vid && sp->info.vid == vid)) &&
            sp->info.transaction_id == transaction_id) {
            *info = sp->info;
            DHCP_HELPER_CRIT_EXIT();
            return TRUE;
        }
        sp = sp->next;
    }

    DHCP_HELPER_CRIT_EXIT();
    return FALSE;
}

/* Getnext DHCP helper frame information entry */
BOOL dhcp_helper_frame_info_getnext(u8 *mac, vtss_vid_t vid, uint transaction_id, dhcp_helper_frame_info_t *info)
{
    u8 null_mac[6] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

    DHCP_HELPER_CRIT_ENTER();
    if (!dhcp_helper_frame_info) {
        DHCP_HELPER_CRIT_EXIT();
        return FALSE;
    }
    DHCP_HELPER_CRIT_EXIT();

    if (!memcmp(null_mac, mac, 6) && vid == 0 && transaction_id == 0) { // Getfirst
        DHCP_HELPER_CRIT_ENTER();
        *info = dhcp_helper_frame_info->info;
        DHCP_HELPER_CRIT_EXIT();
        return TRUE;
    } else {
        struct dhcp_helper_frame_info_list  *sp;
        int                                 found = 0;

        DHCP_HELPER_CRIT_ENTER();
        sp = dhcp_helper_frame_info;
        while (sp) {
            if (!memcmp(sp->info.mac, mac, 6) && sp->info.vid == vid && sp->info.transaction_id == transaction_id) {
                found = 1;
                break;
            }
            sp = sp->next;
        }
        DHCP_HELPER_CRIT_EXIT();

        if (found && sp && sp->next) {
            *info = sp->next->info;
            return TRUE;
        } else {
            return FALSE;
        }
    }
}


/****************************************************************************/
/*  Callback functions                                                      */
/****************************************************************************/

static void DHCP_HELPER_port_global_change_callback(vtss_isid_t    isid,
                                                    vtss_port_no_t port_no,
                                                    port_info_t    *info)
{
    /* Clear releated frame information entries when port status is link-down */
    if (!info->link) {
        DHCP_HELPER_frame_info_del_by_port(isid, port_no);
    }
}

/* Do IP checksum */
static u16 DHCP_HELPER_do_ip_chksum(u16 ip_hdr_len, u16 ip_hdr[])
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
static u16 DHCP_HELPER_do_udp_chksum(u16 udp_len, u16 *src_addr, u16 *dest_addr, u16 *udp_hdr)
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

/*  Get output destination portmask.
*/
static u64 DHCP_HELPER_dst_port_mask_get(vtss_vid_t vid, vtss_isid_t dst_isid, vtss_isid_t src_isid, vtss_port_no_t src_port_no, vtss_glag_no_t src_glag_no, u8 dhcp_message)
{
    port_iter_t                 pit;
    port_status_t               port_status;
    vlan_mgmt_entry_t           vlan_conf;
    aggr_mgmt_group_no_t        aggr_group_no = 0;
    u64                         dst_port_mask = 0;

    T_D("enter, vid=%d, dst_isid=%d, src_isid=%d, src_port_no=%d src_glag_no=%d", vid, dst_isid, src_isid, src_port_no, src_glag_no);

    if (!msg_switch_is_master() || !msg_switch_exists(dst_isid)) {
        return dst_port_mask;
    }

    if (VTSS_ISID_LEGAL(src_isid) && src_port_no < VTSS_PORT_NO_END) {
        aggr_group_no = aggr_mgmt_get_aggr_id(src_isid, src_port_no);
    }
    if (vlan_mgmt_vlan_get(dst_isid, vid, &vlan_conf, FALSE, VLAN_USER_ALL) == VTSS_OK) {
        (void) port_iter_init(&pit, NULL, dst_isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
        while (port_iter_getnext(&pit)) {
            if (vlan_conf.ports[pit.iport] == 0 ||
                (src_isid != VTSS_ISID_GLOBAL && (dst_isid == src_isid && src_port_no == pit.iport))) { //Filter source port
                continue;
            }

            if (port_mgmt_status_get(dst_isid, pit.iport, &port_status) == VTSS_OK && port_status.status.link) {
                if (aggr_group_no && (aggr_group_no == aggr_mgmt_get_aggr_id(dst_isid, pit.iport))) {   //Filter same aggregation ID
                    continue;
                }

#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
                /* Filter untrusted ports */
                DHCP_HELPER_CRIT_ENTER();
                if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING] && !DHCP_HELPER_MSG_FROM_SERVER(dhcp_message)) {
                    if (DHCP_HELPER_global.port_conf[dst_isid].port_mode[pit.iport] == DHCP_HELPER_PORT_MODE_UNTRUSTED) {
                        T_D("Filter untrusted ports: %d/%d", dst_isid, pit.iport);
                        DHCP_HELPER_CRIT_EXIT();
                        continue;
                    }
                }
                DHCP_HELPER_CRIT_EXIT();
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */

                dst_port_mask |= VTSS_BIT64(pit.iport);
            }
        }
    }

    T_D("exit");
    return dst_port_mask;
}

void DHCP_HELPER_stats_add(dhcp_helper_user_t user, vtss_isid_t isid, u64 dst_port_mask, u8 dhcp_message, dhcp_helper_direction_t dhcp_message_direction)
{
    vtss_port_no_t  port_idx;

    if (user >= DHCP_HELPER_USER_CNT) {
        return;
    }

    if (!msg_switch_is_master()) {
        isid = VTSS_ISID_LOCAL;
    }

    DHCP_HELPER_CRIT_ENTER();
    /* Filter destination ports */
    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
        if (VTSS_EXTRACT_BITFIELD64(dst_port_mask, port_idx,  1) == 0) {
            continue;
        }

        if (dhcp_message_direction == DHCP_HELPER_DIRECTION_RX) {
            switch (dhcp_message) {
            case DHCP_HELPER_MSG_TYPE_DISCOVER:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.discover_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_OFFER:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.offer_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_REQUEST:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.request_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_DECLINE:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.decline_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_ACK:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.ack_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_NAK:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.nak_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_RELEASE:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.release_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_INFORM:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.inform_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_LEASEQUERY:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.leasequery_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_LEASEUNASSIGNED:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.leaseunassigned_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_LEASEUNKNOWN:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.leaseunknown_rx++;
                break;
            case DHCP_HELPER_MSG_TYPE_LEASEACTIVE:
                DHCP_HELPER_global.stats[user][isid][port_idx].rx_stats.leaseactive_rx++;
                break;
            }
        } else if (dhcp_message_direction == DHCP_HELPER_DIRECTION_TX) {
            switch (dhcp_message) {
            case DHCP_HELPER_MSG_TYPE_DISCOVER:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.discover_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_OFFER:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.offer_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_REQUEST:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.request_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_DECLINE:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.decline_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_ACK:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.ack_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_NAK:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.nak_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_RELEASE:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.release_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_INFORM:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.inform_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_LEASEQUERY:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.leasequery_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_LEASEUNASSIGNED:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.leaseunassigned_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_LEASEUNKNOWN:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.leaseunknown_tx++;
                break;
            case DHCP_HELPER_MSG_TYPE_LEASEACTIVE:
                DHCP_HELPER_global.stats[user][isid][port_idx].tx_stats.leaseactive_tx++;
                break;
            }
        }
    }
    DHCP_HELPER_CRIT_EXIT();

#if 0 // Don't use anymore
//#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
    /* When DHCP snooping mode is enabled, the DHCP Snooping statistics will be increased. */
    if (user != DHCP_HELPER_USER_SNOOPING) {
        DHCP_HELPER_stats_add(DHCP_HELPER_USER_SNOOPING, isid, dst_port_mask, dhcp_message, dhcp_message_direction);
    }
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */
}

/* Check DIP is myself interfaces */
static BOOL DHCP_HELPER_is_myself_interfaces(u32 dip)
{
    vtss_if_id_vlan_t   search_vid = VTSS_VID_NULL, vid;
    vtss_if_status_t    ifstat;
    u32                 ifct;

    while (vtss_ip2_if_id_next(search_vid, &vid) == VTSS_OK) {
        if (vtss_ip2_if_status_get(VTSS_IF_STATUS_TYPE_IPV4, vid, 1, &ifct, &ifstat) == VTSS_OK &&
            ifct == 1 &&
            ifstat.type == VTSS_IF_STATUS_TYPE_IPV4 &&
            ifstat.u.ipv4.net.address == dip) {
            return TRUE;
        }
        search_vid = vid;
    }

    return FALSE;
}

static void DHCP_HELPER_process_rx_pkt(const u8 *const packet,
                                       size_t len,
                                       vtss_vid_t vid,
                                       vtss_isid_t isid,
                                       vtss_port_no_t port_no,
                                       vtss_glag_no_t glag_no)
{
    dhcp_helper_frame_info_t    record_info;
    u8                          *ptr = (u8 *)(packet);
    u16                         *ether_type = (u16 *)(ptr + 12);
    struct ip                   *ip = (struct ip *)(ptr + 14);
    int                         ip_header_len = IP_VHL_HL(ip->ip_hl) << 2;
    struct udphdr               *udp_hdr = (struct udphdr *)(ptr + 14 + ip_header_len);
    struct bootp                bp;
    u8                          *client_mac, dhcp_message;
    u32                         transaction_id, bp_client_addr;
    u8                          system_mac_addr[6];
    dhcp_helper_user_t          user = DHCP_HELPER_USER_HELPER;

    T_D("enter, RX isid %d vid %d port %d len %zd", isid, vid, port_no, len);

    /* We only process IPv4 now */
    if (*ether_type != htons(0x0800)) {
        T_D("exit: Not IPv4");
        return;
    }
    memset(system_mac_addr, 0, 6);
    (void)conf_mgmt_mac_addr_get(system_mac_addr, 0);

    /* Get DHCP message type, chaddr and transaction ID */
    memcpy(&bp, ptr + 14 + ip_header_len + 8, sizeof(bp)); /* 14:DA+SA+ETYPE, 8:udp header length */
    dhcp_message = bp.bp_vend[6];
    client_mac = (u8 *)bp.bp_chaddr;
    transaction_id = bp.bp_xid;
    bp_client_addr = bp.bp_ciaddr.s_addr;
    bp_client_addr = ntohl(bp_client_addr);

    /* Initialize DHCP user */
    DHCP_HELPER_CRIT_ENTER();
#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
    /* Lowest priority */
    if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING]) {
        user = DHCP_HELPER_USER_SNOOPING;
    }
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */

#if defined(VTSS_SW_OPTION_DHCP_RELAY)
    /* Relay priority is higher than Snooping */
    if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_RELAY]) {
        user = DHCP_HELPER_USER_RELAY;
    }
#endif /* VTSS_SW_OPTION_DHCP_RELAY */

#if defined(VTSS_SW_OPTION_DHCP_SERVER)
    /* All DHCP request packet come to Server module first */
    if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SERVER] && !DHCP_HELPER_MSG_FROM_SERVER(dhcp_message)) {
        user = DHCP_HELPER_USER_SERVER;
    }
#endif /* VTSS_SW_OPTION_DHCP_SERVER */

#if defined(VTSS_SW_OPTION_DHCP_CLIENT)
    /* If DHCP reply packet and "chaddr" equals swicth MAC address, come to Client module */
    if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_CLIENT] &&
        DHCP_HELPER_MSG_FROM_SERVER(dhcp_message) &&
        !memcmp(system_mac_addr, client_mac, 6)) {
        user = DHCP_HELPER_USER_CLIENT;
    }
#endif /* VTSS_SW_OPTION_DHCP_CLIENT */
    DHCP_HELPER_CRIT_EXIT();

    /* Verify IP/UDP checksum */
    if (DHCP_HELPER_do_ip_chksum(ip_header_len, (u16 *)ip) ||
        (udp_hdr->uh_sum && DHCP_HELPER_do_udp_chksum(htons(udp_hdr->uh_ulen), (u16 *)&ip->ip_src.s_addr, (u16 *)&ip->ip_dst.s_addr, (u16 *)udp_hdr))) {
        DHCP_HELPER_CRIT_ENTER();
        DHCP_HELPER_global.stats[user][isid][port_no].rx_stats.discard_chksum_err_rx++;
        DHCP_HELPER_CRIT_EXIT();
        T_D("exit: IP/UDP checksum error");
        return;
    }

#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
    DHCP_HELPER_CRIT_ENTER();
    if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING]) {
        /* Drop incoming reply packet if it come from untrust port */
        if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING] &&
            DHCP_HELPER_MSG_FROM_SERVER(dhcp_message) &&
            DHCP_HELPER_global.port_conf[isid].port_mode[port_no] == DHCP_HELPER_PORT_MODE_UNTRUSTED) {
            DHCP_HELPER_global.stats[DHCP_HELPER_USER_SNOOPING][isid][port_no].rx_stats.discard_untrust_rx++;
            DHCP_HELPER_CRIT_EXIT();

            //Delete related entry
            memcpy(record_info.mac, client_mac, 6);
            record_info.vid = vid;
            record_info.transaction_id = transaction_id;
            DHCP_HELPER_frame_info_del(&record_info);

            T_D("exit: Come from untrust port");
            return;
        }
    }
    DHCP_HELPER_CRIT_EXIT();
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */

    /* Record incoming frame information */
    memset(&record_info, 0x0, sizeof(record_info));
    memcpy(record_info.mac, client_mac, 6);
    record_info.vid = vid;
    record_info.isid = isid;
    record_info.port_no = port_no;
    record_info.glag_no = glag_no;
    record_info.op_code = dhcp_message;
    record_info.transaction_id = transaction_id;
    if (bp_client_addr) {
        record_info.assigned_ip = bp_client_addr;
    }

    /* Record assigned IP and lease time */
    if (dhcp_message == DHCP_HELPER_MSG_TYPE_REQUEST || dhcp_message == DHCP_HELPER_MSG_TYPE_ACK) {
        DHCP_HELPER_frame_info_parse(ptr, len, &record_info);
    }

    //Save the DHCP frame info. expect for this switch
#if defined(VTSS_SW_OPTION_DHCP_CLIENT)
    if (user != DHCP_HELPER_USER_CLIENT)
#endif /* VTSS_SW_OPTION_DHCP_CLIENT */
    {
        if (dhcp_message == DHCP_HELPER_MSG_TYPE_NAK || dhcp_message == DHCP_HELPER_MSG_TYPE_RELEASE) {
            //Delete related entry
            DHCP_HELPER_frame_info_del(&record_info);
#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
            DHCP_HELPER_CRIT_ENTER();
            /* Notice snooping module the client release the dynamic IP */
            if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING] &&
                dhcp_message == DHCP_HELPER_MSG_TYPE_RELEASE &&
                DHCP_HELPER_release_ip_addr_cb) {
                DHCP_HELPER_release_ip_addr_cb(&record_info);
            }
            DHCP_HELPER_CRIT_EXIT();
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */
        } else if (DHCP_HELPER_frame_info_add(&record_info)) {
            //Clear other uncompleted entry
            DHCP_HELPER_frame_info_clear(&record_info, FALSE);
        }
    }

#if defined(VTSS_SW_OPTION_DHCP_SERVER)
    /* Pass through the DHCP packet to DHCP server module.
       The callback API should retrun TRUE if the DHCP server module will handle this DHCP packet.
       And retrun FALSE if the DHCP server module won't handle this DHCP packet (may be due to VLAN mode is disabled)  */
    if (user == DHCP_HELPER_USER_SERVER) {
        if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SERVER] &&
            DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SERVER](packet, len, vid, isid, port_no, glag_no)) {
            DHCP_HELPER_stats_add(user, isid, VTSS_BIT64(port_no), dhcp_message, DHCP_HELPER_DIRECTION_RX);
            T_D("exit: DHCP Server");
            return;
        } else {    /* Change user if the DHCP Server module doesn't process it */
            user = DHCP_HELPER_USER_HELPER;
            DHCP_HELPER_CRIT_ENTER();
#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
            if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING]) {
                user = DHCP_HELPER_USER_SNOOPING;
            }
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */
#if defined(VTSS_SW_OPTION_DHCP_RELAY)
            if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_RELAY]) {
                user = DHCP_HELPER_USER_RELAY;
            }
#endif /* VTSS_SW_OPTION_DHCP_RELAY */
            DHCP_HELPER_CRIT_EXIT();
        }
    }
#endif /* VTSS_SW_OPTION_DHCP_SERVER */

#if defined(VTSS_SW_OPTION_DHCP_CLIENT)
    if (user == DHCP_HELPER_USER_CLIENT) {
        DHCP_HELPER_stats_add(user, isid, VTSS_BIT64(port_no), dhcp_message, DHCP_HELPER_DIRECTION_RX);
        if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_CLIENT]) {
            (void) DHCP_HELPER_rx_cb[DHCP_HELPER_USER_CLIENT](packet, len, vid, isid, port_no, glag_no);
        }
        T_D("exit: DHCP Client");
        return;
    }
#endif /* VTSS_SW_OPTION_DHCP_CLIENT */

#if defined(VTSS_SW_OPTION_DHCP_RELAY)
    if (user == DHCP_HELPER_USER_RELAY) {
        u8 broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        if ((!DHCP_HELPER_MSG_FROM_SERVER(dhcp_message) && !memcmp(broadcast_mac, ptr, 6)) ||
            (DHCP_HELPER_MSG_FROM_SERVER(dhcp_message) && !memcmp(ptr, system_mac_addr, 6) && DHCP_HELPER_is_myself_interfaces(ip->ip_dst.s_addr))) {
            DHCP_HELPER_stats_add(user, isid, VTSS_BIT64(port_no), dhcp_message, DHCP_HELPER_DIRECTION_RX);
            if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_RELAY]) {
                (void) DHCP_HELPER_rx_cb[DHCP_HELPER_USER_RELAY](packet, len, vid, isid, port_no, glag_no);
            }
            T_D("exit: DHCP Relay");
            return;
        } else {
            user = DHCP_HELPER_USER_HELPER;
        }
#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
        if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING]) {
            user = DHCP_HELPER_USER_SNOOPING;
        }
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */
    }
#endif /* VTSS_SW_OPTION_DHCP_RELAY */

    /* Do L3 forwarding if DMAC == myself && DIP != myself.
       Otherwise, do L2 forwarding */
    if (!memcmp(ptr, system_mac_addr, 6) && !DHCP_HELPER_is_myself_interfaces(ip->ip_dst.s_addr)) {
        DHCP_HELPER_stats_add(DHCP_HELPER_USER_HELPER, isid, VTSS_BIT64(port_no), dhcp_message, DHCP_HELPER_DIRECTION_RX);
        (void) vtss_ip2_if_inject((vtss_if_id_vlan_t) vid, len, packet);
        T_D("exit: L3 forwarding");
        return;
    }

#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
    if (user == DHCP_HELPER_USER_SNOOPING) {
        DHCP_HELPER_stats_add(user, isid, VTSS_BIT64(port_no), dhcp_message, DHCP_HELPER_DIRECTION_RX);
        if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING]) {
            (void) DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING](packet, len, vid, isid, port_no, glag_no);
        }
        T_D("exit: DHCP Snooping");
        return;
    }
#endif /* VTSS_SW_OPTION_DHCP_RELAY */

    if (user == DHCP_HELPER_USER_HELPER) {
        /* No specific DHCP user processed the DHCP packet. Forward it to other front ports. */
        u8   *pkt_buf;
        void *bufref;

        DHCP_HELPER_stats_add(user, isid, VTSS_BIT64(port_no), dhcp_message, DHCP_HELPER_DIRECTION_RX);
        if ((pkt_buf = dhcp_helper_alloc_xmit(len, VTSS_ISID_GLOBAL, &bufref)) != NULL) {
            memcpy(pkt_buf, packet, len);
            if (dhcp_helper_xmit(DHCP_HELPER_USER_HELPER, pkt_buf, len, vid, VTSS_ISID_GLOBAL, 0, isid, port_no, glag_no, bufref)) {
                T_D("Calling dhcp_helper_xmit() failed");
            }
        }
        T_D("exit: DHCP Helper");
    }

    T_D("exit");
    return;
}

static BOOL DHCP_HELPER_source_port_lookup(vtss_vid_mac_t *vid_mac, vtss_isid_t *src_isid, vtss_port_no_t *src_port_no)
{
    vtss_mac_table_entry_t  mac_entry;
    switch_iter_t           sit;
    port_iter_t             pit;

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        if (mac_mgmt_table_get_next(sit.isid, vid_mac, &mac_entry, FALSE) == VTSS_OK) {
            (void) port_iter_init(&pit, NULL, sit.isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
            while (port_iter_getnext(&pit)) {
                if (mac_entry.destination[pit.iport]) {
#if defined(VTSS_FEATURE_VSTAX_V2) && defined(VTSS_SWITCH_STACKABLE) && VTSS_SWITCH_STACKABLE
                    if (vtss_stacking_enabled()) {
                        *src_isid = topo_usid2isid(mac_mgmt_upsid2usid(mac_entry.vstax2.upsid));
                    }
#else
                    *src_isid = sit.isid;
#endif
                    *src_port_no = pit.iport;
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

// Do transmit DHCP frame
static void DHCP_HELPER_tx_pkt(dhcp_helper_bip_buf_t *bip_buf)
{
    struct bootp      bp;
    struct ip         *ip = (struct ip *)(bip_buf->pkt + 14);
    int               ip_header_len = IP_VHL_HL(ip->ip_hl) << 2;
    u8                dhcp_message;
    u8                broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    BOOL              flooding_pkt = FALSE;
    vtss_isid_t       isid_idx, isid_idx_start, isid_idx_end;
    dhcp_helper_msg_t *msg = (dhcp_helper_msg_t *)bip_buf->bufref_p, *original_msg = msg;
    int               need_send_cnt = 0;
    u64               temp_port_mask, send_port_mask[VTSS_ISID_END];

    /* Get DHCP message type. */
    memcpy(&bp, bip_buf->pkt + 14 + ip_header_len + 8, sizeof(bp)); /* 14:DA+SA+ETYPE, 8:udp header length */
    dhcp_message = bp.bp_vend[6];

    if (bip_buf->dst_isid == VTSS_ISID_GLOBAL) {
        if (!memcmp(broadcast_mac, bip_buf->pkt, 6)) {
            flooding_pkt   = TRUE;
            isid_idx_start = VTSS_ISID_START;
            isid_idx_end   = VTSS_ISID_END - 1;
        } else {    // Unicast. Lookup the source port
            vtss_vid_mac_t  vid_mac;
            vtss_isid_t     unicast_pkt_src_isid = VTSS_ISID_START;
            vtss_port_no_t  unicast_pkt_src_port_no = VTSS_PORT_NO_START;

            vid_mac.vid = bip_buf->vid;
            memcpy(vid_mac.mac.addr, bip_buf->pkt, 6);
            if (DHCP_HELPER_source_port_lookup(&vid_mac, &unicast_pkt_src_isid, &unicast_pkt_src_port_no)) {
                isid_idx_start = isid_idx_end = unicast_pkt_src_isid;
                bip_buf->dst_port_mask  = VTSS_BIT64(unicast_pkt_src_port_no);
            } else {
                // Cannot find the source port. Flood it.
                flooding_pkt   = TRUE;
                isid_idx_start = VTSS_ISID_START;
                isid_idx_end   = VTSS_ISID_END - 1;
            }
        }
    } else {
        isid_idx_start = isid_idx_end = bip_buf->dst_isid;

#if defined(VTSS_SW_OPTION_DHCP_SNOOPING)
        /* Filter untrusted ports */
        DHCP_HELPER_CRIT_ENTER();
        if (DHCP_HELPER_rx_cb[DHCP_HELPER_USER_SNOOPING] &&
            bip_buf->user != DHCP_HELPER_USER_SNOOPING &&
#if defined(VTSS_SW_OPTION_DHCP_SERVER)
            bip_buf->user != DHCP_HELPER_USER_SERVER &&
#endif /* VTSS_SW_OPTION_DHCP_SERVER */
            !DHCP_HELPER_MSG_FROM_SERVER(dhcp_message) &&
            bip_buf->dst_port_mask) {
            port_iter_t pit;
            (void) port_iter_init(&pit, NULL, bip_buf->dst_isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_FRONT);
            while (port_iter_getnext(&pit)) {
                if (VTSS_EXTRACT_BITFIELD64(bip_buf->dst_port_mask, pit.iport,  1) == 0) {
                    continue;
                }

                /* Only forward client's packet to trust port when DHCP Snooping is enabled. */
                if (DHCP_HELPER_global.port_conf[bip_buf->dst_isid].port_mode[pit.iport] == DHCP_HELPER_PORT_MODE_UNTRUSTED) {
                    T_D("Filter untrusted ports: %d/%d", bip_buf->dst_isid, pit.iport);
                    bip_buf->dst_port_mask &= ~VTSS_BIT64(pit.iport);

                    if (bip_buf->dst_port_mask == 0) {
                        T_D("No destination ports");
                        VTSS_FREE(msg);
                        DHCP_HELPER_CRIT_EXIT();
                        return;
                    }
                }
            }
        }
        DHCP_HELPER_CRIT_EXIT();
#endif /* VTSS_SW_OPTION_DHCP_SNOOPING */
    }

    /* Get destination port mask */
    memset(send_port_mask, 0, sizeof(send_port_mask));
    for (isid_idx = isid_idx_start; isid_idx <= isid_idx_end; isid_idx++) {
        if (!msg_switch_exists(isid_idx)) {
            continue;
        }

        if (flooding_pkt) {
            temp_port_mask = DHCP_HELPER_dst_port_mask_get(bip_buf->vid, isid_idx, bip_buf->src_isid, bip_buf->src_port_no, bip_buf->src_glag_no, dhcp_message);
        } else {
            temp_port_mask = bip_buf->dst_port_mask;
        }

        if (temp_port_mask) {
            send_port_mask[isid_idx] = temp_port_mask;
            need_send_cnt++;
        }
    }

    /* Free allocated memory if we don't need to send out any packet */
    if (need_send_cnt == 0) {
        VTSS_FREE(msg);
        return;
    }

    for (isid_idx = isid_idx_start; isid_idx <= isid_idx_end; isid_idx++) {
        u8      *new_pkt_buf_p;
        void    *new_bufref_p;

        if (send_port_mask[isid_idx] == 0) {
            continue;
        }

        if (need_send_cnt != 1) {
            // Call msg_tx() will automatic free the dynamic memory. We need to re-alloc a new memory.
            if ((new_pkt_buf_p = dhcp_helper_alloc_xmit(bip_buf->len, isid_idx, &new_bufref_p)) == NULL) {
                T_W("Calling dhcp_helper_alloc_xmit() failed.\n");
                continue;
            } else {
                memcpy(new_pkt_buf_p, bip_buf->pkt, bip_buf->len);
                msg = (dhcp_helper_msg_t *)new_bufref_p;
            }
        }
        msg->data.tx_req.user          = bip_buf->user;
        msg->data.tx_req.vid           = bip_buf->vid;
        msg->data.tx_req.isid          = isid_idx;
        msg->data.tx_req.dst_port_mask = send_port_mask[isid_idx];
        msg->data.tx_req.src_port_no   = isid_idx == bip_buf->src_isid ? bip_buf->src_port_no : VTSS_PORT_NO_NONE;
        msg->data.tx_req.src_glag_no   = bip_buf->src_glag_no;
        msg->data.tx_req.dhcp_message  = dhcp_message;
        msg_tx(VTSS_MODULE_ID_DHCP_HELPER, isid_idx, msg, bip_buf->len + sizeof(*msg));
    }

    if (need_send_cnt != 1) { // Free original allocated memory
        VTSS_FREE(original_msg);
    }

    if (dhcp_message == DHCP_HELPER_MSG_TYPE_OFFER ||
        dhcp_message == DHCP_HELPER_MSG_TYPE_ACK ||
        dhcp_message == DHCP_HELPER_MSG_TYPE_NAK) {
        dhcp_helper_frame_info_t    record_info;
        memset(&record_info, 0x0, sizeof(record_info));
        memcpy(record_info.mac, bp.bp_chaddr, 6);
        record_info.vid = bip_buf->vid;
        record_info.transaction_id = bp.bp_xid;
        record_info.op_code = dhcp_message;

#if defined(VTSS_SW_OPTION_DHCP_SERVER) || defined(VTSS_SW_OPTION_DHCP_RELAY)
        if (dhcp_message == DHCP_HELPER_MSG_TYPE_OFFER ||
            dhcp_message == DHCP_HELPER_MSG_TYPE_ACK) {
            if (dhcp_message == DHCP_HELPER_MSG_TYPE_ACK) {
                /* Record assigned IP and lease time */
                DHCP_HELPER_frame_info_parse(bip_buf->pkt, bip_buf->len, &record_info);

#if defined(VTSS_SW_OPTION_DHCP_SERVER)
                if (bip_buf->user == DHCP_HELPER_USER_SERVER) {
                    record_info.local_dhcp_server = TRUE;
                }
#endif /* VTSS_SW_OPTION_DHCP_SERVER */
            }
            (void) DHCP_HELPER_frame_info_add(&record_info);
        }
#endif /* VTSS_SW_OPTION_DHCP_SERVER || VTSS_SW_OPTION_DHCP_RELAY */

        if (dhcp_message == DHCP_HELPER_MSG_TYPE_ACK ||
            dhcp_message == DHCP_HELPER_MSG_TYPE_NAK) {
            /* Clear releated record informations when relayed packets to client,
            cause we got the output sid/port now, don't need it anymore */
            DHCP_HELPER_frame_info_clear(&record_info, TRUE);
        }
    }
}


static void DHCP_HELPER_bip_buffer_rx_enqueue(const u8 *const packet,
                                              size_t len,
                                              vtss_vid_t vid,
                                              vtss_isid_t isid,
                                              vtss_port_no_t port_no,
                                              vtss_glag_no_t glag_no)
{
    dhcp_helper_bip_buf_t *bip_buf;

    /* Check input parameters */
    if (packet == NULL || len == 0) {
        return;
    }

    DHCP_HELPER_BIP_CRIT_ENTER();
    bip_buf = (dhcp_helper_bip_buf_t *)vtss_bip_buffer_reserve(&DHCP_HELPER_bip_buf, DHCP_HELPER_BIP_BUF_ALIGNED_SIZE);
    if (bip_buf == NULL) {
        DHCP_HELPER_BIP_CRIT_EXIT();
        T_D("Failure in reserving DHCP Helper BIP buffer");
        return;
    }

    memcpy(bip_buf->pkt, packet, len);
    bip_buf->len         = len;
    bip_buf->is_rx       = TRUE;
    bip_buf->vid         = vid;
    bip_buf->src_isid    = isid;
    bip_buf->src_port_no = port_no;
    bip_buf->src_glag_no = glag_no;

    vtss_bip_buffer_commit(&DHCP_HELPER_bip_buf);
    cyg_flag_setbits(&DHCP_HELPER_bip_buffer_thread_events, DHCP_HELPER_EVENT_PKT_RX);
    DHCP_HELPER_BIP_CRIT_EXIT();
}

static void DHCP_HELPER_bip_buffer_dequeue(BOOL clear_it)
{
    dhcp_helper_bip_buf_t *bip_buf;
    int                   buf_size;

    do {
        DHCP_HELPER_BIP_CRIT_ENTER();
        bip_buf = (dhcp_helper_bip_buf_t *)vtss_bip_buffer_get_contiguous_block(&DHCP_HELPER_bip_buf, &buf_size);
        DHCP_HELPER_BIP_CRIT_EXIT();

        if (bip_buf) {
            if (buf_size < (int)DHCP_HELPER_BIP_BUF_ALIGNED_SIZE) {
                T_E("Odd. buf_size = %d, expected at least %d", buf_size, DHCP_HELPER_BIP_BUF_ALIGNED_SIZE);
            } else if (clear_it) {
                // Just discard the buffer, but make sure to free any allocated
                // buffer that it may contain in case it's a Tx message.
                if (!bip_buf->is_rx && bip_buf->bufref_p) {
                    VTSS_FREE(bip_buf->bufref_p);
                }
            } else if (bip_buf->is_rx) {
                T_D("BIP buffer Rx, len = %zd", bip_buf->len);
                DHCP_HELPER_process_rx_pkt((u8 *)bip_buf->pkt, bip_buf->len, bip_buf->vid, bip_buf->src_isid, bip_buf->src_port_no, bip_buf->src_glag_no);
            } else {
                T_D("BIP buffer Tx, len = %zd", bip_buf->len);
                DHCP_HELPER_tx_pkt(bip_buf);
            }

            if (buf_size) {
                DHCP_HELPER_BIP_CRIT_ENTER();
                vtss_bip_buffer_decommit_block(&DHCP_HELPER_bip_buf, DHCP_HELPER_BIP_BUF_ALIGNED_SIZE);
                DHCP_HELPER_BIP_CRIT_EXIT();
            }
        }
    } while (bip_buf);
}

/****************************************************************************/
/*  Reserved ACEs functions                                                 */
/****************************************************************************/
/* Add reserved ACE */
static void DHCP_HELPER_ace_add(void)
{
    acl_entry_conf_t conf;

    /* Set two ACEs (bootps and bootpc) on all switches (master and slaves). */
    //bootps
    if (acl_mgmt_ace_init(VTSS_ACE_TYPE_IPV4, &conf) != VTSS_OK) {
        return;
    }
    conf.id = DHCP_HELPER_BOOTPS_ACE_ID;

#if defined(VTSS_ARCH_SERVAL)
    conf.isdx_disable = TRUE;
#endif /* VTSS_FEATURE_ACL_V2 */

#if defined(VTSS_FEATURE_ACL_V2)
    conf.action.port_action = VTSS_ACL_PORT_ACTION_FILTER;
    memset(conf.action.port_list, 0, sizeof(conf.action.port_list));
#else
    conf.action.permit = FALSE;
#endif /* VTSS_FEATURE_ACL_V2 */
    conf.action.force_cpu = TRUE;
    conf.action.cpu_once = FALSE;
    conf.isid = VTSS_ISID_LOCAL;
    VTSS_BF_SET(conf.flags.mask, ACE_FLAG_IP_FRAGMENT, 1);
    VTSS_BF_SET(conf.flags.value, ACE_FLAG_IP_FRAGMENT, 0);
    conf.frame.ipv4.proto.value = 17; //UDP
    conf.frame.ipv4.proto.mask = 0xFF;
    conf.frame.ipv4.sport.in_range = conf.frame.ipv4.dport.in_range = TRUE;
    conf.frame.ipv4.sport.high = 65535;
    conf.frame.ipv4.dport.low = conf.frame.ipv4.dport.high = DHCP_SERVER_UDP_PORT;
    if (acl_mgmt_ace_add(ACL_USER_DHCP, ACE_ID_NONE, &conf) != VTSS_OK) {
        T_D("Delete DHCP helper reserved ACE (BOOTPS) fail.\n");
    }

    //bootpc
    conf.id = DHCP_HELPER_BOOTPC_ACE_ID;
    conf.frame.ipv4.dport.low = conf.frame.ipv4.dport.high = DHCP_CLINET_UDP_PORT;
    if (acl_mgmt_ace_add(ACL_USER_DHCP, ACE_ID_NONE, &conf) != VTSS_OK) {
        T_D("Delete DHCP helper reserved ACE (BOOTPC) fail.\n");
    }
}

/* Delete reserved ACE */
static void DHCP_HELPER_ace_del(void)
{
    if (acl_mgmt_ace_del(ACL_USER_DHCP, DHCP_HELPER_BOOTPS_ACE_ID) != VTSS_OK) {
        T_D("Delete DHCP helper reserved ACE (BOOTPS) fail.\n");
    }
    if (acl_mgmt_ace_del(ACL_USER_DHCP, DHCP_HELPER_BOOTPC_ACE_ID) != VTSS_OK) {
        T_D("Delete DHCP helper reserved ACE (BOOTPC) fail.\n");
    }
}


/****************************************************************************/
/*  Local Transmit functions                                                */
/****************************************************************************/
static vtss_rc DHCP_HELPER_local_xmit(dhcp_helper_user_t user,
                                      const u8 *const packet,
                                      size_t len,
                                      vtss_vid_t vid,
                                      vtss_isid_t isid,
                                      u64 dst_port_mask,
                                      vtss_port_no_t src_port_no,
                                      vtss_glag_no_t src_glag_no,
                                      u8 dhcp_message)
{
    vtss_packet_port_info_t     info;
    vtss_packet_port_filter_t   filter[VTSS_PORT_ARRAY_SIZE];
    vtss_port_no_t              port_idx;
    void                        *frame;
    vtss_rc                     rc;
    packet_tx_props_t           tx_props;

    if ((rc = vtss_packet_port_info_init(&info)) != VTSS_OK) {
        return rc;
    }

    //Get port filter information
    info.vid = vid;
    if ((rc = vtss_packet_port_filter_get(NULL, &info, filter)) != VTSS_OK) {
        return rc;
    }

    /* Filter destination ports */
    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
        if (VTSS_EXTRACT_BITFIELD64(dst_port_mask, port_idx,  1) == 0) {
            continue;
        }
        if (filter[port_idx].filter == VTSS_PACKET_FILTER_DISCARD) {
            dst_port_mask &= ~VTSS_BIT64(port_idx);
            continue;
        }

        /* Allloc frame buffer */
        if ((frame = packet_tx_alloc(len)) == NULL) {
            return DHCP_HELPER_ERROR_FRAME_BUF_ALLOCATED;
        }

        memcpy(frame, packet, len);
        packet_tx_props_init(&tx_props);
        tx_props.tx_info.tag.vid             = vid;
        tx_props.packet_info.filter.enable   = TRUE;
        tx_props.packet_info.filter.src_port = src_port_no; /* Not needed if you know that dst != src */
#if defined(VTSS_FEATURE_AGGR_GLAG)
        tx_props.packet_info.filter.glag_no  = src_glag_no; /* Avoid transmitting on ingress GLAG */
#endif /* VTSS_FEATURE_AGGR_GLAG */
        tx_props.packet_info.modid           = VTSS_MODULE_ID_DHCP_HELPER;
        tx_props.packet_info.frm[0]          = frame;
        tx_props.packet_info.len[0]          = len;
        tx_props.tx_info.dst_port_mask       = VTSS_BIT64(port_idx);

        if ((rc = packet_tx(&tx_props)) == VTSS_RC_ERROR) {
            T_D("Frame transmit failed");
            return rc;
        } else if (rc == VTSS_RC_INV_STATE) {
            T_D("Frame was filtered");
            return rc;
        }
    }

    if (rc == VTSS_OK) {
        DHCP_HELPER_stats_add(user, isid, dst_port_mask, dhcp_message, DHCP_HELPER_DIRECTION_TX);
    }

    return VTSS_OK;
}


/****************************************************************************/
/*  Message functions                                                       */
/****************************************************************************/

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static char *DHCP_HELPER_msg_id_txt(dhcp_helper_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case DHCP_HELPER_MSG_ID_FRAME_RX_IND:
        txt = "DHCP_HELPER_MSG_ID_FRAME_RX_IND";
        break;
    case DHCP_HELPER_MSG_ID_FRAME_TX_REQ:
        txt = "DHCP_HELPER_MSG_ID_FRAME_TX_REQ";
        break;
    case DHCP_HELPER_MSG_ID_LOCAL_ACE_SET:
        txt = "DHCP_HELPER_MSG_ID_LOCAL_ACE_SET";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_TRACE_LVL_DEBUG */

/* Allocate request buffer */
static dhcp_helper_msg_t *DHCP_HELPER_msg_alloc(dhcp_helper_msg_buf_t *buf, dhcp_helper_msg_id_t msg_id, BOOL request)
{
    dhcp_helper_msg_t *msg = request ? &DHCP_HELPER_global.request.msg : &DHCP_HELPER_global.reply.msg;

    buf->sem = request ? &DHCP_HELPER_global.request.sem : &DHCP_HELPER_global.reply.sem;
    buf->msg = msg;
    (void) VTSS_OS_SEM_WAIT(buf->sem);
    msg->msg_id = msg_id;
    return msg;
}

/* Free request/reply buffer */
static void DHCP_HELPER_msg_free(vtss_os_sem_t *sem)
{
    VTSS_OS_SEM_POST(sem);
}

static void DHCP_HELPER_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    dhcp_helper_msg_id_t msg_id = *(dhcp_helper_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s", msg_id, DHCP_HELPER_msg_id_txt(msg_id));
    DHCP_HELPER_msg_free(contxt);
}

static void DHCP_HELPER_msg_tx(dhcp_helper_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    dhcp_helper_msg_id_t msg_id = *(dhcp_helper_msg_id_t *)buf->msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s, len: %zd, isid: %d", msg_id, DHCP_HELPER_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(buf->sem, DHCP_HELPER_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_MODULE_ID_DHCP_HELPER, isid, buf->msg, len + MSG_TX_DATA_HDR_LEN(dhcp_helper_msg_t, data));
}

/* When we're master, the isid is the slave's ISID.
   When we're slave, the isid is sthe connection ID which is the "arbitrarily" chosen mcb->connid.
   Please be careful under the slave role. In such case, we *MUST* use 'VTSS_ISID_LOCAL' instead of the value of isid.
*/
static BOOL DHCP_HELPER_msg_rx(void *contxt,
                               const void *rx_msg,
                               size_t len,
                               vtss_module_id_t modid,
                               u32 isid)
{
    const dhcp_helper_msg_t *msg = (dhcp_helper_msg_t *)rx_msg;

    T_D("Sid %u, rx %zd bytes, msg %d", isid, len, msg->msg_id);

    switch (msg->msg_id) {
    case DHCP_HELPER_MSG_ID_FRAME_RX_IND: {
        /* The API of mac_mgmt_table_get_next() will use msg to get slave data.
           It will occur message timeout in current message thread.
           To avoid it, we store the incoming packet to BIP buffer first and
           use another thread to process it. */
        if (msg_switch_is_master()) {
            DHCP_HELPER_bip_buffer_rx_enqueue((u8 *)&msg[1], msg->data.rx_ind.len, msg->data.rx_ind.vid, isid, msg->data.rx_ind.port_no, msg->data.rx_ind.glag_no);
        }
        break;
    }
    case DHCP_HELPER_MSG_ID_FRAME_TX_REQ: {
        (void) DHCP_HELPER_local_xmit(msg->data.tx_counters_get.user,
                                      (u8 *)&msg[1],
                                      msg->data.tx_req.len,
                                      msg->data.tx_req.vid,
                                      msg->data.tx_req.isid,
                                      msg->data.tx_req.dst_port_mask,
                                      msg->data.tx_req.src_port_no,
                                      msg->data.tx_req.src_glag_no,
                                      msg->data.tx_req.dhcp_message);
        break;
    }
    case DHCP_HELPER_MSG_ID_LOCAL_ACE_SET: {
        if (msg->data.local_ace_set.add) {
            DHCP_HELPER_rx_filter_register(TRUE);
            DHCP_HELPER_ace_add();
        } else {
            DHCP_HELPER_ace_del();
            DHCP_HELPER_rx_filter_register(FALSE);
            DHCP_HELPER_frame_info_clear_all();
        }
        break;
    }
    case DHCP_HELPER_MSG_ID_COUNTERS_GET_REQ: {
        if (msg->data.tx_counters_get.port_no < VTSS_PORT_NO_END) {
            dhcp_helper_msg_t       *msg_rep;
            dhcp_helper_msg_buf_t   buf;

            DHCP_HELPER_CRIT_ENTER();
            msg_rep = DHCP_HELPER_msg_alloc(&buf, DHCP_HELPER_MSG_ID_COUNTERS_GET_REP, FALSE);
            msg_rep->data.tx_counters_get.user = msg->data.tx_counters_get.user;
            msg_rep->data.tx_counters_get.port_no = msg->data.tx_counters_get.port_no;
            msg_rep->data.tx_counters_get.tx_stats = DHCP_HELPER_global.stats[msg->data.tx_counters_get.user][msg_switch_is_master() ? isid : VTSS_ISID_LOCAL][msg->data.tx_counters_get.port_no].tx_stats;
            DHCP_HELPER_CRIT_EXIT();
            DHCP_HELPER_msg_tx(&buf, isid, sizeof(msg_rep->data.tx_counters_get));
        }
        break;
    }
    case DHCP_HELPER_MSG_ID_COUNTERS_GET_REP: {
        if (msg_switch_is_master() &&
            msg->data.tx_counters_get.port_no < VTSS_PORT_NO_END) {   //master only
            dhcp_helper_user_t user = msg->data.tx_counters_get.user;
            dhcp_helper_stats_tx_t *tx_stats;
            DHCP_HELPER_CRIT_ENTER();
            tx_stats = &DHCP_HELPER_global.stats[user][isid][msg->data.tx_counters_get.port_no].tx_stats;
#if defined(VTSS_SW_OPTION_DHCP_RELAY)
            if (user == DHCP_HELPER_USER_RELAY && !msg_switch_is_local(isid)) {
                //Relay's TX statistics: Slave local counter +  Master socket counter
                tx_stats->discover_tx          += msg->data.tx_counters_get.tx_stats.discover_tx;
                tx_stats->offer_tx             += msg->data.tx_counters_get.tx_stats.offer_tx;
                tx_stats->request_tx           += msg->data.tx_counters_get.tx_stats.request_tx;
                tx_stats->decline_tx           += msg->data.tx_counters_get.tx_stats.decline_tx;
                tx_stats->ack_tx               += msg->data.tx_counters_get.tx_stats.ack_tx;
                tx_stats->nak_tx               += msg->data.tx_counters_get.tx_stats.nak_tx;
                tx_stats->release_tx           += msg->data.tx_counters_get.tx_stats.release_tx;
                tx_stats->inform_tx            += msg->data.tx_counters_get.tx_stats.inform_tx;
                tx_stats->leasequery_tx        += msg->data.tx_counters_get.tx_stats.leasequery_tx;
                tx_stats->leaseunassigned_tx   += msg->data.tx_counters_get.tx_stats.leaseunassigned_tx;
                tx_stats->leaseunknown_tx      += msg->data.tx_counters_get.tx_stats.leaseunknown_tx;
                tx_stats->leaseactive_tx       += msg->data.tx_counters_get.tx_stats.leaseactive_tx;
            } else
#endif /* VTSS_SW_OPTION_DHCP_RELAY */
            {
                *tx_stats = msg->data.tx_counters_get.tx_stats;
            }
            VTSS_MTIMER_START(&DHCP_HELPER_global.stats_timer[isid], DHCP_HELPER_COUNTERS_TIMER);
            cyg_flag_setbits(&DHCP_HELPER_global.stats_flags, 1 << isid);
            DHCP_HELPER_CRIT_EXIT();
        }
        break;
    }
    case DHCP_HELPER_MSG_ID_COUNTERS_CLR: {
        DHCP_HELPER_CRIT_ENTER();
        if (msg->data.counters_clear.user == DHCP_HELPER_USER_CNT) {
            dhcp_helper_user_t user_idx;
            for (user_idx = DHCP_HELPER_USER_HELPER; user_idx < DHCP_HELPER_USER_CNT; user_idx++) {
                if (msg->data.counters_clear.port_no == VTSS_PORT_NO_END) {
                    memset(&DHCP_HELPER_global.stats[user_idx][VTSS_ISID_LOCAL][msg->data.counters_clear.port_no], 0, sizeof(dhcp_helper_stats_t));
                } else {
                    memset(DHCP_HELPER_global.stats[user_idx][VTSS_ISID_LOCAL], 0, sizeof(DHCP_HELPER_global.stats[user_idx][VTSS_ISID_LOCAL]));
                }
            }
        } else {
            if (msg->data.counters_clear.port_no == VTSS_PORT_NO_END) {
                memset(DHCP_HELPER_global.stats[msg->data.counters_clear.user][VTSS_ISID_LOCAL], 0, sizeof(DHCP_HELPER_global.stats[msg->data.counters_clear.user][VTSS_ISID_LOCAL]));
            } else {
                memset(&DHCP_HELPER_global.stats[msg->data.counters_clear.user][VTSS_ISID_LOCAL][msg->data.counters_clear.port_no], 0, sizeof(dhcp_helper_stats_t));
            }
        }
        DHCP_HELPER_CRIT_EXIT();
        break;
    }
    default:
        T_W("unknown message ID: %d", msg->msg_id);
        break;
    }
    return TRUE;
}

/* Stack Register */
static vtss_rc DHCP_HELPER_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = DHCP_HELPER_msg_rx;
    filter.modid = VTSS_MODULE_ID_DHCP_HELPER;
    return msg_rx_filter_register(&filter);
}

/* Set stack DHCP helper local ACEs */
static void DHCP_HELPER_stack_ace_conf_set(vtss_isid_t isid_add)
{
    dhcp_helper_msg_t       *msg;
    dhcp_helper_msg_buf_t   buf;
    vtss_isid_t             isid;
    dhcp_helper_user_t      user_idx;

    T_D("enter, isid_add: %d", isid_add);
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if ((isid_add != VTSS_ISID_GLOBAL && isid_add != isid) ||
            !msg_switch_exists(isid)) {
            continue;
        }
        DHCP_HELPER_CRIT_ENTER();
        msg = DHCP_HELPER_msg_alloc(&buf, DHCP_HELPER_MSG_ID_LOCAL_ACE_SET, TRUE);
        msg->data.local_ace_set.add = FALSE;

        for (user_idx = DHCP_HELPER_USER_HELPER; user_idx < DHCP_HELPER_USER_CNT; user_idx++) {
            if (msg->data.local_ace_set.add == FALSE) {
                msg->data.local_ace_set.add = (DHCP_HELPER_rx_cb[user_idx] ? TRUE : FALSE);
            }
        }
        DHCP_HELPER_CRIT_EXIT();
        DHCP_HELPER_msg_tx(&buf, isid, sizeof(msg->data.local_ace_set));
    }

    T_D("exit, isid_add: %d", isid_add);
}


/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* DHCP helper error text */
char *dhcp_helper_error_txt(vtss_rc rc)
{
    switch (rc) {
    case DHCP_HELPER_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case DHCP_HELPER_ERROR_ISID:
        return "Invalid Switch ID";

    case DHCP_HELPER_ERROR_ISID_NON_EXISTING:
        return "Switch ID is non-existing";

    case DHCP_HELPER_ERROR_INV_PARAM:
        return "Invalid parameter supplied to function";

    case DHCP_HELPER_ERROR_REQ_TIMEOUT:
        return "Request timeout";

    case DHCP_HELPER_ERROR_FRAME_BUF_ALLOCATED:
        return "DHCP frame buffer allocated fail";

    default:
        return "DHCP Helper: Unknown error code";
    }
}

/* Get DHCP helper port configuration */
vtss_rc dhcp_helper_mgmt_port_conf_get(vtss_isid_t isid, dhcp_helper_port_conf_t *switch_cfg)
{
    T_D("enter");

    if (switch_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return DHCP_HELPER_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return DHCP_HELPER_ERROR_MUST_BE_MASTER;
    }
    if (!msg_switch_configurable(isid)) {
        T_W("isid: %d isn't configurable switch", isid);
        T_D("exit");
        return DHCP_HELPER_ERROR_ISID_NON_EXISTING;
    }

    DHCP_HELPER_CRIT_ENTER();
    *switch_cfg = DHCP_HELPER_global.port_conf[isid];
    DHCP_HELPER_CRIT_EXIT();

    T_D("exit");
    return VTSS_OK;
}

/* Set DHCP helper port configuration */
vtss_rc dhcp_helper_mgmt_port_conf_set(vtss_isid_t isid, dhcp_helper_port_conf_t *switch_cfg)
{
    vtss_rc         rc = VTSS_OK;
    vtss_port_no_t  port_idx;

    T_D("enter");

    if (switch_cfg == NULL) {
        T_W("not master");
        T_D("exit");
        return DHCP_HELPER_ERROR_INV_PARAM;
    }
    if (!msg_switch_is_master()) {
        T_W("not master");
        T_D("exit");
        return DHCP_HELPER_ERROR_MUST_BE_MASTER;
    }
    if (!msg_switch_configurable(isid)) {
        T_W("isid: %d isn't configurable switch", isid);
        T_D("exit");
        return DHCP_HELPER_ERROR_ISID_NON_EXISTING;
    }

    DHCP_HELPER_CRIT_ENTER();
    DHCP_HELPER_global.port_conf[isid] = *switch_cfg;

    /* Reset stack */
    for (port_idx = VTSS_PORT_NO_START; port_idx < VTSS_PORT_NO_END; port_idx++) {
        if (port_isid_port_no_is_stack(isid, port_idx)) {
            switch_cfg->port_mode[port_idx] = DHCP_HELPER_PORT_MODE_UNTRUSTED;
        }
    }
    DHCP_HELPER_CRIT_EXIT();

    T_D("exit");

    return rc;
}


/****************************************************************************
 * Module thread
 ****************************************************************************/
/* Clear the uncompleted entries in period time */
static void DHCP_HELPER_thread(cyg_addrword_t data)
{
    while (1) {
        if (msg_switch_is_master()) {
            while (msg_switch_is_master()) {
                VTSS_OS_MSLEEP(DHCP_HELPER_FRAME_INFO_MONITOR_INTERVAL);
                DHCP_HELPER_frame_info_clear_obsoleted();
            }
        }

        // Gracefully empty the BIP buffer.
        DHCP_HELPER_bip_buffer_dequeue(TRUE);

        // No reason for using CPU ressources when we're a slave
        T_D("Suspending DHCP helper thread");
        cyg_thread_suspend(DHCP_HELPER_thread_handle);
        T_D("Resumed DHCP helper thread");
    }
}

static void DHCP_HELPER_bip_buffer_thread(cyg_addrword_t data)
{
    while (1) {
        if (msg_switch_is_master()) {
            while (msg_switch_is_master()) {
                (void)cyg_flag_wait(&DHCP_HELPER_bip_buffer_thread_events, DHCP_HELPER_EVENT_ANY, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
                T_D("Dequeueing");
                DHCP_HELPER_bip_buffer_dequeue(FALSE);
            }
        }

        // No reason for using CPU ressources when we're a slave
        T_D("Suspending DHCP helper bip buffer thread");
        cyg_thread_suspend(DHCP_HELPER_bip_buffer_thread_handle);
        T_D("Resumed DHCP helper bip buffer thread");
    }
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Module start */
static void DHCP_HELPER_start(BOOL init)
{
    vtss_rc     rc;
    vtss_isid_t isid_idx;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize callback function reference */
        memset(DHCP_HELPER_rx_cb, 0, sizeof(DHCP_HELPER_rx_cb));
        memset(DHCP_HELPER_clear_local_stat_cb, 0, sizeof(DHCP_HELPER_clear_local_stat_cb));

        /* Initialize DHCP helper port configuration */
        memset(DHCP_HELPER_global.port_conf, 0x0, sizeof(DHCP_HELPER_global.port_conf));
        memset(DHCP_HELPER_global.stats, 0x0, sizeof(DHCP_HELPER_global.stats));

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&DHCP_HELPER_global.request.sem, 1);
        VTSS_OS_SEM_CREATE(&DHCP_HELPER_global.reply.sem, 1);

        /* Initialize BIP buffer */
        if (!vtss_bip_buffer_init(&DHCP_HELPER_bip_buf, DHCP_HELPER_BIP_BUF_TOTAL_SIZE)) {
            T_E("vtss_bip_buffer_init failed!");
        }

        /* Create semaphore for critical regions */
        critd_init(&DHCP_HELPER_global.crit, "DHCP_HELPER_global.crit", VTSS_MODULE_ID_DHCP_HELPER, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        DHCP_HELPER_CRIT_EXIT();
        critd_init(&DHCP_HELPER_global.bip_crit, "DHCP_HELPER_global.bip_crit", VTSS_MODULE_ID_DHCP_HELPER, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        DHCP_HELPER_BIP_CRIT_EXIT();

        /* Initialize counter timers */
        for (isid_idx = VTSS_ISID_START; isid_idx < VTSS_ISID_END; isid_idx++) {
            VTSS_MTIMER_START(&DHCP_HELPER_global.stats_timer[isid_idx], 1);
        }

        /* Initialize counter flag */
        cyg_flag_init(&DHCP_HELPER_global.stats_flags);

        /* Create DHCP helper thread */
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          DHCP_HELPER_thread,
                          0,
                          "DHCP Helper",
                          DHCP_HELPER_thread_stack,
                          sizeof(DHCP_HELPER_thread_stack),
                          &DHCP_HELPER_thread_handle,
                          &DHCP_HELPER_thread_block);

        /* Create DHCP helper bip buffer thread */
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          DHCP_HELPER_bip_buffer_thread,
                          0,
                          "DHCP Helper bip buffer",
                          DHCP_HELPER_bip_buffer_thread_stack,
                          sizeof(DHCP_HELPER_bip_buffer_thread_stack),
                          &DHCP_HELPER_bip_buffer_thread_handle,
                          &DHCP_HELPER_bip_buffer_thread_block);

    } else {
        /* Register port change callback function */
        if ((rc = port_global_change_register(VTSS_MODULE_ID_DHCP_HELPER, DHCP_HELPER_port_global_change_callback)) != VTSS_OK) {
            T_E("Calling port_global_change_register() failed, rc = %d", rc);
        }

        /* Register for stack messages */
        if ((rc = DHCP_HELPER_stack_register()) != VTSS_OK) {
            T_W("VOICE_VLAN_stack_register(): failed rc = %d", rc);
        }
    }
    T_D("exit");
}

/* Initialize module */
vtss_rc dhcp_helper_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&DHCP_HELPER_trace_reg, DHCP_HELPER_trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&DHCP_HELPER_trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        DHCP_HELPER_start(1);
        break;
    case INIT_CMD_START:
        /* Register for stack messages */
        DHCP_HELPER_start(0);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            DHCP_HELPER_CRIT_ENTER();
            memset(DHCP_HELPER_global.stats, 0x0, sizeof(DHCP_HELPER_global.stats));
            DHCP_HELPER_CRIT_EXIT();
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP:
        T_D("MASTER_UP");

        /* Clear statistics */
        DHCP_HELPER_CRIT_ENTER();
        memset(DHCP_HELPER_global.stats, 0x0, sizeof(DHCP_HELPER_global.stats));
        DHCP_HELPER_CRIT_EXIT();

        /* Starting DHCP Helper frame information thread (became master) */
        cyg_thread_resume(DHCP_HELPER_thread_handle);

        /* Starting DHCP Helper BIP buffer thread (became master) */
        cyg_thread_resume(DHCP_HELPER_bip_buffer_thread_handle);
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
        /* Clear statistics */
        DHCP_HELPER_CRIT_ENTER();
        memset(DHCP_HELPER_global.stats, 0x0, sizeof(DHCP_HELPER_global.stats));
        DHCP_HELPER_CRIT_EXIT();

        /* Clear all frame information enteries */
        DHCP_HELPER_frame_info_clear_all();
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        DHCP_HELPER_stack_ace_conf_set(isid);
        T_D("SWITCH_ADD, isid: %d", isid);
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
/*  DHCP helper receive functions                                           */
/****************************************************************************/

// Avoid "Custodual pointer 'msg' has not been freed or returned, since
// the msg is freed by the message module.
/*lint -e{429} */
static void DHCP_HELPER_receive_indication(const u8 *const packet,
                                           size_t len,
                                           vtss_vid_t vid,
                                           vtss_port_no_t switchport,
                                           vtss_glag_no_t glag_no)
{
    T_D("len %zd port %u vid %d glag %u", len, switchport, vid, glag_no);

    if (msg_switch_is_master()) {   /* Bypass message module! */
        DHCP_HELPER_bip_buffer_rx_enqueue(packet, len, vid, msg_master_isid(), switchport, glag_no);
    } else {
        size_t msg_len = sizeof(dhcp_helper_msg_t) + len;
        dhcp_helper_msg_t *msg = VTSS_MALLOC(msg_len);
        if (msg) {
            msg->msg_id = DHCP_HELPER_MSG_ID_FRAME_RX_IND;
            msg->data.rx_ind.len = len;
            msg->data.rx_ind.vid = vid;
            msg->data.rx_ind.port_no = switchport;
            msg->data.rx_ind.glag_no = glag_no;
            memcpy(&msg[1], packet, len); /* Copy frame */
            //These frames are subject to shaping.
            msg_tx_adv(NULL, NULL, MSG_TX_OPT_NO_ALLOC_ON_LOOPBACK | MSG_TX_OPT_SHAPE, VTSS_MODULE_ID_DHCP_HELPER, 0, msg, msg_len);
        } else {
            T_W("Unable to allocate %zd bytes, tossing frame on port %u", msg_len, switchport);
        }
    }
}


/****************************************************************************/
/*  Rx filter register functions                                            */
/****************************************************************************/

/* Local port packet receive indication - forward through DHCP helper */
static BOOL DHCP_HELPER_rx_packet_callback(void *contxt, const u8 *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    u8              *ptr = (u8 *)(frm);
    struct ip       *ip = (struct ip *)(ptr + 14);
    int             ip_header_len = IP_VHL_HL(ip->ip_hl) << 2;
    struct udphdr   *udp_header = (struct udphdr *)(ptr + 14 + ip_header_len); /* 14:DA+SA+ETYPE */
    u16             sport = ntohs(udp_header->uh_sport);
    u16             dport = ntohs(udp_header->uh_dport);
    BOOL            rc = FALSE;

    T_D("enter, port_no: %u len %d vid %d glag %u", rx_info->port_no, rx_info->length, rx_info->tag.vid, rx_info->glag_no);

    if (sport == DHCP_SERVER_UDP_PORT || sport == DHCP_CLINET_UDP_PORT || dport == DHCP_SERVER_UDP_PORT || dport == DHCP_CLINET_UDP_PORT) {
        DHCP_HELPER_receive_indication(frm, rx_info->length, rx_info->tag.vid, rx_info->port_no, rx_info->glag_no);
        rc = TRUE; //Do not allow other subscribers to receive the packet
    }

    T_D("exit");

    return rc;
}

static void DHCP_HELPER_rx_filter_register(BOOL registerd)
{
    DHCP_HELPER_CRIT_ENTER();

    if (!DHCP_HELPER_filter_id) {
        memset(&DHCP_HELPER_rx_filter, 0, sizeof(DHCP_HELPER_rx_filter));
    }

    DHCP_HELPER_rx_filter.modid           = VTSS_MODULE_ID_DHCP_HELPER;
    DHCP_HELPER_rx_filter.match           = PACKET_RX_FILTER_MATCH_ACL | PACKET_RX_FILTER_MATCH_ETYPE | PACKET_RX_FILTER_MATCH_IPV4_PROTO;
    DHCP_HELPER_rx_filter.etype           = 0x0800; //IP
    DHCP_HELPER_rx_filter.ipv4_proto      = 17; //UDP
    DHCP_HELPER_rx_filter.prio            = PACKET_RX_FILTER_PRIO_NORMAL;
    DHCP_HELPER_rx_filter.cb              = DHCP_HELPER_rx_packet_callback;

    if (registerd && !DHCP_HELPER_filter_id) {
        if (packet_rx_filter_register(&DHCP_HELPER_rx_filter, &DHCP_HELPER_filter_id) != VTSS_OK) {
            T_W("DHCP helper module register packet RX filter fail./n");
        }
    } else if (!registerd && DHCP_HELPER_filter_id) {
        if (packet_rx_filter_unregister(DHCP_HELPER_filter_id) == VTSS_OK) {
            DHCP_HELPER_filter_id = NULL;
        }
    }

    DHCP_HELPER_CRIT_EXIT();
}


/****************************************************************************/
/*  Receive register functions                                              */
/****************************************************************************/

/* Register DHCP user frame receive */
void dhcp_helper_user_receive_register(dhcp_helper_user_t user, dhcp_helper_stack_rx_callback_t cb)
{
    BOOL update_ace = FALSE;

    if (user >= DHCP_HELPER_USER_CNT) {
        return;
    }

    DHCP_HELPER_CRIT_ENTER();
    if (!DHCP_HELPER_rx_cb[user]) {
        update_ace = TRUE;
        DHCP_HELPER_rx_cb[user] = cb;
    }
    DHCP_HELPER_CRIT_EXIT();

    if (update_ace) {
        DHCP_HELPER_stack_ace_conf_set(VTSS_ISID_GLOBAL);
    }
}

/* Unregister DHCP user frame receive */
void dhcp_helper_user_receive_unregister(dhcp_helper_user_t user)
{
    BOOL update_ace = FALSE;

    if (user >= DHCP_HELPER_USER_CNT) {
        return;
    }

    DHCP_HELPER_CRIT_ENTER();
    if (DHCP_HELPER_rx_cb[user]) {
        update_ace = TRUE;
        DHCP_HELPER_rx_cb[user] = NULL;
    }
    DHCP_HELPER_CRIT_EXIT();

    if (update_ace) {
        DHCP_HELPER_stack_ace_conf_set(VTSS_ISID_GLOBAL);
    }
}


/****************************************************************************/
/*  Clear local statistics register functions                                              */
/****************************************************************************/
/* Register DHCP user clear local statistics */
#if defined(VTSS_SW_OPTION_DHCP_SERVER) || defined(VTSS_SW_OPTION_DHCP_RELAY)
void dhcp_helper_user_clear_local_stat_register(dhcp_helper_user_t user, dhcp_helper_user_clear_local_stat_callback_t cb)
{
    if (user >= DHCP_HELPER_USER_CNT) {
        return;
    }
    DHCP_HELPER_CRIT_ENTER();
    if (!DHCP_HELPER_clear_local_stat_cb[user]) {
        DHCP_HELPER_clear_local_stat_cb[user] = cb;
    }
    DHCP_HELPER_CRIT_EXIT();
}
#endif /* VTSS_SW_OPTION_DHCP_SERVER || VTSS_SW_OPTION_DHCP_RELAY */

/****************************************************************************/
/*  DHCP helper transmit functions                                          */
/****************************************************************************/

/* Alloc memory for transmit DHCP frame */
void *dhcp_helper_alloc_xmit(size_t len, vtss_isid_t isid, void **pbufref)
{
    void *p = NULL;

    dhcp_helper_msg_t *msg = VTSS_MALLOC(sizeof(dhcp_helper_msg_t) + len);
    if (msg) {
        msg->msg_id = DHCP_HELPER_MSG_ID_FRAME_TX_REQ;
        msg->data.tx_req.len = len;
        msg->data.tx_req.isid = isid;
        *pbufref = (void *)msg; /* Remote op */
        p = ((u8 *)msg) + sizeof(*msg);
    } else {
        T_E("Allocation failure, length %zd", len);
    }

    T_D("%s(%zd) ret %p", __FUNCTION__, len, p);
    return p;
}

/* Transmit DHCP frame.

   Input parameter of "frame_p" must cotains DA + SA + ETYPE + UDP header + UDP payload.
   Calling API dhcp_helper_alloc_xmit() to allocate the resource for "frame_p" and get the "bufref_p".

   Input parameters of "dst_isid", "dst_port_mask", "src_port_no" and "src_glag_no"
   will be ingored when user is "DHCP_HELPER_USER_SERVER" or "DHCP_HELPER_USER_CLIENT".

   Return 0  : Success
   Return -1 : Fail */
int dhcp_helper_xmit(dhcp_helper_user_t user,
                     void               *frame_p,
                     size_t             len,
                     vtss_vid_t         vid,
                     vtss_isid_t        dst_isid,
                     u64                dst_port_mask,
                     vtss_isid_t        src_isid,
                     vtss_port_no_t     src_port_no,
                     vtss_glag_no_t     src_glag_no,
                     void               *bufref_p)
{
    dhcp_helper_bip_buf_t *bip_buf;

    T_D("%s(frame_p = %p, len = %zd, dst_isid = %d, vid = %d, src_port_no = %d, src_glag_no = %d)", __FUNCTION__, frame_p, len, dst_isid, vid, src_port_no, src_glag_no);

    if (!msg_switch_is_master()) {
        // Avoid pushing on the BIP buffer if we're not master.
        goto do_exit_with_error;
    }

    /* Check input parameters. */
    // Check NULL pointer
    if (!frame_p || !bufref_p) {
        T_E("TX frame pointer is NULL");
        goto do_exit_with_error;
    }

    // Check VID
    if ((vid < VLAN_ID_MIN) || (vid > VLAN_ID_MAX)) {
        T_E("Invalid VID (%u)", vid);
        goto do_exit_with_error;
    }

    // Check length
    if (len > sizeof(bip_buf->pkt)) {
        T_E("Unable to transmit more than %zd bytes (requested %zd)", sizeof(bip_buf->pkt), len);
        goto do_exit_with_error;
    }

    // Check isid
#if defined(VTSS_SW_OPTION_DHCP_SERVER)
    if (user == DHCP_HELPER_USER_SERVER) {
        /* Initialize default value */
        dst_isid = VTSS_ISID_GLOBAL;
        dst_port_mask = 0;
        src_port_no = VTSS_PORT_NO_NONE;
        src_glag_no = VTSS_GLAG_NO_NONE;
    }
#endif /* VTSS_SW_OPTION_DHCP_SERVER */

#if defined(VTSS_SW_OPTION_DHCP_CLIENT)
    if (user == DHCP_HELPER_USER_CLIENT) {
        /* Initialize default value */
        dst_isid = VTSS_ISID_GLOBAL;
        dst_port_mask = 0;
        src_port_no = VTSS_PORT_NO_NONE;
        src_glag_no = VTSS_GLAG_NO_NONE;
    }
#endif /* VTSS_SW_OPTION_DHCP_CLIENT */

    if (dst_isid != VTSS_ISID_GLOBAL && !msg_switch_exists(dst_isid)) {
        T_W("ISID %u doesn't exist", dst_isid);
        goto do_exit_with_error;
    }

    // At this point, the original implementation of this function
    // could no longer return with error (-1), so it's OK to make
    // the remainder an asynchronous operation.
    // The reason for making the remainder asynchronous is to
    // be able to return to the current caller so that he can
    // release his mutex, which may be asked for by another module
    // that awaits for it in the message Rx thread. In one circumstance,
    // we also need the message Rx thread, and that's when transmitting
    // to a unicast DMAC. In that case DHCP_HELPER_source_port_lookup()
    // calls mac_mgmt_table_get_next(), which attempts to perform
    // message request/response operations, that will not succeed,
    // because the message Rx thread is occupied.
    // See Bugzilla#13886 for details.

    DHCP_HELPER_BIP_CRIT_ENTER();
    bip_buf = (dhcp_helper_bip_buf_t *)vtss_bip_buffer_reserve(&DHCP_HELPER_bip_buf, DHCP_HELPER_BIP_BUF_ALIGNED_SIZE);

    if (bip_buf == NULL) {
        DHCP_HELPER_BIP_CRIT_EXIT();
        T_D("BIP-buffer ran out of entries");
        goto do_exit_with_error;
    }

    memcpy(bip_buf->pkt, frame_p, len);
    bip_buf->len           = len;
    bip_buf->is_rx         = FALSE;
    bip_buf->vid           = vid;
    bip_buf->src_isid      = src_isid;
    bip_buf->src_port_no   = src_port_no;
    bip_buf->src_glag_no   = src_glag_no;
    bip_buf->dst_isid      = dst_isid;
    bip_buf->dst_port_mask = dst_port_mask;
    bip_buf->user          = user;
    bip_buf->bufref_p      = bufref_p;

    vtss_bip_buffer_commit(&DHCP_HELPER_bip_buf);
    cyg_flag_setbits(&DHCP_HELPER_bip_buffer_thread_events, DHCP_HELPER_EVENT_PKT_TX);
    DHCP_HELPER_BIP_CRIT_EXIT();

    return 0;

do_exit_with_error:
    if (bufref_p) {
        VTSS_FREE(bufref_p);
    }

    return -1;
}

/****************************************************************************/
/*  Statistics functions                                                    */
/****************************************************************************/

/* Wait for reply to request */
static BOOL DHCP_HELPER_req_counter_timeout(dhcp_helper_user_t user,
                                            vtss_isid_t isid,
                                            vtss_port_no_t port_no,
                                            dhcp_helper_msg_id_t msg_id,
                                            vtss_mtimer_t *timer,
                                            cyg_flag_t *flags)
{
    dhcp_helper_msg_t       *msg;
    BOOL                    timeout;
    cyg_flag_value_t        flag;
    cyg_tick_count_t        time_tick;
    dhcp_helper_msg_buf_t   buf;

    T_D("enter, isid: %d", isid);

    DHCP_HELPER_CRIT_ENTER();
    timeout = VTSS_MTIMER_TIMEOUT(timer);
    DHCP_HELPER_CRIT_EXIT();

    if (timeout) {
        T_D("info old, sending GET_REQ(isid=%d)", isid);
        DHCP_HELPER_CRIT_ENTER();
        msg = DHCP_HELPER_msg_alloc(&buf, msg_id, TRUE);
        msg->data.tx_counters_get.user = user;
        msg->data.tx_counters_get.port_no = port_no;
        flag = (1 << isid);
        cyg_flag_maskbits(flags, ~flag);
        DHCP_HELPER_CRIT_EXIT();
        DHCP_HELPER_msg_tx(&buf, isid, sizeof(msg->data.tx_counters_get));
        time_tick = cyg_current_time() + VTSS_OS_MSEC2TICK(DHCP_HELPER_REQ_TIMEOUT * 1000);
        return (cyg_flag_timed_wait(flags, flag, CYG_FLAG_WAITMODE_OR, time_tick) & flag ? 0 : 1);
    }
    return FALSE;
}

/*lint -esym(459,DHCP_HELPER_global) */ //Avoid Lint detecte an unprotected access to variable 'DHCP_HELPER_global.stats_flags'
/* Get DHCP helper statistics */
vtss_rc dhcp_helper_stats_get(dhcp_helper_user_t user, vtss_isid_t isid, vtss_port_no_t port_no, dhcp_helper_stats_t *stats)
{
    dhcp_helper_user_t user_idx, user_idx_start, user_idx_end;

    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    if (!msg_switch_is_master()) {
        T_W("not master");
        return DHCP_HELPER_ERROR_MUST_BE_MASTER;
    }

    if (!msg_switch_configurable(isid)) {
        T_W("isid: %d isn't configurable switch", isid);
        T_D("exit");
        return DHCP_HELPER_ERROR_ISID_NON_EXISTING;
    } else if (!msg_switch_exists(isid)) {
        memset(stats, 0, sizeof(*stats));
        return VTSS_OK;
    }

    if (stats == NULL || port_no >= VTSS_PORT_NO_END || user > DHCP_HELPER_USER_CNT) {
        T_W("illegal port_no: %u", port_no);
        return DHCP_HELPER_ERROR_INV_PARAM;
    }

    if (user == DHCP_HELPER_USER_CNT) {
        user_idx_start = DHCP_HELPER_USER_HELPER;
        user_idx_end = DHCP_HELPER_USER_CNT - 1;
    } else {
        user_idx_start = user_idx_end = user;
    }

    memset(stats, 0, sizeof(*stats));
    for (user_idx = user_idx_start; user_idx <= user_idx_end; user_idx++) {
        if (!msg_switch_is_local(isid) &&   // speed up the process if the "isid" is local switch
            DHCP_HELPER_req_counter_timeout(user_idx,
                                            isid,
                                            port_no,
                                            DHCP_HELPER_MSG_ID_COUNTERS_GET_REQ,
                                            &DHCP_HELPER_global.stats_timer[isid],
                                            &DHCP_HELPER_global.stats_flags)) {
            T_W("timeout, DHCP_HELPER_COUNTERS_GET_REQ");
            return DHCP_HELPER_ERROR_REQ_TIMEOUT;
        }

        if (user != DHCP_HELPER_USER_CNT) {
            DHCP_HELPER_CRIT_ENTER();
            *stats = DHCP_HELPER_global.stats[user][isid][port_no];
            DHCP_HELPER_CRIT_EXIT();
            break;
        }

        DHCP_HELPER_CRIT_ENTER();
        stats->rx_stats.discover_rx           += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.discover_rx;
        stats->rx_stats.offer_rx              += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.offer_rx;
        stats->rx_stats.request_rx            += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.request_rx;
        stats->rx_stats.decline_rx            += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.decline_rx;
        stats->rx_stats.ack_rx                += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.ack_rx;
        stats->rx_stats.nak_rx                += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.nak_rx;
        stats->rx_stats.release_rx            += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.release_rx;
        stats->rx_stats.inform_rx             += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.inform_rx;
        stats->rx_stats.leasequery_rx         += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.leasequery_rx;
        stats->rx_stats.leaseunassigned_rx    += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.leaseunassigned_rx;
        stats->rx_stats.leaseunknown_rx       += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.leaseunknown_rx;
        stats->rx_stats.leaseactive_rx        += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.leaseactive_rx;
        stats->rx_stats.discard_untrust_rx    += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.discard_untrust_rx;
        stats->rx_stats.discard_chksum_err_rx += DHCP_HELPER_global.stats[user_idx][isid][port_no].rx_stats.discard_chksum_err_rx;
        stats->tx_stats.discover_tx           += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.discover_tx;
        stats->tx_stats.offer_tx              += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.offer_tx;
        stats->tx_stats.request_tx            += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.request_tx;
        stats->tx_stats.decline_tx            += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.decline_tx;
        stats->tx_stats.ack_tx                += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.ack_tx;
        stats->tx_stats.nak_tx                += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.nak_tx;
        stats->tx_stats.release_tx            += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.release_tx;
        stats->tx_stats.inform_tx             += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.inform_tx;
        stats->tx_stats.leasequery_tx         += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.leasequery_tx;
        stats->tx_stats.leaseunassigned_tx    += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.leaseunassigned_tx;
        stats->tx_stats.leaseunknown_tx       += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.leaseunknown_tx;
        stats->tx_stats.leaseactive_tx        += DHCP_HELPER_global.stats[user_idx][isid][port_no].tx_stats.leaseactive_tx;
        DHCP_HELPER_CRIT_EXIT();
    }

    T_D("exit, isid: %d, port_no: %u", isid, port_no);
    return VTSS_OK;
}

/* Clear DHCP helper statistics */
vtss_rc dhcp_helper_stats_clear(dhcp_helper_user_t user, vtss_isid_t isid, vtss_port_no_t port_no)
{
    dhcp_helper_msg_t       *msg;
    dhcp_helper_msg_buf_t   buf;

    T_D("enter, isid: %d, port_no: %u", isid, port_no);

    if (!msg_switch_is_master()) {
        T_W("not master");
        return DHCP_HELPER_ERROR_MUST_BE_MASTER;
    }

    if (!msg_switch_configurable(isid)) {
        T_W("isid: %d isn't configurable switch", isid);
        T_D("exit");
        return DHCP_HELPER_ERROR_ISID_NON_EXISTING;
    } else if (!msg_switch_exists(isid)) {
        return VTSS_OK;
    }

    if (port_no >= VTSS_PORT_NO_END || user > DHCP_HELPER_USER_CNT) {
        T_W("illegal port_no: %u", port_no);
        return DHCP_HELPER_ERROR_INV_PARAM;
    }

    DHCP_HELPER_CRIT_ENTER();
    if (user == DHCP_HELPER_USER_CNT) {
        dhcp_helper_user_t user_idx;
        for (user_idx = DHCP_HELPER_USER_HELPER; user_idx < DHCP_HELPER_USER_CNT; user_idx++) {
            memset(&DHCP_HELPER_global.stats[user_idx][isid][port_no], 0, sizeof(dhcp_helper_stats_t));
        }
    } else {
        memset(&DHCP_HELPER_global.stats[user][isid][port_no], 0, sizeof(dhcp_helper_stats_t));
    }
    DHCP_HELPER_CRIT_EXIT();

    // Clear TX counter on slaves
    if (!msg_switch_is_local(isid)) {
        DHCP_HELPER_CRIT_ENTER();
        msg = DHCP_HELPER_msg_alloc(&buf, DHCP_HELPER_MSG_ID_COUNTERS_CLR, TRUE);
        msg->data.counters_clear.user = user;
        msg->data.counters_clear.port_no = VTSS_PORT_NO_END;
        DHCP_HELPER_CRIT_EXIT();
        DHCP_HELPER_msg_tx(&buf, isid, sizeof(msg->data.counters_clear));
    }

    T_D("exit, isid: %d, port_no: %u", isid, port_no);

    return VTSS_OK;
}

/* Clear DHCP helper statistics by user */
vtss_rc dhcp_helper_stats_clear_by_user(dhcp_helper_user_t user)
{
    dhcp_helper_msg_t       *msg;
    dhcp_helper_msg_buf_t   buf;
    switch_iter_t           sit;

    T_D("enter, user: %d", user);

    if (!msg_switch_is_master()) {
        T_W("not master");
        return DHCP_HELPER_ERROR_MUST_BE_MASTER;
    }

    if (user >= DHCP_HELPER_USER_CNT) {
        return DHCP_HELPER_ERROR_INV_PARAM;
    }

    DHCP_HELPER_CRIT_ENTER();
    memset(DHCP_HELPER_global.stats[user], 0, sizeof(DHCP_HELPER_global.stats[user]));
    DHCP_HELPER_CRIT_EXIT();

    // Clear TX counter on slaves
    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID);
    while (switch_iter_getnext(&sit)) {
        DHCP_HELPER_CRIT_ENTER();
        msg = DHCP_HELPER_msg_alloc(&buf, DHCP_HELPER_MSG_ID_COUNTERS_CLR, TRUE);
        msg->data.counters_clear.user = user;
        msg->data.counters_clear.port_no = VTSS_PORT_NO_END;
        DHCP_HELPER_CRIT_EXIT();
        DHCP_HELPER_msg_tx(&buf, sit.isid, sizeof(msg->data.counters_clear));
    }

    /* Synchronize the global overview statistics */
    DHCP_HELPER_CRIT_ENTER();
    if (DHCP_HELPER_clear_local_stat_cb[user]) {
        DHCP_HELPER_clear_local_stat_cb[user]();
    }
    DHCP_HELPER_CRIT_EXIT();

    T_D("exit");

    return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
