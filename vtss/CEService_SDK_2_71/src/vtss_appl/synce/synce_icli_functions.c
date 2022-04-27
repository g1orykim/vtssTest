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
#include "icli_api.h"
#include "icli_porting_util.h"
#include "misc_api.h" // For e.g. vtss_uport_no_t
#include "msg_api.h"
#include "mgmt_api.h" // For mgmt_prio2txt
#ifdef VTSS_SW_OPTION_ICFG
#include "icfg_api.h"
#endif
#include "synce.h" // For management functions
#include "synce_trace.h"// For Trace

/***************************************************************************/
/*  Internal types                                                         */
/****************************************************************************/
// Enum for selecting which synce configuration to perform.
typedef enum {
  ANEG,              // Configuration of aneg
  HOLDOFF,           // Configuration of holdoff
  SELECTION_MODE,    // Configuration of selection mode
  EEC_OPTION,        // Configuration of EEC OPTION
  WTR_CONF,               // Configuration of WTR
  PRIO,              // Configuration of pririty
  SSM_HOLDOVER,      // Configuration of ssm hodover
  SSM_FREERUN,       // Configuration of ssm freerun
  SSM_OVERWRITE,     // Configuration of ssm overwrite
  NOMINATE           // Configuration of nominate
} synce_conf_type_t;

// Used to passed the synce values to configure
typedef union {
  synce_mgmt_aneg_mode_t aneg_mode; // Preferred auto negotiation mode
  u8                     holdoff;   // Holdoff value
  struct {
    u8                             clk_src;         // Source clock when mode is manual
    synce_mgmt_selection_mode_t    mode;            // selection mode
  } selection;
  uint                           wtr_time;          // WTR timer value in minutes
  synce_mgmt_quality_level_t     ssm_holdover;      // tx overwrite SSM used when clock controller is hold over
  synce_mgmt_quality_level_t     ssm_freerun;       // tx overwrite SSM used when clock controller is free run
  synce_mgmt_quality_level_t     ssm_overwrite;
  synce_eec_option_t             eec_option;        // Synchronous Ethernet Equipment Clock option
  u8                             prio;              // Priority
  u8                             wtr;               // Wait-To-Restore time
  u8                             nominated_src;     // port number of the norminated sources
} synce_conf_value_t;

/***************************************************************************/
/*  Internal functions                                                     */
/****************************************************************************/
// Function to configure selection
// IN : conf  - Which configuration parameter to configure
//      value - New configuration value
//      no    - TRUE if value shall be set to default value
// Return - error code
static vtss_rc synce_icli_selection_set(synce_conf_type_t conf, synce_conf_value_t value, BOOL no)
{
  // get current configuration
  synce_mgmt_clock_conf_blk_t  clock_conf;
  VTSS_RC(synce_mgmt_clock_config_get(&clock_conf));

  // Get default
  synce_mgmt_clock_conf_blk_t  clock_conf_default;
  synce_mgmt_port_conf_blk_t   port_conf_default_dummy;
  synce_set_conf_to_default(&clock_conf_default, &port_conf_default_dummy);

  switch (conf) {
  case SELECTION_MODE:
    if (no) {
      clock_conf.selection_mode = clock_conf_default.selection_mode;
    } else {
      clock_conf.selection_mode = value.selection.mode;
    }

    // If the selection mode is manual then the clock must to be set as well
    if (clock_conf.selection_mode == SYNCE_MGMT_SELECTOR_MODE_MANUEL) {
      if (no) {
        clock_conf.source = clock_conf_default.source;
      } else {
        clock_conf.source = value.selection.clk_src;
      }
    }
    break;

  case WTR_CONF:
    if (no) {
      clock_conf.wtr_time = clock_conf_default.wtr_time;
    } else {
      clock_conf.wtr_time = value.wtr_time;
    }
    break;

  case SSM_HOLDOVER:
    if (no) {
      clock_conf.ssm_holdover = clock_conf_default.ssm_holdover;
    } else {
      clock_conf.ssm_holdover = value.ssm_holdover;
    }
    break;

  case SSM_FREERUN:
    if (no) {
      clock_conf.ssm_freerun =     clock_conf_default.ssm_freerun;
    } else {
      clock_conf.ssm_freerun = value.ssm_freerun;
    }
    break;

  case EEC_OPTION:
    if (no) {
      clock_conf.eec_option = clock_conf_default.eec_option;
    } else {
      clock_conf.eec_option = value.eec_option;
    }
    break;

  default:
    T_E("Unknown configuration");
  }

  T_NG(TRACE_GRP_CLI, "clock_conf.source:%d", clock_conf.source);

  // Update new settings
  VTSS_RC(synce_mgmt_nominated_selection_mode_set(clock_conf.selection_mode,
                                                  clock_conf.source,
                                                  clock_conf.wtr_time,
                                                  clock_conf.ssm_holdover,
                                                  clock_conf.ssm_freerun,
                                                  clock_conf.eec_option));
  return VTSS_RC_OK;
}

// Function to configure station clock
// IN : input_source - TRUE to configure input clock, FALSE to configure output clock
//      freq - New configuration frequency
// Return - error code
static vtss_rc synce_icli_station_clock_set(BOOL input_source, synce_mgmt_frequency_t clk_freq)
{
  if (input_source) {
    VTSS_RC(synce_mgmt_station_clock_in_set(clk_freq));
  } else {
    VTSS_RC(synce_mgmt_station_clock_out_set(clk_freq));
  }
  return VTSS_RC_OK;
}

