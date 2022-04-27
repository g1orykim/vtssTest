/*

Vitesse Switch API software.

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
**********************************************************************
*/

//**********************************************************************
//  This file contains functions that are general for the PoE system.
//  Depending upon which PoE chip set the hardware uses each function
//  calls the corresponding function for that chip set.
//**********************************************************************



#include "critd_api.h"
#include "poe.h"
#include "poe_custom_api.h"
#include "poe_custom_pd63000_api.h"
#include "poe_custom_si3452_api.h"
#include "poe_custom_slus787_api.h"
#include "poe_custom_pd690xx_api.h"

#if defined(VTSS_ARCH_LUTON28)
#include <cyg/io/i2c_vcoreii.h>
#else
#include <cyg/io/i2c_vcoreiii.h>
#endif

/****************************************************************************/
/*  Hardware configuration for Vitesse Enzo board.                          */
/****************************************************************************/

#define VTSS_HW_CUSTOM_POE_CHIP_PD690xx_SUPPORT 1  // Remove if autodetection of PD690xx should be disabled
#define VTSS_HW_CUSTOM_POE_CHIP_SI3452_SUPPORT  1  // Remove if autodetection of SI3452 should be disabled
#define VTSS_HW_CUSTOM_POE_CHIP_SLUS787_SUPPORT 1  // Remove if autodetection of SLUS787 should be disabled


// PD6xxxx chipset - Only ONE must be set.
//#define VTSS_HW_CUSTOM_POE_CHIP_PD63000_SUPPORT 1  // Remove if autodetection of PD63000 should be disabled
// #define VTSS_HW_CUSTOM_POE_CHIP_PD69100_SUPPORT 1  // Remove if autodetection of PD61000 should be disabled.

