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

#include "vtss_api.h"
#include "vtss_state.h"
#include "vtss_common.h"

#if defined(VTSS_FEATURE_MISC)

/* Read register */
vtss_rc vtss_reg_read(const vtss_inst_t    inst,
                      const vtss_chip_no_t chip_no,
                      const u32            addr,
                      u32 * const          value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(misc.reg_read, chip_no, addr, value);
        VTSS_N("addr: 0x%08x, value: 0x%08x", addr, *value);
    }
    VTSS_EXIT();

    return rc;
}


/* Write register */
vtss_rc vtss_reg_write(const vtss_inst_t    inst,
                       const vtss_chip_no_t chip_no,
                       const u32            addr,
                       const u32            value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(misc.reg_write, chip_no, addr, value);
        VTSS_N("addr: 0x%08x, value: 0x%08x", addr, value);
    }
    VTSS_EXIT();

    return rc;
}

/* Write masked to register */
vtss_rc vtss_reg_write_masked(const vtss_inst_t    inst,
                              const vtss_chip_no_t chip_no,
                              const u32            addr,
                              const u32            value,
                              const u32            mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    u32          val = 0; /* Please Lint */

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK &&
        (rc = VTSS_FUNC(misc.reg_read, chip_no, addr, &val)) == VTSS_RC_OK) {
        val = ((val & ~mask) | (value & mask));
        rc = VTSS_FUNC(misc.reg_write, chip_no, addr, val);
        VTSS_D("addr: 0x%08x, value: 0x%08x", addr, val);
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_chip_id_get(const vtss_inst_t  inst,
                         vtss_chip_id_t     *const chip_id)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.chip_id_get, chip_id);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_intr_sticky_clear(const vtss_inst_t    inst,
                               u32                  ext)
{
    vtss_state_t *vtss_state = vtss_inst_check_no_persist(inst);
    
    return VTSS_FUNC_FROM_STATE(vtss_state, misc.intr_sticky_clear, ext);
}

vtss_rc vtss_poll_1sec(const vtss_inst_t  inst)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

#if defined(VTSS_CHIP_10G_PHY)
    rc = vtss_phy_10g_poll_1sec(inst);
