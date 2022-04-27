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
#include "vtss_ecos_mutex_api.h"
#include "misc_api.h"
#include "vtss_upnp_api.h"
#include "vtss_upnp.h"
#include "upnp_device_main.h"
#include "acl_api.h"
#ifdef VTSS_SW_OPTION_PACKET
#include "packet_api.h"
#endif /* VTSS_SW_OPTION_PACKET */
#include "ip2_api.h"
#include "port_api.h"

#include <network.h>
#include <pthread.h>
#include <netinet/udp.h>

#ifndef IP_VHL_HL
#define IP_VHL_HL(vhl)      ((vhl) & 0x0f)
#endif /* IP_VHL_HL */

#ifndef IP_VHL_HL
#define IP_VHL_HL(vhl)      ((vhl) & 0x0f)
#endif /* IP_VHL_HL */

#ifdef VTSS_SW_OPTION_VCLI
#include "vtss_upnp_cli.h"
#endif
#ifdef VTSS_SW_OPTION_ICFG
#include "upnp_icfg.h"
#endif

#include "upnp_callout.h"
#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_UPNP

static char upnp_xml_string[] =
#if 1
{
    "<?xml version=\"1.0\"?>"
    "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion>"
    "<major>1</major>"
    "<minor>0</minor>"
    "</specVersion>"
    "<device>"
    "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
    "<friendlyName>SMBStaX</friendlyName>"
    "<manufacturer>Vitesse Semiconductor Corp.</manufacturer>"
    "<manufacturerURL>http://www.vitesse.com</manufacturerURL>"
    "<modelDescription>Layer2+ Giga Stacking Switch SMBStaX</modelDescription>"
    "<modelName></modelName>"
    "<modelNumber></modelNumber>"
    "<serialNumber>A830023251</serialNumber>"
    "<UDN>%s</UDN>"
    "<serviceList>"
    "<service>"
    "<serviceType>urn:schemas-upnp-org:service:Layer4_Layer2:1</serviceType>"
    "<serviceId>urn:upnp-org:serviceId:Layer4_Layer21</serviceId>"
    "<controlURL></controlURL>"
    "<eventSubURL></eventSubURL>"
    "<SCPDURL></SCPDURL>"
    "</service>"
    "</serviceList>"
    "<presentationURL>http://%s:80</presentationURL>"
    "</device>"
    "</root>"
};
#else
    {
        "<?xml version=\"1.0\"?>\n"
        "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\n"
        "<specVersion>\n"
        "<major>1</major>\n"
        "<minor>0</minor>\n"
        "</specVersion>\n"
        "<device>\n"
        "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>\n"
        "<friendlyName>SMBStaX</friendlyName>\n"
        "<manufacturer>Vitesse Semiconductor Corp.</manufacturer>\n"
        "<manufacturerURL>http://www.vitesse.com</manufacturerURL>\n"
        "<modelDescription>Layer2+ Giga Stacking Switch SMBStaX</modelDescription>\n"
        "<modelName></modelName>\n"
        "<modelNumber></modelNumber>\n"
        "<serialNumber>A830023251</serialNumber>\n"
        "<UDN>%s</UDN>\n"
        "<serviceList>\n"
        "<service>\n"
        "<serviceType>urn:schemas-upnp-org:service:Layer2_Layer2:1</serviceType>\n"
        "<serviceId>urn:upnp-org:serviceId:Layer2_Layer21</serviceId>\n"
        "<controlURL></controlURL>\n"
        "<eventSubURL></eventSubURL>\n"
        "<SCPDURL></SCPDURL>\n"
        "</service>\n"
        "</serviceList>\n"
        "<presentationURL>http://%s:80</presentationURL>\n"
        "</device>\n"
        "</root>\n"
    };
#endif

/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

/* Global structure */
static upnp_global_t upnp_global;

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "upnp",
    .descr     = "UPNP"
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

#define UPNP_CRIT_ENTER() critd_enter(&upnp_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define UPNP_CRIT_EXIT()  critd_exit( &upnp_global.crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define UPNP_CRIT_ENTER() critd_enter(&upnp_global.crit)
#define UPNP_CRIT_EXIT()  critd_exit(&upnp_global.crit)
#endif /* VTSS_TRACE_ENABLED */

/* Thread variables */
#define UPNP_CERT_THREAD_STACK_SIZE         8192
#define UPNP_CERT_THREAD_NO                 15
static char upnp_thread_stack[UPNP_CERT_THREAD_STACK_SIZE];
static char memstack_pool[UPNP_CERT_THREAD_NO][UPNP_CERT_THREAD_STACK_SIZE];
static int memstack_used_no = 0;

/* packet rx filter */
#ifdef VTSS_SW_OPTION_PACKET
packet_rx_filter_t  upnp_rx_ssdp_filter;
packet_rx_filter_t  upnp_rx_igmp_filter;
static void         *upnp_ssdp_filter_id = NULL; // Filter id for subscribing upnp packet.
static void         *upnp_igmp_filter_id = NULL; // Filter id for subscribing upnp packet.
#endif /* VTSS_SW_OPTION_PACKET */

/* RX loopback on master */
static vtss_isid_t master_isid = VTSS_ISID_LOCAL;

#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
/* Flag that denotes using SFM via API */
static vtss_vid_t   upnp_sfm_vid = VTSS_VID_NULL;
#endif /* VTSS_FEATURE_IPV4_MC_SIP */

/****************************************************************************/
/*  Various local functions                                                 */
/****************************************************************************/

/* Determine if UPNP configuration has changed */
static int upnp_conf_changed(upnp_conf_t *old, upnp_conf_t *new)
{
    return (new->mode != old->mode ||
            new->ttl != old->ttl ||
            new->adv_interval != old->adv_interval);
}