// Gets the hardware board configuration for PoE
//
// In : port_idx - The port index (starting from 0) for which you know the board configuration (e.g. which I2C address uses the port for the PoE chip)
//
// In/Out : hw_conf - The hardware board configuration.
//
// Return : The hardware board configuration. (same as hw_conf)
poe_custom_entry_t poe_custom_get_hw_config(vtss_port_no_t port_idx, poe_custom_entry_t *hw_conf)
{
    hw_conf->pse_pairs_control_ability = 0; // 0 = hardware has fixed PSE pinout, 1 = Hardware PSE pin is configurable. See pethPsePortPowerPairsControlAbility, rfc 3621
    hw_conf->pse_power_pair    = 1; // 1 = Signal pins pinout, 2 =  Spare pins pinout, ee pethPsePortPowerPairs, rfc 3621


    // Set default values
    hw_conf->available = false;
    hw_conf->i2c_addr[SI3452]  = 0x00;
    hw_conf->i2c_addr[SLUS787] = 0x00;
    hw_conf->i2c_addr[PD63000] = 0x20; // The Pd63000 controls all PoE chips via their uController, so all PoE ports use the same I2C address.
    hw_conf->i2c_addr[PD690xx] = 0x00;
    hw_conf->i2c_addr[NO_POE_CHIPSET_FOUND] = 0x00;

    // Define which ports that supports PoE/PoE+.
    switch (port_idx) {
    case 0:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x21;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        hw_conf->i2c_addr[SLUS787] = 0x20;
        break;
    case 1:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x21;
        hw_conf->i2c_addr[SLUS787] = 0x20;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 2:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x21;
        hw_conf->i2c_addr[SLUS787] = 0x20;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 3:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x21;
        hw_conf->i2c_addr[SLUS787] = 0x20;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 4:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x25;
        hw_conf->i2c_addr[SLUS787] = 0x21;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 5:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x25;
        hw_conf->i2c_addr[SLUS787] = 0x21;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 6:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x25;
        hw_conf->i2c_addr[SLUS787] = 0x21;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 7:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x25;
        hw_conf->i2c_addr[SLUS787] = 0x21;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 8:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x29;
        hw_conf->i2c_addr[SLUS787] = 0x22;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 9:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x29;
        hw_conf->i2c_addr[SLUS787] = 0x22;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 10:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x29;
        hw_conf->i2c_addr[SLUS787] = 0x22;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 11:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x29;
        hw_conf->i2c_addr[SLUS787] = 0x22;
        hw_conf->i2c_addr[PD690xx] = 0x00;
        break;

    case 12:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2B;
        hw_conf->i2c_addr[SLUS787] = 0x23;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 13:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2B;
        hw_conf->i2c_addr[SLUS787] = 0x23;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 14:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2B;
        hw_conf->i2c_addr[SLUS787] = 0x23;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 15:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2B;
        hw_conf->i2c_addr[SLUS787] = 0x23;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;


    case 16:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2D;
        hw_conf->i2c_addr[SLUS787] = 0x24;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 17:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2D;
        hw_conf->i2c_addr[SLUS787] = 0x24;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 18:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2D;
        hw_conf->i2c_addr[SLUS787] = 0x24;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 19:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2D;
        hw_conf->i2c_addr[SLUS787] = 0x24;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 20:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2F;
        hw_conf->i2c_addr[SLUS787] = 0x25;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 21:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2F;
        hw_conf->i2c_addr[SLUS787] = 0x25;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 22:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2F;
        hw_conf->i2c_addr[SLUS787] = 0x25;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 23:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2F;
        hw_conf->i2c_addr[SLUS787] = 0x25;
        hw_conf->i2c_addr[PD690xx] = 0x78;
        break;

    case 24:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x20;
        hw_conf->i2c_addr[SLUS787] = 0x28;
        break;

    case 25:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x20;
        hw_conf->i2c_addr[SLUS787] = 0x28;
        break;

    case 26:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x20;
        hw_conf->i2c_addr[SLUS787] = 0x28;
        break;

    case 27:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x20;
        hw_conf->i2c_addr[SLUS787] = 0x28;
        break;

    case 28:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x24;
        hw_conf->i2c_addr[SLUS787] = 0x29;
        break;

    case 29:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x24;
        hw_conf->i2c_addr[SLUS787] = 0x29;
        break;


    case 30:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x24;
        hw_conf->i2c_addr[SLUS787] = 0x29;
        break;

    case 31:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x24;
        hw_conf->i2c_addr[SLUS787] = 0x29;
        break;


    case 32:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x28;
        hw_conf->i2c_addr[SLUS787] = 0x2A;
        break;

    case 33:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x28;
        hw_conf->i2c_addr[SLUS787] = 0x2A;
        break;

    case 34:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x28;
        hw_conf->i2c_addr[SLUS787] = 0x2A;
        break;

    case 35:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x28;
        hw_conf->i2c_addr[SLUS787] = 0x2A;
        break;

    case 36:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2A;
        hw_conf->i2c_addr[SLUS787] = 0x2B;
        break;

    case 37:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2A;
        hw_conf->i2c_addr[SLUS787] = 0x2B;
        break;

    case 38:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2A;
        hw_conf->i2c_addr[SLUS787] = 0x2B;
        break;

    case 39:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2A;
        hw_conf->i2c_addr[SLUS787] = 0x2B;
        break;

    case 40:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2C;
        hw_conf->i2c_addr[SLUS787] = 0x2C;
        break;

    case 41:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2C;
        hw_conf->i2c_addr[SLUS787] = 0x2C;
        break;

    case 42:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2C;
        hw_conf->i2c_addr[SLUS787] = 0x2C;
        break;

    case 43:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2C;
        hw_conf->i2c_addr[SLUS787] = 0x2C;
        break;

    case 44:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2E;
        hw_conf->i2c_addr[SLUS787] = 0x2D;
        break;

    case 45:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2E;
        hw_conf->i2c_addr[SLUS787] = 0x2D;
        break;

    case 46:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2E;
        hw_conf->i2c_addr[SLUS787] = 0x2D;
        break;

    case 47:
        hw_conf->available = TRUE;
        hw_conf->i2c_addr[SI3452]  = 0x2E;
        hw_conf->i2c_addr[SLUS787] = 0x2D;
        break;

    default:
        break;
    }

    return *hw_conf;
}


//**********************************************************************
//* Generic functions that is called by the host module
//**********************************************************************



void poe_init_configuration_applied(BOOL *already_initalized, BOOL *configuration_initialized_done)
{

}


// Function that shall be called after reset / board boot.
void poe_custom_init(void)
{
    vtss_port_no_t port_index;
    for (port_index = 0 ; port_index < VTSS_PORTS ; port_index++) {
        if (poe_custom_is_chip_found(port_index) == PD63000) {
            pd63000_poe_init();
            break; // At the moment this is all handled in the PD63000 driver.
        } else if (poe_custom_is_chip_found(port_index) == SI3452) {
            si3452_poe_init(port_index);
        } else if (poe_custom_is_chip_found(port_index) == SLUS787) {
            slus787_poe_init(port_index);
        } else if (poe_custom_is_chip_found(port_index) == PD690xx) {
            pd690xx_poe_init();
        }
        cyg_thread_yield(); // Make sure that other threads get access to the i2c controller.
    }
}


