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

/* ================================================================= *
 * Trace definitions
 * ================================================================= */
#include "vtss_module_id.h"
#include "vtss_trace_lvl_api.h"
#include "vtss_trace_api.h"
#include "poe_api.h"

#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_POE
#define VTSS_TRACE_GRP_DEFAULT      0
#define VTSS_TRACE_GRP_CRIT         1
#define VTSS_TRACE_GRP_MGMT         2
#define VTSS_TRACE_GRP_CONF         3
#define VTSS_TRACE_GRP_STATUS       4
#define VTSS_TRACE_GRP_CUSTOM       5
#define VTSS_TRACE_GRP_MSG          6
#define VTSS_TRACE_GRP_ICLI         7
#define VTSS_TRACE_GRP_CNT          8

/**********************************************************************
 ** PoE types
 **********************************************************************/
#define POE_CONF_VERSION    1 // Configuration version.

/**********************************************************************
 ** Configuration structs
 **********************************************************************/

/* Configuration stored on flash  */
typedef struct {
    unsigned long     version;      /* Block version */
    poe_stack_conf_t  stack_cfg;
} poe_flash_conf_t;


/**********************************************************************
 * PoE Messages
 **********************************************************************/
typedef enum {
    POE_MSG_ID_CONF_SET_REQ,          /* Configuration set request (no reply) */
    POE_MSG_ID_GET_STAT_REQ,          /* Master's request to get status from a slave */
    POE_MSG_ID_GET_STAT_REP,          /* Slave's reply upon a get status */
    POE_MSG_ID_GET_PD_CLASSES_REP,    /* Slave's reply upon a get PD classes  */
    POE_MSG_ID_GET_PD_CLASSES_REQ,    /* Master's request to get PD classes from a slave */
} poe_msg_id_t;

// Request message
typedef struct {
    // Message ID
    poe_msg_id_t msg_id;

    union {
        /* POE_MSG_ID_CONF_SET_REQ */
        poe_local_conf_t local_conf;

        /* POE_MSG_ID_GET_STAT_REQ: No data */

        /* POE_MSG_ID_GET_PD_CLASSES_REQ: No data */
    } req;
} poe_msg_req_t;

// Reply message
typedef struct {
    // Message ID
    poe_msg_id_t msg_id;

    union {
        /* POE_MSG_ID_CONF_STAT_REP */
        poe_status_t status;

        /* POE_MSG_ID_GET_PD_CLASSES_REP */
        char         classes[VTSS_PORTS];
    } rep;
} poe_msg_rep_t;

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
