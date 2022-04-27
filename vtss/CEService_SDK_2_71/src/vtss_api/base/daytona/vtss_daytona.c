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

// Avoid "vtss_api.h not used in module vtss_daytona.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "../ail/vtss_common.h"

#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_wis.h"
#include "vtss_daytona_otn.h"
#include "vtss_daytona_gfp.h"
#include "vtss_daytona_upi.h"
#include "vtss_daytona_xaui.h"
#include "vtss_daytona_xfi.h"
#include "vtss_daytona_mac10g.h"
#include "vtss_daytona_pcs_10gbase_r.h"
#include "vtss_daytona_rab.h"
#include "vtss_daytona_i2c.h"
#include "vtss_daytona_oha.h"
#include "vtss_daytona_reg_init_gcb.h"
#include "vtss_daytona_reg_init_gcb_client_pll.h"
#include "vtss_daytona_reg_init_gcb_line_pll.h"
#include "vtss_daytona_reg_init_gfp.h"
#include "vtss_daytona_reg_init_mac.h"
#include "vtss_daytona_reg_init_otu.h"
#include "vtss_daytona_reg_init_otu_fec.h"
#include "vtss_daytona_reg_init_pcs.h"
#include "vtss_daytona_reg_init_predictor.h"
#include "vtss_daytona_reg_init_ptp.h"
#include "vtss_daytona_reg_init_rab.h"
#include "vtss_daytona_reg_init_sfi.h"
#include "vtss_daytona_reg_init_tfi.h"
#include "vtss_daytona_reg_init_upi.h"
#include "vtss_daytona_reg_init_upi_hss6g.h"
#include "vtss_daytona_reg_init_wis.h"
#include "vtss_daytona_reg_init_xaui.h"
#include "vtss_daytona_reg_init_xfi.h"
#include "vtss_daytona_reg_init_xfi_hss11g.h"
#include "vtss_daytona_reg_init_block_states.h"

#include "vtss_daytona_regs_ip_1588.h"
#include "vtss_daytona_regs_upi.h"
#include "vtss_daytona_regs_xfi.h"

/* Chip port numbers */
#define VTSS_CHIP_PORTS 10 /* all physical ports */
#define  DAYTONA_TARGET_ID_MASK 0x3f0000  /* bits in the address that holds the target ID. */

/*lint +linebuf */ /*... Long lines expected in the init files */
/*lint +linebuf */ /*... Long lines expected in the init files */

#include "vtss_daytona_regs_used_debug.h"
#include "vtss_daytona_reg_init_gcb.inc"
#include "vtss_daytona_reg_init_gcb_client_pll.inc"
#include "vtss_daytona_reg_init_gcb_line_pll.inc"
#include "vtss_daytona_reg_init_wis.inc"
#include "vtss_daytona_reg_init_pcs.inc"
#include "vtss_daytona_reg_init_mac.inc"
#include "vtss_daytona_reg_init_rab.inc"
#include "vtss_daytona_reg_init_predictor.inc"
#include "vtss_daytona_reg_init_ptp.inc"
#include "vtss_daytona_reg_init_gfp.inc"
#include "vtss_daytona_reg_init_otu.inc"
#include "vtss_daytona_reg_init_otu_fec.inc"
#include "vtss_daytona_reg_init_xaui.inc"
#include "vtss_daytona_reg_init_sfi.inc"
#include "vtss_daytona_reg_init_upi.inc"
#include "vtss_daytona_reg_init_tfi.inc"
#include "vtss_daytona_reg_init_xfi.inc"
#include "vtss_daytona_reg_init_upi_hss6g.inc"
#include "vtss_daytona_reg_init_xfi_hss11g.inc"
#include "vtss_daytona_reg_init_block_states.inc"

#define VAR0 0x01       /* variant 0 full featured Daytona */
#define VAR1 0x02       /* variant 1 10GbE client only with LAN/WAN support. SONET, 8/10GFC, OTN and FEC disabled */
#define VAR2 0x04       /* variant 2 10GbE client only with OTN and FEC. SONET, 8GFC disabled */
#define VAR3 0x08       /* variant 3 full featured Daytona */

static const u32 variant_masks [] = {VAR0, VAR1, VAR2, VAR0};
/*
 * This table defines which modes are supported in which variants.
 * Table last updated according to mail received from Mike Scholten 11-mar-2010.
 * More modes are supported in variant 2 than originally expected.
 */
static const u32 feature_mask [] = {
    
    /* Mode type 1: PHY */
    [VTSS_CONFIG_MODE_PSS]              =  VAR0,                   /* VTSS_CONFIG_MODE_PSS, */
    [VTSS_CONFIG_MODE_PSS_W]            =  VAR0,                   /* VTSS_CONFIG_MODE_PSSW, */
    [VTSS_CONFIG_MODE_PSOC_A]           =  VAR0,                   /* VTSS_CONFIG_MODE_PSOC_A, */
    [VTSS_CONFIG_MODE_PSOC_WA]          =  VAR0,                   /* VTSS_CONFIG_MODE_PSOC_WA, */
    [VTSS_CONFIG_MODE_PSOC_S]           =  VAR0,                   /* VTSS_CONFIG_MODE_PSOC_S, */
    [VTSS_CONFIG_MODE_PSOC_WS]          =  VAR0,                   /* VTSS_CONFIG_MODE_PSOC_WS, */
    [VTSS_CONFIG_MODE_PEO_WMWS]         =  VAR0,                   /* VTSS_CONFIG_MODE_PEO_WMWS, */
    [VTSS_CONFIG_MODE_PEO_WMWA]         =  VAR0,                   /* VTSS_CONFIG_MODE_PEO_WMWA, */
    [VTSS_CONFIG_MODE_PEE_W]            =  VAR0 | VAR1 | VAR2,     /* VTSS_CONFIG_MODE_PEE_W, */
    [VTSS_CONFIG_MODE_PEE_MW]           =  VAR0 | VAR1 | VAR2,     /* VTSS_CONFIG_MODE_PEE_MW, */
    [VTSS_CONFIG_MODE_XAUI_2M_PEE_MW]   =  VAR0 | VAR1 | VAR2,     /* VTSS_CONFIG_MODE_XAUI_2M_PEE_MW, */
    [VTSS_CONFIG_MODE_PEE_P]            =  VAR0 | VAR1 | VAR2,     /* VTSS_CONFIG_MODE_PEE_P, */
    [VTSS_CONFIG_MODE_PEE_MP]           =  VAR0 | VAR1 | VAR2,     /* VTSS_CONFIG_MODE_PEE_MP, */
    [VTSS_CONFIG_MODE_XAUI_2M_PEE_MP]   =  VAR0 | VAR1 | VAR2,     /* VTSS_CONFIG_MODE_XAUI_2M_PEE_MP, */
    [VTSS_CONFIG_MODE_PFF_P]            =  VAR0,                   /* VTSS_CONFIG_MODE_PFF_P, */
    [VTSS_CONFIG_MODE_PEO_WA]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PEO_WA, */
    [VTSS_CONFIG_MODE_PEO_MWA]          =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PEO_MWA, */
    [VTSS_CONFIG_MODE_PEO_WS]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PEO_WS, */
    [VTSS_CONFIG_MODE_PEO_MWS]          =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PEO_MWS, */
    [VTSS_CONFIG_MODE_PEO_P2E_20]       =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PEO_P2E_20, */
    [VTSS_CONFIG_MODE_PEO_MP2E_20]      =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PEO_MP2E_20, */
    [VTSS_CONFIG_MODE_PEO_P2E_100]      =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PEO_P2E_100, */
    [VTSS_CONFIG_MODE_PEO_P1E_100]      =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PEO_P2E_100, */
    [VTSS_CONFIG_MODE_XAUI_2M_PEO_P1E_100] =  VAR0 | VAR2,         /* VTSS_CONFIG_MODE_XAUI_2M_PEO_P2E_100, */
    [VTSS_CONFIG_MODE_PFO_P2F]          =  VAR0,                   /* VTSS_CONFIG_MODE_PFO_P2F, */
    [VTSS_CONFIG_MODE_PGO_2]            =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PGO_2, */
    [VTSS_CONFIG_MODE_PGO_2SUB]         =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PGO_2SUB, */
    [VTSS_CONFIG_MODE_PGO_2SUBSMALL]    =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PGO_2SUBSMALL, */
    [VTSS_CONFIG_MODE_PGO_2GEN]         =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_PGO_2GEN, */

    /* Mode type 2: Backplane */
    [VTSS_CONFIG_MODE_BOO_2]            =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOO_2, */
    [VTSS_CONFIG_MODE_BOO_1E]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOO_1E, */
    [VTSS_CONFIG_MODE_BOO_2E]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOO_2E, */
    [VTSS_CONFIG_MODE_BOO_2F]           =  VAR0,                   /* VTSS_CONFIG_MODE_BOO_2F, */
    [VTSS_CONFIG_MODE_BMX_2]            =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BMX_2, */
    [VTSS_CONFIG_MODE_BMX_2E]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BMX_2E, */
    [VTSS_CONFIG_MODE_BMX_1E]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BMX_1E, */
    [VTSS_CONFIG_MODE_BMX_2F]           =  VAR0,                   /* VTSS_CONFIG_MODE_BMX_2F, */
    [VTSS_CONFIG_MODE_BOE_WMWA]         =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOE_WMWA, */
    [VTSS_CONFIG_MODE_BOE_WMWS]         =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOE_WMWS, */
    [VTSS_CONFIG_MODE_BOE_PMP2E_20]     =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOE_PMP2E_20, */
    [VTSS_CONFIG_MODE_BOE_PMP1E_20]     =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOE_PMP1E_20, */
    [VTSS_CONFIG_MODE_BOEC_2E]          =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOEC_2E, */
    [VTSS_CONFIG_MODE_BOEC_1E]          =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOEC_1E, */
    [VTSS_CONFIG_MODE_BOG_2]            =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOG_2, */
    [VTSS_CONFIG_MODE_BOG_2SUB]         =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOG_2SUB, */
    [VTSS_CONFIG_MODE_BOG_W2]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOG_W2, */
    [VTSS_CONFIG_MODE_BOSC_A]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOSC_A, */
    [VTSS_CONFIG_MODE_BOSC_WA]          =  VAR0,                   /* VTSS_CONFIG_MODE_BOSC_WA, */
    [VTSS_CONFIG_MODE_BOSC_S]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_BOSC_S, */
    [VTSS_CONFIG_MODE_BOSC_WS]          =  VAR0,                   /* VTSS_CONFIG_MODE_BOSC_WS, */
    [VTSS_CONFIG_MODE_BOFC_2F]          =  VAR0,                   /* VTSS_CONFIG_MODE_BOFC_2F, */
    [VTSS_CONFIG_MODE_BO8F]             =  VAR0,                   /* VTSS_CONFIG_MODE_BO8F, */
 
    /* Mode type 3: Transponder */
    [VTSS_CONFIG_MODE_TSOC_A]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TSOC_A, */
    [VTSS_CONFIG_MODE_TSOC_WA]          =  VAR0,                   /* VTSS_CONFIG_MODE_TSOC_WA, */
    [VTSS_CONFIG_MODE_TSOC_S]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TSOC_S, */
    [VTSS_CONFIG_MODE_TSOC_WS]          =  VAR0,                   /* VTSS_CONFIG_MODE_TSOC_WS, */
    [VTSS_CONFIG_MODE_TEO_PMP_2E]       =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEO_PMP_2E, */
    [VTSS_CONFIG_MODE_TEO_PMP_1E]       =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEO_PMP_1E, */
    [VTSS_CONFIG_MODE_TEO_PP_2E]        =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEO_PMP_2E, */
    [VTSS_CONFIG_MODE_TFO_PP2F]         =  VAR0,                   /* VTSS_CONFIG_MODE_TFO_PP2F, */
    [VTSS_CONFIG_MODE_TEOC_2E]          =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEOC_2E, */
    [VTSS_CONFIG_MODE_TEOC_1E]          =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEOC_1E, */
    [VTSS_CONFIG_MODE_TFOC_2F]          =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TFOC_2E, */
    [VTSS_CONFIG_MODE_TEO_WMWA]         =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEO_WMWA, */
    [VTSS_CONFIG_MODE_TEO_WMWS]         =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEO_WMWS, */
    [VTSS_CONFIG_MODE_TGO_2]            =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TGO_2, */
    [VTSS_CONFIG_MODE_TGO_2SUB]         =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TGO_2SUB, */
    [VTSS_CONFIG_MODE_TGO_2SUBSMALL]    =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TGO_2SUBSMALL, */
    [VTSS_CONFIG_MODE_TGO_W2]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TGO_W2, */
    [VTSS_CONFIG_MODE_T8FO]             =  VAR0,                   /* VTSS_CONFIG_MODE_T8FO,  */
    [VTSS_CONFIG_MODE_TOO_2]            =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TOO_2, */
    [VTSS_CONFIG_MODE_TOO_2E2]          =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TOO_2E2, */
    [VTSS_CONFIG_MODE_TOO_1E2]          =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TOO_1E2, */
    [VTSS_CONFIG_MODE_TOO_2F2]          =  VAR0,                   /* VTSS_CONFIG_MODE_TOO_2F2, */
    [VTSS_CONFIG_MODE_TEE_L]            =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEE_L, */
    [VTSS_CONFIG_MODE_TEE_PMP_S]        =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEE_PMP_S, */
    [VTSS_CONFIG_MODE_TEE_PMP_A]        =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEE_PMP_A, */
    [VTSS_CONFIG_MODE_TEE_PMW]          =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEE_PMW */
    [VTSS_CONFIG_MODE_TEE_PP]           =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEE_PP, */
    [VTSS_CONFIG_MODE_TEE_W]            =  VAR0 | VAR2,            /* VTSS_CONFIG_MODE_TEE_W, */
    [VTSS_CONFIG_MODE_TFF]              =  VAR0,                   /* VTSS_CONFIG_MODE_TFF, */
    [VTSS_CONFIG_MODE_TSS]              =  VAR0,                   /* VTSS_CONFIG_MODE_TSS, */
    [VTSS_CONFIG_MODE_DISABLED]         =  VAR0 | VAR1 | VAR2,     /* VTSS_CONFIG_MODE_DISABLED,  */

    [VTSS_CONFIG_MODE_MAX]              =  0                       /* VTSS_CONFIG_MODE_MAX,  */
};



/* ================================================================= *
 *  Miscellaneous
 * ================================================================= */

static vtss_rc daytona_chip_var_get(vtss_state_t *vtss_state, u16 *chip_var)
{
    u32 value;
    DAYTONA_RD(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_MODE, &value);
    *chip_var = VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_MODE_VAR_SEL(value);
    return VTSS_RC_OK;
}

static vtss_rc daytona_chip_id_get(vtss_state_t *vtss_state, vtss_chip_id_t *const chip_id)
{
    u32 value;

    DAYTONA_RD(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID, &value);
    if (value == 0 || value == 0xffffffff) {
        VTSS_E("CPU interface error, chipid: 0x%08x", value);
        return VTSS_RC_ERROR;
    }

    chip_id->part_number = VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(value);
    chip_id->revision = VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(value);
    VTSS_RC(daytona_chip_var_get(vtss_state, &chip_id->variant));
    VTSS_D("part_number: 0x%04x, revision: %u, variant: %d", chip_id->part_number, chip_id->revision, chip_id->variant);
    return VTSS_RC_OK;
}

static vtss_rc daytona_reg_read(vtss_state_t *vtss_state,
                                const vtss_chip_no_t chip_no, const u32 reg, u32 *const value)
{
    return daytona_pi_rd(vtss_state, chip_no, reg, value);
}

static vtss_rc daytona_reg_write(vtss_state_t *vtss_state,
                                 const vtss_chip_no_t chip_no, const u32 reg, const u32 value)
{
    return daytona_pi_wr(vtss_state, chip_no, reg, value);
}


