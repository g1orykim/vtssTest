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

#ifndef _VTSS_ARP_INSPECTION_H_
#define _VTSS_ARP_INSPECTION_H_


/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "arp_inspection_api.h"
#include "vtss_module_id.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_ARP_INSPECTION

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>

/* ARP Inspection ACE IDs */
#define ARP_INSPECTION_ACE_ID   1


/* ================================================================= *
 *  ARP_INSPECTION configuration blocks
 * ================================================================= */

/* Block versions */
#define ARP_INSPECTION_CONF_BLK_VERSION1    1
#define ARP_INSPECTION_CONF_BLK_VERSION2    2

/* ARP_INSPECTION configuration block */
typedef struct {
    unsigned long               version;                /* Block version */
    arp_inspection_conf_t       arp_inspection_conf;    /* ARP_INSPECTION configuration */
} arp_inspection_conf_blk_t;

typedef struct {
    u_int16_t ar_hrd;       /* format of hardware address */
#define ARPHRD_ETHER    1   /* ethernet hardware format */
#define ARPHRD_IEEE802  6   /* IEEE 802 hardware format */
#define ARPHRD_FRELAY   15  /* frame relay hardware format */
    u_int16_t ar_pro;       /* format of protocol address */
    u_int8_t  ar_hln;       /* length of hardware address */
    u_int8_t  ar_pln;       /* length of protocol address */
    u_int16_t ar_op;        /* one of: */
#define ARPOP_REQUEST   1   /* request to resolve address */
#define ARPOP_REPLY 2       /* response to previous request */
#define ARPOP_REVREQUEST 3  /* request protocol address given hardware */
#define ARPOP_REVREPLY  4   /* response giving protocol address */
#define ARPOP_INVREQUEST 8  /* request to identify peer */
#define ARPOP_INVREPLY  9   /* response identifying peer */
    /*
     * The remaining fields are variable in size,
     * according to the sizes above.
     */
    u_int8_t  ar_sha[6];    /* sender hardware address */
    ulong     ar_spa;       /* sender protocol address */
    u_int8_t  ar_tha[6];    /* target hardware address */
    ulong     ar_tpa;       /* target protocol address */
} __attribute__((packed)) vtss_arp_header;


/* ================================================================= *
 *  ARP_INSPECTION stack messages
 * ================================================================= */

/* ARP_INSPECTION messages IDs */
typedef enum {
    ARP_INSPECTION_MSG_ID_ARP_INSPECTION_CONF_SET_REQ,          /* ARP_INSPECTION configuration set request (no reply) */
    ARP_INSPECTION_MSG_ID_FRAME_RX_IND,                         /* Frame receive indication */
    ARP_INSPECTION_MSG_ID_FRAME_TX_REQ                          /* Frame transmit request */
} arp_inspection_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    arp_inspection_msg_id_t    msg_id;

    /* Request data, depending on message ID */
    union {
        /* ARP_INSPECTION_MSG_ID_ARP_INSPECTION_CONF_SET_REQ */
        struct {
            //arp_inspection_conf_t  conf;
            arp_inspection_stacking_conf_t  conf;
        } conf_set;

        /* ARP_INSPECTION_MSG_ID_FRAME_RX_IND */
        struct {
            size_t              len;
            unsigned long       vid;
            unsigned long       port_no;
        } rx_ind;
        /* ARP_INSPECTION_MSG_ID_FRAME_TX_REQ */
        struct {
            size_t              len;
            unsigned long       vid;
            unsigned long       isid;
            BOOL                port_list[VTSS_PORT_ARRAY_SIZE]; // egress port list
        } tx_req;
    } req;
} arp_inspection_msg_req_t;

/* ARP_INSPECTION message buffer */
typedef struct {
    vtss_os_sem_t   *sem;           /* Semaphore */
    void            *msg;           /* Message */
} arp_inspection_msg_buf_t;


/* ================================================================= *
 *  ARP_INSPECTION global structure
 * ================================================================= */

/* ARP_INSPECTION global structure */
typedef struct {
    critd_t                         crit;
    critd_t                         bip_crit;
    arp_inspection_conf_t           arp_inspection_conf;
    arp_inspection_entry_t          arp_inspection_dynamic_entry[ARP_INSPECTION_MAX_ENTRY_CNT];

    /* Request buffer and semaphore */
    struct {
        vtss_os_sem_t               sem;
        arp_inspection_msg_req_t    msg;
    } request;

} arp_inspection_global_t;

#endif /* _VTSS_ARP_INSPECTION_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
