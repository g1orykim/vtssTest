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
 
 $Id$
 $Revision$

*/

#include "critd.h"

#ifdef VTSS_SW_OPTION_CLI
#include "cli_api.h"       /* For cli_print_thread_status() */
#endif

#ifdef VTSS_SW_OPTION_SYSUTIL
#include "sysutil_api.h"       /* For cli_print_thread_status() */
#endif

#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
#include "daylight_saving_api.h"
#endif

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_CRITD

// ===========================================================================
// Trace
// ---------------------------------------------------------------------------
#if (VTSS_TRACE_ENABLED)
/* Trace registration */
static vtss_trace_reg_t trace_reg =
{ 
    .module_id = VTSS_TRACE_MODULE_ID,
    .name      = "critd",
    .descr     = "Critd module"
};


#ifndef CRITD_DEFAULT_TRACE_LVL
#define CRITD_DEFAULT_TRACE_LVL VTSS_TRACE_LVL_ERROR
#endif

static vtss_trace_grp_t trace_grps[TRACE_GRP_CNT] =
{
    [VTSS_TRACE_GRP_DEFAULT] = { 
        .name      = "default",
        .descr     = "Default",
        .lvl       = CRITD_DEFAULT_TRACE_LVL,
        .timestamp = 1,
    },
};
#endif /* VTSS_TRACE_ENABLED */

// ###########################################################################
// Definitions and constants
// ---------------------------------------------------------------------------

#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)	((a) > (b) ? (b) : (a))

#if VTSS_TRACE_ENABLED
#define CRITD_ASSERT(expr, fmt, ...) { \
    if (!(expr)) { \
        T_EXPLICIT(crit_p->trace_module_id, trace_grp, VTSS_TRACE_LVL_ERROR, \
                   file, line, "ASSERTION FAILED"); \
        T_EXPLICIT(crit_p->trace_module_id, trace_grp, VTSS_TRACE_LVL_ERROR, \
                   file, line, fmt, ##__VA_ARGS__); \
        VTSS_ASSERT(expr); \
    } \
}
#else
#define CRITD_ASSERT(expr, fmt, ...) { \
    VTSS_ASSERT(expr); \
}
#endif

#define CRITD_THREAD_ID_NONE (0)

#define CRITD_T_COOKIE 0x0BADBABE

// ###########################################################################


// ###########################################################################
// Global variables
// ---------------------------------------------------------------------------

// Thread variables
static cyg_handle_t critd_thread_handle;
static cyg_thread   critd_thread_block;
static char         critd_thread_stack[THREAD_DEFAULT_STACK_SIZE];

// Linked list of critds for each module
static critd_t* critd_tbl[VTSS_MODULE_ID_NONE];

// Deadlock surveillance 
// Global variable is easier to inspect from gdb
static BOOL critd_deadlock = 0;

// ###########################################################################


// ###########################################################################
// Internal functions
// ---------------------------------------------------------------------------

// Current number of tokens
// The function returns 0 if the mutex or semaphore is taken, > 0 otherwise
static int critd_peek(critd_t* const crit_p) 
{

    // If we've not yet had the initial critd_exit(),
    // then "pretend" that we're locked.
    if (cyg_flag_peek(&crit_p->flag) == 0) {
        return 0;
    }

    if (crit_p->type == CRITD_TYPE_MUTEX) {
        // The initial critd_exit() has been called, so
        // now we can check to see if we can lock it (in the
        // lack of a peek function).
        if (cyg_mutex_trylock(&crit_p->u.mutex)) {
            // The mutex was not taken, but now it is. Undo that.
            cyg_mutex_unlock(&crit_p->u.mutex);
            return 1;
        }
        // The mutex is already taken.
        return 0;
    } else {
        int peek_cnt;
        cyg_semaphore_peek(&crit_p->u.semaphore, &peek_cnt);
        return peek_cnt;
    }
} // critd_peek

// Check whether critical region is locked
static BOOL critd_is_locked(
    critd_t* const crit_p)
{
    return (critd_peek(crit_p) == 0);
} // critd_is_locked


