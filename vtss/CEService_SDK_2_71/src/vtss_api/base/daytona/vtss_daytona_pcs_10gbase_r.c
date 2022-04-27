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

// Avoid "vtss_api.h not used in module vtss_daytona_pcs_10gbase_r.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_pcs_10gbase_r_api.h"
#include "vtss_daytona_pcs_10gbase_r.h"
#include "vtss_daytona_regs_pcs_10gbase_r.h"

/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
static vtss_rc daytona_pcs_10gbase_r_event_enable(vtss_state_t *vtss_state,
                                                  const vtss_port_no_t               port_no,
                                                  const BOOL                         enable,
                                                  const vtss_pcs_10gbase_r_event_t   ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_10GBASE_R, &target));

    mask = 0;

    if (ev_mask & VTSS_PCS_10GBASE_R_RX_HI_BER_EV)             mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_HI_BER_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_RX_FIFO_OFLOW_EV)         mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_OFLOW_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_RX_FIFO_UFLOW_EV)         mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_UFLOW_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_LOCK_CHANGED_EV)          mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_LOCK_CHANGED_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_C64B66B_ERR_EV)           mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_C64B66B_ERR_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_RX_OSET_EV)               mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_OSET_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_RX_OSET_FIFO_FULL_EV)     mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_OSET_FIFO_FULL_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_TX_FIFO_OFLOW_EV)         mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_OFLOW_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_TX_FIFO_UFLOW_EV)         mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_UFLOW_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_XGMII_ERR_EV)             mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_XGMII_ERR_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_RX_FSET_EV)               mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FSET_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_RX_FSET_FIFO_FULL_EV)     mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FSET_FIFO_FULL_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_TX_FIFO_ECC_1BIT_EV)      mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_ECC_1BIT_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_TX_FIFO_ECC_2BIT_EV)      mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_ECC_2BIT_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_RX_FIFO_ECC_1BIT_EV)      mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_ECC_1BIT_MASK;
    if (ev_mask & VTSS_PCS_10GBASE_R_RX_FIFO_ECC_2BIT_EV)      mask |= VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_ECC_2BIT_MASK;

    DAYTONA_WRM(VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}


static vtss_rc daytona_pcs_10gbase_r_event_poll(vtss_state_t *vtss_state,
                                                const vtss_port_no_t         port_no,
                                                vtss_pcs_10gbase_r_event_t   *const status)
{
    u32 target;
    u32 sticky, mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_10GBASE_R, &target));

    DAYTONA_RD(VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT(target), &sticky);
    DAYTONA_RD(VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK(target), &mask);
    sticky &= mask;
    DAYTONA_WR(VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT(target), sticky);

    *status = 0;

    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_HI_BER_STICKY)           *status |= VTSS_PCS_10GBASE_R_RX_HI_BER_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_OFLOW_STICKY)       *status |= VTSS_PCS_10GBASE_R_RX_FIFO_OFLOW_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_UFLOW_STICKY)       *status |= VTSS_PCS_10GBASE_R_RX_FIFO_UFLOW_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_LOCK_CHANGED_STICKY)        *status |= VTSS_PCS_10GBASE_R_LOCK_CHANGED_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_C64B66B_ERR_STICKY)         *status |= VTSS_PCS_10GBASE_R_C64B66B_ERR_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_OSET_STICKY)             *status |= VTSS_PCS_10GBASE_R_RX_OSET_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_OSET_FIFO_FULL_STICKY)   *status |= VTSS_PCS_10GBASE_R_RX_OSET_FIFO_FULL_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_OFLOW_STICKY)       *status |= VTSS_PCS_10GBASE_R_TX_FIFO_OFLOW_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_UFLOW_STICKY)       *status |= VTSS_PCS_10GBASE_R_TX_FIFO_UFLOW_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_XGMII_ERR_STICKY)           *status |= VTSS_PCS_10GBASE_R_XGMII_ERR_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FSET_STICKY)             *status |= VTSS_PCS_10GBASE_R_RX_FSET_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FSET_FIFO_FULL_STICKY)   *status |= VTSS_PCS_10GBASE_R_RX_FSET_FIFO_FULL_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_ECC_1BIT_STICKY)    *status |= VTSS_PCS_10GBASE_R_TX_FIFO_ECC_1BIT_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_ECC_2BIT_STICKY)    *status |= VTSS_PCS_10GBASE_R_TX_FIFO_ECC_2BIT_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_ECC_1BIT_STICKY)    *status |= VTSS_PCS_10GBASE_R_RX_FIFO_ECC_1BIT_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_ECC_2BIT_STICKY)    *status |= VTSS_PCS_10GBASE_R_RX_FIFO_ECC_2BIT_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_pcs_10gbase_r_event_poll_without_mask(vtss_state_t *vtss_state,
                                                             const vtss_port_no_t         port_no,
                                                             vtss_pcs_10gbase_r_event_t   *const status)
{
    u32 target;
    u32 sticky;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_10GBASE_R, &target));

    DAYTONA_RD(VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT(target), &sticky);
    DAYTONA_WR(VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT(target), sticky);

    *status = 0;

    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_HI_BER_STICKY)           *status |= VTSS_PCS_10GBASE_R_RX_HI_BER_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_OFLOW_STICKY)       *status |= VTSS_PCS_10GBASE_R_RX_FIFO_OFLOW_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_UFLOW_STICKY)       *status |= VTSS_PCS_10GBASE_R_RX_FIFO_UFLOW_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_LOCK_CHANGED_STICKY)        *status |= VTSS_PCS_10GBASE_R_LOCK_CHANGED_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_C64B66B_ERR_STICKY)         *status |= VTSS_PCS_10GBASE_R_C64B66B_ERR_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_OSET_STICKY)             *status |= VTSS_PCS_10GBASE_R_RX_OSET_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_OSET_FIFO_FULL_STICKY)   *status |= VTSS_PCS_10GBASE_R_RX_OSET_FIFO_FULL_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_OFLOW_STICKY)       *status |= VTSS_PCS_10GBASE_R_TX_FIFO_OFLOW_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_UFLOW_STICKY)       *status |= VTSS_PCS_10GBASE_R_TX_FIFO_UFLOW_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_XGMII_ERR_STICKY)           *status |= VTSS_PCS_10GBASE_R_XGMII_ERR_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FSET_STICKY)             *status |= VTSS_PCS_10GBASE_R_RX_FSET_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FSET_FIFO_FULL_STICKY)   *status |= VTSS_PCS_10GBASE_R_RX_FSET_FIFO_FULL_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_ECC_1BIT_STICKY)    *status |= VTSS_PCS_10GBASE_R_TX_FIFO_ECC_1BIT_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_ECC_2BIT_STICKY)    *status |= VTSS_PCS_10GBASE_R_TX_FIFO_ECC_2BIT_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_ECC_1BIT_STICKY)    *status |= VTSS_PCS_10GBASE_R_RX_FIFO_ECC_1BIT_EV;
    if (sticky & VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_ECC_2BIT_STICKY)    *status |= VTSS_PCS_10GBASE_R_RX_FIFO_ECC_2BIT_EV;

    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */


