/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_VOICE_VLAN_H_
#define _VTSS_VOICE_VLAN_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "voice_vlan_api.h"
#include "vtss_module_id.h"
#ifdef VTSS_SW_OPTION_LLDP
#include "lldp_api.h"
#endif

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_VOICE_VLAN

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  VOICE_VLAN configuration blocks
 * ================================================================= */

/* Block versions */
#define VOICE_VLAN_CONF_BLK_VERSION         1
#define VOICE_VLAN_PORT_CONF_BLK_VERSION    1
#define VOICE_VLAN_OUI_CONF_BLK_VERSION     1

/* VOICE_VLAN configuration block */
typedef struct {
    u32                 version;            /* Block version */
    voice_vlan_conf_t   conf;               /* VOICE_VLAN configuration */
} voice_vlan_conf_blk_t;

/* VOICE_VLAN port configuration block */
typedef struct {
    u32                     version;                    /* Block version */
    voice_vlan_port_conf_t  port_conf[VTSS_ISID_CNT];   /* VOICE_VLAN port configuration */
} voice_vlan_port_conf_blk_t;


/* VOICE_VLAN OUI configuration */
typedef struct {
    u32                     entry_num;
    voice_vlan_oui_entry_t  entry[VOICE_VLAN_OUI_ENTRIES_CNT];
} voice_vlan_oui_conf_t;

/* VOICE_VLAN OUI configuration block */
typedef struct {
    u32                     version;                    /* Block version */
    voice_vlan_oui_conf_t   oui_conf;                   /* VOICE_VLAN OUI configuration */
} voice_vlan_oui_conf_blk_t;

/* VOICE_VLAN LLDP telephony MAC */
typedef struct {
    u32                                     entry_num;
    voice_vlan_lldp_telephony_mac_entry_t   entry[4 * VTSS_ISID_CNT *VTSS_PORTS];
} voice_vlan_lldp_telephony_mac_t;


/* ================================================================= *
 *  VOICE_VLAN stack messages
 * ================================================================= */

/* VOICE_VLAN messages IDs */
typedef enum {
    VOICE_VLAN_MSG_ID_LLDP_CB_IND,      /* LLDP callback indication */
    VOICE_VLAN_MSG_ID_QCE_SET_REQ      /* Voice VLAN QCE configuration set request (no reply) */
} voice_vlan_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    voice_vlan_msg_id_t    msg_id;

    /* Request data, depending on message ID */
    union {
        /* VOICE_VLAN_MSG_ID_PORT_CHANGE_CB_IND */
        struct {
            vtss_port_no_t  port_no;
            BOOL            link;
        } port_change_cb_ind;

        /* VOICE_VLAN_MSG_ID_LLDP_CB_IND */
        struct {
            vtss_port_no_t      port_no;
#ifdef VTSS_SW_OPTION_LLDP
            lldp_remote_entry_t entry;
#endif
        } lldp_cb_ind;

        /* VOICE_VLAN_MSG_ID_QCE_SET_REQ */
        struct {
            BOOL            del_qce;
            BOOL            is_port_list;
            BOOL            port_list[VTSS_PORT_ARRAY_SIZE];
            BOOL            is_port_add;
            BOOL            is_port_del;
            vtss_port_no_t  iport;
            BOOL            change_traffic_class;
            vtss_prio_t     traffic_class;
            BOOL            change_vid;
            vtss_vid_t      vid;
        } qce_conf_set;
    } req;
} voice_vlan_msg_req_t;


/* ================================================================= *
 *  VOICE_VLAN global structure
 * ================================================================= */

/* VOICE_VLAN global structure */
typedef struct {
    critd_t                         crit;
    voice_vlan_conf_t               conf;
    cyg_flag_t                      conf_flags;
    vtss_mtimer_t                   conf_timer[VTSS_ISID_END];
    voice_vlan_port_conf_t          port_conf[VTSS_ISID_END];
    voice_vlan_oui_conf_t           oui_conf;
    voice_vlan_lldp_telephony_mac_t lldp_telephony_mac;
    u32                             oui_cnt[VTSS_ISID_END][VTSS_PORTS];
    BOOL                            sw_learn[VTSS_ISID_END][VTSS_PORTS];
    vtss_qcl_id_t                   qcl_no[VTSS_ISID_END][VTSS_PORTS];

    /* Request buffer and semaphore */
    struct {
        vtss_os_sem_t           sem;
        voice_vlan_msg_req_t    msg;
    } request;

} voice_vlan_global_t;

#endif /* _VTSS_VOICE_VLAN_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
