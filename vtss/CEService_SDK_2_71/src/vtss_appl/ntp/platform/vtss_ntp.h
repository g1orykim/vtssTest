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

#ifndef _VTSS_NTP_H_
#define _VTSS_NTP_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "vtss_ntp_api.h"
#include "critd_api.h"
#include "vtss_module_id.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_NTP

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  ntp configuration blocks
 * ================================================================= */

#define NTP_DEFAULT_DRIFT         0 /* 9500: the value was used before BugZilla# 1022 fixed */

/* Block versions */
#define NTP_CONF_BLK_VERSION      1

/* ntp configuration block */
typedef struct {
    unsigned long   version;    /* Block version */
    ntp_conf_t      ntp_conf;   /* ntp configuration */
} ntp_conf_blk_t;

/* ================================================================= *
 *  ntp stack messages
 * ================================================================= */

/* ntp messages IDs */
typedef enum {
    NTP_MSG_ID_NTP_CONF_SET_REQ,          /* ntp configuration set request (no reply) */
} ntp_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    ntp_msg_id_t    msg_id;

    /* Request data, depending on message ID */
    union {
        /* NTP_MSG_ID_NTP_CONF_SET_REQ */
        struct {
            ntp_conf_t  conf;
        } conf_set;
    } req;
} ntp_msg_req_t;

/* ntp message buffer */
typedef struct {
    vtss_os_sem_t   *sem; /* Semaphore */
    void            *msg; /* Message */
} ntp_msg_buf_t;

/* ================================================================= *
 *  ntp global structure
 * ================================================================= */

/* ntp global structure */
typedef struct {
    critd_t     crit;
    ntp_conf_t  ntp_conf;

    /* Request buffer and semaphore */
    struct {
        vtss_os_sem_t   sem;
        ntp_msg_req_t   msg;
    } request;

} ntp_global_t;

#endif /* _VTSS_NTP_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
