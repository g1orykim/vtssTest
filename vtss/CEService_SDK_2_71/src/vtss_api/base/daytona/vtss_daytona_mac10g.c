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

// Avoid "vtss_api.h not used in module vtss_daytona_mac10g.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_mac10g_api.h"
#include "vtss_daytona_mac10g.h"
#include "vtss_daytona_regs_mac10g.h"
#include "vtss_daytona_regs_used_debug.h"


#define VTSS_DAYTONA_MAC10G_DEFAULT_MAX_LEN  10240
#define VTSS_DAYTONA_MAC10G_DEFAULT_MAX_TAGS VTSS_PORT_MAX_TAGS_TWO

#define VTSS_DAYTONA_GCB_CFG_MAC2_TX_CLK_MUX     3  /* This is the order used in AN1029 for mac2_tx_clk_mux*/   
/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

#define VTSS_DAYTONA_MAC10G_TX_PAUSE_MAX_INT   (u32)0xff /* Use 255 for TX Pause VALUE */
#define VTSS_DAYTONA_MAC10G_TX_PAUSE_ENA       (u32)0x02 /* Enable XON Mode; XON is enabled when Tx flow control is enabled */
#define VTSS_DAYTONA_MAC10G_TX_PAUSE_DIS       (u32)0x00 

#define VTSS_DAYTONA_MAC10G_RX_PAUSE_DROP      (u32)0x0
#define VTSS_DAYTONA_MAC10G_RX_PAUSE_FWD       (u32)0x1
#define VTSS_DAYTONA_MAC10G_RX_PAUSE_DIS       (u32)0x0
#define VTSS_DAYTONA_MAC10G_RX_PAUSE_ENA       (u32)0x1

#define VTSS_DAYTONA_MAC10G_MAX_RX_LEN_CFG     (u32)10240
#define VTSS_DAYTONA_MAC10G_MAX_RX_TAG_CHK_DIS (u32)0x0
#define VTSS_DAYTONA_MAC10G_MAX_RX_TAG_CHK_ENA (u32)0x1

#define VTSS_DAYTONA_MAC10G_VLAN_TPID_SERVICE   (u16)0x8100 /* Note :: This canbe Changed in the Future */
#define VTSS_DAYTONA_MAC10G_VLAN_TAG_AWARE      (u16)1 
#define VTSS_DAYTONA_MAC10G_VLAN_TAG_UNAWARE    (u16)0 

#define VTSS_DAYTONA_MAC10G_VLAN_DOUBLE_TAG_ENA (u16)1
#define VTSS_DAYTONA_MAC10G_VLAN_DOUBLE_TAG_DIS (u16)0

#define VTSS_DAYTONA_MAC10G_LOOPBACK_ENA        (u16)1
#define VTSS_DAYTONA_MAC10G_LOOPBACK_DIS        (u16)0

#define VTSS_DAYTONA_MAC10G_LOCAL_FAULT_FORCE_ENA    (u16)1
#define VTSS_DAYTONA_MAC10G_LOCAL_FAULT_FORCE_DIS    (u16)0

#define VTSS_DAYTONA_MAC10G_REMOTE_FAULT_FORCE_ENA   (u16)1
#define VTSS_DAYTONA_MAC10G_REMOTE_FAULT_FORCE_DIS   (u16)0

#define VTSS_DAYTONA_MAC10G_PREAMBLE_CHECK_ENA       (u16)1
#define VTSS_DAYTONA_MAC10G_PREAMBLE_CHECK_DIS       (u16)0


