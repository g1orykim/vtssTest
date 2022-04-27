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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_I2C

// Avoid "vtss_api.h not used in module vtss_daytona_i2c.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_i2c.h"


#if defined(VTSS_FEATURE_I2C)

/* ================================================================= *
 *  Static Config
 * ================================================================= */


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief Set I2C controller configuration.
 *
 * \param controller_cfg [IN]   I2C controller configuration.
 * \param cfg            [IN]   I2C configuration.
 *
 * \return Return code.
 **/
static vtss_rc vtss_daytona_i2c_config_set(vtss_state_t *vtss_state,
                                           const vtss_i2c_controller_t *const controller_cfg,
                                           const vtss_i2c_cfg_t        *const cfg)
{
    vtss_rc rc = VTSS_RC_OK;
    u32     reg_val_tx = 0;
    u32     reg_mask_tx = 0;

    switch (controller_cfg->controller_no) {
        case 0:
            /* Update the I2C prescale value */
            reg_val_tx = VTSS_F_FAST_REGS_CSR_REGS_I2C_PRESCALE_0_I2C_PRESCALE_0(cfg->pre_scale);
            reg_mask_tx = VTSS_M_FAST_REGS_CSR_REGS_I2C_PRESCALE_0_I2C_PRESCALE_0;
            DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_I2C_PRESCALE_0, reg_val_tx, reg_mask_tx);
           
            /* Update the I2C TXACK init mask */
            reg_val_tx = VTSS_F_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_TXACK_MASK_0(cfg->int_mask & VTSS_I2C_INT_MASK_TXACK);
            reg_mask_tx = VTSS_M_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_TXACK_MASK_0;
            DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0, reg_val_tx, reg_mask_tx);

            /* Update the I2C AL init mask */
            reg_val_tx = VTSS_F_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_AL_MASK_0(cfg->int_mask & VTSS_I2C_INT_MASK_AL);
            reg_mask_tx = VTSS_M_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_AL_MASK_0;
            DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0, reg_val_tx, reg_mask_tx);

            /* Enable/Disable I2C controller */
            reg_val_tx = VTSS_F_FAST_REGS_CSR_REGS_I2C_CTL_REG_0_I2C_EN_0(cfg->enable);
            reg_mask_tx = VTSS_M_FAST_REGS_CSR_REGS_I2C_CTL_REG_0_I2C_EN_0;
            DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_I2C_CTL_REG_0, reg_val_tx, reg_mask_tx);
            break;

        case 1:
            /* Update the I2C prescale value */
            reg_val_tx = VTSS_F_FAST_REGS_CSR_REGS_I2C_PRESCALE_1_I2C_PRESCALE_1(cfg->pre_scale);
            reg_mask_tx = VTSS_M_FAST_REGS_CSR_REGS_I2C_PRESCALE_1_I2C_PRESCALE_1;
            DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_I2C_PRESCALE_1, reg_val_tx, reg_mask_tx);
           
            /* Update the I2C TXACK init mask */
            reg_val_tx = VTSS_F_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_TXACK_MASK_1(cfg->int_mask & VTSS_I2C_INT_MASK_TXACK);
            reg_mask_tx = VTSS_M_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_TXACK_MASK_1;
            DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1, reg_val_tx, reg_mask_tx);

            /* Update the I2C AL init mask */
            reg_val_tx = VTSS_F_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_AL_MASK_1(cfg->int_mask & VTSS_I2C_INT_MASK_AL);
            reg_mask_tx = VTSS_M_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_AL_MASK_1;
            DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1, reg_val_tx, reg_mask_tx);

            /* Enable/Disable I2C controller */
            reg_val_tx = VTSS_F_FAST_REGS_CSR_REGS_I2C_CTL_REG_1_I2C_EN_1(cfg->enable);
            reg_mask_tx = VTSS_M_FAST_REGS_CSR_REGS_I2C_CTL_REG_1_I2C_EN_1;
            DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_I2C_CTL_REG_1, reg_val_tx, reg_mask_tx);
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
    } 
    return rc;
 } 

/**
 * \brief Get I2C controller Configuration.
 *
 * \param controller_cfg [IN]   I2C controller configuration.
 * \param cfg            [OUT]  I2C configuration.
 *
 * \return Return code.
 **/
