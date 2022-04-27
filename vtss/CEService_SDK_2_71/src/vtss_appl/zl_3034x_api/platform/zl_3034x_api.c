/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

*/

#include "main.h"
#include "vtss_types.h"
#include "vtss_api.h"
#include "zl_3034x_api_api.h"
#include "zl_3034x_api.h"

// xl3034x base interface 
#include "zl303xx_Api.h"
#include "zl303xx_Var.h"
#include "zl303xx_DebugMisc.h"
#include "zl303xx_TsEngInternal.h"
#include "interrupt_api.h"

/* interface to other components */
#include "vtss_tod_api.h"

#ifdef VTSS_SW_OPTION_CLI
#include "zl_3034x_api_cli.h"
#endif

//#define ZL_SPI_WRITE_DEBUG

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */


#if (VTSS_TRACE_ENABLED)

static vtss_trace_reg_t trace_reg = {
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "zl_api",
    .descr     = "ZL3034x DPLL API."
};

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] = {
    [VTSS_TRACE_GRP_DEFAULT] = {
        .name      = "default",
        .descr     = "Default (ZL3034x core)",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
    [TRACE_GRP_CRIT] = {
        .name      = "crit",
        .descr     = "Critical Regions",
        .lvl       = VTSS_TRACE_LVL_ERROR,
        .timestamp = 1,
        .usec = 1,
    },
    [TRACE_GRP_OS_PORT] = {
        .name      = "os_port",
        .descr     = "ZL OS porting functions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
    [TRACE_GRP_SYNC_INTF] = {
        .name      = "sync_intf",
        .descr     = "ZL-Synce application interfacefunctions",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
    [TRACE_GRP_ZL_TRACE] = {
        .name      = "zl-trace",
        .descr     = "ZL-code trace level",
        .lvl       = VTSS_TRACE_LVL_WARNING,
        .timestamp = 1,
        .usec = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

#ifdef VTSS_ARCH_LUTON26
#include <cyg/io/spi_vcoreiii.h>
#define SYNCE_SPI_GPIO_CS 10
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev_zl, VTSS_SPI_CS_NONE, SYNCE_SPI_GPIO_CS);
#endif

#ifdef VTSS_ARCH_JAGUAR_1
#include <cyg/io/spi_vcoreiii.h>
#define SYNCE_SPI_GPIO_CS 17
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev_zl, 2, VTSS_SPI_GPIO_NONE);
#endif

#ifdef VTSS_ARCH_SERVAL
#include <cyg/io/spi_vcoreiii.h>
#define SYNCE_SPI_GPIO_CS 6
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev_zl, VTSS_SPI_CS_NONE, SYNCE_SPI_GPIO_CS);
#endif


/* ================================================================= *
 *  Configuration definitions
 * ================================================================= */
zl303xx_ParamsS *zl_3034x_zl303xx_params; /* device instance returned from zl303xx_CreateDeviceInstance*/


static BOOL zarlink=FALSE;


/****************************************************************************
 * Configuration API
 ****************************************************************************/


vtss_rc zl_3034x_register_get(const u32   page,
                              const u32   reg,
                              u32 *const  value)
{
    *value = 0;

    if (!zarlink) return(VTSS_RC_OK);

    ZL_3034X_CHECK(zl303xx_Read(zl_3034x_zl303xx_params, NULL, ZL303XX_MAKE_MEM_ADDR((Uint32T)page, (Uint32T)reg, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE), (Uint32T *)value));

    return(VTSS_RC_OK);
}



vtss_rc zl_3034x_register_set(const u32   page,
                              const u32   reg,
                              const u32   value)
{
    if (!zarlink) return(VTSS_RC_OK);

    ZL_3034X_CHECK(zl303xx_Write(zl_3034x_zl303xx_params, NULL, ZL303XX_MAKE_MEM_ADDR((Uint32T)page, (Uint32T)reg, ZL303XX_MEM_SIZE_1_BYTE, ZL303XX_MEM_OVRLY_NONE), (Uint32T)value));

    return(VTSS_RC_OK);
}


void zl_3034x_spi_read(u32 address, u8 *data, u32 size)
{
    cyg_uint8 tx_data[21], rx_data[21];

    if (size > 20)  return;

    memset(tx_data, 0, sizeof(tx_data));

    address |= 0x80;    /* Set first bit to indicate read */
    tx_data[0] = address;
    cyg_spi_transfer((cyg_spi_device*)&dev_zl, 0, size+1, tx_data, rx_data);
    memcpy(data, &rx_data[1], size);
}


void zl_3034x_spi_write(u32 address, u8 *data, u32 size)
{
    cyg_uint8 tx_data[21], rx_data[21];
    
    if (size > 20)  return;

#if defined (ZL_SPI_WRITE_DEBUG)
    char data_string[80];
    int str_idx = 0;
    data_string[0] = 0;
    uint idx;
    for (idx = 0; idx < size;idx++) {
        str_idx += sprintf(data_string+str_idx, " %x",data[idx]);
    }
    T_W("address %x, size %d, data %s", address, size, data_string);
#endif
    memcpy(&tx_data[1], data, size);

    address &= 0x7F;    /* Clear first bit to indicate write */
    tx_data[0] = address;
    cyg_spi_transfer((cyg_spi_device*)&dev_zl, 0, size+1, tx_data, rx_data);
}


vtss_rc zl_3034x_debug_pll_status(void)
{
    vtss_rc rc = VTSS_RC_OK;
    if (!zarlink) return(VTSS_RC_OK);
    if (zl303xx_DebugPllStatus(zl_3034x_zl303xx_params) != ZL303XX_OK) {
        T_D("Error during Zarlink pll status get");
        rc = VTSS_RC_ERROR;
    }
    return(rc);

}

vtss_rc zl_3034x_debug_hw_ref_status(u32 ref_id)
{
    vtss_rc rc = VTSS_RC_OK;
    if (!zarlink) return(VTSS_RC_OK);
    if (zl303xx_DebugHwRefStatus(zl_3034x_zl303xx_params, ref_id) != ZL303XX_OK) {
        T_D("Error during Zarlink hw ref status get");
        rc = VTSS_RC_ERROR;
    }
    return(rc);
}

vtss_rc zl_3034x_debug_hw_ref_cfg(u32 ref_id)
{
    vtss_rc rc = VTSS_RC_OK;
    if (!zarlink) return(VTSS_RC_OK);
    if (zl303xx_DebugHwRefCfg(zl_3034x_zl303xx_params, ref_id) != ZL303XX_OK) {
        T_D("Error during Zarlink hw ref cfg get");
        rc = VTSS_RC_ERROR;
    }
    return(rc);
}

vtss_rc zl_3034x_debug_dpll_status(u32 pll_id)
{
    vtss_rc rc = VTSS_RC_OK;
    if (!zarlink) return(VTSS_RC_OK);
    if (zl303xx_DebugDpllStatus(zl_3034x_zl303xx_params, pll_id) != ZL303XX_OK) {
        T_D("Error during Zarlink dpll status get");
        rc = VTSS_RC_ERROR;
    }
    return(rc);
}

vtss_rc zl_3034x_debug_dpll_cfg(u32 pll_id)
{
    vtss_rc rc = VTSS_RC_OK;
    if (!zarlink) return(VTSS_RC_OK);
    if (zl303xx_DebugDpllConfig(zl_3034x_zl303xx_params, pll_id) != ZL303XX_OK) {
        T_D("Error during Zarlink dpll cfg get");
        rc = VTSS_RC_ERROR;
    }
    return(rc);
}


vtss_rc zl_3034x_api_init(vtss_init_data_t *data)
{
    vtss_isid_t isid = data->isid;
    
    u8 reg;

    switch (data->cmd) {
    case INIT_CMD_INIT:
        /* Initialize and register trace ressources */
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
#if defined(VTSS_ARCH_SERVAL) || defined(VTSS_ARCH_LUTON26)
        if (vtss_gpio_mode_set(NULL, 0, SYNCE_SPI_GPIO_CS, VTSS_GPIO_OUT) != VTSS_RC_OK)
            T_D("Error during vtss_gpio_mode_set()");
        if (vtss_gpio_write(NULL, 0, SYNCE_SPI_GPIO_CS, TRUE) != VTSS_RC_OK)
            T_D("Error during vtss_gpio_write()");
        dev_zl.delay = 1;
#endif
#ifdef VTSS_ARCH_JAGUAR_1
        if (vtss_gpio_mode_set(NULL, 0, SYNCE_SPI_GPIO_CS, VTSS_GPIO_ALT_0) != VTSS_RC_OK)
            T_D("Error during vtss_gpio_mode_set()");
        dev_zl.delay = 1;
#endif
#ifdef VTSS_SW_OPTION_CLI
        zl_3034x_api_cli_req_init();
#endif

        T_D("INIT_CMD_INIT ZL_3034X" );
        break;
    case INIT_CMD_START:
        T_D("INIT_CMD_START ZL_3034X");

        reg = 0;
        zl_3034x_spi_write(0x64, &reg, 1);   /* Set page register to '0' */
        zl_3034x_spi_read(0x00, &reg, 1);   /* read ID register */
        zarlink = (((reg & 0x1F) == 0x0C) || ((reg & 0x1F) == 0x0D));
        if (!zarlink) {
            T_I("Zarlink device NOT detected");
            break;
        }

        T_D("apr_env_init finished");
        ZL_3034X_CHECK(zl303xx_ReadWriteInit());

        /* Create a HW Driver */
        ZL_3034X_CHECK(zl303xx_CreateDeviceInstance(&zl_3034x_zl303xx_params));
        T_I("device instance created %p", zl_3034x_zl303xx_params);

        zl_3034x_zl303xx_params->spiParams.deviceType = ZL3034X_DEVICETYPE;
        
        ZL_3034X_CHECK(zl303xx_InitDeviceIdAndRev(zl_3034x_zl303xx_params));
        T_D("device instance created %p", zl_3034x_zl303xx_params);
        
        ZL_3034X_CHECK(zl303xx_Write(zl_3034x_zl303xx_params, NULL, ZL303XX_TOP_ISR0_MASK_REG, 0x00));  /* Disable all interrupt from Zarlink */
        ZL_3034X_CHECK(zl303xx_Write(zl_3034x_zl303xx_params, NULL, ZL303XX_TSENG_ISR_MASK_REG, 0x00)); /* Disable all TS interrupt as we do not use is now and it generates heavy interrupt load */
        T_D("INIT_CMD_START - ISID %u", isid);
        break;
    case INIT_CMD_CONF_DEF:
        T_D("INIT_CMD_CONF_DEF ZL_3034X" );
        break;
    case INIT_CMD_MASTER_UP:
        T_D("INIT_CMD_MASTER_UP ");
        break;
    case INIT_CMD_SWITCH_ADD:
        if (!zarlink)   break;
       
        break;
    case INIT_CMD_MASTER_DOWN:
        T_D("INIT_CMD_MASTER_DOWN - ISID %u", isid);
        break;
    default:
        break;
    }

    return 0;
}


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

