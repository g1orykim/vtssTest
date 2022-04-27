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

#include "critd_api.h"      /* For semaphore wrapper                                */
#include "psec_limit_api.h" /* To get access to our own structures and enumerations */
#ifdef VTSS_SW_OPTION_VCLI
#include "psec_limit_cli.h" /* For CLI initialization function                      */
#endif
#include "conf_api.h"       /* For flash management                                 */
#include "msg_api.h"        /* For msg_switch_is_master()                           */
#include "main.h"           /* For vtss_xstr()                                      */
#include "cli.h"            /* For iport2uport()                                    */
#ifdef VTSS_SW_OPTION_SYSLOG
#include "syslog_api.h"     /* For S_xxx() macros                                   */
#endif
#if VTSS_SWITCH_STACKABLE
#include "topo_api.h"       /* For topo_isid2usid()                                 */
#endif
#include "psec_limit_trace.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "psec_limit_icli_functions.h" /* For psec_limit_icfg_init()                */
#endif
#ifdef VTSS_SW_OPTION_SNMP
#include <ucd-snmp/config.h>  /* For HAVE_STDLIB_H, etc.                            */
#include <ucd-snmp/mibincl.h> /* Standard set of SNMP includes                      */
#include "snmp_custom_api.h"  /* For snmp_private_mib_trap_send()                   */
#endif

/****************************************************************************/
// Trace definitions
/****************************************************************************/

#if (VTSS_TRACE_ENABLED)
/* Trace registration. Initialized by psec_init() */
static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "psec_limit",
    .descr     = "Port Security Limit Control module"
};

#ifndef PSEC_LIMIT_DEFAULT_TRACE_LVL
#define PSEC_LIMIT_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_WARNING
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default",
        .lvl       = PSEC_LIMIT_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical regions",
        .lvl       = PSEC_LIMIT_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    }, [TRACE_GRP_ICLI] = {
        .name      = "iCLI",
        .descr     = "ICLI",
        .lvl       = PSEC_LIMIT_DEFAULT_TRACE_LVL,
        .timestamp = 1,

    },
};
#endif /* VTSS_TRACE_ENABLED */

/******************************************************************************/
// Semaphore stuff.
/******************************************************************************/
static critd_t PSEC_LIMIT_crit;

// Macros for accessing semaphore functions
// -----------------------------------------
#if VTSS_TRACE_ENABLED
#define PSEC_LIMIT_CRIT_ENTER()         critd_enter(        &PSEC_LIMIT_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PSEC_LIMIT_CRIT_EXIT()          critd_exit(         &PSEC_LIMIT_crit, TRACE_GRP_CRIT, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define PSEC_LIMIT_CRIT_ASSERT_LOCKED() critd_assert_locked(&PSEC_LIMIT_crit, TRACE_GRP_CRIT,                       __FILE__, __LINE__)
#else
// Leave out function and line arguments
#define PSEC_LIMIT_CRIT_ENTER()         critd_enter(        &PSEC_LIMIT_crit)
#define PSEC_LIMIT_CRIT_EXIT()          critd_exit(         &PSEC_LIMIT_crit)
#define PSEC_LIMIT_CRIT_ASSERT_LOCKED() critd_assert_locked(&PSEC_LIMIT_crit)
#endif

// Overall configuration (valid on master only).
typedef struct {
    // One instance of the global configuration
    psec_limit_glbl_cfg_t glbl_cfg;

    // One instance per port in the stack of the per-port configuration.
    // Index 0 corresponds to VTSS_ISID_START. Used by the master
    // for the per-switch configuration.
    psec_limit_switch_cfg_t switch_cfg[VTSS_ISID_CNT];
} psec_limit_stack_cfg_t;

// Overall configuration as saved in flash.
typedef struct {
    // Current version of the configuration in flash.
    u32 version;

    // Overall config
    psec_limit_stack_cfg_t cfg;
} psec_limit_flash_cfg_t;

#define PSEC_LIMIT_FLASH_CFG_VERSION 1

static psec_limit_stack_cfg_t PSEC_LIMIT_stack_cfg;

// Reference counting per port.
// Index 0 holds number of MACs that we told the PSEC module to put in forwarding state.
// Index 1 holds number of MACs that we told the PSEC module to put in blocked state.
static u32 PSEC_LIMIT_ref_cnt[VTSS_ISID_CNT][VTSS_PORTS][2];

// If not debugging, set PSEC_LIMIT_INLINE to inline
#define PSEC_LIMIT_INLINE inline

/******************************************************************************/
//
// Module Private Functions
//
/******************************************************************************/

