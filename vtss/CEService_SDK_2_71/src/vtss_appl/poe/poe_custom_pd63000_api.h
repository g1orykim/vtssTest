/*

Vitesse Switch Software.

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
****************************************************************************/

void pd63000_reset_command (void);
void pd63000_factory_default (void);
void pd63000_set_enable_disable_channel(char channel, char enable, poe_mode_t mode);
void pd63000_set_pm_method(poe_stack_conf_t *poe_stack_cfg);
void pd63000_set_power_limit_channel(char channel, uint ppl );
void pd63000_set_port_priority(char channel, poe_priority_t priority );
void pd63000_get_port_measurement(vtss_port_no_t port_index, poe_status_t *poe_status);
void pd63000_set_power_banks(int primary_power_limit, int backup_power_limit);
int pd63000_get_port_power_limit(int port_index);
void pd63000_get_all_port_class(char *classes);
void pd63000_poe_init(void);
int pd63000_wr(uchar *data, char size);
vtss_rc pd63000_rd(uchar *data, char size);
void pd63000_get_all_ports_status (poe_port_status_t *port_status);
void pd63000_port_status_get(vtss_port_no_t port_index, poe_port_status_t *port_status);
poe_power_source_t pd63000_get_power_source (void);

// Function that returns 1 is a PoE chip set is found, else 0.
int pd63000_is_chip_available(void);

// Function for getting the legacy capacitor detection mode
BOOL pd63000_capacitor_detection_get(void);

// Function for setting the legacy capacitor detection mode
//      enable         : True - Enable legacy capacitor detection
void pd63000_capacitor_detection_set(BOOL enable);

// Get software version - Section 4.7.1 in user guide
// Out - Pointer to TXT string that shall contain the firmware information
void pd63000_get_sw_version(char *info);

// Function for updating PoE chipset firmware
void DownloadFirmwareFunc (u8 *microsemi_firmware, u32 firmware_size);


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