/* Set UPNP defaults */
void upnp_default_get(upnp_conf_t *conf)
{
    conf->mode = UPNP_MGMT_DISABLED;
    conf->ttl = UPNP_MGMT_DEFAULT_TTL;
    conf->adv_interval = UPNP_MGMT_DEFAULT_INT;
}

/* Allocate request buffer */
static upnp_msg_req_t *
upnp_alloc_pkt_message(size_t size, upnp_msg_id_t msg_id)
{
    upnp_msg_req_t *msg = VTSS_MALLOC(size);
    if (msg) {
        msg->msg_id = msg_id;
    }
    T_D("msg len %zu, type %d => %p", size, msg_id, msg);
    return msg;
}

/****************************************************************************/
/*  Callback functions                                                      */
/****************************************************************************/

static void
upnp_do_rx_callback(const void *packet,
                    size_t len,
                    ulong vid,
                    ulong isid,
                    ulong port_no)
{
    vtss_if_id_vlan_t   ingress_vid;
    vtss_rc             rc;

    T_D("enter, RX isid %d vid %d port %d len %zu", isid, vid, port_no, len);

    ingress_vid = vid & 0xFFFF;
    if ((rc = vtss_ip2_if_inject(ingress_vid, len, packet)) != VTSS_OK) {
        T_D("Failed(%d) in vtss_ip2_if_inject(%u, %d)!\n", rc, ingress_vid, len);
    }
    T_D("exit");
}

/****************************************************************************/
/*  Reserved ACEs functions                                                 */
/****************************************************************************/
/* Add reserved ACE */
static vtss_rc upnp_ace_add(void)
{
    acl_entry_conf_t    conf;
    vtss_rc             rc;

    if ((rc = acl_mgmt_ace_init(VTSS_ACE_TYPE_IPV4, &conf)) != VTSS_OK) {
        return rc;
    }
    conf.id = UPNP_SSDP_ACE_ID;
    conf.action.force_cpu = TRUE;
    conf.action.cpu_once = FALSE;
    conf.isid = VTSS_ISID_LOCAL;
    conf.frame.ipv4.proto.value = 17; //UDP
    conf.frame.ipv4.proto.mask = 0xFF;
    conf.frame.ipv4.sport.in_range = conf.frame.ipv4.dport.in_range = 1;
    conf.frame.ipv4.sport.high = 65535;
    conf.frame.ipv4.dport.low = conf.frame.ipv4.dport.high = UPNP_UDP_PORT;

    rc = acl_mgmt_ace_add(ACL_USER_UPNP, ACE_ID_NONE, &conf);
    if (rc != VTSS_OK) {
        return rc;
    }

    if ((rc = acl_mgmt_ace_init(VTSS_ACE_TYPE_IPV4, &conf)) != VTSS_OK) {
        return rc;
    }
    conf.id = IGMP_QUERY_ACE_ID;
    conf.action.force_cpu = TRUE;
    conf.action.cpu_once = FALSE;
    conf.isid = VTSS_ISID_LOCAL;
    conf.frame.ipv4.dip.value = 0xe0000001; /* 224.0.0.1 */
    conf.frame.ipv4.dip.mask = 0xffffffff; /* 255.255.255.255 */
    conf.frame.ipv4.sport.in_range = conf.frame.ipv4.dport.in_range = TRUE;
    conf.frame.ipv4.sport.high = conf.frame.ipv4.dport.high = 65535;

    return (acl_mgmt_ace_add(ACL_USER_UPNP, ACE_ID_NONE, &conf));
}

/* Delete reserved ACE */
static vtss_rc upnp_ace_del(void)
{
    vtss_rc rc;

    rc = acl_mgmt_ace_del(ACL_USER_UPNP, UPNP_SSDP_ACE_ID);
    if (rc != VTSS_OK) {
        return rc;
    }
    return (acl_mgmt_ace_del(ACL_USER_UPNP, IGMP_QUERY_ACE_ID));
}

/****************************************************************************/
/*  UPNP receive functions                                           */
/****************************************************************************/

static void upnp_receive_indication(const void *packet,
                                    size_t len,
                                    vtss_port_no_t switchport,
                                    vtss_vid_t vid,
                                    vtss_glag_no_t glag_no)
{
    T_D("len %zu port %u vid %d glag %u", len, switchport, vid, glag_no);

    if (msg_switch_is_master() && VTSS_ISID_LEGAL(master_isid)) {   /* Bypass message module! */
        upnp_do_rx_callback(packet, len, vid, master_isid, switchport);
    } else {
        size_t msg_len = sizeof(upnp_msg_req_t) + len;
        upnp_msg_req_t *msg = upnp_alloc_pkt_message(msg_len, UPNP_MSG_ID_FRAME_RX_IND);
        if (msg) {
            msg->req.rx_ind.len = len;
            msg->req.rx_ind.vid = vid;
            msg->req.rx_ind.port_no = switchport;
            memcpy(&msg[1], packet, len); /* Copy frame */
            msg_tx(VTSS_MODULE_ID_UPNP, 0, msg, msg_len);
        } else {
            T_W("Unable to allocate %zu bytes, tossing frame on port %u", msg_len, switchport);
        }
    }
}

/****************************************************************************/
/*  Rx filter register functions                                            */
/****************************************************************************/