// Poe Chipset reset - Should be called upon INIT_CMD_INIT
void poe_custom_reset_poe_chipset(void)
{
    if (poe_custom_is_chip_found(0) == PD63000) {
        (void) pd63000_reset_command();
    }
}

// PoE chipset restore to default
void poe_custom_restore_to_default (void)
{
    if (poe_custom_is_chip_found(0) == PD63000) {
        pd63000_factory_default();
    }
}

// Enable or disable a port
void poe_custom_port_enable(vtss_port_no_t port_index, BOOL enable, poe_mode_t mode)
{

    // We don't want to update the PoE chipset if there is not change so we keep track up the current settings.
    static BOOL first = TRUE;
    static BOOL configuration_init[VTSS_PORTS]; // used to keep track of if the configuration has been applied.
    static BOOL last_enable[VTSS_PORTS];        // Stores the current enable setting in the poe chipset
    static poe_mode_t last_mode[VTSS_PORTS];    // Stores the current mode setting in the poe chipset
    int i;

    // If this is the first access then store that the port settings haven't been applied yet.
    if (first) {
        first = FALSE;
        for (i = 0 ; i < VTSS_PORTS ; i++) {
            configuration_init[i] = FALSE;
        }
    }

    T_DG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "poe_custom_port_enable, Enabled:%d, last_enable:%d, mode:%d, last_mode:%d",
              enable, last_enable[port_index], mode, last_mode[port_index]);

    // Only update if the configuration has changed or if the configuration hasn't been applied yet.
    if (last_enable[port_index] != enable || last_mode[port_index] != mode || configuration_init[port_index] == FALSE) {
        configuration_init[port_index] = TRUE;

        if (poe_custom_is_chip_found(port_index) == PD63000) {
            pd63000_set_enable_disable_channel(port_index, enable, mode);
        } else if (poe_custom_is_chip_found(port_index) == SI3452) {
            si3452_poe_enable(port_index, enable);
        } else if (poe_custom_is_chip_found(port_index) == SLUS787) {
            slus787_poe_enable(port_index, enable);
        } else if (poe_custom_is_chip_found(port_index) == PD690xx) {
            pd690xx_poe_enable(port_index, enable, mode);
        }
    }

    // Remember the settings in the poe chipset.
    last_enable[port_index] = enable;
    last_mode[port_index] = mode;
}

// Power management
void poe_custom_pm(poe_stack_conf_t *poe_stack_cfg)
{
    if (poe_custom_is_chip_found(0) == PD63000) {
        pd63000_set_pm_method(poe_stack_cfg);
    }
}

// Set max. power per port in deci watts
void poe_custom_set_port_max_power(vtss_port_no_t port_index, int max_port_power)
{
    static BOOL first = TRUE;
    static int last_max_port_power[VTSS_PORTS];


    int i;
    // Initialize last_max_port_power first time we enter this function.
    if (first) {
        first = FALSE;
        for (i = 0 ; i < VTSS_PORTS ; i++) {
            last_max_port_power[i] = 0;    // Set to a value the will force registers to be updated
        }
    }

    T_NG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "Max Power:%d, last Max power:%d", max_port_power, last_max_port_power[port_index]);

    // Since access to PoE registers can take quite a bit of time, we only updates the registers when
    // the max_port_power has changed.
    if (last_max_port_power[(int) port_index] == max_port_power) {
        return ;
    }



    last_max_port_power[(int) port_index] = max_port_power;

    // Make sure that we don't configure max power to more then the chip set supports.
    if (max_port_power > poe_custom_get_port_power_max(port_index)) {
        max_port_power = poe_custom_get_port_power_max(port_index);
    }

    if (poe_custom_is_chip_found(port_index) == PD63000) {
        pd63000_set_power_limit_channel(port_index, max_port_power);
    } else if (poe_custom_is_chip_found(port_index) == SI3452) {
        si3452_set_power_limit_channel(port_index, max_port_power);
    } else if (poe_custom_is_chip_found(port_index) == SLUS787) {
        slus787_set_power_limit_channel(port_index, max_port_power);
    } else if (poe_custom_is_chip_found(port_index) == PD690xx) {
        pd690xx_set_power_limit_channel(port_index, max_port_power);
    }
}

