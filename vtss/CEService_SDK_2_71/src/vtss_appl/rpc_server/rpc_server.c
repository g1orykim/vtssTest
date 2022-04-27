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
#include "port_api.h"
#include "rpc_api.h"
#include "rpc_udp.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <cyg/io/file.h>        /* iovec */
#include <cyg/fileio/sockio.h>  /* sendmsg */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_RPC

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_RPC
#define TRACE_GRP_CNT        1

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "rpc",
    .descr     = "RPC Server"
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = { 
        .name      = "default",
        .descr     = "Default (RPC server)",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

/* Static data */

static cyg_handle_t rpcserver_thread_handle;
static cyg_thread   rpcserver_thread_block;
static char         rpcserver_thread_stack[THREAD_DEFAULT_STACK_SIZE];

static int          fd;         /* Main UDP socket */

static rpc_msg_t * udp_receive_message(u8 *data, size_t max, size_t len)
{
    static rpc_msg_t rpcbuf;
    rpc_udp_header_t *hdr;
    rpc_msg_init(&rpcbuf, data, max);
    rpc_msg_put(&rpcbuf, len);
    if((hdr = (rpc_udp_header_t*) rpc_msg_pull(&rpcbuf, sizeof(rpc_udp_header_t)))) {
        rpcbuf.rc     = RPC_NTOHL(hdr->rc);
        rpcbuf.type   = RPC_NTOHL(hdr->type);
        rpcbuf.seq    = RPC_NTOHL(hdr->seq);
        rpcbuf.id     = RPC_NTOHL(hdr->id);
    }
    return &rpcbuf;
}

static void  udp_send_message(int sock_fd, rpc_msg_t *msg, struct sockaddr_in *addr)
{
    static u32 seq;
    rpc_udp_header_t rhdr;
    struct iovec frags[2];
    struct msghdr mh;

    if(msg->type != MSG_TYPE_RSP) {
        cyg_scheduler_lock();
        msg->seq = seq++;       /* New sequence number unless response message */
        cyg_scheduler_unlock();
    }

    /* UDP encapsulation header */
    rhdr.rc     = RPC_HTONL(msg->rc);
    rhdr.type   = RPC_HTONL(msg->type);
    rhdr.seq    = RPC_HTONL(msg->seq);
    rhdr.id     = RPC_HTONL(msg->id);

    /* Send 2 fragments: UDP encapsulation + data */
    frags[0].iov_base = &rhdr;
    frags[0].iov_len = sizeof(rhdr);
    frags[1].iov_base = msg->head;
    frags[1].iov_len = msg->length;
    
    /* sendmsg arguments setup */
    mh.msg_name = addr;
    mh.msg_namelen = sizeof(*addr);
    mh.msg_iov = frags;
    mh.msg_iovlen = 2;
    mh.msg_control = NULL;
    mh.msg_controllen = 0;
    mh.msg_flags = 0;

    if(sendmsg(sock_fd, &mh, 0) < 0) {
        T_E("sendmsg: %s", strerror(errno));
    }
}

/* This dispatch RPC events */
vtss_rc rpc_message_send(rpc_msg_t *msg)
{
    struct ifreq ifreq;
    struct sockaddr_in tx_si;

    /* NB - we broadcast on VID 1 ethernet interface */
    memset(&ifreq, 0, sizeof(ifreq));
    (void) snprintf(ifreq.ifr_name, 16, "eth%u", 1);

    if (ioctl(fd, SIOCGIFBRDADDR, (char *)&ifreq) >= 0) {
        /* Event address */
        memset(&tx_si, 0, sizeof(tx_si));
        tx_si.sin_family = AF_INET;
        tx_si.sin_port = htons(RPC_EVENT_PORT);
        tx_si.sin_addr = ((struct sockaddr_in *)&ifreq.ifr_broadaddr)->sin_addr;

        udp_send_message(fd, msg, &tx_si);
        return VTSS_RC_OK;
    }
    return VTSS_RC_ERROR;
}

rpc_msg_t *rpc_message_alloc(void)
{
    rpc_msg_t *msg;
    if((msg = VTSS_MALLOC(sizeof(*msg) + RPC_BUFSIZE))) {
        memset(msg, 0, sizeof(*msg));
        rpc_msg_init(msg, (void*) &msg[1], RPC_BUFSIZE);
    }
    return msg;
}

void rpc_message_dispose(rpc_msg_t *msg)
{
    VTSS_FREE(msg);
}

static void 
port_state_change_callback(vtss_isid_t isid, 
                           vtss_port_no_t port_no, 
                           port_info_t *info)
{
    (void) evt_port_state(port_no, info);
}

static void rpc_server_thread(cyg_addrword_t data)
{
    socklen_t slen;
    ssize_t nbytes;
    struct sockaddr_in rx_cl;
    struct sockaddr_in rx_si;
    unsigned char membuf[RPC_BUFSIZE];
    int enable = 1;

    if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        exit(1);
    }

    memset(&rx_si, 0, sizeof(rx_si));
    memset(&rx_cl, 0, sizeof(rx_cl));

    rx_si.sin_family = AF_INET;
    rx_si.sin_port = htons(RPC_PORT);
    rx_si.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(fd, (struct sockaddr *)&rx_si, sizeof(rx_si)) == -1) {
        perror("bind");
    }

    /* Enable broadcasting */
    (void) setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));

    /* Port change callback */
    (void) port_global_change_register(VTSS_MODULE_ID_RPC, port_state_change_callback);

    slen = sizeof(rx_cl);
    while ((nbytes = recvfrom(fd, membuf, sizeof(membuf), 0, (struct sockaddr *)&rx_cl, &slen)) > 0) {
        rpc_msg_t *msg = udp_receive_message(membuf, sizeof(membuf), nbytes);
        T_D("Received packet %zu bytes from %s:%d", (size_t) nbytes, inet_ntoa(rx_cl.sin_addr), ntohs(rx_cl.sin_port));
        if (msg->type == MSG_TYPE_REQ) {
            rpc_message_dispatch(msg);
            if(msg->type == MSG_TYPE_RSP) {
                udp_send_message(fd, msg, &rx_cl);
            }
        } else {
            T_W("Dropping type %d from %s:%d", msg->type, inet_ntoa(rx_cl.sin_addr), ntohs(rx_cl.sin_port));
        }
    }

    close(fd);
}

/****************************************************************************/
/*  Initialization functions                                                */
/****************************************************************************/

vtss_rc
rpcserver_init(vtss_init_data_t *data)
{
    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
        cyg_thread_create(THREAD_DEFAULT_PRIO,
                          rpc_server_thread,
                          0,
                          "rpc.server",
                          rpcserver_thread_stack,
                          sizeof(rpcserver_thread_stack),
                          &rpcserver_thread_handle,
                          &rpcserver_thread_block);
        break;
    case INIT_CMD_START:
        cyg_thread_resume(rpcserver_thread_handle);
        break;
    case INIT_CMD_CONF_DEF:
        break;
    case INIT_CMD_MASTER_UP:
        break;
    case INIT_CMD_SWITCH_ADD:
        break;
    case INIT_CMD_SWITCH_DEL:
        break;
    case INIT_CMD_MASTER_DOWN:
        break;
    default:
        break;
    }

    return VTSS_RC_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
