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

// Avoid "vtss_options.h not used in module vtss_phy.c"
/*lint --e{766} */

#include "vtss_options.h"

#ifdef VTSS_CHIP_CU_PHY

#define VTSS_TRACE_GROUP VTSS_TRACE_GROUP_PHY

#include "vtss_api.h"
#include "../../ail/vtss_state.h"
#include "../../ail/vtss_common.h"
#include "vtss_phy_init_scripts.h"


// Because the PHYs operates with paged registers, we have sometimes expired that the registers are accessed to wrong pages due to code bugs.
// These bugs can be hard to find therefore definitions of registers are including the page they belong to. By setting the do_page_chk variable the page register is checked whenever a register is accessed, and giving an error message if there is an unexpected page access.
// For this to work the register definitions must be used, and therefor all new updates to the phy api should use these register definition and the corresponding rd_page and wr_page functions and macros.
#ifdef VTSS_SW_OPTION_DEBUG
static BOOL do_page_chk = TRUE;
#else
static BOOL do_page_chk = FALSE;
#endif


/* ================================================================= *
 *  Pre-declare functions
 * ================================================================= */
static vtss_rc vtss_phy_conf_set_private(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no);

/* ================================================================= *
 *  Private functions
 * ================================================================= */
// Function that is called any time the micro reset is being asserted
// This function specifically avoids bugzilla #5731
// IN : port_no - Any port within the chip.
vtss_rc vtss_phy_micro_assert_reset(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{

    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];


    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_TESLA:
        // Bugzilla#5731 - The following code-sequence is needed for Luton26 Rev. A and B where
        // a consumptive patch disables interrupts inside a micro-command patch.

        // Set to micro/GPIO-page
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

        //----------------------------------------------------------------------
        // Pass the NOP cmd to Micro to insure that any consumptive patch exits
        // There is no issue with doing this on any revision since it is just a NOP on any Vitesse PHY.
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x800f));

        // Poll on 18G.15 to clear
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
        //----------------------------------------------------------------------

        // Set to micro/GPIO-page (Has been set to std page by vtss_phy_wait_for_micro_complete
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

        // force micro into a loop, preventing any SMI accesses
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x0000, 0x0800)); // Disable patch vector 3 (just in case)
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_9, 0x005b));     // Setup patch vector 3 to trap MicroWake interrupt
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_10, 0x005b));     // Loop forever on MicroWake interrupts
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x0800, 0x0800)); // Enable patch vector 3
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x800f));     // Trigger MicroWake interrupt to make safe to reset

        // Assert reset after micro is trapped in a loop (averts micro-SMI access deadlock at reset)
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_0, 0x0000, 0x8000));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x0000));     // Make sure no MicroWake persists after reset
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x0000, 0x0800)); // Disable patch vector 3

        break;
    default:
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_0, 0, 0x8000));
        break;
    }
    return vtss_phy_page_std(vtss_state, port_no);
}


//
// Conversion from vtss_phy_media_interface_t to printable text.
//
// In : media_if_type : The interface type
//
// Return :  Printable text
static const char *vtss_phy_media_if2txt(vtss_phy_media_interface_t media_if_type)
{
    switch (media_if_type) {
    case VTSS_PHY_MEDIA_IF_CU:
        return "VTSS_PHY_MEDIA_IF_CU";
    case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
        return "VTSS_PHY_MEDIA_IF_SFP_PASSTHRU";
    case VTSS_PHY_MEDIA_IF_FI_1000BX:
        return "VTSS_PHY_MEDIA_IF_FI_1000BX";
    case VTSS_PHY_MEDIA_IF_FI_100FX:
        return "VTSS_PHY_MEDIA_IF_FI_100FX";
    case VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU:
        return "VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU";
    case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
        return "VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU";
    case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
        return "VTSS_PHY_MEDIA_IF_AMS_CU_1000BX";
    case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
        return "VTSS_PHY_MEDIA_IF_AMS_FI_1000BX";
    case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
        return "VTSS_PHY_MEDIA_IF_AMS_CU_100FX";
    case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
        return "VTSS_PHY_MEDIA_IF_AMS_FI_100FX";
    }
    return "Media Interface not defined";
}


//
// Conversion from vtss_phy_mac_interface_t to printable text.
//
// In : media_if_type : The interface type
//
// Return :  Printable text
static const char *vtss_phy_mac_if2txt(vtss_port_interface_t mac_if_type)
{
    switch (mac_if_type) {
    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        return "No connection";
    case VTSS_PORT_INTERFACE_LOOPBACK:
        return "Internal loopback in MAC ";
    case VTSS_PORT_INTERFACE_INTERNAL:
        return "Internal interface ";
    case VTSS_PORT_INTERFACE_MII:
        return "MII (RMII does not exist) ";
    case VTSS_PORT_INTERFACE_GMII:
        return "GMII ";
    case VTSS_PORT_INTERFACE_RGMII:
        return "RGMII ";
    case VTSS_PORT_INTERFACE_TBI:
        return "TBI ";
    case VTSS_PORT_INTERFACE_RTBI:
        return "RTBI ";
    case VTSS_PORT_INTERFACE_SGMII:
        return "SGMII ";
    case VTSS_PORT_INTERFACE_SERDES:
        return "SERDES ";
    case VTSS_PORT_INTERFACE_VAUI:
        return "VAUI ";
    case VTSS_PORT_INTERFACE_100FX:
        return "100FX ";
    case VTSS_PORT_INTERFACE_XAUI:
        return "XAUI ";
    case VTSS_PORT_INTERFACE_RXAUI:
        return "RXAUI ";
    case VTSS_PORT_INTERFACE_XGMII:
        return "XGMII ";
    case VTSS_PORT_INTERFACE_SPI4:
        return "SPI4 ";
    case VTSS_PORT_INTERFACE_SGMII_CISCO:
        return "SGMII_CISCO ";
    case VTSS_PORT_INTERFACE_QSGMII:
        return "QSGMII ";
    default:
        return "Mac Interface not defined";
    }
}

// Function that returns TRUE if the port is any AMS mode, else FALSE
//
// In : port_mode : Port in question
//
// Return : TRUE if the port is any AMS mode, else FALSE
static BOOL is_phy_in_ams_mode(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    u16 reg_val;
    u16 media_operating_mode;

    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL, &reg_val));
    media_operating_mode = (reg_val >> 8) & 0x7;
    return  media_operating_mode > 0x3; // See datasheet register 23, non AMS modes are 0x0, 0x1, 0x2, 0x3.
}

static vtss_rc vtss_phy_i2c_wait_for_ready(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    u16 timeout = 500;
    u16 reg_value;

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_I2C_MUX_CONTROL_2, &reg_value));
    while ((reg_value & 0x8000) == 0 && timeout > 0) {
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_I2C_MUX_CONTROL_2, &reg_value));
        timeout--; // Make sure that we don't run forever
        VTSS_MSLEEP(1);
    }
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    if (timeout == 0) {
        VTSS_E("vtss_phy_i2c_wait_for_ready timeoout, port_no %u", port_no);
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;
}

// Function for doing phy i2c reads
// In: port_no - The PHY port number starting from 0.
static vtss_rc vtss_phy_i2c_wr_private(vtss_state_t *vtss_state, vtss_port_no_t port_no, u8 i2c_mux, u8 i2c_reg_addr, u8 i2c_device_addr, u8 *value, u8 cnt)
{
    u8 i;
    u32 reg_val;

    VTSS_D("i2c_mux = %d, i2c_reg_addr = 0x%X, i2c_device_addr =0x%X", i2c_mux, i2c_reg_addr, i2c_device_addr);

    VTSS_RC(vtss_phy_i2c_wait_for_ready(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

    reg_val = VTSS_F_PHY_I2C_MUX_CONTROL_1_DEV_ADDR(i2c_device_addr) |
              VTSS_F_PHY_I2C_MUX_CONTROL_1_SCL_CLK_FREQ(1) |
              (i2c_mux == 3 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_3_ENABLE : 0) |
              (i2c_mux == 2 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_2_ENABLE : 0) |
              (i2c_mux == 1 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_1_ENABLE : 0) |
              (i2c_mux == 0 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_0_ENABLE : 0);

    VTSS_D("REg_val = 0x%X, 0x%X", reg_val, VTSS_F_PHY_I2C_MUX_CONTROL_1_DEV_ADDR(i2c_device_addr));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_I2C_MUX_CONTROL_1,
                        VTSS_F_PHY_I2C_MUX_CONTROL_1_DEV_ADDR(i2c_device_addr) |
                        VTSS_F_PHY_I2C_MUX_CONTROL_1_SCL_CLK_FREQ(1) |
                        (i2c_mux == 3 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_3_ENABLE : 0) |
                        (i2c_mux == 2 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_2_ENABLE : 0) |
                        (i2c_mux == 1 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_1_ENABLE : 0) |
                        (i2c_mux == 0 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_0_ENABLE : 0)));



    for (i = 0; i < cnt; i++) {
        // setup data to be written
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_I2C_MUX_DATA_READ_WRITE, value[i]));

        // Execute the write
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_I2C_MUX_CONTROL_2,
                            VTSS_F_PHY_I2C_MUX_CONTROL_2_MUX_READY |
                            VTSS_F_PHY_I2C_MUX_CONTROL_2_PHY_PORT_ADDR(i2c_mux) |
                            VTSS_F_PHY_I2C_MUX_CONTROL_2_ENA_I2C_MUX_ACCESS |
                            VTSS_F_PHY_I2C_MUX_CONTROL_2_ADDR(i2c_reg_addr + i)));

        VTSS_D("i2c_reg_addr:%d", i2c_reg_addr);
        VTSS_RC(vtss_phy_i2c_wait_for_ready(vtss_state, port_no));
    }

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}

// Function for doing phy i2c writes
// In: port_no - The PHY port number starting from 0.
static vtss_rc vtss_phy_i2c_rd_private(vtss_state_t *vtss_state, vtss_port_no_t port_no, u8 i2c_mux, u8 i2c_reg_addr, u8 i2c_device_addr, u8 *value, u8 cnt)
{

    u32 reg_val32;
    u16 reg_val16;
    u8 i;

    VTSS_D("i2c_mux = %d, i2c_reg_addr = 0x%X, i2c_device_addr =0x%X", i2c_mux, i2c_reg_addr, i2c_device_addr);

    VTSS_RC(vtss_phy_i2c_wait_for_ready(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

    reg_val32 = VTSS_F_PHY_I2C_MUX_CONTROL_1_DEV_ADDR(i2c_device_addr) |
                VTSS_F_PHY_I2C_MUX_CONTROL_1_SCL_CLK_FREQ(1) |
                (i2c_mux == 3 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_3_ENABLE : 0) |
                (i2c_mux == 2 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_2_ENABLE : 0) |
                (i2c_mux == 1 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_1_ENABLE : 0) |
                (i2c_mux == 0 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_0_ENABLE : 0);

    VTSS_D("REg_val = 0x%X, 0x%X", reg_val32, VTSS_F_PHY_I2C_MUX_CONTROL_1_DEV_ADDR(i2c_device_addr));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_I2C_MUX_CONTROL_1,
                        VTSS_F_PHY_I2C_MUX_CONTROL_1_DEV_ADDR(i2c_device_addr) |
                        VTSS_F_PHY_I2C_MUX_CONTROL_1_SCL_CLK_FREQ(1) |
                        (i2c_mux == 3 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_3_ENABLE : 0) |
                        (i2c_mux == 2 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_2_ENABLE : 0) |
                        (i2c_mux == 1 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_1_ENABLE : 0) |
                        (i2c_mux == 0 ? VTSS_F_PHY_I2C_MUX_CONTROL_1_PORT_0_ENABLE : 0)));



    for (i = 0; i < cnt; i++) {
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_I2C_MUX_CONTROL_2,
                            VTSS_F_PHY_I2C_MUX_CONTROL_2_MUX_READY |
                            VTSS_F_PHY_I2C_MUX_CONTROL_2_PHY_PORT_ADDR(i2c_mux) |
                            VTSS_F_PHY_I2C_MUX_CONTROL_2_ENA_I2C_MUX_ACCESS |
                            VTSS_F_PHY_I2C_MUX_CONTROL_2_RD |
                            VTSS_F_PHY_I2C_MUX_CONTROL_2_ADDR(i2c_reg_addr + i)));


        VTSS_RC(vtss_phy_i2c_wait_for_ready(vtss_state, port_no));


        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_I2C_MUX_DATA_READ_WRITE, &reg_val16));

        value[i] = (reg_val16 & VTSS_M_PHY_I2C_MUX_DATA_READ_WRITE_READ_DATA) >> 8;
        VTSS_D("i2c_reg_addr:%d", i2c_reg_addr);
    }

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}


static vtss_rc vtss_phy_rd_wr_masked(vtss_state_t         *vtss_state,
                                     BOOL                 read,
                                     const vtss_port_no_t port_no,
                                     const u32            addr,
                                     u16                  *const value,
                                     const u16            mask)
{
    vtss_rc           rc = VTSS_RC_OK;
    vtss_miim_read_t  read_func;
    vtss_miim_write_t write_func;
    u16               reg, page, val;

    /* Setup read/write function pointers */
    read_func = vtss_state->init_conf.miim_read;
    write_func = vtss_state->init_conf.miim_write;

    /* Page is encoded in address */
    page = (addr >> 5);
    reg = (addr & 0x1f);

    /* Change page */
    if (page) {
        rc = write_func(vtss_state, port_no, 31, page);
    }

    if (rc == VTSS_RC_OK) {
        if (read) {
            /* Read */
            rc = read_func(vtss_state, port_no, reg, value);
            VTSS_N("Read - port:%d, reg:0x%X, value:0x%X", port_no, reg, *value);
        } else if (mask != 0xffff) {
            /* Read-modify-write */
            if ((rc = read_func(vtss_state, port_no, reg, &val)) == VTSS_RC_OK) {
                rc = write_func(vtss_state, port_no, reg, (val & ~mask) | (*value & mask));
            }

            VTSS_N("Read-modify-write - port:%d, reg:0x%X, val:0x%X, value:0x%X, mask:0x%X", port_no, reg, val, *value, mask);
        } else {
            /* Write */
            rc = write_func(vtss_state, port_no, reg, *value);
            VTSS_N("Write - port:%d, reg:0x%X, value:0x%X", port_no, reg, *value);
        }
    }

    /* Restore standard page */
    if (page && rc == VTSS_RC_OK) {
        rc = write_func(vtss_state, port_no, 31, VTSS_PHY_PAGE_STANDARD);
    }

    return rc;
}

/* Read PHY register - used for legacy code where the programmer keep track of the page.*/
vtss_rc vtss_phy_rd(vtss_state_t         *vtss_state,
                    const vtss_port_no_t port_no,
                    const u32            addr,
                    u16                  *const value)
{
    return vtss_phy_rd_wr_masked(vtss_state, 1, port_no, addr, value, 0);
}



// For debugging - See comment at the do_page_chk
static void vtss_phy_do_page_chk_set_private(BOOL enable)
{
    do_page_chk = enable;
}

// For debugging - See comment at the do_page_chk
static void vtss_phy_do_page_chk_get_private(BOOL *enable)
{
    *enable = do_page_chk;
}




// See comment at the do_page_chk
static vtss_rc  vtss_phy_do_page_chk(vtss_state_t         *vtss_state,
                                     const vtss_port_no_t port_no,
                                     const u16            page,
                                     const u16            line)
{
    u16 current_page;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];


    switch (ps->family) {
    case VTSS_PHY_FAMILY_MUSTANG:
    case VTSS_PHY_FAMILY_QUATTRO:
    case VTSS_PHY_FAMILY_COBRA:
    case VTSS_PHY_FAMILY_ENZO:
    case VTSS_PHY_FAMILY_LUTON:
    case VTSS_PHY_FAMILY_SPYDER:
        // These chip will return 0x0 from the page register, so they can't be checked.
        // Mustang(8201), Quattro(8224,8234,8244), Cobra(8211,8221), and Luton (1st gen in TSMC)
        return VTSS_RC_OK;
    default:
        // Do the page check
        VTSS_RC(vtss_phy_rd(vtss_state, port_no, 31, &current_page));
        if (current_page != page) {
            VTSS_E("Unexpected page - Current_Page:0x%X, expected page:0x%X, line:%d, port:%d", current_page, page, line, port_no);
        } else {
            VTSS_N("Correct page: Current_Page:0x%X, expected page:0x%X, line:%d", current_page, page, line);
        }
    }
    return VTSS_RC_OK;
}



// See comment at the do_page_chk
/* Read PHY register (include page) - Use this function in combination with the register defines for new code. */
vtss_rc vtss_phy_rd_page(vtss_state_t         *vtss_state,
                         const vtss_port_no_t port_no,
                         const u16            page,
                         const u32            addr,
                         u16                  *const value,
                         const u16            line)
{
    if (do_page_chk) {
        VTSS_RC(vtss_phy_do_page_chk(vtss_state, port_no, page, line));
    }
    return vtss_phy_rd_wr_masked(vtss_state, 1, port_no, addr, value, 0);
}



// See comment at the do_page_chk
/* Write PHY register, masked (including the page) */
vtss_rc vtss_phy_wr_masked_page(vtss_state_t         *vtss_state,
                                const vtss_port_no_t port_no,
                                const u16            page,
                                const u32            addr,
                                const u16            value,
                                const u16            mask,
                                const u16            line)
{
    u16 val = value;
    if (do_page_chk) {
        VTSS_RC(vtss_phy_do_page_chk(vtss_state, port_no, page, line));
    }
    return vtss_phy_rd_wr_masked(vtss_state, 0, port_no, addr, &val, mask);
}


/* Write PHY register, masked - used for legacy code where the programmer keep track of the page.*/
vtss_rc vtss_phy_wr_masked(vtss_state_t         *vtss_state,
                           const vtss_port_no_t port_no,
                           const u32            addr,
                           const u16            value,
                           const u16            mask)
{
    u16 val = value;
    return vtss_phy_rd_wr_masked(vtss_state, 0, port_no, addr, &val, mask);
}

// Macro for doing warm start register writes. Checks if the register has changed. Also inserts the calling line number when doing register writes. Useful for debugging warm start,
#define VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, page_addr, value, mask) vtss_phy_warm_wr_masked(vtss_state, port_no, page_addr, value, mask, 0xFFFF, __FUNCTION__,  __LINE__)

// See VTSS_PHY_WARM_WR_MASKED. Some registers may not contain the last value written to they (e.g. self clearing bits), and therefor must be written without doing read check.
// This is possible with this macro which contains a "chk_mask" bit mask for selecting which bit to do read check of. Use this function with care and ONLY with registers there the read value doesn't reflect the last written value.
#define VTSS_PHY_WARM_WR_MASKED_CHK_MASK(vtss_state, port_no, page_addr, value, mask, chk_mask) vtss_phy_warm_wr_masked(vtss_state, port_no, page_addr, value, mask, chk_mask, __FUNCTION__, __LINE__)

// Function for doing phy register writes for functions that supports warm start.
// This function only writes to the register if the value is different from what is currently in the register in order not to affect traffic
// In : port_no - Phy port number
//      addr    - The register address in the phy.
//      value   - The data to be written.
//      mask     - Bit mask for bit to be written.
//      chk_mask - Bit mask for bit to be checked.
//      function - Pointer to string containing the name of the calling function.
//      line     - Line number calling the function.
static vtss_rc vtss_phy_warm_wr_masked(vtss_state_t         *vtss_state,
                                       const vtss_port_no_t port_no,
                                       const u16            page,
                                       const u32            addr,
                                       const u16            value,
                                       const u16            mask,
                                       const u16            chk_mask,
                                       const char           *function,
                                       const u16            line)

{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    u16     val;

    // If in warm start phase then only update the register if the value is different from the current register value.
    if (vtss_state->sync_calling_private) {
        VTSS_RC(vtss_phy_rd_page(vtss_state, port_no, page, addr, &val, line));
        if ((val ^ value) & mask & chk_mask) { /* Change in bit field */
            VTSS_E("Warm start synch. field changed: port:%02d, register:0x%02X, mask:0x%04X, from value:0x%04X to value:0x%04X by function:%s, line:%d",
                   port_no + 1, addr, mask, val, value, function, line);
            VTSS_I("warm_start_reg_changed:%d, chk_mask:%d", ps->warm_start_reg_changed, chk_mask);

            ps->warm_start_reg_changed = TRUE; // Signaling the a register for this port port has changed.

            VTSS_RC(vtss_phy_wr_masked_page(vtss_state, port_no, page, addr, value, mask, line));
        }
    } else {
        VTSS_RC(vtss_phy_wr_masked_page(vtss_state, port_no, page, addr, value, mask, line));
    }
    return VTSS_RC_OK;
}

// Function returning VTSS_RC_ERROR if any registers were unexpected needed to be changed during warm start
static vtss_rc vtss_phy_warm_start_failed_get_private(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    VTSS_I("warm_start_reg_changed:%d", ps->warm_start_reg_changed);
    return ps->warm_start_reg_changed ? VTSS_RC_ERROR : VTSS_RC_OK;
}

/* Write PHY register */
vtss_rc vtss_phy_wr(vtss_state_t         *vtss_state,
                    const vtss_port_no_t port_no,
                    const u32            addr,
                    const u16            value)
{
    u16 val = value;
    return vtss_phy_rd_wr_masked(vtss_state, 0, port_no, addr, &val, 0xffff);
}


/* Write PHY register */
// See comment at the do_page_chk
vtss_rc vtss_phy_wr_page(vtss_state_t         *vtss_state,
                         const vtss_port_no_t port_no,
                         const u16            page,
                         const u32            addr,
                         const u16            value,
                         const u16            line)
{
    u16 val = value;
    if (do_page_chk) {
        VTSS_RC(vtss_phy_do_page_chk(vtss_state, port_no, page, line));
    }
    return vtss_phy_rd_wr_masked(vtss_state, 0, port_no, addr, &val, 0xFFFF);
}

// Macro that inserts the calling line number when doing register writes. Useful for debugging warm start,
#define VTSS_PHY_WARM_WR(vtss_state, port_no, page_addr, value) vtss_phy_warm_wr(vtss_state, port_no, page_addr, value, __FUNCTION__, __LINE__)

// Function for doing phy register writes for functions that supports warm start.
// This function only writes to the register if the value is different from what is currently in the register in order not to affect traffic
// In : port_no - Phy port number
//      addr    - The register address in the phy.
//      value   - The data to be written.
//      function - Pointer to string containing the name of the calling function.
//      line     - Line number calling the function.
static vtss_rc vtss_phy_warm_wr(vtss_state_t         *vtss_state,
                                const vtss_port_no_t port_no,
                                const u16            page,
                                const u32            addr,
                                const u16            value,
                                const char           *function,
                                const u16            line)
{
    return vtss_phy_warm_wr_masked(vtss_state, port_no, page, addr, value, 0xFFFF, 0xFFFF, function, line);
}


// Clause 45 register access setup. Setups the MMD register access with the address.
//
static vtss_rc vtss_phy_mmd_reg_setup(vtss_state_t         *vtss_state,
                                      const vtss_port_no_t port_no,
                                      const u8  devad,
                                      const u16 addr)
{
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MMD_EEE_ACCESS, devad)); // Setup cmd=address + devad address
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MMD_ADDR_OR_DATA, addr));  // Setup address
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MMD_EEE_ACCESS, (1 << 14) + devad)); // Setup cmd=data + devad address
    return VTSS_RC_OK;
}


// Clause 45 writes
static vtss_rc vtss_phy_mmd_wr(vtss_state_t *vtss_state,
                               const vtss_port_no_t port_no,
                               const u8  devad,
                               const u16 addr,
                               const u16 data_val)
{
    VTSS_RC(vtss_phy_mmd_reg_setup(vtss_state, port_no, devad, addr));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MMD_ADDR_OR_DATA, data_val));  // Write data

    return VTSS_RC_OK;
}


// Clause 45 reads
static vtss_rc vtss_phy_mmd_rd(vtss_state_t *vtss_state,
                               const vtss_port_no_t port_no,
                               const u8 devad,
                               const u16 addr,
                               u16   *value)
{
    VTSS_RC(vtss_phy_mmd_reg_setup(vtss_state, port_no, devad, addr));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MMD_ADDR_OR_DATA, value));  // read data

    return VTSS_RC_OK;
}

#if defined(VTSS_FEATURE_EEE)
// Clause 45 read-modify mask writes
static vtss_rc vtss_phy_mmd_reg_wr_masked(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no,
                                          const u16 devad,
                                          const u16 addr,
                                          const u16 data_val,
                                          const u16 mask)
{
    u16 current_reg_val;
    VTSS_RC(vtss_phy_mmd_rd(vtss_state, port_no, devad, addr, &current_reg_val)); // Read current value of the register
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MMD_ADDR_OR_DATA, (current_reg_val & ~mask) | (data_val & mask) ));  // Modify current value and write.

    return VTSS_RC_OK;
}
#endif

static u16 rgmii_clock_skew(u16 ps)
{
    return (ps == 0 ? 0 : ps < 1600 ? 1 : ps < 2050 ? 2 : 3);
}

vtss_rc vtss_phy_page_std(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_phy_wr(vtss_state, port_no, 31, VTSS_PHY_PAGE_STANDARD);
}

vtss_rc vtss_phy_page_0x2daf(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_phy_wr(vtss_state, port_no, 31, VTSS_PHY_PAGE_0x2DAF);
}

vtss_rc vtss_phy_page_ext(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_phy_wr(vtss_state, port_no, 31, VTSS_PHY_PAGE_EXTENDED);
}

vtss_rc vtss_phy_page_ext2(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_phy_wr(vtss_state, port_no, 31, VTSS_PHY_PAGE_EXTENDED_2);
}

vtss_rc vtss_phy_page_ext3(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    VTSS_N("Port:%d - Page 3", port_no)
    return vtss_phy_wr(vtss_state, port_no, 31, VTSS_PHY_PAGE_EXTENDED_3);
}

vtss_rc vtss_phy_page_gpio(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_phy_wr(vtss_state, port_no, 31, VTSS_PHY_PAGE_GPIO);
}

// Selecting 1588/ptp page registers
vtss_rc vtss_phy_page_1588(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_phy_wr(vtss_state, port_no, 31, VTSS_PHY_PAGE_1588);
}

// Selecting MACSEC page registers
vtss_rc vtss_phy_page_macsec(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_phy_wr(vtss_state, port_no, 31, VTSS_PHY_PAGE_MACSEC);
}

vtss_rc vtss_phy_page_test(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_phy_wr(vtss_state, port_no, 31, VTSS_PHY_PAGE_TEST);
}

vtss_rc vtss_phy_page_tr(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    return vtss_phy_wr(vtss_state, port_no, 31, VTSS_PHY_PAGE_TR);
}

// Function for mapping the port_no giving to a function to the  physical port number.
static vtss_port_no_t vtss_phy_chip_port(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    u16 reg_val;
    u16 current_page;
    VTSS_RC(vtss_phy_rd(vtss_state, port_no, 31, &current_page));
    VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_4, &reg_val));
    VTSS_D("reg_val:0x%X, chip_no:%d, port_no:%u", reg_val,  (reg_val & VTSS_M_VTSS_PHY_EXTENDED_PHY_CONTROL_4_PHY_ADDRESS) >> 11, port_no);
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 31, current_page));
    return (reg_val & VTSS_M_VTSS_PHY_EXTENDED_PHY_CONTROL_4_PHY_ADDRESS) >> 11;
}

// Function for soft resetting a single phy port
// In: port_no : The phy port number to be soft reset
static vtss_rc vtss_phy_soft_reset_port(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_mtimer_t         timer;
    u16                   reg;

    VTSS_D("Soft resetting port:%d", port_no);
    if (!vtss_state->sync_calling_private || ps->warm_start_reg_changed) { // Don't reset during warm start
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL,
                                   VTSS_F_PHY_MODE_CONTROL_SW_RESET,
                                   VTSS_F_PHY_MODE_CONTROL_SW_RESET)); // Reset phy port

        VTSS_MSLEEP(40);/* pause after reset */
        VTSS_MTIMER_START(&timer, 5000); /* Wait up to 5 seconds */
        while (1) {
            if (PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, &reg) == VTSS_RC_OK && (reg & VTSS_F_PHY_MODE_CONTROL_SW_RESET) == 0) {
                break;
            }
            VTSS_MSLEEP(20);
            if (VTSS_MTIMER_TIMEOUT(&timer)) {
                VTSS_E("port_no %u, reset timeout, reg = 0x%X", port_no, reg);
                return VTSS_RC_ERROR;
            }
        }
        VTSS_MTIMER_CANCEL(&timer);

        // After reset of a port, we need to re-configure it
        VTSS_RC(vtss_phy_conf_set_private(vtss_state, port_no));
    }
    return VTSS_RC_OK;
}

// Function for resetting a phy port
// In: port_no : The phy port number to be reset
static vtss_rc port_reset(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    VTSS_RC(atom_patch_suspend(vtss_state, port_no, TRUE)); // Suspend the micro patch while resetting

    VTSS_RC(vtss_phy_soft_reset_port(vtss_state, port_no));

    VTSS_RC(atom_patch_suspend(vtss_state, port_no, FALSE)); // Restart micro patch
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_optimize_receiver_init(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    u16 reg17;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];

    VTSS_N("vtss_phy_optimize_receiver_init enter port = %d", port_no);

    /* BZ 1776/1860/2095/2107, part 1/3 and 2/3 */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));

    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        break;
    default:
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_12, 0x0200, 0x0300)); // Not needed for the above
    }

    if (ps->power.vtss_phy_power_dynamic) {
        /* Restore changed dsp setting used by dynamic power saving algorithm */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_TEST_PAGE_12, 0x0000, 0xfc00));

        switch (ps->family) {
        case VTSS_PHY_FAMILY_ATOM:
        case VTSS_PHY_FAMILY_LUTON26:
        case VTSS_PHY_FAMILY_TESLA:
            // Read-modify-write word containing half_comp_en
            VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xafe4));
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg17));
            reg17 = (reg17 & 0xffef); //Clear half_adc as desired
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg17));
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8fe4));
            break;
        default:
            /* Restore 1000-BT LD-Q Current Control */
            // For now don't restore at warm start
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED_CHK_MASK(vtss_state, port_no, VTSS_PHY_TEST_PAGE_24, 0x2000, 0x2000, 0x0));
        }

        if (ps->power.vtss_phy_actiphy_on) {
            ps->power.pwrusage = VTSS_PHY_ACTIPHY_PWR;
        } else {
            ps->power.pwrusage = VTSS_PHY_LINK_DOWN_PWR;
        }
    }
    return vtss_phy_page_std(vtss_state, port_no);
}

static vtss_rc vtss_phy_optimize_dsp(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    /* BZ 2226/2227/2228/2229/2230/2231/2232/2294 */
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xaf8a));               /*- Read PMA internal Register 5 */
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0000, 0x0000)); /*- Configure PMA */
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0008, 0x000c)); /*- Register 5 */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f8a));               /*- Write PMA internal Register 5 */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xaf86));               /*- Read PMA internal Register 3 */
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0008, 0x000c)); /*- Configure PMA */
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0000, 0x0000)); /*- Register 3 */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f86));               /*- Write PMA internal Register 3 */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xaf82));               /*- Read PMA internal Register 1 */
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0000, 0x0000)); /*- Configure PMA */
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0100, 0x0180)); /*- Register 1 */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f82));               /*- Write PMA internal Register 1 */

    return VTSS_RC_OK;
}

/* Dynamic Transmit Side Power Optimizations */
#define WORST_SUBCHAN_MSE 120
#define WORST_AVERAGE_MSE 100


/*- EC length power-down optimzations */
/*- 1:Full-length(0), 2:64 taps(5), 3:48 taps(9), 4:32 taps(12), 5:16 taps(14) */
static const u16 ecpdset[] = { 0, 5, 9, 12, 14 };
#define NUM_ECPD_SETTINGS (sizeof(ecpdset)/sizeof(ecpdset[0]))

/* Computes optimal power setting level reductions based on */
/* calculated noise values from the specific cable being used */
static vtss_rc vtss_phy_power_opt(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    u16 reg17, reg18, ecpd_idx, ncpd, half_adc;
    u32 maxMse, meanMse, tempMse, c, r, popt_idx, set[5][3];
    i16 done;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];

    VTSS_D("vtss_phy_power_opt enter, port = %d", port_no);

    reg17 = reg18 = ecpd_idx = maxMse = meanMse = done = half_adc = 0;
    ncpd = 1;


    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        half_adc = 1;
        break;
    default:
        ncpd = 1;
        break;
    }

    /*- Prepare TR node for MSE Averaging - Normkmse_m7 = 3 (default 0) */
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
    VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa3aa));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg17));
    VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0003));
    VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg17));
    VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x83aa));

    for (c = 0; c < 3; c++) {
        for (r = 0; r < 5; r++) {
            set[r][c] = (5 * c) + (r + 1);
        }
    }

    while (done < 2) {
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_TEST_PAGE_12, ((ecpdset[ecpd_idx] << 12) | (ncpd << 10)), 0xfc00));

        switch (ps->family) {
        case VTSS_PHY_FAMILY_ATOM:
        case VTSS_PHY_FAMILY_LUTON26:
        case VTSS_PHY_FAMILY_TESLA:
        case VTSS_PHY_FAMILY_VIPER:
        case VTSS_PHY_FAMILY_ELISE:
            // Read-modify-write word containing half_comp_en
            VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xafe4));
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg17));
            reg17 = (reg17 & 0xffef) | ((half_adc & 1) << 4); //Seat or clear half_adc as desired
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg17));
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8fe4));
            break;
        default:
            break;
        }


        if (done != 0) {
            if (done < 0) {
                VTSS_MSLEEP(50);
            }
            ++done;
        } else {
            /*- Read MSE A and B */
            VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa3c0));
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg17));
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, &reg18));

            maxMse = (reg17 & 0x0fff);
            tempMse = (reg17 >> 12) | (reg18 << 4);
            meanMse = maxMse + tempMse;
            if (tempMse > maxMse) {
                maxMse = tempMse;
            }

            /*- Read MSE C and D */
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa3c2));
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg17));
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, &reg18));

            meanMse += (reg17 & 0x0fff);
            if ((reg17 & 0x0fff) > maxMse) {
                maxMse = (reg17 & 0x0fff);
            }
            tempMse = (reg17 >> 12) | (reg18 << 4);
            meanMse += tempMse;
            if (tempMse > maxMse) {
                maxMse = tempMse;
            }
            meanMse /= 4;

            if ((maxMse >= WORST_SUBCHAN_MSE) || (meanMse >= WORST_AVERAGE_MSE)) {
                if (ncpd == 0) {    /*- ADC Optimization went too far */
                    half_adc--;
                    ncpd = 1;
                } else if (ecpd_idx == 0) {    /*- NC Optimization went too far */
                    ncpd--;
                    done = -1;
                    ecpd_idx = 1;
                } else {                /*- EC Optimization went too far */
                    ecpd_idx--;
                    done = 1;
                }
            } else if ((ecpd_idx == 0) && (ncpd < 2)) {
                ncpd++;
            } else if (++ecpd_idx >= NUM_ECPD_SETTINGS) {
                done = 2;
            }
        }
    }

    if (ecpd_idx >= 1) {
        popt_idx = set[ecpd_idx - 1][ncpd];
    } else {
        popt_idx = set[0][ncpd];
    }


    vtss_state->phy_state[port_no].power.pwrusage = ((ncpd > 0 || ecpd_idx > 0) ?
                                                     popt_idx : VTSS_PHY_LINK_UP_FULL_PWR);

    /*- Restore Normkmse_m7 = 0 */
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
    VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa3aa));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg17));
    VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0000));
    VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg17));
    VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x83aa));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    VTSS_D("ncpd 0x%X, ecpd_idx = %d", ncpd, ecpd_idx);

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_optimize_receiver_reconfig(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    u16                   vga_state_a;
    i16                   max_vga_state_to_optimize;


    VTSS_N("vtss_phy_optimize_receiver_reconfig enter port_no = %u", port_no);

    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        //65nm PHY adjusted VGA window to more effectively use dynamic range
        //as a result, VGA gains for a given cable length are higher here.
        max_vga_state_to_optimize = -9;
        break;
    default:
        max_vga_state_to_optimize = -12;
        break;
    }

    /* BZ 1776/1860/2095/2107 part 3/3 */
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
    VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xaff0));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &vga_state_a));

    vga_state_a = ((vga_state_a >> 4) & 0x01f);
    /* vga_state_a is a 2's complement signed number ranging from -13 to +8 */
    /* Test for vga_state_a < 16 is really a check for positive vga_state_a */
    if ((vga_state_a < 16) || (vga_state_a > (max_vga_state_to_optimize & 0x1f))) {
        VTSS_N("vga_state_a = %u", vga_state_a);
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_TEST_PAGE_12, 0x0000, 0x0300));
        ps->power.pwrusage = VTSS_PHY_LINK_UP_FULL_PWR;
    } else {    /*- Short loop */
        VTSS_D("ps->power.vtss_phy_power_dynamic = %d", ps->power.vtss_phy_power_dynamic)
        if (ps->power.vtss_phy_power_dynamic) {
            VTSS_RC(vtss_phy_page_test(vtss_state, port_no));

            switch (ps->family) {
            case VTSS_PHY_FAMILY_ATOM:
            case VTSS_PHY_FAMILY_LUTON26:
            case VTSS_PHY_FAMILY_TESLA:
            case VTSS_PHY_FAMILY_VIPER:
            case VTSS_PHY_FAMILY_ELISE:
                break;
            default:
                /*- Transmit side power reduction - 1000-BT LD-Q Current Control */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_24, 0x0000, 0x2000));
                break;
            }

            /*- Dynamic Power Reduction */
            VTSS_RC(vtss_phy_power_opt(vtss_state, port_no));
        } else {
            ps->power.pwrusage = VTSS_PHY_LINK_UP_FULL_PWR;
        }
    }
    return vtss_phy_page_std(vtss_state, port_no);
}

