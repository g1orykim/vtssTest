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

#include "vtss_serval_cil.h"

#if defined(VTSS_ARCH_SERVAL)

// Avoid Lint Warning 572: Excessive shift value (precision 1 shifted right by 2), which occurs
// in this file because (t) - VTSS_IO_ORIGIN1_OFFSET == 0 for t = VTSS_TO_CFG (i.e. ICPU_CFG), and 0 >> 2 gives a lint warning.
/*lint --e{572} */

/* - CIL functions ------------------------------------------------- */

/* =================================================================
 *  EEE - Energy Efficient Ethernet
 * =================================================================*/
static vtss_rc srvl_eee_port_conf_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t       port_no, 
                                      const vtss_eee_port_conf_t *const conf)
{
    u32            closest_match_index, closest_match, i, requested_time;
    u32            eee_cfg_reg = 0x0; // SYS::EEE_CFG register value.
    vtss_port_no_t chip_port = VTSS_CHIP_PORT(port_no);

    // The formula for the actual wake-up time given a
    // register value is non-linear (though periodic with
    // increasing base values).
    // The easiest way to find the closest match to a user-specified
    // value is to create a static lookup table that will have to be
    // traversed everytime.
    if (!vtss_state->eee.timer_table_initialized) {
        vtss_state->eee.timer_table_initialized = TRUE;
        for (i = 0; i < VTSS_EEE_TIMER_TABLE_CNT; i++) {
            vtss_state->eee.timer_table[i] = (1 << (2 * (i / 16UL))) * (i % 16UL);
        }
    }

    // Make sure that we don't get out of bound
    if (port_no >= VTSS_PORT_ARRAY_SIZE) {
        VTSS_E("Out of bounds: chip_port:%d, VTSS_PORT_ARRAY_SIZE:%d", port_no, VTSS_PORT_ARRAY_SIZE);
        return VTSS_RC_ERROR;
    }

    // EEE enable
    if (conf->eee_ena) {
        //LPI is really an old error code redefined to tell the link partner to go to
        // lower power or when removed to wakeup.
        // Some devices are seeing the error code and assuming there is a
        // problem causing the link to go down. A work around is to only enable EEE in the MAC (No LPI signal to the PHY)
        // when the PHY has auto negotiated and have found that the link partner supports EEE.
        if (conf->lp_advertisement == 0) {
            VTSS_I("Link partner doesn't support EEE - Keeping EEE disabled. Port:%d", chip_port);
        } else if (!(vtss_state->phy_state[port_no].status.fdx)) {
            // EEE and Half duplex are not supposed to work together, so we disables EEE in the case where the port is in HDX mode.
            VTSS_I("EEE disabled due to that port is in HDX mode, port:%d, fdx:%d", chip_port, vtss_state->phy_state[port_no].status.fdx);
        } else {
            eee_cfg_reg |= VTSS_F_DEV_PORT_MODE_EEE_CFG_EEE_ENA;
        }
    }

    // EEE wakeup timer (See datasheet for understanding calculation)
    closest_match_index = 0;
    closest_match       = 0xFFFFFFFFUL;
    requested_time      = conf->tx_tw;
    for (i = 0; i< VTSS_EEE_TIMER_TABLE_CNT; i++) {
        u32 table_val = vtss_state->eee.timer_table[i];
        if (table_val >= requested_time) {
            u32 diff = table_val - requested_time;
            if (diff < closest_match) {
                closest_match       = diff;
                closest_match_index = i;
                if (closest_match == 0) {
                    break;
                }
            }
        }
    }

    if (closest_match == 0xFFFFFFFFUL) {
        closest_match_index = 127;
    }

    eee_cfg_reg |= VTSS_F_DEV_PORT_MODE_EEE_CFG_EEE_TIMER_WAKEUP(closest_match_index);


    // EEE holdoff timer
    eee_cfg_reg |= VTSS_F_DEV_PORT_MODE_EEE_CFG_EEE_TIMER_HOLDOFF(0x5) | VTSS_F_DEV_PORT_MODE_EEE_CFG_EEE_TIMER_AGE(0x23);


    // Registers write
    SRVL_WR(VTSS_DEV_PORT_MODE_EEE_CFG(VTSS_TO_DEV(chip_port)), eee_cfg_reg);

    // EEE fast queues
    eee_cfg_reg = VTSS_F_QSYS_SYSTEM_EEE_CFG_EEE_FAST_QUEUES(conf->eee_fast_queues);

    VTSS_N("eee_cfg_reg = 0x%X, conf->tx_tw = %d", eee_cfg_reg, conf->tx_tw);
    SRVL_WR(VTSS_QSYS_SYSTEM_EEE_CFG(chip_port), eee_cfg_reg);

    // Setting Buffer size to 3 Kbyte & 255 frames.
    SRVL_WR(VTSS_QSYS_SYSTEM_EEE_THRES, 0x3EFF);

    return VTSS_RC_OK;
}

/* =================================================================
 * FAN speed control
 * =================================================================*/
