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
#ifdef VTSS_SW_OPTION_PSEC_LIMIT

#include "icli_api.h"
#include "icli_porting_util.h"

#include "cli.h" // For cli_port_iter_init

#include "psec_limit_api.h"
#include "psec_limit_trace.h"

#include "msg_api.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif

/***************************************************************************/
/*  Type defines                                                           */
/***************************************************************************/
// Enum for selecting which status or configuration to access
typedef enum {
    REOPEN,     // Reopen shut downed ports (no shutdown)
    ENABLE,     // Enable port security
    MAXIMUM,    // Configure max. number of MAC addresses that can be learned on this set of ports
    VIOLATION,  // Configure the action involved with exceeding the limit.
    AGING_TIME, // Configure aging time
    AGING,      // Configure aging
    GLBL_ENABLE // Configure global enable
} psec_limit_cmd_type_t;


// Used to passed the configuration to be changed.or status information to display
typedef struct {
    psec_limit_cmd_type_t type;          // Which configuration to access

    BOOL no; // "invert" the command (User no command)

    // New values
    union {
        u32 limit;
        psec_limit_action_t action;
        u32 aging_period_secs;
    } value;
} psec_limit_cmd_t;

/***************************************************************************/
/*  Internal functions                                                     */
/****************************************************************************/
// Function for configuring global configuration
// In : session_id - For printing
//      cmd        - Containing information about which function to call.
static vtss_rc psec_limit_icli_glbl_conf(i32 session_id, const psec_limit_cmd_t *cmd)
{
    psec_limit_glbl_cfg_t   glbl_cfg;
    psec_limit_glbl_cfg_t   glbl_cfg_default;

    // Get current configuration
    VTSS_RC(psec_limit_mgmt_glbl_cfg_get(&glbl_cfg));

    // Get default configuration values.
    PSEC_LIMIT_cfg_default_glbl(&glbl_cfg_default);


    switch (cmd->type) {
    case AGING_TIME:
        if (cmd->no) {
            glbl_cfg.aging_period_secs = glbl_cfg_default.aging_period_secs;
        } else {
            glbl_cfg.aging_period_secs = cmd->value.aging_period_secs;
        }
        break;

    case AGING:
        glbl_cfg.enable_aging = !cmd->no;
        break;

    case GLBL_ENABLE:
        glbl_cfg.enabled = !cmd->no;
        break;

    default:
        T_E("Un-expected type:%d", cmd->type);
        break;
    }

    // Set new configuration
    VTSS_RC(psec_limit_mgmt_glbl_cfg_set(&glbl_cfg));

    return VTSS_RC_OK;
}

// Function for looping over all switches and all ports a the port list, and the calling a configuration or status/statistics function.
// In : session_id - For printing
//      plist      - Containing information about which switches and ports to "access"
//      cmd        - Containing information about which function to call.
static vtss_rc psec_limit_icli_sit_port_loop(const i32 session_id, icli_stack_port_range_t *plist, const psec_limit_cmd_t *cmd)
{
    switch_iter_t           sit;
    psec_limit_switch_cfg_t switch_cfg;
    psec_limit_switch_cfg_t switch_cfg_default;

    // Get default configuration values.
    PSEC_LIMIT_cfg_default_switch(&switch_cfg_default);

    // Loop through all switches in a stack.
    // For all commands, but reopen, the switch needs to be configurable.
    // For reopen, it must exist.
    if (cmd->type == REOPEN) {
        // Must exist
        VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID));
    } else {
        // Must be configurable
        VTSS_RC(icli_switch_iter_init(&sit));
    }
    while (icli_switch_iter_getnext(&sit, plist)) {
        // Get current configuration
        VTSS_RC(psec_limit_mgmt_switch_cfg_get(sit.isid, &switch_cfg));

        // Loop through all ports
        port_iter_t pit;
        VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_NPI ));
        while (icli_port_iter_getnext(&pit, plist)) {
            switch (cmd->type) {
            case REOPEN:
                VTSS_RC(psec_mgmt_reopen_port(PSEC_USER_PSEC_LIMIT, sit.isid, pit.iport));
                return VTSS_RC_OK;
            case ENABLE:
                switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].enabled = !cmd->no;
                break;
            case MAXIMUM:
                if (cmd->no) {
                    switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].limit = switch_cfg_default.port_cfg[pit.iport - VTSS_PORT_NO_START].limit;
                } else {
                    switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].limit = cmd->value.limit;
                }
                break;
            case VIOLATION:
                if (cmd->no) {
                    switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].action = switch_cfg_default.port_cfg[pit.iport - VTSS_PORT_NO_START].action;
                } else {
                    switch_cfg.port_cfg[pit.iport - VTSS_PORT_NO_START].action = cmd->value.action;
                }
                break;
            default:
                T_E("Unexpected type: %d", cmd->type);
                break;
            }
        }

        VTSS_RC(psec_limit_mgmt_switch_cfg_set(sit.isid, &switch_cfg));
    }
    return VTSS_RC_OK;
}