/******************************************************************************/
// PSEC_LIMIT_master_isid_port_check()
// Returns VTSS_OK if we're master and isid and port are legal.
/******************************************************************************/
static vtss_rc PSEC_LIMIT_master_isid_port_check(vtss_isid_t isid, vtss_port_no_t port, BOOL check_port)
{
    if (!msg_switch_is_master()) {
        return PSEC_LIMIT_ERROR_MUST_BE_MASTER;
    }

    if (!VTSS_ISID_LEGAL(isid)) {
        return PSEC_LIMIT_ERROR_INV_ISID;
    }

    // In case someone changes VTSS_PORT_NO_START back to 1, we better survive that,
    // so tell lint to not report "Relational operator '<' always evaluates to 'false'"
    // and "non-negative quantity is never less than zero".
    /*lint -e{685, 568} */
    if (check_port && (port < VTSS_PORT_NO_START || port >= port_isid_port_count(isid) + VTSS_PORT_NO_START || port_isid_port_no_is_stack(isid, port))) {
        // port is only set to something different from VTSS_PORT_NO_START in case
        // we really need to check that the port exists on a given switch and not a stack port,
        // so it's safe to check against actual number of ports and stack ports, rather than
        // checking against VTSS_PORTS.
        return PSEC_LIMIT_ERROR_INV_PORT;
    }

    return VTSS_OK;
}

/******************************************************************************/
// PSEC_LIMIT_on_mac_add_callback()
// This function will be called by the PSEC module whenever a new MAC address
// is to be added.
// We will return PSEC_ADD_METHOD_FORWARD in all cases but one, since we normally
// don't have an interest in blocking an entry.
// The only case where we ask the PSEC module to block the entry is when
// the limit is exceeded and the action involved is an SNMP trap, but no
// port shut-down. In that case, we disallow the new MAC address for a certain
// hold time, so that we can get another SNMP trap after that hold time.
/******************************************************************************/
static psec_add_method_t PSEC_LIMIT_on_mac_add_callback(vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, u32 mac_cnt_before_callback, psec_add_action_t *action)
{
    vtss_rc               rc;
    psec_limit_port_cfg_t *port_cfg;
    psec_add_method_t     result = PSEC_ADD_METHOD_FORWARD;
    u32                   *fwd_cnt, *blk_cnt;

#ifdef VTSS_SW_OPTION_SYSLOG
    char                  macstr[18];
    char                  prefix_str[100];
    (void)misc_mac_txt(vid_mac->mac.addr, macstr);
#if VTSS_SWITCH_STACKABLE
    sprintf(prefix_str, "Port Security Limit Control (Switch %d Port %d, MAC=%s): ", topo_isid2usid(isid), iport2uport(port), macstr);
#else
    sprintf(prefix_str, "Port Security Limit Control (Port %d, MAC=%s): ", iport2uport(port), macstr);
#endif /* VTSS_SWITCH_STACKABLE */
#endif /* VTSS_SW_OPTION_SYSLOG */

    if ((rc = PSEC_LIMIT_master_isid_port_check(isid, port, TRUE)) != VTSS_OK || action == NULL) {
        if (rc != PSEC_LIMIT_ERROR_MUST_BE_MASTER) {
            T_E("Internal error: Invalid parameter (rc = %d)", rc);
        }
        return PSEC_ADD_METHOD_FORWARD; // There's a bug that must be fixed, so we can return anything.
    }

    PSEC_LIMIT_CRIT_ENTER();

    if (!PSEC_LIMIT_stack_cfg.glbl_cfg.enabled) {
        // We're not globally enabled. Allow it (as seen from our PoV)
        goto do_exit;
    }

    port_cfg = &PSEC_LIMIT_stack_cfg.switch_cfg[isid - VTSS_ISID_START].port_cfg[port - VTSS_PORT_NO_START];
    if (!port_cfg->enabled) {
        // Limit control is not enabled on that port. Allow it.
        goto do_exit;
    }

    // PSEC_LIMIT_ref_cnt[][][] uses zero-based indexing.
    fwd_cnt = &PSEC_LIMIT_ref_cnt[isid - VTSS_ISID_START][port - VTSS_PORT_NO_START][0];
    blk_cnt = &PSEC_LIMIT_ref_cnt[isid - VTSS_ISID_START][port - VTSS_PORT_NO_START][1];

    if (*fwd_cnt + *blk_cnt != mac_cnt_before_callback) {
        T_E("%u:%u: Disagreement between PSEC and PSEC Limit: mac_cnt = %u, fwd = %u, blk = %u", isid, iport2uport(port), mac_cnt_before_callback, *fwd_cnt, *blk_cnt);
    }

    // If action == NONE, then we shouldn't be called if the number of MAC addresses before this
    // one gets added is at or above the limit, since we've turned off CPU copy in previous call.
    // Likewise, if action != NONE, then we shouldn't be called if the number of MAC addresses before
    // this one gets added is above the limit, since we've either stopped CPU copy or shut-down the
    // port in previous call.
    if ((port_cfg->action == PSEC_LIMIT_ACTION_NONE && *fwd_cnt >= port_cfg->limit) ||
        (port_cfg->action != PSEC_LIMIT_ACTION_NONE && *fwd_cnt >  port_cfg->limit)) {
        T_E("%u:%u: Called with invalid mac_cnt (%u, %u, %u). Limit=%u, action=%d", isid, iport2uport(port), mac_cnt_before_callback, *fwd_cnt, *blk_cnt, port_cfg->limit, port_cfg->action);
    }

    if (port_cfg->action == PSEC_LIMIT_ACTION_NONE) {
        if (*fwd_cnt + 1 == port_cfg->limit) {
            // When adding this new MAC address, then the limit will be reached.
            *action = PSEC_ADD_ACTION_LIMIT_REACHED;
#ifdef VTSS_SW_OPTION_SYSLOG
            S_I("%sLimit reached. No action involved.", prefix_str);
#endif
        }
    } else {
        // An action is involved with this port.
        if (*fwd_cnt == port_cfg->limit) {
            // When adding this new MAC address, then the limit will be exceeded.
            // Take proper action.
            if (port_cfg->action == PSEC_LIMIT_ACTION_SHUTDOWN || port_cfg->action == PSEC_LIMIT_ACTION_TRAP_AND_SHUTDOWN) {
                // If the action involves shutting down the port, then do it.
                *action = PSEC_ADD_ACTION_SHUT_DOWN;
#ifdef VTSS_SW_OPTION_SYSLOG
                S_W("%sLimit exceeded. %shutting down the port.", prefix_str, port_cfg->action == PSEC_LIMIT_ACTION_TRAP_AND_SHUTDOWN ? "Sending SNMP trap and s" : "S");
#endif
            } else {
                // Otherwise (we should only send an SNMP trap - not shut-down the port), we simply stop CPU copying
                *action = PSEC_ADD_ACTION_LIMIT_REACHED;
                // And tell the PSEC module to block the entry for hold-time seconds.
                // When this hold-time expires, it may be that we send another SNMP trap,
                // if another MAC address comes in.
                result = PSEC_ADD_METHOD_BLOCK;
#ifdef VTSS_SW_OPTION_SYSLOG
                S_W("%sLimit exceeded. Sending SNMP trap.", prefix_str);
#endif
            }

#ifdef VTSS_SW_OPTION_SNMP
            if (port_cfg->action == PSEC_LIMIT_ACTION_TRAP || port_cfg->action == PSEC_LIMIT_ACTION_TRAP_AND_SHUTDOWN) {
                // Send an SNMP trap.
                snmp_private_mib_trap_send(isid, port, SNMP_PRIVATE_MIB_TRAP_PSEC_LIMIT_EXCEEDED);
            }
#endif
        }
    }

    if (result == PSEC_ADD_METHOD_FORWARD) {
        (*fwd_cnt)++;
    } else {
        (*blk_cnt)++;
    }
    T_I("%u:%u: New ref. count (add_method = %d) = fwd: %u, blk: %u", isid, iport2uport(port), result, *fwd_cnt, *blk_cnt);


do_exit:
    PSEC_LIMIT_CRIT_EXIT();
    return result;
}