// Function to configure nominate
// IN : clk_src_list - list of clock sources to configure
//      conf  - Which configuration parameter to configure
//      value - New configuration value
//      no    - TRUE if value shall be set to default value
// Return - error code
static vtss_rc synce_icli_nominate_set(icli_range_t *clk_src_list, synce_conf_type_t conf, synce_conf_value_t value, BOOL no)
{
  u8 clk_src;
  u8 list_cnt;
  u8 cnt_index;

  // get current configuration
  synce_mgmt_clock_conf_blk_t  clock_conf;
  VTSS_RC(synce_mgmt_clock_config_get(&clock_conf));

  // Get default
  synce_mgmt_clock_conf_blk_t  clock_conf_default;
  synce_mgmt_port_conf_blk_t   port_conf_default_dummy;
  synce_set_conf_to_default(&clock_conf_default, &port_conf_default_dummy);


  T_RG(TRACE_GRP_CLI, "no:%d", no);

  // Determine which soirce clock to configure
  list_cnt = 1; // Default the list to only have one source list                   ;

  if (clk_src_list != NULL) { // User has given a source clock list
    list_cnt = clk_src_list->u.sr.cnt;
  }

  // Run through all the lists
  for (cnt_index = 0; cnt_index < list_cnt; cnt_index++) {
    u8 src_clk_min = 1;                       // Default to start from 1
    u8 src_clk_max = synce_my_nominated_max;  // Default to end at the last clock source

    if (clk_src_list != NULL) { // User has specified a source clock list
      src_clk_min = clk_src_list->u.sr.range[cnt_index].min;
      src_clk_max = clk_src_list->u.sr.range[cnt_index].max;
    }

    T_IG(TRACE_GRP_CLI, "src_clk_min:%d, src_clk_max:%d, list_cnt:%d, clk_src_list is NULL:%d",
         src_clk_min, src_clk_max, list_cnt, clk_src_list == NULL);


    // Clock src from a user point of view is starting from 1, while indexes starts from 0, so subtract 1 in the loop
    for (clk_src = synce_uclk2iclk(src_clk_min); clk_src <= synce_uclk2iclk(src_clk_max); clk_src++) {
      // Update the parameter in question
      switch (conf) {
      case ANEG:
        if (no) {
          clock_conf.aneg_mode[clk_src] = clock_conf_default.aneg_mode[clk_src];
        } else {
          clock_conf.aneg_mode[clk_src] = value.aneg_mode;
        }
        break;

      case NOMINATE:
        if (no) {
          clock_conf.nominated[clk_src] = clock_conf_default.nominated[clk_src];
          clock_conf.port[clk_src] = clock_conf_default.port[clk_src];
        } else {
          clock_conf.nominated[clk_src] = TRUE;
          clock_conf.port[clk_src] = value.nominated_src;
        }
        break;

      case HOLDOFF:
        if (no) {
          clock_conf.holdoff_time[clk_src] = clock_conf_default.holdoff_time[clk_src];
        } else {
          clock_conf.holdoff_time[clk_src] = value.holdoff;
        }
        break;

      case SSM_OVERWRITE:
        if (no) {
          clock_conf.ssm_overwrite[clk_src] = clock_conf_default.ssm_overwrite[clk_src];
        } else {
          clock_conf.ssm_overwrite[clk_src] = value.ssm_overwrite;
        }
        break;

      case PRIO:
        if (no) {
          VTSS_RC(synce_mgmt_nominated_priority_set(clk_src, clock_conf_default.priority[clk_src]));
        } else {
          VTSS_RC(synce_mgmt_nominated_priority_set(clk_src, value.prio));
        }
        break;

      default:
        T_E("Unknown configuration");
      }

      T_DG(TRACE_GRP_CLI, "clock_conf.aneg_mode:%d, clk_src:%d, port:%d, holdoff_time:%d, clock_conf.ssm_overwrite[clk_src]:%d, nominate:%d",
           clock_conf.aneg_mode[clk_src],
           clk_src, clock_conf.port[clk_src],
           clock_conf.holdoff_time[clk_src],
           clock_conf.ssm_overwrite[clk_src],
           clock_conf.nominated[clk_src]);

      // Update new settings
      VTSS_RC(synce_mgmt_nominated_source_set(clk_src, clock_conf.nominated[clk_src], clock_conf.port[clk_src], clock_conf.aneg_mode[clk_src],
                                              clock_conf.holdoff_time[clk_src], clock_conf.ssm_overwrite[clk_src]));
    }
  }
  return VTSS_RC_OK;
}


// Function to configure SSM enable/disable
// IN : plist - list ports to configure
//      no    - TRUE if value shall be set to default value
// Return - error code
static vtss_rc sync_ssm_enable( icli_stack_port_range_t *plist, BOOL no)
{
  switch_iter_t   sit;

  // Loop through all switches in stack
  VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG));
  while (icli_switch_iter_getnext(&sit, plist)) {

    // Loop through all ports
    port_iter_t pit;
    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT));
    while (icli_port_iter_getnext(&pit, plist)) {
      VTSS_RC(synce_mgmt_ssm_set(pit.iport, !no)); // Enable / Disable ssm for the port in question
    }
  }
  return VTSS_RC_OK;
}

