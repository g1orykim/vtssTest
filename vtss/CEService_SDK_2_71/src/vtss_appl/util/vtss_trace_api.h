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

/*
 * API for trace module
 */

/*
 * Introduction to trace module
 * ============================
 * The trace module provides macros for generating printf-trace to be output 
 * on - e.g. - a serial console. Each trace statement is categorized by:
 * - module
 * - group within module
 * - trace level
 *
 * For each (module, group) and each thread (eCos only), a trace level can be 
 * configured.
 * A given trace statement is only output if its trace level is 
 * - higher than the trace level configured for (module, group).
 * - higher than the trace level configured for the thread (eCos only)
 * - higher than the compile-time minimum trace level.
 *
 * The following trace levels are defined:
 * - ERROR   Code error encountered
 * - WARNING Potential code error, manual inspection required
 * - INFO    Useful information
 * - DEBUG   Some debug information
 * - NOISE   Lot's of debug information
 * - RACKET  Even more ...
 *
 * "RACKET" generates the most output, "ERROR" generates the least amount of 
 * output. By default, modules should have trace level set to ERROR for all 
 * groups.
 * 
 *
 * Registering a module with vtss_trace
 * ------------------------------------
 * In order for a module to use vtss_trace the following is required (here using
 * sw_sprout/vtss_sprout* as example):
 * - In module's header file (vtss_sprout.h) include <vtss_trace_lvl_api.h>
 * - In module's header file (vtss_sprout.h) define
 *    - Constant VTSS_TRACE_MODULE_ID 
 *      Usually the same as the module's vtss_module_id_t.
 *    - Constants for each of the module's trace group.
 *      Trace group 0, must be used for default group.
 * - In module's header file (vtss_sprout.h) include <vtss_trace_api.h>
 * 
 * - In module's .c file define 
 *    - trace registration (vtss_trace_reg_t)
 *    - trace groups (vtss_trace_grp_t)
 * 
 * - In module's initialization function
 *    - Call VTSS_TRACE_REG_INIT
 *    - Call VTSS_TRACE_REGISTER
 * 
 * Pls. refer to an existing module for detailed code example 
 * (e.g. vtss_sprout.h+vtss_sprout.c).
 * 
 * 
 * Controlling trace level at compile-time
 * ---------------------------------------
 * To control the trace level at compile time, VTSS_TRACE_LVL_MIN must be used.
 * To have all trace included, use the value VTSS_TRACE_LVL_ALL.
 * To have only error trace included, use the value VTSS_TRACE_LVL_ERROR.
 * To have all trace excluded, use the value VTSS_TRACE_LVL_NONE.
 * 
 * See vtss_trace_lvl_api.h for the exact numeric values of these constants.
 * 
 *
 * Controlling trace level at run-time
 * -----------------------------------
 * Functions are available for controlling the trace level at 
 * compile time. 
 * 
 * Additional each module provides can provide a default trace level in its
 * registration of trace groups. If no such default level is set, ERROR
 * is used.
 *
 *
 * Using trace in module code
 * --------------------------
 * Once a module has been registered with the vtss_trace, the T_... macroes 
 * below can be used.
 * The simplest type of trace is:
 *   T_D("bla bla");
 * This call will output "bla bla" if trace level for the module's default group
 * is DEBUG or lower.
 * If the trace shall be part of another group than the default group, then
 * macro T_DG must be used.
 * Macroes for hex-dumping (e.g. a packet) also exist. See below.
 *
 *
 * trace io
 * --------
 * The trace io functions are defined in vtss_trace_io.
 * By default trace is output with fputs/putchar on stdout.
 * Additional trace output devices can be registered, e.g. for Telnet 
 * connections.
 * 
 *
 * Dependencies
 * ============
 * vtss_trace requires the following modules:
 * - Switch API (gw_api/...)
 * 
 *
 * Compilation Directives
 * ======================
 * The following defines are used to control the compilation of vtss_trace:
 * + VTSS_TRACE_LVL_MIN
 *   The amount of trace included at compile time. By default all trace is 
 *   included. Must be set to one of the values defined in vtss_trace_lvl_api.h.
 *   It is recommended always to include error trace.
 * + VTSS_TRACE_MULTI_THREAD
 *   Must be set to 1, if trace macroes are called from multiple threads.
 * + VTSS_SWITCH
 *   Must be set to 1 when compiling SPROUT for Vitesse turnkey SW solution 
 *   (managed as well as unmanaged).
 *   If not defined, 0 is assumed.
 */

