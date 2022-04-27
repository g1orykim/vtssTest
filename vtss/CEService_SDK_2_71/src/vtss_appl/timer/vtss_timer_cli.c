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

#include "cli_api.h"        /* For cli_xxx()                                           */
#include "cli.h"            /* For cli_req_t (sadly enough)                            */
#include "vtss_timer_api.h" /* For vtss_timer_xxx() functions.                         */
#include "vtss_timer_cli.h" /* Check that public function decls. and defs. are in sync */

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_TIMER

typedef struct {
  vtss_timer_t *timer;
} timer_cli_req_t;

#define TIMER_CLI_CMD_DEBUG_CANCEL "Debug Timer Cancel"

/****************************************************************************/
// TIMER_get_cur_time()
/****************************************************************************/
static char *TIMER_get_cur_time(char *buf)
{
  cyg_uint64 usecs = hal_time_get();
  cyg_uint64 s, m, u;

  s = (usecs / 1000000ULL);
  m = (usecs - 1000000ULL * s) / 1000ULL;
  u = (usecs - 1000000ULL * s - 1000ULL * m);

  sprintf(buf, "%02llu.%03llu,%03llu ", s, m, u);
  return buf;
}

/****************************************************************************/
// TIMER_cli_callback()
/****************************************************************************/
static void TIMER_cli_callback(vtss_timer_t *timer)
{
  char buf[50];
  // Cannot use cli_printf() here, since this function is called from a non-CLI
  // context thread. It may even be called from DSR context, so use diag_printf()
  // which is safe in all contexts.
  (void)diag_printf("Timer(0x%08x) expired at %s. Total count = %u.", (u32)timer, TIMER_get_cur_time(buf), timer->total_cnt);
  if (timer->repeat) {
    (void)diag_printf("\n");
  } else {
    VTSS_FREE(timer);
    (void)diag_printf(" Done.\n");
  }
}

/****************************************************************************/
// Command invokation functions
/****************************************************************************/

/****************************************************************************/
// TIMER_cli_cmd_debug_start()
/****************************************************************************/
static void TIMER_cli_cmd_debug_start(cli_req_t *req)
{
  char          buf[50];
  vtss_timer_t *timer = VTSS_MALLOC(sizeof(*timer));

  if (timer == NULL) {
    cli_printf("Error: Out of memory\n");
    return;
  }

  (void)vtss_timer_initialize(timer);
  timer->period_us = req->value;
  timer->repeat    = req->enable;
  timer->callback  = TIMER_cli_callback;
  timer->modid     = VTSS_MODULE_ID_TIMER;

  if (vtss_timer_start(timer) == VTSS_RC_OK) {
    cli_printf("Timer started at %s\n", TIMER_get_cur_time(buf));
    cli_printf("Value to use in call to \"%s\": 0x%08x\n", TIMER_CLI_CMD_DEBUG_CANCEL, (u32)timer);
  } else {
    // Error message already printed.
    VTSS_FREE(timer);
  }
}

/****************************************************************************/
// TIMER_cli_do_cancel()
/****************************************************************************/
static void TIMER_cli_do_cancel(vtss_timer_t *timer)
{
  // We can't de-reference #timer here to check if it's our module's timer, because
  // the user may have requested an invalid timer pointer to cancel.
  if (vtss_timer_cancel(timer) == VTSS_RC_OK) {
    cli_printf("Timer(0x%08x) expired for a total of %u times\n", (u32)timer, timer->total_cnt);
    if ((timer->modid) == VTSS_MODULE_ID_TIMER) {
      // Other modules may have allocated the timer statically. We might end up with
      // unfreed memory on the heap this way, but what the heck - this is a debug function.
      VTSS_FREE(timer);
    }
  } else {
    cli_printf("Error: Timer cancel failed on timer = 0x%08x\n", (u32)timer);
  }
}

/****************************************************************************/
// TIMER_cli_cmd_debug_cancel()
/****************************************************************************/
static void TIMER_cli_cmd_debug_cancel(cli_req_t *req)
{
  timer_cli_req_t *timer_req = req->module_req;

  if (req->all) {
    vtss_timer_t *timers, *timer;
    u64          total_cnt;
    (void)vtss_timer_list(&timers, &total_cnt);
    timer = timers;
    while (timer) {
      if (timer->modid == VTSS_MODULE_ID_TIMER) {
        // We only cancel timers that are ours.
        TIMER_cli_do_cancel(timer->this);
      }
      timer = timer->next_active;
    }
    VTSS_FREE(timers);
  } else {
    TIMER_cli_do_cancel(timer_req->timer);
  }
}

