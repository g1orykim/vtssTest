/*

 Vitesse Switch Application software.

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
//----------------------------------------------------------------------------
/**
 *  \file
 *      dhcp_server.c
 *
 *  \brief
 *      DHCP server APIs
 *      1. platform APIs for base part
 *      2. public APIs
 *
 *  \author
 *      CP Wang
 *
 *  \date
 *      05/08/2013 17:51
 */
//----------------------------------------------------------------------------

/*
==============================================================================

    Include File

==============================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "dhcp_server_api.h"
#include "dhcp_server.h"
#include "vtss_dhcp_server.h"
#include "vtss_dhcp_server_message.h"
#include "msg_api.h"
#include "conf_api.h"
#include "dhcp_helper_api.h"
#include "ip2_utils.h"
#include "ip2_api.h"

#ifdef VTSS_SW_OPTION_ICFG
#include "dhcp_server_icfg.h"
#endif

#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"
#endif

#include "critd_api.h"
#include <sys/time.h>

/*
==============================================================================

    Constant

==============================================================================
*/
#define _DHCP_SERVER_PORT       67      /**< UDP port of DHCP server */
#define _DHCP_CLIENT_PORT       68      /**< UDP port of DHCP client */
#define _TIMER_IDLE_TIME        1000    /**< 1 second in milli-second */
#define _THREAD_MAX_CNT         2       /**< max number of threads */
#define _MAX_STR_BUF_SIZE       128     /**< max buffer size for trace and syslog */

/*
==============================================================================

    Macro

==============================================================================
*/
#if VTSS_TRACE_ENABLED

#define __SEMA_TAKE() \
    critd_enter( &g_critd, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__ )

#define __SEMA_GIVE() \
    critd_exit( &g_critd, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__ )

#else // VTSS_TRACE_ENABLED

#define __SEMA_TAKE() \
    critd_enter( &g_critd )

#define __SEMA_GIVE() \
    critd_exit( &g_critd )

#endif // VTSS_TRACE_ENABLED

/*
==============================================================================

    Type Definition

==============================================================================
*/
/**
 *  \brief
 *      entry of the created thread.
 *
 *  \param
 *      thread_data [IN]: initialization data and callbacks.
 *
 *  \return
 *      TRUE  : successful.\n
 *      FALSE : failed.
 */
typedef BOOL dhcp_server_thread_entry_cb_t(
    IN i32      thread_data
);

/*
==============================================================================

    Static Variable

==============================================================================
*/
static cyg_handle_t     g_thread_handle[_THREAD_MAX_CNT];
static cyg_thread       g_thread_block[_THREAD_MAX_CNT];
static char             g_thread_stack[_THREAD_MAX_CNT][THREAD_DEFAULT_STACK_SIZE];
static critd_t          g_critd;

/* Vitesse trace */
#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "dhcp_server",
    .descr     = "DHCP Server"
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
#endif

/*
==============================================================================

    Static Function

==============================================================================
*/
/**
 * \brief
 *      Create thread.
 *
 * \param
 *      session_id [IN]: session ID
 *      name       [IN]: name of thread.
 *      priority   [IN]: thread priority.
 *      entry_cb   [IN]: thread running entry.
 *      entry_data [IN]: input parameter of thread running entry.
 *
 * \return
 *      TRUE : successful.
 *      FALSE: failed.
 */
static BOOL _thread_create(
    IN  i32                             session_id,
    IN  char                            *name,
    IN  u32                             priority,
    IN  dhcp_server_thread_entry_cb_t   *entry_cb,
    IN  i32                             entry_data
)
{
    if ( session_id >= _THREAD_MAX_CNT ) {
        T_E("session_id(%d) is too large\n", session_id);
        return FALSE;
    }

    priority = THREAD_DEFAULT_PRIO;

    cyg_thread_create(  priority,
                        (cyg_thread_entry_t *)entry_cb,
                        (cyg_addrword_t)session_id,
                        name,
                        g_thread_stack[session_id],
                        THREAD_DEFAULT_STACK_SIZE,
                        &(g_thread_handle[session_id]),
                        &(g_thread_block[session_id]) );

    cyg_thread_resume( g_thread_handle[session_id] );
    return TRUE;
}

