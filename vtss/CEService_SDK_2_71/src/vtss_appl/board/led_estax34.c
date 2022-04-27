/* -*- Mode: C; c-basic-offset: 2; tab-width: 8; c-comment-only-line-offset: 0; -*- */
/*

 Vitesse Switch API software.

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

#include <cyg/hal/hal_io.h>

#include "vtss_os.h"
#include "main.h"
#include "led_api.h"
#include "led.h"
#ifdef VTSS_SW_OPTION_SERIALIZED_GPIO
#include "ser_gpio.h"
#endif

#include <cyg/hal/vcoreii.h>

  // Unfortunately we need to bypass the API when setting the GPIO outputs
  // because API calls must be called with VTSS_RCS(), which start by
  // acquiring a semaphore, which is not necessarily available when we need
  // it, because we may have been called from a function that already
  // has the semaphore. So let's re-define the register addresses:
  #define SYSTEM_GPIO (*((volatile unsigned long*)(VCOREII_SWC_REG(7, 0, 0x34)))) /* VCOREII_SWC_REG() is defined in ../eCos/packages/hal/arm/arm9/vcoreii/current/include/vcoreii.h */
  #define SYSTEM_GPIO_F_DATA_VALUE_FPOS     0
  #define SYSTEM_GPIO_F_OUTPUT_ENABLE_FPOS 16

  #define SYSTEM_GPIOCTRL (*((volatile unsigned long*)(VCOREII_SWC_REG(7, 0, 0x33)))) /* VCOREII_SWC_REG() is defined in ../eCos/packages/hal/arm/arm9/vcoreii/current/include/vcoreii.h */
  #define SYSTEM_GPIOCTRL_F_REGCTRL_FPOS   0

  #define LED_FRONT_LED_GPIO_0 13
  #define LED_FRONT_LED_GPIO_1 15

  #define SYSTEM_LEDTIMER VCOREII_SWC_REG(7, 0, 0x3c)
  #define SYSTEM_LEDMODES VCOREII_SWC_REG(7, 0, 0x3d)
  #define LED_FRONT_LCD_GPIO_0 10
  #define LED_FRONT_LCD_GPIO_1 11
  #define LED_PORTSEL(x)       VTSS_ENCODE_BITFIELD(x,26,5)
  #define LED_PORTMODE0(x)     VTSS_ENCODE_BITFIELD(x,0,3)
  #define LED_PORTMODE1(x)     VTSS_ENCODE_BITFIELD(x,3,3)
  #define LED_PORTMODE2(x)     VTSS_ENCODE_BITFIELD(x,6,3)

/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS - LED board dependent functions               */
/*                                                                          */
/****************************************************************************/

/* The reference implementation - estax_34_ref */
void LED_led_init(void)
{
  ulong cur_reg_val;

  // The following piece of code must be protected from pre-emption,
  // but since we are called before the scheduler is started, we are OK

  /*
   * Internal LEDs module - direct via GPIO's
   */

#ifdef VTSS_SW_OPTION_SERIALIZED_GPIO
  // Initializing is done in the serialized gpio module.
#else
  // Set the front LED GPIOs to be outputs.
  cur_reg_val = SYSTEM_GPIO;
  cur_reg_val |= 
    VTSS_BIT((SYSTEM_GPIO_F_OUTPUT_ENABLE_FPOS + LED_FRONT_LED_GPIO_0)) | 
    VTSS_BIT((SYSTEM_GPIO_F_OUTPUT_ENABLE_FPOS + LED_FRONT_LED_GPIO_1));
  SYSTEM_GPIO = cur_reg_val;

  // Set the control register to gate GPIOs.
  cur_reg_val = SYSTEM_GPIOCTRL;
  cur_reg_val |= 
    VTSS_BIT((SYSTEM_GPIOCTRL_F_REGCTRL_FPOS + LED_FRONT_LED_GPIO_0)) | 
    VTSS_BIT((SYSTEM_GPIOCTRL_F_REGCTRL_FPOS + LED_FRONT_LED_GPIO_1));
  SYSTEM_GPIOCTRL = cur_reg_val;
#endif

  /*
   * External LED module - via LED Controller interface
   */

  // Set the control register to gate GPIOs.
  cur_reg_val = SYSTEM_GPIOCTRL;
  cur_reg_val &= ~(VTSS_BIT(LED_FRONT_LCD_GPIO_0) | VTSS_BIT(LED_FRONT_LCD_GPIO_1));
  SYSTEM_GPIOCTRL = cur_reg_val;

  /* Enable as outputs */
  cur_reg_val = (20 << 16) | 0xffff; /* 10MHz CLK, Max pause (bit burst/pause 87/0xffff = 1/753 */
  HAL_WRITE_UINT32(SYSTEM_LEDTIMER, cur_reg_val);

  int i;
  for (i = 0; i <= 29; i++) {
    /* Address the appropriate port */
    HAL_READ_UINT32(SYSTEM_LEDTIMER, cur_reg_val);
    cur_reg_val = (cur_reg_val & ~LED_PORTSEL(0x1f)) | LED_PORTSEL(i);
    HAL_WRITE_UINT32(SYSTEM_LEDTIMER, cur_reg_val);

    /* Init modes */
    cur_reg_val = LED_PORTMODE0(0) | LED_PORTMODE1(0) | LED_PORTMODE2(0);
    HAL_WRITE_UINT32(SYSTEM_LEDMODES, cur_reg_val);
  }
}