static vtss_rc daytona_mac10g_config_set(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no)
{
    u32 target;
    u32 value;
    u32 value_mac_clk = 0;
    vtss_mac_t mac_null = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
    daytona_channel_t channel = 0;
    daytona_side_t side = 0;
    u32 mode,mask;
    const static_cfg_t **gcb_conf_table;
    u8 blk_mode;

    if (vtss_state->mac10g_state[port_no].mac10g_cfg.max_frame_length &&
       (vtss_state->mac10g_state[port_no].mac10g_cfg.max_frame_length > VTSS_DAYTONA_MAC10G_MAX_RX_LEN_CFG)) {
        return VTSS_RC_ERROR;
    }

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_MAC10G, &target));
    VTSS_RC(daytona_port_2_channel_side(vtss_state, port_no, &channel, &side));

    if (vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.generate != FALSE) { /* Pause Frame Generation is enabled */
        /* VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_2 is left with the default value 0xa*/
        value = 0;
        value = VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_VALUE(VTSS_DAYTONA_MAC10G_TX_PAUSE_MAX_INT) |
                VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_MODE(VTSS_DAYTONA_MAC10G_TX_PAUSE_ENA);
        DAYTONA_WR(VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL(target), value);
    } else { /* Pause Frame Generation is Disabled */
        value = 0;
        value = VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_MODE(VTSS_DAYTONA_MAC10G_TX_PAUSE_DIS);
        DAYTONA_WR(VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL(target), value);
    }

    value = 0;
    if (vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.rx_drop != FALSE) { /* Pause Frame are dropped */
        value |= VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_FRAME_DROP_DIS(VTSS_DAYTONA_MAC10G_RX_PAUSE_DROP);
    } else { /* Pause Frames are farwarded */
        value |= VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_FRAME_DROP_DIS(VTSS_DAYTONA_MAC10G_RX_PAUSE_FWD);
    }

    if (vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.obey != FALSE) { /* Pause Frame obey is enabled */
        value |= VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_MODE(VTSS_DAYTONA_MAC10G_RX_PAUSE_ENA);;
    } else {
        value |= VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_MODE(VTSS_DAYTONA_MAC10G_RX_PAUSE_DIS);
    }
    DAYTONA_WR(VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL(target), value);

    /* If the smac is {00-00-00-00-00-00}; don't configure the value in the register */
    if (memcmp(vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.smac.addr, &mac_null,
                                               sizeof(vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.smac)) != 0) {
        value = 0;
        value = vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.smac.addr[3] << 24 |
                vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.smac.addr[2] << 16 |
                vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.smac.addr[1] << 8  |
                vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.smac.addr[0];
        DAYTONA_WR(VTSS_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_LSB(target), value);

        value = 0;
        value = vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.smac.addr[5] << 8 |
                vtss_state->mac10g_state[port_no].mac10g_cfg.flow_control.smac.addr[4];
        DAYTONA_WR(VTSS_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_MSB(target), value);
    }

    if (vtss_state->mac10g_state[port_no].mac10g_cfg.max_frame_length) {
        value = 0;
        /* VLAN tags are enabled so include the extra bytes (4 or 8) while calculating max frame length */
        if (vtss_state->mac10g_state[port_no].mac10g_cfg.max_tags == VTSS_PORT_MAX_TAGS_NONE) {
            value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN_TAG_CHK(VTSS_DAYTONA_MAC10G_MAX_RX_TAG_CHK_DIS);
        } else {
            value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN_TAG_CHK(VTSS_DAYTONA_MAC10G_MAX_RX_TAG_CHK_ENA);
        }
        value |= VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN(vtss_state->mac10g_state[port_no].mac10g_cfg.max_frame_length);
        DAYTONA_WR(VTSS_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG(target), value);
    }

    value = 0;
    if (vtss_state->mac10g_state[port_no].mac10g_cfg.max_tags == VTSS_PORT_MAX_TAGS_NONE) {
        /* VLAN_AWARE_ENA : 0; DOUBLE_TAG_ENA : 0; TAG_ID : 0x8100 */
        value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID(VTSS_DAYTONA_MAC10G_VLAN_TPID_SERVICE) |
                VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_DOUBLE_TAG_ENA(VTSS_DAYTONA_MAC10G_VLAN_DOUBLE_TAG_DIS) |
                VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_AWR_ENA(VTSS_DAYTONA_MAC10G_VLAN_TAG_UNAWARE);
    } else if (vtss_state->mac10g_state[port_no].mac10g_cfg.max_tags == VTSS_PORT_MAX_TAGS_ONE) {
        /* VLAN_AWARE_ENA : 1; DOUBLE_TAG_ENA : 0; TAG_ID : 0x8100 */
        value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID(VTSS_DAYTONA_MAC10G_VLAN_TPID_SERVICE) |
                VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_DOUBLE_TAG_ENA(VTSS_DAYTONA_MAC10G_VLAN_DOUBLE_TAG_DIS) |
                VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_AWR_ENA(VTSS_DAYTONA_MAC10G_VLAN_TAG_AWARE);
    } else if (vtss_state->mac10g_state[port_no].mac10g_cfg.max_tags == VTSS_PORT_MAX_TAGS_TWO) {
        /* VLAN_AWARE_ENA : 1; DOUBLE_TAG_ENA : 1; TAG_ID : 0x8100 */
        value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID(VTSS_DAYTONA_MAC10G_VLAN_TPID_SERVICE) |
                VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_DOUBLE_TAG_ENA(VTSS_DAYTONA_MAC10G_VLAN_DOUBLE_TAG_ENA) |
                VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_AWR_ENA(VTSS_DAYTONA_MAC10G_VLAN_TAG_AWARE);
    }
    DAYTONA_WR(VTSS_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG(target), value);

    /* Loopback Configurations */
    value = 0;
    if (vtss_state->mac10g_state[port_no].mac10g_cfg.tx_rx_loopback != FALSE) { /*Tx to Rx Loopback Enabled */
        value |= VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_TX_RX_LB_ENA(VTSS_DAYTONA_MAC10G_LOOPBACK_ENA);
    } else { /* Tx to Rx loopback Disabled */
        value |= VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_TX_RX_LB_ENA(VTSS_DAYTONA_MAC10G_LOOPBACK_DIS);
    }
    if (vtss_state->mac10g_state[port_no].mac10g_cfg.rx_tx_loopback != FALSE) { /*Tx to Rx Loopback Enabled */
        /* Set the Clock Register MAC2_RX_CLK_MUX_0/MAC1_RX_CLK_MUX_0 to Line Tx/Rx gapped clock is source
         * 1. This setting is required for Line Side ports only.
         * 2. We need to know the channel number.
         */
        /* This work-around is required only for line side.
         * Each channel has it's own MAC Clock configuration register.
         */
        if (channel == 0 && side == 1)  {
            value_mac_clk = VTSS_F_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0_MAC2_RX_CLK_MUX_0(3) |
                            VTSS_F_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0_MAC2_TX_CLK_MUX_0(3);
            DAYTONA_WR(VTSS_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0, value_mac_clk);
        } else if (channel == 1 && side == 1) {
            value_mac_clk = VTSS_F_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1_MAC2_RX_CLK_MUX_1(3) |
                            VTSS_F_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1_MAC2_TX_CLK_MUX_1(3);
            DAYTONA_WR(VTSS_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1, value_mac_clk);
        }

        value |= VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_RX_TX_LB_ENA(VTSS_DAYTONA_MAC10G_LOOPBACK_ENA);
    } else { /* Rx to Tx loopback Disabled */
        /* Clear the Clock Register MAC2_RX_CLK_MUX_0/MAC1_RX_CLK_MUX_0 to Line Tx/Rx gapped clock is source.
         * 1. This setting is required for Line Side ports only.
         * 2. We need to know the channel number.
         */
        /* This work-around is required only for line side.
         * Each channel has it's own MAC Clock configuration register.
         */
        if (side == 1)  {
          VTSS_RC(daytona_port_2_mode(vtss_state, port_no, &mode));
          if (mode != VTSS_CONFIG_MODE_DISABLED) {
            VTSS_RC(vtss_daytona_blk_oper_mode_get(vtss_state, port_no, channel,BLOCK_GCB, &blk_mode));
            gcb_conf_table = vtss_daytona_gcb_config_get(channel);
            value_mac_clk = ((gcb_conf_table[blk_mode])[VTSS_DAYTONA_GCB_CFG_MAC2_TX_CLK_MUX]).value;
            mask = ((gcb_conf_table[blk_mode])[VTSS_DAYTONA_GCB_CFG_MAC2_TX_CLK_MUX]).mask;
            if(channel == 0) {
              DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0, value_mac_clk, mask);
            }
            else {
              DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1, value_mac_clk, mask);
            }
          }
        } 
        value |= VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_RX_TX_LB_ENA(VTSS_DAYTONA_MAC10G_LOOPBACK_DIS);
    }
    DAYTONA_WR(VTSS_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG(target), value);

    /* Local Fault Force Configurations */
    value = 0;
    if (vtss_state->mac10g_state[port_no].mac10g_cfg.local_fault_force != FALSE) {
        value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LOCAL_FAULT_FORCE_ENA(VTSS_DAYTONA_MAC10G_LOCAL_FAULT_FORCE_ENA);
    } else {
        value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LOCAL_FAULT_FORCE_ENA(VTSS_DAYTONA_MAC10G_LOCAL_FAULT_FORCE_DIS);
    }
    DAYTONA_WRM(VTSS_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG(target), value, VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LOCAL_FAULT_FORCE_ENA);


    /* Remote Fault Force Configurations */
    value = 0;
    if (vtss_state->mac10g_state[port_no].mac10g_cfg.remote_fault_force != FALSE) {
        value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_REMOTE_FAULT_FORCE_ENA(VTSS_DAYTONA_MAC10G_REMOTE_FAULT_FORCE_ENA);
    } else {
        value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_REMOTE_FAULT_FORCE_ENA(VTSS_DAYTONA_MAC10G_REMOTE_FAULT_FORCE_DIS);
    }
    DAYTONA_WRM(VTSS_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG(target), value, VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_REMOTE_FAULT_FORCE_ENA);

    /* Preamble Check Configurations */
    value = 0;
    if (vtss_state->mac10g_state[port_no].mac10g_cfg.preamble_check != FALSE) {
        value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_CHK_ENA(VTSS_DAYTONA_MAC10G_PREAMBLE_CHECK_ENA);
    } else {
        value = VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_CHK_ENA(VTSS_DAYTONA_MAC10G_PREAMBLE_CHECK_ENA);
    }
    DAYTONA_WRM(VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG(target), value, VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_CHK_ENA);

   return VTSS_RC_OK;
}

