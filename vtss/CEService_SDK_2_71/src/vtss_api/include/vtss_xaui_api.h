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
 * \file vtss_xaui_api.h
 * \brief  XAUI API
 */

#ifndef _VTSS_XAUI_API_H_
#define _VTSS_XAUI_API_H_

#include <vtss_options.h>
#include <vtss_types.h>

#if defined(VTSS_FEATURE_XAUI)

/* ================================================================= *
 *  Data structures / types
 * ================================================================= */
/** \brief xaui config data   */
typedef struct vtss_xaui_cfg_t {
    BOOL equipment_loopback;        /**< Testloop, if enabled (XAUI encoded) data are looped from TX path to RX path just before the SERDES */
    BOOL line_loopback;             /**< Line loopback that passes through the entire PCS in both directions */
} vtss_xaui_cfg_t;

/** \brief xaui status   */
typedef struct vtss_xaui_status_t {
    char sync_status;       /**< bit 0..3 indicates sync_status for lane 0..4 (1 indicates that the lane is in sync) */
    BOOL lanes_aligned;     /**< TRUE indicates that all lanes are aligned */
    BOOL link_ok;           /**< TRUE indicates that link state is ok */
} vtss_xaui_status_t;

/** \brief xaui statistics   */
typedef struct vtss_xaui_pm_cnt_t {
    u32 rx_align_err_cnt;       /**< Counts the number of alignment errors in the RX direction */
    u32 xgmii_err_cnt;          /**< Counts the number of /E/ control codes that are generated in the TX path */
    u32 rx_fifo_overflow_cnt;   /**< Number of detected fifo overflow errors (normal)/Number of errors in lane 0 (test)*/
    u32 rx_fifo_underflow_cnt;  /**< Number of detected fifo underflow errors (normal)/Number of errors in lane 1 (test)*/
    u32 rx_disparity_err_cnt;   /**< Number of detected disparity errors (normal)/Number of errors in lane 2 (test)*/
    u32 code_group_err_cnt;     /**< Number of detected codegroup errors (normal)/Number of errors in lane 3 (test)*/
    u64 good_bytes_rx;          /**< Counts the number of good payload bytes in the RX direction */
    u64 good_bytes_tx;          /**< Counts the number of good payload bytes in the TX direction */
} vtss_xaui_pm_cnt_t;

/** \brief xaui state - container for vtss_state    */
typedef struct vtss_xaui_state_t {
    vtss_xaui_cfg_t     xaui_cfg; /**< xaui config data */
} vtss_xaui_state_t;

/* ================================================================= *
 *  Defects/Events
 * ================================================================= */

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief   Set XAUI dynamic configuration (currently not used).
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg  [IN]      pointer to configuration data structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_xaui_config_set(const vtss_inst_t inst,
                             const vtss_port_no_t port_no,
                             const vtss_xaui_cfg_t *const cfg);

/**
 * \brief   Get XAUI dynamic configuration (currently not used).
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cfg  [OUT]     pointer to configuration data structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_xaui_config_get(const vtss_inst_t inst,
                             const vtss_port_no_t port_no,
                             vtss_xaui_cfg_t *const cfg);

/* ================================================================= *
 *  State Reporting
 * ================================================================= */
/**
 * \brief Get current XAUI status.
 *
 * \param inst    [IN]   Target instance reference.
 * \param port_no [IN]   Port number.
 * \param status  [OUT]  pointer to defect status structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_xaui_status_get(const vtss_inst_t inst,
                             const vtss_port_no_t port_no,
                             vtss_xaui_status_t *const status);

/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */
/**
 * \brief Read PCS_XAUI statistics counters.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 * \param cnt [OUT]      pointer to counter structure.
 *
 * \return Return code.
 **/
vtss_rc vtss_xaui_counters_get(const vtss_inst_t inst,
                               const vtss_port_no_t port_no,
                               vtss_xaui_pm_cnt_t *const cnt);

/**
 * \brief Clear PCS_XAUI statistics counters.
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]   Port number.
 *
 * \return Return code.
 **/
vtss_rc vtss_xaui_counters_clear(const vtss_inst_t inst,
                                 const vtss_port_no_t port_no);

#endif /* VTSS_FEATURE_XAUI */
#endif /* _VTSS_XAUII_API_H_ */
