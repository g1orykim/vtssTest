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

 $Id$
 $Revision$

*/

#ifndef _SYSLOG_H_
#define _SYSLOG_H_

#include <time.h>
#include "critd_api.h"

/* ================================================================= *
 *  syslog configuration blocks
 * ================================================================= */

/* Block versions
   1: Initial version
   2: Added syslog server */
#define SYSLOG_CONF_BLK_VERSION     2

/* syslog configuration block */
typedef struct {
    u32             version;    /* Block version */
    syslog_conf_t   conf;       /* syslog configuration */
} syslog_conf_blk_t;

#define SYSLOG_FLASH_HDR_COOKIE  0x474F4C53   /* Cookie 'SLOG'(for "Syslog") */
#define SYSLOG_FLASH_HDR_VERSION 1            /* Configuration version number */

#define SYSLOG_FLASH_ENTRY_COOKIE 0x59544E53  /* Cookie 'SNTY' (for "Syslog Entry") */
#define SYSLOG_FLASH_ENTRY_VERSION 1          /* Entry version number */

#define SYSLOG_FLASH_UNINIT_FLASH_VALUE_LONG 0xFFFFFFFF

/****************************************************************************/
/****************************************************************************/
typedef struct {
    ulong size;    // Header size in bytes
    ulong cookie;  // Header cookie
    ulong version; // Header version number
} SL_flash_hdr_t;

/****************************************************************************/
/****************************************************************************/
typedef struct {
    ulong        size;    // Entry size in bytes
    ulong        cookie;  // Entry cookie
    ulong        version; // Entry version number
    time_t       time;    // Time of saving.
    syslog_cat_t cat;     // Category (either of the SYSLOG_CAT_xxx constants)
    syslog_lvl_t lvl;     // Level (either of the SYSLOG_LVL_xxx constants)
    // Here follows the NULL-terminated message
} SL_flash_entry_t;


/*---- RAM System Log ------------------------------------------------------*/

/* Stack message IDs */
typedef enum {
    SL_MSG_ID_ENTRY_GET_REQ,    /* Get entry request */
    SL_MSG_ID_ENTRY_GET_REP,    /* Get entry reply */
    SL_MSG_ID_STAT_GET_REQ,     /* Get statistics request */
    SL_MSG_ID_STAT_GET_REP,     /* Get statistics reply */
    SL_MSG_ID_CLR_REQ,          /* Clear request (no reply) */
    SL_MSG_ID_CONF_SET_REQ      /* syslog configuration set request (no reply) */
} SL_msg_id_t;

/* Stack request message */
typedef struct {
    SL_msg_id_t msg_id; /* Message ID */
    union {
        /* SL_MSG_ID_ENTRY_GET_REQ */
        struct {
            BOOL             next;
            ulong            id;
            syslog_lvl_t     lvl;
            vtss_module_id_t mid;
        } entry_get;

        /* SL_MSG_ID_STAT_GET_REQ: No data */

        /* SL_MSG_ID_CLR_REQ */
        struct {
            syslog_lvl_t     lvl;
        } entry_clear;

        /* SL_MSG_ID_CONF_SET_REQ */
        struct {
            syslog_conf_t  conf;
        } conf_set;
    } data;
} SL_msg_req_t;

/* Stack reply message */
typedef struct {
    SL_msg_id_t msg_id; /* Message ID */
    union {
        /* SL_MSG_ID_ENTRY_GET_REP */
        struct {
            BOOL               found;
            syslog_ram_entry_t entry;
        } entry_get;

        /* SL_MSG_ID_STAT_GET_REP */
        struct {
            syslog_ram_stat_t stat;
        } stat_get;
    } data;
} SL_msg_rep_t;

#define SYSLOG_RAM_SIZE (1024*1024)

/* RAM entry */
typedef struct SL_ram_entry_t {
    struct SL_ram_entry_t *next;
    ulong                 id;     /* Message ID */
    syslog_lvl_t          lvl;    /* Level */
    vtss_module_id_t      mid;    /* Module ID */
    time_t                time;   /* Time stamp */
    char                  msg[SYSLOG_RAM_MSG_MAX]; /* Message */
} SL_ram_entry_t;

/* Variables for RAM system log */
typedef struct {
    uchar             log[SYSLOG_RAM_SIZE];
    syslog_ram_stat_t stat[VTSS_ISID_END];
    cyg_flag_t        stat_flags;
    SL_ram_entry_t    *first;       /* First entry in list */
    SL_ram_entry_t    *last;        /* Last entry in list */
    ulong             current_id;   /* current ID */

    /* Request buffer */
    void *request;

    /* Reply buffer */
    void *reply;

    /* Management reply buffer */
    struct {
        vtss_os_sem_t      sem;
        cyg_flag_t         flags;
        BOOL               found;
        syslog_ram_entry_t *entry;
    } mgmt_reply;
} SL_ram_t;

/* ================================================================= *
 *  syslog global structure
 * ================================================================= */

/* syslog global structure */
typedef struct {
    critd_t         crit;
    syslog_conf_t   conf;
    cyg_flag_t      conf_flags;
    vtss_mtimer_t   conf_timer[VTSS_ISID_END];
    ulong           send_msg_id[VTSS_ISID_END];     /* Record message ID that already send to syslog server */
    time_t          current_time;
} syslog_global_t;

/*--------------------------------------------------------------------------*/

#endif /* _SYSLOG_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