static vtss_rc vtss_phy_optimize_jumbo(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    /* BZ 1799/1801 */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0000));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0E35));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x9786));
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));
    return vtss_phy_page_std(vtss_state, port_no);
}

static vtss_rc vtss_phy_optimize_rgmii_strength(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    /* BZ 2094 */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_3, 0xf082));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_3, 0xf082));
    return vtss_phy_page_std(vtss_state, port_no);
}


// Function for setting mdi control register when in force mdi mode.
static vtss_rc vtss_phy_mdi_control_reg(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_port_status_t   status;
    VTSS_RC(vtss_phy_status_get_private(vtss_state, port_no, &status));


    if (status.speed == VTSS_SPEED_1G) {
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, 0x0000,
                                        VTSS_F_PHY_BYPASS_CONTROL_HP_AUTO_MDIX_AT_FORCE));
        VTSS_N("speed:%d", status.speed);
    } else {
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, VTSS_F_PHY_BYPASS_CONTROL_HP_AUTO_MDIX_AT_FORCE,
                                        VTSS_F_PHY_BYPASS_CONTROL_HP_AUTO_MDIX_AT_FORCE));
    }

    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL,
                                    0,
                                    VTSS_F_PHY_BYPASS_CONTROL_DISABLE_PARI_SWAP_CORRECTION));

    return VTSS_RC_OK;
}

// Function for configuring MDI for a given port, with the MDI selected in the vtss_state.
static vtss_rc vtss_phy_mdi_setup(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_phy_conf_t       *conf = &ps->setup;

    switch (conf->mdi) {
    case VTSS_PHY_MDIX_AUTO:
        VTSS_N("VTSS_PHY_MDIX_AUTO, port:%d", port_no);
        // Enable  HP AUto-MDIX, and en pair swap correction.
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, 0x0000,
                                        VTSS_F_PHY_BYPASS_CONTROL_HP_AUTO_MDIX_AT_FORCE | VTSS_F_PHY_BYPASS_CONTROL_DISABLE_PARI_SWAP_CORRECTION));

        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EXTENDED_MODE_CONTROL,
                                        0x0, // Cu media forced normal HP-AUTO-MDIX operation, See datasheet
                                        VTSS_M_PHY_EXTENDED_MODE_CONTROL_FORCE_MDI_CROSSOVER));

        break;

    case VTSS_PHY_MDIX:
        VTSS_N("VTSS_PHY_MDIX, port:%d", port_no);

        VTSS_RC(vtss_phy_mdi_control_reg(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EXTENDED_MODE_CONTROL,
                                        0x000C, // Cu media forced MDI-X, See datasheet
                                        VTSS_M_PHY_EXTENDED_MODE_CONTROL_FORCE_MDI_CROSSOVER));
        break;

    case VTSS_PHY_MDI:
        VTSS_N("VTSS_PHY_MDI, port:%d", port_no);

        VTSS_RC(vtss_phy_mdi_control_reg(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EXTENDED_MODE_CONTROL,
                                        0x0008, // Cu media forced MDI, See datasheet
                                        VTSS_M_PHY_EXTENDED_MODE_CONTROL_FORCE_MDI_CROSSOVER));


        break;
    default:
        VTSS_E("Unknown mdi mode:%d, port:%d", conf->mdi, port_no);

    }
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}


static vtss_rc vtss_phy_detect(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_rc               rc = VTSS_RC_OK;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    u16                   reg2, reg3, model;
    u32                   oui;

    /* Only detect PHY once (avoid overwriting base_port_no) */
    if (ps->family != VTSS_PHY_FAMILY_NONE) {
        return VTSS_RC_OK;
    }

    ps->features = 0;

    /* Detect PHY type and family -- */
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_IDENTIFIER_1, &reg2));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_IDENTIFIER_2, &reg3));
    oui = ((reg2 << 6) | ((reg3 >> 10) & 0x3F));
    model = ((reg3 >> 4) & 0x3F);
    ps->type.revision = (reg3 & 0xF);

    ps->conf_none = (oui == 0x005043 && model == 2 && ps->type.revision == 4 ? 1 : 0);
    ps->type.port_cnt = 1; //Default to 1, just to make sure that it is always set.
    ps->type.phy_api_base_no = 0;
    ps->type.channel_id = 0;
    ps->family = VTSS_PHY_FAMILY_NONE; // Default no phy detected


    /* Lookup PHY Family and Type */
    if (oui == 0x0001C1) {
        /* Vitesse OUI */
        switch (model) {
        case 0x02:
            ps->type.part_number = VTSS_PHY_TYPE_8601;
            ps->family = VTSS_PHY_FAMILY_COOPER;
            ps->type.port_cnt = 1;
            break;
        case 0x03:
            ps->type.part_number = VTSS_PHY_TYPE_8641;
            ps->family = VTSS_PHY_FAMILY_COOPER;
            ps->type.port_cnt = 1;
            break;
        case 0x05:
            ps->type.part_number = VTSS_PHY_TYPE_7385;
            ps->family = VTSS_PHY_FAMILY_LUTON;
            ps->type.port_cnt = 5;
            break;
        case 0x08:
            if (ps->reset.mac_if == VTSS_PORT_INTERFACE_INTERNAL) {
                ps->type.part_number = VTSS_PHY_TYPE_7388;
                ps->family = VTSS_PHY_FAMILY_LUTON;
                ps->type.port_cnt = 8;
            } else {
                /* VSC8538 revision A1 */
                ps->type.part_number = VTSS_PHY_TYPE_8538;
                ps->family = VTSS_PHY_FAMILY_SPYDER;
                ps->type.port_cnt = 8;
            }
            break;
        case 0x09: /* VSC7389 SparX-G16 */


        case 0x0a: // VSC8574 Tesla
            ps->type.part_number = VTSS_PHY_TYPE_8574;
            ps->family = VTSS_PHY_FAMILY_TESLA;
            ps->type.port_cnt = 4;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            break;

        case 0x0C : // VSC8504 Tesla
            ps->type.part_number = VTSS_PHY_TYPE_8504;
            ps->family = VTSS_PHY_FAMILY_TESLA;
            ps->type.port_cnt = 4;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            break;

        case 0x0D : // VSC8572 Tesla
            ps->type.part_number = VTSS_PHY_TYPE_8572;
            ps->family = VTSS_PHY_FAMILY_TESLA;
            ps->type.port_cnt = 2;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            break;

        case 0x0E : // VSC8552 Tesla
            ps->type.part_number = VTSS_PHY_TYPE_8552;
            ps->family = VTSS_PHY_FAMILY_TESLA;
            ps->type.port_cnt = 2;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            break;

        case 0x0F : // VSC8502 Tesla
            ps->type.part_number = VTSS_PHY_TYPE_8502;
            ps->family = VTSS_PHY_FAMILY_TESLA;
            ps->type.port_cnt = 2;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            break;

        case 0x10:
            ps->type.part_number = VTSS_PHY_TYPE_7390;
            ps->family = VTSS_PHY_FAMILY_LUTON24;
            ps->type.port_cnt = 12;
            break;

        case 0x11: /* VSC7391 SparX-G16R */
            ps->type.part_number = VTSS_PHY_TYPE_7389;
            ps->family = VTSS_PHY_FAMILY_LUTON24;
            ps->type.port_cnt = 8;
            break;

        case 0x15:
            ps->type.part_number = VTSS_PHY_TYPE_7395;
            ps->family = VTSS_PHY_FAMILY_LUTON_E;
            ps->type.port_cnt = 5;
            break;

        case 0x18:
            if (ps->reset.mac_if == VTSS_PORT_INTERFACE_INTERNAL) {
                ps->type.part_number = VTSS_PHY_TYPE_7398;
                ps->family = VTSS_PHY_FAMILY_LUTON_E;
                ps->type.port_cnt = 8;
            } else {
                /* VSC8558 revision A1 */
                ps->type.part_number = VTSS_PHY_TYPE_8558;
                ps->family = VTSS_PHY_FAMILY_SPYDER;
                ps->type.port_cnt = 8;
            }
            break;
        case 0x24:
            ps->type.part_number = VTSS_PHY_TYPE_8634;
            ps->family = VTSS_PHY_FAMILY_ENZO;
            ps->type.port_cnt = 4;
            ps->type.channel_id = port_no % 4; // PHY address bit 4:2 are set by CMODE pins. The channel id is bits 0-1. See PHY Datasheet page 31.
            break;
        case 0x26:
            ps->type.part_number = VTSS_PHY_TYPE_8664;
            ps->family = VTSS_PHY_FAMILY_ENZO;
            ps->type.port_cnt = 4;
            ps->type.channel_id = port_no % 4;// PHY address bit 4:2 are set by CMODE pins. The channel id is bits 0-1. See PHY Datasheet page 31.
            break;
        case 0x28:
            ps->type.part_number = VTSS_PHY_TYPE_8538;
            ps->family = VTSS_PHY_FAMILY_SPYDER;
            ps->type.port_cnt = 8;
            break;
        case 0x2D:
            ps->type.part_number = VTSS_PHY_TYPE_7420;
            ps->family = VTSS_PHY_FAMILY_LUTON26;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            ps->type.port_cnt = 10;
            break;
        case 0x2E:
            ps->type.part_number = VTSS_PHY_TYPE_8512;
            ps->family = VTSS_PHY_FAMILY_ATOM;
            ps->type.port_cnt = 12;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            break;
        case 0x2F:
            ps->type.part_number = VTSS_PHY_TYPE_8522;
            ps->family = VTSS_PHY_FAMILY_ATOM;
            ps->type.port_cnt = 12;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            break;
        case 0x38:
            ps->type.part_number = VTSS_PHY_TYPE_8558;
            ps->family = VTSS_PHY_FAMILY_SPYDER;
            ps->type.port_cnt = 8;
            break;
        case 0x35:
            ps->type.part_number = VTSS_PHY_TYPE_8658;
            ps->family = VTSS_PHY_FAMILY_SPYDER;
            ps->type.port_cnt = 8;
            break;

        //
        // Viper family
        //
        case 0x3B: // VSC8582 (dual full-featured)
            ps->type.part_number = VTSS_PHY_TYPE_8582;
            ps->family = VTSS_PHY_FAMILY_VIPER;
            ps->type.port_cnt = 2;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            ps->features |= VTSS_CAP_MACSEC;

            break;
        case 0x3C: // VSC8584 (quad full-featured)
            ps->type.part_number = VTSS_PHY_TYPE_8584;
            ps->family = VTSS_PHY_FAMILY_VIPER;
            ps->type.port_cnt = 4;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            ps->features |= VTSS_CAP_MACSEC;
            break;
        case 0x3D: // VSC8575 (quad w/o MACsec)
            ps->type.part_number = VTSS_PHY_TYPE_8575;
            ps->family = VTSS_PHY_FAMILY_VIPER;
            ps->type.port_cnt = 4;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            break;
        case 0x3E: //VSC8564 (quad w/o 1588*)
            ps->type.part_number = VTSS_PHY_TYPE_8564;
            ps->family = VTSS_PHY_FAMILY_VIPER;
            ps->type.port_cnt = 4;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            ps->features |= VTSS_CAP_MACSEC;
            break;
        case 0x3F: //VSC8586 - (reserved SKU)
            VTSS_E("Please implement CHIP information for VSC8586");
            break;

        //
        // ELISE family - Cost optimized VIPER (Without SGMII, FIBER media, 1588, MacSec and Legacy MAC EEE supprt)
        //
        case 0x27: //
            ps->type.part_number = VTSS_PHY_TYPE_8514;
            ps->family = VTSS_PHY_FAMILY_ELISE;
            ps->type.port_cnt = 4;
            ps->type.channel_id = vtss_phy_chip_port(vtss_state, port_no);
            break;

        default:
            ps->type.part_number = VTSS_PHY_TYPE_NONE;
            ps->family = VTSS_PHY_FAMILY_NONE;
            rc = VTSS_RC_ERROR;
            ps->type.port_cnt = 1;
            break;
        }
    } else if (oui == 0x0003F1) {
        /* Cicada OUI */
        switch (model) {
        case 0x01:
            ps->type.part_number = VTSS_PHY_TYPE_8201;
            ps->family = VTSS_PHY_FAMILY_MUSTANG;
            ps->type.port_cnt = 1;
            break;
        case 0x04:
            ps->type.part_number = VTSS_PHY_TYPE_8204;
            ps->family = VTSS_PHY_FAMILY_BLAZER;
            ps->type.port_cnt = 4;
            break;
        case 0x0B:
            ps->type.part_number = VTSS_PHY_TYPE_8211;
            ps->family = VTSS_PHY_FAMILY_COBRA;
            ps->type.port_cnt = 1;
            break;
        case 0x15:
            ps->type.part_number = VTSS_PHY_TYPE_8221;
            ps->family = VTSS_PHY_FAMILY_COBRA;
            ps->type.port_cnt = 1;
            break;
        case 0x18:
            ps->type.part_number = VTSS_PHY_TYPE_8224;
            ps->family = VTSS_PHY_FAMILY_QUATTRO;
            ps->type.port_cnt = 4;
            break;
        case 0x22:
            ps->type.part_number = VTSS_PHY_TYPE_8234;
            ps->family = VTSS_PHY_FAMILY_QUATTRO;
            ps->type.port_cnt = 4;
            break;
        case 0x2C:
            ps->type.part_number = VTSS_PHY_TYPE_8244;
            ps->family = VTSS_PHY_FAMILY_QUATTRO;
            ps->type.port_cnt = 4;
            break;
        default:
            ps->type.part_number = VTSS_PHY_TYPE_NONE;
            ps->family = VTSS_PHY_FAMILY_NONE;
            ps->type.port_cnt = 1;
            rc = VTSS_RC_ERROR;
            break;
        }
    } else {
        ps->type.part_number = VTSS_PHY_TYPE_NONE;
        ps->family = VTSS_PHY_FAMILY_NONE;
        rc = VTSS_RC_ERROR;
        ps->type.port_cnt = 1;
    }

    /* Capabilities */
    if (ps->family == VTSS_PHY_FAMILY_ATOM    ||
        ps->family == VTSS_PHY_FAMILY_LUTON26 ||
        ps->family == VTSS_PHY_FAMILY_VIPER   ||
        ps->family == VTSS_PHY_FAMILY_ELISE   ||
        ps->family == VTSS_PHY_FAMILY_TESLA) {
        ps->features |= VTSS_CAP_EEE;    /* Can EEE */
    }
    if (ps->family == VTSS_PHY_FAMILY_ATOM    ||
        ps->family == VTSS_PHY_FAMILY_LUTON26 ||
        ps->family == VTSS_PHY_FAMILY_TESLA   ||
        ps->family == VTSS_PHY_FAMILY_VIPER   ||
        ps->family == VTSS_PHY_FAMILY_ELISE   ||
        ps->family == VTSS_PHY_FAMILY_COBRA   ||
        ps->family == VTSS_PHY_FAMILY_ENZO) {
        ps->features |= VTSS_CAP_INT;    /* Can interrupts */
    }


    // Get the channel number (the phy number )
    if (ps->type.part_number == VTSS_PHY_TYPE_NONE) {
        ps->type.channel_id = 0;
        ps->type.base_port_no = 0;
    } else {
        ps->type.base_port_no = port_no - ps->type.channel_id; // Find the first port number for the PHY.
    }

    VTSS_I("port_no:%u, Detect oui:0x%X, model:0x%X, revision:%d, detected:%s PHY:%d_%d (model:0x%02x, features:0x%02x, family:%s)",
           port_no,
           oui, model, ps->type.revision,
           ps->type.part_number != VTSS_PHY_TYPE_NONE ? "Vitesse" : "Unknown",
           ps->type.part_number, ps->type.revision, model, ps->features, vtss_phy_family2txt(ps->family));

    return rc;
}

// Function that returns the chip type/id for a given port.
// In     : port_no  - Internal port (starting from 0)
// In/Out : phy_id - Pointer to phy_id to be returned.
static vtss_rc vtss_phy_id_get_private(vtss_state_t *vtss_state, const vtss_port_no_t port_no, vtss_phy_type_t *phy_id)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    *phy_id = ps->type;
    VTSS_N("channel_id:%d, port_no:%d, phy_api_base_no:%d", phy_id->channel_id, port_no, phy_id->phy_api_base_no);
    return VTSS_RC_OK;
}

//=============================================================================
// 6G Macro setup - Code comes from James McIntosh
//=============================================================================
static vtss_rc vtss_phy_sd6g_ib_cfg0_wr_private(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no,
                                                const u8             ib_rtrm_adj,
                                                const u8             ib_sig_det_clk_sel,
                                                const u8             ib_reg_pat_sel_offset,
                                                const u8             ib_cal_ena)
{
    u32 base_val;
    u32 reg_val;

    // constant terms
    base_val = (1 << 30) | (1 << 29) | (5 << 21) | (1 << 19) | (1 << 14) | (1 << 12) | (2 << 10) | (1 << 5) | (1 << 4) | 7;
    // configurable terms
    reg_val = base_val | ((u32)(ib_rtrm_adj) << 25) | ((u32)(ib_sig_det_clk_sel) << 16) | ((u32)(ib_reg_pat_sel_offset) << 8) | ((u32)(ib_cal_ena) << 3);
    return vtss_phy_macsec_csr_wr_private(vtss_state, port_no, 7, 0x22, reg_val); // ib_cfg0
}


static vtss_rc vtss_phy_sd6g_ib_cfg1_wr_private(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no,
                                                const u8             ib_tjtag,
                                                const u8             ib_tsdet,
                                                const u8             ib_scaly,
                                                const u8             ib_frc_offset)
{
    u32 ib_filt_val;
    u32 ib_frc_val;
    u32 reg_val = 0;

    // constant terms
    ib_filt_val = (1 << 7) + (1 << 6) + (1 << 5) + (1 << 4);
    ib_frc_val = (0 << 3) + (0 << 2) + (0 << 1);
    // configurable terms
    reg_val  = ((u32)ib_tjtag << 17) + ((u32)ib_tsdet << 12) + ((u32)ib_scaly << 8) + ib_filt_val + ib_frc_val + ((u32)ib_frc_offset << 0);
    return vtss_phy_macsec_csr_wr_private(vtss_state, port_no, 7, 0x23, reg_val); // ib_cfg1
}

static vtss_rc vtss_phy_sd6g_ib_cfg2_wr_private(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no,
                                                const u32            ib_cfg2_val)
{
    return vtss_phy_macsec_csr_wr_private(vtss_state, port_no, 7, 0x24, ib_cfg2_val); // ib_cfg2
}

static vtss_rc vtss_phy_sd6g_ib_cfg3_wr_private(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no,
                                                const u8             ib_ini_hp,
                                                const u8             ib_ini_mid,
                                                const u8             ib_ini_lp,
                                                const u8             ib_ini_offset)
{
    u32 reg_val;

    reg_val  = ((u32)ib_ini_hp << 24) + ((u32)ib_ini_mid << 16) + ((u32)ib_ini_lp << 8) + ((u32)ib_ini_offset << 0);
    return vtss_phy_macsec_csr_wr_private(vtss_state, port_no, 7, 0x25, reg_val); // ib_cfg3
}

static vtss_rc vtss_phy_sd6g_ib_cfg4_wr_private(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no,
                                                const u8             ib_max_hp,
                                                const u8             ib_max_mid,
                                                const u8             ib_max_lp,
                                                const u8             ib_max_offset)
{
    u32 reg_val;

    reg_val  = ((u32)ib_max_hp << 24) + ((u32)ib_max_mid << 16) + ((u32)ib_max_lp << 8) + ((u32)ib_max_offset << 0);
    return vtss_phy_macsec_csr_wr_private(vtss_state, port_no, 7, 0x26, reg_val); // ib_cfg4
}

static vtss_rc vtss_phy_sd6g_pll_cfg_wr_private(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no,
                                                const u8             pll_ena_offs,
                                                const u8             pll_fsm_ctrl_data,
                                                const u8             pll_fsm_ena)
{
    u32 reg_val;

    reg_val  = ((u32)pll_ena_offs << 21) + ((u32)pll_fsm_ctrl_data << 8) + ((u32)pll_fsm_ena << 7);
    return vtss_phy_macsec_csr_wr_private(vtss_state, port_no, 7, 0x2b, reg_val); // pll_cfg
}

static vtss_rc vtss_phy_sd6g_common_cfg_wr_private(vtss_state_t *vtss_state,
                                                   const vtss_port_no_t port_no,
                                                   const u8             sys_rst,
                                                   const u8             ena_lane,
                                                   const u8             ena_loop,
                                                   const u8             qrate,
                                                   const u8             if_mode)
{
//  ena_loop = 8 for eloop
//           = 4 for floop
//           = 2 for iloop
//           = 1 for ploop
//  qrate    = 1 for SGMII, 0 for QSGMII
//  if_mode  = 1 for SGMII, 3 for QSGMII

    u32 reg_val;

    reg_val  = ((u32)sys_rst << 31) + ((u32)ena_lane << 18) + ((u32)ena_loop << 8) + ((u32)qrate << 6) + ((u32)if_mode << 4);
    return vtss_phy_macsec_csr_wr_private(vtss_state, port_no, 7, 0x2c, reg_val); // common_cfg
}

static vtss_rc vtss_phy_sd6g_gp_cfg_wr_private(vtss_state_t *vtss_state,
                                               const vtss_port_no_t port_no,
                                               const u32            gp_cfg_val)
{
    return vtss_phy_macsec_csr_wr_private(vtss_state, port_no, 7, 0x2e, gp_cfg_val); // gp_cfg
}

static vtss_rc vtss_phy_sd6g_misc_cfg_wr_private(vtss_state_t *vtss_state,
                                                 const vtss_port_no_t port_no,
                                                 const u8             lane_rst)
{
    // all other bits are 0 for now
    return vtss_phy_macsec_csr_wr_private(vtss_state, port_no, 7, 0x3b, (u32)lane_rst); // misc_cfg
}

// Macro for maing sure that we don't run forever
#define SD6G_TIMEOUT(timeout_var) if (timeout_var-- == 0) {goto macro_6g_err;} else {VTSS_MSLEEP(1);}

static vtss_rc vtss_phy_sd6g_patch_private(vtss_state_t                *vtss_state,
                                           const vtss_port_no_t        port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    BOOL viper_rev_a = FALSE;
    vtss_phy_type_t phy_id;
    u8 rcomp;
    u8 ib_rtrm_adj;
    u8 iter;

    u8 pll_fsm_ctrl_data;
    u8 qrate;
    u8 if_mode;

    u8 ib_sig_det_clk_sel_cal;
    u8 ib_sig_det_clk_sel_mm  = 7;
    u8 ib_tsdet_cal = 16;
    u8 ib_tsdet_mm  = 5;
    u32 rd_dat;
    u8 timeout;
    u8 timeout2;

    u16 reg_val;
    u8 mac_if;

    switch (ps->family) {
    // These chips support the 6G macro setup
    case VTSS_PHY_FAMILY_VIPER:
        viper_rev_a = (ps->type.revision == VTSS_PHY_VIPER_REV_A);

    // Fall through on purpose
    case VTSS_PHY_FAMILY_ELISE:
        break;
    default:
        return VTSS_RC_OK;
    }

    ib_sig_det_clk_sel_cal = viper_rev_a ? 0 : 7; // 7 for Elise and Viper Rev. B+

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MAC_MODE_AND_FAST_LINK, &reg_val));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    mac_if = (reg_val >> 14) & 0x3;

    if (mac_if ==  0x1) { // QSGMII, See data sheet register 19G
        pll_fsm_ctrl_data = 120;
        qrate   = 0;
        if_mode = 3;
    } else if (mac_if ==  0x0) { // SGMII, See data sheet register 19G
        pll_fsm_ctrl_data = 60;
        qrate   = 1;
        if_mode = 1;
    } else {
        return VTSS_RC_OK;
    }


    VTSS_D("Setting 6G");
    VTSS_RC(vtss_phy_id_get_private(vtss_state, port_no, &phy_id));
    VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x40000001)); // read 6G MCB into CSRs

    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB read to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x4000000);

    if (viper_rev_a) {
        timeout = 200;
        // modify RComp value for Viper Rev. A
        VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x11, 0x40000001)); // read LCPLL MCB into CSRs
        do {
            VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x11, &rd_dat)); // wait for MCB read to complete
            SD6G_TIMEOUT(timeout);
        } while (rd_dat & 0x4000000);

        VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x0f, &rd_dat)); // rcomp_status
        rcomp = rd_dat & 0xf; //~10;
        ib_rtrm_adj = rcomp - 3;
    } else {
        // use trim offset for Viper Rev. B+ and Elise
        ib_rtrm_adj = 16 - 3;
    }

    VTSS_D("Initializing 6G...");
    VTSS_RC(vtss_phy_sd6g_ib_cfg0_wr_private(vtss_state, phy_id.base_port_no, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0, 0)); // ib_cfg0
    if (viper_rev_a) {
        VTSS_RC(vtss_phy_sd6g_ib_cfg1_wr_private(vtss_state, phy_id.base_port_no, 8, ib_tsdet_cal, 0, 1)); // ib_cfg1
    } else {
        VTSS_RC(vtss_phy_sd6g_ib_cfg1_wr_private(vtss_state, phy_id.base_port_no, 8, ib_tsdet_cal, 15, 0)); // ib_cfg1
    }
    VTSS_RC(vtss_phy_sd6g_ib_cfg2_wr_private(vtss_state, phy_id.base_port_no, 0x3f878194)); // ib_cfg2
    VTSS_RC(vtss_phy_sd6g_ib_cfg3_wr_private(vtss_state, phy_id.base_port_no,  0, 31, 1, 31)); // ib_cfg3
    VTSS_RC(vtss_phy_sd6g_ib_cfg4_wr_private(vtss_state, phy_id.base_port_no, 63, 63, 2, 63)); // ib_cfg4
    VTSS_RC(vtss_phy_sd6g_pll_cfg_wr_private(vtss_state, phy_id.base_port_no, 3, pll_fsm_ctrl_data, 0)); // pll_cfg
    VTSS_RC(vtss_phy_sd6g_common_cfg_wr_private(vtss_state, phy_id.base_port_no, 0, 0, 0, qrate, if_mode)); // sys_rst, ena_lane
    VTSS_RC(vtss_phy_sd6g_misc_cfg_wr_private(vtss_state, phy_id.base_port_no, 1)); // lane_rst
    VTSS_RC(vtss_phy_sd6g_gp_cfg_wr_private(vtss_state, phy_id.base_port_no, 768)); // gp_cfg
    VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x8000000);

    VTSS_D("Calibrating PLL...");
    VTSS_RC(vtss_phy_sd6g_pll_cfg_wr_private(vtss_state, phy_id.base_port_no, 3, pll_fsm_ctrl_data, 1)); // pll_fsm_ena
    VTSS_RC(vtss_phy_sd6g_common_cfg_wr_private(vtss_state, phy_id.base_port_no, 1, 1, 0, qrate, if_mode)); // sys_rst, ena_lane
    VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB

    do {
        VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x8000000);

    // wait for PLL cal to complete
    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x40000001)); // read 6G MCB into CSRs
        timeout2 = 200;
        do {
            VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB read to complete
            SD6G_TIMEOUT(timeout2);
        } while (rd_dat & 0x4000000);
        VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x31, &rd_dat)); // pll_status
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x0001000); // wait for bit 12 to clear

    VTSS_D("Calibrating IB...");
    // only for Viper Rev. A
    if (viper_rev_a) {
        // one time with SW clock
        VTSS_RC(vtss_phy_sd6g_ib_cfg0_wr_private(vtss_state, phy_id.base_port_no, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0, 1)); // ib_cal_ena
        //VTSS_RC(vtss_phy_sd6g_ib_cfg1_wr_private(vtss_state, phy_id.base_port_no, 8, ib_tsdet_cal, 0, 1)); // ib_cfg1
        VTSS_RC(vtss_phy_sd6g_misc_cfg_wr_private(vtss_state, phy_id.base_port_no, 0)); // lane_rst
        VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
        timeout = 200;
        do {
            VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
            SD6G_TIMEOUT(timeout);
        } while (rd_dat & 0x8000000);
        // 11 cycles w/ SW clock
        for (iter = 0; iter < 11; iter++) {
            VTSS_RC(vtss_phy_sd6g_gp_cfg_wr_private(vtss_state, phy_id.base_port_no, 769)); // set gp(0)
            VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
            timeout = 200;
            do {
                VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
                SD6G_TIMEOUT(timeout);
            } while (rd_dat & 0x8000000);
            VTSS_RC(vtss_phy_sd6g_gp_cfg_wr_private(vtss_state, phy_id.base_port_no, 768)); // clear gp(0)
            VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
            timeout = 200;
            do {
                VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
                SD6G_TIMEOUT(timeout);
            } while (rd_dat & 0x8000000);
        }
    }
    // auto. cal
    if (viper_rev_a) {
        VTSS_RC(vtss_phy_sd6g_ib_cfg0_wr_private(vtss_state, phy_id.base_port_no, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0, 0)); // ib_cal_ena
        VTSS_RC(vtss_phy_sd6g_ib_cfg1_wr_private(vtss_state, phy_id.base_port_no, 8, ib_tsdet_cal, 15, 0)); // ib_cfg1
        VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
        timeout = 200;
        do {
            VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
            SD6G_TIMEOUT(timeout);
        } while (rd_dat & 0x8000000);
    }
    VTSS_RC(vtss_phy_sd6g_ib_cfg0_wr_private(vtss_state, phy_id.base_port_no, ib_rtrm_adj, ib_sig_det_clk_sel_cal, 0, 1)); // ib_cal_ena
    VTSS_RC(vtss_phy_sd6g_misc_cfg_wr_private(vtss_state, phy_id.base_port_no, 0)); // lane_rst
    VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x8000000);
    // wait for IB cal to complete
    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x40000001)); // read 6G MCB into CSRs
        timeout2 = 200;
        do {
            VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB read to complete
            SD6G_TIMEOUT(timeout2);
        } while (rd_dat & 0x4000000);
        VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x2f, &rd_dat)); // ib_status0
        SD6G_TIMEOUT(timeout);
    } while (~rd_dat & 0x0000100); // wait for bit 8 to set

    VTSS_D("Final settings...");
    VTSS_RC(vtss_phy_sd6g_ib_cfg0_wr_private(vtss_state, phy_id.base_port_no, ib_rtrm_adj, ib_sig_det_clk_sel_mm, 1, 1)); // ib_sig_det_clk_sel, ib_reg_pat_sel_offset
    VTSS_RC(vtss_phy_sd6g_ib_cfg1_wr_private(vtss_state, phy_id.base_port_no, 8, ib_tsdet_mm, 15, 0)); // ib_tsdet
    VTSS_RC(vtss_phy_macsec_csr_wr_private(vtss_state, phy_id.base_port_no, 7, 0x3f, 0x80000001)); // write back 6G MCB
    timeout = 200;
    do {
        VTSS_RC(vtss_phy_macsec_csr_rd_private(vtss_state, phy_id.base_port_no, 7, 0x3f, &rd_dat)); // wait for MCB write to complete
        SD6G_TIMEOUT(timeout);
    } while (rd_dat & 0x8000000);

    return VTSS_RC_OK;

macro_6g_err:
    VTSS_E("Viper 6G macro not configured correctly for port:%d", port_no);
    return VTSS_RC_ERR_PHY_6G_MACRO_SETUP;
}

//
// Phy Smart premphasis
//
static vtss_rc vtss_phy_enab_smrt_premphasis (vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    /* BZ 2675 */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200)); //Ensure RClk125 enabled even in powerdown
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa7fa));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0000, 0x0000));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0008, 0x0008));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87fa));
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200)); //Restore RClk125 gating
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    return VTSS_RC_OK;
}

void vga_adc_debug(const vtss_inst_t inst, u8 vga_adc_pwr, vtss_port_no_t port_no)
{
    vtss_state_t *vtss_state;

    // vga_adc_pwr: allows VGA and/or ADC to power down for EEE
    // 00: power down neither
    // 01: power down ADCs only
    // 10: power down VGAs only
    // 11: power down both

    if (vtss_inst_check(inst, &vtss_state) != VTSS_RC_OK) {
        return;
    }

    // turn off micro VGA patch temporarily to allow token-ring access
    (void) vtss_phy_page_gpio(vtss_state, port_no); // switch to micro page (glabal to all 12 PHYs)
    (void) PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9024);

    (void) vtss_phy_page_tr(vtss_state, port_no);   // Switch to token-ring register page
    (void) PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0001);
    (void) PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x40b9 | (vga_adc_pwr << 1));
    (void) PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8fda);   // for 100
    (void) PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0000);
    (void) PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0159 | (vga_adc_pwr << 1));
    (void) PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8fd6);   // for 1000

    // turn micro VGA patch back on to allow correct PHY start-up
    (void) vtss_phy_page_gpio(vtss_state, port_no); // switch to micro page (glabal to all 12 PHYs)
    (void) PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9004);
    (void) vtss_phy_page_std(vtss_state, port_no);
}

// Function for testing phy features
BOOL vtss_phy_can(vtss_state_t *vtss_state, const vtss_port_no_t port_no, vtss_phy_feature_t feature)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    VTSS_N("port_no:%d ps->features:0x%X 0x%X", port_no, ps->features, feature);
    return !!(ps->features & feature);
}

#if defined(VTSS_FEATURE_EEE)
// Function for setting EEE (Energy Efficient Ethernet)
//
// In: port_no      - The PHY port number starting from 0.
//     enable_conf  - 0=Update EEE state to disabled, 1=Update EEE state to enabled, 2=Update EEE register with current state.
//
// Return: VTSS_RC_OK if configuration went well - else error code
static vtss_rc vtss_phy_eee_ena_private(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_rc rc = VTSS_RC_OK;
    BOOL reconfigure = FALSE; // Default don't update registers.

    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
        if (ps->type.revision == VTSS_PHY_ATOM_REV_B) {
            if (vtss_state->phy_state[port_no].eee_conf.eee_mode == EEE_ENABLE) {
                VTSS_RC(atom_patch_suspend(vtss_state, port_no, FALSE)); // Make sure that 8051 patch is running
            } else if (vtss_state->phy_state[port_no].eee_conf.eee_mode == EEE_DISABLE) {
                VTSS_RC(atom_patch_suspend(vtss_state, port_no, TRUE)); // Suspend 8051 Patch.
            }
        }
    // Pass through
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        // Called
        if (vtss_state->phy_state[port_no].eee_conf.eee_mode == EEE_REG_UPDATE) {
            // Called with re-configure registers but don't change state
            if (!vtss_state->sync_calling_private || ps->warm_start_reg_changed) {
                reconfigure = TRUE;
            }
        } else if (ps->eee_conf.eee_ena_phy && vtss_state->phy_state[port_no].eee_conf.eee_mode == EEE_DISABLE) {
            //current state is Enabled, New state is disable. Re-configure registers and change state.
            ps->eee_conf.eee_ena_phy = FALSE;
            reconfigure = TRUE;
        } else if (!ps->eee_conf.eee_ena_phy && vtss_state->phy_state[port_no].eee_conf.eee_mode == EEE_ENABLE) {
            //Current state is Disable, New state is Enable. Re-configure registers and change state.
            ps->eee_conf.eee_ena_phy = TRUE;
            reconfigure = TRUE;
        }


        // re-configure registers
        if (reconfigure) {
            VTSS_D("New EEE Enable = %d, port = %d", vtss_state->phy_state[port_no].eee_conf.eee_mode, port_no);
            // Enable/Disable all EEE
            if (ps->eee_conf.eee_ena_phy) {
                VTSS_RC(vtss_phy_page_ext2(vtss_state, port_no));
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EEE_CONTROL,
                                                VTSS_F_PHY_EEE_CONTROL_ENABLE_10BASE_TE,
                                                VTSS_F_PHY_EEE_CONTROL_ENABLE_10BASE_TE)); // Enable EEE (EEE control, Adress 17E2, bit 15)
                VTSS_RC(vtss_phy_mmd_reg_wr_masked(vtss_state, port_no, 7, 60, 6, 0x0006)); // Enable advertisement

                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));                      // Switch to test-register page
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_TEST_PAGE_25, 0, 1));                 // Clear EEE bit (bit 0)
            } else {
                VTSS_RC(vtss_phy_page_ext2(vtss_state, port_no));
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EEE_CONTROL, 0x0, VTSS_F_PHY_EEE_CONTROL_ENABLE_10BASE_TE));  // Disable EEE (EEE control, Adress 17E2, bit 15)
                VTSS_RC(vtss_phy_mmd_reg_wr_masked(vtss_state, port_no, 7, 60, 0, 0x0006));    // Disable advertisement
            }

            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));   // Go back to standard page.
            VTSS_N("restart auto neg - Needed for disable/enable EEE advertisement, port = %d", port_no);

            // Only re-start auto-neg if in auto neg mode (Primary due to Bugzilla#7343,
            // which is also the reason to setting the AUTO_NED_ENA bit)
            if (ps->setup.mode == VTSS_PHY_MODE_ANEG) {
                if (!vtss_state->sync_calling_private || ps->warm_start_reg_changed) {// Only reset under warm start if something has changed.
                    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MODE_CONTROL,
                                                    VTSS_F_PHY_MODE_CONTROL_AUTO_NEG_ENA | VTSS_F_PHY_MODE_CONTROL_RESTART_AUTO_NEG,
                                                    VTSS_F_PHY_MODE_CONTROL_AUTO_NEG_ENA | VTSS_F_PHY_MODE_CONTROL_RESTART_AUTO_NEG)); // Restart Auto-negotiation
                } else {
                    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MODE_CONTROL,
                                                    VTSS_F_PHY_MODE_CONTROL_AUTO_NEG_ENA,
                                                    VTSS_F_PHY_MODE_CONTROL_AUTO_NEG_ENA)); // During warm start only enable Auto-negotiation

                }
                VTSS_D("Autoneg, port:%d", port_no);
            }
        }
        break;

    default:
        VTSS_D("EEE not support for family:%s, port = %d",
               vtss_phy_family2txt(ps->family), port_no);
        rc = VTSS_RC_OK;
    }

    return rc;
}

