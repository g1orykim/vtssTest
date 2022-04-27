/*

 Vitesse API software.

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

/**
 * \file vtss_i2c_api.h
 * \brief  I2C  API
 */

#ifndef _VTSS_I2C_API_H_
#define _VTSS_I2C_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_I2C)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */


/** \brief  I2C commands */
#define VTSS_I2C_CMD_STA     1  /**< START command */ 
#define VTSS_I2C_CMD_STO     2  /**< STOP command  */
#define VTSS_I2C_CMD_RD      4  /**< READ command  */
#define VTSS_I2C_CMD_WR      8  /**< WRITE command */
#define VTSS_I2C_CMD_RD_ACK 16  /**< READ completion acknowledge command */

/** \brief  I2C status */
#define VTSS_I2C_STATUS_TXACK_STICKY    1  /**< TX operation acknowledge sticky */
#define VTSS_I2C_STATUS_AL_STICKY       2  /**< Arbitartion lost sticky */
#define VTSS_I2C_STATUS_TC_ACK          4  /**< Tx operation completion sticky */ 
#define VTSS_I2C_STATUS_TIP             8  /**< I2C cycle/transfer status      */

/** \brief  I2C interupt mask */
#define VTSS_I2C_INT_MASK_TXACK 1  /**< TX acknowledge interrupt mask */
#define VTSS_I2C_INT_MASK_AL    2  /**< Arbitaration lost interrupt mask */

/** \brief I2C controller */
typedef struct vtss_i2c_controller {
    u16 controller_no; /**< I2C controller number */
} vtss_i2c_controller_t;

/** \brief I2C config data   */
typedef struct vtss_i2c_cfg {
    BOOL enable;       /**< Enable/disable the controller */
    u16  pre_scale;    /**< pre scale value based on system and SCL clocks */
    u16  int_mask;     /**< Mask to enable/disable I2C interrupts */
} vtss_i2c_cfg_t;


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief   I2C controller configuration set.
 *
 * \param inst           [IN]      Target instance reference.
 * \param controller_cfg [IN]      I2C controller configuration.
 * \param cfg            [IN]      I2C configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_i2c_conf_set(const vtss_inst_t                  inst,
                          const vtss_i2c_controller_t *const controller_cfg,
                          const vtss_i2c_cfg_t        *const cfg);

/**
 * \brief   I2C controller configuration get.
 *
 * \param inst           [IN]      Target instance reference.
 * \param controller_cfg [IN]      I2C controller configuration.
 * \param cfg            [OUT]     I2C configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_i2c_conf_get(const vtss_inst_t                  inst,
                          const vtss_i2c_controller_t *const controller_cfg,
                                vtss_i2c_cfg_t        *const cfg);

/**
 * \brief   Read data fom the I2C controller.
 *
 * \param inst           [IN]      Target instance reference.
 * \param controller_cfg [IN]      I2C controller configuration.
 * \param data           [OUT]     data.
 *
 * \return Return code.
 **/
vtss_rc vtss_i2c_rx(const vtss_inst_t                  inst,
                    const vtss_i2c_controller_t *const controller_cfg,
                          u8                    *const data);
/**
 * \brief   Write data to the I2C controller.
 *
 * \param inst           [IN]      Target instance reference.
 * \param controller_cfg [IN]      I2C controller configuration.
 * \param addr_data      [IN]      Address of the device/register or data.
 *
 * \return Return code.
 **/
vtss_rc vtss_i2c_tx(const vtss_inst_t                  inst,
                    const vtss_i2c_controller_t *const controller_cfg,
                    const u8                    *const addr_data);

/**
 * \brief   Write command flags to the I2C controller.
 *
 * \param inst           [IN]      Target instance reference.
 * \param controller_cfg [IN]      I2C controller configuration.
 * \param cmd_flags      [IN]      I2C controller Command flags .
 *
 * \return Return code.
 **/
vtss_rc vtss_i2c_cmd(const vtss_inst_t                  inst,
                     const vtss_i2c_controller_t *const controller_cfg,
                     const u16                          cmd_flags);

/**
 * \brief   Stop indication to the controller.
 *
 * \param inst           [IN]      Target instance reference.
 * \param controller_cfg [IN]      I2C controller configuration.
 *
 * \return Return code.
 **/
vtss_rc vtss_i2c_stop(const vtss_inst_t                  inst,
                      const vtss_i2c_controller_t *const controller_cfg);

/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/**
 * \brief   I2C status get.
 *
 * \param inst           [IN]      Target instance reference.
 * \param controller_cfg [IN]      I2C controller configuration.
 * \param status         [OUT]     I2C status.
 *
 * \return Return code.
 **/
vtss_rc vtss_i2c_status_get(const vtss_inst_t                  inst,
                            const vtss_i2c_controller_t *const controller_cfg,
                                  u16                   *const status);



/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */

#endif /* VTSS_FEATURE_I2C */

#endif /* _VTSS_I2C_API_H_ */
