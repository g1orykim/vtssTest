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


#ifndef _DHCP_HELPER_H_
#define _DHCP_HELPER_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "dhcp_helper_api.h"
#include "vtss_module_id.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_DHCP_HELPER

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>

/* DHCP helper statistics timer */
#define DHCP_HELPER_STATS_TIMER     1000

/* DHCP helper ACE IDs */
#define DHCP_HELPER_BOOTPS_ACE_ID   1
#define DHCP_HELPER_BOOTPC_ACE_ID   2

/* ================================================================= *
 *  DHCP helper stack messages
 * ================================================================= */

/* DHCP helper request message timeout */
#define DHCP_HELPER_REQ_TIMEOUT     5

/* DHCP helper counters timer */
#define DHCP_HELPER_COUNTERS_TIMER  1000

/* DHCP helper messages IDs */
typedef enum {
    DHCP_HELPER_MSG_ID_FRAME_RX_IND,        /* Frame receive indication */
    DHCP_HELPER_MSG_ID_FRAME_TX_REQ,        /* Frame transmit request */
    DHCP_HELPER_MSG_ID_LOCAL_ACE_SET,       /* Set local ACE for trap DHCP packet to CPU */
    DHCP_HELPER_MSG_ID_COUNTERS_GET_REQ,    /* Counters get request */
    DHCP_HELPER_MSG_ID_COUNTERS_GET_REP,    /* Counters get reply */
    DHCP_HELPER_MSG_ID_COUNTERS_CLR         /* Counter clear */
} dhcp_helper_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    dhcp_helper_msg_id_t msg_id;

    /* Request data, depending on message ID */
    union {
        /* DHCP_HELPER_MSG_ID_FRAME_RX_IND */
        struct {
            size_t          len;
            vtss_vid_t      vid;
            vtss_port_no_t  port_no;
            vtss_glag_no_t  glag_no;
        } rx_ind;

        /* DHCP_HELPER_MSG_ID_FRAME_TX_REQ */
        struct {
            dhcp_helper_user_t  user;
            size_t              len;
            vtss_vid_t          vid;
            vtss_isid_t         isid;
            u64                 dst_port_mask;
            vtss_port_no_t      src_port_no;
            vtss_glag_no_t      src_glag_no;
            u8                  dhcp_message;
        } tx_req;

        /* DHCP_HELPER_MSG_ID_LOCAL_ACE_SET */
        struct {
            BOOL add;
        } local_ace_set;

        /* DHCP_HELPER_MSG_ID_COUNTERS_GET_REQ,
           DHCP_HELPER_MSG_ID_COUNTERS_GET_REP,
           DHCP_HELPER_MSG_ID_COUNTERS_CLR */
        struct {
            dhcp_helper_user_t      user;
            vtss_port_no_t          port_no;
            dhcp_helper_stats_tx_t  tx_stats; // The RX statistic is saving on master, since only master can get the user information.
        } tx_counters_get, counters_clear;
    } data;
} dhcp_helper_msg_t;

/* DHCP helper message buffer */
typedef struct {
    vtss_os_sem_t   *sem; /* Semaphore */
    void            *msg; /* Message */
} dhcp_helper_msg_buf_t;

/* ================================================================= *
 *  DHCP helper global structure
 * ================================================================= */

/* DHCP helper global structure */
typedef struct {
    critd_t                 crit;
    critd_t                 bip_crit;
    dhcp_helper_port_conf_t port_conf[VTSS_ISID_END];
    dhcp_helper_stats_t     stats[DHCP_HELPER_USER_CNT][VTSS_ISID_END][VTSS_PORTS];
    vtss_mtimer_t           stats_timer[VTSS_ISID_END];                             /* Counters timer */
    cyg_flag_t              stats_flags;                                            /* Counters flags */
    u32                     frame_info_cnt;                                         /* Frame information counter */

    /* Request buffer and semaphore */
    struct {
        vtss_os_sem_t       sem;
        dhcp_helper_msg_t   msg;
    } request;

    struct {
        vtss_os_sem_t       sem;
        dhcp_helper_msg_t   msg;
    } reply;
} dhcp_helper_global_t;

#endif /* _DHCP_HELPER_H_ */


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