static void critd_trace(
    critd_t*    const crit_p,
    const int   trace_grp,
    const int   trace_lvl,
    const char* const file,
    const int   line)
{
    T_EXPLICIT(crit_p->trace_module_id, trace_grp, trace_lvl, file, line, 
               "Current state for critical region \"%s\":\n"
               "Locked=%d\n"
               "Latest lock: %u/%s#%d\n"
               "Latest unlock: %u/%s#%d",

               crit_p->name,
               critd_is_locked(crit_p),

               crit_p->lock_thread_id,
               crit_p->lock_file ? misc_filename(crit_p->lock_file) : crit_p->lock_file,
               crit_p->lock_line,

               crit_p->unlock_thread_id,
               crit_p->unlock_file ? misc_filename(crit_p->unlock_file) : crit_p->unlock_file,
               crit_p->unlock_line);
} // critd_trace


static inline ulong critd_get_thread_id(void)
{
    return (ulong) cyg_thread_get_id(cyg_thread_self());
} // critd_get_thread_id

// ===========================================================================
// Module thread - for semaphore surveillance
// ---------------------------------------------------------------------------

// Function used to have critd_dbg_list sprint output into deadlock_str buffer
#define DEADLOCK_STR_LEN 10000
static char* deadlock_str;
int deadlock_print(const char *fmt, ...)
{
    va_list args;
    int     rv;

    va_start(args, fmt);
    rv = vsprintf(deadlock_str + strlen(deadlock_str), fmt, args);
    va_end(args);
    
    return rv;
} // deadlock_print

#ifdef VTSS_SW_OPTION_CLI
static int thread_status_printf(const char *fmt, ...)
{
  va_list args;
  int     rv;
  // Function not API-public, but we need it in order to print
  // to all attached consoles and without using the T_E()
  // or other macros, because such macros would print stuff
  // for each call that would clutter the output.
  int trace_vprintf(char *err_buf, const char *fmt, va_list ap);
  
  va_start(args, fmt);
  rv = trace_vprintf(NULL, fmt, args);
  va_end(args);

  return rv;
} // thread_status_printf()
#endif //  VTSS_SW_OPTION_CLI

static void critd_thread(cyg_addrword_t data)
{
    critd_t*         critd_p;
    const uint       POLL_PERIOD = 5;
    vtss_module_id_t mid;
    BOOL             deadlock_found = 0;

    T_D("enter, data: %d", data);

    VTSS_OS_MSLEEP(10000); // Let everything start before starting surveillance

    // Semaphore surveillance
    while (1) {
        VTSS_OS_MSLEEP(POLL_PERIOD*1000);

        for (mid = 0; mid < VTSS_MODULE_ID_NONE; mid++) {
            critd_p = critd_tbl[mid];
            while (critd_p) { 
                // Check cookie
                if (critd_p->cookie != CRITD_T_COOKIE &&
                    critd_p->cookie_illegal != 1) {
                    T_E("%s: Cookie=0x%08x, expected 0x%08x",
                        vtss_module_names[mid], critd_p->cookie, CRITD_T_COOKIE);
                    critd_p->cookie_illegal = 1;
                }

                if (!critd_deadlock) {
                    // Check for deadlock
                    if (critd_p->current_lock_thread_id && critd_p->max_lock_time != -1) {
                        if (critd_p->lock_cnt == critd_p->last_lock_cnt) {
                            // Same lock as last poll!
                            critd_p->lock_poll_cnt++;
                        } else {
                            // Store lock count, so we can see if same lock persists
                            critd_p->last_lock_cnt = critd_p->lock_cnt;
                            critd_p->lock_poll_cnt      = 0;
                        }
                    } else {
                        critd_p->lock_poll_cnt  = 0;
                    }
                
                    if (critd_p->lock_poll_cnt > (critd_p->max_lock_time / POLL_PERIOD)) {
                        // Semaphore has been locked for too long
                        critd_deadlock = 1;
                        deadlock_found = 1;

                        if (!(deadlock_str = VTSS_MALLOC(DEADLOCK_STR_LEN))) {
                            T_E("VTSS_MALLOC(%d) failed", DEADLOCK_STR_LEN);
                        } else {
                            memset(deadlock_str, 0, DEADLOCK_STR_LEN);
                            critd_dbg_list(&deadlock_print,
                                           mid,
                                           1,
                                           0,
                                           critd_p);
                            T_E("Semaphore deadlock:\n%s", deadlock_str);
                        }
                    }
                }
            
                critd_p = critd_p->nxt;
            }
        }

        if (deadlock_found) {
            // Write the state of all locked semaphores to syslog and 
            // suspend any further surveillance

            if (deadlock_str) {
                T_E("All locked semaphores listed below.");
                for (mid = 0; mid < VTSS_MODULE_ID_NONE; mid++) {
                    critd_p = critd_tbl[mid];
                    while (critd_p) {
                        if (critd_p->current_lock_thread_id) {
                            memset(deadlock_str, 0, DEADLOCK_STR_LEN);
                            critd_dbg_list(&deadlock_print,
                                           mid,
                                           1,
                                           0,
                                           critd_p);
                            T_E("\n%s", deadlock_str);
                        }
                        critd_p = critd_p->nxt;
                    }
                }
#ifdef VTSS_SW_OPTION_CLI
                cli_print_thread_status(thread_status_printf, TRUE, FALSE); // Print thread status including stack trace. If CLI is deadlocked, then this is the only way to get that info.
#endif // VTSS_SW_OPTION_CLI
            }
            // Only report deadlock once
            deadlock_found = 0;
        }
    }
} // critd_thread