/* Setup port map */
static vtss_rc daytona_port_map_set(vtss_state_t *vtss_state)
{
    VTSS_D("enter");
    /* Nothing to do: everything is done in AIL layer */
    return VTSS_RC_OK;
}

/* Get CFG_FAST_REGS Base Offset */
static void daytona_fast_regs_conf_get(vtss_state_t *vtss_state, u32 *const reg_addr)
{
    *reg_addr = VTSS_TO_CFG_IF_FAST_REGS;
}

#if defined(VTSS_GPIOS)

/* ================================================================= *
 *  GPIO's
 * ================================================================= */

/* Set GPIO mode, read:0  write:1 */
static vtss_rc daytona_gpio_mode(vtss_state_t           *vtss_state,
                                 const vtss_chip_no_t   chip_no,
                                 const vtss_gpio_no_t   gpio_no,
                                 const vtss_gpio_mode_t mode)
{
    vtss_rc rc = VTSS_RC_OK;
    if (gpio_no < VTSS_GPIO_NO_END) {
        switch (mode) {
            case VTSS_GPIO_OUT:
                DAYTONA_WRF(VTSS_DEVCPU_GCB_GPIO_GPIO_OUTPUT_ENA, gpio_no, 1, 1);
                DAYTONA_WRF(VTSS_DEVCPU_GCB_GPIO_GPIO_INT_MASK, gpio_no, 1, 0);
                break;
            case VTSS_GPIO_IN:
                DAYTONA_WRF(VTSS_DEVCPU_GCB_GPIO_GPIO_OUTPUT_ENA, gpio_no, 1, 0);
                DAYTONA_WRF(VTSS_DEVCPU_GCB_GPIO_GPIO_INT_MASK, gpio_no, 1, 0);
                break;
            case VTSS_GPIO_IN_INT:
                DAYTONA_WRF(VTSS_DEVCPU_GCB_GPIO_GPIO_OUTPUT_ENA, gpio_no, 1, 0);
                DAYTONA_WRF(VTSS_DEVCPU_GCB_GPIO_GPIO_INT_MASK, gpio_no, 1, 1);
                break;
            default:
                VTSS_E("mode (%sd) not supported",mode);
                rc = VTSS_RC_ERROR;
                break;
        }
        return rc;
    }
    VTSS_E("illegal gpio_no: %d",gpio_no);
    return VTSS_RC_ERROR;
}

/* Read GPIO input pin */
static vtss_rc daytona_gpio_read(vtss_state_t         *vtss_state,
                                 const vtss_chip_no_t chip_no,
                                 const vtss_gpio_no_t gpio_no,
                                 BOOL                 *const value)
{
    u32 val;
    if (gpio_no < VTSS_GPIO_NO_END) {
        DAYTONA_RDF(VTSS_DEVCPU_GCB_GPIO_GPIO_I, gpio_no, 1, &val);
        *value = VTSS_BOOL(val);
        return VTSS_RC_OK;
    }
    VTSS_E("illegal gpio_no: %d",gpio_no);
    return VTSS_RC_ERROR;
}

/* Write GPIO output pin */
static vtss_rc daytona_gpio_write(vtss_state_t         *vtss_state,
                                  const vtss_chip_no_t chip_no,
                                  const vtss_gpio_no_t gpio_no,
                                  const BOOL           value)
{
    if (gpio_no < VTSS_GPIO_NO_END) {
        DAYTONA_WRF(VTSS_DEVCPU_GCB_GPIO_GPIO_O, gpio_no, 1, value);
        return VTSS_RC_OK;
    }

    VTSS_E("illegal gpio_no: %d",gpio_no);
    return VTSS_RC_ERROR;
}

#endif /* VTSS_GPIOS  */

/* ================================================================= *
 *  MII Port Control
 * ================================================================= */

/* PHY commands */
#define PHY_CMD_ADDRESS  0 /* Clause 45 only */
#define PHY_CMD_WRITE    1
#define PHY_CMD_READ_INC 2 /* NB: READ for Clause 22 */
#define PHY_CMD_READ     3 /* Clause 45 only */

static vtss_rc daytona_miim_cmd(vtss_state_t *vtss_state,
                                u32 cmd, u32 sof, vtss_miim_controller_t miim_controller,
                                u8 miim_addr, u8 addr, u16 *data, 
                                BOOL report_errors)
{
    u32 n;
    u32 value;

    VTSS_I("cmd: %d, sof %d, miim_contr %d, miim adr %d, addr %d, data %x", cmd, sof, miim_controller, miim_addr, addr, *data);

    switch (miim_controller) {
    case VTSS_MIIM_CONTROLLER_0:
        break;
    default:
        VTSS_E("illegal miim_controller: %d", miim_controller);
        return VTSS_RC_ERROR;
    }

    /* Set Start of frame field */
    DAYTONA_WRM(VTSS_DEVCPU_GCB_MIIM_MIIM_CFG,
                VTSS_F_DEVCPU_GCB_MIIM_MIIM_CFG_MIIM_ST_CFG_FIELD(sof) | /* ST field */
                VTSS_F_DEVCPU_GCB_MIIM_MIIM_CFG_MIIM_CFG_PRESCALE(0x40), /* prescale field */
                VTSS_F_DEVCPU_GCB_MIIM_MIIM_CFG_MIIM_ST_CFG_FIELD(0x3) |
                VTSS_F_DEVCPU_GCB_MIIM_MIIM_CFG_MIIM_CFG_PRESCALE(0xff));/* mask */
    \

    /* Read command is different for Clause 22 */
    if (sof == 1 && cmd == PHY_CMD_READ) {
        cmd = PHY_CMD_READ_INC;
    }

    DAYTONA_WRM(VTSS_DEVCPU_GCB_MIIM_MIIM_CMD,
                VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_VLD | /* VLD f1ield */
                VTSS_F_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_PHYAD(miim_addr) | /* addressed phy number */
                VTSS_F_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_REGAD(addr) |
                VTSS_F_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_WRDATA(*data) |
                VTSS_F_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_OPR_FIELD(cmd),
                VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_VLD | /* VLD field */
                VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_PHYAD | /* addressed phy number */
                VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_REGAD |
                VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_WRDATA |
                VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_OPR_FIELD);/* mask */
    \

    /* Wait for access to complete */
    for (n = 0; ; n++) {

        DAYTONA_RD(VTSS_DEVCPU_GCB_MIIM_MIIM_STATUS, &value);
        if ((value & (VTSS_M_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_PENDING_RD |
                      VTSS_M_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_PENDING_WR)) == 0)
            break;
        if (n == 1000) {
            goto mmd_error;
        }
    }

    /* Read data */
    if (cmd == PHY_CMD_READ || cmd == PHY_CMD_READ_INC) {
        DAYTONA_RD(VTSS_DEVCPU_GCB_MIIM_MIIM_DATA, &value);
        if (VTSS_X_DEVCPU_GCB_MIIM_MIIM_DATA_MIIM_DATA_SUCCESS(value)) {
            goto mmd_error;
        }
        *data = VTSS_X_DEVCPU_GCB_MIIM_MIIM_DATA_MIIM_DATA_RDDATA(value);
    }
    return VTSS_RC_OK;

mmd_error:
    if(report_errors) {
        VTSS_E("miim failed, cmd: %s, miim_addr: %u, addr: %u",
               cmd == PHY_CMD_READ ? "PHY_CMD_READ" :
               cmd == PHY_CMD_WRITE ? "PHY_CMD_WRITE" :
               cmd == PHY_CMD_ADDRESS ? "PHY_CMD_ADDRESS" : "PHY_CMD_READ_INC",
               miim_addr, addr);
    }
    return VTSS_RC_ERROR;

    
    
    
}

static vtss_rc daytona_miim_read(vtss_state_t *vtss_state,
                                 vtss_miim_controller_t miim_controller,
                                 u8 miim_addr, u8 addr, u16 *value, 
                                 BOOL report_errors)
{
    return daytona_miim_cmd(vtss_state, PHY_CMD_READ, 1, miim_controller, miim_addr, addr, value, report_errors);
}

static vtss_rc daytona_miim_write(vtss_state_t *vtss_state,
                                  vtss_miim_controller_t miim_controller,
                                  u8 miim_addr, u8 addr, u16 value, 
                                  BOOL report_errors)
{
    return daytona_miim_cmd(vtss_state, PHY_CMD_WRITE, 1, miim_controller, miim_addr, addr, &value, report_errors);
}

/* MMD (MDIO Management Devices (10G)) read */
static vtss_rc daytona_mmd_read(vtss_state_t *vtss_state,
                                vtss_miim_controller_t miim_controller,
                                u8 miim_addr, u8 mmd, u16 addr, u16 *value, 
                                BOOL report_errors)
{
    VTSS_RC(daytona_miim_cmd(vtss_state, PHY_CMD_ADDRESS, 0, miim_controller, miim_addr, mmd, &addr, report_errors));
    return daytona_miim_cmd(vtss_state, PHY_CMD_READ, 0, miim_controller, miim_addr, mmd, value, report_errors);
}

/* MMD (MDIO Management Devices (10G)) write */
static vtss_rc daytona_mmd_write(vtss_state_t *vtss_state,
                                 vtss_miim_controller_t miim_controller,
                                 u8 miim_addr, u8 mmd, u16 addr, u16 data, 
                                 BOOL report_errors)
{
    VTSS_RC(daytona_miim_cmd(vtss_state, PHY_CMD_ADDRESS, 0, miim_controller, miim_addr, mmd, &addr, report_errors));
    return daytona_miim_cmd(vtss_state, PHY_CMD_WRITE, 0, miim_controller, miim_addr, mmd, &data, report_errors);
}



/* ================================================================= *
 *  Interrupts
 * ================================================================= */
static vtss_rc daytona_interrupt_enable_set(vtss_state_t *vtss_state,
                                            const vtss_interrupt_block_t  *const blocks)
{
    u16 value;

    value = 0;

    if (blocks->ch_0 & VTSS_CH_TFI_INT)  value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_TFI_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_SFI_INT)  value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_SFI_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_XAUI_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_XAUI_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_OTN1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_OTN_1_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_WIS1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_WIS_1_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_PCS1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_PCS_1_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_GFP1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_GFP_1_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_MAC1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_MAC_1_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_RAB_INT)  value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_RAB_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_MAC2_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_MAC_2_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_GFP2_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_GFP_2_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_PCS2_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_PCS_2_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_WIS2_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_WIS_2_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_OTN2_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_OTN_2_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_EFEC_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_EFEC_0_INT_ENABLE;
    if (blocks->ch_0 & VTSS_CH_UPI_INT)  value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_UPI_0_INT_ENABLE;

    DAYTONA_WR(VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0, value);

    value = 0;

    if (blocks->ch_1 & VTSS_CH_TFI_INT)  value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_TFI_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_SFI_INT)  value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_SFI_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_XAUI_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_XAUI_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_OTN1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_OTN_1_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_WIS1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_WIS_1_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_PCS1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_PCS_1_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_GFP1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_GFP_1_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_MAC1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_MAC_1_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_RAB_INT)  value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_RAB_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_MAC2_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_MAC_2_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_GFP2_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_GFP_2_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_PCS2_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_PCS_2_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_WIS2_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_WIS_2_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_OTN2_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_OTN_2_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_EFEC_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_EFEC_1_INT_ENABLE;
    if (blocks->ch_1 & VTSS_CH_UPI_INT)  value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_UPI_1_INT_ENABLE;

    DAYTONA_WR(VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1, value);

    value = 0;

    if (blocks->misc & VTSS_MISC_I2C_INT)      value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_I2C_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_PMTICK_INT)   value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_PMTICK_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_IP1588_0_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_IP1588_0_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_IP1588_1_INT) value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_IP1588_1_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_EFEC7_0_INT)  value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_EFEC7_0_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_EFEC7_1_INT)  value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_EFEC7_1_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_LXFI_0_INT)   value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_0_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_LXFI_1_INT)   value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_1_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_CXFI_0_INT)   value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_0_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_CXFI_1_INT)   value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_1_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_GPIO_INT)     value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_GPIO_INT_ENABLE;
    if (blocks->misc & VTSS_MISC_CPU_IF_INT)   value |= VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CPU_IF_INT_ENABLE;

    DAYTONA_WR(VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC, value);

    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_0_INT_ENABLE)
        /* When XFI is enabled the interrupt from XFI is enabled */
        DAYTONA_WRM(VTSS_XFI_XFI_CONTROL_INT_CTRL(VTSS_TO_LXFI_0), VTSS_M_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK, VTSS_M_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK);
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_1_INT_ENABLE)
        /* When XFI is enabled the interrupt from XFI is enabled */
        DAYTONA_WRM(VTSS_XFI_XFI_CONTROL_INT_CTRL(VTSS_TO_LXFI_1), VTSS_M_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK, VTSS_M_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK);
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_0_INT_ENABLE)
        /* When XFI is enabled the interrupt from XFI is enabled */
        DAYTONA_WRM(VTSS_XFI_XFI_CONTROL_INT_CTRL(VTSS_TO_CXFI_0), VTSS_M_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK, VTSS_M_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK);
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_1_INT_ENABLE)
        /* When XFI is enabled the interrupt from XFI is enabled */
        DAYTONA_WRM(VTSS_XFI_XFI_CONTROL_INT_CTRL(VTSS_TO_CXFI_1), VTSS_M_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK, VTSS_M_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK);

    return VTSS_RC_OK;
}