/******************************************************************************/
// PSEC_LIMIT_on_mac_del_callback()
/******************************************************************************/
static void PSEC_LIMIT_on_mac_del_callback(vtss_isid_t isid, vtss_port_no_t port, vtss_vid_mac_t *vid_mac, psec_del_reason_t reason, psec_add_method_t add_method)
{
    int idx = add_method == PSEC_ADD_METHOD_FORWARD ? 0 : 1;
    u32 *val, *fwd_cnt, *blk_cnt;

    PSEC_LIMIT_CRIT_ENTER();

    // PSEC_LIMIT_ref_cnt[][][] uses zero-based indexing.
    fwd_cnt = &PSEC_LIMIT_ref_cnt[isid - VTSS_ISID_START][port - VTSS_PORT_NO_START][0];
    blk_cnt = &PSEC_LIMIT_ref_cnt[isid - VTSS_ISID_START][port - VTSS_PORT_NO_START][1];
    val     = &PSEC_LIMIT_ref_cnt[isid - VTSS_ISID_START][port - VTSS_PORT_NO_START][idx];

    if (add_method != PSEC_ADD_METHOD_FORWARD && add_method != PSEC_ADD_METHOD_BLOCK) {
        // Odd to get called with an add_method that this module doesn't support.
        // We can only add with forward or block.
        T_E("%u:%u: Odd add_method %d", isid, iport2uport(port), add_method);
        goto do_exit;
    }

    if (*val == 0) {
        T_E("%u:%u: Reference count for add_method %d is 0", isid, iport2uport(port), add_method);
        goto do_exit;
    }

    (*val)--;
    T_I("%u:%u: New ref. count (add_method = %d) = fwd: %u, blk: %u", isid, iport2uport(port), add_method, *fwd_cnt, *blk_cnt);

do_exit:
    PSEC_LIMIT_CRIT_EXIT();
}

