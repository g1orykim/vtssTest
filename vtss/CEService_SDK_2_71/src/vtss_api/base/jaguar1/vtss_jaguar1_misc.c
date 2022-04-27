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

#include "vtss_jaguar1_cil.h"

#if defined(VTSS_ARCH_JAGUAR_1)

/* - CIL functions ------------------------------------------------- */

// Function for initializing chip temperature sensor
//  
// In: enable - True if temperature sensor shall be enabled else false
//
// Return: VTSS_RC_OK if temperature sensor was initialized - else error code


static vtss_rc jr_temp_sensor_init(vtss_state_t *vtss_state, const BOOL enable) 
{
    if (enable) {
        JR_WR(DEVCPU_GCB, TEMP_SENSOR_TEMP_SENSOR_CTRL, 0x1);   // Enable sensor (see datasheet)
    } else {
        JR_WR(DEVCPU_GCB, TEMP_SENSOR_TEMP_SENSOR_CTRL, 0x2);    // Disable sensor (see datasheet)
    }
    return VTSS_RC_OK;
}

// Function for getting chip temperature 
//  
// In: port_no - The PHY port number starting from 0.
//
// In/Out: Temp - The chip temperature (from -46 to 135 degC)
//
// Return: VTSS_RC_OK if we got the temperature - else error code

