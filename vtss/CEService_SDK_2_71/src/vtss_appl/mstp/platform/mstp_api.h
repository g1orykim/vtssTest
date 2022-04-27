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

#ifndef _MSTP_API_H_
#define _MSTP_API_H_

#include "vtss_mstp_api.h"
#include "vtss_mstp_callout.h"
#include "l2proto_api.h"

#if defined(VTSS_SW_OPTION_BUILD_SMB) || defined(VTSS_SW_OPTION_BUILD_CE)
#define VTSS_MSTP_FULL	1
#endif

#define L2_NULL			0xffff /* Layer 2 invalid port */

typedef struct {
    BOOL enabled;               /* Enabled for xSTP */
    BOOL active;                /* Active for xSTP */
    l2_port_no_t parent;        /* Aggregated? */
    const char *fwdstate;       /* Current fwdstate */
    mstp_port_status_t core;    /* Core RSTP state (if active) */
} mstp_port_mgmt_status_t;

typedef struct {
    char configname[MSTP_CONFIG_NAME_MAXLEN];
    u16 revision;
    mstp_map_t map;
} mstp_msti_config_t;

vtss_rc mstp_init(vtss_init_data_t *data);

BOOL
mstp_get_system_config(mstp_bridge_param_t *conf);

BOOL
mstp_set_system_config(const mstp_bridge_param_t *conf);

BOOL
mstp_get_msti_config(mstp_msti_config_t *conf, u8 cfg_digest[MSTP_DIGEST_LEN]);

u8
mstp_get_msti_priority(u8 msti);

BOOL
mstp_set_msti_priority(u8 msti, 
                       u8 priority);

BOOL
mstp_set_msti_config(mstp_msti_config_t *conf);

BOOL
mstp_get_port_config(vtss_isid_t isid,
                     vtss_port_no_t port_no,
                     BOOL *enable,
                     mstp_port_param_t *pconf);

BOOL
mstp_set_port_config(vtss_isid_t isid,
                     vtss_port_no_t port_no,
                     BOOL enable,
                     const mstp_port_param_t *pconf);

BOOL
mstp_get_msti_port_config(vtss_isid_t isid,
                          u8 msti, 
                          vtss_port_no_t port_no,
                          mstp_msti_port_param_t *pconf);

BOOL
mstp_set_msti_port_config(vtss_isid_t isid,
                          u8 msti, 
                          vtss_port_no_t port_no,
                          const mstp_msti_port_param_t *pconf);

BOOL
mstp_get_bridge_status(u8 msti, 
                       mstp_bridge_status_t *status);

BOOL
mstp_get_port_status(u8 msti,
                     l2_port_no_t l2port,
                     mstp_port_mgmt_status_t *status);

BOOL
mstp_get_port_vectors(u8 msti,
                      l2_port_no_t l2port,
                      mstp_port_vectors_t *vectors);

BOOL
mstp_get_port_statistics(l2_port_no_t l2port,
                         mstp_port_statistics_t *stats,
                         BOOL clear);

BOOL
mstp_set_port_mcheck(l2_port_no_t l2port);

typedef void (*mstp_trap_sink_t)(u32 eventmask);

/** mstp_register_trap_sink() - (Un)-register SNMP MSTP trap event
 * callback.
 * 
 * \parm cb Trap callback function to register. NULL unregister any
 * previous callback.
 *
 * \return TRUE if (un)-registration succeeded.
 *
 * \note The callback will be called at periodic intervals when an
 * MSTP event has occurred, typically within the last second. The \e
 * eventmask argument will have a bit set for each (1 <<
 * mstp_trap_event_t) having occurred.
 */
BOOL
mstp_register_trap_sink(mstp_trap_sink_t cb);

typedef void (*mstp_config_change_cb_t)(void);

/** mstp_register_config_change_cb() - (Un)-register MSTP
 * configuration change callback.
 * 
 * \parm cb Configuration change callback function to register. NULL
 * unregister any previous callback.
 *
 * \return TRUE if (un)-registration succeeded.
 *
 * \note The callback will be called when the MSTP configuration
 * changes. Only one callback is supported presently.
 */
BOOL
mstp_register_config_change_cb(mstp_config_change_cb_t cb);

const char *msti_name(u8 msti);

char *mstp_mstimap2str(const mstp_msti_config_t *conf, u8 msti, char *buf, size_t bufsize);

#endif // _MSTP_API_H_


// ***************************************************************************
// 
//  End of file.
// 
// ***************************************************************************