// Function to show SyncE state
// IN : session_id - For icli print
// Return - error code
static vtss_rc synce_icli_show_state(i32 session_id)
{
  uint                            source, i;
  BOOL                            master;
  synce_mgmt_selector_state_t     selector_state;
  synce_mgmt_alarm_state_t        alarm_state;
  synce_mgmt_port_conf_blk_t      port_config;
  synce_mgmt_quality_level_t      ssm_rx;
  synce_mgmt_quality_level_t      ssm_tx;
  port_iter_t       pit;

  // Get current states
  VTSS_RC(synce_mgmt_clock_state_get(&selector_state, &source, &alarm_state));

  ICLI_PRINTF("\nSelector State is: ");
  switch (selector_state) {
  case SYNCE_MGMT_SELECTOR_STATE_LOCKED:
    ICLI_PRINTF("Locked to %d\n", source + 1);
    break;
  case SYNCE_MGMT_SELECTOR_STATE_HOLDOVER:
    ICLI_PRINTF("Holdover\n");
    break;
  case SYNCE_MGMT_SELECTOR_STATE_FREERUN:
    ICLI_PRINTF("Free Run\n");
    break;
  }

  ICLI_PRINTF("\nAlarm State is:\n");
  ICLI_PRINTF("Clk:");
  for (i = 0; i < synce_my_nominated_max; ++i) {
    ICLI_PRINTF("       %2d", i + 1);
  }
  ICLI_PRINTF("\n");
  ICLI_PRINTF("LOCS:");
  for (i = 0; i < synce_my_nominated_max; ++i) {
    if (alarm_state.locs[i]) {
      ICLI_PRINTF("    TRUE ");
    } else {
      ICLI_PRINTF("    FALSE");
    }
  }
  ICLI_PRINTF("\nSSM: ");
  for (i = 0; i < synce_my_nominated_max; ++i) {
    if (alarm_state.ssm[i]) {
      ICLI_PRINTF("    TRUE ");
    } else {
      ICLI_PRINTF("    FALSE");
    }
  }
  ICLI_PRINTF("\nWTR: ");
  for (i = 0; i < synce_my_nominated_max; ++i) {
    if (alarm_state.wtr[i]) {
      ICLI_PRINTF("    TRUE ");
    } else {
      ICLI_PRINTF("    FALSE");
    }
  }

  ICLI_PRINTF("\n");
  if (alarm_state.lol) {
    ICLI_PRINTF("\nLOL:     TRUE");
  } else {
    ICLI_PRINTF("\nLOL:     FALSE");
  }
  if (alarm_state.dhold) {
    ICLI_PRINTF("\nDHOLD:   TRUE \n");
  } else {
    ICLI_PRINTF("\nDHOLD:   FALSE\n");
  }

  VTSS_RC(synce_mgmt_port_config_get(&port_config));

  ICLI_PRINTF("\nSSM State is:\n");

  ICLI_PRINTF("%-23s %12s %12s %-8s\n", "Interface", "Tx SSM", "Rx SSM", "Mode");

  port_iter_init_local(&pit);


  switch_iter_t   sit;
  VTSS_RC(switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_ISID_CFG));
  while (icli_switch_iter_getnext(&sit, NULL)) {
    // Loop through all ports
    VTSS_RC(icli_port_iter_init(&pit, sit.isid, PORT_ITER_FLAGS_FRONT));
    while (icli_port_iter_getnext(&pit, NULL)) {
      if (port_config.ssm_enabled[pit.iport]) {
        i8 buf[150];

        VTSS_RC(synce_mgmt_port_state_get(pit.iport + VTSS_PORT_NO_START, &ssm_rx, &ssm_tx, &master));
        ICLI_PRINTF("%-23s %12s %12s %-8s\n",
                    icli_port_info_txt(sit.usid, pit.uport, buf), ssm_string(ssm_tx), ssm_string(ssm_rx), (!master) ? "Slave" : "Master");
      }
    }
  }
  ICLI_PRINTF("\n");

  return VTSS_RC_OK;
}

// Function for at runtime figure out which input/out frequencies is supported
// IN : ask - Asking
//      supported - Set to TRUE is supported (present gets set)
// OUT  runtime - Pointer to where to put the "answer"
// Return - TRUE if answer found
static BOOL synce_icli_runtime_synce_clk_chk(icli_runtime_ask_t ask,
                                             icli_runtime_t     *runtime,
                                             BOOL supported)
{
  switch (ask) {
  case ICLI_ASK_PRESENT:
    runtime->present = supported;
    return TRUE;
  default:
    break;
  }
  return FALSE;
}

// Function for checking if any input frequency is supported
// Return - TRUE if at least one input frequency is supported, else FALSE
static BOOL synce_icli_any_input_freq_supported(void)
{
  return clock_in_range_check(SYNCE_MGMT_STATION_CLK_1544_KHZ) |
         clock_in_range_check(SYNCE_MGMT_STATION_CLK_2048_KHZ) |
         clock_in_range_check(SYNCE_MGMT_STATION_CLK_10_MHZ);
}

// Function for checking if any output frequency is supported
// Return - TRUE if at least one output frequency is supported, else FALSE
static BOOL synce_icli_any_output_freq_supported(void)
{
  return clock_out_range_check(SYNCE_MGMT_STATION_CLK_1544_KHZ) |
         clock_out_range_check(SYNCE_MGMT_STATION_CLK_2048_KHZ) |
         clock_out_range_check(SYNCE_MGMT_STATION_CLK_10_MHZ);
}