/******************************************************************************/
// PSEC_LIMIT_cfg_valid_glbl()
/******************************************************************************/
static vtss_rc PSEC_LIMIT_cfg_valid_glbl(psec_limit_glbl_cfg_t *cfg)
{
    if (cfg->aging_period_secs < PSEC_LIMIT_AGING_PERIOD_SECS_MIN || cfg->aging_period_secs > PSEC_LIMIT_AGING_PERIOD_SECS_MAX) {
        return PSEC_LIMIT_ERROR_INV_AGING_PERIOD;
    }
    return VTSS_OK;
}

/******************************************************************************/
// PSEC_LIMIT_cfg_valid_port()
/******************************************************************************/
static vtss_rc PSEC_LIMIT_cfg_valid_port(psec_limit_port_cfg_t *cfg)
{
    if (cfg->limit < PSEC_LIMIT_LIMIT_MIN || cfg->limit > PSEC_LIMIT_LIMIT_MAX) {
        return PSEC_LIMIT_ERROR_INV_LIMIT;
    }

    if (cfg->action >= PSEC_LIMIT_ACTION_LAST) {
        return PSEC_LIMIT_ERROR_INV_ACTION;
    }

    return VTSS_OK;
}

/******************************************************************************/
// PSEC_LIMIT_cfg_valid()
/******************************************************************************/
static PSEC_LIMIT_INLINE vtss_rc PSEC_LIMIT_cfg_valid(psec_limit_stack_cfg_t *cfg)
{
    vtss_isid_t    isid;
    vtss_port_no_t port;
    vtss_rc        rc;

    if ((rc = PSEC_LIMIT_cfg_valid_glbl(&cfg->glbl_cfg)) == VTSS_OK) {
        for (isid = 0; isid < VTSS_ISID_CNT; isid++) {
            for (port = 0; port < VTSS_PORTS; port++) {
                if ((rc = PSEC_LIMIT_cfg_valid_port(&cfg->switch_cfg[isid].port_cfg[port])) != VTSS_OK) {
                    T_W("%u:%u", isid, iport2uport(port + VTSS_PORT_NO_START));
                    break;
                }
            }
        }
    }
    if (rc != VTSS_OK) {
        T_W(psec_limit_error_txt(rc));
    }
    return rc;
}

/******************************************************************************/
// PSEC_LIMIT_cfg_default_glbl()
/******************************************************************************/
void PSEC_LIMIT_cfg_default_glbl(psec_limit_glbl_cfg_t *cfg)
{
    memset(cfg, 0, sizeof(psec_limit_glbl_cfg_t));
    cfg->aging_period_secs = PSEC_LIMIT_AGING_PERIOD_SECS_DEFAULT;
}

/******************************************************************************/
// PSEC_LIMIT_cfg_default_switch()
/******************************************************************************/
void PSEC_LIMIT_cfg_default_switch(psec_limit_switch_cfg_t *cfg)
{
    vtss_port_no_t port;
    memset(cfg, 0, sizeof(psec_limit_switch_cfg_t));
    for (port = 0; port < VTSS_PORTS; port++) {
        /*lint -e{506} */ /* Avoid "Constant value Boolean" Lint warning */
        cfg->port_cfg[port].limit = PSEC_LIMIT_LIMIT_DEFAULT;
    }
}

/******************************************************************************/
// PSEC_LIMIT_cfg_default_all()
/******************************************************************************/
static void PSEC_LIMIT_cfg_default_all(psec_limit_stack_cfg_t *cfg)
{
    vtss_isid_t isid;

    PSEC_LIMIT_cfg_default_glbl(&cfg->glbl_cfg);
    for (isid = 0; isid < VTSS_ISID_CNT; isid++) {
        PSEC_LIMIT_cfg_default_switch(&cfg->switch_cfg[isid]);
    }
}

/******************************************************************************/
// PSEC_LIMIT_loop_through_callback()
// In all cases where this function is called back, we delete all existing
// entries. The @\keep and return value are not used by the PSEC module if we're
// disabling ourselves.
/******************************************************************************/
static psec_add_method_t PSEC_LIMIT_loop_through_callback(void                      *user_ctx,
                                                          vtss_isid_t                isid,
                                                          vtss_port_no_t             port,
                                                          vtss_vid_mac_t             *vid_mac,
                                                          u32                        mac_cnt_before_callback,
                                                          BOOL                       *keep,
                                                          psec_loop_through_action_t *action)
{
    // Remove this entry.
    *keep = FALSE;
    // Return value doesn't matter when we remove it.
    return PSEC_ADD_METHOD_FORWARD;
}

