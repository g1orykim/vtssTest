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

#ifndef _FAN_API_H_
#define _FAN_API_H_

#include "fan_custom_api.h"

//************************************************
// Definition of rc errors - See also fan_error_txt in fan.c
//************************************************
enum {
    FAN_ERROR_ISID = MODULE_ERROR_START(VTSS_MODULE_ID_FAN),
    FAN_ERROR_FLASH,
    FAN_ERROR_SLAVE,
    FAN_ERROR_NOT_MASTER,
    FAN_ERROR_VALUE,
    FAN_ERROR_T_CONF,
    FAN_ERROR_FAN_NOT_RUNNING,
};
char *fan_error_txt(vtss_rc rc);



//************************************************
// Configuration definition
//************************************************

// Global configuration (configuration that are shared for all switches in the stack)
typedef struct {
    i8   t_max; // The temperature were fan is running a full speed
    i8   t_on;  // The temperature were cooling is needed (fan is started)
} fan_glbl_conf_t;


// Switch configuration (configurations that are local for a switch in the stack)
typedef struct {
    fan_glbl_conf_t glbl_conf;
} fan_local_conf_t;

// Switch status (local status for a switch in the stack)
typedef struct {
    i16 chip_temp[FAN_TEMPERATURE_SENSOR_CNT_MAX]; // Chip temperature ( in C.)
    u16 fan_speed;             // The speed the fan is currently running (in RPM)
    u8  fan_speed_setting_pct; // The fan speed level at which it is set to (in %).
} fan_local_status_t;



//************************************************
// Constants
//************************************************

// Defines chip temperature range (Select to be within a u8 type)
#define FAN_TEMP_MAX 127
#define FAN_TEMP_MIN -127

//************************************************
// Functions
//************************************************
void    fan_mgmt_get_switch_conf(fan_local_conf_t *local_conf);
vtss_rc fan_mgmt_set_switch_conf(fan_local_conf_t *new_local_conf);
vtss_rc fan_mgmt_get_switch_status(fan_local_status_t *status, vtss_isid_t isid);
vtss_rc fan_init(vtss_init_data_t *data);
i16     fan_find_highest_temp(i16 *temperature_array);
#endif // _FAN_API_H_


//***************************************************************************
//  End of file.
//***************************************************************************