/* Local port packet receive indication - forward through UPnP */
static BOOL upnp_rx_packet_callback(void  *contxt, const uchar *const frm, const vtss_packet_rx_info_t *const rx_info)
{
    uchar            *ptr = (uchar *)(frm);
    struct ip       *ip = (struct ip *)(ptr + 14);
    int             ip_header_len = IP_VHL_HL(ip->ip_hl) << 2;
    struct udphdr   *udp_header = (struct udphdr *)(ptr + 14 + ip_header_len); /* 14:DA+SA+ETYPE */
    ushort          dport = ntohs(udp_header->uh_dport);

    if (msg_switch_is_master() && VTSS_ISID_LEGAL(master_isid)) {   /* Bypass message module! */
        /*  If this is a master, let the packet go to IP layer naturally  */
        return FALSE;

    } else if ((ip->ip_p == 17 && dport == UPNP_UDP_PORT) || ip->ip_p == 2) {
        /* If this a slave, use the message to pack the packet and then transmit to the master */
        T_D("enter, port_no: %u len %d vid %d glag %u", rx_info->port_no, rx_info->length, rx_info->tag.vid, rx_info->glag_no);

        // NB: Null out the GLAG (port is 1st in aggr)
        upnp_receive_indication(frm, rx_info->length, rx_info->port_no, rx_info->tag.vid,
                                (vtss_glag_no_t)(VTSS_GLAG_NO_START - 1));

        T_D("exit");

        return TRUE; // Do not allow other subscribers to receive the packet
    }

    return FALSE;
}

void upnp_rx_filter_register(BOOL registerd)
{
#ifdef VTSS_SW_OPTION_PACKET
    UPNP_CRIT_ENTER();

    if (!upnp_ssdp_filter_id) {
        memset(&upnp_rx_ssdp_filter, 0, sizeof(upnp_rx_ssdp_filter));
    }
    if (!upnp_igmp_filter_id) {
        memset(&upnp_rx_igmp_filter, 0, sizeof(upnp_rx_igmp_filter));
    }

    upnp_rx_ssdp_filter.modid           = VTSS_MODULE_ID_UPNP;
    upnp_rx_ssdp_filter.match           = PACKET_RX_FILTER_MATCH_ACL | PACKET_RX_FILTER_MATCH_ETYPE | PACKET_RX_FILTER_MATCH_IPV4_PROTO;
    upnp_rx_ssdp_filter.etype           = 0x0800; // IP
    upnp_rx_ssdp_filter.ipv4_proto      = 17; //UDP
    upnp_rx_ssdp_filter.prio            = PACKET_RX_FILTER_PRIO_NORMAL;
    upnp_rx_ssdp_filter.cb              = upnp_rx_packet_callback;

    upnp_rx_igmp_filter.modid           = VTSS_MODULE_ID_UPNP;
    upnp_rx_igmp_filter.match           = PACKET_RX_FILTER_MATCH_ACL | PACKET_RX_FILTER_MATCH_ETYPE | PACKET_RX_FILTER_MATCH_IPV4_PROTO;
    upnp_rx_igmp_filter.etype           = 0x0800; // IP
    upnp_rx_igmp_filter.ipv4_proto      = 2; //IGMP
    upnp_rx_igmp_filter.prio            = PACKET_RX_FILTER_PRIO_NORMAL;
    upnp_rx_igmp_filter.cb              = upnp_rx_packet_callback;

    if (registerd && !upnp_ssdp_filter_id) {
        if (packet_rx_filter_register(&upnp_rx_ssdp_filter, &upnp_ssdp_filter_id) != VTSS_OK) {
            T_W("UPNP module register packet RX filter fail./n");
        }
    } else if (!registerd && upnp_ssdp_filter_id) {
        if (packet_rx_filter_unregister(upnp_ssdp_filter_id) == VTSS_OK) {
            upnp_ssdp_filter_id = NULL;
        }
    }

    if (registerd && !upnp_igmp_filter_id) {
        if (packet_rx_filter_register(&upnp_rx_igmp_filter, &upnp_igmp_filter_id) != VTSS_OK) {
            T_W("UPNP module register packet RX filter fail./n");
        }
    } else if (!registerd && upnp_igmp_filter_id) {
        if (packet_rx_filter_unregister(upnp_igmp_filter_id) == VTSS_OK) {
            upnp_igmp_filter_id = NULL;
        }
    }

    UPNP_CRIT_EXIT();
#endif /* VTSS_SW_OPTION_PACKET */
}

