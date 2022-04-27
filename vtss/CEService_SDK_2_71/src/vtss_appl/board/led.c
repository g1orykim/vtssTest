/* -*- Mode: C; c-basic-offset: 2; tab-width: 8; c-comment-only-line-offset: 0; -*- */
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

#include <cyg/hal/hal_io.h>

#include "vtss_os.h"
#include "main.h"
#include "led_api.h"
#include "led.h"
#include "port_api.h"  /* For port_phy_wait_until_ready() */
#include "interrupt_api.h"

// ===========================================================================
// Trace
// ---------------------------------------------------------------------------

#if (VTSS_TRACE_ENABLED)
static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "led",
    .descr     = "LED module"
};


#ifndef LED_DEFAULT_TRACE_LVL
#define LED_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_ERROR
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = { 
        .name      = "default",
        .descr     = "Default",
        .lvl       = LED_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
};
#endif // VTSS_TRACE_ENABLED

// ===========================================================================


/****************************************************************************/
/*                                                                          */
/*  NAMING CONVENTION FOR INTERNAL FUNCTIONS:                               */
/*    LED_<function_name>                                                    */
/*                                                                          */
/*  NAMING CONVENTION FOR EXTERNAL (API) FUNCTIONS:                         */
/*    led_<function_name>                                                */
/*                                                                          */
/****************************************************************************/

// LED thread variables
static cyg_handle_t          LED_thread_handle;
static cyg_thread            LED_thread_state;  // Contains space for the scheduler to hold the current thread state.
static char                  LED_thread_stack[THREAD_DEFAULT_STACK_SIZE/2]; // The LED thread by default has a stack size of 2400 bytes. This is not enough when T_E() occuring.
static led_front_led_state_t LED_next_front_led_state;
static vtss_os_crit_t        LED_front_led_crit;
static vtss_os_crit_t        LED_int_crit;
static BOOL                  LED_is_master_in_stack;
static vtss_usid_t           LED_usid = VTSS_USID_ALL;
static BOOL                  LED_init_done;

/****************************************************************************/
// The .colors member must end with an lc_NULL, since that tells the number
// of sub-states that the LED undergo before starting all over.
// The .timeout member must be one shorter than the .colors. It contains
// the number of milliseconds (>= 10) to wait when going from one sub-state to
// the next.
// For instance: In the LED_FRONT_LED_NORMAL state the first color displayed
// is lc_GREEN, then it waits 500 ms (first entry in .timeout), then it
// turns off the led (lc_OFF) and waits another 500 ms (second entry in
// .timeout). Now, since the next entry is lc_NULL, the state machine goes back
// and turns the LED green again.
/****************************************************************************/
static LED_front_led_state_cfg_t LED_front_led_state_cfg[] = {
  [LED_FRONT_LED_NORMAL] = {
    .colors           = {lc_GREEN, lc_OFF, lc_NULL}, // DO NOT CHANGE THIS ONE. IT'S JUST A PLACEHOLDER
    .timeout_ms       = {500, 500},                  // IF YOU NEED TO CHANGE THE NORMAL MODE, CHANGE
    .least_next_state = LED_FRONT_LED_NORMAL,     // EITHER OF THE MASTER OR SLAVE CONFIGURATIONS BELOW
  },                                                 // (LED_front_led_master_cfg or LED_front_led_slave_cfg).
  [LED_FRONT_LED_FLASHING_BOARD] = {
    .colors           = {lc_GREEN, lc_OFF, lc_NULL},
    .timeout_ms       = {100, 100},
    .least_next_state = LED_FRONT_LED_NORMAL,
  },
  [LED_FRONT_LED_STACK_FW_CHK_ERROR] = {
    .colors           = {lc_GREEN, lc_RED, lc_NULL},
    .timeout_ms       = {100, 100},
    .least_next_state = LED_FRONT_LED_NORMAL,
  },
  [LED_FRONT_LED_ERROR] = {
    .colors           = {lc_RED, lc_OFF, lc_NULL},
    .timeout_ms       = {500, 500},
    .least_next_state = LED_FRONT_LED_ERROR, // Cannot go below LED_FRONT_LED_ERROR when first set.
  },
  [LED_FRONT_LED_FATAL] = {
    .colors           = {lc_RED, lc_NULL},      // The FATAL state should only contain one single color, since it may be that we never enter the LED_thread(), which is used to toggle colors, again.
    .timeout_ms       = {2000},                 // Even when only one color is displayed, we must be able to wake up the thread
    .least_next_state = LED_FRONT_LED_FATAL, // Cannot go below LED_FRONT_LED_FATAL when first set.
  },
};

