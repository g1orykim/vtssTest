/*

 Vitesse API software.

 Copyright (c) 2002-2014 Vitesse Semiconductor Corporation "Vitesse". All
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

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_PHY

// Avoid "vtss_api.h not used in module vtss_phy_10g.c"
/*lint --e{766} */

#include "vtss_api.h" 

#if defined(VTSS_CHIP_10G_PHY)
#include "../../ail/vtss_state.h"
#include "../../ail/vtss_common.h"
#include "vtss_phy_10g.h"
#if defined(VTSS_FEATURE_WIS)
#include "vtss_wis_api.h"
#endif /* VTSS_FEATURE_WIS */
#if defined(VTSS_FEATURE_EDC_FW_LOAD)
#include "vtss_phy_10g_edc_fw.h"
#endif /* VTSS_FEATURE_EDC_FW_LOAD */

/* register addresses used to identiify the Venice SKU's */
#define VENICE_REG_EFUSE 0x0290
#define VENICE_REG_FEATURE_STATE 0x02A0


/* ================================================================= *
 *  Private functions - only reachable from this file
 * ================================================================= */
// Function returning VTSS_RC_ERROR if any registers were unexpected needed to be changed during warm start
static vtss_rc vtss_phy_warm_start_10g_failed_get_private(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_phy_10g_port_state_t *ps = &vtss_state->phy_10g_state[port_no];
    VTSS_I("iport_no:%d warm_start_reg_changed:%d\n", port_no, ps->warm_start_reg_changed);
    return ps->warm_start_reg_changed ? VTSS_RC_ERROR : VTSS_RC_OK;
}

static vtss_rc vtss_phy_rd_wr_masked(vtss_state_t         *vtss_state,
                                     BOOL                 read,
                                     const vtss_port_no_t port_no,
                                     const u16            mmd,
                                     const u32            addr,
                                     u16                  *const value,
                                     const u16            mask)
{
    vtss_rc          rc;
    vtss_mmd_read_t  read_func;
    vtss_mmd_write_t write_func;
    u16              val;
    vtss_port_no_t   p;

    /* Setup read/write function pointers */
    read_func = vtss_state->init_conf.mmd_read;
    write_func = vtss_state->init_conf.mmd_write;

    // GLOBAL can only be accessed through Channel 0 on Venice
    if (mmd == MMD_GLOBAL && vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
        p = vtss_state->phy_10g_state[port_no].phy_api_base_no;
    } else {
        p = port_no;
    }

    if (read) {
        /* Read */
        rc = read_func(vtss_state, p, mmd, addr, value);
    } else if (mask != 0xffff) {
        /* Read-modify-write */
        if ((rc = read_func(vtss_state, p, mmd, addr, &val)) == VTSS_RC_OK)            
            rc = write_func(vtss_state, p, mmd, addr, (val & ~mask) | (*value & mask));
    } else {
        /* Write */
        rc = write_func(vtss_state, p, mmd, addr, *value);
    }

    return rc;
}

/* Read PHY register */
static vtss_rc vtss_mmd_rd(vtss_state_t         *vtss_state,
                           const vtss_port_no_t port_no,
                           const u16            mmd,
                           const u32            addr,
                           u16                  *const value)
{
    return vtss_phy_rd_wr_masked(vtss_state, 1, port_no, mmd, addr, value, 0);
}


/* Write PHY register */
static vtss_rc vtss_mmd_wr(vtss_state_t         *vtss_state,
                           const vtss_port_no_t port_no,
                           const u16            mmd,
                           const u32            addr,
                           const u16            value)
{
    u16 val = value;
    return vtss_phy_rd_wr_masked(vtss_state, 0, port_no, mmd, addr, &val, 0xffff);
}

/* Write PHY register, masked */
static vtss_rc vtss_mmd_wr_masked(vtss_state_t         *vtss_state,
                                  const vtss_port_no_t port_no,
                                  const u16            mmd,
                                  const u32            addr,
                                  const u16            value,
                                  const u16            mask)
{
    u16 val = value;
    return vtss_phy_rd_wr_masked(vtss_state, 0, port_no, mmd, addr, &val, mask);
}

static vtss_rc vtss_mmd_warm_wr_masked(vtss_state_t         *vtss_state,
                                       const vtss_port_no_t port_no,
                                       const u16            mmd,
                                       const u32            addr,
                                       const u16            value,
                                       const u16            mask,
                                       const u16            chk_mask,
                                       const char           *function,                                 
                                       const u16            line)
{
    u16  val;
    vtss_phy_10g_port_state_t *ps = &vtss_state->phy_10g_state[port_no];

    if (vtss_state->sync_calling_private){
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, mmd, addr, &val));
        if ((val ^ value) & mask & chk_mask) { /* Change in bit field */
            VTSS_E("Warm start synch. field changed: port:%u MMD:%d Register:0x%X  Mask:0x%X  from value:0x%X  to value:0x%X  by function:%s, line:%d",
                   port_no+1, mmd, addr, mask, val, value, function, line);

            VTSS_I("ps->warm_start_reg_changed is: true, on port: %d\n", port_no+1);
            ps->warm_start_reg_changed = TRUE; // Signaling that a register for this port has changed.

            VTSS_RC(vtss_mmd_wr_masked(vtss_state, port_no, mmd, addr, value, mask));
        }
    }
    else {
        VTSS_RC(vtss_mmd_wr_masked(vtss_state, port_no, mmd, addr, value, mask));
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_mmd_warm_wr(vtss_state_t         *vtss_state,
                                const vtss_port_no_t port_no,
                                const u16            mmd,
                                const u32            addr,
                                const u16            value,
                                const char           *function,                                 
                                const u16            line)
{
    return vtss_mmd_warm_wr_masked(vtss_state, port_no, mmd, addr, value, 0xFFFF, 0xFFFF, function, line);
}

// Macro for doing warm start register writes. Checks if the register has changed. Also inserts the calling line number when doing register writes. Useful for debugging warm start,
#define VTSS_PHY_WARM_WR_MASKED(port_no, mmd, addr, value, mask) vtss_mmd_warm_wr_masked(vtss_state, port_no, mmd, addr, value, mask, 0xFFFF, __FUNCTION__,  __LINE__)

// See VTSS_PHY_WARM_WR_MASKED. Some registers may not contain the last value written (e.g. self clearing bits), and therefor must be written without doing read check.
// This is possible with this macro which contains a "chk_mask" bit mask for selecting which bit to do read check of. Use this function with care and ONLY with registers there the read value doesn't reflect the last written value.
#define VTSS_PHY_WARM_WR_MASKED_CHK_MASK(port_no, mmd, addr, value, mask, chk_mask) vtss_mmd_warm_wr_masked(vtss_state, port_no, mmd, addr, value, mask, chk_mask, __FUNCTION__, __LINE__)

// Macro that inserts the calling line number when doing register writes. Useful for debugging warm start,
#define VTSS_PHY_WARM_WR(port_no, mmd, addr, value) vtss_mmd_warm_wr(vtss_state, port_no, mmd, addr, value, __FUNCTION__, __LINE__)


static vtss_port_no_t pma_port_no(vtss_state_t *vtss_state, vtss_port_no_t port_no) 
{
    /* Only one PMA in VSC8487 at channel 0  */
    if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8487) {
        return vtss_state->phy_10g_state[port_no].phy_api_base_no;
    }
    return port_no;
}

static vtss_port_no_t base_port(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_state->phy_10g_state[port_no].phy_api_base_no;
}


/* Read status from all sublayers */
static vtss_rc vtss_phy_10g_status_get_private(vtss_state_t *vtss_state,
                                               const vtss_port_no_t port_no,
                                               vtss_phy_10g_status_t   *const status)
{
    u16     value;

    /* PMA */
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_PMA, REG_STATUS_1, &value));
    status->pma.link_down = (value & (1<<2) ? 0: 1);
    if (status->pma.link_down) {
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_PMA, REG_STATUS_1, &value));
        status->pma.rx_link = (value & (1<<2) ? 1: 0);
    } else {
        status->pma.rx_link = 1;
    }
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_PMA, REG_STATUS_2, &value));
    status->pma.rx_fault = (value & (1<<10) ? 1 : 0);
    status->pma.tx_fault = (value & (1<<11) ? 1 : 0);

    /* WIS */
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, REG_STATUS_1, &value));
    status->wis.link_down = (value & (1<<2) ? 0: 1);
    if (status->wis.link_down) {
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, REG_STATUS_1, &value));
        status->wis.rx_link = (value & (1<<2) ? 1: 0);
    } else {
        status->wis.rx_link = 1;
    }
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, REG_STATUS_2, &value));
    status->wis.rx_fault = (value & (1<<10) ? 1 : 0);
    status->wis.tx_fault = (value & (1<<11) ? 1 : 0);

    /* PCS */
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_PCS, REG_STATUS_1, &value));
    status->pcs.link_down = (value & (1<<2) ? 0: 1);
    if (status->pcs.link_down) {
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_PCS, REG_STATUS_1, &value));
        status->pcs.rx_link = (value & (1<<2) ? 1: 0);
    } else {
        status->pcs.rx_link = 1;
    }
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_PCS, REG_STATUS_2, &value));
    status->pcs.rx_fault = (value & (1<<10) ? 1 : 0);
    status->pcs.tx_fault = (value & (1<<11) ? 1 : 0);

    /* XS */
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_XS, REG_STATUS_1, &value));
    status->xs.link_down = (value & (1<<2) ? 0: 1);
    if (status->xs.link_down) {
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_XS, REG_STATUS_1, &value));
        status->xs.rx_link = (value & (1<<2) ? 1: 0);
    } else {
        status->xs.rx_link = 1;
    }
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_XS, REG_STATUS_2, &value));
    status->xs.rx_fault = (value & (1<<10) ? 1 : 0);
    status->xs.tx_fault = (value & (1<<11) ? 1 : 0);

    return VTSS_RC_OK;
}

/* Identify the PHY */
static vtss_rc vtss_phy_10g_identify_private(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no)
{
    u16                   model=0, rev=0, device_feature_status = 0, efuse = 0;


    if (vtss_state->phy_10g_state[port_no].channel_id_lock) {
        return VTSS_RC_OK;     /* The Phy is already identified */
    }

    /*  Identify the phy */
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0, &model)); /* CHIP_id location for 8484 / 8487 / 8488 / Venice */

    if (model == 0x8484 || model == 0x8487 || model == 0x8488 || model == 0x8489 || model == 0x8490 || model == 0x8491) {
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 1, &rev));
    } else if ((vtss_mmd_rd(vtss_state, port_no, MMD_PMA, 0xE800, &model) == VTSS_RC_OK) && (model == 0x8486)) { /* CHIP_id location for 8486 */
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_PMA, 0xE801, &rev));
    } else {
        /* Venice Global Reg must be accessed via base port */
        if (vtss_state->phy_10g_api_base_no != VTSS_PORT_NO_NONE) {
            if (vtss_state->phy_10g_state[vtss_state->phy_10g_api_base_no].family == VTSS_PHY_FAMILY_VENICE) {
                VTSS_RC(vtss_mmd_rd(vtss_state, vtss_state->phy_10g_api_base_no, MMD_GLOBAL, 0, &model)); 
                VTSS_RC(vtss_mmd_rd(vtss_state, vtss_state->phy_10g_api_base_no, MMD_GLOBAL, 1, &rev));
            }
        }
    }

    vtss_state->phy_10g_state[port_no].revision = rev;
    VTSS_D("10G Phy Model:%x  Rev:%x",model,rev);
    if (model == 0x8489 || model == 0x8490 || model == 0x8491) {
        VTSS_D("Venice port %d, base_no %d",port_no, vtss_state->phy_10g_api_base_no);
        if (vtss_state->phy_10g_api_base_no != VTSS_PORT_NO_NONE) {
            /* Read additional EFUSE information */
            VTSS_RC(vtss_mmd_rd(vtss_state, vtss_state->phy_10g_api_base_no, MMD_GLOBAL, VENICE_REG_EFUSE, &efuse)); 
            VTSS_RC(vtss_mmd_rd(vtss_state, vtss_state->phy_10g_api_base_no, MMD_GLOBAL, VENICE_REG_FEATURE_STATE, &device_feature_status));
        } else {
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, VENICE_REG_EFUSE, &efuse)); 
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, VENICE_REG_FEATURE_STATE, &device_feature_status));
        }
        VTSS_D("Venice port %d, device_feature_status %x, efuse %x",port_no, device_feature_status, efuse);
        device_feature_status &= 0xfc; /* clear bit 0,1 */
        device_feature_status |= (0x02 & efuse); /* bit 1 is read from efuse */
        device_feature_status |= (0x01 & (efuse>>2)); /* bit 0 is read from efuse bit 2 */
    }

    switch (model) {
    case 0x8486:
        vtss_state->phy_10g_state[port_no].type = VTSS_PHY_TYPE_8486;
        vtss_state->phy_10g_state[port_no].family = VTSS_PHY_FAMILY_XAUI_XGMII_XFI; 
        break;
    case 0x8484:
        vtss_state->phy_10g_state[port_no].type = VTSS_PHY_TYPE_8484;
        vtss_state->phy_10g_state[port_no].family = VTSS_PHY_FAMILY_XAUI_XFI;
        break;
    case 0x8487:
        vtss_state->phy_10g_state[port_no].type = VTSS_PHY_TYPE_8487;
        vtss_state->phy_10g_state[port_no].family = VTSS_PHY_FAMILY_XAUI_XFI;
        break;
    case 0x8488:
        vtss_state->phy_10g_state[port_no].type = VTSS_PHY_TYPE_8488;
        vtss_state->phy_10g_state[port_no].family = VTSS_PHY_FAMILY_XAUI_XFI;
        break;
    case 0x8489:
        vtss_state->phy_10g_state[port_no].type = VTSS_PHY_TYPE_8489;
        vtss_state->phy_10g_state[port_no].family = VTSS_PHY_FAMILY_VENICE;
        break;
    case 0x8490:
        vtss_state->phy_10g_state[port_no].type = VTSS_PHY_TYPE_8490;
        vtss_state->phy_10g_state[port_no].family = VTSS_PHY_FAMILY_VENICE;
        break;
    case 0x8491:
        vtss_state->phy_10g_state[port_no].type = VTSS_PHY_TYPE_8491;
        vtss_state->phy_10g_state[port_no].family = VTSS_PHY_FAMILY_VENICE;
        break;
    default:
        vtss_state->phy_10g_state[port_no].type = VTSS_PHY_TYPE_10G_NONE;
        vtss_state->phy_10g_state[port_no].family = VTSS_PHY_FAMILY_10G_NONE;
        VTSS_E("Not a Vitesse phy: model:%u",model);
        return VTSS_RC_ERROR;
    }
    VTSS_D("port %d, device_feature_status %x",port_no, device_feature_status);
    vtss_state->phy_10g_state[port_no].device_feature_status = device_feature_status;
    return VTSS_RC_OK;
}