static vtss_rc daytona_interrupt_enable_get(vtss_state_t *vtss_state,
                                            vtss_interrupt_block_t   *const blocks)
{
    u32 value;

    DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0, &value);

    blocks->ch_0 = 0;

    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_TFI_0_INT_ENABLE)    blocks->ch_0 |= VTSS_CH_TFI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_SFI_0_INT_ENABLE)    blocks->ch_0 |= VTSS_CH_SFI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_XAUI_0_INT_ENABLE)   blocks->ch_0 |= VTSS_CH_XAUI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_OTN_1_0_INT_ENABLE)  blocks->ch_0 |= VTSS_CH_OTN1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_WIS_1_0_INT_ENABLE)  blocks->ch_0 |= VTSS_CH_WIS1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_PCS_1_0_INT_ENABLE)  blocks->ch_0 |= VTSS_CH_PCS1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_GFP_1_0_INT_ENABLE)  blocks->ch_0 |= VTSS_CH_GFP1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_MAC_1_0_INT_ENABLE)  blocks->ch_0 |= VTSS_CH_MAC1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_RAB_0_INT_ENABLE)    blocks->ch_0 |= VTSS_CH_RAB_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_MAC_2_0_INT_ENABLE)  blocks->ch_0 |= VTSS_CH_MAC2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_GFP_2_0_INT_ENABLE)  blocks->ch_0 |= VTSS_CH_GFP2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_PCS_2_0_INT_ENABLE)  blocks->ch_0 |= VTSS_CH_PCS2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_WIS_2_0_INT_ENABLE)  blocks->ch_0 |= VTSS_CH_WIS2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_OTN_2_0_INT_ENABLE)  blocks->ch_0 |= VTSS_CH_OTN2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_EFEC_0_INT_ENABLE)   blocks->ch_0 |= VTSS_CH_EFEC_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_UPI_0_INT_ENABLE)    blocks->ch_0 |= VTSS_CH_UPI_INT;

    DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1, &value);

    blocks->ch_1 = 0;

    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_TFI_1_INT_ENABLE)    blocks->ch_1 |= VTSS_CH_TFI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_SFI_1_INT_ENABLE)    blocks->ch_1 |= VTSS_CH_SFI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_XAUI_1_INT_ENABLE)   blocks->ch_1 |= VTSS_CH_XAUI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_OTN_1_1_INT_ENABLE)  blocks->ch_1 |= VTSS_CH_OTN1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_WIS_1_1_INT_ENABLE)  blocks->ch_1 |= VTSS_CH_WIS1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_PCS_1_1_INT_ENABLE)  blocks->ch_1 |= VTSS_CH_PCS1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_GFP_1_1_INT_ENABLE)  blocks->ch_1 |= VTSS_CH_GFP1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_MAC_1_1_INT_ENABLE)  blocks->ch_1 |= VTSS_CH_MAC1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_RAB_1_INT_ENABLE)    blocks->ch_1 |= VTSS_CH_RAB_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_MAC_2_1_INT_ENABLE)  blocks->ch_1 |= VTSS_CH_MAC2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_GFP_2_1_INT_ENABLE)  blocks->ch_1 |= VTSS_CH_GFP2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_PCS_2_1_INT_ENABLE)  blocks->ch_1 |= VTSS_CH_PCS2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_WIS_2_1_INT_ENABLE)  blocks->ch_1 |= VTSS_CH_WIS2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_OTN_2_1_INT_ENABLE)  blocks->ch_1 |= VTSS_CH_OTN2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_EFEC_1_INT_ENABLE)   blocks->ch_1 |= VTSS_CH_EFEC_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_UPI_1_INT_ENABLE)    blocks->ch_1 |= VTSS_CH_UPI_INT;

    DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC, &value);

    blocks->misc = 0;

    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_I2C_INT_ENABLE)      blocks->misc |= VTSS_MISC_I2C_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_I2C_INT_ENABLE)      blocks->misc |= VTSS_MISC_PMTICK_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_IP1588_0_INT_ENABLE) blocks->misc |= VTSS_MISC_IP1588_0_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_IP1588_1_INT_ENABLE) blocks->misc |= VTSS_MISC_IP1588_1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_EFEC7_0_INT_ENABLE)  blocks->misc |= VTSS_MISC_EFEC7_0_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_EFEC7_1_INT_ENABLE)  blocks->misc |= VTSS_MISC_EFEC7_1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_0_INT_ENABLE)   blocks->misc |= VTSS_MISC_LXFI_0_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_1_INT_ENABLE)   blocks->misc |= VTSS_MISC_LXFI_1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_0_INT_ENABLE)   blocks->misc |= VTSS_MISC_CXFI_0_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_1_INT_ENABLE)   blocks->misc |= VTSS_MISC_CXFI_1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_GPIO_INT_ENABLE)     blocks->misc |= VTSS_MISC_GPIO_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CPU_IF_INT_ENABLE)   blocks->misc |= VTSS_MISC_CPU_IF_INT;

    return VTSS_RC_OK;
}


static vtss_rc daytona_interrupt_status_get(vtss_state_t *vtss_state,
                                            vtss_interrupt_block_t   *const status)
{
    u32 value;

    DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0, &value);

    status->ch_0 = 0;

    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_TFI_0_INT_STAT)    status->ch_0 |= VTSS_CH_TFI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_SFI_0_INT_STAT)    status->ch_0 |= VTSS_CH_SFI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_XAUI_0_INT_STAT)   status->ch_0 |= VTSS_CH_XAUI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_OTN_1_0_INT_STAT)  status->ch_0 |= VTSS_CH_OTN1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_WIS_1_0_INT_STAT)  status->ch_0 |= VTSS_CH_WIS1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_PCS_1_0_INT_STAT)  status->ch_0 |= VTSS_CH_PCS1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_GFP_1_0_INT_STAT)  status->ch_0 |= VTSS_CH_GFP1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_MAC_1_0_INT_STAT)  status->ch_0 |= VTSS_CH_MAC1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_RAB_0_INT_STAT)    status->ch_0 |= VTSS_CH_RAB_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_MAC_2_0_INT_STAT)  status->ch_0 |= VTSS_CH_MAC2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_GFP_2_0_INT_STAT)  status->ch_0 |= VTSS_CH_GFP2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_PCS_2_0_INT_STAT)  status->ch_0 |= VTSS_CH_PCS2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_WIS_2_0_INT_STAT)  status->ch_0 |= VTSS_CH_WIS2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_OTN_2_0_INT_STAT)  status->ch_0 |= VTSS_CH_OTN2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_EFEC_0_INT_STAT)   status->ch_0 |= VTSS_CH_EFEC_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_UPI_0_INT_STAT)    status->ch_0 |= VTSS_CH_UPI_INT;

    DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1, &value);

    status->ch_1 = 0;

    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_TFI_1_INT_STAT)    status->ch_1 |= VTSS_CH_TFI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_SFI_1_INT_STAT)    status->ch_1 |= VTSS_CH_SFI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_XAUI_1_INT_STAT)   status->ch_1 |= VTSS_CH_XAUI_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_OTN_1_1_INT_STAT)  status->ch_1 |= VTSS_CH_OTN1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_WIS_1_1_INT_STAT)  status->ch_1 |= VTSS_CH_WIS1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_PCS_1_1_INT_STAT)  status->ch_1 |= VTSS_CH_PCS1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_GFP_1_1_INT_STAT)  status->ch_1 |= VTSS_CH_GFP1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_MAC_1_1_INT_STAT)  status->ch_1 |= VTSS_CH_MAC1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_RAB_1_INT_STAT)    status->ch_1 |= VTSS_CH_RAB_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_MAC_2_1_INT_STAT)  status->ch_1 |= VTSS_CH_MAC2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_GFP_2_1_INT_STAT)  status->ch_1 |= VTSS_CH_GFP2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_PCS_2_1_INT_STAT)  status->ch_1 |= VTSS_CH_PCS2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_WIS_2_1_INT_STAT)  status->ch_1 |= VTSS_CH_WIS2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_OTN_2_1_INT_STAT)  status->ch_1 |= VTSS_CH_OTN2_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_EFEC_1_INT_STAT)   status->ch_1 |= VTSS_CH_EFEC_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_UPI_1_INT_STAT)    status->ch_1 |= VTSS_CH_UPI_INT;

    DAYTONA_RD(VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC, &value);

    status->misc = 0;

    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_I2C_INT_STAT)      status->misc |= VTSS_MISC_I2C_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_PMTICK_INT_STAT)   status->misc |= VTSS_MISC_PMTICK_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_IP1588_0_INT_STAT) status->misc |= VTSS_MISC_IP1588_0_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_IP1588_1_INT_STAT) status->misc |= VTSS_MISC_IP1588_1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_EFEC7_0_INT_STAT)  status->misc |= VTSS_MISC_EFEC7_0_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_EFEC7_1_INT_STAT)  status->misc |= VTSS_MISC_EFEC7_1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_LXFI_0_INT_STAT)   status->misc |= VTSS_MISC_LXFI_0_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_LXFI_1_INT_STAT)   status->misc |= VTSS_MISC_LXFI_1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CXFI_0_INT_STAT)   status->misc |= VTSS_MISC_CXFI_0_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CXFI_1_INT_STAT)   status->misc |= VTSS_MISC_CXFI_1_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_GPIO_INT_STAT)     status->misc |= VTSS_MISC_GPIO_INT;
    if (value & VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CPU_IF_INT_STAT)   status->misc |= VTSS_MISC_CPU_IF_INT;

    return VTSS_RC_OK;
}



/* ================================================================= *
 *  Initialization
 * ================================================================= */
//


#if defined(VTSS_FEATURE_WARM_START)
#define DAYTONA_API_VERSION 1

/*
 * Restart port configuration
 */