/**
 * \brief
 *      get the time elapsed from system start in seconds.
 *      process wrap around.
 *
 * \param
 *      n/a.
 *
 * \return
 *      seconds from system start.
 */
static u32 _current_time_get(
    void
)
{
    struct timespec     tp;

    if ( clock_gettime(CLOCK_MONOTONIC, &tp) == -1 ) {
        T_E("failed to get system up time\n");
        return 0;
    }
    return tp.tv_sec;
}

/**
 * \brief
 *      sleep for milli-seconds.
 *
 * \param
 *      t [IN]: milli-seconds for sleep.
 *
 * \return
 *      n/a.
 */
static void _sleep_in_ms(
    IN u32  t
)
{
    VTSS_OS_MSLEEP( t );
}

/*
    becauase the lease time resolution is in minute,
    it does not need so exact so just sleep 1 second
*/
/**
 * \brief
 *      becauase the lease time resolution is in minute,
 *      it does not need so exact so just sleep 1 second.
 *
 * \param
 *      t [IN]: milli-seconds for sleep.
 *
 * \return
 *      n/a.
 */
static BOOL _timer_thread(
    IN i32      thread_data
)
{
    if ( thread_data ) {}

    for (;;) {

        /* sleep 1 second */
        _sleep_in_ms( _TIMER_IDLE_TIME );

        if ( msg_switch_is_master() ) {

            __SEMA_TAKE();

            vtss_dhcp_server_timer_process();

            __SEMA_GIVE();
        }
    }

#ifdef DHCP_SERVER_TARGET
    // Avoid "Unreachable code at token "return".
    // Statement must be here for the sake of GCC 4.7
    /*lint -e(527) */
    return TRUE;
#endif
}

/**
 *  \brief
 *      callback function to receive DHCP packets from client.
 *      because this is callback, the APIs called inside should have semaphore protection.
 *
 *  \param
 *      packet      [IN]: Ethernet packet.
 *      len         [IN]: packet length.
 *      vid         [IN]: VLAN ID.
 *      isid        [IN]: isid
 *      src_port_no [IN]: source port
 *      src_glag_no [IN]: source trunk
 *
 *  \return
 *      TRUE  : DHCP server process this packet.\n
 *      FALSE : the packet is not processed, so pass it to other APP.
 */
static BOOL _packet_rx(
    IN const u8 *const  packet,
    IN size_t           len,
    IN vtss_vid_t       vid,
    IN vtss_isid_t      isid,
    IN vtss_port_no_t   src_port_no,
    IN vtss_glag_no_t   src_glag_no
)
{
    BOOL    b;

    /* if not master then de-register */
    if ( msg_switch_is_master() == FALSE ) {
        dhcp_helper_user_receive_unregister(DHCP_HELPER_USER_SERVER);
        return FALSE;
    }

    /* invalid packet */
    if ( packet == NULL ) {
        return FALSE;
    }

    /* unused parameters */
    if ( len ) {}
    if ( isid ) {}
    if ( src_port_no ) {}
    if ( src_glag_no ) {}

    __SEMA_TAKE();

    b = vtss_dhcp_server_packet_rx( packet, vid );

    __SEMA_GIVE();

    return b;
}

/**
 * \brief
 *      calculate IP checksum.
 */
static u16 _ip_chksum(
    IN  u16     ip_hdr_len,
    IN  u16     *ip_hdr
)
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

/**
 * \brief
 *      calculate UDP checksum.
 */
static u16 _udp_chksum(
    IN u16      udp_len,
    IN u16      *src_addr,
    IN u16      *dest_addr,
    IN u16      *udp_hdr
)
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

/*
==============================================================================

    APIs for base/
        these APIs do not need semaphore protection
        because they are called by base/

==============================================================================
*/
/**
 *  \brief
 *      syslog message.
 *
 *  \param
 *      format [IN] : message format.
 *      ...    [IN] : message parameters
 *
 *  \return
 *      n/a.
 */
