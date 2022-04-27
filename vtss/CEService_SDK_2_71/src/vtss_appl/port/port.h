/*

 Vitesse Switch API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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
 
 $Id$
 $Revision$

*/

#ifndef _VTSS_PORT_H_
#define _VTSS_PORT_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_PORT

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_ITER         1
#define TRACE_GRP_CRIT         2
#define TRACE_GRP_SFP          3
#define TRACE_GRP_ICLI         4
#define TRACE_GRP_CNT          5

#include <vtss_trace_api.h>
#include "critd_api.h" // for critd_t
/* ================================================================= *
 *  Port change events
 * ================================================================= */

#define PORT_CHANGE_REG_MAX 16

/* Port change registration table */
typedef struct {
    int               count;
    port_change_reg_t reg[PORT_CHANGE_REG_MAX];
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    port_oobfc_conf_t oobfc_conf[2];
    BOOL              mode_change; /* Rember that there is a mode change */
#endif
} port_change_table_t;

#define PORT_CHANGE_USER_LENGTH 16

/* Port global change registration table */
typedef struct {
    int                      count;
    port_global_change_reg_t reg[PORT_CHANGE_REG_MAX];
} port_global_change_table_t;

#define PORT_SHUTDOWN_REG_MAX 16

/* Port shutdown registration table */
typedef struct {
    int                 count;
    port_shutdown_reg_t reg[PORT_SHUTDOWN_REG_MAX];
} port_shutdown_table_t;

/* ================================================================= *
 *  Port configuration block
 * ================================================================= */

#define PORT_CONF_VERSION 1

/* Port configuration block */
typedef struct {
    ulong       version; /* Block version */
    port_conf_t conf[VTSS_ISID_CNT*VTSS_PORTS];
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    port_oobfc_conf_t oobfc_conf[2];
    BOOL              mode_change; /* Rember that there is a mode change */
#endif
} port_conf_blk_t;

/* ================================================================= *
 *  Port veriPHY state
 * ================================================================= */

typedef struct {
    BOOL                      running; /* Running or idle */
    BOOL                      valid;   /* Result valid */
    vtss_phy_veriphy_result_t result;  /* Result */
    port_veriphy_mode_t       mode;    /* The mode to run VeriPhy. */
    u8                        repeat_cnt; /* Number of time VeriPhy should be repeated */
    u8                        variate_cnt[VTSS_PORTS]; /* Number of time the result is not the same */
} port_veriphy_t;

/* ================================================================= *
 *  Port stack messages
 * ================================================================= */

/* Port messages IDs */
typedef enum {
    PORT_MSG_ID_CONF_SET_REQ,       /* Configuration set request (no reply) */
    PORT_MSG_ID_CONF_GET_REQ,       /* OBSOLETE: Configuration get request */
    PORT_MSG_ID_CONF_GET_REP,       /* OBSOLETE: Configuration get reply */
    PORT_MSG_ID_STATUS_GET_REQ,     /* Status get request */
    PORT_MSG_ID_STATUS_GET_REP,     /* Status get reply */
    PORT_MSG_ID_COUNTERS_GET_REQ,   /* Statistics get request */
    PORT_MSG_ID_COUNTERS_GET_REP,   /* Statistics get reply */
    PORT_MSG_ID_COUNTERS_CLEAR_REQ, /* Statistics clear request (no reply) */
    PORT_MSG_ID_VERIPHY_GET_REQ,    /* VeriPHY get request */
    PORT_MSG_ID_VERIPHY_GET_REP     /* VeriPHY get reply */
} port_msg_id_t;

/* Request message */
typedef struct {
    /* Message ID */
    port_msg_id_t msg_id; 
    
    /* Request data, depending on message ID */
    union {
        /* PORT_MSG_ID_CONF_SET_REQ */
        struct {
            port_conf_t conf[VTSS_PORTS]; /* Configuration */
        } conf_set;
        
        /* PORT_MSG_ID_STATUS_GET_REQ: No data */

        /* PORT_MSG_ID_COUNTERS_GET_REQ: No data */

        /* PORT_MSG_ID_COUNTERS_CLEAR_REQ */
        struct {
            vtss_port_no_t port_no; /* Port number */
        } counters_clr;
        
        /* PORT_MSG_ID_VERIPHY_GET_REQ */
        struct {
            port_veriphy_mode_t mode[VTSS_PORTS]; /* Mode */
        } veriphy_get;
    } req; 
} port_msg_req_t;

