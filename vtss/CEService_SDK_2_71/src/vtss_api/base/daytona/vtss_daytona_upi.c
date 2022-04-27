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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_UPI

// Avoid "vtss_api.h not used in module vtss_daytona_upi.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_upi.h"
#include "vtss_daytona_regs_upi.h"



/* ================================================================= *
 *  Static Config
 * ================================================================= */


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */
//static u32 vtss_proto_bitfield(vtss_upi_protocol_t protocol)
//{
//    switch (protocol)
//    {
//        case tfi5: return 2;
//        case xaui: return 0;
//        case sfi4: return 1;
//        default: return 0;
//    }
//}

/**
 * \brief Dynamic Config set config.
 *
 * \param port_no [IN]   Port number.
 *
 * \return Return code.
 **/
static vtss_rc daytona_upi_config_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));
    VTSS_I("port_no %d, target %X", port_no, target);

    /* loopback modes  */
    DAYTONA_WRM(VTSS_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG(target),
                VTSS_F_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG_TX_TO_RX_LB(vtss_state->upi_state[port_no].upi_cfg.tx_to_rx_loopback) |
                VTSS_F_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG_RX_TO_TX_LB(vtss_state->upi_state[port_no].upi_cfg.rx_to_tx_loopback),
                VTSS_M_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG_TX_TO_RX_LB | VTSS_M_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG_RX_TO_TX_LB);

    /* 6G Equalizer initialization */ 
    
    DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(1),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL);

    DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(0),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL);

    DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC(1),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC);

    DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD(2),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD);

    DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1(255),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1);

    DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(1),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);

    DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(0),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);

    DAYTONA_WRM (VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE(target,4),
        VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_DMSEL(0),
        VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_DMSEL);

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  State Reporting
 * ================================================================= */
/**
 * \brief status get from UPI interface.
 *
 * \param port_no [IN]   Port number.
 * \param status [OUT]   returned UPI functional block status.
 *
 * \return Return code.
 **/