/******************************************************************************/
// PSEC_LIMIT_apply_cfg()
/******************************************************************************/
static vtss_rc PSEC_LIMIT_apply_cfg(vtss_isid_t isid, psec_limit_stack_cfg_t *new_cfg, BOOL switch_cfg_may_be_changed)
{
    vtss_isid_t            isid_start, isid_end;
    vtss_port_no_t         port;
    vtss_rc                rc;
    psec_limit_stack_cfg_t *old_cfg = &PSEC_LIMIT_stack_cfg;

    PSEC_LIMIT_CRIT_ASSERT_LOCKED();

    // Change the age- and hold-times if requested to.
    if (isid == VTSS_ISID_GLOBAL) {
        if ((rc = psec_mgmt_time_cfg_set(PSEC_USER_PSEC_LIMIT, new_cfg->glbl_cfg.enable_aging ? new_cfg->glbl_cfg.aging_period_secs : 0, PSEC_LIMIT_HOLD_TIME_SECS)) != VTSS_OK) {
            return rc;
        }
    }

    // In the following, we use zero-based port- and isid-counters.
    if ((switch_cfg_may_be_changed && isid == VTSS_ISID_GLOBAL) || (old_cfg->glbl_cfg.enabled != new_cfg->glbl_cfg.enabled)) {
        // If glbl_cfg.enable has changed, then we must apply the enable thing to all ports.
        isid_start = 0;
        isid_end   = VTSS_ISID_CNT - 1;
    } else if (switch_cfg_may_be_changed && isid != VTSS_ISID_GLOBAL) {
        isid_start = isid_end = isid - VTSS_ISID_START;
    } else {
        // Nothing more to do.
        return VTSS_OK;
    }

    for (isid = isid_start; isid <= isid_end; isid++) {
        psec_limit_switch_cfg_t *old_switch_cfg = &old_cfg->switch_cfg[isid];
        psec_limit_switch_cfg_t *new_switch_cfg = &new_cfg->switch_cfg[isid];
        for (port = 0; port < VTSS_PORTS; port++) {
            psec_limit_port_cfg_t *old_port_cfg = &old_switch_cfg->port_cfg[port];
            psec_limit_port_cfg_t *new_port_cfg = &new_switch_cfg->port_cfg[port];
            BOOL old_was_enabled = old_port_cfg->enabled && old_cfg->glbl_cfg.enabled;
            BOOL new_is_enabled  = new_port_cfg->enabled && new_cfg->glbl_cfg.enabled;
            BOOL call_ena_func   = FALSE;
            BOOL reopen_port     = FALSE;

            if (!old_was_enabled) {
                if (new_is_enabled) {
                    // Old was not enabled, but we're going to enable.
                    // Remove all entries from the port.
                    call_ena_func = TRUE;
                    reopen_port   = FALSE;
                }
            } else {
                if (!new_is_enabled) {
                    // We're going from enabled to disabled.
                    // Delete limits and shutdown properties on the port.
                    call_ena_func = TRUE;
                    reopen_port   = TRUE;
                } else {
                    // Old was enabled and new is enabled.
                    // According to DS, we should remove all entries on the port if the limit or the action changes
                    // (a bit silly I think, but they get what they want).
                    if (new_port_cfg->limit != old_port_cfg->limit || new_port_cfg->action != old_port_cfg->action) {
                        call_ena_func = TRUE;
                        reopen_port   = TRUE;
                    }
                }
            }

            if (call_ena_func) {
                PSEC_LIMIT_ref_cnt[isid][port][0] = 0;
                PSEC_LIMIT_ref_cnt[isid][port][1] = 0;
                if ((rc = psec_mgmt_port_cfg_set(PSEC_USER_PSEC_LIMIT, NULL, isid + VTSS_ISID_START, port + VTSS_PORT_NO_START, new_is_enabled, reopen_port, PSEC_LIMIT_loop_through_callback, PSEC_PORT_MODE_NORMAL)) != VTSS_OK) {
                    return rc;
                }
            }
        }
    }
    return VTSS_OK;
}

/******************************************************************************/
// PSEC_LIMIT_cfg_flash_write()
/******************************************************************************/
static void PSEC_LIMIT_cfg_flash_write(void)
{
#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    psec_limit_flash_cfg_t *flash_cfg;

    PSEC_LIMIT_CRIT_ASSERT_LOCKED();
    T_D("Enter");

    if ((flash_cfg = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PSEC_LIMIT_CONF, NULL)) == NULL) {
        T_W("Failed to open flash configuration");
    } else {
        flash_cfg->cfg = PSEC_LIMIT_stack_cfg;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_PSEC_LIMIT_CONF);
    }
