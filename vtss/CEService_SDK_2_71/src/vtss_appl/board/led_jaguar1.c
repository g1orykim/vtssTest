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

#include "main.h"
#include "led_api.h"
#include "port_custom_api.h"

#ifdef VTSS_SW_OPTION_PHY
#include "misc_api.h"
#include "phy_api.h" // For PHY_INST
#endif

/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS - LED board dependent functions               */
/*                                                                          */
/****************************************************************************/

static LED_led_colors_t jr1_sys_color;
static lcd_symbol_t jr1_sym1, jr1_sym2;
static BOOL jr1_master;

static void jr1_led_update(BOOL init)
{
    u32 i, data;

    static u8 sym2pat[LCD_SYMBOL_COUNT] = {
        [LCD_SYMBOL_0]      = 0xFC, /* 11111100 */
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
    
    switch (vtss_board_type()) {
    case VTSS_BOARD_JAG_CU24_REF:
    case VTSS_BOARD_JAG_PCB107_REF:
    case VTSS_BOARD_JAG_SFP24_REF:
    case VTSS_BOARD_JAG_CU48_REF: 
        /* These board types are handled below */
        break;
    default:
        /* Unknown board types are ignored */
        return;
    }

    /* Initialization: GPIO_22 and GPIO_23 are outputs */
    if (init) {
        vtss_gpio_direction_set(NULL, 0, 22, 1);
        vtss_gpio_direction_set(NULL, 0, 23, 1);

        // Set up LED enhanced led (Used for LED power reduction which is only supported for JR48)
        if (vtss_board_type() == VTSS_BOARD_JAG_CU48_REF) {
          vtss_phy_enhanced_led_control_t conf;
          // We are running non-serial
          conf.ser_led_output_2 = FALSE; 
          conf.ser_led_output_1 = FALSE;
          conf.ser_led_frame_rate = 0;
          conf.ser_led_select = 0;
          
          // Setup all ports since LED control is used in parallel mode.
          for (i = 0; i < 48; i++) {        
            vtss_phy_enhanced_led_control_init(PHY_INST, i, conf);
          }
        }
        return;
    }
    
    /* Use GPIO_22 (clock) and GPIO_23 (data) to update LCDs and master LED (UG1043).
       For the LCD, a zero value must be used to turn ON, so we invert the LCD bits */
    data = (0xffff - sym2pat[jr1_sym2] - (sym2pat[jr1_sym1] << 8));
#if VTSS_SWITCH_STACKABLE
    if (vtss_stacking_enabled() && jr1_master)
        data |= (1 << 19);
#endif 
   for (i = 0; i < 24; i++) {
        vtss_gpio_write(NULL, 0, 23, (data & (1<< i)) ? 1 : 0);
        vtss_gpio_write(NULL, 0, 22, 0);
        vtss_gpio_write(NULL, 0, 22, 1);
    }
    
    /* Update system LED using GPIO_22 and GPIO_23 */
    vtss_gpio_write(NULL, 0, 22, jr1_sys_color == lc_GREEN);
    vtss_gpio_write(NULL, 0, 23, jr1_sys_color == lc_RED);


}

void LED_led_init(void)
{
    jr1_led_update(1);
}

void LED_front_led_set(LED_led_colors_t color)
{
    if (color != jr1_sys_color) {
        jr1_sys_color = color;
        jr1_led_update(0);
    }
}

void LED_master_led_set(BOOL master)
{
    if (master != jr1_master) {
        jr1_master = master;
        jr1_led_update(0);
    }
}

void LED_lcd_set(lcd_symbol_t sym1, lcd_symbol_t sym2)
{
    if (sym1 != jr1_sym1 || sym2 != jr1_sym2) {
        jr1_sym1 = sym1;
        jr1_sym2 = sym2;
        jr1_led_update(0);
    }
}