static vtss_rc daytona_mac10g_status_get(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no,
                                         vtss_mac10g_status_t *const status)
{
    u32 target;
    u32 value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_MAC10G, &target));
    /* Extract the Link State */
    value = 0;
    DAYTONA_RD(VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL(target), &value);
    status->link_state = VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_LINK_STATE(value);

    /* Extract the Pause State */
    value = 0;
    DAYTONA_RD(VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_STATE(target), &value);
    status->pause = VTSS_X_MAC10G_DEV_CFG_STATUS_PAUSE_STATE_PAUSE_STATE(value);

    /* Extract the Local Fault & Remote Fault Status */
    value = 0;
    DAYTONA_RD(VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR(target), &value);
    status->local_fault = VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_LFS_LOCAL_FAULT(value);
    status->remote_fault = VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_LFS_REMOTE_FAULT(value);

   return VTSS_RC_OK;
}

/* Clear or increment chip counter */
static void daytona_cnt_update(u64 value,
                               vtss_chip_counter_t *counter,
                               BOOL clr, BOOL large)
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
            if (large)
              counter->value += (0xffffffffff - counter->prev);
            else
              counter->value += (0xffffffff - counter->prev);
            counter->value += (value + 1);
        }
    }
    counter->prev = value;
}

/* Handle 10G counter in DEV10G */
#define DAYTONA_CNT_MAC10G(reg, cnt, clr)     \
{                                             \
    u32 value;                                \
    DAYTONA_RD(reg, &value);                  \
    daytona_cnt_update((u64)value, cnt, clr, 0); \
}

