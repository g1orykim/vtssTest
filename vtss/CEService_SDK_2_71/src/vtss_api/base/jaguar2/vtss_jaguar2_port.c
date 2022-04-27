/*

 Vitesse API software.

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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_PORT
#include "vtss_jaguar2_cil.h"

#if defined(VTSS_ARCH_JAGUAR_2)

/* - CIL functions ------------------------------------------------- */

#if defined(VTSS_FEATURE_TIMESTAMP)
static vtss_rc jr2_synce_clock_out_set(vtss_state_t *vtss_state, const u32 clk_port)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_synce_clock_in_set(vtss_state_t *vtss_state, const u32 clk_port)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}
#endif /* VTSS_FEATURE_TIMESTAMP */

/* ================================================================= *
 *  Port control
 * ================================================================= */

vtss_rc vtss_jr2_port_max_tags_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_miim_read_write(vtss_state_t *vtss_state,
                                    BOOL read, 
                                    u32 miim_controller, 
                                    u8 miim_addr, 
                                    u8 addr, 
                                    u16 *value, 
                                    BOOL report_errors)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_miim_read(vtss_state_t *vtss_state,
                              vtss_miim_controller_t miim_controller, 
                              u8 miim_addr, 
                              u8 addr, 
                              u16 *value, 
                              BOOL report_errors)
{
    return jr2_miim_read_write(vtss_state, TRUE, miim_controller, miim_addr, addr, value, report_errors);
}

static vtss_rc jr2_miim_write(vtss_state_t *vtss_state,
                               vtss_miim_controller_t miim_controller, 
                               u8 miim_addr, 
                               u8 addr, 
                               u16 value, 
                               BOOL report_errors)
{
    return jr2_miim_read_write(vtss_state, FALSE, miim_controller, miim_addr, addr, &value, report_errors);
}

static vtss_rc jr2_port_conf_get(vtss_state_t *vtss_state,
                                  const vtss_port_no_t port_no, vtss_port_conf_t *const conf)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

vtss_rc vtss_jr2_port_policer_fc_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_port_conf_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_port_ifh_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_port_clause_37_control_get(vtss_state_t *vtss_state,
                                               const vtss_port_no_t port_no,
                                               vtss_port_clause_37_control_t *const control)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_port_clause_37_control_set(vtss_state_t *vtss_state,
                                               const vtss_port_no_t port_no)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_port_clause_37_status_get(vtss_state_t *vtss_state,
                                              const vtss_port_no_t         port_no, 
                                              vtss_port_clause_37_status_t *const status)
    
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_port_status_get(vtss_state_t *vtss_state,
                                    const vtss_port_no_t  port_no, 
                                    vtss_port_status_t    *const status)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_port_counters_read(vtss_state_t                 *vtss_state,
                                      vtss_port_no_t               port_no,
                                      u32                          port,
                                      vtss_port_jr2_counters_t     *c,
                                      vtss_port_counters_t *const  counters, 
                                      BOOL                         clear)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_port_basic_counters_get(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no,
                                            vtss_basic_counters_t *const counters)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_port_counters_cmd(vtss_state_t                *vtss_state,
                                      const vtss_port_no_t        port_no, 
                                      vtss_port_counters_t *const counters, 
                                      BOOL                        clear)
{
    return jr2_port_counters_read(vtss_state,
                                  port_no,
                                  VTSS_CHIP_PORT(port_no),
                                  &vtss_state->port.counters[port_no].counter.jr2,
                                  counters,
                                  clear);
}

static vtss_rc jr2_port_counters_update(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return jr2_port_counters_cmd(vtss_state, port_no, NULL, 0);
}

static vtss_rc jr2_port_counters_clear(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return jr2_port_counters_cmd(vtss_state, port_no, NULL, 1);
}

static vtss_rc jr2_port_counters_get(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no,
                                      vtss_port_counters_t *const counters)
{
    memset(counters, 0, sizeof(*counters)); 
    return jr2_port_counters_cmd(vtss_state, port_no, counters, 0);
}

static vtss_rc jr2_port_forward_set(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

u32 vtss_jr2_wm_dec(u32 value)
{
    // JR2-TBD: Stub
//    if (value > 2048) {
//        return (value - 2048) * 16;
//    }
    return value;
}

static vtss_rc jr2_port_buf_conf_set(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub: Buffers, watermarks
    return VTSS_RC_ERROR;
}

/* - Debug print --------------------------------------------------- */

blab
static vtss_rc jr2_debug_example(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)

{
     // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

vtss_rc vtss_jr2_port_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    // JR2-TBD: Stub
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_PORT, jr2_debug_example, vtss_state, pr, info));
    return VTSS_RC_ERROR;
}

/* - Initialization ------------------------------------------------ */

vtss_rc vtss_jr2_port_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_port_state_t *state = &vtss_state->port;
    u32               port;

    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->miim_read = jr2_miim_read;
        state->miim_write = jr2_miim_write;
        state->conf_get = jr2_port_conf_get;
        state->conf_set = jr2_port_conf_set;
        state->clause_37_status_get = jr2_port_clause_37_status_get;
        state->clause_37_control_get = jr2_port_clause_37_control_get;
        state->clause_37_control_set = jr2_port_clause_37_control_set;
        state->status_get = jr2_port_status_get;
        state->counters_update = jr2_port_counters_update;
        state->counters_clear = jr2_port_counters_clear;
        state->counters_get = jr2_port_counters_get;
        state->basic_counters_get = jr2_port_basic_counters_get;
        state->ifh_set = jr2_port_ifh_set;
        state->forward_set = jr2_port_forward_set;
        
        /* SYNCE features */
#if defined(VTSS_FEATURE_SYNCE)
        vtss_state->synce.clock_out_set = jr2_synce_clock_out_set;
        vtss_state->synce.clock_in_set = jr2_synce_clock_in_set;
#endif /* VTSS_FEATURE_SYNCE */
        break;

    case VTSS_INIT_CMD_INIT:
        /* Tweak the default PCIe settings */
        // JR2-TBD: Stub
        /* Clear port counters */
        for (port = 0; port <= VTSS_CHIP_PORTS; port++) {
            // JR2-TBD: Stub
        }
        break;

    case VTSS_INIT_CMD_PORT_MAP:
        if (!vtss_state->warm_start_cur) {
            VTSS_RC(jr2_port_buf_conf_set(vtss_state));
        }
        break;

    default:
        break;
    }
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_JAGUAR_2 */