/* The reference implementation - estax_34_ref */
void LED_front_led_set(LED_led_colors_t color)
{
  BOOL gpio_0, gpio_1;
  
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

#ifdef VTSS_SW_OPTION_SERIALIZED_GPIO
  uint gpio_ser_vector = gpio_0 << 15 | gpio_1 << 14;
  serialized_gpio_data_wr(gpio_ser_vector,0xC000);
#else 
  // The following piece of code must be protected from pre-emption, because it's a read-modify-write operation.
  // It doesn't help much to protect it with a local semaphore, since the API code may be called in between anyway.
  // Therefore, we protect it by a scheduler lock.
  cyg_scheduler_lock();
  ulong cur_gpio, mask;
  cur_gpio = SYSTEM_GPIO;
  mask      = VTSS_BIT((SYSTEM_GPIO_F_DATA_VALUE_FPOS + LED_FRONT_LED_GPIO_0)) | VTSS_BIT((SYSTEM_GPIO_F_DATA_VALUE_FPOS + LED_FRONT_LED_GPIO_1));
  cur_gpio &= ~mask; // Clear bits.
  cur_gpio |= (gpio_0<<(SYSTEM_GPIO_F_DATA_VALUE_FPOS + LED_FRONT_LED_GPIO_0)) | (gpio_1<<(SYSTEM_GPIO_F_DATA_VALUE_FPOS + LED_FRONT_LED_GPIO_1)); // Set bits
  SYSTEM_GPIO = cur_gpio;
  cyg_scheduler_unlock();
#endif
}

/* The reference implementation - estax_34_ref */
void LED_master_led_set(BOOL master)
{
  ulong cur_reg_val;

  cyg_scheduler_lock();         /* Avoid pre-emption while setting up */
  
  HAL_READ_UINT32(SYSTEM_LEDTIMER, cur_reg_val);
  cur_reg_val = (cur_reg_val & ~LED_PORTSEL(0x1f)) | LED_PORTSEL(29); /* LEDB */
  HAL_WRITE_UINT32(SYSTEM_LEDTIMER, cur_reg_val);

  /* Set modes */
  cur_reg_val = 
    LED_PORTMODE0(0) |          /* -None- */
    LED_PORTMODE1(0) |          /* Yellow */
    LED_PORTMODE2(master ? 1 : 0); /* Green */
  HAL_WRITE_UINT32(SYSTEM_LEDMODES, cur_reg_val);

  cyg_scheduler_unlock();
}

/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS - LCD board dependent functions               */
/*                                                                          */
/****************************************************************************/

/* The reference implementation - estax_34_ref */
void LED_lcd_set(lcd_symbol_t sym1, lcd_symbol_t sym2)
{
  static unsigned char sym2pat[LCD_SYMBOL_COUNT] = {
    [LCD_SYMBOL_0]      = 0xFC, /* 01100000 */
    [LCD_SYMBOL_1]      = 0x60, /* 01100000 */
    [LCD_SYMBOL_2]      = 0xDA, /* 11011010 */
    [LCD_SYMBOL_3]      = 0xF2, /* 11110010 */
    [LCD_SYMBOL_4]      = 0x66, /* 01100110 */
    [LCD_SYMBOL_5]      = 0xB6, /* 10110110 */
    [LCD_SYMBOL_6]      = 0xBE, /* 10111110 */
    [LCD_SYMBOL_7]      = 0xE0, /* 11100000 */
    [LCD_SYMBOL_8]      = 0xFE, /* 11111110 */
    [LCD_SYMBOL_9]      = 0xF6, /* 11110110 */
    [LCD_SYMBOL_A]      = 0xEE, /* 11101110 */
    [LCD_SYMBOL_B]      = 0x3E, /* 00111110 */
    [LCD_SYMBOL_C]      = 0x9C, /* 10011100 */
    [LCD_SYMBOL_D]      = 0x7A, /* 01111010 */
    [LCD_SYMBOL_E]      = 0x9E, /* 10011110 */
    [LCD_SYMBOL_F]      = 0x8E, /* 10001110 */
    [LCD_SYMBOL_BLANK]  = 0x00, /* 00000000 */
    [LCD_SYMBOL_DASH]   = 0x02, /* 00000010 */
    [LCD_SYMBOL_PERIOD] = 0x01, /* 00000001 */
  };

  cyg_scheduler_lock();         /* Avoid pre-emption while setting up */
  
  int led_port;
  ulong cur_reg_val, symval;

  /* 2 digits into one bitstream */
  symval = (sym2pat[sym1] << 8) + sym2pat[sym2];
  for(led_port = 22; led_port <= 27; led_port++) {
    HAL_READ_UINT32(SYSTEM_LEDTIMER, cur_reg_val);
    cur_reg_val = (cur_reg_val & ~LED_PORTSEL(0x1f)) | LED_PORTSEL(led_port);
    HAL_WRITE_UINT32(SYSTEM_LEDTIMER, cur_reg_val);

    /* Set modes */

    HAL_READ_UINT32(SYSTEM_LEDMODES, cur_reg_val);

    // Mode 1&2 is not used for port led port 27. See schematic "Serial LED, Stacking Port ID"
    if (led_port == 27) {
      cur_reg_val &= 0xFFFFFFF8; // Clear the mode 0 bits - Datasheet table 210.
      cur_reg_val |= LED_PORTMODE0((symval>>0) & 1);
    } else {
      cur_reg_val = 
	LED_PORTMODE0((symval>>0) & 1) | 
	LED_PORTMODE1((symval>>1) & 1) | 
	LED_PORTMODE2((symval>>2) & 1);
    }
    HAL_WRITE_UINT32(SYSTEM_LEDMODES, cur_reg_val);
    symval >>= 3;
  }

  cyg_scheduler_unlock();
}