static vtss_rc srvl_fan_controller_init(vtss_state_t *vtss_state,
                                        const vtss_fan_conf_t *const spec)
{
    // Set GPIO alternate functions. PWM is bit 5.
    (void) vtss_srvl_gpio_mode(vtss_state, 0, 5, VTSS_GPIO_ALT_0);

    // Set PWM frequency 
    SRVL_WRM(VTSS_DEVCPU_GCB_FAN_CFG_FAN_CFG,
             VTSS_F_DEVCPU_GCB_FAN_CFG_FAN_CFG_PWM_FREQ(spec->fan_pwm_freq),
             VTSS_M_DEVCPU_GCB_FAN_CFG_FAN_CFG_PWM_FREQ);

    // Set PWM polarity 
    SRVL_WRM(VTSS_DEVCPU_GCB_FAN_CFG_FAN_CFG,
             spec->fan_low_pol ? VTSS_F_DEVCPU_GCB_FAN_CFG_FAN_CFG_INV_POL : 0,
             VTSS_F_DEVCPU_GCB_FAN_CFG_FAN_CFG_INV_POL);
    
    // Set PWM open collector 
    SRVL_WRM(VTSS_DEVCPU_GCB_FAN_CFG_FAN_CFG,
             spec->fan_open_col ? VTSS_F_DEVCPU_GCB_FAN_CFG_FAN_CFG_INV_POL : 0,
             VTSS_F_DEVCPU_GCB_FAN_CFG_FAN_CFG_PWM_OPEN_COL_ENA);
    
    // Set fan speed measurement
    if (spec->type == VTSS_FAN_3_WIRE_TYPE) {
        // Enable gating for 3-wire fan types.
        SRVL_WRM(VTSS_DEVCPU_GCB_FAN_CFG_FAN_CFG,
                 1,
                 VTSS_F_DEVCPU_GCB_FAN_CFG_FAN_CFG_GATE_ENA);
    } else {
        //  For 4-wire fan types we need to disable gating (2-wire types doesn't matter)
        SRVL_WRM(VTSS_DEVCPU_GCB_FAN_CFG_FAN_CFG,
                 0,
                 VTSS_F_DEVCPU_GCB_FAN_CFG_FAN_CFG_GATE_ENA);
    }
    

    // Set GPIO alternate functions. ROTA is bit 4.
    (void) vtss_srvl_gpio_mode(vtss_state, 0, 4, VTSS_GPIO_ALT_0);
 
    return VTSS_RC_OK;
}

static vtss_rc srvl_fan_cool_lvl_set(vtss_state_t *vtss_state, u8 lvl)
{
    SRVL_WRM(VTSS_DEVCPU_GCB_FAN_CFG_FAN_CFG,
             VTSS_F_DEVCPU_GCB_FAN_CFG_FAN_CFG_DUTY_CYCLE(lvl),
             VTSS_M_DEVCPU_GCB_FAN_CFG_FAN_CFG_DUTY_CYCLE);

    return VTSS_RC_OK;
}

static vtss_rc srvl_fan_cool_lvl_get(vtss_state_t *vtss_state, u8 *duty_cycle)
{
    u32 fan_cfg_reg;

    // Read the register 
    SRVL_RD(VTSS_DEVCPU_GCB_FAN_CFG_FAN_CFG,&fan_cfg_reg);

    // Get PWM duty cycle
    *duty_cycle = VTSS_X_DEVCPU_GCB_FAN_CFG_FAN_CFG_DUTY_CYCLE(fan_cfg_reg);

    return VTSS_RC_OK;
}

