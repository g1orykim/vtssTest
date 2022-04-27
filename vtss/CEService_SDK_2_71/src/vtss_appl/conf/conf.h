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

#ifndef _VTSS_CONF_H_
#define _VTSS_CONF_H_

#define CONF_HDR_COOKIE_UNCOMP  0x56545353 /* Uncompressed cookie 'VTSS' */
#define CONF_HDR_COOKIE_COMP    0x53535456 /* Compressed cookie 'SSTV' */
#define CONF_HDR_VERSION_LOCAL  3          /* Local section version number */
#define CONF_HDR_VERSION_GLOBAL 3          /* Global section version number */

#define CONF_SIZE_MAX       (5*1024*1024)  /* Maximum size of uncompressed data */
#define CONF_BLK_MAX_LOCAL  (32*1024)      /* Maximum block size in local section */
#define CONF_BLK_MAX_GLOBAL (1280*1024)    /* Maximum block size in global section */

#define CONF_THREAD_FLAG_COMMIT (1 << 0) /* Flush pending changes after delay */
#define CONF_THREAD_FLAG_FLUSH  (1 << 1) /* Flush pending changes immediately */

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_CONF

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  Configuration block structures
 * ================================================================= */

/* Configuration file header */
typedef struct {
    ulong size;       /* Total length */
    ulong cookie;     /* Cookie */
    ulong save_count; /* Number of Flash save operations */
    ulong version;    /* Version number */
} conf_hdr_t;

/* Configuration block header */
typedef struct {
    ulong         size;  /* Block length */
    conf_blk_id_t id;    /* Block ID */
} conf_blk_hdr_t;

/* Configuration block */
typedef struct conf_blk_t {
    struct conf_blk_t *next;        /* Next in list */
    void              *data;        /* Configuration data */
    ulong             change_count; /* Number of changes */
    ulong             crc;          /* CRC for change detection */
    conf_blk_hdr_t    hdr;          /* Header */
} conf_blk_t;

/* Configuration section */
typedef struct {
    conf_blk_t    *blk_list;     /* List of blocks */
    const char    *name;         /* Symbolic name */
    BOOL          changed;       /* Change flag */
    BOOL          copy;          /* Force copy to slaves */ 
    BOOL          timer_started; /* Timer started flag */
    vtss_mtimer_t mtimer;        /* Millisec timer */
    ulong         save_count;    /* Number of Flash save operations */
    ulong         crc;           /* CRC for change detection */
} conf_section_t;

/* ================================================================= *
 *  Stack messages
 * ================================================================= */

typedef enum {
    CONF_MSG_CONF_SET_REQ /* Set configuration data */
} conf_msg_id_t;

/* CONF_MSG_CONF_SET_REQ */
typedef struct {
    conf_msg_id_t msg_id;  /* Message ID */
    uchar         data[0]; /* Configuration data */
} conf_msg_conf_set_req_t;

/* ================================================================= *
 *  Global variable structure
 * ================================================================= */

/* Structure for global variables */
typedef struct {
    /* Thread variables */
    cyg_handle_t     thread_handle;
    cyg_thread       thread_block;
    char             thread_stack[THREAD_DEFAULT_STACK_SIZE];

    /* Board and application configuration variables */
    conf_board_t     board;
    BOOL             board_changed;
    critd_t          crit;
    conf_section_t   section[CONF_SEC_CNT];
    int              msg_tx_count; /* Number of Tx messages outstanding */
    BOOL             isid_copy[VTSS_ISID_CNT];
    conf_mgmt_conf_t conf;
} conf_global_t;

#endif /* _VTSS_CONF_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