#else
    T_N("Silent-upgrade build: Not saving to conf");
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/******************************************************************************/
// PSEC_LIMIT_cfg_flash_read()
// Read/create and activate configuration
//
// If @isid_add == [VTSS_ISID_START; VTSS_ISID_END[ then
//   If reading flash fails then /* Called from either INIT_CMD_CONF_DEF or INIT_CMD_MASTER_UP */
//     create defaults for all switches, including global settings. Only transfer to slave
//     switches if @create_defaults is TRUE, since a master-up event shouldn't cause a message Tx
//   else if @create_defaults == TRUE then /* Called from INIT_CMD_CONF_DEF */
//     create defaults for @isid_add, only and transfer message to @isid_add switch if necessary.
//   else
//     copy flash configuration to RAM cfg for @isid_add, only. Don't Tx messages.
// else /* @isid_add == VTSS_ISID_GLOBAL */
//   if reading flash fails then /* Called from either INIT_CMD_CONF_DEF or INIT_CMD_MASTER_UP */
//     create defaults for all switches, including global settings. Only transfer to slave
//     switches if @create_defaults is TRUE, since a master-up event shouldn't cause a message Tx.
//   else if @create_defaults is TRUE then /* Called from INIT_CMD_CONF_DEF */
//     only create global configuration defaults, and transfer messages to all slave switches if changed.
//   else /* Called from INIT_CMD_MASTER_UP */
//     special case. Copy *both* global and per-switch configuration to RAM cfg.
//     Don't Tx messages to any switches.
/******************************************************************************/
static void PSEC_LIMIT_cfg_flash_read(vtss_isid_t isid_add, BOOL create_defaults)
{
    psec_limit_flash_cfg_t *flash_cfg;
    psec_limit_stack_cfg_t tmp_stack_cfg;
    BOOL                   flash_read_failed = FALSE;
    BOOL                   switch_cfg_may_be_changed;
    ulong                  size;

    T_D("enter, isid: %d, create_defaults = %d", isid_add, create_defaults);

    if (misc_conf_read_use()) {
        // Open or create configuration block
        if ((flash_cfg = conf_sec_open(CONF_SEC_GLOBAL, CONF_BLK_PSEC_LIMIT_CONF, &size)) == NULL ||
            size != sizeof(psec_limit_flash_cfg_t)) {
            T_W("conf_sec_open() failed or size mismatch, creating defaults");
            flash_cfg = conf_sec_create(CONF_SEC_GLOBAL, CONF_BLK_PSEC_LIMIT_CONF, sizeof(psec_limit_flash_cfg_t));
            flash_read_failed = TRUE;
        } else if (flash_cfg->version != PSEC_LIMIT_FLASH_CFG_VERSION) {
            T_W("Version mismatch, creating defaults");
            flash_read_failed = TRUE;
        }
    } else {
        flash_cfg         = NULL;
        flash_read_failed = TRUE;
    }

    PSEC_LIMIT_CRIT_ENTER();
    // Get the current settings into tmp_stack_cfg, so that we can compare changes later on.
    tmp_stack_cfg = PSEC_LIMIT_stack_cfg;

    if (flash_read_failed || (flash_cfg && (PSEC_LIMIT_cfg_valid(&flash_cfg->cfg) != VTSS_OK))) {
        // We need to create new defaults for both local and global settings if flash read failed
        // or the configuration read from flash wasn't invalid.
        PSEC_LIMIT_cfg_default_all(&tmp_stack_cfg);
        isid_add = VTSS_ISID_GLOBAL;
        switch_cfg_may_be_changed = TRUE;
    } else if (create_defaults) {
        // We could read the flash, but were asked to default either global or per-switch settings.
        if (isid_add == VTSS_ISID_GLOBAL) {
            // Default the global settings.
            PSEC_LIMIT_cfg_default_glbl(&tmp_stack_cfg.glbl_cfg);
            switch_cfg_may_be_changed = FALSE;
        } else {
            // Default per-switch settings.
            PSEC_LIMIT_cfg_default_switch(&tmp_stack_cfg.switch_cfg[isid_add - VTSS_ISID_START]);
            switch_cfg_may_be_changed = TRUE;
        }
    } else {
        // Flash read succeeded, the contents is valid, and we're not forced to create defaults.
        isid_add = VTSS_ISID_GLOBAL;
        if (flash_cfg != NULL) {        // Quiet lint
            tmp_stack_cfg = flash_cfg->cfg;
            memcpy(&tmp_stack_cfg, &flash_cfg->cfg, sizeof(psec_limit_stack_cfg_t));
        }
        switch_cfg_may_be_changed = TRUE;
    }

    // Apply the new configuration
    (void)PSEC_LIMIT_apply_cfg(isid_add, &tmp_stack_cfg, switch_cfg_may_be_changed);

    // Copy our temporary settings to the real settings.
    PSEC_LIMIT_stack_cfg = tmp_stack_cfg;

    PSEC_LIMIT_CRIT_EXIT();

#ifndef VTSS_SW_OPTION_SILENT_UPGRADE
    // Write our settings back to flash
    if (flash_cfg) {
        flash_cfg->version = PSEC_LIMIT_FLASH_CFG_VERSION;
        flash_cfg->cfg = tmp_stack_cfg;
        conf_sec_close(CONF_SEC_GLOBAL, CONF_BLK_PSEC_LIMIT_CONF);
    }
#endif /* VTSS_SW_OPTION_SILENT_UPGRADE */
}

/******************************************************************************/
//
// Module Public Functions
//
/******************************************************************************/