// Configuration of the port priority
void poe_custom_set_port_priority(vtss_port_no_t port_index, poe_priority_t priority)
{
    if (poe_custom_is_chip_found(port_index) == PD63000) {
        pd63000_set_port_priority(port_index, priority);
    }
}


// Returns Minimum power required for the power supply
u16 poe_custom_get_power_supply_min(void)
{
    return 0;
}

// Returns Maximum power supported for a port (in deciWatts)
u16 poe_custom_get_port_power_max(vtss_port_no_t port_index)
{
    return 300; // PoE+ maximum power.
}


// Get which power supply source the switch is running on.
poe_power_source_t poe_custom_get_power_source(void)
{
    if (poe_custom_is_chip_found(0) == PD63000) {
        return pd63000_get_power_source();
    } else {
        // Hardware only supports one power supply.
        return PRIMARY;
    }

}

// Function that return TRUE is backup power supply is supported.
BOOL poe_custom_is_backup_power_supported (void)
{
    if (poe_custom_is_chip_found(0) == PD63000) {
        return FALSE;
    } else {
        return FALSE;
    }
}


// Maximum power for the power supply.
void poe_custom_set_power_supply(int primary_max_power, int backup_max_power)
{

    static int current_primary_max_power = 0;
    static int current_backup_max_power  = 0;

    // We only want to change the settings if they have changed
    if (current_primary_max_power == primary_max_power &&
        current_backup_max_power == backup_max_power) {
        return;
    }

    current_backup_max_power  = backup_max_power;
    current_primary_max_power = primary_max_power;

    T_RG(VTSS_TRACE_GRP_CUSTOM, "Entering poe_custom_set_power_supply");

    if (primary_max_power < poe_custom_get_power_supply_min()) {
        primary_max_power = poe_custom_get_power_supply_min();
    }

    if (backup_max_power < poe_custom_get_power_supply_min()) {
        backup_max_power = poe_custom_get_power_supply_min();
    }

    if (primary_max_power > POE_POWER_SUPPLY_MAX) {
        primary_max_power = POE_POWER_SUPPLY_MAX;
    }

    if (backup_max_power > POE_POWER_SUPPLY_MAX) {
        backup_max_power = POE_POWER_SUPPLY_MAX;
    }

    if (poe_custom_is_chip_found(0) == PD63000) {
        T_NG(VTSS_TRACE_GRP_CUSTOM, "Setting Power Banks for PD63000");
        pd63000_set_power_banks(primary_max_power, backup_max_power);
    }
}

// Get status ( Real time current and power cunsumption )
void poe_custom_get_status(poe_status_t *poe_status)
{
    vtss_port_no_t port_index;
    for (port_index = 0 ; port_index < VTSS_PORTS; port_index++) {
        if (poe_custom_is_chip_found(port_index) == PD63000) {
            pd63000_get_port_measurement(port_index, poe_status); // Gets power consumption and current
        } else if (poe_custom_is_chip_found(port_index) == SI3452) {
            si3452_get_port_measurement(poe_status, port_index);
        } else if (poe_custom_is_chip_found(port_index) == SLUS787) {
            slus787_get_port_measurement(poe_status, port_index);
        } else if (poe_custom_is_chip_found(port_index) == PD690xx) {
            pd690xx_get_port_measurement(poe_status, port_index);
        }
    }
}


// Updates the power reserved fields in the poe_status with the power given by the class.
void poe_custom_get_all_ports_classes(char *classes)
{
    vtss_port_no_t port_index;
    for (port_index = 0 ; port_index < VTSS_PORTS; port_index++) {
        if (poe_custom_is_chip_found(port_index) == PD63000) {
            pd63000_get_all_port_class(classes);
            break; // At the moment this is all handled in the PD63000 driver.
        } else if (poe_custom_is_chip_found(port_index) == SI3452) {
            si3452_get_all_port_class(classes, port_index);
        } else if (poe_custom_is_chip_found(port_index) == SLUS787) {
            slus787_get_all_port_class(classes, port_index);
        } else if (poe_custom_is_chip_found(port_index) == PD690xx) {
            pd690xx_get_all_port_class(classes, port_index);
        }
    }
}