// These two are meant to be copied into the LED_FRONT_LED_NORMAL entry in
// the LED_front_led_state_cfg[] array depending on the master/slave state.
static LED_front_led_state_cfg_t LED_front_led_master_state_cfg = {
  .colors           = {lc_GREEN, lc_NULL},
  .timeout_ms       = {200},
  .least_next_state = LED_FRONT_LED_NORMAL,
};
static LED_front_led_state_cfg_t LED_front_led_slave_state_cfg = {
  .colors           = {lc_OFF, lc_NULL},
  .timeout_ms       = {200},
  .least_next_state = LED_FRONT_LED_NORMAL,
};

/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS - LED board dependent functions               */
/*                                                                          */
/****************************************************************************/

/*
 * This implementation can be overridden.
 */
void LED_led_init(void) __attribute__ ((weak, alias("__LED_led_init")));

/* The reference implementation - dummy */
static 
void __LED_led_init(void)
{
}

/*
 * This implementation can be overridden.
 */
void LED_front_led_set(LED_led_colors_t color) __attribute__ ((weak, alias("__LED_front_led_set")));

/* The reference implementation - dummy */
static
void __LED_front_led_set(LED_led_colors_t color)
{
}

/*
 * This implementation can be overridden.
 */
void LED_master_led_set(BOOL master) __attribute__ ((weak, alias("__LED_master_led_set")));

/* The reference implementation - dummy */
static
void __LED_master_led_set(BOOL master)
{
}



/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS - LCD board dependent functions               */
/*                                                                          */
/****************************************************************************/

/*
 * This implementation can be overridden.
 */
void LED_lcd_set(lcd_symbol_t sym1, lcd_symbol_t sym2) __attribute__ ((weak, alias("__LED_lcd_set")));

/* The reference implementation - dummy */
static
void __LED_lcd_set(lcd_symbol_t sym1, lcd_symbol_t sym2)
{
}

/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS                                               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/****************************************************************************/
static void LED_front_led_state_cfg_sanity_check(LED_front_led_state_cfg_t *cfg)
{
  int i;

  BOOL null_color_seen = FALSE;
  // The first color cannot be NULL, since that's the "color" used to designate the last
  // color before starting over.
  VTSS_ASSERT(cfg->colors[0] != lc_NULL);

  for (i = 1; i <= LED_FRONT_LED_MAX_SUBSTATES; i++) {
    // The timeout between two consecutive sub-states must
    // be >= 10 ms and less than or equal to, say, 2 seconds.
    VTSS_ASSERT(cfg->timeout_ms[i-1] >= 10 && cfg->timeout_ms[i-1] <= 2000);
    if(cfg->colors[i]==lc_NULL) {
      null_color_seen = TRUE;
      break;
    }
  }

  // The array must have been terminated with an lc_NULL before the end of the sub-states
  VTSS_ASSERT(null_color_seen);
}

/****************************************************************************/
/****************************************************************************/
cyg_flag_t  interrupt_flag;
#if defined(VTSS_ARCH_SERVAL)
static BOOL                  LED_do_update_tower = TRUE;
// Function used if push button is interrupt driven.
// In : None of the inputs are used at the moment,
static void LED_push_button_interrupt(vtss_interrupt_source_t     source_id,
                                      u32                         instance_id)
{
  T_N("LED_push_button_interrupt");
  cyg_flag_setbits(&interrupt_flag, 0xFFFFFFFF); // Wake the LED thread
  LED_do_update_tower = TRUE; // Signal to thread to update the tower LEDs  
}

