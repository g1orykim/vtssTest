/*

 Vitesse Switch API software.

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

*/
#ifdef VTSS_SW_OPTION_THERMAL_PROTECT

#include "icli_api.h"
#include "icli_porting_util.h"

#include "cli.h" // For cli_port_iter_init

#include "thermal_protect.h"
#include "thermal_protect_api.h"

#include "msg_api.h"
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif

#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()

/***************************************************************************/
/*  Code start :)                                                           */
/****************************************************************************/
// See thermal_protect_icli_functions.h
void thermal_protect_status(i32 session_id, BOOL has_interface, icli_stack_port_range_t *port_list)
{
  thermal_protect_local_status_t   switch_status;
  vtss_rc rc;
  switch_iter_t sit;
  port_iter_t pit;

  // Loop through all switches in stack
  // Loop all the switches in question.
  if (icli_switch_iter_init(&sit) != VTSS_RC_OK) {
    return;
  }

  while (icli_switch_iter_getnext(&sit, port_list)) {
    if (!msg_switch_exists(sit.isid)) {
      continue;
    }

    // Print out of status data
    ICLI_PRINTF("Interface     Chip Temp.  Port Status\n");
    if ((rc = thermal_protect_mgmt_get_switch_status(&switch_status, sit.isid))) {
      ICLI_PRINTF("%s", error_txt(rc));
      continue;
    }

    // Loop through all ports
    if (icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_STACK) == VTSS_RC_OK) {
      while (icli_port_iter_getnext(&pit, port_list)) {
        char port[20];
        (void) icli_port_info_txt_short(sit.usid, pit.uport, port);
        ICLI_PRINTF("%-12s %3d C        %s\n", port, switch_status.port_temp[pit.iport],
                    thermal_protect_power_down2txt(switch_status.port_powered_down[pit.iport]));
      }
    }
  }
}

// See thermal_protect_icli_functions.h
void thermal_protect_prio(icli_stack_port_range_t *port_list, u8 prio, BOOL no)
{
  thermal_protect_switch_conf_t     switch_conf;
  vtss_rc rc;
  switch_iter_t sit;
  port_iter_t pit;

  thermal_protect_switch_conf_t switch_conf_default;
  thermal_protect_switch_conf_default_get(&switch_conf_default); // Get default configuration

  // Loop through all switches in stack
  (void) icli_switch_iter_init(&sit);
  while (icli_switch_iter_getnext(&sit, port_list)) {

    // Get configuration for the current switch
    thermal_protect_mgmt_switch_conf_get(&switch_conf, sit.isid);

    // Loop through all ports
    (void)icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT | PORT_ITER_FLAGS_STACK);
    while (icli_port_iter_getnext(&pit, port_list)) { // !interface is used to get all ports in case no interface is specified.

      // Set to default if this is the no command
      if (no) {
        switch_conf.local_conf.port_prio[pit.iport] = switch_conf_default.local_conf.port_prio[pit.iport];
      } else {
        switch_conf.local_conf.port_prio[pit.iport] = prio;
      }
    }

    // Set the new configuration
    if ((rc = thermal_protect_mgmt_switch_conf_set(&switch_conf, sit.isid)) != VTSS_OK) {
      T_E("%s \n", error_txt(rc));
    }
  }
}

