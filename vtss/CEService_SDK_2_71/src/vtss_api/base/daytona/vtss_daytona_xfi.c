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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_XFI

// Avoid "vtss_api.h not used in module vtss_daytona_xfi.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_xfi.h"
#include "vtss_daytona_regs_xfi.h"




/* ================================================================= *
 *  Static Config
 * ================================================================= */
/*done in a separate file */

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief Dynamic Config set config.
 *
 * \param port_no [IN]   Port number.
 *
 * Configuration parameters comments:
 *    enable; not used
 *    sel_lane_b; Data is always sent to both lane A and B. "SEL_TX_LANEB" is used to select which of the 2 Tx
 *      Lanes Serdes clock is used to drive the clock to the core of the Daytona Channel.  Theoretically, it doens't
 *      matter if you use A or B as they should be the exact same frequency, but only slightly different phase.
 *      The main datapath is active to both lanes all the time, It is just a matter of whether the Serdes lane is
 *      powered down, or not as to whether the TX data would be transmitted externally.
 *      The API sets both SEL_RX_LANEB and SEL_TX_LANEB equally.
 *      The INT_CTRL register controls the generation of XFI_SSF indication. It is set up to match the selected lane, i.e:
 *        lane a selected: ssf = rxa_los | ^rxasigdet
 *        lane b selected: ssf = rxb_los | ^rxbsigdet
 *    pat_check; Not supported any more by the chip
 *    equipment loopback is set up using FDWRAP in VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL register
 *      this register exists in 4 instances, instance 0 controls lane A instance 1 controls lane B
 *      When the loopback is cleared, a DFE reset is issued by:
 *        1) Clear FDWRAP (= 0)
 *        2) Set DFERST (=1)
 *        3) Clear DFERST (=0)
 *      It is the rising DFERST that activates the reset (not the high level) so the order is important.
 *      There is no other timing constraints (i.e. how long it must be held, etc).
 *
 * \return Return code.
 **/
static vtss_rc daytona_xfi_config_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no)
{
    u32 target;
    u32 reg_val=0;
    u32 reg_mask=0;
    u32 elb_old, elb; /*equipment loopback internals */

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &target));

    reg_val |= (vtss_state->xfi_state[port_no].xfi_cfg.sel_lane_b == TRUE) ? 
               VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_TX_LANEB | VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_RX_LANEB:reg_val;

    reg_val |= (vtss_state->xfi_state[port_no].xfi_cfg.bypass_aneg == TRUE) ? VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_ANEG:reg_val;
    reg_val |= (vtss_state->xfi_state[port_no].xfi_cfg.bypass_ae == TRUE) ? VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_AE:reg_val;
    /* |VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_PD AKR: removed */
    /* line loopback mode */
    reg_val |= (vtss_state->xfi_state[port_no].xfi_cfg.line_loopback == TRUE) ? VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_LINELB:reg_val;

    reg_mask = (VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC|VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TXF_ERR_MODE|VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_TX_LANEB|
                VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_RX_LANEB | VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_ANEG |
                VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_AE | VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_LINELB);

    DAYTONA_WRM(VTSS_XFI_XFI_CONTROL_XFI_MODE(target),reg_val,reg_mask);
       
    
    /* equipment loopback mode: Note that the test control register for laneA is index 0, and laneB is index 1
     * index 2 hits both lanes, but this is a writeonly register, so a Masked write does not work
     */
    DAYTONA_RD(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,0x0),&elb_old);
    elb_old = VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_FDWRAP(elb_old);
    elb = VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_FDWRAP(vtss_state->xfi_state[port_no].xfi_cfg.equipment_loopback);
    DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,0x0),elb,VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_FDWRAP);
    DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,0x1),elb,VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_FDWRAP);
    /* if loopback cleared, the DFE Reset must be issued, i.e. setting DFERST  bit and then clearing it*/
    if (elb_old && !vtss_state->xfi_state[port_no].xfi_cfg.equipment_loopback) {
        DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,0x0),
                    VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFERST,
                    VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFERST);
        DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,0x1),
                    VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFERST,
                    VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFERST);
        DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,0x0),
                    0,
                    VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFERST);
        DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,0x1),
                    0,
                    VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFERST);
    } 
    if (vtss_state->xfi_state[port_no].xfi_cfg.equipment_loopback == TRUE) {
        /* INT_CTRL register controls the generation of XFI_SSF indication */
        reg_val = (vtss_state->xfi_state[port_no].xfi_cfg.sel_lane_b == TRUE) ?  
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_INV | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_MASK | 
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXB_LOS_MASK | VTSS_F_XFI_XFI_CONTROL_INT_CTRL_SSF_MASK(0)
                 :
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_INV | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_MASK | 
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXA_LOS_MASK | VTSS_F_XFI_XFI_CONTROL_INT_CTRL_SSF_MASK(0);
        reg_mask = 
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_INV | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_MASK | 
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXB_LOS_MASK | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_SSF_MASK |
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_INV | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_MASK | 
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXA_LOS_MASK;
        DAYTONA_WRM(VTSS_XFI_XFI_CONTROL_INT_CTRL(target),reg_val,reg_mask);

    } else {
        /* INT_CTRL register controls the generation of XFI_SSF indication */
        reg_val = (vtss_state->xfi_state[port_no].xfi_cfg.sel_lane_b == TRUE) ?  
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_INV | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_MASK | 
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXB_LOS_MASK | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_SSF_MASK
                 :
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_INV | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_MASK | 
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXA_LOS_MASK | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_SSF_MASK;
        reg_mask = 
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_INV | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_MASK | 
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXB_LOS_MASK | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_SSF_MASK |
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_INV | VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_MASK | 
                 VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXA_LOS_MASK;
        DAYTONA_WRM(VTSS_XFI_XFI_CONTROL_INT_CTRL(target),reg_val,reg_mask);
    }
 
    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(target, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST(1),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST);

    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(target, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST(0),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST);

    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(target, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF(1),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF);

    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL(target, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE(1),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE);

    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE(target, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE_NIDAC1(115),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE_NIDAC1);

    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(target, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(1),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);

    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(target, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(0),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);

    DAYTONA_WRM (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(target, 2),
        VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_DMSEL(0),
        VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_DMSEL);

    return VTSS_RC_OK;
}