#define DAYTONA_40B_CNT_MAC10G(msb_reg, reg, cnt, clr) \
{                                                      \
    u32 msw, lsw;                                      \
    u64 result = 0;                                    \
    DAYTONA_RD(reg, &lsw);                             \
    DAYTONA_RD(msb_reg, &msw);                         \
    result = msw;                                      \
    result = result << 32;                             \
    result |= lsw;                                     \
    daytona_cnt_update(result, cnt, clr, 1);           \
}

static vtss_rc daytona_mac10g_counters(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no, 
                                       vtss_port_counters_t *const counters,
                                       BOOL clr)
{

    u32 target;
    vtss_port_mac10g_counters_t        *c = &vtss_state->port.counters[port_no].counter.mac10g;
    vtss_port_counter_t                rx_errors;
    vtss_port_rmon_counters_t          *rmon;
    vtss_port_ethernet_like_counters_t *eth_like_cntrs;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_MAC10G, &target));
    /* Note :: Some extra registers which are not required for RMON statistics are omitted */
    /* TBD :: What values do we need to set the other register values to */

#if 0 /* Not required for RMON counters */
    DAYTONA_CNT_MAC10G(40BIT_RX_IN_BYTES, &c->rx_in_bytes, clr);
#endif /* 0 */
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_SYMBOL_ERR_CNT(target), &c->rx_symbol_err, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_PAUSE_CNT(target), &c->rx_pause, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_UNSUP_OPCODE_CNT(target), &c->rx_unsup_opcode, clr);
    DAYTONA_40B_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT_MSB(target), 
                       VTSS_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT(target),
                       &c->rx_ok_bytes, clr);    
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_BAD_BYTES_CNT(target), &c->rx_bad_bytes, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_UC_CNT(target), &c->rx_unicast, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_MC_CNT(target), &c->rx_multicast, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_BC_CNT(target), &c->rx_broadcast, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_CRC_ERR_CNT(target), &c->rx_crc_err, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_UNDERSIZE_CNT(target), &c->rx_undersize, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_FRAGMENTS_CNT(target), &c->rx_fragments, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_IN_RANGE_LEN_ERR_CNT(target), &c->rx_in_range_len_err, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_OUT_OF_RANGE_LEN_ERR_CNT(target), &c->rx_out_of_range_len_err, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_OVERSIZE_CNT(target), &c->rx_oversize, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_JABBERS_CNT(target), &c->rx_jabbers, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_SIZE64_CNT(target), &c->rx_size64, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_SIZE65TO127_CNT(target), &c->rx_size65_127, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_SIZE128TO255_CNT(target), &c->rx_size128_255, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_SIZE256TO511_CNT(target), &c->rx_size256_511, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_SIZE512TO1023_CNT(target), &c->rx_size512_1023, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_SIZE1024TO1518_CNT(target), &c->rx_size1024_1518, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_RX_SIZE1519TOMAX_CNT(target), &c->rx_size1519_max, clr);