/***************************************************************************/
/*  Functions called by iCLI                                                */
/****************************************************************************/
//  see synce_icli_functions.h
BOOL synce_icli_runtime_synce_sources(u32                session_id,
                                      icli_runtime_ask_t ask,
                                      icli_runtime_t     *runtime)
{
  switch (ask) {
  case ICLI_ASK_PRESENT:
    runtime->present = TRUE;
    return TRUE;
  case ICLI_ASK_BYWORD:
    icli_sprintf(runtime->byword, "<clk-source : %u-%u>", 1, synce_my_nominated_max);
    return TRUE;
  case ICLI_ASK_RANGE:
    runtime->range.type = ICLI_RANGE_TYPE_UNSIGNED;
    runtime->range.u.sr.cnt = 1;
    runtime->range.u.sr.range[0].min = 1;
    runtime->range.u.sr.range[0].max = synce_my_nominated_max;
    T_RG(TRACE_GRP_CLI, "synce_my_nominated_max:%d", synce_my_nominated_max);
    return TRUE;
  case ICLI_ASK_HELP:
    icli_sprintf(runtime->help, "Clock source number");
    return TRUE;
  default:
    break;
  }
  return FALSE;
}

//  see synce_icli_functions.h
BOOL synce_icli_runtime_input_10MHz(u32                session_id,
                                    icli_runtime_ask_t ask,
                                    icli_runtime_t     *runtime)
{
  return synce_icli_runtime_synce_clk_chk(ask, runtime, clock_in_range_check(SYNCE_MGMT_STATION_CLK_10_MHZ));
}

//  see synce_icli_functions.h
BOOL synce_icli_runtime_input_2048khz(u32               session_id,
                                      icli_runtime_ask_t ask,
                                      icli_runtime_t    *runtime)
{
  return synce_icli_runtime_synce_clk_chk(ask, runtime, clock_in_range_check(SYNCE_MGMT_STATION_CLK_2048_KHZ));
}

//  see synce_icli_functions.h
BOOL synce_icli_runtime_input_1544khz(u32                session_id,
                                      icli_runtime_ask_t ask,
                                      icli_runtime_t     *runtime)
{
  return synce_icli_runtime_synce_clk_chk(ask, runtime, clock_in_range_check(SYNCE_MGMT_STATION_CLK_1544_KHZ));
}

//  see synce_icli_functions.h
BOOL synce_icli_runtime_any_input_freq(u32                session_id,
                                       icli_runtime_ask_t ask,
                                       icli_runtime_t     *runtime)
{
  return synce_icli_runtime_synce_clk_chk(ask, runtime, synce_icli_any_input_freq_supported());
}


//  see synce_icli_functions.h
BOOL synce_icli_runtime_output_10MHz(u32                session_id,
                                     icli_runtime_ask_t ask,
                                     icli_runtime_t     *runtime)
{
  return synce_icli_runtime_synce_clk_chk(ask, runtime, clock_out_range_check(SYNCE_MGMT_STATION_CLK_10_MHZ));
}

//  see synce_icli_functions.h
BOOL synce_icli_runtime_output_2048khz(u32               session_id,
                                       icli_runtime_ask_t ask,
                                       icli_runtime_t    *runtime)
{
  return synce_icli_runtime_synce_clk_chk(ask, runtime, clock_out_range_check(SYNCE_MGMT_STATION_CLK_2048_KHZ));
}

//  see synce_icli_functions.h
BOOL synce_icli_runtime_output_1544khz(u32                session_id,
                                       icli_runtime_ask_t ask,
                                       icli_runtime_t     *runtime)
{
  return synce_icli_runtime_synce_clk_chk(ask, runtime, clock_out_range_check(SYNCE_MGMT_STATION_CLK_1544_KHZ));
}

//  see synce_icli_functions.h
BOOL synce_icli_runtime_any_output_freq(u32                session_id,
                                        icli_runtime_ask_t ask,
                                        icli_runtime_t     *runtime)
{
  return synce_icli_runtime_synce_clk_chk(ask, runtime, synce_icli_any_output_freq_supported());
}