/******************************************************************************/
// psec_limit_mgmt_glbl_cfg_get()
/******************************************************************************/
vtss_rc psec_limit_mgmt_glbl_cfg_get(psec_limit_glbl_cfg_t *glbl_cfg)
{
    vtss_rc rc;

    if (!glbl_cfg) {
        return PSEC_LIMIT_ERROR_INV_PARAM;
    }

    if ((rc = PSEC_LIMIT_master_isid_port_check(VTSS_ISID_START, VTSS_PORT_NO_START, FALSE)) != VTSS_OK) {
        return rc;
    }

    PSEC_LIMIT_CRIT_ENTER();
    *glbl_cfg = PSEC_LIMIT_stack_cfg.glbl_cfg;
    PSEC_LIMIT_CRIT_EXIT();
    return VTSS_OK;
}

/******************************************************************************/
// psec_limit_mgmt_glbl_cfg_set()
/******************************************************************************/
vtss_rc psec_limit_mgmt_glbl_cfg_set(psec_limit_glbl_cfg_t *glbl_cfg)
{
    vtss_rc                rc;
    psec_limit_stack_cfg_t tmp_stack_cfg;

    if (!glbl_cfg) {
        return PSEC_LIMIT_ERROR_INV_PARAM;
    }

    if ((rc = PSEC_LIMIT_master_isid_port_check(VTSS_ISID_START, VTSS_PORT_NO_START, FALSE)) != VTSS_OK) {
        return rc;
    }

    if ((rc = PSEC_LIMIT_cfg_valid_glbl(glbl_cfg)) != VTSS_OK) {
        return rc;
    }

    PSEC_LIMIT_CRIT_ENTER();

    // We need to create a new structure with the current config
    // and only replace the glbl_cfg member.
    tmp_stack_cfg          = PSEC_LIMIT_stack_cfg;
    tmp_stack_cfg.glbl_cfg = *glbl_cfg;

    // Apply the configuration to the PSEC module. The function
    // will check differences between old and new config
    if ((rc = PSEC_LIMIT_apply_cfg(VTSS_ISID_GLOBAL, &tmp_stack_cfg, FALSE)) == VTSS_OK) {
        // Copy the user's configuration to our configuration
        PSEC_LIMIT_stack_cfg.glbl_cfg = *glbl_cfg;

        // Save the configuration to flash
        PSEC_LIMIT_cfg_flash_write();
    } else {
        // Roll back to previous settings without checking the return code
        (void)PSEC_LIMIT_apply_cfg(VTSS_ISID_GLOBAL, &PSEC_LIMIT_stack_cfg, FALSE);
    }

    PSEC_LIMIT_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// psec_limit_mgmt_switch_cfg_get()
/******************************************************************************/
vtss_rc psec_limit_mgmt_switch_cfg_get(vtss_isid_t isid, psec_limit_switch_cfg_t *switch_cfg)
{
    vtss_rc rc;

    if (!switch_cfg) {
        return PSEC_LIMIT_ERROR_INV_PARAM;
    }

    if ((rc = PSEC_LIMIT_master_isid_port_check(isid, VTSS_PORT_NO_START, FALSE)) != VTSS_OK) {
        return rc;
    }

    PSEC_LIMIT_CRIT_ENTER();
    *switch_cfg = PSEC_LIMIT_stack_cfg.switch_cfg[isid - VTSS_ISID_START];
    PSEC_LIMIT_CRIT_EXIT();
    return VTSS_OK;
}

/******************************************************************************/
// psec_limit_mgmt_switch_cfg_set()
/******************************************************************************/
vtss_rc psec_limit_mgmt_switch_cfg_set(vtss_isid_t isid, psec_limit_switch_cfg_t *switch_cfg)
{
    vtss_rc                rc;
    vtss_port_no_t         port;
    psec_limit_stack_cfg_t tmp_stack_cfg;

    if (!switch_cfg) {
        return PSEC_LIMIT_ERROR_INV_PARAM;
    }

    if ((rc = PSEC_LIMIT_master_isid_port_check(isid, VTSS_PORT_NO_START, FALSE)) != VTSS_OK) {
        return rc;
    }

    PSEC_LIMIT_CRIT_ENTER();

    for (port = 0; port < VTSS_PORTS; port++) {
        if ((rc = PSEC_LIMIT_cfg_valid_port(&switch_cfg->port_cfg[port])) != VTSS_OK) {
            goto do_exit;
        }
    }

    // We need to create a new structure with the current config
    // and only replace this switch's member.
    tmp_stack_cfg = PSEC_LIMIT_stack_cfg;
    tmp_stack_cfg.switch_cfg[isid - VTSS_ISID_START] = *switch_cfg;

    // Apply the configuration to the PSEC module. The function
    // will check differences between old and new config
    if ((rc = PSEC_LIMIT_apply_cfg(isid, &tmp_stack_cfg, TRUE)) == VTSS_OK) {
        // Copy the user's configuration to our configuration
        PSEC_LIMIT_stack_cfg.switch_cfg[isid - VTSS_ISID_START] = *switch_cfg;

        // Save the configuration to flash
        PSEC_LIMIT_cfg_flash_write();
    } else {
        // Roll back to previous settings without checking the return code
        (void)PSEC_LIMIT_apply_cfg(VTSS_ISID_GLOBAL, &PSEC_LIMIT_stack_cfg, TRUE);
    }

do_exit:
    PSEC_LIMIT_CRIT_EXIT();
    return rc;
}

/******************************************************************************/
// psec_limit_error_txt()
/******************************************************************************/
char *psec_limit_error_txt(vtss_rc rc)
{
    switch (rc) {
    case PSEC_LIMIT_ERROR_INV_PARAM:
        return "Invalid parameter";

    case PSEC_LIMIT_ERROR_MUST_BE_MASTER:
        return "Operation only valid on master switch";

    case PSEC_LIMIT_ERROR_INV_ISID:
        return "Invalid Switch ID";

    case PSEC_LIMIT_ERROR_INV_PORT:
        return "Invalid port number";

    case PSEC_LIMIT_ERROR_INV_AGING_PERIOD:
        // Not nice to use specific values in this string, but much easier than constructing a constant string dynamically.
        return "Aging period out of bounds (valid values are 0 or [" vtss_xstr(PSEC_LIMIT_AGING_PERIOD_SECS_MIN) "; " vtss_xstr(PSEC_LIMIT_AGING_PERIOD_SECS_MAX) "] seconds)";

    case PSEC_LIMIT_ERROR_INV_LIMIT:
        return "Invalid limit (valid range is [" vtss_xstr(PSEC_LIMIT_LIMIT_MIN) "; " vtss_xstr(PSEC_LIMIT_LIMIT_MAX) "])";

    case PSEC_LIMIT_ERROR_INV_ACTION:
        return "The action taken when limit is reached is out of bounds";

    case PSEC_LIMIT_ERROR_STATIC_AGGR_ENABLED:
        return "Limit control cannot be enabled for ports that are enabled for static aggregation";

    case PSEC_LIMIT_ERROR_DYNAMIC_AGGR_ENABLED:
        return "Limit control cannot be enabled for ports that are enabled for LACP";

    default:
        return "Port Security Limit Control: Unknown error code";
    }
}

/******************************************************************************/
// psec_limit_init()
/******************************************************************************/
vtss_rc psec_limit_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
    vtss_rc     rc;

    switch (data->cmd) {
    case INIT_CMD_INIT:
        // Initialize and register trace resources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);

#ifdef VTSS_SW_OPTION_VCLI
        // Initialize our CLI stuff.
        psec_limit_cli_init();
#endif
        // Initialize sempahores.
        critd_init(&PSEC_LIMIT_crit, "crit_psec_limit", VTSS_MODULE_ID_PSEC_LIMIT, VTSS_TRACE_MODULE_ID, CRITD_TYPE_MUTEX);
        // When created, the semaphore was initially locked.
        PSEC_LIMIT_CRIT_EXIT();
        break;

    case INIT_CMD_START:
        // We must also install callback handlers in the psec module. We will then be called
        // whenever the psec module adds MAC addresses to the MAC table. We don't care
        // when the psec module deletes a MAC address from the MAC table (if the port limit is
        // currently reached, the PSEC module will autonomously clear the reached limit flag
        // when deleting a MAC address).
        // Do this as soon as possible in the boot process.
        if ((rc = psec_mgmt_register_callbacks(PSEC_USER_PSEC_LIMIT, PSEC_LIMIT_on_mac_add_callback, PSEC_LIMIT_on_mac_del_callback)) != VTSS_OK) {
            T_E("Unable to register callbacks (%s)", error_txt(rc));
        }

#ifdef VTSS_SW_OPTION_ICFG
        VTSS_RC(psec_limit_icfg_init()); // iCFG initialization (Show running)
#endif
        break;

    case INIT_CMD_CONF_DEF:
        if (isid == VTSS_ISID_LOCAL) {
            // Reset local configuration
            // No such configuration for this module
        } else if (VTSS_ISID_LEGAL(isid) || isid == VTSS_ISID_GLOBAL) {
            // Reset switch or stack configuration
            PSEC_LIMIT_cfg_flash_read(isid, TRUE);
        }
        break;

    case INIT_CMD_MASTER_UP:
        // Reset stack configuration (needed to find differences
        // between default configuration and the configuration
        // loaded from flash)
        PSEC_LIMIT_CRIT_ENTER();
        PSEC_LIMIT_cfg_default_all(&PSEC_LIMIT_stack_cfg);
        PSEC_LIMIT_CRIT_EXIT();

        // Read and apply all settings from flash
        PSEC_LIMIT_cfg_flash_read(VTSS_ISID_GLOBAL, FALSE);
        break;

    case INIT_CMD_MASTER_DOWN:
        break;

    case INIT_CMD_SWITCH_ADD:
        break;

    case INIT_CMD_SWITCH_DEL:
        break;

    default:
        break;
    }

    return VTSS_OK;
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