static vtss_rc vtss_daytona_i2c_config_get(vtss_state_t *vtss_state,
                                           const vtss_i2c_controller_t *const controller_cfg,
                                           vtss_i2c_cfg_t        *const cfg)
{
    vtss_rc rc = VTSS_RC_OK;
    u32     reg_val_rx = 0;

    switch(controller_cfg->controller_no) {
        case 0:
            /* Read I2C controller prescale value */
            DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_I2C_PRESCALE_0, &reg_val_rx);
            cfg->pre_scale = VTSS_X_FAST_REGS_CSR_REGS_I2C_PRESCALE_0_I2C_PRESCALE_0(reg_val_rx);

            /* Read I2C controller interupt mask value */
            cfg->int_mask = 0;
            DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0, &reg_val_rx);
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_TXACK_MASK_0(reg_val_rx)) {
                cfg->int_mask |= VTSS_I2C_INT_MASK_TXACK; 
            } 
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_AL_MASK_0(reg_val_rx)) {
                cfg->int_mask |= VTSS_I2C_INT_MASK_AL; 
            }

            /* Read I2C controller status */
            DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_I2C_CTL_REG_0, &reg_val_rx);
            cfg->enable = VTSS_X_FAST_REGS_CSR_REGS_I2C_CTL_REG_0_I2C_EN_0(reg_val_rx);
           
            break;
        case 1:
            /* Read I2C controller prescale value */
            DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_I2C_PRESCALE_1, &reg_val_rx);
            cfg->pre_scale = VTSS_X_FAST_REGS_CSR_REGS_I2C_PRESCALE_1_I2C_PRESCALE_1(reg_val_rx);

            /* Read I2C controller interupt mask value */
            cfg->int_mask = 0;
            DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1, &reg_val_rx);
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_TXACK_MASK_1(reg_val_rx)) {
                cfg->int_mask |= VTSS_I2C_INT_MASK_TXACK; 
            } 
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_AL_MASK_1(reg_val_rx)) {
                cfg->int_mask |= VTSS_I2C_INT_MASK_AL; 
            }

            /* Read I2C controller status */
            DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_I2C_CTL_REG_1, &reg_val_rx);
            cfg->enable = VTSS_X_FAST_REGS_CSR_REGS_I2C_CTL_REG_1_I2C_EN_1(reg_val_rx);
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
    }

    return rc;
}

/**
 * \brief Get I2C controller status.
 *
 * \param controller_cfg [IN]   I2C controller configuration.
 * \param cfg            [OUT]  I2C controller status.
 *
 * \return Return code.
 **/
static vtss_rc vtss_daytona_i2c_status_get(vtss_state_t *vtss_state,
                                           const vtss_i2c_controller_t *const controller_cfg,
                                           u16                   *const status)
{
    vtss_rc rc = VTSS_RC_OK;
    u32     reg_val_rx = 0;

    switch (controller_cfg->controller_no) {
        case 0:
            DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0, &reg_val_rx);
            *status = 0;
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TXACK_STICKY_0(reg_val_rx)) {
                *status |= VTSS_I2C_STATUS_TXACK_STICKY;
            }
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_AL_STICKY_0(reg_val_rx)) {
                *status |= VTSS_I2C_STATUS_AL_STICKY;
            }
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TXACK_0(reg_val_rx)) {
                *status |= VTSS_I2C_STATUS_TC_ACK;
            }
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TIP_0(reg_val_rx)) {
                *status |= VTSS_I2C_STATUS_TIP;
            }
            /* Write back the status to clear the sticky bits */
            DAYTONA_WR(VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0, reg_val_rx);
            break;
        case 1:
            DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1, &reg_val_rx);
            *status = 0;
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TXACK_STICKY_1(reg_val_rx)) {
                *status |= VTSS_I2C_STATUS_TXACK_STICKY;
            }
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_AL_STICKY_1(reg_val_rx)) {
                *status |= VTSS_I2C_STATUS_AL_STICKY;
            }
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TXACK_1(reg_val_rx)) {
                *status |= VTSS_I2C_STATUS_TC_ACK;
            }
            if (VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TIP_1(reg_val_rx)) {
                *status |= VTSS_I2C_STATUS_TIP;
            }
            /* Write back the status to clear the sticky bits */
            DAYTONA_WR(VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1, reg_val_rx);
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;

    }
    return rc;
}

/**
 * \brief Read data from the I2C controller.
 *
 * \param controller_cfg [IN]   I2C controller configuration.
 * \param data           [IN]   data. 
 *
 * \return Return code.
 **/
static vtss_rc vtss_daytona_i2c_rx(vtss_state_t *vtss_state,
                                   const vtss_i2c_controller_t *const controller_cfg,
                                   u8                    *const data)
{

    vtss_rc rc = VTSS_RC_OK;
    u32     reg_val_rx = 0;

    switch (controller_cfg->controller_no) {
        case 0:
            DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_I2C_RX_REG_0, &reg_val_rx);
            *data = VTSS_X_FAST_REGS_CSR_REGS_I2C_RX_REG_0_I2C_RX_REG_0(reg_val_rx);
            break;
        case 1:
            DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_I2C_RX_REG_1, &reg_val_rx);
            *data = VTSS_X_FAST_REGS_CSR_REGS_I2C_RX_REG_1_I2C_RX_REG_1(reg_val_rx);
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
    }
    return rc;
}

/**
 * \brief Write data to the I2C controller.
 *
 * \param controller_cfg [IN]   I2C controller configuration.
 * \param addr_data      [IN]   I2C device/register address or data.
 *
 * \return Return code.
 **/