#ifndef _VTSS_TRACE_API_H_
#define _VTSS_TRACE_API_H_

#include <main.h>
#include <stdarg.h>


#ifndef _VTSS_TRACE_LVL_API_H_
#error "vtss_trace_lvl_api.h has not been included prior to including vtss_trace_api.h"
#endif

#ifndef VTSS_TRACE_LVL_MIN
#define VTSS_TRACE_LVL_MIN VTSS_TRACE_LVL_ALL   /* Include all trace by default */
#endif

/* 
 * Compile-time trace disabling 
 */
#ifndef VTSS_TRACE_LVL_MIN
#define VTSS_TRACE_LVL_MIN 0 /* Default: All trace enabled */
#endif

#ifdef VTSS_TRACE_ENABLED
#error "VTSS_TRACE_ENABLED defined outside vtss_trace_api.h. Not allowed. Use VTSS_TRACE_LVL_MIN instead."
#endif


#if (VTSS_TRACE_LVL_MIN < VTSS_TRACE_LVL_NONE)
#define VTSS_TRACE_ENABLED 1
#else
#define VTSS_TRACE_ENABLED 0
#endif

/* Default trace group */
#define VTSS_TRACE_GRP_DEFAULT 0

/* ===========================================================================
 * Trace registration
 * ------------------------------------------------------------------------ */

/* 
 * Before using the trace macroes, each module must register with vtss_trace.
 * This is done using the vtss_trace_reg_t and vtss_trace_grp_t structures 
 * and the macroes VTSS_TRACE_REG_INIT and VTSS_TRACE_REGISTER:
 * 1) Allocate vtss_trace_reg_t and vtss_trace_grp_t (not on the stack)
 *    Typically allocated statically.
 * 2) memset both structs to 0 (if allocated statically, this is 'C' default).
 * 3) Set the desired values in both structures.
 * 4) Call VTSS_TRACE_REG_INIT.
 * 5) Call VTSS_TRACE_REGISTER.
 * 
 * The module may now call the trace macroes (T_...).
 */

/* 
 * Note:
 * First call to VTSS_TRACE_REG_INIT() must be done in single-thread context,
 * since this call is used for internal initialization within the trace module.
 * After the initial call to VTSS_TRACE_REG_INIT(), the macro becomes 
 * reentrant and any following calls to VTSS_TRACE_REG_INIT() may thus be done 
 * in multi-thread context (provided trace has been compiled with 
 * VTSS_TRACE_MULTI_THREAD=1).
 */

#define VTSS_TRACE_MAX_NAME_LEN  10
#define VTSS_TRACE_MAX_DESCR_LEN 60

/* Group definition */
typedef struct {
    uint cookie;                            /* VTSS_TRACE_GRP_T_COOKIE */

    char name[VTSS_TRACE_MAX_NAME_LEN+1];   /* Name of group        */
    char descr[VTSS_TRACE_MAX_DESCR_LEN+1]; /* Description of group */

    int  lvl;       /* Default trace level. If 0, ERROR is used          */
    BOOL timestamp; /* Include wall clock time stamp in trace            */
    BOOL usec;      /* Include usec time stamp in trace                  */
    BOOL ringbuf;   /* Output trace into ring buffer, instead of console */
    BOOL irq;       /* Supposedly called from an interrupt handler, or
                       at least with interrupts and/or scheduler disabled.
                       Do not insert waiting points. Trace is *NOT* output
                       unless .ringbuf == TRUE. If .timestamp == TRUE,
                       it will be inserted as a usec time stamp, which
                       is not supported on all platforms. Thread info
                       will not be printed. */

    /* ----- Internal fields, not to be changed by application ----- */
    int  lvl_prv;   /* Trace level prior to previous call to 
                       vtss_trace_module_lvl_set */
} vtss_trace_grp_t;