static void LED_check_for_tower_update(void) {
  T_N("LED_do_update_tower:%d", LED_do_update_tower);
  if (LED_do_update_tower) {
    led_tower_update();
      
      LED_do_update_tower = FALSE;

      // Wait 200 mSec between each interrupt in order to avoid that short "pushes" are detected as multiple "pushes" 
      VTSS_OS_MSLEEP(200);

      // Hook up for new interrupts
      T_D("Hooking up");
      (void) vtss_interrupt_source_hook_set(LED_push_button_interrupt,
                                            INTERRUPT_SOURCE_SGPIO_PUSH_BUTTON,
                                            INTERRUPT_PRIORITY_NORMAL);
  }  
}
#endif


static void LED_thread(cyg_addrword_t data)
{
  led_front_led_state_t cur_led_state=(led_front_led_state_t) - 1;
  int substate_idx = 0;
  BOOL initial;
  cyg_tick_count_t time_tick;
  cyg_flag_init(&interrupt_flag);

  // This will block this thread from running further until the PHYs are initialized.
  port_phy_wait_until_ready();

  /* Board dependent */
  LED_led_init();
  LED_init_done = TRUE;

  // Can't call these functions until the API is initialized.
  if (LED_usid == VTSS_USID_ALL) {
    LED_lcd_set(LCD_SYMBOL_DASH, LCD_SYMBOL_DASH);
  } else {
    led_usid_set(LED_usid);
  }
  
  initial = TRUE;
  while (1) {
    VTSS_OS_CRIT_ENTER(&LED_front_led_crit);
    if(initial || cur_led_state != LED_next_front_led_state) {
      cur_led_state = LED_next_front_led_state;
      substate_idx=0;
      initial = FALSE;
    }
    VTSS_OS_CRIT_EXIT(&LED_front_led_crit);

    LED_front_led_set(LED_front_led_state_cfg[cur_led_state].colors[substate_idx]);

    time_tick = cyg_current_time() + VTSS_OS_MSEC2TICK(LED_front_led_state_cfg[cur_led_state].timeout_ms[substate_idx]);
    (void)cyg_flag_timed_wait(&interrupt_flag, 0xFFFFFFFF, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, time_tick);
    
#if defined(VTSS_ARCH_SERVAL)
    T_N("LED_check_for_tower_update");
    LED_check_for_tower_update();

    if (!LED_do_update_tower) 
#endif
      if (LED_front_led_state_cfg[cur_led_state].colors[++substate_idx] == lc_NULL)
        substate_idx = 0;
  }
}

/****************************************************************************/
/*                                                                          */
/*  MODULE EXTERNAL FUNCTIONS                                               */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
// Function for checking if the front LED is indicating error/fatal.
// Return : TRUE if LED is in error/fatal state (LED flashing red) else FALSE
/****************************************************************************/
BOOL led_front_led_in_error_state(void) {
  T_N("LED_next_front_led_state:%d", LED_next_front_led_state);
  return  (LED_next_front_led_state == LED_FRONT_LED_ERROR || LED_next_front_led_state == LED_FRONT_LED_FATAL);
}

/****************************************************************************/
// led_front_led_state()
/****************************************************************************/
void led_front_led_state(led_front_led_state_t state)
{
  BOOL local_led_init_done = LED_init_done;

  // Guard against someone having added a new state to led_front_led_state_t
  // without having added a configuration to the LED_front_led_state_cfg[] array.
  // Use CYG_ASSERT() rather than VTSS_ASSERT() to avoid recursion.
  CYG_ASSERT(state < sizeof(LED_front_led_state_cfg) / sizeof(LED_front_led_state_cfg[0]), "");

  if (local_led_init_done) {
    // Can't (shouldn't) take semaphore is scheduler is not running
    VTSS_OS_CRIT_ENTER(&LED_front_led_crit);
  }
  // Don't allow a smaller new state than the current state's least_next_state member.
  if (state < LED_front_led_state_cfg[LED_next_front_led_state].least_next_state) {
    goto exit_func;
  }
    
  LED_next_front_led_state = state;

  // In case of a fatal, we may end up with not entering the LED_thread()
  // again, so we better force the LED to the color of fatality
  // right away - provided the LED API is initialized.
  if (state == LED_FRONT_LED_FATAL && local_led_init_done) {
    LED_front_led_set(LED_front_led_state_cfg[LED_FRONT_LED_FATAL].colors[0]);
  }

exit_func:
  if (local_led_init_done) {
    VTSS_OS_CRIT_EXIT(&LED_front_led_crit);
  }
}