static vtss_rc daytona_upi_status_get(vtss_state_t *vtss_state,
                                      const vtss_port_no_t  port_no, vtss_upi_status_t *const status)
{
    u32 target;
    u32 upi_status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));

    DAYTONA_RD(VTSS_UPI_UPI_CONFIG_UPI_STATUS(target), &upi_status);
    VTSS_D("port_no %d, target %X, upi_status %X", port_no, target, upi_status);

    status->ssf = VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_SSF(upi_status) != 0;
    status->rx_los = VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_RX_LOS(upi_status) != 0;
    status->tx_fifo_oflow = VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_TX_FIFO_OVERFLOW(upi_status) != 0;
    status->tx_fifo_uflow = VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_TX_FIFO_UNDERFLOW(upi_status) != 0;

    status->hss_status.pll_lock = VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_PLLLOCK(upi_status) != 0;
    status->hss_status.hss_rst_out = VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_HSS_RESETOUT(upi_status) != 0;
    status->hss_status.hss_eyequality = VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_EYE_QUALITY_ERR(upi_status) != 0;
    status->hss_status.rx_sigdet[0] = (VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_RX_SIGDET(upi_status) & 0x01) != 0;
    status->hss_status.rx_sigdet[1] = (VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_RX_SIGDET(upi_status) & 0x02) != 0;
    status->hss_status.rx_sigdet[2] = (VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_RX_SIGDET(upi_status) & 0x04) != 0;
    status->hss_status.rx_sigdet[3] = (VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_RX_SIGDET(upi_status) & 0x08) != 0;

    DAYTONA_RD(VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG(target), &upi_status);
    status->upi_2lane = VTSS_X_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_DDR_MODE(upi_status) != 0;
    status->proto = VTSS_X_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_PROTO_SEL(upi_status) == 1 ? sfi4 :
                               (VTSS_X_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_PROTO_SEL(upi_status) == 2 ? tfi5 : xaui);
    
    
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
static vtss_rc daytona_upi_test_config_set(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no)
{
    vtss_rc upi_rc = VTSS_RC_OK;
    u32 target;
    vtss_upi_test_mode_t tst_pat;
    u32 patsel = 0, prbs_en = 0;
    u32 lben = 0, ri;
    u32 wrpmd = 0, wrpen = 0;
    vtss_upi_test_wrap_t wrap;
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));
    VTSS_I("port_no %d, target %X", port_no, target);

    /* set up test generator */
    tst_pat = vtss_state->upi_state[port_no].upi_test_cfg.test_cfg_gen;
    switch (tst_pat) {
        case VTSS_UPI_TEST_MODE_DISABLE: patsel = 0; prbs_en = 0; break;
        case VTSS_UPI_TEST_MODE_PRBS7_P: patsel = 0; prbs_en = 0xf; 
            lben = 1;
            break;
        case VTSS_UPI_TEST_MODE_PRBS7_N: patsel = 1; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_PRBS23_P: patsel = 2; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_PRBS23_N: patsel = 3; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_PRBS31_P: patsel = 4; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_PRBS31_N: patsel = 5; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_HF: patsel = 6; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_LF: patsel = 7; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_PRBS9_P:
        default:
            VTSS_E("Invalid test generator config %d", tst_pat);
            upi_rc = VTSS_RC_ERROR;
            break;
    }
    VTSS_D("port_no %d, gen mode %d, patsel %d, prbs_en %d, lben %d", port_no, tst_pat, patsel, prbs_en, lben);
    /* broadcast pattern setting to all 4 lanes */
    for (ri = 0 ; ri < 4; ri++) {
        /* enable/disable prbs generator */
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL(target,ri),
                    VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_PATSEL(patsel),
                    VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_PATSEL);
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL(target,ri),
                    VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBEN(lben),
                    VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBEN);
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL(target,ri),
                    VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_TPDSEL(prbs_en!=0 ? 1:0),
                    VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_TPDSEL);
            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL(target,ri),
                        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBRST(1),
                        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBRST);
            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL(target,ri),
                        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBRST(0),
                        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBRST);
    }
    /* enable counter */
    DAYTONA_WRM(VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS(target),
                VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSERR_CNTR_EN(prbs_en),
                VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSERR_CNTR_EN);

    /* set up test checker */
    tst_pat = vtss_state->upi_state[port_no].upi_test_cfg.test_cfg_chk;
    switch (tst_pat) {
        case VTSS_UPI_TEST_MODE_DISABLE: patsel = 0; prbs_en = 0; break;
        case VTSS_UPI_TEST_MODE_PRBS7_P: patsel = 0; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_PRBS7_N: patsel = 1; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_PRBS23_P: patsel = 2; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_PRBS23_N: patsel = 3; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_PRBS31_P: patsel = 4; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_PRBS31_N: patsel = 5; prbs_en = 0xf; break;
        case VTSS_UPI_TEST_MODE_HF:
        case VTSS_UPI_TEST_MODE_LF:
        case VTSS_UPI_TEST_MODE_PRBS9_P:
        default:
            VTSS_E("Invalid test checker config %d", tst_pat);
            upi_rc = VTSS_RC_ERROR;
            break;
    }
    wrap = vtss_state->upi_state[port_no].upi_test_cfg.wrap_mode;
    switch (wrap) {
        case VTSS_UPI_TEST_WRAP_RX: wrpmd = 0; wrpen = 0; break;
        case VTSS_UPI_TEST_WRAP_INT_LOOP: wrpmd = 0; wrpen = 1; break;
        case VTSS_UPI_TEST_WRAP_INT_WRAP: wrpmd = 1; wrpen = 1; break;
        default:
            VTSS_E("Invalid test rx wrap mode %d", wrap);
            upi_rc = VTSS_RC_ERROR;
            break;
    }
    VTSS_D("port_no %d, chk mode %d, patsel %d, prbs_en %d, wrpmd %d, wrpen %d", port_no, tst_pat, patsel, prbs_en, wrpmd, wrpen);
    /* broadcast pattern setting to all 4 lanes */
    for (ri = 0 ; ri < 4; ri++) {
        DAYTONA_WRM(VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL(target,ri),
                    VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PATSEL(patsel),
                    VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PATSEL);
        DAYTONA_WRM(VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL(target,ri),
                    VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PCHKEN(prbs_en!=0 ? 1:0),
                    VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PCHKEN);
        DAYTONA_WRM(VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL(target,ri),
                    VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_WRPMD(wrpmd),
                    VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_WRPMD);
        DAYTONA_WRM(VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL(target,ri),
                    VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_WRPEN(wrpen),
                    VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_WRPEN);
        DAYTONA_WRM(VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL(target,ri),
                    VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PRST(1),
                    VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PRST);
        DAYTONA_WRM(VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL(target,ri),
                    VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PRST(0),
                    VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PRST);
    }
    /* enable/disable prbs checker */
    DAYTONA_WRM(VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS(target),
                VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSERR_CNTR_EN(prbs_en),
                VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSERR_CNTR_EN);

    return upi_rc;
}

/*
 * Test status get from UPI interface.
 *
 * param port_no [IN]   Port number.
 * param status [OUT]   returned UPI functional block test status.
 *
 * return Return code.
 */
