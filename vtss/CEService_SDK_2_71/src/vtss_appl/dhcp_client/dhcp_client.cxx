/*

 Vitesse Switch Software.

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
#include "types.hxx"
#include "string.hxx"
#include "dhcp_pool.hxx"
#include "dhcp_client.hxx"
#include "dhcp_frame.hxx"
#include "eCos_frame_service.hxx"

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

extern "C" {
#include "main.h"
#include "critd_api.h"
#include "packet_api.h"
#include "sysutil_api.h"
#include "dhcp_client_api.h"
#include "ip2_utils.h"
#if defined(VTSS_SW_OPTION_DHCP_HELPER)
#include "dhcp_helper_api.h"
#endif
}

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_DHCP_CLIENT

#if (VTSS_TRACE_ENABLED)
#define VTSS_TRACE_DHCP_CLIENT_GRP_DEFAULT 0
#define VTSS_TRACE_DHCP_CLIENT_GRP_CRIT    1
#define VTSS_TRACE_DHCP_CLIENT_GRP_CNT     2

static vtss_trace_reg_t trace_reg;
static vtss_trace_grp_t trace_grps[VTSS_TRACE_DHCP_CLIENT_GRP_CNT];

static void DHCP_CLIENT_init_trace_data() {
    trace_reg.module_id = VTSS_MODULE_ID_DHCP_CLIENT;
    strcpy(trace_reg.name, "dhcpc");
    strcpy(trace_reg.descr, "dhcp client");
    strcpy(trace_grps[VTSS_TRACE_DHCP_CLIENT_GRP_DEFAULT].name, "default");
    strcpy(trace_grps[VTSS_TRACE_DHCP_CLIENT_GRP_DEFAULT].descr, "Default");
    trace_grps[VTSS_TRACE_DHCP_CLIENT_GRP_DEFAULT].lvl = VTSS_TRACE_LVL_WARNING;
    trace_grps[VTSS_TRACE_DHCP_CLIENT_GRP_DEFAULT].timestamp = 1;
    strcpy(trace_grps[VTSS_TRACE_DHCP_CLIENT_GRP_CRIT].name, "crit");
    strcpy(trace_grps[VTSS_TRACE_DHCP_CLIENT_GRP_CRIT].descr, "Critical regions");
    trace_grps[VTSS_TRACE_DHCP_CLIENT_GRP_CRIT].lvl = VTSS_TRACE_LVL_ERROR;
    trace_grps[VTSS_TRACE_DHCP_CLIENT_GRP_CRIT].timestamp = 1;
};

static critd_t DHCP_CLIENT_crit;

#  define DHCP_CLIENT_CRIT_ENTER()                 \
      critd_enter(&DHCP_CLIENT_crit,               \
                  VTSS_TRACE_DHCP_CLIENT_GRP_CRIT, \
                  VTSS_TRACE_LVL_NOISE,            \
                  __FILE__, __LINE__)

#  define DHCP_CLIENT_CRIT_EXIT()                  \
      critd_exit(&DHCP_CLIENT_crit,                \
                 VTSS_TRACE_DHCP_CLIENT_GRP_CRIT,  \
                 VTSS_TRACE_LVL_NOISE,             \
                 __FILE__, __LINE__)

#  define DHCP_CLIENT_CRIT_ASSERT_LOCKED()         \
    critd_assert_locked(&DHCP_CLIENT_crit,         \
                        TRACE_GRP_CRIT,            \
                        __FILE__, __LINE__)
#else
#  define DHCP_CLIENT_CRIT_ENTER() critd_enter(&DHCP_CLIENT_crit)
#  define DHCP_CLIENT_CRIT_EXIT()  critd_exit( &DHCP_CLIENT_crit)
#  define DHCP_CLIENT_CRIT_ASSERT_LOCKED() critd_assert_locked(&DHCP_CLIENT_crit)
#endif /* VTSS_TRACE_ENABLED */

#define DHCP_CLIENT_CRIT_RETURN(T, X) \
do {                          \
    T __val = (X);            \
    DHCP_CLIENT_CRIT_EXIT();          \
    return __val;             \
} while(0)

#define DHCP_CLIENT_CRIT_RETURN_RC(X)   \
    DHCP_CLIENT_CRIT_RETURN(vtss_rc, X)

static struct LockRef {
    void lock() { DHCP_CLIENT_CRIT_ENTER(); }
    void unlock() { DHCP_CLIENT_CRIT_EXIT(); }
} lock;

static VTSS::eCosRawFrameService raw_frame_service(VTSS_MODULE_ID_DHCP_CLIENT);
static VTSS::SubjectThread thread("DhcpClient");

