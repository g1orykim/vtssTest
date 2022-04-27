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

*/

#ifndef _THERMAL_PROTECT_API_H_
#define _THERMAL_PROTECT_API_H_


//************************************************
// Definition of rc errors - See also thermal_protect_error_txt in thermal_protect.c
//************************************************
enum {
    THERMAL_PROTECT_ERROR_ISID = MODULE_ERROR_START(VTSS_MODULE_ID_THERMAL_PROTECT),
    THERMAL_PROTECT_ERROR_FLASH,
    THERMAL_PROTECT_ERROR_SLAVE,
    THERMAL_PROTECT_ERROR_NOT_MASTER,
    THERMAL_PROTECT_ERROR_VALUE
};
char *thermal_protect_error_txt(vtss_rc rc);
char *thermal_protect_power_down2txt(BOOL powered_down);

//************************************************
// Constants
//************************************************

// Defines chip temperature range (Selected to be within a u8 type)
#define THERMAL_PROTECT_TEMP_MAX 255
#define THERMAL_PROTECT_TEMP_MIN 0


// Defines the port priorities
#define THERMAL_PROTECT_PRIOS_MIN 0
#define THERMAL_PROTECT_PRIOS_MAX 3
#define THERMAL_PROTECT_PRIOS_CNT 4

//************************************************
// Configuration definition
//************************************************

// Global configuration (configuration that are shared for all switches in the stack)
typedef struct {
    u8   prio_temperatures[THERMAL_PROTECT_PRIOS_CNT]; // Array with the shut down temperature for each thermal protect priority.
} thermal_protect_glbl_conf_t;

// Switch local configuration (configurations that are local for a switch in the stack)
typedef struct {
    u8 port_prio[VTSS_PORTS];              // The thermal protect priority for each port.
} thermal_protect_local_conf_t;

// Switch configuration (Both local information, and global configuration which is common for all switches in the stack.
typedef struct {
    thermal_protect_glbl_conf_t glbl_conf;     // The configuration that is common for all switches in the stack.
    thermal_protect_local_conf_t local_conf;   // The thermal protect priority for each port.
} thermal_protect_switch_conf_t;

// Switch status (local status for a switch in the stack)
typedef struct {
    u8 port_temp[VTSS_PORTS];           // The temperature associated to a given port
    BOOL port_powered_down[VTSS_PORTS]; // True if the port is powered down due to thermal protection.
} thermal_protect_local_status_t;

// See thermal_protect.c
void thermal_protect_switch_conf_default_get(thermal_protect_switch_conf_t *switch_conf);

//************************************************
// Functions
//************************************************
void    thermal_protect_mgmt_switch_conf_get(thermal_protect_switch_conf_t *local_conf, vtss_isid_t isid);
vtss_rc thermal_protect_mgmt_switch_conf_set(thermal_protect_switch_conf_t *new_local_conf, vtss_isid_t isid);
vtss_rc thermal_protect_mgmt_get_switch_status(thermal_protect_local_status_t *status, vtss_isid_t isid);
vtss_rc thermal_protect_init(vtss_init_data_t *data);
#endif // _THERMAL_PROTECT_API_H_
//***************************************************************************
//  End of file.
//***************************************************************************