#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
static void upnp_tx_mac_register(const vtss_vid_t  vid,
                                 const vtss_ipv4_t dip)
{
    BOOL            member[VTSS_PORT_ARRAY_SIZE];
    port_iter_t     pit;

    if (vid == VTSS_VID_NULL) {
        return;
    } else {
        T_D("VID: %u; upnp_sfm_vid: %u", vid, upnp_sfm_vid);
    }

    memset(member, 0x0, sizeof(member));
    (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
    while (port_iter_getnext(&pit)) {
        member[pit.iport] = TRUE;
    }

#if defined(VTSS_FEATURE_IPV4_MC_SIP)
    if (vtss_ipv4_mc_add(NULL, vid, ntohl(0), dip, member) != VTSS_OK) {
        T_E("vtss_ipv4_mc_add failed");
    } else {
        UPNP_CRIT_ENTER(); /* Protect global variable upnp_sfm_vid */
        upnp_sfm_vid = vid;
        UPNP_CRIT_EXIT();
        T_D("vtss_ipv4_mc_add successed");
    }
#endif
}
#endif

/****************************************************************************/
/*  Receive register functions                                              */
/****************************************************************************/

#define SSDP_MC_IPADDR    0xeffffffa /* 239.255.255.250 */
/* Register UPnP receive */
void upnp_receive_register(void)
{
#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_vid_t          intf_vid;
    vtss_vid_t          intf_ifid;  /* With IP2, VID is used as the IFID */
    vtss_ipv4_t         intf_adr;
    vtss_if_status_t    *ops, ipst[UPNP_IP_INTF_MAX_OPST];
    u32                 ops_idx;
    u32                 ops_cnt;
#endif /* VTSS_FEATURE_IPV4_MC_SIP || VTSS_ARCH_JAGUAR_1 || VTSS_ARCH_SERVAL */

    upnp_rx_filter_register(TRUE);
    if (upnp_ace_add() != VTSS_OK) {
        T_E("Calling upnp_receive_register() failed.\n");
        return;
    }

#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    intf_vid = VTSS_VID_NULL;

    intf_adr = 0;
    ops_cnt = 0;
    memset(ipst, 0x0, sizeof(ipst));
    intf_ifid = VTSS_VID_NULL;
    if (UPNP_IP_INTF_IFID_GET_NEXT(intf_ifid) &&
        UPNP_IP_INTF_OPST_GET(intf_ifid, ipst, ops_cnt)) {
        T_D("INTF-VID-%u", intf_ifid);

        if (!(ops_cnt > UPNP_IP_INTF_MAX_OPST)) {
            for (ops_idx = 0; ops_idx < ops_cnt; ops_idx++) {
                ops = &ipst[ops_idx];

                if ((intf_vid == VTSS_VID_NULL) && UPNP_IP_INTF_OPST_VID(ops)) {
                    intf_vid = UPNP_IP_INTF_OPST_VID(ops);
                }
                if (ops->type == VTSS_IF_STATUS_TYPE_IPV4) {
                    intf_adr = UPNP_IP_INTF_OPST_ADR4(ops);
                }
            }
        }
    }

    if ((intf_vid == VTSS_VID_NULL) || !intf_adr) {
        return;
    }

    upnp_tx_mac_register(intf_vid, SSDP_MC_IPADDR);
#endif /* VTSS_FEATURE_IPV4_MC_SIP || VTSS_ARCH_JAGUAR_1 || VTSS_ARCH_SERVAL */
}

/* Unregister UPnP receive */
void upnp_receive_unregister(void)
{
#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    vtss_vid_t          intf_vid;
    vtss_vid_t          intf_ifid;  /* With IP2, VID is used as the IFID */
    vtss_ipv4_t         intf_adr;
    vtss_if_status_t    *ops, ipst[UPNP_IP_INTF_MAX_OPST];
    u32                 ops_idx;
    u32                 ops_cnt;

    intf_adr = 0;
    intf_vid = VTSS_VID_NULL;
    ops_cnt = 0;
    memset(ipst, 0x0, sizeof(ipst));
    intf_ifid = VTSS_VID_NULL;
    if (UPNP_IP_INTF_IFID_GET_NEXT(intf_ifid) &&
        UPNP_IP_INTF_OPST_GET(intf_ifid, ipst, ops_cnt)) {
        T_D("INTF-VID-%u", intf_ifid);

        if (!(ops_cnt > UPNP_IP_INTF_MAX_OPST)) {
            for (ops_idx = 0; ops_idx < ops_cnt; ops_idx++) {
                ops = &ipst[ops_idx];

                if ((intf_vid == VTSS_VID_NULL) && UPNP_IP_INTF_OPST_VID(ops)) {
                    intf_vid = UPNP_IP_INTF_OPST_VID(ops);
                }
                if (ops->type == VTSS_IF_STATUS_TYPE_IPV4) {
                    intf_adr = UPNP_IP_INTF_OPST_ADR4(ops);
                }
            }
        }

        if (!intf_adr) {
            intf_vid = VTSS_VID_NULL;
        }
    }

    upnp_tx_mac_register(intf_vid, SSDP_MC_IPADDR);
#endif

    if (upnp_ace_del() != VTSS_OK) {
        T_D("Calling upnp_receive_unregister() failed.\n");
    }
    upnp_rx_filter_register(FALSE);
}


/* update MAC address table the VLAN ID of IP muticast address 239.255.255.250 */
void upnp_mac_mc_update(vtss_vid_t old_vid, vtss_vid_t new_vid)
{
#if 0
    /* Comment it as the part of Bug 8540 workaroud*/
    if (upnp_global.upnp_conf.mode != UPNP_MGMT_ENABLED) {
        return;
    }
#endif
#if defined(VTSS_FEATURE_IPV4_MC_SIP) || defined(VTSS_ARCH_JAGUAR_1) || defined(VTSS_ARCH_SERVAL)
    /* delete the old one if VID changed */
    if (new_vid == old_vid) {
        return;
    }

    T_D("VID: %u->%u; upnp_sfm_vid: %u", old_vid, new_vid, upnp_sfm_vid);
    /*
        old_vid may be notified zero after upnp_sfm_vid is set
        (By upnp_tx_mac_register or upnp_receive_unregister)
        We only need to delete the active one with upnp_sfm_vid
    */
    UPNP_CRIT_ENTER(); /* Protect global variable upnp_sfm_vid */
#if defined(VTSS_FEATURE_IPV4_MC_SIP)
    if (upnp_sfm_vid != VTSS_VID_NULL) {
        if (vtss_ipv4_mc_del(NULL, upnp_sfm_vid, ntohl(0), SSDP_MC_IPADDR) != VTSS_OK) {
            T_E("vtss_ipv4_mc_del(%u) failed %u->%u", upnp_sfm_vid, old_vid, new_vid);
        } else {
            upnp_sfm_vid = VTSS_VID_NULL;
            T_D("vtss_ipv4_mc_del(%u) successed %u->%u", upnp_sfm_vid, old_vid, new_vid);
        }
    }
#else
    {
        vtss_vid_mac_t  vid_mac;
        mac_addr_t       SSDP_MC_MACADDR = {0x01, 0x00, 0x5e, 0x7f, 0xff, 0xfa};

        memset(&vid_mac, 0x0, sizeof(vtss_vid_mac_t));
        memcpy(vid_mac.mac.addr, SSDP_MC_MACADDR, 6);
        vid_mac.vid = upnp_sfm_vid;

        if (upnp_sfm_vid != VTSS_VID_NULL) {
            if (vtss_mac_table_del(NULL, &vid_mac) != VTSS_OK) {
                T_E("vtss_ipv4_mc_del(%u) failed %u->%u", upnp_sfm_vid, old_vid, new_vid);
            } else {
                upnp_sfm_vid = VTSS_VID_NULL;
                T_D("vtss_ipv4_mc_del(%u) successed %u->%u", upnp_sfm_vid, old_vid, new_vid);
            }
        }
    }
#endif
    UPNP_CRIT_EXIT();

    /* Add the new one */
    upnp_tx_mac_register(new_vid, SSDP_MC_IPADDR);
#endif
}

/****************************************************************************/
/*  Stack/switch functions                                                  */
/****************************************************************************/

#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
static char *upnp_msg_id_txt(upnp_msg_id_t msg_id)
{
    char *txt;

    switch (msg_id) {
    case UPNP_MSG_ID_UPNP_CONF_SET_REQ:
        txt = "UPNP_MSG_ID_UPNP_CONF_SET_REQ";
        break;
    case UPNP_MSG_ID_FRAME_RX_IND:
        txt = "UPNP_MSG_ID_FRAME_RX_IND";
        break;
    default:
        txt = "?";
        break;
    }
    return txt;
}
#endif /* VTSS_TRACE_LVL_DEBUG */

/* Allocate request buffer */
static upnp_msg_req_t *upnp_msg_req_alloc(upnp_msg_buf_t *buf, upnp_msg_id_t msg_id)
{
    upnp_msg_req_t *msg = &upnp_global.request.msg;

    buf->sem = &upnp_global.request.sem;
    buf->msg = msg;
    (void) VTSS_OS_SEM_WAIT(buf->sem);
    msg->msg_id = msg_id;
    return msg;
}

/* Free request/reply buffer */
static void upnp_msg_free(vtss_os_sem_t *sem)
{
    VTSS_OS_SEM_POST(sem);
}

static void upnp_msg_tx_done(void *contxt, void *msg, msg_tx_rc_t rc)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    upnp_msg_id_t msg_id = *(upnp_msg_id_t *)msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s", msg_id, upnp_msg_id_txt(msg_id));
    upnp_msg_free(contxt);
}