/**
 * \brief Dynamic Config AE reset.
 *
 * \param port_no [IN]   Port number.
 * \param lane [IN]      lane A or B.
 *
 * \return Return code.
 **/
static vtss_rc daytona_xfi_ae_reset(vtss_state_t *vtss_state,
                                    const vtss_port_no_t port_no,vtss_xfi_lane_t lane)
{
    u32 target;
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &target));


    return VTSS_RC_OK;
}
/**
 * \brief Recovered Clock Output Parameters set.
 *
 * \param port_no [IN]   Port number.
 * \param xfi_rec_clk_output[IN]     Set parameters for the XFI Recovered Clock output.
 *
 * \return Return code.
 **/
static vtss_rc daytona_xfi_recovered_clock_output_set(vtss_state_t *vtss_state,
                                                      const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &target));
    DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,0), VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN(vtss_state->xfi_state[port_no].xfi_cfg.xfi_rec_clock_output.lane_a), VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN);
    DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,1), VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN(vtss_state->xfi_state[port_no].xfi_cfg.xfi_rec_clock_output.lane_b), VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN);
    return VTSS_RC_OK;
}
/* ================================================================= *
 *  State Reporting
 * ================================================================= */
/**
 * \brief XFI status indication get.
 *
 * \param port_no [IN]   Port number.
 * \param status [OUT]   status from the XFI interface.
 *
 * Status parameters comments:
 *    ae_status.train_local; there are both a lane A and B version which one is reported, should be report both ?
 *    ae_status.train_ok;    same comment
 *
 * \return Return code.
 **/
static vtss_rc daytona_xfi_status_get(vtss_state_t *vtss_state,
                                      const vtss_port_no_t  port_no, vtss_xfi_status_t *const status)
{
    u32 target;
    u32 xfi_status;
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &target));

    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_XFI_STATUS(target), &xfi_status);
    VTSS_D("port_no %d, target %X, xfi_status %X", port_no, target, xfi_status);
    status->ssf = VTSS_X_XFI_XFI_CONTROL_XFI_STATUS_SSF_STATUS(xfi_status) != 0;

    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_HSS_STATUS(target), &xfi_status);
    VTSS_D("port_no %d, target %X, hss_status %X", port_no, target, xfi_status);
    status->hss_status.pll_lock = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_HSSPLLLOCK_STATUS(xfi_status) != 0;
    status->hss_status.hss_rst_out = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_HSSRESETOUT_STATUS(xfi_status) != 0;
    status->hss_status.hss_eyequality = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_HSSEYEQUALITY_STATUS(xfi_status) != 0;
    status->hss_status.prt_rdy = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_HSSPRTREADY_STATUS(xfi_status) != 0;
    status->hss_status.tx_fifo_los = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_TX_FIFO_LOS_STATUS(xfi_status) != 0;
    status->hss_status.ifpll_lock = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_IFPLL_LOCK_STATUS(xfi_status) != 0;
    status->hss_status.rx_a_sigdet = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXASIGDET_STATUS(xfi_status) != 0;
    status->hss_status.rx_a_los = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXA_LOS_STATUS(xfi_status) != 0;
    status->hss_status.rx_b_sigdet = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXBSIGDET_STATUS(xfi_status) != 0;
    status->hss_status.rx_b_los = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXB_LOS_STATUS(xfi_status) != 0;
    
    DAYTONA_RD(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS(target,0), &xfi_status);
    status->internal_status_a = xfi_status & 0x0000ffff;
    DAYTONA_WR(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS(target,0), xfi_status);

    DAYTONA_RD(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS(target,1), &xfi_status);
    status->internal_status_b = xfi_status & 0x0000ffff;
    DAYTONA_WR(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS(target,1), xfi_status);

    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_AE_STATUS(target), &xfi_status);
    VTSS_D("port_no %d, target %X, ae_status %X", port_no, target, xfi_status);
    status->ae_status.train_local = VTSS_X_XFI_XFI_CONTROL_AE_STATUS_TRAIN_LOCAL_A_STATUS(xfi_status) != 0;
    status->ae_status.train_ok = VTSS_X_XFI_XFI_CONTROL_AE_STATUS_TRAIN_OK_A_STATUS(xfi_status) != 0;

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */


/* ================================================================= *
 *  Test/Debug Config
 * ================================================================= */

/**
 * Test Config set.
 *
 * param port_no [IN]   Port number.
 *
 * return VTSS_RC_ERROR if invalid port or block id
 *        VTSS_RC_OK if ok.
 **/