// Function for returning link partners EEE advertisement.
// In: port_no - The PHY port number starting from 0.
// Out: Advertisement bit mask.
//      Bit 0 = Link partner advertises 100BASE-T capability.
//      Bit 1 = Link partner advertises 1000BASE-T capability.
static vtss_rc vtss_phy_eee_link_partner_advertisements_get_private(
    vtss_state_t *vtss_state,
    const vtss_port_no_t        port_no,
    u8  *advertisements)
{

    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_rc rc = VTSS_RC_OK;
    u16 reg_val;

    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        VTSS_RC(vtss_phy_mmd_rd(vtss_state, port_no, 7, 61, &reg_val)); // Get the link partner advertisement.
        *advertisements = reg_val >> 1; // Bit 0 is reserved. See data sheet.
        break;

    default:
        *advertisements = 0;
        VTSS_D("EEE not support for family:%s",
               vtss_phy_family2txt(ps->family));
        rc = VTSS_RC_OK;
    }
    return rc;

}

// Function for returning if phy is currently powered save mode due to EEE
// In: port_no - The PHY port number starting from 0.
// Out: rx_in_power_save_state - TRUE if rx part of the PHY is in power save mode
//      tx_in_power_save_state - TRUE if tx part of the PHY is in power save mode
static vtss_rc vtss_phy_eee_power_save_state_get_private(
    vtss_state_t *vtss_state,
    const vtss_port_no_t        port_no,
    BOOL  *rx_in_power_save_state,
    BOOL  *tx_in_power_save_state)
{

    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_rc rc = VTSS_RC_OK;
    u16 reg_val;

    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        VTSS_RC(vtss_phy_mmd_rd(vtss_state, port_no, 3, 1, &reg_val)); // Get PCS status

        // Bit 8 is RX LPI. See data sheet.
        if (reg_val & 0x0100) {
            *rx_in_power_save_state = TRUE;
        } else {
            *rx_in_power_save_state = FALSE;
        }

        // Bit 9 is TX LPI. See data sheet.
        if (reg_val & 0x0200) {
            *tx_in_power_save_state = TRUE;
        } else {
            *tx_in_power_save_state = FALSE;
        }

        VTSS_N("PCS Status:0x%X, port:%d, rx_in_power_save_state:%d, tx_in_power_save_state:%d",
               reg_val, port_no, *rx_in_power_save_state, *tx_in_power_save_state);
        break;

    default:
        *rx_in_power_save_state = FALSE;
        *tx_in_power_save_state = FALSE;
        VTSS_D("EEE not support for family:%s",
               vtss_phy_family2txt(ps->family));
        rc = VTSS_RC_OK;
    }
    return rc;
}

#endif // end defined(VTSS_FEATURE_EEE)



/************************************************************************/
/* COBRA family functions                                               */
/************************************************************************/

// Setup MAC and Media interface
//
// In : port_no - port starting from 0.
//      conf    - Configuration for the port
static vtss_rc vtss_phy_mac_media_if_cobra_setup(vtss_state_t *vtss_state,
                                                 const vtss_port_no_t port_no, const vtss_phy_reset_conf_t *const conf)
{
    u16 reg = 0;
    u16 mac_media_mode_sel_15_12 = 0x0; // Mac/Media interface mode selct bits 15:12 - Datasheet table 36
    u16 mac_media_mode_sel_2_1 = 0x0;   // Mac/Media interface mode selct bits 2:1 - Datasheet table 36


    switch (conf->mac_if) {

    case VTSS_PORT_INTERFACE_SGMII:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0xB;
            mac_media_mode_sel_2_1   = 0x0;
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
            mac_media_mode_sel_15_12 = 0x9;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;


    case VTSS_PORT_INTERFACE_RGMII:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0x1;
            mac_media_mode_sel_2_1   = 0x2;
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_FI_100FX:
            mac_media_mode_sel_15_12 = 0x1;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
        case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
            mac_media_mode_sel_15_12 = 0x0;
            mac_media_mode_sel_2_1   = 0x2;
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
            mac_media_mode_sel_15_12 = 0x0;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;

    case VTSS_PORT_INTERFACE_RTBI:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0x4;
            mac_media_mode_sel_2_1   = 0x0;
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_FI_100FX:
            mac_media_mode_sel_15_12 = 0x5;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;


    case VTSS_PORT_INTERFACE_TBI:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0x6;
            mac_media_mode_sel_2_1   = 0x0;
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_FI_100FX:
            mac_media_mode_sel_15_12 = 0x7;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;



    case VTSS_PORT_INTERFACE_GMII:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0x3;
            mac_media_mode_sel_2_1   = 0x2;
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_FI_100FX:
            mac_media_mode_sel_15_12 = 0x3;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
        case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
            mac_media_mode_sel_15_12 = 0x2;
            mac_media_mode_sel_2_1   = 0x2;
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
            mac_media_mode_sel_15_12 = 0x2;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;

    case VTSS_PORT_INTERFACE_SERDES:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0xF;
            mac_media_mode_sel_2_1   = 0x0;
            break;
        case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
        case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
            mac_media_mode_sel_15_12 = 0x9;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;
    default:
        goto conf_error;
    }


    // Do the register (address 0x23)  write
    reg = 0x0a20; // Setup skew and RX idle clock, datasheet table 36.
    reg |= mac_media_mode_sel_15_12 << 12;
    reg |= mac_media_mode_sel_2_1 << 1;

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL, reg));
    return VTSS_RC_OK;


// Configuration error.
conf_error:
    VTSS_E("Cobra family, Port %u, Media interface:%s not supported for mac if:%s",
           port_no, vtss_phy_media_if2txt(conf->media_if),
           vtss_port_if_txt(conf->mac_if));
    return VTSS_RC_ERROR;
}

/* Init Scripts for VSC8211/VSC8221 aka Cobra */
static vtss_rc vtss_phy_init_seq_cobra(vtss_state_t *vtss_state,
                                       vtss_phy_port_state_t *ps,
                                       vtss_port_no_t        port_no)
{
    /* Enable token-ring register access for entire init script */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));

    /* BZ 1769 */
    /* Performance robustness improvement on 50m loops */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0xafa4));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_2, 0x0000, 0x0000));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_1, 0x4000, 0xf000));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0x8fa4));

    /* BZ 2337 */
    /* Inter-operability with Intel 82547E1 L322SQ96 */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0xafae));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_2, 0x0000, 0x0000));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_1, 0x0600, 0x7f80));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0x8fae));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, 0x0040, 0x0040));
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));

    /* BZ 1800 */
    /* 100BT Jumbo packet mode script */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0xb786));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_2, 0x0000, 0x0000));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_1, 0x0e20, 0x1fe0));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0x9786));

    /* BZ 2331 */
    /* Insufficient RGMII drive strength on long traces */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_3, 0x2000, 0x2000));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_3, 0x1080, 0x1ff8));
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));

    /* BZ 2232 */
    /* DSP Optimization script */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0xaf8a));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_2, 0x0000, 0x0000));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_1, 0x0008, 0x000c));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0x8f8a));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0xaf86));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_2, 0x0008, 0x000c));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_1, 0x0000, 0x0000));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0x8f86));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0xaf82));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_2, 0x0000, 0x0000));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_1, 0x0100, 0x0180));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0x8f82));

    /* New bugzilla 11377 */
    /* Forced-10BASE-T PHY falsely linking up with forced-100BASE-TX link partner */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0xa7f4));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_2, 0x0000, 0x0000));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_1, 0x0002, 0x0006));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0x87f4));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0xa7f8));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_2, 0x0000, 0x0000));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_1, 0x0800, 0x0800));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0x87f8));

    /* Restore normal clock gating and return to main page after init script */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    /* Enable Smart Pre-emphasis */
    VTSS_RC(vtss_phy_enab_smrt_premphasis(vtss_state, port_no));

    return VTSS_RC_OK;
}

/************************************************************************/
/* Atom family functions                                                */
/************************************************************************/
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
// Function that returns if the conf id configured as fiber port
// Return - TRUE - conf is configured as fiber port else return FALSE
static BOOL is_fiber_port(const vtss_phy_reset_conf_t *const conf)
{
    switch (conf->media_if) {
    case VTSS_PHY_MEDIA_IF_FI_100FX:
    case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
    case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
    case VTSS_PHY_MEDIA_IF_FI_1000BX:
    case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
    case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
    case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
        return TRUE;
    default:
        return FALSE;
    }
}


// function cfg_ib(port_no, ib_cterm_ena, ib_eq_mode);
//   set the ib_cterm_ena and ib_eq_mode bits to the input values (0 or 1)
//   ib_cterm_ena => cfg_vec[91] => cfg_buf[11].3
//   ib_eq_mode    => cfg_vec[90] => cfg_buf[11].2
//
//   ib_rst       => cfg_vec[86] => cfg_buf[10].6
//
static vtss_rc vtss_phy_atom12_cfg_ib_cterm_ena_private(vtss_state_t *vtss_state, vtss_port_no_t port_no, u8 ib_cterm_ena, u8 ib_eq_mode)
{
    u8 tmp, tmp1;
    u16 reg18g = 0;
    vtss_rc rc = VTSS_RC_OK;

    VTSS_I("ib_eq_mode:%d, ib_cterm_ena:%d", ib_eq_mode, ib_cterm_ena);

    // fetch MCB data
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8113));     // Read MCB for 6G macro 1 into PRAM
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // Assert ib_rst
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0xd7d9));     // set mem_addr to cfg_buf[10] (0x47d9)
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8007));     // read the value of cfg_buf[10] w/o post-incr.
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    // get bits 11:4 from return value and OR in ib_rst (bit 6)
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
    tmp1 = (u8)(((reg18g >> 4) & 0xff) | 0x40);

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9006 | (tmp1 << 4)));    // write the value back to cfg_buf[10] w/ post-incr.
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // update ib_cterm_ena and ib_eq_mode
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8007));     // read the value of cfg_buf[11] w/o post-incr.
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    // get bits 11:4 from return value and mask off bits 2 & 3
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
    tmp = (u8)((reg18g >> 4) & 0xf3);
    // OR in ib_cterm_ena shifted by 3 bits
    tmp |= ((ib_cterm_ena & 1) << 3);
    // OR in ib_eq_mode shifted by 2 bits
    tmp |= ((ib_eq_mode & 1) << 2);

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8006 | (tmp << 4)));    // write the value back to cfg_buf[11] w/o post-incr.
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // Update MCB w/ ib_rst asserted
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0xd7ce));     // set mem_addr to 0x47ce (addr_vec)
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x80e6));     // set addr_vec for macros 1-3
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9cc0));     // Write MCB for 6G macros 1-3 from PRAM
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // Clear ib_rst
    // mask off ib_rst (bit 6)
    tmp1 &= 0xbf;

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0xd7d9));     // set mem_addr to cfg_buf[10] (0x47d9)
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8006 | (tmp1 << 4)));    // write the value back to cfg_buf[10] w/o post-incr.
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // Update MCB w/ ib_rst cleared
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9cc0));     // Write MCB for 6G macros 1-3 from PRAM
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    return rc;
}

// function cfg_ib(port_no, ib_cterm_ena, ib_eq_mode);
//   set the ib_cterm_ena and ib_eq_mode bits to the input values (0 or 1)
//   ib_cterm_ena => cfg_vec[91] => cfg_buf[11].3
//   ib_eq_mode    => cfg_vec[90] => cfg_buf[11].2
//
//   ib_rst       => cfg_vec[86] => cfg_buf[10].6
//
static vtss_rc vtss_phy_tesla_cfg_ib_cterm_ena_private(vtss_state_t *vtss_state, vtss_port_no_t port_no, u8 ib_cterm_ena, u8 ib_eq_mode)
{
    u8 tmp, tmp1;
    u16 reg18g = 0;
    vtss_rc rc = VTSS_RC_OK;

    VTSS_I("ib_eq_mode:%d, ib_cterm_ena:%d", ib_eq_mode, ib_cterm_ena);

    // fetch MCB data
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8013));     // Read MCB for 6G macro 0 into PRAM
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // Assert ib_rst
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0xd7d1));     // set mem_addr to cfg_buf[10] (0x47d1)
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8007));     // read the value of cfg_buf[10] w/o post-incr.
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    // get bits 11:4 from return value and OR in ib_rst (bit 6)
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
    tmp1 = (u8)(((reg18g >> 4) & 0xff) | 0x40);

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9006 | (tmp1 << 4)));    // write the value back to cfg_buf[10] w/ post-incr.
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // update ib_cterm_ena and ib_eq_mode
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8007));     // read the value of cfg_buf[11] w/o post-incr.
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    // get bits 11:4 from return value and mask off bits 2 & 3
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
    tmp = (u8)((reg18g >> 4) & 0xf3);
    // OR in ib_cterm_ena shifted by 3 bits
    tmp |= ((ib_cterm_ena & 1) << 3);
    // OR in ib_eq_mode shifted by 2 bits
    tmp |= ((ib_eq_mode & 1) << 2);

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8006 | (tmp << 4)));    // write the value back to cfg_buf[11] w/o post-incr.
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // Update MCB w/ ib_rst asserted
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9c40));     // Write MCB for 6G macro 0 from PRAM
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // Clear ib_rst
    // mask off ib_rst (bit 6)
    tmp1 &= 0xbf;

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0xd7d1));     // set mem_addr to cfg_buf[10] (0x47d1)
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8006 | (tmp1 << 4)));    // write the value back to cfg_buf[10] w/o post-incr.
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // Update MCB w/ ib_rst cleared
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9c40));     // Write MCB for 6G macro 0 from PRAM
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    return rc;
}

// Function call the ib_cterm configuration for the current chipset
static vtss_rc vtss_phy_cfg_ib_cterm_ena_private(vtss_state_t *vtss_state, vtss_port_no_t port_no, u8 ib_cterm_ena, u8 ib_eq_mode)
{
    switch (vtss_state->phy_state[port_no].family) {
    case VTSS_PHY_FAMILY_ATOM:
        VTSS_RC(vtss_phy_atom12_cfg_ib_cterm_ena_private(vtss_state, port_no, ib_cterm_ena, ib_eq_mode));
        break;

    case VTSS_PHY_FAMILY_TESLA:
        VTSS_RC(vtss_phy_tesla_cfg_ib_cterm_ena_private(vtss_state, port_no, ib_cterm_ena, ib_eq_mode));
        break;

    default :
        VTSS_I("No cterm configuration for %s family", vtss_phy_family2txt(vtss_state->phy_state[port_no].family));
    }
    return VTSS_RC_OK;
}

// Function for getting the PHY settings set by the micro patch.
// In - port_no - Phy port number.
// Out - cfg_buf  - Array containing the configuration setting
//       stat_buf - Array containing the status
static vtss_rc vtss_phy_atom12_patch_setttings_get_private(vtss_state_t *vtss_state,
                                                           vtss_port_no_t port_no, u8 *mcb_bus, u8 *cfg_buf, u8 *stat_buf)
{
    u16 idx;
    u16 reg18g;
    u8 slave_num = 0;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_phy_reset_conf_t *conf = &ps->reset;


    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM :


        // assume MCB bus is passed in as mcb_bus
        //    it is passed to 8051 in bits 7:4
        //    0: SerDes 1G
        //    1: SerDes 6G
        //    2: LCPLL/RComp



        //    it is passed to 8051 in bits 12:8
        //    for 1G the range is 0-7 (see port to MCB mappings in TN1085)
        //    for GG the range is 0-3 (see port to MCB mappings in TN1085)
        //    for LCPLL/RComp (only one) the value is always 0
        if (*mcb_bus == 2) {
            slave_num = 0;
        } else if (is_fiber_port(conf)) {
            // for fiber media port mappings:
            *mcb_bus = 0;  // only 1G macros used for fiber media

            // 6G macro 0 not used in QSGMII mode
            switch (port_no % ps->type.port_cnt) {
            case 8  :    // 1G macro 7
                slave_num = 7;
                break;
            case 9  :    // 1G macro 6
                slave_num = 6;
                break;
            case 10 :    // 1G macro 5
                slave_num = 5;
                break;
            case 11 :    // 1G macro 4
                slave_num = 4;
                break;
            default :
                VTSS_E("Invalid port number:%d", port_no);
            }
        } else if (conf->mac_if ==  VTSS_PORT_INTERFACE_QSGMII) {  // QSGMII mapping (these are one macro per quad)
            *mcb_bus = 1;  // only 6G macros used for QSGMII MACs
            switch (port_no % ps->type.port_cnt) {
            // 6G macro 0 not used in QSGMII mode
            case 0 :    // 6G macro 1
            case 1 :
            case 2 :
            case 3 :
                slave_num = 1;
                break;
            case 4 :    // 6G macro 2
            case 5 :
            case 6 :
            case 7 :
                slave_num = 2;
                break;
            case 8  :    // 6G macro 3
            case 9  :
            case 10 :
            case 11 :
                slave_num = 3;
                break;
            default :
                VTSS_E("Invalid port:%d", port_no);;
            }
        } else { // SGMII
            switch (port_no % ps->type.port_cnt) {
            // all 6G and 1G macros used for MACs in this mode
            case 0 :    // 6G macro 0
                *mcb_bus = 1;
                slave_num = 0;
                break;
            case 1 :    // 1G macro 0
                *mcb_bus = 0;
                slave_num = 0;
                break;
            case 2 :    // 1G macro 1
                *mcb_bus = 0;
                slave_num = 1;
                break;
            case 3 :    // 6G macro 1
                *mcb_bus = 1;
                slave_num = 1;
                break;
            case 4 :    // 1G macro 2
                *mcb_bus = 0;
                slave_num = 2;
                break;
            case 5 :    // 1G macro 3
                *mcb_bus = 0;
                slave_num = 3;
                break;
            case 6 :    // 6G macro 2
                *mcb_bus = 1;
                slave_num = 2;
                break;
            case 7 :    // 1G macro 4
                *mcb_bus = 0;
                slave_num = 4;
                break;
            case 8  :    // 1G macro 5
                *mcb_bus = 0;
                slave_num = 5;
                break;
            case 9  :    // 6G macro 3
                *mcb_bus = 1;
                slave_num = 3;
                break;
            case 10 :    // 1G macro 6
                *mcb_bus = 0;
                slave_num = 6;
                break;
            case 11 :    // 1G macro 7
                *mcb_bus = 0;
                slave_num = 7;
                break;
            default :
                VTSS_E("Invalid port:%d", port_no);  // some sort of error handling should go here
            }
        }
        break;
    default:
        VTSS_E("Patch setting get not supported for family:%s", vtss_phy_family2txt(ps->family));
        return VTSS_RC_ERROR;
    }



    // fetch MCB data
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8003 | (slave_num << 8) | (*mcb_bus << 4)));     // Read MCB macro into PRAM
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0xd7cf));     // set mem_addr to cfg_buf (0x47cf)
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // read out cfg_buf
    for (idx = 0; idx < MAX_CFG_BUF_SIZE; idx++) { // MAX_CFG_BUF_SIZE is 36
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9007));     // read the value of cfg_buf[idx] w/ post-incr.
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
        // get bits 11:4 from return value
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
        cfg_buf[idx] = (u8)((reg18g >> 4) & 0xff);
    }

    // read out stat_buf (starts where cfg_buf leaves off)
    for (idx = 0; idx < MAX_STAT_BUF_SIZE; idx++) { // MAX_STAT_BUF_SIZE is 6
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9007));     // read the value of stat_buf[idx] w/ post-incr.
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
        // get bits 11:4 from return value
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
        stat_buf[idx] = (u8)((reg18g >> 4) & 0xff);
    }
    return VTSS_RC_OK;
}

// Function for getting the PHY settings set by the micro patch.
// In - port_no - Phy port number.
// Out - cfg_buf  - Array containing the configuration setting
//       stat_buf - Array containing the status
static vtss_rc vtss_phy_tesla_patch_setttings_get_private(vtss_state_t *vtss_state,
                                                          vtss_port_no_t port_no, u8 *mcb_bus, u8 *cfg_buf, u8 *stat_buf)
{
    u16 idx;
    u16 reg18g;
    u8 slave_num = 0;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_phy_reset_conf_t *conf = &ps->reset;


    switch (ps->family) {
    case VTSS_PHY_FAMILY_TESLA :

        // assume MCB bus is passed in as mcb_bus
        //    it is passed to 8051 in bits 7:4
        //    0: SerDes 1G
        //    1: SerDes 6G
        //    2: LCPLL/RComp

        //    it is passed to 8051 in bits 12:8
        //    for 1G the range is 0-7 (see port to MCB mappings in TN1121)
        //    for GG the range is 0-3 (see port to MCB mappings in TN1121)
        //    for LCPLL/RComp (only one) the value is always 0
        if (*mcb_bus == 2) {
            slave_num = 0;
        } else if (is_fiber_port(conf)) {
            // for fiber media port mappings:
            *mcb_bus = 0;  // only 1G macros used for fiber media

            // 6G macro 0 not used in QSGMII mode
            switch (port_no % ps->type.port_cnt) {
            case 0  :    // 1G macro 0
                slave_num = 0;
                break;
            case 1  :    // 1G macro 2
                slave_num = 2;
                break;
            case 2 :     // 1G macro 4
                slave_num = 4;
                break;
            case 3 :     // 1G macro 6
                slave_num = 6;
                break;
            default :
                VTSS_E("Invalid port number:%d", port_no);
            }
        } else if (conf->mac_if ==  VTSS_PORT_INTERFACE_QSGMII) {  // QSGMII mapping (these are one macro per quad)
            *mcb_bus = 1;  // only 6G macros used for QSGMII MACs
            switch (port_no % ps->type.port_cnt) {
            // 6G macro 0 not used in QSGMII mode
            case 0 :    // 6G macro 0
            case 1 :
            case 2 :
            case 3 :
                slave_num = 0;
                break;
            default :
                VTSS_E("Invalid port:%d", port_no);;
            }
        } else { // SGMII
            switch (port_no % ps->type.port_cnt) {
            // all 6G and 1G macros used for MACs in this mode
            case 0 :    // 6G macro 0
                *mcb_bus = 1;
                slave_num = 0;
                break;
            case 1 :    // 1G macro 1
                *mcb_bus = 0;
                slave_num = 1;
                break;
            case 2 :    // 1G macro 3
                *mcb_bus = 0;
                slave_num = 3;
                break;
            case 3 :    // 1G macro 5
                *mcb_bus = 0;
                slave_num = 5;
                break;
            default :
                VTSS_E("Invalid port:%d", port_no);  // some sort of error handling should go here
            }
        }
        break;
    default:
        VTSS_E("Patch setting get not supported for family:%s", vtss_phy_family2txt(ps->family));
        return VTSS_RC_ERROR;
    }



    // fetch MCB data
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8003 | (slave_num << 8) | (*mcb_bus << 4)));     // Read MCB macro into PRAM
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0xd7c7));     // set mem_addr to cfg_buf (0x47cf for Atom12)(0x47c7 for Tesla)
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    // read out cfg_buf
    for (idx = 0; idx < MAX_CFG_BUF_SIZE + 2; idx++) { // MAX_CFG_BUF_SIZE is 36 for Atom12 and 38 for Tesla
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9007));     // read the value of cfg_buf[idx] w/ post-incr.
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
        // get bits 11:4 from return value
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
        cfg_buf[idx] = (u8)((reg18g >> 4) & 0xff);
    }

    // read out stat_buf (starts where cfg_buf leaves off)
    for (idx = 0; idx < MAX_STAT_BUF_SIZE; idx++) { // MAX_STAT_BUF_SIZE is 6
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9007));     // read the value of stat_buf[idx] w/ post-incr.
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));     // Switch back to micro/GPIO register-page
        // get bits 11:4 from return value
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
        stat_buf[idx] = (u8)((reg18g >> 4) & 0xff);
    }
    return VTSS_RC_OK;
}

// Function call the patch setting get configuration for the current chipset
static vtss_rc vtss_phy_patch_setttings_get_private(vtss_state_t *vtss_state,
                                                    vtss_port_no_t port_no, u8 *mcb_bus, u8 *cfg_buf, u8 *stat_buf)
{

    switch (vtss_state->phy_state[port_no].family) {
    case VTSS_PHY_FAMILY_ATOM:
        VTSS_RC(vtss_phy_atom12_patch_setttings_get_private(vtss_state, port_no, mcb_bus, cfg_buf, stat_buf));
        break;

    case VTSS_PHY_FAMILY_TESLA:
        VTSS_RC(vtss_phy_tesla_patch_setttings_get_private(vtss_state, port_no, mcb_bus, cfg_buf, stat_buf));
        break;

    default :
        VTSS_I("No cterm configuration for %s family", vtss_phy_family2txt(vtss_state->phy_state[port_no].family));
    }
    return VTSS_RC_OK;
}

// Function for finding a specific bit(s) in the phy patch configuration/status array.
// IN : msb    - MSB bit in array.
//      lsb    - LSB bit in array - If only one bit the LSB shall be set to -1
//      vec    - Pointer the setting/status patch array
static u8 patch_array_value_decode(i16 msb, i16 lsb, u8 *vec)
{
    u16 bit_idx;
    u8 value = 0;

    if (lsb > 0) {
        for (bit_idx = lsb; bit_idx <= msb; bit_idx++) {
            value += VTSS_BF_GET(vec, bit_idx) << (bit_idx - lsb);
        }
    } else {
        value += VTSS_BF_GET(vec, msb);
    }
    return value;
}

static vtss_rc vtss_phy_warmstart_chk_micro_patch_mac_mode(vtss_state_t *vtss_state, vtss_port_no_t port_no, const vtss_phy_reset_conf_t *const conf)
{
    u8 mcb_bus = 0;
    u8 cfg_buf[MAX_CFG_BUF_SIZE];
    u8 stat_buf[MAX_STAT_BUF_SIZE];
    u8 hrate;
    u8 grate;
    u8 if_mode;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];

    if (vtss_state->sync_calling_private) {
        VTSS_RC(vtss_phy_patch_setttings_get_private(vtss_state, port_no, &mcb_bus, &cfg_buf[0], &stat_buf[0]));
        hrate = patch_array_value_decode(19, -1, &cfg_buf[0]);
        grate = patch_array_value_decode(18, -1, &cfg_buf[0]);
        if_mode = patch_array_value_decode(17, 16, &cfg_buf[0]);

        vtss_port_interface_t micro_patch_mac_mode = VTSS_PORT_INTERFACE_SGMII;
        // From James : To tell if the 6G macro has been configured for QSGMII or SGMII operation (should correspond with 19G.14):
        // QSGMII    SGMII
        // ------    -----
        // hrate     0      0
        // qrate     0      1
        // if_mode   3      1
        if ((if_mode == 0x3) && (grate == 0) && (hrate == 0)) {
            micro_patch_mac_mode = VTSS_PORT_INTERFACE_QSGMII;
        }

        if (conf->mac_if != micro_patch_mac_mode) {
            ps->warm_start_reg_changed = TRUE; // Signaling the a register for this port port has changed.
            VTSS_E("Warmstart micropatch mode not in sync micro_patch_mode:%d, configuration mode:%d", micro_patch_mac_mode, conf->mac_if);
        }
    }
    return VTSS_RC_OK;
}

// Function to set the ob_post0 parameter after configuring the 6G macro for QSGMII for ATOM12
// In : port_no - Port to configure
//      val     - Value to configure
static vtss_rc vtss_phy_cfg_ob_post0_private(vtss_state_t *vtss_state, vtss_port_no_t port_no, u8 val)
{
    u8 tmp;
    u16 reg18g = 0;
    vtss_rc rc = VTSS_RC_OK;

    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8113));  // Read MCB for 6G macro 1 into PRAM
        (void)  vtss_phy_wait_for_micro_complete(vtss_state, port_no);

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0xd7d8));  // set mem_addr to cfg_buf[9] (0x47d8)
        (void)(vtss_phy_wait_for_micro_complete(vtss_state, port_no));


        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8007));  // read the value of cfg_buf[9] w/o post-incr.
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));


        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        // get bits 11:4 from return value and mask off upper 3 bits
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
        tmp = (u8)((reg18g >> 4) & 0x1f);
        // OR in lower 3 bits of val shifted aprropriately
        tmp |= ((val & 7) << 5);

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        // write the value back to cfg_buf[9] w/ post-incr.
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9006 | ((u16)tmp << 4)));
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8007));  // read the value of cfg_buf[10] w/o post-incr.
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        // get bits 11:4 from return value and mask off lower 3 bits
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
        tmp = (u8)((reg18g >> 4) & 0xf8);
        // OR in upper 3 bits of val shifted aprropriately
        tmp |= ((val >> 3) & 7);

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        // write the value back to cfg_buf[10] w/o post-incr.
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8006 | ((u16)tmp << 4)));
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0xd7ce));  // set mem_addr to 0x47ce (addr_vec)
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x80e6));  // set addr_vec for macros 1-3
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9cc0));  // Write MCB for 6G macros 1-3 from PRAM
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
        break;
    case VTSS_PHY_FAMILY_TESLA:
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8013));  // Read MCB for 6G macro 0 into PRAM
        (void)  vtss_phy_wait_for_micro_complete(vtss_state, port_no);

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0xd7d0));  // set mem_addr to cfg_buf[9] (0x47d0)
        (void)(vtss_phy_wait_for_micro_complete(vtss_state, port_no));


        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8007));  // read the value of cfg_buf[9] w/o post-incr.
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));


        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        // get bits 11:4 from return value and mask off upper 3 bits
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
        tmp = (u8)((reg18g >> 4) & 0x1f);
        // OR in lower 3 bits of val shifted aprropriately
        tmp |= ((val & 7) << 5);

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        // write the value back to cfg_buf[9] w/ post-incr.
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9006 | ((u16)tmp << 4)));
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8007));  // read the value of cfg_buf[10] w/o post-incr.
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        // get bits 11:4 from return value and mask off lower 3 bits
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));
        tmp = (u8)((reg18g >> 4) & 0xf8);
        // OR in upper 3 bits of val shifted aprropriately
        tmp |= ((val >> 3) & 7);

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        // write the value back to cfg_buf[10] w/o post-incr.
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8006 | ((u16)tmp << 4)));
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9c40));  // Write MCB for 6G macro 0 from PRAM
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
        break;
    default:
        rc = VTSS_RC_ERROR;
        VTSS_I("vtss_phy_cfg_ob_post0 not supported at port %u, Chip family:%s", port_no + 1, vtss_phy_family2txt(vtss_state->phy_state[port_no].family));
    }

    return rc;
}
#endif

// Function that check if mac interface for a specific port  has changed since the last time this function was called.
//  Needed for Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
// IN : port_no - The port in question.
//      current_mac_if - The current mac interface.

// OK - That multiple function call access this function at the same time, since the mac interface is not changed once it is configured for a specific port.
static BOOL mac_if_changed(vtss_state_t *vtss_state,
                           vtss_port_no_t port_no, vtss_port_interface_t current_mac_if)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    BOOL                  mac_if_chged;

    // Find out if interface has changed.
    if (ps->mac_if_old == current_mac_if) {
        mac_if_chged = FALSE;
    } else {
        mac_if_chged = TRUE;
    }

    VTSS_N("port:%u, mac_if_changed:%d, mac_if_old:%d, current_mac_if:%d",
           port_no, mac_if_chged, ps->mac_if_old, current_mac_if);

    ps->mac_if_old = current_mac_if; // Remember the last mac interface configured for this port.

    return mac_if_chged;
}

static vtss_rc vtss_phy_mac_media_if_atom_setup(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no, const vtss_phy_reset_conf_t *const conf)
{
    u16 reg23 = 0;
    u16 reg19g = 0;
    BOOL fi_1000_mode = FALSE; // Use to signal if 1000base-x mode is selected
    BOOL fi_100_mode = FALSE; // Use to signal if 100base-fx mode is selected

    switch (conf->mac_if) {
    case VTSS_PORT_INTERFACE_SGMII:
        reg19g = 1 << 14; // SGMII to  CAT5 mode
        reg23  = 0 << 12; /* SGMII   */
        break;
    case VTSS_PORT_INTERFACE_SERDES:
        reg19g = 1 << 14; // SGMII to  CAT5 mode
        reg23  = 1 << 12; //  1000 BASE-X
        break;
    case VTSS_PORT_INTERFACE_QSGMII:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_FI_100FX:
        case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
        case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
        case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
        case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
            vtss_state->phy_inst_state.at_least_one_fiber_port = TRUE;
            break;
        default:
            // Copper port, but don't touch at_least_one_fiber_port.
            break;
        }

        //If some ports are copper only, you would not put these ports in an AMS or Fiber mode.  If you always set 19G.14:15 to 10, then you will enable the HSIO to power up the fiber media ports, so if we don't see any fiber ports at all, we keep the HSIO powered down in order to reduce the power consumption.
        if (vtss_state->phy_inst_state.at_least_one_fiber_port) {
            reg19g = 2 << 14; // QSGMII to CAT5 & Fiber mode
        } else {
            reg19g = 0 << 14; // QSGMII to CAT5 mode
        }
        break;
    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        reg23 |= 0; //  Shouldn't matter since there is no "interface" for this port
        break;

    default:
        VTSS_E("port_no %u, Mac interface %d not supported", port_no, conf->mac_if);
        return VTSS_RC_ERROR;
    }

    switch (conf->media_if) {
    case VTSS_PHY_MEDIA_IF_CU:
        reg23 |= 0 << 8;
        break;
    case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
        reg23 |= 1 << 8;
        break;
    case VTSS_PHY_MEDIA_IF_FI_1000BX:
        fi_1000_mode = TRUE;
        reg23 |= 2 << 8;
        break;
    case VTSS_PHY_MEDIA_IF_FI_100FX:
        fi_100_mode = TRUE;
        reg23 |= 3 << 8;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU:
        reg23 |= VTSS_F_PHY_EXTENDED_PHY_CONTROL_AMS_PREFERENCE;
        reg23 |=  (4 << 8);
        break;
    case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
        fi_1000_mode = TRUE;
        reg23 |=  (5 << 8);
        reg23 &= ~VTSS_F_PHY_EXTENDED_PHY_CONTROL_AMS_PREFERENCE;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
        reg23 |= VTSS_F_PHY_EXTENDED_PHY_CONTROL_AMS_PREFERENCE;
        fi_1000_mode = TRUE;
        reg23 |=  (6 << 8);
        break;
    case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
        fi_1000_mode = TRUE;
        reg23 |=  (6 << 8);
        reg23 &= ~VTSS_F_PHY_EXTENDED_PHY_CONTROL_AMS_PREFERENCE;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
        reg23 |= VTSS_F_PHY_EXTENDED_PHY_CONTROL_AMS_PREFERENCE;
        fi_100_mode = TRUE;
        reg23 |= (7 << 8);
        break;
    case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
        fi_100_mode = TRUE;
        reg23 &= ~VTSS_F_PHY_EXTENDED_PHY_CONTROL_AMS_PREFERENCE;
        reg23 |= (7 << 8);
        break;
    default:
        VTSS_E("port_no %u, Media interface %d not supported", port_no, conf->media_if);
        return VTSS_RC_ERR_PHY_MEDIA_IF_NOT_SUPPORTED;
    }

    // Do the register accesses
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MAC_MODE_AND_FAST_LINK, reg19g, 0xC000));

    // Could be a Luton26 which is also using this function, but shouldn't set the MAC interface
    if (vtss_state->phy_state[port_no].family == VTSS_PHY_FAMILY_ATOM ||  vtss_state->phy_state[port_no].family == VTSS_PHY_FAMILY_TESLA) {
        VTSS_RC(atom_patch_suspend(vtss_state, port_no, TRUE)); // Suspend 8051 Patch.
        if (mac_if_changed(vtss_state, port_no, conf->mac_if)) {
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
            VTSS_RC(vtss_phy_warmstart_chk_micro_patch_mac_mode(vtss_state, port_no, conf));
#endif
            if (conf->mac_if ==  VTSS_PORT_INTERFACE_QSGMII) {
                VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

                //  Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x80A0)); // See TN1085

#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)

                // ib_cterm and ob_post0 patches is only designed to run on Atom12 devices where the 8051 controls the MCB (otherwise the dreaded timeout will occur).
                // ib_cterm called to override the ROM default.
                VTSS_RC(vtss_phy_cfg_ib_cterm_ena_private(vtss_state, port_no, vtss_state->init_conf.serdes.ib_cterm_ena, 1));

                // ob post called to override the ROM default of 2.
                VTSS_RC(vtss_phy_cfg_ob_post0_private(vtss_state, port_no, vtss_state->init_conf.serdes.qsgmii.ob_post0));
#endif
            } else {
                VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
                //  Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x80B0));// Seee TN1085
            }

            // Wait for micro to complete MCB command to configure for QSGMII
            VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
        }

        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
        if (fi_100_mode) {
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE , 0x8091 | 0x0100 << (vtss_phy_chip_port(vtss_state, port_no) % 4)));
            VTSS_D("MCB : Setting 100Fx mode, 0x%X", 0x8091 | 0x0100 << (vtss_phy_chip_port(vtss_state, port_no) % 4));
        }

        if (fi_1000_mode) {
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8081 | 0x0100 << (vtss_phy_chip_port(vtss_state, port_no) % 4)));
            VTSS_D("MCB : Setting 1000x mode, 0x%X", 0x8081 | 0x0100 << (vtss_phy_chip_port(vtss_state, port_no) % 4));
        }

        // Wait for micro to complete MCB command
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
        VTSS_RC(atom_patch_suspend(vtss_state, port_no, FALSE)); // Resume 8051 Patch.
    }


    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    VTSS_D("reg23:0x%X, media_if:%s, mac if:%d, port_no:%u, chip_no:%u",
           reg23, vtss_phy_media_if2txt(conf->media_if), conf->mac_if, port_no, vtss_phy_chip_port(vtss_state, port_no) % 12);
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL, reg23));


    // Port must be reset in order to update the media setting for register 23
    VTSS_RC(vtss_phy_soft_reset_port(vtss_state, port_no));
    VTSS_MSLEEP(10);

    // Bugzilla#8714 - LabQual failures in QSGMII operation on FFFF parts
    // The sequence of PHY writes setting register 20E3.2 must be after the soft-reset.
    // This is because comma-bypass is envisioned as a debug bit that
    // should be reset when soft-reset is asserted; however, in this case, when the
    // QSGMII MAC interface is selected, it is desired to always bypass the
    // comma-detection logic to avoid issues on corner silicon.
    if (conf->mac_if == VTSS_PORT_INTERFACE_QSGMII) {
        VTSS_RC(vtss_phy_page_ext3(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_MAC_SERDES_STATUS, 0x0004, 0x0004));
    }

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}

