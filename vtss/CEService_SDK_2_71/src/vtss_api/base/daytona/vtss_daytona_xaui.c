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

#define VTSS_TRACE_LAYER VTSS_TRACE_LAYER_CIL 

// Avoid "vtss_api.h not used in module vtss_daytona_xaui.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_xaui.h"
#include "vtss_daytona_regs_pcs_xaui.h"



/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

static vtss_rc daytona_xaui_config_set(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no)
{
    u32 target;
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_XAUI, &target));
    
    DAYTONA_WRM(VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG(target),
                VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_LINE_LOOP_ENA(vtss_state->xaui_state[port_no].xaui_cfg.line_loopback),
                VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_LINE_LOOP_ENA);
    
    DAYTONA_WRM(VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG(target),
                VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_XAUI_LOOP_ENA(vtss_state->xaui_state[port_no].xaui_cfg.equipment_loopback),
                VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_XAUI_LOOP_ENA);
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  State Reporting
 * ================================================================= */
static vtss_rc daytona_xaui_status_get(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no,
                                       vtss_xaui_status_t *const status)
{
    u32 target;
    u32 xaui_status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_XAUI, &target));
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS(target), &xaui_status);
    VTSS_D("port_no %d, target %X, xaui_status %X", port_no, target, xaui_status);
    status->sync_status = VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_SYNC_STATUS(xaui_status);
    status->lanes_aligned = VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_ALIGNMENT_STATUS(xaui_status);
    status->link_ok = VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_LINK_STATE(xaui_status);
    VTSS_D("port_no %d, xaui: link_ok %d, lanes_aligned, %d sync_status %x", port_no, status->link_ok,
           status->lanes_aligned, status->sync_status);

    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */

static vtss_rc daytona_xaui_counters_get(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no,
                                         vtss_xaui_pm_cnt_t *const cnt)
{
    u32 target;
    u32 xaui_lsb;
    u32 xaui_msb;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_XAUI, &target));
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_ALIGN_ERR_CNT(target), &cnt->rx_align_err_cnt);
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_XGMII_ERR_CNT(target), &cnt->xgmii_err_cnt);
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS(target),
               &cnt->rx_fifo_overflow_cnt);
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS(target),
               &cnt->rx_fifo_underflow_cnt);
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS(target),
               &cnt->rx_disparity_err_cnt);
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS(target),
               &cnt->code_group_err_cnt);
    /* ok byte counters are 48 bits, LSB must be read first */
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT(target), &xaui_lsb);
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_MSB(target), &xaui_msb);
    cnt->good_bytes_rx = xaui_lsb + ((u64)VTSS_X_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_MSB_PCS_XAUI_RX_OK_BYTES_CNT_MSB(xaui_msb)<<32);
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT(target), &xaui_lsb);
    DAYTONA_RD(VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_MSB(target), &xaui_msb);
    cnt->good_bytes_tx = xaui_lsb + ((u64)VTSS_X_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_MSB_PCS_XAUI_TX_OK_BYTES_CNT_MSB(xaui_msb)<<32);

    VTSS_D("port_no %d, xaui counters", port_no);

    return VTSS_RC_OK;
}


static vtss_rc daytona_xaui_counters_clear(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_XAUI, &target));
    DAYTONA_WR(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_ALIGN_ERR_CNT(target), 0);
    DAYTONA_WR(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_XGMII_ERR_CNT(target), 0);
    DAYTONA_WR(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS(target), 0);
    DAYTONA_WR(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS(target), 0);
    DAYTONA_WR(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS(target), 0);
    DAYTONA_WR(VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS(target), 0);
    /* ok byte counters are 48 bits, LSB is cleared first */
    DAYTONA_WR(VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT(target), 0);
    DAYTONA_WR(VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_MSB(target), 0);
    DAYTONA_WR(VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT(target), 0);
    DAYTONA_WR(VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_MSB(target), 0);

    VTSS_D("port_no %d, clear xaui counters", port_no);

    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Utilities and internal
 * ================================================================= */


vtss_rc vtss_daytona_xaui_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_rc rc;

    rc = daytona_xaui_counters_clear(vtss_state, port_no);
    rc = daytona_xaui_config_set(vtss_state, port_no);
    VTSS_D("port_no %d, restart conf set, rc = %x", port_no, rc);
    return VTSS_RC_OK;
}

/**
 * \brief static initialization of xaui configuration
 */
static vtss_xaui_cfg_t xaui_cfg_default = {
        .equipment_loopback = FALSE,
        .line_loopback      = FALSE,
};

/**
 * \brief Set default xaui state
 *
 * \param port_no [IN] Port number
 * \return Return code
 */
vtss_rc vtss_daytona_xaui_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_state->xaui_state[port_no].xaui_cfg = xaui_cfg_default;

    return VTSS_RC_OK;
}

/**
 * \brief Create instance (set up function pointers).
 *
 * \param inst [IN]      Target instance reference.
 * \param port_map [IN]  Port map array.
 *
 * \return Return code.
 **/
vtss_rc vtss_daytona_inst_xaui_create(vtss_state_t *vtss_state)
{
    vtss_cil_func_t *func = &vtss_state->cil;

    /* Daytona XAUI functions */
    func->xaui_config_set = daytona_xaui_config_set;
    func->xaui_status_get = daytona_xaui_status_get;
    func->xaui_counters_get = daytona_xaui_counters_get;
    func->xaui_counters_clear = daytona_xaui_counters_clear;
    return VTSS_RC_OK;
}


#endif /* VTSS_ARCH_DAYTONA */



