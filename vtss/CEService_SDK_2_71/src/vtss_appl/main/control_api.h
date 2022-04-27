/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _CONTROL_API_H_
#define _CONTROL_API_H_

#include <pkgconf/system.h>
#ifdef CYGPKG_CPULOAD
#include <cyg/cpuload/cpuload.h>
#endif /* CYGPKG_CPULOAD */
#include <cyg/io/flash.h>

#include "main_types.h" /* Types */
#include "vtss_init_api.h" /* For vtss_restart_t */

/* Control messages IDs */
typedef enum {
    CONTROL_MSG_ID_SYSTEM_REBOOT, /* Reboot system */
    CONTROL_MSG_ID_CONFIG_RESET,  /* Reset config (restore default) */
} control_msg_id_t;

typedef struct main_msg {
    control_msg_id_t msg_id;
    ulong            flags;
} control_msg_t;

/* Configuration reset */
void control_config_reset(vtss_usid_t usid, ulong flags);

// Convert the restart type to a string.
char *control_system_restart_to_str(vtss_restart_t restart);

/* System reset */
void control_system_reset_sync(vtss_restart_t restart)  __attribute__ ((noreturn));
void control_system_assert_do_reset(void)               __attribute__ ((noreturn));
/* Retrun 0: reset success, -1: reset fail (System is updating by another process) */
int control_system_reset(BOOL local, vtss_usid_t usid, vtss_restart_t restart);
void control_system_flash_lock(void);
BOOL control_system_flash_trylock(void);
void control_system_flash_unlock(void);
/* Retrun TRUE: mutex locked, FALSE: mutex unlocked */
BOOL control_system_flash_islock(void);

/* Register for reset early warning (1 second) */
typedef void (*control_system_reset_callback_t)(vtss_restart_t restart);
/*
 * NB: The callback should be NON-BLOCKING, and return asap.
 * Execution started on other threads is given 1 second grace to complete.
 */
void control_system_reset_register(control_system_reset_callback_t cb);

void 
dump_exception_data(int (*pr)(const char *fmt, ...), cyg_code_t exception_number, const HAL_SavedRegisters *regs, BOOL dump_threads);

// Overloaded functions that first acquire a flash semaphore.
// These functions have the same signatures, except for the initial
// "_printf *pf", which is passed to flash_init() in case the flash
// is not already initialized or the function has changed since
// the last call to control_flash_erase() or control_flash_program().
int control_flash_erase(cyg_flashaddr_t base, size_t len);
int control_flash_program(cyg_flashaddr_t flash_base, const void *ram_base, size_t len);
int control_flash_read(cyg_flashaddr_t flash_base, void *dest, size_t len);
void control_flash_get_info(cyg_flashaddr_t *start, size_t *size);

// Retrieve current system load
cyg_bool_t control_sys_get_cpuload(cyg_uint32 *average_point1s,
                                   cyg_uint32 *average_1s,
                                   cyg_uint32 *average_10s);

// Debug stuff
void control_dbg_latest_init_modules_get(vtss_init_data_t *data, char **init_module_func_name);

#endif // _CONTROL_API_H_

// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