/* Reply message */
typedef struct {
    /* Message ID */
    port_msg_id_t msg_id; 
    
    /* Reply data, depending on message ID */
    union {
        /* PORT_MSG_ID_STATUS_GET_REP */
        struct {
            port_status_t status[VTSS_PORTS];    /* Port status */
        } status_get;

        /* PORT_MSG_ID_COUNTERS_GET_REP */
        struct {
            vtss_port_counters_t counters[VTSS_PORTS]; /* Port counters */
        } counters_get;

        /* PORT_MSG_ID_VERIPHY_GET_REP */
        struct {
            port_veriphy_t result[VTSS_PORTS];
        } veriphy_get;
    } rep; 
} port_msg_rep_t;

/* Port message request timer */
#define PORT_REQ_TIMEOUT 5

/* Port status and counters timer */
#define PORT_CONFIG_TIMER   1000
#define PORT_STATUS_TIMER   1000
#define PORT_COUNTERS_TIMER 1000

/* Port change flags */
#define PORT_CHANGE_UP   0x01
#define PORT_CHANGE_DOWN 0x02
#define PORT_CHANGE_ANY  (PORT_CHANGE_UP | PORT_CHANGE_DOWN)         

/* Port module state */
typedef enum {
    PORT_MODULE_STATE_INIT, /* Initial state */
    PORT_MODULE_STATE_CONF, /* Configuration applied after first SWITCH_ADD */
    PORT_MODULE_STATE_ANEG, /* Auto negotiating ports (some seconds) */
    PORT_MODULE_STATE_POLL, /* Polling all ports */
    PORT_MODULE_STATE_READY /* Warm start ready  */
} port_module_state_t;



/* ================================================================= *
 *  Port global variables structure
 * ================================================================= */
#define PORT_MGMT_THREAD_STACK_SIZE     (THREAD_DEFAULT_STACK_SIZE * 2) // It needs 8.3K stack size on 48 ports device

typedef struct {
    /* Thread variables */
    cyg_handle_t         thread_handle;
    cyg_thread           thread_block;
    char                 thread_stack[PORT_MGMT_THREAD_STACK_SIZE];
    BOOL                 thread_suspended;
    
    /* Critical region protection protecting callbacks, i.e.
       #change_table, #global_change_table, and #shutdown_table */
    critd_t              cb_crit;

    /* Critical region protection protecting the remaining variables in this struct */
    critd_t              crit; 

    port_module_state_t  module_state;                        /* Port module state */
    BOOL                 isid_added[VTSS_ISID_END];           /* SWITCH_ADD received */
    int                  board_type[VTSS_ISID_END];           /* Board type */
    u32                  port_count[VTSS_ISID_END];           /* Number of ports */
    vtss_port_no_t       stack_port_0[VTSS_ISID_END];         /* Stack port 0 */
    vtss_port_no_t       stack_port_1[VTSS_ISID_END];         /* Stack port 1 */
    port_conf_t          config[VTSS_ISID_END][VTSS_PORTS];   /* Port configuration */
    port_vol_conf_t      vol_conf[PORT_USER_CNT][VTSS_ISID_END][VTSS_PORTS]; /* Volatile conf */
    vtss_mtimer_t        config_timer[VTSS_ISID_END];         /* Port configuration timer */
    cyg_flag_t           config_flags;                        /* Port configuration flags */
    port_status_t        status[VTSS_ISID_END][VTSS_PORTS];   /* Port status */
    BOOL                 cap_valid[VTSS_ISID_END][VTSS_PORTS];/* Port capability valid */
    vtss_mtimer_t        status_timer[VTSS_ISID_END];         /* Port status timer */
    cyg_flag_t           status_flags;                        /* Port status flags */
    vtss_port_counters_t counters[VTSS_ISID_END][VTSS_PORTS]; /* Port counters */
    vtss_mtimer_t        counters_timer[VTSS_ISID_END];       /* Port counters timer */
    cyg_flag_t           counters_flags;                      /* Port counters flags */
    port_change_table_t  change_table;              /* Port change registrations */
    port_global_change_table_t global_change_table; /* Port global change registrations */
    uchar                change_flags[VTSS_ISID_END][VTSS_PORTS]; /* Port change flags */
    port_shutdown_table_t shutdown_table;           /* Port shutdown registrations */
    port_veriphy_t       veriphy[VTSS_ISID_END][VTSS_PORTS];  /* Port veriPHY */
    vtss_port_interface_t mac_sfp_if[VTSS_PORT_NO_END];       /* MAC Interface to SFP module */
    vtss_port_status_t    aneg_status[VTSS_PORT_NO_END];      /* Current aneg status results for the port */
#if defined(VTSS_ARCH_JAGUAR_1_CE_MAC)
    port_oobfc_conf_t    oobfc_conf[2]; /* 2 host port specific configurations */
#endif    
    /* Request message buffer pool */
    void *request;
    /* Reply message buffer pool */
    void *reply;

} port_global_t;

#endif /* _VTSS_PORT_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