/****************************************************************************/
// led_master_in_stack_set()
// For now, it causes the front LED to blink a bit differently depending
// on the master or slave setting.
/****************************************************************************/
static void led_master_in_stack_set(BOOL master)
{
  BOOL local_led_init_done = LED_init_done;

  if (local_led_init_done) {
    VTSS_OS_CRIT_ENTER(&LED_front_led_crit);
  }
  LED_is_master_in_stack = master;
  if (master) {
    // Copy the master LED configuration
    memcpy(&LED_front_led_state_cfg[LED_FRONT_LED_NORMAL], &LED_front_led_master_state_cfg, sizeof(LED_front_led_master_state_cfg));
  } else {
    memcpy(&LED_front_led_state_cfg[LED_FRONT_LED_NORMAL], &LED_front_led_slave_state_cfg, sizeof(LED_front_led_slave_state_cfg));
  }
  if (local_led_init_done) {
    VTSS_OS_CRIT_EXIT(&LED_front_led_crit);
  }
}

/****************************************************************************/
// led_usid_set()
// Set USID value to be shown on 7-segment LEDs.
// If usid==0, then "-" is shown. 
/****************************************************************************/
void led_usid_set(vtss_usid_t usid)
{
    T_I("usid=%d", usid);
    LED_usid = usid;

    if (!LED_init_done) {
      return; // Defer to start of thread.
    }
    
    /* Set decimal or "invalid" */
    if(VTSS_USID_LEGAL(usid)) {
      lcd_symbol_t sym1 = (usid / 10), sym2 = (usid % 10);
      VTSS_ASSERT(sym1 <= LCD_SYMBOL_9);
      LED_lcd_set(sym1, sym2);  /* '99' */
    } else {
      LED_lcd_set(LCD_SYMBOL_PERIOD, LCD_SYMBOL_PERIOD); /* Invalid */
    }
} // led_usid_set

/****************************************************************************/
// led_init()
/****************************************************************************/
vtss_rc led_init(vtss_init_data_t *data)
{
  switch(data->cmd) {
  case INIT_CMD_INIT: 
  {
    int i;

    // Initialize and register trace ressources
    VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
    VTSS_TRACE_REGISTER(&trace_reg);

    // The front led state must also be protected by a critical region.
    VTSS_OS_CRIT_CREATE(&LED_front_led_crit);
    VTSS_OS_CRIT_CREATE(&LED_int_crit);

    // Sanity check of LED_front_led_state_cfg[] and master/slave dittos
    for(i = 0; i < sizeof(LED_front_led_state_cfg)/sizeof(LED_front_led_state_cfg[0]); i++) {
      LED_front_led_state_cfg_sanity_check(&LED_front_led_state_cfg[i]);
    }
    LED_front_led_state_cfg_sanity_check(&LED_front_led_master_state_cfg);
    LED_front_led_state_cfg_sanity_check(&LED_front_led_slave_state_cfg);
    
    led_master_in_stack_set(VTSS_SWITCH_MANAGED); // Managed is initially master in the stack.
    LED_next_front_led_state = LED_FRONT_LED_NORMAL;

    // Create thread that receives "tx done" messages and callback packet txers in thread context
    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      LED_thread,
                      0,
                      "LED",
                      LED_thread_stack,
                      sizeof(LED_thread_stack),
                      &LED_thread_handle,
                      &LED_thread_state);

    cyg_thread_resume(LED_thread_handle);
    
    break;
  }
  case INIT_CMD_MASTER_UP:     /* Change from SLAVE to MASTER state */
  case INIT_CMD_MASTER_DOWN:   /* Change from MASTER to SLAVE state */
    LED_master_led_set(data->cmd == INIT_CMD_MASTER_UP);
    led_master_in_stack_set(data->cmd == INIT_CMD_MASTER_UP);
    break;
  default:;                     /* Avoid warning */
  }
  return VTSS_OK;
}

/****************************************************************************/
// Debug functions
/****************************************************************************/

// Return current USID LED value
vtss_usid_t led_usid_get(void) 
{
    return LED_usid;
} // led_usid_get


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