static vtss_rc daytona_xfi_test_config_set(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no)
{
    u32 target;
    vtss_xfi_test_mode_t tst_pat;
    u32 patsel = 0, prbs_en = 0;
    u32 twdp = 0;
    u32 ri;
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &target));
    VTSS_I("port_no %d, target %X", port_no, target);

    /* set up test generator */
    tst_pat = vtss_state->xfi_state[port_no].xfi_test_cfg.test_cfg_gen;
    switch (tst_pat) {
        case VTSS_XFI_TEST_MODE_DISABLE: patsel = 0; prbs_en = 0; break;
        case VTSS_XFI_TEST_MODE_PRBS7_P: patsel = 0; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS7_N: patsel = 1; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS23_P: patsel = 2; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS23_N: patsel = 3; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS31_P: patsel = 4; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS31_N: patsel = 5; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_HF: patsel = 6; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_LF: patsel = 7; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS9_P: patsel = 0; twdp = 1; break;
        default:
            VTSS_E("Invalid test generator config %d", tst_pat);
            return VTSS_RC_ERROR;
    }
    /* broadcast pattern setting to both lanes */
    for (ri = 0 ; ri < 2; ri++) {
        DAYTONA_WRM(VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL(target,ri),
                    VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPSEL(patsel),
                    VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPSEL);
        DAYTONA_WRM(VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL(target,ri),
                    VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TWDP(twdp),
                    VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TWDP);
        DAYTONA_WRM(VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL(target,ri),
                    VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGEN(prbs_en!=0 ? 1:0),
                    VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGEN);
        /* reset generator */
        DAYTONA_WRM(VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL(target,ri),
                    VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGRST(prbs_en!=0 ? 1:0),
                    VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGRST);
        DAYTONA_WRM(VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL(target,ri),
                    VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGRST(0),
                    VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGRST);
    }
    /* select pattern generator as source for the transmitter */
    DAYTONA_WRM(VTSS_XFI_XFI_CONTROL_XFI_MODE(target), VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SEL_PG(prbs_en!=0 ? 1:0),
                VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_PG);

    /* set up test checker */
    tst_pat = vtss_state->xfi_state[port_no].xfi_test_cfg.test_cfg_chk;
    switch (tst_pat) {
        case VTSS_XFI_TEST_MODE_DISABLE: patsel = 0; prbs_en = 0; break;
        case VTSS_XFI_TEST_MODE_PRBS7_P: patsel = 0; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS7_N: patsel = 1; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS23_P: patsel = 2; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS23_N: patsel = 3; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS31_P: patsel = 4; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_PRBS31_N: patsel = 5; prbs_en = 0xf; break;
        case VTSS_XFI_TEST_MODE_HF:
        case VTSS_XFI_TEST_MODE_LF:
        case VTSS_XFI_TEST_MODE_PRBS9_P:
        default:
            VTSS_E("Invalid test checker config %d", tst_pat);
            return VTSS_RC_ERROR;
    }
    /* broadcast pattern setting to both lanes */
    for (ri = 0 ; ri < 2; ri++) {
        DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,ri),
                    VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRBSSEL(patsel),
                    VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRBSSEL);
        DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,ri),
                    VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PCHKEN(prbs_en!=0 ? 1:0),
                    VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PCHKEN);
        /* reset generator */
        DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,ri),
                    VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRST(prbs_en!=0 ? 1:0),
                    VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRST);
        DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,ri),
                    VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRST(0),
                    VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRST);
    }
    /* if checker enabled, then clear error counter */
    if (prbs_en) {
        DAYTONA_WR(VTSS_XFI_XFI_CONTROL_RXPRBSERR_CTR(target), 0);
    }
    return VTSS_RC_OK;
}

/*
 * Test status get from UPI interface.
 *
 * param port_no [IN]   Port number.
 * param status [OUT]   returned XFI functional block test status.
 *
 * return Return code.
 */
static vtss_rc daytona_xfi_test_status_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t  port_no, vtss_xfi_test_status_t *const status)
{
    u32 target;
    u32 xfi_status, hss_status;
    int ri;
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &target));
    
    for (ri = 0; ri < 2; ri++) {
        DAYTONA_RD(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,ri), &hss_status);
        VTSS_D("port_no %d, target %X, hss_status %X", port_no, target, hss_status);
        status->lbsync[ri] = VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_LBSYNC(hss_status);
        status->lberror[ri] = VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_LBERROR(hss_status);

    }
    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_HSS_STATUS(target), &xfi_status);
    status->rx_prbssync = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXPRBSSYNC_STATUS(xfi_status);
    status->rx_prbs_err = VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXPRBSERR_STATUS(xfi_status);
    
    /* read and clear error counters */
    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_RXPRBSERR_CTR(target), &status->rx_prbs_err_count);
    DAYTONA_WR(VTSS_XFI_XFI_CONTROL_RXPRBSERR_CTR(target), 0);
    
    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