static vtss_rc daytona_upi_test_status_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t  port_no, vtss_upi_test_status_t *const status)
{
    u32 target;
    u32 upi_status, hss_status;
    int ri;
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));

    DAYTONA_RD(VTSS_UPI_UPI_CONFIG_UPI_STATUS(target), &upi_status);
    
    for (ri = 0; ri < 4; ri++) {
        DAYTONA_RD(VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL(target,ri), &hss_status);
        VTSS_D("port_no %d, target %X, hss_status %X", port_no, target, hss_status);
        status[ri].lbsync = VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_SYNCST(hss_status);
        status[ri].lberror = VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_ERRST(hss_status);
        /* read and clear error counters */
        DAYTONA_RD(VTSS_UPI_UPI_HSS6G_CONFIG_STAT_RX_PRBSERR_CNTR(target,ri), &status[ri].rx_prbs_err_count);
        DAYTONA_WR(VTSS_UPI_UPI_HSS6G_CONFIG_STAT_RX_PRBSERR_CNTR(target,ri), 0);
        DAYTONA_RD(VTSS_UPI_UPI_HSS6G_CONFIG_STAT_TX_PRBSERR_CNTR(target,ri), &status[ri].tx_prbs_err_count);
        DAYTONA_WR(VTSS_UPI_UPI_HSS6G_CONFIG_STAT_TX_PRBSERR_CNTR(target,ri), 0);

        status[ri].tx_prbssync = (VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_TX_HSS_PRBSSYNC(upi_status) & (1<<ri)) != 0;
        status[ri].tx_prbs_err = (VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_TX_PRBS_ERR(upi_status) & (1<<ri)) != 0;

    }
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  DFE/FFE Equalization 
 * ================================================================= */

/***********************************************
 Mapping from Modes to co-efficients
 ***********************************************/

int xaui_override[upi_chloss_max][upi_txeq_coef_max] = {{0,255,  0,0},
                                                        {0,244,-20,0},
                    								    {0,244,-20,0}};
int xaui_normal[upi_chloss_max][upi_txeq_coef_max]  =  {{0,63,  0,0},
                                                        {0,63,-20,0},
                                                        {0,63,-20,0}};
int rxaui_override[upi_chloss_max][upi_txeq_coef_max] = {{0,255,  0,0},
                           							     {0,244,-20,0},
                                                         {0,244,-20,0}};
int rxaui_normal[upi_chloss_max][upi_txeq_coef_max]  =  {{0,63,  0,0},
                                                         {0,63,-20,0},
                                                         {0,50,-20,0}};

static vtss_rc daytona_upi_txeq_mode_set(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no)
{
    u32 target; 
    u32 ntxpol;
    vtss_upi_txeq_cfg_t * cfg = &vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));