/* Enable / Disable the power for all sublayers */
static vtss_rc vtss_phy_10g_power_set_private(vtss_state_t *vtss_state,
                                              const vtss_port_no_t port_no)

{
    BOOL state;
    u16 value;

    VTSS_D("Enter");
    if (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
        VTSS_RC(VTSS_FUNC_COLD(cil.phy_10g_power_set, port_no));
        return VTSS_RC_OK;
        
    }
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x3, &value));
    state = (vtss_state->phy_10g_state[port_no].power == VTSS_PHY_10G_POWER_DISABLE) ? 1 : 0;
    if (port_no == pma_port_no(vtss_state, port_no)) { /* VSC8487: Only for channel 0   */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA, 0x0000, state<<11, 1<<11));    
    }
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, 0x0000, state<<11, 1<<11));    
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PCS, 0x0000, state<<11, 1<<11));    
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_XS, 0x0000, state<<11, 1<<11));
    VTSS_D("Exit");
    return VTSS_RC_OK;
}


static vtss_rc vtss_phy_10g_failover_set_private(vtss_state_t *vtss_state,
                                                 const vtss_port_no_t port_no)
{
    u16 value;
    vtss_phy_10g_failover_mode_t *mode = &vtss_state->phy_10g_state[base_port(vtss_state, port_no)].failover;

    VTSS_D("Enter");
    if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
        return VTSS_RC_OK; /* Silently ignore it */
    }

    if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8487) {
        if ((*mode == VTSS_PHY_10G_PMA_1_TO_FROM_XAUI_0_TO_XAUI_1) || 
            (*mode == VTSS_PHY_10G_PMA_1_TO_FROM_XAUI_1_TO_XAUI_0)) {
            VTSS_E("Failover mode not supported on VSC8487");
            return VTSS_RC_ERROR;
        }
    }

    if (vtss_state->phy_10g_state[port_no].power == VTSS_PHY_10G_POWER_DISABLE) {
        VTSS_E("Failover not supported while lower power mode is enabled");
        return VTSS_RC_ERROR;
    }

    switch (*mode) {
    case VTSS_PHY_10G_PMA_TO_FROM_XAUI_NORMAL:
        value = 0x20;
        break;
    case VTSS_PHY_10G_PMA_TO_FROM_XAUI_CROSSED:
        value = 0x11;
        break;
    case VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_0_TO_XAUI_1:
        value = 0;
        break;
    case VTSS_PHY_10G_PMA_0_TO_FROM_XAUI_1_TO_XAUI_0:
        value = 0x1;
        break;
    case VTSS_PHY_10G_PMA_1_TO_FROM_XAUI_0_TO_XAUI_1:
        value = 0x30;
        break;
    case VTSS_PHY_10G_PMA_1_TO_FROM_XAUI_1_TO_XAUI_0:
        value = 0x31;
        break;
    default:
        return VTSS_RC_ERROR;
    }        
    
    VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x3, value));
    
    VTSS_D("Exit");
    return VTSS_RC_OK;
}


/* Reset the Phy */
static vtss_rc vtss_phy_10g_reset_private(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no)
{
    /* Reset */
    VTSS_D("Enter");
    VTSS_RC(vtss_mmd_wr_masked(vtss_state, port_no, MMD_PMA, REG_CONTROL_1, 1<<15, 1<<15));

    if  (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_XAUI_XFI) {
        /* PLL run-away workaround for 8484/8487/8488*/
        VTSS_RC(vtss_mmd_wr_masked(vtss_state, port_no, MMD_PMA, 0x8003, 1, 1));  
        VTSS_RC(vtss_mmd_wr_masked(vtss_state, port_no, MMD_PMA, 0x8003, 0, 1));  
    }
   
    /* Workaround for Vitesse Fatsuite board */
    if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486 && vtss_state->phy_10g_state[port_no].revision == 1) {
        VTSS_RC(vtss_mmd_wr(vtss_state, port_no, MMD_PMA, 0x8000, 0xB55F));
    }        
    VTSS_D("Exit");
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_10g_reset_blocks(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no)
{
    /* Reset */
    VTSS_D("Enter");
    if (vtss_state->sync_calling_private) {
        return VTSS_RC_OK;
    }
    if (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
        return VTSS_RC_OK;
    }

    if (!vtss_state->phy_10g_state[port_no].channel_id_lock || vtss_state->phy_10g_state[port_no].family != VTSS_PHY_FAMILY_XAUI_XFI) {
        /* Full reset */
        VTSS_RC(vtss_phy_10g_reset_private(vtss_state, port_no));
    } else {
        if (vtss_state->phy_10g_state[port_no].mode.oper_mode != VTSS_PHY_WAN_MODE) {   /* WAN Reset is handled by WIS code block */
            /* Reset PCS */
            VTSS_RC(vtss_mmd_wr(vtss_state, port_no, MMD_PMA, 0xAE00, (1<<1|1<<2)));
        }
    }
    VTSS_D("Exit");
    return VTSS_RC_OK;
}


/* Verify the Phy  */
static vtss_rc vtss_inst_phy_10G_no_check_private(const vtss_inst_t    inst,
                                                  vtss_state_t         **vtss_state,
                                                  const vtss_port_no_t port_no)
{
    vtss_rc rc;

    if ((rc = vtss_inst_port_no_check(inst, vtss_state, port_no)) == VTSS_RC_OK) {
        if ((*vtss_state)->phy_10g_state[port_no].type == VTSS_PHY_TYPE_10G_NONE) {
            rc = VTSS_RC_ERROR;
        }
    }
    
    return rc;
}

/* Enable/disable a system or a network loopback  */
static vtss_rc vtss_phy_10g_loopback_set_private(vtss_state_t *vtss_state,
                                                 const vtss_port_no_t port_no)

{
    vtss_phy_10g_loopback_t *loopback = &vtss_state->phy_10g_state[port_no].loopback;
    vtss_phy_10g_mode_t         *mode = &vtss_state->phy_10g_state[port_no].mode;
    vtss_port_no_t          pma_port = pma_port_no(vtss_state, port_no);

    VTSS_D("Enter");
    if (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
        VTSS_RC(VTSS_FUNC_COLD(cil.phy_10g_loopback_set, port_no));
        return VTSS_RC_OK;
        
    }
    if (vtss_state->phy_10g_state[port_no].type != VTSS_PHY_TYPE_8486) {
        /* Some loopbacks are only availble in VSC8486 */
        if (loopback->lb_type == VTSS_LB_SYSTEM_XS_DEEP     || 
            loopback->lb_type == VTSS_LB_SYSTEM_PCS_SHALLOW ||
            loopback->lb_type == VTSS_LB_NETWORK_XS_SHALLOW ||
            loopback->lb_type == VTSS_LB_NETWORK_WIS) {            
            VTSS_D("Loopback type is not supported for chip:%d\n",vtss_state->phy_10g_state[port_no].type);
            return VTSS_RC_ERROR;
        }
    }
  
    /* System */
    switch (loopback->lb_type) {
    case VTSS_LB_SYSTEM_XS_SHALLOW:            
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_XS,   0x800E, loopback->enable<<13, 1<<13)); /* Loopback B */
        break;
    case VTSS_LB_SYSTEM_XS_DEEP:            
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_XS,   0x800F, loopback->enable<<2, 1<<2)); /* Loopback C */
        break;
    case VTSS_LB_SYSTEM_PCS_SHALLOW:            
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PCS,  0x8005, loopback->enable<<2, 1<<2)); /* Loopback E */
        break;
    case VTSS_LB_SYSTEM_PCS_DEEP:            
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PCS,  0x0, loopback->enable<<14, 1<<14)); /* Loopback G */
        break;
    case VTSS_LB_SYSTEM_PMA:            
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA,  0x0, loopback->enable, 1)); /* Loopback J (resides on the WIS block) */
        break;
    case VTSS_LB_NETWORK_XS_SHALLOW:            
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_XS,  0x800F, loopback->enable<<1, 1<<1)); /* Loopback D */
        break;
    case VTSS_LB_NETWORK_XS_DEEP:            
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_XS,  0x0, loopback->enable<<14, 1<<14)); /* Loopback A */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_XS,  0x800E, 0<<13, 1<<13)); /* Loopback A */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_XS,  0xE600, loopback->enable<<1, 1<<1)); /* Get XAUI to sync up in case of no XAUI signal */
        break;
    case VTSS_LB_NETWORK_PCS:            
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PCS, 0x8005, loopback->enable<<3, 1<<3)); /* Loopback F */
        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
            break;
        }
        /* 8484 / 8487 / 8488 */
        if (loopback->enable) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(pma_port, MMD_PMA, 0x8017, 0x1C, 0xFF)); /* Loopback F enable */
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol, 0xFF)); /* Loopback F disable */
        }        
        break;
    case VTSS_LB_NETWORK_WIS:            
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, 0xE600, loopback->enable, 1));  /* Loopback H */
        break;
    case VTSS_LB_NETWORK_PMA: 
        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(pma_port, MMD_PMA, 0x8000, (loopback->enable?0:1)<<8, 1<<8)); /* Loopback K */
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(pma_port, MMD_PMA, 0x8012, (loopback->enable?0:1)<<4, 1<<4)); /* Loopback K */
            if (loopback->enable) {
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(pma_port, MMD_PMA, 0x8017, 0x18, 0xFF)); /* Loopback K enable */
            } else {
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol, 0xFF)); /* Loopback K disable */
            }
        }
        break;
    case VTSS_LB_NONE:            
        break;
    default:
        VTSS_E("Loopback not supported");
        break;
    }    
    VTSS_D("Exit");
    return VTSS_RC_OK;
}

/* Register the channel id for this phy port */
static vtss_rc vtss_phy_10g_set_channel(vtss_state_t *vtss_state, const vtss_port_no_t port_no)