// ===========================================================================

// ---------------------------------------------------------------------------
// Internal functions
// ###########################################################################


// ###########################################################################
// API functions
// ---------------------------------------------------------------------------

// Initialize critd_t
void critd_init(
    critd_t*               const crit_p, 
    const char*            const name, 
    const vtss_module_id_t module_id,
    const int              trace_module_id,
    const critd_type_t     type)
{
    memset(crit_p, 0, sizeof(critd_t));
    
    crit_p->max_lock_time = CRITD_MAX_LOCK_TIME_DEFAULT;
    crit_p->cookie        = CRITD_T_COOKIE;
    crit_p->type          = type;

    // From the application point of view, the critd is created locked
    // to allow the application to do some initializations of the data
    // it protects before opening up for other modules to access the
    // data.
    // In reality, this is not possible with mutexes, so a flag is used
    // to indicate whether the critd has been exited the first time.
    // To make the implementation alike for both semaphores and mutexes,
    // the semaphore is created unlocked, and the flag is used to stop
    // access until the initial critd_exit() call.
    cyg_flag_init(&crit_p->flag);

    if (type == CRITD_TYPE_MUTEX) {
        // Always created unlocked.
        cyg_mutex_init(&crit_p->u.mutex);
    } else {
        // Create it unlocked. The @flag holds back accesses.
        cyg_semaphore_init(&crit_p->u.semaphore, 1);
    }

#if VTSS_TRACE_ENABLED || CRITD_CHK
    strncpy(crit_p->name, name, MIN(CRITD_NAME_LEN, strlen(name)));
    crit_p->trace_module_id          = trace_module_id;
    crit_p->current_lock_thread_id   = CRITD_THREAD_ID_NONE;
    crit_p->nxt                      = NULL;

    // Insert into linked list
    // Note that it is assumed that each module does not concurrently
    // create semaphores from different threads.
    if (critd_tbl[module_id]) {
        crit_p->nxt = critd_tbl[module_id];
    }
    critd_tbl[module_id] = crit_p;
#endif

#if VTSS_TRACE_ENABLED
    // Initially, the critd is taken, so update the time for the first lock.
    // and pretend the user knows where this function is called from.
    crit_p->lock_tick_cnt            = cyg_current_time();
    crit_p->max_lock_tick_cnt        = 0;
    crit_p->total_lock_tick_cnt      = 0;
    crit_p->max_lock_thread_id       = critd_get_thread_id();
#endif

    crit_p->init_done = 1;
} // critd_init