// Function for keeping track of if any port is running VeriPhy.
// In - port_no - Phy port number.
//      set     - True if the running state shall be updated for the port_no
//      running - Set to TRUE if VeriPhy is running for port_no.
BOOL vtss_phy_veriphy_running(vtss_state_t *vtss_state,
                              vtss_port_no_t port_no, BOOL set, u8 running)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_port_no_t        port_idx;

    // Remember if veriphy is running for this port.
    if (set) {
        ps->veriphy_running = (running ? TRUE : FALSE);
    }
    VTSS_N("set =%d, running = %d, veriphy_running[%d] = %u",
           set, running, port_no, ps->veriphy_running);


    // Return TRUE is any port has veriphy running
    for (port_idx = 0; port_idx < VTSS_PORTS; port_idx++) {
        if (vtss_state->phy_state[port_idx].veriphy_running) {
            return TRUE;
        }
    }

    return FALSE;
}

//Function for suspending / resuming the 8051 patch.
//
// In : port_no - Any port within the chip where to supend 8051 patch
//      suspend - True if 8051 patch shall be suspended, else patch is resumed.
// Return : VTSS_RC_OK if patch was suspended else error code.
vtss_rc atom_patch_suspend(vtss_state_t *vtss_state, vtss_port_no_t port_no, BOOL suspend)
{
    u16 word;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];

    switch (vtss_state->phy_state[port_no].family) {
    case VTSS_PHY_FAMILY_TESLA:
        if (ps->type.revision == VTSS_PHY_TESLA_REV_A) {
            // No patch suspending for Rev. A.
            return VTSS_RC_OK;
        }
    // Fall through
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no)); // Change to GPIO page

        VTSS_N("atom_patch_suspend = %d, port = %d", suspend, port_no);




        // We must disable the EEE patch when VeriPHY is running,  When VeriPHY  is running, the patch  needs to be suspended by
        // writing 0x800f to register 18 on the Micro/GPIO page (0x10).  0x800f is chosen as this is an unimplemented micro-command
        // and issuing any micro command when the patch is running, causes the patch to stop.
        // The micro patch must not be enabled until all pending VeriPHY requests have been completed on all ports in the relevant Luton26 or Atom12 chip.
        // When all have been completed, the micro patch should re-enable.  Note that this is necessary only because the patch for EEE consumes
        //the micro continually to service all 12 PHYs in a timely manner and workaround one of the weaknesses in gigabit EEE in Luton26/Atom12.
        if (vtss_phy_veriphy_running(vtss_state, port_no, FALSE, FALSE)) {
            VTSS_N("atom_patch_suspend, port = %d", port_no);
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x800F)); // Suspend the micro patch
            VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no)); // Back to standard page
            return VTSS_RC_OK;
        }

        if (suspend) {
            // Suspending 8051 patch
            if ((vtss_state->phy_state[port_no].family == VTSS_PHY_FAMILY_ATOM ||
                 vtss_state->phy_state[port_no].family == VTSS_PHY_FAMILY_LUTON26) && (ps->type.revision == VTSS_PHY_ATOM_REV_A)) {
                // From JimB
                // You are suspending the micro patch momentarily by writing
                // 0x9014 to GPIO-page register 18 and resuming by writing 0x9004 to
                // GPIO-page register 18..
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9014)); // Suspend vga patch
                VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
            }  else {
                // See comment below.
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &word));
                if (!(word & 0x4000)) {
                    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x800F)); // Suspend 8051 EEE patch
                }
                VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
            }
            VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
        } else {
            // Resuming 8051 patch
            if ((vtss_state->phy_state[port_no].family == VTSS_PHY_FAMILY_ATOM ||
                 vtss_state->phy_state[port_no].family == VTSS_PHY_FAMILY_LUTON26) && (ps->type.revision == VTSS_PHY_ATOM_REV_A)) {
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x9004));
                VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

            }  else {
                // On page 0x10 (Reg31 = 0x10), write register 18 with 0x8009 to turn on the EEE patch.
                // Once this is done all code that might attempt to access page 0x52b5 will fail and likely cause issues.
                // If you need to access page 0x52b5 or run another page 0x10, register 18 function, you must first disable
                // the patch by writing 0x8009 again to register 18.  In response, the error bit (bit 14) will be set,
                // but the micro is now freed from running the EEE patch and you may issue your other micro requests.
                // When the other requests are complete, you will want to rerun the EEE patch by writing 0x8009 to register 18 on page 0x10.
                // The events that are handled by the micro patch occur occasionally, say one event across 12 ports every 30 seconds.
                // As a result, suspending the EEE patch for short durations Is unlikely to result in link drops, but it is possible.
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &word));
                if (word & 0x4000) {
                    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x8009));
                }
                VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
            }
            VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
        }

        break;
    default:
        VTSS_D("No micro patch for this PHY");
    }
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no)); // Change to standard page

    return VTSS_RC_OK;
}

//  Function for pulling the coma mode pin high or low (The coma mode pin MUST be pulled high by an external pull up resistor)
//
// In : port  : Phy port (Any port within the PHY chip to pull down coma mode pin).
//      Low   : True to pull coma mode pin low (disable coma mode - ports are powered up)
// Return :  VTSS_RC_OK is setup was preformed correct else VTSS_RC_ERROR

static vtss_rc vtss_phy_coma_mode_private(vtss_state_t *vtss_state, const vtss_port_no_t port_no, BOOL low)
{
// COMA mode is a new concept for Luton26/Atom12 for the PHYs and when the COMA_MODE pin is asserted, the PHY will
// remain powered down which is the ideal state to configure the PHY and SerDes MAC interfaces.  C
// COMA_MODE is designed to also synchronize LED clocks upon deassertion of COMA_MODE.  T
// The board designer should tie all COMA_MODE pins together. When the pin floats, an internal pull-up asserts the COMA_MDOE pin.
// After all internal Luton26 and external Atom12 PHYs have been configured, the initialization scripts should configure one of the devices to
// drive COMA_MODE pin low by changing COMA_MODE from an input to an output and setting the output to 0.  This will be sensed simultaneously
// at all PHYs and synchronize LED clocks between these PHYs.  Note that the LED blink on reset will occur, if configured, at the time that COMA_MODE goes low.


// The COMA_MODE pin may be configured using micro/GPIO register 14G.[13:12].
// Bit 13 is an active-low output enable which defaults on reset to 1 (e.g. output disabled).
// Bit 12 is the bit to drive the COMA_MODE pin to when the output is enabled.
    VTSS_RC(vtss_phy_page_gpio(vtss_state, (port_no)));
    if (low) {
        VTSS_N("Setting coma pin low");
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_CONTROL_2, 0x0000,
                                   VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_ENABLE | VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_DATA));
    } else {
        VTSS_I("Setting coma pin high");
        // The coma mode pin MUST pulled high by an external pull up resistor, so only the output enable is changed)
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_CONTROL_2, VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_ENABLE,
                                   VTSS_F_PHY_GPIO_CONTROL_2_COMA_MODE_OUTPUT_ENABLE));
    }
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}


static vtss_rc vtss_phy_set_private_atom(vtss_state_t *vtss_state, const vtss_port_no_t port_no, const vtss_phy_mode_t mode)
{
    VTSS_RC(atom_patch_suspend(vtss_state, port_no, TRUE));
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no,  VTSS_PHY_TEST_PAGE_8, 0x8000, 0x8000)); //Ensure RClk125 enabled even in powerdown
    // Clear Cl40AutoCrossover in forced-speed mode, but set it in non-forced modes
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa7fa));  // issue token-ring read request
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, (mode == VTSS_PHY_MODE_FORCED)
                               ? 0x0000 : 0x1000, 0x1000));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87fa));  // issue token-ring write request

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xaf82));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x2, 0xf));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f82));

    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no,  VTSS_PHY_TEST_PAGE_8, 0x0000, 0x8000)); //Restore RClk125 gating
    VTSS_RC(atom_patch_suspend(vtss_state, port_no, FALSE));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));


    // Enable HP Auto-MDIX in forced-mode (by clearing disable bit)
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, 0x0000, VTSS_F_PHY_BYPASS_CONTROL_HP_AUTO_MDIX_AT_FORCE));
    return VTSS_RC_OK;
}


/************************************************************************/
/* Tesla family functions                                               */
/************************************************************************/

// Work-around for issue with Changing speed at one port gives CRC errors at other ports.

// In : Port_no - Port in question
//      Start   - TRUE to start work-around
static vtss_rc vtss_phy_bugzero_48512_workaround(vtss_state_t *vtss_state, vtss_port_no_t port_no, BOOL start)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    if (!vtss_state->sync_calling_private || ps->warm_start_reg_changed) { // Speed is no changed during warm start, so we don't need this for warm start.
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        VTSS_N("vtss_phy_bugzero_48512_workaround:%d", start)
        if (start) {
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, 0x4040));
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, 0x0000));
        }
    }
    return VTSS_RC_OK;
}


static vtss_rc vtss_phy_mac_media_if_tesla_setup(vtss_state_t *vtss_state, const vtss_port_no_t port_no, const vtss_phy_reset_conf_t *const conf)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    u16 micro_cmd_100fx = 0; // Use to signal to micro program if the fiber is 100FX (Bit 4). Default is 1000BASE-x
    u8 media_operating_mode = 0;
    BOOL cu_prefered = FALSE;
    u16 reg_val;
    u16 reg_mask;

    // Setup MAC Configuration
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
    switch (conf->mac_if) {
    case VTSS_PORT_INTERFACE_SGMII:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MAC_MODE_AND_FAST_LINK,
                                        0, VTSS_M_MAC_MODE_AND_FAST_LINK_MAC_IF_MODE_SELECT));
        break;
    case VTSS_PORT_INTERFACE_QSGMII:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MAC_MODE_AND_FAST_LINK,
                                        0x4000, VTSS_M_MAC_MODE_AND_FAST_LINK_MAC_IF_MODE_SELECT));
        break;
    case VTSS_PORT_INTERFACE_RGMII:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MAC_MODE_AND_FAST_LINK,
                                        0x8000, VTSS_M_MAC_MODE_AND_FAST_LINK_MAC_IF_MODE_SELECT));
        break;
    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        break;
    default:
        VTSS_E("port_no %u, Mac interface %d not supported", port_no, conf->mac_if);
        return VTSS_RC_ERROR;
    }

    // Setup media interface
    switch (conf->media_if) {
    case VTSS_PHY_MEDIA_IF_CU:
        media_operating_mode = 0;
        cu_prefered = TRUE;
        break;
    case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
        media_operating_mode = 1;
        cu_prefered = TRUE;
        break;
    case VTSS_PHY_MEDIA_IF_FI_1000BX:
        media_operating_mode = 2;
        cu_prefered = FALSE;
        break;
    case VTSS_PHY_MEDIA_IF_FI_100FX:
        media_operating_mode = 3;
        cu_prefered = FALSE;
        micro_cmd_100fx = 1 << 4;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU:
        media_operating_mode = 5;
        cu_prefered = TRUE;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
        media_operating_mode = 5;
        cu_prefered = FALSE;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
        media_operating_mode = 6;
        cu_prefered = TRUE;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
        media_operating_mode = 6;
        cu_prefered = FALSE;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
        media_operating_mode = 7;
        cu_prefered = TRUE;
        micro_cmd_100fx = 1 << 4;
        break;
    case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
        media_operating_mode = 7;
        cu_prefered = FALSE;
        micro_cmd_100fx = 1 << 4;
        break;
    default:
        VTSS_E("port_no %u, Media interface %d not supported", port_no, conf->media_if);
        return VTSS_RC_ERR_PHY_MEDIA_IF_NOT_SUPPORTED;
    }

    VTSS_RC(vtss_phy_bugzero_48512_workaround(vtss_state, port_no, TRUE));//Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
    if (mac_if_changed(vtss_state, port_no, conf->mac_if)) {
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
        VTSS_RC(vtss_phy_warmstart_chk_micro_patch_mac_mode(vtss_state, port_no, conf));
#endif
        if (conf->mac_if ==  VTSS_PORT_INTERFACE_QSGMII) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED_CHK_MASK(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x80E0, 0xFFFF, 0x0)); // Configure SerDes macros for QSGMII MAC interface (See TN1080)
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED_CHK_MASK(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x80F0, 0xFFFF, 0)); // Configure SerDes macros for 4xSGMII MAC interface (See TN1080)
        }
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        VTSS_RC(vtss_phy_sd6g_patch_private(vtss_state, port_no));
    }
    VTSS_MSLEEP(10);

    VTSS_RC(vtss_phy_bugzero_48512_workaround(vtss_state, port_no, FALSE));//Bugzero#48512, Changing speed at one port gives CRC errors at other ports.

    if (conf->media_if != VTSS_PHY_MEDIA_IF_CU) {
        VTSS_RC(vtss_phy_bugzero_48512_workaround(vtss_state, port_no, TRUE));//Bugzero#48512, Changing speed at one port gives CRC errors at other ports.

        // Setup media in micro program. Bit 8-11 is bit for the corresponding port (See TN1080)
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
        // Should be warmstart checked, but not possible at the moment (Bugzilla#11826)
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x80C1 | (0x0100 << (vtss_phy_chip_port(vtss_state, port_no) % 4)) | micro_cmd_100fx));
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));
        VTSS_MSLEEP(10);

        VTSS_RC(vtss_phy_bugzero_48512_workaround(vtss_state, port_no, FALSE));//Bugzero#48512, Changing speed at one port gives CRC errors at other ports.
    }
    // Setup Media interface
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    reg_val  = VTSS_F_PHY_EXTENDED_PHY_CONTROL_MEDIA_OPERATING_MODE(media_operating_mode) |
               (cu_prefered ? VTSS_F_PHY_EXTENDED_PHY_CONTROL_AMS_PREFERENCE : 0);


    reg_mask  = VTSS_M_PHY_EXTENDED_PHY_CONTROL_MEDIA_OPERATING_MODE | VTSS_F_PHY_EXTENDED_PHY_CONTROL_AMS_PREFERENCE;

    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL, reg_val, reg_mask));

    VTSS_D("media_operating_mode:%d, media_if:%d", media_operating_mode, conf->media_if);
    // Only reset under warm start if something has changed.
    if (!vtss_state->sync_calling_private || ps->warm_start_reg_changed) {

        // Port must be reset in order to update the media operating mode for register 23
        VTSS_RC(vtss_phy_soft_reset_port(vtss_state, port_no));
        VTSS_MSLEEP(10);
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_mac_media_if_elise_setup(vtss_state_t *vtss_state, const vtss_port_no_t port_no, const vtss_phy_reset_conf_t *const conf)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    u16 reg_val;
    u16 reg_mask;

    // Setup MAC Configuration
    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

    switch (conf->mac_if) {
    case VTSS_PORT_INTERFACE_QSGMII:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MAC_MODE_AND_FAST_LINK,
                                        0x4000, VTSS_M_MAC_MODE_AND_FAST_LINK_MAC_IF_MODE_SELECT));
        break;
    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        break;
    default:
        VTSS_E("port_no %u, Mac interface %d not supported", port_no, conf->mac_if);
        return VTSS_RC_ERROR;
    }

    // Setup media interface
    switch (conf->media_if) {
    case VTSS_PHY_MEDIA_IF_CU:
        break;
    default:
        VTSS_E("port_no %u, Media interface %d not supported", port_no, conf->media_if);
        return VTSS_RC_ERR_PHY_MEDIA_IF_NOT_SUPPORTED;
    }

    // Setup Media interface
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    reg_val  = VTSS_F_PHY_EXTENDED_PHY_CONTROL_MEDIA_OPERATING_MODE(0);
    reg_mask = VTSS_M_PHY_EXTENDED_PHY_CONTROL_MEDIA_OPERATING_MODE;
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL, reg_val, reg_mask));

    // Only reset under warm start if something has changed.
    if (!vtss_state->sync_calling_private || ps->warm_start_reg_changed) {
        // Port must be reset in order to update the media operating mode for register 23
        VTSS_RC(vtss_phy_soft_reset_port(vtss_state, port_no));
        VTSS_MSLEEP(10);
    }
    return VTSS_RC_OK;
}



/************************************************************************/
/* Quattro family functions                                               */
/************************************************************************/

static vtss_rc vtss_phy_mac_media_if_quattro_setup (vtss_state_t *vtss_state, const vtss_port_no_t port_no, const vtss_phy_reset_conf_t *const conf)
{
    u16 reg = 0;
    switch (conf->mac_if) {
    case VTSS_PORT_INTERFACE_SGMII:
        /* VSC8234 */
        reg = ((0xB << 12) | (0 << 1)); /* 4-pin SGMII, auto negotiation disabled */
        break;
    case VTSS_PORT_INTERFACE_RGMII:
        /* VSC8224/44 */
        reg = (((conf->media_if == VTSS_PHY_MEDIA_IF_AMS_CU_1000BX ||
                 conf->media_if == VTSS_PHY_MEDIA_IF_AMS_FI_1000BX ? 0 : 1) << 12) |
               (rgmii_clock_skew(conf->rgmii.tx_clk_skew_ps) << 10) |
               (rgmii_clock_skew(conf->rgmii.rx_clk_skew_ps) << 8) |
               ((conf->media_if == VTSS_PHY_MEDIA_IF_CU ||
                 conf->media_if == VTSS_PHY_MEDIA_IF_AMS_CU_1000BX ? 2 : 1) << 1));
        break;
    case VTSS_PORT_INTERFACE_RTBI:
        /* VSC8224/44 */
        reg = (((conf->tbi.aneg_enable ? 4 : 5) << 12) |
               ((conf->tbi.aneg_enable ? 1 : 0) << 1));
        break;
    default:
        VTSS_E("port_no %u, MAC interface %s not supported for Quattro",
               port_no, vtss_port_if_txt(conf->mac_if));
        return VTSS_RC_ERROR;
    }
    reg |= (1 << 5); /* Rx Idle Clock Enable, must be 1 */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL, reg));
    return VTSS_RC_OK;
}

/* Init Scripts for VSC8224/VSC8234/VSC8244 aka Quattro */
static vtss_rc vtss_phy_init_seq_quattro(vtss_state_t *vtss_state,
                                         vtss_phy_port_state_t *ps,
                                         vtss_port_no_t        port_no)
{
    u16 reg = 0;

    /* BZ 1380 - PLL Error Detect Bit Enable */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_2, 0x8000, 0x8000));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    if (ps->type.revision < 3) {
        /* BZ 1671 */
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
        VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0004));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0671));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8fae));
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, 0x0040, 0x0040));

        /* BZ 1746 */
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
        VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x000f));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x492a));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8fa4));
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

        /* BZ 1799 */
        VTSS_RC(vtss_phy_optimize_jumbo(vtss_state, port_no));

        /* BZ 2094 */
        if (ps->reset.mac_if == VTSS_PORT_INTERFACE_RGMII) {
            VTSS_RC(vtss_phy_optimize_rgmii_strength(vtss_state, port_no));
        }
    }

    /* BZ 1776 */
    VTSS_RC(vtss_phy_optimize_receiver_init(vtss_state, port_no));

    /* BZ 2229 */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
    VTSS_RC(vtss_phy_optimize_dsp(vtss_state, port_no));
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    /* BZ 2080 */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xaf82));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg));
    reg = (reg & 0xffef) | 0;           /*- Enable DFE in 100BT */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f82));
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));

    /* Enable signal detect input (active low) if not copper media */
    VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_MODE_CONTROL,
                        ps->reset.media_if == VTSS_PHY_MEDIA_IF_CU ?
                        0x0002 : 0x0001));

    /* Disable down shifting */
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_3, 0x0000, 0x0010));

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    /* Enable Smart Pre-emphasis */
    VTSS_RC(vtss_phy_enab_smrt_premphasis(vtss_state, port_no));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_mac_media_if_spyder_gto_setup (vtss_state_t *vtss_state, const vtss_port_no_t port_no, const vtss_phy_reset_conf_t *const conf, \
                                                       const vtss_phy_type_t phy_type)
{
    u16 reg = 0;

    if (phy_type.part_number == VTSS_PHY_TYPE_8538) {
        switch (conf->mac_if) {
        case VTSS_PORT_INTERFACE_SGMII:
            reg = ((0 << 13) | (0 << 12));  /* SGMII, Auto negotiation disabled */
            break;
        case VTSS_PORT_INTERFACE_SERDES:
            reg = ((0 << 13) | (1 << 12));      /* SerDes 1000-BaseX, Auto negotiation disabled */
            break;

        case VTSS_PORT_INTERFACE_NO_CONNECTION:
            reg |= (0 << 11) | (8 << 12); // Setting to Reserved. Shouldn't matter since there is no "interface" for this port
            break;

        default:
            VTSS_E("port_no %u, MAC interface %s not supported for Spyder",
                   port_no, vtss_port_if_txt(conf->mac_if));
            return VTSS_RC_ERROR;
        }
    }         /* phy_type = VTSS_PHY_TYPE_8538 */
    else if (phy_type.part_number == VTSS_PHY_TYPE_8558) {
        switch (conf->mac_if) {
        case VTSS_PORT_INTERFACE_SGMII:
            reg = ((0 << 13) | (0 << 12));                                                    /* SGMII MAC; MAC Auto Negotiation disabled */
            break;
        case VTSS_PORT_INTERFACE_SERDES:                              /* SerDes MAC; MAC Auto Negotiation disabled */
            reg = ((0 << 13)) | ((1 << 12));
            break;
        case VTSS_PORT_INTERFACE_NO_CONNECTION:
            reg |= (0 << 11) | (8 << 12); // Setting to Reserved. Shouldn't matter since there is no "interface" for this port
            break;

        default:
            VTSS_E("port_no %u, MAC interface %s not supported for Spyder",
                   port_no, vtss_port_if_txt(conf->mac_if));
            return VTSS_RC_ERROR;
        }

        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            reg |= (0 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
            reg |= (1 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
            reg |= (2 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU:
            reg |= (1 << 11) | (5 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
            reg |= (0 << 11) | (5 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
            reg |= (1 << 11) | (6 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
            reg |= (0 << 11) | (6 << 8);
            break;
        default:
            VTSS_E("port_no %u, Media interface not supported for Spyder", port_no);
            return VTSS_RC_ERR_PHY_MEDIA_IF_NOT_SUPPORTED;
        }
    }              /* phy_type == VTSS_PHY_TYPE_8558 */
    else if (phy_type.part_number == VTSS_PHY_TYPE_8658) {
        switch (conf->mac_if) {
        case VTSS_PORT_INTERFACE_SGMII:
            reg = ((0 << 13) | (0 << 12));                                                    /* SGMII MAC; MAC Auto Negotiation disabled */
            switch (conf->media_if) {
            case VTSS_PHY_MEDIA_IF_CU:
                reg |= (0 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
                reg |= (1 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_FI_1000BX:
                reg |= (2 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_FI_100FX:
                reg |= (3 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU:
                reg |= (1 << 11)  |  (5 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
                reg |= (0 << 11) | (5 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
                reg |= (1 << 11) | (6 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
                reg |= (0 << 11) | (6 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
                reg |= (1 << 11) | (7 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
                reg |= (0 << 11) | (7 << 8);
                break;
            default:
                VTSS_E("port_no %u, Media interface not supported for Spyder", port_no);
                return VTSS_RC_ERR_PHY_MEDIA_IF_NOT_SUPPORTED;
            }
            break;
        case VTSS_PORT_INTERFACE_SERDES:
            reg = ((0 << 13)) | ((1 << 12));                                                   /* SerDes MAC; MAC Auto Negotiation disabled */
            switch (conf->media_if) {
            case VTSS_PHY_MEDIA_IF_CU:
                reg |= (0 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
                reg |= (1 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_FI_1000BX:
                reg |= (2 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU:
                reg |= (1 << 11)  |  (5 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
                reg |= (0 << 11) | (5 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
                reg |= (1 << 11) | (6 << 8);
                break;
            case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
                reg |= (0 << 11) | (6 << 8);
                break;
            default:
                VTSS_E("port_no %u, Media interface not supported for Spyder", port_no);
                return VTSS_RC_ERR_PHY_MEDIA_IF_NOT_SUPPORTED;
            }
            break;

        case VTSS_PORT_INTERFACE_NO_CONNECTION:
            reg |= (0 << 11) | (8 << 8); // Setting to Reserved. Shouldn't matter since there is no "interface" for this port
            break;


        default:
            VTSS_E("port_no %u, MAC interface %s not supported for Spyder",
                   port_no, vtss_port_if_txt(conf->mac_if));
            return VTSS_RC_ERROR;
        }
    }             /* phy_type == VTSS_PHY_TYPE_8658 */

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL, reg));
    return VTSS_RC_OK;
}

/* Init scripts for VSC8538/VSC8558/VSC8658 aka Spyder/GTO */
static vtss_rc vtss_phy_init_seq_spyder(vtss_state_t *vtss_state,
                                        vtss_phy_port_state_t *ps,
                                        vtss_port_no_t        port_no)
{
    u16 reg = 0;

    /* Init scripts common to all Octal PHY devices */
    /* BZ 2486/2487 */
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    /* BZ 2112 */
    /* Turn off Carrier Extensions */
    VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_3, 0x8000, 0x8000));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    if (ps->type.part_number == VTSS_PHY_TYPE_8538 || ps->type.part_number == VTSS_PHY_TYPE_8558) {
        if (ps->type.revision == 0) { /* VSC8538/58 Rev A */
            /* BZ 2020 */
            VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_27, 0x8000, 0x8000));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_19, 0x0300, 0x0f00));
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

            /* BZ 2063 */
            VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa60c));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa60c));
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg));
            if ((reg & (1 << 3)) == 0) {
                /* !RxTrLock */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0010));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8604));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x00df));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8600));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x00ff));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8600));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0000));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8604));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa60c));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa60c));
            }
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

            /* BZ 2069/2086 */
            VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_REG_17E, 0x0000, 0x0001));
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_4, &reg)); /* PHY address at bit 11-15 */
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

            /* BZ 2084 */
            if ((reg & (0x7 << 11)) == 0) {
                VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no,  VTSS_PHY_GPIO_0, 0x7009)); /*- Hold 8051 in SW Reset,Enable auto incr address and patch clock,Disable the 8051 clock */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5000)); /*- Dummy write to start off */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_11, 0xffff)); /*- Dummy write to start off */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5002)); /*- Dummy write to addr 16384= 02 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5040)); /*- Dummy write to addr 16385= 40 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x500C)); /*- Dummy write to addr 16386= 0C */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5002)); /*- Dummy write to addr 16387= 02 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5040)); /*- Dummy write to addr 16388= 40 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5021)); /*- Dummy write to addr 16389= 21 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5002)); /*- Dummy write to addr 16390= 02 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5040)); /*- Dummy write to addr 16391= 40 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5022)); /*- Dummy write to addr 16392= 22 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5002)); /*- Dummy write to addr 16393= 02 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5040)); /*- Dummy write to addr 16391= 40 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5023)); /*- Dummy write to addr 16392= 23 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50C2)); /*- Dummy write to addr 16396= C2 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50AD)); /*- Dummy write to addr 16397= AD */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50C2)); /*- Dummy write to addr 16396= C2 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50CA)); /*- Dummy write to addr 16399= CA */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5075)); /*- Dummy write to addr 16400= 75 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50CB)); /*- Dummy write to addr 16401= CB */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x509A)); /*- Dummy write to addr 16402= 9A */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5075)); /*- Dummy write to addr 16400= 75 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50CA)); /*- Dummy write to addr 16399= CA */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5046)); /*- Dummy write to addr 16405= 46 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5085)); /*- Dummy write to addr 16406= 85 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50CB)); /*- Dummy write to addr 16401= CB */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50CD)); /*- Dummy write to addr 16408= CD */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5085)); /*- Dummy write to addr 16406= 85 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50CA)); /*- Dummy write to addr 16399= CA */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50CC)); /*- Dummy write to addr 16411= CC */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50D2)); /*- Dummy write to addr 164VTSS_PHY_GPIO_12= D2 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50CA)); /*- Dummy write to addr 16399= CA */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50D2)); /*- Dummy write to addr 16414= D2 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x50AD)); /*- Dummy write to addr 16415= AD */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5022)); /*- Dummy write to addr 16416= 22 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5022)); /*- Dummy write to addr 16416= 22 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5022)); /*- Dummy write to addr 16416= 22 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x5022)); /*- Dummy write to addr 16416= 22 */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_12, 0x0000)); /*- Clear internal memory access */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_0, 0x4099)); /*- Allow 8051 to run again, with patch enabled */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_0, 0xc099)); /*- Allow 8051 to run again, with patch enabled */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
            }

            /* BZ 2087 */
            VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
            if (ps->type.part_number == VTSS_PHY_TYPE_8558) {
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa606));    /*- Request read, Media SerDes Control */
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg));
                reg = (reg & 0xfff8) | 5;                        /*- Optimize sample delay setting */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0000));    /*- Is this OK? */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8606));    /* Write Media SerDes Control Word */
            }
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xae06));       /*- Request read, MAC SerDes control */
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg));
            reg = (reg & 0xfff8) | 5;                           /*- Optimize sample delay setting */
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0000));       /*- Is this OK? */
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8e06));        /* Write MAC SerDes Control Word */
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        }

        /* BZ 2411 - PLL Error Detector Bit Enable */
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_2, 0x8000, 0x8000));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

        /* BZ 2230 - DSP Optimization, BZ 2230 */
        VTSS_RC(vtss_phy_optimize_dsp(vtss_state, port_no));

        /* BZ 1971 */
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, 0x0040, 0x0040));

        /* BZ 1860 */
        VTSS_RC(vtss_phy_optimize_receiver_init(vtss_state, port_no));

        /* BZ 2114 */
        VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xaf82));
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg));
        reg = (reg & 0xffef) | 0;           /*- Enable DFE in 100BT */
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f82));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

        /* BZ 2221*/
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_3, 0x1800, 0x1800));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

        /* BZ 2012 */
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear Force Bit */
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x6000, 0x6000));  /* MDI Impedance setting = +2 */
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_2, 0xa002, 0xe00e));  /* 100-BT Amplitude/Slew Rate Control */
    }   /* (ps->type.part_number == VTSS_PHY_TYPE_8538 || ps->type.part_number == VTSS_PHY_TYPE_8558) */
    else if (ps->type.part_number == VTSS_PHY_TYPE_8658) {
        if (ps->type.revision == 0) {
            /* BZ 2545 */
            /* 100-Base FX Clock Data Recovery Improvement */
            {
                u16 reg17, reg18;

                VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xae0e));
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, &reg18));
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg17));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, reg18));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, (reg17 & 0xffef)));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8e0e));
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
            }
        } /* ps->type.revision == 0 */
    }   /* (ps->type.part_number == VTSS_PHY_TYPE_8658) */

    // Improve 100BASE-TX link startup robustness to address interop issue
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0060));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0980));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f90));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    if ((!ps->reset.i_cpu_en) && (port_no % 8 == 0)) {
        /* disable iCPU */
        VTSS_RC(vtss_phy_micro_assert_reset(vtss_state, port_no));
    }


    /* Enable Smart Pre-emphasis */
    VTSS_RC(vtss_phy_enab_smrt_premphasis(vtss_state, port_no));

    return VTSS_RC_OK;
}

/* Init scripts for VSC8601/VSC8641 aka Cooper */
static vtss_rc vtss_phy_init_seq_cooper(vtss_state_t *vtss_state,
                                        vtss_phy_port_state_t *ps,
                                        vtss_port_no_t        port_no)
{
    u16 reg = 0;

    /* BZ 2474 */
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    if (ps->type.revision == 0) {    /*- Rev A */
        /* BZ 2231 */
        VTSS_RC(vtss_phy_optimize_dsp(vtss_state, port_no));

        /* BZ 2253 */
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_REG_17E, 0x0010, 0x0010)); /*- Enab Enh mode for Reg17E chg */
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_REG_17E, 0x0007, 0x0007)); /*- sets the LEDs combine/separate */
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_REG_17E, 0x0000, 0x0010)); /*- sets back to simple mode */
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

        /* BZ 2234 */
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_1000BASE_T_CONTROL, 0x0000, 0x0400));    /*- Port Type #2234 */
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_STD_27, 0x0004, 0x0004));   /*- nVidia Req */
    }

    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200)); //Ensure RClk125 enabled even in powerdown

    /* BZ 2644 - Improvement for marginal 10-Base T settings */
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x9e));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xdd39));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87aa));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa7b4));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, &reg));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, reg));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg));
    reg = (reg & ~0x003f) | 0x003c;
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87b4));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa794));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, &reg));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, reg));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg));
    reg = (reg & ~0x003f) | 0x003e;
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8794));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0xf7));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xbe36));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x879e));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa7a0));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, &reg));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, reg));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg));
    reg = (reg & ~0x003f) | 0x0034;
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87a0));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x3c));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xf3cf));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87a2));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x3c));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xf3cf));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87a4));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x3c));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xd287));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87a6));

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xa7a8));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, &reg));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, reg));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, &reg));
    reg = (reg & ~0x0fff) | 0x0125;
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, reg));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87a8));

    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200)); //Restore RClk125 gating
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    /* Enable Smart Pre-emphasis */
    VTSS_RC(vtss_phy_enab_smrt_premphasis(vtss_state, port_no));

    return VTSS_RC_OK;
}