/* 0 = four lane operation */
    if (vtss_state->init_conf.two_lane_upi == 0) {
        switch(cfg->mode) {

        case UPI_TXEQ_OVERRIDE:
            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC);
 
			DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL(target,4),
				VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD(2),
				VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD);

			ntxpol = 0;
            if (xaui_override[cfg->channel_loss][0] > 0) {
                ntxpol = 1;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0] = xaui_override[cfg->channel_loss][0];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0] = xaui_override[cfg->channel_loss][0] * -1;
                
            if (xaui_override[cfg->channel_loss][1] > 0) {
                ntxpol |= 1 << 1;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1] = xaui_override[cfg->channel_loss][1];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1] = xaui_override[cfg->channel_loss][1] * -1;

            if (xaui_override[cfg->channel_loss][2] > 0) {
                ntxpol |= 1 << 2;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2] = xaui_override[cfg->channel_loss][2];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2] = xaui_override[cfg->channel_loss][2] * -1;

            if (xaui_override[cfg->channel_loss][3] > 0) {
                ntxpol |= 1 << 3;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3] = xaui_override[cfg->channel_loss][3];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3] = xaui_override[cfg->channel_loss][3] * -1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[0] = 
                                (xaui_override[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][0] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[1] = 
                                (xaui_override[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][1] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[2] = 
                                (xaui_override[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][2] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[3] = 
                                (xaui_override[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][3] < 0) ? 0 : 1;

            DAYTONA_WRM (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL(ntxpol),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_NXTD0(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_NXTD0);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1]),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2(target,4), 
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_NXTD2(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_NXTD2);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3(target,4), 
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_NXTD3(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_NXTD3);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);
            break;
 
        case UPI_TXEQ_NORMAL:
            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC);
 
            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD(2),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD);

            ntxpol = 0;
            if (xaui_normal[cfg->channel_loss][0] > 0) {
                ntxpol = 1;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0] = xaui_normal[cfg->channel_loss][0];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0] = xaui_normal[cfg->channel_loss][0] * -1;
                
            if (xaui_normal[cfg->channel_loss][1] > 0) {
                ntxpol |= 1 << 1;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1] = xaui_normal[cfg->channel_loss][1];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1] = xaui_normal[cfg->channel_loss][1] * -1;

            if (xaui_normal[cfg->channel_loss][2] > 0) {
                ntxpol |= 1 << 2;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2] = xaui_normal[cfg->channel_loss][2];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2] = xaui_normal[cfg->channel_loss][2] * -1;

            if (xaui_normal[cfg->channel_loss][3] > 0) {
                ntxpol |= 1 << 3;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3] = xaui_normal[cfg->channel_loss][3];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3] = xaui_normal[cfg->channel_loss][3] * -1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[0] = 
                                (xaui_normal[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][0] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[1] = 
                                (xaui_normal[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][1] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[2] = 
                                (xaui_normal[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][2] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[3] = 
                                (xaui_normal[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][3] < 0) ? 0 : 1;

            DAYTONA_WRM (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL(ntxpol),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_NXTT0(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_NXTT0);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_NXTT1(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1]),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_NXTT1);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2(target,4), 
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_NXTT2(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_NXTT2);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3(target,4), 
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_NXTT3(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_NXTT3);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);
            break;
        default:
            return VTSS_RC_ERROR;
        }      
    }
    else {
        /* RXAUI    */
        switch (cfg->mode) {

        case UPI_TXEQ_OVERRIDE : 
            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC);
 
            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD(2),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD);

            ntxpol = 0;
            if (rxaui_override[cfg->channel_loss][0] > 0) {
                ntxpol = 1;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0] = rxaui_override[cfg->channel_loss][0];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0] = rxaui_override[cfg->channel_loss][0] * -1;
                
            if (rxaui_override[cfg->channel_loss][1] > 0) {
                ntxpol |= 1 << 1;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1] = rxaui_override[cfg->channel_loss][1];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1] = rxaui_override[cfg->channel_loss][1] * -1;

            if (rxaui_override[cfg->channel_loss][2] > 0) {
                ntxpol |= 1 << 2;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2] = rxaui_override[cfg->channel_loss][2];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2] = rxaui_override[cfg->channel_loss][2] * -1;

            if (rxaui_override[cfg->channel_loss][3] > 0) {
                ntxpol |= 1 << 3;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3] = rxaui_override[cfg->channel_loss][3];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3] = xaui_override[cfg->channel_loss][3] * -1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[0] = 
                                (rxaui_override[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][0] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[1] = 
                                (rxaui_override[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][1] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[2] = 
                                (rxaui_override[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][2] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[3] = 
                                (rxaui_override[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][3] < 0) ? 0 : 1;

            DAYTONA_WRM (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL(ntxpol),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_NXTD0(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_NXTD0);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1]),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2(target,4), 
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_NXTD2(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_NXTD2);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3(target,4), 
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_NXTD3(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_NXTD3);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);
            break;

        case UPI_TXEQ_NORMAL:
            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC);
 
            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD(2),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD);

            ntxpol = 0;
            if (rxaui_normal[cfg->channel_loss][0] > 0) {
                ntxpol = 1;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0] = rxaui_normal[cfg->channel_loss][0];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0] = rxaui_normal[cfg->channel_loss][0] * -1;
                
            if (rxaui_normal[cfg->channel_loss][1] > 0) {
                ntxpol |= 1 << 1;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1] = rxaui_normal[cfg->channel_loss][1];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1] = rxaui_normal[cfg->channel_loss][1] * -1;

            if (rxaui_normal[cfg->channel_loss][2] > 0) {
                ntxpol |= 1 << 2;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2] = rxaui_normal[cfg->channel_loss][2];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2] = rxaui_normal[cfg->channel_loss][2] * -1;

            if (rxaui_normal[cfg->channel_loss][3] > 0) {
                ntxpol |= 1 << 3;
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3] = rxaui_normal[cfg->channel_loss][3];
            }
            else
                vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3] = rxaui_normal[cfg->channel_loss][3] * -1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[0] = 
                                (rxaui_normal[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][0] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[1] = 
                                (rxaui_normal[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][1] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[2] = 
                                (rxaui_normal[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][2] < 0) ? 0 : 1;

            vtss_state->upi_state[port_no].upi_tx_cfg.polarity[3] = 
                                (rxaui_normal[vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.channel_loss][3] < 0) ? 0 : 1;

            DAYTONA_WRM (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL(ntxpol),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_NXTT0(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_NXTT0);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_NXTT1(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1]),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_NXTT1);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2(target,4), 
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_NXTT2(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_NXTT2);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3(target,4), 
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_NXTT3(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_NXTT3);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);

            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);
            break;
        default:
            return VTSS_RC_ERROR;
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_upi_txeq_coef_adjust(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no,
                                            vtss_upi_ffe_coef_t coef,
                                            BOOL polarity)
{
    u32 target;
    u32 ntxpol = 0;
    
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));

    DAYTONA_RDF (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY(target,4), 0, 3, &ntxpol);	

    if (vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.mode == 
                                                    UPI_TXEQ_OVERRIDE) {
        switch (coef) {
        case upi_txeq_coef_0 :
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0(target,4),
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_NXTD0(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0]), 
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_NXTD0);
        polarity ? ( ntxpol |= (u32)1) : (ntxpol &= 0xfffffffe);
		break;
			
        case upi_txeq_coef_1 :            
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1(target,4),
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1]),
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1);
        polarity ? ( ntxpol |= (u32)(1 << 1)) : (ntxpol &= 0xfffffffd);
        break;

        case upi_txeq_coef_2 :
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2(target,4), 
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_NXTD2(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2]), 
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_NXTD2);
			polarity ? ( ntxpol |= (u32)(1 << 2)) : (ntxpol &= 0xfffffffb);
        break;

        case upi_txeq_coef_3 :
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3(target,4), 
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_NXTD3(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3]), 
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_NXTD3);
        polarity ? ( ntxpol |= (u32)(1 << 3)) : (ntxpol &= 0xfffffff7);
        break;

        default :
            break;
		}

        DAYTONA_WRM (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY(target,4),
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL(ntxpol),
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL);
		
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(1),
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);

        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(0),
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);
    }	
    else if (vtss_state->upi_state[port_no].upi_tx_cfg.txeq_cfg.mode == 
                                                        UPI_TXEQ_NORMAL) {
        switch (coef) {
        case upi_txeq_coef_0 :			
            DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0(target,4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_NXTT0(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[0]), 
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_NXTT0);
        polarity ? ( ntxpol |= (u32)1) : (ntxpol &= 0xfffffffe);
            break;

        case upi_txeq_coef_1:
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1(target,4),
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_NXTT1(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[1]),
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_NXTT1);
        polarity ? ( ntxpol |= (u32)(1 << 1)) : (ntxpol &= 0xfffffffd);
        break;

        case upi_txeq_coef_2:
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2(target,4), 
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_NXTT2(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[2]), 
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_NXTT2);
        polarity ? ( ntxpol |= (u32)(1 << 2)) : (ntxpol &= 0xfffffffb);
        break;

        case upi_txeq_coef_3:
        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3(target,4), 
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_NXTT3(vtss_state->upi_state[port_no].upi_tx_cfg.coefs[3]), 
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_NXTT3);
        polarity ? ( ntxpol |= (u32)(1 << 3)) : (ntxpol &= 0xfffffff7);				
        break;
		
        default:
            return VTSS_RC_ERROR;
        }

        DAYTONA_WRM (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY(target,4),
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL(ntxpol),
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL);

        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(1),
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);

        DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
            VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(0),
            VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);
    }
    else
        return VTSS_RC_ERROR;

    return VTSS_RC_OK;
}		