static vtss_rc daytona_port_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    /* Update chip with current state data */
#if defined(VTSS_FEATURE_WIS)
    VTSS_DAYTONA_RC(vtss_daytona_wis_restart_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_WIS */
#if defined(VTSS_FEATURE_UPI)
    VTSS_DAYTONA_RC(vtss_daytona_upi_restart_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_UPI */
#if defined(VTSS_FEATURE_XAUI)
    VTSS_DAYTONA_RC(vtss_daytona_xaui_restart_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_XAUI */
#if defined(VTSS_FEATURE_OTN)
    VTSS_DAYTONA_RC(vtss_daytona_otn_restart_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_XAUI */
#if defined(VTSS_FEATURE_XFI)
    VTSS_DAYTONA_RC(vtss_daytona_xfi_restart_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_XFI */
#if defined(VTSS_FEATURE_RAB)
    VTSS_DAYTONA_RC(vtss_daytona_rab_restart_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_RAB */
#if defined(VTSS_FEATURE_GFP)
    VTSS_DAYTONA_RC(vtss_daytona_gfp_restart_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_GFP */
#if defined(VTSS_FEATURE_PCS_10GBASE_R)
    VTSS_DAYTONA_RC(vtss_daytona_10gbase_r_restart_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_PCS_10GBASE_R */
#if defined(VTSS_FEATURE_MAC10G)
    VTSS_DAYTONA_RC(vtss_daytona_mac10g_restart_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_MAC10G */
#if defined(VTSS_FEATURE_I2C)
//    VTSS_DAYTONA_RC(vtss_daytona_i2c_restart_conf_set(vtss_state));
#endif /* VTSS_FEATURE_I2C */
#if defined(VTSS_FEATURE_OHA)
    VTSS_DAYTONA_RC(vtss_daytona_oha_restart_conf_set(vtss_state, port_no));
#endif /* VTSS_FEATURE_OHA */
    
    return VTSS_RC_OK;
}    

/*
 * Restart daytona configuration
 */
static vtss_rc daytona_restart_conf_set(vtss_state_t *vtss_state)
{
    u32 value, mask;
    vtss_port_no_t port_no;
    VTSS_D("daytona_restart_conf_set");

    value = (VTSS_ENCODE_BITFIELD((u32)vtss_state->restart_cur,
                                  VTSS_RESTART_TYPE_OFFSET,
                                  VTSS_RESTART_TYPE_WIDTH) |
             VTSS_ENCODE_BITFIELD(DAYTONA_API_VERSION,
                                  VTSS_RESTART_VERSION_OFFSET,
                                  VTSS_RESTART_VERSION_WIDTH));
    mask = ((VTSS_BITMASK(VTSS_RESTART_VERSION_WIDTH) << VTSS_RESTART_VERSION_OFFSET) |
            (VTSS_BITMASK(VTSS_RESTART_TYPE_WIDTH)    << VTSS_RESTART_TYPE_OFFSET));
    DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_0, value, mask);

    for (port_no = 0 ; port_no < vtss_state->port_count; port_no++) 
        VTSS_RC(daytona_port_restart_conf_set(vtss_state, port_no));       
 
    return VTSS_RC_OK;
}

#endif

/*
 * Set default configuration for port
 */
static vtss_rc daytona_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    VTSS_RC(vtss_daytona_wis_default_conf_set(vtss_state, port_no));
    VTSS_RC(vtss_daytona_upi_default_conf_set(vtss_state, port_no));
    VTSS_RC(vtss_daytona_xaui_default_conf_set(vtss_state, port_no));
    VTSS_RC(vtss_daytona_otn_default_conf_set(vtss_state, port_no));
    VTSS_RC(vtss_daytona_xfi_default_conf_set(vtss_state, port_no));
    VTSS_RC(vtss_daytona_rab_default_conf_set(vtss_state, port_no));
    VTSS_RC(vtss_daytona_gfp_default_conf_set(vtss_state, port_no));
    VTSS_RC(vtss_daytona_pcs_default_conf_set(vtss_state, port_no));
    VTSS_RC(vtss_daytona_mac10g_default_conf_set(vtss_state, port_no));
    VTSS_RC(vtss_daytona_oha_default_conf_set(vtss_state, port_no));

    return VTSS_RC_OK;
}

static mode_name_t mode_convert(vtss_config_mode_t  conf_mode)
{
    switch (conf_mode)
    {
        case VTSS_CONFIG_MODE_PSS:                  return(MODE_PSS);
        case VTSS_CONFIG_MODE_PSS_W:                return(MODE_PSS_W);
        case VTSS_CONFIG_MODE_PSOC_A:               return(MODE_PSOC_A);
        case VTSS_CONFIG_MODE_PSOC_WA:              return(MODE_PSOC_WA);
        case VTSS_CONFIG_MODE_PSOC_S:               return(MODE_PSOC_S);
        case VTSS_CONFIG_MODE_PSOC_WS:              return(MODE_PSOC_WS);
        case VTSS_CONFIG_MODE_PEO_WMWS:             return(MODE_PEO_WMWS);
        case VTSS_CONFIG_MODE_PEO_WMWA:             return(MODE_PEO_WMWA);
        case VTSS_CONFIG_MODE_PEE_W:                return(MODE_PEE_W);
        case VTSS_CONFIG_MODE_PEE_MW:               return(MODE_PEE_MW);
        case VTSS_CONFIG_MODE_XAUI_2M_PEE_MW:       return(MODE_XAUI_2M_PEE_MW);
        case VTSS_CONFIG_MODE_PEE_P:                return(MODE_PEE_P);
        case VTSS_CONFIG_MODE_PEE_MP:               return(MODE_PEE_MP);
        case VTSS_CONFIG_MODE_XAUI_2M_PEE_MP:       return(MODE_XAUI_2M_PEE_MP);
        case VTSS_CONFIG_MODE_PFF_P:                return(MODE_PFF_P);
        case VTSS_CONFIG_MODE_PEO_WA:               return(MODE_PEO_WA);
        case VTSS_CONFIG_MODE_PEO_MWA:              return(MODE_PEO_MWA);
        case VTSS_CONFIG_MODE_PEO_WS:               return(MODE_PEO_WS);
        case VTSS_CONFIG_MODE_PEO_MWS:              return(MODE_PEO_MWS);
        case VTSS_CONFIG_MODE_PEO_P2E_20:           return(MODE_PEO_P2E_20);
        case VTSS_CONFIG_MODE_PEO_MP2E_20:          return(MODE_PEO_MP2E_20);
        case VTSS_CONFIG_MODE_PEO_P2E_100:          return(MODE_PEO_P2E_100);
        case VTSS_CONFIG_MODE_PEO_P1E_100:          return(MODE_PEO_P1E_100);
        case VTSS_CONFIG_MODE_XAUI_2M_PEO_P1E_100:  return(MODE_XAUI_2M_PEO_P1E_100);
        case VTSS_CONFIG_MODE_PFO_P2F:              return(MODE_PFO_P2F);
        case VTSS_CONFIG_MODE_PGO_2:                return(MODE_PGO_2);
        case VTSS_CONFIG_MODE_PGO_2SUB:             return(MODE_PGO_2SUB);
        case VTSS_CONFIG_MODE_PGO_2SUBSMALL:        return(MODE_PGO_2SUBSMALL);
        case VTSS_CONFIG_MODE_PGO_2GEN:             return(MODE_PGO_2GEN);
        case VTSS_CONFIG_MODE_BOO_2:                return(MODE_BOO_2);
        case VTSS_CONFIG_MODE_BOO_1E:               return(MODE_BOO_1E);
        case VTSS_CONFIG_MODE_BOO_2E:               return(MODE_BOO_2E);
        case VTSS_CONFIG_MODE_BOO_2F:               return(MODE_BOO_2F);
        case VTSS_CONFIG_MODE_BMX_2:                return(MODE_BMX_2);
        case VTSS_CONFIG_MODE_BMX_2E:               return(MODE_BMX_2E);
        case VTSS_CONFIG_MODE_BMX_1E:               return(MODE_BMX_1E);
        case VTSS_CONFIG_MODE_BMX_2F:               return(MODE_BMX_2F);
        case VTSS_CONFIG_MODE_BOE_WMWA:             return(MODE_BOE_WMWA);
        case VTSS_CONFIG_MODE_BOE_WMWS:             return(MODE_BOE_WMWS);
        case VTSS_CONFIG_MODE_BOE_PMP2E_20:         return(MODE_BOE_PMP2E_20);
        case VTSS_CONFIG_MODE_BOE_PMP1E_20:         return(MODE_BOE_PMP1E_20);
        case VTSS_CONFIG_MODE_BOEC_2E:              return(MODE_BOEC_2E);
        case VTSS_CONFIG_MODE_BOEC_1E:              return(MODE_BOEC_1E);
        case VTSS_CONFIG_MODE_BOG_2:                return(MODE_BOG_2);
        case VTSS_CONFIG_MODE_BOG_2SUB:             return(MODE_BOG_2SUB);
        case VTSS_CONFIG_MODE_BOG_W2:               return(MODE_BOG_W2);
        case VTSS_CONFIG_MODE_BOSC_A:               return(MODE_BOSC_A);
        case VTSS_CONFIG_MODE_BOSC_WA:              return(MODE_BOSC_WA);
        case VTSS_CONFIG_MODE_BOSC_S:               return(MODE_BOSC_S);
        case VTSS_CONFIG_MODE_BOSC_WS:              return(MODE_BOSC_WS);
        case VTSS_CONFIG_MODE_BOFC_2F:              return(MODE_BOFC_2F);
        case VTSS_CONFIG_MODE_BO8F:                 return(MODE_BO8F);
        case VTSS_CONFIG_MODE_TSOC_A:               return(MODE_TSOC_A);
        case VTSS_CONFIG_MODE_TSOC_WA:              return(MODE_TSOC_WA);
        case VTSS_CONFIG_MODE_TSOC_S:               return(MODE_TSOC_S);
        case VTSS_CONFIG_MODE_TSOC_WS:              return(MODE_TSOC_WS);
        case VTSS_CONFIG_MODE_TEO_PMP_2E:           return(MODE_TEO_PMP2E);
        case VTSS_CONFIG_MODE_TEO_PMP_1E:           return(MODE_TEO_PMP1E);
        case VTSS_CONFIG_MODE_TEO_PP_2E:            return(MODE_TEO_PP2E);
        case VTSS_CONFIG_MODE_TFO_PP2F:             return(MODE_TFO_PP2F);
        case VTSS_CONFIG_MODE_TEOC_2E:              return(MODE_TEOC_2E);
        case VTSS_CONFIG_MODE_TEOC_1E:              return(MODE_TEOC_1E);
        case VTSS_CONFIG_MODE_TFOC_2F:              return(MODE_TFOC_2F);
        case VTSS_CONFIG_MODE_TEO_WMWA:             return(MODE_TEO_WMWA);
        case VTSS_CONFIG_MODE_TEO_WMWS:             return(MODE_TEO_WMWS);
        case VTSS_CONFIG_MODE_TGO_2:                return(MODE_TGO_2);
        case VTSS_CONFIG_MODE_TGO_2SUB:             return(MODE_TGO_2SUB);
        case VTSS_CONFIG_MODE_TGO_2SUBSMALL:        return(MODE_TGO_2SUBSMALL);
        case VTSS_CONFIG_MODE_TGO_W2:               return(MODE_TGO_W2);
        case VTSS_CONFIG_MODE_T8FO:                 return(MODE_T8FO);
        case VTSS_CONFIG_MODE_TOO_2:                return(MODE_TOO_2);
        case VTSS_CONFIG_MODE_TOO_2E2:              return(MODE_TOO_2E2);
        case VTSS_CONFIG_MODE_TOO_1E2:              return(MODE_TOO_1E2);
        case VTSS_CONFIG_MODE_TOO_2F2:              return(MODE_TOO_2F2);
        case VTSS_CONFIG_MODE_TEE_L:                return(MODE_TEE_L);
        case VTSS_CONFIG_MODE_TEE_PMP_S:            return(MODE_TEE_PMP_S);
        case VTSS_CONFIG_MODE_TEE_PMP_A:            return(MODE_TEE_PMP_A);
        case VTSS_CONFIG_MODE_TEE_PP:               return(MODE_TEE_PP);
        case VTSS_CONFIG_MODE_TEE_W:                return(MODE_TEE_W);
        case VTSS_CONFIG_MODE_TEE_PMW:              return(MODE_TEE_PMW);
        case VTSS_CONFIG_MODE_TFF:                  return(MODE_TFF);
        case VTSS_CONFIG_MODE_TSS:                  return(MODE_TSS);
        case VTSS_CONFIG_MODE_DISABLED:             return(MODE_DISABLED);
        case VTSS_CONFIG_MODE_MAX:                  return(MODE_PSS);
        default:                                    return(MODE_PSS);
    }
}

static vtss_rc static_conf_set(vtss_state_t *vtss_state,
                               u32 channel, u32 side, const static_cfg_t *config_table,  u32 size) 
{
    u32 target, i;

    target = 0;
    for (i=0; i<size; ++i) {
        /* Only blocks with more than one target must add the target address */
        if ((config_table[i].block != DAYTONA_BLOCK_DEVCPU_ORG) && (config_table[i].block != DAYTONA_BLOCK_FAST_REGS) && (config_table[i].block != DAYTONA_BLOCK_DEVCPU_GCB)){
            VTSS_RC(daytona_target(channel, side, config_table[i].block, &target));
        }
        DAYTONA_WRM(target + config_table[i].offset, config_table[i].value, config_table[i].mask);

    }

    return VTSS_RC_OK;
}

static vtss_rc daytona_conf_mode_set(vtss_state_t *vtss_state,
                                     u16 channel, vtss_config_mode_t conf_mode, u32 two_lane_upi)
{
    u16 chip_variant;
    u32 variant_mask;
    mode_name_t mode;
    //u32 block_no;

    /* read chip variant */
    VTSS_RC(daytona_chip_var_get(vtss_state, &chip_variant));
    if (chip_variant >= (sizeof(variant_masks)/sizeof(u32))) {
        VTSS_E("Undefined Daytona variant: %d",chip_variant);
        return VTSS_RC_ERROR;
    }
    /* check if the requested mode is valid */
    if ((uint)conf_mode >= (sizeof(feature_mask)/sizeof(u32))-1) {
        VTSS_E("Invalid configuration mode %d", conf_mode);
        return VTSS_RC_ERROR;
    }
    /* check if the variant supports the requested mode */
    variant_mask = variant_masks[chip_variant];
    if (!(variant_mask & feature_mask[conf_mode])) {
        VTSS_E("Daytona variant %d does not support mode %d",chip_variant, conf_mode);
        return VTSS_RC_ERROR;
    }

    mode = mode_convert(conf_mode);
    VTSS_I("Daytona Channel %d Mode %d",channel, mode);
#if defined(VTSS_DAYTONA_INIT_INTERFACE)
    printf("Block modes: block id  mode\n");
    for (block_no = 0; block_no < BLOCK_LAST; block_no++) {
        printf("             %8u  %4d\n", block_no, block_mode[mode][block_no]);
    }
    printf("Block modes: block id  mode\n");
    for (block_no = 0; block_no < BLOCK_LAST; block_no++) {
        printf("             %8u  %4d\n", block_no, block_mode[mode][block_no]);
    }
    VTSS_RC(static_conf_set(vtss_state, channel, 0, ((channel) ? gcb_1_config_table[block_mode[mode][BLOCK_GCB]] : gcb_0_config_table[block_mode[mode][BLOCK_GCB]]), GCB_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, ((channel) ? gcb_client_pll_1_config_table[block_mode[mode][BLOCK_GCB_CLIENT_PLL]] : gcb_client_pll_0_config_table[block_mode[mode][BLOCK_GCB_CLIENT_PLL]]), GCB_CLIENT_PLL_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, upi_config_table[block_mode[mode][BLOCK_UPI]], UPI_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, upi_hss6g_config_table[block_mode[mode][BLOCK_UPI_HSS6G]], UPI_HSS6G_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, xaui_config_table[block_mode[mode][BLOCK_XAUI]], XAUI_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, tfi_config_table[block_mode[mode][BLOCK_TFI]], TFI_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, sfi_config_table[block_mode[mode][BLOCK_SFI]], SFI_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, xfi_config_table[block_mode[mode][BLOCK_XFI_1]], XFI_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, xfi_hss11g_config_table[block_mode[mode][BLOCK_XFI_HSS11G_1]], XFI_HSS11G_TBL_SIZE));
#endif /* VTSS_DAYTONA_INIT_INTERFACE */

    VTSS_RC(static_conf_set(vtss_state, channel, 0, tfi_config_table[block_mode[mode][BLOCK_TFI]], TFI_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, sfi_config_table[block_mode[mode][BLOCK_SFI]], SFI_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, otu_config_table[block_mode[mode][BLOCK_OTU_1]], OTU_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, otu_fec_config_table[block_mode[mode][BLOCK_OTU_FEC_1]], OTU_FEC_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, gfp_config_table[block_mode[mode][BLOCK_GFP_1]], GFP_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, wis_config_table[block_mode[mode][BLOCK_WIS_1]], WIS_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, pcs_config_table[block_mode[mode][BLOCK_PCS_1]], PCS_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, mac_config_table[block_mode[mode][BLOCK_MAC_1]], MAC_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 1, ptp_config_table[block_mode[mode][BLOCK_PTP]], PTP_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, rab_config_table[block_mode[mode][BLOCK_RAB]], RAB_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 1, mac_config_table[block_mode[mode][BLOCK_MAC_2]], MAC_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 1, pcs_config_table[block_mode[mode][BLOCK_PCS_2]], PCS_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 1, wis_config_table[block_mode[mode][BLOCK_WIS_2]], WIS_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 1, gfp_config_table[block_mode[mode][BLOCK_GFP_2]], GFP_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 1, otu_config_table[block_mode[mode][BLOCK_OTU_2]], OTU_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 1, otu_fec_config_table[block_mode[mode][BLOCK_OTU_FEC_2]], OTU_FEC_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 1, predictor_config_table[block_mode[mode][BLOCK_PREDICTOR]], PREDICTOR_TBL_SIZE));

#if defined(VTSS_DAYTONA_INIT_INTERFACE)
    VTSS_RC(static_conf_set(vtss_state, channel, 0, ((channel) ? gcb_1_config_table[block_mode[mode][BLOCK_GCB]] : gcb_0_config_table[block_mode[mode][BLOCK_GCB]]), GCB_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, ((channel) ? gcb_client_pll_1_config_table[block_mode[mode][BLOCK_GCB_CLIENT_PLL]] : gcb_client_pll_0_config_table[block_mode[mode][BLOCK_GCB_CLIENT_PLL]]), GCB_CLIENT_PLL_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 0, ((channel) ? gcb_line_pll_1_config_table[block_mode[mode][BLOCK_GCB_LINE_PLL]] : gcb_line_pll_0_config_table[block_mode[mode][BLOCK_GCB_LINE_PLL]]), GCB_LINE_PLL_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 1, xfi_config_table[block_mode[mode][BLOCK_XFI_2]], XFI_TBL_SIZE));
    VTSS_RC(static_conf_set(vtss_state, channel, 1, xfi_hss11g_config_table[block_mode[mode][BLOCK_XFI_HSS11G_2]], XFI_HSS11G_TBL_SIZE));
#endif /* VTSS_DAYTONA_INIT_INTERFACE */

    return VTSS_RC_OK;
}

static vtss_rc daytona_interface_conf_mode_set(vtss_state_t *vtss_state,
                                               u16 channel, vtss_config_mode_t conf_mode, u32 two_lane_upi, u32 xfi_clk, u32 x4_case)
{

    u16 chip_variant;
    u32 variant_mask;
    mode_name_t mode;
    //u32 block_no;
    u32 channel_rst_value;
    u32 channel_rst_mask;
    u32 my_block_mode, sfi_block_mode, tfi_block_mode;
    u32 tgt;
    u32 reg_val;
    u32 reg_mask;

    //u32 x;

    /* read chip variant */
    VTSS_RC(daytona_chip_var_get(vtss_state, &chip_variant));
    if (chip_variant >= (sizeof(variant_masks)/sizeof(u32))) {
        VTSS_E("Undefined Daytona variant: %d",chip_variant);
        return VTSS_RC_ERROR;
    }
    /* check if the requested mode is valid */
    if ((uint)conf_mode >= (sizeof(feature_mask)/sizeof(u32))-1) {
        VTSS_E("Invalid configuration mode %d", conf_mode);
        return VTSS_RC_ERROR;
    }
    /* check if the variant supports the requested mode */
    variant_mask = variant_masks[chip_variant];
    if (!(variant_mask & feature_mask[conf_mode])) {
        VTSS_E("Daytona variant %d does not support mode %d",chip_variant, conf_mode);
        return VTSS_RC_ERROR;
    }


    mode = mode_convert(conf_mode);
    VTSS_I("Daytona Channel %d Mode %d",channel, mode);
    if (mode != MODE_PEE_MP && mode != MODE_PEE_P && 
        mode != MODE_PEE_MW && mode != MODE_PEE_W && 
        mode != MODE_XAUI_2M_PEE_MP && mode != MODE_XAUI_2M_PEE_MW && 
        mode != MODE_PEO_P1E_100 && mode != MODE_XAUI_2M_PEO_P1E_100 && 
        mode != MODE_PEO_P2E_100 && mode != MODE_PEO_WS && 
        mode != MODE_PEO_MWS && mode != MODE_PEO_MWA && 
        mode != MODE_PEO_WA && mode != MODE_PEO_P2E_20 && 
        mode != MODE_PEO_MP2E_20 && mode != MODE_TEO_PMP2E && 
        mode != MODE_PGO_2 && mode != MODE_TGO_2 && 
        mode != MODE_BOO_2 && mode != MODE_TEE_L &&
        mode != MODE_TEE_PMP_S && mode != MODE_TEE_PMP_A &&
        mode != MODE_TEE_W && mode != MODE_TFF &&
        mode != MODE_TSS && mode != MODE_TGO_2SUB &&
        mode != MODE_TEOC_2E && mode != MODE_TGO_2SUBSMALL &&
        mode != MODE_TEO_PMP1E && mode != MODE_PGO_2SUB && 
        mode != MODE_TSOC_S && mode != MODE_TOO_2 && 
        mode != MODE_TOO_2E2 && mode != MODE_TOO_2F2 &&
        mode != MODE_TFO_PP2F && mode != MODE_TSOC_WS && 
        mode != MODE_TEE_PP && mode != MODE_TEOC_1E  &&
        mode != MODE_TEO_PP2E && mode != MODE_BMX_2 &&
        mode != MODE_TEE_PMW) {
        VTSS_E("Daytona variant %d does not support mode %d for now",chip_variant, mode);
        return VTSS_RC_ERROR;
    }

    channel_rst_value = (channel == 0) ? VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0(1) : VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1(1);

    channel_rst_mask = (channel == 0) ? VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0 : VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1;

    //erf_wr -inst:daytona_0 cfg_if_gcb devcpu_rst_regs channel_rst_0 1
    DAYTONA_WRM (VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS, channel_rst_value, channel_rst_mask);

    /* GCB-Line-PLL */
    my_block_mode = block_mode[mode][BLOCK_GCB_LINE_PLL];
    if ((mode == MODE_PEE_P || mode == MODE_PEE_MP)&& (xfi_clk == 161))
      my_block_mode = BM_GCB_LINE_PLL_10GELAN;
    if ((x4_case == 1) || (x4_case == 2)) {
        if (mode == MODE_TSS)
            my_block_mode = BM_GCB_LINE_PLL_SONET_X4;
        else if (mode == MODE_TSOC_S)
            my_block_mode = BM_GCB_LINE_PLL_OTU2_X4;
        else if (mode == MODE_TOO_2)
            my_block_mode = BM_GCB_LINE_PLL_OTU2_X4;
        else if (mode == MODE_TOO_2E2)
            my_block_mode = BM_GCB_LINE_PLL_OTU2E_X4;
    }    
    VTSS_RC(static_conf_set(vtss_state, channel, 0, ((channel) ? gcb_line_pll_1_config_table[my_block_mode] : gcb_line_pll_0_config_table[my_block_mode]), GCB_LINE_PLL_TBL_SIZE)); 
    /* PLL CTL target */
    tgt = (channel == 0) ? VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL : VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL;
    reg_val = (channel == 0) ? VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_RESET(0) : VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_RESET(0);
    reg_mask = (channel == 0) ? VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_RESET : VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_RESET;

    DAYTONA_WRM (tgt, reg_val, reg_mask);

    VTSS_RC(static_conf_set(vtss_state, channel, 0, ((channel) ? gcb_line_pll_1_config_table[my_block_mode] : gcb_line_pll_0_config_table[my_block_mode]), GCB_LINE_PLL_TBL_SIZE));

    /* GCB - MUX */
    VTSS_RC(static_conf_set(vtss_state, channel, 0, ((channel) ? gcb_1_config_table[block_mode[mode][BLOCK_GCB]] : gcb_0_config_table[block_mode[mode][BLOCK_GCB]]), GCB_TBL_SIZE));

    /* GCB-Client-PLL */
    my_block_mode = block_mode[mode][BLOCK_GCB_CLIENT_PLL];
    if (my_block_mode == BM_GCB_CLIENT_PLL_XAUI_10GE_SDR && two_lane_upi) {
        my_block_mode = BM_GCB_CLIENT_PLL_XAUI_10GE_DDR;
    }
    if ((x4_case == 1) || (x4_case == 2)) {
        if (mode == MODE_TSS)
            my_block_mode = BM_GCB_CLIENT_PLL_SONET_X4;
        else if (mode == MODE_TSOC_S)
            my_block_mode = BM_GCB_CLIENT_PLL_SONET_X4;
        else if (mode == MODE_TOO_2)
            my_block_mode = BM_GCB_CLIENT_PLL_OTU2_X4;
        else if (mode == MODE_TOO_2E2)
            my_block_mode = BM_GCB_CLIENT_PLL_OTU2E_X4;
    }    
    VTSS_RC(static_conf_set(vtss_state, channel, 0, ((channel) ? gcb_client_pll_1_config_table[my_block_mode] : gcb_client_pll_0_config_table[my_block_mode]), GCB_CLIENT_PLL_TBL_SIZE));
    tgt = (channel == 0) ? VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL : VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL;
    reg_val = (channel == 0) ? VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_RESET(0) : VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_RESET(0);
    reg_mask = (channel == 0) ? VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_RESET : VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_RESET;

    DAYTONA_WRM (tgt, reg_val, reg_mask);
    VTSS_RC(static_conf_set(vtss_state, channel, 0, ((channel) ? gcb_client_pll_1_config_table[my_block_mode] : gcb_client_pll_0_config_table[my_block_mode]), GCB_CLIENT_PLL_TBL_SIZE));

    //erf_wr -inst:daytona_0 cfg_if_gcb devcpu_rst_regs channel_rst_0 0
    channel_rst_value = (channel == 0) ? VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0(0) : VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1(0);

    channel_rst_mask = (channel == 0) ? VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0 : VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1;

    DAYTONA_WRM (VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS, channel_rst_value, channel_rst_mask);

    //#UPI
    //UPI target

    my_block_mode = block_mode[mode][BLOCK_UPI];
    if (my_block_mode != BM_UPI_UNUSED && my_block_mode != BM_UPI_LAST) {
        //VTSS_RC(static_conf_set(vtss_state, channel, 0, upi_config_table[block_mode[mode][BLOCK_UPI]], UPI_TBL_SIZE));

        tgt = (channel == 0) ? VTSS_TO_UPI_0 : VTSS_TO_UPI_1;
        //#UPI
        //erf_wr -inst:daytona_0 upi_0 upi_hss6g_pll divsel 1

        my_block_mode = block_mode[mode][BLOCK_UPI_HSS6G];
        if (my_block_mode == BM_UPI_HSS6G_XAUI_10GE_SDR && two_lane_upi) {
            my_block_mode = BM_UPI_HSS6G_XAUI_10GE_DDR;
        }
        VTSS_RC(static_conf_set(vtss_state, channel, 0, upi_hss6g_config_table[my_block_mode], UPI_HSS6G_TBL_SIZE));

        //erf_wr -inst:daytona_0 upi_0 upi_hss6g_pll reset 1
        DAYTONA_WRM (VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL(tgt),
                VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESET(1),
                VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESET);

        //erf_wr -inst:daytona_0 upi_0 upi_hss6g_pll reset 0
        DAYTONA_WRM (VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL(tgt),
                VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESET(0),
                VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESET);

        //erf_wr -inst:daytona_0 upi_0 upi_hss6g_pll acmode 1
        DAYTONA_WRM (VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL(tgt),
                VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_ACMODE(1),
                VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_ACMODE);

        //erf_wr -inst:daytona_0 upi_0 upi_hss6g_pll resync_input 1
        DAYTONA_WRM (VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL(tgt),
                VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESYNC_INPUT(1),
                VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESYNC_INPUT);

        //erf_wr -inst:daytona_0 upi_0 upi_hss6g_pll resync_input 0
        DAYTONA_WRM (VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL(tgt),
                VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESYNC_INPUT(0),
                VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESYNC_INPUT);

        DAYTONA_POLL_FIELD(VTSS_UPI_UPI_CONFIG_UPI_STATUS(tgt), 2, 1, 1);
        DAYTONA_POLL_FIELD(VTSS_UPI_UPI_CONFIG_UPI_STATUS(tgt), 13, 1, 1);

        my_block_mode = block_mode[mode][BLOCK_UPI_HSS6G];
        if (my_block_mode == BM_UPI_HSS6G_XAUI_10GE_SDR && two_lane_upi) {
            my_block_mode = BM_UPI_HSS6G_XAUI_10GE_DDR;
        }
        VTSS_RC(static_conf_set(vtss_state, channel, 0, upi_hss6g_config_table[my_block_mode], UPI_HSS6G_TBL_SIZE));

        my_block_mode = block_mode[mode][BLOCK_UPI];
        if (my_block_mode == BM_UPI_XAUI_10GE_SDR && two_lane_upi) {
            my_block_mode = BM_UPI_XAUI_10GE_DDR;
        }
        VTSS_RC(static_conf_set(vtss_state, channel, 0, upi_config_table[my_block_mode], UPI_TBL_SIZE));

        //erf_wr -inst:daytona_0 upi_0 upi_tx_config tx_sw_rst 1
        DAYTONA_WRM (VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG(tgt),
                VTSS_F_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_SW_RST(1),
                VTSS_M_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_SW_RST);

        //erf_wr -inst:daytona_0 upi_0 upi_tx_config tx_sw_rst 0
        DAYTONA_WRM (VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG(tgt),
                VTSS_F_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_SW_RST(0),
                VTSS_M_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_SW_RST);

        //erf_wr -inst:daytona_0 upi_0 upi_config sw_ena 1
        DAYTONA_WRM (VTSS_UPI_UPI_CONFIG_UPI_CONFIG(tgt),
                VTSS_F_UPI_UPI_CONFIG_UPI_CONFIG_SW_ENA(1),
                VTSS_M_UPI_UPI_CONFIG_UPI_CONFIG_SW_ENA);

        //erf_wr -inst:daytona_0 upi_0 hss6g_transmit_coefficient_control:4 appld 1
        DAYTONA_WRM (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(1),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);
        //erf_wr -inst:daytona_0 upi_0 hss6g_transmit_coefficient_control:4 appld 0
        DAYTONA_WRM (VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 4),
                VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(0),
                VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD);

        //erf_wr -inst:daytona_0 upi_0 upi_tx_hss6g_prbs tx_ts 0xf
        DAYTONA_WRM (VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS(tgt),
                VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_TS(0xF),
                VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_TS);

        //#XAUI
        //XAUI target
        my_block_mode = block_mode[mode][BLOCK_XAUI];
        sfi_block_mode = block_mode[mode][BLOCK_SFI];
        tfi_block_mode = block_mode[mode][BLOCK_TFI];
        if ((my_block_mode != BM_XAUI_UNUSED) && (my_block_mode != BM_XAUI_LAST)) {
            if (my_block_mode == BM_XAUI_SDR && two_lane_upi) {
                switch (two_lane_upi) {
                    case 1:
                        my_block_mode = BM_XAUI_DDR_A;
                        break; 
                    case 2:
                        my_block_mode = BM_XAUI_DDR_K;
                        break; 
                    case 3:
                my_block_mode = BM_XAUI_DDR_M;
                        break; 
                    default:
                        my_block_mode = BM_XAUI_DDR_M;
                        break;
                }
            }
            VTSS_RC(static_conf_set(vtss_state, channel, 0, xaui_config_table[my_block_mode], XAUI_TBL_SIZE));
        } else if ((sfi_block_mode != BM_SFI_UNUSED) && (sfi_block_mode != BM_SFI_LAST)) {
            VTSS_RC(static_conf_set(vtss_state, channel, 0, sfi_config_table[sfi_block_mode], SFI_TBL_SIZE));
        } else if ((tfi_block_mode != BM_TFI_UNUSED) && (tfi_block_mode != BM_TFI_LAST)) {
            VTSS_RC(static_conf_set(vtss_state, channel, 0, tfi_config_table[tfi_block_mode], TFI_TBL_SIZE));
        }
    } else {
    
        //#CXFI
        // CXFI Target
        tgt = (channel == 0) ? VTSS_TO_CXFI_0 : VTSS_TO_CXFI_1;

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
                VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL(0),
                VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
                VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL(0),
                VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
                VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET(1),
                VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
                VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET(0),
                VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET);

        /* Wait for the PLL lock */
        DAYTONA_POLL_FIELD(VTSS_XFI_XFI_CONTROL_HSS_STATUS(tgt), 0, 1, 1);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
                VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL(0),
                VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
                VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL(0),
                VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
                VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESYNC(1),
                VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESYNC);

        // waiting for HSSRESETOUT_STATUS to clear
        DAYTONA_POLL_FIELD(VTSS_XFI_XFI_CONTROL_HSS_STATUS(tgt), 2, 1, 0);
        /* Waiting for HSSPRTREADY */
        DAYTONA_POLL_FIELD(VTSS_XFI_XFI_CONTROL_HSS_STATUS(tgt), 6, 1, 1);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
                VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_TXATS(1),
                VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_TXATS);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
                VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_TXBTS(1),
                VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_TXBTS);
        my_block_mode = block_mode[mode][BLOCK_XFI_HSS11G_1];
        if (x4_case == 2) {
            if (mode == MODE_TSS)
                my_block_mode = BM_XFI_HSS11G_SONET_X4;
            else if (mode == MODE_TSOC_S)
                my_block_mode = BM_XFI_HSS11G_SONET_X4;
            else if (mode == MODE_TOO_2)
                my_block_mode = BM_XFI_HSS11G_OTU2_X4;
            else if (mode == MODE_TOO_2E2)
                my_block_mode = BM_XFI_HSS11G_OTU2E_X4;
        }    
        
        VTSS_RC(static_conf_set(vtss_state, channel, 0, xfi_hss11g_config_table[my_block_mode], XFI_HSS11G_TBL_SIZE));


        DAYTONA_WRM (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(tgt, 2),
                VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL(2),
                VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(tgt, 2),
                VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL(0),
                VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(tgt, 2),
                VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL(2),
                VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
                VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(1),
                VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);

        DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
                VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(0),
                VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);


        VTSS_RC(static_conf_set(vtss_state, channel, 0, xfi_config_table[block_mode[mode][BLOCK_XFI_1]], XFI_TBL_SIZE));

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_XFI_MODE(tgt),
                VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SW_ENA(1),
                VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SW_ENA);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_XFI_MODE(tgt),
                VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SW_RST(0),
                VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SW_RST);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_XFI_MODE(tgt),
                VTSS_F_XFI_XFI_CONTROL_XFI_MODE_TX_RESYNC(1),
                VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_RESYNC);

        DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_XFI_MODE(tgt),
                VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SW_ENA(1),
                VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SW_ENA);
    }

    //#LXFI
    // LXFI Target
    tgt = (channel == 0) ? VTSS_TO_LXFI_0 : VTSS_TO_LXFI_1;
    //erf_wr -inst:daytona_0 lxfi_0 hss_controls hssdivsel 0
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
            VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL(0),
            VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL);

    //erf_wr -inst:daytona_0 lxfi_0 hss_controls hsspdwnpll 0
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
            VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL(0),
            VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL);

    //erf_wr -inst:daytona_0 lxfi_0 hss_controls hssreset 1
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
            VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET(1),
            VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET);

    //erf_wr -inst:daytona_0 lxfi_0 hss_controls hssreset 0
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
            VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET(0),
            VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET);

    /* Wait for the PLL lock */
    DAYTONA_POLL_FIELD(VTSS_XFI_XFI_CONTROL_HSS_STATUS(tgt), 0, 1, 1);

    //erf_wr -inst:daytona_0 lxfi_0 hss_controls hssdivsel 0
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
            VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL(0),
            VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL);

    //erf_wr -inst:daytona_0 lxfi_0 hss_controls hsspdwnpll 0
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
            VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL(0),
            VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL);

    //erf_wr -inst:daytona_0 lxfi_0 hss_controls hssresync 1
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
            VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESYNC(1),
            VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESYNC);
    
    // waiting for HSSRESETOUT_STATUS to clear
    DAYTONA_POLL_FIELD(VTSS_XFI_XFI_CONTROL_HSS_STATUS(tgt), 2, 1, 0);
    /* Waiting for HSSPRTREADY */
    DAYTONA_POLL_FIELD(VTSS_XFI_XFI_CONTROL_HSS_STATUS(tgt), 6, 1, 1);
    
    //erf_wr -inst:daytona_0 lxfi_0 hss_controls txats 1
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
            VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_TXATS(1),
            VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_TXATS);

    //erf_wr -inst:daytona_0 lxfi_0 hss_controls txbts 1
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(tgt),
            VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_TXBTS(1),
            VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_TXBTS);
    my_block_mode = block_mode[mode][BLOCK_XFI_HSS11G_2];
    if (x4_case == 2) {
        if (mode == MODE_TSS){
            my_block_mode = BM_XFI_HSS11G_SONET_X4;
        }
        else if (mode == MODE_TSOC_S)
            my_block_mode = BM_XFI_HSS11G_OTU2_X4;
        else if (mode == MODE_TOO_2)
            my_block_mode = BM_XFI_HSS11G_OTU2_X4;
        else if (mode == MODE_TOO_2E2)
            my_block_mode = BM_XFI_HSS11G_OTU2E_X4;
    }    
    VTSS_RC(static_conf_set(vtss_state, channel, 1, xfi_hss11g_config_table[my_block_mode], XFI_HSS11G_TBL_SIZE));
    

    //erf_wr -inst:daytona_0 lxfi_0 hss11g_receiver_configuration_mode:2 bwsel 2
    DAYTONA_WRM (VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(tgt, 2),
            VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL(2),
            VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL);

    //erf_wr -inst:daytona_0 lxfi_0 hss11g_transmit_configuration_mode:2 rtsel 0
    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL);

    //erf_wr -inst:daytona_0 lxfi_0 hss11g_transmit_configuration_mode:2 bwsel 2
    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL(2),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL);

    //erf_wr -inst:daytona_0 lxfi_0 hss11g_transmit_coefficient_control:2 aload 1
    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(1),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);

    //erf_wr -inst:daytona_0 lxfi_0 hss11g_transmit_coefficient_control:2 aload 0
    DAYTONA_WRM (VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(tgt, 2),
            VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(0),
            VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD);


    VTSS_RC(static_conf_set(vtss_state, channel, 1, xfi_config_table[block_mode[mode][BLOCK_XFI_2]], XFI_TBL_SIZE));

    //erf_wr -inst:daytona_0 lxfi_0 xfi_mode sw_ena 1
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_XFI_MODE(tgt),
            VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SW_ENA(1),
            VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SW_ENA);

    //erf_wr -inst:daytona_0 lxfi_0 xfi_mode sw_rst 0
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_XFI_MODE(tgt),
            VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SW_RST(0),
            VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SW_RST);

    //erf_wr -inst:daytona_0 lxfi_0 xfi_mode tx_resync 1
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_XFI_MODE(tgt),
            VTSS_F_XFI_XFI_CONTROL_XFI_MODE_TX_RESYNC(1),
            VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_RESYNC);

    //erf_wr -inst:daytona_0 lxfi_0 xfi_mode sw_ena 1
    DAYTONA_WRM (VTSS_XFI_XFI_CONTROL_XFI_MODE(tgt),
            VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SW_ENA(1),
            VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SW_ENA);

    
    return VTSS_RC_OK;
}