typedef VTSS::Dhcp::DhcpPool<
    VTSS::eCosRawFrameService,
    VTSS::SubjectThread,
    LockRef,
    VTSS_VIDS
> DhcpPool_t;
DhcpPool_t pool(raw_frame_service, thread, lock);

#if !defined(VTSS_SW_OPTION_DHCP_HELPER)
static void *packet_filter_id = NULL;
static packet_rx_filter_t packet_filter;

static void DHCP_CLIENT_rx_filter_unreg()
{
    if (!packet_filter_id) {
        return;
    }

    vtss_rc rc = packet_rx_filter_unregister(packet_filter_id);

    if (rc == VTSS_RC_OK) {
        packet_filter_id = 0;
    } else {
        T_W("packet_rx_filter_unregister() failed");
    }
}

static void DHCP_CLIENT_rx_filter_reg()
{
    using namespace VTSS;
    memset(&packet_filter, 0, sizeof(packet_filter));
    packet_filter.modid = VTSS_MODULE_ID_DHCP_CLIENT;
    packet_filter.match = PACKET_RX_FILTER_MATCH_UDP_DST_PORT;
    packet_filter.prio = PACKET_RX_FILTER_PRIO_BELOW_NORMAL;
    packet_filter.cb = vtss_dhcp_client_packet_handler;
    packet_filter.udp_dst_port_min = 68;
    packet_filter.udp_dst_port_max = 68;

    if (packet_filter_id) {
        DHCP_CLIENT_rx_filter_unreg();
    }

    vtss_rc rc = packet_rx_filter_register(&packet_filter, &packet_filter_id);
    if (rc != VTSS_RC_OK) {
        T_W("packet_rx_filter_register() failed");
    }
}
#endif /* !VTSS_SW_OPTION_DHCP_HELPER */

const char *  vtss_dhcp4c_state_to_txt(vtss_dhcp4c_state_t s) {
    switch(s) {
    case DHCP4C_STATE_STOPPED:    return "STOPPED";
    case DHCP4C_STATE_INIT:       return "INIT";
    case DHCP4C_STATE_SELECTING:  return "SELECTING";
    case DHCP4C_STATE_REQUESTING: return "REQUESTING";
    case DHCP4C_STATE_REBINDING:  return "REBINDING";
    case DHCP4C_STATE_BOUND:      return "BOUND";
    case DHCP4C_STATE_RENEWING:   return "RENEWING";
    case DHCP4C_STATE_FALLBACK:   return "FALLBACK";
    default:                      return "UNKNOWN";
    }
}

int vtss_dhcp4c_status_to_txt(char                            *buf,
                              int                              size,
                              const vtss_dhcp_client_status_t *const st)
{
    int s = 0;
    unsigned i;

    PRINTF("State: %s", vtss_dhcp4c_state_to_txt(st->state));

    switch (st->state) {
    case DHCP4C_STATE_SELECTING:
        PRINTF(" offers: [");

        if (st->offers.valid_offers == 0) {
            PRINTF("none");
        }

        for (i = 0; i < st->offers.valid_offers; ++i) {
            if (i != 0) {
                PRINTF(", ");
            }
            PRINTF(VTSS_IPV4N_FORMAT " from " VTSS_IPV4_FORMAT,
                   VTSS_IPV4N_ARG(st->offers.list[i].ip),
                   VTSS_IPV4_ARGS(st->offers.list[i].server_ip));
        }
        PRINTF("]");
        break;

    case DHCP4C_STATE_REQUESTING:
    case DHCP4C_STATE_REBINDING:
    case DHCP4C_STATE_BOUND:
    case DHCP4C_STATE_RENEWING:
        PRINTF(" server: "VTSS_IPV4_FORMAT, VTSS_IPV4_ARGS(st->server_ip));
        break;

    default:
        ;
    }

    return s;
}

vtss_rc vtss_dhcp_client_start(vtss_vid_t vlan)
{
    T_I("Start dhcp client on vlan %u", vlan);
#if defined(VTSS_SW_OPTION_DHCP_HELPER)
        //Receive DHCP packet from DHCP helper
        dhcp_helper_user_receive_register(DHCP_HELPER_USER_CLIENT, vtss_dhcp_client_packet_handler);
#endif /* VTSS_SW_OPTION_DHCP_HELPER */
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.start(vlan));
}