static vtss_rc vtss_daytona_i2c_tx(vtss_state_t *vtss_state,
                                   const vtss_i2c_controller_t *const controller_cfg,
                                   const u8                    *const addr_data)
{
    vtss_rc rc = VTSS_RC_OK;

    switch(controller_cfg->controller_no) {
        case 0:
            /* Update the I2C WR register */
            DAYTONA_WR(VTSS_FAST_REGS_CSR_REGS_I2C_TX_REG_0, *addr_data);
            break;
        case 1:
            /* Update the I2C WR register */
            DAYTONA_WR(VTSS_FAST_REGS_CSR_REGS_I2C_TX_REG_1, *addr_data);
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
    }

    return rc;
}

/**
 * \brief Write commands to the I2C controller.
 *
 * \param controller_cfg [IN]   I2C controller configuration.
 * \param cmd_flags      [IN]   I2C controller commands.
 *
 * \return Return code.
 **/
static vtss_rc vtss_daytona_i2c_cmd(vtss_state_t *vtss_state,
                                    const vtss_i2c_controller_t *const controller_cfg,
                                    const u16                          cmd_flags)
{
    vtss_rc rc = VTSS_RC_OK;
    u32     tmp_cmd_flags = 0;

    switch(controller_cfg->controller_no) {
        case 0:
            /* Update the I2C command register */
            if (cmd_flags & VTSS_I2C_CMD_STA) {
                tmp_cmd_flags |= VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_STA_0(1);
            }
            if (cmd_flags & VTSS_I2C_CMD_STO) {
                tmp_cmd_flags |= VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_STO_0(1);
            }
            if (cmd_flags & VTSS_I2C_CMD_RD) {
                tmp_cmd_flags |= VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_RD_0(1);
            }
            if (cmd_flags & VTSS_I2C_CMD_WR) {
                tmp_cmd_flags |= VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_WR_0(1);
            }
            if (cmd_flags & VTSS_I2C_CMD_RD_ACK) {
                tmp_cmd_flags |= VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_ACK_0(1);
            }
            DAYTONA_WR(VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_0, tmp_cmd_flags);
            break;
        case 1:
            /* Update the I2C command register */
            if (cmd_flags & VTSS_I2C_CMD_STA) {
                tmp_cmd_flags |= VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_STA_1(1);
            }
            if (cmd_flags & VTSS_I2C_CMD_STO) {
                tmp_cmd_flags |= VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_STO_1(1);
            }
            if (cmd_flags & VTSS_I2C_CMD_RD) {
                tmp_cmd_flags |= VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_RD_1(1);
            }
            if (cmd_flags & VTSS_I2C_CMD_WR) {
                tmp_cmd_flags |= VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_WR_1(1);
            }
            if (cmd_flags & VTSS_I2C_CMD_RD_ACK) {
                tmp_cmd_flags |= VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_ACK_1(1);
            }
            DAYTONA_WR(VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_1, tmp_cmd_flags);
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
    }

    return rc;
}

/**
 * \brief Stop indication to the I2C controller.
 *
 * \param controller_cfg [IN]   I2C controller configuration.
 *
 * \return Return code.
 **/
static vtss_rc vtss_daytona_i2c_stop(vtss_state_t *vtss_state,
                                     const vtss_i2c_controller_t *const controller_cfg)
{
    vtss_rc rc = VTSS_RC_OK;
    u32     reg_val_tx = 0;
    u32     reg_mask_tx = 0;

    switch (controller_cfg->controller_no) {
        case 0:
            /* Update the I2C STOP bit */
            reg_val_tx = VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_STO_0(1);
            reg_mask_tx = VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_STO_0;
            DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_0, reg_val_tx, reg_mask_tx);
            break;
        case 1:
            /* Update the I2C STOP bit */
            reg_val_tx = VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_STO_1(1);
            reg_mask_tx = VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_STO_1;
            DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_1, reg_val_tx, reg_mask_tx);
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
    }
     
    return rc;
}


/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */



/* ================================================================= *
 *  Utilities and internal
 * ================================================================= */

/**
 * \brief Create instance (set up function pointers).
 *
 * \return Return code.
 **/
vtss_rc vtss_daytona_inst_i2c_create(vtss_state_t *vtss_state)
{
    vtss_cil_func_t *func = &vtss_state->cil;

    /* Daytona I2C functions */
    func->i2c_set_config = vtss_daytona_i2c_config_set;
    func->i2c_get_config = vtss_daytona_i2c_config_get;
    func->i2c_get_status = vtss_daytona_i2c_status_get;
    func->i2c_rx         = vtss_daytona_i2c_rx;
    func->i2c_tx         = vtss_daytona_i2c_tx;
    func->i2c_cmd        = vtss_daytona_i2c_cmd;
    func->i2c_stop       = vtss_daytona_i2c_stop;
    return VTSS_RC_OK;
}
#endif /* VTSS_FEATURE_I2C */
#endif /* VTSS_ARCH_DAYTONA */



