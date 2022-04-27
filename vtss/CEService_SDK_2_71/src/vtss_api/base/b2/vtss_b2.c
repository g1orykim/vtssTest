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

// Avoid "vtss_api.h not used in module vtss_b2.c"
/*lint --e{766} */

#include "vtss_api.h"

#if defined(VTSS_ARCH_B2)
#include "../ail/vtss_state.h"
#include "vtss_b2.h"
#include "vtss_b2_reg.h"
#include "string.h"


/* Chip port numbers */
#define VTSS_CHIP_PORTS 26
#define CHIP_PORT_10G_0 24
#define CHIP_PORT_10G_1 25
#define CHIP_PORT_SPI4  26 /* Pseudo port */
#define CHIP_PORT_IDLE  31 /* Pseudo port */
#define VTSS_PORT_IS_1G(port) (port < CHIP_PORT_10G_0)
#define VTSS_PORT_IS_10G(port) (port == CHIP_PORT_10G_0 || port == CHIP_PORT_10G_1)

#define B2_PRIOS 8

/* ================================================================= *
 *  Register access
 * ================================================================= */

#define VTSS_BIT(x)                  (1U << (x))
#define VTSS_BITMASK(x)              ((1U << (x)) - 1)
#define VTSS_EXTRACT_BITFIELD(x,o,w) (((x) >> (o)) & VTSS_BITMASK(w))
#define VTSS_ENCODE_BITFIELD(x,o,w)  (((x) & VTSS_BITMASK(w)) << (o))

