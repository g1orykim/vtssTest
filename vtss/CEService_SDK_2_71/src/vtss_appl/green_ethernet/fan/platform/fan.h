/*

 Vitesse Switch API software.

 Copyright (c) 2002-2008 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _FAN_H_
#define _FAN_H_

#include "port_api.h" // For VTSS_FRONT_PORT_COUNT
#include "fan_api.h"  // for fan_switch_conf_t
//************************************************
// Trace definitions
//************************************************
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>
#include <vtss_trace_api.h>
#include "critd_api.h"

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_FAN
#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CONF         2
#define TRACE_GRP_CLI          3
#define TRACE_GRP_CNT          4


//************************************************
// Configuration
//************************************************
#define FAN_CONF_VERSION       1

// Default values for the configuration
#define FAN_CONF_T_ON_DEFAULT 75
#define FAN_CONF_T_MAX_DEFAULT 100



// Configuration for the whole stack - Overall configuration (valid on master only).
typedef struct {
    // One instance of the global configuration (configuration that are shared for all switches in the stack)
    fan_glbl_conf_t   glbl_conf;

    // One instance per switch in the stack of the switch configuration.
    // Indices are in the range [0; VTSS_ISID_CNT[, so all derefs must
    // subtract VTSS_ISID_START from @isid to index correctly.
    // *** None at the moment *** fan_switch_conf_t switch_conf[VTSS_ISID_CNT];
} fan_stack_conf_t;


// Overall configuration as saved in flash.
typedef struct {
    fan_stack_conf_t  stack_conf; // Configuration for the whole stack
    ulong             version;    // Current version of the configuration in flash.
} fan_flash_conf_t;



//************************************************
// messages
//************************************************

/* message buffer */
typedef struct {
    vtss_os_sem_t *sem; /* Semaphore */
    uchar         *msg; /* Message */
} fan_msg_buf_t;


typedef enum {
    FAN_MSG_ID_CONF_SET_REQ,   /* Configuration set request (no reply) */
    FAN_MSG_ID_STATUS_REQ,     /* Status request */
    FAN_MSG_ID_STATUS_REP,     /* Status reply */
} fan_msg_id_t;


// Message for configuration
typedef struct {
    fan_msg_id_t              msg_id;        /* Message ID: */
    fan_local_conf_t          local_conf;   /* Configuration that are local for a switch*/
} fan_msg_local_switch_conf_t;

// Message for status
typedef struct {
    fan_msg_id_t              msg_id;        /* Message ID: */
    fan_local_status_t        status;        // Status
} fan_msg_local_switch_status_t;


// Message for requests without
typedef struct {
    fan_msg_id_t             msg_id;        /* Message ID: */
} fan_msg_id_req_t;

/* Using the largest of the lot. */
#define msg_size  sizeof(fan_msg_local_switch_conf_t)

typedef struct {

    /* Thread variables */
    cyg_handle_t         thread_handle;
    cyg_thread           thread_block;
    char                 thread_stack[THREAD_DEFAULT_STACK_SIZE];


    /* Request buffer and semaphore */
    struct {
        vtss_os_sem_t    sem;
        uchar            msg[msg_size];
    } request;

    /* Reply buffer and semaphore. */
    struct {
        vtss_os_sem_t    sem;
        uchar            msg[msg_size];
    } reply;
} fan_msg_t;


//************************************************
// Critical region
//************************************************
#include <vtss_ecos_mutex_api.h>
typedef struct {
    vtss_ecos_mutex_t if_mutex;    /* Protection for multiple user interfaces access */
} fan_mutex_t;



//************************************************
// Functions declartions
//************************************************



//************************************************
// misc
//************************************************


#endif /* _FAN_H_ */

//****************************************************************************
//End of file.                                                               *
//****************************************************************************