/* Module registration */
/* See registration procedure above */
typedef struct {
    uint cookie;                            /* VTSS_TRACE_REG_T_COOKIE */

    int  module_id;                         /* Module ID             */
    char name[VTSS_TRACE_MAX_NAME_LEN+1];   /* Name of module        */
    char descr[VTSS_TRACE_MAX_DESCR_LEN+1]; /* Description of module */
  
    /* ----- Internal fields, not to be changed by application ----- */
    /* Pointer to array of trace groups. */
    int               grp_cnt;
    vtss_trace_grp_t* grps;
} vtss_trace_reg_t;


/* Registration macroes */
#if (VTSS_TRACE_LVL_MIN < VTSS_TRACE_LVL_NONE)
#define VTSS_TRACE_REG_INIT(trace_reg, trace_grps, grp_cnt) vtss_trace_reg_init(trace_reg, trace_grps, grp_cnt)
#define VTSS_TRACE_REGISTER(trace_reg)                      vtss_trace_register(trace_reg)
#else
#define VTSS_TRACE_REG_INIT(trace_reg, trace_grps, grp_cnt)
#define VTSS_TRACE_REGISTER(trace_reg)
#endif

/* ======================================================================== */


/* ===========================================================================
 * Trace macroes
 *
 * T_E/T_W/T_I/T_D/T_N/T_R:
 * Printf-style trace output for default trace group.
 * 
 * T_E_HEX/T_W_HEX/T_I_HEX/T_D_HEX/T_N_HEX/T_R_HEX:
 * Hex dump of any number of bytes, e.g. a packet, for default trace group.
 * 
 * T_EG/T_WG/T_IG/T_DG/T_NG/T_RG:
 * Printf-style trace output for specific trace group.
 * 
 * T_EG_HEX/T_WG_HEX/T_IG_HEX/T_DG_HEX/T_NG_HEX/T_RG_HEX:
 * Hex dump of any number of bytes, e.g. a packet, for specific trace group.
 * ------------------------------------------------------------------------ */


#define __VTSS_LOCATION__	__FUNCTION__

#ifdef __GNUC__
#define _trace_likely(x)   __builtin_expect((x),1)
#define _trace_unlikely(x) __builtin_expect((x),0)
#else
#define _trace_likely(x)   (x)
#define _trace_unlikely(x) (x)
#endif

#define TRACE_IS_ENABLED(_m, _grp, _lvl) _trace_unlikely(_lvl >= T_LVL_GET(_m, _grp))