/* Init scripts for VSC7385/VSC7388/VSC7395/VSC7398/VSC7389/VSC7390/VSC7391 aka "Luton" family */
static vtss_rc vtss_phy_init_seq_luton(vtss_state_t *vtss_state,
                                       vtss_phy_port_state_t *ps,
                                       vtss_port_no_t        port_no)
{
    /* BZ 1801, 1973 */
    /* 100 Base-TX jumbo frame support */
    VTSS_RC(vtss_phy_optimize_jumbo(vtss_state, port_no));

    /* BZ 2094 */
    /* Insufficient RGMII drive-strength seen especially on long traces */
    if (ps->family != VTSS_PHY_FAMILY_LUTON24) {
        if (ps->reset.mac_if == VTSS_PORT_INTERFACE_RGMII) {
            VTSS_RC(vtss_phy_optimize_rgmii_strength(vtss_state, port_no));
        }
    }

    /* BZ 1682, BZ 2345 */
    /* Enable PLL Error Detector Bit */
    /* Applicable to VSC 7385/7385 Rev A, VSC 7389/90 */
    if ((ps->family == VTSS_PHY_FAMILY_LUTON24) ||
        (ps->family == VTSS_PHY_FAMILY_LUTON && ps->type.revision == 0)) {
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_2, 0x8000, 0x8000));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    }

    /* Applicable to VSC 7385/7388 Rev A*/
    if (ps->family == VTSS_PHY_FAMILY_LUTON && ps->type.revision == 0) {
        /* BZ 1954 */
        /* Tweak 100 Base-TX DSP setting for VGA */
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
        VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0000));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0689));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f92));
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));

        /* BZ 1933 */
        /* Kick start Tx line-driver common-mode voltage */
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_23, 0xFF80));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_23, 0x0000));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    }

    /* BZ 2226/2227/2228 - DSP Optimization */
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
    VTSS_RC(vtss_phy_optimize_dsp(vtss_state, port_no));
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));

    /* BZ 2095/2107 - DSP short cable optimization */
    VTSS_RC(vtss_phy_optimize_receiver_init(vtss_state, port_no));
    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));                       /*- Additional Init for parts that */
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));  /*- lack Viterbi decoder */
    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));                         /*- to prevent DSP drift from */
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xb68a));               /*- leading to bit errors */
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0003, 0xff07));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x00a2, 0x00ff));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x968a));

    // Improve 100BASE-TX link startup robustness to address interop issue
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0060));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0980));
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f90));

    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    /* BZ 1742/1971/2000/2034 */
    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, 0x0040, 0x0040));

    /* Amplitude/ Z-Cal Adjustments at startup */
    /* These values are start-up values and will get readjusted based on link-up speed */
    if (ps->family == VTSS_PHY_FAMILY_LUTON24) {
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_2, 0xa002, 0xe00e));
    } else if (ps->family == VTSS_PHY_FAMILY_LUTON && ps->type.revision == 0) {
        /* BZ 2012 - Applies to VSC 7385/7388 Rev A */
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x4000, 0x6000)); /* Trim MDI Termination Impedance */
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_2, 0xa00e, 0xe00e)); /* Trim 100/1000 Tx amplitude & edge rate */
    } else {
        /* BZ 2043 - Applies to VSC 7385/7388 Rev B and later, VSC 7395/7398 all revs */
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_22, 0x0240, 0x0fc0)); /* Trim reference currents */
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x4000, 0x6000)); /* Trim termination impedance */
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_24, 0x0030, 0x0038)); /* 1000-BT Edge Rate Ctrl */
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_3, 0x2000, 0xe000)); /* Trim 1000 Base-T amplitude */
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_2, 0x8002, 0xe00e)); /* 100-BT Edge Rate Ctrl */
    }

    /* Enable Smart Pre-emphasis */
    VTSS_RC(vtss_phy_enab_smrt_premphasis(vtss_state, port_no));

    return VTSS_RC_OK;
}

/************************************************************************/
/* GPIO control                                                         */
/************************************************************************/
#define comma ,
#define GPIO_NOT_SUPPORTED -1
#define GPIOS_SUPPORT_CNT 14

static vtss_rc vtss_phy_is_gpio_supported(vtss_state_t               *vtss_state,
                                          const vtss_port_no_t       port_no,
                                          const u8                   gpio_no,
                                          gpio_table_t               *gpio_ctrl_table)
{

    static gpio_table_t gpio_ctrl_table_viper[GPIOS_SUPPORT_CNT] = {{13, 0}, {13, 2}, {13, 4}, {13, 6}, {13, 8}, {13, 10}, {13, 12}, {13, 14}, {14, 0}, {14, 2}, {14, 4}, {14, 6}, {14, 14}, {14, 14}}; // See Viper datasheet table 55.
    static gpio_table_t gpio_ctrl_table_tesla_8572[GPIOS_SUPPORT_CNT] = {{13, 0}, {13, 2}, {GPIO_NOT_SUPPORTED, GPIO_NOT_SUPPORTED}, {GPIO_NOT_SUPPORTED, GPIO_NOT_SUPPORTED}, {13, 8}, {13, 10}, {GPIO_NOT_SUPPORTED, GPIO_NOT_SUPPORTED}, {GPIO_NOT_SUPPORTED, GPIO_NOT_SUPPORTED}, {14, 0}, {14, 2}, {14, 4}, {14, 6}, {14, 14}, {14, 14}}; // See Tesla 7472 datasheet, table 33.

    static gpio_table_t gpio_ctrl_table_tesla_8574[GPIOS_SUPPORT_CNT] = {{13, 0}, {13, 2}, {13, 4}, {13, 6}, {13, 8}, {13, 10}, {13, 12}, {13, 14}, {14, 0}, {14, 2}, {14, 4}, {14, 6}, {14, 14}, {14, 14}};  // See Tesla 7474 datasheet, table 29.

    if (gpio_no > GPIOS_SUPPORT_CNT) {
        return VTSS_RC_ERR_PHY_GPIO_PIN_NOT_SUPPORTED;
    }

    switch (vtss_state->phy_state[port_no].family) {
    case VTSS_PHY_FAMILY_TESLA:
        VTSS_D("Pointer to gpio_ctrl_table_tesla");
        if (vtss_state->phy_state[port_no].type.part_number == VTSS_PHY_TYPE_8572) {
            memcpy(gpio_ctrl_table, &gpio_ctrl_table_tesla_8572[0], sizeof(gpio_ctrl_table_tesla_8572));
        } else {
            memcpy(gpio_ctrl_table, &gpio_ctrl_table_tesla_8574[0], sizeof(gpio_ctrl_table_tesla_8574));
        }
        break;
    case VTSS_PHY_FAMILY_VIPER:
        VTSS_D("Pointer to gpio_ctrl_table_viper");
        memcpy(gpio_ctrl_table, &gpio_ctrl_table_viper[0], sizeof(gpio_ctrl_table_viper));
        break;
    default:
        return VTSS_RC_ERR_PHY_GPIO_PIN_NOT_SUPPORTED;
    }

    if (gpio_ctrl_table[gpio_no].reg == GPIO_NOT_SUPPORTED) {
        VTSS_D("GPIO NOT Supported");
        return VTSS_RC_ERR_PHY_GPIO_PIN_NOT_SUPPORTED;
    }

    if (VTSS_PHY_BASE_PORTS_FOUND != VTSS_RC_OK) {
        VTSS_I("Base port not found");
        return VTSS_RC_ERR_PHY_BASE_NO_NOT_FOUND;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_gpio_mode_private(vtss_state_t               *vtss_state,
                                          const vtss_port_no_t       port_no,
                                          const u8                   gpio_no,
                                          const vtss_phy_gpio_mode_t mode)
{

    gpio_table_t gpio_ctrl_table[GPIOS_SUPPORT_CNT];
    u8            mode_val = 0x3;

    VTSS_RC(vtss_phy_is_gpio_supported(vtss_state, port_no, gpio_no, &gpio_ctrl_table[0]));

    VTSS_D("GPIO:%d supported, mode:%d", gpio_no, mode);

    if (mode == VTSS_PHY_GPIO_ALT_1 || mode == VTSS_PHY_GPIO_ALT_2) {
        // Currently we don't support this
        return VTSS_RC_ERR_PHY_GPIO_ALT_MODE_NOT_SUPPORTED;
    }


    switch (mode) {
    case VTSS_PHY_GPIO_OUT:
    case VTSS_PHY_GPIO_IN:
        mode_val = 0x3; // So far all GPIO is set to in/out with the value 0x3 (See datasheet)
        break;
    case VTSS_PHY_GPIO_ALT_0:
        mode_val = 0;
        break;
    case VTSS_PHY_GPIO_ALT_1:
        mode_val = 1;
        break;
    case VTSS_PHY_GPIO_ALT_2:
        mode_val = 2;
        break;
    }


    VTSS_RC(vtss_phy_page_gpio(vtss_state, vtss_state->phy_state[port_no].type.base_port_no));
    VTSS_D("port_no:%d, gpio_no:%d, base port:%d, reg:%d, bit:%d, mode_val:%d, mode:%d", port_no, gpio_no, vtss_state->phy_state[port_no].type.base_port_no, gpio_ctrl_table[gpio_no].reg, gpio_ctrl_table[gpio_no].bit, mode_val, mode);


    // Setup mode
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, vtss_state->phy_state[port_no].type.base_port_no, VTSS_PHY_PAGE_GPIO comma  gpio_ctrl_table[gpio_no].reg, mode_val << gpio_ctrl_table[gpio_no].bit, 0x3 << gpio_ctrl_table[gpio_no].bit));


    // Setup in or output
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, vtss_state->phy_state[port_no].type.base_port_no, VTSS_PHY_GPIO_IN_OUT_CONF,
                                    mode == VTSS_PHY_GPIO_OUT ? (1 << gpio_no) : 0, (1 << gpio_no)));

    VTSS_RC(vtss_phy_page_std(vtss_state, vtss_state->phy_state[port_no].type.base_port_no));

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_gpio_get_private(vtss_state_t         *vtss_state,
                                         const vtss_port_no_t port_no,
                                         const u8             gpio_no,
                                         BOOL                 *value)
{
    u16          reg;
    gpio_table_t gpio_ctrl_table[GPIOS_SUPPORT_CNT]; // Dummy, in fact not needed here
    VTSS_RC(vtss_phy_is_gpio_supported(vtss_state, port_no, gpio_no, gpio_ctrl_table));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_INPUT, &reg));
    *value = (((reg >> gpio_no) & 0x1) == 0x1) ? TRUE : FALSE;
    VTSS_D("gpio_no:%d, reg:0x%X, %d, val:%d", gpio_no, reg, ((reg >> gpio_no) & 0x1), *value);
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_gpio_set_private(vtss_state_t         *vtss_state,
                                         const vtss_port_no_t port_no,
                                         const u8             gpio_no,
                                         const BOOL           value)
{

    gpio_table_t gpio_ctrl_table[GPIOS_SUPPORT_CNT]; // Dummy, in fact not needed here
    VTSS_RC(vtss_phy_is_gpio_supported(vtss_state, port_no, gpio_no, gpio_ctrl_table));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
    // Setup in or output
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, vtss_state->phy_state[port_no].type.base_port_no, VTSS_PHY_GPIO_OUTPUT,
                                    value << gpio_no, 1 << gpio_no));

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}


/************************************************************************/
/* Interrupt functions                                               */
/************************************************************************/

// Function for setting interrupt mask events
// In - port_no - PHY port number.
static vtss_rc vtss_phy_event_enable_private(vtss_state_t *vtss_state,
                                             const vtss_port_no_t  port_no)
{
    u16     mask = 0;
    vtss_rc rc = VTSS_RC_OK;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_phy_event_t   ev_mask = ps->ev_mask;
    vtss_phy_conf_t       *conf = &ps->setup;
    u16     reg;
    vtss_phy_reset_conf_t *reset_conf = &ps->reset;

    //Bugzilla#4467 - Board/interrupt: Rev-B support of Fast Link Fail from internal PHY
    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
        if (ps->type.revision == VTSS_PHY_ATOM_REV_B) {
            if (vtss_state->phy_state[port_no].eee_conf.eee_mode == EEE_ENABLE) {
                ev_mask = 0;     // Disable all interrupts when EEE is enabled due to Bugzilla#4467
                rc = VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_INTERRUPT_MASK, 0);
            }
        }
        break;
    case VTSS_PHY_FAMILY_COBRA:
        VTSS_N("Port:%d  Bugzilla#7489 - Interrupt must be disable when powered down.", port_no);
        if (conf->mode ==  VTSS_PHY_MODE_POWER_DOWN) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_INTERRUPT_MASK, 0x0, 0xFFFF));
            return VTSS_RC_OK;
        }
        break;
    default:
        break;
    }


    VTSS_N("vtss_phy_event_enable_private - ev_mask:0x%X", ev_mask);
    // Exit if this PHY doesn't support interrupts
    if (!vtss_phy_can(vtss_state, port_no, VTSS_CAP_INT)) {
        return (VTSS_RC_ERROR);
    }

    if (ev_mask & VTSS_PHY_LINK_LOS_EV) {
        mask |= VTSS_F_PHY_INTERRUPT_MASK_LINK_MASK;
    }

#ifdef VTSS_SW_OPTION_EEE
    u16     reg_val;
    // Bug in chip. Mask out fast link failure when EEE is included due to bugzilla#2965 & #2966
    VTSS_RC(vtss_phy_mmd_rd(vtss_state, port_no, 7, 60, &reg_val)); // Read current value of the register
    if (ev_mask && (reg_val & 0x0006) && (ps->family == VTSS_PHY_FAMILY_ATOM || ps->family != VTSS_PHY_FAMILY_LUTON26)) { // Do not allow enable if EEE is enabled - Check if advertisement is enabled
    } else
#endif
    {
        if (ev_mask & VTSS_PHY_LINK_FFAIL_EV) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_FAST_LINK_MASK;
            // Work around of Bugzilla#8881, False interrupts when no 100FX SFP module plugged in (Disable fast link interrupt if link is down)
            if (reset_conf->media_if == VTSS_PHY_MEDIA_IF_FI_100FX || reset_conf->media_if == VTSS_PHY_MEDIA_IF_AMS_FI_100FX) {
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MODE_STATUS, &reg));

                // Disable dat link interrupt if link is down.
                if (!(reg & VTSS_F_PHY_STATUS_LINK_STATUS)) {
                    mask &= ~VTSS_F_PHY_INTERRUPT_MASK_FAST_LINK_MASK; // Clear the mask again.

                    // Disable the fast link interrupt
                    rc = VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_INTERRUPT_MASK, 0, VTSS_F_PHY_INTERRUPT_MASK_FAST_LINK_MASK);
                }
                VTSS_D("reg:0x%X, mask:0x%X, port_no:%d", reg, mask, port_no);
            }
        }


        if (ev_mask & VTSS_PHY_LINK_SPEED_STATE_CHANGE_EV) {
            if (ps->family == VTSS_PHY_FAMILY_COBRA) {
                VTSS_E("PHY Family %s doesn't support \"Speed state change\" interrupt", vtss_phy_family2txt(ps->family));
            } else {
                mask |= VTSS_F_PHY_INTERRUPT_MASK_SPEED_STATE_CHANGE_MASK;
            }
        }

        if (ev_mask & VTSS_PHY_LINK_FDX_STATE_CHANGE_EV) {
            if (ps->family == VTSS_PHY_FAMILY_COBRA) {
                VTSS_I("PHY Family %s doesn't support \"FDX state change\" interrupt", vtss_phy_family2txt(ps->family));
            } else {
                mask |= VTSS_F_PHY_INTERRUPT_MASK_FDX_STATE_CHANGE_MASK;
            }
        }

        if (ev_mask & VTSS_PHY_LINK_AUTO_NEG_ERROR_EV) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_AUTO_NEG_ERROR_MASK;
        }

        if (ev_mask & VTSS_PHY_LINK_AUTO_NEG_COMPLETE_EV) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_AUTO_NEG_COMPLETE_MASK;
        }

        if (ev_mask & VTSS_PHY_LINK_INLINE_POW_DEV_DETECT_EV) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_INLINE_POW_DEV_DETECT_MASK;
        }

        if (ev_mask & VTSS_PHY_LINK_SYMBOL_ERR_INT_EV) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_SYMBOL_ERR_INT_MASK;
        }

        if (ev_mask & VTSS_PHY_LINK_TX_FIFO_OVERFLOW_INT_EV) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_TX_FIFO_OVERFLOW_INT_MASK;
        }

        if (ev_mask & VTSS_PHY_LINK_RX_FIFO_OVERFLOW_INT_EV) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_RX_FIFO_OVERFLOW_INT_MASK;
        }

        if (ev_mask & VTSS_PHY_LINK_FALSE_CARRIER_INT_EV) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_FALSE_CARRIER_INT_MASK;
        }

        if (ev_mask & VTSS_PHY_LINK_SPEED_STATE_CHANGE_EV ) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_LINK_SPEED_DS_DETECT_MASK;
        }

        if (ev_mask & VTSS_PHY_LINK_MASTER_SLAVE_RES_ERR_EV) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_MASTER_SLAVE_RES_ERR_MASK;
        }

        if (ev_mask & VTSS_PHY_LINK_RX_ER_INT_EV) {
            mask |= VTSS_F_PHY_INTERRUPT_MASK_RX_ER_INT_MASK;
        }
    }

    if (ev_mask & VTSS_PHY_LINK_AMS_EV) {
        if (ps->family == VTSS_PHY_FAMILY_COBRA || ps->family == VTSS_PHY_FAMILY_ENZO) {
            VTSS_D("PHY Family %s doesn't support \"AMS state change\" interrupt or is not in AMS mode", vtss_phy_family2txt(ps->family));
        } else {
            if (!is_phy_in_ams_mode(vtss_state, port_no)) {
                VTSS_I("Note PHY is not in AMS mode, the signal detect pins must not be floating when setting enabling AMS interrupts");
            }
            mask |= VTSS_F_PHY_INTERRUPT_MASK_AMS_MEDIA_CHANGE_MASK;
        }
    }

    rc = vtss_phy_page_std(vtss_state, port_no);
    if (mask) {
        mask |= VTSS_F_PHY_INTERRUPT_MASK_INT_MASK;         /* Add Interrupt Status */
    }
    VTSS_N("port_no:%d, ev_mask:0x%X, mask:0x%X", port_no, ev_mask, mask);
    rc = VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_INTERRUPT_MASK, mask, mask);
    ps->int_mask_reg = mask;

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return rc;
}


// Function for getting the interrupt event status
static vtss_rc vtss_phy_event_poll_private(vtss_state_t *vtss_state,
                                           const vtss_port_no_t  port_no,
                                           vtss_phy_event_t      *const events)
{
    u16     pending, mask;
    vtss_rc rc = VTSS_RC_OK;

    *events = 0;

    if (!vtss_phy_can(vtss_state, port_no, VTSS_CAP_INT)) {
        return VTSS_RC_ERROR;
    }
    rc = vtss_phy_page_std(vtss_state, port_no);
    rc = PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_INTERRUPT_STATUS, &pending); /* Pending is cleared by read */
    //rc = PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_INTERRUPT_MASK, &mask);
    mask = vtss_state->phy_state[port_no].int_mask_reg;
    VTSS_N("port_no:%d, pending:0x%X, mask:0x%X", port_no, pending, mask);


    pending &= ~VTSS_F_PHY_INTERRUPT_MASK_INT_MASK; /* pending on 'Interrupt Status' is not included */
    pending &= mask; /* Only include enabled interrupts */
    if (pending) {
        if (pending & VTSS_F_PHY_INTERRUPT_MASK_LINK_MASK) {
            *events |= VTSS_PHY_LINK_LOS_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_FAST_LINK_MASK) {
            *events |= VTSS_PHY_LINK_FFAIL_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_AMS_MEDIA_CHANGE_MASK) {
            *events |= VTSS_PHY_LINK_AMS_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_SPEED_STATE_CHANGE_MASK) {
            *events |= VTSS_PHY_LINK_SPEED_STATE_CHANGE_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_FDX_STATE_CHANGE_MASK) {
            *events |= VTSS_PHY_LINK_FDX_STATE_CHANGE_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_AUTO_NEG_ERROR_MASK) {
            *events |= VTSS_PHY_LINK_AUTO_NEG_ERROR_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_AUTO_NEG_COMPLETE_MASK) {
            *events |= VTSS_PHY_LINK_AUTO_NEG_COMPLETE_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_INLINE_POW_DEV_DETECT_MASK) {
            *events |= VTSS_PHY_LINK_INLINE_POW_DEV_DETECT_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_SYMBOL_ERR_INT_MASK) {
            *events |= VTSS_PHY_LINK_SYMBOL_ERR_INT_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_TX_FIFO_OVERFLOW_INT_MASK) {
            *events |= VTSS_PHY_LINK_TX_FIFO_OVERFLOW_INT_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_RX_FIFO_OVERFLOW_INT_MASK) {
            *events |= VTSS_PHY_LINK_RX_FIFO_OVERFLOW_INT_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_FALSE_CARRIER_INT_MASK) {
            *events |= VTSS_PHY_LINK_FALSE_CARRIER_INT_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_LINK_SPEED_DS_DETECT_MASK) {
            *events |= VTSS_PHY_LINK_SPEED_STATE_CHANGE_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_MASTER_SLAVE_RES_ERR_MASK) {
            *events |= VTSS_PHY_LINK_MASTER_SLAVE_RES_ERR_EV;
        }

        if (pending & VTSS_F_PHY_INTERRUPT_MASK_RX_ER_INT_MASK) {
            *events |= VTSS_PHY_LINK_RX_ER_INT_EV;
        }
    }

    VTSS_N("port_no:%d, ev_mask:0x%X", port_no, *events);

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return rc;
}



/************************************************************************/
/* Reset functions                                               */
/************************************************************************/

// Function for setting phy in pass through mode according to "Application Note : Protocol transfer mode guide"
static vtss_rc vtss_phy_pass_through_speed_mode(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_phy_conf_t       *setup_conf = &ps->setup;

    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
        // From Protocol Transfer mode Guide (Written by James McIntosh and Jim Barnette)
        VTSS_I("Port:%d: Pass through mode setting mode:%d, speed:%d", port_no, setup_conf->mode, setup_conf->forced.speed);

        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        //Protocol Transfer mode Guide : Section 4.1.1 - Aneg must be enabled
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, VTSS_F_PHY_MODE_CONTROL_AUTO_NEG_ENA, VTSS_F_PHY_MODE_CONTROL_AUTO_NEG_ENA));

        VTSS_RC(vtss_phy_page_ext3(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MAC_SERDES_PCS_CONTROL, VTSS_F_MAC_SERDES_PCS_CONTROL_ANEG_ENA, VTSS_F_MAC_SERDES_PCS_CONTROL_ANEG_ENA | VTSS_F_MAC_SERDES_PCS_CONTROL_FORCE_ADV_ABILITY)); // Default clear "force advertise ability" bit as well

        // Protocol Transfer mode Guide : Section 4.1.3
        if (setup_conf->mode == VTSS_PHY_MODE_FORCED) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MAC_SERDES_PCS_CONTROL, VTSS_F_MAC_SERDES_PCS_CONTROL_FORCE_ADV_ABILITY, VTSS_F_MAC_SERDES_PCS_CONTROL_FORCE_ADV_ABILITY));

            switch (setup_conf->forced.speed) {
            case VTSS_SPEED_100M:
                VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_MAC_SERDES_CLAUSE_37_ADVERTISED_ABILITY, 0x8401));
                break;

            case VTSS_SPEED_10M:
                VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_MAC_SERDES_CLAUSE_37_ADVERTISED_ABILITY, 0x8001));
                break;

            case VTSS_SPEED_1G:
                VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_MAC_SERDES_CLAUSE_37_ADVERTISED_ABILITY, 0x8801));
                break;

            default:
                VTSS_E("Unexpected port speed:%d defaulting to 1G", setup_conf->forced.speed);
                VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_MAC_SERDES_CLAUSE_37_ADVERTISED_ABILITY, 0x8801));
                break;
            }
        }
        break;
    default:
        VTSS_D("Port:%d: All other PHYs don't need this or are not supporting SFP pass through mode.", port_no);
    }

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_gp_reg_rd(vtss_state_t *vtss_state, vtss_port_no_t port_no, u16 *value)
{
    VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
    VTSS_RC(vtss_phy_rd(vtss_state, port_no, 30, value));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_gp_reg_wr(vtss_state_t *vtss_state, vtss_port_no_t port_no, u16 value)
{
    VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
    VTSS_RC(vtss_phy_wr(vtss_state, port_no, 30, value));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_detect_base_ports_private(vtss_state_t *vtss_state)
{
    vtss_port_no_t port_x, port_y, base_port_no, base_port_1588inst0, base_port_1588inst1;
    BOOL           port_skip[VTSS_PORTS], port_phy[VTSS_PORTS];
    u16            gp_reg[VTSS_PORTS], gp_x, gp_y, addr, addr_min;
    u8             port_addr[VTSS_PORTS];

    /* Read and clear GP register for all ports */
    for (port_x = 0; port_x < vtss_state->port_count; port_x++) {
        gp_reg[port_x] = 0; /* Just to please Lint */
        port_skip[port_x] = 1;
        switch (vtss_state->phy_state[port_x].family) {
        case VTSS_PHY_FAMILY_TESLA:
        case VTSS_PHY_FAMILY_ATOM:
        case VTSS_PHY_FAMILY_ENZO:
        case VTSS_PHY_FAMILY_LUTON26:
        case VTSS_PHY_FAMILY_VIPER:
        case VTSS_PHY_FAMILY_ELISE:
            if (vtss_phy_gp_reg_rd(vtss_state, port_x, &gp_reg[port_x]) == VTSS_RC_OK &&
                vtss_phy_gp_reg_wr(vtss_state, port_x, 0) == VTSS_RC_OK) {
                port_skip[port_x] = 0;
            }
            break;
        default:
            break;
        }
    }

    /* Write unique value to GP register using broadcast write (register 22) */
    for (port_x = 0; port_x < vtss_state->port_count; port_x++) {
        if (port_skip[port_x]) {
            continue;
        }
        VTSS_RC(vtss_phy_wr_masked(vtss_state, port_x, 22, 0x0001, 0x0001));
        VTSS_RC(vtss_phy_gp_reg_wr(vtss_state, port_x, 0x0100 + port_x));
        VTSS_RC(vtss_phy_wr_masked(vtss_state, port_x, 22, 0x0000, 0x0001));
    }

    /* Find base port for each PHY */
    for (port_x = 0; port_x < vtss_state->port_count; port_x++) {
        if (port_skip[port_x] || vtss_phy_gp_reg_rd(vtss_state, port_x, &gp_x) != VTSS_RC_OK) {
            continue;
        }

        base_port_no = VTSS_PORT_NO_NONE;
        base_port_1588inst0 = VTSS_PORT_NO_NONE;
        base_port_1588inst1 = VTSS_PORT_NO_NONE;
        addr_min = 0xffff;
        for (port_y = port_x; port_y < vtss_state->port_count; port_y++) {
            port_phy[port_y] = 0;
            port_addr[port_y] = 0xff;
            if (!port_skip[port_y] &&
                vtss_phy_gp_reg_rd(vtss_state, port_y, &gp_y) == VTSS_RC_OK &&
                gp_x == gp_y) {
                /* Found port_x and port_y on same PHY */
                port_phy[port_y] = 1;
                if (vtss_phy_page_ext(vtss_state, port_y) == VTSS_RC_OK &&
                    vtss_phy_rd(vtss_state, port_y, 23, &addr) == VTSS_RC_OK &&
                    vtss_phy_page_std(vtss_state, port_y) == VTSS_RC_OK) {
                    /* Base port has smallest PHY address (register 23E1) */
                    if (addr <= addr_min) {
                        addr_min = addr;
                        base_port_no = port_y;
                    }
                    if (((vtss_state->phy_state[port_y].family) == VTSS_PHY_FAMILY_TESLA) ||
                        ((vtss_state->phy_state[port_y].family) == VTSS_PHY_FAMILY_VIPER)) {
                        port_addr[port_y] = (addr & 0x1800) >> 11;
                        if (port_addr[port_y] == 0) {
                            base_port_1588inst0 = port_y;
                        }
                        if (port_addr[port_y] == 1) {
                            base_port_1588inst1 = port_y;
                        }
                    }
                }
            }
        }
        /* Save 1588 base port for all ports on PHY */
        for (port_y = port_x; port_y < vtss_state->port_count; port_y++) {
            if (((vtss_state->phy_state[port_y].family) == VTSS_PHY_FAMILY_TESLA) ||
                ((vtss_state->phy_state[port_y].family) == VTSS_PHY_FAMILY_VIPER)) {
                if (port_phy[port_y]) {
                    /* Base port was found */
                    if ((port_addr[port_y] == 0) || (port_addr[port_y] == 2)) {
                        vtss_state->phy_state[port_y].type.phy_api_base_no = base_port_1588inst0;
                        VTSS_D("port: %u, 1588 base: %u", port_y, base_port_1588inst0);
                    } else if ((port_addr[port_y] == 1) || (port_addr[port_y] == 3)) {
                        vtss_state->phy_state[port_y].type.phy_api_base_no = base_port_1588inst1;
                        VTSS_D("port: %u, 1588 base: %u", port_y, base_port_1588inst1);
                    }
                }
            }
        }

        /* Save base port for all ports on PHY */
        for (port_y = port_x; port_y < vtss_state->port_count; port_y++) {
            if (port_phy[port_y]) {
                /* Base port was found */
                VTSS_D("port: %u, base: %u", port_y, base_port_no);
                vtss_state->phy_state[port_y].type.base_port_no = base_port_no;
                port_skip[port_y] = 1;
            }
        }

        /* Restore GP register */
        VTSS_RC(vtss_phy_gp_reg_wr(vtss_state, port_x, gp_reg[port_x]));
    }
    vtss_state->phy_inst_state.base_ports_found = TRUE; // Signaling that base port found (mainly for the 6g macro configuration setup)
    return VTSS_RC_OK;
}

// Function that is called at boot, after port reset.
// The function is calling the post initialization script (setting coma).
// IN : port_no - Any port within the chip.
vtss_rc vtss_phy_post_reset_private(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_port_no_t port_idx;

    // The 6G Macro can't be setup before all base ports are found, so that we do now.
    for (port_idx = 0; port_idx < vtss_state->port_count; port_idx++) {
        VTSS_RC(vtss_phy_sd6g_patch_private(vtss_state, port_idx));
    }

    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        VTSS_RC(vtss_phy_coma_mode_private(vtss_state, port_no, TRUE));
        break;
    default:
        VTSS_D("No post-initialising needed for family:%s, port = %d",
               vtss_phy_family2txt(ps->family), port_no);
    }
    return VTSS_RC_OK;
}


static vtss_rc vtss_phy_mac_media_if_enzo_setup(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no, const vtss_phy_reset_conf_t *const conf)
{
    u16 reg = 0;

    switch (conf->mac_if) {
    case VTSS_PORT_INTERFACE_SGMII:
        reg = ((0 << 13) | (0 << 12)); /* SGMII, auto negotiation disabled */
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            reg |= (0 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
            reg |= (1 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
            reg |= (2 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_FI_100FX:
            reg |= (3 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU:
            reg |= (1 << 11)  |  (5 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
            reg |= (0 << 11) | (5 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
            reg |= (1 << 11) | (6 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
            reg |= (0 << 11) | (6 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
            reg |= (1 << 11) | (7 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
            reg |= (0 << 11) | (7 << 8);
            break;
        default:
            VTSS_E("port_no %u, Media interface %d not supported", port_no, conf->media_if);
            return VTSS_RC_ERR_PHY_MEDIA_IF_NOT_SUPPORTED;
        }
        break;

    case VTSS_PORT_INTERFACE_SERDES:
        reg = ((0 << 13) | (1 << 12));
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            reg |= (0 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
            reg |= (1 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
            reg |= (2 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_PASSTHRU:
            reg |= (1 << 11) | (5 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_PASSTHRU:
            reg |= (0 << 11) | (5 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
            reg |= (1 << 11) | (6 << 8);
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
            reg |= (0 << 11) | (6 << 8);
            break;
        default:
            VTSS_E("port_no %u, Media interface %d not supported", port_no, conf->media_if);
            return VTSS_RC_ERR_PHY_MEDIA_IF_NOT_SUPPORTED;
        }
        break;

    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        reg |= (0 << 11) | (8 << 8); // Setting to Reserved. Shouldn't matter since there is no "interface" for this port
        break;


    default:
        VTSS_E("port_no %u, MAC interface %s not supported",
               port_no, vtss_port_if_txt(conf->mac_if));
        return VTSS_RC_ERROR;
    }

    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL, reg));
    return VTSS_RC_OK;
}


/* Init scripts for VSC8634-VSC8664 aka "Enzo" family */
static vtss_rc vtss_phy_init_seq_enzo(vtss_state_t *vtss_state,
                                      vtss_phy_port_state_t *ps,
                                      vtss_port_no_t        port_no)
{
    u16 reg = 0;

    VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_4, &reg)); /* PHY address at bit 11-15 */
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    /* Script runs only for the first port of each device in system */
    if ((reg & (0x3 << 11)) == 0) {
        /* Enable Broad-cast writes for this device */
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_CONTROL_AND_STATUS, 0x0001, 0x0001));

        if (ps->type.revision == 0) {    /*- Rev A */

            /* BZ 2633 */
            /* Enable LED blinking after reset */
            VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_MODE_CONTROL, 0x0800, 0x0800));

            /* BZ 2637 */
            /* 100/1000 Base-T amplitude compenstation */
            VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_24, 0x0003, 0x0007)); // ->(val & ~mask) | (*value & mask) i.e (value at 24 & 0xfff8) | (0x0003 & 0x0007)
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_2, 0x0004, 0x000e)); // ->(val & ~mask) | (*value & mask) i.e(value at 24 & 0xfff1) | (0x0004 & 0x000e)

            /* BZ 2639 / BZ 2642 */
            /* Improve robustness of 10Base-T performance */
            VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x3f));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8794));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0xf7));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xadb4));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x879e));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x32));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87a0));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x41));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x410));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87a2));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x41));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x410));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87a4));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x41));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x284));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87a6));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x92));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xbcb8));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87a8));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x3));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xcfbf));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87aa));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x49));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x2451));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87ac));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x1));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x1410));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87c0));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x10));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xb498));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87e8));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x71));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xe7dd));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87ea));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x69));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x6512));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87ec));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x49));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x2451));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87ee));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x45));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x410));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87f0));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x41));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x410));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87f2));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x10));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87f4));

            VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_9, 0x0040, 0x0040));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_22, 0x0010, 0x0010));
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

            /* BZ 2643 */
            /* Performance optimization - 100Base-TX/1000Base-T slave */
            VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_0, 0x0060, 0x00e0));
            VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x12));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x480a));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f82));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x422));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f86));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x3c));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x3800));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x8f8a));

            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x8));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0xe33f));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x83ae));
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        }   /*- Rev A */

        /* BZ 2112 */
        /* Turn off Carrier Extensions */
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_3, 0x8000, 0x8000));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

        /* Turn-off broad-cast writes for this device */
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_CONTROL_AND_STATUS, 0x0000, 0x0001));
    }   /* ps->map.addr % 4) == 0 */

    if ((!ps->reset.i_cpu_en) && (port_no % 4 == 0)) {
        /* disable iCPU */
        VTSS_RC(vtss_phy_micro_assert_reset(vtss_state, port_no));
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_phy_init_conf_set(vtss_state_t *vtss_state)
{
    vtss_init_conf_t *conf = &vtss_state->init_conf;
    vtss_port_no_t   port_no = conf->restart_info_port;
    u16              reg;
    u32              value;
    VTSS_D("vtss_phy_init_conf_set, port_no:%d", port_no);
    if (conf->restart_info_src == VTSS_RESTART_INFO_SRC_CU_PHY &&
        vtss_phy_detect(vtss_state, port_no) == VTSS_RC_OK &&
        vtss_state->phy_state[port_no].family != VTSS_PHY_FAMILY_NONE) {
        /* Get restart information from Vitesse 1G PHY */
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, EPG_CTRL_REG_1, &reg)); /* 16 MSB at register 29E1 */
        value = reg;
        value = (value << 16);
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, EPG_CTRL_REG_2, &reg)); /* 16 LSB at register 30E1 */
        value += reg;
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        VTSS_RC(vtss_cmn_restart_update(vtss_state, value));
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_phy_restart_conf_set(vtss_state_t *vtss_state)
{
    vtss_init_conf_t *conf = &vtss_state->init_conf;
    vtss_port_no_t   port_no = conf->restart_info_port;
    u16              reg;
    u32              value;

    if (conf->restart_info_src == VTSS_RESTART_INFO_SRC_CU_PHY &&
        vtss_state->phy_state[port_no].family != VTSS_PHY_FAMILY_NONE) {
        /* Set restart information in Vitesse 1G PHY */
        value = vtss_cmn_restart_value_get(vtss_state);
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        reg = ((value >> 16) & 0x7fff); /* Bit 15 must be zero to disable EPG */
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, EPG_CTRL_REG_1, reg)); /* 16 MSB at register 29E1 */
        reg = (value & 0xffff);
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, EPG_CTRL_REG_2, reg)); /* 16 LSB at register 30E1 */
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    }

    return VTSS_RC_OK;
}