{
    vtss_phy_10g_mode_t *mode; 

    VTSS_D("Enter");
    if (vtss_state->phy_10g_state[port_no].channel_id_lock) {
        return VTSS_RC_OK; /* Channel id is set  */
    }
    mode = &vtss_state->phy_10g_state[port_no].mode;
    if (mode->channel_id == VTSS_CHANNEL_AUTO) {        
        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) { 
            vtss_state->phy_channel_id = 0;  /* only one channel in this phy */
        } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8484) {
            vtss_state->phy_10g_state[port_no].channel_id = vtss_state->phy_channel_id;
            vtss_state->phy_10g_state[port_no].channel_id_lock = 1;
            if (vtss_state->phy_channel_id > 2) {
                vtss_state->phy_channel_id = 0; /* 4 channels phy */
            } else {
                vtss_state->phy_channel_id++;
            }
        } else if ((vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8488) ||
                   (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8487)) {
            vtss_state->phy_10g_state[port_no].channel_id = vtss_state->phy_channel_id;
            vtss_state->phy_10g_state[port_no].channel_id_lock = 1;
            if (vtss_state->phy_channel_id > 0) {
                vtss_state->phy_channel_id = 0; /* 2 channels phy */
            } else {
                vtss_state->phy_channel_id++;
            }
        } else if ((vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE)) {
            vtss_state->phy_10g_state[port_no].channel_id = vtss_state->phy_channel_id;
            vtss_state->phy_10g_state[port_no].channel_id_lock = 1;
            if (vtss_state->phy_channel_id > 0) {
                vtss_state->phy_channel_id = 0; /* 2 channels phy */
            } else {
                vtss_state->phy_channel_id++;
            }
        }            
    } else {
        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8484) {
            switch(mode->channel_id) {
            case VTSS_CHANNEL_0:
                vtss_state->phy_10g_state[port_no].channel_id = 0;
                break;
            case VTSS_CHANNEL_1:
                vtss_state->phy_10g_state[port_no].channel_id = 1;
                break;
            case VTSS_CHANNEL_2:
                vtss_state->phy_10g_state[port_no].channel_id = 2;
                break;
            case VTSS_CHANNEL_3:
                vtss_state->phy_10g_state[port_no].channel_id = 3;
                break;
            default:
                VTSS_E("Channel id not supported for this Phy");
                return VTSS_RC_ERROR;
            }
        } else if ((vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8488) ||
                   (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8487)) { 
            switch(mode->channel_id) {
            case VTSS_CHANNEL_0:
                vtss_state->phy_10g_state[port_no].channel_id = 0;
                break;
            case VTSS_CHANNEL_1:
                vtss_state->phy_10g_state[port_no].channel_id = 1;
                break;
            default:
                VTSS_E("Channel id not supported for this Phy");
                return VTSS_RC_ERROR;
            }
        } else {
            vtss_state->phy_channel_id = 0;  /* only one channel in this phy */
        }
    }

    if (vtss_state->phy_10g_state[port_no].channel_id == 0) {
        vtss_state->phy_10g_api_base_no = port_no;
    } else {
        if (vtss_state->phy_10g_api_base_no == VTSS_PORT_NO_NONE) {
            VTSS_E("Please start with initilizing channel 0, i.e. the Phy with the lowest MDIO number");
            return VTSS_RC_ERROR;
        }
    }

    vtss_state->phy_10g_state[port_no].phy_api_base_no = vtss_state->phy_10g_api_base_no;

    if ((vtss_state->phy_channel_id == 0) && (vtss_state->phy_10g_state[port_no].channel_id > 0)) {
        vtss_state->phy_10g_api_base_no = VTSS_PORT_NO_NONE;
    }
    VTSS_D("Exit");
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_10g_mode_set_8486(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    u16     value;   
    vtss_phy_10g_mode_t *mode; 

    mode = &vtss_state->phy_10g_state[port_no].mode;
    VTSS_D("Enter");
    /* Set the Interface mode */
    if (mode->interface == VTSS_PHY_XGMII_XFI) {
        /* XGMII <-> XFI */       
        VTSS_D("XGMII <-> XFI mode");       
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PCS, 0x8005, 0x140, 0x140));
    } else {
        /* XAUI <-> XFI */
        VTSS_D("XAUI <-> XFI mode");       
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PCS, 0x8005, 0x0, 0x140));
    }
    
    /* Set the  TX XFI data polarity */
    if (mode->xfi_pol_invert) {
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA, 0x8000, 0x0000, 0x0080));
    } else {
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA, 0x8000, 0x0080, 0x0080));
    }


    /* Set pre-emphasis of XFI output */
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA, 0xE601, 0x00AF, 0x00FF));
    /* Input sensitivity gain */
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA, 0xE604, (mode->high_input_gain?0:1)<<15, 1<<15));
    
    /* Normal timing / Line timing */
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA, 0xE605, 0<<4, 1<<4)); 
        
    if (mode->oper_mode == VTSS_PHY_LAN_MODE) {
        /* LAN mode */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA, 0xE605, 0x8, 0xC));               
    } else {
        /* WAN mode */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA, 0xE605, 0xC, 0xC));
        if (mode->wrefclk == VTSS_WREFCLK_155_52) {
            /* WREFCLK = 155.52Mhz */
            VTSS_D("WAN mode with wrefclk @ 155.52Mhz");       
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA, 0xE605, 0xC0, 0xC0));
        } else {
            VTSS_D("WAN mode with wrefclk @ 622.08Mhz");       
            /* WREFCLK = 622.08Mhz */
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_PMA, 0xE605, 0x80, 0xC0));
        }
    }
    /* Read and check the status */
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_PMA, 0xE606, &value));                   
    
    if (((mode->oper_mode == VTSS_PHY_WAN_MODE) != VTSS_BOOL(value & (1<<15)))) {
        VTSS_E("WAN API mode does not match the actual chip mode");       
    }   
    VTSS_D("Exit");
    return VTSS_RC_OK;
}

/*
 * According to VSC8488-AN-r10-VPPD-02957.pdf, when writing to global variable,
 * S/W need to follow:
 * 1. read control register until 0x0000
 * 2. write 16-bit variable to data register
 * 3. write WR (bit 15) = 1, C[5:0] and VA[7:0] to Control Register (0x7FE3)
 * 4. Continuously read control register until WR = 0.
 */
static vtss_rc vtss_phy_write_global_variable(vtss_state_t *vtss_state, const vtss_port_no_t port_no, u8 reg, u16 val)
{
    u16 in_reg;

    // Wait control register to become 0 before do anything
    do {
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7FE3, &in_reg));
    } while (in_reg);

    VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7FE4, val));

    VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7FE3, 0x8000 | reg));

    // Wait Control Register (0x7FE3) to notify register setting is done
    do {
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7FE3, &in_reg));
    } while (in_reg & 0x8000);

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_10g_rxckout_848x(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_phy_10g_mode_t *mode = &vtss_state->phy_10g_state[port_no].mode;
    vtss_phy_10g_rxckout_conf_t  rxckout = vtss_state->phy_10g_state[port_no].rxckout;
    vtss_port_no_t      pma_port = pma_port_no(vtss_state, port_no);
    u16 regvalue = 0;
    switch (rxckout.mode) {
        case VTSS_RECVRD_CLKOUT_DISABLE:
            regvalue = 0;
            break;
        case VTSS_RECVRD_CLKOUT_LINE_SIDE_RX_CLOCK:
            regvalue |= 0x0008;
            if (mode->rcvrd_clk_div == VTSS_RECVRDCLK_CDR_DIV_64) { /* (RXCLKOUT => CDR/64) */
                regvalue |= 0x0001;
            } else if (mode->rcvrd_clk_div == VTSS_RECVRDCLK_CDR_DIV_66) { /* (RXCLKOUT => CDR/66) */
                regvalue |= 0x0000;
            } else {
                VTSS_E("Invalid option of rcvrdclk divisor");       
            }
            break;
        case VTSS_RECVRD_CLKOUT_LINE_SIDE_TX_CLOCK:
            regvalue |= 0x0008;
            if (mode->sref_clk_div == VTSS_SREFCLK_DIV_64) { /* (RXCLKOUT => CMU/64) */
                regvalue |= 0x0002;
            } else if (mode->sref_clk_div == VTSS_SREFCLK_DIV_66){ /* (RXCLKOUT => CMU/66) */
                regvalue |= 0x0003;
            } else {
                VTSS_E("Invalid option of srefclk  divisor");       
            }
            break;
    }
    VTSS_D("(i)port_no %d, (i)pma_port %d, Rx clock Output Control %x", port_no, pma_port, regvalue);       
    VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0xA008, regvalue));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_10g_txckout_848x(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_phy_10g_mode_t *mode = &vtss_state->phy_10g_state[port_no].mode;
    vtss_phy_10g_txckout_conf_t  txckout = vtss_state->phy_10g_state[port_no].txckout;
    vtss_port_no_t      pma_port = pma_port_no(vtss_state, port_no);
    u16 regvalue = 0;
    switch (txckout.mode) {
        case VTSS_RECVRD_CLKOUT_DISABLE:
            regvalue = 0;
            break;
        case VTSS_RECVRD_CLKOUT_LINE_SIDE_RX_CLOCK:
            regvalue |= 0x0008;
            if (mode->rcvrd_clk_div == VTSS_RECVRDCLK_CDR_DIV_64) { /* (RXCLKOUT => CDR/64) */
                regvalue |= 0x0003;
            } else if (mode->rcvrd_clk_div == VTSS_RECVRDCLK_CDR_DIV_66) { /* (RXCLKOUT => CDR/66) */
                regvalue |= 0x0002;
            } else {
                VTSS_E("Invalid option of rcvrdclk divisor");       
            }
            break;
        case VTSS_RECVRD_CLKOUT_LINE_SIDE_TX_CLOCK:
            regvalue |= 0x0008;
            if (mode->sref_clk_div == VTSS_SREFCLK_DIV_64) { /* (RXCLKOUT => CMU/64) */
                regvalue |= 0x0000;
            } else if (mode->sref_clk_div == VTSS_SREFCLK_DIV_66){ /* (RXCLKOUT => CMU/66) */
                regvalue |= 0x0001;
            } else {
                VTSS_E("Invalid option of srefclk  divisor");       
            }
            break;
    }
    VTSS_D("(i)port_no %d, (i)pma_port %d, Tx clock Output Control %x", port_no, pma_port, regvalue);       
    VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0xA009, regvalue));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_10g_mode_set_848x(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_phy_10g_mode_t *mode = &vtss_state->phy_10g_state[port_no].mode;
    vtss_port_no_t      pma_port = pma_port_no(vtss_state, port_no);
#if defined(VTSS_FEATURE_SYNCE_10G)
    u16 value = 0;
#endif /* VTSS_FEATURE_SYNCE_10G */

    VTSS_D("Enter");
    switch (mode->oper_mode) {
    case VTSS_PHY_LAN_MODE: 
        /* Regular LAN mode with a single Ref clock: XREFCLK @ 156.25Mhz */        
        VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_WIS, 0x0007, 0));                      /* Disable WAN MODE */
        VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x0188));              /* Global XREFCLK from CMU clkgen     */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x0002, 0x1f)); /* Global Refclk CMU 14Mhz BW control */
        VTSS_RC(vtss_phy_write_global_variable(vtss_state, port_no, 0x94, 0x0002));  /* Set EDC variable MODE (0x94) = 0x0002 (10G) */
        VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_PMA, 0xAE00, 0x1806));                 /* Reset PCS Tx/Rx and 3G/1G Tx/Rx */
        mode->pma_txratecontrol = 0x0004;
        VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol));       /* PMA TX Rate control. CMU Varclk sel bit 16 */        
        VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8018, 0x0000));       /* PMA TX Rate control. 7:6=00 (TX = 10G) */   
        VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8019, 0x0008));       /* PMA RX Rate control */        
        break;        
    case VTSS_PHY_WAN_MODE : 
        /* WAN mode */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x0002, 0x1f));  /* Global Refclk CMU 7Mhz BW control (default) */
        if (mode->wrefclk == VTSS_WREFCLK_155_52) {
            VTSS_D("WAN mode with wrefclk @ 155.52Mhz");       
            VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x0188));           /* Global TX/RX WREFCLK @ rate 155.52Mhz */
            mode->pma_txratecontrol = 0x0020;
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol));    /* PMA TX Rate control. */   
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8019, 0x0020));    /* PMA RX Rate control. */      
#ifndef VTSS_FEATURE_WIS
            VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_WIS, 0x0007, 1));                   /* WIS Control, Enable WAN MODE */      
#endif /* VTSS_FEATURE_WIS */
        } else if (mode->wrefclk == VTSS_WREFCLK_622_08) {
            VTSS_D("WAN mode with wrefclk @ 622.08Mhz");       
            VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x0088));           /* Global TX/RX WREFCLK @ rate 622.08Mhz */
            mode->pma_txratecontrol = 0x0024;
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol));    /* PMA TX Rate control. */   
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8019, 0x0020));    /* PMA RX Rate control. */      
#ifndef VTSS_FEATURE_WIS
            VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_WIS, 0x0007, 1));                   /* WIS Control, Enable WAN MODE */      
#endif /* VTSS_FEATURE_WIS */
        } else {
            VTSS_E("WAN clock not currently supported");       
        }
        break;
    case VTSS_PHY_1G_MODE:
        VTSS_D("1G pass-through mode");       
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x0002, 0x1f)); /* Global Refclk CMU 7Mhz BW control (default) */
        VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_WIS, 0x0007, 0));                      /* Disable WAN MODE */
        VTSS_RC(vtss_phy_write_global_variable(vtss_state, port_no, 0x94, 0x0008));  /* Set EDC variable MODE (0x94) = 0x0008 (1G) */
        VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_PMA, 0xAE00, 0x1806));                 /* Reset PCS Tx/Rx and 3G/1G Tx/Rx */
        mode->pma_txratecontrol = 0x001A;
        VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol));         /* PMA TX Rate control. 5:4=01 (Div by 64). 3:1=101 (XAUI CDR)) */   
        VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8018, 0xC0));         /* PMA TX Rate control. 7:6=11 (TX = 1G) */   
        VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8019, 0x04));         /* PMA RX Rate control. 4:3=00 (Div by 64)  2:1=10 (Rx = 1G) */                   
        break;