/* Main macros */
#define T(_grp, _lvl, _fmt, ...)                                        \
    do {                                                                \
        if(TRACE_IS_ENABLED(VTSS_TRACE_MODULE_ID, _grp, _lvl))          \
            vtss_trace_printf(VTSS_TRACE_MODULE_ID, _grp, _lvl,         \
                              __VTSS_LOCATION__, __LINE__,              \
                              _fmt, ##__VA_ARGS__);                     \
    } while(0)

#define T_HEX(_grp, _lvl, _byte, _cnt)                                  \
    do {                                                                \
        if(TRACE_IS_ENABLED(VTSS_TRACE_MODULE_ID, _grp, _lvl))          \
            vtss_trace_hex_dump(VTSS_TRACE_MODULE_ID, _grp, _lvl,       \
                                __VTSS_LOCATION__, __LINE__,            \
                                _byte, _cnt);                           \
    } while(0)

// Macro for only printing out trace for ports that has trace enabled
#define T_PORT(_p, _grp, _lvl, _fmt, ...)                               \
    do {                                                                \
        if(TRACE_IS_ENABLED(VTSS_TRACE_MODULE_ID, _grp, _lvl) && !vtss_trace_port_get(_p)) {   \
            char str[150];                                              \
            int port_len = snprintf(str, sizeof(str), "Port %u: ", _p + 1);  \
            strncpy(str + port_len, _fmt, sizeof(str) - port_len);                \
            vtss_trace_printf(VTSS_TRACE_MODULE_ID, _grp, _lvl,         \
                              __VTSS_LOCATION__, __LINE__,              \
                              str, ##__VA_ARGS__);                      \
        }                                                               \
    } while(0)

/* ERROR level trace macroes */
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_ERROR)
#define T_E(_fmt, ...)                    T_EG(VTSS_TRACE_GRP_DEFAULT, _fmt, ##__VA_ARGS__)
#define T_E_PORT(_port, _fmt, ...)        T_EG_PORT(VTSS_TRACE_GRP_DEFAULT, _port, _fmt, ##__VA_ARGS__)
#define T_EG(_grp, _fmt, ...)             T(_grp, VTSS_TRACE_LVL_ERROR, _fmt, ##__VA_ARGS__)
#define T_EG_PORT(_grp, _port, _fmt, ...) T_PORT(_port, _grp, VTSS_TRACE_LVL_ERROR, _fmt, ##__VA_ARGS__)
#define T_E_HEX(byte_p, byte_cnt)         T_EG_HEX(VTSS_TRACE_GRP_DEFAULT, byte_p, byte_cnt)
#define T_EG_HEX(_grp, byte_p, byte_cnt)  T_HEX(_grp, VTSS_TRACE_LVL_ERROR, byte_p, byte_cnt)
#else
#define T_E(_fmt, ...)
#define T_E_PORT(_port, _fmt, ...)
#define T_EG(_grp, _fmt, ...)
#define T_WG_PORT(_grp, _fmt, ...)
#define T_E_HEX(byte_p, byte_cnt)           
#define T_EG_HEX(_grp, byte_p, byte_cnt) 
#endif

/* WARNING level trace macroes */
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_WARNING)
#define T_W(_fmt, ...)                    T_WG(VTSS_TRACE_GRP_DEFAULT, _fmt, ##__VA_ARGS__)
#define T_WG(_grp, _fmt, ...)             T(_grp, VTSS_TRACE_LVL_WARNING, _fmt, ##__VA_ARGS__)
#define T_WG_PORT(_grp, _port, _fmt, ...) T_PORT(_port, _grp, VTSS_TRACE_LVL_WARNING, _fmt, ##__VA_ARGS__)
#define T_W_HEX(byte_p, byte_cnt)         T_WG_HEX(VTSS_TRACE_GRP_DEFAULT, byte_p, byte_cnt)
#define T_WG_HEX(_grp, byte_p, byte_cnt)  T_HEX(_grp, VTSS_TRACE_LVL_WARNING, byte_p, byte_cnt)
#else
#define T_W(_fmt, ...)
#define T_WG(_grp, _fmt, ...)
#define T_WG_PORT(_grp, _fmt, ...)
#define T_W_HEX(byte_p, byte_cnt)
#define T_WG_HEX(_grp, byte_p, byte_cnt)
#endif

/* INFO level trace macroes */
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_INFO)
#define T_I(_fmt, ...)                    T_IG(VTSS_TRACE_GRP_DEFAULT, _fmt, ##__VA_ARGS__)
#define T_I_PORT(_port, _fmt, ...)        T_IG_PORT(VTSS_TRACE_GRP_DEFAULT, _port, _fmt, ##__VA_ARGS__)
#define T_IG(_grp, _fmt, ...)             T(_grp, VTSS_TRACE_LVL_INFO, _fmt, ##__VA_ARGS__)
#define T_IG_PORT(_grp, _port, _fmt, ...) T_PORT(_port, _grp, VTSS_TRACE_LVL_INFO, _fmt, ##__VA_ARGS__)
#define T_I_HEX(byte_p, byte_cnt)         T_IG_HEX(VTSS_TRACE_GRP_DEFAULT, byte_p, byte_cnt)
#define T_IG_HEX(_grp, byte_p, byte_cnt)  T_HEX(_grp, VTSS_TRACE_LVL_INFO, byte_p, byte_cnt)
#else
#define T_I(_fmt, ...)
#define T_IG(_grp, _fmt, ...)
#define T_I_HEX(byte_p, byte_cnt)
#define T_IG_HEX(_grp, byte_p, byte_cnt)
#endif