// Enter critical region
void critd_enter(
    critd_t*    const crit_p
#if VTSS_TRACE_ENABLED
    ,
    const int   trace_grp,
    const int   trace_lvl,
    const char* const file,
    const int   line
#endif
    )
{
    ulong my_thread_id = critd_get_thread_id();

#if VTSS_TRACE_ENABLED
    // Calling T_LVL_GET() is actually redundant, since both 
    // T_EXPLICIT() and critd_trace() does the same things, but
    // not calling T_LVL_GET() would cause it to be called two times
    // instead of just one when trace_lvl is not at an adequate level.
    // The drawback is that T_LVL_GET() will get called three times
    // when something is actually printed, but we live with that, since
    // that's the exception rather than the rule.
    if (T_LVL_GET(crit_p->trace_module_id, trace_grp) <= trace_lvl) {
        T_EXPLICIT(crit_p->trace_module_id, trace_grp, trace_lvl, file, line, "Locking critical region \"%s\"", crit_p->name);
        critd_trace(crit_p, trace_grp, trace_lvl, file, line);
    }
#endif

#if VTSS_TRACE_ENABLED || CRITD_CHK
    VTSS_ASSERT(crit_p->init_done);

    // Assert that this thread has not already locked this critical region.
    // This check is only possible for mutexes. For semaphores, it's indeed
    // quite possible that the same thread attempts to take the semaphore twice
    // before the unlock (typically called from another thread) occurs.
    if (crit_p->type                   == CRITD_TYPE_MUTEX     &&
        crit_p->current_lock_thread_id != CRITD_THREAD_ID_NONE &&
        crit_p->current_lock_thread_id == my_thread_id) {
        critd_trace(crit_p, trace_grp, VTSS_TRACE_LVL_ERROR, file, line);
        CRITD_ASSERT(0,  "Critical region already locked by this thread id!");
    }
#endif

#if VTSS_TRACE_ENABLED
    // Store information about lock attempt
    // This operation may go wrong due to concurrent access from multiple threads
    // However in many cases the information will be correct (and useful)
    {
        uint idx;
        ulong lock_cnt_new;
        cyg_scheduler_lock();
        lock_cnt_new = crit_p->lock_attempt_cnt;
        HAL_REORDER_BARRIER();
        // Do not reorder these lines.
        // Increases probability of concurrent accesses from multiple thread being
        // visible through comparison of lock_attempt_cnt and lock_cnt.
        lock_cnt_new++;
        idx = (lock_cnt_new % CRITD_LOCK_ATTEMPT_SIZE);
        crit_p->lock_attempt_thread_id[idx] = my_thread_id;
        crit_p->lock_attempt_line[idx]      = line;
        crit_p->lock_attempt_file[idx]      = file;
        HAL_REORDER_BARRIER();
        crit_p->lock_attempt_cnt            = lock_cnt_new;
        cyg_scheduler_unlock();
    }
#endif

    // Here is the trick that causes the locking of all
    // waiters until the very first critd_exit() call has
    // taken place.
    // The flag is initially 0, and will be set on the
    // first call to critd_exit(), and remain set ever after,
    // causing this call to be fast.
    cyg_flag_wait(&crit_p->flag, 1, CYG_FLAG_WAITMODE_OR);

    if (crit_p->type == CRITD_TYPE_MUTEX) {
        cyg_mutex_lock(&crit_p->u.mutex);
    } else {
        cyg_semaphore_wait(&crit_p->u.semaphore);
    }

#if VTSS_TRACE_ENABLED || CRITD_CHK
    // Store information about lock
    crit_p->current_lock_thread_id = my_thread_id;
    crit_p->lock_thread_id         = my_thread_id;
    crit_p->lock_file              = file;
    crit_p->lock_line              = line;
    crit_p->lock_time              = time(NULL);
    crit_p->lock_tick_cnt          = cyg_current_time();
    crit_p->lock_cnt++;
#endif

#if VTSS_TRACE_ENABLED
    if (T_LVL_GET(crit_p->trace_module_id, trace_grp) <= trace_lvl) {
        T_EXPLICIT(crit_p->trace_module_id, trace_grp, trace_lvl, file, line, "Locked critical region \"%s\"", crit_p->name);
        critd_trace(crit_p, trace_grp, trace_lvl, file, line);
    }
#endif
} // critd_enter