#if 0 /* Not required for RMON counters */
    DAYTONA_CNT_MAC10G(40BIT_TX_OUT_BYTES, &c->tx_out_bytes, clr);
#endif /* 0 */
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_PAUSE_CNT(target), &c->tx_pause, clr);
    DAYTONA_40B_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT_MSB(target),
                         VTSS_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT(target),
                         &c->tx_ok_bytes, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_UC_CNT(target), &c->tx_unicast, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_MC_CNT(target), &c->tx_multicast, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_BC_CNT(target), &c->tx_broadcast, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_SIZE64_CNT(target), &c->tx_size64, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_SIZE65TO127_CNT(target), &c->tx_size65_127, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_SIZE128TO255_CNT(target), &c->tx_size128_255, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_SIZE256TO511_CNT(target), &c->tx_size256_511, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_SIZE512TO1023_CNT(target), &c->tx_size512_1023, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_SIZE1024TO1518_CNT(target), &c->tx_size1024_1518, clr);
    DAYTONA_CNT_MAC10G(VTSS_MAC10G_DEV_STATISTICS_TX_SIZE1519TOMAX_CNT(target), &c->tx_size1519_max, clr);

    if (counters == NULL) {
        return VTSS_RC_OK;
    }

    /* RMON Rx counters */
    rmon = &counters->rmon;

    rmon->rx_etherStatsDropEvents = 0;   /* Note :: This counter is not updated */

    rmon->rx_etherStatsOctets = (c->rx_ok_bytes.value + c->rx_bad_bytes.value);
    rx_errors = (c->rx_crc_err.value +  c->rx_undersize.value + c->rx_oversize.value +
                 c->rx_out_of_range_len_err.value + c->rx_symbol_err.value +
                 c->rx_jabbers.value + c->rx_fragments.value);
    rmon->rx_etherStatsPkts = (c->rx_unicast.value + c->rx_multicast.value +
                               c->rx_broadcast.value + rx_errors);
    rmon->rx_etherStatsBroadcastPkts = c->rx_broadcast.value;
    rmon->rx_etherStatsMulticastPkts = c->rx_multicast.value;
    rmon->rx_etherStatsCRCAlignErrors = c->rx_crc_err.value;
    rmon->rx_etherStatsUndersizePkts = c->rx_undersize.value;
    rmon->rx_etherStatsOversizePkts = c->rx_oversize.value;
    rmon->rx_etherStatsFragments = c->rx_fragments.value;
    rmon->rx_etherStatsJabbers = c->rx_jabbers.value;
    rmon->rx_etherStatsPkts64Octets = c->rx_size64.value;
    rmon->rx_etherStatsPkts65to127Octets = c->rx_size65_127.value;
    rmon->rx_etherStatsPkts128to255Octets = c->rx_size128_255.value;
    rmon->rx_etherStatsPkts256to511Octets = c->rx_size256_511.value;
    rmon->rx_etherStatsPkts512to1023Octets = c->rx_size512_1023.value;
    rmon->rx_etherStatsPkts1024to1518Octets = c->rx_size1024_1518.value;
    rmon->rx_etherStatsPkts1519toMaxOctets = c->rx_size1519_max.value;

    /* RMON Tx counters */
    rmon->tx_etherStatsDropEvents = 0;  /* Note :: This counter is not updated */
    /* Note :; This counter needs to be updates with the errors if Any */
    rmon->tx_etherStatsPkts = (c->tx_unicast.value + c->tx_multicast.value +
                                      c->tx_broadcast.value );

    rmon->tx_etherStatsOctets = c->tx_ok_bytes.value;
    rmon->tx_etherStatsBroadcastPkts = c->tx_broadcast.value;
    rmon->tx_etherStatsMulticastPkts = c->tx_multicast.value;
    rmon->tx_etherStatsCollisions = 0; /* TBD ::(c->tx_multi_coll.value + c->tx_backoff1.value +
                                                 c->tx_late_coll.value + c->tx_xcoll.value); */
    rmon->tx_etherStatsPkts64Octets = c->tx_size64.value;
    rmon->tx_etherStatsPkts65to127Octets = c->tx_size65_127.value;
    rmon->tx_etherStatsPkts128to255Octets = c->tx_size128_255.value;
    rmon->tx_etherStatsPkts256to511Octets = c->tx_size256_511.value;
    rmon->tx_etherStatsPkts512to1023Octets = c->tx_size512_1023.value;
    rmon->tx_etherStatsPkts1024to1518Octets = c->tx_size1024_1518.value;
    rmon->tx_etherStatsPkts1519toMaxOctets = c->tx_size1519_max.value;

    eth_like_cntrs = &counters->ethernet_like;

    eth_like_cntrs->dot3InPauseFrames = c->rx_pause.value;
    eth_like_cntrs->dot3OutPauseFrames = c->tx_pause.value;

    return VTSS_RC_OK;
}