vtss_rc vtss_dhcp_client_stop(vtss_vid_t vlan)
{
    T_I("Stopping dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.stop(vlan));
}

vtss_rc vtss_dhcp_client_fallback(vtss_vid_t vlan)
{
    T_I("Fallback dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.fallback(vlan));
}

vtss_rc vtss_dhcp_client_kill(vtss_vid_t vlan)
{
    T_I("Kill dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.kill(vlan));
}

vtss_rc vtss_dhcp_client_if_down(vtss_vid_t vlan)
{
    T_I("id_down dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.if_down(vlan));
}

vtss_rc vtss_dhcp_client_if_up(vtss_vid_t vlan)
{
    T_I("id_up dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.if_up(vlan));
}

vtss_rc vtss_dhcp_client_release(vtss_vid_t vlan)
{
    T_I("release dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.release(vlan));
}

vtss_rc vtss_dhcp_client_decline(vtss_vid_t vlan)
{
    T_I("decline dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.decline(vlan));
}

BOOL vtss_dhcp_client_bound_get(vtss_vid_t vlan)
{
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN(BOOL, pool.bound_get(vlan));
}

vtss_rc vtss_dhcp_client_offers_get(vtss_vid_t vlan,
                                    vtss_dhcp_client_offer_list_t *list)
{
    T_I("offers_get dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.offers_get(vlan, list));
}

vtss_rc vtss_dhcp_client_offer_accept(vtss_vid_t                  vlan,
                                      unsigned idx)
{
    T_I("accept dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.accept(vlan, idx));
}

vtss_rc vtss_dhcp_client_status(      vtss_vid_t                  vlan,
                                      vtss_dhcp_client_status_t  *status)
{
    T_N("status dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.status(vlan, status));
}

vtss_rc vtss_dhcp_client_callback_add(vtss_vid_t                  vlan,
                                      vtss_dhcp_client_callback_t v1)
{
    T_I("cb_add dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.callback_add(vlan, v1));
}

vtss_rc vtss_dhcp_client_callback_del(vtss_vid_t                  vlan,
                                      vtss_dhcp_client_callback_t v1)
{
    T_I("cb_del dhcp client on vlan %u", vlan);
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.callback_del(vlan, v1));
}

vtss_rc vtss_dhcp_client_fields_get(vtss_vid_t          vlan,
                                    vtss_dhcp_fields_t *v1)
{
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.fields_get(vlan, v1));
}

vtss_rc vtss_dhcp_client_dns_option_ip_any_get(vtss_ipv4_t  prefered,
                                               vtss_ipv4_t *ip)
{
    DHCP_CLIENT_CRIT_ENTER();
    DHCP_CLIENT_CRIT_RETURN_RC(pool.dns_option_any_get(prefered, ip));
}

size_t vtss_dhcp_client_hostname_get(char * buf, int max)
{
    system_conf_t conf;
    int           c, i, len;
    uchar         mac[6];

    if (!max) {
        return 0;
    }

    if (system_get_config(&conf) == VTSS_OK) {
        if ((len = strlen(conf.sys_name)) == 0) {
            /* Create unique name */
            (void) conf_mgmt_mac_addr_get(mac, 0);
            sprintf(conf.sys_name, "estax-%02x-%02x-%02x",
                    mac[3], mac[4], mac[5]);
        } else {
            /* Convert system name to lower case and replace spaces by dashes */
            for (i = 0; i < len; i++) {
                c = tolower(conf.sys_name[i]);
                if (c == ' ') {
                    c = '-';
                }
                conf.sys_name[i] = c;
            }
        }
    }

    strncpy(buf, conf.sys_name, max);
    for (i = 0; i < max;) { // strnlen - including terminating zero
        if (buf[i++] == 0) {
            break;
        }
    }

    T_D("Host name is: %s %s %d", buf, conf.sys_name, i);
    return i;
}

#if defined(VTSS_SW_OPTION_DHCP_HELPER)
BOOL vtss_dhcp_client_packet_handler(const u8 *const frm,
                                     size_t length,
                                     vtss_vid_t vid,
                                     vtss_isid_t src_isid,
                                     vtss_port_no_t src_port_no,
                                     vtss_glag_no_t src_glag_no)
{
    DHCP_CLIENT_CRIT_ENTER();
    DhcpPool_t::DhcpClient_t *client = pool.get(vid);

    if (client == 0) {
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    typedef const VTSS::FrameRef L0;
    L0 f(frm, length);

    T_N_HEX(frm, length);

    typedef const VTSS::EthernetFrame<L0> L1;
    L1 e(f);

    if (e.etype() != 0x0800) {
        T_D("%u Ethernet: Not IP", vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    typedef const VTSS::IpFrame<L1> L2;
    L2 i(e);

    if (!i.check()) {
        T_D("%u IP: Did not parse checks", vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    if (!i.is_simple()) {
        T_D("%u IP: Non simple IP", vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    if (i.protocol() != 0x11) {
        T_D("%u IP: Not UDP", vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    typedef const VTSS::UdpFrame<L2> L3;
    L3 u(i);

    if (!u.check()) {
        T_D("%u UDP: Did not parse checks", vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    if (u.dst() != 68) {
        T_D("%u UDP: wrong port", vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    typedef const VTSS::Dhcp::DhcpFrame<L3> L4;
    L4 d(u);

    if (!d.check()) {
        T_D("%u DHCP-frame: Did not parse checks", vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }
    client->frame_event(d, e.src());
    T_N("%u Packet has been delivered to dhcp client", vid);

    DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
}

#else

BOOL vtss_dhcp_client_packet_handler(void *contxt, const u8 *const frm,
                                     const vtss_packet_rx_info_t *const rx_info)
{
    DHCP_CLIENT_CRIT_ENTER();
    DhcpPool_t::DhcpClient_t *client = pool.get(rx_info->tag.vid);

    if (client == 0) {
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    typedef const VTSS::FrameRef L0;
    L0 f(frm, rx_info->length);

    T_N_HEX(frm, rx_info->length);

    typedef const VTSS::EthernetFrame<L0> L1;
    L1 e(f);

    if (e.etype() != 0x0800) {
        T_D("%u Ethernet: Not IP", rx_info->tag.vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    typedef const VTSS::IpFrame<L1> L2;
    L2 i(e);

    if (!i.check()) {
        T_D("%u IP: Did not parse checks", rx_info->tag.vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    if (!i.is_simple()) {
        T_D("%u IP: Non simple IP", rx_info->tag.vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    if (i.protocol() != 0x11) {
        T_D("%u IP: Not UDP", rx_info->tag.vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    typedef const VTSS::UdpFrame<L2> L3;
    L3 u(i);

    if (!u.check()) {
        T_D("%u UDP: Did not parse checks", rx_info->tag.vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    if (u.dst() != 68) {
        T_D("%u UDP: wrong port", rx_info->tag.vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }

    typedef const VTSS::Dhcp::DhcpFrame<L3> L4;
    L4 d(u);

    if (!d.check()) {
        T_D("%u DHCP-frame: Did not parse checks", rx_info->tag.vid);
        DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
    }
    client->frame_event(d, e.src());
    T_N("%u Packet has been delivered to dhcp client", rx_info->tag.vid);

    DHCP_CLIENT_CRIT_RETURN(BOOL, FALSE);
}
#endif /* VTSS_SW_OPTION_DHCP_HELPER */

vtss_rc vtss_dhcp_client_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;

    if (data->cmd == INIT_CMD_INIT) {
        DHCP_CLIENT_init_trace_data();
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps,
                            VTSS_TRACE_DHCP_CLIENT_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    T_D("enter, cmd: %d, isid: %u, flags: 0x%x",
        data->cmd, data->isid, data->flags);

    switch (data->cmd) {
    case INIT_CMD_INIT:
        T_I("INIT");
        critd_init(&DHCP_CLIENT_crit, "dhcp_client.crit",
                   VTSS_MODULE_ID_DHCP_CLIENT,
                   VTSS_TRACE_MODULE_ID,
                   CRITD_TYPE_MUTEX);
        DHCP_CLIENT_CRIT_EXIT();

        T_I("Starting thread");
        subject_thread_start(&thread);
        break;

    case INIT_CMD_START:
        T_I("START");
        break;

    case INIT_CMD_CONF_DEF:
        T_I("CONF_DEF, isid: %u", isid);
        break;

    case INIT_CMD_MASTER_UP:
        T_I("MASTER_UP");
#if !defined(VTSS_SW_OPTION_DHCP_HELPER)
        DHCP_CLIENT_rx_filter_reg();
#endif /* !VTSS_SW_OPTION_DHCP_HELPER */
        break;

    case INIT_CMD_MASTER_DOWN:
#if !defined(VTSS_SW_OPTION_DHCP_HELPER)
        DHCP_CLIENT_rx_filter_unreg();
#endif /* !VTSS_SW_OPTION_DHCP_HELPER */
        T_I("MASTER_DOWN");
        break;

    case INIT_CMD_SWITCH_ADD:
        T_I("SWITCH_ADD, isid: %u", isid);
        break;

    case INIT_CMD_SWITCH_DEL:
        T_I("SWITCH_DEL, isid: %u", isid);
        break;

    default:
        T_I("UNKNOWN CMD: %d, isid: %u", data->cmd, isid);
        break;
    }

    T_D("exit");
    return VTSS_OK;
}

const char * dhcp_client_error_txt(vtss_rc rc){
    return 0;
}
