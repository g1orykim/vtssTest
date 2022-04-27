/*

Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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


#ifndef _VTSS_POE_CUSTOM_API_H_
#define _VTSS_POE_CUSTOM_API_H_
#include "poe_api.h"

/**********************************************************************
* Struct for defining PoE capabilites for each port
**********************************************************************/
typedef struct {
    BOOL available;    /* True is PoE is available for the port  */
    int i2c_addr[NO_POE_CHIPSET_FOUND + 1];
    u8   pse_pairs_control_ability;  // pethPsePortPowerPairsControlAbility, rfc3621
    u8   pse_power_pair;             // pethPsePortPowerPairs, rfc3621
} poe_custom_entry_t;



/**********************************************************************
** Generic function for controlling the PoE chipsets
**********************************************************************/
void poe_custom_reset_poe_chipset(void);    // Reseting the PoE chip set
void poe_custom_restore_to_default(void);   // PoE chip restore to default
void poe_custom_port_enable(vtss_port_no_t port_index, BOOL enable, poe_mode_t mode); // Enable a port in the PoE chipset
void poe_custom_set_port_max_power(vtss_port_no_t port_index, int max_port_power); // Configuration of maximum power per port
void poe_custom_set_port_priority(vtss_port_no_t port_index, poe_priority_t priority) ; // Configuration of port priority
void poe_custom_set_power_supply(int primary_max_power, int backup_max_power); // Maximum power for the power supply.
void poe_custom_get_status(poe_status_t *poe_status); // Updates the poe_status structure.
void poe_custom_port_status_get(vtss_port_no_t port_index, poe_port_status_t *port_status); // Getting status for a single port
void poe_set_power_limit_channel(char channel, int ppl ); // Sets power limit for a port
void poe_custom_init(void); // Initialise the PoE chip
void poe_custom_pm(poe_stack_conf_t *poe_stack_cfg);  // Configuration of power management
void poe_custom_get_all_ports_classes(char *classes);// updates the power reserved fields according to the PD's detected class

void poe_custom_get_all_ports_status(poe_port_status_t *port_status);

u16  poe_custom_get_power_supply_min(void);// Returns Minimum power required for the power supply
u16  poe_custom_get_port_power_max(vtss_port_no_t port_index);// Returns Maximum power supported for a port (in DeciWatts)
BOOL poe_custom_is_backup_power_supported (void); // Returns TRUE if backup power supply is supported by the HW.


poe_chipset_t poe_custom_is_chip_found(vtss_port_no_t port_index); // Function that returns which PoE chip that have been found
poe_power_source_t poe_custom_get_power_source(void); // Function that returns which power source the host is using right now,
poe_custom_entry_t poe_custom_get_hw_config(vtss_port_no_t port_idx, poe_custom_entry_t *hw_conf);

//
// Debug functions
//
void poe_custom_wr(vtss_port_no_t port_index, u16 addr, u16 data);
void poe_custom_rd(vtss_port_no_t port_index, u16 addr, u16 *data);
void poe_custom_capacitor_detection_set(vtss_port_no_t port_index, BOOL enable);
BOOL poe_custom_capacitor_detection_get(vtss_port_no_t port_index);

// Debug function for converting PoE chipset to a printable text string.
// In : poe_chipset - The PoE chipset type.
//      buf        - Pointer to a buffer that can contain the printable Poe chipset string.
i8   *poe_chipset2txt(poe_chipset_t poe_chipset, i8 *buf);
i8   *poe_custom_firmware_info_get(vtss_port_no_t port_index, i8 *info);
void poe_download_firmware(u8 *firmware, u32 firmware_size);


/**********************************************************************
** Misc.
**********************************************************************/

// Defines the maximum number of retries that we shall try an re-transmit
// I2C accesses before giving up.
#define I2C_RETRIES_MAX 2

// Default power supply maximum configuration in watts
#define POE_POWER_SUPPLY_MAX 2000


// Set this define if PoE chipset firmware shall be supported
#define FIRMWARE_UPDATE_SUPPORTED 1


#endif // _VTSS_POE_CUSTOM_API_H_
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
