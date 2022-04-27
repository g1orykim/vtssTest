//==========================================================================
//
//      hw_dep_tests.c
//
//      HAL support for H/W dependent Power-On-Self-Test for VCOREII
//      NOTE: THIS IS AN EXAMPLE FILE ONLY. IT IS ONLY INCLUDED IN
//            REDBOOT IF CYGBLD_BUILD_POST_HW_DEP IS DEFINED.
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Rene Schipp von Branitz Nielsen
// Contributors:
// Date:         2009-04-16
// Purpose:      Allow for H/W dependent tests.
// Description:  Implementations of H/W dependent HAL POST.
//
//####DESCRIPTIONEND####

#include <redboot.h>
#include "vcoreii_diag.h"
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/vcoreii.h>

// This file must support a range of functions. They may all be empty, but may also
// be used to blink with LEDs, beep a speaker, or whatever your H/W supports.
// There is no restrictions on the amount of time you may spend in each function.

// In this example, we use GPIO 13 and 15 to control the color of a RED/GREEN LED.
// This configuration is supported by the Vitesse Reference Board.
#define LED_FRONT_LED_GPIO_0 13
#define LED_FRONT_LED_GPIO_1 15
typedef enum {
  lc_NULL,
  lc_OFF,
  lc_GREEN,
  lc_RED
} LED_colors_t;

#define SYS_REG(reg) (*((volatile unsigned long *)(VCOREII_SWC_REG(7, 0, (reg)))))

/******************************************************************************/
// LED_init()
// Initialize the LED that we use to signal things to the surrounding world.
/******************************************************************************/
static void LED_init(void)
{
  cyg_uint32 cur_reg_val;

  // Set the front LED GPIOs to be outputs.
  cur_reg_val = SYS_REG(0x34);
  cur_reg_val |= VTSS_BIT((16 + LED_FRONT_LED_GPIO_0)) | VTSS_BIT((16 + LED_FRONT_LED_GPIO_1));
  SYS_REG(0x34) = cur_reg_val;

  // Set the control register to gate GPIOs.
  cur_reg_val = SYS_REG(0x33);
  cur_reg_val |= VTSS_BIT((LED_FRONT_LED_GPIO_0)) | VTSS_BIT((LED_FRONT_LED_GPIO_1));
  SYS_REG(0x33) = cur_reg_val;
}

/******************************************************************************/
// LED_set()
// Set the LED to a given color.
/******************************************************************************/
static void LED_set(LED_colors_t color)
{
  cyg_uint8 gpio_0, gpio_1;
  cyg_uint32 cur_gpio, mask;
  
  switch(color) {
    case lc_GREEN:
      gpio_0 = 0;
      gpio_1 = 1;
      break;
      
  case lc_RED:
      gpio_0 = 1;
      gpio_1 = 0;
      break;

  default:
      gpio_0 = 0;
      gpio_1 = 0;
      break;
  }

  cur_gpio      = SYS_REG(0x34);
  mask          = VTSS_BIT((LED_FRONT_LED_GPIO_0)) | VTSS_BIT((LED_FRONT_LED_GPIO_1));
  cur_gpio     &= ~mask; // Clear bits.
  cur_gpio     |= (gpio_0 << (LED_FRONT_LED_GPIO_0)) | (gpio_1 << (LED_FRONT_LED_GPIO_1)); // Set bits
  SYS_REG(0x34) = cur_gpio;
}

/******************************************************************************/
// LED_toggle()
// Toggle the LED between two colors.
/******************************************************************************/
static void LED_toggle(LED_colors_t col_on, LED_colors_t col_off)
{
  static int led = 0;
  if(led) {
    LED_set(col_on);
    led = 0;
  } else {
    LED_set(col_off);
    led = 1;
  }
}

/******************************************************************************/
// HERE STARTS THE FUNCTIONS THAT YOU MUST IMPLEMENT IF THIS CDL OPTION:
//   CYGBLD_BUILD_POST_HW_DEP
// IS DEFINED AND NON-ZERO. THEY MAY ALL BE EMPTY.
/******************************************************************************/

/******************************************************************************/
// vcoreii_diag_hw_dep_begin()
// Called once from vcoreii_diag.c. This function can be used to
// do various initialization (e.g. initialize LED control).
/******************************************************************************/
void vcoreii_diag_hw_dep_begin(bool show_progress)
{
  // In this example we initialize the H/W that controls the LED.
  LED_init();

  // Also, light the LED for a short while to indicate we just booted.
  LED_set(lc_GREEN);
  CYGACC_CALL_IF_DELAY_US((cyg_int32)100000);
  LED_set(lc_OFF);
}

/******************************************************************************/
// vcore_diag_hw_dep_end()
// Called once from vcoreii_diag.c whether or not the subtests passed.
// The err_info parameter's failing_test member will contain
// VCOREII_DIAG_SUBTEST_NONE if all tests passed, otherwise the enumeration of
// the failing test. The info1, info2, and info3 members of err_info will contain
// subtest-specific information about how the test failed. See vcoreii_diag.h
// for the subtest-specific infoX fields.
/******************************************************************************/
void vcoreii_diag_hw_dep_end(bool show_progress, vcoreii_diag_err_info_t *err_info)
{
  if(err_info->failing_test != VCOREII_DIAG_SUBTEST_NONE) {
    LED_set(lc_RED);
    diag_printf("\n***HALTING***\n");
    while(1);
    /* ENOTREACHED */
  }
}

/******************************************************************************/
// vcoreii_diag_hw_dep_subtest_begin()
// Called whenever a subtest begins.
/******************************************************************************/
void vcoreii_diag_hw_dep_subtest_begin(vcoreii_diag_subtests_t subtest)
{
  // In this example we turn on the LED and make it green.
  LED_set(lc_GREEN);
}

/******************************************************************************/
// vcoreii_diag_hw_dep_subtest_progress()
// Called in various lengthy subtests.
/******************************************************************************/
void vcoreii_diag_hw_dep_subtest_progress(vcoreii_diag_subtests_t subtest)
{
  // In this example, we toggle the LED between red and green
  LED_toggle(lc_RED, lc_GREEN);
}

/******************************************************************************/
// vcoreii_diag_hw_dep_subtest_end()
// Called whenever a subtest SUCCESSFULLY completes.
/******************************************************************************/
void vcoreii_diag_hw_dep_subtest_end(vcoreii_diag_subtests_t subtest)
{
  // Turn LED off.
  LED_set(lc_OFF);
}

/******************************************************************************/
// vcoreii_diag_hw_dep_tests()
// This is called by vcoreii_diag.c when H/W dependent tests should be executed.
/******************************************************************************/
void vcoreii_diag_hw_dep_tests(bool show_progress, vcoreii_diag_subtests_t tests_to_run, vcoreii_diag_err_info_t *err_info)
{
  if(tests_to_run & VCOREII_DIAG_SUBTEST_HW_DEPENDENT_TESTS) {
    // None - so far.
    if(show_progress) {
      diag_printf("H/W specific tests: Running... Done\n");
    }
  }
}