#if defined(VTSS_FEATURE_SYNCE_10G)
    case VTSS_PHY_LAN_SYNCE_MODE:
        VTSS_D("LAN SyncE mode");       
        /* Set the RXCKOUT and TXCKOUT */
        VTSS_RC(vtss_phy_10g_rxckout_848x(vtss_state, port_no));
        VTSS_RC(vtss_phy_10g_txckout_848x(vtss_state, port_no));

        
        if (mode->hl_clk_synth == 0) {
            /* PMA Global Refclk Select */
            VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x01C0)); /* (set up clean up PLL for SREFCLK) */
            /* PMA Global Refclk CMU Control */
            if(mode->sref_clk_div == VTSS_SREFCLK_DIV_64){
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x00C2, 0x1f)); /* (/64 clock SREFCLK to clean up PLL) */
            }else if(mode->sref_clk_div == VTSS_SREFCLK_DIV_66){
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x00D2, 0x1f)); /* (/66 clock SREFCLK to clean up PLL) */
            }else{
                VTSS_E("Invalid option of SREFCLK divisor");       
            }
        } else if (mode->hl_clk_synth == 1) {
            /* PMA Global Refclk Select */
            VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x0100)); /*  (set up clean up PLL for XREFCLK) */
                /* PMA Global Refclk CMU Control */
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x00D2, 0x1f)); /* (/66 XREFCLK by clean up PLL) */
            } 
        /* PMA Tx Rate Control */
        mode->pma_txratecontrol = 0x0020;
        VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol)); /*  (cleaned up /16 SREFCLK to be used by CMU) */
        /* PMA Rx Rate Control */
        VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8019, 0x0008)); /*  (/66 XREFCLK used by CRU) */
        break;
        
    case VTSS_PHY_WAN_SYNCE_MODE:
        VTSS_D("WAN SyncE mode");       
        /* Set the RXCKOUT and TXCKOUT */
        VTSS_RC(vtss_phy_10g_rxckout_848x(vtss_state, port_no));
        VTSS_RC(vtss_phy_10g_txckout_848x(vtss_state, port_no));
        
        if (mode->hl_clk_synth == 0) {
            /* PMA Global Refclk Select */
            if ((mode->sref_clk_div == VTSS_SREFCLK_DIV_64) && (mode->wref_clk_div == VTSS_WREFCLK_DIV_16)) {
                /* (set up clean up PLL for SREFCLK and WREFCLK/4 for CRU) */
                VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x01E5)); 
            } else if((mode->sref_clk_div == VTSS_SREFCLK_DIV_64) && (mode->wref_clk_div == VTSS_WREFCLK_NONE)) {
                /* (set up clean up PLL for SREFCLK and WREFCLK/4 for CRU) */
                VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x01EA)); 
            } else if ((mode->sref_clk_div == VTSS_SREFCLK_DIV_16) && (mode->wref_clk_div == VTSS_WREFCLK_DIV_16)) {
                /* (no clean up PLL required but set up SREFCLK for CMU and WREFCLK/4 for CRU) */
                VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x00E5)); 
            } else if ((mode->sref_clk_div == VTSS_SREFCLK_DIV_16) && (mode->wref_clk_div == VTSS_WREFCLK_NONE)) {
                /* (no clean up PLL required but set up SREFCLK for CMU and WREFCLK for CRU) */
                VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x00EA)); 
            } else {
                VTSS_E("Unsupported SREFCLK divisor and WREFCLK divisor");       
            }
        } else if (mode->hl_clk_synth == 1){
            if (mode->wref_clk_div == VTSS_WREFCLK_DIV_16) {
                /* (no clean up PLL required but set up WREFCLK for CMU and WREFCLK/4 for CRU) */
                VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x0A5)); 
            }
            else if (mode->wref_clk_div == VTSS_WREFCLK_NONE) {
                /* (set up clean up PLL for WREFCLK) */
                VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x01AA)); 
            } else {
                VTSS_E("Unsupported WREFCLK divisor with external hit less clock synthsizer");       
            }
        } 

        /* PMA Global Refclk CMU Control */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x00C2, 0x1f)); /* (/64 clock SREFCLK by clean up PLL) */
        
        /* PMA Tx Rate Control */
        mode->pma_txratecontrol = 0x0020;
        VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol)); /* (cleaned up /16 SREFCLK/WREFCLK to be used by CMU) */
        
        /* PMA Rx Rate Control */
        VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8019, 0x0000)); /* (/64 WREFCLK to be used by CRU) */
        break;
        
    case VTSS_PHY_LAN_MIXED_SYNCE_MODE:
        VTSS_D("LAN SyncE mixed mode");
        /* Set the RXCKOUT and TXCKOUT */
        VTSS_RC(vtss_phy_10g_rxckout_848x(vtss_state, port_no));
        VTSS_RC(vtss_phy_10g_txckout_848x(vtss_state, port_no));
        
        if (mode->wref_clk_div == VTSS_WREFCLK_NONE) {
            /* (set up clean up PLL for WREFCLK in clock path 0 and XREFCLK is used in clock path 1) */
            value = 0;
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7F10, &value)); 
            if (value != 0x0182 ) {
                VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x0182)); 
            }
            value = 0;
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7F11, &value)); 
            if ((value & 0x001F) != (0x00C2 & 0x001F)) { 
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x00C2, 0x1f)); /* (/64 clock WREFCLK by clean up PLL) */
            }
            mode->pma_txratecontrol = 0x0004;
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol)); 
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8019, 0x0028)); 
        } else if (mode->wref_clk_div == VTSS_WREFCLK_DIV_16) {
            value = 0;
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7F10, &value)); 
            if (value != 0x0124){
                VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x0124)); 
            }
            value = 0; 
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7F11, &value)); 
            if ((value & 0x001F) != (0x00D2 & 0x001F)){
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x00D2, 0x1f)); /* (/64 clock WREFCLK by clean up PLL) */
            }
            mode->pma_txratecontrol = 0x0020;
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol)); 
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8019, 0x0008)); 
        } else {
            VTSS_E("Invalid option of WREFCLK divisor");       
            }
        break;
        
    case VTSS_PHY_WAN_MIXED_SYNCE_MODE:
        VTSS_D("WAN SyncE mixed mode");       
        /* Set the RXCKOUT and TXCKOUT */
        VTSS_RC(vtss_phy_10g_rxckout_848x(vtss_state, port_no));
        VTSS_RC(vtss_phy_10g_txckout_848x(vtss_state, port_no));
        
        if (mode->wref_clk_div == VTSS_WREFCLK_NONE) {
            /* 1Ex7F10 => 0x0182 (set up clean up PLL for WREFCLK in clock path 0 and XREFCLK is used in clock path 1) */
            value = 0;
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7F10, &value)); 
            if (value != 0x0182) {
                VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x0182)); 
            }
            value = 0;
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7F11, &value)); 
            if ((value & 0x001F) != (0x00C2 & 0x001F)) { 
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x00C2, 0x1f)); /* (/64 clock WREFCLK by clean up PLL) */
            }
            mode->pma_txratecontrol = 0x0020;
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol)); 
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8019, 0x0000)); 
        } else if (mode->wref_clk_div == VTSS_WREFCLK_DIV_16) {
            value = 0;
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7F10, &value)); 
            if (value != 0x0124){
                VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, 0x7F10, 0x0124)); 
            }
            value = 0; 
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7F11, &value)); 
            if ((value & 0x001F) != (0x00D2 & 0x001F)){
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, 0x7F11, 0x00D2, 0x1f)); /* (/64 clock WREFCLK by clean up PLL) */
            }
            mode->pma_txratecontrol = 0x0024;
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8017, mode->pma_txratecontrol)); 
            VTSS_RC(VTSS_PHY_WARM_WR(pma_port, MMD_PMA, 0x8019, 0x0020)); 
        }else{
            VTSS_E("Invalid option of WREFCLK divisor");       
        }
        break;
#endif /* VTSS_FEATURE_SYNCE_10G */
    default:
        VTSS_E("Invalid PHY mode");       
    }
    VTSS_D("Exit");
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_10g_edc_fw_status_private(vtss_state_t *vtss_state,
                                                  const vtss_port_no_t port_no, 
                                                  vtss_phy_10g_fw_status_t *const status)
{
    u16 value, value2;
    VTSS_D("Enter");
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7FE1, &value));  
    VTSS_MSLEEP(100);
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7FE1, &value2));  
    if (value == value2) {
        status->icpu_activity = 0;
    } else {
        status->icpu_activity = 1;
    }
    /* Verify checksum */
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7FE0, &value));  
    if ((value & 0x3) != 0x3) {
        status->edc_fw_chksum = 0;
    } else {
        status->edc_fw_chksum = 1;
    }

    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7FE2, &value));  
    status->edc_fw_rev = value;
    status->edc_fw_api_load = vtss_state->phy_10g_state[port_no].edc_fw_api_load;
    VTSS_D("Exit");
    return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_EDC_FW_LOAD)
static vtss_rc vtss_phy_10g_fw_load_private(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no)
{
    vtss_phy_10g_fw_status_t status;
    u16 value;
    u32 i;
    
    VTSS_D("Enter");
    /* Check MODE0/MODE1 status */
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x1B, &value));  
    if ((value & 0x3) != 0) {
        VTSS_E("MODE0/MODE1 Pins must be 0 for MDIO fw-load"); 
        return VTSS_RC_ERROR; 
    }

    /* Assert uP reset */
    VTSS_RC(vtss_mmd_wr_masked(vtss_state, port_no, MMD_GLOBAL, 0x2, 0x0080, 0x0080));

    /* Format On-Chip-Memory with the EDC firmware.  */
    /* Note that 'edc_fw_arr' array is auto-generated from a file */
    /* in a Motorola S3 format, delivered by the EDC designers  */
    /* The convertion is conducted by a TCL script  */
    for (i = 0; i<(sizeof(edc_fw_arr)/(sizeof(u16))); i++) {
        VTSS_RC(vtss_mmd_wr(vtss_state, port_no, 0x1F, i, edc_fw_arr[i]));
    }
    /* Relase uP reset */
    VTSS_RC(vtss_mmd_wr_masked(vtss_state, port_no, MMD_GLOBAL, 0x2, 0, 0x0080));  
    vtss_state->phy_10g_state[port_no].edc_fw_api_load = 1;    
    VTSS_RC(vtss_phy_10g_edc_fw_status_private(vtss_state, port_no, &status));      
    if ((status.edc_fw_chksum) && (status.icpu_activity)) {
        return VTSS_RC_OK;
    } 
    VTSS_E("Internal microprocessor is not active after fw-load"); 
    return VTSS_RC_ERROR;
}
#endif

vtss_rc vtss_phy_10g_init_conf_set(vtss_state_t *vtss_state)
{
    vtss_init_conf_t *conf = &vtss_state->init_conf;
    vtss_port_no_t   port_no = conf->restart_info_port;
    u16              reg;
    u32              value;

    VTSS_D("Enter");
    if (conf->restart_info_src == VTSS_RESTART_INFO_SRC_10G_PHY && 
        vtss_phy_10g_identify_private(vtss_state, port_no) == VTSS_RC_OK &&
        vtss_state->phy_10g_state[port_no].family != VTSS_PHY_FAMILY_10G_NONE) {

        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
            /* Get restart information from Vitesse 10G PHY */
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_NVR_DOM, 0x8007, &reg));
            value = reg;
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_NVR_DOM, 0x8008, &reg));
            value += (reg << 8);
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_NVR_DOM, 0x8009, &reg));
            value += (reg << 16);
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_NVR_DOM, 0x800A, &reg));
            value += (reg << 24);            
        } else {
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, 0xE603, &reg));
            value = reg;
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, 0xE604, &reg));
            value += (reg << 16);
        }
        VTSS_RC(vtss_cmn_restart_update(vtss_state, value));
    }

    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        if  (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
            vtss_state->phy_10g_state[port_no].gpio_count = 0;  
        } else {      
            vtss_state->phy_10g_state[port_no].gpio_count = 12;       
        }
    }
    VTSS_D("Exit");    
    return VTSS_RC_OK;
}

