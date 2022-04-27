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

#ifndef _VTSS_LOOP_PROTECT_API_H_
#define _VTSS_LOOP_PROTECT_API_H_

#include "main.h"     /* MODULE_ERROR_START */
#include "vtss_api.h" /* For vtss_rc, vtss_vid_t, etc. */

#include <time.h>                /* time_t */

/* LOOP_PROTECT error codes (vtss_rc) */
enum {
    LOOP_PROTECT_ERROR_GEN = MODULE_ERROR_START(VTSS_MODULE_ID_LOOP_PROTECT),  /* Generic error code */
    LOOP_PROTECT_ERROR_PARM,    /* Illegal parameter */
    LOOP_PROTECT_ERROR_INACTIVE, /* Port is inactive */
    LOOP_PROTECT_ERROR_TIMEOUT, /* Timeout */
    LOOP_PROTECT_ERROR_MSGALLOC, /* Malloc error */
};

typedef enum {
    LOOP_PROTECT_ACTION_SHUTDOWN,
    LOOP_PROTECT_ACTION_SHUT_LOG,
    LOOP_PROTECT_ACTION_LOG_ONLY,
} loop_protect_action_t;

/* default configuration */
#define LOOP_PROTECT_DEFAULT_GLOBAL_ENABLED         FALSE
#define LOOP_PROTECT_DEFAULT_GLOBAL_TX_TIME         5
#define LOOP_PROTECT_DEFAULT_GLOBAL_SHUTDOWN_TIME   180
#define LOOP_PROTECT_DEFAULT_PORT_ENABLED           TRUE
#define LOOP_PROTECT_DEFAULT_PORT_ACTION            LOOP_PROTECT_ACTION_SHUTDOWN
#define LOOP_PROTECT_DEFAULT_PORT_TX_MODE           TRUE

/* Config structure */

typedef struct {
    BOOL                  enabled;  /**< Enabled loop protection on port */
    loop_protect_action_t action;   /**< Action if loop detected */
    BOOL                  transmit; /**< Actively generate PDUs */
} loop_protect_port_conf_t;

typedef struct {
    BOOL enabled;
    int  transmission_time;
    int  shutdown_time;
} loop_protect_conf_t;

/* LOOP_PROTECT error text */
char *loop_protect_error_txt(vtss_rc rc);

/* Set Loop_Protect General configuration  */
vtss_rc loop_protect_conf_set(const loop_protect_conf_t *conf);

/* Get Loop_Protect General configuration  */
vtss_rc loop_protect_conf_get(loop_protect_conf_t *conf);

/* Set Port Loop_Protect General configuration  */
vtss_rc loop_protect_conf_port_set(vtss_isid_t isid, 
                                   vtss_port_no_t port_no, 
                                   const loop_protect_port_conf_t *conf);

/* Get Port Loop_Protect General configuration  */
vtss_rc loop_protect_conf_port_get(vtss_isid_t isid, 
                                   vtss_port_no_t port_no, 
                                   loop_protect_port_conf_t *conf);

typedef struct {
    BOOL   disabled;
    BOOL   loop_detect;
    u32    loops;
    time_t last_loop;
} loop_protect_port_info_t;

/* Get Loop_Protect port info  */
vtss_rc loop_protect_port_info_get(vtss_isid_t isid, vtss_port_no_t port_no, loop_protect_port_info_t *info);

const char *loop_protect_action2string(loop_protect_action_t action);

/* Initialize module */
vtss_rc loop_protect_init(vtss_init_data_t *data);

#endif /* _VTSS_LOOP_PROTECT_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