static vtss_rc daytona_upi_txamp_set(vtss_state_t *vtss_state,
                                     const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));

    DAYTONA_WRM (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POWER(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_POWER_MXPWR(vtss_state->upi_state[port_no].upi_tx_cfg.transmit_power),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_POWER_MXPWR);

    DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(1),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);

    DAYTONA_WRM(VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(0),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);
		
    return VTSS_RC_OK;
}
	
static vtss_rc daytona_upi_txslew_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));

    DAYTONA_WRM (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL(target,4),
        VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_MNSLEW(vtss_state->upi_state[port_no].upi_tx_cfg.slew_rate),
        VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_MNSLEW);

    return VTSS_RC_OK;
}

static vtss_rc daytona_upi_rxeq_mode_set(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));

    DAYTONA_WRM (VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE(target,4),
        VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_DMSEL(0),
        VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_DMSEL);

    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
static vtss_rc daytona_upi_event_enable(vtss_state_t           *vtss_state,
                                        const vtss_port_no_t   port_no,
                                        const BOOL             enable,
                                        const vtss_upi_event_t ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));

    mask = 0;

    if (ev_mask & VTSS_UPI_RX_HSS_PRBSSYNC_0_EV)    mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_HSS_PRBSSYNC_MASK(0x1);
    if (ev_mask & VTSS_UPI_RX_HSS_PRBSSYNC_1_EV)    mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_HSS_PRBSSYNC_MASK(0x2);
    if (ev_mask & VTSS_UPI_RX_HSS_PRBSSYNC_2_EV)    mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_HSS_PRBSSYNC_MASK(0x4);
    if (ev_mask & VTSS_UPI_RX_HSS_PRBSSYNC_3_EV)    mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_HSS_PRBSSYNC_MASK(0x8);
    if (ev_mask & VTSS_UPI_TX_HSS_PRBSSYNC_0_EV)    mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_HSS_PRBSSYNC_MASK(0x1);
    if (ev_mask & VTSS_UPI_TX_HSS_PRBSSYNC_1_EV)    mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_HSS_PRBSSYNC_MASK(0x2);
    if (ev_mask & VTSS_UPI_TX_HSS_PRBSSYNC_2_EV)    mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_HSS_PRBSSYNC_MASK(0x4);
    if (ev_mask & VTSS_UPI_TX_HSS_PRBSSYNC_3_EV)    mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_HSS_PRBSSYNC_MASK(0x8);
    if (ev_mask & VTSS_UPI_HSS_RESETOUT_EV)         mask |= VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_HSS_RESETOUT_MASK;
    if (ev_mask & VTSS_UPI_RX_LOS_EV)               mask |= VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_LOS_MASK;
    if (ev_mask & VTSS_UPI_RX_SIGDET_0_EV)          mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_SIGDET_MASK(0x1);
    if (ev_mask & VTSS_UPI_RX_SIGDET_1_EV)          mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_SIGDET_MASK(0x2);
    if (ev_mask & VTSS_UPI_RX_SIGDET_2_EV)          mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_SIGDET_MASK(0x4);
    if (ev_mask & VTSS_UPI_RX_SIGDET_3_EV)          mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_SIGDET_MASK(0x8);
    if (ev_mask & VTSS_UPI_TX_FIFO_OVERFLOW_EV)     mask |= VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_FIFO_OVERFLOW_MASK;
    if (ev_mask & VTSS_UPI_TX_FIFO_UNDERFLOW_EV)    mask |= VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_FIFO_UNDERFLOW_MASK;
    if (ev_mask & VTSS_UPI_HSS_PRTREADY_EV)         mask |= VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_HSS_PRTREADY_MASK;
    if (ev_mask & VTSS_UPI_RX_PRBS_ERR_0_EV)        mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_PRBS_ERR_MASK(0x1);
    if (ev_mask & VTSS_UPI_RX_PRBS_ERR_1_EV)        mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_PRBS_ERR_MASK(0x2);
    if (ev_mask & VTSS_UPI_RX_PRBS_ERR_2_EV)        mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_PRBS_ERR_MASK(0x4);
    if (ev_mask & VTSS_UPI_RX_PRBS_ERR_3_EV)        mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_PRBS_ERR_MASK(0x8);
    if (ev_mask & VTSS_UPI_TX_PRBS_ERR_0_EV)        mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_PRBS_ERR_MASK(0x1);
    if (ev_mask & VTSS_UPI_TX_PRBS_ERR_1_EV)        mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_PRBS_ERR_MASK(0x2);
    if (ev_mask & VTSS_UPI_TX_PRBS_ERR_2_EV)        mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_PRBS_ERR_MASK(0x4);
    if (ev_mask & VTSS_UPI_TX_PRBS_ERR_3_EV)        mask |= VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_PRBS_ERR_MASK(0x8);
    if (ev_mask & VTSS_UPI_SSF_EV)                  mask |= VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_SSF_MASK;
    if (ev_mask & VTSS_UPI_PLLLOCK_EV)              mask |= VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_PLLLOCK_MASK;
    if (ev_mask & VTSS_UPI_EYE_QUALITY_ERR_EV)      mask |= VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_EYE_QUALITY_ERR_MASK;

    DAYTONA_WRM(VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if (ev_mask & VTSS_UPI_OTN_FAS_POS_EV)          mask |= VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_FAS_POS_MASK;
    if (ev_mask & VTSS_UPI_OTN_LOM_EV)              mask |= VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOM_MASK; 
    if (ev_mask & VTSS_UPI_OTN_OOM_EV)              mask |= VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOM_MASK;
    if (ev_mask & VTSS_UPI_OTN_LOF_EV)              mask |= VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOF_MASK;
    if (ev_mask & VTSS_UPI_OTN_OOF_EV)              mask |= VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOF_MASK;

    DAYTONA_WRM(VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}

static vtss_rc daytona_upi_event_poll(vtss_state_t *vtss_state,
                                      const vtss_port_no_t   port_no,
                                      vtss_upi_event_t       *const status)
{
    u32 target;
    u32 sticky, mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));

    DAYTONA_RD(VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY(target), &sticky);
    DAYTONA_RD(VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK(target), &mask);

    sticky &= mask;
    DAYTONA_WR(VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY(target), sticky);

    *status = 0;

    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY(0x1))    *status |= VTSS_UPI_RX_HSS_PRBSSYNC_0_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY(0x2))    *status |= VTSS_UPI_RX_HSS_PRBSSYNC_1_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY(0x4))    *status |= VTSS_UPI_RX_HSS_PRBSSYNC_2_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY(0x8))    *status |= VTSS_UPI_RX_HSS_PRBSSYNC_3_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY(0x1))    *status |= VTSS_UPI_TX_HSS_PRBSSYNC_0_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY(0x2))    *status |= VTSS_UPI_TX_HSS_PRBSSYNC_1_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY(0x4))    *status |= VTSS_UPI_TX_HSS_PRBSSYNC_2_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY(0x8))    *status |= VTSS_UPI_TX_HSS_PRBSSYNC_3_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_HSS_RESETOUT_STICKY)            *status |= VTSS_UPI_HSS_RESETOUT_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_LOS_STICKY)                  *status |= VTSS_UPI_RX_LOS_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY(0x1))          *status |= VTSS_UPI_RX_SIGDET_0_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY(0x2))          *status |= VTSS_UPI_RX_SIGDET_1_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY(0x4))          *status |= VTSS_UPI_RX_SIGDET_2_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY(0x8))          *status |= VTSS_UPI_RX_SIGDET_3_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_FIFO_OVERFLOW_STICKY)        *status |= VTSS_UPI_TX_FIFO_OVERFLOW_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_FIFO_UNDERFLOW_STICKY)       *status |= VTSS_UPI_TX_FIFO_UNDERFLOW_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_HSS_PRTREADY_STICKY)            *status |= VTSS_UPI_HSS_PRTREADY_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY(0x1))        *status |= VTSS_UPI_RX_PRBS_ERR_0_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY(0x2))        *status |= VTSS_UPI_RX_PRBS_ERR_1_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY(0x4))        *status |= VTSS_UPI_RX_PRBS_ERR_2_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY(0x8))        *status |= VTSS_UPI_RX_PRBS_ERR_3_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY(0x1))        *status |= VTSS_UPI_TX_PRBS_ERR_0_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY(0x2))        *status |= VTSS_UPI_TX_PRBS_ERR_1_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY(0x4))        *status |= VTSS_UPI_TX_PRBS_ERR_2_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY(0x8))        *status |= VTSS_UPI_TX_PRBS_ERR_3_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_SSF_STICKY)                     *status |= VTSS_UPI_SSF_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_PLLLOCK_STICKY)                 *status |= VTSS_UPI_PLLLOCK_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_EYE_QUALITY_ERR_STICKY)         *status |= VTSS_UPI_EYE_QUALITY_ERR_EV;

    DAYTONA_RD(VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY(target), &sticky);
    DAYTONA_RD(VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK(target), &mask);

    sticky &= mask;
    DAYTONA_WR(VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY(target), sticky);

    if (sticky & VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_FAS_POS_STICKY)     *status |= VTSS_UPI_OTN_FAS_POS_EV;
    if (sticky & VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOM_STICKY)         *status |= VTSS_UPI_OTN_LOM_EV;
    if (sticky & VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOM_STICKY)         *status |= VTSS_UPI_OTN_OOM_EV;
    if (sticky & VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOF_STICKY)         *status |= VTSS_UPI_OTN_LOF_EV;
    if (sticky & VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOF_STICKY)         *status |= VTSS_UPI_OTN_OOF_EV;

    return VTSS_RC_OK;
}