static vtss_rc daytona_init_conf_set(vtss_state_t *vtss_state)
{
    u32                 value;
    vtss_init_conf_t    *conf = &vtss_state->init_conf;
    u32                 channel;
    u32                 channel_rst_value;
    u32                 channel_rst_mask;

#if defined(VTSS_FEATURE_WARM_START)
    /* Check for warm restart */
    DAYTONA_RD(VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_0, &value);
    vtss_state->restart_prev = (vtss_restart_t)VTSS_EXTRACT_BITFIELD(value,
                                                     VTSS_RESTART_TYPE_OFFSET,
                                                     VTSS_RESTART_TYPE_WIDTH);
    vtss_state->version_prev = VTSS_EXTRACT_BITFIELD(value,
                                                     VTSS_RESTART_VERSION_OFFSET,
                                                     VTSS_RESTART_VERSION_WIDTH);
    vtss_state->restart_cur = vtss_state->restart_prev;
    vtss_state->version_cur = DAYTONA_API_VERSION;
    if (vtss_state->restart_cur == VTSS_RESTART_WARM) {
        if (conf->warm_start_enable) {
            if (vtss_state->version_prev > vtss_state->version_cur) {
                VTSS_I("downgrade from version %u to %u",
                       vtss_state->version_prev, vtss_state->version_cur);
            } else {
                vtss_state->warm_start_cur = 1;
                VTSS_I("warm starting");
                return VTSS_RC_OK;
            }
        } else {
            VTSS_I("warm start disabled");
        }
        /* Fall back to cool start */
        vtss_state->restart_cur = VTSS_RESTART_COOL;
        vtss_state->restart_prev = VTSS_RESTART_COOL;
    }
#endif /* VTSS_FEATURE_WARM_START */
    VTSS_I("cold/cool starting");

    DAYTONA_WRM (VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS,
            VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_SOFT_CHIP_RST(1),
            VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_SOFT_CHIP_RST);
    DAYTONA_POLL_FIELD(VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS, 1, 1, 0);

    /* Setup PI */
    if ((conf->pi.cs_wait_ns > 0) && (conf->pi.cs_wait_ns <= 200)) {
        value = (conf->pi.cs_wait_ns/13);
        DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2,
            VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_WAIT(value),
            VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_WAIT); /* PI_WAIT Configuration */
    }

    if ((conf->pi.cs_hold_ns > 0) && (conf->pi.cs_hold_ns <= 100)) {
        value = (conf->pi.cs_hold_ns/13);
        DAYTONA_WRM(VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2,
            VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_HOLD(value),
            VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_HOLD); /* PI_HOLD Configuration */
    }

    for (channel = 0; channel < 2; channel++) {
      if (conf->conf_mode[channel] != VTSS_CONFIG_MODE_DISABLED) {
        VTSS_RC(daytona_interface_conf_mode_set(vtss_state, channel, conf->conf_mode[channel], conf->two_lane_upi, conf->xfi_clk[channel], conf->x4_case[channel]));
        VTSS_RC(daytona_conf_mode_set(vtss_state, channel, conf->conf_mode[channel], conf->two_lane_upi));
      }
      else {
        /* Reset the channel */
        channel_rst_value = (channel == 0) ? VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0(1) : VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1(1);
        channel_rst_mask = (channel == 0) ? VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0 : VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1;

        DAYTONA_WRM (VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS, channel_rst_value, channel_rst_mask);
      }
    }
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Debug print
 * ================================================================= */