static vtss_rc daytona_mac10g_counters_update(vtss_state_t *vtss_state,
                                              const vtss_port_no_t port_no)
{
    return daytona_mac10g_counters(vtss_state, port_no, NULL, 0);
}

static vtss_rc daytona_mac10g_counters_clear(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no)
{
    return daytona_mac10g_counters(vtss_state, port_no, NULL, 1);
}

static vtss_rc daytona_mac10g_counters_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, vtss_port_counters_t *const counters)
{
    return daytona_mac10g_counters(vtss_state, port_no, counters, 0);
}

/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
static vtss_rc daytona_mac10g_event_enable(vtss_state_t              *vtss_state,
                                           const vtss_port_no_t      port_no,
                                           const BOOL                enable,
                                           const vtss_mac10g_event_t ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_MAC10G, &target));

    mask = 0;

    if (ev_mask & VTSS_MAC10G_LINK_DELTA_EV)        mask |= VTSS_M_MAC10G_DEV_CFG_STATUS_LINK_INT_MASK_LINK_DELTA_INT_MASK;

    DAYTONA_WRM(VTSS_MAC10G_DEV_CFG_STATUS_LINK_INT_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if (ev_mask & VTSS_MAC10G_REMOTE_FAULT_STATE_DELTA_EV)       mask |= VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK_REMOTE_FAULT_STATE_INT_MASK;
    if (ev_mask & VTSS_MAC10G_LOCAL_FAULT_STATE_DELTA_EV)        mask |= VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK_LOCAL_FAULT_STATE_INT_MASK;

    DAYTONA_WRM(VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if (ev_mask & VTSS_MAC10G_TX_ABORT_EV)              mask |= VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_TX_ABORT_INT_MASK;
    if (ev_mask & VTSS_MAC10G_RX_NON_STD_PREAM_EV)      mask |= VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_NON_STD_PREAM_INT_MASK;
    if (ev_mask & VTSS_MAC10G_RX_PREAM_ERR_EV)          mask |= VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_ERR_INT_MASK;
    if (ev_mask & VTSS_MAC10G_RX_PREAM_MISMATCH_EV)     mask |= VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_MISMATCH_INT_MASK;
    if (ev_mask & VTSS_MAC10G_RX_PREAM_SHRINK_EV)       mask |= VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_SHRINK_INT_MASK;
//    if (ev_mask & VTSS_MAC10G_TX_IFG_COLUMN_DROP_EV)    mask |= VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_TX_IFG_COLUMN_DROP_INT_MASK;    /* This mask does not exist in the physical design !!!! */

    DAYTONA_WRM(VTSS_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}


static vtss_rc daytona_mac10g_event_poll(vtss_state_t         *vtss_state,
                                         const vtss_port_no_t port_no,
                                         vtss_mac10g_event_t  *const status)
{
    u32 target;
    u32 sticky, mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_MAC10G, &target));

    DAYTONA_RD(VTSS_MAC10G_DEV_CFG_STATUS_LINK_DELTA(target), &sticky);
    DAYTONA_RD(VTSS_MAC10G_DEV_CFG_STATUS_LINK_INT_MASK(target), &mask);
    sticky &= mask;
    DAYTONA_WR(VTSS_MAC10G_DEV_CFG_STATUS_LINK_DELTA(target), sticky);

    *status = 0;

    if (sticky & VTSS_M_MAC10G_DEV_CFG_STATUS_LINK_DELTA_LINK_DELTA)         *status |= VTSS_MAC10G_LINK_DELTA_EV;

    DAYTONA_RD(VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY(target), &sticky);
    DAYTONA_RD(VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK(target), &mask);
    sticky &= mask;
    DAYTONA_WR(VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY(target), sticky);

    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY_REMOTE_FAULT_STATE_DELTA)        *status |= VTSS_MAC10G_REMOTE_FAULT_STATE_DELTA_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY_LOCAL_FAULT_STATE_DELTA)         *status |= VTSS_MAC10G_LOCAL_FAULT_STATE_DELTA_EV;

    DAYTONA_RD(VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY(target), &sticky);
    DAYTONA_RD(VTSS_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK(target), &mask);
    sticky &= mask;
    DAYTONA_WR(VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY(target), sticky);

    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_TX_ABORT_STICKY)              *status |= VTSS_MAC10G_TX_ABORT_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_NON_STD_PREAM_STICKY)      *status |= VTSS_MAC10G_RX_NON_STD_PREAM_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_ERR_STICKY)          *status |= VTSS_MAC10G_RX_PREAM_ERR_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_MISMATCH_STICKY)     *status |= VTSS_MAC10G_RX_PREAM_MISMATCH_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_SHRINK_STICKY)       *status |= VTSS_MAC10G_RX_PREAM_SHRINK_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_TX_IFG_COLUMN_DROP_STICKY)    *status |= VTSS_MAC10G_TX_IFG_COLUMN_DROP_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_mac10g_event_poll_without_mask(vtss_state_t         *vtss_state,
                                                      const vtss_port_no_t port_no,
                                                      vtss_mac10g_event_t  *const status)
{
    u32 target;
    u32 sticky;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_MAC10G, &target));

    DAYTONA_RD(VTSS_MAC10G_DEV_CFG_STATUS_LINK_DELTA(target), &sticky);
    DAYTONA_WR(VTSS_MAC10G_DEV_CFG_STATUS_LINK_DELTA(target), sticky);

    *status = 0;

    if (sticky & VTSS_M_MAC10G_DEV_CFG_STATUS_LINK_DELTA_LINK_DELTA)         *status |= VTSS_MAC10G_LINK_DELTA_EV;

    DAYTONA_RD(VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY(target), &sticky);
    DAYTONA_WR(VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY(target), sticky);

    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY_REMOTE_FAULT_STATE_DELTA)        *status |= VTSS_MAC10G_REMOTE_FAULT_STATE_DELTA_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY_LOCAL_FAULT_STATE_DELTA)         *status |= VTSS_MAC10G_LOCAL_FAULT_STATE_DELTA_EV;

    DAYTONA_RD(VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY(target), &sticky);
    DAYTONA_WR(VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY(target), sticky);

    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_TX_ABORT_STICKY)              *status |= VTSS_MAC10G_TX_ABORT_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_NON_STD_PREAM_STICKY)      *status |= VTSS_MAC10G_RX_NON_STD_PREAM_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_ERR_STICKY)          *status |= VTSS_MAC10G_RX_PREAM_ERR_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_MISMATCH_STICKY)     *status |= VTSS_MAC10G_RX_PREAM_MISMATCH_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_SHRINK_STICKY)       *status |= VTSS_MAC10G_RX_PREAM_SHRINK_EV;
    if (sticky & VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_TX_IFG_COLUMN_DROP_STICKY)    *status |= VTSS_MAC10G_TX_IFG_COLUMN_DROP_EV;

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Utilities and internal
 * ================================================================= */