void dhcp_server_syslog(
    IN  const char  *format,
    IN  ...
)
{
    char        str_buf[ _MAX_STR_BUF_SIZE + 1 ];
    va_list     arglist = NULL;
    int         r;

    memset(str_buf, 0, sizeof(str_buf));

    va_start( arglist, format );
    r = vsnprintf(str_buf, _MAX_STR_BUF_SIZE, format, arglist);
    va_end( arglist );

    if ( r ) {
#ifdef VTSS_SW_OPTION_SYSLOG
        S_I( str_buf );
#else
        puts( str_buf );
#endif
    }
}

/**
 *  \brief
 *      get IP interface of VLAN
 *
 *  \param
 *      vid     [IN] : VLAN ID
 *      ip      [OUT]: IP address of the VLAN
 *      netmask [OUT]: Netmask of the VLAN
 *
 *  \return
 *      TRUE  : successful
 *      FALSE : failed
 */
BOOL dhcp_server_vid_info_get(
    IN  vtss_vid_t          vid,
    OUT vtss_ipv4_t         *ip,
    OUT vtss_ipv4_t         *netmask
)
{
    vtss_if_status_t    ipv4_status;

    if ( vtss_ip2_if_status_get_first(VTSS_IF_STATUS_TYPE_IPV4, vid, &ipv4_status) != VTSS_RC_OK ) {
        //T_E("Fail to get ip interface status on VLAN %u\n", vid);
        return FALSE;
    }

    if ( ip ) {
        *ip = ipv4_status.u.ipv4.net.address;
    }

    if ( netmask ) {
        (void)vtss_conv_prefix_to_ipv4mask((u32)(ipv4_status.u.ipv4.net.prefix_size), netmask);
    }

    return TRUE;
}

/**
 * \brief
 *      register packet rx callback.
 *
 * \param
 *      n/a.
 *
 * \return
 *      n/a.
 */
void dhcp_server_packet_rx_register(
    void
)
{
    /* register only if master */
    if ( msg_switch_is_master() ) {
        dhcp_helper_user_receive_register(DHCP_HELPER_USER_SERVER, _packet_rx );
    }
}

/**
 * \brief
 *      deregister packet rx callback.
 *
 * \param
 *      n/a.
 *
 * \return
 *      n/a.
 */
void dhcp_server_packet_rx_deregister(
    void
)
{
    dhcp_helper_user_receive_unregister(DHCP_HELPER_USER_SERVER);
}

/**
 * \brief
 *      send DHCP message.
 *
 * \param
 *      dhcp_message [IN]: DHCP message.
 *      option_len   [IN]: option field length.
 *      vid          [IN]: VLAN ID to send.
 *      sip          [IN]: source IP.
 *      dmac         [IN]: destination MAC.
 *      dip          [IN]: destination IP.
 *
 * \return
 *      TRUE  : successfully.
 *      FALSE : fail to send
 */
