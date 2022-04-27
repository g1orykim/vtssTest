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

// Avoid "vtss_api.h not used in module vtss_daytona_basics.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"

/* Chip port numbers */
#define VTSS_CHIP_PORTS 10 /* all physical ports */
#define DAYTONA_TARGET_ID_MASK 0x3f0000  /* bits in the address that holds the target ID. */
#define DAYTONA_REGISTER_OFFSET_MASK 0x0ffff  /* bits in the address that holds the target ID. */
#define VTSS_DAYTONA_DIRECT_REG_ADDRESS(tgt, addr) {addr = (tgt) | ((addr & DAYTONA_REGISTER_OFFSET_MASK) >> 1);}

/* Read target register via PI */
vtss_rc daytona_pi_rd(vtss_state_t *vtss_state,
                      vtss_chip_no_t chip_no, u32 addr, u32 *value)
{
    u32              val, mask, i;
    vtss_reg_read_t  read_func= vtss_state->init_conf.reg_read;
    u32 tgt = addr & DAYTONA_TARGET_ID_MASK;
    u32              tmp_addr;

    VTSS_N("enter");
    if (tgt == VTSS_TO_CFG_IF_FAST_REGS || tgt == VTSS_TO_CFG_IF_ORG) {
        VTSS_DAYTONA_DIRECT_REG_ADDRESS(tgt, addr);
        /* Direct operation */
        return read_func(chip_no, addr, value);
    }

    /* Indirect operation */
    VTSS_RC(read_func(chip_no, addr, &val));

    VTSS_N("step2");
    /* Wait for operation to complete */
    mask = (VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_RD_IN_PROGRESS |
            VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_WR_IN_PROGRESS);
    for (i = 0; i < 25; i++) {
        VTSS_N("step2a");
        tmp_addr = VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2;
        VTSS_DAYTONA_DIRECT_REG_ADDRESS(VTSS_TO_CFG_IF_FAST_REGS, tmp_addr);
        VTSS_RC(read_func(chip_no, tmp_addr, &val));
        VTSS_N("step2b");
        if ((val & mask) == 0) {
            /* Read/write operation completed */
            VTSS_N("step3");
            tmp_addr = VTSS_FAST_REGS_CSR_REGS_SLOWDATA_MSB;
            VTSS_DAYTONA_DIRECT_REG_ADDRESS(VTSS_TO_CFG_IF_FAST_REGS, tmp_addr);
            VTSS_RC(read_func(chip_no, tmp_addr, &val));
            tmp_addr = VTSS_FAST_REGS_CSR_REGS_SLOWDATA_LSB;
            VTSS_DAYTONA_DIRECT_REG_ADDRESS(VTSS_TO_CFG_IF_FAST_REGS, tmp_addr);
            VTSS_RC(read_func(chip_no, tmp_addr, value));
            VTSS_N("step4");
            *value += (val << 16);
            VTSS_N("step5");
            VTSS_I("chip_no %d, addr %x, value %x", chip_no, addr, *value);
            return VTSS_RC_OK;
        }
        VTSS_NSLEEP(100);
    }
    return VTSS_RC_ERROR;
}

/* Write target register via PI
   direct or indirect register access */
vtss_rc daytona_pi_wr(vtss_state_t *vtss_state,
                      vtss_chip_no_t chip_no, u32 addr, const u32 value)
{
    u32              val, mask, i;
    vtss_reg_read_t  read_func = vtss_state->init_conf.reg_read;
    vtss_reg_write_t write_func = vtss_state->init_conf.reg_write;
    u32              tgt = addr & DAYTONA_TARGET_ID_MASK;
    u32              tmp_addr = VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2;

    if (tgt == VTSS_TO_CFG_IF_FAST_REGS || tgt == VTSS_TO_CFG_IF_ORG) {
        /* Adjust the address to reflect 2-byte offset */
        VTSS_DAYTONA_DIRECT_REG_ADDRESS(tgt, addr);

        /* Direct operation */
        return (write_func(chip_no, addr, value));
    }

    /* Indirect operation */
    VTSS_RC(write_func(chip_no, addr,     value >> 16));    /* MSB */
    VTSS_RC(write_func(chip_no, addr + 2, value & 0xffff)); /* LSB */

    VTSS_DAYTONA_DIRECT_REG_ADDRESS(VTSS_TO_CFG_IF_FAST_REGS, tmp_addr);

    /* Wait for operation to complete */
    mask = (VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_RD_IN_PROGRESS |
            VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_WR_IN_PROGRESS);
    for (i = 0; i < 25; i++) {
        VTSS_RC(read_func(chip_no, tmp_addr, &val));
        if ((val & mask) == 0) {
            /* Read/write operation completed */
            VTSS_I("chip_no %d, addr %x, value %x", chip_no, addr, value);
            return VTSS_RC_OK;
        }
        VTSS_NSLEEP(100);
    }
    return VTSS_RC_ERROR;
}