vtss_rc vtss_phy_10g_restart_conf_set(vtss_state_t *vtss_state)
{
    vtss_init_conf_t *conf = &vtss_state->init_conf;
    vtss_port_no_t   port_no = conf->restart_info_port;
    u32              value;
    
    VTSS_D("Enter");
    if (conf->restart_info_src == VTSS_RESTART_INFO_SRC_10G_PHY &&
        vtss_state->phy_10g_state[port_no].family != VTSS_PHY_FAMILY_10G_NONE) {
        /* Set restart information in Vitesse 10G PHY */
        value = vtss_cmn_restart_value_get(vtss_state);
        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
            /* Make the 1Ex8007-1Ex800A writeble */
            VTSS_RC(vtss_mmd_wr_masked(vtss_state, port_no, MMD_PMA, 0xED00, 0xF000, 0xF000));  
            /* 32bit restart info is written to 4x8bit registers */
            VTSS_RC(vtss_mmd_wr(vtss_state, port_no, MMD_NVR_DOM, 0x8007, (value >> 0  & 0x000000FF)));  
            VTSS_RC(vtss_mmd_wr(vtss_state, port_no, MMD_NVR_DOM, 0x8008, (value >> 8  & 0x0000FF)));  
            VTSS_RC(vtss_mmd_wr(vtss_state, port_no, MMD_NVR_DOM, 0x8009, (value >> 16 & 0x00FF)));  
            VTSS_RC(vtss_mmd_wr(vtss_state, port_no, MMD_NVR_DOM, 0x800A, (value >> 24 & 0xFF)));
        } else {
            VTSS_RC(vtss_mmd_wr(vtss_state, port_no, MMD_WIS, 0xE603, (value & 0x0000FFFF)));  
            VTSS_RC(vtss_mmd_wr(vtss_state, port_no, MMD_WIS, 0xE604, (value >> 16)));  
        }
    }
    VTSS_D("Exit");
    return VTSS_RC_OK;
}


/* Set the operating mode of the Phy  */
static vtss_rc vtss_phy_10g_mode_set_private(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no)
{
    u16     value;
    vtss_phy_10g_mode_t *mode; 

    mode = &vtss_state->phy_10g_state[port_no].mode;
    VTSS_D("Enter");
    switch (vtss_state->phy_10g_state[port_no].type) {
    case VTSS_PHY_TYPE_8486: 
        /* Setup the operating mode */
        VTSS_RC(vtss_phy_10g_mode_set_8486(vtss_state, port_no));
        vtss_state->phy_10g_state[port_no].gpio_count = 0;  
        break;
    case VTSS_PHY_TYPE_8484:
    case VTSS_PHY_TYPE_8487:
    case VTSS_PHY_TYPE_8488:       
#if defined(VTSS_FEATURE_EDC_FW_LOAD)
        /* Load the EDC firmware via MDIO */
        if ((mode->edc_fw_load == VTSS_EDC_FW_LOAD_MDIO) && 
            (vtss_state->phy_10g_state[port_no].channel_id == 0) && 
            (vtss_state->phy_10g_state[port_no].edc_fw_api_load == 0)) {
            VTSS_RC(vtss_phy_10g_fw_load_private(vtss_state, port_no));
        }
        /* FW version 1.09 and above requires 1xA201.2 to be set to 1  */
        if (mode->edc_fw_load == VTSS_EDC_FW_LOAD_MDIO) {
            VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, 0x7FE2, &value));  
            if (value >= 109) {
                VTSS_RC(vtss_mmd_wr_masked(vtss_state, port_no, MMD_PMA, 0xA201, 0x0004, 0x0004));
            } 
        }        
#endif
        /* Set the  TX XFI data polarity */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(pma_port_no(vtss_state, port_no), MMD_PMA, 0x8012, (mode->xfi_pol_invert?8:0), 0x0008));
        /* Set the  Xaui Lane flipping */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_XS, 0x800F, (mode->xaui_lane_flip?0:0x600), 0x600));         
        /* Setup the operating mode */
        VTSS_RC(vtss_phy_10g_mode_set_848x(vtss_state, port_no));

        vtss_state->phy_10g_state[port_no].gpio_count = 12;       
        /* Reset counters */
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_PCS, 0x21, &value));
        break;
    default:
        return VTSS_RC_ERROR; /* Should not happened */
    }
    VTSS_D("Exit");
    return VTSS_RC_OK;   
}

#if defined(VTSS_FEATURE_SYNCE_10G)

static vtss_rc vtss_phy_10g_rxckout_set_private(vtss_state_t *vtss_state,
        const vtss_port_no_t port_no)
{
    vtss_rc rc = VTSS_RC_OK;
    VTSS_D("Enter");
    if (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
        VTSS_RC(VTSS_FUNC_COLD(cil.phy_10g_rxckout_set, port_no));
    } else {
        /* Set the RXCKOUT */
        VTSS_RC(vtss_phy_10g_rxckout_848x(vtss_state, port_no));
    }
    VTSS_D("Exit");
    return rc;
}

static vtss_rc vtss_phy_10g_txckout_set_private(vtss_state_t *vtss_state,
        const vtss_port_no_t port_no)
{
    vtss_rc rc = VTSS_RC_OK;
    VTSS_D("Enter");
    if (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
        VTSS_RC(VTSS_FUNC_COLD(cil.phy_10g_txckout_set, port_no));
    } else {
        /* Set the TXCKOUT */
        VTSS_RC(vtss_phy_10g_txckout_848x(vtss_state, port_no));
    }
    VTSS_D("Exit");
    return rc;
}

static vtss_rc vtss_phy_10g_srefclk_conf_set_private(vtss_state_t *vtss_state,
        const vtss_port_no_t port_no)
{
    vtss_rc rc = VTSS_RC_OK;
    VTSS_D("Enter");
    if (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
        VTSS_RC(VTSS_FUNC_COLD(cil.phy_10g_srefclk_set, port_no));
    } else {
        rc = VTSS_RC_ERROR;
    }
    VTSS_D("Exit");
    return rc;
}

#endif /* VTSS_FEATURE_SYNCE_10G */

#ifdef VTSS_FEATURE_10GBASE_KR
static vtss_rc vtss_phy_10g_base_kr_conf_set_private(vtss_state_t *vtss_state,
        const vtss_port_no_t port_no)
{
    vtss_rc rc = VTSS_RC_OK;
    VTSS_D("Enter");
    if (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
        VTSS_RC(VTSS_FUNC_COLD(cil.phy_10g_base_kr_conf_set, port_no));
    } else {
        rc = VTSS_RC_ERR_KR_CONF_NOT_SUPPORTED;
    }
    VTSS_D("Exit");
    return rc;
}

#endif /* VTSS_FEATURE_10GBASE_KR */


static vtss_rc vtss_phy_10g_86_event_enable_private(vtss_state_t *vtss_state,
                                                    const vtss_port_no_t port_no)
{
    BOOL enable;

    VTSS_D("Enter");
    enable = (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_10G_LINK_LOS_EV);
    vtss_state->phy_10g_state[port_no].event_86_enable = enable;
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_NVR_DOM, 0x9000, enable?0x1B:0, 0x1B));     /* Enable events */
    if (!enable)    /* Interrupt disable can always be done but enable is only done once a second when no event is pending in PHY */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_NVR_DOM, 0x9002, 0, 0x7));

    VTSS_D("Exit");
    return VTSS_RC_OK;   
}

static vtss_rc vtss_phy_10g_88_event_enable_private(vtss_state_t *vtss_state,
                                                    const vtss_port_no_t port_no)
{
    u16 mask_addr1, mask_addr2, mask_addr3;
    VTSS_D("Enter");

    mask_addr1 = (vtss_state->phy_88_event_B[vtss_state->phy_10g_state[port_no].channel_id]) ? 0xEE02 : 0xEE01;
    mask_addr2 = (vtss_state->phy_88_event_B[vtss_state->phy_10g_state[port_no].channel_id]) ? 0xEE06 : 0xEE05;
    mask_addr3 = (vtss_state->phy_88_event_B[vtss_state->phy_10g_state[port_no].channel_id]) ? 0xEE0A : 0xEE09;
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_10G_LINK_LOS_EV)?0x0040:0, 0x0040));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_10G_RX_LOL_EV)?0x1000:0, 0x1000));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr3, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_10G_PCS_RECEIVE_FAULT_EV)?0x0020:0, 0x0020));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_10G_LOPC_EV)?0x0800:0, 0x0800));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_10G_TX_LOL_EV)?0x2000:0, 0x2000));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_10G_HIGH_BER_EV)?0x0001:0, 0x0001));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_10G_MODULE_STAT_EV)?0x8000:0, 0x8000));
#ifdef VTSS_FEATURE_WIS
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_SEF_EV)? 0x0800 : 0, 0x0800));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_FPLM_EV)? 0x0400 : 0, 0x0400));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_FAIS_EV)? 0x0200 : 0, 0x0200));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_LOF_EV)? 0x0080 : 0, 0x0080));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_RDIL_EV)? 0x0020 : 0, 0x0020));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_AISL_EV)? 0x0010 : 0, 0x0010));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_LCDP_EV)? 0x0008 : 0, 0x0008));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_PLMP_EV)? 0x0004 : 0, 0x0004));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_AISP_EV)? 0x0002 : 0, 0x0002));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr1, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_LOPP_EV)? 0x0001 : 0, 0x0001));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_UNEQP_EV)? 0x0400 : 0, 0x0400));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_FEUNEQP_EV)? 0x0200 : 0, 0x0200));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_FERDIP_EV)? 0x0100 : 0, 0x0100));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_REIL_EV)? 0x0010 : 0, 0x0010));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_REIP_EV)? 0x0008 : 0, 0x0008));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_B1_NZ_EV)? 0x0080 : 0, 0x0080));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_B2_NZ_EV)? 0x0040 : 0, 0x0040));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_B3_NZ_EV)? 0x0020 : 0, 0x0020));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_REIL_NZ_EV)? 0x0004 : 0, 0x0004));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr2, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_REIP_NZ_EV)? 0x0002 : 0, 0x0002));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr3, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_B1_THRESH_EV)? 0x0004 : 0, 0x0004));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr3, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_B2_THRESH_EV)? 0x0002 : 0, 0x0002));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr3, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_B3_THRESH_EV)? 0x0001 : 0, 0x0001));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr3, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_REIL_THRESH_EV)? 0x0008 : 0, 0x0008));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_WIS, mask_addr3, (vtss_state->phy_10g_state[port_no].ev_mask & VTSS_PHY_EWIS_REIP_THRESH_EV)? 0x0010 : 0, 0x0010));
#endif /* VTSS_FEATURE_WIS */
    VTSS_D("Exit");
    return VTSS_RC_OK;   
}

static vtss_rc vtss_phy_10g_86_event_poll_private(vtss_state_t *vtss_state,
                                                  const vtss_port_no_t        port_no,
                                                  vtss_phy_10g_event_t  *const events)
{
    u16  pending, mask, enable;

    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_PMA, 0x0008, &pending));   /* Read register in order to clear alarm status */
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_NVR_DOM, 0x9002, &enable));

    if (enable & 0x4) { /* Only caculate pending if enabled - don't want to return active events when not enabled */
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_NVR_DOM, 0x9003, &pending));
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_NVR_DOM, 0x9000, &mask));
        pending &= mask; /* Only include enabled interrupts */
    }
    else
        pending = 0;

    *events = 0;
    if (pending) {
//        if (pending & LOPC_MASK)
        *events |= VTSS_PHY_10G_LINK_LOS_EV;
    }
    return VTSS_RC_OK;   
}


static vtss_rc vtss_phy_10g_88_event_poll_private(vtss_state_t *vtss_state,
                                                  const vtss_port_no_t        port_no,
                                                  vtss_phy_10g_event_t  *const events)
{
    u16     pending1, pending2, pending3, mask, mask_addr;

    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, 0xEE00, &pending1));   /* Read register in order to clear alarm status */
    mask_addr = (vtss_state->phy_88_event_B[vtss_state->phy_10g_state[port_no].channel_id]) ? 0xEE02 : 0xEE01;
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, mask_addr, &mask));
    pending1 &= mask; /* Only include enabled interrupts */

    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, 0xEE04, &pending2));   /* Read register in order to clear alarm status */
    mask_addr = (vtss_state->phy_88_event_B[vtss_state->phy_10g_state[port_no].channel_id]) ? 0xEE06 : 0xEE05;
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, mask_addr, &mask));
    pending2 &= mask; /* Only include enabled interrupts */

    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, 0xEE08, &pending3));   /* Read register in order to clear alarm status */
    mask_addr = (vtss_state->phy_88_event_B[vtss_state->phy_10g_state[port_no].channel_id]) ? 0xEE0A : 0xEE09;
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_WIS, mask_addr, &mask));
    pending3 &= mask; /* Only include enabled interrupts */

    *events = 0;
    if (pending2 & 0x1000) {
        *events |= VTSS_PHY_10G_RX_LOL_EV;
    }
    if (pending2 & 0x0800) {
        *events |= VTSS_PHY_10G_LOPC_EV;
    }
    if (pending3 & 0x0020) {
        *events |= VTSS_PHY_10G_PCS_RECEIVE_FAULT_EV;
    }
    if (pending2 & 0x2000) {
        *events |= VTSS_PHY_10G_TX_LOL_EV;
    }
    if (pending2 & 0x0001) {
        *events |= VTSS_PHY_10G_HIGH_BER_EV;
    }
    if (pending2 & 0x8000) {
        *events |= VTSS_PHY_10G_MODULE_STAT_EV;
    }

