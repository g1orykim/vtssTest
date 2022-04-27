/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _COMMON_OS_H_
#define _COMMON_OS_H_

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_L2PROTO

#define VTSS_TRACE_GRP_DEFAULT 0
#define VTSS_TRACE_GRP_PACKET  1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>

/* L2 messages IDs */
typedef enum {
    L2_MSG_ID_FRAME_RX_IND,     /* Frame receive indication */
    L2_MSG_ID_FRAME_TX_REQ,     /* Frame transmit request */
    L2_MSG_ID_SET_STP_STATE,    /* Set STP state for one port */
    L2_MSG_ID_SYNC_STP_STATE,   /* Set STP state for all ports */
    L2_MSG_ID_FLUSH_FDB,        /* Flush Mac Address table */
    L2_MSG_ID_FLUSH_FDB_STACK,  /* Flush Mac Address table - stack variant */
    L2_MSG_ID_SET_MSTI_STP_STATE, /* Set STP state for one MSTI port - to the same value*/
    L2_MSG_ID_SET_MSTI_STP_STATE_ALL, /* Set STP state for all MSTIs on port - indivdually */
    L2_MSG_ID_SET_STP_STATES,   /* Set STP state for port, all MSTIs on port */
    L2_MSG_ID_SET_MSTI_MAP,     /* Set MSTI map */
} l2_msg_id_t;

typedef enum {
    L2_PORT_TYPE_VLAN,          /* VLAN */
    L2_PORT_TYPE_POAG,          /* Physical switch port / LLAG */
    L2_PORT_TYPE_GLAG,          /* GLAG */
} l2_port_type;

typedef struct l2_msg {
    l2_msg_id_t msg_id;

    /* Message data, depending on message ID */
    union {
        /* L2_MSG_ID_FRAME_RX_IND */
        struct {
            vtss_module_id_t modid;
            vtss_port_no_t   switchport;
            size_t           len;
            vtss_vid_t       vid;
            vtss_glag_no_t   glag_no;
        } rx_ind;

        /* L2_MSG_ID_FRAME_TX_REQ */
        struct {
            vtss_port_no_t    switchport;
            size_t            len;
            l2_port_no_t      l2port;
            vtss_isid_t       isid;
        } tx_req;

        /* L2_MSG_ID_SET_STP_STATE */
        struct {
            vtss_port_no_t   switchport;
            vtss_stp_state_t stp_state;
        } set_stp_state;

        /* L2_MSG_ID_SYNC_STP_STATE */
        struct {
            vtss_stp_state_t stp_states[L2_MAX_SWITCH_PORTS];
        } synch_stp_state;

        /* L2_MSG_ID_FLUSH_FDB */
        struct {
            l2_port_type type;  /* Port type */
            int port;           /* port, LLAG, GLAG */
            vtss_common_vlanid_t vlan_id;
        } flush_fdb;

        /* L2_MSG_ID_FLUSH_FDB_STACK */
#if defined(VTSS_FEATURE_VSTAX_V2)
        struct {
            vtss_vstax_upsid_t upsid;
            vtss_vstax_upspn_t upspn;
        } flush_fdb_stack;
#endif  /* VTSS_FEATURE_VSTAX_V2 */

        /* L2_MSG_ID_SET_MSTI_STP_STATE, 
         * L2_MSG_ID_SET_MSTI_STP_STATE_ALL */
        struct {
            uchar	      msti;
            vtss_port_no_t    switchport;
            vtss_stp_state_t  stp_state;
        } set_msti_stp_state;

        /* L2_MSG_ID_SET_MSTI_STP_STATES */
        struct {
            vtss_port_no_t    switchport;
            vtss_stp_state_t  port_state;
            vtss_stp_state_t  msti_state[N_L2_MSTI_MAX];
        } set_stp_states;

        /* L2_MSG_ID_SET_MSTI_MAP */
        struct {
            BOOL	all_to_cist;   /* Map all VLANs to CIST */
            size_t	maplen;        /* Length of map below */
            uchar	map[];         /* VLAN-to-MSTI mappings (entry zero unused) */
        } set_msti_map;

    } data;
} l2_msg_t;

#define STP_STATE_CHANGE_REG_MAX 4

static void 
l2local_flush_vport(l2_port_type type, 
                    int port,
                    vtss_common_vlanid_t vlan_id);

#endif /* _COMMON_OS_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