// Function for getting port status for a single port.
// In - Port_index - The port for which top get status
// Out - port_status - Pointer to where to put the status
void poe_custom_port_status_get(vtss_port_no_t port_index, poe_port_status_t *port_status)
{
    if (poe_custom_is_chip_found(port_index) == PD63000) {
        pd63000_port_status_get(port_index, port_status);
    } else if (poe_custom_is_chip_found(port_index) == SI3452) {
        si3452_port_status_get(port_status, port_index);
    } else if (poe_custom_is_chip_found(port_index) == SLUS787) {
        slus787_port_status_get(port_status, port_index);
    } else if (poe_custom_is_chip_found(port_index) == PD690xx) {
        pd690xx_port_status_get(port_status, port_index);
    } else {
        // Well, no PoE chip sets found at all......
        port_status[port_index] = POE_NOT_SUPPORTED;
    }
}


// Update the port status struct
void poe_custom_get_all_ports_status(poe_port_status_t *port_status)
{
    vtss_port_no_t port_index;
    for (port_index = 0 ; port_index < VTSS_PORTS; port_index++) {
        if (poe_custom_is_chip_found(port_index) == PD63000) {
            pd63000_get_all_ports_status(port_status);
            break; // At the moment this is all handled in the PD63000 driver.
        } else if (poe_custom_is_chip_found(port_index) == SI3452) {
            si3452_port_status_get(port_status, port_index);
        } else if (poe_custom_is_chip_found(port_index) == SLUS787) {
            slus787_port_status_get(port_status, port_index);
        } else if (poe_custom_is_chip_found(port_index) == PD690xx) {
            pd690xx_port_status_get(port_status, port_index);
        } else {
            // Well, no PoE chip sets found at all......
            port_status[port_index] = POE_NOT_SUPPORTED;
        }
    }
}


// Function that returns which PoE chip set is found (PoE chip sets are auto detected).
// If not PoE chip is found, NO_POE_CHIPSET_FOUND is returned.
poe_chipset_t poe_custom_is_chip_found(vtss_port_no_t port_index)
{
    // Get PoE hardware board configuration
    poe_custom_entry_t poe_hw_conf;

    if (!(vtss_board_features() & VTSS_BOARD_FEATURE_POE)) {
        T_IG(VTSS_TRACE_GRP_CUSTOM, "No PoE Board feature");
        return NO_POE_CHIPSET_FOUND;
    }

    poe_hw_conf = poe_custom_get_hw_config(port_index, &poe_hw_conf);

    if (!poe_hw_conf.available) {
        T_RG(VTSS_TRACE_GRP_CUSTOM, "PoE not supported for port_index:%u", port_index);
        return NO_POE_CHIPSET_FOUND;
    }

#ifdef VTSS_HW_CUSTOM_POE_CHIP_SI3452_SUPPORT
    if (si3452_is_chip_available(port_index)) {
        return SI3452;
    }
#endif


#ifdef VTSS_HW_CUSTOM_POE_CHIP_SLUS787_SUPPORT
    if (slus787_is_chip_available(port_index)) {
        return SLUS787;
    }
#endif


#ifdef VTSS_HW_CUSTOM_POE_CHIP_PD690xx_SUPPORT
    if (pd690xx_is_chip_available(port_index)) {
        return PD690xx;
    }
#endif


// PD6xxxx Must be last because it can take very long time to detect.
#ifdef VTSS_HW_CUSTOM_POE_CHIP_PD63000_SUPPORT
    if (pd63000_is_chip_available()) {
        T_RG(VTSS_TRACE_GRP_CUSTOM, "pd63000_is_chip_available");
        return PD63000;
    } else {
        T_RG(VTSS_TRACE_GRP_CUSTOM, "NOT pd63000_is_chip_available");
    }
#endif


// Must be last because it can take very long time to detect.
#ifdef VTSS_HW_CUSTOM_POE_CHIP_PD69100_SUPPORT
    if (pd63000_is_chip_available()) {
        return PD63000;
    }
#endif

    T_RG_PORT(VTSS_TRACE_GRP_CUSTOM, port_index, "NO_POE_CHIPSET_FOUND");
    return NO_POE_CHIPSET_FOUND;
}


//
// Debug functions
//