static vtss_rc srvl_fan_rotation_get(vtss_state_t *vtss_state, vtss_fan_conf_t *fan_spec, u32 *rotation_count)
{
    // Read the register 
    SRVL_RD(VTSS_DEVCPU_GCB_FAN_STAT_FAN_CNT,rotation_count);
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Miscellaneous
 * ================================================================= */

static vtss_rc srvl_reg_read(vtss_state_t *vtss_state,
                             const vtss_chip_no_t chip_no, const u32 addr, u32 * const value)
{
    return vtss_srvl_rd(vtss_state, addr, value);
}

static vtss_rc srvl_reg_write(vtss_state_t *vtss_state,
                              const vtss_chip_no_t chip_no, const u32 addr, const u32 value)
{
    return vtss_srvl_wr(vtss_state, addr, value);
}

vtss_rc vtss_srvl_chip_id_get(vtss_state_t *vtss_state, vtss_chip_id_t *const chip_id)
{
    u32 value;

    SRVL_RD(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID, &value);

    if (value == 0 || value == 0xffffffff) {
        VTSS_E("CPU interface error, chipid: 0x%08x", value);
        return VTSS_RC_ERROR;
    }

    chip_id->part_number = VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(value);
    chip_id->revision = VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(value);

    return VTSS_RC_OK;
}

static vtss_rc srvl_ptp_event_poll(vtss_state_t *vtss_state, vtss_ptp_event_type_t *ev_mask)
{
    u32 sticky, mask;

    /* PTP events */
    *ev_mask = 0;
    SRVL_RD(VTSS_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT, &sticky);
    SRVL_WR(VTSS_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT, sticky);
    SRVL_RD(VTSS_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG, &mask);
    mask |= VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_CLK_ADJ_UPD_STICKY; /* CLK ADJ event has no enable bit - do not generate interrupt */
    sticky &= mask;      /* Only handle enabled sources */

    *ev_mask |= (sticky & VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_SYNC_STAT) ? VTSS_PTP_SYNC_EV : 0;
    *ev_mask |= (sticky & VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_EXT_SYNC_CURRENT_TIME_STICKY(1)) ? VTSS_PTP_EXT_SYNC_EV : 0;
    *ev_mask |= (sticky & VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_EXT_SYNC_CURRENT_TIME_STICKY(2)) ? VTSS_PTP_EXT_1_SYNC_EV : 0;
    *ev_mask |= (sticky & VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_CLK_ADJ_UPD_STICKY) ? VTSS_PTP_CLK_ADJ_EV : 0;
    VTSS_D("sticky: 0x%x, ev_mask 0x%x", sticky, *ev_mask);

    return VTSS_RC_OK;
}

static vtss_rc srvl_ptp_event_enable(vtss_state_t *vtss_state,
                                     vtss_ptp_event_type_t ev_mask, BOOL enable)
{
    /* PTP masks */

    if (ev_mask & VTSS_PTP_SYNC_EV) {
        SRVL_WRM(VTSS_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG,
                enable ? VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_SYNC_STAT_ENA : 0,
                VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_SYNC_STAT_ENA);
    }
    if (ev_mask & VTSS_PTP_EXT_SYNC_EV) {
        SRVL_WRM(VTSS_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG,
                enable ? VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_EXT_SYNC_CURRENT_TIME_ENA(1) : 0,
                VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_EXT_SYNC_CURRENT_TIME_ENA(1));
    }
    if (ev_mask & VTSS_PTP_EXT_1_SYNC_EV) {
        SRVL_WRM(VTSS_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG,
                 enable ? VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_EXT_SYNC_CURRENT_TIME_ENA(2) : 0,
                 VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_EXT_SYNC_CURRENT_TIME_ENA(2));
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_intr_cfg(vtss_state_t *vtss_state,
                             const u32  intr_mask, const BOOL polarity, const BOOL enable)
{
    return VTSS_RC_OK;
}

static vtss_rc srvl_intr_pol_negation(vtss_state_t *vtss_state)
{
    return VTSS_RC_OK;
}

#ifdef VTSS_FEATURE_IRQ_CONTROL

#define SRVL1_IRQ_DEV_ALL       (0)
#define SRVL1_IRQ_EXT0          (1)
#define SRVL1_IRQ_EXT1          (2)
#define SRVL1_IRQ_TIMER0        (3)
#define SRVL1_IRQ_TIMER1        (4)
#define SRVL1_IRQ_TIMER2        (5)
#define SRVL1_IRQ_UART          (6)
#define SRVL1_IRQ_UART2         (7)
#define SRVL1_IRQ_TWI           (8)
#define SRVL1_IRQ_SW0           (9)
#define SRVL1_IRQ_SW1           (10)
#define SRVL1_IRQ_SGPIO         (11)
#define SRVL1_IRQ_GPIO          (12)
#define SRVL1_IRQ_MIIM0_INTR    (13)
#define SRVL1_IRQ_MIIM1_INTR    (14)
#define SRVL1_IRQ_FDMA          (15)
#define SRVL1_IRQ_OAM_MEP       (16)
#define SRVL1_IRQ_PTP_RDY       (17)
#define SRVL1_IRQ_PTP_SYNC      (18)
#define SRVL1_IRQ_INTEGRITY     (19)
#define SRVL1_IRQ_XTR_RDY0      (20)
#define SRVL1_IRQ_XTR_RDY1      (21)
#define SRVL1_IRQ_INJ_RDY0      (22)
#define SRVL1_IRQ_INJ_RDY1      (23)
#define SRVL1_IRQ_PCIE          (24)

#define SRVL_IRQ_DEST_CPU0      0 /* IRQ destination CPU0 */
#define SRVL_IRQ_DEST_CPU1      1 /* IRQ destination CPU1 */
#define SRVL_IRQ_DEST_EXT0      2 /* IRQ destination EXT0 */
#define SRVL_IRQ_DEST_EXT1      3 /* IRQ destination EXT1 */

static vtss_rc srvl_misc_irq_external(struct vtss_state_s *vtss_state, 
                                      u32 mask,
                                      BOOL external)
{
    u32 dest;
    if (external) {
        SRVL_WRM_CLR(VTSS_ICPU_CFG_INTR_DST_INTR_MAP(SRVL_IRQ_DEST_CPU0), mask);
    } else {
        SRVL_WRM_SET(VTSS_ICPU_CFG_INTR_DST_INTR_MAP(SRVL_IRQ_DEST_CPU0), mask);
    }
    //SRVL_WR(VTSS_ICPU_CFG_INTR_DST_INTR_MAP(SRVL_IRQ_DEST_CPU1), 0);
    //SRVL_WR(VTSS_ICPU_CFG_INTR_DST_INTR_MAP(SRVL_IRQ_DEST_EXT0), 0);
    if (external) {
        SRVL_WRM_SET(VTSS_ICPU_CFG_INTR_DST_INTR_MAP(SRVL_IRQ_DEST_EXT1), mask);
    } else {
        SRVL_WRM_CLR(VTSS_ICPU_CFG_INTR_DST_INTR_MAP(SRVL_IRQ_DEST_EXT1), mask);
    }

    /* Always use sticky IRQ's */
    SRVL_WRM_CLR(VTSS_ICPU_CFG_INTR_INTR_BYPASS, mask);

    SRVL_RD(VTSS_ICPU_CFG_INTR_DST_INTR_MAP(SRVL_IRQ_DEST_EXT1), &dest);
    if (vtss_state->sys_config.using_pcie) {
        if (dest) {
            SRVL_WRM_SET(VTSS_ICPU_CFG_PCIe_PCIE_INTR_CFG(1), 
                         VTSS_F_ICPU_CFG_PCIe_PCIE_INTR_CFG_INTR_FALLING_ENA|
                         VTSS_F_ICPU_CFG_PCIe_PCIE_INTR_CFG_INTR_RISING_ENA);
            SRVL_WRM_SET(VTSS_ICPU_CFG_PCIe_PCIE_INTR_COMMON_CFG, VTSS_F_ICPU_CFG_PCIe_PCIE_INTR_COMMON_CFG_PCIE_INTR_ENA);
        } else {
            SRVL_WRM_CLR(VTSS_ICPU_CFG_PCIe_PCIE_INTR_CFG(1), 
                         VTSS_F_ICPU_CFG_PCIe_PCIE_INTR_CFG_INTR_FALLING_ENA|
                         VTSS_F_ICPU_CFG_PCIe_PCIE_INTR_CFG_INTR_RISING_ENA);
            SRVL_WRM_CLR(VTSS_ICPU_CFG_PCIe_PCIE_INTR_COMMON_CFG, VTSS_F_ICPU_CFG_PCIe_PCIE_INTR_COMMON_CFG_PCIE_INTR_ENA);
        }
    }
    /* XXX: This explicitly only use EXT1 ... Must check */
    if (dest) {
        SRVL_WRM(VTSS_ICPU_CFG_INTR_EXT_INTR_DRV, 
                 VTSS_F_ICPU_CFG_INTR_EXT_INTR_DRV_EXT_INTR_DRV(1),
                 VTSS_M_ICPU_CFG_INTR_EXT_INTR_DRV_EXT_INTR_DRV);
        SRVL_WRM(VTSS_ICPU_CFG_INTR_EXT_INTR_DIR,
                 VTSS_F_ICPU_CFG_INTR_EXT_INTR_DIR_EXT_INTR_DIR(1),
                 VTSS_M_ICPU_CFG_INTR_EXT_INTR_DIR_EXT_INTR_DIR);
    } else {
        SRVL_WRM(VTSS_ICPU_CFG_INTR_EXT_INTR_DRV, 
                 0,
                 VTSS_M_ICPU_CFG_INTR_EXT_INTR_DRV_EXT_INTR_DRV);
        SRVL_WRM(VTSS_ICPU_CFG_INTR_EXT_INTR_DIR,
                 0,
                 VTSS_M_ICPU_CFG_INTR_EXT_INTR_DIR_EXT_INTR_DIR);
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_misc_irq_cfg(struct vtss_state_s *vtss_state,
                                 const vtss_irq_t irq,
                                 const vtss_irq_conf_t *const conf)
{
    vtss_rc rc;
    switch (irq) {
    case VTSS_IRQ_XTR:
        rc = srvl_misc_irq_external(vtss_state, VTSS_BIT(SRVL1_IRQ_XTR_RDY0)|VTSS_BIT(SRVL1_IRQ_XTR_RDY1), conf->external);
        break;
    case VTSS_IRQ_FDMA_XTR:     /* NB: XTR and INJ are lumped together*/
        rc = srvl_misc_irq_external(vtss_state, VTSS_BIT(SRVL1_IRQ_FDMA), conf->external);
        break;
    case VTSS_IRQ_SOFTWARE:
        rc = srvl_misc_irq_external(vtss_state, VTSS_BIT(SRVL1_IRQ_SW0)|VTSS_BIT(SRVL1_IRQ_SW1), conf->external);
        break;
    case VTSS_IRQ_PTP_RDY:
        rc = srvl_misc_irq_external(vtss_state, VTSS_BIT(SRVL1_IRQ_PTP_RDY), conf->external);
        break;
    case VTSS_IRQ_PTP_SYNC:
        rc = srvl_misc_irq_external(vtss_state, VTSS_BIT(SRVL1_IRQ_PTP_SYNC), conf->external);
        break;
    default:
        rc = VTSS_RC_ERROR;
    }
    return rc;
}

static vtss_rc srvl_misc_irq_status(struct vtss_state_s *vtss_state,
                                    vtss_irq_status_t *status)
{
    u32 val;
    
    memset(status, 0, sizeof(*status));

    SRVL_RD(VTSS_ICPU_CFG_INTR_INTR_IDENT, &val);
    status->raw_ident = val;
    SRVL_RD(VTSS_ICPU_CFG_INTR_INTR_RAW, &status->raw_status);
    SRVL_RD(VTSS_ICPU_CFG_INTR_INTR_ENA, &status->raw_mask);

    if(val & VTSS_BIT(SRVL1_IRQ_FDMA)) {
        u32 man_intr;
        SRVL_RD(VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_INTR, &man_intr);
        if (man_intr & VTSS_F_ICPU_CFG_MANUAL_XTRINJ_MANUAL_INTR_INTR_XTR_ANY_RDY) {
            status->active |= VTSS_BIT(VTSS_IRQ_FDMA_XTR);
        }
    }
    if(val & (VTSS_BIT(SRVL1_IRQ_XTR_RDY0)|VTSS_BIT(SRVL1_IRQ_XTR_RDY1))) {
        status->active |= VTSS_BIT(VTSS_IRQ_XTR);
    }
    if(val & (VTSS_BIT(SRVL1_IRQ_SW0)|VTSS_BIT(SRVL1_IRQ_SW1))) {
        status->active |= VTSS_BIT(VTSS_IRQ_SOFTWARE);
    }
    if(val & (VTSS_BIT(SRVL1_IRQ_PTP_RDY))) {
        status->active |= VTSS_BIT(VTSS_IRQ_PTP_RDY);
    }
    if(val & (VTSS_BIT(SRVL1_IRQ_PTP_SYNC))) {
        status->active |= VTSS_BIT(VTSS_IRQ_PTP_SYNC);
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_misc_irq_enable(struct vtss_state_s *vtss_state,
                                    const vtss_irq_t irq,
                                    const BOOL enable)
{
    u32 mask;
    switch (irq) {
    case VTSS_IRQ_XTR:
        mask = VTSS_BIT(SRVL1_IRQ_XTR_RDY0)|VTSS_BIT(SRVL1_IRQ_XTR_RDY1);
        break;
    case VTSS_IRQ_FDMA_XTR:
        SRVL_WRM_CTL(VTSS_ICPU_CFG_MANUAL_XTRINJ_MANUAL_INTR_ENA, enable, 
                     VTSS_F_ICPU_CFG_MANUAL_XTRINJ_MANUAL_INTR_ENA_INTR_XTR_ANY_RDY_ENA);
        mask = VTSS_BIT(SRVL1_IRQ_FDMA);
        break;
    case VTSS_IRQ_SOFTWARE:
        mask = VTSS_BIT(SRVL1_IRQ_SW0)|VTSS_BIT(SRVL1_IRQ_SW1);
        break;
    case VTSS_IRQ_PTP_RDY:
        mask = VTSS_BIT(SRVL1_IRQ_PTP_RDY);
        break;
    case VTSS_IRQ_PTP_SYNC:
        mask = VTSS_BIT(SRVL1_IRQ_PTP_SYNC);
        break;
    default:
        return VTSS_RC_ERROR;
    }
    /* Ack sticky bits */
    SRVL_WR(VTSS_ICPU_CFG_INTR_INTR_STICKY, mask);
    if (irq == VTSS_IRQ_FDMA_XTR) {
        SRVL_WRM_CTL(VTSS_ICPU_CFG_FDMA_FDMA_INTR_ENA, enable, 1); /* Always using ch 1 */
        SRVL_WR(VTSS_ICPU_CFG_INTR_INTR_ENA_SET, mask); /* Always enable top FDMA (cascading) IRQ */
    } else {
        if(enable) {
            SRVL_WR(VTSS_ICPU_CFG_INTR_INTR_ENA_SET, mask);
        } else {
            SRVL_WR(VTSS_ICPU_CFG_INTR_INTR_ENA_CLR, mask);
        }
    }
    return VTSS_RC_OK;
}
#endif  /* VTSS_FEATURE_IRQ_CONTROL */

/* =================================================================
 *  Miscellaneous - GPIO
 * =================================================================*/

vtss_rc vtss_srvl_gpio_mode(vtss_state_t *vtss_state,
                            const vtss_chip_no_t   chip_no,
                            const vtss_gpio_no_t   gpio_no,
                            const vtss_gpio_mode_t mode)
{
    u32 mask = VTSS_BIT(gpio_no), alt_0 = 0, alt_1 = 0;

    SRVL_WRM_CLR(VTSS_DEVCPU_GCB_GPIO_GPIO_INTR_ENA, mask); /* Disable IRQ */
    switch (mode) {
    case VTSS_GPIO_OUT:
    case VTSS_GPIO_IN:
    case VTSS_GPIO_IN_INT:
        SRVL_WRM_CTL(VTSS_DEVCPU_GCB_GPIO_GPIO_OE, mode == VTSS_GPIO_OUT, mask);
        break;
    case VTSS_GPIO_ALT_0:
        alt_0 = mask;
        break;
    case VTSS_GPIO_ALT_1:
        alt_1 = mask;
        break;
    case VTSS_GPIO_ALT_2:
        alt_0 = mask;
        alt_1 = mask;
        break;
    default:
        VTSS_E("illegal mode");
        return VTSS_RC_ERROR;
    }
    SRVL_WRM(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0), alt_0, mask);
    SRVL_WRM(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1), alt_1, mask);
    if (mode == VTSS_GPIO_IN_INT)
        SRVL_WRM_SET(VTSS_DEVCPU_GCB_GPIO_GPIO_INTR_ENA, mask);
    return VTSS_RC_OK;
}

static vtss_rc srvl_gpio_read(vtss_state_t *vtss_state,
                              const vtss_chip_no_t  chip_no,
                              const vtss_gpio_no_t  gpio_no,
                              BOOL                  *const value)
{
    u32 val, mask = VTSS_BIT(gpio_no);

    SRVL_RD(VTSS_DEVCPU_GCB_GPIO_GPIO_IN, &val);
    *value = VTSS_BOOL(val & mask);
    return VTSS_RC_OK;
}

static vtss_rc srvl_gpio_write(vtss_state_t *vtss_state,
                               const vtss_chip_no_t  chip_no,
                               const vtss_gpio_no_t  gpio_no,
                               const BOOL            value)
{
    u32 mask = VTSS_BIT(gpio_no);

    if (value) {
        SRVL_WR(VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_SET, mask);
    } else {
        SRVL_WR(VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_CLR, mask);
    }
    return VTSS_RC_OK;
}

static vtss_rc srvl_sgpio_event_poll(vtss_state_t *vtss_state,
                                     const vtss_chip_no_t     chip_no,
                                     const vtss_sgpio_group_t group,
                                     const u32                bit,
                                     BOOL                     *const events)
{
    u32 i, val;

    SRVL_RD(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_INT_REG(bit), &val);
    SRVL_WR(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_INT_REG(bit), val);  /* Clear pending */

    /* Setup serial IO port enable register */
    for (i = 0; i < 32; i++) {
        events[i] = VTSS_BOOL(val & (1<<i));
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_sgpio_event_enable(vtss_state_t *vtss_state,
                                       const vtss_chip_no_t     chip_no,
                                       const vtss_sgpio_group_t group,
                                       const u32                port,
                                       const u32                bit,
                                       const BOOL               enable)
{
    u32 data, pol, i;

    if (enable) {
        SRVL_RD(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_INPUT_DATA(bit), &data);
        SRVL_RD(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_INT_POL(bit), &pol);
        pol = ~pol;     /* '0' means interrupt when input is one */
        data &= pol;    /* Now data '1' means active interrupt */
        if (!(data & 1<<port)) {   /* Only enable if not active interrupt - as interrupt pending cannot be cleared */
            SRVL_WRM(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_INT_ENA, 1<<port, 1<<port);
        }
        SRVL_WRM(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG,
                 VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA(1<<bit),
                 VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA(1<<bit));
    }
    else {
        SRVL_WRM(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_INT_ENA, 0, 1<<port);
        for (i = 0; i < 32; ++i) {
            if (vtss_state->misc.sgpio_event_enabled[0][group].enable[i][bit])
                break;
        }
        if (i == 32)
            SRVL_WRM(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG, 0,
                     VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA(1<<bit));
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_sgpio_conf_set(vtss_state_t *vtss_state,
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
    SRVL_WR(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_ENABLE, val);

    /* Setup general configuration register
     *
     * The burst gap is 0x1f(33.55ms)
     * The load signal is active low
     * The auto burst repeat function is on
     * The SIO reverse output is off */
    for (i = 0; i < 2; i++) {
        switch (conf->bmode[i]) {
        case VTSS_SGPIO_BMODE_TOGGLE:
            if (i == 0) {
                VTSS_E("blink mode 0 does not support TOGGLE");
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

    /* Configure "LD" polarity signal to 0 (active low) for input SGPIO */
    SRVL_WRM(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG,
             VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_0(bmode[0]) |
             VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_1(bmode[1]) |
             VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BURST_GAP(0x00) |
             VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_PORT_WIDTH(conf->bit_count - 1) |
             VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_AUTO_REPEAT,
             ~VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA);

    /* Setup the serial IO clock frequency - 12.5MHz (0x14) */
    SRVL_WR(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CLOCK, 0x14);

    /* Configuration of output data values
     * The placement of the source select bits for each output bit in the register:
     * Output bit0 : (2 downto 0)
     * Output bit1 : (5 downto 3)
     * Output bit2 : (8 downto 6)
     * Output bit3 : (11 downto 9) */
    for (port = 0; port < 32; port++) {
        for (val = 0, bit_idx = 0; bit_idx < 4; bit_idx++) {
            /* Set output bit n */
            val |= VTSS_ENCODE_BITFIELD(conf->port_conf[port].mode[bit_idx], bit_idx * 3, 3);
        }
        SRVL_WR(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CONFIG(port), val);
    }

    return VTSS_RC_OK;
}

static vtss_rc srvl_sgpio_read(vtss_state_t *vtss_state,
                               const vtss_chip_no_t     chip_no,
                               const vtss_sgpio_group_t group,
                               vtss_sgpio_port_data_t   data[VTSS_SGPIO_PORTS])
{
    u32 i, port, value;

    for (i = 0; i < 4; i++) {
        SRVL_RD(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_INPUT_DATA(i), &value);
        for (port = 0; port < 32; port++) {
            data[port].value[i] = VTSS_BOOL(value & (1 << port));
        }
    }

    return VTSS_RC_OK;
}

/* - Debug print --------------------------------------------------- */

static void srvl_debug_tgt(const vtss_debug_printf_t pr, const char *name, u32 to)
{
    u32 tgt = ((to >> 16) & 0x3f);
    pr("%-10s  0x%02x (%u)\n", name, tgt, tgt);
}

#define SRVL_DEBUG_TGT(pr, name) srvl_debug_tgt(pr, #name, VTSS_TO_##name)
#define SRVL_DEBUG_GPIO(pr, addr, name) vtss_srvl_debug_reg(vtss_state, pr, VTSS_DEVCPU_GCB_GPIO_GPIO_##addr, "GPIO_"name)
#define SRVL_DEBUG_SIO(pr, addr, name) vtss_srvl_debug_reg(vtss_state, pr, VTSS_DEVCPU_GCB_SIO_CTRL_SIO_##addr, "SIO_"name)
#define SRVL_DEBUG_SIO_INST(pr, addr, i, name) vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_DEVCPU_GCB_SIO_CTRL_SIO_##addr, i, "SIO_"name)

static vtss_rc srvl_debug_misc(vtss_state_t *vtss_state,
                               const vtss_debug_printf_t pr,
                               const vtss_debug_info_t   *const info)
{
    u32  port, i;
    char buf[32];
    
    pr("Name        Target\n");
    SRVL_DEBUG_TGT(pr, DEVCPU_ORG);
    SRVL_DEBUG_TGT(pr, SYS);
    SRVL_DEBUG_TGT(pr, REW);
    SRVL_DEBUG_TGT(pr, ES0);
    SRVL_DEBUG_TGT(pr, S1);
    SRVL_DEBUG_TGT(pr, S2);
    SRVL_DEBUG_TGT(pr, DEVCPU_GCB);
    SRVL_DEBUG_TGT(pr, DEVCPU_QS);
    SRVL_DEBUG_TGT(pr, HSIO);
    SRVL_DEBUG_TGT(pr, IS0);
    SRVL_DEBUG_TGT(pr, OAM_MEP);
    for (port = 0; port < VTSS_CHIP_PORTS; port++) {
        sprintf(buf, "DEV_%u", port);
        srvl_debug_tgt(pr, buf, VTSS_TO_DEV(port));
    }
    pr("\n");
    SRVL_DEBUG_TGT(pr, QSYS);
    SRVL_DEBUG_TGT(pr, ANA);

    vtss_srvl_debug_reg_header(pr, "GPIOs");
    SRVL_DEBUG_GPIO(pr, OUT, "OUT");
    SRVL_DEBUG_GPIO(pr, IN, "IN");
    SRVL_DEBUG_GPIO(pr, OE, "OE");
    SRVL_DEBUG_GPIO(pr, INTR, "INTR");
    SRVL_DEBUG_GPIO(pr, INTR_ENA, "INTR_ENA");
    SRVL_DEBUG_GPIO(pr, INTR_IDENT, "INTR_IDENT");
    SRVL_DEBUG_GPIO(pr, ALT(0), "ALT_0");
    SRVL_DEBUG_GPIO(pr, ALT(1), "ALT_1");
    pr("\n");
    
    vtss_srvl_debug_reg_header(pr, "SGPIO");
    for (i = 0; i < 4; i++)
        SRVL_DEBUG_SIO_INST(pr, INPUT_DATA(i), i, "INPUT_DATA");
    for (i = 0; i < 4; i++)
        SRVL_DEBUG_SIO_INST(pr, INT_POL(i), i, "INT_POL");
    SRVL_DEBUG_SIO(pr, PORT_INT_ENA, "PORT_INT_ENA");
    for (i = 0; i < 32; i++)
        SRVL_DEBUG_SIO_INST(pr, PORT_CONFIG(i), i, "PORT_CONFIG");
    SRVL_DEBUG_SIO(pr, PORT_ENABLE, "PORT_ENABLE");
    SRVL_DEBUG_SIO(pr, CONFIG, "CONFIG");
    SRVL_DEBUG_SIO(pr, CLOCK, "CLOCK");
    for (i = 0; i < 4; i++)
        SRVL_DEBUG_SIO_INST(pr, INT_REG(i), i, "INT_REG");
    pr("\n");
    
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_STAT, "GENERAL_STAT");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_INTR_INTR_IDENT, "INTR_IDENT");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_INTR_INTR_RAW, "INTR_RAW");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_INTR_INTR_ENA, "INTR_ENA");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_INTR_INTR_BYPASS, "INTR_BYPASS");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_FDMA_FDMA_INTR_IDENT, "FDMA_INTR_IDENT");
    vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_FDMA_FDMA_INTR_ENA, "FDMA_INTR_ENA");
    for (i = 0; i < 4; i++) {
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_ICPU_CFG_INTR_DST_INTR_MAP(i), i, "DST_INTR_MAP");
        vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_ICPU_CFG_INTR_DST_INTR_IDENT(i), i, "DST_INTR_IDENT");
    }
    if (vtss_state->sys_config.using_pcie) {
        vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_PCIe_PCIE_CFG, "PCIE_CFG");
        vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_PCIe_PCIE_STAT, "PCIE_STAT");
        vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_PCIe_PCIE_AUX_CFG, "PCIE_AUX_CFG");
        vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_PCIe_PCIE_INTR, "PCIE_INTR");
        vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_PCIe_PCIE_INTR_ENA, "PCIE_INTR_ENA");
        vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_PCIe_PCIE_INTR_IDENT, "PCIE_INTR_IDENT");
        vtss_srvl_debug_reg(vtss_state, pr, VTSS_ICPU_CFG_PCIe_PCIE_INTR_COMMON_CFG, "PCIE_INTR_COMMON_CFG");
        for (i = 0; i < 2; i++)
            vtss_srvl_debug_reg_inst(vtss_state, pr, VTSS_ICPU_CFG_PCIe_PCIE_INTR_CFG(i), i, "PCIE_INTR_CFG");
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_srvl_misc_debug_print(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr,
                                   const vtss_debug_info_t   *const info)
{
    return vtss_debug_print_group(VTSS_DEBUG_GROUP_MISC, srvl_debug_misc, vtss_state, pr, info);
}