BOOL dhcp_server_packet_tx(
    IN  dhcp_server_message_t   *dhcp_message,
    IN  u32                     option_len,
    IN  vtss_vid_t              vid,
    IN  vtss_ipv4_t             sip,
    IN  u8                      *dmac,
    IN  vtss_ipv4_t             dip
)
{
    void                        *pbufref;
    u8                          *packet;
    size_t                      packet_size;
    dhcp_server_eth_header_t    *ether;
    dhcp_server_ip_header_t     *ip;
    dhcp_server_udp_header_t    *udp;
    char                        *payload;
    static u16                  ident = 11111;
    u8                          smac[DHCP_SERVER_MAC_LEN];
    u16                         udp_len;

    /* padding DHCP message */
    udp_len = (u16)sizeof(dhcp_server_udp_header_t) + (u16)sizeof(dhcp_server_message_t) + (u16)option_len;
    if ( udp_len % 4 ) {
        udp_len += ( 4 - ( udp_len % 4 ) );
    } else {
        udp_len += 4;
    }

    // get packet buffer
    packet_size = sizeof(dhcp_server_eth_header_t) + sizeof(dhcp_server_ip_header_t) + udp_len;
    packet = (u8 *)dhcp_helper_alloc_xmit(packet_size, VTSS_ISID_GLOBAL, &pbufref);
    if ( packet == NULL ) {
        T_E("memory insufficient\n");
        return FALSE;
    }

    // clear packet buffer
    memset( packet, 0, packet_size );

    // get source MAC
    (void)conf_mgmt_mac_addr_get(smac, 0);

    // Ethernet header
    ether = (dhcp_server_eth_header_t *)packet;

    memcpy(ether->dmac, dmac, DHCP_SERVER_MAC_LEN);
    memcpy(ether->smac, smac, DHCP_SERVER_MAC_LEN);
    ether->etype[0] = 0x08;
    ether->etype[1] = 0x00;

    // IP header
    ip = (dhcp_server_ip_header_t *)( packet + sizeof(dhcp_server_eth_header_t) );

    ip->vhl   = 0x45;
    ip->len   = htons( (u16)(packet_size - sizeof(dhcp_server_eth_header_t)) );
    ip->ident = htons( ident++ );
    ip->ttl   = 64; // hops
    ip->proto = 17; // UDP
    ip->sip   = htonl( sip );
    ip->dip   = htonl( dip );

    // UDP header
    udp = (dhcp_server_udp_header_t *)( packet + sizeof(dhcp_server_eth_header_t) + sizeof(dhcp_server_ip_header_t) );

    udp->sport = htons( _DHCP_SERVER_PORT );
    udp->dport = htons( _DHCP_CLIENT_PORT );
    udp->len   = htons( udp_len );

    // Payload
    payload = (char *)(packet + sizeof(dhcp_server_eth_header_t) + sizeof(dhcp_server_ip_header_t) + sizeof(dhcp_server_udp_header_t));

    memcpy( payload, dhcp_message, sizeof(dhcp_server_message_t) + option_len);

    // IP checksum
    ip->chksum = _ip_chksum(sizeof(dhcp_server_ip_header_t), (u16 *)ip);

    // UDP checksum
    udp->chksum = 0;
    udp->chksum = _udp_chksum(udp_len, (u16 *) & (ip->sip), (u16 *) & (ip->dip), (u16 *)udp);

    if ( dhcp_helper_xmit(DHCP_HELPER_USER_SERVER, packet, packet_size, vid, VTSS_ISID_GLOBAL, 0, VTSS_ISID_END, VTSS_PORT_NO_NONE, VTSS_GLAG_NO_NONE, pbufref) != 0 ) {
        T_E("dhcp_helper_xmit( %d )\n", vid);
        return FALSE;
    }
    return TRUE;
}

/*
==============================================================================

    Public Function
        These APIs may need semaphore protection
        because they are called by other modules.

==============================================================================
*/
/**
 * \brief
 *      Start DHCP server.
 *
 * \param
 *      data [IN]: init data.
 *
 * \return
 *      vtss_rc: VTSS_OK on success.\n
 *               others are failed.
 */
vtss_rc dhcp_server_init(
    IN vtss_init_data_t     *data
)
{
    if ( data == NULL ) {
        T_E("invalid parameter data\n");
        return VTSS_RC_ERROR;
    }

    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

        /* create semaphore */
        critd_init(&g_critd, "ICLI", VTSS_MODULE_ID_ICLI, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        /* if not give, then it will crash. Wierd! */
#if (VTSS_TRACE_ENABLED)
        critd_exit( &g_critd, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__ );
#else
        critd_exit( &g_critd );
#endif

        /* start dhcp server engine */
        if ( vtss_dhcp_server_init() == FALSE ) {
            T_E("fail to initialize DHCP server engine\n");
            return VTSS_RC_ERROR;
        }

        /* create timer thread */
        if ( _thread_create(1, "DHCP_SERVER_TIMER", 0, _timer_thread, 0) == FALSE ) {
            T_E("Fail to create thread for timer\n");
            return FALSE;
        }

#ifdef VTSS_SW_OPTION_ICFG
        if ( dhcp_server_icfg_init() != VTSS_RC_OK ) {
            T_E("ICFG not initialized correctly");
        }
#endif
        break;

    case INIT_CMD_START:
        break;

    case INIT_CMD_MASTER_UP:
        break;

    case INIT_CMD_CONF_DEF:
        __SEMA_TAKE();
        vtss_dhcp_server_reset_to_default();
        __SEMA_GIVE();
        break;

    case INIT_CMD_MASTER_DOWN:
        __SEMA_TAKE();
        vtss_dhcp_server_stat_clear();
        __SEMA_GIVE();
        break;

    case INIT_CMD_SWITCH_ADD:
    case INIT_CMD_SWITCH_DEL:
    default:
        break;
    }

    return VTSS_OK;
}