static void daytona_debug_reg_header(const vtss_debug_printf_t pr, const char *name) 
{
    char buf[80];
    
    sprintf(buf, "%-45s  Tgt   Addr  ", name);
    vtss_debug_print_reg_header(pr, buf);
}

static void daytona_debug_reg(vtss_state_t *vtss_state,
                              const vtss_debug_printf_t pr, u32 addr, const char *name)
{
    u32 value;
    char buf[80];

    if (DAYTONA_RD_D(addr, &value) == VTSS_RC_OK) {
        sprintf(buf, "%-45s  0x%02x  0x%04x", name, (addr >> 16) & 0x3f, (addr>>2) & 0x3fff);
        vtss_debug_print_reg(pr, buf, value);
    }
}

static void daytona_debug_reg_inst(vtss_state_t *vtss_state,
                                   const vtss_debug_printf_t pr, u32 addr, u32 i, const char *name)
{
    char buf[80];

    sprintf(buf, "%s_%u", name, i);
    daytona_debug_reg(vtss_state, pr, addr, buf);
}

static BOOL daytona_debug_port_skip(const vtss_port_no_t port_no,
                               const vtss_debug_info_t   *const info)
{
    /* Skip disabled ports */
    return (info->port_list[port_no] ? 0 : 1);
}



static vtss_rc daytona_debug_ts(vtss_state_t *vtss_state,
                                const vtss_debug_printf_t pr,
                                const vtss_debug_info_t   *const info)
{
    char buf[32];
    vtss_port_no_t port_no;
    u32 target;
    daytona_channel_t channel;
    daytona_side_t side;
    /*  */
    for (port_no = 0; port_no < VTSS_PORT_NO_END; port_no++) {
        if (daytona_debug_port_skip(port_no, info)) {
            continue;
        }
        VTSS_RC(daytona_port_2_channel_side(vtss_state, port_no, &channel, &side));
        if (side != LINE_SIDE) {
            continue;
        }
        sprintf(buf, "1588_IP, CHIP PORT %d", port_no);
        daytona_debug_reg_header(pr, buf);
        VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_IP_1588, &target));
        daytona_debug_reg(vtss_state, pr, VTSS_IP_1588_IP_1588_TOP_CFG_STAT_INTERFACE_CTL(target), "INTERFACE_CTL");
        daytona_debug_reg(vtss_state, pr, VTSS_IP_1588_IP_1588_TOP_CFG_STAT_ANALYZER_MODE(target), "ANALYZER_MODE");
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_debug_upi(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    char buf[45];
    vtss_port_no_t port_no;
    u32 target;
    int ch;

    /*  */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (daytona_debug_port_skip(port_no, info)) {
            continue;
        }
        if (VTSS_RC_OK == daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_UPI, &target)) {
            sprintf(buf, "UPI_CONFIG, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_CONFIG(target), "UPI_CONFIG");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG(target), "UPI_TX_CONFIG");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG(target), "UPI_RX_CONFIG");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_STATUS(target), "UPI_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY(target), "UPI_INTR_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK(target), "UPI_INTR_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG(target), "UPI_LOOPBACK_CONFIG");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_LB_RX_TO_TX_FIFO_ERR(target), "UPI_LB_RX_TO_TX_FIFO_ERR");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_ADDR(target), "UPI_INDIR_HSS6G_ADDR");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_WDATA(target), "UPI_INDIR_HSS6G_WDATA");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_RDATA(target), "UPI_INDIR_HSS6G_RDATA");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_CMD(target), "UPI_INDIR_HSS6G_CMD");
            
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS(target), "UPI_TX_HSS6G_PRBS");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS(target), "UPI_RX_HSS6G_PRBS");
            
            sprintf(buf, "UPI_HSS6G_PLL_RST_CONFIG_STAT, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL(target), "UPI_HSS6G_PLL");
            daytona_debug_reg(vtss_state, pr, VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOCK(target), "UPI_HSS6G_PLL_LOCK");
            for (ch = 0; ch < 4; ch++) {
                daytona_debug_reg_inst(vtss_state, pr, VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_CONFIGURATION_MODE(target, ch), ch, "HSS6G_TRANSMIT_CONFIGURATION_MODE");
                daytona_debug_reg_inst(vtss_state, pr, VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL(target, ch), ch, "HSS6G_TRANSMIT_TEST_CONTROL");
                daytona_debug_reg_inst(vtss_state, pr, VTSS_UPI_UPI_HSS6G_CONFIG_STAT_TX_PRBSERR_CNTR(target, ch), ch, "CONFIG_STAT_TX_PRBSERR_CNTR");
                daytona_debug_reg_inst(vtss_state, pr, VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE(target, ch), ch, "HSS6G_RECEIVER_CONFIGURATION_MODE");
                daytona_debug_reg_inst(vtss_state, pr, VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL(target, ch), ch, "HSS6G_RECEIVER_TEST_CONTROL");
                daytona_debug_reg_inst(vtss_state, pr, VTSS_UPI_UPI_HSS6G_CONFIG_STAT_RX_PRBSERR_CNTR(target, ch), ch, "CONFIG_STAT_RX_PRBSERR_CNTR");
            }
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_debug_gfp(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t *const info)
{
    char           buf[40];
    vtss_port_no_t port_no;
    u32            target;
    memset(buf, 0, sizeof(buf));

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (daytona_debug_port_skip(port_no, info)) {
            continue;
        }
        if (VTSS_RC_OK == daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target)) {
            sprintf(buf, "GFP_CONFIGURATION, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFP_BASE_GFP_CONTROL(target), "GFP_BASE_GFP_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI(target), "GFP_BASE_GFP_CLIENT_DATA_UPI");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL(target), "GFP_GFPM_TX_GLOBAL_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI(target), "GFP_GFPM_TX_PPOS_UPI");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL(target), "GFP_GFPM_TX_GFP_FRAME_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), "GFP_GFPM_TX_GFP_CLIENT_MGMT_FRAME_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL(target), "GFP_GFPM_RX_GLOBAL_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL(target), "GFP_GFPM_RX_PPOS_GLOBAL_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI(target), "GFP_GFPM_RX_PPOS_UPI");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL(target), "GFP_GFPM_RX_GFP_CONTROL");
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "GFP_STATUS, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFP_BASE_GFP_STATUS(target), "GFP_BASE_GFP_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY(target), "GFP_BASE_GFP_UPI_INTERRUPT_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_MASK(target), "GFP_BASE_GFP_UPI_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS(target), "GFP_GFPM_TX_8B10B_INTERRUPT_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY(target), "GFP_GFPM_TX_8B10B_INTERRUPT_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK(target), "GFP_GFPM_TX_8B10B_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STATUS(target), "GFP_GFPM_RX_GFP_INTERRUPT_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY(target), "GFP_GFPM_RX_GFP_INTERRUPT_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK(target), "GFP_GFPM_RX_GFP_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY(target), "GFP_GFPM_RX_GFP_CLIENT_INTERRUPT_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK(target), "GFP_GFPM_RX_GFP_CLIENT_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE(target), "GFP_GFPM_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE");
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_debug_otn(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t *const info)
{
    char           buf[40];
    vtss_port_no_t port_no;
    u32            target;
    memset(buf, 0, sizeof(buf));

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (daytona_debug_port_skip(port_no, info)) {
            continue;
        }
        if (VTSS_RC_OK == daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target)) {
            sprintf(buf, "OTN_CONFIGURATION, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG(target), "OTN_CONTROL_REG");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(target), "OTN_TX_OTN_GLOBAL_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL(target), "OTN_TX_OTN_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), "OTN_TX_OTN_FA_OTU_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), "OTN_TX_OTN_OTU_SM_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), "OTN_TX_OTN_ODU_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL(target), "OTN_TX_OTN_ODU_PM_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target), "OTN_TX_OTN_ODU_PM_AUTO_BDI_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), "OTN_TX_OTN_OPU_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE(target), "OTN_TX_OTN_OPU_PAYLOAD_TYPE");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(target), "OTN_RX_OTN_GLOBAL_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL(target), "OTN_RX_OTN_OTU_SM_TTI_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL(target), "OTN_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target), "OTN_RX_OTN_ODU_MAINTENANCE_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL(target), "OTN_RX_OTN_ODU_PM_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL(target), "OTN_RX_OTN_OPU_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT(target), "OTN_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT");
            memset(buf, 0, sizeof(buf));
            sprintf(buf, "OTN_STATUS, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS(target), "OTN_TX_OTN_INTERRUPT_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY(target), "OTN_TX_OTN_INTERRUPT_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK(target), "OTN_TX_OTN_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS(target), "OTN_RX_OTN_FA_FEC_INTERRUPT_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY(target), "OTN_RX_OTN_FA_FEC_INTERRUPT_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK(target), "OTN_RX_OTN_FA_FEC_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY(target), "OTN_RX_OTN_OTU_INTERRUPT_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK(target), "OTN_RX_OTN_OTU_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY(target), "OTN_RX_OTN_ODU_INTERRUPT_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK(target), "OTN_RX_OTN_ODU_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY(target), "OTN_RX_OTN_ODU_PM_INTERRUPT_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK(target), "OTN_RX_OTN_ODU_PM_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY(target), "OTN_RX_OTN_OPU_INTERRUPT_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK(target), "OTN_RX_OTN_OPU_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BEI(target), "OTN_RX_OTN_OTU_SM_BEI");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIP(target), "OTN_RX_OTN_OTU_SM_BIP");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BEI(target), "OTN_RX_OTN_ODU_PM_BEI");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BIP(target), "OTN_RX_OTN_ODU_PM_BIP");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT(target), "OTN_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PJ(target), "OTN_RX_OTN_OPU_PJ");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_NJ(target), "OTN_RX_OTN_OPU_NJ");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_UPPER_ERROR(target), "OTN_RX_OTN_OPU_PRBS_UPPER_ERROR");
            daytona_debug_reg(vtss_state, pr, VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_LOWER_ERROR(target), "OTN_RX_OTN_OPU_PRBS_LOWER_ERROR");
        }
    }
    return VTSS_RC_OK;
}
static vtss_rc daytona_debug_xaui(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info)
{
    char buf[32];
    vtss_port_no_t port_no;
    u32 target;

    /*  */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (daytona_debug_port_skip(port_no, info)) {
            continue;
        }
        if (VTSS_RC_OK == daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_XAUI, &target)) {
            sprintf(buf, "PCS_XAUI_CONFIGURATION, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG(target), "PCS_XAUI_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG(target), "PCS_XAUI_EXT_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG(target), "PCS_XAUI_SD_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG(target), "PCS_XAUI_TX_SEQ_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG(target), "PCS_XAUI_RX_ERR_CNT_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG(target), "PCS_XAUI_INTERLEAVE_MODE_CFG");
            
            sprintf(buf, "PCS_XAUI_STATUS, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS(target), "PCS_XAUI_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT(target), "PCS_XAUI_INT");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK(target), "PCS_XAUI_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS(target), "PCS_XAUI_RX_SEQ_REC_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_FSIG_REC_STATUS(target), "PCS_XAUI_RX_FSIG_REC_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_STAT(target), "RX_OSET_FIFO_STAT");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_DATA(target), "RX_OSET_FIFO_DATA");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_STAT(target), "RX_FSET_FIFO_STAT");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_DATA(target), "RX_FSET_FIFO_DATA");

            sprintf(buf, "PCS_XAUI_ERR_COUNTERS, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_ALIGN_ERR_CNT(target), "PCS_XAUI_RX_ALIGN_ERR_CNT");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_XGMII_ERR_CNT(target), "PCS_XAUI_XGMII_ERR_CNT");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS(target), "PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS(target), "PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS(target), "PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS(target), "PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS");

            sprintf(buf, "PCS_XAUI_COUNTERS , PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT(target), "PCS_XAUI_RX_OK_BYTES_CNT");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_MSB(target), "PCS_XAUI_RX_OK_BYTES_CNT_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT(target), "PCS_XAUI_TX_OK_BYTES_CNT");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_MSB(target), "PCS_XAUI_TX_OK_BYTES_CNT_MSB");
            
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_debug_xfi(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    char buf[32];
    vtss_port_no_t port_no;
    u32 target;
    int lane;
    
    /*  */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (daytona_debug_port_skip(port_no, info)) {
            continue;
        }
        if (VTSS_RC_OK == daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_XFI, &target)) {
            sprintf(buf, "XFI_CONTROL, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_XFI_MODE(target), "XFI_MODE");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_XFI_STATUS(target), "XFI_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_INT_CTRL(target), "INT_CTRL");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_HSS_STICKY(target), "HSS_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_HSS_MASK(target), "HSS_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_HSS_STATUS(target), "HSS_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(target), "HSS_CONTROLS");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_RXPRBSERR_CTR(target), "RXPRBSERR_CTR");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_TESTPAT_MODE(target), "TESTPAT_MODE");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_TESTPAT_THRESHOLD(target), "TESTPAT_THRESHOLD");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_TESTPAT_ERRORS(target), "TESTPAT_ERRORS");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_TESTPAT_CYCS(target), "TESTPAT_CYCS");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_AE_CONTROLS(target), "AE_CONTROLS");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_AE_STATUS(target), "AE_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_AE_STICKY(target), "AE_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_AE_MASK(target), "AE_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_XFI_XFI_CONTROL_XFI_SPARE(target), "XFI_SPARE");
            for (lane = 0; lane < 2; lane++) {
                sprintf(buf, "HSS11G_RX, PORT %d, LANE %s", port_no, lane == 0 ? "A" : "B");
                daytona_debug_reg_header(pr, buf);
                daytona_debug_reg(vtss_state, pr, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(target,lane), "HSS11G_RECEIVER_CONFIGURATION_MODE");
                daytona_debug_reg(vtss_state, pr, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,lane), "HSS11G_RECEIVER_TEST_CONTROL");
                daytona_debug_reg(vtss_state, pr, VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,lane), "HSS11G_DFE_CONTROL");
                daytona_debug_reg(vtss_state, pr, VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS(target,lane), "HSS11G_RECEIVER_INTERNAL_STATUS");
                
                
                sprintf(buf, "HSS11G_TX, PORT %d, LANE %s", port_no, lane == 0 ? "A" : "B");
                daytona_debug_reg_header(pr, buf);
                daytona_debug_reg(vtss_state, pr, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(target,lane), "HSS11G_TRANSMIT_CONFIGURATION_MODE");
                daytona_debug_reg(vtss_state, pr, VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL(target,lane), "HSS11G_TRANSMIT_TEST_CONTROL");
            }
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_debug_pcs_10gbase_r(vtss_state_t *vtss_state,
                                           const vtss_debug_printf_t pr,
                                           const vtss_debug_info_t   *const info)
{
    char buf[32];
    vtss_port_no_t port_no;
    u32 target;
    
    /*  */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (daytona_debug_port_skip(port_no, info)) {
            continue;
        }
        if (VTSS_RC_OK == daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_PCS_10GBASE_R, &target)) {
            sprintf(buf, "PCS_10GBASE_R_CONTROL, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG(target), "PCS_10GBR_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG(target), "PCS_10GBR_TX_FIFO_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG(target), "PCS_10GBR_RX_FIFO_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_MSB(target), "PCS_10GBR_TX_SEEDA_MSB_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_LSB(target), "PCS_10GBR_TX_SEEDA_LSB_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_MSB(target), "PCS_10GBR_RX_SEEDB_MSB_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_LSB(target), "PCS_10GBR_RX_SEEDB_LSB_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_PRBS31_INIT(target), "PCS_10GBR_RX_PRBS31_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_MSB(target), "PCS_10GBR_TX_DATAPAT_MSB_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_LSB(target), "PCS_10GBR_TX_DATAPAT_LSB_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_MSB(target), "PCS_10GBR_RX_DATAPAT_MSB_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_LSB(target), "PCS_10GBR_RX_DATAPAT_LSB_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG(target), "PCS_10GBR_TEST_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK(target), "PCS_10GBR_INTR_MASK_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125(target), "PCS_10GBR_TIMER_125_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_SPARE_CFG(target), "PCS_10GBR_SPARE_CFG");
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_debug_mac10g(vtss_state_t *vtss_state,
                                    const vtss_debug_printf_t pr,
                                    const vtss_debug_info_t   *const info)
{
    char buf[32];
    vtss_port_no_t port_no;
    u32 target;
    
    /*  */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (daytona_debug_port_skip(port_no, info)) {
            continue;
        }
        if (VTSS_RC_OK == daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_MAC10G, &target)) {
            sprintf(buf, "MAC10G_CONTROL, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL(target), "MAC10G_RST_CTRL");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_DEV_CFG_STATUS_LINK_DELTA(target), "MAC10G_LINK_DELTA");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_DEV_CFG_STATUS_LINK_INT_MASK(target), "MAC10G_LINK_INT");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL(target), "MAC10G_PAUSE_TX_FRAME_CTRL");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_2(target), "MAC10G_PAUSE_TX_FRAME_CTRL_2");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL(target), "MAC10G_PAUSE_RX_FRAME_CTRL");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_STATE(target), "MAC10G_PAUSE_STATE");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_LSB(target), "MAC10G_MAC_ADDRESS_LSB");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_MSB(target), "MAC10G_MAC_ADDRESS_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_DEV_CFG_STATUS_PPM_COMPENSATION_CONTROL(target), "MAC10G_PPM_COMPENSATION_CTRL");
            sprintf(buf, "MAC10G_CONFIF_STATUS ");
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_ENA_CFG(target), "MAC10G_MAC_ENA_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG(target), "MAC10G_MAC_MODE_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_MSB(target), "MAC10G_TX_PREAMBLE_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_LSB(target), "MAC10G_TX_PREAMBLE_LSB");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG(target), "MAC10G_MAX_LEN_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG(target), "MAC10G_MAC_TAGS_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG(target), "MAC10G_ADV_CHK_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG(target), "MAC10G_LFS_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG(target), "MAC10G_LB_CFG");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR(target), "MAC10G_TX_MONITOR");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY(target), "MAC10G_TX_MONITOR_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY(target), "MAC10G_MAC_STICKY");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK(target), "MAC10G_TX_MONITOR_INT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK(target), "MAC10G_TX_MAC_INT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK(target), "MAC10G_TX_MAC_INT_MASK");
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_debug_wis(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    char buf[32];
    vtss_port_no_t port_no;
    u32 target;
#if 0
    u32 i = 0;
#endif /* 0 */
    
    /*  */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (daytona_debug_port_skip(port_no, info)) {
            continue;
        }
        if (VTSS_RC_OK == daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target)) {
            sprintf(buf, "WIS_CONTROL, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_RX_TOP_LEVEL(target), "WIS_RX_TOP_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_LOS_CTRL(target), "WIS_RX_LOS_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_WORD_ALIGN(target), "WIS_RX_WORD_ALIGN_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_FRAME_ALIGN(target), "WIS_RX_FRAME_ALIGN_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_LOF_CTRL(target), "WIS_RX_LOF_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL(target), "WIS_RX_LOF_TIMER_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL(target), "WIS_RX_MISC_SOH_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL(target), "WIS_RX_MISC_LOH_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL(target), "WIS_RX_PNTR_INTRPRTR_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_C2_CTRL(target), "WIS_RX_C2_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_MISC_POH_CTRL(target), "WIS_RX_MISC_POH_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_CTRL_ROSI_CTRL(target), "WIS_RX_CTRL_ROSI_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS(target), "WIS_RX_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_WIS_STATUS_RX_STATUS_S1(target), "WIS_RX_STATUS_S1");
#if 0
            for (i = 0; i < 63; i++) {
                daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_J0_STATUS_RX_BYTE_J0(target, i), "WIS_RX_STATUS_J0");
            }
            for (i = 0; i < 63; i++) {
                daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_J1_STATUS_RX_BYTE_J1(target, i), "WIS_RX_STATUS_J1");
            }