#ifdef VTSS_FEATURE_WIS
    if (pending1 & 0x0800) {
        *events |= VTSS_PHY_EWIS_SEF_EV;
    }
    if (pending1 & 0x0400) {
        *events |= VTSS_PHY_EWIS_FPLM_EV;
    }
    if (pending1 & 0x0200) {
        *events |= VTSS_PHY_EWIS_FAIS_EV;
    }
    if (pending1 & 0x0080) {
        *events |= VTSS_PHY_EWIS_LOF_EV;
    }
    if (pending1 & 0x0040) {
        *events |= VTSS_PHY_10G_LINK_LOS_EV;
    }
    if (pending1 & 0x0020) {
        *events |= VTSS_PHY_EWIS_RDIL_EV;
    }
    if (pending1 & 0x0010) {
        *events |= VTSS_PHY_EWIS_AISL_EV;
    }
    if (pending1 & 0x0008) {
        *events |= VTSS_PHY_EWIS_LCDP_EV;
    }
    if (pending1 & 0x0004) {
        *events |= VTSS_PHY_EWIS_PLMP_EV;
    }
    if (pending1 & 0x0002) {
        *events |= VTSS_PHY_EWIS_AISP_EV;
    }
    if (pending1 & 0x0001) {
        *events |= VTSS_PHY_EWIS_LOPP_EV;
    }
    if (pending2 & 0x0400) {
        *events |= VTSS_PHY_EWIS_UNEQP_EV;
    }
    if (pending2 & 0x0200) {
        *events |= VTSS_PHY_EWIS_FEUNEQP_EV;
    }
    if (pending2 & 0x0100) {
        *events |= VTSS_PHY_EWIS_FERDIP_EV;
    }
    if (pending2 & 0x0010) {
        *events |= VTSS_PHY_EWIS_REIL_EV;
    }
    if (pending2 & 0x0008) {
        *events |= VTSS_PHY_EWIS_REIP_EV;
    }
    if (pending2 & 0x0080) {
        *events |= VTSS_PHY_EWIS_B1_NZ_EV;
    }
    if (pending2 & 0x0040) {
        *events |= VTSS_PHY_EWIS_B2_NZ_EV;
    }
    if (pending2 & 0x0020) {
        *events |= VTSS_PHY_EWIS_B3_NZ_EV;
    }
    if (pending2 & 0x0004) {
        *events |= VTSS_PHY_EWIS_REIL_NZ_EV;
    }
    if (pending2 & 0x0002) {
        *events |= VTSS_PHY_EWIS_REIP_NZ_EV;
    }
    if (pending3 & 0x0004) {
        *events |= VTSS_PHY_EWIS_B1_THRESH_EV;
    }
    if (pending3 & 0x0002) {
        *events |= VTSS_PHY_EWIS_B2_THRESH_EV;
    }
    if (pending3 & 0x0001) {
        *events |= VTSS_PHY_EWIS_B3_THRESH_EV;
    }
    if (pending3 & 0x0008) {
        *events |= VTSS_PHY_EWIS_REIL_THRESH_EV;
    }
    if (pending3 & 0x0010) {
        *events |= VTSS_PHY_EWIS_REIP_THRESH_EV;
    }
#endif /* VTSS_FEATURE_WIS */


    return VTSS_RC_OK;   
}


static vtss_rc vtss_phy_10g_gpio_mode_set_private(vtss_state_t *vtss_state,
                                                  const vtss_port_no_t        port_no,
                                                  const vtss_gpio_10g_no_t    gpio_no)
{
    u16 gpio_addr, ch_id, int_sel;
    u32 port;
    u16 val;

    VTSS_D("Enter");
    if (gpio_no <6) {
        gpio_addr = gpio_no*2 + 0x100;
    } else {
        gpio_addr = (gpio_no-6)*2 + 0x124;
    }
    port = base_port(vtss_state, port_no);
    ch_id = vtss_state->phy_10g_state[vtss_state->phy_10g_state[port].gpio_mode[gpio_no].port].channel_id;

    switch(vtss_state->phy_10g_state[port].gpio_mode[gpio_no].mode) {
    case VTSS_10G_PHY_GPIO_NOT_INITIALIZED:
        return VTSS_RC_OK;
    case VTSS_10G_PHY_GPIO_OUT:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, 0, ~0xC00));
        return VTSS_RC_OK;
    case VTSS_10G_PHY_GPIO_IN:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, 0x8000, ~0xC00));    
        return VTSS_RC_OK;
    case VTSS_10G_PHY_GPIO_WIS_INT:
        vtss_state->phy_88_event_B[ch_id] = ((gpio_no == 0) || (gpio_no == 2) || (gpio_no == 3) || (gpio_no == 4) || (gpio_no == 6) || (gpio_no == 10)) ? TRUE : FALSE;
        int_sel = ch_id ? 0x0022 : 0x0002;
        VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, gpio_addr, &val));
        val &= 0x3ff;
        if ((val == 0x0002 && int_sel == 0x0022) || (val == 0x0022 && int_sel == 0x0002) || (val == 0x0082)) {
            /* if both channels use the same interrupt, the WIS interrupt Selection is 100b (bit 7-5) */
            int_sel = 0x0082;
        }
        VTSS_D("port_no %d, port %d, ch_id %d, gpio_no %d, val 0x%x, int_sel 0x%x, event_B %d", port_no, port, ch_id, gpio_no, val, int_sel, vtss_state->phy_88_event_B[ch_id]);
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, int_sel, ~0xC00));
        return VTSS_RC_OK;
/*
    case VTSS_10G_PHY_GPIO_INT_FALLING:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, 0xC000, ~0xC00));
        return VTSS_RC_OK;
    case VTSS_10G_PHY_GPIO_INT_RAISING:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, 0xA000, ~0xC00));
        return VTSS_RC_OK;
    case VTSS_10G_PHY_GPIO_INT_CHANGED:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, 0xE000, ~0xC00));
        return VTSS_RC_OK;
*/
    case VTSS_10G_PHY_GPIO_1588_LOAD_SAVE:
        if (gpio_no != 1) { /* load/save only supported for GPIO 1 */
            return VTSS_RC_ERROR;
        } else {
            if (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, 0x8104, ~0xC00)); /* GPIO_1 configured as Load/Save */
            } else {
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, 0x8100, ~0xC00)); /* GPIO_1 configured as Load/Save */
            }
        }
        return VTSS_RC_OK;
    case VTSS_10G_PHY_GPIO_1588_1PPS_0:
        if (gpio_no != 0 && gpio_no != 5 && gpio_no != 10) { /* 1588-1PPS channel 0 only supported for GPIO 0, 5, 10 */
            return VTSS_RC_ERROR;
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, 0x0003, ~0xC00)); /* GPIO_x configured as 1PPS_0 */
            VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, gpio_addr+1, 0x0051)); /* GPIO_x configured as 1PPS_0 */
        }
        return VTSS_RC_OK;
    case VTSS_10G_PHY_GPIO_1588_1PPS_1:
        if (gpio_no != 4 && gpio_no != 9 && gpio_no != 11) { /* 1588-1PPS channel 0 only supported for GPIO 4, 9, 11 */
            return VTSS_RC_ERROR;
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, 0x0003, ~0xC00));   /* GPIO_x configured to transmit internal signal */
            VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, gpio_addr+1, 0x0051)); /* GPIO_x configured as 1PPS_1 */
        }
        return VTSS_RC_OK;
    case VTSS_10G_PHY_GPIO_PCS_RX_FAULT:
        /* Is valid for all GPIOs */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(port_no, MMD_GLOBAL, gpio_addr, 0x0003, ~0xC00));   /* GPIO_x configured to transmit internal signal */     
        VTSS_RC(VTSS_PHY_WARM_WR(port_no, MMD_GLOBAL, gpio_addr+1, (ch_id==0) ? 0x006C : 0x006D)); /* GPIO_x configured to transmit PCS_RX_STATUS */
        return VTSS_RC_OK;

    }
    VTSS_D("Exit");
    return VTSS_RC_ERROR;       /* NOTREACHED */
}

static vtss_rc vtss_phy_10g_gpio_read_private(vtss_state_t *vtss_state,
                                              const vtss_port_no_t   port_no,
                                              const vtss_gpio_10g_no_t   gpio_no, 
                                              BOOL                  *const value)
{
    u16 val;
    u32 gpio_addr;

    if (gpio_no <6) {
        gpio_addr = gpio_no*2 + 0x100;
    } else {
        gpio_addr = (gpio_no-6)*2 + 0x124;
    }
    VTSS_RC(vtss_mmd_rd(vtss_state, port_no, MMD_GLOBAL, gpio_addr, &val));    
    *value = VTSS_BOOL(val & 0x0400);
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_10g_gpio_write_private(vtss_state_t *vtss_state,
                                               const vtss_port_no_t   port_no,
                                               const vtss_gpio_10g_no_t   gpio_no, 
                                               const BOOL              value)
{
    u32 gpio_addr;
    if (gpio_no <6) {
        gpio_addr = gpio_no*2 + 0x100;
    } else {
        gpio_addr = (gpio_no-6)*2 + 0x124;
    }
    VTSS_RC(vtss_mmd_wr_masked(vtss_state, port_no, MMD_GLOBAL, gpio_addr, value<<12, 1<<12));    
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_10g_gpio_no_check(vtss_state_t *vtss_state,
                                          const vtss_port_no_t   port_no,
                                          const vtss_gpio_10g_no_t  gpio_no)
{
    if (gpio_no > vtss_state->phy_10g_state[port_no].gpio_count) {        
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}

/* Initial function. Sets the operating mode of the Phy.   */
static vtss_rc vtss_phy_10g_mode_set_init (vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, 
                                           const vtss_phy_10g_mode_t *const mode)
{
    vtss_rc rc = VTSS_RC_OK;

    vtss_state->phy_10g_state[port_no].mode = *mode;                   
    /* Identify and store the Phy id */
    VTSS_RC(vtss_phy_10g_identify_private(vtss_state, port_no));
    /* Reset what is needed */
    VTSS_RC(VTSS_RC_COLD(vtss_phy_10g_reset_blocks(vtss_state, port_no)));
    /* Register the channel id       */
    VTSS_RC(vtss_phy_10g_set_channel(vtss_state, port_no));

#if defined(VTSS_FEATURE_SYNCE_10G)
    if ((vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8484) || 
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8487) ||
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8488) ||
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8489) ||
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8490) ||
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8491)) {
        if ((mode->oper_mode == VTSS_PHY_LAN_SYNCE_MODE) ||
            (mode->oper_mode == VTSS_PHY_WAN_SYNCE_MODE) ||
            (mode->oper_mode == VTSS_PHY_LAN_MIXED_SYNCE_MODE) ||
            (mode->oper_mode == VTSS_PHY_WAN_MIXED_SYNCE_MODE)){
            vtss_state->phy_10g_state[port_no].synce_clkout = 1;
            vtss_state->phy_10g_state[port_no].xfp_clkout = 1;
            vtss_state->phy_10g_state[port_no].rxckout.mode = VTSS_RECVRD_CLKOUT_LINE_SIDE_RX_CLOCK;
            vtss_state->phy_10g_state[port_no].rxckout.squelch_on_pcs_fault = FALSE;
            vtss_state->phy_10g_state[port_no].rxckout.squelch_on_lopc = FALSE;
            vtss_state->phy_10g_state[port_no].txckout.mode = VTSS_RECVRD_CLKOUT_LINE_SIDE_TX_CLOCK;
        } else {
            vtss_state->phy_10g_state[port_no].rxckout.mode = VTSS_RECVRD_CLKOUT_DISABLE;
            vtss_state->phy_10g_state[port_no].rxckout.squelch_on_pcs_fault = FALSE;
            vtss_state->phy_10g_state[port_no].rxckout.squelch_on_lopc = FALSE;
            vtss_state->phy_10g_state[port_no].txckout.mode = VTSS_RECVRD_CLKOUT_DISABLE;
        }
    }
#endif /* VTSS_FEATURE_SYNCE_10G */
    if (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
        VTSS_RC(VTSS_FUNC_COLD(cil.phy_10g_mode_conf_set, port_no));
#ifdef VTSS_FEATURE_WIS
        if ((mode->oper_mode == VTSS_PHY_WAN_MODE) ||
                (mode->oper_mode == VTSS_PHY_WAN_SYNCE_MODE) ||
        (mode->oper_mode == VTSS_PHY_WAN_MIXED_SYNCE_MODE)) {
            vtss_state->ewis_conf[port_no].ewis_mode = VTSS_WIS_OPERMODE_WIS_MODE;
            //VTSS_RC(VTSS_FUNC_COLD(cil.ewis_reset, port_no)); /* venice differs slightlu from 8488 */
            VTSS_RC(VTSS_FUNC_COLD(cil.ewis_static_conf_set, port_no));
        } else {
            vtss_state->ewis_conf[port_no].ewis_mode = VTSS_WIS_OPERMODE_DISABLE;
        }
        VTSS_RC(VTSS_FUNC_COLD(cil.ewis_mode_conf_set, port_no));
#endif  /* VTSS_FEATURE_WIS */
        return VTSS_RC_OK;
    }