static void upnp_msg_tx(upnp_msg_buf_t *buf, vtss_isid_t isid, size_t len)
{
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
    upnp_msg_id_t msg_id = *(upnp_msg_id_t *)buf->msg;
#endif /* VTSS_TRACE_LVL_DEBUG */

    T_D("msg_id: %d, %s, len: %zu, isid: %d", msg_id, upnp_msg_id_txt(msg_id), len, isid);
    msg_tx_adv(buf->sem, upnp_msg_tx_done, MSG_TX_OPT_DONT_FREE,
               VTSS_MODULE_ID_UPNP, isid, buf->msg, len + MSG_TX_DATA_HDR_LEN(upnp_msg_req_t, req));
}

static BOOL upnp_msg_rx(void *contxt, const void *const rx_msg, const size_t len, const vtss_module_id_t modid, const ulong isid)
{
    upnp_msg_id_t msg_id = *(upnp_msg_id_t *)rx_msg;
    upnp_msg_req_t *msg = (void *)rx_msg;

    T_D("msg_id: %d, %s, len: %zu, isid: %u", msg_id, upnp_msg_id_txt(msg_id), len, isid);

    switch (msg_id) {
    case UPNP_MSG_ID_UPNP_CONF_SET_REQ: {
        if (msg->req.conf_set.conf.mode == UPNP_MGMT_ENABLED) {
            upnp_receive_register();
        } else {
            upnp_receive_unregister();
        }
        break;
    }
    case UPNP_MSG_ID_FRAME_RX_IND: {
        upnp_do_rx_callback(&msg[1], msg->req.rx_ind.len, msg->req.rx_ind.vid, isid, msg->req.rx_ind.port_no);
        break;
    }
    default:
        T_W("unknown message ID: %d", msg_id);
        break;
    }
    return TRUE;
}

static vtss_rc upnp_stack_register(void)
{
    msg_rx_filter_t filter;

    memset(&filter, 0, sizeof(filter));
    filter.cb = upnp_msg_rx;
    filter.modid = VTSS_MODULE_ID_UPNP;
    return msg_rx_filter_register(&filter);
}

/* Set stack UPNP configuration */
static vtss_rc upnp_stack_upnp_conf_set(vtss_isid_t isid_add)
{
    upnp_msg_req_t  *msg;
    upnp_msg_buf_t  buf;
    vtss_isid_t     isid;

    T_D("enter, isid_add: %d", isid_add);
    for (isid = VTSS_ISID_START; isid < VTSS_ISID_END; isid++) {
        if ((isid_add != VTSS_ISID_GLOBAL && isid_add != isid) ||
            !msg_switch_exists(isid)) {
            continue;
        }
        UPNP_CRIT_ENTER();
        msg = upnp_msg_req_alloc(&buf, UPNP_MSG_ID_UPNP_CONF_SET_REQ);
        msg->req.conf_set.conf = upnp_global.upnp_conf;
        UPNP_CRIT_EXIT();
        upnp_msg_tx(&buf, isid, sizeof(msg->req.conf_set.conf));
    }

    T_D("exit, isid_add: %d", isid_add);
    return VTSS_OK;
}

#define UPNP_STATUS_ENABLED     1
#define UPNP_STATUS_DISABLED    0