#endif /* 0 */
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT(target), "WIS_RX_INTRPT");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL(target), "WIS_TX_CTRL");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL(target), "WIS_TX_MISC");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target), "WIS_TX_PASSTHRU");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_WIS_CTRL_TOSI_CTRL(target), "WIS_TX_TOSI");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_Z0(target), "WIS_TX_Z0");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_E1(target), "WIS_TX_E1");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_F1(target), "WIS_TX_F1");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D1(target), "WIS_TX_D1");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D2(target), "WIS_TX_D2");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D3(target), "WIS_TX_D3");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_K1(target), "WIS_TX_K1");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_K2(target), "WIS_TX_K2");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D4(target), "WIS_TX_D4");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D5(target), "WIS_TX_D5");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D6(target), "WIS_TX_D6");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D7(target), "WIS_TX_D7");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D8(target), "WIS_TX_D8");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D9(target), "WIS_TX_D9");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D10(target), "WIS_TX_D10");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D11(target), "WIS_TX_D11");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_D12(target), "WIS_TX_D12");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_S1(target), "WIS_TX_S1");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_E2(target), "WIS_TX_E2");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_Z1(target), "WIS_TX_Z1");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_Z2(target), "WIS_TX_Z2");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_C2(target), "WIS_TX_C2");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_F2(target), "WIS_TX_F2");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_H4(target), "WIS_TX_H4");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_Z3(target), "WIS_TX_Z3");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_Z4(target), "WIS_TX_Z4");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_N1(target), "WIS_TX_N1");
            daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_BYTE_TX_N1(target), "WIS_TX_N1");
#if 0
            for (i = 0; i < 63; i++) {
                daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_J0_INS_TX_J0(target, i), "WIS_TX_INS_J0");
            }
            for (i = 0; i < 63; i++) {
                daytona_debug_reg(vtss_state, pr, VTSS_EWIS_TX_J1_INS_TX_J1(target, i), "WIS_TX_INS_J1");
            }
#endif /* 0 */
        }
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_debug_rab(vtss_state_t *vtss_state,
                                 const vtss_debug_printf_t pr,
                                 const vtss_debug_info_t   *const info)
{
    char buf[32];
    vtss_port_no_t port_no;
    u32 target;
    
    /*  */
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if (daytona_debug_port_skip(port_no, info)) {
            continue;
        }
        if (VTSS_RC_OK == daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_RAB, &target)) {
            sprintf(buf, "RAB_CONTROL, PORT %d", port_no);
            daytona_debug_reg_header(pr, buf);
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RAB_BASE_RAB_CONTROL(target), "RAB_RAB_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RAB_BASE_RAB_INTERRUPT(target), "RAB_RAB_INTERRUPT");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK(target), "RAB_RAB_INTERRUPT_MASK");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_FIFO_CONTROL(target), "RAB_RX_FIFO_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_FIFO_HIGH_THRESH(target), "RAB_RX_FIFO_HIGH_THRESH");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_FIFO_LOW_THRESH(target), "RAB_RX_FIFO_LOW_THRESH");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_FIFO_ADAPT_THRESH(target), "RAB_RX_FIFO_ADAPT_THRESH");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_PKT_DROP_CNT(target), "RAB_RX_PKT_DROP_CNT");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_PKT_DROP_CNT_PERF(target), "RAB_RX_PKT_DROP_CNT_PERF");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_FIFO_PKT_PEND_CNT(target), "RAB_RX_PKT_PEND_CNT");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_FIFO_HIGH_STATUS(target), "RAB_RX_FIFO_HIGH_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_FIFO_LOW_STATUS(target), "RAB_RX_FIFO_LOW_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_SYNC_CONTROL(target), "RAB_RX_SYNC_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_FIFO_FREEZE(target), "RAB_RX_FIFO_FREEZE");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_FIFO_RX_FIFO_FLUSH(target), "RAB_RX_FIFO_FLUSH");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_INSERT_RX_INSERT_CONTROL(target), "RAB_RX_INSERT_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_INSERT_RX_INSERT_DATA_LSB(target), "RAB_RX_INSERT_DATA_LSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_INSERT_RX_INSERT_DATA_MSB(target), "RAB_RX_INSERT_DATA_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_INSERT_RX_INSERT_PREAMBLE_LSB(target), "RAB_RX_INSERT_PREAMBLE_LSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_INSERT_RX_INSERT_PREAMBLE_MSB(target), "RAB_RX_INSERT_PREAMBLE_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_INSERT_RX_INSERT_PKT_CNT(target), "RAB_RX_INSERT_PKT_CNT");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_INSERT_RX_INSERT_TIMER(target), "RAB_RX_INSERT_TIMER");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_FILTER_0_ETYPE(target), "RAB_RX_FILTER_0_ETYPE");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_FILTER_0_N2B(target), "RAB_RX_FILTER_0_N2B");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_FILTER_1_ETYPE(target), "RAB_RX_FILTER_1_ETYPE");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_FILTER_1_N2B(target), "RAB_RX_FILTER_1_N2B");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_FILTER_2_ETYPE(target), "RAB_RX_FILTER_2_ETYPE");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_FILTER_2_N2B(target), "RAB_RX_FILTER_2_N2B");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_FILTER_3_ETYPE(target), "RAB_RX_FILTER_3_ETYPE");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_FILTER_3_N2B(target), "RAB_RX_FILTER_3_N2B");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_EXTRACT_STATUS(target), "RAB_RX_EXTRACT_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_EXTRACT_DATA_LSB(target), "RAB_RX_EXTRACT_DATA_LSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_EXTRACT_DATA_MSB(target), "RAB_RX_EXTRACT_DATA_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_LSB(target), "RAB_RX_EXTRACT_PREAMBLE_LSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_MSB(target), "RAB_RX_EXTRACT_PREAMBLE_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_0(target), "RAB_RX_EXTRACT_PKT_CNT_0");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_1(target), "RAB_RX_EXTRACT_PKT_CNT_1");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_2(target), "RAB_RX_EXTRACT_PKT_CNT_2");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_3(target), "RAB_RX_EXTRACT_PKT_CNT_3");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_FIFO_CONTROL(target), "RAB_TX_FIFO_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_FIFO_HIGH_THRESH(target), "RAB_TX_FIFO_HIGH_THRESH");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_FIFO_LOW_THRESH(target), "RAB_TX_FIFO_LOW_THRESH");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_FIFO_ADAPT_THRESH(target), "RAB_TX_FIFO_ADAPT_THRESH");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_PKT_DROP_CNT(target), "RAB_TX_PKT_DROP_CNT");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_PKT_DROP_CNT_PERF(target), "RAB_TX_PKT_DROP_CNT_PREF");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_FIFO_PKT_PEND_CNT(target), "RAB_TX_FIFO_PKT_PEND_CNT");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_FIFO_HIGH_STATUS(target), "RAB_TX_FIFO_HIGH_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_FIFO_LOW_STATUS(target), "RAB_TX_FIFO_LOW_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_SYNC_CONTROL(target), "RAB_TX_SYNC_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_FIFO_FREEZE(target), "RAB_TX_FIFO_FREEZE");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_FIFO_TX_FIFO_FLUSH(target), "RAB_TX_FIFO_FLUSH");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_INSERT_TX_INSERT_CONTROL(target), "RAB_TX_INSERT_CONTROL");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_INSERT_TX_INSERT_DATA_LSB(target), "RAB_TX_INSERT_DATA_LSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_INSERT_TX_INSERT_DATA_MSB(target), "RAB_TX_INSERT_DATA_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_INSERT_TX_INSERT_PREAMBLE_LSB(target), "RAB_TX_INSERT_PREAMBLE_LSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_INSERT_TX_INSERT_PREAMBLE_MSB(target), "RAB_TX_INSERT_PREAMBLE_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_INSERT_TX_INSERT_PKT_CNT(target), "RAB_TX_INSERT_PKT_CNT");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_FILTER_0_ETYPE(target), "RAB_TX_FILTER_0_ETYPE");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_FILTER_0_N2B(target), "RAB_TX_FILTER_0_N2B");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_FILTER_1_ETYPE(target), "RAB_TX_FILTER_1_ETYPE");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_FILTER_1_N2B(target), "RAB_TX_FILTER_1_N2B");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_FILTER_2_ETYPE(target), "RAB_TX_FILTER_2_ETYPE");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_FILTER_2_N2B(target), "RAB_TX_FILTER_2_N2B");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_FILTER_3_ETYPE(target), "RAB_TX_FILTER_3_ETYPE");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_FILTER_3_N2B(target), "RAB_TX_FILTER_3_N2B");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_EXTRACT_STATUS(target), "RAB_TX_EXTRACT_STATUS");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_EXTRACT_DATA_LSB(target), "RAB_TX_EXTRACT_DATA_LSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_EXTRACT_DATA_MSB(target), "RAB_TX_EXTRACT_DATA_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_LSB(target), "RAB_TX_EXTRACT_PREAMBLE_LSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_MSB(target), "RAB_TX_EXTRACT_PREAMBLE_MSB");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_0(target), "RAB_TX_EXTRACT_PKT_CNT_0");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_1(target), "RAB_TX_EXTRACT_PKT_CNT_1");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_2(target), "RAB_TX_EXTRACT_PKT_CNT_2");
            daytona_debug_reg(vtss_state, pr, VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_3(target), "RAB_TX_EXTRACT_PKT_CNT_3");
        }
    }
    return VTSS_RC_OK;
}