static vtss_rc jr_temp_sensor_read(vtss_state_t *vtss_state, i16 *temp) 
{
    u32 reg_val;
    JR_RD(DEVCPU_GCB, TEMP_SENSOR_TEMP_SENSOR_DATA, &reg_val);
    
    // Bit 8 determines if the data is valid (see datasheet)
    if (reg_val & 0x100) {
        //135.3degC - 0.71degC*ADCOUT
        *temp = (13530 - 71 * (reg_val & 0xFF))/100;
    } else {
        *temp = 135; // Data not valid
        return VTSS_RC_ERROR;
    }
   
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_gpio_mode(vtss_state_t           *vtss_state,
                           const vtss_chip_no_t   chip_no,
                           const vtss_gpio_no_t   gpio_no,
                           const vtss_gpio_mode_t mode)
{
    u32 mask = VTSS_BIT(gpio_no);

    JR_WRM_CLR(DEVCPU_GCB, GPIO_GPIO_INTR_ENA, mask); /* Disable IRQ */
    switch(mode) {
    case VTSS_GPIO_OUT:
    case VTSS_GPIO_IN:
    case VTSS_GPIO_IN_INT:
        JR_WRXM_CLR(DEVCPU_GCB_GPIO_GPIO, ALT, 0, mask); /* GPIO mode 0b00 */
        JR_WRXM_CLR(DEVCPU_GCB_GPIO_GPIO, ALT, 1, mask); /* -"- */
        JR_WRM(DEVCPU_GCB_GPIO_GPIO, OE, (mode == VTSS_GPIO_OUT ? mask : 0), mask);
        if(mode == VTSS_GPIO_IN_INT)
            JR_WRM_SET(DEVCPU_GCB_GPIO_GPIO, INTR_ENA, mask);
        return VTSS_RC_OK;
    case VTSS_GPIO_ALT_0:
        JR_WRXM_SET(DEVCPU_GCB_GPIO_GPIO, ALT, 0, mask); /* GPIO mode 0b01 */
        JR_WRXM_CLR(DEVCPU_GCB_GPIO_GPIO, ALT, 1, mask); /* -"- */
        return VTSS_RC_OK;
    case VTSS_GPIO_ALT_1:
        JR_WRXM_CLR(DEVCPU_GCB_GPIO_GPIO, ALT, 0, mask); /* GPIO mode 0b10 */
        JR_WRXM_SET(DEVCPU_GCB_GPIO_GPIO, ALT, 1, mask); /* -"- */
        return VTSS_RC_OK;
    default:
        break;
    }
    return VTSS_RC_ERROR;
}

static vtss_rc jr_gpio_read(vtss_state_t          *vtss_state,
                            const vtss_chip_no_t  chip_no,
                            const vtss_gpio_no_t  gpio_no,
                            BOOL                  *const value)
{
    u32 val,  mask = VTSS_BIT(gpio_no);

    JR_RD(DEVCPU_GCB, GPIO_GPIO_IN, &val);
    *value = VTSS_BOOL(val & mask);
    return VTSS_RC_OK;
}

static vtss_rc jr_gpio_write(vtss_state_t          *vtss_state,
                             const vtss_chip_no_t  chip_no,
                             const vtss_gpio_no_t  gpio_no, 
                             const BOOL            value)
{
    u32 mask = VTSS_BIT(gpio_no);

    if(value) {
        JR_WR(DEVCPU_GCB, GPIO_GPIO_OUT_SET, mask);
    } else {
        JR_WR(DEVCPU_GCB, GPIO_GPIO_OUT_CLR, mask);
    }
    return VTSS_RC_OK;
}

static vtss_rc jr_gpio_event_poll(vtss_state_t          *vtss_state,
                                  const vtss_chip_no_t  chip_no,
                                  BOOL                  *const events)
{
    u32 pending, mask, i;

    JR_RD(DEVCPU_GCB, GPIO_GPIO_INTR, &pending);
    JR_RD(DEVCPU_GCB, GPIO_GPIO_INTR_ENA, &mask);
    pending &= mask;
    JR_WR(DEVCPU_GCB, GPIO_GPIO_INTR, pending);

    for (i=0; i<VTSS_GPIOS; i++) {
        events[i] = (pending & 1<<i) ? TRUE : FALSE;
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_gpio_event_enable(vtss_state_t          *vtss_state,
                                    const vtss_chip_no_t  chip_no,
                                    const vtss_gpio_no_t  gpio_no,
                                    const BOOL            enable)
{
    JR_WRM(DEVCPU_GCB, GPIO_GPIO_INTR_ENA, enable ? 1<<gpio_no : 0, 1<<gpio_no);

    return VTSS_RC_OK;
}

static vtss_rc jr_sgpio_event_poll(vtss_state_t             *vtss_state,
                                   const vtss_chip_no_t     chip_no,
                                   const vtss_sgpio_group_t group,
                                   const u32                bit,
                                   BOOL                     *const events)
{
    u32 i, val = 0;

    JR_RDXY(DEVCPU_GCB, SIO_CTRL_SIO_INT_REG, group, bit, &val);
    JR_WRXY(DEVCPU_GCB, SIO_CTRL_SIO_INT_REG, group, bit, val);

    /* Setup serial IO port enable register */
    for (i=0; i<32; i++) {
        events[i] = (val & 1<<i) ? TRUE : FALSE;
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_sgpio_event_enable(vtss_state_t             *vtss_state,
                                     const vtss_chip_no_t     chip_no,
                                     const vtss_sgpio_group_t group,
                                     const u32                port,
                                     const u32                bit,
                                     const BOOL               enable)
{
    u32 data, pol, i;

    if (enable) {
        JR_RDXY(DEVCPU_GCB, SIO_CTRL_SIO_INPUT_DATA, group, bit, &data);
        JR_RDXY(DEVCPU_GCB, SIO_CTRL_SIO_INT_POL, group ,bit, &pol);

        VTSS_I("Enable port:%d, group:%d, bit:%d,pol:0x%X, data:0x%X", port, group, bit, pol, data);

        // XOR with polarity in order to find ports with non_active interrupts 
        data ^= ~pol;    /* Since pol is 0 when interrupt level high we invert the polarity - Now data '1' means active interrupt */
        
        if (data & 1<<port) {   /* Only enable if not active interrupt - as interrupt pending cannot be cleared */
            VTSS_I("Enable SGPIO int port:%d, group:%d, data:0x%X, pol:0x%X, bit:%d", port, group, data, pol, bit);
            JR_WRXM(DEVCPU_GCB, SIO_CTRL_SIO_PORT_INT_ENA, group, 1<<port, 1<<port);
        }
        JR_WRXM(DEVCPU_GCB, SIO_CTRL_SIO_CONFIG, group, VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA(1<<bit), VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA(1<<bit));
    }
    else {
        VTSS_I("Disable port:%d, group:%d, bit:%d", port, group, bit);
        JR_WRXM(DEVCPU_GCB, SIO_CTRL_SIO_PORT_INT_ENA, group, 0, 1<<port);
        for (i=0; i<32; ++i)
            if (vtss_state->misc.sgpio_event_enabled[chip_no][group].enable[i][bit])      break;
        if (i == 32)
            JR_WRXM(DEVCPU_GCB, SIO_CTRL_SIO_CONFIG, group, 0, VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA(1<<bit));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_sgpio_conf_set(vtss_state_t             *vtss_state,
                                 const vtss_chip_no_t     chip_no,
                                 const vtss_sgpio_group_t group,
                                 const vtss_sgpio_conf_t  *const conf)
{
    u32 i, port, val = 0, bmode[2], bit_idx;

    /* Setup serial IO port enable register */
    for (port = 0; port < 32; port++) {
        if (conf->port_conf[port].enabled)
            val |= VTSS_BIT(port);
    }
    JR_WRX(DEVCPU_GCB, SIO_CTRL_SIO_PORT_ENABLE, group, val);

    /* 
     * Setup general configuration register
     *
     * The burst gap is 0x1f(33.55ms)
     * The load signal is active low
     * The auto burst repeat function is on
     * The SIO reverse output is off
     */
    for (i = 0; i < 2; i++) {
        switch (conf->bmode[i]) {
        case VTSS_SGPIO_BMODE_TOGGLE:
            if (i == 0) {
                VTSS_E("blink mode 0 does not support TOGGLE (group:%d)",group);
                return VTSS_RC_ERROR;
            }
            bmode[i] = 3;
            break;
        case VTSS_SGPIO_BMODE_0_625:
            if (i == 1) {
                VTSS_E("blink mode 1 does not support 0.625 Hz");
                return VTSS_RC_ERROR;
            }
            bmode[i] = 3;
            break;
        case VTSS_SGPIO_BMODE_1_25:
            bmode[i] = 2;
            break;
        case VTSS_SGPIO_BMODE_2_5:
            bmode[i] = 1;
            break;
        case VTSS_SGPIO_BMODE_5:
            bmode[i] = 0;
            break;
        default:
            return VTSS_RC_ERROR;
        }
    }

    val = (VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_0(bmode[0]) |
           VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_1(bmode[1]) |
           VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BURST_GAP(0x00) |
           VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_PORT_WIDTH(conf->bit_count - 1) |
           VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_AUTO_REPEAT);
    JR_WRXM(DEVCPU_GCB, SIO_CTRL_SIO_CONFIG, group, val, ~VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA);

    /* Setup the serial IO clock frequency - 12.5MHz (0x28) */
    JR_WRX(DEVCPU_GCB, SIO_CTRL_SIO_CLOCK, group, 0x28);

    /*
     * Configuration of output data values
     * The placement of the source select bits for each output bit in the register:
     * Output bit0 : (2 downto 0)
     * Output bit1 : (5 downto 3)
     * Output bit2 : (8 downto 6)
     * Output bit3 : (11 downto 9)
    */
    for (port = 0; port < 32; port++) {
        for (val = 0, bit_idx = 0; bit_idx < 4; bit_idx++) {
            /* Set output bit n */
            val |= VTSS_ENCODE_BITFIELD(conf->port_conf[port].mode[bit_idx], (bit_idx * 3), 3);

            // Setup the interrupt polarity
            JR_WRXYM(DEVCPU_GCB, SIO_CTRL_SIO_INT_POL, group , 
                     bit_idx, conf->port_conf[port].int_pol_high[bit_idx] ? (0 << port) : (1 << port), 1 << port);
   
            VTSS_I("group:%d, port:%d, bit_idx:%d, int_pol_high:%d", group, port, bit_idx, conf->port_conf[port].int_pol_high[bit_idx]);
        }
        JR_WR(DEVCPU_GCB, SIO_CTRL_SIO_PORT_CONFIG(group, port), val);
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_sgpio_read(vtss_state_t             *vtss_state,
                             const vtss_chip_no_t     chip_no,
                             const vtss_sgpio_group_t group,
                             vtss_sgpio_port_data_t   data[VTSS_SGPIO_PORTS])
{
    u32 i, port, value;

    for (i = 0; i < 4; i++) {
        JR_RDXY(DEVCPU_GCB, SIO_CTRL_SIO_INPUT_DATA, group, i, &value);
        for (port = 0; port < 32; port++) {
            data[port].value[i] = VTSS_BOOL(value & (1 << port));
        }
    }
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_chip_id_get_chip(vtss_state_t *vtss_state, vtss_chip_id_t *const chip_id)
{
    u32 value;

    JR_RD(DEVCPU_GCB, CHIP_REGS_CHIP_ID, &value);

    if (value == 0 || value == 0xffffffff) {
        VTSS_E("CPU interface[%u] error, chipid: 0x%08x", vtss_state->chip_no, value);
        return VTSS_RC_ERROR;
    }

    chip_id->part_number = JR_GET_FLD(DEVCPU_GCB, CHIP_REGS_CHIP_ID, PART_ID, value);
    chip_id->revision = JR_GET_FLD(DEVCPU_GCB, CHIP_REGS_CHIP_ID, REV_ID, value);

    return VTSS_RC_OK;
}

static vtss_rc jr_chip_id_get(vtss_state_t *vtss_state, vtss_chip_id_t *const chip_id)
{
    return vtss_jr1_chip_id_get_chip(vtss_state, chip_id);
}

static vtss_rc jr_intr_sticky_clear(const vtss_state_t *vtss_state, u32 ext)
{
    if (vtss_state->chip_count == 2)
        JR_WR_REG_CHIP(1, VTSS_ICPU_CFG_INTR_INTR, (ext == 0) ? VTSS_F_ICPU_CFG_INTR_INTR_EXT_IRQ0_INTR : VTSS_F_ICPU_CFG_INTR_INTR_EXT_IRQ1_INTR);

    return VTSS_RC_OK;
}

static vtss_rc jr_poll_sgpio_1sec(vtss_state_t *vtss_state)
{
    vtss_rc rc = VTSS_RC_OK;
    u32     group, port, bit, enable;

    for (group=0; group<2; ++group) {   /* All SGPIO is checked for "enabled" interrupt that is actually disabled because interrupt is active - pending can not be cleared */
        JR_RDX(DEVCPU_GCB, SIO_CTRL_SIO_PORT_INT_ENA, group, &enable);
        for (port=0; port<32; ++port)
            if (!(enable & 1<<port))
                for (bit=0; bit<4; ++bit)    /* port is not enabled - check if it is configured to be */
                    if (vtss_state->misc.sgpio_event_enabled[0][group].enable[port][bit]) {
                        rc = jr_sgpio_event_enable(vtss_state, 0, group, port, bit, TRUE);  /* this port,bit is configured to be enabled - try and enable */
                    }
    }
    return rc;
}

static vtss_rc jr_ptp_event_poll(vtss_state_t *vtss_state, vtss_ptp_event_type_t *ev_mask)
{
    u32 sticky, mask;
    
    /* PTP events */
    *ev_mask = 0;
    JR_RD(DEVCPU_GCB, PTP_STAT_PTP_EVT_STAT, &sticky);
    JR_WR(DEVCPU_GCB, PTP_STAT_PTP_EVT_STAT, sticky);   /* Clear pending */
    JR_RD(DEVCPU_GCB, PTP_CFG_PTP_SYNC_INTR_ENA_CFG, &mask);
    mask |= VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_CLK_ADJ_UPD_STICKY; /* CLK ADJ event has no enable bit - do not generate interrupt */
    sticky &= mask;      /* Only handle enabled sources */
    
    *ev_mask |= (sticky & VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_SYNC_STAT) ? VTSS_PTP_SYNC_EV : 0;
    *ev_mask |= (sticky & VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_EXT_SYNC_CURRENT_TIME_STICKY) ? VTSS_PTP_EXT_SYNC_EV : 0;
    *ev_mask |= (sticky & VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_CLK_ADJ_UPD_STICKY) ? VTSS_PTP_CLK_ADJ_EV : 0;
    
    VTSS_D("sticky: 0x%x", sticky);
    
    return VTSS_RC_OK;
}

static vtss_rc jr_ptp_event_enable(vtss_state_t *vtss_state,
                                   vtss_ptp_event_type_t ev_mask, BOOL enable)
{
    
    /* PTP masks */
    if (ev_mask & VTSS_PTP_SYNC_EV) {
        JR_WRM(DEVCPU_GCB, PTP_CFG_PTP_SYNC_INTR_ENA_CFG, 
               enable ? VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_SYNC_STAT_ENA : 0, 
               VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_SYNC_STAT_ENA);
    }
    if (ev_mask & VTSS_PTP_EXT_SYNC_EV) {
        JR_WRM(DEVCPU_GCB, PTP_CFG_PTP_SYNC_INTR_ENA_CFG, 
               enable ? VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_EXT_SYNC_CURRENT_TIME_ENA : 0, 
               VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_EXT_SYNC_CURRENT_TIME_ENA);
    }
    
    return VTSS_RC_OK;
}


static vtss_rc jr_dev_all_event_poll(vtss_state_t *vtss_state,
                                     vtss_dev_all_event_poll_t poll_type,
                                     vtss_dev_all_event_type_t  *ev_mask)
{
    u32 sticky=0, mask=0, ident=0, chip_port;
    vtss_port_no_t port;
    
    /* poll timestamp events */
    for (port=VTSS_PORT_NO_START; port<vtss_state->port_count; port++) {
        ev_mask[port] = 0;

        if ((poll_type == VTSS_DEV_ALL_POLL_ALL) ||
            ((poll_type == VTSS_DEV_ALL_POLL_PRIMARY) && (VTSS_CHIP_NO(port) == 0)) ||
            ((poll_type == VTSS_DEV_ALL_POLL_SECONDARY) && (VTSS_CHIP_NO(port) == 1))) { /* This chip needs to be polled */
            VTSS_SELECT_CHIP_PORT_NO(port);     /* Select the chip for this port */
            chip_port = VTSS_CHIP_PORT(port);
    
            JR_RD(ICPU_CFG, INTR_DEV_IDENT, &ident);
            if (ident & (1 << chip_port)) {
                if (VTSS_PORT_IS_1G(chip_port)) {
                    JR_RDX(DEV1G, DEV1G_INTR_CFG_STATUS_DEV1G_INTR, VTSS_TO_DEV(chip_port), &sticky);
                    JR_WRX(DEV1G, DEV1G_INTR_CFG_STATUS_DEV1G_INTR, VTSS_TO_DEV(chip_port), sticky);    /* Clear pending */
                    JR_RDX(DEV1G, DEV1G_INTR_CFG_STATUS_DEV1G_INTR_CFG, VTSS_TO_DEV(chip_port), &mask);
                    sticky &= mask;      /* Only handle enabled sources */
        
                    ev_mask[port] |= (sticky & VTSS_F_DEV1G_DEV1G_INTR_CFG_STATUS_DEV1G_INTR_PTP_UPDATE_INTR_STICKY) ? VTSS_DEV_ALL_TX_TSTAMP_EV : 0;
                    ev_mask[port] |= (sticky & (VTSS_F_DEV1G_DEV1G_INTR_CFG_STATUS_DEV1G_INTR_LINK_UP_INTR_STICKY | VTSS_F_DEV1G_DEV1G_INTR_CFG_STATUS_DEV1G_INTR_LINK_DOWN_INTR_STICKY)) ? VTSS_DEV_ALL_LINK_EV : 0;
                } else {
                    JR_RDX(DEV10G, DEV_CFG_STATUS_INTR, VTSS_TO_DEV(chip_port), &sticky);
                    JR_WRX(DEV10G, DEV_CFG_STATUS_INTR, VTSS_TO_DEV(chip_port), sticky);    /* Clear pending */
                    JR_RDX(DEV10G, DEV_CFG_STATUS_INTR_ENA, VTSS_TO_DEV(chip_port), &mask);
                    sticky &= mask;      /* Only handle enabled sources */
        
                    ev_mask[port] |= (sticky & VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_PTP_UPDT_INTR) ? VTSS_DEV_ALL_TX_TSTAMP_EV : 0;
                    ev_mask[port] |= (sticky & (VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_LINK_UP_INTR | VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_LINK_DWN_INTR)) ? VTSS_DEV_ALL_LINK_EV : 0;
                }
            }
        }
    }
    
    VTSS_D("sticky: 0x%x", sticky);
    
    return VTSS_RC_OK;
}

static vtss_rc jr_dev_all_event_enable(vtss_state_t *vtss_state,
                                       vtss_port_no_t port, vtss_dev_all_event_type_t ev_mask, BOOL enable)
{
    u32 chip_port, mask=0;
    
    if (ev_mask == 0) return VTSS_RC_OK;

    VTSS_SELECT_CHIP_PORT_NO(port);     /* Select the chip for this port */
    chip_port = VTSS_CHIP_PORT(port);

    if (VTSS_PORT_IS_1G(chip_port)) {
        if (ev_mask & VTSS_DEV_ALL_TX_TSTAMP_EV)    mask |= VTSS_F_DEV1G_DEV1G_INTR_CFG_STATUS_DEV1G_INTR_PTP_UPDATE_INTR_STICKY;
        if (ev_mask & VTSS_DEV_ALL_LINK_EV)         mask |= (VTSS_F_DEV1G_DEV1G_INTR_CFG_STATUS_DEV1G_INTR_LINK_UP_INTR_STICKY | VTSS_F_DEV1G_DEV1G_INTR_CFG_STATUS_DEV1G_INTR_LINK_DOWN_INTR_STICKY);
        JR_WRXM(DEV1G, DEV1G_INTR_CFG_STATUS_DEV1G_INTR_CFG, VTSS_TO_DEV(chip_port), enable ? mask : 0, mask);
    } else {
        if (ev_mask & VTSS_DEV_ALL_TX_TSTAMP_EV)    mask |= VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_PTP_UPDT_INTR;
        if (ev_mask & VTSS_DEV_ALL_LINK_EV)         mask |= (VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_LINK_UP_INTR | VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_LINK_DWN_INTR);
        JR_WRXM(DEV10G, DEV_CFG_STATUS_INTR_ENA, VTSS_TO_DEV(chip_port), enable ? mask : 0, mask);
    }
    
    return VTSS_RC_OK;
}

static vtss_rc jr_reg_read(vtss_state_t *vtss_state,
                           const vtss_chip_no_t chip_no, const u32 addr, u32 *const value)
{
    /* Do not change this to calling jr_rd() without chip_no, since it's also called by the FDMA */
    return vtss_jr1_rd_chip(vtss_state, chip_no, addr, value);
}

static vtss_rc jr_reg_write(vtss_state_t *vtss_state,
                            const vtss_chip_no_t chip_no, const u32 addr, const u32 value)
{
    /* Do not change this to calling jr_wr() without chip_no, since it's also called by the FDMA */
    return vtss_jr1_wr_chip(vtss_state, chip_no, addr, value);
}

#ifdef VTSS_FEATURE_EEE
/* =================================================================
 *  EEE - Energy Efficient Ethernet
 * =================================================================*/

// EEE needs a loopback port per chip. Get an unused port.
static vtss_rc jr_eee_loopback_port(vtss_state_t *vtss_state)
{
    u32 chip_port = vtss_jr1_unused_chip_port(vtss_state);

    // If in mux-mode 1 and we need flow control on port 0, we cannot enable
    // EEE, because a host-mode register must be set to the loop port (ref.
    // the other function that uses vtss_jr1_unused_chip_port()).

    // We need flow control when not in CE mode.
    if (vtss_state->port.mux_mode[vtss_state->chip_no] == VTSS_PORT_MUX_MODE_0 && vtss_state->create.target == VTSS_TARGET_E_STAX_III_24_DUAL) {
        VTSS_E("EEE cannot be enabled in mux-mode 0 on non-CE targets");
        return VTSS_RC_ERROR;
    }

    vtss_state->eee.loopback_chip_port[vtss_state->chip_no] = chip_port;
    return VTSS_RC_OK;
}

static vtss_rc jr_eee_ace_add_remove(vtss_state_t *vtss_state)
{
    vtss_vcap_data_t      data;
    vtss_is2_entry_t      entry;
    vtss_ace_t            *ace = &entry.ace;
    vtss_chip_no_t        chip_no;
    vtss_vcap_id_t        id = 1;

    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        // Enable/disable second IS2 lookup on loopback port
        VTSS_RC(vtss_jr1_vcap_is1_is2_set(vtss_state, vtss_state->eee.loopback_chip_port[chip_no], 0, vtss_state->eee.ena_cnt ? IS2_LOOKUP_SECOND : IS2_LOOKUP_NONE));
    }

    if (vtss_state->eee.ena_cnt) {
        // Add block of default entries
        memset(&data, 0, sizeof(data));
        memset(&entry, 0, sizeof(entry));
        data.u.is2.entry = &entry;
        for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
            entry.chip_port_mask[chip_no] = VTSS_BIT(vtss_state->eee.loopback_chip_port[chip_no]);
        }
        ace->action.port_no     = VTSS_PORT_NO_NONE;
        ace->action.irq_trigger = TRUE;
        ace->id                 = id;
        VTSS_RC(vtss_jr1_is2_add_any(vtss_state, VTSS_IS2_USER_EEE, id, VTSS_VCAP_ID_LAST, &data, 1));
    } else {
        // Delete block of default entries
        VTSS_RC(vtss_jr1_is2_del_any(vtss_state, VTSS_IS2_USER_EEE, id, 1));
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_eee_port_conf_set(vtss_state_t *vtss_state,
                                    const vtss_port_no_t port_no,
                                    const vtss_eee_port_conf_t *const eee_conf)
{
    vtss_chip_no_t   chip_no;
    u32              loopback_port;
    vtss_port_conf_t port_conf;
    BOOL             reconfigure;

    if (vtss_state->eee.ena[port_no] == eee_conf->eee_ena) {
        return VTSS_RC_OK;
    }

    vtss_state->eee.ena[port_no] = eee_conf->eee_ena;

    if (eee_conf->eee_ena) {
        reconfigure = vtss_state->eee.ena_cnt == 0;
        vtss_state->eee.ena_cnt++;
    } else {
        vtss_state->eee.ena_cnt--;
        reconfigure = vtss_state->eee.ena_cnt == 0;
    }

    if (!reconfigure) {
        return VTSS_RC_OK;
    }

    // If we're still here, there's a state change.
    for (chip_no = 0; chip_no < vtss_state->chip_count; chip_no++) {
        VTSS_SELECT_CHIP(chip_no);

        // EEE needs one loopback port per chip.
        // Search for available ports in the port map.
        VTSS_RC(jr_eee_loopback_port(vtss_state));

        loopback_port = vtss_state->eee.loopback_chip_port[vtss_state->chip_no];

        // For EEE, we need Tx mirroring (direction == 1).
        // Set all other fields than ANA_AC:MIRROR_PROBE:PROBE_CFG.PROBE_DIRECTION to 0.
        JR_WRX(ANA_AC, MIRROR_PROBE_PROBE_CFG, JR_MIRROR_PROBE_EEE, VTSS_F_ANA_AC_MIRROR_PROBE_PROBE_CFG_PROBE_DIRECTION(1));

        // No ports are mirrored yet.
        JR_WRX(ANA_AC, MIRROR_PROBE_PROBE_PORT_CFG, JR_MIRROR_PROBE_EEE, 0);

        // Arbiter. Configure mirror port in arbiter and set type to 'Tx'.
        JR_WRX(ARB, CFG_STATUS_MIRROR_CFG, JR_MIRROR_PROBE_EEE, VTSS_F_ARB_CFG_STATUS_MIRROR_CFG_MIRROR_PROBE_CFG(loopback_port) | VTSS_F_ARB_CFG_STATUS_MIRROR_CFG_MIRROR_PROBE_TYPE);

        // Set-up port.
        memset(&port_conf, 0, sizeof(port_conf));
        port_conf.fdx              = 1;
        port_conf.max_frame_length = VTSS_MAX_FRAME_LENGTH_MAX;
        port_conf.if_type          = VTSS_PORT_INTERFACE_LOOPBACK;
        port_conf.power_down       = vtss_state->eee.ena_cnt == 0;
        port_conf.speed            = VTSS_SPEED_100M;
        VTSS_RC(vtss_jr1_port_setup(vtss_state, VTSS_PORT_NO_NONE, loopback_port, &port_conf, 0));

        // vtss_jr1_port_setup() modifies the DSM's rate to defaults for a 100Mbps port.
        // We need to change that to approx. 100Mbps. Units are 15.626Mbps, so with 6, we get 93.75 Mbps.
        JR_WRX(DSM, CFG_RATE_CTRL, loopback_port,
           VTSS_F_DSM_CFG_RATE_CTRL_FRM_GAP_COMP(12) |
           VTSS_F_DSM_CFG_RATE_CTRL_TAXI_RATE_HIGH(6) |
           VTSS_F_DSM_CFG_RATE_CTRL_TAXI_RATE_LOW(6));

        // Loop-back on the taxi bus (the earliest possible loopback position).
        // In this way, we avoid setting up the MAC.
        if (VTSS_PORT_IS_10G(loopback_port)) {
#define VTSS_DEV10G_DEV_CFG_STATUS_DEV_LB_CFG(target)            VTSS_IOREG(target,0x62)
#define VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_LB_CFG_TAXI_HOST_LB_ENA  VTSS_BIT(0)
            JR_WRXB(DEV10G, DEV_CFG_STATUS_DEV_LB_CFG, VTSS_TO_DEV(loopback_port), TAXI_HOST_LB_ENA, vtss_state->eee.ena_cnt ? 1 : 0);
            // Set the rate ctrl watermark to minimize the latency from frame arrival to analyzer interrupt.
            JR_WRX(DSM, CFG_RATE_CTRL_WM_DEV10G, (loopback_port - 27), 0x80);
        } else {
            JR_WRXB(DEV1G,  DEV_CFG_STATUS_DEV_LB_CFG, VTSS_TO_DEV(loopback_port), TAXI_HOST_LB_ENA, vtss_state->eee.ena_cnt ? 1 : 0);
            // Set the rate ctrl watermark to minimize the latency from frame arrival to analyzer interrupt.
            JR_WRX(DSM, CFG_RATE_CTRL_WM_DEV2G5, (loopback_port - 18), 0x80);
        }

        // Setup logical port numbers
        JR_WRX(DSM, CM_CFG_LPORT_NUM_CFG, loopback_port, loopback_port);
        JR_WRX(ANA_CL_2, PORT_PORT_ID_CFG, loopback_port, JR_PUT_FLD(ANA_CL_2, PORT_PORT_ID_CFG, LPORT_NUM, loopback_port));
    }

    // Create IS2 rules on the loopback ports that kill all frames,
    // but generates an interrupt to the CPU.
    VTSS_RC(jr_eee_ace_add_remove(vtss_state));

    return VTSS_RC_OK;
}

static vtss_rc jr_eee_port_state_set(const vtss_state_t          *vtss_state,
                                     const vtss_port_no_t        port_no,
                                     const vtss_eee_port_state_t *const eee_state)
{
    u32            chip_port = VTSS_CHIP_PORT_FROM_STATE(vtss_state, port_no);
    vtss_chip_no_t chip_no   = VTSS_CHIP_NO_FROM_STATE  (vtss_state, port_no);

    switch (eee_state->select) {
    case VTSS_EEE_STATE_SELECT_LPI: {
// Register defines not available in the general JR register header files.
#define VTSS_DEV1G_PCS1G_CFG_STATUS_PCS1G_LPI_CFG(target)            VTSS_IOREG(target, 0x23)
#define VTSS_F_DEV1G_PCS1G_CFG_STATUS_PCS1G_LPI_CFG_TX_ASSERT_LPIDLE VTSS_BIT(0)
        JR_WRXB_CHIP(chip_no, DEV1G, PCS1G_CFG_STATUS_PCS1G_LPI_CFG, VTSS_TO_DEV(chip_port), TX_ASSERT_LPIDLE, eee_state->val ? 1 : 0);
        break;
    }

    case VTSS_EEE_STATE_SELECT_SCH:
        JR_WRXF_CHIP(chip_no, SCH, QSIF_QSIF_ETH_PORT_CTRL, chip_port, ETH_TX_DIS, eee_state->val ? 0 : 0xFF);
        break;

    case VTSS_EEE_STATE_SELECT_FP: {
        u32 one_hot = VTSS_BIT(chip_port);
        u32 mask;
        JR_RDX_CHIP(chip_no, ANA_AC, MIRROR_PROBE_PROBE_PORT_CFG, JR_MIRROR_PROBE_EEE, &mask);
        if (eee_state->val) {
            mask |= one_hot;
        } else {
            mask &= ~one_hot;
        }
        JR_WRX_CHIP(chip_no, ANA_AC, MIRROR_PROBE_PROBE_PORT_CFG, JR_MIRROR_PROBE_EEE, mask);
        break;
    }

    case VTSS_EEE_STATE_SELECT_INTR_ACK: {
        JR_WR_CHIP(eee_state->val, ANA_L2, COMMON_INTR, VTSS_F_ANA_L2_COMMON_INTR_VCAP_S2_INTR);
        break;
    }

    case VTSS_EEE_STATE_SELECT_INTR_ENA: {
        JR_WRB_CHIP(eee_state->val, ANA_L2, COMMON_INTR_ENA, VCAP_S2_INTR_ENA, 1);
        break;
    }

    default:
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

static vtss_rc jr_eee_port_counter_get(const vtss_state_t      *vtss_state,
                                       const vtss_port_no_t    port_no,
                                       vtss_eee_port_counter_t *const eee_counter)
{
    u32            chip_port = VTSS_CHIP_PORT_FROM_STATE(vtss_state, port_no);
    vtss_chip_no_t chip_no   = VTSS_CHIP_NO_FROM_STATE(vtss_state, port_no);
    u8             qu;
    u32            cell_cnt;

// Define register not defined in header files
#define VTSS_OQS_QU_MEM_RAM_STATUS_QU_MEM_ALLOC_STATUS(ri)  VTSS_IOREG(VTSS_TO_OQS, 0x8c00 + (ri))
#define  VTSS_F_OQS_QU_MEM_RAM_STATUS_QU_MEM_ALLOC_STATUS_QU_MEM_ALLOC(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_OQS_QU_MEM_RAM_STATUS_QU_MEM_ALLOC_STATUS_QU_MEM_ALLOC     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_OQS_QU_MEM_RAM_STATUS_QU_MEM_ALLOC_STATUS_QU_MEM_ALLOC(x)  VTSS_EXTRACT_BITFIELD(x,0,15)

    if (eee_counter->fill_level_get) {
        eee_counter->fill_level = 0;

        for (qu = 0; qu < 8; qu++) {
            JR_RDXF_CHIP(chip_no, OQS, QU_MEM_RAM_STATUS_QU_MEM_ALLOC_STATUS, 8 * chip_port + qu, QU_MEM_ALLOC, &cell_cnt);
            eee_counter->fill_level += cell_cnt * 160;
            if (eee_counter->fill_level > eee_counter->fill_level_thres) {
                break;
            }
        }
    }

    if (eee_counter->tx_out_bytes_get) {
        JR_RDX_CHIP(chip_no, ASM, DEV_STATISTICS_TX_OUT_BYTES_CNT, chip_port, &eee_counter->tx_out_bytes);
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_EEE */

static vtss_rc jr_poll_1sec(vtss_state_t *vtss_state)
{
    /* Poll function groups */
    return vtss_jr1_init_groups(vtss_state, VTSS_INIT_CMD_POLL);
}

/* - Debug print --------------------------------------------------- */

#define JR_DEBUG_GPIO(pr, addr, name) JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, GPIO_GPIO_##addr, "GPIO_"name)
#define JR_DEBUG_SIO(pr, addr, name) JR_DEBUG_REG_NAME(pr, DEVCPU_GCB, SIO_CTRL_SIO_##addr, "SIO_"name)
#define JR_DEBUG_SIO_INST(pr, addr, i, name) vtss_jr1_debug_reg_inst(vtss_state, pr, VTSS_DEVCPU_GCB_SIO_CTRL_SIO_##addr, i, "SIO_"name)

static void jr_debug_tgt(const vtss_debug_printf_t pr, const char *name, u32 to)
{
    u32 tgt = ((to >> 16) & 0xff);
    pr("%-12s  0x%02x (%u)\n", name, tgt, tgt);
}

#define JR_DEBUG_TGT(pr, name) jr_debug_tgt(pr, #name, VTSS_TO_##name)

static vtss_rc jr_debug_init(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info)
{
    vtss_jr1_debug_reg_header(pr, "ICPU/PI");
#if VTSS_OPT_VCORE_III
    if (vtss_state->chip_no == 0) {
        JR_DEBUG_REG_NAME(pr, ICPU_CFG, CPU_SYSTEM_CTRL_GENERAL_CTRL, "GENERAL_CTRL");
        JR_DEBUG_REG_NAME(pr, ICPU_CFG, PI_MST_PI_MST_CFG, "PI_MST_CFG");
        JR_DEBUG_REGX_NAME(pr, ICPU_CFG, PI_MST_PI_MST_CTRL, 0, "PI_MST_CTRL");
    }
#endif /* VTSS_OPT_VCORE_III */
    JR_DEBUG_REG_NAME(pr, DEVCPU_PI, PI_PI_MODE, "PI_MODE");
    pr("\n");

    return VTSS_RC_OK;
}

static vtss_rc jr_debug_misc(vtss_state_t *vtss_state,
                             const vtss_debug_printf_t pr,
                             const vtss_debug_info_t   *const info)
{
    u32  port, i, g;
    char name[32];
    
    pr("Name          Target\n");
    JR_DEBUG_TGT(pr, DEVCPU_ORG);
    JR_DEBUG_TGT(pr, DEVCPU_GCB);
    JR_DEBUG_TGT(pr, DEVCPU_QS);
    JR_DEBUG_TGT(pr, DEVCPU_PI);
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        sprintf(name, "DEV_%s_%u",
                port == VTSS_PORT_NPI ? "RGMII" : 
                VTSS_PORT_IS_10G(port) ? "10G" : port < 18 ? "1G" : "2G5",
                port == VTSS_PORT_NPI ? 0 : VTSS_PORT_IS_10G(port) ? (port - 27) :
                port < 18 ? port : (port - 18));
        jr_debug_tgt(pr, name, VTSS_TO_DEV(port));
    }
    JR_DEBUG_TGT(pr, ASM);
    JR_DEBUG_TGT(pr, ANA_CL);
    JR_DEBUG_TGT(pr, LRN);
    JR_DEBUG_TGT(pr, VCAP_IS0);
    JR_DEBUG_TGT(pr, VCAP_IS1);
    JR_DEBUG_TGT(pr, VCAP_IS2);
    JR_DEBUG_TGT(pr, ARB);
    JR_DEBUG_TGT(pr, SCH);
    JR_DEBUG_TGT(pr, DSM);
    JR_DEBUG_TGT(pr, HSIO);
    JR_DEBUG_TGT(pr, VCAP_ES0);
    JR_DEBUG_TGT(pr, ANA_L3);
    JR_DEBUG_TGT(pr, ANA_L2);
    JR_DEBUG_TGT(pr, ANA_AC);
    JR_DEBUG_TGT(pr, IQS);
    JR_DEBUG_TGT(pr, OQS);
    JR_DEBUG_TGT(pr, REW);
    pr("\n");

    vtss_jr1_debug_reg_header(pr, "GPIOs");
    JR_DEBUG_GPIO(pr, OUT, "OUT");
    JR_DEBUG_GPIO(pr, IN, "IN");
    JR_DEBUG_GPIO(pr, OE, "OE");
    JR_DEBUG_GPIO(pr, INTR, "INTR");
    JR_DEBUG_GPIO(pr, INTR_ENA, "INTR_ENA");
    JR_DEBUG_GPIO(pr, INTR_IDENT, "INTR_IDENT");
    JR_DEBUG_GPIO(pr, ALT(0), "ALT_0");
    JR_DEBUG_GPIO(pr, ALT(1), "ALT_1");
    pr("\n");

    for (g=0; g<2; g++) {
        sprintf(name, "SGPIOs Group:%u", g);
        vtss_jr1_debug_reg_header(pr, name);
        for (i = 0; i < 4; i++)
            JR_DEBUG_SIO_INST(pr, INPUT_DATA(g,i), i, "INPUT_DATA");
        for (i = 0; i < 4; i++)
            JR_DEBUG_SIO_INST(pr, INT_POL(g,i), i, "INT_POL");
        JR_DEBUG_SIO(pr, PORT_INT_ENA(g), "PORT_INT_ENA");
        for (i = 0; i < 32; i++)
            JR_DEBUG_SIO_INST(pr, PORT_CONFIG(g,i), i, "PORT_CONFIG");
        JR_DEBUG_SIO(pr, PORT_ENABLE(g), "PORT_ENABLE");
        JR_DEBUG_SIO(pr, CONFIG(g), "CONFIG");
        JR_DEBUG_SIO(pr, CLOCK(g), "CLOCK");
        for (i = 0; i < 4; i++)
            JR_DEBUG_SIO_INST(pr, INT_REG(g,i), i, "INT_REG");
        pr("\n");
    }
    
    return VTSS_RC_OK;
}

vtss_rc vtss_jr1_misc_debug_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_INIT, jr_debug_init, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_MISC, jr_debug_misc, vtss_state, pr, info));
    return VTSS_RC_OK;
}

/* - Initialization ------------------------------------------------ */

vtss_rc vtss_jr1_misc_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_misc_state_t *state = &vtss_state->misc;
    
    switch (cmd) {
    case VTSS_INIT_CMD_CREATE:
        state->reg_read = jr_reg_read;
        state->reg_write = jr_reg_write;
        state->gpio_mode = vtss_jr1_gpio_mode;
        state->gpio_read = jr_gpio_read;
        state->gpio_write = jr_gpio_write;
        state->gpio_event_poll = jr_gpio_event_poll;
        state->gpio_event_enable = jr_gpio_event_enable;
        state->chip_id_get = jr_chip_id_get;
        state->intr_sticky_clear = jr_intr_sticky_clear;
        state->poll_1sec = jr_poll_1sec;
        state->sgpio_conf_set = jr_sgpio_conf_set;
        state->sgpio_read = jr_sgpio_read;
        state->sgpio_event_enable = jr_sgpio_event_enable;
        state->sgpio_event_poll = jr_sgpio_event_poll;
        state->ptp_event_poll = jr_ptp_event_poll;
        state->ptp_event_enable = jr_ptp_event_enable;
        state->dev_all_event_poll = jr_dev_all_event_poll;
        state->dev_all_event_enable = jr_dev_all_event_enable;
        state->gpio_count = JR_GPIOS;
        state->sgpio_group_count = JR_SGPIO_GROUPS;
#ifdef VTSS_FEATURE_EEE
    /* EEE */
        vtss_state->eee.port_conf_set    = jr_eee_port_conf_set;
        vtss_state->eee.port_state_set   = jr_eee_port_state_set;
        vtss_state->eee.port_counter_get = jr_eee_port_counter_get;
#endif
        vtss_state->fan.chip_temp_get = jr_temp_sensor_read;
        vtss_state->fan.chip_temp_init = jr_temp_sensor_init;
        break;
    case VTSS_INIT_CMD_INIT:
        break;
    case VTSS_INIT_CMD_POLL:
        VTSS_RC(jr_poll_sgpio_1sec(vtss_state));
        break;
    default:
        break;
    }
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_JAGUAR_1 */