/**
 * \brief
 *      error string for DHCP server.
 *
 * \param
 *      rc [IN]: error code.
 *
 * \return
 *      char *: error string.
 */
char *dhcp_server_error_txt(
    IN vtss_rc  rc
)
{
    switch (rc) {
    case DHCP_SERVER_RC_ERROR:
        return "General error";

    case DHCP_SERVER_RC_ERR_PARAMETER:
        return "Invalid parameter";

    case DHCP_SERVER_RC_ERR_NOT_EXIST:
        return "Data not exist";

    case DHCP_SERVER_RC_ERR_MEMORY:
        return "Memory insufficient";

    case DHCP_SERVER_RC_ERR_FULL:
        return "Table full";

    case DHCP_SERVER_RC_ERR_DUPLICATE:
        return "Duplicate data";

    case DHCP_SERVER_RC_ERR_IP:
        return "Invalid IP address";

    case DHCP_SERVER_RC_ERR_SUBNET:
        return "Invalid subnet address";

    default:
        return "";
    }
}

/**
 *  \brief
 *      reset DHCP server engine to default configuration.
 *
 *  \param
 *      n/a.
 *
 *  \return
 *      n/a.
 */
void dhcp_server_reset_to_default(
    void
)
{
    __SEMA_TAKE();

    vtss_dhcp_server_reset_to_default();

    __SEMA_GIVE();
}

/**
 * \brief
 *      get the time elapsed from system start in seconds.
 *      process wrap around.
 *
 * \param
 *      n/a.
 *
 * \return
 *      seconds from system start.
 */
u32 dhcp_server_current_time_get(
    void
)
{
    return _current_time_get();
}

/**
 *  \brief
 *      Enable/Disable DHCP server.
 *
 *  \param
 *      b_enable [IN]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_enable_set(
    IN  BOOL    b_enable
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_enable_set( b_enable );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Get if DHCP server is enabled or not.
 *
 *  \param
 *      b_enable [OUT]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_enable_get(
    OUT BOOL    *b_enable
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_enable_get( b_enable );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Add excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      exclued_ip [IN]: IP address range to be excluded.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_excluded_add(
    IN  dhcp_server_excluded_ip_t     *exclued_ip
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_excluded_add( exclued_ip );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Delete excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      exclued_ip [IN]: Excluded IP address range to be deleted.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_excluded_delete(
    IN  dhcp_server_excluded_ip_t     *exclued_ip
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_excluded_delete( exclued_ip );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Get excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      exclued_ip [IN] : index.
 *      exclued_ip [OUT]: Excluded IP address range data.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_excluded_get(
    INOUT  dhcp_server_excluded_ip_t     *exclued_ip
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_excluded_get( exclued_ip );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Get next of current excluded IP address range.
 *      index : low_ip, high_ip
 *
 *  \param
 *      exclued_ip [IN] : Currnet excluded IP address range index.
 *      exclued_ip [OUT]: Next excluded IP address range data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_excluded_get_next(
    INOUT  dhcp_server_excluded_ip_t     *exclued_ip
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_excluded_get_next( exclued_ip );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Set DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN]: new or modified DHCP pool.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_pool_set(
    IN  dhcp_server_pool_t     *pool
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_pool_set( pool );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Delete DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN]: DHCP pool to be deleted.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_pool_delete(
    IN  dhcp_server_pool_t     *pool
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_pool_delete( pool );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Get DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN] : index.
 *      pool [OUT]: DHCP pool data.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_pool_get(
    INOUT  dhcp_server_pool_t     *pool
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_pool_get( pool );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Get next of current DHCP pool.
 *      index: name
 *
 *  \param
 *      pool [IN] : Currnet DHCP pool index.
 *      pool [OUT]: Next DHCP pool data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_pool_get_next(
    INOUT  dhcp_server_pool_t     *pool
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_pool_get_next( pool );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Set DHCP pool to be default value.
 *
 *  \param
 *      pool [OUT]: default DHCP pool.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_pool_default(
    IN  dhcp_server_pool_t     *pool
)
{
    return vtss_dhcp_server_pool_default( pool );
}

/**
 *  \brief
 *      Clear DHCP server statistics.
 *
 *  \param
 *      n/a.
 *
 *  \return
 *      n/a.
 */