vtss_rc vtss_daytona_mac10g_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_rc rc;

    rc = daytona_mac10g_counters_clear(vtss_state, port_no);
    rc = daytona_mac10g_config_set(vtss_state, port_no);
    VTSS_D("port_no %d, restart conf set, rc = %x", port_no, rc);
    return VTSS_RC_OK;
}

static vtss_mac10g_cfg_t mac10g_default = {
    .flow_control = {
       .obey    = TRUE,
       .rx_drop = TRUE,
       .generate = TRUE,
    },
    .max_frame_length   = VTSS_DAYTONA_MAC10G_DEFAULT_MAX_LEN,
    .max_tags           = VTSS_DAYTONA_MAC10G_DEFAULT_MAX_TAGS,
    .tx_rx_loopback     = FALSE,
    .rx_tx_loopback     = FALSE,
    .preamble_check     = FALSE,
    .local_fault_force  = FALSE,
    .remote_fault_force = FALSE,

};

/*
 * \brief Set Default Mac10g configuration
 *
 * \return Return code.
 **/
vtss_rc vtss_daytona_mac10g_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_state->mac10g_state[port_no].mac10g_cfg = mac10g_default;

    return VTSS_RC_OK;
}

/**
 * \brief Create instance (set up function pointers).
 *
 * \return Return code.
 **/
vtss_rc vtss_daytona_inst_mac10g_create(vtss_state_t *vtss_state)
{
    vtss_port_no_t port_no;
    vtss_cil_func_t *func = &vtss_state->cil;

    /* TBD :: Determine the mac address that needs to be initialized */
    for (port_no = 0 ; port_no < VTSS_PORT_ARRAY_SIZE; port_no++) {
        vtss_state->mac10g_state[port_no].mac10g_cfg = mac10g_default;
    }

    /* Daytona MAC 10G functions */        
    func->mac10g_config_set               = daytona_mac10g_config_set;
    func->mac10g_status_get               = daytona_mac10g_status_get;
    func->mac10g_counters_update          = daytona_mac10g_counters_update;
    func->mac10g_counters_clear           = daytona_mac10g_counters_clear;
    func->mac10g_counters_get             = daytona_mac10g_counters_get;
    func->mac10g_event_poll               = daytona_mac10g_event_poll;
    func->mac10g_event_poll_without_mask  = daytona_mac10g_event_poll_without_mask;
    func->mac10g_event_enable             = daytona_mac10g_event_enable;

    return VTSS_RC_OK;
}
#endif /* VTSS_ARCH_DAYTONA */