static vtss_rc daytona_pcs_10gbase_r_config_set(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no)
{
    u32 target;
    BOOL loopback;  /**< In loopback mode the PCS accepts data from XGMII and returns it to the XGMII */
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    u32 value, mask;
#endif

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_10GBASE_R, &target));

    /* TBD:: Test mode and seed is implemented later. */
    loopback = vtss_state->pcs_10gbase_r_state[port_no].pcs_10gbase_r_cfg.loopback;
    DAYTONA_WRM(VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG(target),
                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_LOOPBACK_ENA(loopback),
                VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_LOOPBACK_ENA);

#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
    if (vtss_state->phy_ts_port_conf[port_no].port_ts_init_done == TRUE ) {
        /* Enable the 1588 datapath */
        value = VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_RX_XGMII(TRUE) |
                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_TX_XGMII(TRUE);
    } else {
        value = VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_RX_XGMII(FALSE) |
                VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_TX_XGMII(FALSE);
    }
    mask = VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_RX_XGMII |
           VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_TX_XGMII;
    DAYTONA_WRM(VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG(target), value, mask);
#endif

   return VTSS_RC_OK;
}

static vtss_rc daytona_pcs_10gbase_r_status_get(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no,
                                                vtss_pcs_10gbase_r_status_t *const status)
{
    u32 target;
    u32 value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_10GBASE_R, &target));

    value = 0;
    DAYTONA_RD(VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS(target) ,&value);
    status->PCS_status    = VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_RX_BLOCK_LOCK(value) &&
                           (!VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_RX_HI_BER(value));
    status->block_lock    = VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_RX_BLOCK_LOCK(value);
    status->hi_ber        = VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_RX_HI_BER(value);
    status->testpat_match = VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_TESTPAT_MATCH(value);
 
    return VTSS_RC_OK; 
}

#if 0
static vtss_rc daytona_pcs_10gbase_r_counters_get(const vtss_port_no_t port_no,
                                                  vtss_pcs_10gbase_r_cnt_t *const counters)
{
    u32 target;
    u32 value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_10GBASE_R, &target));
    value = 0;
    DAYTONA_RD(VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_BER_CNT(target) ,&value);
    counters->ber_count = value;
    value = 0;
    DAYTONA_RD(VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ERRBLK_CNT(target) ,&value);
    counters->rx_errored_block_count = value;
    value = 0;
    DAYTONA_RD(VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ERRBLK_CNT(target) ,&value);
    counters->tx_errored_block_count = value;
    value = 0;
    DAYTONA_RD(VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TEST_ERR_CNT(target) ,&value);
    counters->test_pattern_error_count = value;

    return VTSS_RC_OK;
}
#endif /* 0 */

static void daytona_cnt_update(u32 value,
                               vtss_chip_counter_t *counter,
                               BOOL clr)
{
    if (clr) {
        /* Clear counter */
        counter->value = 0;
    } else {
        /* Accumulate counter */
        if (value >= counter->prev) {
            /* Not wrapped */
            counter->value += (value - counter->prev);
        } else {
            /* Wrapped */
            counter->value += (0xffffffff - counter->prev);
            counter->value += (value + 1);
        }
        counter->prev = value;
    }
}