// Exit critical region
void critd_exit(
    critd_t*   const crit_p
#if VTSS_TRACE_ENABLED
    ,
    const int   trace_grp,
    const int   trace_lvl,
    const char* const file,
    const int   line
#endif
    )
{
#if VTSS_TRACE_ENABLED || CRITD_CHK
    ulong my_thread_id = critd_get_thread_id();
#endif

#if VTSS_TRACE_ENABLED
    if (T_LVL_GET(crit_p->trace_module_id, trace_grp) <= trace_lvl) {
        T_EXPLICIT(crit_p->trace_module_id, trace_grp, trace_lvl, file, line, "Unlocking critical region \"%s\"", crit_p->name);
        critd_trace(crit_p, trace_grp, trace_lvl, file, line);
    }
#endif

    if (cyg_flag_peek(&crit_p->flag) == 0) {
        // This is the very first call to critd_exit().
        // The call must open up for other threads waiting for the mutex.
        // This is accomplished by setting the event flag that others may
        // wait for in the critd_enter(). Once that is done, we simply return
        // because the mutex is not really taken, so we shouldn't unlock it.
        cyg_flag_setbits(&crit_p->flag, 1);
#if VTSS_TRACE_ENABLED
        // The very first time it's exited, we only update the time it took
        // and pretend that we know where it came from.
        crit_p->max_lock_file        = "critd_init";
        crit_p->max_lock_tick_cnt    = cyg_current_time() - crit_p->lock_tick_cnt;
        crit_p->total_lock_tick_cnt  = crit_p->max_lock_tick_cnt;
#endif
        goto do_exit; // Since the mutex was already unlocked, nothing more to do here, except some trace stuff.
    }

#if VTSS_TRACE_ENABLED || CRITD_CHK
    // Assert that critical region is currently locked
    if (!critd_is_locked(crit_p)) {
        critd_trace(crit_p, trace_grp, VTSS_TRACE_LVL_ERROR, file, line);
        CRITD_ASSERT(0, "Unlock called, but critical region is not locked!");
    }

    // If it's a mutex, the unlocking thread can never differ from the locking thread.
    // If it's a semaphore, this is indeed typically the case.
    if (crit_p->type == CRITD_TYPE_MUTEX) {
        // Assert that any current lock is this thread
        if (crit_p->lock_thread_id != my_thread_id) {
            critd_trace(crit_p, trace_grp, VTSS_TRACE_LVL_ERROR, file, line);
            CRITD_ASSERT(0, "Unlock called, but mutex is locked by different thread id!");
        }
    }
    
    // Clear current lock id
    crit_p->current_lock_thread_id = CRITD_THREAD_ID_NONE;

    // Store information about unlock
    crit_p->unlock_thread_id  = my_thread_id;
    crit_p->unlock_file       = file;
    crit_p->unlock_line       = line;
#endif
#if VTSS_TRACE_ENABLED
    {
        cyg_tick_count_t diff_ticks = cyg_current_time() - crit_p->lock_tick_cnt;
        crit_p->total_lock_tick_cnt += diff_ticks;
        if (diff_ticks > crit_p->max_lock_tick_cnt) {
            crit_p->max_lock_tick_cnt  = diff_ticks;
            crit_p->max_lock_file      = crit_p->lock_file;
            crit_p->max_lock_line      = crit_p->lock_line;
            crit_p->max_lock_thread_id = crit_p->lock_thread_id;
        }
    }
#endif

    if (crit_p->type == CRITD_TYPE_MUTEX) {
        cyg_mutex_unlock(&crit_p->u.mutex);
    } else {
        cyg_semaphore_post(&crit_p->u.semaphore);
    }
    
do_exit:

#if VTSS_TRACE_ENABLED
    if (T_LVL_GET(crit_p->trace_module_id, trace_grp) <= trace_lvl) {
        T_EXPLICIT(crit_p->trace_module_id, trace_grp, trace_lvl, file, line, "Unlocked critical region \"%s\"", crit_p->name);
    }
#endif
} // critd_exit