/****************************************************************************/
// TIMER_cli_cmd_debug_list()
/****************************************************************************/
static void TIMER_cli_cmd_debug_list(cli_req_t *req)
{
  vtss_timer_t *timer, *timers;
  u64          cnt = 0, total_cnt;
  (void)vtss_timer_list(&timers, &total_cnt);

  cli_printf("\nvtss_timer_start() has been called %llu times\n", total_cnt);
  timer = timers;
  while (timer) {
    if (cnt++ == 0) {
      cli_printf("\n");
      cli_table_header("Timer       Module               Period [us]  Repeat  Context  Expirations");
    }
    cli_printf("0x%08x  %-19s  %11u  %-6s  %-7s  %11u\n", (u32)timer->this, vtss_module_names[timer->modid], timer->period_us, timer->repeat ?  "Yes" : "No", timer->dsr_context ? "DSR" : "Thread", timer->total_cnt);
    timer = timer->next_active;
  }
  cli_printf("\nNumber of timers: %llu\n", cnt);
  VTSS_FREE(timers);
}

/****************************************************************************/
// Parameter parse functions
/****************************************************************************/

/****************************************************************************/
// TIMER_cli_parse_timer_ptr()
/****************************************************************************/
static int32_t TIMER_cli_parse_timer_ptr(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
  int32_t          error     = 0;
  timer_cli_req_t *timer_req = req->module_req;
  char            *found     = cli_parse_find(cmd, "all");

  req->parm_parsed = 1;

  if (found && !strncmp(found, "all", 3)) {
    req->all = 1;
  } else {
    ulong val;
    error = cli_parse_ulong(cmd, &val, 0, 0xFFFFFFFF);
    timer_req->timer = (vtss_timer_t *)val;
  }

  return error;
}

/****************************************************************************/
// TIMER_cli_parse_period()
/****************************************************************************/
static int32_t TIMER_cli_parse_period(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
  int32_t error = 0;

  req->parm_parsed = 1;
  error = cli_parse_ulong(cmd, &req->value, 0x1, 0xFFFFFFFF);
  return error;
}

/****************************************************************************/
// TIMER_cli_parse_repeat()
/****************************************************************************/
static int32_t TIMER_cli_parse_repeat(char *cmd, char *cmd2, char *stx, char *cmd_org, cli_req_t *req)
{
  char *found = cli_parse_find(cmd, stx);

  req->parm_parsed = 1;
  if (found && !strncmp(found, "repeat", 6)) {
    req->enable = TRUE;
    return 0;
  }

  return 1;
}

/****************************************************************************/
//  Parameter table
/****************************************************************************/
static cli_parm_t TIMER_cli_parm_table[] = {
  {
    "<timer_ptr>",
    "Pointer to a vtss_timer_t structure identifying a timer you wish to cancel (any module's timer can be cancelled). Use 'all' to cancel all timers started through CLI commands",
    CLI_PARM_FLAG_NONE,
    TIMER_cli_parse_timer_ptr,
    TIMER_cli_cmd_debug_cancel
  },
  {
    "<period>",
    "The period measured in microseconds of the timer (must be positive)",
    CLI_PARM_FLAG_NONE,
    TIMER_cli_parse_period,
    TIMER_cli_cmd_debug_start
  },
  {
    "repeat",
    "Specify to have the timer re-fire repeatedly until cancelled\n",
    CLI_PARM_FLAG_SET,
    TIMER_cli_parse_repeat,
    TIMER_cli_cmd_debug_start
  },
  {NULL, NULL, 0, 0, NULL}
};

/****************************************************************************/
// Command table
/****************************************************************************/
cli_cmd_tab_entry (
  NULL,
  "Debug Timer Start <period> [repeat]",
  "Start a one-shot or repeating timer.",
  CLI_CMD_SORT_KEY_DEFAULT,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  TIMER_cli_cmd_debug_start,
  NULL,
  TIMER_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  TIMER_CLI_CMD_DEBUG_CANCEL " <timer_ptr>",
  "Cancel one or all timers.\nWarning: Be careful only to cancel timers started with 'Debug Timer Start', since other timers are started programmatically by the application",
  CLI_CMD_SORT_KEY_DEFAULT,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  TIMER_cli_cmd_debug_cancel,
  NULL,
  TIMER_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
  NULL,
  "Debug Timer List",
  "Show list of currently active timers.",
  CLI_CMD_SORT_KEY_DEFAULT,
  CLI_CMD_TYPE_CONF,
  VTSS_MODULE_ID_DEBUG,
  TIMER_cli_cmd_debug_list,
  NULL,
  TIMER_cli_parm_table,
  CLI_CMD_FLAG_NONE
);

/****************************************************************************/
// vtss_timer_cli_init()
/****************************************************************************/
void vtss_timer_cli_init(void)
{
  // Register the size required for timer req. structure
  cli_req_size_register(sizeof(timer_cli_req_t));
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