/* Extract field from register */
#define B2F(tgt, addr, fld, value) \
VTSS_EXTRACT_BITFIELD(value, VTSS_OFF_##tgt##_##addr##_##fld, VTSS_LEN_##tgt##_##addr##_##fld)

/* Read register */
#define B2_RD(tgt, addr, port, value) \
{ \
    vtss_rc rc; \
    if ((rc = b2_rd(vtss_state, VTSS_TGT_##tgt, VTSS_ADDR_##tgt##_##addr, port, value))<VTSS_RC_OK) \
        return rc; \
}

/* Read replicated register */
#define B2_RDX(tgt, addr, x, value) \
{ \
    vtss_rc rc; \
    if ((rc = b2_rd(vtss_state, VTSS_TGT_##tgt, VTSS_ADDX_##tgt##_##addr(x), 0, value))<VTSS_RC_OK) \
        return rc; \
}

/* Read replicated register x,y*/
#define B2_RDXY(tgt, addr, x, y, value) \
{ \
    vtss_rc rc; \
    if ((rc = b2_rd(vtss_state, VTSS_TGT_##tgt, VTSS_ADDXY_##tgt##_##addr(x,y), 0, value))<VTSS_RC_OK) \
        return rc; \
}

/* Write register */
#define B2_WR(tgt, addr, port, value) \
{ \
    vtss_rc rc; \
    if ((rc = b2_wr(vtss_state, VTSS_TGT_##tgt, VTSS_ADDR_##tgt##_##addr, port, value))<VTSS_RC_OK) \
        return rc; \
}

/* Write replicated register */
#define B2_WRX(tgt, addr, x, value) \
{ \
    vtss_rc rc; \
    if ((rc = b2_wr(vtss_state, VTSS_TGT_##tgt, VTSS_ADDX_##tgt##_##addr(x), 0, value))<VTSS_RC_OK) \
        return rc; \
}

/* Write replicated register with a mask */
#define B2_WRXM(tgt, addr, x, value, mask) \
{ \
    u32 val_tmp; \
    vtss_rc rc; \
    if ((rc = b2_rd(vtss_state, VTSS_TGT_##tgt, VTSS_ADDX_##tgt##_##addr(x), 0, &val_tmp))<VTSS_RC_OK) \
        return rc; \
    mask = 0xFFFFFFFF^mask; \
    value = value | (val_tmp & mask); \
    if ((rc = b2_wr(vtss_state, VTSS_TGT_##tgt, VTSS_ADDX_##tgt##_##addr(x), 0, value))<VTSS_RC_OK) \
        return rc; \
}

/* Write replicated register x,y */
#define B2_WRXY(tgt, addr, x, y, value) \
{ \
    vtss_rc rc; \
    if ((rc = b2_wr(vtss_state, VTSS_TGT_##tgt, VTSS_ADDXY_##tgt##_##addr(x,y), 0, value))<VTSS_RC_OK) \
        return rc; \
}

/* Read register field */
#define B2_RDF(tgt, addr, fld, port, value) \
{ \
    vtss_rc rc; \
    if ((rc = b2_rdf(vtss_state, VTSS_TGT_##tgt, VTSS_ADDR_##tgt##_##addr, VTSS_OFF_##tgt##_##addr##_##fld, VTSS_LEN_##tgt##_##addr##_##fld, port, value))<VTSS_RC_OK) \
        return rc; \
}

/* Write register field */
#define B2_WRF(tgt, addr, fld, port, value) \
{ \
    vtss_rc rc; \
    if ((rc = b2_wrf(vtss_state, VTSS_TGT_##tgt, VTSS_ADDR_##tgt##_##addr, VTSS_OFF_##tgt##_##addr##_##fld, VTSS_LEN_##tgt##_##addr##_##fld, port, value))<VTSS_RC_OK) \
        return rc; \
}

/* Read or write target register via PI */
static vtss_rc b2_pi_rd_wr(vtss_state_t *vtss_state,
                           u32 tgt, u32 addr, u32 *value, BOOL read)
{
    u32              address, val, mask, i;
    vtss_reg_read_t  read_func;
    vtss_reg_write_t write_func;
    
    read_func = vtss_state->init_conf.reg_read;
    write_func = vtss_state->init_conf.reg_write;

    address = (tgt << 18);
    if (tgt == VTSS_TGT_DEVCPU_ORG || tgt == VTSS_TGT_FAST_REGS) {
        /* Direct operation */
        address += (addr << 1);
        return (read ? read_func(0, address, value) : write_func(0, address, *value));
    }

    /* Indirect operation */
    address += (addr << 2);
    if (read) {
        VTSS_RC(read_func(0, address, &val));
    } else {
        val = *value;
        VTSS_RC(write_func(0, address,     val >> 16));    /* MSB */
        VTSS_RC(write_func(0, address + 2, val & 0xffff)); /* LSB */
    }

    /* Wait for operation to complete */
    mask = ((1 << VTSS_OFF_FAST_REGS_CFG_STATUS_2_WR_IN_PROGRESS) |
            (1 << VTSS_OFF_FAST_REGS_CFG_STATUS_2_RD_IN_PROGRESS));
    for (i = 0; i < 25; i++) {
        VTSS_RC(b2_pi_rd_wr(vtss_state, VTSS_TGT_FAST_REGS,
                            VTSS_ADDR_FAST_REGS_CFG_STATUS_2, &val, 1));
        if ((val & mask) == 0) {
            /* Read/write operation completed */
            if (read) {
                VTSS_RC(b2_pi_rd_wr(vtss_state, VTSS_TGT_FAST_REGS, 
                                    VTSS_ADDR_FAST_REGS_SLOWDATA_MSB, &val, 1));
                VTSS_RC(b2_pi_rd_wr(vtss_state, VTSS_TGT_FAST_REGS, 
                                    VTSS_ADDR_FAST_REGS_SLOWDATA_LSB, value, 1));
                *value += (val << 16);
            }
            return VTSS_RC_OK;
        }
        VTSS_NSLEEP(100);
    }    
    return VTSS_RC_ERROR;
}

/* Read/write register */
static vtss_rc b2_rd_wr(vtss_state_t *vtss_state,
                        u32 tgt, u32 addr, u32 port, u32 *value, BOOL read)
{
    switch (tgt) {
    case VTSS_TGT_ASM:
        if (addr < VTSS_ADDR_ASM_STAT_CFG) {
            if (VTSS_PORT_IS_1G(port))
                addr += port*VTSS_WIDTH_ASM_DEV_STATISTICS;
            else {
                VTSS_E("illegal port for 1G target: %u", port);
                return VTSS_RC_ERROR;
            }
        } else if (port) {
            VTSS_E("non-zero port for ASM target");
            return VTSS_RC_ERROR;
        }
        break;
    case VTSS_TGT_DEV1G:
        if (VTSS_PORT_IS_1G(port))
            tgt += port;
        else {
            VTSS_E("illegal port for 1G target: %u", port);
            return VTSS_RC_ERROR;
        }
        break;
    case VTSS_TGT_DEV10G:
    case VTSS_TGT_XAUI_PHY_STAT:
        if (port == CHIP_PORT_10G_1)
            tgt += 2;
        else if (port != CHIP_PORT_10G_0) {
            VTSS_E("illegal port for 10G target: %u", port);
            return VTSS_RC_ERROR;
        }
        break;
    case VTSS_TGT_DEVCPU_ORG:
    case VTSS_TGT_DEVCPU_GCB:
    case VTSS_TGT_DSM:
    case VTSS_TGT_ANA_CL:
    case VTSS_TGT_ANA_AC:
    case VTSS_TGT_SCH:
    case VTSS_TGT_QSS:
    case VTSS_TGT_DEVSPI:
    case VTSS_TGT_FAST_REGS:
        if (port) {
            VTSS_E("non-zero port for target: 0x%02x", tgt);
            return VTSS_RC_ERROR;
        }
        break;
    default:
        VTSS_E("illegal target: 0x%02x", tgt);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(b2_pi_rd_wr(vtss_state, tgt, addr, value, read));
    VTSS_N("%s, tgt: 0x%02x, addr: 0x%04x, value: 0x%08x", 
           read ? "RD" : "WR", tgt, addr, *value);
    return VTSS_RC_OK;
}

/* Read register */
static vtss_rc b2_rd(vtss_state_t *vtss_state, u32 tgt, u32 addr, u32 port, u32 *value)
{
    return b2_rd_wr(vtss_state, tgt, addr, port, value, 1);
}

/* Write register */
static vtss_rc b2_wr(vtss_state_t *vtss_state, u32 tgt, u32 addr, u32 port, u32 value)
{
    return b2_rd_wr(vtss_state, tgt, addr, port, &value, 0);
}


/* Read register field */
static vtss_rc b2_rdf(vtss_state_t *vtss_state,
                      u32 tgt, u32 addr, u32 offset, u32 length, u32 port, u32 *value)
{
    VTSS_RC(b2_rd_wr(vtss_state, tgt, addr, port, value, 1));
    *value = VTSS_EXTRACT_BITFIELD(*value, offset, length);
    return VTSS_RC_OK;
}

/* Write register field */
static vtss_rc b2_wrf(vtss_state_t *vtss_state,
                      u32 tgt, u32 addr, u32 offset, u32 length, u32 port, u32 value)
{
    u32 val, mask;
    
    mask = VTSS_BITMASK(length);
    if ((value & mask) != value) {
        VTSS_E("illegal value: 0x%08x, tgt: %u, addr: %u, offs: %u, length: %u, port: %u",
               value, tgt, addr, offset, length, port);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(b2_rd_wr(vtss_state, tgt, addr, port, &val, 1));
    val = ((val & ~(mask << offset)) | (value << offset));
    VTSS_RC(b2_rd_wr(vtss_state, tgt, addr, port, &val, 0));
    return VTSS_RC_OK;
}

/* Revision C feature check */
static vtss_rc b2_rev_c_check(vtss_state_t *vtss_state, const char *feature)
{
    if (vtss_state->misc.chip_id.revision < 3) {
        VTSS_E("%s only available in chip version -01", feature);
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

/* Check if the port is a Host Port */
static BOOL b2_port_is_host(vtss_state_t *vtss_state, int chip_port) 
{
    
    if ((chip_port == CHIP_PORT_SPI4) || 
       ((chip_port == CHIP_PORT_10G_0 || 
         chip_port == CHIP_PORT_10G_1) && 
        (vtss_state->init_conf.host_mode < VTSS_HOST_MODE_4))) {
        return TRUE;
    } 
    return FALSE;
}

/* ================================================================= *
 *  Port Control      
 * ================================================================= */

/* PHY commands */
#define PHY_CMD_ADDRESS  0 /* 10G only */
#define PHY_CMD_WRITE    1 
#define PHY_CMD_READ_INC 2 /* NB: READ for 1G */
#define PHY_CMD_READ     3 /* 10G only */

static vtss_rc b2_miim_cmd(vtss_state_t *vtss_state,
                           u32 cmd, u32 sof, vtss_miim_controller_t miim_controller,
                           u8 miim_addr, u8 addr, u16 *data, BOOL report_errors)
{
    u32 i, n;
    u32 value;


    if (vtss_state->port.miim_addr_err[miim_addr] ) {
      return VTSS_RC_ERROR;      
    }
    switch (miim_controller) {
    case VTSS_MIIM_CONTROLLER_0:
        i = 0;
        break;
    case VTSS_MIIM_CONTROLLER_1:
        i = 1;
        break;
    default:
        VTSS_E("illegal miim_controller: %d", miim_controller);
        return VTSS_RC_ERROR;
    }

    /* Set Start of frame field */
    B2_WRX(DEVCPU_GCB, MIIM_CFG, i,
           (sof << VTSS_OFF_DEVCPU_GCB_MIIM_CFG_MIIM_ST_CFG_FIELD) |
           (0x40 << VTSS_OFF_DEVCPU_GCB_MIIM_CFG_MIIM_CFG_PRESCALE));


    /* Read command is different for Clause 22 */
    if (sof == 1 && cmd == PHY_CMD_READ) {
        cmd = PHY_CMD_READ_INC;
    }

    B2_WRX(DEVCPU_GCB, MIIM_CMD, i,
           ((unsigned)1 << VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_VLD) |
           (miim_addr << VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_PHYAD) |
           (addr << VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_REGAD) |
           (*data << VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_WRDATA) |
           (cmd << VTSS_OFF_DEVCPU_GCB_MIIM_CMD_MIIM_CMD_OPR_FIELD));



    /* Wait for access to complete */
    for (n = 0; ; n++) {
        B2_RDX(DEVCPU_GCB, MIIM_STATUS, i, &value);
        if (B2F(DEVCPU_GCB, MIIM_STATUS, MIIM_STAT_PENDING_RD, value) == 0 &&
            B2F(DEVCPU_GCB, MIIM_STATUS, MIIM_STAT_PENDING_WR, value) == 0)
            break;
        if (n == 1000) {
          goto mmd_error;
        }
    }

    /* Read data */
    if (cmd == PHY_CMD_READ || cmd == PHY_CMD_READ_INC) {
        B2_RDX(DEVCPU_GCB, MIIM_DATA, i, &value);
        if (B2F(DEVCPU_GCB, MIIM_DATA, MIIM_DATA_SUCCESS, value)) {
          goto mmd_error;
        }
        *data = B2F(DEVCPU_GCB, MIIM_DATA, MIIM_DATA_RDDATA, value);
    }
    return VTSS_RC_OK;

 mmd_error:
    if (report_errors) {
        VTSS_E("miim failed, cmd: %s, miim_addr: %u, addr: %u", 
               cmd == PHY_CMD_READ ? "PHY_CMD_READ" : 
               cmd == PHY_CMD_WRITE ? "PHY_CMD_WRITE" : 
               cmd == PHY_CMD_ADDRESS ? "PHY_CMD_ADDRESS" : "PHY_CMD_READ_INC", 
               miim_addr, addr);
    }
    vtss_state->port.miim_addr_err[miim_addr] = TRUE;
    return VTSS_RC_ERROR;

}

static vtss_rc b2_miim_read(vtss_state_t *vtss_state,
                            vtss_miim_controller_t miim_controller, 
                            u8 miim_addr, u8 addr, u16 *value, BOOL report_errors)
{
    return b2_miim_cmd(vtss_state, PHY_CMD_READ, 1, miim_controller, miim_addr, addr,
                       value, report_errors);
}

static vtss_rc b2_miim_write(vtss_state_t *vtss_state,
                             vtss_miim_controller_t miim_controller, 
                             u8 miim_addr, u8 addr, u16 value, BOOL report_errors)
{
    return b2_miim_cmd(vtss_state, PHY_CMD_WRITE, 1, miim_controller, miim_addr, addr,
                       &value, report_errors);
}

/* MMD (MDIO Management Devices (10G)) read */
static vtss_rc b2_mmd_read(vtss_state_t *vtss_state,
                           vtss_miim_controller_t miim_controller, 
                           u8 miim_addr, u8 mmd, u16 addr, u16 *value, BOOL report_errors)
{
    VTSS_RC(b2_miim_cmd(vtss_state, PHY_CMD_ADDRESS, 0, miim_controller, miim_addr, mmd,
                        &addr, 1));
    return b2_miim_cmd(vtss_state, PHY_CMD_READ, 0, miim_controller, miim_addr, mmd, value, 1);
}

/* MMD (MDIO Management Devices (10G)) write */
static vtss_rc b2_mmd_write(vtss_state_t *vtss_state,
                            vtss_miim_controller_t miim_controller, 
                            u8 miim_addr, u8 mmd, u16 addr, u16 data, BOOL report_errors)
{
    VTSS_RC(b2_miim_cmd(vtss_state, PHY_CMD_ADDRESS, 0, miim_controller, miim_addr, mmd,
                        &addr, 1));
    return b2_miim_cmd(vtss_state, PHY_CMD_WRITE, 0, miim_controller, miim_addr, mmd, &data, 1);
}

/* Convert from chip port to line port */
static u32 b2_port2line(u32 port)
{
    if (port >= CHIP_PORT_10G_0)
        port -= CHIP_PORT_10G_0;
    return port; 
}

static vtss_rc b2_ct_setup(vtss_state_t *vtss_state,
                           u32 chip_port, const vtss_port_ct_t *ct, int maxlen)
{
    u32 aport, dport, aport_min, aport_max;
    u32 rx_threshold, tx_threshold;
    vtss_host_mode_t host_mode;
    
    /* Cut-through only supported in Chip Rev C  */
    VTSS_RC(b2_rev_c_check(vtss_state, "Cut-through"));

    if (ct->rx_ct/128 > 0x1fff || ct->tx_ct/128 > 0x1fff) {
        VTSS_E("illegal CT value");
        return VTSS_RC_ERROR;
    }
    
    aport = b2_port2line(chip_port);
    dport = vtss_state->port.dep_port[aport];
    host_mode = vtss_state->init_conf.host_mode;
            
    if (ct->rx_ct == 0) {        
        rx_threshold = 0x1fff; /* Disable thresehold   */
    } else {
        rx_threshold = ct->rx_ct/128;
    }

    if (ct->tx_ct == 0) {
        tx_threshold = 0x1fff; /* Disable thresehold   */
    } else {
        tx_threshold = ct->tx_ct/128;
    }
   
    if (host_mode < VTSS_HOST_MODE_8) {
        B2_WRX(QSS, PORT_Q_CT_TH, aport, tx_threshold);    
        B2_WRX(QSS, PORT_Q_CT_TH, dport, rx_threshold);    
    } else {
        /* For Hostmodes 8-11, lports are configured with a common Rx/Tx threshold  */
        if (host_mode == VTSS_HOST_MODE_8) {  /* Dual XAUI -> SPI4  - Aggregator */            
            if (chip_port == 24) {
                aport_min = 0;
                aport_max = 23;
            } else {
                aport_min = 24;
                aport_max = 47;
            }
        } else if (host_mode == VTSS_HOST_MODE_9) { /* Dual XAUI -> SPI4 - Converter */ 
            if (chip_port == 24) {
                aport_min = 0;
                aport_max = 7;
            } else {
                aport_min = 24;
                aport_max = 31;
            }
        } else {
            /* Host mode 10-11,  Single XAUI -> SPI4  - Aggregator*/ 
            aport_min = 0;
            aport_max = 47;
        }

        for (aport = aport_min; aport <= aport_max; aport++) {
            B2_WRX(QSS, PORT_Q_CT_TH, aport, tx_threshold);    
            B2_WRX(QSS, PORT_Q_CT_TH, vtss_state->port.dep_port[aport], rx_threshold);    
        }         
    }
    return VTSS_RC_OK;
}

/* Port setup common for 1G and 10G ports */
static vtss_rc b2_port_setup(vtss_state_t *vtss_state, u32 port, vtss_port_conf_t *conf)
{
    const u8 *mac;
    u32      wm_low = 0, wm_high, wm_stop = 3, mac_hi, mac_lo;
    
    switch (conf->speed) {
    case VTSS_SPEED_10M:
        wm_high = 3;
        break;
    case VTSS_SPEED_100M:
        wm_high = 21;
        break;
    case VTSS_SPEED_1G:
        wm_low = 204;
        wm_high = 206;
        break;
    case VTSS_SPEED_10G:
        wm_low = 511;
        wm_high = 513;
        wm_stop = 5;
        break;
    case VTSS_SPEED_12G:
        wm_low = 614;
        wm_high = 617;
        wm_stop = 5;
        break;
    default:
        VTSS_E("illegal speed");
        return VTSS_RC_ERROR;
    }
    
    B2_WRX(DSM, SCH_STOP_WM_CFG, port, wm_stop << VTSS_OFF_DSM_SCH_STOP_WM_CFG_SCH_STOP_WM);
    B2_WRX(DSM, RATE_CTRL, port, 
           ((unsigned)12 << VTSS_OFF_DSM_RATE_CTRL_FRM_GAP_COMP) |
           (wm_high << VTSS_OFF_DSM_RATE_CTRL_TAXI_RATE_HIGH) |
           (wm_low << VTSS_OFF_DSM_RATE_CTRL_TAXI_RATE_LOW));

    B2_WRX(DSM, RX_PAUSE_CFG, port, 
           (conf->flow_control.obey ? 1 : 0) << VTSS_OFF_DSM_RX_PAUSE_CFG_RX_PAUSE_EN);
    
    B2_WRX(DSM, ETH_FC_GEN, port, 
           (conf->flow_control.generate ? 1 : 0) << VTSS_OFF_DSM_ETH_FC_GEN_ETH_PORT_FC_GEN);
    
    B2_WRX(DSM, MAC_CFG, port, 
           (0xff << VTSS_OFF_DSM_MAC_CFG_TX_PAUSE_VAL) |
           ((conf->fdx ? 0 : 1) << VTSS_OFF_DSM_MAC_CFG_HDX_BACKPRESSURE) |
           (0 << VTSS_OFF_DSM_MAC_CFG_SEND_PAUSE_FRM_TWICE) |
           (1 << VTSS_OFF_DSM_MAC_CFG_TX_PAUSE_XON_XOFF));
    
    mac = conf->flow_control.smac.addr;
    mac_hi = ((mac[0] << 16) | (mac[1] << 8) | (mac[2] << 0));
    mac_lo = ((mac[3] << 16) | (mac[4] << 8) | (mac[5] << 0)); 
    B2_WRX(DSM, MAC_ADDR_HIGH_CFG, port, 
           mac_hi << VTSS_OFF_DSM_MAC_ADDR_HIGH_CFG_MAC_ADDR_HIGH);
    B2_WRX(DSM, MAC_ADDR_LOW_CFG, port, 
           mac_lo << VTSS_OFF_DSM_MAC_ADDR_LOW_CFG_MAC_ADDR_LOW);
    B2_WRX(ASM, MAC_ADDR_HIGH_CFG, port, 
           mac_hi << VTSS_OFF_ASM_MAC_ADDR_HIGH_CFG_MAC_ADDR_HIGH);
    B2_WRX(ASM, MAC_ADDR_LOW_CFG, port, 
           mac_lo << VTSS_OFF_ASM_MAC_ADDR_LOW_CFG_MAC_ADDR_LOW);
    B2_WRX(ASM, PAUSE_CFG, port, 1 << VTSS_OFF_ASM_PAUSE_CFG_ABORT_PAUSE_ENA);

    /* Enable / Disable Cut-Through */
    VTSS_RC(b2_ct_setup(vtss_state, port, &conf->ct, conf->max_frame_length));  

    /* PFCI */
    if (conf->flow_control.pfci_enable) {
      VTSS_D("PFCI enabled");
      VTSS_RC(b2_rev_c_check(vtss_state, "PFCI")); // Print error message if wrong chip revision
    }
    VTSS_RC(b2_wrf(vtss_state, VTSS_TGT_QSS, VTSS_ADDR_QSS_PFCI_LPORT_ENA, b2_port2line(port), 
                   1, 0, conf->flow_control.pfci_enable ? 1 : 0));
    
    return VTSS_RC_OK;
}

/* Determine queue ID */
static u32 b2_qid(vtss_state_t *vtss_state, u32 dport, u32 queue)
{
    return ((dport - (vtss_state->init_conf.host_mode < VTSS_HOST_MODE_4 ? 24 : 48))*B2_PRIOS + queue);
}

/* Flush queue */
static vtss_rc b2_queue_flush(vtss_state_t *vtss_state, u32 port, u32 queue)
{
    u32 i, value;
    
    B2_WR(QSS, Q_FLUSH, 0,
          (queue << VTSS_OFF_QSS_Q_FLUSH_Q_FLUSH_PRIO) |
          (port << VTSS_OFF_QSS_Q_FLUSH_Q_FLUSH_PORT));
    B2_WR(QSS, Q_FLUSH_REQ, 0, 1 << VTSS_OFF_QSS_Q_FLUSH_REQ_Q_FLUSH_REQ);

    for (i = 0; i < 100; i++) {
        B2_RDF(QSS, Q_FLUSH_REQ, Q_FLUSH_REQ, 0, &value);
        if (value == 0)
            return VTSS_RC_OK;
    }
    VTSS_E("flush failed, port: %u, queue: %u", port, queue);
    return VTSS_RC_ERROR;
}

static vtss_rc b2_port_conf_1g_set(vtss_state_t *vtss_state, u32 port, vtss_port_conf_t *conf)
{
    u32               aport, giga = 0, fdx, clk, rx_ifg1, rx_ifg2, tx_ifg;
    u32               sgmii = 0, if_100fx = 0, disabled;
    vtss_port_speed_t speed;
    
    disabled = (conf->power_down ? 1 : 0);

    fdx = conf->fdx;
    speed = conf->speed;
    switch (speed) {
    case VTSS_SPEED_10M:
        clk = 0;
        break;
    case VTSS_SPEED_100M:
        clk = 1;
        break;
    case VTSS_SPEED_1G:
        clk = 2;
        giga = 1;
        break;
    default:
        VTSS_E("illegal speed: %d", speed);
        return VTSS_RC_ERROR;
    }

    /* Interface type */
    switch (conf->if_type) {
    case VTSS_PORT_INTERFACE_SGMII:
        sgmii = 1; /* Implies serdes = 0  */
        break;
    case VTSS_PORT_INTERFACE_100FX:
        if_100fx = 1;      
        break;
    default:
        VTSS_E("illegal interface type");
        return VTSS_RC_ERROR;
    }

    /* Disable and flush Tx queue */
    aport = b2_port2line(port);
    B2_WRX(QSS, PORT_ENA, aport, 0 << VTSS_OFF_QSS_PORT_ENA_PORT_ENA);
    VTSS_RC(b2_queue_flush(vtss_state, aport, 0));    
        
    /* Disable Rx and Tx in MAC */
    B2_WR(DEV1G, MAC_ENA_CFG, port, 
          (0 << VTSS_OFF_DEV1G_MAC_ENA_CFG_RX_ENA) |
          (0 << VTSS_OFF_DEV1G_MAC_ENA_CFG_TX_ENA));

    /* Reset MAC */
    B2_WR(DEV1G, DEV_RST_CTRL, port, 
          (1 << VTSS_OFF_DEV1G_DEV_RST_CTRL_MAC_TX_RST) |
          (1 << VTSS_OFF_DEV1G_DEV_RST_CTRL_MAC_RX_RST));
    
    /* Common port setup */
    VTSS_RC(b2_port_setup(vtss_state, port, conf));

    /* Interframe gaps */
    rx_ifg1 = conf->frame_gaps.hdx_gap_1;
    if (rx_ifg1 == VTSS_FRAME_GAP_DEFAULT)
        rx_ifg1 = (giga ? 5 : 7);
    
    rx_ifg2 = conf->frame_gaps.hdx_gap_2;
    if (rx_ifg2 == VTSS_FRAME_GAP_DEFAULT)
        rx_ifg2 = (giga ? 1 : fdx ? 11 : 8);

    tx_ifg = conf->frame_gaps.fdx_gap;
    if (tx_ifg == VTSS_FRAME_GAP_DEFAULT)
        tx_ifg = (giga ? 6 : (fdx == 0 && speed == VTSS_SPEED_100M) ? 15 : 17);
    
    B2_WR(DEV1G, MAC_IFG_CFG, port,
          (tx_ifg << VTSS_OFF_DEV1G_MAC_IFG_CFG_TX_IFG) |
          (rx_ifg2 << VTSS_OFF_DEV1G_MAC_IFG_CFG_RX_IFG2) |
          (rx_ifg1 << VTSS_OFF_DEV1G_MAC_IFG_CFG_RX_IFG1));
    
    /* Frame length */
    B2_WR(DEV1G, MAC_MAXLEN_CFG, port, conf->max_frame_length);

    /* Half duplex */
    B2_WR(DEV1G, MAC_HDX_CFG, port,
          (conf->flow_control.smac.addr[5] << VTSS_OFF_DEV1G_MAC_HDX_CFG_SEED) |
          ((fdx ? 0 : 1) << VTSS_OFF_DEV1G_MAC_HDX_CFG_SEED_LOAD) |
          (0 << VTSS_OFF_DEV1G_MAC_HDX_CFG_RETRY_AFTER_EXC_COL_ENA) |
          (0x41 << VTSS_OFF_DEV1G_MAC_HDX_CFG_LATE_COL_POS));

    /* SGMII */
    B2_WR(DEV1G, PCS1G_MODE_CFG, port, sgmii << VTSS_OFF_DEV1G_PCS1G_MODE_CFG_SGMII_MODE_ENA);

    /* Signal detect */
    B2_WR(DEV1G, PCS1G_SD_CFG, port, 
          ((conf->sd_active_high ? 1 : 0) << 
           VTSS_OFF_DEV1G_PCS1G_SD_CFG_SD_POL) |
          ((conf->sd_enable ? 1 : 0) << VTSS_OFF_DEV1G_PCS1G_SD_CFG_SD_ENA));


    /* Release MAC from reset */
    B2_WR(DEV1G, DEV_RST_CTRL, port, 
          ((disabled ? 5 : clk) << VTSS_OFF_DEV1G_DEV_RST_CTRL_SPEED_SEL) |
          (if_100fx << VTSS_OFF_DEV1G_DEV_RST_CTRL_FX100_ENABLE) |
          (0 << VTSS_OFF_DEV1G_DEV_RST_CTRL_SGMII_MACRO_RST) |
          (0 << VTSS_OFF_DEV1G_DEV_RST_CTRL_FX100_PCS_TX_RST) |
          (0 << VTSS_OFF_DEV1G_DEV_RST_CTRL_FX100_PCS_RX_RST) |
          (disabled << VTSS_OFF_DEV1G_DEV_RST_CTRL_PCS_TX_RST) |
          (disabled << VTSS_OFF_DEV1G_DEV_RST_CTRL_PCS_RX_RST) |
          (disabled << VTSS_OFF_DEV1G_DEV_RST_CTRL_MAC_TX_RST) |
          (disabled << VTSS_OFF_DEV1G_DEV_RST_CTRL_MAC_RX_RST));


    /* MAC mode */
    B2_WR(DEV1G, MAC_MODE_CFG, port, 
          (giga << VTSS_OFF_DEV1G_MAC_MODE_CFG_GIGA_MODE_ENA) |
          (fdx << VTSS_OFF_DEV1G_MAC_MODE_CFG_FDX_ENA));

    /* Enable Rx and Tx in MAC */
    B2_WR(DEV1G, MAC_ENA_CFG, port, 
          (1 << VTSS_OFF_DEV1G_MAC_ENA_CFG_RX_ENA) |
          (1 << VTSS_OFF_DEV1G_MAC_ENA_CFG_TX_ENA));

    /* Enable Tx queue */
    B2_WRX(QSS, PORT_ENA, aport, 1 << VTSS_OFF_QSS_PORT_ENA_PORT_ENA);
      
    return VTSS_RC_OK;
}

static vtss_rc b2_port_conf_10g_set(vtss_state_t *vtss_state, u32 port, vtss_port_conf_t *conf)
{
    u32               disabled;
    vtss_host_mode_t  host_mode;
    BOOL              hih;
    vtss_port_speed_t speed;

   
    VTSS_D("port: %u", port);

    disabled = (conf->power_down ? 1 : 0);
    host_mode = vtss_state->init_conf.host_mode;
    hih = (host_mode < VTSS_HOST_MODE_4 || host_mode == VTSS_HOST_MODE_8 || host_mode > VTSS_HOST_MODE_9);
    speed = conf->speed;
    if (speed == VTSS_SPEED_12G) {
        VTSS_RC(b2_rev_c_check(vtss_state, "12G"));
    } else if (speed != VTSS_SPEED_10G) {
        VTSS_E("illegal speed");
        return VTSS_RC_ERROR;
    }

    /* Disable Rx and Tx in MAC */
    B2_WR(DEV10G, MAC_ENA_CFG, port, 
          (0 << VTSS_OFF_DEV10G_MAC_ENA_CFG_RX_ENA) |
          (0 << VTSS_OFF_DEV10G_MAC_ENA_CFG_TX_ENA));

    /* Gnats #6580 PLL runaway.  Workaround copied from TCL code - but not tested on a defect chip */    
    /* Power off/on CMU to make sure that the PLL comes back if it has run away        */    
    B2_WR(XAUI_PHY_STAT, CMU_CTRL, port, 
          (12 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_TXLBW) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_TXLDPPM) |
          (1 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_REF_SRC) |
          (1 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_REFSEL) |
          (1 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_CMUPOWEROFF) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_SETUP_CP));
    VTSS_MSLEEP(20);
    B2_WR(XAUI_PHY_STAT, CMU_CTRL, port, 
          (12 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_TXLBW) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_TXLDPPM) |
          (1 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_REF_SRC) |
          (1 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_REFSEL) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_CMUPOWEROFF) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_SETUP_CP));
    /* Workaround completed */
    
    /* Reset MAC */
    B2_WR(DEV10G, DEV_RST_CTRL, port, 
          (0 << VTSS_OFF_DEV10G_DEV_RST_CTRL_CLK_DIVIDE_SEL) |
          (4 << VTSS_OFF_DEV10G_DEV_RST_CTRL_SPEED_SEL) |
          (0 << VTSS_OFF_DEV10G_DEV_RST_CTRL_CLK_DRIVE_EN) |
          (1 << VTSS_OFF_DEV10G_DEV_RST_CTRL_PCS_TX_RST) |
          (1 << VTSS_OFF_DEV10G_DEV_RST_CTRL_PCS_RX_RST) |
          (1 << VTSS_OFF_DEV10G_DEV_RST_CTRL_MAC_TX_RST) |
          (1 << VTSS_OFF_DEV10G_DEV_RST_CTRL_MAC_RX_RST));
    
    /* Speed selection */
    B2_WR(XAUI_PHY_STAT, CMU_CTRL, port, 
          (12 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_TXLBW) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_TXLDPPM) |
          (1 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_REF_SRC) |
          ((speed == VTSS_SPEED_12G ? 0 : 1) << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_REFSEL) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_CMUPOWEROFF) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_CMU_CTRL_SETUP_CP));

    /* Common port setup */
    VTSS_RC(b2_port_setup(vtss_state, port, conf));

    /* Frame length */
    B2_WR(DEV10G, MAC_MAXLEN_CFG, port, conf->max_frame_length);
    /* Preamble checking */
    B2_WR(DEV10G, MAC_ADV_CHK_CFG, port,
          (0 << VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_FCS_ERROR_DISCARD_DIS) |
          (0 << VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_EXT_EOP_CHK_ENA) |
          (0 << VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_EXT_SOP_CHK_ENA) |
          ((hih ? 0 : 1) << VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_SFD_CHK_ENA) |
          (1 << VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_PRM_SHK_CHK_DIS) |
          (0 << VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_PRM_CHK_ENA) |
          (0 << VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_OOR_ERR_ENA) |
          (0 << VTSS_OFF_DEV10G_MAC_ADV_CHK_CFG_INR_ERR_ENA));
    
    /* Link fault signalling used for line ports */
    B2_WR(DEV10G, MAC_LFS_CFG, port, 
          (0 << VTSS_OFF_DEV10G_MAC_LFS_CFG_LFS_INH_TX) |
          (0 << VTSS_OFF_DEV10G_MAC_LFS_CFG_LFS_DIS_TX) |
          ((hih ? 0 : 1) << VTSS_OFF_DEV10G_MAC_LFS_CFG_LFS_MODE_ENA));
    
    /* Release MAC from reset */
    B2_WR(DEV10G, DEV_RST_CTRL, port, 
          (0 << VTSS_OFF_DEV10G_DEV_RST_CTRL_CLK_DIVIDE_SEL) |
          ((disabled ? 5 : 4) << VTSS_OFF_DEV10G_DEV_RST_CTRL_SPEED_SEL) |
          (0 << VTSS_OFF_DEV10G_DEV_RST_CTRL_CLK_DRIVE_EN) |
          (disabled << VTSS_OFF_DEV10G_DEV_RST_CTRL_PCS_TX_RST) |
          (disabled << VTSS_OFF_DEV10G_DEV_RST_CTRL_PCS_RX_RST) |
          (disabled << VTSS_OFF_DEV10G_DEV_RST_CTRL_MAC_TX_RST) |
          (disabled << VTSS_OFF_DEV10G_DEV_RST_CTRL_MAC_RX_RST));
    
    /* Enable Rx and Tx in MAC */
    B2_WR(DEV10G, MAC_ENA_CFG, port, 
          (1 << VTSS_OFF_DEV10G_MAC_ENA_CFG_RX_ENA) |
          (1 << VTSS_OFF_DEV10G_MAC_ENA_CFG_TX_ENA));

    return VTSS_RC_OK;
}

static vtss_rc b2_port_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    u32              port = VTSS_CHIP_PORT(port_no);
    vtss_port_conf_t *conf = &vtss_state->port.conf[port_no];

    if (b2_port_is_host(vtss_state, port)) 
        return VTSS_RC_ERROR;
       
    
    if (VTSS_PORT_IS_10G(port))
        return b2_port_conf_10g_set(vtss_state, port, conf);
    else
        return b2_port_conf_1g_set(vtss_state, port, conf);
}

static vtss_rc b2_cnt(vtss_state_t *vtss_state,
                      u32 tgt, u32 addr, u32 port, vtss_chip_counter_t *counter, BOOL clear)
{
    vtss_rc rc;
    u32     value;
    
    /* Read chip counter */
    if ((rc = b2_rd(vtss_state, tgt, addr, port, &value)) != VTSS_RC_OK)
        return rc;
    
    if (clear) {
        /* Clear counter */
        counter->value = 0;
    } else {
        /* Accumulate counter */
        if (value >= counter->prev) {
            /* Not wrapped */
            counter->value += (value - counter->prev);
        } else {
            /* Wrapped */
            counter->value += (value + 0x100000000 - counter->prev);
        }
    }
    counter->prev = value;
    return VTSS_RC_OK;
}

#define B2_CNT(tgt, addr, port, counter, clear) \
{ \
    vtss_rc rc; \
    if ((rc = b2_cnt(vtss_state, VTSS_TGT_##tgt, VTSS_ADDR_##tgt##_##addr, port, counter, clear))<VTSS_RC_OK) \
        return rc; \
}

#define B2_CNTX(tgt, addr, x, counter, clear) \
{ \
    vtss_rc rc; \
    if ((rc = b2_cnt(vtss_state, VTSS_TGT_##tgt, VTSS_ADDX_##tgt##_##addr(x), 0, counter, clear))<VTSS_RC_OK) \
        return rc; \
}

static vtss_rc b2_port_counters_cmd(vtss_state_t *vtss_state,
                                    const vtss_port_no_t port_no, 
                                    vtss_port_counters_t *const counters, BOOL clr)
{
    u32                                port, aport, dport, i, qid;
    vtss_port_counter_t                rx_errors;
    vtss_port_b2_counters_t            *c;
    vtss_port_rmon_counters_t          *rmon;
    vtss_port_if_group_counters_t      *if_group;
    vtss_port_ethernet_like_counters_t *elike;
    vtss_port_proprietary_counters_t   *prop;

    port = VTSS_CHIP_PORT(port_no);
    if (port == CHIP_PORT_SPI4) {
        VTSS_E("Counter structure does not support SPI4 counters");  
        return VTSS_RC_ERROR;
    }
            
    aport = b2_port2line(port);
    dport = vtss_state->port.dep_port[aport];
    c = &vtss_state->port.counters[port_no].counter.b2;
    
    VTSS_D("port_no: %u, port: %u, aport: %u, dport: %u", port_no, port, aport, dport);

    if (port < CHIP_PORT_10G_0) {
        /* 1G/Rx */
        B2_CNT(ASM, RX_IN_BYTES_CNT, port, &c->rx_in_bytes, clr);
        B2_CNT(ASM, RX_SYMBOL_ERR_CNT, port, &c->rx_symbol_carrier_err, clr);
        B2_CNT(ASM, RX_PAUSE_CNT, port, &c->rx_pause, clr);
        B2_CNT(ASM, RX_UNSUP_OPCODE_CNT, port, &c->rx_unsup_opcode, clr);
        B2_CNT(ASM, RX_OK_BYTES_CNT, port, &c->rx_ok_bytes, clr);
        B2_CNT(ASM, RX_BAD_BYTES_CNT, port, &c->rx_bad_bytes, clr);
        B2_CNT(ASM, RX_UC_CNT, port, &c->rx_unicast, clr);
        B2_CNT(ASM, RX_MC_CNT, port, &c->rx_multicast, clr);
        B2_CNT(ASM, RX_BC_CNT, port, &c->rx_broadcast, clr);
        B2_CNT(ASM, RX_CRC_ERR_CNT, port, &c->rx_crc_err, clr);
        B2_CNT(ASM, RX_UNDERSIZE_CNT, port, &c->rx_undersize, clr);
        B2_CNT(ASM, RX_FRAGMENTS_CNT, port, &c->rx_fragments, clr);
        B2_CNT(ASM, RX_IN_RANGE_LEN_ERR_CNT, port, &c->rx_in_range_length_err, clr);
        B2_CNT(ASM, RX_OUT_OF_RANGE_LEN_ERR_CNT, port, &c->rx_out_of_range_length_err, clr);
        B2_CNT(ASM, RX_OVERSIZE_CNT, port, &c->rx_oversize, clr);
        B2_CNT(ASM, RX_JABBERS_CNT, port, &c->rx_jabbers, clr);
        B2_CNT(ASM, RX_SIZE64_CNT, port, &c->rx_size64, clr);
        B2_CNT(ASM, RX_SIZE65TO127_CNT, port, &c->rx_size65_127, clr);
        B2_CNT(ASM, RX_SIZE128TO255_CNT, port, &c->rx_size128_255, clr);
        B2_CNT(ASM, RX_SIZE256TO511_CNT, port, &c->rx_size256_511, clr);
        B2_CNT(ASM, RX_SIZE512TO1023_CNT, port, &c->rx_size512_1023, clr);
        B2_CNT(ASM, RX_SIZE1024TO1518_CNT, port, &c->rx_size1024_1518, clr);
        B2_CNT(ASM, RX_SIZE1519TOMAX_CNT, port, &c->rx_size1519_max, clr);
    
        /* 1G/Tx */
        B2_CNT(ASM, TX_OUT_BYTES_CNT, port, &c->tx_out_bytes, clr);
        B2_CNT(ASM, TX_PAUSE_CNT, port, &c->tx_pause, clr);
        B2_CNT(ASM, TX_OK_BYTES_CNT, port, &c->tx_ok_bytes, clr);
        B2_CNT(ASM, TX_UC_CNT, port, &c->tx_unicast, clr);
        B2_CNT(ASM, TX_MC_CNT, port, &c->tx_multicast, clr);
        B2_CNT(ASM, TX_BC_CNT, port, &c->tx_broadcast, clr);
        B2_CNT(ASM, TX_SIZE64_CNT, port, &c->tx_size64, clr);
        B2_CNT(ASM, TX_SIZE65TO127_CNT, port, &c->tx_size65_127, clr);
        B2_CNT(ASM, TX_SIZE128TO255_CNT, port, &c->tx_size128_255, clr);
        B2_CNT(ASM, TX_SIZE256TO511_CNT, port, &c->tx_size256_511, clr);
        B2_CNT(ASM, TX_SIZE512TO1023_CNT, port, &c->tx_size512_1023, clr);
        B2_CNT(ASM, TX_SIZE1024TO1518_CNT, port, &c->tx_size1024_1518, clr);
        B2_CNT(ASM, TX_SIZE1519TOMAX_CNT, port, &c->tx_size1519_max, clr);
        B2_CNT(ASM, TX_MULTI_COLL_CNT, port, &c->tx_multi_coll, clr);
        B2_CNT(ASM, TX_LATE_COLL_CNT, port, &c->tx_late_coll, clr);
        B2_CNT(ASM, TX_XCOLL_CNT, port, &c->tx_xcoll, clr);
        B2_CNT(ASM, TX_DEFER_CNT, port, &c->tx_defer, clr);
        B2_CNT(ASM, TX_XDEFER_CNT, port, &c->tx_xdefer, clr);
        B2_CNT(ASM, TX_BACKOFF1_CNT, port, &c->tx_backoff1, clr);
        B2_CNT(ASM, TX_CSENSE_CNT, port, &c->tx_csense, clr);
    } else { 
        /* 10G/Rx */
        B2_CNT(DEV10G, RX_IN_BYTES_CNT, port, &c->rx_in_bytes, clr);
        B2_CNT(DEV10G, RX_SYMBOL_ERR_CNT, port, &c->rx_symbol_carrier_err, clr);
        B2_CNT(DEV10G, RX_PAUSE_CNT, port, &c->rx_pause, clr);
        B2_CNT(DEV10G, RX_UNSUP_OPCODE_CNT, port, &c->rx_unsup_opcode, clr);
        B2_CNT(DEV10G, RX_OK_BYTES_CNT, port, &c->rx_ok_bytes, clr);
        B2_CNT(DEV10G, RX_BAD_BYTES_CNT, port, &c->rx_bad_bytes, clr);
        B2_CNT(DEV10G, RX_UC_CNT, port, &c->rx_unicast, clr);
        B2_CNT(DEV10G, RX_MC_CNT, port, &c->rx_multicast, clr);
        B2_CNT(DEV10G, RX_BC_CNT, port, &c->rx_broadcast, clr);
        B2_CNT(DEV10G, RX_CRC_ERR_CNT, port, &c->rx_crc_err, clr);
        B2_CNT(DEV10G, RX_UNDERSIZE_CNT, port, &c->rx_undersize, clr);
        B2_CNT(DEV10G, RX_FRAGMENTS_CNT, port, &c->rx_fragments, clr);
        B2_CNT(DEV10G, RX_IN_RANGE_LEN_ERR_CNT, port, &c->rx_in_range_length_err, clr);
        B2_CNT(DEV10G, RX_OUT_OF_RANGE_LEN_ERR_CNT, port, &c->rx_out_of_range_length_err, clr);
        B2_CNT(DEV10G, RX_OVERSIZE_CNT, port, &c->rx_oversize, clr);
        B2_CNT(DEV10G, RX_JABBERS_CNT, port, &c->rx_jabbers, clr);
        B2_CNT(DEV10G, RX_SIZE64_CNT, port, &c->rx_size64, clr);
        B2_CNT(DEV10G, RX_SIZE65TO127_CNT, port, &c->rx_size65_127, clr);
        B2_CNT(DEV10G, RX_SIZE128TO255_CNT, port, &c->rx_size128_255, clr);
        B2_CNT(DEV10G, RX_SIZE256TO511_CNT, port, &c->rx_size256_511, clr);
        B2_CNT(DEV10G, RX_SIZE512TO1023_CNT, port, &c->rx_size512_1023, clr);
        B2_CNT(DEV10G, RX_SIZE1024TO1518_CNT, port, &c->rx_size1024_1518, clr);
        B2_CNT(DEV10G, RX_SIZE1519TOMAX_CNT, port, &c->rx_size1519_max, clr);
    
        /* 10G/Tx */
        B2_CNT(DEV10G, TX_OUT_BYTES_CNT, port, &c->tx_out_bytes, clr);
        B2_CNTX(DSM, TX_PAUSE_CNT, aport, &c->tx_pause, clr);
        B2_CNT(DEV10G, TX_OK_BYTES_CNT, port, &c->tx_ok_bytes, clr);
        B2_CNT(DEV10G, TX_UC_CNT, port, &c->tx_unicast, clr);
        B2_CNT(DEV10G, TX_MC_CNT, port, &c->tx_multicast, clr);
        B2_CNT(DEV10G, TX_BC_CNT, port, &c->tx_broadcast, clr);
        B2_CNT(DEV10G, TX_SIZE64_CNT, port, &c->tx_size64, clr);
        B2_CNT(DEV10G, TX_SIZE65TO127_CNT, port, &c->tx_size65_127, clr);
        B2_CNT(DEV10G, TX_SIZE128TO255_CNT, port, &c->tx_size128_255, clr);
        B2_CNT(DEV10G, TX_SIZE256TO511_CNT, port, &c->tx_size256_511, clr);
        B2_CNT(DEV10G, TX_SIZE512TO1023_CNT, port, &c->tx_size512_1023, clr);
        B2_CNT(DEV10G, TX_SIZE1024TO1518_CNT, port, &c->tx_size1024_1518, clr);
        B2_CNT(DEV10G, TX_SIZE1519TOMAX_CNT, port, &c->tx_size1519_max, clr);
    }

    /* Filtered frames is counter 0 */
    B2_CNTX(ANA_AC, PORT_STAT_LSB_CNT_0, aport, &c->rx_filter_drops, clr);
    
    /* Policed frames are counter 4 and 6 */
    B2_CNTX(ANA_AC, PORT_STAT_LSB_CNT_4, aport, &c->rx_policer_drops[0], clr);
    B2_CNTX(ANA_AC, PORT_STAT_LSB_CNT_6, aport, &c->rx_policer_drops[1], clr);
    
    /* QSS Tx drop counters based on line port */
    qid = (192 + aport);
    B2_CNTX(QSS, Q_BRM_DROP_FCNT, qid, &c->tx_queue_drops, clr);
    B2_CNTX(QSS, Q_RED_DROP_FCNT, qid, &c->tx_red_drops, clr);
    B2_CNTX(QSS, Q_ERR_DROP_FCNT, qid, &c->tx_error_drops, clr);

    /*  100fx counter workaround */
    /*  tx_ok_bytes, tx_out_bytes and the histogram counters are broken for 100fx */
    /*  tx_ok_bytes and tx_out_bytes can be reconstructed from the queue counters */
    B2_CNTX(QSS, Q_FCNT, qid, &c->tx_queue, clr);
    B2_CNTX(QSS, Q_BCNT_L, qid, &c->tx_queue_bytes, clr);

    /* QSS Rx counters based on departure port */
    for (i = 0; i < B2_PRIOS; i++) {
        qid = b2_qid(vtss_state, dport, i);
        B2_CNTX(QSS, Q_FCNT, qid, &c->rx_class[i], clr);
        B2_CNTX(QSS, Q_BRM_DROP_FCNT, qid, &c->rx_queue_drops[i], clr);
        B2_CNTX(QSS, Q_RED_DROP_FCNT, qid, &c->rx_red_drops[i], clr);
        B2_CNTX(QSS, Q_ERR_DROP_FCNT, qid, &c->rx_error_drops[i], clr);
    }

    if (counters == NULL)
        return VTSS_RC_OK;

    /* Calculate API counters based on chip counters */
    rmon = &counters->rmon;
    if_group = &counters->if_group;
    elike = &counters->ethernet_like;
    prop = &counters->prop;

    /* Proprietary counters */
    prop->rx_filter_drops = c->rx_filter_drops.value;
    prop->rx_policer_drops = (c->rx_policer_drops[0].value + c->rx_policer_drops[1].value);
    for (i = 0; i < B2_PRIOS; i++) {
        prop->rx_prio[i] = c->rx_class[i].value;
        prop->rx_prio_drops[i] = (c->rx_queue_drops[i].value +
                                  c->rx_red_drops[i].value +
                                  c->rx_error_drops[i].value);
    }
    
    /* RMON Rx counters */
    rmon->rx_etherStatsDropEvents = (prop->rx_filter_drops + prop->rx_policer_drops);
    for (i = 0; i < B2_PRIOS; i++) 
        rmon->rx_etherStatsDropEvents += prop->rx_prio_drops[i];
    rmon->rx_etherStatsOctets = (c->rx_ok_bytes.value + c->rx_bad_bytes.value); 
    rx_errors = (c->rx_crc_err.value +  c->rx_undersize.value + c->rx_oversize.value + 
                 c->rx_out_of_range_length_err.value + c->rx_symbol_carrier_err.value + 
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
    rmon->tx_etherStatsDropEvents = (c->tx_queue_drops.value +
                                     c->tx_red_drops.value +
                                     c->tx_error_drops.value);
    rmon->tx_etherStatsOctets = (c->tx_queue_bytes.value + 64*c->tx_pause.value);
    rmon->tx_etherStatsPkts = (c->tx_unicast.value + c->tx_multicast.value +
                               c->tx_broadcast.value + c->tx_late_coll.value);
    rmon->tx_etherStatsBroadcastPkts = c->tx_broadcast.value;
    rmon->tx_etherStatsMulticastPkts = c->tx_multicast.value;
    rmon->tx_etherStatsCollisions = (c->tx_multi_coll.value + c->tx_backoff1.value + 
                                     c->tx_late_coll.value + c->tx_xcoll.value);
    rmon->tx_etherStatsPkts64Octets = c->tx_size64.value;
    rmon->tx_etherStatsPkts65to127Octets = c->tx_size65_127.value;
    rmon->tx_etherStatsPkts128to255Octets = c->tx_size128_255.value;
    rmon->tx_etherStatsPkts256to511Octets = c->tx_size256_511.value;
    rmon->tx_etherStatsPkts512to1023Octets = c->tx_size512_1023.value;
    rmon->tx_etherStatsPkts1024to1518Octets = c->tx_size1024_1518.value;
    rmon->tx_etherStatsPkts1519toMaxOctets = c->tx_size1519_max.value;
    
    /* Interfaces Group Rx counters */
    if_group->ifInOctets = c->rx_in_bytes.value;
    if_group->ifInUcastPkts = c->rx_unicast.value;
    if_group->ifInMulticastPkts = c->rx_multicast.value;
    if_group->ifInBroadcastPkts = c->rx_broadcast.value;
    if_group->ifInNUcastPkts = (c->rx_multicast.value + c->rx_broadcast.value);
    if_group->ifInDiscards = rmon->rx_etherStatsDropEvents;
    if_group->ifInErrors = (rx_errors + c->rx_in_range_length_err.value);
    
    /* Interfaces Group Tx counters */
    if_group->ifOutOctets = (rmon->tx_etherStatsOctets + 
                             8*(c->tx_queue.value + c->tx_pause.value));
    if_group->ifOutUcastPkts = c->tx_unicast.value;
    if_group->ifOutMulticastPkts = c->tx_multicast.value;
    if_group->ifOutBroadcastPkts = c->tx_broadcast.value;
    if_group->ifOutNUcastPkts = (c->tx_multicast.value + c->tx_broadcast.value);
    if_group->ifOutDiscards = rmon->tx_etherStatsDropEvents;
    if_group->ifOutErrors = (c->tx_late_coll.value + c->tx_csense.value + c->tx_xcoll.value);

    /* Ethernet-like Rx counters */
    elike->dot3StatsAlignmentErrors = 0; /* Not supported */
    elike->dot3StatsFCSErrors = c->rx_crc_err.value;
    elike->dot3StatsFrameTooLongs = c->rx_oversize.value;
    elike->dot3StatsSymbolErrors = c->rx_symbol_carrier_err.value;
    elike->dot3ControlInUnknownOpcodes = c->rx_unsup_opcode.value;
    elike->dot3InPauseFrames = c->rx_pause.value;
    
    /* Ethernet-like Tx counters */
    elike->dot3StatsSingleCollisionFrames = c->tx_backoff1.value;
    elike->dot3StatsMultipleCollisionFrames = c->tx_multi_coll.value;
    elike->dot3StatsDeferredTransmissions = c->tx_defer.value;
    elike->dot3StatsLateCollisions = c->tx_late_coll.value;
    elike->dot3StatsExcessiveCollisions = c->tx_xcoll.value;
    elike->dot3StatsCarrierSenseErrors = c->tx_csense.value;
    elike->dot3OutPauseFrames = c->tx_pause.value;

    return VTSS_RC_OK;
}

static vtss_rc b2_port_counters_update(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return b2_port_counters_cmd(vtss_state, port_no, NULL, 0);
}

static vtss_rc b2_port_counters_clear(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    return b2_port_counters_cmd(vtss_state, port_no, NULL, 1);
}

static vtss_rc b2_port_counters_get(vtss_state_t *vtss_state,
                                    const vtss_port_no_t port_no,
                                    vtss_port_counters_t *const counters)
{
    memset(counters, 0, sizeof(*counters)); 
    return b2_port_counters_cmd(vtss_state, port_no, counters, 0);
}

/* ================================================================= *
 *  Port Filters
 * ================================================================= */
static vtss_rc b2_port_filter_set(vtss_state_t *vtss_state,
                                  const vtss_port_no_t port_no, 
                                  const vtss_port_filter_t * const filter)
{
    u32 tags, value, mask=0;
    int line_port;


    if (filter->max_tags == VTSS_TAG_ONE) {
        tags = 1;
    } else if (filter->max_tags == VTSS_TAG_TWO) {
        tags = 2;
    } else {
        tags = 0;
    }

    line_port = b2_port2line(VTSS_CHIP_PORT(port_no));

    VTSS_N("port_no: %u, mac_ctrl_enable:%d, mac_zero_enable:%d dmac_bc_enable:%d smac_mc_enable:%d \
             tags:%d stag_enable:%d, ctag_enable:%d, prio_tag_enable:%d, untag_enable:%d",\
            filter->mac_ctrl_enable,filter->mac_zero_enable,filter->dmac_bc_enable,filter->smac_mc_enable,\
            tags,filter->stag_enable,filter->ctag_enable,filter->prio_tag_enable,filter->untag_enable);

    /* Only write to the specific fields in the FILTER_CTRL register    */
    mask = 1 << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_MAC_CTRL_ENA |
           1 << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_NULL_MAC_ENA |
           1 << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_BC_ENA | 
           1 << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_SMAC_MC_ENA;

    value = (1^filter->mac_ctrl_enable) << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_MAC_CTRL_ENA | 
            (1^filter->mac_zero_enable) << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_NULL_MAC_ENA |
            (1^filter->dmac_bc_enable)  << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_BC_ENA |
            (1^filter->smac_mc_enable)  << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_SMAC_MC_ENA;

    B2_WRXM(ANA_CL, FILTER_CTRL, line_port, value, mask);       

    /* Only write to the specific fields in the TAG_AND_LBL_CFG register    */
    mask = 3 << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_MAX_VLAN_TAGS |
           1 << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_STAGGED_DIS |
           1 << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_TAGGED_DIS | 
           1 << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_PRIO_TAGGED_DIS |
           1 << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_UNTAGGED_DIS;

    value = tags                        << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_MAX_VLAN_TAGS |
           (1^filter->stag_enable)     << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_STAGGED_DIS |
           (1^filter->ctag_enable)     << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_TAGGED_DIS | 
           (1^filter->prio_tag_enable) << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_PRIO_TAGGED_DIS |
           (1^filter->untag_enable)    << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_UNTAGGED_DIS;
    
    B2_WRXM(ANA_CL, TAG_AND_LBL_CFG, line_port, value, mask);
          
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Quality of Service
 * ================================================================= */

/* Calculate rate register value */
static u32 b2_rate(vtss_bitrate_t rate)
{
    u64 value;
    
    value = rate;
    value = (value*1000/1589);
    if (value > 0x7fffff)
        value = 0x7fffff;
    return value;
}

/* Minimum weight is 2*MTU */
static u32 b2_min_weight(vtss_state_t *vtss_state, u32 weight)
{
    u32 min = (2 * vtss_state->init_conf.qs.mtu);
    
    return (weight < min ? min : weight);
}

/* Minimum rate corresponding to minimum weight */
static vtss_bitrate_t b2_min_rate(vtss_state_t *vtss_state, vtss_bitrate_t rate)
{
    u64 min;
 
    min = b2_min_weight(vtss_state, 0);
    min = min*vtss_state->init_conf.sch_max_rate/0x1fffff;
    if (rate == VTSS_BITRATE_DISABLED || rate < min)
        rate = min;

    return rate;
}

/* Calculate port scheduler weight */
static vtss_rc b2_weight(vtss_state_t *vtss_state, vtss_bitrate_t rate, u32 *weight)
{
    u32 max;
    u64 w;

    max = vtss_state->init_conf.sch_max_rate;
    if (rate == VTSS_BITRATE_DISABLED)
        w = 0;
    else if (rate > max) {
        VTSS_E("sch_max_rate exceeded");
        return VTSS_RC_ERROR;
    } else {
        w = rate;
        w = (0x1fffff*w)/max;
    }
    *weight = b2_min_weight(vtss_state, w);
    return VTSS_RC_OK;
}

/* Check that the sum of scheduler rates do not exceed the XAUI/SPI4 rate */
static vtss_rc b2_scheduler_check(vtss_state_t *vtss_state)
{
    int               chip_port;
    vtss_host_mode_t  host_mode;
    vtss_spi4_conf_t *spi4;
    vtss_bitrate_t    rate, spi4_rate = 0, x0_rate = 0, x1_rate = 0, spi4_max, x0_max, x1_max;
    vtss_port_no_t    port_no;
    vtss_lport_no_t   lport_no;
    uint              count;
    
    host_mode = vtss_state->init_conf.host_mode;

    /* SPI4 maximum rate */
    spi4 = &vtss_state->port.spi4;
    spi4_max = (((int)spi4->ob.clock*2 + 8)*1000000); /* 8/10/12/14/16 Gbps */
    rate = spi4->qos.shaper.rate;
    if (rate < spi4_max)
        spi4_max = rate;

    /* XAUI maximum rate */
    x0_max = 10000000; /* 10G */

    x0_rate = vtss_state->port.xaui[0].qos.shaper.rate;
    x1_rate = vtss_state->port.xaui[1].qos.shaper.rate;
    if (host_mode == VTSS_HOST_MODE_0 || host_mode == VTSS_HOST_MODE_10) /* XAUI0 */
        rate = x0_rate;
    else if (host_mode == VTSS_HOST_MODE_1 || host_mode == VTSS_HOST_MODE_11) /* XAUI1 */
        rate = x1_rate;
    else { /* BOTH, take the largest value*/
        if (x1_rate > x0_rate)
            rate = x1_rate;
        else
            rate = x0_rate;
    }

    if (host_mode < VTSS_HOST_MODE_4 && rate < x0_max) /* XAUI host port shaper used */
        x0_max = rate;
    x1_max = x0_max;
    
    x0_rate = 0;
    x1_rate = 0;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if ((chip_port = VTSS_CHIP_PORT(port_no)) < 0)
            continue;

        if (host_mode > VTSS_HOST_MODE_6) {
            /* Check if XAUI line port shapers are used */
            rate = vtss_state->qos.port_conf[port_no].shaper_port.rate;
            if (chip_port == CHIP_PORT_10G_0 && rate < x0_max)
                x0_max = rate;
            if (chip_port == CHIP_PORT_10G_1 && rate < x1_max)
                x1_max = rate;
            continue;
        }

        rate = b2_min_rate(vtss_state, vtss_state->qos.port_conf[port_no].scheduler.rate);
        lport_no = vtss_state->port.map[port_no].lport_no;
        if (host_mode > VTSS_HOST_MODE_3)
            spi4_rate += rate;
        else if (host_mode == VTSS_HOST_MODE_0 || (host_mode == VTSS_HOST_MODE_3 && lport_no < 12))
            x0_rate += rate;
        else
            x1_rate += rate;
    }
    
    if (host_mode > VTSS_HOST_MODE_6) {
        /* Aggregator/converter mode */
        count = (host_mode == VTSS_HOST_MODE_9 ? 16 : 48);
        for (lport_no = 0; lport_no < count; lport_no++) {
            /* Rx rate towards SPI4 */
            rate = b2_min_rate(vtss_state, vtss_state->qos.lport[lport_no].scheduler.rx_rate);
            spi4_rate += rate;
            
            /* Tx rate towards XAUI */
            rate = b2_min_rate(vtss_state, vtss_state->qos.lport[lport_no].scheduler.tx_rate);
            if (lport_no < (count/2))
                x0_rate += rate;
            else
                x1_rate += rate;
        }
    }
//    VTSS_D("spi4_rate: %u, x0_rate: %u, x1_rate: %u", spi4_rate, x0_rate, x1_rate);
//    VTSS_D("spi4_max: %u, x0_max: %u, x1_max: %u", spi4_max, x0_max, x1_max);

    /* Now, check the SPI4/XAUI rates */
    if (spi4_rate > spi4_max) {
        VTSS_E("SPI4 rate exceeded: %u, max: %u", spi4_rate, spi4_max);
        return VTSS_RC_ERROR;
    }
    if (x0_rate > x0_max) {
        VTSS_E("XAUI0 rate exceeded: %u, max: %u", x0_rate, x0_max);
        return VTSS_RC_ERROR;
    }
    if (x1_rate > x1_max) {
        VTSS_E("XAUI1 rate exceeded: %u, max: %u", x1_rate, x1_max);
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

/* Setup shaper */
static vtss_rc b2_shaper_setup(vtss_state_t *vtss_state,
                               u32 port, const vtss_shaper_t *shaper)
{
    u32 value;
  
    /* Threshold, must be setup first */
    value = (shaper->level/1024);
    if (value > 0xff)
        value = 0xff;
    if (value < 2)
        value = 2;
    B2_WRX(SCH, SHAPER_THRESHOLD, port, 
           value << VTSS_OFF_SCH_SHAPER_THRESHOLD_SHAPER_THRESHOLD);

    /* Rate */
    value = b2_rate(shaper->rate);
    B2_WRX(SCH, SHAPER_RATE, port, value << VTSS_OFF_SCH_SHAPER_RATE_SHAPER_RATE);

    /* GAP per frame */
    B2_WRX(SCH, RATE_CONVERSION, port, 20 << VTSS_OFF_SCH_RATE_CONVERSION_RATE_CONVERSION);

    /* Enable/disable */
    return b2_wrf(vtss_state, VTSS_TGT_SCH, VTSS_ADDR_SCH_SPR_PORT_ENABLE, port, 1, 0, 
                  shaper->rate == VTSS_BITRATE_DISABLED ? 0 : 1);
}

/* Setup lport shaper */
static vtss_rc b2_shaper_lport_setup(vtss_state_t *vtss_state,
                                     uint lport, const vtss_shaper_t *shaper)
{
    u32 value, value2;
    
    /* Threshold, must be setup first */
    value = (shaper->level/1024);
    if (value > 0xff)
        value = 0xff;
    if (value < 2)
        value = 2;
    B2_WRX(SCH, PP_SHAPER_THRESHOLD, lport, 
           value << VTSS_OFF_SCH_PP_SHAPER_THRESHOLD_PP_SHAPER_THRESHOLD); 
    
    /* Rate */
    value = b2_rate(shaper->rate);

    B2_WRX(SCH, PP_SHAPER_RATE, lport, value);
    
    /* GAP per frame */
    B2_WRX(SCH, RATE_CONVERSION, lport, 20 << VTSS_OFF_SCH_RATE_CONVERSION_RATE_CONVERSION);
    
    /* Enable the lport shaper (1 bit for each lport)  */
    if (lport < 32 ) {
        VTSS_RC(b2_wrf(vtss_state, VTSS_TGT_SCH, VTSS_ADDR_SCH_PP_SPR_PORT_ENABLE_LS,
                       lport, 1, 0, shaper->rate == VTSS_BITRATE_DISABLED ? 0 : 1));
    } else {
        VTSS_RC(b2_wrf(vtss_state, VTSS_TGT_SCH, VTSS_ADDR_SCH_PP_SPR_PORT_ENABLE_MS,
                       lport - 32, 1, 0, shaper->rate == VTSS_BITRATE_DISABLED ? 0 : 1));
    }

    /* Read lport shaper enable/disable status  */
    B2_RD(SCH, PP_SPR_PORT_ENABLE_LS, 0, &value);
    B2_RD(SCH, PP_SPR_PORT_ENABLE_MS, 0, &value2);

    /* Enable/disable the global shaper accordingly  */
    B2_WR(SCH, PP_SPR_ENABLE, 0, (value|value2) ? 1 : 0);

    return VTSS_RC_OK;
}

/* Setup port/queue policer */
static vtss_rc b2_policer_setup(vtss_state_t *vtss_state,
                                int offset, const vtss_policer_t *policer, BOOL queue)
{
    u32 cfg, level, rate;

    /* Unicast/multicast/broadcast flags */
    cfg = (((policer->multicast ? 1 : 0) << 1) |
           ((policer->broadcast ? 1 : 0) << 2) |
           ((policer->unicast ? 1 : 0) << 3));

    /* Burst level */
    level = (policer->level / 1024); /* Unit is 1024 bytes. */
    if (level > 0xff)
        level = 0xff;

    /* Rate */
    rate = b2_rate(policer->rate);

    if (queue) {
        /* Queue policer */
        B2_WRX(ANA_AC, POL_PRIO_CFG, offset, cfg);
        B2_WRX(ANA_AC, POL_PRIO_THRES_CFG_0, offset, level);
        B2_WRX(ANA_AC, POL_PRIO_RATE_CFG, offset, rate);
    } else {
        /* Port policer */
        B2_WRX(ANA_AC, POL_PORT_CFG, offset, cfg);
        B2_WRX(ANA_AC, POL_PORT_THRES_CFG_0, offset, level);
        B2_WRX(ANA_AC, POL_PORT_RATE_CFG, offset, rate);
    }
    return VTSS_RC_OK;
}

/* Chip priority */
#define B2_PRIO(qos) (qos.prio - VTSS_PRIO_START)

/* Initialize QoS for chip port */
static vtss_rc b2_port_qos_set(vtss_state_t *vtss_state,
                               int chip_port, const vtss_qos_port_conf_t *qos)
{
    int   line_port, queue, filt_no, indx;
    u32   mask, value, max_pct, pct, weight;
    const u8 *dmac_p;

    VTSS_D("Enter w/port %u",chip_port);

    line_port = b2_port2line(chip_port);

    mask = ((1 << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_TAG_SEL) |
            (1 << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_AWARE_ENA) | 
            (1 << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_CTAG_STOP_ENA) |
            (1 << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_CTAG_CFI_STOP_ENA));
           
    value = qos->tag_inner     << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_TAG_SEL | 
            qos->vlan_aware    << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_VLAN_AWARE_ENA | 
            qos->ctag_cfi_stop << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_CTAG_CFI_STOP_ENA |
            qos->ctag_stop     << VTSS_OFF_ANA_CL_TAG_AND_LBL_CFG_CTAG_STOP_ENA;    

    /* Only write to the specific fields in the TAG_AND_LBL_CFG register    */    
    B2_WRXM(ANA_CL, TAG_AND_LBL_CFG, line_port, value, mask);    

    /* Set the Primary endpoint order    */
    for (indx = 0; indx < (int)VTSS_QOS_ORDER_COUNT; indx++) {
        B2_WRXY(ANA_CL, ENDPT_REMAP_PRIO_CFG, line_port, indx,(int)qos->order[indx]);            
    }
    
    /* Set DMAC/VID/EType filter */
    for (indx = 0; indx < 2; indx++) {
        dmac_p = qos->dmac_vid_etype.filter[indx].dmac.value;

        /* DMAC Pattern MSB bits 47:32  */
        B2_WRXY(ANA_CL, FILTER_LOCAL_CFG_0, line_port, indx, dmac_p[0] << 8 |
                                                             dmac_p[1] << 0); 

        /* DMAC Pattern LSB bits 31:0  */
        B2_WRXY(ANA_CL, FILTER_LOCAL_CFG_1, line_port, indx, dmac_p[2] << 24 | 
                                                             dmac_p[3] << 16 | 
                                                             dmac_p[4] << 8  | 
                                                             dmac_p[5] << 0);            

        dmac_p = qos->dmac_vid_etype.filter[indx].dmac.mask;
        /* DMAC Mask MSB bits 47:32  */
        B2_WRXY(ANA_CL, FILTER_LOCAL_CFG_2, line_port, indx, dmac_p[0] << 8 |
                                                             dmac_p[1] << 0);
        /* DMAC Mask LSB bits 31:0  */
        B2_WRXY(ANA_CL, FILTER_LOCAL_CFG_3, line_port, indx, dmac_p[2] << 24 | 
                                                             dmac_p[3] << 16 | 
                                                             dmac_p[4] << 8  | 
                                                             dmac_p[5] << 0);            
        /* Ethernet type value */
        B2_WRXY(ANA_CL, FILTER_LOCAL_ETYPE_CFG_0, line_port, indx, qos->dmac_vid_etype.filter[indx].etype.value);
        /* Ethernet type mask */
        B2_WRXY(ANA_CL, FILTER_LOCAL_ETYPE_CFG_1, line_port, indx, qos->dmac_vid_etype.filter[indx].etype.mask);

        /* VLAN ID value */
        B2_WRXY(ANA_CL, FILTER_VID_CFG_0, line_port, indx, qos->dmac_vid_etype.filter[indx].vid.value);
        /* VLAN ID mask  */
        B2_WRXY(ANA_CL, FILTER_VID_CFG_1, line_port, indx, qos->dmac_vid_etype.filter[indx].vid.mask);        
    }
    /* Enable/Disable filter instances (2) */        
    mask = 3 << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_VID_ENA |
           3 << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_ETYPE_ENA |
           3 << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_DMAC_ENA;
    value = qos->dmac_vid_etype.filter[0].enable << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_VID_ENA |
            qos->dmac_vid_etype.filter[1].enable << (VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_VID_ENA+1) |
            qos->dmac_vid_etype.filter[0].enable << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_ETYPE_ENA |
            qos->dmac_vid_etype.filter[1].enable << (VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_ETYPE_ENA+1) |
            qos->dmac_vid_etype.filter[0].enable << VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_DMAC_ENA |
            qos->dmac_vid_etype.filter[1].enable << (VTSS_OFF_ANA_CL_FILTER_CTRL_FILTER_DMAC_ENA+1);        
    B2_WRXM(ANA_CL, FILTER_CTRL, line_port, value, mask);       
    
    /* Enable/disable alternate ordering.  Disable means that non-matching frames are dropped  */
    mask = 1 << VTSS_OFF_ANA_CL_FILTER_CTRL_ALT_ORDER_ENA;
    value = qos->dmac_vid_etype.order_enable << VTSS_OFF_ANA_CL_FILTER_CTRL_ALT_ORDER_ENA;
    B2_WRXM(ANA_CL, FILTER_CTRL, line_port, value, mask);       

    /* Set the alternate endpoint order    */
    for (indx = 0; indx < (int)VTSS_QOS_ORDER_COUNT; indx++) {
        B2_WRXY(ANA_CL, ENDPT_REMAP_ALT_CFG, line_port, indx, (int)qos->dmac_vid_etype.order[indx]);
    }
    
    /* Default endpoint, RED and Prio   */
    B2_WRX(ANA_CL, ENDPT_DEFAULT_CFG, line_port, 
           qos->default_qos.red << VTSS_OFF_ANA_CL_ENDPT_DEFAULT_CFG_QOS_DEFAULT_RED |
           B2_PRIO(qos->default_qos) << VTSS_OFF_ANA_CL_ENDPT_DEFAULT_CFG_QOS_DEFAULT_PRIO);

    /* Endpoint for L2 control frames, RED and Prio   */
    B2_WRX(ANA_CL, ENDPT_L2_CTRL_CFG, line_port, 
           qos->l2_control_qos.red << VTSS_OFF_ANA_CL_ENDPT_L2_CTRL_CFG_QOS_L2_CTRL_RED |
           B2_PRIO(qos->l2_control_qos) << VTSS_OFF_ANA_CL_ENDPT_L2_CTRL_CFG_QOS_L2_CTRL_PRIO);

    /* Endpoint for L3 control frames, RED and Prio   */
    B2_WRX(ANA_CL, ENDPT_L3_CTRL_CFG, line_port, 
           qos->l3_control_qos.red << VTSS_OFF_ANA_CL_ENDPT_L3_CTRL_CFG_QOS_L3_CTRL_RED |
           B2_PRIO(qos->l3_control_qos) << VTSS_OFF_ANA_CL_ENDPT_L3_CTRL_CFG_QOS_L3_CTRL_PRIO);

    /* Endpoint for CFI set, RED and Prio             */
    B2_WRX(ANA_CL, ENDPT_CFI_CFG, line_port, 
           qos->cfi_qos.red << VTSS_OFF_ANA_CL_ENDPT_CFI_CFG_QOS_VLAN_CFI_RED |
           B2_PRIO(qos->cfi_qos) << VTSS_OFF_ANA_CL_ENDPT_CFI_CFG_QOS_VLAN_CFI_PRIO);

    /* Endpoint for IPv4 and ARP, RED and Prio        */
    B2_WRX(ANA_CL, ENDPT_IP4_CFG, line_port, 
           qos->ipv4_arp_qos.red << VTSS_OFF_ANA_CL_ENDPT_IP4_CFG_QOS_IP4_RED |
           B2_PRIO(qos->ipv4_arp_qos) << VTSS_OFF_ANA_CL_ENDPT_IP4_CFG_QOS_IP4_PRIO);

    /* Endpoint for IPv6, RED and Prio                */
    B2_WRX(ANA_CL, ENDPT_IP6_CFG, line_port, 
           qos->ipv6_qos.red << VTSS_OFF_ANA_CL_ENDPT_IP6_CFG_QOS_IP6_RED |
           B2_PRIO(qos->ipv6_qos) << VTSS_OFF_ANA_CL_ENDPT_IP6_CFG_QOS_IP6_PRIO);

    /* Endpoint for EXP MPLS, RED and Prio            */
    for (indx = 0; indx < 8; indx++) {
        B2_WRXY(ANA_CL, ENDPT_MPLS_CFG, line_port, indx, 
                qos->mpls_exp_qos[indx].red << VTSS_OFF_ANA_CL_ENDPT_MPLS_CFG_QOS_MPLS_RED |
                B2_PRIO(qos->mpls_exp_qos[indx]) << VTSS_OFF_ANA_CL_ENDPT_MPLS_CFG_QOS_MPLS_PRIO);    
    }

    /* Endpoint for VLAN UserPrio, RED and Prio       */
    for (indx = 0; indx < 8; indx++) {
        B2_WRXY(ANA_CL, ENDPT_UPRIO_CFG, line_port, indx, 
                qos->vlan_tag_qos[indx].red << VTSS_OFF_ANA_CL_ENDPT_UPRIO_CFG_QOS_UPRIO_RED |
                B2_PRIO(qos->vlan_tag_qos[indx]) << VTSS_OFF_ANA_CL_ENDPT_UPRIO_CFG_QOS_UPRIO_PRIO);    
    }    
    /* Endpoint for LLC SNAP frame, RED and Prio      */
    B2_WRX(ANA_CL, ENDPT_SNAP_CFG, line_port, 
           qos->llc_qos.red << VTSS_OFF_ANA_CL_ENDPT_SNAP_CFG_QOS_SNAP_RED |
           B2_PRIO(qos->llc_qos) << VTSS_OFF_ANA_CL_ENDPT_SNAP_CFG_QOS_SNAP_PRIO);

    /* Endpoint for IP protocol (0-255), RED and Prio      */
    B2_WRX(ANA_CL, ENDPT_IP_PROTO_CFG, line_port, 
           qos->ip_proto.proto << VTSS_OFF_ANA_CL_ENDPT_IP_PROTO_CFG_QOS_IP_PROTO_VAL |
           qos->ip_proto.qos.red << VTSS_OFF_ANA_CL_ENDPT_IP_PROTO_CFG_QOS_IP_PROTO_RED |
           B2_PRIO(qos->ip_proto.qos) << VTSS_OFF_ANA_CL_ENDPT_IP_PROTO_CFG_QOS_IP_PROTO_PRIO);

    /* Endpoint for Ethernet Type, RED and Prio      */
    B2_WRX(ANA_CL, ENDPT_ETYPE_CFG, line_port, 
           qos->etype.etype << VTSS_OFF_ANA_CL_ENDPT_ETYPE_CFG_QOS_ETYPE_VAL |
           qos->etype.qos.red << VTSS_OFF_ANA_CL_ENDPT_ETYPE_CFG_QOS_ETYPE_RED |
           B2_PRIO(qos->etype.qos) << VTSS_OFF_ANA_CL_ENDPT_ETYPE_CFG_QOS_ETYPE_PRIO);

    /* Endpoint for VLAN ID (2), RED and Prio      */
    for (indx = 0; indx < 2; indx++) {
        B2_WRXY(ANA_CL, ENDPT_VID_CFG, line_port, indx, 
                qos->vlan[indx].vid << VTSS_OFF_ANA_CL_ENDPT_VID_CFG_QOS_VID_VAL |
                qos->vlan[indx].qos.red << VTSS_OFF_ANA_CL_ENDPT_VID_CFG_QOS_VID_RED |
                B2_PRIO(qos->vlan[indx].qos) << VTSS_OFF_ANA_CL_ENDPT_VID_CFG_QOS_VID_PRIO);
    }  

    /* Port index 0-23 to DSCP tables 0-1  */
    VTSS_RC(b2_wrf(vtss_state, VTSS_TGT_ANA_CL, VTSS_ADDR_ANA_CL_DSCP_REMAP_IDX_CFG, line_port, 1, 0, qos->dscp_table_no));

    /* Local TCP/UDP port settings */
    for(indx=0; indx<2; indx++) {
        B2_WRXY(ANA_CL, ENDPT_TCPUDP_CFG_1, line_port, indx, 
                qos->udp_tcp.local.pair[indx].port << VTSS_OFF_ANA_CL_ENDPT_TCPUDP_CFG_1_QOS_LOCAL_TCPUDP_PORT_VAL |
                qos->udp_tcp.local.pair[indx].qos.red << VTSS_OFF_ANA_CL_ENDPT_TCPUDP_CFG_1_QOS_LOCAL_TCPUDP_PORT_RED |
                B2_PRIO(qos->udp_tcp.local.pair[indx].qos) << VTSS_OFF_ANA_CL_ENDPT_TCPUDP_CFG_1_QOS_LOCAL_TCPUDP_PORT_PRIO);
    }

    /* Global TCP/UDP enable settings and local TCP/UDP range settings */
    value = 0;
    mask = 0;
    for(indx=0; indx < VTSS_UDP_TCP_PAIR_COUNT; indx++) {
        value = value | (qos->udp_tcp.global_enable[indx][0] << mask) | (qos->udp_tcp.global_enable[indx][1] << (mask+1));
        mask = mask+2;
    }   

    B2_WRX(ANA_CL, ENDPT_TCPUDP_CFG_0, line_port, value << VTSS_OFF_ANA_CL_ENDPT_TCPUDP_CFG_0_QOS_GLOBAL_TCPUDP_PORT_ENA |
                               qos->udp_tcp.local.range << VTSS_OFF_ANA_CL_ENDPT_TCPUDP_CFG_0_QOS_LOCAL_TCPUDP_RNG_ENA);

    /* Global filter enable/disable for each port */
    value = 0;
    for (indx = 0; indx < VTSS_CUSTOM_FILTER_COUNT; indx++) {
        value = value | (qos->custom_filter.global_enable[indx] << indx);
    }
    B2_WRX(ANA_CL, FILTER_GLOBAL_CUSTOM_CFG, line_port, value);

    /* Local filter setup for each port. If matched, drop frame or forward to endpoint */
    mask = 1 << VTSS_OFF_ANA_CL_MISC_PORT_CFG_FILTER_LOCAL_CUSTOM_ENA;
    value = 1^qos->custom_filter.local.forward << VTSS_OFF_ANA_CL_MISC_PORT_CFG_FILTER_LOCAL_CUSTOM_ENA;
    B2_WRXM(ANA_CL, MISC_PORT_CFG, line_port, value, mask);       
     
    for (filt_no = 0; filt_no < 7; filt_no++) {
        /*  Position the filter in L2, L3 or L4 Header and set the offset   */
        B2_WRXY(ANA_CL, FILTER_LOCAL_CUSTOM_CFG_0, line_port, filt_no, 
                (int)qos->custom_filter.local.filter[filt_no].header << VTSS_OFF_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0_FILTER_LOCAL_CUSTOM_TYPE | 
                (qos->custom_filter.local.filter[filt_no].offset/2) << VTSS_OFF_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_0_FILTER_LOCAL_CUSTOM_POS);
        
        /* Set the 2 byte mask and pattern */
        B2_WRXY(ANA_CL, FILTER_LOCAL_CUSTOM_CFG_1, line_port, filt_no, 
                qos->custom_filter.local.filter[filt_no].mask << VTSS_OFF_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1_FILTER_LOCAL_CUSTOM_MASK | 
                qos->custom_filter.local.filter[filt_no].val  << VTSS_OFF_ANA_CL_FILTER_LOCAL_CUSTOM_CFG_1_FILTER_LOCAL_CUSTOM_PATTERN);        
    }

    /*  Endpoint for local custom filter               */
    B2_WRX(ANA_CL, ENDPT_LOCAL_CUSTOM_FILTER_CFG, line_port, 
           qos->custom_filter.local.qos.red  << VTSS_OFF_ANA_CL_ENDPT_LOCAL_CUSTOM_FILTER_CFG_QOS_LOCAL_CUSTOM_FILTER_RED | 
           B2_PRIO(qos->custom_filter.local.qos) << VTSS_OFF_ANA_CL_ENDPT_LOCAL_CUSTOM_FILTER_CFG_QOS_LOCAL_CUSTOM_FILTER_PRIO);                
  

    /** Port policing (2 policers) **/
    for (indx = 0; indx < VTSS_PORT_POLICERS; indx++) {
        VTSS_RC(b2_policer_setup(vtss_state, VTSS_PORT_POLICERS * line_port + indx, 
                                 &qos->policer_port[indx], 0));
    }
    B2_WRX(ANA_AC, POL_PORT_GAP, line_port, 20); /* 20 bytes IFG */

    /** Queue policing (8 policers) **/
    for (queue = VTSS_QUEUE_START; queue < VTSS_QUEUE_END; queue++) {
        VTSS_RC(b2_policer_setup(vtss_state, VTSS_QUEUES * line_port + queue - VTSS_QUEUE_START,
                                 &qos->policer_queue[queue], 1));
    }
    /** Shaper (1 shaper per port) **/
    VTSS_RC(b2_shaper_setup(vtss_state, line_port, &qos->shaper_port));

    /** Channel shaper (1 shaper per lport) **/
    VTSS_RC(b2_shaper_lport_setup(vtss_state, line_port, &qos->shaper_host));

    /** RED (Random Early Detection) queue configuration         **/
    for (queue = VTSS_QUEUE_START; queue < VTSS_QUEUE_END; queue++) {
        if (qos->red[queue].max < qos->red[queue].min) {
            VTSS_E(("red max is less than red min"));
            return VTSS_RC_ERROR;
        }
        /* Convert Red min/max bytes to 128B cells and max_prob % to a number between 0-255*/       
        indx = VTSS_QUEUES * line_port + (queue - VTSS_QUEUE_START);
        B2_WRX(QSS, Q_RED_WQ, indx, qos->red[queue].weight);
        B2_WRX(QSS, Q_RED_MIN_MAX_TH, indx, 
               (qos->red[queue].max/128 - qos->red[queue].min/128) << VTSS_OFF_QSS_Q_RED_MIN_MAX_TH_Q_RED_MAXMIN_TH |
               (qos->red[queue].min/128) << VTSS_OFF_QSS_Q_RED_MIN_MAX_TH_Q_RED_MIN_TH);
        B2_WRX(QSS, Q_RED_MISC_CFG, indx, 
               qos->red[queue].enable << VTSS_OFF_QSS_Q_RED_MISC_CFG_Q_RED_ENA |
               (qos->red[queue].max_prob_1*255/100) << VTSS_OFF_QSS_Q_RED_MISC_CFG_Q_RED_MAXP_1 |
               (qos->red[queue].max_prob_2*255/100) << VTSS_OFF_QSS_Q_RED_MISC_CFG_Q_RED_MAXP_2 |
               (qos->red[queue].max_prob_3*255/100) << VTSS_OFF_QSS_Q_RED_MISC_CFG_Q_RED_MAXP_3);
    }
    
    /* Queue percentages, 6 queues per port */
    max_pct = 0; /* Maximum percentage */
    for (queue = VTSS_QUEUE_START; queue < (VTSS_QUEUE_END - 2); queue++) {
        pct = qos->scheduler.queue_pct[queue];
        if (pct > 100) {
            VTSS_E("illegal queue_pct: %u", pct);
            return VTSS_RC_ERROR;
        }
        if (pct > max_pct)
            max_pct = pct;
    }
    if (max_pct == 0)
        max_pct = 100;
    for (queue = VTSS_QUEUE_START; queue < (VTSS_QUEUE_END - 2); queue++) {
        weight = b2_min_weight(vtss_state, 0x1fffff*qos->scheduler.queue_pct[queue]/max_pct);
        
        if (vtss_state->init_conf.qs.qw[queue-1] == VTSS_QUEUE_WEIGHT_MAX_ENABLE) {
            B2_WRX(SCH, QS0_WEIGHT, 
                   line_port * (VTSS_QUEUES - 2) + queue - VTSS_QUEUE_START, 
                   weight << VTSS_OFF_SCH_QS0_WEIGHT_QS0_WEIGHT);
        }        
    }

    return VTSS_RC_OK;    
}

static vtss_rc b2_qos_port_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_host_mode_t host_mode;
    vtss_lport_no_t  lport_no;
    u32              weight;
    vtss_qos_port_conf_t *qos;

    if (VTSS_CHIP_PORT(port_no) == CHIP_PORT_SPI4)
        return VTSS_RC_ERROR;

    qos = &vtss_state->qos.port_conf[port_no];
    
    VTSS_RC(b2_scheduler_check(vtss_state));
    
    VTSS_RC(b2_port_qos_set(vtss_state, VTSS_CHIP_PORT(port_no), qos));

    /* Host interface scheduler */
    host_mode = vtss_state->init_conf.host_mode;
    if (host_mode < VTSS_HOST_MODE_8) {
        lport_no = vtss_state->port.map[port_no].lport_no;
        VTSS_RC(b2_weight(vtss_state, qos->scheduler.rate, &weight));
        
        if (host_mode > VTSS_HOST_MODE_3) {
            /* SPI4 */
            B2_WRX(SCH, SPI4_PS_WEIGHT, lport_no, 
                   weight << VTSS_OFF_SCH_SPI4_PS_WEIGHT_SPI4_PS_WEIGHT);

            if (vtss_state->port.spi4.ob.frame_interleave == 0 ) { 
                /* Burst interleave */
                B2_WRX(DSM, SPI4_BS_WEIGHT, lport_no, 
                       weight << VTSS_OFF_DSM_SPI4_BS_WEIGHT_SPI4_BS_WEIGHT);

            }

        } else if (host_mode == VTSS_HOST_MODE_0 || (host_mode == VTSS_HOST_MODE_3 && lport_no < 12)) {
            /* XAUI0 */
            B2_WRX(SCH, XAUI0_PS_WEIGHT, lport_no, 
                   weight << VTSS_OFF_SCH_XAUI0_PS_WEIGHT_XAUI0_PS_WEIGHT);
        } else {
            /* XAUI1 */
            if (host_mode == VTSS_HOST_MODE_3) /* Logical port 12-23 means XAUI1 LPID 0-11 */
                lport_no -= 12;
            B2_WRX(SCH, XAUI1_PS_WEIGHT, lport_no, 
                   weight << VTSS_OFF_SCH_XAUI1_PS_WEIGHT_XAUI1_PS_WEIGHT);
        }
    }

    return VTSS_RC_OK;
}

/* Global chip QoS  */
static vtss_rc b2_qos_conf_set(vtss_state_t *vtss_state, BOOL changed)
{
    int glb_no, filt_no, pair_no, offset;
    vtss_qos_conf_t *qos;

    qos = &vtss_state->qos.conf;

    if (changed) {
        /* Not used */
    }

    /* Global filter configuration         */
    for (glb_no = 0; glb_no < VTSS_CUSTOM_FILTER_COUNT; glb_no++) {
        /*  If matched, drop frame or forward to endpoint  */
        B2_WRX(ANA_CL, FILTER_GLOBAL_CUSTOM_CFG_2, glb_no, 1^qos->custom_filter[glb_no].forward);                        
        /*  Assign RED profile and Priority to this filter              */
        B2_WRX(ANA_CL, ENDPT_GLOBAL_CUSTOM_FILTER_CFG, glb_no, 
           qos->custom_filter[glb_no].qos.red  << VTSS_OFF_ANA_CL_ENDPT_GLOBAL_CUSTOM_FILTER_CFG_QOS_GLOBAL_CUSTOM_FILTER_RED | 
               B2_PRIO(qos->custom_filter[glb_no].qos) << VTSS_OFF_ANA_CL_ENDPT_GLOBAL_CUSTOM_FILTER_CFG_QOS_GLOBAL_CUSTOM_FILTER_PRIO);                

        for (filt_no = 0; filt_no < 7; filt_no++) {
            
            /*  Position the filter in L2, L3 or L4 Header and set the offset   */
            B2_WRXY(ANA_CL, FILTER_GLOBAL_CUSTOM_CFG_0, glb_no, filt_no, 
                    (int)qos->custom_filter[glb_no].filter[filt_no].header << VTSS_OFF_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0_FILTER_GLOBAL_CUSTOM_TYPE | 
                    (qos->custom_filter[glb_no].filter[filt_no].offset/2) << VTSS_OFF_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_0_FILTER_GLOBAL_CUSTOM_POS);

            /* Set the 2 byte mask and pattern */
            B2_WRXY(ANA_CL, FILTER_GLOBAL_CUSTOM_CFG_1, glb_no, filt_no, 
                    qos->custom_filter[glb_no].filter[filt_no].mask << VTSS_OFF_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1_FILTER_GLOBAL_CUSTOM_MASK | 
                    qos->custom_filter[glb_no].filter[filt_no].val << VTSS_OFF_ANA_CL_FILTER_GLOBAL_CUSTOM_CFG_1_FILTER_GLOBAL_CUSTOM_PATTERN);

        }
    }
        
    /* Global TCP/UDP port parameters (8)   */
    for (pair_no = 0; pair_no < VTSS_UDP_TCP_PAIR_COUNT; pair_no++) {        
        offset = pair_no * 2;
            
        B2_WRX(ANA_CL, ENDPT_GLOBAL_TCPUDP_CFG, offset, 
               qos->udp_tcp[pair_no].pair[0].port     << VTSS_OFF_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_VAL | 
               qos->udp_tcp[pair_no].pair[0].qos.red  << VTSS_OFF_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_PORT_RED | 
               B2_PRIO(qos->udp_tcp[pair_no].pair[0].qos) << VTSS_OFF_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_PORT_PRIO); 

        B2_WRX(ANA_CL, ENDPT_GLOBAL_TCPUDP_CFG, offset+1, 
               qos->udp_tcp[pair_no].pair[1].port     << VTSS_OFF_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_VAL | 
               qos->udp_tcp[pair_no].pair[1].qos.red  << VTSS_OFF_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_PORT_RED | 
               B2_PRIO(qos->udp_tcp[pair_no].pair[1].qos) << VTSS_OFF_ANA_CL_ENDPT_GLOBAL_TCPUDP_CFG_QOS_GLOBAL_TCPUDP_PORT_PRIO); 
        
        VTSS_RC(b2_wrf(vtss_state, VTSS_TGT_ANA_CL, VTSS_ADDR_ANA_CL_ENDPT_GLOBAL_TCPUDP_RNG_CFG, pair_no, 1, 0, qos->udp_tcp[pair_no].range));
    }    
    
    return VTSS_RC_OK;
}

/* 2x64 DSCP entry tables exists    */
static vtss_rc b2_dscp_table_set(vtss_state_t *vtss_state, const vtss_dscp_table_no_t table_no)
{
    uint indx, offset;
    vtss_dscp_entry_t    *dscp_table;

    if (table_no == 0) {
        offset = 0;
    } else if (table_no == 1) {
        offset = 64;
    } else {
        return VTSS_RC_ERROR;
    }

    dscp_table = vtss_state->qos.dscp_table[table_no];
        
    for (indx = 0; indx < 64; indx++) {   
        B2_WRX(ANA_CL, ENDPT_DSCP_CFG, offset+indx, 
               dscp_table[indx].enable   << VTSS_OFF_ANA_CL_ENDPT_DSCP_CFG_QOS_DSCP_TRUST_ENA | 
               dscp_table[indx].qos.red  << VTSS_OFF_ANA_CL_ENDPT_DSCP_CFG_QOS_DSCP_RED | 
               B2_PRIO(dscp_table[indx].qos) << VTSS_OFF_ANA_CL_ENDPT_DSCP_CFG_QOS_DSCP_PRIO);
        
    }
    
    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Miscellaneous 
 * ================================================================= */

static vtss_rc b2_chip_id_get(vtss_state_t *vtss_state, vtss_chip_id_t *const chip_id)
{
    u32 value;

    B2_RD(DEVCPU_GCB, CHIP_ID, 0, &value);
    if (B2F(DEVCPU_GCB, CHIP_ID, MFG_ID, value) != 0x074) {
        VTSS_E("CPU interface error, chip_id: 0x%08x", value);
        return VTSS_RC_ERROR;
    }
    chip_id->part_number = B2F(DEVCPU_GCB, CHIP_ID, PART_ID, value);
    chip_id->revision = B2F(DEVCPU_GCB, CHIP_ID, REV_ID, value);
    VTSS_D("part_number: 0x%04x, revision: %u", chip_id->part_number, chip_id->revision);
    return VTSS_RC_OK;
}

static vtss_rc b2_reg_read(vtss_state_t *vtss_state,
                           const vtss_chip_no_t chip_no, const u32 reg, u32 *const value)
{
    return b2_pi_rd_wr(vtss_state, reg >> 16, reg & 0xffff, value, 1);
}

static vtss_rc b2_reg_write(vtss_state_t *vtss_state,
                            const vtss_chip_no_t chip_no, const u32 reg, const u32 value)
{
    u32 val = value;
    return b2_pi_rd_wr(vtss_state, reg >> 16, reg & 0xffff, &val, 0);
}

/* Poll bit until zero */
static vtss_rc b2_poll_bit(vtss_state_t *vtss_state, u32 tgt, u32 addr, u32 offset)
{
    u32 value, count = 0;

    VTSS_N("tgt: %u, addr: %u, offset: %u", tgt, addr, offset);
    do {
        VTSS_RC(b2_rdf(vtss_state, tgt, addr, offset, 1, 0, &value));
        VTSS_MSLEEP(1);
        count++;
        if (count == 100) {
            VTSS_E("timeout, tgt: %u, addr: %u, offset: %u", tgt, addr, offset);
            return VTSS_RC_ERROR;
        }
    } while (value);
    
    VTSS_N("done, count: %u", count);

    return VTSS_RC_OK;
}

#define B2_POLL_BIT(tgt, addr, fld) \
{ \
    vtss_rc rc; \
    if ((rc = b2_poll_bit(vtss_state, VTSS_TGT_##tgt, VTSS_ADDR_##tgt##_##addr, VTSS_OFF_##tgt##_##addr##_##fld)) < VTSS_RC_OK) \
        return rc; \
}

/* Initialize ANA_CL and ANA_AC */
static vtss_rc b2_ana_init(vtss_state_t *vtss_state, vtss_init_conf_t *conf)
{
    u32 port, queue, i;
    
    /* ANA_CL initialization */
    B2_WR(ANA_CL, DEBUG_CFG, 0, 1 << VTSS_OFF_ANA_CL_DEBUG_CFG_CFG_RAM_INIT);

    /* ANA_AC initialization */
    B2_WR(ANA_AC, POL_ALL_CFG, 0, 1 << VTSS_OFF_ANA_AC_POL_ALL_CFG_FORCE_INIT);
    B2_WR(ANA_AC, PORT_STAT_RESET, 0, 1 << VTSS_OFF_ANA_AC_PORT_STAT_RESET_RESET);
    B2_WR(ANA_AC, QUEUE_STAT_RESET, 0, 1 << VTSS_OFF_ANA_AC_QUEUE_STAT_RESET_RESET);
    
    /* Poll one shot bits */
    B2_POLL_BIT(ANA_CL, DEBUG_CFG, CFG_RAM_INIT);
    B2_POLL_BIT(ANA_AC, POL_ALL_CFG, FORCE_INIT);
    B2_POLL_BIT(ANA_AC, PORT_STAT_RESET, RESET);
    B2_POLL_BIT(ANA_AC, QUEUE_STAT_RESET, RESET);

    /* S-tag Ethernet type */
    B2_WR(ANA_CL, STAG_ETYPE_CFG, 0, 
          conf->stag_etype << VTSS_OFF_ANA_CL_STAG_ETYPE_CFG_STAG_ETYPE_VAL);

    /* ANA_AC port counters */
    for (i = 0; i < 8; i++) {
        /* Enable port counters */
        B2_WR(ANA_AC, PORT_STAT_GLOBAL_EVENT_MASK_0 + i, 0, 
              1 << VTSS_OFF_ANA_AC_PORT_STAT_GLOBAL_EVENT_MASK_0_GLOBAL_EVENT_MASK_0);
        
        /* Counter 0,2,4 and 6 are used for dropped frames.
           Counter 1,3,5 and 7 are used for dropped bytes */
        for (port = 0; port < 24; port++) {
            VTSS_RC(b2_wr(vtss_state, VTSS_TGT_ANA_AC, 
                          VTSS_ADDX_ANA_AC_PORT_STAT_CFG_0(port) + i*3, 0, 
                          (i & 1) << VTSS_OFF_ANA_AC_PORT_STAT_CFG_0_CFG_CNT_BYTE_0));
        }
    }
    
    /* Setup the first classifier counter to count all filter events */
    B2_WRX(ANA_CL, TAG_AND_LBL_MASK, 0, 0xffffffff);
    B2_WRX(ANA_CL, DETECTION_MASK, 0, 0xffffffff);

    /* ANA_AC queue counters */
    for (i = 0; i < 2; i++) {
        /* Enable queue counters */
        B2_WR(ANA_AC, QUEUE_STAT_GLOBAL_EVENT_MASK_0 + i, 0, 
              1 << VTSS_OFF_ANA_AC_QUEUE_STAT_GLOBAL_EVENT_MASK_0_GLOBAL_EVENT_MASK_0);
        B2_WR(ANA_AC, QUEUE_STAT_GLOBAL_EVENT_MASK_0 + i, 0, 
              1 << VTSS_OFF_ANA_AC_QUEUE_STAT_GLOBAL_EVENT_MASK_0_GLOBAL_EVENT_MASK_0);
        
        /* Counter 0 is used for dropped frames.
           Counter 1 is used for dropped bytes */
        for (port = 0; port < 24; port++) {
            for (queue = 0; queue < 8; queue++) {
                VTSS_RC(b2_wr(vtss_state, VTSS_TGT_ANA_AC, 
                              VTSS_ADDX_ANA_AC_QUEUE_STAT_CFG_0(port*8 + queue) + i*3, 0,
                              (i & 1) << VTSS_OFF_ANA_AC_QUEUE_STAT_CFG_0_CFG_CNT_BYTE_0));
            }
        }
    }
    return VTSS_RC_OK;
}

/* Initialize ASM */
static vtss_rc b2_asm_init(vtss_state_t *vtss_state, vtss_host_mode_t host_mode)
{
    u32 i, value, port_1g, port_10g;
    BOOL  spi4, xaui0, xaui1, sgmii0_1, sgmii2_23, next_10g;
    
    /* Determine which interfaces are used for the host mode */
    sgmii0_1 = (host_mode < VTSS_HOST_MODE_5);
    sgmii2_23 = (host_mode < VTSS_HOST_MODE_6);
    spi4 = (host_mode > VTSS_HOST_MODE_3);
    xaui0 = (host_mode != VTSS_HOST_MODE_1 && host_mode != VTSS_HOST_MODE_4 && host_mode != VTSS_HOST_MODE_11);
    xaui1 = (host_mode != VTSS_HOST_MODE_0 && host_mode != VTSS_HOST_MODE_4 && host_mode != VTSS_HOST_MODE_10);
    
    /* Calculate Cell Bus Calendar */
    port_1g = 0;
    port_10g = CHIP_PORT_10G_0;
    next_10g = 1;
    for (i = 0; i < 64; i++) {
        if ((i % 4) == 0) {
            /* SPI4 slot */
            value = (spi4 ? CHIP_PORT_SPI4 : CHIP_PORT_IDLE);
        } else if (next_10g) {
            /* 10G slot */
            if (port_10g == CHIP_PORT_10G_0) {
                value = (xaui0 ? port_10g : CHIP_PORT_IDLE);
                port_10g = CHIP_PORT_10G_1;
            } else {
                value = (xaui1 ? port_10g : CHIP_PORT_IDLE);
                port_10g = CHIP_PORT_10G_0;
            }
            next_10g = 0;
        } else {
            /* 1G slot */
            if (port_1g < 2)
                value = (sgmii0_1 ? port_1g : CHIP_PORT_IDLE);
            else 
                value = (sgmii2_23 ? port_1g : CHIP_PORT_IDLE); 
            port_1g += 8;
            if (port_1g > 23)
                port_1g -= 23;
            next_10g = 1;
        }
        B2_WRX(ASM, CBC_CFG, i, value);
    }

    /* Clear lookup table */
    B2_WR(ASM, LUT_INIT_CFG, 0, 1 << VTSS_OFF_ASM_LUT_INIT_CFG_LPORT_MAP_INIT);

    /* Clear ASM statistics */
    B2_WR(ASM, STAT_CFG, 0, 1 << VTSS_OFF_ASM_STAT_CFG_STAT_CNT_CLR_SHOT);
    
    /* Poll one shot bits */
    B2_POLL_BIT(ASM, LUT_INIT_CFG, LPORT_MAP_INIT);
    B2_POLL_BIT(ASM, STAT_CFG, STAT_CNT_CLR_SHOT);

    /* Reset FIFOs */
    B2_WR(ASM, DBG_CFG, 0, 0xffffffff);
    B2_WR(ASM, DBG_CFG, 0, 0);

    /* Clear sticky bits */
    B2_WR(ASM, CSC_STICKY, 0, 0xffffffff);

    return VTSS_RC_OK;
}

/* BRM threshold disabled */
#define BRM_DISABLED 8191

/* Division with round up */
#define ROUND_UP_DIV(x, y) ((x) + (y) - 1)/(y)

/* Setup QSS */
static vtss_rc b2_qss_setup(vtss_state_t *vtss_state)
{
    vtss_init_conf_t *conf = &vtss_state->init_conf;
    vtss_spi4_conf_t *spi4 = &vtss_state->port.spi4;
    vtss_qs_conf_t   *qs;
    int              aport, dport;
    vtss_host_mode_t host_mode;
    vtss_port_no_t   port_no;
    u32              rx = 0, tx = 1; /* Used as index 0/1 */
    u32              port, chan, port_min, port_max, lport_count, rsvd;
    u32              hport_count, chan_min, chan_max, rx_share;
    u32              mtu, total_size[2], gbl_shmax[2], gbl_fc_hth[2], gbl_drop_hth[2];
    u32              gbl_skid[2], gbl_sum_rsvd[2], host_skid;
    u32              port_skid_1g, port_skid_10g, port_skid;
    u32              host_lat_1g, host_lat_10g;
    u32              port_shmax, port_rsvd, port_fc_hth, port_alloc, q6_rsvd, q7_rsvd;
    u32              port_rsvd_min, ratio, sum_rsvd, max_skid, qss_qsp_size;
    u32              a, i, lport, port_1g = 0, port_10g = 0, num_w_queues=0, q_max=0, no_of_ports;
    i32              chip_port;
    
    conf = &vtss_state->init_conf;
    qs = &conf->qs;
    host_mode = conf->host_mode;

    if (host_mode < VTSS_HOST_MODE_8) {
        /* Normal mode: Calculate number of active ports */
        lport_count = 0;
        for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
            if ((chip_port = VTSS_CHIP_PORT(port_no)) < 0 || b2_port_is_host(vtss_state, chip_port))
                continue;
            lport_count++;
            if (VTSS_PORT_IS_10G(chip_port))
                port_10g++;
            else
                port_1g++;
        }
        VTSS_D("lport_count: %u", lport_count);
    } else {
        /* Aggregator/Converter: Setup fixed mapping from arrival ports to departure ports */
        lport_count = (host_mode == VTSS_HOST_MODE_9 ? 16 : 48);
        port_10g = lport_count;
        for (port = 0; port < lport_count; port++) {
            aport = (port + (host_mode == VTSS_HOST_MODE_9 && port > 7 ? 16 : 0));
            dport = (port + 48);
            VTSS_D("aport: %d, dport: %d", aport, dport);
            vtss_state->port.dep_port[aport] = dport;
            vtss_state->port.dep_port[dport] = aport;
        }
    }

    no_of_ports = port_1g+port_10g;

    if (host_mode < VTSS_HOST_MODE_8 || host_mode > VTSS_HOST_MODE_9) {
        q6_rsvd = qs->qs_6==VTSS_QUEUE_STRICT_RESERVE ? 1 : 0;
        q7_rsvd = qs->qs_7==VTSS_QUEUE_STRICT_RESERVE ? 1 : 0;
    } else {
        /* Queues and priorties are used for mapping purposes in HM 8 and 9 */
        /* Forced to default */
        q6_rsvd = 0;
        q7_rsvd = 0;
        for (a=0; a<6; a++) 
            qs->qw[a] = VTSS_QUEUE_WEIGHT_MAX_DISABLE;
    }


    rsvd = q6_rsvd + q7_rsvd;
    if ((rx_share = qs->rx_share) == 0) {
        /* Default Rx share */
        if (qs->mtu <= VTSS_MAX_FRAME_LENGTH_STANDARD)
            rx_share = 50;
        else if (host_mode == VTSS_HOST_MODE_6)
            rx_share = 50;
        else if (host_mode > VTSS_HOST_MODE_6) {
            if (qs->rx_mode == VTSS_RX_MODE_SHARED_DROP)
                rx_share = 50;
            else if (qs->tx_mode == VTSS_TX_MODE_PORT_FIFO)
                rx_share = 60;
            else
                rx_share = 67;
        } else if (qs->rx_mode == VTSS_RX_MODE_SHARED_DROP && 
                   qs->tx_mode == VTSS_TX_MODE_PORT_FIFO)
            rx_share = (rsvd == 2 ? 65 : rsvd == 1 ? 56 : 50);
        else if (qs->tx_mode == VTSS_TX_MODE_PORT_FIFO)
            rx_share = (rsvd == 2 ? 70 : rsvd == 1 ? 67 : 60);
        else if (qs->rx_mode == VTSS_RX_MODE_SHARED_DROP && 
                 qs->tx_mode == VTSS_TX_MODE_SHARED_LOSSLESS)
            rx_share = (rsvd == 2 ? 80 : rsvd == 1 ? 75 : 50);
        else if (qs->tx_mode == VTSS_TX_MODE_SHARED_LOSSLESS)
            rx_share = (rsvd == 2 ? 86 : rsvd == 1 ? 80 : 67);
    }
    
    /* Rx and Tx total size */
    total_size[rx] = (8192*rx_share/100);
    total_size[tx] = (8192 - total_size[rx]);
    B2_WR(QSS, RX_TOTAL_SIZE, 0, total_size[rx]);
    B2_WR(QSS, TX_TOTAL_SIZE, 0, total_size[tx]);

    /* QssMTU */
    mtu = ROUND_UP_DIV(qs->mtu + 24, 128);
    port_skid_1g = (3 * mtu + 2 + 1);
    port_skid_10g = (port_skid_1g + 4);
    
    /* GblRxSkidCells */
    gbl_skid[rx] = (port_1g*port_skid_1g + port_10g*port_skid_10g);
    if (lport_count >= 12)
        gbl_skid[rx] = gbl_skid[rx]*12/lport_count;

    /* HostSkidCells (52.5 = 105/2) */
    host_skid = ROUND_UP_DIV((qs->mtu + (host_mode > VTSS_HOST_MODE_3 ? 1560 : 1240))*2, 105);

    /* QssQspSize (affects Qss strict reservation)  */
    qss_qsp_size = (u32)(13*(2+(float)qs->qss_qsp_bwf_pct/(float)100));

    /* Number of weigthed queues enabled */
    for(a=0; a<6; a++)
        num_w_queues = num_w_queues+(qs->qw[a] == VTSS_QUEUE_WEIGHT_MAX_ENABLE ? 1:0);

    /* GblTxSkidCells */
    hport_count = (host_mode == VTSS_HOST_MODE_3 ? 2 : 1);
    gbl_skid[tx] = hport_count*host_skid;
    
    /* HostAvgSchLat for 1G and 10G port */
    ratio = 2*(host_mode < VTSS_HOST_MODE_4 ? 10 : 
               spi4->ob.clock == VTSS_SPI4_CLOCK_250_0 ? 8 :
               spi4->ob.clock == VTSS_SPI4_CLOCK_312_5 ? 10 :
               spi4->ob.clock == VTSS_SPI4_CLOCK_375_0 ? 12 :
               spi4->ob.clock == VTSS_SPI4_CLOCK_437_5 ? 14 : 16);
    host_lat_1g = ROUND_UP_DIV(mtu*(lport_count/hport_count - 1), ratio);
    host_lat_10g = ROUND_UP_DIV(10*mtu*(lport_count/hport_count - 1), ratio);
    
    /* GblRxSumRsvd and GblTxSumRsvd */
    port_rsvd_min = 2*mtu;
    if (port_rsvd_min > 81)
        port_rsvd_min = 81;

    sum_rsvd = rsvd * qss_qsp_size * no_of_ports;
    gbl_sum_rsvd[rx] = (lport_count*port_rsvd_min + sum_rsvd);
    gbl_sum_rsvd[tx] = lport_count*mtu;

    VTSS_D("------ QSS Setup ---------------------------------");
    VTSS_D("MaxNumPorts       : %u", lport_count);
    VTSS_D("QssRxBufferStyle  : %s", 
           qs->rx_mode == VTSS_RX_MODE_PORT_FIFO ? "Port_FIFO" : 
           qs->rx_mode == VTSS_RX_MODE_SHARED_DROP ? "Shared_Drop" : "Shared_Lossless");
    VTSS_D("QssTxBufferStyle  : %s", 
            qs->tx_mode == VTSS_TX_MODE_PORT_FIFO ? "Port_FIFO" : "Shared_Lossless");
    VTSS_D("QssCBMPctRx       : %u", rx_share);
    VTSS_D("QssReserveQ6      : %u", qs->qs_6==VTSS_QUEUE_STRICT_RESERVE ? 1 : 0);
    VTSS_D("QssReserveQ7      : %u", qs->qs_7==VTSS_QUEUE_STRICT_RESERVE ? 1 : 0);
    VTSS_D("QssQspBwF         : %u", qs->qss_qsp_bwf_pct);
    VTSS_D("QssEnableQw       : q5:%u q4:%u q3:%u q2:%u q1:%u q0:%u",
            qs->qw[5]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0,qs->qw[4]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0,qs->qw[3]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0,
            qs->qw[2]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0,qs->qw[1]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0,qs->qw[0]==VTSS_QUEUE_WEIGHT_MAX_ENABLE?1:0);
    VTSS_D("QssMTU            : %u", mtu);
    VTSS_D("PortSkidCells(1G) : %u", port_skid_1g);
    VTSS_D("PortSkidCells(10G): %u", port_skid_10g);
    VTSS_D("GblRxSkidCells    : %u", gbl_skid[rx]);
    VTSS_D("GblTxSkidCells    : %u", gbl_skid[tx]);
    VTSS_D("GblRxSumRsvd      : %u", gbl_sum_rsvd[rx]);
    VTSS_D("GblTxSumRsvd      : %u", gbl_sum_rsvd[tx]);
    VTSS_D("HostNumIF         : %u", hport_count);
    VTSS_D("HostSkidCells     : %u", host_skid);
    VTSS_D("HostAvgSchLat(1G) : %u", host_lat_1g);
    VTSS_D("HostAvgSchLat(10G): %u", host_lat_10g);
    VTSS_D("--------------------------------------------------");

    /* Configuration rules */
    /* Rules for VTSS_RX_MODE_PORT_FIFO     */
    if (qs->rx_mode == VTSS_RX_MODE_PORT_FIFO &&
        total_size[rx] < (lport_count*mtu + gbl_skid[rx] + sum_rsvd)) {        
        VTSS_E("illegal setup for RX_MODE_PORT_FIFO");
        return VTSS_RC_ERROR;
    }

    if (no_of_ports != 0 && num_w_queues != 0) {    
        if (qs->rx_mode == VTSS_RX_MODE_PORT_FIFO &&(((total_size[rx]/(no_of_ports) - sum_rsvd) / num_w_queues) < mtu)) {
            VTSS_E(("illegal setup for RX_MODE_PORT_FIFO"));
            return VTSS_RC_ERROR;
        }
    }
   
    /* Rules for VTSS_RX_MODE_SHARED_DROP */
    max_skid = (port_10g ? port_skid_10g : port_skid_1g);
    if (qs->rx_mode == VTSS_RX_MODE_SHARED_DROP &&
        total_size[rx] < (gbl_sum_rsvd[rx] + max_skid)) {
        VTSS_E(("illegal setup for RX_MODE_SHARED_DROP"));
        return VTSS_RC_ERROR;
    }
    if (no_of_ports != 0 && num_w_queues != 0) {    
        if (qs->rx_mode == VTSS_RX_MODE_SHARED_DROP &&
            ((total_size[rx] - gbl_sum_rsvd[rx] / (num_w_queues * no_of_ports)) < mtu)) {
            VTSS_E(("illegal setup for RX_MODE_SHARED_DROP"));        
            return VTSS_RC_ERROR;
        }
    }

    /* Rule for VTSS_RX_MODE_SHARED_LOSSLESS */
    if (qs->rx_mode == VTSS_RX_MODE_SHARED_LOSSLESS &&
        total_size[rx] < (gbl_sum_rsvd[rx] + gbl_skid[rx] + max_skid)) {
        VTSS_E(("illegal setup for RX_MODE_SHARED_LOSSLESS"));
        return VTSS_RC_ERROR;
    }
    
    /* Rule for VTSS_TX_MODE_PORT_FIFO */
    if (qs->tx_mode == VTSS_TX_MODE_PORT_FIFO &&
        total_size[tx] < (lport_count*(mtu + host_skid))) {
        VTSS_E(("illegal setup for TX_MODE_PORT_FIFO"));
        return VTSS_RC_ERROR;
    }

    /* Rule for VTSS_TX_MODE_SHARED_LOSSLESS */
    if (qs->tx_mode == VTSS_TX_MODE_SHARED_LOSSLESS &&
        total_size[tx] < (gbl_sum_rsvd[tx] + gbl_skid[tx] + host_skid)) {
        VTSS_E(("illegal setup for TX_MODE_SHARED_LOSSLESS"));
        return VTSS_RC_ERROR;
    }
        
    /* Rx settings */
    gbl_drop_hth[rx] = BRM_DISABLED;
    switch (qs->rx_mode) {
    case VTSS_RX_MODE_PORT_FIFO:
        VTSS_D(("VTSS_RX_MODE_PORT_FIFO"));
        gbl_shmax[rx] = 0;
        gbl_fc_hth[rx] = BRM_DISABLED;
        break;
    case VTSS_RX_MODE_SHARED_DROP:
        VTSS_D(("VTSS_RX_MODE_SHARED_DROP"));
        gbl_shmax[rx] = (total_size[rx] - gbl_sum_rsvd[rx]);
        gbl_fc_hth[rx] = BRM_DISABLED;
        break;
    case VTSS_RX_MODE_SHARED_LOSSLESS:
        VTSS_D(("VTSS_RX_MODE_SHARED_LOSSLESS"));
        gbl_shmax[rx] = (total_size[rx] - gbl_sum_rsvd[rx]);
        gbl_fc_hth[rx] = (gbl_shmax[rx] - gbl_skid[rx]);
        break;
    default:
        VTSS_E(("illegal rx_mode"));
        return VTSS_RC_ERROR;
    }

    /* Tx settings */
    gbl_drop_hth[tx] = BRM_DISABLED;
    switch (qs->tx_mode) {
    case VTSS_TX_MODE_PORT_FIFO:
        VTSS_D(("VTSS_TX_MODE_PORT_FIFO"));
        gbl_shmax[tx] = 0;
        gbl_fc_hth[tx] = BRM_DISABLED;
        break;
    case VTSS_TX_MODE_SHARED_LOSSLESS:
        VTSS_D(("VTSS_TX_MODE_SHARED_LOSSLESS"));
        gbl_shmax[tx] = (total_size[tx] - gbl_sum_rsvd[tx]);
        gbl_fc_hth[tx] = (gbl_shmax[tx] - gbl_skid[tx]);
        break;
    default:
        VTSS_E(("illegal tx_mode"));
        return VTSS_RC_ERROR;
    }

    /* Setup Rx and Tx for classes 0 and 1 */
    for (i = 0; i < 2; i++) {
        /* Rx */
        B2_WRX(QSS, RX_GBL_SHMAX, i, gbl_shmax[rx] << VTSS_OFF_QSS_RX_GBL_SHMAX_RX_GBL_SHMAX);
        B2_WRX(QSS, RX_GBL_DROP_HLTH, i,
               (gbl_drop_hth[rx] << VTSS_OFF_QSS_RX_GBL_DROP_HLTH_RX_GBL_DROP_HTH) |
               (gbl_drop_hth[rx] << VTSS_OFF_QSS_RX_GBL_DROP_HLTH_RX_GBL_DROP_LTH));
        B2_WRX(QSS, RX_GBL_FC_HLTH, i,
               (gbl_fc_hth[rx] << VTSS_OFF_QSS_RX_GBL_FC_HLTH_RX_GBL_FC_HTH) |
               (gbl_fc_hth[rx] << VTSS_OFF_QSS_RX_GBL_FC_HLTH_RX_GBL_FC_LTH));
        B2_WRX(QSS, RX_GBL_ENA, i, 1 << VTSS_OFF_QSS_RX_GBL_ENA_RX_GBL_ENA);
        
        /* Tx */
        B2_WRX(QSS, TX_GBL_SHMAX, i, gbl_shmax[tx] << VTSS_OFF_QSS_TX_GBL_SHMAX_TX_GBL_SHMAX);
        B2_WRX(QSS, TX_GBL_DROP_HLTH, i,
               (gbl_drop_hth[tx] << VTSS_OFF_QSS_TX_GBL_DROP_HLTH_TX_GBL_DROP_HTH) |
               (gbl_drop_hth[tx] << VTSS_OFF_QSS_TX_GBL_DROP_HLTH_TX_GBL_DROP_LTH));
        B2_WRX(QSS, TX_GBL_FC_HLTH, i,
               (gbl_fc_hth[tx] << VTSS_OFF_QSS_TX_GBL_FC_HLTH_TX_GBL_FC_HTH) |
               (gbl_fc_hth[tx] << VTSS_OFF_QSS_TX_GBL_FC_HLTH_TX_GBL_FC_LTH));
        B2_WRX(QSS, TX_GBL_ENA, i, 1 << VTSS_OFF_QSS_TX_GBL_ENA_TX_GBL_ENA);
    }

    /* Port settings */
    port_min = (host_mode < VTSS_HOST_MODE_5 ? 0 : host_mode == VTSS_HOST_MODE_5 ? 2 : host_mode == VTSS_HOST_MODE_11 ? 25 : 24);
    port_max = (host_mode < VTSS_HOST_MODE_5 ? 23 : host_mode == VTSS_HOST_MODE_10 ? 24 : 25);
    chan_min = 0;
    chan_max = (host_mode < VTSS_HOST_MODE_8 ? 0 : host_mode == VTSS_HOST_MODE_8 ? 23 : host_mode == VTSS_HOST_MODE_9 ? 7 : 47);
    for (port = port_min; port <= port_max; port++) {
        for (chan = chan_min; chan <= chan_max; chan++) {
            aport = (host_mode > VTSS_HOST_MODE_6 ? 
                     port == CHIP_PORT_10G_0 ? chan : (chan + 24) :
                     b2_port2line(port));
            if ((dport = vtss_state->port.dep_port[aport]) < 0)
                continue; /* Skip unused ports */
            if (VTSS_PORT_IS_10G(port)) {
                port_skid = port_skid_10g;
                /* host_lat = host_lat_10g; */
            } else {
                port_skid = port_skid_1g;
                /* host_lat = host_lat_1g; */
            }
            for (i = rx; i <= tx; i++) {
                port_alloc = 1;
                if (i == rx) {
                    /* Rx */
                    lport = dport;

                    q6_rsvd = qs->qs_6 == VTSS_QUEUE_STRICT_RESERVE ? qss_qsp_size : 0;
                    q7_rsvd = qs->qs_7 == VTSS_QUEUE_STRICT_RESERVE ? qss_qsp_size : 0;
                    switch (qs->rx_mode) {
                    case VTSS_RX_MODE_PORT_FIFO:
                        port_shmax = (total_size[rx]/lport_count - q6_rsvd - q7_rsvd);
                        port_rsvd = port_shmax;                        
                        port_fc_hth = (port_shmax - port_skid);
                        if (num_w_queues>0)
                            q_max = port_shmax/num_w_queues;
                        else 
                            q_max = 0;
                        break;
                    case VTSS_RX_MODE_SHARED_DROP:
                        port_rsvd = port_rsvd_min;
                        port_shmax = (port_rsvd + gbl_shmax[rx]);
                        port_fc_hth = (port_shmax - port_skid);
                        port_alloc = 13;
                        if (num_w_queues>0)
                            q_max = port_shmax/num_w_queues;
                        else 
                            q_max = 0;
                        break;
                    case VTSS_RX_MODE_SHARED_LOSSLESS:
                    default:
                        port_rsvd = port_rsvd_min;
                        port_shmax = (port_rsvd + gbl_shmax[rx]);
                        port_fc_hth = BRM_DISABLED;
                        q_max = 0x1FFF;
                        break;
                    }
                    
                    if (qs->qs_6 == VTSS_QUEUE_STRICT_RESERVE) {
                        B2_WRX(QSS, Q_RSVD, b2_qid(vtss_state, dport, 6), q6_rsvd);
                    } else if (qs->qs_6 == VTSS_QUEUE_STRICT_DISABLE) {
                        B2_WRX(QSS, Q_ENA, b2_qid(vtss_state, dport, 6), 0); 
                    }    /* Default enabled */
                    

                    if (qs->qs_7 == VTSS_QUEUE_STRICT_RESERVE) {
                        B2_WRX(QSS, Q_RSVD, b2_qid(vtss_state, dport, 7), q7_rsvd);
                    } else if (qs->qs_7 == VTSS_QUEUE_STRICT_DISABLE) {
                        B2_WRX(QSS, Q_ENA, b2_qid(vtss_state, dport, 7), 0); 
                    } /* Default enabled */

                    for (a=0; a<6; a++) {
                        if (qs->qw[a] == VTSS_QUEUE_WEIGHT_MAX_ENABLE) {
                            B2_WRX(QSS, Q_MAX, b2_qid(vtss_state, dport, a), q_max);
                        } else if (qs->qw[a] == VTSS_QUEUE_WEIGHT_DISABLE) {
                            B2_WRX(QSS, Q_ENA, b2_qid(vtss_state, dport, a), 0); 
                        } /* Default enabled */
                    }
                } else {
                    /* Tx */
                    lport = aport;
                    switch (qs->tx_mode) {
                    case VTSS_TX_MODE_PORT_FIFO:
                        port_shmax = total_size[tx]/lport_count;
                        port_rsvd = port_shmax;
                        port_fc_hth = (port_shmax - host_skid);
                        break;
                    case VTSS_TX_MODE_SHARED_LOSSLESS:
                    default:
                        port_rsvd = mtu;
                        port_shmax = (port_rsvd + gbl_shmax[tx]);
                        port_fc_hth = BRM_DISABLED;
                        break;
                    }
                } 

                B2_WRX(QSS, PORT_RSVD_GFC_LTH, lport,
                       (port_rsvd << VTSS_OFF_QSS_PORT_RSVD_GFC_LTH_PORT_RSVD) |
                       (port_rsvd << VTSS_OFF_QSS_PORT_RSVD_GFC_LTH_PORT_GFC_LTH));
                B2_WRX(QSS, PORT_SHMAX, lport,
                       port_shmax << VTSS_OFF_QSS_PORT_SHMAX_PORT_SHMAX);
                B2_WRX(QSS, PORT_DROP_HLTH, lport,
                       (BRM_DISABLED << VTSS_OFF_QSS_PORT_DROP_HLTH_PORT_DROP_HTH) |
                       (BRM_DISABLED << VTSS_OFF_QSS_PORT_DROP_HLTH_PORT_DROP_LTH));
                B2_WRX(QSS, PORT_FC_HLTH, lport,
                       (port_fc_hth << VTSS_OFF_QSS_PORT_FC_HLTH_PORT_FC_HTH) |
                       (port_fc_hth << VTSS_OFF_QSS_PORT_FC_HLTH_PORT_FC_LTH));
                B2_WRX(QSS, PORT_PRE_ALLOC, lport,
                       port_alloc << VTSS_OFF_QSS_PORT_PRE_ALLOC_PORT_PRE_ALLOC);
                B2_WRX(QSS, PORT_ENA, lport, 1 << VTSS_OFF_QSS_PORT_ENA_PORT_ENA);

                /* Arrival port map */
                B2_WRX(QSS, APORT_ENA_CLASS_MAP, lport,
                       (1 << VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_ENA) |
                       (0 << VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_CLASS) |
                       (vtss_state->port.dep_port[lport] << 
                        VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_MAP));
            }
        }
    }

    return VTSS_RC_OK;
}

/* Initialize QSS */
static vtss_rc b2_qss_init(vtss_state_t *vtss_state)
{    
    B2_WR(QSS, QSS_INIT, 0, 1 << VTSS_OFF_QSS_QSS_INIT_QSS_INIT);
    B2_POLL_BIT(QSS, QSS_INIT, QSS_INIT);         
    return VTSS_RC_OK;
}


/* Setup host flow control */
static vtss_rc b2_host_flow_ctrl(vtss_state_t *vtss_state)
{
  BOOL              fc_spi4, fc_xaui0, fc_xaui1, ibfc_xaui0, ibfc_xaui1 ;
  

/* Channel flowcontrol */
  fc_spi4 = (vtss_state->port.spi4.fc.enable ? 1 : 0);
  fc_xaui0 = (vtss_state->port.xaui[0].fc.channel_enable ? 1 : 0);
  fc_xaui1 = (vtss_state->port.xaui[1].fc.channel_enable ? 1 : 0);
  ibfc_xaui0 = (vtss_state->port.xaui[0].fc.ib.enable ? 1 : 0);
  ibfc_xaui1 = (vtss_state->port.xaui[1].fc.ib.enable ? 1 : 0);
  
  fc_xaui1 |= ibfc_xaui1;
  fc_xaui0 |= ibfc_xaui0;
  B2_WR(SCH, FLOW_CONTROL_ENABLE, 0, 
        (ibfc_xaui0 << VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_XAUI0_FC_SELECT) |
        (ibfc_xaui1 << VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_XAUI1_FC_SELECT) |
        (fc_xaui0   << VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_XAUI0_FC_EN) |
        (fc_xaui1   << VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_XAUI1_FC_EN) |
        (fc_spi4 << VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_SPI4_STATUS_FC_EN) |
        (0 << VTSS_OFF_SCH_FLOW_CONTROL_ENABLE_SPI4_PS_BM_STATUS_FC_EN));
  return VTSS_RC_OK;
}

/* Initialize SCH */
static vtss_rc b2_sch_init(vtss_state_t *vtss_state, vtss_init_conf_t *conf)
{
    u32 i, w;
 
    
    B2_WR(SCH, SPR_INIT, 0, 1 << VTSS_OFF_SCH_SPR_INIT_SPR_INIT);
    B2_POLL_BIT(SCH, SPR_INIT, SPR_INIT);

    B2_WR(SCH, PP_SPR_INIT, 0, 1 << VTSS_OFF_SCH_PP_SPR_INIT_PP_SPR_INIT);
    B2_POLL_BIT(SCH, PP_SPR_INIT, PP_SPR_INIT);

    B2_WR(SCH, SPR_PORT_ENABLE, 0, 0 << VTSS_OFF_SCH_SPR_PORT_ENABLE_SPR_PORT_ENABLE);

    /* Scheduler weights */
    VTSS_RC(b2_weight(vtss_state, VTSS_BITRATE_DISABLED, &w));

    for (i = 0; i < 24; i++) {
        B2_WRX(SCH, XAUI0_PS_WEIGHT, i, w << VTSS_OFF_SCH_XAUI0_PS_WEIGHT_XAUI0_PS_WEIGHT);
        B2_WRX(SCH, XAUI1_PS_WEIGHT, i, w << VTSS_OFF_SCH_XAUI1_PS_WEIGHT_XAUI1_PS_WEIGHT);
    }



    return VTSS_RC_OK;
}


/* Determine total number of logical ports based on departure port mapping */
static u32 b2_lport_count(vtss_state_t *vtss_state)
{
    u32 iport, count = 0;

    for (iport = 0; iport < VTSS_INT_PORT_COUNT; iport++)
        if (vtss_state->port.dep_port[iport] >= 0)
            count++;
    return (count/2);
}

/* Initialize SPI4.2 */
static vtss_rc b2_spi4_conf(vtss_state_t *vtss_state)
{
    int              i, count;
    vtss_spi4_conf_t *spi4;
    u32              vco, cmu, w;
    vtss_host_mode_t host_mode;
    BOOL             fc_spi4, fc_xaui0, fc_xaui1;

    VTSS_D("Enter");
    
    host_mode = vtss_state->init_conf.host_mode;

    if (host_mode < VTSS_HOST_MODE_4) {
        /* XAUI mode, power down SPI4 */
        B2_WR(DEVSPI, SPI4_DDS_CONFIG, 0, 1 << VTSS_OFF_DEVSPI_SPI4_DDS_CONFIG_CONF_POWEROFF);
    } else {      
        /* SPI4.2 setup */
        spi4 = &vtss_state->port.spi4;

        /* Check revision C features */
        if (spi4->ob.fcs_strip || spi4->ob.hih_length_update)
            VTSS_RC(b2_rev_c_check(vtss_state, "SPI4 FCS stripping"));

        if (!spi4->ob.frame_interleave && !spi4->fc.enable)
            VTSS_I("SPI4 channel flowcontrol is always enabled in SPI4 burst mode");
        
        /* Setup SPI4 interleave mode */
        B2_WR(DEVCPU_GCB, CHIP_MODE, 0,
              (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_ID_SEL) |
              ((vtss_state->port.xaui[0].status_select == VTSS_XAUI_STATUS_XAUI_1 ? 1 : 0) <<
               VTSS_OFF_DEVCPU_GCB_CHIP_MODE_XAUI_STATUS_CHANNEL_SEL) |
              (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_B_SRC_SEL) |
              (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_A_SRC_SEL) |
              (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_B_DRIVE_EN) |
              (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_A_DRIVE_EN) |
              (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_STD_PREAMBLE_ENA) |
              ((spi4->ob.frame_interleave ? 0 : 1) << 
               VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SPI4_INTERLEAVE_MODE) |
              ((vtss_state->port.xaui[0].hih.format == VTSS_HIH_PRE_SFD ? 1 : 0) << 
               VTSS_OFF_DEVCPU_GCB_CHIP_MODE_XAUI_TAG_FORM) |
              ((int)host_mode << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_HOST_MODE));
        

        /* Scheduler weights */
        VTSS_RC(b2_weight(vtss_state, VTSS_BITRATE_DISABLED, &w));
        for (i = 0; i < 48; i++) {
            B2_WRX(SCH, SPI4_PS_WEIGHT, i, w << VTSS_OFF_SCH_SPI4_PS_WEIGHT_SPI4_PS_WEIGHT);
            if (spi4->ob.frame_interleave == 0 ) { 
                /* Burst interleave */
                B2_WRX(DSM, SPI4_BS_WEIGHT, i, w << VTSS_OFF_DSM_SPI4_BS_WEIGHT_SPI4_BS_WEIGHT);            
            }
        }

        /* QSS 3 level flowcontrol */
        fc_spi4 = (spi4->fc.three_level_enable ? 1 : 0);
        fc_xaui0 = (vtss_state->port.xaui[0].fc.three_level_enable ? 1 : 0);
        fc_xaui1 = (vtss_state->port.xaui[1].fc.three_level_enable ? 1 : 0);
        B2_WR(QSS, FC_3LVL, 0,
              (2 << VTSS_OFF_QSS_FC_3LVL_DISABLED_PORT_FC) |
              (fc_spi4 << VTSS_OFF_QSS_FC_3LVL_SPI4_FC_3LVL) |
              (fc_xaui0 << VTSS_OFF_QSS_FC_3LVL_XAUI1_FC_3LVL) |
              (fc_xaui1 << VTSS_OFF_QSS_FC_3LVL_XAUI0_FC_3LVL));

        /* QSS setup */
        VTSS_RC(b2_qss_setup(vtss_state));
      
        /* Shaper */
        VTSS_RC(b2_shaper_setup(vtss_state, 26, &spi4->qos.shaper));

        /* Put both IB and OB in reset */
        B2_WRF(DEVSPI, SPI4_DDS_CONFIG, CONF_IB_RESET, 0, 1);
        B2_WRF(DEVSPI, SPI4_DDS_CONFIG, CONF_OB_RESET, 0, 1);
     
        /* Inbound */
        B2_WR(ASM, SPI4_CH_CFG, 0,
              ((spi4->ib.fcs == VTSS_SPI4_FCS_ADD ? 2 : 0) << 
               VTSS_OFF_ASM_SPI4_CH_CFG_SPI4_FRM_FCS_MODE_SEL) |
              ((spi4->ib.fcs == VTSS_SPI4_FCS_DISABLED ? 0 : 1) << 
               VTSS_OFF_ASM_SPI4_CH_CFG_SPI4_FRM_FCS_ENA));

        B2_WR(DEVSPI, SPI4_DDS_IB_CONFIG, 0,
              ((int)spi4->ib.clock << VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_VCO_RANGE) |
              ((spi4->ib.data_swap ? 1 : 0) << 
               VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_DATA_SWAP) |
              ((spi4->ib.data_invert ? 0x1ffff : 0) << 
               VTSS_OFF_DEVSPI_SPI4_DDS_IB_CONFIG_CONF_IB_INV_DATA));

        /* Outbound */
        switch (spi4->ob.clock) {
        case VTSS_SPI4_CLOCK_250_0:
            cmu = 0;
            vco = 0;
            break;
        case VTSS_SPI4_CLOCK_312_5:
            cmu = 2;
            vco = 1;
            break;
        case VTSS_SPI4_CLOCK_375_0:
            cmu = 4;
            vco = 2;
            break;
        case VTSS_SPI4_CLOCK_437_5:
            cmu = 6;
            vco = 2;
            break;
        case VTSS_SPI4_CLOCK_500_0:
            cmu = 8;
            vco = 3;
            break;
        default:
            VTSS_E("illegal outbound clock: %d", spi4->ob.clock);
            return VTSS_RC_ERROR;
        }
        if (spi4->ob.burst_size > VTSS_SPI4_BURST_256) {
            VTSS_E("illegal burst_size: %d", spi4->ob.burst_size);
            return VTSS_RC_ERROR;
        }
        B2_WR(DSM, SPI4_CFG, 0,
              ((spi4->ob.max_burst_2) << VTSS_OFF_DSM_SPI4_CFG_MAXBURST2) |
              ((spi4->ob.max_burst_1) << VTSS_OFF_DSM_SPI4_CFG_MAXBURST1) |
              (((int)spi4->ob.burst_size + 2) << VTSS_OFF_DSM_SPI4_CFG_SPI4_BURST_SIZE) |
              ((spi4->ob.hih_length_update ? 1 : 0) << 
               VTSS_OFF_DSM_SPI4_CFG_SPI4_ALTER_HIH_PLI_VALUE) |
              ((spi4->ob.fcs_strip ? 1 : 0) << VTSS_OFF_DSM_SPI4_CFG_SPI4_STRIP_FCS) |
              ((spi4->ob.hih_enable ? 1 : 0) << VTSS_OFF_DSM_SPI4_CFG_SPI4_HDR_CFG));
        B2_WR(DEVSPI, SPI4_DDS_OB_CONFIG, 0,
              (vco << VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_VCO_RANGE) |
              ((int)spi4->ob.clock_phase << VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_CLK_PHASE) |
              (cmu << VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_CMU_RATIO) |
              ((spi4->ob.data_swap ? 1 : 0) << 
               VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_DATA_SWAP) |
              ((spi4->ob.data_invert ? 0x1ffff : 0) << 
               VTSS_OFF_DEVSPI_SPI4_DDS_OB_CONFIG_CONF_OB_INV_DATA));
        B2_WR(DEVSPI, SPI4_IB_TC_CONFIG, 0,
              (1 << VTSS_OFF_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_NO_LOCK) |
              (1 << VTSS_OFF_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_ENA));
        B2_WR(DEVSPI, SPI4_IB_SYNC_CONFIG, 0,
              (1 << VTSS_OFF_DEVSPI_SPI4_IB_SYNC_CONFIG_CONF_IB_DIP4_RESYNC_EN) |
              (1 << VTSS_OFF_DEVSPI_SPI4_IB_SYNC_CONFIG_CONF_IB_FRC_DSKW_RESYNC));
        if (vtss_state->misc.chip_id.revision == 2) {
            /* Revision B reset using IB_QUARTER_RATE field */
            B2_WRF(DEVSPI, SPI4_DDS_IB_CONFIG, CONF_IB_QUARTER_RATE, 0, 1);
        }
        VTSS_MSLEEP(500);
        /* Gnats #6580 PLL runaway.  Workaround copied from TCL code - but not tested on a defect chip */    
        /* force PLL to slowest, which will kick start PLL to re-lock */
        B2_WRF(DEVSPI, SPI4_DDS_OB_CONFIG, CONF_OB_FORCEMIN, 0, 1);
        B2_WRF(DEVSPI, SPI4_DDS_IB_CONFIG, CONF_IB_FORCEMIN, 0, 1);
        /* wait for pll to settle */
        VTSS_MSLEEP(20);
        /* Back to normal operation */
        B2_WRF(DEVSPI, SPI4_DDS_OB_CONFIG, CONF_OB_FORCEMIN, 0, 0);
        B2_WRF(DEVSPI, SPI4_DDS_IB_CONFIG, CONF_IB_FORCEMIN, 0, 0);

        /*  take IB and OB out of reset */
        B2_WRF(DEVSPI, SPI4_DDS_CONFIG, CONF_IB_RESET, 0, 0);
        B2_WRF(DEVSPI, SPI4_DDS_CONFIG, CONF_OB_RESET, 0, 0);
        /* Workaround (Gnats #6580) completed */    

        B2_WR(DEVSPI, SPI4_IB_TC_CONFIG, 0,
              (1 << VTSS_OFF_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_NO_LOCK) |
              (0 << VTSS_OFF_DEVSPI_SPI4_IB_TC_CONFIG_CONF_IB_TC_ENA));
    
        /* Calendar */
        count = b2_lport_count(vtss_state);
        for (i = 0; i < count; i++) {
            B2_WRX(DEVSPI, SPI4_IBS_CALENDAR, i, 
                   (i + 48)<< VTSS_OFF_DEVSPI_SPI4_IBS_CALENDAR_CONF_IBS_CAL_VAL);
            B2_WRX(DEVSPI, SPI4_OBS_CALENDAR, i, 
                   (i + 48)<< VTSS_OFF_DEVSPI_SPI4_OBS_CALENDAR_CONF_OBS_CAL_VAL);
        }
        B2_WR(DEVSPI, SPI4_OBS_LINK_CONFIG, 0,
              (8 << VTSS_OFF_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_IDLE_THRESH) |
              (0 << VTSS_OFF_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_LINK_DOWN_DIS) |
              ((spi4->ob.link_up_limit - 1)<< 
               VTSS_OFF_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_LINKUP_LIM) |
              ((spi4->ob.link_down_limit - 1)<< 
               VTSS_OFF_DEVSPI_SPI4_OBS_LINK_CONFIG_CONF_OBS_LINKDN_LIM));
        B2_WR(DEVSPI, SPI4_OBS_CONFIG, 0,
              ((spi4->ob.clock_shift ? 1 : 0) << 
               VTSS_OFF_DEVSPI_SPI4_OBS_CONFIG_CONF_OBS_CLK_SHIFT) | 
              (0 << VTSS_OFF_DEVSPI_SPI4_OBS_CONFIG_CONF_OBS_CAL_M) |
              (count << VTSS_OFF_DEVSPI_SPI4_OBS_CONFIG_CONF_OBS_CAL_LEN));
        B2_WR(DEVSPI, SPI4_OB_TRAIN_CONFIG, 0,
              ((spi4->ob.training_mode == VTSS_SPI4_TRAINING_AUTO ? 1 : 0) <<
               VTSS_OFF_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_ENA_AUTO_TRAINING) |
              ((spi4->ob.training_mode == VTSS_SPI4_TRAINING_FORCED ? 1 : 0) <<
               VTSS_OFF_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_SEND_TRAINING) |
              (spi4->ob.alpha << VTSS_OFF_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_ALPHA_MAX_T) |
              ((spi4->ob.training_mode == VTSS_SPI4_TRAINING_DISABLED ? 0 : 
                (spi4->ob.data_max_t/128)) << 
               VTSS_OFF_DEVSPI_SPI4_OB_TRAIN_CONFIG_CONF_OB_DATA_MAX_T));
        B2_WR(DEVSPI, SPI4_IBS_CONFIG, 0,
              ((spi4->ib.clock_shift ? 1 : 0) << 
               VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_CLK_SHIFT) |
              (0 << VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_FORCE_IDLE) |
              (1 << VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_DSKW_GENS_IDLE) |
              (1 << VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_DIP4_GENS_IDLE) |
              (0 << VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_CAL_M) |
              (count << VTSS_OFF_DEVSPI_SPI4_IBS_CONFIG_CONF_IBS_CAL_LEN));
    }

    /* Clear sticky bits */
    B2_WR(DEVSPI, SPI4_IB_STICKY, 0, 0xffffffff); 
    B2_WR(DEVSPI, SPI4_OB_STICKY, 0, 0xffffffff); 
    B2_WR(DEVSPI, SPI4_OBS_STICKY, 0, 0xffffffff); 

    /* Clear counters */
    B2_WR(DEVSPI, SPI4_IB_DIP4_ERR_CNT, 0, 0);
    B2_WR(DEVSPI, SPI4_IB_PKT_CNT, 0, 0);
    B2_WR(DEVSPI, SPI4_IB_EOPA_CNT, 0, 0);
    B2_WR(DEVSPI, SPI4_IB_BYTE_CNT, 0, 0);
    B2_WR(DEVSPI, SPI4_OB_PKT_CNT, 0, 0);
    B2_WR(DEVSPI, SPI4_OB_EOPA_CNT, 0, 0);
    B2_WR(DEVSPI, SPI4_OB_BYTE_CNT, 0, 0);
    B2_WR(DEVSPI, SPI4_OBS_DIP2_ERR_CNT, 0, 0);

    VTSS_D("Exit");
    return VTSS_RC_OK;
}

/* Setup inbound flowcontrol */
static vtss_rc b2_xaui_ibfc_setup(vtss_state_t *vtss_state, u32 port, vtss_xaui_conf_t *xaui)
{  
    u32 aport, lport=0, value, mask = 0;
    u32 i;
    vtss_host_mode_t host_mode;

    /* Feature only supported in Chip Rev C  */
    if (xaui->fc.ib.enable)
        VTSS_RC(b2_rev_c_check(vtss_state, "IBFC"));

    host_mode = vtss_state->init_conf.host_mode;

    if (host_mode > VTSS_HOST_MODE_3 && host_mode < VTSS_HOST_MODE_8) {
        return VTSS_RC_OK; /* Modes 0,1,3,8,9,10,11 are supported for IBFC */
    }

    /* Simplified setup for 2 devices attached directly to each other  */
    if (port == CHIP_PORT_10G_0) {
        /* XAUI_0  */
        /* Enable/disable inbound fc */
        B2_WR(DSM, XAUI0_TX_FC_FRAME_EN, 0, xaui->fc.ib.enable);

        /* Assign slot id to XAUI port */
        B2_WR(DSM, XAUI0_SOURCE_SLOT_ID, 0, 0); /* XAUI_0:0 */

        /* DMAC Pattern MSB bits 47:24  */
        B2_WR(DSM, XAUI0_FC_FRAME_DMAC_HI, 0, xaui->fc.ib.dmac.addr[0] << 16 |
                                              xaui->fc.ib.dmac.addr[1] << 8  |
                                              xaui->fc.ib.dmac.addr[2] << 0  );
        /* DMAC Pattern MSB bits 23:0  */
        B2_WR(DSM, XAUI0_FC_FRAME_DMAC_LO, 0, xaui->fc.ib.dmac.addr[3] << 16 |
                                              xaui->fc.ib.dmac.addr[4] << 8  |
                                              xaui->fc.ib.dmac.addr[5] << 0  );
        /* 16-bit EtherType */
        B2_WR(DSM, XAUI0_FC_FRAME_ETYPE, 0, xaui->fc.ib.etype); 

        /* Only write to the specific fields in the DSM::XAUI0_INB_LPORT_CFG register  */
        mask = 0x3f << VTSS_OFF_DSM_XAUI0_INB_LPORT_CFG_XAUI0_DSTN_FC_ID |
               0x1f << VTSS_OFF_DSM_XAUI0_INB_LPORT_CFG_XAUI0_DSTN_SLOT_ID;
                
        /* Destination FC and SLOT ID  */
        for(aport=0; aport <= 23; aport++) {

            if (host_mode == VTSS_HOST_MODE_0 || host_mode==VTSS_HOST_MODE_8 || host_mode==VTSS_HOST_MODE_10) { 
                lport = aport; /*  XAUI0_DSTN_FC_ID range 0-23   */
            } else if (host_mode == VTSS_HOST_MODE_3) {
                if (aport > 11) {
                    lport = 0x3f; /*  logical port is disabled */
                } else {
                    lport = aport; /*  XAUI0_DSTN_FC_ID range 0-11   */
                }
            } else if (host_mode == VTSS_HOST_MODE_9) {
                if (aport > 8) {
                    lport = 0x3f; /*  logical port is disabled */
                } else {
                    lport = aport; /*  XAUI0_DSTN_FC_ID range 0-7   */
                }
            } 
            
            if ((vtss_state->port.dep_port[aport]) == -1) 
                lport = 0x3f; /*  logical port is disabled */
          
            value = lport << VTSS_OFF_DSM_XAUI0_INB_LPORT_CFG_XAUI0_DSTN_FC_ID |
                        0 << VTSS_OFF_DSM_XAUI0_INB_LPORT_CFG_XAUI0_DSTN_SLOT_ID;

            B2_WRXM(DSM, XAUI0_INB_LPORT_CFG, aport, value, mask);
            if (host_mode == VTSS_HOST_MODE_10) {
                /* Need also to write to XAUI1  */
                value = (lport+24) << VTSS_OFF_DSM_XAUI1_INB_LPORT_CFG_XAUI1_DSTN_FC_ID |
                                 0 << VTSS_OFF_DSM_XAUI1_INB_LPORT_CFG_XAUI1_DSTN_SLOT_ID;
                B2_WRXM(DSM, XAUI1_INB_LPORT_CFG, aport, value, mask);
            }
        }
    } else {
        /* XAUI_1  */
        B2_WR(DSM, XAUI1_TX_FC_FRAME_EN, 0, xaui->fc.ib.enable);
        B2_WR(DSM, XAUI1_SOURCE_SLOT_ID, 0, 1); /* XAUI_1:1 */

        B2_WR(DSM, XAUI1_FC_FRAME_DMAC_HI, 0, xaui->fc.ib.dmac.addr[0] << 16 |
                                              xaui->fc.ib.dmac.addr[1] << 8  |
                                              xaui->fc.ib.dmac.addr[2] << 0  );
        B2_WR(DSM, XAUI1_FC_FRAME_DMAC_LO, 0, xaui->fc.ib.dmac.addr[3] << 16 |
                                              xaui->fc.ib.dmac.addr[4] << 8  |
                                              xaui->fc.ib.dmac.addr[5] << 0  );
        B2_WR(DSM, XAUI1_FC_FRAME_ETYPE, 0, xaui->fc.ib.etype);               

        mask = 0x3f << VTSS_OFF_DSM_XAUI1_INB_LPORT_CFG_XAUI1_DSTN_FC_ID |
               0x1f << VTSS_OFF_DSM_XAUI1_INB_LPORT_CFG_XAUI1_DSTN_SLOT_ID;

        for (aport=0; aport <= 23; aport++) {
            if (host_mode == VTSS_HOST_MODE_1 || host_mode==VTSS_HOST_MODE_8 || host_mode==VTSS_HOST_MODE_11) { 
                lport = aport+24; /*  XAUI1_DSTN_FC_ID range 24-47   */
            } else if (host_mode == VTSS_HOST_MODE_3) {
                if (aport < 12) {
                    lport = 0x3f; /*  logical port is disabled */
                } else {
                    lport = aport+12; /*  XAUI1_DSTN_FC_ID range 24-35   */
                }
            } else if (host_mode == VTSS_HOST_MODE_9) {
                if (aport > 8) {
                    lport = 0x3f; /*  logical port is disabled */
                } else {
                    lport = aport+24; /*  XAUI1_DSTN_FC_ID range 0-7   */
                }
            } 

            if ((vtss_state->port.dep_port[aport]) == -1) {
                lport = 0x3f; /*  logical port is disabled */
            }

            value = lport << VTSS_OFF_DSM_XAUI1_INB_LPORT_CFG_XAUI1_DSTN_FC_ID |
                        1 << VTSS_OFF_DSM_XAUI1_INB_LPORT_CFG_XAUI1_DSTN_SLOT_ID;

            B2_WRXM(DSM, XAUI1_INB_LPORT_CFG, aport, value, mask);
            if (host_mode == VTSS_HOST_MODE_11) {
                /* Need also to write to XAUI0  */
                value = (lport-24) << VTSS_OFF_DSM_XAUI0_INB_LPORT_CFG_XAUI0_DSTN_FC_ID |
                                 1 << VTSS_OFF_DSM_XAUI0_INB_LPORT_CFG_XAUI0_DSTN_SLOT_ID;
                B2_WRXM(DSM, XAUI0_INB_LPORT_CFG, aport, value, mask);
            }

        }
    }
    
    /* ASM / DMAC match MSB bits 47:24  */
    i = (port == CHIP_PORT_10G_0 ? 0 : 1);
    B2_WRX(ASM, FC_DMAC_HIGH_CFG, i, 
           xaui->fc.ib.dmac.addr[0] << 16 |
           xaui->fc.ib.dmac.addr[1] << 8  |
           xaui->fc.ib.dmac.addr[2] << 0  );    

    /* ASM / DMAC match MSB bits 23:0  */
    B2_WRX(ASM, FC_DMAC_LOW_CFG, i,  
           xaui->fc.ib.dmac.addr[3] << 16 |
           xaui->fc.ib.dmac.addr[4] << 8  |
           xaui->fc.ib.dmac.addr[5] << 0  );    

    /* ASM / 16-bit Etype match   */
    B2_WRX(ASM, FC_ETYPE_CFG, i, xaui->fc.ib.etype);
        
    /* Only write to the specific fields in the ASM register */
    mask = 0x1f << VTSS_OFF_ASM_FC_FRAME_CFG_DSTN_SLOT_ID |
           3 << VTSS_OFF_ASM_FC_FRAME_CFG_FRAME_MATCH_ENA;
    
    /* DSTN_SLOT_ID = 0 or 1,   FRAME_MATCH_EN = DMAC_and_ETYPE (3)   */
    value = (i) << VTSS_OFF_ASM_FC_FRAME_CFG_DSTN_SLOT_ID |
            3 << VTSS_OFF_ASM_FC_FRAME_CFG_FRAME_MATCH_ENA;
       
    /* ASM / The FC frame matching criteria */
    B2_WRXM(ASM, FC_FRAME_CFG, i, value, mask);

    /* Configure the Pause Value     */
    B2_WRX(DSM, MAC_CFG, port, 
           (xaui->fc.ib.pause_value << VTSS_OFF_DSM_MAC_CFG_TX_PAUSE_VAL) |
           (0 << VTSS_OFF_DSM_MAC_CFG_HDX_BACKPRESSURE) |
           (0 << VTSS_OFF_DSM_MAC_CFG_SEND_PAUSE_FRM_TWICE) |
           (1 << VTSS_OFF_DSM_MAC_CFG_TX_PAUSE_XON_XOFF));
            
    return VTSS_RC_OK;
}



/* Initialize XAUI */
static vtss_rc b2_xaui_conf(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_xaui_conf_t  *xaui;
    u32               i, count, offs;
    vtss_host_mode_t  host_mode;
    vtss_port_conf_t  conf;
    BOOL              both;
    vtss_port_no_t    p;
    u32               port = VTSS_CHIP_PORT(port_no);
    BOOL              fc_spi4, fc_xaui0, fc_xaui1 ;

    VTSS_D("Enter");
   
    host_mode = vtss_state->init_conf.host_mode;

    if (host_mode > VTSS_HOST_MODE_3 && host_mode < VTSS_HOST_MODE_8) {
        return VTSS_RC_OK; /* SPI4 mode */
    }
    /* XAUI setup */
    xaui = &vtss_state->port.xaui[port-CHIP_PORT_10G_0];

    /* Check revision C features */
    if (xaui->status_select != VTSS_XAUI_STATUS_BOTH)
        VTSS_RC(b2_rev_c_check(vtss_state, "single XAUI status channel"));
    if (xaui->status_select != VTSS_XAUI_STATUS_BOTH && 
        host_mode != VTSS_HOST_MODE_3 && host_mode != VTSS_HOST_MODE_8 && host_mode != VTSS_HOST_MODE_9) {
        VTSS_E("single XAUI status channel can only be used for host mode 3, 8 and 9");
        return VTSS_RC_ERROR;
    }
    if (xaui->fc.channel_enable && xaui->fc.ib.enable) {
        VTSS_E("Channel FC and IB FC cannot be enabled simultaneously");
        return VTSS_RC_ERROR;
    }

    /* Setup XAUI status channel and XAUI HIH format */
    B2_WR(DEVCPU_GCB, CHIP_MODE, 0,
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_ID_SEL) |
          ((xaui->status_select == VTSS_XAUI_STATUS_XAUI_1 ? 1 : 0) <<
           VTSS_OFF_DEVCPU_GCB_CHIP_MODE_XAUI_STATUS_CHANNEL_SEL) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_B_SRC_SEL) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_A_SRC_SEL) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_B_DRIVE_EN) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_A_DRIVE_EN) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_STD_PREAMBLE_ENA) |
          ((vtss_state->port.spi4.ob.frame_interleave ? 0 : 1) << 
           VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SPI4_INTERLEAVE_MODE) |
          ((xaui->hih.format == VTSS_HIH_PRE_SFD ? 1 : 0) << 
           VTSS_OFF_DEVCPU_GCB_CHIP_MODE_XAUI_TAG_FORM) |
          ((int)host_mode << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_HOST_MODE));
    
    /*  'vtss_state' must hold the same values for status_select and hih.format for both XAUI ports */
    vtss_state->port.xaui[port-CHIP_PORT_10G_0?0:1].status_select = xaui->status_select;
    vtss_state->port.xaui[port-CHIP_PORT_10G_0?0:1].hih.format = xaui->hih.format;

    /* 3 level flowcontrol */
    fc_spi4 = (vtss_state->port.spi4.fc.three_level_enable ? 1 : 0);
    fc_xaui0 = (vtss_state->port.xaui[0].fc.three_level_enable ? 1 : 0);
    fc_xaui1 = (vtss_state->port.xaui[1].fc.three_level_enable ? 1 : 0);
    B2_WR(QSS, FC_3LVL, 0,
          (2 << VTSS_OFF_QSS_FC_3LVL_DISABLED_PORT_FC) |
          (fc_spi4 << VTSS_OFF_QSS_FC_3LVL_SPI4_FC_3LVL) |
          (fc_xaui0 << VTSS_OFF_QSS_FC_3LVL_XAUI1_FC_3LVL) |
          (fc_xaui1 << VTSS_OFF_QSS_FC_3LVL_XAUI0_FC_3LVL));
    
    count = b2_lport_count(vtss_state);
    both = (xaui->status_select == VTSS_XAUI_STATUS_BOTH);
    if ((host_mode > VTSS_HOST_MODE_1 && host_mode < VTSS_HOST_MODE_10) && both)
        count /= 2; /* Both status channels used */    
    memset(&conf, 0, sizeof(conf));
    conf.if_type = VTSS_PORT_INTERFACE_XAUI;
    conf.speed = VTSS_SPEED_10G;
    conf.fdx = 1;
    conf.flow_control.obey = xaui->fc.obey_pause;
    conf.max_frame_length = VTSS_MAX_FRAME_LENGTH_MAX;
    VTSS_RC(b2_port_conf_10g_set(vtss_state, port, &conf));
    /* Store the host port conf in vtss_state */
    for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
        if (VTSS_CHIP_PORT(p) == port) {
            vtss_state->port.conf[p] = conf;
            break;
        }
    }
    
    /* Enable HIH checksum */
    i = (port - 24);
    B2_WRX(ASM, HIH_CFG, i,
           (xaui->hih.cksum_enable ? 1 : 0) << VTSS_OFF_ASM_HIH_CFG_HIH_CHK_ENA);

    /* Setup logical port map mode */
    B2_WRX(ASM, LPORT_MAP_CFG, i,
           (xaui->dmac_map_enable ? 0 : 1) << VTSS_OFF_ASM_LPORT_MAP_CFG_LPORT_MAP_MODE);

    /* Shaper */
    VTSS_RC(b2_shaper_setup(vtss_state, port, &xaui->qos.shaper));

    /* Inbound Xaui flowcontrol */ 
    VTSS_RC(b2_xaui_ibfc_setup(vtss_state, port, xaui));

    /* Source Mac address of Pause frames     */
    B2_WRX(DSM, MAC_ADDR_LOW_CFG, port, 
           port << VTSS_OFF_DSM_MAC_ADDR_LOW_CFG_MAC_ADDR_LOW);
        
    /* Status channel */
    if (host_mode < VTSS_HOST_MODE_3)
        offs = 24;
    else if (host_mode == VTSS_HOST_MODE_3)
        offs = (port == 25 && both ? 36 : 24);
    else if (host_mode == VTSS_HOST_MODE_8)
        offs = (port == 25 && both ? 24 : 0);
    else if (host_mode == VTSS_HOST_MODE_9)
        offs = (port == 25 && both ? 24 : 0);
    else /* Host mode 10/11 */
        offs = 0;
    for (i = 0; i < count; i++) {
        if (host_mode == VTSS_HOST_MODE_9 && i == 8 && !both)
            offs = 24;
        VTSS_RC(b2_wr(vtss_state, VTSS_TGT_XAUI_PHY_STAT, 
                      VTSS_ADDX_XAUI_PHY_STAT_XAUI_IBS_CAL(i), port,
                      (i + offs) << VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CAL_CONF_IBS_CAL_VAL));
        VTSS_RC(b2_wr(vtss_state, VTSS_TGT_XAUI_PHY_STAT, 
                      VTSS_ADDX_XAUI_PHY_STAT_XAUI_OBS_CAL(i), port,
                      (i + offs) << VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_CAL_CONF_OBS_CAL_VAL));
    }
    B2_WR(XAUI_PHY_STAT, XAUI_IBS_CFG, port, 
          (0 << VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CLK_SOURCE) |
          ((xaui->ib.clock_shift ? 1 : 0) << 
           VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CLK_SHIFT) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_FORCE_IDLE) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CAL_M) |
          (count << VTSS_OFF_XAUI_PHY_STAT_XAUI_IBS_CFG_CONF_IBS_CAL_LEN));
    B2_WR(XAUI_PHY_STAT, XAUI_OBS_CFG, port, 
          ((xaui->ob.clock_shift ? 1 : 0) << 
           VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_CFG_CONF_OBS_CLK_SHIFT) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_CFG_CONF_OBS_CAL_M) |
          (count << VTSS_OFF_XAUI_PHY_STAT_XAUI_OBS_CFG_CONF_OBS_CAL_LEN));
    B2_WR(XAUI_PHY_STAT, XAUI_SB_CONFIG, port, 
          (0 << VTSS_OFF_XAUI_PHY_STAT_XAUI_SB_CONFIG_SB_INTERPRET_01) |
          (0 << VTSS_OFF_XAUI_PHY_STAT_XAUI_SB_CONFIG_DIP_2_ERROR_INT_MASK) |
          ((both ? 0 : 1) << 
           VTSS_OFF_XAUI_PHY_STAT_XAUI_SB_CONFIG_STATUS_CHANNEL_AGGREGATION_MODE));
    
    VTSS_D("Exit");
    return VTSS_RC_OK;
}    

static vtss_rc b2_host_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    u32     port = VTSS_CHIP_PORT(port_no);
   
    /* Setup flow control */
    VTSS_RC(b2_host_flow_ctrl(vtss_state));

    if (VTSS_PORT_IS_10G(port))
        return b2_xaui_conf(vtss_state, port_no);
    else if (port == CHIP_PORT_SPI4)
        return b2_spi4_conf(vtss_state);
    else
        return VTSS_RC_ERROR;
}

/* Setup port map */
static vtss_rc b2_port_map_set(vtss_state_t *vtss_state)
{
    vtss_port_no_t   port_no;
    int              chip_port;
    vtss_host_mode_t host_mode;
    u32              aport, dport;
    vtss_lport_no_t  lport_no, max_port;

    host_mode = vtss_state->init_conf.host_mode;
    
    
     /* Converter/aggregator mode, fixed host port mapping */
    if (host_mode > VTSS_HOST_MODE_6)
        return VTSS_RC_OK;
    

    /* Check and store host port numbers */
    max_port = (host_mode == VTSS_HOST_MODE_6 ? 1 : 23);
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        chip_port = VTSS_CHIP_PORT(port_no);
        if (chip_port < 0 || b2_port_is_host(vtss_state, chip_port))
            continue;
        lport_no = vtss_state->port.map[port_no].lport_no;
        if (lport_no == VTSS_LPORT_MAP_DEFAULT) /* Default mapping */
            lport_no = (port_no - VTSS_PORT_NO_START);
        if (lport_no > max_port) {
            VTSS_E("illegal lport_no: %d for port_no: %d, host mode: %u", 
                   lport_no, port_no, host_mode);
            return VTSS_RC_ERROR;
        }
        vtss_state->port.map[port_no].lport_no = lport_no;
        aport = b2_port2line(chip_port);
        dport = (lport_no + (host_mode < VTSS_HOST_MODE_4 ? 24 : 48));
        vtss_state->port.dep_port[aport] = dport;
#ifdef BOARD_B2_EVAL
        /* For loopback test, 0<->1, 2<->3, ... , 22<->23 */
        aport = ((aport & 1) ? (aport - 1) : (aport + 1)); 
#endif /* BOARD_B2_EVAL */
        vtss_state->port.dep_port[dport] = aport;

        VTSS_D("API port:%u->chip port:%d->arr_port:%d->dep_port:%d. Arr_port:%d->dep_port:%d",
               port_no,chip_port,b2_port2line(chip_port),dport,dport,vtss_state->port.dep_port[dport]);

    }  
        
    /* Update chip mappings */   
    if (host_mode < VTSS_HOST_MODE_4) /* Only for XAUI modes */
        VTSS_RC(b2_qss_setup(vtss_state));
    
    return VTSS_RC_OK;
}


/* Set port forwarding state */
static vtss_rc b2_port_forward_state_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    int rx, tx, queue;
    vtss_host_mode_t host_mode;
    uint port, aport, dport, iport;

    host_mode = vtss_state->init_conf.host_mode;
    if (host_mode > VTSS_HOST_MODE_6) /* Converter/aggregator mode always forwarding */
        return VTSS_RC_OK;

    // Port forwarding not supported for host ports
    if (b2_port_is_host(vtss_state, port_no)) {
      return VTSS_RC_ERROR; 
    }
    
    /* Arrival and departure ports */
    port = VTSS_CHIP_PORT(port_no);
    aport = b2_port2line(port);
    dport = vtss_state->port.dep_port[aport];
    
    rx = VTSS_PORT_RX_FORWARDING(vtss_state->port.forward[port_no]);
    
    if (rx) {
        /* Enable Rx queues */
        B2_WRX(QSS, PORT_ENA, dport, rx << VTSS_OFF_QSS_PORT_ENA_PORT_ENA);
    }

    /* Enable/disable arrival port in Rx direction */
    B2_WRX(QSS, APORT_ENA_CLASS_MAP, aport,
           (rx << VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_ENA) |
           (0 << VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_CLASS) |
           (dport << VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_MAP));

    if (!rx) {
        /* Disable and flush Rx queues */
        B2_WRX(QSS, PORT_ENA, dport, rx << VTSS_OFF_QSS_PORT_ENA_PORT_ENA);
        for (queue = 0; queue < VTSS_QUEUES; queue++) {
            VTSS_RC(b2_queue_flush(vtss_state, dport, queue));
        }
    }
    
    tx = VTSS_PORT_TX_FORWARDING(vtss_state->port.forward[port_no]);
    if (tx) {
        /* Enable Tx queue */
        B2_WRX(QSS, PORT_ENA, aport, tx << VTSS_OFF_QSS_PORT_ENA_PORT_ENA);
    }
    
    /* Enable/disable arrival port in Tx direction */
    for (iport = 0; iport < VTSS_INT_PORT_COUNT; iport++) {
        if (vtss_state->port.dep_port[iport] == aport) {
            B2_WRX(QSS, APORT_ENA_CLASS_MAP, iport,
                   (tx << VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_ENA) |
                   (0 << VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_CLASS) |
                   (aport << VTSS_OFF_QSS_APORT_ENA_CLASS_MAP_APORT_MAP));
        }
    }
    
    if (!tx) {
        /* Disable and flush Tx queue */
        B2_WRX(QSS, PORT_ENA, aport, tx << VTSS_OFF_QSS_PORT_ENA_PORT_ENA);
        VTSS_RC(b2_queue_flush(vtss_state, aport, 0));
    }
    
    return VTSS_RC_OK;
}


/* Setup PSI */
static vtss_rc b2_psi_set(vtss_state_t *vtss_state, const u32 clock)
{

 // See PSI Configuration Register in datasheet for understanding this function.
  u32 value, psi_enable;
    
  // Calculate register value
  if (clock == 0) {
    value = 0;
    psi_enable = 0;
  } else {
    psi_enable = 1;
    value = (312500000/(32*clock)) - 1;
  }

  VTSS_D("value = %u",value);

  // Give error message if invalid clock value
  if (value > 255) {
    VTSS_E("PSI clock out of range - Setting to minimum clock frequency");
    value = 255;
  }


  // register write
  B2_WR(FAST_REGS, PSI_CFG, 0, 
        (value << VTSS_OFF_FAST_REGS_PSI_CFG_PSI_CFG_PRESCALE) |
        (psi_enable << VTSS_OFF_FAST_REGS_PSI_CFG_PSI_ENABLE));
  
  return VTSS_RC_OK;
}


/* Get PSI clock */
static vtss_rc b2_psi_get(vtss_state_t *vtss_state, u32 *clock)
{
  // See PSI Configuration Register in datasheet for understanding this function.
  
  u32 value;

  // Read the PSI 2374 Configuration register
  B2_RD(FAST_REGS, PSI_CFG, 0, &value);

  // Pick out the PSI_CFG_PRESCALE bits
  value = value >> VTSS_OFF_FAST_REGS_PSI_CFG_PSI_CFG_PRESCALE;

  // System clock = 312.5 MHz - 
  *clock = 312500000/(32*(1+value));
  
  return VTSS_RC_OK;
}

/* Get the 100FX port status    */
static vtss_rc b2_100fx_status_get(vtss_state_t *vtss_state,
                                   const vtss_port_no_t  port_no,
                                   vtss_port_status_t *status)
{
    u32 value;
    uint port = VTSS_CHIP_PORT(port_no);

    if (vtss_state->port.conf[port_no].power_down) {
        memset(status,0,sizeof(*status));
        return VTSS_RC_OK;
    }

    /* Get the link state 'down' sticky bit  */
    B2_RDF(DEV1G, FX100_STICKY, FX100_LINK_DOWN_STICKY, port, &value);        
    if (VTSS_BOOL(value)) {
        /* The link has been down. Clear the sticky bit and return the 'down' value  */
        B2_WRF(DEV1G, FX100_STICKY, FX100_LINK_DOWN_STICKY, port, 1);
    }        
        
    status->link_down = VTSS_BOOL(value);
    B2_RDF(DEV1G, FX100_RX_STAT, LINK_DOWN, port, &value);
    status->link = VTSS_BOOL(value) ? 0 : 1;;
    status->speed = VTSS_SPEED_100M;
    status->fdx = 1;

    return VTSS_RC_OK;
}

/* Get status of the XAUI ports */
static vtss_rc b2_10g_status_get(vtss_state_t *vtss_state,
                                 vtss_port_no_t port_no, vtss_port_status_t * const status)
{
    uint port = VTSS_CHIP_PORT(port_no);
    u32 value;
    
    VTSS_N("port_no: %u, port: %u", port_no, port);

    status->fdx = 1;
    status->speed = vtss_state->port.conf[port_no].speed;

    B2_RDF(DEV10G, PCS10G_RX_LANE_STAT, LANE_SYNC, port, &value);
    status->link = (value == 0xf ? 1 : 0);

    /* TBD: MAC_TX_MONITOR is currently debug field */
    VTSS_RC(b2_rdf(vtss_state, VTSS_TGT_DEV10G, 0x15, 0, 1, port, &value));
    status->remote_fault = (value ? 1 : 0);
   
    return VTSS_RC_OK;
}

/* Get status of the XAUI or 100FX ports */
static vtss_rc b2_port_status_get(vtss_state_t *vtss_state,
                                  const vtss_port_no_t  port_no, 
                                  vtss_port_status_t    *const status)
{
    vtss_port_conf_t *conf = &vtss_state->port.conf[port_no];

    if (conf->if_type == VTSS_PORT_INTERFACE_100FX) {
        return b2_100fx_status_get(vtss_state, port_no, status);
    }  else if (conf->if_type == VTSS_PORT_INTERFACE_XAUI) {
        return b2_10g_status_get(vtss_state, port_no, status);
    } else 
        VTSS_E("Status not supported for port:%u ",port_no);

    return VTSS_RC_ERROR;
}

/* Set PCS autoneg capabilities and enable/disable aneg */
static vtss_rc b2_port_clause_37_control_set(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no)
{
    u32 value;
    vtss_port_clause_37_control_t *control = &vtss_state->port.clause_37[port_no];
    vtss_port_clause_37_adv_t *adv = &control->advertisement;
    uint port = VTSS_CHIP_PORT(port_no);
       
    value =  (((adv->next_page ? 1 : 0)<<15) |
              ((adv->acknowledge ? 1 : 0)<<14) |
              ((int)adv->remote_fault<<12) |
              ((adv->asymmetric_pause ? 1 : 0)<<8) |
              ((adv->symmetric_pause ? 1 : 0)<<7) |
              ((adv->hdx ? 1 : 0)<<6) |
              ((adv->fdx ? 1 : 0)<<5));
     
    /* Set aneg capabilities                   */
    B2_WRF(DEV1G, PCS1G_ANEG_CFG, DAR, port, value);

    if (!control->enable) {
        /* Restart aneg before disabling it */
        B2_WRF(DEV1G, PCS1G_ANEG_CFG, ANEG_ENA, port, 1);
        B2_WRF(DEV1G, PCS1G_ANEG_CFG, ANEG_RESTART, port, 1);
    } 

    /* Enable/Disable aneg and restart                 */
    B2_WRF(DEV1G, PCS1G_ANEG_CFG, ANEG_ENA, port, control->enable);
    B2_WRF(DEV1G, PCS1G_ANEG_CFG, ANEG_RESTART, port, 1); 

    return VTSS_RC_OK;
}

/* Restart PCS aneg */
//static vtss_rc b2_port_clause_37_aneg_restart(uint port_no)
//{
//    uint port = VTSS_CHIP_PORT(port_no);
//    B2_WRF(DEV1G, PCS1G_ANEG_CFG, ANEG_RESTART, port, 1);
//
//    return VTSS_RC_OK;
//}

/* Get the Serdes PCS status */
static vtss_rc b2_port_clause_37_status_get(vtss_state_t *vtss_state,
                                            const vtss_port_no_t         port_no, 
                                            vtss_port_clause_37_status_t *const status)

{
    u32 value;
    uint port = VTSS_CHIP_PORT(port_no);

    if (vtss_state->port.conf[port_no].power_down) {
        status->link = 0;
        return VTSS_RC_OK;
    }

    /* Get the link state 'down' sticky bit  */
    B2_RDF(DEV1G, PCS1G_STICKY, LINK_DOWN_STICKY, port, &value);

    if (VTSS_BOOL(value)) {
        /* The link has been down. Clear the sticky bit and return the 'down' value  */
        B2_WRF(DEV1G, PCS1G_STICKY, LINK_DOWN_STICKY, port, 1);
    } else {
        /*  Return the current status     */
        B2_RDF(DEV1G, PCS1G_LINK_STATUS, LINK_DOWN, port, &value);
    }
    status->link = VTSS_BOOL(value) ? 0 : 1;
    
    /* Get PCS ANEG status register */
    B2_RD(DEV1G, PCS1G_ANEG_STATUS, port, &value);
    
    /* Aneg complete   */
    status->autoneg.complete = VTSS_BOOL(value & (1<<0));
  
    /* Get Link partner aneg status */
    B2_RDF(DEV1G, PCS1G_ANEG_STATUS, LPA, port, &value);
    status->autoneg.partner_advertisement.fdx =             VTSS_BOOL(value & (1<<5));
    status->autoneg.partner_advertisement.hdx =             VTSS_BOOL(value & (1<<6));
    status->autoneg.partner_advertisement.symmetric_pause = VTSS_BOOL(value & (1<<7));
    status->autoneg.partner_advertisement.asymmetric_pause =VTSS_BOOL(value & (1<<8));
    status->autoneg.partner_advertisement.remote_fault =    (vtss_port_clause_37_remote_fault_t)((value>>12)&0x3);
    status->autoneg.partner_advertisement.acknowledge =     VTSS_BOOL(value & (1<<14));
    status->autoneg.partner_advertisement.next_page =       VTSS_BOOL(value & (1<<15));

    return VTSS_RC_OK;
}


static vtss_rc b2_vid2port_set(vtss_state_t *vtss_state,
                               const vtss_vid_t     vid, 
                               const vtss_port_no_t port_no)
{
    vtss_host_mode_t host_mode;
    u32 enabled = 0, offset = 0, xaui = 0;

    VTSS_RC(b2_rev_c_check(vtss_state, "VID to line port mapping"));

    host_mode = vtss_state->init_conf.host_mode;
    if (host_mode > VTSS_HOST_MODE_3) {
        VTSS_E(("mapping is only allowed in host mode 0, 1 and 3"));
        return VTSS_RC_ERROR;
    }
    
    if (port_no != VTSS_PORT_NO_NONE) {
        enabled = 1;
        offset = vtss_state->port.map[port_no].lport_no;
        if (host_mode == VTSS_HOST_MODE_3 && offset > 11) {
            offset -= 12;
            xaui = 1;
        }
        if (host_mode == VTSS_HOST_MODE_1)
            xaui = 1;
    }

    VTSS_D("port = %d, enabled =%d, offset = %d, zaui = %d, host_mode =%d, vid =%d, vtss_state->port.map[port_no].lport_no = %d",
           port_no,enabled,offset,xaui,host_mode,vid,vtss_state->port.map[port_no].lport_no);

    B2_WRX(ASM, PORT_LUT_CFG, vid, 
           (offset << VTSS_OFF_ASM_PORT_LUT_CFG_CELLBUS_PORT_OFFSET) |
           (xaui << VTSS_OFF_ASM_PORT_LUT_CFG_HOST_PORT_NUM_ENA) |
           (enabled << VTSS_OFF_ASM_PORT_LUT_CFG_OFFSET_ENA));

    B2_WRF(DEVCPU_GCB, CHIP_MODE, STD_PREAMBLE_ENA, 0, enabled);

    return VTSS_RC_OK;
}

static vtss_rc b2_vid2lport_set(vtss_state_t *vtss_state,
                                const vtss_vid_t      vid, 
                                const vtss_lport_no_t lport_no)
{
    vtss_host_mode_t host_mode;
    u32 enabled = 0, offset = 0, xaui = 0;

    VTSS_RC(b2_rev_c_check(vtss_state, "VID to logical port mapping"));

    host_mode = vtss_state->init_conf.host_mode;
    if (host_mode < VTSS_HOST_MODE_8 || host_mode == VTSS_HOST_MODE_9) {
        VTSS_E(("mapping is only allowed in host mode 8, 10 and 11"));
        return VTSS_RC_ERROR;
    }

    if (lport_no != VTSS_LPORT_NONE) {
        enabled = 1;
        offset = lport_no;
        if (offset > 23) {
            offset -= 24;
            xaui = 1;
        }
    }
    B2_WRX(ASM, PORT_LUT_CFG, vid, 
           (offset << VTSS_OFF_ASM_PORT_LUT_CFG_CELLBUS_PORT_OFFSET) |
           (xaui << VTSS_OFF_ASM_PORT_LUT_CFG_HOST_PORT_NUM_ENA) |
           (enabled << VTSS_OFF_ASM_PORT_LUT_CFG_OFFSET_ENA));

    B2_WRF(DEVCPU_GCB, CHIP_MODE, STD_PREAMBLE_ENA, 0, enabled);

    return VTSS_RC_OK;
}

/* Set QoS setup for logical port */
static vtss_rc b2_qos_lport_conf_set(vtss_state_t *vtss_state, const vtss_lport_no_t lport_no)
{
    vtss_lport_no_t count;
    vtss_host_mode_t host_mode;
    vtss_qos_lport_conf_t *qos;
    u32 w;

    qos = &vtss_state->qos.lport[lport_no];
    
    /* Ignore call for 1G/10G MAC modes */
    host_mode  = vtss_state->init_conf.host_mode;
    if (host_mode < VTSS_HOST_MODE_8)
        return VTSS_RC_OK;
    
    /* Check against logical port range */
    count = (host_mode == VTSS_HOST_MODE_9 ? 16 : 48);
    if (lport_no >= count) {
        VTSS_E("illegal lport_no: %u", lport_no);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(b2_scheduler_check(vtss_state));
    VTSS_RC(b2_weight(vtss_state, qos->scheduler.rx_rate, &w));
    B2_WRX(SCH, SPI4_PS_WEIGHT, lport_no, w << VTSS_OFF_SCH_SPI4_PS_WEIGHT_SPI4_PS_WEIGHT);

    if (vtss_state->port.spi4.ob.frame_interleave == 0) { 
        /* Burst interleave */
        B2_WRX(DSM, SPI4_BS_WEIGHT, lport_no, w << VTSS_OFF_DSM_SPI4_BS_WEIGHT_SPI4_BS_WEIGHT);
    }
    
    VTSS_RC(b2_weight(vtss_state, qos->scheduler.tx_rate, &w));
    if (lport_no < (count/2)) {
        B2_WRX(SCH, XAUI0_PS_WEIGHT, lport_no, 
               w << VTSS_OFF_SCH_XAUI0_PS_WEIGHT_XAUI0_PS_WEIGHT);
    } else {
        B2_WRX(SCH, XAUI1_PS_WEIGHT, lport_no, 
               w << VTSS_OFF_SCH_XAUI1_PS_WEIGHT_XAUI1_PS_WEIGHT);
    }
    
    /* Shaper configuration for logical ports  */
    VTSS_RC(b2_shaper_lport_setup(vtss_state, lport_no, &qos->shaper));    

    return VTSS_RC_OK;
}

#if defined(VTSS_GPIOS)

/* ================================================================= *
 *  GPIO's
 * ================================================================= */

/* Set GPIO mode */
static vtss_rc b2_gpio_mode(vtss_state_t *vtss_state,
                            const vtss_chip_no_t  chip_no, 
                            const vtss_gpio_no_t  gpio_no, 
                            const vtss_gpio_mode_t mode)
{
    vtss_rc rc = VTSS_RC_OK;

    switch(mode) {
    case VTSS_GPIO_OUT:
    case VTSS_GPIO_IN:
        VTSS_RC(b2_wrf(vtss_state, VTSS_TGT_DEVCPU_GCB, VTSS_ADDR_DEVCPU_GCB_GPIO_OUTPUT_ENA, gpio_no, 1, 0, mode == VTSS_GPIO_OUT ? 1 : 0));
        break;
    default:
        rc = VTSS_RC_ERROR; /* Other modes not supported */
    }
    return rc;
}

/* Read GPIO input pin */
static vtss_rc b2_gpio_read(vtss_state_t         *vtss_state,
                            const vtss_chip_no_t chip_no, 
                            const vtss_gpio_no_t gpio_no, 
                            BOOL                 *const value)
{
    u32 val;

    /* Chip version -01 */
    if (gpio_no == 0 || gpio_no == 1) {
        VTSS_E("gpio_no %d can only be used as output",gpio_no);
        return VTSS_RC_ERROR;
    }

    if (gpio_no < VTSS_GPIO_NO_END) {
        VTSS_RC(b2_rdf(vtss_state, VTSS_TGT_DEVCPU_GCB, VTSS_ADDR_DEVCPU_GCB_GPIO_I, gpio_no, 1, 0, &val));
        *value = VTSS_BOOL(val);
        return VTSS_RC_OK;
    } 
    VTSS_E("illegal gpio_no: %d",gpio_no);
    return VTSS_RC_ERROR;
}

/* Write GPIO output pin */
static vtss_rc b2_gpio_write(vtss_state_t         *vtss_state,
                             const vtss_chip_no_t chip_no, 
                             const vtss_gpio_no_t gpio_no, 
                             const BOOL           value)
{
    if (gpio_no < VTSS_GPIO_NO_END) {
        VTSS_RC(b2_wrf(vtss_state, VTSS_TGT_DEVCPU_GCB, VTSS_ADDR_DEVCPU_GCB_GPIO_O, gpio_no, 1, 0, value));
        return VTSS_RC_OK;
    }
    
    VTSS_E("illegal gpio_no: %d",gpio_no);
    return VTSS_RC_ERROR;
}

/* Use GPIO pin for clock output  */
static vtss_rc b2_gpio_clk_set(vtss_state_t                 *vtss_state,
                               const vtss_gpio_no_t         gpio_no, 
                               const vtss_port_no_t         port_no,
                               const vtss_recovered_clock_t clk)
{
    uint chip_port, p, port, clk_div;

    /* Feature only supported in Chip Rev C  */
    VTSS_RC(b2_rev_c_check(vtss_state, "recovered clock"));

    if (gpio_no != 0 && gpio_no != 1) {
        VTSS_E("Only GPIO 0 or GPIO 1 allowed");
        return VTSS_RC_ERROR;
    }

    if (port_no == VTSS_PORT_NO_NONE) {
        /*  Set GPIO pin to normal GPIO role */
        if (gpio_no == 0) {
            B2_WRF(DEVCPU_GCB, CHIP_MODE, SYNC_E_CLK_A_DRIVE_EN, 0, 0); /* Use GPIO_0 pin as GPIO_0 */ 
            for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
                port = VTSS_CHIP_PORT(p);                                       
                if (VTSS_PORT_IS_1G(port))
                    B2_WRF(DEV1G, DEV_RST_CTRL, CLK_A_DRIVE_EN, port, 0);                           
            }
        } else {
            B2_WRF(DEVCPU_GCB, CHIP_MODE, SYNC_E_CLK_B_DRIVE_EN, 0, 0); /* Use GPIO_1 pin as GPIO_1 */        
            for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
                port = VTSS_CHIP_PORT(p);                                       
                if (VTSS_PORT_IS_1G(port))
                    B2_WRF(DEV1G, DEV_RST_CTRL, CLK_B_DRIVE_EN, port, 0);        
            }
        }
        return VTSS_RC_OK;
    }

    /* Set the GPIO to output direction */
    VTSS_RC(b2_gpio_mode(vtss_state, 0, gpio_no, VTSS_GPIO_OUT));

    chip_port = VTSS_CHIP_PORT(port_no);

    if(VTSS_PORT_IS_10G(chip_port)) {

        if (clk == VTSS_RECOVERED_CLOCK_156_25) {
            clk_div = 0; /* Divide by 2 */
        } else if (clk == VTSS_RECOVERED_CLOCK_78_125) {
            clk_div = 1; /* Divide by 4 */
        } else if (clk == VTSS_RECOVERED_CLOCK_39_06) {
            clk_div = 2; /* Divide by 8 */
        } else if (clk == VTSS_RECOVERED_CLOCK_31_25) {
            clk_div = 3; /* Divide by 10 */ 
        } else {
            VTSS_E("illegal clock value for this port speed");
            return VTSS_RC_ERROR;
        }
        /* Divide the 10G clock     */
        B2_WRF(DEV10G, DEV_RST_CTRL, CLK_DIVIDE_SEL, chip_port, clk_div);
        /* Enable port clock drive */
        B2_WRF(DEV10G, DEV_RST_CTRL, CLK_DRIVE_EN, chip_port, 1);      
        
        if (gpio_no == 0) {
            /* GPIO_0  */
            B2_WRF(DEVCPU_GCB, CHIP_MODE, SYNC_E_CLK_A_SRC_SEL, 0, 
                   (chip_port == CHIP_PORT_10G_0) ? 0 : 1);  /* Choose 10G port as source */
            B2_WRF(DEVCPU_GCB, CHIP_MODE, SYNC_E_CLK_A_DRIVE_EN, 0, 1); /* Use GPIO_0 pin as CLKOUT */
        } else {
            /* GPIO_1  */
            B2_WRF(DEVCPU_GCB, CHIP_MODE, SYNC_E_CLK_B_SRC_SEL, 0, 
                   (chip_port == CHIP_PORT_10G_0) ? 0 : 1);  /* Choose 10G port as source */
            B2_WRF(DEVCPU_GCB, CHIP_MODE, SYNC_E_CLK_B_DRIVE_EN, 0, 1); /* Use GPIO_1 pin as CLKOUT */
        }
    } else if (VTSS_PORT_IS_1G(chip_port)) {
        if (clk == VTSS_RECOVERED_CLOCK_125) {
            clk_div = 0; /* Undivided  */
        } else if (clk == VTSS_RECOVERED_CLOCK_62_5) {
            clk_div = 1; /* Divide by 2 */
        } else if (clk == VTSS_RECOVERED_CLOCK_31_25) {
            clk_div = 2; /* Divide by 4 */
        } else if (clk == VTSS_RECOVERED_CLOCK_25) {
            clk_div = 3; /* Divide by 5 */
        } else {
            VTSS_E("illegal clock value for this port speed");
            return VTSS_RC_ERROR;
        }
        /* Divide the 1G clock     */
        B2_WRF(DEV1G, DEV_RST_CTRL, CLK_DIVIDE_SEL, chip_port, clk_div);
      
        if (gpio_no == 0) {
            /* Disable clk drive enable A for all 1G ports           */
            for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
                port = VTSS_CHIP_PORT(p);                                       
                if (VTSS_PORT_IS_1G(port))
                    B2_WRF(DEV1G, DEV_RST_CTRL, CLK_A_DRIVE_EN, port, 0);        
            }
            B2_WRF(DEV1G, DEV_RST_CTRL, CLK_A_DRIVE_EN, chip_port, 1);  /* Enable clk drive */      
            B2_WRF(DEVCPU_GCB, CHIP_MODE, SYNC_E_CLK_A_SRC_SEL, 0, 2);  /* Choose 1G/A port as source */
            B2_WRF(DEVCPU_GCB, CHIP_MODE, SYNC_E_CLK_A_DRIVE_EN, 0, 1); /* Use GPIO_0 pin as CLKOUT */
        } else {
            /* Disable clk drive enable B for all 1G ports           */
            for (p = VTSS_PORT_NO_START; p < VTSS_PORT_NO_END; p++) {
                port = VTSS_CHIP_PORT(p);                                       
                if (VTSS_PORT_IS_1G(port))
                    B2_WRF(DEV1G, DEV_RST_CTRL, CLK_B_DRIVE_EN, port, 0);        
            }
            B2_WRF(DEV1G, DEV_RST_CTRL, CLK_B_DRIVE_EN, chip_port, 1);  /* Enable clk drive */            
            B2_WRF(DEVCPU_GCB, CHIP_MODE, SYNC_E_CLK_B_SRC_SEL, 0, 2);  /* Choose 1G/B port as source */
            B2_WRF(DEVCPU_GCB, CHIP_MODE, SYNC_E_CLK_B_DRIVE_EN, 0, 1); /* Use GPIO_1 pin as CLKOUT */
        }
    } else {
        VTSS_E("Port not supported for GPIO clk");
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}
#endif /* VTSS_GPIOS  */


/* ================================================================= *
 *  Interrupts
 * ================================================================= */
#if defined(VTSS_FEATURE_INTERRUPTS)
/* Set the interrupt masks      */
static vtss_rc b2_intr_mask_set(vtss_state_t *vtss_state, vtss_intr_t *mask)
{
  vtss_port_no_t port_no;
  u32            chip_port;    
  u32            reg_mask,value,val_tmp = 0;
  BOOL           if_100fx=0, if_xaui=0;  
  
  /*  Set link change interrupt on XAUI 100FX or 1000-X ports                     */
  /*  The interrupts are enabled on device level as well as for each sticky bit.  */
  /*  The sticky bits are initially cleared.                                      */
  for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {        
    chip_port = VTSS_CHIP_PORT(port_no);
    if_100fx = 0;
    if_xaui = 0;
    if (vtss_state->port.conf[port_no].if_type == VTSS_PORT_INTERFACE_100FX) {
      if_100fx = 1;
    } else if (vtss_state->port.conf[port_no].if_type == VTSS_PORT_INTERFACE_XAUI) {
      if_xaui = 1;
    } else if (vtss_state->port.conf[port_no].if_type != VTSS_PORT_INTERFACE_SERDES) {
      continue;
    } 
    
    /* Link change interrupt not supported for XAUI and 100FX in chip rev A/B     */
    if (if_xaui || if_100fx)
      VTSS_RC(b2_rev_c_check(vtss_state, "XAUI/100FX link change interrupt"));
    
    /* Enable/disable sticky bit interrupt mask   */
    if (if_xaui) {
      /* XAUI       */
      B2_WR(DEV10G, DEV_STICKY, chip_port, 0x3000000);   /* Clear the sticky */
      B2_WRF(DEV10G, DEV_INT_MASK, LINK_UP_MASK, chip_port, mask->link_change[port_no]);
      B2_WRF(DEV10G, DEV_INT_MASK, LINK_DOWN_MASK, chip_port, mask->link_change[port_no]);
    } else if (if_100fx) {
      /* 100-FX     */
      B2_WR(DEV1G, FX100_STICKY, chip_port, 0x3);  /* Clear the sticky */
      B2_WRF(DEV1G, FX100_INT_MASK, FX100_LINK_UP_INT_MASK, chip_port, mask->link_change[port_no]);
      B2_WRF(DEV1G, FX100_INT_MASK, FX100_LINK_DOWN_INT_MASK, chip_port, mask->link_change[port_no]);            
    } else {
      /* 1000-X     */
      B2_WR(DEV1G, PCS1G_STICKY, chip_port, 0x30); /* Clear the sticky */
      B2_WRF(DEV1G, PCS1G_INT_MASK, LINK_UP_INT_MASK, chip_port, mask->link_change[port_no]);
      B2_WRF(DEV1G, PCS1G_INT_MASK, LINK_DOWN_INT_MASK, chip_port, mask->link_change[port_no]);
    }
    
    /* Enable/disable dev1g/dev10g port device interrupts    */
    if (if_xaui) {
      if (port_no == 0) {
        reg_mask = 1 << VTSS_OFF_FAST_REGS_INT_ENABLE_3_XAUI_INT_ENABLE_0;
        value = mask->link_change[port_no] << VTSS_OFF_FAST_REGS_INT_ENABLE_3_XAUI_INT_ENABLE_0;
      } else {
        reg_mask = 1 << VTSS_OFF_FAST_REGS_INT_ENABLE_3_XAUI_INT_ENABLE_1;
        value = mask->link_change[port_no] << VTSS_OFF_FAST_REGS_INT_ENABLE_3_XAUI_INT_ENABLE_1;
      }
      B2_RD(FAST_REGS, INT_ENABLE_3, 0, &val_tmp);            
      reg_mask = 0xFFFF^reg_mask;
      value = value | (val_tmp & reg_mask);
      B2_WR(FAST_REGS, INT_ENABLE_3, 0, value);            
    } else {
      if (chip_port < 16) {
        reg_mask = 1 << (15-chip_port);
        value = mask->link_change[port_no] << (15-chip_port);
        B2_RD(FAST_REGS, INT_ENABLE_1, 0, &val_tmp);            
        reg_mask = 0xFFFF^reg_mask;
        value = value | (val_tmp & reg_mask);
        B2_WR(FAST_REGS, INT_ENABLE_1, 0, value);            
      } else {
        reg_mask = 1 << (23-chip_port);
        value = mask->link_change[port_no] << (23-chip_port);
        B2_RD(FAST_REGS, INT_ENABLE_2, 0, &val_tmp);            
        reg_mask = 0xFF^reg_mask;
        value = value | (val_tmp & reg_mask);
        B2_WR(FAST_REGS, INT_ENABLE_2, 0, value);            
      }            
    }
  }       
  return VTSS_RC_OK;
}

/* MAKEBOOL01(value): Convert BOOL value to 0 (false) or 1 (true). */
/* Use this to ensure BOOL values returned are always 1 or 0. */
#ifndef MAKEBOOL01
#define MAKEBOOL01(value) ((value)?1:0)
#endif

/* Get the status of the interrupt sources    */
static vtss_rc b2_intr_status_get(vtss_state_t *vtss_state, vtss_intr_t *status)
{
  vtss_port_no_t port_no;
  int            chip_port, intr_index=0;    
  u32            int_status, mask, value=0, intr=0;
  BOOL           dev1g_intr_en;
  BOOL           if_100fx=0, if_xaui=0;  

  /****  Get link change interrupt status XAUI, 100FX or 1000-X ports *****/    
  /* The status is checked on device level before checking each sticky bit */    
  /* Only sticky bits with the interrupt mask enabled are checked.         */    
  /* The sticky bits are cleared after reading.                            */    
  
  for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
    chip_port = VTSS_CHIP_PORT(port_no);
    
    if_100fx = 0;
    if_xaui = 0;
    if (vtss_state->port.conf[port_no].if_type == VTSS_PORT_INTERFACE_100FX) {
      if_100fx = 1;
    } else if (vtss_state->port.conf[port_no].if_type == VTSS_PORT_INTERFACE_XAUI) {
      if_xaui = 1;
    } else if (vtss_state->port.conf[port_no].if_type != VTSS_PORT_INTERFACE_SERDES) {            
      continue;
    } 
    
    /* Link change interrupt not supported for XAUI and 100FX in chip rev A/B     */
    if (if_xaui || if_100fx)
      VTSS_RC(b2_rev_c_check(vtss_state, "XAUI/100FX link change interrupt"));
        
    if (if_xaui) {
      /* Get the device level interrupt status   */
      B2_RD(FAST_REGS, INT_STATUS_3, 0, &int_status);
      /* Check if dev10g interrupt is enabled           */
      intr_index = (chip_port == 24)?2:1;
      if (MAKEBOOL01(int_status & intr_index)) {
        B2_RDF(DEV10G, DEV_INT_MASK, LINK_UP_MASK, chip_port, &mask);
        if(mask) {
          B2_RD(DEV10G, DEV_STICKY, chip_port, &value);
          B2_WR(DEV10G, DEV_STICKY, chip_port, 0x3000000);   /* Clear the sticky */
        }
        /* Return  Link up || link down sticky        */
        status->link_change[port_no] = MAKEBOOL01(value & (3<<24));
      }           
    } else {
      /* Check if dev1g interrupt is enabled            */

      if (port_no < 16) {
        /* Get the device level interrupt status   */
        B2_RD(FAST_REGS, INT_STATUS_1, 0, &int_status);
        intr_index = 0xf^chip_port;
        dev1g_intr_en = MAKEBOOL01(int_status & (1<<intr_index));
      } else {
        /* Get the device level interrupt status   */
        B2_RD(FAST_REGS, INT_STATUS_2, 0, &int_status);
        intr_index = 0x17^chip_port;
        dev1g_intr_en = MAKEBOOL01(int_status & (1<<intr_index));
      }
                  
      if (dev1g_intr_en) {                
        /* dev1g interrupt is enabled.  */
        
        if (if_100fx) {
          /* 100-FX  */
          B2_RDF(DEV1G, FX100_INT_MASK, FX100_LINK_UP_INT_MASK, chip_port, &mask);
          if(mask) {
            B2_RD(DEV1G, FX100_STICKY, chip_port, &intr);
            B2_WR(DEV1G, FX100_STICKY, chip_port, 0x3);  /* Clear the sticky */
            status->link_change[port_no] = MAKEBOOL01(intr & (3<<0));
          }
        } else {                    
          /* 1000-X  */
          B2_RDF(DEV1G, PCS1G_INT_MASK, LINK_UP_INT_MASK, chip_port, &mask);
          if(mask) {
            B2_RD(DEV1G, PCS1G_STICKY, chip_port, &intr);              
            B2_WR(DEV1G, PCS1G_STICKY, chip_port, 0x30); /* Clear the sticky */
            status->link_change[port_no] = MAKEBOOL01(intr & (3<<4));
          }
        }               
      } else {
        status->link_change[port_no] = 0;
      }
    }
  }
  return VTSS_RC_OK;
}

#endif //VTSS_FEATURE_INTERRUPTS
/* ================================================================= *
 *  Initialization
 * ================================================================= */

static vtss_rc b2_init_conf_set(vtss_state_t *vtss_state)
{
    u32              value;
    int              port;
    vtss_host_mode_t host_mode;
    vtss_init_conf_t *conf = &vtss_state->init_conf;

    /* Mark all internal ports as unused */
    for (port = 0; port < VTSS_INT_PORT_COUNT; port++)
        vtss_state->port.dep_port[port] = -1;

    /* Setup PI */
    if (conf->pi.cs_wait_ns > 200) {
        VTSS_E("illegal pi_cs_wait_ns: %u", conf->pi.cs_wait_ns);
        return VTSS_RC_ERROR;
    }
    value = ((conf->pi.cs_wait_ns/13) << VTSS_OFF_FAST_REGS_CFG_STATUS_2_PI_WAIT);
    B2_WR(FAST_REGS, CFG_STATUS_2, 0, value);
    
    /* Read chip ID */
    VTSS_RC(b2_chip_id_get(vtss_state, &vtss_state->misc.chip_id));

    /* Check host mode */
    host_mode = conf->host_mode;
    VTSS_D("host_mode: %d", host_mode);
    /*lint -e{568}*/
    if (host_mode < VTSS_HOST_MODE_0 ||
#if defined(VTSS_CHIP_SCHAUMBURG_II)
        host_mode == VTSS_HOST_MODE_3 || host_mode > VTSS_HOST_MODE_5 ||
#endif /* VTSS_CHIP_SCHAUMBURG_II */
#if defined(VTSS_CHIP_EXEC_1)
        host_mode < VTSS_HOST_MODE_6 ||
#endif /* VTSS_CHIP_EXEC_1 */
        host_mode > VTSS_HOST_MODE_11) {
        VTSS_E("illegal host_mode: %d", host_mode);
        return VTSS_RC_ERROR;
    }
    
    if (host_mode > VTSS_HOST_MODE_9)
        VTSS_RC(b2_rev_c_check(vtss_state, "host mode 10/11"));

    /* Reset chip */
    B2_WR(DEVCPU_GCB, DEVCPU_RST_REGS, 0, 
          (0 << VTSS_OFF_DEVCPU_GCB_DEVCPU_RST_REGS_AUTO_BIST_DISABLE) |
          (0 << VTSS_OFF_DEVCPU_GCB_DEVCPU_RST_REGS_MEMLOCK_ENABLE) |
          (1 << VTSS_OFF_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST) |
          (0 << VTSS_OFF_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_NON_CFG_RST));
    VTSS_MSLEEP(10);
    
    /* Setup PI again */
    B2_WR(FAST_REGS, CFG_STATUS_2, 0, value);

    /* Setup host mode */
    B2_WR(DEVCPU_GCB, CHIP_MODE, 0,
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_ID_SEL) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_XAUI_STATUS_CHANNEL_SEL) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_B_SRC_SEL) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_A_SRC_SEL) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_B_DRIVE_EN) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SYNC_E_CLK_A_DRIVE_EN) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_STD_PREAMBLE_ENA) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_SPI4_INTERLEAVE_MODE) |
          (0 << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_XAUI_TAG_FORM) |
          ((int)host_mode << VTSS_OFF_DEVCPU_GCB_CHIP_MODE_HOST_MODE));
    VTSS_MSLEEP(10);
    
    /* ANA_CL and ANA_AC initialization */
    VTSS_RC(b2_ana_init(vtss_state, conf));

    /* ASM initialization */
    VTSS_RC(b2_asm_init(vtss_state, host_mode));
  
    /* QSS initialization */
    VTSS_RC(b2_qss_init(vtss_state));
    
    /* QoS and filter initialization */
    for (port = 0; port < 26; port++) {
        if (VTSS_PORT_IS_1G(port)) {
        } else {
            B2_WR(DEV10G, MAC_TX_MONITOR_STICKY, port, 0xffffffff);
        }

        VTSS_RC(b2_port_qos_set(vtss_state, port, &vtss_state->qos.port_conf[0]));
        VTSS_RC(b2_port_filter_set(vtss_state, port, &vtss_state->port.port_filter[0]));
    }

    /* SCH initialization */
    VTSS_RC(b2_sch_init(vtss_state, conf));
    

    return VTSS_RC_OK;
}

vtss_rc vtss_b2_inst_create(vtss_state_t *vtss_state)
{
    /* Initialization */
    vtss_state->cil.init_conf_set = b2_init_conf_set;

    /* Miscellaneous */
    vtss_state->misc.reg_read = b2_reg_read;
    vtss_state->misc.reg_write = b2_reg_write;
    vtss_state->misc.chip_id_get = b2_chip_id_get;
#if defined(VTSS_GPIOS)
    vtss_state->misc.gpio_mode = b2_gpio_mode;
    vtss_state->misc.gpio_read = b2_gpio_read;
    vtss_state->misc.gpio_write = b2_gpio_write;
    vtss_state->misc.gpio_clk_set = b2_gpio_clk_set;
#endif /* VTSS_GPIOS */
#if defined(VTSS_FEATURE_INTERRUPTS)
    vtss_state->misc.intr_mask_set = b2_intr_mask_set;
    vtss_state->misc.intr_status_get = b2_intr_status_get;
#endif

    /* Port control */
    vtss_state->port.miim_read = b2_miim_read;
    vtss_state->port.miim_write = b2_miim_write;
    vtss_state->port.mmd_read = b2_mmd_read;
    vtss_state->port.mmd_write = b2_mmd_write;
    vtss_state->port.status_interface_set = b2_psi_set;
    vtss_state->port.status_interface_get = b2_psi_get;
    vtss_state->port.forward_set  = b2_port_forward_state_set;
    vtss_state->port.map_set = b2_port_map_set;
    vtss_state->port.conf_set = b2_port_conf_set;
    vtss_state->port.counters_update = b2_port_counters_update;
    vtss_state->port.counters_clear = b2_port_counters_clear;
    vtss_state->port.counters_get = b2_port_counters_get;
    vtss_state->port.status_get = b2_port_status_get;
    vtss_state->port.clause_37_status_get = b2_port_clause_37_status_get;
    vtss_state->port.clause_37_control_set = b2_port_clause_37_control_set;
    vtss_state->port.host_conf_set = b2_host_conf_set;
    vtss_state->port.vid2port_set = b2_vid2port_set;
    vtss_state->port.vid2lport_set = b2_vid2lport_set;
    vtss_state->port.port_filter_set = b2_port_filter_set;

    /* QoS */
    vtss_state->qos.port_conf_set = b2_qos_port_conf_set;
    vtss_state->qos.lport_conf_set = b2_qos_lport_conf_set;
    vtss_state->qos.conf_set = b2_qos_conf_set;
    vtss_state->qos.dscp_table_set = b2_dscp_table_set;

    /* State data */
    vtss_state->qos.prio_count = B2_PRIOS;
#if defined(VTSS_GPIOS)
    vtss_state->misc.gpio_count = VTSS_GPIOS;
#endif /* VTSS_GPIOS */
    
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_B2 */