static vtss_rc daytona_xfi_event_enable(vtss_state_t *vtss_state,
                                        const vtss_port_no_t      port_no,
                                        const BOOL                enable,
                                        const vtss_xfi_event_t    ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &target));

    mask = 0;

    if (ev_mask & VTSS_XFI_HSS_HSSPLLLOCK_EV)        mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSPLLLOCK_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSPLLLOCK_HI_MASK);
    if (ev_mask & VTSS_XFI_HSS_HSSRESETOUT_EV)       mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSRESETOUT_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSRESETOUT_HI_MASK);
    if (ev_mask & VTSS_XFI_HSS_HSSEYEQUALITY_EV)     mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSEYEQUALITY_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSEYEQUALITY_HI_MASK);
    if (ev_mask & VTSS_XFI_HSS_HSSPRTREADY_EV)       mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSPRTREADY_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSPRTREADY_HI_MASK);
    if (ev_mask & VTSS_XFI_HSS_RXPRBSSYNC_EV)        mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXPRBSSYNC_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXPRBSSYNC_HI_MASK);
    if (ev_mask & VTSS_XFI_HSS_RXPRBSERR_EV)         mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXPRBSERR_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXPRBSERR_HI_MASK);
    if (ev_mask & VTSS_XFI_HSS_RXASIGDET_EV)         mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXASIGDET_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXASIGDET_HI_MASK);
    if (ev_mask & VTSS_XFI_HSS_RXA_LOS_EV)           mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXA_LOS_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXA_LOS_HI_MASK);
    if (ev_mask & VTSS_XFI_HSS_IFPLL_LOCK_EV)        mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_IFPLL_LOCK_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_IFPLL_LOCK_HI_MASK);
    if (ev_mask & VTSS_XFI_HSS_TX_FIFO_LOS_EV)       mask |= VTSS_M_XFI_XFI_CONTROL_HSS_MASK_TX_FIFO_LOS_HI_MASK;
    if (ev_mask & VTSS_XFI_HSS_RXBSIGDET_EV)         mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXBSIGDET_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXBSIGDET_HI_MASK);
    if (ev_mask & VTSS_XFI_HSS_RXB_LOS_EV)           mask |= (VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXB_LOS_LO_MASK | VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXB_LOS_HI_MASK);

    DAYTONA_WRM(VTSS_XFI_XFI_CONTROL_HSS_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if (ev_mask & VTSS_XFI_AE_AET_RUN_B_EV)          mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_RUN_B_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_RUN_B_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_AET_FRM_LCK_B_EV)      mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_B_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_B_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_PCS_RX_READY_B_EV)     mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_B_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_B_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_PCS_TX_READY_B_EV)     mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_B_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_B_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_FEC_NEGOTIATED_B_EV)   mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_B_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_B_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_TRAIN_OK_B_EV)         mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_B_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_B_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_TRAIN_FAIL_B_EV)       mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_B_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_B_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_TRAIN_LOCAL_B_EV)      mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_B_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_B_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_AET_RUN_A_EV)          mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_RUN_A_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_RUN_A_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_AET_FRM_LCK_A_EV)      mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_A_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_A_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_PCS_RX_READY_A_EV)     mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_A_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_A_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_PCS_TX_READY_A_EV)     mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_A_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_A_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_FEC_NEGOTIATED_A_EV)   mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_A_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_A_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_TRAIN_OK_A_EV)         mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_A_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_A_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_TRAIN_FAIL_A_EV)       mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_A_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_A_LO_MASK);
    if (ev_mask & VTSS_XFI_AE_TRAIN_LOCAL_A_EV)      mask |= (VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_A_HI_MASK | VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_A_LO_MASK);

    DAYTONA_WRM(VTSS_XFI_XFI_CONTROL_AE_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if (ev_mask & VTSS_XFI_OTN_FAS_POS_EV)           mask |= VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_FAS_POS_MASK;
    if (ev_mask & VTSS_XFI_OTN_LOM_EV)               mask |= VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOM_MASK; 
    if (ev_mask & VTSS_XFI_OTN_OOM_EV)               mask |= VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOM_MASK;
    if (ev_mask & VTSS_XFI_OTN_LOF_EV)               mask |= VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOF_MASK;
    if (ev_mask & VTSS_XFI_OTN_OOF_EV)               mask |= VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOF_MASK;

    DAYTONA_WRM(VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if (ev_mask & VTSS_XFI_ANEG_INCOMPATIBLE_LINK_EV)    mask |= VTSS_M_XFI_ANEG_CFG_ANEG_MASK_INCOMPATIBLE_LINK_MASK;
    if (ev_mask & VTSS_XFI_ANEG_PAR_DETECT_FAULT_EV)     mask |= VTSS_M_XFI_ANEG_CFG_ANEG_MASK_PAR_DETECT_FAULT_MASK;
    if (ev_mask & VTSS_XFI_ANEG_AN_GOOD_CHECK_EV)        mask |= VTSS_M_XFI_ANEG_CFG_ANEG_MASK_AN_GOOD_CHECK_MASK;
    if (ev_mask & VTSS_XFI_ANEG_PAGE_RX_EV)              mask |= VTSS_M_XFI_ANEG_CFG_ANEG_MASK_PAGE_RX_MASK;
    if (ev_mask & VTSS_XFI_ANEG_ANEG_COMPLETE_EV)        mask |= VTSS_M_XFI_ANEG_CFG_ANEG_MASK_ANEG_COMPLETE_MASK;

    DAYTONA_WRM(VTSS_XFI_ANEG_CFG_ANEG_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}


static vtss_rc daytona_xfi_event_poll(vtss_state_t *vtss_state,
                                      const vtss_port_no_t  port_no,
                                      vtss_xfi_event_t      *const status)
{
    u32 target;
    u32 sticky, mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &target));

    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_HSS_STICKY(target), &sticky);
    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_HSS_MASK(target), &mask);
    sticky &= mask;
    DAYTONA_WR(VTSS_XFI_XFI_CONTROL_HSS_STICKY(target), sticky);

    *status = 0;

    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPLLLOCK_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPLLLOCK_HI_STICKY))        *status |= VTSS_XFI_HSS_HSSPLLLOCK_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSRESETOUT_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSRESETOUT_HI_STICKY))      *status |= VTSS_XFI_HSS_HSSRESETOUT_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSEYEQUALITY_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSEYEQUALITY_HI_STICKY))  *status |= VTSS_XFI_HSS_HSSEYEQUALITY_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPRTREADY_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPRTREADY_HI_STICKY))      *status |= VTSS_XFI_HSS_HSSPRTREADY_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSSYNC_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSSYNC_HI_STICKY))        *status |= VTSS_XFI_HSS_RXPRBSSYNC_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSERR_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSERR_HI_STICKY))          *status |= VTSS_XFI_HSS_RXPRBSERR_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXASIGDET_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXASIGDET_HI_STICKY))          *status |= VTSS_XFI_HSS_RXASIGDET_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXA_LOS_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXA_LOS_HI_STICKY))              *status |= VTSS_XFI_HSS_RXA_LOS_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_IFPLL_LOCK_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_IFPLL_LOCK_HI_STICKY))        *status |= VTSS_XFI_HSS_IFPLL_LOCK_EV;
    if (sticky & VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_TX_FIFO_LOS_HI_STICKY)                                                                  *status |= VTSS_XFI_HSS_TX_FIFO_LOS_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXBSIGDET_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXBSIGDET_HI_STICKY))          *status |= VTSS_XFI_HSS_RXBSIGDET_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXB_LOS_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXB_LOS_HI_STICKY))              *status |= VTSS_XFI_HSS_RXB_LOS_EV;

    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_AE_STICKY(target), &sticky);
    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_AE_MASK(target), &mask);
    sticky &= mask;
    DAYTONA_WR(VTSS_XFI_XFI_CONTROL_AE_STICKY(target), sticky);

    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_B_LO_STICKY))                *status |= VTSS_XFI_AE_AET_RUN_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_B_LO_STICKY))        *status |= VTSS_XFI_AE_AET_FRM_LCK_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_B_LO_STICKY))      *status |= VTSS_XFI_AE_PCS_RX_READY_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_B_LO_STICKY))      *status |= VTSS_XFI_AE_PCS_TX_READY_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_B_LO_STICKY))  *status |= VTSS_XFI_AE_FEC_NEGOTIATED_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_B_LO_STICKY))              *status |= VTSS_XFI_AE_TRAIN_OK_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_B_LO_STICKY))          *status |= VTSS_XFI_AE_TRAIN_FAIL_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_B_LO_STICKY))        *status |= VTSS_XFI_AE_TRAIN_LOCAL_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_A_LO_STICKY))                *status |= VTSS_XFI_AE_AET_RUN_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_A_LO_STICKY))        *status |= VTSS_XFI_AE_AET_FRM_LCK_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_A_LO_STICKY))      *status |= VTSS_XFI_AE_PCS_RX_READY_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_A_LO_STICKY))      *status |= VTSS_XFI_AE_PCS_TX_READY_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_A_LO_STICKY))  *status |= VTSS_XFI_AE_FEC_NEGOTIATED_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_A_LO_STICKY))              *status |= VTSS_XFI_AE_TRAIN_OK_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_A_LO_STICKY))          *status |= VTSS_XFI_AE_TRAIN_FAIL_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_A_LO_STICKY))        *status |= VTSS_XFI_AE_TRAIN_LOCAL_A_EV;

    DAYTONA_RD(VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY(target), &sticky);
    DAYTONA_RD(VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK(target), &mask);

    sticky &= mask;
    DAYTONA_WR(VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY(target), sticky);

    if (sticky & VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_FAS_POS_STICKY)     *status |= VTSS_XFI_OTN_FAS_POS_EV;
    if (sticky & VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOM_STICKY)         *status |= VTSS_XFI_OTN_LOM_EV;
    if (sticky & VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOM_STICKY)         *status |= VTSS_XFI_OTN_OOM_EV;
    if (sticky & VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOF_STICKY)         *status |= VTSS_XFI_OTN_LOF_EV;
    if (sticky & VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOF_STICKY)         *status |= VTSS_XFI_OTN_OOF_EV;

    DAYTONA_RD(VTSS_XFI_ANEG_STATUS_ANEG_STATUS(target), &sticky);
    DAYTONA_RD(VTSS_XFI_ANEG_CFG_ANEG_MASK(target), &mask);

    sticky &= mask;
    DAYTONA_WR(VTSS_XFI_ANEG_STATUS_ANEG_STATUS(target), sticky);

    if (sticky & VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK_STICKY)     *status |= VTSS_XFI_ANEG_INCOMPATIBLE_LINK_EV;
    if (sticky & VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_PAR_DETECT_FAULT_STICKY)      *status |= VTSS_XFI_ANEG_PAR_DETECT_FAULT_EV;
    if (sticky & VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_AN_GOOD_CHECK_STICKY)         *status |= VTSS_XFI_ANEG_AN_GOOD_CHECK_EV;
    if (sticky & VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_PAGE_RX_STICKY)               *status |= VTSS_XFI_ANEG_PAGE_RX_EV;
    if (sticky & VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE_STICKY)         *status |= VTSS_XFI_ANEG_ANEG_COMPLETE_EV;

    return VTSS_RC_OK;
}