static vtss_rc daytona_upi_event_poll_without_mask(vtss_state_t         *vtss_state,
                                                   const vtss_port_no_t port_no,
                                                   vtss_upi_event_t     *const status)
{
    u32 target;
    u32 sticky;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target));

    DAYTONA_RD(VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY(target), &sticky);
    DAYTONA_WR(VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY(target), sticky);

    *status = 0;

    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY(0x1))    *status |= VTSS_UPI_RX_HSS_PRBSSYNC_0_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY(0x2))    *status |= VTSS_UPI_RX_HSS_PRBSSYNC_1_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY(0x4))    *status |= VTSS_UPI_RX_HSS_PRBSSYNC_2_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY(0x8))    *status |= VTSS_UPI_RX_HSS_PRBSSYNC_3_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY(0x1))    *status |= VTSS_UPI_TX_HSS_PRBSSYNC_0_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY(0x2))    *status |= VTSS_UPI_TX_HSS_PRBSSYNC_1_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY(0x4))    *status |= VTSS_UPI_TX_HSS_PRBSSYNC_2_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY(0x8))    *status |= VTSS_UPI_TX_HSS_PRBSSYNC_3_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_HSS_RESETOUT_STICKY)            *status |= VTSS_UPI_HSS_RESETOUT_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_LOS_STICKY)                  *status |= VTSS_UPI_RX_LOS_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY(0x1))          *status |= VTSS_UPI_RX_SIGDET_0_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY(0x2))          *status |= VTSS_UPI_RX_SIGDET_1_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY(0x4))          *status |= VTSS_UPI_RX_SIGDET_2_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY(0x8))          *status |= VTSS_UPI_RX_SIGDET_3_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_FIFO_OVERFLOW_STICKY)        *status |= VTSS_UPI_TX_FIFO_OVERFLOW_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_FIFO_UNDERFLOW_STICKY)       *status |= VTSS_UPI_TX_FIFO_UNDERFLOW_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_HSS_PRTREADY_STICKY)            *status |= VTSS_UPI_HSS_PRTREADY_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY(0x1))        *status |= VTSS_UPI_RX_PRBS_ERR_0_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY(0x2))        *status |= VTSS_UPI_RX_PRBS_ERR_1_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY(0x4))        *status |= VTSS_UPI_RX_PRBS_ERR_2_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY(0x8))        *status |= VTSS_UPI_RX_PRBS_ERR_3_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY(0x1))        *status |= VTSS_UPI_TX_PRBS_ERR_0_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY(0x2))        *status |= VTSS_UPI_TX_PRBS_ERR_1_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY(0x4))        *status |= VTSS_UPI_TX_PRBS_ERR_2_EV;
    if (sticky & VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY(0x8))        *status |= VTSS_UPI_TX_PRBS_ERR_3_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_SSF_STICKY)                     *status |= VTSS_UPI_SSF_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_PLLLOCK_STICKY)                 *status |= VTSS_UPI_PLLLOCK_EV;
    if (sticky & VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_EYE_QUALITY_ERR_STICKY)         *status |= VTSS_UPI_EYE_QUALITY_ERR_EV;

    DAYTONA_RD(VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY(target), &sticky);
    DAYTONA_WR(VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY(target), sticky);

    if (sticky & VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_FAS_POS_STICKY)     *status |= VTSS_UPI_OTN_FAS_POS_EV;
    if (sticky & VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOM_STICKY)         *status |= VTSS_UPI_OTN_LOM_EV;
    if (sticky & VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOM_STICKY)         *status |= VTSS_UPI_OTN_OOM_EV;
    if (sticky & VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOF_STICKY)         *status |= VTSS_UPI_OTN_LOF_EV;
    if (sticky & VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOF_STICKY)         *status |= VTSS_UPI_OTN_OOF_EV;

    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Utilities and internal
 * ================================================================= */