/* Read/write register */
/* TBD if needed, */
#if 0
static vtss_rc daytona_rd_wr(u32 tgt, u32 addr, u32 port, u32 *value, BOOL read)
{
    /* validate parameter ranges */
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
    if (read) {
        VTSS_RC(daytona_pi_rd(0, (tgt<<16) + addr, value));
    } else {
        VTSS_RC(daytona_pi_wr(0, (tgt<<16) + addr, *value));
    }
    VTSS_N("%s, tgt: 0x%02x, addr: 0x%04x, value: 0x%08x",
           read ? "RD" : "WR", tgt, addr, *value);
    return VTSS_RC_OK;
}
#endif

/* Read register field */
vtss_rc daytona_rdf(vtss_state_t *vtss_state,
                    vtss_chip_no_t chip_no, u32 reg, u32 bit_offset, u32 bit_width, u32 *value)
{
    VTSS_RC(daytona_pi_rd(vtss_state, chip_no, reg, value));
    *value = VTSS_EXTRACT_BITFIELD(*value, bit_offset, bit_width);
    return VTSS_RC_OK;
}

/* Write register field */
vtss_rc daytona_wrf(vtss_state_t *vtss_state,
                    vtss_chip_no_t chip_no, u32 reg, u32 bit_offset, u32 bit_width, u32 value)
{
    u32 val, mask;

    mask = VTSS_BITMASK(bit_width);
    if ((value & mask) != value) {
        VTSS_E("illegal value: 0x%08x, chip: %u, reg: %u, offs: %u, width: %u",
               value, chip_no, reg, bit_offset, bit_width);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(daytona_pi_rd(vtss_state, chip_no, reg, &val));
    val = ((val & ~(mask << bit_offset)) | (value << bit_offset));
    VTSS_RC(daytona_pi_wr(vtss_state, chip_no, reg, val));
    return VTSS_RC_OK;
}

/* Write register mask */
vtss_rc daytona_wrm(vtss_state_t *vtss_state,
                    vtss_chip_no_t chip_no, u32 reg, u32 value, u32 mask)
{
    u32 val;

    VTSS_RC(daytona_pi_rd(vtss_state, chip_no, reg, &val));
    val = ((val & ~mask) | (value & mask));
    VTSS_RC(daytona_pi_wr(vtss_state, chip_no, reg, val));
    return VTSS_RC_OK;
}



/* ================================================================= *
 *  Target address mapping
 * ================================================================= */

vtss_rc daytona_target(daytona_channel_t channel, daytona_side_t side, daytona_block_id_t fb, u32 *target)
{
    /*                                          channel 0          channel 0           channel 1           channel 1 */
    /*                                          client side        line side           client side         line side */
    static const u32 upi_targets[] =           {VTSS_TO_UPI_0,                         VTSS_TO_UPI_1};
    static const u32 xfi_targets[] =           {VTSS_TO_CXFI_0,    VTSS_TO_LXFI_0,     VTSS_TO_CXFI_1,     VTSS_TO_LXFI_1};
    static const u32 tfi_5_targets[] =         {VTSS_TO_TFI_0,                         VTSS_TO_TFI_1};
    static const u32 sfi4_targets[] =          {VTSS_TO_SFI_0,                         VTSS_TO_SFI_1};
    static const u32 pcs_xaui_targets[] =      {VTSS_TO_XAUI_0,                        VTSS_TO_XAUI_1};
    static const u32 otn_wrapper_targets[] =   {VTSS_TO_OTU_1_0,   VTSS_TO_OTU_2_0,    VTSS_TO_OTU_1_1,    VTSS_TO_OTU_2_1};
    static const u32 ewis_targets[] =          {VTSS_TO_WIS_1_0,   VTSS_TO_WIS_2_0,    VTSS_TO_WIS_1_1,    VTSS_TO_WIS_2_1};
    static const u32 pcs_10gbase_r_targets[] = {VTSS_TO_PCS_1_0,   VTSS_TO_PCS_2_0,    VTSS_TO_PCS_1_1,    VTSS_TO_PCS_2_1};
    static const u32 gfp_targets[] =           {VTSS_TO_GFP_1_0,   VTSS_TO_GFP_2_0,    VTSS_TO_GFP_1_1,    VTSS_TO_GFP_2_1};
    static const u32 mac10g_targets[] =        {VTSS_TO_MAC_1_0,   VTSS_TO_MAC_2_0,    VTSS_TO_MAC_1_1,    VTSS_TO_MAC_2_1};
    static const u32 rab_targets[] =           {VTSS_TO_RAB_0,                         VTSS_TO_RAB_1};
    static const u32 efeci4_targets[] =        {                   VTSS_TO_EFEC_0,                         VTSS_TO_EFEC_1};
    static const u32 efeci7_targets[] =        {                   VTSS_TO_EFECI7_0,                       VTSS_TO_EFECI7_1};
    static const u32 ip_1588_targets[] =       {VTSS_TO_PTP_0,                         VTSS_TO_PTP_1};
    static const u32 org_targets =             {VTSS_TO_CFG_IF_ORG};
    static const u32 fast_targets =            {VTSS_TO_CFG_IF_FAST_REGS};
    static const u32 gcb_targets =             {VTSS_TO_CFG_IF_GCB};

    if (channel < 2 && side < 2) {
        switch (fb) {
        case DAYTONA_BLOCK_UPI:
            if (side != CLIENT_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = upi_targets[channel];
            }
            break;
        case DAYTONA_BLOCK_XFI:
            *target = xfi_targets[(channel<<1)+side];
            break;
        case DAYTONA_BLOCK_TFI_5:
            if (side != CLIENT_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = tfi_5_targets[channel];
            }
            break;
        case DAYTONA_BLOCK_SFI4:
            if (side != CLIENT_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = sfi4_targets[channel];
            }
            break;
        case DAYTONA_BLOCK_PCS_XAUI:
            if (side != CLIENT_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = pcs_xaui_targets[channel];
            }
            break;
        case DAYTONA_BLOCK_OTN_WRAPPER:
            *target = otn_wrapper_targets[(channel<<1)+side];
            break;
        case DAYTONA_BLOCK_EWIS:
            *target = ewis_targets[(channel<<1)+side];
            break;
        case DAYTONA_BLOCK_PCS_10GBASE_R:
            *target = pcs_10gbase_r_targets[(channel<<1)+side];
            break;
        case DAYTONA_BLOCK_GFP:
            *target = gfp_targets[(channel<<1)+side];
            break;
        case DAYTONA_BLOCK_MAC10G:
            *target = mac10g_targets[(channel<<1)+side];
            break;
        case DAYTONA_BLOCK_RAB:
            *target = rab_targets[channel];
            break;
        case DAYTONA_BLOCK_EFECI4:
            *target = efeci4_targets[channel];
            break;
        case DAYTONA_BLOCK_EFECI7:
            *target = efeci7_targets[channel];
            break;
        case DAYTONA_BLOCK_IP_1588:
            if (side != LINE_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = ip_1588_targets[channel];
            }
            break;
        case DAYTONA_BLOCK_IP_1588_ANA0_INGR:
            if (side != LINE_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = VTSS_TO_PTP_ANA0_INGR_CFG;
            }
            break;
        case DAYTONA_BLOCK_IP_1588_ANA0_EGR:
            if (side != LINE_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = VTSS_TO_PTP_ANA0_EGR_CFG;
            }
            break;
        case DAYTONA_BLOCK_IP_1588_ANA1_INGR:
            if (side != LINE_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = VTSS_TO_PTP_ANA1_INGR_CFG;
            }
            break;
        case DAYTONA_BLOCK_IP_1588_ANA1_EGR:
            if (side != LINE_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = VTSS_TO_PTP_ANA1_EGR_CFG;
            }
            break;
        case DAYTONA_BLOCK_IP_1588_ANA2_INGR:
            if (side != LINE_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = VTSS_TO_PTP_ANA2_INGR_CFG;
            }
            break;
        case DAYTONA_BLOCK_IP_1588_ANA2_EGR:
            if (side != LINE_SIDE) {
                *target = 0;
                return VTSS_RC_ERROR;
            } else {
                *target = VTSS_TO_PTP_ANA2_EGR_CFG;
            }
            break;
        case DAYTONA_BLOCK_DEVCPU_ORG:
            *target = org_targets;
            break;
        case DAYTONA_BLOCK_FAST_REGS:
            *target = fast_targets;
            break;
        case DAYTONA_BLOCK_DEVCPU_GCB:
            *target = gcb_targets;
            break;
        default:
            *target = 0;
            return VTSS_RC_ERROR;
            //break;
        }
        return VTSS_RC_OK;
    } else {
        return VTSS_RC_ERROR;
    }
}

vtss_rc daytona_port_2_mode(vtss_state_t *vtss_state,
                            vtss_port_no_t port, vtss_config_mode_t *mode)
{
    daytona_channel_t channel;
    daytona_side_t    side;

    VTSS_RC(daytona_port_2_channel_side(vtss_state, port, &channel, &side));
    *mode = vtss_state->init_conf.conf_mode[channel];
    return VTSS_RC_OK;
}

vtss_rc daytona_port_2_target(vtss_state_t *vtss_state,
                              vtss_port_no_t port_no, daytona_block_id_t fb, u32 *target)
{
    u32 port = VTSS_CHIP_PORT(port_no);
    return daytona_target(port % 2, port / 2, fb,target);
}

vtss_rc daytona_port_2_channel_side(vtss_state_t *vtss_state,
                                    vtss_port_no_t port_no, daytona_channel_t *channel, daytona_side_t *side)
{
    u32 port = VTSS_CHIP_PORT(port_no);
    *channel = (port % 2);
    *side = (port / 2);
    return VTSS_RC_OK;
}


#endif /* VTSS_ARCH_DAYTONA */