#if defined(VTSS_FEATURE_EDC_FW_LOAD)
    if (vtss_state->warm_start_cur) {
        if (vtss_state->phy_10g_state[port_no].mode.edc_fw_load == VTSS_EDC_FW_LOAD_MDIO && 
            vtss_state->phy_10g_state[port_no].channel_id == 0) {
            vtss_state->phy_10g_state[port_no].edc_fw_api_load = 1;    
        }
    }
#endif /* VTSS_FEATURE_EDC_FW_LOAD */
    rc = VTSS_RC_COLD(vtss_phy_10g_mode_set_private(vtss_state, port_no));
#ifdef VTSS_FEATURE_WIS
    if ((vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8484) || 
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8487) ||
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8488)) {
        if ((mode->oper_mode == VTSS_PHY_WAN_MODE) ||
            (mode->oper_mode == VTSS_PHY_WAN_SYNCE_MODE) ||
            (mode->oper_mode == VTSS_PHY_WAN_MIXED_SYNCE_MODE)) {
            vtss_state->ewis_conf[port_no].ewis_mode = VTSS_WIS_OPERMODE_WIS_MODE;
            VTSS_RC(VTSS_FUNC_COLD(cil.ewis_reset, port_no));
            VTSS_RC(VTSS_FUNC_COLD(cil.ewis_static_conf_set, port_no));
        } else {
            vtss_state->ewis_conf[port_no].ewis_mode = VTSS_WIS_OPERMODE_DISABLE;
        }
        VTSS_RC(VTSS_FUNC_COLD(cil.ewis_mode_conf_set, port_no));
    }
#endif  /* VTSS_FEATURE_WIS */
    return rc;
}

/* ================================================================= *
 *  Private functions - End
 * ================================================================= */

/* ================================================================= *
 *  Public functions - 10G Phy API functions
 * ================================================================= */
/* Gets the operating mode of the Phy */
vtss_rc vtss_phy_10g_mode_get (const vtss_inst_t inst, 
                               const vtss_port_no_t port_no, 
                               vtss_phy_10g_mode_t *const mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *mode = vtss_state->phy_10g_state[port_no].mode;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_mode_set (const vtss_inst_t inst, 
                               const vtss_port_no_t port_no, 
                               const vtss_phy_10g_mode_t *const mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_10g_mode_set_init(vtss_state, port_no, mode);
    }
    VTSS_EXIT();
    return rc;
}

#if defined(VTSS_FEATURE_SYNCE_10G)
/* Gets the value of whether recovered clock is enabled or not */
vtss_rc vtss_phy_10g_synce_clkout_get (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       BOOL *const synce_clkout)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *synce_clkout = vtss_state->phy_10g_state[port_no].rxckout.mode != VTSS_RECVRD_CLKOUT_DISABLE;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_synce_clkout_set (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       const BOOL synce_clkout)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_10g_state[port_no].rxckout.mode = synce_clkout ? VTSS_RECVRD_CLKOUT_LINE_SIDE_RX_CLOCK : VTSS_RECVRD_CLKOUT_DISABLE;
        vtss_state->phy_10g_state[port_no].rxckout.squelch_on_lopc = FALSE;
        vtss_state->phy_10g_state[port_no].rxckout.squelch_on_pcs_fault = FALSE;
        rc = VTSS_RC_COLD(vtss_phy_10g_rxckout_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

/* Gets the value of whether XFP clock out is enabled or not */
vtss_rc vtss_phy_10g_xfp_clkout_get (const vtss_inst_t inst,
                                     const vtss_port_no_t port_no,
                                     BOOL *const xfp_clkout)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *xfp_clkout = vtss_state->phy_10g_state[port_no].txckout.mode != VTSS_RECVRD_CLKOUT_DISABLE;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_xfp_clkout_set (const vtss_inst_t inst,
                                     const vtss_port_no_t port_no,
                                     const BOOL xfp_clkout)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_10g_state[port_no].txckout.mode = xfp_clkout ? VTSS_RECVRD_CLKOUT_LINE_SIDE_TX_CLOCK : VTSS_RECVRD_CLKOUT_DISABLE;
        rc = VTSS_RC_COLD(vtss_phy_10g_txckout_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_rxckout_get (const vtss_inst_t inst,
                                  const vtss_port_no_t port_no,
                                  vtss_phy_10g_rxckout_conf_t *const rxckout)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *rxckout = vtss_state->phy_10g_state[port_no].rxckout;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_rxckout_set (const vtss_inst_t inst,
                                  const vtss_port_no_t port_no,
                                  const vtss_phy_10g_rxckout_conf_t *const rxckout)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_10g_state[port_no].rxckout = *rxckout;
        rc = VTSS_RC_COLD(vtss_phy_10g_rxckout_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_10g_txckout_get (const vtss_inst_t inst,
                                  const vtss_port_no_t port_no,
                                  vtss_phy_10g_txckout_conf_t *const txckout)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *txckout = vtss_state->phy_10g_state[port_no].txckout;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_txckout_set (const vtss_inst_t inst,
                                  const vtss_port_no_t port_no,
                                  const vtss_phy_10g_txckout_conf_t *const txckout)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_10g_state[port_no].txckout = *txckout;
        rc = VTSS_RC_COLD(vtss_phy_10g_txckout_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}




vtss_rc vtss_phy_10g_srefclk_conf_get (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       vtss_phy_10g_srefclk_mode_t *const srefclk)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *srefclk = vtss_state->phy_10g_state[port_no].srefclk;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_srefclk_conf_set (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       const vtss_phy_10g_srefclk_mode_t *const srefclk)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_10g_state[port_no].srefclk = *srefclk;
        rc = VTSS_RC_COLD(vtss_phy_10g_srefclk_conf_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}


#endif /* VTSS_FEATURE_SYNCE_10G */

#ifdef VTSS_FEATURE_10GBASE_KR
/*
 * Get the configuration of 10f_base_kr setting.
 * Avaliable for PHY family VENICE
 *
 */
vtss_rc vtss_phy_10g_base_kr_conf_get (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       vtss_phy_10g_base_kr_conf_t *const kr_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *kr_conf = vtss_state->phy_10g_state[port_no].kr_conf;
    }
    VTSS_EXIT();
    return rc;
}

/*
 * Set the configuration of 10f_base_kr setting.
 * Avaliable for PHY family VENICE
 *
 */
vtss_rc vtss_phy_10g_base_kr_conf_set (const vtss_inst_t inst,
                                       const vtss_port_no_t port_no,
                                       const vtss_phy_10g_base_kr_conf_t *const kr_conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_10g_state[port_no].kr_conf = *kr_conf;
        rc = VTSS_RC_COLD(vtss_phy_10g_base_kr_conf_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

#endif /* VTSS_FEATURE_10GBASE_KR */


/* Resets the Phy */
vtss_rc vtss_phy_10g_reset(const vtss_inst_t       	inst,
                           const vtss_port_no_t    	port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_RC_COLD(vtss_phy_10g_reset_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

/* Gets the sublayer status */
vtss_rc vtss_phy_10g_status_get (const vtss_inst_t inst, 
                    		 const vtss_port_no_t port_no, 
                    		 vtss_phy_10g_status_t *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_10g_status_get_private(vtss_state, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_clause_37_status_get(const vtss_inst_t                   inst,
                                          vtss_port_no_t                      port_no,
                                          vtss_phy_10g_clause_37_cmn_status_t *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_FUNC_COLD(cil.phy_10g_clause_37_status_get, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_10g_clause_37_control_get(const vtss_inst_t                 inst,
                                           const vtss_port_no_t              port_no,
                                           vtss_phy_10g_clause_37_control_t  *const control)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *control = vtss_state->phy_10g_state[port_no].clause_37;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_clause_37_control_set(const vtss_inst_t                       inst,
                                           const vtss_port_no_t                    port_no,
                                           const vtss_phy_10g_clause_37_control_t  *const control)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_10g_state[port_no].clause_37 = *control;
        vtss_state->phy_10g_state[port_no].clause_37 = *control;
        rc = VTSS_FUNC_COLD(cil.phy_10g_clause_37_control_set, port_no);
    }
    VTSS_EXIT();
    return rc;
}


/*  Gets the phy counters */
vtss_rc vtss_phy_10g_cnt_get(const vtss_inst_t       inst,
			     const vtss_port_no_t    port_no,
                             vtss_phy_10g_cnt_t      *const cnt)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    u16          value;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_mmd_rd(vtss_state, port_no, MMD_PCS, 0x21, &value);
        cnt->pcs.block_lock_latched = VTSS_BOOL(value & (1<<15));
        cnt->pcs.high_ber_latched = VTSS_BOOL(value & (1<<14));
        cnt->pcs.ber_cnt = (value>>8) & 0x3F;
        cnt->pcs.err_blk_cnt = value & 0xFF;
    }
    VTSS_EXIT();
    return rc;
}

/* Sets a system or a network loopback */
vtss_rc vtss_phy_10g_loopback_set(const vtss_inst_t       	inst,
                                  const vtss_port_no_t   	port_no,
                                  const vtss_phy_10g_loopback_t	*const loopback)

{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    vtss_phy_10g_loopback_t	cur_loopback;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        cur_loopback = vtss_state->phy_10g_state[port_no].loopback;
        if (loopback->enable && vtss_state->phy_10g_state[port_no].loopback.enable) {
            rc = VTSS_RC_ERROR;
        } else if (!loopback->enable && !vtss_state->phy_10g_state[port_no].loopback.enable) {
            rc = VTSS_RC_OK;
        } else {
            if (loopback->enable) {
                vtss_state->phy_10g_state[port_no].loopback = *loopback;
            } else {
                vtss_state->phy_10g_state[port_no].loopback.enable = loopback->enable;
            }
            rc = VTSS_RC_COLD(vtss_phy_10g_loopback_set_private(vtss_state, port_no));
        }
        if (rc != VTSS_RC_OK) {
            vtss_state->phy_10g_state[port_no].loopback = cur_loopback;
        }
    }
    VTSS_EXIT();
    return rc;
}

/* Get the current loopback */
vtss_rc vtss_phy_10g_loopback_get(const vtss_inst_t       	inst,
                                  const vtss_port_no_t   	port_no,
                                  vtss_phy_10g_loopback_t	*const loopback)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *loopback = vtss_state->phy_10g_state[port_no].loopback;
        /* the lb_type holds the value of the last loopback type that has been disabled, therefore it is set to NONE if enable is FALSE */
        if (!loopback->enable) loopback->lb_type = VTSS_LB_NONE;
    }
    VTSS_EXIT();
    return rc;
}

/* Get the power mode */
vtss_rc vtss_phy_10g_power_get(const vtss_inst_t      inst,
                                    const vtss_port_no_t   port_no,
                                    vtss_phy_10g_power_t  *const power)
{    
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *power = vtss_state->phy_10g_state[port_no].power;
    }
    VTSS_EXIT();
    return rc;
}

/* Sets the power mode */
vtss_rc vtss_phy_10g_power_set(const vtss_inst_t          inst,
                               const vtss_port_no_t       port_no,
                               const vtss_phy_10g_power_t *const power)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_10g_state[port_no].power = *power;
        rc = VTSS_RC_COLD(vtss_phy_10g_power_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

/* Sets the failover  mode */
vtss_rc vtss_phy_10g_failover_set(const vtss_inst_t      inst,
                                  const vtss_port_no_t   port_no,
                                  vtss_phy_10g_failover_mode_t  *const mode)

{
    vtss_state_t *vtss_state;
    vtss_rc rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (vtss_state->phy_10g_state[port_no].type != VTSS_PHY_TYPE_8486) {
            vtss_state->phy_10g_state[base_port(vtss_state, port_no)].failover = *mode;
            rc = VTSS_RC_COLD(vtss_phy_10g_failover_set_private(vtss_state, port_no));
        } 
    }
    VTSS_EXIT();
    return rc;
}

/* Get the failover mode */
vtss_rc vtss_phy_10g_failover_get(const vtss_inst_t      inst,
                                  const vtss_port_no_t   port_no,
                                  vtss_phy_10g_failover_mode_t  *const mode)
{    
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *mode = vtss_state->phy_10g_state[base_port(vtss_state, port_no)].failover;
    }
    VTSS_EXIT();
    return rc;
}

/* Returns True if the Phy is valid Vitesse 10G Phy  */
BOOL vtss_phy_10G_is_valid(const vtss_inst_t    inst,
                           const vtss_port_no_t port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no);    
    VTSS_EXIT();
    return (rc == VTSS_RC_OK) ? 1 : 0;
}