/**
 * \brief Static Initialization of the configuration data base in the vtss_state.
 **/
static vtss_upi_cfg_t cfg_default = {
    .tx_to_rx_loopback = FALSE,
    .rx_to_tx_loopback = FALSE,
};
static vtss_upi_test_cfg_t test_cfg_default = {
    .test_cfg_gen = VTSS_UPI_TEST_MODE_DISABLE,
    .test_cfg_chk = VTSS_UPI_TEST_MODE_DISABLE,
    .wrap_mode = VTSS_UPI_TEST_WRAP_RX,
};
static vtss_upi_tx_cfg_t upi_txeq_cfg_default = {
    .txeq_cfg       = {.mode = UPI_TXEQ_OVERRIDE, .channel_loss = upi_chloss_lt10inch},
    .transmit_power = 0x73,
    .slew_rate      = 0,
    .coefs          = {[upi_txeq_coef_0] = 0, [upi_txeq_coef_1] = 255,
                       [upi_txeq_coef_2] = 0, [upi_txeq_coef_3] = 0},
    .polarity       = {[upi_txeq_coef_0] = 1, [upi_txeq_coef_1] = 1,
                       [upi_txeq_coef_2] = 1, [upi_txeq_coef_3] = 1},
} ;
static vtss_upi_rxeq_cfg_t upi_rxeq_cfg_def = {
    .channel_loss  = upi_chloss_lt10inch,
};