void *upnp_base_pthread( void *args )
{
    upnp_conf_t      conf;
    unsigned long    pre_upnp_status = 0;
    char             ip_address[16];

    if (args) {
        ;
    }

    while (vtss_upnp_get_ip(ip_address) != 0) { /* waiting for IP address ready */
        VTSS_OS_MSLEEP(1000);
    }

    if (upnp_mgmt_conf_get(&conf) == VTSS_OK) {
        if (msg_switch_is_master() && conf.mode == UPNP_MGMT_ENABLED) {
            upnp_device_start();
            pre_upnp_status = UPNP_STATUS_ENABLED;
            T_D("UPNP_STATUS_ENABLED");
        } else {
            pre_upnp_status = UPNP_STATUS_DISABLED;
        }
    }


    while (1) {

        if (upnp_mgmt_conf_get(&conf) == VTSS_OK) {
            if (msg_switch_is_master()) {
                if (conf.mode == UPNP_MGMT_ENABLED &&
                    pre_upnp_status == UPNP_STATUS_DISABLED) {

                    upnp_device_start();
                    pre_upnp_status = UPNP_STATUS_ENABLED;
                    T_D("UPNP_STATUS_ENABLED");
                }
                if (conf.mode == UPNP_MGMT_DISABLED &&
                    pre_upnp_status == UPNP_STATUS_ENABLED) {

                    upnp_device_stop();
                    pre_upnp_status = UPNP_STATUS_DISABLED;
                    T_D("UPNP_STATUS_DISABLED");
                }
            } else {
                /* non master */
                if (pre_upnp_status == UPNP_STATUS_ENABLED) {
                    upnp_device_stop();
                    pre_upnp_status = UPNP_STATUS_DISABLED;
                    T_D("UPNP_STATUS_DISABLED");
                }
            }
        }

        VTSS_OS_MSLEEP(2000);
    }


}


static void upnp_create_pthread(void)
{
    pthread_t upnp_thread;
    pthread_attr_t attr;
    struct sched_param schedparam;
    int rc;

    /* Create UPNP thread */
    rc = pthread_attr_init( &attr );
    if (rc != 0) {
        T_W("upnp_creat_pthread: pthread_attr_init fails");
    }

    schedparam.sched_priority = 10;
    rc = pthread_attr_setinheritsched( &attr, PTHREAD_EXPLICIT_SCHED );
    if (rc != 0) {
        T_W("upnp_creat_pthread: pthread_attr_setinheritsched fails");
    }
    rc = pthread_attr_setschedpolicy( &attr, SCHED_RR );
    if (rc != 0) {
        T_W("upnp_creat_pthread: pthread_attr_setschedpolicy fails");
    }
    rc = pthread_attr_setschedparam( &attr, &schedparam );
    if (rc != 0) {
        T_W("upnp_creat_pthread: pthread_attr_setschedparam fails");
    }
    UPNP_CRIT_ENTER();
    rc = pthread_attr_setstackaddr( &attr, (void *)&upnp_thread_stack[sizeof(upnp_thread_stack)] );
    if (rc != 0) {
        T_W("upnp_creat_pthread: pthread_attr_setstackaddr fails");
    }
    UPNP_CRIT_EXIT();
    rc = pthread_attr_setstacksize( &attr, sizeof(upnp_thread_stack) );
    if (rc != 0) {
        T_W("upnp_creat_pthread: pthread_attr_setstacksize fails");
    }

    rc = pthread_create( &upnp_thread, &attr, upnp_base_pthread, NULL );
    if (rc != 0) {
        T_W("upnp_creat_pthread: pthread_create fails");
    } else {
        T_D( "upnp_init \n");
    }
}

/****************************************************************************/
/*  UPnP Base module callout functions                                      */
/****************************************************************************/

void *upnp_callout_malloc(size_t size)
{
    return VTSS_MALLOC(size);
}

void *upnp_callout_realloc(void *ptr, size_t size)
{
    return VTSS_REALLOC(ptr, size);
}

char *upnp_callout_strdup(const char *str)
{
    return VTSS_STRDUP(str);
}

void upnp_callout_free(void *ptr)
{
    VTSS_FREE(ptr);
}

/****************************************************************************/
/*  Management functions                                                    */
/****************************************************************************/

/* UPNP error text */
char *upnp_error_txt(vtss_rc rc)
{
    char *txt;

    switch (rc) {
    case UPNP_ERROR_GEN:
        txt = "UPNP generic error";
        break;
    case UPNP_ERROR_PARM:
        txt = "UPNP parameter error";
        break;
    case UPNP_ERROR_STACK_STATE:
        txt = "UPNP stack state error";
        break;
    default:
        txt = "UPNP unknown error";
        break;
    }
    return txt;
}

/* Get UPNP configuration */
vtss_rc upnp_mgmt_conf_get(upnp_conf_t *conf)
{
    T_D("enter");
    UPNP_CRIT_ENTER();

    *conf = upnp_global.upnp_conf;

    UPNP_CRIT_EXIT();
    T_D("exit");

    return VTSS_OK;
}

/* Set UPNP configuration */
vtss_rc upnp_mgmt_conf_set(upnp_conf_t *conf)
{
    vtss_rc         rc      = VTSS_OK;
    int             changed = 0;
#if 0
    unsigned long   current_mode = 0, config_mode = 0;
#endif
    T_D("enter, mode: %ld", conf->mode);

    /* check illegal parameter */
    if (conf->mode != UPNP_MGMT_ENABLED && conf->mode != UPNP_MGMT_DISABLED) {
        return ((vtss_rc) UPNP_ERROR_PARM);
    }
    if (conf->ttl < UPNP_MGMT_MIN_TTL) {
        /*
         *  The condition conf->ttl > UPNP_MGMT_MAX_TTL is always false because
         *  ttl is declared as a unsigned char.
         */
        return ((vtss_rc) UPNP_ERROR_PARM);
    }
    if (conf->adv_interval < UPNP_MGMT_MIN_INT || conf->adv_interval > UPNP_MGMT_MAX_INT) {
        return ((vtss_rc) UPNP_ERROR_PARM);
    }

    UPNP_CRIT_ENTER();
    if (msg_switch_is_master()) {
#if 0
        current_mode = upnp_global.upnp_conf.mode;
#endif
        changed = upnp_conf_changed(&upnp_global.upnp_conf, conf);
        upnp_global.upnp_conf = *conf;
#if 0
        config_mode = upnp_global.upnp_conf.mode;
#endif
    } else {
        T_W("not master");
        rc = (vtss_rc) UPNP_ERROR_STACK_STATE;
    }
    UPNP_CRIT_EXIT();

    if (changed) {
#if 0
        if (config_mode != current_mode) {
            if ( config_mode == UPNP_MGMT_ENABLED) {
                upnp_create_pthread();
            }  else {
                upnp_device_stop();
            }
        }
#endif

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
        /* Save changed configuration */
        conf_blk_id_t   blk_id = CONF_BLK_UPNP_CONF;
        upnp_conf_blk_t *upnp_conf_blk_p;
        if ((upnp_conf_blk_p = conf_sec_open(CONF_SEC_GLOBAL, blk_id, NULL)) == NULL) {
            T_W("failed to open UPNP table");
        } else {
            upnp_conf_blk_p->upnp_conf = *conf;
            conf_sec_close(CONF_SEC_GLOBAL, blk_id);
        }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

        /* Activate changed configuration */
        rc = upnp_stack_upnp_conf_set(VTSS_ISID_GLOBAL);
        if (rc != VTSS_OK) {
            T_W("upnp_mgmt_conf_set: upnp_stack_upnp_conf_set fails");
        }
    }

    T_D("exit");

    return rc;
}

