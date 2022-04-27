/*

 Vitesse Switch Software.

 #####ECOSGPLCOPYRIGHTBEGIN#####
 -------------------------------------------
 This file is part of eCos, the Embedded Configurable Operating System.
 Copyright (C) 1998-2012 Free Software Foundation, Inc.

 eCos is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 or (at your option) any later
 version.

 eCos is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License
 along with eCos; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 As a special exception, if other files instantiate templates or use
 macros or inline functions from this file, or you compile this file
 and link it with other works to produce a work based on this file,
 this file does not by itself cause the resulting work to be covered by
 the GNU General Public License. However the source code for this file
 must still be made available in accordance with section (3) of the GNU
 General Public License v2.

 This exception does not invalidate any other reasons why a work based
 on this file might be covered by the GNU General Public License.
 -------------------------------------------
 #####ECOSGPLCOPYRIGHTEND#####

*/
/*

 Vitesse Switch Software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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
#include <cyg/kernel/kapi.h>

#include <cyg/infra/diag.h>      // For diagnostic printing.
#include <cyg/hal/drv_api.h> /* For cyg_drv_interrupt_XXX() */
#include <cyg/infra/testcase.h>

#define NTHREADS 1
#define STACKSIZE 512

static cyg_handle_t thread[NTHREADS];
static cyg_thread thread_obj[NTHREADS];
static char stack[NTHREADS][STACKSIZE];

volatile cyg_uint32    ctr;
cyg_interrupt          timer_int_object;
cyg_handle_t           timer_int_handle;

cyg_vector_t irq;

cyg_uint32 timer_isr(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vector);            // Block this interrupt until the dsr completes
    cyg_drv_interrupt_acknowledge(vector);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}

void timer_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    ctr++;
    // Allow interrupts to happen again (we are using auto reload)
    cyg_drv_interrupt_unmask(irq);
}

void setup_timer(void)
{
    VTSS_ICPU_CFG_TIMERS_TIMER_TICK_DIV = 0x4E1; /* 0.01 ms */
    VTSS_ICPU_CFG_TIMERS_TIMER_VALUE(0) = 1;
    VTSS_ICPU_CFG_TIMERS_TIMER_RELOAD_VALUE(0) = 1;
    VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(0) = VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_TIMER_ENA;
}

void isr_thread( cyg_addrword_t data )
{
    ctr = 0;

    irq = CYGNUM_HAL_INTERRUPT_TIMER0;

    HAL_ENABLE_INTERRUPTS();

    CYG_TEST_INFO("Start ISR test");

    // Hook the TIMER0 interrupt
    cyg_drv_interrupt_create(
        irq,                    // Interrupt Vector
        0,                      // Interrupt Priority
        (cyg_addrword_t)NULL,
        timer_isr,
        timer_dsr,
        &timer_int_handle,
        &timer_int_object);
    
    cyg_drv_interrupt_attach(timer_int_handle);

    // Program a timed interrupt
    setup_timer();

    CYG_TEST_INFO("Inited, unmask");

    // Enable TIMER0 interrupts
    cyg_drv_interrupt_unmask(irq);

    CYG_TEST_INFO("Waiting...");

    // Wait for ints
    while(ctr < 3)
        cyg_thread_delay(1);

    cyg_uint32 _old;
    HAL_DISABLE_INTERRUPTS(_old);

    CYG_TEST_PASS_FINISH("Got at least 3 IRQs");
}

void cyg_start( void )
{
    CYG_TEST_INIT();
    cyg_thread_create(4, isr_thread , (cyg_addrword_t)0, "isr_timer0",
                      (void *)stack[0], STACKSIZE, &thread[0], &thread_obj[0]);
    cyg_thread_resume(thread[0]);
    cyg_scheduler_start();
}