static vtss_rc daytona_debug_info_print(vtss_state_t *vtss_state,
                                        const vtss_debug_printf_t pr,
                                        const vtss_debug_info_t   *const info)
{
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_TS,   daytona_debug_ts, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_UPI,  daytona_debug_upi, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_XFI,  daytona_debug_xfi, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_PCS_10GBASE_R,  daytona_debug_pcs_10gbase_r, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_MAC10G,  daytona_debug_mac10g, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_WIS,  daytona_debug_wis, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_RAB,  daytona_debug_rab, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_XAUI, daytona_debug_xaui, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_OTN, daytona_debug_otn, vtss_state, pr, info));
    VTSS_RC(vtss_debug_print_group(VTSS_DEBUG_GROUP_GFP, daytona_debug_gfp, vtss_state, pr, info));

    return VTSS_RC_OK;
}

#define VTSS_DAYTONA_PMTICK_ENABLE  (u16)1
#define VTSS_DAYTONA_PMTICK_DISABLE (u16)0

#define VTSS_DAYTONA_PMTICK_SOURCE_INTERNAL (u16)0
#define VTSS_DAYTONA_PMTICK_SOURCE_EXTERNAL (u16)1

static vtss_rc daytona_pmtick_enable_set(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no,
                                         const vtss_pmtick_control_t *const control)
{
    daytona_channel_t channel_no;
    daytona_side_t    side;
    u32               value = 0;

    if (VTSS_RC_OK == daytona_port_2_channel_side(vtss_state, port_no, &channel_no, &side)) {
        if (control->mode == VTSS_MISC_PMTICK_INTERNAL) {
            if (channel_no == 0) {
                /* enable PMTick and select source as Internal */
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_ENA_0(VTSS_DAYTONA_PMTICK_ENABLE);
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_SRC_0(VTSS_DAYTONA_PMTICK_SOURCE_INTERNAL);
                DAYTONA_WR(VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0, value);
                /* configure the time period */
                DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_0,
                            VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_0_PMTICK_PERIOD_0(control->period),
                            VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_0_PMTICK_PERIOD_0);
            } else if (channel_no == 1) {
                /* enable PMTick and select source as Internal */
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_ENA_1(VTSS_DAYTONA_PMTICK_ENABLE);
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_SRC_1(VTSS_DAYTONA_PMTICK_SOURCE_INTERNAL);
                DAYTONA_WR(VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1, value);
                /* configure the time period */
                DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_1,
                            VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_1_PMTICK_PERIOD_1(control->period),
                            VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_1_PMTICK_PERIOD_1);
            }
        } else if (control->mode == VTSS_MISC_PMTICK_EXTERNAL) {
            if (channel_no == 0) {
                /* enable PMTick and select source as External */
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_ENA_0(VTSS_DAYTONA_PMTICK_ENABLE);
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_SRC_0(VTSS_DAYTONA_PMTICK_SOURCE_EXTERNAL);
                DAYTONA_WR(VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0, value);
            } else if (channel_no == 1) {
                /* enable PMTick and select source as External */
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_ENA_1(VTSS_DAYTONA_PMTICK_ENABLE);
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_SRC_1(VTSS_DAYTONA_PMTICK_SOURCE_EXTERNAL);
                DAYTONA_WR(VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1, value);
            }
        } else {
            if (channel_no == 0) {
                /* disable PMTick and select source as Internal */
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_ENA_0(VTSS_DAYTONA_PMTICK_DISABLE);
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_SRC_0(VTSS_DAYTONA_PMTICK_SOURCE_INTERNAL);
                DAYTONA_WR(VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0, value);
            } else if (channel_no == 1) {
                /* disable PMTick and select source as Internal */
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_ENA_1(VTSS_DAYTONA_PMTICK_DISABLE);
                value |= VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_SRC_1(VTSS_DAYTONA_PMTICK_SOURCE_INTERNAL);
                DAYTONA_WR(VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1, value);
            }
        }
    }
    return VTSS_RC_OK;
}

/* ================================================================= *
 *  EFEC events
 * ================================================================= */
static vtss_rc daytona_efec_event_enable(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no, const BOOL enable, const vtss_efec_event_t ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI4, &target));

    /* EFEC I4 */
    mask = 0;
    if (ev_mask & VTSS_I4_ED_UCCW_EV)  mask |= VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_ED_UCCW_MASK;
    if (ev_mask & VTSS_I4_RS_UCCW_EV)  mask |= VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_RS_UCCW_MASK;
    if (ev_mask & VTSS_I4_BCH_UCCW_EV) mask |= VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_BCH_UCCW_MASK;

    DAYTONA_WRM(VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR_EN(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}

static vtss_rc daytona_efec_event_poll(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no, vtss_efec_event_t *const status)
{
    u32 target;
    u32 sticky, mask, new_sticky;

    /* EFECI4 */
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI4, &target));

    DAYTONA_RD(VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR(target), &sticky);
    DAYTONA_RD(VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR_EN(target), &mask);
    new_sticky = sticky & mask;
    DAYTONA_WR(VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR(target), new_sticky);

    if (new_sticky & VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_ED_UCCW_STICKY)         *status |= VTSS_I4_ED_UCCW_EV;
    if (new_sticky & VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_RS_UCCW_STICKY)         *status |= VTSS_I4_RS_UCCW_EV;
    if (new_sticky & VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_BCH_UCCW_STICKY)        *status |= VTSS_I4_BCH_UCCW_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_efec_event_poll_without_mask(vtss_state_t *vtss_state,
                                                    const vtss_port_no_t port_no, vtss_efec_event_t *const status)
{
    u32 target;
    u32 sticky;

    /* EFECI4 */
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI4, &target));

    DAYTONA_RD(VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR(target), &sticky);
    DAYTONA_WR(VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR(target), sticky);

    if (sticky & VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_ED_UCCW_STICKY)         *status |= VTSS_I4_ED_UCCW_EV;
    if (sticky & VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_RS_UCCW_STICKY)         *status |= VTSS_I4_RS_UCCW_EV;
    if (sticky & VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_BCH_UCCW_STICKY)        *status |= VTSS_I4_BCH_UCCW_EV;

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  EFEC7 events
 * ================================================================= */
static vtss_rc daytona_efeci7_event_enable(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no, const BOOL enable, const vtss_efeci7_event_t ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI7, &target));

    /* EFEC I7 */
    mask = 0;
    if (ev_mask & VTSS_I7_BER_ALARM_EV)   mask |= VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MASK_BER_ALARM_MASK;
    DAYTONA_WRM(VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}

static vtss_rc daytona_efeci7_event_poll(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no, vtss_efeci7_event_t *const status)
{
    u32 target;
    u32 sticky, mask, new_sticky;

    /* EFECI7 */
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI7, &target));

    DAYTONA_RD(VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY(target), &sticky);
    DAYTONA_RD(VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MASK(target), &mask);
    new_sticky = sticky & mask;
    DAYTONA_WR(VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY(target), new_sticky);

    if (new_sticky & VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_BER_ALARM_STICKY)  *status |= VTSS_I7_BER_ALARM_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_efeci7_event_poll_without_mask(vtss_state_t *vtss_state,
                                                    const vtss_port_no_t port_no, vtss_efeci7_event_t *const status)
{
    u32 target;
    u32 sticky;

    /* EFECI7 */
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI7, &target));

    DAYTONA_RD(VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY(target), &sticky);
    DAYTONA_WR(VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY(target), sticky);
    if (sticky & VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_BER_ALARM_STICKY)  *status |= VTSS_I7_BER_ALARM_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_channel_mode_set(vtss_state_t *vtss_state,
                                        u16 channel, vtss_config_mode_t conf_mode, u32 two_lane_upi)
{
    u32 channel_rst_value, channel_rst_mask;

    if (conf_mode != VTSS_CONFIG_MODE_DISABLED) {
        VTSS_RC(daytona_default_conf_set(vtss_state, channel));
        VTSS_RC(daytona_default_conf_set(vtss_state, channel + 2));

        VTSS_RC(daytona_interface_conf_mode_set(vtss_state, channel, conf_mode, two_lane_upi, 0, 0));
        VTSS_RC(daytona_conf_mode_set(vtss_state, channel, conf_mode, two_lane_upi));

        /*Restart Configuration for both Line and Client sides */
        VTSS_RC(daytona_port_restart_conf_set(vtss_state, channel));
        VTSS_RC(daytona_port_restart_conf_set(vtss_state, channel + 2));
    }
    else {
        /* Reset the channel */
        channel_rst_value = (channel == 0) ? VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0(1) : VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1(1);
        channel_rst_mask = (channel == 0) ? VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0 : VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1;
        DAYTONA_WRM (VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS, channel_rst_value, channel_rst_mask);
    }
    return VTSS_RC_OK;
}

vtss_rc vtss_daytona_blk_oper_mode_get(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no, daytona_channel_t channel, block_name_t blk_name, u8 *blk_mode)
{
    vtss_init_conf_t  *conf = &vtss_state->init_conf;
    mode_name_t       mode;

    mode = mode_convert(conf->conf_mode[channel]);
    *blk_mode = block_mode[mode][blk_name];
    return VTSS_RC_OK;
}

const static_cfg_t ** vtss_daytona_gcb_config_get(daytona_channel_t channel)
{
    if(channel%2 == 0)
      return gcb_0_config_table;
    else
      return gcb_1_config_table;
}

const static_cfg_t * vtss_daytona_rab_config_get(u32 mode)
{
    return rab_config_table[block_mode[mode][BLOCK_RAB]];
}


vtss_rc vtss_daytona_inst_create(vtss_state_t *vtss_state)
{
    vtss_cil_func_t *func = &vtss_state->cil;

    /* Initialization */
    func->init_conf_set = daytona_init_conf_set;
#if defined(VTSS_FEATURE_WARM_START)
    func->restart_conf_set = daytona_restart_conf_set;
#endif /* VTSS_FEATURE_WARM_START */
    func->debug_info_print = daytona_debug_info_print;

    /* Port control */
    vtss_state->port.map_set = daytona_port_map_set;
    vtss_state->port.miim_read = daytona_miim_read;
    vtss_state->port.miim_write = daytona_miim_write;
    vtss_state->port.mmd_read = daytona_mmd_read;
    vtss_state->port.mmd_write = daytona_mmd_write;

    /* Miscellaneous */
    vtss_state->misc.reg_read = daytona_reg_read;
    vtss_state->misc.reg_write = daytona_reg_write;
    vtss_state->misc.chip_id_get = daytona_chip_id_get;
#if defined(VTSS_GPIOS)
    vtss_state->misc.gpio_mode = daytona_gpio_mode;
    vtss_state->misc.gpio_read = daytona_gpio_read;
    vtss_state->misc.gpio_write = daytona_gpio_write;
#endif /* VTSS_GPIOS */

     /* EFEC events */
    func->efec_event_enable              = daytona_efec_event_enable;
    func->efec_event_poll                = daytona_efec_event_poll;
    func->efec_event_poll_without_mask   = daytona_efec_event_poll_without_mask;
    func->efeci7_event_enable            = daytona_efeci7_event_enable;
    func->efeci7_event_poll              = daytona_efeci7_event_poll;
    func->efeci7_event_poll_without_mask = daytona_efeci7_event_poll_without_mask;

    func->interrupt_enable_set = daytona_interrupt_enable_set;
    func->interrupt_enable_get = daytona_interrupt_enable_get;
    func->interrupt_status_get = daytona_interrupt_status_get;
    func->pmtick_enable_set    = daytona_pmtick_enable_set;
    func->channel_mode_set     = daytona_channel_mode_set;
    func->fast_regs_conf_get   = daytona_fast_regs_conf_get;

    /* State data */
#if defined(VTSS_GPIOS)
    vtss_state->misc.gpio_count = VTSS_GPIOS;
#endif /* VTSS_GPIOS */

#if defined(VTSS_FEATURE_WIS)
    VTSS_DAYTONA_RC(vtss_daytona_inst_wis_create(vtss_state));
#endif /* VTSS_FEATURE_WIS */
#if defined(VTSS_FEATURE_OTN)
    VTSS_DAYTONA_RC(vtss_daytona_inst_otn_create(vtss_state));
#endif /* VTSS_FEATURE_WIS */
#if defined(VTSS_FEATURE_UPI)
    VTSS_DAYTONA_RC(vtss_daytona_inst_upi_create(vtss_state));
#endif /* VTSS_FEATURE_UPI */
#if defined(VTSS_FEATURE_XAUI)
    VTSS_DAYTONA_RC(vtss_daytona_inst_xaui_create(vtss_state));
#endif /* VTSS_FEATURE_XAUI */
#if defined(VTSS_FEATURE_XFI)
    VTSS_DAYTONA_RC(vtss_daytona_inst_xfi_create(vtss_state));
#endif /* VTSS_FEATURE_XFI */
#if defined(VTSS_FEATURE_RAB)
    VTSS_DAYTONA_RC(vtss_daytona_inst_rab_create(vtss_state));
#endif /* VTSS_FEATURE_RAB */
#if defined(VTSS_FEATURE_GFP)
    VTSS_DAYTONA_RC(vtss_daytona_inst_gfp_create(vtss_state));
#endif /* VTSS_FEATURE_GFP */
#if defined(VTSS_FEATURE_PCS_10GBASE_R)
    VTSS_DAYTONA_RC(vtss_daytona_inst_pcs_10gbase_r_create(vtss_state));
#endif /* VTSS_FEATURE_PCS_10GBASE_R */
#if defined(VTSS_FEATURE_MAC10G)
    VTSS_DAYTONA_RC(vtss_daytona_inst_mac10g_create(vtss_state));
#endif /* VTSS_FEATURE_MAC10G */
#if defined(VTSS_FEATURE_I2C)
    VTSS_DAYTONA_RC(vtss_daytona_inst_i2c_create(vtss_state));
#endif /* VTSS_FEATURE_I2C */
#if defined(VTSS_FEATURE_OHA)
    VTSS_DAYTONA_RC(vtss_daytona_inst_oha_create(vtss_state));
#endif /* VTSS_FEATURE_I2C */

    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_DAYTONA */

