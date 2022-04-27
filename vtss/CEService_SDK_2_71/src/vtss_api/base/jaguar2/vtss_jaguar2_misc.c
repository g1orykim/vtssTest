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

#include "vtss_jaguar2_cil.h"

#if defined(VTSS_ARCH_JAGUAR_2)

// Avoid Lint Warning 572: Excessive shift value (precision 1 shifted right by 2), which occurs
// in this file because (t) - VTSS_IO_ORIGIN1_OFFSET == 0 for t = VTSS_TO_CFG (i.e. ICPU_CFG), and 0 >> 2 gives a lint warning.
/*lint --e{572} */

/* - CIL functions ------------------------------------------------- */


#if defined(VTSS_FEATURE_EEE)
/* =================================================================
 *  EEE - Energy Efficient Ethernet
 * =================================================================*/
static vtss_rc jr2_eee_port_conf_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t       port_no, 
                                      const vtss_eee_port_conf_t *const conf)
{
    // JR2-TBD: Stub
}
#endif /* VTSS_FEATURE_EEE */

#if defined(VTSS_FEATURE_FAN)
/* =================================================================
 * FAN speed control
 * =================================================================*/
static vtss_rc jr2_fan_controller_init(vtss_state_t *vtss_state,
                                        const vtss_fan_conf_t *const spec)
{
    // JR2-TBD: Stub
}

static vtss_rc jr2_fan_cool_lvl_set(vtss_state_t *vtss_state, u8 lvl)
{
    // JR2-TBD: Stub
}

static vtss_rc jr2_fan_cool_lvl_get(vtss_state_t *vtss_state, u8 *duty_cycle)
{
    // JR2-TBD: Stub
}

static vtss_rc jr2_fan_rotation_get(vtss_state_t *vtss_state, vtss_fan_conf_t *fan_spec, u32 *rotation_count)
{
    // JR2-TBD: Stub
}
#endif /* VTSS_FEATURE_FAN */


/* ================================================================= *
 *  Miscellaneous
 * ================================================================= */

static vtss_rc jr2_reg_read(vtss_state_t *vtss_state,
                             const vtss_chip_no_t chip_no, const u32 addr, u32 * const value)
{
    return vtss_jr2_rd(vtss_state, addr, value);
}

static vtss_rc jr2_reg_write(vtss_state_t *vtss_state,
                              const vtss_chip_no_t chip_no, const u32 addr, const u32 value)
{
    return vtss_jr2_wr(vtss_state, addr, value);
}

vtss_rc vtss_jr2_chip_id_get(vtss_state_t *vtss_state, vtss_chip_id_t *const chip_id)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_ptp_event_poll(vtss_state_t *vtss_state, vtss_ptp_event_type_t *ev_mask)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_ptp_event_enable(vtss_state_t *vtss_state,
                                     vtss_ptp_event_type_t ev_mask, BOOL enable)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_intr_cfg(vtss_state_t *vtss_state,
                             const u32  intr_mask, const BOOL polarity, const BOOL enable)
{
    // JR2-TBD: Stub
    return VTSS_RC_OK;
}

static vtss_rc jr2_intr_pol_negation(vtss_state_t *vtss_state)
{
    // JR2-TBD: Stub
    return VTSS_RC_OK;
}

#ifdef VTSS_FEATURE_IRQ_CONTROL
static vtss_rc jr2_misc_irq_cfg(struct vtss_state_s *vtss_state,
                                 const vtss_irq_t irq,
                                 const vtss_irq_conf_t *const conf)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_misc_irq_status(struct vtss_state_s *vtss_state,
                                    vtss_irq_status_t *status)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_misc_irq_enable(struct vtss_state_s *vtss_state,
                                    const vtss_irq_t irq,
                                    const BOOL enable)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}
#endif  /* VTSS_FEATURE_IRQ_CONTROL */

static vtss_rc jr2_poll_1sec(vtss_state_t *vtss_state)
{
    /* Poll function groups */
    return vtss_jr2_init_groups(vtss_state, VTSS_INIT_CMD_POLL);
}

/* =================================================================
 *  Miscellaneous - GPIO
 * =================================================================*/