/* Get UPNP xml file */
void vtss_upnp_xml_get(char **xml)
{
    UPNP_CRIT_ENTER();
    *xml = &upnp_xml_string[0];
    UPNP_CRIT_EXIT();
}

char *vtss_upnp_get_memstack(int *size)
{
    if (memstack_used_no + 1 > UPNP_CERT_THREAD_NO) {
        T_D("vtss_upnp_get_memstack - the number of the created UPnP threads exceed the design\n");
        *size = 0;
        return 0;
    } else {
        T_D("memstack_used_no: %d\n", memstack_used_no);
        *size = UPNP_CERT_THREAD_STACK_SIZE;
        return (&memstack_pool[memstack_used_no++][0]);
    }
}

void vtss_upnp_free_all_memstack(void)
{
    memstack_used_no = 0;
}


void vtss_upnp_get_location(char *location)
{
    char                ip_address[UPNP_MGMT_IPSTR_SIZE];
    vtss_ipv4_t         intf_adr;
    vtss_vid_t          intf_ifid;  /* With IP2, VID is used as the IFID */
    vtss_if_status_t    *ops, ipst[UPNP_IP_INTF_MAX_OPST];
    u32                 ops_idx;
    u32                 ops_cnt;

    if (!location) {
        return;
    }

    intf_adr = 0;
    /* FIXME!SGETZ: Should be based on per-interface-ifid */
    ops_cnt = 0;
    memset(ipst, 0x0, sizeof(ipst));
    intf_ifid = VTSS_VID_NULL;
    if (UPNP_IP_INTF_IFID_GET_NEXT(intf_ifid) &&
        UPNP_IP_INTF_OPST_GET(intf_ifid, ipst, ops_cnt)) {
        T_D("INTF-VID-%u", intf_ifid);

        if (!(ops_cnt > UPNP_IP_INTF_MAX_OPST)) {
            for (ops_idx = 0; ops_idx < ops_cnt; ops_idx++) {
                ops = &ipst[ops_idx];

                if (ops->type == VTSS_IF_STATUS_TYPE_IPV4) {
                    intf_adr = UPNP_IP_INTF_OPST_ADR4(ops);
                }
            }
        }
    }

    (void)misc_ipv4_txt(intf_adr, ip_address);
    (void)snprintf(location, UPNP_MGMT_DESC_SIZE, "http://%s:%d/%s", ip_address, UPNP_MGMT_XML_PORT, UPNP_MGMT_DESC_DOC_NAME);
}


int vtss_upnp_get_ip(char *ipaddr)
{
    vtss_vid_t          intf_ifid;  /* With IP2, VID is used as the IFID */
    vtss_ipv4_t         intf_adr;
    vtss_if_status_t    *ops, ipst[UPNP_IP_INTF_MAX_OPST];
    u32                 ops_idx;
    u32                 ops_cnt;

    if (!ipaddr) {
        return 1;
    }

    intf_adr = 0;
    /* FIXME!SGETZ: Should be based on per-interface-ifid */
    ops_cnt = 0;
    memset(ipst, 0x0, sizeof(ipst));
    intf_ifid = VTSS_VID_NULL;
    while (UPNP_IP_INTF_IFID_GET_NEXT(intf_ifid) &&
           UPNP_IP_INTF_OPST_GET(intf_ifid, ipst, ops_cnt)) {
        T_D("INTF-VID-%u", intf_ifid);

        if (!(ops_cnt > UPNP_IP_INTF_MAX_OPST)) {
            for (ops_idx = 0; ops_idx < ops_cnt; ops_idx++) {
                ops = &ipst[ops_idx];

                if (ops->type == VTSS_IF_STATUS_TYPE_IPV4) {
                    intf_adr = UPNP_IP_INTF_OPST_ADR4(ops);
                }
            }
        }

        if (intf_adr) {
            (void) misc_ipv4_txt(intf_adr, ipaddr);
            return 0;
        } else {
            T_D("NULL ip address");
            continue;
        }
    }
    return 1;
}

void vtss_upnp_get_udnstr(char *buffer)
{
    static char udnstr[UPNP_MGMT_UDNSTR_SIZE];
    static int  init_flag = 0;
    uchar       mac[6];
    int         rc;

    if (!init_flag) {
        rc = conf_mgmt_mac_addr_get(mac, 0);
        if (rc  !=  0) {
            T_D("vtss_upnp_get_udnstr: conf_mgmt_mac_addr_get fails");
        }
        memset(udnstr, 0, UPNP_MGMT_UDNSTR_SIZE);

        (void)sprintf(udnstr,
                      "uuid:%8.8x-%4.4x-%4.4x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
                      (unsigned int)mac[5],
                      mac[3],
                      mac[1],
                      mac[1],
                      mac[0],
                      mac[0],
                      mac[1],
                      mac[2],
                      mac[3],
                      mac[4],
                      mac[5]);
    }

    (void)strncpy(buffer, udnstr, UPNP_MGMT_UDNSTR_SIZE);
    return;
}

