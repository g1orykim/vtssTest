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
#ifdef VTSS_SW_OPTION_FAN

#include "icli_api.h"
#include "icli_porting_util.h"

#include "fan.h"
#include "fan_api.h"

#include "msg_api.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif


/***************************************************************************/
/*  Code start :)                                                           */
/****************************************************************************/
// See fan_icli_functions.h
void fan_status(i32 session_id)
{
  fan_local_status_t   status;
  i8 header_txt[255];
  i8 str_buf[255];
  switch_iter_t   sit;
  vtss_rc rc;
  u8 sensor_id;
  u8 sensor_cnt;


  // Loop through all switches in stack
  (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
  while (switch_iter_getnext(&sit)) {
    if ((rc = fan_mgmt_get_switch_status(&status, sit.isid))) {
      T_E("%s", error_txt(rc));
    } else {
      strcpy(header_txt, ""); //Clear string

      sensor_cnt = FAN_TEMPERATURE_SENSOR_CNT(sit.isid);
      for (sensor_id = 0; sensor_id < sensor_cnt; sensor_id++) {
        strcat(header_txt, "Chip Temp.  ");
      }
      strcat(header_txt, "Fan Speed\n");

      ICLI_PRINTF(header_txt);

      for (sensor_id = 0; sensor_id < sensor_cnt; sensor_id++) {
        sprintf(str_buf, "%d %s", status.chip_temp[sensor_id], "C");
        ICLI_PRINTF("%-12s", str_buf);
      }
      sprintf(str_buf, "%d %s", status.fan_speed, "RPM");
      ICLI_PRINTF("%-10s", str_buf);
    }
    ICLI_PRINTF("\n");
  }
}

// See fan_icli_functions.h
void fan_temp(u32 session_id, BOOL t_on, i8 new_temp, BOOL no)
{
  fan_local_conf_t     fan_conf;


  T_I("t_on:%d, New temp:%d", t_on, new_temp);

  // Get configuration for the current switch
  fan_mgmt_get_switch_conf(&fan_conf);

  // update with new configuration
  if (t_on) {
    if (no) {
      fan_conf.glbl_conf.t_on = FAN_CONF_T_ON_DEFAULT;
    } else {
      if (fan_conf.glbl_conf.t_max > new_temp) {
        fan_conf.glbl_conf.t_on = new_temp;
      } else {
        ICLI_PRINTF("temp-on MUST be lower than temp-max\n");
      }
    }

    // Not t_on then it is t_max
  } else {
    if (no) {
      fan_conf.glbl_conf.t_max = FAN_CONF_T_MAX_DEFAULT;
    } else {
      if (fan_conf.glbl_conf.t_on < new_temp) {
        fan_conf.glbl_conf.t_max  = new_temp;
      } else {
        ICLI_PRINTF("temp-max MUST be higher than temp-on\n");
      }
    }
  }

  // Write back new configuration
  if (fan_mgmt_set_switch_conf(&fan_conf) != VTSS_RC_OK) {
    T_E("Could not set new configuration");
  }
}




#ifdef VTSS_SW_OPTION_ICFG

/* ICFG callback functions */
static vtss_rc fan_global_conf(const vtss_icfg_query_request_t *req,
                               vtss_icfg_query_result_t *result)
{
  fan_local_conf_t     fan_conf;
  // Get configuration for the current switch
  fan_mgmt_get_switch_conf(&fan_conf);

  vtss_icfg_conf_print_t conf_print;
  vtss_icfg_conf_print_init(&conf_print);

  conf_print.is_default = fan_conf.glbl_conf.t_on == FAN_CONF_T_ON_DEFAULT;
  VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "green-ethernet fan temp-on", "%u", fan_conf.glbl_conf.t_on));

  conf_print.is_default = fan_conf.glbl_conf.t_max == FAN_CONF_T_MAX_DEFAULT;
  VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "green-ethernet fan temp-max", "%u", fan_conf.glbl_conf.t_max));

  return VTSS_RC_OK;
}


/* ICFG Initialization function */
vtss_rc fan_icfg_init(void)
{
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_FAN_GLOBAL_CONF, "green-ethernet", fan_global_conf));
  return VTSS_RC_OK;
}
#endif // VTSS_SW_OPTION_ICFG
#endif // #ifdef VTSS_SW_OPTION_FAN
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
