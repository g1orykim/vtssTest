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
#ifdef VTSS_SW_OPTION_LED_POW_REDUC

#include "icli_api.h"
#include "icli_porting_util.h"

#include "led_pow_reduc.h"
#include "led_pow_reduc_api.h"

#include "msg_api.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif

/***************************************************************************/
/*  Code start :)                                                           */
/****************************************************************************/
// See led_pow_reduc_icli_functions.h
void led_pow_reduc_icli_led_interval(u32 session_id, icli_unsigned_range_t *led_interval_list, u8 intensity,  BOOL no)
{
  // Loop through the elements in the list
  u8 element_index;
  u32 min, max;
  u32 cnt;
  if (led_interval_list == NULL) {
    cnt = 1;
  } else {
    cnt = led_interval_list->cnt;
  }
  for (element_index = 0; element_index < cnt; element_index++) {

    if (led_interval_list == NULL) {
      min = LED_POW_REDUC_TIMERS_MIN;
      max = LED_POW_REDUC_TIMERS_MIN;
    } else {
      min = led_interval_list->range[element_index].min;
      max = led_interval_list->range[element_index].max;
    }

    if (no) {
      intensity = LED_POW_REDUC_INTENSITY_DEFAULT;
    }

    if (led_pow_reduc_mgmt_timer_set(min, max, intensity) != VTSS_RC_OK) {
      T_E("Could not set LED timers");
    }
  }
}

// See led_pow_reduc_icli_functions.h
void led_pow_reduc_icli_on_event(u32 session_id, BOOL has_link_change, u16 v_0_to_65535, BOOL has_error, BOOL no)
{
  led_pow_reduc_local_conf_t     led_local_conf;

  // Get current configuration
  led_pow_reduc_mgmt_get_switch_conf(&led_local_conf); // Get current configuration

  if (has_link_change) {
    if (no) {
      led_local_conf.glbl_conf.maintenance_time = LED_POW_REDUC_MAINTENANCE_TIME_DEFAULT;
    } else {
      led_local_conf.glbl_conf.maintenance_time = v_0_to_65535;
    }
  }

  T_I("has_error:%d, has_link_change:%d, v_0_to_65535:%d, no:%d", has_error, has_link_change, v_0_to_65535, no);
  if (has_error) {
    if (no) {
      led_local_conf.glbl_conf.on_at_err = LED_POW_REDUC_ON_AT_ERR_DEFAULT;
    } else {
      led_local_conf.glbl_conf.on_at_err = TRUE;
    }
  }

  // Set new configuration
  vtss_rc rc;
  if ((rc = led_pow_reduc_mgmt_set_switch_conf(&led_local_conf)) != VTSS_OK) {
    T_E(error_txt(rc));
  }
}

#ifdef VTSS_SW_OPTION_ICFG
static vtss_rc led_pow_reduc_icfg_print_led_intensity(u8 start_index, u8 end_index, u8 intensity, vtss_icfg_query_result_t *result)
{
  // When an interval ends at midnight we use 24:00 and not 00:00
  if (end_index == 0) {
    end_index = 24;
  }

  if (intensity == LED_POW_REDUC_INTENSITY_DEFAULT) {
    VTSS_RC(vtss_icfg_printf(result, "no green-ethernet led interval %d-%d\n",
                             start_index, end_index));
  } else {
    VTSS_RC(vtss_icfg_printf(result, "green-ethernet led interval %d-%d intensity %d\n",
                             start_index, end_index, intensity));
  }
  return VTSS_RC_OK;
}

/* ICFG callback functions */
static vtss_rc led_pow_reduc_global_conf(const vtss_icfg_query_request_t *req,
                                         vtss_icfg_query_result_t *result)
{
  led_pow_reduc_local_conf_t     led_local_conf;

  // Get current configuration
  led_pow_reduc_mgmt_get_switch_conf(&led_local_conf); // Get current configuration

  //
  // Maintenance_Time
  //
  if (req->all_defaults ||
      (led_local_conf.glbl_conf.maintenance_time != LED_POW_REDUC_MAINTENANCE_TIME_DEFAULT)) {

    if (led_local_conf.glbl_conf.maintenance_time == LED_POW_REDUC_MAINTENANCE_TIME_DEFAULT) {
      VTSS_RC(vtss_icfg_printf(result, "no green-ethernet led on-event link-change\n"));
    } else {
      VTSS_RC(vtss_icfg_printf(result, "green-ethernet led on-event link-change %d\n", led_local_conf.glbl_conf.maintenance_time));
    }
  }

  //
  // on at error
  //
  if (req->all_defaults ||
      (led_local_conf.glbl_conf.on_at_err != LED_POW_REDUC_ON_AT_ERR_DEFAULT)) {

    if (led_local_conf.glbl_conf.on_at_err == LED_POW_REDUC_ON_AT_ERR_DEFAULT) {
      VTSS_RC(vtss_icfg_printf(result, "no green-ethernet led on-event error\n"));
    } else {
      VTSS_RC(vtss_icfg_printf(result, "green-ethernet led on-event error\n"));
    }
  }

  //
  // Intensity
  //
  led_pow_reduc_timer_t led_timer;
  led_pow_reduc_mgmt_timer_get_init(&led_timer); // Prepare for looping through all timers

  // Loop through all timers and print them
  while (led_pow_reduc_mgmt_timer_get(&led_timer)) {
    if (req->all_defaults ||
        (led_local_conf.glbl_conf.led_timer_intensity[led_timer.start_index] != LED_POW_REDUC_INTENSITY_DEFAULT)) {
      VTSS_RC(led_pow_reduc_icfg_print_led_intensity(led_timer.start_index, led_timer.end_index, led_local_conf.glbl_conf.led_timer_intensity[led_timer.start_index], result));
    }
  }

  return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc led_pow_reduc_icfg_init(void)
{
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_LED_POW_REDUC_GLOBAL_CONF, "green-ethernet", led_pow_reduc_global_conf));
  return VTSS_RC_OK;
}
#endif // VTSS_SW_OPTION_ICFG
#endif // #ifdef VTSS_SW_OPTION_LED_POW_REDUC
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
