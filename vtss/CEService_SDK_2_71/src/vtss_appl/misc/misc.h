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
 
 $Id$
 $Revision$

*/

#ifndef _VTSS_MISC_H_
#define _VTSS_MISC_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_MISC

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  ACL stack messages
 * ================================================================= */

/* ACL messages IDs */
typedef enum {
    MISC_MSG_ID_REG_READ_REQ,  /* Register read request */
    MISC_MSG_ID_REG_WRITE_REQ, /* Register write request */
    MISC_MSG_ID_REG_READ_REP,  /* Register read reply */
    MISC_MSG_ID_PHY_READ_REQ,  /* Register read request */
    MISC_MSG_ID_PHY_WRITE_REQ, /* Register write request */
    MISC_MSG_ID_PHY_READ_REP,  /* Register read reply */
    MISC_MSG_ID_SUSPEND_RESUME /* Suspend/resume */
} misc_msg_id_t;

/* Message */
typedef struct {
    /* Message ID */
    misc_msg_id_t msg_id;

    /* Message data, depending on message ID */
    union {
        /* Register access */
        struct {
            vtss_chip_no_t chip_no;
            ulong          addr;
            ulong          value;
        } reg;

        /* PHY access */
        struct {
            vtss_port_no_t port_no;
            uint           addr;
            ushort         value;
            BOOL           mmd_access;
            ushort         devad;
        } phy;
        
        /* Suspend/resume */
        struct {
            BOOL resume;
        } suspend_resume;
    } data;
} misc_msg_t;

/* Message buffer */
typedef struct {
    vtss_os_sem_t *sem; /* Semaphore */
    misc_msg_t    *msg; /* Message */
} misc_msg_buf_t;

/* ================================================================= *
 *  Global structure
 * ================================================================= */

#define MISC_FLAG_READ_IDLE (1<<0) /* Read is idle */
#define MISC_FLAG_READ_DONE (1<<1) /* Read is done */

/* Global structure */
typedef struct {
    vtss_os_sem_t  sem;     /* Message semaphore */
    misc_msg_t     msg;     /* Message buffer */
    cyg_flag_t     flags;   /* Message flags */
    ulong          value;   /* Read value */
    vtss_chip_no_t chip_no; /* Chip number context */
    vtss_inst_t    phy_inst;/* Phy instance used by debug phy commands */
} misc_global_t;


#endif /* _VTSS_MISC_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