// Debug function for converting PoE chipset to a printable text string.
// In : poe_chipset - The PoE chipset type.
//      buf        - Pointer to a buffer that can contain the printable Poe chipset string.
i8 *poe_chipset2txt(poe_chipset_t poe_chipset, i8 *buf)
{
    strcpy (buf, "-"); // Default to no chipset.

    if (poe_chipset == PD63000) {
        strcpy(buf, "MicroSemi PD69300");
    }

    if (poe_chipset == SI3452) {
        strcpy(buf, "SilliconLabs 3452");
    }

    if (poe_chipset == SLUS787) {
        strcpy(buf, "Texas Instruments slus787");
    }

    if (poe_chipset == PD690xx) {
        strcpy(buf, "MicroSemi PD69xx");
    }
    return buf;
}

// Debug function for writing PoE chipset registers
// In : port_index - The port at which the PoE chipset is matched to.
//      addr       - The PoE chipset register address
//      Data       - The data to be written to the PoE chipset.
void poe_custom_wr(vtss_port_no_t port_index, u16 addr, u16 data)
{
    if (poe_custom_is_chip_found(port_index) == PD63000) {
//        (void) pd63000_wr(data, addr);
    } else if (poe_custom_is_chip_found(port_index) == PD690xx) {
        pd690xx_device_wr(port_index, addr, data);
    } else if (poe_custom_is_chip_found(port_index) == SLUS787) {
        slus787_device_wr(port_index, addr, data);
    } else if (poe_custom_is_chip_found(port_index) == SI3452) {
        si3452_device_wr(port_index, addr, data);
    }
}

// Debug function for reading PoE chipset registers
// In : port_index - The port at which the PoE chipset is matched to.
//      addr       - The PoE chipset register address
// Out: Data       - Pointer to the read result
void poe_custom_rd(vtss_port_no_t port_index, u16 addr, u16 *data)
{
    u8 data_u8 = 0;
    if (poe_custom_is_chip_found(0) == PD63000) {
//        (void) pd63000_rd(data, addr);
    } else if (poe_custom_is_chip_found(port_index) == PD690xx) {
        pd690xx_device_rd(port_index, addr, data);
    } else if (poe_custom_is_chip_found(port_index) == SLUS787) {
        slus787_device_rd(port_index, (char) addr, &data_u8, 1);
        *data = (u16) data_u8;
    } else if (poe_custom_is_chip_found(port_index) == SI3452) {
        si3452_device_rd(port_index, (char) addr, &data_u8);
        *data = (u16) data_u8;
    }
}

// Debug function for setting the legacy capacitor detection
// In : port_index - The port at which the PoE chipset is matched to.
//      enable     - True to enable legacy capacitor detection. False to disable.
void poe_custom_capacitor_detection_set(vtss_port_no_t port_index, BOOL enable)
{
    if (poe_custom_is_chip_found(port_index) == PD690xx) {
        pd690xx_capacitor_detection_set(port_index, enable);
    } else if (poe_custom_is_chip_found(port_index) == PD63000) {
        pd63000_capacitor_detection_set(enable);
    } else {
        T_W("Capacitor detect setting not support for this chipset");
    }
}

// Debug function for getting current state of the legacy capacitor detection
// In : port_index - The port at which the PoE chipset is matched to.
//      enable     - True to enable legacy capacitor detection. False to disable.
BOOL poe_custom_capacitor_detection_get(vtss_port_no_t port_index)
{
    if (poe_custom_is_chip_found(port_index) == PD690xx) {
        return pd690xx_capacitor_detection_get(port_index);
    } else if (poe_custom_is_chip_found(port_index) == PD63000) {
        return pd63000_capacitor_detection_get();
    } else {
        T_W("Capacitor detect setting not support for this chipset");
        return FALSE;
    }
}

// Debug function for getting firmware information
// In : port_index - The port at which the PoE chipset is matched to.
// Out : info_txt - Pointer to printable string with firmware information
i8 *poe_custom_firmware_info_get(vtss_port_no_t port_index, i8 *info)
{
    if (poe_custom_is_chip_found(port_index) == PD690xx) {
        strcpy(info, "-");
    } else if (poe_custom_is_chip_found(port_index) == PD63000) {
        pd63000_get_sw_version(info);
    } else {
        strcpy(info, "-");
    }
    return info;
}


// Debug function for updating the PoE chipset firmware
void poe_download_firmware(u8 *firmware, u32 firmware_size)
{
#ifdef FIRMWARE_UPDATE_SUPPORTED
//    if (poe_custom_is_chip_found(0) == PD63000) {
    DownloadFirmwareFunc(firmware, firmware_size);
//    }
#endif
}


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