void critd_assert_locked(
    critd_t*   const crit_p
#if VTSS_TRACE_ENABLED
    ,
    const int   trace_grp,
    const char* const file,
    const int   line
#endif
    )
{
    if (critd_peek(crit_p) != 0) {
        critd_trace(crit_p, trace_grp, VTSS_TRACE_LVL_ERROR, file, line);
        CRITD_ASSERT(0, "Critical region not locked!");
    }
} // critd_assert_locked


// Debug functions
// ---------------
// To be used from CLI as well as to print a single critd instance in
// case of deadlock detection (in surveillance thread).
void critd_dbg_list(
    const critd_dbg_printf_t dbg_printf,
    const vtss_module_id_t   module_id,
    const BOOL               detailed,
    const BOOL               header,
    const critd_t*           single_critd_p)
{
#if VTSS_TRACE_ENABLED || CRITD_CHK
    int i;
    const uint MODULE_NAME_WID = 17;
    char module_name_format[10];
    char critd_name_format[10];
    vtss_module_id_t mid_start = 0;
    vtss_module_id_t mid_end = VTSS_MODULE_ID_NONE-1;
    vtss_module_id_t mid;
    const critd_t *critd_p;
    int crit_cnt = 0;
    char buf[40] = "";
    BOOL first = header;
    
    /* Work-around for problem with printf("%*s", ...) */
    sprintf(module_name_format, "%%-%d.%ds", MODULE_NAME_WID, MODULE_NAME_WID);
    sprintf(critd_name_format,  "%%-%d.%ds", CRITD_NAME_LEN, CRITD_NAME_LEN);

    if (module_id != VTSS_MODULE_ID_NONE) {
        mid_start = module_id;
        mid_end   = module_id;
    }

    for (mid = mid_start; mid <= mid_end; mid++) {
        if (single_critd_p) {
            critd_p = single_critd_p;
        } else {
            critd_p = critd_tbl[mid];
        }
        while (critd_p) {
            if (critd_p->cookie != CRITD_T_COOKIE) {
                T_E("Cookie=0x%08x, expected 0x%08x", critd_p->cookie, CRITD_T_COOKIE);
            }
            
            crit_cnt++;
            if (first) {
                if (!detailed)
                    first = 0;
                dbg_printf(module_name_format, "Module");
                dbg_printf(" ");
                dbg_printf(critd_name_format, "Critd Name");
                dbg_printf(" ");
                dbg_printf("T");
                dbg_printf(" ");
                dbg_printf("State   "); // "Unlocked" or "Locked"
                dbg_printf(" ");
                dbg_printf("Lock Cnt  ");
                dbg_printf(" ");
                dbg_printf("Latest Lock, Latest Unlock\n");
                for (i = 0; i < MODULE_NAME_WID; i++) dbg_printf("-");
                dbg_printf(" ");
                for (i = 0; i < CRITD_NAME_LEN; i++) dbg_printf("-");
                dbg_printf(" ");
                dbg_printf("- -------- ---------- ------------------------------\n");
            }
            
            dbg_printf(module_name_format, vtss_module_names[mid]);
            dbg_printf(" ");
            dbg_printf(critd_name_format, critd_p->name);
            dbg_printf(" ");
            dbg_printf("%c", critd_p->type == CRITD_TYPE_MUTEX ? 'M' : 'S');
            dbg_printf(" ");
            if (critd_p->current_lock_thread_id == CRITD_THREAD_ID_NONE) {
                dbg_printf("Unlocked");
            } else {
                dbg_printf("Locked  ");
            }
            dbg_printf(" ");

            // Lock cnt
            sprintf(buf, "0x%08x", critd_p->lock_cnt);
            dbg_printf("%10s ", buf);

            {
                // Print lock time
                struct tm *timeinfo_p;
                time_t lock_time;
                
                lock_time = critd_p->lock_time;
#ifdef VTSS_SW_OPTION_SYSUTIL
                lock_time += (system_get_tz_off() * 60); /* Adjust for TZ minutes => seconds */
#endif
#ifdef VTSS_SW_OPTION_DAYLIGHT_SAVING
                lock_time += (time_dst_get_offset() * 60); /* Correct for DST */
#endif
                timeinfo_p = localtime(&lock_time);
                
                dbg_printf("%02d:%02d:%02d ",
                           timeinfo_p->tm_hour,
                           timeinfo_p->tm_min,
                           timeinfo_p->tm_sec);
            }
            {
                // Protect against concurrent update of lock information
                char file_lock[100] = "";
                char file_unlock[100] = "";
                strncpy(file_lock,   critd_p->lock_file   ? misc_filename(critd_p->lock_file)   : "",   98);
                strncpy(file_unlock, critd_p->unlock_file ? misc_filename(critd_p->unlock_file) : "", 98);
                file_lock[99]   = 0;
                file_unlock[99] = 0;
                
                dbg_printf("%u/%s#%d, %u/%s#%d",
                           critd_p->lock_thread_id, 
                           file_lock,
                           critd_p->lock_line,
                           critd_p->unlock_thread_id, 
                           file_unlock,
                           critd_p->unlock_line);
            }

            if (detailed) {
                int i;
                char file_lock[100] = "";
                int j = 0;
                dbg_printf(", lock_attempt_cnt=0x%x", critd_p->lock_attempt_cnt);
                for (i = 0; i < CRITD_LOCK_ATTEMPT_SIZE; i++) {
                    if (j++ % 4 == 0) {
                        dbg_printf("\n  ");
                    }
                    strncpy(file_lock,   critd_p->lock_attempt_file[i] ? misc_filename(critd_p->lock_attempt_file[i])   : "",   98);
                    dbg_printf("[%x]=%u/%s#%d ", 
                               i,
                               critd_p->lock_attempt_thread_id[i],
                               file_lock,
                               critd_p->lock_attempt_line[i]);
                }
            }
            if (single_critd_p) {
                critd_p = NULL;
            } else {
                dbg_printf("\n");
                if (detailed) dbg_printf("\n");

                critd_p = critd_p->nxt;
            }
        }
    }
    if (module_id != VTSS_MODULE_ID_NONE && crit_cnt == 0) {
    } else if (detailed && header) {
        dbg_printf("Note that the logging of lock attempts could be incorrect, since it is not protected by any critical region.\n");
        dbg_printf("However in most cases the information will likely be correct.\n");
        dbg_printf("\n");
        dbg_printf("The last logged lock attempt is stored in entry [lock_attempt_cnt %% %d].\n",
                   CRITD_LOCK_ATTEMPT_SIZE);
    }
    

#else
    dbg_printf("Not supported.\n");
#endif

} // critd_dbg_list