/* If the VTSS_FEATURE_10G is defined then the MMD read functions are defined elsewhere  */
#if !defined(VTSS_FEATURE_10G) 
vtss_rc vtss_port_mmd_read(const vtss_inst_t     inst,
                           const vtss_port_no_t  port_no,
                           const u8              mmd,
                           const u16             addr,
                           u16                   *const value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_mmd_rd(vtss_state, port_no, mmd, addr, value);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_port_mmd_write(const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no,
                            const u8              mmd,
                            const u16             addr,
                            const u16             value)
{       
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_mmd_wr(vtss_state, port_no, mmd, addr, value);
    }
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_FEATURE_10G */

/* The the Phy ID - can only be used after setting the operating mode */
vtss_rc vtss_phy_10g_id_get(const vtss_inst_t   inst, 
                            const vtss_port_no_t  port_no, 
                            vtss_phy_10g_id_t *const phy_id)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
            phy_id->part_number = 0x8486;
        } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8484) {
            phy_id->part_number = 0x8484;
        } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8488) {
            phy_id->part_number = 0x8488;
        } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8487) {
            phy_id->part_number = 0x8487;
        } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8489) {
            phy_id->part_number = 0x8489;
        } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8490) {
            phy_id->part_number = 0x8490;
        } else if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8491) {
            phy_id->part_number = 0x8491;
        } else {
            VTSS_E("Phy not reckognized");
            rc = VTSS_RC_ERROR;
        }     

        phy_id->revision        = vtss_state->phy_10g_state[port_no].revision;
        phy_id->channel_id      = vtss_state->phy_10g_state[port_no].channel_id;
        phy_id->phy_api_base_no = vtss_state->phy_10g_state[port_no].phy_api_base_no;
        phy_id->type            = vtss_state->phy_10g_state[port_no].type;
        phy_id->family          = vtss_state->phy_10g_state[port_no].family;
        phy_id->device_feature_status = vtss_state->phy_10g_state[port_no].device_feature_status;
    }
    VTSS_EXIT();
    return rc;
}

/* - Events --------------------------------------------------- */

//#define LOPC_MASK  0x1000   /* Acording to VSC8486 Datasheet this should have been bit 11 and not bit 12 */

vtss_rc vtss_phy_10g_event_poll(const vtss_inst_t     inst,
                                const vtss_port_no_t  port_no,
                                vtss_phy_10g_event_t  *const events)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        switch (vtss_state->phy_10g_state[port_no].type) {
        case VTSS_PHY_TYPE_8486:
            rc = vtss_phy_10g_86_event_poll_private(vtss_state, port_no, events);
            break;
        case VTSS_PHY_TYPE_8484:
        case VTSS_PHY_TYPE_8487:
        case VTSS_PHY_TYPE_8488:
        case VTSS_PHY_TYPE_8489:    /* venice phy's are compatible with 8488 */
        case VTSS_PHY_TYPE_8490:
        case VTSS_PHY_TYPE_8491:
            rc = vtss_phy_10g_88_event_poll_private(vtss_state, port_no, events);
            break;
        default:
            rc = VTSS_RC_ERROR;
            break;
        }
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_10g_event_enable_set(const vtss_inst_t           inst,
                                      const vtss_port_no_t        port_no,
                                      const vtss_phy_10g_event_t  ev_mask,
                                      const BOOL                  enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    if (!ev_mask)
        return(VTSS_RC_OK);

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (vtss_state->phy_10g_state[port_no].family == VTSS_PHY_FAMILY_VENICE) {
            if (vtss_state->phy_10g_state[port_no].mode.oper_mode == VTSS_PHY_1G_MODE) {  
                VTSS_EXIT();
                VTSS_D("Events not currently supported for Venice in 1G mode");
                return VTSS_RC_OK;  
            } 
        }
        if (enable)
            vtss_state->phy_10g_state[port_no].ev_mask |= ev_mask;
        else
            vtss_state->phy_10g_state[port_no].ev_mask &= ~ev_mask;
        
        switch (vtss_state->phy_10g_state[port_no].type) {
        case VTSS_PHY_TYPE_8486:
            rc = VTSS_RC_COLD(vtss_phy_10g_86_event_enable_private(vtss_state, port_no));
            break;
        case VTSS_PHY_TYPE_8484:
        case VTSS_PHY_TYPE_8487:
        case VTSS_PHY_TYPE_8488:
        case VTSS_PHY_TYPE_8489:    /* venice phy's are compatible with 8488 */
        case VTSS_PHY_TYPE_8490:
        case VTSS_PHY_TYPE_8491:

            rc = VTSS_RC_COLD(vtss_phy_10g_88_event_enable_private(vtss_state, port_no));
            break;
        default:
            VTSS_D("Events not currently supported for Phy type %d",
                   vtss_state->phy_10g_state[port_no].type);
            rc = VTSS_RC_ERROR;
            break;
        }            
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_event_enable_get(const vtss_inst_t      inst,
                                      const vtss_port_no_t   port_no,
                                      vtss_phy_10g_event_t   *const ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK)
        *ev_mask = vtss_state->phy_10g_state[port_no].ev_mask;
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_poll_1sec(const vtss_inst_t inst)

{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;
    u32 port, port_count = 0;
    u16 pending, mask, enable;

    VTSS_ENTER();
    if ((rc = vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK)
        port_count = vtss_state->port_count;
    for (port=0; port<port_count; ++port) {      /* Check all 8486 10G ports to see if events are "enabled"  */
        if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port)) == VTSS_RC_OK &&
            vtss_state->phy_10g_state[port].type == VTSS_PHY_TYPE_8486 &&
            vtss_state->phy_10g_state[port].event_86_enable) {
            /* Interrupt can be enable once a second. Problem is that active failure
               generates interrupt not possible to clear */
            rc = vtss_mmd_rd(vtss_state, port, MMD_NVR_DOM, 0x9002, &enable);
            if (!(enable & 0x4)) { /* Interrupt is not actually enabled - wait for no pending and then enable */
                rc = vtss_mmd_rd(vtss_state, pma_port_no(vtss_state, port), MMD_PMA, 0x0008, &pending);   /* Read register in order to clear alarm status */
                rc = vtss_mmd_rd(vtss_state, port, MMD_NVR_DOM, 0x9003, &pending);
                rc = vtss_mmd_rd(vtss_state, port, MMD_NVR_DOM, 0x9000, &mask);
                if (!(pending & mask))
                    rc = vtss_mmd_wr_masked(vtss_state, port, MMD_NVR_DOM, 0x9002, 0x4, 0x7);
            }
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_gpio_mode_set(const vtss_inst_t                inst,
                                   const vtss_port_no_t             port_no,
                                   const vtss_gpio_10g_no_t         gpio_no,
                                   const vtss_gpio_10g_gpio_mode_t  *mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();    
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK &&
        (rc = vtss_phy_10g_gpio_no_check(vtss_state, port_no, gpio_no)) == VTSS_RC_OK) {
        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
            VTSS_E("GPIOs not currently supported for %d",vtss_state->phy_10g_state[port_no].type);
            rc = VTSS_RC_ERROR;
        } else if (vtss_state->phy_10g_state[port_no].type != VTSS_PHY_TYPE_8487 && 
                   vtss_state->phy_10g_state[port_no].type != VTSS_PHY_TYPE_8488 &&
                   vtss_state->phy_10g_state[port_no].type != VTSS_PHY_TYPE_8489 &&
                   vtss_state->phy_10g_state[port_no].type != VTSS_PHY_TYPE_8490 &&
                   vtss_state->phy_10g_state[port_no].type != VTSS_PHY_TYPE_8491 &&
                   (mode->mode == VTSS_10G_PHY_GPIO_1588_LOAD_SAVE || 
                    mode->mode == VTSS_10G_PHY_GPIO_1588_1PPS_0 || 
                    mode->mode == VTSS_10G_PHY_GPIO_1588_1PPS_1)) {
            VTSS_E("1588 feature not supported for %d",vtss_state->phy_10g_state[port_no].type);
            rc = VTSS_RC_ERROR;
        } else {
            vtss_state->phy_10g_state[base_port(vtss_state, port_no)].gpio_mode[gpio_no] = *mode;
            rc = VTSS_RC_COLD(vtss_phy_10g_gpio_mode_set_private(vtss_state, port_no, gpio_no));
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_gpio_mode_get(const vtss_inst_t          inst,
                                   const vtss_port_no_t       port_no,
                                   const vtss_gpio_10g_no_t   gpio_no,
                                   vtss_gpio_10g_gpio_mode_t  *const mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();    
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK &&
        (rc = vtss_phy_10g_gpio_no_check(vtss_state, port_no, gpio_no)) == VTSS_RC_OK) {
        *mode = vtss_state->phy_10g_state[base_port(vtss_state, port_no)].gpio_mode[gpio_no];
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_gpio_read(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               const vtss_gpio_10g_no_t  gpio_no,
                               BOOL                  *const value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK &&
        (rc = vtss_phy_10g_gpio_no_check(vtss_state, port_no, gpio_no)) == VTSS_RC_OK) {
        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
            VTSS_E("GPIOs not currently supported for %d",vtss_state->phy_10g_state[port_no].type);
            rc = VTSS_RC_ERROR;
        } else { 
            rc = vtss_phy_10g_gpio_read_private(vtss_state, port_no, gpio_no, value);
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_10g_gpio_write(const vtss_inst_t     inst,
                                const vtss_port_no_t  port_no,
                                const vtss_gpio_10g_no_t  gpio_no,
                                const BOOL            value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK &&
        (rc = vtss_phy_10g_gpio_no_check(vtss_state, port_no, gpio_no)) == VTSS_RC_OK) {
        if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
            VTSS_E("GPIOs not currently supported for %d",vtss_state->phy_10g_state[port_no].type);
            rc = VTSS_RC_ERROR;
        } else {
            rc = vtss_phy_10g_gpio_write_private(vtss_state, port_no, gpio_no, value);
        }
    }
    VTSS_EXIT();
    return rc;    
}

vtss_rc vtss_phy_10g_edc_fw_status_get(const vtss_inst_t     inst,
                                       const vtss_port_no_t  port_no,
                                       vtss_phy_10g_fw_status_t  *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;
    
    VTSS_ENTER();
    if ((rc = vtss_inst_phy_10G_no_check_private(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if ((vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8484) ||
            (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8487) ||
            (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8488)) {            
            rc = vtss_phy_10g_edc_fw_status_private(vtss_state, port_no, status);
        } else {
            VTSS_E("Internal CPU EDC Firmware status currently not supported for %d",vtss_state->phy_10g_state[port_no].type);
            rc = VTSS_RC_ERROR;
        }
    }
    VTSS_EXIT();
    return rc;
}
/* - Warm start synchronization ------------------------------------ */
#define VTSS_SYNC_RC(expr) { vtss_rc __rc__ = (expr); if (__rc__ < VTSS_RC_OK) {vtss_state->sync_calling_private = FALSE; return __rc__; }}
extern vtss_rc vtss_phy_ts_sync(vtss_state_t *vtss_state, const vtss_port_no_t port_no);
vtss_rc vtss_phy_10g_sync(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    u32 i;

    vtss_phy_10g_port_state_t *ps = &vtss_state->phy_10g_state[port_no];

    if (ps->family == VTSS_PHY_FAMILY_10G_NONE) {
        VTSS_D("port_no %u not connected to 10G PHY", port_no);
        return VTSS_RC_OK;
    }

    vtss_state->sync_calling_private = TRUE;
    vtss_state->phy_10g_state[port_no].warm_start_reg_changed = FALSE;

   /* Here all private functions should be called */
    VTSS_SYNC_RC(vtss_phy_10g_mode_set_private(vtss_state, port_no));
    VTSS_SYNC_RC(vtss_phy_10g_loopback_set_private(vtss_state, port_no));
    VTSS_SYNC_RC(vtss_phy_10g_failover_set_private(vtss_state, port_no));
    VTSS_SYNC_RC(vtss_phy_10g_power_set_private(vtss_state, port_no));
    VTSS_SYNC_RC(vtss_phy_10g_rxckout_set_private(vtss_state, port_no));
    VTSS_SYNC_RC(vtss_phy_10g_txckout_set_private(vtss_state, port_no));
    for (i=0; i<vtss_state->phy_10g_state[port_no].gpio_count; ++i)
        VTSS_SYNC_RC(vtss_phy_10g_gpio_mode_set_private(vtss_state, port_no, i))

    if (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8486) {
        VTSS_SYNC_RC(vtss_phy_10g_86_event_enable_private(vtss_state, port_no));
    }

    if ((vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8484) ||
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8487) ||
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8489) ||
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8490) ||
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8491) ||
        (vtss_state->phy_10g_state[port_no].type == VTSS_PHY_TYPE_8488)) {
        VTSS_SYNC_RC(vtss_phy_10g_88_event_enable_private(vtss_state, port_no));
    }

    vtss_state->sync_calling_private = FALSE;

    return VTSS_RC_OK;
}

// Function returning VTSS_RC_ERROR if any registers were unexpected needed to be changed during warm start, else return VTSS_RC_OK - Note: In a working system VTSS_RC_ERROR should never be seen.
vtss_rc vtss_phy_warm_start_10g_failed_get(const vtss_inst_t    inst,
                                       const vtss_port_no_t port_no)
{
    vtss_rc rc;
    vtss_state_t *vtss_state;
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_warm_start_10g_failed_get_private(vtss_state, port_no);
    }
    VTSS_EXIT();
    return rc;
}

/* - Debug print --------------------------------------------------- */



/* ================================================================= *
 *  Public functions - End
 * ================================================================= */


#endif /* VTSS_CHIP_10G_PHY */