#endif

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        vtss_rc rc2 = VTSS_FUNC_0(misc.poll_1sec);
        if (rc == VTSS_RC_OK) {
            rc = rc2;
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_ptp_event_poll(const vtss_inst_t      inst,
                            vtss_ptp_event_type_t  *const ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(misc.ptp_event_poll, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_ptp_event_enable(const vtss_inst_t            inst,
                              const vtss_ptp_event_type_t  ev_mask,
                              const BOOL                   enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        if(rc == VTSS_RC_OK && ev_mask) {
            rc = VTSS_FUNC(misc.ptp_event_enable, ev_mask, enable);
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_dev_all_event_poll(const vtss_inst_t                 inst,
                                const vtss_dev_all_event_poll_t   poll_type,
                                vtss_dev_all_event_type_t         *const ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(misc.dev_all_event_poll, poll_type, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_dev_all_event_enable(const vtss_inst_t                inst,
                                  const vtss_port_no_t             port_no,
                                  const vtss_dev_all_event_type_t  ev_mask,
                                  const BOOL                       enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK && ev_mask) {
        rc = VTSS_FUNC(misc.dev_all_event_enable, port_no, ev_mask, enable);
    }
    VTSS_EXIT();
    return rc;
}

#if defined(VTSS_GPIOS)
static vtss_rc vtss_gpio_no_check(vtss_state_t *vtss_state, const vtss_gpio_no_t gpio_no)
{
    if (gpio_no > vtss_state->misc.gpio_count) {
        VTSS_E("illegal gpio_no: %u", gpio_no);
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

vtss_rc vtss_gpio_mode_set(const vtss_inst_t      inst,
                           const vtss_chip_no_t   chip_no,
                           const vtss_gpio_no_t   gpio_no,
                           const vtss_gpio_mode_t mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK &&
        (rc = vtss_gpio_no_check(vtss_state, gpio_no)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.gpio_mode, chip_no, gpio_no, mode);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_gpio_direction_set(const vtss_inst_t     inst,
                                const vtss_chip_no_t  chip_no,
                                const vtss_gpio_no_t  gpio_no,
                                const BOOL            output)
{
    /* Use general form: vtss_gpio_mode_set() */
    return vtss_gpio_mode_set(inst, chip_no, gpio_no, output ? VTSS_GPIO_OUT : VTSS_GPIO_IN);
}

vtss_rc vtss_gpio_read(const vtss_inst_t     inst,
                       const vtss_chip_no_t  chip_no,
                       const vtss_gpio_no_t  gpio_no,
                       BOOL                  *const value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK &&
        (rc = vtss_gpio_no_check(vtss_state, gpio_no)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.gpio_read, chip_no, gpio_no, value);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_gpio_write(const vtss_inst_t     inst,
                        const vtss_chip_no_t  chip_no,
                        const vtss_gpio_no_t  gpio_no,
                        const BOOL            value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK &&
        (rc = vtss_gpio_no_check(vtss_state, gpio_no)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.gpio_write, chip_no, gpio_no, value);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_gpio_event_poll(const vtss_inst_t        inst,
                             const vtss_chip_no_t     chip_no,
                             BOOL                     *const events)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.gpio_event_poll, chip_no, events);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_gpio_event_enable(const vtss_inst_t       inst,
                               const vtss_chip_no_t    chip_no,
                               const vtss_gpio_no_t    gpio_no,
                               BOOL                    enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK &&
        (rc = vtss_gpio_no_check(vtss_state, gpio_no)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.gpio_event_enable, chip_no, gpio_no, enable);
    VTSS_EXIT();
    return rc;
}

#if defined(VTSS_ARCH_B2)
vtss_rc vtss_gpio_clk_set(const vtss_inst_t            inst,
                          const vtss_gpio_no_t         gpio_no,
                          const vtss_port_no_t         port_no,
                          const vtss_recovered_clock_t clk)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK &&
        (rc = vtss_gpio_no_check(vtss_state, gpio_no)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.gpio_clk_set, gpio_no, port_no, clk);
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_ARCH_B2 */
#endif /* GPIOS */

#if defined(VTSS_ARCH_DAYTONA)
vtss_rc vtss_efec_event_enable(const vtss_inst_t         inst,
                               const vtss_port_no_t      port_no,
                               const BOOL                enable,
                               const vtss_efec_event_t   ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.efec_event_enable, port_no, enable, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_efec_event_poll(const vtss_inst_t     inst,
                             const vtss_port_no_t    port_no,
                             vtss_efec_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK)
        rc = VTSS_FUNC_COLD(cil.efec_event_poll, port_no, status);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_efec_event_poll_without_mask(const vtss_inst_t     inst,
                                          const vtss_port_no_t  port_no,
                                          vtss_efec_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.efec_event_poll_without_mask, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_efeci7_event_enable(const vtss_inst_t         inst,
                                const vtss_port_no_t      port_no,
                                const BOOL                enable,
                                const vtss_efeci7_event_t   ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.efeci7_event_enable, port_no, enable, ev_mask);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_efeci7_event_poll(const vtss_inst_t     inst,
                                const vtss_port_no_t    port_no,
                                vtss_efeci7_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK)
        rc = VTSS_FUNC(cil.efeci7_event_poll, port_no, status);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_efeci7_event_poll_without_mask(const vtss_inst_t     inst,
                                          const vtss_port_no_t  port_no,
                                          vtss_efeci7_event_t     *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("port_no: %u", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.efeci7_event_poll_without_mask, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}
#endif

/* - Serial LED ---------------------------------------------------- */

#if defined(VTSS_FEATURE_SERIAL_LED)
vtss_rc vtss_serial_led_set(const vtss_inst_t      inst,
                            const vtss_led_port_t  port,
                            const vtss_led_mode_t  *const mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.serial_led_set, port, mode);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_serial_led_intensity_set(const vtss_inst_t      inst,
                                      const vtss_led_port_t  port,
                                      const i8               intensity)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.serial_led_intensity_set, port, intensity);
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_serial_led_intensity_get(const vtss_inst_t      inst,
                                      i8               *intensity)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.serial_led_intensity_get, intensity);
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_FEATURE_SERIAL_LED */

/* - Serial IO control ---------------------------------------------------- */

#if defined(VTSS_FEATURE_SERIAL_GPIO)
static vtss_rc vtss_sgpio_group_check(vtss_state_t *vtss_state, const vtss_sgpio_group_t group)
{
    if (group >= vtss_state->misc.sgpio_group_count) {
        VTSS_E("illegal SGPIO group: %u", group);
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

vtss_rc vtss_sgpio_conf_get(const vtss_inst_t        inst,
                            const vtss_chip_no_t     chip_no,
                            const vtss_sgpio_group_t group,
                            vtss_sgpio_conf_t        *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK &&
        (rc = vtss_sgpio_group_check(vtss_state, group)) == VTSS_RC_OK) {
        *conf = vtss_state->misc.sgpio_conf[chip_no][group];
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_sgpio_conf_set(const vtss_inst_t        inst,
                            const vtss_chip_no_t     chip_no,
                            const vtss_sgpio_group_t group,
                            const vtss_sgpio_conf_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK &&
        (rc = vtss_sgpio_group_check(vtss_state, group)) == VTSS_RC_OK) {
        if (conf->bit_count == 0 || conf->bit_count > 4) {
            VTSS_E("illegal bit_count: %u", conf->bit_count);
            rc = VTSS_RC_ERROR;
        } else {
            vtss_state->misc.sgpio_conf[chip_no][group] = *conf;
            rc = VTSS_FUNC(misc.sgpio_conf_set, chip_no, group, conf);
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_sgpio_read(const vtss_inst_t        inst,
                        const vtss_chip_no_t     chip_no,
                        const vtss_sgpio_group_t group,
                        vtss_sgpio_port_data_t   data[VTSS_SGPIO_PORTS])
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK &&
        (rc = vtss_sgpio_group_check(vtss_state, group)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(misc.sgpio_read, chip_no, group, data);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_sgpio_event_poll(const vtss_inst_t        inst,
                              const vtss_chip_no_t     chip_no,
                              const vtss_sgpio_group_t group,
                              const u32                bit,
                              BOOL                     *const events)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK && 
        (rc = vtss_sgpio_group_check(vtss_state, group)) == VTSS_RC_OK) {
        if (bit >= 4) {
            VTSS_E("illegal parameter  bit: %u", bit);
            rc = VTSS_RC_ERROR;
        } else {
            rc = VTSS_FUNC(misc.sgpio_event_poll, chip_no, group, bit, events);
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_sgpio_event_enable(const vtss_inst_t        inst,
                                const vtss_chip_no_t     chip_no,
                                const vtss_sgpio_group_t group,
                                const vtss_port_no_t     port,
                                const u32                bit,
                                BOOL                     enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_chip_no_check(inst, &vtss_state, chip_no)) == VTSS_RC_OK && 
        (rc = vtss_sgpio_group_check(vtss_state, group)) == VTSS_RC_OK) {
        if ((port >= VTSS_SGPIO_PORTS) || (bit >= 4)) {
            VTSS_E("illegal parameter  port: %u  bit: %u", port, bit);
            rc = VTSS_RC_ERROR;
        } else {
            vtss_state->misc.sgpio_event_enabled[chip_no][group].enable[port][bit] = enable;
            rc = VTSS_FUNC(misc.sgpio_event_enable, chip_no, group, port, bit, enable);
        }
    }
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_FEATURE_SERIAL_GPIO */

/* - Interrupts ----------------------------------------------------------- */

#if defined(VTSS_FEATURE_INTERRUPTS)
vtss_rc vtss_intr_cfg(const vtss_inst_t       inst,
                      const u32               mask,
                      const BOOL              polarity,
                      const BOOL              enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.intr_cfg, mask, polarity, enable);
    VTSS_EXIT();
    return rc;

}
                      
vtss_rc vtss_intr_mask_set(const vtss_inst_t       inst,
                           vtss_intr_t *mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.intr_mask_set,mask);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_intr_status_get(const vtss_inst_t       inst,
                             vtss_intr_t *status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.intr_status_get, status);
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_intr_pol_negation(const vtss_inst_t   inst)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC_0(misc.intr_pol_negation);
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_FEATURE_INTERRUPTS  */

#ifdef VTSS_FEATURE_IRQ_CONTROL

/**
 * 
 **/
vtss_rc  vtss_irq_conf_get(const vtss_inst_t inst,
                           const vtss_irq_t irq,
                           vtss_irq_conf_t *conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc = VTSS_RC_ERROR;

    VTSS_ENTER();
    if (irq >= 0 && irq < VTSS_IRQ_MAX &&
        (rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        *conf = vtss_state->misc.irq_conf[irq];
    }
    VTSS_EXIT();
    return rc;
}

/**
 * 
 **/
vtss_rc  vtss_irq_conf_set(const vtss_inst_t inst,
                           const vtss_irq_t irq,
                           const vtss_irq_conf_t *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc = VTSS_RC_ERROR;

    VTSS_ENTER();
    if (irq >= 0 && irq < VTSS_IRQ_MAX &&
        (rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        if((rc = VTSS_FUNC(misc.irq_cfg, irq, conf)) == VTSS_RC_OK) {
            vtss_state->misc.irq_conf[irq] = *conf;
        }
    }
    VTSS_EXIT();
    return rc;
}

/**
 * 
 **/
vtss_rc  vtss_irq_status_get_and_mask(const vtss_inst_t inst,
                                      vtss_irq_status_t *status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.irq_status, status);
    VTSS_EXIT();
    return rc;
}

/**
 * 
 **/
vtss_rc  vtss_irq_enable(const vtss_inst_t inst,
                         vtss_irq_t irq,
                         BOOL enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        rc = VTSS_FUNC(misc.irq_enable, irq, enable);
    VTSS_EXIT();
    return rc;
}


#endif /* VTSS_FEATURE_IRQ_CONTROL */

/* - Fan control ---------------------------------------------------------- */

#if defined(VTSS_FEATURE_FAN)
vtss_rc vtss_temp_sensor_init(const vtss_inst_t     inst,
                              const BOOL enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(fan.chip_temp_init, enable);
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_temp_sensor_get(const vtss_inst_t     inst,
                             i16                   *temperature)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(fan.chip_temp_get, temperature);
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_fan_rotation_get(const vtss_inst_t     inst,
                              vtss_fan_conf_t       *const fan_spec,
                              u32                   *const rotation_count)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(fan.rotation_get, fan_spec, rotation_count);
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_fan_controller_init(const vtss_inst_t      inst,
                                 const vtss_fan_conf_t *const fan_spec)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(fan.controller_init, fan_spec);
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_fan_cool_lvl_set(const vtss_inst_t     inst,
                              u8 lvl)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(fan.cool_lvl_set, lvl);
    }
    VTSS_EXIT();

    return rc;
}

vtss_rc vtss_fan_cool_lvl_get(const vtss_inst_t     inst,
                              u8 *lvl)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(fan.cool_lvl_get, lvl);
    }
    VTSS_EXIT();

    return rc;
}
#endif /* VTSS_FEATURE_FAN */

/* - EEE ------------------------------------------------------------------ */

#if defined(VTSS_FEATURE_EEE)
vtss_rc vtss_eee_port_conf_set(const vtss_inst_t                 inst,
                               vtss_port_no_t                    port_no,
                               const vtss_eee_port_conf_t *const eee_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC(eee.port_conf_set, port_no, eee_conf);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_eee_port_state_set(const vtss_inst_t                  inst,
                                vtss_port_no_t                     port_no,
                                const vtss_eee_port_state_t *const eee_state)
{
    // Cannot take API lock or change vtss_state on JR, since EEE
    // control is S/W-based and must go fast.
    vtss_state_t *vtss_state = vtss_inst_check_no_persist(inst);

    return VTSS_FUNC_FROM_STATE(vtss_state, eee.port_state_set, port_no, eee_state);
}

vtss_rc vtss_eee_port_counter_get(const vtss_inst_t                    inst,
                                  const vtss_port_no_t                 port_no,
                                        vtss_eee_port_counter_t *const eee_counter)
{
    // Cannot take API lock or change vtss_state on JR, since EEE
    // control is S/W-based and must go fast.
    vtss_state_t *vtss_state = vtss_inst_check_no_persist(inst);

    return VTSS_FUNC_FROM_STATE(vtss_state, eee.port_counter_get, port_no, eee_counter);
}
#endif /* VTSS_FEATURE_EEE */

/* - Instance create and initialization ---------------------------- */

vtss_rc vtss_misc_inst_create(vtss_state_t *vtss_state)
{
#if defined(VTSS_FEATURE_SERIAL_GPIO)
    {
        vtss_sgpio_group_t group;

        for (group = 0; group < VTSS_SGPIO_GROUPS; group++) {
            vtss_state->misc.sgpio_conf[0][group].bit_count = 4;
            vtss_state->misc.sgpio_conf[1][group].bit_count = 4;
        }
    }
#endif /* VTSS_FEATURE_SERIAL_GPIO */

    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

static void vtss_debug_print_misc(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    vtss_chip_id_t chip_id;

    if (!vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_MISC))
        return;
    
    if (vtss_state->misc.chip_id_get != NULL && 
        vtss_state->misc.chip_id_get(vtss_state, &chip_id) == VTSS_RC_OK) {
        pr("Chip ID : 0x%04x\n", chip_id.part_number);
        pr("Revision: 0x%04x\n", chip_id.revision);
    }
    pr("\n");
}

#if defined(VTSS_FEATURE_SERIAL_GPIO)
static void vtss_debug_print_ser_gpio(vtss_state_t *vtss_state,
                                      const vtss_debug_printf_t pr,
                                      const vtss_debug_info_t   *const info)
{
    vtss_chip_no_t         chip_no;
    vtss_sgpio_group_t     group;
    char                   buf[64];
    vtss_sgpio_conf_t      *conf;
    vtss_sgpio_port_conf_t *port_conf;
    u32                    port, i;

    if (!vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_SER_GPIO))
        return;

    /* Print CIL information for all devices and groups */
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        for (group = 0; group < vtss_state->misc.sgpio_group_count; group++) {
            sprintf(buf, "Device %u, Group %u", chip_no, group);
            vtss_debug_print_header(pr, buf);
            
            conf  = &vtss_state->misc.sgpio_conf[chip_no][group]; 
            pr("Bit Count   : %u\n", conf->bit_count);

            // Print out blink mode 
            for (i = 0; i < 2; i++) {
                vtss_sgpio_bmode_t m = conf->bmode[i];
                pr("Blink Mode_%u: %s\n",
                   i, 
                   m == VTSS_SGPIO_BMODE_TOGGLE ? "Burst Toggle (mode 1)" :
                   m == VTSS_SGPIO_BMODE_0_625 ? "0.625 Hz (mode 0)" :
                   m == VTSS_SGPIO_BMODE_1_25 ? "1.25 Hz" :
                   m == VTSS_SGPIO_BMODE_2_5 ? "2.5 Hz" :
                   m == VTSS_SGPIO_BMODE_5 ? "5 Hz" : "?");
            }

            pr("\n");
            pr("Port  Status    ");
            for (i = 0; i < 4; i++)
                pr("Mode_%u     ", i);
            pr("\n");
            for (port = 0; port < VTSS_SGPIO_PORTS; port++) {
                port_conf = &conf->port_conf[port];
                pr("%-4u  %-10s", port, vtss_bool_txt(port_conf->enabled));
                for (i = 0; i < 4; i++) {
                    vtss_sgpio_mode_t m = port_conf->mode[i];
                    pr("%-11s", 
                       m == VTSS_SGPIO_MODE_OFF ? "Off" :
                       m == VTSS_SGPIO_MODE_ON ? "On" :
                       m == VTSS_SGPIO_MODE_0 ? "0" :
                       m == VTSS_SGPIO_MODE_1 ? "1" :
                       m == VTSS_SGPIO_MODE_0_ACTIVITY ? "0_ACT" :
                       m == VTSS_SGPIO_MODE_1_ACTIVITY ? "1_ACT" :
                       m == VTSS_SGPIO_MODE_0_ACTIVITY_INV ? "0_ACT_INV" :
                       m == VTSS_SGPIO_MODE_1_ACTIVITY_INV ? "1_ACT_INV" : "?");
                }
                pr("\n");
            }
            pr("\n");
        }
    }
}
#endif /* VTSS_FEATURE_SERIAL_GPIO */
    
void vtss_misc_debug_print(vtss_state_t *vtss_state,
                           const vtss_debug_printf_t pr,
                           const vtss_debug_info_t   *const info)
{
    vtss_debug_print_misc(vtss_state, pr, info);

#if defined(VTSS_FEATURE_SERIAL_GPIO)
    vtss_debug_print_ser_gpio(vtss_state, pr, info);
#endif
}
#endif /* VTSS_FEATURE_MISC */