//  see synce_icli_functions.h
vtss_rc synce_icli_station_clk(i32 session_id, BOOL input_source, BOOL has_1544, BOOL has_2048, BOOL has_10M, BOOL no)
{
  synce_mgmt_frequency_t clk_freq;

  if (has_1544) {
    clk_freq = SYNCE_MGMT_STATION_CLK_1544_KHZ;
  } else  if (has_2048) {
    clk_freq = SYNCE_MGMT_STATION_CLK_2048_KHZ;
  } else if (has_10M) {
    clk_freq = SYNCE_MGMT_STATION_CLK_10_MHZ;
  } else {
    clk_freq = SYNCE_MGMT_STATION_CLK_DIS;
  }

  VTSS_ICLI_ERR_PRINT(synce_icli_station_clock_set(input_source, clk_freq));
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc synce_icli_aneg(i32 session_id, icli_range_t *clk_list, BOOL has_master, BOOL has_slave, BOOL has_forced, BOOL no)
{
  synce_conf_type_t conf = ANEG;
  synce_conf_value_t value;

  if (has_master) {
    value.aneg_mode = SYNCE_MGMT_ANEG_PREFERED_MASTER;
  } else if (has_slave) {
    value.aneg_mode = SYNCE_MGMT_ANEG_PREFERED_SLAVE;
  } else if (has_forced) {
    value.aneg_mode = SYNCE_MGMT_ANEG_FORCED_SLAVE;
  } else if (no) {
    value.aneg_mode = SYNCE_MGMT_ANEG_FORCED_SLAVE; // Dummy assign, not real used.
  } else {
    T_E("At least one parameter must be selected");
    return VTSS_RC_ERROR;
  }
  T_IG(TRACE_GRP_CLI, "ang:%d, has_forced:%d, has_master:%d, has_slave:%d, no:%d", value.aneg_mode, has_forced, has_master, has_slave, no);
  VTSS_ICLI_ERR_PRINT(synce_icli_nominate_set(clk_list, conf, value, no));
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc synce_icli_hold_time(i32 session_id, icli_range_t *clk_list, u8 v_3_to_18, BOOL no)
{
  synce_conf_type_t conf = HOLDOFF;
  synce_conf_value_t value;
  value.holdoff = v_3_to_18;
  VTSS_ICLI_ERR_PRINT(synce_icli_nominate_set(clk_list, conf, value, no));
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc synce_icli_show(i32 session_id)
{
  VTSS_ICLI_ERR_PRINT(synce_icli_show_state(session_id));
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc synce_icli_clear(i32 session_id, icli_range_t *clk_src_list)
{
  u8 clk_src;
  u8 list_cnt;
  u8 cnt_index;

  list_cnt = 1;                    ;

  if (clk_src_list != NULL) {
    list_cnt = clk_src_list->u.sr.cnt;
  }

  for (cnt_index = 0; cnt_index < list_cnt; cnt_index++) {
    u8 src_clk_min = 1;
    u8 src_clk_max = synce_my_nominated_max;
    if (clk_src_list != NULL) {
      src_clk_min = clk_src_list->u.sr.range[cnt_index].min;
      src_clk_max = clk_src_list->u.sr.range[cnt_index].max;
    }

    for (clk_src = src_clk_min; clk_src <= src_clk_max; clk_src++) {
      T_DG(TRACE_GRP_CLI, "clk_src:%d, src_clk_min:%d, src_clk_max:%d, list_cnt:%d", clk_src, src_clk_min, src_clk_max, list_cnt);
      VTSS_ICLI_ERR_PRINT(synce_mgmt_wtr_clear_set(synce_uclk2iclk(clk_src)));
    }
  }
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc synce_icli_nominate(i32 session_id, icli_range_t *clk_list, BOOL clk_in, icli_switch_port_range_t *port, BOOL no)
{
  synce_conf_type_t conf = NOMINATE;
  synce_conf_value_t value;

  if (clk_in) {
    value.nominated_src = SYNCE_STATION_CLOCK_PORT;
  } else if (port != NULL) { // port is not set for the no command
    value.nominated_src = port->begin_iport;
  } else {
    value.nominated_src = 1;
  }
  T_IG(TRACE_GRP_CLI, "clk_in %d, src_clk_port:%d", clk_in, value.nominated_src);

  VTSS_ICLI_ERR_PRINT(synce_icli_nominate_set(clk_list, conf, value, no));
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc synce_icli_selector(i32 session_id, u8 clk_src, BOOL has_manual, BOOL has_selected, BOOL has_nonrevertive, BOOL has_revertive, BOOL has_holdover, BOOL has_freerun, BOOL no)
{

  synce_conf_type_t conf = SELECTION_MODE;
  synce_conf_value_t value;

  if (has_manual) {
    value.selection.mode = SYNCE_MGMT_SELECTOR_MODE_MANUEL;
  } else if (has_selected) {
    value.selection.mode = SYNCE_MGMT_SELECTOR_MODE_MANUEL_TO_SELECTED;
  } else if (has_nonrevertive) {
    value.selection.mode = SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_NONREVERTIVE;
  } else if (has_revertive) {
    value.selection.mode = SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_REVERTIVE;
  } else if (has_holdover) {
    value.selection.mode = SYNCE_MGMT_SELECTOR_MODE_FORCED_HOLDOVER;
  } else if (has_freerun) {
    value.selection.mode = SYNCE_MGMT_SELECTOR_MODE_FORCED_FREE_RUN;
  } else {
    value.selection.mode = SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_REVERTIVE; /* default selection mode */
  }

  value.selection.clk_src = clk_src;
  VTSS_ICLI_ERR_PRINT(synce_icli_selection_set(conf, value, no));
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc synce_icli_prio(i32 session_id, icli_range_t *clk_list, u8 prio, BOOL no)
{
  synce_conf_type_t conf = PRIO;
  synce_conf_value_t value;

  value.prio = prio;

  VTSS_ICLI_ERR_PRINT(synce_icli_nominate_set(clk_list, conf, value, no));
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc synce_icli_wtr(i32 session_id, u8 wtr_value, BOOL no)
{
  synce_conf_type_t conf = WTR_CONF;
  synce_conf_value_t value;

  value.wtr = wtr_value;

  VTSS_ICLI_ERR_PRINT(synce_icli_selection_set(conf, value, no));
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc synce_icli_ssm(i32 session_id, icli_range_t *clk_list, synce_icli_ssm_type_t type, BOOL has_prc, BOOL has_ssua, BOOL has_ssub, BOOL has_eec2, BOOL has_eec1, BOOL has_dnu, BOOL has_inv, BOOL no)
{
  synce_conf_type_t conf;
  if (type == HOLDOVER) {
    conf = SSM_HOLDOVER;
  } else if (type == FREERUN) {
    conf = SSM_FREERUN;
  } else if (type == OVERWRITE) {
    conf = SSM_OVERWRITE;
  } else {
    T_E("Unknown type :%d", type);
    return VTSS_RC_ERROR;
  }

  synce_conf_value_t value;

  if (has_prc) {
    value.ssm_holdover = QL_PRC;
  } else if (has_ssua) {
    value.ssm_holdover = QL_SSUA;
  } else if (has_ssub) {
    value.ssm_holdover = QL_SSUB;
  } else if (has_eec2) {
    value.ssm_holdover = QL_EEC2;
  } else if (has_eec1) {
    value.ssm_holdover = QL_EEC1;
  } else if (has_inv) {
    value.ssm_holdover = QL_INV;
  } else if (has_dnu) {
    value.ssm_holdover = QL_DNU;
  } else {
    value.ssm_holdover = QL_NONE;
  }

  if (conf == SSM_OVERWRITE) {
    VTSS_ICLI_ERR_PRINT(synce_icli_nominate_set(clk_list, conf, value, no));
  } else {
    VTSS_ICLI_ERR_PRINT(synce_icli_selection_set(conf, value, no));
  }
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc sycne_icli_option(i32 session_id, BOOL has_eec1, BOOL has_eec2, BOOL no)
{
  synce_conf_type_t conf = EEC_OPTION;
  synce_conf_value_t value;

  if (has_eec1) {
    value.eec_option = EEC_OPTION_1;
  } else if (has_eec2) {
    value.eec_option = EEC_OPTION_2;
  } else {
    value.eec_option = EEC_OPTION_1; // Assign a default value.
  }
  VTSS_ICLI_ERR_PRINT(synce_icli_selection_set(conf, value, no));
  return VTSS_RC_OK;
}

//  see synce_icli_functions.h
vtss_rc synce_icli_sync(i32 session_id, icli_stack_port_range_t *plist, BOOL no)
{
  VTSS_ICLI_ERR_PRINT(sync_ssm_enable(plist, no));
  return VTSS_RC_OK;

}
/***************************************************************************/
/* ICFG callback functions */
/****************************************************************************/
#ifdef VTSS_SW_OPTION_ICFG
static vtss_rc synce_icfg_conf(const vtss_icfg_query_request_t *req,
                               vtss_icfg_query_result_t *result)
{
  vtss_isid_t isid;
  vtss_port_no_t iport;
  vtss_icfg_conf_print_t conf_print;
  vtss_icfg_conf_print_init(&conf_print);

  i8 buf[75]; // Buffer for storage of string

  isid =  req->instance_id.port.isid;

  // Get default configuration
  synce_mgmt_clock_conf_blk_t  clock_conf_default;
  synce_mgmt_port_conf_blk_t   p_conf_default;
  synce_set_conf_to_default(&clock_conf_default, &p_conf_default);

  T_NG(TRACE_GRP_CLI, "mode:%d", req->cmd_mode);
  switch (req->cmd_mode) {
  case ICLI_CMD_MODE_GLOBAL_CONFIG: {

    // Get current configuration
    synce_mgmt_clock_conf_blk_t  clock_conf;
    VTSS_RC(synce_mgmt_clock_config_get(&clock_conf));

    synce_mgmt_frequency_t clk_freq;
    //
    // Input clock
    //
    if (synce_icli_any_input_freq_supported()) {
      VTSS_RC(synce_mgmt_station_clock_in_get(&clk_freq));

      conf_print.is_default = clk_freq == clock_conf_default.station_clk_in;
      VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock input-source", "%s",
                                   clk_freq == SYNCE_MGMT_STATION_CLK_1544_KHZ ? "1544khz" :
                                   clk_freq == SYNCE_MGMT_STATION_CLK_2048_KHZ ? "2048khz" :
                                   clk_freq == SYNCE_MGMT_STATION_CLK_10_MHZ ? "10mhz" :
                                   "Wrong parameter"));
    }
    //
    // Output clock
    //
    if (synce_icli_any_output_freq_supported()) {
      VTSS_RC(synce_mgmt_station_clock_out_get(&clk_freq));

      conf_print.is_default = clk_freq == clock_conf_default.station_clk_out;
      VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock output-source", "%s",
                                   clk_freq == SYNCE_MGMT_STATION_CLK_1544_KHZ ? "1544khz" :
                                   clk_freq == SYNCE_MGMT_STATION_CLK_2048_KHZ ? "2048khz" :
                                   clk_freq == SYNCE_MGMT_STATION_CLK_10_MHZ ? "10mhz" :
                                   "Wrong parameter"));
    }


    //
    // wtr
    //
    conf_print.is_default = clock_conf.wtr_time == clock_conf_default.wtr_time;
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock wait-to-restore", "%u", clock_conf.wtr_time));

    //
    // EEC option
    //
    conf_print.is_default = clock_conf.eec_option == clock_conf_default.eec_option;
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock option", "%s", clock_conf.eec_option == EEC_OPTION_1 ? "eec1" :
                                 clock_conf.eec_option == EEC_OPTION_2 ? "eec2" :
                                 "error"));

    //
    // SSH HOLDOVER
    //
    conf_print.is_default = clock_conf.ssm_holdover == clock_conf_default.ssm_holdover;
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock ssm-holdover", "%s", clock_conf.ssm_holdover == QL_PRC ? "prc" :
                                 clock_conf.ssm_holdover == QL_SSUA ? "ssua" :
                                 clock_conf.ssm_holdover == QL_SSUB ? "ssub" :
                                 clock_conf.ssm_holdover == QL_EEC1 ? "eec1" :
                                 clock_conf.ssm_holdover == QL_EEC2 ? "eec2" :
                                 clock_conf.ssm_holdover == QL_INV ? "inv" :
                                 "dnu"));

    //
    // SSH FREE RUNNING
    //
    conf_print.is_default = clock_conf.ssm_freerun == clock_conf_default.ssm_freerun;
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock ssm-freerun", "%s", clock_conf.ssm_freerun == QL_PRC ? "prc" :
                                 clock_conf.ssm_freerun == QL_SSUA ? "ssua" :
                                 clock_conf.ssm_freerun == QL_SSUB ? "ssub" :
                                 clock_conf.ssm_freerun == QL_EEC1 ? "eec1" :
                                 clock_conf.ssm_freerun == QL_EEC2 ? "eec2" :
                                 clock_conf.ssm_freerun == QL_INV ? "inv" :
                                 "dnu"));
    //
    // Selection
    //
    conf_print.is_default = clock_conf.selection_mode == clock_conf_default.selection_mode;

    switch (clock_conf.selection_mode) {
    case SYNCE_MGMT_SELECTOR_MODE_MANUEL:
      sprintf(buf, "manual clk-source %u", clock_conf.source);
      break;
    case SYNCE_MGMT_SELECTOR_MODE_MANUEL_TO_SELECTED:
      strcpy(buf, "selected");
      break;
    case SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_NONREVERTIVE:
      strcpy(buf, "nonrevertive");
      break;
    case SYNCE_MGMT_SELECTOR_MODE_AUTOMATIC_REVERTIVE:
      strcpy(buf, "revertive");
      break;
    case SYNCE_MGMT_SELECTOR_MODE_FORCED_HOLDOVER:
      strcpy(buf, "holdover");
      break;
    case SYNCE_MGMT_SELECTOR_MODE_FORCED_FREE_RUN:
      strcpy(buf, "freerun");
      break;
    }

    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock selector", "%s", &buf[0]));

    //
    // The commands below takes a list of source clock as arguments,
    // so we build a BOOL list in order to avoid having to print all clock sources if the configuration is the same
    //

    BOOL default_list[SYNCE_NOMINATED_MAX];

    u8 clk_src;
    conf_print.bool_list_max = synce_my_nominated_max - 1; // synce_my_nominated_max is in fact the number of clocks, while the bool_list_max is the last index
    conf_print.bool_list_min = 0;

    //
    // Nominate
    //
    for (clk_src = 0; clk_src < synce_my_nominated_max; clk_src++) {
      default_list[clk_src] = FALSE;

      if (clock_conf.nominated[clk_src] == clock_conf_default.nominated[clk_src]) {
        default_list[clk_src] = TRUE;
      } else {
        conf_print.is_default = FALSE;
        // Print "non-no" commands
        if (clock_conf.port[clk_src] == SYNCE_STATION_CLOCK_PORT) {
          VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "%d nominate clk-in",
                                       synce_iclk2uclk(clk_src))); // Not supporting stacking at the moment.
        } else {
          VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "%d nominate interface %s",
                                       synce_iclk2uclk(clk_src), icli_port_info_txt(VTSS_USID_START, iport2uport(clock_conf.port[clk_src]), buf))); // Not supporting stacking at the moment.
        }
      }
    }

    // Print no commands
    conf_print.print_no_arguments = TRUE;
    conf_print.is_default = TRUE;
    conf_print.bool_list = &default_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "nominate"));


    //
    // Aneg
    //
    BOOL master_list[SYNCE_NOMINATED_MAX];
    BOOL slave_list[SYNCE_NOMINATED_MAX];
    BOOL forced_list[SYNCE_NOMINATED_MAX];

    for (clk_src = 0; clk_src < synce_my_nominated_max; clk_src++) {
      default_list[clk_src] = FALSE;
      forced_list[clk_src] = FALSE;
      master_list[clk_src] = FALSE;
      slave_list[clk_src] = FALSE;

      if (clock_conf.aneg_mode[clk_src] == clock_conf_default.aneg_mode[clk_src]) {
        default_list[clk_src] = TRUE;
      }

      if (clock_conf.aneg_mode[clk_src] == SYNCE_MGMT_ANEG_PREFERED_MASTER) {
        master_list[clk_src] = TRUE;
      }

      if (clock_conf.aneg_mode[clk_src] == SYNCE_MGMT_ANEG_PREFERED_SLAVE) {
        T_E("clk_src:%d", clk_src);
        slave_list[clk_src] = TRUE;
      }

      if (clock_conf.aneg_mode[clk_src] == SYNCE_MGMT_ANEG_FORCED_SLAVE) {
        forced_list[clk_src] = TRUE;
      }
    }

    // Print no commands
    conf_print.print_no_arguments = TRUE;
    conf_print.is_default = TRUE;
    conf_print.bool_list = &default_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "aneg-mode"));

    // Print master commands
    conf_print.is_default = FALSE;
    conf_print.bool_list = &master_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "aneg-mode master"));

    // Print slave commands
    conf_print.is_default = FALSE;
    conf_print.bool_list = &slave_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "aneg-mode slave"));

    // Print force commands
    conf_print.is_default = FALSE;
    conf_print.bool_list = &forced_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "aneg-mode forced"));


    //
    // Prio
    //
    BOOL prio_list[SYNCE_NOMINATED_MAX];

    for (clk_src = 0; clk_src < synce_my_nominated_max; clk_src++) {
      prio_list[clk_src] = FALSE;
      default_list[clk_src] = FALSE;

      if (clock_conf.priority[clk_src] == clock_conf_default.priority[clk_src]) {
        default_list[clk_src] = TRUE;
      } else {
        prio_list[clk_src] = TRUE;
      }
    }

    // Print no commands
    conf_print.is_default = TRUE;
    conf_print.bool_list = &default_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "priority"));

    // Print "non-no" commands
    conf_print.is_default = FALSE;
    conf_print.bool_list = &prio_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "priority %u", clock_conf.priority[clk_src]));

    //
    // SSM overwrite
    //
    BOOL prc_list[SYNCE_NOMINATED_MAX];
    BOOL dnu_list[SYNCE_NOMINATED_MAX];
    BOOL eec1_list[SYNCE_NOMINATED_MAX];
    BOOL eec2_list[SYNCE_NOMINATED_MAX];
    BOOL ssub_list[SYNCE_NOMINATED_MAX];
    BOOL ssua_list[SYNCE_NOMINATED_MAX];

    for (clk_src = 0; clk_src < synce_my_nominated_max; clk_src++) {
      default_list[clk_src] = FALSE;
      prc_list[clk_src] = FALSE;
      dnu_list[clk_src] = FALSE;
      eec1_list[clk_src] = FALSE;
      eec2_list[clk_src] = FALSE;
      ssub_list[clk_src] = FALSE;
      ssua_list[clk_src] = FALSE;

      if (clock_conf.ssm_overwrite[clk_src] == clock_conf_default.ssm_overwrite[clk_src]) {
        default_list[clk_src] = TRUE;
      }

      if (clock_conf.ssm_overwrite[clk_src] == QL_PRC) {
        prc_list[clk_src] = TRUE;
      }

      if (clock_conf.ssm_overwrite[clk_src] == QL_DNU) {
        dnu_list[clk_src] = TRUE;
      }

      if (clock_conf.ssm_overwrite[clk_src] == QL_EEC2) {
        eec2_list[clk_src] = TRUE;
      }

      if (clock_conf.ssm_overwrite[clk_src] == QL_EEC1) {
        eec1_list[clk_src] = TRUE;
      }

      if (clock_conf.ssm_overwrite[clk_src] == QL_SSUB) {
        ssub_list[clk_src] = TRUE;
      }

      if (clock_conf.ssm_overwrite[clk_src] == QL_SSUA) {
        ssua_list[clk_src] = TRUE;
      }
    }

    // Print no/default
    conf_print.print_no_arguments = TRUE;
    conf_print.is_default = TRUE;
    conf_print.bool_list = &default_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "ssm-overwrite"));

    conf_print.is_default = FALSE;

    // Print prc commands
    conf_print.bool_list = &prc_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "ssm-overwrite prc"));

    // Print dnu commands
    conf_print.bool_list = &dnu_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "ssm-overwrite dnu"));

    // Print eec2 commands
    conf_print.bool_list = &eec2_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "ssm-overwrite eec2"));

    // Print eec1 commands
    conf_print.bool_list = &eec1_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "ssm-overwrite eec1"));

    // Print ssub commands
    conf_print.bool_list = &ssub_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "ssm-overwrite ssub"));

    // Print ssua commands
    conf_print.bool_list = &ssua_list[0];
    VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock clk-source", "ssm-overwrite ssua"));

    break;
    case ICLI_CMD_MODE_INTERFACE_PORT_LIST:
      iport = req->instance_id.port.begin_iport;
      T_DG_PORT(TRACE_GRP_CLI, iport, "Isid:%d, req->instance_id.port.usid:%d configurable:%d",
                isid, req->instance_id.port.usid, msg_switch_configurable(isid));

      if (msg_switch_configurable(isid)) {

        // Get current configuration
        synce_mgmt_port_conf_blk_t      port_conf;
        VTSS_RC(synce_mgmt_port_config_get(&port_conf));

        //
        // Enable/Disable
        //

        conf_print.is_default =  port_conf.ssm_enabled[iport] == p_conf_default.ssm_enabled[iport];
        VTSS_RC(vtss_icfg_conf_print(req, result, conf_print, "network-clock synchronization ssm", ""));

      }
      break;

    }
  default:
    //Not needed
    break;
  }

  return VTSS_RC_OK;
}

/* ICFG Initialization function */
vtss_rc synce_icfg_init(void)
{
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_SYNCE_GLOBAL_CONF, "network-clock", synce_icfg_conf));
  VTSS_RC(vtss_icfg_query_register(VTSS_ICFG_SYNCE_INTERFACE_CONF, "network-clock", synce_icfg_conf));
  return VTSS_RC_OK;
}
#endif // VTSS_SW_OPTION_ICFG
/****************************************************************************/
/*  End of file.                                                            */
/****************************************************************************/