vtss_rc vtss_jr2_gpio_mode(vtss_state_t *vtss_state,
                            const vtss_chip_no_t   chip_no,
                            const vtss_gpio_no_t   gpio_no,
                            const vtss_gpio_mode_t mode)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_gpio_read(vtss_state_t *vtss_state,
                              const vtss_chip_no_t  chip_no,
                              const vtss_gpio_no_t  gpio_no,
                              BOOL                  *const value)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_gpio_write(vtss_state_t *vtss_state,
                               const vtss_chip_no_t  chip_no,
                               const vtss_gpio_no_t  gpio_no,
                               const BOOL            value)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_sgpio_event_poll(vtss_state_t *vtss_state,
                                     const vtss_chip_no_t     chip_no,
                                     const vtss_sgpio_group_t group,
                                     const u32                bit,
                                     BOOL                     *const events)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_sgpio_event_enable(vtss_state_t *vtss_state,
                                       const vtss_chip_no_t     chip_no,
                                       const vtss_sgpio_group_t group,
                                       const u32                port,
                                       const u32                bit,
                                       const BOOL               enable)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_sgpio_conf_set(vtss_state_t *vtss_state,
                                   const vtss_chip_no_t     chip_no,
                                   const vtss_sgpio_group_t group,
                                   const vtss_sgpio_conf_t  *const conf)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

static vtss_rc jr2_sgpio_read(vtss_state_t *vtss_state,
                               const vtss_chip_no_t     chip_no,
                               const vtss_sgpio_group_t group,
                               vtss_sgpio_port_data_t   data[VTSS_SGPIO_PORTS])
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

/* - Debug print --------------------------------------------------- */

static vtss_rc jr2_debug_misc(vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t   *const info)
{
    // JR2-TBD: Stub
    return VTSS_RC_ERROR;
}

vtss_rc vtss_jr2_misc_debug_print(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_MISC, jr2_debug_misc, vtss_state, pr, info);
}

/* - Initialization ------------------------------------------------ */

vtss_rc vtss_jr2_misc_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_misc_state_t *state = &vtss_state->misc;

    if (cmd == VTSS_INIT_CMD_CREATE) {
        state->reg_read = jr2_reg_read;
        state->reg_write = jr2_reg_write;
        state->chip_id_get = vtss_jr2_chip_id_get;
        state->poll_1sec = jr2_poll_1sec;
        state->gpio_mode = vtss_jr2_gpio_mode;
        state->gpio_read = jr2_gpio_read;
        state->gpio_write = jr2_gpio_write;
        state->sgpio_conf_set = jr2_sgpio_conf_set;
        state->sgpio_read = jr2_sgpio_read;
        state->sgpio_event_enable = jr2_sgpio_event_enable;
        state->sgpio_event_poll = jr2_sgpio_event_poll;
        state->ptp_event_poll = jr2_ptp_event_poll;
        state->ptp_event_enable = jr2_ptp_event_enable;
        state->intr_cfg = jr2_intr_cfg;
        state->intr_pol_negation = jr2_intr_pol_negation;
#ifdef VTSS_FEATURE_IRQ_CONTROL
        vtss_state->misc.irq_cfg = jr2_misc_irq_cfg;
        vtss_state->misc.irq_status = jr2_misc_irq_status;
        vtss_state->misc.irq_enable = jr2_misc_irq_enable;
#endif  /* VTSS_FEATURE_IRQ_CONTROL */
        state->gpio_count = JR2_GPIOS;
        state->sgpio_group_count = JR2_SGPIO_GROUPS;

#if defined(VTSS_FEATURE_EEE)
        vtss_state->eee.port_conf_set   = jr2_eee_port_conf_set;
#endif /* VTSS_FEATURE_EEE */
#if defined(VTSS_FEATURE_FAN)
        vtss_state->fan.controller_init = jr2_fan_controller_init;
        vtss_state->fan.cool_lvl_get    = jr2_fan_cool_lvl_get;
        vtss_state->fan.cool_lvl_set    = jr2_fan_cool_lvl_set;
        vtss_state->fan.rotation_get    = jr2_fan_rotation_get;
#endif /* VTSS_FEATURE_FAN */
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_ARCH_JAGUAR_2 */