/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/
// See psec_limit_icli_functions.h
vtss_rc psec_limit_icli_no_shutdown(i32 session_id, icli_stack_port_range_t *plist)
{
    psec_limit_cmd_t cmd;
    cmd.type = REOPEN;
    VTSS_ICLI_ERR_PRINT(psec_limit_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

// See psec_limit_icli_functions.h
vtss_rc psec_limit_icli_enable(i32 session_id, icli_stack_port_range_t *plist, BOOL no)
{
    psec_limit_cmd_t cmd;
    cmd.type = ENABLE;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(psec_limit_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

// See psec_limit_icli_functions.h
vtss_rc psec_limit_icli_maximum(i32 session_id, icli_stack_port_range_t *plist, u32 v_1_to_1024, BOOL no)
{
    psec_limit_cmd_t cmd;
    cmd.type = MAXIMUM;
    cmd.no = no;
    cmd.value.limit = v_1_to_1024;
    VTSS_ICLI_ERR_PRINT(psec_limit_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

// See psec_limit_icli_functions.h
vtss_rc psec_limit_icli_violation(i32 session_id, BOOL has_protect, BOOL has_trap, BOOL has_trap_shut, BOOL has_shutdown, icli_stack_port_range_t *plist, BOOL no)
{
    psec_limit_cmd_t cmd;
    cmd.type = VIOLATION;
    cmd.value.action = has_protect ? PSEC_LIMIT_ACTION_NONE :
                       has_trap ? PSEC_LIMIT_ACTION_TRAP :
                       has_trap_shut ? PSEC_LIMIT_ACTION_TRAP_AND_SHUTDOWN :
                       PSEC_LIMIT_ACTION_SHUTDOWN;

    cmd.no = no;
    T_IG(TRACE_GRP_ICLI, "Action:%d, no:%d, has_trap:%d, has_trap_shut:%d, has_shutdown:%d", cmd.value.action, cmd.no, has_trap, has_trap_shut, has_shutdown);
    VTSS_ICLI_ERR_PRINT(psec_limit_icli_sit_port_loop(session_id, plist, &cmd));
    return VTSS_RC_OK;
}

// See psec_limit_icli_functions.h
vtss_rc psec_limit_icli_enable_global(i32 session_id, BOOL no)
{
    psec_limit_cmd_t cmd;
    cmd.type = GLBL_ENABLE;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(psec_limit_icli_glbl_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

// See psec_limit_icli_functions.h
vtss_rc psec_limit_icli_aging_time(i32 session_id, u32 value, BOOL no)
{
    psec_limit_cmd_t cmd;
    cmd.type = AGING_TIME;
    cmd.value.aging_period_secs = value;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(psec_limit_icli_glbl_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

// See psec_limit_icli_functions.h
vtss_rc psec_limit_icli_aging(i32 session_id, BOOL no)
{
    psec_limit_cmd_t cmd;
    cmd.type = AGING;
    cmd.no = no;
    VTSS_ICLI_ERR_PRINT(psec_limit_icli_glbl_conf(session_id, &cmd));
    return VTSS_RC_OK;
}

/***************************************************************************/
/* ICFG callback functions */
/****************************************************************************/
#ifdef VTSS_SW_OPTION_ICFG
static vtss_rc psec_limit_icfg_conf(const vtss_icfg_query_request_t *req,
                                    vtss_icfg_query_result_t *result)
{
    vtss_icfg_conf_print_t conf_print;

    vtss_icfg_conf_print_init(&conf_print);

    switch (req->cmd_mode) {
    case ICLI_CMD_MODE_GLOBAL_CONFIG: {
        psec_limit_glbl_cfg_t glbl_cfg;
        psec_limit_glbl_cfg_t glbl_cfg_default;

        // Get current configuration
        VTSS_RC(psec_limit_mgmt_glbl_cfg_get(&glbl_cfg));

        // Get default configuration values.
        PSEC_LIMIT_cfg_default_glbl(&glbl_cfg_default);

        // Aging Enable
        conf_print.is_default = glbl_cfg.enable_aging == glbl_cfg_default.enable_aging;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "port-security aging", ""));

        // Aging time
        conf_print.is_default = glbl_cfg.aging_period_secs == glbl_cfg_default.aging_period_secs;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "port-security aging time", "%d", glbl_cfg.aging_period_secs));

        // Port security enable/disabled
        conf_print.is_default = glbl_cfg.enabled == glbl_cfg_default.enabled;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "port-security", ""));
    }
    break;

    case ICLI_CMD_MODE_INTERFACE_PORT_LIST: {
        vtss_isid_t isid = req->instance_id.port.isid;
        vtss_port_no_t iport = req->instance_id.port.begin_iport;
        psec_limit_switch_cfg_t switch_cfg, switch_cfg_default;

        // Get current configuration
        VTSS_RC(psec_limit_mgmt_switch_cfg_get(isid, &switch_cfg));

        // get default configuration
        PSEC_LIMIT_cfg_default_switch(&switch_cfg_default);

        // Enable
        conf_print.is_default = switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].enabled == switch_cfg_default.port_cfg[iport - VTSS_PORT_NO_START].enabled;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "port-security", ""));

        // Limit
        conf_print.is_default = switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].limit == switch_cfg_default.port_cfg[iport - VTSS_PORT_NO_START].limit;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "port-security maximum", "%d", switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].limit));

        // Action
        psec_limit_action_t action = switch_cfg.port_cfg[iport - VTSS_PORT_NO_START].action;
        conf_print.is_default = action == switch_cfg_default.port_cfg[iport - VTSS_PORT_NO_START].action;
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "port-security violation", "%s", action == PSEC_LIMIT_ACTION_NONE ? "protect" :
                                     action == PSEC_LIMIT_ACTION_TRAP ? "trap" :
                                     action == PSEC_LIMIT_ACTION_SHUTDOWN ? "shutdown" :
                                     action == PSEC_LIMIT_ACTION_TRAP_AND_SHUTDOWN ? "trap-shutdown" :
                                     "Unknown action"));
    }
    break;

    default:
        // Not needed
        break;
    }

    return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc psec_limit_icfg_init(void)
{
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_PSEC_LIMIT_GLOBAL_CONF, "port-security", psec_limit_icfg_conf));
    VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_PSEC_LIMIT_INTERFACE_CONF, "port-security", psec_limit_icfg_conf));
    return VTSS_RC_OK;
}

#endif //VTSS_SW_OPTION_ICFG
#endif // #ifdef VTSS_SW_OPTION_PSEC_LIMIT
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
