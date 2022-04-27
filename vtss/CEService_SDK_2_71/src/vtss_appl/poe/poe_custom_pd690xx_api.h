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
*/
// Set how many ports the PoE chip can control
#define PD690xx_PORTS_PER_POE_CHIP 12


u16 pd690xx_is_chip_available(vtss_port_no_t iport);
void pd690xx_poe_enable(vtss_port_no_t port_index, BOOL enable, poe_mode_t mode);
void pd690xx_poe_init(void);
void pd690xx_get_port_measurement(poe_status_t *poe_status, vtss_port_no_t iport);
void pd690xx_port_status_get(poe_port_status_t *port_status, vtss_port_no_t iport);
void pd690xx_get_all_port_class(i8 *classes, vtss_port_no_t iport);
void pd690xx_set_power_limit_channel(vtss_port_no_t port_index, u16 max_port_power);
void pd690xx_device_rd(vtss_port_no_t iport, u16 reg_addr, u16 *data);
void pd690xx_device_wr(vtss_port_no_t iport, u16 reg_addr, u16 data);
void pd690xx_capacitor_detection_set(vtss_port_no_t iport, BOOL enable);
BOOL pd690xx_capacitor_detection_get(vtss_port_no_t iport);

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