// See thermal_protect_icli_functions.h
vtss_rc thermal_protect_temp(i32 session_id, icli_unsigned_range_t *prio_list, i8 new_temp, BOOL no)
{
  u8 element_index;
  u8 prio_index;
  switch_iter_t sit;
  thermal_protect_switch_conf_t     switch_conf;
  thermal_protect_switch_conf_t switch_conf_default;

  thermal_protect_switch_conf_default_get(&switch_conf_default); // Get default configuration

  // prio_list can be NULL if the uses simply want to set all priorities
  if (prio_list != NULL) {

    // Loop through all switches in stack
    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID_CFG);
    while (switch_iter_getnext(&sit)) {

      // Get configuration for the current switch
      thermal_protect_mgmt_switch_conf_get(&switch_conf, sit.isid);

      // Loop through the elements in the list
      for (element_index = 0; element_index < prio_list->cnt; element_index++) {
        for (prio_index = prio_list->range[element_index].min; prio_index <= prio_list->range[element_index].max; prio_index++) {
          // Make sure we don't get out of bounds
          if (prio_index > THERMAL_PROTECT_PRIOS_MAX) {
            ICLI_PRINTF("%% Ignoring invalid priority:%d. Valid range is %u-%u.\n",
                        prio_index, THERMAL_PROTECT_PRIOS_MIN, THERMAL_PROTECT_PRIOS_MAX);
            continue;
          }

          // Set to default if this is the no command
          if (no) {
            switch_conf.glbl_conf.prio_temperatures[prio_index] = switch_conf_default.glbl_conf.prio_temperatures[prio_index];
          } else {
            switch_conf.glbl_conf.prio_temperatures[prio_index] = new_temp;
          }

        }
      }

      // Set the new configuration
      VTSS_ICLI_ERR_PRINT(thermal_protect_mgmt_switch_conf_set(&switch_conf, sit.isid));
    }
  }
  return VTSS_RC_OK;
}

#ifdef VTSS_SW_OPTION_ICFG
/* ICFG callback functions */
static vtss_rc thermal_protect_conf(const vtss_icfg_query_request_t *req,
                                    vtss_icfg_query_result_t *result)
{
  u8 prio_index;
  thermal_protect_switch_conf_t switch_conf;
  thermal_protect_switch_conf_t switch_conf_default;
  vtss_port_no_t iport;
  vtss_icfg_conf_print_t conf_print;
  vtss_icfg_conf_print_init(&conf_print);

  thermal_protect_switch_conf_default_get(&switch_conf_default); // Get default configuration

  T_N("req->cmd_mode:%d", req->cmd_mode);
  switch (req->cmd_mode) {
  case ICLI_CMD_MODE_GLOBAL_CONFIG:
    // Get configuration for the current switch
    thermal_protect_mgmt_switch_conf_get(&switch_conf, VTSS_ISID_LOCAL);

    // Priorities temperatures configuration
    i8 cmd_buf[100];
    for (prio_index = THERMAL_PROTECT_PRIOS_MIN; prio_index <= THERMAL_PROTECT_PRIOS_MAX; prio_index++) {
      conf_print.is_default = switch_conf.glbl_conf.prio_temperatures[prio_index] == switch_conf_default.glbl_conf.prio_temperatures[prio_index];
      sprintf(cmd_buf, "thermal-protect prio %d", prio_index);
      VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, cmd_buf, "temperature %d", switch_conf.glbl_conf.prio_temperatures[prio_index]));
    }
    break;

  case ICLI_CMD_MODE_INTERFACE_PORT_LIST: {
    vtss_isid_t isid = topo_usid2isid(req->instance_id.port.usid);
    iport = uport2iport(req->instance_id.port.begin_uport);

    // Get configuration for the current switch
    thermal_protect_mgmt_switch_conf_get(&switch_conf, isid);

    conf_print.is_default = switch_conf.local_conf.port_prio[iport] == switch_conf_default.local_conf.port_prio[iport];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "thermal-protect port-prio", "%d", switch_conf.local_conf.port_prio[iport]));
    break;
  }
  default:
    break;
  }
  return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc thermal_protect_icfg_init(void)
{
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_THERMAL_PROTECT_GLOBAL_CONF, "thermal-protect", thermal_protect_conf));
  return vtss_icfg_query_register(VTSS_ICFG_THERMAL_PROTECT_PORT_CONF, "thermal-protect", thermal_protect_conf);
}
#endif // VTSS_SW_OPTION_ICFG
#endif // #ifdef VTSS_SW_OPTION_THERMAL_PROTECT
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