static vtss_rc daytona_xfi_event_poll_without_mask(vtss_state_t *vtss_state,
                                                   const vtss_port_no_t  port_no,
                                                   vtss_xfi_event_t      *const status)
{
    u32 target;
    u32 sticky;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &target));

    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_HSS_STICKY(target), &sticky);
    DAYTONA_WR(VTSS_XFI_XFI_CONTROL_HSS_STICKY(target), sticky);

    *status = 0;

    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPLLLOCK_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPLLLOCK_HI_STICKY))        *status |= VTSS_XFI_HSS_HSSPLLLOCK_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSRESETOUT_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSRESETOUT_HI_STICKY))      *status |= VTSS_XFI_HSS_HSSRESETOUT_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSEYEQUALITY_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSEYEQUALITY_HI_STICKY))  *status |= VTSS_XFI_HSS_HSSEYEQUALITY_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPRTREADY_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPRTREADY_HI_STICKY))      *status |= VTSS_XFI_HSS_HSSPRTREADY_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSSYNC_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSSYNC_HI_STICKY))        *status |= VTSS_XFI_HSS_RXPRBSSYNC_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSERR_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSERR_HI_STICKY))          *status |= VTSS_XFI_HSS_RXPRBSERR_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXASIGDET_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXASIGDET_HI_STICKY))          *status |= VTSS_XFI_HSS_RXASIGDET_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXA_LOS_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXA_LOS_HI_STICKY))              *status |= VTSS_XFI_HSS_RXA_LOS_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_IFPLL_LOCK_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_IFPLL_LOCK_HI_STICKY))        *status |= VTSS_XFI_HSS_IFPLL_LOCK_EV;
    if (sticky & VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_TX_FIFO_LOS_HI_STICKY)                                                                  *status |= VTSS_XFI_HSS_TX_FIFO_LOS_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXBSIGDET_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXBSIGDET_HI_STICKY))          *status |= VTSS_XFI_HSS_RXBSIGDET_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXB_LOS_LO_STICKY | VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXB_LOS_HI_STICKY))              *status |= VTSS_XFI_HSS_RXB_LOS_EV;

    DAYTONA_RD(VTSS_XFI_XFI_CONTROL_AE_STICKY(target), &sticky);
    DAYTONA_WR(VTSS_XFI_XFI_CONTROL_AE_STICKY(target), sticky);

    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_B_LO_STICKY))                *status |= VTSS_XFI_AE_AET_RUN_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_B_LO_STICKY))        *status |= VTSS_XFI_AE_AET_FRM_LCK_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_B_LO_STICKY))      *status |= VTSS_XFI_AE_PCS_RX_READY_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_B_LO_STICKY))      *status |= VTSS_XFI_AE_PCS_TX_READY_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_B_LO_STICKY))  *status |= VTSS_XFI_AE_FEC_NEGOTIATED_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_B_LO_STICKY))              *status |= VTSS_XFI_AE_TRAIN_OK_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_B_LO_STICKY))          *status |= VTSS_XFI_AE_TRAIN_FAIL_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_B_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_B_LO_STICKY))        *status |= VTSS_XFI_AE_TRAIN_LOCAL_B_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_A_LO_STICKY))                *status |= VTSS_XFI_AE_AET_RUN_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_A_LO_STICKY))        *status |= VTSS_XFI_AE_AET_FRM_LCK_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_A_LO_STICKY))      *status |= VTSS_XFI_AE_PCS_RX_READY_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_A_LO_STICKY))      *status |= VTSS_XFI_AE_PCS_TX_READY_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_A_LO_STICKY))  *status |= VTSS_XFI_AE_FEC_NEGOTIATED_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_A_LO_STICKY))              *status |= VTSS_XFI_AE_TRAIN_OK_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_A_LO_STICKY))          *status |= VTSS_XFI_AE_TRAIN_FAIL_A_EV;
    if (sticky & (VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_A_HI_STICKY | VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_A_LO_STICKY))        *status |= VTSS_XFI_AE_TRAIN_LOCAL_A_EV;

    DAYTONA_RD(VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY(target), &sticky);
    DAYTONA_WR(VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY(target), sticky);

    if (sticky & VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_FAS_POS_STICKY)     *status |= VTSS_XFI_OTN_FAS_POS_EV;
    if (sticky & VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOM_STICKY)         *status |= VTSS_XFI_OTN_LOM_EV;
    if (sticky & VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOM_STICKY)         *status |= VTSS_XFI_OTN_OOM_EV;
    if (sticky & VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOF_STICKY)         *status |= VTSS_XFI_OTN_LOF_EV;
    if (sticky & VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOF_STICKY)         *status |= VTSS_XFI_OTN_OOF_EV;

    DAYTONA_RD(VTSS_XFI_ANEG_STATUS_ANEG_STATUS(target), &sticky);
    DAYTONA_WR(VTSS_XFI_ANEG_STATUS_ANEG_STATUS(target), sticky);

    if (sticky & VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK_STICKY)     *status |= VTSS_XFI_ANEG_INCOMPATIBLE_LINK_EV;
    if (sticky & VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_PAR_DETECT_FAULT_STICKY)      *status |= VTSS_XFI_ANEG_PAR_DETECT_FAULT_EV;
    if (sticky & VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_AN_GOOD_CHECK_STICKY)         *status |= VTSS_XFI_ANEG_AN_GOOD_CHECK_EV;
    if (sticky & VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_PAGE_RX_STICKY)               *status |= VTSS_XFI_ANEG_PAGE_RX_EV;
    if (sticky & VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE_STICKY)         *status |= VTSS_XFI_ANEG_ANEG_COMPLETE_EV;

    return VTSS_RC_OK;
}