void dhcp_server_statistics_clear(
    void
)
{
    __SEMA_TAKE();

    vtss_dhcp_server_statistics_clear();

    __SEMA_GIVE();
}

/**
 *  \brief
 *      Get DHCP server statistics.
 *
 *  \param
 *      statistics [OUT]: DHCP server statistics data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_statistics_get(
    OUT dhcp_server_statistics_t  *statistics
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_statistics_get( statistics );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Delete DHCP binding.
 *      index: ip
 *
 *  \param
 *      binding [IN]: DHCP binding to be deleted.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_binding_delete(
    IN  dhcp_server_binding_t     *binding
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_binding_delete( binding );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Get DHCP binding.
 *      index: ip
 *
 *  \param
 *      binding [IN] : index.
 *      binding [OUT]: DHCP binding data.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_binding_get(
    INOUT  dhcp_server_binding_t     *binding
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_binding_get( binding );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Get next of current DHCP binding.
 *      index: ip
 *
 *  \param
 *      binding [IN] : Currnet DHCP binding index.
 *      binding [OUT]: Next DHCP binding data to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_binding_get_next(
    INOUT  dhcp_server_binding_t     *binding
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_binding_get_next( binding );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Clear DHCP bindings by binding type.
 *
 *  \param
 *      type - binding type
 *
 *  \return
 *      n/a.
 */
void dhcp_server_binding_clear_by_type(
    IN dhcp_server_binding_type_t     type
)
{
    __SEMA_TAKE();

    vtss_dhcp_server_binding_clear_by_type( type );

    __SEMA_GIVE();
}

/**
 *  \brief
 *      Enable/Disable DHCP server per VLAN.
 *
 *  \param
 *      vid      [IN]: VLAN ID
 *      b_enable [IN]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_vlan_enable_set(
    IN  vtss_vid_t      vid,
    IN  BOOL            b_enable
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_vlan_enable_set( vid, b_enable );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Get Enable/Disable DHCP server per VLAN.
 *
 *  \param
 *      vid      [IN] : VLAN ID
 *      b_enable [OUT]: TRUE - enable, FALSE - disable.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_vlan_enable_get(
    IN  vtss_vid_t      vid,
    OUT BOOL            *b_enable
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_vlan_enable_get( vid, b_enable );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      add a declined IP. This API should be used for debug only.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : index.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_declined_add(
    IN  vtss_ipv4_t     declined_ip
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_declined_add( declined_ip );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      delete a declined IP. This API should be used for debug only.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : index.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_declined_delete(
    IN  vtss_ipv4_t     declined_ip
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_declined_delete( declined_ip );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Get declined IP.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : index.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_declined_get(
    IN  vtss_ipv4_t     *declined_ip
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_declined_get( declined_ip );

    __SEMA_GIVE();

    return rc;
}

/**
 *  \brief
 *      Get next declined IP.
 *      index: declined_ip
 *
 *  \param
 *      declined_ip [IN] : Currnet declined IP.
 *      declined_ip [OUT]: Next declined IP to get.
 *
 *  \return
 *      DHCP_SERVER_RC_OK : successful.\n
 *      DHCP_SERVER_RC_XX : failed.
 */
dhcp_server_rc_t dhcp_server_declined_get_next(
    INOUT   vtss_ipv4_t     *declined_ip
)
{
    dhcp_server_rc_t    rc;

    __SEMA_TAKE();

    rc = vtss_dhcp_server_declined_get_next( declined_ip );

    __SEMA_GIVE();

    return rc;
}