/* DEBUG level trace macroes */
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_DEBUG)
#define T_D(_fmt, ...)                    T_DG(VTSS_TRACE_GRP_DEFAULT, _fmt, ##__VA_ARGS__)
#define T_D_PORT(_port, _fmt, ...)        T_DG_PORT(VTSS_TRACE_GRP_DEFAULT, _port, _fmt, ##__VA_ARGS__)
#define T_DG(_grp, _fmt, ...)             T(_grp, VTSS_TRACE_LVL_DEBUG, _fmt, ##__VA_ARGS__)
#define T_DG_PORT(_grp, _port, _fmt, ...) T_PORT(_port, _grp, VTSS_TRACE_LVL_DEBUG, _fmt, ##__VA_ARGS__)
#define T_D_HEX(byte_p, byte_cnt)         T_DG_HEX(VTSS_TRACE_GRP_DEFAULT, byte_p, byte_cnt)
#define T_DG_HEX(_grp, byte_p, byte_cnt)  T_HEX(_grp, VTSS_TRACE_LVL_DEBUG, byte_p, byte_cnt)
#else
#define T_D(_fmt, ...)
#define T_D_PORT(_port, _fmt, ...)
#define T_DG(_grp, _fmt, ...)
#define T_DG_PORT(_port, _grp, _fmt, ...)
#define T_D_HEX(byte_p, byte_cnt)
#define T_DG_HEX(_grp, byte_p, byte_cnt)
#endif

/* NOISE level trace macroes */
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_NOISE)
#define T_N(_fmt, ...)                    T_NG(VTSS_TRACE_GRP_DEFAULT, _fmt, ##__VA_ARGS__)
#define T_N_PORT(_port, _fmt, ...)        T_NG_PORT(VTSS_TRACE_GRP_DEFAULT, _port, _fmt, ##__VA_ARGS__)
#define T_NG(_grp, _fmt, ...)             T(_grp, VTSS_TRACE_LVL_NOISE, _fmt, ##__VA_ARGS__)
#define T_NG_PORT(_grp, _port, _fmt, ...) T_PORT(_port, _grp, VTSS_TRACE_LVL_NOISE, _fmt, ##__VA_ARGS__)
#define T_N_HEX(byte_p, byte_cnt)         T_NG_HEX(VTSS_TRACE_GRP_DEFAULT, byte_p, byte_cnt)
#define T_NG_HEX(_grp, byte_p, byte_cnt)  T_HEX(_grp, VTSS_TRACE_LVL_NOISE, byte_p, byte_cnt)
#else
#define T_N(_fmt, ...)
#define T_N_PORT(_port, _fmt, ...)
#define T_NG(_grp, _fmt, ...)
#define T_NG_PORT(_grp, _port, _fmt, ...)
#define T_N_HEX(byte_p, byte_cnt)
#define T_NG_HEX(_grp, byte_p, byte_cnt)
#endif

/* RACKET level trace macroes */
#if (VTSS_TRACE_LVL_MIN <= VTSS_TRACE_LVL_RACKET)
#define T_R(_fmt, ...)                    T_RG(VTSS_TRACE_GRP_DEFAULT, _fmt, ##__VA_ARGS__)
#define T_R_PORT(_port, _fmt, ...)        T_RG_PORT(VTSS_TRACE_GRP_DEFAULT, _port, _fmt, ##__VA_ARGS__)
#define T_RG(_grp, _fmt, ...)             T(_grp, VTSS_TRACE_LVL_RACKET, _fmt, ##__VA_ARGS__)
#define T_RG_PORT(_grp, _port, _fmt, ...) T_PORT(_port, _grp, VTSS_TRACE_LVL_RACKET, _fmt, ##__VA_ARGS__)
#define T_R_HEX(byte_p, byte_cnt)         T_RG_HEX(VTSS_TRACE_GRP_DEFAULT, byte_p, byte_cnt)
#define T_RG_HEX(_grp, byte_p, byte_cnt)  T_HEX(_grp, VTSS_TRACE_LVL_RACKET, byte_p, byte_cnt)
#else
#define T_R(_fmt, ...)
#define T_R_PORT(_port, _fmt, ...)
#define T_RG(_grp, _fmt, ...)
#define T_RG_PORT(_grp, _port, _fmt, ...)  
#define T_R_HEX(byte_p, byte_cnt)
#define T_RG_HEX(_grp, byte_p, byte_cnt)
#endif

/* Macro for checking whether trace is enabled for (module, grp, lvl) */
#if (VTSS_TRACE_LVL_MIN < VTSS_TRACE_LVL_NONE)
#define T_LVL_GET(module_id, grp_idx) (vtss_trace_global_module_lvl_get(module_id, grp_idx))
#else
#define T_LVL_GET(module_id, grp_idx) (VTSS_TRACE_LVL_NONE)
#endif