/****************************************************************************
 * Module thread
 ****************************************************************************/
/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

/* Read/create UPNP stack configuration */
static vtss_rc upnp_conf_read_stack(BOOL create)
{
    int             changed;
    BOOL            do_create;
    ulong           size;
    upnp_conf_t      *old_upnp_conf_p, new_upnp_conf;
    upnp_conf_blk_t  *conf_blk_p;
    conf_blk_id_t   blk_id;
    ulong           blk_version;
    vtss_rc         rc;

    T_D("enter, create: %d", create);

    blk_id = CONF_BLK_UPNP_CONF;
    blk_version = UPNP_CONF_BLK_VERSION;

    if (misc_conf_read_use()) {
        /* Read/create UPNP configuration */
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
        do_create  = 1;
    }

    changed = 0;
#if 0
    mode_changed = 0;
#endif
    UPNP_CRIT_ENTER();
    if (do_create) {
        /* Use default values */
        upnp_default_get(&new_upnp_conf);
        if (conf_blk_p != NULL) {
            conf_blk_p->upnp_conf = new_upnp_conf;
        }
    } else {
        /* Use new configuration */
        if (conf_blk_p != NULL) {  // Quiet lint
            new_upnp_conf = conf_blk_p->upnp_conf;
//        } else {
//            upnp_default_get(&new_upnp_conf);  // Also quiet lint
        }
    }
    old_upnp_conf_p = &upnp_global.upnp_conf;
    if (upnp_conf_changed(old_upnp_conf_p, &new_upnp_conf)) {
        changed = 1;
#if 0
        if (old_upnp_conf_p->mode != new_upnp_conf.mode) {
            mode_changed = 1;
        }
#endif
    }
    upnp_global.upnp_conf = new_upnp_conf;
    UPNP_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    if (conf_blk_p == NULL) {
        T_W("failed to open UPNP table");
    } else {
        conf_blk_p->version = blk_version;
        conf_sec_close(CONF_SEC_GLOBAL, blk_id);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */

    if (changed && create) {
#if 0
        if (mode_changed != 0) {
            if (new_upnp_conf.mode == UPNP_MGMT_ENABLED) {
                upnp_create_pthread();
            }  else {
                upnp_device_stop();
            }
        }
#endif
        rc = upnp_stack_upnp_conf_set(VTSS_ISID_GLOBAL);
        if (rc != VTSS_OK) {
            T_W("upnp_conf_read_stack: upnp_stack_upnp_conf_set fails");
        }
    }

    T_D("exit");

    return VTSS_OK;
}


/* Module start */
static void upnp_start(BOOL init)
{
    upnp_conf_t *conf_p;
    vtss_rc     rc;

    T_D("enter, init: %d", init);

    if (init) {
        /* Initialize UPNP configuration */
        conf_p = &upnp_global.upnp_conf;
        upnp_default_get(conf_p);

        /* Initialize message buffers */
        VTSS_OS_SEM_CREATE(&upnp_global.request.sem, 1);

        /* Create semaphore for critical regions */
        critd_init(&upnp_global.crit, "upnp_global.crit", VTSS_MODULE_ID_UPNP, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        UPNP_CRIT_EXIT();

        /* create upnp pthread */
        upnp_create_pthread();

    } else {
        /* Register for stack messages */
        rc = upnp_stack_register();
        if (rc != VTSS_OK) {
            T_W("upnp_start: upnp_stack_register fails");
        }
    }
    T_D("exit");
}


/* Initialize module */
vtss_rc upnp_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
    vtss_rc     rc;

    if (data->cmd == INIT_CMD_INIT) {
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x", data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_D("INIT");
#ifdef VTSS_SW_OPTION_VCLI
        upnp_cli_init();
#endif
#ifdef VTSS_SW_OPTION_ICFG
        if ((rc = upnp_icfg_init()) != VTSS_OK) {
            T_D("Calling upnp_icfg_init() failed rc = %s", error_txt(rc));
        }
#endif
        upnp_start(1);
        break;
    case INIT_CMD_START:
        T_D("START");
        upnp_start(0);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("CONF_DEF, isid: %d", isid);
        if (isid == VTSS_ISID_LOCAL) {
            /* Reset local configuration */
        } else if (isid == VTSS_ISID_GLOBAL) {
            /* Reset stack configuration */
            rc = upnp_conf_read_stack(1);
            if (rc != VTSS_OK) {
                T_W("upnp_init: upnp_conf_read_stack(1) fails");
            }
        } else if (VTSS_ISID_LEGAL(isid)) {
            /* Reset switch configuration */
        }
        break;
    case INIT_CMD_MASTER_UP: {
        T_D("MASTER_UP");

        /* Read stack and switch configuration */
        rc = upnp_conf_read_stack(0);
        if (rc != VTSS_OK) {
            T_W("upnp_init: upnp_conf_read_stack(0) fails");
        }
#if 0
        /* Starting UPNP thread (became master) */
        if (upnp_global.upnp_conf.mode == UPNP_MGMT_ENABLED) {
            upnp_create_pthread();
        }
#endif
        break;
    }
    case INIT_CMD_MASTER_DOWN:
        T_D("MASTER_DOWN");
#if 0
        if (upnp_global.upnp_conf.mode == UPNP_MGMT_ENABLED) {
            upnp_device_stop();
        }
#endif
        break;
    case INIT_CMD_SWITCH_ADD:
        T_D("SWITCH_ADD, isid: %d", isid);
        if (msg_switch_is_master()) {
            if (msg_switch_is_local(isid)) {
                master_isid = isid;
            }
        }

        /* Apply all configuration to switch */
        rc = upnp_stack_upnp_conf_set(isid);
        if (rc != VTSS_OK) {
            T_W("upnp_init: upnp_stack_upnp_conf_set fails");
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
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