vtss_rc vtss_phy_reset_private(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_phy_reset_conf_t *conf = &ps->reset;
    u16                   reg;

    // Don't signal link down if doing warm start unless sonething in fact has changed
    if (!vtss_state->sync_calling_private || ps->warm_start_reg_changed) {

        // If the link is up we have to remember that link has been down due to port reset.
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MODE_STATUS, &reg));

        if (reg & VTSS_F_PHY_STATUS_LINK_STATUS) {
            ps->link_down_due_to_port_reset = TRUE;
            VTSS_D("link_down_due_to_port_reset = TRUE, port_no = %d", port_no);
        } else {
            VTSS_D("link_down_due_to_port_reset = FALSE, port_no = %d", port_no);
            ps->link_down_due_to_port_reset = FALSE;
        }
    }


    /* -- Step 2: Pre-reset setup of MAC and Media interface -- */
    switch (ps->family) {
    case VTSS_PHY_FAMILY_MUSTANG:
        /* TBD */
        break;
    case VTSS_PHY_FAMILY_BLAZER:
        /* Interface is setup after reset */
        if (ps->type.revision < 4 || ps->type.revision > 6) {
            VTSS_E("port_no %u, unsupported Blazer revision: %u", port_no, ps->type.revision);
            return VTSS_RC_ERROR;
        }
        break;
    case VTSS_PHY_FAMILY_COBRA:
        VTSS_RC(vtss_phy_mac_media_if_cobra_setup(vtss_state, port_no, conf));
        break;
    case VTSS_PHY_FAMILY_QUATTRO:
        VTSS_RC(vtss_phy_mac_media_if_quattro_setup(vtss_state, port_no, conf));
        break;
    case VTSS_PHY_FAMILY_SPYDER:
        if (ps->type.revision == 0) {
            /* BZ 2027 */
            VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_GPIO_0, 0x4c19));
            VTSS_RC(vtss_phy_micro_assert_reset(vtss_state, port_no));
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        }
        VTSS_RC(vtss_phy_mac_media_if_spyder_gto_setup(vtss_state, port_no, conf, ps->type));
        break;
    case VTSS_PHY_FAMILY_ENZO:
        VTSS_RC(vtss_phy_mac_media_if_enzo_setup(vtss_state, port_no, conf));
        break;
    case VTSS_PHY_FAMILY_COOPER: {
        u16 tx_clk_skew, rx_clk_skew;
        reg = 0;
        switch (conf->rgmii.tx_clk_skew_ps) {
        // Setting 0 - disabled RGMII skew compensation (0 ps of skew)
        case 0 :
            rx_clk_skew = 0;
            tx_clk_skew = 0;
            break;
        // Setting 1 - enabled RGMII skew compensation at low delay of approximately 1400 ps of skew (original target was 1.5 ns)
        case 1400 :
            rx_clk_skew = 1;
            tx_clk_skew = 1;
            break;
        case 1700:
            // Setting 2 - enabled RGMII skew compensation at medium delay of approximately 1700 ps of skew (original target was 2.0 ns)
            rx_clk_skew = 2;
            tx_clk_skew = 2;
            break;
        default :
            // Setting 3 - enabled RGMII skew compensation at high delay of approximately 2000 ps of skew (original target was 2.5 ns)
            rx_clk_skew = 3;
            tx_clk_skew = 3;
            break;
        }

        reg = tx_clk_skew << 14 | rx_clk_skew << 12;
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXT_28, reg, 0xf000));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        break;
    }
    case VTSS_PHY_FAMILY_LUTON:
    case VTSS_PHY_FAMILY_LUTON24:
    case VTSS_PHY_FAMILY_LUTON_E:
        switch (conf->mac_if) {
        case VTSS_PORT_INTERFACE_INTERNAL:
            /* Register 23 is setup correctly by default */
            break;
        default:
            VTSS_E("port_no %u, MAC interface %s not supported for SparX",
                   port_no, vtss_port_if_txt(conf->mac_if));
            return VTSS_RC_ERROR;
        }
        break;

    case VTSS_PHY_FAMILY_LUTON26:
        //  There is logic that was attempting to keep the SMI from interfering with the micro access to token-ring registers.
        // We need to disable this logic as it appears to be causing problems. To disable this token-ring access blocking mechanism,
        // set bit 3 of register 28TP (page 0x2a30).
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_28, 0x8 , 0x8));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    // Fall through on purpose
    case VTSS_PHY_FAMILY_ATOM:
        VTSS_RC(vtss_phy_mac_media_if_atom_setup(vtss_state, port_no, conf));
        break;

    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
        VTSS_RC(vtss_phy_mac_media_if_tesla_setup(vtss_state, port_no, conf));
        break;

    case VTSS_PHY_FAMILY_ELISE:
        VTSS_RC(vtss_phy_mac_media_if_elise_setup(vtss_state, port_no, conf));
        break;

    case VTSS_PHY_FAMILY_NONE:
    default:
        if (ps->conf_none) {
            switch (conf->mac_if) {
            case VTSS_PORT_INTERFACE_RGMII:
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_STD_27, 0x848B));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_CONTROL_AND_STATUS_20, 0x0CE2));
                break;
            case VTSS_PORT_INTERFACE_RTBI:
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_STD_27, 0x8489));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_CONTROL_AND_STATUS_20, 0x0CE2));
                break;
            default:
                break;
            }
            break;
        }
        break;
    }

    /* -- Step 3: Reset PHY -- */



    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    VTSS_RC(port_reset(vtss_state, port_no));

    /* -- Step 4: Run startup scripts -- */
    switch (ps->family) {

    case VTSS_PHY_FAMILY_MUSTANG:
        /* TBD */
        break;

    case VTSS_PHY_FAMILY_BLAZER:
        VTSS_RC(vtss_phy_init_seq_blazer(vtss_state, ps, port_no));

        /* Setup MAC interface */
        switch (conf->mac_if) {
        case VTSS_PORT_INTERFACE_MII:
        case VTSS_PORT_INTERFACE_GMII:
            reg = (0 << 12);
            break;
        case VTSS_PORT_INTERFACE_RGMII:
            reg = (1 << 12);
            break;
        case VTSS_PORT_INTERFACE_TBI:
            reg = (2 << 12);
            break;
        case VTSS_PORT_INTERFACE_RTBI:
            reg = (3 << 12);
            break;
        default:
            VTSS_E("port_no %u, MAC interface %s not supported for Blazer",
                   port_no, vtss_port_if_txt(conf->mac_if));
            return VTSS_RC_ERROR;
        }
        reg |= (((ps->type.revision == 6 ? 1 : 0) << 9) | /* 2.5V */
                ((conf->rgmii.rx_clk_skew_ps || conf->rgmii.tx_clk_skew_ps ? 1 : 0) << 8) |
                (0x14 << 0)); /* Reserved */
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL, reg));
        VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_CONTROL_AND_STATUS, 0x3000));
        break;

    case VTSS_PHY_FAMILY_COBRA:
        VTSS_RC(vtss_phy_init_seq_cobra(vtss_state, ps, port_no));
        break;

    case VTSS_PHY_FAMILY_QUATTRO:
        VTSS_RC(vtss_phy_init_seq_quattro(vtss_state, ps, port_no));
        break;

    case VTSS_PHY_FAMILY_SPYDER:
        VTSS_RC(vtss_phy_init_seq_spyder(vtss_state, ps, port_no));
        break;

    case VTSS_PHY_FAMILY_COOPER:
        VTSS_RC(vtss_phy_init_seq_cooper(vtss_state, ps, port_no));
        break;

    case VTSS_PHY_FAMILY_LUTON:
    case VTSS_PHY_FAMILY_LUTON_E:
    case VTSS_PHY_FAMILY_LUTON24:
        VTSS_RC(vtss_phy_init_seq_luton(vtss_state, ps, port_no));
        break;

    case VTSS_PHY_FAMILY_LUTON26:
        VTSS_RC(vtss_phy_init_seq_atom(vtss_state, ps, port_no, TRUE));

#if defined(VTSS_FEATURE_EEE)
        vtss_state->phy_state[port_no].eee_conf.eee_mode = EEE_REG_UPDATE;
        VTSS_RC(vtss_phy_eee_ena_private(vtss_state, port_no));
#endif
        break;
    case VTSS_PHY_FAMILY_ATOM:
        VTSS_RC(vtss_phy_init_seq_atom(vtss_state, ps, port_no, FALSE));

#if defined(VTSS_FEATURE_EEE)
        vtss_state->phy_state[port_no].eee_conf.eee_mode = EEE_REG_UPDATE;
        VTSS_RC(vtss_phy_eee_ena_private(vtss_state, port_no));
#endif
        break;
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
#if defined(VTSS_FEATURE_EEE)
        vtss_state->phy_state[port_no].eee_conf.eee_mode = EEE_REG_UPDATE;
        VTSS_RC(vtss_phy_eee_ena_private(vtss_state, port_no));
#endif
        // No init seq defined yet.
        break;
    case VTSS_PHY_FAMILY_ENZO:
        VTSS_RC(vtss_phy_init_seq_enzo(vtss_state, ps, port_no));
        break;

    case VTSS_PHY_FAMILY_NONE:
    default:
        break;
    }
    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_conf_set_private(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps         = &vtss_state->phy_state[port_no];
    vtss_phy_conf_t       *conf       = &ps->setup;
    vtss_phy_reset_conf_t *reset_conf = &ps->reset;
    u16                   reg, revision;
    vtss_phy_family_t     family;

    /* Save setup */
    VTSS_D("enter, port_no: %u", port_no);
    family = ps->family;
    revision = ps->type.revision;

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    switch (conf->mode) {
    case VTSS_PHY_MODE_ANEG:
        /* Setup register 4 */
        reg = (((conf->aneg.asymmetric_pause ? 1 : 0) << 11) |
               ((conf->aneg.symmetric_pause ? 1 : 0) << 10) |
               ((conf->aneg.speed_100m_fdx ? 1 : 0) << 8) |
               ((conf->aneg.speed_100m_hdx ? 1 : 0) << 7) |
               ((conf->aneg.speed_10m_fdx ? 1 : 0) << 6) |
               ((conf->aneg.speed_10m_hdx ? 1 : 0) << 5) |
               (1 << 0));
        VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_DEVICE_AUTONEG_ADVERTISEMENT, reg));

        /* Setup register 9 */
        reg = 0;
        if (conf->aneg.speed_1g_fdx) {
            reg |= VTSS_PHY_1000BASE_T_CONTROL_1000BASE_T_FDX_CAPABILITY;
        }

        if (conf->aneg.speed_1g_hdx) {
            reg |= VTSS_PHY_1000BASE_T_CONTROL_1000BASE_T_HDX_CAPABILITY;
        }
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_1000BASE_T_CONTROL, reg,
                                        VTSS_PHY_1000BASE_T_CONTROL_1000BASE_T_FDX_CAPABILITY | VTSS_PHY_1000BASE_T_CONTROL_1000BASE_T_HDX_CAPABILITY));

        switch (family) {
        case VTSS_PHY_FAMILY_COOPER:
            /* BZ 2528 - Part 2/2
             * Solution for Interop (slow link-up) issue with Marvell PHYs */
            VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200)); //Ensure RClk125 enabled even in powerdown
            VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xA7F8));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0000, 0x0018));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0, 0));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87F8));
            VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200)); //Restore RClk125 gating
            VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
            break;

        case VTSS_PHY_FAMILY_ATOM:
        case VTSS_PHY_FAMILY_LUTON26:
        case VTSS_PHY_FAMILY_TESLA:
        case VTSS_PHY_FAMILY_VIPER:
        case VTSS_PHY_FAMILY_ELISE:
            VTSS_RC(vtss_phy_set_private_atom(vtss_state, port_no, conf->mode));
            break;

        default:
            break;
        }

        /* Use register 0 to restart auto negotiation */
        // Only restart auton-neg at warm start if something has changed
        if (!vtss_state->sync_calling_private || ps->warm_start_reg_changed) {
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_MODE_CONTROL,
                                     VTSS_F_PHY_MODE_CONTROL_AUTO_NEG_ENA | VTSS_F_PHY_MODE_CONTROL_RESTART_AUTO_NEG));
            VTSS_D("VTSS_PHY_MODE_FORCED, port:%d", port_no);

        } else {
            // If warm start then don't restart auto-negotiation.
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_MODE_CONTROL,
                                     VTSS_F_PHY_MODE_CONTROL_AUTO_NEG_ENA));
        }
        VTSS_D("VTSS_PHY_MODE_ANEG, port:%d, reg:0x%X", port_no, reg);
        break;
    case VTSS_PHY_MODE_FORCED:
        /* Setup register 0 */
        reg = (((conf->forced.speed == VTSS_SPEED_100M ? 1 : 0) << 13) | (0 << 12) |
               ((conf->forced.fdx ? 1 : 0) << 8) |
               ((conf->forced.speed == VTSS_SPEED_1G ? 1 : 0) << 6));
        VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, reg));
        VTSS_D("VTSS_PHY_MODE_FORCED, port:%d, reg:0x%X", port_no, reg);
        if (conf->forced.speed != VTSS_SPEED_1G) {
            /* Enable Auto MDI/MDI-X in forced 10/100 mode */
            switch (family) {
            case VTSS_PHY_FAMILY_QUATTRO:
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
                VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0012));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x2803));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87fa));
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                break;
            case VTSS_PHY_FAMILY_LUTON:
                if (revision == 0) {
                    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
                    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
                    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0012));
                    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x2803));
                    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87fa));
                    VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));
                    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                } else {
                    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, 0x0000, 0x0080));
                }
                break;
            case VTSS_PHY_FAMILY_SPYDER:
            case VTSS_PHY_FAMILY_LUTON_E:
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, 0x0000, 0x0080));
                break;
            case VTSS_PHY_FAMILY_LUTON24:
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
                VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0x0092));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x2803));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87FA));
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                break;
            case VTSS_PHY_FAMILY_COOPER:
                /* BZ 2528 - Part 1/2
                 * Solution for Interop (slow link-up) issue with Marvell PHYs */
                if (conf->forced.speed == VTSS_SPEED_100M) {
                    //Note: no need to ungate/gate RClk125 here since known to be in forced-100
                    VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
                    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0xA7F8));
                    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_17, 0x0018, 0x0018));
                    VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_18, 0, 0));
                    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_16, 0x87F8));
                    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                }
                break;

            case VTSS_PHY_FAMILY_COBRA:
                // Work around bugzilla8953 - Port is not coming UP without auto negotiation
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0200, 0x0200));
                VTSS_RC(vtss_phy_page_tr(vtss_state, port_no));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_2, 0x0012));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_1, 0x2803));
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_TR_0, 0x87fa));
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_8, 0x0000, 0x0200));
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                break;
            case VTSS_PHY_FAMILY_ENZO:
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_BYPASS_CONTROL, 0x0000, 0x0080));
                break;

            case VTSS_PHY_FAMILY_ATOM:
            case VTSS_PHY_FAMILY_LUTON26:
            case VTSS_PHY_FAMILY_TESLA:
            case VTSS_PHY_FAMILY_VIPER:
            case VTSS_PHY_FAMILY_ELISE:
                VTSS_RC(vtss_phy_set_private_atom(vtss_state, port_no, conf->mode));
                break;

            default:
                break;
            }
        }
        break;
    case VTSS_PHY_MODE_POWER_DOWN:
        /* Setup register 0 */
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, VTSS_F_PHY_MODE_CONTROL_POWER_DOWN, VTSS_F_PHY_MODE_CONTROL_POWER_DOWN));

        // Work-around for bugzilla#12650 - Link status shows link up even when PHY is powered down
        if (family == VTSS_PHY_FAMILY_QUATTRO || family == VTSS_PHY_FAMILY_COBRA) {
            /* Briefly power up-down to make sure the link status bit clears */
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, !VTSS_F_PHY_MODE_CONTROL_POWER_DOWN));
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, VTSS_F_PHY_MODE_CONTROL_POWER_DOWN));
        }

        break;
    default:
        VTSS_E("port_no %u, unknown mode %d", port_no, conf->mode);
        return VTSS_RC_ERROR;
    }


    if (family == VTSS_PHY_FAMILY_COBRA) {
        VTSS_N("Port:%d  Bugzilla#7489 - Interrupt must be disable when powered down.", port_no);
        VTSS_RC(vtss_phy_event_enable_private(vtss_state, port_no));
    }

    VTSS_RC(vtss_phy_mdi_setup(vtss_state, port_no));

    if (reset_conf->media_if == VTSS_PHY_MEDIA_IF_SFP_PASSTHRU) {
        VTSS_RC(vtss_phy_pass_through_speed_mode(vtss_state, port_no));
    }
    return VTSS_RC_OK;
}

// Function for enabling/disabling squelch work around.
// In : port_no - Any phy port with the chip
//    : enable  - TRUE = enable squelch workaround, FALSE = Disable squelch workaround
// Return - VTSS_RC_OK - Workaround was enabled/disable. VTSS_RC_ERROR - Squelch workaround patch not loaded
static vtss_rc squelch_workaround_private(vtss_state_t *vtss_state,
                                          vtss_port_no_t port_no, BOOL enable)
{
    u16 reg_value;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];

    switch (ps->family) {
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
        //Enable workaround by writing command to 18G
        //Command 18G instruction
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));   // switch to micro page (glabal to all 12 PHYs)
        if (enable) {
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x801d));
        } else {
            VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, 0x800d));
        }
        //After writing to register 18G wait for bit 15 to equal 0.
        VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

        // Check bit register 18G bit 14, if equal 1 there was an error.
        // If there was an error then most likely the squelch patch is not loaded.
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));   // switch to micro page (glabal to all 12 PHYs)
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg_value));
        if (reg_value & (1 << 14)) {
            VTSS_E("squelch workaround not loaded");
            return VTSS_RC_ERROR;
        }
        VTSS_I("squelch_workaround setup reg_value = 0x%X, enable:%d", reg_value, enable);
        break;
    default:
        break;
    }
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}

// Function for checking if 1588 registers are ready for new accesses. NOTE: **** Page register is left at 0x1588, and not return to standard page  ****
// In - port_no : Any phy port with the chip
static vtss_rc vtss_phy_wait_for_1588_command_busy(vtss_state_t *vtss_state,
                                                   const vtss_port_no_t      port_no)
{
    u16 val;
    u8 timeout = 255; // For making sure that we don't get stucked
    VTSS_RC(vtss_phy_page_1588(vtss_state, port_no));

    // Wait for bit 15 to be set (or timeout)
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_1588_16, &val));
    while (!(val & VTSS_PHY_F_PAGE_1588_16_CMD_BIT) && timeout)  {
        timeout--;
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_1588_16, &val));
    }

    if (timeout == 0) {
        VTSS_E("Unexpected timeout");
        return VTSS_RC_ERROR;
    } else {
        return VTSS_RC_OK;
    }
}

// Function for checking if MACSEC registers are ready for new accesses. NOTE: **** Page register is left at 0xMACSEC, and not return to standard page  ****
// In - port_no : Any phy port with the chip
static vtss_rc vtss_phy_wait_for_macsec_command_busy(vtss_state_t *vtss_state, const vtss_port_no_t  port_no, u32 page)
{
    u16 val;
    u8 timeout = 255; // For making sure that we don't get stucked
    VTSS_RC(vtss_phy_page_macsec(vtss_state, port_no));

    // Wait for bit 15 to be set (or timeout)
    if (page == 19) {
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_19, &val));
        while (!(val & VTSS_PHY_F_PAGE_MACSEC_19_CMD_BIT) && timeout)  {
            timeout--;
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_19, &val));
        }
    } else if (page == 20) {
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_20, &val));
        while (!(VTSS_PHY_F_PAGE_MACSEC_20_READY(val) == 0) && timeout)  {
            timeout--;
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_20, &val));
        }
    }

    if (timeout == 0) {
        VTSS_E("Unexpected timeout when accesing port:%d page:%d", port_no, page);
        return VTSS_RC_ERROR;
    } else {
        return VTSS_RC_OK;
    }
}

// See vtss_phy_csr_wr
vtss_rc vtss_phy_1588_csr_wr_private(vtss_state_t         *vtss_state,
                                     const vtss_port_no_t port_no,
                                     const u16            target,
                                     const u32            csr_reg_addr,
                                     const u32            value)
{

    /* Divide the 32 bit value to [15..0] Bits & [31..16] Bits */
    u16 reg_value_lower = (value & 0xffff);
    u16 reg_value_upper = (value >> 16);

    VTSS_RC(vtss_phy_wait_for_1588_command_busy(vtss_state, port_no)); // Wait for 1588 register access
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_1588_16,
                        VTSS_PHY_F_PAGE_1588_16_CMD_BIT | VTSS_PHY_F_PAGE_1588_16_TARGET(target) |  VTSS_PHY_F_PAGE_1588_16_CSR_REG_ADDR(csr_reg_addr)));
    VTSS_RC(vtss_phy_wait_for_1588_command_busy(vtss_state, port_no)); // Wait for 1588 register access
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_1588_CSR_DATA_LSB, reg_value_lower));

    VTSS_RC(vtss_phy_wait_for_1588_command_busy(vtss_state, port_no)); // Wait for 1588 register access
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_1588_CSR_DATA_MSB, reg_value_upper));

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}

// See vtss_phy_csr_rd
vtss_rc vtss_phy_1588_csr_rd_private(vtss_state_t         *vtss_state,
                                     const vtss_port_no_t port_no,
                                     const u16            target,
                                     const u32            csr_reg_addr,
                                     u32                  *value)
{

    u16 reg_value_lower;
    u16 reg_value_upper;

    VTSS_RC(vtss_phy_wait_for_1588_command_busy(vtss_state, port_no)); // Wait for 1588 register access
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_1588_16,
                        VTSS_PHY_F_PAGE_1588_16_CMD_BIT | VTSS_PHY_F_PAGE_1588_16_TARGET(target) |
                        VTSS_PHY_F_PAGE_1588_16_READ    | VTSS_PHY_F_PAGE_1588_16_CSR_REG_ADDR(csr_reg_addr)));

    VTSS_RC(vtss_phy_wait_for_1588_command_busy(vtss_state, port_no)); // Wait for 1588 register access
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_1588_CSR_DATA_LSB, &reg_value_lower));

    VTSS_RC(vtss_phy_wait_for_1588_command_busy(vtss_state, port_no)); // Wait for 1588 register access
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_1588_CSR_DATA_MSB, &reg_value_upper));

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    *value = (reg_value_upper << 16) | reg_value_lower;
    return VTSS_RC_OK;
}

// See vtss_phy_csr_wr
vtss_rc vtss_phy_macsec_csr_wr_private(vtss_state_t         *vtss_state,
                                       const vtss_port_no_t port_no,
                                       const u16            target,
                                       const u32            csr_reg_addr,
                                       const u32            value)
{

    /* Divide the 32 bit value to [15..0] Bits & [31..16] Bits */
    u16 reg_value_lower = (value & 0xffff);
    u16 reg_value_upper = (value >> 16);
    u32 target_tmp = 0;
    //    u16 val;

    // The only ones not accessible in non-MACsec devices are the MACsec ingress and egress blocks at 0x38 and 0x3C (for each port).
    // Everything else is accessible using the so-called macsec_csr_wr/rd functions using registers 17-20 in extended page 4 (as described in PS1046).
    if (!vtss_phy_can(vtss_state, port_no, VTSS_CAP_MACSEC) && (target == 0x38 || target == 0x3C)) {
        VTSS_E("Port:%d, MACSEC to phy without MACSEC support", port_no);
        return VTSS_RC_ERROR;
    }
    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(vtss_state, port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_20, VTSS_PHY_F_PAGE_MACSEC_20_TARGET((target >> 2))));

    //    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_20, &val));
    //    printf("reg20:%x\n",val);

    if (target >> 2 == 1 || target >> 2 == 3) {
        target_tmp = target; // non-macsec access
    }
    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(vtss_state, port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_CSR_DATA_LSB, reg_value_lower));
    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(vtss_state, port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_CSR_DATA_MSB, reg_value_upper));
    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(vtss_state, port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_19,  VTSS_PHY_F_PAGE_MACSEC_19_CMD_BIT |
                        VTSS_PHY_F_PAGE_MACSEC_19_TARGET(target_tmp) | VTSS_PHY_F_PAGE_MACSEC_19_CSR_REG_ADDR(csr_reg_addr)));


    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}

// See vtss_phy_csr_rd
vtss_rc vtss_phy_macsec_csr_rd_private(vtss_state_t         *vtss_state,
                                       const vtss_port_no_t port_no,
                                       const u16            target,
                                       const u32            csr_reg_addr,
                                       u32                  *value)
{
    u16 reg_value_lower;
    u16 reg_value_upper;
    u32 target_tmp = 0;

    if (!vtss_phy_can(vtss_state, port_no, VTSS_CAP_MACSEC) && (target == 0x38 || target == 0x3C)) {
        VTSS_E("Port:%d, MACSEC to phy without MACSEC support, target:0x%X", port_no, target);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(vtss_state, port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_20, VTSS_PHY_F_PAGE_MACSEC_20_TARGET((target >> 2))));

    if (target >> 2 == 1) {
        target_tmp = target & 3; // non-macsec access
    }
    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(vtss_state, port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_19,
                        VTSS_PHY_F_PAGE_MACSEC_19_CMD_BIT | VTSS_PHY_F_PAGE_MACSEC_19_TARGET(target_tmp) |
                        VTSS_PHY_F_PAGE_MACSEC_19_READ    | VTSS_PHY_F_PAGE_MACSEC_19_CSR_REG_ADDR(csr_reg_addr)));

    VTSS_RC(vtss_phy_wait_for_macsec_command_busy(vtss_state, port_no, 19)); // Wait for MACSEC register access
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_CSR_DATA_LSB, &reg_value_lower));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_PAGE_MACSEC_CSR_DATA_MSB, &reg_value_upper));
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    *value = (reg_value_upper << 16) | reg_value_lower;
    return VTSS_RC_OK;
}

// Function for determining the type of flow control
// In: port_no - The port in question.
//     lp_auto_neg_advertisment_reg - The value from the register containing the Link partners auto negotiation advertisement (Standard page 5)
//     phy_setup - The PHY configuration
// In/out:   Status  - Pointer to where to put the result
static void vtss_phy_flowcontrol_decode_status_private(vtss_port_no_t port_no, u16 lp_auto_neg_advertisment_reg, const vtss_phy_conf_t phy_setup, vtss_port_status_t *const status)
{
    BOOL                  sym_pause, asym_pause, lp_sym_pause, lp_asym_pause;
    sym_pause = phy_setup.aneg.symmetric_pause;
    asym_pause = phy_setup.aneg.asymmetric_pause;
    lp_sym_pause = (lp_auto_neg_advertisment_reg & (1 << 10) ? 1 : 0);
    lp_asym_pause = (lp_auto_neg_advertisment_reg & (1 << 11) ? 1 : 0);
    status->aneg.obey_pause =
        (sym_pause && (lp_sym_pause || (asym_pause && lp_asym_pause)) ? 1 : 0);
    status->aneg.generate_pause =
        (lp_sym_pause && (sym_pause || (asym_pause && lp_asym_pause)) ? 1 : 0);
    VTSS_N("port:%u, status->aneg.generate_pause:%d, status->aneg.generate_pause:%d, lp_asym_pause:%d, lp_sym_pause:%d, sym_pause:%d, asym_pause:%d",
           port_no, status->aneg.generate_pause, status->aneg.generate_pause, lp_asym_pause, lp_sym_pause, sym_pause, asym_pause);
}


// Function for determining the link speed
// In: port_no - The port in question.
//     lp_1000base_t_status_reg - The value from the register containing the Link partners 1000BASE-T Status (Standard page 10)
//     mii_status_reg - The value from the register containing mii status (Standard page 1)
//     phy_setup - The PHY configuration
// In/out:   Status  - Pointer to where to put the result
static void vtss_phy_link_speeed_decode_status(vtss_port_no_t port_no, u16 lp_1000base_t_status_reg, u16 mii_status_reg, const vtss_phy_conf_t phy_setup, vtss_port_status_t *const status)
{
    if (phy_setup.aneg.speed_1g_fdx &&  /* 1G fdx advertised */
        (lp_1000base_t_status_reg & (1 << 15)) == 0 &&     /* No master/slave fault */
        (lp_1000base_t_status_reg & (1 << 11))) {          /* 1G fdx advertised by LP */
        status->speed = VTSS_SPEED_1G;
        status->fdx = 1;
    } else if (phy_setup.aneg.speed_100m_fdx && /* 100M fdx advertised */
               (mii_status_reg & (1 << 8))) {              /* 100M fdx advertised by LP */
        status->speed = VTSS_SPEED_100M;
        status->fdx = 1;
    } else if (phy_setup.aneg.speed_100m_hdx && /* 100M hdx advertised */
               (mii_status_reg & (1 << 7))) {              /* 100M hdx advertised by LP */
        status->speed = VTSS_SPEED_100M;
        status->fdx = 0;
    } else if (phy_setup.aneg.speed_10m_fdx &&  /* 10M fdx advertised */
               (mii_status_reg & (1 << 6))) {              /* 10M fdx advertised by LP */
        status->speed = VTSS_SPEED_10M;
        status->fdx = 1;
    } else if (phy_setup.aneg.speed_10m_hdx &&  /* 10M hdx advertised */
               (mii_status_reg & (1 << 5))) {              /* 10M hdx advertised by LP */
        status->speed = VTSS_SPEED_10M;
        status->fdx = 0;
    } else {
        status->speed = VTSS_SPEED_UNDEFINED;
        status->fdx = 0;
    }
    VTSS_N("port:%d, lp_1000base_t_status_reg:0x%X, mii_status_reg:0x%X, speed:%d",
           port_no, lp_1000base_t_status_reg, mii_status_reg, status->speed);
}


static void vtss_phy_decode_status_reg(vtss_port_no_t port_no, u16 mii_status_reg, vtss_port_status_t *const status)
{
    // Link up/down
    status->link                   = (mii_status_reg & (1 << 2) ? TRUE : FALSE);
    status->link_down              = (mii_status_reg & (1 << 2) ? FALSE : TRUE);
    status->aneg_complete          = (mii_status_reg & (1 << 5) ? TRUE : FALSE);
    status->remote_fault           = (mii_status_reg & (1 << 4) ? TRUE : FALSE);
    status->unidirectional_ability = ((mii_status_reg & VTSS_F_PHY_UNIDIRECTIONAL_ABILITY) ? TRUE : FALSE);
}

// Function for determining status from PHY registers
// In: port_no - The port in question.
//     lp_1000base_t_status_reg - The value from the register containing the Link partners 1000BASE-T Status (Standard page 10)
//     mii_status_reg - The value from the register containing mii status (Standard page 1)
//     lp_auto_neg_advertisment_reg - The value from the register containing the Link partners auto negotiation advertisement (Standard page 5)
// In/out:   Status  - Pointer to where to put the result
void vtss_phy_reg_decode_status(vtss_port_no_t port_no, u16 lp_auto_neg_advertisment_reg, u16 lp_1000base_t_status_reg, u16 mii_status_reg, const vtss_phy_conf_t phy_setup, vtss_port_status_t *const status)
{
    vtss_phy_flowcontrol_decode_status_private(port_no, lp_auto_neg_advertisment_reg, phy_setup, status); // Flow control
    vtss_phy_link_speeed_decode_status(port_no, lp_1000base_t_status_reg, mii_status_reg, phy_setup, status); // Speed + Duplex

    // Link up/down
    vtss_phy_decode_status_reg(port_no, mii_status_reg, status);
}

vtss_rc vtss_phy_status_get_private(vtss_state_t *vtss_state,
                                    const vtss_port_no_t port_no,
                                    vtss_port_status_t   *const status)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    u16                   reg, reg10;
    u16                   revision;
    vtss_phy_reset_conf_t *conf = &ps->reset;
    revision = ps->type.revision;

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    VTSS_N("vtss_phy_status_get_private, port_no: %u", port_no);

    /* Read link status from register 1 */
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MODE_STATUS, &reg));

    // Check if link has been down due to port has been reset
    if (ps->link_down_due_to_port_reset) {
        status->link_down = TRUE; // Link due the port reset
        VTSS_D("Setting link_down_due_to_port_reset, port_no = %d", port_no);
        ps->link_down_due_to_port_reset = FALSE; // OK - Link down is now detected by the Application.
    } else {
        status->link_down = (reg & (1 << 2) ? 0 : 1);
    }
    vtss_phy_decode_status_reg(port_no, reg, status);

    if (status->link_down) {
        /* Read status again if link down (latch low field) */
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MODE_STATUS, &reg));
        status->link = (reg & (1 << 2) ? 1 : 0);
        status->link_down = (reg & (1 << 2) ? 0 : 1);
        VTSS_N("status->link = %d, port = %d, reg = 0x%X", status->link, port_no, reg);
    } else {
        status->link = 1;
    }

    if (ps->status.link != status->link) {
        VTSS_I("Link change ps->status.link=%d, status->link=%d, port_no = %d, status->link_down =%d, ps->setup.mode =%d", ps->status.link, status->link, port_no, status->link_down, ps->setup.mode);
    }

    if (status->link) {
        switch (ps->setup.mode) {
        case VTSS_PHY_MODE_ANEG:
            if ((reg & (1 << 5)) == 0) {
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_AUXILIARY_CONTROL_AND_STATUS, &reg));
                /* Workaround to have SW aware 100FX link up state for VSC8664*/
                if ((reg & 0x1b) != 0xa) {
                    /* Auto negotiation not complete, link considered down */
                    status->link = 0;
                } else {
                    status->speed = VTSS_SPEED_100M;
                    status->fdx = 1;
                }
                break;
            }

            /* Use register 5 to determine flow control result */
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_AUTONEGOTIATION_LINK_PARTNER_ABILITY, &reg));
            vtss_phy_flowcontrol_decode_status_private(port_no, reg, ps->setup, status); // Decode type of flow control

            if (ps->family == VTSS_PHY_FAMILY_NONE) {
                /* Standard PHY, use register 10 and 5 to determine speed/duplex */
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_1000BASE_T_STATUS, &reg10));
                vtss_phy_link_speeed_decode_status(port_no, reg10, reg, ps->setup, status);
            } else {
                /* Vitesse PHY, use register 28 to determine speed/duplex */
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_AUXILIARY_CONTROL_AND_STATUS, &reg));
                switch ((reg >> 3) & 0x3) {
                case 0:
                    status->speed = VTSS_SPEED_10M;
                    break;
                case 1:
                    status->speed = VTSS_SPEED_100M;
                    break;
                case 2:
                    status->speed = VTSS_SPEED_1G;
                    break;
                case 3:
                    status->speed = VTSS_SPEED_UNDEFINED;
                    break;
                }
                status->fdx = (reg & (1 << 5) ? 1 : 0);
            }
            break;
        case VTSS_PHY_MODE_FORCED:
            status->speed = ps->setup.forced.speed;
            status->fdx = ps->setup.forced.fdx;
            break;
        case VTSS_PHY_MODE_POWER_DOWN:
            break;
        default:
            VTSS_E("port_no %u, unknown mode %d", port_no, ps->setup.mode);
            return VTSS_RC_ERROR;
        }
    }

    /* Handle link down event */
    if ((!status->link || status->link_down) && ps->status.link) {
        VTSS_D("link down event on port_no %u, status->link = %d, status->link_down = %d, ps->status.link =%d",
               port_no, status->link, status->link_down, ps->status.link);

        switch (ps->family) {
        case VTSS_PHY_FAMILY_QUATTRO:
        case VTSS_PHY_FAMILY_COBRA:
            /* BZ ? */
            VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, &reg));
            if ((reg & (1 << 11)) == 0) {
                /* Briefly power down PHY to avoid problem sometimes seen when
                   changing speed on SmartBit */
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, reg | (1 << 11)));
                status->link = 0;
                VTSS_MSLEEP(100);
                VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, reg));
            }
            /*lint -e(616) */ /* Fall through OK. */
        case VTSS_PHY_FAMILY_SPYDER:
        case VTSS_PHY_FAMILY_LUTON:
        case VTSS_PHY_FAMILY_LUTON_E:
        case VTSS_PHY_FAMILY_LUTON24:
            VTSS_RC(vtss_phy_optimize_receiver_init(vtss_state, port_no));

            if (ps->family != VTSS_PHY_FAMILY_QUATTRO) {
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear MDI Impedance Force Bit - Safety  */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
            }

            if (ps->family == VTSS_PHY_FAMILY_LUTON_E) {
                /*- Raise the 10-Base T squelch control to eliminate */
                /*- link-up deadlocks. Observed in 7395 parts so far */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_CONTROL_AND_STATUS, 0x0800, 0x0c00));
            }
            break;

        case VTSS_PHY_FAMILY_ATOM:
        case VTSS_PHY_FAMILY_LUTON26:
            // Work-around for bugzilla#5050/#5568 - Problem with going from 10/100 mode to 1000 auto-neg
            if (revision == VTSS_PHY_ATOM_REV_B || revision == VTSS_PHY_ATOM_REV_A) {
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, &reg));
                if (!(reg & VTSS_F_PHY_MODE_CONTROL_POWER_DOWN)) {
                    // pt. This doesn't work for SFP ports
                    if (conf->media_if == VTSS_PHY_MEDIA_IF_CU && ps->setup.mode == VTSS_PHY_MODE_ANEG) {
                        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL,
                                                   VTSS_F_PHY_MODE_CONTROL_POWER_DOWN,
                                                   VTSS_F_PHY_MODE_CONTROL_POWER_DOWN));
                        VTSS_D("Work-around for bugzilla#5050/#5568, conf->media_if = %d, port_no =%d", conf->media_if, port_no);
                        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, 0, VTSS_F_PHY_MODE_CONTROL_POWER_DOWN));
                    }
                }
            }

            // Work-around for bugzilla#8381 - Link not coming up if disconnected while transmitting (only in 10 Mbit forced mode).
            //We agree that the issue is still present and recommend the following workaround for bugzilla 8381:
            //When the link drops in any copper PHY speed, read MII register 1 and if the link status is still down, do the following:
            //If in forced 10BASE-T mode, momentarily force 10BASE-T link status (after you are sure the MAC is no longer transmitting to the PHY) and then unforce 10BASE-T link status using MII register 22, bit 15 toggling 1/0.
            //    Else momentarily force power-down using MII register 0, bit 11 toggling 1/0.
            // Note that if link status comes back up, the workaround wasnt required.  The only remaining danger is if link status just came up after reading link status and before power-down.  This is unlikely and unlikely to repeat.
            VTSS_D("Mode:%d, speed:%d", ps->setup.mode, ps->setup.forced.speed);
            if (ps->setup.mode == VTSS_PHY_MODE_FORCED) {
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MODE_STATUS, &reg));
                VTSS_N("reg:0x%X", reg);
                if (reg & VTSS_F_PHY_STATUS_LINK_STATUS) {
                    VTSS_I("Link is up, port:%d", port_no);
                } else {
                    if (ps->setup.forced.speed == VTSS_SPEED_10M) {
                        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_CONTROL_AND_STATUS,
                                                   VTSS_PHY_EXTENDED_CONTROL_AND_STATUS_FORCE_10BASE_T_HIGH,
                                                   VTSS_PHY_EXTENDED_CONTROL_AND_STATUS_FORCE_10BASE_T_HIGH));

                        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_CONTROL_AND_STATUS,
                                                   0,
                                                   VTSS_PHY_EXTENDED_CONTROL_AND_STATUS_FORCE_10BASE_T_HIGH));
                        VTSS_MSLEEP(20); // Make sure that status register is updated before we clear it (20 ms chosen due to link pulse interval timer).
                        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MODE_STATUS, &reg)); // Clear the status register, to get rid of the false link up, due to that we have just forced the link up/down.
                        VTSS_D("Bugzilla#8381 work-around 10 MBit force high - port_no:%d reg:0x%X", port_no, reg);
                    } else {
                        VTSS_D("port_no:%d power down/up", port_no);
                        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL,
                                                   VTSS_F_PHY_MODE_CONTROL_POWER_DOWN,
                                                   VTSS_F_PHY_MODE_CONTROL_POWER_DOWN));
                        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, 0, VTSS_F_PHY_MODE_CONTROL_POWER_DOWN));
                    }
                }
            }



            // Set optimize
            VTSS_RC(vtss_phy_optimize_receiver_init(vtss_state, port_no));
            break;

        case VTSS_PHY_FAMILY_TESLA:
        case VTSS_PHY_FAMILY_VIPER:
        case VTSS_PHY_FAMILY_ELISE:
        case VTSS_PHY_FAMILY_MUSTANG:
        case VTSS_PHY_FAMILY_BLAZER:
        case VTSS_PHY_FAMILY_NONE:
        default:
            break;
        }

    }

    /* Handle link up event */
    if (status->link && (!ps->status.link || status->link_down)) {
        VTSS_D("link up event on port_no %u", port_no);

        switch (ps->family) {

        case VTSS_PHY_FAMILY_QUATTRO:
            if (status->speed == VTSS_SPEED_1G) {
                VTSS_RC(vtss_phy_optimize_receiver_reconfig(vtss_state, port_no));
            }
            break;

        case VTSS_PHY_FAMILY_SPYDER:
            if (status->speed == VTSS_SPEED_1G) {
                VTSS_RC(vtss_phy_optimize_receiver_reconfig(vtss_state, port_no));
            } else if (status->speed == VTSS_SPEED_100M) {
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear Force Bit */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x2000, 0x6000));  /*- MDI Impedance offset -1 */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_2, 0xa004, 0xe00e));   /*- 10/100 Edge Rate/Amplitude Control */
            } else if (status->speed == VTSS_SPEED_10M) {
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear Force Bit */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x6000, 0x6000));  /*- MDI Impedance Offset = +1 */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
            }
            break;

        case VTSS_PHY_FAMILY_LUTON:
            if (status->speed == VTSS_SPEED_1G) {
                VTSS_RC(vtss_phy_optimize_receiver_reconfig(vtss_state, port_no));
            } else if (status->speed == VTSS_SPEED_100M) {
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear Force Bit */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x6000));  /*- MDI Impedance offset default */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                /*- MII 24 can be 0xa000(0xe000), if customer has 100-BT rise time issues */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_2, 0x8002, 0xe00e));   /*- 10/100 Edge Rate/Amplitude Control */
            } else if (status->speed == VTSS_SPEED_10M) {
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear Force Bit */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x2000, 0x6000));  /*- MDI Impedance Offset = -1 */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
            }
            break;

        case VTSS_PHY_FAMILY_LUTON_E:
            if (status->speed == VTSS_SPEED_1G) {
                VTSS_RC(vtss_phy_optimize_receiver_reconfig(vtss_state, port_no));
                VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_3, 0x2000, 0xe000));  /*- 1000-BT DAC Amplitude control = +2% */
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_24, 0x0030, 0x0038));  /*- 1000-BT Edge Rate Control */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear Force Bit */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x4000, 0x6000));  /*- MDI Impedance Offset = +2 */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
            } else if (status->speed == VTSS_SPEED_100M) {
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear Force Bit */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x2000, 0x6000));  /*- MDI Impedance Offset = -1 */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                /*- MII 24 can be 0xa000(0xe000), if customer has 100-BT rise time issues */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_2, 0x8002, 0xe00e));   /*- 10/100 Edge Rate/Amplitude Control */
            } else if (status->speed == VTSS_SPEED_10M) {
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear Force Bit */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x2000, 0x6000));   /*- MDI Impedance Offset = -1 */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                /*- Restore the 10-Base T squelch control when link comes up in 10M*/
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_CONTROL_AND_STATUS, 0x0000, 0x0c00));
            }
            break;

        case VTSS_PHY_FAMILY_LUTON24:
            if (status->speed == VTSS_SPEED_1G) {
                VTSS_RC(vtss_phy_optimize_receiver_reconfig(vtss_state, port_no));
            } else if (status->speed == VTSS_SPEED_100M) {
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear Force Bit */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x6000));   /*- MDI Impedance offset default */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_2, 0xa002, 0xe00e));    /*- 10/100 Edge Rate/Amplitude Control */
            } else if (status->speed == VTSS_SPEED_10M) {
                VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x0000, 0x1000));  /*- Clear Force Bit */
                VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEST_PAGE_20, 0x2000, 0x6000));   /*- MDI Impedance Offset = -1 */
                VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
            }
            break;

        case VTSS_PHY_FAMILY_COOPER:
        case VTSS_PHY_FAMILY_ATOM:
        case VTSS_PHY_FAMILY_LUTON26:
        case VTSS_PHY_FAMILY_TESLA:
        case VTSS_PHY_FAMILY_VIPER:
            if (status->speed == VTSS_SPEED_1G) {
                VTSS_RC(vtss_phy_optimize_receiver_reconfig(vtss_state, port_no));
            }
            break;

        case VTSS_PHY_FAMILY_MUSTANG:
        case VTSS_PHY_FAMILY_BLAZER:
        case VTSS_PHY_FAMILY_COBRA:
        case VTSS_PHY_FAMILY_NONE:
        default:
            break;
        }