/* Handle 10G counter in DEV10G */
#define DAYTONA_CNT_PCS_10GBASE_R(reg, cnt, clr)         \
{                                                 \
        u32 value;                                \
        DAYTONA_RD(reg, &value);                  \
        daytona_cnt_update(value, cnt, clr);      \
}

static vtss_rc daytona_pcs_10gbase_r_counters(vtss_state_t *vtss_state,
                                              const vtss_port_no_t port_no,
                                              vtss_pcs_10gbase_r_cnt_t *const counters,
                                              BOOL clr)
{
    u32  target;
    vtss_pcs_10gbase_r_chip_counters_t *c = &vtss_state->pcs_10gbase_r_cntrs[port_no];

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_10GBASE_R, &target));

    DAYTONA_CNT_PCS_10GBASE_R(VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_BER_CNT(target), &c->ber_count, clr);
    DAYTONA_CNT_PCS_10GBASE_R(VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ERRBLK_CNT(target), &c->rx_errored_block_count, clr);
    DAYTONA_CNT_PCS_10GBASE_R(VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ERRBLK_CNT(target), &c->tx_errored_block_count, clr);
    DAYTONA_CNT_PCS_10GBASE_R(VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TEST_ERR_CNT(target), &c->test_pattern_error_count, clr);
    if (counters == NULL) {
        return VTSS_RC_OK;
    }

    counters->ber_count                = c->ber_count.value;
    counters->rx_errored_block_count   = c->rx_errored_block_count.value;
    counters->tx_errored_block_count   = c->tx_errored_block_count.value;
    counters->test_pattern_error_count = c->test_pattern_error_count.value;
    return VTSS_RC_OK;
}

static vtss_rc daytona_pcs_10gbase_r_counters_update(vtss_state_t *vtss_state,
                                                     const vtss_port_no_t port_no)
{
    return daytona_pcs_10gbase_r_counters(vtss_state, port_no, NULL, 0);
}

static vtss_rc daytona_pcs_10gbase_r_counters_clear(vtss_state_t *vtss_state,
                                                    const vtss_port_no_t port_no)
{
    return daytona_pcs_10gbase_r_counters(vtss_state, port_no, NULL, 1);
}

static vtss_rc daytona_pcs_10gbase_r_counters_get(vtss_state_t *vtss_state,
                                                  const vtss_port_no_t port_no,
                                                  vtss_pcs_10gbase_r_cnt_t *const counters)
{
    return daytona_pcs_10gbase_r_counters(vtss_state, port_no, counters, 0);
}

/* ================================================================= *
 *  Utilities and internal
 * ================================================================= */
/*
 * \brief Restart 10gbase_r configuration on a port
 *
 * \return Return code
 */
vtss_rc vtss_daytona_10gbase_r_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_rc rc;

    rc = daytona_pcs_10gbase_r_counters_clear(vtss_state, port_no);
    rc = daytona_pcs_10gbase_r_config_set(vtss_state, port_no);
    VTSS_D("port_no %d, 10gbase_r restart conf set, rc = %x", port_no, rc);
    return VTSS_RC_OK;
}

static vtss_pcs_10gbase_r_cfg_t pcs_10gbase_r_default = {
    .tx_test_mode = FALSE,
    .rx_test_mode = FALSE,
    .loopback     = FALSE,
};

/*
 * \brief Set 10gbase_r Default configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_daytona_pcs_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_state->pcs_10gbase_r_state[port_no].pcs_10gbase_r_cfg = pcs_10gbase_r_default;

    return VTSS_RC_OK;
}

/**
 * \brief Create instance (set up function pointers).
 *
 * \return Return code.
 **/
vtss_rc vtss_daytona_inst_pcs_10gbase_r_create(vtss_state_t *vtss_state)
{
    vtss_port_no_t port_no;
    vtss_cil_func_t *func = &vtss_state->cil;

    for (port_no = 0 ; port_no < VTSS_PORT_ARRAY_SIZE; port_no++) {
        vtss_state->pcs_10gbase_r_state[port_no].pcs_10gbase_r_cfg = pcs_10gbase_r_default;
    }
    /* Daytona PCS 10GBaseR functions */        
    func->pcs_10gbase_r_config_set                  = daytona_pcs_10gbase_r_config_set;
    func->pcs_10gbase_r_status_get                  = daytona_pcs_10gbase_r_status_get;
    func->pcs_10gbase_r_counters_update             = daytona_pcs_10gbase_r_counters_update;
    func->pcs_10gbase_r_counters_clear              = daytona_pcs_10gbase_r_counters_clear;
    func->pcs_10gbase_r_counters_get                = daytona_pcs_10gbase_r_counters_get;
    func->pcs_10gbase_r_event_poll                  = daytona_pcs_10gbase_r_event_poll;
    func->pcs_10gbase_r_event_poll_without_mask     = daytona_pcs_10gbase_r_event_poll_without_mask;
    func->pcs_10gbase_r_event_enable                = daytona_pcs_10gbase_r_event_enable;
        
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_DAYTONA */