/* - Initialization ------------------------------------------------ */

static vtss_rc srvl_misc_poll_1sec(vtss_state_t *vtss_state)
{
    u32 port, bit, enable, rc=VTSS_RC_OK;

    SRVL_RD(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_INT_ENA, &enable);
    for (port=0; port<32; ++port)
        if (!(enable & 1<<port))
            for (bit=0; bit<4; ++bit)    /* port is not enabled - check if it is configured to be */
                if (vtss_state->misc.sgpio_event_enabled[0][0].enable[port][bit]) {
                    rc = srvl_sgpio_event_enable(vtss_state, 0, 0, port, bit, TRUE);  /* this port,bit is configured to be enabled - try and enable */
                }
    return rc;
}

static vtss_rc srvl_poll_1sec(vtss_state_t *vtss_state)
{
    /* Poll function groups */
    return vtss_srvl_init_groups(vtss_state, VTSS_INIT_CMD_POLL);
}

vtss_rc vtss_srvl_misc_init(vtss_state_t *vtss_state, vtss_init_cmd_t cmd)
{
    vtss_misc_state_t *state = &vtss_state->misc;

    if (cmd == VTSS_INIT_CMD_CREATE) {
        state->reg_read = srvl_reg_read;
        state->reg_write = srvl_reg_write;
        state->chip_id_get = vtss_srvl_chip_id_get;
        state->poll_1sec = srvl_poll_1sec;
        state->gpio_mode = vtss_srvl_gpio_mode;
        state->gpio_read = srvl_gpio_read;
        state->gpio_write = srvl_gpio_write;
        state->sgpio_conf_set = srvl_sgpio_conf_set;
        state->sgpio_read = srvl_sgpio_read;
        state->sgpio_event_enable = srvl_sgpio_event_enable;
        state->sgpio_event_poll = srvl_sgpio_event_poll;
        state->ptp_event_poll = srvl_ptp_event_poll;
        state->ptp_event_enable = srvl_ptp_event_enable;
        state->intr_cfg = srvl_intr_cfg;
        state->intr_pol_negation = srvl_intr_pol_negation;
#ifdef VTSS_FEATURE_IRQ_CONTROL
        vtss_state->misc.irq_cfg = srvl_misc_irq_cfg;
        vtss_state->misc.irq_status = srvl_misc_irq_status;
        vtss_state->misc.irq_enable = srvl_misc_irq_enable;
#endif  /* VTSS_FEATURE_IRQ_CONTROL */
        state->gpio_count = SRVL_GPIOS;
        state->sgpio_group_count = SRVL_SGPIO_GROUPS;
        
        /* EEE */
        vtss_state->eee.port_conf_set   = srvl_eee_port_conf_set;
        vtss_state->fan.controller_init = srvl_fan_controller_init;
        vtss_state->fan.cool_lvl_get    = srvl_fan_cool_lvl_get;
        vtss_state->fan.cool_lvl_set    = srvl_fan_cool_lvl_set;
        vtss_state->fan.rotation_get    = srvl_fan_rotation_get;
    }

    if (cmd == VTSS_INIT_CMD_POLL) {
        VTSS_RC(srvl_misc_poll_1sec(vtss_state));
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_ARCH_SERVAL */