#if defined(VTSS_FEATURE_MACSEC)
        if (vtss_phy_can(vtss_state, port_no, VTSS_CAP_MACSEC)) {
            ps->status = *status; // When booting "vtss_state phy status" is not updated yet, so we need to make sure that status is updated before calling vtss_macsec_speed_conf_priv (We do it all the time, even though it in fact was only needed the first time we come through here).
            VTSS_RC(vtss_macsec_speed_conf_priv(vtss_state, port_no));
        }
#endif
    }

    // Determine if it is a fiber or CU port
    VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_3, &reg));
    status->fiber = (((reg >> 6) & 0x3) == 2 ? TRUE : FALSE);
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    /* Save status */
    ps->status = *status;

    return VTSS_RC_OK;
}



static vtss_rc vtss_phy_power_conf_set_private(vtss_state_t *vtss_state,
                                               const vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_phy_power_conf_t *conf = &ps->conf_power;

    /* Check to see if operation supported on this PHY */
    switch (ps->family) {
    case VTSS_PHY_FAMILY_QUATTRO:
    case VTSS_PHY_FAMILY_LUTON_E:
    case VTSS_PHY_FAMILY_LUTON24:
    case VTSS_PHY_FAMILY_SPYDER:
    case VTSS_PHY_FAMILY_ENZO:
    case VTSS_PHY_FAMILY_COBRA:
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
    case VTSS_PHY_FAMILY_COOPER:
        break;
    default:
        VTSS_E("Power control not supported, PHY family %d", ps->family);
        return VTSS_RC_ERROR;
    }

    switch (conf->mode) {
    case VTSS_PHY_POWER_ACTIPHY:
        /* Restore original power level settings */
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_TEST_PAGE_12, 0x0000, 0xfc00));
        // For now don't restore at warm start if nothing has changed
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED_CHK_MASK(vtss_state, port_no, VTSS_PHY_TEST_PAGE_24, 0x2000, 0x2000, 0x0));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        /* Enable Link down Power savings - aka / "Enhanced ActiPHY" */
        if (ps->family == VTSS_PHY_FAMILY_COOPER) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, COOPER_VTSS_PHY_AUXILIARY_CONTROL_AND_STATUS, 0x0020, 0x0020));
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_AUXILIARY_CONTROL_AND_STATUS,
                                            VTSS_F_PHY_AUXILIARY_CONTROL_AND_STATUS_ACTIPHY_MODE_ENABLE,
                                            VTSS_F_PHY_AUXILIARY_CONTROL_AND_STATUS_ACTIPHY_MODE_ENABLE));
        }
        ps->power.vtss_phy_actiphy_on = 1;
        ps->power.vtss_phy_power_dynamic = 0;
        ps->power.pwrusage = VTSS_PHY_ACTIPHY_PWR;
        break;
    case VTSS_PHY_POWER_DYNAMIC:
        /* Enabled Link-up power savings - algorithm runs at link-up time */
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        if (ps->family == VTSS_PHY_FAMILY_COOPER) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, COOPER_VTSS_PHY_AUXILIARY_CONTROL_AND_STATUS, 0x0000, 0x0020));
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_AUXILIARY_CONTROL_AND_STATUS, 0x0000,
                                            VTSS_F_PHY_AUXILIARY_CONTROL_AND_STATUS_ACTIPHY_MODE_ENABLE));
        }
        ps->power.vtss_phy_actiphy_on = 0;
        ps->power.vtss_phy_power_dynamic = 1;
        break;
    case VTSS_PHY_POWER_ENABLED:
        /* Enable ActiPHY and link-up power savings */
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        if (ps->family == VTSS_PHY_FAMILY_COOPER) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, COOPER_VTSS_PHY_AUXILIARY_CONTROL_AND_STATUS, 0x0020, 0x0020));
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_AUXILIARY_CONTROL_AND_STATUS,
                                            VTSS_F_PHY_AUXILIARY_CONTROL_AND_STATUS_ACTIPHY_MODE_ENABLE,
                                            VTSS_F_PHY_AUXILIARY_CONTROL_AND_STATUS_ACTIPHY_MODE_ENABLE));
        }
        ps->power.vtss_phy_actiphy_on = 1;
        ps->power.vtss_phy_power_dynamic = 1;
        ps->power.pwrusage = VTSS_PHY_ACTIPHY_PWR;
        break;
    case VTSS_PHY_POWER_NOMINAL:
        /* Restore original power level settings */
        VTSS_RC(vtss_phy_page_test(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_TEST_PAGE_12, 0x0000, 0xfc00));
        // For now don't restore at warm start if nothing has changed
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED_CHK_MASK(vtss_state, port_no, VTSS_PHY_TEST_PAGE_24, 0x2000, 0x2000, 0x0));
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        if (ps->family == VTSS_PHY_FAMILY_COOPER) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, COOPER_VTSS_PHY_AUXILIARY_CONTROL_AND_STATUS, 0x0000, 0x0020));
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_AUXILIARY_CONTROL_AND_STATUS, 0x0000,
                                            VTSS_F_PHY_AUXILIARY_CONTROL_AND_STATUS_ACTIPHY_MODE_ENABLE));
        }
        ps->power.vtss_phy_actiphy_on = 0;
        ps->power.vtss_phy_power_dynamic = 0;
        ps->power.pwrusage = VTSS_PHY_LINK_DOWN_PWR ;
        break;
    default:
        break;
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_conf_1g_set_private(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no)
{
    vtss_phy_conf_1g_t *conf = &vtss_state->phy_state[port_no].conf_1g;
    u16                reg, reg_cont = 0, reg_status = 0;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_1000BASE_T_CONTROL, &reg_cont));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_1000BASE_T_STATUS, &reg_status));

    reg  = ((conf->master.cfg ? 1 : 0) << 12) | ((conf->master.val ? 1 : 0) << 11);

    /* Commit 1000-BT Control values */
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_1000BASE_T_CONTROL, reg, 0x1800));

    /* Re-start Auto-Neg if Master/Slave Manual Config/Value changed */
    if ((reg_cont ^ reg) & 0x1800) {
        if ((reg & 0x1000) || (reg_status & 0x8000)) { /* Only start Auto-neg if Manuel is selected or master/slave resolution failed */
            if (!vtss_state->sync_calling_private || ps->warm_start_reg_changed) {
                VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MODE_CONTROL, 0x0200, 0x0200));
            }
        }
    }

    return VTSS_RC_OK;
}

static vtss_rc vtss_phy_status_1g_get_private(vtss_state_t *vtss_state,
                                              const vtss_port_no_t  port_no,
                                              vtss_phy_status_1g_t  *const status)
{
    u16 reg = 0;

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_1000BASE_T_STATUS, &reg));

    status->master_cfg_fault = (reg >> 15) & 1;
    status->master = (reg >> 14) & 1;

    return VTSS_RC_OK;
}

/* Function to set the current micro peek/poke address */
static vtss_rc vtss_phy_set_micro_set_addr_private(vtss_state_t *vtss_state,
                                                   const vtss_port_no_t  port_no,
                                                   const u16             addr)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_phy_type_t       phy_id;
    u16                   reg18g;
    vtss_rc               rc;

    if ((rc = vtss_phy_id_get_private(vtss_state, port_no, &phy_id)) != VTSS_RC_OK) {
        return rc;
    }

    switch (ps->family) {
    case VTSS_PHY_FAMILY_VIPER:
        VTSS_RC(VTSS_PHY_BASE_PORTS_FOUND); // Make sure that base ports are found
        VTSS_RC(vtss_phy_page_ext(vtss_state, phy_id.base_port_no));        // Switch to extended register-page 1
        VTSS_RC(PHY_WR_PAGE(vtss_state, phy_id.base_port_no, VTSS_PHY_VERIPHY_CTRL_REG2, addr));
        VTSS_RC(vtss_phy_page_std(vtss_state, phy_id.base_port_no));        // Switch to standard register-page
        reg18g = 0xc000;
        break;

    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
        if (ps->type.revision > VTSS_PHY_ATOM_REV_A) {
            if (addr & 0x4000) {
                reg18g = 0xd000 | (addr & 0xfff);
            } else {
                reg18g = 0xc000 | (addr & 0xfff);
            }
            break;
        }
    // Let Luton26, Atom12, and Tesla rev. A fall through to error
    default:
        VTSS_E("Micro peek/poke not supported, PHY family %d, revision %u", ps->family, ps->type.revision);
        return VTSS_RC_ERROR;
    }

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, reg18g));  // set micro peek/poke address
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    return VTSS_RC_OK;
}

/* Function to peek a byte from the current micro address */
static vtss_rc vtss_phy_micro_peek_private(vtss_state_t *vtss_state,
                                           const vtss_port_no_t  port_no,
                                           const BOOL            post_increment,
                                           u8                   *peek_byte_ptr)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    u16                   reg18g;

    switch (ps->family) {
    case VTSS_PHY_FAMILY_VIPER:
        break;

    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
        if (ps->type.revision > VTSS_PHY_ATOM_REV_A) {
            break;
        }
    // Let Luton26, Atom12, and Tesla rev. A fall through to error
    default:
        VTSS_E("Micro peek/poke not supported, PHY family %d, revision %u", ps->family, ps->type.revision);
        return VTSS_RC_ERROR;
    }

    // Setup peek command with or without post-increment
    reg18g = (post_increment) ? 0x9007 : 0x8007;

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, reg18g));  // Peek micro memory
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, &reg18g));  // read to get peek'ed byte
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));        // Switch to standard register-page

    if (reg18g & 0x4000) {
        return VTSS_RC_ERROR;
    }

    *peek_byte_ptr = (reg18g >> 4) & 0xff;
    return VTSS_RC_OK;
}

/* Function to poke a byte to the current micro address */
static vtss_rc vtss_phy_micro_poke_private(vtss_state_t *vtss_state,
                                           const vtss_port_no_t  port_no,
                                           const BOOL            post_increment,
                                           const u8              poke_byte)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    u16                   reg18g;

    switch (ps->family) {
    case VTSS_PHY_FAMILY_VIPER:
        break;

    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
        if (ps->type.revision > VTSS_PHY_ATOM_REV_A) {
            break;
        }
    // Let Luton26, Atom12, and Tesla rev. A fall through to error
    default:
        VTSS_E("Micro peek/poke not supported, PHY family %d, revision %u", ps->family, ps->type.revision);
        return VTSS_RC_ERROR;
    }

    // Setup peek command with or without post-increment
    reg18g = ((post_increment) ? 0x9006 : 0x8006) | (((u16)poke_byte) << 4);

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));       // Switch back to micro/GPIO register-page
    VTSS_RC(PHY_WR_PAGE(vtss_state, port_no, VTSS_PHY_MICRO_PAGE, reg18g));  // Poke byte into micro memory
    VTSS_RC(vtss_phy_wait_for_micro_complete(vtss_state, port_no));

    return VTSS_RC_OK;
}

/* Configure 23G/24G (Recovered Clock1/Recovered Clock2) */
static vtss_rc vtss_phy_clock_conf_set_private(vtss_state_t *vtss_state,
                                               const vtss_port_no_t         port_no,
                                               const vtss_phy_recov_clk_t   clock_port)
{
    u16  reg_val = 0;
    vtss_phy_type_t phy_id;
    vtss_phy_clock_conf_t conf;
    vtss_port_no_t        source;
    BOOL                  use_squelch_workaround;
    u16                   save_squelch_ctrl_addr;
    u8                    other_port_save_squelch_ctrl = 0;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];

    VTSS_RC(vtss_phy_id_get_private(vtss_state, port_no, &phy_id));
    VTSS_RC(VTSS_PHY_BASE_PORTS_FOUND); // Make sure that base ports are found
    conf = vtss_state->phy_state[phy_id.base_port_no].clock_conf[clock_port].conf;
    source = vtss_state->phy_state[phy_id.base_port_no].clock_conf[clock_port].source;

    switch (vtss_state->phy_state[port_no].family) {
    // These families support clock recovery, but need a squelch work-around
    case VTSS_PHY_FAMILY_VIPER:
        if (ps->type.revision == VTSS_PHY_VIPER_REV_A) {
            use_squelch_workaround = TRUE;
        } else {
            use_squelch_workaround = FALSE;
        }
        break;

    case VTSS_PHY_FAMILY_TESLA:
        use_squelch_workaround = TRUE;
        break;

    // These families support clock recovery
    case VTSS_PHY_FAMILY_ENZO:
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
        use_squelch_workaround = FALSE;
        break;

    case VTSS_PHY_FAMILY_ELISE:
        // Only a special test chip support clock configuration. The chip detection is determined by register 18E3
        VTSS_RC(vtss_phy_page_ext3(vtss_state, port_no));
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_REVERED_18, &reg_val));
        if ((reg_val & 0x3) != 0x3) {
            return VTSS_RC_ERR_PHY_CLK_CONF_NOT_SUPPORTED;
        }
        use_squelch_workaround = FALSE;
        break;

    default:
        // All other families don't support clock recovery
        if (!vtss_state->sync_calling_private) {
            VTSS_E("clock recovery setup not supported on port %u, Chip family:%s", port_no + 1, vtss_phy_family2txt(vtss_state->phy_state[port_no].family));
        }
        return VTSS_RC_ERR_PHY_CLK_CONF_NOT_SUPPORTED;
    }

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
    if (clock_port == VTSS_PHY_RECOV_CLK1) {
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_RECOVERED_CLOCK_0_CONTROL, &reg_val));
    } else {
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_RECOVERED_CLOCK_1_CONTROL, &reg_val));
    }

    /* return if already disabled and new source is disable (due to the 'weired' disable) */
    if (!(reg_val & 0x8000) && (conf.src == VTSS_PHY_CLK_DISABLED)) {
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        return VTSS_RC_OK;
    }

    switch (conf.src) {
    case VTSS_PHY_SERDES_MEDIA:
        reg_val = 0x8000;
        break;
    case VTSS_PHY_COPPER_MEDIA:
        reg_val = 0x8001;
        break;
    case VTSS_PHY_TCLK_OUT:
        reg_val = 0x8002;
        break;
    case VTSS_PHY_LOCAL_XTAL:
        reg_val = 0x8003;
        break;
    case VTSS_PHY_CLK_DISABLED:
        reg_val = (reg_val + 1) & 0x0001;
        break; /* Know this is weird - have to set media wrong to get clock out disabled */
    default:
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        return VTSS_RC_OK;
    }

    if (vtss_state->phy_state[port_no].family == VTSS_PHY_FAMILY_ENZO) {
        reg_val |= (((source & 3) << 12) | (conf.freq << 8) | (conf.squelch << 4));
    }


    switch (vtss_state->phy_state[port_no].family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
        reg_val |= ((((vtss_phy_chip_port(vtss_state, source) % 12) & 0xF) << 11) | (conf.freq << 8) | (conf.squelch << 4));
        break;
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        reg_val |= ((((vtss_phy_chip_port(vtss_state, source) % 4) & 0xF) << 11) | (conf.freq << 8) | (conf.squelch << 4));
        break;
    default:
        break;
    }

    if (use_squelch_workaround) {
        VTSS_RC(squelch_workaround_private(vtss_state, port_no, FALSE));

        // Set address to save squelch control for the other recovered-clock port
        if (ps->type.revision == VTSS_PHY_TESLA_REV_A) {
            save_squelch_ctrl_addr = (clock_port == VTSS_PHY_RECOV_CLK1) ? 0x00BE : 0x00BD;
        } else {
            save_squelch_ctrl_addr = (clock_port == VTSS_PHY_RECOV_CLK1) ? 0x0026 : 0x0025;
        }
        VTSS_RC(vtss_phy_set_micro_set_addr_private(vtss_state, port_no, save_squelch_ctrl_addr));

        // Peek saved squelch control for the other recovered-clock port
        VTSS_RC(vtss_phy_micro_peek_private(vtss_state, port_no, 0, &other_port_save_squelch_ctrl));
    }

    VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));

    if (clock_port == VTSS_PHY_RECOV_CLK1) {
        if (vtss_state->phy_state[port_no].family == VTSS_PHY_FAMILY_ENZO) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED_CHK_MASK(vtss_state, port_no, VTSS_PHY_RECOVERED_CLOCK_0_CONTROL, reg_val, 0xFFFF, 0x7FFF));     /* On Enzo it is not possible to set bit 15 - do not check */
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_RECOVERED_CLOCK_0_CONTROL, reg_val));
        }
    } else {
        if (vtss_state->phy_state[port_no].family == VTSS_PHY_FAMILY_ENZO) {
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED_CHK_MASK(vtss_state, port_no, VTSS_PHY_RECOVERED_CLOCK_1_CONTROL, reg_val, 0xFFFF, 0x7FFF));     /* On Enzo it is not possible to set bit 15 - do not check */
        } else {
            VTSS_RC(VTSS_PHY_WARM_WR(vtss_state, port_no, VTSS_PHY_RECOVERED_CLOCK_1_CONTROL, reg_val));
        }
    }

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));

    if (use_squelch_workaround) {
        VTSS_RC(squelch_workaround_private(vtss_state, port_no, TRUE));

        // Poke back saved squelch control for the other recovered-clock port
        VTSS_RC(vtss_phy_micro_poke_private(vtss_state, port_no, 0, other_port_save_squelch_ctrl));
    }
    return VTSS_RC_OK;
}


// Setting LED mode (blink mode)
//
// In : port : Phy port
//      led : LED mode to run the LEDs in
//      led_number : Which LED to configure
//
// Return :  VTSS_RC_OK is setup was preformed correct else VTSS_RC_ERROR
static vtss_rc vtss_phy_led_mode_set_private(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no)
{
    u16 reg19e_mode = 0;
    u16 reg29_mode = 0;
    switch (vtss_state->led_mode_select.mode) {
    case LINK_ACTIVITY:
        reg29_mode = 0;
        break;
    case LINK1000_ACTIVITY:
        reg29_mode = 1;
        break;
    case LINK100_ACTIVITY:
        reg29_mode = 2;
        break;
    case  LINK10_ACTIVITY:
        reg29_mode = 3;
        break;
    case  LINK100_1000_ACTIVITY:
        reg29_mode = 4;
        break;
    case  LINK10_1000_ACTIVITY:
        reg29_mode = 5;
        break;
    case  LINK10_100_ACTIVITY:
        reg29_mode = 6;
        break;
    case  LINK100BASE_FX_1000BASE_X_ACTIVITY:
        reg29_mode = 7;
        break;
    case  DUPLEX_COLLISION:
        reg29_mode = 8;
        break;
    case  COLLISION:
        reg29_mode = 9;
        break;
    case  ACTIVITY:
        reg29_mode = 10;
        break;
    case  BASE100_FX_1000BASE_X_FIBER_ACTIVITY:
        reg29_mode = 11;
        break;
    case  AUTONEGOTIATION_FAULT:
        reg29_mode = 12;
        break;
    case  FORCE_LED_OFF:
        reg29_mode = 14;
        break;
    case  FORCE_LED_ON:
        reg29_mode = 15;
        break;
    case  LINK1000BASE_X_ACTIVITY:
        reg29_mode = 0;
        reg19e_mode = 1;
        break;
    case  LINK100BASE_FX_ACTIVITY:
        reg29_mode = 1;
        reg19e_mode = 1;
        break;
    case  BASE1000_ACTIVITY:
        reg29_mode = 2;
        reg19e_mode = 1;
        break;
    case  BASE100_FX_ACTIVITY:
        reg29_mode = 3;
        reg19e_mode = 1;
        break;
    case  FAST_LINK_FAIL:
        reg29_mode = 6;
        reg19e_mode = 1;
        break;
    }


    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    switch (vtss_state->led_mode_select.number) {
    case LED3:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_LED_MODE_SELECT, reg29_mode << 12, 0xF000));
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EXTENDED_MODE_CONTROL, reg19e_mode << 15, 0x8000));
        break;
    case LED2:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_LED_MODE_SELECT, reg29_mode << 8, 0x0F00));
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EXTENDED_MODE_CONTROL, reg19e_mode << 14, 0x4000));
        break;
    case LED1:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_LED_MODE_SELECT, reg29_mode << 4, 0x00F0));
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EXTENDED_MODE_CONTROL, reg19e_mode << 13, 0x2000));
        break;
    case LED0:
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_LED_MODE_SELECT, reg29_mode, 0x000F));
        VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EXTENDED_MODE_CONTROL, reg19e_mode << 12, 0x1000));
        break;
    }


    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}


#if defined(VTSS_FEATURE_LED_POW_REDUC)
// Setting LED intensity.
//
// In : port : Phy port
//
// Return :  VTSS_RC_OK is setup was preformed correct else VTSS_RC_ERROR
static vtss_rc vtss_phy_led_intensity_set_private(vtss_state_t *vtss_state,
                                                  const vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_phy_family_t     family;


    /* Save setup */
    family = ps->family;

    VTSS_N("Family: %s, port_no = %u, vtss_state->led_intensity = %d",
           vtss_phy_family2txt(family), port_no, vtss_state->led_intensity);


    switch (family) {
    case VTSS_PHY_FAMILY_ENZO:
    case VTSS_PHY_FAMILY_SPYDER:
        // We only operate with on/off for these PHYs
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        if (vtss_state->led_intensity == 0) {
            // Turn LED off
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_LED_MODE_SELECT, 0xEEEE, 0xFFFF));
        } else {
            // Turn LED on
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_LED_MODE_SELECT, 0xEE64, 0xFFFF));
        }

        break;
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        // Datasheet section 4.6.15, register 25G
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
        // Set intensity. Multiply with 2 in order to convert from percent to register values (See datasheet 25G)
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_ENHANCED_LED_CONTROL, (vtss_state->led_intensity * 2) << 8, 0xFF00));
        break;

    default:
        VTSS_E("LED intensity not supported for family: %s, port = %d", vtss_phy_family2txt(family), port_no);
        return VTSS_RC_ERROR;
    }
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}



// Setting the enhanced LED control initial state (Should only be set once at startup).
//
// In : port  : Phy port
//      conf  : Enhanced LED control configuration
//
// Return :  VTSS_RC_OK is setup was preformed correct else VTSS_RC_ERROR
static vtss_rc vtss_phy_enhanced_led_control_init_private(vtss_state_t *vtss_state,
                                                          const vtss_port_no_t port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_phy_family_t     family;

    /* Save setup */
    VTSS_D("enter, port_no: %u", port_no);
    family = ps->family;

    vtss_state->phy_led_control_warm_start_port_list[port_no] = TRUE; // Signaling that this port is used for LED control (for warm start)

    switch (family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_ENHANCED_LED_CONTROL, vtss_state->enhanced_led_control.ser_led_output_2 << 7, 0x80)); // Datasheet section 4.6.15, register 25G
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_ENHANCED_LED_CONTROL, vtss_state->enhanced_led_control.ser_led_output_1 << 6, 0x40)); // Datasheet section 4.6.15, register 25G
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_ENHANCED_LED_CONTROL, vtss_state->enhanced_led_control.ser_led_frame_rate << 3, 0x38)); // Datasheet section 4.6.15, register 25G
        VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_ENHANCED_LED_CONTROL, vtss_state->enhanced_led_control.ser_led_select << 1, 0x06)); // Datasheet section 4.6.15, register 25G


        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        if (!vtss_state->enhanced_led_control.ser_led_output_2 && !vtss_state->enhanced_led_control.ser_led_output_1) {
            VTSS_N("ser_led_output_2:%d, ser_led_output_1:%d", vtss_state->enhanced_led_control.ser_led_output_2, vtss_state->enhanced_led_control.ser_led_output_1);
            // If non serial LED mode, we enable LED pulsing
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_LED_BEHAVIOR, VTSS_F_PHY_LED_BEHAVIOR_LED_PULSING_ENABLE, VTSS_F_PHY_LED_BEHAVIOR_LED_PULSING_ENABLE));
        }  else {
            VTSS_N("ser_led_output_2:%d, ser_led_output_1:%d", vtss_state->enhanced_led_control.ser_led_output_2, vtss_state->enhanced_led_control.ser_led_output_1);
            VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_LED_BEHAVIOR, 0, VTSS_F_PHY_LED_BEHAVIOR_LED_PULSING_ENABLE));
        }
        break;

    default:
        VTSS_E("Enhanced LED control not supported for family: %s", vtss_phy_family2txt(family));
        return VTSS_RC_ERROR;
    }
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    return VTSS_RC_OK;
}

#endif

/* - Chip temperature  ----------------------------------- */

static vtss_rc vtss_phy_read_temp_reg (vtss_state_t *vtss_state,
                                       vtss_port_no_t port_no, u8 *temp_reading)
{
    u16 reg;
    u8 timeout = 255; // Used to make sure that we don't run forever in while loop.
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];

    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no)); // Change to GPIO page

        // Workaround because background temperature monitoring doesn't work
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEMP_CONF, 0x0, 0x0040));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEMP_CONF, 0x0040, 0x0040));

        // Read current register value (Temperature Registers 26G,27G and 28G)
        VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_TEMP_VAL, &reg));
        *temp_reading = reg & 0xFF; // See data sheet for Registers 26G,27G and 28G


        // Revision A special
        if ((ps->family == VTSS_PHY_FAMILY_ATOM || ps->family == VTSS_PHY_FAMILY_LUTON26) && (ps->type.revision == VTSS_PHY_ATOM_REV_A)) {
            // From Jim B - You need to suspend the 8051 patch prior to triggering the TMON.
            //  I put code in the patch to automatically trigger a temperature reading at the end of the 5ms polling loop,
            // but here is no guarantee that you're not reading in the middle of the patch where we are using the aux-ADC to read out VGA power-up status.
            VTSS_RC(atom_patch_suspend(vtss_state, port_no, TRUE));


            VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no)); // Change to GPIO page

            // Workaround because background temperature monitoring doesn't work
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEMP_CONF, 0x0, 0x0040));
            VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEMP_CONF, 0x0040, 0x0040));

            // Due to a bug in Atom12 rev A (which is going to fixed in next chip revision) we sometimes get wrong
            // temperature reading. This is a work around that makes sure that these wrong temperatures doesn't make dramatics
            // changes.
            while (*temp_reading != ps->last_temp_reading && timeout != 0) {


                // Read current register value (Temperature Registers 26G,27G and 28G)
                VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_TEMP_VAL, &reg));
                *temp_reading = reg & 0xFF; // See data sheet for Registers 26G,27G and 28G

                if (*temp_reading == 0xFF || *temp_reading == 0x00) {
                    // Do nothing
                } else if (*temp_reading > ps->last_temp_reading) {
                    ps->last_temp_reading += 1;
                } else {
                    ps->last_temp_reading -= 1;
                }
                timeout--;

                VTSS_N("temp_reading = %d, last_temp_reading = %d",
                       *temp_reading, ps->last_temp_reading);
            }

            VTSS_RC(atom_patch_suspend(vtss_state, port_no, FALSE)); // Resume patch
        }
        break;

    default:
        return VTSS_RC_ERROR;
    }

    VTSS_RC(vtss_phy_page_std(vtss_state, port_no)); // Back to standard page.
    return VTSS_RC_OK;
}


// Function for getting chip temperature
//
// In: port_no - The PHY port number starting from 0.
//
// In/Out: Temp - The chip temperature (from -46 to 135 degC)
//
// Return: VTSS_RC_OK if we got the temperature - else error code
vtss_rc vtss_phy_chip_temp_get_private (vtss_state_t *vtss_state,
                                        const vtss_port_no_t  port_no,
                                        i16                    *const temp)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_rc rc = VTSS_RC_OK;
    u8 temp_reading = 0;
    i16 temperature;


    /* Check to see if operation supported on this PHY */
    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        // Read the temperature.
        VTSS_RC(vtss_phy_read_temp_reg(vtss_state, port_no, &temp_reading));

        //135.3degC - 0.71degC*ADCOUT
        temperature = (13530 - 71 * temp_reading) / 100;
        *temp = temperature; // Temperature bits - See PHY Data sheet section 4.6.18
        VTSS_N("Temperature = %d, temp_reading = 0x%X", *temp, temp_reading);
        break;
    default:
        VTSS_E("Temperature reading not supported for family: %s", vtss_phy_family2txt(ps->family));
        rc = VTSS_RC_ERROR;
    }

    return rc;
}


// Function for init.  chip temperature censor
//
// In: port_no - The PHY port number starting from 0 ( Any port with the phy can be used ).
//
// In/Out: Temp - The chip temperature
//
// Return: VTSS_RC_OK if init went well, else error code.
vtss_rc vtss_phy_chip_temp_init_private(vtss_state_t *vtss_state, const vtss_port_no_t  port_no)
{
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];
    vtss_rc rc = VTSS_RC_OK;

    VTSS_D("Init chip for port:%u", port_no);

    /* Check to see if operation supported on this PHY */
    switch (ps->family) {
    case VTSS_PHY_FAMILY_ATOM:
    case VTSS_PHY_FAMILY_LUTON26:
    case VTSS_PHY_FAMILY_TESLA:
    case VTSS_PHY_FAMILY_VIPER:
    case VTSS_PHY_FAMILY_ELISE:
        // Read current register value (Temperature Registers 26G,27G and 28G)
        VTSS_RC(vtss_phy_page_gpio(vtss_state, port_no));
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEMP_CONF, 0x0080, 0x0080)); // Deassert TMON0, Reset and enable background monitoring
        VTSS_RC(PHY_WR_MASKED_PAGE(vtss_state, port_no, VTSS_PHY_TEMP_CONF, 0x00C0, 0x00C0)); // Disable TMON1
        VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
        break;
    default:
        VTSS_E("Temperature reading not supported for family: %s", vtss_phy_family2txt(ps->family));
        rc = VTSS_RC_ERROR;
    }

    return rc;
}


/* - Debug functions  ----------------------------------- */
/* Enable/disable a loopback  */
static vtss_rc vtss_phy_loopback_set_private(vtss_state_t *vtss_state,
                                             const vtss_port_no_t       port_no)

{
    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL,
                                    (vtss_state->phy_state[port_no].loopback.far_end_enable ? VTSS_F_PHY_EXTENDED_PHY_CONTROL_FAR_END_LOOPBACK_MODE : 0),
                                    VTSS_F_PHY_EXTENDED_PHY_CONTROL_FAR_END_LOOPBACK_MODE)); // Far end Loopback

    VTSS_RC(VTSS_PHY_WARM_WR_MASKED(vtss_state, port_no, VTSS_PHY_MODE_CONTROL,
                                    (vtss_state->phy_state[port_no].loopback.near_end_enable ? VTSS_F_PHY_MODE_CONTROL_LOOP : 0),
                                    VTSS_F_PHY_MODE_CONTROL_LOOP)); // Near end Loopback

    return VTSS_RC_OK;
}