static i32 coef_trad_override [xfi_chloss_max][txeq_ffe_max_coef] = {{0, 115 , 0},
                                                                     {0, 110 ,-20},
                                                                     {0, 115, 0},
                                                                     {0, 90, -40}};

static i32 coef_trad [xfi_chloss_max][txeq_ffe_max_coef] = {{0, 63, 0},
                                                            {0, 63, -20},
                                                            {0, 63, 0},
                                                            {0, 50,-31}};

static vtss_rc daytona_xfi_txeq_mode_set(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no)
{
    u32 tgt;
    u32 ntxpol;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &tgt));

    switch (vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.mode) {
    case XFI_TXEQ_TRADITIONAL_OVERRIDE:
        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE);

        ntxpol = 0;
        (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][0] > 0) ? (ntxpol = 1) : (ntxpol = 0);
        (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][1] > 0) ? (ntxpol |= (1 << 1)) : (ntxpol |= (0 << 1));
        (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][2] > 0) ? (ntxpol |= (1 << 2)) : (ntxpol |= (0 << 2));

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL(ntxpol),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL);
        
        vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[0] = (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][0] < 0) ?
                                                             -(coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][0]) :
                                                             (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][0]);
        vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[1] = (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][1] < 0) ?
                                                             -(coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][1]) :
                                                             (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][1]);
        vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[2] = (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][2] < 0) ?
                                                             -(coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][2]) :
                                                             (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][2]);

        vtss_state->xfi_state[port_no].xfi_tx_cfg.polarity[0] = 
                             (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][0] < 0) ? 0 : 1; 
        vtss_state->xfi_state[port_no].xfi_tx_cfg.polarity[1] = 
                             (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][1] < 0) ? 0 : 1;
        vtss_state->xfi_state[port_no].xfi_tx_cfg.polarity[2] = 
                             (coef_trad_override[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][2] < 0) ? 0 : 1;

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE_NIDAC0(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[0]),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE_NIDAC0);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE_NIDAC1(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[1]),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE_NIDAC1);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE_NIDAC2(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[2]),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE_NIDAC2);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);
        break;

    case XFI_TXEQ_TRADITIONAL: 
        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE);

        ntxpol = 0;
        (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][0] > 0) ? (ntxpol = 1) : (ntxpol = 0);
        (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][1] > 0) ? (ntxpol |= (1 << 1)) : (ntxpol |= (0 << 1));
        (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][2] > 0) ? (ntxpol |= (1 << 2)) : (ntxpol |= (0 << 2));

        vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[0] = (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][0] < 0) ?
                                                             -(coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][0]) :
                                                             (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][0]);

        vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[1] = (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][1] < 0) ?
                                                             -(coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][1]) :
                                                             (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][1]);
                                                              
        vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[2] = (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][2] < 0) ?
                                                             -(coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][2]) :
                                                             (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][2]);

        vtss_state->xfi_state[port_no].xfi_tx_cfg.polarity[0] = 
                             (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][0] < 0) ? 0 : 1; 
        vtss_state->xfi_state[port_no].xfi_tx_cfg.polarity[1] = 
                             (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][1] < 0) ? 0 : 1;
        vtss_state->xfi_state[port_no].xfi_tx_cfg.polarity[2] = 
                             (coef_trad[vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.channel_loss][2] < 0) ? 0 : 1;

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL(ntxpol),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT_NXTT0(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[0]),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT_NXTT0);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT_NXTT1(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[1]),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT_NXTT1);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT_NXTT2(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[2]),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT_NXTT2);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);
        break;

    case XFI_TXEQ_8023AP_OVERRIDE:
        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_CXINIT(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_CXINIT);
        break;

    case XFI_TXEQ_8023AP:
        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_CXINIT(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_CXINIT);

        /* LANE A */
        DAYTONA_WRM (VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A(tgt),
            VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT(1),
            VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT);

        DAYTONA_WRM (VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A(tgt),
            VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT(0),
            VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT);

        /* LANE B */
        DAYTONA_WRM (VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B(tgt),
            VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT(1),
            VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT);

        DAYTONA_WRM (VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B(tgt),
            VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT(0),
            VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT);
       
        /* LANE A */
        DAYTONA_WRM (VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A(tgt),
            VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT(1),
            VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT);

        DAYTONA_WRM (VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A(tgt),
            VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT(0),
            VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT);

        /* LANE B */
        DAYTONA_WRM (VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B(tgt),
            VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT(1),
            VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT);

        DAYTONA_WRM (VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B(tgt),
            VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT(0),
            VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT);
        break;

    default:
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_xfi_txeq_coef_adjust(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no,
                                            vtss_xfi_ffe_coef_t coef,
                                            BOOL polarity)
{
    u32 tgt;
    u32 ntxpol = 0; 

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &tgt));

    DAYTONA_RDF (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY(tgt,2), 0, 3, &ntxpol); 

    switch (vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.mode) {
    case XFI_TXEQ_TRADITIONAL_OVERRIDE:
        if (coef == txeq_ffe_coef_0) { 
            DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE(tgt, 2),
                VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE_NIDAC0(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[0]),
                VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE_NIDAC0);
            polarity ? ( ntxpol |= (u32)1) : (ntxpol &= 0xfffffffe);
        } else if (coef == txeq_ffe_coef_1) {
            DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE(tgt, 2),
                VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE_NIDAC1(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[1]),
                VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE_NIDAC1);
            polarity ? ( ntxpol |= (u32)(1 << 1)) : (ntxpol &= 0xfffffffd);
        } else if (coef == txeq_ffe_coef_2) {
            DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE(tgt, 2),
                VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE_NIDAC2(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[2]),
                VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE_NIDAC2);
            polarity ? ( ntxpol |= (u32)(1 << 2)) : (ntxpol &= 0xfffffffb);
        }

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL(ntxpol),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);
        break;

    case XFI_TXEQ_TRADITIONAL:
        if (coef == txeq_ffe_coef_0) {
        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT_NXTT0(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[0]),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT_NXTT0);
            polarity ? ( ntxpol |= (u32)1) : (ntxpol &= 0xfffffffe);
        } else if (coef == txeq_ffe_coef_1) {
        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT_NXTT1(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[1]),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT_NXTT1);
            polarity ? ( ntxpol |= (u32)(1 << 1)) : (ntxpol &= 0xfffffffd);
        } else if (coef == txeq_ffe_coef_2) {
        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT_NXTT2(vtss_state->xfi_state[port_no].xfi_tx_cfg.coefs[2]),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT_NXTT2);
            polarity ? ( ntxpol |= (u32)(1 << 2)) : (ntxpol &= 0xfffffffb);
        }
        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL(ntxpol),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);
        break;
    default:
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_xfi_txamp_set(vtss_state_t *vtss_state,
                                     const vtss_port_no_t port_no)
{
    u32 tgt;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &tgt));

    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER(tgt, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_TXPWR(vtss_state->xfi_state[port_no].xfi_tx_cfg.transmit_power),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_TXPWR);

    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(1),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);

    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(0),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);

    return VTSS_RC_OK;
}