/*
 * \brief Restart UPI Configuration on a port
 *
 * \param port_no [IN] Port number
 *
 * \return Return code
 */   
vtss_rc vtss_daytona_upi_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_rc rc;

    rc = daytona_upi_config_set(vtss_state, port_no);
    rc = daytona_upi_test_config_set(vtss_state, port_no);
    VTSS_D("port_no %d, restart conf set, rc = %x", port_no, rc);
    return VTSS_RC_OK;
}

/*
 * \brief Set Default UPI configuration for a port
 *
 * \param port_no [IN] Port number
 *
 * \return Return code
 */
vtss_rc vtss_daytona_upi_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_state->upi_state[port_no].upi_cfg = cfg_default;
    vtss_state->upi_state[port_no].upi_test_cfg = test_cfg_default;
    vtss_state->upi_state[port_no].upi_tx_cfg = upi_txeq_cfg_default;
    vtss_state->upi_state[port_no].upi_rxeq_cfg = upi_rxeq_cfg_def;

    return VTSS_RC_OK;
}
/**
 * \brief Create instance (set up function pointers).
 *
 * \return Return code.
 **/
vtss_rc vtss_daytona_inst_upi_create(vtss_state_t *vtss_state)
{
    vtss_cil_func_t *func = &vtss_state->cil;

    vtss_port_no_t port_no;
    for (port_no = 0 ; port_no < VTSS_PORT_ARRAY_SIZE; port_no++) {
        vtss_state->upi_state[port_no].upi_cfg = cfg_default;
        vtss_state->upi_state[port_no].upi_test_cfg = test_cfg_default;
        vtss_state->upi_state[port_no].upi_tx_cfg = upi_txeq_cfg_default;
    }

    /* Daytona UPI functions */
    func->upi_config_set                = daytona_upi_config_set;
    func->upi_status_get                = daytona_upi_status_get;
    func->upi_test_config_set           = daytona_upi_test_config_set;
    func->upi_test_status_get           = daytona_upi_test_status_get;
    func->upi_event_poll                = daytona_upi_event_poll;
    func->upi_event_poll_without_mask   = daytona_upi_event_poll_without_mask;
    func->upi_event_enable              = daytona_upi_event_enable;
    /*Daytona UPI FFE/DFE functions */
    func->upi_txeq_mode_set				= daytona_upi_txeq_mode_set;
    func->upi_txeq_coef_adjust			= daytona_upi_txeq_coef_adjust;
    func->upi_txamp_set					= daytona_upi_txamp_set;
    func->upi_txslew_set				= daytona_upi_txslew_set;
    func->upi_rxeq_mode_set				= daytona_upi_rxeq_mode_set;

    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_DAYTONA */



