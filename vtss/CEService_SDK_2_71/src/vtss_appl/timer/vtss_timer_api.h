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

#ifndef _VTSS_TIMER_API_H_
#define _VTSS_TIMER_API_H_

#include "main_types.h"     /* For vtss_init_data_t */
#include "vtss_module_id.h" /* For vtss_module_id_t */

typedef struct vtss_timer_s vtss_timer_t;
typedef void (vtss_timer_cb_f)(struct vtss_timer_s *timer);

/**
 * If more than one timer expires at the same time,
 * the priority of the timer determines the order
 * that the callback functions are invoked.
 */
typedef enum {
  VTSS_TIMER_PRIO_LOW,
  VTSS_TIMER_PRIO_NORMAL,
  VTSS_TIMER_PRIO_HIGH,

  // This must come last.
  VTSS_TIMER_PRIO_LAST
} vtss_timer_prio_t;

/**
 * Create an instance of this structure per timer.
 *
 * Implementation hints:
 *
 * If you need more than 4 bytes data, I suggest
 * you create a structure like this:
 *
 * type struct {
 *   vtss_timer_t timer; // Must come first.
 *   my_data_t    my_data;
 * } my_timer_data_t;
 *
 * Then suppose you create an instance of my_timer_data_t.
 * You can do this either statically or dynamically, but
 * it must exist throughout the existence of the timer.
 *
 * static my_timer_data_t my_timer_data;
 *
 * Then initialize the #timer member
 * and call the vtss_timer_xxx() functions
 * with a typecasted version of vtss_timer_xxx().
 * E.g. vtss_timer_start((vtss_timer_t *)&my_timer_data);
 *
 * Once called back, you know that it's actually
 * a my_timer_data_t you're called back with.
 *
 * void my_timer_expired_callback(vtss_timer_t *timer)
 * {
 *    my_timer_data_t *my_timer_data;
 *    my_timer_data = (my_timer_data_t *)timer;
 *    ...
 * }
 *
 * In this case, the vtss_timer_t::user_data field need not be used
 * for anything.
 */
struct vtss_timer_s {
  // --------------------------------------
  // Private members
  // --------------------------------------
  /**
   * Holds the time since boot (in microsoconds) that this timer
   * will fire next time.
   */
  u64 next_abs_timeout_us;

  /**
   * Link to the next timer in the active list
   */
  struct vtss_timer_s *next_active;

  /**
   * Link to the next timer in the done list
   */
  struct vtss_timer_s *next_done;

  /**
   * Counts the number of times the DSR
   * has signaled this timer to be called
   * back.
   */
  u32 dsr_cnt;

  /**
   * Various internal flags
   */
  u32 flags;

  /**
   * A 'this' pointer, pointing to the start of this structure.
   * Only needed by the debug function that cancels a timer.
   */
  struct vtss_timer_s *this;

  // --------------------------------------
  // Public read-only members
  // --------------------------------------
  /**
   * Counts the number of times the timer
   * has expired without being able to call
   * back the application.
   * Can only take on a value greater than 1
   * if #repeat is TRUE.
   */
  u32 cnt;

  /**
   * Counts the total number of times the
   * counter has expired.
   */
  u32 total_cnt;

  // --------------------------------------
  // Public write members
  // --------------------------------------
  /**
   * The timer will trigger this many microseconds from now.
   * See restrictions under #repeat.
   */
  u32 period_us;

  /**
   * When TRUE, the timer will trigger repeatedly every
   * #period_us microseconds.
   * When FALSE, the timer will only trigger once.
   *
   * When TRUE, #period_us must be at least 1000 microseconds.
   */
  BOOL repeat;

  /**
   * When TRUE, #callback function will be called back in DSR context
   * (if applicable on this platform). Otherwise #callback function will
   * be called back in thread context.
   *
   * Whether called back in DSR or thread context, it's permitted to
   * call any vtss_timer_xxx() function and possibly free the resources
   * required for the timer.
   */
  BOOL dsr_context;

  /**
   * Function to callback when the timer expires.
   * Once called back, the resources occupying
   * the timer should be freed if this is a
   * one-shot timer (i.e. #repeat == FALSE).
   */
  vtss_timer_cb_f *callback;

  /**
   * If more than one timer expires at the same time
   * the #prio field determines the order that the
   * callbacks are called.
   * This is not taken into account if #dsr_context is TRUE.
   *
   * RBNTBD: Currently not used.
   */
  vtss_timer_prio_t prio;

  /**
   * Set your module's ID here, or leave it as is if your
   * module doesn't have a module ID.
   * The module ID is used for providing per-module
   * statistics, and is mainly for debugging.
   */
  vtss_module_id_t modid;

  /**
   * User data. Not used by this module.
   */
  void *user_data;
};

/**
 * \brief Initialize a timer structure.
 *
 * Call this function prior to any of the others
 * to initialize the timer structure.
 *
 * The function must not be called from within
 * a timer-callback function if the timer is
 * repeating (unless it's a completely unrelated
 * timer).
 *
 * \param timer [INOUT] The timer to initialize.
 *
 * \return VTSS_RC_OK if the timer structure is
 * initializaed correctly, VTSS_RC_ERROR otherwise.
 */
vtss_rc vtss_timer_initialize(vtss_timer_t *timer);

/**
 * \brief Start a timer.
 *
 * Once started, the timer will run and callback
 * as described in the #timer structure.
 *
 * The function must not be called from within
 * a timer-callback function if the timer is
 * repeating (unless it's a completely unrelated
 * timer).
 *
 * \param timer [IN] The timer to start.
 *
 * \return VTSS_RC_OK if the timer is valid
 * and started correctly, VTSS_RC_ERROR otherwise.
 */
vtss_rc vtss_timer_start(vtss_timer_t *timer);

/**
 * \brief Cancel an on-going timer.
 *
 * Normally it only makes sense to cancel a repeating
 * timer, but once in a while it could happen that the state
 * of your module changes so that a one-shot timer needs
 * to be cancelled. In this case, the return value from
 * this function should be ignored, since the timer might
 * have fired or fire during the cancellation.
 *
 * The function may be called from within the callback function
 * in thread context if it's a repeating timer.
 *
 * \param timer [IN] The timer to cancel
 *
 * \return VTSS_RC_OK if the timer was successfully cancelled,
  * VTSS_RC_ERROR otherwise.
 */
vtss_rc vtss_timer_cancel(vtss_timer_t *timer);

/**
 * \brief Get a list of active timers.
 *
 * The function takes a snapshot of the current
 * active timers by VTSS_MALLOC()ing an area into which the
 * timers are copied and linked (using the #next_active
 * field).
 * The caller *MUST* call VTSS_FREE() once done with the list.
 *
 * \param timers [OUT] List of timers. NULL if no timers active.
 * \param total_timer_cnt [OUT] The total number of times that
 * vtss_timer_start() has been called successfully.
 *
 * \return VTSS_RC_OK if the timer list was successfully
 * VTSS_MALLOC()ed and populated, VTSS_RC_ERROR otherwise.
 */
vtss_rc vtss_timer_list(vtss_timer_t **timers, u64 *total_timer_cnt);

/**
 * \brief Module initialization function
 */
vtss_rc vtss_timer_init(vtss_init_data_t *data);

#endif /* _VTSS_TIMER_API_H_ */