static vtss_rc daytona_xfi_txslew_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no)
{
    u32 tgt;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &tgt));

    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL(tgt, 2),
        VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_SLEW(vtss_state->xfi_state[port_no].xfi_tx_cfg.slew_rate),
        VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_SLEW);

    return VTSS_RC_OK;
}

static vtss_rc daytona_xfi_rxeq_mode_set(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no)
{
    u32 tgt;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &tgt));

    DAYTONA_WRM (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(tgt, 2),
        VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_DMSEL(0),
        VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_DMSEL);

    return VTSS_RC_OK;
}

static vtss_rc daytona_xfi_txeq_8023ap_coef_update(vtss_state_t *vtss_state,
                                                   const vtss_port_no_t port_no, vtss_xfi_lane_t lane, 
                                                   vtss_xfi_ffe_coef_t coef, vtss_txeq_8023ap_updreq_t updreq)
{
    u32 tgt;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &tgt));

    if (lane == xfi_lane_a) {
        switch (coef) {
        case txeq_ffe_coef_0:
            DAYTONA_WRM(VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A(tgt), 
                        VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C0(updreq),
                        VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C0);
            break;
        case txeq_ffe_coef_1:
            DAYTONA_WRM(VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A(tgt), 
                        VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C1(updreq),
                        VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C1);
             break;
        case txeq_ffe_coef_2:
            DAYTONA_WRM(VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A(tgt), 
                        VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C2(updreq),
                        VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C2);
            break;
        case txeq_ffe_max_coef:
        default:
            return VTSS_RC_ERROR;
        }
    } else if (lane == xfi_lane_b) {
        switch (coef) {
        case txeq_ffe_coef_0:
            DAYTONA_WRM(VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B(tgt), 
                        VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C0(updreq),
                        VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C0);
            break;
        case txeq_ffe_coef_1:
            DAYTONA_WRM(VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B(tgt), 
                        VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C1(updreq),
                        VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C1);
             break;
        case txeq_ffe_coef_2:
            DAYTONA_WRM(VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B(tgt), 
                        VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C2(updreq),
                        VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C2);
            break;
        case txeq_ffe_max_coef:
        default:
            return VTSS_RC_ERROR;
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_xfi_txeq_8023ap_coef_stat_get(vtss_state_t *vtss_state,
                                                     const vtss_port_no_t port_no, 
                                                     vtss_xfi_lane_t lane, 
                                                     vtss_txeq_8023ap_coef_stat_t *ae_status)
{
    u32 tgt;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &tgt));

    if (vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.mode == XFI_TXEQ_8023AP_OVERRIDE) {
        DAYTONA_RDF(VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS(tgt, lane%2),0,2, &ae_status->state_c0)
        DAYTONA_RDF(VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS(tgt, lane%2),2,2, &ae_status->state_c1)
        DAYTONA_RDF(VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS(tgt, lane%2),4,2, &ae_status->state_c2)
    } else if (vtss_state->xfi_state[port_no].xfi_tx_cfg.txeq_cfg.mode == XFI_TXEQ_8023AP) {
        if (lane == xfi_lane_a) {
            DAYTONA_RDF(VTSS_XFI_AE_REGS_A_RX_COEF_STAT_A(tgt),4,2, &ae_status->state_c0);
            DAYTONA_RDF(VTSS_XFI_AE_REGS_A_RX_COEF_STAT_A(tgt),2,2, &ae_status->state_c1);
            DAYTONA_RDF(VTSS_XFI_AE_REGS_A_RX_COEF_STAT_A(tgt),0,2, &ae_status->state_c2);
        } else if (lane == xfi_lane_b) {
            DAYTONA_RDF(VTSS_XFI_AE_REGS_B_RX_COEF_STAT_B(tgt),4,2, &ae_status->state_c0);
            DAYTONA_RDF(VTSS_XFI_AE_REGS_B_RX_COEF_STAT_B(tgt),2,2, &ae_status->state_c1);
            DAYTONA_RDF(VTSS_XFI_AE_REGS_B_RX_COEF_STAT_B(tgt),0,2, &ae_status->state_c2);
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_xfi_txeq_8023ap_fsm_ctl_set(vtss_state_t *vtss_state,
                                                   const vtss_port_no_t port_no, vtss_xfi_lane_t lane, u32 val)
{
    u32 tgt;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &tgt));
    if (lane == xfi_lane_a) {
        DAYTONA_WR(VTSS_XFI_AE_REGS_A_FSM_CTL_A(tgt), val);
    } else if (lane == xfi_lane_b) {
        DAYTONA_WR(VTSS_XFI_AE_REGS_B_FSM_CTL_B(tgt), val);
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_xfi_txeq_8023ap_fsm_stat_get(vtss_state_t *vtss_state,
                                                    const vtss_port_no_t port_no, vtss_xfi_lane_t lane, u32 *val)
{
    u32 tgt;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &tgt));

    if (lane == xfi_lane_a) {
        DAYTONA_RD(VTSS_XFI_AE_REGS_A_FSM_STATE_A(tgt), val);
    } else if (lane == xfi_lane_b) {
        DAYTONA_RD(VTSS_XFI_AE_REGS_B_FSM_STATE_B(tgt), val);
    }

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Utilities and internal
 * ================================================================= */


/**
 * \brief Static Initialization of the configuration data base in the vtss_state.
 **/
static vtss_xfi_cfg_t cfg_default = {
    .sel_lane_b         = FALSE,
    .equipment_loopback = FALSE,
    .line_loopback      = FALSE,
    .bypass_aneg        = FALSE,
    .bypass_ae          = FALSE,
};

static vtss_xfi_tx_cfg_t tx_cfg_default = {
    .txeq_cfg                 = {.mode = XFI_TXEQ_TRADITIONAL_OVERRIDE, .channel_loss = xfi_chloss_lt10inch },
    .transmit_power           = 0x7F,
    .coefs                    = {[txeq_ffe_coef_0] = 0, [txeq_ffe_coef_1] = 115 , [txeq_ffe_coef_2] =0},
    .polarity                 = {[txeq_ffe_coef_0] = 1, [txeq_ffe_coef_2] = 1, [txeq_ffe_coef_2] = 1},
};


static vtss_xfi_test_cfg_t test_cfg_default = {
    .test_cfg_gen = VTSS_XFI_TEST_MODE_DISABLE,
    .test_cfg_chk = VTSS_XFI_TEST_MODE_DISABLE,
};

static vtss_xfi_rxeq_cfg_t rx_cfg_default = {
    .channel_loss = xfi_chloss_lt10inch,
};

/*
 * \brief Restart xfi configuration on a port
 *
 * \param port_no [IN] Port number
 * \return Return code
 */
vtss_rc vtss_daytona_xfi_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_rc rc;

    VTSS_D("xfi restart conf set, port_count %d", vtss_state->port_count);
    rc = daytona_xfi_config_set(vtss_state, port_no);
    rc = daytona_xfi_test_config_set(vtss_state, port_no);
    VTSS_D("port_no %d, xfi restart conf set, rx %x", port_no, rc);
    return VTSS_RC_OK;
}

/*
 * \brief Set default xfi configuration for a port
 *
 * \param port_no [IN] Port number
 * \return Return code
 */
vtss_rc vtss_daytona_xfi_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_state->xfi_state[port_no].xfi_cfg = cfg_default;
    vtss_state->xfi_state[port_no].xfi_test_cfg = test_cfg_default;
    vtss_state->xfi_state[port_no].xfi_tx_cfg = tx_cfg_default;
    vtss_state->xfi_state[port_no].xfi_rxeq_cfg = rx_cfg_default;
    vtss_state->xfi_state[port_no].xfi_cfg.xfi_rec_clock_output.lane_a = TRUE;
    vtss_state->xfi_state[port_no].xfi_cfg.xfi_rec_clock_output.lane_b = FALSE;
    
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
vtss_rc vtss_daytona_inst_xfi_create(vtss_state_t *vtss_state)
{
    vtss_port_no_t port_no;
    vtss_cil_func_t *func = &vtss_state->cil;

    for (port_no = 0 ; port_no < VTSS_PORT_ARRAY_SIZE; port_no++) {
        vtss_state->xfi_state[port_no].xfi_cfg = cfg_default;
        vtss_state->xfi_state[port_no].xfi_test_cfg = test_cfg_default;
        vtss_state->xfi_state[port_no].xfi_tx_cfg = tx_cfg_default;
        vtss_state->xfi_state[port_no].xfi_cfg.xfi_rec_clock_output.lane_a = TRUE;
        vtss_state->xfi_state[port_no].xfi_cfg.xfi_rec_clock_output.lane_b = FALSE;
    }

    /* Daytona XFI functions */
    func->xfi_config_set   = daytona_xfi_config_set;
    func->xfi_status_get   = daytona_xfi_status_get;
    func->xfi_test_config_set = daytona_xfi_test_config_set;
    func->xfi_test_status_get = daytona_xfi_test_status_get;
    func->xfi_event_poll   = daytona_xfi_event_poll;
    func->xfi_event_poll_without_mask   = daytona_xfi_event_poll_without_mask;
    func->xfi_event_enable = daytona_xfi_event_enable;
    func->xfi_ae_reset = daytona_xfi_ae_reset;
    func->xfi_recovered_clock_output_set = daytona_xfi_recovered_clock_output_set;
    func->xfi_txeq_mode_set = daytona_xfi_txeq_mode_set;
    func->xfi_txeq_coef_adjust = daytona_xfi_txeq_coef_adjust;
    func->xfi_txamp_set = daytona_xfi_txamp_set;
    func->xfi_txslew_set = daytona_xfi_txslew_set;
    func->xfi_rxeq_mode_set = daytona_xfi_rxeq_mode_set;
    func->xfi_txeq_8023ap_coef_update = daytona_xfi_txeq_8023ap_coef_update;
    func->xfi_txeq_8023ap_coef_stat_get = daytona_xfi_txeq_8023ap_coef_stat_get;
    func->xfi_txeq_8023ap_fsm_ctl_set = daytona_xfi_txeq_8023ap_fsm_ctl_set;
    func->xfi_txeq_8023ap_fsm_stat_get = daytona_xfi_txeq_8023ap_fsm_stat_get;

    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_DAYTONA */


