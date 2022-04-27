/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_LOOP_PROTECT_H_
#define _VTSS_LOOP_PROTECT_H_

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_LOOP_PROTECT

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CRIT         1
#define TRACE_GRP_CNT          2

#include <vtss_trace_api.h>

#include "critd_api.h"
#include "loop_protect_api.h"

#include <tomcrypt.h>

/* Block versions */
#define LOOP_PROTECT_CONF_BLK_VERSION            0x86876203

typedef struct {
    loop_protect_conf_t global;
    loop_protect_port_conf_t ports[VTSS_ISID_END][VTSS_PORT_ARRAY_SIZE];
} lprot_conf_t;

/* LOOP_PROTECT configuration block */
typedef struct {
    ulong        version;
    lprot_conf_t conf;
} loop_protect_conf_blk_t;

#define LPROT_ACE_ID	1       /* ID of sole ACE entry */

#define LPROT_TTL_NOCHECK   2   /* Seconds */
#define LPROT_MAX_TTL_MSECS 5000 /* Msecs */

typedef struct {
    u8  dst[6];
    u8  src[6];
    u16 oui;                    /* 9003 */
#define LPROT_PROTVERSION 1
    u16 version;
    u32 tstamp;                 /* Aligned! */
    u8  switchmac[6];
    u16 usid;
    u16 lport;
    u8  authcode[20];
    u8  pad[8];
} __attribute__((packed)) loop_prot_pdu_t;

/* LOOP_PROTECT messages IDs */
typedef enum {
    LOOP_PROTECT_MSG_ID_CONF,
    LOOP_PROTECT_MSG_ID_CONF_PORT,
    LOOP_PROTECT_MSG_ID_PORT_CTL,
    LOOP_PROTECT_MSG_ID_PORT_STATUS_REQ,
    LOOP_PROTECT_MSG_ID_PORT_STATUS_RSP,
} loop_protect_msg_id_t;

typedef struct loop_protect_msg {
    loop_protect_msg_id_t msg_id;
    /* Message data, depending on message ID */
    union {
        struct {
            u8 key[SHA1_HASH_SIZE];
            u8 mac[6];
            vtss_usid_t usid;
            loop_protect_conf_t global_conf;
            loop_protect_port_conf_t port_conf[VTSS_PORT_ARRAY_SIZE];
        } unit_conf;
        struct {
            vtss_port_no_t port_no;
            loop_protect_port_conf_t port_conf;
        } port_conf;
        struct {
            vtss_port_no_t port_no;
            BOOL           disable;
        } port_ctl;
        struct {
            loop_protect_port_info_t ports[VTSS_PORT_ARRAY_SIZE];
        } port_info;
    } data;
} loop_protect_msg_t;

#endif /* _VTSS_LOOP_PROTECT_H_ */

/*********************************************************************/
/*                                                                   */
/*  End of file.                                                     */
/*                                                                   */
/*********************************************************************/
