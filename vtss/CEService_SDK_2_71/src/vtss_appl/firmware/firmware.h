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
 
*/

#ifndef _VTSS_FIRMWARE_H_
#define _VTSS_FIRMWARE_H_

#include "firmware_api.h"
#include "vtss_module_id.h"
#include "control_api.h" /* For vtss_restart_t */

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_FIRMWARE

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CNT          1

#include <vtss_trace_api.h>
/* ============== */

/* Firmware update */
typedef struct {
    cli_iolayer_t       *io;
    const unsigned char *buffer;        /* Firmware buffer (malloc) */
    size_t               length;        /* malloc'ed bytes */
    cyg_bool             sync;          /* Signal completion through request (no reboot) */
    cyg_sem_t            sem;           /* Synchronization element */
    vtss_rc              rc;            /* Async reply */
    unsigned long        image_version; /* Currently: 0 = image uses fixed flash layout, 1 = image uses FIS to get flash layout, falls back to fixed flash layout */
    cyg_uint32		 chiptype;      /* Chip target - VSCXXXX */
    cyg_uint8            cputype;       /* CPU Target - ARM=1, MIPS=2 */
    vtss_restart_t       restart;       /* How to reboot when done uploading */
} firmware_flash_args_t;

/* ================================================================= *
 *  Port stack messages
 * ================================================================= */

/* Main thread mail message types */
typedef enum {
    FIRMWARE_MBMSG_CHECKGO = 0xcafe, /* Check whether all switches are ready */
    /* (void *) = */
} firmware_mbmsg_t;

/* Port messages IDs */
typedef enum {
    FIRMWARE_MSG_ID_IMAGE_CNF,   /* Image received, ready */
    FIRMWARE_MSG_ID_IMAGE_BURN,  /* Start flash update    */
    FIRMWARE_MSG_ID_IMAGE_ABRT,  /* Abort flash update    */
    FIRMWARE_MSG_ID_IMAGE_BEGIN, /* Begin firmware update */
} firmware_msg_id_t;

typedef struct firmware_msg {
    firmware_msg_id_t msg_id;
    vtss_restart_t    restart;
} firmware_msg_t;

void firmware_program_doit(firmware_flash_args_t *pReq, const char *name);

vtss_rc firmware_update_mkreq(cli_iolayer_t *io, 
                              const unsigned char *buffer,
                              size_t length,
                              firmware_flash_args_t **ppReq,
                              cyg_bool sync,
                              unsigned long type,
                              unsigned long flag,
                              vtss_restart_t restart);

#endif /* _VTSS_FIRMWARE_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