/* Trace macroes with module, group and level as argument */
#if (VTSS_TRACE_LVL_MIN < VTSS_TRACE_LVL_NONE)
#define T_EXPLICIT(_m, _grp, _lvl, _f, _l, _fmt, ...)                   \
    do {                                                                \
        if(TRACE_IS_ENABLED(_m, _grp, _lvl))                            \
            vtss_trace_printf(_m, _grp, _lvl, _f, _l,                   \
                              _fmt, ##__VA_ARGS__);                     \
    } while(0)
#define T_HEX_EXPLICIT(_m, _grp, _lvl, _f, _l, _p, _c)                  \
    do {                                                                \
        if(TRACE_IS_ENABLED(_m, _grp, _lvl))                            \
            vtss_trace_hex_dump(_m, _grp, _lvl, _f, _l, _p, _c);        \
    } while(0)
#else
#define T_EXPLICIT(_m, _grp, _lvl, _f, _l, _fmt, ...)  /* Go Away */
#define T_HEX_EXPLICIT(_m, _grp, _lvl, _f, _l, _p, _c) /* Go Away */
#endif

/* ======================================================================== */


/* ===========================================================================
 * Run-time trace level control
 *
 * These functions can be used to implement CLI-based trace control
 * ------------------------------------------------------------------------ */

/*
 * Set trace level for module/group
 * 
 * module_id/grp_idx may be set to -1 for wildcarding.
 */
void vtss_trace_module_lvl_set( int module_id, int grp_idx, int lvl);


/*
 * Limit trace for thread. -1 = all threads. eCos only.
 */
void vtss_trace_thread_lvl_set(int thread_id, int lvl);


/* 
 * Reverse the lastest trace level change
 */
void vtss_trace_lvl_reverse(void);


/*
 * Set/get trace level for all modules/threads
 */ 
void vtss_trace_global_lvl_set(int lvl);
int  vtss_trace_global_lvl_get(void);


/* ======================================================================== */


/* ===========================================================================
 * Run-time trace format control
 *
 * These functions can be used to implement CLI-based trace control
 * ------------------------------------------------------------------------ */

/*
 * Enable timestamp/usec for module/group. eCos only.
 */
typedef enum {VTSS_TRACE_MODULE_PARM_TIMESTAMP,
              VTSS_TRACE_MODULE_PARM_USEC,
              VTSS_TRACE_MODULE_PARM_RINGBUF,
              VTSS_TRACE_MODULE_PARM_IRQ /* R/O */
} vtss_trace_module_parm_t;
void vtss_trace_module_parm_set(
    vtss_trace_module_parm_t parm,
    int module_id, 
    int grp_idx, 
    BOOL enable);

/*
 * Include/exclude stack size in trace for thread. -1 = all threads.
 * eCos only.
 */
void vtss_trace_thread_stackuse_set(int thread_id, BOOL enable);

/* ======================================================================== */


/* ===========================================================================
 * Utility functions
 * ------------------------------------------------------------------------ */

void vtss_trace_port_set(int port_index,BOOL trace_disabled);
BOOL vtss_trace_port_get(int port_index);

/* Trace settings for thread */
typedef struct {
    /* Limit trace for thread (default: all trace enabled) */
    int  lvl;

    /* Enable stack size output for thread's trace (default: disabled) */
    BOOL stackuse;

    /* ----- Internal fields, not to be changed by application ----- */
    int  lvl_prv;   /* Trace level prior to previous call to 
                       vtss_trace_thread_lvl_set */
} vtss_trace_thread_t;

/*
 * Get trace settings for thread. eCos only.
 */
void vtss_trace_thread_get(int thread_id, vtss_trace_thread_t *trace_thread);

/*
 * Get trace level for (module, group)
 */
int vtss_trace_module_lvl_get(int module_id, int grp_idx);

/*
 * Get trace level for (global, module, group)
 */
int vtss_trace_global_module_lvl_get(int module_id, int grp_idx);

/* 
 * Get name of next module or group with id/idx greater than the specified 
 * value.
 * 
 * NULL pointer is returned when no more modules/groups.
 * To get all names, start by requesting id/idx -1 and keep requesting until 
 * NULL is returned.
 * *module_id_p / *grp_idx_p is updated to correspond to the returned name.
 */  
char *vtss_trace_module_name_get_nxt(int *module_id_p);
char *vtss_trace_grp_name_get_nxt(int module_id, int *grp_idx_p);

/* Get module/group description */
char *vtss_trace_module_get_descr(int module_id);
char *vtss_trace_grp_get_descr(int module_id, int grp_idx);

/* Get parm setting for group */  
BOOL vtss_trace_grp_get_parm(
    vtss_trace_module_parm_t parm,
    int module_id, int grp_idx);

/*
 * Convert module/group name to module_id/grp_idx
 * Name only needs to be unique, not complete.
 *
 * Returns 1 if id/idx was found. 
 */
BOOL vtss_trace_module_to_val(const char *name, int *module_id_p);
BOOL vtss_trace_grp_to_val(   const char *name, int  module_id, int *grp_idx_p);

const char *vtss_trace_lvl_to_str(int lvl);
/* ======================================================================== */


/* ===========================================================================
 * Ring buffer
 * ------------------------------------------------------------------------ */

void vtss_trace_rb_output(void);
void vtss_trace_rb_flush(void);
void vtss_trace_rb_start(void);
void vtss_trace_rb_stop(void);

/* 
 * Enable/disable ring buffer (enabled at startup)
 * If disabled, any trace destined for ring buffer is discarded.
 */
void vtss_trace_rb_ena(BOOL ena);

/* ======================================================================== */


/* ===========================================================================
 * Additional IO
 *
 * By default serial port is the only output device for trace macroes.
 * Additional IO devices can be registered using below functions.
 * Such IO devices could - e.g. - be telnet connections.
 * ------------------------------------------------------------------------ */

/* IO layer */
typedef struct _vtss_trace_io_t {
    void (*trace_putchar)      (struct _vtss_trace_io_t *pIO, char ch);
    int  (*trace_vprintf)      (struct _vtss_trace_io_t *pIO, const char *fmt, va_list ap);
    void (*trace_write_string) (struct _vtss_trace_io_t *pIO, const char *str);
    void (*trace_flush)        (struct _vtss_trace_io_t *pIO);
} vtss_trace_io_t;

/* 
 * Registration function. 
 * trace_io_t must NOT be allocated on stack.
 * 
 * io_reg_id is returned. 
 * To be used if later calling vtss_trace_io_unregister().
 */
vtss_rc vtss_trace_io_register(
    vtss_trace_io_t   *io,
    vtss_module_id_t  module_id,
    uint              *io_reg_id);

/* 
 * Unregistration function
 */
vtss_rc vtss_trace_io_unregister(
    uint *io_reg_id);

/* ======================================================================== */


/* ===========================================================================
 * Internal definitions - do NOT use outside trace module
 * ------------------------------------------------------------------------ */

/* Do not use these functions - use the wrappers above */
void vtss_trace_reg_init(vtss_trace_reg_t* trace_reg_p, vtss_trace_grp_t* trace_grp_p, int grp_cnt);
vtss_rc vtss_trace_register(vtss_trace_reg_t* trace_reg_p);
void vtss_trace_printf(int module_id, 
                       int grp_idx,
                       int lvl,
                       const char *location,
                       uint line_no,
                       const char *fmt,
                       ...)
__attribute__ ((format (printf, 6, 7)));

void vtss_trace_hex_dump(int module_id, 
                         int grp_idx,
                         int lvl,
                         const char *location,
                         uint line_no,
                         const uchar *byte_p,
                         int  byte_cnt);

/* ======================================================================== */


/* ===========================================================================
 * Special macroes, only intended for use in Switch API
 * ------------------------------------------------------------------------ */
#if (VTSS_TRACE_ENABLED)
#define T_EXPLICIT_NO_VA(_m, _grp, _lvl, _f, _l, _msg)                  \
    do {                                                                \
        if(TRACE_IS_ENABLED(_m, _grp, _lvl))                            \
            vtss_trace_printf(_m, _grp, _lvl, _f, _l,                   \
                              "%s", _msg);                              \
    } while(0)
#else
#define T_EXPLICIT_NO_VA(_m, _grp, _lvl, _f, _l, _msg) /* Go Away */
#endif /* (VTSS_TRACE_LVL_MIN < VTSS_TRACE_LVL_NONE) */
/* ======================================================================== */


#endif /* _VTSS_TRACE_API_H_ */


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