// To be used from CLI
void critd_dbg_max_lock(
    const critd_dbg_printf_t dbg_printf,
    const vtss_module_id_t   module_id,
    const BOOL               header,
    const BOOL               clear)
{
#if VTSS_TRACE_ENABLED
    int i;
    const uint MODULE_NAME_WID = 17;
    char module_name_format[10];
    char critd_name_format[10];
    vtss_module_id_t mid_start = 0;
    vtss_module_id_t mid_end = VTSS_MODULE_ID_NONE-1;
    vtss_module_id_t mid;
    critd_t *critd_p;
    char file_lock[100] = "";
    BOOL first = header;
    
    /* Work-around for problem with printf("%*s", ...) */
    sprintf(module_name_format, "%%-%d.%ds", MODULE_NAME_WID, MODULE_NAME_WID);
    sprintf(critd_name_format,  "%%-%d.%ds", CRITD_NAME_LEN, CRITD_NAME_LEN);

    if (module_id != VTSS_MODULE_ID_NONE) {
        mid_start = module_id;
        mid_end   = module_id;
    }

    // This does not ensure atomic update, but is better than not doing it.
    cyg_scheduler_lock();
    
    for (mid = mid_start; mid <= mid_end; mid++) {
        critd_p = critd_tbl[mid];
        while (critd_p) {
            if (critd_p->cookie != CRITD_T_COOKIE) {
                T_E("Cookie=0x%08x, expected 0x%08x", critd_p->cookie, CRITD_T_COOKIE);
            }

            if (clear) {
                critd_p->max_lock_tick_cnt   = 0;
                critd_p->total_lock_tick_cnt = 0;
                critd_p->max_lock_file       = "";
                critd_p->max_lock_line       = 0;
                critd_p->max_lock_thread_id  = CRITD_THREAD_ID_NONE;
            } else {
                if (first) {
                    first = 0;
                    dbg_printf(module_name_format, "Module");
                    dbg_printf(" ");
                    dbg_printf(critd_name_format, "Critd Name");
                    dbg_printf(" T Max Lock [ms] Tot Lock [ms] Lock Position\n");
                    
                    for (i = 0; i < MODULE_NAME_WID; i++) dbg_printf("-");
                    dbg_printf(" ");
                    for (i = 0; i < CRITD_NAME_LEN; i++) dbg_printf("-");
                    dbg_printf(" ");
                    dbg_printf("- ------------- ------------- ------------------------------\n");
                }
                
                dbg_printf(module_name_format, vtss_module_names[mid]);
                dbg_printf(" ");
                dbg_printf(critd_name_format, critd_p->name);
                dbg_printf(" %c", critd_p->type == CRITD_TYPE_MUTEX ? 'M' : 'S');

                // Max. lock time
                dbg_printf(" %13llu", critd_p->max_lock_tick_cnt * ECOS_MSECS_PER_HWTICK);

                // Total lock time
                dbg_printf(" %13llu ", critd_p->total_lock_tick_cnt * ECOS_MSECS_PER_HWTICK);
 
                // Lock file and line number
                // Protect against concurrent update of lock information
                strncpy(file_lock, critd_p->max_lock_file ? misc_filename(critd_p->max_lock_file) : "", 98);
                file_lock[99]   = 0;
                dbg_printf("%u/%s#%d\n", critd_p->max_lock_thread_id, file_lock, critd_p->max_lock_line);
            }
            critd_p = critd_p->nxt;
        }
    }

    cyg_scheduler_unlock();
#else
    dbg_printf("Not supported.\n");
#endif
} // critd_dbg_time

vtss_rc critd_module_init(vtss_init_data_t *data)
{
    vtss_rc rc = VTSS_OK;

    if (data->cmd == INIT_CMD_INIT) {
        // Initialize and register trace ressources
        VTSS_TRACE_REG_INIT(&trace_reg, trace_grps, TRACE_GRP_CNT);
        VTSS_TRACE_REGISTER(&trace_reg);
    }

    switch (data->cmd) {
    case INIT_CMD_INIT:
        // Create thread
        cyg_thread_create(THREAD_DEFAULT_PRIO, 
                          critd_thread, 
                          0, 
                          "Critd", 
                          critd_thread_stack, 
                          sizeof(critd_thread_stack),
                          &critd_thread_handle,
                          &critd_thread_block);
        break;

    case INIT_CMD_START:
        /* Resume thread */
        cyg_thread_resume(critd_thread_handle);
        break;
        
    default:
        break;
    }

    return rc;
} // critd_module_init


// ---------------------------------------------------------------------------
// API functions
// ###########################################################################


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