// Getting phy statistic
static vtss_rc vtss_phy_statistic_get_private(vtss_state_t *vtss_state,
                                              const vtss_port_no_t port_no, vtss_phy_statistic_t *statistic)
{
    u16 reg_val;


    // Update CRC good count
    VTSS_RC(vtss_phy_page_ext(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_CU_MEDIA_CRC_GOOD_COUNTER, &reg_val));
    VTSS_D("reg_val:0x%X", reg_val);
    if (reg_val & VTSS_F_PHY_CU_MEDIA_CRC_GOOD_COUNTER_PACKET_SINCE_LAST_READ) {
        statistic->cu_good = reg_val & VTSS_M_PHY_CU_MEDIA_CRC_GOOD_COUNTER_CONTENTS;
    } else {
        statistic->cu_good = 0;
    }


    // Update CRC bad count
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_EXTENDED_PHY_CONTROL_4, &reg_val));
    statistic->cu_bad = reg_val & VTSS_M_VTSS_PHY_EXTENDED_PHY_CONTROL_4_CU_MEDIA_CRC_ERROR_COUNTER;


    // Update serdes good count
    VTSS_RC(vtss_phy_page_ext3(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MEDIA_SERDES_TX_GOOD_PACKET_COUNTER, &reg_val));
    if (reg_val & VTSS_F_PHY_MEDIA_SERDES_TX_GOOD_PACKET_COUNTER_ACTIVE) {
        statistic->serdes_tx_good = reg_val & VTSS_M_PHY_MEDIA_SERDES_TX_GOOD_PACKET_COUNTER_CNT;
    } else {
        statistic->serdes_tx_good = 0;
    }

    // Update serdes bad count
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MEDIA_SERDES_TX_CRC_ERROR_COUNTER, &reg_val));
    statistic->serdes_tx_bad = reg_val & VTSS_M_PHY_MEDIA_SERDES_TX_CRC_ERROR_COUNTER_CNT;


    // Update 100/1000BASE-TX media/mac serdes CRC error counter
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MEDIA_MAC_SERDES_RX_CRC_CRC_ERR_COUNTER, &reg_val));
    statistic->media_mac_serdes_crc = reg_val & VTSS_M_PHY_MEDIA_MAC_SERDES_RX_CRC_ERR_COUNTER_CNT;

    // Update 100/1000BASE-TX media/mac serdes counter
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_MEDIA_MAC_SERDES_RX_GOOD_COUNTER, &reg_val));
    statistic->media_mac_serdes_good = reg_val & VTSS_M_PHY_MEDIA_MAC_SERDES_RX_GOOD_COUNTER_CNT;

    // Update 100/1000BASE-TX receive error counter
    VTSS_RC(vtss_phy_page_std(vtss_state, port_no));
    VTSS_RC(PHY_RD_PAGE(vtss_state, port_no, VTSS_PHY_ERROR_COUNTER_1, &reg_val));
    statistic->rx_err_cnt_base_tx = reg_val & VTSS_M_PHY_ERROR_COUNTER_1_100_1000BASETX_RX_ERR_CNT;



    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Public functions
 * ================================================================= */



/* - Reset --------------------------------------------------------- */

vtss_rc vtss_phy_pre_reset(const vtss_inst_t           inst,
                           const vtss_port_no_t        port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    VTSS_D("Enter vtss_phy_pre_reset port_no:%d", port_no);
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        /* -- Step 1: Detect PHY type and family -- */
        rc = vtss_phy_detect(vtss_state, port_no);

        if (rc == VTSS_RC_OK) {
            rc = VTSS_RC_COLD(vtss_phy_pre_reset_private(vtss_state, port_no));
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_post_reset(const vtss_inst_t           inst,
                            const vtss_port_no_t        port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    VTSS_D("Enter vtss_phy_post_reset port_no:%d", port_no);
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        /* -- Step 1: Detect PHY type and family -- */
        rc = vtss_phy_detect(vtss_state, port_no);

        /* Detect base ports, since all PHY should now have been detected */
        if (rc == VTSS_RC_OK) {
            rc = vtss_phy_detect_base_ports_private(vtss_state);
        }

        if (rc == VTSS_RC_OK) {
            rc = VTSS_RC_COLD(vtss_phy_post_reset_private(vtss_state, port_no));
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_pre_system_reset(const vtss_inst_t           inst,
                                  const vtss_port_no_t        port_no)
{
    return VTSS_RC_OK;
}

vtss_rc vtss_phy_reset(const vtss_inst_t           inst,
                       const vtss_port_no_t        port_no,
                       const vtss_phy_reset_conf_t *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_D("vtss_phy_reset, Port:%d", port_no);
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_state[port_no].reset = *conf;

        /* -- Step 1: Detect PHY type and family -- */
        rc = vtss_phy_detect(vtss_state, port_no);

        if (rc == VTSS_RC_OK) {
            rc = VTSS_RC_COLD(vtss_phy_reset_private(vtss_state, port_no));
        }
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_reset_get(const vtss_inst_t           inst,
                           const vtss_port_no_t        port_no,
                           vtss_phy_reset_conf_t *conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *conf = vtss_state->phy_state[port_no].reset;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_detect_base_ports(const vtss_inst_t inst)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc =  vtss_inst_check(inst, &vtss_state)) == VTSS_RC_OK) {
        if (rc == VTSS_RC_OK) {
            rc = (vtss_phy_detect_base_ports_private(vtss_state));
        }
    }
    VTSS_EXIT();
    return rc;
}


// Returns if a port is EEE capable
vtss_rc vtss_phy_port_eee_capable(const vtss_inst_t           inst,
                                  const vtss_port_no_t        port_no,
                                  BOOL                        *eee_capable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *eee_capable = vtss_phy_can(vtss_state, port_no, VTSS_CAP_EEE);
    }
    VTSS_EXIT();
    return rc;
}

#if defined(VTSS_FEATURE_EEE)
// Enable disable EEE (energy Efficient Ethernet)
vtss_rc vtss_phy_eee_ena(const vtss_inst_t           inst,
                         const vtss_port_no_t        port_no,
                         const BOOL                  enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (enable) {
            vtss_state->phy_state[port_no].eee_conf.eee_mode = EEE_ENABLE;
        } else {
            vtss_state->phy_state[port_no].eee_conf.eee_mode = EEE_DISABLE;
        }
        rc = VTSS_RC_COLD(vtss_phy_eee_ena_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_eee_conf_get(const vtss_inst_t           inst,
                              const vtss_port_no_t        port_no,
                              vtss_phy_eee_conf_t         *conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *conf = vtss_state->phy_state[port_no].eee_conf;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_eee_conf_set(const vtss_inst_t           inst,
                              const vtss_port_no_t        port_no,
                              const vtss_phy_eee_conf_t   conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_state[port_no].eee_conf = conf;
        rc = VTSS_RC_COLD(vtss_phy_eee_ena_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

// EEE link partners advertisements
vtss_rc vtss_phy_eee_link_partner_advertisements_get(const vtss_inst_t           inst,
                                                     const vtss_port_no_t        port_no,
                                                     u8  *advertisements)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_RC_COLD(vtss_phy_eee_link_partner_advertisements_get_private(vtss_state, port_no, advertisements));
    }
    VTSS_EXIT();
    return rc;
}

// EEE current phy power save mode state
vtss_rc vtss_phy_eee_power_save_state_get(const vtss_inst_t           inst,
                                          const vtss_port_no_t        port_no,
                                          BOOL  *rx_in_power_save_state,
                                          BOOL  *tx_in_power_save_state)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = VTSS_RC_COLD(vtss_phy_eee_power_save_state_get_private(vtss_state, port_no, rx_in_power_save_state, tx_in_power_save_state));
    }
    VTSS_EXIT();
    return rc;
}

#endif // end VTSS_FEATURE_EEE
/* - Main configuration and status --------------------------------- */

// See header file
vtss_rc vtss_phy_id_get(const vtss_inst_t   inst,
                        const vtss_port_no_t  port_no,
                        vtss_phy_type_t *phy_id)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_id_get_private(vtss_state, port_no, phy_id);
    }

    VTSS_EXIT();
    return rc;
}


// See vtss_misc_api.h
vtss_rc vtss_phy_chip_temp_init(const vtss_inst_t     inst,
                                const vtss_port_no_t port_no)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_chip_temp_init_private(vtss_state, port_no);
    }

    VTSS_EXIT();
    return rc;
}



vtss_rc vtss_phy_chip_temp_get(const vtss_inst_t     inst,
                               const vtss_port_no_t port_no,
                               i16                    *const temp)
{
    vtss_state_t *vtss_state;
    i16 chip_temp;
    vtss_rc rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_chip_temp_get_private(vtss_state, port_no, &chip_temp);
        *temp = chip_temp;
    }

    VTSS_EXIT();
    return rc;
}



vtss_rc vtss_phy_conf_get(const vtss_inst_t    inst,
                          const vtss_port_no_t port_no,
                          vtss_phy_conf_t      *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *conf = vtss_state->phy_state[port_no].setup;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_conf_set(const vtss_inst_t     inst,
                          const vtss_port_no_t  port_no,
                          const vtss_phy_conf_t *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_state[port_no].setup = *conf;
        rc = VTSS_RC_COLD(vtss_phy_conf_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_status_get(const vtss_inst_t    inst,
                            const vtss_port_no_t port_no,
                            vtss_port_status_t   *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_status_get_private(vtss_state, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}


/* - 1G configuration and status ----------------------------------- */

vtss_rc vtss_phy_conf_1g_get(const vtss_inst_t     inst,
                             const vtss_port_no_t  port_no,
                             vtss_phy_conf_1g_t    *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *conf = vtss_state->phy_state[port_no].conf_1g;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_conf_1g_set(const vtss_inst_t         inst,
                             const vtss_port_no_t      port_no,
                             const vtss_phy_conf_1g_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_state[port_no].conf_1g = *conf;
        rc = VTSS_RC_COLD(vtss_phy_conf_1g_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_status_1g_get(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               vtss_phy_status_1g_t  *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_status_1g_get_private(vtss_state, port_no, status);
    }
    VTSS_EXIT();
    return rc;
}

/* - Power configuration and status -------------------------------- */

vtss_rc vtss_phy_power_conf_get(const vtss_inst_t      inst,
                                const vtss_port_no_t   port_no,
                                vtss_phy_power_conf_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *conf = vtss_state->phy_state[port_no].conf_power;
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_power_conf_set(const vtss_inst_t            inst,
                                const vtss_port_no_t         port_no,
                                const vtss_phy_power_conf_t  *const conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_state[port_no].conf_power = *conf;
        rc = VTSS_RC_COLD(vtss_phy_power_conf_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_power_status_get(const vtss_inst_t        inst,
                                  const vtss_port_no_t     port_no,
                                  vtss_phy_power_status_t  *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        status->level = vtss_state->phy_state[port_no].power.pwrusage;
    }
    VTSS_EXIT();
    return rc;
}

/* - Clock configuration ---------- -------------------------------- */
vtss_rc vtss_phy_clock_conf_set(const vtss_inst_t           inst,
                                const vtss_port_no_t        port_no,
                                const vtss_phy_recov_clk_t  clock_port,
                                const vtss_phy_clock_conf_t *const conf)
{
    vtss_state_t    *vtss_state;
    vtss_rc         rc;
    vtss_phy_type_t phy_id;


    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (clock_port < VTSS_PHY_RECOV_CLK_NUM) { /* Valid clock port */
            if ((rc = vtss_phy_id_get_private(vtss_state, port_no, &phy_id)) == VTSS_RC_OK) {
                if ((rc = VTSS_PHY_BASE_PORTS_FOUND) == VTSS_RC_OK) { // Make sure that base ports are found
                    vtss_state->phy_state[phy_id.base_port_no].clock_conf[clock_port].conf = *conf;      /* Configuration is always saved in the base port phy_state */
                    vtss_state->phy_state[phy_id.base_port_no].clock_conf[clock_port].source = port_no;  /* Also save the clock source port number */
                    rc = VTSS_RC_COLD(vtss_phy_clock_conf_set_private(vtss_state, port_no, clock_port));
                }
            }
        }
    }
    VTSS_EXIT();
    return rc;
}

/* get function for above set */
vtss_rc vtss_phy_clock_conf_get(const vtss_inst_t            inst,
                                const vtss_port_no_t         port_no,
                                const vtss_phy_recov_clk_t   clock_port,
                                vtss_phy_clock_conf_t        *const conf,
                                vtss_port_no_t               *const clock_source)
{
    vtss_state_t    *vtss_state;
    vtss_rc         rc;
    vtss_phy_type_t phy_id;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (clock_port < VTSS_PHY_RECOV_CLK_NUM) { /* Valid clock port */
            if ((rc = vtss_phy_id_get_private(vtss_state, port_no, &phy_id)) == VTSS_RC_OK) {
                if ((rc = VTSS_PHY_BASE_PORTS_FOUND) == VTSS_RC_OK) { // Make sure that base ports are found
                    *conf = vtss_state->phy_state[phy_id.base_port_no].clock_conf[clock_port].conf;      /* Configuration is always saved in the base port phy_state */
                    *clock_source = vtss_state->phy_state[phy_id.base_port_no].clock_conf[clock_port].source;    /* Also return the clock source port number */
                }
            }
        }
    }
    VTSS_EXIT();
    return rc;
}

/* - Warm start synchronization ------------------------------------ */

#define VTSS_SYNC_RC(function) if ((rc = function) != VTSS_RC_OK) {vtss_state->sync_calling_private = FALSE; return rc;}
vtss_rc vtss_phy_sync(vtss_state_t *vtss_state, const vtss_port_no_t port_no)
{
    vtss_rc rc;
    vtss_port_no_t i;
    vtss_phy_port_state_t *ps = &vtss_state->phy_state[port_no];

    VTSS_I("OK - Warm start : port:%d", port_no);
    // Make sure that this port in fact has a 1G PHY

    if (ps->family == VTSS_PHY_FAMILY_NONE) {
        VTSS_N("port_no %u not connected to 1G PHY", port_no);
        return VTSS_RC_OK;
    }

    VTSS_D("vtss_phy_sync, port_no:%d", port_no);
    vtss_state->sync_calling_private = TRUE;

    // Starting wiht no registers changed.
    vtss_state->phy_state[port_no].warm_start_reg_changed = FALSE;

    // Call the private function for updating H/W registers
    VTSS_SYNC_RC(vtss_phy_conf_set_private(vtss_state, port_no));
#if defined(VTSS_FEATURE_LED_POW_REDUC)
    if (vtss_state->phy_led_control_warm_start_port_list[port_no]) { // Only warm-start the ports that was initialized
        VTSS_SYNC_RC(vtss_phy_led_intensity_set_private(vtss_state, port_no));
        VTSS_SYNC_RC(vtss_phy_enhanced_led_control_init_private(vtss_state, port_no));
    }
#endif
    VTSS_SYNC_RC(vtss_phy_power_conf_set_private(vtss_state, port_no));
    VTSS_SYNC_RC(vtss_phy_conf_1g_set_private(vtss_state, port_no));

#if defined(VTSS_FEATURE_EEE)
    VTSS_SYNC_RC(vtss_phy_eee_ena_private(vtss_state, port_no));
#endif

    VTSS_SYNC_RC(vtss_phy_reset_private(vtss_state, port_no));
    VTSS_SYNC_RC(vtss_phy_loopback_set_private(vtss_state, port_no));
    VTSS_N("Calling vtss_phy_event_enable_private(%d)", port_no);
    VTSS_SYNC_RC(vtss_phy_event_enable_private(vtss_state, port_no));

    for (i = 0; i < VTSS_PHY_RECOV_CLK_NUM; ++i) {
        VTSS_SYNC_RC(vtss_phy_clock_conf_set_private(vtss_state, port_no, i));
    }

    VTSS_I("OK - Warm start done");
    vtss_state->sync_calling_private = FALSE;

    return VTSS_RC_OK;

    // These functions shall not be called during warm start (The list below is just to show that we have thought about these functiona)
    // vtss_phy_coma_mode_private(port_no, TRUE)
    // vtss_phy_pre_system_reset_private(port_no)
    // vtss_phy_post_reset_private(port_no)
    // vtss_phy_pre_reset_private(port_no)

}

// Function returning VTSS_RC_ERROR if any registers were unexpected needed to be changed during warm start, else return VTSS_RC_OK - Note: In a working system VTSS_RC_ERROR should never be seen.
vtss_rc vtss_phy_warm_start_failed_get(const vtss_inst_t    inst,
                                       const vtss_port_no_t port_no)
{
    vtss_rc rc;
    vtss_state_t *vtss_state;
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_warm_start_failed_get_private(vtss_state, port_no);
    }
    VTSS_EXIT();
    return rc;
}

/* - LED Control ------------------------------------------------------- */

vtss_rc vtss_phy_led_mode_set(const vtss_inst_t            inst,
                              const vtss_port_no_t         port_no,
                              const vtss_phy_led_mode_select_t led_mode_select)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        VTSS_N("enter, port_no: %u", port_no);
        vtss_state->led_mode_select = led_mode_select;
        rc = VTSS_RC_COLD(vtss_phy_led_mode_set_private(vtss_state, port_no));
    }

    VTSS_EXIT();
    return rc;
}




#if defined(VTSS_FEATURE_LED_POW_REDUC)


vtss_rc vtss_phy_led_intensity_set(const vtss_inst_t            inst,
                                   const vtss_port_no_t         port_no,
                                   const vtss_phy_led_intensity intensity)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->led_intensity  = intensity;
        rc = VTSS_RC_COLD(vtss_phy_led_intensity_set_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_led_intensity_get(const vtss_inst_t            inst,
                                   const vtss_port_no_t         port_no,
                                   vtss_phy_led_intensity       *intensity)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *intensity = vtss_state->led_intensity;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_enhanced_led_control_init(const vtss_inst_t            inst,
                                           const vtss_port_no_t         port_no,
                                           const vtss_phy_enhanced_led_control_t conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        VTSS_N("enter, port_no: %u", port_no);
        vtss_state->enhanced_led_control  = conf;
        rc = VTSS_RC_COLD(vtss_phy_enhanced_led_control_init_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_enhanced_led_control_init_get(const vtss_inst_t            inst,
                                               const vtss_port_no_t         port_no,
                                               vtss_phy_enhanced_led_control_t *conf)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *conf = vtss_state->enhanced_led_control;
    }
    VTSS_EXIT();
    return rc;
}

#endif

/* - Coma mode  ---------------------------------------------------- */


vtss_rc vtss_phy_coma_mode_disable(const vtss_inst_t            inst,
                                   const vtss_port_no_t         port_no)

{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        VTSS_I("enter, port_no: %u", port_no);
        rc = VTSS_RC_COLD(vtss_phy_coma_mode_private(vtss_state, port_no, TRUE));
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_coma_mode_enable(const vtss_inst_t            inst,
                                  const vtss_port_no_t         port_no)

{
    vtss_state_t *vtss_state;
    vtss_rc rc;
    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        VTSS_I("enter, port_no: %u", port_no);
        rc = VTSS_RC_COLD(vtss_phy_coma_mode_private(vtss_state, port_no, FALSE));
    }
    VTSS_EXIT();
    return rc;
}


/* - I2C ---------------------------------------------------- */

/* Read PHY i2c register */
vtss_rc vtss_phy_i2c_read(const vtss_inst_t    inst,
                          const vtss_port_no_t port_no,
                          const u8             i2c_mux,
                          const u8             i2c_reg_addr,
                          const u8             i2c_device_addr,
                          u8                   *const value,
                          u8                   cnt)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_i2c_rd_private(vtss_state, port_no, i2c_mux, i2c_reg_addr, i2c_device_addr, value, cnt);
    }
    VTSS_EXIT();
    return rc;
}


/* Read PHY i2c register */
vtss_rc vtss_phy_i2c_write(const vtss_inst_t    inst,
                           const vtss_port_no_t port_no,
                           const u8             i2c_mux,
                           const u8             i2c_reg_addr,
                           const u8             i2c_device_addr,
                           u8                   *value,
                           u8                   cnt)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_i2c_wr_private(vtss_state, port_no, i2c_mux, i2c_reg_addr, i2c_device_addr, value, cnt);
    }
    VTSS_EXIT();
    return rc;
}


/* - Read/write ---------------------------------------------------- */


/* Read PHY mmd register */
vtss_rc vtss_phy_mmd_read(const vtss_inst_t    inst,
                          const vtss_port_no_t port_no,
                          const u16            devad,
                          const u32            addr,
                          u16                  *const value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_mmd_rd(vtss_state, port_no, devad, addr, value);
    }
    VTSS_EXIT();
    return rc;
}


/* Write PHY mmd register */
vtss_rc vtss_phy_mmd_write(const vtss_inst_t    inst,
                           const vtss_port_no_t port_no,
                           const u16            devad,
                           const u32            addr,
                           u16                  value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_mmd_wr(vtss_state, port_no, devad, addr, value);
    }
    VTSS_EXIT();
    return rc;
}


/* Get PHY statistic  */
vtss_rc vtss_phy_statistic_get(const vtss_inst_t    inst,
                               const vtss_port_no_t port_no,
                               vtss_phy_statistic_t *statistic)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_statistic_get_private(vtss_state, port_no, statistic);
    }
    VTSS_EXIT();
    return rc;
}


/* Read PHY register at a specific page  */
vtss_rc vtss_phy_read_page(const vtss_inst_t    inst,
                           const vtss_port_no_t port_no,
                           const u32            page,
                           const u32            addr,
                           u16                  *const value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc |= vtss_phy_wr(vtss_state, port_no, 31, page);
        rc |= vtss_phy_rd(vtss_state, port_no, addr, value);
        rc |= vtss_phy_page_std(vtss_state, port_no);
    }
    VTSS_EXIT();
    return rc;
}


/* Read PHY register */vtss_rc vtss_phy_read(const vtss_inst_t    inst,
                                             const vtss_port_no_t port_no,
                                             const u32            addr,
                                             u16                  *const value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_rd(vtss_state, port_no, addr, value);
    }
    VTSS_EXIT();
    return rc;
}

/* Write PHY register */
vtss_rc vtss_phy_write(const vtss_inst_t    inst,
                       const vtss_port_no_t port_no,
                       const u32            addr,
                       const u16            value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_wr(vtss_state, port_no, addr, value);
    }
    VTSS_EXIT();
    return rc;
}

/* Write PHY register, masked */
vtss_rc vtss_phy_write_masked(const vtss_inst_t    inst,
                              const vtss_port_no_t port_no,
                              const u32            addr,
                              const u16            value,
                              const u16            mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_wr_masked(vtss_state, port_no, addr, value, mask);
    }
    VTSS_EXIT();
    return rc;
}



/* Write PHY register, masked (including page setup) */
vtss_rc vtss_phy_write_masked_page(const vtss_inst_t    inst,
                                   const vtss_port_no_t port_no,
                                   const u16            page,
                                   const u16            addr,
                                   const u16            value,
                                   const u16            mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        (void) vtss_phy_wr(vtss_state, port_no, 31, page);
        rc = vtss_phy_wr_masked(vtss_state, port_no, addr, value, mask);
        (void) vtss_phy_wr(vtss_state, port_no, 31, 0);
    }
    VTSS_EXIT();
    return rc;
}



/* - VeriPHY ------------------------------------------------------- */
#if VTSS_PHY_OPT_VERIPHY
static vtss_rc vtss_phy_veriphy_get_private(vtss_state_t *vtss_state,
                                            const vtss_port_no_t      port_no,
                                            vtss_phy_veriphy_result_t *const result)
{
    vtss_veriphy_task_t c51_idata *tsk;
    vtss_rc                       rc;
    u32                           i;

    tsk = &vtss_state->phy_state[port_no].veriphy;
    rc = vtss_phy_veriphy(vtss_state, tsk);
    if (rc == VTSS_RC_OK && !(tsk->flags & (1 << 1))) {
        /* Invalid result */
        rc = VTSS_RC_ERROR;
    }

    result->link = (rc == VTSS_RC_OK && (tsk->flags & (1 << 0)) ? 1 : 0);
    for (i = 0; i < 4; i++) {
        result->status[i] = (rc == VTSS_RC_OK ? (tsk->stat[i] & 0x0f) :
                             VTSS_VERIPHY_STATUS_OPEN);
        result->length[i] = (rc == VTSS_RC_OK ? tsk->loc[i] : 0);
    }
    return rc;
}

vtss_rc vtss_phy_veriphy_start(const vtss_inst_t     inst,
                               const vtss_port_no_t  port_no,
                               const u8              mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_veriphy_task_start(vtss_state, port_no, mode);
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_veriphy_get(const vtss_inst_t          inst,
                             const vtss_port_no_t       port_no,
                             vtss_phy_veriphy_result_t  *const result)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_veriphy_get_private(vtss_state, port_no, result);
    }
    VTSS_EXIT();
    return rc;
}
#endif /* VTSS_PHY_OPT_VERIPHY */



/* - Events --------------------------------------------------- */


vtss_rc vtss_phy_event_enable_set(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  const vtss_phy_event_t    ev_mask,
                                  const BOOL                enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        if (enable) {
            vtss_state->phy_state[port_no].ev_mask |= ev_mask;
        } else {
            vtss_state->phy_state[port_no].ev_mask &= ~ev_mask;
        }

        VTSS_N("vtss_phy_event_enable_set - ev_mask:0x%X, enable:%d", ev_mask, enable);
        rc = VTSS_RC_COLD(vtss_phy_event_enable_private(vtss_state, port_no));
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_event_enable_get(const vtss_inst_t         inst,
                                  const vtss_port_no_t      port_no,
                                  vtss_phy_event_t          *ev_mask)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *ev_mask = vtss_state->phy_state[port_no].ev_mask;
    }
    VTSS_EXIT();
    return rc;
}


vtss_rc vtss_phy_event_poll(const vtss_inst_t     inst,
                            const vtss_port_no_t  port_no,
                            vtss_phy_event_t      *const events)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_event_poll_private(vtss_state, port_no, events);
    }
    VTSS_EXIT();
    return rc;
}

/* - Misc.  --------------------------------------------------- */

// Function for enabling/disabling squelch work around.
// In : port_no - Any phy port with the chip
//    : enable  - TRUE = enable squelch workaround, FALSE = Disable squelch workaround
// Return - VTSS_RC_OK - Workaround was enabled/disable. VTSS_RC_ERROR - Squelch workaround patch not loaded
vtss_rc vtss_squelch_workaround(const vtss_inst_t         inst,
                                const vtss_port_no_t      port_no,
                                const BOOL enable)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = squelch_workaround_private(vtss_state, port_no, enable);
    }
    VTSS_EXIT();
    return rc;
}

// See vtss_phy_api.h
vtss_rc vtss_phy_csr_wr(const vtss_inst_t    inst,
                        const u16            page,
                        const vtss_port_no_t port_no,
                        const u16            target,
                        const u32            csr_reg_addr,
                        const u32            value)
{

    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        switch (page) {
        case VTSS_PHY_PAGE_1588:
            rc = vtss_phy_1588_csr_wr_private(vtss_state, port_no, target, csr_reg_addr, value);
            break;

        case VTSS_PHY_PAGE_MACSEC:
            rc = vtss_phy_macsec_csr_wr_private(vtss_state, port_no, target, csr_reg_addr, value);
            break;
        default:
            VTSS_E("Unknown page:0x%X for CSR access", page);
        }

    }
    VTSS_EXIT();

    return rc;
}

// See vtss_phy_api.h
vtss_rc vtss_phy_csr_rd(const vtss_inst_t    inst,
                        const u16            page,
                        const vtss_port_no_t port_no,
                        const u16            target,
                        const u32            csr_reg_addr,
                        u32                  *value)
{

    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        switch (page) {
        case VTSS_PHY_PAGE_1588:
            rc = vtss_phy_1588_csr_rd_private(vtss_state, port_no, target, csr_reg_addr, value);
            break;

        case VTSS_PHY_PAGE_MACSEC:
            rc = vtss_phy_macsec_csr_rd_private(vtss_state, port_no, target, csr_reg_addr, value);
            break;
        default:
            VTSS_E("Unknown page:0x%X for CSR access", page);
        }
    }
    VTSS_EXIT();

    return rc;
}


/* - Debug  --------------------------------------------------- */
// Function printing PHY statistics
static void vtss_phy_debug_stat_print_private(vtss_state_t *vtss_state,
                                              const vtss_debug_printf_t pr,
                                              const vtss_port_no_t      port_no,
                                              const BOOL                print_hdr)
{
    vtss_phy_statistic_t statistics;
    if (print_hdr) {
        pr("%-5s %-10s %-13s %-9s %-14s %-17s %-14s %-13s\n", "Port", "CU Rx Good", "CU Rx CRC err", "CU Rx Err", "SerDes Tx Good", "SerDes Tx CRC Err", "SerDes Rx Good", "SerDes Rx CRC");
        pr("%-5s %-10s %-13s %-9s %-14s %-17s %-14s %-13s\n", "----", "----------", "-------------", "---------", "--------------", "-----------------", "--------------", "-------------");
    }

    if (vtss_phy_statistic_get_private(vtss_state, port_no, &statistics) == VTSS_RC_OK) {
        pr("%-5u %-10d %-13d %-9d %-14d %-17d %-14d %-13d\n",
           port_no,
           statistics.cu_good,
           statistics.cu_bad,
           statistics.serdes_tx_good,
           statistics.serdes_tx_bad,
           statistics.rx_err_cnt_base_tx,
           statistics.media_mac_serdes_good,
           statistics.media_mac_serdes_crc);
    } else {
        pr("Could not get PHY statistics for port:%d \n", port_no);
    }
}

/* Get statistics */
vtss_rc vtss_phy_debug_stat_print(const vtss_inst_t         inst,
                                  const vtss_debug_printf_t pr,
                                  const vtss_port_no_t      port_no,
                                  const BOOL                print_hdr)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_phy_debug_stat_print_private(vtss_state, pr, port_no, print_hdr);
    }
    VTSS_EXIT();
    return rc;
}

vtss_rc vtss_phy_debug_info_print(vtss_state_t *vtss_state,
                                  const vtss_debug_printf_t pr,
                                  const vtss_debug_info_t   *const info,
                                  const BOOL                ail,
                                  const BOOL                ignore_group_enabled,
                                  const BOOL                print_hdr)
{
    vtss_port_no_t        port_no;
    vtss_phy_port_state_t *ps;
    vtss_phy_event_t      events;
    vtss_phy_type_t       phy_id;

    if (!ignore_group_enabled) {
        if (!vtss_debug_group_enabled(pr, info, VTSS_DEBUG_GROUP_PHY)) {
            return VTSS_RC_OK;
        }
    }

    if (ail) {
        /* Application Interface Layer */
        if (print_hdr) {
            pr("Port  Family    Type  Rev  MediaIf                         MacIf         IntrEvent EEE INT MAC Chan Port   1588   uPatch Chip\n");
            pr("                                                                         Poll              SEC ID   BaseNo BaseNo CRC    Port\n");
            pr("----- --------- ----- ---- ------------------------------- ------------- --------- --- --- --- ---- ------ ------ ------ ----\n");
        }
        for (port_no = VTSS_PORT_NO_START; port_no < vtss_state->port_count; port_no++) {
            vtss_port_no_t base_port = vtss_state->phy_state[port_no].type.base_port_no;
            if (!info->port_list[port_no] || vtss_state->phy_state[port_no].type.part_number == VTSS_PHY_TYPE_NONE) {
                continue;
            }

            VTSS_RC(vtss_phy_id_get_private(vtss_state, port_no, &phy_id));
            (void)vtss_phy_event_poll_private(vtss_state, port_no, &events);
            ps = &vtss_state->phy_state[port_no];
            VTSS_I("ps->features:0x%X, %p", ps->features, &vtss_state);
            pr("%-4u  %-8s  %-4u  %-4u %-31s %-13s 0x%-7X %-3s %-3s %-3s %-4d %-6u %-6u 0x%-4X %-4d\n",
               port_no, vtss_phy_family2txt(ps->family),
               ps->type.part_number,
               ps->type.revision,
               vtss_phy_media_if2txt(ps->reset.media_if),
               vtss_phy_mac_if2txt(ps->reset.mac_if),
               events,
               vtss_phy_can(vtss_state, port_no, VTSS_CAP_EEE) ? "Yes" : "No",
               vtss_phy_can(vtss_state, port_no, VTSS_CAP_INT) ? "Yes" : "No",
               vtss_phy_can(vtss_state, port_no, VTSS_CAP_MACSEC) ? "Yes" : "No",
               phy_id.channel_id,
               phy_id.base_port_no,
               phy_id.phy_api_base_no,
               vtss_state->phy_state[base_port].micro_patch_crc,
               vtss_phy_chip_port(vtss_state, port_no));
        }
    } else {
        /* Chip Interface Layer */
    }
    return VTSS_RC_OK;
}

// Print internal PHY state
vtss_rc vtss_phy_debug_phyinfo_print(const vtss_inst_t         inst,
                                     const vtss_debug_printf_t pr,
                                     const vtss_port_no_t      port_no,
                                     const BOOL                print_hdr)
{
    vtss_state_t *vtss_state;
    vtss_rc rc = VTSS_RC_OK;
    vtss_debug_info_t info;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        memset(&info, 0, sizeof(info));
        info.port_list[port_no] = TRUE;
        rc = vtss_phy_debug_info_print(vtss_state, pr, &info, TRUE, TRUE, print_hdr);
    }
    VTSS_EXIT();
    return rc;
}


// Setup Internal Loopback in the PHY
vtss_rc vtss_phy_loopback_set(const vtss_inst_t    inst,
                              const vtss_port_no_t port_no,
                              vtss_phy_loopback_t  loopback)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_state->phy_state[port_no].loopback.far_end_enable = loopback.far_end_enable;
        vtss_state->phy_state[port_no].loopback.near_end_enable = loopback.near_end_enable;
        rc = vtss_phy_loopback_set_private(vtss_state, port_no);
    }
    VTSS_EXIT();
    return rc;
}


/* Set do_page_chk */
vtss_rc vtss_phy_do_page_chk_set(const vtss_inst_t          inst,
                                 const BOOL                     enable)
{
    vtss_rc rc = VTSS_RC_OK;
    VTSS_ENTER();
    vtss_phy_do_page_chk_set_private(enable);
    VTSS_EXIT();
    return rc;
}

/* Set do_page_chk */
vtss_rc vtss_phy_do_page_chk_get(const vtss_inst_t          inst,
                                 BOOL                       *enable)
{
    vtss_rc rc = VTSS_RC_OK;
    VTSS_ENTER();
    vtss_phy_do_page_chk_get_private(enable);
    VTSS_EXIT();
    return rc;
}

/* Get the current loopback */
vtss_rc vtss_phy_loopback_get(const vtss_inst_t         inst,
                              const vtss_port_no_t      port_no,
                              vtss_phy_loopback_t   *loopback)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        *loopback = vtss_state->phy_state[port_no].loopback;
    }
    VTSS_EXIT();
    return rc;
}

// Function for checking if firmware is downloaded correctly
vtss_rc vtss_phy_is_8051_crc_ok(const vtss_inst_t       inst,
                                const vtss_port_no_t    port_no,
                                u16                     code_length,
                                u16                     expected_crc)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_is_8051_crc_ok_private(vtss_state, port_no, FIRMWARE_START_ADDR, code_length, expected_crc);
    }
    VTSS_EXIT();
    return rc;
}

// Function for setting ob_post0 patch
vtss_rc vtss_phy_cfg_ob_post0(const vtss_inst_t       inst,
                              const vtss_port_no_t    port_no,
                              const u16               value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
        rc = vtss_phy_cfg_ob_post0_private(vtss_state, port_no, value);
#endif
    }
    VTSS_EXIT();
    return rc;
}

// Function for setting ib_cterm patch
vtss_rc vtss_phy_cfg_ib_cterm(const vtss_inst_t       inst,
                              const vtss_port_no_t    port_no,
                              const u8                ib_cterm_ena,
                              const                   u8 ib_eq_mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
        rc = vtss_phy_cfg_ib_cterm_ena_private(vtss_state, port_no, ib_cterm_ena, ib_eq_mode);
#endif
    }
    VTSS_EXIT();
    return rc;
}


// Function for getting patch settings
vtss_rc vtss_phy_atom12_patch_setttings_get(const vtss_inst_t       inst,
                                            const vtss_port_no_t    port_no,
                                            u8                      *mcb_bus,
                                            u8                      *cfg_buf,
                                            u8                      *stat_buf)

{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
#if defined(VTSS_FEATURE_SERDES_MACRO_SETTINGS)
        rc = vtss_phy_patch_setttings_get_private(vtss_state, port_no, mcb_bus, cfg_buf, stat_buf);
#endif
    }
    VTSS_EXIT();
    return rc;
}

// Function for finding flow status in auto neg mode
vtss_rc vtss_phy_flowcontrol_decode_status(const vtss_inst_t       inst,
                                           const vtss_port_no_t port_no,
                                           u16 lp_auto_neg_advertisment_reg,
                                           const vtss_phy_conf_t phy_setup,
                                           vtss_port_status_t *const status)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        vtss_phy_flowcontrol_decode_status_private(port_no, lp_auto_neg_advertisment_reg, phy_setup, status);
    }
    VTSS_EXIT();
    return rc;
}


// See vtss_phy_api.h
vtss_rc vtss_phy_gpio_mode(const vtss_inst_t          inst,
                           const vtss_port_no_t       port_no,
                           const u8                   gpio_no,
                           const vtss_phy_gpio_mode_t mode)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_gpio_mode_private(vtss_state, port_no, gpio_no, mode);
    }
    VTSS_EXIT();
    return rc;
}

// See vtss_phy_api.h
vtss_rc vtss_phy_gpio_get(const vtss_inst_t    inst,
                          const vtss_port_no_t port_no,
                          const u8             gpio_no,
                          BOOL                 *value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_gpio_get_private(vtss_state, port_no, gpio_no, value);
    }
    VTSS_EXIT();
    return rc;
}

// See vtss_phy_api.h
vtss_rc vtss_phy_gpio_set(const vtss_inst_t          inst,
                          const vtss_port_no_t      port_no,
                          const u8                  gpio_no,
                          const BOOL                value)
{
    vtss_state_t *vtss_state;
    vtss_rc      rc;

    VTSS_ENTER();
    if ((rc = vtss_inst_port_no_check(inst, &vtss_state, port_no)) == VTSS_RC_OK) {
        rc = vtss_phy_gpio_set_private(vtss_state, port_no, gpio_no, value);
    }
    VTSS_EXIT();
    return rc;
}


#endif


